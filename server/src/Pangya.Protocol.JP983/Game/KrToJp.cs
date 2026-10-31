using System.Buffers.Binary;
using Pangya.Core.Net;
using Pangya.Core.Text;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Conversão byte a byte das structs do KR 645 (Structs.g.cs) para as do JP 983 e de volta, por faixas de offsets
/// (docs/server/protocolo/SPEC-jp983-game.md, "Structs que mudaram"). Typeids que o cliente JP não conhece viram 0.
/// </summary>
public static class KrToJp
{
    public const int KrCharacterSize = 444, KrItemSize = 168, KrMascotSize = 63, KrChannelSize = 77, KrLobbyUserSize = 196,
        KrRoomSize = 178, KrEquipSize = 108;
    public const int RoomSize = 221, LobbyUserSize = 200, KrSlotSize = 338, SlotSize = 348, KrClubSize = 18, KrUserInfoSize = 2962;

    /// <summary>Maestria e os 12 cards de um personagem pelo id (null = desconhecido: zeros).</summary>
    public delegate (int Mastery, int[] Cards)? CharacterExtras(int characterId);

    static bool Known(JpStructs.KnownType? known, uint tid) => tid == 0 || known == null || known((int)tid);

    static uint U32(ReadOnlySpan<byte> s, int o) => BinaryPrimitives.ReadUInt32LittleEndian(s[o..]);

    /// <summary>
    /// sCharacterInfo KR (444: tid, guid, peças[24], cabelo, camisa|presente&lt;&lt;4, upgrades[5], compra, anéis[5],
    /// Self Design 216, ids das peças[24]) -> CharacterInfo JP (513).
    /// </summary>
    public static PacketWriter Character(this PacketWriter w, ReadOnlySpan<byte> k, JpStructs.KnownType? known = null,
        CharacterExtras? extras = null)
    {
        w.Bytes(k[..8]).U8(k[104]).U8((byte)(k[105] & 0x0F)).U8((byte)(k[105] >> 4)).U8(k[111]);
        for (int i = 0; i < 24; i++) w.U32(Known(known, U32(k, 8 + i * 4)) ? U32(k, 8 + i * 4) : 0);
        for (int i = 0; i < 24; i++) w.U32(Known(known, U32(k, 8 + i * 4)) ? U32(k, 348 + i * 4) : 0);
        w.Bytes(k.Slice(132, 216));
        for (int i = 0; i < 5; i++) w.U32(Known(known, U32(k, 112 + i * 4)) ? U32(k, 112 + i * 4) : 0);
        w.Zeros(16).Bytes(k.Slice(106, 5));                                            // cut-in; upgrades
        if (extras?.Invoke(BinaryPrimitives.ReadInt32LittleEndian(k[4..])) is not var (mastery, cards)) return w.U32(0).Zeros(48);
        w.U32((uint)mastery);                                                           // maestria e os 12 slots de card
        for (int i = 0; i < 12; i++) w.U32((uint)(i < cards.Length ? cards[i] : 0));
        return w;
    }

    /// <summary>CharacterInfo JP (513) -> sCharacterInfo KR (444): o 0x20 tipo 0 (roupa do personagem).</summary>
    public static byte[] CharacterToKr(ReadOnlySpan<byte> j)
    {
        var k = new byte[KrCharacterSize];
        j[..8].CopyTo(k);
        j.Slice(12, 96).CopyTo(k.AsSpan(8));                                           // peças
        k[104] = j[8];
        k[105] = (byte)((j[9] & 0x0F) | (j[10] << 4));
        j.Slice(456, 5).CopyTo(k.AsSpan(106));                                         // upgrades
        k[111] = j[11];
        j.Slice(420, 20).CopyTo(k.AsSpan(112));                                        // anéis
        j.Slice(204, 216).CopyTo(k.AsSpan(132));                                       // Self Design
        j.Slice(108, 96).CopyTo(k.AsSpan(348));                                        // ids das peças
        return k;
    }

    /// <summary>
    /// sItemInfo KR (168) -> WarehouseItem JP (196). Iguais até o byte 23 (id, typeid, tempo, i16[5], compra, flags) e de
    /// 82 a 115 (Self Design); o nome do KR tem 41 (o JP: 40 + venda); o JP põe o uid de quem copiou antes dos cards e a
    /// oficina do club set no fim. As datas (i64) ficam 0. O byte 40 é o mesmo nos dois: no KR o bit "válido" (2), no JP o
    /// tipo do item, que a referência cria com 2 (sem ele o cliente trata o item como inválido: a bola equipada aparece no
    /// lobby mas perde o efeito na partida). Tempo: o KR usa 0 para permanente, o JP -1 (referência: ano = -1).
    /// Relatório do Tiki: a 1ª data (+24) leva o fim do prazo em segundos unix, como o cliente grava ao receber o 0xAA.
    /// </summary>
    public static PacketWriter Item(this PacketWriter w, ReadOnlySpan<byte> k, Action<PacketWriter, int>? workshop = null)
    {
        int hours = BinaryPrimitives.ReadInt32LittleEndian(k[8..]);
        w.Bytes(k[..8]).I32(hours == 0 ? -1 : hours).Bytes(k.Slice(12, 12));
        if (BinaryPrimitives.ReadInt32LittleEndian(k[4..]) == Domain.Rooms.TikiReport.ReportTypeId) w.I64(UnixSeconds(k.Slice(24, 16))).Zeros(8);
        else w.Zeros(16);
        w.U8(k[40]).Bytes(k.Slice(41, 40)).U8(0).Bytes(k.Slice(82, 34)).U32(0).Bytes(k.Slice(116, 48));
        if (workshop == null) return w.Zeros(28);
        workshop(w, BinaryPrimitives.ReadInt32LittleEndian(k));                         // oficina do club set (28)
        return w;
    }

    /// <summary>SYSTEMTIME em hora local -> segundos unix (o SysTimeToSec do cliente, 00b30830); vazio ou inválido = 0.</summary>
    public static long UnixSeconds(ReadOnlySpan<byte> t)
    {
        static int U(ReadOnlySpan<byte> s, int i) => BinaryPrimitives.ReadUInt16LittleEndian(s[(i * 2)..]);
        try { return new DateTimeOffset(new DateTime(U(t, 0), U(t, 1), U(t, 3), U(t, 4), U(t, 5), U(t, 6), DateTimeKind.Local)).ToUnixTimeSeconds(); }
        catch (ArgumentOutOfRangeException) { return 0; }
    }

    /// <summary>
    /// sMascotInfo KR (63: …, msg 30, u16 horas, compra, SYSTEMTIME fim, PC bang) -> MascotInfo JP (62). No lugar das horas
    /// o JP tem o tipo (referência MascotInfo.tipo): 0 sem prazo (a janela de detalhe diz "No Limit"), 1 com prazo (conta
    /// pelo fim); sem prazo é o fim em <see cref="KR645.Game.PlayerStructs.Forever"/>.
    /// </summary>
    public static PacketWriter Mascot(this PacketWriter w, ReadOnlySpan<byte> k) =>
        w.Bytes(k[..43]).I16((short)(BinaryPrimitives.ReadUInt16LittleEndian(k[46..]) >= KR645.Game.PlayerStructs.Forever.Year ? 0 : 1))
            .Bytes(k.Slice(46, 16)).U8(k[62]);

    /// <summary>sChannelInfo KR (77) -> ChannelInfo JP (85): + propriedade 16 (como a referência), nível mín 0 e máx 70.</summary>
    public static PacketWriter Channel(this PacketWriter w, ReadOnlySpan<byte> k) =>
        w.Bytes(k[..73]).U32(16).U32(0).U32((uint)Domain.Players.Levels.Max);

    /// <summary>
    /// sSlotInfo KR (338) -> PlayerRoomInfo JP (348): guilda 21 -> 20, + u32 nº do emblema depois do emblema, + f32 média de
    /// placar e 3 bytes no fim (referência: PlayerRoomInfo.ToArray).
    /// </summary>
    public static PacketWriter Slot(this PacketWriter w, ReadOnlySpan<byte> k)
    {
        w.Bytes(k[..26]).Bytes(k.Slice(26, 19)).U8(0).Bytes(k.Slice(47, 58)).U32(MarkNo(k.Slice(93, 12))).Bytes(k.Slice(105, 233));
        return w.F32(0).Zeros(3);
    }

    /// <summary>Item da loja pessoal: sTradeItem KR 170 -> JP 172 (u16 slots de NPC no offset 107).</summary>
    public const int KrTradeItemSize = 170, TradeItemSize = 172, TradeItemGap = 107;

    public static PacketWriter TradeItem(this PacketWriter w, ReadOnlySpan<byte> k) => w.Bytes(k[..TradeItemGap]).U16(0).Bytes(k[TradeItemGap..]);

    public static byte[] TradeItemToKr(ReadOnlySpan<byte> j)
    {
        var k = new byte[KrTradeItemSize];
        j[..TradeItemGap].CopyTo(k);
        j[(TradeItemGap + 2)..].CopyTo(k.AsSpan(TradeItemGap));
        return k;
    }

    /// <summary>
    /// sClubInfo KR (18: guid, typeid, i16[5] upgrades) -> ClubSetInfo JP (28: + i16[5] slots DEPOIS dos upgrades). O 983
    /// (00664c30, Practice 0063e3a8) lê +8 como upgrades e +0x12 como slots; invertido, os upgrades do taco não entravam na sala.
    /// </summary>
    public static PacketWriter Club(this PacketWriter w, ReadOnlySpan<byte> k) => w.Bytes(k[..18]).Zeros(10);

    /// <summary>sBriefUserInfo KR (196) -> PlayerLobbyInfo JP (200): o JP tem o nº do emblema (u32) depois do id da guilda.</summary>
    public static PacketWriter LobbyUser(this PacketWriter w, ReadOnlySpan<byte> k) => w.Bytes(k[..50]).U32(MarkNo(k.Slice(50, 12))).Bytes(k[50..]);

    /// <summary>Nº do emblema a partir da marca do KR (char[12] "g&lt;hex&gt;").</summary>
    static uint MarkNo(ReadOnlySpan<byte> mark) => (uint)Domain.Guilds.GuildTag.MarkNo(Cp949.Read(mark));

    /// <summary>
    /// sRoomInfo KR (178) -> RoomInfo JP (221): nome 64 (o KR tem título 32 + senha 16; o JP não manda a senha), 5 flags,
    /// chave 16, os mesmos 22 bytes (galeria … estado), guildas no formato do JP (ids, emblemas 12, nº do emblema u16, nomes
    /// 20), taxas de pang/EXP, flag GM, dono, tipo real, artefato (o da sala, que o KR não tem) e natural/short game e grand
    /// prix zerados.
    /// </summary>
    public static PacketWriter Room(this PacketWriter w, ReadOnlySpan<byte> k, uint artifact = 0, uint flags = 0)
    {
        Span<byte> mid = stackalloc byte[22];                                   // galeria … estado; o modo (+3) no JP é o "tipo"
        k.Slice(69, 22).CopyTo(mid);
        mid[3] = TipoShow(mid[3]);
        w.Bytes(k[..32]).Zeros(32).Bytes(k.Slice(48, 5)).Bytes(k.Slice(53, 16)).Bytes(mid);
        var g = k.Slice(91, 74);                                                        // sGuildRoomInfo
        w.Bytes(g[..8]).Bytes(g.Slice(50, 12)).Bytes(g.Slice(62, 12)).U16((ushort)MarkNo(g.Slice(50, 12))).U16((ushort)MarkNo(g.Slice(62, 12)))
            .Fixed(Cp949.Read(g.Slice(8, 21)), 20).Fixed(Cp949.Read(g.Slice(29, 21)), 20);
        return w.Bytes(k.Slice(165, 8)).U8(0).Bytes(k.Slice(173, 4)).U8(ModeToJp(k[177])).U32(artifact).U32(flags).Zeros(16);
    }

    /// <summary>Special Shuffle Course: modo 14 no KR, 18 no JP (os outros modos têm o mesmo número).</summary>
    public static byte ModeToJp(byte krMode) => krMode == 14 ? (byte)18 : krMode;
    public static byte ModeFromJp(byte jpMode) => jpMode == 18 ? (byte)14 : jpMode;
    /// <summary>"Tipo de exibição" do JP (referência: Room.cs): os modos acima de 14 do JP aparecem como torneio (4).</summary>
    public static byte TipoShow(byte krMode) => krMode == 14 ? (byte)4 : krMode;

    /// <summary>
    /// Estatística da partida (0x06/0x31): UserInfo do JP (265, JpStructs.UserInfo) -> sPangYaUserStatistics do KR (235).
    /// Iguais até o byte 44; o JP tem hole in one e albatross com 4 bytes (o KR com 2) e o resto 2 a 4 bytes adiante.
    /// </summary>
    public static byte[] StatsToKr(ReadOnlySpan<byte> j)
    {
        var k = new byte[235];
        j[..44].CopyTo(k);
        BinaryPrimitives.WriteUInt16LittleEndian(k.AsSpan(44), (ushort)Math.Clamp(BinaryPrimitives.ReadInt32LittleEndian(j[44..]), 0, ushort.MaxValue));
        j.Slice(48, 2).CopyTo(k.AsSpan(46));                                    // bunker
        j.Slice(50, 4).CopyTo(k.AsSpan(48));                                    // fairway
        BinaryPrimitives.WriteUInt16LittleEndian(k.AsSpan(52), (ushort)Math.Clamp(BinaryPrimitives.ReadInt32LittleEndian(j[54..]), 0, ushort.MaxValue));
        j.Slice(58, 33).CopyTo(k.AsSpan(54));                                   // hole in, putt in, putt/chip, EXP, nível, pang, placar
        j.Slice(91, 5).CopyTo(k.AsSpan(87));                                    // melhor placar por estrela (o KR tem 6)
        k[92] = 127;
        j.Slice(97, 40).CopyTo(k.AsSpan(93));                                   // melhor pang por estrela
        j.Slice(145, 4).CopyTo(k.AsSpan(141));                                  // jogos
        j.Slice(189, 4).CopyTo(k.AsSpan(185));                                  // abandonos
        return k;
    }

    /// <summary>
    /// Estatísticas KR (235) -> UserInfo do JP (265), o inverso de <see cref="StatsToKr"/> (perfil de outro jogador, 0x158).
    /// O que só o JP tem (pang battle, medalhas, ...) vai zerado.
    /// </summary>
    public static byte[] StatsToJp(ReadOnlySpan<byte> k)
    {
        var j = new byte[JpStructs.UserInfoSize];
        k[..44].CopyTo(j);
        BinaryPrimitives.WriteInt32LittleEndian(j.AsSpan(44), BinaryPrimitives.ReadUInt16LittleEndian(k[44..]));   // hole in one
        k.Slice(46, 2).CopyTo(j.AsSpan(48));                                    // bunker
        k.Slice(48, 4).CopyTo(j.AsSpan(50));                                    // fairway
        BinaryPrimitives.WriteInt32LittleEndian(j.AsSpan(54), BinaryPrimitives.ReadUInt16LittleEndian(k[52..]));   // albatross
        k.Slice(54, 33).CopyTo(j.AsSpan(58));                                   // hole in, putt in, putt/chip, EXP, nível, pang, placar
        k.Slice(87, 5).CopyTo(j.AsSpan(91));                                    // melhor placar por estrela
        k.Slice(93, 40).CopyTo(j.AsSpan(97));                                   // melhor pang por estrela
        k.Slice(141, 4).CopyTo(j.AsSpan(145));                                  // jogos
        k.Slice(185, 4).CopyTo(j.AsSpan(189));                                  // abandonos
        return j;
    }
}
