using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Achievements;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Missão diária (docs/server/protocolo/SPEC-jp983-missoes.md §1.4): abrir 0x151 -> 0x216 (missões novas) + 0x225; aceitar 0x152
/// -> 0x216 (contadores) + 0x226; prêmio 0x153 -> 0x216 (itens e contadores zerados) + 0x227; desistir 0x154 -> 0x216 +
/// 0x228. O progresso vai pelo caminho das conquistas (0x216, 0x22E, 0x220) e as missões entram no 0x21D/0x21E do login.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CDailyOpen = 0x151, CDailyAccept = 0x152, CDailyPrize = 0x153, CDailyGiveUp = 0x154;
    const ushort SDailyOpen = 0x225, SDailyAccept = 0x226, SDailyPrize = 0x227, SDailyGiveUp = 0x228;

    DailyQuestState? daily;

    async Task<DailyQuestState?> DailyAsync() =>
        dailyQuests == null ? null : daily ??= await dailyQuests.LoadAsync(kr.Player.AccountId);

    static List<int> ReadIds(PacketReader r)
    {
        var ids = new List<int>();
        int n = r.Remaining >= 4 ? r.I32() : 0;
        for (int i = 0; i < n && r.Remaining >= 4; i++) ids.Add(r.I32());
        r.Skip(r.Remaining);
        return ids;
    }

    async Task<bool> HandleDailyAsync(PacketReader r)
    {
        var s = await DailyAsync();
        ushort answer = r.Id switch { CDailyOpen => SDailyOpen, CDailyAccept => SDailyAccept, CDailyPrize => SDailyPrize, _ => SDailyGiveUp };
        if (s == null || dailyQuests == null)
        {
            r.Skip(r.Remaining);
            if (r.Id == CDailyOpen) conn.SendTranslated(new PacketWriter(answer).I32(1));
            else conn.SendTranslated(new PacketWriter(answer).I32(1).I32(0));
            return true;
        }
        var p = kr.Player;
        switch (r.Id)
        {
            case CDailyOpen:
            {
                r.Skip(r.Remaining);
                var o = await dailyQuests.OpenAsync(p.AccountId, s, DateOnly.FromDateTime(DateTime.Now));
                var w216 = new PacketWriter(SItemsChanged, 16 + o.Created.Count * 50).U32((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds()).U32((uint)o.Created.Count);
                foreach (var q in o.Created) w216.U8(2).U32((uint)q.TypeId).I32(q.Id).U32(0).I32(0).I32(1).I32(1).Zeros(25);
                conn.SendTranslated(w216);
                var w = new PacketWriter(answer).I32(0).U32((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds())
                    .U32(s.AcceptedAt is { } at ? (uint)new DateTimeOffset(at, TimeSpan.Zero).ToUnixTimeSeconds() : 0).U32(3);
                foreach (var tid in o.Today) w.U32((uint)tid);
                w.I32(o.Removed.Count);
                foreach (var id in o.Removed) w.I32(id);
                conn.SendTranslated(w);
                return true;
            }
            case CDailyAccept:
            {
                var ids = ReadIds(r);
                var list = await dailyQuests.AcceptAsync(p.AccountId, s, ids);
                if (list.Count == 0) { conn.SendTranslated(new PacketWriter(answer).I32(1).I32(0)); return true; }
                var counters = new List<ItemChange>(list.Count);
                foreach (var q in list) counters.Add(new ItemChange(dailyQuests.MainCounter(q), DailyQuestService.CounterId(q), 0, 0));
                conn.SendTranslated(ItemsChanged(counters));
                var w = new PacketWriter(answer).I32(0).I32(list.Count);
                foreach (var q in list) WriteDaily(w, q);
                conn.SendTranslated(w);
                Log.Info($"{conn} JP: missão diária aceita ({list.Count})");
                return true;
            }
            case CDailyPrize:
            {
                var (taken, items) = await dailyQuests.PrizeAsync(p, s, ReadIds(r));
                if (taken.Count == 0) { conn.SendTranslated(new PacketWriter(answer).I32(1).I32(0)); return true; }
                foreach (var q in taken) items.Add(new ItemChange(dailyQuests.MainCounter(q), DailyQuestService.CounterId(q), (int)q.Counter, 0));
                conn.SendTranslated(ItemsChanged(items));
                var w = new PacketWriter(answer).I32(0).I32(taken.Count);
                foreach (var q in taken) w.I32(q.Id);
                conn.SendTranslated(w);
                Log.Info($"{conn} JP: prêmio de {taken.Count} missão(ões) diária(s)");
                int prizes = 0;
                foreach (var q in taken) if (q.Id != DailyQuestService.ClearTenId) prizes++;
                await RecordAsync(new CounterBag().Add(DailyQuestService.QuestPrize, prizes));
                return true;
            }
            default:
            {
                var list = await dailyQuests.GiveUpAsync(p.AccountId, s, ReadIds(r));
                if (list.Count == 0) { conn.SendTranslated(new PacketWriter(answer).I32(1)); return true; }
                var counters = new List<ItemChange>(list.Count);
                foreach (var q in list) counters.Add(new ItemChange(dailyQuests.MainCounter(q), DailyQuestService.CounterId(q), (int)q.Counter, 0));
                conn.SendTranslated(ItemsChanged(counters));
                var w = new PacketWriter(answer).I32(0).I32(list.Count);
                foreach (var q in list) w.I32(q.Id);
                conn.SendTranslated(w);
                return true;
            }
        }
    }

    /// <summary>AchInfo da missão: u8 1, u32 typeid, i32 id, i32 estado, u32 1, (u32 QuestStuff, u32 contador, i32 id do contador, u32 fechada em).</summary>
    void WriteDaily(PacketWriter w, DailyQuest q)
    {
        w.U8(1).U32((uint)q.TypeId).I32(q.Id).I32(q.Status).U32(1).U32((uint)dailyQuests!.StuffOf(q)).U32((uint)dailyQuests.MainCounter(q))
            .I32(DailyQuestService.CounterId(q)).U32(q.DoneAt is { } at ? (uint)new DateTimeOffset(at, TimeSpan.Zero).ToUnixTimeSeconds() : 0);
    }

    /// <summary>Login: os contadores (0x21D) e as missões (0x21E) do jogador, depois das conquistas.</summary>
    async Task DailyAfterLoginAsync()
    {
        if (await DailyAsync() is not { Quests.Count: > 0 } s) return;
        var w = new PacketWriter(SCounters, 16 + s.Quests.Count * 13).I32(0).U32((uint)s.Quests.Count).U32((uint)s.Quests.Count);
        foreach (var q in s.Quests) w.U8(1).U32((uint)dailyQuests!.MainCounter(q)).I32(DailyQuestService.CounterId(q)).I32((int)Math.Min(q.Counter, int.MaxValue));
        conn.SendTranslated(w);
        var a = new PacketWriter(SAchievements, 16 + s.Quests.Count * 40).I32(0).U32((uint)s.Quests.Count).U32((uint)s.Quests.Count);
        foreach (var q in s.Quests) WriteDaily(a, q);
        conn.SendTranslated(a);
    }

    /// <summary>Evento: as missões aceitas que andaram (0x216) e as que fecharam (0x22E + 0x220).</summary>
    async Task DailyRecordAsync(CounterBag bag)
    {
        if (dailyQuests == null || await DailyAsync() is not { } s) return;
        var progress = await dailyQuests.RecordAsync(kr.Player.AccountId, s, bag);
        if (progress.Count == 0) return;
        var counters = new List<ItemChange>(progress.Count);
        var closed = new List<DailyQuest>();
        foreach (var pr in progress)
        {
            counters.Add(new ItemChange(pr.Counter, DailyQuestService.CounterId(pr.Quest), (int)pr.Before, (int)Math.Min(pr.After, int.MaxValue)));
            if (pr.Closed) closed.Add(pr.Quest);
        }
        conn.SendTranslated(ItemsChanged(counters));
        if (closed.Count == 0) return;
        var w = new PacketWriter(SQuestsClosed, 8 + closed.Count * 8).U32((uint)closed.Count);
        foreach (var q in closed) w.U32((uint)q.TypeId).U32((uint)dailyQuests.StuffOf(q));
        conn.SendTranslated(w);
        var c = new PacketWriter(SAchievementsChanged, 16 + closed.Count * 40).I32(0).U32((uint)closed.Count);
        foreach (var q in closed) WriteDaily(c, q);
        conn.SendTranslated(c);
        Log.Info($"{conn} JP: {closed.Count} missão(ões) diária(s) concluída(s)");
        int daily = 0;
        foreach (var q in closed) if (q.Id != DailyQuestService.ClearTenId) daily++;
        if (daily > 0 && achievements != null) await RecordAchievementsAsync(new CounterBag().Add(DailyQuestService.QuestClear, daily));
    }
}
