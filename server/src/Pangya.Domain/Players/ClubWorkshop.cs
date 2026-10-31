using Pangya.Domain.Rooms;
using Pangya.Domain.Shop;

namespace Pangya.Domain.Players;

/// <summary>Bloco work_shop do ClubSet.iff (docs/server/protocolo/SPEC-jp983-oficina.md §4.1) e os stats/slots base do taco.</summary>
public sealed record ClubWorkshopInfo(int TypeId, int Type, int RankSStat, int TotalRecovery, float Rate, int RankUpRow,
    bool Transform, int Family, int[] Attr, int[] Slot);

/// <summary>Tabelas da oficina (só o JP tem).</summary>
public interface IClubWorkshopData
{
    IReadOnlyDictionary<int, ClubWorkshopInfo> ClubWorkshop { get; }
    /// <summary>LevelUpProb: peso de cada stat (5) pelo tipo do taco.</summary>
    int[]? LevelUpWeights(int type);
    /// <summary>LevelUpLimit: limite de Slot + ws.c por stat (5) pelo tipo e rank.</summary>
    int[]? LevelUpLimit(int type, int rank);
    /// <summary>RankUpExp: maestria para chegar a cada rank (6) pela linha (tipo_rank_s).</summary>
    int[]? RankUpCost(int row);
    /// <summary>Taco especial da família (text_pangya) no rank pedido (pela soma dos slots base); 0 = nenhum.</summary>
    int SpecialAt(int family, int rank);
}

/// <summary>Estado da oficina num taco: slots ganhos (ws.c), maestria e cancelamentos usados (attrs wsc/wsm/wsr).</summary>
public static class ClubWorkshop
{
    public const int Stats = 5;
    public const int KitTypeId = 0x1A00020F, ShardTypeId = 0x7C800026, RankCardTypeId = 0x7C800041, CoatingTypeId = 0x1A000210,
        UcimChipTypeId = 0x1A000211, SoftPowderTypeId = 0x1A000247, HardPowderTypeId = 0x1A00024B;
    /// <summary>Bônus na maestria: Drops of Ebot no inventário (+200 %), anel Graphite equipado (+10 %).</summary>
    public const int EbotTypeId = 0x1A000338, GraphiteRingTypeId = 0x70210009;
    /// <summary>Maestria por chip UCIM na transferência.</summary>
    public const int MasteryPerChip = 300;

    public static int[] Slots(Item it) => it.IntArray("wsc", Stats);
    public static int Mastery(Item it) => it.Int("wsm");
    public static int Recovery(Item it) => it.Int("wsr");

    /// <summary>Soma dos slots (base + oficina): 30..59 dá o rank (E..S) e o nível (0..4); fora disso rank −1.</summary>
    public static int Sum(ClubWorkshopInfo info, int[] c)
    {
        int s = 0;
        for (int i = 0; i < Stats; i++) s += info.Slot[i] + c[i];
        return s;
    }
    public static int RankOf(int sum) => sum is >= 30 and <= 59 ? (sum - 30) / 5 : -1;
    public static int LevelOf(int sum) => sum is >= 30 and <= 59 ? (sum - 30) % 5 : 0;
    public static int Rank(ClubWorkshopInfo info, Item it) => RankOf(Sum(info, Slots(it)));
    public static int Level(ClubWorkshopInfo info, Item it) => LevelOf(Sum(info, Slots(it)));

    /// <summary>
    /// Maestria por buraco terminado (SPEC §1.1): 10 × rate do taco × taxa do servidor × taxa de bônus (100 = 1×);
    /// Special Shuffle 15; Approach 1 (sem o 10 e sem o rate); Grand Zodiac 0.
    /// </summary>
    public static int PerHole(GameMode mode, bool shuffle, bool zodiac, float clubRate, int serverRate, int bonusRate)
    {
        if (zodiac) return 0;
        double t = (serverRate <= 0 ? 1.0 : serverRate / 100.0) * (bonusRate <= 0 ? 1.0 : bonusRate / 100.0);
        if (mode is GameMode.Approach or GameMode.NewApproach) return (int)t;
        return (int)((shuffle ? 15 : 10) * clubRate * t);
    }

    /// <summary>Taxa de bônus da maestria do jogador (100 = 1×): Drops of Ebot no inventário, anel Graphite no personagem.</summary>
    public static int BonusRate(Player p)
    {
        int rate = 100;
        if (p.FindType(EbotTypeId) is { Quantity: > 0 }) rate += 200;
        if (p.Find(p.Equip.CharacterId) is { } ch)
            foreach (var a in ch.IntArray("aux", 5)) if (a == GraphiteRingTypeId) { rate += 10; break; }
        return rate;
    }
}

/// <summary>Resultado de um pedido da oficina: código (0 = ok), stat, itens gastos (0x216 tipo 2) e tacos mudados (0xCC).</summary>
public sealed record WorkshopResult(uint Code, int Stat = 0, int ClubId = 0, int TypeId = 0, bool Offer = false)
{
    public List<ItemChange> Spent { get; } = [];
    public List<Item> Clubs { get; } = [];
    /// <summary>Taco removido (transformação) e pang devolvido (reset Hard).</summary>
    public Item? Removed { get; set; }
    public long PangBack { get; set; }
}

/// <summary>Pendências da oficina numa conexão: o nível sorteado (0x164 até 0x165/0x166) e a transformação oferecida.</summary>
public sealed class WorkshopSession
{
    public (int ClubId, int Stat)? Level { get; set; }
    public (int ClubId, int Stat, int TypeId)? Transform { get; set; }
}

/// <summary>
/// Oficina de club set do JP 983 (SPEC-jp983-oficina.md): subir nível (slot sorteado), subir rank (gasta maestria; pode
/// oferecer um taco especial), recuperar cancelamentos, transferir maestria, resetar e a maestria ganha nas partidas.
/// </summary>
public sealed class ClubWorkshopService(IPlayerStore store, IClubWorkshopData data, IGameData game, Random? random = null)
{
    readonly Random rng = random ?? Random.Shared;
    /// <summary>Chance (em 11 250) de cada um dos 3 especiais no rank-up de um taco transformável (referência: 250 cada).</summary>
    public int TransformWeight { get; init; } = 250;
    public int NoTransformWeight { get; init; } = 10_500;
    public static readonly int[] Specials = [0x1000005D, 0x1000005E, 0x1000005F];

    const uint ELevel = 0x5300200, EConfirm = 0x5300300, ECancel = 0x5300250, ERank = 0x5300350, ETransform = 0x5300450,
        ENoTransform = 0x5300400, ERecover = 0x5300150, ETransfer = 0x5300100, EReset = 0x5300500;

    public IClubWorkshopData Data => data;

    (Item Club, ClubWorkshopInfo Info)? Club(Player p, int id) =>
        p.Find(id) is { Location: ItemLocation.Inventory } c && c.Group == ItemGroup.ClubSet
            && data.ClubWorkshop.TryGetValue(c.TypeId, out var info) ? (c, info) : null;

    /// <summary>Tira qty do item (typeid) do inventário em ch; false = não tem o bastante.</summary>
    static bool Spend(Player p, int typeId, int qty, PlayerChanges ch, WorkshopResult r)
    {
        if (qty <= 0) return true;
        if (p.FindType(typeId) is not { } it || it.Quantity < qty) return false;
        int after = it.Quantity - qty;
        if (after == 0) ch.Removed.Add(it.Id);
        else { var e = it.Clone(); e.Quantity = after; ch.Updated.Add(e); }
        r.Spent.Add(new ItemChange(it.TypeId, it.Id, it.Quantity, after));
        return true;
    }

    static Item Edit(Item club, int[] c, int? mastery = null, int? recovery = null)
    {
        var e = club.Clone();
        e.Set("wsc", c);
        if (mastery is { } m) e.Set("wsm", Math.Max(0, m));
        if (recovery is { } rc) e.Set("wsr", Math.Max(0, rc));
        return e;
    }

    async Task<WorkshopResult> CommitAsync(Player p, PlayerChanges ch, WorkshopResult r)
    {
        await store.ApplyAsync(p.AccountId, ch);
        foreach (var id in ch.Removed) p.Items.Remove(id);
        foreach (var it in ch.Updated) p.Items[it.Id] = it;
        if (ch.Pang is { } pang) p.Pang = pang;
        return r;
    }

    /// <summary>
    /// 0x164: +1 slot num stat sorteado (pesos do LevelUpProb, só os abaixo do limite do rank). Material: o kit (qtd) ou o
    /// shard (qtd 1..5 cards: favorece 1 impacto, 2 curva, 3 força, 4 spin, 5 controle com +qtd×200 no peso).
    /// </summary>
    public async Task<WorkshopResult> UpLevelAsync(Player p, WorkshopSession s, int material, int qty, int clubId)
    {
        if (material is not (ClubWorkshop.KitTypeId or ClubWorkshop.ShardTypeId)) return new(ELevel + 1);
        if (qty <= 0) return new(ELevel + 3);
        if (Club(p, clubId) is not var (club, info)) return new(ELevel + 5);
        if (info.Type < 0) return new(ELevel + 7);
        var c = ClubWorkshop.Slots(club);
        int sum = ClubWorkshop.Sum(info, c), rank = ClubWorkshop.RankOf(sum);
        if (rank < 0) return new(ELevel + 10);
        if (ClubWorkshop.LevelOf(sum) == 4) return new(ELevel + 10);              // o próximo passo é o rank-up
        if (data.LevelUpWeights(info.Type) is not { } weights || data.LevelUpLimit(info.Type, rank) is not { } limit) return new(ELevel + 9);
        var w = new int[ClubWorkshop.Stats];
        int favored = material == ClubWorkshop.ShardTypeId ? qty switch { 2 => 4, 3 => 0, 4 => 3, 5 => 1, _ => 2 } : -1;
        int total = 0;
        for (int i = 0; i < w.Length; i++)
        {
            if (info.Slot[i] + c[i] >= limit[i]) continue;
            w[i] = weights[i] + (i == favored ? qty * 200 : 0);
            total += w[i];
        }
        if (total <= 0) return new(ELevel + 11);
        int roll = rng.Next(total), stat = 0;
        for (; stat < w.Length - 1; stat++) { if (roll < w[stat]) break; roll -= w[stat]; }
        var r = new WorkshopResult(0, stat, clubId, club.TypeId);
        var ch = new PlayerChanges();
        if (!Spend(p, material, material == ClubWorkshop.ShardTypeId ? qty : 1, ch, r)) return new(ELevel + 3);
        c[stat]++;
        var e = Edit(club, c);
        ch.Updated.Add(e);
        r.Clubs.Add(e);
        s.Level = (clubId, stat);
        return await CommitAsync(p, ch, r);
    }

    /// <summary>0x165: fica com o slot (já gravado).</summary>
    public WorkshopResult ConfirmLevel(Player p, WorkshopSession s)
    {
        if (s.Level is not var (id, stat) || p.Find(id) is not { } club) return new(EConfirm + 1);
        s.Level = null;
        var r = new WorkshopResult(0, stat, id, club.TypeId);
        r.Clubs.Add(club);
        return r;
    }

    /// <summary>0x166: desfaz o slot e gasta 1 cancelamento (até o total_recovery do taco).</summary>
    public async Task<WorkshopResult> CancelLevelAsync(Player p, WorkshopSession s)
    {
        if (s.Level is not var (id, stat) || Club(p, id) is not var (club, info)) return new(ECancel + 1);
        int rec = ClubWorkshop.Recovery(club);
        if (rec >= info.TotalRecovery) return new(ECancel + 4);
        var c = ClubWorkshop.Slots(club);
        if (c[stat] <= 0) return new(ECancel + 2);
        c[stat]--;
        s.Level = null;
        var r = new WorkshopResult(0, stat, id, club.TypeId);
        var ch = new PlayerChanges();
        var e = Edit(club, c, recovery: rec + 1);
        ch.Updated.Add(e);
        r.Clubs.Add(e);
        return await CommitAsync(p, ch, r);
    }

    /// <summary>
    /// 0x167: rank + 1 com +1 slot no stat escolhido pelos cards (0 impacto, 1 curva, 2 força, 3 spin, 4 controle); gasta a
    /// maestria do RankUpExp, zera os cancelamentos e, no S, +1 no rank_s_stat. Taco transformável pode oferecer um especial
    /// (Offer: o cliente pergunta; 0x168/0x169).
    /// </summary>
    public async Task<WorkshopResult> UpRankAsync(Player p, WorkshopSession s, int card, int qty, int clubId)
    {
        if (qty is < 0 or > 4) return new(ERank + 6);
        if (qty > 0 && card != ClubWorkshop.RankCardTypeId) return new(ERank + 1);
        if (Club(p, clubId) is not var (club, info)) return new(ERank + 3);
        if (info.Type < 0) return new(ERank + 5);
        var c = ClubWorkshop.Slots(club);
        int sum = ClubWorkshop.Sum(info, c), rank = ClubWorkshop.RankOf(sum);
        if (rank is < 0 or >= 5) return new(ERank + 11);
        if (ClubWorkshop.LevelOf(sum) != 4) return new(ERank + 11);              // precisa estar no nível 4
        int next = rank + 1;
        if (data.LevelUpLimit(info.Type, next) is not { } limit) return new(ERank + 9);
        int stat = qty switch { 1 => 4, 2 => 0, 3 => 3, 4 => 1, _ => 2 };
        if (info.Slot[stat] + c[stat] >= limit[stat]) return new(ERank + 8);
        if (data.RankUpCost(info.RankUpRow) is not { } cost) return new(ERank + 9);
        int mastery = ClubWorkshop.Mastery(club);
        if (mastery < cost[next]) return new(ERank + 10);
        var r = new WorkshopResult(0, stat, clubId, club.TypeId);
        var ch = new PlayerChanges();
        if (!Spend(p, card, qty, ch, r)) return new(ERank + 2);
        c[stat]++;
        if (next == 5 && info.RankSStat is >= 0 and < 5) c[info.RankSStat]++;
        var e = Edit(club, c, mastery - cost[next], 0);
        ch.Updated.Add(e);
        r.Clubs.Add(e);
        await CommitAsync(p, ch, r);
        if (info.Transform && RollSpecial() is int fam && data.SpecialAt(fam, next) is var special && special != 0 && p.FindType(special) == null)
        {
            s.Transform = (clubId, stat, special);
            return new WorkshopResult(0, stat, clubId, special, Offer: true) { }.With(r);
        }
        return r;
    }

    /// <summary>Sorteio da transformação: a família do especial (93/94/95 = Specials) ou null.</summary>
    int? RollSpecial()
    {
        int total = TransformWeight * Specials.Length + NoTransformWeight;
        if (total <= 0) return null;
        int roll = rng.Next(total);
        for (int i = 0; i < Specials.Length; i++)
        {
            if (roll < TransformWeight) return data.ClubWorkshop.TryGetValue(Specials[i], out var sp) ? sp.Family : null;
            roll -= TransformWeight;
        }
        return null;
    }

    /// <summary>0x168 (sim): troca o taco pelo especial, levando a maestria que sobrou; 0x169 (não): fica como está.</summary>
    public async Task<WorkshopResult> TransformAsync(Player p, WorkshopSession s, bool accept, Func<int, Task<Item?>> create)
    {
        if (s.Transform is not var (id, stat, special) || p.Find(id) is not { } club) return new(accept ? ETransform + 1 : ENoTransform + 1);
        s.Transform = null;
        if (!accept) return new WorkshopResult(0, stat, id, club.TypeId);
        if (!data.ClubWorkshop.ContainsKey(special)) return new(ETransform + 3);
        if (await create(special) is not { } made) return new(ETransform + 5);
        var e = made.Clone();
        e.Set("wsm", ClubWorkshop.Mastery(club));
        var ch = new PlayerChanges();
        ch.Removed.Add(club.Id);
        ch.Updated.Add(e);
        var r = new WorkshopResult(0, stat, e.Id, special) { Removed = club };
        r.Clubs.Add(e);
        await CommitAsync(p, ch, r);
        if (p.Equip.ClubSetId == club.Id)
        {
            p.Equip.ClubSetId = e.Id;
            await store.ApplyAsync(p.AccountId, new PlayerChanges { Equip = p.Equip });
        }
        return r;
    }

    /// <summary>0x16B: o spray zera os cancelamentos usados.</summary>
    public async Task<WorkshopResult> RecoverAsync(Player p, int item, int clubId)
    {
        if (item != ClubWorkshop.CoatingTypeId) return new(ERecover + 1);
        if (Club(p, clubId) is not var (club, info)) return new(ERecover + 3);
        if (info.Type < 0) return new(ERecover + 5);
        if (ClubWorkshop.Recovery(club) == 0) return new(ERecover + 7);
        var r = new WorkshopResult(0, 0, clubId, club.TypeId);
        var ch = new PlayerChanges();
        if (!Spend(p, item, 1, ch, r)) return new(ERecover + 1);
        var e = Edit(club, ClubWorkshop.Slots(club), recovery: 0);
        ch.Updated.Add(e);
        r.Clubs.Add(e);
        return await CommitAsync(p, ch, r);
    }

    /// <summary>0x16C: passa até qtd × 300 de maestria de um taco para outro (o destino evolui e não é S), gastando os chips.</summary>
    public async Task<WorkshopResult> TransferAsync(Player p, int chip, int fromId, int toId, int qty)
    {
        if (chip != ClubWorkshop.UcimChipTypeId) return new(ETransfer + 4);
        if (qty <= 0) return new(ETransfer + 5);
        if (fromId == toId || Club(p, fromId) is not var (from, _) || Club(p, toId) is not var (to, toInfo)) return new(ETransfer + 1);
        if (toInfo.Type < 0) return new(ETransfer + 3);
        if (ClubWorkshop.Rank(toInfo, to) == 5) return new(ETransfer + 8);
        int have = ClubWorkshop.Mastery(from);
        if (have <= 0 || qty > (have + ClubWorkshop.MasteryPerChip - 1) / ClubWorkshop.MasteryPerChip) return new(ETransfer + 6);
        int moved = Math.Min(qty * ClubWorkshop.MasteryPerChip, have);
        var r = new WorkshopResult(0, 0, toId, to.TypeId);
        var ch = new PlayerChanges();
        if (!Spend(p, chip, qty, ch, r)) return new(ETransfer + 5);
        var ef = Edit(from, ClubWorkshop.Slots(from), have - moved);
        var et = Edit(to, ClubWorkshop.Slots(to), ClubWorkshop.Mastery(to) + moved);
        ch.Updated.Add(ef);
        ch.Updated.Add(et);
        r.Clubs.Add(ef);
        r.Clubs.Add(et);
        return await CommitAsync(p, ch, r);
    }

    /// <summary>
    /// 0x16D: volta o taco ao rank base (slots da oficina e upgrades com pang zerados; a maestria fica). O pó Hard devolve
    /// 50 % da maestria gasta nos ranks e do pang gasto nos upgrades.
    /// </summary>
    public async Task<WorkshopResult> ResetAsync(Player p, int powder, int clubId)
    {
        if (powder is not (ClubWorkshop.SoftPowderTypeId or ClubWorkshop.HardPowderTypeId)) return new(EReset + 6);
        if (Club(p, clubId) is not var (club, info)) return new(EReset + 3);
        var c = ClubWorkshop.Slots(club);
        int baseRank = ClubWorkshop.RankOf(ClubWorkshop.Sum(info, new int[ClubWorkshop.Stats])), rank = ClubWorkshop.RankOf(ClubWorkshop.Sum(info, c));
        if (baseRank < 0 || rank < 0) return new(EReset + 6);
        var r = new WorkshopResult(0, 0, clubId, club.TypeId);
        var ch = new PlayerChanges();
        if (!Spend(p, powder, 1, ch, r)) return new(EReset + 1);
        int mastery = ClubWorkshop.Mastery(club);
        if (powder == ClubWorkshop.HardPowderTypeId)
        {
            if (data.RankUpCost(info.RankUpRow) is not { } cost) return new(EReset + 5);
            int spentM = 0;
            for (int k = baseRank + 1; k <= rank && k < cost.Length; k++) spentM += cost[k];
            long spentP = 0;
            var pcl0 = club.IntArray("pcl", ClubWorkshop.Stats);
            for (int i = 0; i < ClubWorkshop.Stats; i++)
                for (int lv = 0; lv < pcl0[i]; lv++) spentP += game.UpgradePrice(i, lv) ?? 0;
            mastery += spentM / 2;
            r.PangBack = spentP / 2;
            if (r.PangBack > 0) ch.Pang = p.Pang + r.PangBack;
        }
        var e = Edit(club, new int[ClubWorkshop.Stats], mastery, 0);
        e.Set("pcl", new int[ClubWorkshop.Stats]);
        ch.Updated.Add(e);
        r.Clubs.Add(e);
        return await CommitAsync(p, ch, r);
    }

    /// <summary>Maestria da partida no taco (holes × por buraco). null = nada a gravar.</summary>
    public async Task<Item?> AddMasteryAsync(Player p, int clubId, int holes, GameMode mode, bool shuffle, bool zodiac, int serverRate)
    {
        if (holes <= 0 || Club(p, clubId) is not var (club, info)) return null;
        int gain = holes * ClubWorkshop.PerHole(mode, shuffle, zodiac, info.Rate, serverRate, ClubWorkshop.BonusRate(p));
        if (gain <= 0) return null;
        var e = Edit(club, ClubWorkshop.Slots(club), ClubWorkshop.Mastery(club) + gain);
        var ch = new PlayerChanges();
        ch.Updated.Add(e);
        await CommitAsync(p, ch, new WorkshopResult(0));
        return e;
    }
}

static class WorkshopResultExt
{
    /// <summary>Copia os itens gastos e os tacos de outro resultado.</summary>
    public static WorkshopResult With(this WorkshopResult to, WorkshopResult from)
    {
        to.Spent.AddRange(from.Spent);
        to.Clubs.AddRange(from.Clubs);
        return to;
    }
}
