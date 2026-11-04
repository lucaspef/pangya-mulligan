namespace Pangya.Domain.Players;

/// <summary>
/// Anéis que se gastam jogando. O Desc.iff diz "1 por hora" ou "1 por buraco"; os lendários não têm custo. O servidor
/// conta as horas de partida e guarda os segundos que sobram no item, para não perder o resto entre partidas.
/// </summary>
public static class Rings
{
    const string SecondsKey = "ring_seconds";
    const long Hour = 3600;

    public static bool PerHour(int tid) => tid is >= 0x70000000 and <= 0x70000004 or 0x70000006;
    public static bool PerHole(int tid) => tid is 0x70000005 or 0x70000085;

    /// <summary>Unidades a gastar; atualiza os segundos acumulados no item.</summary>
    public static int Spend(Item ring, long seconds, int holes)
    {
        if (PerHole(ring.TypeId)) return Math.Max(holes, 0);
        if (!PerHour(ring.TypeId)) return 0;
        long total = (ring.Attrs[SecondsKey]?.GetValue<long>() ?? 0) + Math.Max(seconds, 0);
        ring.Attrs[SecondsKey] = total % Hour;
        return (int)(total / Hour);
    }
}
