using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Intrusão no JP (SPEC-jp983-intrusao-vip-modos.md §1): entrar num torneio já começado.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task LateComerJoinsARunningTournament()
    {
        await using var env = new JpEnv();
        var (a, accA) = await InChannelAsync(env);
        await using var _a = a;
        var (b, accB) = await InChannelAsync(env);
        await using var _b = b;
        await a.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)30, (byte)4, (byte)3, (byte)0, (byte)0,
            0u, "torneio", "", 0u));
        await a.ExpectAsync(0x49);
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        await a.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        await a.ExpectAsync(0x52);
        Assert.True(((TourneyGame)room.Game!).IntrusionOpen);

        await b.SendAsync(new PacketWriter(0x9D).U8(0).U16((ushort)room.Index));        // duplo clique: confirmação
        var confirm = await b.ExpectAsync(0x113);
        Assert.Equal((3, 0, (ushort)room.Index), (confirm.U8(), confirm.U8(), confirm.U16()));
        confirm.Skip(8);
        Assert.Equal(KrToJp.RoomSize, confirm.Remaining);

        await b.SendAsync(new PacketWriter(0x9D).U8(1).U16((ushort)room.Index));        // "sim": entra
        var enter = await b.ExpectAsync(0x113);
        Assert.Equal((4, 0, 2 + KrToJp.RoomSize), (enter.U8(), enter.U8(), enter.Remaining + 2));
        var players = await b.ExpectAsync(0x113);
        Assert.Equal((4, 1, (byte)2), (players.U8(), players.U8(), players.U8()));
        Assert.Equal(2 * KrToJp.SlotSize, players.Remaining);
        await b.ExpectAsync(0x113);                                                      // {4,2}
        await b.ExpectAsync(0x113);                                                      // {4,3}
        var start = await b.ExpectAsync(0x113);
        Assert.Equal((4, 4), (start.U8(), start.U8()));
        Assert.Equal(4 + 12 + 18 * 6 + 4 + 18, start.Remaining);
        var joined = await a.ExpectAsync(0x113);                                         // quem já jogava: alguém entrou
        Assert.Equal((7, 0), (joined.U8(), joined.U8()));
        Assert.Equal(2, room.Players.Count);

        await b.SendAsync(new PacketWriter(0x11).U8(100));                               // carregou: o placar do rival e {9}
        var score = await b.ExpectAsync(0x113);
        Assert.Equal((10, 0, (uint)accA.Id), (score.U8(), score.U8(), score.U32()));
        PacketReader nine;
        do nine = await b.ExpectAsync(0x113); while (nine.U8() != 9);
        Assert.Equal((0, (uint)accB.Id, 2u), (nine.U8(), nine.U32(), nine.U32()));

        await b.SendAsync(new PacketWriter(0x9D).U8(0).U16((ushort)room.Index));        // já jogou nesta partida: 6
        Assert.Equal((6, 5), (await b.ExpectAsync(0x113)).Let(r => (r.U8(), r.U8())));  // (já está numa sala: 5)
    }
}
