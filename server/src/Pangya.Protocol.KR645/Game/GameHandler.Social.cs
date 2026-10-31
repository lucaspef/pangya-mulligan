using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.KR645.Game;

/// <summary>
/// Pacotes pequenos de sala/lobby e de itens (SPEC-coverage.md do emulador): ícone sobre a cabeça, expulsar, detalhe da
/// sala, ausente, desistir no solo, chat de equipe, apagar item, trocar nick e recontratar caddie.
/// </summary>
public sealed partial class GameHandler
{
    const ushort CHeadIcon = 0x18, CBanish = 0x26, CBanishVote = 0x27, CRoomDetail = 0x2D, CIdle = 0x32, CGiveUpSolo = 0x37,
        CChangeNick = 0x38, CCaddieRehire = 0x39, CTeamChat = 0x54, CDeleteItem = 0x64;
    const ushort CWhisper = 0x2A, CWhisperRejected = 0xE0, CInvite = 0xB2, CInviteAck = 0x29, CDirectMove = 0xAC,
        CQuickInterests = 0xB6, CQuickMatch = 0xB7, CQuickAnswer = 0xB8, CPrivateTrade = 0xDB;
    const ushort SWhisper = 0x82, SInviteResult = 0x127, SInvited = 0x81, SQuickMatch = 0x133, SQuickAsk = 0x132, SQuickGo = 0x135;
    /// <summary>0x133: 0 procurando, 1 ninguém, 3 todos recusaram (lobbytask.cpp case 0x133).</summary>
    const byte QuickSearching = 0, QuickNobody = 1, QuickAllRefused = 3, QuickFailed = 5;
    /// <summary>Tempo do dono da sala para responder (a janela dele fecha sozinha em 5 s).</summary>
    static readonly TimeSpan QuickAnswerWait = TimeSpan.FromSeconds(8);
    /// <summary>0x3E tipos de aviso de sussurro: 4 não recebe, 6 não está conectado.</summary>
    const byte ChatNoWhisper = 4, ChatNotConnected = 6;
    /// <summary>0x127 códigos: 2 sala cheia, 6 já está numa sala, 0x17 não está num lugar onde dá para convidar.</summary>
    const ushort InviteRoomFull = 2, InviteInRoom = 6, InviteNotHere = 0x17;
    const ushort SHeadIcon = 0x5B, SRoomDetail = 0x84, STeamChat = 0xAE, SChangeNick = 0x4E, SCaddieRehire = 0x91, SDeleteResult = 0xC3,
        SIdle = 0x8C, SChatBlock = 0xAA;

    async ValueTask<bool> HandleSocialAsync(PacketReader p)
    {
        switch (p.Id)
        {
            case CHeadIcon: HeadIcon(p.U16()); return true;
            case CBanish: Banish(p.U32()); return true;
            case CBanishVote: p.Skip(p.Remaining); return true;                     // sem votação
            case CIdle: RoomFlag(SIdle, p.Remaining > 0 && p.U8() != 0); p.Skip(p.Remaining); return true;
            case CRoomDetail: RoomDetail(p.U16()); return true;
            case CGiveUpSolo: GiveUpSolo(); return true;
            case CTeamChat: TeamChat(p.Str(256)); return true;
            case CDeleteItem: await DeleteItemAsync((int)p.U32(), (int)p.U32()); return true;
            case CChangeNick:                                                        // o nick é a chave da conta: ainda não
                p.Skip(p.Remaining);
                conn.Send(new PacketWriter(SChangeNick).U32(7));                      // 7 = "troca de nick suspensa"
                return true;
            case CCaddieRehire: await CaddieRehireAsync(p.U32()); return true;
            case CWhisper: Whisper(p.Str(32), p.Str(256)); return true;
            case CWhisperRejected:
                if (FindOnline(p.Str(32)) is { } from && from != this)
                    from.Connection.Send(RoomPackets.Chat(Player.Nickname, "", ChatNoWhisper));
                return true;
            case CInvite: Invite(p.Str(32), p.U32()); return true;
            case CInviteAck: InviteAck(p.U32()); return true;
            case CDirectMove: p.U8(); JoinRoom(p.U16(), ""); return true;
            case CQuickInterests: p.Skip(p.Remaining); return true;
            case CQuickAnswer: QuickAnswer(p.U32(), p.U8() != 0); return true;
            case CPrivateTrade:                     // troca direta: desligada no cliente 645 (conteúdo 0x77; SPEC-troca-direta.md)
                Log.Info($"{conn} troca direta (0xDB) ignorada: precisa do cliente com o conteúdo 0x77 ligado");
                p.Skip(p.Remaining);
                return true;
            case CQuickMatch: QuickMatch(p.U8()); return true;
            default: return false;
        }
    }

    /// <summary>
    /// Ausente (0x32 u8) e bloqueio de chat na partida (0x4F u8): a sala inteira recebe u32 guid, u8 estado (0x8C ícone de
    /// ausente na vaga, lobbytask.cpp; 0xAA chat bloqueado, referência requestChangeStateChatBlock). Fora de sala: nada.
    /// </summary>
    void RoomFlag(ushort id, bool on)
    {
        lock (Rooms.Sync)
            if (room != null) InGameOutput.Broadcast(room, new PacketWriter(id).U32((uint)Player.AccountId).U8(on ? (byte)1 : (byte)0));
    }

    // ---- Quick Start (S4 matching; lobbytask.cpp 0x132..0x136, dlgquickstartsetting.cpp)
    /// <summary>No dono da sala: quem pediu, o modo e a sala.</summary>
    (GameHandler From, byte Type, int Room)? quickAsked;
    /// <summary>Em quem procura: salas já tentadas nesta busca.</summary>
    readonly HashSet<int> quickTried = [];
    byte quickType;

    /// <summary>
    /// 0xB7 u8 modo (0 VS, 4 torneio, 10 Approach, 2 lounge): procura uma sala esperando, desse modo, sem senha e com vaga
    /// no canal e pergunta ao dono (0x132 u32 uid, str nick, u8 modo); a quem procura, 0x133 0. Sem sala: 0x133 1 (ou 3
    /// se todas recusaram).
    /// </summary>
    void QuickMatch(byte type)
    {
        lock (Rooms.Sync) { quickTried.Clear(); quickType = type; }
        QuickNext();
    }

    void QuickNext()
    {
        GameHandler? master = null;
        Room? target = null;
        lock (Rooms.Sync)
        {
            if (room != null) { conn.Send(new PacketWriter(SQuickMatch).U8(QuickFailed)); return; }   // já está numa sala
            foreach (var r in Rooms.Rooms)
            {
                if (r.State != RoomState.Waiting || r.Settings.Hidden || r.Settings.Password.Length > 0 || (byte)r.Settings.Mode != quickType
                    || r.Players.Count >= r.Settings.MaxPlayers || quickTried.Contains(r.Index)) continue;
                foreach (var rp in r.Players)
                    if (rp.Master && rp.Session is GameHandler h && h != this && h.quickAsked == null) { master = h; break; }
                if (master != null) { target = r; break; }
            }
            if (target == null)
            {
                conn.Send(new PacketWriter(SQuickMatch).U8(quickTried.Count > 0 ? QuickAllRefused : QuickNobody));
                quickTried.Clear();
                return;
            }
            quickTried.Add(target.Index);
            master!.quickAsked = (this, quickType, target.Index);
        }
        conn.Send(new PacketWriter(SQuickMatch).U8(QuickSearching));
        master.Connection.Send(new PacketWriter(SQuickAsk).U32((uint)Player.AccountId).Str(Player.Nickname ?? "").U8(quickType));
        Log.Info($"{conn} quick start: modo {quickType}, pergunta ao dono da sala {target.Index}");
        var asked = master;
        _ = Task.Run(async () =>                                              // sem resposta: próxima sala
        {
            await Task.Delay(QuickAnswerWait);
            bool expired;
            lock (Rooms.Sync)
            {
                expired = asked.quickAsked is { } q && q.From == this;
                if (expired) asked.quickAsked = null;
            }
            if (expired) QuickNext();
        });
    }

    /// <summary>0xB8 u32 uid de quem pediu, u8 aceita: sim -> 0x135 (u32 servidor, u8 modo, u16 sala) para ele entrar (0xAC).</summary>
    void QuickAnswer(uint uid, bool accept)
    {
        GameHandler from;
        int index;
        byte type;
        lock (Rooms.Sync)
        {
            if (quickAsked is not { } q || (uint)q.From.Player.AccountId != uid) return;
            quickAsked = null;
            (from, type, index) = q;
            if (accept && (room?.Index != index || room.Players.Count >= room.Settings.MaxPlayers)) accept = false;
            if (accept) from.quickTried.Clear();
        }
        Log.Info($"{conn} quick start: {(accept ? "aceitou" : "recusou")} {uid}");
        if (accept) from.Connection.Send(new PacketWriter(SQuickGo).U32((uint)ctx.World.Config.Id).U8(type).U16((ushort)index));
        else from.QuickNext();
    }

    GameHandler? FindOnline(string nick)
    {
        foreach (var s in ctx.World.Online)
            if (s is GameHandler h && h.player != null && string.Equals(h.player.Nickname, nick, StringComparison.OrdinalIgnoreCase)) return h;
        return null;
    }

    /// <summary>
    /// 0x2A str nick, str texto (sussurro) -> 0x82 u8 0 (eco), nick do alvo, texto para mim e 0x82 u8 1, meu nick, texto
    /// para o alvo (bit 0x80 = cor de GM); alvo fora: 0x3E tipo 6 "não está conectado".
    /// </summary>
    void Whisper(string nick, string text)
    {
        if (text.Length == 0) return;
        var t = FindOnline(nick);
        if (t == null) { conn.Send(RoomPackets.Chat(nick, "", ChatNotConnected)); return; }
        byte gm = IsGm ? ChatGm : (byte)0;
        conn.Send(new PacketWriter(SWhisper).U8(gm).Str(t.Player.Nickname).Str(text));
        if (t != this) t.Connection.Send(new PacketWriter(SWhisper).U8((byte)(1 | gm)).Str(Player.Nickname).Str(text));
    }

    /// <summary>
    /// 0xB2 str nick, u32 uid: convidar para a minha sala. 0x127 u16 0, u32 servidor, u8 canal, u16 sala, u32 uid, str nick,
    /// u32 convite (o cliente confirma com 0x29) ou só o código (2 cheia, 6 já em sala, 0x17 não dá para convidar).
    /// </summary>
    void Invite(string nick, uint uid)
    {
        lock (Rooms.Sync)
        {
            var r = room;
            var t = (ctx.World.Find(uid) as GameHandler) ?? FindOnline(nick);
            ushort code = r == null || t == null || t == this ? InviteNotHere
                : t.room != null ? InviteInRoom
                : r.Players.Count >= r.Settings.MaxPlayers ? InviteRoomFull : (ushort)0;
            if (code != 0) { conn.Send(new PacketWriter(SInviteResult).U16(code)); return; }
            uint id = ctx.NewInvite(this, t!, r!.Index);
            conn.Send(new PacketWriter(SInviteResult).U16(0).U32((uint)ctx.World.Config.Id).U8((byte)(channel?.Id ?? 0))
                .U16((ushort)r.Index).U32((uint)t!.Player.AccountId).Str(t.Player.Nickname).U32(id));
        }
    }

    /// <summary>0x29 u32 convite -> 0x81 para o convidado: u16 0, u32 servidor, u8 canal, u16 sala, u32 uid, str nick, u32 id.</summary>
    void InviteAck(uint id)
    {
        if (ctx.TakeInvite(id) is not { } inv || inv.From != this) return;
        inv.To.Connection.Send(new PacketWriter(SInvited).U16(0).U32((uint)ctx.World.Config.Id).U8((byte)(channel?.Id ?? 0))
            .U16((ushort)inv.Room).U32((uint)Player.AccountId).Str(Player.Nickname).U32(id));
    }

    /// <summary>0x18 u16 ícone (0xFFFF limpa) -> 0x5B u32 guid, u16 para os outros da sala.</summary>
    void HeadIcon(ushort icon)
    {
        lock (Rooms.Sync)
            if (room != null) InGameOutput.Broadcast(room, new PacketWriter(SHeadIcon).U32((uint)Player.AccountId).U16(icon), except: this);
    }

    /// <summary>0x26 u32 uid: o dono tira alguém da sala (o bot também). O expulso volta para a lista (0x4A).</summary>
    void Banish(uint uid)
    {
        GameHandler? target = null;
        lock (Rooms.Sync)
        {
            var r = room;
            if (r == null || r.State != RoomState.Waiting || r.Find(this) is not { Master: true } || uid == Player.AccountId) return;
            if (r.Settings.Mode == GameMode.GuildMatch) return;                // o cliente também não deixa expulsar
            var rp = r.Find(uid);
            if (rp == null) return;
            if (rp.IsBot) { RemoveBot(); return; }
            target = rp.Session as GameHandler;
            target?.LeaveRoom(notifySelf: true);
        }
        if (target != null) Log.Info($"{conn} expulsou {target.Player.Login} da sala");
    }

    /// <summary>
    /// 0x2D u16 sala -> 0x84 u8 n (0 = "a sala não existe"), sRoomDetail 12 B {u8 buracos, u32 tempo de jogo, u8 mapa,
    /// u8 modo, u8 ordem dos buracos, u32 troféu}, n × {u32 uid, u8 nível, u8 buraco, u32 capacidade, u32 0, u32 ladder}.
    /// </summary>
    void RoomDetail(ushort index)
    {
        lock (Rooms.Sync)
        {
            var r = Rooms.Get(index);
            if (r == null) { conn.Send(new PacketWriter(SRoomDetail).U8(0)); return; }
            var s = r.Settings;
            var w = new PacketWriter(SRoomDetail, 16 + r.Players.Count * 18).U8((byte)r.Players.Count)
                .U8(s.Holes).U32(MassGame.IsMass(s.Mode) && s.Mode != GameMode.NewApproach ? s.GameTimeMs : s.ShotTimeMs).U8(s.Course).U8((byte)s.Mode).U8(s.HoleType).U32(0);
            foreach (var p in r.Players)
                w.U32((uint)p.Player.AccountId).U8((byte)p.Player.Level).U8(0).U32(0).U32(0).U32(0);
            conn.Send(w);
        }
    }

    /// <summary>0x37: desistir da partida stroke sozinho ("desistir e voltar para a sala"): placar e fim, sem recompensa, conta abandono.</summary>
    void GiveUpSolo()
    {
        lock (Rooms.Sync)
        {
            if (room?.Game is not StrokeGame g || room.Settings.Mode != GameMode.Stroke) return;
            int quitHoles = g.QuitHoles((uint)Player.AccountId);
            if (!g.GiveUp((uint)Player.AccountId)) return;
            if (quitHoles > 0 && !g.Aborted) CountQuit(GameMode.Stroke, quitHoles, alone: true);
            var ball = TakeBall(g, (uint)Player.AccountId);
            if (ball.Holes > 0) _ = Task.Run(() => SpendBallAsync(ball));
            Log.Info($"{conn} desistiu da partida solo");
        }
    }

    /// <summary>0x54 str -> 0xAE str nick, str texto para os da mesma equipe ("nick>팀에게 : texto").</summary>
    void TeamChat(string text)
    {
        lock (Rooms.Sync)
        {
            var r = room;
            if (r?.Find(this) is not { } me || text.Length == 0) return;
            var w = new PacketWriter(STeamChat).Str(Player.Nickname).Str(text);
            foreach (var p in r.Players)
                if (p.Team == me.Team && p.Session is GameHandler h) h.Connection.Send(w.Body);
            w.Dispose();
        }
    }

    /// <summary>
    /// 0x64 u32 tid, u32 quantidade (apagar item, My Room) -> 0xC3 u8 1, u32 tid, i32 apagados, u32 id (o cliente desconta e
    /// mostra "N itens apagados"; recusa: u8 0, u32 tid, i32 0 = "não foi possível apagar"), depois 0xA5 com a contagem
    /// nova (0 apaga; vem depois para não descontar duas vezes) e 0xA8 vazio para fechar a espera.
    /// </summary>
    async Task DeleteItemAsync(int tid, int count)
    {
        int before = Player.FindType(tid)?.Quantity ?? 0;
        var it = await ctx.Actions.DeleteItemAsync(Player, tid, count);
        conn.Send(it == null ? new PacketWriter(SDeleteResult).U8(0).U32((uint)tid).I32(0)
            : new PacketWriter(SDeleteResult).U8(1).U32((uint)tid).I32(Math.Max(before - Math.Max(it.Quantity, 0), 1)).U32((uint)it.Id));
        if (it != null) conn.Send(new PacketWriter(SItemCounts).U8(1).U32((uint)it.TypeId).U32((uint)it.Id).U16((ushort)Math.Max(it.Quantity, 0)));
        conn.Send(new PacketWriter(SBought).U16(0));                         // 0xA8 vazio: fecha a espera
        Log.Info($"{conn} apagar item {tid:X8} x{count}: {(it == null ? "recusado" : $"sobrou {it.Quantity}")}");
    }

    /// <summary>
    /// 0x39 u32 caddie: renova o contrato vencido pagando a mensalidade em pang (CaddieRent). 0x91 u8 resultado (2 ok,
    /// 1 ainda contratado, 4 sem pang, 0 erro); no 2, u32 caddie e u64 pang depois.
    /// </summary>
    async Task CaddieRehireAsync(uint id)
    {
        var (result, pang) = await ctx.Actions.RenewCaddieAsync(Player, (int)id);
        var w = new PacketWriter(SCaddieRehire).U8((byte)result);
        if (result == Domain.Players.CaddieRenewal.Ok) w.U32(id).U64((ulong)pang);
        conn.Send(w);
        Log.Info($"{conn} renovar caddie {id}: {result}, pang {pang}");
    }
}
