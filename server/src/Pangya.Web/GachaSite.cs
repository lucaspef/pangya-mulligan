using System.Security;
using System.Text;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Http;
using Pangya.Core.Logging;
using Pangya.Data;
using Pangya.Domain.Shop;

namespace Pangya.Web;

/// <summary>
/// Gacha do cliente JP 983 (docs/server/protocolo/SPEC-jp983-eventos-drops.md §6): o jogo abre
/// /on_client/gacha/login.aspx?ID=&lt;login&gt;&amp;KID=&lt;chave do 0x1AD&gt; no navegador interno. As jogadas usam o jogador
/// que está online no game-jp (GachaDesk); o jogo recebe os itens quando a página fecha (0x9E).
/// </summary>
public sealed class GachaSite(ServerServices services)
{
    const string Base = "/on_client/gacha", Cookie = "gacha";

    public void Map(WebApplication app)
    {
        app.MapGet(Base + "/login.aspx", async (HttpContext ctx) =>
        {
            string login = ctx.Request.Query["ID"].ToString(), key = ctx.Request.Query["KID"].ToString();
            var acc = await AccountAsync(key);
            if (acc == null || !string.Equals(acc.Login, login, StringComparison.OrdinalIgnoreCase))
                return Page("<p>Sessão inválida. Feche esta janela e abra o gacha de novo.</p>");
            ctx.Response.Cookies.Append(Cookie, key, new CookieOptions { HttpOnly = true, Path = Base });
            return Results.Redirect(Base + "/home");
        });
        app.MapGet(Base + "/home", async (HttpContext ctx) => { return await HomeAsync(ctx, null); });
        app.MapPost(Base + "/spin", async (HttpContext ctx) =>
        {
            if (await AccountAsync(ctx.Request.Cookies[Cookie] ?? "") is not { } acc) return Page("<p>A sessão expirou.</p>");
            var form = ctx.Request.HasFormContentType ? await ctx.Request.ReadFormAsync() : null;
            string kind = form?["kind"].ToString() ?? "ticket";
            var (error, prizes) = await services.Gacha.SpinAsync(acc.Id, kind);
            var sb = new StringBuilder();
            if (error != null) sb.Append(E(error));
            else
            {
                sb.Append("Você ganhou: ");
                for (int i = 0; i < prizes.Count; i++) sb.Append(i > 0 ? ", " : "").Append(prizes[i].TypeId.ToString("X8")).Append(" ×").Append(prizes[i].Quantity);
                sb.Append(". Os itens aparecem no jogo quando você fechar esta janela.");
                Log.Info($"WEB gacha: {acc.Login} {kind} -> {prizes.Count} prêmio(s)");
            }
            return await HomeAsync(ctx, sb.ToString());
        });
    }

    async Task<Domain.Accounts.Account?> AccountAsync(string key) =>
        await services.Sessions.ValidateClubWebAsync(key) is { } id ? await services.Accounts.FindByIdAsync(id) : null;

    async Task<IResult> HomeAsync(HttpContext ctx, string? message)
    {
        if (await AccountAsync(ctx.Request.Cookies[Cookie] ?? "") is not { } acc) return Page("<p>A sessão expirou.</p>");
        var sb = new StringBuilder("<h2>Gacha</h2>");
        if (message != null) sb.Append("<p class=msg>").Append(message).Append("</p>");
        if (services.Gacha.Online(acc.Id) is not { } p)
            return Page(sb.Append("<p>Entre no jogo para usar o gacha.</p>").ToString());
        var (tickets, partials, plus5, plus11) = GachaDesk.Counts(p);
        sb.Append("<p>Gacha Ticket: <b>").Append(tickets).Append("</b> · Partial: <b>").Append(partials).Append("</b> · (+5): <b>")
          .Append(plus5).Append("</b> · (+11): <b>").Append(plus11).Append("</b></p>");
        sb.Append(Button("ticket", "Jogar (1 Gacha Ticket)")).Append(Button("partial", "Jogar (10 Partial)"))
          .Append(Button("plus5", "5 prêmios (Gacha Ticket +5)")).Append(Button("plus11", "11 prêmios (Gacha Ticket +11)"));
        int total = 0;
        foreach (var it in services.Gacha.Config.Items) total += Math.Max(1, it.Weight);
        sb.Append("<h3>Prêmios</h3><table border=1 cellpadding=3><tr><th>Item</th><th>Qtd</th><th>Chance</th></tr>");
        foreach (var it in services.Gacha.Config.Items)
            sb.Append("<tr><td>").Append(it.TypeId.ToString("X8")).Append("</td><td>").Append(it.Quantity).Append(it.Days > 0 ? $" ({it.Days} dias)" : "")
              .Append("</td><td>").Append((100.0 * Math.Max(1, it.Weight) / Math.Max(1, total)).ToString("0.0")).Append(" %</td></tr>");
        return Page(sb.Append("</table>").ToString());
    }

    static string Button(string kind, string label) =>
        $"<form method=post action=\"{Base}/spin\" style=\"display:inline\"><input type=hidden name=kind value={kind}><input type=submit value=\"{E(label)}\"></form> ";

    static string E(string s) => SecurityElement.Escape(s) ?? "";

    static IResult Page(string body) => Results.Content(
        "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>Gacha</title>" +
        "<style>body{font-family:Tahoma,Arial,sans-serif;font-size:13px;background:#fff6e8;color:#321;margin:12px}" +
        "h2{font-size:18px}table{border-collapse:collapse;background:#fff}.msg{background:#fff0b0;padding:6px;border:1px solid #db3}</style>" +
        "</head><body>" + body + "</body></html>", "text/html; charset=utf-8");
}
