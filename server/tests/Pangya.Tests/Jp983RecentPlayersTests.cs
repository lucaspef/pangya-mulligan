using Pangya.Core.Net;
using Pangya.Core.Text;

namespace Pangya.Tests;

/// <summary>Últimos jogadores do perfil (JP 0x9C -> 0x10E).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task RecentPlayersListTheLastOnesFirst()
    {
        await using var env = new JpEnv();
        var (_, b) = await env.NewAccountAsync();
        var (_, c) = await env.NewAccountAsync();
        var (g, a) = await InChannelAsync(env);
        await using var _ = g;
        await env.S.RecentPlayers.AddAsync(a.Id, [b.Id]);
        await Task.Delay(20);
        await env.S.RecentPlayers.AddAsync(a.Id, [c.Id]);

        await g.SendAsync(new PacketWriter(0x9C));
        var r = await g.ExpectAsync(0x10E);
        Assert.Equal(5 * 0x34, r.Remaining);
        r.U32();
        var nick = Cp949.Read(r.Bytes(22));
        Assert.Equal(c.Login, Cp949.Read(r.Bytes(22)));
        Assert.Equal((uint)c.Id, r.U32());
        Assert.Equal((await env.S.Accounts.FindByIdAsync(c.Id))!.Nickname, nick);
        r.U32(); r.Bytes(44);
        Assert.Equal((uint)b.Id, r.U32());
        r.Skip(3 * 0x34);
    }
}
