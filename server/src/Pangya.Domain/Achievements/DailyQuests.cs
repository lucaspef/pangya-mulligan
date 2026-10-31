using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Domain.Achievements;

/// <summary>Missão diária (QuestItem.iff): dificuldade 1..3, a etapa (QuestStuff) e o prêmio.</summary>
public sealed record DailyQuestDef(int TypeId, int Difficulty, int Stuff, (int TypeId, int Quantity)[] Rewards);

public interface IDailyQuestData
{
    IReadOnlyDictionary<int, DailyQuestDef> DailyQuests { get; }
}

/// <summary>Uma missão do jogador: 1 pendente (do dia, não aceita), 3 aceita, 4 concluída (falta pegar o prêmio).</summary>
public sealed class DailyQuest
{
    public const int Pending = 1, Active = 3, Done = 4;
    public int Id { get; set; }
    public int TypeId { get; set; }
    public int Status { get; set; }
    public long Counter { get; set; }
    public DateTime? DoneAt { get; set; }
}

/// <summary>Missões de um jogador: o dia das pendentes, o último aceite e as missões (com a permanente "Clear 10").</summary>
public sealed class DailyQuestState
{
    public DateOnly Day { get; set; }
    public DateTime? AcceptedAt { get; set; }
    public List<DailyQuest> Quests { get; set; } = [];
    public DailyQuest? Find(int id)
    {
        foreach (var q in Quests) if (q.Id == id) return q;
        return null;
    }
}

public interface IDailyQuestStore
{
    Task<DailyQuestState?> LoadAsync(long account);
    Task SaveAsync(long account, DailyQuestState state);
}

/// <summary>O que abrir a janela mudou: missões novas do dia e ids das pendentes do dia anterior que saíram.</summary>
public sealed record DailyOpen(DailyQuestState State, List<DailyQuest> Created, List<int> Removed, int[] Today);

/// <summary>Contador de missão que andou (id do contador = id × 16 + 1) e se a missão fechou.</summary>
public readonly record struct DailyProgress(DailyQuest Quest, int Counter, long Before, long After, bool Closed);

/// <summary>
/// Missão diária do JP 983 (docs/server/protocolo/SPEC-jp983-missoes.md §1): 3 do dia (uma fácil, uma média, uma difícil, as
/// mesmas para todos: sorteio com a data), aceitar, andar pelos contadores das conquistas (só aceitas), pegar o prêmio do
/// QuestItem, desistir. Fechar uma soma +1 em "Quest Clear" (0x6C400039), que anda a permanente "Clear 10".
/// </summary>
public sealed class DailyQuestService(IDailyQuestData data, IAchievementData quests, IDailyQuestStore store, ShopService? shop = null)
{
    public const int ClearTenTypeId = 0x78800001, ClearTenId = 99_999, QuestClear = 0x6C400039, QuestPrize = 0x6C40009F;
    /// <summary>Contadores que nada no servidor incrementa ainda: missões com eles ficam fora do sorteio (seriam impossíveis).</summary>
    static readonly HashSet<int> NotCounted =
    [
        0x6C400008, 0x6C400009, 0x6C400033, 0x6C400034, 0x6C400035, 0x6C400036, 0x6C400037, 0x6C400054, 0x6C400078,
        0x6C4000A5, 0x6C4000A9, 0x6C4000AB, 0x6C4000AC, 0x6C4000AD,
    ];
    static readonly DateOnly Epoch = new(2020, 1, 1);

    public IDailyQuestStore Store => store;

    public static int QuestId(DateOnly day, int slot) => 100_000 + (day.DayNumber - Epoch.DayNumber) * 4 + slot;
    public static int CounterId(DailyQuest q) => q.Id * 16 + 1;

    QuestDef? Stuff(int questTypeId) =>
        data.DailyQuests.TryGetValue(questTypeId, out var d) && quests.AchievementQuests.TryGetValue(d.Stuff, out var s) ? s : null;

    public int StuffOf(DailyQuest q) => data.DailyQuests.TryGetValue(q.TypeId, out var d) ? d.Stuff : 0;
    /// <summary>Contador principal da missão (o 1º do QuestStuff).</summary>
    public int MainCounter(DailyQuest q) => Stuff(q.TypeId) is { Needs.Length: > 0 } s ? s.Needs[0].Counter : 0;

    /// <summary>As 3 do dia: uma por dificuldade, sorteada com a data (mesmo resultado em qualquer servidor).</summary>
    public int[] Today(DateOnly day)
    {
        var groups = new List<int>[3] { [], [], [] };
        foreach (var d in data.DailyQuests.Values)
        {
            if (d.Difficulty is < 1 or > 3 || ((d.TypeId & 0xFFFFFF) >> 16) >= 0x40) continue;
            if (!quests.AchievementQuests.TryGetValue(d.Stuff, out var s) || s.Needs.Length == 0) continue;
            bool possible = true;
            foreach (var (c, _) in s.Needs) if (NotCounted.Contains(c)) { possible = false; break; }
            if (possible) groups[d.Difficulty - 1].Add(d.TypeId);
        }
        var rng = new Random(day.Year * 10_000 + day.Month * 100 + day.Day);
        var today = new int[3];
        for (int i = 0; i < 3; i++)
        {
            groups[i].Sort();
            today[i] = groups[i].Count == 0 ? 0 : groups[i][rng.Next(groups[i].Count)];
        }
        return today;
    }

    public async Task<DailyQuestState> LoadAsync(long account)
    {
        var s = await store.LoadAsync(account) ?? new DailyQuestState();
        if (s.Find(ClearTenId) == null) s.Quests.Add(new DailyQuest { Id = ClearTenId, TypeId = ClearTenTypeId, Status = DailyQuest.Active });
        return s;
    }

    /// <summary>0x151: no 1º acesso do dia as pendentes antigas saem e entram as 3 do dia (as aceitas ficam).</summary>
    public async Task<DailyOpen> OpenAsync(long account, DailyQuestState s, DateOnly today)
    {
        var created = new List<DailyQuest>();
        var removed = new List<int>();
        var tids = Today(today);
        if (s.Day != today)
        {
            for (int i = s.Quests.Count - 1; i >= 0; i--)
                if (s.Quests[i].Status == DailyQuest.Pending) { removed.Add(s.Quests[i].Id); s.Quests.RemoveAt(i); }
            for (int i = 0; i < 3; i++)
            {
                if (tids[i] == 0) continue;
                var q = new DailyQuest { Id = QuestId(today, i + 1), TypeId = tids[i], Status = DailyQuest.Pending };
                s.Quests.Add(q);
                created.Add(q);
            }
            s.Day = today;
            await store.SaveAsync(account, s);
        }
        return new DailyOpen(s, created, removed, tids);
    }

    /// <summary>0x152: as pendentes pedidas viram aceitas (contador 0).</summary>
    public async Task<List<DailyQuest>> AcceptAsync(long account, DailyQuestState s, IReadOnlyList<int> ids)
    {
        var list = new List<DailyQuest>();
        foreach (var id in ids)
            if (s.Find(id) is { Status: DailyQuest.Pending } q) { q.Status = DailyQuest.Active; q.Counter = 0; list.Add(q); }
        if (list.Count > 0)
        {
            s.AcceptedAt = DateTime.UtcNow;
            await store.SaveAsync(account, s);
        }
        return list;
    }

    /// <summary>
    /// Evento (o mesmo saco das conquistas): cada missão aceita cujos contadores estão todos no saco e cujas condições
    /// passam anda o principal; ao chegar na quantidade fecha. Fechar soma Quest Clear (que anda a "Clear 10").
    /// </summary>
    public async Task<List<DailyProgress>> RecordAsync(long account, DailyQuestState s, CounterBag bag)
    {
        var progress = new List<DailyProgress>();
        int closedDaily = 0;
        foreach (var q in s.Quests)
        {
            if (q.Status != DailyQuest.Active || q.Id == ClearTenId || Advance(q, bag) is not { } p) continue;
            progress.Add(p);
            if (p.Closed) closedDaily++;
        }
        if (closedDaily > 0 && s.Find(ClearTenId) is { Status: DailyQuest.Active } ten
            && Advance(ten, new CounterBag().Add(QuestClear, closedDaily)) is { } tp) progress.Add(tp);
        if (progress.Count > 0) await store.SaveAsync(account, s);
        return progress;
    }

    DailyProgress? Advance(DailyQuest q, CounterBag bag)
    {
        if (Stuff(q.TypeId) is not { Needs.Length: > 0 } st) return null;
        foreach (var (c, _) in st.Needs) if (c != 0 && !bag.Values.ContainsKey(c)) return null;
        for (int j = 1; j < st.Needs.Length; j++)
        {
            var (c, amount) = st.Needs[j];
            if (c != 0 && !(amount < 0 ? bag.Values[c] <= amount : bag.Values[c] >= amount)) return null;
        }
        var (main, need) = st.Needs[0];
        long delta = bag.Values[main], before = q.Counter, after = before + delta;
        q.Counter = after;
        bool closed = need < 0 ? delta <= need : after >= need;
        if (closed) { q.Status = DailyQuest.Done; q.DoneAt = DateTime.UtcNow; }
        return new DailyProgress(q, main, before, after, closed);
    }

    /// <summary>0x153: prêmio das concluídas (itens do QuestItem); a missão sai (a "Clear 10" recomeça). Devolve as pegas e os itens.</summary>
    public async Task<(List<DailyQuest> Taken, List<ItemChange> Items)> PrizeAsync(Player p, DailyQuestState s, IReadOnlyList<int> ids)
    {
        var taken = new List<DailyQuest>();
        var items = new List<ItemChange>();
        foreach (var id in ids)
        {
            if (s.Find(id) is not { Status: DailyQuest.Done } q) continue;
            if (data.DailyQuests.TryGetValue(q.TypeId, out var d) && shop != null)
                foreach (var (tid, n) in d.Rewards)
                {
                    if (tid == 0 || n <= 0) continue;
                    int before = p.FindType(tid)?.Quantity ?? 0;
                    var (code, granted) = await shop.GiveAsync(p, tid, n);
                    if (code != ShopCode.Ok) continue;
                    foreach (var g in granted) items.Add(new ItemChange(g.TypeId, g.Id, before, p.Find(g.Id)?.Quantity ?? n));
                }
            taken.Add(new DailyQuest { Id = q.Id, TypeId = q.TypeId, Status = q.Status, Counter = q.Counter });
            if (q.Id == ClearTenId) { q.Status = DailyQuest.Active; q.Counter = 0; q.DoneAt = null; }
            else s.Quests.Remove(q);
        }
        if (taken.Count > 0) await store.SaveAsync(p.AccountId, s);
        return (taken, items);
    }

    /// <summary>0x154: desiste das aceitas (saem com o contador).</summary>
    public async Task<List<DailyQuest>> GiveUpAsync(long account, DailyQuestState s, IReadOnlyList<int> ids)
    {
        var list = new List<DailyQuest>();
        foreach (var id in ids)
            if (id != ClearTenId && s.Find(id) is { Status: DailyQuest.Active or DailyQuest.Done } q) { s.Quests.Remove(q); list.Add(q); }
        if (list.Count > 0) await store.SaveAsync(account, s);
        return list;
    }
}
