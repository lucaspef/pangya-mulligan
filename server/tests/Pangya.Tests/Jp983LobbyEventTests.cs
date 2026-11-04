using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Tests;

/// <summary>Bingo (0xE3 -> 0x18B/0x18C) e Mission (0xE1/0xE2 -> 0x189/0x18A) do lobby, com o banco.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task BingoOpensDrawsAndSavesProgress()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));

        await g.SendAsync(new PacketWriter(0xE3).U32(0));
        var open = await g.ExpectAsync(0x18B);
        Assert.Equal((0u, 0u, 3, 0u, 0u, 0u), (open.U32(), open.U32(), open.I32(), open.U32(), open.U32(), open.U32()));

        await g.SendAsync(new PacketWriter(0xE3).U32(1));
        var draw = await g.ExpectAsync(0x18B);
        Assert.Equal((0u, 1u, 2), (draw.U32(), draw.U32(), draw.I32()));
        uint aimed = draw.U32();
        draw.U32();
        uint number = draw.U32();
        Assert.Equal(1u << (int)(number - 1), aimed);
        Assert.Equal(new BingoProgress(2, (int)aimed, 0), await env.S.Bingo.GetAsync(acc.Id));
    }

    [Fact]
    public async Task MissionSendsSevenMissionsAndRefusesEarlyGift()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var today = DateOnly.FromDateTime(DateTime.Now);
        await env.S.Mission.SaveAsync(acc.Id, new MissionProgress(today, 3, 1, false, 5, 1, 0, 0, 0x0003, false));
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));

        await g.SendAsync(new PacketWriter(0xE1));
        var list = await g.ExpectAsync(0x189);
        Assert.Equal(7u, list.U32());
        Assert.Equal((3, (byte)1, 1, (byte)0), (list.I32(), list.U8(), list.I32(), list.U8()));
        for (int i = 2; i < 6; i++) { list.I32(); list.U8(); }
        Assert.Equal((3, (byte)0, (byte)0, (byte)0), (list.I32(), list.U8(), list.U8(), list.U8()));

        await g.SendAsync(new PacketWriter(0xE2).U32(1));                          // falta 1 torneio
        Assert.Equal((uint)MissionEventService.GiftFailed, (await g.ExpectAsync(0x18A)).U32());
    }
}
