using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Relatório do Tiki pelo cliente JP (SPEC-jp983-tiki-report.md): usar o pergaminho no fim do torneio e abrir o relatório.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task TikiReportScrollLeavesWithPangAndTheReportGivesExpWhenOpened()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var ch = new PlayerChanges { Level = TikiReport.MinLevel, Pang = 1000 };
        ch.Added.Add(new Item { Id = (await env.S.Players.NewIdsAsync(1))[0], TypeId = TikiReport.ScrollTypeId, Quantity = 2 });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, await env.S.Sessions.IssueGameLoginAsync(acc.Id)));
        await g.ExpectAsync(0x96);
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        await g.ExpectAsync(0x4E);
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)30, (byte)4, (byte)3, (byte)0, (byte)0,
            0u, "torneio", "", 0u));
        await g.ExpectAsync(0x49);
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        await g.ExpectAsync(0x52);
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        var tg = (TourneyGame)room.Game!;
        lock (env.Game.World.Rooms.Sync)                                      // terminou os buracos (o placar vem da partida)
        {
            var me = tg.Players[0];
            (me.Pang, me.Bonus, me.Score, me.Finished) = (100, 10, -1, true);
        }
        long pang0 = h.Player.Pang;
        var (level0, exp0) = (h.Player.Level, h.Player.Exp);

        // usar o pergaminho: sai da sala (0x4C), relatório novo (0xAA) e o pang da partida agora
        await g.SendAsync(new PacketWriter(0xAA).Zeros(0x4F).I64(110).Zeros(0x109 - 0x57));
        Assert.Equal(0xFFFF, (await g.ExpectAsync(0x4C)).U16());
        var got = await g.ExpectAsync(0xAA);
        Assert.Equal((1, (uint)TikiReport.ReportTypeId), (got.U16(), got.U32()));
        int id = got.I32();
        got.Skip(2 + 1 + 2 + 16 + 9);
        ushort c1 = got.U16(), c2 = got.U16();
        var report = h.Player.Find(id)!;
        Assert.Equal(((int)c1, (int)c2), (report.Int("c1"), report.Int("c2")));
        Assert.InRange(report.ExpiresAt!.Value, DateTime.UtcNow.AddDays(6.9), DateTime.UtcNow.AddDays(7));
        Assert.Equal(1, h.Player.FindType(TikiReport.ScrollTypeId)!.Quantity);
        Assert.Empty(env.Game.World.Rooms.Rooms);                            // era o único humano
        for (int i = 0; i < 50 && h.Player.Pang == pang0; i++) await Task.Delay(50);
        Assert.Equal(pang0 + 110, h.Player.Pang);
        Assert.Equal((level0, exp0), (h.Player.Level, h.Player.Exp));      // EXP só ao abrir
        int exp = report.Int("exp");
        Assert.True(exp > 0);

        // fora da sala o pergaminho não vale: só a saída (tira a espera), sem relatório nem gasto
        await g.SendAsync(new PacketWriter(0xAA).Zeros(0x109));
        Assert.Equal(0xFFFF, (await g.ExpectAsync(0x4C)).U16());
        Assert.Equal(1, h.Player.FindType(TikiReport.ScrollTypeId)!.Quantity);

        // abrir com o parâmetro errado: -2; certo: o resultado e o EXP; de novo: 0 (não existe mais)
        await g.SendAsync(new PacketWriter(0xAB).I32(id).U32((uint)(c1 * 0x8000 + c2 + 1)));
        Assert.Equal(TikiReport.BadData, (await g.ExpectAsync(0x11A)).I32());
        await g.SendAsync(new PacketWriter(0xAB).I32(id).U32((uint)(c1 * 0x8000 + c2)));
        var open = await g.ExpectAsync(0x11A);
        Assert.Equal(1, open.I32());
        open.Skip(16);
        Assert.Equal((uint)acc.Id, open.U32());
        Assert.Equal((100L, 10L), (open.I64(), open.I64()));
        open.U32();
        Assert.Equal((uint)exp, open.U32());
        Assert.Null(h.Player.Find(id));
        var after = new Player { Level = level0, Exp = exp0 };
        Levels.AddExp(after, exp);
        Assert.Equal((after.Level, after.Exp), (h.Player.Level, h.Player.Exp));
        await g.ExpectAsync(0x45);                                               // EXP/nível novos na tela
        await g.SendAsync(new PacketWriter(0xAB).I32(id).U32((uint)(c1 * 0x8000 + c2)));
        Assert.Equal(TikiReport.Fail, (await g.ExpectAsync(0x11A)).I32());
        var saved = (await env.S.Players.LoadAsync(acc.Id))!;
        Assert.Null(saved.Find(id));
        Assert.Equal((after.Level, after.Exp, pang0 + 110), (saved.Level, saved.Exp, saved.Pang));
    }
}
