namespace Pangya.Domain.Rooms;

/// <summary>
/// Ponto onde um item de campo pode nascer. Location = o tipo_location do GB (0 borda do green, 1 carpete, 2 ar, 3 chão),
/// que vai como textura no registro do JP (0 = moeda/caixa com ímã, != 0 gira).
/// </summary>
public readonly record struct FieldSpot(int Type, byte Location, float X, float Y, float Z)
{
    public const byte EdgeGreen = 0, Carpet = 1, Air = 2, Ground = 3;
}

/// <summary>Linha gravada (migração 024): um ponto de um buraco, com quantas vezes uma bola parou ali e o par do buraco.</summary>
public sealed record FieldSpotRow(byte Course, byte Hole, FieldSpot Spot, int Hits, byte Par);

/// <summary>Banco dos pontos de itens de campo (migração 024, tabela field_spots).</summary>
public interface IFieldSpotStore
{
    Task<List<FieldSpotRow>> LoadAsync();
    Task SaveAsync(IReadOnlyList<FieldSpotRow> rows);
}

/// <summary>
/// Pontos de moedas/caixas por (curso, buraco). Como o GB (CoinCubeLocationUpdateSystem): aprende onde as bolas param
/// (tacada boa: área jogável, fora do bunker, até o par, a mais de 70 y da bandeira); perto de um ponto já conhecido só
/// soma acertos. O Wiz City vem do banco (seed do GB) e não aprende. Carregado ao subir; gravado só o que mudou.
/// Thread-safe: os dois game servers (KR e JP) do processo dividem a mesma instância.
/// </summary>
public sealed class FieldSpots(IFieldSpotStore? store = null)
{
    /// <summary>Escala do PangYa (GB SCALE_PANGYA): 1 jarda = 3,2 unidades do mundo.</summary>
    public const float Yard = 3.2f;
    public const float MergeRadius = 5 * Yard, MinPinDistance = 70 * Yard;
    public const int MaxPerHole = 100;
    public const byte StatePlayable = 2;

    sealed class Entry(FieldSpot spot, int hits) { public FieldSpot Spot = spot; public int Hits = hits; public bool Dirty; }
    sealed class HoleSpots { public byte Par; public readonly List<Entry> List = []; }

    readonly object sync = new();
    readonly Dictionary<(byte, byte), HoleSpots> holes = [];
    Task<int>? loading;

    /// <summary>Quantos pontos estão guardados (todos os cursos).</summary>
    public int Count { get { lock (sync) { int n = 0; foreach (var h in holes.Values) n += h.List.Count; return n; } } }

    /// <summary>Par do buraco, se algum ponto aprendido o trouxe (0 = desconhecido).</summary>
    public byte ParOf(byte course, byte hole) { lock (sync) return holes.TryGetValue((course, hole), out var h) ? h.Par : (byte)0; }

    /// <summary>Cópia dos pontos de um tipo no buraco (para sortear fora do lock).</summary>
    public List<FieldSpot> Of(byte course, byte hole, int type)
    {
        var r = new List<FieldSpot>();
        lock (sync)
            if (holes.TryGetValue((course, hole), out var h))
                foreach (var e in h.List) if (e.Spot.Type == type) r.Add(e.Spot);
        return r;
    }

    /// <summary>Acrescenta (ou soma acertos a um ponto a menos de 5 y) um ponto. Devolve true se é novo. Do banco: sem juntar.</summary>
    public bool Add(byte course, byte hole, FieldSpot s, byte par = 0, int hits = 1, bool dirty = true, bool merge = true)
    {
        lock (sync)
        {
            if (!holes.TryGetValue((course, hole), out var h)) holes[(course, hole)] = h = new HoleSpots();
            if (par != 0) h.Par = par;                                         // vai junto com a próxima linha gravada
            if (merge) foreach (var e in h.List)
            {
                if (e.Spot.Type != s.Type) continue;
                float dx = e.Spot.X - s.X, dy = e.Spot.Y - s.Y, dz = e.Spot.Z - s.Z;
                if (dx * dx + dy * dy + dz * dz > MergeRadius * MergeRadius) continue;
                e.Hits += hits;
                e.Dirty |= dirty;
                return false;
            }
            if (h.List.Count >= MaxPerHole) return false;                     // cheio: o conjunto fica limitado
            h.List.Add(new Entry(s, hits) { Dirty = dirty });
            return true;
        }
    }

    /// <summary>
    /// Bola parada depois de uma tacada (0x1B) no buraco físico <paramref name="hole"/>: vira ponto de moeda se a tacada
    /// foi boa (regras do GB calcule_shot_to_coin). strokes = tacadas no buraco contando esta. Wiz City não aprende.
    /// </summary>
    public bool Learn(byte course, byte hole, HoleInfo info, int strokes, in ShotResult r)
    {
        if (course == FieldItems.WizCity || course >= 0x40 || hole is < 1 or > 18) return false;
        if (r.State != StatePlayable || r.Bunker != 0 || info.Par is < 3 or > 5 || strokes < 1 || strokes > info.Par) return false;
        if (!float.IsFinite(r.X) || !float.IsFinite(r.Y) || !float.IsFinite(r.Z) || Math.Abs(r.X) > 1e5f || Math.Abs(r.Z) > 1e5f) return false;
        float dx = r.X - info.PinX, dz = r.Z - info.PinZ;
        if (dx * dx + dz * dz <= MinPinDistance * MinPinDistance) return false;   // perto demais da bandeira
        Add(course, hole, new FieldSpot(FieldItems.Coin, FieldSpot.Ground, r.X, r.Y, r.Z), info.Par);
        return true;
    }

    /// <summary>Carrega do banco uma vez só: os dois servidores chamam e esperam a mesma carga. Devolve quantos pontos vieram.</summary>
    public Task<int> LoadAsync() { lock (sync) return loading ??= LoadCoreAsync(); }

    async Task<int> LoadCoreAsync()
    {
        if (store == null) return 0;
        var rows = await store.LoadAsync();
        foreach (var r in rows) Add(r.Course, r.Hole, r.Spot, r.Par, r.Hits, dirty: false, merge: false);
        return rows.Count;
    }

    /// <summary>Grava o que mudou desde a última vez (falhou = tenta de novo na próxima). Devolve quantas linhas.</summary>
    public async Task<int> FlushAsync()
    {
        if (store == null) return 0;
        var rows = new List<FieldSpotRow>();
        var taken = new List<Entry>();
        lock (sync)
            foreach (var ((course, hole), h) in holes)
                foreach (var e in h.List)
                    if (e.Dirty) { e.Dirty = false; taken.Add(e); rows.Add(new FieldSpotRow(course, hole, e.Spot, e.Hits, h.Par)); }
        if (rows.Count == 0) return 0;
        try { await store.SaveAsync(rows); }
        catch
        {
            lock (sync) foreach (var e in taken) e.Dirty = true;
            throw;
        }
        return rows.Count;
    }
}
