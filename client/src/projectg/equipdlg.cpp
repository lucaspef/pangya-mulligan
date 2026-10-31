#include "minatl.h"
#include "equipdlg.h"
#include "frdesktop.h"
#include "cardsystem.h"
#include "projectg.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrEquipDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrEquipDlg, FrForm)

ON_FRESH_VI("additem", FRCMD_INIT, FrEquipDlg::OnEquipSlotInit)
ON_FRESH_VV("additem", FRCMD_LBUTTONUP, FrEquipDlg::OnEquipSlotBtnUp)
ON_FRESH_VI("delitem", FRCMD_INIT, FrEquipDlg::OnUnequipSlotInit)
ON_FRESH_VV("delitem", FRCMD_LBUTTONUP, FrEquipDlg::OnUnequipSlotBtnUp)
ON_FRESH_VI("reset", FRCMD_INIT, FrEquipDlg::OnResetSlotInit)
ON_FRESH_VV("reset", FRCMD_LBUTTONUP, FrEquipDlg::OnResetSlotBtnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrEquipDlg::OnCloseInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrEquipDlg::OnCloseBtnUp)
ON_FRESH_VI("warehouse", FRCMD_INIT, FrEquipDlg::OnWarehouseInit)
ON_FRESH_VI("warehouse", FRCMD_OWNERDRAW, FrEquipDlg::OnWarehouseOwnerDraw)
ON_FRESH_VV("warehouse", FRCMD_LBUTTONDOWN, FrEquipDlg::OnWarehouseBtnDown)
ON_FRESH_VV("warehouse", FRCMD_RBUTTONDOWN, FrEquipDlg::OnWarehouseRBtnDown)
ON_FRESH_VV("warehouse", FRCMD_DBLCLICK, FrEquipDlg::OnWarehouseDblClick)
ON_FRESH_VI("itemslot", FRCMD_INIT, FrEquipDlg::OnItemSlotInit)
ON_FRESH_VI("itemslot", FRCMD_OWNERDRAW, FrEquipDlg::OnItemSlotOwnerDraw)
ON_FRESH_VV("itemslot", FRCMD_DBLCLICK, FrEquipDlg::OnItemSlotDblClick)

END_FRESH_MSGMAP()

FrEquipDlg::FrEquipDlg()
{
	m_pWarehouse = NULL;
	m_pItemSlot = NULL;
	memset(m_mySlot, 0, sizeof(m_mySlot));
	m_pWarehouseBtn[0] = g_pFresh->GetBitmap("equipitem_warehouse_bn_normal");
	m_pWarehouseBtn[1] = g_pFresh->GetBitmap("equipitem_warehouse_bn_over");
	m_pWarehouseBtn[2] = g_pFresh->GetBitmap("equipitem_warehouse_bn_click");
	m_pItemSelectBtn[0] = g_pFresh->GetBitmap("equipitem_itemselect_bn_over");
	m_pItemSelectBtn[1] = g_pFresh->GetBitmap("equipitem_itemselect_bn_click");
	static char* s_fontFile[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

FrEquipDlg::~FrEquipDlg()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

// 983 006336a0 (CSharedDoc, sem nome): item escondido do jogador; definida global __stdcall em scoreboard.cpp
// (a copia static daqui saia com o argumento em registro, sem o push do 983 00ad4cc8)
bool __stdcall IsHiddenItem983(unsigned long tid);

void FrEquipDlg::Init(unsigned long* slot, bool bSubtract)
{
	if (!m_pWarehouse || !m_pItemSlot)
		return;
	memcpy(m_mySlot, slot, 10 * sizeof(unsigned long));
	m_itemCount.clear();
	if (!bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
			++it)
		{
			sItemInfo* info = &*it;
			bool hidden = false; // 983 00ad4cd0: zera antes e so liga se == true
			if (IsHiddenItem983(info->tid) == true)
				hidden = true;
			IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(info->tid);
			if ((!pItem || !pItem->Artifact) &&
				hidden != true && !ItemManager()->GetCouponKind(info->tid))
			{
				if (m_itemCount.find(info->tid) == m_itemCount.end())
				{
					m_itemCount[info->tid] = info->Common[0];
					m_pWarehouse->AddItem(info);
				}
				else
					m_itemCount[info->tid] += info->Common[0];
			}
		}
		if (bSubtract)
		{
			for (int i = 0; i < 10; ++i)
			{
				FrListBox::ITEM_LIST::iterator item;
				for (item = m_pWarehouse->m_itemList.begin();
					item != m_pWarehouse->m_itemList.end(); ++item)
				{
					sItemInfo* info = (sItemInfo*)(*item)->pData;
					if (info->tid == m_mySlot[i])
					{
						if (--m_itemCount[info->tid] <= 0)
						{
							std::map<unsigned long, int>::iterator found =
								m_itemCount.find(info->tid);
							m_itemCount.erase(found);
							m_pWarehouse->_DelItem(item);
						}
						break;
					}
				}
			}
		}
	}
	if (IsLocalContent(S4_NT_SC_ITEMSLOT))
	{
		int slots = Doc()->GetNumAddItemSlotByMascot() + 8;
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			int cards = CCardManager::Instance()->GetCardPeriodSlot();
			if (cards > 0)
				slots += cards;
		}
		for (int i = 0; i < 10; ++i)
		{
			FrListItem* item = m_pItemSlot->AddItem(&m_mySlot[i]);
			if (i >= slots)
				UnequipItem(item);
		}
	}
	else
	{
		int extra = !IsLocalContent(S4_CARD_SYSTEM);
		for (int i = 0; i < 10 + extra; ++i)
			m_pItemSlot->AddItem(&m_mySlot[i]);
	}
}

void FrEquipDlg::OnEquipSlotInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
	{
		if (bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
			button->Enable(false);
		else
		{
			button->SetPushDelay(0);
			button->UseDblClick(false); // 983 00ad36d0/00ad3770/00ad3810: +0x44 |= 0x200
		}
	}
}

void FrEquipDlg::OnEquipSlotBtnUp()
{
	if (m_pWarehouse->GetSelected())
		EquipItem(m_pWarehouse->GetSelected());
}

void FrEquipDlg::OnUnequipSlotInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
	{
		if (bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
			button->Enable(false);
		else
		{
			button->SetPushDelay(0);
			button->UseDblClick(false); // 983 00ad36d0/00ad3770/00ad3810: +0x44 |= 0x200
		}
	}
}

void FrEquipDlg::OnUnequipSlotBtnUp()
{
	FrListItem* item = m_pItemSlot->GetSelected();
	if (item)
	{
		FrListBox::ITEM_LIST::iterator it;
		for (it = m_pItemSlot->m_itemList.begin();
			it != m_pItemSlot->m_itemList.end(); ++it)
			if (*it == item)
				break;
		for (++it; it != m_pItemSlot->m_itemList.end(); ++it)
		{
			if ((*it)->pData)
			{
				m_pItemSlot->SelectItem(*it, true);
				break;
			}
		}
		UnequipItem(item);
	}
}

void FrEquipDlg::OnResetSlotInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
	{
		if (bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
			button->Enable(false);
		else
		{
			button->SetPushDelay(0);
			button->UseDblClick(false); // 983 00ad36d0/00ad3770/00ad3810: +0x44 |= 0x200
		}
	}
}

void FrEquipDlg::OnResetSlotBtnUp()
{
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pItemSlot->m_itemList.begin();
		it != m_pItemSlot->m_itemList.end(); ++it)
		UnequipItem(*it);
}

void FrEquipDlg::OnCloseInit(int param)
{
}

void FrEquipDlg::OnCloseBtnUp()
{
	Close((eFormRet)(!(int)(Doc()->m_myInfo.info.IsIdentity(2) && true)), true);
}

void FrEquipDlg::OnWarehouseInit(int param)
{
	m_pWarehouse = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	C_ASSERT_VC10(offsetof(FrEquipDlg, m_pFont) == 0x168 && offsetof(FrEquipDlg, m_deleteIcon) == 0x1b4 &&
		offsetof(FrEquipDlg, m_clockIcon) == 0x1cc); // E432
	m_pWarehouse->UseRightButton(true);
	m_clockIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockIcon.rect = WRect(50, 35, m_clockIcon.pBitmap->Width(),
		m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "delete");
	m_deleteIcon.rect = WRect(50, 35, m_deleteIcon.pBitmap->Width(),
		m_deleteIcon.pBitmap->Height());
}

inline WRect WRect::operator+(WPoint& p) const
{
	return WRect(x + p.x, y + p.y, w, h);
}

void FrEquipDlg::OnWarehouseOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	sItemInfo* info = (sItemInfo*)item->pData;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* base = item->selected ? m_pWarehouseBtn[2]
		: item->underCursor             ? m_pWarehouseBtn[1]
										: m_pWarehouseBtn[0];
	if (base)
	{
		WRect dest;
		dest.x = item->pos.x;
		dest.y = item->pos.y;
		dest.w = base->Width();
		dest.h = base->Height();
		gdi->DrawTexture(base, dest, 0xffffffff, 0);
	}
	unsigned long& type = info->tid;
	if (!type)
		return;
	IFF_STRUCT::sItem* data = ItemManager()->FindItem(type);
	if (!data)
		return;
	const Bitmap* icon =
		g_pFresh->GetManager()->GetBitmap("ITEMS", data->c.Icon);
	if (icon)
	{
		WRect dest(item->pos.x + 18, item->pos.y + 11, (float)icon->Width(),
			(float)icon->Height());
		gdi->DrawTexture(icon, dest, 0xffffffff, 0);
		gdi->SetTextColor(0xff000000, 0xffffffff);
		gdi->SetTextStyle(4);
		if (info->ItemType)
		{
			if (!info->Expired)
				gdi->DrawTexture(m_clockIcon.pBitmap,
					m_clockIcon.rect + item->pos + WPoint(0, 0), 0xffffffff, 0);
		}
		else
		{
			const char* count;
			if (type & 0x02000000)
			{
				if (info->Common[0] <= 0)
					return;
				count = MakeStr("%d", info->Common[0]);
			}
			else
				count = MakeStr("%d", m_itemCount[type]);
			if (count && ItemManager()->IsDisplayItemNumber(type))
			{
				unsigned int i = 0;
				float x = item->pos.x + 70;
				for (; i < strlen(count); ++i)
					x -= m_pFont->GetCharWidth(g_view, count[i] - '0');
				m_pFont->Print(g_view, x, item->pos.y + 45, count, 0,
					0xffffffff, 0);
			}
		}
	}
}

void FrEquipDlg::OnWarehouseBtnDown()
{
	// HACK
	if (0)
		OnItemSlotInit(0);
}

void FrEquipDlg::OnWarehouseRBtnDown()
{
	FrListItem* item = m_pWarehouse->GetItemUnderCursor();
	if (item)
		OpenInformation(((sItemInfo*)item->pData)->tid);
}

void FrEquipDlg::OnWarehouseDblClick()
{
	if (!bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (m_pWarehouse->GetItemUnderCursor())
		{
			EquipItem(m_pWarehouse->GetItemUnderCursor());
			FrListItem* selected = m_pWarehouse->GetSelected();
			if (selected)
				m_pWarehouse->UnselectItem(selected);
		}
	}
}

void FrEquipDlg::OnItemSlotInit(int param)
{
	m_pItemSlot = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	CCardManager::Instance()->SetPlayerIndex(0xff);
	CCardManager::Instance()->CalcCardPeriodAndStatus();
}

void FrEquipDlg::OnItemSlotOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	unsigned long* type = (unsigned long*)item->pData;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (IsLocalContent(S3_MASCOT))
	{
		int slots = Doc()->GetNumAddItemSlotByMascot() + 8;
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			int cards = CCardManager::Instance()->GetCardPeriodSlot();
			if (cards > 0)
				slots += cards;
			if (!Doc()->GetNumAddItemSlotByMascot() && !cards)
			{
				m_mySlot[8] = 0;
				m_mySlot[9] = 0;
			}
			else if (Doc()->GetNumAddItemSlotByMascot() <= 0 || cards <= 0)
				m_mySlot[9] = 0;
		}
		if (item->idx >= slots)
		{
			const Bitmap* bitmap =
				g_pFresh->GetBitmap("equipitem_itemslot_empty");
			WRect dest(item->pos.x + 5, item->pos.y + 5, (float)bitmap->Width(),
				(float)bitmap->Height());
			gdi->DrawTexture(bitmap, dest, 0xffffffff, 0);
			return;
		}
	}
	const Bitmap* base;
	if (item->selected)
		base = m_pItemSelectBtn[1];
	else if (item->underCursor)
		base = m_pItemSelectBtn[0];
	else
		base = NULL;
	if (base)
	{
		WRect dest;
		dest.x = item->pos.x - 9;
		dest.y = item->pos.y - 9;
		dest.w = base->Width();
		dest.h = base->Height();
		gdi->DrawTexture(base, dest, 0xffffffff, 0);
	}
	if (type && *type)
	{
		IFF_STRUCT::sItem* info = ItemManager()->FindItem(*type);
		if (info)
		{
			const Bitmap* bitmap = g_pFresh->GetBitmap(info->c.Icon);
			if (bitmap)
			{
				WRect dest(item->pos.x + 4, item->pos.y + 4,
					(float)bitmap->Width(), (float)bitmap->Height());
				gdi->DrawTexture(bitmap, dest, 0xffffffff, 0);
			}
		}
	}
}

void FrEquipDlg::OnItemSlotDblClick()
{
	if (!bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		FrListItem* item = m_pItemSlot->GetItemUnderCursor();
		if (item && item->pData && *(unsigned long*)item->pData)
			UnequipItem(item);
	}
}

unsigned long* FrEquipDlg::GetMyItemSlot()
{
	return m_mySlot;
}

void FrEquipDlg::EquipItem(FrListItem* item)
{
	sItemInfo* info = (sItemInfo*)item->pData;
	unsigned long& type = info->tid;
	if (info->tid & 0x02000000)
	{
		if (Doc()->m_gameMode == 1)
		{
			AfxGetTask()->GetActor("Family") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\345\302\370\307\317\301\366 \276\312\300\272 \303\244 \310\277\267\302\300\273 \271\337\310\326\307\317\264\302 \276\306\300\314\305\333\300\324\264\317\264\331"),
				0, 0, 0, 0);
		}
		else
		{
			AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\300\345\302\370\307\317\301\366 \276\312\300\272 \303\244 \310\277\267\302\300\273 \271\337\310\326\307\317\264\302 \276\306\300\314\305\333\300\324\264\317\264\331"),
				0, 0, 0, 0);
		}
	}
	else
	{
		int slots = 10;
		if (IsLocalContent(S3_MASCOT))
			slots = Doc()->GetNumAddItemSlotByMascot() + 8;
		if (IsLocalContent(S4_CARD_SYSTEM))
		{
			if (IsLocalContent(S4_NT_SC_ITEMSLOT))
			{
				int cards = CCardManager::Instance()->GetCardPeriodSlot();
				if (cards > 0)
					slots += cards;
			}
			else
				slots += CCardManager::Instance()->GetCardPeriodSlot();
		}
		int i;
		for (i = 0; i < slots; ++i)
			if (!m_mySlot[i])
				break;
		if (i == slots)
		{
			if (Doc()->m_gameMode == 1)
			{
				AfxGetTask()->GetActor("Family") << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\264\365 \300\314\273\363 \300\345\302\370\307\322 \274\366 \276\370\275\300\264\317\264\331"),
					0, 0, 0, 0);
			}
			else
			{
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\264\365 \300\314\273\363 \300\345\302\370\307\322 \274\366 \276\370\275\300\264\317\264\331"),
					0, 0, 0, 0);
			}
		}
		else
		{
			// 983 (00632d00): 1800002d/1800002f nao repetem nos slots -> aviso 0x23 ao ator principal
			if (type == 0x1800002d || type == 0x1800002f)
			{
				for (int k = 0; k < 11; ++k)
					if (m_mySlot[k] == type)
					{
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
							(int)K2L_Compatibility("\xb4\xf5 \xc0\xcc\xbb\xf3 \xc0\xe5\xc2\xf8\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
							0, 0, 0, 0));
						return;
					}
			}
			m_mySlot[i] = type;
			std::map<unsigned long, int>::iterator it = m_itemCount.find(type);
			if (it != m_itemCount.end())
			{
				if (--m_itemCount[type] <= 0)
				{
					m_itemCount.erase(it);
					m_pWarehouse->DelItem(item);
				}
			}
		}
	}
}

void FrEquipDlg::UnequipItem(FrListItem* item)
{
	unsigned long* type = (unsigned long*)item->pData;
	if (type && *type)
	{
		if (m_itemCount.find(*type) == m_itemCount.end())
		{
			m_itemCount[*type] = 1;
			std::list<sItemInfo>::iterator it;
			for (it = Doc()->m_myItemList.begin();
				it != Doc()->m_myItemList.end(); ++it)
				if (it->tid == *type)
				{
					m_pWarehouse->AddItem(&*it);
					break;
				}
		}
		else
			m_itemCount[*type] = m_itemCount[*type] + 1;
		*type = 0;
	}
	m_pItemSlot->UnselectItem(item);
}

void FrEquipDlg::OpenInformation(unsigned long typeId)
{
	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName("information"))
		return;
	FrForm* form = CreateForm<FrForm>(g_pFresh->GetManager(), this,
		"information", NULL);
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
	FrArea* portrait = DYNAMIC_CAST(FrArea, form->FindChildByName("portrait"));
	FrButton* reset = DYNAMIC_CAST(FrButton, form->FindChildByName("reset"));
	if (reset)
		reset->SetVisible(false);
	FrButton* club = DYNAMIC_CAST(FrButton,
		form->FindChildByName("icon_clubsetworkshop"));
	if (club)
		club->SetVisible(false);
	bool bNoCommon = false;
	if (portrait)
	{
		if (!common)
		{
			portrait->SetBgImg("hide");
			bNoCommon = true;
		}
		else
			portrait->SetBgImg(common->Icon);
	}
	if (!bNoCommon && common)
	{
		FrEdit* name = DYNAMIC_CAST(FrEdit, form->FindChildByName("name"));
		FrArea* level = DYNAMIC_CAST(FrArea, form->FindChildByName("level"));
		if (name && level)
		{
			if (common->IsUnderLvl)
				name->AddText(
					MakeStr(
						K2L_Compatibility("\300\314\270\247: %s\012\012\267\271\272\247:                \300\314\307\317"),
						common->Name),
					false, true);
			else
				name->AddText(
					MakeStr(
						K2L_Compatibility("\300\314\270\247: %s\012\012\267\271\272\247:                \300\314\273\363"),
						common->Name),
					false, true);
			level->SetBgImg(MakeStr("level_%03d", common->Level + 1));
		}
		bool bTimed = false;
		std::list<sItemInfo>::iterator it = Doc()->m_myItemList.begin();
		while (it != Doc()->m_myItemList.end() && it->tid != typeId)
			++it;
		if (it != Doc()->m_myItemList.end())
		{
			int days = it->HourRemain / 24;
			int hours = it->HourRemain % 24;
			bTimed = it->ItemType != 0;
			if (bTimed)
				name->AddText(K2L_Compatibility("\012"), false, true);
			switch (it->ItemType)
			{
			case 1:
				name->AddText(
					MakeStr(
						K2L_Compatibility("\273\347\277\353\261\342\260\243: %d\300\317 %d\275\303\260\243"),
						days, hours),
					false, true);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				if (it->Expired)
					name->AddText(
						K2L_Compatibility("\\c0xffff0000\\c\273\347\277\353\261\342\260\243\\c0xff000000\\c\300\314 \\c0xffff0000\\c\270\270\267\341\\c0xff000000\\c\265\307\276\372\275\300\264\317\264\331.\012"),
						false, true);
				else if (!days && !hours)
					name->AddText(
						K2L_Compatibility("\263\262\300\272\275\303\260\243: 1\275\303\260\243\271\314\270\270\012"),
						false, true);
				else
					name->AddText(
						MakeStr(
							K2L_Compatibility("\263\262\300\272\275\303\260\243: %d\300\317 %d\275\303\260\243\012"),
							days, hours),
						false, true);
				break;
			}
		}
		FrStatic* avility =
			DYNAMIC_CAST(FrStatic, form->FindChildByName("static_avility"));
		if (avility)
		{
			WRect rect = avility->GetRect();
			if (bTimed)
				rect.y = (float)(rect.y + 25.0);
			else
				rect.y = (float)(rect.y - 3.0);
			avility->SetRect(rect);
		}
		IFF_STRUCT::sDesc* desc = ItemManager()->FindDesc(common->TypeId);
		if (desc)
			form->SetMessage(desc->Desc, false);
	}
	else
	{
		FrEdit* name = DYNAMIC_CAST(FrEdit, form->FindChildByName("name"));
		if (name)
			name->AddText(
				K2L_Compatibility("\300\314\270\247: \272\322\270\355\012\012\267\271\272\247: \272\322\270\355"),
				false, true);
		form->SetMessage(
			K2L_Compatibility("\272\243\300\317\277\241 \275\316\300\316 \301\244\303\274\272\322\270\355\300\307 \301\270\300\347"),
			false);
	}
	form->Open(NULL, 3);
}
