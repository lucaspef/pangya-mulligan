using Pangya.Core.Net;
using Pangya.Domain.Shop;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Caixa do My Room no JP 983 (0xB2 -> 0x216 + 0x129).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task MyRoomBoxGivesOnePrizeAndCountsDown()
    {
        await using var env = new JpEnv();
        const int bag = 0x1A000004;                                                      // Lucky Bag of Fortune
        Assert.Contains(bag, JpBoxes.All);
        var (g, acc) = await InChannelWithAsync(env, 100, (bag, 2), (0x18000004, 1));
        await using var _ = g;

        await g.SendAsync(new PacketWriter(0xB2).U32(0x18000004));                      // não é caixa: recusa
        var refused = await g.ExpectAsync(0x129);
        Assert.NotEqual(0, refused.U8());

        await g.SendAsync(new PacketWriter(0xB2).U32(bag));
        var r = await g.ExpectAsync(0x129);
        Assert.Equal((0, (uint)bag, 1, 1u), (r.U8(), r.U32(), r.I32(), r.U32()));
        int tid = (int)r.U32(), id = r.I32(), qty = r.I32();
        r.Skip(8);
        Assert.Equal(0, r.Remaining);
        Assert.True(qty > 0);
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal(1, p.FindType(bag)!.Quantity);
        if (tid == BoxService.PangTid) Assert.Equal(100 + qty, p.Pang);
        else Assert.Equal(tid, p.Items[id].TypeId);
    }
}
