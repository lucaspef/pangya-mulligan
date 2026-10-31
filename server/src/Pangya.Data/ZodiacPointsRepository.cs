using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Pontos do Grand Zodiac (migração 020).</summary>
public sealed class ZodiacPointsRepository(Db db) : IZodiacPointsStore
{
    public async Task<int> GetAsync(long account)
    {
        await using var c = await db.OpenAsync();
        return await c.ExecuteScalarAsync<int?>("select points from zodiac_points where account_id = @account", new { account }) ?? 0;
    }

    public async Task<int> AddAsync(long account, int points)
    {
        await using var c = await db.OpenAsync();
        return await c.ExecuteScalarAsync<int>("""
            insert into zodiac_points(account_id, points) values (@account, @points)
            on conflict (account_id) do update set points = zodiac_points.points + excluded.points
            returning points
            """, new { account, points });
    }
}
