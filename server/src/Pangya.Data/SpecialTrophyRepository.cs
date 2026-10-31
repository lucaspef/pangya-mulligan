using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Troféus especiais (migração 015).</summary>
public sealed class SpecialTrophyRepository(Db db) : ISpecialTrophyStore
{
    sealed record Row(int TypeId, int Quantity);

    public async Task<List<(int TypeId, int Quantity)>> ListAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var list = new List<(int, int)>();
        foreach (var r in await c.QueryAsync<Row>(
            "select type_id, quantity from special_trophies where account_id = @account order by given_at", new { account }))
            list.Add((r.TypeId, r.Quantity));
        return list;
    }

    public async Task AddAsync(long account, int typeId, int quantity)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into special_trophies(account_id, type_id, quantity) values (@account, @typeId, @quantity)
            on conflict (account_id, type_id) do update set quantity = special_trophies.quantity + excluded.quantity, given_at = now()
            """, new { account, typeId, quantity });
    }
}
