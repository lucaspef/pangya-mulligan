using System.Text;

namespace Pangya.Core.Text;

/// <summary>
/// Texto do cliente: char[N] terminado em zero, no codepage da conexão. Coreano (949) por padrão; a conexão do cliente
/// JP liga o japonês (932, Shift-JIS) em <see cref="Ambient"/> (Connection.TextEncoding), e tudo o que roda a partir
/// dela (tradutor, handler, timers da partida) lê e grava strings nele.
/// </summary>
public static class Cp949
{
    public static readonly Encoding Korean = Load(949), Japanese = Load(932);
    static readonly AsyncLocal<Encoding?> ambient = new();

    /// <summary>Codepage do contexto atual (o da conexão; coreano fora dela).</summary>
    public static Encoding Encoding => ambient.Value ?? Korean;

    /// <summary>Codepage do fluxo atual (null = coreano). Vale para o que roda a partir daqui (AsyncLocal).</summary>
    public static Encoding? Ambient { get => ambient.Value; set => ambient.Value = value; }

    static Encoding Load(int codepage)
    {
        Encoding.RegisterProvider(CodePagesEncodingProvider.Instance);
        return Encoding.GetEncoding(codepage, EncoderFallback.ReplacementFallback, DecoderFallback.ReplacementFallback);
    }

    /// <summary>Primeiro byte de um caractere de 2 bytes (no 932 o katakana de meia largura 0xA1..0xDF tem 1 byte).</summary>
    static bool IsLead(Encoding e, byte b) => e.CodePage == 932 ? b is >= 0x81 and <= 0x9F or >= 0xE0 and <= 0xFC : b >= 0x81;

    /// <summary>Lê um char[N]: até o primeiro zero.</summary>
    public static string Read(ReadOnlySpan<byte> field)
    {
        int end = field.IndexOf((byte)0);
        return Encoding.GetString(end < 0 ? field : field[..end]);
    }

    /// <summary>Grava em char[N], cortando em N-1 bytes (sem partir caractere de 2 bytes) e completando com zeros.</summary>
    public static void Write(Span<byte> field, string value)
    {
        field.Clear();
        var e = Encoding;
        var bytes = e.GetBytes(value);
        int n = Math.Min(bytes.Length, field.Length - 1);
        // não deixar meio caractere de 2 bytes no fim: conta os bytes de lead até o corte
        int i = 0;
        while (i < n) i += IsLead(e, bytes[i]) ? 2 : 1;
        if (i > n) n -= 1;
        bytes.AsSpan(0, n).CopyTo(field);
    }
}
