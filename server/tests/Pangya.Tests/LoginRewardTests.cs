using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Tests;

/// <summary>Prêmio de login por evento (SPEC-jp983-eventos-drops.md §5).</summary>
public class LoginRewardTests
{
    sealed class MemStore : ILoginRewardStore
    {
        public readonly Dictionary<(long, int), LoginRewardProgress> Rows = [];
        public Task<LoginRewardProgress?> GetAsync(long a, int e) => Task.FromResult(Rows.TryGetValue((a, e), out var r) ? r : null);
        public Task SaveAsync(long a, int e, LoginRewardProgress p) { Rows[(a, e)] = p; return Task.CompletedTask; }
    }

    [Fact]
    public async Task CountsNewDaysAndGivesOnTheLastOne()
    {
        var store = new MemStore();
        var events = new List<LoginRewardEvent>
        {
            new() { Id = 1, Name = "uma vez", Days = 3, Times = 1, TypeId = 0x1A000272 },
            new() { Id = 2, Name = "sempre", Days = 2, Forever = true, TypeId = 0x1A000011 },
            new() { Id = 3, Name = "acabou", Days = 1, TypeId = 0x1A000011, End = new DateOnly(2020, 1, 1) },
        };
        var svc = new LoginRewardService(store, null, events);
        var p = new Player { AccountId = 7 };
        var d = new DateOnly(2026, 10, 12);
        Assert.Empty(await svc.OnLoginAsync(p, d));
        Assert.Empty(await svc.OnLoginAsync(p, d));                                    // mesmo dia não conta
        Assert.Equal(["sempre"], await svc.OnLoginAsync(p, d.AddDays(1)));
        Assert.Equal(["uma vez"], await svc.OnLoginAsync(p, d.AddDays(2)));
        Assert.True(store.Rows[(7, 1)].Clear);
        Assert.Equal(["sempre"], await svc.OnLoginAsync(p, d.AddDays(3)));            // o "uma vez" acabou; o "sempre" continua
        Assert.False(store.Rows.ContainsKey((7, 3)));                                   // evento vencido
    }
}
