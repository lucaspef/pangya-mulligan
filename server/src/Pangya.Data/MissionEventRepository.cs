using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Mission do lobby (migração 027).</summary>
public sealed class MissionEventRepository(Db db) : IMissionStore
{
    sealed record Row(DateOnly Day, int DayVersus, int DayTourney, bool DayGift, int Versus, int LongPutt, int Record, int Eagles,
        int Courses, bool TermGift);

    public async Task<MissionProgress?> GetAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>("""
            select day, day_versus, day_tourney, day_gift, versus, long_putt, record, eagles, courses, term_gift
            from mission_event where account_id = @account
            """, new { account });
        return r == null ? null : new MissionProgress(r.Day, r.DayVersus, r.DayTourney, r.DayGift, r.Versus,
            r.LongPutt, r.Record, r.Eagles, r.Courses, r.TermGift);
    }

    public async Task SaveAsync(long account, MissionProgress p)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into mission_event(account_id, day, day_versus, day_tourney, day_gift, versus, long_putt, record, eagles, courses, term_gift)
            values (@account, @day, @dayVersus, @dayTourney, @dayGift, @versus, @longPutt, @record, @eagles, @courses, @termGift)
            on conflict (account_id) do update set day = excluded.day, day_versus = excluded.day_versus, day_tourney = excluded.day_tourney,
                day_gift = excluded.day_gift, versus = excluded.versus, long_putt = excluded.long_putt, record = excluded.record,
                eagles = excluded.eagles, courses = excluded.courses, term_gift = excluded.term_gift
            """, new { account, day = p.Day, dayVersus = p.DayVersus, dayTourney = p.DayTourney,
                dayGift = p.DayGift, versus = p.Versus, longPutt = p.LongPutt, record = p.Record, eagles = p.Eagles, courses = p.Courses,
                termGift = p.TermGift });
    }
}
