using Pangya.Core.Logging;

namespace Pangya.Tests;

/// <summary>Logging.PacketHex: uma linha por pacote (hora, direção, id, tamanho, hexa) num arquivo por conexão.</summary>
public class PacketDumpTests
{
    [Fact]
    public void WritesOneLinePerPacket()
    {
        var dir = Path.Combine(Path.GetTempPath(), "pangya-dump-" + Guid.NewGuid().ToString("N"));
        var old = PacketDump.Dir;
        try
        {
            PacketDump.Dir = dir;
            using (var d = PacketDump.Open("GAME-JP", 7)!)
            {
                d.Write(true, [0x3F, 0x00, 0x01, 0xAB]);
                d.Write(false, [0x02, 0x00]);
            }
            var file = Assert.Single(Directory.GetFiles(dir));
            Assert.EndsWith("-GAME-JP#7.log", file);
            var lines = File.ReadAllLines(file);
            Assert.Equal(2, lines.Length);
            Assert.Matches(@"^\d\d:\d\d:\d\d\.\d{3} -> 0x003F 4 3F0001AB$", lines[0]);
            Assert.Matches(@"^\d\d:\d\d:\d\d\.\d{3} <- 0x0002 2 0200$", lines[1]);
        }
        finally
        {
            PacketDump.Dir = old;
            Directory.Delete(dir, true);
        }
    }
}
