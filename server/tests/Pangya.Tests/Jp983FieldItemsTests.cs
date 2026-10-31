using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Itens de campo no JP 983: posições do seed (migração 024) no 0x52 e o 0x1C de pegar (SPEC-jp983-itens-campo.md).</summary>
public partial class Jp983GameTests
{
    [Fact]
    public async Task WizCityItemsComeFromTheSeedAndPickupsAreChecked()
    {
        await using var env = new JpEnv();
        Assert.Equal(378, await env.S.FieldSpots.LoadAsync());                       // seed do GB: 355 do Wiz City + 23 do Shining Sand
        var (g, acc) = await InChannelAsync(env);
        await using var _ = g;
        // sala stroke, 1 buraco, Wiz City (0x13)
        await g.SendAsync(Jp983Client.Build(0x08, "FUN_00ae19c0", (byte)0, 60000u, 0u, (byte)4, (byte)0, (byte)1, FieldItems.WizCity,
            (byte)0, 0u, "wiz", "", 0u));
        await g.ExpectAsync(0x49);
        await g.SendAsync(Jp983Client.Build(0x0E, null, (uint)acc.Id));
        var init = await g.ExpectAsync(0x52);
        var room = Assert.Single(env.Game.World.Rooms.Rooms);
        var field = room.Field!;
        Assert.Equal(FieldItems.WizCity, init.U8());
        init.Skip(3 + 12 + 18 * 7);
        Assert.Equal(field.Seed, init.U32());
        int total = 0;
        for (int i = 0; i < 18; i++)
        {
            int n = init.U8();
            byte hole = room.HoleOrder[i];
            Assert.Equal(field.Spots[hole].Count(s => s != null), n);
            for (int k = 0; k < n; k++)
            {
                init.Skip(4);
                uint index = init.U32();
                init.Skip(8);
                Assert.Equal((hole, (byte)i), (init.U8(), init.U8()));
                init.Skip(2);
                var at = field.SpotOf(hole, index & 0xFF)!.Value;
                Assert.Equal((at.X, at.Y, at.Z), (init.F32(), init.F32(), init.F32()));
                Assert.InRange(init.U32(), 0u, 3u);
            }
            total += n;
        }
        Assert.Equal(0, init.Remaining);
        Assert.Equal(338, total);                                                     // Wiz City: quantidades do KR limitadas pelos pontos do seed

        // buraco 1, tacada e fim da tacada com 3 itens: a moeda 0, ela de novo e uma do buraco da vez 1 (não é o atual)
        await g.SendAsync(Jp983Client.Build(0x1A, null, (byte)1, new byte[0x19]));
        await g.SendAsync(Jp983Client.Build(0x11, null));
        await g.ExpectAsync(0x53);
        await g.SendAsync(new PacketWriter(0x12).U16(0).Bytes(new byte[JpTranslator.ShotSize + JpTranslator.ShotSyncSize]));
        await g.ExpectAsync(0x55);
        long before = (await env.Game.Context.Kr.Players.LoadAsync(acc.Id))!.Pang;
        byte first = room.HoleOrder[0];
        Assert.Equal(FieldItems.Coin, field.PerHole[first][0]);
        await g.SendAsync(new PacketWriter(0x1C).U8(1).U8(3)
            .U8(0).U32(JpTranslator.FieldIndex(0, 0)).U8(0).U32(JpTranslator.FieldIndex(0, 0)).U8(0).U32(JpTranslator.FieldIndex(1, 0)));
        var pang = await g.ExpectAsync((ushort)JpTranslator.ServerId(0xC6));
        Assert.InRange((long)pang.U64() - before, 1, 200);                            // uma moeda só
        lock (env.Game.World.Rooms.Sync)
        {
            Assert.Null(field.Take((uint)acc.Id, first, FieldItems.Coin, 0, 0, new Random(1)));          // já foi pega
            Assert.NotNull(field.Take((uint)acc.Id, room.HoleOrder[1], FieldItems.Coin, 0, 0, new Random(1)));   // a forjada não
        }
    }
}
