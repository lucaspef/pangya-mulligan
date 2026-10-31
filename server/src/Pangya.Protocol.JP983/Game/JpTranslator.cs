using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// O game server do KR 645 (GameHandler) falando com o cliente JP 983: ids e structs traduzidos nos dois sentidos
/// (docs/server/protocolo/SPEC-jp983-game.md). Este arquivo: ids, despacho e login/inventário. Sala e partida: .Room.cs; loja e
/// My Room: .Shop.cs; pacotes que só o JP tem: .JpOnly.cs. Pacote do KR sem id conhecido no JP, ou com struct que mudou e
/// ainda sem conversão, não vai (fica no log); pacote do JP sem par no KR também não.
/// </summary>
public sealed partial class JpTranslator : IPacketTranslator
{
    readonly Connection conn;
    readonly GameHandler kr;
    readonly Jp983GameData? jpData;
    readonly uint property;
    /// <summary>KR -> JP (S->C): corpo do KR (sem o id) -> pacote do JP; null = não vai.</summary>
    readonly Dictionary<int, Func<ushort, byte[], PacketWriter?>> toClient = [];
    /// <summary>JP -> KR (C->S): corpo novo (id + dados), <see cref="Pass"/> = como veio (com o id do KR), null = não vai.</summary>
    readonly Dictionary<int, Func<PacketReader, byte[]?>> fromClient = [];
    static readonly byte[] Pass = [];

    public JpTranslator(Connection conn, GameHandler kr, Jp983GameData? jpData, uint property = 0)
    {
        this.conn = conn;
        this.kr = kr;
        this.jpData = jpData;
        this.property = property;
        AddInventory();
        AddRoom();
        AddShop();
        AddProfile();
        AddJpOnly();
        AddEffects();
        AddIntrusion();
        foreach (var id in JpSystems.Ids) fromClient[id] = r => Body((ushort)r.Id, w => w.Bytes(r.Bytes(r.Remaining)));   // JpSystems
    }

    /// <summary>Ids S->C do KR fora das faixas, conferidos um a um (SPEC-jp983-game.md, specs de loja e correio).</summary>
    static readonly Dictionary<int, int> ServerIdExceptions = new()
    {
        [0x16E] = 0x168, [0x171] = 0x16C, [0xD4] = 0x21B, [0xED] = 0x22F, [0x15E] = 0x210,
    };

    /// <summary>
    /// Id S->C do KR no JP, pelas faixas medidas no exe (V4_RELATORIO.md): +0 até 0x09, +2 de 0x2d a 0x113 (0x108..0x113
    /// conferidos um a um: desconexão, últimas partidas, bloqueios, aviso...), +4 de 0x116 a 0x11b, +8 de 0x11e a 0x15a
    /// (0x11e..0x122 conferidos: caixa do My Room 0x121 -> 0x129), −5 de 0x172 a 0x1a2; fora delas, desconhecido (−1).
    /// Exceções: <see cref="ServerIdExceptions"/>.
    /// </summary>
    public static int ServerId(int kr) => ServerIdExceptions.TryGetValue(kr, out var jp) ? jp : kr switch
    {
        <= 0x09 => kr,
        >= 0x2D and <= 0x113 => kr + 2,
        >= 0x116 and <= 0x11B => kr + 4,
        >= 0x11E and <= 0x15A => kr + 8,
        >= 0x172 and <= 0x1A2 => kr - 5,
        _ => -1,
    };

    /// <summary>Ids C->S do JP fora das faixas: jogar no Papel Shop e a Caixa Mágica.</summary>
    static readonly Dictionary<int, int> ClientIdExceptions = new() { [0x14B] = 0x6D, [0x158] = 0x7E };

    /// <summary>Id C->S do JP no KR: +0 até 0x82, −3 de 0x8b a 0x9d, −8 de 0xad a 0xca, +2 de 0xcc a 0xef; senão −1.</summary>
    public static int ClientId(int jp) => ClientIdExceptions.TryGetValue(jp, out var krId) ? krId : jp switch
    {
        <= 0x82 => jp,
        >= 0x8B and <= 0x9D => jp - 3,
        >= 0xAD and <= 0xCA => jp - 8,
        >= 0xCC and <= 0xEF => jp + 2,
        _ => -1,
    };

    /// <summary>
    /// Ids JP que leem struct que mudou e ainda não têm conversão: não vão, para o cliente não ler fora do pacote e cair.
    /// Correio.
    /// </summary>
    static readonly HashSet<int> NotConvertedYet =
        [0x98, 0x9D, 0xAE, 0xB1, 0xBC, 0xDF, 0x210, 0x211, 0x215];

    /// <summary>sMailInfoBrief do KR (194).</summary>
    const int KrMailBriefSize = 194;

    bool Known(int typeId) => jpData?.Exists(typeId) ?? true;

    public PacketWriter? ToClient(PacketWriter w)
    {
        int krId = w.Id, jpId = ServerId(krId);
        bool convert = toClient.TryGetValue(krId, out var f);
        if (jpId < 0 || (NotConvertedYet.Contains(jpId) && !convert))
        {
            Log.Info($"{conn} JP: 0x{krId:X4} do KR não vai (" + (jpId < 0 ? "sem id no JP" : $"0x{jpId:X4} sem conversão") + ")");
            w.Dispose();
            return null;
        }
        if (!convert) return w.SetId((ushort)jpId);
        using (w) return f!((ushort)jpId, w.Body[2..].ToArray());
    }

    public bool FromClient(PacketReader r, out byte[]? replaced)
    {
        replaced = null;
        int jp = r.Id, start = r.Position, krId = ClientId(jp);
        if (fromClient.TryGetValue(jp, out var f))
        {
            var body = f(r);
            if (body == null) return false;
            if (body.Length > 0) { replaced = body; return true; }
        }
        else if (krId < 0)
        {
            Log.Info($"{conn} JP: pacote 0x{jp:X4} sem par no KR ({r.Remaining} bytes)");
            return false;
        }
        if (krId != jp) { r.Position = start; replaced = Body((ushort)krId, w => w.Bytes(r.Bytes(r.Remaining))); }
        return true;
    }

    static byte[] Body(ushort id, Action<PacketWriter> write)
    {
        using var w = new PacketWriter(id);
        write(w);
        return w.Body.ToArray();
    }

    /// <summary>Manda um pacote que só o JP tem (já no formato do JP).</summary>
    void Reply(ushort jpId, Action<PacketWriter> write)
    {
        var w = new PacketWriter(jpId);
        write(w);
        conn.SendTranslated(w);
    }

    /// <summary>Nível que o cliente conhece (para o "subiu N níveis" do 0x10F).</summary>
    int knownLevel = -1;

    void AddInventory()
    {
        toClient[0x42] = (jp, k) =>                                            // dados do jogador: do Domain, no formato JP
        {
            knownLevel = kr.Player.Level;
            conn.SendTranslated(new PacketWriter(jp).U8(0xD3).U8(0));
            return new PacketWriter(jp, 0x3300).U8(0).Str(Jp983.ClientVersion).PlayerBlock(kr.Player, Known, workshop: kr.Context.Data as Domain.Players.IClubWorkshopData).Time(DateTime.Now)
                .U16(0).Zeros(6).U32(0).U64(0).I32(0).U32(property);   // Papel: 0 (0xFFFF como no KR faz o JP avisar ao abrir a loja)
        };
        toClient[0x70] = (jp, k) => new PacketWriter(jp).UserEquip(kr.Player, Known);
        toClient[0x4B] = (jp, k) =>                                            // u8 n + n × canal
        {
            var w = new PacketWriter(jp).U8(k[0]);
            for (int i = 0; i < k[0]; i++) w.Channel(k.AsSpan(1 + i * KrToJp.KrChannelSize, KrToJp.KrChannelSize));
            return w;
        };
        toClient[0x6E] = (jp, k) => List(jp, k, 4, KrToJp.KrCharacterSize, 0, (w, s) => w.Character(s, Known, Extras), JpStructs.CharacterSize);
        toClient[0x71] = (jp, k) => List(jp, k, 4, KrToJp.KrItemSize, 4, (w, s) => w.Item(s, WorkshopTail), JpStructs.ItemSize);
        toClient[0x78] = (_, _) => null;                                       // caixa de presentes: o JP não trata o 0x7A (nenhum despachante)
        toClient[0x6F] = (jp, k) => List(jp, k, 4, JpStructs.CaddieSize, 4, (w, s) => w.Bytes(s), JpStructs.CaddieSize);
        toClient[0xDF] = (jp, k) => List(jp, k, 1, KrToJp.KrMascotSize, 4, (w, s) => w.Mascot(s), JpStructs.MascotSize);
        toClient[0x43] = (jp, k) => new PacketWriter(jp).UserInfo(kr.Player).Bytes(k.AsSpan(235));   // estatísticas 235 -> 265
        toClient[0x10D] = (jp, k) =>                                           // KR u8 1, u8 nível, u8 -> JP i32 0, u8 quantos, u8 nível
        {
            int to = k.Length > 1 ? k[1] : kr.Player.Level;
            int up = knownLevel >= 0 ? Math.Max(1, to - knownLevel) : 1;
            knownLevel = to;
            return new PacketWriter(jp).I32(0).U8((byte)up).U8((byte)to);
        };
        // cartas não lidas (login e JP 0xE4 -> KR 0xE6): KR u32 n, n × 194 -> JP i32 0, i32 n, n × 196 (anexo 53 -> 55)
        toClient[0x15E] = (jp, k) =>
        {
            int n = (int)BitConverter.ToUInt32(k, 0);
            var w = new PacketWriter(jp, 16 + n * JpSystems.MailBriefSize).I32(0).I32(n);
            for (int i = 0; i < n && 4 + (i + 1) * KrMailBriefSize <= k.Length; i++)
            {
                var b = k.AsSpan(4 + i * KrMailBriefSize, KrMailBriefSize);
                w.Bytes(b[..141]).Bytes(b.Slice(141, 17)).Zeros(24).Bytes(b.Slice(141 + 39, 9)).Zeros(3).I16(0);
            }
            return w;
        };
        // busca de nick (carta/presente): KR u8 0, u32 uid, sPangYaUserInfo 269 -> JP u8 0, u32 uid, MemberInfo 297 + uid
        toClient[0x9F] = (jp, k) => k.Length >= 5 + KrMemberInfoSize
            ? MemberInfo301(new PacketWriter(jp).Bytes(k.AsSpan(0, 5)), k.AsSpan(5, KrMemberInfoSize)) : new PacketWriter(jp).Bytes(k);
        // amigos pela conexão do game (0x3C/0x11F, para correio e presente): KR 0x2E -> JP 0x30, amigo 141 -> 145
        toClient[0x2E] = (jp, k) => k.Length >= 2 ? Messenger.JpMessenger.Sub(jp, k, null) : null;
        fromClient[0x06] = Stats;
        fromClient[0x31] = Stats;
    }

    /// <summary>
    /// Lista com contagem (u8, ou u16 total + u16 n) e registros de tamanho fixo; os de typeid que o cliente JP não
    /// conhece (u32 no offset tidAt) saem e a contagem acompanha.
    /// </summary>
    PacketWriter List(ushort jp, byte[] k, int header, int size, int tidAt, Action<PacketWriter, ReadOnlySpan<byte>> write, int jpSize)
    {
        int n = header == 1 ? k[0] : BitConverter.ToUInt16(k, 2);
        var keep = new List<int>(n);
        for (int i = 0; i < n; i++)
            if (Known(BitConverter.ToInt32(k, header + i * size + tidAt))) keep.Add(i);
        var w = new PacketWriter(jp, 8 + keep.Count * jpSize);
        if (header == 1) w.U8((byte)keep.Count);
        else w.U16((ushort)keep.Count).U16((ushort)keep.Count);
        foreach (var i in keep) write(w, k.AsSpan(header + i * size, size));
        return w;
    }

    /// <summary>
    /// 0x06 (fim) / 0x31 (cada buraco): o JP manda as estatísticas da partida no UserInfo de 265 bytes; o KR lê o
    /// sPangYaUserStatistics de 235 (KrToJp.StatsToKr).
    /// </summary>
    byte[]? Stats(PacketReader r) =>
        r.Remaining >= JpStructs.UserInfoSize ? Body((ushort)r.Id, w => w.Bytes(KrToJp.StatsToKr(r.Bytes(JpStructs.UserInfoSize)))) : Pass;

    /// <summary>
    /// Fim do item 196 (SPEC-jp983-oficina.md §7.1): i16 flag, i16[5] ws.c, u32 maestria, u32 recuperação, i32 nível, i32 rank
    /// do club set; zeros nos outros itens.
    /// </summary>
    void WorkshopTail(PacketWriter w, int id)
    {
        if (kr.Context.Data is not Domain.Players.IClubWorkshopData data || kr.Player.Find(id) is not { } it
            || !data.ClubWorkshop.TryGetValue(it.TypeId, out var info))
        {
            w.Zeros(28);
            return;
        }
        w.I16(0);
        foreach (var v in Domain.Players.ClubWorkshop.Slots(it)) w.I16((short)v);
        w.U32((uint)Domain.Players.ClubWorkshop.Mastery(it)).U32((uint)Domain.Players.ClubWorkshop.Recovery(it));
        w.I32(info.Type < 0 ? -1 : Domain.Players.ClubWorkshop.Level(info, it)).I32(info.Type < 0 ? -1 : Domain.Players.ClubWorkshop.Rank(info, it));
    }

    /// <summary>
    /// Maestria e cards de um personagem (CharacterInfo 465..512): do próprio jogador ou de quem estiver online (sala, perfil).
    /// </summary>
    (int Mastery, int[] Cards)? Extras(int characterId)
    {
        if (characterId == 0) return null;
        if (kr.Player.Find(characterId) is { } own) return (Domain.Players.CharacterSystems.Mastery(own), Domain.Players.CharacterSystems.Cards(kr.Player, characterId));
        try
        {
            foreach (var s in kr.Context.World.Online)
                if (s.Player is { } p && p != kr.Player && p.Find(characterId) is { Group: Domain.Players.ItemGroup.Character } c)
                    return (Domain.Players.CharacterSystems.Mastery(c), Domain.Players.CharacterSystems.Cards(p, characterId));
        }
        catch (InvalidOperationException) { }                                    // inventário de outro mudando agora: sem os extras
        return null;
    }
}
