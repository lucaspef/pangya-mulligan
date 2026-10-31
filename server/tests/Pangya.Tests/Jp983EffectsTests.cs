using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Pedidos da sala/partida (SPEC-jp983-efeitos.md): Assist, Auto Command, troca de servidor, buff, recarga, intrusão.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task AssistAutoCommandAndServerChange()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (JpSystems.AutoCommanderTypeId, 2));
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;

        await g.SendAsync(new PacketWriter(0x184));                                      // liga: o item aparece antes do 0x26A
        var on = await g.ExpectAsync(0x216);
        on.Skip(8 + 1);
        Assert.Equal((uint)JpSystems.AssistTypeId, on.U32());
        Assert.Equal(0u, (await g.ExpectAsync(0x26A)).U32());
        Assert.NotNull(h.Player.FindType(JpSystems.AssistTypeId));
        await g.SendAsync(new PacketWriter(0x185).U32(JpSystems.AssistTypeId));
        Assert.Equal(0u, (await g.ExpectAsync(0x26B)).U32());
        await g.SendAsync(new PacketWriter(0x184));                                      // desliga
        await g.ExpectAsync(0x216);
        await g.ExpectAsync(0x26A);
        Assert.Null(h.Player.FindType(JpSystems.AssistTypeId));

        await g.SendAsync(new PacketWriter(0x156));                                      // auto command: sem resposta, gasta 1
        await g.SendAsync(new PacketWriter(0x156));
        await g.SendAsync(new PacketWriter(0x156));                                      // acabou: 0x22B
        Assert.Equal(1u, (await g.ExpectAsync(0x22B)).U32());
        Assert.Null(h.Player.FindType(JpSystems.AutoCommanderTypeId));

        await g.SendAsync(new PacketWriter(0x119).U32(99999));                           // servidor que não existe
        Assert.Equal(1u, (await g.ExpectAsync(0x1D4)).U32());
        await g.SendAsync(new PacketWriter(0x9D).U8(0).U16(1));                          // intrusão: recusa
        var intr = await g.ExpectAsync(0x113);
        Assert.Equal((6, 5), (intr.U8(), intr.U8()));
    }

    [Fact]
    public async Task TimedBuffAndCometRefill()
    {
        if (Jp983DataTests.Data is not { } data) return;
        Assert.Equal((100, 100, 60), data.Buff(0x1A0000E1) is { } rainbow ? (rainbow.ExpPercent, rainbow.PangPercent, rainbow.Minutes) : default);
        Assert.Equal((50, 120), data.Buff(ItemBuffService.GoldenSweetYam) is { } gy ? (gy.ExpPercent, gy.Minutes) : default);
        await using var env = new JpEnv();
        const int ball = 0x14000001;
        var (g, acc) = await InChannelWithAsync(env, 0, (ItemBuffService.GoldenSweetYam, 1), (ItemBuffService.SweetYam, 1),
            (ItemBuffService.RefillPack, 1), (ball, 5));
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;

        await g.SendAsync(new PacketWriter(0xD8).U32(ItemBuffService.GoldenSweetYam));
        var used = await g.ExpectAsync(0x181);
        Assert.Equal((2u, 1u, (uint)ItemBuffService.GoldenSweetYam), (used.U32(), used.U32(), used.U32()));
        Assert.Equal(65, used.Remaining);
        await g.SendAsync(new PacketWriter(0xD8).U32(ItemBuffService.SweetYam));         // não anda junto com o dourado
        var refused = await g.ExpectAsync(0x181);
        Assert.Equal((2u, 0u), (refused.U32(), refused.U32()));
        Assert.Equal((50, 0), env.Game.Context.Kr.Buffs.Rates(h.Player, DateTime.UtcNow));

        await g.SendAsync(new PacketWriter(0xEC).U32(ItemBuffService.RefillPack).U32(ball));
        var rf = await g.ExpectAsync(0x197);
        Assert.Equal(((byte)1, (uint)ItemBuffService.RefillPack, (uint)ball), (rf.U8(), rf.U32(), rf.U32()));
        Assert.InRange(rf.U32(), 5u + 30, 5u + 50);
        Assert.Null(h.Player.FindType(ItemBuffService.RefillPack));
    }

    [Fact]
    public async Task TickerCostsACookieAndReachesEveryone()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        await g.SendAsync(new PacketWriter(0x66).Str("ola"));                           // sem cookies: 0x50 código 4
        Assert.Equal(4u, (await g.ExpectAsync(0x50)).U32());
        h.Player.Cookie = 5;
        await g.SendAsync(new PacketWriter(0x66).Str("ola a todos"));
        Assert.Equal(4ul, (await g.ExpectAsync(0x96)).U64());
        await g.SendAsync(new PacketWriter(0x67));
        await g.ExpectAsync(0xCA);
        var t = await g.ExpectAsync(0xC9, 10_000);                                      // passa no letreiro (KR 0xC7)
        Assert.Equal((h.Player.Nickname, "ola a todos"), (t.Str(), t.Str()));
    }
}
