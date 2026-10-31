using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Relatório do Tiki (docs/server/protocolo/SPEC-jp983-tiki-report.md). C->S 0xAA (sPangYaUserStatistics 0x109; o cliente já
/// descontou 1 pergaminho e espera) -> saída da sala (0x4C 0xFFFF), pang (0x45, 0xC8) e o relatório (0xAA). C->S 0xAB u32
/// id do relatório, u32 Common[1]*0x8000 + Common[2] -> 0x11A i32 n, SYSTEMTIME, n × sCaddieReportData (0x83); erro: n = 0,
/// -1 (vencido) ou -2 (dados inválidos) e a data zerada. Depois do resultado, o EXP (0x10F, 0x45).
/// </summary>
public sealed partial class JpSystems
{
    /// <summary>Ids internos dos C->S 0xAA/0xAB (o tradutor troca; os do JP colidiriam com ids do KR).</summary>
    public const ushort CTikiReportUse = 0x7FAA, CTikiReportOpen = 0x7FAB;
    const ushort SReportItem = 0xAA, SReportOpen = 0x11A;
    /// <summary>sPangYaUserStatistics do 0xAA: EXP e pang que o cliente mostra (só para conferir com o do servidor).</summary>
    const int StatsExp = 0x4A, StatsPang = 0x4F;
    /// <summary>Equipe vencedora "nenhuma" (o 4º byte do 0x79).</summary>
    const byte NoWinner = 2;

    async Task<bool> HandleTikiReportAsync(PacketReader r)
    {
        if (r.Id == CTikiReportOpen)
        {
            int id = r.Remaining >= 4 ? r.I32() : 0;
            uint param = r.Remaining >= 4 ? r.U32() : 0;
            r.Skip(r.Remaining);
            await OpenTikiReportAsync(id, param);
            return true;
        }
        var stats = r.Bytes(r.Remaining).ToArray();
        var leave = kr.LeaveWithTikiReport();
        if (leave == null) return true;
        if (stats.Length >= StatsPang + 8)                                  // o servidor usa o próprio placar; só registra a diferença
        {
            long pang = BitConverter.ToInt64(stats, StatsPang), mine = leave.Me.Pang + leave.Me.Bonus;
            uint exp = BitConverter.ToUInt32(stats, StatsExp);
            if (pang != mine) Log.Info($"{conn} relatório do Tiki: o cliente diz {pang} pang/{exp} EXP, o servidor tem {mine} pang/{leave.Report.Exp} EXP");
        }
        if (await kr.CreateTikiReportAsync(leave) is { } it) conn.SendTranslated(ReportItem(it));
        return true;
    }

    /// <summary>
    /// 0xAA com o relatório (só o CLobbyTask trata): u16 1, sBuyItemResult (typeid, id, u16 horas, u8 tipo 2, u16 qtd,
    /// SYSTEMTIME fim, char[9]) e, para o 0x1A000042 novo, u16 Common[1], u16 Common[2], SYSTEMTIME (fim do prazo).
    /// </summary>
    public static PacketWriter ReportItem(Item it)
    {
        int hours = (int)Math.Ceiling(TikiReport.Lifetime.TotalHours);
        if (it.ExpiresAt is { } end) hours = Math.Max(1, (int)Math.Ceiling((end - DateTime.UtcNow).TotalHours));
        var end1 = it.ExpiresAt?.ToLocalTime();
        return new PacketWriter(SReportItem).U16(1).U32((uint)it.TypeId).I32(it.Id).U16((ushort)hours).U8(2).U16(1).Time(end1).Zeros(9)
            .U16((ushort)it.Int("c1")).U16((ushort)it.Int("c2")).Time(end1);
    }

    async Task OpenTikiReportAsync(int id, uint param)
    {
        var (code, d, before) = await kr.OpenTikiReportAsync(id, param);
        if (d == null)
        {
            conn.SendTranslated(new PacketWriter(SReportOpen).I32(code == TikiReport.Ok ? TikiReport.Fail : code).Zeros(16));
            Log.Info($"{conn} relatório do Tiki {id} recusado ({code})");
            return;
        }
        conn.SendTranslated(ReportResult(d, (uint)kr.Player.AccountId));
        await kr.AfterTikiExpAsync(before);
    }

    /// <summary>0x11A com as linhas; a minha (o cliente acha pelo nick) leva EXP, troféu da sala, modo e o meu troféu.</summary>
    public static PacketWriter ReportResult(TikiReport.Data d, uint me)
    {
        var w = new PacketWriter(SReportOpen, 24 + d.Rows.Count * 0x83).I32(d.Rows.Count).Time(d.Date.ToLocalTime());
        foreach (var r in d.Rows)
        {
            w.U32(r.Uid).I64(r.Pang).I64(r.Bonus).U32((uint)d.MatchTid).U32(r.Uid == me ? (uint)d.Exp : 0).U32((uint)r.Mascot)
                .U8((byte)(r.Premium ? 1 : 0)).U8(0).U16((ushort)r.Level).Zeros(2).U8((byte)(sbyte)Math.Clamp(r.Score, -128, 127))
                .U8((byte)(r.Medals & 0x3F)).U8((byte)r.Trophy).Zeros(22).Fixed(r.Nick, 22).Zeros(4).U32((uint)r.GuildId).U32(0)
                .Fixed(r.GuildMark, 12).U8((byte)d.Mode).Zeros(3).U8((byte)(r.Left ? 4 : 0)).U8(NoWinner).Zeros(16);
        }
        return w;
    }
}
