using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Grand Zodiac de evento (SPEC-jp983-grand-zodiac.md §1.2, §3): sala do servidor, contagem, pontos GZ e troféus.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public void ZodiacEventRules()
    {
        Assert.Equal((9, 8, 1), (ZodiacPoints.Cup(0), ZodiacPoints.Cup(300), ZodiacPoints.Cup(10_000)));
        Assert.Equal(0, GrandZodiacGame.TrophyFor(1, 19));                                  // menos de 20: nenhum
        Assert.Equal((0x2D0A6400, 0), (GrandZodiacGame.TrophyFor(1, 25), GrandZodiacGame.TrophyFor(2, 25)));
        Assert.Equal((0x2D0A6300, 0x2D0A6400), (GrandZodiacGame.TrophyFor(1, 30), GrandZodiacGame.TrophyFor(2, 30)));
        Assert.Equal((0x2D0A6200, 0x2D0A6300, 0x2D0A6400), (GrandZodiacGame.TrophyFor(1, 50), GrandZodiacGame.TrophyFor(2, 50),
            GrandZodiacGame.TrophyFor(3, 50)));
        Assert.Equal((ZodiacKind.Intermediate, ZodiacKind.Advanced), (JpZodiacEvent.Parse("int"), JpZodiacEvent.Parse("adv")));
    }

    [Fact]
    public async Task GmCreatesTheZodiacEventRoomLikeANormalRoom()
    {
        await using var env = new JpEnv();
        env.Game.Zodiac.StartWait = TimeSpan.FromHours(1);
        PacketWriter Make(byte type) => Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 1_800_000u, (byte)100, type, (byte)18,
            (byte)0, (byte)0, 0u, "evento", "", 0u);
        var (g0, _) = await InChannelAsync(env);                                     // jogador comum: recusada
        await using (g0)
        {
            await g0.SendAsync(Make(11));
            Assert.Equal((byte)JoinResult.NotFound, (await g0.ExpectAsync(0x49)).U8());
            Assert.Empty(env.Game.World.Rooms.Rooms);
        }

        var (login, acc) = await env.NewAccountAsync();
        await env.S.Accounts.SetIdentityFlagsAsync(acc.Id, 0x04);
        var g = await env.ConnectGameAsync();
        await using var _ = g;
        await g.SendAsync(GameLogin(login, acc.Id, await env.S.Sessions.IssueGameLoginAsync(acc.Id)));
        await g.ExpectAsync(0x96);
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        await g.ExpectAsync(0x4E);
        await g.SendAsync(Make(13));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var info = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal((11, 13), (info[88], info[196]));                                // exibição GZ, avançado
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        Assert.Equal((ZodiacKind.Advanced, "evento", (byte)100, acc.Id), (room.Settings.Zodiac, room.Settings.Title, room.Settings.MaxPlayers, room.OwnerId));
        Assert.Contains(room.Players, rp => rp.Guid == (uint)acc.Id);
        await g.SendAsync(new PacketWriter(0x0E).U32(0));                             // o GM (dono) começa pelo botão
        await g.ExpectAsync(0x1F9);
        Assert.IsType<GrandZodiacGame>(room.Game);
    }

    [Fact]
    public async Task ZodiacEventRoomStartsByItselfAndGivesPoints()
    {
        await using var env = new JpEnv();
        env.Game.Zodiac.StartWait = TimeSpan.Zero;
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        int index = env.Game.Zodiac.Open(ZodiacKind.Intermediate);
        var room = env.Game.World.Rooms.Get(index)!;
        Assert.Equal((false, 0x40, GrandZodiacGame.EventTimeMs), (room.Settings.Hidden, (int)room.Settings.Course, room.Settings.GameTimeMs));

        await g.SendAsync(new PacketWriter(0x09).U16((ushort)index).Str(""));          // entra pela lista
        var info = (await g.ExpectAsync(0x49)).Bytes(2 + KrToJp.RoomSize).ToArray();
        Assert.Equal((11, 11), (info[2 + 88], info[2 + 196]));
        var count = await g.ExpectAsync(0x40);                                         // contagem de 10 s
        Assert.Equal(11, count.U8());
        var cup = await g.ExpectAsync(0x1F9, 20_000);                                  // começou: copo 9 (0 pontos)
        Assert.Equal((9, 9), (cup.I32(), cup.I32()));
        await g.ExpectAsync(0x76);
        var game = Assert.IsType<GrandZodiacGame>(room.Game);
        await g.SendAsync(new PacketWriter(0x11).U8(100));
        await g.ExpectAsync(0x201);
        await g.SendAsync(new PacketWriter(0x137));
        await g.ExpectAsync(0x1F4);
        lock (env.Game.World.Rooms.Sync)
        {
            var mp = game.Players[0];
            game.Shoot(mp, 0);
            game.Result(mp, new ShotResult((uint)acc.Id, 0, 0, 0, 0, 10, 0, Display: 0x100));
            game.ShotFinished(mp);
        }
        await g.ExpectAsync(0x1EF);
        await g.SendAsync(new PacketWriter(0x131));                                    // sai antes do fim
        var st = await g.ExpectAsync(0x1F3);
        Assert.Equal((2u, 1u), (st.U32(), st.U32()));
        st.Skip(4 + 4);
        int total = st.I32();
        st.Skip(4 + 4);
        Assert.Equal((uint)(total * 3.5), st.U32());                                    // pontos GZ = total × 3,5
        for (int i = 0; i < 50 && await env.S.ZodiacPoints.GetAsync(acc.Id) == 0; i++) await Task.Delay(20);
        Assert.Equal((int)(total * 3.5), await env.S.ZodiacPoints.GetAsync(acc.Id));
    }
}
