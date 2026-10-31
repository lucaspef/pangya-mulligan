using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>
/// Artefatos da sala (JP 983; ArtifactManaInfo.iff e referência GameBase.initArtefact/CourseManager): o dono escolhe ao
/// criar ou mudar a sala, precisa ter o artefato e a mana dele (typeid + 1), e o efeito vale para a partida inteira;
/// no fim, 1 mana é gasta. Os itens de regra (vento de 1 m, Super Wind...) são regras do Grand Prix, não artefatos.
/// </summary>
public static class Artifacts
{
    public const int OrchidBlossom = 0x1A0001A4, PenneAbacus = 0x1A0001A6, TitanWindmill = 0x1A0001A8,
        LuminescentCoral = 0x1A0001AA, TropicalTree = 0x1A0001AC, TwinLunarMirror = 0x1A0001AE, MachinaWrench = 0x1A0001B0,
        SilviaManual = 0x1A0001B2, WickedBroomstick = 0x1A0001B4, TeoriteOre = 0x1A0001B6, RednoseWizberry = 0x1A0001B8,
        MaganiFlower = 0x1A0001BA, RogerK = 0x1A0001BC, RainbowMagicHat = 0x1A0001BE, ScrollOfFourGods = 0x1A0001C0,
        ZephyrTotem = 0x1A0001C2, DragonOrb = 0x1A0001F8, FrozenFlame = 0x1A0001FA;

    static readonly HashSet<int> All =
    [
        OrchidBlossom, PenneAbacus, TitanWindmill, LuminescentCoral, TropicalTree, TwinLunarMirror, MachinaWrench, SilviaManual,
        WickedBroomstick, TeoriteOre, RednoseWizberry, MaganiFlower, RogerK, RainbowMagicHat, ScrollOfFourGods, ZephyrTotem,
        DragonOrb, FrozenFlame,
    ];

    public static bool IsArtifact(int typeId) => All.Contains(typeId);
    public static int ManaOf(int artifact) => artifact + 1;

    /// <summary>O dono pode usar: é artefato, tem o item e ao menos 1 mana (fora da loja pessoal).</summary>
    public static bool CanUse(Player p, int artifact) =>
        IsArtifact(artifact) && p.FindType(artifact) is { Location: ItemLocation.Inventory }
        && p.FindType(ManaOf(artifact)) is { Location: ItemLocation.Inventory } mana && PlayerActions.Free(p, mana) > 0;

    /// <summary>EXP a mais em % (Luminescent Coral +2 … Silvia Manual +10).</summary>
    public static int ExpPercent(int artifact) => artifact switch
    {
        LuminescentCoral => 2, TropicalTree => 4, TwinLunarMirror => 6, MachinaWrench => 8, SilviaManual => 10, _ => 0,
    };

    /// <summary>Chance de chuva a mais em ‰ (referência: +5/+10/+20 no peso da chuva, que pesa ~1/4 do tempo bom).</summary>
    public static int RainPerMille(int artifact) => artifact switch
    {
        ScrollOfFourGods => 50, ZephyrTotem => 100, DragonOrb => 200, _ => 0,
    };

    /// <summary>
    /// Vento do buraco (0 = 1 m) com o artefato: Orchid Blossom 1–8 m, Penne Abacus só metros ímpares, Titan Windmill só
    /// pares (referência shuffleWind); sem artefato, 0..max.
    /// </summary>
    public static byte Wind(int artifact, Random rng, int max = 8)
    {
        if (artifact == OrchidBlossom) max = Math.Min(max, 7);
        int w;
        do w = rng.Next(max + 1);
        while (artifact == PenneAbacus && (w + 1) % 2 == 0 || artifact == TitanWindmill && (w + 1) % 2 == 1);
        return (byte)w;
    }

    /// <summary>Rainbow Magic Hat: +1 em cada item do copo.</summary>
    public static int DropExtra(int artifact) => artifact == RainbowMagicHat ? 1 : 0;

    /// <summary>Frozen Flame: os itens usados na partida não são gastos nem desequipados.</summary>
    public static bool KeepsItems(int artifact) => artifact == FrozenFlame;

    /// <summary>
    /// Bolsas de pang (× 500) no último buraco de partida de 18 (referência requestInitDrop): Wicked Broomstick 1, Teorite
    /// Ore 2, Rednose Wizberry 3, Magani Flower 6, Roger K 1..6.
    /// </summary>
    public static int PangPouches(int artifact, Random rng) => artifact switch
    {
        WickedBroomstick => 1, TeoriteOre => 2, RednoseWizberry => 3, MaganiFlower => 6, RogerK => 1 + rng.Next(6), _ => 0,
    };
}
