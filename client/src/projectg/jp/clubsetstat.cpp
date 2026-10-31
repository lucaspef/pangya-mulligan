// JP 983: unidade 0049c430–0049cb90 (atributos do taco: funções livres, __fastcall pelo /Gr). Sem classe nem RTTI, do
// 839/983, sem par no 645; ver include/jp/clubsetstat.h. Sessão B88 (F3.7.9, 13º lote). Fica entre a unidade da
// ResetProgress (…0049c430: 0049c120/0049c380) e a ClubSetWorkshopFx983 (0049cb90…, que começa pelos COMDATs dela).
#include "minatl.h"
#include "projectg.h"
#include "cardsystem.h"
#include "clubsetstat.h"

// 0049c430
unsigned short AddStat983(short a, short b, short c)
{
	return a + b + c;
}

// 0049c440
CCardManager* GetCardManager983()
{
	return CCardManager::Instance();
}

// 0049c450
short MinStat983(short a, short b)
{
	return a > b ? b : a;
}

// 0049c460
unsigned short AddStatMax983(short a, short b, short max)
{
	short v = a + b;
	return v > max ? max : v;
}

// 0049c480
short GetCardSlot983(int type)
{
	return CCardManager::Instance()->GetClubCardStatusSlot983(type);
}

// 0049c490: slots do taco + slots ganhos na oficina (+ cartas do taco)
unsigned short GetClubSlotMax983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem, bool bCard)
{
	int value = 0;
	if (pClub && pItem)
	{
		short card = bCard ? GetCardSlot983(type) : 0;
		value = AddStat983(pClub->Slot[type], pItem->slotUp983[type], card);
	}
	return value;
}

// 0049c4e0
IFF_STRUCT::sClubSet* FindClubSetIff983(unsigned long tid)
{
	return ItemManager()->FindClubSet(tid);
}

// 0049c500: atributo-base + subidas, no máximo o teto de slots
unsigned short GetClubStat983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem, bool bCard)
{
	int value = 0;
	if (pClub && pItem)
		value = AddStatMax983(pClub->Attr[type], pItem->Common[type], GetClubSlotMax983(type, pClub, pItem, bCard));
	return value;
}

// 0049c550
int GetClubSetSlotSum983(unsigned long tid)
{
	int sum = 0;
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(tid);
	if (pClub)
	{
		for (int i = 0; i < 5; i++)
			sum += pClub->Slot[i];
	}
	return sum;
}

// 0049c590
const char* GetClubSetName983(unsigned long tid)
{
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(tid);
	if (pClub)
		return pClub->c.Name;
	return NULL;
}

// 0049c5a0
int GetClubSetStep983(unsigned long tid)
{
	switch (GetClubSetSlotSum983(tid))
	{
	case 30: case 35: case 40: case 45: case 50:
		return 0;
	case 31: case 36: case 41: case 46: case 51:
		return 1;
	case 32: case 37: case 42: case 47: case 52:
		return 2;
	case 33: case 38: case 43: case 48: case 53:
		return 3;
	case 34: case 39: case 44: case 49: case 54:
	case 55: case 56: case 57: case 58: case 59:
		return 4;
	}
	return -1;
}

// 0049c610
int GetClubSetGrade983(unsigned long tid)
{
	switch (GetClubSetSlotSum983(tid))
	{
	case 30: case 31: case 32: case 33: case 34:
		return 0;
	case 35: case 36: case 37: case 38: case 39:
		return 1;
	case 40: case 41: case 42: case 43: case 44:
		return 2;
	case 45: case 46: case 47: case 48: case 49:
		return 3;
	case 50: case 51: case 52: case 53: case 54:
		return 4;
	case 55: case 56: case 57: case 58: case 59:
		return 5;
	}
	return -1;
}

// 0049c690
sItemInfo* FindMyClubSet983(unsigned long guid)
{
	std::map<unsigned int, sItemInfo>& clubs = Doc()->m_clubSetMap;
	for (std::map<unsigned int, sItemInfo>::const_iterator it = clubs.begin(); it != clubs.end(); ++it)
	{
		if (it->second.guid == guid)
			return (sItemInfo*)&it->second;
	}
	return NULL;
}

// 0049c6e0
short GetMyClubStat983(int type, unsigned long guid, bool bCard)
{
	int value = 0;
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(pItem->tid);
		if (pClub)
			value = GetClubStat983(type, pClub, pItem, true);
	}
	return value;
}

// 0049c720
short GetMyClubSlotMax983(int type, unsigned long guid, bool bCard)
{
	int value = 0;
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(pItem->tid);
		if (pClub)
			value = GetClubSlotMax983(type, pClub, pItem, bCard);
	}
	return value;
}

// 0049c770
int GetMyClubSlotSum983(unsigned long guid)
{
	int sum = 0;
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(pItem->tid);
		if (pClub)
		{
			short* up = pItem->slotUp983;
			sum = pClub->Slot[0] + pClub->Slot[1] + pClub->Slot[2] + pClub->Slot[3] + pClub->Slot[4] + up[0] + up[1] + up[2] + up[3] + up[4];
		}
	}
	return sum;
}

// 0049c7f0
bool HasClubCard983(unsigned long guid)
{
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		for (int i = 0; i < 12; i++)
		{
			if (pItem->attachCard[i])
				return true;
		}
	}
	return false;
}

// 0049c820
IFF_STRUCT::sClubSet* FindMyClubSetIff983(unsigned long guid)
{
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
		return FindClubSetIff983(pItem->tid);
	return NULL;
}

// 0049c840
int GetMyClubStep983(unsigned long guid)
{
	switch (GetMyClubSlotSum983(guid))
	{
	case 30: case 35: case 40: case 45: case 50:
		return 0;
	case 31: case 36: case 41: case 46: case 51:
		return 1;
	case 32: case 37: case 42: case 47: case 52:
		return 2;
	case 33: case 38: case 43: case 48: case 53:
		return 3;
	case 34: case 39: case 44: case 49: case 54:
	case 55: case 56: case 57: case 58: case 59:
		return 4;
	}
	return -1;
}

// 0049c8b0
int GetMyClubGrade983(unsigned long guid)
{
	switch (GetMyClubSlotSum983(guid))
	{
	case 30: case 31: case 32: case 33: case 34:
		return 0;
	case 35: case 36: case 37: case 38: case 39:
		return 1;
	case 40: case 41: case 42: case 43: case 44:
		return 2;
	case 45: case 46: case 47: case 48: case 49:
		return 3;
	case 50: case 51: case 52: case 53: case 54:
		return 4;
	case 55: case 56: case 57: case 58: case 59:
		return 5;
	}
	return -1;
}

// 0049c930: o taco do jogador mudou de passo em relação ao do ClubSet.iff
bool IsClubStepChanged983(unsigned long guid)
{
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		int base = GetClubSetStep983(pItem->tid);
		return GetMyClubStep983(guid) != base;
	}
	return false;
}

// 0049c960: o taco do jogador mudou de grau (rank)
bool IsClubGradeChanged983(unsigned long guid)
{
	sItemInfo* pItem = FindMyClubSet983(guid);
	if (pItem)
	{
		int base = GetClubSetGrade983(pItem->tid);
		return GetMyClubGrade983(guid) != base;
	}
	return false;
}

// 00a06fb0: acha o registro (tipo, grau) da tabela +0x38c (ClubSetWorkShopLevelUpLimit, 0x14 bytes: int,int,short x5 a partir de +8 com +0xa, +0xe, +0x10)
bool CItemManagerStatJp::GetClubRankBonus983(int type, int grade, short* p0, short* p1, short* p2, short* p3, short* p4)
{
	std::vector<IFF983::sClubSetWorkShopLevelUpLimit>& v = ((CItemManager*)this)->m_WorkShopLevelUpLimit983;
	for (size_t i = 0; i < v.size(); i++)
	{
		const unsigned char* r = v[i].raw;
		if (*(const int*)(r + 0) == type && *(const int*)(r + 4) == grade)
		{
			*p0 = *(const short*)(r + 8);
			*p1 = *(const short*)(r + 10);
			*p2 = *(const short*)(r + 12);
			*p3 = *(const short*)(r + 14);
			*p4 = *(const short*)(r + 16);
			return true;
		}
	}
	return false;
}

// 00a07030: registros de 0x1c bytes (+0 tipo do taco, +4 + 4*rank = exp)
bool CItemManagerRankUpJp::GetRankUpExp983(unsigned long typeId, int rank, int* pExp)
{
	std::vector<IFF983::sClubSetWorkShopRankUpExp>& v = ((CItemManager*)this)->m_WorkShopRankUpExp983;
	for (size_t i = 0; i < v.size(); i++)
	{
		const unsigned char* r = v[i].raw;
		if (*(const unsigned long*)r == typeId)
		{
			*pExp = *(const int*)(r + 4 + 4 * rank);
			return true;
		}
	}
	return false;
}

// 0049c990: bônus de slot do rank (tabela do CItemManager por tipo do taco e grau; +1 no grau 5)
short GetClubRankBonus983(int type, IFF_STRUCT::sClubSet* pClub, sItemInfo* pItem)
{
	int value = 0;
	if (pClub && pItem)
	{
		int grade = GetMyClubGrade983(pItem->guid);
		unsigned short bonus[5];
		((CItemManagerStatJp*)ItemManager())->GetClubRankBonus983(pClub->nWorkshopType983, grade, (short*)&bonus[0],
			(short*)&bonus[1], (short*)&bonus[2], (short*)&bonus[3], (short*)&bonus[4]);
		value = bonus[type];
		if (grade == 5)
			value++;
	}
	return value;
}

// 0049ca10
short GetCapacitySlot983(const sCapacityParam983* p, bool bCard)
{
	int value = 0;
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(p->tidClub);
	if (pClub && p->clubSlotUp)
	{
		int type = p->type;
		short slot = pClub->Slot[type];
		short up = p->clubSlotUp[type];
		short card = bCard ? GetCardSlot983(type) : 0;
		bool bGuid = true;
		if (p->guidClub == 0)
			bGuid = false;

		sItemInfo* pItem = FindMyClubSet983(p->guidClub);
		bool bRank;
		if (pItem)
		{
			bRank = false;
			if (bGuid && pClub->nWorkshopType983 != -1)
				bRank = GetClubRankBonus983(type, pClub, pItem) < slot + up;
		}
		else
			bRank = bGuid;
		if (bRank)
			value = AddStat983(up, 0, card);
		else
			value = AddStat983(slot, up, card);
	}
	return value;
}

// 0049cb00
short GetMyClubRankBonus983(int type, unsigned long guid)
{
	sItemInfo* pItem = FindMyClubSet983(guid);
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(pItem->tid);
	return GetClubRankBonus983(type, pClub, pItem);
}

// 0049cb30
short GetCapacityStat983(const sCapacityParam983* p, bool bCard)
{
	int value = 0;
	IFF_STRUCT::sClubSet* pClub = FindClubSetIff983(p->tidClub);
	if (pClub && p->clubAttrUp)
	{
		int type = p->type;
		value = AddStatMax983(pClub->Attr[type], p->clubAttrUp[type], GetCapacitySlot983(p, bCard));
	}
	return value;
}
