using Pangya.Core.Net;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Point Shop antiga e Tiki Shop novo do JP 983 (SPEC-jp983-missoes.md §2).</summary>
public partial class Jp983GameTests
{
    const int LemonDrop = 0x1A0000E4;

    [Fact]
    public async Task PointShopTradesItemsForPointsAndBack()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var ball = FindTiki(data, 35, 1);                                                // uma bola: 35 = 1 ponto
        Assert.NotEqual(0, ball);
        Assert.Equal((10, 3), data.PointShop[LemonDrop]);
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 1000, (ball, 400));
        await using var _ = g;

        await g.SendAsync(new PacketWriter(0x126));
        Assert.Equal(0u, (await g.ExpectAsync(0x1E7)).U32());                             // 0 = a janela abre
        await g.SendAsync(new PacketWriter(0x127));
        var bal = await g.ExpectAsync(0x1E8);
        Assert.Equal((0u, 0u), (bal.U32(), bal.U32()));

        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        int ballId = h.Player.FindType(ball)!.Id;
        await g.SendAsync(new PacketWriter(0x128).U8(1).U32((uint)ball).I32(ballId).I32(11).U32(0));   // 11 lotes = 385 bolas
        await g.ExpectAsync(0x216);
        var sell = await g.ExpectAsync(0x1E9);
        Assert.Equal((0u, 11u), (sell.U32(), sell.U32()));
        Assert.Equal(15, h.Player.FindType(ball)!.Quantity);

        await g.SendAsync(new PacketWriter(0x129).U8(1).U32(LemonDrop).I32(1).U32(10));
        await g.ExpectAsync(0x216);
        var buy = await g.ExpectAsync(0x1EA);
        Assert.Equal((0u, 1u), (buy.U32(), buy.U32()));
        Assert.Equal(3, h.Player.FindType(LemonDrop)!.Quantity);
        await g.SendAsync(new PacketWriter(0x129).U8(1).U32(LemonDrop).I32(1).U32(10));   // sem pontos
        Assert.NotEqual(0u, (await g.ExpectAsync(0x1EA)).U32());
        Assert.Equal(1, await env.S.Tiki.PointsAsync(acc.Id));
    }

    [Fact]
    public async Task TikiShopTurnsMileageIntoTikiPoints()
    {
        if (Jp983DataTests.Data is not { } data) return;
        int ball = FindTiki(data, 35, 1);
        var t = data.Tiki(ball)!;
        int qty = (TikiService.MileagePerPoint + t.Mileage - 1) / t.Mileage;              // o bastante para 1 TP
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 1000, (ball, qty));
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        await g.SendAsync(new PacketWriter(0x18D).U32(1).U32((uint)ball).I32(h.Player.FindType(ball)!.Id).U32((uint)qty));
        Assert.Equal((ulong)(1000 - t.Pang), (await g.ExpectAsync(0xC8)).U64());
        await g.ExpectAsync(0x216);
        var r = await g.ExpectAsync(0x274);
        Assert.Equal((0u, (uint)(qty * t.Mileage)), (r.U32(), r.U32()));
        Assert.True(h.Player.FindType(TikiService.TikiPointsTypeId)!.Quantity >= 1);
        Assert.Null(h.Player.FindType(ball));
    }

    static int FindTiki(Pangya.Protocol.JP983.Jp983GameData d, int perLot, int tp)
    {
        foreach (var tid in new[] { 0x14000001, 0x14000002, 0x14000003, 0x14000004, 0x14000005, 0x14000006, 0x14000007, 0x14000008 })
            if (d.Tiki(tid) is { Tradeable: true } t && t.PerLot == perLot && t.TpPerLot == tp) return tid;
        for (int i = 0; i < 0x400; i++)
            if (d.Tiki(0x14000000 + i) is { Tradeable: true } t && t.PerLot == perLot && t.TpPerLot == tp && t.Kind == 2) return 0x14000000 + i;
        return 0;
    }
}
