using Pangya.Core.Net;
using Pangya.Domain.Achievements;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Missão diária do JP 983 (SPEC-jp983-missoes.md §1): sorteio do dia, aceitar, andar, prêmio, desistir.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public void DailyQuestsOfTheDayAreOnePerDifficulty()
    {
        if (Jp983DataTests.Data is not { } d) return;
        Assert.True(d.DailyQuests.Count > 240);
        var svc = new DailyQuestService(d, d, null!);
        var day = new DateOnly(2026, 10, 12);
        var today = svc.Today(day);
        Assert.All(today, t => Assert.NotEqual(0, t));
        Assert.Equal([1, 2, 3], today.Select(t => d.DailyQuests[t].Difficulty));
        Assert.Equal(today, svc.Today(day));                                             // o mesmo para todos no dia
        Assert.Equal(100_000 + (day.DayNumber - new DateOnly(2020, 1, 1).DayNumber) * 4 + 2, DailyQuestService.QuestId(day, 2));
    }

    [Fact]
    public async Task DailyQuestIsAcceptedPlayedAndPaid()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0);
        await using var _ = g;
        var day = DateOnly.FromDateTime(DateTime.Now);

        await g.SendAsync(new PacketWriter(0x151));
        var created = await g.ExpectAsync(0x216);
        created.U32();
        Assert.Equal(3u, created.U32());
        var open = await g.ExpectAsync(0x225);
        Assert.Equal(0, open.I32());
        open.U32();
        Assert.Equal((0u, 3u), (open.U32(), open.U32()));
        int[] tids = [(int)open.U32(), (int)open.U32(), (int)open.U32()];
        Assert.Equal(0, open.I32());                                                     // nada removido

        await g.SendAsync(new PacketWriter(0x151));                                     // de novo no mesmo dia: nada novo
        created = await g.ExpectAsync(0x216);
        created.U32();
        Assert.Equal(0u, created.U32());
        await g.ExpectAsync(0x225);

        int id = DailyQuestService.QuestId(day, 1);
        await g.SendAsync(new PacketWriter(0x152).I32(1).I32(id));
        await g.ExpectAsync(0x216);
        var acc226 = await g.ExpectAsync(0x226);
        Assert.Equal((0, 1), (acc226.I32(), acc226.I32()));
        Assert.Equal(((byte)1, (uint)tids[0], id, DailyQuest.Active), (acc226.U8(), acc226.U32(), acc226.I32(), acc226.I32()));

        // o evento que fecha a missão: todos os contadores da etapa com a quantidade pedida
        var stuff = data.AchievementQuests[data.DailyQuests[tids[0]].Stuff];
        var bag = new CounterBag();
        foreach (var (c, amount) in stuff.Needs) if (c != 0) bag.Add(c, amount);
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        await h.Achievements!.RecordAsync(bag);
        // o 0x22E das conquistas comuns que os mesmos contadores fecham vem antes do da missão diária
        var closedTids = new List<uint>();
        for (int k = 0; k < 3 && !closedTids.Contains((uint)tids[0]); k++)
        {
            var closed = await g.ExpectAsync(0x22E);
            for (uint n = closed.U32(), i = 0; i < n; i++) { closedTids.Add(closed.U32()); closed.U32(); }
        }
        Assert.Contains((uint)tids[0], closedTids);

        await g.SendAsync(new PacketWriter(0x153).I32(1).I32(id));
        var prize = await g.ExpectAsync(0x227);
        Assert.Equal((0, 1, id), (prize.I32(), prize.I32(), prize.I32()));
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        var (rewardTid, _) = data.DailyQuests[tids[0]].Rewards[0];
        Assert.True(p.FindType(rewardTid) != null || rewardTid == 0x1A000010);

        await g.SendAsync(new PacketWriter(0x153).I32(1).I32(id));                       // de novo: já foi
        Assert.NotEqual(0, (await g.ExpectAsync(0x227)).I32());

        int id2 = DailyQuestService.QuestId(day, 2);                                     // desistir da média
        await g.SendAsync(new PacketWriter(0x152).I32(1).I32(id2));
        await g.ExpectAsync(0x226);
        await g.SendAsync(new PacketWriter(0x154).I32(1).I32(id2));
        var gu = await g.ExpectAsync(0x228);
        Assert.Equal((0, 1, id2), (gu.I32(), gu.I32(), gu.I32()));
        var st = (await env.S.DailyQuests.LoadAsync(acc.Id))!;
        Assert.Null(st.Find(id2));
        Assert.NotNull(st.Find(DailyQuestService.ClearTenId));
    }
}
