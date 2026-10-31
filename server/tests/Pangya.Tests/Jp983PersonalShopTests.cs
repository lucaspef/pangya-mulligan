using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Loja pessoal do lounge no JP 983: a compra (0xEC) chega ao comprador e ao vendedor no formato do JP.</summary>
public partial class Jp983GameTests
{
    static async Task<(TestClient G, Domain.Accounts.Account Acc)> InChannelLevelAsync(JpEnv env, long pang, params (int Tid, int Qty)[] items)
    {
        var (login, acc) = await env.NewAccountAsync();
        var ch = new PlayerChanges { Pang = pang, Level = 5 };                       // loja exige nível
        var ids = await env.S.Players.NewIdsAsync(Math.Max(items.Length, 1));
        for (int i = 0; i < items.Length; i++) ch.Added.Add(new Item { Id = ids[i], TypeId = items[i].Tid, Quantity = items[i].Qty });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, await env.S.Sessions.IssueGameLoginAsync(acc.Id)));
        await g.ExpectAsync(0x96);
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        await g.ExpectAsync(0x4E);
        return (g, acc);
    }

    /// <summary>sTradeItem do KR (170) no formato do JP (172: u16 a mais no 107).</summary>
    static byte[] JpTrade(sTradeItem t)
    {
        var k = Kr(t);
        return [.. k[..KrToJp.TradeItemGap], 0, 0, .. k[KrToJp.TradeItemGap..]];
    }

    [Fact]
    public async Task PersonalShopPurchaseReachesBuyerAndSellerInTheJpFormat()
    {
        await using var env = new JpEnv();
        var data = env.Game.Context.Kr.Data;
        int tid = 0;
        for (int t = 0x18000000; t < 0x18000200 && tid == 0; t++)
            if (data.Exists(t) && data.CanTrade(t)) tid = t;
        Assert.NotEqual(0, tid);
        var (a, accA) = await InChannelLevelAsync(env, 1000, (tid, 10));
        var (b, _) = await InChannelLevelAsync(env, 500);
        await using var _a = a;
        await using var _b = b;
        int stack = (await env.Game.Context.Kr.Players.LoadAsync(accA.Id))!.FindType(tid)!.Id;

        await a.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 0u, (byte)30, (byte)2, (byte)1, (byte)0, (byte)0, 0u,
            "lounge", "", 0u));
        var enter = await a.ExpectAsync(0x49);
        enter.I16();
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        await b.SendAsync(new PacketWriter(0x09).U16((ushort)room.Index).Str(""));
        await b.ExpectAsync(0x49);

        await a.SendAsync(new PacketWriter(0x76));
        await a.SendAsync(new PacketWriter(0x79).Str("Loja"));
        await a.SendAsync(new PacketWriter(0x7C).U32(1).Bytes(JpTrade(new sTradeItem { dwTid = (uint)tid, dwGuid = (uint)stack, iNum = 4, i64Price = 100 })));
        Assert.Equal(1u, (await a.ExpectAsync(0xEB)).U32());                           // publicada (0xE9 do KR)

        await b.SendAsync(new PacketWriter(0x77).U32((uint)accA.Id));                  // entra na loja
        Assert.Equal(1u, (await b.ExpectAsync(0xE6)).U32());                           // 0xE4 do KR
        await b.SendAsync(new PacketWriter(0x7D).U32((uint)accA.Id).Bytes(JpTrade(new sTradeItem { iIndex = 0, dwTid = (uint)tid, dwGuid = (uint)stack, iNum = 2 })));
        var bought = await b.ExpectAsync(0xEC);                                        // comprador
        Assert.Equal(1u, bought.U32());
        Assert.Equal((0, 300UL), (bought.U8(), bought.U64()));
        bought.Skip(KrToJp.TradeItemSize);
        Assert.Equal(2, bought.U8());                                                  // objeto novo
        var item = bought.Bytes(JpStructs.ItemSize).ToArray();
        Assert.Equal((uint)tid, BitConverter.ToUInt32(item, 4));
        Assert.Equal(0, bought.Remaining);
        var sold = await a.ExpectAsync(0xEC);                                          // vendedor
        Assert.Equal((1u, 1, 200UL), (sold.U32(), sold.U8(), sold.U64()));
        sold.Skip(KrToJp.TradeItemSize);
        Assert.Equal(0, sold.Remaining);

        await b.SendAsync(new PacketWriter(0x7D).U32((uint)accA.Id).Bytes(JpTrade(new sTradeItem { iIndex = 0, iNum = 9 })));
        var refused = await b.ExpectAsync(0xEC);                                       // recusa: só o código
        Assert.NotEqual(1u, refused.U32());
        Assert.Equal(0, refused.Remaining);
    }
}
