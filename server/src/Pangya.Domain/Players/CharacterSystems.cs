using Pangya.Domain.Shop;

namespace Pangya.Domain.Players;

/// <summary>Etapa do CharacterMastery.iff: stat que ganha +1 slot (0..4), nível mínimo e condições (TP gasto ou quest concluída).</summary>
public sealed record MasteryStep(int Seq, int Stat, int Level, (int TypeId, int Qty)[] Conditions);

/// <summary>Dados do personagem que só o JP tem (docs/server/protocolo/SPEC-jp983-cards.md §5).</summary>
public interface IJpCharacterData
{
    /// <summary>Etapas da maestria do personagem, em ordem (vazio = sem maestria).</summary>
    IReadOnlyList<MasteryStep> MasterySteps(int characterTypeId);
    /// <summary>Peça com CaddieSlot (libera o slot 7 de card).</summary>
    bool OpensCaddieSlot(int partTypeId);
}

/// <summary>Uma linha do 0x216 do JP: tipo 2 (quantidade), 0xC9 (stats), 0xCB (card no slot), 0xCD (maestria).</summary>
public readonly record struct ItemEntry(byte Kind, int TypeId, int Id, int Before = 0, int After = 0, int[]? Stats = null,
    int CardTypeId = 0, byte Slot = 0, int Mastery = 0)
{
    public const byte KindQty = 2, KindStats = 0xC9, KindCard = 0xCB, KindMastery = 0xCD;
    public static ItemEntry Qty(ItemChange c) => new(KindQty, c.TypeId, c.Id, c.Before, c.After);
}

/// <summary>Resultado: código (0 = ok), linhas do 0x216, typeid/stat para a resposta e pang gasto.</summary>
public sealed record CharResult(uint Code, int TypeId = 0, int Stat = 0, int Rarity = 0)
{
    public List<ItemEntry> Entries { get; } = [];
    public long PangSpent { get; set; }
}

/// <summary>
/// Sistemas do personagem no JP 983 (SPEC-jp983-cards.md): 12 slots de card (1–4 personagem, 5–8 caddie, 9–12 NPC; 4 e 8
/// com patcher, 7 com peça de CaddieSlot), maestria (0x187), stat ±1 pela tela da maestria (0x188/0x189) e o Lolo (0x155).
/// Cards no personagem são itens ActiveCard (parts_tid/parts_id = o personagem, slot 1..12, jp = 1): vão sozinhos no 0x137.
/// </summary>
public sealed class CharacterSystems(IPlayerStore store, IGameData data, IJpCharacterData? jp, Random? random = null)
{
    readonly Random rng = random ?? Random.Shared;
    public const int PatcherTypeId = 0x1A00018F, TikiPointsTypeId = 0x1A0002A6;
    static readonly int[] Removers = [0x1A0000C2, 0x1A0000CD, 0x1A0000CE, 0x1A0000CF];
    /// <summary>Custo do Lolo por card (N, R, SR; SC não entra).</summary>
    static readonly int[] LoloCost = [1000, 2000, 5000];
    static readonly int[] RarityWeight = [70, 22, 7, 1];

    const uint EEquip = 0x5200750, EPatch = 0x5200800, ERemove = 0x5200850, EMastery = 0x5200650, EStatUp = 0x5200500,
        EStatDown = 0x5200550, ELolo = 0x5400150;

    static bool IsCharacter(int tid) => Item.GroupOf(tid) == ItemGroup.Character;

    Item? Character(Player p, int tid, int id) =>
        p.Find(id) is { Location: ItemLocation.Inventory } c && c.TypeId == tid && IsCharacter(tid) ? c : null;

    static Item? SlotCard(Player p, int charId, int slot)
    {
        foreach (var it in p.Items.Values)
            if (it.Location == ItemLocation.ActiveCard && it.Int("jp") == 1 && it.Int("parts_id") == charId && it.Int("slot") == slot) return it;
        return null;
    }

    /// <summary>Typeids dos 12 slots de card do personagem (o CharacterInfo 513, bytes 465..512).</summary>
    public static int[] Cards(Player p, int charId)
    {
        var a = new int[12];
        foreach (var it in p.Items.Values)
            if (it.Location == ItemLocation.ActiveCard && it.Int("jp") == 1 && it.Int("parts_id") == charId && it.Int("slot") is >= 1 and <= 12 and var s)
                a[s - 1] = it.TypeId;
        return a;
    }

    public static int Mastery(Item character) => character.Int("mastery");

    static bool Spend(Player p, Item stack, int qty, PlayerChanges ch, List<ItemEntry> e)
    {
        if (stack.Quantity < qty || p.Reserved.GetValueOrDefault(stack.Id) > stack.Quantity - qty) return false;
        int after = stack.Quantity - qty;
        if (after == 0) ch.Removed.Add(stack.Id);
        else { var s = stack.Clone(); s.Quantity = after; ch.Updated.Add(s); }
        e.Add(new ItemEntry(ItemEntry.KindQty, stack.TypeId, stack.Id, stack.Quantity, after));
        return true;
    }

    async Task CommitAsync(Player p, PlayerChanges ch)
    {
        await store.ApplyAsync(p.AccountId, ch);
        foreach (var id in ch.Removed) p.Items.Remove(id);
        foreach (var it in ch.Updated) p.Items[it.Id] = it;
        foreach (var it in ch.Added) p.Add(it);
        if (ch.Pang is { } pang) p.Pang = pang;
    }

    /// <summary>0x18A (patcher = false) / 0x18B (patcher = true): card no slot vazio do personagem.</summary>
    public async Task<CharResult> EquipCardAsync(Player p, int charTid, int charId, int cardTid, int cardId, int slot, bool patcher)
    {
        uint e0 = patcher ? EPatch : EEquip;
        if (Character(p, charTid, charId) is not { } ch) return new(e0 + 1);
        var stack = p.Find(cardId) is { Location: ItemLocation.Inventory } s && s.TypeId == cardTid ? s : p.FindType(cardTid);
        if (stack is not { Quantity: > 0 }) return new(e0 + 2);
        if (!data.Cards.ContainsKey(cardTid)) return new(e0 + 7);
        if (slot is < 1 or > 12) return new(e0 + 5);
        bool patchSlot = slot is 4 or 8;
        if (patcher != patchSlot) return new(e0 + 3);
        if (slot == 7 && !CaddieSlotOpen(p, ch)) return new(e0 + 4);
        int sub = (cardTid >> 22) & 0xF;
        if (sub != (slot <= 4 ? CardInfo.SubCharacter : slot <= 8 ? CardInfo.SubCaddie : CardInfo.SubNpc)) return new(e0 + 6);
        if (SlotCard(p, charId, slot) != null) return new(e0 + 9);
        var r = new CharResult(0, cardTid);
        var change = new PlayerChanges();
        if (!Spend(p, stack, 1, change, r.Entries)) return new(e0 + 8);
        if (patcher && (p.FindType(PatcherTypeId) is not { } patch || !Spend(p, patch, 1, change, r.Entries))) return new(EPatch + 10);
        var active = new Item { Id = (await store.NewIdsAsync(1))[0], TypeId = cardTid, Location = ItemLocation.ActiveCard };
        active.Set("parts_tid", charTid);
        active.Set("parts_id", charId);
        active.Set("slot", slot);
        active.Set("jp", 1);
        change.Added.Add(active);
        r.Entries.Add(new ItemEntry(ItemEntry.KindCard, charTid, charId, CardTypeId: cardTid, Slot: (byte)slot));
        await CommitAsync(p, change);
        return r;
    }

    bool CaddieSlotOpen(Player p, Item ch)
    {
        if (jp == null) return false;
        foreach (var tid in ch.IntArray("parts", 24)) if (tid != 0 && jp.OpensCaddieSlot(tid)) return true;
        return false;
    }

    /// <summary>0x18C: tira o card do slot (gasta 1 removedor); o card volta ao inventário.</summary>
    public async Task<CharResult> RemoveCardAsync(Player p, int charTid, int charId, int removerTid, int removerId, int slot)
    {
        if (Character(p, charTid, charId) == null) return new(ERemove + 1);
        if (Array.IndexOf(Removers, removerTid) < 0) return new(ERemove + 2);
        var rem = p.Find(removerId) is { Location: ItemLocation.Inventory } x && x.TypeId == removerTid ? x : p.FindType(removerTid);
        if (rem is not { Quantity: > 0 }) return new(ERemove + 5);
        if (SlotCard(p, charId, slot) is not { } card) return new(ERemove + 4);
        var r = new CharResult(0, card.TypeId);
        var change = new PlayerChanges();
        if (!Spend(p, rem, 1, change, r.Entries)) return new(ERemove + 5);
        change.Removed.Add(card.Id);
        if (p.FindType(card.TypeId) is { } back)
        {
            var b = back.Clone();
            b.Quantity++;
            change.Updated.Add(b);
            r.Entries.Add(new ItemEntry(ItemEntry.KindQty, b.TypeId, b.Id, back.Quantity, b.Quantity));
        }
        else
        {
            var b = new Item { Id = (await store.NewIdsAsync(1))[0], TypeId = card.TypeId, Quantity = 1 };
            change.Added.Add(b);
            r.Entries.Add(new ItemEntry(ItemEntry.KindQty, b.TypeId, b.Id, 0, 1));
        }
        r.Entries.Add(new ItemEntry(ItemEntry.KindCard, charTid, charId, Slot: (byte)slot));
        await CommitAsync(p, change);
        return r;
    }

    /// <summary>0x187: libera a próxima etapa da maestria (nível mínimo, TP gasto, quests concluídas).</summary>
    public async Task<CharResult> ExpandMasteryAsync(Player p, int charTid, int charId, Func<int, bool> questCleared)
    {
        if (Character(p, charTid, charId) is not { } ch) return new(EMastery + 1);
        var steps = jp?.MasterySteps(charTid);
        if (steps == null || steps.Count == 0) return new(EMastery + 2);
        int m = Mastery(ch);
        if (m >= steps.Count) return new(EMastery + 3);
        var step = steps[m];
        if (step.Seq != m + 1) return new(EMastery + 4);
        if (step.Level > p.Level) return new(EMastery + 5);
        var r = new CharResult(0, charTid);
        var change = new PlayerChanges();
        foreach (var (tid, qty) in step.Conditions)
        {
            if (tid == 0) continue;
            var g = Item.GroupOf(tid);
            if (g == ItemGroup.Usable)
            {
                if (p.FindType(tid) is not { } it || !Spend(p, it, qty, change, r.Entries)) return new(EMastery + 7);
            }
            else if (((uint)tid >> 26) == 0x1D) { if (!questCleared(tid)) return new(EMastery + 9); }   // QuestStuff 0x748...
            else return new(EMastery + 6);
        }
        var e = ch.Clone();
        e.Set("mastery", m + 1);
        change.Updated.Add(e);
        r.Entries.Add(new ItemEntry(ItemEntry.KindMastery, charTid, charId, Mastery: m + 1));
        await CommitAsync(p, change);
        return r;
    }

    /// <summary>0x188 (+1, cobra o Enchant) / 0x189 (−1, sem devolver): upgrade de stat do personagem.</summary>
    public async Task<CharResult> StatAsync(Player p, int charTid, int charId, int stat, bool up)
    {
        uint e0 = up ? EStatUp : EStatDown;
        if (stat is < 0 or > 4) return new(e0 + 1);
        if (Character(p, charTid, charId) is not { } ch) return new(e0 + 2);
        var pcl = ch.IntArray("pcl", 5);
        long cost = 0;
        if (up)
        {
            if (data.UpgradePrice(stat, pcl[stat]) is not { } price) return new(e0 + 4);
            if (price > p.Pang) return new(e0 + 5);
            cost = price;
            pcl[stat]++;
        }
        else
        {
            if (pcl[stat] <= 0) return new(e0 + 3);
            pcl[stat]--;
        }
        var e = ch.Clone();
        e.Set("pcl", pcl);
        var change = new PlayerChanges();
        change.Updated.Add(e);
        if (cost > 0) change.Pang = p.Pang - cost;
        var r = new CharResult(0, charTid, stat) { PangSpent = cost };
        r.Entries.Add(new ItemEntry(ItemEntry.KindStats, charTid, charId, Stats: pcl));
        await CommitAsync(p, change);
        return r;
    }

    /// <summary>0x155 Lolo: 3 cards (N 1000, R 2000, SR 5000 de pang; SC não) viram 1 card sorteado das séries 1..5.</summary>
    public async Task<CharResult> LoloAsync(Player p, long pang, int[] cards)
    {
        if (cards.Length != 3) return new(ELolo + 1);
        long cost = 0;
        var need = new Dictionary<int, int>();
        int lastRarity = 0;
        foreach (var tid in cards)
        {
            if (!data.Cards.TryGetValue(tid, out var ci) || ci.SubType >= CardInfo.SubPack && ci.SubType != CardInfo.SubNpc) return new(ELolo + 2);
            if (ci.Rarity is < 0 or > 2) return new(ELolo + 3);
            cost += LoloCost[ci.Rarity];
            lastRarity = ci.Rarity;
            need[tid] = need.GetValueOrDefault(tid) + 1;
        }
        if (cost != pang) return new(ELolo + 4);
        if (p.Pang < cost) return new(ELolo + 5);
        var r = new CharResult(0) { PangSpent = cost };
        var change = new PlayerChanges { Pang = p.Pang - cost };
        foreach (var (tid, n) in need)
            if (p.FindType(tid) is not { } st || !Spend(p, st, n, change, r.Entries)) return new(ELolo + 6);
        // sorteio: cards finais das séries 1..5 (personagem, caddie, especial, NPC), peso por raridade + bônus da raridade do último
        var pool = new List<CardInfo>();
        var weights = new List<int>();
        int total = 0;
        foreach (var c in data.Cards.Values)
        {
            if (!c.Final || c.Volume is < 1 or > 5 || c.SubType is not (CardInfo.SubCharacter or CardInfo.SubCaddie or CardInfo.SubSpecial or CardInfo.SubNpc)) continue;
            int w = (c.Rarity is >= 0 and < 4 ? RarityWeight[c.Rarity] : 1) + (c.Rarity > 0 ? 3 * (lastRarity + 1) : 0);
            pool.Add(c);
            weights.Add(w);
            total += w;
        }
        if (total == 0) return new(ELolo + 7);
        int roll = rng.Next(total), pick = 0;
        for (; pick < pool.Count - 1; pick++) { if (roll < weights[pick]) break; roll -= weights[pick]; }
        var won = pool[pick];
        Item? stack = null;
        foreach (var u in change.Updated) if (u.TypeId == won.TypeId) stack = u;
        int before;
        if (stack != null) { before = stack.Quantity; stack.Quantity++; }
        else if (p.FindType(won.TypeId) is { } have && !change.Removed.Contains(have.Id))
        {
            stack = have.Clone(); before = have.Quantity; stack.Quantity++; change.Updated.Add(stack);
        }
        else
        {
            stack = new Item { Id = (await store.NewIdsAsync(1))[0], TypeId = won.TypeId, Quantity = 1 };
            before = 0;
            change.Added.Add(stack);
        }
        r.Entries.Add(new ItemEntry(ItemEntry.KindQty, won.TypeId, stack.Id, before, stack.Quantity));
        await CommitAsync(p, change);
        return r with { TypeId = won.TypeId, Rarity = won.Rarity };
    }
}
