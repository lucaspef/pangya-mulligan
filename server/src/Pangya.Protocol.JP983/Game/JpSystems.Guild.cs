using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Guilda ("club") do JP 983 (docs/server/protocolo/SPEC-jp983-guilda.md): a gestão é um site dentro do navegador do jogo.
/// 0xFB -> 0x1AD u32 1, str chave (o cliente abre login.aspx?ID=&amp;KID= ou, no login com guilda, lê o rss.aspx);
/// 0xA1 u8 lugar (entrou no navegador); 0xA2 (fechou o club): recarrega a guilda e o pang que o site pode ter mudado.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CWebKey = 0xFB, CWebEnter = 0xA1, CWebClubClosed = 0xA2, CGuildPending = 0xA5, CGuildAnswer = 0xA7,
        CGuildInviteAnswer = 0xA8;
    const ushort SWebKey = 0x1AD, SGuildPending = 0x116, SGuildAnswer = 0x117, SGuildInvites = 0x118, SGuildInviteAnswer = 0x119;
    /// <summary>Registro de pendente (55) e de convite recebido (44); tipos do pendente: 2 pediu para entrar, 3 convidado.</summary>
    const int PendingSize = 55, InviteNameSize = 36;
    const ushort PendingRequest = 2, PendingInvited = 3;

    /// <summary>Códigos do JP para a guilda (0x117, 0x119, MSN sub 0x126): 1 ok, 3 sem conta, 5 já em guilda, 6 já convidado,
    /// 7 só gestor, 8 10 convites, 9 5 guildas, 10 sem guilda, 11 não pediu; o resto 4 (falha).</summary>
    public static uint JpCode(Domain.Guilds.GuildCode c) => c switch
    {
        Domain.Guilds.GuildCode.Ok => 1, Domain.Guilds.GuildCode.NoAccount => 3, Domain.Guilds.GuildCode.AlreadyInGuild => 5,
        Domain.Guilds.GuildCode.AlreadyInvited => 6,
        Domain.Guilds.GuildCode.NoPermission or Domain.Guilds.GuildCode.NotManager or Domain.Guilds.GuildCode.NotMaster => 7,
        Domain.Guilds.GuildCode.InviteLimit => 8, Domain.Guilds.GuildCode.InvitedLimit => 9, Domain.Guilds.GuildCode.NoGuild => 10,
        Domain.Guilds.GuildCode.NotWaiting or Domain.Guilds.GuildCode.NotMember => 11, _ => 4,
    };

    /// <summary>Lugar do navegador aberto (0 = nenhum; 5 = club).</summary>
    public byte WebPlace { get; private set; }

    async Task<bool> HandleGuildAsync(PacketReader r)
    {
        switch (r.Id)
        {
            case CWebKey:
                r.Skip(r.Remaining);
                var key = await kr.Context.Sessions.IssueClubWebAsync(kr.Player.AccountId);
                conn.SendTranslated(new PacketWriter(SWebKey).U32(1).Str(key));
                return true;
            case CWebEnter:
                WebPlace = r.Remaining >= 1 ? r.U8() : (byte)0;
                r.Skip(r.Remaining);
                return true;
            case CWebClubClosed:
            {
                r.Skip(r.Remaining);
                WebPlace = 0;
                bool changed = await kr.RefreshGuildAsync();
                var p = kr.Player;
                if (await ctx.Players.Store.LoadAsync(p.AccountId) is { } fresh && fresh.Pang != p.Pang)   // o site pode cobrar pang
                {
                    p.Pang = fresh.Pang;
                    conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
                }
                if (changed) Log.Info($"{conn} JP: guilda depois do site: {p.Guild?.Name ?? "nenhuma"} (cargo {p.Guild?.Class ?? 0})");
                return true;
            }
            case CGuildPending:
                r.Skip(r.Remaining);
                conn.SendTranslated(await PendingListAsync());
                return true;
            case CGuildAnswer:
            {
                long uid = r.U32();
                int gid = (int)r.U32();
                bool approve = r.U8() != 0;
                r.Skip(r.Remaining);
                var code = kr.Context.Guilds is { } gs ? await gs.AnswerRequestAsync(kr.Player.AccountId, gid, uid, approve) : Domain.Guilds.GuildCode.Failed;
                if (code == Domain.Guilds.GuildCode.Ok && kr.Context.World.Find(uid) is KR645.Game.GameHandler other) await other.RefreshGuildAsync();
                conn.SendTranslated(new PacketWriter(SGuildAnswer).U32(JpCode(code)).U32((uint)gid));
                Log.Info($"{conn} JP: pedido de guilda de {uid} {(approve ? "aceito" : "recusado")}: {code}");
                return true;
            }
            case CGuildInviteAnswer:
            {
                r.U32();                                                         // guilda (a chave da janela)
                int invite = (int)r.U32();
                bool accept = r.U8() != 0;
                r.Skip(r.Remaining);
                var (code, gid) = kr.Context.Guilds is { } gs ? await gs.AnswerInviteAsync(kr.Player.AccountId, invite, accept)
                    : (Domain.Guilds.GuildCode.Failed, 0);
                bool joined = accept && code == Domain.Guilds.GuildCode.Ok;
                if (joined) await kr.RefreshGuildAsync();
                conn.SendTranslated(new PacketWriter(SGuildInviteAnswer).U32(JpCode(code)).U8(joined ? (byte)1 : (byte)0).U32((uint)gid));
                Log.Info($"{conn} JP: convite de guilda {invite} {(accept ? "aceito" : "recusado")}: {code}");
                return true;
            }
        }
        return false;
    }

    /// <summary>0x116 u16 n, n × 55 (u32 uid, u32 guilda, char[22] login, char[22] nick, u8 ícone, u16 tipo): os pedidos para
    /// entrar (2) e os convidados que não responderam (3). Só para o gestor; senão a lista vai vazia.</summary>
    async Task<PacketWriter> PendingListAsync()
    {
        var w = new PacketWriter(SGuildPending, 8 + 20 * PendingSize);
        if (kr.Context.Guilds is not { } gs || kr.Player.Guild is not { } tag || !Domain.Guilds.GuildClass.IsManager(tag.Class))
            return w.U16(0);
        var (members, _) = await gs.Store.MembersAsync(tag.Id, 1, Domain.Guilds.GuildService.MaxMembers * 4);
        var invites = gs.Invites == null ? [] : await gs.Invites.OfGuildAsync(tag.Id);
        var rows = new List<(long Uid, string Login, string Nick, ushort Type)>();
        foreach (var m in members) if (m.Class == Domain.Guilds.GuildClass.Waiting) rows.Add((m.AccountId, m.Nickname, m.Nickname, PendingRequest));
        foreach (var i in invites) rows.Add((i.AccountId, i.Login, i.Nickname, PendingInvited));
        w.U16((ushort)rows.Count);
        foreach (var (uid, login, nick, type) in rows) w.U32((uint)uid).U32((uint)tag.Id).Fixed(login, 22).Fixed(nick, 22).U8(0).U16(type);
        return w;
    }

    /// <summary>Login: os convites recebidos (0x118), se houver.</summary>
    public async Task SendInvitesAsync()
    {
        if (kr.Context.Guilds?.Invites is not { } store) return;
        var w = await InvitesPacketAsync(store, kr.Player.AccountId);
        if (w.Body.Length > 4) conn.SendTranslated(w); else w.Dispose();
    }

    /// <summary>0x118 u16 n, n × 44 (u32 convite, u32 guilda, char[36] nome da guilda): os convites que o jogador recebeu.</summary>
    public static async Task<PacketWriter> InvitesPacketAsync(Domain.Guilds.IGuildInviteStore store, long account)
    {
        var list = await store.OfAccountAsync(account);
        var w = new PacketWriter(SGuildInvites, 8 + list.Count * 44).U16((ushort)list.Count);
        foreach (var i in list) w.U32((uint)i.Id).U32((uint)i.GuildId).Fixed(i.GuildName, InviteNameSize);
        return w;
    }
}
