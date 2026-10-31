using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Premium ticket do JP 983 (SPEC-jp983-efeitos.md §9): loja por prazo, ativação no login, vencimento.</summary>
public partial class Jp983GameTests
{
    async Task<(TestClient G, Domain.Accounts.Account Acc)> LoginWithTicketAsync(JpEnv env, DateTime expires, int ticket = Premium.NormalTicket)
    {
        var (login, acc) = await env.NewAccountAsync();
        var ch = new PlayerChanges();
        ch.Added.Add(new Item { Id = (await env.S.Players.NewIdsAsync(1))[0], TypeId = ticket, Quantity = 1, ExpiresAt = expires });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        return (g, acc);
    }

    /// <summary>0x4B tipo 2 (troca de bola): u32 resultado, u8 tipo, u32 guid, u32 bola.</summary>
    static async Task<uint> PremiumBallAsync(TestClient g)
    {
        var w = await g.ExpectAsync(0x4B);
        w.U32();
        Assert.Equal(2, w.U8());
        w.U32();
        return w.U32();
    }

    [Fact]
    public async Task PremiumEndRestoresTheDefaultBall()
    {
        if (Jp983DataTests.Data is null) return;
        await using var env = new JpEnv();
        var (login, acc) = await env.NewAccountAsync();
        var ids = await env.S.Players.NewIdsAsync(2);
        var p = (await env.S.Players.LoadAsync(acc.Id))!;
        p.Equip.BallTypeId = Premium.VipComet;
        var ch = new PlayerChanges { Equip = p.Equip };
        ch.Added.Add(new Item { Id = ids[0], TypeId = Premium.VipTicket, Quantity = 1, ExpiresAt = DateTime.UtcNow.AddMinutes(-1) });
        ch.Added.Add(new Item { Id = ids[1], TypeId = Premium.VipComet, Quantity = Premium.CometStock });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        await using var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        Assert.Equal((uint)Item.BasicBall, await PremiumBallAsync(g));
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        Assert.Equal(Item.BasicBall, h.Player.Equip.BallTypeId);
        Assert.Null(h.Player.FindType(Premium.VipComet));
    }

    [Fact]
    public async Task PremiumTicketTurnsOnAndExpires()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var shopItem = data.GetShopItem(Premium.NormalTicket)!;
        Assert.Equal((25L, 80L, 800L), (ShopService.Price(shopItem, 1, 7)!.Value, ShopService.Price(shopItem, 1, 30)!.Value,
            ShopService.Price(shopItem, 1, 365)!.Value));
        Assert.True(shopItem.IsCash);

        await using var env = new JpEnv();
        var (g, acc) = await LoginWithTicketAsync(env, DateTime.UtcNow.AddDays(3));
        await using (g)
        {
            Assert.Equal(Premium.Capability, (await g.ExpectAsync(0x9A)).I32() & Premium.Capability);
            var h = (GameHandler)env.Game.World.Find(acc.Id)!;
            Assert.Equal(Premium.CometStock, h.Player.FindType(Premium.Comet)!.Quantity);
            Assert.Equal((uint)Premium.Comet, await PremiumBallAsync(g));                    // a premium toma o lugar da básica
            Assert.Equal(Premium.Comet, h.Player.Equip.BallTypeId);
            var gp = new GrandPrixDef(1, 1, 0, 0, "x", 0x1A000264, 1, false, false, 0, 0, 0, 3, 0, 0, (0, 0, 0), 0, [], -1, -1, 0, false);
            Assert.Equal(GrandPrix.Ok, GrandPrix.CanEnter(h.Player, gp, [], DateTime.Now));   // GP sem ticket
            var more = await env.Game.Context.Kr.Shop.GiveAsync(h.Player, Premium.NormalTicket, 1, 30);   // comprar de novo soma
            Assert.InRange((more.Granted[0].End!.Value - DateTime.UtcNow).TotalDays, 32.9, 33.1);
        }

        var (g2, acc2) = await LoginWithTicketAsync(env, DateTime.UtcNow.AddMinutes(-1));   // vencido: sai no login
        await using (g2)
        {
            await g2.ExpectAsync(0x26D);
            var h2 = (GameHandler)env.Game.World.Find(acc2.Id)!;
            Assert.Null(h2.Player.FindType(Premium.NormalTicket));
        }
    }

    [Fact]
    public async Task VipBringsLoloBoxesAndBonus()
    {
        if (Jp983DataTests.Data is null) return;
        await using var env = new JpEnv();
        var (g, acc) = await LoginWithTicketAsync(env, DateTime.UtcNow.AddDays(10), Premium.VipTicket);
        await using var _ = g;
        await g.ExpectAsync(0x9A);
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        for (int i = 0; i < 50 && h.Player.FindType(Premium.VipMascot) == null; i++) await Task.Delay(20);
        var lolo = h.Player.FindType(Premium.VipMascot)!;
        Assert.Equal(1, lolo.Int("premium"));
        Assert.NotNull(h.Player.FindType(Premium.VipComet));
        Assert.Equal(10, Premium.RatePercent(h.Player, DateTime.UtcNow));
        var (mails, _) = await env.S.Mail.ListAsync(acc.Id, 1, 20);
        var box = Assert.Single(mails);
        Assert.Equal((Premium.GiftBox, 8), (box.Items[0].TypeId, box.Items[0].Quantity));

        await env.Game.Context.Kr.Shop.GiveAsync(h.Player, Premium.GiftBox, 1);       // abrir a caixa (JP 0xEF -> KR 0xF1)
        await g.SendAsync(new PacketWriter(0xEF).U32(Premium.GiftBox));
        var opened = await g.ExpectAsync(0x19D);
        Assert.Equal((0u, (uint)Premium.GiftBox), (opened.U32(), opened.U32()));
        Assert.Null(h.Player.FindType(Premium.GiftBox));
    }
}
