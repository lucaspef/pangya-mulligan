using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

[Collection("db")]
public class CaddieRentTests(DbFixture fx)
{
    [Fact]
    public async Task CaddieWithMonthlyFeeExpiresAndIsRenewedWithPang()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, _) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        Assert.Equal(4100, env.Data.CaddieMonthlyFee(0x1C000001));
        Assert.Equal(0, env.Data.CaddieMonthlyFee(0x1C000000));

        var (code, granted) = await new ShopService(env.Players.Store, env.Data).GiveAsync(p, 0x1C000001, 1);
        Assert.Equal(ShopCode.Ok, code);
        int id = granted[0].Id;
        var cad = p.Items[id];
        Assert.NotNull(cad.ExpiresAt);
        var s = PlayerStructs.Caddie(cad);
        Assert.Equal((1u, (ushort)30), (s.Rent_flag, s.Remain_Date));

        var actions = new PlayerActions(env.Players.Store, env.Data);
        Assert.Equal(CaddieRenewal.StillHired, (await actions.RenewCaddieAsync(p, id)).Result);

        // vence: o cliente vê 0 dias e o caddie não conta nos stats
        cad.ExpiresAt = DateTime.UtcNow.AddMinutes(-1);
        Assert.True(CaddieRent.IsExpired(cad, DateTime.UtcNow));
        Assert.Equal((ushort)0, PlayerStructs.Caddie(cad).Remain_Date);
        p.Pang = 4000;
        Assert.Equal(CaddieRenewal.NoPang, (await actions.RenewCaddieAsync(p, id)).Result);

        p.Pang = 5000;
        var (result, pang) = await actions.RenewCaddieAsync(p, id);
        Assert.Equal((CaddieRenewal.Ok, 900L), (result, pang));
        var saved = (await env.Players.LoadAsync(acc.Id))!;
        Assert.Equal(900, saved.Pang);
        Assert.True(saved.Items[id].ExpiresAt > DateTime.UtcNow.AddDays(29));
        Assert.Equal(CaddieRenewal.Fail, (await actions.RenewCaddieAsync(p, 12345)).Result);
    }

    [Fact]
    public void CaddieLevelsWithTheClientTable()
    {
        var c = new Item { Id = 1, TypeId = 0x1C000000 };
        var e = CaddieLevels.Gain(c, 600)!;                                       // 520 para o nível 2, sobra 80
        Assert.Equal((1, 80), (e.Int("level"), e.Int("exp")));
        e = CaddieLevels.Gain(e, 680 + 840)!;                                     // passa direto ao 4 (máximo, sem barra)
        Assert.Equal((3, 0), (e.Int("level"), e.Int("exp")));
        Assert.Null(CaddieLevels.Gain(e, 100));
        Assert.Null(CaddieLevels.Gain(c, 0));
        var s = PlayerStructs.Caddie(CaddieLevels.Gain(c, 700)!);
        Assert.Equal(((byte)1, 180u), (s.Level, s.Exp));
    }

    [Fact]
    public async Task EquippedCaddieGainsTheGameExp()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, _) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var (_, granted) = await new ShopService(env.Players.Store, env.Data).GiveAsync(p, 0x1C000000, 1);
        int id = granted[0].Id;
        p.Equip.CaddieId = id;
        var cfg = new Pangya.Core.Config.RewardConfig();
        var exp = new Rewards.ExpInput(4, 5, 0, false, 0);                       // 4 x 18 x 5 = 360
        var r = await Rewards.ApplyAsync(env.Players.Store, p, 100, 0, 18, true, cfg, expIn: exp);
        Assert.Equal(360, r.Exp);
        await Rewards.ApplyAsync(env.Players.Store, p, 100, 0, 18, true, cfg, expIn: exp);
        var saved = (await env.Players.LoadAsync(acc.Id))!.Items[id];
        Assert.Equal((1, 200), (saved.Int("level"), saved.Int("exp")));          // 720 - 520
        await Rewards.ApplyAsync(env.Players.Store, p, 100, 0, 3, false, cfg, expIn: exp);   // saiu antes: nada
        Assert.Equal(200, (await env.Players.LoadAsync(acc.Id))!.Items[id].Int("exp"));
    }

    [Fact]
    public void PartKeepsAnExpiredContractUsable()
    {
        var cad = new Item { Id = 1, TypeId = 0x1C000001, ExpiresAt = DateTime.UtcNow.AddHours(-1) };
        cad.Set("part", 0x20200001);
        cad.Attrs["part_until"] = DateTime.UtcNow.AddDays(3);
        Assert.False(CaddieRent.IsExpired(cad, DateTime.UtcNow));
        Assert.Equal(3, CaddieRent.RemainingDays(cad, DateTime.UtcNow));
        var free = new Item { Id = 2, TypeId = 0x1C000000 };
        Assert.False(CaddieRent.IsExpired(free, DateTime.UtcNow));
        Assert.Equal(0u, PlayerStructs.Caddie(free).Rent_flag);
    }

    [Fact]
    public async Task LoginWarnsAboutCaddiesOnVacation()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (acc, key) = await env.NewPlayerAsync();
        var p = (await env.Players.LoadAsync(acc.Id))!;
        var (_, granted) = await new ShopService(env.Players.Store, env.Data).GiveAsync(p, 0x1C000001, 1);
        var cad = p.Items[granted[0].Id].Clone();
        Assert.Equal(1, cad.Int("warning"));                                            // comprado: aviso ligado
        cad.ExpiresAt = DateTime.UtcNow.AddDays(-1);                                   // contrato vencido
        var ch = new PlayerChanges();
        ch.Updated.Add(cad);
        await env.Players.Store.ApplyAsync(acc.Id, ch);

        await using var c = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c, acc, key);
        var warn = await c.ExpectAsync(0xD2);
        Assert.Equal(1u, warn.U32());
        var info = warn.Struct<Pangya.Protocol.KR645.sCaddieInfo>();
        Assert.Equal(((uint)cad.Id, (ushort)0, 1u), (info.guid, info.Remain_Date, info.Rent_flag));

        await c.SendAsync(new PacketWriter(0x6B).U32((uint)cad.Id).U8(0));           // desmarcou a caixa
        await Task.Delay(200);
        await using var c2 = await env.ConnectAsync();
        await GameEnv.SendLoginAsync(c2, acc, await env.S.Sessions.IssueGameLoginAsync(acc.Id));
        var seen = new HashSet<ushort>();
        for (ushort id = 0; id != 0x94;) { (id, _) = await c2.ReceiveAsync(); seen.Add(id); }   // até o fim do login
        Assert.DoesNotContain((ushort)0xD2, seen);
    }

    [Fact]
    public async Task RehirePacketAnswersWithTheClientCodes()
    {
        _ = fx;
        await using var env = await GameEnv.StartAsync();
        var (c, _) = await LobbyClient.EnterAsync(env);
        await using var _c = c;
        var (code, recs) = await LobbyClient.BuyAsync(c, (0x1C000001, 0, 1));
        Assert.Equal(0u, code);
        await c.SendAsync(new PacketWriter(0x39).U32(recs[0].Id));
        Assert.Equal(1, (await c.ExpectAsync(0x91)).U8());                    // ainda contratado
        await c.SendAsync(new PacketWriter(0x39).U32(999999));
        Assert.Equal(0, (await c.ExpectAsync(0x91)).U8());
    }
}
