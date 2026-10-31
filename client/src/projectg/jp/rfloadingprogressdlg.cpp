// JP 983: unidade 005d1580–005d1a70 — RFLoadingProgressDlg, a janela modal de espera do lobby RF
// ("common_loading_waiting_dialog.xml"), e a 1ª cópia de RFManager::CreateWnd<T>/CreateDesktop<T> dela (005d16b0/005d1740).
// 005d1580/005d1590 (gravam/zeram a instância; ninguém chama) abrem a unidade. Sessão B137.
// Nomes meus (…983), menos a classe e os da RF.
#include "stdafx.h"
#include "refresh/rfloadingprogressdlg.h"

extern int g_rfModalCreateMode983; // 00e03260 (= 1): 0 = filha do pai, 1 = sem pai, 2 = área de trabalho

RFLoadingProgressDlg* RFLoadingProgressDlg::ms_pInstance;
RFWnd* RFLoadingProgressDlg::s_pParent983;
int RFLoadingProgressDlg::s_218_983;

// 005d1580
void RFLoadingProgressDlg::SetInstance983()
{
	ms_pInstance = this;
}

// 005d1590
void RFLoadingProgressDlg::ClearInstance983()
{
	ms_pInstance = NULL;
}

// 005d15a0
void RFLoadingProgressDlg::OnInitialUpdate()
{
	SetPreDraw(false);
}

// 005d15b0
RFLoadingProgressDlg::RFLoadingProgressDlg()
{
	ms_pInstance = this;
}

// 005d1640
RFLoadingProgressDlg::~RFLoadingProgressDlg()
{
	ms_pInstance = NULL;
}

// 005d1800
void RFLoadingProgressDlg::OnDestroy()
{
	s_218_983 = 0;
	RFFormModalBase<RFWndEx>::OnDestroy();
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->SetRFMouseFocus983(false);
}

// 005d1870
RFLoadingProgressDlg* RFLoadingProgressDlg::Create983(RFWnd* parent)
{
	RFLoadingProgressDlg* p = NULL;
	switch (g_rfModalCreateMode983)
	{
	case 0:
		p = RFManager::GetInstance()->CreateWnd<RFLoadingProgressDlg>(L"common_loading_waiting_dialog.xml", parent);
		break;
	case 1:
		p = RFManager::GetInstance()->CreateWnd<RFLoadingProgressDlg>(L"common_loading_waiting_dialog.xml", NULL);
		break;
	case 2:
		p = RFManager::GetInstance()->CreateDesktop<RFLoadingProgressDlg>(L"common_loading_waiting_dialog.xml");
		break;
	}
	s_pParent983 = parent;
	return p;
}

// 005d19b0
void RFLoadingProgressDlg::Start983()
{
	DoModal();
	PlayAnimation(L"_Opening_");
	if (RFSceneManager::Instance())
		RFSceneManager::Instance()->SetRFMouseFocus983(true);
}
