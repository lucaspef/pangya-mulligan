#include "minatl.h"
#include "vc10_layout.h"
#include "playerdlg.h"
#include "frbutton.h"
#include "fredit.h"
#include "frarea.h"
#include "frgaugebar.h"
#include "frlistbox.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "actor.h"
#include "exhibition.h"
#include "parttidlist.h"
#include "../shared/itemmanager.h"
#include "commonutil.h"
#include "cardsystem.h"
#include "../shared/localize.h"
#include "wminmax.h"
#include "mathconsts.h"
#include "wfont.h"
#include "s5/utilities.h"
#include "jp/docofs983.h" // B1446: crus do CSharedDoc ancorados no membro (VS 2022)

// HACK: the linker keeps the copy of "club" of golfplayer.obj (earlier in link order), which is a local static of a
// naked function there, so this unit must not emit its own copy: name that one.
#define RB_STR_CLUB "club"


extern Fresh* g_pFresh;
int CalcPowerPenalty(int upgrade);

CStatBar::CStatBar(float gap, float height)
{
	m_gap = gap;
	m_height = height;
	m_bars.reserve(5);
	for (int i = 0; i < 5; ++i)
	{
		sBar bar(MakeStr("stat_gauge_%d", i));
		bar.fMax = (i == 0) ? 50.0f : 30.0f;
		m_bars.push_back(bar);
	}
	m_destTime = 0.0f;
}

CStatBar::~CStatBar()
{
}

void CStatBar::SetRange(eLvlType type, int level, int capacity, int penalty)
{
	float fLevel = (float)level;
	if (0.0f > fLevel)
		fLevel = 0.0f;
	m_bars[type].fLevel = fLevel;
	float fCapacity = (float)capacity;
	m_bars[type].fCur = 0.0f;
	if (!(fCapacity < m_bars[type].fMax))
		fCapacity = m_bars[type].fMax;
	m_bars[type].fCapacity = fCapacity;
	m_bars[type].penalty = penalty;
}

void CStatBar::SetDestTime(float time)
{
	m_destTime = time;
}

void CStatBar::Process(float delta)
{
	for (int i = 0; i < 5; ++i)
	{
		if (m_bars[i].fCur < m_bars[i].fLevel)
		{
			if (m_destTime > 1.1920929e-7f)
				m_bars[i].fCur += (m_bars[i].fLevel / m_destTime) * delta;
			else
				m_bars[i].fCur = m_bars[i].fLevel;
			if (m_bars[i].fCur > m_bars[i].fLevel)
				m_bars[i].fCur = m_bars[i].fLevel;
		}
		if (m_bars[i].fCur > m_bars[i].fLevel)
		{
			if (m_destTime > 1.1920929e-7f)
				m_bars[i].fCur -= (m_bars[i].fLevel / m_destTime) * delta;
			else
				m_bars[i].fCur = m_bars[i].fLevel;
			if (m_bars[i].fCur < m_bars[i].fLevel)
				m_bars[i].fCur = m_bars[i].fLevel;
		}
	}
}

void CStatBar::SetPos(eLvlType type, int level, int capacity, int penalty)
{
	float fLevel = (float)level;
	if (0.0f > fLevel)
		fLevel = 0.0f;
	m_bars[type].fLevel = fLevel;
	m_bars[type].fCapacity = (float)capacity;
	m_bars[type].penalty = penalty;
}

void CStatBar::Render(const WPoint& pos)
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device)
	{
		for (int i = 0; i < 5; ++i)
		{
			float fi = (float)i;
			WRect rect;
			rect.x = pos.x;
			rect.y = (m_height + m_gap) * fi + pos.y;
			rect.w = (m_bars[i].fCur / m_bars[i].fMax) * 100.0f;
			rect.h = m_height;
			device->DrawTexture(m_bars[i].pBitmap, rect, 0xffffffff, 0);
			device->SetTextStyle(0);
			device->SetTextColor(0xffffffff, 0xffffffff);
			WPoint text(pos.x + 100.0f,
				((m_height + m_gap) * fi + pos.y) - 2.0f);
			device->Print(text, 0, "%d/%d", (int)m_bars[i].fCur,
				(int)m_bars[i].fCapacity);
		}
	}
}

IMPLEMENT_OBJECT(FrSubEquipBar, FrForm)

BEGIN_FRESH_MSGMAP(FrSubEquipBar, FrForm)

ON_FRESH_VI("sub_equip", FRCMD_INIT, FrSubEquipBar::OnInitSubEquipList)
ON_FRESH_VI("sub_equip", FRCMD_LBUTTONDOWN, FrSubEquipBar::OnDownSubEquipList)
ON_FRESH_VI("sub_equip", FRCMD_OWNERDRAW, FrSubEquipBar::OnDrawSubEquipList)

END_FRESH_MSGMAP()

FrSubEquipBar::FrSubEquipBar()
	: m_pResult(NULL)
	, m_pCountFont983(NULL)
{
	C_ASSERT_VC10(sizeof(FrSubEquipBar) == 0x150 && offsetof(FrSubEquipBar, m_pCountFont983) == 0x140); // 983: push 0x150 em 00b240b3
}

FrSubEquipBar::~FrSubEquipBar()
{
	if (g_resrcmng && m_pCountFont983)
	{
		g_resrcmng->Release(m_pCountFont983);
		m_pCountFont983 = NULL;
	}
}

// std::list::push_back reaches _Insert through two inline levels; the original
// has _Insert expanded in place at two call sites (node allocation stays out of line).
#if _MSC_VER == 1600
template <class T>
struct ListAppender : public std::list<T>
{
	__declspec(dllimport) __forceinline void InsertAt(_Nodeptr pnode,
		const T& val)
	{
		_Nodeptr newnode = _Buynode(pnode, _Prevnode(pnode), val);
		_Incsize(1);
		_Prevnode(pnode) = newnode;
		_Nextnode(_Prevnode(newnode)) = newnode;
	}
	__declspec(dllimport) __forceinline void Append(const T& val)
	{
		InsertAt(_Myhead, val);
	}
};
#else // D740: STL novo (_Nodeptr privado): Append = push_back
template <class T>
struct ListAppender : public std::list<T>
{
	void Append(const T& val) { this->push_back(val); }
};
#endif

void FrSubEquipBar::Initialize()
{
	if (m_pSubEquipList == NULL)
		return;

	switch (m_subType)
	{
	case SUBTYPE_0:
		m_pSubEquipList->SetItemSize(0x30, 0x34);
		break;
	case SUBTYPE_1:
		m_pSubEquipList->SetItemSize(0x28, 0x28);
		break;
	case SUBTYPE_2:
		m_pSubEquipList->SetItemSize(0x1a, 0x1c);
		break;
	case SUBTYPE_3:
		m_pSubEquipList->SetItemSize(0x1e, 0x1e);
		break;
	}

	int count = 0;
	float extra = 0.0f;
	switch (m_category)
	{
	case CATEGORY_CHARACTER:
		count = (int)Doc()->m_charMap.size();
		break;
	case CATEGORY_CADDIE:
		count = (int)Doc()->m_caddieMap.size() + 1;
		extra = 2.0f;
		break;
	case CATEGORY_CLUB:
		count = (int)Doc()->m_clubSetMap.size();
		break;
	case CATEGORY_BALL:
		count = (int)Doc()->m_ballList.size();
		break;
	case CATEGORY_MASCOT:
		count = (int)Doc()->m_mascotMap.size() + 1;
		extra = 2.0f;
		break;
	}

	float margin;
	if (m_category == CATEGORY_CHARACTER)
	{
		WRect rect = m_rect;
		margin = (float)count * 3.0f + 10.0f;
		rect.w =
			(float)(m_pSubEquipList->GetItemWidth() * count) + margin + 0.5f;
		rect.h = (float)m_pSubEquipList->GetItemHeight() + 13.0f;
		SetRect(rect);
		rect.x += 8.0f;
		rect.y += 5.0f;
		rect.w =
			(float)(m_pSubEquipList->GetItemWidth() * count) + margin + 0.5f;
		rect.h = (float)m_pSubEquipList->GetItemHeight();
		m_pSubEquipList->SetRect(rect);
	}
	else if (count > 6)
	{
		WRect rect = m_rect;
		rect.w = (float)(m_pSubEquipList->GetItemWidth() * 6) + 28.0f + 13.0f;
		rect.h = 13.0f + (float)m_pSubEquipList->GetItemHeight();
		SetRect(rect);
		rect.x += 5.0f;
		rect.y = 5.0f + rect.y;
		rect.w = (float)(m_pSubEquipList->GetItemWidth() * 6) + 28.0f;
		rect.h = (float)m_pSubEquipList->GetItemHeight();
		m_pSubEquipList->SetRect(rect);
	}
	else
	{
		WRect rect = m_rect;
		margin = (float)count * 3.0f + 10.0f + extra;
		rect.w =
			(float)(m_pSubEquipList->GetItemWidth() * count) + margin + 0.5f;
		rect.h = (float)m_pSubEquipList->GetItemHeight() + 13.0f;
		SetRect(rect);
		rect.x += 8.0f;
		rect.y += 5.0f;
		rect.w =
			(float)(m_pSubEquipList->GetItemWidth() * count) + margin + 0.5f;
		rect.h = (float)m_pSubEquipList->GetItemHeight();
		m_pSubEquipList->SetRect(rect);
	}
	m_pSubEquipList->Resize(m_pSubEquipList->GetItemWidth() + 2,
		m_pSubEquipList->GetItemHeight());

	switch (m_category)
	{
	case CATEGORY_CHARACTER:
		for (std::map<unsigned int, sCharacterInfo>::iterator it =
				 Doc()->m_charMap.begin();
			it != Doc()->m_charMap.end(); ++it)
		{
			IFF_ITEM_COMMON* pItem =
				ItemManager()->FindCommonItem(it->second.tid);
			if (pItem)
				((ListAppender<FrSubEquipBar::_sortitem>&)m_sortList)
					.Append(_sortitem(pItem, it->first));
		}
		break;
	case CATEGORY_CADDIE:
		if (Doc()->m_caddieMap.size() != 0)
		{
			for (std::map<unsigned int, sCaddieInfo>::iterator it =
					 Doc()->m_caddieMap.begin();
				it != Doc()->m_caddieMap.end(); ++it)
			{
				IFF_ITEM_COMMON* pItem =
					ItemManager()->FindCommonItem(it->second.tid);
				if (pItem)
					m_sortList.push_back(_sortitem(pItem, it->second.guid));
			}
		}
		break;
	case CATEGORY_CLUB:
	{
		// 983 00b24120: cartas de personagem/caddie de cada taco (a ultima de cada tipo; slot sem carta = imagem vazia)
		const Bitmap* pCharEmpty = g_pFresh->GetBitmap(S5::UTIL::GetCardClubCharEmpty983());
		const Bitmap* pCaddieEmpty = g_pFresh->GetBitmap(S5::UTIL::GetCardClubCaddieEmpty983());
		for (std::map<unsigned int, sItemInfo>::iterator it =
				 Doc()->m_clubSetMap.begin();
			it != Doc()->m_clubSetMap.end(); ++it)
		{
			IFF_STRUCT::sClubSet* pClubSet =
				ItemManager()->FindClubSet(it->second.tid);
			if (pClubSet == NULL)
				continue;
			IFF_ITEM_COMMON* pItem = &pClubSet->c;
			if (!((pItem->Level >= Doc()->m_myInfo.stat.Level &&
					  pItem->IsUnderLvl) ||
					!pItem->IsUnderLvl))
				continue;
			m_sortList.push_back(_sortitem(pItem, it->second.guid));
			if (pClubSet->CharacterSlot983 == 0 && pClubSet->CaddieSlot983 == 0)
				continue;

			int nCards = 0;
			for (int i = 0; i < 12; ++i)
			{
				if (it->second.attachCard[i])
					++nCards;
			}
			const Bitmap* pChar = NULL;
			const Bitmap* pCaddie = NULL;
			for (std::list<sSCardAvilityPeriodInfo>::iterator c =
					 Doc()->m_cardAbilityList[0].begin();
				c != Doc()->m_cardAbilityList[0].end(); ++c)
			{
				if (it->second.guid != c->partsUid)
					continue;
				IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(c->tid);
				switch (c->cardType)
				{
				case 0:
					pChar = g_pFresh->GetBitmap(pCard->SlotImg);
					--nCards;
					break;
				case 1:
					pCaddie = g_pFresh->GetBitmap(pCard->SlotImg);
					--nCards;
					break;
				}
				if (nCards == 0)
					break;
			}
			if (pChar == NULL && pClubSet->CharacterSlot983)
				pChar = pCharEmpty;
			if (pCaddie == NULL && pClubSet->CaddieSlot983)
				pCaddie = pCaddieEmpty;
			m_clubCardMap983.insert(std::make_pair(it->second.guid,
				std::make_pair(pChar, pCaddie)));
		}
		break;
	}
	case CATEGORY_BALL:
		for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
			it != Doc()->m_ballList.end(); ++it)
		{
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(it->tid);
			if (pItem)
				((ListAppender<FrSubEquipBar::_sortitem>&)m_sortList)
					.Append(_sortitem(pItem, 0));
		}
		break;
	case CATEGORY_MASCOT:
		if (Doc()->m_mascotMap.size() != 0)
		{
			for (std::map<unsigned int, sMascotInfo>::iterator it =
					 Doc()->m_mascotMap.begin();
				it != Doc()->m_mascotMap.end(); ++it)
			{
				IFF_ITEM_COMMON* pItem =
					ItemManager()->FindCommonItem(it->second.tid);
				if (pItem)
					m_sortList.push_back(_sortitem(pItem, it->second.guid));
			}
		}
		break;
	default:
		break;
	}

	m_sortList.sort();
	// 983 00b24120: com premium (bit 14 de +0x812) a bola 0x140000d8 entra no lugar da bola padrao 0x14000000
	bool bPremiumBall = false;
	for (std::list<_sortitem>::iterator it = m_sortList.begin();
		it != m_sortList.end(); ++it)
	{
		if (((*(unsigned long*)DOCPTR983(0x812) >> 0xe) & 1) &&
			m_category == CATEGORY_BALL)
		{
			unsigned long tid = it->pItem->TypeId;
			if (tid == 0x14000000)
			{
				std::list<_sortitem>::iterator it2;
				for (it2 = m_sortList.begin(); it2 != m_sortList.end(); ++it2)
				{
					if (it2->pItem->TypeId == 0x140000d8)
						break;
				}
				if (it2 != m_sortList.end())
				{
					m_pSubEquipList->AddItem(&*it2);
					bPremiumBall = true;
					continue;
				}
				if (bPremiumBall)
					continue;
			}
			else if (tid == 0x140000d8 && bPremiumBall)
				continue;
		}
		m_pSubEquipList->AddItem(&*it);
	}

	if (m_category == CATEGORY_MASCOT || m_category == CATEGORY_CADDIE)
		m_pSubEquipList->AddItem(NULL);

	static char* s_countFont[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_countFont;
	info.numPages = 1;
	info.fontw = 16;
	info.fonth = 16;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pCountFont983 = g_resrcmng->GetTitleFont();
	m_pCountFont983->Create(&info);
}

bool FrSubEquipBar::SetClientEquipInfo(PANGYA_ITEM_GROUP group,
	unsigned long guid)
{
	bool bFound = false;
	switch ((int)group)
	{
	case 1:
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			Doc()->m_charMap.find(guid);
		if (it != Doc()->m_charMap.end())
			bFound = true;
		break;
	}
	case 5:
	{
		std::list<sItemInfo>::iterator it =
			std::find(Doc()->m_ballList.begin(), Doc()->m_ballList.end(), guid);
		if (it != Doc()->m_ballList.end())
			bFound = true;
		break;
	}
	case 4:
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(guid);
		if (it != Doc()->m_clubSetMap.end())
			bFound = true;
		break;
	}
	case 16:
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(guid);
		if (it != Doc()->m_mascotMap.end())
			bFound = true;
		break;
	}
	case 7:
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			Doc()->m_caddieMap.find(guid);
		if (it != Doc()->m_caddieMap.end())
			bFound = true;
		break;
	}
	}
	return bFound;
}

void FrSubEquipBar::OnInitSubEquipList(int param)
{
	m_pSubEquipList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pSubEquipList->SetVisible(true);
}

void FrSubEquipBar::OnDownSubEquipList(int param)
{
	if (m_pSubEquipList->GetSelected())
	{
		_sortitem* pSort = (_sortitem*)m_pSubEquipList->GetSelected()->pData;
		if (pSort)
		{
			m_pResult = pSort->pItem;
			m_lastSelectedGuid = pSort->guid;
			FrForm::OnOK();
			return;
		}
		if (m_category == CATEGORY_CADDIE || m_category == CATEGORY_MASCOT)
		{
			m_pResult = NULL;
			m_lastSelectedGuid = 0;
			FrForm::OnOK();
		}
	}
}

void FrSubEquipBar::OnDrawSubEquipList(int param)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	FrListItem* pListItem = (FrListItem*)param;
	if (pListItem == NULL)
		return;
	if (pDevice == NULL)
		return;

	IFF_ITEM_COMMON* pItem;
	if (pListItem->pData)
		pItem = ((_sortitem*)pListItem->pData)->pItem;
	else
		pItem = NULL;

	const Bitmap* pBitmap;
	if (pItem != NULL)
	{
		switch (m_subType)
		{
		case SUBTYPE_0:
			pBitmap = g_pFresh->GetBitmap(pItem->Icon);
			break;
		case SUBTYPE_1:
			pBitmap = g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
			break;
		case SUBTYPE_2:
			pBitmap = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->Icon));
			break;
		case SUBTYPE_3:
			pBitmap = g_pFresh->GetBitmap(MakeStr("bar_%s", pItem->Icon));
			break;
		default:
			return;
		}
	}
	else
	{
		switch (m_subType)
		{
		case SUBTYPE_0:
			pBitmap = g_pFresh->GetBitmap("caddie_no.tga");
			break;
		case SUBTYPE_1:
			pBitmap = g_pFresh->GetBitmap("w_caddie_no");
			break;
		case SUBTYPE_2:
			pBitmap = g_pFresh->GetBitmap("s_no");
			break;
		case SUBTYPE_3:
			pBitmap = g_pFresh->GetBitmap("bar_no");
			break;
		default:
			return;
		}
	}

	if (pBitmap)
	{
		if (pListItem->idx % m_pSubEquipList->GetColCapacity() == 0)
		{
			const Bitmap* pBar = g_pFresh->GetBitmap("select_bar_02");
			if (pBar)
				pDevice->DrawTexture(pBar, m_rect, 0xffffffff, 0);
		}

		WRect rect(pListItem->pos.x, pListItem->pos.y, (float)pBitmap->Width(),
			(float)pBitmap->Height());
		pDevice->DrawTexture(pBitmap, rect, 0xffffffff, 0);

		if (pItem && (pItem->TypeId & 0xfc000000) == 0x1c000000)
		{
			for (std::map<unsigned int, sCaddieInfo>::iterator it =
					 Doc()->m_caddieMap.begin();
				it != Doc()->m_caddieMap.end(); ++it)
			{
				if (pItem->TypeId == it->second.tid)
				{
					IFF_STRUCT::sCaddie* pCaddie =
						ItemManager()->FindCaddie(pItem->TypeId);
					if (pCaddie && it->second.Rent_flag &&
						it->second.Remain_Date <= 0 &&
						it->second.tidPart == 0 && pCaddie->MonthlyFee)
					{
						g_view->DrawLine2D(
							WPoint(rect.x, rect.h * 0.5f + rect.y - 1.0f),
							WPoint(rect.x + rect.w,
								rect.h * 0.5f + rect.y - 1.0f),
							0xffff0000, 0xffff0000, 0);
						g_view->DrawLine2D(
							WPoint(rect.x, rect.h * 0.5f + rect.y + 0.0f),
							WPoint(rect.x + rect.w, rect.h * 0.5f + rect.y + 0.0f),
							0xffff0000, 0xffff0000, 0);
						g_view->DrawLine2D(
							WPoint(rect.x, rect.h * 0.5f + rect.y + 1.0f),
							WPoint(rect.x + rect.w,
								rect.h * 0.5f + rect.y + 1.0f),
							0xffff0000, 0xffff0000, 0);
					}
					break;
				}
			}
		}

		else if (pItem && (pItem->TypeId >> 26) == 4)
		{
			// 983 00b21930: cartas do taco (Initialize) sobre o icone; deslocamento pelo subtipo
			float offX = 0.0f;
			float offY = 0.0f;
			switch (m_subType)
			{
			case SUBTYPE_0:
				offX = 37.0f;
				offY = 29.0f;
				break;
			case SUBTYPE_1:
				offX = 25.0f;
				offY = 27.0f;
				break;
			case SUBTYPE_2:
				offX = 11.0f;
				offY = 15.0f;
				break;
			case SUBTYPE_3:
				offX = 15.0f;
				offY = 17.0f;
				break;
			}
			std::map<unsigned long, std::pair<const Bitmap*, const Bitmap*> >::iterator
				it = m_clubCardMap983.find(((_sortitem*)pListItem->pData)->guid);
			if (it != m_clubCardMap983.end())
			{
				const Bitmap* pCard = it->second.first;
				if (pCard)
					pDevice->DrawTexture(pCard,
						WRect(pListItem->pos.x, pListItem->pos.y + offY,
							(float)pCard->Width(), (float)pCard->Height()),
						0xffffffff, 0);
				pCard = it->second.second;
				if (pCard)
					pDevice->DrawTexture(pCard,
						WRect(pListItem->pos.x + offX, pListItem->pos.y + offY,
							(float)pCard->Width(), (float)pCard->Height()),
						0xffffffff, 0);
			}
		}
		else if (pItem && (pItem->TypeId >> 26) == 5 && m_subType == SUBTYPE_1)
		{
			// 983 00b21930: numero de bolas a direita (x+35, y+22), branco sob o mouse senao meio transparente
			for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
				 it != Doc()->m_ballList.end(); ++it)
			{
				if (it->tid == pItem->TypeId)
				{
					WPoint mouse(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
					unsigned long color = rect.IsInRect(mouse) ? 0xffffffff : 0x7fffffff;
					if ((short)it->Common[0] != 0)
						pDevice->PrintTitleFont983(m_pCountFont983,
							pListItem->pos.x + 35.0f, pListItem->pos.y + 22.0f,
							MakeStr("%d", (int)(short)it->Common[0]), 2.0f, 2, 1.0f, color);
					break;
				}
			}
		}

		WPoint point(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (rect.IsInRect(point))
		{
			const Bitmap* pFrame = g_pFresh->GetBitmap("select_frame");
			if (pFrame)
				pDevice->DrawTexture(pFrame,
					WRect(rect.x - 3.0f, rect.y - 3.0f, rect.w + 6.0f,
						rect.h + 6.0f),
					0xffffffff, 0);
		}
	}
}

IFF_ITEM_COMMON* CEquipHelper::GetItemCommon(eCategory category,
	const sUserEquip& equip)
{
	unsigned long tid = 0;
	switch (category)
	{
	case CATEGORY_CHARACTER:
	{
		std::map<unsigned int, sCharacterInfo>::iterator it =
			Doc()->m_charMap.find(equip.guidChar);
		if (it != Doc()->m_charMap.end())
			return ItemManager()->FindCommonItem(it->second.tid);
		break;
	}
	case CATEGORY_CLUB:
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(equip.guidClubSet);
		if (it != Doc()->m_clubSetMap.end())
			tid = it->second.tid;
		break;
	}
	case CATEGORY_BALL:
		return ItemManager()->FindCommonItem(equip.tidBall);
	case CATEGORY_CADDIE:
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			Doc()->m_caddieMap.find(equip.guidCaddie);
		if (it != Doc()->m_caddieMap.end())
			tid = it->second.tid;
		break;
	}
	case CATEGORY_MASCOT:
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(equip.guidMascot);
		if (it != Doc()->m_mascotMap.end())
			tid = it->second.tid;
		break;
	}
	}
	return ItemManager()->FindCommonItem(tid);
}

unsigned long CEquipHelper::GetEquipItemGuid(eCategory category, bool bRandom)
{
	switch (category)
	{
	case CATEGORY_CHARACTER:
		if (!bRandom)
			return Doc()->m_myInfo.userEquip.guidChar;
		return RandSelectGuid(Doc()->m_charMap);
	case CATEGORY_CADDIE:
	{
		if (!bRandom)
			return Doc()->m_myInfo.userEquip.guidCaddie;
		unsigned int size = (unsigned int)Doc()->m_caddieMap.size();
		if (size != 0 && rand() % size != 0)
			return RandSelectGuid(Doc()->m_caddieMap);
		break;
	}
	case CATEGORY_BALL:
	{
		if (!bRandom)
			return Doc()->m_myInfo.userEquip.tidBall;
		int size = (int)Doc()->m_ballList.size();
		if (size == 0)
			return 0;
		std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
		int n = rand() % size;
		for (int i = 0; i < n; ++i)
			++it;
		return it->tid;
	}
	case CATEGORY_CLUB:
		if (bRandom)
			return RandSelectGuid(Doc()->m_clubSetMap);
		return Doc()->m_myInfo.userEquip.guidClubSet;
	case CATEGORY_MASCOT:
	{
		if (!bRandom)
			return Doc()->m_myInfo.userEquip.guidMascot;
		unsigned int size = (unsigned int)Doc()->m_mascotMap.size();
		if (size != 0 && rand() % size != 0)
			return RandSelectGuid(Doc()->m_mascotMap);
		break;
	}
	}
	return 0;
}

void FrSubEquipBar::OnDraw()
{
	FrForm::OnDraw();
}

void FrSubEquipBar::OnProc(const float delta)
{
	FrForm::OnProc(delta);
	if (g_input->GetButton(LEFT_BUTTON) == 1)
	{
		// 983 00b1e6a0: fecha ao clicar fora da barra
		WRect rect = m_rect;
		WPoint point(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
		if (!rect.IsInRect(point))
			FrForm::OnCancel();
	}
}

void FrSubEquipBar::OnOK()
{
	FrForm::OnOK();
}

void FrSubEquipBar::OnCancel()
{
	FrForm::OnCancel();
}

void FrSubEquipBar::Open(bool (FrCmdTarget::*pFnResult)(int, FrForm*),
	eCategory category, const WPoint& pos, eSubType subType)
{
	m_category = category;
	m_subType = subType;
	Initialize();
	FrForm::Open(pFnResult, pos, 1);
}

CEquipHelper::CEquipHelper()
{
}

CEquipHelper::~CEquipHelper()
{
}

unsigned long CEquipHelper::GetEquipItemTid(eCategory category, bool bRandom)
{
	unsigned long tid = 0;
	switch (category)
	{
	case CATEGORY_CHARACTER:
		if (bRandom)
			tid = MapFindEquipTid(Doc()->m_charMap, 0);
		else
			tid = MapFindEquipTid(Doc()->m_charMap,
				Doc()->m_myInfo.userEquip.guidChar);
		break;
	case CATEGORY_CADDIE:
		if (bRandom)
			tid = MapFindEquipTid(Doc()->m_caddieMap, 0);
		else
			tid = MapFindEquipTid(Doc()->m_caddieMap,
				Doc()->m_myInfo.userEquip.guidCaddie);
		break;
	case CATEGORY_BALL:
		if (bRandom)
		{
			int size = (int)Doc()->m_ballList.size();
			if (size != 0)
			{
				std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
				int n = rand() % size;
				for (int i = 0; i < n; ++i)
					++it;
				tid = it->tid;
			}
		}
		else
		{
			for (std::list<sItemInfo>::iterator it = Doc()->m_ballList.begin();
				it != Doc()->m_ballList.end(); ++it)
			{
				if (it->tid == Doc()->m_myInfo.userEquip.tidBall)
				{
					tid = it->tid;
					break;
				}
			}
		}
		break;
	case CATEGORY_CLUB:
		if (bRandom)
			tid = MapFindEquipTid(Doc()->m_clubSetMap, 0);
		else
			tid = MapFindEquipTid(Doc()->m_clubSetMap,
				Doc()->m_myInfo.userEquip.guidClubSet);
		break;
	case CATEGORY_MASCOT:
		if (bRandom)
			tid = MapFindEquipTid(Doc()->m_mascotMap, 0);
		else
			tid = MapFindEquipTid(Doc()->m_mascotMap,
				Doc()->m_myInfo.userEquip.guidMascot);
		break;
	}
	return tid;
}

unsigned long CEquipHelper::GetGuidFromTid(eCategory category,
	unsigned long tid)
{
	if (0)
		((CEquipHelper*)0)
			->CEquipHelper::GetEquipItemTid(*(eCategory*)0, *(bool*)0);
	unsigned long guid = 0;
	switch (category)
	{
	case CATEGORY_CHARACTER:
		for (std::map<unsigned int, sCharacterInfo>::const_iterator it =
				 Doc()->m_charMap.begin();
			it != Doc()->m_charMap.end(); ++it)
		{
			if ((*it).second.tid == tid)
			{
				guid = (*it).second.guid;
				break;
			}
		}
		break;
	case CATEGORY_CADDIE:
		for (std::map<unsigned int, sCaddieInfo>::const_iterator it =
				 Doc()->m_caddieMap.begin();
			it != Doc()->m_caddieMap.end(); ++it)
		{
			if ((*it).second.tid == tid)
			{
				guid = (*it).second.guid;
				break;
			}
		}
		break;
	case CATEGORY_BALL:
		for (std::list<sItemInfo>::const_iterator it =
				 Doc()->m_ballList.begin();
			it != Doc()->m_ballList.end(); ++it)
		{
			if ((*it).tid == tid)
			{
				guid = (*it).guid;
				break;
			}
		}
		break;
	case CATEGORY_CLUB:
		for (std::map<unsigned int, sItemInfo>::const_iterator it =
				 Doc()->m_clubSetMap.begin();
			it != Doc()->m_clubSetMap.end(); ++it)
		{
			if ((*it).second.tid == tid)
			{
				guid = (*it).second.guid;
				break;
			}
		}
		break;
	case CATEGORY_MASCOT:
		for (std::map<unsigned int, sMascotInfo>::const_iterator it =
				 Doc()->m_mascotMap.begin();
			it != Doc()->m_mascotMap.end(); ++it)
		{
			if ((*it).second.tid == tid)
			{
				guid = (*it).second.guid;
				break;
			}
		}
		break;
	}
	return guid;
}

IMPLEMENT_OBJECT(FrPlayerDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrPlayerDlg, FrForm)

ON_FRESH_VI("base", FRCMD_INIT, FrPlayerDlg::OnBaseInit)
ON_FRESH_VV("base", FRCMD_LBUTTONUP, FrPlayerDlg::OnBaseBtnUp)
ON_FRESH_VI("select", FRCMD_INIT, FrPlayerDlg::OnSelectInit)
ON_FRESH_VI("pet", FRCMD_INIT, FrPlayerDlg::OnPetInit)
ON_FRESH_VV("pet", FRCMD_OWNERDRAW, FrPlayerDlg::OnPetOwnerDraw)
ON_FRESH_VI("charprev", FRCMD_INIT, FrPlayerDlg::OnCharPrevInit)
ON_FRESH_VV("charprev", FRCMD_LBUTTONUP, FrPlayerDlg::OnCharPrevBtnUp)
ON_FRESH_VI("charnext", FRCMD_INIT, FrPlayerDlg::OnCharNextInit)
ON_FRESH_VV("charnext", FRCMD_LBUTTONUP, FrPlayerDlg::OnCharNextBtnUp)
ON_FRESH_VI("caddie", FRCMD_INIT, FrPlayerDlg::OnCaddieInit)
ON_FRESH_VI("caddie_limit", FRCMD_INIT, FrPlayerDlg::OnCaddieLimitInit)
ON_FRESH_VI("caddieprev", FRCMD_INIT, FrPlayerDlg::OnCaddiePrevInit)
ON_FRESH_VV("caddieprev", FRCMD_LBUTTONUP, FrPlayerDlg::OnCaddiePrevBtnUp)
ON_FRESH_VI("caddienext", FRCMD_INIT, FrPlayerDlg::OnCaddieNextInit)
ON_FRESH_VV("caddienext", FRCMD_LBUTTONUP, FrPlayerDlg::OnCaddieNextBtnUp)
ON_FRESH_VI(RB_STR_CLUB, FRCMD_INIT, FrPlayerDlg::OnClubInit)
ON_FRESH_VI("club_limit", FRCMD_INIT, FrPlayerDlg::OnClubLimitInit)
ON_FRESH_VI("clubprev", FRCMD_INIT, FrPlayerDlg::OnClubPrevInit)
ON_FRESH_VV("clubprev", FRCMD_LBUTTONUP, FrPlayerDlg::OnClubPrevBtnUp)
ON_FRESH_VI("clubnext", FRCMD_INIT, FrPlayerDlg::OnClubNextInit)
ON_FRESH_VV("clubnext", FRCMD_LBUTTONUP, FrPlayerDlg::OnClubNextBtnUp)
ON_FRESH_VI("ball", FRCMD_INIT, FrPlayerDlg::OnBallInit)
ON_FRESH_VI("ballprev", FRCMD_INIT, FrPlayerDlg::OnBallPrevInit)
ON_FRESH_VV("ballprev", FRCMD_LBUTTONUP, FrPlayerDlg::OnBallPrevBtnUp)
ON_FRESH_VI("ballnext", FRCMD_INIT, FrPlayerDlg::OnBallNextInit)
ON_FRESH_VV("ballnext", FRCMD_LBUTTONUP, FrPlayerDlg::OnBallNextBtnUp)
ON_FRESH_VI("name", FRCMD_INIT, FrPlayerDlg::OnNameInit)
ON_FRESH_VI("remove", FRCMD_INIT, FrPlayerDlg::OnRemoveInit)
ON_FRESH_VV("remove", FRCMD_LBUTTONUP, FrPlayerDlg::OnRemoveBtnUp)
ON_FRESH_VI("index", FRCMD_INIT, FrPlayerDlg::OnIndexInit)
ON_FRESH_VI("powerbar", FRCMD_INIT, FrPlayerDlg::OnPowerBarInit)
ON_FRESH_VI("controlbar", FRCMD_INIT, FrPlayerDlg::OnControlBarInit)
ON_FRESH_VI("impactbar", FRCMD_INIT, FrPlayerDlg::OnImpactBarInit)
ON_FRESH_VI("spinbar", FRCMD_INIT, FrPlayerDlg::OnSpinBarInit)
ON_FRESH_VI("curvebar", FRCMD_INIT, FrPlayerDlg::OnCurveBarInit)
ON_FRESH_VI("epower", FRCMD_INIT, FrPlayerDlg::OnPowerEditInit)
ON_FRESH_VI("econtrol", FRCMD_INIT, FrPlayerDlg::OnControlEditInit)
ON_FRESH_VI("eimpact", FRCMD_INIT, FrPlayerDlg::OnImpactEditInit)
ON_FRESH_VI("espin", FRCMD_INIT, FrPlayerDlg::OnSpinEditInit)
ON_FRESH_VI("ecurve", FRCMD_INIT, FrPlayerDlg::OnCurveEditInit)

END_FRESH_MSGMAP()

FrPlayerDlg::~FrPlayerDlg()
{
	HidePet();
}

void FrPlayerDlg::Init(int index)
{
	m_index = index;
	if (m_pIndex)
		m_pIndex->SetBgImg(MakeStr("%dp", index + 1));
	if (m_pRemove)
		m_pRemove->SetVisible(m_index >= 2);
	SetVisible(strlen(Doc()->m_userInfo[m_index].info.sID) != 0);
}

void FrPlayerDlg::ShowPet(bool bShow)
{
	HidePet();
	if (m_pPet && strlen(Doc()->m_userInfo[m_index].info.sID) != 0)
	{
		WRect rect = m_pPet->GetRect();
		m_pExhibition = new CExhibitionByHardwareTnL(rect.x + rect.w * 0.5f,
			rect.y + rect.h * 0.5f, 0.35f, true);
		m_pExhibition->SetDistanceRange(15.0f, 25.0f);
		m_pExhibition->SetDistance(20.0f, true, false);
		m_pExhibition->SetArea(rect.w, rect.h);
		m_pPartTidList = new CPartTidList;
		if (bShow)
			SetRandomState();
		else
			SetState();
	}
}

void FrPlayerDlg::HidePet()
{
	if (m_pExhibition)
	{
		delete m_pExhibition;
		m_pExhibition = NULL;
	}
	if (m_pPartTidList)
	{
		delete m_pPartTidList;
		m_pPartTidList = NULL;
	}
}

void FrPlayerDlg::SetState()
{
	if (strlen(Doc()->m_userInfo[m_index].info.sID) != 0)
	{
		if (m_pName)
			m_pName->SetLine(1, Doc()->m_userInfo[m_index].info.sID, 0, false,
				0);
		SetCharControl(BD_CURRENT);
		SetCaddieControl(BD_CURRENT);
		SetClubControl(BD_CURRENT);
		SetBallControl(BD_CURRENT);
	}
}

void FrPlayerDlg::SetRandomState()
{
	if (m_pName)
		m_pName->SetLine(1, Doc()->m_userInfo[m_index].info.sID, 0, false, 0);
	SetCharControl(BD_RANDOM);
	SetCaddieControl(BD_RANDOM);
	SetClubControl(BD_RANDOM);
	SetBallControl(BD_RANDOM);
}

void FrPlayerDlg::OnCustomProc(float delta)
{
	if (m_pExhibition)
		m_pExhibition->Process(delta, true);
}

void FrPlayerDlg::OnPetOwnerDraw()
{
	if (m_pExhibition)
		m_pExhibition->Display(0.0f, 0.0f);
}

void FrPlayerDlg::SetTeam(unsigned char team)
{
	m_team = team;
	if (m_pBase)
	{
		switch (team)
		{
		case 0:
			m_pBase->SetButtonImg("team_base_red", FrButton::NORMAL);
			break;
		case 1:
			m_pBase->SetButtonImg("team_base_blue", FrButton::NORMAL);
			break;
		case 2:
			m_pBase->SetButtonImg("team_base", FrButton::NORMAL);
			break;
		}
	}
}

void FrPlayerDlg::OnBaseInit(int param)
{
	m_pBase = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrPlayerDlg::OnBaseBtnUp()
{
	AfxGetTask()->GetActor("Family")
		<< MsgObject(NULL, M983(0xe3), m_index, 0, 0, 0, 0);
}

void FrPlayerDlg::OnSelectInit(int param)
{
	m_pSelect = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pSelect->SetVisible(false);
}

void FrPlayerDlg::Select()
{
	if (m_pSelect)
		m_pSelect->SetVisible(true);
}

void FrPlayerDlg::UnSelect()
{
	if (m_pSelect)
		m_pSelect->SetVisible(false);
}

void FrPlayerDlg::OnPetInit(int param)
{
	m_pPet = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnCharPrevInit(int param)
{
	m_pCharPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCharPrev->SetPushDelay(0.0f);
	m_pCharPrev->UseDblClick(false); // 983 00b1e930: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnCharPrevBtnUp()
{
	SetCharControl(BD_PREV);
}

void FrPlayerDlg::OnCharNextInit(int param)
{
	m_pCharNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCharNext->SetPushDelay(0.0f);
	m_pCharNext->UseDblClick(false); // 983 00b1e9c0: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnCharNextBtnUp()
{
	SetCharControl(BD_NEXT);
}

void FrPlayerDlg::OnCaddieInit(int param)
{
	m_pCaddie = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnCaddieLimitInit(int param)
{
	m_pCaddieLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnCaddiePrevInit(int param)
{
	m_pCaddiePrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCaddiePrev->SetPushDelay(0.0f);
	m_pCaddiePrev->UseDblClick(false); // 983 00b1eb30: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnCaddiePrevBtnUp()
{
	SetCaddieControl(BD_PREV);
}

void FrPlayerDlg::OnCaddieNextInit(int param)
{
	m_pCaddieNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCaddieNext->SetPushDelay(0.0f);
	m_pCaddieNext->UseDblClick(false); // 983 00b1ebc0: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnCaddieNextBtnUp()
{
	SetCaddieControl(BD_NEXT);
}

void FrPlayerDlg::OnClubInit(int param)
{
	m_pClub = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnClubLimitInit(int param)
{
	m_pClubLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnClubPrevInit(int param)
{
	m_pClubPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pClubPrev->SetPushDelay(0.0f);
	m_pClubPrev->UseDblClick(false); // 983 00b1ed30: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnClubPrevBtnUp()
{
	SetClubControl(BD_PREV);
}

void FrPlayerDlg::OnClubNextInit(int param)
{
	m_pClubNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pClubNext->SetPushDelay(0.0f);
	m_pClubNext->UseDblClick(false); // 983 00b1edc0: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnClubNextBtnUp()
{
	SetClubControl(BD_NEXT);
}

void FrPlayerDlg::OnBallInit(int param)
{
	m_pBall = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerDlg::OnBallPrevInit(int param)
{
	m_pBallPrev = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBallPrev->SetPushDelay(0.0f);
	m_pBallPrev->UseDblClick(false); // 983 00b1eec0: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnBallPrevBtnUp()
{
	SetBallControl(BD_PREV);
}

void FrPlayerDlg::OnBallNextInit(int param)
{
	m_pBallNext = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBallNext->SetPushDelay(0.0f);
	m_pBallNext->UseDblClick(false); // 983 00b1ef50: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnBallNextBtnUp()
{
	SetBallControl(BD_NEXT);
}

void FrPlayerDlg::OnRemoveInit(int param)
{
	m_pRemove = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pRemove->SetPushDelay(0.0f);
	m_pRemove->UseDblClick(false); // 983 00b1efe0: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerDlg::OnRemoveBtnUp()
{
	SetVisible(false);
	if (m_pEmptyDlg)
		m_pEmptyDlg->SetVisible(true);
	Doc()->m_userInfo[m_index].info.sID[0] = 0;
	Doc()->m_userInfo[m_index].info.sNick[0] = 0;
	AfxGetTask()->GetActor("Family")
		<< MsgObject(NULL, M983(0xe2), m_index, 0, 0, 0, 0);
}

void FrPlayerDlg::OnNameInit(int param)
{
	m_pName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

const char* FrPlayerDlg::GetName()
{
	if (m_pName)
		return m_pName->GetLine(1, false);
	return NULL;
}

void FrPlayerDlg::SetName(const char* name)
{
	if (m_pName)
		m_pName->SetLine(1, name, 0, false, 0);
	strcpy(Doc()->m_userInfo[m_index].info.sID, name);
	strcpy(Doc()->m_userInfo[m_index].info.sNick, name);
}

void FrPlayerDlg::OnIndexInit(int param)
{
	m_pIndex = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 00b22230: monta o sCapacityParam983 com as partes do personagem do m_charMap (pela guid equipada), o caddie e o
// mascote pelos mapas e o numero sai pelo SetLevelNum (00b1da10).
void FrPlayerDlg::SetLevelBar()
{
	CSharedDoc* pDoc = Doc();
	unsigned int guidChar = pDoc->m_userInfo[m_index].userEquip.guidChar;
	sCapacityParam983 param;
	unsigned long tidChar = pDoc->m_userInfo[m_index].charInfo.tid;
	const char* pStat = pDoc->m_userInfo[m_index].charInfo.PCL;
	unsigned long tidClub = pDoc->m_userInfo[m_index].clubInfo.tid;
	unsigned long guidClub = pDoc->m_userInfo[m_index].clubInfo.guid;
	unsigned int guidCaddie = pDoc->m_userInfo[m_index].userEquip.guidCaddie;
	const short* pClubStat = pDoc->m_userInfo[m_index].clubInfo.PCL;
	const short* pClubSlot = pDoc->m_userInfo[m_index].clubInfo.slotUp983;
	unsigned int guidMascot = pDoc->m_userInfo[m_index].userEquip.guidMascot;
	unsigned long tidMascot;
	if (guidMascot == 0)
		tidMascot = 0;
	else
		tidMascot = pDoc->m_mascotMap.find(guidMascot)->second.tid;
	sCharacterInfo& ch = pDoc->m_charMap.find(guidChar)->second;
	const unsigned long* pParts = ch.tidParts;
	const unsigned long* pAuxParts = ch.tidAuxParts;
	unsigned long tidCaddie;
	if (guidCaddie == 0)
		tidCaddie = 0;
	else
		tidCaddie = pDoc->m_caddieMap.find(guidCaddie)->second.tid;
	unsigned char level = pDoc->m_myInfo.stat.Level;
	int penalty = CalcPowerPenalty(level);

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(255);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
	}

	for (int i = 0; i < 5; ++i)
	{
		param.type = i;
		param.tidChar = tidChar;
		param.m_8 = (unsigned long)pStat;
		param.tidParts = pParts;
		param.tidAuxParts = pAuxParts;
		param.tidClub = tidClub;
		param.clubAttrUp = pClubStat;
		param.clubSlotUp = pClubSlot;
		param.guidClub = guidClub;
		param.tidCaddie = tidCaddie;
		param.m_28 = tidMascot;
		param.level = level;
		param.m_30 = 0;
		unsigned int nCapacity = ItemManager()->GetLevel(&param);
		unsigned int nLevel = nCapacity;
		if (i == 0)
		{
			if (m_pLevelBar[0])
			{
				int expand = nCapacity - penalty - 20;
				m_pLevelBar[1]->SetExpand(-Max(0, expand));
				m_pLevelBar[2]->SetExpand(-Max(0, expand));
			}
		}
		else if ((unsigned int)(i - 1) < 2)
		{
			status_t status = ItemManager()->GetLevelWithPenalty(&param);
			nLevel = status.level;
		}
		SetLevelNum((unsigned char)i, (unsigned char)nLevel, (unsigned char)nCapacity);
		if (m_pLevelBar[i])
			m_pLevelBar[i]->SetDestPos(nCapacity, false);
	}
}

void FrPlayerDlg::SetLevelNum(unsigned char type, unsigned char level,
	unsigned char max)
{
	if (m_pLevelNum[type])
		m_pLevelNum[type]->SetLine(1,
			MakeStr("\\c0xffff0000\\c%d\\c0xff000000\\c/%d", level, max), 0,
			false, 0);
}

void FrPlayerDlg::SetCharControl(eButtonDir dir)
{
	std::map<unsigned int, sCharacterInfo>& charMap = Doc()->m_charMap;
	unsigned int size = (unsigned int)charMap.size();
	if (size != 0)
	{
		std::map<unsigned int, sCharacterInfo>::iterator it;
		if (dir == BD_RANDOM)
		{
			unsigned int n = rand() % size;
			it = charMap.begin();
			for (; it != charMap.end() && n != 0; --n)
				++it;
		}
		else
		{
			it = charMap.find(Doc()->m_userInfo[m_index].userEquip.guidChar);
		}
		if (dir == BD_PREV)
			--it;
		else if (dir == BD_NEXT)
			++it;
		Doc()->m_userInfo[m_index].userEquip.guidChar = it->second.guid;
		Doc()->m_userInfo[m_index].charInfo = it->second;
		SetCharacter();
		SetLevelBar();
		if (m_pCharPrev)
			m_pCharPrev->Enable(it != charMap.begin());
		if (m_pCharNext)
		{
			++it;
			m_pCharNext->Enable(it != charMap.end());
		}
	}
}

void FrPlayerDlg::SetCaddieControl(eButtonDir dir)
{
	std::map<unsigned int, sCaddieInfo>& caddieMap = Doc()->m_caddieMap;
	unsigned int size = (unsigned int)caddieMap.size();
	if (size == 0)
	{
		Doc()->m_userInfo[m_index].userEquip.guidCaddie = size;
		SetCaddie(size);
		if (m_pCaddiePrev)
			m_pCaddiePrev->Enable(size != 0);
		if (m_pCaddieNext)
			m_pCaddieNext->Enable(size != 0);
		return;
	}

	std::map<unsigned int, sCaddieInfo>::iterator it;
	unsigned long guid;
	if (dir == BD_RANDOM)
	{
		// guid doubles as the countdown of the random pick
		guid = rand() % size;
		it = caddieMap.begin();
		for (; it != caddieMap.end() && guid != 0; --guid)
			++it;
	}
	else
	{
		guid = Doc()->m_userInfo[m_index].userEquip.guidCaddie;
		it = caddieMap.find(guid);
	}

	switch (dir)
	{
	case BD_PREV:
		if (it == caddieMap.begin())
		{
			Doc()->m_userInfo[m_index].userEquip.guidCaddie = 0;
			SetCaddie(0);
			if (m_pCaddiePrev)
				m_pCaddiePrev->Enable(false);
		}
		else
		{
			--it;
			Doc()->m_userInfo[m_index].caddieInfo = it->second;
			SetCaddie(it->second.tid);
		}
		if (m_pCaddieNext)
			m_pCaddieNext->Enable(true);
		break;
	case BD_NEXT:
		if (guid == 0)
			it = caddieMap.begin();
		else
			++it;
		if (it == caddieMap.end())
			return;
		Doc()->m_userInfo[m_index].caddieInfo = it->second;
		SetCaddie(it->second.tid);
		if (m_pCaddiePrev)
			m_pCaddiePrev->Enable(true);
		if (m_pCaddieNext)
		{
			++it;
			m_pCaddieNext->Enable(it != caddieMap.end());
		}
		break;
	case BD_CURRENT:
		if (guid == 0)
		{
			SetCaddie(0);
			if (m_pCaddiePrev)
				m_pCaddiePrev->Enable(false);
			if (m_pCaddieNext)
				m_pCaddieNext->Enable(caddieMap.size() > 0);
		}
		else
		{
			SetCaddie(it->second.tid);
			if (m_pCaddiePrev)
				m_pCaddiePrev->Enable(true);
			if (m_pCaddieNext)
			{
				++it;
				m_pCaddieNext->Enable(it != caddieMap.end());
			}
		}
		break;
	case BD_RANDOM:
		Doc()->m_userInfo[m_index].caddieInfo = it->second;
		SetCaddie(it->second.tid);
		if (m_pCaddiePrev)
			m_pCaddiePrev->Enable(true);
		if (m_pCaddieNext)
		{
			++it;
			m_pCaddieNext->Enable(it != caddieMap.end());
		}
		break;
	default:
		return;
	}
	SetLevelBar();
}

void FrPlayerDlg::SetClubControl(eButtonDir dir)
{
	std::map<unsigned int, sItemInfo>& clubMap = Doc()->m_clubSetMap;
	unsigned int size = (unsigned int)clubMap.size();
	if (size != 0)
	{
		std::map<unsigned int, sItemInfo>::iterator it;
		if (dir == BD_RANDOM)
		{
			unsigned int n = rand() % size;
			it = clubMap.begin();
			for (; it != clubMap.end() && n != 0; --n)
				++it;
		}
		else
		{
			it = clubMap.find(Doc()->m_userInfo[m_index].clubInfo.guid);
		}
		if (dir == BD_PREV)
			--it;
		else if (dir == BD_NEXT)
			++it;
		Doc()->m_userInfo[m_index].clubInfo.tid = it->second.tid;
		Doc()->m_userInfo[m_index].clubInfo.guid = it->second.guid;
		memcpy(Doc()->m_userInfo[m_index].clubInfo.PCL, it->second.Common,
			sizeof(Doc()->m_userInfo[m_index].clubInfo.PCL));
		SetClub(it->second.tid);
		SetLevelBar();
		if (m_pClubPrev)
			m_pClubPrev->Enable(it != clubMap.begin());
		if (m_pClubNext)
		{
			++it;
			m_pClubNext->Enable(it != clubMap.end());
		}
	}
}

void FrPlayerDlg::SetBallControl(eButtonDir dir)
{
	// 00b22760: the list holds runs of equal tid; prev/next/random move by run
	std::list<sItemInfo>& ballList = Doc()->m_ballList;
	if (ballList.empty())
		return;

	std::list<sItemInfo>::iterator it;
	if (dir == BD_RANDOM)
	{
		unsigned int runs = 1;
		unsigned long last = ballList.front().tid;
		for (it = ballList.begin(); it != ballList.end(); ++it)
		{
			if (it->tid != last)
			{
				++runs;
				last = it->tid;
			}
		}
		unsigned int n = rand() % runs;
		unsigned long prevTid = 0;
		for (it = ballList.begin(); it != ballList.end(); ++it)
		{
			if (it->tid != prevTid)
			{
				if (n == 0)
					break;
				--n;
				prevTid = it->tid;
			}
		}
	}
	else
	{
		for (it = ballList.begin(); it != ballList.end() &&
			it->tid != Doc()->m_userInfo[m_index].userEquip.tidBall;
			++it)
		{
		}
	}

	if (dir == BD_PREV)
	{
		if (it != ballList.begin())
		{
			unsigned long tid = it->tid;
			--it;
			while (it != ballList.begin() && it->tid == tid)
				--it;
		}
	}
	else if (dir == BD_NEXT)
	{
		unsigned long tid = it->tid;
		++it;
		while (it != ballList.end() && it->tid == tid)
			++it;
		if (it == ballList.end())
			--it;
	}

	Doc()->m_userInfo[m_index].userEquip.tidBall = it->tid;
	SetBall(it->tid);
	SetLevelBar();
	std::list<sItemInfo>::iterator nx = it;
	++nx;
	while (nx != ballList.end() && nx->tid == it->tid)
		++nx;
	m_pBallNext->Enable(nx != ballList.end());
	if (m_pBallPrev)
		m_pBallPrev->Enable(nx != ballList.begin());
}

void FrPlayerDlg::SetCaddie(unsigned long tid)
{
	if (m_pCaddie)
	{
		Doc()->m_userInfo[m_index].caddieInfo.tid = tid;
		IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(tid);
		if (pCaddie)
			m_pCaddie->SetBgImg(pCaddie->c.Icon);
		else
			m_pCaddie->SetBgImg("caddie_no");
		if (m_pCaddieLimit)
		{
			if (pCaddie)
			{
				if (Doc()->m_myInfo.stat.Level < (pCaddie->c.Level & 0x7f))
					m_pCaddieLimit->SetBgImg("unused");
				else
					m_pCaddieLimit->SetBgImg("caddie_rest");
				unsigned long guid = Doc()->m_userInfo[m_index].caddieInfo.guid;
				Doc()->m_userInfo[m_index].userEquip.guidCaddie = guid;
				if (guid != 0)
				{
					std::map<unsigned int, sCaddieInfo>::iterator it =
						Doc()->m_caddieMap.find(guid);
					if (it == Doc()->m_caddieMap.end())
					{
						m_pCaddieLimit->SetVisible(false);
						return;
					}
					pCaddie = ItemManager()->FindCaddie(it->second.tid);
					if (pCaddie == NULL)
						return;
					if ((it->second.Rent_flag && it->second.Remain_Date == 0 &&
							pCaddie->MonthlyFee) ||
						Doc()->m_myInfo.stat.Level < (pCaddie->c.Level & 0x7f))
					{
						m_pCaddieLimit->SetVisible(true);
						return;
					}
				}
				m_pCaddieLimit->SetVisible(false);
				return;
			}
			m_pCaddieLimit->SetVisible(false);
		}
	}
}

void FrPlayerDlg::SetCharacter()
{
	if (m_pPartTidList)
		m_pPartTidList->SetTids(&Doc()->m_userInfo[m_index].charInfo, 0xff);
	if (m_pExhibition)
		m_pExhibition->SetModel(*m_pPartTidList,
			Doc()->m_userInfo[m_index].charInfo.tidAuxParts, NULL, 3.0f,
			m_index);
}

void FrPlayerDlg::SetClub(unsigned long guid)
{
	if (m_pClub)
	{
		IFF_STRUCT::sClubSet* pClubSet = ItemManager()->FindClubSet(guid);
		if (pClubSet)
		{
			m_pClub->SetBgImg(pClubSet->c.Icon);
			if (m_pClubLimit)
				m_pClubLimit->SetVisible(
					Doc()->m_myInfo.stat.Level < (pClubSet->c.Level & 0x7f));
		}
	}
}

void FrPlayerDlg::SetBall(unsigned long tid)
{
	if (m_pBall)
	{
		IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(tid);
		if (pBall && m_pBall)
			m_pBall->SetBgImg(pBall->c.Icon);
	}
}

void FrPlayerDlg::OnPowerBarInit(int param)
{
	m_pLevelBar[0] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
}

void FrPlayerDlg::OnControlBarInit(int param)
{
	m_pLevelBar[1] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pLevelBar[1]->SetRange(0, 30, 0);
}

void FrPlayerDlg::OnImpactBarInit(int param)
{
	m_pLevelBar[2] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pLevelBar[2]->SetRange(0, 30, 0);
}

void FrPlayerDlg::OnSpinBarInit(int param)
{
	m_pLevelBar[3] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pLevelBar[3]->SetRange(0, 30, 0);
}

void FrPlayerDlg::OnCurveBarInit(int param)
{
	m_pLevelBar[4] = DYNAMIC_CAST(FrGaugeBar, (FrWnd*)param);
	m_pLevelBar[4]->SetRange(0, 30, 0);
}

void FrPlayerDlg::OnPowerEditInit(int param)
{
	m_pLevelNum[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrPlayerDlg::OnControlEditInit(int param)
{
	m_pLevelNum[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrPlayerDlg::OnImpactEditInit(int param)
{
	m_pLevelNum[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrPlayerDlg::OnSpinEditInit(int param)
{
	m_pLevelNum[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrPlayerDlg::OnCurveEditInit(int param)
{
	m_pLevelNum[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

IMPLEMENT_OBJECT(FrPlayerEmptyDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrPlayerEmptyDlg, FrForm)

ON_FRESH_VI("base", FRCMD_INIT, FrPlayerEmptyDlg::OnBaseInit)
ON_FRESH_VI("create", FRCMD_INIT, FrPlayerEmptyDlg::OnCreateInit)
ON_FRESH_VV("create", FRCMD_LBUTTONUP, FrPlayerEmptyDlg::OnCreateBtnUp)
ON_FRESH_VI("index", FRCMD_INIT, FrPlayerEmptyDlg::OnIndexInit)

END_FRESH_MSGMAP()

void FrPlayerEmptyDlg::Init(int index)
{
	m_index = index;
	if (m_pIndex)
		m_pIndex->SetBgImg(MakeStr("%dp", index + 1));
	SetVisible(strlen(Doc()->m_userInfo[m_index].info.sID) == 0);
}

void FrPlayerEmptyDlg::OnBaseInit(int param)
{
	m_pBase = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrPlayerEmptyDlg::OnCreateInit(int param)
{
	m_pCreate = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pCreate->SetPushDelay(0.0f);
	m_pCreate->UseDblClick(false); // 983 00b1f660: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrPlayerEmptyDlg::OnCreateBtnUp()
{
	SetVisible(false);
	if (m_pPlayerDlg)
	{
		sprintf(Doc()->m_userInfo[m_index].info.sID, "PLAYER%d", m_index + 1);
		sprintf(Doc()->m_userInfo[m_index].info.sNick, "PLAYER%d", m_index + 1);
		m_pPlayerDlg->ShowPet(true);
		m_pPlayerDlg->SetVisible(true);
	}
	AfxGetTask()->GetActor("Family")
		<< MsgObject(NULL, M983(0xe2), m_index, 1, 0, 0, 0);
	AfxGetTask()->GetActor("Family")
		<< MsgObject(NULL, M983(0xe3), m_index, 0, 0, 0, 0);
}

void FrPlayerEmptyDlg::OnIndexInit(int param)
{
	m_pIndex = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}
