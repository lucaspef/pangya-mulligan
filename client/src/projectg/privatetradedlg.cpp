#include "minatl.h"
#include <iterator> // VC10: std::back_inserter
#include "privatetradedlg.h"
#include "fredit.h"
#include "fresh.h"
#include "frgraphicinterface.h"
#include "projectg.h"
#include "binstr.h"
#include "frlistbox.h"
#include "frwndinl.h"
#include "frbutton.h"
#include "frscrollbar.h"
#include "frdesktop.h"
#include "wtemplate.h"
#include <algorithm>


extern Fresh* g_pFresh;

struct sItemInfo;
struct sCards;

struct sWarpItem
{
	sWarpItem(const sWarpItem* item);
	sWarpItem(const sItemInfo& info);
	sWarpItem(const sCards& card);

	void SetIcon(unsigned long typeId);

	unsigned long typeId;
	unsigned long id;
	const Bitmap* pIcon;
	std::string name;
	WRect iconRect;
	int count;
};

sWarpItem::sWarpItem(const sWarpItem* pItem)
	: typeId(pItem->typeId),
	  id(pItem->id),
	  pIcon(pItem->pIcon),
	  name(pItem->name),
	  iconRect(pItem->iconRect),
	  count(pItem->count)
{
}

sWarpItem::sWarpItem(const sItemInfo& item)
	: typeId(item.tid), id(item.guid), pIcon(0)
{
	if (ItemManager()->IsDisplayItemNumber(typeId))
	{
		count = item.Common[0];
	}
	else
	{
		count = 1;
	}
	SetIcon(typeId);
}

sWarpItem::sWarpItem(const sCards& card)
	: typeId(card.tid), id(card.uid), pIcon(0)
{
	count = card.cardCount;
	SetIcon(typeId);
}

void sWarpItem::SetIcon(unsigned long typeId)
{
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(typeId);
	if (pCommon)
	{
		pIcon = g_pFresh->GetBitmap(pCommon->Icon);
		name = pCommon->Name;

		if (pIcon == 0)
		{
			pIcon = g_pFresh->GetBitmap("caddie_no");
			name = MakeStr("INVAID(%d)", typeId);
		}
	}

	if (pIcon)
	{
		iconRect.x = iconRect.y = 0.0f;
		iconRect.w = (float)pIcon->Width();
		iconRect.h = (float)pIcon->Height();
	}
}

// Outside the anonymous namespace: FrPrivateTradeWnd::OpenNumEditWnd refers to
// it by name from asm.
unsigned long GetMyItemCount(unsigned long itemId, unsigned long typeId);

void DrawPartsCardSlot(_sPrivateTradeItemSingle* pItem, const WRect& rect,
	float alpha, int bSmall);

template <class T>
T* FindPrivateTradeChildForm(const char* name)
{
	FrWnd* pTrade = g_pFresh->GetDesktop()->FindChildForm("FrPrivateTrade");
	if (pTrade)
	{
		FrWnd* pChild = pTrade->FindChildForm(name);
		if (pChild)
		{
			return DYNAMIC_CAST(T, pChild);
		}
	}
	return 0;
}

void Formator(const sTradeDropItem& item, _sPrivateTradeItemSingle& single);

// HACK: emission order only (see GetCardInfoText). A template body whose dead
// reference instantiates string::iterator::operator- one round after the
// non-template code; dllimport + __forceinline keeps it from being emitted.
template <class It>
__declspec(dllimport) __forceinline int RbIterDistance(It first, It last)
{
	return last - first;
}

namespace
{

	void SetListParts(std::vector<sWarpItem*>& items);
	void SetListClubSet(std::vector<sWarpItem*>& items);
	void SetListCard(std::vector<sWarpItem*>& items);
	void DoSortItem(std::vector<sWarpItem*>& items, eSetItemButton button,
		int sortType);
	void GetAblilityText(std::vector<std::string>& lines, short* pStats,
		short* pSlotStats);

	// The period cards put on the part (typeId, id); false when there are none
	// or the item is not a part.
	int GetCardsAttachedToPart(std::vector<sSCardAvilityPeriodInfo*>& cards,
		unsigned long typeId, unsigned long id)
	{
		if ((typeId & 0xfc000000) != 0x8000000)
		{
			return 0;
		}

		std::list<sSCardAvilityPeriodInfo>& cardList =
			Doc()->m_cardAbilityList[0];
		for (std::list<sSCardAvilityPeriodInfo>::iterator it = cardList.begin();
			it != cardList.end(); ++it)
		{
			sSCardAvilityPeriodInfo& card = *it;
			bool bMine = typeId == card.partsTid && id == card.partsUid;
			if (bMine)
			{
				cards.push_back(&*it);
			}
		}

		return !cards.empty();
	}

} // namespace

// Draws the card slots of a traded part: the character row above the
// caddie row, both at the bottom of the rect.
void DrawPartsCardSlot(_sPrivateTradeItemSingle* pItem, const WRect& rect,
	float alpha, int bSmall)
{
	if (pItem == 0)
	{
		return;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(pItem->typeId);
	if (pPart == 0)
	{
		return;
	}

	float bottom = rect.y + rect.h;
	const Bitmap* pBmp = 0;
	float x = 5.0f;
	for (int slot = 0; slot < pPart->CharacterSlot; ++slot)
	{
		bool bEmpty = true;
		for (int i = 0; i < 10; ++i)
		{
			if (pItem->cards[i].slot == slot + 1 && pItem->cards[i].kind == 0)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard =
					ItemManager()->FindCard(pItem->cards[i].typeId);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x,
						bottom - ((float)pBmp->Height() + 5.0f) * 2.0f,
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_character_empty");
			if (pBmp)
			{
				WRect r(rect.x + x,
					bottom - ((float)pBmp->Height() + 5.0f) * 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}

	x = 5.0f;
	for (int slot = 0; slot < pPart->CaddieSlot; ++slot)
	{
		bool bEmpty = true;
		for (int i = 0; i < 10; ++i)
		{
			if (pItem->cards[i].slot == slot + 1 && pItem->cards[i].kind == 1)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard =
					ItemManager()->FindCard(pItem->cards[i].typeId);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_caddie_empty");
			if (pBmp)
			{
				WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}

	// 983 008d3627 / 008d7713: 3a linha, slots NPC (kind 5), na mesma altura da do caddie
	x = 5.0f;
	for (int slot = 0; slot < pPart->NpcSlot983; ++slot)
	{
		bool bEmpty = true;
		for (int i = 0; i < 10; ++i)
		{
			if (pItem->cards[i].slot == slot + 1 && pItem->cards[i].kind == 5)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard =
					ItemManager()->FindCard(pItem->cards[i].typeId);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_npc_empty");
			if (pBmp)
			{
				WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}
}

// How many of the item (itemId, typeId) the player holds; 1 for an item
// that is not counted.
unsigned long GetMyItemCount(unsigned long itemId, unsigned long typeId)
{
	if (ItemManager()->IsDisplayItemNumber(typeId))
	{
		switch (typeId >> 26)
		{
		case 6:
		{
			CSharedDoc* pDoc = Doc();
			std::list<sItemInfo>::iterator it =
				std::find_if(pDoc->m_myItemList.begin(),
					pDoc->m_myItemList.end(), _FindByGuid(itemId));
			if (it != pDoc->m_myItemList.end())
			{
				return it->Common[0];
			}
			break;
		}
		case 0x1f:
		{
			CSharedDoc* pDoc = Doc();
			std::list<sCards>::iterator it =
				std::find_if(pDoc->m_cardList.begin(), pDoc->m_cardList.end(),
					_FindByUid(itemId));
			if (it != pDoc->m_cardList.end())
			{
				return (*it).cardCount;
			}
			break;
		}
		case 0x1c:
		{
			CSharedDoc* pDoc = Doc();
			std::list<sItemInfo>::iterator it =
				std::find_if(pDoc->m_auxPartList.begin(),
					pDoc->m_auxPartList.end(), _FindByGuid(itemId));
			if (it != pDoc->m_auxPartList.end())
			{
				return it->Common[0];
			}
			break;
		}
		}
	}

	return 1;
}

namespace
{

	// The card's name and the first line of its description.
	std::string GetCardInfoText(unsigned long typeId)
	{
		std::string text;
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(typeId);
		if (pCommon == 0)
		{
			text += "unknown card";
		}
		else
		{
			text += pCommon->Name;
		}

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(typeId);
		if (pDesc)
		{
			text += " : ";
			std::string desc(pDesc->Desc);
			std::string::size_type pos = desc.find('\n');
			if (pos != std::string::npos)
			{
				// HACK: emission order only (no code). The original instantiates
				// _Iter_cat<string::iterator> in the first round and
				// iterator::operator- in the second, before the range
				// constructor's own chain reaches them.
				if (0)
				{
#if _MSC_VER == 1600 // D740: _Iter_cat so existe no STL do VC10
					std::_Iter_cat(desc.begin());
#endif
					pos = RbIterDistance(desc.begin(), desc.end());
				}
				text += std::string(desc.begin(), desc.begin() + pos);
			}
			else
			{
				text += desc;
			}
		}

		return text;
	}

} // namespace

void Formator(const sTradeDropItem& item, _sPrivateTradeItemSingle& single)
{
	single.typeId = item.typeId;
	single.itemId = item.itemId;
	single.count = item.count;

	switch (single.typeId >> 26)
	{
	case 2:
	{
		std::vector<sSCardAvilityPeriodInfo*> cards;
		if (GetCardsAttachedToPart(cards, single.typeId, single.itemId))
		{
			int i = 0;
			for (std::vector<sSCardAvilityPeriodInfo*>::iterator it =
					 cards.begin();
				it != cards.end(); ++it)
			{
				if (i >= 10) // 983: cmp edi, 0xa
				{
					break;
				}

				sSCardAvilityPeriodInfo* pCard = *it;
				single.cards[i].id = pCard->uid;
				single.cards[i].typeId = pCard->tid;
				single.cards[i].effect = pCard->Avility;
				single.cards[i].effectValue = pCard->AvilityValue;
				single.cards[i].kind = pCard->cardType;
				single.cards[i].slot = pCard->slotNum;
				i++;
			}
		}
		break;
	}
	case 4:
	{
		CSharedDoc* pDoc = Doc();
		std::map<unsigned int, sItemInfo>::iterator it =
			pDoc->m_clubSetMap.find(single.itemId);
		if (it != pDoc->m_clubSetMap.end())
		{
			for (int i = 0; i < 5; i++)
				single.slotStats[i] = it->second.Common[i];
		}
		break;
	}
	}
}

namespace
{

} // namespace

// Same as DrawPartsCardSlot, for a part of the player's own warehouse: the
// cards come from the period card list.
void DrawMyPartsCardSlot(unsigned long typeId, unsigned long id,
	const WRect& rect, float alpha, int bSmall)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(typeId);
	CSharedDoc* pDoc = Doc();
	std::list<sItemInfo>::iterator itItem =
		std::find_if(pDoc->m_partsList.begin(), pDoc->m_partsList.end(),
			_FindItem(typeId, id));
	if (pPart == 0 || itItem == pDoc->m_partsList.end())
	{
		return;
	}

	std::vector<sSCardAvilityPeriodInfo*> cards;
	GetCardsAttachedToPart(cards, typeId, id);

	const Bitmap* pBmp = 0;
	float bottom = rect.y + rect.h;
	float x = 5.0f;
	for (int slot = 0; slot < pPart->CharacterSlot; ++slot)
	{
		bool bEmpty = true;
		for (std::vector<sSCardAvilityPeriodInfo*>::iterator it = cards.begin();
			it != cards.end(); ++it)
		{
			if ((*it)->slotNum == slot + 1 && (*it)->cardType == 0)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard((*it)->tid);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x,
						bottom - ((float)pBmp->Height() + 5.0f) * 2.0f,
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_character_empty");
			if (pBmp)
			{
				WRect r(rect.x + x,
					bottom - ((float)pBmp->Height() + 5.0f) * 2.0f,
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}

	x = 5.0f;
	for (int slot = 0; slot < pPart->CaddieSlot; ++slot)
	{
		bool bEmpty = true;
		for (std::vector<sSCardAvilityPeriodInfo*>::iterator it = cards.begin();
			it != cards.end(); ++it)
		{
			if ((*it)->slotNum == slot + 1 && (*it)->cardType == 1)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard((*it)->tid);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_caddie_empty");
			if (pBmp)
			{
				WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}

	// 983 008d3627 / 008d7713: 3a linha, slots NPC (kind 5), na mesma altura da do caddie
	x = 5.0f;
	for (int slot = 0; slot < pPart->NpcSlot983; ++slot)
	{
		bool bEmpty = true;
		for (std::vector<sSCardAvilityPeriodInfo*>::iterator it = cards.begin();
			it != cards.end(); ++it)
		{
			if ((*it)->slotNum == slot + 1 && (*it)->cardType == 5)
			{
				bEmpty = false;
				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard((*it)->tid);
				if (pCard)
				{
					pBmp = g_pFresh->GetBitmap(pCard->SlotImg);
				}
				if (pBmp)
				{
					WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
						(float)pBmp->Width(), (float)pBmp->Height());
					pGDI->DrawTexture(pBmp, r,
						((unsigned long)(unsigned char)alpha << 24) | 0xffffff,
						0);
					x += (float)pBmp->Width() + 3.0f;
					break;
				}
			}
		}
		if (bSmall && bEmpty)
		{
			pBmp = g_pFresh->GetBitmap("card_npc_empty");
			if (pBmp)
			{
				WRect r(rect.x + x, bottom - ((float)pBmp->Height() + 5.0f),
					(float)pBmp->Width(), (float)pBmp->Height());
				pGDI->DrawTexture(pBmp, r,
					((unsigned long)(unsigned char)alpha << 24) | 0xffffff, 0);
				x += (float)pBmp->Width() + 3.0f;
			}
		}
	}
}

namespace
{

	template <class T>
	struct less_gana_ptr : public std::binary_function<T, T, bool>
	{
		bool operator()(const T a, const T b) const
		{
			IFF_ITEM_COMMON* pCommonA =
				ItemManager()->FindCommonItem(a->typeId);
			IFF_ITEM_COMMON* pCommonB =
				ItemManager()->FindCommonItem(b->typeId);
			if (pCommonA == NULL || pCommonB == NULL)
				return false;

			std::string nameA(pCommonA->Name);
			std::string nameB(pCommonB->Name);
			return nameA < nameB;
		}
	};

	bool IsSalable(const sItemInfo& item)
	{
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(item.tid);
		if (pCommon)
		{
			return pCommon->IsSalable != 0;
		}

		return false;
	}

	bool IsEquipParts(const sItemInfo& item)
	{
		std::map<unsigned int, sCharacterInfo>& charMap = Doc()->m_charMap;
		for (std::map<unsigned int, sCharacterInfo>::iterator it =
				 charMap.begin();
			it != charMap.end(); ++it)
		{
			int i;
			for (i = 0; i < 24; i++)
			{
				if (it->second.tidParts[i] != 0 &&
					item.guid == it->second.ItemIdList[i])
					return true;
			}

			for (i = 0; i < 5; i++)
			{
				if (it->second.tidAuxParts[i] != 0 &&
					item.tid == it->second.tidAuxParts[i])
					return true;
			}
		}

		return false;
	}

	void SetListParts(std::vector<sWarpItem*>& items)
	{
		std::list<sItemInfo>& partsList = Doc()->m_partsList;
		for (std::list<sItemInfo>::iterator it = partsList.begin();
			it != partsList.end(); ++it)
		{
			if (!IsEquipParts(*it))
			{
				if (IsSalable(*it))
				{
					items.push_back(new sWarpItem(*it));
				}
			}
		}

		std::list<sItemInfo>& auxPartList = Doc()->m_auxPartList;
		for (std::list<sItemInfo>::iterator it2 = auxPartList.begin();
			it2 != auxPartList.end(); ++it2)
		{
			if (!IsEquipParts(*it2))
			{
				if (IsSalable(*it2))
				{
					items.push_back(new sWarpItem(*it2));
				}
			}
		}
	}

	void SetListAztec(std::vector<sWarpItem*>& items)
	{
		unsigned long ballId = Doc()->m_myInfo.userEquip.tidBall;
		std::list<sItemInfo>& ballList = Doc()->m_ballList;

		items.reserve(ballList.size());

		for (std::list<sItemInfo>::iterator it = ballList.begin();
			it != ballList.end(); ++it)
		{
			if (it->tid != ballId)
			{
				if (IsSalable(*it))
				{
					sWarpItem* pItem = new sWarpItem(*it);
					items.push_back(pItem);
				}
			}
		}
	}

	void SetListClubSet(std::vector<sWarpItem*>& items)
	{
		unsigned long clubSetId = Doc()->m_myInfo.userEquip.guidClubSet;
		std::map<unsigned int, sItemInfo>& clubSetMap = Doc()->m_clubSetMap;

		items.reserve(clubSetMap.size());

		for (std::map<unsigned int, sItemInfo>::iterator it =
				 clubSetMap.begin();
			it != clubSetMap.end(); ++it)
		{
			if (it->second.guid != clubSetId)
			{
				if (IsSalable(it->second))
				{
					items.push_back(new sWarpItem(it->second));
				}
			}
		}
	}

	void SetListHItem(std::vector<sWarpItem*>& items)
	{
		std::list<sItemInfo>& itemList = Doc()->m_myItemList;
		for (std::list<sItemInfo>::iterator it = itemList.begin();
			it != itemList.end(); ++it)
		{
			if ((it->tid & 0x2000000) != 0)
			{
				if (IsSalable(*it))
				{
					sWarpItem* pItem = new sWarpItem(*it);
					items.push_back(pItem);
				}
			}
		}
	}

	void SetListUitem(std::vector<sWarpItem*>& items)
	{
		std::list<sItemInfo>& itemList = Doc()->m_myItemList;
		for (std::list<sItemInfo>::iterator it = itemList.begin();
			it != itemList.end(); ++it)
		{
			if ((it->tid & 0x2000000) == 0)
			{
				if (IsSalable(*it))
				{
					sWarpItem* pItem = new sWarpItem(*it);
					items.push_back(pItem);
				}
			}
		}
	}

	void SetListCard(std::vector<sWarpItem*>& items)
	{
		std::list<sCards>& cardList = Doc()->m_cardList;

		items.reserve(cardList.size());

		for (std::list<sCards>::iterator it = cardList.begin();
			it != cardList.end(); ++it)
		{
			IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem((*it).tid);
			if (pCommon != 0 && pCommon->IsSalable != 0)
			{
				sWarpItem* pItem = new sWarpItem(*it);
				items.push_back(pItem);
			}
		}
	}

	void DoSortItem(std::vector<sWarpItem*>& items, eSetItemButton button,
		int sortType)
	{
		switch (button)
		{
		case SET_ITEM_BUTTON_PART:
		{
			// sortType is the character here: drop the parts of the others.
			std::vector<sWarpItem*>::iterator it = items.begin();
			while (it != items.end())
			{
				unsigned long typeId = (*it)->typeId;
				if ((typeId >> 26 == 2 || typeId >> 26 == 0x1c) &&
					((typeId >> 18) & 0xff) != sortType)
				{
					delete *it;
					it = items.erase(it);
				}
				else
				{
					++it;
				}
			}

			std::sort(items.begin(), items.end(), less_gana_ptr<sWarpItem*>());
			break;
		}
		case SET_ITEM_BUTTON_CLUB:
		case SET_ITEM_BUTTON_CARD:
			switch (sortType)
			{
			case 0:
			case 1:
				std::sort(items.begin(), items.end(),
					less_guid_ptr<sWarpItem*>());
				break;
			case 2:
				std::sort(items.begin(), items.end(),
					less_gana_ptr<sWarpItem*>());
				break;
			}
			break;
		case SET_ITEM_BUTTON_ITEM:
		{
			std::vector<sWarpItem*> uItems;
			std::vector<sWarpItem*> hItems;
			for (std::vector<sWarpItem*>::iterator it = items.begin();
				it != items.end(); ++it)
			{
				if (((*it)->typeId & 0xfc000000) == 0x18000000)
				{
					sWarpItem* pItem = new sWarpItem(*it);
					if ((*it)->typeId & 0x2000000)
					{
						hItems.push_back(pItem);
					}
					else
					{
						uItems.push_back(pItem);
					}
				}
			}

			std::for_each(items.begin(), items.end(), Delete_Object());
			items.clear();

			switch (sortType)
			{
			case 0:
				std::sort(uItems.begin(), uItems.end(),
					less_guid_ptr<sWarpItem*>());
				std::sort(hItems.begin(), hItems.end(),
					less_guid_ptr<sWarpItem*>());
				items.assign(uItems.begin(), uItems.end());
				std::copy(hItems.begin(), hItems.end(),
					std::back_inserter(items));
				break;
			case 3:
				std::sort(uItems.begin(), uItems.end(),
					less_guid_ptr<sWarpItem*>());
				items.assign(uItems.begin(), uItems.end());
				std::for_each(hItems.begin(), hItems.end(), Delete_Object());
				hItems.clear();
				break;
			case 4:
				std::sort(uItems.begin(), uItems.end(),
					less_gana_ptr<sWarpItem*>());
				items.assign(uItems.begin(), uItems.end());
				std::for_each(hItems.begin(), hItems.end(), Delete_Object());
				hItems.clear();
				break;
			case 5:
				std::sort(hItems.begin(), hItems.end(),
					less_guid_ptr<sWarpItem*>());
				items.assign(hItems.begin(), hItems.end());
				std::for_each(uItems.begin(), uItems.end(), Delete_Object());
				uItems.clear();
				break;
			case 6:
				std::sort(hItems.begin(), hItems.end(),
					less_gana_ptr<sWarpItem*>());
				items.assign(hItems.begin(), hItems.end());
				std::for_each(uItems.begin(), uItems.end(), Delete_Object());
				uItems.clear();
				break;
			}
			break;
		}
		}
	}

	template <class T>
	void GetIconNameDesc(const Bitmap*& pIcon, std::string& name,
		std::string& desc, const T* pItem)
	{
		if (pItem)
		{
			pIcon = g_pFresh->GetBitmap(pItem->c.Icon);
			name = pItem->c.Name;
			IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pItem->c.TypeId);
			if (pDesc)
			{
				desc = pDesc->Desc;
			}
		}
	}

	// One line per stat (power, control, accuracy, spin, curve): the base value
	// and the slot upgrades.
	void GetAblilityText(std::vector<std::string>& lines, short* pStats,
		short* pSlotStats)
	{
		const char* names[5] = { K2L_Compatibility("\xc6\xc4\xbf\xf6"), K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1"),
			K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5"), K2L_Compatibility("\xbd\xba\xc7\xc9"),
			K2L_Compatibility("\xc4\xbf\xba\xea") };

		for (int i = 0; i < 5; i++)
		{
			std::string text;
			if (pStats[i] != 0)
			{
				text = MakeStr("%s +%d", names[i], pStats[i]);
			}

			if (pSlotStats[i] != 0)
			{
				if (text.empty())
				{
					text = MakeStr("%s \xbd\xbd\xb7\xd4 +%d", names[i],
						pSlotStats[i]);
				}
				else
				{
					text += MakeStr(" / %s \xbd\xbd\xb7\xd4 +%d", names[i],
						pSlotStats[i]);
				}
			}

			if (!text.empty())
			{
				lines.push_back(text);
			}
		}
	}

}

CTradeDragDrop* CTradeDragDrop::m_pCaller = NULL;
sTradeDropItem* CTradeDragDrop::m_pItem = NULL;

CTradeDragDrop::CTradeDragDrop()
{
	if (m_pItem == NULL)
		m_pItem = new sTradeDropItem();
}

CTradeDragDrop::~CTradeDragDrop()
{
	if (m_pItem)
	{
		delete m_pItem;
		m_pItem = NULL;
	}
}

const sTradeDropItem* CTradeDragDrop::GetDropItem()
{
	if (!IsPoped())
		return NULL;

	return m_pItem;
}

bool CTradeDragDrop::IsPoped()
{
	return m_pCaller != NULL && m_pItem->typeId != 0 && m_pItem->itemId != 0;
}

void CTradeDragDrop::Cancel()
{
	m_pCaller = NULL;
}

bool CTradeDragDrop::Pop()
{
	m_pCaller = this;
	return true;
}

void CTradeDragDrop::SetPopItem(const sTradeDropItem& item)
{
	*m_pItem = item;
}

void CTradeDragDrop::Drop(CTradeDragDrop* target)
{
	if (m_pCaller && target)
	{
		if (m_pCaller != target)
		{
			if (target->DropItemDown())
				m_pCaller->DropItemMoved();
		}
	}

	m_pCaller = NULL;
	memset(m_pItem, 0, sizeof(sTradeDropItem));
}

// 983 (cliente japonês): 1º byte de caractere duplo em Shift-JIS (0x80-0x9f, 0xe0-0xfc); no 645 era `c & 0x80`. Nome provisório
// (mesma macro de src/engine/wfont.cpp).
#ifndef W_SJISLEAD983
#define W_SJISLEAD983(c) (((unsigned char)(c) >= 0x80 && (unsigned char)(c) < 0xa0) || (unsigned char)((c) - 0xe0) <= 0x1c)
#endif

IMPLEMENT_OBJECT(FrNumEdit, FrForm)

BEGIN_FRESH_MSGMAP(FrNumEdit, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrNumEdit::OnViewDraw)
ON_FRESH_VI("NumEdit", FRCMD_INIT, FrNumEdit::OnEditInit)
ON_FRESH_BI("NumEdit", FRCMD_ENTERKEY, FrNumEdit::OnEditEnter)

END_FRESH_MSGMAP()

FrNumEdit::FrNumEdit()
	: m_pEdit(NULL),
	  m_value(0),
	  m_returnParam(NULL),
	  m_min(0),
	  m_max(0x7fffffff)
{
}

FrNumEdit::~FrNumEdit()
{
}

void FrNumEdit::SetValue(unsigned long value)
{
	SAFE_RANGEVALUE(m_value, m_min, m_max, value);

	char buf[32] = "";
	m_pEdit->SetLine(1, itoa(m_value, buf, 10), 0, false, 0);
}

void FrNumEdit::SetEditRange(unsigned long min, unsigned long max)
{
	char buf[32] = "";
	itoa(max, buf, 10);
	m_pEdit->SetCharLimit(strlen(buf) + 1, false);
	m_max = max;
	m_min = min;
}

void FrNumEdit::OnProc(const float deltaTime)
{
	if (m_pEdit->m_nFlags.GetFlag(0x10))
	{
		unsigned long value = 0;
		const char* text = m_pEdit->GetLine(1, false);
		const char* digits = "0123456789";
		std::string number;

		for (unsigned int i = 0; i < strlen(text); i++)
		{
			if (!W_SJISLEAD983(text[i]))
			{
				for (unsigned int j = 0; j < strlen(digits); j++)
				{
					if (text[i] == digits[j])
					{
						number += text[i];
						break;
					}
				}
			}
		}

		if (number.length() > 0)
		{
			value = atoi(number.c_str());
			if (value < m_min)
				value = m_min;
			if (value > m_max)
				value = m_max;
		}

		char buf[32] = "";
		itoa(value, buf, 10);
		m_pEdit->ForcedEdit(buf, 0);
	}
}

void FrNumEdit::OnViewDraw()
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* bitmap = g_pFresh->GetBitmap("cfrm00");
	gdi->DrawTexture(bitmap,
		WRect(0.0f, 0.0f, (float)bitmap->Width(), (float)bitmap->Height()),
		m_rect, 0xffffffff, 0);
}

void FrNumEdit::OnEditInit(int param)
{
	m_pEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	char buf[32] = "";
	itoa(0x7fffffff, buf, 10);
	m_pEdit->SetCharLimit(strlen(buf) + 1, false);
	m_pEdit->SetWidthLimit(m_pEdit->GetRect().w);

	if (m_value != 0)
	{
		char line[32] = "";
		m_pEdit->SetLine(1, itoa(m_value, line, 10), 0, false, 0);
	}
}

bool FrNumEdit::OnEditEnter(int param)
{
	int value = 0;
	if (param != 0 && strlen((const char*)param) != 0)
		value = atoi((const char*)param);

	SAFE_RANGEVALUE(m_value, m_min, m_max, (unsigned long)value);

	return FrForm::Close(FrOK, true);
}

IMPLEMENT_OBJECT(FrPangEdit, FrForm)

BEGIN_FRESH_MSGMAP(FrPangEdit, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrPangEdit::OnViewDraw)
ON_FRESH_VI("EditPang", FRCMD_INIT, FrPangEdit::OnEditInit)
ON_FRESH_BI("EditPang", FRCMD_ENTERKEY, FrPangEdit::OnEditEnter)

END_FRESH_MSGMAP()

FrPangEdit::FrPangEdit()
	: m_pEdit(NULL), m_pang(0), m_texRate(0)
{
}

FrPangEdit::~FrPangEdit()
{
}

void FrPangEdit::SetPangValue(__int64 pang)
{
	m_pang = pang;
	if (pang != 0)
	{
		char buf[32] = "";
		m_pEdit->SetLine(1, _i64toa(pang, buf, 10), 0, false, 0);
	}
}

void FrPangEdit::OnProc(const float deltaTime)
{
	if (m_pEdit->m_nFlags.GetFlag(0x10))
	{
		const char* text = m_pEdit->GetLine(1, false);
		const char* digits = "0123456789";
		std::string number;
		bool bInvalid = false;

		for (unsigned int i = 0; i < strlen(text); i++)
		{
			if (!W_SJISLEAD983(text[i]))
			{
				unsigned int j;
				for (j = 0; j < strlen(digits); j++)
				{
					if (text[i] == digits[j])
					{
						number += text[i];
						break;
					}
				}

				if (j == strlen(digits))
					bInvalid = true;
			}
		}

		if (bInvalid)
			m_pEdit->ForcedEdit(number.c_str(), 0);

		if (number.length() > 0)
		{
			__int64 pang = _atoi64(number.c_str());
			if (pang <= 0)
				return;

			// The pang plus its tax may not exceed what the player has.
			__int64 total = m_texRate * pang / 100 + pang;
			if (Doc()->m_myInfo.stat.i64Pang <= total)
			{
				__int64 limit =
					Doc()->m_myInfo.stat.i64Pang * 100 / (m_texRate + 100);
				char buf[32] = "";
				_i64toa(limit, buf, 10);
				m_pEdit->ForcedEdit(buf, 0);
			}
		}
	}
}

void FrPangEdit::OnViewDraw()
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* bitmap = g_pFresh->GetBitmap("cfrm00");
	WRect src(0.0f, 0.0f, (float)bitmap->Width(), (float)bitmap->Height());
	gdi->DrawTexture(bitmap, src, m_rect, 0xffffffff, 0);

	const Bitmap* icon = g_pFresh->GetBitmap("pang_bar_pang");
	if (icon)
	{
		gdi->DrawTexture(icon,
			WRect(m_rect.x + 10.0f, m_rect.y + 5.0f, (float)icon->Width(),
				(float)icon->Height()),
			0xffffffff, 0);
	}
}

void FrPangEdit::OnEditInit(int param)
{
	m_pEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	char buf[32] = "";
	_i64toa(Doc()->m_myInfo.stat.i64Pang, buf, 10);
	m_pEdit->SetCharLimit(strlen(buf) + 1, false);
	m_pEdit->SetWidthLimit(m_pEdit->GetRect().w);

	if (m_pang != 0)
	{
		char line[32] = "";
		m_pEdit->SetLine(1, _i64toa(m_pang, line, 10), 0, false, 0);
	}
}

bool FrPangEdit::OnEditEnter(int param)
{
	if (param != 0 && strlen((const char*)param) != 0)
		m_pang = _atoi64((const char*)param);
	else
		m_pang = 0;

	return FrForm::Close(FrOK, true);
}

EncapsulatedPang::EncapsulatedPang()
	: m_pang(0), m_pRect(NULL), m_style(STYLE_BAR | STYLE_ICON)
{
}

EncapsulatedPang::~EncapsulatedPang()
{
}

void EncapsulatedPang::SetRect(const WRect& rect)
{
	if (m_pRect == NULL)
		m_pRect = new WRect(rect);
	else
		*m_pRect = rect;
}

int EncapsulatedPang::IsStyle(eStyle style)
{
	return m_style & style;
}

int EncapsulatedPang::SubPang(__int64 pang)
{
	__int64 left = m_pang - pang;
	if (left < 0)
		return 0;

	m_pang = left;
	return 1;
}

void EncapsulatedPang::AddPangBarStyle(eStyle style)
{
	m_style |= style;
}

void EncapsulatedPang::DrawPang()
{
	FrWndManager* manager = g_pFresh->GetManager();
	FrGraphicInterface* gdi = manager->GetGDI();
	if (gdi && m_pRect)
	{
		float w = m_pRect->w;
		float x = m_pRect->x;
		float y = m_pRect->y;
		unsigned long color = 0xffffffff;
		unsigned char alpha = 0xff;

		if ((m_style & STYLE_HOVER) &&
			m_pRect->IsInRect(manager->GetMousePos()))
		{
			alpha = 0xa0;
			color = 0xa000ff00;
		}

		if (m_style & STYLE_BAR)
		{
			const Bitmap* left = g_pFresh->GetBitmap("pang_bar_01");
			const Bitmap* middle = g_pFresh->GetBitmap("pang_bar_02");
			const Bitmap* right = g_pFresh->GetBitmap("pang_bar_03");
			if (left && middle && right)
			{
				float leftW = (float)left->Width();
				float middleW = w - (float)right->Width() - leftW;
				float middleX = leftW + x;

				gdi->DrawTexture(left,
					WRect(x, y, (float)left->Width(), (float)left->Height()),
					color, 0);
				gdi->DrawTexture(middle,
					WRect(middleX, y, middleW, (float)middle->Height()), color,
					0);
				gdi->DrawTexture(right,
					WRect(middleX + middleW, y, (float)right->Width(),
						(float)right->Height()),
					color, 0);
			}
		}

		if (m_style & STYLE_ICON)
		{
			const Bitmap* icon = g_pFresh->GetBitmap("pang_bar_pang");
			if (icon)
			{
				gdi->DrawTexture(icon,
					WRect(x + 10.0f, y + 5.0f, (float)icon->Width(),
						(float)icon->Height()),
					color, 0);
			}
		}

		x = x + m_pRect->w - 20.0f;
		y = m_pRect->y + 5.0f;

		char price[64] = "";
		MakePriceStrA(price, m_pang);
		x -= GetStringWidth(price);
		unsigned long textColor = (unsigned char)(alpha * 0xff >> 8);
		textColor = textColor << 8 | (unsigned char)(alpha * 0xff >> 8);
		textColor = textColor << 8 | (unsigned char)(alpha * 0xd1 >> 8);
		textColor = textColor << 8 | (unsigned char)(alpha * 0x8e >> 8);
		m_pFont->Print(g_view, x, y, price, 0, textColor, NULL);
	}
}

IMPLEMENT_OBJECT(FrMyWareHouse, FrForm)

BEGIN_FRESH_MSGMAP(FrMyWareHouse, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrMyWareHouse::OnViewDraw)
ON_FRESH_VI("ItemList", FRCMD_INIT, FrMyWareHouse::OnListInit)
ON_FRESH_VI("ItemList", FRCMD_OWNERDRAW, FrMyWareHouse::OnListDraw)
ON_FRESH_VV("ItemList", FRCMD_DBLCLICK, FrMyWareHouse::OnListDblClick)
ON_FRESH_VI("SortItem", FRCMD_INIT, FrMyWareHouse::OnSortInit)
ON_FRESH_VI("SortItem", FRCMD_OWNERDRAW, FrMyWareHouse::OnSortDraw)
ON_FRESH_VI("SortItem", FRCMD_LBUTTONUP, FrMyWareHouse::OnSortDown)
ON_FRESH_VI("PartItem", FRCMD_INIT, FrMyWareHouse::OnPartBtnInit)
ON_FRESH_VI("ClubItem", FRCMD_INIT, FrMyWareHouse::OnClubBtnInit)
ON_FRESH_VI("WareItem", FRCMD_INIT, FrMyWareHouse::OnItemBtnInit)
ON_FRESH_VI("CardItem", FRCMD_INIT, FrMyWareHouse::OnCardBtnInit)
ON_FRESH_VV("PartItem", FRCMD_LBUTTONUP, FrMyWareHouse::OnPartBtnDown)
ON_FRESH_VV("ClubItem", FRCMD_LBUTTONUP, FrMyWareHouse::OnClubBtnDown)
ON_FRESH_VV("WareItem", FRCMD_LBUTTONUP, FrMyWareHouse::OnItemBtnDown)
ON_FRESH_VV("CardItem", FRCMD_LBUTTONUP, FrMyWareHouse::OnCardBtnDown)

END_FRESH_MSGMAP()

FrMyWareHouse::FrMyWareHouse()
	: m_pItemList(NULL), m_pSortList(NULL), m_curButton(SET_ITEM_BUTTON_PART)
{
	back_insert_null(m_buttons, 4);
}

FrMyWareHouse::~FrMyWareHouse()
{
	std::for_each(m_warpItems.begin(), m_warpItems.end(), Delete_Object());
}

bool FrMyWareHouse::OnInit()
{
	WRect rect = m_rect;
	rect.x += 15.0f;
	rect.y += 80.0f;
	rect.w -= 30.0f;
	rect.h = 20.0f;
	EncapsulatedPang::SetRect(rect);

	SetPang(Doc()->m_myInfo.stat.i64Pang);
	return FrForm::OnInit();
}

void FrMyWareHouse::OnProc(const float deltaTime)
{
	FrForm::OnProc(deltaTime);

	WRect rect = m_pItemList->GetScrollBar()->GetRect();
	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()) && IsPoped())
		Cancel();
}

bool FrMyWareHouse::DropItemUp()
{
	return true;
}

bool FrMyWareHouse::DropItemDown()
{
	const sTradeDropItem* pItem = GetDropItem();
	if (pItem == NULL)
		return false;
	if (pItem->typeId == 0 && pItem->itemId == 0)
		return false;

	switch (pItem->typeId >> 26)
	{
	case 2:
	case 0x1c:
		UnPushedButtonAll();
		m_buttons[SET_ITEM_BUTTON_PART]->SetStatus(FrButton::PRESSED);
		SetItemList(SET_ITEM_BUTTON_PART,
			(eSetSortType)((pItem->typeId >> 18) & 0xff));
		break;
	case 4:
		OnClubBtnDown();
		break;
	case 6:
		OnItemBtnDown();
		break;
	case 0x1f:
		OnCardBtnDown();
		break;
	default:
		return false;
	}

	std::vector<sWarpItem*>::iterator it = std::find_if(m_warpItems.begin(),
		m_warpItems.end(), _FindByGuid_Ptr(pItem->itemId));
	if (it == m_warpItems.end())
		return false;

	FrListItem* pListItem = FindListItem(pItem->itemId, pItem->typeId);
	if (pListItem)
	{
		m_pItemList->DelItem(pListItem);
		m_pItemList->AddItem(*it);
	}
	else
	{
		m_pItemList->AddItem(*it);
	}
	m_pItemList->GetScrollBar()->ScrollToBottom();

	return true;
}

void FrMyWareHouse::DropItemMoved()
{
	const sTradeDropItem* item = GetDropItem();
	if (ItemManager()->IsDisplayItemNumber(item->typeId))
		SubListItemCount(item->itemId, item->typeId, 1);
	else
		EraseListItem(item->itemId, item->typeId);
}

FrListItem* FrMyWareHouse::FindListItem(unsigned long guid,
	unsigned long typeId)
{
	int size = m_pItemList->GetCurrentItemSize(true);
	for (int i = 0; i < size; i++)
	{
		FrListItem* item = m_pItemList->GetItem(i);
		if (item == NULL)
			continue;

		sWarpItem* warp = (sWarpItem*)item->pData;
		if (warp && warp->id == guid && warp->typeId == typeId)
			return item;
	}

	return NULL;
}

bool FrMyWareHouse::SubListItemCount(unsigned long guid, unsigned long typeId,
	int count)
{
	FrListItem* item = FindListItem(guid, typeId);
	if (item)
	{
		sWarpItem* warp = (sWarpItem*)item->pData;
		int remain = warp->count - count;
		if (remain >= 0)
		{
			warp->count = remain;
			if (remain == 0)
				m_pItemList->DelItem(item);
			return true;
		}
	}

	return false;
}

bool FrMyWareHouse::AddListItemCount(unsigned long guid, unsigned long typeId,
	int count)
{
	FrListItem* item = FindListItem(guid, typeId);
	if (item)
	{
		((sWarpItem*)item->pData)->count += count;
		return true;
	}

	return false;
}

int FrMyWareHouse::GetListItemCount(unsigned long guid, unsigned long typeId)
{
	FrListItem* item = FindListItem(guid, typeId);
	if (item)
		return ((sWarpItem*)item->pData)->count;

	return 0;
}

bool FrMyWareHouse::SetListItem(unsigned long guid, unsigned long typeId,
	int count)
{
	FrListItem* item = FindListItem(guid, typeId);
	if (item)
	{
		sWarpItem* warp = (sWarpItem*)item->pData;
		if (warp)
		{
			if (count == 0)
				EraseListItem(guid, typeId);
			else
				warp->count = count;

			return true;
		}
	}
	else if (count > 0)
	{
		return AddListItem(guid, typeId, count);
	}

	return false;
}

bool FrMyWareHouse::AddListItem(unsigned long guid, unsigned long typeId,
	int count)
{
	if (FindListItem(guid, typeId))
		return false;

	for (std::vector<sWarpItem*>::iterator it = m_warpItems.begin();
		it != m_warpItems.end(); ++it)
	{
		sWarpItem* warp = *it;
		for (int i = 0; i < 4; i++)
		{
			if (typeId == warp->typeId && guid == warp->id)
			{
				warp->count = count;
				m_pItemList->AddItem(*it);
				m_pItemList->GetScrollBar()->ScrollToBottom();
				break;
			}
		}
	}

	return true;
}

bool FrMyWareHouse::EraseListItem(unsigned long guid, unsigned long typeId)
{
	FrListItem* item = FindListItem(guid, typeId);
	if (item)
	{
		m_pItemList->DelItem(item);
		return true;
	}

	return false;
}

void FrMyWareHouse::InitSortList(eSetItemButton button)
{
	if (m_pSortList == NULL)
		return;

	m_pSortList->ClearItem();
	switch (button)
	{
	case SET_ITEM_BUTTON_PART:
		for (int i = 0; i < 15; i++) // 983: 15 personagens
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem(i | 0x4000000);
			if (common)
				m_pSortList->AddItem(
					(void*)g_pFresh->GetBitmap(MakeStr("s_%s", common->Icon)));
		}
		break;
	}
}

void FrMyWareHouse::OnPartBtnInit(int param)
{
	SetButtonStyle(param, SET_ITEM_BUTTON_PART);
}

void FrMyWareHouse::OnClubBtnInit(int param)
{
	SetButtonStyle(param, SET_ITEM_BUTTON_CLUB);
}

void FrMyWareHouse::OnItemBtnInit(int param)
{
	SetButtonStyle(param, SET_ITEM_BUTTON_ITEM);
}

void FrMyWareHouse::OnCardBtnInit(int param)
{
	SetButtonStyle(param, SET_ITEM_BUTTON_CARD);
}

void FrMyWareHouse::OnSortInit(int param)
{
	m_pSortList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	InitSortList(m_curButton);
}

void FrMyWareHouse::OnSortDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item == NULL)
		return;

	const Bitmap* bitmap = (const Bitmap*)item->pData;
	if (bitmap == NULL)
		return;

	WPoint pos(item->pos.x, item->pos.y);
	WSize size((float)bitmap->Width(), (float)bitmap->Height());
	WRect rect(pos, size);

	g_pFresh->GetManager()->GetGDI()->DrawTexture(bitmap, rect, 0xffffffff, 0);
}

void FrMyWareHouse::OnSortDown(int param)
{
	if (m_pSortList->GetSelected() && m_curButton == SET_ITEM_BUTTON_PART)
		SetItemList(SET_ITEM_BUTTON_PART,
			(eSetSortType)m_pSortList->GetSelected()->idx);
}

void FrMyWareHouse::OnPartBtnDown()
{
	UnPushedButtonAll();
	m_buttons[SET_ITEM_BUTTON_PART]->SetStatus(FrButton::PRESSED);
	SetItemList(SET_ITEM_BUTTON_PART, SET_SORT_TYPE_DEFAULT);
}

void FrMyWareHouse::OnClubBtnDown()
{
	UnPushedButtonAll();
	m_buttons[SET_ITEM_BUTTON_CLUB]->SetStatus(FrButton::PRESSED);
	SetItemList(SET_ITEM_BUTTON_CLUB, SET_SORT_TYPE_DEFAULT);
}

void FrMyWareHouse::OnItemBtnDown()
{
	UnPushedButtonAll();
	m_buttons[SET_ITEM_BUTTON_ITEM]->SetStatus(FrButton::PRESSED);
	SetItemList(SET_ITEM_BUTTON_ITEM, SET_SORT_TYPE_DEFAULT);
}

void FrMyWareHouse::OnCardBtnDown()
{
	UnPushedButtonAll();
	m_buttons[SET_ITEM_BUTTON_CARD]->SetStatus(FrButton::PRESSED);
	SetItemList(SET_ITEM_BUTTON_CARD, SET_SORT_TYPE_DEFAULT);
}

void FrMyWareHouse::OnListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	SetItemList(m_curButton, SET_SORT_TYPE_DEFAULT);
}

void FrMyWareHouse::OnListDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == 0)
	{
		return;
	}

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	sWarpItem* pWarp = (sWarpItem*)pItem->pData;
	WPoint pos(pItem->pos);

	float scale;
	switch (pWarp->typeId >> 26)
	{
	case 4:
		scale = 0.7f;
		break;
	case 6:
		scale = 0.8f;
		break;
	case 0x1f:
		scale = 0.6f;
		break;
	default:
		scale = 0.5f;
		break;
	}

	WRect dst(pos.x + 3.0f, pos.y + 3.0f, scale * pWarp->iconRect.w,
		scale * pWarp->iconRect.h);
	pDevice->DrawTexture(pWarp->pIcon, pWarp->iconRect, dst, 0xffffffff, 0);

	if ((pWarp->typeId & 0xfc000000) == 0x8000000)
	{
		dst.x += dst.w;
		DrawMyPartsCardSlot(pWarp->typeId, pWarp->id, dst, 255.0f, 0);
	}

	if ((pWarp->typeId & 0xfc000000) == 0x18000000)
	{
		dst.x += dst.w - 20.0f;
		dst.y += 25.0f;
		m_pFont->Print(g_view, dst.x, dst.y, MakeStr("%d", pWarp->count), 0,
			0xffffffff, 0);
	}

	if ((pWarp->typeId & 0xfc000000) == 0x7c000000)
	{
		dst.x += dst.w;
		dst.y += 25.0f;
		m_pFont->Print(g_view, dst.x, dst.y, MakeStr("%d", pWarp->count), 0,
			0xffffffff, 0);
	}

	pDevice->SetTextColor(0xffffffff, 0xff000000);
	pDevice->SetTextStyle(2);
	pDevice->PrintText11(WPoint(pos.x + 80.0f, pos.y + 17.0f), 0,
		pWarp->name.c_str(), 0);
	g_view->DrawLine2D(pos, WPoint(pos.x + 235.0f, pos.y), 0xff000000,
		0xff000000, 0);
	pDevice->SetTextColor(0xff000000, 0xffffffff);
	pDevice->SetTextStyle(0);

	WRect rect(pos.x, pos.y, m_pItemList->GetItemWidth(),
		m_pItemList->GetItemHeight());
	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		if (g_input->GetButton(LEFT_BUTTON) == 1)
		{
			*m_pItem = sTradeDropItem(pWarp->id, pWarp->typeId);
		}

		pDevice->LineBox(
			WRect(rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f, rect.h - 2.0f),
			0xffffff00, 0, 0.0f);

		FrTradeToolTip* pToolTip =
			FindPrivateTradeChildForm<FrTradeToolTip>("FrTradeToolTip");
		if (pToolTip)
		{
			_sPrivateTradeItemSingle single;
			memset(&single, 0, sizeof(single));
			Formator(sTradeDropItem(pWarp->id, pWarp->typeId), single);
			pToolTip->SetToolTip(&single);
		}
	}
}

void FrMyWareHouse::OnViewDraw()
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* bitmap = g_pFresh->GetBitmap("cfrm00");
	gdi->DrawTexture(bitmap,
		WRect(0.0f, 0.0f, (float)bitmap->Width(), (float)bitmap->Height()),
		m_rect, 0xa0800000, 0);
	DrawPang();
}

void FrMyWareHouse::OnListDblClick()
{
	if (m_pItemList->GetSelected() == NULL)
		return;

	const void* data = m_pItemList->GetSelected()->pData;
	if (data == NULL)
		return;

	FrPrivateTrade* trade =
		(FrPrivateTrade*)g_pFresh->GetManager()->GetDesktop()->FindChildForm(
			"FrPrivateTrade");
	if (trade)
		trade->MoveItem("FrMyWareHouse", data);
}

void FrMyWareHouse::SetItemList(eSetItemButton button, eSetSortType sortType)
{
	InitSortList(button);

	m_pItemList->ClearItem();
	std::for_each(m_warpItems.begin(), m_warpItems.end(), Delete_Object());
	m_warpItems.clear();

	switch (button)
	{
	case SET_ITEM_BUTTON_PART:
		SetListParts(m_warpItems);
		break;
	case SET_ITEM_BUTTON_CLUB:
		SetListClubSet(m_warpItems);
		break;
	case SET_ITEM_BUTTON_ITEM:
		SetListHItem(m_warpItems);
		SetListUitem(m_warpItems);
		break;
	case SET_ITEM_BUTTON_CARD:
		SetListCard(m_warpItems);
		break;
	}

	_sPrivateTradeItem items;
	memset(&items, 0, sizeof(items));
	FrPrivateTrade* pTrade =
		(FrPrivateTrade*)g_pFresh->GetDesktop()->FindChildForm(
			"FrPrivateTrade");

	if (pTrade)
	{
		pTrade->GetMyTradeItems(items); // 983: pelo método de FrPrivateTrade (008d0810)
	}

	DoSortItem(m_warpItems, button, sortType);
	for (std::vector<sWarpItem*>::iterator it = m_warpItems.begin();
		it != m_warpItems.end(); ++it)
	{
		sWarpItem* pItem = *it;
		bool bOnTrade = false;
		for (int i = 0; i < 4; i++)
		{
			if (items.items[i].typeId == pItem->typeId &&
				items.items[i].itemId == pItem->id)
			{
				if (items.items[i].count == pItem->count)
				{
					bOnTrade = true;
					break;
				}
			}
		}

		if (!bOnTrade)
		{
			m_pItemList->AddItem(pItem);
		}
	}

	m_curButton = button;
	m_pItemList->GetScrollBar()->ScrollToFirst();
}

void FrMyWareHouse::SetButtonStyle(int param, eSetItemButton button)
{
	FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (pButton)
	{
		pButton->SetPushDelay(0.0f);
		pButton->SetStyle(FrButton::BT_3TYPES);

		m_buttons[button] = pButton;
	}
}

void FrMyWareHouse::UnPushedButtonAll()
{
	for (std::vector<FrButton*>::iterator it = m_buttons.begin();
		it != m_buttons.end(); ++it)
	{
		(*it)->SetStatus(FrButton::NORMAL);
	}
}

IMPLEMENT_OBJECT(FrTradeToolTip, FrForm)

BEGIN_FRESH_MSGMAP(FrTradeToolTip, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrTradeToolTip::OnViewDraw)
ON_FRESH_VI("TextList", FRCMD_INIT, FrTradeToolTip::OnTextListInit)
ON_FRESH_VI("TextList", FRCMD_OWNERDRAW, FrTradeToolTip::OnTextListDraw)

END_FRESH_MSGMAP()

FrTradeToolTip::FrTradeToolTip()
	: m_alpha(0),
	  m_pTextList(0),
	  m_pIcon(0),
	  m_pItem(0),
	  m_bDragDraw(0),
	  m_pDragItem(0)
{
	m_pItem = new _sPrivateTradeItemSingle;
	memset(m_pItem, 0, sizeof(_sPrivateTradeItemSingle));
}

FrTradeToolTip::~FrTradeToolTip()
{
	if (m_pItem != 0)
	{
		delete m_pItem;
		m_pItem = 0;
	}
}

void FrTradeToolTip::SetDragItem(const sTradeDropItem* pItem)
{
	m_pDragItem = pItem;
}

void FrTradeToolTip::OnProc(const float deltaTime)
{
	FrForm::OnProc(deltaTime);

	if (CTradeDragDrop::IsPoped())
	{
		m_alpha = 255.0f;
		return;
	}

	WRect rect = m_rect;
	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{ // WRect::IsInRect is inline in the original (wtypes.h:174)
		m_alpha = 255.0f;
		m_pTextList->GetScrollBar()->SetVisible(true);
		return;
	}

	if (m_alpha > 0.0f)
	{
		m_pTextList->GetScrollBar()->SetVisible(false);
		m_alpha -= deltaTime * 120.0f;
		if (m_alpha < 0.0f)
		{
			m_alpha = 0.0f;
		}
	}
}

void FrTradeToolTip::SetToolTip(_sPrivateTradeItemSingle* pItem)
{
	m_alpha = 255.0f;
	m_pTextList->GetScrollBar()->SetVisible(true);

	if (m_pItem->itemId == pItem->itemId && m_pItem->typeId == pItem->typeId)
	{
		return;
	}

	if (CTradeDragDrop::IsPoped())
	{
		return;
	}

	*m_pItem = *pItem;

	m_pTextList->ClearItem();

	m_pTextList->GetScrollBar()->ScrollToFirst();

	m_pIcon = 0;
	m_textLines.clear();

	std::string name, desc, text;
	switch (m_pItem->typeId >> 26)
	{
	case 28:
		GetIconNameDesc(m_pIcon, name, desc,
			ItemManager()->FindAuxPart(m_pItem->typeId));
		break;
	case 2:
		GetIconNameDesc(m_pIcon, name, desc,
			ItemManager()->FindPart(m_pItem->typeId));
		break;
	case 4:
		GetIconNameDesc(m_pIcon, name, desc,
			ItemManager()->FindClubSet(m_pItem->typeId));
		break;
	case 6:
		GetIconNameDesc(m_pIcon, name, desc,
			ItemManager()->FindItem(m_pItem->typeId));
		break;
	case 31:
		GetIconNameDesc(m_pIcon, name, desc,
			ItemManager()->FindCard(m_pItem->typeId));
		break;
	}

	m_textLines.push_back(std::string(""));
	m_textLines.push_back(name);

	if ((m_pItem->typeId & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(m_pItem->typeId);
		if (pPart)
		{
			m_textLines.push_back(std::string(""));
			GetAblilityText(m_textLines, pPart->Attr, pPart->Slot);
		}
	}

	if ((m_pItem->typeId & 0xfc000000) == 0x10000000)
	{
		IFF_STRUCT::sClubSet* pClubSet =
			ItemManager()->FindClubSet(m_pItem->typeId);
		if (pClubSet)
		{
			m_textLines.push_back(std::string(""));
			GetAblilityText(m_textLines, pClubSet->Attr, m_pItem->slotStats);
		}
	}

	for (int i = 0; i < 10; i++)
	{
		unsigned long cardId = m_pItem->cards[i].typeId;
		if (cardId != 0)
		{
			std::string cardText(" + ");
			cardText += GetCardInfoText(cardId);

			m_textLines.push_back(cardText.c_str());
		}
	}

	m_textLines.push_back(std::string(""));
	MakeFormatText(desc, m_textLines, 0);

	for (std::vector<std::string>::iterator it = m_textLines.begin();
		it != m_textLines.end(); ++it)
	{
		m_pTextList->AddItem(&*it);
	}
}

// Breaks the text into lines of at most 40 bytes (a DBCS pair is kept
// together) and appends them to the tooltip lines.
void FrTradeToolTip::MakeFormatText(const std::string& text,
	std::vector<std::string>& lines, int indent)
{
	std::string line;
	if (indent)
	{
		line.assign(indent, ' ');
	}

	int length = text.length();
	for (int i = 0; i < length; ++i)
	{
		if (text[i] == '\n')
		{
			m_textLines.push_back(line);
			line.clear();
			if (indent)
			{
				line.assign(indent, ' ');
			}
		}
		else
		{
			if (Custom_IsLeadByte(text[i])) // 983 008da3a6: Shift-JIS (o 645: bit 7)
			{
				line += text[i];
				++i;
			}
			line += text[i];

			if (line.length() > 40)
			{
				m_textLines.push_back(line);
				line.clear();
				if (text[i + 1] == ' ')
				{
					line.assign(indent + 2, ' ');
				}
				else
				{
					line.assign(indent + 3, ' ');
				}
			}
		}
	}

	if (!line.empty())
	{
		m_textLines.push_back(line);
	}
}


void FrTradeToolTip::OnViewDraw()
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pBg = g_pFresh->GetBitmap("cfrm00");
	pDevice->DrawTexture(pBg,
		WRect(0.0f, 0.0f, (float)pBg->Width(), (float)pBg->Height()), m_rect,
		0xc8282828, 0);

	if (m_pIcon)
	{
		WRect iconRect(m_rect.x + 10.0f,
			(m_rect.h - m_pIcon->Height()) * 0.5f + m_rect.y, m_pIcon->Width(),
			m_pIcon->Height());
		pDevice->DrawTexture(m_pIcon, iconRect,
			((unsigned long)(unsigned char)m_alpha << 24) | 0x00ffffff, 0);

		if ((m_pItem->typeId & 0xfc000000) == 0x8000000)
		{
			// The bar of the character the part belongs to.
			IFF_ITEM_COMMON* pCharacter = ItemManager()->FindCommonItem(
				((m_pItem->typeId >> 18) & 0xff) | 0x4000000);
			if (pCharacter)
			{
				const Bitmap* pBar =
					g_pFresh->GetBitmap(MakeStr("bar_%s", pCharacter->Icon));
				if (pBar)
				{
					pDevice->DrawTexture(pBar,
						WRect(m_rect.x + 5.0f, m_rect.y + 5.0f, pBar->Width(),
							pBar->Height()),
						((unsigned long)(unsigned char)m_alpha << 24) |
							0x00ffffff,
						0);
				}
			}
			DrawPartsCardSlot(m_pItem, m_rect, m_alpha, 1);
		}
	}

	if (m_bDragDraw)
	{
		FrWndManager* pManager = g_pFresh->GetManager();
		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(
			m_pDragItem ? m_pDragItem->typeId : 0);
		if (pCommon)
		{
			const Bitmap* pIcon = g_pFresh->GetBitmap(pCommon->Icon);
			if (pIcon == 0)
			{
				pIcon = g_pFresh->GetBitmap("caddie_no");
			}

			const WPoint& mousePos = pManager->GetMousePos();
			pDevice->DrawTexture(pIcon,
				WRect(mousePos.x - pIcon->Width() * 0.5f,
					mousePos.y - pIcon->Height() * 0.5f, pIcon->Width(),
					pIcon->Height()),
				0xc8ffffff, 0);
		}
	}
}

void FrTradeToolTip::OnTextListInit(int param)
{
	m_pTextList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrTradeToolTip::OnTextListDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == 0)
	{
		return;
	}

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	std::string* pText = (std::string*)pItem->pData;

	pDevice->SetTextColor(
		((unsigned long)(unsigned char)m_alpha << 24) | 0x00ffffff, 0xffffffff);
	pDevice->PrintText11(WPoint(pItem->pos.x, pItem->pos.y + 2.0f), 0,
		pText->c_str(), 0);
	pDevice->SetTextColor(0xff000000, 0xffffffff);
}

IMPLEMENT_OBJECT(FrPrivateTradeWnd, FrForm)

BEGIN_FRESH_MSGMAP(FrPrivateTradeWnd, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrPrivateTradeWnd::OnViewDraw)
ON_FRESH_VI("ItemList", FRCMD_INIT, FrPrivateTradeWnd::OnListInit)
ON_FRESH_VI("ItemList", FRCMD_OWNERDRAW, FrPrivateTradeWnd::OnListDraw)
ON_FRESH_VV("ItemList", FRCMD_LBUTTONDOWN, FrPrivateTradeWnd::OnListDown)
ON_FRESH_VV("ItemList", FRCMD_DBLCLICK, FrPrivateTradeWnd::OnListDblClick)
ON_FRESH_VI("NumEditBtn01", FRCMD_INIT, FrPrivateTradeWnd::OnNumEditBtn01Init)
ON_FRESH_VV("NumEditBtn01", FRCMD_LBUTTONUP,
	FrPrivateTradeWnd::OnNumEditBtn01Down)
ON_FRESH_VI("NumEditBtn02", FRCMD_INIT, FrPrivateTradeWnd::OnNumEditBtn02Init)
ON_FRESH_VV("NumEditBtn02", FRCMD_LBUTTONUP,
	FrPrivateTradeWnd::OnNumEditBtn02Down)
ON_FRESH_VI("NumEditBtn03", FRCMD_INIT, FrPrivateTradeWnd::OnNumEditBtn03Init)
ON_FRESH_VV("NumEditBtn03", FRCMD_LBUTTONUP,
	FrPrivateTradeWnd::OnNumEditBtn03Down)
ON_FRESH_VI("NumEditBtn04", FRCMD_INIT, FrPrivateTradeWnd::OnNumEditBtn04Init)
ON_FRESH_VV("NumEditBtn04", FRCMD_LBUTTONUP,
	FrPrivateTradeWnd::OnNumEditBtn04Down)

END_FRESH_MSGMAP()

FrPrivateTradeWnd::FrPrivateTradeWnd()
	: m_nick("unknown_nick"),
	  m_avatarIndex(0),
	  m_pPangEditWnd(0),
	  m_pNumEditWnd(0),
	  m_bActiveControl(true),
	  m_tradeState(0),
	  m_pTradeItem(0)
{
	back_insert_null(m_buttons, 4);
	m_pTradeItem = new _sPrivateTradeItem;
	memset(m_pTradeItem, 0, sizeof(_sPrivateTradeItem));
}

FrPrivateTradeWnd::~FrPrivateTradeWnd()
{
	if (m_pTradeItem != 0)
	{
		delete m_pTradeItem;
		m_pTradeItem = 0;
	}
	m_pItemList->ClearItem();
	m_buttons.clear();
}

bool FrPrivateTradeWnd::OnInit()
{
	WRect rect = m_rect;
	rect.x += 145.0f;
	rect.y += 5.0f;
	rect.w -= 150.0f;
	rect.h = 20.0f;
	EncapsulatedPang::SetRect(rect);

	// HACK: dead reference to a small (inlinable) function defined further
	// down; it makes the compiler emit OnInit one pass later, as in the
	// original.
	if (0)
		DoConfirm();

	return FrForm::OnInit();
}

bool FrPrivateTradeWnd::DropItemUp()
{
	return false;
}

bool FrPrivateTradeWnd::DropItemDown()
{
	if (!m_bActiveControl)
	{
		return false;
	}

	const sTradeDropItem* pItem = GetDropItem();
	if (pItem)
	{
		int count = 0;
		for (int i = 0; i < 4; i++)
		{
			FrListItem* pListItem = m_pItemList->GetItem(i);
			if (pListItem != 0 && pListItem->pData)
			{
				_sPrivateTradeItemSingle* pSingle =
					(_sPrivateTradeItemSingle*)pListItem->pData;

				if (pSingle->itemId != 0 && pSingle->typeId != 0)
				{
					count++;
				}

				if (pItem->itemId == pSingle->itemId &&
					pItem->typeId == pSingle->typeId)
				{
					pSingle->count++;
					OnSendItemUpdate();
					return true;
				}
			}
		}

		if (count < 4)
		{
			AddItem(pItem);
			return true;
		}
	}

	return false;
}

void FrPrivateTradeWnd::DropItemMoved()
{
	if (m_bActiveControl)
	{
		SubItem(GetDropItem());
		DoTradeUnLock();
	}
}

void FrPrivateTradeWnd::SetAvatarIndex(int index)
{
	m_avatarIndex = index;
}

void FrPrivateTradeWnd::GetTradeItems(_sPrivateTradeItem& items)
{
	items.pang = GetPang();
	for (int i = 0; i < 4; i++)
	{
		FrListItem* pItem = m_pItemList->GetItem(i);
		if (pItem != 0 && pItem->pData)
		{
			items.items[i] = *(const _sPrivateTradeItemSingle*)pItem->pData;
		}
	}
}

void FrPrivateTradeWnd::DoConfirm()
{
	m_tradeState = 1;
}

void FrPrivateTradeWnd::DoPermit()
{
	m_tradeState = 2;
	m_bActiveControl = false;
}

void FrPrivateTradeWnd::DoTradeUnLock()
{
	if (m_tradeState != 0)
	{
		FrPrivateTrade* pTrade =
			(FrPrivateTrade*)g_pFresh->GetDesktop()->FindChildForm(
				"FrPrivateTrade");
		if (pTrade)
		{
			pTrade->ButtonContolByConfirmCancel();
		}

		m_tradeState = 0;
	}
}

void FrPrivateTradeWnd::OpenPangEditWnd()
{
	if (m_bActiveControl)
	{
		WRect rect = *GetRectPtr();
		if (m_pPangEditWnd == 0 &&
			rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		{
			OnSendItemUpdate();

			m_pPangEditWnd = CreateForm<FrPangEdit>(g_pFresh->GetManager(),
				this, "FrPangEdit", 0);

			m_pPangEditWnd->EnableDrag(false);
			m_pPangEditWnd->SetFixed(true);
			m_pPangEditWnd->SetPangValue(GetPang());

			m_pPangEditWnd->SetTexRate(5);
			m_pPangEditWnd->Open(
				(FRESH_PFN_RESULT)&FrPrivateTradeWnd::OnResultEditPang,
				WPoint(rect.x + 10.0f, rect.y + 10.0f), 1);
		}
	}
}

void FrPrivateTradeWnd::OpenNumEditWnd(int index)
{
	if (!m_bActiveControl || index < 0 || index > 3)
	{
		return;
	}

	FrButton* pButton = m_buttons[index];
	if (pButton != 0 && !pButton->IsVisible())
	{
		return;
	}

	FrListItem* pListItem = m_pItemList->GetItem(index);
	if (pListItem == 0)
	{
		return;
	}

	sTradeDropItem* pItem = (sTradeDropItem*)pListItem->pData;
	if (pItem->itemId == 0 && pItem->typeId == 0)
	{
		return;
	}

	WPoint pos(pButton->GetRect().x, pButton->GetRect().y);
	if (m_pNumEditWnd == 0)
	{
		OnSendItemUpdate();

		m_pNumEditWnd =
			CreateForm<FrNumEdit>(g_pFresh->GetManager(), this, "FrNumEdit", 0);

		m_pNumEditWnd->EnableDrag(false);
		m_pNumEditWnd->SetFixed(true);
		m_pNumEditWnd->SetValue(pItem->count);
		m_pNumEditWnd->SetReturnParam(pListItem);
		m_pNumEditWnd->SetEditRange(0,
			GetMyItemCount(pItem->itemId, pItem->typeId));
		m_pNumEditWnd->Open(
			(FRESH_PFN_RESULT)&FrPrivateTradeWnd::OnResultEditNum,
			WPoint(pos.x + 10.0f, pos.y + 10.0f), 1);
	}
}

void FrPrivateTradeWnd::OnSendItemUpdate()
{
	if (m_bActiveControl)
	{
		AfxGetTask()->GetActor("PrivateTrade")
			<< MsgObject(0, 0x2c3/*983*/, 1, (int)m_pTradeItem, 0, 0, 0);

		FrPrivateTrade* pTrade =
			(FrPrivateTrade*)g_pFresh->GetDesktop()->FindChildForm(
				"FrPrivateTrade");
		if (pTrade)
		{
			pTrade->SetTradeUnLockAll(); // 983: chama o método (008d26b0) em vez dos 2 DoTradeUnLock
		}
	}
}

void FrPrivateTradeWnd::OnProc(const float deltaTime)
{
	if (EncapsulatedPang::IsStyle(EncapsulatedPang::STYLE_HOVER) &&
		g_input->GetButton(LEFT_BUTTON) == 1)
	{
		OpenPangEditWnd();
	}
}

bool FrPrivateTradeWnd::OnResultEditPang(int result, FrForm* pForm)
{
	if (m_bActiveControl && result == FrOK)
	{
		FrPangEdit* pEdit = DYNAMIC_CAST(FrPangEdit, pForm);
		if (pEdit)
		{
			__int64 pang = pEdit->GetPangValue();
			__int64 tex = pEdit->GetTex();

			if (Doc()->m_myInfo.stat.i64Pang >= pang + tex)
			{
				SetPang(pang);
				m_pTradeItem->pang = pang;

				FrPrivateTrade* pTrade =
					(FrPrivateTrade*)g_pFresh->GetDesktop()->FindChildForm(
						"FrPrivateTrade");
				if (pTrade)
				{
					pTrade->SetPang(tex);
				}

				FrMyWareHouse* pWareHouse =
					FindPrivateTradeChildForm<FrMyWareHouse>("FrMyWareHouse");
				if (pWareHouse)
				{
					pWareHouse->SetPang(
						Doc()->m_myInfo.stat.i64Pang - tex - pang);
				}
				OnSendItemUpdate();
			}
		}
	}

	m_pPangEditWnd = 0;
	return true;
}

bool FrPrivateTradeWnd::OnResultEditNum(int result, FrForm* pForm)
{
	if (m_bActiveControl && result == FrOK)
	{
		FrNumEdit* pEdit = DYNAMIC_CAST(FrNumEdit, pForm);
		if (pEdit)
		{
			unsigned long value = pEdit->GetValue();
			FrListItem* pListItem = (FrListItem*)pEdit->GetReturnParam();
			if (pListItem != 0 && pListItem->pData)
			{
				_sPrivateTradeItemSingle* pSingle =
					(_sPrivateTradeItemSingle*)pListItem->pData;
				unsigned long count =
					GetMyItemCount(pSingle->itemId, pSingle->typeId);
				if (value <= count)
				{
					pSingle->count = (short)value;
					FrMyWareHouse* pWareHouse =
						FindPrivateTradeChildForm<FrMyWareHouse>(
							"FrMyWareHouse");
					if (pWareHouse)
					{
						pWareHouse->SetListItem(pSingle->itemId,
							pSingle->typeId, count - value);
					}
					OnSendItemUpdate();
				}
			}
		}
	}

	m_pNumEditWnd = 0;
	return true;
}

void FrPrivateTradeWnd::AddItem(const sTradeDropItem* pItem)
{
	if (m_bActiveControl)
	{
		for (int i = 0; i < 4; i++)
		{
			FrListItem* pListItem = m_pItemList->GetItem(i);
			if (pListItem)
			{
				_sPrivateTradeItemSingle* pSingle =
					(_sPrivateTradeItemSingle*)pListItem->pData;
				if (pSingle->itemId == 0 && pSingle->typeId == 0)
				{
					Formator(*pItem, *pSingle);
					OnSendItemUpdate();
					break;
				}
			}
		}
	}
}

void FrPrivateTradeWnd::OnUpdateTradeItem(const _sPrivateTradeItem& items)
{
	if (!m_bActiveControl)
	{
		*m_pTradeItem = items;
		SetPang(m_pTradeItem->pang);
	}
}

void FrPrivateTradeWnd::SubItem(const sTradeDropItem* pItem)
{
	for (int i = 0; i < 4; i++)
	{
		FrListItem* pListItem = m_pItemList->GetItem(i);
		if (pListItem != 0 && pListItem->pData)
		{
			_sPrivateTradeItemSingle* pSingle =
				(_sPrivateTradeItemSingle*)pListItem->pData;
			if (pSingle->itemId == pItem->itemId &&
				pSingle->typeId == pItem->typeId)
			{
				memset(pSingle, 0, sizeof(_sPrivateTradeItemSingle));
				OnSendItemUpdate();
				break;
			}
		}
	}
}

void FrPrivateTradeWnd::OnListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	for (int i = 0; i < 4; i++)
	{
		m_pItemList->AddItem(&m_pTradeItem->items[i]);
	}
}

void FrPrivateTradeWnd::OnListDown()
{
	if (m_bActiveControl)
	{
		FrListItem* pItem = m_pItemList->GetSelected();
		if (pItem)
		{
			sTradeDropItem* pTradeItem = (sTradeDropItem*)pItem->pData;
			if (pTradeItem)
			{
				if (pTradeItem->itemId != 0 || pTradeItem->typeId != 0)
				{
					CTradeDragDrop::SetPopItem(
						sTradeDropItem(pTradeItem->itemId, pTradeItem->typeId));
				}
			}
		}
	}
}

void FrPrivateTradeWnd::OnListDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == 0)
	{
		return;
	}

	if (pItem->pData == 0)
	{
		return;
	}

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	WRect rect(pItem->pos.x, pItem->pos.y, m_pItemList->GetItemWidth(),
		m_pItemList->GetItemHeight());

	const Bitmap* pBg = g_pFresh->GetBitmap("cfrm00");
	WRect src(0.0f, 0.0f, pBg->Width(), pBg->Height());

	_sPrivateTradeItemSingle* pSingle = (_sPrivateTradeItemSingle*)pItem->pData;
	if (pSingle->itemId == 0 && pSingle->typeId == 0)
	{
		// An empty slot: the frame, an outline, and no number-entry button.
		pDevice->DrawTexture(pBg, src, rect, 0xa0ffffff, 0);
		pDevice->LineBox(
			WRect(rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f, rect.h - 2.0f),
			0xff0000ff, 0, 0.0f);

		if (IS_INRANGE(pItem->idx, 0, 3))
		{
			m_buttons[pItem->idx]->SetVisible(false);
		}
		return;
	}

	// "unknown item" (CP949)
	std::string name(
		K2L_Compatibility("\xbe\xcb\xbc\xf6 \xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb"));
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pSingle->typeId);
	const Bitmap* pIcon = 0;
	if (pCommon)
	{
		pIcon = g_pFresh->GetBitmap(pCommon->Icon);
		name = pCommon->Name;
	}

	if (pIcon == 0)
	{
		pIcon = g_pFresh->GetBitmap("caddie_no");
	}

	WOverlay::DrawLineBox(g_view,
		WRect(rect.x + 1.0f, rect.y + 1.0f, rect.w - 2.0f, rect.h - 2.0f), 0,
		0xffffff00);

	float x = (rect.w - pIcon->Width()) * 0.5f + pItem->pos.x;
	float y = (rect.h - pIcon->Height()) * 0.5f + pItem->pos.y;

	WRect iconRect(x, y, pIcon->Width(), pIcon->Height());

	if ((pSingle->typeId & 0xfc000000) == 0x18000000)
	{
		iconRect.x = x + 10.0f;
		iconRect.y = y + 10.0f;
	}
	pDevice->DrawTexture(pIcon, iconRect, 0xffffffff, 0);

	if ((pSingle->typeId & 0xfc000000) == 0x8000000)
	{
		DrawPartsCardSlot(pSingle, rect, 255.0f, 1);
	}

	unsigned long group = pSingle->typeId >> 26;
	if (group == 6 || group == 0x1f)
	{
		m_pFont->Print(g_view, rect.x + 10.0f, rect.y + rect.h - 25.0f,
			MakeStr("%d", (unsigned short)pSingle->count), 0, 0xffffffff, 0);
		if (IS_INRANGE(pItem->idx, 0, 3) && m_bActiveControl)
		{
			m_buttons[pItem->idx]->SetVisible(true);
		}
	}
	else
	{
		if (IS_INRANGE(pItem->idx, 0, 3))
		{
			m_buttons[pItem->idx]->SetVisible(false);
		}
	}

	if (m_tradeState == 1)
	{
		pDevice->DrawTexture(pBg, src, rect, 0x40ffff00, 0);
	}
	if (m_tradeState == 2)
	{
		pDevice->DrawTexture(pBg, src, rect, 0x4000ff00, 0);
	}

	if (rect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
	{
		FrTradeToolTip* pToolTip =
			FindPrivateTradeChildForm<FrTradeToolTip>("FrTradeToolTip");
		if (pToolTip)
		{
			pToolTip->SetToolTip(pSingle);
		}
	}
}

void FrPrivateTradeWnd::OnListDblClick()
{
	if (m_bActiveControl)
	{
		FrListItem* pItem = m_pItemList->GetSelected();
		if (pItem != 0 && pItem->pData)
		{
			void* pData = pItem->pData;
			FrPrivateTrade* pTrade =
				(FrPrivateTrade*)g_pFresh->GetDesktop()->FindChildForm(
					"FrPrivateTrade");
			if (pTrade)
			{
				pTrade->MoveItem("FrPrivateTradeWnd", pData);
			}
		}
	}
}

void FrPrivateTradeWnd::OnNumEditBtn01Init(int param)
{
	m_buttons[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_buttons[0] != 0)
	{
		m_buttons[0]->SetVisible(false);
	}
}

void FrPrivateTradeWnd::OnNumEditBtn02Init(int param)
{
	m_buttons[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_buttons[1] != 0)
	{
		m_buttons[1]->SetVisible(false);
	}
}

void FrPrivateTradeWnd::OnNumEditBtn03Init(int param)
{
	m_buttons[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_buttons[2] != 0)
	{
		m_buttons[2]->SetVisible(false);
	}
}

void FrPrivateTradeWnd::OnNumEditBtn04Init(int param)
{
	m_buttons[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_buttons[3] != 0)
	{
		m_buttons[3]->SetVisible(false);
	}
}

void FrPrivateTradeWnd::OnNumEditBtn01Down()
{
	OpenNumEditWnd(0);
}

void FrPrivateTradeWnd::OnNumEditBtn02Down()
{
	OpenNumEditWnd(1);
}

void FrPrivateTradeWnd::OnNumEditBtn03Down()
{
	OpenNumEditWnd(2);
}

void FrPrivateTradeWnd::OnNumEditBtn04Down()
{
	OpenNumEditWnd(3);
}

// The rectangle grown by `margin` on every side.
// HACK: dllimport + __forceinline keeps the compiler from emitting an
// out-of-line copy (the original has none).
__declspec(dllimport) __forceinline WRect ExpandRect(const WRect& rect,
	float margin)
{
	return WRect(rect.x - margin, rect.y - margin, rect.w + margin * 2.0f,
		rect.h + margin * 2.0f);
}

void FrPrivateTradeWnd::OnViewDraw()
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pBg = g_pFresh->GetBitmap("cfrm00");
	WRect src(0.0f, 0.0f, pBg->Width(), pBg->Height());
	pDevice->DrawTexture(pBg, src, m_rect, 0xa0000080, 0);

	const Bitmap* pFace = 0;
	IFF_ITEM_COMMON* pCharacter =
		ItemManager()->FindCommonItem(m_avatarIndex | 0x4000000);
	if (pCharacter)
	{
		pFace = g_pFresh->GetBitmap(MakeStr("w_%s", pCharacter->Icon));
	}
	if (pFace == 0)
	{
		pFace = g_pFresh->GetBitmap("caddie_no");
	}
	pDevice->DrawTexture(pFace,
		WRect(m_rect.x - 5.0f, m_rect.y - 5.0f, pFace->Width(),
			pFace->Height()),
		0xffffffff, 0);

	pDevice->SetTextStyle(1);
	pDevice->SetTextColor(0xff000000, 0xffffffff);
	g_pFresh->GetManager()->PrintText(
		WPoint(m_rect.x + 40.0f, m_rect.y + 11.0f), 0, m_nick.c_str(), -1.0f,
		0xffffffff);
	pDevice->SetTextStyle(0);

	DrawPang();

	if (m_tradeState)
	{
		// Confirmed (yellow) or permitted (green).
		if (m_tradeState == 1)
			pDevice->DrawTexture(pBg, src, ExpandRect(m_rect, 5.0f), 0x40ffff00,
				0);
		else
			pDevice->DrawTexture(pBg, src, ExpandRect(m_rect, 5.0f), 0x4000ff00,
				0);

		pDevice->LineBox(ExpandRect(ExpandRect(m_rect, 5.0f), 1.0f), 0xff000000,
			0, 0.0f);
	}
}

IMPLEMENT_OBJECT(FrPrivateTrade, FrForm)

BEGIN_FRESH_MSGMAP(FrPrivateTrade, FrForm)

ON_FRESH_VV("RenderArea", FRCMD_OWNERDRAW, FrPrivateTrade::OnViewDraw)
ON_FRESH_VI("Confirm", FRCMD_INIT, FrPrivateTrade::OnInitConfirmBtn)
ON_FRESH_VI("Permit", FRCMD_INIT, FrPrivateTrade::OnInitPermitBtn)
ON_FRESH_VI("Cancel", FRCMD_INIT, FrPrivateTrade::OnInitCancelBtn)
ON_FRESH_VV("Confirm", FRCMD_LBUTTONUP, FrPrivateTrade::OnDownConfirmBtn)
ON_FRESH_VV("Permit", FRCMD_LBUTTONUP, FrPrivateTrade::OnDownPermitBtn)
ON_FRESH_VV("Cancel", FRCMD_LBUTTONUP, FrPrivateTrade::OnDownCancelBtn)

END_FRESH_MSGMAP()

FrPrivateTrade::FrPrivateTrade()
	: m_pDragItem(0), m_pConfirmBtn(0), m_pPermitBtn(0), m_pCancelBtn(0)
{
}

FrPrivateTrade::~FrPrivateTrade()
{
}

void FrPrivateTrade::SetCompanion(const std::string& nickName,
	unsigned long guid, unsigned long typeId)
{
	m_companionNick = nickName;
	m_companionGuid = guid;
	m_companionTypeId = typeId;
}

bool FrPrivateTrade::OnInit()
{
	FrForm::OnInit();

	FrPrivateTradeWnd* pCompanion = CreateForm<FrPrivateTradeWnd>(
		g_pFresh->GetManager(), 0, "FrPrivateTradeWnd", this);
	if (pCompanion == 0)
	{
		return false;
	}
	pCompanion->SetFixed(true);
	pCompanion->EnableDrag(false);

	pCompanion->Open(NULL, WPoint(m_rect.x + 16.0f, m_rect.y + 16.0f), 12);
	m_pCompanionTradeWnd = pCompanion;
	pCompanion->EnableActiveControl(false);
	m_pCompanionTradeWnd->SetNick(m_companionNick);
	m_pCompanionTradeWnd->SetAvatarIndex(m_companionTypeId & 0x3ffffff);

	FrPrivateTradeWnd* pMine = CreateForm<FrPrivateTradeWnd>(
		g_pFresh->GetManager(), 0, "FrPrivateTradeWnd", this);
	if (pMine == 0)
	{
		return false;
	}
	pMine->SetFixed(true);
	pMine->EnableDrag(false);
	pMine->Open(NULL,
		WPoint(m_rect.x + 16.0f,
			pCompanion->GetRect().y + pCompanion->GetRect().h + 16.0f),
		12);
	m_pMyTradeWnd = pMine;
	m_pMyTradeWnd->SetNick(std::string(Doc()->m_myInfo.info.sNick));
	CSharedDoc* pDoc = Doc();
	std::map<unsigned int, sCharacterInfo>::iterator it =
		pDoc->m_charMap.find(pDoc->m_myInfo.userEquip.guidChar);
	if (it != pDoc->m_charMap.end())
	{
		m_pMyTradeWnd->SetAvatarIndex(it->second.tid & 0x3ffffff);
	}
	m_pMyTradeWnd->AddPangBarStyle(EncapsulatedPang::STYLE_HOVER);

	const WRect& myRect = pMine->GetRect();
	float right = myRect.x + myRect.w;
	EncapsulatedPang::SetRect(
		WRect(right - 150.0f, myRect.y + myRect.h + 5.0f, 150.0f, 20.0f));

	FrMyWareHouse* pWareHouse = CreateForm<FrMyWareHouse>(
		g_pFresh->GetManager(), 0, "FrMyWareHouse", this);
	if (pWareHouse == 0)
	{
		return false;
	}
	pWareHouse->SetFixed(true);
	pWareHouse->EnableDrag(false);
	pWareHouse->Open(NULL,
		WPoint(m_rect.w + m_rect.x - pWareHouse->GetRect().w - 16.0f,
			(m_rect.h - pWareHouse->GetRect().h) * 0.5f + m_rect.y),
		12);
	m_pWareHouse = pWareHouse;

	FrTradeToolTip* pToolTip = CreateForm<FrTradeToolTip>(
		g_pFresh->GetManager(), 0, "FrTradeToolTip", this);
	if (pToolTip == 0)
	{
		return false;
	}
	pToolTip->SetFixed(true);
	pToolTip->EnableDrag(false);
	pToolTip->Open(NULL,
		WPoint(m_rect.x + 16.0f,
			m_rect.y + m_rect.h - pToolTip->GetRect().h - 16.0f),
		12);
	m_pToolTip = pToolTip;
	return true;
}

void FrPrivateTrade::GetMyTradeItems(_sPrivateTradeItem& items)
{
	m_pMyTradeWnd->GetTradeItems(items);
}

void FrPrivateTrade::OnProc(const float deltaTime)
{
	// The trade window the left button went down on (the drop target).
	static CTradeDragDrop* s_pDropTarget = 0;

	FrForm::OnProc(deltaTime);

	const WPoint& mousePos = g_pFresh->GetManager()->GetMousePos();

	if (g_input->GetButton(LEFT_BUTTON) == 1)
	{
		WRect myRect = m_pMyTradeWnd->GetRect();
		if (myRect.IsInRect(mousePos))
		{
			s_pDropTarget = m_pMyTradeWnd;
		}
		else
		{
			WRect rect = m_pWareHouse->GetRect();
			if (rect.IsInRect(mousePos))
			{
				s_pDropTarget = m_pWareHouse;
			}
			else
			{
				m_pDragItem = 0;
				s_pDropTarget = 0;
			}
		}

		if (s_pDropTarget != 0)
		{
			s_pDropTarget->Pop();
		}
	}

	if (g_input->GetButton(LEFT_BUTTON) == 3)
	{
		if (s_pDropTarget != 0)
		{
			m_pDragItem = s_pDropTarget->GetDropItem();
			if (m_pDragItem != 0)
			{
				m_pToolTip->SetDragDraw(1);
				m_pToolTip->SetDragItem(m_pDragItem);
			}
			else
			{
				m_pDragItem = 0;
				s_pDropTarget = 0;
			}
		}
	}

	if (g_input->GetButton(LEFT_BUTTON) == 2)
	{
		CTradeDragDrop* pTarget = 0;
		WRect myRect = m_pMyTradeWnd->GetRect();
		if (myRect.IsInRect(mousePos))
		{
			pTarget = m_pMyTradeWnd;
		}
		else
		{
			WRect rect = m_pWareHouse->GetRect();
			if (rect.IsInRect(mousePos))
			{
				pTarget = m_pWareHouse;
			}
		}

		CTradeDragDrop::Drop(pTarget);

		m_pDragItem = 0;
		s_pDropTarget = 0;
	}

	if (m_pDragItem == 0)
	{
		m_pToolTip->SetDragDraw(0);
	}
}

void FrPrivateTrade::UpdateFrTradeItem(const _sPrivateTradeItem& items)
{
	m_pCompanionTradeWnd->OnUpdateTradeItem(items);
	SetTradeUnLockAll();
}

void FrPrivateTrade::FinalFrWait(const _sPrivateTradeItem& items)
{
	_sPrivateTradeItem current;
	memset(&current, 0, sizeof(current));

	m_pCompanionTradeWnd->GetTradeItems(current);
	if (memcmp(&current, &items, sizeof(current)) != 0)
	{
		m_pCompanionTradeWnd->OnUpdateTradeItem(items);
		SetTradeUnLockAll();
	}

	m_pCompanionTradeWnd->DoConfirm();
}

void FrPrivateTrade::SetTradeUnLockAll()
{
	m_pCompanionTradeWnd->DoTradeUnLock();
	m_pMyTradeWnd->DoTradeUnLock();
}

void FrPrivateTrade::MoveItem(const char* from, const void* pData)
{
	CTradeDragDrop* pFrom = NULL;
	CTradeDragDrop* pTo = NULL;

	if (stricmp(from, "FrMyWareHouse") == 0)
	{
		if (pData == NULL)
			return;

		pFrom = m_pWareHouse;
		pTo = m_pMyTradeWnd;

		const sWarpItem* pWarp = (const sWarpItem*)pData;
		pFrom->SetPopItem(sTradeDropItem(pWarp->id, pWarp->typeId));
	}

	if (stricmp(from, "FrPrivateTradeWnd") == 0)
	{
		if (pData == NULL)
			return;

		pFrom = m_pMyTradeWnd;
		pTo = m_pWareHouse;

		const sTradeDropItem* pDrop = (const sTradeDropItem*)pData;
		pFrom->SetPopItem(sTradeDropItem(pDrop->itemId, pDrop->typeId));
	}

	if (pFrom && pTo && pTo->DropItemDown())
		pFrom->DropItemMoved();
}

void FrPrivateTrade::FinalFrOK(const _sPrivateTradeItem& items)
{
	_sPrivateTradeItem current;
	memset(&current, 0, sizeof(current));

	m_pCompanionTradeWnd->GetTradeItems(current);
	if (memcmp(&current, &items, sizeof(current)) != 0)
	{
		m_pCompanionTradeWnd->OnUpdateTradeItem(items);
	}

	m_pCompanionTradeWnd->DoPermit();
}

void FrPrivateTrade::ButtonContolByConfirmCancel()
{
	m_pConfirmBtn->Enable(true);
	m_pPermitBtn->Enable(false);
	m_pCancelBtn->Enable(true);
}

void FrPrivateTrade::OnViewDraw()
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	const Bitmap* pBitmap = g_pFresh->GetBitmap("cfrm00");
	WRect src(0.0f, 0.0f, pBitmap->Width(), pBitmap->Height());
	pDevice->DrawTexture(pBitmap, src, m_rect, 0xffffffff, 0);

	DrawPang();
}

void FrPrivateTrade::OnInitConfirmBtn(int param)
{
	m_pConfirmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrPrivateTrade::OnInitPermitBtn(int param)
{
	m_pPermitBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrPrivateTrade::OnInitCancelBtn(int param)
{
	m_pCancelBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrPrivateTrade::OnDownConfirmBtn()
{
	m_pConfirmBtn->Enable(false);
	m_pPermitBtn->Enable(true);
	m_pMyTradeWnd->DoConfirm();

	_sPrivateTradeItem items;
	memset(&items, 0, sizeof(items));
	m_pMyTradeWnd->GetTradeItems(items);
	AfxGetTask()->GetActor("PrivateTrade")
		<< MsgObject(0, 0x2c3/*983*/, 2, (int)&items, 0, 0, 0);
}

void FrPrivateTrade::OnDownPermitBtn()
{
	m_pConfirmBtn->Enable(false);
	m_pPermitBtn->Enable(false);

	m_pMyTradeWnd->DoPermit();

	_sPrivateTradeItem myItems;
	memset(&myItems, 0, sizeof(myItems));
	_sPrivateTradeItem companionItems;
	memset(&companionItems, 0, sizeof(companionItems));
	m_pMyTradeWnd->GetTradeItems(myItems);
	m_pCompanionTradeWnd->GetTradeItems(companionItems);

	AfxGetTask()->GetActor("PrivateTrade")
		<< MsgObject(0, 0x2c3/*983*/, 3, (int)&myItems, (int)&companionItems, 0, 0);
}

void FrPrivateTrade::OnDownCancelBtn()
{
	FrForm::Close(FrCANCEL, true);
}
