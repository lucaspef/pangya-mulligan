using Pangya.Core.Config;
using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>Item que caiu no copo (o 0xCC do JP: typeid, mapa, buraco, quantidade, tipo 1 = quantidade normal, 2 = bolsas de 500 pang).</summary>
public readonly record struct HoleDrop(int TypeId, byte Course, byte Hole, short Quantity, byte Kind = HoleDrop.Normal)
{
    public const byte Normal = 1, PangPouch500 = 2;
}

/// <summary>
/// Drop por buraco (docs/server/protocolo/SPEC-jp983-eventos-drops.md §3): ao acertar o buraco, sorteios da config (itens com
/// chance em ‰, mana de artefato, ticket do Special Shuffle) e, no último buraco de partidas de 9/18 fora do GP, o ticket
/// do Grand Prix até 50. O item aparece no copo na hora e entra no inventário no fim da partida.
/// </summary>
public static class HoleDrops
{
    public const int GrandPrixTicket = 0x1A000264, SpecialShuffleTicket = 0x1A0000F7, PangTid = 0x1A000010;
    /// <summary>Pang de cada bolsa do artefato (tipo 2 do 0xCC).</summary>
    public const int PangPerPouch = 500;
    /// <summary>Mana dos artefatos (Item.iff ItemType 4).</summary>
    static readonly int[] Mana =
    [
        0x1A0001A5, 0x1A0001A7, 0x1A0001A9, 0x1A0001AB, 0x1A0001AD, 0x1A0001AF, 0x1A0001B1, 0x1A0001B3, 0x1A0001B5,
        0x1A0001B7, 0x1A0001B9, 0x1A0001BB, 0x1A0001BD, 0x1A0001BF, 0x1A0001C1, 0x1A0001C3, 0x1A0001F9, 0x1A0001FB,
    ];

    /// <summary>
    /// Sorteia os drops deste acerto. seq = quantos buracos o jogador já fechou (1 = o primeiro); holes = buracos da partida;
    /// artifact = artefato da sala; stars = dificuldade do mapa (itens com MinStars maior não caem).
    /// </summary>
    public static List<HoleDrop> Roll(HoleDropsConfig cfg, Player p, byte course, byte hole, int seq, int holes, bool grandPrix,
        Random rng, int artifact = 0, int stars = 1)
    {
        var list = new List<HoleDrop>();
        if (!cfg.Enabled) return list;
        bool last = seq >= holes;
        int extra = Artifacts.DropExtra(artifact);                            // Rainbow Magic Hat da sala: +1 item do copo
        if (last && holes >= 18 && Artifacts.PangPouches(artifact, rng) is > 0 and var pouches)   // artefato de pang (18H)
            list.Add(new HoleDrop(PangTid, course, hole, (short)pouches, HoleDrop.PangPouch500));
        foreach (var it in cfg.Items)
        {
            if (it.TypeId == 0 || it.LastHoleOnly && !last || stars < it.MinStars) continue;
            if (rng.Next(1000) < it.PerMille) list.Add(new HoleDrop(it.TypeId, course, hole, (short)(Math.Max(1, it.Quantity) + extra)));
        }
        if (cfg.ManaPerMille > 0 && rng.Next(1000) < cfg.ManaPerMille)
            list.Add(new HoleDrop(Mana[rng.Next(Mana.Length)], course, hole, 1));
        if (cfg.SpecialShufflePerMille > 0 && rng.Next(1000) < cfg.SpecialShufflePerMille)
            list.Add(new HoleDrop(SpecialShuffleTicket, course, hole, (short)(1 + extra)));
        if (last && !grandPrix && cfg.GrandPrixTicketMax > 0)                  // ticket do GP: 9/18 buracos sempre; menos, 1 em 3
        {
            int have = p.FindType(GrandPrixTicket)?.Quantity ?? 0;
            int n = holes >= 18 ? 2 : holes >= 9 ? 1 : rng.Next(3) == 0 ? 1 : 0;
            n = Math.Min(n, cfg.GrandPrixTicketMax - have);
            if (n > 0) list.Add(new HoleDrop(GrandPrixTicket, course, hole, (short)n));
        }
        return list;
    }
}
