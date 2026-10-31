// JP 983: unidade 008515c0–008527c0 (o corte da F3.7.1 começava em 008524d0 e ia até 008529c0; 008515c0 é a 1ª função depois
// do último membro da FrPointShopDlg, 008515a0, e 008527c0–008529c0 é da CChampionShipImpl): CPointShop, a lógica da loja de
// pontos do Tiki. Entre as funções próprias o 983 guarda os moldes de std::map<unsigned long, CPointShop::sItem>
// (00851640–00851a70, 00851ae0–00851e60, 008521a0–008524a0, 00852550, 008525c0). Só a FrPointShopDlg chama. No 645 o
// pointshop.cpp existe vazio. Nomes meus (sessão B106); ver include/jp/pointshop.h.
#include "minatl.h"
#include "projectg.h"
#include "packet.h"
#include "shareddoc.h"
#include "../../shared/itemmanager.h"
#include "pointshop.h"

// 008515c0
unsigned long CPointShop::GetKey983(const sItem& item)
{
	sItem copy = item;
	unsigned long key = 0;
	switch (m_mode)
	{
	case 1: return copy.uid;
	case 2: return copy.typeId;
	}
	return key;
}

// 00851810
int CPointShop::GetPoint983(unsigned long typeId)
{
	IFF_ITEM_COMMON* pItem;
	int point = 0;
	switch (typeId >> 26)
	{
	case 2: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindPart(typeId); break;
	case 5: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindBall(typeId); break;
	case 6: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindItem(typeId); break;
	case 4: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(typeId); break;
	case 0x1f: pItem = (IFF_ITEM_COMMON*)ItemManager()->FindCard(typeId); break;
	default: return point;
	}
	if (pItem != NULL)
		point = pItem->TikiPoints;
	return point;
}

// 008518e0
int CPointShop::GetUnitCount983(unsigned long typeId)
{
	IFF_ITEM_COMMON* pItem;
	int count = 0;
	switch (typeId >> 26)
	{
	case 2:
		if (ItemManager()->FindPart(typeId) != NULL)
			count = 1;
		break;
	case 4:
		if (ItemManager()->FindClubSet(typeId) != NULL)
			count = 1;
		break;
	case 5:
		pItem = (IFF_ITEM_COMMON*)ItemManager()->FindBall(typeId);
		if (pItem != NULL)
			count = pItem->TikiQtyPerPoint;
		break;
	case 6:
		pItem = (IFF_ITEM_COMMON*)ItemManager()->FindItem(typeId);
		if (pItem != NULL)
			count = pItem->TikiQtyPerPoint;
		break;
	case 0x1f:
		pItem = (IFF_ITEM_COMMON*)ItemManager()->FindCard(typeId);
		if (pItem != NULL)
			count = pItem->TikiQtyPerPoint;
		break;
	}
	return count;
}

// 00851e90
void CPointShop::Send983()
{
	unsigned char count = (unsigned char)m_items.size();
	if (m_mode == 1)
	{
		WSendPacket packet((enumClientPacket)0x128);
		packet.Encode1(count);
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			packet.Encode4(it->second.typeId);
			packet.Encode4(it->second.uid);
			packet.Encode4(it->second.count);
			packet.Encode4(GetPoint983(it->second.typeId));
		}
		packet.Send(TO_GAME);
	}
	else if (m_mode == 2)
	{
		WSendPacket packet((enumClientPacket)0x129);
		packet.Encode1(count);
		for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
		{
			IFF983::sPointShop* pShop = ItemManager()->FindPointShop983(it->second.typeId);
			if (pShop != NULL)
			{
				packet.Encode4(it->second.typeId);
				packet.Encode4(it->second.count);
				packet.Encode4(*(int*)&pShop->raw[0]);
			}
		}
		packet.Send(TO_GAME);
	}
}

// 00852080
int CPointShop::SetMode983(int mode)
{
	if (m_mode == mode)
		return 0;
	if (mode <= 0 && mode >= 4)
		return -1;
	m_items.clear();
	m_mode = mode;
	return 1;
}

// 008520c0
void CPointShop::Clear983()
{
	m_items.clear();
}

// 008520d0
int CPointShop::GetTotal983()
{
	if (m_mode == 3)
		return 0;
	int total = 0;
	for (ItemMap::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		int point = 0;
		switch (m_mode)
		{
		case 1:
			point = GetPoint983(it->second.typeId) * it->second.count;
			break;
		case 2:
			{
				IFF983::sPointShop* pShop = ItemManager()->FindPointShop983(it->second.typeId);
				if (pShop != NULL)
					point = *(int*)&pShop->raw[0] * it->second.count;
			}
			break;
		}
		total += point;
	}
	return total;
}

// 008523e0
int CPointShop::Remove983(const sItem& item)
{
	if (m_mode == 3)
		return 0;
	unsigned long key = GetKey983(item);
	if (key == 0)
		return 0;
	ItemMap::iterator it = m_items.find(key);
	if (it == m_items.end())
		return 0;
	--it->second.count;
	int left = it->second.count;
	if (left <= 0)
		m_items.erase(it);
	return left;
}

// 00852450
CPointShop::sItem* CPointShop::Find983(const sItem& item)
{
	if (m_mode == 3)
		return NULL;
	unsigned long key = GetKey983(item);
	if (key == 0)
		return NULL;
	sItem* p = NULL;
	ItemMap::iterator it = m_items.find(key);
	if (it != m_items.end())
		p = &it->second;
	return p;
}

// 008524d0
CPointShop::~CPointShop()
{
	Clear983();
}

// 00852650
CPointShop::CPointShop()
{
}

// 00852700
CPointShop::sItem* CPointShop::Add983(const sItem& item)
{
	if (m_mode == 3)
		return NULL;
	sItem copy = item;
	unsigned long key = GetKey983(item);
	if (key == 0)
		return NULL;
	sItem* p = Find983(item);
	if (p == NULL)
	{
		if (m_items.size() > MAX_ITEMS - 1)
			return NULL;
		m_items[key] = copy;
		sItem* pNew = &m_items.find(key)->second;
		++pNew->count;
		return pNew;
	}
	unsigned long kind = p->typeId >> 26;
	if (kind == 2 || kind == 4)
		return NULL;
	++p->count;
	return p;
}
