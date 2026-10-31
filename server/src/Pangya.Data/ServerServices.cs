using Pangya.Core.Config;
using Pangya.Domain.Accounts;
using Pangya.Domain.Auth;
using Pangya.Domain.Players;
using Pangya.Domain.Servers;

namespace Pangya.Data;

/// <summary>Serviços compartilhados pelos servidores de um processo (um pool de conexões com o banco).</summary>
public sealed class ServerServices : IAsyncDisposable
{
    public PangyaConfig Config { get; }
    public Db Db { get; }
    public IAccountStore Accounts { get; }
    public AccountService AccountService { get; }
    public SessionService Sessions { get; }
    public IServerRegistry Registry { get; }
    public IPlayerStore Players { get; }
    public Pangya.Domain.Guilds.IGuildStore Guilds { get; }
    public Pangya.Domain.Mail.IMailStore Mail { get; }
    public Pangya.Domain.Messenger.IFriendStore Friends { get; }
    public Pangya.Domain.Messenger.INoteStore Notes { get; }
    public Pangya.Domain.Ranking.IRankingStore Ranking { get; }
    public Pangya.Domain.Admin.AuditLog Audit { get; }
    public Pangya.Domain.Rooms.IBotKnowledgeStore BotKnowledge { get; }
    /// <summary>Pontos dos itens de campo: uma instância para os game servers KR e JP do processo.</summary>
    public Pangya.Domain.Rooms.FieldSpots FieldSpots { get; }
    public Pangya.Domain.Achievements.IAchievementStore Achievements { get; }
    public Pangya.Domain.Rooms.IGrandPrixStore GrandPrix { get; }
    public Pangya.Domain.Players.ISpecialTrophyStore SpecialTrophies { get; }
    public Pangya.Domain.Players.IAttendanceStore Attendance { get; }
    public Pangya.Domain.Achievements.IDailyQuestStore DailyQuests { get; }
    public Pangya.Domain.Shop.ITikiStore Tiki { get; }
    public Pangya.Domain.Guilds.IGuildInviteStore GuildInvites { get; }
    public Pangya.Domain.Players.IZodiacPointsStore ZodiacPoints { get; }
    public Pangya.Domain.Players.ILoginRewardStore LoginRewards { get; }
    public Pangya.Domain.Players.IWorldTourStore WorldTour { get; }
    public Pangya.Domain.Players.IRecentPlayersStore RecentPlayers { get; }
    /// <summary>Gacha do JP: o site sorteia no jogador online do game-jp.</summary>
    public Pangya.Domain.Shop.GachaDesk Gacha { get; }

    public ServerServices(PangyaConfig config)
    {
        Config = config;
        Db = new Db(config.Database.ConnectionString);
        Accounts = new AccountRepository(Db);
        AccountService = new AccountService(Accounts, config.Limits.MaxLoginAttemptsPerMinute, config.Web.AutoRegister);
        Sessions = new SessionService(new SessionRepository(Db));
        Registry = new ServerRegistry(Db);
        Players = new PlayerRepository(Db);
        Guilds = new GuildRepository(Db);
        Mail = new MailRepository(Db);
        Friends = new FriendRepository(Db);
        Notes = new NoteRepository(Db);
        Ranking = new RankingRepository(Db);
        Audit = new Pangya.Domain.Admin.AuditLog(new AuditRepository(Db));
        BotKnowledge = new BotKnowledgeRepository(Db);
        FieldSpots = new Pangya.Domain.Rooms.FieldSpots(new FieldSpotRepository(Db));
        Achievements = new AchievementRepository(Db);
        GrandPrix = new GrandPrixRepository(Db);
        SpecialTrophies = new SpecialTrophyRepository(Db);
        Attendance = new AttendanceRepository(Db);
        DailyQuests = new DailyQuestRepository(Db);
        Tiki = new TikiRepository(Db);
        GuildInvites = new GuildInviteRepository(Db);
        ZodiacPoints = new ZodiacPointsRepository(Db);
        LoginRewards = new LoginRewardRepository(Db);
        WorldTour = new WorldTourRepository(Db);
        RecentPlayers = new RecentPlayersRepository(Db);
        Gacha = new Pangya.Domain.Shop.GachaDesk(config.Gacha);
    }

    public ValueTask DisposeAsync() => Db.DisposeAsync();
}
