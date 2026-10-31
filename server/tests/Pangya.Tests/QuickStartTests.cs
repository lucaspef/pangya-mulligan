using Pangya.Core.Net;
using Pangya.Protocol.KR645;

namespace Pangya.Tests;

/// <summary>Quick Start (matching): 0xB7 -> 0x132 ao dono -> 0xB8 -> 0x135 -> 0xAC entra na sala.</summary>
[Collection("db")]
public class QuickStartTests(DbFixture fx)
{
    [Fact]
    public async Task QuickStartAsksTheRoomMasterAndSendsThePlayerIn()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (a, accA) = await LobbyClient.EnterAsync(env);
        var (b, accB) = await LobbyClient.EnterAsync(env);
        await using var _a = a;
        await using var _b = b;

        await b.SendAsync(new PacketWriter(0xB7).U8(0));                          // nenhuma sala ainda
        Assert.Equal(1, (await b.ExpectAsync(0x133)).U8());

        await a.SendAsync(new PacketWriter(0x08).U8(0).U32(0).U32(0).U8(4).U8(0).U8(18).U8(0).U8(0).Str("vs").Str(""));
        var enter = await a.ExpectAsync(0x47);
        enter.U8(); enter.U8();
        ushort index = enter.Struct<sRoomInfo>().roomGuid;

        await b.SendAsync(new PacketWriter(0xB7).U8(0));
        Assert.Equal(0, (await b.ExpectAsync(0x133)).U8());                       // procurando
        var ask = await a.ExpectAsync(0x132);
        Assert.Equal(((uint)accB.Id, accB.Nickname, (byte)0), (ask.U32(), ask.Str(), ask.U8()));
        await a.SendAsync(new PacketWriter(0xB8).U32((uint)accB.Id).U8(1));
        var go = await b.ExpectAsync(0x135);
        go.U32();
        Assert.Equal(((byte)0, index), (go.U8(), go.U16()));
        await b.SendAsync(new PacketWriter(0xAC).U8(0).U16(index));
        await b.ExpectAsync(0x47);                                                // entrou na sala
        _ = accA;
    }
}
