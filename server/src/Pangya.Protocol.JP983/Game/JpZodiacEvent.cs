using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Evento do Grand Zodiac (SPEC-jp983-grand-zodiac.md §1.2): o servidor abre as salas "HIO Event" (11 intermediário, 13
/// avançado) pelo comando de GM "!zodiac int|adv" ou no horário da config (GameJp.ZodiacSchedule, "HH:MM int|adv").
/// A sala começa sozinha 2 min depois de entrar o 1º jogador (ou na hora se lotar), com a contagem de 10 s (0x40 tipo 11).
/// </summary>
public sealed class JpZodiacEvent(GameContext ctx)
{
    /// <summary>Espera depois do 1º jogador antes da contagem (referência: 2 min).</summary>
    public TimeSpan StartWait { get; set; } = TimeSpan.FromMinutes(2);
    const int CountdownSeconds = 10;
    const ushort SNotice = 0x40;
    const byte NoticeCountdown = 11;

    public static ZodiacKind? Parse(string s) => s.Trim().ToLowerInvariant() switch
    {
        "int" or "intermediate" or "11" => ZodiacKind.Intermediate,
        "adv" or "advanced" or "13" => ZodiacKind.Advanced,
        _ => null,
    };

    /// <summary>Abre uma sala do evento e devolve o índice.</summary>
    public int Open(ZodiacKind kind)
    {
        Room r;
        lock (ctx.World.Rooms.Sync)
        {
            var s = new RoomSettings
            {
                Title = kind == ZodiacKind.Advanced ? "HIO Event (Advanced)" : "HIO Event (Intermediate)",
                Mode = GameMode.Tournament, Zodiac = kind, MaxPlayers = 30,
            };
            r = ctx.World.Rooms.Create(s, 0);
        }
        Log.Info($"Grand Zodiac: sala {r.Index} '{r.Settings.Title}' aberta");
        _ = RunAsync(r);
        return r.Index;
    }

    /// <summary>GM no chat: "!zodiac int|adv". Devolve a resposta (null = não é este comando).</summary>
    public string? Gm(string command)
    {
        var parts = command.Trim().Split(' ', StringSplitOptions.RemoveEmptyEntries);
        if (parts.Length == 0 || parts[0] != "!zodiac") return null;
        if (parts.Length < 2 || Parse(parts[1]) is not { } kind) return "Uso: !zodiac int|adv";
        return $"Sala {Open(kind)} do Grand Zodiac aberta.";
    }

    async Task RunAsync(Room room)
    {
        try
        {
            DateTime? firstJoin = null;
            int countdown = -1;
            while (true)
            {
                await Task.Delay(TimeSpan.FromSeconds(1));
                List<GameHandler> handlers = [];
                bool full;
                lock (ctx.World.Rooms.Sync)
                {
                    if (room.State != RoomState.Waiting || ctx.World.Rooms.Get(room.Index) != room) return;
                    foreach (var rp in room.Players) if (rp.Session is GameHandler h) handlers.Add(h);
                    full = room.Players.Count >= room.Settings.MaxPlayers;
                }
                if (handlers.Count == 0) { firstJoin = null; countdown = -1; continue; }   // esvaziou: volta a esperar
                firstJoin ??= DateTime.UtcNow;
                if (countdown < 0 && (full || DateTime.UtcNow - firstJoin >= StartWait)) countdown = CountdownSeconds;
                if (countdown < 0) continue;
                if (countdown == 0) { handlers[0].StartAsServer(); return; }
                foreach (var h in handlers)
                    h.Connection.SendTranslated(new PacketWriter(SNotice).U8(NoticeCountdown).Str("").Str("").U32((uint)countdown));
                countdown--;
            }
        }
        catch (Exception e) { Log.Error($"Grand Zodiac: falha na sala {room.Index}", e); }
    }

    /// <summary>Horários da config: abre um tipo por linha "HH:MM int|adv" quando o relógio do servidor passa por ela.</summary>
    public async Task ScheduleAsync(string[] schedule, CancellationToken ct)
    {
        var entries = new List<(int Minute, ZodiacKind Kind)>();
        foreach (var line in schedule)
        {
            var parts = line.Split(' ', StringSplitOptions.RemoveEmptyEntries);
            if (parts.Length == 2 && TimeOnly.TryParse(parts[0], out var t) && Parse(parts[1]) is { } k) entries.Add((t.Hour * 60 + t.Minute, k));
            else Log.Warn($"Grand Zodiac: horário inválido na config: '{line}'");
        }
        if (entries.Count == 0) return;
        int last = -1;
        while (!ct.IsCancellationRequested)
        {
            var now = DateTime.Now;
            int minute = now.Hour * 60 + now.Minute;
            if (minute != last)
            {
                last = minute;
                foreach (var (m, k) in entries) if (m == minute) Open(k);
            }
            try { await Task.Delay(TimeSpan.FromSeconds(20), ct); }
            catch (OperationCanceledException) { return; }
        }
    }
}
