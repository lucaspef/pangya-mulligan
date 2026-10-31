using System.Net;
using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Data;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Servers;
using Pangya.Protocol.KR645;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Game;

/// <summary>Game server (cliente KR 645): escuta a porta, e se anuncia no registro de servidores a cada 10 s.</summary>
public sealed class GameServer
{
    public static readonly TimeSpan HeartbeatInterval = TimeSpan.FromSeconds(10);

    readonly ServerServices services;
    public GameWorld World { get; }
    public TcpServer Tcp { get; }
    /// <summary>Serviços do jogo (correio, guildas, loja...), para administração e testes.</summary>
    public Pangya.Protocol.KR645.Game.GameContext Context { get; }

    public GameServer(ServerServices s, int? portOverride = null)
    {
        services = s;
        var cfg = s.Config;
        World = new GameWorld(cfg.Game);
        var data = Kr645GameData.Load(cfg.Data.IffPath);
        World.Rooms.Courses = cfg.Game.Courses.Length > 0 ? Bytes(cfg.Game.Courses) : data.Courses;
        var ctx = Context = new GameContext(World, s.Sessions, new PlayerService(s.Players, data, cfg.NewPlayer), data, cfg.Lottery, s.Registry, s.Guilds, s.Mail, s.Notes, s.Friends)
            { SpecialTrophies = s.SpecialTrophies };
        ctx.Audit = s.Audit;
        ctx.BotKnowledge = new Pangya.Domain.Rooms.BotKnowledge(s.BotKnowledge);
        ctx.FieldSpots = s.FieldSpots;
        Tcp = new TcpServer("GAME", new IPEndPoint(IPAddress.Parse(cfg.Network.BindIp), portOverride ?? cfg.Game.Port), cfg.Limits,
            c => new GameHandler(c, ctx));
    }

    internal static byte[] Bytes(int[] v)
    {
        var b = new byte[v.Length];
        for (int i = 0; i < v.Length; i++) b[i] = (byte)v[i];
        return b;
    }

    /// <summary>Grava o que o bot aprendeu desde a última vez (falha = tenta de novo no próximo ciclo).</summary>
    async Task FlushBotKnowledgeAsync()
    {
        try { await Context.BotKnowledge.FlushAsync(World.Rooms.Sync); }
        catch (Exception e) { Log.Warn($"GAME: falha ao gravar o aprendizado do bot: {e.Message}"); }
        await FlushFieldSpotsAsync(services.FieldSpots, "GAME");
    }

    /// <summary>Pontos dos itens de campo: carregados uma vez por processo (KR e JP dividem a instância).</summary>
    internal static async Task LoadFieldSpotsAsync(Pangya.Domain.Rooms.FieldSpots spots, string name)
    {
        try
        {
            int n = await spots.LoadAsync();
            if (n > 0) Log.Info($"{name}: {n} pontos de itens de campo carregados");
        }
        catch (Exception e) { Log.Warn($"{name}: falha ao carregar os pontos de itens de campo: {e.Message}"); }
    }

    /// <summary>Grava os pontos de itens de campo aprendidos desde a última vez (falha = tenta de novo no próximo ciclo).</summary>
    internal static async Task FlushFieldSpotsAsync(Pangya.Domain.Rooms.FieldSpots spots, string name)
    {
        try
        {
            int n = await spots.FlushAsync();
            if (n > 0) Log.Info($"{name}: {n} pontos de itens de campo gravados");
        }
        catch (Exception e) { Log.Warn($"{name}: falha ao gravar os pontos de itens de campo: {e.Message}"); }
    }

    ServerInfo Info => new(services.Config.Game.Id, "game", services.Config.Game.Name, services.Config.Network.PublicIp,
        Tcp.Port, services.Config.Game.MaxUsers, World.OnlineCount, 0);

    public async Task RunAsync(CancellationToken ct)
    {
        try
        {
            var (holes, cals) = await Context.BotKnowledge.LoadAsync();
            Log.Info($"GAME: aprendizado do bot carregado ({holes} buracos, {cals} calibrações)");
        }
        catch (Exception e) { Log.Warn($"GAME: falha ao carregar o aprendizado do bot: {e.Message}"); }
        await LoadFieldSpotsAsync(services.FieldSpots, "GAME");
        await RunRegisteredAsync(services.Registry, Tcp, () => Info, FlushBotKnowledgeAsync, ct);
    }

    /// <summary>
    /// Aceita conexões e se anuncia no registro de servidores a cada <see cref="HeartbeatInterval"/> (com uma tarefa extra
    /// por ciclo e no fim); ao parar, sai do registro.
    /// </summary>
    internal static async Task RunRegisteredAsync(IServerRegistry registry, TcpServer tcp, Func<ServerInfo> info, Func<Task> tick,
        CancellationToken ct)
    {
        var accept = tcp.StartAsync(ct);
        try
        {
            while (!ct.IsCancellationRequested)
            {
                try { await registry.HeartbeatAsync(info(), HeartbeatInterval * 3); }
                catch (Exception e) { Log.Warn($"{tcp.Name}: falha ao renovar registro: {e.Message}"); }
                await tick();
                await Task.Delay(HeartbeatInterval, ct);
            }
        }
        catch (OperationCanceledException) { }
        finally
        {
            await tick();
            try { await registry.RemoveAsync(info().Id); } catch { /* banco fora: o registro expira sozinho */ }
            await accept;
        }
    }
}
