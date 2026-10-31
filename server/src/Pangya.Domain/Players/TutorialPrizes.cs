namespace Pangya.Domain.Players;

/// <summary>
/// Prêmios das missões do tutorial (cliente JP 983, comportamento da referência; docs/server/protocolo/SPEC-jp983-pratica.md §2):
/// por missão e por categoria completa (rookie 0x01..0x80, beginner 0x100..0x2000, advancer 0x10000..0x40000). Vão por
/// carta, só na primeira vez que a missão é concluída.
/// </summary>
public static class TutorialPrizes
{
    const int PangPouch = 0x1A000010;
    static readonly int[] Masks = [0xFF, 0x3F00, 0x70000];
    static readonly int[] FirstBit = [0, 8, 16];

    static readonly (int TypeId, int Qty)[][] Rookie =
    [
        [(0x1A000002, 3)], [(0x1800000B, 3)], [(0x18000025, 3)], [(0x18000005, 3)],
        [(0x18000004, 3)], [(0x1800000A, 3)], [(0x18000000, 3)], [(PangPouch, 1000)],
    ];
    static readonly (int TypeId, int Qty)[][] Beginner =
    [
        [(0x1A000002, 10)], [(0x18000028, 1)], [(0x18000006, 1)], [(0x1800000A, 3)], [(0x18000000, 4)], [(0x18000001, 3)],
    ];
    static readonly (int TypeId, int Qty)[][] AllDone =
    [
        [(0x1C000000, 1), (0x10000014, 1)],                       // rookie: caddie Papel + Lucky Air Knight Set
        [(0x18000027, 10), (PangPouch, 10_000)],
        [(0x18000000, 1), (PangPouch, 30_000)],
    ];

    /// <summary>Prêmios por ter concluído agora a missão bit da categoria (flags antes e depois). Vazio = nada.</summary>
    public static List<(int TypeId, int Qty)> For(int category, int before, int after)
    {
        var list = new List<(int, int)>();
        if (category is < 0 or > 2) return list;
        int added = after & ~before & Masks[category];
        for (int b = 0; b < 32; b++)
        {
            if ((added & (1 << b)) == 0) continue;
            int i = b - FirstBit[category];
            var table = category == 0 ? Rookie : category == 1 ? Beginner : [];
            if (i >= 0 && i < table.Length) list.AddRange(table[i]);
        }
        if (added != 0 && (after & Masks[category]) == Masks[category]) list.AddRange(AllDone[category]);
        return list;
    }
}
