using System.Buffers.Binary;
using System.IO.Compression;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Pangya.Core.Iff;

/// <summary>
/// Leitor do pangya.iff (dados do jogo): um ZIP de tabelas "Nome.iff". Cada tabela é
/// u16 quantidade, u16 bind, u32 versão, e depois registros de tamanho fixo. O tipo do registro (struct gerada
/// pelo StructGen) depende da versão do cliente; o tamanho é conferido na leitura.
/// </summary>
public sealed class IffArchive
{
    readonly Dictionary<string, byte[]> tables = new(StringComparer.OrdinalIgnoreCase);

    public IReadOnlyCollection<string> Names => tables.Keys;

    public static IffArchive Load(string path)
    {
        var a = new IffArchive();
        using var zip = ZipFile.OpenRead(path);
        foreach (var e in zip.Entries)
        {
            using var s = e.Open();
            var data = new byte[e.Length];
            s.ReadExactly(data);
            a.tables[e.FullName] = data;
        }
        return a;
    }

    public bool Has(string name) => tables.ContainsKey(name);

    /// <summary>Tamanho do registro de uma tabela (calculado do arquivo).</summary>
    public int RecordSize(string name)
    {
        var d = tables[name];
        int count = BinaryPrimitives.ReadUInt16LittleEndian(d);
        return count == 0 ? 0 : (d.Length - 8) / count;
    }

    /// <summary>
    /// Registros crus de uma tabela (sem o cabeçalho de 8 bytes) e o tamanho de cada um: para versões cujo layout é lido por
    /// offset (JP 983). Tabela ausente ou vazia: vazio.
    /// </summary>
    public ReadOnlyMemory<byte> Records(string name, out int count, out int size)
    {
        count = size = 0;
        if (!tables.TryGetValue(name, out var d) || d.Length < 8) return ReadOnlyMemory<byte>.Empty;
        count = BinaryPrimitives.ReadUInt16LittleEndian(d);
        if (count == 0 || (d.Length - 8) % count != 0) { count = 0; return ReadOnlyMemory<byte>.Empty; }
        size = (d.Length - 8) / count;
        return d.AsMemory(8);
    }

    /// <summary>
    /// Typeid de cada registro de uma tabela de item (u32 logo depois do u32 "ativo" do cabeçalho comum, igual em todas
    /// as versões), sem depender do tamanho do registro.
    /// </summary>
    public int[] TypeIds(string name)
    {
        if (!tables.TryGetValue(name, out var d) || d.Length < 8) return [];
        int count = BinaryPrimitives.ReadUInt16LittleEndian(d);
        if (count == 0 || (d.Length - 8) % count != 0 || (d.Length - 8) / count < 8) return [];
        int size = (d.Length - 8) / count;
        var ids = new int[count];
        for (int i = 0; i < count; i++) ids[i] = BinaryPrimitives.ReadInt32LittleEndian(d.AsSpan(8 + i * size + 4));
        return ids;
    }

    /// <summary>Lê todos os registros de uma tabela como T; erro se o tamanho não bate (versão errada dos dados).</summary>
    public T[] Table<T>(string name) where T : unmanaged
    {
        if (!tables.TryGetValue(name, out var d)) throw new InvalidDataException($"pangya.iff sem a tabela {name}");
        int count = BinaryPrimitives.ReadUInt16LittleEndian(d);
        int size = Unsafe.SizeOf<T>();
        if (d.Length != 8 + count * size)
            throw new InvalidDataException($"{name}: {count} registros em {d.Length - 8} bytes, mas {typeof(T).Name} tem {size} bytes");
        return MemoryMarshal.Cast<byte, T>(d.AsSpan(8)).ToArray();
    }
}
