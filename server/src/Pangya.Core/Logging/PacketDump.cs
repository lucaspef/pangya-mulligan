namespace Pangya.Core.Logging;

/// <summary>
/// Registro do conteúdo dos pacotes (Logging.PacketHex): um arquivo por conexão em &lt;Dir&gt;/pacotes/
/// AAAAMMDD-HHMMSS-&lt;Nome&gt;#&lt;Id&gt;.log, uma linha por pacote: hora, direção (-&gt; servidor para cliente,
/// &lt;- cliente para servidor), id, tamanho e o corpo (id + dados) em hexadecimal, já decifrado e sem compressão.
/// </summary>
public sealed class PacketDump : IDisposable
{
    /// <summary>Pasta dos arquivos (null = desligado). Definida uma vez na partida do processo.</summary>
    public static string? Dir { get; set; }

    readonly StreamWriter writer;
    readonly object sync = new();
    bool disposed;

    PacketDump(StreamWriter writer) => this.writer = writer;

    /// <summary>Abre o arquivo da conexão (null se desligado ou se não der para criar).</summary>
    public static PacketDump? Open(string name, int id)
    {
        if (Dir is not { } dir) return null;
        try
        {
            Directory.CreateDirectory(dir);
            var path = Path.Combine(dir, $"{DateTime.Now:yyyyMMdd-HHmmss}-{name}#{id}.log");
            return new PacketDump(new StreamWriter(path, append: true) { AutoFlush = true });
        }
        catch (Exception e)
        {
            Log.Warn($"registro de pacotes: não deu para criar o arquivo de {name}#{id}: {e.Message}");
            return null;
        }
    }

    /// <summary>body = id (2 bytes) + dados.</summary>
    public void Write(bool toClient, ReadOnlySpan<byte> body)
    {
        int id = body.Length >= 2 ? body[0] | body[1] << 8 : -1;
        var line = $"{DateTime.Now:HH:mm:ss.fff} {(toClient ? "->" : "<-")} 0x{id:X4} {body.Length} {Convert.ToHexString(body)}";
        lock (sync)
        {
            if (disposed) return;
            try { writer.WriteLine(line); } catch (IOException) { }
        }
    }

    public void Dispose()
    {
        lock (sync)
        {
            if (disposed) return;
            disposed = true;
            writer.Dispose();
        }
    }
}
