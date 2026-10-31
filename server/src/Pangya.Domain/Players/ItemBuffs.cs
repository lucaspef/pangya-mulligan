namespace Pangya.Domain.Players;

/// <summary>Buff por tempo (Sweet Yam, balas): EXP e pang em % no fim da partida e quantos minutos dura.</summary>
public sealed record BuffInfo(int TypeId, int Kind, int ExpPercent, int PangPercent, int Minutes);

/// <summary>Tabela de buffs do cliente (JP: TimeLimitItem.iff; KR: fixa no cliente).</summary>
public interface IItemBuffData
{
    BuffInfo? Buff(int typeId);
}

/// <summary>
/// Buffs por tempo (docs/server/protocolo/SPEC-itens-restantes.md §3, SPEC-jp983-efeitos.md §5): usar tira 1 do item e liga o
/// buff (o mesmo item ativo soma o prazo; Sweet Yam e Golden Sweet Yam não andam juntos). Cada buff é um objeto com
/// location = Buff e prazo. Também a recarga de cometa (§4/§8): o pacote soma de Min a Max (≥ 10) numa pilha de bola.
/// </summary>
public sealed class ItemBuffService(IPlayerStore store, IItemBuffData? data, IGameData game, Random? random = null)
{
    public const int SweetYam = 0x1A0000B3, GoldenSweetYam = 0x1A0000B4, RefillPack = 0x1A000105;
    /// <summary>Os do KR 645 (o cliente tem a tabela fixa).</summary>
    public static readonly IItemBuffData KrTable = new Fixed();
    readonly Random rng = random ?? Random.Shared;

    sealed class Fixed : IItemBuffData
    {
        public BuffInfo? Buff(int typeId) => typeId switch
        {
            SweetYam => new(SweetYam, 1, 5, 0, 120),
            GoldenSweetYam => new(GoldenSweetYam, 1, 50, 0, 120),
            _ => null,
        };
    }

    IItemBuffData Data => data ?? KrTable;

    public BuffInfo? Info(int typeId) => Data.Buff(typeId);

    /// <summary>Buffs em vigor.</summary>
    public static List<Item> Active(Player p, DateTime nowUtc)
    {
        var list = new List<Item>();
        foreach (var it in p.Items.Values)
            if (it.Location == ItemLocation.Buff && it.ExpiresAt > nowUtc) list.Add(it);
        list.Sort(static (a, b) => a.Id.CompareTo(b.Id));
        return list;
    }

    /// <summary>% de EXP e de pang somados dos buffs em vigor.</summary>
    public (int Exp, int Pang) Rates(Player p, DateTime nowUtc)
    {
        int exp = 0, pang = 0;
        foreach (var it in Active(p, nowUtc))
            if (Info(it.TypeId) is { } b) { exp += b.ExpPercent; pang += b.PangPercent; }
        return (exp, pang);
    }

    /// <summary>Usa o item de buff: devolve o buff (objeto com o prazo) ou null (recusado).</summary>
    public async Task<Item?> UseAsync(Player p, int typeId)
    {
        if (Info(typeId) is not { } info || p.FindType(typeId) is not { Quantity: > 0 } stack
            || PlayerActions.Free(p, stack) < 1) return null;
        var now = DateTime.UtcNow;
        Item? same = null;
        foreach (var it in Active(p, now))
        {
            if (it.TypeId == typeId) same = it;
            else if (typeId is SweetYam or GoldenSweetYam && it.TypeId is SweetYam or GoldenSweetYam) return null;   // um ou outro
        }
        var ch = new PlayerChanges();
        var st = stack.Clone();
        st.Quantity--;
        if (st.Quantity == 0) ch.Removed.Add(st.Id); else ch.Updated.Add(st);
        Item buff;
        if (same != null)
        {
            buff = same.Clone();
            buff.ExpiresAt = same.ExpiresAt!.Value.AddMinutes(info.Minutes);
            ch.Updated.Add(buff);
        }
        else
        {
            buff = new Item { Id = (await store.NewIdsAsync(1))[0], TypeId = typeId, Location = ItemLocation.Buff, ExpiresAt = now.AddMinutes(info.Minutes) };
            buff.Attrs["start"] = now;
            ch.Added.Add(buff);
        }
        await store.ApplyAsync(p.AccountId, ch);
        if (st.Quantity == 0) p.Items.Remove(st.Id); else p.Items[st.Id] = st;
        p.Items[buff.Id] = buff;
        return buff;
    }

    /// <summary>Recarga de cometa: o pacote (0x1A000105) soma de min a max (≥ 10) na pilha da bola. Devolve o total novo.</summary>
    public async Task<int?> RefillAsync(Player p, int pack, int ball, int min, int max)
    {
        if (pack != RefillPack || Item.GroupOf(ball) != ItemGroup.Ball || ball is Item.BasicBall or 0x14000004) return null;
        if (p.FindType(pack) is not { Quantity: > 0 } pk || PlayerActions.Free(p, pk) < 1) return null;
        if (p.FindType(ball) is not { Quantity: > 0 } bl || game.GetShopItem(ball) == null) return null;
        min = Math.Max(10, min);
        int add = min + rng.Next(Math.Max(0, max - min) + 1);
        int total = Math.Min(bl.Quantity + add, short.MaxValue);
        if (total - bl.Quantity < 10) return null;
        var ch = new PlayerChanges();
        var pe = pk.Clone();
        pe.Quantity--;
        if (pe.Quantity == 0) ch.Removed.Add(pe.Id); else ch.Updated.Add(pe);
        var be = bl.Clone();
        be.Quantity = total;
        ch.Updated.Add(be);
        await store.ApplyAsync(p.AccountId, ch);
        if (pe.Quantity == 0) p.Items.Remove(pe.Id); else p.Items[pe.Id] = pe;
        p.Items[be.Id] = be;
        return total;
    }
}
