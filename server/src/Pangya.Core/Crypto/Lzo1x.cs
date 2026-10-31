namespace Pangya.Core.Crypto;

/// <summary>
/// LZO1X (o formato do lzo1x_decompress que o cliente usa no CCompressBuffer, shared/uselzo.cpp). O compressor é guloso e
/// só usa o que o descompressor sempre aceita: literais (o 1º trecho com "17 + n"; depois 0000LLLL), matches M3
/// (001LLLLL, distância até 16384) com 0..3 literais nos 2 bits baixos, e o fim 0x11 0x00 0x00. Conferido nos testes
/// contra a liblzo2.
/// </summary>
public static class Lzo1x
{
    const int MaxDistance = 16384, MinMatch = 3, HashBits = 14;

    [ThreadStatic] static int[]? hashTable;

    /// <summary>Pior caso do tamanho comprimido (dados que não se repetem).</summary>
    public static int MaxCompressedSize(int n) => n + n / 16 + 64 + 3;

    /// <summary>Comprime src em dst (dst com pelo menos <see cref="MaxCompressedSize"/>); devolve o tamanho.</summary>
    public static int Compress(ReadOnlySpan<byte> src, Span<byte> dst)
    {
        var head = (hashTable ??= new int[1 << HashBits]).AsSpan();
        head.Fill(-1);
        int ip = 0, op = 0, lit = 0;                // lit: início dos literais ainda não escritos
        bool afterMatch = false;                     // literais depois de um match vão nos 2 bits dele (até 3)
        int lastMatchOp = -1;                        // onde está o 1º byte de distância do último match
        while (ip + MinMatch <= src.Length)
        {
            int h = (int)(((uint)(src[ip] | src[ip + 1] << 8 | src[ip + 2] << 16) * 2654435761u) >> (32 - HashBits));
            int cand = head[h];
            head[h] = ip;
            if (cand < 0 || ip - cand > MaxDistance || src[cand] != src[ip] || src[cand + 1] != src[ip + 1] || src[cand + 2] != src[ip + 2])
            {
                ip++;
                continue;
            }
            int len = MinMatch;
            while (ip + len < src.Length && src[cand + len] == src[ip + len]) len++;
            op = Literals(src[lit..ip], dst, op, afterMatch, lastMatchOp);
            // M3: 001LLLLL (comprimento − 2, 0 = estendido) + u16 ((distância − 1) << 2 | literais seguintes)
            if (len - 2 <= 31) dst[op++] = (byte)(0x20 | (len - 2));
            else { dst[op++] = 0x20; op = Extend(len - 33, dst, op); }
            int d = (ip - cand - 1) << 2;
            lastMatchOp = op;
            dst[op++] = (byte)d;
            dst[op++] = (byte)(d >> 8);
            afterMatch = true;
            ip += len;
            lit = ip;
        }
        op = Literals(src[lit..], dst, op, afterMatch, lastMatchOp);
        dst[op++] = 0x11;                            // fim: M4 com distância 16384
        dst[op++] = 0;
        dst[op++] = 0;
        return op;
    }

    /// <summary>Escreve um trecho de literais (pode ser vazio).</summary>
    static int Literals(ReadOnlySpan<byte> lits, Span<byte> dst, int op, bool afterMatch, int lastMatchOp)
    {
        int n = lits.Length;
        if (n == 0) return op;
        if (afterMatch && n <= 3) dst[lastMatchOp] |= (byte)n;          // nos 2 bits do match anterior
        else if (!afterMatch && op == 0 && n <= 238) dst[op++] = (byte)(17 + n);   // 1º trecho
        else if (n - 3 <= 15) dst[op++] = (byte)(n - 3);
        else { dst[op++] = 0; op = Extend(n - 18, dst, op); }
        lits.CopyTo(dst[op..]);
        return op + n;
    }

    /// <summary>Comprimento estendido: um 0 para cada 255 e o resto (≥ 1).</summary>
    static int Extend(int n, Span<byte> dst, int op)
    {
        while (n > 255) { dst[op++] = 0; n -= 255; }
        dst[op++] = (byte)n;
        return op;
    }

    /// <summary>Descomprime um LZO1X (todos os tipos de match); devolve o tamanho ou −1 se o stream é inválido.</summary>
    public static int Decompress(ReadOnlySpan<byte> src, Span<byte> dst)
    {
        int ip = 0, op = 0, state = 0;               // state: literais copiados pela última instrução (0..3; 4 = trecho longo)
        try
        {
            if (src[0] > 17)
            {
                int t = src[ip++] - 17;
                Copy(src.Slice(ip, t), dst, ref op);
                ip += t;
                state = t < 4 ? t : 4;
            }
            while (true)
            {
                int t = src[ip++], len, dist;
                if (t < 16)
                {
                    if (state == 0)                                          // trecho de literais
                    {
                        len = t != 0 ? t + 3 : 18 + Ext(src, ref ip);
                        Copy(src.Slice(ip, len), dst, ref op);
                        ip += len;
                        state = 4;
                        continue;
                    }
                    // M1: 2 bytes (depois de 1..3 literais) ou 3 bytes (depois de trecho longo)
                    dist = (t >> 2) + (src[ip++] << 2) + 1 + (state == 4 ? 2048 : 0);
                    len = state == 4 ? 3 : 2;
                }
                else if (t >= 64) { len = (t >> 5) + 1; dist = ((t >> 2) & 7) + (src[ip++] << 3) + 1; }       // M2
                else if (t >= 32)                                                                              // M3
                {
                    len = (t & 31) != 0 ? (t & 31) + 2 : 33 + Ext(src, ref ip);
                    dist = ((src[ip] | src[ip + 1] << 8) >> 2) + 1;
                    t = src[ip];
                    ip += 2;
                }
                else                                                                                           // M4
                {
                    len = (t & 7) != 0 ? (t & 7) + 2 : 9 + Ext(src, ref ip);
                    int d = (src[ip] | src[ip + 1] << 8) >> 2;
                    int hi = (t & 8) << 11;
                    t = src[ip];
                    ip += 2;
                    if (hi == 0 && d == 0) return op;                       // fim
                    dist = 16384 + hi + d;
                }
                if (dist > op) return -1;
                for (int i = 0; i < len; i++, op++) dst[op] = dst[op - dist];
                state = t & 3;
                Copy(src.Slice(ip, state), dst, ref op);
                ip += state;
            }
        }
        catch (IndexOutOfRangeException) { return -1; }
        catch (ArgumentException) { return -1; }                         // fora do src ou dst pequeno demais
    }

    static int Ext(ReadOnlySpan<byte> src, ref int ip)
    {
        int n = 0;
        while (src[ip] == 0) { n += 255; ip++; }
        return n + src[ip++];
    }

    static void Copy(ReadOnlySpan<byte> s, Span<byte> dst, ref int op)
    {
        s.CopyTo(dst[op..]);
        op += s.Length;
    }
}
