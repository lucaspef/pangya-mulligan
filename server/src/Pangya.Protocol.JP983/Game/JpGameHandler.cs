using Pangya.Core.Net;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Protocol.JP983.Game;

/// <summary>Serviços do game server JP: os do game server KR (outro mundo) + o que o cliente JP conhece.</summary>
public sealed class JpGameContext(GameContext kr, Jp983GameData? jpData = null, uint property = 0, Core.Config.MemorialConfig? memorial = null,
    Domain.Achievements.IAchievementStore? achievementStore = null, Domain.Rooms.IGrandPrixStore? grandPrixStore = null,
    Domain.Players.IAttendanceStore? attendanceStore = null, Core.Config.AttendanceConfig? attendance = null)
{
    /// <summary>Presença diária (null = sem banco).</summary>
    public Domain.Players.AttendanceService? Attendance { get; } =
        attendanceStore == null ? null : new(attendanceStore, kr.Shop, attendance ?? new(), mail: kr.Mail);
    /// <summary>GPs concluídos por jogador (null = sem banco).</summary>
    public Domain.Rooms.IGrandPrixStore? GrandPrixStore { get; } = grandPrixStore;
    /// <summary>Conquistas (só com os dados do JP e o banco).</summary>
    public Domain.Achievements.AchievementService? Achievements { get; } =
        jpData == null || achievementStore == null ? null : new(jpData, achievementStore, kr.Shop);
    /// <summary>Memorial Shop (só com os dados do JP, que trazem as moedas e os raros).</summary>
    public Domain.Shop.MemorialService? Memorial { get; } = jpData == null ? null : new(kr.Shop, jpData, memorial ?? new());
    /// <summary>Caixas do My Room (conteúdo da config; JpGameServer liga).</summary>
    public Domain.Shop.BoxService? Boxes { get; init; }
    /// <summary>Propriedade do servidor no 0x44 (0x800 = Grand Prix).</summary>
    public uint Property { get; } = property;
    public GameContext Kr { get; } = kr;
    /// <summary>Missão diária (só com os dados do JP e o banco).</summary>
    public Domain.Achievements.DailyQuestService? DailyQuests { get; init; }
    /// <summary>Gacha (o site sorteia no jogador registrado aqui).</summary>
    public Domain.Shop.GachaDesk? Gacha { get; init; }
    /// <summary>Prêmio de login por evento.</summary>
    public Domain.Players.LoginRewardService? LoginRewards { get; init; }
    /// <summary>World Tour (16 campos).</summary>
    public Domain.Players.WorldTourService? WorldTour { get; init; }
    /// <summary>Pontos do Grand Zodiac por conta (tamanho do copo).</summary>
    public Domain.Players.IZodiacPointsStore? ZodiacPoints { get; init; }
    /// <summary>Point Shop e Tiki Shop (só com os dados do JP e o banco).</summary>
    public Domain.Shop.TikiService? Tiki { get; init; }
    /// <summary>Typeids do pangya_jp.iff (null = sem filtro).</summary>
    public Jp983GameData? JpData { get; } = jpData;
}

/// <summary>
/// Game server do cliente JP R7.983 (docs/server/protocolo/SPEC-jp983-game.md): a lógica é a do game server do KR 645
/// (<see cref="GameHandler"/>: lobby, salas, partida, loja, My Room), com ids e structs traduzidos por
/// <see cref="JpTranslator"/>. Daqui: hello do JP (0x3F) e pacotes comprimidos (LZO), como o servidor oficial.
/// </summary>
public sealed class JpGameHandler : IConnectionHandler
{
    const ushort SHello = 0x3F, SLogin = 0x44, CLogin = 0x02;
    const byte LoginFail = 1;

    readonly Connection conn;
    readonly JpGameContext ctx;
    public GameHandler Kr { get; }
    readonly JpSystems systems;

    public JpGameHandler(Connection conn, JpGameContext ctx)
    {
        this.conn = conn;
        this.ctx = ctx;
        Kr = new GameHandler(conn, ctx.Kr);
        var translator = new JpTranslator(conn, Kr, ctx.JpData, ctx.Property);
        conn.Translator = translator;
        conn.TextEncoding = Core.Text.Cp949.Japanese;                   // strings em Shift-JIS
        systems = new JpSystems(conn, Kr, ctx.Kr, ctx.Memorial, ctx.Achievements, ctx.GrandPrixStore, ctx.Attendance, ctx.DailyQuests, ctx.Tiki, ctx.Gacha);
        translator.GrandPrixLobby = systems.GrandPrixLobby;
        translator.GrandPrixStarted = systems.ConsumeTicketAsync;
        translator.ItemSpent = systems.SpendAsync;
        Kr.TrophyGiven = systems.TrophyWon;
        systems.WorldTour = ctx.WorldTour;
        systems.Boxes = ctx.Boxes;
        Kr.ClubMastery = async g =>                                                 // fim da partida
        {
            await systems.ClubMasteryAsync(g);
            await systems.CreditDropsAsync(g);
            await systems.WorldTourGameEndAsync(g);
        };
        Kr.Holed = systems.OnHoled;
        Kr.BallSpent = c => conn.SendTranslated(JpSystems.ItemsChanged([c]));
    }

    /// <summary>Hello cru: u8 1, u8 1, u8 chave, str IP do cliente.</summary>
    public ValueTask OnConnectedAsync()
    {
        conn.ParseKey = Random.Shared.Next(16);
        conn.CompressPackets = true;                                    // o cliente cai se o buffer de recepção enche (Connection)
        conn.SendRaw(new PacketWriter(SHello).U8(1).U8(1).U8((byte)conn.ParseKey).Str(conn.Remote.Address.ToString()));
        return ValueTask.CompletedTask;
    }

    public async ValueTask OnPacketAsync(PacketReader packet)
    {
        if (packet.Id == CLogin && !await LoginKeyIsValidAsync(packet))
        {
            conn.SendTranslated(new PacketWriter(SLogin).U8(LoginFail).U8(0));        // o KR só fecha; o JP mostra a falha
            conn.Close("login inválido no game server JP", afterSend: true);
            return;
        }
        if (JpSystems.Ids.Contains(packet.Id) && await systems.HandleAsync(packet)) return;   // só do JP, com o Domain
        bool login = packet.Id == CLogin;
        await Kr.OnPacketAsync(packet);
        if (login && Kr.LoggedIn)
        {
            await systems.AfterLoginAsync();                              // conquistas depois do login
            if (ctx.ZodiacPoints is { } zp && conn.Translator is JpTranslator t) t.ZodiacPoints = await zp.GetAsync(Kr.Player.AccountId);
            if (ctx.Gacha is { } gacha)
            {
                gacha.Register(Kr.Player, ctx.Kr.Shop);
                systems.SendGachaCounts();
            }
            await systems.SendWorldTourAsync();
            if (ctx.LoginRewards is { } lr)
                foreach (var name in await lr.OnLoginAsync(Kr.Player, DateOnly.FromDateTime(DateTime.Now)))
                    Core.Logging.Log.Info($"{conn} prêmio de login: {name}");
        }
    }

    /// <summary>Olha o 0x02 (já no formato do KR) sem consumir: a chave do game server é desta conta?</summary>
    async ValueTask<bool> LoginKeyIsValidAsync(PacketReader p)
    {
        int start = p.Position;
        p.Str(22);
        uint uid = p.U32();
        p.Skip(4 + 2);
        var key = p.Str(64);
        p.Position = start;
        return await ctx.Kr.Sessions.ValidateGameLoginAsync(key) == uid;
    }

    public ValueTask OnDisconnectedAsync()
    {
        if (Kr.LoggedIn) ctx.Gacha?.Unregister(Kr.Player.AccountId);
        return Kr.OnDisconnectedAsync();
    }

    /// <summary>
    /// C->S 0x02: str id, u32 uid, u32 uid Ntreev, u16 comando, str chave do login, str versão, u32 versão de pacote,
    /// str MAC, str chave do game server (o nosso login manda a mesma chave nos dois).
    /// </summary>
    public static (string Id, uint Uid, string LoginKey, string Version, string Mac, string GameKey) ReadLogin(PacketReader p)
    {
        var id = p.Str(32);
        var uid = p.U32();
        p.U32();
        p.U16();
        var lkey = p.Str(64);
        var version = p.Str(32);
        p.U32();
        var mac = p.Str(64);
        var gkey = p.Str(64);
        return (id, uid, lkey, version, mac, gkey);
    }
}
