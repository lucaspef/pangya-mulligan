using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.KR645.Game;

/// <summary>
/// Relatório do Tiki (só o cliente JP pede; docs/server/protocolo/SPEC-jp983-tiki-report.md): sair do torneio terminado com o
/// pergaminho, levando o pang agora e o EXP no relatório; abrir o relatório. Os pacotes do JP ficam no JpSystems.
/// </summary>
public sealed partial class GameHandler
{
    /// <summary>Saída com o pergaminho que valeu: o relatório a criar e o jogador da partida (recebe o id do item).</summary>
    public sealed record TikiLeave(TikiReport.Data Report, MassPlayer Me);

    readonly Random reportRng = new();

    /// <summary>
    /// Sai da sala como na saída normal (0x4A 0xFFFF, que também tira o cliente da espera). Se o pergaminho vale (ver
    /// TikiReport.Refusal), antes disso o fim da partida dele roda com EXP 0 (pang, estatística e recorde agora) e o EXP
    /// pela fórmula do torneio (sem desconto por posição, então já é o final) vai para o relatório. null = recusado: só sai.
    /// </summary>
    public TikiLeave? LeaveWithTikiReport()
    {
        TikiLeave? res = null;
        string? why;
        lock (Rooms.Sync)
        {
            var r = room;
            if (r == null)
            {
                conn.Send(new PacketWriter(RoomPackets.SLeftRoom).U16(0xFFFF));   // fora de sala: só tira a espera
                return null;
            }
            var g = r.Game as MassGame;
            var me = g != null && r.Find(this) is { } rp ? g.Find(rp.Guid) : null;
            why = g == null ? "fora de partida" : TikiReport.Refusal(Player, r, g, me);
            if (why == null)
            {
                var now = DateTime.UtcNow;
                int players = g!.Players.Count;
                var cfg = ctx.World.Config.Rewards;
                int exp = Rewards.Exp(new Rewards.ExpInput(players, ctx.Data.CourseStars(r.CoursePlayed), 0, false, Player.Level),
                    g.HoleCount, true, RewardRates(now).Exp, cfg);
                me!.Reported = true;
                BeginGameEnd(me.Pang, me.Bonus, g.HoleCount, true, (r.CoursePlayed, me.Score), players, positionPenalty: false,
                    coursePlayed: r.CoursePlayed, fixedExp: 0);
                res = new TikiLeave(new TikiReport.Data(now, exp, ((TourneyGame)g).MatchTid, (int)r.Settings.Mode,
                    TikiReport.Rows(g, null, now)), me);
            }
            LeaveRoom(notifySelf: true);
        }
        if (res == null)
        {
            Log.Info($"{conn} pergaminho do Tiki recusado ({why}): saída normal");
            return null;
        }
        FinishGameEnd();                                                   // pang agora (0x43, 0xC6)
        return res;
    }

    /// <summary>Gasta 1 pergaminho e cria o relatório (7 dias); null = não tinha mais o pergaminho.</summary>
    public async Task<Item?> CreateTikiReportAsync(TikiLeave leave)
    {
        var p = Player;
        if (await ctx.Actions.DeleteItemAsync(p, TikiReport.ScrollTypeId, 1) == null)
        {
            Log.Info($"{conn} pergaminho do Tiki sumiu antes do relatório");
            return null;
        }
        var it = TikiReport.NewItem((await ctx.Players.Store.NewIdsAsync(1))[0], leave.Report, DateTime.UtcNow, reportRng);
        var ch = new PlayerChanges();
        ch.Added.Add(it);
        await ctx.Players.Store.ApplyAsync(p.AccountId, ch);
        p.Items[it.Id] = it;
        leave.Me.ReportItemId = it.Id;
        Log.Info($"{conn} relatório do Tiki {it.Id}: {leave.Report.Exp} EXP ao abrir, {leave.Report.Rows.Count} linhas");
        return it;
    }

    /// <summary>Fim da partida de quem saiu com o pergaminho: as linhas finais no relatório (se ainda não foi aberto).</summary>
    public async Task FinishTikiReportAsync(int itemId, int matchTid, List<TikiReport.Row> rows)
    {
        try
        {
            if (Player.Find(itemId) is not { TypeId: TikiReport.ReportTypeId } it || TikiReport.Read(it) is not { } d) return;
            var edit = it.Clone();
            TikiReport.Write(edit, d with { MatchTid = matchTid, Rows = rows });
            var ch = new PlayerChanges();
            ch.Updated.Add(edit);
            await ctx.Players.Store.ApplyAsync(Player.AccountId, ch);
            if (Player.Find(itemId) != null) Player.Items[itemId] = edit;          // aberto enquanto gravava: fica fora
            Log.Info($"{conn} relatório do Tiki {itemId}: resultado final ({rows.Count} linhas)");
        }
        catch (Exception e) { Log.Error($"{conn} falha ao fechar o relatório do Tiki {itemId}", e); }
    }

    /// <summary>
    /// Abrir o relatório: confere (TikiReport.Check), credita o EXP e apaga o item numa gravação só. Devolve o código
    /// (TikiReport.Ok ou o erro do 0x11A), o relatório e o nível de antes (para AfterTikiExpAsync).
    /// </summary>
    public async Task<(int Code, TikiReport.Data? Report, int LevelBefore)> OpenTikiReportAsync(int itemId, uint param)
    {
        var p = Player;
        var it = p.Find(itemId);
        int code = TikiReport.Check(it, param, DateTime.UtcNow);
        if (code != TikiReport.Ok) return (code, null, p.Level);
        var d = TikiReport.Read(it!)!;
        int before = p.Level;
        var ch = new PlayerChanges();
        ch.Removed.Add(itemId);
        await Rewards.AddExpAsync(ctx.Players.Store, p, d.Exp, ch);
        Log.Info($"{conn} relatório do Tiki {itemId} aberto: +{d.Exp} EXP (nível {before} -> {p.Level})");
        return (TikiReport.Ok, d, before);
    }

    /// <summary>Depois do resultado: presentes de nível (0x10D) e o 0x43 com EXP/nível novos.</summary>
    public async Task AfterTikiExpAsync(int levelBefore)
    {
        var p = Player;
        if (p.Level > levelBefore) await LevelUpGiftsAsync(levelBefore, p.Level);
        conn.Send(new PacketWriter(SStatsUpdate, 0x160).Struct(PlayerStructs.Statistics(p)).Struct(PlayerStructs.Trophies(p)).U8(0xFF).U8(0xFF));
    }

    /// <summary>Login: os relatórios vencidos ou sem dados saem (TikiReport.Stale).</summary>
    async Task PurgeExpiredReportsAsync()
    {
        var old = TikiReport.Stale(Player, DateTime.UtcNow);
        if (old.Count == 0) return;
        var ch = new PlayerChanges();
        foreach (var it in old) ch.Removed.Add(it.Id);
        await ctx.Players.Store.ApplyAsync(Player.AccountId, ch);
        foreach (var it in old) Player.Items.Remove(it.Id);
        Log.Info($"{conn} {old.Count} relatório(s) do Tiki vencido(s) ou sem dados apagado(s)");
    }
}
