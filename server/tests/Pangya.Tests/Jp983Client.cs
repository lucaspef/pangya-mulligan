using System.Text.RegularExpressions;
using Pangya.Core.Net;

namespace Pangya.Tests;

/// <summary>
/// "Cliente JP falso" guiado pela análise do exe 983 (docs/server/jp983-protocolo): monta os pacotes C->S pela sequência de
/// escritas que o cliente faz (pacotes_envio_983.tsv: E1/E2/E4/E8/ES/EB(n)) e diz que blocos o cliente lê em cada
/// pacote S->C (pacotes_recebe_983.tsv), para conferir os tamanhos das nossas structs contra o exe.
/// </summary>
public static partial class Jp983Client
{
    static readonly string Dir = Path.Combine(TestEnv.Root, "..", "docs", "server", "jp983-protocolo");
    static readonly string[][] Sends = Load("pacotes_envio_983.tsv");
    static readonly string[][] Receives = Load("pacotes_recebe_983.tsv");

    static string[][] Load(string file) =>
        File.ReadAllLines(Path.Combine(Dir, file)).Skip(1).Where(l => l.Length > 0).Select(l => l.Split('\t')).ToArray();

    /// <summary>
    /// Pacote como o cliente monta: acha no exe o lugar que monta este id com exatamente estes tipos de campo (função
    /// opcional, para desempatar) e escreve os valores na ordem (byte, ushort, uint, ulong, string, byte[]).
    /// </summary>
    public static PacketWriter Build(ushort id, string? function, params object[] values)
    {
        var fields = Fields(values);
        if (!Sends.Any(r => Convert.ToUInt16(r[0], 16) == id && (function == null || r[5].Contains(function)) && r[2] == fields))
            throw new Xunit.Sdk.XunitException($"o exe 983 não monta 0x{id:X} com campos \"{fields}\"");
        var w = new PacketWriter(id);
        foreach (var v in values)
            switch (v)
            {
                case byte b: w.U8(b); break;
                case ushort s: w.U16(s); break;
                case uint u: w.U32(u); break;
                case ulong q: w.U64(q); break;
                case string str: w.Str(str); break;
                case byte[] bytes: w.Bytes(bytes); break;
            }
        return w;
    }

    static string Fields(object[] values) => string.Join(' ', values.Select(v => v switch
    {
        byte => "E1", ushort => "E2", uint => "E4", ulong => "E8", string => "ES", byte[] b => $"EB(0x{b.Length:x})",
        _ => throw new ArgumentException($"tipo de campo sem equivalente no exe: {v.GetType().Name}"),
    }));

    /// <summary>Leituras que o despachante principal do cliente (FUN_009c20a0) faz no caso deste id.</summary>
    public static string Reads(ushort id, string dispatcher = "FUN_009c20a0") =>
        Receives.FirstOrDefault(r => r[1] == dispatcher && Convert.ToUInt16(r[2], 16) == id)?[4]
        ?? throw new Xunit.Sdk.XunitException($"o cliente 983 não trata 0x{id:X} em {dispatcher}");

    /// <summary>Tamanhos dos blocos (DB(n)) que o cliente lê neste pacote, na ordem.</summary>
    public static int[] Blocks(ushort id) =>
        BlockRx().Matches(Reads(id)).Select(m => Convert.ToInt32(m.Groups[1].Value, 16)).ToArray();

    [GeneratedRegex(@"DB\(0x([0-9a-f]+)\)")]
    private static partial Regex BlockRx();
}
