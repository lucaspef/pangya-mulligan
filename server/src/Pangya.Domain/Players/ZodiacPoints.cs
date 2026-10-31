namespace Pangya.Domain.Players;

/// <summary>Pontos do Grand Zodiac da conta (somados nos eventos; definem o tamanho do copo).</summary>
public interface IZodiacPointsStore
{
    Task<int> GetAsync(long account);
    /// <summary>Soma e devolve o total novo.</summary>
    Task<int> AddAsync(long account, int points);
}

public static class ZodiacPoints
{
    static readonly int[] CupLimits = [300, 600, 1200, 1800, 4000, 5200, 7600, 10_000];

    /// <summary>Tamanho do copo pelo total de pontos (referência): 9 abaixo de 300 ... 1 a partir de 10 000.</summary>
    public static int Cup(int points)
    {
        for (int i = 0; i < CupLimits.Length; i++) if (points < CupLimits[i]) return 9 - i;
        return 1;
    }
}
