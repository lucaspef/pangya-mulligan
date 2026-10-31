namespace Pangya.Domain.Players;

/// <summary>
/// Premium ticket do JP 983 (docs/server/protocolo/SPEC-jp983-efeitos.md §9): o ticket é um item com prazo (comprado por 7, 15, 30
/// ou 365 dias; comprar de novo soma). Enquanto vale: bit 0x4000 na capacidade, a bola Premium Comet, Auto Command e
/// ticket do Grand Prix sem gastar. VIP: também não vendido na loja (só GM).
/// </summary>
public static class Premium
{
    public const int NormalTicket = 0x1A100002, VipTicket = 0x1A100003, Comet = 0x140000D8, Capability = 0x4000;
    /// <summary>VIP (Desc.iff): Sakura (Premium), Lolo (Premium), Eternal Voice ClubSet (Premium); caixa de presente de ambos.</summary>
    public const int VipComet = 0x140000E9, VipMascot = 0x4000004C, VipClubSet = 0x10001001, GiftBox = 0x1A0002BA;
    /// <summary>Caixas de presente por compra do ticket (referência: 4 no Normal, 8 no VIP).</summary>
    public static int GiftBoxes(int ticket) => ticket == VipTicket ? 8 : 4;
    /// <summary>Bônus de EXP e pang (Desc.iff): 5 % no Normal, 10 % no VIP.</summary>
    public static int RatePercent(Player p, DateTime nowUtc) => Ticket(p, nowUtc) is { } t ? t.TypeId == VipTicket ? 10 : 5 : 0;
    public static int CometOf(int ticket) => ticket == VipTicket ? VipComet : Comet;
    /// <summary>Quantidade da Premium Comet enquanto o premium vale (a referência dá uma bola virtual que não acaba).</summary>
    public const int CometStock = 999;

    public static bool IsTicket(int typeId) => typeId is NormalTicket or VipTicket;

    /// <summary>O ticket em vigor (null = não é premium).</summary>
    public static Item? Ticket(Player p, DateTime nowUtc)
    {
        Item? best = null;
        foreach (var it in p.Items.Values)
            if (IsTicket(it.TypeId) && it.Location == ItemLocation.Inventory && it.ExpiresAt > nowUtc && (best == null || it.ExpiresAt > best.ExpiresAt))
                best = it;
        return best;
    }

    public static bool IsActive(Player p, DateTime nowUtc) => Ticket(p, nowUtc) != null;

    /// <summary>Tickets vencidos (para tirar do inventário).</summary>
    public static List<Item> Expired(Player p, DateTime nowUtc)
    {
        var list = new List<Item>();
        foreach (var it in p.Items.Values)
            if (IsTicket(it.TypeId) && it.Location == ItemLocation.Inventory && it.ExpiresAt is { } end && end <= nowUtc) list.Add(it);
        return list;
    }
}
