namespace Pangya.Domain.Rooms;

/// <summary>O que uma partida por vez manda para os clientes (a camada de protocolo transforma em pacotes).</summary>
public interface IGameOutput
{
    void Wind(byte wind, byte direction);
    void HoleStart(GamePlayer first);
    void TeeReady();
    void NextTurn(GamePlayer p);
    /// <summary>Buraco encerrado. holeWinner: pang battle (0xFFFFFFFF = acumula para o próximo).</summary>
    void NextHole(uint holeWinner);
    void GameEnd(GameEnd end);
    void PlayerLeft(GamePlayer p);
    /// <summary>Vez do bot: a camada de protocolo monta e manda a tacada e chama <see cref="StrokeGame.BotShoot"/>.</summary>
    void BotTurn(GamePlayer bot);
    /// <summary>
    /// Antes da tacada do bot: planeja e, se for usar power shot, avisa os clientes (0x56) e devolve quanto esperar
    /// pela animação de carga antes do <see cref="BotTurn"/>.
    /// </summary>
    TimeSpan BotPrepare(GamePlayer bot) => TimeSpan.Zero;
    /// <summary>Acabou o tempo da tacada (0x5A): os clientes somam a tacada e mandam o 0x1C, como numa tacada.</summary>
    void TimeOut(GamePlayer p) { }
    /// <summary>Os adversários saíram: pergunta ao que ficou se continua sozinho (0x90; resposta 0x36).</summary>
    void AskContinue(GamePlayer p) { }
    /// <summary>Acabou no 1º buraco sem todos terminarem (0x65): sem placar, o cliente volta direto para a sala.</summary>
    void Aborted() { }
}

/// <summary>Estado de um participante na partida.</summary>
public sealed class GamePlayer(RoomPlayer rp, int order)
{
    public RoomPlayer RoomPlayer { get; } = rp;
    /// <summary>Posição na ordem dos slots (desempate da ordem do tee).</summary>
    public int Order { get; } = order;
    public uint Guid => RoomPlayer.Guid;
    public bool IsBot => RoomPlayer.IsBot;
    public int[] Strokes { get; } = new int[18];
    public bool Done { get; set; }
    public bool Holed { get; set; }
    public bool Left { get; set; }
    /// <summary>Bola no buraco atual; HasPos = false enquanto está no tee.</summary>
    public bool HasPos { get; set; }
    public float X { get; set; }
    public float Z { get; set; }
    public uint Pang { get; set; }
    public uint Bonus { get; set; }

    public int TotalStrokes(int holes)
    {
        int t = 0;
        for (int i = 0; i < holes; i++) t += Strokes[i];
        return t;
    }
}

/// <summary>
/// Partida por vez: Stroke, e base de Team/Match (<see cref="SideGame"/>) e Pang Battle (<see cref="SkinsGame"/>).
/// Porte do ingame.py validado com o cliente real (docs/server/protocolo/SPEC-ingame.md). A física e o julgamento da tacada
/// ficam no cliente; o servidor controla turnos, buracos e placar:
/// - o buraco começa quando todos os humanos carregaram: vento + primeiro jogador;
/// - durante o buraco joga quem está mais longe da bandeira (quem ainda está no tee joga antes, na ordem do tee);
/// - o buraco acaba para o jogador quando ele acerta (estado 4) ou chega a par + 4 tacadas;
/// - tempo da tacada: online o relógio do cliente só para no zero; quem passa a vez é o servidor (0x5A), contando a partir
///   do 0x22 que o cliente da vez manda quando o relógio começa (clock.cpp, CClock 0x1e9);
/// - no Stroke, se os adversários saem e sobra um, a partida acaba; depois de <see cref="SoloAfterHoles"/> buracos o cliente
///   pergunta se ele quer continuar sozinho (0x90 -> 0x36; golfrule.cpp msg 0x95 e GoStopDlgResult).
/// </summary>
public class StrokeGame : RoomGame
{
    readonly TimeSpan botDelay, teeFallback;
    readonly Dictionary<uint, GamePlayer> byGuid = [];
    readonly HashSet<uint> loaded = [], teeReady = [], synced = [];
    int teeAcked = -1;
    bool resultSent;
    /// <summary>Muda a cada vez e a cada tacada: o estouro de tempo agendado só vale para a mesma vez.</summary>
    int turnSeq;
    /// <summary>Relógio da vez: de quem, quanto falta (descontado nas pausas) e quando começou a contar.</summary>
    GamePlayer? clockOf;
    TimeSpan clockLeft;
    DateTime clockAt;
    int clockSeq;
    bool paused;
    /// <summary>Quem estourou o tempo: a próxima vez vai para outro, se houver (senão a ordem do tee devolveria a ele).</summary>
    GamePlayer? timedOut;
    enum Solo { None, End, Ask, Asking, Alone }
    Solo solo;

    /// <summary>Folga sobre o tempo da tacada (o 0x22 chega um pouco depois do relógio começar; a tacada pode estar no meio).</summary>
    public TimeSpan ClockGrace { get; set; } = TimeSpan.FromSeconds(3);
    /// <summary>Sem resposta à pergunta de continuar (a janela do cliente fecha em 10 s com "não"): acaba.</summary>
    public static readonly TimeSpan ContinueWait = TimeSpan.FromSeconds(30);
    /// <summary>Buracos terminados para poder continuar sozinho quando os adversários saem (antes disso a partida acaba).</summary>
    public int SoloAfterHoles { get; set; } = 9;

    protected IGameOutput Out { get; }
    /// <summary>Participantes na ordem dos slots (no máximo 4: o cliente tem m_userInfo[4]).</summary>
    public List<GamePlayer> Players { get; } = [];
    public IGameOutput Output => Out;
    public int HoleIndex { get; private set; }
    public byte Hole => HoleAt(HoleIndex);
    public GamePlayer? Turn { get; private set; }
    public bool ShotOpen { get; private set; }
    public bool Started { get; private set; }
    /// <summary>Vento do buraco (bytes do 0x59: intensidade 0..8, direção 0..255).</summary>
    public byte WindStrength { get; private set; }
    public byte WindDirection { get; private set; }
    /// <summary>Vencedor do buraco que acabou de fechar (pang battle; 0xFFFFFFFF = ninguém).</summary>
    protected uint HoleWinner { get; set; } = 0xFFFFFFFF;

    public StrokeGame(Room room, IGameOutput output, object sync, TimeSpan botDelay, TimeSpan teeFallback) : base(room, sync)
    {
        Out = output;
        this.botDelay = botDelay;
        this.teeFallback = teeFallback;
        for (int i = 0; i < room.Players.Count && i < 4; i++)
        {
            var gp = new GamePlayer(room.Players[i], i);
            Players.Add(gp);
            byGuid[gp.Guid] = gp;
        }
    }

    /// <summary>Partida certa para o modo da sala (os modos em massa usam <see cref="MassGame"/>).</summary>
    public static StrokeGame For(Room room, IGameOutput output, object sync, TimeSpan botDelay, TimeSpan teeFallback) =>
        room.Settings.Mode switch
        {
            GameMode.Team => new SideGame(room, output, sync, botDelay, teeFallback, perPlayer: false),
            GameMode.Match => new SideGame(room, output, sync, botDelay, teeFallback, perPlayer: true),
            GameMode.PangBattle => new SkinsGame(room, output, sync, botDelay, teeFallback),
            _ => new StrokeGame(room, output, sync, botDelay, teeFallback),
        };

    public GamePlayer? Find(uint guid) => byGuid.GetValueOrDefault(guid);

    protected bool HasHumans()
    {
        foreach (var p in Players)
            if (!p.IsBot && !p.Left) return true;
        return false;
    }

    bool AllHumansIn(HashSet<uint> set)
    {
        bool any = false;
        foreach (var p in Players)
        {
            if (p.IsBot || p.Left) continue;
            if (!set.Contains(p.Guid)) return false;
            any = true;
        }
        return any;
    }

    // ------------------------------------------------------------------ entradas (pacotes do cliente)

    public void Loaded(GamePlayer p)
    {
        loaded.Add(p.Guid);
        TryStartHole();
    }

    public void TeeShotReady(GamePlayer p)
    {
        teeReady.Add(p.Guid);
        if (AllHumansIn(teeReady) && teeAcked != HoleIndex)
        {
            teeAcked = HoleIndex;
            AfterTee();
        }
    }

    /// <summary>Tacada de um humano. false = repetida (ignorar). Quem tacou passa a ser o da vez.</summary>
    public bool Shoot(GamePlayer p)
    {
        if (Over || (ShotOpen && Turn == p)) return false;
        Turn = p;
        BeginShot(p);
        return true;
    }

    /// <summary>Resultado da tacada (todos os clientes mandam; vale o primeiro, do jogador da vez).</summary>
    public bool Result(ShotResult r)
    {
        var p = Find(r.Guid);
        if (p == null || resultSent || !ShotOpen || p != Turn) return false;
        resultSent = true;
        p.HasPos = true;
        p.X = r.X;
        p.Z = r.Z;
        p.Pang = r.Pang;
        p.Bonus = r.BonusPang;
        if (r.State == ShotResult.StateHoled) p.Holed = p.Done = true;
        else if (r.Penalized) p.Strokes[HoleIndex]++;                                   // água/OB/área não jogável: +1 tacada
        if (!p.Done && p.Strokes[HoleIndex] >= ParOf(Hole) + GiveUpOverPar) p.Done = true;
        OnResult(p, r);
        return true;
    }

    /// <summary>Um cliente terminou de mostrar a tacada; quando todos terminam, passa a vez.</summary>
    public void ShotFinished(GamePlayer p)
    {
        synced.Add(p.Guid);
        TryCloseShot();
    }

    void TryCloseShot()
    {
        if (!ShotOpen || !AllHumansIn(synced)) return;
        if (!resultSent && Turn is { } t && t.Strokes[HoleIndex] >= ParOf(Hole) + GiveUpOverPar) t.Done = true;
        ShotOpen = false;
        Advance();
    }

    /// <summary>É a vez do jogador e não há tacada em andamento (ex.: para usar item).</summary>
    public bool IsTurnOf(GamePlayer p) => Turn == p && !ShotOpen && !Over;

    public override void PlayerLeft(RoomPlayer rp)
    {
        var p = Find(rp.Guid);
        if (p == null || p.Left) return;
        p.Left = p.Done = true;
        if (Over) return;
        Out.PlayerLeft(p);
        if (!HasHumans()) { Cancel(); return; }
        if (solo == Solo.None && (Room.Settings.Mode == GameMode.Stroke || EndsAlone) && Players.Count >= 2 && Remaining() is { IsBot: false })
            solo = !EndsAlone && HoleIndex >= SoloAfterHoles ? Solo.Ask : Solo.End;
        if (solo == Solo.End && !(Started && ShotOpen)) { Finish(); return; }
        if (!Started) TryStartHole();
        else if (ShotOpen) TryCloseShot();      // quem saiu não precisa mais confirmar
        else if (solo == Solo.Ask) { if (teeAcked == HoleIndex) AskSolo(); }   // senão pergunta depois do tee
        else if (Turn == p) Advance();
    }

    /// <summary>Buraco atual incluído; quem fechou o último buraco e só espera os outros já terminou.</summary>
    protected override int HolesIfQuit(uint guid) =>
        Find(guid) is { Left: false } p && !(p.Done && HoleIndex >= HoleCount - 1) ? Math.Min(HoleIndex + 1, HoleCount) : 0;

    /// <summary>Sobrou um: a partida acaba sem perguntar se ele continua (pang battle).</summary>
    protected virtual bool EndsAlone => false;

    /// <summary>O buraco atual já foi decidido pelo modo (match, pang battle), mesmo sem o jogador ter embocado.</summary>
    protected virtual bool HoleScored => false;

    /// <summary>Buracos fechados + o atual se ele embocou, estourou as tacadas ou o modo já decidiu.</summary>
    protected override int HolesCompleted(uint guid) => Find(guid) is { } p
        ? HoleIndex + (Started && (p.Holed || HoleScored || p.Strokes[HoleIndex] >= ParOf(Hole) + GiveUpOverPar) ? 1 : 0) : 0;

    /// <summary>O único participante que ainda joga (null se houver mais de um).</summary>
    GamePlayer? Remaining()
    {
        GamePlayer? one = null;
        foreach (var p in Players)
        {
            if (p.Left) continue;
            if (one != null) return null;
            one = p;
        }
        return one;
    }

    void AskSolo()
    {
        if (Remaining() is not { } me) return;
        solo = Solo.Asking;
        turnSeq++;                                      // o relógio dele não corre durante a pergunta
        Out.AskContinue(me);
        Later(ContinueWait, () => { if (solo == Solo.Asking) Finish(); });
    }

    /// <summary>0x36: resposta à pergunta de continuar sozinho (true = continua; a vez segue como depois de uma tacada).</summary>
    public void ContinueReply(GamePlayer p, bool go)
    {
        if (solo != Solo.Asking || p.Left || Over) return;
        solo = Solo.Alone;
        if (!go) { Finish(); return; }
        Advance();
    }

    /// <summary>
    /// 0x22: o relógio do jogador da vez começou. Sem tacada até o tempo da sala (+ folga), o servidor passa a vez como o
    /// estouro de tempo do bot: +1 tacada e 0x5A para todos.
    /// </summary>
    public void ClockStarted(GamePlayer p)
    {
        uint ms = Room.Settings.ShotTimeMs;
        if (ms == 0 || Over || Turn != p || ShotOpen || p.IsBot || solo == Solo.Asking) return;
        clockOf = p;
        clockLeft = TimeSpan.FromMilliseconds(ms) + ClockGrace;
        if (!paused) RunClock();
    }

    void RunClock()
    {
        if (clockOf is not { } p) return;
        int seq = turnSeq, cs = ++clockSeq;
        clockAt = DateTime.UtcNow;
        Later(clockLeft, () =>
        {
            if (cs != clockSeq || seq != turnSeq || Turn != p || ShotOpen || p.Left || solo == Solo.Asking) return;
            clockOf = null;
            BeginShot(p);
            timedOut = p;
            Out.TimeOut(p);
        });
    }

    /// <summary>0x30: pausa (true) ou volta (false); o relógio da vez para enquanto a partida está pausada.</summary>
    public void Pause(bool on)
    {
        if (on == paused || Over) return;
        paused = on;
        if (on)
        {
            clockSeq++;
            if (clockOf != null) clockLeft -= DateTime.UtcNow - clockAt;
            if (clockLeft < TimeSpan.Zero) clockLeft = TimeSpan.Zero;
        }
        else RunClock();
    }

    // ------------------------------------------------------------------ ganchos dos modos

    /// <summary>Depois de aplicar o resultado da tacada do jogador da vez.</summary>
    protected virtual void OnResult(GamePlayer p, ShotResult r) { }
    /// <summary>Antes de escolher o primeiro jogador de um buraco novo.</summary>
    protected virtual void OnHoleStart() { }
    /// <summary>Fim de uma tacada, antes de escolher o próximo. true = a partida acabou aqui.</summary>
    protected virtual bool OnTurnOver() => false;
    /// <summary>Placar final.</summary>
    protected virtual GameEnd BuildEnd()
    {
        var ranked = new List<GamePlayer>(Players);    // quem saiu por último; menor placar; empate = ordem do slot
        ranked.Sort((a, b) => a.Left != b.Left ? a.Left.CompareTo(b.Left)
            : Score(a) != Score(b) ? Score(a).CompareTo(Score(b)) : a.Order.CompareTo(b.Order));
        var results = new List<GameResult>(Players.Count);
        foreach (var p in Players)
            results.Add(new GameResult(p.Guid, ranked.IndexOf(p) + 1, Score(p), p.TotalStrokes(HoleCount), p.Pang, p.Bonus));
        return new GameEnd { Kind = GameEndKind.Stroke, Results = results };
    }

    // ------------------------------------------------------------------ fluxo

    void TryStartHole()
    {
        if (Started || Over || !AllHumansIn(loaded)) return;
        Started = true;
        foreach (var p in Players) { p.Done = p.Left; p.Holed = false; p.HasPos = false; }
        teeReady.Clear();
        synced.Clear();
        ShotOpen = false;
        OnHoleStart();
        Turn = NextPlayer();
        turnSeq++;
        if (Turn == null) { Finish(); return; }
        NewWind();
        Out.HoleStart(Turn);
        int idx = HoleIndex;
        Later(teeFallback, () =>
        {
            // um cliente que não manda o "pronto para o tee" não pode travar a partida
            if (HoleIndex != idx || teeAcked == idx || !Started) return;
            teeAcked = idx;
            AfterTee();
        });
    }

    void AfterTee()
    {
        Out.TeeReady();
        if (solo == Solo.Ask) AskSolo();
        else MaybeBot();
    }

    void BeginShot(GamePlayer p)
    {
        turnSeq++;
        ShotOpen = true;
        resultSent = false;
        synced.Clear();
        p.Strokes[HoleIndex]++;
    }

    /// <summary>Vento posto por GM (/wind): vale para as próximas tacadas do buraco (0..8, direção 0..255).</summary>
    public void SetWind(byte strength, byte direction)
    {
        if (Over) return;
        WindStrength = Math.Min(strength, (byte)8);
        WindDirection = direction;
        Out.Wind(WindStrength, WindDirection);
    }

    /// <summary>Vento novo: sorteado uma vez por buraco.</summary>
    void NewWind()
    {
        WindStrength = Artifacts.Wind(Room.Settings.Artifact, Rng);      // 0..8; o artefato limita a faixa ou a paridade
        WindDirection = (byte)Rng.Next(256);
        Out.Wind(WindStrength, WindDirection);
    }

    void Advance()
    {
        if (Over || OnTurnOver()) return;
        if (solo == Solo.End) { Finish(); return; }
        if (solo == Solo.Ask) { AskSolo(); return; }
        if (solo == Solo.Asking) return;                // espera a resposta
        GamePlayer? next;
        if (timedOut is { Done: false } skip)
        {
            skip.Done = true;
            next = NextPlayer() ?? skip;
            skip.Done = false;
        }
        else next = NextPlayer();
        timedOut = null;
        turnSeq++;
        if (next != null)
        {
            Turn = next;
            Out.Wind(WindStrength, WindDirection);    // mesmo vento do buraco (o cliente espera o 0x59 antes da vez)
            Out.NextTurn(next);
            MaybeBot();
            return;
        }
        if (HoleIndex + 1 >= HoleCount) { Finish(); return; }
        HoleIndex++;
        loaded.Clear();
        Started = false;
        Turn = null;
        Out.NextHole(HoleWinner);   // o cliente mostra o placar, manda os dados do próximo buraco e o "carregado" de novo
    }

    protected virtual GamePlayer? NextPlayer()
    {
        // 1) alguém ainda no tee: o primeiro na ordem do tee
        //    (1º buraco = ordem dos slots; depois, menos tacadas no buraco anterior primeiro)
        GamePlayer? tee = null;
        int prev = HoleIndex - 1;
        foreach (var p in Players)
        {
            if (p.Left || p.Done || p.HasPos) continue;
            if (tee == null || (prev >= 0 && p.Strokes[prev] < tee.Strokes[prev])) tee = p;
        }
        if (tee != null) return tee;
        // 2) todos já tacaram: stroke = joga quem está mais longe da bandeira
        GamePlayer? far = null;
        float farDist = -1;
        bool hasPin = Holes.TryGetValue(Hole, out var h);
        foreach (var p in Players)
        {
            if (p.Left || p.Done) continue;
            float d = hasPin ? (p.X - h.PinX) * (p.X - h.PinX) + (p.Z - h.PinZ) * (p.Z - h.PinZ) : -p.TotalStrokes(18);
            if (far == null || d > farDist) { far = p; farDist = d; }
        }
        return far;
    }

    /// <summary>Distância (ao quadrado) da bola até a bandeira do buraco atual.</summary>
    protected float PinDistance2(float x, float z) =>
        Holes.TryGetValue(Hole, out var h) ? (x - h.PinX) * (x - h.PinX) + (z - h.PinZ) * (z - h.PinZ) : 0;

    public int Score(GamePlayer p)
    {
        int s = 0;
        for (int i = 0; i < Math.Min(HoleIndex + 1, HoleCount); i++) s += p.Strokes[i] - ParOf(HoleAt(i));
        return s;
    }

    /// <summary>
    /// "Desistir e voltar para a sala" (0x37): só no stroke com um único jogador. Ele conta como quem saiu (sem
    /// recompensa) e a partida termina com o placar.
    /// </summary>
    public bool GiveUp(uint guid)
    {
        var p = Find(guid);
        if (p == null || Over || (Players.Count != 1 && solo != Solo.Alone)) return false;
        p.Left = p.Done = true;
        Finish();
        return true;
    }

    protected void Finish()
    {
        if (Over) return;
        if (EndsOnFirstHole())
        {
            // referência (Versus/Match/PangBattle finish_game): ainda no 1º buraco e nem todos terminaram -> 0x65, sem placar
            Aborted = true;
            Cancel();
            Out.Aborted();
            RoomManager.FinishGame(Room);
            return;
        }
        Cancel();
        Out.GameEnd(BuildEnd());
        RoomManager.FinishGame(Room);
    }

    bool EndsOnFirstHole()
    {
        if (HoleIndex != 0 || Players.Count < 2) return false;
        foreach (var p in Players)
            if (!p.Left && !p.Done) return true;
        foreach (var p in Players)                                     // todos saíram ou desistiram antes de acabar o buraco
            if (!p.Left && p.Done) return false;
        return true;
    }

    // ------------------------------------------------------------------ bot

    void MaybeBot()
    {
        if (Turn is not { IsBot: true } bot || ShotOpen || Over) return;
        Later(botDelay, () =>
        {
            if (Turn != bot || ShotOpen || !HasHumans()) return;
            var charge = Out.BotPrepare(bot);
            if (charge <= TimeSpan.Zero) { Out.BotTurn(bot); return; }
            Later(charge, () =>                                          // power shot: espera a animação de carga
            {
                if (Turn != bot || ShotOpen || !HasHumans()) return;
                Out.BotTurn(bot);
            });
        });
    }

    /// <summary>A camada de protocolo mandou a tacada do bot (ou o estouro de tempo dele).</summary>
    public void BotShoot(GamePlayer bot) => BeginShot(bot);
}
