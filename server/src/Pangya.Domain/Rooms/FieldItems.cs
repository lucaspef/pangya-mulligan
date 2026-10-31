namespace Pangya.Domain.Rooms;

/// <summary>
/// Itens de campo: moedas de pang e caixas (Spinning Cube). A tabela é da partida e a mesma para todos da sala.
/// KR 645 (SPEC-modes.md "Wiz City"): o 0x50 manda só a semente e tipo/índice por buraco, e o cliente gera as posições;
/// por isso o KR só recebe itens com a sala inteira no Wiz City, como antes. JP 983 (SPEC-jp983-itens-campo.md): o 0x52
/// leva a posição de cada item; elas vêm dos pontos guardados em <see cref="FieldSpots"/> (seed do GB + aprendidos).
/// Ao pegar, o 0x1C traz {tipo, índice}: vale só a cópia de quem tacou, conferida contra a tabela e nunca duas vezes.
/// </summary>
public sealed class FieldItems
{
    public const byte WizCity = 0x13;
    public const int Coin = 0, Box = 1;
    /// <summary>A caixa do campo dá 1 Spin Cube (aberto depois no My Room com uma Lucky Key; Shop.SpinCubeService).</summary>
    public const int BoxPrize = Shop.SpinCubeService.SpinCube;
    /// <summary>
    /// Fora do Wiz City a caixa nasce num ponto de chão aprendido, erguida: o cliente baixa 18 em y na caixa (0085df20).
    /// [suposição: o GB a põe no ar, no topo do voo calculado pela física]
    /// </summary>
    public const float BoxLift = 20f;

    readonly HashSet<(uint, byte, uint)> taken = [];
    readonly Dictionary<uint, List<int>> won = [];

    /// <summary>Itens que o jogador ganhou nas caixas (para a lista de itens da tela de resultado: 0xF8/0xCC).</summary>
    public IReadOnlyList<int> WonBy(uint guid) => won.TryGetValue(guid, out var l) ? l : [];
    public uint Seed { get; private set; }
    /// <summary>Por buraco físico (1..18): tipo de cada índice.</summary>
    public Dictionary<byte, int[]> PerHole { get; } = [];
    /// <summary>Por buraco físico: posição de cada índice (null = sem ponto conhecido; só o KR, que gera a posição, o mostra).</summary>
    public Dictionary<byte, FieldSpot?[]> Spots { get; } = [];
    /// <summary>Curso de cada buraco físico com itens (vai no registro do JP, como o GB).</summary>
    public Dictionary<byte, byte> CourseOf { get; } = [];
    /// <summary>A sala toda no Wiz City: o 0x50 do KR leva a lista (nos outros casos o KR não recebe itens).</summary>
    public bool ForKr { get; private init; }

    /// <summary>
    /// Tabela da partida; null se nenhum buraco tem item. order = buraco físico de cada vez (Room.HoleOrder), holeCourses =
    /// curso de cada vez (Special Shuffle / GP; vazio = <paramref name="course"/>). Wiz City: quantidades fixas por
    /// buraco e posições do seed. Outros cursos: só buracos com pontos aprendidos, quantidade pelo par.
    /// </summary>
    public static FieldItems? For(byte course, Random rng, FieldSpots? spots = null, byte[]? order = null, byte[]? holeCourses = null)
    {
        bool perHole = holeCourses is { Length: 18 };
        var f = new FieldItems { ForKr = course == WizCity && !perHole };
        for (int i = 0; i < 18; i++)
        {
            byte hole = order is { Length: 18 } ? order[i] : (byte)(i + 1), c = perHole ? holeCourses![i] : course;
            if (hole is < 1 or > 18 || f.PerHole.ContainsKey(hole)) continue;             // repetir buraco: um só conjunto
            if (c == WizCity) f.WizHole(hole, spots, rng);
            else if (spots != null) f.LearnedHole(c, hole, spots, rng);
        }
        if (f.PerHole.Count == 0) return null;                                            // sem itens: não gasta o rng
        f.Seed = (uint)rng.Next();
        return f;
    }

    void Set(byte course, byte hole, int[] types, FieldSpot?[] at) { PerHole[hole] = types; Spots[hole] = at; CourseOf[hole] = course; }

    void WizHole(byte hole, FieldSpots? spots, Random rng)
    {
        // quantidades por buraco [suposição: CubeCoinSystem.getAllCoinCubeInHoleWizCity do servidor S9]
        var (boxes, total) = hole switch { 3 or 12 => (5, 60), 14 => (2, 48), 18 => (3, 33), _ => (0, 20) };
        var coins = Shuffled(spots?.Of(WizCity, hole, Coin), rng);
        var cubes = Shuffled(spots?.Of(WizCity, hole, Box), rng);
        var types = new int[total];
        var at = new FieldSpot?[total];
        int nc = 0, nb = 0;
        for (int i = 0; i < total; i++)
        {
            types[i] = i < boxes ? Box : Coin;
            if (i < boxes) { if (nb < cubes.Length) at[i] = cubes[nb++]; }
            else if (nc < coins.Length) at[i] = coins[nc++];
        }
        Set(WizCity, hole, types, at);
    }

    void LearnedHole(byte course, byte hole, FieldSpots spots, Random rng)
    {
        var coins = Shuffled(spots.Of(course, hole, Coin), rng);
        if (coins.Length == 0) return;                                                    // nada aprendido: sem itens
        // quantidade pelo par [suposição: o GB dá 1/5/8 por par 3/4/5]; caixa rara: 1 em 3 buracos
        int total = Math.Min(coins.Length, spots.ParOf(course, hole) switch { 3 => 2, 5 => 8, _ => 5 });
        int boxes = total >= 2 && rng.Next(3) == 0 ? 1 : 0;
        var types = new int[total];
        var at = new FieldSpot?[total];
        for (int i = 0; i < total; i++)
        {
            var s = coins[i];
            types[i] = i < boxes ? Box : Coin;
            at[i] = i < boxes ? new FieldSpot(Box, FieldSpot.Air, s.X, s.Y + BoxLift, s.Z) : s;
        }
        Set(course, hole, types, at);
    }

    static FieldSpot[] Shuffled(List<FieldSpot>? l, Random rng)
    {
        var a = l?.ToArray() ?? [];
        rng.Shuffle(a);
        return a;
    }

    /// <summary>Posição do item (null = não existe ou sem ponto conhecido).</summary>
    public FieldSpot? SpotOf(byte hole, uint index) =>
        Spots.TryGetValue(hole, out var at) && index < at.Length ? at[index] : null;

    /// <summary>Um item reportado; devolve o prêmio (pang &gt; 0, ou o Spin Cube da caixa) ou null se inválido/repetido.</summary>
    public (int Pang, int ItemTypeId)? Take(uint guid, byte hole, int type, uint index, int texture, Random rng)
    {
        if (!PerHole.TryGetValue(hole, out var types) || index >= types.Length || types[index] != type) return null;
        if (!taken.Add((guid, hole, index))) return null;
        if (type == Coin) return (rng.Next(1, (texture == 1 ? 50 : 200) + 1), 0);
        if (!won.TryGetValue(guid, out var list)) won[guid] = list = [];
        list.Add(BoxPrize);
        return (0, BoxPrize);
    }
}
