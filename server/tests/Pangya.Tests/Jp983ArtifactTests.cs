using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Artefato da sala no JP 983: só com o item e a mana; aparece no RoomInfo (+197).</summary>
public partial class Jp983GameTests
{
    static PacketWriter MakeStroke(uint artifact) => Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)4, (byte)0,
        (byte)18, (byte)0, (byte)0, 0u, "art", "", artifact);

    [Fact]
    public async Task RoomArtifactNeedsTheItemAndTheMana()
    {
        await using var env = new JpEnv();
        var (g0, _) = await InChannelWithAsync(env, 0, (Artifacts.SilviaManual, 1));        // sem a mana: sala sem artefato
        await using (g0)
        {
            await g0.SendAsync(MakeStroke((uint)Artifacts.SilviaManual));
            await g0.ExpectAsync(0x49);
            Assert.Equal(0, Assert.Single(env.Game.World.Rooms.Rooms).Settings.Artifact);
        }

        var (g, _) = await InChannelWithAsync(env, 0, (Artifacts.SilviaManual, 1), (Artifacts.ManaOf(Artifacts.SilviaManual), 2));
        await using var _g = g;
        await g.SendAsync(MakeStroke((uint)Artifacts.SilviaManual));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var info = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal((uint)Artifacts.SilviaManual, BitConverter.ToUInt32(info, 197));
        Room? room = null;
        foreach (var r in env.Game.World.Rooms.Rooms) if (r.Settings.Title == "art" && r.Players.Count > 0) room = r;
        Assert.Equal(Artifacts.SilviaManual, room!.Settings.Artifact);
    }
}
