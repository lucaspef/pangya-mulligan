// JP 983: unidade 004c0690–004c6280 (FrClubSetOptionalFunctionsWnd, aba "cuidados" da oficina de tacos: mover experiência entre
// dois tacos e recuperar as vezes de cancelar). Classe do 839/983, sem par no 645; ver include/jp/clubsetoptionalfunctionswnd.h.
// Sessão B82 (F3.7.9, 7º lote). O compilador gera: 004c1620 (ajuste do Helper), 004c1ef0 (`delete`), cópias fora de linha das
// embutidas do form/ChildWndBase (004c12a0, 004c13b0, 004c1470, 004c20f0, 004c22c0) e moldes do vector. A LoadLayout983
// (004c3ea0, 9.074 B) foi escrita na sessão B83 (F3.7.9, 8º lote; D 0,86 só por ordem de ramos/pilha, mesmas chamadas).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "bitmap.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "titlefontex.h"
#include "packet.h"
#include "shareddoc.h"
#include "inputmanager.h"
#include "lobbyjp.h"
#include "../../../include/packets/clubsetworkpackets.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopmaindlg.h"
#include "clubsetworkshopfx.h"
#include "clubsetworkshopselector.h"
#include "clubsetoptionalfunctionswnd.h"
#include "lua983.h"

extern Fresh* g_pFresh; // 00f02340
#define FORMGDI983() (FRMGR_GDI983(FRESHMGR983()))

FrClubSetOptionalFunctionsWnd* FrClubSetOptionalFunctionsWnd::s_pInstance = NULL;

// 004c0690
void FrClubSetOptionalFunctionsWnd::OnEndWork983()
{
	ClubSetWorkshopChildWndBase::OnEndWork983();
}

// 004c06a0
void FrClubSetOptionalFunctionsWnd::OnDrawExpmoveItemIcon983()
{
}

// 004c06b0
void FrClubSetOptionalFunctionsWnd::OnDrawRecoveryItemIcon983()
{
}

// 004c07c0: taco de origem, fundo, "escolha" piscando e a marca do rank máximo
void FrClubSetOptionalFunctionsWnd::OnDrawClubselectExpmove0_983()
{
	FrGraphicInterface* pGDI = FORMGDI983();
	if (pGDI && m_clubBmp[0])
		pGDI->DrawTexture(m_clubBmp[0], WRect(m_pForm->GetRect().x + m_clubPos[0][0], m_pForm->GetRect().y + m_clubPos[0][1],
			(float)m_clubBmp[0]->Width(), (float)m_clubBmp[0]->Height()), 0xffffffff, 0);
	DrawOverlayByIndex983(m_clubBgOverlay[0], (int)(m_pForm->GetRect().x + m_clubPos[0][0] + m_clubBgPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[0][1] + m_clubBgPos[1]), 0);
	DrawOverlayByIndex983(m_selTextOverlay[0], (int)(m_pForm->GetRect().x + m_clubPos[0][0] + m_selTextPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[0][1] + m_selTextPos[1]), 0);
	DrawOverlayByIndex983(m_rankOverlay[0], (int)(m_pForm->GetRect().x + m_clubPos[0][0] + m_rankPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[0][1] + m_rankPos[1]), 0);
}

// 004c0920: taco de destino (sem a marca do rank)
void FrClubSetOptionalFunctionsWnd::OnDrawClubselectExpmove1_983()
{
	FrGraphicInterface* pGDI = FORMGDI983();
	if (pGDI && m_clubBmp[1])
		pGDI->DrawTexture(m_clubBmp[1], WRect(m_pForm->GetRect().x + m_clubPos[1][0], m_pForm->GetRect().y + m_clubPos[1][1],
			(float)m_clubBmp[1]->Width(), (float)m_clubBmp[1]->Height()), 0xffffffff, 0);
	DrawOverlayByIndex983(m_clubBgOverlay[1], (int)(m_pForm->GetRect().x + m_clubPos[1][0] + m_clubBgPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[1][1] + m_clubBgPos[1]), 0);
	DrawOverlayByIndex983(m_selTextOverlay[1], (int)(m_pForm->GetRect().x + m_clubPos[1][0] + m_selTextPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[1][1] + m_selTextPos[1]), 0);
}

// 004c0a50: taco a recuperar
void FrClubSetOptionalFunctionsWnd::OnDrawClubselectRecovery983()
{
	FrGraphicInterface* pGDI = FORMGDI983();
	if (pGDI && m_clubBmp[2])
		pGDI->DrawTexture(m_clubBmp[2], WRect(m_pForm->GetRect().x + m_clubPos[2][0], m_pForm->GetRect().y + m_clubPos[2][1],
			(float)m_clubBmp[2]->Width(), (float)m_clubBmp[2]->Height()), 0xffffffff, 0);
	DrawOverlayByIndex983(m_clubBgOverlay[2], (int)(m_pForm->GetRect().x + m_clubPos[2][0] + m_clubBgPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[2][1] + m_clubBgPos[1]), 0);
	DrawOverlayByIndex983(m_selTextOverlay[2], (int)(m_pForm->GetRect().x + m_clubPos[2][0] + m_selTextPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[2][1] + m_selTextPos[1]), 0);
	DrawOverlayByIndex983(m_rankOverlay[1], (int)(m_pForm->GetRect().x + m_clubPos[2][0] + m_rankPos[0]),
		(int)(m_pForm->GetRect().y + m_clubPos[2][1] + m_rankPos[1]), 0);
}

// 004c0c70
void FrClubSetOptionalFunctionsWnd::Notify983(int kind)
{
	switch (kind)
	{
	case 1:
	{
		FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), m_pForm, "notify", NULL);
		if (pDlg)
		{
			// "Experiência movida com sucesso."
			pDlg->SetMessage(K2L_Compatibility("\274\272\260\370\300\373\300\270\267\316 \274\367\267\303\265\265\260\241 \300\314"
				"\300\374\265\307\276\372\275\300\264\317\264\331."), false);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareNotifyResultA983, 0x802);
		}
		ClubSetWorkshopFx983::StopFx983(3);
		m_bExpSending = false;
		break;
	}
	case 2:
	{
		FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), m_pForm, "notify", NULL);
		if (pDlg)
		{
			// "As vezes de cancelar a modificação foram recuperadas."
			pDlg->SetMessage(K2L_Compatibility("\260\263\301\266 \303\353\274\322 \260\241\264\311 \310\275\274\366\260\241 \310\270"
				"\272\271\265\307\276\372\275\300\264\317\264\331."), false);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareNotifyResultB983, 0x802);
		}
		ClubSetWorkshopFx983::StopFx983(4);
		m_bRecSending = false;
		break;
	}
	}
}

// 004c0e30
void FrClubSetOptionalFunctionsWnd::Proc983(float deltaTime)
{
	if (!m_bShow)
		return;
	if (m_srcGuid && m_dstGuid)
		m_expAnim.Proc983(deltaTime, this);
	m_blinkTime += deltaTime;
	if (m_blinkTime > 0.5f)
	{
		if (!m_srcGuid)
			SetOverlayUse983(m_selTextOverlay[0], m_bBlink);
		else
			SetOverlayUse983(m_selTextOverlay[0], true);
		if (!m_dstGuid)
			SetOverlayUse983(m_selTextOverlay[1], m_bBlink);
		else
			SetOverlayUse983(m_selTextOverlay[1], true);
		if (!m_recGuid)
			SetOverlayUse983(m_selTextOverlay[2], m_bBlink);
		else
			SetOverlayUse983(m_selTextOverlay[2], true);
		m_blinkTime = 0;
		m_bBlink = !m_bBlink;
	}
	if (m_notifyKind)
	{
		g_input->SetFakeInput(LEFT_BUTTON);
		g_input->SetFakeInput("ESCAPE", true);
		m_notifyTime += deltaTime;
		if (m_notifyTime > m_notifyDelay)
		{
			Notify983(m_notifyKind);
			m_notifyTime = 0;
			m_notifyKind = 0;
			return;
		}
		if (m_notifyKind == 1)
			ClubSetWorkshopFx983::HoldFx983(3);
		else
			ClubSetWorkshopFx983::HoldFx983(4);
	}
}

// 004c0fc0: "Mover a experiência?" com origem(−) → destino(+) e as figuras dos dois tacos
void FrClubSetOptionalFunctionsWnd::OnConfirmExpmoveBtnUp983()
{
	ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
	if (!pDlg)
		return;
	ClubSetWorkshopFx983::sSlot983* pSrc = ClubSetWorkshopFx983::GetSlot983(2);
	ClubSetWorkshopFx983::sSlot983* pDst = ClubSetWorkshopFx983::GetSlot983(3);
	if (!pSrc || !pDst)
		return;
	OnBeginWork983();
	int move = 99900 - pDst->exp;
	if (m_moveExp <= move)
		move = m_moveExp;
	pDlg->SetSize983(320.0f, 260.0f, true);
	pDlg->SetMessage(MakeStr("%d(-%d)  ->  %d(+%d)\n\n%s", pSrc->exp, m_moveExp, pDst->exp, move,
		K2L_Compatibility("\274\367\267\303\265\265 \300\314\300\374\300\273 \301\370\307\340\307\317\275\303\260\332\275\300\264\317"
			"\261\356?")), false);
	pDlg->AddBitmap983(pSrc->pBmp);
	pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap("clubmade_arrow"));
	pDlg->AddBitmap983(pDst->pBmp);
	pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareExpMoveConfirmResult983, 3);
}

// 004c1140: "Recuperar ao máximo as vezes de cancelar a modificação?"
void FrClubSetOptionalFunctionsWnd::OnConfirmRecoveryBtnUp983()
{
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(4);
	if (!pSlot)
		return;
	ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
	if (!pDlg)
		return;
	OnBeginWork983();
	pDlg->SetSize983(320.0f, 240.0f, true);
	pDlg->SetMessage(K2L_Compatibility("\260\263\301\266 \303\353\274\322 \260\241\264\311 \310\275\274\366\270\246\n\303\326\264\353"
		"\267\316 \310\270\272\271\307\317\275\303\260\332\275\300\264\317\261\356?"), false);
	pDlg->AddBitmap983(pSlot->pBmp);
	pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareRecoveryConfirmResult983, 3);
}

// 004c1540
FrClubSetOptionalFunctionsWnd::~FrClubSetOptionalFunctionsWnd()
{
	s_pInstance = NULL;
}

// 004c1630
bool FrClubSetOptionalFunctionsWnd::Init983()
{
	CreateFonts983(2);
	AddCtrlByName983("area_npc_img3");
	AddCtrlByName983("area_logo_of");
	AddCtrlByName983("area_expmove_bgfrm_l");
	AddCtrlByName983("area_expmove_bgfrm_m");
	AddCtrlByName983("area_expmove_bgfrm_r");
	AddCtrlByName983("area_recovery_bgfrm_l");
	AddCtrlByName983("area_recovery_bgfrm_m");
	AddCtrlByName983("area_recovery_bgfrm_r");
	AddCtrlByName983("area_title_expmove");
	AddCtrlByName983("area_expmove_icon");
	AddCtrlByName983("area_clubset_exp_bg");
	AddCtrlByName983("area_expmove_bg_bot");
	AddCtrlByName983("area_clubselect_expmove0");
	AddCtrlByName983("area_clubselect_expmove1");
	AddCtrlByName983("btn_confirm_expmove");
	AddCtrlByName983("btn_clubselect_expmove0");
	AddCtrlByName983("btn_clubselect_expmove1");
	AddCtrlByName983("btn_exp_add");
	AddCtrlByName983("btn_exp_sub");
	AddCtrlByName983("static_exppoint0");
	AddCtrlByName983("static_exppoint1");
	AddCtrlByName983("static_exppoint_move");
	AddCtrlByName983("static_itemname_expmove");
	AddCtrlByName983("static_itemcount_expmove");
	AddCtrlByName983("area_expmove_item_icon");
	AddCtrlByName983("area_title_recovery");
	AddCtrlByName983("area_clubselect_recovery");
	AddCtrlByName983("area_clubset_slotadd_bg");
	AddCtrlByName983("area_recovery_bg_bot");
	AddCtrlByName983("btn_confirm_recovery");
	AddCtrlByName983("btn_clubselect_recovery");
	AddCtrlByName983("static_slotadd_recovery");
	AddCtrlByName983("area_recovery_item_icon");
	AddCtrlByName983("area_item_num_recovery");
	AddCtrlByName983("area_addslot_cnt_imgfont");
	AddCtrlByName983("area_npc_speech_op");
	AddCtrlByName983("area_fx_expmove");
	AddCtrlByName983("area_fx_recovery");
	return true;
}

// 004c1b90
void FrClubSetOptionalFunctionsWnd::UpdateRecoveryItem983()
{
	ShowCtrl983("area_recovery_item_icon", m_recItemCount != 0);
}

// 004c1c40
void FrClubSetOptionalFunctionsWnd::OnBeginWork983()
{
	EnableCtrl983("btn_confirm_expmove", false);
	EnableCtrl983("btn_confirm_recovery", false);
	ClubSetWorkshopChildWndBase::OnBeginWork983();
}

// 004c1db0
FrClubSetOptionalFunctionsWnd::FrClubSetOptionalFunctionsWnd(ClubSetWorkShopForm* pForm)
{
	s_pInstance = this;
	// campos lidos pelo 983 nesses deslocamentos (disasm 004c27a0/004c2ba0/004c2fb0/004c3af0/004c3b60, A410-A412)
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_srcGuid) == 0x88 && offsetof(FrClubSetOptionalFunctionsWnd, m_moveExp) == 0xa0);
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_recGuid) == 0xa8 && offsetof(FrClubSetOptionalFunctionsWnd, m_expItemCount) == 0x144);
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_notifyKind) == 0x154 && offsetof(FrClubSetOptionalFunctionsWnd, m_bRecSending) == 0x15e);
	// envio no OK (004c3af0: [+0x88],[+0x8c],[+0x90],[+0x94], trava +0x15d; 004c3b60: [+0xa8],[+0xac]; result == 1, A416)
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_dstGuid) == 0x8c && offsetof(FrClubSetOptionalFunctionsWnd, m_expItemTypeId) == 0x90);
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_expItemUse) == 0x94 && offsetof(FrClubSetOptionalFunctionsWnd, m_recItemTypeId) == 0xac);
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_bExpSending) == 0x15d && FrOK == 1);
	// aviso do resultado (004c0f2a: +0x14c += dt; > +0x150 (fx_clubmanage_playtime, 004c6163) -> 004c0c70; SetFakeInput(0), A419)
	C_ASSERT_VC10(offsetof(FrClubSetOptionalFunctionsWnd, m_notifyTime) == 0x14c && offsetof(FrClubSetOptionalFunctionsWnd, m_notifyDelay) == 0x150 && LEFT_BUTTON == 0);
	m_notifyTime = 0;
	m_notifyDelay = 0;
	m_pForm = pForm;
	m_blinkTime = 0;
	m_srcGuid = 0;
	m_dstGuid = 0;
	m_expItemTypeId = 0;
	m_expItemUse = 0;
	m_srcExp = 0;
	m_moveExp = 0;
	m_expItemName = NULL;
	m_recGuid = 0;
	m_recItemTypeId = 0;
	m_clubBmp[0] = NULL;
	m_clubBmp[1] = NULL;
	m_clubBmp[2] = NULL;
	m_clubBgOverlay[0] = 0;
	m_clubBgOverlay[1] = 0;
	m_clubBgOverlay[2] = 0;
	m_expIconName[0] = NULL;
	m_expIconName[1] = NULL;
	m_expAnimName = NULL;
	m_expItemCount = 0;
	m_recItemCount = 0;
	m_notifyKind = 0;
	m_bBlink = false;
	m_bExpSending = false;
	m_bRecSending = false;
}

// 004c1f20
void FrClubSetOptionalFunctionsWnd::OnClubselectExpmove0BtnUp983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pForm, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long guid = m_srcGuid;
		pDlg->m_marked.push_back(guid);
		guid = m_dstGuid;
		pDlg->m_marked.push_back(guid);
		pDlg->m_kind = 3;
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareExpMoveSel0Result983, 1);
	}
}

// 004c1fc0
void FrClubSetOptionalFunctionsWnd::OnClubselectExpmove1BtnUp983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pForm, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long guid = m_srcGuid;
		pDlg->m_marked.push_back(guid);
		guid = m_dstGuid;
		pDlg->m_marked.push_back(guid);
		pDlg->m_kind = 4;
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareExpMoveSel1Result983, 1);
	}
}

// 004c2060
void FrClubSetOptionalFunctionsWnd::OnClubselectRecoveryBtnUp983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pForm, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long guid = m_recGuid;
		pDlg->m_marked.push_back(guid);
		pDlg->m_kind = 5;
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnCareRecoverySelResult983, 1);
	}
}

// 004c2250
FrClubSetOptionalFunctionsWnd* FrClubSetWorkshopMainDlg::CreateCare983()
{
	return new FrClubSetOptionalFunctionsWnd(this);
}

// 004c2340
void FrClubSetOptionalFunctionsWnd::UpdateExpMoveClubs983()
{
	const char* tip;
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(2);
	if (pSlot)
	{
		m_clubBmp[0] = pSlot->pBmp;
		SetOverlayUse983(m_clubBgOverlay[0], true);
		SetOverlayUse983(m_rankOverlay[0], pSlot->rank == 5);
		m_srcExp = pSlot->exp;
		FrStatic* pStat = GetCtrl983<FrStatic>("static_exppoint0");
		if (pStat)
			pStat->SetCaption(MakeStr("%05d", m_srcExp));
		tip = MakeStr("%s", pSlot->name.c_str());
	}
	else
	{
		m_clubBmp[0] = NULL;
		SetOverlayUse983(m_clubBgOverlay[0], false);
		SetOverlayUse983(m_rankOverlay[0], false);
		m_srcExp = 0;
		FrStatic* pStat = GetCtrl983<FrStatic>("static_exppoint0");
		if (pStat)
			pStat->SetCaption("-");
		tip = "";
	}
	SetCtrlToolTip983("btn_clubselect_expmove0", tip);

	pSlot = ClubSetWorkshopFx983::GetSlot983(3);
	if (pSlot)
	{
		m_clubBmp[1] = pSlot->pBmp;
		SetOverlayUse983(m_clubBgOverlay[1], true);
		m_dstExp = pSlot->exp;
		FrStatic* pStat = GetCtrl983<FrStatic>("static_exppoint1");
		if (pStat)
			pStat->SetCaption(MakeStr("%05d", m_dstExp));
		tip = MakeStr("%s", pSlot->name.c_str());
	}
	else
	{
		m_clubBmp[1] = NULL;
		SetOverlayUse983(m_clubBgOverlay[1], false);
		m_dstExp = 0;
		FrStatic* pStat = GetCtrl983<FrStatic>("static_exppoint1");
		if (pStat)
			pStat->SetCaption("-");
		tip = "";
	}
	SetCtrlToolTip983("btn_clubselect_expmove1", tip);
}

// 004c27a0
void FrClubSetOptionalFunctionsWnd::UpdateExpMove983()
{
	m_moveExp = m_expItemUse * 300;
	if (m_moveExp > m_srcExp)
		m_moveExp = m_srcExp;
	ShowCtrl983("area_expmove_item_icon", m_expItemCount != 0);
	FrStatic* pMove = GetCtrl983<FrStatic>("static_exppoint_move");
	if (pMove)
	{
		if ((unsigned int)m_moveExp > 99900)
			pMove->SetCaption(MakeStr("%05d", 99900));
		else if (m_moveExp > 0)
			pMove->SetCaption(MakeStr("%05d", m_moveExp));
		else
			pMove->SetCaption("-");
	}
	FrStatic* pName = GetCtrl983<FrStatic>("static_itemname_expmove");
	if (pName)
		pName->SetCaption(MakeStr("%s", m_expItemName));
	FrStatic* pCount = GetCtrl983<FrStatic>("static_itemcount_expmove");
	if (pCount)
		pCount->SetCaption(MakeStr("x%04d", m_expItemUse));
	FrButton* pAdd = GetCtrl983<FrButton>("btn_exp_add");
	if (pAdd)
	{
		bool bEnable = true;
		if (m_expItemUse >= m_expItemCount || m_moveExp == m_srcExp || (unsigned int)(m_dstExp + m_moveExp) >= 99900 ||
			m_expItemCount == 0 || m_srcGuid == 0 || m_dstGuid == 0)
			bEnable = false;
		pAdd->Enable(bEnable);
	}
	FrButton* pSub = GetCtrl983<FrButton>("btn_exp_sub");
	if (pSub)
	{
		bool bEnable = true;
		if (m_expItemUse == 0 || m_expItemCount == 0 || m_srcGuid == 0 || m_dstGuid == 0)
			bEnable = false;
		pSub->Enable(bEnable);
	}
}

// 004c2ba0
void FrClubSetOptionalFunctionsWnd::UpdateExpMoveConfirm983()
{
	int state;
	if (m_srcGuid == 0)
		state = 0;
	else if (m_dstGuid == 0)
		state = 1;
	else if (m_srcExp == 0)
		state = 3;
	else
		state = m_expItemUse ? -1 : 2;
	bool bEnable = state == -1;
	FrButton* pBtn = GetCtrl983<FrButton>("btn_confirm_expmove");
	if (!pBtn)
		return;
	std::string tip;
	switch (state)
	{
	case 0: // "Escolha o conjunto de tacos de onde mover."
		tip = K2L_Compatibility("\300\314\300\374\307\322 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\330 \301\326\274"
			"\274\277\344.");
		break;
	case 1: // "Escolha o conjunto de tacos que vai receber."
		tip = K2L_Compatibility("\300\314\300\374 \271\336\300\273 \305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\330 \301"
			"\326\274\274\277\344.");
		break;
	case 2: // "Use o item de mover experiência."
		tip = K2L_Compatibility("\274\367\267\303\265\265 \300\314\300\374 \276\306\300\314\305\333\300\273 \273\347\277\353\307\330\301"
			"\326\274\274\277\344.");
		break;
	case 3: // "A experiência do conjunto de origem é 0."
		tip = K2L_Compatibility("\300\314\300\374\307\322 \305\254\267\264\274\274\306\256\300\307 \274\367\267\303\265\265\260\241 0\300"
			"\324\264\317\264\331.");
		break;
	}
	pBtn->SetToolTipText(bEnable ? "" : tip.c_str());
	pBtn->Enable(bEnable);
}

// 004c2dd0
void FrClubSetOptionalFunctionsWnd::UpdateRecoveryClub983()
{
	int count = ClubSetWorkshopFx983::GetSlotCancelCount983(4);
	int max = ClubSetWorkshopFx983::GetSlotCancelMax983(4);
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(4);
	if (pSlot)
	{
		m_clubBmp[2] = pSlot->pBmp;
		SetOverlayUse983(m_clubBgOverlay[2], true);
		SetOverlayUse983(m_rankOverlay[1], pSlot->rank == 5);
	}
	else
	{
		m_clubBmp[2] = NULL;
		SetOverlayUse983(m_clubBgOverlay[2], false);
		SetOverlayUse983(m_rankOverlay[1], false);
	}
	FrStatic* pStat = GetCtrl983<FrStatic>("static_slotadd_recovery");
	if (pStat)
	{
		if (pSlot)
			pStat->SetCaption(MakeStr("%d / %d", count, max));
		else
			pStat->SetCaption("-");
	}
	SetCtrlToolTip983("btn_clubselect_recovery", pSlot ? MakeStr("%s", pSlot->name.c_str()) : "");
}

// 004c2fb0
void FrClubSetOptionalFunctionsWnd::UpdateRecoveryConfirm983()
{
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(4);
	int state;
	if (m_recGuid == 0 || !pSlot)
		state = 0;
	else if (pSlot->recoveryLeft == pSlot->recoveryMax)
		state = 2;
	else
		state = m_recItemCount == 0 ? 1 : -1;
	bool bEnable = state == -1;
	FrButton* pBtn = GetCtrl983<FrButton>("btn_confirm_recovery");
	if (!pBtn)
		return;
	std::string tip;
	switch (state)
	{
	case 0: // "Escolha o conjunto de tacos."
		tip = K2L_Compatibility("\305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\330 \301\326\274\274\277\344.");
		break;
	case 1: // "Não há item de recuperar."
		tip = K2L_Compatibility("\310\270\272\271 \276\306\300\314\305\333\300\314 \276\370\275\300\264\317\264\331.");
		break;
	case 2: // "As vezes de cancelar a modificação já estão no máximo."
		tip = K2L_Compatibility("\260\263\301\266 \303\353\274\322 \260\241\264\311 \310\275\274\366\260\241 \303\326\264\353 \273\363\305"
			"\302\300\324\264\317\264\331.");
		break;
	}
	pBtn->SetToolTipText(bEnable ? "" : tip.c_str());
	pBtn->Enable(bEnable);
}

// 004c31b0
void FrClubSetOptionalFunctionsWnd::OnDrawNpcImg983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_npc_img3");
	if (pArea)
		DrawOverlay983(m_npcImgName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c32c0
void FrClubSetOptionalFunctionsWnd::OnDrawLogo983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_logo_of");
	if (pArea)
		DrawOverlay983(m_logoName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c33d0
void FrClubSetOptionalFunctionsWnd::OnDrawExpmoveIcon983()
{
	FrArea* pArea = GetCtrl983<FrArea>("area_expmove_icon");
	if (pArea)
	{
		DrawOverlay983(m_expIconName[0], (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
		DrawOverlay983(m_expIconName[1], (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
		DrawOverlay983(m_expAnimName, (int)(m_expAnimPos[0] + pArea->GetRect().x), (int)(m_expAnimPos[1] + pArea->GetRect().y), 0);
	}
}

// 004c3590
void FrClubSetOptionalFunctionsWnd::OnExpAddBtnUp983()
{
	m_expItemUse++;
	UpdateExpMove983();
	if (m_expItemUse == 1)
		UpdateExpMoveConfirm983();
}

// 004c35c0
void FrClubSetOptionalFunctionsWnd::OnExpSubBtnUp983()
{
	m_expItemUse--;
	if (m_expItemUse <= 0)
	{
		m_expItemUse = 0;
		UpdateExpMoveConfirm983();
	}
	UpdateExpMove983();
}

// 004c35f0: itens de mover experiência e de recuperar que o jogador tem
void FrClubSetOptionalFunctionsWnd::OnDrawItemNumRecovery983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_item_num_recovery");
	if (!pArea)
		return;
	if ((int)m_fonts.size() != 0 && m_fonts[0] != NULL)
		GetFont983(0)->Print(m_recNumPos[0] + pArea->GetRect().x + pArea->GetRect().w, m_recNumPos[1] + pArea->GetRect().y,
			MakeStr("%d", m_expItemCount), TFA_RIGHT, 0.0f, 1.0f);
	if ((int)m_fonts.size() != 0 && m_fonts[1] != NULL)
		GetFont983(1)->Print(pArea->GetRect().w + pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", m_recItemCount), TFA_RIGHT,
			0.0f, 1.0f);
}

// 004c37a0
void FrClubSetOptionalFunctionsWnd::OnDrawNpcSpeech983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_npc_speech_op");
	if (pArea)
		DrawOverlayByIndex983(m_npcSpeechOverlay, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c3880
void FrClubSetOptionalFunctionsWnd::OnDrawFxExpmove983()
{
	FrWnd* pWnd = GetCtrl983<FrWnd>("area_fx_expmove");
	if (pWnd && pWnd->IsVisible())
		ClubSetWorkshopFx983::DrawFx983(3);
}

// 004c3940
void FrClubSetOptionalFunctionsWnd::OnDrawFxRecovery983()
{
	FrWnd* pWnd = GetCtrl983<FrWnd>("area_fx_recovery");
	if (pWnd && pWnd->IsVisible())
		ClubSetWorkshopFx983::DrawFx983(4);
}

// 004c3a00
bool FrClubSetOptionalFunctionsWnd::OnExpMoveSel0Result983(int result, FrForm* pForm)
{
	if (result == FrOK && pForm)
	{
		m_srcGuid = ((FrClubSetWorkshopSelector*)pForm)->m_selGuid;
		ClubSetWorkshopFx983::SetSlotClub983(2, m_srcGuid);
		m_expItemUse = 0;
		UpdateExpMoveClubs983();
		UpdateExpMove983();
		UpdateExpMoveConfirm983();
	}
	return true;
}

// 004c3a50
bool FrClubSetOptionalFunctionsWnd::OnExpMoveSel1Result983(int result, FrForm* pForm)
{
	if (result == FrOK && pForm)
	{
		m_dstGuid = ((FrClubSetWorkshopSelector*)pForm)->m_selGuid;
		ClubSetWorkshopFx983::SetSlotClub983(3, m_dstGuid);
		UpdateExpMoveClubs983();
		UpdateExpMove983();
		UpdateExpMoveConfirm983();
	}
	return true;
}

// 004c3aa0
bool FrClubSetOptionalFunctionsWnd::OnRecoverySelResult983(int result, FrForm* pForm)
{
	if (result == FrOK && pForm)
	{
		m_recGuid = ((FrClubSetWorkshopSelector*)pForm)->m_selGuid;
		ClubSetWorkshopFx983::SetSlotClub983(4, m_recGuid);
		UpdateRecoveryClub983();
		UpdateRecoveryItem983();
		UpdateRecoveryConfirm983();
	}
	return true;
}

// 004c3af0
bool FrClubSetOptionalFunctionsWnd::OnExpMoveConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		if (!m_bExpSending && m_expItemUse)
		{
			m_bExpSending = true;
			SendClubSetWork16c_983(m_srcGuid, m_dstGuid, m_expItemTypeId, m_expItemUse);
			ClubSetWorkshopFx983::StartFx983(7);
		}
		return true;
	}
	OnEndWork983();
	UpdateExpMoveConfirm983();
	UpdateRecoveryConfirm983();
	return true;
}

// 004c3b60
bool FrClubSetOptionalFunctionsWnd::OnRecoveryConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		if (!m_bRecSending)
		{
			m_bRecSending = true;
			SendClubSetWork16b_983(m_recGuid, m_recItemTypeId);
			ClubSetWorkshopFx983::StartFx983(7);
		}
		ClubSetWorkshopFx983::StartFx983(7);
		return true;
	}
	OnEndWork983();
	UpdateExpMoveConfirm983();
	UpdateRecoveryConfirm983();
	return true;
}

// 004c3bd0
void FrClubSetOptionalFunctionsWnd::UpdateExpItem983(bool bKeepSlots)
{
	sItemInfo* pItem = FindMyItem983(3);
	if (pItem)
		m_expItemCount = pItem->Common[0];
	else
		m_expItemCount = 0;
	IFF_STRUCT::sItem* pIff = ItemManager()->FindItem(m_expItemTypeId);
	if (pIff)
		m_expItemName = pIff->c.Name;
	if (!bKeepSlots)
	{
		ClubSetWorkshopFx983::ClearSlot983(2);
		ClubSetWorkshopFx983::ClearSlot983(3);
	}
	UpdateExpMoveClubs983();
	UpdateExpMove983();
	UpdateExpMoveConfirm983();
}

// 004c3c60
void FrClubSetOptionalFunctionsWnd::UpdateRecoveryItemCount983(bool bKeepSlot)
{
	sItemInfo* pItem = FindMyItem983(4);
	if (pItem)
		m_recItemCount = pItem->Common[0];
	else
		m_recItemCount = 0;
	if (!bKeepSlot)
		ClubSetWorkshopFx983::ClearSlot983(4);
	UpdateRecoveryClub983();
	UpdateRecoveryItem983();
	UpdateRecoveryConfirm983();
}

// 004c3cc0
bool FrClubSetOptionalFunctionsWnd::OnNotifyResultA983(int result, FrForm* pForm)
{
	m_expItemUse = 0;
	m_bExpSending = false;
	OnEndWork983();
	UpdateExpItem983(false);
	UpdateRecoveryItemCount983(false);
	return true;
}

// 004c3d00
bool FrClubSetOptionalFunctionsWnd::OnNotifyResultB983(int result, FrForm* pForm)
{
	m_bRecSending = false;
	OnEndWork983();
	UpdateExpItem983(false);
	UpdateRecoveryItemCount983(false);
	return true;
}

// 004c3d30
void FrClubSetOptionalFunctionsWnd::ResetExpMove983()
{
	m_expItemUse = 0;
	m_expItemCount = 0;
	m_srcGuid = 0;
	m_dstGuid = 0;
	m_expItemTypeId = GetTypeId983(3);
	ClubSetWorkshopFx983::ResetSlot983(2);
	ClubSetWorkshopFx983::ResetSlot983(3);
	m_bExpSending = false;
	UpdateExpItem983(true);
}

// 004c3d80
void FrClubSetOptionalFunctionsWnd::ResetRecovery983()
{
	m_recItemCount = 0;
	m_recGuid = 0;
	m_recItemTypeId = GetTypeId983(4);
	ClubSetWorkshopFx983::ResetSlot983(4);
	m_bRecSending = false;
	UpdateRecoveryItemCount983(true);
}

// 004c3dd0
void FrClubSetOptionalFunctionsWnd::OnRecoveryResult983(WReceivedPacket* pPacket)
{
	if (pPacket)
		m_notifyKind = 2;
	else
	{
		OnEndWork983();
		m_recGuid = 0;
		ResetRecovery983();
	}
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004c3e20
void FrClubSetOptionalFunctionsWnd::OnExpMoveResult983(WReceivedPacket* pPacket)
{
	if (pPacket)
		m_notifyKind = 1;
	else
	{
		OnEndWork983();
		ResetExpMove983();
	}
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004c3e60
void FrClubSetOptionalFunctionsWnd::SetShow983(bool bShow)
{
	ClubSetWorkshopChildWndBase::SetShow983(bShow);
	if (bShow)
	{
		ResetExpMove983();
		ResetRecovery983();
	}
}

// 004c3e90
void FrClubSetOptionalFunctionsWnd::ResetAll983()
{
	ResetExpMove983();
	ResetRecovery983();
}

// 004c3ea0: tipos dos itens/cartas da oficina (clubset_workshop_tid_table), figuras e posições da página
// (clubset_workshop_ui_optionalfunc; o npc, o logo, o balão, a marca do rank S e o tempo do efeito pela clubset_workshop_ui_main),
// molduras de três partes dos dois quadros (bgfrm_l/m/r), ícones dos itens e áreas dos efeitos 3/4 do Fx. Sessão B83.
void FrClubSetOptionalFunctionsWnd::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	{
		lua_system::lua_table_helper tidTbl = script->GetTable("clubset_workshop_tid_table");
		for (int i = 0; i < 5; i++)
			m_typeIds.push_back(((LuaHelper983&)tidTbl).GetChild983("clubset_workshop_tid_list").GetInl983<unsigned long>(i + 1));
	}
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_optionalfunc");

	m_npcImgName = LUA983_GET(tbl, const char*, "npc_img_filename");
	m_logoName = LUA983_GET(tbl, const char*, "logo_img_filename");
	LoadOverlay983(m_npcImgName, true);
	LoadOverlay983(m_logoName, true);
	m_expIconName[0] = LUA983_GET(tbl, const char*, "icon_img_expmove_filename0");
	m_expIconName[1] = LUA983_GET(tbl, const char*, "icon_img_expmove_filename1");
	m_expAnim.frames.push_back(LoadOverlay983(m_expIconName[0], true));
	m_expAnim.frames.push_back(LoadOverlay983(m_expIconName[1], false));
	m_expAnim.interval = 0.5f;
	m_clubPos[0][0] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_expmove0_pos").GetInl983<int>(1);
	m_clubPos[0][1] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_expmove0_pos").GetInl983<int>(2);
	m_clubPos[1][0] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_expmove1_pos").GetInl983<int>(1);
	m_clubPos[1][1] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_expmove1_pos").GetInl983<int>(2);
	m_clubPos[2][0] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_recovery_pos").GetInl983<int>(1);
	m_clubPos[2][1] = (float)((LuaHelper983&)tbl).GetChild983("clubset_icon_recovery_pos").GetInl983<int>(2);
	m_recNumPos[0] = (float)((LuaHelper983&)tbl).GetChild983("img_font_pos_expmove").GetInl983<int>(1);
	m_recNumPos[1] = (float)((LuaHelper983&)tbl).GetChild983("img_font_pos_expmove").GetInl983<int>(2);

	const char* selName = LUA983_GET(tbl, const char*, "clubset_select_o_filename");
	m_clubBgPos[0] = (float)((LuaHelper983&)tbl).GetChild983("clubset_select_img_pos").GetInl983<int>(1);
	m_clubBgPos[1] = (float)((LuaHelper983&)tbl).GetChild983("clubset_select_img_pos").GetInl983<int>(2);
	m_clubBgOverlay[0] = LoadOverlay983(selName, false);
	m_clubBgOverlay[1] = LoadOverlay983(selName, false);
	m_clubBgOverlay[2] = LoadOverlay983(selName, false);
	const char* selText = LUA983_GET(tbl, const char*, "clubset_select_text_filename");
	m_selTextPos[0] = (float)((LuaHelper983&)tbl).GetChild983("clubset_select_text_pos").GetInl983<int>(1);
	m_selTextPos[1] = (float)((LuaHelper983&)tbl).GetChild983("clubset_select_text_pos").GetInl983<int>(2);
	m_selTextOverlay[0] = LoadOverlay983(selText, true);
	m_selTextOverlay[1] = LoadOverlay983(selText, true);
	m_selTextOverlay[2] = LoadOverlay983(selText, true);

	OffsetCtrlLua983("area_npc_img3", &mainTbl, "npc_img_x", "npc_img_y");
	OffsetCtrlLua983("area_logo_of", &mainTbl, "logo_img_x", "logo_img_y");

	// moldura do quadro "mover experiência": esquerda e meio postos lado a lado a partir de bg_img_expmove_pos
	{
		int x = ((LuaHelper983&)tbl).GetChild983("bg_img_expmove_pos").GetInl983<int>(1);
		int y = ((LuaHelper983&)tbl).GetChild983("bg_img_expmove_pos").GetInl983<int>(2);
		int w = LUA983_GET(tbl, int, "bg_img_expmove_width");
		FrArea* pL = GetCtrl983<FrArea>("area_expmove_bgfrm_l");
		if (pL)
		{
			const WRect& rc = pL->GetRect();
			pL->SetRect(WRect(x + rc.x, rc.y + y, rc.w, rc.h));
			x += rc.w;
		}
		FrArea* pM = GetCtrl983<FrArea>("area_expmove_bgfrm_m");
		if (pM)
		{
			const WRect& rc = pM->GetRect();
			pM->SetRect(WRect(x + rc.x, rc.y + y, (float)w, rc.h));
			x += w;
		}
		OffsetCtrl983("area_expmove_bgfrm_r", (float)x, (float)y);
	}
	m_expAnimName = LUA983_GET(tbl, const char*, "bg_img_move_exppoint_filename");
	m_expAnimPos[0] = (float)((LuaHelper983&)tbl).GetChild983("bg_img_move_exppoint_pos").GetInl983<int>(1);
	m_expAnimPos[1] = (float)((LuaHelper983&)tbl).GetChild983("bg_img_move_exppoint_pos").GetInl983<int>(2);
	LoadOverlay983(m_expAnimName, true);
	// moldura do quadro "recuperar"
	{
		int x = ((LuaHelper983&)tbl).GetChild983("bg_img_recovery_pos").GetInl983<int>(1);
		int y = ((LuaHelper983&)tbl).GetChild983("bg_img_recovery_pos").GetIntByIndex983(2);
		int w = LUA983_GET(tbl, int, "bg_img_recovery_width");
		FrArea* pL = GetCtrl983<FrArea>("area_recovery_bgfrm_l");
		if (pL)
		{
			const WRect& rc = pL->GetRect();
			pL->SetRect(WRect(x + rc.x, rc.y + y, rc.w, rc.h));
			x += rc.w;
		}
		FrArea* pM = GetCtrl983<FrArea>("area_recovery_bgfrm_m");
		if (pM)
		{
			const WRect& rc = pM->GetRect();
			pM->SetRect(WRect(x + rc.x, rc.y + y, (float)w, rc.h));
			x += w;
		}
		OffsetCtrl983("area_recovery_bgfrm_r", (float)x, (float)y);
	}

	OffsetCtrlLua983("area_title_expmove", &tbl, "title_expmove_pos");
	OffsetCtrlLua983("area_expmove_icon", &tbl, "icon_img_expmove_pos");
	OffsetCtrlLua983("area_clubset_exp_bg", &tbl, "bg_clubset_own_exp_pos");
	OffsetCtrlLua983("area_expmove_bg_bot", &tbl, "bg_expmove_bottom_pos");
	OffsetCtrlLua983("area_clubselect_expmove0", &tbl, "clubset_select_expmove0_pos");
	OffsetCtrlLua983("area_clubselect_expmove1", &tbl, "clubset_select_expmove1_pos");
	OffsetCtrlLua983("btn_confirm_expmove", &tbl, "expmove_confirm_btn_pos");
	OffsetCtrlLua983("btn_clubselect_expmove0", &tbl, "clubset_select_btn_expmove0_pos");
	OffsetCtrlLua983("btn_clubselect_expmove1", &tbl, "clubset_select_btn_expmove1_pos");
	OffsetCtrlLua983("btn_exp_add", &tbl, "btn_exp_add_pos");
	OffsetCtrlLua983("btn_exp_sub", &tbl, "btn_exp_sub_pos");
	OffsetCtrlLua983("static_exppoint0", &tbl, "text_clubset_own_exp0_pos");
	OffsetCtrlLua983("static_exppoint1", &tbl, "text_clubset_own_exp1_pos");
	OffsetCtrlLua983("static_exppoint_move", &tbl, "text_clubset_expmove_pos");
	OffsetCtrlLua983("static_itemname_expmove", &tbl, "text_expmove_itemname_pos");
	OffsetCtrlLua983("static_itemcount_expmove", &tbl, "text_expmove_itemcount_pos");
	OffsetCtrlLua983("area_expmove_item_icon", &tbl, "icon_img_expmove_item_pos");
	OffsetCtrlLua983("area_title_recovery", &tbl, "title_recovery_pos");
	OffsetCtrlLua983("area_clubselect_recovery", &tbl, "clubset_select_recovery_pos");
	OffsetCtrlLua983("area_clubset_slotadd_bg", &tbl, "bg_slotadd_cnt_pos");
	OffsetCtrlLua983("area_recovery_bg_bot", &tbl, "bg_recovery_bottom_pos");
	OffsetCtrlLua983("btn_confirm_recovery", &tbl, "recovery_confirm_btn_pos");
	OffsetCtrlLua983("btn_clubselect_recovery", &tbl, "clubset_select_btn_recovery_pos");
	OffsetCtrlLua983("static_slotadd_recovery", &tbl, "text_clubset_slotadd_cnt");
	OffsetCtrlLua983("area_recovery_item_icon", &tbl, "icon_img_recovery_item_pos");
	OffsetCtrlLua983("area_item_num_expmove", &tbl, "img_font_pos_expmove");
	OffsetCtrlLua983("area_item_num_recovery", &tbl, "img_font_pos_recovery");
	OffsetCtrlLua983("area_addslot_cnt_imgfont", &tbl, "letter_slotadd_cnt_pos");
	OffsetCtrlLua983("area_npc_speech_op", &mainTbl, "img_npc_speech_pos");
	OffsetCtrlLua983("area_fx_expmove", &tbl, "fx_area_expmove_pos");
	OffsetCtrlLua983("area_fx_recovery", &tbl, "fx_area_recovery_pos");

	const char* speechName = LUA983_GET(tbl, const char*, "img_npc_speech_filename");
	m_npcSpeechOverlay = LoadOverlay983(speechName, true);
	IFF_STRUCT::sItem* pExpItem = ItemManager()->FindItem(GetTypeId983(3));
	IFF_STRUCT::sItem* pRecItem = ItemManager()->FindItem(GetTypeId983(4));
	if (pRecItem)
		SetCtrlToolTip983("area_recovery_item_icon", MakeStr("%s", pRecItem->c.Name));
	FrArea* pExpIcon = GetCtrl983<FrArea>("area_expmove_item_icon");
	if (pExpItem && pExpIcon)
	{
		const Bitmap* pBmp = g_pFresh->RegisterBitmap(pExpItem->c.Icon);
		if (pBmp)
			pExpIcon->SetBgImg(pBmp);
	}
	FrArea* pRecIcon = GetCtrl983<FrArea>("area_recovery_item_icon");
	if (pRecItem && pRecIcon)
	{
		const Bitmap* pBmp = g_pFresh->RegisterBitmap(pRecItem->c.Icon);
		if (pBmp)
			pRecIcon->SetBgImg(pBmp);
	}

	const char* rankName = LUA983_GET(mainTbl, const char*, "img_srank_clubselect_border_filename");
	m_rankPos[0] = (float)((LuaHelper983&)tbl).GetChild983("img_srank_clubselect_border_pos").GetIntByIndex983(1);
	m_rankPos[1] = (float)((LuaHelper983&)tbl).GetChild983("img_srank_clubselect_border_pos").GetIntByIndex983(2);
	m_rankOverlay[0] = LoadOverlay983(rankName, false);
	m_rankOverlay[1] = LoadOverlay983(rankName, false);
	m_notifyDelay = LUA983_GET(mainTbl, float, "fx_clubmanage_playtime");

	FrWnd* pFx = FindCtrl983("area_fx_expmove");
	if (pFx)
		ClubSetWorkshopFx983::SetFxRect983(3, &pFx->GetRect());
	pFx = FindCtrl983("area_fx_recovery");
	if (pFx)
		ClubSetWorkshopFx983::SetFxRect983(4, &pFx->GetRect());
	ResetAll983();
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}
