using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Domain.Achievements;

/// <summary>
/// Contadores de conquista (CounterItem.iff, typeids 0x6C4000xx; docs/server/protocolo/SPEC-jp983-conquistas.md §3) e o saco de
/// cada evento. Os typeids são dados do cliente: num cliente sem conquistas o saco não vai a lugar nenhum.
/// </summary>
public static class AchievementCounters
{
    public const int NormalGame = 0x6C400002, GameComplete = 0x6C400004, HolesPlayed = 0x6C400005, HoleInOne = 0x6C400006,
        Albatross = 0x6C400007, Pangya = 0x6C40000B, Score = 0x6C40000C, PangEarned = 0x6C40000D, Versus = 0x6C40001D,
        MatchMode = 0x6C40001E, Tourney = 0x6C40001F, PracticeRepeat = 0x6C40003D, PracticeCourse = 0x6C40003E,
        PapelPlay = 0x6C40004A, Ob = 0x6C40004C, Bunker = 0x6C40004E, Shots = 0x6C400055, Yards = 0x6C400056,
        BestDrive = 0x6C400057, BestChipIn = 0x6C400058, BestLongPutt = 0x6C400077, PapelRare = 0x6C400081,
        MagicBox = 0x6C400082, RoomMaster = 0x6C400098, LoginDay = 0x6C4000A0, MemorialNormal = 0x6C4000B2,
        MemorialSpecial = 0x6C4000B3, MemorialRareHigh = 0x6C4000B4, MemorialRare = 0x6C4000B5, ShortGame = 0x6C4000BB,
        CharacterStatUp = 0x6C400084, CharacterStatDown = 0x6C400085, CharacterCardEquip = 0x6C400087,
        CharacterCardRemove = 0x6C400088, Lolo = 0x6C400089, CharacterMastery = 0x6C4000C3;

    static readonly Dictionary<int, int> Characters = new()
    {
        [0x04000000] = 0x6C40000F, [0x04000001] = 0x6C400010, [0x04000002] = 0x6C400011, [0x04000003] = 0x6C400012,
        [0x04000004] = 0x6C400013, [0x04000005] = 0x6C400014, [0x04000006] = 0x6C400015, [0x04000007] = 0x6C400016,
        [0x04000008] = 0x6C400017, [0x04000009] = 0x6C400018, [0x0400000A] = 0x6C400040, [0x0400000B] = 0x6C4000B9,
        [0x0400000C] = 0x6C4000BA, [0x0400000E] = 0x6C4000C4,
    };

    static readonly Dictionary<int, int> Caddies = new()
    {
        [0x1C000000] = 0x6C400019, [0x1C000001] = 0x6C40001C, [0x1C000002] = 0x6C40001B, [0x1C000003] = 0x6C400042,
        [0x1C000004] = 0x6C400043, [0x1C000005] = 0x6C400041, [0x1C000006] = 0x6C400044, [0x1C000007] = 0x6C40001A,
        [0x1C000010] = 0x6C40001C, [0x1C000011] = 0x6C40001B, [0x1C000012] = 0x6C400042, [0x1C000013] = 0x6C400043,
        [0x1C000014] = 0x6C400041, [0x1C000015] = 0x6C400044, [0x1C000016] = 0x6C40001A, [0x1C00001E] = 0x6C400045,
    };

    static readonly Dictionary<int, int> Mascots = new()
    {
        [0x40000000] = 0x6C400049, [0x40000001] = 0x6C400048, [0x40000002] = 0x6C400047, [0x40000003] = 0x6C400046,
    };

    /// <summary>Contador do campo (o 12 não tem; 17 = Special Shuffle Course, 21 = Mystic Ruins).</summary>
    static readonly Dictionary<int, int> Courses = new()
    {
        [0] = 0x6C400020, [1] = 0x6C400021, [2] = 0x6C400022, [3] = 0x6C400023, [4] = 0x6C400024, [5] = 0x6C400025,
        [6] = 0x6C400026, [7] = 0x6C400027, [8] = 0x6C400028, [9] = 0x6C400029, [10] = 0x6C40002A, [11] = 0x6C40002B,
        [13] = 0x6C40002C, [14] = 0x6C40002D, [15] = 0x6C40002E, [16] = 0x6C40002F, [17] = 0x6C40006D, [18] = 0x6C400031,
        [19] = 0x6C400030, [20] = 0x6C4000A1, [21] = 0x6C4000C5,
    };

    static readonly Dictionary<int, int> HoleCounts = new() { [3] = 0x6C400069, [6] = 0x6C40006A, [9] = 0x6C40006B, [18] = 0x6C40006C };

    /// <summary>O que vale para uma partida inteira (avaliado no fim, como a referência).</summary>
    public sealed record GameEnd(GameMode Mode, bool Practice, byte RepeatHole, byte Course, int Holes, bool Master, bool Finished,
        GameStats? Stats, long Pang, (int Course, int Score)? Result, bool Shuffle = false, bool Zodiac = false);

    /// <summary>Saco do fim da partida: início (personagem, caddie, mascote, campo, buracos, modo) + totais da partida.</summary>
    public static CounterBag ForGame(Player p, GameEnd g)
    {
        var bag = new CounterBag();
        if (g.Practice)                                                       // prática: só buracos e o tipo (referência)
        {
            bag.Add(HolesPlayed, g.Holes).Add(g.RepeatHole > 0 ? PracticeRepeat : PracticeCourse);
            return bag;
        }
        bag.Add(NormalGame);
        if (p.Character is { } ch && Characters.TryGetValue(ch.TypeId, out var cc)) bag.Add(cc);
        if (p.Find(p.Equip.CaddieId) is { } cd && Caddies.TryGetValue(cd.TypeId, out var cdc)) bag.Add(cdc);
        if (p.Find(p.Equip.MascotId) is { } ms && Mascots.TryGetValue(ms.TypeId, out var mc)) bag.Add(mc);
        if (Courses.TryGetValue(g.Course & 0x7F, out var course)) bag.Add(course);
        if (HoleCounts.TryGetValue(g.Holes, out var hc)) bag.Add(hc);
        if (g.Master) bag.Add(RoomMaster);
        bag.Add(g.Mode switch
        {
            GameMode.Stroke => Versus,
            GameMode.Match => MatchMode,
            GameMode.Tournament or (GameMode)14 => Tourney,
            _ => 0,
        });
        if (!g.Finished) return bag;
        bag.Add(GameComplete);
        if (g.Stats is { } s)
        {
            bag.Add(HolesPlayed, s.Hole).Add(Shots, s.Drive + s.Putt).Add(Yards, s.Distance).Add(Pangya, s.Pangya).Add(Ob, s.OB)
                .Add(Bunker, s.Bunker).Add(HoleInOne, s.HoleInOne).Add(Albatross, s.Albatross)
                .Add(BestDrive, (long)s.Longest).Add(BestChipIn, (long)s.LongestChipIn).Add(BestLongPutt, (long)s.LongestPuttIn);
        }
        if (g.Pang > 0) bag.Add(PangEarned, g.Pang);
        if (g.Result is { } r) bag.Values[Score] = r.Score;                    // pode ser negativo ("no máximo")
        return bag;
    }
}
