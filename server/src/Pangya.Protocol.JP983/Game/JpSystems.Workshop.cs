using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Oficina de club set (docs/server/protocolo/SPEC-jp983-oficina.md §5): cada pedido manda o 0x216 (itens gastos tipo 2 e os
/// tacos tipo 0xCC com o bloco da oficina) e a sua resposta (u32 código, ...). A maestria das partidas chega pelo fim de jogo.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CWsLevel = 0x164, CWsLevelOk = 0x165, CWsLevelCancel = 0x166, CWsRank = 0x167, CWsTransformYes = 0x168,
        CWsTransformNo = 0x169, CWsRecover = 0x16B, CWsTransfer = 0x16C, CWsReset = 0x16D, CWsEvent = 0x172, CWsEventCount = 0x173;
    const ushort SWsLevel = 0x23D, SWsLevelOk = 0x23E, SWsLevelCancel = 0x23F, SWsRank = 0x240, SWsOffer = 0x241, SWsTransformed = 0x242,
        SWsKept = 0x243, SWsTransfer = 0x245, SWsRecover = 0x246, SWsReset = 0x247, SWsEventCount = 0x24B, SWsEvent = 0x24E;
    /// <summary>Tipo do item no 0x216 com o bloco da oficina, e o de stats (upgrades com pang).</summary>
    const byte ItemWorkshop = 0xCC, ItemStats = 0xC9;

    ClubWorkshopService? workshop;
    readonly WorkshopSession wsSession = new();

    ClubWorkshopService? Workshop => workshop ??= ctx.Data is IClubWorkshopData d ? new ClubWorkshopService(ctx.Players.Store, d, ctx.Data) : null;

    async Task<bool> HandleWorkshopAsync(PacketReader r)
    {
        if (r.Id is CWsEvent or CWsEventCount)
        {
            r.Skip(r.Remaining);
            conn.SendTranslated(r.Id == CWsEvent
                ? new PacketWriter(SWsEvent).U32(0).I32(0).I32(3000).I32(0).U8(100).U8(0).U8(10).U8(10)
                : new PacketWriter(SWsEventCount).U32(0));
            return true;
        }
        if (Workshop is not { } ws)
        {
            r.Skip(r.Remaining);
            conn.SendTranslated(new PacketWriter(ResponseOf(r.Id)).U32(1).U32(0).U32(0));
            return true;
        }
        var p = kr.Player;
        WorkshopResult res;
        switch (r.Id)
        {
            case CWsLevel: res = await ws.UpLevelAsync(p, wsSession, (int)r.U32(), r.U16(), r.I32()); break;
            case CWsLevelOk: res = ws.ConfirmLevel(p, wsSession); break;
            case CWsLevelCancel: res = await ws.CancelLevelAsync(p, wsSession); break;
            case CWsRank: res = await ws.UpRankAsync(p, wsSession, (int)r.U32(), r.U16(), r.I32()); break;
            case CWsTransformYes or CWsTransformNo:
                res = await ws.TransformAsync(p, wsSession, r.Id == CWsTransformYes, CreateClubAsync);
                break;
            case CWsRecover: res = await ws.RecoverAsync(p, (int)r.U32(), r.I32()); break;
            case CWsTransfer: res = await ws.TransferAsync(p, (int)r.U32(), r.I32(), r.I32(), (int)r.U32()); break;
            case CWsReset: res = await ws.ResetAsync(p, (int)r.U32(), r.I32()); break;
            default: return false;
        }
        r.Skip(r.Remaining);
        SendWorkshop(r.Id, res);
        return true;
    }

    async Task<Item?> CreateClubAsync(int typeId)
    {
        var (code, granted) = await ctx.Shop.GiveAsync(kr.Player, typeId, 1);
        return code == ShopCode.Ok && granted.Count > 0 ? kr.Player.Find(granted[0].Id) : null;
    }

    static ushort ResponseOf(int request) => request switch
    {
        CWsLevel => SWsLevel, CWsLevelOk => SWsLevelOk, CWsLevelCancel => SWsLevelCancel, CWsRank => SWsRank,
        CWsTransformYes => SWsTransformed, CWsTransformNo => SWsKept, CWsRecover => SWsRecover, CWsTransfer => SWsTransfer, _ => SWsReset,
    };

    void SendWorkshop(int request, WorkshopResult res)
    {
        ushort id = ResponseOf(request);
        if (res.Code != 0)
        {
            conn.SendTranslated(new PacketWriter(id).U32(res.Code).U32(0).U32(0));
            Log.Info($"{conn} JP: oficina 0x{request:X} recusada (0x{res.Code:X})");
            return;
        }
        var p = kr.Player;
        if (res.PangBack > 0) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64((ulong)res.PangBack));
        var spent = new List<ItemChange>(res.Spent);
        if (res.Removed is { } old) spent.Add(new ItemChange(old.TypeId, old.Id, 1, 0));
        if (request == CWsLevel) conn.SendTranslated(ItemsChanged(spent));                   // nível: só o material (o taco no 0x165)
        else if (spent.Count > 0 || res.Clubs.Count > 0)
            conn.SendTranslated(ItemsChanged(spent, res.Clubs, statsToo: request == CWsReset));
        var w = new PacketWriter(res.Offer ? SWsOffer : id).U32(0);
        if (res.Offer) w = new PacketWriter(SWsOffer);                                         // "transformar?" vem vazio
        else switch (request)
        {
            case CWsLevel: w.U32((uint)res.Stat); break;
            case CWsLevelOk or CWsRank or CWsTransformNo: w.U32((uint)res.Stat).I32(res.ClubId); break;
            case CWsLevelCancel: w.I32(res.ClubId); break;
            case CWsTransformYes or CWsReset: w.U32((uint)res.TypeId).I32(res.ClubId); break;
        }
        conn.SendTranslated(w);
        Log.Info($"{conn} JP: oficina 0x{request:X} ok (stat {res.Stat}, taco {res.ClubId}{(res.Offer ? $", oferta {res.TypeId:X8}" : "")})");
    }

    /// <summary>Fim de partida: maestria no taco usado (holes × por buraco) e o 0x216 com o bloco da oficina.</summary>
    public async Task ClubMasteryAsync(Domain.Achievements.AchievementCounters.GameEnd g)
    {
        if (Workshop is not { } ws) return;
        try
        {
            var p = kr.Player;
            if (await ws.AddMasteryAsync(p, p.Equip.ClubSetId, g.Holes, g.Mode, g.Shuffle, g.Zodiac, ctx.World.Config.Rewards.ClubMasteryRate) is { } club)
            {
                conn.SendTranslated(ItemsChanged([], [club]));
                Log.Info($"{conn} JP: maestria do taco {club.TypeId:X8} = {ClubWorkshop.Mastery(club)}");
            }
        }
        catch (Exception e) { Log.Error($"{conn} falha na maestria do taco", e); }
    }

    /// <summary>
    /// 0x216 com tacos: os itens gastos (tipo 2) e cada taco tipo 0xCC (50 B + i16[5] ws.c, u32 maestria, u8 nível, u32 rank,
    /// u32 recuperação); statsToo (reset) manda o taco também como 0xC9 com os upgrades com pang (i16[5] em +25).
    /// </summary>
    PacketWriter ItemsChanged(List<ItemChange> changes, IReadOnlyList<Item> clubs, bool statsToo = false)
    {
        int n = changes.Count + clubs.Count * (statsToo ? 2 : 1);
        var w = new PacketWriter(SItemsChanged, 16 + n * 73).U32((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds()).U32((uint)n);
        foreach (var c in changes)
            w.U8(2).U32((uint)c.TypeId).I32(c.Id).U32(0).I32(c.Before).I32(c.After).I32(c.After - c.Before).Zeros(25);
        var data = Workshop?.Data;
        foreach (var club in clubs)
        {
            w.U8(ItemWorkshop).U32((uint)club.TypeId).I32(club.Id).U32(0).I32(0).I32(0).I32(0).Zeros(25);
            WorkshopBlock(w, club, data);
            if (!statsToo) continue;
            w.U8(ItemStats).U32((uint)club.TypeId).I32(club.Id).U32(0).I32(0).I32(0).I32(0);
            foreach (var v in club.IntArray("pcl", 5)) w.I16((short)v);
            w.Zeros(15);
        }
        return w;
    }

    /// <summary>Bloco da oficina do 0x216 (23 B): i16[5] ws.c, u32 maestria, u8 nível, u32 rank, u32 recuperação.</summary>
    static void WorkshopBlock(PacketWriter w, Item club, IClubWorkshopData? data)
    {
        foreach (var v in ClubWorkshop.Slots(club)) w.I16((short)v);
        int rank = -1, level = 0;
        if (data != null && data.ClubWorkshop.TryGetValue(club.TypeId, out var info) && info.Type >= 0)
        {
            rank = ClubWorkshop.Rank(info, club);
            level = ClubWorkshop.Level(info, club);
        }
        w.U32((uint)ClubWorkshop.Mastery(club)).U8((byte)level).I32(rank).U32((uint)ClubWorkshop.Recovery(club));
    }
}
