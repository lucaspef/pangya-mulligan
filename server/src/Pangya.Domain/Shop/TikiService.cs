using Pangya.Domain.Players;

namespace Pangya.Domain.Shop;

/// <summary>
/// Bloco Tiki do cabeçalho comum do item (0x84): itens por lote e TP por lote (Point Shop antiga), milhagem por unidade,
/// chance e faixa do bônus, tipo (1 normal, 2 cookie, 3 raro) e o pang da troca (Tiki Shop novo).
/// </summary>
public sealed record TikiInfo(int TypeId, int PerLot, int TpPerLot, int Mileage, int BonusProb, int BonusMin, int BonusMax, int Kind, int Pang)
{
    public bool Tradeable => Kind is >= 1 and <= 3 && Pang > 0 && Mileage > 0;
}

public interface ITikiData
{
    TikiInfo? Tiki(int typeId);
    /// <summary>Vitrine da Point Shop (PointShop.iff): typeid -> (TP por lote, quantidade por lote).</summary>
    IReadOnlyDictionary<int, (int Tp, int Quantity)> PointShop { get; }
}

/// <summary>Saldo da Point Shop antiga (não é item: um contador do jogador).</summary>
public interface ITikiStore
{
    Task<int> PointsAsync(long account);
    Task SetPointsAsync(long account, int points);
}

/// <summary>Resultado de uma troca: código (0 = ok), saldo/milhagem e as mudanças de itens (0x216).</summary>
public sealed record TikiResult(uint Code, int Points = 0, int Mileage = 0, int Bonus = 0)
{
    public List<ItemChange> Changes { get; } = [];
    public long PangSpent { get; set; }
    public int TikiGained { get; set; }
}

/// <summary>
/// Point Shop (Legacy Tiki) e Tiki Shop novo do JP 983 (docs/server/protocolo/SPEC-jp983-missoes.md §2): itens viram pontos
/// (antiga: TP por lote; nova: milhagem, 1000 MP = 1 TP no item 0x1A0002A6) e pontos viram os itens da vitrine.
/// </summary>
public sealed class TikiService(IPlayerStore store, ITikiData data, ITikiStore points, ShopService shop, Random? random = null)
{
    public const int TikiPointsTypeId = 0x1A0002A6, MileageTypeId = 0x1A0002A7, MileagePerPoint = 1000;
    const uint EGeneric = 0x5200900, ENoItem = 0x5200901, ENotInShop = 0x5200902, ERemove = 0x5200903, EGive = 0x5200904,
        ENotTradeable = 0x5200905, ENoPoints = 0x5200906, EGiveShop = 0x5200907, EBusy = 0x5201010;
    readonly Random rng = random ?? Random.Shared;

    public Task<int> PointsAsync(Player p) => points.PointsAsync(p.AccountId);

    /// <summary>Tira count unidades (pilha) ou o objeto (único) para a troca, em ch; código ≠ 0 = recusado.</summary>
    static uint Take(Player p, int typeId, int id, int count, PlayerChanges ch, List<ItemChange> changes)
    {
        var it = p.Find(id) is { Location: ItemLocation.Inventory } byId && byId.TypeId == typeId ? byId : p.FindType(typeId);
        if (it == null) return ENoItem;
        if (it.Group is ItemGroup.Character || PlayerActions.IsEquipped(p, it) || it.TypeId == p.Equip.BallTypeId && it.Quantity <= count) return EBusy;
        if (it.Group is ItemGroup.Caddie or ItemGroup.Mascot && it.ExpiresAt != null) return ENotTradeable;
        bool stack = it.IsConsumable || it.Group == ItemGroup.Card;
        int have = PlayerActions.Free(p, it);
        if (stack ? have < count : have == 0 || count != 1) return stack ? ENoItem : EBusy;
        int before = stack ? it.Quantity : 1, after = stack ? it.Quantity - count : 0;
        if (after == 0) ch.Removed.Add(it.Id);
        else { var e = it.Clone(); e.Quantity = after; ch.Updated.Add(e); }
        changes.Add(new ItemChange(it.TypeId, it.Id, before, after));
        return 0;
    }

    async Task CommitAsync(Player p, PlayerChanges ch)
    {
        await store.ApplyAsync(p.AccountId, ch);
        foreach (var id in ch.Removed) p.Items.Remove(id);
        foreach (var it in ch.Updated) p.Items[it.Id] = it;
        foreach (var it in ch.Added) p.Add(it);
        if (ch.Pang is { } pang) p.Pang = pang;
    }

    /// <summary>0x128: lotes de itens trocáveis viram pontos (TP por lote; itens por lote 0 = 1).</summary>
    public async Task<TikiResult> ItemsToPointsAsync(Player p, IReadOnlyList<(int TypeId, int Id, int Lots)> items)
    {
        var ch = new PlayerChanges();
        var r = new TikiResult(0);
        int gained = 0;
        foreach (var (tid, id, lots) in items)
        {
            if (lots <= 0 || data.Tiki(tid) is not { Tradeable: true } t) return new(ENotTradeable);
            uint code = Take(p, tid, id, Math.Max(1, t.PerLot) * lots, ch, r.Changes);
            if (code != 0) return new(code);
            gained += Math.Max(1, t.TpPerLot) * lots;
        }
        if (gained <= 0) return new(ENotTradeable);
        int total = await points.PointsAsync(p.AccountId) + gained;
        await CommitAsync(p, ch);
        await points.SetPointsAsync(p.AccountId, total);
        return new TikiResult(0, total).With(r);
    }

    /// <summary>0x129: pontos viram os itens da vitrine (TP × lotes; recebe quantidade × lotes).</summary>
    public async Task<TikiResult> PointsToItemsAsync(Player p, IReadOnlyList<(int TypeId, int Lots)> items)
    {
        int cost = 0;
        foreach (var (tid, lots) in items)
        {
            if (lots <= 0 || !data.PointShop.TryGetValue(tid, out var e)) return new(ENotInShop);
            cost += e.Tp * lots;
        }
        int have = await points.PointsAsync(p.AccountId);
        if (cost > have) return new(ENoPoints);
        var r = new TikiResult(0, have - cost);
        foreach (var (tid, lots) in items)
        {
            int before = p.FindType(tid)?.Quantity ?? 0, n = data.PointShop[tid].Quantity * lots;
            var (code, granted) = await shop.GiveAsync(p, tid, n);
            if (code != ShopCode.Ok) return new(EGiveShop);
            foreach (var g in granted) r.Changes.Add(new ItemChange(g.TypeId, g.Id, before, p.Find(g.Id)?.Quantity ?? n));
        }
        await points.SetPointsAsync(p.AccountId, have - cost);
        return r;
    }

    /// <summary>
    /// 0x18D Tiki Shop novo: cada linha soma o pang da troca (uma vez por linha) e milhagem × quantidade; chance de bônus
    /// (prob do último item: ~prob/(3·prob+1)); 1000 MP viram 1 TP (item 0x1A0002A6) e o resto fica no item de milhagem.
    /// </summary>
    public async Task<TikiResult> ExchangeAsync(Player p, IReadOnlyList<(int TypeId, int Id, int Quantity)> items)
    {
        if (items.Count is < 1 or > 5) return new(EGeneric);
        var ch = new PlayerChanges();
        var r = new TikiResult(0);
        long pang = 0;
        int mileage = 0, bonusMin = 0, bonusMax = 0, prob = 0;
        foreach (var (tid, id, qty) in items)
        {
            if (qty <= 0 || data.Tiki(tid) is not { Tradeable: true } t) return new(ENotTradeable);
            uint code = Take(p, tid, id, qty, ch, r.Changes);
            if (code != 0) return new(code);
            pang += t.Pang;
            mileage += t.Mileage * qty;
            bonusMin += t.BonusMin;
            bonusMax += t.BonusMax;
            prob = t.BonusProb;
        }
        if (pang > p.Pang) return new(EGeneric);
        int bonus = 0;
        if (prob > 0 && rng.Next(prob * 3 + 1) < prob) bonus = bonusMax > bonusMin ? bonusMin + rng.Next(bonusMax - bonusMin) : bonusMin;
        var mpItem = p.FindType(MileageTypeId);
        int total = mileage + (mpItem?.Quantity ?? 0) + bonus, tp = total / MileagePerPoint, rest = total % MileagePerPoint;
        ch.Pang = p.Pang - pang;
        await CommitAsync(p, ch);
        r.PangSpent = pang;
        r.TikiGained = tp;
        // milhagem e TP pelos caminhos da loja (pilhas): acerta a milhagem para o resto
        int mpBefore = mpItem?.Quantity ?? 0;
        if (rest != mpBefore)
        {
            if (rest > mpBefore) await shop.GiveAsync(p, MileageTypeId, rest - mpBefore);
            else if (mpItem != null)
            {
                var e = mpItem.Clone();
                e.Quantity = rest;
                var mc = new PlayerChanges();
                if (rest == 0) mc.Removed.Add(e.Id); else mc.Updated.Add(e);
                await CommitAsync(p, mc);
            }
            r.Changes.Add(new ItemChange(MileageTypeId, p.FindType(MileageTypeId)?.Id ?? mpItem?.Id ?? 0, mpBefore, rest));
        }
        if (tp > 0)
        {
            int tpBefore = p.FindType(TikiPointsTypeId)?.Quantity ?? 0;
            var (code, granted) = await shop.GiveAsync(p, TikiPointsTypeId, tp);
            if (code != ShopCode.Ok) return new(EGive);
            r.Changes.Add(new ItemChange(TikiPointsTypeId, granted.Count > 0 ? granted[0].Id : 0, tpBefore, tpBefore + tp));
        }
        return new TikiResult(0, 0, mileage, bonus) { PangSpent = pang, TikiGained = tp }.With(r);
    }
}

static class TikiResultExt
{
    public static TikiResult With(this TikiResult to, TikiResult from)
    {
        to.Changes.AddRange(from.Changes);
        return to;
    }
}
