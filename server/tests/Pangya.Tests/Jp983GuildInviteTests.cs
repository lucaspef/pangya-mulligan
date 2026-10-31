using Pangya.Core.Config;
using Pangya.Core.Net;
using Pangya.Domain.Guilds;
using Pangya.Messenger;
using Pangya.Protocol.JP983.Messenger;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Guilda do JP fora do site (SPEC-jp983-guilda.md §2): pedidos (0xA5/0xA7) e convites (MSN 0x26, 0x118, 0xA8).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task GuildRequestsAndInvitesFromTheClient()
    {
        await using var env = new JpEnv();
        var msn = new MessengerServer(env.S, env.Game.World, new MessengerConfig { Id = 39305, Port = 0 }, JpMessenger.ServerType,
            39302, "MSN-JP", JpMessenger.Connect, 0);
        env.Game.Context.Kr.Messenger = msn.Context;
        using var cts = new CancellationTokenSource();
        _ = msn.Tcp.StartAsync(cts.Token);
        await Task.Yield();
        var players = env.Game.Context.Kr.Players;
        var (ga, accA) = await InChannelAsync(env);                                     // A: mestre
        await using var _a = ga;
        var (gc, accC) = await InChannelAsync(env);                                     // C: convidado
        await using var _c = gc;
        var (gd, accD) = await InChannelAsync(env);                                     // D: pede para entrar
        await using var _d = gd;
        var guilds = new GuildService(env.S.Guilds, env.S.GuildInvites);
        var (code, gid) = await guilds.CreateWebAsync(accA.Id, "G" + Guid.NewGuid().ToString("N")[..8], "");
        Assert.Equal(GuildCode.Ok, code);
        await ga.SendAsync(new PacketWriter(0xA2));                                     // A volta do site: é o mestre
        var hA = (GameHandler)env.Game.World.Find(accA.Id)!;
        for (int i = 0; i < 50 && hA.Player.Guild == null; i++) await Task.Delay(20);

        // pedido de D -> A vê na lista (tipo 2) e aceita
        Assert.Equal(GuildCode.Ok, await guilds.RequestJoinAsync(accD.Id, gid, "oi"));
        await ga.SendAsync(new PacketWriter(0xA5));
        var pend = await ga.ExpectAsync(0x116);
        Assert.Equal(1, pend.U16());
        Assert.Equal(((uint)accD.Id, (uint)gid), (pend.U32(), pend.U32()));
        pend.Skip(44 + 1);
        Assert.Equal(2, pend.U16());
        await ga.SendAsync(Jp983Client.Build(0xA7, "FUN_008fb120", (uint)accD.Id, (uint)gid, (byte)1));
        var ans = await ga.ExpectAsync(0x117);
        Assert.Equal((1u, (uint)gid), (ans.U32(), ans.U32()));
        var hD = (GameHandler)env.Game.World.Find(accD.Id)!;
        Assert.Equal(gid, hD.Player.Guild?.Id);

        // convite de A para C pelo mensageiro: 0x126 ok; C recebe a lista no jogo (0x118) e aceita (0xA8 -> 0x119)
        string nickA = (await players.LoadAsync(accA.Id))!.Nickname;
        await using var ma = await JpMsnConnectAsync(msn);
        await ma.SendAsync(new PacketWriter(0x12).U32((uint)accA.Id).Str(nickA));
        await ma.ExpectAsync(0x2F);
        await ma.SendAsync(new PacketWriter(0x26).Str(accA.Login).Str("nao-existe").U32((uint)gid).U8(1));
        Assert.Equal(3u, (await JpSub(ma, 0x126)).U32());                                // login inexistente
        await ma.SendAsync(Jp983Client.Build(0x26, "FUN_008f9fa0", accA.Login, accC.Login, (uint)gid, (byte)1));
        Assert.Equal(1u, (await JpSub(ma, 0x126)).U32());
        await ma.SendAsync(new PacketWriter(0x26).Str(accA.Login).Str(accC.Login).U32((uint)gid).U8(1));
        Assert.Equal(6u, (await JpSub(ma, 0x126)).U32());                                // já convidado
        var inv = await gc.ExpectAsync(0x118);
        Assert.Equal(1, inv.U16());
        uint inviteId = inv.U32();
        Assert.Equal((uint)gid, inv.U32());
        await ga.SendAsync(new PacketWriter(0xA5));                                     // o convite aparece como tipo 3
        var pend2 = await ga.ExpectAsync(0x116);
        Assert.Equal(1, pend2.U16());
        pend2.Skip(8 + 44 + 1);
        Assert.Equal(3, pend2.U16());

        await gc.SendAsync(Jp983Client.Build(0xA8, "FUN_00901b60", (uint)gid, inviteId, (byte)1));
        var joined = await gc.ExpectAsync(0x119);
        Assert.Equal((1u, (byte)1, (uint)gid), (joined.U32(), joined.U8(), joined.U32()));
        var hC = (GameHandler)env.Game.World.Find(accC.Id)!;
        Assert.Equal((gid, GuildClass.Member), (hC.Player.Guild!.Id, hC.Player.Guild.Class));
        Assert.Empty(await env.S.GuildInvites.OfAccountAsync(accC.Id));
        cts.Cancel();
    }
}
