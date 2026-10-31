using Pangya.Core.Net;

namespace Pangya.Tests;

/// <summary>Início da partida no JP: semente do efeito de mascote (0x16A) e taxa de pang (0x77 = KR 0x75).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task GameStartSendsMascotSeedAndRate()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)1, (byte)19, (byte)3, (byte)0, (byte)0,
            0u, PracticeName, "12345678", 0u));
        await g.ExpectAsync(0x49);
        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        // taxa antes dos jogadores: o 0x77 zera os sUserInfo da partida no cliente (depois do 0x76 apagava o taco)
        Assert.Equal(100u, (await g.ExpectAsync(0x77)).U32());
        await g.ExpectAsync(0x76);
        var seed = await g.ExpectAsync(0x16A);
        Assert.Equal(4, seed.Remaining);
    }
}
