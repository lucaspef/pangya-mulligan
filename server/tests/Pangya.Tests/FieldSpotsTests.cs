using Pangya.Core.Net;
using Pangya.Domain.Rooms;
using Pangya.Protocol.JP983.Game;

namespace Pangya.Tests;

/// <summary>Pontos dos itens de campo (aprender, limitar, gravar) e a tabela da partida com posições (JP 983).</summary>
public class FieldSpotsTests
{
    const byte Course = 5, Hole = 2;
    static readonly HoleInfo Par4 = new(4, 0, 0, 0, 0);                         // bandeira na origem
    static ShotResult At(float x, float z, byte state = FieldSpots.StatePlayable, byte bunker = 0) =>
        new(1, x, 10, z, state, 0, 0, Bunker: bunker);

    [Fact]
    public void LearnsOnlyGoodRestingBallsFarFromThePin()
    {
        var s = new FieldSpots();
        Assert.True(s.Learn(Course, Hole, Par4, 2, At(500, 0)));
        Assert.False(s.Learn(Course, Hole, Par4, 2, At(600, 0, state: ShotResult.StateWaterOrOut)));    // água/OB
        Assert.False(s.Learn(Course, Hole, Par4, 2, At(600, 0, bunker: 1)));                             // bunker
        Assert.False(s.Learn(Course, Hole, Par4, 5, At(600, 0)));                                       // passou do par
        Assert.False(s.Learn(Course, Hole, Par4, 2, At(100, 100)));                                     // < 70 y da bandeira
        Assert.False(s.Learn(Course, Hole, new HoleInfo(0, 0, 0, 0, 0), 1, At(600, 0)));               // par desconhecido
        Assert.False(s.Learn(FieldItems.WizCity, Hole, Par4, 2, At(600, 0)));                           // Wiz City: só o seed
        Assert.False(s.Learn(Course, Hole, Par4, 2, At(float.NaN, 0)));
        Assert.Equal(1, s.Count);
        Assert.Equal(4, s.ParOf(Course, Hole));
        var spot = Assert.Single(s.Of(Course, Hole, FieldItems.Coin));
        Assert.Equal((FieldSpot.Ground, 500f, 10f, 0f), (spot.Location, spot.X, spot.Y, spot.Z));
    }

    [Fact]
    public void NearbySpotsMergeAndEachHoleIsBounded()
    {
        var s = new FieldSpots();
        s.Learn(Course, Hole, Par4, 1, At(500, 0));
        s.Learn(Course, Hole, Par4, 1, At(505, 5));                                   // dentro de 5 y: o mesmo ponto
        Assert.Single(s.Of(Course, Hole, FieldItems.Coin));
        for (int i = 0; i < FieldSpots.MaxPerHole + 20; i++) s.Learn(Course, Hole, Par4, 1, At(300 + i * 20, 400));
        Assert.Equal(FieldSpots.MaxPerHole, s.Of(Course, Hole, FieldItems.Coin).Count);
    }

    sealed class MemoryStore : IFieldSpotStore
    {
        public readonly List<FieldSpotRow> Saved = [];
        public List<FieldSpotRow> Rows = [];
        public Task<List<FieldSpotRow>> LoadAsync() => Task.FromResult(Rows);
        public Task SaveAsync(IReadOnlyList<FieldSpotRow> rows) { Saved.AddRange(rows); return Task.CompletedTask; }
    }

    [Fact]
    public async Task LoadsOnceAndFlushesOnlyWhatChanged()
    {
        var store = new MemoryStore { Rows = [new(Course, Hole, new FieldSpot(FieldItems.Coin, FieldSpot.Ground, 500, 0, 0), 3, 4)] };
        var s = new FieldSpots(store);
        Assert.Equal(1, await s.LoadAsync());
        Assert.Equal(1, await s.LoadAsync());                                         // a mesma carga (KR e JP)
        Assert.Equal(1, s.Count);
        Assert.Equal(0, await s.FlushAsync());                                        // o que veio do banco não volta
        s.Learn(Course, Hole, Par4, 1, At(502, 0));                                   // soma acerto no ponto carregado
        s.Learn(Course, 3, Par4, 1, At(900, 0));                                      // ponto novo
        Assert.Equal(2, await s.FlushAsync());
        Assert.Contains(store.Saved, r => r.Hole == Hole && r.Hits == 4);
        Assert.Contains(store.Saved, r => r.Hole == 3 && r.Hits == 1 && r.Par == 4);
        Assert.Equal(0, await s.FlushAsync());
    }

    [Fact]
    public void LearnedCourseGetsItemsOnlyWhereSpotsExist()
    {
        var s = new FieldSpots();
        Assert.Null(FieldItems.For(Course, new Random(1), s));                       // nada aprendido: sem itens, sem erro
        for (int i = 0; i < 12; i++) s.Learn(Course, Hole, Par4, 1, At(300 + i * 30, 500));
        var f = FieldItems.For(Course, new Random(1), s)!;
        Assert.False(f.ForKr);
        Assert.Equal([Hole], f.PerHole.Keys);
        Assert.Equal(5, f.PerHole[Hole].Length);                                      // par 4: 5 itens
        Assert.True(f.PerHole[Hole].Count(t => t == FieldItems.Box) <= 1);
        var learned = s.Of(Course, Hole, FieldItems.Coin);
        for (int i = 0; i < 5; i++)
        {
            var at = f.SpotOf(Hole, (uint)i)!.Value;
            bool box = f.PerHole[Hole][i] == FieldItems.Box;
            Assert.Contains(learned, l => l.X == at.X && l.Z == at.Z && l.Y + (box ? FieldItems.BoxLift : 0) == at.Y);
        }
        Assert.Equal(Course, f.CourseOf[Hole]);
    }

    [Fact]
    public void WizCityUsesTheSeedSpotsAndKeepsTheKrCounts()
    {
        var s = new FieldSpots();
        for (int i = 0; i < 14; i++) s.Add(FieldItems.WizCity, 1, new FieldSpot(FieldItems.Coin, FieldSpot.EdgeGreen, i * 100, 0, 0));
        for (int i = 0; i < 5; i++) s.Add(FieldItems.WizCity, 3, new FieldSpot(FieldItems.Box, FieldSpot.Carpet, i * 100, 5, 0));
        var f = FieldItems.For(FieldItems.WizCity, new Random(1), s)!;
        Assert.True(f.ForKr);
        Assert.Equal(20, f.PerHole[1].Length);                                        // o KR segue com 20
        Assert.Equal(14, f.Spots[1].Count(p => p != null));                           // o JP vê os 14 com posição
        Assert.Equal(5, f.Spots[3].Count(p => p is { Type: FieldItems.Box }));        // caixas nos pontos de caixa
        for (int i = 0; i < 5; i++) Assert.Equal(FieldItems.Box, f.PerHole[3][i]);
        Assert.Equal(18, f.PerHole.Count);
    }

    [Fact]
    public void ShuffleUsesTheCourseOfEachHole()
    {
        var s = new FieldSpots();
        s.Learn(Course, 4, Par4, 1, At(500, 0));
        var order = new byte[18];
        var courses = new byte[18];
        for (int i = 0; i < 18; i++) (order[i], courses[i]) = ((byte)(i + 1), (byte)(i == 3 ? Course : 1));
        var f = FieldItems.For(RoomManager.ChaosCourse, new Random(1), s, order, courses)!;
        Assert.Equal([(byte)4], f.PerHole.Keys);
        Assert.False(f.ForKr);
    }

    static Room RoomWith(FieldItems f, byte[] order)
    {
        var room = new RoomManager().Create(new RoomSettings { Holes = 18 }, 1);
        room.HoleOrder = order;
        room.Field = f;
        return room;
    }

    /// <summary>Fim do 0x52: u32 semente + 18 × {u8 n, n × 0x24}; cada registro aponta a vez (0-based) em +0x11.</summary>
    [Fact]
    public void JpTailCarriesPositionsPerPlayOrder()
    {
        var s = new FieldSpots();
        for (int i = 0; i < 3; i++) s.Learn(Course, 7, new HoleInfo(3, 0, 0, 0, 0), 1, At(400 + i * 50, 300));
        var f = FieldItems.For(Course, new Random(3), s)!;
        var order = new byte[18];
        for (int i = 0; i < 18; i++) order[i] = (byte)((i + 4) % 18 + 1);           // buraco 7 na vez 2
        using var w = JpTranslator.FieldItemsTail(new PacketWriter(0x52), RoomWith(f, order), 99);
        var r = new PacketReader();
        r.Reset(w.Body.ToArray(), 0, w.Body.Length);
        Assert.Equal(f.Seed, r.U32());
        for (int i = 0; i < 18; i++)
        {
            int n = r.U8();
            if (order[i] != 7) { Assert.Equal(0, n); continue; }
            Assert.Equal(2, n);                                                       // par 3: 2 itens
            for (int k = 0; k < n; k++)
            {
                int type = r.I32();
                Assert.Equal(f.PerHole[7][k], type);
                Assert.Equal(JpTranslator.FieldIndex(i, k), r.U32());
                Assert.Equal((0u, (uint)Course, (byte)7, (byte)i, (ushort)0), (r.U32(), r.U32(), r.U8(), r.U8(), r.U16()));
                var at = f.SpotOf(7, (uint)k)!.Value;
                Assert.Equal((at.X, at.Y, at.Z, (uint)at.Location), (r.F32(), r.F32(), r.F32(), r.U32()));
            }
        }
        Assert.Equal(0, r.Remaining);
    }

    [Fact]
    public void JpTailRepeatHoleSendsTheItemsOnce()
    {
        var s = new FieldSpots();
        s.Learn(Course, 7, Par4, 1, At(500, 0));
        var f = FieldItems.For(Course, new Random(1), s)!;
        var order = Enumerable.Repeat((byte)7, 18).ToArray();
        using var w = JpTranslator.FieldItemsTail(new PacketWriter(0x52), RoomWith(f, order), 0);
        var b = w.Body.ToArray();
        Assert.Equal(1, b[2 + 4]);                                                    // vez 0: o item
        Assert.Equal(2 + 4 + 1 + 0x24 + 17, b.Length);                                // as outras 17: zero
    }
}
