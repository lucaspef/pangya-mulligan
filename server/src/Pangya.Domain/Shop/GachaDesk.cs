using System.Collections.Concurrent;
using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Domain.Shop;

/// <summary>
/// Gacha do JP 983 (docs/server/protocolo/SPEC-jp983-eventos-drops.md §6): o site (/on_client/gacha) sorteia no jogador que está
/// online no game-jp (o inventário em memória é o dele), e o jogo manda as mudanças quando a página fecha (0x9E -> 0x216 +
/// 0x102). Ticket: 1 prêmio; 10 parciais: 1; Gacha Ticket (+5): 5; (+11): 11.
/// </summary>
public sealed class GachaDesk(GachaConfig cfg)
{
    public const int Ticket = 0x1A000080, TicketPlus5 = 0x1A000081, TicketPlus11 = 0x1A000082, Partial = 0x1A000083, PartialsPerSpin = 10;

    readonly ConcurrentDictionary<long, (Player Player, ShopService Shop)> online = new();
    readonly ConcurrentDictionary<long, List<ItemChange>> pending = new();

    public GachaConfig Config => cfg;

    public void Register(Player p, ShopService shop) => online[p.AccountId] = (p, shop);
    public void Unregister(long account) { online.TryRemove(account, out _); pending.TryRemove(account, out _); }
    public Player? Online(long account) => online.TryGetValue(account, out var o) ? o.Player : null;

    public static (int Tickets, int Partials, int Plus5, int Plus11) Counts(Player p) =>
        (p.FindType(Ticket)?.Quantity ?? 0, p.FindType(Partial)?.Quantity ?? 0, p.FindType(TicketPlus5)?.Quantity ?? 0, p.FindType(TicketPlus11)?.Quantity ?? 0);

    /// <summary>Mudanças desde a última vez (para o 0x216 ao fechar a página).</summary>
    public List<ItemChange> TakePending(long account) => pending.TryRemove(account, out var l) ? l : [];

    /// <summary>Uma jogada: kind = ticket | partial | plus5 | plus11. Devolve o erro (null = ok) e os prêmios.</summary>
    public async Task<(string? Error, List<(int TypeId, int Quantity)> Prizes)> SpinAsync(long account, string kind, Random? random = null)
    {
        if (!online.TryGetValue(account, out var o)) return ("Entre no jogo para usar o gacha.", []);
        var (p, shop) = o;
        var (cost, qty, spins) = kind switch
        {
            "partial" => (Partial, PartialsPerSpin, 1),
            "plus5" => (TicketPlus5, 1, 5),
            "plus11" => (TicketPlus11, 1, 11),
            _ => (Ticket, 1, 1),
        };
        if (cfg.Items.Count == 0) return ("O gacha está vazio.", []);
        var rng = random ?? Random.Shared;
        var changes = new List<ItemChange>();
        lock (p)
        {
            if (p.FindType(cost) is not { } t || t.Quantity < qty) return ("Você não tem tickets suficientes.", []);
        }
        var stack = p.FindType(cost)!;
        int before = stack.Quantity;
        var e = stack.Clone();
        e.Quantity -= qty;
        var ch = new PlayerChanges();
        if (e.Quantity == 0) ch.Removed.Add(e.Id); else ch.Updated.Add(e);
        await shop.Store.ApplyAsync(p.AccountId, ch);
        if (e.Quantity == 0) p.Items.Remove(e.Id); else p.Items[e.Id] = e;
        changes.Add(new ItemChange(cost, e.Id, before, e.Quantity));
        var prizes = new List<(int, int)>();
        int total = 0;
        foreach (var it in cfg.Items) total += Math.Max(1, it.Weight);
        for (int s = 0; s < spins; s++)
        {
            int roll = rng.Next(total);
            GachaItem pick = cfg.Items[^1];
            foreach (var it in cfg.Items) { if ((roll -= Math.Max(1, it.Weight)) < 0) { pick = it; break; } }
            int had = p.FindType(pick.TypeId)?.Quantity ?? 0;
            var (code, granted) = await shop.GiveAsync(p, pick.TypeId, Math.Max(1, pick.Quantity), pick.Days);
            if (code != ShopCode.Ok || granted.Count == 0) continue;
            prizes.Add((pick.TypeId, Math.Max(1, pick.Quantity)));
            changes.Add(new ItemChange(pick.TypeId, granted[0].Id, had, p.Find(granted[0].Id)?.Quantity ?? 1));
        }
        pending.AddOrUpdate(account, changes, (_, l) => { l.AddRange(changes); return l; });
        return (null, prizes);
    }
}
