using Pangya.Core.Config;
using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.JP983.Ranking;
using Pangya.Ranking;

namespace Pangya.Tests;

/// <summary>Ranking do JP 983 (SPEC-jp983-correio-social.md §5): endereço pelo game, hello, página e ficha no layout do JP.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task JpRankingTalksTheJpLayout()
    {
        await using var env = new JpEnv();
        var (loginA, a) = await env.NewAccountAsync();
        var (_, b) = await env.NewAccountAsync();
        await env.S.Players.ApplyAsync(a.Id, new PlayerChanges { Stats = new PlayerStats { Hole = 36, TotalScore = 9 } });
        await env.S.Players.ApplyAsync(b.Id, new PlayerChanges { Stats = new PlayerStats { Hole = 18, TotalScore = -3 } });
        var rank = new RankingServer(env.S, env.Game.World, new RankingConfig { Id = 39475 }, JpRanking.ServerType, "RANK-JP",
            JpRanking.Connect, 0, Pangya.Domain.Ranking.RankingTables.Jp983);
        using var cts = new CancellationTokenSource();
        _ = await rank.StartAsync(cts.Token);
        env.Game.Context.Kr.Ranking = rank.Context;

        var key = await env.S.Sessions.IssueGameLoginAsync(a.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(loginA, a.Id, key));
        await g.ExpectAsync(0x96);
        await g.SendAsync(new PacketWriter(0x47));                                    // botão Ranking -> 0xA2 (KR 0xA0)
        var addr = await g.ExpectAsync(0xA2);
        Assert.Equal(("127.0.0.1", (uint)rank.Tcp.Port), (addr.Str(), addr.U32()));

        await using var r = await TestClient.ConnectAsync(rank.Tcp.Port);
        var (id, hello) = await r.ReceiveAsync();
        Assert.Equal(0x1388, id);
        r.Key = (int)hello.U32();
        Assert.Equal(5, hello.U8());
        Assert.Matches(@"^\d{4}-\d\d-\d\d", hello.Str());

        await r.SendAsync(new PacketWriter(0x00).U32((uint)a.Id).Str(loginA).U8(0).U8(1).U8(0).U8(0).U32(0).U8(1));
        await r.ExpectAsync(0x1389);                                                  // página (KR 0x1F5)

        string nickB = (await env.Game.Context.Kr.Players.LoadAsync(b.Id))!.Nickname;
        await r.SendAsync(new PacketWriter(0x01).U32((uint)b.Id).Str(nickB).U8(0));  // ficha (KR 0x1F6): personagem 513
        var card = await r.ExpectAsync(0x138A);
        Assert.Equal(0, card.U8());
        Assert.Equal(4 + 22 + 22 + 2 + JpStructs.CharacterSize + 1 + 6 * 12, card.Remaining);   // 6 linhas: + pontos de conquista
        Assert.Equal((uint)b.Id, card.U32());
        card.Skip(22);
        Assert.Equal(nickB, Cp949.Read(card.Bytes(22)));
        card.Skip(2);
        Assert.Equal(0x04000000u, card.U32());                                        // personagem (typeid) no começo dos 513
        cts.Cancel();
    }

    /// <summary>Ranking por curso no JP: 20 cursos na ordem do 983 (Wiz City é o 17, não o 16 como no KR).</summary>
    [Fact]
    public void JpCourseRankingFollowsTheJpOrder()
    {
        var p = new Player { AccountId = 1, Nickname = "a", Level = 5 };
        p.Courses[19] = new CourseRecord { BestScore = -3, Games = 1, Holes = 18 };      // Wiz City
        var jp = Pangya.Domain.Ranking.RankingService.Build([p], null, DateTime.UtcNow, Pangya.Domain.Ranking.RankingTables.Jp983);
        Assert.Equal(69, Assert.Single(jp.Board(1, 17, 0)!.Entries).Value);
        Assert.Empty(jp.Board(1, 16, 0)!.Entries);                                      // 16 = Ice Inferno no JP
        Assert.NotNull(jp.Board(0, 5, 0));                                              // geral: 6ª linha (pontos de conquista)
        var kr = Pangya.Domain.Ranking.RankingService.Build([p], null, DateTime.UtcNow);
        Assert.Single(kr.Board(1, 16, 0)!.Entries);                                     // no KR o Wiz City é o 16
        Assert.Null(kr.Board(1, 17, 0));
    }
}
