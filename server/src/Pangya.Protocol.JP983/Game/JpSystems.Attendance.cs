using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Presença diária (docs/server/protocolo/SPEC-jp983-sistemas.md): C->S 0x16E (ao abrir a lista de canais) -> 0x248 i32 0 +
/// ARI (u8 já registrado hoje, u32 prêmio de hoje, u32 qtd, u32 próximo prêmio, u32 qtd, u32 dias); com 0 o cliente
/// mostra a janela do dia e depois manda 0x16F -> 0x249 (mesmo layout, já registrado). O próximo prêmio aparece ao sair.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CAttendance = 0x16E, CAttendanceAck = 0x16F;
    const ushort SAttendance = 0x248, SAttendanceAck = 0x249;

    AttendanceResult? attendance;

    async Task AttendanceAsync()
    {
        if (attendanceService == null) { conn.SendTranslated(Ari(SAttendance, new AttendanceResult(false, 0, (0, 0), (0, 0)))); return; }
        var p = kr.Player;
        var r = await attendanceService.CheckInAsync(p, DateTime.Now);
        attendance = r;
        conn.SendTranslated(Ari(SAttendance, r));
        if (!r.NewDay) return;
        if (ctx.Mail != null) kr.Connection.Send(new PacketWriter(0x31));        // prêmio por carta: carta nova (KR 0x31 -> JP 0x33)
        else if (r.Now.TypeId == AttendancePangPouch) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
        else if (r.Now.TypeId != 0 && p.FindType(r.Now.TypeId) is { } it)
        {
            int after = it.IsConsumable ? it.Quantity : 1;
            conn.SendTranslated(ItemsChanged([new ItemChange(it.TypeId, it.Id, Math.Max(0, after - r.Now.Quantity), after)]));
        }
        await RecordAsync(new Domain.Achievements.CounterBag().Add(Domain.Achievements.AchievementCounters.LoginDay));
        Log.Info($"{conn} JP: presença dia {r.Days}: 0x{r.Now.TypeId:X8} x{r.Now.Quantity}; próximo 0x{r.Next.TypeId:X8} x{r.Next.Quantity}");
    }

    const int AttendancePangPouch = 0x1A000010;

    static PacketWriter Ari(ushort id, AttendanceResult r, bool registered = false) => new PacketWriter(id).I32(0)
        .U8((byte)(registered || !r.NewDay ? 1 : 0)).U32((uint)r.Now.TypeId).U32((uint)r.Now.Quantity)
        .U32((uint)r.Next.TypeId).U32((uint)r.Next.Quantity).U32((uint)r.Days);
}
