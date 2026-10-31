using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Domain.Shop;

/// <summary>
/// Moeda do Memorial Shop (MemorialShopCoinItem): Kind 0 normal, 1 premium, 2 especial, 3 personagem; Bonus &gt; 0 = mais
/// chance de raro (a Premium); faixa de "gacha" (0/0 = todas) e filtros (vazio = todos os raros).
/// </summary>
public sealed record MemorialCoin(int TypeId, int Kind, int Bonus, int GachaMin, int GachaMax, int[] Filters);

/// <summary>Prêmio raro (MemorialShopRareItem): peso, tipo 0..4 (como o cliente mostra), nº do gacha e filtros.</summary>
public sealed record MemorialRare(int TypeId, int Weight, int RareType, int Gacha, int[] Filters);

/// <summary>Tabelas do Memorial Shop (só clientes que o têm, como o JP 983).</summary>
public interface IMemorialData
{
    IReadOnlyDictionary<int, MemorialCoin> MemorialCoins { get; }
    IReadOnlyList<MemorialRare> MemorialRares { get; }
}

/// <summary>
/// Um prêmio do Memorial: RareType do raro (0..4: o cliente abre a janela "Rare items received"), ou -1 na consolação,
/// como a referência ("item normal": janela "You won the item", que este cliente mostra vazia porque não traz a
/// MemorialShopNormalItem.sff; com 0..4 o consumível apareceria como raro).
/// </summary>
public readonly record struct MemorialPrize(int RareType, int TypeId, int Quantity);

/// <summary>Item que mudou (o cliente JP atualiza o inventário por esta lista): quantidade antes e depois.</summary>
public readonly record struct ItemChange(int TypeId, int Id, int Before, int After);

/// <summary>Code 0 = ok; os outros são os do cliente (SPEC-jp983-papel-memorial.md §3.5, 16 bits baixos).</summary>
public sealed record MemorialResult(uint Code, List<MemorialPrize> Prizes, List<ItemChange> Changes)
{
    public const uint NoCoin = 0x301, NotAnItem = 0x302, NotOwned = 0x303, UnknownCoin = 0x304, NothingDrawn = 0x306;
    public static MemorialResult Fail(uint code) => new(code, [], []);
}

/// <summary>
/// Memorial Shop (cliente JP 983; docs/server/protocolo/SPEC-jp983-papel-memorial.md §3): gasta 1 moeda e sorteia 1 prêmio.
/// Raros: os da faixa e dos filtros da moeda, pelo peso; chance de raro RarePercent (PremiumRarePercent nas moedas com
/// bônus, como a Premium). Senão (ou sem raro possível, como as moedas de personagem), um prêmio de consolação da
/// config (o cliente não traz a tabela). Raro que o jogador já tem (e não empilha) é sorteado de novo.
/// </summary>
public sealed class MemorialService(ShopService shop, IMemorialData data, MemorialConfig cfg, Random? random = null)
{
    const int MaxRedraws = 10;
    readonly Random rng = random ?? Random.Shared;
    public IMemorialData Data => data;
    readonly Dictionary<int, List<MemorialRare>> pools = [];

    /// <summary>Raros que a moeda pode dar (faixa de gacha e filtros), montados uma vez.</summary>
    public List<MemorialRare> PoolOf(MemorialCoin coin)
    {
        lock (pools)
        {
            if (pools.TryGetValue(coin.TypeId, out var pool)) return pool;
            pool = [];
            foreach (var r in data.MemorialRares)
            {
                if ((coin.GachaMin != 0 || coin.GachaMax != 0) && (r.Gacha < coin.GachaMin || r.Gacha > coin.GachaMax)) continue;
                if (coin.Filters.Length > 0 && !Shares(coin.Filters, r.Filters)) continue;
                if (r.Weight > 0) pool.Add(r);
            }
            pools[coin.TypeId] = pool;
            return pool;
        }
    }

    static bool Shares(int[] a, int[] b)
    {
        foreach (var x in a)
            if (x != 0 && Array.IndexOf(b, x) >= 0) return true;
        return false;
    }

    public async Task<MemorialResult> PlayAsync(Player p, int coinTid)
    {
        if (coinTid == 0) return MemorialResult.Fail(MemorialResult.NoCoin);
        if (Item.GroupOf(coinTid) != ItemGroup.Usable) return MemorialResult.Fail(MemorialResult.NotAnItem);
        if (!data.MemorialCoins.TryGetValue(coinTid, out var coin)) return MemorialResult.Fail(MemorialResult.UnknownCoin);
        if (p.FindType(coinTid) is not { Location: ItemLocation.Inventory } owned || PlayerActions.Free(p, owned) <= 0)
            return MemorialResult.Fail(MemorialResult.NotOwned);

        var before = new Dictionary<int, int>();                              // typeid -> quantidade antes (para a lista)
        var d = new ShopDraft(p, shop.Store);
        d.Edit(owned).Quantity--;
        var granted = new List<Granted>();
        var prizes = new List<MemorialPrize>(1);
        var pool = PoolOf(coin);
        int rarePercent = coin.Bonus > 0 ? cfg.PremiumRarePercent : cfg.RarePercent;
        bool rare = pool.Count > 0 && rng.Next(100) < rarePercent;
        for (int attempt = 0; rare && attempt < MaxRedraws; attempt++)
        {
            var r = Pick(pool);
            Remember(p, before, r.TypeId);
            var code = await shop.GrantAsync(d, r.TypeId, 1, 0, granted, 0);
            if (code == ShopCode.AlreadyOwned) continue;
            if (code != ShopCode.Ok) break;
            prizes.Add(new MemorialPrize(r.RareType, r.TypeId, 1));
            break;
        }
        if (prizes.Count == 0)                                                // consolação
        {
            var options = coin.Bonus > 0 && cfg.Premium.Length > 0 ? cfg.Premium : cfg.Normal;
            if (PickConfig(options) is not { } c) return MemorialResult.Fail(MemorialResult.NothingDrawn);
            int qty = rng.Next(Math.Max(1, c.Min), Math.Max(c.Min, c.Max) + 1);
            Remember(p, before, c.TypeId);
            var code = await shop.GrantAsync(d, c.TypeId, qty, 0, granted, 0);
            if (code != ShopCode.Ok) return MemorialResult.Fail(MemorialResult.NothingDrawn);
            prizes.Add(new MemorialPrize(-1, c.TypeId, qty));
        }
        int coinBefore = owned.Quantity;
        await d.CommitAsync();

        var changes = new List<ItemChange>(granted.Count + 1) { new(coinTid, owned.Id, coinBefore, coinBefore - 1) };
        foreach (var g in granted)
        {
            int after = p.Items.TryGetValue(g.Id, out var it) ? it.Quantity : g.Count;
            changes.Add(new ItemChange(g.TypeId, g.Id, before.GetValueOrDefault(g.TypeId), after));
        }
        return new MemorialResult(0, prizes, changes);
    }

    static void Remember(Player p, Dictionary<int, int> before, int tid)
    {
        if (!before.ContainsKey(tid)) before[tid] = p.FindType(tid) is { Location: ItemLocation.Inventory } it ? it.Quantity : 0;
    }

    MemorialRare Pick(List<MemorialRare> pool)
    {
        long total = 0;
        foreach (var r in pool) total += r.Weight;
        long x = rng.NextInt64(total);
        foreach (var r in pool)
        {
            if (x < r.Weight) return r;
            x -= r.Weight;
        }
        return pool[^1];
    }

    PrizeConfig? PickConfig(PrizeConfig[] options)
    {
        int total = 0;
        foreach (var c in options) total += Math.Max(c.Weight, 0);
        if (total == 0) return null;
        int x = rng.Next(total);
        foreach (var c in options)
        {
            if (x < Math.Max(c.Weight, 0)) return c;
            x -= Math.Max(c.Weight, 0);
        }
        return null;
    }
}
