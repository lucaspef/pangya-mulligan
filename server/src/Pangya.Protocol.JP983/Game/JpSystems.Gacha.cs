using Pangya.Core.Net;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Gacha (SPEC-jp983-eventos-drops.md §6): o site sorteia no jogador online (GachaDesk); ao fechar a página (0x9E) vão o
/// 0x216 com o que mudou e o 0x102 u32 tickets, u32 parciais, u64 pang, u64 cookie. O 0x102 também vai no login.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CGachaClosed = 0x9E;
    const ushort SGachaCoupons = 0x102;

    public void SendGachaCounts()
    {
        var p = kr.Player;
        var (tickets, partials, _, _) = GachaDesk.Counts(p);
        conn.SendTranslated(new PacketWriter(SGachaCoupons).U32((uint)tickets).U32((uint)partials).U64((ulong)p.Pang).U64((ulong)p.Cookie));
    }

    void GachaClosed()
    {
        if (gacha?.TakePending(kr.Player.AccountId) is { Count: > 0 } changes) conn.SendTranslated(ItemsChanged(changes));
        SendGachaCounts();
    }
}
