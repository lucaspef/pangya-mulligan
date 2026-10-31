using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>Lobby, sala, partida e o lobby do Grand Prix (SPEC-jp983-game.md, SPEC-jp983-sistemas.md, SPEC-jp983-grandprix.md).</summary>
public sealed partial class JpTranslator
{
    /// <summary>
    /// Tacada do JP: os 46 bytes do KR (CGolfRule::HitShot) + f32 zona de impacto (referência: ShotDataBase). O eco 0x55 lê
    /// u32 + estes 50; o resultado (0x1B / eco 0x64) tem 38 (o KR: 37, sem a penalidade do Grand Prix).
    /// </summary>
    public const int ShotSize = 50, ShotSyncSize = 12, ShotResultSize = 38;

    /// <summary>Lobby do Grand Prix (0x176/0x177): a lógica é a do lobby normal do KR, sem o 0xF5/0xF6 e com 0x250/0x251.</summary>
    bool enteringGpLobby, leavingGpLobby;

    void AddRoom()
    {
        toClient[0x44] = LobbyUsers;
        toClient[0x45] = (jp, k) =>                                            // u8 n, u8 sub, u16, n × sala
        {
            var w = new PacketWriter(jp, 8 + k[0] * KrToJp.RoomSize).Bytes(k.AsSpan(0, 4));
            for (int i = 0; i < k[0]; i++)
            {
                var room = k.AsSpan(4 + i * KrToJp.KrRoomSize, KrToJp.KrRoomSize);
                w.Room(room, ArtifactOf(room), FlagsOf(room));
            }
            return w;
        };
        toClient[0x46] = Slots;
        toClient[0x47] = (jp, k) =>                                            // u8 0, u8 galeria, sala -> i16 0 + sala; ou u8 erro
            k.Length >= 2 + KrToJp.KrRoomSize ? new PacketWriter(jp).Bytes(k.AsSpan(0, 2)).Bytes(PracticeRoom(k.AsSpan(2, KrToJp.KrRoomSize)))
                : new PacketWriter(jp).Bytes(k);
        toClient[0x48] = RoomSettings;
        toClient[0x49] = QuickEquip;
        toClient[0x50] = GameInit;
        toClient[0x74] = GamePlayers;
        toClient[0x53] = (jp, k) => new PacketWriter(jp).Bytes(k).Zeros(Math.Max(0, 4 + ShotSize + ShotSyncSize - k.Length));   // 983: u32 + 50 + 12
        toClient[0x59] = CardWind;
        // detalhe da sala (direito na lista): KR u8 n -> 983 i32 n (00ac3733; -1 = a sala não existe), 12 B, n × 0x12 B
        toClient[0x84] = (jp, k) => k.Length < 13 ? new PacketWriter(jp).I32(-1) : new PacketWriter(jp).I32(k[0]).Bytes(k.AsSpan(1));
        toClient[0x62] = (jp, k) => new PacketWriter(jp).Bytes(k).Zeros(Math.Max(0, ShotResultSize - k.Length));  // 37 -> 38
        // posição do rival (torneio/approach): o JP tem u32 estado da tacada depois de x, z (referência: TourneyBase)
        toClient[0x6C] = (jp, k) =>
        {
            RepeatSeq(k);
            return new PacketWriter(jp).Bytes(k.AsSpan(0, Math.Min(13, k.Length))).U32(0).Bytes(k.AsSpan(Math.Min(13, k.Length)));
        };
        // repetir buraco: o 0x52 numera as vezes 1..N (referência); o fim do buraco e a posição vêm do KR com o buraco
        // físico (o mesmo em todas): trocados pelo nº da vez, que conta os inícios de buraco (KR 0x51 -> JP 0x53)
        // início de buraco: antes dele, o clima do buraco (0x9E; o 0x52 não leva clima, o 3º byte do buraco é o pin)
        toClient[0x51] = (jp, k) =>
        {
            int hole = Math.Clamp(kr.CurrentRoom?.Settings.RepeatHole > 0 ? 0 : repeatSeq, 0, 17);
            repeatSeq++;
            conn.SendTranslated(new PacketWriter(SWeatherJp).U8(kr.CurrentRoom?.Weather ?? holeWeather[hole]).U8(0).U8(0));
            return new PacketWriter(jp).Bytes(k);
        };
        // Grand Prix: o cliente só vai ao próximo buraco com o 0x255 ("todos terminaram o buraco"; SPEC §5.2); o torneio do
        // KR manda só o 0x63 (-> 0x65) a quem terminou. Cada um segue no seu ritmo (os bots não esperam).
        toClient[0x63] = (jp, k) =>
        {
            if (kr.CurrentRoom?.GrandPrix == null) return new PacketWriter(jp).Bytes(k);
            conn.SendTranslated(new PacketWriter(jp).Bytes(k));
            return new PacketWriter(0x255);
        };
        toClient[0x6B] = (jp, k) => { RepeatSeq(k); return new PacketWriter(jp).Bytes(k); };
        toClient[0x77] = (jp, k) => kr.CurrentRoom?.Settings.Mode switch       // resultado do torneio (SPEC-jp983-intrusao-vip-modos §3.2)
        {
            GameMode.NewApproach or GameMode.Approach => null,                  // approach: o JP lê outro layout no campo; a referência não manda
            GameMode.GuildMatch when k.Length >= 122 => new PacketWriter(jp).Bytes(k.AsSpan(0, 18)).Bytes(k.AsSpan(26, 96)),   // só os 2 u32 do jogador
            _ => new PacketWriter(jp).Bytes(k).Zeros(24),                       // + medalhas do jogador (6 × i32)
        };
        // Treasure Hunter (KR 0x12C -> JP 0x134): u8 n, n × 21; o JP lê a quantidade em i32 e o total depois em 2 × u16
        toClient[0x12C] = (jp, k) =>
        {
            if (k.Length < 1) return null;
            int n = k[0];
            var w = new PacketWriter(jp, 4 + n * 21).U8((byte)n);
            for (int i = 0; i < n && 1 + (i + 1) * 21 <= k.Length; i++)
            {
                var e = k.AsSpan(1 + i * 21, 21);
                int id = BitConverter.ToInt32(e[8..]), qty = BitConverter.ToUInt16(e[12..]);
                int total = kr.Player.Find(id)?.Quantity ?? qty;
                w.Bytes(e[..12]).I32(qty).U8(0).U16((ushort)(total / 0x8000)).U16((ushort)(total % 0x8000));
            }
            return w;
        };
        toClient[0x147] = (jp, k) =>                                           // missão do approach: u8 nº, u8 caixas, u8 tipo, i32, i32, str
            (kr.CurrentRoom?.Game as ApproachGame)?.Mission is { } m
                ? new PacketWriter(jp).U8(m.No).U8(m.Boxes).U8(m.Type).I32(m.C1).I32(m.C2).Str("")
                : new PacketWriter(jp).Zeros(13);
        toClient[0xF3] = (jp, k) => enteringGpLobby ? null : new PacketWriter(jp);
        toClient[0xF4] = (jp, k) =>
        {
            if (!leavingGpLobby) return new PacketWriter(jp);
            leavingGpLobby = false;
            return new PacketWriter(0x251).U32(0);
        };

        fromClient[0x02] = Login;
        fromClient[0x08] = MakeRoom;
        fromClient[0x0A] = ChangeRoom;
        fromClient[0x12] = Shot;
        fromClient[0x1C] = FieldPickups;
        // prática (SPEC-jp983-pratica.md): sair pelo botão da prática; trocar o vento do próximo buraco; sair do Chip-in
        fromClient[0x130] = r => { kr.QuitPractice(); return null; };
        fromClient[0x141] = r => { kr.NewPracticeWind(); return null; };
        // Grand Zodiac: pronto para o 1º buraco, terminou, saiu do Chip-in; eco dos valores do 0x1EC (só ler)
        fromClient[0x137] = r => { kr.ZodiacReady(); return null; };
        fromClient[0x12C] = r => { kr.ZodiacFinish(complete: true); return null; };
        fromClient[0x131] = r => { kr.ZodiacFinish(complete: false); return null; };
        fromClient[0x12D] = r => null;
        // Grand Prix, fase 1 (SPEC-jp983-grandprix.md §6): lobby do GP = lobby normal; ainda sem salas de GP
        fromClient[0x176] = r => { enteringGpLobby = true; return Body(0x81, _ => { }); };
        fromClient[0x177] = r => { leavingGpLobby = true; return Body(0x82, _ => { }); };
    }

    /// <summary>u8 sub, u8 n, n × jogador do lobby (196 -> 200); no lobby do GP, depois da lista vai o 0x250.</summary>
    PacketWriter? LobbyUsers(ushort jp, byte[] k)
    {
        var w = new PacketWriter(jp).U8(k[0]).U8(k[1]);
        for (int i = 0; i < k[1]; i++) w.LobbyUser(k.AsSpan(2 + i * KrToJp.KrLobbyUserSize, KrToJp.KrLobbyUserSize));
        if (!enteringGpLobby) return w;
        enteringGpLobby = false;
        conn.SendTranslated(w);
        Reply(0x250, x => { if (GrandPrixLobby != null) GrandPrixLobby(x); else x.U32(0).U32(0).U32(0).F32(0); });   // entrou no lobby do GP
        return null;
    }

    /// <summary>
    /// 0x59 KR -> 0x5B JP (u8 vento, u8 cor, u16 direção, u8): no online o 983 não aplica o card de vento no cliente; o servidor manda o
    /// vento já reduzido pelos cards do personagem equipado e cor 1 (número azul; 983 0069f5f8 -> CCardManager+0xc98; frente B, B1421).
    /// Habilidade do Card.iff (vento em m = byte + 1): 13 = 2–5 m tira 1, 6–9 m tira o valor; 17 = 6–9 m tira o valor; 3 = tira o valor;
    /// 12 = só no 9 m. Mínimo 1 m.
    /// </summary>
    PacketWriter CardWind(ushort jp, byte[] k)
    {
        var w = new PacketWriter(jp).Bytes(k);
        if (k.Length < 2 || jpData == null || JpStructs.EquippedCharacter(kr.Player, null) is not { } ch) return w;
        int m = k[0] + 1, down = 0;
        foreach (var tid in Domain.Players.CharacterSystems.Cards(kr.Player, ch.Id))
            if (tid != 0 && jpData.Cards.TryGetValue(tid, out var c))
                down += c.Ability switch
                {
                    13 => m >= 6 ? c.AbilityValue : m >= 2 ? 1 : 0,
                    17 => m >= 6 ? c.AbilityValue : 0,
                    3 => c.AbilityValue,
                    12 => m >= 9 ? c.AbilityValue : 0,
                    _ => 0,
                };
        if (down <= 0) return w;
        var b = (byte[])k.Clone();
        b[0] = (byte)Math.Max(0, k[0] - down);
        b[1] = 1;
        return new PacketWriter(jp).Bytes(b);
    }

    /// <summary>
    /// 0x46 KR -> 0x48 JP: u8 sub (0 lista, 1 entrou, 2 saiu, 3 atualiza), u16 chave (o JP usa 0xFFFF, como a referência),
    /// [u8 n], jogador(es) 338 -> 348 (+ personagem 444 -> 513 fora dos modos em massa), u8 0 no fim (menos no 2).
    /// </summary>
    PacketWriter Slots(ushort jp, byte[] k)
    {
        byte sub = k[0];
        var w = new PacketWriter(jp, 0x400).U8(sub).U16(0xFFFF);
        var d = k.AsSpan(3);
        switch (sub)
        {
            case 2: return w.Bytes(d);
            case 3: return w.Bytes(d[..4]).Slot(d.Slice(4, KrToJp.KrSlotSize)).U8(0);
            case 1:
                SlotWithCharacter(w, d, d.Length >= KrToJp.KrSlotSize + KrToJp.KrCharacterSize);
                return w.U8(0);
            default:
                int n = d[0];
                if (n == 0) return w.U8(0).U8(0);
                bool withChar = (d.Length - 2) / n >= KrToJp.KrSlotSize + KrToJp.KrCharacterSize;
                int size = KrToJp.KrSlotSize + (withChar ? KrToJp.KrCharacterSize : 0);
                w.U8((byte)n);
                for (int i = 0; i < n; i++) SlotWithCharacter(w, d.Slice(1 + i * size, size), withChar);
                return w.U8(0);
        }
    }

    void SlotWithCharacter(PacketWriter w, ReadOnlySpan<byte> d, bool withChar)
    {
        w.Slot(d[..KrToJp.KrSlotSize]);
        if (withChar) w.Character(d.Slice(KrToJp.KrSlotSize, KrToJp.KrCharacterSize), Known, Extras);
    }

    /// <summary>
    /// 0x48 KR -> 0x4A JP: u16 sala, u8 tipo (o modo como o JP mostra), u8 mapa, u8 buracos, u8 ordem, + u32 natural/short
    /// game (JP), u8 máx, u8 galeria, u8 dormindo, u32 tacada, u32 jogo, u32 troféu, e a senha vira u8 "pública" (+ str
    /// senha se privada), str título.
    /// </summary>
    PacketWriter RoomSettings(ushort jp, byte[] k)
    {
        var r = new PacketReader();
        r.Reset(k, 0, k.Length);                                               // o "id" lido aqui é o u16 da sala
        byte tipo = KrToJp.TipoShow(r.U8());
        if (kr.CurrentRoom?.Settings.Zodiac is not (null or ZodiacKind.None)) tipo = ZodiacTipoShow;
        var w = new PacketWriter(jp).U16(r.Id).U8(tipo).Bytes(r.Bytes(2));
        r.U8();
        byte repeat = kr.CurrentRoom?.Settings.RepeatHole ?? 0;                // repetir buraco: ordem 4, u8 buraco, u32 fixos
        if (repeat > 0) w.U8(4).U8(repeat).U32(PracticeFixed);
        else w.U8(k[5]);
        w.U32(kr.CurrentRoom?.Settings.Natural == true ? 1u : 0u).Bytes(r.Bytes(3 + 12));   // bit 0 natural
        var password = r.Str(64);
        var title = r.Str(128);
        w.U8(password.Length == 0 ? (byte)1 : (byte)0);
        if (password.Length > 0) w.Str(password);
        return w.Str(title);
    }

    /// <summary>
    /// 0x49 KR -> 0x4B JP (equipamento de quem está na sala): o JP começa com i32 erro (0 = ok; referência: pacote04B, sem
    /// ele o cliente lê "u8 tipo + uid" como código de erro), depois u8 tipo, u32 uid, e 1 caddie 25, 2 bola u32,
    /// 3 club set 18 -> 28, 4 personagem 444 -> 513, outro: mascote 63 -> 62.
    /// </summary>
    /// <summary>Troca de taco (0x4B tipo 3): os 18 B do KR + os slots do taco do dono do uid (k[1..4]), como no 0x76.</summary>
    PacketWriter ClubOf(PacketWriter w, byte[] k, ReadOnlySpan<byte> d)
    {
        uint guid = BitConverter.ToUInt32(k, 1);
        Item? club = null;
        foreach (var x in kr.CurrentRoom?.Players ?? [])
            if (x.Guid == guid) { club = x.Player.Find(BitConverter.ToInt32(d)); break; }
        if (club == null) return w.Club(d);
        return w.Bytes(d[..18]).ClubSlots(club, kr.Context.Data as Domain.Players.IClubWorkshopData);
    }

    PacketWriter QuickEquip(ushort jp, byte[] k)
    {
        var w = new PacketWriter(jp).I32(0).Bytes(k.AsSpan(0, 5));
        var d = k.AsSpan(5);
        return k[0] switch
        {
            3 => ClubOf(w, k, d),
            4 => w.Character(d, Known, Extras),
            1 or 2 or 6 => w.Bytes(d),                                          // 6: efeito SP do lounge (fogos), 8 B
            _ => w.Mascot(d),
        };
    }

    /// <summary>
    /// 0x50 KR -> 0x52 JP: u8 mapa, u8 tipo, u8 ordem, u8 buracos, u32 (KR: semente do jogo; JP: troféu), u32 tacada, u32 jogo,
    /// 18 × (KR: u32 semente [+ u8 mapa no Shuffle], u8 nº | JP: u32 id, u8 pin, u8 mapa, u8 nº), u32 semente do jogo e
    /// 18 × u8 caixas/moedas (o registro das caixas do JP tem a posição x, y, z, que o KR não tem: vão 0; Wiz City fica sem
    /// itens). O pin (0..2, sorteado na referência) sai da semente do buraco: o mesmo para todos da sala.
    /// </summary>
    /// <summary>Nº da vez no repetir buraco (inícios de buraco desde o 0x50).</summary>
    int repeatSeq;

    /// <summary>Clima de cada buraco do 0x52 (na ordem de jogo), mandado no início do buraco.</summary>
    readonly byte[] holeWeather = new byte[18];

    const ushort SWeatherJp = 0x9E;

    /// <summary>Repetir buraco: o u8 buraco (offset 4, depois do u32 guid) vira o nº da vez.</summary>
    void RepeatSeq(byte[] k)
    {
        if (k.Length > 4 && kr.CurrentRoom?.Settings.RepeatHole > 0 && repeatSeq > 0) k[4] = (byte)repeatSeq;
    }

    /// <summary>
    /// Clima do buraco (0 bom, 2 chuva), tirado da semente do buraco para ser o mesmo em todos: chuva com a chance da
    /// config + a do artefato (referência: sorteio de tempo bom × chuva no CourseManager). O nublado (1) não é sorteado:
    /// como no oficial, ele só anuncia a chuva (ver <see cref="CloudBeforeRain"/>).
    /// </summary>
    static byte Weather(uint holeSeed, int rainPerMille) => holeSeed % 1000 < rainPerMille ? (byte)2 : (byte)0;

    /// <summary>Oficial: buraco nublado sempre vem antes de chuva; o bom que precede uma chuva vira nublado.</summary>
    public static void CloudBeforeRain(byte[] weather, int count)
    {
        for (int i = 0; i + 1 < count; i++)
            if (weather[i] == 0 && weather[i + 1] == 2) weather[i] = 1;
    }

    /// <summary>Pin do buraco (0..2) pela semente: o mesmo para todos, independente do clima.</summary>
    static byte Pin(uint holeSeed) => (byte)(holeSeed / 4000 % 3);

    const ushort SMascotSeed = 0x16A;
    const uint MascotSeedSalt = 0x9E3779B9;

    PacketWriter GameInit(ushort jp, byte[] k)
    {
        repeatSeq = 0;
        if (kr.CurrentRoom?.GrandPrix is { } gp) conn.SendTranslated(GrandPrixBots(gp));   // bots entre o 0x76 e o 0x52
        var room = kr.CurrentRoom;
        bool perHoleCourse = k[1] == RoomManager.ShuffleCourseMode;
        uint seed = BitConverter.ToUInt32(k, 4);
        bool repeat = room?.Settings.RepeatHole > 0;
        byte tipoShow = room?.Settings.Zodiac is not (null or ZodiacKind.None) ? ZodiacTipoShow : KrToJp.TipoShow(k[1]);
        int rain = kr.Context.World.Config.RainPerMille + Artifacts.RainPerMille(room?.Settings.Artifact ?? 0);
        var w = new PacketWriter(jp, 160).U8(k[0]).U8(tipoShow).U8(repeat ? (byte)4 : k[2]).U8(k[3])
            .U32((uint)(room != null ? RoomPackets.MatchTid(room) : 0)).Bytes(k.AsSpan(8, 8));
        int o = 16;
        for (int i = 0; i < 18; i++)
        {
            uint holeSeed = BitConverter.ToUInt32(k, o);
            byte course = perHoleCourse ? k[o + 4] : room is { GrandPrix: not null, HoleCourses.Length: 18 } ? room.HoleCourses[i] : (byte)(k[0] & 0x7F);
            byte number = k[o + (perHoleCourse ? 5 : 4)];
            o += perHoleCourse ? 6 : 5;
            holeWeather[i] = Weather(holeSeed, rain);
            w.U32(holeSeed).U8(Pin(holeSeed)).U8(course).U8(repeat ? (byte)(i + 1) : number);   // repetir: nº da vez (referência)
        }
        CloudBeforeRain(holeWeather, holeWeather.Length);
        // semente do efeito de mascote (só do JP, referência VersusBase: depois dos dados iniciais, a mesma para todos): u32
        if (room != null) Reply(SMascotSeed, x => x.U32(room.GameSeed ^ MascotSeedSalt));
        return FieldItemsTail(w, room, seed);
    }

    /// <summary>Índice do item no 0x52 (único na partida): vez &lt;&lt; 8 | índice no buraco (o do KR, até 60).</summary>
    public static uint FieldIndex(int order, int index) => (uint)(order << 8 | index);

    /// <summary>
    /// Fim do 0x52 e da intrusão (CGimmickContainer::LoadFromPacket 009103e0; SPEC-jp983-itens-campo.md): u32 semente +
    /// 18 × {u8 n, n × 0x24: i32 tipo, u32 índice, u32 0, u32 curso, u8 buraco físico, u8 vez (0-based, a que manda),
    /// u16 0, f32 x, y, z, u32 textura (tipo_location)}. Só itens com posição; repetir buraco: só na 1ª vez.
    /// </summary>
    public static PacketWriter FieldItemsTail(PacketWriter w, Room? room, uint seed)
    {
        var f = room?.Field;
        w.U32(f?.Seed ?? seed);
        Span<bool> seen = stackalloc bool[19];
        for (int i = 0; i < 18; i++)
        {
            byte hole = f != null && i < room!.HoleOrder.Length ? room.HoleOrder[i] : (byte)0;
            if (hole is < 1 or > 18 || seen[hole] || !f!.Spots.TryGetValue(hole, out var at)) { w.U8(0); continue; }
            seen[hole] = true;
            var types = f.PerHole[hole];
            int n = 0;
            foreach (var s in at) if (s != null) n++;
            w.U8((byte)Math.Min(n, 255));
            for (int k = 0, c = 0; k < at.Length && c < 255; k++)
                if (at[k] is { } s)
                {
                    c++;
                    w.I32(types[k]).U32(FieldIndex(i, k)).U32(0).U32(f.CourseOf[hole]).U8(hole).U8((byte)i).U16(0)
                        .F32(s.X).F32(s.Y).F32(s.Z).U32(s.Location);
                }
        }
        return w;
    }

    /// <summary>
    /// 0x1C (fim da tacada, CGolfRule::EncodePacket 00688e80): JP u8 é minha tacada, u8 n, n × {u8 tipo, u32 índice} ->
    /// KR n × {u8 tipo, u32 índice no buraco, u8 1, u8 textura}. Só passa item que existe com posição e é do buraco atual de
    /// quem tacou; o resto (forjado, de outro buraco) fica no log. O KR confere tipo e repetição e dá o prêmio.
    /// </summary>
    byte[]? FieldPickups(PacketReader r)
    {
        if (r.Remaining < 2) return Body(0x1C, w => w.Bytes(r.Bytes(r.Remaining)));
        byte mine = r.U8();
        int n = r.U8();
        var room = kr.CurrentRoom;
        var target = mine == 1 && n > 0 ? kr.ShotEndTarget() : null;
        var ok = new List<(byte Type, uint Index, byte Tex)>();
        for (int i = 0; i < n && r.Remaining >= 5; i++)
        {
            byte type = r.U8();
            uint index = r.U32();
            if (mine != 1) continue;
            int order = (int)(index >> 8);
            uint k = index & 0xFF;
            byte hole = room != null && order < room.HoleOrder.Length ? room.HoleOrder[order] : (byte)0;
            if (room?.Field?.SpotOf(hole, k) is { } s && target?.Hole == hole)
                ok.Add((type, k, s.Location == FieldSpot.EdgeGreen ? (byte)1 : (byte)2));   // textura do KR: 1 borda do green (até 50 pang)
            else Log.Warn($"{conn} JP: item de campo suspeito (tipo {type}, índice 0x{index:X}, buraco {hole}, atual {target?.Hole})");
        }
        return Body(0x1C, w =>
        {
            w.U8(mine).U8((byte)ok.Count);
            foreach (var (type, index, tex) in ok) w.U8(type).U32(index).U8(1).U8(tex);
            w.Bytes(r.Bytes(r.Remaining));
        });
    }

    /// <summary>
    /// 0x74 KR -> 0x76 JP: u8 tipo da sala (o KR manda 0) e, fora dos modos em massa, u8 n + por jogador o bloco do jogador
    /// (2962 -> 0x31C6, refeito do Domain com o índice da sala e o guid do jogador na sala), SYSTEMTIME, u8 m, m cards.
    /// Nos modos em massa a referência manda n = 1 (u8 tipo, u32 1, SYSTEMTIME).
    /// </summary>
    PacketWriter GamePlayers(ushort jp, byte[] k)
    {
        var room = kr.CurrentRoom;
        if (room?.Settings.Zodiac is ZodiacKind zk and not ZodiacKind.None)    // GZ: copo e pontos (0x1F9) antes do 0x76; ticket
        {
            int cup = Domain.Players.ZodiacPoints.Cup(ZodiacPoints);
            Reply(0x1F9, w => w.I32(cup).I32(cup).U32((uint)ZodiacPoints));
            if (zk == ZodiacKind.ChipIn && ItemSpent != null) _ = ItemSpent(ChipInTicket, 1);
            return new PacketWriter(jp).U8(ZodiacTipoShow).U32(1).Bytes(k.AsSpan(5, Math.Min(16, k.Length - 5)));
        }
        if (room?.GrandPrix is { } gp)                                         // GP: 0x253 ok, bots criados, ticket gasto
        {
            Reply(0x253, w => w.I32(0));
            if (gp.Bots.Count == 0 && jpData != null)
                gp.Bots.AddRange(GrandPrix.MakeBots(gp.Def, jpData.GrandPrixBots(gp.Def.BotClass), room.Players.Count, room.HoleOrder,
                    Random.Shared));
            if (GrandPrixStarted != null) _ = GrandPrixStarted(gp.Def);
        }
        int n = k[1];
        byte tipo = KrToJp.TipoShow((byte)(room?.Settings.Mode ?? 0));
        if (room == null || k.Length < 2 + KrToJp.KrUserInfoSize)                // modos em massa: u8 tipo, u32 1, SYSTEMTIME
            return new PacketWriter(jp).U8(tipo).U32(1).Bytes(k.AsSpan(5, Math.Min(16, k.Length - 5)));
        var w = new PacketWriter(jp, 8 + n * (JpStructs.PlayerBlockSize + 17 + 16 * 0x41)).U8(tipo).U8((byte)n);
        int o = 2;
        for (int i = 0; i < n; i++)
        {
            uint guid = BitConverter.ToUInt32(k, o + 91);                      // sUserInfo.info.dwGuid
            RoomPlayer? rp = null;
            foreach (var x in room.Players)
                if (x.Guid == guid) { rp = x; break; }
            if (rp != null) w.PlayerBlock(rp.Player, Known, (ushort)room.Index, guid, kr.Context.Data as Domain.Players.IClubWorkshopData);
            else w.Zeros(JpStructs.PlayerBlockSize);
            o += KrToJp.KrUserInfoSize;
            int m = k[o + 16];
            w.Bytes(k.AsSpan(o, 17 + m * 0x41));
            o += 17 + m * 0x41;
        }
        return w;
    }

    /// <summary>0x02 do JP (JpGameHandler.ReadLogin) -> 0x02 do KR (id, uid, MemberNo, u16, chave, versão, versão de pacote, u32).</summary>
    static byte[]? Login(PacketReader r)
    {
        var (id, uid, _, _, _, gkey) = JpGameHandler.ReadLogin(r);
        return Body(0x02, w => w.Str(id).U32(uid).U32(uid).U16(0).Str(gkey).Str(KR645.Kr645.ClientVersion).U32(KR645.Kr645.PacketVersion).U32(0));
    }

    /// <summary>
    /// 0x08 do JP (referência: requestMakeRoom): u8, u32 tacada, u32 jogo, u8 máx, u8 modo, u8 buracos, u8 mapa, u8 ordem,
    /// u32 natural/short game, str nome, str senha, u32 artefato -> o 0x08 do KR (sem as flags e o artefato; modo 18 -> 14).
    /// O modo 19 (repetir buraco, que o KR não tem) traz 5 bytes a mais antes das flags.
    /// </summary>
    byte[]? MakeRoom(PacketReader r)
    {
        var head = r.Bytes(14).ToArray();
        byte type = head[10], repeat = 0;
        if (head[13] == 4)                                                     // ordem 4 = repetir buraco: u8 buraco, u32 fixos
        {
            repeat = r.U8();
            uint fixedFlags = r.U32();
            head[13] = 0;
            Log.Info($"{conn} JP: prática repetindo o buraco {repeat} (fixos 0x{fixedFlags:X})");
        }
        bool chipIn = type == (byte)ZodiacKind.ChipIn;
        if (chipIn && (kr.Player.FindType(ChipInTicket)?.Quantity ?? 0) < 1)  // Chip-in Practice: precisa do ticket
        {
            Log.Info($"{conn} JP: Chip-in Practice sem ticket");
            Reply(0x49, w => w.U8((byte)JoinResult.NotFound));
            return null;
        }
        // GM cria a sala do evento como uma sala comum (dono, nome, senha e máximo dele; começa pelo botão)
        bool gmZodiac = type is (byte)ZodiacKind.Intermediate or (byte)ZodiacKind.Advanced && (kr.Player.IdentityFlags & GmFlag) != 0;
        if (gmZodiac) Log.Info($"{conn} JP: GM cria sala do Grand Zodiac (tipo {type})");
        if (!chipIn && !gmZodiac && type is >= 11 and <= 17 or Jp983.GrandPrixType or 3)   // GZ de evento (servidor), GP (pelo 0x179); 3 não existe no JP
        {
            Log.Info($"{conn} JP: sala do tipo {type} recusada");
            Reply(0x49, w => w.U8((byte)JoinResult.NotFound));
            return null;
        }
        bool practice = type == Jp983.PracticeType;
        head[10] = practice || chipIn || gmZodiac ? (byte)GameMode.Tournament : KrToJp.ModeFromJp(type);
        uint flags = r.U32();                                                  // bit 0 natural, bit 1 short game
        var name = r.Str(64);
        var password = r.Str(32);
        uint artifact = r.Remaining >= 4 ? r.U32() : 0;                        // artefato da sala (o KR confere o item e a mana)
        return Body(0x08, w => w.Bytes(head).Str(name).Str(password).U8(practice ? (byte)1 : (byte)0).U8(repeat)
            .U8(chipIn || gmZodiac ? type : (byte)0).U32(artifact).U32(flags & 1));
    }

    /// <summary>Artefato da sala de um sRoomInfo do KR (pelo número da sala, u16 no offset 73).</summary>
    uint ArtifactOf(ReadOnlySpan<byte> k) =>
        (uint)(kr.Context.World.Rooms.Get(BitConverter.ToUInt16(k.Slice(73, 2)))?.Settings.Artifact ?? 0);

    /// <summary>Flags do JP da sala (+201; bit 0 = natural/clássico) de um sRoomInfo do KR.</summary>
    uint FlagsOf(ReadOnlySpan<byte> k) =>
        kr.Context.World.Rooms.Get(BitConverter.ToUInt16(k.Slice(73, 2)))?.Settings.Natural == true ? 1u : 0u;

    /// <summary>Conta de GM (identity_flags, como o GameHandler.IsGm).</summary>
    internal const int GmFlag = 0x04;

    /// <summary>Ticket do Chip-in Practice (gasto no início).</summary>
    const int ChipInTicket = 0x1A00017E;
    /// <summary>"Tipo de exibição" do Grand Zodiac (11 para 11, 13 e 14).</summary>
    const byte ZodiacTipoShow = 11;

    /// <summary>"Fixos" do repetir buraco no 0x4A (referência: fixed_hole = 7, pin/vento/clima fixos).</summary>
    const uint PracticeFixed = 7;

    /// <summary>
    /// RoomInfo do JP da sala (221); na prática, tipo real (+196) 19 e, no repetir buraco, ordem (+91) 4 (referência:
    /// type_extend = tipo para os tipos de 11 em diante; tipo_show = 4).
    /// </summary>
    byte[] PracticeRoom(ReadOnlySpan<byte> k)
    {
        using var t = new PacketWriter(0, KrToJp.RoomSize + 8).Room(k, ArtifactOf(k), FlagsOf(k));
        var b = t.Body[2..].ToArray();
        if (kr.CurrentRoom?.Settings is { Practice: true } s)
        {
            b[196] = Jp983.PracticeType;
            if (s.RepeatHole > 0) b[91] = 4;
        }
        if (kr.CurrentRoom?.Settings is { Zodiac: not ZodiacKind.None } zs)      // Grand Zodiac: tipo de exibição 11, tipo real
        {
            b[88] = ZodiacTipoShow;
            b[196] = (byte)zs.Zodiac;
        }
        if (kr.CurrentRoom?.GrandPrix is { } gp)                                // Grand Prix (SPEC §3.3): tipo 0x14, regra, bloco GP
        {
            var d = gp.Def;
            b[196] = Jp983.GrandPrixType;
            BitConverter.TryWriteBytes(b.AsSpan(197), d.Rule);
            BitConverter.TryWriteBytes(b.AsSpan(201), (d.Natural ? 1 : 0) | (d.ShortGame ? 2 : 0));
            BitConverter.TryWriteBytes(b.AsSpan(205), d.Id);
            BitConverter.TryWriteBytes(b.AsSpan(209), d.Link);
            BitConverter.TryWriteBytes(b.AsSpan(213), d.TimeHoleSeconds * 1000);
            BitConverter.TryWriteBytes(b.AsSpan(217), 1);
        }
        return b;
    }

    /// <summary>Corpo do 0x250 (lobby do GP: concluídos e média), escrito pelo JpSystems.</summary>
    public Action<PacketWriter>? GrandPrixLobby { get; set; }
    /// <summary>Início do GP: gastar o ticket (JpSystems, assíncrono).</summary>
    public Func<GrandPrixDef, Task>? GrandPrixStarted { get; set; }
    /// <summary>Pontos do Grand Zodiac da conta (carregados no login): o tamanho do copo no 0x1F9.</summary>
    public int ZodiacPoints { get; set; }
    /// <summary>Gastar um item do inventário (typeid, quantidade) com o 0x216 (JpSystems, assíncrono): ticket do Chip-in.</summary>
    public Func<int, int, Task>? ItemSpent { get; set; }

    /// <summary>
    /// 0x256 (bots do GP, logo depois do 0x76): u32 0, u16 n, n × (u32 id, u8 h, h × (u32 mapa, u32 buraco, i32 placar,
    /// u32 0, u64 pang, u64 bônus, u64 0)).
    /// </summary>
    static PacketWriter GrandPrixBots(GrandPrixRun run)
    {
        var w = new PacketWriter(0x256, 16 + run.Bots.Count * (5 + 40 * run.Def.Holes)).U32(0).U16((ushort)run.Bots.Count);
        foreach (var b in run.Bots)
        {
            w.I32(b.Id).U8((byte)b.Holes.Count);
            foreach (var h in b.Holes) w.U32(h.Course).U32(h.Hole).I32(h.Score).U32(0).U64((ulong)h.Pang).U64((ulong)h.Bonus).U64(0);
        }
        return w;
    }

    /// <summary>
    /// 0x0A (mudar a sala; referência: ROOM_INFO_CHANGE): u16, u8 n, n × (u8 chave, valor): 0 título e 1 senha = str,
    /// 6 tempo da tacada = u16 segundos (o KR: u8), 2..5 e 7..11 = u8 (2 modo, 18 -> 14; 10 galeria; 11 buraco que a
    /// prática repete), 12 buraco fixo, 13 artefato e 14 natural = u32. Vão ao KR as que ele trata (0..9, 11, 13 e 14; a 6 vai como 15 = u32 ms).
    /// </summary>
    static byte[]? ChangeRoom(PacketReader r)
    {
        ushort room = r.U16();
        int n = r.U8(), kept = 0;
        using var keys = new PacketWriter(0);
        for (int i = 0; i < n && r.Remaining > 0; i++)
        {
            byte key = r.U8();
            switch (key)
            {
                case 0 or 1: keys.U8(key).Str(r.Str(64)); break;
                case 2: keys.U8(key).U8(KrToJp.ModeFromJp(r.U8())); break;
                case 6: keys.U8(15).U32(r.U16() * 1000u); break;              // tempo da tacada em ms (o 6 do KR é u8: 300 s virava 255)
                case 10: r.U8(); continue;
                case <= 11: keys.U8(key).U8(r.U8()); break;
                case 13: keys.U8(key).U32(r.U32()); break;                   // artefato
                case 14: keys.U8(key).U32(r.U32() & 1); break;              // natural
                case 12: r.U32(); continue;
                default: i = n; continue;                                  // desconhecida: para aqui
            }
            kept++;
        }
        r.Skip(r.Remaining);
        return Body(0x0A, w => w.U16(room).U8((byte)kept).Bytes(keys.Body[2..]));
    }

    /// <summary>
    /// 0x12 do JP (referência: requestInitShot): u16 opção, [opção 1: u8 + i32 + i32 do power shot], 50 bytes da tacada ->
    /// 0x12 do KR com u16 0 e os 50 bytes (o KR acha os 46 depois do u16 e repassa o resto no eco) + os 12 do fim
    /// (ClassicWind 8 + tempo de sincronia 4): o eco 0x55 do 983 tem u32 + 50 + 12.
    /// </summary>
    static byte[]? Shot(PacketReader r)
    {
        var d = r.Bytes(r.Remaining);
        int start = d.Length switch
        {
            ShotSize => 0,
            _ when d.Length >= 2 + 9 + ShotSize && d[0] == 1 && d[1] == 0 => 2 + 9,
            _ when d.Length >= 2 + ShotSize => 2,
            _ => Math.Max(0, d.Length - ShotSize),
        };
        var shot = d.Slice(start, Math.Min(ShotSize + ShotSyncSize, d.Length - start)).ToArray();
        return Body(0x12, w => w.U16(0).Bytes(shot).Zeros(ShotSize + ShotSyncSize - shot.Length));
    }
}
