using Pangya.Core.Config;
using Pangya.Domain.Shop;

namespace Pangya.Domain.Players;

/// <summary>Presença: último dia (data local do servidor), dias contados e o próximo prêmio.</summary>
public sealed record AttendanceState(DateOnly LastDay, int Days, int NextTypeId, int NextQuantity);

public interface IAttendanceStore
{
    Task<AttendanceState?> LoadAsync(long account);
    Task SaveAsync(long account, AttendanceState state);
}

/// <summary>Resultado da entrada do dia: NewDay = primeira vez hoje (o prêmio Now foi entregue agora).</summary>
public readonly record struct AttendanceResult(bool NewDay, int Days, (int TypeId, int Quantity) Now, (int TypeId, int Quantity) Next);

/// <summary>
/// Presença diária (cliente JP 983; docs/server/protocolo/SPEC-jp983-sistemas.md): a primeira entrada do dia conta um dia e
/// entrega o prêmio sorteado na véspera (o "próximo" que o cliente mostra ao sair); a cada 10 dias o prêmio é da lista
/// especial. O cliente não traz a tabela (a referência lê do banco): os prêmios vêm de AttendanceConfig. O prêmio vai por
/// carta do sistema, como no oficial (sem correio, direto no inventário).
/// </summary>
public sealed class AttendanceService(IAttendanceStore store, ShopService shop, AttendanceConfig cfg, Random? random = null,
    Mail.MailService? mail = null)
{
    const int PangPouch = 0x1A000010;
    readonly Random rng = random ?? Random.Shared;

    public async Task<AttendanceResult> CheckInAsync(Player p, DateTime now)
    {
        var today = DateOnly.FromDateTime(now);
        var s = await store.LoadAsync(p.AccountId);
        if (s != null && s.LastDay == today)
            return new AttendanceResult(false, s.Days, (0, 0), (s.NextTypeId, s.NextQuantity));
        int days = (s?.Days ?? 0) + 1;
        (int TypeId, int Quantity) gift = s != null && s.NextTypeId != 0 ? (s.NextTypeId, s.NextQuantity) : Draw(days);
        var next = Draw(days + 1);
        await store.SaveAsync(p.AccountId, new AttendanceState(today, days, next.TypeId, next.Quantity));
        if (mail != null)
        {
            var items = new List<(int TypeId, int Quantity)>();
            if (gift.TypeId != 0) items.Add(gift);
            foreach (var t in cfg.DailyTickets)                              // tickets do dia, até o teto
            {
                int have = p.FindType(t.TypeId)?.Quantity ?? 0, give = Math.Min(t.Min, t.Max - have);
                if (give > 0) items.Add((t.TypeId, give));
            }
            if (items.Count > 0) await mail.SendSystemAsync(p.AccountId, "@Pangya", $"Login bonus: day {days}", items);
        }
        else if (gift.TypeId == PangPouch)
        {
            long pang = p.Pang + gift.Quantity;
            await shop.Store.ApplyAsync(p.AccountId, new PlayerChanges { Pang = pang });
            p.Pang = pang;
        }
        else if (gift.TypeId != 0) await shop.GiveAsync(p, gift.TypeId, gift.Quantity);
        return new AttendanceResult(true, days, gift, next);
    }

    /// <summary>Prêmio do dia (a cada EveryNth dias, a lista especial).</summary>
    (int TypeId, int Quantity) Draw(int day)
    {
        var pool = cfg.EveryNth > 0 && day % cfg.EveryNth == 0 && cfg.Special.Length > 0 ? cfg.Special : cfg.Daily;
        int total = 0;
        foreach (var c in pool) total += Math.Max(c.Weight, 0);
        if (total == 0) return (0, 0);
        int x = rng.Next(total);
        foreach (var c in pool)
        {
            if (x < Math.Max(c.Weight, 0)) return (c.TypeId, rng.Next(Math.Max(1, c.Min), Math.Max(c.Min, c.Max) + 1));
            x -= Math.Max(c.Weight, 0);
        }
        return (0, 0);
    }
}
