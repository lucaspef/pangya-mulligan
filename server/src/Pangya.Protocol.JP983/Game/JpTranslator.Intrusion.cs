using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Intrusão no JP (docs/server/protocolo/SPEC-jp983-intrusao-vip-modos.md §1): JP 0x9D = KR 0x9A (−3, mesmo layout); KR 0x111 ->
/// JP 0x113 com a sala (178 -> 221) e os slots (338 -> 348) no tamanho do JP e o início {4,4} sem o mapa por buraco.
/// </summary>
public sealed partial class JpTranslator
{
    void AddIntrusion()
    {
        toClient[0x111] = (jp, k) =>
        {
            if (k.Length < 2) return null;
            var w = new PacketWriter(jp, k.Length + 256);
            switch (k[0], k[1])
            {
                case (3, _) when k.Length >= 12 + KrToJp.KrRoomSize:
                    return w.Bytes(k.AsSpan(0, 12)).Room(k.AsSpan(12, KrToJp.KrRoomSize), ArtifactOf(k.AsSpan(12, KrToJp.KrRoomSize)), FlagsOf(k.AsSpan(12, KrToJp.KrRoomSize)));
                case (4, 0) when k.Length >= 2 + KrToJp.KrRoomSize:
                    return w.Bytes(k.AsSpan(0, 2)).Room(k.AsSpan(2, KrToJp.KrRoomSize), ArtifactOf(k.AsSpan(2, KrToJp.KrRoomSize)), FlagsOf(k.AsSpan(2, KrToJp.KrRoomSize)));
                case (4, 1) when k.Length >= 3:
                {
                    int n = k[2];
                    w.Bytes(k.AsSpan(0, 3));
                    for (int i = 0; i < n && 3 + (i + 1) * KrToJp.KrSlotSize <= k.Length; i++) w.Slot(k.AsSpan(3 + i * KrToJp.KrSlotSize, KrToJp.KrSlotSize));
                    return w;
                }
                case (4, 4) when k.Length >= 2 + 16 + 18 * 5 + 4:
                {
                    var b = k.AsSpan(2);
                    w.U8(4).U8(4).U8(b[0]).U8(KrToJp.TipoShow(b[1])).U8(b[2]).U8(b[3]).Bytes(b.Slice(4, 12));
                    int o = 16;
                    for (int i = 0; i < 18; i++, o += 5)
                    {
                        uint seed = BitConverter.ToUInt32(b[o..]);
                        w.U32(seed).U8((byte)(seed % 3)).U8(b[o + 4]);
                    }
                    return FieldItemsTail(w, kr.CurrentRoom, BitConverter.ToUInt32(b[o..]));
                }
                case (7, _):
                {
                    var r = new PacketReader();
                    var full = new byte[k.Length + 2];                                 // o leitor espera o id na frente
                    k.CopyTo(full, 2);
                    r.Reset(full, 0, full.Length);
                    r.U8(); r.U8();
                    string nick = r.Str();
                    var room = r.Bytes(KrToJp.KrRoomSize);
                    byte n = r.U8();
                    uint bonus = r.U32();
                    w.U8(7).U8(0).Str(nick).Room(room, ArtifactOf(room), FlagsOf(room)).U8(n).U32(bonus);
                    for (int i = 0; i < n && r.Remaining >= KrToJp.KrSlotSize; i++) w.Slot(r.Bytes(KrToJp.KrSlotSize));
                    return w;
                }
                default: return w.Bytes(k);
            }
        };
    }
}
