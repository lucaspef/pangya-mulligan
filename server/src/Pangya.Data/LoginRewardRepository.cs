using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Prêmio de login por evento (migração 021).</summary>
public sealed class LoginRewardRepository(Db db) : ILoginRewardStore
{
    sealed record Row(int Days, int Times, bool Clear, DateTime LastDay);

    public async Task<LoginRewardProgress?> GetAsync(long account, int eventId)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>(
            "select days, times, clear, last_day::timestamp as last_day from login_rewards where account_id = @account and event_id = @eventId",
            new { account, eventId });
        return r == null ? null : new LoginRewardProgress(r.Days, r.Times, r.Clear, DateOnly.FromDateTime(r.LastDay));
    }

    public async Task SaveAsync(long account, int eventId, LoginRewardProgress p)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into login_rewards(account_id, event_id, days, times, clear, last_day) values (@account, @eventId, @days, @times, @clear, @day)
            on conflict (account_id, event_id) do update set days = excluded.days, times = excluded.times, clear = excluded.clear,
                last_day = excluded.last_day
            """, new { account, eventId, days = p.Days, times = p.Times, clear = p.Clear, day = p.LastDay.ToDateTime(TimeOnly.MinValue) });
    }
}
