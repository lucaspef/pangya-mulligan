using Pangya.Core.Net;
using Pangya.Domain.Rooms;

namespace Pangya.Tests;

/// <summary>Bot GM Event (SPEC-jp983-eventos-drops.md §2): aviso 0x1D3 tipo 20, sala de torneio do servidor, começa sozinha.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task BotGmEventOpensATournamentThatStartsByItself()
    {
        await using var env = new JpEnv();
        env.Game.BotGm.StartWait = TimeSpan.Zero;
        var (g, _) = await InChannelAsync(env);
        await using var _g = g;
        int index = env.Game.BotGm.Open(new Random(1));
        var notice = await g.ExpectAsync(0x1D3);
        Assert.Equal((1u, 20u), (notice.U32(), notice.U32()));
        Assert.Contains("Bot GM Event", notice.Str(1024));
        var room = env.Game.World.Rooms.Get(index)!;
        Assert.Equal((GameMode.Tournament, (byte)18, (byte)3), (room.Settings.Mode, room.Settings.Holes, room.Settings.HoleType));
        await g.SendAsync(new PacketWriter(0x09).U16((ushort)index).Str(""));
        await g.ExpectAsync(0x49);
        Assert.Equal(11, (await g.ExpectAsync(0x40)).U8());                            // contagem
        await g.ExpectAsync(0x76, 20_000);                                              // começou
        Assert.IsAssignableFrom<MassGame>(room.Game);
    }
}
