using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Premium (SPEC-jp983-efeitos.md §9): conferido no login e a cada 20 s (pega a compra do ticket e o vencimento). Ligou:
/// 0x9A com o bit 0x4000 e a Premium Comet completa (0x216). Venceu: tira o ticket e a bola (0x216), 0x9A sem o bit,
/// 0x26D (unix do fim) e o aviso 0x40 tipo 7.
/// </summary>
public sealed partial class JpSystems
{
    const ushort SCapability = 0x9A, SPremiumEnd = 0x26D;
    const byte NoticePremiumEnd = 7;
    static readonly TimeSpan PremiumCheck = TimeSpan.FromSeconds(20);
    bool premium;

    /// <summary>
    /// Uma vez por compra/extensão do ticket (marca "premium_end" no ticket): as caixas de presente pelo correio e, no VIP, o
    /// Lolo (Premium) e o club set premium com o prazo do ticket.
    /// </summary>
    async Task PremiumGrantedAsync(Item ticket, DateTime now)
    {
        string end = ticket.ExpiresAt!.Value.ToString("O");
        if (ticket.Attrs["premium_end"]?.GetValue<string>() == end) return;
        var p = kr.Player;
        var t = ticket.Clone();
        t.Attrs["premium_end"] = end;
        var ch = new PlayerChanges();
        ch.Updated.Add(t);
        await ctx.Players.Store.ApplyAsync(p.AccountId, ch);
        p.Items[t.Id] = t;
        if (kr.Context.Mail is { } mail)
            await mail.SendSystemAsync(p.AccountId, "@Premium", "Premium Gift Box", [(Premium.GiftBox, Premium.GiftBoxes(ticket.TypeId))]);
        if (ticket.TypeId != Premium.VipTicket) return;
        int days = Math.Max(1, (int)Math.Ceiling((ticket.ExpiresAt.Value - now).TotalDays));
        var given = new List<Item>();
        foreach (var extra in new[] { Premium.VipMascot, Premium.VipClubSet })
        {
            var have = p.FindType(extra);
            if (have != null && have.Int("premium") != 1) continue;                  // comprado de verdade: não mexe
            if (have == null)
            {
                var (code, granted) = await ctx.Shop.GiveAsync(p, extra, 1, days);
                if (code != ShopCode.Ok || granted.Count == 0 || p.Find(granted[0].Id) is not { } got) continue;
                have = got;
            }
            var e = have.Clone();
            e.Set("premium", 1);
            e.ExpiresAt = ticket.ExpiresAt;
            var c2 = new PlayerChanges();
            c2.Updated.Add(e);
            await ctx.Players.Store.ApplyAsync(p.AccountId, c2);
            p.Items[e.Id] = e;
            given.Add(e);
        }
        kr.SendNewItems(given);
        Log.Info($"{conn} JP: premium {(ticket.TypeId == Premium.VipTicket ? "VIP" : "Normal")}: caixas pelo correio" +
            (ticket.TypeId == Premium.VipTicket ? ", Lolo e club set premium" : ""));
    }

    async Task PremiumLoopAsync()
    {
        while (!conn.IsClosed)
        {
            try { await PremiumCheckAsync(); }
            catch (Exception e) { Log.Error($"{conn} premium: falha", e); }
            await Task.Delay(PremiumCheck);
        }
    }

    public async Task PremiumCheckAsync()
    {
        var p = kr.Player;
        var now = DateTime.UtcNow;
        var ticket = Premium.Ticket(p, now);
        if (ticket != null)
        {
            int cometTid = Premium.CometOf(ticket.TypeId);
            var comet = p.FindType(cometTid);
            if ((comet?.Quantity ?? 0) < Premium.CometStock)                         // a bola premium não acaba
            {
                int before = comet?.Quantity ?? 0;
                var (code, granted) = await ctx.Shop.GiveAsync(p, cometTid, Premium.CometStock - before);
                if (code == ShopCode.Ok && granted.Count > 0)
                    conn.SendTranslated(ItemsChanged([new ItemChange(cometTid, granted[0].Id, before, Premium.CometStock)]));
            }
            await PremiumGrantedAsync(ticket, now);
            if (!premium)
            {
                premium = true;
                conn.SendTranslated(new PacketWriter(SCapability).I32(p.IdentityFlags | Premium.Capability));
                if (p.Equip.BallTypeId is 0 or Item.BasicBall) await kr.EquipBallAsync(cometTid);   // a premium toma o lugar da básica
                Log.Info($"{conn} JP: premium até {ticket.ExpiresAt:yyyy-MM-dd HH:mm} UTC");
            }
            return;
        }
        var expired = Premium.Expired(p, now);
        if (!premium && expired.Count == 0) return;
        var changes = new List<ItemChange>();
        var ch = new PlayerChanges();
        DateTime end = now;
        foreach (var t in expired)
        {
            ch.Removed.Add(t.Id);
            changes.Add(new ItemChange(t.TypeId, t.Id, 1, 0));
            if (t.ExpiresAt is { } e && e < end) end = e;
        }
        foreach (var it in p.Items.Values)                                           // bolas premium e os extras do VIP
            if (it.Location == ItemLocation.Inventory && (it.TypeId is Premium.Comet or Premium.VipComet
                || it.Int("premium") == 1 && it.TypeId is Premium.VipMascot or Premium.VipClubSet))
            {
                ch.Removed.Add(it.Id);
                changes.Add(new ItemChange(it.TypeId, it.Id, Math.Max(it.Quantity, 1), 0));
            }
        await ctx.Players.Store.ApplyAsync(p.AccountId, ch);
        foreach (var id in ch.Removed) p.Items.Remove(id);
        conn.SendTranslated(new PacketWriter(SCapability).I32(p.IdentityFlags));     // antes do 0x216: sem o bit o cliente volta à básica
        if (changes.Count > 0) conn.SendTranslated(ItemsChanged(changes));
        if (p.Equip.BallTypeId is Premium.Comet or Premium.VipComet) await kr.EquipBallAsync(Item.BasicBall);
        conn.SendTranslated(new PacketWriter(SPremiumEnd).I32((int)new DateTimeOffset(end, TimeSpan.Zero).ToUnixTimeSeconds()));
        conn.SendTranslated(new PacketWriter(SNotice).U8(NoticePremiumEnd).Str(p.Nickname).Str(""));
        premium = false;
        Log.Info($"{conn} JP: premium acabou");
    }
}
