using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Mission do lobby (CLobbyTask 0x189/0x18A): 0xE1 abre -> 0x189 u32 7, 7 × {i32 valor, u8 completa}, u8 prêmio do dia já pego,
/// u8 do período já pego (o cliente lê os 7: com menos a janela lê fora). 0xE2 u32 tipo (1 dia, 2 período) -> 0x18A u32
/// 0 carta enviada, 1 já recebeu, 2 falhou. Os eagles vêm buraco a buraco (OnHoled); o resto, do fim da partida.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CMissionGift = 0xE2;
    const ushort SMission = 0x189, SMissionGift = 0x18A;

    public Domain.Players.MissionEventService? Mission { get; set; }
    int gameEagles;
    object? eagleGame;

    async Task<bool> HandleMissionAsync(PacketReader r)
    {
        int type = r.Id == CMissionGift && r.Remaining >= 4 ? r.I32() : 0;
        r.Skip(r.Remaining);
        if (Mission is not { } m || !m.Open(Today)) return EventUnavailable(r);
        long uid = kr.Player.AccountId;
        if (r.Id == CMissionGift)
        {
            int code = await m.ClaimAsync(uid, type, Today);
            conn.SendTranslated(new PacketWriter(SMissionGift).U32((uint)code));
            if (code == Domain.Players.MissionEventService.GiftSent && ctx.Mail != null) kr.Connection.Send(new PacketWriter(0x31));
            Log.Info($"{conn} JP: prêmio do Mission ({(type == 1 ? "dia" : "período")}): {code}");
            return true;
        }
        var p = await m.GetAsync(uid, Today);
        var w = new PacketWriter(SMission, 48).U32(7);
        foreach (var (cond, done) in Domain.Players.MissionEventService.Missions(p)) w.I32(cond).U8(done ? (byte)1 : (byte)0);
        conn.SendTranslated(w.U8(p.DayGift ? (byte)1 : (byte)0).U8(p.TermGift ? (byte)1 : (byte)0));
        return true;
    }

    /// <summary>Eagle (ou melhor) no buraco que o jogador acabou de acertar; chamado sob o lock da sala.</summary>
    void CountEagle(Room room)
    {
        if (room.Game is not { } g) return;
        if (eagleGame != g) { eagleGame = g; gameEagles = 0; }
        uint guid = room.Find(kr)?.Guid ?? 0;
        (int strokes, int par) = g switch
        {
            StrokeGame s when s.Find(guid) is { } p => (p.Strokes[Math.Min(s.HoleIndex, 17)], s.ParOf(s.Hole)),
            MassGame m when m.Find(guid) is { } p => (p.Strokes[Math.Min(p.HoleIndex, 17)], m.ParOf(m.HoleOfPlayer(p))),
            _ => (0, 0),
        };
        if (strokes > 0 && par > 0 && strokes <= par - 2) gameEagles++;
    }

    public async Task MissionGameEndAsync(Domain.Achievements.AchievementCounters.GameEnd g)
    {
        if (Mission is not { } m) return;
        int eagles = gameEagles;
        gameEagles = 0;
        try
        {
            if (await m.OnGameEndAsync(kr.Player.AccountId, g.Mode, g.Practice, g.Finished, g.Course, g.Holes,
                    g.Stats?.LongestPuttIn ?? 0, g.NewRecord, eagles, Today) is { } next)
                Log.Info($"{conn} JP: Mission dia {next.DayVersus}/{next.DayTourney}, período {next.Versus} versus, {next.Eagles} eagles, campos 0x{next.Courses:X4}");
        }
        catch (Exception e) { Log.Error($"{conn} falha no Mission", e); }
    }
}
