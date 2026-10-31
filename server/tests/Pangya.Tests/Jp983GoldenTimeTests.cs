using Pangya.Core.Net;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Golden Time (SPEC-jp983-eventos-drops.md §1): avisos 0x1D3, sorteio entre quem está no lounge/jogando, carta.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task GoldenTimeRoundPicksTheLoungePlayer()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        Assert.Equal(0, await env.Game.GoldenTime.RoundAsync(random: new Random(1)));     // no lobby: ninguém concorre
        Assert.Equal(12u, (await g.ExpectAsync(0x1D3)).Let(r => { r.U32(); return r.U32(); }));
        Assert.Equal(18u, (await g.ExpectAsync(0x1D3)).Let(r => { r.U32(); return r.U32(); }));

        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 0u, (byte)10, (byte)2, (byte)1, (byte)0, (byte)0,
            0u, "lounge", "", 0u));                                                    // lounge (tipo 2)
        await g.ExpectAsync(0x49);
        Assert.Equal(1, await env.Game.GoldenTime.RoundAsync(random: new Random(1)));
        PacketReader w;
        do { w = await g.ExpectAsync(0x1D3); w.U32(); } while (w.U32() != 14);
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        Assert.Contains($"<NICKNAME>{h.Player.Nickname}</NICKNAME>", w.Str(1024));
        await g.ExpectAsync(0x4B);                                                     // fogos no lounge
        var (mails, _) = await env.S.Mail.ListAsync(acc.Id, 1, 20);
        Assert.Single(mails);
    }
}
