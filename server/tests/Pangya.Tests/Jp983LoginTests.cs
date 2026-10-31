using Pangya.Core.Net;
using Pangya.Domain.Accounts;
using Pangya.Domain.Players;
using Pangya.Domain.Servers;
using Pangya.Login;

namespace Pangya.Tests;

/// <summary>Login do cliente JP 983 na mesma porta do KR 645 (LoginRouter + JpLoginHandler).</summary>
[Collection("db")]
public class Jp983LoginTests
{
    static string NewLogin() => "j" + Guid.NewGuid().ToString("N")[..12];

    /// <summary>Pedido de login do JP: str id, str senha, u8 n, n × 8 bytes, str MAC.</summary>
    internal static PacketWriter JpLogin(string id, string pwd) =>
        new PacketWriter(0x01).Str(id).Str(pwd).U8(2).U32(1).U32(2).U32(3).U32(4).Str("00-11-22-33-44-55");

    [Fact]
    public void RouterTellsTheVersionsApart()
    {
        static PacketReader Read(PacketWriter w)
        {
            var body = w.Body.ToArray();
            var r = new PacketReader();
            r.Reset(body, 0, body.Length);
            return r;
        }
        var kr = Read(new PacketWriter(0x01).Str("royal").Str("0123456789ABCDEF").U32(2).U8(0).U32(100001).U64(0x7fffffffffffffff).U8(0));
        var jp = Read(JpLogin("royal", "123456"));
        Assert.False(LoginRouter.IsJp(kr));
        Assert.True(LoginRouter.IsJp(jp));
        Assert.Equal("royal", kr.Str());                                    // olhar não consome o pacote
        var (id, pwd, mac) = Protocol.JP983.Login.JpLoginHandler.ReadLogin(jp);
        Assert.Equal(("royal", "123456", "00-11-22-33-44-55"), (id, pwd, mac));
    }

    /// <summary>Voltar do jogo para a lista: o JP reconecta e manda 0x0B str id, u32 uid, str chave.</summary>
    [Fact]
    public async Task JpReloginWithTheGameKeyGetsTheServerList()
    {
        await using var env = await Phase2Env.StartAsync();
        var login = NewLogin();
        var (_, acc) = await env.S.AccountService.RegisterAsync(login, "senha123", "t");
        Assert.True(await env.S.Accounts.SetNicknameAsync(acc!.Id, "Jp" + login[1..8]));
        await env.S.Players.CreateAsync(acc.Id, new NewPlayer(0, 0, []));
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);

        await using (var bad = await env.ConnectLoginAsync())
        {
            await bad.SendAsync(new PacketWriter(0x0B).Str(login).U32((uint)acc.Id).Str("0123456789ABCDEF"));
            Assert.Equal(2, (await bad.ExpectAsync(0x01)).U8());                    // chave errada: id inválido
        }
        await using var c = await env.ConnectLoginAsync();
        await c.SendAsync(new PacketWriter(0x0B).Str(login).U32((uint)acc.Id).Str(key));
        Assert.Equal(key, (await c.ExpectAsync(0x10)).Str());
        await c.ExpectAsync(0x02);
        await c.ExpectAsync(0x09);
        await c.SendAsync(new PacketWriter(0x03).U32(20202));                   // escolhe o servidor de novo
        var g = await c.ExpectAsync(0x03);
        Assert.Equal((0, key), (g.I32(), g.Str()));
    }

    /// <summary>Conta nova pelo 983 (referência FIRST_LOGIN/FIRST_SET): nickname e personagem antes da lista.</summary>
    [Fact]
    public async Task JpNewAccountCreatesNicknameAndCharacter()
    {
        await using var env = await Phase2Env.StartAsync();
        var login = NewLogin();
        var (_, acc) = await env.S.AccountService.RegisterAsync(login, "senha123", "t");
        string nick = "Nj" + login[1..9];

        await using var c = await env.ConnectLoginAsync();
        await c.SendAsync(JpLogin(login, "senha123"));
        var first = await c.ExpectAsync(0x0F);                                   // primeiro acesso
        Assert.Equal((1, login), (first.U8(), first.Str()));
        var d8 = await c.ExpectAsync(0x01);
        Assert.Equal(0xD8, d8.U8());                                             // janela do nickname
        await c.SendAsync(new PacketWriter(0x07).Str(login));                    // igual ao id: recusado
        Assert.NotEqual(0u, (await c.ExpectAsync(0x0E)).U32());
        await c.SendAsync(new PacketWriter(0x07).Str(nick));
        var ok = await c.ExpectAsync(0x0E);
        Assert.Equal((0u, nick), (ok.U32(), ok.Str()));
        await c.SendAsync(new PacketWriter(0x06).Str(nick));
        await c.ExpectAsync(0x0F);
        Assert.Equal(0xD9, (await c.ExpectAsync(0x01)).U8());                    // escolha do personagem
        await c.SendAsync(new PacketWriter(0x08).U32(0x04000000).U8(1).U8(0));
        await c.ExpectAsync(0x11);
        var key = (await c.ExpectAsync(0x10)).Str();
        Assert.Equal(acc!.Id, await env.S.Sessions.ValidateGameLoginAsync(key));
        Assert.Equal(0, (await c.ExpectAsync(0x01)).U8());                       // login completo
        var saved = (await env.S.Accounts.FindByIdAsync(acc.Id))!;
        Assert.Equal(nick, saved.Nickname);
        Assert.NotNull((await env.S.Players.LoadAsync(acc.Id))?.Character);
    }

    [Fact]
    public async Task JpLoginWithPasswordReachesServerList()
    {
        await using var env = await Phase2Env.StartAsync();
        var login = NewLogin();
        var (_, acc) = await env.S.AccountService.RegisterAsync(login, "senha123", "t");
        Assert.True(await env.S.Accounts.SetNicknameAsync(acc!.Id, "Jp" + login[1..8]));
        await env.S.Players.CreateAsync(acc.Id, new NewPlayer(0, 0, []));
        await env.S.Registry.HeartbeatAsync(new ServerInfo(20202, "game-jp", "Servidor JP", "127.0.0.1", 20202, 3000, 3, 0), TimeSpan.FromMinutes(1));
        await env.S.Registry.HeartbeatAsync(new ServerInfo(20203, "game", "Servidor KR", "127.0.0.1", 20203, 3000, 3, 0), TimeSpan.FromMinutes(1));

        await using var c = await env.ConnectLoginAsync();
        await c.SendAsync(JpLogin(login, "senha123"));
        var key = (await c.ExpectAsync(0x10)).Str();
        Assert.Equal(acc.Id, await env.S.Sessions.ValidateGameLoginAsync(key));
        var r = await c.ExpectAsync(0x01);
        Assert.Equal(0, r.U8());
        Assert.Equal(login, r.Str());
        Assert.Equal((uint)acc.Id, r.U32());
        r.U32(); r.U8(); r.U8(); r.U32(); r.U32();                           // como o 983 lê (005a2620): u8, u8, u32, u32
        Assert.Equal("", r.Str());
        Assert.Equal(key, r.Str());
        Assert.Equal(0ul, r.U64());
        Assert.StartsWith("Jp", r.Str());
        var list = await c.ExpectAsync(0x02);
        int n = list.U8();
        bool found = false, kr = false;
        for (int i = 0; i < n; i++)
        {
            var name = Core.Text.Cp949.Read(list.Bytes(40));
            uint id = list.U32();
            list.Skip(4 + 4 + 18 + 4 + 4 + 4 + 2 + 8);
            found |= id == 20202 && name == "Servidor JP";
            kr |= id == 20203;
        }
        Assert.True(found);
        Assert.False(kr);                                                   // o game server do KR não aparece para o JP
        Assert.Equal(0, (await c.ExpectAsync(0x09)).U8());
        Assert.Equal(9 * 64, (await c.ExpectAsync(0x06)).Remaining);

        await c.SendAsync(new PacketWriter(0x03).U32(20202));               // escolheu o servidor
        var g = await c.ExpectAsync(0x03);
        Assert.Equal((0, key), (g.I32(), g.Str()));
    }

    [Fact]
    public async Task JpWrongPasswordAndUnknownId()
    {
        await using var env = await Phase2Env.StartAsync();
        var login = NewLogin();
        await env.S.AccountService.RegisterAsync(login, "senha123", "t");
        await using var c = await env.ConnectLoginAsync();
        await c.SendAsync(JpLogin(login, "errada!!"));
        Assert.Equal(6, (await c.ExpectAsync(0x01)).U8());                 // ID ou senha errados
        await using var c2 = await env.ConnectLoginAsync();
        await c2.SendAsync(JpLogin(NewLogin(), "senha123"));
        Assert.Equal(2, (await c2.ExpectAsync(0x01)).U8());                // ID inexistente
    }
}
