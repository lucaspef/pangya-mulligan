using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>
/// Um Grand Prix (GrandPrixData; docs/server/protocolo/SPEC-jp983-grandprix.md §4.1). Id = o que o cliente pede; Link = grupo
/// (prêmios por posição, lista de concluídos). Aba (Id &amp; 0xFFFFFF) &gt;&gt; 19: 0 Rookie (sem horário, sala só do jogador).
/// Open/Start em minutos do dia (-1 = sem horário).
/// </summary>
/// <summary>Regras do GP (GrandPrixData.rule): faixas de vento (o valor do protocolo: 0 = 1 m).</summary>
public static class GrandPrixRules
{
    public const int Wind1m = 0x1A000265, SuperWind = 0x1A000266, PowerShotPenalty = 0x1A000267, TenSeconds = 0x1A000268,
        FifteenSeconds = 0x1A00029E, Wind3to5 = 0x1A00028F, Wind7to9 = 0x1A000290;

    /// <summary>
    /// A tacada leva +1 (o cliente já soma no próprio placar e põe 1 no 38º byte do 0x1B): relógio de 10/15 s estourado, ou
    /// tacada especial (tomahawk/spike/cobra) na regra 0x1A000267.
    /// </summary>
    public static bool Penalized(int rule, in ShotResult r) => rule switch
    {
        PowerShotPenalty => r.Penalty != 0 || (r.ShotFlags & 0x0E) != 0,
        TenSeconds or FifteenSeconds => r.Penalty != 0,
        _ => false,
    };

    /// <summary>Bônus de "clear" por mapa (pang, × buracos / 2) quando a última tacada traz o bit 0x2000 no display.</summary>
    public static int ClearBonus(byte course) => course switch
    {
        0 => 20, 1 => 50, 2 => 55, 3 => 80, 4 => 65, 5 => 24, 6 => 50, 7 => 70, 8 => 40, 9 => 55, 10 => 40, 11 => 20, 13 => 80,
        14 => 20, 15 => 20, 16 => 40, 17 => 360, 18 => 70, 19 => 40, 20 => 40, 21 => 40, _ => 0,
    };

    /// <summary>Faixa de vento da regra (referência: CourseManager); null = a normal.</summary>
    public static (int Min, int Max)? WindRange(int rule) => rule switch
    {
        Wind1m => (0, 1), SuperWind => (9, 15), Wind3to5 => (2, 5), Wind7to9 => (6, 8), _ => null,
    };
}

public sealed record GrandPrixDef(int Id, int Link, int TypeGP, int TimeHoleSeconds, string Name, int TicketTypeId, int TicketQty,
    bool Natural, bool ShortGame, int Rule, byte Course, byte HoleOrder, byte Holes, int MinLevel, int MaxLevel,
    (int Max, int Med, int Min) BotScore, int BotClass, (int TypeId, int Qty, int Days)[] Rewards, int OpenMinute, int StartMinute,
    int ClearLink, bool Locked)
{
    public int Aba => (Id & 0xFFFFFF) >> 19;
    /// <summary>Buracos especiais (GrandPrixSpecialHole.iff, pelo Link, em ordem de HolePOS): mapa e buraco de cada um.</summary>
    public (byte Course, byte Hole)[] SpecialHoles { get; init; } = [];
    public bool IsEvent => ((Id >> 24) & 3) == 3;
    public bool IsRookie => Aba == 0 && !IsEvent;
}

/// <summary>Prêmio de uma posição (GrandPrixRankReward): itens e o troféu.</summary>
public sealed record GrandPrixRankReward(int Link, int Rank, (int TypeId, int Qty, int Days)[] Items, int Trophy);

/// <summary>Tabelas do Grand Prix (só o cliente JP 983).</summary>
public interface IGrandPrixData
{
    IReadOnlyDictionary<int, GrandPrixDef> GrandPrix { get; }
    /// <summary>Prêmios por posição (1..3) de um grupo (Link), em ordem de posição.</summary>
    IReadOnlyList<GrandPrixRankReward> GrandPrixRankRewards(int link);
    /// <summary>Ids dos bots da classe (GrandPrixAIOptionalData).</summary>
    IReadOnlyList<int> GrandPrixBots(int botClass);
}

/// <summary>GPs concluídos por jogador: Link -> melhor posição (1 = 1º; contando os bots).</summary>
public interface IGrandPrixStore
{
    Task<Dictionary<int, int>> CompletedAsync(long account);
    Task SaveAsync(long account, int link, int position);
}

/// <summary>Buraco de um bot do GP (só placar: o cliente mostra o progresso dele).</summary>
public readonly record struct GrandPrixBotHole(byte Course, byte Hole, int Score, long Pang, long Bonus);

public sealed record GrandPrixBot(int Id, List<GrandPrixBotHole> Holes)
{
    public int Score { get { int s = 0; foreach (var h in Holes) s += h.Score; return s; } }
    public long Pang { get { long s = 0; foreach (var h in Holes) s += h.Pang + h.Bonus; return s; } }
}

/// <summary>Jogador no fim do GP: placar (contra o par) e pang da partida.</summary>
public readonly record struct GrandPrixEntry(uint Guid, Player Player, int Score, long Pang);

/// <summary>Quem recebe o fim do GP de uma conexão (posição, prêmios, pódio: só o cliente que tem GP).</summary>
public interface IGrandPrixSink
{
    Task FinishedAsync(GrandPrixRun run, GrandPrixEntry me, IReadOnlyList<GrandPrixEntry> players);
    /// <summary>Acabou o tempo do buraco (0x259): o buraco foi dado como desistência.</summary>
    void HoleTimeOver() { }
    /// <summary>Fechou o último buraco (0x199).</summary>
    void LastHole() { }
}

/// <summary>GP em andamento numa sala: a definição, quando começa e os bots (criados no início).</summary>
public sealed class GrandPrixRun(GrandPrixDef def)
{
    public GrandPrixDef Def { get; } = def;
    public List<GrandPrixBot> Bots { get; } = [];
    public DateTime? StartsAt { get; set; }
}

/// <summary>
/// Regras do Grand Prix que não dependem do cliente: checagens de entrada (códigos do cliente), horário, bots e posição
/// final (jogadores + bots por placar, empate pelo pang).
/// </summary>
public static class GrandPrix
{
    public const int MaxPlayers = 30;
    public const uint Ok = 0, Generic = 0x6700000, NotFound = 0x6700001, Full = 0x6700005, Level = 0x6700006, Ticket = 0x6700009,
        Locked = 0x670000A, EventOff = 0x670000B, Closed = 0x670000C;

    /// <summary>Horário de entrada: do Open ao Start (o Start pode cair no dia seguinte). Sem horário = sempre.</summary>
    public static bool IsOpen(GrandPrixDef d, DateTime now)
    {
        if (d.OpenMinute < 0 || d.StartMinute < 0) return true;
        int m = now.Hour * 60 + now.Minute, start = d.StartMinute < d.OpenMinute ? d.StartMinute + 1440 : d.StartMinute;
        if (m < d.OpenMinute) m += 1440;
        return m >= d.OpenMinute && m <= start;
    }

    /// <summary>Quando a partida começa sozinha (próximo Start depois de agora); null = o jogador começa (Rookie).</summary>
    public static DateTime? StartTime(GrandPrixDef d, DateTime now)
    {
        if (d.IsRookie || d.StartMinute < 0) return null;
        var t = now.Date.AddMinutes(d.StartMinute);
        return t < now ? t.AddDays(1) : t;
    }

    public static uint CanEnter(Player p, GrandPrixDef d, Dictionary<int, int> completed, DateTime now, bool eventOn = false)
    {
        if (d.IsEvent && !eventOn && d.TypeGP > 0) return EventOff;
        if (!IsOpen(d, now)) return Closed;
        if (p.Level < d.MinLevel || (d.MaxLevel > 0 && p.Level > d.MaxLevel)) return Level;
        if (d.TicketTypeId != 0 && d.TicketQty > 0 && (p.FindType(d.TicketTypeId)?.Quantity ?? 0) < d.TicketQty
            && !Premium.IsActive(p, DateTime.UtcNow)) return Ticket;                  // premium entra sem ticket
        if (d.Locked && d.ClearLink != 0 && !completed.ContainsKey(d.ClearLink)) return Locked;
        return Ok;
    }

    /// <summary>
    /// Bots: 30 − jogadores, ids sorteados da classe; cada um com um placar alvo (máx/médio/mín da tabela) espalhado
    /// pelos buracos (−2..+3 por buraco), pang 0..350 e bônus 0..199 por buraco. O cliente só mostra.
    /// </summary>
    public static List<GrandPrixBot> MakeBots(GrandPrixDef d, IReadOnlyList<int> ids, int players, byte[] holeOrder, Random rng)
    {
        var bots = new List<GrandPrixBot>();
        int n = Math.Max(0, MaxPlayers - players);
        if (ids.Count == 0) return bots;
        int holes = Math.Clamp((int)d.Holes, 1, 18);
        for (int b = 0; b < n; b++)
        {
            int pick = rng.Next(5);
            int target = pick == 0 ? d.BotScore.Max : pick < 3 ? d.BotScore.Med : d.BotScore.Min;
            var list = new List<GrandPrixBotHole>(holes);
            int left = target;
            for (int h = 0; h < holes; h++)
            {
                int remaining = holes - h;
                int s = h == holes - 1 ? left : Math.Clamp((int)Math.Round((double)left / remaining) + rng.Next(-1, 2), -2, 3);
                s = Math.Clamp(s, -3, 5);
                left -= s;
                byte hole = h < holeOrder.Length ? holeOrder[h] : (byte)(h + 1);
                list.Add(new GrandPrixBotHole(d.Course, hole, s, rng.Next(351), rng.Next(200)));
            }
            bots.Add(new GrandPrixBot(ids[rng.Next(ids.Count)], list));
        }
        return bots;
    }

    /// <summary>Posição (1..) de um placar entre os bots e os outros jogadores (menor placar primeiro; empate: mais pang).</summary>
    public static int Position(int score, long pang, IEnumerable<(int Score, long Pang)> others)
    {
        int pos = 1;
        foreach (var (s, pg) in others) if (s < score || (s == score && pg > pang)) pos++;
        return pos;
    }
}
