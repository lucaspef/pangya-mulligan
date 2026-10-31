// JP 983: unidade 004ea7a0–004ec7b0 (FrClubSetLevelUpWnd, aba "subir nível" da oficina de tacos). Classe do 839/983, sem par
// no 645; ver include/jp/clubsetlevelupwnd.h. Sessão B79 (F3.7.9, 4º lote). O compilador gera: 004eaa00 (ajuste do Helper),
// 004eb230 (`delete`), 004ea8e0 (std::string::clear, dobrada). A LoadLayout983 (004ec7b0) é a unidade seguinte (não escrita).
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "titlefontex.h"
#include "packet.h"
#include "shareddoc.h"
#include "../../../include/packets/clubsetworkpackets.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopmaindlg.h"
#include "clubsetworkshopprogresswnd.h"
#include "clubsetworkshopfx.h"
#include "clubsetworkshopselector.h"
#include "clubsetinfownd.h"
#include "clubsetlevelupwnd.h"

extern Fresh* g_pFresh; // 00f02340

// nome do bônus das cartas postas (unidade da FrClubSetWorkshopSelector; nome meu)
const char* __fastcall GetUpgradeStatText983(int count); // 004a5240

FrClubSetLevelUpWnd* FrClubSetLevelUpWnd::s_pInstance = NULL;

// 004ea7a0
void FrClubSetLevelUpWnd::Proc983(float deltaTime)
{
	if (m_bShow && m_pInfoWnd)
		m_pInfoWnd->Proc983(deltaTime);
}

// 004ea7d0
void FrClubSetLevelUpWnd::UpdateUseItem983()
{
	if (m_cardCount == 0)
	{
		m_useTypeId = GetTypeId983(0);
		m_useCount = 0 < m_itemCount;
		return;
	}
	m_useTypeId = GetTypeId983(1);
	m_useCount = m_cardCount;
}

// 004ea820
void FrClubSetLevelUpWnd::ResetInfo983(unsigned long guid, int mode)
{
	if (m_pInfoWnd)
	{
		m_pInfoWnd->SetMode983(0);
		m_pInfoWnd->Reset983(1);
	}
}

// 004ea850: resposta ao 0x164 (o trabalho começou)
void FrClubSetLevelUpWnd::OnLevelUpStart983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		unsigned long value = pPacket->Decode4();
		if (m_pInfoWnd)
		{
			m_pInfoWnd->SetMode983(1);
			m_pInfoWnd->SetSelStat983(value);
		}
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
		{
			FrClubSetWorkshopProgressWnd::s_pInstance->Start983(true);
			FrClubSetWorkshopProgressWnd::s_pInstance->m_bfa983 = true;
		}
	}
	else
	{
		m_bSending = false;
		OnEndWork983();
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->Finish983();
	}
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004ea900
FrClubSetLevelUpWnd::FrClubSetLevelUpWnd(ClubSetWorkShopForm* pForm)
{
	s_pInstance = this;
	m_itemIconX = 0;
	m_itemIconY = 0;
	m_pForm = pForm;
	m_cardIconX = 0;
	m_pInfoWnd = NULL;
	m_cardIconY = 0;
	m_npcImgName = NULL;
	m_itemBgName = NULL;
	m_itemIconOverlay = 0;
	m_cardIconOverlay = 0;
	m_bSending = false;
	m_confirmState = -1;
	m_pNpcImg = NULL;
	m_pLogo = NULL;
	m_clubGuid = 0;
	m_clubRank = 0;
	m_useTypeId = 0;
	m_useCount = 0;
	m_unusedc4983 = 0;
	m_itemCount = 0;
	m_cardCount = 0;
	m_cardMax = 0;
}

// 004eaa10
FrClubSetLevelUpWnd::~FrClubSetLevelUpWnd()
{
	s_pInstance = NULL;
	if (m_pInfoWnd)
	{
		delete m_pInfoWnd;
		m_pInfoWnd = NULL;
	}
}

// 004eaad0
void FrClubSetLevelUpWnd::UpdateItemIcon983()
{
	if (m_cardCount)
	{
		ShowCtrl983("area_item_icon_levelup", false);
		SetOverlayUse983(m_itemIconOverlay, false);
	}
	else
	{
		ShowCtrl983("area_item_icon_levelup", m_itemCount != 0);
		SetOverlayUse983(m_itemIconOverlay, m_itemCount != 0);
	}
}

// 004eac10
void FrClubSetLevelUpWnd::OnBeginWork983()
{
	ShowCtrl983("btn_confirm_levelup", false);
	EnableCtrl983("btn_confirm_levelup", false);
	ShowCtrl983("area_item_icon_levelup", false);
	ShowCtrl983("area_card_icon_levelup", false);
	ShowCtrl983("area_cardinfo_tooltip_levelup", false);
	ClubSetWorkshopChildWndBase::OnBeginWork983();
}

// 004eae40
void FrClubSetLevelUpWnd::OnDrawNpcImg983()
{
	if (IsShow983() && m_pNpcImg)
	{
		const WRect& rc = m_pNpcImg->GetRect();
		DrawOverlay983(m_npcImgName, (int)rc.x, (int)rc.y, 0);
	}
}

// 004eaeb0
void FrClubSetLevelUpWnd::OnDrawLogo983()
{
	if (IsShow983() && m_pLogo)
	{
		const WRect& rc = m_pLogo->GetRect();
		DrawOverlay983(m_logoName, (int)rc.x, (int)rc.y, 0);
	}
}

// 004eaf20: "Level %d -> %d / Deseja fazer a reforma? …" (texto coreano pelo K2L)
void FrClubSetLevelUpWnd::OnConfirmBtnUp983()
{
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(0);
	if (!pSlot)
		return;
	EnableCtrl983("btn_confirm_levelup", false);
	ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
	if (pDlg)
	{
		int level = pSlot->level + 1;
		if (pSlot->recoveryLeft == 0)
		{
			pDlg->SetSize983(320.0f, 290.0f, true);
			pDlg->SetMessage(MakeStr(K2L_Compatibility("Level %d -> %d\012\012\260\263\301\266\270\246 \301\370\307\340\307\317"
				"\275\303\260\332\275\300\264\317\261\356?\012\260\263\301\266 \260\341\260\372 \310\256\300\316 \310\304, "
				"\134c0xffff0000\134c\303\353\274\322\260\241 \272\322\260\241\264\311\134c0xff000000\134c\307\325\264\317\264\331."
				"\012(\305\254\267\264\274\274\306\256 \310\270\272\271\300\314 \307\312\277\344)"), level, level + 1), false);
			pDlg->AddBitmap983(pSlot->pBmp);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnLevelUpConfirmResult983, 1);
		}
		else
		{
			pDlg->SetSize983(320.0f, 280.0f, true);
			pDlg->SetMessage(MakeStr(K2L_Compatibility("Level %d -> %d\012\012\260\263\301\266\270\246 \301\370\307\340\307\317"
				"\275\303\260\332\275\300\264\317\261\356?\012\260\263\301\266 \260\341\260\372 \310\256\300\316 \310\304, "
				"\303\353\274\322\260\241 \260\241\264\311\307\325\264\317\264\331."), level, level + 1), false);
			pDlg->AddBitmap983(pSlot->pBmp);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnLevelUpConfirmResult983, 3);
		}
		return;
	}
	EnableCtrl983("btn_confirm_levelup", true);
}

// 004eb130: resposta final (o taco subiu de nível)
void FrClubSetLevelUpWnd::OnLevelUpResult983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		pPacket->Decode4();
		unsigned long guid = pPacket->Decode4();
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->SetResult983(guid);
		std::map<unsigned int, sItemInfo>::iterator it = Doc()->m_clubSetMap.find(guid);
		if (it != Doc()->m_clubSetMap.end())
			Doc()->m_itemManager.FindClubSet((*it).second.typeId);
		return;
	}
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->Start983(false);
}

// 004eb1c0
FrClubSetLevelUpWnd* FrClubSetWorkshopMainDlg::CreateLevelUp983()
{
	return new FrClubSetLevelUpWnd(this);
}

// 004eb260
void FrClubSetLevelUpWnd::OnClubselectBtnUp983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pForm, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long guid = m_clubGuid;
		pDlg->m_marked.push_back(guid);
		pDlg->m_kind = 1;
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnLevelUpClubSelected983, 1);
	}
}

// 004eb2f0
bool FrClubSetLevelUpWnd::Init983()
{
	CreateFonts983(1);
	CreateCardNumFont983();
	m_pNpcImg = GetCtrl983<FrArea>("area_npc_img1");
	m_pLogo = GetCtrl983<FrArea>("area_logo_levelup");
	AddCtrlByName983("area_npc_img1");
	AddCtrlByName983("area_logo_levelup");
	AddCtrlByName983("area_clubinfo_levelup");
	AddCtrlByName983("area_item_levelup");
	AddCtrlByName983("btn_confirm_levelup");
	AddCtrlByName983("static_clubname_levelup");
	AddCtrlByName983("static_power_levelup");
	AddCtrlByName983("static_control_levelup");
	AddCtrlByName983("static_accuracy_levelup");
	AddCtrlByName983("static_spin_levelup");
	AddCtrlByName983("static_curve_levelup");
	AddCtrlByName983("static_power_max_levelup");
	AddCtrlByName983("static_control_max_levelup");
	AddCtrlByName983("static_accuracy_max_levelup");
	AddCtrlByName983("static_spin_max_levelup");
	AddCtrlByName983("static_curve_max_levelup");
	AddCtrlByName983("static_clubtype_levelup");
	AddCtrlByName983("static_exppoint_levelup");
	AddCtrlByName983("static_slotcount_levelup");
	AddCtrlByName983("btn_clubselect_levelup");
	AddCtrlByName983("area_item_icon_levelup");
	AddCtrlByName983("area_item_num_levelup");
	AddCtrlByName983("static_item_name_levelup");
	AddCtrlByName983("area_card_icon_levelup");
	AddCtrlByName983("btn_card_add_levelup");
	AddCtrlByName983("btn_card_sub_levelup");
	AddCtrlByName983("static_upgrade_stat");
	AddCtrlByName983("area_card_num_levelup");
	AddCtrlByName983("area_cardinfo_tooltip_levelup");
	AddCtrlByName983("area_npc_speech_levelup");
	return true;
}

// 004eb830: até 5 cartas e no máximo as que o jogador tem
void FrClubSetLevelUpWnd::UpdateCards983()
{
	bool bSub = m_cardCount != 0;
	bool bAdd = m_cardCount > -1;
	if (bAdd)
	{
		if (m_cardCount >= m_cardMax)
		{
			m_cardCount = m_cardMax;
			bAdd = false;
		}
		else if (m_cardCount >= 5)
		{
			m_cardCount = 5;
			bAdd = false;
		}
	}
	ShowCtrl983("area_card_icon_levelup", m_cardCount != 0);
	SetOverlayUse983(m_cardIconOverlay, m_cardCount != 0);
	FrButton* pAdd = GetCtrl983<FrButton>("btn_card_add_levelup");
	if (pAdd)
		pAdd->Enable(bAdd);
	FrButton* pSub = GetCtrl983<FrButton>("btn_card_sub_levelup");
	if (pSub)
		pSub->Enable(bSub);
	FrStatic* pStat = GetCtrl983<FrStatic>("static_upgrade_stat");
	if (pStat)
		pStat->SetCaption(GetUpgradeStatText983(m_cardCount));
	FrArea* pTip = GetCtrl983<FrArea>("area_cardinfo_tooltip_levelup");
	if (pTip)
	{
		if (m_cardCount > 0)
			pTip->SetToolTipText(MakeStr(K2L_Compatibility("[%s] \303\337\260\241\310\256\267\374 \273\363\275\302"),
				GetUpgradeStatText983(m_cardCount)));
		else
			pTip->SetToolTipText("");
	}
}

// 004ebbd0
void FrClubSetLevelUpWnd::UpdateConfirm983()
{
	bool bEnable = true;
	if (m_clubGuid == 0)
		m_confirmState = 0;
	else if (ClubSetWorkshopFx983::GetSlotRank983(0) == 5 || ClubSetWorkshopFx983::GetSlotLevel983(0) == 4)
		m_confirmState = 2;
	else if (m_useCount == 0)
		m_confirmState = 1;
	else
		m_confirmState = -1;
	if (m_bSending || m_confirmState != -1)
		bEnable = false;
	FrButton* pBtn = GetCtrl983<FrButton>("btn_confirm_levelup");
	if (!pBtn)
		return;
	std::string tip;
	switch (m_confirmState)
	{
	case 0: // "Escolha o conjunto de tacos."
		tip = K2L_Compatibility("\305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\330 \301\326\274\274\277\344.");
		break;
	case 1: // "Faltam itens para a reforma."
		tip = K2L_Compatibility("\260\263\301\266\277\241 \307\312\277\344\307\321 \276\306\300\314\305\333\300\314 \272\316\301\267"
			"\307\325\264\317\264\331.");
		break;
	case 2: // "Não dá para reformar mais."
		tip = K2L_Compatibility("\264\365 \300\314\273\363 \260\263\301\266\307\322 \274\366 \276\370\275\300\264\317\264\331.");
		break;
	}
	pBtn->SetToolTipText(bEnable ? "" : tip.c_str());
	pBtn->Enable(bEnable);
}

// 004ebe00
void FrClubSetLevelUpWnd::OnEndWork983()
{
	ShowCtrl983("btn_confirm_levelup", true);
	UpdateConfirm983();
	ShowCtrl983("area_cardinfo_tooltip_levelup", true);
	ClubSetWorkshopChildWndBase::OnEndWork983();
}

// 004ebf20
void FrClubSetLevelUpWnd::OnDrawClubinfo983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_clubinfo_levelup");
	if (pArea && m_pInfoWnd)
		m_pInfoWnd->Draw983();
}

// 004ebff0
void FrClubSetLevelUpWnd::OnDrawItem983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_item_levelup");
	if (pArea)
		DrawOverlay983(m_itemBgName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
	DrawOverlayByIndex983(m_itemIconOverlay, (int)(m_pForm->GetRect().x + m_itemIconX), (int)(m_pForm->GetRect().y + m_itemIconY), 0);
	DrawOverlayByIndex983(m_cardIconOverlay, (int)(m_pForm->GetRect().x + m_cardIconX), (int)(m_pForm->GetRect().y + m_cardIconY), 0);
}

// 004ec160
void FrClubSetLevelUpWnd::OnDrawItemNum983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_item_num_levelup");
	if (pArea && (int)m_fonts.size() != 0 && m_fonts[0] != NULL)
		GetFont983(0)->Print(pArea->GetRect().w + pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", m_itemCount), TFA_RIGHT,
			0.0f, 1.0f);
}

// 004ec290
void FrClubSetLevelUpWnd::OnCardAddBtnUp983()
{
	m_cardCount++;
	UpdateUseItem983();
	UpdateItemIcon983();
	UpdateCards983();
	UpdateConfirm983();
}

// 004ec2c0
void FrClubSetLevelUpWnd::OnCardSubBtnUp983()
{
	m_cardCount--;
	UpdateUseItem983();
	UpdateItemIcon983();
	UpdateCards983();
	UpdateConfirm983();
}

// 004ec2f0
void FrClubSetLevelUpWnd::OnDrawCardNum983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_card_num_levelup");
	if (pArea && m_pCardNumFont)
	{
		m_pCardNumFont->Print(pArea->GetRect().x, pArea->GetRect().y, "x");
		m_pCardNumFont->Print(m_pCardNumFont->GetFontW() + pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", m_cardCount));
	}
}

// 004ec410
void FrClubSetLevelUpWnd::OnDrawNpcSpeech983()
{
	if (m_bShow)
	{
		FrArea* pArea = GetCtrl983<FrArea>("area_npc_speech_levelup");
		if (pArea)
			DrawOverlayByIndex983(m_npcSpeechOverlay, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
	}
}

// 004ec4f0
bool FrClubSetLevelUpWnd::OnConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		if (!m_bSending)
		{
			m_bSending = true;
			SendClubSetLevelUp983(m_clubGuid, m_useTypeId, m_useCount);
			if (FrClubSetWorkshopProgressWnd::s_pInstance)
				FrClubSetWorkshopProgressWnd::s_pInstance->OnSent983();
			OnBeginWork983();
			UpdateConfirm983();
			return true;
		}
	}
	else
		EnableCtrl983("btn_confirm_levelup", true);
	return true;
}

// 004ec600
void FrClubSetLevelUpWnd::SelectClub983(unsigned long guid)
{
	m_itemCount = 0;
	sItemInfo* pItem = FindMyItem983(0);
	if (pItem)
		m_itemCount = pItem->Common[0];
	m_cardCount = 0;
	m_cardMax = 0;
	sCards* pCard = FindCard983(1);
	if (pCard)
		m_cardMax = pCard->cardCount;
	m_bSending = false;
	ClubSetWorkshopFx983::SetSlotClub983(0, guid);
	UpdateUseItem983();
	ResetInfo983(guid, 1);
	UpdateItemIcon983();
	UpdateCards983();
	UpdateConfirm983();
}

// 004ec690
bool FrClubSetLevelUpWnd::OnClubSelected983(int result, FrForm* pForm)
{
	if (result == FrOK && pForm)
	{
		FrClubSetWorkshopSelector* pSel = (FrClubSetWorkshopSelector*)pForm;
		m_clubGuid = pSel->m_selGuid;
		m_clubRank = pSel->m_selTid;
		SelectClub983(m_clubGuid);
	}
	return true;
}

// 004ec6d0: o trabalho foi desfeito
void FrClubSetLevelUpWnd::OnLevelUpCancel983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		unsigned long guid = pPacket->Decode4();
		SelectClub983(guid);
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->Finish983();
		std::map<unsigned int, sItemInfo>::iterator it = Doc()->m_clubSetMap.find(guid);
		if (it != Doc()->m_clubSetMap.end())
			Doc()->m_itemManager.FindClubSet((*it).second.typeId);
		return;
	}
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->Start983(false);
}

// 004ec760
void FrClubSetLevelUpWnd::SetShow983(bool bShow)
{
	ClubSetWorkshopChildWndBase::SetShow983(bShow);
	if (bShow)
	{
		if (m_pInfoWnd)
		{
			m_pInfoWnd->SetShow983(bShow);
			m_pInfoWnd->Reset983(1);
		}
		SelectClub983(m_clubGuid);
	}
}
