using Pangya.Core.Logging;
using Pangya.Core.Net;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// World Tour (docs/server/protocolo/SPEC-jp983-world-tour.md): 0x11D u32 campos (bit por campo), u8 já pegou o prêmio, no login
/// e quando um campo acende; com 0xFFFF e sem prêmio o cliente mostra o botão do presente. "Sim" manda o 0xAD u32 guid
/// (aqui com id interno) e o cliente já se marca como premiado: o servidor só confere e manda os itens por carta.
/// </summary>
public sealed partial class JpSystems
{
    /// <summary>Id interno do 0xAD do JP (o tradutor troca, porque o 0xAD viraria o KR 0xA5).</summary>
    public const ushort CWorldTourGift = 0x7FAD;
    const ushort SWorldTour = 0x11D;

    public Domain.Players.WorldTourService? WorldTour { get; set; }

    static DateOnly Today => DateOnly.FromDateTime(DateTime.Now);

    public async Task SendWorldTourAsync()
    {
        if (WorldTour is not { } wt || !wt.Open(Today)) return;
        var cur = await wt.GetAsync(kr.Player.AccountId);
        conn.SendTranslated(new PacketWriter(SWorldTour).U32((uint)cur.MapFlag).U8(cur.GotGift ? (byte)1 : (byte)0));
    }

    public async Task WorldTourGameEndAsync(Domain.Achievements.AchievementCounters.GameEnd g)
    {
        if (WorldTour is not { } wt) return;
        try
        {
            if (await wt.OnGameEndAsync(kr.Player.AccountId, g.Mode, g.Practice, g.Course, g.Holes, g.Finished, Today) is not { } next) return;
            conn.SendTranslated(new PacketWriter(SWorldTour).U32((uint)next.MapFlag).U8(next.GotGift ? (byte)1 : (byte)0));
            Log.Info($"{conn} JP: World Tour campo {g.Course} completo (0x{next.MapFlag:X4})");
        }
        catch (Exception e) { Log.Error($"{conn} falha no World Tour", e); }
    }

    async Task WorldTourGiftAsync()
    {
        if (WorldTour is not { } wt) return;
        bool ok = await wt.ClaimAsync(kr.Player.AccountId, Today);
        Log.Info($"{conn} JP: prêmio do World Tour {(ok ? "entregue por carta" : "recusado (faltam campos ou já pegou)")}");
    }
}
