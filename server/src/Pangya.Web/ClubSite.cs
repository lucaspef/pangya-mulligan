using System.Security;
using System.Text;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Http;
using Pangya.Core.Logging;
using Pangya.Data;
using Pangya.Domain.Guilds;

namespace Pangya.Web;

/// <summary>
/// Site da guilda ("club") do cliente JP 983 (docs/server/protocolo/SPEC-jp983-guilda.md §1): o jogo abre
/// /on_client/club/login.aspx?ID=&lt;login&gt;&amp;KID=&lt;chave do 0x1AD&gt; no navegador interno (800×600) e, no login com
/// guilda, lê /on_client/club/rss.aspx. As páginas usam as regras do GuildService (as mesmas do KR). HTML simples: o
/// navegador do jogo é um IE antigo.
/// </summary>
public sealed class ClubSite(ServerServices services, string markDir)
{
    const string Base = "/on_client/club", Cookie = "club";
    readonly GuildService guilds = new(services.Guilds, services.GuildInvites);

    public void Map(WebApplication app)
    {
        app.MapGet(Base + "/login.aspx", async (HttpContext ctx) =>
        {
            string login = ctx.Request.Query["ID"].ToString(), key = ctx.Request.Query["KID"].ToString();
            var acc = await AccountAsync(key);
            if (acc == null || !string.Equals(acc.Login, login, StringComparison.OrdinalIgnoreCase))
                return Page("Club", "<p>Sessão inválida. Feche esta janela e abra o club de novo.</p>");
            ctx.Response.Cookies.Append(Cookie, key, new CookieOptions { HttpOnly = true, Path = Base });
            Log.Info($"WEB club: {acc.Login} entrou");
            return Results.Redirect(Base + "/home");
        });
        app.MapGet(Base + "/rss.aspx", () => Results.Content(
            "<?xml version=\"1.0\" encoding=\"utf-8\"?><rss version=\"2.0\"><channel><title>Club</title><link>" + Base +
            "</link><description></description></channel></rss>", "text/xml; charset=utf-8"));
        app.MapGet(Base, () => Results.Redirect(Base + "/home"));
        app.MapGet(Base + "/home", async (HttpContext ctx) => { return await HomeAsync(ctx, null); });   // corpo com return: senão vira RequestDelegate
        app.MapGet(Base + "/list", async (HttpContext ctx) =>
        {
            if (await MeAsync(ctx) is not { } me) return Expired();
            int.TryParse(ctx.Request.Query["page"], out int page);
            return await ListAsync(me, Math.Max(1, page), ctx.Request.Query["q"].ToString());
        });
        app.MapPost(Base + "/{action}", async (HttpContext ctx, string action) =>
        {
            if (await MeAsync(ctx) is not { } me) return Expired();
            if (action == "emblem" && ctx.Features.Get<Microsoft.AspNetCore.Http.Features.IHttpMaxRequestBodySizeFeature>() is { IsReadOnly: false } limit)
                limit.MaxRequestBodySize = GuildService.EmblemMaxBytes + 4 * 1024;      // o PNG + o envelope do multipart
            var form = ctx.Request.HasFormContentType ? await ctx.Request.ReadFormAsync() : null;
            int.TryParse(form?["guild"], out int gid);
            long.TryParse(form?["target"], out long target);
            string text = form?["text"].ToString() ?? "";
            GuildCode code;
            switch (action)
            {
                case "create": (code, _) = await guilds.CreateWebAsync(me.Id, (form?["name"].ToString() ?? "").Trim(), text); break;
                case "join": code = await guilds.RequestJoinAsync(me.Id, gid, text); break;
                case "withdraw": code = await guilds.WithdrawAsync(me.Id, gid); break;
                case "approve": code = await guilds.AnswerRequestAsync(me.Id, gid, target, true); break;
                case "reject": code = await guilds.AnswerRequestAsync(me.Id, gid, target, false); break;
                case "kick": code = await guilds.KickAsync(me.Id, gid, target); break;
                case "promote": (code, _) = await guilds.ChangeClassAsync(me.Id, gid, target, GuildClass.SubMaster); break;
                case "demote": (code, _) = await guilds.ChangeClassAsync(me.Id, gid, target, GuildClass.Member); break;
                case "delegate": (code, _) = await guilds.ChangeClassAsync(me.Id, gid, target, GuildClass.Master); break;
                case "notice": code = await guilds.SetTextAsync(me.Id, gid, text, notice: true); break;
                case "intro": code = await guilds.SetTextAsync(me.Id, gid, text, notice: false); break;
                case "message": code = await guilds.SetMessageAsync(me.Id, gid, me.Id, text); break;
                case "leave": code = await guilds.LeaveAsync(me.Id, gid); break;
                case "close": (code, _) = await guilds.CloseAsync(me.Id, gid); break;
                case "emblem": code = await EmblemAsync(me.Id, gid, form?.Files.GetFile("file")); break;
                default: return Results.NotFound();
            }
            Log.Info($"WEB club: {me.Login} {action} guilda={gid} alvo={target}: {code}");
            return await HomeAsync(ctx, code == GuildCode.Ok ? "Feito." : Message(code));
        });
        // emblema do JP: "<guilda>_<nº>.png" (o nº só invalida o cache do cliente)
        app.MapGet("/upload/club/mark/{file}", async (string file) =>
        {
            int us = file.IndexOf('_');
            if (us <= 0 || !int.TryParse(file.AsSpan(0, us), out int gid) || await services.Guilds.GetAsync(gid) is not { } g
                || !GuildService.ValidMarkName(g.Mark)) return Results.NotFound();
            string path = Path.Combine(markDir, g.Mark + ".png");
            return File.Exists(path) ? Results.File(path, "image/png") : Results.NotFound();
        });
    }

    async Task<Domain.Accounts.Account?> AccountAsync(string key) =>
        await services.Sessions.ValidateClubWebAsync(key) is { } id ? await services.Accounts.FindByIdAsync(id) : null;

    Task<Domain.Accounts.Account?> MeAsync(HttpContext ctx) => AccountAsync(ctx.Request.Cookies[Cookie] ?? "");

    async Task<GuildCode> EmblemAsync(long account, int gid, IFormFile? file)
    {
        if (file == null || file.Length is <= 0 or > GuildService.EmblemMaxBytes) return GuildCode.Failed;
        using var ms = new MemoryStream();
        await file.CopyToAsync(ms);
        var png = ms.ToArray();
        if (!GuildService.ValidEmblemPng(png)) return GuildCode.Failed;
        var (code, _) = await guilds.SetEmblemWebAsync(account, gid, async mark =>
        {
            Directory.CreateDirectory(markDir);
            await File.WriteAllBytesAsync(Path.Combine(markDir, mark + ".png"), png);
        });
        return code;
    }

    async Task<IResult> HomeAsync(HttpContext ctx, string? message)
    {
        if (await MeAsync(ctx) is not { } me) return Expired();
        var sb = new StringBuilder();
        if (message != null) sb.Append("<p class=msg>").Append(E(message)).Append("</p>");
        var m = await services.Guilds.MembershipAsync(me.Id);
        var g = m == null ? null : await services.Guilds.GetAsync(m.GuildId);
        if (m == null || g == null)
        {
            sb.Append("<h2>Você não está em nenhum club</h2>");
            sb.Append("<p><a href=\"").Append(Base).Append("/list\">Procurar um club</a></p>");
            sb.Append("<h3>Criar um club</h3><form method=post action=\"").Append(Base).Append("/create\">")
              .Append("<table><tr><td>Nome</td><td><input name=name maxlength=20></td></tr>")
              .Append("<tr><td>Apresentação</td><td><input name=text maxlength=100 size=40></td></tr></table>")
              .Append("<input type=submit value=\"Criar\"></form>");
            return Page("Club", sb.ToString());
        }
        if (m.Class == GuildClass.Waiting)
        {
            sb.Append("<h2>Pedido enviado ao club ").Append(E(g.Name)).Append("</h2><p>Aguarde a resposta do mestre.</p>");
            sb.Append(Button("withdraw", g.Id, 0, "Desistir do pedido"));
            return Page("Club", sb.ToString());
        }
        bool manager = GuildClass.IsManager(m.Class), master = m.Class == GuildClass.Master;
        sb.Append("<h2>");
        if (GuildService.ValidMarkName(g.Mark)) sb.Append("<img src=\"/upload/club/mark/").Append(g.Id).Append('_').Append(GuildTag.MarkNo(g.Mark)).Append(".png\"> ");
        sb.Append(E(g.Name)).Append("</h2>");
        sb.Append("<p>Mestre: ").Append(E(g.MasterNick)).Append(" · Membros: ").Append(g.MemberCount).Append('/').Append(GuildService.MaxMembers)
          .Append(" · Seu cargo: ").Append(ClassName(m.Class)).Append("</p>");
        sb.Append("<p><b>Aviso:</b> ").Append(E(g.Notice)).Append("</p><p><b>Apresentação:</b> ").Append(E(g.Introduce)).Append("</p>");
        var (members, _) = await services.Guilds.MembersAsync(g.Id, 1, GuildService.MaxMembers * 4);
        var waiting = new StringBuilder();
        sb.Append("<h3>Membros</h3><table border=1 cellpadding=3><tr><th>Nick</th><th>Cargo</th><th>Mensagem</th><th></th></tr>");
        foreach (var x in members)
        {
            if (x.Class == GuildClass.Waiting)
            {
                waiting.Append("<tr><td>").Append(E(x.Nickname)).Append("</td><td>").Append(E(x.Message)).Append("</td><td>");
                if (manager) waiting.Append(Button("approve", g.Id, x.AccountId, "Aceitar")).Append(Button("reject", g.Id, x.AccountId, "Recusar"));
                waiting.Append("</td></tr>");
                continue;
            }
            sb.Append("<tr><td>").Append(E(x.Nickname)).Append("</td><td>").Append(ClassName(x.Class)).Append("</td><td>").Append(E(x.Message)).Append("</td><td>");
            if (x.AccountId != me.Id)
            {
                if (master && x.Class == GuildClass.Member) sb.Append(Button("promote", g.Id, x.AccountId, "Submestre"));
                if (master && x.Class == GuildClass.SubMaster)
                    sb.Append(Button("demote", g.Id, x.AccountId, "Membro")).Append(Button("delegate", g.Id, x.AccountId, "Passar o cargo de mestre"));
                if (manager && x.Class > m.Class) sb.Append(Button("kick", g.Id, x.AccountId, "Expulsar"));
            }
            sb.Append("</td></tr>");
        }
        sb.Append("</table>");
        if (waiting.Length > 0)
            sb.Append("<h3>Pedidos para entrar</h3><table border=1 cellpadding=3><tr><th>Nick</th><th>Mensagem</th><th></th></tr>").Append(waiting).Append("</table>");
        sb.Append("<h3>Minha mensagem</h3>").Append(TextForm("message", g.Id, "Salvar", 20));
        if (manager)
        {
            sb.Append("<h3>Aviso</h3>").Append(TextForm("notice", g.Id, "Salvar", 100));
            sb.Append("<h3>Apresentação</h3>").Append(TextForm("intro", g.Id, "Salvar", 100));
            sb.Append("<h3>Emblema</h3><form method=post enctype=\"multipart/form-data\" action=\"").Append(Base).Append("/emblem\">")
              .Append("<input type=hidden name=guild value=").Append(g.Id).Append("><input type=file name=file> <input type=submit value=\"Enviar\">")
              .Append("<br><small>PNG de 32 bits, até ").Append(GuildService.EmblemMaxWidth).Append('×').Append(GuildService.EmblemMaxHeight)
              .Append(" pixels e 8 KB.</small></form>");
        }
        sb.Append("<h3>Sair</h3>");
        sb.Append(master ? Button("close", g.Id, 0, "Encerrar o club (só sem outros membros)") : Button("leave", g.Id, 0, "Sair do club"));
        return Page("Club", sb.ToString());
    }

    async Task<IResult> ListAsync(Domain.Accounts.Account me, int page, string q)
    {
        var (items, total) = await services.Guilds.ListAsync(page, GuildService.PerPage, string.IsNullOrWhiteSpace(q) ? null : q.Trim());
        var sb = new StringBuilder("<h2>Clubs</h2>");
        sb.Append("<form method=get action=\"").Append(Base).Append("/list\"><input name=q value=\"").Append(E(q)).Append("\"> <input type=submit value=\"Procurar\"></form>");
        sb.Append("<table border=1 cellpadding=3><tr><th>Nome</th><th>Mestre</th><th>Membros</th><th>Apresentação</th><th></th></tr>");
        foreach (var g in items)
            sb.Append("<tr><td>").Append(E(g.Name)).Append("</td><td>").Append(E(g.MasterNick)).Append("</td><td>").Append(g.MemberCount)
              .Append("</td><td>").Append(E(g.Introduce)).Append("</td><td>").Append(Button("join", g.Id, 0, "Pedir para entrar")).Append("</td></tr>");
        sb.Append("</table><p>");
        int pages = Math.Max(1, (total + GuildService.PerPage - 1) / GuildService.PerPage);
        if (page > 1) sb.Append("<a href=\"").Append(Base).Append("/list?page=").Append(page - 1).Append("&q=").Append(Uri.EscapeDataString(q)).Append("\">« anterior</a> ");
        sb.Append("página ").Append(page).Append(" de ").Append(pages);
        if (page < pages) sb.Append(" <a href=\"").Append(Base).Append("/list?page=").Append(page + 1).Append("&q=").Append(Uri.EscapeDataString(q)).Append("\">próxima »</a>");
        sb.Append("</p><p><a href=\"").Append(Base).Append("/home\">Voltar</a></p>");
        return Page("Clubs", sb.ToString());
    }

    static string Button(string action, int gid, long target, string label) =>
        $"<form method=post action=\"{Base}/{action}\" style=\"display:inline\"><input type=hidden name=guild value={gid}>" +
        $"<input type=hidden name=target value={target}><input type=submit value=\"{E(label)}\"></form> ";

    static string TextForm(string action, int gid, string label, int max) =>
        $"<form method=post action=\"{Base}/{action}\"><input type=hidden name=guild value={gid}><input name=text maxlength={max} size=40> " +
        $"<input type=submit value=\"{E(label)}\"></form>";

    static string ClassName(int c) => c switch
    {
        GuildClass.Master => "Mestre", GuildClass.SubMaster => "Submestre", GuildClass.Member => "Membro", _ => "Pedido",
    };

    static string Message(GuildCode c) => c switch
    {
        GuildCode.NameTaken => "Esse nome já existe.",
        GuildCode.AlreadyInGuild => "Você já está num club (ou com um pedido).",
        GuildCode.Cooldown => "Espere 24 horas depois de sair de um club.",
        GuildCode.Full => "O club está cheio.",
        GuildCode.HasMembers => "O club ainda tem membros.",
        GuildCode.MasterCannotLeave => "O mestre não pode sair: passe o cargo ou encerre o club.",
        GuildCode.NotManager or GuildCode.NotMaster or GuildCode.NoPermission => "Você não tem permissão para isso.",
        GuildCode.SubMasterLimit => "Limite de submestres atingido.",
        GuildCode.NoGuild => "Esse club não existe mais.",
        _ => $"Não deu certo (código {(uint)c}).",
    };

    static IResult Expired() => Page("Club", "<p>A sessão expirou. Feche esta janela e abra o club de novo.</p>");

    static string E(string s) => SecurityElement.Escape(s) ?? "";

    static IResult Page(string title, string body) => Results.Content(
        "<!DOCTYPE html><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><title>" + E(title) +
        "</title><style>body{font-family:Tahoma,Arial,sans-serif;font-size:13px;background:#eef6ff;color:#123;margin:12px}" +
        "h2{font-size:18px;margin:6px 0}h3{font-size:14px;margin:14px 0 4px}table{border-collapse:collapse;background:#fff}" +
        ".msg{background:#fff6cc;padding:6px;border:1px solid #db3}</style></head><body>" + body + "</body></html>",
        "text/html; charset=utf-8");
}
