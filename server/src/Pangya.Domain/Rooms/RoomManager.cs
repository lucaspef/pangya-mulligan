using Pangya.Domain.Game;
using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>Códigos do 0x47 que o cliente conhece (msg 0x3B): 2 cheia, 3 não existe, 4 senha, 8 jogando, 13 precisa de guilda.</summary>
/// <summary>Códigos do 0x47 (0x49 no JP). Restricted (5): o cliente não tem código para taxa de abandono; é o "não pode criar ou entrar".</summary>
public enum JoinResult : byte { Ok = 0, Full = 2, NotFound = 3, WrongPassword = 4, Restricted = 5, Playing = 8, GuildRequired = 13 }

/// <summary>
/// Salas de um game server e quem está na tela de lista de salas. Todas as operações de sala e partida
/// acontecem sob <see cref="Sync"/> (um lock por game server: operações curtas, os envios só enfileiram).
/// </summary>
public sealed class RoomManager
{
    readonly Dictionary<int, Room> rooms = [];
    int nextIndex = 1;

    public object Sync { get; } = new();
    /// <summary>Mapas permitidos (o "aleatório" sorteia entre eles). Vazio = qualquer 0..10 (como o emulador fazia).</summary>
    public IReadOnlyList<byte> Courses { get; set; } = [];
    public const byte RandomCourse = 0x7F;

    /// <summary>Mapa escolhido pelo cliente: tem de ser permitido; senão vira aleatório.</summary>
    public byte ValidCourse(byte course)
    {
        if (course >= RandomCourse) return RandomCourse;
        if (Courses.Count == 0) return course;
        foreach (var c in Courses)
            if (c == course) return course;
        return RandomCourse;
    }
    public Dictionary<int, Room>.ValueCollection Rooms => rooms.Values;
    /// <summary>Sessões na tela de lista de salas (recebem as atualizações da lista).</summary>
    public HashSet<IGameSession> Lobby { get; } = [];

    public Room? Get(int index) => rooms.GetValueOrDefault(index);

    public Room Create(RoomSettings settings, long ownerId)
    {
        settings.Normalize();
        settings.Course = settings.Zodiac != ZodiacKind.None ? RoomSettings.ZodiacCourse : ValidCourse(settings.Course);
        while (rooms.ContainsKey(nextIndex)) nextIndex = nextIndex % 0xFFFE + 1;
        var room = new Room { Index = nextIndex, OwnerId = ownerId };
        nextIndex = nextIndex % 0xFFFE + 1;
        CopySettings(settings, room.Settings);
        rooms[room.Index] = room;
        return room;
    }

    static void CopySettings(RoomSettings from, RoomSettings to)
    {
        to.Title = from.Title; to.Password = from.Password; to.Mode = from.Mode; to.Course = from.Course;
        to.Holes = from.Holes; to.HoleType = from.HoleType; to.ShotTimeMs = from.ShotTimeMs;
        to.GameTimeMs = from.GameTimeMs; to.MaxPlayers = from.MaxPlayers; to.Sleep = from.Sleep;
        to.Practice = from.Practice; to.RepeatHole = from.RepeatHole; to.GrandPrixId = from.GrandPrixId; to.Zodiac = from.Zodiac;
        to.ByGm = from.ByGm; to.Artifact = from.Artifact; to.Natural = from.Natural;
    }

    /// <summary>
    /// Pode entrar? No GuildMatch: só membro (cargo 1..3) de uma das duas guildas da sala, ou de uma nova se houver lado
    /// vazio (SPEC-guildmatch.md §1.3).
    /// </summary>
    public static JoinResult CanJoin(Room? room, string password, Player? player = null)
    {
        if (room == null || room.Settings.Hidden) return JoinResult.NotFound;     // prática: só o dono; GP: pelo pedido do GP
        if (room.Settings.Password.Length > 0 && password != room.Settings.Password) return JoinResult.WrongPassword;
        if (room.State != RoomState.Waiting) return JoinResult.Playing;
        if (room.Players.Count >= room.Settings.MaxPlayers) return JoinResult.Full;
        if (player != null && QuitRate.Barred(player, room.Settings.Mode)) return JoinResult.Restricted;
        if (room.Settings.Mode == GameMode.GuildMatch && player != null)
        {
            if (player.Guild is not { } g || !Guilds.GuildClass.IsMember(g.Class)) return JoinResult.GuildRequired;
            if (room.SideOf(g.Id) < 0 && room.GuildSides[0] != null && room.GuildSides[1] != null) return JoinResult.Full;
        }
        return JoinResult.Ok;
    }

    /// <summary>Adiciona o jogador (time alternado; o primeiro humano é o dono). GuildMatch: time = lado da guilda.</summary>
    public static RoomPlayer Join(Room room, RoomPlayer p)
    {
        p.Master = room.HumanCount == 0 && !p.IsBot;
        p.Team = (byte)(room.Players.Count % 2);
        if (room.Settings.Mode == GameMode.GuildMatch && p.Player.Guild is { } g)
        {
            int side = room.SideOf(g.Id);
            if (side < 0) side = room.GuildSides[0] == null ? 0 : 1;
            room.GuildSides[side] ??= new GuildSide(g.Id, g.Name, g.Mark);
            p.Team = (byte)side;
        }
        room.Add(p);
        return p;
    }

    /// <summary>Tira o jogador. Devolve o novo dono (se o dono saiu e sobrou humano) e se a sala fechou (sem humanos).</summary>
    public (RoomPlayer? NewMaster, bool Closed) Leave(Room room, RoomPlayer p)
    {
        room.Remove(p);
        room.Game?.PlayerLeft(p);
        if (room.Settings.Mode == GameMode.GuildMatch && room.State == RoomState.Waiting)   // lado que esvaziou fica livre
            for (int side = 0; side < 2; side++)
            {
                bool any = false;
                foreach (var m in room.Players) any |= m.Team == side;
                if (!any) room.GuildSides[side] = null;
            }
        if (room.HumanCount == 0)
        {
            room.Game?.Cancel();
            rooms.Remove(room.Index);
            return (null, true);
        }
        if (!p.Master) return (null, false);
        foreach (var m in room.Players)
        {
            if (m.IsBot) continue;
            m.Master = true;
            room.OwnerId = m.Player.AccountId;
            return (m, false);
        }
        return (null, false);
    }

    /// <summary>Sorteia mapa (se aleatório), ordem e sementes dos buracos e marca a sala como jogando.</summary>
    public static void PrepareStart(Room room, Random rng, IReadOnlyList<byte>? courses = null, FieldSpots? spots = null)
    {
        var s = room.Settings;
        room.CoursePlayed = s.Course < RandomCourse ? s.Course                     // >= 0x7F = aleatório
            : courses is { Count: > 0 } ? courses[rng.Next(courses.Count)] : (byte)rng.Next(11);
        int start = s.HoleType switch { 1 => 9, 2 => rng.Next(18), _ => 0 };
        var order = new byte[18];
        var seeds = new uint[18];
        for (int i = 0; i < 18; i++)
        {
            order[i] = (byte)((start + i) % 18 + 1);
            seeds[i] = (uint)rng.NextInt64(uint.MaxValue);
        }
        if (s.HoleType == 3) rng.Shuffle(order);
        if (s.RepeatHole > 0)                                                   // repetir buraco: o mesmo buraco e pin em todas
            for (int i = 0; i < 18; i++) (order[i], seeds[i]) = (s.RepeatHole, seeds[0]);
        room.HoleCourses = [];
        if ((int)s.Mode == ShuffleCourseMode) ShuffleCourses(room, order, rng, courses);
        else if (room.GrandPrix?.Def.SpecialHoles is { Length: > 0 } sh)            // GP com buracos especiais: mapa e buraco de cada um
        {
            room.HoleCourses = new byte[18];
            for (int i = 0; i < 18; i++)
                (order[i], room.HoleCourses[i]) = i < sh.Length ? (sh[i].Hole, sh[i].Course) : ((byte)(i + 1), room.CoursePlayed);
        }
        room.HoleOrder = order;
        room.HoleSeeds = seeds;
        room.GameSeed = (uint)rng.NextInt64(uint.MaxValue);
        room.Field = s.GrandPrixId != 0 || s.Zodiac != ZodiacKind.None ? null                // GP/GZ: sem cubos nem moedas
            : FieldItems.For(room.CoursePlayed, rng, spots, order, room.HoleCourses);
        room.State = RoomState.Playing;
        foreach (var p in room.Players) p.Ready = p.Master || p.IsBot;
    }

    /// <summary>Modo 14 do cliente: Special Shuffle Course (botão "chaos").</summary>
    public const int ShuffleCourseMode = 14;
    /// <summary>Chronicle 1 Chaos: o mapa do último buraco do Special Shuffle Course.</summary>
    public const byte ChaosCourse = 17;

    /// <summary>
    /// Special Shuffle Course (como o GB, CourseManager.init_seq/init_hole): os buracos 1..17 são de mapas sorteados
    /// (os permitidos, menos o Chaos) com números embaralhados sem repetir; o 18 é o buraco 1 ou 2 do Chronicle 1 Chaos.
    /// A sala fica com o mapa do Chaos.
    /// </summary>
    static void ShuffleCourses(Room room, byte[] order, Random rng, IReadOnlyList<byte>? courses)
    {
        var pool = new List<byte>();
        if (courses != null) foreach (var c in courses) if (c != ChaosCourse && c < RandomCourse) pool.Add(c);
        if (pool.Count == 0) for (byte c = 0; c <= 10; c++) pool.Add(c);
        byte last = (byte)(rng.Next(2) + 1);
        int k = 0;
        for (byte h = 1; h <= 18; h++) if (h != last) order[k++] = h;
        rng.Shuffle(order.AsSpan(0, 17));
        order[17] = last;
        var holeCourses = new byte[18];
        for (int i = 0; i < 17; i++) holeCourses[i] = pool[rng.Next(pool.Count)];
        holeCourses[17] = ChaosCourse;
        room.HoleCourses = holeCourses;
        room.CoursePlayed = ChaosCourse;
    }

    /// <summary>Fim de partida: a sala volta a esperar.</summary>
    public static void FinishGame(Room room)
    {
        room.State = RoomState.Waiting;
        room.Game = null;
        foreach (var p in room.Players) p.Ready = p.IsBot;
    }
}
