using Pangya.Domain.Players;

namespace Pangya.Domain.Rooms;

/// <summary>Tipos do Grand Zodiac (cliente JP 983): 11 intermediário e 13 avançado (salas de evento), 14 Chip-in Practice.</summary>
public enum ZodiacKind : byte { None = 0, Intermediate = 11, Advanced = 13, ChipIn = 14 }

/// <summary>Jogador no Grand Zodiac: tee do buraco, tacadas, pontos, HIOs e o que a última tacada trouxe.</summary>
public sealed class ZodiacPlayer(MassPlayer p)
{
    public MassPlayer Mass { get; } = p;
    public uint Guid => Mass.Guid;
    public float TeeX { get; set; }
    public float TeeZ { get; set; }
    public int HoleShots { get; set; }
    public int TotalShots { get; set; }
    public int Total { get; set; }
    public int HoleInOnes { get; set; }
    public uint Pang { get; set; }
    public uint Bonus { get; set; }
    public byte PangyaFlag { get; set; }
    public uint SpecialShot { get; set; }
    public bool Holed { get; set; }
    public List<byte> HoleBonuses { get; } = [];
    public bool Ready { get; set; }
    public bool Finished { get; set; }
    /// <summary>Terminou com o tempo cheio (o resto é "sem completar").</summary>
    public bool Complete { get; set; }
    /// <summary>Evento: acertou na janela do golden beam; jackpot ganho; itens que caíram (typeid -> qtd).</summary>
    public bool InBeam { get; set; }
    public long Jackpot { get; set; }
    public Dictionary<int, int> Drops { get; } = [];
}

/// <summary>Linha do placar do Grand Zodiac (troféu 0 = nenhum).</summary>
public readonly record struct ZodiacResult(ZodiacPlayer Player, int Position, int Exp, uint GzPoints, int Trophy = 0);

/// <summary>Pacotes do Grand Zodiac (os ids são do cliente que tem o modo; a camada de protocolo implementa).</summary>
public interface IZodiacOutput
{
    /// <summary>Carregou: vento, os 10 valores da partida (0x1EC) e o fim do carregamento (0x201).</summary>
    void Loaded(ZodiacPlayer p, byte wind, byte direction, double[] seeds);
    /// <summary>Todos prontos: relógio (0x8D 0), começa o buraco (0x53), posição (0x1EE), próximo buraco (0x1F4 1).</summary>
    void FirstHole(ZodiacPlayer p);
    /// <summary>Bônus do acerto (0x1F5), só com mais de um.</summary>
    void ScoreTypes(ZodiacPlayer p, List<byte> types);
    /// <summary>Fim da tacada (0xCC sem itens).</summary>
    void EndShot(ZodiacPlayer p);
    /// <summary>Acertou: placar do buraco (0x1EF) e o próximo (0x1F4, vento 0x5B, segundos 0x200, posição 0x1EE).</summary>
    void Holed(ZodiacPlayer p, int before, int points, byte wind, byte direction, uint seconds);
    /// <summary>Errou: volta ao tee (0x1EE).</summary>
    void BackToTee(ZodiacPlayer p);
    /// <summary>Acabou o tempo (0x8D com o tempo total) a quem ainda joga.</summary>
    void TimeOver(ZodiacPlayer p, uint totalMs);
    /// <summary>O jogador terminou (0x1F2).</summary>
    void Ended(ZodiacPlayer p);
    /// <summary>Fim de todos: placar (0x8C, 0x1F3) e a recompensa de cada um.</summary>
    void GameOver(List<ZodiacResult> results);
    /// <summary>Evento: abriu a janela do golden beam (0x1F0, 1 min antes do fim).</summary>
    void BeamOpen() { }
    /// <summary>Evento: fechou a janela (0x1F1) e o jackpot (um ganhador: 0x40 tipo 13 a todos; dividido: tipo 14 a cada um).</summary>
    void BeamClose(List<ZodiacPlayer> winners, bool split) { }
    /// <summary>Evento: caiu um item para o jogador (0x40 tipo 15 a todos).</summary>
    void ItemDropped(ZodiacPlayer p, int typeId, int quantity) { }
}

/// <summary>
/// Grand Zodiac (docs/server/protocolo/SPEC-jp983-grand-zodiac.md): partida por tempo no mapa 0x40. Cada tacada sai do tee; errou =
/// volta ao tee; acertou (hole-in-one / chip-in) = 1 ponto + bônus (1ª tentativa, tacada especial, sem comandos) e o
/// próximo buraco com vento novo. Ganha quem somar mais pontos. Fase 1: o Chip-in Practice (14, um jogador).
/// </summary>
public sealed class GrandZodiacGame : MassGame
{
    /// <summary>Bônus do 0x1F5: 1 o acerto, 2 primeira tacada, 3 tacada especial, 4 sem comandos, 5 sem acertar o Pangya (+3).</summary>
    public const byte BonusHole = 1, BonusFirst = 2, BonusSpecial = 3, BonusNoCommand = 4, BonusMissPangya = 5;
    /// <summary>Bit do "errou o Pangya" no u8 acerto do pangya do 0x12.</summary>
    const byte MissedPangya = 2;
    /// <summary>Bits da tacada especial no u32 shot do resultado: tomahawk, spike, cobra.</summary>
    const uint SpecialShots = 0b1110;
    /// <summary>Bit do "acertou o buraco" no u32 display do resultado.</summary>
    const uint DisplayHoled = 0x100;

    readonly IZodiacOutput z;
    readonly Dictionary<uint, ZodiacPlayer> players = [];
    readonly double[] seeds = new double[10];
    readonly long startedAt = System.Diagnostics.Stopwatch.GetTimestamp();
    bool running;
    (float X, float Z) tee;

    public ZodiacKind Kind => Room.Settings.Zodiac;
    public uint TotalMs => Room.Settings.GameTimeMs;

    /// <summary>Evento: tempo padrão (referência: 7 min), janela do beam (de 60 s a 30 s antes do fim), item que cai (10 %).</summary>
    public const uint EventTimeMs = 420_000;
    public const int DropTypeId = 0x1800002C, DropQuantity = 5, DropChance = 10, JackpotPerPlayer = 500;
    /// <summary>Troféus do evento (Match.iff): 1º, 2º e 3º.</summary>
    public static readonly int[] Trophies = [0x2D0A6200, 0x2D0A6300, 0x2D0A6400];

    bool beamOpen;
    public bool IsEvent => Kind is ZodiacKind.Intermediate or ZodiacKind.Advanced;

    public GrandZodiacGame(Room room, IMassOutput output, IZodiacOutput zodiac, object sync) : base(room, output, sync, TimeSpan.Zero)
    {
        z = zodiac;
        // bot não joga o Zodiac (não manda o 0x137): fora da lista, senão o Ready esperava por ele e o 1º buraco não começava
        foreach (var p in Players) if (!p.IsBot) players[p.Guid] = new ZodiacPlayer(p);
        double d = Rng.Next(1000) / 100.0 + 1;
        for (int i = 0; i < seeds.Length; i++) seeds[i] = d + i * (Rng.Next(10) / 10.0) + 1;
    }

    public ZodiacPlayer? Of(MassPlayer p) => players.GetValueOrDefault(p.Guid);

    uint ElapsedSeconds => (uint)System.Diagnostics.Stopwatch.GetElapsedTime(startedAt).TotalSeconds;

    /// <summary>O tee do buraco (o último 0x1A do cliente).</summary>
    public override void HoleData(byte hole, HoleInfo info)
    {
        base.HoleData(hole, info);
        tee = (info.TeeX, info.TeeZ);
        foreach (var p in players.Values) if (p.HoleShots == 0) (p.TeeX, p.TeeZ) = tee;
    }

    public override void Loaded(MassPlayer p)
    {
        if (Of(p) is not { } zp || p.Started >= 0) return;
        p.Started = 0;
        (zp.TeeX, zp.TeeZ) = tee;
        var (w, d) = NewWind();
        z.Loaded(zp, w, d, seeds);
    }

    /// <summary>0x137: pronto para o 1º buraco. Todos prontos: começa o relógio.</summary>
    public void Ready(MassPlayer p)
    {
        if (Of(p) is not { Ready: false } zp) return;
        zp.Ready = true;
        foreach (var x in players.Values) if (!x.Ready && !x.Mass.Left) return;
        if (running) return;
        running = true;
        foreach (var x in players.Values) if (!x.Mass.Left) z.FirstHole(x);
        if (TotalMs > 0) Later(TimeSpan.FromMilliseconds(TotalMs), TimeIsOver);
        if (IsEvent && TotalMs > 60_000)                                     // golden beam: de 60 s a 30 s antes do fim
        {
            Later(TimeSpan.FromMilliseconds(TotalMs - 60_000), () => { beamOpen = true; z.BeamOpen(); });
            Later(TimeSpan.FromMilliseconds(TotalMs - 30_000), CloseBeam);
        }
    }

    /// <summary>Fim da janela: quem acertou nela divide (ou um leva) o jackpot = jogadores × 500 × (1..4).</summary>
    void CloseBeam()
    {
        beamOpen = false;
        var winners = new List<ZodiacPlayer>();
        foreach (var p in players.Values) if (p.InBeam && !p.Mass.Left) winners.Add(p);
        if (winners.Count == 0) return;
        long jackpot = (long)players.Count * JackpotPerPlayer * (1 + Rng.Next(4));
        bool split = winners.Count > 1 && Rng.Next(2) == 0;
        if (split)
            foreach (var w in winners) w.Jackpot += jackpot / winners.Count;
        else
        {
            var one = winners[Rng.Next(winners.Count)];
            one.Jackpot += jackpot;
            winners = [one];
        }
        z.BeamClose(winners, split);
    }

    public override void TeeShotReady(MassPlayer p) { }

    /// <summary>Bloco da tacada (0x12): +0x10 u8 acerto do pangya, +0x11 u32 comandos (setas).</summary>
    public override void ShotData(MassPlayer p, ReadOnlySpan<byte> block)
    {
        if (Of(p) is not { } zp || block.Length < 0x15) return;
        zp.PangyaFlag = block[0x10];
        zp.SpecialShot = BitConverter.ToUInt32(block[0x11..]);
    }

    public override void Shoot(MassPlayer p, int shotTimeMs)
    {
        if (Of(p) is not { } zp || zp.Finished) return;
        zp.HoleShots++;
        zp.TotalShots++;
        p.ShotOpen = true;
    }

    public override void Result(MassPlayer p, ShotResult r)
    {
        if (Of(p) is not { } zp || zp.Finished) return;
        zp.Pang = r.Pang;
        zp.Bonus = r.BonusPang;
        zp.Holed = (r.Display & DisplayHoled) != 0 || r.State == ShotResult.StateHoled;
        zp.HoleBonuses.Clear();
        if (!zp.Holed) return;
        if ((r.ShotFlags & SpecialShots) != 0) zp.HoleBonuses.Add(BonusSpecial);
        if (zp.HoleShots == 1) zp.HoleBonuses.Add(BonusFirst);
        if (zp.SpecialShot == 0) zp.HoleBonuses.Add(BonusNoCommand);
        if ((zp.PangyaFlag & MissedPangya) != 0) zp.HoleBonuses.Add(BonusMissPangya);
        zp.HoleBonuses.Add(BonusHole);
        if (zp.HoleBonuses.Count > 1) z.ScoreTypes(zp, zp.HoleBonuses);
    }

    public override void ShotFinished(MassPlayer p)
    {
        if (Of(p) is not { } zp || zp.Finished) return;
        p.ShotOpen = false;
        z.EndShot(zp);
        if (!zp.Holed) { z.BackToTee(zp); return; }
        int before = zp.Total, points = Points(zp.HoleBonuses);
        zp.Total += points;
        zp.HoleInOnes++;
        if (IsEvent)
        {
            if (beamOpen) zp.InBeam = true;
            if (Rng.Next(100) < DropChance)
            {
                zp.Drops[DropTypeId] = zp.Drops.GetValueOrDefault(DropTypeId) + DropQuantity;
                z.ItemDropped(zp, DropTypeId, DropQuantity);
            }
        }
        zp.HoleShots = 0;
        zp.Holed = false;
        var (w, d) = NewWind();
        z.Holed(zp, before, points, w, d, ElapsedSeconds);
    }

    /// <summary>Pontos do buraco: +1 cada bônus, +3 o "sem acertar o Pangya".</summary>
    public static int Points(List<byte> bonuses)
    {
        int n = 0;
        foreach (var b in bonuses) n += b == BonusMissPangya ? 3 : 1;
        return n;
    }

    /// <summary>Acabou o tempo: avisa quem ainda joga; quem não terminar em 15 s termina sozinho.</summary>
    void TimeIsOver()
    {
        foreach (var p in players.Values)
            if (!p.Finished && !p.Mass.Left) z.TimeOver(p, TotalMs);
        Later(TimeSpan.FromSeconds(15), () =>
        {
            foreach (var p in players.Values) if (!p.Finished && !p.Mass.Left) Finish(p.Mass, complete: true);
        });
    }

    /// <summary>0x12C (terminou; completo se faltava menos de 1 min) ou saída da prática (0x131, sem completar).</summary>
    public void Finish(MassPlayer p, bool complete)
    {
        if (Of(p) is not { Finished: false } zp) return;
        zp.Finished = true;
        zp.Complete = complete && (TotalMs == 0 || ElapsedSeconds * 1000L >= TotalMs - 60_000);
        p.Finished = true;
        z.Ended(zp);
        CheckEnd();
    }

    protected override int HolesIfQuit(uint guid) => 0;       // como a prática: o Grand Zodiac não entra nos totais do perfil

    protected override int HolesCompleted(uint guid) => players.TryGetValue(guid, out var zp) ? zp.HoleInOnes : 0;   // cada acerto é um buraco

    public override void PlayerLeft(RoomPlayer rp)
    {
        if (players.TryGetValue(rp.Guid, out var zp)) { zp.Mass.Left = true; zp.Finished = true; }
        CheckEnd();
    }

    void CheckEnd()
    {
        if (Over) return;
        foreach (var p in players.Values) if (!p.Finished && !p.Mass.Left) return;
        Cancel();
        z.GameOver(Standings());
        RoomManager.FinishGame(Room);
    }

    /// <summary>Troféu do evento pela posição e o nº de jogadores: &lt; 20 nenhum, 20–29 só o 1º, 30–49 1º e 2º, ≥ 50 os três.</summary>
    public static int TrophyFor(int position, int playerCount)
    {
        int count = playerCount >= 50 ? 3 : playerCount >= 30 ? 2 : playerCount >= 20 ? 1 : 0;
        return position >= 1 && position <= count ? Trophies[3 - count + position - 1] : 0;
    }

    /// <summary>Posição pelo total (empate = mesma posição); EXP e pontos GZ (o Chip-in não ganha).</summary>
    public List<ZodiacResult> Standings()
    {
        var list = new List<ZodiacPlayer>();
        foreach (var p in players.Values) if (!p.Mass.Left) list.Add(p);
        list.Sort((a, b) => b.Total.CompareTo(a.Total));
        var results = new List<ZodiacResult>(list.Count);
        for (int i = 0; i < list.Count; i++)
        {
            int pos = i > 0 && list[i].Total == list[i - 1].Total ? results[i - 1].Position : i + 1;
            var p = list[i];
            int exp = Kind == ZodiacKind.ChipIn ? 0 : p.Complete ? 45 * (121 - pos) / 100 : p.HoleInOnes / 2;
            uint gz = Kind switch { ZodiacKind.Intermediate => (uint)(p.Total * 3.5), ZodiacKind.Advanced => (uint)(p.Total * 5), _ => 0 };
            results.Add(new ZodiacResult(p, pos, exp, gz, IsEvent && !QuitRate.NoTrophy(p.Mass.RoomPlayer.Player) ? TrophyFor(pos, players.Count) : 0));
        }
        return results;
    }
}
