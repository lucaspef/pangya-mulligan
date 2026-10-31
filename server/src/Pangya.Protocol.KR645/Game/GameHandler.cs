using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Domain.Auth;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.KR645.Game;

/// <summary>Serviços do game server (um por processo).</summary>
public sealed class GameContext(GameWorld world, SessionService sessions, PlayerService players, IGameData data,
    Core.Config.LotteryConfig? lottery = null, Domain.Servers.IServerRegistry? registry = null, Domain.Guilds.IGuildStore? guilds = null,
    Domain.Mail.IMailStore? mail = null, Domain.Messenger.INoteStore? notes = null, Domain.Messenger.IFriendStore? friends = null)
{
    /// <summary>Bilhetes do mensageiro (null sem banco de bilhetes).</summary>
    public Domain.Messenger.NoteService? Notes { get; } = notes == null ? null : new(notes, friends);
    /// <summary>Correio (null sem banco de correio).</summary>
    public Domain.Mail.MailService? Mail { get; } = mail == null ? null : new(mail, players.Store, data);
    /// <summary>Guildas (null nos testes antigos: sem guilda).</summary>
    public Domain.Guilds.GuildService? Guilds { get; private set; } = guilds == null ? null : new(guilds);
    /// <summary>Comandos de GM de outras camadas ("!zodiac" do JP): devolve a resposta ou null se não é dela.</summary>
    public Func<string, string?>? GmExtra { get; init; }
    /// <summary>Convites de guilda (JP): troca o serviço de guilda por um com os convites.</summary>
    public Domain.Guilds.IGuildInviteStore? GuildInvites { init => Guilds = Guilds == null ? null : new(Guilds.Store, value); }
    readonly System.Collections.Concurrent.ConcurrentDictionary<uint, (GameHandler From, GameHandler To, int Room)> invites = new();
    int nextInvite;

    /// <summary>Convite pendente (0xB2 -> 0x29): id único; os antigos somem ao serem usados ou com o servidor.</summary>
    public uint NewInvite(GameHandler from, GameHandler to, int room)
    {
        uint id = (uint)Interlocked.Increment(ref nextInvite);
        invites[id] = (from, to, room);
        return id;
    }

    public (GameHandler From, GameHandler To, int Room)? TakeInvite(uint id) => invites.TryRemove(id, out var v) ? v : null;

    /// <summary>Ranking do mesmo processo (endereço do 0xA0); null = botão Ranking mostra "manutenção".</summary>
    public Ranking.RankingContext? Ranking { get; set; }

    /// <summary>Auditoria das ações de GM (null nos testes que não usam).</summary>
    public Domain.Admin.AuditLog? Audit { get; set; }

    /// <summary>Mensageiro do mesmo processo (lista de amigos pelo game, status "jogando"); null sem mensageiro.</summary>
    public Messenger.MessengerContext? Messenger { get; set; }

    /// <summary>Aprendizado do bot por nível (o game server liga ao banco; sem banco fica só em memória).</summary>
    public Domain.Rooms.BotKnowledge BotKnowledge { get; set; } = new();

    /// <summary>Pontos dos itens de campo (moedas/caixas) por curso e buraco: seed + aprendidos; sem banco, só em memória.</summary>
    public Domain.Rooms.FieldSpots FieldSpots { get; set; } = new();

    /// <summary>Registro de servidores (lista de servidores no jogo, 0x43); null nos testes que não usam.</summary>
    public Domain.Servers.IServerRegistry? Registry { get; } = registry;
    /// <summary>Tipo dos game servers deste mundo no registro (o 0x43 lista estes): "game" no KR, "game-jp" no JP.</summary>
    public string GameServerType { get; init; } = "game";
    /// <summary>Tipo do mensageiro deste mundo no registro (o game 0x88 lista estes): "messenger" no KR, "messenger-jp" no JP.</summary>
    public string MessengerType { get; init; } = "messenger";
    /// <summary>Troféus especiais (eventos de GM, copas, Grand Prix): perfil e /giveitem com typeid de troféu.</summary>
    public Domain.Players.ISpecialTrophyStore? SpecialTrophies { get; init; }
    /// <summary>Partida do Grand Zodiac (só o JP 983 tem o modo): sala, saída comum dos modos em massa, lock -> a partida.</summary>
    public Func<Domain.Rooms.Room, Domain.Rooms.IMassOutput, object, Domain.Rooms.RoomGame>? ZodiacGame { get; init; }
    /// <summary>Últimos jogadores com quem se jogou (perfil do JP); null = não registra.</summary>
    public Domain.Players.IRecentPlayersStore? RecentPlayers { get; init; }
    /// <summary>Missões do tutorial dão prêmios por carta (o JP 983; o KR 645 não dá).</summary>
    public bool TutorialPrizes { get; init; }
    public GameWorld World { get; } = world;
    /// <summary>Letreiro do mundo (fila de mensagens no topo da tela).</summary>
    public TickerBoard Ticker { get; } = new(world);
    public SessionService Sessions { get; } = sessions;
    public PlayerService Players { get; } = players;
    public IGameData Data { get; } = LinkUcc(data);
    /// <summary>As structs do 0x71 precisam reconhecer as peças Self Design.</summary>
    static IGameData LinkUcc(IGameData d) { PlayerStructs.Data = d; return d; }
    public Domain.Shop.ShopService Shop { get; } = new(players.Store, data);

    public Domain.Shop.CardService Cards { get; } = new(players.Store, data);
    /// <summary>Buffs por tempo e recarga de cometa (a tabela de buffs é a do cliente: JP pelo IFF, KR fixa).</summary>
    public Domain.Players.ItemBuffService Buffs { get; } = new(players.Store, data as Domain.Players.IItemBuffData, data);
    public PlayerActions Actions { get; } = new(players.Store, data);
    Domain.Shop.LotteryService? lotteryService;
    public Domain.Shop.LotteryService Lottery => lotteryService ??= new(Shop, Data, lottery ?? new());
    Domain.Shop.MagicBoxService? magicBoxService;
    public Domain.Shop.MagicBoxService MagicBox => magicBoxService ??= new(Shop, Data);
    Domain.Shop.SpinCubeService? spinCubeService;
    public Domain.Shop.SpinCubeService SpinCube => spinCubeService ??= new(Shop, Data, null, Mail);
}

/// <summary>
/// Game server do cliente 645. Este arquivo: entrada até o lobby (docs/server/protocolo/SPEC-game.md, SPEC-player-shop.md):
/// hello -> 0x02 login -> 0x42 dados do jogador + 0x4B canais + inventário -> 0x04 entrar no canal -> 0x4C.
/// Salas: GameHandler.Room.cs. Partida: GameHandler.Play.cs.
/// </summary>
public sealed partial class GameHandler(Connection conn, GameContext ctx) : IConnectionHandler, IGameSession
{
    // ids C->S
    const ushort CLogin = 0x02, CEnterChannel = 0x04, CEnterChannelAlt = 0x83, CAfterChannel = 0x99, CHeartbeat = 0xF6, CUnknown55 = 0x55,
        CGhost = 0xBA;
    // ids S->C
    const ushort SStatsUpdate = 0x43;
    const ushort SHello = 0x3D, SPlayerInfo = 0x42, SChannels = 0x4B, SEnterChannel = 0x4C, SCharacters = 0x6E, SCaddies = 0x6F,
        STreasureGifts = 0x12C, STreasureGaugeList = 0x129,
        SEquip = 0x70, SItems = 0x71, SGiftBox = 0x78, SCaddieVacation = 0xD2, SCookie = 0x94, SMascots = 0xDF, SItemCounts = 0xA5,
        SCardsClear = 0x12D, SCardPeriodsClear = 0x12E, SCardPeriods = 0x12F, SCards = 0x130;

    Player? player;
    Channel? channel;

    public Player Player => player!;
    public Connection Connection => conn;
    public bool LoggedIn => player != null;
    public GameContext Context => ctx;
    /// <summary>Recebe os eventos de conquista (fim de partida, Papel...): só o cliente que tem conquistas (JP 983).</summary>
    public Domain.Achievements.IAchievementSink? Achievements { get; set; }
    /// <summary>Recebe o fim do Grand Prix (só o JP 983).</summary>
    public Domain.Rooms.IGrandPrixSink? GrandPrix { get; set; }
    /// <summary>O jogador acertou o buraco (nº do buraco): drop do copo do JP. Chamado sob o lock da sala.</summary>
    public Action<byte>? Holed { get; set; }
    /// <summary>Fim de partida para a maestria do taco (só o JP tem a oficina).</summary>
    public Func<Domain.Achievements.AchievementCounters.GameEnd, Task>? ClubMastery { get; set; }
    /// <summary>Quantidade nova da bola no cliente (JP: 0x216); null = KR 0xA5.</summary>
    public Action<ItemChange>? BallSpent { get; set; }

    public ValueTask OnConnectedAsync()
    {
        conn.ParseKey = Random.Shared.Next(16);
        // o 645 também descomprime tudo que recebe (WReceivedPacket::IsValid -> CCompressBuffer): sem isso um inventário
        // grande (~800 KB no login) estoura o buffer de recepção de 0x3FFFC (WClientSocket::TryRead) e o cliente cai
        conn.CompressPackets = ctx.World.Config.CompressPackets;
        conn.SendRaw(new PacketWriter(SHello).U8(0).U8(0).U8((byte)conn.ParseKey));
        return ValueTask.CompletedTask;
    }

    public ValueTask OnDisconnectedAsync()
    {
        if (player == null) return ValueTask.CompletedTask;
        lock (ctx.World.Rooms.Sync)
        {
            ctx.World.Rooms.Lobby.Remove(this);
            LeaveRoom(notifySelf: false, deliberate: false);
            LobbyUser(LobbyUserRemove);                                         // sai da lista dos outros
        }
        ctx.World.Leave(this);
        FinishGameEnd();                                                  // grava a recompensa de quem fechou o jogo cedo
        GameWorld.LeaveChannel(channel);
        channel = null;
        return ValueTask.CompletedTask;
    }

    public void Kick(string reason) => conn.Close(reason);

    public System.Net.IPAddress RemoteAddress => conn.Remote.Address;

    public PlayerWhere Where
    {
        get
        {
            var ch = channel;
            var r = room;
            return new PlayerWhere(ch?.Id ?? -1, ch?.Name ?? "", r?.Index ?? -1, r == null ? -1 : (int)r.Settings.Mode);
        }
    }

    public async ValueTask OnPacketAsync(PacketReader p)
    {
        if (player == null && p.Id != CLogin)
        {
            Log.Debug($"{conn} pacote 0x{p.Id:X4} antes do login: ignorado");
            return;
        }
        if (await HandleRoomAsync(p) || HandleIntrusion(p) || HandlePlay(p) || await HandleShopAsync(p) || await HandleMyRoomAsync(p) || await HandleLotteryAsync(p) || HandleGm(p) || await HandleSocialAsync(p) || await HandleTradeAsync(p) || await HandleBoxesAsync(p) || await HandleGuildAsync(p) || await HandleMailAsync(p) || await HandleMessengerAsync(p) || await HandleUccAsync(p)) return;
        switch (p.Id)
        {
            case CLogin: await LoginAsync(p); break;
            case CEnterChannel or CEnterChannelAlt: EnterChannel(p.U8()); break;
            case CAfterChannel or CHeartbeat or CUnknown55: break;
            case CGhost:                                                     // modo Ghost: código morto no 645 (SPEC-ghost.md)
                Log.Info($"{conn} pacote de Ghost 0xBA sub 0x{(p.Remaining > 0 ? p.U8() : 0):X2} ignorado (exe modificado?)");
                p.Skip(p.Remaining);
                break;
            default: Log.Info($"{conn} pacote não tratado 0x{p.Id:X4} ({p.Remaining} bytes)"); break;
        }
    }

    async Task LoginAsync(PacketReader p)
    {
        if (player != null) throw new PacketException("login repetido");
        var id = p.Str(22);
        var uid = p.U32();
        p.U32();                                        // MemberNo
        p.U16();
        var key = p.Str(64);
        var version = p.Str(16);
        if (ctx.World.IsFull) { conn.Close("servidor cheio"); return; }
        var accountId = await ctx.Sessions.ValidateGameLoginAsync(key);
        var loaded = accountId == uid ? await ctx.Players.LoadAsync(uid) : null;
        if (loaded == null || !string.Equals(loaded.Login, id, StringComparison.OrdinalIgnoreCase))
        {
            conn.Close($"login inválido no game server (id={id} uid={uid})");
            return;
        }
        player = loaded;
        conn.IdleTimeoutSeconds = conn.Limits.SessionIdleTimeoutSeconds;
        ctx.World.Enter(this);
        Log.Info($"{conn} entrou: {player.Login} ({player.Nickname}) uid={uid} versão={version}");
        await LoadGuildAsync();
        await PurgeExpiredReportsAsync();
        SendPlayerInfo();
        SendChannels();
        SendInventory();
        await SendUnreadMailAsync();                                        // cartas não lidas (botão de presente pisca)
        conn.Send(new PacketWriter(SCookie).U64((ulong)player.Cookie));     // último do login
    }

    void SendPlayerInfo()
    {
        var w = new PacketWriter(SPlayerInfo, 0xD00).U8(0).Str(Kr645.ClientVersion).Str("")
            .Struct(PlayerStructs.UserInfo(player!))
            .Struct(PlayerStructs.SystemTime(DateTime.Now))       // hora do servidor (loja, validade de itens)
            .U8(0).U8(0).U16(0xFFFF).U16(0xFFFF).U16(0)           // flag, ?, papel: jogadas (-1 = sem limite), bônus (-1), faltam
            .U32(0).U32(ctx.Ranking == null ? RankingOff : 0u).U32(0).U32(0)   // flagBlock, controlServerService, ?, serverProperty
            .Struct(myGuildInfo);                                 // GUILD_USER_INFO (o RSS do guildId != 0 é inofensivo)
        conn.Send(w);
    }

    void SendChannels()
    {
        var w = new PacketWriter(SChannels).U8((byte)ctx.World.Channels.Count);
        foreach (var c in ctx.World.Channels)
        {
            var s = new sChannelInfo { Max_Num = (ushort)c.MaxUsers, Current_Num = (ushort)c.Count, Uid = (byte)c.Id };
            Cp949.Write(s.Name, c.Name);
            w.Struct(s);
        }
        conn.Send(w);
    }

    /// <summary>Itens por pacote 0x71 no login: o cliente vai somando os pacotes (o GB manda no máximo 50).</summary>
    const int ItemsPerPacket = 50;

    static bool InItemList(ItemGroup g) => g is ItemGroup.Part or ItemGroup.Club or ItemGroup.ClubSet or ItemGroup.Ball
        or ItemGroup.Usable or ItemGroup.Skin or ItemGroup.SetItem or ItemGroup.AuxPart;   // anéis vão na lista de itens

    /// <summary>
    /// Listas que o cliente guarda (o 0x42 limpa o inventário, então vêm depois dele). Em cada pacote total == n
    /// (senão o cliente espera mais); a lista de itens vai em vários pacotes, que o cliente acumula.
    /// </summary>
    /// <summary>Itens que apareceram fora da loja (premium do JP): na lista de itens (0x71 de 1) e a lista de mascotes de novo.</summary>
    public void SendNewItems(IReadOnlyList<Item> items)
    {
        bool mascot = false;
        foreach (var it in items)
            if (it.Group == ItemGroup.Mascot) mascot = true;
            else if (InItemList(it.Group)) conn.Send(new PacketWriter(SItems).U16(1).U16(1).Struct(PlayerStructs.ItemInfo(it)));
        if (!mascot) return;
        var mascots = Player.OfGroup(ItemGroup.Mascot);
        int nm = Math.Min(mascots.Count, 255);
        var w = new PacketWriter(SMascots).U8((byte)nm);
        for (int i = 0; i < nm; i++) w.Struct(PlayerStructs.Mascot(mascots[i]));
        conn.Send(w);
    }

    /// <summary>Avisos de caddie de férias que o lobby guarda (m_caddieWarning[12]).</summary>
    const int CaddieWarningMax = 12;

    void SendInventory()
    {
        var p = player!;
        var chars = p.OfGroup(ItemGroup.Character);
        var w = new PacketWriter(SCharacters, 8 + chars.Count * 0x1BC).U16((ushort)chars.Count).U16((ushort)chars.Count);
        foreach (var c in chars) w.Struct(PlayerStructs.Character(c));
        conn.Send(w);

        var caddies = p.OfGroup(ItemGroup.Caddie);
        w = new PacketWriter(SCaddies).U16((ushort)caddies.Count).U16((ushort)caddies.Count);
        foreach (var c in caddies) w.Struct(PlayerStructs.Caddie(c));
        conn.Send(w);
        // caddies de férias (contrato vencido, CaddieRent) com o aviso ligado: 0xD2 u32 n, n × sCaddieInfo; o lobby abre o
        // aviso de cada um (lobbytask.cpp case 0xD2 -> CLobbyMain msg 0x61, até 12). O cliente não filtra: a caixa
        // "aviso da mensalidade" só volta ao servidor pelo 0x6B (1 = avisar), e é aqui que ela vale.
        var vacation = new List<Item>();
        var now = DateTime.UtcNow;
        foreach (var c in caddies)
            if (CaddieRent.IsExpired(c, now) && c.Int("warning") == 1 && vacation.Count < CaddieWarningMax) vacation.Add(c);
        if (vacation.Count > 0)
        {
            w = new PacketWriter(SCaddieVacation).U32((uint)vacation.Count);
            foreach (var c in vacation) w.Struct(PlayerStructs.Caddie(c));
            conn.Send(w);
        }

        var items = new List<Item>();
        foreach (var it in p.Items.Values)
            if (InItemList(it.Group) && it.Location == ItemLocation.Inventory) items.Add(it);
        // o cliente só lista a bola que veio no 0x71 (myroomwidget.cpp:1209): quem não tem a linha recebe uma virtual
        if (p.FindType(Item.BasicBall) == null) items.Add(new Item { Id = Item.VirtualBallId, TypeId = Item.BasicBall });
        for (int start = 0; start == 0 || start < items.Count; start += ItemsPerPacket)   // páginas com total = n, como o GB
        {
            int n = Math.Min(ItemsPerPacket, items.Count - start);
            w = new PacketWriter(SItems, 8 + n * 0xA8).U16((ushort)n).U16((ushort)n);
            for (int i = start; i < start + n; i++) w.Struct(PlayerStructs.ItemInfo(items[i]));
            conn.Send(w);
        }

        // mascotes antes do 0x70: o mascote equipado (guidMascot) só vale se já estiver na lista do cliente
        var mascots = p.OfGroup(ItemGroup.Mascot);
        int nm = Math.Min(mascots.Count, 255);
        w = new PacketWriter(SMascots).U8((byte)nm);
        for (int i = 0; i < nm; i++) w.Struct(PlayerStructs.Mascot(mascots[i]));
        conn.Send(w);

        conn.Send(new PacketWriter(SEquip).Struct(PlayerStructs.Equip(p)));

        conn.Send(new PacketWriter(SGiftBox).U8(1).U16(1).U16(0).U16(0));   // caixa de presentes vazia (modo 1)
        SendCards();
        SendBuffs();
        conn.Send(TreasureGauges());                                       // barras do Treasure Hunter na escolha de mapa
        conn.Send(TutorialPacket(p.Tutorial));                             // missões do tutorial feitas
    }

    /// <summary>0x12D limpa + 0x130 pilhas de cards (sCards) + 0x12E limpa + 0x12F cards ativos/encaixados.</summary>
    void SendCards()
    {
        var p = player!;
        var stacks = p.OfGroup(ItemGroup.Card);
        conn.Send(new PacketWriter(SCardsClear));
        var w = new PacketWriter(SCards, 8 + stacks.Count * 0x3A).U32(0).U16((ushort)stacks.Count);
        foreach (var c in stacks) w.Struct(new sCards { uid = (uint)c.Id, typeId = (uint)c.TypeId, count = c.Quantity, type = 1 });
        conn.Send(w);
        var active = PlayerStructs.ActiveCards(p, ctx.Data.Cards);
        conn.Send(new PacketWriter(SCardPeriodsClear));
        w = new PacketWriter(SCardPeriods, 8 + active.Count * 0x41).U16((ushort)active.Count);
        foreach (var a in active) w.Struct(a);
        conn.Send(w);
    }

    /// <summary>Fim de partida: credita pang (limitado) e EXP e atualiza o pang mostrado. Chamado sob o lock da sala.</summary>
    void EnterChannel(byte id)
    {
        var r = ctx.World.JoinChannel(id, channel, out var joined);
        if (r == ChannelJoinResult.Ok) channel = joined;
        Log.Info($"{conn} canal {id}: {r}");
        conn.Send(new PacketWriter(SEnterChannel).U8((byte)r));
        if (r == ChannelJoinResult.Ok) _ = DeliverNotesAsync(Player.AccountId, ctx);   // bilhetes que chegaram fora do lobby
    }
}
