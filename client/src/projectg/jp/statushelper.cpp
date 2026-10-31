// JP 983: unidade 005e3430–005e49b0 (CStatusHelper). Classe nova do 983 (existe no 839); ver include/jp/statushelper.h.
// Sessão B125 (pedido A77). 005e3450–005e3b00 são moldes da STL e das lambdas (for_each, push_back do vector de ponteiros).
#include "minatl.h"
#include "wlocalize.h"
#include "projectg.h"
#include "shareddoc.h"
#include "cardsystem.h"
#include "wminmax.h"
#include "statushelper.h"
#include <algorithm>

int CalcPowerPenalty(int upgrade); // 009d9c50 (itemmanager.cpp)

// Métodos do CItemManager do 983 sem declaração pública na A (empréstimo, como CItemManagerStatJp da clubsetstat.h).
class CItemManagerStatusJp
{
public:
	// 00a189c0: o GetItemLevel do 645 (privado no itemmanager.h): soma Attr das peças e auxiliares
	unsigned char GetItemLevel983(unsigned long type, const unsigned long* tidParts, const unsigned long* tidAuxParts);
	// 009e9cc0: min(Attr do taco + subida, teto 009e9c80 com os slots ganhos)
	unsigned char GetClubSetStat983(unsigned long type, IFF_STRUCT::sClubSet* pClub, const short* attrUp,
		const short* slotUp);
	// 009d9ce0 / 009d9d10: Attr do caddie / mascote se o nível do item (c +0x48 & 0x7f) ≤ level
	unsigned char GetCaddieStat983(unsigned long type, IFF_STRUCT::sCaddie* pCaddie, unsigned char level);
	unsigned char GetMascotStat983(unsigned long type, IFF_STRUCT::sMascot* pMascot, unsigned char level);
	// 00a1fad0: slots do bônus de conjunto (SetRangeQuery983 + CalcSetEffect983 + 009d9a90)
	unsigned char GetSetEffectSlot983(unsigned long type, const unsigned long* tidParts, const unsigned long* tidAuxParts);
};

static inline CItemManagerStatusJp* ItemMgr983()
{
	return (CItemManagerStatusJp*)ItemManager();
}

// sItemInfo +0xaa: slots ganhos na oficina (5 short; o sItemInfo da A ainda não tem o campo, pedido B122.1)
static inline const short* ClubSlotUp983(const sItemInfo& info)
{
	return (const short*)((const char*)&info + 0xaa);
}

// 005e3430
CStatusHelper::CStatusHelper()
{
}

// 005e3440
CStatusHelper::~CStatusHelper()
{
}

// 00a189c0: soma Attr das pecas (24) e dos auxiliares (5, tipo < 5, 99 = sem valor); igual ao GetItemLevel do 645
unsigned char CItemManagerStatusJp::GetItemLevel983(unsigned long type, const unsigned long* tidParts,
	const unsigned long* tidAuxParts)
{
	if (tidParts == NULL)
		return 0;

	CItemManager* pMgr = (CItemManager*)this;
	unsigned char level = 0;
	for (int i = 0; i < 24; i++)
	{
		IFF_STRUCT::sPart* pPart = tidParts[i] ? pMgr->FindPart(tidParts[i]) : NULL;
		if (pPart)
			level += (unsigned char)pPart->Attr[type];
	}

	if (tidAuxParts && (int)type < 5)
	{
		for (int i = 0; i < 5; i++)
		{
			IFF_STRUCT::sAuxPart* pAux = tidAuxParts[i] ? pMgr->FindAuxPart(tidAuxParts[i]) : NULL;
			if (pAux && pAux->Attr[type] != 99)
				level += (unsigned char)pAux->Attr[type];
		}
	}
	return level;
}

// 009d9ce0: Attr do caddie se o nivel do item (c +0x48 & 0x7f) <= level
unsigned char CItemManagerStatusJp::GetCaddieStat983(unsigned long type, IFF_STRUCT::sCaddie* pCaddie,
	unsigned char level)
{
	if (pCaddie && (*((unsigned char*)pCaddie + 0x48) & 0x7f) <= level)
		return (unsigned char)pCaddie->Attr[type];
	return 0;
}

// 009d9d10: idem para o mascote (Attr em bytes, +0x115)
unsigned char CItemManagerStatusJp::GetMascotStat983(unsigned long type, IFF_STRUCT::sMascot* pMascot,
	unsigned char level)
{
	if (pMascot && (*((unsigned char*)pMascot + 0x48) & 0x7f) <= level)
		return pMascot->Attr[type];
	return 0;
}

// 009e9cc0 (+ teto 009e9c80): min(Attr+subida, Slot+slots ganhos+cartas)
unsigned char CItemManagerStatusJp::GetClubSetStat983(unsigned long type, IFF_STRUCT::sClubSet* pClub,
	const short* attrUp, const short* slotUp)
{
	unsigned char stat = 0;
	if (pClub)
		stat = (unsigned char)pClub->Attr[type];
	if (attrUp)
		stat += (unsigned char)attrUp[type];

	unsigned char cap = 0;
	if (pClub)
		cap = (unsigned char)pClub->Slot[type];
	if (slotUp)
		cap += (unsigned char)slotUp[type];
	cap += (unsigned char)CCardManager::Instance()->GetClubCardStatusSlot983(type);

	return stat > cap ? cap : stat;
}

// 00a1fad0: slot do bonus de conjunto (SetEffectTable): value40[type] + quantos effect[] valem 6
unsigned char CItemManagerStatusJp::GetSetEffectSlot983(unsigned long type, const unsigned long* tidParts,
	const unsigned long* tidAuxParts)
{
	if ((int)type >= 5 || tidParts == NULL || tidAuxParts == NULL)
		return 0;

	CItemManager* pMgr = (CItemManager*)this;
	pMgr->SetRangeQuery983(tidParts, tidAuxParts, 0, 0, 0);
	pMgr->CalcSetEffect983();

	IFF983::sSetEffectTable* pEffect = pMgr->m_pSetEffect983;
	if (pEffect == NULL)
		return 0;
	int count = 0;
	for (int i = 0; i < 3; i++)
	{
		if (pEffect->effect[i] == 6)
			count++;
	}
	return (unsigned char)(pEffect->value40[type] + count);
}

// 005e3850
int CStatusHelper::GetCharStat983(unsigned long type, unsigned long tidChar)
{
	if (type <= 4)
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(tidChar);
		if (pChar)
			return pChar->Attr[type];
	}
	return 0;
}

// 005e3890
int CStatusHelper::GetEquipStat983(unsigned long type, const unsigned long* tidParts, const unsigned long* tidAuxParts,
	unsigned long tidClub, unsigned long tidCaddie, unsigned long tidMascot)
{
	if (type > 4)
		return 0;

	int stat = 0;
	IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(tidCaddie);
	IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tidMascot);
	IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(tidClub);
	if (pClub)
		stat = ItemMgr983()->GetClubSetStat983(type, pClub, NULL, NULL);

	stat += ItemMgr983()->GetItemLevel983(type, tidParts, tidAuxParts);
	stat += ItemMgr983()->GetCaddieStat983(type, pCaddie, Doc()->m_myInfo.stat.Level);
	stat += ItemMgr983()->GetMascotStat983(type, pMascot, Doc()->m_myInfo.stat.Level);
	return stat;
}

// 005e3970
int CStatusHelper::GetPowerLimit983()
{
	return CalcPowerPenalty(Doc()->m_myInfo.stat.Level) + 20;
}

// 005e3990
int CStatusHelper::GetClubSlot983(unsigned long type, unsigned long tidClub)
{
	if (type > 4)
		return 0;
	if (tidClub == 0)
		return 0;

	IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(tidClub);
	if (pClub == NULL)
		return 0;
	return pClub->Slot[type];
}

// 005e39d0
int CStatusHelper::GetLevelSlot983(unsigned long type, int level)
{
	int slot = 0;
	if (type <= 4)
	{
		if (level == -1)
			level = Doc()->m_myInfo.stat.Level;
		if (type == 0 && level >= 6)
			slot = (level - 1) / 5;
	}
	return slot;
}

// 005e3c00
int CStatusHelper::GetCharPCL983(unsigned long type, unsigned long tidChar)
{
	if (type > 4)
		return 0;

	const std::map<unsigned int, sCharacterInfo>& charMap = Doc()->m_charMap;
	for (std::map<unsigned int, sCharacterInfo>::const_iterator it = charMap.begin(); it != charMap.end(); ++it)
	{
		if (it->second.tid == tidChar)
			return it->second.PCL[type];
	}
	return 0;
}

// 005e3cc0
int CStatusHelper::GetStatusOf983(unsigned long type, unsigned long tidChar, const unsigned long* tidParts,
	const unsigned long* tidAuxParts, unsigned long tidClub, unsigned long tidCaddie, unsigned long tidMascot)
{
	if (type > 4)
		return 0;

	int status = GetCharStat983(type, tidChar);
	status += GetEquipStat983(type, tidParts, tidAuxParts, tidClub, tidCaddie, tidMascot);
	status += GetCharPCL983(type, tidChar);
	status -= GetPenaltyOf983(type, tidChar, tidParts, tidAuxParts, tidClub, tidCaddie, tidMascot);
	return status;
}

// 005e3d40
int CStatusHelper::GetPenaltyOf983(unsigned long type, unsigned long tidChar, const unsigned long* tidParts,
	const unsigned long* tidAuxParts, unsigned long tidClub, unsigned long tidCaddie, unsigned long tidMascot)
{
	int penalty = 0;
	if (type <= 4 && (type == 1 || type == 2))
	{
		int power = GetStatusOf983(0, tidChar, tidParts, tidAuxParts, tidClub, tidCaddie, tidMascot);
		penalty = Max(0, power - GetPowerLimit983());
	}
	return penalty;
}

// 005e3f60
int CStatusHelper::GetMyCharStat983(unsigned long type)
{
	if (type > 4)
		return 0;

	int stat = 0;
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	if (pInfo)
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(pInfo->tid);
		if (pChar)
			stat = pChar->Attr[type];
	}
	return stat;
}

// 005e3fc0
int CStatusHelper::GetMyEquipStat983(unsigned long type)
{
	if (type > 4)
		return 0;

	int stat = 0;
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];

	IFF_STRUCT::sCaddie* pCaddie;
	std::map<unsigned int, sCaddieInfo>::iterator itCaddie = Doc()->m_caddieMap.find(Doc()->m_myInfo.userEquip.guidCaddie);
	if (itCaddie == Doc()->m_caddieMap.end())
		pCaddie = NULL;
	else
		pCaddie = ItemManager()->FindCaddie(itCaddie->second.tid);

	IFF_STRUCT::sMascot* pMascot;
	std::map<unsigned int, sMascotInfo>::iterator itMascot = Doc()->m_mascotMap.find(Doc()->m_myInfo.userEquip.guidMascot);
	if (itMascot == Doc()->m_mascotMap.end())
		pMascot = NULL;
	else
		pMascot = ItemManager()->FindMascot(itMascot->second.tid);

	std::map<unsigned int, sItemInfo>::iterator itClub = Doc()->m_clubSetMap.find(Doc()->m_myInfo.userEquip.guidClubSet);
	if (itClub != Doc()->m_clubSetMap.end())
	{
		IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(itClub->second.tid);
		stat = ItemMgr983()->GetClubSetStat983(type, pClub, itClub->second.Common, ClubSlotUp983(itClub->second));
	}

	stat += ItemMgr983()->GetItemLevel983(type, pInfo->tidParts, pInfo->tidAuxParts);
	stat += ItemMgr983()->GetCaddieStat983(type, pCaddie, Doc()->m_myInfo.stat.Level);
	stat += ItemMgr983()->GetMascotStat983(type, pMascot, Doc()->m_myInfo.stat.Level);
	stat += CCardManager::Instance()->GetNpcCardStatus983(type);
	stat += CCardManager::Instance()->GetCardPeriodStatus(type);
	return stat;
}

// 005e4180
int CStatusHelper::GetPartsSlot983(unsigned long type, const unsigned long* tidParts)
{
	if (type > 4)
		return 0;

	int slot = 0;
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	if (tidParts == NULL)
		tidParts = pInfo->tidParts;
	const unsigned long* tidAuxParts = pInfo->tidAuxParts;

	for (int i = 0; i < 24; i++)
	{
		if (tidParts[i])
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(tidParts[i]);
			if (pPart)
				slot += pPart->Slot[type];
		}
	}
	for (int i = 0; i < 5; i++)
	{
		if (tidAuxParts[i])
		{
			IFF_STRUCT::sAuxPart* pAux = ItemManager()->FindAuxPart(tidAuxParts[i]);
			if (pAux)
				slot += pAux->Slot[type];
		}
	}
	return ItemMgr983()->GetSetEffectSlot983(type, tidParts, tidAuxParts) + slot;
}

// 005e4260
int CStatusHelper::GetMyPartsSlot983(unsigned long type)
{
	if (type > 4)
		return 0;

	int slot = 0;
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	for (int i = 0; i < 24; i++)
	{
		if (pInfo->tidParts[i])
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pInfo->tidParts[i]);
			if (pPart)
				slot += pPart->Slot[type];
		}
	}
	for (int i = 0; i < 5; i++)
	{
		if (pInfo->tidAuxParts[i])
		{
			IFF_STRUCT::sAuxPart* pAux = ItemManager()->FindAuxPart(pInfo->tidAuxParts[i]);
			if (pAux)
				slot += pAux->Slot[type];
		}
	}
	return ItemMgr983()->GetSetEffectSlot983(type, pInfo->tidParts, pInfo->tidAuxParts) + slot;
}

// 005e4340
int CStatusHelper::GetMyCardSlot983(unsigned long type)
{
	if (type > 4)
		return 0;

	int slot = 0;
	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	for (int i = 0; i < 12; i++)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(pInfo->card[0][i]);
		if (pCard)
			slot += pCard->COM[type];
	}
	return slot;
}

// 005e43c0
void CStatusHelper::GetMasteryList983(MasteryList983& out, unsigned long tidChar, int level)
{
	out.clear();
	if (tidChar == 0)
	{
		sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		if (pInfo)
			tidChar = pInfo->tid;
	}

	MasteryList983 unused;
	const std::vector<sCharacterMastery983>& table =
		*(const std::vector<sCharacterMastery983>*)&Doc()->m_itemManager.m_CharacterMastery983;
	std::for_each(table.begin(), table.end(), [&](const sCharacterMastery983& rec)
	{
		if (tidChar == rec.tidChar && (level < 0 || rec.level <= level))
			out.push_back(&rec);
	});
}

// 005e44c0
int CStatusHelper::GetMasterySlot983(unsigned long type, sCharacterInfo* pChar)
{
	if (type > 4)
		return 0;

	int count = 0;
	if (pChar == NULL)
		pChar = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];

	MasteryList983 list;
	GetMasteryList983(list, pChar->tid, pChar->mastery);
	auto isType = [](unsigned long type, int statType) { return type == statType - 1; };
	if (!list.empty())
	{
		std::for_each(list.begin(), list.end(), [&](const sCharacterMastery983* p)
		{
			if (isType(type, p->statType))
				++count;
		});
	}
	return count;
}

// 005e45c0
int CStatusHelper::GetMasterySlotOf983(unsigned long type, unsigned long tidChar, int level)
{
	if (type > 4)
		return 0;

	int count = 0;
	MasteryList983 list;
	GetMasteryList983(list, tidChar, level);
	auto isType = [](unsigned long type, int statType) { return type == statType - 1; };
	if (!list.empty())
	{
		std::for_each(list.begin(), list.end(), [&](const sCharacterMastery983* p)
		{
			if (isType(type, p->statType))
				++count;
		});
	}
	return count;
}

// 005e4690
int CStatusHelper::GetMySlot983(unsigned long type)
{
	if (type > 4)
		return 0;

	int slot = GetMasterySlot983(type, NULL);
	slot += GetMyPartsSlot983(type);
	slot += GetMyCardSlot983(type);
	slot += GetLevelSlot983(type);
	return slot;
}

// 005e46e0
int CStatusHelper::GetSlotOf983(unsigned long type, const unsigned long* tidParts, unsigned long tidChar, int level)
{
	if (type > 4)
		return 0;

	int slot = GetMasterySlotOf983(type, tidChar, level);
	slot += GetPartsSlot983(type, tidParts);
	slot += GetMyCardSlot983(type);
	slot += GetLevelSlot983(type);
	return slot;
}

// 005e4740
int CStatusHelper::GetMyPCL983(unsigned long type, bool bRaw, sCharacterInfo* pChar)
{
	if (type > 4)
		return 0;

	int pcl = 0;
	if (pChar == NULL)
		pChar = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	if (pChar)
	{
		if (bRaw)
			pcl = pChar->PCL[type];
		else
			pcl = Min((int)pChar->PCL[type], GetSlotOf983(type, pChar->tidParts, pChar->tid, pChar->mastery));
	}
	return pcl;
}

// 005e47e0
int CStatusHelper::GetMyOverSlot983(unsigned long type)
{
	if (type > 4)
		return 0;

	sCharacterInfo* pInfo = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
	if (pInfo)
		return Max(0, pInfo->PCL[type] - GetMySlot983(type));
	return 0;
}

// 005e4850
int CStatusHelper::GetFreeSlotOf983(unsigned long type, const unsigned long* tidParts, unsigned long tidChar, int level)
{
	if (type > 4)
		return 0;

	int slot = GetSlotOf983(type, tidParts, tidChar, level);
	return Max(0, slot - GetMyPCL983(type, true, NULL));
}

// 005e48a0
int CStatusHelper::GetMyFreeSlot983(unsigned long type)
{
	if (type > 4)
		return 0;

	int pcl = GetMyPCL983(type, true, NULL);
	return Max(0, GetMySlot983(type) - pcl);
}

// 005e48e0
int CStatusHelper::GetStatus983(unsigned long type)
{
	if (type > 4)
		return 0;

	int status = GetMyCharStat983(type);
	status += GetMyEquipStat983(type);
	status += GetMyPCL983(type, false, NULL);
	status -= GetMyPenalty983(type);
	return status;
}

// 005e4930
int CStatusHelper::GetMyPenalty983(unsigned long type)
{
	int penalty = 0;
	if (type <= 4 && (type == 1 || type == 2))
		penalty = Max(0, GetStatus983(0) - GetPowerLimit983());
	return penalty;
}
