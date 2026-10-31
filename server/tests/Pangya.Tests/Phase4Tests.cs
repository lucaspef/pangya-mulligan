using Pangya.Core.Net;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Saída falsa: registra os eventos da partida.</summary>
sealed class FakeOutput : IGameOutput
{
    public List<string> Events { get; } = [];
    public StrokeGame? Game { get; set; }
    public (byte, byte) LastWind { get; private set; }
    public void Wind(byte wind, byte direction) { Events.Add("wind"); LastWind = (wind, direction); }
    public void HoleStart(GamePlayer first) => Events.Add($"start {first.Guid}");
    public void TeeReady() => Events.Add("tee");
    public void NextTurn(GamePlayer p) => Events.Add($"turn {p.Guid}");
    public void NextHole(uint holeWinner) => Events.Add("hole");
    public GameEnd? End { get; private set; }
    public void GameEnd(GameEnd end) { End = end; Events.Add("end " + string.Join(",", end.Results.Select(r => $"{r.Guid}:{r.Rank}:{r.Score}:{r.TotalStrokes}"))); }
    public void PlayerLeft(GamePlayer p) => Events.Add($"left {p.Guid}");
    public void TimeOut(GamePlayer p) => Events.Add($"timeout {p.Guid}");
    public void AskContinue(GamePlayer p) => Events.Add($"ask {p.Guid}");
    public void Aborted() => Events.Add("aborted");
    public void BotTurn(GamePlayer bot) { Events.Add($"bot {bot.Guid}"); Game!.BotShoot(bot); }
    public string Last => Events[^1];
}

sealed class FakeSession(long id) : IGameSession
{
    public Player Player { get; } = new() { AccountId = id, Nickname = "p" + id };
    public bool Kicked { get; private set; }
    public void Kick(string reason) => Kicked = true;
}

public class StrokeGameTests
{
    static (Room, StrokeGame, FakeOutput, object) Setup(int humans, bool bot, byte holes = 2, double botDelay = 10)
    {
        var mgr = new RoomManager();
        var room = mgr.Create(new RoomSettings { Holes = holes }, 1);
        for (int i = 0; i < humans; i++)
        {
            var s = new FakeSession(100 + i);
            RoomManager.Join(room, new RoomPlayer { Guid = (uint)(100 + i), Player = s.Player, Session = s });
        }
        if (bot) RoomManager.Join(room, new RoomPlayer { Guid = 0x7F000001, Player = new Player { AccountId = 0x7F000001 } });
        RoomManager.PrepareStart(room, new Random(1));
        room.HoleOrder = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
        var o = new FakeOutput();
        var g = new StrokeGame(room, o, mgr.Sync, TimeSpan.FromMilliseconds(botDelay), TimeSpan.FromSeconds(30));
        o.Game = g;
        room.Game = g;
        return (room, g, o, mgr.Sync);
    }

    static void Load(StrokeGame g, byte hole)
    {
        g.HoleData(hole, new HoleInfo(4, 0, 0, 0, 320));
        foreach (var p in g.Players) if (!p.IsBot) g.Loaded(p);
    }

    static void Play(StrokeGame g, GamePlayer p, float z, byte state)
    {
        Assert.True(g.Shoot(p));
        Assert.True(g.Result(new ShotResult(p.Guid, 0, 0, z, state, 10, 0)));
        foreach (var h in g.Players) if (!h.IsBot) g.ShotFinished(h);
    }

    [Fact]
    public void TwoHumansFarthestFromPinPlaysAndGameEnds()
    {
        var (_, g, o, _) = Setup(2, bot: false, holes: 1);
        Load(g, 1);
        Assert.Equal("start 100", o.Last);                               // 1º buraco: ordem dos slots
        var (a, b) = (g.Players[0], g.Players[1]);
        Play(g, a, 100, 2);
        Assert.Equal("turn 101", o.Last);                                // b ainda no tee
        Play(g, b, 200, 2);
        Assert.Equal("turn 100", o.Last);                                // a está mais longe da bandeira (z=320)
        Play(g, a, 320, 4);                                              // a acertou o buraco (estado 4)
        Assert.Equal("turn 101", o.Last);
        Play(g, b, 320, 4);
        Assert.Equal("end 100:1:-2:2,101:2:-2:2", o.Last);              // empate: desempata pela ordem do slot
    }

    [Fact]
    public void DuplicateShotAndForeignResultAreIgnored()
    {
        var (_, g, o, _) = Setup(2, bot: false, holes: 1);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Assert.True(g.Shoot(a));
        Assert.False(g.Shoot(a));                                         // repetida
        Assert.False(g.Result(new ShotResult(b.Guid, 0, 0, 1, 2, 0, 0)));  // resultado de quem não é a vez
        Assert.True(g.Result(new ShotResult(a.Guid, 0, 0, 1, 2, 0, 0)));
        Assert.False(g.Result(new ShotResult(a.Guid, 0, 0, 1, 2, 0, 0)));  // só o primeiro vale
        g.ShotFinished(a);
        Assert.DoesNotContain("turn", o.Last);                            // espera b confirmar
        g.ShotFinished(b);
        Assert.Equal("turn 101", o.Last);
        Assert.Equal(1, a.Strokes[0]);
    }

    [Fact]
    public void OutOfBoundsAddsPenaltyAndGiveUpAtParPlusFour()
    {
        var (_, g, o, _) = Setup(1, bot: false, holes: 1);
        Load(g, 1);
        var a = g.Players[0];
        Play(g, a, 10, ShotResult.StateWaterOrOut);                       // 1 + 1 de penalidade
        Assert.Equal(2, a.Strokes[0]);
        Play(g, a, 20, 2);
        Play(g, a, 30, 2);
        Assert.Equal(4, a.Strokes[0]);
        Assert.Equal("turn 100", o.Last);
        Assert.False(o.Events.Exists(e => e.StartsWith("end")));
        Play(g, a, 40, 2);                                               // 5 tacadas = par + 1... continua
        Play(g, a, 50, 2);
        Play(g, a, 60, 2);
        Play(g, a, 70, 2);                                               // 8 = par + 4: desiste
        Assert.Equal(8, a.Strokes[0]);
        Assert.StartsWith("end 100:1:4:8", o.Last);
    }

    [Fact]
    public void UnplayableAreaAddsPenaltyLikeOutOfBounds()
    {
        var (_, g, _, _) = Setup(1, bot: false, holes: 1);
        Load(g, 1);
        var a = g.Players[0];
        Play(g, a, 0, ShotResult.StateUnplayable);                        // 983: volta ao ponto anterior, +1 tacada
        Assert.Equal(2, a.Strokes[0]);
    }

    [Fact]
    public async Task BotPlaysItsTurnAfterDelay()
    {
        var (room, g, o, sync) = Setup(1, bot: true, holes: 1, botDelay: 20);
        lock (sync) Load(g, 1);
        var me = g.Players[0];
        var bot = g.Players[1];
        lock (sync) { g.TeeShotReady(me); Play(g, me, 100, 2); }
        Assert.Equal("turn 2130706433", o.Last);
        for (int i = 0; i < 100 && !o.Events.Contains("bot 2130706433"); i++) await Task.Delay(10);
        Assert.Contains("bot 2130706433", o.Events);
        lock (sync)
        {
            Assert.True(g.ShotOpen);
            Assert.True(g.Result(new ShotResult(bot.Guid, 0, 0, 320, 4, 0, 0)));   // o cliente humano simula a bola do bot
            g.ShotFinished(me);
        }
        Assert.Equal("turn 100", o.Last);
        Assert.Equal(RoomState.Playing, room.State);
        Assert.Equal(o.LastWind, (g.WindStrength, g.WindDirection));      // o bot mira com o vento que os clientes têm
        Assert.InRange(g.WindStrength, 0, 8);
    }

    [Fact]
    public void PlayerLeavingMidHoleDoesNotBlockTheOthers()
    {
        var (room, g, o, _) = Setup(2, bot: false, holes: 1);
        g.SoloAfterHoles = 0;                                             // já pode continuar sozinho
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Assert.True(g.Shoot(a));
        Assert.True(g.Result(new ShotResult(a.Guid, 0, 0, 100, 2, 0, 0)));
        g.ShotFinished(a);                                                // b sai antes de confirmar
        g.PlayerLeft(b.RoomPlayer);
        Assert.Contains("left 101", o.Events);
        Assert.Equal("ask 100", o.Last);                                 // continuar sozinho?
        g.ContinueReply(a, true);
        Assert.Equal("turn 100", o.Last);                                // a segue sozinho
        Play(g, a, 320, 4);
        Assert.StartsWith("end 100:1", o.Last);
        Assert.Equal(RoomState.Waiting, room.State);
    }

    [Fact]
    public void OnlyGmRoomsPassThirtyPlayers()
    {
        var s = new RoomSettings { Mode = GameMode.Tournament, MaxPlayers = 100 };
        s.Normalize();
        Assert.Equal(30, s.MaxPlayers);
        s = new RoomSettings { Mode = GameMode.Tournament, MaxPlayers = 200, ByGm = true };
        s.Normalize();
        Assert.Equal(200, s.MaxPlayers);
        s = new RoomSettings { Mode = GameMode.Stroke, MaxPlayers = 100, ByGm = true };
        s.Normalize();
        Assert.Equal(4, s.MaxPlayers);                                    // VS continua até 4
    }

    [Fact]
    public void OpponentLeavingBeforeTheSoloHoleEndsTheVersus()
    {
        var (room, g, o, _) = Setup(2, bot: false, holes: 18);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        g.TeeShotReady(a); g.TeeShotReady(b);
        g.PlayerLeft(b.RoomPlayer);                                       // buraco 1 de 18, sem terminar: volta para a sala
        Assert.Equal("aborted", o.Last);
        Assert.Equal(RoomState.Waiting, room.State);
    }

    [Fact]
    public void QuitCountsOnlyWhileTheGameRunsAndNotOnTheFirstHoleAbort()
    {
        var (_, g, _, _) = Setup(2, bot: false, holes: 18);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Assert.Equal(1, g.QuitHoles(b.Guid));
        g.PlayerLeft(b.RoomPlayer);
        Assert.Equal(0, g.QuitHoles(b.Guid));                              // já saiu
        Assert.True(g.Aborted);                                            // 0x65: quem chama descarta o abandono
        Assert.Equal(0, g.QuitHoles(a.Guid));                              // partida acabou
    }

    [Fact]
    public void BallPaysCompletedHolesOnce()
    {
        var (_, g, _, _) = Setup(1, bot: false, holes: 3);
        Load(g, 1);
        var p = g.Players[0];
        Play(g, p, 320, 4);                                                // fechou o 1º
        Load(g, 2);
        Play(g, p, 100, 2);                                                // no 2º sem embocar: não conta
        g.PlayerLeft(p.RoomPlayer);
        Assert.Equal(1, g.TakeBallHoles(p.Guid));
        Assert.Equal(0, g.TakeBallHoles(p.Guid));                          // saída + fim: não paga duas vezes
    }

    [Fact]
    public void BallCountsTheLastHoleAndSkipsAbortedGames()
    {
        var (_, g, _, _) = Setup(1, bot: false, holes: 1);
        Load(g, 1);
        Play(g, g.Players[0], 320, 4);
        Assert.True(g.Over);
        Assert.Equal(1, g.TakeBallHoles(g.Players[0].Guid));

        var (_, g2, _, _) = Setup(2, bot: false, holes: 18);
        Load(g2, 1);
        g2.PlayerLeft(g2.Players[1].RoomPlayer);
        Assert.True(g2.Aborted);
        Assert.Equal(0, g2.TakeBallHoles(g2.Players[0].Guid));
    }

    [Fact]
    public void QuitHolesSkipsPlayerWhoClosedTheLastHole()
    {
        var (_, g, _, _) = Setup(2, bot: false, holes: 1);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        var first = g.Turn!;
        Play(g, first, 320, 4);                                            // embocou: o outro ainda joga
        Assert.Equal(0, g.QuitHoles(first.Guid));
        Assert.Equal(1, g.QuitHoles((first == a ? b : a).Guid));
    }

    [Fact]
    public void OpponentLeavingAfterTheFirstHoleEndsWithTheScore()
    {
        var (room, g, o, _) = Setup(2, bot: false, holes: 18);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Play(g, a, 320, 4); Play(g, b, 320, 4);
        g.HoleData(2, new HoleInfo(4, 0, 0, 0, 320));
        g.Loaded(a); g.Loaded(b);
        g.PlayerLeft(b.RoomPlayer);                                       // buraco 2: acaba com o placar
        Assert.StartsWith("end 100:1", o.Last);
        Assert.Equal(RoomState.Waiting, room.State);
    }

    [Fact]
    public void SayingNoToPlayingAloneEndsTheGame()
    {
        var (_, g, o, _) = Setup(2, bot: false, holes: 2);
        g.SoloAfterHoles = 1;
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Play(g, a, 320, 4); Play(g, b, 320, 4);
        Assert.Equal("hole", o.Last);
        g.HoleData(2, new HoleInfo(4, 0, 0, 0, 320));
        g.Loaded(a); g.Loaded(b);
        g.PlayerLeft(b.RoomPlayer);                                       // buraco 2: pergunta depois do tee
        Assert.DoesNotContain("ask 100", o.Events);
        g.TeeShotReady(a);
        Assert.Equal("ask 100", o.Last);
        g.ContinueReply(a, false);
        Assert.StartsWith("end 100:1", o.Last);
    }

    [Fact]
    public async Task ShotClockRunningOutPassesTheTurn()
    {
        var (room, g, o, sync) = Setup(2, bot: false, holes: 1);
        room.Settings.ShotTimeMs = 50;
        g.ClockGrace = TimeSpan.Zero;
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        lock (sync) g.ClockStarted(a);
        await Task.Delay(400);
        lock (sync)
        {
            Assert.Equal("timeout 100", o.Last);
            Assert.Equal(1, a.Strokes[0]);                                // +1 tacada, como o cliente conta
            g.ShotFinished(a); g.ShotFinished(b);
            Assert.Equal("turn 101", o.Last);
            g.ClockStarted(b);
            Assert.True(g.Shoot(b));                                      // tacou a tempo: nada
        }
        await Task.Delay(400);
        Assert.DoesNotContain("timeout 101", o.Events);
    }

    [Fact]
    public async Task PausedGameStopsTheShotClock()
    {
        var (room, g, o, sync) = Setup(2, bot: false, holes: 1);
        room.Settings.ShotTimeMs = 150;
        g.ClockGrace = TimeSpan.Zero;
        Load(g, 1);
        var a = g.Players[0];
        lock (sync) { g.ClockStarted(a); g.Pause(true); }
        await Task.Delay(400);
        lock (sync)
        {
            Assert.DoesNotContain("timeout 100", o.Events);
            g.Pause(false);
        }
        await Task.Delay(500);
        Assert.Contains("timeout 100", o.Events);
    }

    [Fact]
    public void NextHoleWaitsForEveryoneToLoadAndBestScoreTeesFirst()
    {
        var (_, g, o, _) = Setup(2, bot: false, holes: 2);
        Load(g, 1);
        var (a, b) = (g.Players[0], g.Players[1]);
        Play(g, a, 100, 2); Play(g, b, 320, 4); Play(g, a, 320, 4);    // a: 2 tacadas, b: 1
        Assert.Equal("hole", o.Last);
        g.HoleData(2, new HoleInfo(4, 0, 0, 0, 320));
        g.Loaded(a);
        Assert.Equal("hole", o.Last);                                     // espera b carregar
        g.Loaded(b);
        Assert.Equal("start 101", o.Last);                                // b fez menos tacadas: tee primeiro
    }
}

public class RoomManagerTests
{
    [Fact]
    public void JoinLeaveMasterAndClose()
    {
        var mgr = new RoomManager();
        var room = mgr.Create(new RoomSettings { Title = new string('x', 100), Holes = 40, MaxPlayers = 2, Password = "pw" }, 1);
        Assert.Equal(31, room.Settings.Title.Length);                    // normalizado
        Assert.Equal(18, room.Settings.Holes);
        var a = new FakeSession(1);
        var b = new FakeSession(2);
        Assert.Equal(JoinResult.WrongPassword, RoomManager.CanJoin(room, "x"));
        Assert.Equal(JoinResult.NotFound, RoomManager.CanJoin(mgr.Get(999), ""));
        var ra = RoomManager.Join(room, new RoomPlayer { Guid = 1, Player = a.Player, Session = a });
        var rb = RoomManager.Join(room, new RoomPlayer { Guid = 2, Player = b.Player, Session = b });
        Assert.True(ra.Master && !rb.Master);
        Assert.Equal(JoinResult.Full, RoomManager.CanJoin(room, "pw"));
        Assert.Equal(2, rb.Slot);
        var (newMaster, closed) = mgr.Leave(room, ra);
        Assert.False(closed);
        Assert.Same(rb, newMaster);
        Assert.Equal(1, rb.Slot);
        RoomManager.Join(room, new RoomPlayer { Guid = 0x7F000001, Player = new Player() });   // bot
        (_, closed) = mgr.Leave(room, rb);
        Assert.True(closed);                                              // só sobrou o bot: a sala fecha
        Assert.Null(mgr.Get(room.Index));
    }

    [Theory]
    [InlineData(0, 1)]
    [InlineData(1, 10)]
    public void HoleOrderFollowsHoleType(byte type, int first)
    {
        var room = new RoomManager().Create(new RoomSettings { HoleType = type }, 1);
        RoomManager.PrepareStart(room, new Random(5));
        Assert.Equal(first, room.HoleOrder[0]);
        Assert.Equal(Enumerable.Range(1, 18), room.HoleOrder.Select(h => (int)h).Order());
        Assert.Equal(RoomState.Playing, room.State);
    }

    [Fact]
    public void ShuffledAndRandomCourseAreValid()
    {
        var room = new RoomManager().Create(new RoomSettings { HoleType = 3, Course = 0x7F }, 1);
        RoomManager.PrepareStart(room, new Random(7));
        Assert.Equal(Enumerable.Range(1, 18), room.HoleOrder.Select(h => (int)h).Order());
        Assert.InRange(room.CoursePlayed, 0, 10);
    }

    [Fact]
    public void ShuffleCourseGivesEachHoleACourseAndEndsInChaos()
    {
        var room = new RoomManager().Create(new RoomSettings { Mode = (GameMode)RoomManager.ShuffleCourseMode, Holes = 18, Course = 0x7F }, 1);
        byte[] allowed = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 19];
        RoomManager.PrepareStart(room, new Random(3), allowed);
        Assert.Equal(RoomManager.ChaosCourse, room.CoursePlayed);
        Assert.Equal(18, room.HoleCourses.Length);
        Assert.Equal(RoomManager.ChaosCourse, room.HoleCourses[17]);                 // último: Chronicle 1 Chaos
        Assert.InRange(room.HoleOrder[17], 1, 2);
        for (int i = 0; i < 17; i++) Assert.Contains(room.HoleCourses[i], allowed);  // os outros: mapas sorteados
        Assert.Equal(Enumerable.Range(1, 18), room.HoleOrder.Select(h => (int)h).Order());

        var body = RoomPackets.GameInit(room).Body.ToArray();                        // u16 id, u8 mapa, ...
        Assert.Equal(0x80 | RoomManager.ChaosCourse, body[2]);
        int hole0 = 2 + 4 + 12;                                                       // 4 × u8 + 3 × u32
        Assert.Equal(room.HoleCourses[0], body[hole0 + 4]);                          // u32 semente, u8 mapa, u8 buraco
        Assert.Equal(room.HoleOrder[0], body[hole0 + 5]);
        Assert.Equal(RoomManager.ChaosCourse, body[hole0 + 17 * 6 + 4]);
    }
}

public class ShotSplitTests
{
    // tacada real do cliente (log do usuário 2026-10-07 16:17): u16 0 + 46 bytes
    static readonly byte[] Real12 = Convert.FromHexString("00000080f64300000f434341c8b18988083e03000000003e020000009282bce936000000000c43000000000000000080");

    [Fact]
    public void RealShotIsLastFortySixBytes()
    {
        Assert.True(InGameOutput.SplitShot(Real12, out int start, out int tail));
        Assert.Equal(2, start);
        Assert.Equal(0, tail);
    }

    [Fact]
    public void CardsWindAndSyncTimeAreHandled()
    {
        // u16 1, u32 1 card (8 bytes), vento u8 + u32, bloco, f32 de sincronia
        var b = new byte[2 + 4 + 8 + 5 + 46 + 4];
        b[0] = 1; b[2] = 1;
        Assert.True(InGameOutput.SplitShot(b, out int start, out int tail));
        Assert.Equal(19, start);
        Assert.Equal(4, tail);
        Assert.False(InGameOutput.SplitShot(new byte[10], out _, out _));
    }
}

[Collection("db")]
public class InGameItemTests(DbFixture fx)
{
    [Fact]
    public async Task ItemUseOnOwnTurnIsConsumedAndSaved()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        // dá 2 unidades do item 0x18000004 (power shot) equipadas em 2 slots
        var ids = await env.S.Players.NewIdsAsync(1);
        await using (var c0 = await env.S.Db.OpenAsync())
            await Dapper.SqlMapper.ExecuteAsync(c0, "insert into items(id, account_id, type_id, quantity) values (@id, @acc, @tid, 2)",
                new { id = ids[0], acc = acc.Id, tid = 0x18000004 });
        var p0 = (await env.Players.LoadAsync(acc.Id))!;
        p0.Equip.ItemSlots[0] = 0x18000004;
        p0.Equip.ItemSlots[1] = 0x18000004;
        await env.S.Players.SaveEquipAsync(acc.Id, p0.Equip);

        await using var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        await c.ExpectAsync(0x94);
        await c.SendAsync(new PacketWriter(0x08).U8(0).U32(60000).U32(0).U8(4).U8(0).U8(3).U8(0).U8(0).Str("t").Str(""));
        await c.ExpectAsync(0x46);
        await c.SendAsync(new PacketWriter(0x0E).U32((uint)acc.Id));
        await c.ExpectAsync(0x50);
        await c.SendAsync(new PacketWriter(0x17).U32(0x18000004));        // antes do buraco começar: recusado
        await c.SendAsync(new PacketWriter(0x1A).U8(1).U32(0).U32(0).U8(4).F32(0).F32(0).F32(0).F32(320));
        await c.SendAsync(new PacketWriter(0x11));
        await c.ExpectAsync(0x51);
        await c.SendAsync(new PacketWriter(0x17).U32(0x18000005));        // não equipado: recusado
        await c.SendAsync(new PacketWriter(0x17).U32(0x18000004));
        var r = await c.ExpectAsync(0x58);
        Assert.Equal(0x18000004u, r.U32());
        r.U32();
        Assert.Equal((uint)acc.Id, r.U32());
        var counts = await c.ExpectAsync(0xA5);
        Assert.Equal(1, counts.U8());
        Assert.Equal(0x18000004u, counts.U32());
        Assert.Equal((uint)ids[0], counts.U32());
        Assert.Equal(1, counts.U16());
        var equip = (await c.ExpectAsync(0x70)).Struct<Pangya.Protocol.KR645.sUserEquip>();
        Assert.Equal(0x18000004u, equip.tidItemSlot[0]);
        Assert.Equal(0u, equip.tidItemSlot[1]);
        for (int i = 0; i < 50; i++)
        {
            var saved = (await env.Players.LoadAsync(acc.Id))!;
            if (saved.Find(ids[0])!.Quantity == 1 && saved.Equip.ItemSlots[1] == 0) return;
            await Task.Delay(20);
        }
        Assert.Fail("consumo do item não foi salvo no banco");
    }
}

public class WindTests
{
    [Fact]
    public void WindIsDrawnOncePerHoleAndRepeatedOnEachTurn()
    {
        var mgr = new RoomManager();
        var room = mgr.Create(new RoomSettings { Holes = 2 }, 1);
        for (int i = 0; i < 2; i++)
        {
            var s = new FakeSession(100 + i);
            RoomManager.Join(room, new RoomPlayer { Guid = (uint)(100 + i), Player = s.Player, Session = s });
        }
        RoomManager.PrepareStart(room, new Random(1));
        room.HoleOrder = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18];
        var winds = new List<(byte, byte)>();
        var o = new WindRecorder(winds);
        var g = new StrokeGame(room, o, mgr.Sync, TimeSpan.FromSeconds(10), TimeSpan.FromSeconds(30));
        room.Game = g;
        g.HoleData(1, new HoleInfo(4, 0, 0, 0, 320));
        foreach (var p in g.Players) g.Loaded(p);
        for (int shot = 0; shot < 3; shot++)
        {
            var p = g.Turn!;
            g.Shoot(p);
            g.Result(new ShotResult(p.Guid, 0, 0, 50 + shot * 10, 2, 0, 0));
            foreach (var h in g.Players) g.ShotFinished(h);
        }
        Assert.True(winds.Count >= 4);
        Assert.All(winds, w => Assert.Equal(winds[0], w));              // o mesmo vento em todas as vezes do buraco
    }

    sealed class WindRecorder(List<(byte, byte)> winds) : IGameOutput
    {
        public void Wind(byte wind, byte direction) => winds.Add((wind, direction));
        public void HoleStart(GamePlayer first) { }
        public void TeeReady() { }
        public void NextTurn(GamePlayer p) { }
        public void NextHole(uint holeWinner) { }
        public void GameEnd(GameEnd end) { }
        public void PlayerLeft(GamePlayer p) { }
        public void BotTurn(GamePlayer bot) { }
    }
}

public class CourseTests
{
    [Fact]
    public void CoursesFollowTheClientMapList()
    {
        var data = Pangya.Protocol.KR645.Kr645GameData.Load(Path.Combine(TestEnv.Root, "data/pangya.iff"));
        var c = data.Courses;
        Assert.DoesNotContain((byte)0x11, c);                          // Chaos e Ice Inferno não são escolhíveis no 645
        Assert.DoesNotContain((byte)0x12, c);
        Assert.Equal(c.Count, c.Distinct().Count());
        Assert.True(c.Count >= 16);
        // ordem da tela do cliente: (Wiz City se os dados tiverem), 0x10, 0x0F, ...
        Assert.Equal(c.Contains((byte)0x13) ? 0x13 : 0x10, c[0]);
    }

    [Fact]
    public void InvalidCourseBecomesRandomAndRandomPicksAnAllowedMap()
    {
        var mgr = new RoomManager { Courses = [0x10, 0x05, 0x02] };
        Assert.Equal(0x05, mgr.ValidCourse(0x05));
        Assert.Equal(RoomManager.RandomCourse, mgr.ValidCourse(0x11));      // não permitido
        Assert.Equal(RoomManager.RandomCourse, mgr.ValidCourse(0xFD));
        var room = mgr.Create(new RoomSettings { Course = 0x12 }, 1);
        Assert.Equal(RoomManager.RandomCourse, room.Settings.Course);
        var rng = new Random(3);
        for (int i = 0; i < 50; i++)
        {
            RoomManager.PrepareStart(room, rng, mgr.Courses);
            Assert.Contains(room.CoursePlayed, mgr.Courses);
            room.State = RoomState.Waiting;
        }
        room.Settings.Course = 0x02;
        RoomManager.PrepareStart(room, rng, mgr.Courses);
        Assert.Equal(0x02, room.CoursePlayed);
    }
}
