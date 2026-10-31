// JP 983: unidade 004edd80–004ee570 (FrClubSetWorkshopAboutDlg, ajuda da oficina de tacos). Classe do 839/983, sem par no
// 645; ver include/jp/clubsetworkshopaboutdlg.h. Sessão B91 (F3.7.9, 16º lote). O compilador gera (e casam): os ajustes
// 004ede40/004ede50, o 004ede60 (chamada virtual do OnCtrlCreated983 pelo mapa), 004edf30 (CreateForm<FrClubSetWorkshopAboutDlg>),
// 004ee010 / 004ee170 (GetCtrl983<FrViewer> / GetCtrl983<FrTextButton>) e os 12 handlers 004ee490–004ee550 (no cabeçalho).
// GetCtrl983<FrButton> é o 0049f860 da unidade da FrClubSetWorkshopProgressWnd.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "commonutil.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopaboutdlg.h"

FrClubSetWorkshopAboutDlg* FrClubSetWorkshopAboutDlg::s_pInstance = NULL;

// 004edd80
FrWnd* FrClubSetWorkshopAboutDlg::OnCtrlCreated983(IObject* pObj)
{
	return AddCtrlByType983(pObj);
}

// handler das derivadas da ClubSetWorkShopForm (004ede60 = chamada virtual do +0x98), gravado como VI
#define ON_FRESH_CTRL983(name) \
	{ (FRESH_PFN)(void (FrCmdTarget::*)(IObject*))(void (ClubSetWorkShopForm::*)(IObject*)) \
		&ClubSetWorkShopForm::OnCtrlCreated983, name, FRCMD_INIT, FrFuncVI },

// 004edd90 (GetMessageMap); mapa 00e075d8 (25 entradas)
BEGIN_FRESH_MSGMAP(FrClubSetWorkshopAboutDlg, ClubSetWorkShopForm)
ON_FRESH_CTRL983("about_viewer")
ON_FRESH_CTRL983("index01")
ON_FRESH_CTRL983("index02")
ON_FRESH_CTRL983("index03")
ON_FRESH_CTRL983("index04")
ON_FRESH_CTRL983("index05")
ON_FRESH_CTRL983("index06")
ON_FRESH_CTRL983("index07")
ON_FRESH_CTRL983("index08")
ON_FRESH_CTRL983("index09")
ON_FRESH_CTRL983("index10")
ON_FRESH_CTRL983("backward")
ON_FRESH_CTRL983("forward")
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex01Up983)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex02Up983)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex03Up983)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex04Up983)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex05Up983)
ON_FRESH_VV("index06", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex06Up983)
ON_FRESH_VV("index07", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex07Up983)
ON_FRESH_VV("index08", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex08Up983)
ON_FRESH_VV("index09", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex09Up983)
ON_FRESH_VV("index10", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnIndex10Up983)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnBackwardUp983)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrClubSetWorkshopAboutDlg::OnForwardUp983)
END_FRESH_MSGMAP()

// 004edda0
FrClubSetWorkshopAboutDlg::FrClubSetWorkshopAboutDlg()
{
	s_pInstance = this;
	m_page = 0;
	m_pBackward = NULL;
	m_pForward = NULL;
	m_unused148983 = NULL;
	m_pViewer = NULL;
}

IMPLEMENT_OBJECT(FrClubSetWorkshopAboutDlg, ClubSetWorkShopForm)

// 004ede00
FrClubSetWorkshopAboutDlg::~FrClubSetWorkshopAboutDlg()
{
	s_pInstance = NULL;
}

// 004ee000 (o CreateForm<FrClubSetWorkshopAboutDlg> 004edf30 vem antes)
FrClubSetWorkshopAboutDlg* __fastcall FrClubSetWorkshopAboutDlg::Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest)
{
	return CreateForm<FrClubSetWorkshopAboutDlg>(pManager, pCmdDest, "clubset_workshop_help", NULL);
}

// 004ee2d0
void FrClubSetWorkshopAboutDlg::SetPage983(int page)
{
	m_page = page;
	if (page == 0)
	{
		m_pBackward->Enable(false);
		m_pForward->Enable(true);
	}
	else if (page == 9)
	{
		m_pForward->Enable(false);
		m_pBackward->Enable(true);
	}
	else
	{
		m_pBackward->Enable(true);
		m_pForward->Enable(true);
	}

	for (int i = 0; i < 10; ++i)
	{
		FrTextButton* pIndex = GetCtrl983<FrTextButton>(MakeStr("index%02d", i + 1));
		if (pIndex)
		{
			if (page == i)
				pIndex->Select(0);
			else
				pIndex->Unselect(0);
		}
	}

	if (m_pViewer)
		m_pViewer->Open(MakeStr("help_clubmade_%02d.jpg", page + 1));
}

// 004ee3d0
bool FrClubSetWorkshopAboutDlg::OnInit()
{
	m_pViewer = GetCtrl983<FrViewer>("about_viewer");
	m_pBackward = GetCtrl983<FrButton>("backward");
	m_pForward = GetCtrl983<FrButton>("forward");
	SetPage983(0);
	return FrForm::OnInit();
}
