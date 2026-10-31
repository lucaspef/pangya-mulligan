using Pangya.Core.Config;

namespace Pangya.Domain.Players;

/// <summary>
/// Recompensa de fim de partida. O pang vem do resultado que o cliente informa (a física é dele), então o servidor
/// limita por buraco jogado; o EXP é calculado no servidor. Quem saiu antes do fim não ganha nada.
/// </summary>
public static class Rewards
{
    /// <summary>NewRecord = recorde novo no curso (<see cref="CourseResult.IsNewRecord"/>).</summary>
    public readonly record struct Reward(long Pang, int Exp, int LevelsUp, bool NewRecord = false);

    /// <summary>
    /// Entrada da fórmula de EXP (Versus.requestFinishExpGame / Tourney do GB): jogadores na partida, estrelas do
    /// curso (dificuldade 1..5 do Course.iff), posição final (0 = 1º), se a posição desconta 10% por lugar (VS sim,
    /// torneio não) e o nível do jogador (informativo: no 70 a barra ainda enche até o máximo, ver Levels.AddExp).
    /// </summary>
    /// Practice: sala de prática (sem EXP e sem contar nos totais do perfil). FixedExp: EXP já calculado pelo modo (Grand
    /// Zodiac), só com as taxas.
    public readonly record struct ExpInput(int Players, float Stars, int Position, bool PositionPenalty, int Level, bool Practice = false,
        int? FixedExp = null);

    /// <summary>
    /// EXP = estrelas × buracos × jogadores × (1 + EXP% dos cards/itens) × taxa do servidor, × (1 − 0,1 × posição) no VS;
    /// 0 para quem saiu. Truncado a cada passo, como o GB (que dava 0 no nível 70; aqui não: a barra do 70 tem 1,6 M).
    /// </summary>
    public static int Exp(ExpInput e, int holes, bool finished, int expRate, RewardConfig cfg)
    {
        if (e.FixedExp is { } fixedExp)
            return (int)Math.Floor(Math.Max(fixedExp, 0) * (1 + Math.Max(expRate, 0) / 100.0) * (Math.Max(cfg.ExpRate, 0) / 100.0));
        if (!finished || holes <= 0 || e.Practice) return 0;
        double exp = Math.Floor(Math.Max(e.Players, 1) * holes * Math.Max(e.Stars, 1));
        exp = Math.Floor(exp * (1 + Math.Max(expRate, 0) / 100.0) * (Math.Max(cfg.ExpRate, 0) / 100.0));
        if (e.PositionPenalty) exp = Math.Floor(exp * Math.Max(0, 1 - 0.1 * e.Position));
        return (int)exp;
    }

    /// <summary>
    /// Pang = (informado + bônus) limitado por buraco e depois × (1 + pang%/100); EXP pela fórmula do GB (<see cref="Exp"/>).
    /// Os % vêm dos cards especiais em vigor (o cliente só mostra o pang multiplicado na tela; EXP é só do servidor).
    /// Sem ExpInput (testes, chamadas antigas): 1 jogador, 1 estrela, 1º lugar.
    /// </summary>
    public static (long Pang, int Exp) Compute(uint reportedPang, uint reportedBonus, int holes, bool finished, RewardConfig cfg,
        int pangRate = 0, int expRate = 0, ExpInput? expIn = null)
    {
        if (!finished || holes <= 0) return (0, 0);
        long pang = Math.Min((long)reportedPang + reportedBonus, (long)cfg.MaxPangPerHole * holes);
        int exp = Exp(expIn ?? new ExpInput(1, 1, 0, false, 0), holes, finished, expRate, cfg);
        return ((long)Math.Round(pang * (1 + Math.Max(pangRate, 0) / 100.0)), exp);
    }

    /// <summary>
    /// Calcula, grava (pang, nível, EXP e a estatística do curso, se o modo tiver placar contra o par) e só então
    /// aplica no jogador. course = (mapa, placar) ou null; trophy = (troféu da sala, ouro/prata/bronze) do torneio.
    /// </summary>
    public static async Task<Reward> ApplyAsync(IPlayerStore store, Player p, uint reportedPang, uint reportedBonus, int holes, bool finished,
        RewardConfig cfg, (int Course, int Score)? course = null, int pangRate = 0, int expRate = 0, GameStats? stats = null,
        ExpInput? expIn = null, (int RoomTid, int Kind)? trophy = null)
    {
        var (pang, exp) = Compute(reportedPang, reportedBonus, holes, finished, cfg, pangRate, expRate, expIn);
        // totais do perfil: só partida terminada; os contadores vêm do último 0x31/0x06 do cliente, limitados por buraco
        var totals = finished && holes > 0 && expIn?.Practice != true ? PlayerStats.After(p.Stats, stats, holes, course?.Score) : null;
        if (totals != null && trophy is { Kind: > Trophy.None } t) Trophy.Add(totals.Trophies, t.RoomTid, t.Kind);
        if (totals != null) totals.PangEarned += pang;                          // ranking de pang ganho
        Dictionary<int, CourseRecord>? courses = null;
        bool newRecord = false;
        if (finished && holes > 0 && course is { } c)
        {
            courses = new Dictionary<int, CourseRecord>(p.Courses);
            var result = new CourseResult(c.Course, holes, c.Score, pang, p.Character?.TypeId ?? 0);
            var old = p.Courses.GetValueOrDefault(c.Course);
            newRecord = result.IsNewRecord(old);
            courses[c.Course] = result.ApplyTo(old);
        }
        if (pang == 0 && exp == 0 && courses == null && totals == null) return default;
        var after = new Player { Level = p.Level, Exp = p.Exp };
        int levels = Levels.AddExp(after, exp);
        var changes = new PlayerChanges { Pang = p.Pang + pang, Level = after.Level, Exp = after.Exp, Courses = courses, Stats = totals };
        var caddie = CaddieGain(p, exp);
        if (caddie != null) changes.Updated.Add(caddie);
        await store.ApplyAsync(p.AccountId, changes);
        if (caddie != null) p.Items[caddie.Id] = caddie;
        p.Pang += pang;
        p.Level = after.Level;
        p.Exp = after.Exp;
        if (courses != null && course is { } played) p.Courses[played.Course] = courses[played.Course];
        if (totals != null) p.Stats = totals;
        return new Reward(pang, exp, levels, newRecord);
    }

    /// <summary>
    /// Abandono de partida que conta (RoomGame.QuitHoles): +1 jogo, +1 abandono e a pena em pang (QuitRate.PangPenalty, sem
    /// passar de zero). O aviso do cliente anula os registros da partida: nada do parcial, EXP ou recorde entra.
    /// </summary>
    public static async Task QuitAsync(IPlayerStore store, Player p, long penalty)
    {
        var totals = PlayerStats.After(p.Stats, null, 0, null);
        totals.Quits++;
        long pang = Math.Max(p.Pang - penalty, 0);
        await store.ApplyAsync(p.AccountId, new PlayerChanges { Stats = totals, Pang = penalty > 0 ? pang : null });
        (p.Stats, p.Pang) = (totals, pang);
    }

    /// <summary>O caddie equipado (em contrato) ganha a mesma EXP (CaddieLevels); null = nenhum.</summary>
    static Item? CaddieGain(Player p, int exp) =>
        exp > 0 && p.Find(p.Equip.CaddieId) is { Group: ItemGroup.Caddie } cad && !CaddieRent.IsExpired(cad, DateTime.UtcNow)
            ? CaddieLevels.Gain(cad, exp) : null;

    /// <summary>
    /// Só EXP, fora da partida (relatório do Tiki): nível, EXP e o caddie, gravados junto com changes (ex.: o item gasto).
    /// Devolve quantos níveis subiu.
    /// </summary>
    public static async Task<int> AddExpAsync(IPlayerStore store, Player p, int exp, PlayerChanges changes)
    {
        var after = new Player { Level = p.Level, Exp = p.Exp };
        int levels = Levels.AddExp(after, exp);
        (changes.Level, changes.Exp) = (after.Level, after.Exp);
        var caddie = CaddieGain(p, exp);
        if (caddie != null) changes.Updated.Add(caddie);
        await store.ApplyAsync(p.AccountId, changes);
        if (caddie != null) p.Items[caddie.Id] = caddie;
        foreach (var id in changes.Removed) p.Items.Remove(id);
        (p.Level, p.Exp) = (after.Level, after.Exp);
        return levels;
    }
}
