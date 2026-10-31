using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Structs do cliente JP 983 escritas campo a campo (docs/server/protocolo/SPEC-jp983-game.md). Tamanhos conferidos contra as
/// leituras do exe (docs/server/jp983-protocolo/pacotes_recebe_983.tsv) e contra a referência JP da comunidade (pangya_game_st.cs);
/// os testes conferem cada tamanho.
/// </summary>
public static class JpStructs
{
    public const int MemberInfoSize = 299, UserInfoSize = 265, TrophySize = 78, UserEquipSize = 116, MapStatSize = 43,
        EquipedItemSize = 628, CharacterSize = 513, CaddieSize = 25, ClubSetSize = 28, MascotSize = 62, ItemSize = 196,
        ChannelSize = 85;
    /// <summary>Mapas por tabela de estatística (o KR tem 20).</summary>
    public const int MapCount = 22;
    /// <summary>Tabelas de mapa no 0x44: normal, natural, grand prix e normal das 9 temporadas.</summary>
    public const int MapTables = 12;
    /// <summary>Bloco que o 0x44 lê de uma vez (DB(0x31c6)): MemberInfoEx, uid, UserInfo, troféus, equipamento, mapas, equipados.</summary>
    public const int PlayerBlockSize = MemberInfoSize + 4 + UserInfoSize + TrophySize + UserEquipSize + MapTables * MapCount * MapStatSize
        + EquipedItemSize;

    /// <summary>Melhor placar "sem recorde".</summary>
    const byte NoScore = CourseRecord.NoRecord;

    /// <summary>
    /// Typeids que o cliente conhece (Jp983GameData.Exists); null = todos. O que ele não conhece vai como 0 (desequipado).
    /// </summary>
    public delegate bool KnownType(int typeId);

    static bool Known(KnownType? known, int typeId) => known == null || known(typeId);

    /// <summary>Id do item equipado, se existe no inventário e o cliente conhece o typeid; senão 0.</summary>
    static int EquippedId(Player p, int id, KnownType? known) => p.Find(id) is { } it && Known(known, it.TypeId) ? id : 0;

    /// <summary>SYSTEMTIME (16 bytes); null = zeros.</summary>
    public static PacketWriter Time(this PacketWriter w, DateTime? t)
    {
        if (t is not { } v) return w.Zeros(16);
        return w.U16((ushort)v.Year).U16((ushort)v.Month).U16((ushort)v.DayOfWeek).U16((ushort)v.Day)
            .U16((ushort)v.Hour).U16((ushort)v.Minute).U16((ushort)v.Second).U16((ushort)v.Millisecond);
    }

    /// <summary>
    /// MemberInfoEx (299): u16 sala, char[22] id, char[22] nick, char[17] guilda, char[12] emblema, char[35] comentário,
    /// u32 escola, u32 capacidade (4 = GM), u32 galeria, i32 oid (chave do jogador nas salas, como o dwGuid do KR),
    /// u32[3] ranking, u32 guilda, u32 nº do emblema, u8 estado, u16 login (2 = não é o primeiro), u16[3] papel shop,
    /// u32 pontos de evento, u64 bloqueios, u32 canalização, char[128] id de exibição.
    /// </summary>
    /// <summary>
    /// Papel Shop sem limite por dia (referência: PlayerPapelShopInfo, restantes/usadas/limite = 0xFFFF). Com 0 o cliente
    /// acha que as jogadas acabaram: mostra o aviso de "Bonus Turns" e fecha a loja.
    /// </summary>
    public const ushort PapelUnlimited = 0xFFFF;

    public static PacketWriter MemberInfo(this PacketWriter w, Player p, ushort roomIndex = 0xFFFF, uint? oid = null)
    {
        var g = p.Guild;
        return w.U16(roomIndex).Fixed(p.Login, 22).Fixed(p.Nickname, 22).Fixed(g?.Name ?? "", 17).Fixed(g?.Mark ?? "", 12).Zeros(35)
            .U32((uint)p.School).U32((uint)(p.IdentityFlags | (Domain.Players.Premium.IsActive(p, DateTime.UtcNow) ? Domain.Players.Premium.Capability : 0))).U32(0).U32(oid ?? (uint)p.AccountId).Zeros(12).U32((uint)(g?.Id ?? 0))
            .U32((uint)Domain.Guilds.GuildTag.MarkNo(g?.Mark ?? "")).U8(GuildState(g)).U16(2).U16(PapelUnlimited).U16(PapelUnlimited).U16(PapelUnlimited).U32(0).U64(0).U32(0).Zeros(128);
    }

    /// <summary>Estado do MemberInfo: bits 0–1 = cargo no club (1 mestre, 2 submestre: só eles veem a gestão no cliente).</summary>
    static byte GuildState(Domain.Guilds.GuildTag? g) => g?.Class switch
    {
        Domain.Guilds.GuildClass.Master => 1, Domain.Guilds.GuildClass.SubMaster => 2, _ => 0,
    };

    /// <summary>UserInfo (265): estatísticas, nível, EXP e pang (ordem do JP: hole in one e albatross com 4 bytes).</summary>
    public static PacketWriter UserInfo(this PacketWriter w, Player p)
    {
        var t = p.Stats;
        static int I(long v) => (int)Math.Clamp(v, int.MinValue, int.MaxValue);
        w.I32(I(t.Drive)).I32(I(t.Putt)).I32(0).I32(I(t.ShotTime)).F32(t.Longest).I32(I(t.Pangya)).I32(I(t.TimeOut)).I32(I(t.OB))
            .I32(I(t.Distance)).I32(I(t.Hole)).I32(0).I32(I(t.HoleInOne)).I16((short)Math.Clamp(t.Bunker, 0, short.MaxValue))
            .I32(I(t.Fairway)).I32(I(t.Albatross)).I32(I(t.HoleIn)).I32(I(t.PuttIn)).F32(t.LongestPuttIn).F32(t.LongestChipIn)
            .I32(Math.Max(p.Exp, 0)).U8((byte)p.Level).U64((ulong)Math.Max(p.Pang, 0)).I32(I(t.TotalScore));
        for (int i = 0; i < 5; i++) w.U8(NoScore);                // melhor placar por estrela: 127 = sem recorde
        w.U8(0).Zeros(5 * 8).I64(0)                                             // evento, melhor pang por estrela, soma
            .I32(I(t.Games)).Zeros(4 * 3)                                       // jogos, equipe (buracos, vitórias, jogos)
            .Zeros(4 * 5).Zeros(4 * 2).I32(I(t.Quits))                          // ladder; combo, combo máximo, abandonos
            .Zeros(8 + 4 * 6).I16(0).I32(0)                                     // pang battle; evento; desconexões
            .Zeros(4 * 6)                                                       // medalhas
            .I32(0).I32(0).I16(0);                                              // escola antiga, jogos na temporada, ?
        return w;
    }

    /// <summary>Troféus (78): u16[13][3], faixa do Match.iff × ouro/prata/bronze (igual ao KR).</summary>
    public static PacketWriter Trophies(this PacketWriter w, Player p)
    {
        var t = p.Stats.Trophies;
        for (int i = 0; i < Trophy.Ranks * Trophy.Kinds; i++) w.U16(i < t.Length ? (ushort)Math.Clamp(t[i], 0, ushort.MaxValue) : (ushort)0);
        return w;
    }

    /// <summary>
    /// UserEquip (116): i32 caddie, i32 personagem, i32 club set, u32 bola, u32[10] itens, u32[6] id das skins,
    /// u32[6] typeid das skins, i32 mascote, u32[2] pôsteres.
    /// </summary>
    public static PacketWriter UserEquip(this PacketWriter w, Player p, KnownType? known = null)
    {
        var e = p.Equip;
        int ball = e.BallTypeId != 0 && Known(known, e.BallTypeId) ? e.BallTypeId : Item.BasicBall;
        w.I32(EquippedId(p, e.CaddieId, known)).I32(EquippedCharacter(p, known)?.Id ?? 0).I32(EquippedId(p, e.ClubSetId, known))
            .U32((uint)ball);
        for (int i = 0; i < 10; i++) w.U32(i < e.ItemSlots.Length && Known(known, e.ItemSlots[i]) ? (uint)e.ItemSlots[i] : 0);
        w.Zeros(6 * 4);
        for (int i = 0; i < 6; i++) w.U32(i < e.SkinTypeIds.Length && Known(known, e.SkinTypeIds[i]) ? (uint)e.SkinTypeIds[i] : 0);
        return w.I32(EquippedId(p, e.MascotId, known)).Zeros(8);
    }

    /// <summary>Personagem equipado; se o cliente não o conhece, o primeiro que ele conhece.</summary>
    public static Item? EquippedCharacter(Player p, KnownType? known)
    {
        if (p.Character is { } c && Known(known, c.TypeId)) return c;
        foreach (var it in p.OfGroup(ItemGroup.Character))
            if (Known(known, it.TypeId)) return it;
        return null;
    }

    /// <summary>
    /// MapStatistics (43): i8 curso, u32 tacadas, u32 putts, u32 buracos, u32 fairways, u32 hole in, u32 putt in,
    /// i32 placar total, i8 melhor placar (127 = sem), u64 melhor pang, u32 personagem do recorde, u8 evento.
    /// </summary>
    public static PacketWriter MapStat(this PacketWriter w, Player? p, int course)
    {
        if (p == null || !p.Courses.TryGetValue(course, out var r))
            return w.U8((byte)course).Zeros(4 * 7).U8(NoScore).Zeros(8 + 4 + 1);
        return w.U8((byte)course).Zeros(4 * 2).U32((uint)r.Holes).Zeros(4 * 3).I32(r.TotalScore)
            .U8((byte)(sbyte)Math.Clamp(r.BestScore, -128, CourseRecord.NoRecord)).I64(r.MaxPang).U32((uint)r.CharacterTypeId).U8(0);
    }

    /// <summary>Bloco do 0x44 (0x31C6 bytes), com os equipados no fim.</summary>
    public static PacketWriter PlayerBlock(this PacketWriter w, Player p, KnownType? known = null, ushort roomIndex = 0xFFFF,
        uint? oid = null, Domain.Players.IClubWorkshopData? workshop = null)
    {
        w.MemberInfo(p, roomIndex, oid).U32((uint)p.AccountId).UserInfo(p).Trophies(p).UserEquip(p, known);
        for (int i = 0; i < MapCount; i++) w.MapStat(p, i);
        for (int t = 1; t < MapTables; t++)
            for (int i = 0; i < MapCount; i++) w.MapStat(null, i);
        return w.EquipedItems(p, known, workshop);
    }

    /// <summary>
    /// Equipados (628): personagem 513, caddie 25, club set 28 (i32 id, u32 typeid, i16[5] upgrades, i16[5] slots),
    /// mascote 62. A referência manda zeros por engano (escreve vazio justamente quando o item existe).
    /// </summary>
    public static PacketWriter EquipedItems(this PacketWriter w, Player p, KnownType? known = null,
        Domain.Players.IClubWorkshopData? workshop = null)
    {
        if (EquippedCharacter(p, known) is { } ch)
            w.Character(Raw(KR645.Game.PlayerStructs.Character(ch)), known, id => p.Find(id) is { } c
                ? (Domain.Players.CharacterSystems.Mastery(c), Domain.Players.CharacterSystems.Cards(p, id)) : null);
        else w.Zeros(CharacterSize);
        if (Equipped(p, p.Equip.CaddieId, known) is { } cad) w.Bytes(Raw(KR645.Game.PlayerStructs.Caddie(cad)));
        else w.Zeros(CaddieSize);
        if (Equipped(p, p.Equip.ClubSetId, known) is { } club)
        {
            w.I32(club.Id).U32((uint)club.TypeId);
            foreach (var v in club.IntArray("pcl", 5)) w.I16((short)v);    // upgrades (+8), depois slots (+0x12)
            w.ClubSlots(club, workshop);
        }
        else w.Zeros(ClubSetSize);
        if (Equipped(p, p.Equip.MascotId, known) is { } m) w.Mascot(Raw(KR645.Game.PlayerStructs.Mascot(m)));
        else w.Zeros(MascotSize);
        return w;
    }

    /// <summary>
    /// Slots do taco (+0x12, i16[5]) = Slot do ClubSet.iff + ganhos da oficina: o 983 soma estes 5 para o rank do taco dos
    /// outros jogadores (logo "PangYa!" animado no rank S, 0x6900a2); só o próprio jogador usa o inventário.
    /// </summary>
    public static PacketWriter ClubSlots(this PacketWriter w, Item club, Domain.Players.IClubWorkshopData? workshop)
    {
        var info = workshop != null && workshop.ClubWorkshop.TryGetValue(club.TypeId, out var i) ? i : null;
        var ws = Domain.Players.ClubWorkshop.Slots(club);
        for (int s = 0; s < 5; s++) w.I16((short)((info?.Slot[s] ?? 0) + ws[s]));
        return w;
    }

    static Item? Equipped(Player p, int id, KnownType? known) => p.Find(id) is { } it && Known(known, it.TypeId) ? it : null;

    /// <summary>Bytes de uma struct do KR (para os conversores de <see cref="KrToJp"/>).</summary>
    static byte[] Raw<T>(T v) where T : unmanaged
    {
        var b = new byte[System.Runtime.CompilerServices.Unsafe.SizeOf<T>()];
        System.Runtime.InteropServices.MemoryMarshal.Write(b, in v);
        return b;
    }
}
