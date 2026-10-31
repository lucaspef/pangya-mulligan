using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Achievements;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Conquistas do JP (docs/server/protocolo/SPEC-jp983-conquistas.md): no login 0x21D (contadores) e 0x21E (conquistas); a
/// janela (C->S 0x157, também a do Papel/Memorial, que tira daí "Achieve Points" e "Memorial LV") recebe 0x22D + 0x22C;
/// cada evento que anda manda 0x216 (contadores) + 0x216 (prêmios) + 0x22E (etapas fechadas) + 0x220 (conquistas).
/// Listas grandes vão em pedaços de 20, cada um com total = n do pedaço (como a referência).
/// </summary>
public sealed partial class JpSystems : IAchievementSink
{
    public const ushort CAchievements = 0x157;
    const ushort SCounters = 0x21D, SAchievements = 0x21E, SAchievementList = 0x22D, SAchievementDone = 0x22C,
        SQuestsClosed = 0x22E, SAchievementsChanged = 0x220;
    const int Chunk = 20;
    const int StatusActive = 3, StatusDone = 4;

    AchievementBook? book;

    /// <summary>Depois do login: carrega o livro e manda contadores e conquistas.</summary>
    public async Task AfterLoginAsync()
    {
        await LoadGrandPrixAsync();
        await SendTrophiesAsync();
        await SendInvitesAsync();
        _ = PremiumLoopAsync();                                             // premium: liga/vence (conferido a cada 20 s)
        if (achievements == null) return;
        book = await achievements.Store.LoadAsync(kr.Player.AccountId);
        kr.Achievements = this;
        var all = achievements.Data.Achievements;
        var counters = new List<(int Tid, int Id, long Value)>();
        foreach (var a in all)
        {
            if (a.BaseQuest != 0) counters.Add((achievements.MainCounter(a, 0), AchievementService.CounterId(a, 0), book.Counter(a.Index, 0)));
            else
                for (int i = 0; i < a.Quests.Length; i++)
                    counters.Add((achievements.MainCounter(a, i), AchievementService.CounterId(a, i + 1), book.Counter(a.Index, i + 1)));
        }
        for (int s = 0; s < counters.Count; s += Chunk)
        {
            int n = Math.Min(Chunk, counters.Count - s);
            var w = new PacketWriter(SCounters, 16 + n * 13).I32(0).U32((uint)n).U32((uint)n);
            for (int i = s; i < s + n; i++) w.U8(1).U32((uint)counters[i].Tid).I32(counters[i].Id).I32((int)Math.Clamp(counters[i].Value, int.MinValue, int.MaxValue));
            conn.SendTranslated(w);
        }
        SendAchievementInfo(SAchievements, all, chunks: true);
        await DailyAfterLoginAsync();
        Log.Info($"{conn} JP: conquistas {all.Count}, pontos {achievements.Points(book)} (Memorial LV {AchievementService.MemorialLevel(achievements.Points(book))})");
    }

    /// <summary>AchInfo: u8 1, u32 typeid, i32 id, i32 status, u32 q, q × (u32 etapa, u32 contador, i32 id do contador, u32 fechada em).</summary>
    void WriteInfo(PacketWriter w, AchievementDef a, AchievementBook b)
    {
        w.U8(1).U32((uint)a.TypeId).I32(a.Index + 1).I32(AchievementService.IsDone(a, b) ? StatusDone : StatusActive).U32((uint)a.Quests.Length);
        for (int i = 0; i < a.Quests.Length; i++)
        {
            int slot = AchievementService.SlotOf(a, i);
            uint at = b.Cleared.TryGetValue(a.Quests[i], out var c) ? (uint)new DateTimeOffset(c.At).ToUnixTimeSeconds() : 0;
            w.U32((uint)a.Quests[i]).U32((uint)achievements!.MainCounter(a, i)).I32(AchievementService.CounterId(a, slot)).U32(at);
        }
    }

    /// <summary>0x21E (login, em pedaços: i32 0, u32 n, u32 n) ou 0x220 (mudaram: i32 0, u32 n) com AchInfo.</summary>
    void SendAchievementInfo(ushort id, IReadOnlyList<AchievementDef> list, bool chunks)
    {
        if (list.Count == 0 || book == null) return;
        int step = chunks ? Chunk : list.Count;
        for (int s = 0; s < list.Count; s += step)
        {
            int n = Math.Min(step, list.Count - s);
            var w = new PacketWriter(id, 16 + n * 180).I32(0).U32((uint)n);
            if (chunks) w.U32((uint)n);
            for (int i = s; i < s + n; i++) WriteInfo(w, list[i], book);
            conn.SendTranslated(w);
        }
    }

    /// <summary>
    /// 0x157 u32 uid -> 0x22D (i32 0, u32 n, u32 n, n × (u32 typeid, i32 id, u32 q, q × (u32 etapa, i32 valor, u32 fechada em)))
    /// em pedaços + 0x22C i32 0. Sem conquistas (cliente sem as tabelas): sem resposta (a lista vazia mostra erro).
    /// </summary>
    async Task AchievementListAsync(uint uid)
    {
        if (achievements == null) return;
        var b = uid == 0 || uid == kr.Player.AccountId ? book ??= await achievements.Store.LoadAsync(kr.Player.AccountId)
            : await achievements.Store.LoadAsync(uid);
        var all = achievements.Data.Achievements;
        for (int s = 0; s < all.Count; s += Chunk)
        {
            int n = Math.Min(Chunk, all.Count - s);
            var w = new PacketWriter(SAchievementList, 16 + n * 140).I32(0).U32((uint)n).U32((uint)n);
            for (int i = s; i < s + n; i++)
            {
                var a = all[i];
                w.U32((uint)a.TypeId).I32(a.Index + 1).U32((uint)a.Quests.Length);
                for (int j = 0; j < a.Quests.Length; j++)
                {
                    bool done = b.Cleared.TryGetValue(a.Quests[j], out var c);
                    long value = done ? c.Value : b.Counter(a.Index, AchievementService.SlotOf(a, j));
                    w.U32((uint)a.Quests[j]).I32((int)Math.Clamp(value, int.MinValue, int.MaxValue))
                        .U32(done ? (uint)new DateTimeOffset(c.At).ToUnixTimeSeconds() : 0);
                }
            }
            conn.SendTranslated(w);
        }
        conn.SendTranslated(new PacketWriter(SAchievementDone).I32(0));
    }

    /// <summary>Evento (fim de partida, Papel, Memorial...): conquistas e missões diárias aceitas.</summary>
    public async Task RecordAsync(CounterBag bag)
    {
        if (bag.IsEmpty) return;
        await RecordAchievementsAsync(bag);
        await DailyRecordAsync(bag);
    }

    /// <summary>Conquistas: avalia, grava e avisa o cliente do que andou.</summary>
    async Task RecordAchievementsAsync(CounterBag bag)
    {
        if (achievements == null || bag.IsEmpty) return;
        var p = kr.Player;
        book ??= await achievements.Store.LoadAsync(p.AccountId);
        var r = await achievements.RecordAsync(p, book, bag);
        if (r.IsEmpty) return;
        var counters = new List<ItemChange>(r.Counters.Count);
        foreach (var c in r.Counters) counters.Add(new ItemChange(c.CounterTypeId, c.Id, (int)c.Before, (int)c.After));
        if (counters.Count > 0) conn.SendTranslated(ItemsChanged(counters));
        if (r.Rewards.Count > 0)
        {
            var items = new List<ItemChange>(r.Rewards.Count);
            foreach (var (tid, n) in r.Rewards)
                if (tid != AchievementService.PangPouch && p.FindType(tid) is { } it)
                    items.Add(new ItemChange(tid, it.Id, Math.Max(0, (it.IsConsumable ? it.Quantity : 1) - n), it.IsConsumable ? it.Quantity : 1));
            if (items.Count > 0) conn.SendTranslated(ItemsChanged(items));
        }
        if (r.ClosedQuests.Count > 0)
        {
            var w = new PacketWriter(SQuestsClosed, 8 + r.ClosedQuests.Count * 8).U32((uint)r.ClosedQuests.Count);
            foreach (var (a, q) in r.ClosedQuests) w.U32((uint)a).U32((uint)q);
            conn.SendTranslated(w);
        }
        SendAchievementInfo(SAchievementsChanged, r.Changed, chunks: false);
        if (r.ClosedQuests.Count > 0)
            Log.Info($"{conn} JP: {r.ClosedQuests.Count} etapa(s) de conquista fechada(s), pontos {r.Points}");
    }
}
