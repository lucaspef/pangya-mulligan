using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Point Shop antiga (0x126 abrir -> 0x1E7, 0x127 saldo -> 0x1E8, 0x128 itens -> pontos -> 0x216 + 0x1E9, 0x129 pontos ->
/// itens -> 0x216 + 0x1EA) e Tiki Shop novo (0x18D -> 0xC8 + 0x216 + 0x274 u32 0, u32 milhagem, u32 bônus).
/// docs/server/protocolo/SPEC-jp983-missoes.md §2.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CPointShopOpen = 0x126, CPointShopBalance = 0x127, CPointShopSell = 0x128, CPointShopBuy = 0x129, CTikiShop = 0x18D;
    const ushort SPointShopOpen = 0x1E7, SPointShopBalance = 0x1E8, SPointShopSell = 0x1E9, SPointShopBuy = 0x1EA, STikiShop = 0x274;

    async Task<bool> HandleTikiAsync(PacketReader r)
    {
        var p = kr.Player;
        if (tiki == null)
        {
            r.Skip(r.Remaining);
            ushort id = r.Id switch { CPointShopOpen => SPointShopOpen, CPointShopBalance => SPointShopBalance, CPointShopSell => SPointShopSell,
                CPointShopBuy => SPointShopBuy, _ => STikiShop };
            conn.SendTranslated(r.Id == CPointShopOpen ? new PacketWriter(id).U32(0) : new PacketWriter(id).U32(r.Id == CPointShopBalance ? 0u : 1u).U32(0));
            return true;
        }
        switch (r.Id)
        {
            case CPointShopOpen:
                r.Skip(r.Remaining);
                conn.SendTranslated(new PacketWriter(SPointShopOpen).U32(0));
                return true;
            case CPointShopBalance:
                r.Skip(r.Remaining);
                conn.SendTranslated(new PacketWriter(SPointShopBalance).U32(0).U32((uint)await tiki.PointsAsync(p)));
                return true;
            case CPointShopSell:
            {
                int n = r.Remaining >= 1 ? r.U8() : 0;
                var items = new List<(int, int, int)>(n);
                for (int i = 0; i < n && r.Remaining >= 16; i++) { int tid = (int)r.U32(), id = r.I32(), lots = r.I32(); r.U32(); items.Add((tid, id, lots)); }
                r.Skip(r.Remaining);
                Answer(SPointShopSell, await tiki.ItemsToPointsAsync(p, items), w => { });
                return true;
            }
            case CPointShopBuy:
            {
                int n = r.Remaining >= 1 ? r.U8() : 0;
                var items = new List<(int, int)>(n);
                for (int i = 0; i < n && r.Remaining >= 12; i++) { int tid = (int)r.U32(), lots = r.I32(); r.U32(); items.Add((tid, lots)); }
                r.Skip(r.Remaining);
                Answer(SPointShopBuy, await tiki.PointsToItemsAsync(p, items), w => { });
                return true;
            }
            default:
            {
                int n = r.Remaining >= 4 ? (int)r.U32() : 0;
                var items = new List<(int, int, int)>(Math.Min(n, 5));
                for (int i = 0; i < n && r.Remaining >= 12; i++) items.Add(((int)r.U32(), r.I32(), (int)r.U32()));
                r.Skip(r.Remaining);
                var res = await tiki.ExchangeAsync(p, items);
                if (res.Code == 0) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64((ulong)res.PangSpent));
                Answer(STikiShop, res, w => w.U32((uint)res.Mileage).U32((uint)res.Bonus));
                if (res.Code == 0)
                {
                    var bag = new Domain.Achievements.CounterBag().Add(0x6C4000C2, res.TikiGained);
                    if (res.Bonus > 0) bag.Add(0x6C4000C1);
                    await RecordAsync(bag);
                }
                return true;
            }
        }
    }

    /// <summary>0x216 com as mudanças e a resposta (u32 0 + saldo, ou o que write escrever); recusa: u32 código.</summary>
    void Answer(ushort id, TikiResult res, Action<PacketWriter> write)
    {
        if (res.Code != 0)
        {
            conn.SendTranslated(new PacketWriter(id).U32(res.Code));
            Log.Info($"{conn} JP: troca Tiki 0x{id:X} recusada (0x{res.Code:X})");
            return;
        }
        if (res.Changes.Count > 0) conn.SendTranslated(ItemsChanged(res.Changes));
        var w = new PacketWriter(id).U32(0);
        if (id == STikiShop) write(w); else w.U32((uint)res.Points);
        conn.SendTranslated(w);
        Log.Info($"{conn} JP: troca Tiki 0x{id:X} ok (pontos {res.Points}, milhagem {res.Mileage}, TP +{res.TikiGained})");
    }
}
