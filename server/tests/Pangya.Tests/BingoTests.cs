using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Tests;

/// <summary>Bingo do lobby: cartela do cliente (uid % 10), linhas em 12 bits, fichas por partida e sorteio sem repetir.</summary>
public class BingoTests
{
    sealed class MemStore : IBingoStore
    {
        public readonly Dictionary<long, BingoProgress> Rows = [];
        public Task<BingoProgress?> GetAsync(long a) => Task.FromResult(Rows.TryGetValue(a, out var r) ? r : null);
        public Task SaveAsync(long a, BingoProgress p) { Rows[a] = p; return Task.CompletedTask; }
    }

    static readonly DateOnly Day = new(2026, 11, 4);

    static int Bits(params int[] numbers) => numbers.Aggregate(0, (m, n) => m | 1 << (n - 1));

    [Fact]
    public void LinesFollowTheBoardOfTheUid()
    {
        Assert.Equal(1, BingoService.LinesOf(10, Bits(13, 2, 10, 25, 1)));          // cartela 0, fileira 0
        Assert.Equal(1 << 5, BingoService.LinesOf(0, Bits(13, 12, 19, 17, 9)));     // coluna 0
        Assert.Equal(1 << 10, BingoService.LinesOf(0, Bits(13, 8, 15, 21, 3)));     // diagonal principal
        Assert.Equal(1 << 11, BingoService.LinesOf(0, Bits(1, 5, 15, 7, 9)));       // a outra
        Assert.Equal(0, BingoService.LinesOf(1, Bits(13, 2, 10, 25, 1)));           // outra cartela
        Assert.Equal(BingoService.AllLines, BingoService.LinesOf(7, (1 << 25) - 1));
    }

    [Fact]
    public async Task DrawsEveryNumberOnceAndSpendsTries()
    {
        var store = new MemStore();
        var svc = new BingoService(store, null, new BingoConfig { StartTries = 25 }, new Random(5));
        var seen = new HashSet<int>();
        int lines = 0;
        for (int i = 0; i < 25; i++)
        {
            var d = await svc.DrawAsync(3, Day);
            Assert.Equal(0, d.Code);
            Assert.True(seen.Add(d.Number));
            Assert.Equal(0, d.NewLines & lines);
            lines |= d.NewLines;
        }
        Assert.Equal(BingoService.AllLines, lines);
        Assert.Equal(new BingoProgress(0, (1 << 25) - 1, BingoService.AllLines), store.Rows[3]);
        Assert.Equal(BingoService.NoTries, (await svc.DrawAsync(3, Day)).Code);
    }

    [Fact]
    public async Task TriesComeFromFinishedGames()
    {
        var svc = new BingoService(new MemStore(), null, new BingoConfig { StartTries = 0 });
        Assert.Equal(BingoService.NoTries, (await svc.DrawAsync(1, Day)).Code);
        Assert.Null(await svc.OnGameEndAsync(1, true, 18, true, Day));                    // prática
        Assert.Null(await svc.OnGameEndAsync(1, false, 18, false, Day));                  // saiu antes
        Assert.Equal(1, (await svc.OnGameEndAsync(1, false, 3, true, Day))!.Tries);
        Assert.Equal(0, (await svc.DrawAsync(1, Day)).Code);
        Assert.Equal(2, svc.Prizes(0b101)[0].Quantity);                                  // um prêmio por linha
    }
}
