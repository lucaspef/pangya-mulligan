using Dapper;
using Pangya.Domain.Achievements;

namespace Pangya.Data;

/// <summary>Progresso das conquistas (migração 013).</summary>
public sealed class AchievementRepository(Db db) : IAchievementStore
{
    sealed record CounterRow(int AchievementTid, short Slot, long Value);
    sealed record QuestRow(int QuestTid, int AchievementTid, long Value, DateTime ClearedAt);

    public async Task<AchievementBook> LoadAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var book = new AchievementBook();
        foreach (var r in await c.QueryAsync<CounterRow>(
            "select achievement_tid, slot, value from achievement_counters where account_id = @account", new { account }))
            book.Counters[(r.AchievementTid, r.Slot)] = r.Value;
        foreach (var r in await c.QueryAsync<QuestRow>(
            "select quest_tid, achievement_tid, value, cleared_at from achievement_quests where account_id = @account", new { account }))
            book.Cleared[r.QuestTid] = new ClearedQuest(r.AchievementTid, r.Value, DateTime.SpecifyKind(r.ClearedAt, DateTimeKind.Utc));
        return book;
    }

    public async Task SaveAsync(long account, IReadOnlyList<(int Achievement, int Slot, long Value)> counters,
        IReadOnlyList<(int Quest, ClearedQuest Cleared)> cleared)
    {
        await using var c = await db.OpenAsync();
        await using var tx = await c.BeginTransactionAsync();
        foreach (var (ach, slot, value) in counters)
            await c.ExecuteAsync("""
                insert into achievement_counters(account_id, achievement_tid, slot, value) values (@account, @ach, @slot, @value)
                on conflict (account_id, achievement_tid, slot) do update set value = excluded.value
                """, new { account, ach, slot = (short)slot, value }, tx);
        foreach (var (quest, q) in cleared)
            await c.ExecuteAsync("""
                insert into achievement_quests(account_id, quest_tid, achievement_tid, value, cleared_at)
                values (@account, @quest, @ach, @value, @at) on conflict do nothing
                """, new { account, quest, ach = q.AchievementTypeId, value = q.Value, at = q.At }, tx);
        await tx.CommitAsync();
    }
}
