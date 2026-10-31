using Dapper;
using Pangya.Domain.Rooms;

namespace Pangya.Data;

/// <summary>Grand Prix concluídos (migração 014).</summary>
public sealed class GrandPrixRepository(Db db) : IGrandPrixStore
{
    sealed record Row(int Link, int BestPosition);

    public async Task<Dictionary<int, int>> CompletedAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var map = new Dictionary<int, int>();
        foreach (var r in await c.QueryAsync<Row>("select link, best_position from grand_prix_results where account_id = @account", new { account }))
            map[r.Link] = r.BestPosition;
        return map;
    }

    public async Task SaveAsync(long account, int link, int position)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into grand_prix_results(account_id, link, best_position) values (@account, @link, @position)
            on conflict (account_id, link) do update set best_position = least(grand_prix_results.best_position, excluded.best_position)
            """, new { account, link, position });
    }
}
