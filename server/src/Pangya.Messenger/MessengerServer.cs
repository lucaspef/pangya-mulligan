using System.Net;
using Pangya.Core.Logging;
using Pangya.Core.Config;
using Pangya.Core.Net;
using Pangya.Data;
using Pangya.Domain.Game;
using Pangya.Domain.Messenger;
using Pangya.Domain.Servers;
using Pangya.Protocol.KR645.Messenger;

namespace Pangya.Messenger;

/// <summary>
/// Mensageiro: escuta a porta e se anuncia no registro como "messenger" (cliente KR 645; o login manda a lista no 0x09
/// e o game no 0xFA) ou, com outra fábrica de conexão, "messenger-jp" (cliente JP 983). Precisa do mundo do game server
/// do mesmo processo para confirmar quem está jogando.
/// </summary>
public sealed class MessengerServer
{
    public static readonly TimeSpan HeartbeatInterval = TimeSpan.FromSeconds(10);

    readonly ServerServices services;
    readonly MessengerConfig config;
    readonly string type;
    public MessengerContext Context { get; }
    public TcpServer Tcp { get; }

    /// <summary>Mensageiro do KR 645.</summary>
    public MessengerServer(ServerServices s, GameWorld world, int? portOverride = null)
        : this(s, world, s.Config.Messenger, "messenger", s.Config.Game.Id, "MSN", (c, ctx) => new MessengerHandler(c, ctx), portOverride) { }

    /// <param name="handler">conexão nova -> handler (o JP põe tradutor, codepage e o hello dele).</param>
    public MessengerServer(ServerServices s, GameWorld world, MessengerConfig config, string type, int gameServerId, string name,
        Func<Connection, MessengerContext, IConnectionHandler> handler, int? portOverride = null)
    {
        services = s;
        this.config = config;
        this.type = type;
        var cfg = s.Config;
        Context = new MessengerContext(new FriendService(s.Friends, config.MaxFriends), uid => world.Find(uid), gameServerId, s.Notes,
            s.Guilds) { GuildInvites = s.GuildInvites, Accounts = s.Accounts };
        Tcp = new TcpServer(name, new IPEndPoint(IPAddress.Parse(cfg.Network.BindIp), portOverride ?? config.Port), cfg.Limits,
            c => handler(c, Context));
    }

    ServerInfo Info => new(config.Id, type, config.Name, services.Config.Network.PublicIp, Tcp.Port, config.MaxUsers,
        Context.OnlineCount, 0);

    public async Task RunAsync(CancellationToken ct)
    {
        var accept = Tcp.StartAsync(ct);
        try
        {
            while (!ct.IsCancellationRequested)
            {
                try { await services.Registry.HeartbeatAsync(Info, HeartbeatInterval * 3); }
                catch (Exception e) { Log.Warn($"MSN: falha ao renovar registro: {e.Message}"); }
                await Task.Delay(HeartbeatInterval, ct);
            }
        }
        catch (OperationCanceledException) { }
        finally
        {
            try { await services.Registry.RemoveAsync(config.Id); } catch { /* banco fora: o registro expira sozinho */ }
            await accept;
        }
    }
}
