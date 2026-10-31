using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Troféus do My Room e tipo real da sala, nos formatos que o 983 lê (taskmanager.cpp 0xB4, 0x169, 0x1FA, 0x25D).</summary>
public partial class Jp983GameTests
{
    const int SpecialTid = 0x2D0A3B00, GpTid = 0x2F010101;

    static void AssertTrophy(PacketReader r, int tid, int qty) => Assert.Equal(((uint)tid, (uint)tid, (uint)qty), (r.U32(), r.U32(), r.U32()));

    [Fact]
    public async Task LoginSendsTheTrophiesAfterThePlayerData()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var stats = (await env.S.Players.LoadAsync(acc.Id))!.Stats;
        Trophy.Add(stats.Trophies, 0x2C060000, Trophy.Silver);
        await env.S.Players.ApplyAsync(acc.Id, new PlayerChanges { Stats = stats });
        await env.S.SpecialTrophies.AddAsync(acc.Id, SpecialTid, 2);
        await env.S.SpecialTrophies.AddAsync(acc.Id, GpTid, 3);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));

        await g.ExpectAsync(0x44);                                                      // u8 0xD3, u8 0
        var data = await g.ExpectAsync(0x44);
        data.U8();
        data.Str();
        data.Skip(JpStructs.MemberInfoSize + 4 + JpStructs.UserInfoSize + (6 * 3 + 1) * 2);
        Assert.Equal(1, data.U16());                                                    // Trophy[6][1] dentro do 0x44

        var normal = await g.ExpectAsync(0x169);                                        // tudo depois do 0x44, que apaga as listas
        Assert.Equal((5, JpStructs.TrophySize), (normal.U8(), normal.Remaining));
        normal.Skip((6 * 3 + 1) * 2);
        Assert.Equal(1, normal.U16());
        var special = await g.ExpectAsync(0xB4);
        Assert.Equal((5, 0, 1), (special.U8(), special.U8(), special.U8()));
        AssertTrophy(special, SpecialTid, 2);
        var gp = await g.ExpectAsync(0x25D);
        Assert.Equal((5, 1u, 1u), (gp.U8(), gp.U32(), gp.U32()));
        AssertTrophy(gp, GpTid, 3);
    }

    /// <summary>Durante a sessão: 0x1FA com o guid do login (senão o cliente cria outra linha) e 0x216 para o GP.</summary>
    [Fact]
    public async Task TrophyGivenInSessionReachesTheClient()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        h.TrophyGiven!(SpecialTid, 2);
        for (int i = 0; i < 2; i++)
        {
            var won = await g.ExpectAsync(0x1FA);
            Assert.Equal(((uint)SpecialTid, (uint)SpecialTid), (won.U32(), won.U32()));
        }
        h.TrophyGiven(GpTid, 1);
        var item = await g.ExpectAsync(0x216);
        item.U32();
        Assert.Equal((1u, 2), (item.U32(), (int)item.U8()));                            // kind 2 = quantidade
        Assert.Equal(((uint)GpTid, (uint)GpTid), (item.U32(), item.U32()));             // uid 0: o cliente recusa (0x1e8557)
        item.U32();
        Assert.Equal((0, 1, 1), (item.I32(), item.I32(), item.I32()));                  // ele soma a diferença
    }

    [Theory]
    [InlineData(0, 0xFF)]      // Stroke
    [InlineData(1, 0xFF)]      // Match
    [InlineData(4, 0xFF)]      // torneio
    [InlineData(10, 0xFF)]
    [InlineData(11, 11)]       // especiais ficam com o tipo
    [InlineData(14, 18)]       // Special Shuffle
    public void RealTypeIsNoneOutsideTheSpecialRooms(byte krMode, int real) => Assert.Equal(real, KrToJp.RealType(krMode));

    /// <summary>Sala comum com 0xFF no +196: é o que o 983 00479bc0 exige para aceitar artefato.</summary>
    [Fact]
    public async Task NormalRoomGoesWithoutRealType()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 0u, (byte)4, (byte)0, (byte)18, (byte)0, (byte)0,
            0u, "comum", "", 0u));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var room = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal((0, KrToJp.NoRealType), (room[88], room[196]));
    }
}
