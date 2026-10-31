using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>World Tour (migração 022).</summary>
public sealed class WorldTourRepository(Db db) : IWorldTourStore
{
    sealed record Row(int MapFlag, bool GotGift);

    public async Task<WorldTourProgress?> GetAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>("select map_flag, got_gift from world_tour where account_id = @account", new { account });
        return r == null ? null : new WorldTourProgress(r.MapFlag, r.GotGift);
    }

    public async Task SaveAsync(long account, WorldTourProgress p)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into world_tour(account_id, map_flag, got_gift) values (@account, @flag, @got)
            on conflict (account_id) do update set map_flag = excluded.map_flag, got_gift = excluded.got_gift
            """, new { account, flag = p.MapFlag, got = p.GotGift });
    }
}
