using System.Text.Json;
using Dapper;
using Pangya.Domain.Achievements;

namespace Pangya.Data;

/// <summary>Missão diária (migração 017): uma linha por conta, as missões em jsonb.</summary>
public sealed class DailyQuestRepository(Db db) : IDailyQuestStore
{
    sealed record Row(DateTime Day, DateTime? AcceptedAt, string Quests);

    public async Task<DailyQuestState?> LoadAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>(
            "select day::timestamp as day, accepted_at, quests::text as quests from daily_quests where account_id = @account", new { account });
        if (r == null) return null;
        return new DailyQuestState
        {
            Day = DateOnly.FromDateTime(r.Day),
            AcceptedAt = r.AcceptedAt?.ToUniversalTime(),
            Quests = JsonSerializer.Deserialize<List<DailyQuest>>(r.Quests) ?? [],
        };
    }

    public async Task SaveAsync(long account, DailyQuestState s)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into daily_quests(account_id, day, accepted_at, quests) values (@account, @day, @at, @quests::jsonb)
            on conflict (account_id) do update set day = excluded.day, accepted_at = excluded.accepted_at, quests = excluded.quests
            """, new { account, day = s.Day.ToDateTime(TimeOnly.MinValue), at = s.AcceptedAt, quests = JsonSerializer.Serialize(s.Quests) });
    }
}
