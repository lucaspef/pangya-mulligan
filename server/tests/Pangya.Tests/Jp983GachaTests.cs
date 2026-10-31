using System.Net;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Hosting.Server.Features;
using Microsoft.Extensions.DependencyInjection;
using Pangya.Core.Net;
using Pangya.Domain.Shop;
using Pangya.Web;

namespace Pangya.Tests;

/// <summary>Gacha do JP (SPEC-jp983-eventos-drops.md §6): 0x102 no login, jogada pelo site, itens ao fechar (0x9E).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task GachaSpinsOnTheSiteAndDeliversOnClose()
    {
        await using var env = new JpEnv();
        await using var web = WebServer.Build(env.S, 0);
        await web.StartAsync();
        var baseUri = new Uri(web.Services.GetRequiredService<IServer>().Features.Get<IServerAddressesFeature>()!.Addresses.First());
        var (g, acc) = await InChannelWithAsync(env, 0, (GachaDesk.Ticket, 2));
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0xFB));
        var key = (await g.ExpectAsync(0x1AD)).Let(r => { r.U32(); return r.Str(); });

        using var http = new HttpClient(new HttpClientHandler { CookieContainer = new CookieContainer() }) { BaseAddress = baseUri };
        var home = await http.GetStringAsync($"/on_client/gacha/login.aspx?ID={acc.Login}&KID={key}");
        Assert.Contains("Gacha Ticket: <b>2</b>", home);
        var spun = await (await http.PostAsync("/on_client/gacha/spin", new FormUrlEncodedContent(new Dictionary<string, string> { ["kind"] = "ticket" })))
            .Content.ReadAsStringAsync();
        Assert.Contains("Você ganhou", spun);
        Assert.Contains("Gacha Ticket: <b>1</b>", spun);

        await g.SendAsync(new PacketWriter(0x9E));                                     // fechou a página
        var items = await g.ExpectAsync(0x216);
        items.U32();
        Assert.Equal(2u, items.U32());                                                  // o ticket gasto e o prêmio
        var coupons = await g.ExpectAsync(0x102);
        Assert.Equal((1u, 0u), (coupons.U32(), coupons.U32()));
    }
}
