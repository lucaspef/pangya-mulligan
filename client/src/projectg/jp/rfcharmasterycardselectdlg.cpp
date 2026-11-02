// JP 983: unidade 005b7f10–005bd040 — janelas RF das cartas da maestria: RFCharMasteryCardNoticeDlg,
// RFCharMasteryCardSelectColumn, RFCharMasteryCardSelectDlg, RFCharacterMasteryCardDetachDlg, RFCardIcon e RFCharMasteryCardDlg.
// Começa em 005b7f10 (o corte da powershotitemobject ia até 005b8320; a F3.7.1 começava esta em 005b8320) com as funções de
// encaixe e os acessores de instância; 005bd040+ é a rfcharmasteryaboutdlg (acessores dela e as mesmas funções de encaixe).
// Sessão B142. Nomes meus (…983), menos as classes e os da RF; ver include/jp/refresh/rfcharmasterycardselectdlg.h.
#include "stdafx.h"
#include "refresh/rfcharmasterycardselectdlg.h"
#include "refresh/rfmasterytooltipwnd.h"
#include "refresh/rfuseitemdlg.h"
#include "wlocalize.h"
#include "projectg.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "soundmanager.h"
#include "gamestringmanager.h"
#include "../../shared/itemmanager.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "lobbyavataractor.h"
#include <algorithm>

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho
std::wstring MultiByteToWide983(const std::string& text); // 005e4ef0 (textconv.cpp)

RFCharMasteryCardNoticeDlg* RFCharMasteryCardNoticeDlg::ms_pInstance;
RFWnd* RFCharMasteryCardNoticeDlg::s_pParent983;
RFWnd* RFCharMasteryCardSelectColumn::s_pParent983;
RFCharMasteryCardSelectDlg* RFCharMasteryCardSelectDlg::ms_pInstance;
RFWnd* RFCharMasteryCardSelectDlg::s_pParent983;
RFCharacterMasteryCardDetachDlg* RFCharacterMasteryCardDetachDlg::ms_pInstance;
RFWnd* RFCharacterMasteryCardDetachDlg::s_pParent983;
RFWnd* RFCardIcon::s_pParent983;
RFCharMasteryCardDlg* RFCharMasteryCardDlg::ms_pInstance;
RFWnd* RFCharMasteryCardDlg::s_pParent983;

// 005b7f10 (sem chamador): modelo 3D na metade da tela da RF (800x600 sem RFManager), distância pela altura, presa em [min, max]
void FitExhibition983(CExhibition983* pExh, float minDist, float maxDist)
{
	if (pExh)
	{
		RFManager* manager = RFSceneManager::GetManager983();
		RFSize size = manager ? manager->GetRoot983()->GetSize() : RFSize(800.0f, 600.0f);
		pExh->SetArea(size.width * 0.5, size.height * 0.5);
		pExh->SetDistanceRange(minDist, maxDist);
		float dist = 18.0 - size.height * (double)0.015f;
		if (dist < minDist)
			dist = minDist;
		else if (dist > maxDist)
			dist = maxDist;
		pExh->SetDistance(dist, true, false);
	}
}

// 005b7ff0
bool IsNotSlot10_11_983(int slot)
{
	switch (slot)
	{
	case 10:
	case 11:
		return false;
	default:
		return true;
	}
}

// 005b8000
bool IsExtraSlot983(int slot)
{
	switch (slot)
	{
	case 9:
	case 10:
		return true;
	default:
		return false;
	}
}

// 005b8010
bool IsSeventhSlot983(int slot)
{
	return slot == 6;
}

// 005b8020
int SlotToIndex983(int slot)
{
	switch (slot)
	{
	case 1: return 0;
	case 2: return 1;
	case 3: return 2;
	case 4: return 4;
	case 5: return 5;
	case 6: return 6;
	case 7: return 8;
	case 8: return 9;
	case 9: return 3;
	case 10: return 7;
	}
	return -1;
}

// 005b80a0
int IndexToSlot983(int index)
{
	switch (index)
	{
	case 0: return 1;
	case 1: return 2;
	case 2: return 3;
	case 3: return 9;
	case 4: return 4;
	case 5: return 5;
	case 6: return 6;
	case 7: return 10;
	case 8: return 7;
	case 9: return 8;
	}
	return -1;
}

// 005b8120
int SlotCardType983(int slot)
{
	int type = 6;
	switch (slot)
	{
	case 1:
	case 2:
	case 3:
	case 9:
		type = 0;
		break;
	case 4:
	case 5:
	case 6:
	case 10:
		type = 1;
		break;
	case 7:
	case 8:
		type = 5;
		break;
	}
	return type;
}

// 005b8160
int IndexCardType983(int index)
{
	int type = 6;
	switch (index)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		type = 0;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		type = 1;
		break;
	case 8:
	case 9:
		type = 5;
		break;
	}
	return type;
}

// 005b81a0
RFCharMasteryCardNoticeDlg* RFCharMasteryCardNoticeDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005b81b0
RFCharMasteryCardSelectDlg* RFCharMasteryCardSelectDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005b81c0
RFCharMasteryCardSelectDlg* RFCharMasteryCardSelectDlg::GetInstance2_983()
{
	return ms_pInstance;
}

// 005b81d0
void RFCharMasteryCardSelectDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005b81e0
void RFCharMasteryCardSelectDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005b81f0
bool RFCharMasteryCardSelectDlg::IsSent983()
{
	return m_sent983;
}

// 005b8200
void RFCharMasteryCardSelectDlg::SetSent983(bool sent)
{
	m_sent983 = sent;
}

// 005b8210
RFCharacterMasteryCardDetachDlg* RFCharacterMasteryCardDetachDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005b8220
RFCharacterMasteryCardDetachDlg* RFCharacterMasteryCardDetachDlg::GetInstance2_983()
{
	return ms_pInstance;
}

// 005b8230
void RFCharacterMasteryCardDetachDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005b8240
void RFCharacterMasteryCardDetachDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005b8250
RFWnd* RFCardIcon::GetParent983()
{
	return s_pParent983;
}

// 005b8260
RFCharMasteryCardDlg* RFCharMasteryCardDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005b8270
RFCharMasteryCardDlg* RFCharMasteryCardDlg::GetInstance2_983()
{
	return ms_pInstance;
}

// 005b8280
void RFCharMasteryCardDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005b8290
void RFCharMasteryCardDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005b82a0
RFUseItemDlg* RFUseItemDlg::GetInstance983()
{
	return ms_pInstance;
}

// 005b82b0
RFUseItemDlg* RFUseItemDlg::GetInstance2_983()
{
	return ms_pInstance;
}

// 005b82c0
void RFCharMasteryCardNoticeDlg::OnOk983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(1);
}

// 005b8300: o construtor do AddCardColumn983 (embutido no Set983; a cópia fora de linha fica sem chamador)
// 005b8320: ESC fecha
void RFCharMasteryCardSelectDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
	{
		Close(2);
		e.handled = true;
	}
}

// 005b8340
void RFCharMasteryCardSelectDlg::OnCancel983(EventArgs& e)
{
	g_audio->PlaySfx("ui_button_cancel_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005b8380: ESC fecha
void RFCharacterMasteryCardDetachDlg::OnKey983(EventArgs& e)
{
	if (e.m_0c == VK_ESCAPE)
	{
		g_audio->PlaySfx("ui_dialog_close", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		Close(2);
		e.handled = true;
	}
}

// 005b83e0
void RFCharacterMasteryCardDetachDlg::OnClose983(EventArgs& e)
{
	g_audio->PlaySfx("ui_dialog_close", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	Close(2);
}

// 005b8420
void RFCardIcon::SetCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	m_pCard983 = pCard;
	m_slot983 = slot;
}

// 005b8440
void RFCardIcon::SetState983(bool special, bool open)
{
	m_bSpecial983 = special;
	m_bOpen983 = special ? open : true;
}

// 005b87a0
bool RFCharMasteryCardNoticeDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005b87b0
bool RFCharMasteryCardSelectDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005b87c0
bool RFCharacterMasteryCardDetachDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005b87d0
bool RFCharMasteryCardDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005b87e0
bool RFUseItemDlg::IsCreated983()
{
	return ms_pInstance != NULL;
}

// 005b87f0
RFCharMasteryCardNoticeDlg::RFCharMasteryCardNoticeDlg()
{
	m_pCardImg983 = NULL;
	m_pMessage983 = NULL;
}

// 005b8880
RFCharMasteryCardNoticeDlg::~RFCharMasteryCardNoticeDlg()
{
}

// 005b88f0
RFCharMasteryCardSelectColumn::RFCharMasteryCardSelectColumn()
	: m_pCard983(NULL), m_188_983(NULL), m_pCardImg983(NULL), m_pName983(NULL)
{
}

// 005b8920
RFCharMasteryCardSelectColumn::~RFCharMasteryCardSelectColumn()
{
}

// 005b8930
RFCharMasteryCardSelectDlg::RFCharMasteryCardSelectDlg()
{
	ms_pInstance = this;
	m_pList983 = NULL;
	m_pTitle983 = NULL;
	m_pCardImg983 = NULL;
	m_pUseItemImg983 = NULL;
	m_pUseItemText983 = NULL;
	m_pNotice983 = NULL;
	m_pWarning983 = NULL;
	m_pOk983 = NULL;
	m_pCard983 = NULL;
	m_slot983 = -1;
	m_sent983 = false;
}

// 005b8a00
RFCharMasteryCardSelectDlg::~RFCharMasteryCardSelectDlg()
{
	ms_pInstance = NULL;
}

// 005b8a70
RFCharacterMasteryCardDetachDlg::RFCharacterMasteryCardDetachDlg()
{
	ms_pInstance = this;
	m_pCardImg983 = NULL;
	m_slot983 = -1;
}

// 005b8b10
RFCharacterMasteryCardDetachDlg::~RFCharacterMasteryCardDetachDlg()
{
	ms_pInstance = NULL;
}

// 005b8b80
RFCardIcon::RFCardIcon()
	: m_pCard983(NULL)
{
	m_slot983 = -1;
	memset(m_pImg983, 0, sizeof(m_pImg983));
	m_bSpecial983 = false;
	m_bOpen983 = true;
}

// 005b8c00
RFCharMasteryCardDlg::RFCharMasteryCardDlg()
{
	ms_pInstance = this;
	memset(m_pSlot983, 0, sizeof(m_pSlot983));
	m_pTip983 = NULL;
}

// 005b8c60
RFCharMasteryCardDlg::~RFCharMasteryCardDlg()
{
	ms_pInstance = NULL;
}

// 005b8c80
void RFCharMasteryCardDlg::CloseDialogs983()
{
	if (RFUseItemDlg::ms_pInstance)
		RFUseItemDlg::ms_pInstance->Close(2);
	if (RFCharacterMasteryCardDetachDlg::ms_pInstance)
		RFCharacterMasteryCardDetachDlg::ms_pInstance->Close(2);
}

// 005b9880: encaixe 9/10 sem o item 0x1a00018f = ok desligado
void RFCharMasteryCardSelectDlg::UpdateOkButton983()
{
	if (m_pOk983)
	{
		if (IsExtraSlot983(m_slot983))
		{
			sItemInfo* pItem = Doc()->FindMyItemByTypeId983(0x1a00018f);
			if (!pItem || pItem->Common[0] <= 0)
				m_pOk983->EnableWindow(false);
		}
	}
}

// 005b9cd0: RFFormBase<RFWndEx>::ShowWindow983 (molde, rfbase.h)

// 005b9db0
void RFMasteryToolTipWnd::SetText983(const char* text)
{
	if (m_pText)
		m_pText->SetText(MultiByteToWide983(text));
}

// 005b9e90
void RFCharMasteryCardNoticeDlg::SetCard983(unsigned long cardId, bool attach)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(cardId);
	if (pCard && m_pCardImg983)
		m_pCardImg983->SetPicture(MultiByteToWide983(MakeStr("%s.png", pCard->Image)));
	if (m_pMessage983)
	{
		std::string text("");
		if (attach)
			text = MakeStr(GetGameString(0x18847), pCard->c.Name);
		else
			text = MakeStr(GetGameString(0x18848), pCard->c.Name);
		m_pMessage983->SetText(MultiByteToWide983(text.c_str()));
	}
}

// 005ba0f0
void RFCharMasteryCardSelectColumn::SetCard983(sCards* pCard)
{
	IFF_STRUCT::sCard* pIff = ItemManager()->FindCard(pCard->tid);
	if (pIff)
	{
		m_pCard983 = pCard;
		if (m_pCardImg983)
			m_pCardImg983->SetPicture(MultiByteToWide983(MakeStr("%s.png", pIff->Image)));
		if (m_pName983)
			m_pName983->SetLabel(MultiByteToWide983(MakeStr("x%d %s", pCard->cardCount, pIff->c.Name)));
	}
}

// 005ba2c0: o item 0x1a00018f (imagem, "nome x%d") e o aviso
void RFCharMasteryCardSelectDlg::UpdateUseItem983()
{
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(0x1a00018f);
	if (pItem)
	{
		if (m_pUseItemImg983)
			m_pUseItemImg983->SetPicture(MultiByteToWide983(MakeStr("%s.tga", pItem->c.Icon)));
		sItemInfo* pInfo = Doc()->FindMyItemByTypeId983(pItem->c.TypeId);
		if (m_pUseItemText983)
		{
			int count = 0;
			if (pInfo)
				count = pInfo->Common[0];
			m_pUseItemText983->SetText(MultiByteToWide983(MakeStr("%s x%d", pItem->c.Name, count)));
		}
		if (m_pWarning983)
		{
			std::string text("");
			if (IsExtraSlot983(m_slot983))
				text = GetGameString(0x18849);
			else
				text = GetGameString(0x1884a);
			m_pWarning983->SetText(MultiByteToWide983(text));
			m_pWarning983->ShowWindow(true);
		}
	}
}

// 005ba570
void RFCharacterMasteryCardDetachDlg::SetCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	if (pCard)
	{
		if (m_pCardImg983)
			m_pCardImg983->SetPicture(MultiByteToWide983(MakeStr("%s.png", pCard->Image)));
		m_slot983 = slot;
	}
}

// 005ba670
void RFCardIcon::Update983()
{
	if (m_pImg983[IMG_CARD983])
	{
		std::string name(m_pCard983 ? MakeStr("%s.png", m_pCard983->Image) : "");
		m_pImg983[IMG_CARD983]->SetPicture(MultiByteToWide983(name.c_str()));
	}
	if (m_pImg983[IMG_CLOSED983])
	{
		bool show = !m_bOpen983 && m_pCard983;
		m_pImg983[IMG_CLOSED983]->ShowWindow(show);
	}
	if (m_pImg983[IMG_BLANK983])
	{
		bool show = m_bOpen983 && !m_pCard983;
		m_pImg983[IMG_BLANK983]->ShowWindow(show);
	}
}

// 005ba830: dica junto do mouse
void RFCharMasteryCardDlg::ShowTip983(bool show, const char* text)
{
	if (m_pTip983)
	{
		if (show)
		{
			m_pTip983->SetText983(text);
			WVector pos = *RFSceneManager::Instance()->GetPos983();
			RFSize size = m_pTip983->GetSize();
			m_pTip983->SetPos983(pos.x - size.width * 0.5, pos.y - size.height);
		}
		m_pTip983->ShowTip983(show);
	}
}

// 005ba8e0
void RFCharMasteryCardSelectDlg::UpdateNotice983(int type)
{
	if (m_pNotice983)
	{
		std::string name("");
		name += IsExtraSlot983(m_slot983) ? GetGameString(0x1884b) : "";
		switch (type)
		{
		case 0:
			name += "'";
			// 캐릭터 (personagem)
			name += K2L_Compatibility("\xc4\xb3\xb8\xaf\xc5\xcd");
			name += "'";
			break;
		case 1:
			name += "'";
			// 캐디 (caddie)
			name += K2L_Compatibility("\xc4\xb3\xb5\xf0");
			name += "'";
			break;
		case 5:
			name += "'NPC'";
			break;
		}
		m_pNotice983->SetText(MultiByteToWide983(MakeStr(GetGameString(0x1884c), name.c_str())));
	}
}

// 005baad0: dica do encaixe fechado
void RFCardIcon::OnEnter983(EventArgs& e)
{
	if (m_bSpecial983 && !m_bOpen983 && RFCharMasteryCardDlg::ms_pInstance)
	{
		std::string text(GetGameString(0x18850));
		RFCharMasteryCardDlg::ms_pInstance->ShowTip983(true, text.c_str());
	}
}

// 005baba0
void RFCardIcon::OnLeave983(EventArgs& e)
{
	if (m_bSpecial983 && !m_bOpen983 && RFCharMasteryCardDlg::ms_pInstance)
		RFCharMasteryCardDlg::ms_pInstance->ShowTip983(false, NULL);
}

// 005babd0
RFCharMasteryCardNoticeDlg* RFCharMasteryCardNoticeDlg::Create983(RFWnd* parent)
{
	RFCharMasteryCardNoticeDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardNoticeDlg>(L"mastery_card_notice_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardNoticeDlg>(L"mastery_card_notice_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryCardNoticeDlg>(L"mastery_card_notice_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005bad10
void RFCharMasteryCardNoticeDlg::OnInitialUpdate()
{
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFCharMasteryCardNoticeDlg::OnOk983, this));
	m_pCardImg983 = GetChild983<RFPicture>(L"card_img");
	m_pMessage983 = GetChild983<RFTextArea>(L"message");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005baf20
RFCharMasteryCardSelectColumn* RFCharMasteryCardSelectColumn::Create983(RFWnd* parent)
{
	RFCharMasteryCardSelectColumn* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardSelectColumn>(L"kam_mastery_card_select_column.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardSelectColumn>(L"kam_mastery_card_select_column.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryCardSelectColumn>(L"kam_mastery_card_select_column.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005bb060
RFCharMasteryCardSelectDlg* RFCharMasteryCardSelectDlg::Create983(RFWnd* parent)
{
	RFCharMasteryCardSelectDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardSelectDlg>(L"kam_mastery_card_select_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardSelectDlg>(L"kam_mastery_card_select_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryCardSelectDlg>(L"kam_mastery_card_select_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// Set983 005bc080: põe na lista uma coluna por carta do tipo do encaixe (std::for_each; operator() 005bb1a0, construtor 005b8300)
struct AddCardColumn983
{
	AddCardColumn983(int* pType, RFCharMasteryCardSelectDlg* pDlg) : m_pType(pType), m_pDlg(pDlg) { }

	void operator()(sCards& card)
	{
		if (card.tid && card.cardCount > 0 && *m_pType == (card.tid >> 22 & 0xf))
		{
			RFCharMasteryCardSelectColumn* column = RFCharMasteryCardSelectColumn::Create983(m_pDlg->m_pList983);
			column->SetCard983(&card);
			column->ShowWindow983(true);
			m_pDlg->m_pList983->InsertWindowItem(column);
		}
	}

	int* m_pType;
	RFCharMasteryCardSelectDlg* m_pDlg;
};

// 005bb200: carta escolhida
void RFCharMasteryCardSelectDlg::SetCard983(sCards* pCard)
{
	m_pCard983 = pCard;
	if (m_pCardImg983 && pCard)
	{
		IFF_STRUCT::sCard* pIff = ItemManager()->FindCard(pCard->tid);
		m_pCardImg983->SetPicture(MultiByteToWide983(MakeStr("%s.png", pIff->Image)));
		PlayAnimation(L"_open_card_img_");
	}
}

// 005bb360
void RFCharMasteryCardSelectDlg::SetTitle983(int type)
{
	if (m_pTitle983)
	{
		switch (type)
		{
		case 0:
			m_pTitle983->SetPicture(L"title_charater_card.tga");
			break;
		case 1:
			m_pTitle983->SetPicture(L"title_caddie_card.tga");
			break;
		case 5:
			m_pTitle983->SetPicture(L"title_npc_card.tga");
			break;
		}
	}
}

// 005bb460
RFCharacterMasteryCardDetachDlg* RFCharacterMasteryCardDetachDlg::Create983(RFWnd* parent)
{
	RFCharacterMasteryCardDetachDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharacterMasteryCardDetachDlg>(L"kam_card_deselect_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharacterMasteryCardDetachDlg>(L"kam_card_deselect_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharacterMasteryCardDetachDlg>(L"kam_card_deselect_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005bb5a0
RFCardIcon* RFCardIcon::Create983(RFWnd* parent)
{
	RFCardIcon* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCardIcon>(L"kam_mastery_card_thumb.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCardIcon>(L"kam_mastery_card_thumb.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCardIcon>(L"kam_mastery_card_thumb.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005bb6e0
RFCharMasteryCardDlg* RFCharMasteryCardDlg::Create983(RFWnd* parent)
{
	RFCharMasteryCardDlg* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardDlg>(L"kam_mastery_card.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFCharMasteryCardDlg>(L"kam_mastery_card.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFCharMasteryCardDlg>(L"kam_mastery_card.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005bb820: resposta do servidor ao pôr/tirar carta
void RFCharMasteryCardDlg::OnResult983(unsigned long cardId, bool attach)
{
	if (RFCharacterMasteryCardDetachDlg::ms_pInstance)
		RFCharacterMasteryCardDetachDlg::ms_pInstance->Close(2);
	if (RFCharMasteryCardSelectDlg::ms_pInstance)
		RFCharMasteryCardSelectDlg::ms_pInstance->Close(2);
	if (!RFCharMasteryCardNoticeDlg::ms_pInstance)
	{
		RFCharMasteryCardNoticeDlg* dlg = RFCharMasteryCardNoticeDlg::Create983(this);
		if (dlg)
		{
			dlg->SetCard983(cardId, attach);
			dlg->DoModal();
		}
	}
}

// 005bb880: um RFCardIcon no lugar de cada marcador slot_wnd1…10 (que é apagado)
void RFCharMasteryCardDlg::InitSlots983()
{
	for (int i = 0; i < 10; ++i)
	{
		RFWnd* wnd = GetChild(MultiByteToWide983(MakeStr("slot_wnd%d", i + 1)));
		if (wnd)
		{
			m_pSlot983[i] = RFCardIcon::Create983(this);
			m_pSlot983[i]->SetPos(wnd->GetPos().x, wnd->GetPos().y);
			m_pSlot983[i]->ShowWindow983(true);
			DeleteChild(wnd);
		}
	}
}

// 005bbab0: manda 0x18a (0x18b nos encaixes 9 e 10)
void RFCharMasteryCardSelectDlg::OnOk983(EventArgs& e)
{
	bool sent = m_sent983;
	if (!sent)
	{
		if (m_pCard983)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			m_sent983 = true;
			WSendPacket packet;
			if (IsExtraSlot983(m_slot983))
				packet.InitPacket(0x18b);
			else
				packet.InitPacket(0x18a);
			packet.Encode4(Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar].tid);
			packet.Encode4(Doc()->m_myInfo.userEquip.guidChar);
			packet.Encode4(m_pCard983->tid);
			packet.Encode4(m_pCard983->uid);
			packet.Encode4(SlotToIndex983(m_slot983) + 1);
			packet.Send(TO_GAME);
		}
		g_audio->PlaySfx("ui_button_ok_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	}
}

// 005bba60
void RFCharMasteryCardSelectColumn::OnClick983(EventArgs& e)
{
	if (RFCharMasteryCardSelectDlg::ms_pInstance)
		RFCharMasteryCardSelectDlg::ms_pInstance->SetCard983(m_pCard983);
	g_audio->PlaySfx("ui_icon_click", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005bbc50: ok da RFUseItemDlg → manda 0x18c (tira a carta gastando o item 0x1a0000c2)
void RFCharacterMasteryCardDetachDlg::OnUseItem983(EventArgs& e)
{
	std::map<unsigned int, IFF_STRUCT::sCard>::iterator it = ItemManager()->m_CardMap.begin();
	sItemInfo* pItem = Doc()->FindMyItemByTypeId983(0x1a0000c2);
	if (it != ItemManager()->m_CardMap.end() && pItem)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0x18c);
		packet.Encode4(Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar].tid);
		packet.Encode4(Doc()->m_myInfo.userEquip.guidChar);
		packet.Encode4(pItem->typeId);
		packet.Encode4(pItem->id);
		packet.Encode4(SlotToIndex983(m_slot983) + 1);
		packet.Send(TO_GAME);
	}
}

// 005bbdb0: −1 = nada; redesenha os 10 encaixes de carta
void RFCharMasteryCardDlg::SetChar983(unsigned long charId)
{
	if (charId != (unsigned long)-1)
	{
		sCharacterInfo* pInfo = &Doc()->m_charMap[charId];
		if (pInfo)
		{
			bool hasSeventh = Doc()->HasSeventhCardSlot983(pInfo);
			for (int i = 0; i < 10; ++i)
			{
				if (m_pSlot983[i])
				{
					IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(pInfo->card[0][SlotToIndex983(i + 1)]);
					m_pSlot983[i]->SetCard983(pCard, i + 1);
					m_pSlot983[i]->SetState983(IsSeventhSlot983(i + 1), hasSeventh);
					m_pSlot983[i]->Update983();
				}
			}
		}
	}
}

// 005bbe70
void RFCharMasteryCardDlg::OnInitialUpdate()
{
	InitSlots983();
	m_pTip983 = RFMasteryToolTipWnd::Create983(GetSuper(false));
}

// 005bbec0
void RFCharMasteryCardSelectColumn::OnInitialUpdate()
{
	RegisterTo(L"column_body", RFButton::EventClicked, Subscriber(&RFCharMasteryCardSelectColumn::OnClick983, this));
	m_pCardImg983 = GetChild983<RFPicture>(L"card_img");
	m_pName983 = GetChild983<RFLabel>(L"card_name");
}

// 005bc080: lista as cartas do tipo do encaixe
void RFCharMasteryCardSelectDlg::Set983(int slot)
{
	m_slot983 = slot;
	int type = SlotCardType983(slot);
	std::for_each(Doc()->m_cardList.begin(), Doc()->m_cardList.end(), AddCardColumn983(&type, this));
	SetTitle983(type);
	UpdateUseItem983();
	UpdateNotice983(type);
	UpdateOkButton983();
}

// 005bc0f0
void RFCharMasteryCardSelectDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFCharMasteryCardSelectDlg::OnKey983, this));
	RegisterTo(L"ok_btn", RFButton::EventClicked, Subscriber(&RFCharMasteryCardSelectDlg::OnOk983, this));
	RegisterTo(L"cancel_btn", RFButton::EventClicked, Subscriber(&RFCharMasteryCardSelectDlg::OnCancel983, this));
	m_pList983 = GetChild983<RFListSC>(L"card_list");
	m_pTitle983 = GetChild983<RFPicture>(L"title_card_type_img");
	m_pCardImg983 = GetChild983<RFPicture>(L"card_img");
	m_pUseItemImg983 = GetChild983<RFPicture>(L"use_item_img");
	m_pUseItemText983 = GetChild983<RFTextArea>(L"use_item_text");
	m_pNotice983 = GetChild983<RFTextArea>(L"notice_text");
	m_pWarning983 = GetChild983<RFTextArea>(L"warnning_text");
	m_pOk983 = GetChild983<RFButton>(L"ok_btn");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005bc6b0: pede o item 0x1a0000c2 (RFUseItemDlg) ou avisa que falta (RFNoticeDlg)
void RFCharacterMasteryCardDetachDlg::OnDetach983(EventArgs& e)
{
	if (m_slot983 != -1 && m_pCardImg983)
	{
		IFF_STRUCT::sItem* pIff = ItemManager()->FindItem(0x1a0000c2);
		if (pIff)
		{
			sItemInfo* pItem = Doc()->FindMyItemByTypeId983(pIff->c.TypeId);
			if (pItem && pItem->Common[0] >= 1)
			{
				RFUseItemDlg* dlg = RFUseItemDlg::Create983(this);
				std::string text(MakeStr(GetGameString(0x1884d), pIff->c.Name));
				dlg->SetItem983(pItem);
				dlg->RegisterTo(L"ok_btn", RFButton::EventClicked,
					Subscriber(&RFCharacterMasteryCardDetachDlg::OnUseItem983, this));
				dlg->SetMessage983(text.c_str());
				dlg->DoModal();
			}
			else
			{
				RFNoticeDlg* dlg = RFNoticeDlg::Create983(this);
				std::string text(MakeStr(GetGameString(0x1884e), pIff->c.Name));
				dlg->SetMessage983(text.c_str());
				dlg->DoModal();
			}
		}
	}
}

// 005bc880: vazio → escolher carta; com carta → tirar
void RFCardIcon::OnClick983(EventArgs& e)
{
	if (m_pCard983)
	{
		if (!RFCharacterMasteryCardDetachDlg::ms_pInstance)
		{
			RFCharacterMasteryCardDetachDlg* dlg = RFCharacterMasteryCardDetachDlg::Create983(s_pParent983->GetSuper(false));
			if (dlg)
			{
				dlg->SetCard983(m_pCard983, m_slot983);
				dlg->DoModal();
			}
		}
	}
	else if (m_bSpecial983 && !m_bOpen983)
	{
		RFNoticeDlg* dlg = RFNoticeDlg::Create983(this);
		if (dlg)
		{
			std::string text(GetGameString(0x1884f));
			dlg->SetMessage983(text.c_str());
			dlg->DoModal();
		}
	}
	else if (!RFCharMasteryCardSelectDlg::ms_pInstance)
	{
		RFCharMasteryCardSelectDlg* dlg = RFCharMasteryCardSelectDlg::Create983(this);
		if (dlg)
		{
			dlg->Set983(m_slot983);
			dlg->DoModal();
		}
	}
}

// 005bc9a0
void RFCharacterMasteryCardDetachDlg::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKeyDown, Subscriber(&RFCharacterMasteryCardDetachDlg::OnKey983, this));
	RegisterTo(L"detach_btn", RFButton::EventClicked, Subscriber(&RFCharacterMasteryCardDetachDlg::OnDetach983, this));
	RegisterTo(L"close_btn", RFButton::EventClicked, Subscriber(&RFCharacterMasteryCardDetachDlg::OnClose983, this));
	m_pCardImg983 = GetChild983<RFPicture>(L"card_origin_img");
	g_audio->PlaySfx("ui_dialog_open", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// 005bccc0
void RFCardIcon::OnInitialUpdate()
{
	RegisterTo(L"frame_btn", RFButton::EventClicked, Subscriber(&RFCardIcon::OnClick983, this));
	RegisterTo(L"frame_btn", RFWnd::EventMouseEnter, Subscriber(&RFCardIcon::OnEnter983, this));
	RegisterTo(L"frame_btn", RFWnd::EventMouseLeave, Subscriber(&RFCardIcon::OnLeave983, this));
	m_pImg983[IMG_CARD983] = GetChild983<RFPicture>(L"card_img");
	m_pImg983[IMG_CLOSED983] = GetChild983<RFPicture>(L"card_img_not");
	m_pImg983[IMG_BLANK983] = GetChild983<RFPicture>(L"card_blank");
}
