using Pangya.Core.Config;
using Pangya.Core.Net;
using Pangya.Messenger;
using Pangya.Protocol.JP983.Messenger;

namespace Pangya.Tests;

/// <summary>Mensageiro do JP 983 (SPEC-jp983-correio-social.md §4): hello, login, lista e amizade no layout do JP.</summary>
public partial class Jp983GameTests
{
    static async Task<TestClient> JpMsnConnectAsync(MessengerServer msn)
    {
        var m = await TestClient.ConnectAsync(msn.Tcp.Port);
        var (id, hello) = await m.ReceiveAsync();
        Assert.Equal(0x2E, id);                                                       // hello do JP
        Assert.Equal((1, 1), (hello.U8(), hello.U8()));
        m.Key = (int)hello.U32();
        return m;
    }

    [Fact]
    public async Task JpMessengerTalksTheJpLayout()
    {
        await using var env = new JpEnv();
        var msn = new MessengerServer(env.S, env.Game.World, new MessengerConfig { Id = 39304, Port = 0 }, JpMessenger.ServerType,
            39302, "MSN-JP", JpMessenger.Connect, 0);
        env.Game.Context.Kr.Messenger = msn.Context;
        using var cts = new CancellationTokenSource();
        _ = msn.Tcp.StartAsync(cts.Token);
        await Task.Yield();
        var players = env.Game.Context.Kr.Players;
        var (ga, accA) = await InChannelAsync(env);
        await using var _a = ga;
        var (gb, accB) = await InChannelAsync(env);
        await using var _b = gb;
        string nickA = (await players.LoadAsync(accA.Id))!.Nickname, nickB = (await players.LoadAsync(accB.Id))!.Nickname;

        await using var bad = await JpMsnConnectAsync(msn);                            // nick errado: recusa com 1 (KR: 2)
        await bad.SendAsync(new PacketWriter(0x12).U32((uint)accA.Id).Str("outro"));
        Assert.Equal(1, (await bad.ExpectAsync(0x2F)).U8());

        async Task<TestClient> LoginAsync(long uid, string nick)
        {
            var m = await JpMsnConnectAsync(msn);
            await m.SendAsync(new PacketWriter(0x12).U32((uint)uid).Str(nick));
            var r = await m.ExpectAsync(0x2F);
            Assert.Equal((0, (uint)uid), (r.U8(), r.U32()));
            await m.SendAsync(new PacketWriter(0x23).Zeros(0x4B));
            await m.SendAsync(new PacketWriter(0x14));
            var list = await JpSub(m, 0x102);
            Assert.Equal((1, (ushort)0, (ushort)0), (list.U8(), list.U16(), list.U16()));
            return m;
        }
        await using var ma = await LoginAsync(accA.Id, nickA);
        await using var mb = await LoginAsync(accB.Id, nickB);

        await ma.SendAsync(new PacketWriter(0x18).U32((uint)accB.Id).Str(nickB));    // pedir amizade
        var asked = await JpSub(ma, 0x104);
        Assert.Equal((0u, 4 + JpMessenger.FriendSize), (asked.U32(), asked.Remaining + 4));
        Assert.Equal(nickB, Pangya.Core.Text.Cp949.Read(asked.Bytes(22)));
        var incoming = await JpSub(mb, 0x106);
        Assert.Equal(JpMessenger.FriendSize, incoming.Remaining);
        Assert.Equal(nickA, Pangya.Core.Text.Cp949.Read(incoming.Bytes(22)));
        cts.Cancel();
    }

    /// <summary>Próximo 0x30 (sub-pacote do mensageiro JP) com o sub-id pedido.</summary>
    static async Task<PacketReader> JpSub(TestClient c, ushort sub)
    {
        while (true)
        {
            var r = await c.ExpectAsync(0x30);
            if (r.U16() == sub) return r;
        }
    }
}
