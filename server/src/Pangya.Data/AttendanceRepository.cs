using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Presença diária (migração 016).</summary>
public sealed class AttendanceRepository(Db db) : IAttendanceStore
{
    sealed record Row(DateTime LastDay, int Days, int NextTid, int NextQty);

    public async Task<AttendanceState?> LoadAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>(
            "select last_day::timestamp as last_day, days, next_tid, next_qty from attendance where account_id = @account", new { account });
        return r == null ? null : new AttendanceState(DateOnly.FromDateTime(r.LastDay), r.Days, r.NextTid, r.NextQty);
    }

    public async Task SaveAsync(long account, AttendanceState s)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into attendance(account_id, last_day, days, next_tid, next_qty) values (@account, @day, @days, @tid, @qty)
            on conflict (account_id) do update set last_day = excluded.last_day, days = excluded.days,
                next_tid = excluded.next_tid, next_qty = excluded.next_qty
            """, new { account, day = s.LastDay.ToDateTime(TimeOnly.MinValue), days = s.Days, tid = s.NextTypeId, qty = s.NextQuantity });
    }
}
