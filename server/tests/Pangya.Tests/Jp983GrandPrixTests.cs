using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Grand Prix do JP 983, fase 2 (SPEC-jp983-grandprix.md): tabelas, regras, entrar, começar e o fim.</summary>
public partial class Jp983GameTests
{
    const int GpTicket = 0x1A000264;

    [Fact]
    public void GrandPrixTablesAndRules()
    {
        if (Jp983DataTests.Data is not { } d) return;
        Assert.Equal(741, d.GrandPrix.Count);
        var rookie = d.GrandPrix[0x100];
        Assert.True(rookie.IsRookie);
        Assert.Equal((GpTicket, 1, (byte)3), (rookie.TicketTypeId, rookie.TicketQty, rookie.Holes));
        Assert.True(d.GrandPrix[0x200].Locked);                                         // o 2º Rookie exige o 1º
        Assert.NotEmpty(d.GrandPrixBots(rookie.BotClass));
        Assert.Empty(d.GrandPrixRankRewards(rookie.Link));                              // Rookie: só a participação
        Assert.Equal(3, d.GrandPrixRankRewards(0x80100).Count);                        // os com horário: 1º, 2º e 3º

        var p = new Player { Level = 10 };
        Assert.Equal(GrandPrix.Ticket, GrandPrix.CanEnter(p, rookie, [], DateTime.Now));
        p.Items[1] = new Item { Id = 1, TypeId = GpTicket, Quantity = 2 };
        Assert.Equal(GrandPrix.Ok, GrandPrix.CanEnter(p, rookie, [], DateTime.Now));
        Assert.Equal(GrandPrix.Locked, GrandPrix.CanEnter(p, d.GrandPrix[0x200], [], DateTime.Now));
        Assert.Equal(GrandPrix.Ok, GrandPrix.CanEnter(p, d.GrandPrix[0x200], new() { [0x100] = 3 }, DateTime.Now));

        var timed = d.GrandPrix[0x80101];                                               // abre 00:00, começa 00:10
        Assert.True(GrandPrix.IsOpen(timed, DateTime.Today.AddMinutes(timed.OpenMinute + 1)));
        Assert.False(GrandPrix.IsOpen(timed, DateTime.Today.AddMinutes(timed.StartMinute + 5)));

        var bots = GrandPrix.MakeBots(rookie, d.GrandPrixBots(rookie.BotClass), 1, [1, 2, 3], new Random(1));
        Assert.Equal(29, bots.Count);
        Assert.All(bots, b => Assert.Equal(3, b.Holes.Count));
        Assert.Equal(1, GrandPrix.Position(-5, 0, [(0, 0), (-5, 0)]));                   // empate: mais pang na frente
        Assert.Equal(2, GrandPrix.Position(-5, 0, [(0, 0), (-5, 10)]));
        Assert.Equal((0, 1), GrandPrixRules.WindRange(GrandPrixRules.Wind1m));            // vento das regras (referência)
        Assert.Equal((9, 15), GrandPrixRules.WindRange(GrandPrixRules.SuperWind));
        Assert.Null(GrandPrixRules.WindRange(0));
        Assert.Contains(d.GrandPrix.Values, g => g.TimeHoleSeconds > 0);                  // há GPs com relógio por buraco (0x259)
        Assert.Equal(3, d.GrandPrix[0x100].SpecialHoles.Length);                         // Rookie: 3 buracos especiais (mapa e buraco)
        Assert.True(GrandPrixRules.Penalized(GrandPrixRules.TenSeconds, new ShotResult(1, 0, 0, 0, 0, 0, 0, Penalty: 1)));
        Assert.False(GrandPrixRules.Penalized(GrandPrixRules.TenSeconds, new ShotResult(1, 0, 0, 0, 0, 0, 0)));
        Assert.True(GrandPrixRules.Penalized(GrandPrixRules.PowerShotPenalty, new ShotResult(1, 0, 0, 0, 0, 0, 0, ShotFlags: 0x1004)));
        Assert.False(GrandPrixRules.Penalized(GrandPrixRules.PowerShotPenalty, new ShotResult(1, 0, 0, 0, 0, 0, 0, ShotFlags: 0x1000)));
        Assert.Equal(20, GrandPrixRules.ClearBonus(0));
    }

    [Fact]
    public async Task RookieGrandPrixGoesFromLobbyToPrizes()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 1000, (GpTicket, 2));
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x176));                                    // lobby do GP: nada concluído
        var lobby = await g.ExpectAsync(0x250);
        Assert.Equal((0u, 0u, 0u), (lobby.U32(), lobby.U32(), lobby.U32()));
        Assert.Equal(4, lobby.Remaining);

        await g.SendAsync(new PacketWriter(0x179).U32(0x200));                         // travado: precisa do 0x100
        Assert.Equal(GrandPrix.Locked, (await g.ExpectAsync(0x253)).U32());
        await g.SendAsync(new PacketWriter(0x179).U32(0x100));                         // Rookie: sala só dele
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var info = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal((4, 0x14), (info[88], info[196]));                                 // torneio; tipo real Grand Prix
        Assert.Equal(0x100, BitConverter.ToInt32(info, 205));
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        Assert.Equal((0x100, (byte)30), (room.Settings.GrandPrixId, room.Settings.MaxPlayers));

        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));                          // Rookie: o jogador começa
        Assert.Equal(0, (await g.ExpectAsync(0x253)).I32());
        await g.ExpectAsync(0x76);
        var bots = await g.ExpectAsync(0x256);
        var sh = data.GrandPrix[0x100].SpecialHoles;                                     // buracos especiais na ordem do GP
        Assert.Equal((sh[0].Hole, sh[0].Course), (room.HoleOrder[0], room.HoleCourses[0]));
        Assert.Equal((0u, (ushort)29), (bots.U32(), bots.U16()));
        await g.ExpectAsync(0x52);
        var ticket = await g.ExpectAsync(0x216);                                        // ticket gasto
        ticket.Skip(4 + 4 + 1);
        Assert.Equal((uint)GpTicket, ticket.U32());
        ticket.Skip(4 + 4);
        Assert.Equal((2, 1), (ticket.I32(), ticket.I32()));

        // fim de buraco: o torneio manda o 0x63 (-> 0x65); no GP vai junto o 0x255, sem ele o cliente espera "os outros"
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        h.Connection.Send(new PacketWriter(0x63));
        await g.ExpectAsync(0x65);
        Assert.Equal(0, (await g.ExpectAsync(0x255)).Remaining);

        // fim (o placar do torneio chega ao receptor do GP de cada um): placar -30 = 1º lugar
        var me = new GrandPrixEntry((uint)acc.Id, h.Player, -30, 500);
        await h.GrandPrix!.FinishedAsync(room.GrandPrix!, me, [me]);
        var podium = await g.ExpectAsync(0x258);
        Assert.Equal((0u, (byte)1, (uint)acc.Id, 1u), (podium.U32(), podium.U8(), podium.U32(), podium.U32()));
        Assert.Equal(222 - 8, podium.Remaining);
        var cleared = await g.ExpectAsync(0x25A);
        Assert.Equal((0u, (uint)0x100, 1u), (cleared.U32(), cleared.U32(), cleared.U32()));
        Assert.Equal(1, (await env.S.GrandPrix.CompletedAsync(acc.Id))[0x100]);
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.True(p.Pang > 1000);                                                     // Pang Pouch de participação
    }

    /// <summary>GP com horário: o 1º lugar leva o troféu da posição (0x25C) além dos prêmios (fim aplicado direto ao jogador).</summary>
    [Fact]
    public async Task TimedGrandPrixGivesTheTrophy()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var d = data.GrandPrix[0x80101];                                                // Shining Sand (9H), 00:00..00:10
        Assert.False(d.IsRookie);
        var start = DateTime.Today.AddMinutes(d.StartMinute);
        Assert.Equal(start, GrandPrix.StartTime(d, DateTime.Today));                    // começa sozinho no Start de hoje
        Assert.Equal(start.AddDays(1), GrandPrix.StartTime(d, start.AddMinutes(1)));    // passou: o de amanhã
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0);
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        var me = new GrandPrixEntry((uint)acc.Id, h.Player, -40, 900);
        await h.GrandPrix!.FinishedAsync(new GrandPrixRun(d), me, [me]);
        await g.ExpectAsync(0x258);
        var trophy = await g.ExpectAsync(0x25C);
        Assert.Equal((0u, (uint)data.GrandPrixRankRewards(d.Link)[0].Trophy), (trophy.U32(), trophy.U32()));
        var won = await g.ExpectAsync(0x216);                                           // sem novo login: entra no My Room
        won.Skip(4 + 4 + 1);
        Assert.Equal((uint)data.GrandPrixRankRewards(d.Link)[0].Trophy, won.U32());
        var cleared = await g.ExpectAsync(0x25A);
        Assert.Equal((0u, (uint)d.Link, 1u), (cleared.U32(), cleared.U32(), cleared.U32()));
        int gpTrophy = data.GrandPrixRankRewards(d.Link)[0].Trophy;
        Assert.Contains((gpTrophy, 1), await env.S.SpecialTrophies.ListAsync(acc.Id));  // troféu no perfil

        // perfil no JP: o troféu de evento de GM no 0x15A e o do GP no 0x257
        await env.S.SpecialTrophies.AddAsync(acc.Id, 0x2D0A3B00, 1);
        await g.SendAsync(new PacketWriter(0x2F).U32((uint)acc.Id).U8(5));
        var special = await g.ExpectAsync(0x15A);
        special.Skip(5);
        Assert.Equal(1, special.U16());
        special.I32();
        Assert.Equal(0x2D0A3B00u, special.U32());
        var gp = await g.ExpectAsync(0x257);
        gp.Skip(5);
        Assert.Equal(1, gp.U16());
        gp.I32();
        Assert.Equal((uint)gpTrophy, gp.U32());
    }
}
