using Pangya.Domain.Rooms;

namespace Pangya.Domain.Players;

/// <summary>
/// Taxa de abandono, com a fórmula do cliente (user_info.cpp IsHideDisconnectRate; CSharedDoc::GetMyDisconPangPenalty):
/// percentual inteiro de abandonos / jogos, só com mais de 50 jogos e nível 6. Faixas do aviso de saída da partida
/// (SPEC-resultado-fim-de-jogo.md §4.5): 21% sem ranking, 31% sem 30 em equipe, 41% sem Pang Battle e troféus.
/// </summary>
public static class QuitRate
{
    const int MinGames = 50, MinLevel = 6, GmFlag = 0x04;
    const int RankingBan = 21, Team30Ban = 31, TopBan = 41, MannerBelow = 3;

    public static bool Counts(Player p) => p.Stats.Games > MinGames && p.Level >= MinLevel;

    /// <summary>Percentual que vale para as restrições; 0 abaixo do mínimo de jogos ou nível.</summary>
    public static int Of(Player p) => Counts(p) ? Percent(p.Stats.Quits, p.Stats.Games) : 0;

    static int Percent(long quits, long games) => (int)(quits * 100 / Math.Max(games, 1));

    public static bool NoRanking(Player p) => Of(p) >= RankingBan;
    public static bool NoTrophy(Player p) => Of(p) >= TopBan;

    /// <summary>Sala proibida pela taxa; GM passa, como nas recusas do cliente (IsIdentity(4)).</summary>
    public static bool Barred(Player p, GameMode mode) => (p.IdentityFlags & GmFlag) == 0 && mode switch
    {
        GameMode.Team30s => Of(p) >= Team30Ban,
        GameMode.PangBattle => Of(p) >= TopBan,
        _ => false,
    };

    /// <summary>
    /// Escurecimento do ícone (bits 1..2 de gender: i_male_02 e i_male_03). Começa em 31%: o cliente recusa o 30 em equipe
    /// com gender > 1, e a referência documenta os bits como 31-40% e 41%+.
    /// </summary>
    public static uint IconTier(Player p) => Of(p) switch { >= TopBan => 2, >= Team30Ban => 1, _ => 0 };

    /// <summary>Ícone de boas maneiras (bit manner): abaixo de 3%, com o mínimo de jogos e nível.</summary>
    public static bool Manner(Player p) => Counts(p) && Of(p) < MannerBelow;

    /// <summary>
    /// Pena em pang de GetMyDisconPangPenalty, com os totais de antes do abandono (o valor que o cliente mostrou). A partir
    /// do tipo 10 a janela de saída não mostra pena (approach novo, tipos 11-14).
    /// </summary>
    public static long PangPenalty(GameMode mode, int level, long games, long quits)
    {
        if (mode >= GameMode.NewApproach) return 0;
        long penalty = mode switch
        {
            GameMode.Stroke => 100,
            GameMode.Match or GameMode.PangBattle => 350,
            GameMode.Team => 300,
            GameMode.Tournament => 150,
            GameMode.Team30s => 200,
            _ => 0,
        };
        if (level >= MinLevel)
        {
            penalty += (level - 1) / 5 * 100;
            if (games > MinGames) penalty += (Percent(quits, games) - 10) * 10;
        }
        return Math.Max(penalty, 0);
    }
}
