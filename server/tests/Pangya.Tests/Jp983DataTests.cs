using Pangya.Domain.Players;
using Pangya.Protocol.JP983;

namespace Pangya.Tests;

/// <summary>
/// Dados do cliente JP (pangya_jp.iff) lidos por offset: valores conferidos no arquivo real (docs/server/protocolo/SPEC-jp983-iff.md).
/// O arquivo vem do pak do cliente (não está no repositório): sem ele, o teste não roda.
/// </summary>
public class Jp983DataTests
{
    internal static readonly Jp983GameData? Data = Jp983GameData.Load(Path.Combine(TestEnv.Root, "data", "pangya_jp.iff"));

    [Fact]
    public void ReadsTheJpCatalog()
    {
        if (Data is not { } d) return;
        Assert.True(d.Count > 14000);
        var hat = d.GetShopItem(0x08000800)!;                                   // Cowboy Hat: 1100 pang, à venda
        Assert.Equal((1100u, false, 1), (hat.Price, hat.IsCash, hat.InStock));
        Assert.Equal(10, d.GetShopItem(0x18000006)!.PackSize);                  // Silent Wind: pacote de 10
        Assert.Equal([1, 7, 15, 30, 0], d.GetShopItem(0x40000000)!.PeriodPrices); // mascote: preço por período
        Assert.True(d.Exists(0x14000004));                                      // Fro Comet
        Assert.True(d.Exists(0x0400000E));                                      // Cecilia R: personagem que só o JP tem
        Assert.Equal([0x0838C205, 0], d.SubParts(0x0838E00D));                  // Violet Shoes (CR) cobre as pernas
        Assert.Empty(d.SubParts(0x08000800));
    }

    [Fact]
    public void ReadsStatsUpgradesAndRecipes()
    {
        if (Data is not { } d) return;
        Assert.Equal(2100, d.UpgradePrice(0, 0));                               // Enchant 0x34000000
        Assert.Equal(69600, d.UpgradePrice(4, 30));                             // curva nível 30
        Assert.Equal(2056, d.MagicBox.Count);
        Assert.Equal(0x08000400, d.DefaultParts(0x04000000)[0]);               // cabeça básica da Nuri
        Assert.True(d.IsDefaultPart(0x04000000, 0x08000400));
        Assert.Contains((byte)0x14, d.Courses);                                 // Abbot Mine: mapa que só o JP tem
        Assert.Equal(1, d.CourseStars(0));                                      // Blue Lagoon: 1 estrela
        Assert.Equal(5, d.CourseStars(0x0D));                                   // Deep Inferno: 5
        Assert.Equal(3, d.Cards[0x7C000002].Rarity);

        // Nuri sem roupas: teto Attr [9,11,6,2,2] e nível base igual; força -15 no fim
        var p = new Player { Level = 1 };
        p.Add(new Item { Id = 1, TypeId = 0x04000000 });
        p.Equip = new Equipment { CharacterId = 1 };
        var (stats, _) = d.PlayStats(p);
        Assert.Equal([9 - 15, 11, 6, 2, 2], stats);
    }
}
