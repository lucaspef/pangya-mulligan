using Pangya.Core.Config;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Tests;

/// <summary>Mission do lobby: 2 missões do dia (zeram à meia-noite), 5 do período e o prêmio uma vez por tipo.</summary>
public class MissionEventTests
{
    sealed class MemStore : IMissionStore
    {
        public readonly Dictionary<long, MissionProgress> Rows = [];
        public Task<MissionProgress?> GetAsync(long a) => Task.FromResult(Rows.TryGetValue(a, out var r) ? r : null);
        public Task SaveAsync(long a, MissionProgress p) { Rows[a] = p; return Task.CompletedTask; }
    }

    static readonly DateOnly Day = new(2026, 11, 4);

    static Task<MissionProgress?> End(MissionEventService m, GameMode mode, byte course = 0, int holes = 18, float putt = 0,
        bool record = false, int eagles = 0, bool practice = false, bool finished = true, DateOnly? day = null) =>
        m.OnGameEndAsync(1, mode, practice, finished, course, holes, putt, record, eagles, day ?? Day);

    [Fact]
    public async Task DayMissionsResetAtMidnightAndPayOnce()
    {
        var m = new MissionEventService(new MemStore(), null, new MissionEventConfig());
        Assert.Null(await End(m, GameMode.Stroke, practice: true));
        Assert.Null(await End(m, GameMode.Stroke, finished: false));
        for (int i = 0; i < 3; i++) await End(m, GameMode.Stroke, holes: 3);
        await End(m, GameMode.Tournament, holes: 3);
        Assert.Equal(MissionEventService.GiftFailed, await m.ClaimAsync(1, MissionEventService.DayGift, Day));
        await End(m, (GameMode)14, holes: 3);
        var all = MissionEventService.Missions(await m.GetAsync(1, Day));
        Assert.True(all[0].Complete && all[1].Complete);
        Assert.Equal(MissionEventService.GiftSent, await m.ClaimAsync(1, MissionEventService.DayGift, Day));
        Assert.Equal(MissionEventService.GiftTaken, await m.ClaimAsync(1, MissionEventService.DayGift, Day));
        var tomorrow = await m.GetAsync(1, Day.AddDays(1));
        Assert.Equal((0, 0, false, 3), (tomorrow.DayVersus, tomorrow.DayTourney, tomorrow.DayGift, tomorrow.Versus));
    }

    [Fact]
    public async Task TermMissionsNeedAllFive()
    {
        var m = new MissionEventService(new MemStore(), null, new MissionEventConfig());
        for (byte c = 0; c <= 16; c++) await End(m, GameMode.Stroke, course: c, eagles: 2);
        Assert.Equal(MissionEventService.AllCourses, (await m.GetAsync(1, Day)).Courses);
        for (int i = 0; i < 13; i++) await End(m, GameMode.Match, holes: 9, course: 12);
        await End(m, GameMode.Stroke, putt: 31.5f, holes: 3);
        Assert.Equal(MissionEventService.GiftFailed, await m.ClaimAsync(1, MissionEventService.TermGift, Day));   // falta o recorde
        await End(m, GameMode.Stroke, record: true, holes: 3);
        var p = await m.GetAsync(1, Day);
        Assert.Equal((32, 1, 1, 34), (p.Versus, p.LongPutt, p.Record, p.Eagles));
        Assert.Equal(MissionEventService.GiftSent, await m.ClaimAsync(1, MissionEventService.TermGift, Day));
        Assert.Equal(MissionEventService.GiftFailed, await m.ClaimAsync(1, 3, Day));
    }

    [Fact]
    public void CourseBitsSkipCourse12()
    {
        Assert.Equal(1 << 11, MissionEventService.CourseBit(11));
        Assert.Equal(0, MissionEventService.CourseBit(12));
        Assert.Equal(1 << 15, MissionEventService.CourseBit(15));
        Assert.Equal(0, MissionEventService.CourseBit(16));
    }
}
