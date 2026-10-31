namespace Pangya.Domain.Players;

/// <summary>Resposta do 0x91 (KR) / 0x93 (JP) à renovação do caddie, como o cliente trata (taskmanager.cpp:8668).</summary>
public enum CaddieRenewal : byte { Fail = 0, StillHired = 1, Ok = 2, NoPang = 4 }

/// <summary>
/// Caddie com mensalidade (Caddie.iff MonthlyFee &gt; 0: no KR e no JP os caddies 1, 2, 3 e 7 e outros): contratado por
/// 30 dias, vence em <see cref="Item.ExpiresAt"/>. Vencido, o cliente tira o caddie da partida (golftask.cpp:2615,
/// shareddoc.cpp:1690) e oferece a renovação, paga em pang, na janela do caddie (0x39). O cliente estende o prazo pela peça
/// do caddie (Remain_Date = maior entre o contrato e os dias da peça, realmyroomtask.cpp:1342), então com peça em uso ele
/// continua valendo. Caddie sem ExpiresAt (sem mensalidade ou comprado antes desta regra) não vence.
/// </summary>
public static class CaddieRent
{
    public const int Days = 30;

    /// <summary>Remain_Date do sCaddieInfo: dias do contrato (arredondados para cima) ou da peça, o maior; 0 = vencido.</summary>
    public static int RemainingDays(Item c, DateTime nowUtc)
    {
        int days = 0;
        if (c.ExpiresAt is { } end && end > nowUtc) days = (int)Math.Ceiling((end - nowUtc).TotalDays - 1e-9);
        if (c.Int("part") != 0 && c.Attrs["part_until"]?.GetValue<DateTime>() is { } until && until > nowUtc)
            days = Math.Max(days, (int)Math.Ceiling((until - nowUtc).TotalHours - 1e-6) / 24);
        return Math.Min(days, 255);
    }

    /// <summary>Contratado (tem prazo) e sem nenhum dia sobrando: o cliente não usa o caddie na partida.</summary>
    public static bool IsExpired(Item c, DateTime nowUtc) => c.ExpiresAt != null && RemainingDays(c, nowUtc) == 0;
}
