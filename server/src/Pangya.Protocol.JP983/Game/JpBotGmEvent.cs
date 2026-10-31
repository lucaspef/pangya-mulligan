using Pangya.Core.Config;
using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Bot GM Event (docs/server/protocolo/SPEC-jp983-eventos-drops.md §2): nas janelas da config (GameJp.BotGmEvent.Windows) ou
/// pelo GM ("!botgm"), o servidor sorteia 1–2 prêmios, avisa todos (0x1D3 tipo 20, texto) e abre uma sala de torneio de
/// 18 buracos (mapa aleatório, ordem sorteada, 35 min). Começa 2 min depois do 1º jogador (contagem 0x40 tipo 11); quem
/// terminar a partida recebe os prêmios por carta.
/// </summary>
public sealed class JpBotGmEvent(GameContext ctx, BotGmEventConfig cfg)
{
    const ushort SBroadcast = 0x1D3, SNotice = 0x40;
    const uint TypeText = 20;
    const byte NoticeCountdown = 11;
    public TimeSpan StartWait { get; set; } = TimeSpan.FromMinutes(2);

    void Broadcast(string text)
    {
        foreach (var s in ctx.World.Online)
            if (s is GameHandler h && h.Connection.Translator is JpTranslator)
                h.Connection.SendTranslated(new PacketWriter(SBroadcast).U32(1).U32(TypeText).Str(text));
    }

    List<BotGmPrize> DrawPrizes(Random rng)
    {
        var pool = new List<BotGmPrize>(cfg.Prizes);
        int n = Math.Min(pool.Count, rng.Next(Math.Max(1, cfg.MinPrizes), Math.Max(cfg.MinPrizes, cfg.MaxPrizes) + 1));
        var list = new List<BotGmPrize>(n);
        for (int k = 0; k < n && pool.Count > 0; k++)
        {
            int total = 0;
            foreach (var x in pool) total += Math.Max(1, x.Weight);
            int roll = rng.Next(total), i = 0;
            for (; i < pool.Count - 1; i++) { if (roll < Math.Max(1, pool[i].Weight)) break; roll -= Math.Max(1, pool[i].Weight); }
            list.Add(pool[i]);
            pool.RemoveAt(i);
        }
        return list;
    }

    /// <summary>Abre o evento agora; devolve o índice da sala.</summary>
    public int Open(Random? random = null)
    {
        var prizes = DrawPrizes(random ?? Random.Shared);
        Room r;
        lock (ctx.World.Rooms.Sync)
        {
            var s = new RoomSettings
            {
                Title = "Bot GM Event", Mode = GameMode.Tournament, Holes = 18, HoleType = 3, Course = RoomManager.RandomCourse,
                MaxPlayers = 30, GameTimeMs = 35 * 60_000,
            };
            r = ctx.World.Rooms.Create(s, 0);
        }
        var names = new List<string>();
        foreach (var p in prizes) names.Add($"{p.TypeId:X8}({p.Quantity})");
        string text = $"Bot GM Event: sala {r.Index} aberta, o jogo começa 2 minutos depois do primeiro jogador. Prêmios: {string.Join(", ", names)}";
        Broadcast(text);
        Broadcast(text);
        Log.Info($"Bot GM Event: sala {r.Index}, prêmios {string.Join(", ", names)}");
        _ = RunAsync(r, prizes);
        return r.Index;
    }

    public string? Gm(string command) => command.Trim() == "!botgm" ? $"Bot GM Event aberto na sala {Open()}." : null;

    async Task RunAsync(Room room, List<BotGmPrize> prizes)
    {
        try
        {
            DateTime? first = null;
            int countdown = -1;
            RoomGame? game = null;
            while (game == null)                                                  // espera e contagem
            {
                await Task.Delay(TimeSpan.FromSeconds(1));
                List<GameHandler> handlers = [];
                lock (ctx.World.Rooms.Sync)
                {
                    if (ctx.World.Rooms.Get(room.Index) != room) return;
                    if (room.State == RoomState.Playing) { game = room.Game; break; }
                    foreach (var rp in room.Players) if (rp.Session is GameHandler h) handlers.Add(h);
                }
                if (handlers.Count == 0) { first = null; countdown = -1; continue; }
                first ??= DateTime.UtcNow;
                if (countdown < 0 && DateTime.UtcNow - first >= StartWait) countdown = 10;
                if (countdown < 0) continue;
                if (countdown == 0) { handlers[0].StartAsServer(); continue; }
                foreach (var h in handlers) h.Connection.SendTranslated(new PacketWriter(SNotice).U8(NoticeCountdown).Str("").Str("").U32((uint)countdown));
                countdown--;
            }
            if (game is not MassGame mass) return;
            while (!mass.Over) await Task.Delay(TimeSpan.FromSeconds(2));       // fim: quem terminou leva os prêmios
            var winners = new List<long>();
            lock (ctx.World.Rooms.Sync)
                foreach (var p in mass.Players) if (p.Finished && !p.Left && !p.IsBot) winners.Add(p.RoomPlayer.Player.AccountId);
            if (ctx.Mail is { } mail)
            {
                var attach = new List<(int, int)>();
                foreach (var p in prizes) attach.Add((p.TypeId, Math.Max(1, p.Quantity)));
                foreach (var acc in winners) await mail.SendSystemAsync(acc, "@BotGM", $"Bot GM Event ({DateTime.Now:yyyy-MM-dd})", attach);
            }
            Log.Info($"Bot GM Event: sala {room.Index} terminou, {winners.Count} jogador(es) premiado(s)");
        }
        catch (Exception e) { Log.Error($"Bot GM Event: falha na sala {room.Index}", e); }
    }

    /// <summary>Janelas da config ("HH:MM"): abre uma sala quando o relógio passa por ela.</summary>
    public async Task ScheduleAsync(CancellationToken ct)
    {
        var starts = new List<int>();
        foreach (var s in cfg.Windows) if (TimeOnly.TryParse(s, out var t)) starts.Add(t.Hour * 60 + t.Minute);
        if (!cfg.Enabled || starts.Count == 0) return;
        int last = -1;
        while (!ct.IsCancellationRequested)
        {
            int minute = DateTime.Now.Hour * 60 + DateTime.Now.Minute;
            if (minute != last)
            {
                last = minute;
                foreach (var m in starts) if (m == minute) Open();
            }
            try { await Task.Delay(TimeSpan.FromSeconds(20), ct); }
            catch (OperationCanceledException) { return; }
        }
    }
}
