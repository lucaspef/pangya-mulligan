using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Prática (tipo 19 do JP), repetir buraco e tutorial, pelo cliente JP falso (SPEC-jp983-pratica.md).</summary>
public partial class Jp983GameTests
{
    const string PracticeName = "Single Player Practice Mode";

    [Fact]
    public async Task PracticeIsAHiddenOnePlayerTournamentWithoutExp()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        // diálogo de criar sala (makeroomdlg): tipo 19, 3 buracos, ordem 0 = prática do campo
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)1, (byte)19, (byte)3, (byte)0, (byte)0,
            0u, PracticeName, "12345678", 0u));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var info = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal(PracticeName, Pangya.Core.Text.Cp949.Read(info.AsSpan(0, 64)));
        Assert.Equal((4, 19), (info[88], info[196]));                                 // tipo de exibição torneio, tipo real prática
        var settings = await g.ExpectAsync(0x4A);
        settings.U16();
        Assert.Equal((4, 0, 3, 0), (settings.U8(), settings.U8(), settings.U8(), settings.U8()));

        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        Assert.Equal((true, GameMode.Tournament, (byte)1), (room.Settings.Practice, room.Settings.Mode, room.Settings.MaxPlayers));
        using (var list = RoomPackets.RoomListAll(env.Game.World.Rooms)) Assert.Equal(0, list.Body[2]);   // fora da lista
        Assert.Equal(JoinResult.NotFound, RoomManager.CanJoin(room, "12345678"));

        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        var players = await g.ExpectAsync(0x76);                                      // em massa: u8 tipo, u32 1, SYSTEMTIME
        Assert.Equal(((byte)4, 1u, 16), (players.U8(), players.U32(), players.Remaining));
        var init = await g.ExpectAsync(0x52);
        Assert.Equal((0, 4), (init.U8(), init.U8()));
        Assert.Equal(0, ((TourneyGame)room.Game!).MatchTid);                        // sem troféu

        await g.SendAsync(new PacketWriter(0x130));                                   // sair da prática
        var result = await g.ExpectAsync(0x79);
        Assert.Equal(0, result.I32());                                                // sem EXP
        await g.ExpectAsync(0x8C);                                                    // fim (como o fim do tempo)
    }

    [Fact]
    public async Task RepeatHolePracticePlaysTheSameHole()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        // botão de prática do lobby (lobbytask): ordem 4 = repetir, + u8 buraco 7, u32 fixos 7
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00abaa10", (byte)0, 60000u, 0u, (byte)1, (byte)19, (byte)5, (byte)0, (byte)4,
            (byte)7, 7u, 0u, PracticeName, "12345678", 0u));
        var info = (await g.ExpectAsync(0x49)).Bytes(2 + KrToJp.RoomSize).ToArray();
        Assert.Equal((4, 19), (info[2 + 91], info[2 + 196]));
        var settings = await g.ExpectAsync(0x4A);
        settings.U16();
        Assert.Equal((4, 0, 5, 4, 7, 7u), (settings.U8(), settings.U8(), settings.U8(), settings.U8(), settings.U8(), settings.U32()));
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        Assert.Equal(7, room.Settings.RepeatHole);
        // clicar num buraco: 0x0A u16 sala, u8 n, chave 11 (buraco) e chave 6 (tempo da tacada, u16 no JP)
        await g.SendAsync(new PacketWriter(0x0A).U16((ushort)room.Index).U8(2).U8(11).U8(9).U8(6).U16(40));
        settings = await g.ExpectAsync(0x4A);
        settings.U16();
        Assert.Equal((4, 0, 5, 4, 9), (settings.U8(), settings.U8(), settings.U8(), settings.U8(), settings.U8()));
        Assert.Equal((9, 40_000u), (room.Settings.RepeatHole, room.Settings.ShotTimeMs));
        await g.SendAsync(new PacketWriter(0x0A).U16((ushort)room.Index).U8(1).U8(11).U8(7));
        await g.ExpectAsync(0x4A);

        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        await g.ExpectAsync(0x76);
        var init = await g.ExpectAsync(0x52);
        Assert.Equal((0, 4, 4, 5), (init.U8(), init.U8(), init.U8(), init.U8()));
        init.Skip(12);
        uint first = 0;
        for (int i = 0; i < 18; i++)
        {
            uint id = init.U32();
            if (i == 0) first = id;
            Assert.Equal(first, id);                                                  // mesmo buraco e pin em todas
            init.U8(); init.U8();
            Assert.Equal(i + 1, init.U8());                                           // nº da vez (referência)
        }
        Assert.All(room.HoleOrder, h => Assert.Equal(7, h));
        await g.SendAsync(new PacketWriter(0x141));                                   // trocar o vento: sem resposta, sem cair
        await g.SendAsync(new PacketWriter(0x130));
        await g.ExpectAsync(0x79);
    }

    /// <summary>Chip-in Practice (Grand Zodiac 14, SPEC-jp983-grand-zodiac.md): precisa do ticket; carrega, começa, sai.</summary>
    [Fact]
    public async Task ChipInPracticeNeedsTheTicketAndPlays()
    {
        await using var env = new JpEnv();
        PacketWriter Make() => Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 1_800_000u, (byte)1, (byte)14, (byte)1, (byte)0,
            (byte)0, 0u, "chip-in", "", 0u);
        var (g0, _) = await InChannelAsync(env);
        await using (g0)
        {
            await g0.SendAsync(Make());                                               // sem ticket: recusada
            var refused = await g0.ExpectAsync(0x49);
            Assert.Equal((1, (byte)JoinResult.NotFound), (refused.Remaining, refused.U8()));
            Assert.Empty(env.Game.World.Rooms.Rooms);
        }

        var (g, acc) = await InChannelWithAsync(env, 0, (0x1A00017E, 2));
        await using var _ = g;
        await g.SendAsync(Make());
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var info = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal((11, 14), (info[88], info[196]));                                // exibição GZ, tipo real Chip-in
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        Assert.Equal((ZodiacKind.ChipIn, (byte)0x40, (byte)1, 1_800_000u),
            (room.Settings.Zodiac, room.Settings.Course, room.Settings.MaxPlayers, room.Settings.GameTimeMs));

        await g.SendAsync(Jp983Client.Build(0x0E, null, 0u));
        var cup = await g.ExpectAsync(0x1F9);
        Assert.Equal((9, 9, 0u), (cup.I32(), cup.I32(), cup.U32()));
        Assert.Equal(11, (await g.ExpectAsync(0x76)).U8());
        var init = await g.ExpectAsync(0x52);
        Assert.Equal((0x40, 11), (init.U8(), init.U8()));                              // mapa 0x40, tipo GZ
        var ticket = await g.ExpectAsync(0x216);                                        // ticket gasto
        ticket.Skip(4 + 4 + 1 + 4 + 4 + 4);
        Assert.Equal((2, 1), (ticket.I32(), ticket.I32()));
        var game = Assert.IsType<GrandZodiacGame>(room.Game);

        await g.SendAsync(new PacketWriter(0x11).U8(100));                             // carregou: vento, 10 valores, 0x201
        await g.ExpectAsync(0x5B);
        for (int i = 0; i < 10; i++) Assert.Equal(8, (await g.ExpectAsync(0x1EC)).Remaining);
        await g.ExpectAsync(0x201);
        await g.SendAsync(new PacketWriter(0x137));                                    // pronto: relógio e 1º buraco
        Assert.Equal(0u, (await g.ExpectAsync(0x8D)).U32());
        Assert.Equal((uint)acc.Id, (await g.ExpectAsync(0x53)).U32());
        await g.ExpectAsync(0x1EE);
        Assert.Equal(1u, (await g.ExpectAsync(0x1F4)).U32());

        // acerto de primeira, sem comandos: 1 + 2 bônus = 3 pontos (0x1F5 com os tipos, 0xCC, 0x1EF e o próximo buraco)
        var mp = game.Players[0];
        game.Shoot(mp, 0);
        game.Result(mp, new ShotResult((uint)acc.Id, 0, 0, 0, 0, 10, 0, Display: 0x100));
        game.ShotFinished(mp);
        var types = await g.ExpectAsync(0x1F5);
        Assert.Equal((3u, 2u, 4u, 1u), (types.U32(), types.U32(), types.U32(), types.U32()));
        await g.ExpectAsync(0xCC);
        var score = await g.ExpectAsync(0x1EF);
        Assert.Equal(((uint)acc.Id, 0, 3, 3), (score.U32(), score.I32(), score.I32(), score.I32()));
        await g.ExpectAsync(0x1F4);
        await g.ExpectAsync(0x5B);
        await g.ExpectAsync(0x200);
        await g.ExpectAsync(0x1EE);
        game.Shoot(mp, 0);                                                              // errou: volta ao tee
        game.Result(mp, new ShotResult((uint)acc.Id, 0, 0, 0, 0, 0, 0));
        game.ShotFinished(mp);
        await g.ExpectAsync(0xCC);
        await g.ExpectAsync(0x1EE);

        Assert.Equal(1 + 1 + 1 + 3 + 1, GrandZodiacGame.Points([3, 2, 4, 5, 1]));           // os 4 bônus do cliente + o acerto

        await g.SendAsync(new PacketWriter(0x131));                                    // sair: fim sem completar
        await g.ExpectAsync(0x1F2);
        await g.ExpectAsync(0x8C);
        var st = await g.ExpectAsync(0x1F3);
        Assert.Equal((2u, 1u, (uint)acc.Id, 1u, 3), (st.U32(), st.U32(), st.U32(), st.U32(), st.I32()));
        await g.ExpectAsync(0x24F);
        Assert.True(game.Over);
    }

    [Fact]
    public async Task TutorialMissionAnswersWithItsCategory()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0xAE).U8(0).U8(1).U32(0x100));             // beginner, 1ª missão
        var r = await g.ExpectAsync(0x11F);
        Assert.Equal((1, 1, 0x100u, 0), (r.U8(), r.U8(), r.U32(), r.Remaining));
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal(0x100, p.Tutorial[1]);
        await g.ExpectAsync(0x33);                                                    // carta nova com o prêmio (KR 0x31)
        var (mails, total) = await env.S.Mail.ListAsync(acc.Id, 1, 20);
        Assert.Equal(1, total);
        var mail = Assert.Single(mails);
        Assert.Equal((0x1A000002, 10), (mail.Items[0].TypeId, mail.Items[0].Quantity));   // Pang Mastery × 10
        await g.SendAsync(new PacketWriter(0xAE).U8(0).U8(1).U32(0x100));             // de novo: sem prêmio
        await g.ExpectAsync(0x11F);
        Assert.Equal(1, (await env.S.Mail.ListAsync(acc.Id, 1, 20)).Total);
        Assert.Equal([(0x18000027, 10), (0x1A000010, 10_000)],
            Pangya.Domain.Players.TutorialPrizes.For(1, 0x1F00, 0x3F00)[1..]);       // beginner completo: + poção e pang
    }
}
