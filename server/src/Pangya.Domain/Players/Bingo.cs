using Pangya.Core.Config;
using Pangya.Domain.Mail;

namespace Pangya.Domain.Players;

/// <summary>Bingo de uma conta: fichas para sortear, números já sorteados (bit n-1) e linhas completas (bits 0..11).</summary>
public sealed record BingoProgress(int Tries, int Aimed, int Lines);

/// <summary>Resultado de um sorteio: número tirado (0 = nada), linhas novas e o progresso depois dele.</summary>
public sealed record BingoDraw(int Code, int Number, int NewLines, BingoProgress Progress);

public interface IBingoStore
{
    Task<BingoProgress?> GetAsync(long account);
    Task SaveAsync(long account, BingoProgress progress);
}

/// <summary>
/// Bingo do lobby (FrBingoEventDlg, conteúdo 0x82). A cartela 5×5 vem do cliente (ns_board[uid % 10]); o servidor guarda as
/// fichas, os números e as linhas. Linhas: bits 0..4 = fileiras, 5..9 = colunas, 10 = diagonal principal, 11 = a outra.
/// Regras oficiais não são conhecidas: cada partida terminada dá fichas e cada linha nova manda o prêmio por carta.
/// </summary>
public sealed class BingoService(IBingoStore store, MailService? mail, BingoConfig config, Random? rng = null)
{
    public const int AllLines = 0xFFF;
    /// <summary>Códigos do 0x18B que o cliente conhece: 10 = sem ficha.</summary>
    public const int NoTries = 10;

    static readonly int[][] Boards =
    [
        [13, 2, 10, 25, 1, 12, 8, 22, 5, 16, 19, 6, 15, 14, 24, 17, 7, 4, 21, 18, 9, 20, 23, 11, 3],
        [7, 16, 18, 10, 21, 14, 1, 20, 2, 9, 19, 3, 6, 13, 25, 8, 5, 17, 23, 12, 15, 11, 24, 22, 4],
        [12, 3, 16, 9, 1, 4, 5, 2, 21, 10, 23, 6, 18, 14, 22, 20, 24, 15, 13, 17, 7, 8, 19, 11, 25],
        [6, 2, 21, 3, 17, 22, 20, 19, 8, 10, 14, 5, 13, 25, 1, 7, 23, 11, 18, 16, 24, 9, 12, 15, 4],
        [20, 17, 12, 9, 10, 25, 1, 19, 14, 15, 16, 3, 4, 7, 11, 18, 13, 21, 23, 22, 6, 8, 2, 5, 24],
        [23, 7, 3, 21, 11, 2, 4, 22, 24, 5, 19, 17, 10, 8, 18, 14, 15, 20, 6, 16, 12, 13, 25, 1, 9],
        [23, 21, 3, 4, 16, 2, 15, 24, 11, 10, 13, 18, 17, 6, 22, 5, 9, 8, 7, 1, 25, 14, 12, 20, 19],
        [1, 16, 15, 4, 24, 5, 2, 12, 20, 22, 9, 13, 8, 23, 21, 17, 11, 3, 19, 18, 7, 14, 6, 25, 10],
        [5, 20, 10, 4, 7, 8, 23, 17, 24, 22, 3, 13, 12, 25, 14, 16, 9, 19, 6, 11, 18, 21, 15, 1, 2],
        [25, 15, 17, 7, 18, 19, 13, 20, 22, 11, 1, 14, 21, 5, 16, 9, 4, 2, 8, 24, 10, 6, 12, 3, 23],
    ];

    readonly Random rng = rng ?? Random.Shared;

    public BingoConfig Config { get; } = config;

    public bool Open(DateOnly today) =>
        Config.Enabled && (Config.Start is not { } s || today >= s) && (Config.End is not { } e || today <= e);

    public async Task<BingoProgress> GetAsync(long account) =>
        await store.GetAsync(account) ?? new BingoProgress(Math.Max(Config.StartTries, 0), 0, 0);

    /// <summary>Linhas completas da cartela do jogador com os números sorteados.</summary>
    public static int LinesOf(long uid, int aimed)
    {
        var board = Boards[(int)((uint)uid % 10)];
        bool At(int row, int col) => (aimed & (1 << (board[row * 5 + col] - 1))) != 0;
        int lines = 0;
        for (int i = 0; i < 5; i++)
        {
            bool row = true, col = true;
            for (int j = 0; j < 5; j++) { row &= At(i, j); col &= At(j, i); }
            if (row) lines |= 1 << i;
            if (col) lines |= 1 << (5 + i);
        }
        bool d1 = true, d2 = true;
        for (int i = 0; i < 5; i++) { d1 &= At(i, i); d2 &= At(i, 4 - i); }
        return lines | (d1 ? 1 << 10 : 0) | (d2 ? 1 << 11 : 0);
    }

    /// <summary>Sorteia um número ainda não tirado, gasta uma ficha e manda o prêmio das linhas novas por carta.</summary>
    public async Task<BingoDraw> DrawAsync(long account, DateOnly today)
    {
        var cur = await GetAsync(account);
        if (!Open(today) || cur.Tries < 1 || cur.Lines == AllLines) return new BingoDraw(NoTries, 0, 0, cur);
        var free = new List<int>(25);
        for (int n = 1; n <= 25; n++) if ((cur.Aimed & (1 << (n - 1))) == 0) free.Add(n);
        int number = free[rng.Next(free.Count)];
        int aimed = cur.Aimed | 1 << (number - 1), lines = LinesOf(account, aimed);
        var next = new BingoProgress(cur.Tries - 1, aimed, lines);
        await store.SaveAsync(account, next);
        int fresh = lines & ~cur.Lines;
        if (fresh != 0 && mail != null && Prizes(fresh) is { Count: > 0 } items)
            await mail.SendSystemAsync(account, "@Event", "Bingo", items);
        return new BingoDraw(0, number, fresh, next);
    }

    /// <summary>Itens das linhas novas (um prêmio por linha; a cartela cheia soma o prêmio final).</summary>
    public List<(int TypeId, int Quantity)> Prizes(int fresh)
    {
        var items = new List<(int, int)>();
        int n = System.Numerics.BitOperations.PopCount((uint)fresh);
        foreach (var it in Config.LinePrizes) items.Add((it.TypeId, Math.Max(1, it.Quantity) * n));
        return items;
    }

    /// <summary>Fim de partida: fichas por partida terminada (sem prática). Devolve o progresso novo ou null.</summary>
    public async Task<BingoProgress?> OnGameEndAsync(long account, bool practice, int holes, bool finished, DateOnly today)
    {
        if (!Open(today) || practice || !finished || holes < Config.MinHoles || Config.TriesPerGame <= 0) return null;
        var cur = await GetAsync(account);
        var next = cur with { Tries = Math.Min(cur.Tries + Config.TriesPerGame, 99) };
        await store.SaveAsync(account, next);
        return next;
    }
}
