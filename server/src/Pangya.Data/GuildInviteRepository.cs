using Dapper;
using Pangya.Domain.Guilds;

namespace Pangya.Data;

/// <summary>Convites de guilda (migração 019).</summary>
public sealed class GuildInviteRepository(Db db) : IGuildInviteStore
{
    const string Select = """
        select i.id, i.guild_id as guildid, i.account_id as accountid, i.inviter_id as inviterid, g.name as guildname,
               a.login, coalesce(a.nickname, '') as nickname
        from guild_invites i join guilds g on g.id = i.guild_id join accounts a on a.id = i.account_id
        """;

    sealed record Row(int Id, int GuildId, long AccountId, long InviterId, string GuildName, string Login, string Nickname);
    static GuildInvite Map(Row r) => new(r.Id, r.GuildId, r.AccountId, r.InviterId, r.GuildName, r.Login, r.Nickname);

    public async Task<int?> AddAsync(int guildId, long accountId, long inviterId)
    {
        await using var c = await db.OpenAsync();
        return await c.ExecuteScalarAsync<int?>("""
            insert into guild_invites(guild_id, account_id, inviter_id) values (@guildId, @accountId, @inviterId)
            on conflict (guild_id, account_id) do nothing returning id
            """, new { guildId, accountId, inviterId });
    }

    public async Task<bool> RemoveAsync(int guildId, long accountId)
    {
        await using var c = await db.OpenAsync();
        return await c.ExecuteAsync("delete from guild_invites where guild_id = @guildId and account_id = @accountId", new { guildId, accountId }) > 0;
    }

    public async Task RemoveAllOfAsync(long accountId)
    {
        await using var c = await db.OpenAsync();
        await c.ExecuteAsync("delete from guild_invites where account_id = @accountId", new { accountId });
    }

    public async Task<GuildInvite?> GetAsync(int id)
    {
        await using var c = await db.OpenAsync();
        var r = await c.QuerySingleOrDefaultAsync<Row>(Select + " where i.id = @id", new { id });
        return r == null ? null : Map(r);
    }

    public async Task<List<GuildInvite>> OfGuildAsync(int guildId)
    {
        await using var c = await db.OpenAsync();
        var list = new List<GuildInvite>();
        foreach (var r in await c.QueryAsync<Row>(Select + " where i.guild_id = @guildId order by i.id", new { guildId })) list.Add(Map(r));
        return list;
    }

    public async Task<List<GuildInvite>> OfAccountAsync(long accountId)
    {
        await using var c = await db.OpenAsync();
        var list = new List<GuildInvite>();
        foreach (var r in await c.QueryAsync<Row>(Select + " where i.account_id = @accountId order by i.id", new { accountId })) list.Add(Map(r));
        return list;
    }
}
