using Dapper;
using Pangya.Domain.Shop;

namespace Pangya.Data;

/// <summary>Saldo da Point Shop antiga (migração 018).</summary>
public sealed class TikiRepository(Db db) : ITikiStore
{
    public async Task<int> PointsAsync(long account)
    {
        await using var c = await db.OpenAsync();
        return await c.ExecuteScalarAsync<int?>("select points from tiki_points where account_id = @account", new { account }) ?? 0;
    }

    public async Task SetPointsAsync(long account, int points)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into tiki_points(account_id, points) values (@account, @points)
            on conflict (account_id) do update set points = excluded.points
            """, new { account, points });
    }
}
