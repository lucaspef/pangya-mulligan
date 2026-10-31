namespace Pangya.Domain.Players;

/// <summary>Jogador com quem se jogou por último (login e nickname para o perfil).</summary>
public readonly record struct RecentPlayer(long AccountId, string Login, string Nickname);

/// <summary>Últimos jogadores com quem cada conta jogou (migração 023).</summary>
public interface IRecentPlayersStore
{
    /// <summary>Registra que a conta jogou com estas (a mais recente fica primeiro).</summary>
    Task AddAsync(long account, IReadOnlyList<long> others);
    Task<List<RecentPlayer>> LastAsync(long account, int count);
}
