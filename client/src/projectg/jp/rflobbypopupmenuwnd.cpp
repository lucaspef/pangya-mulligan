// JP 983: unidade 005d5d20–005d71e0 — RFLobbyPopupMenuWnd, o menu de botões do lobby RF ("lobby_popupmenu_wnd_jp.xml", dentro
// do "panel_popup" do RFLobbyDesktop). Cada botão repassa o clique a um OnMenu*983 da área de trabalho (que fecha o menu).
// Sessão B136. Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rflobbypopupmenuwnd.h"
#include "refresh/rflobbydesktop.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "taskmain.h"
#include "scratchdlg.h"

extern Fresh* g_pFresh;

extern int g_rfCreateMode983; // 00e73e48: 0 = filha do pai, 1 = sem pai, 2 = área de trabalho

RFLobbyPopupMenuWnd* RFLobbyPopupMenuWnd::ms_pInstance;
RFWnd* RFLobbyPopupMenuWnd::s_pParent983;

// 005d5d20
RFLobbyPopupMenuWnd::RFLobbyPopupMenuWnd()
{
	ms_pInstance = this;
}

// 005d5d40
RFLobbyPopupMenuWnd::~RFLobbyPopupMenuWnd()
{
	ms_pInstance = NULL;
}

// 005d5d60
void RFLobbyPopupMenuWnd::OnKillFocus983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
	{
		RFRect rect = GetScreenRect();
		WVector* pos = RFSceneManager::Instance()->GetPos983();
		if (!rect.PtInRect(pos->x, pos->y))
			RFLobbyDesktop::Instance983()->FocusPopup983();
		RFLobbyDesktop::Instance983()->OnPopupKillFocus983(e);
	}
}

// Scratchy (ponto 6 do dono, 16/10 13:20): no 983 puro btn_scratch e btn_gacha vão os dois para a página web do Gacha
// (005fb620 e 005fb800 chamam 00658e70). A janela "Scratchy Card" que o dono vê no original vem do rugburn de TERCEIRO
// (out/F4/ijl15_rugburn_terceiro.dll 10093130, "Patched Scratchy Card": troca o 005d5dc0). Pedida pelo dono, fica aqui sob
// esta chave; apague a linha para voltar ao 983 puro (REGRAS 7).
#define JP983_SCRATCHY_TERCEIRO

// 005d5dc0 ("btn_scratch" → 005fb620)
void RFLobbyPopupMenuWnd::OnScratchClick983(EventArgs& e)
{
	RFLobbyDesktop* pDesktop = RFLobbyDesktop::Instance983();
	if (!pDesktop)
		return;
#ifdef JP983_SCRATCHY_TERCEIRO
	// como o remendo: sem "scratchdlg" aberta, cria a FrScratchDlg nativa (008ec8b0) no task main e abre; depois fecha o menu
	CTaskMain* pTaskMain = pDesktop->m_pTaskMain;
	if (pTaskMain)
	{
		FrWndManager* pManager = g_pFresh->GetManager();
		if (!pManager->GetDesktop()->FindChildByName("scratchdlg"))
		{
			FrScratchDlg* pDlg = CreateForm<FrScratchDlg>(pManager, pTaskMain, "scratchdlg", NULL);
			if (pDlg)
				pDlg->Open(NULL, 0x11);
		}
	}
	pDesktop->ShowPopup983(false);
#else
	pDesktop->OnMenuGacha983(e);
#endif
}

// 005d5de0 ("btn_bong" → 005fb670)
void RFLobbyPopupMenuWnd::OnBongClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuBongdari983(e);
}

// 005d5e00 ("btn_itemtransform" → 005fb690)
void RFLobbyPopupMenuWnd::OnItemTransformClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuPointShop983(e);
}

// 005d5e20 ("btn_quest" → 005fb6b0)
void RFLobbyPopupMenuWnd::OnQuestClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuOneDayQuest983(e);
}

// 005d5e40 ("btn_notice" → 005fb700)
void RFLobbyPopupMenuWnd::OnNoticeClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuNotice983(e);
}

// 005d5e60 ("btn_ranking" → 005fb730)
void RFLobbyPopupMenuWnd::OnRankingClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuRanking983(e);
}

// 005d5e80 ("btn_achieve" → 005fb760)
void RFLobbyPopupMenuWnd::OnAchieveClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuAchievement983(e);
}

// 005d5ea0 ("btn_myguild" → 005fb780)
void RFLobbyPopupMenuWnd::OnMyGuildClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuClub983(e);
}

// 005d5ec0 ("btn_center" → 005fb7e0)
void RFLobbyPopupMenuWnd::OnCenterClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuCombine983(e);
}

// 005d5ee0 ("btn_gacha" → 005fb800)
void RFLobbyPopupMenuWnd::OnGachaClick983(EventArgs& e)
{
	if (RFLobbyDesktop::Instance983())
		RFLobbyDesktop::Instance983()->OnMenuGacha2_983(e);
}

// 005d6170
RFLobbyPopupMenuWnd* RFLobbyPopupMenuWnd::Create983(RFWnd* parent)
{
	RFLobbyPopupMenuWnd* p = NULL;
	switch (g_rfCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyPopupMenuWnd>(L"lobby_popupmenu_wnd_jp.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLobbyPopupMenuWnd>(L"lobby_popupmenu_wnd_jp.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLobbyPopupMenuWnd>(L"lobby_popupmenu_wnd_jp.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d62b0
void RFLobbyPopupMenuWnd::OnInitialUpdate()
{
	RegisterTo(L"", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_scratch", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_bong", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_itemtransform", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_quest", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_notice", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_ranking", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_achieve", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_myguild", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_center", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_gacha", RFWnd::EventKillFocus, Subscriber(&RFLobbyPopupMenuWnd::OnKillFocus983, this));
	RegisterTo(L"btn_scratch", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnScratchClick983, this));
	RegisterTo(L"btn_bong", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnBongClick983, this));
	RegisterTo(L"btn_itemtransform", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnItemTransformClick983, this));
	RegisterTo(L"btn_quest", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnQuestClick983, this));
	RegisterTo(L"btn_notice", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnNoticeClick983, this));
	RegisterTo(L"btn_ranking", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnRankingClick983, this));
	RegisterTo(L"btn_achieve", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnAchieveClick983, this));
	RegisterTo(L"btn_myguild", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnMyGuildClick983, this));
	RegisterTo(L"btn_center", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnCenterClick983, this));
	RegisterTo(L"btn_gacha", RFButton::EventClicked, Subscriber(&RFLobbyPopupMenuWnd::OnGachaClick983, this));
	SetPreDraw(false);
}
