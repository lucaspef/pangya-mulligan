using System.Buffers.Binary;
using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Relatório do Tiki (SPEC-jp983-tiki-report.md): regras, item, saída da partida e pacotes do JP.</summary>
public class TikiReportTests
{
    static (Room Room, TourneyGame Game, MassRecorder Out, RoomManager Mgr) Setup(int humans, byte holes = 1)
    {
        var mgr = new RoomManager();
        var room = mgr.Create(new RoomSettings { Holes = holes, Mode = GameMode.Tournament, MaxPlayers = 30 }, 1);
        for (int i = 0; i < humans; i++)
        {
            var s = new FakeSession(100 + i);
            s.Player.Level = 6;
            RoomManager.Join(room, new RoomPlayer { Guid = (uint)(100 + i), Player = s.Player, Session = s });
        }
        RoomManager.PrepareStart(room, new Random(1));
        room.HoleOrder = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
        var o = new MassRecorder();
        var g = (TourneyGame)MassGame.For(room, o, mgr.Sync, TimeSpan.FromHours(1));
        room.Game = g;
        for (byte h = 1; h <= holes; h++) g.HoleData(h, new HoleInfo(4, 0, 0, 0, 320));
        return (room, g, o, mgr);
    }

    /// <summary>O buraco atual de p em n tacadas (a última acerta), com o pang do resultado.</summary>
    static void Hole(TourneyGame g, MassPlayer p, int n, uint pang = 0, uint bonus = 0)
    {
        g.Loaded(p);
        for (int k = 1; k <= n; k++)
        {
            g.Shoot(p, 0);
            g.Result(p, new ShotResult(p.Guid, 0, 0, 320, k == n ? ShotResult.StateHoled : (byte)2, pang, bonus));
            g.ShotFinished(p);
        }
    }

    static Item Scroll(int id = 9001) => new() { Id = id, TypeId = TikiReport.ScrollTypeId, Quantity = 2 };

    [Fact]
    public void FinishedTourneyPlayerDoesNotQuit()
    {
        var (_, g, _, _) = Setup(2);
        var (done, playing) = (g.Players[0], g.Players[1]);
        Hole(g, done, 3);
        Assert.Equal(0, g.QuitHoles(done.Guid));                           // terminou e espera o resto
        Assert.Equal(1, g.QuitHoles(playing.Guid));
    }

    [Fact]
    public void OnlyAFinishedTourneyPlayerWithTheScrollCanUseIt()
    {
        var (room, g, _, _) = Setup(2);
        var me = g.Players[0];
        var p = me.RoomPlayer.Player;
        Assert.Equal("não terminou", TikiReport.Refusal(p, room, g, me));
        Hole(g, me, 3);
        Assert.True(me.Finished && !g.Over);
        Assert.Equal("sem pergaminho", TikiReport.Refusal(p, room, g, me));
        p.Add(Scroll());
        Assert.Null(TikiReport.Refusal(p, room, g, me));
        p.Level = 5;
        Assert.Equal("nível 5", TikiReport.Refusal(p, room, g, me));
        p.Level = 6;
        var gm = new Player { Level = 30, IdentityFlags = 4 };
        gm.Add(Scroll());
        Assert.Equal("GM", TikiReport.Refusal(gm, room, g, me));
        for (int i = 0; i < TikiReport.MaxReports; i++) p.Add(new Item { Id = 9100 + i, TypeId = TikiReport.ReportTypeId });
        Assert.Equal("já tem 10 relatórios", TikiReport.Refusal(p, room, g, me));
        for (int i = 0; i < TikiReport.MaxReports; i++) p.Items.Remove(9100 + i);
        room.Settings.Practice = true;
        Assert.Equal("modo sem relatório", TikiReport.Refusal(p, room, g, me));
    }

    [Fact]
    public void ReporterStaysFinishedForTheOthersAndGetsTheFinalRows()
    {
        var (room, g, o, mgr) = Setup(3);
        var (me, b, c) = (g.Players[0], g.Players[1], g.Players[2]);
        Hole(g, me, 3, 300, 20);
        me.Reported = true;
        lock (mgr.Sync) mgr.Leave(room, me.RoomPlayer);
        Assert.DoesNotContain($"state {me.Guid} 3", o.Events);                  // ninguém o vê sair: segue "terminou"
        var now = DateTime.UtcNow;
        var snap = TikiReport.Rows(g, null, now);
        Assert.Equal(3, snap.Count);
        Assert.Equal((-1, 300L, 20L, false), (snap[0].Score, snap[0].Pang, snap[0].Bonus, snap[0].Left));
        Assert.Equal(0, snap[1].Score);                                           // ainda jogando: como está

        lock (mgr.Sync) mgr.Leave(room, c.RoomPlayer);                            // saída normal: estado 3
        Assert.Contains($"state {c.Guid} 3", o.Events);
        Hole(g, b, 5);
        Assert.True(g.Over);
        var final = TikiReport.Rows(g, o.Result, now);
        Assert.Equal((false, 1, true), (final[0].Left, final[1].Score, final[2].Left));
        Assert.Equal("p101", final[1].Nick);
    }

    [Fact]
    public void ReportItemKeepsTheDataAndChecksParamAndDate()
    {
        var now = new DateTime(2026, 10, 30, 12, 0, 0, DateTimeKind.Utc);
        var rows = new List<TikiReport.Row>
        {
            new(100, "eu", 6, -2, 350, 40, 0x40000001, true, 7, "mark", 0, 3, false),
            new(101, "outro", 9, 1, 120, 0, 0, false, 0, "", 2, 0, true),
        };
        var it = TikiReport.NewItem(55, new TikiReport.Data(now, 123, 0x2C060000, 4, rows), now, new Random(3));
        Assert.Equal((TikiReport.ReportTypeId, 1, now.AddDays(7)), (it.TypeId, it.Quantity, it.ExpiresAt!.Value));
        Assert.Equal(0, it.Int("c1"));
        Assert.InRange(it.Int("c2"), 1, 0x7FFF);
        uint param = (uint)TikiReport.Param(it);
        Assert.Equal((uint)it.Int("c2"), param);

        var back = new Item { Id = it.Id, TypeId = it.TypeId, ExpiresAt = it.ExpiresAt,               // como volta do banco (jsonb)
            Attrs = System.Text.Json.Nodes.JsonNode.Parse(it.Attrs.ToJsonString())!.AsObject() };
        var d = TikiReport.Read(back)!;
        Assert.Equal((now, 123, 0x2C060000, 4), (d.Date, d.Exp, d.MatchTid, d.Mode));
        Assert.Equal(rows, d.Rows);

        Assert.Equal(TikiReport.Ok, TikiReport.Check(back, param, now.AddDays(6)));
        Assert.Equal(TikiReport.BadData, TikiReport.Check(back, param + 1, now));
        Assert.Equal(TikiReport.BadDate, TikiReport.Check(back, param, now.AddDays(7)));
        Assert.Equal(TikiReport.Fail, TikiReport.Check(null, param, now));
        Assert.Equal(TikiReport.Fail, TikiReport.Check(Scroll(), param, now));
        back.Attrs.Remove("rows");
        Assert.Equal(TikiReport.BadData, TikiReport.Check(back, param, now));

        var p = new Player();
        p.Add(it);
        Assert.Empty(TikiReport.Stale(p, now));
        Assert.Single(TikiReport.Stale(p, now.AddDays(8)));
        p.Add(new Item { Id = 56, TypeId = TikiReport.ReportTypeId, Quantity = 10 });     // do give-all: sem prazo nem dados
        Assert.Equal([56], TikiReport.Stale(p, now).Select(x => x.Id));
    }

    [Fact]
    public void JpPacketsFollowTheClientLayouts()
    {
        var now = DateTime.UtcNow;
        var rows = new List<TikiReport.Row>
        {
            new(100, "eu", 6, -2, 350, 40, 0x40000001, true, 7, "mark", 0, 3, false),
            new(101, "outro", 9, 1, 120, 5, 0, false, 0, "", 2, 0, true),
        };
        var d = new TikiReport.Data(now, 123, 0x2C060000, 4, rows);
        var it = TikiReport.NewItem(55, d, now, new Random(3));

        using var w = JpSystems.ReportResult(d, 100);
        var b = w.Body[2..].ToArray();
        Assert.Equal(0x11A, w.Id);
        Assert.Equal(4 + 16 + 2 * 0x83, b.Length);
        Assert.Equal(2, BitConverter.ToInt32(b, 0));
        var me = b.AsSpan(20, 0x83);
        var other = b.AsSpan(20 + 0x83, 0x83);
        Assert.Equal((100u, 350L, 40L), (BitConverter.ToUInt32(me), BitConverter.ToInt64(me[4..]), BitConverter.ToInt64(me[0xC..])));
        Assert.Equal((0x2C060000u, 123u, 0x40000001u), (BitConverter.ToUInt32(me[0x14..]), BitConverter.ToUInt32(me[0x18..]), BitConverter.ToUInt32(me[0x1C..])));
        Assert.Equal((1, 6, (byte)0xFE, 3), (me[0x20], BitConverter.ToUInt16(me[0x22..]), me[0x26], me[0x27]));
        Assert.Equal("eu", Cp949.Read(me.Slice(0x3F, 22)));
        Assert.Equal((7u, "mark", 4, 0, 2), (BitConverter.ToUInt32(me[0x59..]), Cp949.Read(me.Slice(0x61, 12)), (int)me[0x6D], (int)me[0x71], (int)me[0x72]));
        Assert.Equal((0u, 2, 4), (BitConverter.ToUInt32(other[0x18..]), (int)other[0x28], (int)other[0x71]));   // EXP só na minha; saiu = 4

        using var item = JpSystems.ReportItem(it);
        var k = item.Body[2..].ToArray();
        Assert.Equal(0xAA, item.Id);
        Assert.Equal(2 + 38 + 4 + 16, k.Length);
        Assert.Equal((1, (uint)TikiReport.ReportTypeId, 55), (BitConverter.ToUInt16(k), BitConverter.ToUInt32(k, 2), BitConverter.ToInt32(k, 6)));
        Assert.InRange(BitConverter.ToUInt16(k, 10), 167, 168);
        Assert.Equal((2, 1), (k[12], BitConverter.ToUInt16(k, 13)));
        Assert.Equal(((ushort)0, (ushort)it.Int("c2")), (BitConverter.ToUInt16(k, 40), BitConverter.ToUInt16(k, 42)));
        Assert.Equal(it.ExpiresAt!.Value.ToLocalTime().Year, BitConverter.ToUInt16(k, 44));
    }

    [Fact]
    public void InventoryCarriesParamHoursAndDateForTheJpClient()
    {
        var now = DateTime.UtcNow;
        var it = TikiReport.NewItem(77, new TikiReport.Data(now, 1, 0, 4, [new(1, "a", 6, 0, 0, 0, 0, false, 0, "", 0, 0, false)]),
            now, new Random(5));
        var s = PlayerStructs.ItemInfo(it);
        Assert.Equal((1, 0, it.Int("c2")), ((int)s.Common[0], (int)s.Common[1], (int)s.Common[2]));
        Assert.InRange(s.HourRemain, 167, 168);
        Assert.Equal(2u, s.ItemType);
        var k = System.Runtime.InteropServices.MemoryMarshal.AsBytes(new ReadOnlySpan<Pangya.Protocol.KR645.sItemInfo>(in s)).ToArray();
        using var w = new PacketWriter(0x73).Item(k);
        var j = w.Body[2..].ToArray();
        Assert.Equal(JpStructs.ItemSize, j.Length);
        long secs = BinaryPrimitives.ReadInt64LittleEndian(j.AsSpan(24));
        Assert.InRange(secs, new DateTimeOffset(it.ExpiresAt!.Value).ToUnixTimeSeconds() - 1, new DateTimeOffset(it.ExpiresAt.Value).ToUnixTimeSeconds());
    }
}

/// <summary>Relatório sem dados (o give-all de 07/10 deu um à conta do dono): ninguém entrega e o login apaga.</summary>
[Collection("db")]
public class TikiReportLoginTests(DbFixture fx)
{
    [Fact]
    public async Task ReportWithoutDataIsNotGivenAndLeavesOnLogin()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var shop = new Pangya.Domain.Shop.ShopService(env.Players.Store, env.Data);
        Assert.Equal(Pangya.Domain.Shop.ShopCode.NotForSale, (await shop.GiveAsync(p, TikiReport.ReportTypeId, 10)).Code);

        var ch = new PlayerChanges();
        ch.Added.Add(new Item { Id = (await env.Players.Store.NewIdsAsync(1))[0], TypeId = TikiReport.ReportTypeId, Quantity = 10 });
        await env.Players.Store.ApplyAsync(acc.Id, ch);
        var c = await env.ConnectAsync();
        await using var _c = c;
        await GameEnv.SendLoginAsync(c, acc, key);
        while ((await c.ReceiveAsync()).Item1 != 0x70) { }                    // a limpeza vem antes do inventário
        Assert.DoesNotContain((await env.Players.LoadAsync(acc.Id))!.Items.Values, x => x.TypeId == TikiReport.ReportTypeId);
    }
}
