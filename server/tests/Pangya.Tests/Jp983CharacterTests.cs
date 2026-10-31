using Pangya.Core.Net;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;
using Pangya.Protocol.JP983.Game;
using Pangya.Protocol.KR645.Game;

namespace Pangya.Tests;

/// <summary>Cards no personagem, maestria, stat ±1, Lolo e card por tempo no JP 983 (SPEC-jp983-cards.md).</summary>
public partial class Jp983GameTests
{
    const int Remover = 0x1A0000C2;

    static List<CardInfo> CardsOf(Pangya.Protocol.JP983.Jp983GameData d, int sub, int rarity) =>
        d.Cards.Values.Where(c => c.Final && c.SubType == sub && c.Rarity == rarity && c.Volume is >= 1 and <= 5).OrderBy(c => c.TypeId).ToList();

    [Fact]
    public void CharacterTablesFromTheJpData()
    {
        if (Jp983DataTests.Data is not { } d) return;
        var nuri = d.MasterySteps(0x04000000);
        Assert.Equal(10, nuri.Count);
        Assert.Equal((1, 0, 0, (CharacterSystems.TikiPointsTypeId, 2)), (nuri[0].Seq, nuri[0].Stat, nuri[0].Level, nuri[0].Conditions[0]));
        Assert.Contains(d.Cards.Values, c => c.SubType == CardInfo.SubNpc);
        Assert.Contains(d.Cards.Values, c => c.Com is { } com && com.Any(v => v != 0));
    }

    [Fact]
    public async Task CardsGoIntoTheCharacterSlotsAndComeBack()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var card = CardsOf(data, CardInfo.SubCharacter, 0)[0].TypeId;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (card, 3), (CharacterSystems.PatcherTypeId, 1), (Remover, 1));
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        var ch = h.Player.Find(h.Player.Equip.CharacterId)!;
        int cardId = h.Player.FindType(card)!.Id;

        await g.SendAsync(Jp983Client.Build(0x18A, "FUN_005bbab0", (uint)ch.TypeId, (uint)ch.Id, (uint)card, (uint)cardId, 1u));
        var e = await g.ExpectAsync(0x216);
        e.U32();
        Assert.Equal(2u, e.U32());
        e.Skip(50);
        Assert.Equal((0xCB, (uint)ch.TypeId, ch.Id), (e.U8(), e.U32(), e.I32()));
        e.Skip(36);
        Assert.Equal(((uint)card, (byte)1), (e.U32(), e.U8()));
        var ok = await g.ExpectAsync(0x271);
        Assert.Equal((0u, (uint)card), (ok.U32(), ok.U32()));
        Assert.Equal(card, CharacterSystems.Cards(h.Player, ch.Id)[0]);

        await g.SendAsync(new PacketWriter(0x18A).U32((uint)ch.TypeId).I32(ch.Id).U32((uint)card).I32(cardId).U32(1));   // ocupado
        Assert.NotEqual(0u, (await g.ExpectAsync(0x271)).U32());
        await g.SendAsync(new PacketWriter(0x18A).U32((uint)ch.TypeId).I32(ch.Id).U32((uint)card).I32(cardId).U32(4));   // 4 pede patcher
        Assert.NotEqual(0u, (await g.ExpectAsync(0x271)).U32());
        await g.SendAsync(new PacketWriter(0x18B).U32((uint)ch.TypeId).I32(ch.Id).U32((uint)card).I32(cardId).U32(4));   // com patcher
        var pe = await g.ExpectAsync(0x216);
        pe.U32();
        Assert.Equal(3u, pe.U32());                                                       // card, patcher, slot
        Assert.Equal(0u, (await g.ExpectAsync(0x272)).U32());
        Assert.Null(h.Player.FindType(CharacterSystems.PatcherTypeId));

        // o personagem no JP leva os cards (465..) e a lista de ativos (0x137) os dois slots
        using (var info = new PacketWriter(0).EquipedItems(h.Player))
            Assert.Equal((uint)card, BitConverter.ToUInt32(info.Body.Slice(2 + 465, 4)));

        await g.SendAsync(Jp983Client.Build(0x18C, "FUN_005bbc50", (uint)ch.TypeId, (uint)ch.Id, (uint)Remover, (uint)h.Player.FindType(Remover)!.Id, 1u));
        PacketReader re;
        do { re = await g.ExpectAsync(0x216); re.U32(); } while (re.U32() != 3);          // o 0x216 das conquistas pode vir antes
        // removedor, card de volta, slot vazio
        Assert.Equal((0u, (uint)card), (await g.ExpectAsync(0x273)).Let(r => (r.U32(), r.U32())));
        Assert.Equal(2, h.Player.FindType(card)!.Quantity);                                // 3 - 2 encaixados + 1 de volta
        Assert.Equal(0, CharacterSystems.Cards(h.Player, ch.Id)[0]);
    }

    [Fact]
    public async Task MasteryAndStatsFromTheMasteryScreen()
    {
        if (Jp983DataTests.Data is not { } data) return;
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 100_000, (CharacterSystems.TikiPointsTypeId, 10));
        await using var _ = g;
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        var ch = h.Player.Find(h.Player.Equip.CharacterId)!;
        var steps = data.MasterySteps(ch.TypeId);
        Assert.NotEmpty(steps);

        await g.SendAsync(Jp983Client.Build(0x187, "FUN_005c16c0", (uint)ch.TypeId, (uint)ch.Id));
        var m = await g.ExpectAsync(0x216);
        m.U32();
        Assert.Equal(2u, m.U32());                                                        // TP gasto e a maestria (0xCD)
        m.Skip(50);
        Assert.Equal((0xCD, (uint)ch.TypeId), (m.U8(), m.U32()));
        m.Skip(45);
        Assert.Equal(1u, m.U32());
        Assert.Equal(0u, (await g.ExpectAsync(0x26E)).U32());
        Assert.Equal(1, CharacterSystems.Mastery(h.Player.Find(ch.Id)!));
        Assert.Equal(10 - steps[0].Conditions[0].Qty, h.Player.FindType(CharacterSystems.TikiPointsTypeId)!.Quantity);

        int price = data.UpgradePrice(0, ch.IntArray("pcl", 5)[0])!.Value;
        await g.SendAsync(new PacketWriter(0x188).U32(0).U32((uint)ch.TypeId).I32(ch.Id).Zeros(513 - 8));   // força +1
        Assert.Equal((ulong)(100_000 - price), (await g.ExpectAsync(0xC8)).U64());
        var st = await g.ExpectAsync(0x216);
        st.Skip(8);
        Assert.Equal(0xC9, st.U8());
        var up = await g.ExpectAsync(0x26F);
        Assert.Equal((0u, 0u), (up.U32(), up.U32()));
        await g.SendAsync(new PacketWriter(0x189).U32(0).U32((uint)ch.TypeId).I32(ch.Id).Zeros(513 - 8));   // força −1: sem devolver
        await g.ExpectAsync(0x216);
        Assert.Equal(0u, (await g.ExpectAsync(0x270)).U32());
        Assert.Equal(100_000 - price, h.Player.Pang);
    }

    [Fact]
    public async Task LoloComposesThreeCardsIntoOne()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var n = CardsOf(data, CardInfo.SubCharacter, 0);
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 10_000, (n[0].TypeId, 2), (n[1].TypeId, 1));
        await using var _ = g;
        await g.SendAsync(new PacketWriter(0x155).U64(2000).U32((uint)n[0].TypeId).U32((uint)n[0].TypeId).U32((uint)n[1].TypeId));
        Assert.NotEqual(0u, (await g.ExpectAsync(0x22A)).U32());                          // pang errado (3 × N = 3000)
        await g.SendAsync(new PacketWriter(0x155).U64(3000).U32((uint)n[0].TypeId).U32((uint)n[0].TypeId).U32((uint)n[1].TypeId));
        Assert.Equal(7000ul, (await g.ExpectAsync(0xC8)).U64());
        await g.ExpectAsync(0x216);
        Assert.InRange((await g.ExpectAsync(0x229)).U32(), 0u, 3u);
        var r = await g.ExpectAsync(0x22A);
        Assert.Equal(0u, r.U32());
        int won = (int)r.U32();
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        Assert.NotNull(h.Player.FindType(won));
        Assert.Null(h.Player.FindType(n[0].TypeId == won ? n[1].TypeId : n[0].TypeId) is { } left && left.TypeId != won ? left : null);
    }

    /// <summary>O mesmo card por tempo usado de novo soma o prazo (referência).</summary>
    [Fact]
    public async Task TimedCardExtendsWhenUsedAgain()
    {
        if (Jp983DataTests.Data is not { } data) return;
        var timed = data.Cards.Values.First(c => c.SubType == CardInfo.SubSpecial && c.Ability == CardInfo.AbilityPangRate && c.UseMinutes > 0);
        await using var env = new JpEnv();
        var (g, acc) = await InChannelWithAsync(env, 0, (timed.TypeId, 2));
        await using var _ = g;
        DateTime End(PacketReader r)
        {
            Assert.Equal(0u, r.U32());
            r.Skip(24 + 16);
            int y = r.U16(), mo = r.U16(); r.U16(); int d = r.U16(), hh = r.U16(), mi = r.U16(), ss = r.U16();
            return new DateTime(y, mo, d, hh, mi, ss);
        }
        await g.SendAsync(new PacketWriter(0xBD).U32((uint)timed.TypeId));
        var first = End(await g.ExpectAsync(0x160));
        await g.SendAsync(new PacketWriter(0xBD).U32((uint)timed.TypeId));
        var second = End(await g.ExpectAsync(0x160));
        Assert.InRange((second - first).TotalMinutes, timed.UseMinutes - 1, timed.UseMinutes + 1);
        var h = (GameHandler)env.Game.World.Find(acc.Id)!;
        Assert.Single(CardService.Active(h.Player, DateTime.UtcNow), it => it.TypeId == timed.TypeId);
    }
}

static class PacketReaderLet
{
    public static T Let<T>(this PacketReader r, Func<PacketReader, T> f) => f(r);
}
