using System.Net;
using Pangya.Core.Config;
using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Data;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Servers;
using Pangya.Protocol.JP983;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Game;

/// <summary>
/// Game server do cliente JP 983: a mesma lógica do game server do KR (GameHandler, traduzido por JpTranslator), com
/// porta, mundo (canais, salas) e registro próprios (tipo "game-jp", que só o login do JP lista). Dados do jogo: os do
/// cliente JP (pangya_jp.iff, Jp983GameData), que também dizem o que o cliente conhece.
/// </summary>
public sealed class JpGameServer
{
    readonly ServerServices services;
    readonly GameConfig config;
    public GameWorld World { get; }
    public TcpServer Tcp { get; }
    public JpGameContext Context { get; }
    /// <summary>Evento do Grand Zodiac (salas abertas pelo servidor).</summary>
    public JpZodiacEvent Zodiac { get; }
    /// <summary>Golden Time (rodadas de sorteio).</summary>
    public JpGoldenTime GoldenTime { get; }
    /// <summary>Bot GM Event (sala de torneio do servidor com prêmios).</summary>
    public JpBotGmEvent BotGm { get; }

    public JpGameServer(ServerServices s, int? portOverride = null)
    {
        services = s;
        config = s.Config.GameJp ?? throw new InvalidOperationException("game-jp precisa da seção GameJp na configuração");
        World = new GameWorld(config);
        // dados do jogo do cliente JP (preços, stats, cards...); sem o pangya_jp.iff, os do KR e sem filtro de itens
        var jpData = Jp983GameData.Load(s.Config.Data.JpIffPath);
        if (jpData == null) Log.Warn($"GAME-JP: {s.Config.Data.JpIffPath} não existe: usando os dados do KR, itens só do KR vão para o cliente JP");
        else Log.Info($"GAME-JP: {jpData.Count} typeids do pangya_jp.iff");
        IGameData data = jpData ?? (IGameData)Kr645GameData.Load(s.Config.Data.IffPath);
        World.Rooms.Courses = config.Courses.Length > 0 ? GameServer.Bytes(config.Courses) : data.Courses;
        JpZodiacEvent? zodiac = null;
        JpGoldenTime? golden = null;
        JpBotGmEvent? botGm = null;
        var kr = new GameContext(World, s.Sessions, new PlayerService(s.Players, data, s.Config.NewPlayer), data, s.Config.Lottery,
            s.Registry, s.Guilds, s.Mail, s.Notes, s.Friends) { Audit = s.Audit, GameServerType = Jp983.GameServerType, MessengerType = Protocol.JP983.Messenger.JpMessenger.ServerType, TutorialPrizes = true,
            ZodiacGame = (room, mass, sync) => new GrandZodiacGame(room, mass, new JpZodiacOutput(room, s.ZodiacPoints), sync),
            GmExtra = cmd => zodiac?.Gm(cmd) ?? golden?.Gm(cmd) ?? botGm?.Gm(cmd),
            RecentPlayers = s.RecentPlayers, FieldSpots = s.FieldSpots,
            SpecialTrophies = s.SpecialTrophies, GuildInvites = s.GuildInvites };
        Context = new JpGameContext(kr, jpData, config.Property, s.Config.Memorial, s.Achievements, s.GrandPrix, s.Attendance,
            s.Config.Attendance)
        {
            DailyQuests = jpData == null ? null : new Domain.Achievements.DailyQuestService(jpData, jpData, s.DailyQuests, kr.Shop),
            Tiki = jpData == null ? null : new Domain.Shop.TikiService(s.Players, jpData, s.Tiki, kr.Shop),
            ZodiacPoints = s.ZodiacPoints,
            Gacha = s.Gacha,
            LoginRewards = new LoginRewardService(s.LoginRewards, kr.Mail, config.LoginRewards),
            WorldTour = new WorldTourService(s.WorldTour, kr.Mail, config.WorldTour),
            Boxes = new Domain.Shop.BoxService(kr.Shop, s.Config.Boxes),
        };
        Zodiac = zodiac = new JpZodiacEvent(kr);
        GoldenTime = golden = new JpGoldenTime(kr, config.GoldenTime);
        BotGm = botGm = new JpBotGmEvent(kr, config.BotGmEvent);
        Tcp = new TcpServer("GAME-JP", new IPEndPoint(IPAddress.Parse(s.Config.Network.BindIp), portOverride ?? config.Port),
            s.Config.Limits, c => new JpGameHandler(c, Context));
    }

    /// <summary>No registro do JP, Flags = propriedade do servidor (o login do JP a manda na lista).</summary>
    ServerInfo Info => new(config.Id, Jp983.GameServerType, config.Name, services.Config.Network.PublicIp, Tcp.Port,
        config.MaxUsers, World.OnlineCount, (int)config.Property, (int)config.EventFlags, (int)config.EventValue, config.IconIndex);

    public async Task RunAsync(CancellationToken ct)
    {
        _ = Zodiac.ScheduleAsync(config.ZodiacSchedule, ct);
        _ = GoldenTime.ScheduleAsync(ct);
        _ = BotGm.ScheduleAsync(ct);
        _ = GameServer.LoadFieldSpotsAsync(services.FieldSpots, "GAME-JP");    // em paralelo: a porta abre já
        await GameServer.RunRegisteredAsync(services.Registry, Tcp, () => Info,
            () => GameServer.FlushFieldSpotsAsync(services.FieldSpots, "GAME-JP"), ct);
    }
}
