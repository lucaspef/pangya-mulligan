using Pangya.Core.Net;

namespace Pangya.Tests;

/// <summary>Ausente (0x32 -> 0x8C) e bloqueio de chat (0x4F -> 0xAA) chegam à sala.</summary>
[Collection("db")]
public class RoomFlagTests(DbFixture fx)
{
    [Fact]
    public async Task IdleAndChatBlockReachTheRoom()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (a, accA) = await LobbyClient.EnterAsync(env);
        var (b, _) = await LobbyClient.EnterAsync(env);
        await using var _a = a;
        await using var _b = b;
        await a.SendAsync(new PacketWriter(0x08).U8(0).U32(0).U32(0).U8(4).U8(0).U8(18).U8(0).U8(0).Str("vs").Str(""));
        var enter = await a.ExpectAsync(0x47);
        enter.U8(); enter.U8();
        ushort index = enter.Struct<Pangya.Protocol.KR645.sRoomInfo>().roomGuid;
        await b.SendAsync(new PacketWriter(0x09).U16(index).Str(""));
        await b.ExpectAsync(0x47);

        await a.SendAsync(new PacketWriter(0x32).U8(1));
        var idle = await b.ExpectAsync(0x8C);
        Assert.Equal(((uint)accA.Id, (byte)1), (idle.U32(), idle.U8()));
        await a.SendAsync(new PacketWriter(0x4F).U8(1));
        var block = await b.ExpectAsync(0xAA);
        Assert.Equal(((uint)accA.Id, (byte)1), (block.U32(), block.U8()));
    }
}
