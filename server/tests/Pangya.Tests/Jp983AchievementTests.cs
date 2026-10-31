using Pangya.Core.Net;
using Pangya.Domain.Achievements;
using Pangya.Domain.Players;

namespace Pangya.Tests;

/// <summary>Conquistas do JP 983 (SPEC-jp983-conquistas.md): tabelas, regra das etapas e os pacotes.</summary>
public partial class Jp983GameTests
{
    sealed class MemoryAchievements : IAchievementStore
    {
        public readonly AchievementBook Saved = new();
        public Task<AchievementBook> LoadAsync(long account) => Task.FromResult(new AchievementBook());
        public Task SaveAsync(long account, IReadOnlyList<(int Achievement, int Slot, long Value)> counters,
            IReadOnlyList<(int Quest, ClearedQuest Cleared)> cleared)
        {
            foreach (var (a, s, v) in counters) Saved.Counters[(a, s)] = v;
            foreach (var (q, c) in cleared) Saved.Cleared[q] = c;
            return Task.CompletedTask;
        }
    }

    [Fact]
    public void AchievementTablesComeFromTheJpData()
    {
        if (Jp983DataTests.Data is not { } d) return;
        Assert.Equal(201, d.Achievements.Count);
        Assert.True(d.AchievementQuests.Count >= 1541);
        Assert.True(d.IsCounter(AchievementCounters.NormalGame));
        Assert.False(d.IsCounter(0x6C4000C5));                                         // Mystic Ruins: não existe
        AchievementDef first = d.Achievements[0];
        Assert.Equal((0x4C000001, 0x748002BC), (first.TypeId, first.BaseQuest));
        Assert.Equal(0x748002BC, first.Quests[^1]);                                     // a base é a última
        var q = d.AchievementQuests[0x748002C4];
        Assert.Equal((AchievementCounters.Tourney, 10), q.Needs[0]);
        Assert.Equal((AchievementService.PointsTypeId, 10), q.Rewards[0]);
    }

    [Fact]
    public async Task TenFinishedTourneysCloseTheFirstLevel()
    {
        if (Jp983DataTests.Data is not { } d) return;
        var store = new MemoryAchievements();
        var service = new AchievementService(d, store);
        var p = new Player { AccountId = 1 };
        var book = new AchievementBook();
        CounterBag Game(bool finished)
        {
            var bag = new CounterBag().Add(AchievementCounters.Tourney).Add(AchievementCounters.NormalGame);
            return finished ? bag.Add(AchievementCounters.GameComplete) : bag;
        }
        var r = await service.RecordAsync(p, book, Game(finished: false));               // não terminou: não conta
        Assert.Equal(0, book.Counter(0, 0));
        for (int i = 0; i < 9; i++) r = await service.RecordAsync(p, book, Game(finished: true));
        Assert.Equal(9, book.Counter(0, 0));
        Assert.False(book.Cleared.ContainsKey(0x748002C4));
        r = await service.RecordAsync(p, book, Game(finished: true));
        Assert.Contains((0x4C000001, 0x748002C4), r.ClosedQuests);
        Assert.True(book.Cleared.ContainsKey(0x748002C4));
        Assert.True(r.Points >= 10);
        Assert.Equal(10, store.Saved.Counter(0x4C000001, 0));
        Assert.Equal(0, AchievementService.MemorialLevel(r.Points));
        Assert.Equal(1, AchievementService.MemorialLevel(301));
        Assert.Equal(24, AchievementService.MemorialLevel(99_999));
    }

    [Fact]
    public async Task AchievementPacketsGoAtLoginWindowAndProgress()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        await env.S.Players.ApplyAsync(acc.Id, new PlayerChanges { Pang = 100_000 });
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        var counters = await g.ExpectAsync(0x21D);                                    // contadores, em pedaços de 20
        Assert.Equal((0, 20u, 20u), (counters.I32(), counters.U32(), counters.U32()));
        Assert.Equal(20 * 13, counters.Remaining);
        var infos = await g.ExpectAsync(0x21E);
        Assert.Equal((0, 20u, 20u), (infos.I32(), infos.U32(), infos.U32()));
        Assert.Equal((1, 0x4C000001u, 1), (infos.U8(), infos.U32(), infos.I32()));
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        await g.ExpectAsync(0x4E);

        await g.SendAsync(new PacketWriter(0x157).U32((uint)acc.Id));                  // janela: 0x22D em pedaços + 0x22C
        int total = 0;
        while (true)
        {
            var (id, r) = await g.ReceiveAsync();
            if (id == 0x22C) { Assert.Equal(0, r.I32()); break; }
            if (id != 0x22D) continue;
            Assert.Equal(0, r.I32());
            total += (int)r.U32();
        }
        Assert.Equal(201, total);

        await g.SendAsync(new PacketWriter(0x14B));                                   // Papel: o contador anda
        var seen = new List<ushort>();
        PacketReader? counter = null;
        while (seen.Count < 40 && !seen.Contains(0x220))
        {
            var (id, r) = await g.ReceiveAsync();
            seen.Add(id);
            if (id == 0x216 && counter == null) counter = r;
        }
        Assert.Contains((ushort)0x21B, seen);                                         // resultado do Papel antes
        Assert.Contains((ushort)0x220, seen);                                         // a conquista mudou
        counter!.Skip(4);
        Assert.Equal(1u, counter.U32());
        Assert.Equal((2, (uint)AchievementCounters.PapelPlay), (counter.U8(), counter.U32()));
        counter.Skip(4 + 4);
        Assert.Equal((0, 1), (counter.I32(), counter.I32()));
    }
}
