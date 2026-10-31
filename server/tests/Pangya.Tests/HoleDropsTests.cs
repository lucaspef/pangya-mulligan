using Pangya.Core.Config;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Tests;

/// <summary>Drop por buraco (SPEC-jp983-eventos-drops.md §3): itens da config, ticket do GP no último buraco, chapéu +1.</summary>
public class HoleDropsTests
{
    [Fact]
    public void DropsFollowTheConfig()
    {
        var cfg = new HoleDropsConfig { ManaPerMille = 0, SpecialShufflePerMille = 0, Items = [new() { TypeId = 0x1A0000AC, PerMille = 1000 }] };
        var p = new Player();
        var first = HoleDrops.Roll(cfg, p, 3, 1, 1, 18, false, new Random(1));
        Assert.Equal(new HoleDrop(0x1A0000AC, 3, 1, 1), Assert.Single(first));                  // não é o último: sem ticket
        var last = HoleDrops.Roll(cfg, p, 3, 18, 18, 18, false, new Random(1));
        Assert.Contains(new HoleDrop(HoleDrops.GrandPrixTicket, 3, 18, 2), last);                  // 18 buracos: 2 tickets
        Assert.DoesNotContain(last, d => d.TypeId == HoleDrops.GrandPrixTicket && HoleDrops.Roll(cfg, p, 3, 18, 18, 18, true, new Random(1)).Contains(d));
        p.Items[1] = new Item { Id = 1, TypeId = HoleDrops.GrandPrixTicket, Quantity = 49 };
        Assert.Contains(new HoleDrop(HoleDrops.GrandPrixTicket, 3, 18, 1), HoleDrops.Roll(cfg, p, 3, 18, 18, 18, false, new Random(1)));   // teto 50
        Assert.Contains(new HoleDrop(0x1A0000AC, 3, 1, 2), HoleDrops.Roll(cfg, p, 3, 1, 1, 18, false, new Random(1),
            Artifacts.RainbowMagicHat));                                                            // chapéu da sala: +1
        Assert.Contains(new HoleDrop(HoleDrops.PangTid, 3, 18, 3, HoleDrop.PangPouch500),           // Rednose Wizberry: 3 × 500 no 18º
            HoleDrops.Roll(cfg, p, 3, 18, 18, 18, false, new Random(1), Artifacts.RednoseWizberry));
        Assert.DoesNotContain(HoleDrops.Roll(cfg, p, 3, 9, 9, 9, false, new Random(1), Artifacts.RednoseWizberry),
            d => d.Kind == HoleDrop.PangPouch500);                                                   // só em partida de 18
        Assert.Empty(HoleDrops.Roll(new HoleDropsConfig { Enabled = false }, p, 3, 1, 1, 18, false, new Random(1)));
    }

    [Fact]
    public void HarderCoursesAddBetterItems()
    {
        var cfg = new HoleDropsConfig
        {
            ManaPerMille = 0, SpecialShufflePerMille = 0,
            Items = [new() { TypeId = 0x1A0000AC, PerMille = 1000 }, new() { TypeId = 0x1A0000AA, PerMille = 1000, MinStars = 4 }],
        };
        var p = new Player();
        Assert.DoesNotContain(HoleDrops.Roll(cfg, p, 0, 1, 1, 18, false, new Random(1), stars: 3), d => d.TypeId == 0x1A0000AA);
        var hard = HoleDrops.Roll(cfg, p, 3, 1, 1, 18, false, new Random(1), stars: 5);
        Assert.Contains(hard, d => d.TypeId == 0x1A0000AA);
        Assert.Contains(hard, d => d.TypeId == 0x1A0000AC);                                         // os comuns também
        Assert.Contains(new HoleDropsConfig().Items, i => i.MinStars >= 4);                         // o padrão tem a faixa difícil
    }

    [Fact]
    public void ArtifactEffects()
    {
        var rng = new Random(7);
        for (int i = 0; i < 200; i++)
        {
            Assert.InRange(Artifacts.Wind(Artifacts.OrchidBlossom, rng), 0, 7);                   // 1–8 m
            Assert.Equal(1, (Artifacts.Wind(Artifacts.PenneAbacus, rng) + 1) % 2);                // metros ímpares
            Assert.Equal(0, (Artifacts.Wind(Artifacts.TitanWindmill, rng) + 1) % 2);              // metros pares
            Assert.InRange(Artifacts.Wind(0, rng), 0, 8);
        }
        Assert.Equal((2, 10, 0), (Artifacts.ExpPercent(Artifacts.LuminescentCoral), Artifacts.ExpPercent(Artifacts.SilviaManual), Artifacts.ExpPercent(0)));
        Assert.Equal(200, Artifacts.RainPerMille(Artifacts.DragonOrb));
        Assert.True(Artifacts.KeepsItems(Artifacts.FrozenFlame));
        var p = new Player();
        Assert.False(Artifacts.CanUse(p, Artifacts.SilviaManual));                                  // sem o item
        p.Items[1] = new Item { Id = 1, TypeId = Artifacts.SilviaManual, Quantity = 1 };
        Assert.False(Artifacts.CanUse(p, Artifacts.SilviaManual));                                  // sem a mana
        p.Items[2] = new Item { Id = 2, TypeId = Artifacts.ManaOf(Artifacts.SilviaManual), Quantity = 1 };
        Assert.True(Artifacts.CanUse(p, Artifacts.SilviaManual));
        Assert.False(Artifacts.CanUse(p, 0x18000004));                                              // não é artefato
    }
}
