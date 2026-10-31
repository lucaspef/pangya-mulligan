using Pangya.Core.Net;
using Pangya.Domain.Shop;

namespace Pangya.Tests;

/// <summary>Respostas pequenas que o cliente espera: fita de replay (0xA2) e denúncia (0x92).</summary>
[Collection("db")]
public class ReplyTests(DbFixture fx)
{
    [Fact]
    public async Task ReplayTapeAndReportAnswer()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        await new ShopService(env.Players.Store, env.Data).GiveAsync(p, 0x1A000000, 2);
        await using var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        await c.ExpectAsync(0x94);

        await c.SendAsync(new PacketWriter(0x4A).U32(0x1A000000));                 // replay salvo: gasta uma fita
        var tape = await c.ExpectAsync(0xA2);
        Assert.Equal(((ushort)1, 0x1A000000u), (tape.U16(), tape.U32()));
        await c.SendAsync(new PacketWriter(0x4A).U32(0x18000004));                 // não é fita: só a resposta
        Assert.Equal((ushort)0, (await c.ExpectAsync(0xA2)).U16());

        await c.SendAsync(new PacketWriter(0x3A));
        Assert.Equal(0, (await c.ExpectAsync(0x92)).U8());
    }
}
