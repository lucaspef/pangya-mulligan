using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Domain.Shop;

/// <summary>Prêmio de uma caixa: typeid, id do objeto (0 no pang) e quantidade.</summary>
public readonly record struct BoxPrize(int TypeId, int Id, int Quantity);

/// <summary>Caixa aberta: Code 0 = ok; Left = caixas que sobraram; Pang = saldo depois.</summary>
public sealed record BoxResult(byte Code, int Left, List<BoxPrize> Prizes, List<ItemChange> Changes)
{
    public const byte NotOwned = 1, NothingDrawn = 2;
    public static BoxResult Fail(byte code) => new(code, 0, [], []);
}

/// <summary>
/// Caixa do My Room (Lucky Pouch, Pangya Box, caixas de aniversário...): gasta 1 e sorteia 1 prêmio pelo peso. O cliente
/// não traz o conteúdo (o oficial e a referência o tinham no banco): vem da config (<see cref="BoxConfig"/>), com uma
/// lista por caixa ou a padrão. 0x1A000010 = pang direto no saldo. Item que o jogador já tem (e não empilha) é sorteado
/// de novo.
/// </summary>
public sealed class BoxService(ShopService shop, BoxConfig cfg, Random? random = null)
{
    public const int PangTid = 0x1A000010;
    const int MaxRedraws = 10;
    readonly Random rng = random ?? Random.Shared;

    /// <summary>Conteúdo configurado da caixa (o da própria caixa ou o padrão).</summary>
    public PrizeConfig[] ContentsOf(int boxTid)
    {
        foreach (var b in cfg.Boxes) if (b.TypeId == boxTid && b.Prizes.Length > 0) return b.Prizes;
        return cfg.Default;
    }

    public async Task<BoxResult> OpenAsync(Player p, int boxTid)
    {
        if (p.FindType(boxTid) is not { Location: ItemLocation.Inventory } box || PlayerActions.Free(p, box) <= 0)
            return BoxResult.Fail(BoxResult.NotOwned);
        var contents = ContentsOf(boxTid);
        var d = new ShopDraft(p, shop.Store);
        d.Edit(box).Quantity--;
        var granted = new List<Granted>();
        var prizes = new List<BoxPrize>(1);
        var before = new Dictionary<int, int>();
        long pang = 0;
        for (int attempt = 0; attempt < MaxRedraws && prizes.Count == 0; attempt++)
        {
            if (Pick(contents) is not { } c) break;
            int qty = rng.Next(Math.Max(1, c.Min), Math.Max(c.Min, c.Max) + 1);
            if (c.TypeId == PangTid) { pang = qty; prizes.Add(new BoxPrize(PangTid, 0, qty)); break; }
            if (!before.ContainsKey(c.TypeId)) before[c.TypeId] = p.FindType(c.TypeId) is { Location: ItemLocation.Inventory } it ? it.Quantity : 0;
            int n = granted.Count;
            var code = await shop.GrantAsync(d, c.TypeId, qty, 0, granted, 0);
            if (code == ShopCode.AlreadyOwned) continue;
            if (code != ShopCode.Ok) break;
            prizes.Add(new BoxPrize(c.TypeId, n < granted.Count ? granted[n].Id : 0, qty));
        }
        if (prizes.Count == 0) return BoxResult.Fail(BoxResult.NothingDrawn);
        int boxBefore = box.Quantity;
        if (pang > 0) d.Changes.Pang = p.Pang + pang;
        await d.CommitAsync();
        p.Pang += pang;

        var changes = new List<ItemChange>(granted.Count + 1) { new(boxTid, box.Id, boxBefore, boxBefore - 1) };
        foreach (var g in granted)
        {
            int after = p.Items.TryGetValue(g.Id, out var it) ? it.Quantity : g.Count;
            changes.Add(new ItemChange(g.TypeId, g.Id, before.GetValueOrDefault(g.TypeId), after));
        }
        return new BoxResult(0, boxBefore - 1, prizes, changes);
    }

    PrizeConfig? Pick(PrizeConfig[] list)
    {
        long total = 0;
        foreach (var c in list) if (c.Weight > 0) total += c.Weight;
        if (total <= 0) return null;
        long x = rng.NextInt64(total);
        foreach (var c in list)
        {
            if (c.Weight <= 0) continue;
            if (x < c.Weight) return c;
            x -= c.Weight;
        }
        return null;
    }
}
