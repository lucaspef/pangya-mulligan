using Pangya.Core.Config;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Tests;

public class RingsTests
{
    const int Curve = 0x70000004, Luck = 0x70000005, Legendary = 0x70000084;

    [Fact]
    public void HourRingsKeepTheRemainderBetweenGames()
    {
        var ring = new Item { TypeId = Curve, Quantity = 5 };
        Assert.Equal(0, Rings.Spend(ring, 2400, 18));
        Assert.Equal(1, Rings.Spend(ring, 1200, 18));
        Assert.Equal(0, Rings.Spend(ring, 3599, 18));
        Assert.Equal(1, Rings.Spend(ring, 1, 18));
    }

    [Fact]
    public void LuckRingSpendsPerHoleAndLegendaryNeverSpends()
    {
        Assert.Equal(18, Rings.Spend(new Item { TypeId = Luck }, 7200, 18));
        Assert.Equal(0, Rings.Spend(new Item { TypeId = Legendary }, 7200, 18));
    }

    [Fact]
    public void DropUpRaisesTheChanceOfTheHoleDrop()
    {
        const int tid = 0x18000004;
        var cfg = new HoleDropsConfig
        {
            Enabled = true, ManaPerMille = 0, SpecialShufflePerMille = 0,
            Items = [new() { TypeId = tid, PerMille = 500, Quantity = 1 }],
        };
        int Count(int up)
        {
            var rng = new Random(7);
            int n = 0;
            for (int i = 0; i < 20000; i++)
                n += HoleDrops.Roll(cfg, new Player(), 0, 1, 1, 18, false, rng, dropUp: up).Count(d => d.TypeId == tid);
            return n;
        }
        Assert.InRange(Count(100), 9500, 10500);
        Assert.InRange(Count(120), 11500, 12500);
    }

    [Fact]
    public void TreasurePointsFollowTheRing() => Assert.Equal(18, TreasureHunter.WithRing(15, 120));
}
