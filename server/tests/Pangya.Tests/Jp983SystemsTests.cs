using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Loja, estatísticas, lobby do Grand Prix, pedidos que só o JP faz e o modo Special Shuffle, pelo cliente JP falso.</summary>
public partial class Jp983GameTests
{
    /// <summary>Conta nova logada no game server JP, já no canal 0 (o login inteiro já passou).</summary>
    static async Task<(TestClient G, Domain.Accounts.Account Acc)> InChannelAsync(JpEnv env)
    {
        var (login, acc) = await env.NewAccountAsync();
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        await g.ExpectAsync(0x96);                                                     // cookie: fim do login
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        Assert.Equal(1, (await g.ExpectAsync(0x4E)).U8());
        return (g, acc);
    }

    [Fact]
    public void ServerIdsFollowTheRangesMeasuredInTheExe()
    {
        // KR 0x108..0x113 = +2 e 0x11E..0x122 = +8 (conferidos no exe JP, docs/JP983-PACOTES-PENDENTES.md)
        Assert.Equal([0x10B, 0x10E, 0x10F, 0x113, 0x115], new[] { 0x109, 0x10C, 0x10D, 0x111, 0x113 }.Select(JpTranslator.ServerId));
        Assert.Equal([0x126, 0x129, 0x12B], new[] { 0x11E, 0x121, 0x123 }.Select(JpTranslator.ServerId));
        Assert.Equal([-1, -1, 0x11A], new[] { 0x114, 0x11D, 0x116 }.Select(JpTranslator.ServerId));
    }

    [Fact]
    public async Task RefusedPersonalShopPublishStillAnswers()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _g = g;
        await g.SendAsync(new PacketWriter(0x7C).U32(0));                              // fora do lounge: recusa só com o código
        var r = await g.ExpectAsync(0xEB);                                             // 0xE9 do KR
        Assert.NotEqual(0u, r.U32());
        Assert.Equal(0, r.Remaining);
    }

    [Fact]
    public async Task BuysWithTheJpItemFormat()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        // 0x1D do JP: u8, u16 n, n × 37 (i32, u32 typeid, i16 dias, i16, u32 qtd, u32 pang, u32 cookie, 13), i32 cupom.
        // Dois itens no carrinho: o KR lia o 2º no lugar errado (16 bytes por item).
        static PacketWriter Item(PacketWriter w, uint tid) => w.I32(-1).U32(tid).I16(0).I16(0).U32(1).U32(0).U32(0).Zeros(13);
        await g.SendAsync(Item(Item(new PacketWriter(0x1D).U8(0).U16(2), 0x18000004), 0x18000005).I32(0));
        Assert.Equal(0u, (await g.ExpectAsync(0x68)).U32());
        var player = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.NotNull(player.FindType(0x18000004));
        Assert.NotNull(player.FindType(0x18000005));
    }

    [Fact]
    public void GameStatsGoFromTheJpLayoutToTheKrOne()
    {
        var p = new Player { Level = 3, Exp = 12, Pang = 999 };
        p.Stats = new PlayerStats { Drive = 40, Putt = 9, Longest = 251.5f, HoleInOne = 1, Bunker = 2, Fairway = 7, Albatross = 1, HoleIn = 3, PuttIn = 4, Quits = 5 };
        using var w = new PacketWriter(0).UserInfo(p);
        var kr = KrToJp.StatsToKr(w.Body[2..]);
        var s = System.Runtime.InteropServices.MemoryMarshal.Read<Protocol.KR645.sPangYaUserStatistics>(kr);
        Assert.Equal((40u, 9u, 251.5f), (s.dwDrive, s.dwPutt, s.fLongest));
        Assert.Equal(((ushort)1, (ushort)2, 7u, (ushort)1, 3u, 4u), (s.wHoleInOne, s.wBunker, s.dwFairway, s.wAlbatross, s.dwHoleIn, s.dwPuttIn));
        Assert.Equal((12u, (byte)3, 999L), (s.dwExp, s.Level, s.i64Pang));
        Assert.Equal(5u, s.dwNoMannerGameCount);
        Assert.Equal(5, BitConverter.ToInt32(KrToJp.StatsToJp(kr), 189));
    }

    [Fact]
    public async Task GrandPrixLobbyOpensAndClosesWithoutHanging()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(Jp983Client.Build(0x176, null));                            // Start Game -> Grand Prix
        var ids = new List<ushort>();
        while (true)
        {
            var (id, r) = await g.ReceiveAsync();
            ids.Add(id);
            if (id != 0x250) continue;
            Assert.Equal(16, r.Remaining);
            break;
        }
        Assert.DoesNotContain((ushort)0xF5, ids);                                      // o lobby do GP não manda o 0xF5
        Assert.Contains((ushort)0x47, ids);
        await g.SendAsync(Jp983Client.Build(0x179, null, 0x100u));                    // escolher um GP sem ticket: recusado
        Assert.Equal(Pangya.Domain.Rooms.GrandPrix.Ticket, (await g.ExpectAsync(0x253)).U32());
        await g.SendAsync(Jp983Client.Build(0x177, null));                            // sair do lobby do GP
        Assert.Equal(0u, (await g.ExpectAsync(0x251)).U32());
    }

    [Fact]
    public async Task JpOnlyRequestsGetAnAnswer()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x16E));                                   // presença diária
        Assert.Equal(25, (await g.ExpectAsync(0x248)).Remaining);
        await g.SendAsync(new PacketWriter(0x157).U32(1));                            // conquistas: sem conquistas, sem resposta
        await g.SendAsync(new PacketWriter(0x5C));                                    // abrir a loja: hora do servidor
        Assert.Equal(DateTime.Now.Year, (await g.ExpectAsync(0xBA)).U16());
        await g.SendAsync(new PacketWriter(0x164).U32(0x1A000001).U16(1).I32(1));     // oficina: recusa (material), sem travar
        Assert.Equal(0x5300201u, (await g.ExpectAsync(0x23D)).U32());                // material errado
    }

    [Fact]
    public async Task SpecialShuffleIsMode18InTheJpAnd14InTheKr()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 0u, 0u, (byte)30, (byte)18, (byte)18, (byte)0, (byte)0,
            0u, "shuffle", "", 0u));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        var room = enter.Bytes(KrToJp.RoomSize).ToArray();
        Assert.Equal(4, room[88]);                                                     // tipo de exibição: torneio
        Assert.Equal(18, room[196]);                                                   // modo real do JP
        foreach (var r in env.Game.World.Rooms.Rooms) Assert.Equal(14, (int)r.Settings.Mode);
    }

    /// <summary>Perfil (0x2F): cada pacote no tamanho que o exe lê (antes ia no layout do KR e o cliente caía).</summary>
    [Fact]
    public async Task ProfileUsesTheJpLayouts()
    {
        Assert.Equal([0x12D], Jp983Client.Blocks(0x157));                              // MemberInfo 297 + uid
        Assert.Equal([JpStructs.UserEquipSize], Jp983Client.Blocks(0x156));
        Assert.Equal([JpStructs.CharacterSize], Jp983Client.Blocks(0x15E));
        Assert.Equal([JpStructs.UserInfoSize], Jp983Client.Blocks(0x158));
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x2F).U32((uint)acc.Id).U8(5));
        Assert.Equal(1 + 4 + 2 + 0x12D + 4, (await g.ExpectAsync(0x157)).Remaining);
        Assert.Equal(1 + 4 + JpStructs.UserEquipSize, (await g.ExpectAsync(0x156)).Remaining);
        Assert.Equal(4 + JpStructs.CharacterSize, (await g.ExpectAsync(0x15E)).Remaining);
        var stats = await g.ExpectAsync(0x158);
        Assert.Equal(1 + 4 + JpStructs.UserInfoSize, stats.Remaining);
        var player = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        stats.Skip(5 + 74);                                                            // EXP e nível nos offsets 74 e 78 do UserInfo
        Assert.Equal((player.Exp, player.Level), (stats.I32(), (int)stats.U8()));
        Assert.Equal(1 + 4 + JpStructs.TrophySize, (await g.ExpectAsync(0x159)).Remaining);
        for (int i = 0; i < 2; i++)
        {
            var maps = await g.ExpectAsync(0x15C);
            maps.Skip(5);
            int n = maps.I32();
            maps.Skip(n * JpStructs.MapStatSize);
            Assert.Equal(0, maps.I32());
            Assert.Equal(0, maps.Remaining);
        }
        await g.ExpectAsync(0x15A);
        await g.ExpectAsync(0x15B);
        await g.ExpectAsync(0x257);
        Assert.Equal(1u, (await g.ExpectAsync(0x89)).U32());
    }

    /// <summary>
    /// Roupa com peça auxiliar (Part.iff SubPart): o cliente veste o Violet Shoes (CR) no slot 7 e a peça auxiliar
    /// 0x0838C205 no slot 6, as duas com o id do sapato. Antes o servidor recusava ("Failed to equip").
    /// </summary>
    [Fact]
    public async Task OutfitWithASubPartIsAccepted()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var ids = await env.S.Players.NewIdsAsync(2);
        var ch = new PlayerChanges();
        ch.Added.Add(new Item { Id = ids[0], TypeId = 0x0400000E });                  // Cecilia R
        ch.Added.Add(new Item { Id = ids[1], TypeId = 0x0838E00D });                  // Violet Shoes (CR)
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        await g.ExpectAsync(0x96);
        var c = new byte[JpStructs.CharacterSize];
        BitConverter.TryWriteBytes(c.AsSpan(0), 0x0400000E);
        BitConverter.TryWriteBytes(c.AsSpan(4), ids[0]);
        BitConverter.TryWriteBytes(c.AsSpan(12 + 6 * 4), 0x0838C205);                  // peça auxiliar no slot 6
        BitConverter.TryWriteBytes(c.AsSpan(12 + 7 * 4), 0x0838E00D);                  // sapato no slot 7
        BitConverter.TryWriteBytes(c.AsSpan(108 + 6 * 4), ids[1]);
        BitConverter.TryWriteBytes(c.AsSpan(108 + 7 * 4), ids[1]);
        await g.SendAsync(new PacketWriter(0x20).U8(0).Bytes(c));
        var r = await g.ExpectAsync(0x6B);
        Assert.Equal((4, 0), (r.U8(), r.U8()));
        var player = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal([0x0838C205, 0x0838E00D], player.Find(ids[0])!.IntArray("parts", 24)[6..8]);
    }

    /// <summary>Nível 70 é o último também no JP (0..70); o EXP continua acumulando nele e vai como está.</summary>
    [Fact]
    public void Level70KeepsItsExp()
    {
        var p = new Player { Level = 70, Exp = 5 };
        using var w = new PacketWriter(0).UserInfo(p);
        Assert.Equal((5, (byte)70), (BitConverter.ToInt32(w.Body[2..].ToArray(), 74), w.Body[2 + 78]));
    }

    /// <summary>Strings do cliente JP em Shift-JIS (cp932): o nome da sala em japonês chega certo ao Domain e volta igual.</summary>
    [Fact]
    public async Task JapaneseTextUsesShiftJis()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelAsync(env);
        await using var _ = g;
        const string title = "日本語の部屋ｶﾀｶﾅ";
        Pangya.Core.Text.Cp949.Ambient = Pangya.Core.Text.Cp949.Japanese;            // o cliente falso fala como o JP
        try
        {
            await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)4, (byte)0, (byte)1, (byte)0, (byte)0,
                0u, title, "", 0u));
            var enter = await g.ExpectAsync(0x49);
            Assert.Equal(0, enter.I16());
            Assert.Equal(title, Pangya.Core.Text.Cp949.Read(enter.Bytes(64)));
            Assert.Equal(title, Assert.Single(env.Game.World.Rooms.Rooms).Settings.Title);
        }
        finally { Pangya.Core.Text.Cp949.Ambient = null; }
    }

    /// <summary>Presença diária: 1ª entrada do dia conta e entrega; a 2ª não; no dia seguinte vem o "próximo" mostrado.</summary>
    [Fact]
    public async Task AttendanceCountsDaysAndGivesTheNextPrize()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        static (byte Registered, uint NowTid, uint NowQty, uint NextTid, uint NextQty, uint Days) Read(PacketReader r)
        {
            Assert.Equal(0, r.I32());
            return (r.U8(), r.U32(), r.U32(), r.U32(), r.U32(), r.U32());
        }
        await g.SendAsync(new PacketWriter(0x16E));
        var first = Read(await g.ExpectAsync(0x248));
        Assert.Equal((0, 1u), (first.Registered, first.Days));                         // dia novo: a janela aparece
        Assert.NotEqual(0u, first.NowTid);
        Assert.NotEqual(0u, first.NextTid);
        await g.ExpectAsync(0x33);                                                    // o prêmio vai por carta (carta nova)
        var (mails, _) = await env.S.Mail.ListAsync(acc.Id, 1, 20);
        var letter = Assert.Single(mails);
        Assert.Equal(((int)first.NowTid, (int)first.NowQty), (letter.Items[0].TypeId, letter.Items[0].Quantity));
        Assert.Equal((0x1A000264, 3), (letter.Items[1].TypeId, letter.Items[1].Quantity));   // + 3 tickets do Grand Prix
        await g.SendAsync(new PacketWriter(0x16F));
        Assert.Equal(1, Read(await g.ExpectAsync(0x249)).Registered);

        await g.SendAsync(new PacketWriter(0x16E));                                   // de novo hoje: nada
        var again = Read(await g.ExpectAsync(0x248));
        Assert.Equal((1, 1u, first.NextTid), (again.Registered, again.Days, again.NextTid));

        // virou o dia: o prêmio é o "próximo" de ontem
        var st = (await env.S.Attendance.LoadAsync(acc.Id))!;
        await env.S.Attendance.SaveAsync(acc.Id, st with { LastDay = st.LastDay.AddDays(-1) });
        await g.SendAsync(new PacketWriter(0x16E));
        var next = Read(await g.ExpectAsync(0x248));
        Assert.Equal((0, 2u, first.NextTid, first.NextQty), (next.Registered, next.Days, next.NowTid, next.NowQty));
    }
}
