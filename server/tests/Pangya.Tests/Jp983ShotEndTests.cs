using Pangya.Core.Net;

namespace Pangya.Tests;

/// <summary>Fim da tacada (JP 0x12F -> 0x1F7 a toda a sala), para a trajetória dos outros nos modos em massa.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task ShotEndIsRelayedToTheRoomWithGuidAndHole()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)1, (byte)19, (byte)3, (byte)0, (byte)0,
            0u, PracticeName, "12345678", 0u));
        await g.ExpectAsync(0x49);
        await g.SendAsync(new PacketWriter(0x12F).Bytes(new byte[0x57]));          // fora da partida: nada
        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        await g.ExpectAsync(0x76);

        var data = new byte[0x57];
        for (int i = 0; i < data.Length; i++) data[i] = (byte)i;
        await g.SendAsync(new PacketWriter(0x12F).Bytes(data));
        var r = await g.ExpectAsync(0x1F7);
        Assert.Equal((uint)acc.Id, r.U32());
        Assert.True(r.U8() >= 1);
        Assert.Equal(data, r.Bytes(0x57).ToArray());
        Assert.Equal(0, r.Remaining);
    }
}
