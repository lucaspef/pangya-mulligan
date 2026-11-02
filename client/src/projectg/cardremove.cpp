#include "minatl.h"
#include "cardremove.h"
#include "frbutton.h"
#include "frarea.h"
#include "frlistbox.h"
#include "ucclibrary.h"
#include "projectg.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;

static unsigned long s_cardRemoveItems[4] = { 0x1a0000c2, 0x1a0000cd,
	0x1a0000ce, 0x1a0000cf };

bool FrCardRemove::IsCardRemoveItem(unsigned long tid)
{
	bool found = false;
	for (int i = 0; i < 4; ++i)
	{
		if (s_cardRemoveItems[i] == tid)
		{
			found = true;
			break;
		}
	}
	return found;
}

FrCardRemove::FrCardRemove()
{
	m_pDrawArea = NULL;
	m_pItemList = NULL;
	memset(m_pBitmap983, 0, sizeof(m_pBitmap983));
	m_removeItemTid = 0;
	m_removeItemGuid = 0;
	m_curPartTid = 0;
	m_curPartUid = 0;
	m_cardList.clear();
}

FrCardRemove::~FrCardRemove()
{
}

// 983 007fe440
bool FrCardRemove::OnInit()
{
	for (int i = 0; i < 6; ++i)
		m_pBitmap983[i] = g_pFresh->GetBitmap(MakeStr("card_remove_bg_%d", i + 1));
	m_pBitmap983[6] = g_pFresh->GetBitmap("itemlist_button_n");
	m_pBitmap983[7] = g_pFresh->GetBitmap("itemlist_button_o");
	return FrForm::OnInit();
}

void FrCardRemove::DrawInDlg(const char* name, float x, float y)
{
	if (name)
	{
		const Bitmap* bitmap = g_pFresh->GetBitmap(name);
		if (bitmap)
		{
			FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
			if (gdi)
			{
				WRect rect = m_rect;
				rect.x = (x + 2.0f) + rect.x;
				rect.y = (y + 23.0f) + rect.y;
				rect.w = bitmap->Width();
				rect.h = bitmap->Height();
				gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
			}
		}
	}
}

void FrCardRemove::DrawTexture(const char* name, float x, float y,
	unsigned long color)
{
	if (name)
	{
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (gdi)
		{
			const Bitmap* bitmap = g_pFresh->GetBitmap(name);
			if (bitmap)
			{
				WRect rect(x, y, bitmap->Width(), bitmap->Height());
				gdi->DrawTexture(bitmap, rect, color, 0);
			}
		}
	}
}

void FrCardRemove::PrintText(const char* text, float x, float y,
	unsigned long color)
{
	if (text)
	{
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (gdi)
		{
			gdi->SetTextColor(color, 0xffffffff);
			WPoint pos(x, y);
			gdi->PrintText(pos, 0, text, NULL);
			gdi->SetTextColor(0xff000000, 0xffffffff);
		}
	}
}

// 983 007ff5e0: peca (tid>>26 == 2) = em algum personagem; taco (== 4) = o taco em uso (CSharedDoc +0xa2e)
bool FrCardRemove::CheckEquiped(unsigned long tid, unsigned long guid)
{
	if ((tid >> 26) == 2)
	{
		for (std::map<unsigned int, sCharacterInfo>::iterator it =
				 Doc()->m_charMap.begin();
			it != Doc()->m_charMap.end(); ++it)
		{
			for (int i = 0; i < 24; ++i)
			{
				if ((*it).second.ItemIdList[i] &&
					(*it).second.ItemIdList[i] == guid)
					return true;
			}
		}
	}
	else if ((tid >> 26) == 4 &&
		guid == Doc()->m_myInfo.userEquip.guidClubSet)
		return true;
	return false;
}

void FrCardRemove::SetCardRemoveItem(unsigned long tid, unsigned long guid)
{
	m_removeItemTid = tid;
	m_removeItemGuid = guid;
}

bool FrCardRemove::SetCurPartUid(unsigned long uid)
{
	bool changed = false;
	if (uid != m_curPartUid)
	{
		m_curPartUid = uid;
		changed = true;
	}
	return changed;
}

void FrCardRemove::BuildCurPartCardList()
{
	m_cardList.clear();
	std::list<sSCardAvilityPeriodInfo>::iterator end =
		Doc()->m_cardAbilityList[0].end();
	for (std::list<sSCardAvilityPeriodInfo>::iterator it =
			 Doc()->m_cardAbilityList[0].begin();
		it != end; ++it)
	{
		if ((*it).partsUid == m_curPartUid)
			m_cardList.push_back((*it).tid);
	}
}

IMPLEMENT_OBJECT(FrCardRemove, FrForm)

BEGIN_FRESH_MSGMAP(FrCardRemove, FrForm)

ON_FRESH_VI("draw_area", FRCMD_INIT, FrCardRemove::OnDrawAreaInit)
ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW, FrCardRemove::OnDrawAreaOwnerDraw)
ON_FRESH_VI("item_list", FRCMD_INIT, FrCardRemove::OnItemListInit)
ON_FRESH_VI("item_list", FRCMD_OWNERDRAW, FrCardRemove::OnItemListOwnerDraw)
ON_FRESH_VV("item_list", FRCMD_DBLCLICK, FrCardRemove::OnItemListDBClick)
ON_FRESH_VV("item_list", FRCMD_RBUTTONDOWN, FrCardRemove::OnItemListRBDown)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrCardRemove::OnConfirmInit)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN, FrCardRemove::OnConfirmLBDown)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrCardRemove::OnCancelInit)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN, FrCardRemove::OnCancelLBDown)

END_FRESH_MSGMAP()

void FrCardRemove::OnDrawAreaInit(int param)
{
	m_pDrawArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}


void FrCardRemove::OnDrawAreaOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi || !m_pItemList)
		return;

	WRect frame = m_rect;
	DrawInDlg("card_remove_bg_1", 0.0f, 0.0f);
	DrawInDlg("card_remove_bg_2", 256.0f, 0.0f);
	DrawInDlg("card_remove_bg_3", 512.0f, 0.0f);
	DrawInDlg("card_remove_bg_4", 0.0f, 256.0f);
	DrawInDlg("card_remove_bg_5", 256.0f, 256.0f);
	DrawInDlg("card_remove_bg_6", 512.0f, 256.0f);

	FrListItem* selected = m_pItemList->GetSelected();
	if (!selected)
	{
		gdi->SetTextColor(0xffff0000, 0xffffffff);
		gdi->Print(WPoint(frame.x + 410.0f, frame.y + 350.0f), 1,
			K2L_Compatibility("\304\253\265\345\270\246 \272\320\270\256\307\322 \300\307\273\363\300\273 "
			"\274\261\305\303\307\330\301\326\274\274\277\344."));
		gdi->SetTextColor(0xffffffff, 0xffffffff);
		return;
	}

	sItemInfo* info = (sItemInfo*)selected->pData;
	if (!info)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(info->tid);
	if (!common)
		return;

	gdi->SetTextStyle(1);

	if (IsLocalContent(S4_UCC) && IsUccClothes(info->guid))
		gdi->Print(WPoint(frame.x + 410.0f, frame.y + 50.0f), 1,
			GetUccItemName(info->guid));
	else
		gdi->Print(WPoint(frame.x + 410.0f, frame.y + 50.0f), 1, common->Name);
	gdi->SetTextStyle(0);

	const Bitmap* icon = g_pFresh->GetBitmap(common->Icon);
	if (IsLocalContent(S4_UCC) && IsUccClothes(info->guid))
		icon = GetUccItemIcon(info->guid);
	if (icon)
	{
		WRect rect(frame.x + 410.0f - icon->Width() * 0.5f, frame.y + 110.0f,
			(float)icon->Width(), (float)icon->Height());
		gdi->DrawTexture(icon, rect, 0xffffffff, 0);
	}

	if (SetCurPartUid(info->guid) == true)
	{
		m_curPartTid = info->tid;
		BuildCurPartCardList();
	}

	if (m_curPartUid)
	{
		float index = 0.0f;
		std::list<unsigned long>::iterator it = m_cardList.begin();
		std::list<unsigned long>::iterator end = m_cardList.end();
		for (; it != end; ++it)
		{
			IFF_STRUCT::sCard* card = ItemManager()->FindCard(*it);
			const Bitmap* bitmap = g_pFresh->GetBitmap(card->c.Icon);
			if (bitmap)
			{
				float bw = bitmap->Width();
				unsigned long bh = bitmap->Height();
				WRect rect(frame.x +
						((410.0f -
							 (m_cardList.size() * bw * 0.5f + index * 5.0f)) +
							(bw + 5.0f) * index),
					frame.y + 215.0f, (float)bitmap->Width(), (float)bh);
				gdi->DrawTexture(bitmap, rect, 0xffffffff, 0);
				index += 1.0f;
			}
		}
		gdi->SetTextColor(0xffff0000, 0xffffffff);
		gdi->Print(WPoint(frame.x + 410.0f, frame.y + 350.0f), 1,
			K2L_Compatibility("\273\363\261\342\300\307 \276\306\300\314\305\333\300\270\267\316 "
			"\272\320\270\256 \265\313\264\317\264\331."));
		gdi->SetTextColor(0xffffffff, 0xffffffff);
	}
}


void FrCardRemove::OnItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pItemList)
	{
		m_pItemList->UseDblClick(true);
		m_pItemList->UseRightButton(true);
		std::list<sItemInfo>::iterator it;
		std::list<sItemInfo>::iterator end = Doc()->m_partsList.end();
		for (it = Doc()->m_partsList.begin(); it != end; it++)
		{
			sItemInfo& item = *it;
			unsigned long guid = item.guid;
			if (!CheckEquiped(item.tid, guid))
			{
				std::list<sSCardAvilityPeriodInfo>::iterator card;
				std::list<sSCardAvilityPeriodInfo>::iterator cardEnd =
					Doc()->m_cardAbilityList[0].end();
				for (card = Doc()->m_cardAbilityList[0].begin();
					card != cardEnd; card++)
				{
					if ((*card).partsUid == guid)
					{
						IFF_STRUCT::sPart* part =
							ItemManager()->FindPart(item.tid);
						if (part)
						{
							item.CharacterSlotNum = part->CharacterSlot;
							item.CaddieSlotNum = part->CaddieSlot;
							// 983 007ff6c0: 3a contagem (sPart+0x22c -> sItemInfo+0xa8)
							item.NpcSlotNum983 = part->NpcSlot983;
							m_pItemList->AddItem(&item);
						}
						break;
					}
				}
			}
		}
		for (it = Doc()->m_uccItemList.begin();
			it != Doc()->m_uccItemList.end(); it++)
		{
			sItemInfo& item = *it;
			unsigned long guid = item.guid;
			if (!CheckEquiped(item.tid, guid))
			{
				std::list<sSCardAvilityPeriodInfo>::iterator card;
				std::list<sSCardAvilityPeriodInfo>::iterator cardEnd =
					Doc()->m_cardAbilityList[0].end();
				for (card = Doc()->m_cardAbilityList[0].begin();
					card != cardEnd; card++)
				{
					if ((*card).partsUid == guid)
					{
						IFF_STRUCT::sPart* part =
							ItemManager()->FindPart(item.tid);
						if (part)
						{
							item.CharacterSlotNum = part->CharacterSlot;
							item.CaddieSlotNum = part->CaddieSlot;
							// 983 007ff6c0: 3a contagem (sPart+0x22c -> sItemInfo+0xa8)
							item.NpcSlotNum983 = part->NpcSlot983;
							m_pItemList->AddItem(&item);
						}
						break;
					}
				}
			}
		}
		// 983 007ff6c0: tacos fora de uso com carta (sClubSet +0xfa/+0xfc)
		for (CLUBSET_MAP_IT club = Doc()->m_clubSetMap.begin();
			club != Doc()->m_clubSetMap.end(); ++club)
		{
			sItemInfo& item = club->second;
			unsigned long guid = item.guid;
			if (!CheckEquiped(item.tid, guid))
			{
				std::list<sSCardAvilityPeriodInfo>::iterator card;
				std::list<sSCardAvilityPeriodInfo>::iterator cardEnd =
					Doc()->m_cardAbilityList[0].end();
				for (card = Doc()->m_cardAbilityList[0].begin();
					card != cardEnd; card++)
				{
					if ((*card).partsUid == guid)
					{
						IFF_STRUCT::sClubSet* clubSet =
							ItemManager()->FindClubSet(item.tid);
						if (clubSet)
						{
							item.CharacterSlotNum = clubSet->CharacterSlot983;
							item.CaddieSlotNum = clubSet->CaddieSlot983;
							m_pItemList->AddItem(&item);
						}
						break;
					}
				}
			}
		}
	}
}


// 983 b2fca0/b2fcd0 (carddlg.cpp): casas de carta de peca/taco.
void DrawPartsSlot983(WPoint pt, unsigned long typeId, unsigned long id,
	FrGraphicInterface* pGDI);

// 983 007fef10: icones e nome centrados na altura da linha (0098c6e0 = a*0.5 - b*0.5 com a = altura - 3);
// retrato do personagem so para peca (tid>>26 == 2); casas de carta de peca/taco em pItem->pos.
void FrCardRemove::OnItemListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	FrListItem* item = (FrListItem*)param;
	if (gdi && item)
	{
		sItemInfo* info = (sItemInfo*)item->pData;
		if (info)
		{
			WPoint ip(item->pos.x, item->pos.y + 2.0f);
			bool expired = info->ItemType == 6 && info->Expired;
			unsigned long color = expired ? 0x80808080 : 0xffffffff;
			// 983 007fe830(6/7): itemlist_button_n/o do OnInit (shop_myroom/cardremove), nao o button_n do armazem.
			const Bitmap* bg =
				m_pBitmap983[(item->selected || item->underCursor) ? 7 : 6];
			if (bg)
				gdi->DrawTexture(bg, WRect(ip.x, ip.y, bg->Width(), bg->Height()), color, 0);
			IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(info->tid);
			if (common)
			{
				float rowH = (float)(m_pItemList->GetItemHeight() - 3);
				unsigned long kind = info->tid >> 26;
				if (kind == 2)
				{
					IFF_ITEM_COMMON* chara = ItemManager()->FindCommonItem(
						((info->tid >> 18) & 0xff) | 0x4000000);
					if (chara)
					{
						const Bitmap* bitmap =
							g_pFresh->GetBitmap(MakeStr("s_%s", chara->Icon));
						if (bitmap)
						{
							float h = (bitmap->Height() / 100.0f) * 90.0f;
							gdi->DrawTexture(bitmap,
								WRect(ip.x + 5.0f, rowH * 0.5f - h * 0.5f + ip.y,
									(bitmap->Width() / 100.0f) * 90.0f, h),
								0xffffffff, 0);
						}
					}
				}
				const Bitmap* icon = g_pFresh->GetBitmap(common->Icon);
				if (IsUccClothes(info->guid))
					icon = GetUccItemIcon(info->guid);
				if (icon)
				{
					float h = (icon->Height() / 100.0f) * 34.0f;
					gdi->DrawTexture(icon,
						WRect(ip.x + 30.0f, rowH * 0.5f - h * 0.5f + ip.y,
							(icon->Width() / 100.0f) * 34.0f, h),
						0xffffffff, 0);
				}
				if (item->selected == 1)
				{
					gdi->SetTextStyle(1);
					gdi->SetTextColor(0xffff0000, 0xffffffff);
				}
				else
				{
					gdi->SetTextStyle(0);
					gdi->SetTextColor(0xff000000, 0xffffffff);
				}
				if (!item->selected && item->underCursor)
					gdi->SetTextStyle(1);
				WPoint pos;
				pos.x = ip.x + 61.0f;
				pos.y = rowH * 0.5f - (float)gdi->GetFontHeight() * 0.5f + ip.y;
				if (IsUccClothes(info->guid))
					gdi->Print(pos, 0, GetUccItemName(info->guid));
				else
					gdi->Print(pos, 0, common->Name);
				if (info->ItemType == 6)
				{
					const Bitmap* clock = g_pFresh->GetBitmap("clock");
					if (clock)
					{
						gdi->DrawTexture(clock,
							WRect(item->pos.x + 198.0f, item->pos.y + 6.0f,
								clock->Width(), clock->Height()),
							0xffffffff, 0);
					}
				}
				if (kind == 2 || kind == 4)
					DrawPartsSlot983(item->pos, info->tid, info->guid, gdi);
				gdi->SetTextStyle(0);
				gdi->SetTextColor(0xff000000, 0xffffffff);
			}
		}
	}
}


void FrCardRemove::OnItemListDBClick()
{
}

void FrCardRemove::OnItemListRBDown()
{
	FrListItem* item = m_pItemList->GetItemUnderCursor();
	if (item)
	{
		if (!item->selected)
			m_pItemList->ToggleItem(item);
		// 983: manda a mensagem 0x108 (abrir informacao) ao ator principal da task
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, 0x108, (int)item, 0, 0, 0, 0));
	}
}

void FrCardRemove::OnConfirmInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
		button->SetPushDelay(0.1f);
}

void FrCardRemove::OnConfirmLBDown()
{
	FrForm* form;
	if (m_curPartUid)
	{
		FrCardRemoveConfirm* confirm = CreateForm<FrCardRemoveConfirm>(
			g_pFresh->GetManager(), this, "card_remove_confirm", NULL);
		if (!confirm)
			return;
		confirm->SetCaption(K2L_Compatibility("\304\253\265\345\303\337\303\342\276\327 "
							"\273\347\277\353\310\256\300\316"));
		confirm->SetPart(m_curPartTid, m_curPartUid);
		confirm->Open((FRESH_PFN_RESULT)&FrCardRemove::OnUseCardRemoveConfirm,
			0);
	}
	else
	{
		form = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		form->SetTimeLimit(5.0f);
		form->SetMessage(K2L_Compatibility("\270\325\300\372 \304\253\265\345\270\246 "
						 "\272\320\270\256\307\322 \300\307\273\363\300\273 "
						 "\274\261\305\303\307\330\301\326\274\274\277\344."),
			false);
		form->Open(NULL, 0);
	}
}

bool FrCardRemove::OnUseCardRemoveConfirm(int result, FrForm* form)
{
	if (result == 1)
	{
		if (IsCardRemoveItem(m_removeItemTid))
		{
			WSendPacket packet((enumClientPacket)0xe0);
			packet.Encode4(m_removeItemTid);
			packet.Encode4(m_removeItemGuid);
			packet.Encode4(m_curPartTid);
			packet.Encode4(m_curPartUid);
			packet.Send(TO_GAME);
		}
		Close(true);
	}
	return true;
}

void FrCardRemove::OnCancelInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
		button->SetPushDelay(0.1f);
}

void FrCardRemove::OnCancelLBDown()
{
	Close(true);
}

FrCardRemoveConfirm::FrCardRemoveConfirm()
	: m_partTid(0), m_partUid(0)
{
}

FrCardRemoveConfirm::~FrCardRemoveConfirm()
{
}

IMPLEMENT_OBJECT(FrCardRemoveConfirm, FrForm)

BEGIN_FRESH_MSGMAP(FrCardRemoveConfirm, FrForm)

ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW,
	FrCardRemoveConfirm::OnDrawAreaOwnerDraw)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrCardRemoveConfirm::OnConfirmInit)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN,
	FrCardRemoveConfirm::OnConfirmLBDown)
ON_FRESH_VI("btn_cancel", FRCMD_INIT, FrCardRemoveConfirm::OnCancelInit)
ON_FRESH_VV("btn_cancel", FRCMD_LBUTTONDOWN,
	FrCardRemoveConfirm::OnCancelLBDown)

END_FRESH_MSGMAP()

void FrCardRemoveConfirm::OnDrawAreaOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (gdi)
	{
		WRect rect = m_rect;
		WPoint pos(rect.w * 0.5f + rect.x, rect.y + 55.0f);
		gdi->PrintText(pos, 1,
			K2L_Compatibility("\304\253\265\345\303\337\303\342\276\327\300\273 "
			"\273\347\277\353\307\317\275\303\260\332\275\300\264\317\261\356?"),
			NULL);
	}
}

void FrCardRemoveConfirm::OnConfirmInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
		button->SetPushDelay(0.1f);
}

void FrCardRemoveConfirm::OnConfirmLBDown()
{
	if (m_partTid && m_partUid)
		Close(FrOK, true);
	else
		Close(true);
}

void FrCardRemoveConfirm::OnCancelInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
		button->SetPushDelay(0.1f);
}

void FrCardRemoveConfirm::OnCancelLBDown()
{
	Close(true);
}

FrCardRemoveResult::FrCardRemoveResult()
	: m_state(0),
	  m_pPartBitmap(NULL),
	  m_pMaskBitmap(NULL),
	  m_partRect(0.0f, 0.0f, 0.0f, 0.0f),
	  m_maskRect(0.0f, 0.0f, 0.0f, 0.0f),
	  m_phase(0),
	  m_itemTid(0)
{
}

FrCardRemoveResult::~FrCardRemoveResult()
{
}

IMPLEMENT_OBJECT(FrCardRemoveResult, FrForm)

BEGIN_FRESH_MSGMAP(FrCardRemoveResult, FrForm)

ON_FRESH_VI("draw_area", FRCMD_OWNERDRAW,
	FrCardRemoveResult::OnDrawAreaOwnerDraw)
ON_FRESH_VI("btn_confirm", FRCMD_INIT, FrCardRemoveResult::OnConfirmInit)
ON_FRESH_VV("btn_confirm", FRCMD_LBUTTONDOWN,
	FrCardRemoveResult::OnConfirmLBDown)

END_FRESH_MSGMAP()

bool FrCardRemoveResult::OnInit()
{
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(m_itemTid);
	if (common)
	{
		m_pPartBitmap =
			g_pFresh->GetBitmap(MakeStr("%s_for_resultdlg", common->Icon));
		m_pMaskBitmap = g_pFresh->GetBitmap("card_remover_covermask");
		if (m_pPartBitmap)
		{
			m_partRect.w = (float)m_pPartBitmap->Width();
			m_partRect.h = m_pPartBitmap->Height();
		}
		if (m_pMaskBitmap)
		{
			m_maskRect.w = m_pMaskBitmap->Width();
			m_maskRect.h = 0;
		}
	}
	return true;
}

void FrCardRemoveResult::OnProc(const float deltaTime)
{
	if (m_state == 0 || m_state == 3)
	{
		FrWnd* button = FindChildByName("btn_confirm");
		if (button && !button->IsVisible())
			button->SetVisible(true);
		return;
	}

	switch (m_phase)
	{
	case 0:
	{
		m_phase = 1;
		WRect rect = GetRect();
		SetRect(WRect(rect.x, rect.y, rect.w, rect.h + 50.0f));
		break;
	}
	case 1:
	{
		m_partRect.x = (m_rect.w - m_partRect.w) * 0.5f + m_rect.x;
		m_partRect.y = m_rect.y + 30.0f;
		m_maskRect.x = m_partRect.x;
		m_maskRect.y = m_partRect.y + 115.0f;
		m_maskRect.h += deltaTime * 20.0f;
		if (m_maskRect.h > 40.0f)
		{
			m_maskRect.h = 40.0f;
			m_phase = 2;
			WRect rect(m_rect);
			SetRect(WRect(rect.x, rect.y, rect.w, rect.h - 50.0f));
		}
		break;
	}
	case 2:
	{
		FrWnd* button = FindChildByName("btn_confirm");
		if (button && !button->IsVisible())
			button->SetVisible(true);
		break;
	}
	}
}


void FrCardRemoveResult::OnDrawAreaOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi)
		return;

	WRect rect = m_rect;
	FrWnd* button = FindChildByName("btn_confirm");
	if (button)
		rect.h = button->GetRect().y - rect.y;

	if (m_state == 0)
	{
		gdi->Print(WPoint(rect.w * 0.5f + rect.x, rect.h * 0.5f + rect.y), 1,
			K2L_Compatibility("\300\341\275\303 \310\304 \264\331\275\303 \275\303\265\265\307\330 "
			"\301\326\274\274\277\344."));
		return;
	}
	if (m_state == 3)
	{
		float cx = rect.w * 0.5f + rect.x;
		gdi->Print(WPoint(cx, rect.y + 120.0f), 1,
			K2L_Compatibility("\304\253\265\345 \260\374\267\303 \300\314\273\363 \307\366\273\363\300\314 "
			"\310\256\300\316\265\307\276\372\275\300\264\317\264\331."));
		gdi->Print(WPoint(cx, rect.y + 135.0f), 1,
			K2L_Compatibility("\260\355\260\264 \274\276\305\315\277\241 \271\256\300\307\307\330\301\326"
			"\274\274\277\344."));
		return;
	}

	if (m_phase == 1)
	{
		gdi->DrawTexture(m_pPartBitmap, m_partRect, 0xffffffff, 0);
		gdi->DrawTexture(m_pMaskBitmap, m_maskRect, 0xffffffff, 0);
		gdi->Print(WPoint(rect.w * 0.5f + rect.x, rect.y + 210.0f), 1,
			K2L_Compatibility("\304\253\265\345 \303\337\303\342\276\327\300\273 \273\347\277\353 "
			"\301\337\300\324\264\317\264\331."));
	}

	if (m_phase == 2)
	{
		if (m_state == 1)
		{
			float cx = rect.w * 0.5f + rect.x;
			float half = rect.w * 0.5f;
			gdi->Print(WPoint(cx, rect.y + 120.0f), 1,
				K2L_Compatibility("\304\253\265\345 \303\337\303\342\277\241 \274\272\260\370\307\317\277\264"
				"\275\300\264\317\264\331!"));
			gdi->Print(WPoint(cx, rect.y + 135.0f), 1,
				K2L_Compatibility("\303\337\303\342\265\310 \304\253\265\345\264\302 \304\253\265\345\272\317"
				"\300\270\267\316 \300\314\265\277 \265\313\264\317\264\331."));

			float startX = half;
			float index = 0.0f;
			std::list<unsigned long>::iterator it = m_cardList.begin();
			std::list<unsigned long>::iterator end = m_cardList.end();
			for (; it != end; ++it)
			{
				if (*it)
				{
					IFF_STRUCT::sCard* card = ItemManager()->FindCard(*it);
					const Bitmap* bitmap = g_pFresh->GetBitmap(card->c.Icon);
					if (bitmap)
					{
						unsigned long bh = bitmap->Height();
						float bw = bitmap->Width();
						WRect crect(rect.x +
								((startX -
									 (m_cardList.size() * bw * 0.5f +
										 index * 5.0f)) +
									(bw + 5.0f) * index),
							rect.y + 40.0f, (float)bitmap->Width(), (float)bh);
						gdi->DrawTexture(bitmap, crect, 0xffffffff, 0);
						index += 1.0f;
					}
				}
			}
		}
		else if (m_state == 2)
		{
			gdi->Print(WPoint(rect.w * 0.5f + rect.x, rect.h * 0.5f + rect.y),
				1,
				K2L_Compatibility("\304\253\265\345 \303\337\303\342\277\241 \275\307\306\320\307\317\277\264"
				"\275\300\264\317\264\331!"));
		}
	}
}


void FrCardRemoveResult::OnConfirmInit(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
	{
		button->SetPushDelay(0.1f);
		button->SetVisible(false);
	}
}

void FrCardRemoveResult::OnConfirmLBDown()
{
	Close(true);
}
