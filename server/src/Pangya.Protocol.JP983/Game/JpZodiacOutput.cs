using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Pacotes do Grand Zodiac no cliente JP (docs/server/protocolo/SPEC-jp983-grand-zodiac.md §2): ids do JP, mandados direto na
/// conexão de cada jogador (o KR não tem esses pacotes). Evento (11/13): golden beam (0x1F0/0x1F1 e o jackpot no 0x40
/// tipo 13/14), item que cai (0x40 tipo 15), troféu (0x1FA) e os pontos GZ da conta.
/// </summary>
public sealed class JpZodiacOutput(Room room, IZodiacPointsStore? points = null) : IZodiacOutput
{
    const ushort SWind = 0x5B, SSeed = 0x1EC, SLoaded = 0x201, SElapsed = 0x8D, SHoleStart = 0x53, SPosition = 0x1EE,
        SNextHole = 0x1F4, SBonusTypes = 0x1F5, SEndShot = 0xCC, SHoleScore = 0x1EF, SSeconds = 0x200, SEnded = 0x1F2,
        STimeOver = 0x8C, SStandings = 0x1F3, SAfterGame = 0xA7, SMoney = 0xAA, SDone = 0x24F, SBeamOpen = 0x1F0, SBeamClose = 0x1F1,
        STrophy = 0x1FA, SNotice = 0x40;
    const byte NoticeJackpotOne = 13, NoticeJackpotSplit = 14, NoticeDrop = 15;
    const int PangPouch = 0x1A000010;

    GameHandler? Handler(ZodiacPlayer p)
    {
        foreach (var rp in room.Players) if (rp.Guid == p.Guid && rp.Session is GameHandler h) return h;
        return null;
    }

    void To(ZodiacPlayer p, PacketWriter w)
    {
        if (Handler(p) is { } h) h.Connection.SendTranslated(w); else w.Dispose();
    }

    void All(Func<PacketWriter> make)
    {
        foreach (var rp in room.Players) if (rp.Session is GameHandler h) h.Connection.SendTranslated(make());
    }

    /// <summary>No intermediário (11) todos veem a posição de todos; no avançado e no chip-in, só a sua.</summary>
    void Position(ZodiacPlayer p)
    {
        if (room.Settings.Zodiac == ZodiacKind.Intermediate) All(() => PositionOf(p)); else To(p, PositionOf(p));
    }

    static PacketWriter Wind(byte wind, byte direction) => new PacketWriter(SWind).U8(wind).U8(0).U16(direction).U8(1);
    static PacketWriter PositionOf(ZodiacPlayer p) => new PacketWriter(SPosition).U32(p.Guid).F32(p.TeeX).F32(p.TeeZ);

    string NickOf(ZodiacPlayer p) => Handler(p)?.Player.Nickname ?? "";

    public void Loaded(ZodiacPlayer p, byte wind, byte direction, double[] seeds)
    {
        To(p, Wind(wind, direction));
        foreach (var s in seeds) To(p, new PacketWriter(SSeed).Bytes(BitConverter.GetBytes(s)));
        To(p, new PacketWriter(SLoaded));
    }

    public void FirstHole(ZodiacPlayer p)
    {
        To(p, new PacketWriter(SElapsed).U32(0));
        To(p, new PacketWriter(SHoleStart).U32(p.Guid));
        Position(p);
        To(p, new PacketWriter(SNextHole).U32(1));
    }

    public void ScoreTypes(ZodiacPlayer p, List<byte> types)
    {
        var w = new PacketWriter(SBonusTypes).U32((uint)types.Count);
        foreach (var t in types) w.U32(t);
        To(p, w);
    }

    public void EndShot(ZodiacPlayer p) => To(p, new PacketWriter(SEndShot).U32(p.Guid).U8(0));

    public void Holed(ZodiacPlayer p, int before, int points, byte wind, byte direction, uint seconds)
    {
        All(() => new PacketWriter(SHoleScore).U32(p.Guid).I32(before).I32(points).I32(p.Total));
        To(p, new PacketWriter(SNextHole).U32(1));
        To(p, Wind(wind, direction));
        To(p, new PacketWriter(SSeconds).U32(seconds));
        Position(p);
    }

    public void BackToTee(ZodiacPlayer p) => Position(p);

    public void TimeOver(ZodiacPlayer p, uint totalMs) => To(p, new PacketWriter(SElapsed).U32(totalMs));

    public void Ended(ZodiacPlayer p) => To(p, new PacketWriter(SEnded));

    public void BeamOpen() => All(() => new PacketWriter(SBeamOpen));

    public void BeamClose(List<ZodiacPlayer> winners, bool split)
    {
        All(() => new PacketWriter(SBeamClose));
        if (split)
            foreach (var w in winners)
                To(w, new PacketWriter(SNotice).U8(NoticeJackpotSplit).Str(NickOf(w)).U16(0).U32(PangPouch).U64((ulong)w.Jackpot));
        else if (winners.Count == 1)
        {
            var w = winners[0];
            string nick = NickOf(w);
            All(() => new PacketWriter(SNotice).U8(NoticeJackpotOne).Str(nick).U16(0).U32(PangPouch).U64((ulong)w.Jackpot));
        }
    }

    public void ItemDropped(ZodiacPlayer p, int typeId, int quantity)
    {
        string nick = NickOf(p);
        All(() => new PacketWriter(SNotice).U8(NoticeDrop).Str(nick).U16(0).U32((uint)typeId).U64((ulong)quantity));
    }

    /// <summary>
    /// Fim: a cada um 0x1FA (troféu), 0x8C, 0x1F3 (u32 1 terminou / 2 sem completar, u32 n, n × (i32 oid, u32 posição, i32 pontos,
    /// u32 HIOs, i32 tacadas, u32 pontos GZ, i32 EXP, u64 pang, u64 bônus, u64 jackpot, u32 troféu, u32 k, k × (u32 typeid,
    /// u32 qtd))), a recompensa (pang; o EXP do modo) e 0xA7, 0xAA, 0x24F. Pontos GZ, itens, jackpot e troféu vão para a conta.
    /// </summary>
    public void GameOver(List<ZodiacResult> results)
    {
        foreach (var me in results)
        {
            if (Handler(me.Player) is not { } h) continue;
            if (me.Trophy != 0) h.Connection.SendTranslated(new PacketWriter(STrophy).U32((uint)me.Trophy).I32(me.Trophy));
            h.Connection.SendTranslated(new PacketWriter(STimeOver));
            var w = new PacketWriter(SStandings, 16 + results.Count * 80).U32(me.Player.Complete ? 1u : 2u).U32((uint)results.Count);
            foreach (var r in results)
            {
                var p = r.Player;
                w.U32(p.Guid).U32((uint)r.Position).I32(p.Total).U32((uint)p.HoleInOnes).I32(p.TotalShots)
                    .U32(r.GzPoints).I32(r.Exp).U64(p.Pang).U64(p.Bonus).U64((ulong)p.Jackpot).U32((uint)r.Trophy).U32((uint)p.Drops.Count);
                foreach (var (tid, qty) in p.Drops) w.U32((uint)tid).U32((uint)qty);
            }
            h.Connection.SendTranslated(w);
            bool chipIn = room.Settings.Zodiac == ZodiacKind.ChipIn;
            h.BeginGameEnd(me.Player.Pang, me.Player.Bonus, Math.Max(1, me.Player.HoleInOnes), true, practice: true,
                fixedExp: chipIn ? null : me.Exp);
            _ = ExtrasAsync(h, me);
            h.Connection.SendTranslated(new PacketWriter(SAfterGame).U8(0));
            h.Connection.SendTranslated(new PacketWriter(SMoney).U16(0).U64((ulong)h.Player.Pang).U64((ulong)h.Player.Cookie));
            h.Connection.SendTranslated(new PacketWriter(SDone).U32(0));
        }
    }

    /// <summary>O que vai para a conta além do pang/EXP da partida: jackpot, itens que caíram, troféu e pontos GZ.</summary>
    async Task ExtrasAsync(GameHandler h, ZodiacResult r)
    {
        try
        {
            var p = h.Player;
            var ctx = h.Context;
            if (r.Player.Jackpot > 0)
            {
                p.Pang += r.Player.Jackpot;
                await ctx.Players.Store.ApplyAsync(p.AccountId, new PlayerChanges { Pang = p.Pang });
            }
            foreach (var (tid, qty) in r.Player.Drops) await ctx.Shop.GiveAsync(p, tid, qty);
            if (r.Trophy != 0 && ctx.SpecialTrophies is { } trophies) await trophies.AddAsync(p.AccountId, r.Trophy, 1);
            if (r.GzPoints > 0 && points != null)
            {
                int total = await points.AddAsync(p.AccountId, (int)r.GzPoints);
                if (h.Connection.Translator is JpTranslator t) t.ZodiacPoints = total;
            }
            if (r.Player.Jackpot > 0 || r.Trophy != 0 || r.GzPoints > 0)
                Log.Info($"{h.Connection} Grand Zodiac: {r.Position}º, +{r.GzPoints} pontos GZ, jackpot {r.Player.Jackpot}, troféu {r.Trophy:X8}");
        }
        catch (Exception e) { Log.Error($"{h.Connection} falha nos prêmios do Grand Zodiac", e); }
    }
}
