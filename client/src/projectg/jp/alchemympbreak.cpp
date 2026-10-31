// JP 983: unidade 00475740–00477740 (o corte da F3.7.1 começava em 004756f9, dentro do fim do aiplayer): CAlchemyMPBreak, a
// lógica da janela de quebra de itens do Tiki (FrAlchemyMPBreakDlg). Entre as funções próprias o 983 guarda os moldes de
// std::map<unsigned long, CAlchemyMPBreak::sItem> (00475890–00476a00, 00477010–00477570). Só a FrAlchemyMPBreakDlg chama.
// Nomes meus (sessão B94); ver include/jp/alchemympbreak.h.
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "ucclibrary.h"
#include "gamestringmanager.h"
#include "clubsetstat.h"
#include "alchemympbreak.h"

// 00475740
unsigned long CAlchemyMPBreak::GetKey(const sItem& item)
{
	sItem copy = item;
	if (m_mode == 3)
		return copy.guid;
	return 0;
}

// 00475770
void CAlchemyMPBreak::SetValues(int a, int b)
{
	m_value2c = a;
	m_value30 = b;
}

// 00475790
void CAlchemyMPBreak::ResetFlags()
{
	*(int*)m_flags = 0;
}

// 004757a0
bool CAlchemyMPBreak::GetFlag(int index)
{
	return m_flags[index];
}

// 004757b0
void CAlchemyMPBreak::SetFlag(int index, bool value)
{
	m_flags[index] = value;
}

// 004757d0
const char* CAlchemyMPBreak::GetFlagText(int index)
{
	if (GetFlag(index))
	{
		switch (index)
		{
		case 0: return GetGameString(0x18705);
		case 1: return GetGameString(0x18706);
		case 2: return GetGameString(0x18707);
		case 3: return GetGameString(0x18708);
		}
	}
	return NULL;
}

// 00475840
bool CAlchemyMPBreak::HasFlag()
{
	for (int i = 0; i < FLAG_COUNT; ++i)
	{
		if (m_flags[i])
			return true;
	}
	return false;
}

// 00475860
int CAlchemyMPBreak::GetFirstFlag()
{
	for (int i = 0; i < FLAG_COUNT; ++i)
	{
		if (m_flags[i])
			return i;
	}
	return FLAG_COUNT;
}

// 00475880
int CAlchemyMPBreak::sItem::GetCount() const
{
	return count;
}

// 00475ed0
int CAlchemyMPBreak::GetBreakType(unsigned long typeId)
{
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(typeId);
	if (pItem == NULL)
		return 0;
	return pItem->TikiType;
}

// 00475f00
int CAlchemyMPBreak::GetBreakKind(unsigned long typeId)
{
	int kind = 0;
	switch (GetBreakType(typeId))
	{
	case 1: kind = 1; break;
	case 2: kind = 2; break;
	case 3: kind = 3; break;
	}
	return kind;
}

// 00475f40
int CAlchemyMPBreak::GetMPItemCount()
{
	sItemInfo* pInfo = Doc()->FindMyItemByTypeId983(0x1a0002a6);
	if (pInfo == NULL)
		return 0;
	return pInfo->Common[0];
}

// 00475f60
int CAlchemyMPBreak::GetMPPointCount()
{
	sItemInfo* pInfo = Doc()->FindMyItemByTypeId983(0x1a0002a7);
	if (pInfo == NULL)
		return 0;
	return pInfo->Common[0];
}

// 00475f80
int CAlchemyMPBreak::GetItemMP(unsigned long typeId)
{
	IFF_ITEM_COMMON* pItem = NULL;
	int mp = 0;
	switch (typeId >> 26)
	{
	case 2: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindPart(typeId); break;
	case 5: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindBall(typeId); break;
	case 6: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindItem(typeId); break;
	case 4: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(typeId); break;
	case 0x1f: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindCard(typeId); break;
	case 0x1c: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindAuxPart(typeId); break;
	}
	if (pItem != NULL)
		mp = pItem->Mileage;
	return mp;
}

// 00476a20
void CAlchemyMPBreak::SendBreak()
{
	unsigned char count = (unsigned char)m_items.size();
	if (m_mode != 3)
		return;
	WSendPacket packet((enumClientPacket)0x18d);
	packet.Encode4(count);
	for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		packet.Encode4(it->second.typeId);
		packet.Encode4(it->second.guid);
		packet.Encode4(it->second.count);
	}
	packet.Send(TO_GAME);
}

// 00476b20
__int64 CAlchemyMPBreak::GetTotalValue()
{
	unsigned long total = 0;
	for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(it->second.typeId);
		if (pItem != NULL)
			total += pItem->TikiPang;
	}
	return total;
}

// 00476ba0
int CAlchemyMPBreak::SetMode(int mode)
{
	if (m_mode == mode)
		return 0;
	if (mode <= 0 && mode >= 4)
		return -1;
	m_items.clear();
	m_mode = mode;
	return 1;
}

// 00476be0
void CAlchemyMPBreak::Clear()
{
	m_items.clear();
}

// 00476bf0
bool CAlchemyMPBreak::HasUccItem()
{
	if (m_mode == 3)
	{
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			if (IsUccClothes(it->second.typeId))
				return true;
		}
	}
	return false;
}

// 00476c70
bool CAlchemyMPBreak::HasType3Item()
{
	if (m_mode == 3)
	{
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(it->second.typeId);
			if (pItem != NULL && (pItem->TypeId & 0xfc000000) != 0x7c000000 && pItem->TikiType == 3)
				return true;
		}
	}
	return false;
}

// 00476d20
bool CAlchemyMPBreak::HasUpgradedClubSet()
{
	if (m_mode == 3)
	{
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			if ((it->second.typeId & 0xfc000000) != 0x10000000)
				continue;
			sItemInfo* pInfo = Doc()->FindClubSet983(it->second.guid);
			if (pInfo == NULL)
				continue;
			unsigned long pang = 0;
			for (int stat = 0; stat < 5; ++stat)
			{
				for (int i = 0; i < pInfo->Common[stat]; ++i)
				{
					IFF_STRUCT::sEnchant* pEnchant = ItemManager()->FindEnchant(((stat | 0x340) << 20) | i);
					if (pEnchant != NULL)
						pang += pEnchant->ReqdPang;
				}
			}
			if (IsClubStepChanged983(it->second.guid) || pang > 0)
				return true;
		}
	}
	return false;
}

// 00476e60
bool CAlchemyMPBreak::HasRareCard()
{
	if (m_mode == 3)
	{
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			if ((it->second.typeId & 0xfc000000) != 0x7c000000)
				continue;
			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(it->second.typeId);
			if (pCard != NULL && pCard->RareType >= 1)
				return true;
		}
	}
	return false;
}

// 00476f10
void CAlchemyMPBreak::UpdateFlags()
{
	ResetFlags();
	bool flag = HasUccItem();
	if (flag)
		SetFlag(0, flag);
	flag = HasType3Item();
	if (flag)
		SetFlag(1, flag);
	flag = HasUpgradedClubSet();
	if (flag)
		SetFlag(2, flag);
	flag = HasRareCard();
	if (flag)
		SetFlag(3, flag);
}

// 00476f90
int CAlchemyMPBreak::CalcMP()
{
	int total = 0;
	for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		int mp = 0;
		if (m_mode == 3)
			mp = GetItemMP(it->second.typeId) * it->second.count;
		total += mp;
	}
	return total;
}

// 00477270
void CAlchemyMPBreak::UpdateMP()
{
	m_mpItems = GetMPItemCount();
	m_mp = CalcMP();
	m_mpTotal = CalcMP() + GetMPPointCount();
}

// 004772a0
void CAlchemyMPBreak::UpdateOver()
{
	int total = CalcMP() + GetMPPointCount();
	m_bOver = false;
	if (total >= 1000)
		m_bOver = true;
}

// 004772d0
int CAlchemyMPBreak::Remove(const sItem& item, int* pRemoved)
{
	unsigned long key = GetKey(item);
	if (key == 0)
		return 0;
	ItemMap::iterator it = m_items.find(key);
	if (it == m_items.end())
		return 0;
	int count = it->second.count;
	*pRemoved = count;
	it->second.count -= count;
	int left = it->second.count;
	if (left <= 0)
		m_items.erase(it);
	return left;
}

// 00477340
CAlchemyMPBreak::sItem* CAlchemyMPBreak::Find(const sItem& item)
{
	unsigned long key = GetKey(item);
	if (key == 0)
		return NULL;
	sItem* p = NULL;
	ItemMap::iterator it = m_items.find(key);
	if (it != m_items.end())
		p = &it->second;
	return p;
}

// 00477480
CAlchemyMPBreak::~CAlchemyMPBreak()
{
	Clear();
}

// 00477600
CAlchemyMPBreak::CAlchemyMPBreak()
{
	ResetFlags();
	m_mpItems = 0;
	m_20 = 0;
	m_mp = 0;
	m_mpTotal = 0;
	m_value2c = 0;
	m_value30 = 0;
	m_bOver = false;
}

// 004776c0
CAlchemyMPBreak::sItem* CAlchemyMPBreak::Add(const sItem& item)
{
	sItem copy = item;
	unsigned long key = GetKey(item);
	if (key == 0)
		return NULL;
	sItem* p = Find(item);
	if (p != NULL)
		return p;
	if (m_items.size() > MAX_ITEMS - 1)
		return NULL;
	m_items[key] = copy;
	return &m_items.find(key)->second;
}
