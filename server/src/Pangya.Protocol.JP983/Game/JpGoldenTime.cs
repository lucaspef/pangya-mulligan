using System.Security;
using System.Text;
using Pangya.Core.Config;
using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Golden Time (docs/server/protocolo/SPEC-jp983-eventos-drops.md §1): rodadas nos horários da config (GameJp.GoldenTime.Rounds)
/// ou pelo GM ("!goldentime"). 5 min antes: aviso 0x1D3 tipo 11; na hora: tipo 12; sorteio entre quem está no lounge ou
/// jogando (peso 100 + 100 jogando + 100 premium; 1 ganhador a cada 100 × taxa, mínimo 1); tipo 14 com os nicks, fogos
/// no lounge e o item por carta; depois tipo 15 (próxima rodada) ou 17 (fim do dia); ninguém: tipo 18.
/// </summary>
public sealed class JpGoldenTime(GameContext ctx, GoldenTimeConfig cfg)
{
    const ushort SBroadcast = 0x1D3;
    const uint TypeAnnounce = 11, TypeStart = 12, TypeWinners = 14, TypeNext = 15, TypeDayEnd = 17, TypeNoWinners = 18;

    static string Time(DateTime t) => t.ToString("yyyy-MM-dd HH:mm:ss.fff");

    /// <summary>0x1D3 u32 1, u32 tipo, str XML a todos os conectados do JP.</summary>
    void Broadcast(uint type, string xml)
    {
        foreach (var s in ctx.World.Online)
            if (s is GameHandler h && h.Connection.Translator is JpTranslator)
                h.Connection.SendTranslated(new PacketWriter(SBroadcast).U32(1).U32(type).Str(xml));
    }

    static string Params(DateTime start, GoldenTimeItem? item, IEnumerable<string>? nicks = null, DateTime? next = null)
    {
        var sb = new StringBuilder("<PARAMS><EVENTSTARTTIME>").Append(Time(start)).Append("</EVENTSTARTTIME>");
        if (next is { } n) sb.Append("<EVENTNEXTTIME>").Append(Time(n)).Append("</EVENTNEXTTIME>");
        if (item != null) sb.Append("<TYPEID>").Append((uint)item.TypeId).Append("</TYPEID><QTY>").Append(item.Days > 0 ? item.Days : item.Quantity).Append("</QTY>");
        if (nicks != null) foreach (var nick in nicks) sb.Append("<NICKNAME>").Append(SecurityElement.Escape(nick)).Append("</NICKNAME>");
        return sb.Append("</PARAMS>").ToString();
    }

    GoldenTimeItem? DrawItem(Random rng)
    {
        int total = 0;
        foreach (var it in cfg.Items) total += Math.Max(1, it.Weight);
        if (total == 0) return null;
        int roll = rng.Next(total);
        foreach (var it in cfg.Items) { if ((roll -= Math.Max(1, it.Weight)) < 0) return it; }
        return null;
    }

    /// <summary>Quem concorre: no lounge sempre; em outras salas só com a partida em andamento (sem prática nem Chip-in).</summary>
    static bool Eligible(GameHandler h, out bool playing)
    {
        playing = false;
        if (h.CurrentRoom is not { } r) return false;
        if (r.Settings.Mode == GameMode.AvatarChat) return true;
        if (r.Game is not { Over: false } || r.Settings.Practice || r.Settings.Zodiac == ZodiacKind.ChipIn) return false;
        playing = true;
        return true;
    }

    /// <summary>Uma rodada agora (GM ou horário). Devolve quantos ganharam.</summary>
    public async Task<int> RoundAsync(DateTime? nextRound = null, Random? random = null)
    {
        var rng = random ?? Random.Shared;
        var now = DateTime.Now;
        if (DrawItem(rng) is not { } item) return 0;
        Broadcast(TypeStart, Params(now, item));
        var pool = new List<(GameHandler H, int Weight)>();
        lock (ctx.World.Rooms.Sync)
            foreach (var s in ctx.World.Online)
                if (s is GameHandler h && Eligible(h, out bool playing))
                    pool.Add((h, 100 + (playing ? 100 : 0) + (Premium.IsActive(h.Player, DateTime.UtcNow) ? 100 : 0)));
        if (pool.Count == 0)
        {
            Broadcast(TypeNoWinners, Params(now, item));
            Log.Info("Golden Time: ninguém concorrendo");
            return 0;
        }
        int n = Math.Clamp((int)(pool.Count / 100.0 * Math.Max(1, cfg.WinnersPer100)), 1, pool.Count);
        var winners = new List<GameHandler>(n);
        for (int k = 0; k < n; k++)
        {
            int total = 0;
            foreach (var x in pool) total += x.Weight;
            int roll = rng.Next(total), i = 0;
            for (; i < pool.Count - 1; i++) { if (roll < pool[i].Weight) break; roll -= pool[i].Weight; }
            winners.Add(pool[i].H);
            pool.RemoveAt(i);
        }
        var nicks = new List<string>(winners.Count);
        foreach (var w in winners) nicks.Add(w.Player.Nickname);
        Broadcast(TypeWinners, Params(now, item, nicks));
        lock (ctx.World.Rooms.Sync)
            foreach (var w in winners)
                if (w.CurrentRoom is { Settings.Mode: GameMode.AvatarChat } lounge && lounge.Find(w) is { } rp)
                    InGameOutput.Broadcast(lounge, GameHandler.FireworksPacket(rp.Guid));
        if (ctx.Mail is { } mail)
            foreach (var w in winners)
                await mail.SendSystemAsync(w.Player.AccountId, "@GoldenTime", $"Golden Time - Round({now:HH:mm})", [(item.TypeId, Math.Max(1, item.Quantity))]);
        Broadcast(nextRound is { } nr ? TypeNext : TypeDayEnd,
            nextRound is { } nx ? Params(now, null, null, nx) : "<PARAMS><BroadCastReservedNoticesIdx>53</BroadCastReservedNoticesIdx></PARAMS>");
        Log.Info($"Golden Time: {item.TypeId:X8} x{item.Quantity} para {string.Join(", ", nicks)}");
        return winners.Count;
    }

    /// <summary>GM no chat: "!goldentime" faz uma rodada agora.</summary>
    public string? Gm(string command)
    {
        if (command.Trim() != "!goldentime") return null;
        _ = RoundAsync();
        return "Rodada do Golden Time sorteada.";
    }

    /// <summary>Horários do dia: aviso 5 min antes e a rodada na hora.</summary>
    public async Task ScheduleAsync(CancellationToken ct)
    {
        var rounds = new List<int>();
        foreach (var s in cfg.Rounds) if (TimeOnly.TryParse(s, out var t)) rounds.Add(t.Hour * 60 + t.Minute);
        rounds.Sort();
        if (!cfg.Enabled || rounds.Count == 0) return;
        int last = -1;
        while (!ct.IsCancellationRequested)
        {
            var now = DateTime.Now;
            int minute = now.Hour * 60 + now.Minute;
            if (minute != last)
            {
                last = minute;
                for (int i = 0; i < rounds.Count; i++)
                {
                    if (rounds[i] - 5 == minute) Broadcast(TypeAnnounce, Params(now.Date.AddMinutes(rounds[i]), null));
                    if (rounds[i] == minute)
                        _ = RoundAsync(i + 1 < rounds.Count ? now.Date.AddMinutes(rounds[i + 1]) : null);
                }
            }
            try { await Task.Delay(TimeSpan.FromSeconds(20), ct); }
            catch (OperationCanceledException) { return; }
        }
    }
}
