namespace Pangya.Domain.Players;

/// <summary>
/// Troféus especiais (Match.iff: 0x2D.. troféus de sala/evento como "GM Event", 0x2E.. eventos e copas, 0x2F.. Grand
/// Prix). Ficam fora do inventário; o perfil mostra a lista (i32 id, u32 typeid, i32 quantidade).
/// </summary>
public static class SpecialTrophies
{
    public static bool IsSpecial(int typeId) => ((uint)typeId >> 24) is 0x2D or 0x2E or 0x2F;
    /// <summary>Troféu do Grand Prix (no JP vai numa lista própria do perfil).</summary>
    public static bool IsGrandPrix(int typeId) => ((uint)typeId >> 24) == 0x2F;
}

public interface ISpecialTrophyStore
{
    Task<List<(int TypeId, int Quantity)>> ListAsync(long account);
    Task AddAsync(long account, int typeId, int quantity);
}
