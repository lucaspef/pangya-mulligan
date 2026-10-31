using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Drop por buraco (SPEC-jp983-eventos-drops.md §3): ao acertar o buraco, 0xCC i32 oid, u8 n, 128 × (u32 typeid, u8 mapa,
/// u8 buraco, i16 qtd, u64 tipo 1) só ao jogador, com os itens novos (o cliente acumula); no fim da partida terminada os
/// itens entram no inventário (0x216). Quem sai no meio não leva.
/// </summary>
public sealed partial class JpSystems
{
    const ushort SHoleDrop = 0xCC;
    const int DropSlots = 128;
    readonly List<HoleDrop> drops = [];
    object? dropGame;
    int holedCount;

    /// <summary>Chamado pelo KR sob o lock da sala quando o jogador acerta o buraco.</summary>
    public void OnHoled(byte hole)
    {
        if (kr.CurrentRoom is not { Game: { } g } room) return;
        var s = room.Settings;
        if (s.Practice || s.Zodiac != ZodiacKind.None || s.Mode is GameMode.Approach or GameMode.NewApproach) return;
        if (dropGame != g) { dropGame = g; drops.Clear(); holedCount = 0; }
        holedCount++;
        byte course = room.HoleCourses.Length == 18 && hole is >= 1 and <= 18 ? room.HoleCourses[Math.Max(0, holedCount - 1)] : room.CoursePlayed;
        var list = HoleDrops.Roll(kr.Context.World.Config.HoleDrops, kr.Player, course, hole, holedCount, s.Holes, room.GrandPrix != null, Random.Shared,
            s.Artifact, (int)kr.Context.Data.CourseStars(course & 0x7F));
        if (list.Count == 0 || drops.Count + list.Count > DropSlots) return;
        drops.AddRange(list);
        uint oid = room.Find(kr)?.Guid ?? (uint)kr.Player.AccountId;
        var w = new PacketWriter(SHoleDrop, 8 + DropSlots * 16).I32((int)oid).U8((byte)list.Count);
        foreach (var d in list) w.U32((uint)d.TypeId).U8((byte)(d.Course & 0x7F)).U8(d.Hole).I16(d.Quantity).U64(d.Kind);
        w.Zeros((DropSlots - list.Count) * 16);
        conn.SendTranslated(w);
    }

    /// <summary>Fim da partida: os drops vão para o inventário (só partida terminada).</summary>
    public async Task CreditDropsAsync(Domain.Achievements.AchievementCounters.GameEnd g)
    {
        if (drops.Count == 0) return;
        var list = new List<HoleDrop>(drops);
        drops.Clear();
        dropGame = null;
        if (!g.Finished) return;
        try
        {
            var p = kr.Player;
            var changes = new List<ItemChange>();
            long pang = 0;
            foreach (var d in list)
            {
                if (d.Kind == HoleDrop.PangPouch500) { pang += (long)d.Quantity * HoleDrops.PangPerPouch; continue; }   // bolsas: saldo
                int before = p.FindType(d.TypeId)?.Quantity ?? 0;
                var (code, granted) = await ctx.Shop.GiveAsync(p, d.TypeId, d.Quantity);
                if (code == ShopCode.Ok && granted.Count > 0) changes.Add(new ItemChange(d.TypeId, granted[0].Id, before, p.Find(granted[0].Id)?.Quantity ?? d.Quantity));
            }
            if (changes.Count > 0) conn.SendTranslated(ItemsChanged(changes));
            if (pang > 0)
            {
                await kr.Context.Players.Store.ApplyAsync(p.AccountId, new Domain.Players.PlayerChanges { Pang = p.Pang + pang });
                p.Pang += pang;
                conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
                Log.Info($"{conn} JP: +{pang} pang do artefato");
            }
            Log.Info($"{conn} JP: {list.Count} item(ns) do copo entregue(s)");
        }
        catch (Exception e) { Log.Error($"{conn} falha nos drops do copo", e); }
    }
}
