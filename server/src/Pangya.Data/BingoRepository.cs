using Dapper;
using Pangya.Domain.Players;

namespace Pangya.Data;

/// <summary>Bingo (migração 026).</summary>
public sealed class BingoRepository(Db db) : IBingoStore
{
    sealed record Row(int Tries, int Aimed, int Lines);

    public async Task<BingoProgress?> GetAsync(long account)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>("select tries, aimed, lines from bingo where account_id = @account", new { account });
        return r == null ? null : new BingoProgress(r.Tries, r.Aimed, r.Lines);
    }

    public async Task SaveAsync(long account, BingoProgress p)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("""
            insert into bingo(account_id, tries, aimed, lines) values (@account, @tries, @aimed, @lines)
            on conflict (account_id) do update set tries = excluded.tries, aimed = excluded.aimed, lines = excluded.lines
            """, new { account, tries = p.Tries, aimed = p.Aimed, lines = p.Lines });
    }
}
