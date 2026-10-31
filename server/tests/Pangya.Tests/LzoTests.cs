using System.Runtime.InteropServices;
using Pangya.Core.Crypto;

namespace Pangya.Tests;

/// <summary>LZO1X do núcleo (pacotes comprimidos do JP), conferido contra a liblzo2 de verdade quando ela existe (WSL).</summary>
public class LzoTests
{
    static IEnumerable<byte[]> Samples()
    {
        var rng = new Random(983);
        yield return [];
        yield return [7];
        yield return [1, 2, 3];
        yield return new byte[10_000];                                       // só zeros: matches longos estendidos
        var rand = new byte[5000];
        rng.NextBytes(rand);
        yield return rand;                                                   // sem repetição: literais longos
        var items = new byte[50 * 196];                                      // como uma página de itens do 0x73
        for (int i = 0; i < 50; i++)
        {
            BitConverter.GetBytes(1000 + i).CopyTo(items, i * 196);
            BitConverter.GetBytes(0x08000400 + rng.Next(64)).CopyTo(items, i * 196 + 4);
            items[i * 196 + 8 + rng.Next(180)] = (byte)rng.Next(256);
        }
        yield return items;
        var text = new byte[20_000];                                         // pouca repetição, distâncias grandes
        for (int i = 0; i < text.Length; i++) text[i] = (byte)"abcdefghij"[rng.Next(10)];
        yield return text;
    }

    static byte[] Compress(byte[] src)
    {
        var z = new byte[Lzo1x.MaxCompressedSize(src.Length)];
        return z[..Lzo1x.Compress(src, z)];
    }

    [Fact]
    public void RoundTripAndShrinksRepetitiveData()
    {
        foreach (var s in Samples())
        {
            var z = Compress(s);
            var back = new byte[s.Length + 16];
            Assert.Equal(s.Length, Lzo1x.Decompress(z, back));
            Assert.Equal(s, back[..s.Length]);
        }
        Assert.True(Compress(new byte[50 * 196]).Length < 200);
    }

    [Fact]
    public void Decompress_RejectsBrokenStreams()
    {
        var z = Compress(new byte[1000]);
        var dst = new byte[1000];
        Assert.Equal(-1, Lzo1x.Decompress(z[..^3], dst));                  // sem o fim
        Assert.Equal(-1, Lzo1x.Decompress(new byte[] { 0x20 | 5, 0xFF, 0xFF, 0x11, 0, 0 }, dst));   // distância antes do início
    }

    const string Lib = "/usr/lib/x86_64-linux-gnu/liblzo2.so.2";

    [DllImport(Lib)] static extern int lzo1x_decompress_safe(byte[] src, nuint srcLen, byte[] dst, ref nuint dstLen, IntPtr wrk);
    [DllImport(Lib)] static extern int lzo1x_1_compress(byte[] src, nuint srcLen, byte[] dst, ref nuint dstLen, byte[] wrk);

    [Fact]
    public void MatchesTheRealLzoLibrary()
    {
        if (!File.Exists(Lib)) return;                                       // só no WSL/Linux com liblzo2
        foreach (var s in Samples())
        {
            // o que nós comprimimos a liblzo2 descomprime igual
            var z = Compress(s);
            var back = new byte[s.Length + 16];
            nuint n = (nuint)back.Length;
            Assert.Equal(0, lzo1x_decompress_safe(z, (nuint)z.Length, back, ref n, IntPtr.Zero));
            Assert.Equal(s, back[..(int)n]);

            // e o que a liblzo2 comprime (com M1/M2/M4) nós descomprimimos
            var theirs = new byte[Lzo1x.MaxCompressedSize(s.Length)];
            nuint tn = (nuint)theirs.Length;
            Assert.Equal(0, lzo1x_1_compress(s, (nuint)s.Length, theirs, ref tn, new byte[16384 * 8]));
            var mine = new byte[s.Length + 16];
            Assert.Equal(s.Length, Lzo1x.Decompress(theirs.AsSpan(0, (int)tn), mine));
            Assert.Equal(s, mine[..s.Length]);
        }
    }
}
