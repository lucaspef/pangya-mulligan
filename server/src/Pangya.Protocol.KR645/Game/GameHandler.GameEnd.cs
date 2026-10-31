using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Game;
using Pangya.Domain.Players;
using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.KR645.Game;

/// <summary>
/// Fim de partida de um jogador (docs/server/protocolo/SPEC-resultado-fim-de-jogo.md), em duas fases:
/// 1) BeginGameEnd, antes do placar: calcula a recompensa (sem gravar) e devolve o EXP que vai no registro do 0x64/0x8F
///    ou no 0x77 (a tela de resultado anima a barra a partir dele);
/// 2) FinishGameEnd, quando o cliente manda o 0x06 (estatística final, logo depois do placar; no torneio, quando a tela
///    abre) ou depois de EndWait: grava recompensa, estatística e presentes de nível e só então manda 0x10D/0x43/0xC6.
///    Antes disso a tela somaria o EXP duas vezes (ela parte do EXP que o cliente já tem).
/// </summary>
public sealed partial class GameHandler
{
    const ushort SLevelUp = 0x10D, SItemsWon = 0xF8, SMyItemsWon = 0xCC, SMassResult = 0x77, CGiftList = 0x93;
    static readonly TimeSpan EndWait = TimeSpan.FromSeconds(60);   // o 0x06 do GP só vem depois da pose, da contagem e da premiação

    const ushort SNewRecord = 0xB7;

    sealed record PendingEnd(uint Pang, uint Bonus, int Holes, bool Finished, (int Course, int Score)? Course, int PangRate, int ExpRate,
        Rewards.ExpInput ExpIn, (int RoomTid, int Kind)? Trophy, IReadOnlyList<int> AwardItems,
        IReadOnlyList<(int TypeId, int Count)> Treasure, byte[]? TreasureBoxes)
    {
        /// <summary>A partida, para os contadores de conquista (sala lida no começo do fim; ela pode sumir até o 0x06).</summary>
        public Domain.Achievements.AchievementCounters.GameEnd? Game { get; init; }
        /// <summary>Mana do artefato que o dono gasta (0 = nenhuma).</summary>
        public int SpendMana { get; init; }
        /// <summary>Bola equipada e buracos a descontar dela.</summary>
        public (int TypeId, int Holes) Ball { get; init; }
    }
    PendingEnd? pendingEnd;

    /// <summary>Partida encerrada sem placar (0x65, no 1º buraco): sem recompensa; só volta o status do mensageiro.</summary>
    public void GameAborted()
    {
        MessengerPlaying(false);
        lastGameStats = null;
    }

    /// <summary>Bola equipada e buracos que ela paga nesta partida (sob o lock; só o primeiro pedido do jogador leva).</summary>
    (int TypeId, int Holes) TakeBall(Domain.Rooms.RoomGame? game, uint guid) => (Player.Equip.BallTypeId, game?.TakeBallHoles(guid) ?? 0);

    /// <summary>Desconta a bola (SPEC-jp983-efeitos.md §12); a quantidade é absoluta, o cliente não desconta sozinho.</summary>
    async Task SpendBallAsync((int TypeId, int Holes) ball)
    {
        try
        {
            if (await ctx.Actions.SpendBallAsync(Player, ball.TypeId, ball.Holes, DateTime.UtcNow) is not { } c) return;
            if (BallSpent != null) BallSpent(c);
            else conn.Send(new PacketWriter(SItemCounts).U8(1).U32((uint)c.TypeId).U32((uint)c.Id).U16((ushort)c.After));
            Log.Info($"{conn} bola {c.TypeId:X8}: {ball.Holes} buraco(s), {c.Before} -> {c.After}");
        }
        catch (Exception ex) { Log.Error($"{conn} falha ao gastar a bola", ex); }
    }

    /// <summary>Abandono (sob o lock): descarta o parcial e grava fora do lock. alone: Stroke sem adversário, a janela do cliente não mostra pena.</summary>
    void CountQuit(GameMode mode, int holes, bool alone)
    {
        lastGameStats = null;
        var p = player!;
        long penalty = alone ? 0 : QuitRate.PangPenalty(mode, p.Level, p.Stats.Games, p.Stats.Quits);
        _ = Task.Run(() => SaveQuitAsync(p, penalty, holes));
    }

    async Task SaveQuitAsync(Player p, long penalty, int holes)
    {
        try
        {
            uint tier = QuitRate.IconTier(p);
            await Rewards.QuitAsync(ctx.Players.Store, p, penalty);
            conn.Send(new PacketWriter(SStatsUpdate, 0x160).Struct(PlayerStructs.Statistics(p)).Struct(PlayerStructs.Trophies(p)).U8(0xFF).U8(0xFF));
            if (penalty > 0) conn.Send(PangUpdate());
            if (QuitRate.IconTier(p) != tier)                                   // o ícone mudou: a lista do lobby já foi mandada na saída
                lock (Rooms.Sync) if (Rooms.Lobby.Contains(this)) LobbyUser(LobbyUserUpdate);
            Log.Info($"{conn} abandonou a partida ({holes} buraco(s)): {p.Stats.Quits} abandono(s) em {p.Stats.Games} jogo(s), pena de {penalty} pang");
        }
        catch (Exception ex) { Log.Error($"{conn} falha ao gravar o abandono", ex); }
    }

    /// <summary>
    /// Fase 1: deixa a recompensa pendente e devolve o EXP a mostrar (0 para quem saiu ou está no nível máximo).
    /// players/position/positionPenalty/coursePlayed alimentam a fórmula de EXP do GB (Rewards.Exp). trophy/awardItems:
    /// troféu do torneio que entra na contagem do perfil e o item do prêmio (vai por carta).
    /// </summary>
    public int BeginGameEnd(uint reportedPang, uint reportedBonus, int holes, bool finished, (int Course, int Score)? course = null,
        int players = 1, int position = 0, bool positionPenalty = true, int coursePlayed = 0, (int RoomTid, int Kind)? trophy = null,
        IReadOnlyList<int>? awardItems = null, IReadOnlyList<(int TypeId, int Count)>? treasure = null, byte[]? treasureBoxes = null,
        bool practice = false, int? fixedExp = null)
    {
        var p = player!;
        MessengerPlaying(false);
        var (pangRate, expRate) = RewardRates(DateTime.UtcNow);
        var expIn = new Rewards.ExpInput(players, ctx.Data.CourseStars(coursePlayed), position, positionPenalty, p.Level, practice, fixedExp);
        var r0 = room;
        var end = new PendingEnd(reportedPang, reportedBonus, holes, finished, course, pangRate, expRate, expIn,
            finished ? trophy : null, finished ? awardItems ?? [] : [], finished ? treasure ?? [] : [], finished ? treasureBoxes : null)
        {
            // o dono da sala gasta 1 mana do artefato no fim (referência requestFinishItemUsedGame)
            SpendMana = r0 is { Settings.Artifact: not 0 } && r0.Find(this)?.Master == true ? Domain.Rooms.Artifacts.ManaOf(r0.Settings.Artifact) : 0,
            Ball = r0?.Find(this) is { } me ? TakeBall(r0.Game, me.Guid) : default,
            Game = r0 == null ? null : new Domain.Achievements.AchievementCounters.GameEnd(r0.Settings.Mode, r0.Settings.Practice,
                r0.Settings.RepeatHole, r0.CoursePlayed, holes, r0.Find(this)?.Master == true, finished, null, 0, course,
                (int)r0.Settings.Mode == Domain.Rooms.RoomManager.ShuffleCourseMode, r0.Settings.Zodiac != Domain.Rooms.ZodiacKind.None),
        };
        if (Interlocked.Exchange(ref pendingEnd, end) is { } stale) _ = FinishAsync(stale);     // partida anterior ainda aberta
        _ = Task.Delay(EndWait).ContinueWith(_ => FinishGameEnd(end), TaskScheduler.Default);
        var (_, exp) = Rewards.Compute(reportedPang, reportedBonus, holes, finished, ctx.World.Config.Rewards, pangRate, expRate, expIn);
        return exp;
    }

    /// <summary>Pang% e EXP% de agora: cards, buffs por tempo, premium e o artefato de EXP da sala.</summary>
    (int Pang, int Exp) RewardRates(DateTime now)
    {
        var p = player!;
        var (buffExp, buffPang) = ctx.Buffs.Rates(p, now);                  // buffs por tempo (Sweet Yam, balas) somam aos cards
        int premiumRate = Domain.Players.Premium.RatePercent(p, now);          // premium (JP): +5 % Normal, +10 % VIP
        return (CardService.ActiveRate(p, ctx.Data.Cards, CardInfo.AbilityPangRate, now) + buffPang + premiumRate,
            CardService.ActiveRate(p, ctx.Data.Cards, CardInfo.AbilityExpRate, now) + buffExp + premiumRate
            + Domain.Rooms.Artifacts.ExpPercent(room?.Settings.Artifact ?? 0));   // artefato de EXP da sala
    }

    /// <summary>Fase 2 (uma vez): pelo 0x06, pelo prazo ou ao desconectar. only = só esta partida (o prazo de uma antiga não fecha a nova).</summary>
    void FinishGameEnd(PendingEnd? only = null)
    {
        var end = only == null ? Interlocked.Exchange(ref pendingEnd, null)
            : Interlocked.CompareExchange(ref pendingEnd, null, only) == only ? only : null;
        if (end != null) _ = FinishAsync(end);
    }

    async Task FinishAsync(PendingEnd e)
    {
        var p = player!;
        try
        {
            if (e.TreasureBoxes != null) conn.Send(e.TreasureBoxes);           // caixas do Treasure Hunter de novo (ver TreasureDraw)
            var stats = lastGameStats;
            lastGameStats = null;
            int levelBefore = p.Level;
            var r = await Rewards.ApplyAsync(ctx.Players.Store, p, e.Pang, e.Bonus, e.Holes, e.Finished, ctx.World.Config.Rewards,
                e.Course, e.PangRate, e.ExpRate, stats, e.ExpIn, e.Trophy);
            if (r.Pang != 0 || r.Exp != 0)
                Log.Info($"{conn} recompensa: +{r.Pang} pang, +{r.Exp} EXP{(r.LevelsUp > 0 ? $", subiu {r.LevelsUp} nível(is) -> {p.Level}" : "")}");
            if (e.Trophy is { Kind: > Trophy.None } t) Log.Info($"{conn} troféu {t.Kind} (1 ouro, 2 prata, 3 bronze) da sala {t.RoomTid:X8}");
            if (r.LevelsUp > 0) await LevelUpGiftsAsync(levelBefore, p.Level);
            if (e.SpendMana != 0 && await ctx.Actions.DeleteItemAsync(p, e.SpendMana, 1) is { } mana)
            {
                conn.Send(new PacketWriter(SItemCounts).U8(1).U32((uint)mana.TypeId).U32((uint)mana.Id).U16((ushort)Math.Max(mana.Quantity, 0)));
                Log.Info($"{conn} mana {mana.TypeId:X8} do artefato gasta (sobram {mana.Quantity})");
            }
            await SpendBallAsync(e.Ball);
            foreach (var tid in e.AwardItems) await AwardItemAsync(tid);
            if (e.Finished && e.Holes > 0)                                  // 0x43: totais, EXP/nível novos, troféus e o registro do curso
            {
                var w = new PacketWriter(SStatsUpdate, 0x160).Struct(PlayerStructs.Statistics(p)).Struct(PlayerStructs.Trophies(p));
                if (e.Course is { } c) w.U8((byte)c.Course).Struct(PlayerStructs.MapStat(p, c.Course)); else w.U8(0xFF);
                conn.Send(w.U8(0xFF));
                if (r.NewRecord && e.Course is { } rc)                      // 0xB7 u8 curso: janela "new_record" na volta à sala
                {
                    conn.Send(new PacketWriter(SNewRecord).U8((byte)(rc.Course & 0x7F)));
                    Log.Info($"{conn} recorde novo no curso {rc.Course} (placar {rc.Score})");
                }
            }
            bool treasurePang = e.Treasure.Count > 0 && await DeliverTreasureAsync(e.Treasure);
            if (r.Pang != 0 || r.LevelsUp > 0 || treasurePang) conn.Send(new PacketWriter(SPang).U64((ulong)p.Pang).U64(0));
            if (Achievements is { } sink && e.Game is { } g)                // conquistas: a partida inteira de uma vez (como a referência)
                await sink.RecordAsync(Domain.Achievements.AchievementCounters.ForGame(p, g with { Stats = stats?.Sane(e.Holes), Pang = r.Pang }));
            if (ClubMastery is { } mastery && e.Game is { } mg) await mastery(mg);     // maestria do taco (oficina do JP)
        }
        catch (Exception ex) { Log.Error($"{conn} falha ao gravar a recompensa", ex); }
    }

    /// <summary>
    /// Presentes da tabela do cliente para cada nível alcançado (o 0x10D abre a janela com o do nível novo; o cliente diz
    /// que o presente vai para o correio). Uma carta do sistema por nível; sem correio, direto no inventário.
    /// </summary>
    async Task LevelUpGiftsAsync(int from, int to)
    {
        var p = Player;
        for (int lv = from + 1; lv <= to; lv++)
        {
            var gifts = Levels.Gifts(lv);
            if (gifts.Length == 0) continue;
            if (ctx.Mail != null)
            {
                await ctx.Mail.SendSystemAsync(p.AccountId, "@Pangya", $"Presente do nivel {lv}", gifts);
                continue;
            }
            foreach (var (tid, qty) in gifts)
            {
                if (tid == Levels.PangPouch)
                {
                    p.Pang += qty;
                    await ctx.Players.Store.ApplyAsync(p.AccountId, new PlayerChanges { Pang = p.Pang });
                    continue;
                }
                bool had = p.FindType(tid) != null;
                var (code, granted) = await ctx.Shop.GiveAsync(p, tid, qty);
                if (code != ShopCode.Ok || granted.Count == 0 || p.Find(granted[0].Id) is not { } it)
                {
                    Log.Info($"{conn} presente do nível {lv} ({tid:X8}) não entregue: {code}");
                    continue;
                }
                conn.Send(had && it.IsConsumable
                    ? new PacketWriter(SItemCounts).U8(1).U32((uint)it.TypeId).U32((uint)it.Id).U16((ushort)it.Quantity)
                    : new PacketWriter(SItems).U16(1).U16(1).Struct(PlayerStructs.ItemInfo(it)));
            }
        }
        conn.Send(new PacketWriter(SLevelUp).U8(1).U8((byte)to).U8(0));       // sLevelUpDone {feito, nível, tipo}
        if (ctx.Mail != null) conn.Send(new PacketWriter(SNewMail));           // carta nova: o cliente pede a lista
        Log.Info($"{conn} presentes de nível {from + 1}..{to} enviados{(ctx.Mail != null ? " pelo correio" : "")}");
    }

    /// <summary>
    /// Entrega do Treasure Hunter: grava os prêmios e manda 0x12C (u8 n, n × {u32 uid, u32 tid, u32 id do item, u16 qtd
    /// ganha, u8 0, i32 0, u16 0}); o cliente soma no inventário dele (pang no saldo). true = ganhou pang.
    /// </summary>
    async Task<bool> DeliverTreasureAsync(IReadOnlyList<(int TypeId, int Count)> prizes)
    {
        var p = Player;
        var w = new PacketWriter(STreasureGifts);
        var lines = new List<(int Tid, int Id, int Count)>(prizes.Count);
        long pang = 0;
        foreach (var (tid, count) in prizes)
        {
            if (tid == TreasureHunter.PangTid) { pang += count; lines.Add((tid, 0, count)); continue; }
            var (code, granted) = await ctx.Shop.GiveAsync(p, tid, count);
            if (code == ShopCode.Ok && granted.Count > 0) lines.Add((tid, granted[0].Id, count));
            else Log.Info($"{conn} treasure hunter: 0x{tid:X8} x{count} não entregue ({code})");
        }
        if (pang > 0)
        {
            p.Pang += pang;
            await ctx.Players.Store.ApplyAsync(p.AccountId, new PlayerChanges { Pang = p.Pang });
        }
        w.U8((byte)lines.Count);
        foreach (var (tid, id, count) in lines)
            w.U32((uint)p.AccountId).U32((uint)tid).U32((uint)id).U16((ushort)count).U8(0).I32(0).U16(0);
        conn.Send(w);
        var got = new System.Text.StringBuilder();
        foreach (var (tid, _, count) in lines) got.Append($" 0x{tid:X8}x{count}");
        Log.Info($"{conn} treasure hunter: {lines.Count} prêmio(s):{got}");
        return pang > 0;
    }

    /// <summary>
    /// 0x129 sub 1: u8 n, n × {u8 curso, u32 gauge} — barra de cada mapa na escolha de mapa (o cliente só mostra 700..1000).
    /// O gauge por curso ainda não é simulado: todos cheios, como o GB.
    /// </summary>
    static PacketWriter TreasureGauges()
    {
        const int Courses = 20, Full = 1000;
        var w = new PacketWriter(STreasureGaugeList).U8(1).U8(Courses);
        for (int i = 0; i < Courses; i++) w.U8((byte)i).U32(Full);
        return w;
    }

    /// <summary>Item do troféu do torneio: carta do sistema (sem correio, direto no inventário).</summary>
    async Task AwardItemAsync(int tid)
    {
        if (!ctx.Data.Exists(tid)) { Log.Info($"{conn} item do troféu {tid:X8} não existe no pangya.iff"); return; }
        if (ctx.Mail != null)
        {
            await ctx.Mail.SendSystemAsync(Player.AccountId, "@Pangya", "Premio do torneio", [(tid, 1)]);
            conn.Send(new PacketWriter(SNewMail));
            return;
        }
        var (code, granted) = await ctx.Shop.GiveAsync(Player, tid, 1);
        if (code == ShopCode.Ok && granted.Count > 0 && Player.Find(granted[0].Id) is { } it)
            conn.Send(new PacketWriter(SItems).U16(1).U16(1).Struct(PlayerStructs.ItemInfo(it)));
    }

    /// <summary>0xF8 (não-mass, antes do 0x64): u16 n, n × {u32 guid, u8 dobro 0, u16 k, k × u32 tid} — itens ganhos na partida.</summary>
    public static PacketWriter ItemsWon(IReadOnlyList<(uint Guid, IReadOnlyList<int> Items)> players)
    {
        var w = new PacketWriter(SItemsWon).U16((ushort)players.Count);
        foreach (var (guid, items) in players)
        {
            w.U32(guid).U8(0).U16((ushort)items.Count);
            foreach (var tid in items) w.U32((uint)tid);
        }
        return w;
    }

    /// <summary>
    /// Torneio/approach: 0xCC (meus itens: u8 dobro 0, u16 n, n × tid) e 0x77 (u32 EXP, u32 troféu da sala, u8 troféu
    /// ganho, u8 equipe vencedora, [guild: 4 × u32], 12 × {u32 guid 0xFFFFFFFF = vazio, u32 tid}: [0..5] medalhas (sorte,
    /// mais rápido, drive, chip-in, long putt, recuperação; ainda nenhuma), [6..11] premiados 1º..6º com o item).
    /// Nunca nos outros modos: no campo o 0x77 manda o cliente de volta à sala.
    /// </summary>
    public void SendMassResult(int exp, IReadOnlyList<int> items, bool guild, int matchTid = 0, int myTrophy = 0,
        IReadOnlyList<Domain.Rooms.TourneyAward>? awards = null, IReadOnlyList<Domain.Rooms.TourneyMedal>? medals = null,
        (int Winner, uint PangWin, uint Points, uint PangRed, uint PangBlue)? guildResult = null)
    {
        var cc = new PacketWriter(SMyItemsWon).U8(0).U16((ushort)items.Count);
        foreach (var tid in items) cc.U32((uint)tid);
        conn.Send(cc);
        var w = new PacketWriter(SMassResult, 0x80).U32((uint)Math.Max(exp, 0)).U32((uint)matchTid).U8((byte)myTrophy)
            .U8((byte)(guildResult?.Winner ?? 2));                           // equipe vencedora (2 = nenhuma/empate)
        if (guild)                                                           // tipo 6: meu pang de guilda, meus pontos, pang de cada lado
        {
            var gr = guildResult ?? default;
            w.U32(gr.PangWin).U32(gr.Points).U32(gr.PangRed).U32(gr.PangBlue);
        }
        var slots = new (uint Guid, uint Tid)[12];
        for (int i = 0; i < slots.Length; i++) slots[i] = (0xFFFFFFFF, 0);
        if (awards != null)
            foreach (var a in awards)
                if (a.Position < 6) slots[6 + a.Position] = (a.Player.Guid, (uint)a.ItemTid);
        if (medals != null)
            foreach (var m in medals)
                if (m.Slot is >= 0 and < Domain.Rooms.Medal.Count) slots[m.Slot] = (m.Player.Guid, (uint)m.ItemTid);
        foreach (var (guid, tid) in slots) w.U32(guid).U32(tid);
        conn.Send(w);
    }
}
