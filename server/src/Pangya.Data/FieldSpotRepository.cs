using Dapper;
using Pangya.Domain.Rooms;

namespace Pangya.Data;

/// <summary>Pontos dos itens de campo (migração 024).</summary>
public sealed class FieldSpotRepository(Db db) : IFieldSpotStore
{
    sealed record Row(int Course, int Hole, int Type, int Location, float X, float Y, float Z, int Hits, int Par);

    public async Task<List<FieldSpotRow>> LoadAsync()
    {
        await using var c = await db.OpenAsync();
        var list = new List<FieldSpotRow>();
        foreach (var r in await c.QueryAsync<Row>(
            "select course::int, hole::int, type::int, location::int, x, y, z, hits, par::int from field_spots order by hits desc"))
            list.Add(new FieldSpotRow((byte)r.Course, (byte)r.Hole, new FieldSpot(r.Type, (byte)r.Location, r.X, r.Y, r.Z), r.Hits, (byte)r.Par));
        return list;
    }

    public async Task SaveAsync(IReadOnlyList<FieldSpotRow> rows)
    {
        await using var c = await db.OpenAsync();
        await using var tx = await c.BeginTransactionAsync();
        foreach (var r in rows)
            await c.ExecuteAsync("""
                insert into field_spots(course, hole, type, location, x, y, z, hits, par)
                values (@course, @hole, @type, @location, @x, @y, @z, @hits, @par)
                on conflict (course, hole, type, x, y, z) do update set hits = excluded.hits, par = excluded.par, updated_at = now()
                """, new { course = (short)r.Course, hole = (short)r.Hole, type = (short)r.Spot.Type, location = (short)r.Spot.Location,
                    x = r.Spot.X, y = r.Spot.Y, z = r.Spot.Z, hits = r.Hits, par = (short)r.Par }, tx);
        await tx.CommitAsync();
    }
}
