using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.KR645.Game;

/// <summary>Lista de salas e sala de espera (docs/server/protocolo/SPEC-room.md).</summary>
public sealed partial class GameHandler
{
    // ids C->S
    const ushort CEnterRoomList = 0x81, CLeaveRoomList = 0x82, CMakeRoom = 0x08, CJoinRoom = 0x09, CChangeRoom = 0x0A,
        CReady = 0x0D, CStart = 0x0E, CLeaveRoom = 0x0F, CTeam = 0x10, CChat = 0x03, CGameOptions = 0x69, CRoomAction = 0x63;

    Room? room;
    bool slotsResent;
    /// <summary>Sala em que o jogador está (null = fora de sala); a tradução de versão (JP) precisa dos jogadores dela.</summary>
    public Room? CurrentRoom => room;
    RoomManager Rooms => ctx.World.Rooms;

    /// <summary>Trata os pacotes de sala; false = não é de sala.</summary>
    async ValueTask<bool> HandleRoomAsync(PacketReader p)
    {
        switch (p.Id)
        {
            case CEnterRoomList:
                lock (Rooms.Sync)
                {
                    bool already = Rooms.Lobby.Contains(this);
                    if (!already) LobbyUser(LobbyUserAdd);                      // os outros me veem
                    Rooms.Lobby.Add(this);
                    conn.Send(new PacketWriter(RoomPackets.SRoomListOn));
                    conn.Send(RoomPackets.RoomListAll(Rooms));
                    SendLobbyUsers();
                }
                return true;
            case CLeaveRoomList:
                lock (Rooms.Sync) Rooms.Lobby.Remove(this);
                conn.Send(new PacketWriter(RoomPackets.SRoomListOff));
                return true;
            case CMakeRoom: MakeRoom(p); return true;
            case CJoinRoom: JoinRoom(p.U16(), p.Str(16)); return true;
            case CChangeRoom: ChangeRoom(p); return true;
            case CReady: SetReady(p.U8() == 0); return true;
            case CStart: p.U32(); Start(); return true;
            case CLeaveRoom: lock (Rooms.Sync) LeaveRoom(notifySelf: true); return true;
            case CTeam: SetTeam(p.U8()); return true;
            case CChat: await ChatAsync(p.Str(32), p.Str(256)); return true;
            case CGameOptions: p.Skip(p.Remaining); return true;           // opções e macros: sem resposta
            case CRoomAction: RoomAction(p); return true;
            case CAvatarData: LoungeSpState(p.U32()); p.Skip(p.Remaining); return true;   // lounge: estado SP do avatar
            default: return false;
        }
    }

    /// <summary>0x44 sub: 1 entrou na lista, 2 saiu, 3 atualiza (sala em que está; 0xFFFF = no lobby).</summary>
    const byte LobbyUserAdd = 1, LobbyUserRemove = 2, LobbyUserUpdate = 3;
    const int MaxLobbyUsers = 250;                                         // 0xC4 cada: cabe num pacote

    /// <summary>Minha entrada na lista de usuários de quem está na lista de salas (eu inclusive, se estiver).</summary>
    void LobbyUser(byte sub) =>
        Lobby(new PacketWriter(RoomPackets.SLobbyUsers).U8(sub).U8(1).Struct(RoomPackets.BriefUser(Player, (ushort)(room?.Index ?? 0xFFFF), GmVisibleState)));

    /// <summary>Lista completa do meu canal (no lobby e em salas), com a sala de cada um.</summary>
    void SendLobbyUsers()
    {
        var list = new List<GameHandler>();
        foreach (var s in ctx.World.Online)
            if (s is GameHandler h && h.player != null && h.channel == channel && list.Count < MaxLobbyUsers) list.Add(h);
        var w = new PacketWriter(RoomPackets.SLobbyUsers, 8 + list.Count * 0xC4).U8(LobbyUserAdd).U8((byte)list.Count);
        foreach (var h in list) w.Struct(RoomPackets.BriefUser(h.Player, (ushort)(h.room?.Index ?? 0xFFFF), h.GmVisibleState));
        conn.Send(w);
    }

    void Lobby(PacketWriter w)
    {
        foreach (var s in Rooms.Lobby) ((GameHandler)s).Connection.Send(w.Body);
        w.Dispose();
    }

    /// <summary>Atualização da sala na lista do lobby (a sala de prática não aparece).</summary>
    void LobbyRoom(byte sub, Room r)
    {
        if (!r.Settings.Hidden) Lobby(RoomPackets.RoomList(sub, r));
    }

    /// <summary>
    /// 0x08: u8 quick, u32 tacada, u32 jogo, u8 máx, u8 modo, u8 buracos, u8 mapa, u8 ordem, str título, str senha. Extensão
    /// do servidor (só a camada JP manda): u8 prática, u8 buraco repetido (RoomSettings.Practice/RepeatHole).
    /// </summary>
    void MakeRoom(PacketReader p)
    {
        p.U8();                                                 // quick
        var s = new RoomSettings { ShotTimeMs = p.U32(), GameTimeMs = p.U32(), MaxPlayers = p.U8(), Mode = (GameMode)p.U8(), Holes = p.U8(), Course = p.U8(), HoleType = p.U8() };
        s.Title = p.Str(64);
        s.Password = p.Str(32);
        if (p.Remaining >= 2) (s.Practice, s.RepeatHole) = (p.U8() != 0, p.U8());
        if (p.Remaining >= 1) s.Zodiac = (ZodiacKind)p.U8();                  // extensão (camada JP): Grand Zodiac
        if (p.Remaining >= 4) s.Artifact = (int)p.U32();                      // extensão (camada JP): artefato da sala
        if (p.Remaining >= 4) s.Natural = (p.U32() & 1) != 0;                // extensão (camada JP): modo natural
        if (s.Artifact != 0 && !Artifacts.CanUse(Player, s.Artifact)) s.Artifact = 0;
        s.ByGm = IsGm;
        if (s.Mode == GameMode.GuildMatch && (Player.Guild is not { } g || !Domain.Guilds.GuildClass.IsMember(g.Class)))
        {
            conn.Send(RoomPackets.EnterRoomFailed(JoinResult.GuildRequired));   // "길드에 가입해야 합니다."
            return;
        }
        if (QuitRate.Barred(Player, s.Mode)) { conn.Send(RoomPackets.EnterRoomFailed(JoinResult.Restricted)); return; }
        lock (Rooms.Sync)
        {
            if (room != null) LeaveRoom(notifySelf: false);
            var r = Rooms.Create(s, Player.AccountId);
            Log.Info($"{conn} criou a sala {r.Index}: modo={r.Settings.Mode} mapa={r.Settings.Course} buracos={r.Settings.Holes} '{r.Settings.Title}'");
            EnterRoom(r);
        }
    }

    void JoinRoom(ushort index, string password)
    {
        lock (Rooms.Sync)
        {
            if (room != null) return;
            var r = Rooms.Get(index);
            var res = RoomManager.CanJoin(r, password, Player);
            if (res != JoinResult.Ok) { conn.Send(RoomPackets.EnterRoomFailed(res)); return; }
            EnterRoom(r!);
        }
    }

    /// <summary>Entra na sala: avisa quem já está, manda a sala completa (0x47, 0x48, 0x46) e atualiza a lista.</summary>
    void EnterRoom(Room r)
    {
        var rp = RoomManager.Join(r, new RoomPlayer { Guid = (uint)Player.AccountId, Player = Player, Session = this });
        room = r;
        slotsResent = false;
        Rooms.Lobby.Remove(this);
        LobbyUser(LobbyUserUpdate);                                         // a lista do lobby mostra a sala
        // GuildMatch: quem já está só aprende a guilda nova pelo 0x45 da própria sala (antes do slot, para achar o lado)
        if (r.Settings.Mode == GameMode.GuildMatch) InGameOutput.Broadcast(r, RoomPackets.RoomList(3, r), except: this);
        // GM que entra numa sala de espera assume o dono; o antigo vira jogador comum no mesmo slot (sem pronto)
        RoomPlayer? oldMaster = null;
        if (IsGm && !rp.Master && r.State == RoomState.Waiting && !IsLounge(r))
            foreach (var m in r.Players)
                if (m.Master && m != rp) { oldMaster = m; m.Master = false; m.Ready = false; rp.Master = true; rp.Ready = true; r.OwnerId = Player.AccountId; break; }
        InGameOutput.Broadcast(r, RoomPackets.SlotAdd(r, rp), except: this);
        if (oldMaster != null)
        {
            InGameOutput.Broadcast(r, new PacketWriter(RoomPackets.SNewMaster).U32(rp.Guid).U16(RoomPackets.SlotKey(r)), except: this);
            InGameOutput.Broadcast(r, new PacketWriter(RoomPackets.SReady).U32(oldMaster.Guid).U8(1), except: this);   // 1 = não pronto
            Log.Info($"{conn} GM assumiu a sala {r.Index} (dono anterior {oldMaster.Guid})");
        }
        conn.Send(RoomPackets.EnterRoom(r));
        conn.Send(RoomPackets.Settings(r));
        conn.Send(RoomPackets.SlotsFull(r));
        if (IsLounge(r)) StartLoungeSync(r);                                // a avatar task ainda não existia: reenvia
        LobbyRoom(1, r);
    }

    /// <summary>
    /// Sai da sala (chamado sob o lock). notifySelf: volta o cliente para a lista de salas. deliberate: só a saída pedida
    /// pelo jogador é abandono; queda de conexão e GM não contam nem custam pang (a bola dos buracos concluídos segue valendo).
    /// </summary>
    void LeaveRoom(bool notifySelf, bool deliberate = true)
    {
        var r = room;
        if (r == null) return;
        CloseMyShopLocked();                                                // a loja fecha com quem sai do lounge
        LeaveVisitedShopLocked();
        room = null;
        MessengerPlaying(false);
        var rp = r.Find(this);
        if (rp != null)
        {
            var game = r.Game;
            int quitHoles = deliberate ? game?.QuitHoles(rp.Guid) ?? 0 : 0;
            var ball = TakeBall(game, rp.Guid);
            if (ball.Holes > 0) _ = Task.Run(() => SpendBallAsync(ball));
            bool alone = r.Settings.Mode == GameMode.Stroke && r.Players.Count == 1;
            var (newMaster, closed) = Rooms.Leave(r, rp);
            if (quitHoles > 0 && game?.Aborted != true) CountQuit(r.Settings.Mode, quitHoles, alone);
            if (closed)
            {
                LobbyRoom(2, r);
                Log.Info($"sala {r.Index} fechada");
            }
            else
            {
                if (r.Settings.Mode == GameMode.GuildMatch) InGameOutput.Broadcast(r, RoomPackets.RoomList(3, r));   // lado pode ter esvaziado
                InGameOutput.Broadcast(r, RoomPackets.SlotRemove(r, rp.Guid));
                if (newMaster != null) InGameOutput.Broadcast(r, new PacketWriter(RoomPackets.SNewMaster).U32(newMaster.Guid).U16(RoomPackets.SlotKey(r)));
                LobbyRoom(3, r);
            }
        }
        if (pendingEnd == null) lastGameStats = null;                      // parcial da partida não vaza para a próxima
        if (!notifySelf) return;
        conn.Send(new PacketWriter(RoomPackets.SLeftRoom).U16(0xFFFF));     // volta para ROOMLIST (ou TOPPAGE)
        Rooms.Lobby.Add(this);
        conn.Send(RoomPackets.RoomListAll(Rooms));
        LobbyUser(LobbyUserUpdate);                                         // fora da sala, para todos e para mim
    }

    /// <summary>0x0A: u16 0xFFFF, u8 n, n × (u8 chave, valor). Só o dono muda a sala.</summary>
    void ChangeRoom(PacketReader p)
    {
        p.U16();
        int n = p.U8();
        lock (Rooms.Sync)
        {
            var r = room;
            if (r == null || r.State != RoomState.Waiting || r.Find(this) is not { Master: true }) return;
            var s = r.Settings;
            for (int i = 0; i < n; i++)
            {
                switch (p.U8())
                {
                    case 0: s.Title = p.Str(64); break;
                    case 1: s.Password = p.Str(32); break;
                    case 2:                                                  // GuildMatch só ao criar (lados e regras próprias)
                    {
                        var m = (GameMode)p.U8();
                        if (m != GameMode.GuildMatch && s.Mode != GameMode.GuildMatch) s.Mode = m;
                        break;
                    }
                    case 3: s.Course = Rooms.ValidCourse(p.U8()); break;
                    case 4: s.Holes = p.U8(); break;
                    case 5:
                    {
                        s.HoleType = p.U8();                             // prática: ordem 4 = repetir buraco (o 1º, até escolher)
                        if (s.Practice) s.RepeatHole = s.HoleType == 4 ? Math.Max(s.RepeatHole, (byte)1) : (byte)0;
                        break;
                    }
                    case 6: s.ShotTimeMs = p.U8() * 1000u; break;
                    case 7: s.MaxPlayers = p.U8(); break;
                    case 8: { var v = p.U8(); s.GameTimeMs = v * (s.Mode == GameMode.NewApproach ? 1000u : 60000u); break; }
                    case 9: s.Sleep = p.U8() != 0; break;
                    case 13:                                             // extensão (camada JP): artefato (o dono tem o item e a mana)
                    {
                        int art = (int)p.U32();
                        s.Artifact = art == 0 || Artifacts.CanUse(Player, art) ? art : 0;
                        break;
                    }
                    case 14: s.Natural = (p.U32() & 1) != 0; break;     // extensão (camada JP): modo natural
                    case 15: s.ShotTimeMs = Math.Min(p.U32(), 3_600_000u); break;   // extensão (camada JP): tempo da tacada em ms
                    case 11:                                             // extensão (camada JP): buraco que a prática repete
                    {
                        byte hole = p.U8();
                        if (s.Practice && hole is >= 1 and <= 18) s.RepeatHole = hole;
                        break;
                    }
                    default: i = n; break;                       // chave desconhecida: para de ler
                }
            }
            s.Normalize();
            InGameOutput.Broadcast(r, RoomPackets.Settings(r));
            LobbyRoom(3, r);
        }
    }

    void SetReady(bool ready)
    {
        lock (Rooms.Sync)
        {
            if (room?.Find(this) is not { } me || room.State != RoomState.Waiting) return;
            me.Ready = ready;
            InGameOutput.Broadcast(room, new PacketWriter(RoomPackets.SReady).U32(me.Guid).U8((byte)(ready ? 0 : 1)));
        }
    }

    void SetTeam(byte team)
    {
        lock (Rooms.Sync)
        {
            if (room?.Find(this) is not { } me || room.Settings.Mode == GameMode.GuildMatch) return;   // GuildMatch: time = guilda
            me.Team = (byte)(team & 3);
            InGameOutput.Broadcast(room, new PacketWriter(RoomPackets.STeam).U32(me.Guid).U8(me.Team));
        }
    }

    /// <summary>GuildMatch: duas guildas, os dois lados com a mesma quantidade (≥ 1). null = pode começar.</summary>
    static byte? GuildStartError(Room r)
    {
        if (r.GuildSides[0] == null || r.GuildSides[1] == null) return 1;
        int red = 0, blue = 0;
        foreach (var p in r.Players)
            if (!p.IsBot) { if (p.Team == 0) red++; else blue++; }
        return red == 0 || blue == 0 ? (byte)1 : red != blue ? (byte)2 : null;
    }

    /// <summary>Começa a partida da sala sem ser o dono (Grand Prix com horário: o servidor começa sozinho).</summary>
    public void StartAsServer() => Start(force: true);
    const ushort SGameRate = 0x75;
    const uint GameRatePercent = 100;

    /// <summary>Cada humano da partida registra os outros como jogadores recentes (perfil do JP, 0x10E).</summary>
    void RecordRecentPlayers(Room r)
    {
        if (ctx.RecentPlayers is not { } store || r.Settings.Hidden) return;
        var ids = new List<long>();
        foreach (var rp in r.Players) if (!rp.IsBot) ids.Add(rp.Player.AccountId);
        if (ids.Count < 2) return;
        foreach (var me in ids)
        {
            var others = new List<long>(ids.Count - 1);
            foreach (var o in ids) if (o != me) others.Add(o);
            _ = Task.Run(async () =>
            {
                try { await store.AddAsync(me, others); }
                catch (Exception e) { Log.Warn($"jogadores recentes de {me}: {e.Message}"); }
            });
        }
    }

    void Start(bool force = false)
    {
        lock (Rooms.Sync)
        {
            var r = room;
            if (r == null || r.State != RoomState.Waiting) return;
            if ((!force && r.Find(this) is not { Master: true }) || IsLounge(r)) { conn.Send(new PacketWriter(RoomPackets.SStartFailed).U8(1)); return; }
            if (r.Settings.Mode == GameMode.GuildMatch && GuildStartError(r) is { } err)
            {
                conn.Send(new PacketWriter(RoomPackets.SStartFailed).U8(err));     // 1 falta gente/guilda, 2 times desiguais
                return;
            }
            RoomManager.PrepareStart(r, Random.Shared, Rooms.Courses, ctx.FieldSpots);
            var cfg = ctx.World.Config;
            var botDelay = TimeSpan.FromSeconds(cfg.BotDelaySeconds);
            // aprendizado guardado do nível (memória dos buracos deste mapa + calibração); cresce a cada partida
            var golfer = BotGolfer.For(r.BotLevel, Random.Shared, cfg.BotAccuracy, ctx.BotKnowledge.For(r.BotLevel));
            golfer.Course = r.CoursePlayed;
            if (r.Bot is { } bot)                                       // kit do nível: vai no 0x74 e muda a física nos clientes
            {
                ctx.Players.EquipBot(bot.Player, r.BotLevel);
                var (stats, driveUp) = ctx.Data.PlayStats(bot.Player);
                (golfer.PowerStat, golfer.DriveUp, golfer.AccuracyStat) = (stats[0], driveUp, stats[2]);
                foreach (var it in bot.Player.Equip.ItemSlots) if (it != 0) golfer.Items.Add(it);
                Log.Info($"sala {r.Index}: bot {r.BotLevel} nível={bot.Player.Level} stats={string.Join('/', stats)} anéis=+{driveUp}jd");
            }
            if (r.Settings.Zodiac != ZodiacKind.None)
            {
                var massOut = new MassOutput(r, cfg.TreasureHunter);
                if (ctx.ZodiacGame?.Invoke(r, massOut, Rooms.Sync) is not { } zg)
                {
                    r.State = RoomState.Waiting;
                    conn.Send(new PacketWriter(RoomPackets.SStartFailed).U8(1));
                    return;
                }
                r.Game = zg;
            }
            else r.Game = MassGame.IsMass(r.Settings.Mode)
                ? MassGame.For(r, new MassOutput(r, cfg.TreasureHunter), Rooms.Sync, botDelay, cfg.Rewards.TrophiesCountBots)
                : StrokeGame.For(r, new InGameOutput(r, cfg.BotPasses, golfer, cfg.BotFastForward, TimeSpan.FromSeconds(cfg.BotFastForwardDelaySeconds), cfg.TreasureHunter), Rooms.Sync,
                    botDelay, TimeSpan.FromSeconds(cfg.TeeFallbackSeconds));
            if (r.Game is StrokeGame vs) vs.SoloAfterHoles = cfg.VersusSoloAfterHoles;
            RecordRecentPlayers(r);
            // taxa de pang (u32 %): o cliente calcula com ela o bônus que mostra na partida e no resultado e reenvia o
            // equipamento (0x0C tipo 7). O servidor não multiplica o pang: 100. Vai ANTES do 0x74: o cliente trata a taxa com
            // ClearRivalVars (zera os sUserInfo da partida); depois do 0x74 apagava o taco/personagem (983 cai em FindClubSet)
            InGameOutput.Broadcast(r, new PacketWriter(SGameRate).U32(GameRatePercent));
            InGameOutput.Broadcast(r, RoomPackets.GamePlayers(r, ctx.Data.Cards));
            if (r.Game is TourneyGame { Pairs.Count: > 0 } tg)          // GuildMatch: pares antes do 0x50 (só o lobby trata)
                InGameOutput.Broadcast(r, MassOutput.GuildPairs(tg.Pairs));
            InGameOutput.Broadcast(r, RoomPackets.GameInit(r));         // o cliente troca para a tela da partida
            foreach (var rp in r.Players)                               // amigos do mensageiro: "jogando"
                if (rp.Session is GameHandler gh) gh.MessengerPlaying(true);
            LobbyRoom(3, r);
            Log.Info($"sala {r.Index}: início mapa={r.CoursePlayed} buracos={r.Settings.Holes} jogadores={r.Players.Count}");
        }
    }

    /// <summary>
    /// Chat (0x03 str nick, str texto). Comandos na sala: !bot, !bot off, !bot nível (easy/normal/hard/veryhard/impossible;
    /// também facil/dificil/muitodificil/impossivel). O cliente nunca manda texto que começa com "/".
    /// </summary>
    async Task ChatAsync(string _, string text)
    {
        var cmd = text.Trim().ToLowerInvariant();
        if ((cmd.StartsWith("!trofeu ") || cmd.StartsWith("!trophy ")) && IsGm) { await GmTrophyAsync(text.Trim()[8..]); return; }
        if (IsGm && cmd.StartsWith('!') && ctx.GmExtra?.Invoke(cmd) is { } gmReply) { GmReply(gmReply); return; }
        if (cmd.StartsWith("!bot ") && BotGolfer.ParseLevel(cmd[5..]) is { } level)
        {
            bool add;
            lock (Rooms.Sync)
            {
                if (room == null || room.State != RoomState.Waiting || room.Settings.Practice) return;
                room.BotLevel = level;
                add = room.Bot == null;
                if (room.Bot is { } present)                                    // já está na sala: troca o kit e reenvia a vaga
                {
                    ctx.Players.EquipBot(present.Player, level);
                    InGameOutput.Broadcast(room, RoomPackets.SlotRemove(room, present.Guid));
                    InGameOutput.Broadcast(room, RoomPackets.SlotAdd(room, present));
                }
                InGameOutput.Broadcast(room, new PacketWriter(0x3F).Str($"Bot: {level}"));
            }
            if (add)
            {
                var b = await ctx.Players.CreateBotAsync();
                lock (Rooms.Sync) AddBot(b);
            }
            return;
        }
        if (cmd is "!bot" or "/bot" or "!bot on")
        {
            var bot = await ctx.Players.CreateBotAsync();
            lock (Rooms.Sync) AddBot(bot);
            return;
        }
        if (cmd is "!bot off" or "/bot off" or "!nobot")
        {
            lock (Rooms.Sync) RemoveBot();
            return;
        }
        lock (Rooms.Sync)
        {
            var w = RoomPackets.Chat(Player.Nickname, text, IsGm ? ChatGm : (byte)0);   // bit 0x80 = texto azul de GM
            if (room != null) { InGameOutput.Broadcast(room, w); return; }
            foreach (var s in Rooms.Lobby)
                if (s != this) ((GameHandler)s).Connection.Send(w.Body);
            conn.Send(w);
        }
    }

    void AddBot(Domain.Players.Player bot)
    {
        var r = room;
        if (r?.Settings.Mode == GameMode.GuildMatch) return;                 // bot não tem guilda
        if (r == null || r.State != RoomState.Waiting || r.Bot != null) return;
        if (r.Players.Count >= Math.Min((int)r.Settings.MaxPlayers, 4))
        {
            conn.Send(RoomPackets.Chat("Server", "sala cheia"));
            return;
        }
        var rp = RoomManager.Join(r, new RoomPlayer { Guid = (uint)bot.AccountId, Player = bot, Ready = true, Team = 1 });
        InGameOutput.Broadcast(r, RoomPackets.SlotAdd(r, rp));
        LobbyRoom(3, r);
    }

    void RemoveBot()
    {
        var r = room;
        if (r?.Bot is not { } bot || r.State != RoomState.Waiting) return;
        r.Remove(bot);
        InGameOutput.Broadcast(r, RoomPackets.SlotRemove(r, bot.Guid));
        LobbyRoom(3, r);
    }

    /// <summary>
    /// 0x63 é o primeiro pacote que o cliente manda de dentro da sala: o avatar só é desenhado quando a tela da sala
    /// existe (lobbymain.cpp:3189/7149), então a lista de slots é reenviada uma vez.
    /// </summary>
    /// <summary>Primeiro 0x63 de uma sala normal: reenvia os slots uma vez (chamado sob o lock).</summary>
    void ResendSlotsOnceLocked()
    {
        if (room == null || room.State != RoomState.Waiting || slotsResent) return;
        slotsResent = true;
        conn.Send(RoomPackets.SlotsFull(room));
    }

    /// <summary>Prática: sair pelo botão da prática (JP 0x130) termina a partida como o fim do tempo.</summary>
    public void QuitPractice()
    {
        lock (Rooms.Sync)
            if (room?.Game is TourneyGame t) t.QuitPractice();
    }

    /// <summary>Prática "repetir buraco": novo vento a partir do próximo buraco (JP 0x141).</summary>
    public void NewPracticeWind()
    {
        lock (Rooms.Sync)
            if (room?.Game is TourneyGame t && room.Settings.Practice) t.NewPracticeWind();
    }

    /// <summary>
    /// Grand Prix: entra na sala do GP (Rookie: sempre uma sala nova só dele; os outros: a do mesmo GP que ainda espera e
    /// tem vaga, senão uma nova). Devolve a sala e se foi criada agora; null = cheia/erro.
    /// </summary>
    public (Room Room, bool Created)? EnterGrandPrix(GrandPrixDef d)
    {
        lock (Rooms.Sync)
        {
            if (room != null) LeaveRoom(notifySelf: false);
            Room? r = null;
            if (!d.IsRookie)
                foreach (var x in Rooms.Rooms)
                    if (x.Settings.GrandPrixId == d.Id && x.State == RoomState.Waiting && x.Players.Count < x.Settings.MaxPlayers) { r = x; break; }
            bool created = r == null;
            if (r == null)
            {
                var s = new RoomSettings { Title = d.Name, Mode = GameMode.Tournament, Course = d.Course, Holes = d.Holes,
                    HoleType = (byte)Math.Min((int)d.HoleOrder, 3), GrandPrixId = d.Id, ShotTimeMs = 0, GameTimeMs = 0 };
                r = Rooms.Create(s, Player.AccountId);
                r.GrandPrix = new GrandPrixRun(d) { StartsAt = Domain.Rooms.GrandPrix.StartTime(d, DateTime.Now) };
                Log.Info($"{conn} criou a sala {r.Index} do Grand Prix 0x{d.Id:X} '{d.Name}'");
            }
            EnterRoom(r);
            return (r, created);
        }
    }

    /// <summary>Sai da sala (Grand Prix: volta ao lobby do GP).</summary>
    public void LeaveCurrentRoom()
    {
        lock (Rooms.Sync) LeaveRoom(notifySelf: true);
    }

    /// <summary>Grand Zodiac: pronto para o 1º buraco (o JP manda 0x137).</summary>
    public void ZodiacReady()
    {
        lock (Rooms.Sync)
            if (room?.Game is GrandZodiacGame g && g.Find((uint)Player.AccountId) is { } me) g.Ready(me);
    }

    /// <summary>Grand Zodiac: terminou (0x12C, tempo cheio) ou saiu da prática (0x131, sem completar).</summary>
    public void ZodiacFinish(bool complete)
    {
        lock (Rooms.Sync)
            if (room?.Game is GrandZodiacGame g && g.Find((uint)Player.AccountId) is { } me) g.Finish(me, complete);
    }
}
