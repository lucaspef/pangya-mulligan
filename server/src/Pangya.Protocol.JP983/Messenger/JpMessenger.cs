using Pangya.Core.Net;
using Pangya.Core.Text;
using Pangya.Protocol.KR645.Messenger;

namespace Pangya.Protocol.JP983.Messenger;

/// <summary>
/// Mensageiro do cliente JP 983 (docs/server/protocolo/SPEC-jp983-correio-social.md §4): a lógica é a do mensageiro do KR
/// (MessengerHandler), com hello 0x2E u8 1, u8 1, u32 chave, ids S->C +2 (0x2D..0x3C -> 0x2F..0x3E), amigo 141 -> 145 e o
/// status 0x115 no layout novo (com a posição). Os C->S têm os mesmos ids. Strings em Shift-JIS.
/// </summary>
public static class JpMessenger
{
    public const string ServerType = "messenger-jp";
    const ushort SHello = 0x2E, KrLogin = 0x2D, KrSub = 0x2E;
    const ushort SubList = 0x102, SubRequested = 0x104, SubAskedMe = 0x106, SubStatus = 0x115;
    /// <summary>sFriend do KR (141) e FriendInfo do JP (145: 4 bytes a mais antes da posição).</summary>
    public const int KrFriendSize = 0x8D, FriendSize = 0x91, FriendGap = 61;
    /// <summary>sUserPosition (75).</summary>
    const int PositionSize = 0x4B;

    /// <summary>Conexão nova do mensageiro JP: tradutor, Shift-JIS e o hello do JP.</summary>
    public static IConnectionHandler Connect(Connection c, MessengerContext ctx)
    {
        c.Translator = new JpMessengerTranslator(ctx);
        c.TextEncoding = Cp949.Japanese;
        return new JpMessengerHandler(c, ctx, new MessengerHandler(c, ctx) { Hello = key => new PacketWriter(SHello).U8(1).U8(1).U32((uint)key) });
    }

    public static PacketWriter Friend(this PacketWriter w, ReadOnlySpan<byte> k) => w.Bytes(k[..FriendGap]).U32(0).Bytes(k[FriendGap..]);

    /// <summary>
    /// Sub-pacote do 0x2E do KR (u16 sub + dados) no 0x30 do JP: lista (u8 página, u16 total, u16 n, n × amigo), pedido
    /// (u32 código [+ amigo]), pediram amizade (amigo) e status (KR u32 status, u32 uid -> JP u32 uid, u32 status, u8 1,
    /// posição). Também chega pela conexão do game (lista de amigos para correio/presente). ctx null = posição "offline".
    /// </summary>
    public static PacketWriter Sub(ushort jp, byte[] k, MessengerContext? ctx)
    {
        ushort sub = BitConverter.ToUInt16(k, 0);
        var d = k.AsSpan(2);
        var w = new PacketWriter(jp, k.Length + 64).U16(sub);
        switch (sub)
        {
            case SubList when d.Length >= 5:
            {
                int n = BitConverter.ToUInt16(k, 2 + 3);
                w.Bytes(d[..5]);
                for (int i = 0; i < n && 5 + (i + 1) * KrFriendSize <= d.Length; i++) w.Friend(d.Slice(5 + i * KrFriendSize, KrFriendSize));
                return w;
            }
            case SubRequested when d.Length >= 4 + KrFriendSize: return w.Bytes(d[..4]).Friend(d.Slice(4, KrFriendSize));
            case SubAskedMe when d.Length >= KrFriendSize: return w.Friend(d[..KrFriendSize]);
            case SubStatus when d.Length >= 8:
            {
                uint status = BitConverter.ToUInt32(k, 2), uid = BitConverter.ToUInt32(k, 6);
                w.U32(uid).U32(status).U8(1);
                return ctx != null ? w.Struct(ctx.PositionOf(uid)) : w.U16(0xFFFF).I32(-1).I32(-1).U8(0xFF).Zeros(64);
            }
            default: return w.Bytes(d);
        }
    }

    /// <summary>Ids S->C do mensageiro do KR no JP: +2 (0x2D..0x3C).</summary>
    public static int ServerId(int kr) => kr is >= KrLogin and <= 0x3C ? kr + 2 : kr;

    sealed class JpMessengerTranslator(MessengerContext ctx) : IPacketTranslator
    {
        public PacketWriter? ToClient(PacketWriter w)
        {
            int kr = w.Id;
            ushort jp = (ushort)ServerId(kr);
            if (kr != KrSub && kr != KrLogin) return w.SetId(jp);
            using (w)
            {
                var k = w.Body[2..].ToArray();
                if (kr == KrSub) return Sub(jp, k, ctx);
                if (k.Length == 1 && k[0] == 2) k[0] = 1;                      // login recusado: o JP usa 1
                return new PacketWriter(jp).Bytes(k);
            }
        }

        public bool FromClient(PacketReader r, out byte[]? replaced)
        {
            replaced = null;
            return true;                                                     // C->S: mesmos ids e layouts
        }
    }
}
