using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;
using Pangya.Protocol.KR645.Ranking;

namespace Pangya.Protocol.JP983.Ranking;

/// <summary>
/// Ranking do cliente JP 983 (docs/server/protocolo/SPEC-jp983-correio-social.md §5): a lógica é a do ranking do KR
/// (RankingHandler), com hello 0x1388 u32 chave, u8 5, str data, ids S->C +0x1194 (0x1F5..0x1F8 -> 0x1389..0x138C) e a
/// ficha com o personagem do JP (513). Os C->S (0x00, 0x01, 0x02) são iguais. Strings em Shift-JIS.
/// </summary>
public static class JpRanking
{
    public const string ServerType = "ranking-jp";
    const ushort SHello = 0x1388, KrUser = 0x1F6, IdShift = 0x1194;

    /// <summary>Conexão nova do ranking JP: tradutor, Shift-JIS e o hello do JP.</summary>
    public static IConnectionHandler Connect(Connection c, RankingContext ctx)
    {
        c.Translator = new JpRankingTranslator();
        c.TextEncoding = Cp949.Japanese;
        return new RankingHandler(c, ctx)
        {
            Hello = key => new PacketWriter(SHello).U32((uint)key).U8(5).Str(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss")),
        };
    }

    /// <summary>
    /// Ficha: KR u8 0, u32 uid, char[22] id, char[22] nick, u8 nível, u8 0, sCharacterInfo 444, u16 0, 5 × (u32, u32, i32)
    /// -> JP u8 0, u32 uid, char[22], char[22], u16 nível, CharacterInfo 513, u8 0 (tem posições), as 5 linhas.
    /// </summary>
    public static PacketWriter User(ushort jp, byte[] k)
    {
        const int head = 1 + 4 + 22 + 22 + 2;
        if (k.Length < head + KrToJp.KrCharacterSize + 2) return new PacketWriter(jp).Bytes(k);
        return new PacketWriter(jp, k.Length + 80).Bytes(k.AsSpan(0, head)).Character(k.AsSpan(head, KrToJp.KrCharacterSize)).U8(0)
            .Bytes(k.AsSpan(head + KrToJp.KrCharacterSize + 2));
    }

    sealed class JpRankingTranslator : IPacketTranslator
    {
        public PacketWriter? ToClient(PacketWriter w)
        {
            int kr = w.Id;
            if (kr is < 0x1F5 or > 0x1F8) return w;
            ushort jp = (ushort)(kr + IdShift);
            if (kr != KrUser) return w.SetId(jp);
            using (w) return User(jp, w.Body[2..].ToArray());
        }

        public bool FromClient(PacketReader r, out byte[]? replaced)
        {
            replaced = null;
            return true;
        }
    }
}
