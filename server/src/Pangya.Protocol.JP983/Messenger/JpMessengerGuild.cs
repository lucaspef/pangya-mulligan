using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Guilds;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;
using Pangya.Protocol.KR645.Messenger;

namespace Pangya.Protocol.JP983.Messenger;

/// <summary>
/// Mensageiro do JP com os pedidos de guilda que o KR não tem (docs/server/protocolo/SPEC-jp983-guilda.md §2.4; no KR 0x26/0x27
/// são outros pacotes): 0x26 convidar/cancelar pelo login -> sub 0x126 (código) ao gestor e 0x127 ao convidado; 0x27
/// "entrei na guilda" (só ler); 0x16B "a guilda mudou" -> sub 0x128 aos gestores online. O resto vai ao handler do KR.
/// </summary>
sealed class JpMessengerHandler(Connection conn, MessengerContext ctx, MessengerHandler kr) : IConnectionHandler
{
    const ushort CInvite = 0x26, CJoined = 0x27, CGuildChanged = 0x16B;
    const ushort SubInviteResult = 0x126, SubInvited = 0x127, SubGuildChanged = 0x128;

    public ValueTask OnConnectedAsync() => kr.OnConnectedAsync();
    public ValueTask OnDisconnectedAsync() => kr.OnDisconnectedAsync();

    public async ValueTask OnPacketAsync(PacketReader p)
    {
        if (kr.AccountId == 0 || p.Id is not (CInvite or CJoined or CGuildChanged)) { await kr.OnPacketAsync(p); return; }
        try
        {
            switch (p.Id)
            {
                case CInvite: await InviteAsync(p); break;
                case CGuildChanged: await GuildChangedAsync(p.Remaining >= 4 ? (int)p.U32() : 0); break;
                default: p.Skip(p.Remaining); break;
            }
        }
        catch (Exception e) { Log.Error($"{conn} mensageiro JP: falha no pedido de guilda 0x{p.Id:X}", e); }
    }

    /// <summary>0x26 str meu login, str login do alvo, u32 minha guilda, u8 op (1 convidar, 0 cancelar).</summary>
    async Task InviteAsync(PacketReader p)
    {
        p.Str();
        string target = p.Str();
        int gid = (int)p.U32();
        bool invite = p.U8() != 0;
        p.Skip(p.Remaining);
        if (ctx.Guilds == null || ctx.Accounts == null) { Reply(SubInviteResult, 4); return; }
        var guilds = new GuildService(ctx.Guilds, ctx.GuildInvites);
        if (await ctx.Accounts.FindByLoginAsync(target) is not { } acc) { Reply(SubInviteResult, JpSystems.JpCode(GuildCode.NoAccount)); return; }
        if (!invite)
        {
            Reply(SubInviteResult, JpSystems.JpCode(await guilds.CancelInviteAsync(kr.AccountId, gid, acc.Id)));
            return;
        }
        var (code, inv) = await guilds.InviteAsync(kr.AccountId, gid, acc.Id);
        Reply(SubInviteResult, JpSystems.JpCode(code));
        Log.Info($"{conn} mensageiro JP: convite de guilda {gid} para {target}: {code}");
        if (code != GuildCode.Ok || inv == null) return;
        if (ctx.Find(acc.Id) is { } h)                                       // no mensageiro: a caixa "convidado para a guilda"
            h.Connection.Send(MessengerContext.Sub(SubInvited, 48).U32((uint)inv.GuildId).U32((uint)inv.Id).Str(inv.GuildName));
        if (ctx.FindGame(acc.Id) is GameHandler g && g.Connection.Translator is JpTranslator && ctx.GuildInvites is { } store)
            g.Connection.SendTranslated(await JpSystems.InvitesPacketAsync(store, acc.Id));   // no jogo: a lista de convites
    }

    /// <summary>0x16B u32 guilda: os gestores online relêem os pendentes (sub 0x128 -> 0xA5).</summary>
    async Task GuildChangedAsync(int gid)
    {
        if (ctx.Guilds == null || gid == 0) return;
        var (members, _) = await ctx.Guilds.MembersAsync(gid, 1, GuildService.MaxMembers * 4);
        foreach (var m in members)
            if (GuildClass.IsManager(m.Class) && ctx.Find(m.AccountId) is { } h)
                h.Connection.Send(MessengerContext.Sub(SubGuildChanged, 8).U32((uint)gid));
    }

    void Reply(ushort sub, uint code) => conn.Send(MessengerContext.Sub(sub, 8).U32(code));
}
