using System.Net;
using System.Net.Sockets;
using System.Text;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Hosting.Server.Features;
using Microsoft.Extensions.DependencyInjection;
using Pangya.Core.Config;
using Pangya.Web;
using Xunit;

namespace Pangya.Tests;

/// <summary>Web do jogo: resposta como o wininet do cliente precisa (Content-Length, sem chunked), sem banco.</summary>
public class GameWebTests : IAsyncLifetime
{
    readonly string dir = Directory.CreateTempSubdirectory("gameweb").FullName;
    WebApplication app = null!;
    int port;

    public async Task InitializeAsync()
    {
        var cfg = new PangyaConfig();
        cfg.Web.GameDir = dir;
        cfg.Web.GameBindIp = "127.0.0.1";
        app = GameWeb.Build(cfg, 0);
        await app.StartAsync();
        port = new Uri(app.Services.GetRequiredService<IServer>().Features.Get<IServerAddressesFeature>()!.Addresses.First()).Port;
    }

    public async Task DisposeAsync()
    {
        await app.DisposeAsync();
        Directory.Delete(dir, true);
    }

    async Task<(string Head, byte[] Body)> RequestAsync(string method, string path)
    {
        using var tcp = new TcpClient();
        await tcp.ConnectAsync(IPAddress.Loopback, port);
        var ns = tcp.GetStream();
        await ns.WriteAsync(Encoding.ASCII.GetBytes($"{method} {path} HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n"));
        using var ms = new MemoryStream();
        await ns.CopyToAsync(ms);                                          // Connection: close encerra o fluxo
        byte[] all = ms.ToArray();
        int end = all.AsSpan().IndexOf("\r\n\r\n"u8) + 4;
        return (Encoding.ASCII.GetString(all, 0, end), all[end..]);
    }

    void Put(string rel, byte[] data)
    {
        string path = Path.Combine(dir, rel);
        Directory.CreateDirectory(Path.GetDirectoryName(path)!);
        File.WriteAllBytes(path, data);
    }

    [Fact]
    public async Task Updatelist_ausente_da_404()
    {
        var (head, _) = await RequestAsync("GET", GameWeb.UpdateListPath);
        Assert.StartsWith("HTTP/1.1 404", head);
    }

    [Fact]
    public async Task Updatelist_vem_inteiro_com_content_length_e_sem_chunked()
    {
        byte[] data = [0, 1, 2, 3, 0xFF, 0, 0x80, 7];                       // binário: tem zero no meio
        Put("patch/updatelist", data);
        var (head, body) = await RequestAsync("GET", GameWeb.UpdateListPath);
        Assert.StartsWith("HTTP/1.1 200", head);
        Assert.Contains("Content-Length: 8", head);
        Assert.DoesNotContain("chunked", head, StringComparison.OrdinalIgnoreCase);
        Assert.Equal(data, body);
    }

    [Fact]
    public async Task Refazer_o_updatelist_nao_exige_reiniciar()
    {
        Put("patch/updatelist", [1]);
        await RequestAsync("GET", GameWeb.UpdateListPath);
        Put("patch/updatelist", [2, 3]);
        Assert.Equal(new byte[] { 2, 3 }, (await RequestAsync("GET", GameWeb.UpdateListPath)).Body);
    }

    [Fact]
    public async Task Translation_sem_arquivo_e_tabela_vazia_valida()
    {
        var (head, body) = await RequestAsync("GET", GameWeb.TranslationPath);
        Assert.StartsWith("HTTP/1.1 200", head);
        Assert.Equal("<TEXT></TEXT>", Encoding.ASCII.GetString(Convert.FromBase64String(Encoding.ASCII.GetString(body))));
    }

    [Fact]
    public async Task Translation_do_usuario_tem_prioridade()
    {
        Put("Translation/Read.aspx", "QUJD"u8.ToArray());
        Assert.Equal("QUJD"u8.ToArray(), (await RequestAsync("GET", GameWeb.TranslationPath)).Body);
    }

    [Fact]
    public async Task Head_anuncia_o_tamanho_sem_corpo()
    {
        Put("patch/updatelist", new byte[16]);
        var (head, body) = await RequestAsync("HEAD", GameWeb.UpdateListPath);
        Assert.Contains("Content-Length: 16", head);
        Assert.Empty(body);
    }

    [Fact]
    public async Task Nada_fora_das_duas_rotas()
    {
        Put("segredo.txt", [1]);
        Assert.StartsWith("HTTP/1.1 404", (await RequestAsync("GET", "/segredo.txt")).Head);
        Assert.StartsWith("HTTP/1.1 404", (await RequestAsync("GET", "/patch/../segredo.txt")).Head);
    }
}
