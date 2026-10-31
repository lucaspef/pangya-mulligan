using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Troféus do My Room (docs/server/protocolo/SPEC-jp983-game.md §Troféus): no login, depois do 0x44 (que apaga as listas), 0x169
/// normais, 0xB4 especiais (0x2D/0x2E) e 0x25D Grand Prix (0x2F); na sessão, +1 especial (0x1FA) e +qtd GP (0x216).
/// O banco não tem id por troféu: o typeid faz de guid, estável entre o login e o 0x1FA (o cliente só soma com o mesmo guid).
/// </summary>
public sealed partial class JpSystems
{
    const ushort SNormalTrophies = 0x169, SSpecialTrophies = 0xB4, SGpTrophies = 0x25D;
    internal const ushort SSpecialWon = 0x1FA;
    /// <summary>Temporada atual: é a lista que o My Room mostra (0 no 0x25D vai para uma lista que ele não lê).</summary>
    const byte CurrentSeason = 5;
    const int MaxSpecial = byte.MaxValue;

    /// <summary>0x169 tipo, u16[13][3]; 0xB4 tipo, u8 0, u8 n, n × 12; 0x25D tipo, u32 n, u32 n, n × 12 (guid, typeid, qtd).</summary>
    async Task SendTrophiesAsync()
    {
        conn.SendTranslated(new PacketWriter(SNormalTrophies, 2 + JpStructs.TrophySize).U8(CurrentSeason).Trophies(kr.Player));
        var all = ctx.SpecialTrophies is { } store ? await store.ListAsync(kr.Player.AccountId) : [];
        var special = all.FindAll(t => !SpecialTrophies.IsGrandPrix(t.TypeId));
        var gp = all.FindAll(t => SpecialTrophies.IsGrandPrix(t.TypeId));
        if (special.Count > MaxSpecial) special.RemoveRange(MaxSpecial, special.Count - MaxSpecial);    // n é u8
        conn.SendTranslated(TrophyList(new PacketWriter(SSpecialTrophies, 8 + special.Count * 12).U8(CurrentSeason).U8(0).U8((byte)special.Count), special));
        conn.SendTranslated(TrophyList(new PacketWriter(SGpTrophies, 16 + gp.Count * 12).U8(CurrentSeason).U32((uint)gp.Count).U32((uint)gp.Count), gp));
    }

    static PacketWriter TrophyList(PacketWriter w, List<(int TypeId, int Quantity)> list)
    {
        foreach (var (tid, qty) in list) w.I32(tid).U32((uint)tid).I32(qty);
        return w;
    }

    /// <summary>Troféu já gravado entra na lista do cliente sem novo login: GP pelo 0x216, os outros um 0x1FA por unidade.</summary>
    public void TrophyWon(int tid, int qty)
    {
        if (SpecialTrophies.IsGrandPrix(tid)) conn.SendTranslated(ItemsChanged([new ItemChange(tid, tid, 0, qty)]));   // id != 0: o cliente recusa 0
        else for (int i = 0; i < qty; i++) conn.SendTranslated(new PacketWriter(SSpecialWon).U32((uint)tid).I32(tid));
    }
}
