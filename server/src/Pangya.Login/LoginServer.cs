using System.Net;
using Pangya.Core.Net;
using Pangya.Data;
using Pangya.Domain.Accounts;
using Pangya.Domain.Players;
using Pangya.Protocol.KR645;
using Pangya.Protocol.KR645.Login;

namespace Pangya.Login;

/// <summary>Sobe um TcpServer por porta de login configurada (KR 645 e JP 983 na mesma porta, LoginRouter).</summary>
public static class LoginServer
{
    public static List<TcpServer> Create(ServerServices s, int[]? ports = null)
    {
        var cfg = s.Config;
        var players = new PlayerService(s.Players, Kr645GameData.Load(cfg.Data.IffPath), cfg.NewPlayer);
        var login = new LoginService(s.Accounts, s.Sessions, players);
        var ctx = new LoginContext(login, s.Registry, cfg.Login, cfg.Limits.MaxLoginAttemptsPerMinute);
        var jp = new Pangya.Protocol.JP983.Login.JpLoginContext(s.AccountService, login, s.Registry, cfg.Login);
        var ip = IPAddress.Parse(cfg.Network.BindIp);
        var servers = new List<TcpServer>();
        foreach (var port in ports ?? cfg.Login.Ports)
            servers.Add(new TcpServer("LOGIN", new IPEndPoint(ip, port), cfg.Limits, c => new LoginRouter(c, ctx, jp)));
        return servers;
    }

    public static Task RunAsync(ServerServices s, CancellationToken ct)
    {
        var servers = Create(s);
        var tasks = new Task[servers.Count];
        for (int i = 0; i < servers.Count; i++) tasks[i] = servers[i].StartAsync(ct);
        return Task.WhenAll(tasks);
    }
}
