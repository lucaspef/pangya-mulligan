using Pangya.Protocol.JP983.Game;
using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Tests;

/// <summary>Big Papel e Memorial Shop do JP 983 (SPEC-jp983-papel-memorial.md), pelo cliente JP falso.</summary>
public partial class Jp983GameTests
{
    /// <summary>Conta nova com pang e itens, já no canal.</summary>
    static async Task<(TestClient G, Domain.Accounts.Account Acc)> InChannelWithAsync(JpEnv env, long pang, params (int Tid, int Qty)[] items)
    {
        var (login, acc) = await env.NewAccountAsync();
        var ch = new PlayerChanges { Pang = pang };
        var ids = await env.S.Players.NewIdsAsync(Math.Max(items.Length, 1));
        for (int i = 0; i < items.Length; i++) ch.Added.Add(new Item { Id = ids[i], TypeId = items[i].Tid, Quantity = items[i].Qty });
        await env.S.Players.ApplyAsync(acc.Id, ch);
        var key = await env.S.Sessions.IssueGameLoginAsync(acc.Id);
        var g = await env.ConnectGameAsync();
        await g.SendAsync(GameLogin(login, acc.Id, key));
        await g.ExpectAsync(0x96);
        await g.SendAsync(Jp983Client.Build(0x04, "FUN_00af5e00", (byte)0));
        await g.ExpectAsync(0x4E);
        return (g, acc);
    }

    [Fact]
    public async Task BigPapelGivesTenBallsForPang()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 100_000);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x186));
        Assert.Equal((ulong)(100_000 - 9000), (await g.ExpectAsync(0xC8)).U64());     // pang na tela
        var times = await g.ExpectAsync(0xFB);
        Assert.Equal((-1, -3), (times.I32(), times.I32()));
        var r = await g.ExpectAsync(0x26C);
        Assert.Equal((0u, 0, 10u), (r.U32(), r.I32(), r.U32()));
        r.Skip(10 * 20);
        long pang = 100_000 - 9000;
        Assert.Equal((ulong)pang, r.U64());
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal(pang, p.Pang);
    }

    [Fact]
    public async Task BigPapelWithoutPangIsRefused()
    {
        await using var env = new JpEnv();
        var (g, _) = await InChannelWithAsync(env, 100);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x186));
        var r = await g.ExpectAsync(0x26C);
        Assert.Equal((2u, 0), (r.U32(), r.Remaining));
    }

    [Fact]
    public void MemorialTablesComeFromTheJpData()
    {
        if (Jp983DataTests.Data is not { } d) return;
        Assert.Equal(14, d.MemorialCoins.Count);
        var premium = d.MemorialCoins[0x1A000272];
        Assert.Equal((1, 100), (premium.Kind, premium.Bonus));
        Assert.Equal(705, d.MemorialRares.Count);
        var memorial = new Domain.Shop.MemorialService(null!, d, new());
        Assert.Equal(705, memorial.PoolOf(d.MemorialCoins[0x1A000273]).Count);    // Normal: todos
        Assert.Equal(15, memorial.PoolOf(d.MemorialCoins[0x1A000276]).Count);     // Club Set: filtro 5
        Assert.Empty(memorial.PoolOf(d.MemorialCoins[0x1A0002AB]));              // personagem: só consolação
    }

    [Fact]
    public async Task MemorialSpendsACoinAndGivesOnePrize()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (0x1A000273, 2), (0x1A0002AB, 1));
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x17F).U32(0x1A000273));                   // Memorial Coin (Normal)
        var changed = await g.ExpectAsync(0x216);
        changed.U32();
        int n = (int)changed.U32();
        Assert.Equal(n * 50, changed.Remaining);
        Assert.Equal((2, 0x1A000273u), (changed.U8(), changed.U32()));
        changed.Skip(4 + 4);
        Assert.Equal((2, 1), (changed.I32(), changed.I32()));                        // moeda 2 -> 1
        var r = await g.ExpectAsync(0x264);
        Assert.Equal((0u, 1u), (r.U32(), r.U32()));
        int type = r.I32();
        Assert.InRange(type, -1, 4);
        int prize = (int)r.U32();
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal(1, p.FindType(0x1A000273)!.Quantity);
        Assert.True(p.FindType(prize) != null || Item.GroupOf(prize) == ItemGroup.SetItem, $"prêmio {prize:X8}");

        await g.SendAsync(new PacketWriter(0x17F).U32(0x1A0002AB));                   // moeda de personagem: consolação
        await g.ExpectAsync(0x216);
        r = await g.ExpectAsync(0x264);
        Assert.Equal((0u, 1u, -1), (r.U32(), r.U32(), r.I32()));                    // consolação: -1, como a referência

        await g.SendAsync(new PacketWriter(0x17F).U32(0x1A000272));                   // não tem a Premium
        r = await g.ExpectAsync(0x264);
        Assert.Equal((0x303u, 0), (r.U32(), r.Remaining));
    }

    /// <summary>
    /// Papel Shop sem limite por dia nos três lugares que o cliente JP lê (com 0 ele mostra "Bonus Turns" e fecha a loja):
    /// MemberInfo (restantes/usadas/limite 0xFFFF), 0x10B (limite 0xFFFF) e 0xFB depois da jogada (-1, -3).
    /// </summary>
    [Fact]
    public async Task PapelShopHasNoDailyLimit()
    {
        using (var m = new PacketWriter(0).MemberInfo(new Player { Login = "a", Nickname = "b" }))
            Assert.Equal(new byte[] { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, m.Body.Slice(2 + 2 + 147, 6).ToArray());
        await using var env = new JpEnv();
        var (g, _) = await InChannelWithAsync(env, 100_000);
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x98));
        var open = await g.ExpectAsync(0x10B);
        Assert.Equal((0u, 0xFFFFul, 0), (open.U32(), open.U64(), open.Remaining));
        await g.SendAsync(new PacketWriter(0x14B));
        Assert.Equal(0u, (await g.ExpectAsync(0x21B)).U32());
        var times = await g.ExpectAsync(0xFB);
        Assert.Equal((-1, -3), (times.I32(), times.I32()));
    }

    /// <summary>Papel com cupom: o JP desconta pela pilha no 0x216 e pelo id no 0x21B (não usa o 0xD5).</summary>
    [Fact]
    public async Task PapelWithACouponShowsTheCouponSpent()
    {
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (0x1A000028, 3));
        await using var _ = g;
        int coupon = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!.FindType(0x1A000028)!.Id;
        await g.SendAsync(new PacketWriter(0x14B));
        var ids = new List<ushort>();
        PacketReader? changed = null, r = null;
        while (r == null)
        {
            var (id, pk) = await g.ReceiveAsync();
            ids.Add(id);
            if (id == 0x216) changed = pk;
            if (id == 0x21B) r = pk;
        }
        Assert.DoesNotContain((ushort)0xD5, ids);                                     // o JP não usa o 0xD5
        Assert.NotNull(changed);                                                      // 0x216 antes do resultado
        changed!.Skip(4);
        Assert.Equal(1u, changed.U32());
        Assert.Equal((2, 0x1A000028u, coupon), (changed.U8(), changed.U32(), changed.I32()));
        changed.U32();
        // depois = a pilha real (o cupom também é um prêmio do Papel: pode voltar na mesma jogada)
        int after = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!.Find(coupon)?.Quantity ?? 0;
        Assert.Equal((3, after, after - 3), (changed.I32(), changed.I32(), changed.I32()));
        Assert.Equal((0u, coupon), (r.U32(), r.I32()));
    }
}
