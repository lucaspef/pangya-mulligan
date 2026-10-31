using System.Net;
using System.Text;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Pangya.Core.Config;
using Pangya.Core.Logging;

namespace Pangya.Web;

/// <summary>
/// Web do jogo (Web.GamePort, 30091 por padrão): o que o cliente JP baixa antes da tela de login.
/// Não usa banco e não tem dados do jogo: entrega o que o usuário gerou em Web.GameDir.
/// - GET /patch/updatelist: gerado por "updlist --generate" (client/tools/run-local.ps1); sem ele, 404 e o jogo não abre;
/// - GET /Translation/Read.aspx: arquivo Translation/Read.aspx, se existir; sem ele, tabela vazia (textos do servidor em branco).
/// O arquivo é lido a cada pedido: refazer o updatelist não exige reiniciar.
/// A resposta sempre leva Content-Length, nunca chunked: o wininet do cliente lê com buffer do tamanho anunciado.
/// </summary>
public static class GameWeb
{
    public const string UpdateListPath = "/patch/updatelist";
    public const string TranslationPath = "/Translation/Read.aspx";

    /// <summary>Base64 de uma tabela sem textos: GameStringManager aceita e o Ready segue sem "string load failed.".</summary>
    public static readonly byte[] EmptyTranslation = Encoding.ASCII.GetBytes(Convert.ToBase64String(Encoding.ASCII.GetBytes("<TEXT></TEXT>")));

    public static WebApplication Build(PangyaConfig cfg, int? portOverride = null)
    {
        string dir = Path.GetFullPath(cfg.Web.GameDir);
        var builder = WebApplication.CreateSlimBuilder();
        builder.Logging.ClearProviders();
        builder.WebHost.ConfigureKestrel(k =>
        {
            k.Listen(IPAddress.Parse(cfg.Web.GameBindIp ?? cfg.Network.BindIp), portOverride ?? cfg.Web.GamePort);
            k.AddServerHeader = false;
        });
        var app = builder.Build();
        app.MapMethods(UpdateListPath, ["GET", "HEAD"], (Delegate)((HttpContext ctx) => Serve(ctx, Path.Combine(dir, "patch", "updatelist"), null)));
        app.MapMethods(TranslationPath, ["GET", "HEAD"], (Delegate)((HttpContext ctx) => Serve(ctx, Path.Combine(dir, "Translation", "Read.aspx"), EmptyTranslation)));
        return app;
    }

    static async Task<IResult> Serve(HttpContext ctx, string path, byte[]? fallback)
    {
        ctx.Response.Headers.Connection = "close";
        byte[]? body = fallback;
        if (File.Exists(path)) body = await File.ReadAllBytesAsync(path);
        Log.Info($"WEB-JOGO {ctx.Request.Method} {ctx.Request.Path} ip={ctx.Connection.RemoteIpAddress}: " +
                 (body == null ? "404, gere o arquivo: " + path : $"{body.Length} bytes" + (ReferenceEquals(body, fallback) ? " (tabela vazia)" : "")));
        return body == null ? Results.NotFound() : Results.Bytes(body, "application/octet-stream");
    }

    public static async Task RunAsync(PangyaConfig cfg, CancellationToken ct)
    {
        await using var app = Build(cfg);
        Log.Info($"WEB-JOGO: escutando em {cfg.Web.GameBindIp ?? cfg.Network.BindIp}:{cfg.Web.GamePort}, pasta {Path.GetFullPath(cfg.Web.GameDir)}");
        await app.StartAsync(ct);
        try { await Task.Delay(Timeout.Infinite, ct); }
        catch (OperationCanceledException) { }
        await app.StopAsync();
    }
}
