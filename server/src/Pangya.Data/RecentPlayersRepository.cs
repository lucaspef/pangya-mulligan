using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Últimos jogadores com quem se jogou (migração 023): uma linha por par, a data da última partida juntos.</summary>
public sealed class RecentPlayersRepository(Db db) : IRecentPlayersStore
{
    sealed record Row(long Id, string Login, string? Nickname);

    public async Task AddAsync(long account, IReadOnlyList<long> others)
    {
        if (others.Count == 0) return;
        var ids = new long[others.Count];
        for (int i = 0; i < ids.Length; i++) ids[i] = others[i];
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into recent_players(account_id, other_id, played_at) select @account, unnest(@others), now()
            on conflict (account_id, other_id) do update set played_at = excluded.played_at
            """, new { account, others = ids });
    }

    public async Task<List<RecentPlayer>> LastAsync(long account, int count)
    {
        await using var c = await db.OpenAsync();
        var rows = await c.QueryAsync<Row>("""
            select a.id, a.login, a.nickname from recent_players r join accounts a on a.id = r.other_id
            where r.account_id = @account order by r.played_at desc limit @count
            """, new { account, count });
        var list = new List<RecentPlayer>();
        foreach (var r in rows) list.Add(new RecentPlayer(r.Id, r.Login, r.Nickname ?? ""));
        return list;
    }
}
