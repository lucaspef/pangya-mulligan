using Pangya.Core.Logging;
using Pangya.Core.Net;
using Pangya.Domain.Players;

namespace Pangya.Protocol.JP983.Game;

/// <summary>
/// Personagem no JP (docs/server/protocolo/SPEC-jp983-cards.md §3–§4, §2.6): cards nos 12 slots (0x18A/0x18B/0x18C), maestria
/// (0x187), stat ±1 pela tela da maestria (0x188/0x189) e o Lolo (0x155). Cada um manda o 0x216 e a sua resposta.
/// </summary>
public sealed partial class JpSystems
{
    public const ushort CMasteryExpand = 0x187, CStatUp = 0x188, CStatDown = 0x189, CCardEquip = 0x18A, CCardPatch = 0x18B,
        CCardRemove = 0x18C, CLolo = 0x155;
    const ushort SMasteryExpand = 0x26E, SStatUp = 0x26F, SStatDown = 0x270, SCardEquip = 0x271, SCardPatch = 0x272,
        SCardRemove = 0x273, SLoloRarity = 0x229, SLolo = 0x22A;

    CharacterSystems? characters;
    CharacterSystems Characters => characters ??= new CharacterSystems(ctx.Players.Store, ctx.Data, ctx.Data as IJpCharacterData);

    async Task<bool> HandleCharacterAsync(PacketReader r)
    {
        var p = kr.Player;
        CharResult res;
        ushort answer;
        switch (r.Id)
        {
            case CMasteryExpand:
                res = await Characters.ExpandMasteryAsync(p, (int)r.U32(), r.I32(), q => book?.Cleared.ContainsKey(q) == true);
                answer = SMasteryExpand;
                break;
            case CStatUp or CStatDown:
            {
                int stat = (int)r.U32();
                int tid = (int)r.U32(), id = r.I32();                                   // o CharacterInfo 513: só typeid e id
                res = await Characters.StatAsync(p, tid, id, stat, r.Id == CStatUp);
                answer = r.Id == CStatUp ? SStatUp : SStatDown;
                break;
            }
            case CCardEquip or CCardPatch:
                res = await Characters.EquipCardAsync(p, (int)r.U32(), r.I32(), (int)r.U32(), r.I32(), (int)r.U32(), r.Id == CCardPatch);
                answer = r.Id == CCardPatch ? SCardPatch : SCardEquip;
                break;
            case CCardRemove:
                res = await Characters.RemoveCardAsync(p, (int)r.U32(), r.I32(), (int)r.U32(), r.I32(), (int)r.U32());
                answer = SCardRemove;
                break;
            case CLolo:
            {
                long pang = (long)r.U64();
                int[] cards = [(int)r.U32(), (int)r.U32(), (int)r.U32()];
                res = await Characters.LoloAsync(p, pang, cards);
                answer = SLolo;
                break;
            }
            default: return false;
        }
        r.Skip(r.Remaining);
        if (res.Code != 0)
        {
            conn.SendTranslated(new PacketWriter(answer).U32(res.Code).U32(0));
            Log.Info($"{conn} JP: pedido do personagem 0x{r.Id:X} recusado (0x{res.Code:X})");
            return true;
        }
        if (res.PangSpent > 0) conn.SendTranslated(new PacketWriter(SPang).U64((ulong)p.Pang).U64((ulong)res.PangSpent));
        conn.SendTranslated(ItemsChanged(res.Entries));
        switch (r.Id)
        {
            case CMasteryExpand: conn.SendTranslated(new PacketWriter(answer).U32(0)); break;
            case CStatUp or CStatDown: conn.SendTranslated(new PacketWriter(answer).U32(0).U32((uint)res.Stat)); break;
            case CLolo:
                conn.SendTranslated(new PacketWriter(SLoloRarity).U32((uint)res.Rarity));
                conn.SendTranslated(new PacketWriter(answer).U32(0).U32((uint)res.TypeId));
                break;
            default: conn.SendTranslated(new PacketWriter(answer).U32(0).U32((uint)res.TypeId)); break;
        }
        Log.Info($"{conn} JP: pedido do personagem 0x{r.Id:X} ok ({res.TypeId:X8})");
        var bag = new Domain.Achievements.CounterBag();
        switch (r.Id)
        {
            case CStatUp: bag.Add(Domain.Achievements.AchievementCounters.CharacterStatUp); break;
            case CStatDown: bag.Add(Domain.Achievements.AchievementCounters.CharacterStatDown); break;
            case CMasteryExpand: bag.Add(Domain.Achievements.AchievementCounters.CharacterMastery); break;
            case CCardEquip or CCardPatch: bag.Add(Domain.Achievements.AchievementCounters.CharacterCardEquip); break;
            case CCardRemove: bag.Add(Domain.Achievements.AchievementCounters.CharacterCardRemove); break;
            case CLolo: bag.Add(Domain.Achievements.AchievementCounters.Lolo); break;
        }
        await RecordAsync(bag);
        return true;
    }

    /// <summary>
    /// 0x216 com entradas de qualquer tipo (SPEC-jp983-cards.md §3.4): 50 B (u8 tipo, u32 typeid, i32 id, u32 tempo, i32 antes,
    /// i32 depois, i32 diferença, i16[5] stats, str vazia, u32 0, u32 0, u32 typeid do card, u8 slot) + u32 maestria no 0xCD.
    /// </summary>
    public static PacketWriter ItemsChanged(List<ItemEntry> entries)
    {
        var w = new PacketWriter(SItemsChanged, 16 + entries.Count * 54)
            .U32((uint)DateTimeOffset.UtcNow.ToUnixTimeSeconds()).U32((uint)entries.Count);
        foreach (var e in entries)
        {
            w.U8(e.Kind).U32((uint)e.TypeId).I32(e.Id).U32(0).I32(e.Before).I32(e.After).I32(e.After - e.Before);
            for (int i = 0; i < 5; i++) w.I16((short)(e.Stats is { } s && i < s.Length ? s[i] : 0));
            w.U16(0).U32(0).U32(0).U32((uint)e.CardTypeId).U8(e.Slot);
            if (e.Kind == ItemEntry.KindMastery && Item.GroupOf(e.TypeId) == ItemGroup.Character) w.U32((uint)e.Mastery);
        }
        return w;
    }
}
