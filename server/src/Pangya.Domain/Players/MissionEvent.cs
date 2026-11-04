using Pangya.Core.Config;
using Pangya.Domain.Mail;
using Pangya.Domain.Rooms;

namespace Pangya.Domain.Players;

/// <summary>Progresso do Mission: contadores do dia (zeram à meia-noite) e do período, e os prêmios já pegos.</summary>
public sealed record MissionProgress(DateOnly Day, int DayVersus, int DayTourney, bool DayGift,
    int Versus, int LongPutt, int Record, int Eagles, int Courses, bool TermGift);

public interface IMissionStore
{
    Task<MissionProgress?> GetAsync(long account);
    Task SaveAsync(long account, MissionProgress progress);
}

/// <summary>
/// Mission do lobby (FrMissionEventDlg, conteúdo 0x80), com as regras da arte do KR (missionevent_explanation): diárias = 3
/// partidas de versus e 2 de torneio; do período = 30 de versus, putt de 30y, recorde novo, 30 eagles e os 15 campos com
/// 18 buracos. Ordem do cliente: 0..1 do dia, 2..6 do período; a 6 leva os campos em bits (campo 12 fica de fora).
/// </summary>
public sealed class MissionEventService(IMissionStore store, MailService? mail, MissionEventConfig config, Random? rng = null)
{
    public const int AllCourses = 0xEFFF;
    /// <summary>Respostas do 0x18A: 0 = enviado por carta, 1 = já recebeu, 2 = falhou.</summary>
    public const int GiftSent = 0, GiftTaken = 1, GiftFailed = 2;
    public const int DayGift = 1, TermGift = 2;

    readonly Random rng = rng ?? Random.Shared;

    public MissionEventConfig Config { get; } = config;

    public bool Open(DateOnly today) =>
        Config.Enabled && (Config.Start is not { } s || today >= s) && (Config.End is not { } e || today <= e);

    /// <summary>Bit do campo na missão dos campos (0 = não conta).</summary>
    public static int CourseBit(byte course) => course is <= 15 and not 12 ? 1 << course : 0;

    public async Task<MissionProgress> GetAsync(long account, DateOnly today)
    {
        var p = await store.GetAsync(account) ?? new MissionProgress(today, 0, 0, false, 0, 0, 0, 0, 0, false);
        return p.Day == today ? p : p with { Day = today, DayVersus = 0, DayTourney = 0, DayGift = false };
    }

    /// <summary>As 7 missões na ordem do cliente: (valor mostrado, completa).</summary>
    public static (int Condition, bool Complete)[] Missions(MissionProgress p) =>
    [
        (p.DayVersus, p.DayVersus >= 3), (p.DayTourney, p.DayTourney >= 2), (p.Versus, p.Versus >= 30),
        (p.LongPutt, p.LongPutt > 0), (p.Record, p.Record > 0), (p.Eagles, p.Eagles >= 30), (p.Courses, p.Courses == AllCourses),
    ];

    /// <summary>Fim de partida terminada (sem prática): soma as missões. Devolve o progresso novo ou null.</summary>
    public async Task<MissionProgress?> OnGameEndAsync(long account, GameMode mode, bool practice, bool finished, byte course, int holes,
        float longestPutt, bool newRecord, int eagles, DateOnly today)
    {
        if (!Open(today) || practice || !finished) return null;
        var p = await GetAsync(account, today);
        bool versus = mode is GameMode.Stroke or GameMode.Match, tourney = mode is GameMode.Tournament or (GameMode)14;
        var next = p with
        {
            DayVersus = Math.Min(p.DayVersus + (versus ? 1 : 0), 999), DayTourney = Math.Min(p.DayTourney + (tourney ? 1 : 0), 999),
            Versus = Math.Min(p.Versus + (versus ? 1 : 0), 999), LongPutt = longestPutt >= 30 ? 1 : p.LongPutt,
            Record = newRecord ? 1 : p.Record, Eagles = Math.Min(p.Eagles + Math.Max(eagles, 0), 999),
            Courses = holes >= 18 ? p.Courses | CourseBit((byte)(course & 0x7F)) : p.Courses,
        };
        if (next == p) return null;
        await store.SaveAsync(account, next);
        return next;
    }

    /// <summary>Pedido do prêmio (1 = do dia, 2 = do período): só com as missões completas e uma vez; os itens vão por carta.</summary>
    public async Task<int> ClaimAsync(long account, int type, DateOnly today)
    {
        if (!Open(today) || type is not (DayGift or TermGift)) return GiftFailed;
        var p = await GetAsync(account, today);
        var m = Missions(p);
        bool day = type == DayGift;
        if (day ? p.DayGift : p.TermGift) return GiftTaken;
        if (day ? !(m[0].Complete && m[1].Complete) : !m[2..].All(x => x.Complete)) return GiftFailed;
        await store.SaveAsync(account, day ? p with { DayGift = true } : p with { TermGift = true });
        if (mail != null && Pick(day ? Config.DayPrizes : Config.TermPrizes) is { } it)
            await mail.SendSystemAsync(account, "@Event", day ? "Mission diaria" : "Mission do periodo", [(it.TypeId, Math.Max(1, it.Quantity))]);
        return GiftSent;
    }

    /// <summary>Um prêmio da lista, pelo peso (o oficial sorteava um).</summary>
    GachaItem? Pick(List<GachaItem> list)
    {
        int total = list.Sum(x => Math.Max(x.Weight, 0));
        if (total <= 0) return null;
        int roll = rng.Next(total);
        foreach (var it in list)
            if ((roll -= Math.Max(it.Weight, 0)) < 0) return it;
        return null;
    }
}
