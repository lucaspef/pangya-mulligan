namespace Pangya.Domain.Servers;

/// <summary>Um servidor online anunciado na lista do cliente.</summary>
/// <param name="EventFlags">eventFlags do sGameServerInfo (bits crus do u32; só o JP usa).</param>
/// <param name="EventValue">eventValue do sGameServerInfo (bits crus do u32; só o JP usa).</param>
/// <param name="IconIndex">iconIndex do sGameServerInfo (só o JP usa).</param>
public sealed record ServerInfo(int Id, string Kind, string Name, string Address, int Port, int MaxUsers, int CurUsers, int Flags,
    int EventFlags = 0, int EventValue = 0, int IconIndex = 0);

/// <summary>Registro dos servidores online (implementado em Pangya.Data).</summary>
public interface IServerRegistry
{
    /// <summary>Cria/renova o registro; vale até agora + ttl.</summary>
    Task HeartbeatAsync(ServerInfo server, TimeSpan ttl);
    Task RemoveAsync(int id);
    Task<IReadOnlyList<ServerInfo>> ListAsync(string kind);
}
