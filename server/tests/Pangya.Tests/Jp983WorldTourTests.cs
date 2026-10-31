using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>World Tour no JP (SPEC-jp983-world-tour.md): 0x11D no login, 0xAD pede o prêmio (vai por carta).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task WorldTourFlagsOnLoginAndGiftByMail()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        await env.S.WorldTour.SaveAsync(acc.Id, new WorldTourProgress(0xFFFF, false));
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        var wt = await g.ExpectAsync(0x11D);
        Assert.Equal((0xFFFFu, (byte)0), (wt.U32(), wt.U8()));

        await g.SendAsync(new PacketWriter(0xAD).U32((uint)acc.Id));                      // "sim" na janela do presente
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));            // entra no canal (o 0xAD já foi tratado)
        await g.ExpectAsync(0x4E);
        Assert.True((await env.S.WorldTour.GetAsync(acc.Id))!.GotGift);
        var (mail, _) = await env.S.Mail.ListAsync(acc.Id, 0, 20);
        Assert.Contains(mail, m => m.Message == "World Tour");
    }
}
