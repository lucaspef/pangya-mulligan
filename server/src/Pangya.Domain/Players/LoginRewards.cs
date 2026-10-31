using Pangya.Core.Config;
using Pangya.Domain.Mail;

namespace Pangya.Domain.Players;

/// <summary>Progresso de um jogador num evento de prêmio de login: dias contados, prêmios já dados, encerrado e o último dia.</summary>
public sealed record LoginRewardProgress(int Days, int Times, bool Clear, DateOnly LastDay);

public interface ILoginRewardStore
{
    Task<LoginRewardProgress?> GetAsync(long account, int eventId);
    Task SaveAsync(long account, int eventId, LoginRewardProgress progress);
}

/// <summary>
/// Prêmio de login por evento (SPEC-jp983-eventos-drops.md §5, como o LoginRewardSystem da referência): cada dia novo com
/// login conta 1; ao chegar em Days o prêmio vai por carta e a contagem recomeça. NTimes encerra depois de Times prêmios;
/// Forever não acaba. Evento com End vencido não conta.
/// </summary>
public sealed class LoginRewardService(ILoginRewardStore store, MailService? mail, IReadOnlyList<LoginRewardEvent> events)
{
    /// <summary>Login do dia: devolve os nomes dos eventos que deram prêmio.</summary>
    public async Task<List<string>> OnLoginAsync(Player p, DateOnly today)
    {
        var given = new List<string>();
        foreach (var e in events)
        {
            if (e.Id <= 0 || e.Days <= 0 || e.TypeId == 0) continue;
            if (e.End is { } end && today > end) continue;
            if (e.Start is { } start && today < start) continue;
            var cur = await store.GetAsync(p.AccountId, e.Id);
            if (cur is { Clear: true }) continue;
            if (cur != null && cur.LastDay >= today) continue;                   // já contou hoje
            int days = (cur?.Days ?? 0) + 1, times = cur?.Times ?? 0;
            bool clear = false;
            if (days >= e.Days)
            {
                days = 0;
                times++;
                clear = !e.Forever && times >= Math.Max(1, e.Times);
                if (mail != null)
                    await mail.SendSystemAsync(p.AccountId, "@Event", $"Login Reward - {e.Name}", [(e.TypeId, Math.Max(1, e.Quantity))]);
                given.Add(e.Name);
            }
            await store.SaveAsync(p.AccountId, e.Id, new LoginRewardProgress(days, times, clear, today));
        }
        return given;
    }
}
