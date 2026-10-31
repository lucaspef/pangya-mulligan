using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Letreiro do JP (SPEC-jp983-efeitos.md §7): 0x66 str mensagem -> gasta o preço em cookies (0x96 u64 cookies) e entra na
/// fila do mundo (erro: 0x50 u32 código, 4 = sem cookies); 0x67 -> 0xCA u16 na fila, u32 ms até a vez.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CTicker = 0x66, CTickerQueue = 0x67;
    const ushort STickerError = 0x50, SCookie = 0x96, STickerQueue = 0xCA;
    /// <summary>Preço da mensagem (referência: 1 cookie; o oficial não é conhecido).</summary>
    public const int TickerCookies = 1;

    async Task<bool> HandleTickerAsync(PacketReader r)
    {
        var p = kr.Player;
        if (r.Id == CTickerQueue)
        {
            r.Skip(r.Remaining);
            int n = ctx.Ticker.Count;
            conn.SendTranslated(new PacketWriter(STickerQueue).U16((ushort)n).U32((uint)(n * TickerBoardSlotMs)));
            return true;
        }
        string text = r.Remaining >= 2 ? r.Str() : "";
        r.Skip(r.Remaining);
        text = text.Trim();
        if (text.Length == 0) { conn.SendTranslated(new PacketWriter(STickerError).U32(1)); return true; }
        if (p.Cookie < TickerCookies) { conn.SendTranslated(new PacketWriter(STickerError).U32(4)); return true; }
        long cookie = p.Cookie - TickerCookies;
        await ctx.Players.Store.ApplyAsync(p.AccountId, new PlayerChanges { Cookie = cookie });
        p.Cookie = cookie;
        conn.SendTranslated(new PacketWriter(SCookie).U64((ulong)p.Cookie));
        ctx.Ticker.Enqueue(p.Nickname, text);
        return true;
    }

    static int TickerBoardSlotMs => (int)KR645.Game.TickerBoard.Slot.TotalMilliseconds;
}
