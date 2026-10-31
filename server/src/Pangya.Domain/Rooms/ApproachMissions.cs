namespace Pangya.Domain.Rooms;

/// <summary>Missão do buraco do approach (o 0x14F do JP): nº do texto no cliente, caixas, tipo (1 todos, 2 individual), c1, c2.</summary>
public sealed record ApproachMission(byte No, byte Boxes, byte Type, int C1, int C2);

/// <summary>
/// Missões do approach (docs/server/protocolo/SPEC-jp983-eventos-drops.md §4): ~50 % por buraco, uma das que o servidor sabe
/// julgar só com a distância (décimos de jarda), o tempo restante e a posição. Quem cumpre ganha as caixas da missão
/// (somadas aos prêmios do buraco). Chip-in = distância 0.
/// </summary>
public static class ApproachMissions
{
    public const uint Out = uint.MaxValue;

    /// <summary>(nº, caixas, tipo, mínimo de jogadores, c1 máx).</summary>
    static readonly (byte No, byte Boxes, byte Type, int MinPlayers, int C1Max)[] Table =
    [
        (1, 1, 1, 2, 2), (3, 2, 2, 1, 0), (7, 1, 2, 3, 0), (11, 1, 1, 4, 2), (12, 2, 1, 4, 0), (14, 1, 2, 1, 5),
        (15, 1, 1, 3, 2), (16, 2, 1, 6, 0), (17, 1, 1, 4, 0), (18, 2, 1, 4, 5), (20, 1, 2, 3, 0), (22, 1, 1, 2, 0),
        (24, 1, 2, 3, 0), (25, 1, 2, 1, 2), (26, 1, 2, 3, 0),
    ];

    public static ApproachMission? Roll(int players, Random rng, int chancePercent = 50)
    {
        if (rng.Next(100) >= chancePercent) return null;
        var ok = new List<(byte, byte, byte, int, int)>();
        foreach (var m in Table) if (players >= m.MinPlayers) ok.Add(m);
        if (ok.Count == 0) return null;
        var (no, boxes, type, _, c1Max) = ok[rng.Next(ok.Count)];
        int c1 = no switch
        {
            7 => 1 + rng.Next(Math.Max(1, players)),
            14 => 1 + rng.Next(5),
            15 => 1 + rng.Next(Math.Max(1, Math.Min(players, 3))),
            18 => 1 + rng.Next(5),
            _ => c1Max > 0 ? rng.Next(c1Max) : 0,
        };
        int c2 = no switch { 15 => rng.Next(2), 17 => players < 15 ? 150 : 250, 18 => rng.Next(9), _ => 0 };
        return new ApproachMission(no, boxes, type, c1, c2);
    }

    /// <summary>
    /// Quem cumpriu e quantas caixas: entrada = (guid, distância em décimos de jarda ou Out, tempo restante ms, posição 1..n
    /// ou 255). Devolve guid -> caixas.
    /// </summary>
    public static Dictionary<uint, int> Judge(ApproachMission m, IReadOnlyList<(uint Guid, uint Dist, uint Time, int Rank)> e)
    {
        var win = new Dictionary<uint, int>();
        var valid = new List<(uint Guid, uint Dist, uint Time, int Rank)>();
        foreach (var x in e) if (x.Dist != Out) valid.Add(x);
        void All() { foreach (var x in e) if (x.Dist != Out) win[x.Guid] = m.Boxes; }
        static int Yards(uint d) => (int)(d / 10);
        (uint Guid, uint Dist, uint Time, int Rank)? AtRank(int r)
        {
            foreach (var x in valid) if (x.Rank == r) return x;
            return null;
        }
        int chips = 0;
        foreach (var x in valid) if (x.Dist == 0) chips++;
        switch (m.No)
        {
            case 1 when AtRank(1) is { } first && Yards(first.Dist) % 2 == m.C1: All(); break;
            case 3: foreach (var x in valid) if (x.Dist == 0) win[x.Guid] = m.Boxes; break;
            case 7 when AtRank(m.C1) is { } r: win[r.Guid] = m.Boxes; break;
            case 11:
            {
                int even = 0, odd = 0;
                foreach (var x in valid) if (Yards(x.Dist) % 2 == 0) even++; else odd++;
                if (m.C1 == 0 ? even > odd : odd > even) All();
                break;
            }
            case 12 when chips * 2 > e.Count: All(); break;
            case 14:
                foreach (var x in valid)
                    if (x.Dist < m.C1 * 10) win[x.Guid] = x.Dist <= 10 ? 3 : x.Dist <= 30 ? 2 : 1;
                break;
            case 15 when AtRank(m.C1) is { } r15 && Yards(r15.Dist) % 2 == m.C2: All(); break;
            case 16 when chips >= 4: All(); break;
            case 17:
            {
                long sum = 0;
                foreach (var x in valid) sum += Yards(x.Dist);
                if (valid.Count == e.Count && sum < m.C2) All();
                break;
            }
            case 18: foreach (var x in valid) if (x.Dist == (uint)(m.C1 * 10 + m.C2)) win[x.Guid] = m.Boxes; break;
            case 20:
            {
                (uint Guid, uint Dist, uint Time, int Rank)? far = null;
                foreach (var x in valid) if (far == null || x.Dist > far.Value.Dist) far = x;
                if (far is { } f) win[f.Guid] = m.Boxes;
                break;
            }
            case 22 when AtRank(1) is { } r22 && r22.Time < 10_000: All(); break;
            case 24 or 26:
            {
                (uint Guid, uint Dist, uint Time, int Rank)? pick = null;
                foreach (var x in valid)
                    if (pick == null || (m.No == 24 ? x.Time > pick.Value.Time : x.Time < pick.Value.Time)) pick = x;
                if (pick is { } pk) win[pk.Guid] = m.Boxes;
                break;
            }
            case 25: foreach (var x in valid) if (Yards(x.Dist) % 2 == m.C1) win[x.Guid] = m.Boxes; break;
        }
        return win;
    }
}
