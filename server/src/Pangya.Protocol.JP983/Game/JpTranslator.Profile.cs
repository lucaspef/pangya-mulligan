using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Perfil de outro jogador (C->S 0x2F): os pacotes que o KR monta em GameHandler.UserInfoAsync, no layout do JP
/// (SPEC-jp983-correio-social.md §2). Mandados só trocando o id, o cliente lia além do fim e caía.
/// </summary>
public sealed partial class JpTranslator
{
    /// <summary>GUILD_INFO do KR (297) e GuildInfo do JP (77).</summary>
    const int KrGuildInfoSize = 297, GuildInfoSize = 77;

    void AddProfile()
    {
        toClient[0x14F] = ProfileMember;                                       // -> 0x157: cria a entrada do perfil
        toClient[0x14E] = (jp, k) => k.Length >= 5 + KrUserEquipSize           // -> 0x156: u8, u32, equip 108 -> 116 (+ 2 pôsteres)
            ? new PacketWriter(jp).Bytes(k.AsSpan(0, 5 + KrUserEquipSize)).Zeros(8) : null;
        toClient[0x156] = (jp, k) => k.Length >= 4 + KrToJp.KrCharacterSize   // -> 0x15E: u32 uid, personagem 444 -> 513
            ? new PacketWriter(jp).Bytes(k.AsSpan(0, 4)).Character(k.AsSpan(4, KrToJp.KrCharacterSize), Known, Extras) : null;
        toClient[0x150] = (jp, k) => k.Length >= 5 + KrStatsSize               // -> 0x158: u8, u32, estatísticas 235 -> 265
            ? new PacketWriter(jp).Bytes(k.AsSpan(0, 5)).Bytes(KrToJp.StatsToJp(k.AsSpan(5, KrStatsSize))) : null;
        toClient[0x154] = (jp, k) =>                                           // -> 0x15C: u16 n -> i32 n, + i32 0 (lista "assist")
        {
            int n = BitConverter.ToUInt16(k, 5), len = Math.Min(n * JpStructs.MapStatSize, k.Length - 7);
            return new PacketWriter(jp, 16 + len).Bytes(k.AsSpan(0, 5)).I32(len / JpStructs.MapStatSize).Bytes(k.AsSpan(7, len)).I32(0);
        };
        toClient[0x155] = (jp, k) => k.Length >= 4 + KrGuildInfoSize ? ProfileGuild(jp, k) : null;   // -> 0x15D
        toClient[0x3B] = toClient[0x3C] = (jp, k) => null;                  // GUILD_INFO (emblema/nome): o JP não tem; a lista do lobby leva
        toClient[0x152] = (jp, k) =>                                           // -> 0x15A: troféus especiais (os do GP vão no 0x257)
        {
            profileGpTrophies.Clear();
            var w = new PacketWriter(jp, k.Length + 8).Bytes(k.AsSpan(0, 5));
            int n = k.Length >= 7 ? BitConverter.ToUInt16(k, 5) : 0;
            var keep = new List<int>(n);
            for (int i = 0; i < n && 7 + (i + 1) * 12 <= k.Length; i++)
                if (Domain.Players.SpecialTrophies.IsGrandPrix(BitConverter.ToInt32(k, 7 + i * 12 + 4))) profileGpTrophies.Add(k.AsSpan(7 + i * 12, 12).ToArray());
                else keep.Add(i);
            w.U16((ushort)keep.Count);
            foreach (int i in keep) w.Bytes(k.AsSpan(7 + i * 12, 12));
            return w;
        };
        toClient[0x87] = (jp, k) =>                                            // -> 0x89; antes, troféus do Grand Prix (0x257)
        {
            if (k.Length >= 9 && BitConverter.ToUInt32(k, 0) == 1)
                Reply(0x257, w =>
                {
                    w.Bytes(k.AsSpan(4, 5)).U16((ushort)profileGpTrophies.Count);
                    foreach (var t in profileGpTrophies) w.Bytes(t);
                });
            return new PacketWriter(jp).Bytes(k);
        };
    }

    const int KrUserEquipSize = 108, KrStatsSize = 235, KrMemberInfoSize = 269;
    /// <summary>Troféus do Grand Prix do perfil em montagem (KR 0x152 -> JP 0x257, mandado antes do 0x89).</summary>
    readonly List<byte[]> profileGpTrophies = [];

    /// <summary>
    /// KR 0x14F u8 temporada, u32 uid, u16 sala, sPangYaUserInfo 269 (termina com dwUID), u32 pontos -> JP 0x157 u8, u32,
    /// MemberInfoEx 299 (u16 sala + 297), u32 uid, u32 pontos. Mapa dos campos: SPEC-jp983-correio-social.md §2.1.
    /// </summary>
    static PacketWriter? ProfileMember(ushort jp, byte[] k)
    {
        if (k.Length < 7 + KrMemberInfoSize + 4) return null;
        var w = MemberInfo301(new PacketWriter(jp, 320).Bytes(k.AsSpan(0, 7)), k.AsSpan(7, KrMemberInfoSize));
        return w.Bytes(k.AsSpan(7 + KrMemberInfoSize, 4));
    }

    /// <summary>sPangYaUserInfo do KR (269, termina com o uid) -> MemberInfo do JP (297) + u32 uid (o bloco de 301 do JP).</summary>
    static PacketWriter MemberInfo301(PacketWriter w, ReadOnlySpan<byte> m) => w
        .Bytes(m[..44])                                                        // id, nick
        .Bytes(m.Slice(44, 16)).U8(0)                                          // guilda: 21 -> 17
        .Bytes(m.Slice(65, 12)).Zeros(35)                                      // emblema; comentário
        .Bytes(m.Slice(77, 36))                                                // escola, capacidade, galeria, oid, ranking, guilda, emblema
        .U8(m[113]).U16(2)                                                     // estado; login (não é o primeiro)
        .U16(JpStructs.PapelUnlimited).U16(JpStructs.PapelUnlimited).U16(JpStructs.PapelUnlimited)   // papel shop sem limite
        .Bytes(m.Slice(121, 4))                                                // pontos de evento
        .Bytes(m.Slice(125, 8)).Bytes(m.Slice(133, 4))                         // bloqueio; canalização
        .Bytes(m.Slice(137, 128))                                              // id de exibição
        .Bytes(m.Slice(265, 4));                                               // uid

    /// <summary>
    /// KR 0x155 u32 uid, GUILD_INFO 297 (u32 id, char[21] nome, i32 pang, i32 pontos, i32 membros, char[12] emblema, ...)
    /// -> JP 0x15D u32 uid, GuildInfo 77 (i32 id, u8, char[20] nome, char[12] emblema, u32 nº emblema, u64, u64 pang, 16, u32 pontos).
    /// </summary>
    static PacketWriter ProfileGuild(ushort jp, byte[] k)
    {
        var g = k.AsSpan(4, KrGuildInfoSize);
        bool leader = BitConverter.ToUInt32(k, 0) == BitConverter.ToUInt32(g.Slice(255, 4));             // o dono do perfil é o mestre
        uint markNo = (uint)Domain.Guilds.GuildTag.MarkNo(Core.Text.Cp949.Read(g.Slice(37, 12)));
        return new PacketWriter(jp, GuildInfoSize + 8).Bytes(k.AsSpan(0, 4)).Bytes(g[..4]).U8(leader ? (byte)1 : (byte)0).Bytes(g.Slice(4, 19)).U8(0)
            .Bytes(g.Slice(37, 12)).U32(markNo).U64(0).U64((ulong)Math.Max(BitConverter.ToInt32(k, 4 + 25), 0)).Zeros(16)
            .Bytes(g.Slice(29, 4));
    }
}
