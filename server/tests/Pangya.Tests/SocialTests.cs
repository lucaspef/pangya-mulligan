using Pangya.Core.Net;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Pacotes pequenos de sala e de itens (GameHandler.Social.cs).</summary>
[Collection("db")]
public class SocialTests(DbFixture fx)
{
    static async Task<(TestClient C, long Id)> EnterAsync(GameEnv env)
    {
        var (acc, key) = await env.NewPlayerAsync();
        var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        await c.ExpectAsync(0x94);
        return (c, acc.Id);
    }

    static PacketWriter MakeRoom(byte mode = 0) =>
        new PacketWriter(0x08).U8(0).U32(60000).U32(0).U8(4).U8(mode).U8(3).U8(0).U8(0).Str("t").Str("");

    [Fact]
    public async Task HeadIconTeamChatDetailAndBanish()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (a, aId) = await EnterAsync(env);
        var (b, bId) = await EnterAsync(env);
        await using var _a = a;
        await using var _b = b;
        await a.SendAsync(MakeRoom());
        var enter = await a.ExpectAsync(0x47);
        enter.U8(); enter.U8();
        ushort index = enter.Struct<Pangya.Protocol.KR645.sRoomInfo>().roomGuid;
        await b.SendAsync(new PacketWriter(0x09).U16(index).Str(""));
        await b.ExpectAsync(0x47);
        await a.ExpectAsync(0x46);                                          // B entrou (vaga para A)

        await a.SendAsync(new PacketWriter(0x18).U16(3));                    // ícone: só os outros recebem
        var icon = await b.ExpectAsync(0x5B);
        Assert.Equal(((uint)aId, (ushort)3), (icon.U32(), icon.U16()));

        await a.SendAsync(new PacketWriter(0x54).Str("vamos"));              // equipe de A (inclui o próprio A)
        var tc = await a.ExpectAsync(0xAE);
        tc.Str();
        Assert.Equal("vamos", tc.Str());

        await a.SendAsync(new PacketWriter(0x2D).U16(index));               // detalhe da sala
        var det = await a.ExpectAsync(0x84);
        Assert.Equal(2, det.U8());
        await a.SendAsync(new PacketWriter(0x2D).U16(999));
        Assert.Equal(0, (await a.ExpectAsync(0x84)).U8());                  // não existe

        await b.SendAsync(new PacketWriter(0x26).U32((uint)aId));           // quem não é dono não expulsa
        await a.SendAsync(new PacketWriter(0x26).U32((uint)bId));           // o dono expulsa B
        await b.ExpectAsync(0x4A);
        var gone = await a.ExpectAsync(0x46);
        Assert.Equal(2, gone.U8());                                          // vaga removida
        Assert.Single(env.Game.World.Rooms.Get(index)!.Players);
    }

    [Fact]
    public async Task DeleteItemRulesAndSmallReplies()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var shop = new ShopService(env.Players.Store, env.Data);
        await shop.GiveAsync(p, 0x18000004, 5);
        await shop.GiveAsync(p, 0x1C000010, 1);                              // caddie (para recontratar)
        int caddie = p.FindType(0x1C000010)!.Id;
        await using var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        await c.ExpectAsync(0x94);

        await c.SendAsync(new PacketWriter(0x64).U32(0x18000004).U32(2));     // 5 -> 3
        var del = await c.ExpectAsync(0xC3);                                  // "2 itens apagados"
        Assert.Equal((1, 0x18000004u, 2), (del.U8(), del.U32(), del.I32()));
        var cnt = await c.ExpectAsync(0xA5);
        Assert.Equal((1, 0x18000004u), (cnt.U8(), cnt.U32()));
        cnt.U32();
        Assert.Equal(3, cnt.U16());
        Assert.Equal(0, (await c.ExpectAsync(0xA8)).U16());
        await c.SendAsync(new PacketWriter(0x64).U32(0x18000004).U32(9));     // tudo: apaga
        Assert.Equal(1, (await c.ExpectAsync(0xA5)).U8());
        await c.ExpectAsync(0xA8);
        await c.SendAsync(new PacketWriter(0x64).U32(Item.BasicBall).U32(1)); // bola básica: recusado
        Assert.Equal(0, (await c.ExpectAsync(0xC3)).U8());
        Assert.Equal(0, (await c.ExpectAsync(0xA8)).U16());
        var saved = (await env.Players.LoadAsync(acc.Id))!;
        Assert.Null(saved.FindType(0x18000004));
        Assert.NotNull(saved.FindType(Item.BasicBall));

        await c.SendAsync(new PacketWriter(0x38).Str("novonick"));            // troca de nick: suspensa
        Assert.Equal(7u, (await c.ExpectAsync(0x4E)).U32());
        await c.SendAsync(new PacketWriter(0x39).U32((uint)caddie));          // recontratar: contrato ainda vale
        Assert.Equal(1, (await c.ExpectAsync(0x91)).U8());
    }

    [Fact]
    public async Task GiveUpSoloEndsTheGameWithoutReward()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        long pang = (await env.Players.LoadAsync(id))!.Pang;
        await c.SendAsync(MakeRoom());
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        await c.SendAsync(new PacketWriter(0x37));
        var end = await c.ExpectAsync(0x64);                                 // placar
        Assert.Equal(1, end.U8());
        end.U32(); end.U8(); end.U8(); end.U8();
        Assert.Equal(0, end.I16());                                          // sem EXP
        await Task.Delay(300);
        Assert.Equal(pang, (await env.Players.LoadAsync(id))!.Pang);
    }

    static PacketWriter HoleStats() =>
        new PacketWriter(0x31).Struct(new Pangya.Protocol.KR645.sPangYaUserStatistics { dwHole = 3, dwDrive = 4, dwOB = 2 });

    const int CountedBall = 0x14000001;

    async Task<GameHandler> GiveBallAsync(GameEnv env, long id, int qty)
    {
        var h = (GameHandler)env.Game.World.Find(id)!;
        await env.Game.Context.Shop.GiveAsync(h.Player, CountedBall, qty);
        h.Player.Equip.BallTypeId = CountedBall;
        return h;
    }

    [Theory]
    [InlineData(1, 0)]                                                     // 1 unidade paga o que jogou e o item sai
    [InlineData(5, 4)]
    public async Task LeavingChargesTheBallForCompletedHoles(int qty, int left)
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        var h = await GiveBallAsync(env, id, qty);
        await c.SendAsync(MakeRoom());
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        var g = (StrokeGame)h.CurrentRoom!.Game!;
        lock (env.Game.World.Rooms.Sync)
        {
            var p = g.Players[0];
            g.HoleData(g.Hole, new HoleInfo(4, 0, 0, 0, 320));
            g.Loaded(p);
            Assert.True(g.Shoot(p));
            Assert.True(g.Result(new ShotResult(p.Guid, 0, 0, 320, 4, 10, 0)));
        }
        await c.SendAsync(new PacketWriter(0x0F));
        var count = await c.ExpectAsync(0xA5);
        Assert.Equal((1, (uint)CountedBall), (count.U8(), count.U32()));
        count.U32();
        Assert.Equal(left, count.U16());
        var saved = (await env.Players.LoadAsync(id))!;
        Assert.Equal(left, saved.FindType(CountedBall)?.Quantity ?? 0);
        Assert.Equal(left == 0 ? Item.BasicBall : CountedBall, h.Player.Equip.BallTypeId);
        if (left == 0) Assert.Equal(Item.BasicBall, saved.Equip.BallTypeId);
    }

    [Fact]
    public async Task PracticeAlsoChargesTheBall()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        var h = await GiveBallAsync(env, id, 10);
        await c.SendAsync(new PacketWriter(0x08).U8(0).U32(60000).U32(0).U8(4).U8(0).U8(3).U8(0).U8(0).Str("t").Str("").U8(1).U8(0));
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        ((MassGame)h.CurrentRoom!.Game!).Find((uint)id)!.HolesDone = 2;
        await c.SendAsync(new PacketWriter(0x0F));
        await c.ExpectAsync(0xA5);
        Assert.Equal(8, (await env.Players.LoadAsync(id))!.FindType(CountedBall)!.Quantity);
    }

    [Fact]
    public async Task BallRuleKeepsFreeBallsAndFallsBack()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, _) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var actions = new PlayerActions(env.Players.Store, env.Data);
        var shop = new ShopService(env.Players.Store, env.Data);
        var now = DateTime.UtcNow;
        await shop.GiveAsync(p, CountedBall, 1);
        p.Equip.BallTypeId = CountedBall;

        Assert.Null(await actions.SpendBallAsync(p, CountedBall, 0, now));              // nenhum buraco concluído
        Assert.Null(await actions.SpendBallAsync(p, Item.BasicBall, 18, now));
        Assert.Null(await actions.SpendBallAsync(p, Premium.Comet, 18, now));
        Assert.Null(await actions.SpendBallAsync(p, Premium.VipComet, 18, now));
        Assert.Equal(100, p.FindType(Item.BasicBall)!.Quantity);

        var one = await actions.SpendBallAsync(p, CountedBall, 3, now);                 // 1 unidade, 3 buracos: piso 0
        Assert.Equal((1, 0), (one!.Value.Before, one.Value.After));
        Assert.Null(p.FindType(CountedBall));
        Assert.Equal(Item.BasicBall, p.Equip.BallTypeId);

        await shop.GiveAsync(p, CountedBall, 5);                                        // 5 unidades, 18 buracos
        p.Equip.BallTypeId = CountedBall;
        p.Add(new Item { Id = 999_999_001, TypeId = Premium.NormalTicket, ExpiresAt = now.AddDays(1) });   // premium: volta para a dele
        Assert.Equal(0, (await actions.SpendBallAsync(p, CountedBall, 18, now))!.Value.After);
        Assert.Equal(Premium.Comet, p.Equip.BallTypeId);
    }

    [Fact]
    public void PremiumBallTakesThePlaceOfTheDefault()
    {
        var now = DateTime.UtcNow;
        var p = new Player();
        Assert.Equal((Item.BasicBall, Premium.Comet), (PlayerActions.EquipBall(p, Item.BasicBall, now), PlayerActions.EquipBall(p, Premium.Comet, now)));
        p.Add(new Item { Id = 1, TypeId = Premium.VipTicket, ExpiresAt = now.AddDays(1) });
        Assert.Equal(Premium.VipComet, PlayerActions.EquipBall(p, Item.BasicBall, now));
        Assert.Equal(Premium.VipComet, PlayerActions.EquipBall(p, Premium.Comet, now));   // o cliente sempre pede a 0x140000D8
        Assert.Equal(CountedBall, PlayerActions.EquipBall(p, CountedBall, now));
    }

    [Fact]
    public async Task DefaultBallIsListedWithoutARow()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var ch = new PlayerChanges();
        ch.Removed.Add(p.FindType(Item.BasicBall)!.Id);
        await env.Players.Store.ApplyAsync(acc.Id, ch);
        await using var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        var items = await c.ExpectAsync(0x71);
        int n = items.U16();
        items.U16();
        var list = Enumerable.Range(0, n).Select(_ => items.Struct<Pangya.Protocol.KR645.sItemInfo>()).ToList();
        Assert.Contains(list, i => i.tid == Item.BasicBall && i.guid == unchecked((uint)Item.VirtualBallId) && i.IsValid == 1);
        Assert.Null((await env.Players.LoadAsync(acc.Id))!.FindType(Item.BasicBall));   // nada no banco
    }

    [Fact]
    public async Task QuittingMidGameCountsOnceWithoutTheMatchRecords()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        long pang = (await env.Players.LoadAsync(id))!.Pang;
        await c.SendAsync(MakeRoom());
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        await c.SendAsync(HoleStats());
        await c.SendAsync(new PacketWriter(0x0F));
        var st = (await c.ExpectAsync(0x43)).Struct<Pangya.Protocol.KR645.sPangYaUserStatistics>();
        Assert.Equal((1u, 1u), (st.dwNoMannerGameCount, st.dwGameCount));
        var s = (await env.Players.LoadAsync(id))!.Stats;
        Assert.Equal((1L, 1L, 0L, 0L, 0L, 0L), (s.Quits, s.Games, s.Hole, s.TotalScore, s.Drive, s.OB));   // "todos os registros anulados"
        Assert.Equal(pang, (await env.Players.LoadAsync(id))!.Pang);        // Stroke sozinho: a janela não mostra pena
        await c.SendAsync(new PacketWriter(0x0F));
        await Task.Delay(100);
        Assert.Equal(1L, (await env.Players.LoadAsync(id))!.Stats.Quits);
    }

    [Fact]
    public async Task QuitKeepsOnlyTheCountersAndStopsThePangAtZero()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, _) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        p.Pang = 150;
        p.Stats = new PlayerStats { Games = 3, Drive = 7 };
        await Rewards.QuitAsync(env.Players.Store, p, 400);
        var s = (await env.Players.LoadAsync(acc.Id))!;
        Assert.Equal((4L, 1L, 0L), (s.Stats.Games, s.Stats.Quits, s.Pang));
    }

    [Fact]
    public async Task DisconnectingMidGameIsNotAQuit()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        long pang = (await env.Players.LoadAsync(id))!.Pang;
        await using (c)
        {
            await c.SendAsync(MakeRoom());
            await c.ExpectAsync(0x46);
            await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
            await c.ExpectAsync(0x50);
        }
        for (int i = 0; i < 100 && env.Game.World.Find(id) != null; i++) await Task.Delay(20);
        Assert.Null(env.Game.World.Find(id));
        await Task.Delay(100);
        var p = (await env.Players.LoadAsync(id))!;
        Assert.Equal((0L, 0L, pang), (p.Stats.Quits, p.Stats.Games, p.Pang));
    }

    [Fact]
    public async Task PracticeRoomDoesNotCountQuit()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        await c.SendAsync(new PacketWriter(0x08).U8(0).U32(60000).U32(0).U8(4).U8(0).U8(3).U8(0).U8(0).Str("t").Str("").U8(1).U8(0));
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        await c.SendAsync(HoleStats());
        await c.SendAsync(new PacketWriter(0x0F));
        await c.ExpectAsync(0x4A);
        await Task.Delay(100);
        var s = (await env.Players.LoadAsync(id))!.Stats;
        Assert.Equal((0L, 0L, 0L), (s.Quits, s.Games, s.Drive));
    }

    [Fact]
    public async Task GiveUpThenLeaveCountsOnce()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, id) = await EnterAsync(env);
        await using var _c = c;
        await c.SendAsync(MakeRoom());
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)id));
        await c.ExpectAsync(0x50);
        await c.SendAsync(HoleStats());
        await c.SendAsync(new PacketWriter(0x37));
        await c.ExpectAsync(0x64);
        await c.ExpectAsync(0x43);
        await c.SendAsync(new PacketWriter(0x0F));
        await c.ExpectAsync(0x4A);
        await Task.Delay(100);
        var s = (await env.Players.LoadAsync(id))!.Stats;
        Assert.Equal((1L, 1L, 0L), (s.Quits, s.Games, s.Drive));
    }

    [Fact]
    public async Task WhisperInviteAndFollow()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (a, aId) = await EnterAsync(env);
        var (b, bId) = await EnterAsync(env);
        await using var _a = a;
        await using var _b = b;
        string bNick = (await env.Players.LoadAsync(bId))!.Nickname, aNick = (await env.Players.LoadAsync(aId))!.Nickname;

        await a.SendAsync(new PacketWriter(0x2A).Str(bNick).Str("oi b"));          // sussurro
        var echo = await a.ExpectAsync(0x82);
        Assert.Equal((0, bNick, "oi b"), (echo.U8(), echo.Str(), echo.Str()));
        var got = await b.ExpectAsync(0x82);
        Assert.Equal((1, aNick, "oi b"), (got.U8(), got.Str(), got.Str()));
        await a.SendAsync(new PacketWriter(0x2A).Str("ninguem").Str("oi"));        // não está online
        Assert.Equal(6, (await a.ExpectAsync(0x3E)).U8());

        await a.SendAsync(MakeRoom());
        var enter = await a.ExpectAsync(0x47);
        enter.U8(); enter.U8();
        ushort index = enter.Struct<Pangya.Protocol.KR645.sRoomInfo>().roomGuid;
        await a.SendAsync(new PacketWriter(0xB2).Str(bNick).U32((uint)bId));        // convite
        var inv = await a.ExpectAsync(0x127);
        Assert.Equal(0, inv.U16());
        inv.U32(); inv.U8();
        Assert.Equal((index, (uint)bId), (inv.U16(), inv.U32()));
        inv.Str();
        uint id = inv.U32();
        await a.SendAsync(new PacketWriter(0x29).U32(id));
        var invited = await b.ExpectAsync(0x81);
        Assert.Equal(0, invited.U16());
        invited.U32(); invited.U8();
        Assert.Equal((index, (uint)aId, aNick, id), (invited.U16(), invited.U32(), invited.Str(), invited.U32()));

        await b.SendAsync(new PacketWriter(0xAC).U8(0).U16(index));                // segue até a sala
        Assert.Equal(0, (await b.ExpectAsync(0x47)).U8());
        await a.SendAsync(new PacketWriter(0xB2).Str(bNick).U32((uint)bId));        // já está numa sala
        Assert.Equal(6, (await a.ExpectAsync(0x127)).U16());
        await b.SendAsync(new PacketWriter(0xB7).U8(0));                           // partida rápida já numa sala: falhou
        Assert.Equal(5, (await b.ExpectAsync(0x133)).U8());
    }
}
