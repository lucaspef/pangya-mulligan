using System.Buffers.Binary;
using Pangya.Core.Iff;
using Pangya.Core.Text;
using Pangya.Domain.Players;
using Pangya.Domain.Shop;

namespace Pangya.Protocol.JP983;

/// <summary>
/// Dados do jogo do cliente JP 983 (pangya_jp.iff do pak mais novo) vistos pelo Domain: o mesmo IGameData do KR, lido por
/// offset (docs/server/protocolo/SPEC-jp983-iff.md). O cabeçalho comum tem 0xC0 bytes (KR 0x90: nome 64 e um bloco da Tiki Shop);
/// os campos de cada tabela começam em 0xC0.
/// </summary>
public sealed class Jp983GameData : IGameData, Domain.Shop.IMemorialData, Domain.Achievements.IAchievementData, Domain.Rooms.IGrandPrixData,
    Domain.Players.IClubWorkshopData, Domain.Players.IJpCharacterData,
    Domain.Achievements.IDailyQuestData, Domain.Shop.ITikiData, Domain.Players.IItemBuffData
{
    // cabeçalho comum (IFF_ITEM_COMMON do JP)
    const int OTypeId = 0x04, OLevel = 0x48, OPrice = 0x74, OSalePrice = 0x78, OShop = 0x80;

    readonly IffArchive archive;
    readonly HashSet<int> typeIds = [];
    readonly HashSet<int> parts = [];
    readonly Dictionary<int, UccPartInfo> ucc = [];

    /// <summary>Tabelas de item cujos typeids o cliente conhece (todas com o cabeçalho comum).</summary>
    static readonly string[] ItemTables =
    [
        "Character.iff", "Part.iff", "HairStyle.iff", "Item.iff", "Ball.iff", "Club.iff", "ClubSet.iff", "Caddie.iff",
        "CaddieItem.iff", "Mascot.iff", "AuxPart.iff", "Card.iff", "Skin.iff", "SetItem.iff", "Furniture.iff", "CounterItem.iff",
        "QuestItem.iff", "QuestStuff.iff", "Achievement.iff",
    ];

    public Jp983GameData(IffArchive archive)
    {
        this.archive = archive;
        foreach (var t in ItemTables)
            foreach (var id in archive.TypeIds(t)) typeIds.Add(id);
        foreach (var r in Rows("Part.iff"))
        {
            int tid = I32(r, OTypeId);
            parts.Add(tid);
            int category = I32(r, 0xE8);                                    // u32 no JP (u8 no KR); Self Design = 8 e 9
            if (category is 7 or 8 or 9) ucc[tid] = UccInfo(r, category);
            int sub0 = I32(r, 0x220), sub1 = I32(r, 0x224);                // SubPart[2] (KR 0x1F0)
            if (sub0 != 0 || sub1 != 0) subParts[tid] = [sub0, sub1];
        }
    }

    /// <summary>Lê o pangya_jp.iff; null se o arquivo não existe.</summary>
    public static Jp983GameData? Load(string path) => File.Exists(path) ? new Jp983GameData(IffArchive.Load(path)) : null;

    public int Count => typeIds.Count;

    /// <summary>Registros de uma tabela, um span por registro (tabela ausente = nenhum).</summary>
    List<ReadOnlyMemory<byte>> Rows(string table)
    {
        var all = archive.Records(table, out int count, out int size);
        var list = new List<ReadOnlyMemory<byte>>(count);
        for (int i = 0; i < count; i++) list.Add(all.Slice(i * size, size));
        return list;
    }

    // ------------------------------------------------------------------ Grand Prix (SPEC-jp983-grandprix.md §4)

    Dictionary<int, Domain.Rooms.GrandPrixDef>? grandPrix;
    Dictionary<int, List<Domain.Rooms.GrandPrixRankReward>>? gpRewards;
    Dictionary<int, List<int>>? gpBots;

    static (int, int, int)[] Items(ReadOnlyMemory<byte> r, int tids, int qtys, int days)
    {
        var list = new List<(int, int, int)>(5);
        for (int i = 0; i < 5; i++) if (I32(r, tids + i * 4) is var t && t != 0) list.Add((t, I32(r, qtys + i * 4), I32(r, days + i * 4)));
        return [.. list];
    }

    /// <summary>GrandPrixData.iff (0x330): offsets da SPEC §4.1; o nome é Shift-JIS.</summary>
    public IReadOnlyDictionary<int, Domain.Rooms.GrandPrixDef> GrandPrix
    {
        get
        {
            if (grandPrix != null) return grandPrix;
            var map = new Dictionary<int, Domain.Rooms.GrandPrixDef>();
            foreach (var r in Rows("GrandPrixData.iff"))
            {
                if (r.Length < 0x130 || !Active(r)) continue;
                var span = r.Span;
                int id = I32(r, 4);
                string name = Cp949.Japanese.GetString(span.Slice(18, 66)[..Math.Max(0, span.Slice(18, 66).IndexOf((byte)0) is var z && z >= 0 ? z : 66)]);
                bool rookie = ((id & 0xFFFFFF) >> 19) == 0 && ((id >> 24) & 3) != 3;
                int open = rookie ? -1 : span[240 + 8] * 60 + span[240 + 10], start = rookie ? -1 : span[256 + 8] * 60 + span[256 + 10];
                map[id] = new Domain.Rooms.GrandPrixDef(id, I32(r, 8), I32(r, 12), I16(r, 16), name, I32(r, 84), I32(r, 88),
                    span[133] != 0, span[134] != 0, I32(r, 136), (byte)I32(r, 140), (byte)I32(r, 144), span[148], span[149], span[150],
                    (I32(r, 160), I32(r, 164), I32(r, 168)), I32(r, 172), Items(r, 180, 200, 220), open, start, I32(r, 292), I32(r, 296) != 0);
            }
            // buracos especiais (GrandPrixSpecialHole.iff, 20 B: ativo, Link, HolePOS, mapa, buraco), em ordem de HolePOS
            var special = new Dictionary<int, List<(int Pos, byte Course, byte Hole)>>();
            foreach (var r in Rows("GrandPrixSpecialHole.iff"))
            {
                if (r.Length < 20 || !Active(r)) continue;
                if (!special.TryGetValue(I32(r, 4), out var list)) special[I32(r, 4)] = list = [];
                list.Add((I32(r, 8), (byte)I32(r, 12), (byte)I32(r, 16)));
            }
            var ids = new List<int>(map.Keys);
            foreach (var id in ids)
                if (special.TryGetValue(map[id].Link, out var list))
                {
                    list.Sort((x, y) => x.Pos.CompareTo(y.Pos));
                    var holes = new (byte, byte)[list.Count];
                    for (int i = 0; i < holes.Length; i++) holes[i] = (list[i].Course, list[i].Hole);
                    map[id] = map[id] with { SpecialHoles = holes };
                }
            return grandPrix = map;
        }
    }

    /// <summary>GrandPrixRankReward.iff (0x4C): u32 ativo, u32 Link, u32 posição, u32[5] itens, u32[5] qtd, u32[5] dias, u32 troféu.</summary>
    public IReadOnlyList<Domain.Rooms.GrandPrixRankReward> GrandPrixRankRewards(int link)
    {
        if (gpRewards == null)
        {
            var map = new Dictionary<int, List<Domain.Rooms.GrandPrixRankReward>>();
            foreach (var r in Rows("GrandPrixRankReward.iff"))
            {
                if (r.Length < 0x4C || !Active(r)) continue;
                var rw = new Domain.Rooms.GrandPrixRankReward(I32(r, 4), I32(r, 8), Items(r, 12, 32, 52), I32(r, 72));
                if (!map.TryGetValue(rw.Link, out var list)) map[rw.Link] = list = [];
                list.Add(rw);
            }
            foreach (var list in map.Values) list.Sort((a, b) => a.Rank.CompareTo(b.Rank));
            gpRewards = map;
        }
        return gpRewards.TryGetValue(link, out var l) ? l : [];
    }

    /// <summary>GrandPrixAIOptionalData.sff (0x25C): u32 ativo, i32 id, ..., +52 u32 classe.</summary>
    public IReadOnlyList<int> GrandPrixBots(int botClass)
    {
        if (gpBots == null)
        {
            var map = new Dictionary<int, List<int>>();
            foreach (var r in Rows("GrandPrixAIOptionalData.sff"))
            {
                if (r.Length < 0x38 || !Active(r)) continue;
                int c = I32(r, 52);
                if (!map.TryGetValue(c, out var list)) map[c] = list = [];
                list.Add(I32(r, 4));
            }
            gpBots = map;
        }
        return gpBots.TryGetValue(botClass, out var l) ? l : [];
    }

    // ------------------------------------------------------------------ oficina de club set (SPEC-jp983-oficina.md §4)

    Dictionary<int, Domain.Players.ClubWorkshopInfo>? workshop;
    Dictionary<int, int[]>? wsWeights, wsCosts;
    Dictionary<(int, int), int[]>? wsLimits;
    Dictionary<(int, int), int>? wsFamily;

    static int[] U32s(ReadOnlyMemory<byte> r, int o, int n)
    {
        var a = new int[n];
        for (int i = 0; i < n; i++) a[i] = I32(r, o + i * 4);
        return a;
    }

    /// <summary>ClubSet.iff: Attr i16[5] 0xD0, Slot i16[5] 0xDA e o bloco work_shop 0xE4..0x103.</summary>
    public IReadOnlyDictionary<int, Domain.Players.ClubWorkshopInfo> ClubWorkshop
    {
        get
        {
            if (workshop != null) return workshop;
            var map = new Dictionary<int, Domain.Players.ClubWorkshopInfo>();
            var family = new Dictionary<(int, int), int>();
            foreach (var r in Rows("ClubSet.iff"))
            {
                if (r.Length < 0x104) continue;
                var info = new Domain.Players.ClubWorkshopInfo(I32(r, OTypeId), I32(r, 0xE4), I32(r, 0xE8), I32(r, 0xEC), F32(r, 0xF0),
                    I32(r, 0xF4), I32(r, 0xF8) == 1, I32(r, 0x100), I16s(r, 0xD0, 5), I16s(r, 0xDA, 5));
                map[info.TypeId] = info;
                int rank = Domain.Players.ClubWorkshop.RankOf(Domain.Players.ClubWorkshop.Sum(info, new int[5]));
                if (info.Family != 0 && rank >= 0) family.TryAdd((info.Family, rank), info.TypeId);
            }
            wsFamily = family;
            return workshop = map;
        }
    }

    /// <summary>ClubSetWorkShopLevelUpProb.iff (24): u32 tipo, u32[5] pesos.</summary>
    public int[]? LevelUpWeights(int type)
    {
        if (wsWeights == null)
        {
            var m = new Dictionary<int, int[]>();
            foreach (var r in Rows("ClubSetWorkShopLevelUpProb.iff")) if (r.Length >= 0x18) m[I32(r, 0)] = U32s(r, 4, 5);
            wsWeights = m;
        }
        return wsWeights.GetValueOrDefault(type);
    }

    /// <summary>ClubSetWorkShopLevelUpLimit.iff (20): u32 tipo, u32 rank, u16[5] limites.</summary>
    public int[]? LevelUpLimit(int type, int rank)
    {
        if (wsLimits == null)
        {
            var m = new Dictionary<(int, int), int[]>();
            foreach (var r in Rows("ClubSetWorkShopLevelUpLimit.iff")) if (r.Length >= 0x12) m[(I32(r, 0), I32(r, 4))] = I16s(r, 8, 5);
            wsLimits = m;
        }
        return wsLimits.GetValueOrDefault((type, rank));
    }

    /// <summary>ClubSetWorkShopRankUpExp.iff (28): u32 linha, u32[6] maestria por rank.</summary>
    public int[]? RankUpCost(int row)
    {
        if (wsCosts == null)
        {
            var m = new Dictionary<int, int[]>();
            foreach (var r in Rows("ClubSetWorkShopRankUpExp.iff")) if (r.Length >= 0x1C) m[I32(r, 0)] = U32s(r, 4, 6);
            wsCosts = m;
        }
        return wsCosts.GetValueOrDefault(row);
    }

    public int SpecialAt(int family, int rank)
    {
        _ = ClubWorkshop;
        return wsFamily!.GetValueOrDefault((family, rank));
    }

    // ------------------------------------------------------------------ personagem: maestria e slot 7 (SPEC-jp983-cards.md §5)

    Dictionary<int, List<Domain.Players.MasteryStep>>? mastery;
    HashSet<int>? caddieSlotParts;

    /// <summary>CharacterMastery.iff (60): u32 ativo, u32 personagem, u32 seq, u32 stat (1..5), u32 nível, u32[5] condição, u32[5] qtd.</summary>
    public IReadOnlyList<Domain.Players.MasteryStep> MasterySteps(int characterTypeId)
    {
        if (mastery == null)
        {
            var m = new Dictionary<int, List<Domain.Players.MasteryStep>>();
            foreach (var r in Rows("CharacterMastery.iff"))
            {
                if (r.Length < 0x3C || !Active(r)) continue;
                var cond = new (int, int)[5];
                for (int i = 0; i < 5; i++) cond[i] = (I32(r, 0x14 + i * 4), I32(r, 0x28 + i * 4));
                int ch = I32(r, 4);
                if (!m.TryGetValue(ch, out var list)) m[ch] = list = [];
                list.Add(new Domain.Players.MasteryStep(I32(r, 8), I32(r, 0x0C) - 1, I32(r, 0x10), cond));
            }
            foreach (var list in m.Values) list.Sort((x, y) => x.Seq.CompareTo(y.Seq));
            mastery = m;
        }
        return mastery.TryGetValue(characterTypeId, out var l) ? l : [];
    }

    /// <summary>Part.iff CaddieSlot (u16 0x22A) ≠ 0.</summary>
    public bool OpensCaddieSlot(int partTypeId)
    {
        if (caddieSlotParts == null)
        {
            var set = new HashSet<int>();
            foreach (var r in Rows("Part.iff")) if (r.Length > 0x22C && I16(r, 0x22A) != 0) set.Add(I32(r, OTypeId));
            caddieSlotParts = set;
        }
        return caddieSlotParts.Contains(partTypeId);
    }

    // ------------------------------------------------------------------ missão diária (SPEC-jp983-missoes.md §1.1)

    Dictionary<int, Domain.Achievements.DailyQuestDef>? dailyQuests;

    /// <summary>QuestItem.iff (0x110): +0xC4 u32 dificuldade, +0xCC u32[10] QuestStuff, +0xF4 u32[2] prêmio, +0xFC u32[2] qtd.</summary>
    public IReadOnlyDictionary<int, Domain.Achievements.DailyQuestDef> DailyQuests
    {
        get
        {
            if (dailyQuests != null) return dailyQuests;
            var m = new Dictionary<int, Domain.Achievements.DailyQuestDef>();
            foreach (var r in Rows("QuestItem.iff"))
            {
                if (r.Length < 0x10C || !Active(r)) continue;
                int stuff = 0;
                for (int i = 0; i < 10 && stuff == 0; i++) stuff = I32(r, 0xCC + i * 4);
                var rewards = new List<(int, int)>(2);
                for (int i = 0; i < 2; i++) if (I32(r, 0xF4 + i * 4) is var t && t != 0) rewards.Add((t, I32(r, 0xFC + i * 4)));
                int tid = I32(r, OTypeId);
                m[tid] = new Domain.Achievements.DailyQuestDef(tid, I32(r, 0xC4), stuff, [.. rewards]);
            }
            return dailyQuests = m;
        }
    }

    // ------------------------------------------------------------------ Point Shop e Tiki Shop (SPEC-jp983-missoes.md §2.1)

    Dictionary<int, Domain.Shop.TikiInfo>? tiki;
    Dictionary<int, (int, int)>? pointShop;

    /// <summary>Bloco Tiki do cabeçalho comum (0x84): u32 por lote, u32 TP, u16 milhagem, u16 prob, i16 bônus mín/máx, u32 tipo, u32 pang.</summary>
    public Domain.Shop.TikiInfo? Tiki(int typeId)
    {
        if (tiki == null)
        {
            var m = new Dictionary<int, Domain.Shop.TikiInfo>();
            foreach (var t in ItemTables)
                foreach (var r in Rows(t))
                {
                    if (r.Length < 0x9C || I32(r, 0x84 + 16) == 0) continue;
                    var span = r.Span;
                    int tid = I32(r, OTypeId);
                    m[tid] = new Domain.Shop.TikiInfo(tid, I32(r, 0x84), I32(r, 0x88), (ushort)I16(r, 0x8C), (ushort)I16(r, 0x8E),
                        I16(r, 0x90), I16(r, 0x92), I32(r, 0x94), I32(r, 0x98));
                }
            tiki = m;
        }
        return tiki.GetValueOrDefault(typeId);
    }

    /// <summary>PointShop.iff (0x14): u32 ativo, u32 typeid, u32 TP, u32 qtd, u32 flag.</summary>
    public IReadOnlyDictionary<int, (int Tp, int Quantity)> PointShop
    {
        get
        {
            if (pointShop != null) return pointShop;
            var m = new Dictionary<int, (int, int)>();
            foreach (var r in Rows("PointShop.iff")) if (r.Length >= 0x14 && Active(r)) m[I32(r, 4)] = (I32(r, 8), I32(r, 12));
            return pointShop = m;
        }
    }

    // ------------------------------------------------------------------ buffs por tempo (SPEC-jp983-efeitos.md §5.1)

    Dictionary<int, Domain.Players.BuffInfo>? buffs;

    /// <summary>
    /// TimeLimitItem.iff (100): u32 ativo, u32 typeid, char[40] nome, char[40] ícone, u32 tipo (1 Yam: EXP; 2/3: EXP e pang;
    /// 4: EXP; 5: pang), u32 % (0 = 100), u32 minutos.
    /// </summary>
    public Domain.Players.BuffInfo? Buff(int typeId)
    {
        if (buffs == null)
        {
            var m = new Dictionary<int, Domain.Players.BuffInfo>();
            foreach (var r in Rows("TimeLimitItem.iff"))
            {
                if (r.Length < 100 || !Active(r)) continue;
                int kind = I32(r, 88), pct = I32(r, 92) is var v && v > 0 ? v : 100;
                int exp = kind is 1 or 2 or 3 or 4 ? pct : 0, pang = kind is 2 or 3 or 5 ? pct : 0;
                m[I32(r, 4)] = new Domain.Players.BuffInfo(I32(r, 4), kind, exp, pang, I32(r, 96));
            }
            buffs = m;
        }
        return buffs.GetValueOrDefault(typeId);
    }

    // ------------------------------------------------------------------ conquistas (SPEC-jp983-conquistas.md §1)

    List<Domain.Achievements.AchievementDef>? achievements;
    Dictionary<int, Domain.Achievements.QuestDef>? achievementQuests;
    HashSet<int>? counters;

    /// <summary>Achievement.iff (0x600): +0xC0 u32 etapa base, +0xC4 u32 categoria, +0x5D4 u32[10] etapas (ordem de nível).</summary>
    public IReadOnlyList<Domain.Achievements.AchievementDef> Achievements
    {
        get
        {
            if (achievements != null) return achievements;
            var list = new List<Domain.Achievements.AchievementDef>();
            var rows = Rows("Achievement.iff");
            for (int i = 0; i < rows.Count; i++)
            {
                var r = rows[i];
                if (r.Length < 0x600 || !Active(r)) continue;
                var quests = new List<int>(10);
                for (int j = 0; j < 10; j++) if (I32(r, 0x5D4 + j * 4) is var q && q != 0) quests.Add(q);
                if (quests.Count > 0) list.Add(new Domain.Achievements.AchievementDef(I32(r, OTypeId), i, I32(r, 0xC4), I32(r, 0xC0), [.. quests]));
            }
            return achievements = list;
        }
    }

    /// <summary>QuestStuff.iff (0x10C): +0xC0 u32[5] contadores, +0xD4 i32[5] quantidades, +0xE8 u32[3] prêmios, +0xF4 u32[3] qtd.</summary>
    public IReadOnlyDictionary<int, Domain.Achievements.QuestDef> AchievementQuests
    {
        get
        {
            if (achievementQuests != null) return achievementQuests;
            var map = new Dictionary<int, Domain.Achievements.QuestDef>();
            foreach (var r in Rows("QuestStuff.iff"))
            {
                if (r.Length < 0x10C) continue;
                var needs = new List<(int, int)>(5);
                for (int j = 0; j < 5; j++) if (I32(r, 0xC0 + j * 4) is var c && c != 0) needs.Add((c, I32(r, 0xD4 + j * 4)));
                var rewards = new List<(int, int)>(3);
                for (int j = 0; j < 3; j++) if (I32(r, 0xE8 + j * 4) is var t && t != 0) rewards.Add((t, I32(r, 0xF4 + j * 4)));
                map[I32(r, OTypeId)] = new Domain.Achievements.QuestDef(I32(r, OTypeId), [.. needs], [.. rewards]);
            }
            return achievementQuests = map;
        }
    }

    /// <summary>Contadores que existem (CounterItem.iff).</summary>
    public bool IsCounter(int typeId)
    {
        if (counters == null)
        {
            var set = new HashSet<int>();
            foreach (var id in archive.TypeIds("CounterItem.iff")) set.Add(id);
            counters = set;
        }
        return counters.Contains(typeId);
    }

    // ------------------------------------------------------------------ Memorial Shop (SPEC-jp983-papel-memorial.md §3.2)

    Dictionary<int, Domain.Shop.MemorialCoin>? memorialCoins;
    List<Domain.Shop.MemorialRare>? memorialRares;

    /// <summary>MemorialShopCoinItem.sff (0x40): u32 ativo, u32 typeid, u32 tipo, u32 bônus, u32 gacha mín/máx, i32 filtros[10].</summary>
    public IReadOnlyDictionary<int, Domain.Shop.MemorialCoin> MemorialCoins
    {
        get
        {
            if (memorialCoins != null) return memorialCoins;
            var coins = new Dictionary<int, Domain.Shop.MemorialCoin>();
            foreach (var r in Rows("MemorialShopCoinItem.sff"))
            {
                if (r.Length < 0x40 || U32(r, 0) == 0) continue;
                int tid = I32(r, 4);
                coins[tid] = new Domain.Shop.MemorialCoin(tid, I32(r, 8), I32(r, 0xC), I32(r, 0x10), I32(r, 0x14), Filters(r, 0x18));
            }
            return memorialCoins = coins;
        }
    }

    /// <summary>MemorialShopRareItem.iff (0x5C): i32 ativo, u32 gacha, u32 nº de gachas, u32 typeid, u32 peso, i32 tipo, i32 filtros[10].</summary>
    public IReadOnlyList<Domain.Shop.MemorialRare> MemorialRares
    {
        get
        {
            if (memorialRares != null) return memorialRares;
            var rares = new List<Domain.Shop.MemorialRare>();
            foreach (var r in Rows("MemorialShopRareItem.iff"))
            {
                if (r.Length < 0x40 || I32(r, 0) == 0) continue;                  // -1 (um registro) conta como ativo, como na referência
                int tid = I32(r, 0xC);
                if (typeIds.Contains(tid)) rares.Add(new Domain.Shop.MemorialRare(tid, I32(r, 0x10), I32(r, 0x14), I32(r, 4), Filters(r, 0x18)));
            }
            return memorialRares = rares;
        }
    }

    static int[] Filters(ReadOnlyMemory<byte> r, int o)
    {
        var list = new List<int>(10);
        for (int i = 0; i < 10; i++)
            if (I32(r, o + i * 4) is var f && f != 0) list.Add(f);
        return [.. list];
    }

    static int I32(ReadOnlyMemory<byte> r, int o) => BinaryPrimitives.ReadInt32LittleEndian(r.Span[o..]);
    static uint U32(ReadOnlyMemory<byte> r, int o) => BinaryPrimitives.ReadUInt32LittleEndian(r.Span[o..]);
    static short I16(ReadOnlyMemory<byte> r, int o) => BinaryPrimitives.ReadInt16LittleEndian(r.Span[o..]);
    static float F32(ReadOnlyMemory<byte> r, int o) => BinaryPrimitives.ReadSingleLittleEndian(r.Span[o..]);
    static bool Active(ReadOnlyMemory<byte> r) => r.Span[0] != 0;
    static int[] I16s(ReadOnlyMemory<byte> r, int o, int n)
    {
        var a = new int[n];
        for (int i = 0; i < n; i++) a[i] = I16(r, o + i * 2);
        return a;
    }
    static int[] U8s(ReadOnlyMemory<byte> r, int o, int n)
    {
        var a = new int[n];
        for (int i = 0; i < n; i++) a[i] = r.Span[o + i];
        return a;
    }

    public bool Exists(int typeId) => typeIds.Contains(typeId);

    public UccPartInfo? UccPart(int typeId) => ucc.GetValueOrDefault(typeId);

    readonly Dictionary<int, int[]> subParts = [];
    public int[] SubParts(int partTypeId) => subParts.TryGetValue(partTypeId, out var s) ? s : [];

    /// <summary>Self Design: textura base e a chave Data + Tex[0..2] + OrgTex[0..2] (igual ao KR, offsets do JP).</summary>
    static UccPartInfo UccInfo(ReadOnlyMemory<byte> r, int category)
    {
        var s = r.Span;
        var key = new System.Text.StringBuilder(Cp949.Read(s.Slice(0xC0, 40)));
        string tex0 = "";
        for (int i = 0; i < 3; i++)
        {
            string t = Cp949.Read(s.Slice(0xF4 + i * 40, 40));
            if (i == 0) tex0 = t;
            key.Append('|').Append(t).Append('|').Append(Cp949.Read(s.Slice(0x16C + i * 40, 40)));
        }
        return new UccPartInfo(category, tex0, key.ToString().ToLowerInvariant());
    }

    /// <summary>Peça padrão: 0x08000400 | índice&lt;&lt;18 | slot&lt;&lt;13, se existir no Part.iff (igual ao KR).</summary>
    public bool IsDefaultPart(int characterTypeId, int partTypeId) =>
        parts.Contains(partTypeId) && ((partTypeId >> 18) & 0xFF) == (characterTypeId & 0xFF)
        && (partTypeId & 0x1FF) == 0 && (partTypeId & 0xC00) == 0x400;

    public int[] DefaultParts(int characterTypeId)
    {
        int idx = characterTypeId & 0xFF;
        var result = new int[24];
        for (int slot = 0; slot < 24; slot++)
        {
            int t = 0x08000400 | (idx << 18) | (slot << 13);
            result[slot] = parts.Contains(t) ? t : 0;
        }
        return result;
    }

    /// <summary>Ordem do KR (mapselectdlg); os mapas que só o JP tem entram depois, em ordem.</summary>
    static readonly byte[] MapOrder = [0x13, 0x10, 0x0F, 0x0E, 0x0D, 0x0B, 0x08, 0x0A, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09];
    List<byte>? courses;

    /// <summary>Mapas ativos no Course.iff (typeid 0x28000000 | mapa; o Grand Zodiac 0x40 fica de fora).</summary>
    public IReadOnlyList<byte> Courses
    {
        get
        {
            if (courses != null) return courses;
            var active = new HashSet<int>();
            foreach (var r in Rows("Course.iff"))
                if (Active(r) && (I32(r, OTypeId) & 0xFF) < 0x40) active.Add(I32(r, OTypeId) & 0xFF);
            var list = new List<byte>();
            foreach (var m in MapOrder)
                if (active.Remove(m)) list.Add(m);
            var rest = new List<int>(active);
            rest.Sort();
            foreach (var m in rest) list.Add((byte)m);
            return courses = list;
        }
    }

    /// <summary>Estrelas: nibble baixo do byte 0x110 do Course.iff.</summary>
    public float CourseStars(int course)
    {
        foreach (var r in Rows("Course.iff"))
            if (I32(r, OTypeId) == (0x28000000 | (course & 0xFF))) return Math.Max(r.Span[0x110] & 0x0F, 1);
        return 1;
    }

    Dictionary<int, ReadOnlyMemory<byte>>? charRows, partRows, clubRows, caddieRows, auxRows;

    static Dictionary<int, ReadOnlyMemory<byte>> ByTypeId(List<ReadOnlyMemory<byte>> rows)
    {
        var d = new Dictionary<int, ReadOnlyMemory<byte>>(rows.Count);
        foreach (var r in rows) d[I32(r, OTypeId)] = r;
        return d;
    }

    /// <summary>
    /// Stats como no KR (Kr645GameData.PlayStats), com os offsets do JP: personagem Attr i16[5] 0x160 e PCL u8[5] 0x174;
    /// peça Attr 0x1E4 e Slot 0x1EE; anel Attr u8[5] 0xCA, Slot 0xCF, DriveUp 0xD4; club set Attr 0xD0; caddie nível 0x48 e
    /// Attr 0xEC.
    /// </summary>
    public (int[] Stats, int DriveUp) PlayStats(Player p)
    {
        if (charRows == null)
        {
            partRows = ByTypeId(Rows("Part.iff"));
            clubRows = ByTypeId(Rows("ClubSet.iff"));
            caddieRows = ByTypeId(Rows("Caddie.iff"));
            auxRows = ByTypeId(Rows("AuxPart.iff"));
            charRows = ByTypeId(Rows("Character.iff"));
        }
        var stats = new int[5];
        int driveUp = 0;
        var cap = new int[5];
        var lvl = new int[5];
        if (p.Character is { } ch && charRows.TryGetValue(ch.TypeId, out var ci))
        {
            var upg = ch.IntArray("pcl", 5);
            for (int t = 0; t < 5; t++) { cap[t] = I16(ci, 0x160 + t * 2); lvl[t] = ci.Span[0x174 + t] + upg[t]; }
            if (p.Level >= 6) cap[0] += (p.Level - 1) / 5;
            foreach (var tid in ch.IntArray("parts", 24))
                if (tid != 0 && partRows!.TryGetValue(tid, out var part))
                    for (int t = 0; t < 5; t++)
                    {
                        int attr = I16(part, 0x1E4 + t * 2);
                        cap[t] += I16(part, 0x1EE + t * 2) + attr;
                        lvl[t] += attr;
                    }
            foreach (var tid in ch.IntArray("aux", 5))
                if (tid != 0 && auxRows!.TryGetValue(tid, out var aux))
                {
                    for (int t = 0; t < 5; t++) { cap[t] += aux.Span[0xCA + t] + aux.Span[0xCF + t]; lvl[t] += aux.Span[0xCA + t]; }
                    driveUp += (sbyte)aux.Span[0xD4];
                }
            for (int t = 0; t < 5; t++) stats[t] = Math.Min(lvl[t], cap[t]);
        }
        if (p.Find(p.Equip.ClubSetId) is { } club && clubRows!.TryGetValue(club.TypeId, out var cset))
        {
            var upg = club.IntArray("pcl", 5);
            for (int t = 0; t < 5; t++) stats[t] += I16(cset, 0xD0 + t * 2) + upg[t];
        }
        if (p.Find(p.Equip.CaddieId) is { } cad && caddieRows!.TryGetValue(cad.TypeId, out var cdi) && (cdi.Span[OLevel] & 0x7F) <= p.Level
            && !CaddieRent.IsExpired(cad, DateTime.UtcNow))
            for (int t = 0; t < 5; t++) stats[t] += I16(cdi, 0xEC + t * 2);
        stats[0] = Math.Clamp(stats[0], 0, 100) - 15;
        int penalty = Math.Max(0, stats[0] - (PowerPenalty(p.Level) + 5));
        stats[1] = Math.Clamp(stats[1] - penalty, 0, 30);
        stats[2] = Math.Clamp(stats[2] - penalty, 0, 30);
        stats[3] = Math.Clamp(stats[3], 1, 30);
        stats[4] = Math.Clamp(stats[4], 1, 30);
        return (stats, driveUp);
    }

    static int PowerPenalty(int level) => level <= 5 ? 0 : Math.Min(level / 5, 13);

    Dictionary<int, int>? caddieFees;

    /// <summary>Caddie.iff MonthlyFee u32 0xC0 (logo depois do comum, como no KR).</summary>
    public int CaddieMonthlyFee(int typeId)
    {
        if (caddieFees == null)
        {
            var f = new Dictionary<int, int>();
            foreach (var r in Rows("Caddie.iff")) if (I32(r, 0xC0) > 0) f[I32(r, OTypeId)] = I32(r, 0xC0);
            caddieFees = f;
        }
        return caddieFees.TryGetValue(typeId, out var fee) ? fee : 0;
    }

    Dictionary<int, int>? salable;

    /// <summary>IsSalable (bits 1-4 do byte 0x80) 1 ou 3, como no KR.</summary>
    public bool CanTrade(int typeId)
    {
        if (salable == null)
        {
            var d = new Dictionary<int, int>();
            foreach (var t in new[] { "Part.iff", "ClubSet.iff", "Ball.iff", "Item.iff" })
                foreach (var r in Rows(t)) d[I32(r, OTypeId)] = (r.Span[OShop] >> 1) & 15;
            salable = d;
        }
        return salable.TryGetValue(typeId, out var s) && s is 1 or 3;
    }

    HashSet<int>? unlimitedBalls;

    /// <summary>COM[0] = 0 no Ball.iff (0x324), o IsUnlimitAztec do cliente (itemmanager.cpp:6371).</summary>
    public bool IsUnlimitedBall(int typeId)
    {
        if (unlimitedBalls == null)
        {
            var set = new HashSet<int>();
            foreach (var r in Rows("Ball.iff")) if (I16(r, 0x324) == 0) set.Add(I32(r, OTypeId));
            unlimitedBalls = set;
        }
        return unlimitedBalls.Contains(typeId);
    }

    Dictionary<int, (int Ability, float Rate)>? spItems;

    /// <summary>SpecialPrizeItem.iff (igual ao KR: u32 typeid, u32 efeito, f32 multiplicador).</summary>
    public IReadOnlyDictionary<int, (int Ability, float Rate)> SpItems
    {
        get
        {
            if (spItems != null) return spItems;
            var d = new Dictionary<int, (int, float)>();
            foreach (var r in Rows("SpecialPrizeItem.iff"))
                if (U32(r, 4) <= 4) d[I32(r, 0)] = (I32(r, 4), F32(r, 8));
            return spItems = d;
        }
    }

    Dictionary<int, CardInfo>? cards;

    /// <summary>Card.iff: raridade 0xC0, efeito u16 0xF4, valor u16 0xF6, minutos u16 0x170, volume u16 0x172.</summary>
    public IReadOnlyDictionary<int, CardInfo> Cards
    {
        get
        {
            if (cards != null) return cards;
            var d = new Dictionary<int, CardInfo>();
            foreach (var r in Rows("Card.iff"))
            {
                int tid = I32(r, OTypeId);
                d[tid] = new CardInfo(tid, Active(r), r.Span[0xC0], (ushort)I16(r, 0xF4), (ushort)I16(r, 0xF6), (ushort)I16(r, 0x170),
                    (ushort)I16(r, 0x172), I16s(r, 0xEA, 5));
            }
            return cards = d;
        }
    }

    List<MagicBoxRecipe>? magicBox;
    Dictionary<int, MagicBoxOutput[]>? magicBoxRandom;

    /// <summary>
    /// CadieMagicBox.iff do JP (0x8C: o KR tem 0x68): número 0x00, nível 0x10, saída 0x18 e quantidade 0x1C, materiais
    /// u32[4] 0x20 e quantidades u32[4] 0x30, sorteio 0x40. Na posição número-1, como no KR.
    /// </summary>
    public IReadOnlyList<MagicBoxRecipe> MagicBox
    {
        get
        {
            if (magicBox != null) return magicBox;
            var byNumber = new Dictionary<int, MagicBoxRecipe>();
            int max = 0;
            foreach (var r in Rows("CadieMagicBox.iff"))
            {
                int number = I32(r, 0);
                max = Math.Max(max, number);
                int n = 0;
                while (n < 4 && U32(r, 0x20 + n * 4) != 0) n++;
                var elems = new int[n];
                var counts = new int[n];
                for (int i = 0; i < n; i++) { elems[i] = I32(r, 0x20 + i * 4); counts[i] = Math.Max(I32(r, 0x30 + i * 4), 1); }
                byNumber[number] = new MagicBoxRecipe(number - 1, I32(r, 0x10), I32(r, 0x18), Math.Max(I32(r, 0x1C), 1), elems, counts, I32(r, 0x40));
            }
            var list = new List<MagicBoxRecipe>(max);
            for (int i = 1; i <= max; i++)
                list.Add(byNumber.GetValueOrDefault(i) ?? new MagicBoxRecipe(i - 1, int.MaxValue, 0, 1, [], [], 0));
            return magicBox = list;
        }
    }

    /// <summary>CadieMagicBoxRandom.iff (igual ao KR: u32 grupo, u32 typeid, u32 quantidade, u32 peso).</summary>
    public IReadOnlyDictionary<int, MagicBoxOutput[]> MagicBoxRandom
    {
        get
        {
            if (magicBoxRandom != null) return magicBoxRandom;
            var groups = new Dictionary<int, List<MagicBoxOutput>>();
            foreach (var r in Rows("CadieMagicBoxRandom.iff"))
            {
                if (!groups.TryGetValue(I32(r, 0), out var g)) groups[I32(r, 0)] = g = [];
                g.Add(new MagicBoxOutput(I32(r, 4), I32(r, 8), I32(r, 12)));
            }
            var d = new Dictionary<int, MagicBoxOutput[]>(groups.Count);
            foreach (var (k, v) in groups) d[k] = v.ToArray();
            return magicBoxRandom = d;
        }
    }

    Dictionary<int, int>? enchant;

    /// <summary>Enchant.iff (igual ao KR): chave 0x34000000 | atributo&lt;&lt;20 | nível -> pang (u32 em 0x08).</summary>
    public int? UpgradePrice(int stat, int current)
    {
        if (enchant == null)
        {
            enchant = [];
            foreach (var r in Rows("Enchant.iff")) enchant[I32(r, OTypeId)] = I32(r, 0x08);
        }
        return enchant.TryGetValue(0x34000000 | (stat << 20) | current, out var p) ? p : null;
    }

    Dictionary<int, ShopItem>? shop;

    public ShopItem? GetShopItem(int typeId) => (shop ??= BuildShop()).GetValueOrDefault(typeId);

    /// <summary>
    /// Catálogo: preço 0x74, desconto 0x78, byte 0x80 = bit0 cookie, bits 5-7 InStock (o mesmo campo de bits do KR) e os
    /// extras de cada tabela nos offsets do JP.
    /// </summary>
    Dictionary<int, ShopItem> BuildShop()
    {
        var d = new Dictionary<int, ShopItem>();
        static ShopItem Common(ReadOnlyMemory<byte> r, int packSize = 0, int[]? periodPrices = null, (int, int)[]? set = null,
            byte hairColor = 0, byte hairChar = 0, float[]? position = null, int[]? slots = null, bool unitSkin = false) => new()
        {
            TypeId = I32(r, OTypeId), Price = U32(r, OPrice), SalePrice = U32(r, OSalePrice), IsCash = (r.Span[OShop] & 1) != 0,
            InStock = r.Span[OShop] >> 5, PackSize = packSize, PeriodPrices = periodPrices ?? [], SetElements = set ?? [],
            HairColor = hairColor, HairCharacter = hairChar, Position = position ?? [], UpgradeSlots = slots ?? [], UnitPricedSkin = unitSkin,
        };
        void Add(ShopItem s) => d[s.TypeId] = s;
        foreach (var t in new[] { "Character.iff", "Part.iff", "Club.iff", "Ball.iff", "Caddie.iff", "AuxPart.iff", "Card.iff" })
            foreach (var r in Rows(t)) Add(Common(r));
        foreach (var r in Rows("ClubSet.iff"))
        {
            var slots = new int[5];
            for (int i = 0; i < 5; i++) slots[i] = Math.Max(0, I16(r, 0xDA + i * 2) - I16(r, 0xD0 + i * 2));
            Add(Common(r, slots: slots));
        }
        foreach (var r in Rows("Item.iff"))                                // premium: u16[5] = preço por 1/7/15/30/365 dias
            Add(Domain.Players.Premium.IsTicket(I32(r, OTypeId)) ? Common(r, periodPrices: I16s(r, 0xEC, 5)) : Common(r, packSize: I16(r, 0xEC)));
        foreach (var r in Rows("CaddieItem.iff"))                          // u16[4] + u32: preços por período (1, 7, 15, 30, 360 dias)
            Add(Common(r, periodPrices: [(ushort)I16(r, 0x110), (ushort)I16(r, 0x112), (ushort)I16(r, 0x114), (ushort)I16(r, 0x116), I32(r, 0x118)]));
        foreach (var r in Rows("Skin.iff"))
            Add(Common(r, periodPrices: I16s(r, 0xEA, 5), unitSkin: (I32(r, OTypeId) & 0x3C00000) == 0x1800000));
        foreach (var r in Rows("Mascot.iff")) Add(Common(r, periodPrices: U8s(r, 0x110, 5)));   // referência: Mascot.price[5]
        foreach (var r in Rows("SetItem.iff"))
        {
            var elems = new List<(int, int)>();
            int n = Math.Min(I32(r, 0xC0), 10);
            for (int i = 0; i < n; i++)
                if (I32(r, 0xC4 + i * 4) != 0) elems.Add((I32(r, 0xC4 + i * 4), Math.Max((int)(ushort)I16(r, 0xEC + i * 2), 1)));
            Add(Common(r, set: [.. elems]));
        }
        foreach (var r in Rows("HairStyle.iff")) Add(Common(r, hairColor: r.Span[0xC0], hairChar: r.Span[0xC1]));
        foreach (var r in Rows("Furniture.iff"))
            Add(Common(r, position: [F32(r, 0xF4), F32(r, 0xF8), F32(r, 0xFC), F32(r, 0x100)]));
        return d;
    }
}
