using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Caixa do My Room (referência requestOpenBoxMyRoom): C->S 0xB2 u32 typeid da caixa -> 0x216 (caixa −1 e itens novos),
/// 0xC8 se veio pang, e 0x129 u8 0, u32 caixa, i32 caixas que sobraram, u32 n, n × (u32 typeid, i32 id, i32 qtd, 8 zeros);
/// recusa: 0x129 u8 código e 12 zeros. Só as caixas da lista do cliente (<see cref="JpBoxes"/>).
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CMyRoomBox = 0xB2;
    const ushort SMyRoomBox = 0x129;

    /// <summary>Conteúdo das caixas (null = recusa todas).</summary>
    public BoxService? Boxes { get; set; }

    async Task MyRoomBoxAsync(int boxTid)
    {
        var r = Boxes == null || !JpBoxes.All.Contains(boxTid) ? BoxResult.Fail(BoxResult.NotOwned) : await Boxes.OpenAsync(kr.Player, boxTid);
        if (r.Code != 0)
        {
            conn.SendTranslated(new PacketWriter(SMyRoomBox).U8(r.Code).Zeros(12));
            Log.Info($"{conn} JP: caixa {boxTid:X8} recusada ({r.Code})");
            return;
        }
        conn.SendTranslated(ItemsChanged(r.Changes));
        bool pang = false;
        foreach (var pz in r.Prizes) if (pz.TypeId == BoxService.PangTid) pang = true;
        if (pang) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)kr.Player.Pang).U64(0));
        var w = new PacketWriter(SMyRoomBox, 16 + r.Prizes.Count * 20).U8(0).U32((uint)boxTid).I32(r.Left).U32((uint)r.Prizes.Count);
        foreach (var pz in r.Prizes) w.U32((uint)pz.TypeId).I32(pz.Id).I32(pz.Quantity).Zeros(8);
        conn.SendTranslated(w);
        Log.Info($"{conn} JP: caixa {boxTid:X8} -> {(r.Prizes.Count > 0 ? $"{r.Prizes[0].TypeId:X8} x{r.Prizes[0].Quantity}" : "nada")}, sobram {r.Left}");
    }
}
