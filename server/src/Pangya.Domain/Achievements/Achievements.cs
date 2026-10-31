using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Domain.Achievements;

/// <summary>
/// Etapa de uma conquista (QuestStuff): até 5 contadores com a quantidade exigida (o 1º é o principal, acumulado; os
/// outros são condições do mesmo evento; negativa = "no máximo") e até 3 prêmios (0x6C000001 = pontos de conquista).
/// </summary>
public sealed record QuestDef(int TypeId, (int Counter, int Amount)[] Needs, (int TypeId, int Quantity)[] Rewards);

/// <summary>
/// Conquista (Achievement.iff): Index = posição no IFF (o id que o cliente vê é Index + 1); Quests em ordem de nível;
/// BaseQuest != 0 = todas as etapas usam o contador principal da base (acumulado); sem base, cada etapa tem o seu.
/// </summary>
public sealed record AchievementDef(int TypeId, int Index, int Category, int BaseQuest, int[] Quests);

/// <summary>Tabelas de conquistas (só clientes que as têm, como o JP 983).</summary>
public interface IAchievementData
{
    IReadOnlyList<AchievementDef> Achievements { get; }
    IReadOnlyDictionary<int, QuestDef> AchievementQuests { get; }
    /// <summary>Typeid de contador que existe (CounterItem.iff); os outros são ignorados.</summary>
    bool IsCounter(int typeId);
}

/// <summary>O que um evento (uma partida, uma ação) incrementou: contador -> quantidade.</summary>
public sealed class CounterBag
{
    public Dictionary<int, long> Values { get; } = [];
    public CounterBag Add(int counter, long amount = 1)
    {
        if (counter != 0 && amount != 0) Values[counter] = Values.GetValueOrDefault(counter) + amount;
        return this;
    }
    public bool IsEmpty => Values.Count == 0;
}

/// <summary>Quem recebe os eventos de conquista de uma conexão (só o cliente que tem conquistas; o KR não tem).</summary>
public interface IAchievementSink
{
    Task RecordAsync(CounterBag bag);
}

/// <summary>Etapa fechada: o valor do contador no fechamento e quando.</summary>
public readonly record struct ClearedQuest(int AchievementTypeId, long Value, DateTime At);

/// <summary>Progresso de um jogador: contadores (conquista, slot: 0 = da base, i + 1 = da etapa i sem base) e etapas fechadas.</summary>
public sealed class AchievementBook
{
    public Dictionary<(int Achievement, int Slot), long> Counters { get; } = [];
    public Dictionary<int, ClearedQuest> Cleared { get; } = [];
    public long Counter(int achievement, int slot) => Counters.GetValueOrDefault((achievement, slot));
}

public interface IAchievementStore
{
    Task<AchievementBook> LoadAsync(long account);
    /// <summary>Grava os contadores novos (valor absoluto) e as etapas fechadas, numa transação.</summary>
    Task SaveAsync(long account, IReadOnlyList<(int Achievement, int Slot, long Value)> counters,
        IReadOnlyList<(int Quest, ClearedQuest Cleared)> cleared);
}

/// <summary>Contador que mudou (para o cliente: id = (índice da conquista + 1) × 16 + slot).</summary>
public readonly record struct CounterChange(int CounterTypeId, int Id, long Before, long After);

/// <summary>Resultado de um evento: contadores, etapas fechadas (conquista, etapa), conquistas que mudaram e prêmios.</summary>
public sealed record AchievementResult(List<CounterChange> Counters, List<(int Achievement, int Quest)> ClosedQuests,
    List<AchievementDef> Changed, List<(int TypeId, int Quantity)> Rewards, int Points)
{
    public bool IsEmpty => Counters.Count == 0 && ClosedQuests.Count == 0;
}

/// <summary>
/// Conquistas (docs/server/protocolo/SPEC-jp983-conquistas.md, comportamento da referência): um saco por evento; para cada
/// conquista com algum contador do saco, as etapas abertas em ordem: todos os contadores da etapa precisam estar no saco;
/// os secundários comparam só o valor do evento, o principal "guardado + evento" com a quantidade (negativa = no
/// máximo). O contador só anda quando a etapa casa (condições do evento ok). Etapa fechada: prêmios (pontos são
/// calculados das etapas fechadas, não gravados; itens vão direto ao inventário).
/// </summary>
public sealed class AchievementService(IAchievementData data, IAchievementStore store, ShopService? shop = null)
{
    public const int PointsTypeId = 0x6C000001, PangPouch = 0x1A000010;
    /// <summary>Memorial LV: a cada 300 pontos, até 24 (referência).</summary>
    public const int PointsPerMemorialLevel = 300, MaxMemorialLevel = 24;

    public IAchievementData Data => data;
    public IAchievementStore Store => store;

    Dictionary<int, List<AchievementDef>>? byCounter;

    /// <summary>Conquistas que usam o contador (em qualquer etapa).</summary>
    List<AchievementDef> Using(int counter)
    {
        if (byCounter == null)
        {
            var map = new Dictionary<int, List<AchievementDef>>();
            foreach (var a in data.Achievements)
                foreach (var qt in a.Quests)
                    if (data.AchievementQuests.TryGetValue(qt, out var q))
                        foreach (var (c, _) in q.Needs)
                        {
                            if (c == 0) continue;
                            if (!map.TryGetValue(c, out var list)) map[c] = list = [];
                            if (list.Count == 0 || list[^1] != a) list.Add(a);
                        }
            byCounter = map;
        }
        return byCounter.TryGetValue(counter, out var l) ? l : [];
    }

    public static int CounterId(AchievementDef a, int slot) => (a.Index + 1) * 16 + slot;

    /// <summary>Slot do contador principal da etapa i: 0 (base) ou i + 1 (sem base).</summary>
    public static int SlotOf(AchievementDef a, int questIndex) => a.BaseQuest != 0 ? 0 : questIndex + 1;

    /// <summary>Typeid do contador principal da etapa (o da base, se houver).</summary>
    public int MainCounter(AchievementDef a, int questIndex)
    {
        int qt = a.BaseQuest != 0 ? a.BaseQuest : a.Quests[questIndex];
        return data.AchievementQuests.TryGetValue(qt, out var q) && q.Needs.Length > 0 ? q.Needs[0].Counter : 0;
    }

    public static bool IsDone(AchievementDef a, AchievementBook b)
    {
        foreach (var q in a.Quests) if (!b.Cleared.ContainsKey(q)) return false;
        return a.Quests.Length > 0;
    }

    /// <summary>Pontos = soma dos prêmios 0x6C000001 das etapas fechadas.</summary>
    public int Points(AchievementBook b)
    {
        long sum = 0;
        foreach (var qt in b.Cleared.Keys)
            if (data.AchievementQuests.TryGetValue(qt, out var q))
                foreach (var (tid, n) in q.Rewards) if (tid == PointsTypeId) sum += n;
        return (int)Math.Min(sum, int.MaxValue);
    }

    public static int MemorialLevel(int points) => points <= 0 ? 0 : Math.Min(MaxMemorialLevel, (points - 1) / PointsPerMemorialLevel);

    static bool Meets(long value, int amount) => amount < 0 ? value <= amount : value >= amount;

    /// <summary>Avalia o evento, grava e entrega os prêmios. O livro do jogador é atualizado.</summary>
    public async Task<AchievementResult> RecordAsync(Player p, AchievementBook book, CounterBag bag)
    {
        var counters = new List<CounterChange>();
        var closed = new List<(int, int)>();
        var changed = new List<AchievementDef>();
        var rewards = new List<(int, int)>();
        var saveCounters = new List<(int, int, long)>();
        var saveCleared = new List<(int, ClearedQuest)>();
        var seen = new HashSet<int>();
        var now = DateTime.UtcNow;
        foreach (var (counter, amount) in bag.Values)
        {
            if (amount == 0 || !data.IsCounter(counter)) continue;
            foreach (var a in Using(counter))
            {
                if (!seen.Add(a.Index) || IsDone(a, book)) continue;
                bool any = false;
                var advanced = new HashSet<int>();                          // slots que já andaram neste evento
                for (int i = 0; i < a.Quests.Length; i++)
                {
                    int qt = a.Quests[i];
                    if (book.Cleared.ContainsKey(qt) || !data.AchievementQuests.TryGetValue(qt, out var q)) continue;
                    bool present = q.Needs.Length > 0;
                    foreach (var (c, _) in q.Needs) if (c != 0 && !bag.Values.ContainsKey(c)) { present = false; break; }
                    if (!present) { if (a.BaseQuest != 0) break; continue; }
                    bool secondaryOk = true;
                    for (int j = 1; j < q.Needs.Length; j++)
                        if (q.Needs[j].Counter != 0 && !Meets(bag.Values[q.Needs[j].Counter], q.Needs[j].Amount)) { secondaryOk = false; break; }
                    if (!secondaryOk) { if (a.BaseQuest != 0) break; continue; }
                    int slot = SlotOf(a, i), main = MainCounter(a, i);
                    long delta = bag.Values.GetValueOrDefault(main);
                    long before = book.Counter(a.Index, slot), after = advanced.Contains(slot) ? before : before + delta;
                    if (!advanced.Contains(slot) && delta != 0)
                    {
                        advanced.Add(slot);
                        book.Counters[(a.Index, slot)] = after;
                        counters.Add(new CounterChange(main, CounterId(a, slot), before, after));
                        saveCounters.Add((a.TypeId, slot, after));
                        any = true;
                    }
                    int need = q.Needs[0].Amount;
                    // "no máximo" (negativa) vale para o valor do evento; "pelo menos" para o acumulado
                    if (!Meets(need < 0 ? delta : after, need)) { if (a.BaseQuest != 0) break; continue; }
                    var cq = new ClearedQuest(a.TypeId, after, now);
                    book.Cleared[qt] = cq;
                    saveCleared.Add((qt, cq));
                    closed.Add((a.TypeId, qt));
                    foreach (var (tid, n) in q.Rewards) if (tid != 0 && tid != PointsTypeId && n > 0) rewards.Add((tid, n));
                    any = true;
                }
                if (any) changed.Add(a);
            }
        }
        if (saveCounters.Count > 0 || saveCleared.Count > 0) await store.SaveAsync(p.AccountId, saveCounters, saveCleared);
        if (shop != null)
            foreach (var (tid, n) in rewards)
                if (tid == PangPouch)                                        // saco de pang: vai para o saldo
                {
                    long pang = p.Pang + n;
                    await shop.Store.ApplyAsync(p.AccountId, new PlayerChanges { Pang = pang });
                    p.Pang = pang;
                }
                else await shop.GiveAsync(p, tid, n);
        return new AchievementResult(counters, closed, changed, rewards, Points(book));
    }
}
