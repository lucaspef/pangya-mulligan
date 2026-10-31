using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Pedidos que só o cliente JP faz e que precisam do Domain (assíncronos): o tradutor os deixa passar com o id do JP
/// (acima de 0x100, fora dos ids do KR que o tradutor produz) e o JpGameHandler os entrega aqui em vez de ao KR.
/// Papel Shop/Big Papel e Memorial Shop: docs/server/protocolo/SPEC-jp983-papel-memorial.md.
/// </summary>
public sealed partial class JpSystems(Connection conn, GameHandler kr, GameContext ctx, MemorialService? memorial,
    Domain.Achievements.AchievementService? achievements = null, Domain.Rooms.IGrandPrixStore? gpStore = null,
    Domain.Players.AttendanceService? attendanceService = null, Domain.Achievements.DailyQuestService? dailyQuests = null,
    TikiService? tiki = null, GachaDesk? gacha = null)
{
    public const ushort CBigPapel = 0x186, CMemorial = 0x17F;
    const ushort SBigPapel = 0x26C, SPapelTimes = 0xFB, SPang = 0xC8, SMemorial = 0x264, SItemsChanged = 0x216;
    /// <summary>Erro genérico do Memorial (referência: sistema indisponível).</summary>
    const uint MemorialOff = 0x6300300;

    /// <summary>Ids que este tratador atende (o tradutor os deixa passar).</summary>
    public static readonly HashSet<int> Ids =
        [CBigPapel, CMemorial, CMailList, CMailRead, CMailSend, CMailTake, CMailDelete, CAchievements, CGpEnter, CGpLeave,
            CAttendance, CAttendanceAck, CWsLevel, CWsLevelOk, CWsLevelCancel, CWsRank, CWsTransformYes, CWsTransformNo, CWsRecover,
            CWsTransfer, CWsReset, CWsEvent, CWsEventCount, CMasteryExpand, CStatUp, CStatDown, CCardEquip, CCardPatch, CCardRemove, CLolo,
            CDailyOpen, CDailyAccept, CDailyPrize, CDailyGiveUp, CPointShopOpen, CPointShopBalance, CPointShopSell, CPointShopBuy, CTikiShop,
            CWebKey, CWebEnter, CWebClubClosed, CGuildPending, CGuildAnswer, CGuildInviteAnswer,
            CAssistToggle, CAssistGreen, CAutoCommand, CChangeServer, CTicker, CTickerQueue, CGachaClosed, CWorldTourGift, CMyRoomBox, CRecentPlayers, CTikiReportUse, CTikiReportOpen];

    public async ValueTask<bool> HandleAsync(PacketReader p)
    {
        switch (p.Id)
        {
            case CBigPapel: await BigPapelAsync(); return true;
            case CMemorial: await MemorialAsync(p.Remaining >= 4 ? (int)p.U32() : 0); return true;
            case CAchievements: await AchievementListAsync(p.Remaining >= 4 ? p.U32() : 0); return true;
            case CGpEnter: await GrandPrixEnterAsync(p.Remaining >= 4 ? (int)p.U32() : 0); return true;
            case CGpLeave: p.Skip(p.Remaining); GrandPrixLeave(); return true;
            case CAttendance: p.Skip(p.Remaining); await AttendanceAsync(); return true;
            case CAttendanceAck:
                p.Skip(p.Remaining);
                conn.SendTranslated(Ari(SAttendanceAck, attendance ?? new Domain.Players.AttendanceResult(false, 0, (0, 0), (0, 0)), registered: true));
                return true;
            case >= CWsLevel and <= CWsReset or CWsEvent or CWsEventCount: return await HandleWorkshopAsync(p);
            case >= CMasteryExpand and <= CCardRemove or CLolo: return await HandleCharacterAsync(p);
            case >= CDailyOpen and <= CDailyGiveUp: return await HandleDailyAsync(p);
            case >= CPointShopOpen and <= CPointShopBuy or CTikiShop: return await HandleTikiAsync(p);
            case CAssistToggle or CAssistGreen or CAutoCommand or CChangeServer: return await HandleEffectsAsync(p);
            case CTicker or CTickerQueue: return await HandleTickerAsync(p);
            case CGachaClosed: p.Skip(p.Remaining); GachaClosed(); return true;
            case CWorldTourGift: p.Skip(p.Remaining); await WorldTourGiftAsync(); return true;
            case CMyRoomBox: await MyRoomBoxAsync(p.Remaining >= 4 ? (int)p.U32() : 0); return true;
            case CRecentPlayers: p.Skip(p.Remaining); await RecentPlayersAsync(); return true;
            case CTikiReportUse or CTikiReportOpen: return await HandleTikiReportAsync(p);
            case CWebKey or CWebEnter or CWebClubClosed or CGuildPending or CGuildAnswer or CGuildInviteAnswer: return await HandleGuildAsync(p);
            default: return await HandleMailAsync(p);
        }
    }

    /// <summary>
    /// Big Papel: 0x186 (vazio) -> 0xC8 u64 pang, u64 0 + 0xFB i32 -1, i32 -3 (sem limite de jogadas) + 0x26C u32 0, i32 0 (sem cupom), u32 n,
    /// n × bola (i32 cor, u32 typeid, u32 id, u32 qtd, u32 classe), u64 pang, u64 cookie; erro: só o u32 código.
    /// </summary>
    async Task BigPapelAsync()
    {
        var p = kr.Player;
        var r = await ctx.Lottery.PlayBigPapelAsync(p);
        var w = new PacketWriter(SBigPapel, 40 + r.Prizes.Count * 20).U32(r.Code);
        if (r.Code == 0)
        {
            w.I32(0).U32((uint)r.Prizes.Count);
            foreach (var pz in r.Prizes) w.I32((int)pz.Class).U32((uint)pz.TypeId).U32((uint)pz.Id).U32((uint)pz.Quantity).U32((uint)pz.Class);
            w.U64((ulong)p.Pang).U64((ulong)p.Cookie);
        }
        if (r.Code == 0) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));   // a tela do pang (referência: antes)
        conn.SendTranslated(new PacketWriter(SPapelTimes).I32(-1).I32(JpTranslator.PapelNoLimit));
        conn.SendTranslated(w);
        Log.Info($"{conn} big papel: código {r.Code}, {r.Prizes.Count} bolas, pang={p.Pang}");
        if (r.Code == 0) await RecordAsync(GameHandler.PapelCounters(r.Prizes));
    }

    /// <summary>
    /// Memorial: 0x17F u32 typeid da moeda -> 0x216 (moeda -1 e prêmios) + 0x264 u32 0, u32 n, n × (i32 tipo do raro, ou
    /// -1 na consolação, u32 typeid, u32 qtd); erro: 0x264 só com o u32 código.
    /// </summary>
    async Task MemorialAsync(int coin)
    {
        if (memorial == null) { conn.SendTranslated(new PacketWriter(SMemorial).U32(MemorialOff)); return; }
        var r = await memorial.PlayAsync(kr.Player, coin);
        if (r.Code != 0)
        {
            conn.SendTranslated(new PacketWriter(SMemorial).U32(r.Code));
            Log.Info($"{conn} memorial: moeda {coin:X8} recusada (0x{r.Code:X})");
            return;
        }
        conn.SendTranslated(ItemsChanged(r.Changes));
        var w = new PacketWriter(SMemorial).U32(0).U32((uint)r.Prizes.Count);
        foreach (var pz in r.Prizes) w.I32(pz.RareType).U32((uint)pz.TypeId).U32((uint)pz.Quantity);
        conn.SendTranslated(w);
        if (memorial.Data.MemorialCoins.TryGetValue(coin, out var mc))         // conquistas: moeda e raros
        {
            var bag = new Domain.Achievements.CounterBag().Add(mc.Kind == 2 ? Domain.Achievements.AchievementCounters.MemorialSpecial
                : Domain.Achievements.AchievementCounters.MemorialNormal);
            foreach (var pz in r.Prizes)
                if (pz.RareType >= 0) bag.Add(pz.RareType >= 3 ? Domain.Achievements.AchievementCounters.MemorialRareHigh
                    : Domain.Achievements.AchievementCounters.MemorialRare);
            await RecordAsync(bag);
        }
        Log.Info($"{conn} memorial: moeda {coin:X8} -> {(r.Prizes.Count > 0 ? $"{r.Prizes[0].TypeId:X8}x{r.Prizes[0].Quantity} (tipo {r.Prizes[0].RareType})" : "nada")}");
    }

    /// <summary>
    /// 0x216 (inventário mudou; SPEC-jp983-correio-social.md §1.3): u32 hora unix, u32 n, n × 50 (u8 2, u32 typeid,
    /// i32 id, u32 flag de tempo, i32 qtd antes, i32 qtd depois, i32 diferença, 25 bytes zerados).
    /// </summary>
    public static PacketWriter ItemsChanged(List<ItemChange> changes)
    {
        var w = new PacketWriter(SItemsChanged, 16 + changes.Count * 50)
            .U32((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds()).U32((uint)changes.Count);
        foreach (var c in changes)
            w.U8(2).U32((uint)c.TypeId).I32(c.Id).U32(0).I32(c.Before).I32(c.After).I32(c.After - c.Before).Zeros(25);
        return w;
    }
}
