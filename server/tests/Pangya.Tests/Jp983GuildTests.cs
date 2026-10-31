using System.Net;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Hosting.Server.Features;
using Microsoft.Extensions.DependencyInjection;
using Pangya.Core.Net;
using Pangya.Domain.Guilds;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;
using Pangya.Web;

namespace Pangya.Tests;

/// <summary>Guilda ("club") do JP 983 (SPEC-jp983-guilda.md §1, §3): chave web, o site e a guilda de volta no jogo.</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task ClubSiteCreatesAGuildThatTheGameSees()
    {
        await using var env = new JpEnv();
        await using var web = WebServer.Build(env.S, 0);
        await web.StartAsync();
        var baseUri = new Uri(web.Services.GetRequiredService<IServer>().Features.Get<IServerAddressesFeature>()!.Addresses.First());
        var (g, acc) = await InChannelWithAsync(env, 0);
        await using var _ = g;

        await g.SendAsync(new PacketWriter(0xFB));                                       // botão club: a chave web
        var r = await g.ExpectAsync(0x1AD);
        Assert.Equal(1u, r.U32());
        string key = r.Str();
        Assert.Equal(16, key.Length);
        await g.SendAsync(new PacketWriter(0xA1).U8(5));                                 // entrou no navegador: sem resposta

        var cookies = new CookieContainer();
        using var http = new HttpClient(new HttpClientHandler { CookieContainer = cookies }) { BaseAddress = baseUri };
        var bad = await http.GetStringAsync($"/on_client/club/login.aspx?ID={acc.Login}&KID=0123456789ABCDEF");
        Assert.Contains("Sessão inválida", bad);
        var home = await http.GetStringAsync($"/on_client/club/login.aspx?ID={acc.Login}&KID={key}");
        Assert.Contains("Criar um club", home);
        string name = "C" + Guid.NewGuid().ToString("N")[..8];
        var created = await (await http.PostAsync("/on_client/club/create",
            new FormUrlEncodedContent(new Dictionary<string, string> { ["name"] = name, ["text"] = "ola" }))).Content.ReadAsStringAsync();
        Assert.Contains(name, created);
        Assert.Contains("Mestre", created);
        Assert.Contains("ok", await http.GetStringAsync("/on_client/club/rss.aspx") is var rss && rss.Contains("<rss") ? "ok" : "");

        await g.SendAsync(new PacketWriter(0xA2));                                       // fechou o club: a guilda volta ao jogo
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        for (int i = 0; i < 50 && h.Player.Guild == null; i++) await Task.Delay(20);
        Assert.Equal((name, GuildClass.Master), (h.Player.Guild!.Name, h.Player.Guild.Class));
        using (var mi = new PacketWriter(0).MemberInfo(h.Player))
        {
            var b = mi.Body[2..];
            Assert.Equal((uint)h.Player.Guild.Id, BitConverter.ToUInt32(b.Slice(2 + 136, 4)));
            Assert.Equal(1, b[2 + 144]);                                                  // estado: bits 0–1 = mestre (gestão no cliente)
        }
        Assert.Equal(0x1A, GuildTag.MarkNo("g1a"));
        Assert.Equal(0, GuildTag.MarkNo(""));
    }
}
