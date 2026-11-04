using Pangya.Core.Logging;
using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Bingo do lobby (CLobbyTask 0x18B/0x18C): 0xE3 u32 0 abre, u32 1 sorteia. 0x18B u32 erro (10 = sem ficha) ou 0 + sBingoInfo
/// {u32 tipo, i32 fichas, u32 números, u32 linhas}; no sorteio (tipo 1) vem u32 número, u32 1, u32 linhas novas. Linha nova
/// manda o 0x18C u32 linhas, u32 n, n × u32 typeid (janela de prêmio); os itens vão por carta.
/// </summary>
public sealed partial class JpSystems
{
    const ushort SBingo = 0x18B, SBingoGift = 0x18C;
    const uint BingoOpen = 0, BingoDraw = 1;

    public Domain.Players.BingoService? Bingo { get; set; }

    async Task<bool> HandleBingoAsync(PacketReader r)
    {
        uint mode = r.Remaining >= 4 ? r.U32() : BingoOpen;
        r.Skip(r.Remaining);
        if (Bingo is not { } bingo || !bingo.Open(Today))
        {
            conn.SendTranslated(new PacketWriter(SBingo).U32(Domain.Players.BingoService.NoTries));
            return true;
        }
        long uid = kr.Player.AccountId;
        if (mode != BingoDraw)
        {
            var cur = await bingo.GetAsync(uid);
            conn.SendTranslated(BingoInfo(BingoOpen, cur).U32(0));
            return true;
        }
        var d = await bingo.DrawAsync(uid, Today);
        if (d.Code != 0) { conn.SendTranslated(new PacketWriter(SBingo).U32((uint)d.Code)); return true; }
        conn.SendTranslated(BingoInfo(BingoDraw, d.Progress).U32((uint)d.Number).U32(1).U32((uint)d.NewLines));
        if (d.NewLines != 0)
        {
            var prizes = bingo.Prizes(d.NewLines);
            var w = new PacketWriter(SBingoGift).U32((uint)d.Progress.Lines).U32((uint)prizes.Count);
            foreach (var (tid, _) in prizes) w.U32((uint)tid);
            conn.SendTranslated(w);
            if (ctx.Mail != null) kr.Connection.Send(new PacketWriter(0x31));             // carta nova (KR 0x31 -> JP 0x33)
        }
        Log.Info($"{conn} JP: bingo número {d.Number}, linhas 0x{d.Progress.Lines:X3} (novas 0x{d.NewLines:X3}), fichas {d.Progress.Tries}");
        return true;
    }

    static PacketWriter BingoInfo(uint type, Domain.Players.BingoProgress p) => new PacketWriter(SBingo).U32(0)
        .U32(type).I32(p.Tries).U32((uint)p.Aimed).U32((uint)p.Lines);

    public async Task BingoGameEndAsync(Domain.Achievements.AchievementCounters.GameEnd g)
    {
        if (Bingo is not { } bingo) return;
        try
        {
            if (await bingo.OnGameEndAsync(kr.Player.AccountId, g.Practice, g.Holes, g.Finished, Today) is { } next)
                Log.Info($"{conn} JP: bingo +{bingo.Config.TriesPerGame} ficha (total {next.Tries})");
        }
        catch (Exception e) { Log.Error($"{conn} falha no bingo", e); }
    }
}
