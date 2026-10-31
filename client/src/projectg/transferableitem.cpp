#include "minatl.h"
#include "transferableitem.h"
#include "shareddoc.h"
#include "../shared/localize.h"

bool IsUccClothes(unsigned long tid);


CTransferableItem::CTransferableItem()
{
}

CTransferableItem::~CTransferableItem()
{
}

bool CompareTradeItem(const sTradeItem& a, const sTradeItem& b)
{
	return a.dwTid < b.dwTid;
}

void CTransferableItem::OnLoad()
{
	Reset();
}

void CTransferableItem::OnDestory()
{
	m_listTransferableItem.clear();
	m_listRegistrationItem.clear();
	m_listEquipParts.clear();

	m_registrationListSize = 0;
}

void CTransferableItem::Process(float time)
{
}

void CTransferableItem::Render()
{
}

void CTransferableItem::Reset()
{
	m_listTransferableItem.clear();
	m_listRegistrationItem.clear();
	m_listEquipParts.clear();

	m_registrationListSize = 0;
}

std::list<sTradeItem>* CTransferableItem::GetList_TransferableItem()
{
	return &m_listTransferableItem;
}

void CTransferableItem::ClearTransferableItemList()
{
	m_listTransferableItem.clear();
}

void CTransferableItem::SetTransferableItemList()
{
	m_listTransferableItem.clear();

	sTradeItem item;
	std::list<sItemInfo>* pList;

	pList = &Doc()->m_partsList; // 983 00811090: Doc+0x3734 (0x117c era o 645)
	for (std::list<sItemInfo>::iterator it = pList->begin(); it != pList->end();
		it++)
	{
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).guid;
		item.dwTid = (*it).tid;
		item.iNum = 1 - GetNumTradingItem((*it).guid);
		item.i64Price = 0;

		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			IFF_STRUCT::sPart* pPart = ItemManager()->FindPart((*it).tid);
			if (pPart)
			{
				// 983 00811090: sPart +0x228/+0x22a/+0x22c (o 645 lia +0x1f8)
				item.CharacterSlotNum = pPart->CharacterSlot;
				item.CaddieSlotNum = pPart->CaddieSlot;
				item.NpcSlotNum983 = pPart->NpcSlot983;

				memset(item.attachCardTid, 0, sizeof(item.attachCardTid));
				for (std::list<sSCardAvilityPeriodInfo>::iterator cit =
						 Doc()->m_cardAbilityList[0].begin();
					cit != Doc()->m_cardAbilityList[0].end(); cit++)
				{
					if ((*cit).partsTid == (*it).tid &&
						(*cit).partsUid == (*it).guid)
					{
						int slot = (*cit).slotNum - 1;
						if (((*cit).tid & 0x3c00000) == 0x400000)
							slot += 4;

						item.attachCardTid[0][slot] = (*cit).tid;
					}
				}
			}
		}

		m_listTransferableItem.push_back(item);
	}

	if (IsLocalContent(S4_UCC))
	{
		pList = &Doc()->m_uccItemList; // 983: Doc+0x3878
		for (std::list<sItemInfo>::iterator it = pList->begin();
			it != pList->end(); it++)
		{
			IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
			if (pCommon == NULL)
				continue;
			if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
				continue;

			bool bEquipped = false;
			for (std::map<unsigned int, sCharacterInfo>::iterator cit =
					 Doc()->m_charMap.begin();
				cit != Doc()->m_charMap.end(); cit++)
			{
				if ((*cit).second.tid ==
					(0x4000000 | (((*it).tid >> 18) & 0xff)))
				{
					int idx = ((*it).tid >> 13) & 0x1f;

					if ((*cit).second.ItemIdList[idx] == (*it).guid)
					{
						bEquipped = true;
						break;
					}
				}
			}
			if (bEquipped)
				continue;

			item.dwGuid = (*it).guid;
			item.dwTid = (*it).tid;
			strncpy(item.UccIndex, (*it).UccIndex, 9);
			item.Seq = (*it).Seq;
			item.iNum = 1;
			item.i64Price = 0;
			item.status = (*it).status;

			if (IsLocalContent(S4_CARD_SYSTEM))
			{
				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart((*it).tid);
				if (pPart)
				{
					// 983 00811090: sPart +0x228/+0x22a/+0x22c (o 645 lia +0x1f8)
					item.CharacterSlotNum = pPart->CharacterSlot;
					item.CaddieSlotNum = pPart->CaddieSlot;
					item.NpcSlotNum983 = pPart->NpcSlot983;

					memset(item.attachCardTid, 0, sizeof(item.attachCardTid));
					for (std::list<sSCardAvilityPeriodInfo>::iterator cit =
							 Doc()->m_cardAbilityList[0].begin();
						cit != Doc()->m_cardAbilityList[0].end(); cit++)
					{
						if ((*cit).partsTid == (*it).tid &&
							(*cit).partsUid == (*it).guid)
						{
							int slot = (*cit).slotNum - 1;
							if (((*cit).tid & 0x3c00000) == 0x400000)
								slot += 4;

							item.attachCardTid[0][slot] = (*cit).tid;
						}
					}
				}
			}

			m_listTransferableItem.push_back(item);
		}
	}

	pList = &Doc()->m_auxPartList; // 983: Doc+0x37bc
	for (std::list<sItemInfo>::iterator it = pList->begin(); it != pList->end();
		it++)
	{
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).guid;
		item.dwTid = (*it).tid;
		item.iNum = (*it).Common[0] - GetNumTradingItem((*it).guid);
		item.i64Price = 0;

		m_listTransferableItem.push_back(item);
	}

	SetListEquipParts();

	for (std::list<sTradeItem>::iterator it = m_listTransferableItem.begin();
		it != m_listTransferableItem.end();)
	{
		if (IsEquipParts((*it).dwTid, (*it).dwGuid))
		{
			it = m_listTransferableItem.erase(it);
			continue;
		}
		++it;
	}

	pList = &Doc()->m_myItemList;
	for (std::list<sItemInfo>::iterator it = pList->begin(); it != pList->end();
		it++)
	{
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).guid;
		item.dwTid = (*it).tid;
		item.iNum = (*it).Common[0] - GetNumTradingItem((*it).guid) -
			GetNumEquipItem((*it).tid);
		item.i64Price = 0;

		if (item.iNum > 0)
			m_listTransferableItem.push_back(item);
	}

	pList = &Doc()->m_ballList;
	for (std::list<sItemInfo>::iterator it = pList->begin(); it != pList->end();
		it++)
	{
		if ((*it).tid == Doc()->m_myInfo.userEquip.tidBall)
			continue;

		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).guid;
		item.dwTid = (*it).tid;
		item.iNum = (*it).Common[0] - GetNumTradingItem((*it).guid);
		item.i64Price = 0;

		m_listTransferableItem.push_back(item);
	}

	std::map<unsigned int, sCaddieInfo>& caddieMap = Doc()->m_caddieMap;
	for (std::map<unsigned int, sCaddieInfo>::iterator it = caddieMap.begin();
		it != caddieMap.end(); it++)
	{
		if ((*it).second.guid == Doc()->m_myInfo.userEquip.guidCaddie)
			continue;

		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).second.guid;
		item.dwTid = (*it).second.tid;
		item.iNum = 1 - GetNumTradingItem((*it).second.guid);
		item.i64Price = 0;

		m_listTransferableItem.push_back(item);
	}

	std::map<unsigned int, sItemInfo>& clubMap = Doc()->m_clubSetMap;
	for (std::map<unsigned int, sItemInfo>::iterator it = clubMap.begin();
		it != clubMap.end(); it++)
	{
		if ((*it).second.guid == Doc()->m_myInfo.userEquip.guidClubSet)
			continue;

		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pCommon == NULL)
			continue;
		// 983 00811938: taco com slot ganho no Club Workshop nao entra
		bool bSlotUp = false;
		for (int i = 0; i < 5; i++)
		{
			if ((*it).second.slotUp983[i] != 0)
			{
				bSlotUp = true;
				break;
			}
		}
		if (bSlotUp)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		memset(&item, 0, sizeof(item));

		item.dwGuid = (*it).second.guid;
		item.dwTid = (*it).second.tid;
		item.iNum = 1 - GetNumTradingItem((*it).second.guid);
		item.i64Price = 0;
		item.dwUpgradeCost = 0;
		memcpy(item.arrEnchant, (*it).second.Common, sizeof(item.arrEnchant));

		for (short i = 0; i < 5; i++)
		{
			for (int j = 0; j < item.arrEnchant[i]; j++)
			{
				IFF_STRUCT::sEnchant* pEnchant =
					ItemManager()->FindEnchant(((0x340 | i) << 20) | j);
				if (pEnchant)
					item.dwUpgradeCost += pEnchant->ReqdPang;
			}
		}

		m_listTransferableItem.push_back(item);
	}

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		std::list<sCards>& cardList = Doc()->m_cardList;
		for (std::list<sCards>::iterator it = cardList.begin();
			it != cardList.end(); it++)
		{
			IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
			if (pCommon == NULL)
				continue;
			if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
				continue;

			item.dwGuid = (*it).uid;
			item.dwTid = (*it).tid;
			item.iNum = (*it).cardCount;
			item.i64Price = 0;

			m_listTransferableItem.push_back(item);
		}
	}
}

bool CTransferableItem::GetTransferableItem(sItemInfo& info, sTradeItem& item,
	bool bCount)
{
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(info.tid);

	int num;
	if (bCount)
		num = info.Common[0];
	else
		num = 1;

	if (pCommon == NULL)
		return false;

	if (pCommon->IsSalable == 1 || pCommon->IsSalable == 3)
	{
		if (num == 0)
			return false;

		item.iNum = num;
		item.dwGuid = info.guid;
		item.dwTid = info.tid;
		item.i64Price = 0;

		if (IsLocalContent(S4_UCC) && IsUccClothes(info.tid))
		{
			strncpy(item.UccIndex, info.UccIndex, 8);
			item.Seq = info.Seq;
			item.status = info.status;
		}

		return true;
	}

	return false;
}

void CTransferableItem::SetTransferableEtcItemList()
{
	sTradeItem item;

	std::map<unsigned int, sCaddieInfo>& caddieMap = Doc()->m_caddieMap;
	for (std::map<unsigned int, sCaddieInfo>::iterator it = caddieMap.begin();
		it != caddieMap.end(); it++)
	{
		if ((*it).second.guid == Doc()->m_myInfo.userEquip.guidCaddie)
			continue;

		memset(&item, 0, sizeof(item));
		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		item.dwGuid = (*it).second.guid;
		item.dwTid = (*it).second.tid;
		item.iNum = 1;
		item.i64Price = 0;

		m_listTransferableItem.push_back(item);
	}

	std::map<unsigned int, sItemInfo>& clubMap = Doc()->m_clubSetMap;
	for (std::map<unsigned int, sItemInfo>::iterator it = clubMap.begin();
		it != clubMap.end(); it++)
	{
		if ((*it).second.guid == Doc()->m_myInfo.userEquip.guidClubSet)
			continue;

		memset(&item, 0, sizeof(item));
		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pCommon == NULL)
			continue;
		if (pCommon->IsSalable != 1 && pCommon->IsSalable != 3)
			continue;

		memset(&item, 0, sizeof(item));

		item.dwGuid = (*it).second.guid;
		item.dwTid = (*it).second.tid;
		item.iNum = 1;
		item.i64Price = 0;
		item.dwUpgradeCost = 0;
		memcpy(item.arrEnchant, (*it).second.Common, sizeof(item.arrEnchant));

		for (short i = 0; i < 5; i++)
		{
			for (int j = 0; j < item.arrEnchant[i]; j++)
			{
				IFF_STRUCT::sEnchant* pEnchant =
					ItemManager()->FindEnchant(((0x340 | i) << 20) | j);
				if (pEnchant)
					item.dwUpgradeCost += pEnchant->ReqdPang;
			}
		}

		m_listTransferableItem.push_back(item);
	}
}

void CTransferableItem::SetListEquipParts()
{
	m_listEquipParts.clear();

	for (std::map<unsigned int, sCharacterInfo>::iterator it =
			 Doc()->m_charMap.begin();
		it != Doc()->m_charMap.end(); it++)
	{
		for (int i = 0; i < 24; i++)
		{
			if ((*it).second.tidParts[i])
				m_listEquipParts.push_back((*it).second.guid);
		}
	}
}

bool CTransferableItem::IsEquipParts(unsigned long tid)
{
	bool bFind = false;
	for (std::list<unsigned long>::iterator it = m_listEquipParts.begin();
		it != m_listEquipParts.end(); it++)
	{
		if (tid == *it)
		{
			bFind = true;
			break;
		}
	}

	return bFind;
}

bool CTransferableItem::IsUccEquipped(sItemInfo* pItem)
{
	bool bEquipped = false;
	for (std::map<unsigned int, sCharacterInfo>::iterator it =
			 Doc()->m_charMap.begin();
		it != Doc()->m_charMap.end(); it++)
	{
		sCharacterInfo* pChar = &(*it).second;
		if (pChar)
			bEquipped = true;
		break;
	}

	return bEquipped;
}

int CTransferableItem::GetNumEquipItem(unsigned long tid)
{
	int count = 0;
	for (int i = 0; i < 10; i++)
	{
		if (tid == Doc()->m_myInfo.userEquip.tidItemSlot[i])
			count++;
	}

	return count;
}

void CTransferableItem::Set_RegistrationListSize(int size)
{
	m_registrationListSize = size;
}

bool CTransferableItem::SetRegistrationItem(unsigned long guid)
{
	sTradeItem item;
	memset(&item, 0, sizeof(item));

	if (m_listRegistrationItem.size() >= m_registrationListSize)
		return false;

	if (IsTransferableItem(guid, item, 0))
	{
		m_listRegistrationItem.push_back(item);
		return true;
	}

	return false;
}

bool CTransferableItem::SetRegistrationItem(unsigned long guid, int num)
{
	sTradeItem item;
	memset(&item, 0, sizeof(item));

	if (m_listRegistrationItem.size() < m_registrationListSize)
	{
		if (IsTransferableItem(guid, item, num))
		{
			item.iNum = num;
			m_listRegistrationItem.push_back(item);
			return true;
		}
	}

	return false;
}

bool CTransferableItem::IsTransferableItem(unsigned long guid, sTradeItem& item,
	int num)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		m_listRegistrationItem.erase(m_listRegistrationItem.begin());
	for (std::list<sTradeItem>::iterator it = m_listTransferableItem.begin();
		it != m_listTransferableItem.end();)
	{
		if ((*it).dwGuid == guid)
		{
			if (num)
			{
				if ((*it).iNum < num)
					return false;
				else if ((*it).iNum == num)
					(*it).iNum -= num;
				else
					(*it).iNum -= num;
			}
			item = *it;
			return true;
		}
		++it;
	}

	return false;
}

bool CTransferableItem::IsRegisterationItem(unsigned long guid,
	sTradeItem& item)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		m_listRegistrationItem.push_back(*m_listRegistrationItem.begin());
	for (std::list<sTradeItem>::iterator it = m_listRegistrationItem.begin();
		it != m_listRegistrationItem.end(); it++)
	{
		if ((*it).dwGuid == guid)
		{
			for (std::list<sTradeItem>::iterator it2 =
					 m_listTransferableItem.begin();
				it2 != m_listTransferableItem.end(); it2++)
			{
				if ((*it2).dwGuid == guid)
				{
					(*it2).iNum += (*it).iNum;
					break;
				}
			}

			item = *it;
			m_listRegistrationItem.erase(it);
			return true;
		}
	}

	return false;
}

bool CTransferableItem::IsRegisterationItem(unsigned long guid)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		m_listRegistrationItem.erase(m_listRegistrationItem.begin());
	for (std::list<sTradeItem>::iterator it = m_listRegistrationItem.begin();
		it != m_listRegistrationItem.end(); it++)
	{
		if ((*it).dwGuid == guid)
			return true;
	}

	return false;
}

std::list<sTradeItem>* CTransferableItem::GetList_RegistrationItem()
{
	return &m_listRegistrationItem;
}

void CTransferableItem::ResetRegistrationItemList()
{
	m_listRegistrationItem.clear();
	SetTransferableItemList();
}

int CTransferableItem::Get_RegistrationListSize()
{
	return m_listRegistrationItem.size();
}

bool CTransferableItem::DelRegistrationItem(unsigned long guid)
{
	sTradeItem item;

	if (m_listRegistrationItem.size() == 0)
		return false;

	if (IsRegisterationItem(guid, item))
	{
		m_listTransferableItem.sort(CompareTradeItem);
		return true;
	}

	return false;
}

bool CTransferableItem::DelRegistrationItem_itor_return(
	unsigned long guid, std::list<sTradeItem>::iterator* next)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		m_listRegistrationItem.push_back(*m_listRegistrationItem.begin());
	if (m_listRegistrationItem.size() == 0)
		return false;

	for (std::list<sTradeItem>::iterator it = m_listRegistrationItem.begin();
		it != m_listRegistrationItem.end(); it++)
	{
		if ((*it).dwGuid == guid)
		{
			for (std::list<sTradeItem>::iterator it2 =
					 m_listTransferableItem.begin();
				it2 != m_listTransferableItem.end(); it2++)
			{
				if ((*it2).dwGuid == guid)
				{
					(*it2).iNum += (*it).iNum;
					break;
				}
			}

			*next = m_listRegistrationItem.erase(it);
			return true;
		}
	}

	return false;
}

void CTransferableItem::SetMailInfoPage(sMailBoxListForPage& page)
{
	m_mailBoxPageList.m_lMailList.clear();
	m_mailBoxPageList = page;
}

void CTransferableItem::SetMailInfo(sMailInfo& info)
{
	m_mailInfo.m_lMailItemList.clear();
	m_mailInfo = info;
}

sMailBoxListForPage* CTransferableItem::GetMailBoxPageList()
{
	return &m_mailBoxPageList;
}

sMailInfo* CTransferableItem::GetMailInfo()
{
	return &m_mailInfo;
}

int CTransferableItem::GetNumTradingItem(unsigned long guid)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		m_listRegistrationItem.erase(m_listRegistrationItem.begin());
	int count = 0;
	for (std::list<sTradeItem>::iterator it = m_listRegistrationItem.begin();
		it != m_listRegistrationItem.end(); it++)
	{
		if ((*it).dwGuid == guid)
			count += (*it).iNum;
	}

	return count;
}

bool CTransferableItem::IsEquipParts(unsigned long uid, unsigned long tid)
{
	bool bFind = false;
	for (std::list<unsigned long>::iterator it = m_listEquipParts.begin();
		it != m_listEquipParts.end(); it++)
	{
		if (*it != 0 && tid == *it)
		{
			m_listEquipParts.erase(it);
			bFind = true;
			break;
		}
	}

	return bFind;
}

bool CTransferableItem::IsCheckCardParts(unsigned long uid, unsigned long tid)
{
	std::list<sSCardAvilityPeriodInfo>::iterator it;
	for (it = Doc()->m_cardAbilityList[0].begin();
		it != Doc()->m_cardAbilityList[0].end(); it++)
	{
		if ((*it).partsTid == tid && (*it).partsUid == uid)
			break;
	}

	if (it != Doc()->m_cardAbilityList[0].end())
		return false;

	return true;
}
