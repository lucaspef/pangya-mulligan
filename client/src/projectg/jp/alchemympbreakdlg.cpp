// JP 983: unidade 004f3110–004fb5f0 (quebra de itens em MP do Tiki: FrItemCountController, FrMPBreakAboutDlg,
// FrAlchemyMPBreakNoticeDlg e FrAlchemyMPBreakDlg). Existe no 839, não no 645; ver include/jp/alchemympbreakdlg.h.
// Sessão B94: as três janelas pequenas. Sessão B95: FrAlchemyMPBreakDlg (0x6d0, 004f4a70–004fb5f0).
#include "minatl.h"
#include "vc10_layout.h"
#include <stdlib.h>
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "fredit.h"
#include "frlistbox.h"
#include "commonutil.h"
#include "chatmsg.h"
#include "gamestringmanager.h"
#include "../../shared/itemmanager.h"
#include "frarea.h"
#include "wresrcmng.h"
#include "wview.h"
#include "fxbox.h"
#include "fx.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "titlefontex.h"
#include "soundmanager.h"
#include "alchemympbreakdlg.h"
#include "gaugebar983.h"
#include "cardcomposedlg.h"
#include "avatar_avatar.h"
#include "binstr.h"
#include "createcharacteroption.h"

// ---------------------------------------------------------------------------------------------------------------------
// FrItemCountController

// 004f3110 (GetMessageMap); mapa 00e07ad0 (7 entradas)
BEGIN_FRESH_MSGMAP(FrItemCountController, FrForm)
ON_FRESH_VI("item_name", FRCMD_INIT, FrItemCountController::OnItemNameInit983)
ON_FRESH_VI("dlg_desc", FRCMD_INIT, FrItemCountController::OnDescInit983)
ON_FRESH_VI("item_count", FRCMD_INIT, FrItemCountController::OnCountInit983)
ON_FRESH_VI("increase_cnt", FRCMD_INIT, FrItemCountController::OnIncreaseInit983)
ON_FRESH_VV("increase_cnt", FRCMD_LBUTTONUP, FrItemCountController::OnIncreaseUp983)
ON_FRESH_VI("Decrease_cnt", FRCMD_INIT, FrItemCountController::OnDecreaseInit983)
ON_FRESH_VV("Decrease_cnt", FRCMD_LBUTTONUP, FrItemCountController::OnDecreaseUp983)
END_FRESH_MSGMAP()

IMPLEMENT_OBJECT(FrItemCountController, FrForm)

// 004f3150
int FrItemCountController::GetEditCount983()
{
	return atoi(m_pCount->GetLine(1, false));
}

// 004f3ca0
void FrItemCountController::Increase983()
{
	++m_repeatCount;
	m_count = GetEditCount983() + 1;
	if (m_max == 0)
		return;
	if (m_count > m_max)
		m_count = m_max;
	if (m_count > 999)
		m_count = 999;
	CChatMsg::Instance()->SetChatText(MakeStr("%d", m_count), false);
	m_pCount->ClearLine();
	m_pCount->SetLine(1, MakeStr("%d", m_count), 0, 0, 0, 0);
}

// 004f3d50
void FrItemCountController::Decrease983()
{
	++m_repeatCount;
	m_count = GetEditCount983() - 1;
	if (m_count < 1)
		m_count = 0;
	CChatMsg::Instance()->SetChatText(MakeStr("%d", m_count), false);
	m_pCount->ClearLine();
	m_pCount->SetLine(1, MakeStr("%d", m_count), 0, 0, 0, 0);
}

// 004f3de0
void FrItemCountController::OnItemNameInit983(int param)
{
	m_pItemName = KINDOF_CAST983(FrEdit, param);
}

// 004f3e50
void FrItemCountController::OnDescInit983(int param)
{
	m_pDesc = KINDOF_CAST983(FrEdit, param);
	m_pDesc->ClearLine();
	m_pDesc->SetLine(1, GetGameString(0x1870f), 0, 0, 0, 0);
}

// 004f3ef0
void FrItemCountController::OnCountInit983(int param)
{
	m_pCount = KINDOF_CAST983(FrEdit, param);
	m_pCount->ClearLine();
	m_pCount->SetLine(1, MakeStr("%d", m_count), 0, 0, 0, 0);
}

// 004f3f90
void FrItemCountController::OnIncreaseInit983(int param)
{
	m_pIncrease = KINDOF_CAST983(FrButton, param);
}

// 004f4000
void FrItemCountController::OnIncreaseUp983()
{
	m_pressState = 0;
	m_repeatCount = 0;
	if (m_bRepeated)
		m_bRepeated = false;
	else
		Increase983();
}

// 004f4030
void FrItemCountController::OnDecreaseInit983(int param)
{
	m_pDecrease = KINDOF_CAST983(FrButton, param);
}

// 004f40a0
void FrItemCountController::OnDecreaseUp983()
{
	m_pressState = 0;
	m_repeatCount = 0;
	if (m_bRepeated)
		m_bRepeated = false;
	else
		Decrease983();
}

// 004f64c0
void FrItemCountController::SetItem983(FrListItem* pItem, int max)
{
	m_pItem = pItem;
	m_max = max;
	if (pItem)
	{
		unsigned long typeId = *(unsigned long*)pItem->pData;
		m_pItemName->ClearLine();
		m_pItemName->SetLine(1, ItemManager()->GetItemName(typeId), 0, 0, 0, 0);
	}
}

// 004f6530
void FrItemCountController::OnProc(const float deltaTime)
{
	if (m_pIncrease && m_pIncrease->GetStatus() == 2)
		m_pressState = 1;
	else if (m_pDecrease && m_pDecrease->GetStatus() == 2)
		m_pressState = 2;
	else
		m_pressState = 0;

	switch (m_pressState)
	{
	case 0:
		m_pressTime = 0.0f;
		break;
	case 1:
	case 2:
		m_pressTime += deltaTime;
		break;
	}

	if (m_pressTime != 0.0f)
	{
		float wait = m_pressTime * m_repeatCount;
		wait = 0.5 - wait;
		if (m_pressTime > wait)
		{
			m_bRepeated = true;
			switch (m_pressState)
			{
			case 1:
				Increase983();
				m_pressTime = 0.0f;
				break;
			case 2:
				Decrease983();
				m_pressTime = 0.0f;
				break;
			}
		}
	}

	if (m_pCount)
	{
		m_count = atoi(m_pCount->GetLine(1, false));
		if (m_count > m_max)
		{
			m_pCount->SetLine(1, MakeStr("%d", m_max), 0, 0, 0, 0);
			m_count = m_max;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// FrMPBreakAboutDlg

// 004f31c0 (GetMessageMap); mapa 00e07bd0 (25 entradas)
BEGIN_FRESH_MSGMAP(FrMPBreakAboutDlg, FrForm)
ON_FRESH_VI("aboutviewer", FRCMD_INIT, FrMPBreakAboutDlg::OnViewerInit983)
ON_FRESH_VI("index01", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex01Init983)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex01Up983)
ON_FRESH_VI("index02", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex02Init983)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex02Up983)
ON_FRESH_VI("index03", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex03Init983)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex03Up983)
ON_FRESH_VI("index04", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex04Init983)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex04Up983)
ON_FRESH_VI("index05", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex05Init983)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex05Up983)
ON_FRESH_VI("index06", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex06Init983)
ON_FRESH_VV("index06", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex06Up983)
ON_FRESH_VI("index07", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex07Init983)
ON_FRESH_VV("index07", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex07Up983)
ON_FRESH_VI("index08", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex08Init983)
ON_FRESH_VV("index08", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex08Up983)
ON_FRESH_VI("index09", FRCMD_INIT, FrMPBreakAboutDlg::OnIndex09Init983)
ON_FRESH_VV("index09", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnIndex09Up983)
ON_FRESH_VI("backward", FRCMD_INIT, FrMPBreakAboutDlg::OnBackwardInit983)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnBackwardUp983)
ON_FRESH_VI("forward", FRCMD_INIT, FrMPBreakAboutDlg::OnForwardInit983)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnForwardUp983)
ON_FRESH_VI("ok", FRCMD_INIT, FrMPBreakAboutDlg::OnOkInit983)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrMPBreakAboutDlg::OnOkUp983)
END_FRESH_MSGMAP()

// 004f31d0
FrMPBreakAboutDlg::FrMPBreakAboutDlg()
{
	for (int i = 0; i < PAGE_COUNT; i++)
		m_pIndex[i] = NULL;
	m_page = 0;
	m_pBackward = NULL;
	m_pForward = NULL;
}

IMPLEMENT_OBJECT(FrMPBreakAboutDlg, FrForm)

// 004f3280
void FrMPBreakAboutDlg::SetPage983(int page)
{
	for (int i = 0; i < PAGE_COUNT; i++)
		m_pIndex[i]->Unselect(0);
	if (m_pBackward)
		m_pBackward->Enable(true);
	if (m_pForward)
		m_pForward->Enable(true);

	if (page <= 0)
	{
		page = 0;
		m_pIndex[0]->Select(0);
		if (m_pBackward)
			m_pBackward->Enable(false);
	}
	else if (page >= PAGE_COUNT - 1)
	{
		page = PAGE_COUNT - 1;
		m_pIndex[PAGE_COUNT - 1]->Select(0);
		if (m_pForward)
			m_pForward->Enable(false);
	}
	m_pIndex[page]->Select(0);
	m_page = page;

	if (m_pViewer)
		m_pViewer->Open(MakeStr("help_tiki_recycle_%02d.jpg", page + 1));
}

// 004f4190
void FrMPBreakAboutDlg::OnViewerInit983(int param)
{
	m_pViewer = KINDOF_CAST983(FrViewer, param);
}

// 004f4200
void FrMPBreakAboutDlg::OnIndex01Init983(int param)
{
	m_pIndex[0] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4280
void FrMPBreakAboutDlg::OnIndex02Init983(int param)
{
	m_pIndex[1] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4300
void FrMPBreakAboutDlg::OnIndex03Init983(int param)
{
	m_pIndex[2] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4380
void FrMPBreakAboutDlg::OnIndex04Init983(int param)
{
	m_pIndex[3] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4400
void FrMPBreakAboutDlg::OnIndex05Init983(int param)
{
	m_pIndex[4] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4480
void FrMPBreakAboutDlg::OnIndex06Init983(int param)
{
	m_pIndex[5] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4500
void FrMPBreakAboutDlg::OnIndex07Init983(int param)
{
	m_pIndex[6] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4580
void FrMPBreakAboutDlg::OnIndex08Init983(int param)
{
	m_pIndex[7] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4600
void FrMPBreakAboutDlg::OnIndex09Init983(int param)
{
	m_pIndex[8] = KINDOF_CAST983(FrTextButton, param);
}

// 004f4680
void FrMPBreakAboutDlg::OnBackwardInit983(int param)
{
	m_pBackward = KINDOF_CAST983(FrButton, param);
	m_pBackward->UseDblClick(false);
}

// 004f4700
void FrMPBreakAboutDlg::OnBackwardUp983()
{
	--m_page;
	SetPage983(m_page);
	m_pBackward->SetPushDelay(0.0f);
}

// 004f4740
void FrMPBreakAboutDlg::OnForwardInit983(int param)
{
	m_pForward = KINDOF_CAST983(FrButton, param);
	m_pForward->UseDblClick(false);
}

// 004f47c0
void FrMPBreakAboutDlg::OnForwardUp983()
{
	++m_page;
	SetPage983(m_page);
	m_pForward->SetPushDelay(0.0f);
}

// 004f4800
void FrMPBreakAboutDlg::OnOkInit983(int param)
{
	KINDOF_CAST983(FrButton, param);
}

// 004f4850
bool FrMPBreakAboutDlg::OnInit()
{
	SetPage983(0);
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------
// FrAlchemyMPBreakNoticeDlg

// 004f3350 (GetMessageMap); mapa 00e07f10 (5 entradas)
BEGIN_FRESH_MSGMAP(FrAlchemyMPBreakNoticeDlg, FrForm)
ON_FRESH_VI("FocuseMsg", FRCMD_INIT, FrAlchemyMPBreakNoticeDlg::OnFocusMsgInit983)
ON_FRESH_VI("Msg", FRCMD_INIT, FrAlchemyMPBreakNoticeDlg::OnMsgInit983)
ON_FRESH_VI("CautionMsg", FRCMD_INIT, FrAlchemyMPBreakNoticeDlg::OnCautionMsgInit983)
ON_FRESH_VV("Apply", FRCMD_LBUTTONUP, FrAlchemyMPBreakNoticeDlg::OnApplyUp983)
ON_FRESH_VV("Exit", FRCMD_LBUTTONUP, FrAlchemyMPBreakNoticeDlg::OnExitUp983)
END_FRESH_MSGMAP()

// 004f3360
FrAlchemyMPBreakNoticeDlg::FrAlchemyMPBreakNoticeDlg()
	: m_pFocusMsg(NULL), m_pMsg(NULL), m_pCautionMsg(NULL)
{
}

IMPLEMENT_OBJECT(FrAlchemyMPBreakNoticeDlg, FrForm)

// 004f4920
void FrAlchemyMPBreakNoticeDlg::OnFocusMsgInit983(int param)
{
	m_pFocusMsg = KINDOF_CAST983(FrEdit, param);
}

// 004f4990
void FrAlchemyMPBreakNoticeDlg::OnMsgInit983(int param)
{
	m_pMsg = KINDOF_CAST983(FrEdit, param);
}

// 004f4a00
void FrAlchemyMPBreakNoticeDlg::OnCautionMsgInit983(int param)
{
	m_pCautionMsg = KINDOF_CAST983(FrEdit, param);
}

// ---------------------------------------------------------------------------------------------------------------------
// FrAlchemyMPBreakDlg (sessão B95)

extern Fresh* g_pFresh; // 00f02340
extern const WVector g_vZero983_f041c4; // 00f041c4

// CFxBox::Finished (0096d7f0) só é visível na fxbox.cpp: visão com o nome no jp.tsv
class CFxBoxJp983 : public CFxBox
{
public:
	bool Finished();
};

// 00a19530: membro do CItemManager sem nome no 645 (lê o typeId do IFF; true = o item pode virar MP). Visão com o nome no jp.tsv.
class CItemManagerJp983 : public CItemManager
{
public:
	bool IsMPBreakItem983(unsigned long typeId);
};

// 004fa660 (GetRTTI); mapa 00e07fd0 (39 entradas)
BEGIN_FRESH_MSGMAP(FrAlchemyMPBreakDlg, FrForm)
ON_FRESH_VI("My_TP_Point", FRCMD_INIT, FrAlchemyMPBreakDlg::OnTPPointInit983)
ON_FRESH_VV("My_TP_Point", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnTPPointDraw983)
ON_FRESH_VI("Alchemy_Exit", FRCMD_INIT, FrAlchemyMPBreakDlg::OnExitInit983)
ON_FRESH_VV("Alchemy_Exit", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnExitUp983)
ON_FRESH_VI("Alchemy_Help", FRCMD_INIT, FrAlchemyMPBreakDlg::OnHelpInit983)
ON_FRESH_VV("Alchemy_Help", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnHelpUp983)
ON_FRESH_VI("MagicBox", FRCMD_INIT, FrAlchemyMPBreakDlg::OnMagicBoxInit983)
ON_FRESH_VV("MagicBox", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnMagicBoxUp983)
ON_FRESH_VI("CardCompose", FRCMD_INIT, FrAlchemyMPBreakDlg::OnCardComposeInit983)
ON_FRESH_VV("CardCompose", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnCardComposeUp983)
ON_FRESH_VI("Exchange", FRCMD_INIT, FrAlchemyMPBreakDlg::OnExchangeInit983)
ON_FRESH_VV("Exchange", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnExchangeUp983)
ON_FRESH_VI("Cookie_Item", FRCMD_INIT, FrAlchemyMPBreakDlg::OnCookieInit983)
ON_FRESH_VV("Cookie_Item", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnCookieUp983)
ON_FRESH_VI("Pang_Item", FRCMD_INIT, FrAlchemyMPBreakDlg::OnPangInit983)
ON_FRESH_VV("Pang_Item", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnPangUp983)
ON_FRESH_VI("Rare_Item", FRCMD_INIT, FrAlchemyMPBreakDlg::OnRareInit983)
ON_FRESH_VV("Rare_Item", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnRareUp983)
ON_FRESH_VI("TikiTalk_Edit", FRCMD_INIT, FrAlchemyMPBreakDlg::OnTalkInit983)
ON_FRESH_VI("Exchange_ListBG1", FRCMD_INIT, FrAlchemyMPBreakDlg::OnListBG1Init983)
ON_FRESH_VI("Exchange_ListBG2", FRCMD_INIT, FrAlchemyMPBreakDlg::OnListBG2Init983)
ON_FRESH_VI("Exchange_ListBG3", FRCMD_INIT, FrAlchemyMPBreakDlg::OnListBG3Init983)
ON_FRESH_VI("Exchange_ListBG4", FRCMD_INIT, FrAlchemyMPBreakDlg::OnListBG4Init983)
ON_FRESH_VI("ExchangeOnEffect", FRCMD_INIT, FrAlchemyMPBreakDlg::OnEffectInit983)
ON_FRESH_VV("ExchangeOnEffect", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnEffectDraw983)
ON_FRESH_VI("MPGauge", FRCMD_INIT, FrAlchemyMPBreakDlg::OnMPGaugeInit983)
ON_FRESH_VV("MPGauge", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::DrawGauges983)
ON_FRESH_VI("ExpectMP", FRCMD_INIT, FrAlchemyMPBreakDlg::OnExpectMPInit983)
ON_FRESH_VV("ExpectMP", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnExpectMPDraw983)
ON_FRESH_VI("PrePoint", FRCMD_INIT, FrAlchemyMPBreakDlg::OnPrePointInit983)
ON_FRESH_VV("PrePoint", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnPrePointDraw983)
ON_FRESH_VI("Exchange_ok", FRCMD_INIT, FrAlchemyMPBreakDlg::OnOkInit983)
ON_FRESH_VV("Exchange_ok", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnOkUp983)
ON_FRESH_VI("Own_item", FRCMD_INIT, FrAlchemyMPBreakDlg::OnOwnItemInit983)
ON_FRESH_VI("Own_item", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnOwnItemDraw983)
ON_FRESH_VV("Own_item", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnOwnItemUp983)
ON_FRESH_VI("Exchange_item", FRCMD_INIT, FrAlchemyMPBreakDlg::OnExchangeItemInit983)
ON_FRESH_VI("Exchange_item", FRCMD_OWNERDRAW, FrAlchemyMPBreakDlg::OnExchangeItemDraw983)
ON_FRESH_VV("Exchange_item", FRCMD_LBUTTONUP, FrAlchemyMPBreakDlg::OnExchangeItemUp983)
END_FRESH_MSGMAP()

// 004f4a70
void FrAlchemyMPBreakDlg::InitFx983()
{
	m_fxBox.Init(g_resrcmng);
	m_fxBox.SetCamera(&g_camera);
	m_fxBox.m_frameTime = 33.333332f;
	m_camera = WMatrix::IDENTITY;
	m_camera.pivot = WVector(0.0f, 0.0f, -30.0f);
}

// 004f4b60: as três barras de MP no lugar do "MPGauge"
void FrAlchemyMPBreakDlg::InitGauges983()
{
	float x = m_pMPGauge->GetRect().x;
	float y = m_pMPGauge->GetRect().y;
	int total = m_break.m_mpTotal;
	m_pRespectGauge->Init("gaugebar_nothing.tga", "gaugebar_respect.tga", WRect(x, y, 0.0f, 1000.0f), 0, true, 2.0f, true, true);
	m_pRespectGauge->Reset();
	m_pRespectGauge->SetValue((float)total);
	m_pMileGauge->Init("gaugebar_nothing.tga", "gaugebar_mile.tga", WRect(x, y, 0.0f, 1000.0f), 0, true, 2.0f, true, false);
	m_pMileGauge->Reset();
	m_pMileGauge->SetValue((float)total);
	m_pFullGauge->Init("gaugebar_nothing.tga", "gaugebar_full.tga", WRect(x, y, 0.0f, 1000.0f), 0, false, 2.0f, true, false);
	m_pFullGauge->Reset();
	m_pFullGauge->SetValue(0.0f);
}

// 004f4cc0
void FrAlchemyMPBreakDlg::DrawBitmap983(const Bitmap* pBmp, WRect rc, unsigned long color, bool bFit)
{
	FrGraphicInterface* pGI;
	if (pBmp && (pGI = g_pFresh->GetManager()->GetGDI()) != NULL)
	{
		WRect dst = rc;
		if (bFit)
		{
			dst.w = (float)pBmp->Width();
			dst.h = (float)pBmp->Height();
		}
		pGI->SetTextStyle(0);
		pGI->DrawTexture(pBmp, dst, color, 0);
	}
}

// 004f34a0: a barra "respect" anda até o MP previsto; cheia (1000) acende a "full"
void FrAlchemyMPBreakDlg::ProcGaugeState983(const float deltaTime)
{
	switch (m_gaugeState)
	{
	case 1:
		if (m_break.m_mpTotal < 1000)
		{
			if (!m_pRespectGauge)
				return;
			m_pRespectGauge->SetTarget((float)m_break.m_mpTotal);
		}
		else
		{
			if (!m_pRespectGauge)
				return;
			m_pRespectGauge->SetTarget(1000.0f);
		}
		m_gaugeState = 2;
		break;
	case 2:
		if (m_break.m_mpTotal < 1000)
		{
			if (m_pRespectGauge && m_pRespectGauge->IsDone())
				m_gaugeState = 3;
		}
		else if (m_pRespectGauge && m_pRespectGauge->IsDone() && m_pRespectGauge->IsFull() && m_pFullGauge)
		{
			m_pFullGauge->SetValue(1000.0f);
			m_gaugeState = 3;
		}
		break;
	case 3:
		m_gaugeState = 0;
		break;
	}
}

// 004f35b0
void FrAlchemyMPBreakDlg::ProcGauges983(const float deltaTime)
{
	if (m_pMileGauge)
		m_pMileGauge->Process(deltaTime);
	if (m_pRespectGauge)
		m_pRespectGauge->Process(deltaTime);
	if (m_pFullGauge)
		m_pFullGauge->Process(deltaTime);
}

// 004f3620 ("MPGauge" OWNERDRAW)
void FrAlchemyMPBreakDlg::DrawGauges983()
{
	if (m_pRespectGauge)
		m_pRespectGauge->Draw();
	if (m_pMileGauge)
		m_pMileGauge->Draw();
	if (m_pFullGauge)
		m_pFullGauge->Draw();
}

// 004f4d70
void FrAlchemyMPBreakDlg::OnTPPointInit983(int param)
{
	m_pTPPoint = KINDOF_CAST983(FrArea, param);
	if (m_pTPPoint)
		m_pTPPoint->SetVisible(false);
}

// 004f4df0: quantos itens de MP (0x1a0002a6) o jogador tem
void FrAlchemyMPBreakDlg::OnTPPointDraw983()
{
	if (m_pTPPoint && m_pTPFont)
	{
		FrArea* pArea = m_pTPPoint;
		m_pTPFont->Print(pArea->GetRect().x + 180.0, pArea->GetRect().y + 10.0, MakeStr("%d", m_break.m_mpItems),
			(eTITLEFONTALINE)2, 1.0f, 1.0f);
	}
}

// 004f4e70
void FrAlchemyMPBreakDlg::OnHelpInit983(int param)
{
	m_pHelp = KINDOF_CAST983(FrButton, param);
	if (m_pHelp)
		m_pHelp->SetVisible(true);
	if (m_pHelp)
		m_pHelp->SetPushDelay(0.01f);
}

// 004f4f10
void FrAlchemyMPBreakDlg::OnExitInit983(int param)
{
	m_pExit = KINDOF_CAST983(FrButton, param);
	if (m_pExit)
		m_pExit->SetVisible(true);
	if (m_pExit)
		m_pExit->SetPushDelay(0.01f);
}

// 004f4fb0
void FrAlchemyMPBreakDlg::OnMagicBoxInit983(int param)
{
	m_pMagicBox = KINDOF_CAST983(FrButton, param);
	if (m_pMagicBox)
		m_pMagicBox->SetVisible(true);
	if (m_pMagicBox)
		m_pMagicBox->SetPushDelay(0.01f);
	if (m_pMagicBox)
		m_pMagicBox->EnableHover(true, 0.0f);
}

// 004f5060
void FrAlchemyMPBreakDlg::OnCardComposeInit983(int param)
{
	m_pCardCompose = KINDOF_CAST983(FrButton, param);
	if (m_pCardCompose)
		m_pCardCompose->SetVisible(true);
	if (m_pCardCompose)
		m_pCardCompose->SetPushDelay(0.01f);
	if (m_pCardCompose)
		m_pCardCompose->EnableHover(true, 0.0f);
}

// 004f5110
void FrAlchemyMPBreakDlg::OnExchangeInit983(int param)
{
	m_pExchange = KINDOF_CAST983(FrButton, param);
	if (m_pExchange)
		m_pExchange->SetVisible(true);
	if (m_pExchange)
		m_pExchange->SetPushDelay(0.01f);
	if (m_pExchange)
		m_pExchange->EnableHover(true, 0.0f);
}

// 004f51c0
void FrAlchemyMPBreakDlg::OnCookieInit983(int param)
{
	m_pCookie = KINDOF_CAST983(FrButton, param);
	if (m_pCookie)
		m_pCookie->SetVisible(false);
	if (m_pCookie)
		m_pCookie->SetPushDelay(0.01f);
}

// 004f5260
void FrAlchemyMPBreakDlg::OnPangInit983(int param)
{
	m_pPang = KINDOF_CAST983(FrButton, param);
	if (m_pPang)
		m_pPang->SetVisible(false);
	if (m_pPang)
		m_pPang->SetPushDelay(0.01f);
}

// 004f5300
void FrAlchemyMPBreakDlg::OnRareInit983(int param)
{
	m_pRare = KINDOF_CAST983(FrButton, param);
	if (m_pRare)
		m_pRare->SetVisible(false);
	if (m_pRare)
		m_pRare->SetPushDelay(0.01f);
}

// 004f53a0
void FrAlchemyMPBreakDlg::OnTalkInit983(int param)
{
	m_pTalk = KINDOF_CAST983(FrEdit, param);
}

// 004f5410
void FrAlchemyMPBreakDlg::OnListBG1Init983(int param)
{
	m_pListBG[0] = KINDOF_CAST983(FrArea, param);
	if (m_pListBG[0])
		m_pListBG[0]->SetVisible(false);
}

// 004f5490
void FrAlchemyMPBreakDlg::OnListBG2Init983(int param)
{
	m_pListBG[1] = KINDOF_CAST983(FrArea, param);
	if (m_pListBG[1])
		m_pListBG[1]->SetVisible(false);
}

// 004f5510
void FrAlchemyMPBreakDlg::OnListBG3Init983(int param)
{
	m_pListBG[2] = KINDOF_CAST983(FrArea, param);
	if (m_pListBG[2])
		m_pListBG[2]->SetVisible(false);
}

// 004f5590
void FrAlchemyMPBreakDlg::OnListBG4Init983(int param)
{
	m_pListBG[3] = KINDOF_CAST983(FrArea, param);
	if (m_pListBG[3])
		m_pListBG[3]->SetVisible(false);
}

// 004f5610
void FrAlchemyMPBreakDlg::OnEffectInit983(int param)
{
	m_pEffect = KINDOF_CAST983(FrArea, param);
	m_fxBox.SetScreenCenter(m_pEffect->GetRect().x, m_pEffect->GetRect().y);
}

// 004f56a0: o efeito da quebra com câmera e campo de visão próprios; devolve os da tela
void FrAlchemyMPBreakDlg::OnEffectDraw983()
{
	if (!((CFxBoxJp983*)&m_fxBox)->Finished())
	{
		bool bFog = g_view->SetFogEnable(false);
		g_view->SetCamera(m_camera);
		float oldNear = g_view->clip_near;
		WView::PROJECTION_MODE mode = g_view->GetProjectionMode();
		float oldFar = g_view->clip_far;
		float oldFov = g_view->GetFOV();
		g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
		g_view->SetFOV(1.5707964f);
		g_view->SetClip(0.5f, 50.0f, false);
		g_view->UpdateCamera();
		m_fxBox.Display(g_view);
		g_view->Flush(1, 1);
		g_view->SetProjectionMode(mode);
		g_view->SetClip(oldNear, oldFar, false);
		g_view->SetFOV(oldFov);
		WRect rc;
		rc.x = 0.0f;
		rc.y = 0.0f;
		rc.w = g_view->SCREEN_XS;
		rc.h = g_view->SCREEN_YS;
		g_view->SetClippingArea(rc);
		g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5, g_view->SCREEN_YS * 0.5);
		g_view->UpdateCamera();
		g_view->SetFogEnable(bFog);
	}
}

// 004f5850
void FrAlchemyMPBreakDlg::OnMPGaugeInit983(int param)
{
	m_pMPGauge = KINDOF_CAST983(FrArea, param);
	if (m_pMPGauge)
		m_pMPGauge->SetVisible(false);
}

// 004f58d0
void FrAlchemyMPBreakDlg::OnExpectMPInit983(int param)
{
	m_pExpectMP = KINDOF_CAST983(FrArea, param);
	if (m_pExpectMP)
		m_pExpectMP->SetVisible(false);
}

// 004f5950: MP que a quebra vai dar
void FrAlchemyMPBreakDlg::OnExpectMPDraw983()
{
	if (m_pExpectMP && m_pMPFont && m_pExpectMP->IsVisible())
	{
		m_pMPFont->Print(m_pExpectMP->GetRect().x, m_pExpectMP->GetRect().y, MakeStr("%d~?", m_break.m_mp), (eTITLEFONTALINE)2,
			0.9f, 1.0f);
	}
}

// 004f59c0
void FrAlchemyMPBreakDlg::OnPrePointInit983(int param)
{
	m_pPrePoint = KINDOF_CAST983(FrArea, param);
	if (m_pPrePoint)
		m_pPrePoint->SetVisible(false);
}

// 004f5a40: "n/1000" (e as voltas completas por cima, quando a barra cheia está acesa)
void FrAlchemyMPBreakDlg::OnPrePointDraw983()
{
	if (m_pPrePoint && m_pMPFont && m_pPrePoint->IsVisible())
	{
		int total = m_break.m_mpTotal;
		if (total > 999)
		{
			int full = total / 1000;
			total %= 1000;
			if (m_pFullGauge && m_pFullGauge->m_target != 0.0f)
			{
				FrArea* pArea = m_pPrePoint;
				m_pMPFont->Print(pArea->GetRect().x, pArea->GetRect().y + 16.0, MakeStr("%02d", full), (eTITLEFONTALINE)2, 0.9f, 1.0f);
			}
		}
		FrArea* pArea = m_pPrePoint;
		m_pMPFont->Print(pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d/1000", total), (eTITLEFONTALINE)2, 0.9f, 1.0f);
	}
}

// 004f5b50
void FrAlchemyMPBreakDlg::OnOkInit983(int param)
{
	m_pOk = KINDOF_CAST983(FrButton, param);
	if (m_pOk)
		m_pOk->SetVisible(false);
}

// 004f5bd0
void FrAlchemyMPBreakDlg::OnOwnItemInit983(int param)
{
	m_pOwnList = KINDOF_CAST983(FrListBox, param);
	if (m_pOwnList)
		m_pOwnList->SetVisible(false);
	m_pOwnList->EnableHover(true, 0.1f);
}

// 004f5c60
void FrAlchemyMPBreakDlg::OnExchangeItemInit983(int param)
{
	m_pExchangeList = KINDOF_CAST983(FrListBox, param);
	if (m_pExchangeList)
		m_pExchangeList->SetVisible(false);
}

// 004f6680: resposta do aviso (1 = quebrar mesmo assim)
bool FrAlchemyMPBreakDlg::OnNoticeResult983(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (!m_break.HasFlag())
		{
			m_break.UpdateOver();
			m_break.SendBreak();
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
			m_pNoticeDlg = NULL;
			return true;
		}
		m_break.ResetFlags();
		OpenNotice983();
	}
	m_pNoticeDlg = NULL;
	return true;
}

// 004f6740: aviso com as flags da quebra e o valor total
void FrAlchemyMPBreakDlg::OpenNotice983()
{
	m_pNoticeDlg = CreateForm<FrAlchemyMPBreakNoticeDlg>(g_pFresh->GetManager(), this, "AlchemyMPBreakNoticeDlg", NULL);
	for (int i = 0; i < CAlchemyMPBreak::FLAG_COUNT; ++i)
		m_pNoticeDlg->AddMsg983(m_break.GetFlagText(i));
	m_pNoticeDlg->AddMsg983("\n");
	const char* text;
	if (!m_break.HasFlag())
	{
		m_pNoticeDlg->AddMsg983(GetGameString(0x18704));
		text = MakeStr(GetGameString(0x18710), m_break.GetTotalValue());
	}
	else
		text = GetGameString(0x18709);
	m_pNoticeDlg->AddMsg983(text);
	m_pNoticeDlg->AddFocusMsg983(GetGameString(0x1870a));
	m_pNoticeDlg->Open((FRESH_PFN_RESULT)&FrAlchemyMPBreakDlg::OnNoticeResult983, 0x11);
}

// 004f6850
void FrAlchemyMPBreakDlg::ShowTalk983()
{
	if (m_pTalk)
	{
		m_pTalk->ClearLine();
		m_pTalk->AddText(m_talk[m_talkIndex].c_str(), false, true, 0, 0);
	}
}

// 004f68a0
void FrAlchemyMPBreakDlg::OnHelpUp983()
{
	if (m_pHelp && m_pHelp->IsVisible())
	{
		FrMPBreakAboutDlg* pDlg = CreateForm<FrMPBreakAboutDlg>(g_pFresh->GetManager(), this, "mpbreak_about", NULL);
		if (pDlg)
			pDlg->Open(NULL, 3);
	}
}

// 004f6900: "MagicBox" (mensagem 0x338 ao CTaskMain)
void FrAlchemyMPBreakDlg::OnMagicBoxUp983()
{
	OnFreshCancel();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x338, 0, 0, 0, 0, 0));
}

// 004f6960: "CardCompose" (mensagem 0x324)
void FrAlchemyMPBreakDlg::OnCardComposeUp983()
{
	OnFreshCancel();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x324, 0, 0, 0, 0, 0));
}

// 004f74d0
sMPBreakListItem983* FrAlchemyMPBreakDlg::FindListItem983(unsigned long typeId)
{
	sMPBreakListItem983* pFound = NULL;
	for (std::list<sMPBreakListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
	{
		if (it->typeId == typeId)
		{
			pFound = &*it;
			break;
		}
	}
	return pFound;
}

// 004f7500
int FrAlchemyMPBreakDlg::GetListCount983(FrListItem* pItem)
{
	sMPBreakListItem983* p = FindListItem983(*(unsigned long*)pItem->pData);
	if (!p)
		return 0;
	if (p->guids.begin() == p->guids.end())
		return 0;
	return p->count;
}

// 004f7530: o item (sItemInfo; sCards para cartas) pode entrar na quebra?
bool FrAlchemyMPBreakDlg::CanBreak983(unsigned long typeId, void* pData)
{
	bool ok = false;
	if (!pData)
		return false;
	unsigned long kind = typeId >> 26;
	IFF_ITEM_COMMON* pIff;
	switch (kind)
	{
	case 2:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindPart(typeId);
		break;
	case 4:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(typeId);
		break;
	case 5:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindBall(typeId);
		break;
	case 6:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindItem(typeId);
		break;
	case 0x1c:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindAuxPart(typeId);
		break;
	case 0x1f:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindCard(typeId);
		break;
	default:
		return false;
	}
	if (!pIff || !((CItemManagerJp983*)ItemManager())->IsMPBreakItem983(pIff->TypeId))
		return false;
	switch (kind)
	{
	case 2:
	case 4:
	case 5:
	case 6:
	case 0x1c:
	{
		sItemInfo* pInfo = (sItemInfo*)pData;
		if (!Doc()->IsEquipParts(pInfo->guid, pInfo->typeId))
		{
			ok = true;
			for (std::list<sSCardAvilityPeriodInfo>::iterator it = Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
			{
				if (it->partsUid == pInfo->guid)
				{
					ok = false;
					break;
				}
			}
		}
		if (kind == 4 && Doc()->m_myInfo.userEquip.guidClubSet == pInfo->guid)
			return false;
		if (kind == 6)
		{
			int equipped = 0;
			for (int i = 0; i < 10; ++i)
			{
				if (typeId == Doc()->m_myInfo.userEquip.tidItemSlot[i])
					++equipped;
			}
			if (pInfo->Common[0] == equipped)
				return false;
			return ok;
		}
		if (kind != 5)
			return ok;
		if (pInfo->Common[0] > 0)
		{
			if (Doc()->m_myInfo.userEquip.tidBall == typeId)
				return false;
			return ok;
		}
		return false;
	}
	case 0x1f:
	{
		sCards* pCard = (sCards*)pData;
		if (pCard->partTid == 0 && pCard->partUid == 0)
			ok = true;
		if (pCard->cardCount > 0)
			return ok;
		return false;
	}
	}
	return false;
}

// 004f77b0: quantos deste tipo já estão na quebra
int FrAlchemyMPBreakDlg::GetBreakCount983(unsigned long typeId)
{
	int count = 0;
	sMPBreakListItem983* p = FindListItem983(typeId);
	if (!p || p->guids.begin() == p->guids.end())
		return 0;
	CAlchemyMPBreak::sItem key;
	key.guid = p->guids.begin()->first;
	key.count = 0;
	key.typeId = typeId;
	CAlchemyMPBreak::sItem* pItem = m_break.Find(key);
	if (!pItem)
		return 0;
	if (pItem->count)
		return pItem->count;
	return count;
}

// 004f7820: inventário da aba atual, sem o que já está na quebra
void FrAlchemyMPBreakDlg::SyncOwnList983()
{
	if (m_mode != 3 || !m_pOwnList)
		return;
	for (std::list<sMPBreakListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
	{
		IFF_ITEM_COMMON* pIff = ItemManager()->FindCommonItem(it->typeId);
		if (!pIff)
			continue;
		switch (m_tab)
		{
		case 1:
			if (pIff->TikiType != 1)
				continue;
			break;
		case 2:
			if (pIff->TikiType != 2)
				continue;
			break;
		case 3:
			if (pIff->TikiType != 3)
				continue;
			break;
		}
		int i;
		for (i = 0; i < m_pExchangeList->GetCurrentItemSize(true); ++i)
		{
			FrListItem* pItem = m_pExchangeList->GetItem(i);
			if (pItem && it->typeId == *(unsigned long*)pItem->pData)
			{
				it->count -= GetBreakCount983(*(unsigned long*)pItem->pData);
				break;
			}
		}
		if (it->count != 0)
			m_pOwnList->AddItem(&*it);
	}
}

// 004f7c20: resultado da quebra numa caixa de mensagem (0x23)
void FrAlchemyMPBreakDlg::SendBreak983()
{
	std::string msg;
	if (m_break.m_value30)
		msg = MakeStr(GetGameString(0x1870b), m_break.m_value30, m_break.m_value30 + m_break.m_value2c);
	else
		msg = MakeStr(GetGameString(0x1870d), m_break.m_value2c);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)msg.c_str(), 0, 0, 0, 0));
}

// 004f7d50: efeito no centro da tela ("Item_break_B" quando passa de 1000)
void FrAlchemyMPBreakDlg::PlayFx983()
{
	std::string seq;
	seq.assign("Item_break_N.seq", 0x10);
	if (m_break.m_value30)
		seq.assign("Item_break_B.seq", 0x10);
	CFxSequence* pSeq = m_fxBox.LoadSequence(seq.c_str(), true, NULL);
	if (pSeq)
	{
		pSeq->m_attachX = (float)(int)(g_view->SCREEN_XS * 0.5f);
		pSeq->m_attachY = (float)(int)(g_view->SCREEN_YS * 0.5f);
		pSeq->m_pos = g_vZero983_f041c4;
		m_fxBox.AddSequence(pSeq);
	}
}

// 004f8000: 1 som + efeito, 2 efeito até o fim, 4/5 barras, 6 resultado
void FrAlchemyMPBreakDlg::OnProc(const float deltaTime)
{
	switch (m_state)
	{
	case 1:
		PlayFx983();
		m_state = 2;
		if (m_break.m_bOver)
			g_audio->PlaySfx("item_break_B", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		else
			g_audio->PlaySfx("item_break_N", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		break;
	case 2:
	{
		if (((CFxBoxJp983*)&m_fxBox)->Finished())
			m_state = 3;
		float ms = deltaTime * 1000.0;
		m_fxBox.ProcessAttacher(ms);
		m_fxBox.Process(ms);
		break;
	}
	case 3:
		m_state = 4;
		break;
	case 4:
		if (m_break.m_bOver)
		{
			if (m_pFullGauge)
				m_pFullGauge->SetValue(0.0f);
			if (m_pMileGauge)
				m_pMileGauge->SetTarget(1000.0f);
		}
		else if (m_pMileGauge)
			m_pMileGauge->SetTarget((float)m_break.m_mpTotal);
		m_state = 5;
		break;
	case 5:
		if (m_break.m_bOver)
		{
			CGaugeBar983* pGauge = m_pMileGauge;
			if (pGauge && pGauge->IsDone())
			{
				pGauge->Reset();
				m_pMileGauge->SetValue((float)m_break.m_mpTotal);
				if (m_pRespectGauge)
				{
					m_pRespectGauge->Reset();
					m_pRespectGauge->SetValue((float)m_break.m_mpTotal);
				}
				m_state = 6;
			}
		}
		else if (m_pMileGauge->IsDone())
			m_state = 6;
		break;
	case 6:
		SendBreak983();
		m_state = 0;
		break;
	}
	ProcGaugeState983(deltaTime);
	ProcGauges983(deltaTime);
}

// 004f8210: um texto ao acaso entre os não vazios de [first, last)
int FrAlchemyMPBreakDlg::PickText983(int first, int last)
{
	std::vector<int> ids;
	int id = first;
	for (int i = 0; i < last - first; ++i, ++id)
	{
		std::string text(GetGameString(id));
		if (!text.empty())
			ids.push_back(id);
	}
	int pick = 0;
	if (!ids.empty())
	{
		std::random_shuffle(ids.begin(), ids.end());
		pick = ids[0];
	}
	return pick;
}

// 004f8650: as quatro falas do Tiki
void FrAlchemyMPBreakDlg::SetTalks983()
{
	m_talk[0] = GetGameString(PickText983(0x186a1, 0x186aa));
	m_talk[1] = GetGameString(PickText983(0x186ab, 0x186b4));
	m_talk[2] = GetGameString(PickText983(0x186b5, 0x186be));
	m_talk[3] = GetGameString(PickText983(0x186bf, 0x186c8));
}

// 004f8730
void FrAlchemyMPBreakDlg::SetTalk983(int index)
{
	m_talkIndex = index;
	SetTalks983();
	ShowTalk983();
}

// 004f89e0 (msg 0x2e4 do CTaskMain)
void FrAlchemyMPBreakDlg::SetMPValues983(int a, int b)
{
	m_break.SetValues(a, b);
	m_break.UpdateMP();
	if (m_break.m_bOver)
		SetTalk983(3);
	else
		SetTalk983(2);
}

// constantes da unidade (00d05ac0–00d05ad8, fora do bloco __real@)
static const double ITEM_ICON_OFFSET_Y983 = 4.5;
static const double ITEM_ICON_SCALE983 = 1.4f; // figura = altura da linha / 1,4
static const double ITEM_NAME_OFFSET_X983 = 1.6f;
static const double ITEM_TEXT_OFFSET_Y983 = 0.35f;

// 004f8b10: uma linha das listas (OWNERDRAW): figura do item (peça: antes o rosto do personagem; taco/bola com "bar_"),
// nome cortado em maxLen ("..."), "x<quantidade>" (só na aba de quebra, m_mode 3) e o MP do item ("<mp>M") na direita
void FrAlchemyMPBreakDlg::DrawListItem983(FrListItem* pItem, float h, int maxLen, int count)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pItem || !pGI)
		return;
	pGI->SetTextColor(0xff000000, 0xffffffff);
	pGI->SetTextStyle(0);
	unsigned long* pTypeId = (unsigned long*)pItem->pData;
	std::string icon("");
	std::string name("");
	int charIndex = 0;
	int mp = 0;
	int shown = 0;
	IFF_ITEM_COMMON* pIff;
	switch (*pTypeId >> 26)
	{
	case 2:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindPart(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			charIndex = (*pTypeId >> 18) & 0xff;
			mp = pIff->Mileage;
		}
		break;
	case 4:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindClubSet(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			mp = pIff->Mileage;
		}
		if (!icon.empty())
			icon = std::string("bar_") + icon;
		break;
	case 5:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindBall(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			mp = pIff->Mileage;
		}
		if (!icon.empty())
			icon = std::string("bar_") + icon;
		break;
	case 6:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindItem(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			mp = pIff->Mileage;
		}
		break;
	case 0x1f:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindCard(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			mp = pIff->Mileage;
		}
		break;
	case 0x1c:
		pIff = (IFF_ITEM_COMMON*)ItemManager()->FindAuxPart(*pTypeId);
		if (pIff)
		{
			icon = pIff->Icon;
			name = pIff->Name;
			mp = pIff->Mileage;
		}
		break;
	}
	if (m_mode == 3)
		shown = count;
	WRect rc;
	rc.x = pItem->pos.x + 5.0;
	rc.y = pItem->pos.y + ITEM_ICON_OFFSET_Y983;
	rc.w = 0.0f;
	rc.h = 0.0f;
	if ((*pTypeId & 0xfc000000) == 0x08000000)
	{
		std::string face("");
		face = CreateCharacterOption::Instance()->GetBarIcon983(charIndex);
		if (!face.empty())
		{
			const Bitmap* pBmp = g_pFresh->GetBitmap(face.c_str());
			if (pBmp)
			{
				float scale = h / (pBmp->Height() * ITEM_ICON_SCALE983);
				scale = scale < 1.0f ? scale : 1.0f;
				rc.w = pBmp->Width() * scale;
				rc.h = pBmp->Height() * scale;
				pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
			}
		}
	}
	if (!icon.empty())
	{
		const Bitmap* pBmp = g_pFresh->GetBitmap(icon.c_str());
		if (pBmp)
		{
			float scale = h / (pBmp->Height() * ITEM_ICON_SCALE983);
			scale += (*pTypeId & 0xfc000000) == 0x18000000 ? scale * 0.5 : 0.0;
			scale = scale < 1.0f ? scale : 1.0f;
			rc.x = scale + scale + rc.w + rc.x;
			rc.w = pBmp->Width() * scale;
			rc.h = pBmp->Height() * scale;
			pGI->DrawTexture(pBmp, rc, 0xffffffff, 0);
		}
	}
	if (!name.empty())
	{
		if (name.length() > (unsigned)maxLen)
		{
			char buf[32] = {0};
			mbscut(buf, name.c_str(), maxLen - 1);
			name = buf;
			name += "...";
		}
		float nameX = h * ITEM_NAME_OFFSET_X983;
		rc.x = nameX + pItem->pos.x; // o "x<quantidade>" abaixo parte daqui
		pGI->Print11(WPoint(rc.x, (h - 11.0) * ITEM_TEXT_OFFSET_Y983 + rc.y), 0, name.c_str());
	}
	float listW = 0.0f;
	if (m_pOwnList)
		listW = (float)m_pOwnList->GetItemWidth();
	std::string countText = MakeStr("x%d", shown);
	std::string mpText = MakeStr("%d", mp);
	std::string unitText = "M";
	pGI->GetTextExtendOld983(countText.c_str());
	float mpW = pGI->GetTextExtendOld983(mpText.c_str());
	float unitW = pGI->GetTextExtendOld983(unitText.c_str());
	WPoint unitPt(pItem->pos.x + listW, (h - 11.0) * ITEM_TEXT_OFFSET_Y983 + rc.y);
	WPoint countPt(listW * 0.5 + rc.x, unitPt.y);
	WPoint mpPt(unitPt.x - (unitW + mpW), unitPt.y);
	pGI->SetTextColor(0xff871ba4, 0xffffffff);
	pGI->Print(countPt, 2, MakeStr("x%d", shown));
	pGI->SetTextStyle(1);
	pGI->SetTextColor(0xffd31b9f, 0xffffffff);
	pGI->Print(mpPt, 0, mpText.c_str());
	pGI->SetTextColor(0xff871ba4, 0xffffffff);
	pGI->Print(unitPt, 2, unitText.c_str());
}

// 004f9500 ("Own_item" OWNERDRAW)
void FrAlchemyMPBreakDlg::OnOwnItemDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	float w = (float)m_pOwnList->GetItemWidth();
	float h = (float)m_pOwnList->GetItemHeight();
	if (m_pOwnList->GetItemUnderCursor() == pItem)
		DrawBitmap983(m_pListOverL, WRect(pItem->pos.x + 2.0, pItem->pos.y + 2.0, w, h), 0xffffffff, true);
	sMPBreakListItem983* p = FindListItem983(*(unsigned long*)pItem->pData);
	if (p)
		DrawListItem983(pItem, h + 1.0, 0xe, p->count);
}

// 004f95c0 ("Exchange_item" OWNERDRAW)
void FrAlchemyMPBreakDlg::OnExchangeItemDraw983(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	float w = (float)m_pExchangeList->GetItemWidth();
	float h = (float)m_pExchangeList->GetItemHeight();
	if (m_pExchangeList->GetItemUnderCursor() == pItem)
		DrawBitmap983(m_pListOverR, WRect(pItem->pos.x + 2.0, pItem->pos.y + 2.0, w, h), 0xffffffff, true);
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sMPBreakListItem983* p = FindListItem983(typeId);
	if (p && p->guids.begin() != p->guids.end())
	{
		CAlchemyMPBreak::sItem key;
		key.guid = p->guids.begin()->first;
		key.typeId = typeId;
		CAlchemyMPBreak::sItem* pBreak = m_break.Find(key);
		if (pBreak)
			DrawListItem983(pItem, h + 1.0, 0x10, pBreak->count);
	}
}

// 004f99b0: "Exchange_ok" só com algo escolhido
void FrAlchemyMPBreakDlg::UpdateOk983()
{
	if (m_pOk && m_mode == 3)
	{
		CAlchemyMPBreak::ItemMap items = m_break.GetItems();
		m_pOk->Enable(items.size() == 0 ? false : true);
	}
}

// 004f9a50: "Exchange_ok"
void FrAlchemyMPBreakDlg::OnOkUp983()
{
	CAlchemyMPBreak::ItemMap items = m_break.GetItems();
	if (items.size() == 0)
		return;
	sItemInfo* pInfo = Doc()->FindMyItemByTypeId983(0x1a0002a6);
	if (pInfo && (unsigned)pInfo->Common[0] > 19999)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)GetGameString(0x1870e), 0, 0, 0, 0));
		return;
	}
	m_break.UpdateFlags();
	OpenNotice983();
}

// 004f9ba0: tira da quebra o item escolhido
void FrAlchemyMPBreakDlg::OnExchangeItemUp983()
{
	FrListItem* pItem = m_pExchangeList->GetItemUnderCursor();
	if (!pItem)
		return;
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sMPBreakListItem983* p = FindListItem983(typeId);
	if (!p || p->guids.begin() == p->guids.end())
		return;
	CAlchemyMPBreak::sItem key;
	key.guid = p->guids.begin()->first;
	key.typeId = typeId;
	int removed = 0;
	if (m_break.Remove(key, &removed) == 0)
		m_pExchangeList->DelItem(pItem->pData);
	p->count += removed;
	m_break.UpdateMP();
	if (m_break.m_mpTotal < 1000 && m_pFullGauge)
		m_pFullGauge->SetValue(0.0f);
	if (m_pRespectGauge)
		m_pRespectGauge->SetValue((float)m_break.m_mpTotal);
	if (!m_pOwnList->FindItem(p) && m_tab == m_break.GetBreakKind(p->typeId))
		m_pOwnList->AddItem(p);
	UpdateOk983();
}

// 004f9d50: resposta da "MPBreakCountSetDlg"
bool FrAlchemyMPBreakDlg::OnCountResult983(int result, FrForm* pForm)
{
	if (!m_pCountDlg)
		return true;
	if (result == 1)
	{
		if (m_pCountDlg->GetCount983() < 1)
		{
			m_pCountDlg = NULL;
			return true;
		}
		FrListItem* pItem = m_pCountDlg->GetItem983();
		unsigned long typeId = *(unsigned long*)pItem->pData;
		sMPBreakListItem983* p = FindListItem983(typeId);
		if (!p || p->guids.begin() == p->guids.end())
		{
			m_pCountDlg = NULL;
			return true;
		}
		CAlchemyMPBreak::sItem key;
		key.guid = p->guids.begin()->first;
		key.typeId = typeId;
		key.count = 0;
		CAlchemyMPBreak::sItem* pBreak = m_break.Add(key);
		if (pBreak)
		{
			pBreak->count += m_pCountDlg->GetCount983();
			int n = m_pCountDlg->GetCount983();
			p->count -= n;
			if (p->count == 0)
				m_pOwnList->DelItem(pItem->pData);
			if (m_pRespectGauge)
			{
				m_break.UpdateMP();
				m_pRespectGauge->SetTarget((float)m_break.m_mpTotal);
				if (m_break.m_mpTotal > 999 && m_pFullGauge)
					m_pFullGauge->SetValue(1000.0f);
			}
			SetTalk983(1);
			if (!m_pExchangeList->FindItem(&pBreak->typeId))
				m_pExchangeList->AddItem(&pBreak->typeId);
		}
	}
	UpdateOk983();
	m_pCountDlg = NULL;
	return true;
}

// 004f9ed0
void FrAlchemyMPBreakDlg::OpenCountDlg983(FrListItem* pItem)
{
	if (!m_pCountDlg)
		m_pCountDlg = CreateForm<FrItemCountController>(g_pFresh->GetManager(), this, "MPBreakCountSetDlg", NULL);
	if (m_pCountDlg)
	{
		m_pCountDlg->SetItem983(pItem, GetListCount983(pItem));
		m_pCountDlg->Open((FRESH_PFN_RESULT)&FrAlchemyMPBreakDlg::OnCountResult983, 3);
	}
}

// 004f9f60: item do inventário para a quebra (empilháveis com mais de 4 abrem a escolha da quantidade)
void FrAlchemyMPBreakDlg::OnOwnItemUp983()
{
	FrListItem* pItem = m_pOwnList->GetItemUnderCursor();
	if (!pItem)
		return;
	unsigned long typeId = *(unsigned long*)pItem->pData;
	sMPBreakListItem983* p = FindListItem983(typeId);
	if (!p || p->guids.begin() == p->guids.end())
		return;
	if ((unsigned)p->count > 4)
	{
		OpenCountDlg983(pItem);
		return;
	}
	CAlchemyMPBreak::sItem key;
	key.guid = p->guids.begin()->first;
	key.typeId = typeId;
	key.count = 0;
	CAlchemyMPBreak::sItem* pBreak = m_break.Add(key);
	if (pBreak)
	{
		++pBreak->count;
		m_break.UpdateMP();
		if (--p->count == 0)
			m_pOwnList->DelItem(pItem->pData);
		m_gaugeState = 1;
		if (!m_pExchangeList->FindItem(&pBreak->typeId))
			m_pExchangeList->AddItem(&pBreak->typeId);
		m_bChanged = 1;
		UpdateOk983();
		SetTalk983(1);
	}
}

// 004fa3d0
FrAlchemyMPBreakDlg::FrAlchemyMPBreakDlg()
{
	m_pHelp = NULL;
	m_pExit = NULL;
	m_pExchange = NULL;
	m_pListBG[0] = NULL;
	m_pListBG[1] = NULL;
	m_pListBG[2] = NULL;
	m_pListBG[3] = NULL;
	m_pTPPoint = NULL;
	m_pExpectMP = NULL;
	m_pPrePoint = NULL;
	m_pOk = NULL;
	m_pOwnList = NULL;
	m_pExchangeList = NULL;
	m_pListOverL = NULL;
	m_pListOverR = NULL;
	m_pCountDlg = NULL;
	m_pCookie = NULL;
	m_pPang = NULL;
	m_pRare = NULL;
	m_tab = 1;
	m_pMPGauge = NULL;
	m_pMileGauge = NULL;
	m_pMileGauge = new CGaugeBar983;
	m_pRespectGauge = NULL;
	m_pRespectGauge = new CGaugeBar983;
	m_pFullGauge = NULL;
	m_pFullGauge = new CGaugeBar983;
	m_pNoticeDlg = NULL;
	m_pTPFont = NULL;
	m_pTPFont = new CTPItemCountFont;
	m_pMPFont = NULL;
	m_pMPFont = new CMPItemCountFont;
	m_pTalk = NULL;
	m_pEffect = NULL;
	m_bChanged = 0;
	m_state = 0;
	m_gaugeState = 0;
	InitFx983();
}

IMPLEMENT_OBJECT(FrAlchemyMPBreakDlg, FrForm)

// 004fa690
FrAlchemyMPBreakDlg::~FrAlchemyMPBreakDlg()
{
	m_pListOverL = NULL;
	m_pListOverR = NULL;
	for (std::list<sMPBreakListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
		it->guids.clear();
	m_list.clear();
	if (m_pTPFont)
	{
		delete m_pTPFont;
		m_pTPFont = NULL;
	}
	if (m_pMPFont)
	{
		delete m_pMPFont;
		m_pMPFont = NULL;
	}
	m_pCountDlg = NULL;
	m_pCookie = NULL;
	m_pPang = NULL;
	m_pRare = NULL;
	m_pMPGauge = NULL;
	if (m_pMileGauge)
	{
		delete m_pMileGauge;
		m_pMileGauge = NULL;
	}
	if (m_pRespectGauge)
	{
		delete m_pRespectGauge;
		m_pRespectGauge = NULL;
	}
	if (m_pFullGauge)
	{
		delete m_pFullGauge;
		m_pFullGauge = NULL;
	}
	m_fxBox.ClearAll(false);
}

// 004fab30: soma no item do tipo (ou cria um novo, no começo ou no fim da lista); guarda o guid com a quantidade
int FrAlchemyMPBreakDlg::AddListItem983(unsigned long typeId, unsigned long guid, int count, bool bFront)
{
	sMPBreakListItem983* p = FindListItem983(typeId);
	if (!p)
	{
		sMPBreakListItem983 item;
		item.typeId = typeId;
		item.count = count;
		if (guid)
			item.guids[guid] = count;
		if (bFront)
			m_list.push_front(item);
		else
			m_list.push_back(item);
		return count;
	}
	p->count += count;
	if (guid)
		p->guids[guid] = count;
	return p->count;
}

// 004faca0: refaz a lista com o inventário que pode ser quebrado
void FrAlchemyMPBreakDlg::RefreshList983()
{
	for (std::list<sMPBreakListItem983>::iterator lt = m_list.begin(); lt != m_list.end(); ++lt)
		lt->guids.clear();
	m_list.clear();
	if (m_mode != 3)
		return;
	if (!OnlinePlay())
	{
		// fora de linha: itens de teste
		unsigned long ids[20] = {
			0x18000006, 0x18000009, 0x1800000a, 0x1800000b, 0x1800000e, 0x18000010, 0x18000011, 0x18000027, 0x18000028, 0x1800002e,
			0x1a000000, 0x1a000002, 0x1a00000a, 0x1a00000b, 0x1a00000d, 0x1a00000e, 0x1a000011, 0x1a000040, 0x1a00004f, 0x1a0000c7 };
		for (unsigned int i = 0; i < 20; ++i)
		{
			unsigned long id = ids[i];
			if (CanBreak983(id, NULL))
				AddListItem983(id, 1, 50, false);
		}
		return;
	}
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_partsList.begin(); it != Doc()->m_partsList.end(); ++it)
	{
		sItemInfo* pInfo = &*it;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
			AddListItem983(pInfo->typeId, pInfo->guid, 1, false);
	}
	for (it = Doc()->m_auxPartList.begin(); it != Doc()->m_auxPartList.end(); ++it)
	{
		sItemInfo* pInfo = &*it;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
			AddListItem983(pInfo->typeId, pInfo->guid, pInfo->Common[0], false);
	}
	for (it = Doc()->m_uccItemList.begin(); it != Doc()->m_uccItemList.end(); ++it)
	{
		sItemInfo* pInfo = &*it;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
			AddListItem983(pInfo->typeId, pInfo->guid, 1, false);
	}
	for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end(); ++it)
	{
		sItemInfo* pInfo = &*it;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
			AddListItem983(pInfo->typeId, pInfo->guid, pInfo->Common[0], false);
	}
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); ++it)
	{
		sItemInfo* pInfo = &*it;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
		{
			unsigned long typeId = pInfo->typeId;
			int count = pInfo->Common[0];
			if ((typeId & 0x2000000) == 0)
			{
				int equipped = 0;
				for (int i = 0; i < 10; ++i)
				{
					if (typeId == Doc()->m_myInfo.userEquip.tidItemSlot[i])
						++equipped;
				}
				count -= equipped;
			}
			AddListItem983(typeId, pInfo->guid, count, false);
		}
	}
	for (std::map<unsigned int, sItemInfo>::iterator mt = Doc()->m_clubSetMap.begin(); mt != Doc()->m_clubSetMap.end(); ++mt)
	{
		sItemInfo* pInfo = &mt->second;
		if (pInfo->typeId && pInfo->ItemType == 0 && CanBreak983(pInfo->typeId, pInfo))
			AddListItem983(pInfo->typeId, pInfo->guid, 1, false);
	}
	for (std::list<sCards>::iterator ct = Doc()->m_cardList.begin(); ct != Doc()->m_cardList.end(); ++ct)
	{
		sCards* pCard = &*ct;
		if (pCard->tid && CanBreak983(pCard->tid, pCard))
			AddListItem983(pCard->tid, pCard->uid, pCard->cardCount, false);
	}
}

// 004fb1a0: aba "Cookie_Item"
void FrAlchemyMPBreakDlg::OnCookieUp983()
{
	m_tab = 2;
	if (m_pCookie)
		m_pCookie->SetStatus((FrButton::eButMode)2);
	if (m_pPang)
		m_pPang->SetStatus((FrButton::eButMode)0);
	if (m_pRare)
		m_pRare->SetStatus((FrButton::eButMode)0);
	m_pOwnList->ClearItem();
	RefreshList983();
	SyncOwnList983();
}

// 004fb200: aba "Pang_Item"
void FrAlchemyMPBreakDlg::OnPangUp983()
{
	m_tab = 1;
	if (m_pCookie)
		m_pCookie->SetStatus((FrButton::eButMode)0);
	if (m_pPang)
		m_pPang->SetStatus((FrButton::eButMode)2);
	if (m_pRare)
		m_pRare->SetStatus((FrButton::eButMode)0);
	m_pOwnList->ClearItem();
	RefreshList983();
	SyncOwnList983();
}

// 004fb260: aba "Rare_Item"
void FrAlchemyMPBreakDlg::OnRareUp983()
{
	m_tab = 3;
	if (m_pCookie)
		m_pCookie->SetStatus((FrButton::eButMode)0);
	if (m_pPang)
		m_pPang->SetStatus((FrButton::eButMode)0);
	if (m_pRare)
		m_pRare->SetStatus((FrButton::eButMode)2);
	m_pOwnList->ClearItem();
	RefreshList983();
	SyncOwnList983();
}

// 004fb2c0: só o modo 3 (itens) mostra as listas, abas e barras
void FrAlchemyMPBreakDlg::SetMode983(int mode)
{
	m_mode = mode;
	m_break.SetMode(mode);
	bool bItems = false;
	if (mode == 3)
		bItems = true;
	if (m_pMagicBox)
		m_pMagicBox->SetVisible(true);
	if (m_pCardCompose)
		m_pCardCompose->SetVisible(true);
	if (m_pExchange)
		m_pExchange->SetVisible(true);
	if (m_pTPPoint)
		m_pTPPoint->SetVisible(bItems);
	if (m_pCookie)
		m_pCookie->SetVisible(bItems);
	if (m_pPang)
		m_pPang->SetVisible(bItems);
	if (m_pRare)
		m_pRare->SetVisible(bItems);
	if (m_pListBG[0])
		m_pListBG[0]->SetVisible(bItems);
	if (m_pListBG[1])
		m_pListBG[1]->SetVisible(bItems);
	if (m_pListBG[2])
		m_pListBG[2]->SetVisible(bItems);
	if (m_pListBG[3])
		m_pListBG[3]->SetVisible(bItems);
	if (m_pMPGauge)
		m_pMPGauge->SetVisible(bItems);
	if (m_pExpectMP)
		m_pExpectMP->SetVisible(bItems);
	if (m_pPrePoint)
		m_pPrePoint->SetVisible(bItems);
	if (m_pOk)
		m_pOk->SetVisible(bItems);
	if (m_pOwnList)
		m_pOwnList->SetVisible(bItems);
	if (m_pOwnList)
		m_pOwnList->ClearItem();
	if (m_pExchangeList)
		m_pExchangeList->SetVisible(bItems);
	if (m_pExchangeList)
		m_pExchangeList->ClearItem();
	RefreshList983();
	SyncOwnList983();
}

// 004fb460 (chamada em 00651086)
void FrAlchemyMPBreakDlg::Reset983()
{
	SetMode983(m_mode);
	m_break.Clear();
	UpdateOk983();
}

// 004fb490
bool FrAlchemyMPBreakDlg::OnInit()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x104, 0, 0, 0, 0, 0));
	EnableDrag(false);
	m_mode = 3;
	m_break.SetMode(3);
	m_pListOverL = g_pFresh->RegisterBitmap("list_button_o_left");
	m_pListOverR = g_pFresh->RegisterBitmap("list_button_o_right");
	m_pExchange->SetStatus((FrButton::eButMode)2);
	FrButton* pTab;
	switch (m_tab)
	{
	case 1:
		pTab = m_pPang;
		break;
	case 2:
		pTab = m_pCookie;
		break;
	case 3:
		pTab = m_pRare;
		break;
	default:
		goto done;
	}
	if (pTab)
		pTab->SetStatus((FrButton::eButMode)2);
done:
	m_break.UpdateMP();
	InitGauges983();
	SetMode983(m_mode);
	SetTalks983();
	SetTalk983(0);
	UpdateOk983();
	// A299: 00e73e80 = ntAvatarManager::m_pInstance (g_pAvatarMotion983 era só declaração = nulo no exe; estado em +0xf0 no nosso)
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pAvatar)
			pAvatar->SetState(pAvatar->m_state983 | 0x20);
		ntAvatarManager::m_pInstance->m_bActive = false;
	}
	return true;
}
