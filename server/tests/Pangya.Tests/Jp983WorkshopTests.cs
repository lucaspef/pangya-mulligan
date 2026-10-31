using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;

namespace Pangya.Tests;

/// <summary>Oficina de club set do JP 983 (SPEC-jp983-oficina.md): tabelas, nível, rank, transformação, reset e maestria.</summary>
public partial class Jp983GameTests
{
    /// <summary>Air Knight 2 (rank E, nível 4: o próximo passo é o rank-up) e um rank D nível 1.</summary>
    const int AirKnight2 = 0x10000001, Batter = 0x10000002;

    [Fact]
    public void WorkshopTablesAndRanks()
    {
        if (Jp983DataTests.Data is not { } d) return;
        Assert.Equal([500, 200, 800, 500, 600], d.LevelUpWeights(0)!);
        Assert.Equal([22, 14, 18, 9, 9], d.LevelUpLimit(4, 4)!);
        Assert.Equal([0, 1200, 5300, 17600, 35000, 90000], d.RankUpCost(3)!);
        Assert.Equal(0x1000005D, d.SpecialAt(d.ClubWorkshop[0x1000005D].Family, 1));
        Assert.True(d.ClubWorkshop[AirKnight2].Transform);
        Assert.Equal((-1, 1.5f), (d.ClubWorkshop[0x10000000].Type, d.ClubWorkshop[0x10000000].Rate));   // Air Knight Set: não evolui
        Assert.Equal((0, 0), (ClubWorkshop.RankOf(30), ClubWorkshop.LevelOf(30)));
        Assert.Equal((1, 4), (ClubWorkshop.RankOf(39), ClubWorkshop.LevelOf(39)));
        Assert.Equal((5, -1), (ClubWorkshop.RankOf(59), ClubWorkshop.RankOf(60)));
        Assert.Equal(180, 18 * ClubWorkshop.PerHole(GameMode.Stroke, false, false, 1f, 100, 100));
        Assert.Equal(540, 18 * ClubWorkshop.PerHole(GameMode.Stroke, false, false, 1f, 100, 300));   // Drops of Ebot
        Assert.Equal(15, ClubWorkshop.PerHole(GameMode.Stroke, true, false, 1f, 100, 100));          // Special Shuffle
        Assert.Equal(1, ClubWorkshop.PerHole(GameMode.Approach, false, false, 1.5f, 100, 100));
        Assert.Equal(0, ClubWorkshop.PerHole(GameMode.Tournament, false, true, 1f, 100, 100));       // Grand Zodiac
    }

    [Fact]
    public async Task WorkshopLevelsUpCancelsRecoversAndResets()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (Batter, 1), (ClubWorkshop.KitTypeId, 3), (ClubWorkshop.CoatingTypeId, 1),
            (ClubWorkshop.HardPowderTypeId, 1));
        await using var _ = g;
        var p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        int club = p.FindType(Batter)!.Id;

        await g.SendAsync(new PacketWriter(0x164).U32(ClubWorkshop.KitTypeId).U16(1).I32(club));   // subir nível: só o kit no 0x216
        var spent = await g.ExpectAsync(0x216);
        spent.U32();
        Assert.Equal(1u, spent.U32());
        Assert.Equal((2, (uint)ClubWorkshop.KitTypeId), (spent.U8(), spent.U32()));
        spent.Skip(8);
        Assert.Equal((3, 2), (spent.I32(), spent.I32()));
        var lv = await g.ExpectAsync(0x23D);
        Assert.Equal(0u, lv.U32());
        uint stat = lv.U32();
        Assert.True(stat < 5);

        await g.SendAsync(new PacketWriter(0x166));                                    // cancelar: volta e gasta 1 recuperação
        var club216 = await g.ExpectAsync(0x216);
        club216.Skip(8);
        Assert.Equal((0xCC, (uint)Batter, club), (club216.U8(), club216.U32(), club216.I32()));
        Assert.Equal(73 - 9, club216.Remaining);
        club216.Skip(16 + 25 + 10 + 4 + 1 + 4);
        Assert.Equal(1u, club216.U32());                                                // recuperação usada
        var cancel = await g.ExpectAsync(0x23F);
        Assert.Equal((0u, club), (cancel.U32(), cancel.I32()));

        await g.SendAsync(new PacketWriter(0x164).U32(ClubWorkshop.KitTypeId).U16(1).I32(club));
        await g.ExpectAsync(0x216);
        var lv2 = await g.ExpectAsync(0x23D);
        lv2.U32();
        stat = lv2.U32();
        await g.SendAsync(new PacketWriter(0x165));                                    // confirmar: fica
        await g.ExpectAsync(0x216);
        var ok = await g.ExpectAsync(0x23E);
        Assert.Equal((0u, stat, club), (ok.U32(), ok.U32(), ok.I32()));
        Assert.Equal(1, ClubWorkshop.Slots((await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!.Find(club)!)[stat]);

        await g.SendAsync(new PacketWriter(0x16B).U32(ClubWorkshop.CoatingTypeId).I32(club));   // spray: zera a recuperação
        await g.ExpectAsync(0x216);
        Assert.Equal(0u, (await g.ExpectAsync(0x246)).U32());
        await g.SendAsync(new PacketWriter(0x16B).U32(ClubWorkshop.CoatingTypeId).I32(club));   // sem spray: recusa
        Assert.NotEqual(0u, (await g.ExpectAsync(0x246)).U32());

        await g.SendAsync(new PacketWriter(0x16D).U32(ClubWorkshop.HardPowderTypeId).I32(club)); // reset: 0x216 (pó, 0xCC, 0xC9)
        var reset216 = await g.ExpectAsync(0x216);
        reset216.U32();
        Assert.Equal(3u, reset216.U32());
        var reset = await g.ExpectAsync(0x247);
        Assert.Equal((0u, (uint)Batter, club), (reset.U32(), reset.U32(), reset.I32()));
        p = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal([0, 0, 0, 0, 0], ClubWorkshop.Slots(p.Find(club)!));

        await g.SendAsync(new PacketWriter(0x172));                                    // ícone do evento
        Assert.Equal(0u, (await g.ExpectAsync(0x24E)).U32());
    }

    /// <summary>Rank-up no nível 4 gasta a maestria; com o sorteio forçado oferece o especial e a troca leva a maestria.</summary>
    [Fact]
    public async Task WorkshopRankUpCanTransformTheClub()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (AirKnight2, 1));
        await using var _ = g;
        var kr = env.Game.Context.Kr;
        var p = (await kr.Players.LoadAsync(acc.Id))!;
        var club = p.FindType(AirKnight2)!;
        var info = data.ClubWorkshop[AirKnight2];
        int sum = ClubWorkshop.Sum(info, new int[5]), rank = ClubWorkshop.RankOf(sum);
        var limit = data.LevelUpLimit(info.Type, rank)!;
        var c = new int[5];
        for (int need = 4 - ClubWorkshop.LevelOf(sum), i = 0; need > 0 && i < 5; i++)  // leva ao nível 4 sem passar dos limites
            while (need > 0 && info.Slot[i] + c[i] < limit[i] && i != 2) { c[i]++; need--; }
        var e = club.Clone();
        e.Set("wsc", c);
        e.Set("wsm", 100_000);
        var ch = new PlayerChanges();
        ch.Updated.Add(e);
        await kr.Players.Store.ApplyAsync(acc.Id, ch);
        p.Items[e.Id] = e;
        Assert.Equal(4, ClubWorkshop.Level(info, e));

        var ws = new ClubWorkshopService(kr.Players.Store, data, data, new Random(1)) { NoTransformWeight = 0 };
        var s = new WorkshopSession();
        Assert.Equal(0x5300200u + 10, (await ws.UpLevelAsync(p, s, ClubWorkshop.KitTypeId, 1, e.Id)).Code);   // nível 4: é rank-up
        var up = await ws.UpRankAsync(p, s, 0, 0, e.Id);
        Assert.Equal((0u, true, 2), (up.Code, up.Offer, up.Stat));                      // sem card: impacto; oferta de especial
        int cost = data.RankUpCost(info.RankUpRow)![rank + 1];
        Assert.Equal(100_000 - cost, ClubWorkshop.Mastery(p.Find(e.Id)!));
        Assert.Equal(rank + 1, ClubWorkshop.Rank(info, p.Find(e.Id)!));
        var tr = await ws.TransformAsync(p, s, true, async tid =>
        {
            var (code, granted) = await kr.Shop.GiveAsync(p, tid, 1);
            return code == ShopCode.Ok ? p.Find(granted[0].Id) : null;
        });
        Assert.Equal(0u, tr.Code);
        Assert.Contains(tr.TypeId, ClubWorkshopService.Specials.Concat([0x10000062, 0x10000063, 0x10000064, 0x10000065, 0x10000066,
            0x10000067, 0x10000068, 0x10000069, 0x1000006A, 0x1000006B, 0x1000006C, 0x1000006D]));
        Assert.Null(p.Find(e.Id));
        Assert.Equal(100_000 - cost, ClubWorkshop.Mastery(p.Find(tr.ClubId)!));

        // maestria da partida: 18 buracos no taco equipado
        p.Equip.ClubSetId = tr.ClubId;
        var gained = await ws.AddMasteryAsync(p, tr.ClubId, 18, GameMode.Stroke, false, false, 100);
        int perHole = ClubWorkshop.PerHole(GameMode.Stroke, false, false, data.ClubWorkshop[tr.TypeId].Rate, 100, 100);
        Assert.Equal(100_000 - cost + 18 * perHole, ClubWorkshop.Mastery(gained!));
    }

    /// <summary>Transferência: até 300 por chip, os dois tacos gravados.</summary>
    [Fact]
    public async Task WorkshopTransfersMastery()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (0x10000000, 1), (AirKnight2, 1), (ClubWorkshop.UcimChipTypeId, 5));
        await using var _ = g;
        var kr = env.Game.Context.Kr;
        var p = ((Pangya.Protocol.KR645.Game.GameHandler)env.Game.World.Find(acc.Id)!).Player;   // o jogador da conexão
        var from = p.FindType(0x10000000)!.Clone();
        from.Set("wsm", 700);
        var ch = new PlayerChanges();
        ch.Updated.Add(from);
        await kr.Players.Store.ApplyAsync(acc.Id, ch);
        p.Items[from.Id] = from;
        int to = p.FindType(AirKnight2)!.Id;
        await g.SendAsync(new PacketWriter(0x16C).U32(ClubWorkshop.UcimChipTypeId).I32(from.Id).I32(to).U32(4));   // 4 chips p/ 700: demais
        Assert.NotEqual(0u, (await g.ExpectAsync(0x245)).U32());
        await g.SendAsync(new PacketWriter(0x16C).U32(ClubWorkshop.UcimChipTypeId).I32(from.Id).I32(to).U32(3));
        var t216 = await g.ExpectAsync(0x216);
        t216.U32();
        Assert.Equal(3u, t216.U32());                                                    // chip, origem, destino
        Assert.Equal(0u, (await g.ExpectAsync(0x245)).U32());
        p = (await kr.Players.LoadAsync(acc.Id))!;
        Assert.Equal((0, 700), (ClubWorkshop.Mastery(p.Find(from.Id)!), ClubWorkshop.Mastery(p.Find(to)!)));
        Assert.Equal(2, p.FindType(ClubWorkshop.UcimChipTypeId)!.Quantity);
    }
}
