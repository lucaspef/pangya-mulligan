using System.Text.Json.Nodes;
using Pangya.Core.Config;
using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Data;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Game;
using Pangya.Login;
using Pangya.Protocol.JP983;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Login JP + game server JP (porta e mundo próprios) até o canal; tamanhos das structs contra o exe 983.</summary>
[Collection("db")]
public partial class Jp983GameTests(DbFixture fx)
{
    static int Len(Func<PacketWriter, PacketWriter> write)
    {
        using var w = new PacketWriter(0);
        write(w);
        return w.BodyLength - 2;
    }

    /// <summary>Bytes de uma struct do KR, como o handler do KR manda.</summary>
    static byte[] Kr<T>(T v) where T : unmanaged
    {
        var b = new byte[System.Runtime.CompilerServices.Unsafe.SizeOf<T>()];
        System.Runtime.InteropServices.MemoryMarshal.Write(b, in v);
        return b;
    }

    static Player SamplePlayer()
    {
        var p = new Player { AccountId = 7, Login = "royal", Nickname = "Royal", Level = 5, Pang = 1234 };
        p.Add(new Item { Id = 1, TypeId = 0x04000000, Attrs = new JsonObject { ["hair"] = 1, ["parts"] = new JsonArray(1, 2, 3) } });
        p.Add(new Item { Id = 2, TypeId = 0x1C000000, Attrs = new JsonObject { ["level"] = 1 } });
        p.Add(new Item { Id = 3, TypeId = 0x10000000, Attrs = new JsonObject { ["pcl"] = new JsonArray(1, 1, 0, 0, 0) } });
        p.Add(new Item { Id = 4, TypeId = 0x40000000, ExpiresAt = DateTime.UtcNow.AddDays(7) });
        p.Add(new Item { Id = 5, TypeId = 0x14000001, Quantity = 30 });
        p.Equip = new Equipment { CharacterId = 1, CaddieId = 2, ClubSetId = 3, MascotId = 4 };
        p.Courses[3] = new CourseRecord { Holes = 18, TotalScore = -3, BestScore = -3, MaxPang = 500 };
        return p;
    }

    [Fact]
    public void StructSizesMatchWhatTheExeReads()
    {
        _ = fx;
        var p = SamplePlayer();
        Assert.Equal(JpStructs.MemberInfoSize, Len(w => w.MemberInfo(p)));
        Assert.Equal(JpStructs.UserInfoSize, Len(w => w.UserInfo(p)));
        Assert.Equal(JpStructs.TrophySize, Len(w => w.Trophies(p)));
        Assert.Equal(JpStructs.UserEquipSize, Len(w => w.UserEquip(p)));
        Assert.Equal(JpStructs.MapStatSize, Len(w => w.MapStat(p, 3)));
        Assert.Equal(JpStructs.MapStatSize, Len(w => w.MapStat(null, 3)));
        Assert.Equal(JpStructs.EquipedItemSize, Len(w => w.EquipedItems(p)));
        Assert.Equal(JpStructs.PlayerBlockSize, Len(w => w.PlayerBlock(p)));

        // conversões KR -> JP (os offsets de KrToJp valem para as structs geradas do KR)
        var ch = Kr(PlayerStructs.Character(p.Find(1)!));
        Assert.Equal(KrToJp.KrCharacterSize, ch.Length);
        Assert.Equal(JpStructs.CharacterSize, Len(w => w.Character(ch)));
        Assert.Equal(KrToJp.KrCharacterSize, KrToJp.CharacterToKr(new PacketWriter(0).Character(ch).Body[2..]).Length);
        var item = Kr(PlayerStructs.ItemInfo(p.Find(5)!));
        Assert.Equal(KrToJp.KrItemSize, item.Length);
        Assert.Equal(JpStructs.ItemSize, Len(w => w.Item(item)));
        using (var jw = new PacketWriter(0).Item(item))
        {
            var j = jw.Body[2..].ToArray();
            Assert.Equal(0x14000001u, BitConverter.ToUInt32(j, 4));
            Assert.Equal(-1, BitConverter.ToInt32(j, 8));                             // permanente
            Assert.Equal(30, BitConverter.ToInt16(j, 12));                            // quantidade
            Assert.Equal(2, j[40]);                                                   // tipo 2 = item válido (referência)
        }
        var mascot = Kr(PlayerStructs.Mascot(p.Find(4)));
        Assert.Equal(KrToJp.KrMascotSize, mascot.Length);
        Assert.Equal(JpStructs.MascotSize, Len(w => w.Mascot(mascot)));
        using (var mw = new PacketWriter(0).Mascot(mascot))
            Assert.Equal(p.Find(4)!.ExpiresAt == null ? 0 : 1, BitConverter.ToInt16(mw.Body[2..].ToArray(), 43));   // tipo: com prazo = 1
        var timed = p.Find(4)!.Clone();
        timed.ExpiresAt = DateTime.UtcNow.AddDays(30);
        using (var mw = new PacketWriter(0).Mascot(Kr(PlayerStructs.Mascot(timed))))
            Assert.Equal(1, BitConverter.ToInt16(mw.Body[2..].ToArray(), 43));
        timed.ExpiresAt = null;
        using (var mw = new PacketWriter(0).Mascot(Kr(PlayerStructs.Mascot(timed))))
            Assert.Equal(0, BitConverter.ToInt16(mw.Body[2..].ToArray(), 43));
        Assert.Equal(JpStructs.CaddieSize, Kr(PlayerStructs.Caddie(p.Find(2)!)).Length);
        Assert.Equal(KrToJp.KrChannelSize, Kr(new sChannelInfo()).Length);
        Assert.Equal(JpStructs.ChannelSize, Len(w => w.Channel(new byte[KrToJp.KrChannelSize])));
        var lobby = Kr(RoomPackets.BriefUser(p));
        Assert.Equal(KrToJp.KrLobbyUserSize, lobby.Length);
        Assert.Equal(KrToJp.LobbyUserSize, Len(w => w.LobbyUser(lobby)));
        Assert.Equal(KrToJp.KrRoomSize, Kr(new sRoomInfo()).Length);
        Assert.Equal(KrToJp.RoomSize, Len(w => w.Room(new byte[KrToJp.KrRoomSize])));

        // o que o exe lê em cada pacote (análise do binário)
        Assert.Equal(0x31C6, JpStructs.PlayerBlockSize);
        Assert.Contains(JpStructs.PlayerBlockSize, Jp983Client.Blocks(0x44));
        Assert.Equal([JpStructs.ChannelSize], Jp983Client.Blocks(0x4D));
        Assert.Equal([JpStructs.CharacterSize], Jp983Client.Blocks(0x70));
        Assert.Equal([JpStructs.CaddieSize], Jp983Client.Blocks(0x71));
        Assert.Equal([JpStructs.UserEquipSize], Jp983Client.Blocks(0x72));
        Assert.Equal([JpStructs.ItemSize], Jp983Client.Blocks(0x73));
        Assert.Equal(JpStructs.MascotSize, Jp983Client.Blocks(0xE1)[0]);
        Assert.Equal("D8", Jp983Client.Reads(0x96));
        Assert.Equal(KrToJp.LobbyUserSize, Jp983Client.Blocks(0x46)[0]);
        Assert.Equal(KrToJp.RoomSize, Jp983Client.Blocks(0x47)[0]);
        Assert.Equal(JpStructs.CharacterSize, Jp983Client.Blocks(0x6B)[0]);         // resultado de equipar
        Assert.Equal(JpStructs.MascotSize, Jp983Client.Blocks(0x6B)[3]);
    }

    [Fact]
    public void PlayerBlockCarriesIdentityLevelAndPang()
    {
        var p = SamplePlayer();
        using var w = new PacketWriter(0).PlayerBlock(p);
        var b = w.Body[2..].ToArray();
        Assert.Equal(0xFFFF, BitConverter.ToUInt16(b, 0));                          // fora de sala
        Assert.Equal("royal", Cp949.Read(b.AsSpan(2, 22)));
        Assert.Equal("Royal", Cp949.Read(b.AsSpan(24, 22)));
        Assert.Equal(7u, BitConverter.ToUInt32(b, JpStructs.MemberInfoSize));        // uid depois do MemberInfoEx
        int ui = JpStructs.MemberInfoSize + 4;
        Assert.Equal(5, b[ui + 78]);                                                  // nível
        Assert.Equal(1234ul, BitConverter.ToUInt64(b, ui + 79));                      // pang
        int maps = ui + JpStructs.UserInfoSize + JpStructs.TrophySize + JpStructs.UserEquipSize;
        Assert.Equal(3, b[maps + 3 * JpStructs.MapStatSize]);                         // curso 3 com recorde
        Assert.Equal(unchecked((byte)-3), b[maps + 3 * JpStructs.MapStatSize + 29]);
        Assert.Equal(127, b[maps + 4 * JpStructs.MapStatSize + 29]);                  // curso 4 sem recorde
    }

    [Fact]
    public void TypeIdsTheJpClientDoesNotHaveAreLeftOut()
    {
        // pangya_jp.iff mínimo: só o personagem 0x04000000 (registro: u32 ativo, u32 typeid)
        var path = Path.Combine(Path.GetTempPath(), $"jp{Guid.NewGuid():N}.iff");
        using (var zip = System.IO.Compression.ZipFile.Open(path, System.IO.Compression.ZipArchiveMode.Create))
        using (var s = zip.CreateEntry("Character.iff").Open())
            s.Write([1, 0, 0, 0, 13, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 4]);
        var data = Jp983GameData.Load(path)!;
        File.Delete(path);
        Assert.True(data.Exists(0x04000000));
        Assert.False(data.Exists(0x40000000));

        var p = SamplePlayer();
        using var w = new PacketWriter(0).UserEquip(p, data.Exists);
        var e = w.Body[2..].ToArray();
        Assert.Equal(0, BitConverter.ToInt32(e, 0));                                 // caddie desconhecido: desequipado
        Assert.Equal(1, BitConverter.ToInt32(e, 4));                                 // personagem conhecido
        Assert.Equal(0, BitConverter.ToInt32(e, 8));                                 // club set desconhecido
        Assert.Equal(0, BitConverter.ToInt32(e, 104));                               // mascote desconhecido
        using var c = new PacketWriter(0).Character(Kr(PlayerStructs.Character(p.Find(1)!)), data.Exists);
        Assert.Equal(0, BitConverter.ToInt32(c.Body[2..].ToArray(), 12));             // peça desconhecida: slot vazio
    }

    /// <summary>Servidores de login e game JP numa porta livre, com o mundo JP separado do KR.</summary>
    sealed class JpEnv : IAsyncDisposable
    {
        public readonly ServerServices S;
        public readonly TcpServer Login;
        public readonly JpGameServer Game;
        readonly CancellationTokenSource cts = new();
        readonly Task run;

        /// <param name="jpData">false: sem os dados do cliente JP (todo typeid passa, como com os dados do KR).</param>
        public JpEnv(bool jpData = true)
        {
            var cfg = TestEnv.Config;
            cfg.GameJp = new GameConfig { Id = 39302, Name = "Teste JP", Port = 0, Channels = [new ChannelConfig { Name = "Canal JP", MaxUsers = 10 }] };
            cfg.Data.JpIffPath = Path.Combine(TestEnv.Root, "data", jpData ? "pangya_jp.iff" : "nao-existe.iff");   // sem o arquivo: dados do KR
            S = new ServerServices(cfg);
            Login = LoginServer.Create(S, [0])[0];
            _ = Login.StartAsync(cts.Token);
            Game = new JpGameServer(S, 0);
            run = Game.RunAsync(cts.Token);
        }

        public async Task<(string Login, Domain.Accounts.Account Acc)> NewAccountAsync()
        {
            var login = "j" + Guid.NewGuid().ToString("N")[..12];
            var (_, acc) = await S.AccountService.RegisterAsync(login, "senha123", "t");
            await S.Accounts.SetNicknameAsync(acc!.Id, "Jp" + login[1..8]);
            await Game.Context.Kr.Players.CreateAsync(acc.Id, 0x04000000, 1, 2);
            return (login, acc);
        }

        public async Task<TestClient> ConnectGameAsync()
        {
            var g = await TestClient.ConnectAsync(Game.Tcp.Port);
            var (id, hello) = await g.ReceiveAsync();
            Assert.Equal(0x3F, id);                                                    // hello do JP
            Assert.Equal((1, 1), (hello.U8(), hello.U8()));
            g.Key = hello.U8();
            Assert.Equal("127.0.0.1", hello.Str());
            return g;
        }

        public async ValueTask DisposeAsync()
        {
            cts.Cancel();
            await run;
            await S.DisposeAsync();
        }
    }

    /// <summary>C->S 0x02 como o 983 monta (gameunit.cpp): id, uid, uid Ntreev, comando, chave, versão, versão de pacote, MAC, chave.</summary>
    static PacketWriter GameLogin(string login, long uid, string key) =>
        Jp983Client.Build(0x02, "FUN_00a51750", login, (uint)uid, (uint)uid, (ushort)0, key, Jp983.ClientVersion, 2017110200u,
            "00-11-22-33-44-55", key);

    [Fact]
    public void ServerListRowKeeps92BytesAndCarriesEventFields()
    {
        var info = new Domain.Servers.ServerInfo(1, Jp983.GameServerType, "Evento", "127.0.0.1", 2, 3, 4, 0x800, 54, -1, 7);
        using var w = Protocol.JP983.Login.JpLoginHandler.ServerList(2, [info]);
        var b = w.Body.ToArray();
        Assert.Equal(2 + 1 + 92, b.Length);                                           // id, n, 1 linha
        Assert.Equal(0x800u, BitConverter.ToUInt32(b, 3 + 0x4A));
        Assert.Equal(54u, BitConverter.ToUInt32(b, 3 + 0x52));
        Assert.Equal(uint.MaxValue, BitConverter.ToUInt32(b, 3 + 0x56));
        Assert.Equal(7, BitConverter.ToUInt16(b, 3 + 0x5A));
    }

    [Fact]
    public async Task LobbyServerListShowsJpServersWithEventFields()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        await env.S.Registry.HeartbeatAsync(new Domain.Servers.ServerInfo(39399, Jp983.GameServerType, "Evento", "127.0.0.1", 39399, 100, 1, 0x800, 54, -1, 7),
            TimeSpan.FromMinutes(1));
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, await env.S.Sessions.IssueGameLoginAsync(acc.Id)));
        await g.ExpectAsync(0x4D);
        await g.SendAsync(new PacketWriter(0x43));
        var list = await g.ExpectAsync(0x9F);
        int n = list.U8();
        var rows = Enumerable.Range(0, n).Select(_ => list.Struct<sGameServerInfo>()).ToList();
        await env.S.Registry.RemoveAsync(39399);
        var e = Assert.Single(rows, r => r.id == 39399);
        Assert.Equal((0x800u, 54u, uint.MaxValue, (ushort)7), (e.property, e.eventFlags, e.eventValue, e.iconIndex));
    }

    [Fact]
    public async Task FakeJpClientGoesFromLoginToChannel()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        for (int i = 0; i < 50 && (await env.S.Registry.ListAsync(Jp983.GameServerType)).All(s => s.Id != 39302); i++) await Task.Delay(100);

        // login server: senha -> chave -> lista só com o game server JP -> escolha
        await using var c = await TestClient.ConnectAsync(env.Login.Port);
        c.Key = (int)(await c.ReceiveAsync()).Body.U32();
        await c.SendAsync(Jp983LoginTests.JpLogin(login, "senha123"));
        var key = (await c.ExpectAsync(0x10)).Str();
        Assert.Equal(0, (await c.ExpectAsync(0x01)).U8());
        var list = await c.ExpectAsync(0x02);
        int n = list.U8(), port = 0;
        for (int i = 0; i < n; i++)
        {
            list.Skip(40);
            uint id = list.U32();
            list.Skip(8 + 18);
            int p = list.I32();
            list.Skip(4 + 4 + 2 + 8);
            if (id == 39302) port = p;
        }
        Assert.Equal(env.Game.Tcp.Port, port);
        await c.SendAsync(Jp983Client.Build(0x03, "FUN_00af4ce0", 39302u));
        var sel = await c.ExpectAsync(0x03);
        Assert.Equal((0, key), (sel.I32(), sel.Str()));

        // game server JP: hello -> login -> dados -> canais -> canal
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        var loading = await g.ExpectAsync(0x44);
        Assert.Equal((0xD3, 0), (loading.U8(), loading.U8()));
        var info = await g.ExpectAsync(0x44);
        Assert.Equal(0, info.U8());
        Assert.Equal(Jp983.ClientVersion, info.Str());
        var block = info.Bytes(JpStructs.PlayerBlockSize).ToArray();
        Assert.Equal(login, Cp949.Read(block.AsSpan(2, 22)));
        Assert.Equal((uint)acc.Id, BitConverter.ToUInt32(block, JpStructs.MemberInfoSize));
        Assert.Equal(DateTime.Now.Year, info.U16());                                 // hora do servidor
        info.Skip(14 + 2 + 6 + 4 + 8 + 4 + 4);
        Assert.Equal(0, info.Remaining);

        var channels = await g.ExpectAsync(0x4D);
        Assert.Equal(1, channels.U8());
        Assert.Equal("Canal JP", Cp949.Read(channels.Bytes(64)));
        channels.Skip(JpStructs.ChannelSize - 64);
        Assert.Equal(0, channels.Remaining);
        var chars = await g.ExpectAsync(0x70);
        int nc = chars.U16();
        Assert.Equal(nc, chars.U16());
        Assert.Equal(nc * JpStructs.CharacterSize, chars.Remaining);
        var firstChar = chars.Bytes(JpStructs.CharacterSize).ToArray();
        Assert.Equal(0x04000000u, BitConverter.ToUInt32(firstChar));
        var cad = await g.ExpectAsync(0x71);
        int ncad = cad.U16();
        cad.U16();
        Assert.Equal(ncad * JpStructs.CaddieSize, cad.Remaining);
        var items = await g.ExpectAsync(0x73);
        int ni = items.U16();
        Assert.Equal(ni, items.U16());
        Assert.Equal(ni * JpStructs.ItemSize, items.Remaining);
        var mascots = await g.ExpectAsync(0xE1);
        Assert.Equal(mascots.U8() * JpStructs.MascotSize, mascots.Remaining);
        var equip = await g.ExpectAsync(0x72);
        Assert.Equal(JpStructs.UserEquipSize, equip.Remaining);
        equip.Skip(8);
        int clubSet = equip.I32();
        Assert.Equal(8, (await g.ExpectAsync(0x96)).Remaining);                       // cookie

        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        Assert.Equal(1, (await g.ExpectAsync(0x4E)).U8());
        Assert.Equal(1, env.Game.World.OnlineCount);

        // My Room: trocar bola + club set (0x20 tipo 3) -> 0x6B 4, 3, u32 bola, i32 club set (o JP lê; o KR não manda)
        await g.SendAsync(Jp983Client.Build(0x20, "FUN_0060e600", (byte)3, 0x14000000u, (uint)clubSet));
        var eq = await g.ExpectAsync(0x6B);
        Assert.Equal((4, 3), (eq.U8(), eq.U8()));
        Assert.Equal((0x14000000u, clubSet), (eq.U32(), eq.I32()));
        // roupa do personagem (0x20 tipo 0 com CharacterInfo do JP) -> 0x6B com o personagem no formato JP
        await g.SendAsync(Jp983Client.Build(0x20, "FUN_00613590", (byte)0, firstChar));
        eq = await g.ExpectAsync(0x6B);
        Assert.Equal((4, 0), (eq.U8(), eq.U8()));
        Assert.Equal(firstChar, eq.Bytes(JpStructs.CharacterSize).ToArray());
        Assert.Equal(0, eq.Remaining);

        // Start Game: 0x81 entra na lista de salas -> 0xF5, 0x47 salas (vazia), 0x46 jogadores do lobby (eu)
        await g.SendAsync(Jp983Client.Build(0x81, "CLobbyMain::HandleMsg"));
        await g.ExpectAsync(0xF5);
        var rooms = await g.ExpectAsync(0x47);
        Assert.Equal(0, rooms.U8());
        var users = await g.ExpectAsync(0x46);
        Assert.Equal((1, 1), (users.U8(), users.U8()));
        Assert.Equal((uint)acc.Id, users.U32());
        users.Skip(4 + 2);
        Assert.StartsWith("Jp", Cp949.Read(users.Bytes(22)));
        Assert.Equal(KrToJp.LobbyUserSize - 32, users.Remaining);

        // criar sala (0x08 do JP: com flags natural/short game e artefato) -> 0x49 sala, 0x48 jogadores (348 + 513)
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)4, (byte)0, (byte)1, (byte)0, (byte)0,
            0u, "sala jp", "", 0u));
        var enter = await g.ExpectAsync(0x49);
        Assert.Equal(0, enter.I16());
        Assert.Equal("sala jp", Cp949.Read(enter.Bytes(64)));
        Assert.Equal(KrToJp.RoomSize - 64, enter.Remaining);
        var slots = await g.ExpectAsync(0x48);
        Assert.Equal((0, (ushort)0xFFFF, 1), (slots.U8(), slots.U16(), slots.U8()));
        Assert.Equal((uint)acc.Id, slots.U32());
        slots.Skip(KrToJp.SlotSize - 4);
        Assert.Equal(0x04000000u, slots.U32());                                      // personagem depois do jogador
        slots.Skip(JpStructs.CharacterSize - 4);
        Assert.Equal((0, 0), (slots.U8(), slots.Remaining));                         // u8 0 no fim

        // começar (0x0E) -> 0x76 jogadores da partida (bloco 0x31C6) e 0x52 mapa e buracos
        await g.SendAsync(Jp983Client.Build(0x0E, null, (uint)acc.Id));
        var players = await g.ExpectAsync(0x76);
        Assert.Equal((0, 1), (players.U8(), players.U8()));
        var gameBlock = players.Bytes(JpStructs.PlayerBlockSize).ToArray();
        Assert.Equal(login, Cp949.Read(gameBlock.AsSpan(2, 22)));
        players.Skip(16);
        Assert.Equal(players.U8() * 0x41, players.Remaining);
        var init = await g.ExpectAsync(0x52);
        // equipamento na sala (0x0C -> 0x4B): i32 erro 0, u8 tipo, u32 uid, personagem 513
        await g.SendAsync(Jp983Client.Build(0x0C, "FUN_00613590", (byte)4, BitConverter.ToUInt32(firstChar, 4)));
        var qe = await g.ExpectAsync(0x4B);
        Assert.Equal((0, 4, (uint)acc.Id), (qe.I32(), qe.U8(), qe.U32()));
        Assert.Equal(JpStructs.CharacterSize, qe.Remaining);
        Assert.Equal(0, init.U8());                                                   // mapa
        init.Skip(3 + 12);
        for (int i = 0; i < 18; i++) { init.U32(); Assert.InRange(init.U8(), 0, 2); Assert.Equal(0, init.U8()); init.U8(); }
        init.U32();
        Assert.Equal(18, init.Remaining);

        // buraco 1: dados do buraco (0x1A) e carregado (0x11) -> 0x53 começa
        await g.SendAsync(Jp983Client.Build(0x1A, null, (byte)1, new byte[0x19]));
        await g.SendAsync(Jp983Client.Build(0x11, null));
        await g.ExpectAsync(0x53);
        // tacada no formato do JP: u16 opção + 50 bytes (46 do KR + zona de impacto) + ClassicWind 8 + tempo 4
        // -> eco 0x55 u32 + os mesmos 62 (68 B com o id, como o 983 lê)
        var shot = new byte[JpTranslator.ShotSize + JpTranslator.ShotSyncSize];
        for (int i = 0; i < shot.Length; i++) shot[i] = (byte)(i + 1);
        await g.SendAsync(new PacketWriter(0x12).U16(0).Bytes(shot));
        var echo = await g.ExpectAsync(0x55);
        Assert.Equal((uint)acc.Id, echo.U32());
        Assert.Equal(shot, echo.Bytes(echo.Remaining).ToArray());
    }

    /// <summary>
    /// Inventário grande (a conta de teste do usuário tem ~5000 itens): o login inteiro tem de caber no buffer de recepção
    /// do cliente JP (0x3FFFC, networksystem 0x00A4D07F), senão o cliente lança exceção e cai. Com LZO cabe com folga.
    /// </summary>
    [Fact]
    public async Task BigInventoryLoginFitsTheClientReceiveBuffer()
    {
        await using var env = new JpEnv(jpData: false);                                // typeids inventados: sem filtro
        var (login, acc) = await env.NewAccountAsync();
        var ids = await env.S.Players.NewIdsAsync(5000);
        var ch = new PlayerChanges();
        for (int i = 0; i < ids.Length; i++) ch.Added.Add(new Item { Id = ids[i], TypeId = 0x08000400 + i % 2000 });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);

        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        int items = 0;
        while (true)
        {
            var (id, r) = await g.ReceiveAsync();
            if (id == 0x73) { items += r.U16(); r.U16(); }
            if (id == 0x137) break;                                                   // último do login
        }
        Assert.True(items >= 5000, $"{items} itens");                                 // + os iniciais do jogador novo
        Assert.True(g.WireBytes < 0x3FFFC / 2, $"login com {g.WireBytes} bytes no fio");
    }

    [Fact]
    public async Task JpGameRejectsWrongKey()
    {
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, "0123456789ABCDEF"));
        var r = await g.ExpectAsync(0x44);
        Assert.Equal(1, r.U8());                                                      // falha de login
        Assert.True(await g.IsClosedByServerAsync());
        Assert.Equal(0, env.Game.World.OnlineCount);
    }
}
