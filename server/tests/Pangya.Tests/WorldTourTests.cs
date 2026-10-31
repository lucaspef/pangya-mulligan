using Pangya.Core.Config;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Tests;

/// <summary>World Tour (SPEC-jp983-world-tour.md): bit por campo, 18 buracos de stroke/torneio, prêmio uma vez com os 16.</summary>
public class WorldTourTests
{
    sealed class MemStore : IWorldTourStore
    {
        public readonly Dictionary<long, WorldTourProgress> Rows = [];
        public Task<WorldTourProgress?> GetAsync(long a) => Task.FromResult(Rows.TryGetValue(a, out var r) ? r : null);
        public Task SaveAsync(long a, WorldTourProgress p) { Rows[a] = p; return Task.CompletedTask; }
    }

    static readonly DateOnly Day = new(2026, 10, 12);

    [Fact]
    public void BitsFollowTheClientMap()
    {
        Assert.Equal(1, WorldTourService.Bit(0));
        Assert.Equal(0x800, WorldTourService.Bit(11));
        Assert.Equal(0, WorldTourService.Bit(12));                                     // campo 12 não está no mapa
        Assert.Equal(0x1000, WorldTourService.Bit(13));
        Assert.Equal(0x8000, WorldTourService.Bit(16));
        Assert.Equal(0, WorldTourService.Bit(19));
        int all = 0;
        for (byte c = 0; c <= 20; c++) all |= WorldTourService.Bit(c);
        Assert.Equal(WorldTourService.AllMaps, all);
    }

    [Fact]
    public async Task OnlyFullRoundsCountAndTheGiftIsGivenOnce()
    {
        var store = new MemStore();
        var svc = new WorldTourService(store, null, new WorldTourConfig());
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Stroke, false, 3, 9, true, Day));        // 9 buracos
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Stroke, true, 3, 18, true, Day));        // prática
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Match, false, 3, 18, true, Day));        // match
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Stroke, false, 3, 18, false, Day));     // saiu antes
        Assert.Equal(8, (await svc.OnGameEndAsync(1, GameMode.Tournament, false, 3, 18, true, Day))!.MapFlag);
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Stroke, false, 3, 18, true, Day));       // já tinha
        Assert.False(await svc.ClaimAsync(1, Day));                                               // faltam campos
        for (byte c = 0; c <= 16; c++) await svc.OnGameEndAsync(1, GameMode.Stroke, false, c, 18, true, Day);
        Assert.Equal(new WorldTourProgress(0xFFFF, false), store.Rows[1]);
        Assert.True(await svc.ClaimAsync(1, Day));
        Assert.False(await svc.ClaimAsync(1, Day));                                               // uma vez só
        Assert.True(store.Rows[1].GotGift);
    }

    [Fact]
    public async Task ClosedEventCountsNothing()
    {
        var svc = new WorldTourService(new MemStore(), null, new WorldTourConfig { End = new DateOnly(2020, 1, 1) });
        Assert.False(svc.Open(Day));
        Assert.Null(await svc.OnGameEndAsync(1, GameMode.Stroke, false, 3, 18, true, Day));
    }
}
