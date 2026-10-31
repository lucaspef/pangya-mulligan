// JP 983: unidade 0082e940–0082fcd0 (_gateway::FrWorkShopEventDlg, a janela do evento da oficina de tacos de 2013; ver
// include/jp/workshopeventdlg.h). Classe nova do 983 (existe no 839, não no 645). Sessão B126. O CreateForm<FrBirthdayEventHelpDlg>
// 0082f960 o compilador gera sozinho.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "commonutil.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "titlefontex.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "birthdayeventdlg.h"
#include "gaugebar983.h"
#include "workshopeventdlg.h"

extern Fresh* g_pFresh; // 00f02340
extern WView* g_pMainView983; // 00f02458
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c (retângulo de origem padrão)

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// estilo do FrWnd (protegido) visto de fora
class FrWndStyleJp983 : public FrWnd
{
public:
	void AddStyle983(unsigned long style) { m_dwStyle.Enable(style); }
};

namespace _gateway
{
// 0082e940..0082e970: setters sem chamador no 983 (o começo da unidade)
// 0082e940
void FrWorkShopEventDlg::SetMyHoles983(int count)
{
	m_myHoles = count;
}

// 0082e950
void FrWorkShopEventDlg::SetOpenedBoxes983(int count)
{
	m_openedBoxes = count;
}

// 0082e960
void FrWorkShopEventDlg::SetTotalHoles983(int count)
{
	m_totalHoles = count;
}

// 0082e970
void FrWorkShopEventDlg::SetTotalGoal983(int count)
{
	m_totalGoal = count;
}

// 0082e980 (GetMessageMap); mapa 00e205c8 (26 entradas)
BEGIN_FRESH_MSGMAP(FrWorkShopEventDlg, FrForm)
ON_FRESH_VI("WorkShopEvent_bg1", FRCMD_INIT, FrWorkShopEventDlg::OnBg1Init)
ON_FRESH_VI("WorkShopEvent_bg2", FRCMD_INIT, FrWorkShopEventDlg::OnBg2Init)
ON_FRESH_VI("WorkShopEvent_bg3", FRCMD_INIT, FrWorkShopEventDlg::OnBg3Init)
ON_FRESH_VI("WorkShopEvent_bg4", FRCMD_INIT, FrWorkShopEventDlg::OnBg4Init)
ON_FRESH_VI("WorkShopEvent_bg5", FRCMD_INIT, FrWorkShopEventDlg::OnBg5Init)
ON_FRESH_VI("WorkShopEvent_bg6", FRCMD_INIT, FrWorkShopEventDlg::OnBg6Init)
ON_FRESH_VI("WorkShopEvent_bg7", FRCMD_INIT, FrWorkShopEventDlg::OnBg7Init)
ON_FRESH_VI("WorkShopEvent_bg8", FRCMD_INIT, FrWorkShopEventDlg::OnBg8Init)
ON_FRESH_VI("WorkShopEvent_bg9", FRCMD_INIT, FrWorkShopEventDlg::OnBg9Init)
ON_FRESH_VI("WorkShopEvent_bg10", FRCMD_INIT, FrWorkShopEventDlg::OnBg10Init)
ON_FRESH_VI("WorkShopEvent_bg11", FRCMD_INIT, FrWorkShopEventDlg::OnBg11Init)
ON_FRESH_VI("WorkShopEvent_bg12", FRCMD_INIT, FrWorkShopEventDlg::OnBg12Init)
ON_FRESH_VI("workshop_random_box_btn", FRCMD_INIT, FrWorkShopEventDlg::OnRandomBoxBtnInit)
ON_FRESH_VV("workshop_random_box_btn", FRCMD_LBUTTONUP, FrWorkShopEventDlg::OnRandomBoxBtnClick)
ON_FRESH_VI("myhole_count_bar_bg_area", FRCMD_INIT, FrWorkShopEventDlg::OnMyBarInit)
ON_FRESH_VI("myhole_count_bar_bg_area", FRCMD_OWNERDRAW, FrWorkShopEventDlg::OnMyBarDraw)
ON_FRESH_VI("total_hole_count_bar_bg_area", FRCMD_INIT, FrWorkShopEventDlg::OnTotalBarInit)
ON_FRESH_VI("total_hole_count_bar_bg_area", FRCMD_OWNERDRAW, FrWorkShopEventDlg::OnTotalBarDraw)
ON_FRESH_VI("workshop_eye_blink", FRCMD_INIT, FrWorkShopEventDlg::OnEyeBlinkInit)
ON_FRESH_VI("randombox_reward_num_area", FRCMD_INIT, FrWorkShopEventDlg::OnBoxCountInit)
ON_FRESH_VI("randombox_reward_num_area", FRCMD_OWNERDRAW, FrWorkShopEventDlg::OnBoxCountDraw)
ON_FRESH_VI("myhole_count_area", FRCMD_INIT, FrWorkShopEventDlg::OnMyHoleCountInit)
ON_FRESH_VI("myhole_count_area", FRCMD_OWNERDRAW, FrWorkShopEventDlg::OnMyHoleCountDraw)
ON_FRESH_VI("workshop_holecount_text_area", FRCMD_INIT, FrWorkShopEventDlg::OnHoleCountTextInit)
ON_FRESH_VI("help", FRCMD_INIT, FrWorkShopEventDlg::OnHelpInit)
ON_FRESH_VV("help", FRCMD_LBUTTONUP, FrWorkShopEventDlg::OnHelpClick)
END_FRESH_MSGMAP()

// 0082e990
FrWorkShopEventDlg::FrWorkShopEventDlg()
	: m_eyeSpeed(2.0f), m_myHoles(0), m_totalHoles(0), m_eyeTime(0), m_totalGoal(0), m_openedBoxes(0), m_boxCount(0),
	  m_eyeFrame(0), m_bGoalDone(false), m_pRandomBoxBtn(NULL), m_pMyGauge(NULL), m_pTotalGauge(NULL), m_pCompleteOverlay(NULL),
	  m_pEyeBlink(NULL), m_pHoleCountText(NULL), m_pBoxCountArea(NULL), m_pMyHoleCountArea(NULL), m_pFont(NULL)
{
	for (int i = 0; i < BG_COUNT; ++i)
		m_pBg[i] = NULL;
}

// 0082ea80
void FrWorkShopEventDlg::UpdateEyeBlink983(float deltaTime)
{
	if (m_pEyeBlink == NULL)
		return;
	m_eyeTime = m_eyeSpeed * deltaTime + m_eyeTime;
	if (m_eyeTime > (m_eyeFrame == 0 ? 5.0f : 0.2f))
	{
		m_eyeTime = 0.0f;
		m_eyeFrame = (m_eyeFrame + 1) % 3;
		if (m_eyeFrame == 0)
		{
			m_pEyeBlink->SetVisible(false);
			return;
		}
		m_pEyeBlink->SetVisible(true);
		m_pEyeBlink->SetBgImg(MakeStr("event_club_eye%d", m_eyeFrame));
	}
}

// 0082eb30
void FrWorkShopEventDlg::ShowGoalBg983()
{
	for (int i = 0; i < BG_COUNT; ++i)
	{
		if (m_pBg[i])
			m_pBg[i]->SetBgImg(MakeStr("event_club_b%02d", i + 1));
	}
	if (m_pHoleCountText)
		m_pHoleCountText->SetVisible(false);
}

// 0082eb90
void FrWorkShopEventDlg::CalcBoxCount983()
{
	m_boxCount = (m_myHoles - m_openedBoxes * HOLES_PER_BOX) / HOLES_PER_BOX;
}

// 0082ebd0
void FrWorkShopEventDlg::OnMyBarDraw(int param)
{
	if (m_pMyGauge)
		m_pMyGauge->Draw();
}

// 0082ebf0
void FrWorkShopEventDlg::OnTotalBarInit(int param)
{
}

IMPLEMENT_OBJECT(FrWorkShopEventDlg, FrForm)

// 0082ecb0
FrWorkShopEventDlg::~FrWorkShopEventDlg()
{
	if (m_pMyGauge)
	{
		delete m_pMyGauge;
		m_pMyGauge = NULL;
	}
	if (m_pTotalGauge)
	{
		delete m_pTotalGauge;
		m_pTotalGauge = NULL;
	}
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
	if (g_resrcmng && m_pCompleteOverlay)
	{
		g_resrcmng->Release(m_pCompleteOverlay);
		m_pCompleteOverlay = NULL;
	}
}

// 0082ed90
void FrWorkShopEventDlg::OnProc(const float deltaTime)
{
	if (m_pMyGauge)
		m_pMyGauge->Process(deltaTime);
	if (m_pTotalGauge)
		m_pTotalGauge->Process(deltaTime);
	UpdateEyeBlink983(deltaTime);
}

// 0082edd0
void FrWorkShopEventDlg::SetData983(int myHoles, int openedBoxes, int totalHoles, int totalGoal)
{
	m_myHoles = myHoles;
	m_openedBoxes = openedBoxes;
	m_totalHoles = totalHoles;
	if (totalGoal > 0)
		m_totalGoal = totalGoal;
	CalcBoxCount983();
	m_bGoalDone = m_totalHoles >= m_totalGoal;
}

// 0082ee20
void FrWorkShopEventDlg::ShowGoalBgIfDone983()
{
	if (m_bGoalDone)
		ShowGoalBg983();
}

// 0082ee30
void FrWorkShopEventDlg::Update983(bool bSet, int myHoles, int openedBoxes, int totalHoles, int totalGoal)
{
	if (bSet)
		SetData983(myHoles, openedBoxes, totalHoles, totalGoal);
	if (m_pMyGauge)
		m_pMyGauge->SetValue((float)(m_myHoles % HOLES_PER_BOX));
	if (m_pTotalGauge)
		m_pTotalGauge->SetValue((float)m_totalHoles);
	if (m_pRandomBoxBtn)
	{
		if (m_boxCount > 0)
			m_pRandomBoxBtn->Enable(true);
		else
			m_pRandomBoxBtn->Enable(false);
	}
	ShowGoalBgIfDone983();
}

// 0082eee0
void FrWorkShopEventDlg::OnRandomBoxBtnInit(int param)
{
	m_pRandomBoxBtn = KINDOF_CAST983(FrButton, param);
	if (m_pRandomBoxBtn)
	{
		((FrWndStyleJp983*)m_pRandomBoxBtn)->AddStyle983(FWS_NODBLCLICK);
		m_pRandomBoxBtn->Enable(false);
	}
}

// 0082ef60
void FrWorkShopEventDlg::OnMyBarInit(int param)
{
	if (m_pMyGauge == NULL)
		m_pMyGauge = new CGaugeBar983;
	m_pMyGauge->Init("event_club_personal-progress_bg.tga", "event_club_personal-progress.tga", WRect(212.0f, 420.0f, 0.0f, 30.0f),
		3, true, 1.0f, true, true);
}

// 0082f020
void FrWorkShopEventDlg::OnTotalBarDraw(int param)
{
	if (m_pTotalGauge && !m_bGoalDone)
	{
		m_pTotalGauge->Draw();
		return;
	}
	if (m_pCompleteOverlay && m_bGoalDone)
	{
		WRect dst(580.0f, 55.0f, (float)m_pCompleteOverlay->GetWidth(), (float)m_pCompleteOverlay->GetHeight());
		m_pCompleteOverlay->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, 0xffffffff, 0.0f, 0);
	}
}

// 0082f0e0
void FrWorkShopEventDlg::OnBoxCountInit(int param)
{
	m_pBoxCountArea = KINDOF_CAST983(FrArea, param);
}

// 0082f150
void FrWorkShopEventDlg::OnBoxCountDraw(int param)
{
	FrArea* p = m_pBoxCountArea;
	if (p && m_boxCount > 0)
	{
		const WRect& rc = p->GetRect();
		float x = (float)(rc.w * 0.5 + rc.x);
		m_pFont->Print(x, rc.y, MakeStr("%d", m_boxCount), (eTITLEFONTALINE)1, 0.0f, 1.0f);
	}
}

// 0082f1c0
void FrWorkShopEventDlg::OnMyHoleCountInit(int param)
{
	m_pMyHoleCountArea = KINDOF_CAST983(FrArea, param);
}

// 0082f230
void FrWorkShopEventDlg::OnMyHoleCountDraw(int param)
{
	FrArea* p = m_pMyHoleCountArea;
	if (p)
	{
		const WRect& rc = p->GetRect();
		float x = (float)(rc.w * 0.5 + rc.x);
		m_pFont->Print(x, rc.y, MakeStr("%d", m_myHoles), (eTITLEFONTALINE)1, 0.0f, 1.0f);
	}
}

// 0082f2a0
void FrWorkShopEventDlg::OnEyeBlinkInit(int param)
{
	m_pEyeBlink = KINDOF_CAST983(FrArea, param);
	if (m_pEyeBlink)
		m_pEyeBlink->SetVisible(false);
	m_eyeFrame = 0;
	m_eyeSpeed = 2.0f;
	m_eyeTime = 0.0f;
}

// 0082f340
void FrWorkShopEventDlg::OnHoleCountTextInit(int param)
{
	m_pHoleCountText = KINDOF_CAST983(FrArea, param);
}

// 0082f3b0
void FrWorkShopEventDlg::OnBg1Init(int param)
{
	m_pBg[0] = KINDOF_CAST983(FrArea, param);
}

// 0082f420
void FrWorkShopEventDlg::OnBg2Init(int param)
{
	m_pBg[1] = KINDOF_CAST983(FrArea, param);
}

// 0082f490
void FrWorkShopEventDlg::OnBg3Init(int param)
{
	m_pBg[2] = KINDOF_CAST983(FrArea, param);
}

// 0082f500
void FrWorkShopEventDlg::OnBg4Init(int param)
{
	m_pBg[3] = KINDOF_CAST983(FrArea, param);
}

// 0082f570
void FrWorkShopEventDlg::OnBg5Init(int param)
{
	m_pBg[4] = KINDOF_CAST983(FrArea, param);
}

// 0082f5e0
void FrWorkShopEventDlg::OnBg6Init(int param)
{
	m_pBg[5] = KINDOF_CAST983(FrArea, param);
}

// 0082f650
void FrWorkShopEventDlg::OnBg7Init(int param)
{
	m_pBg[6] = KINDOF_CAST983(FrArea, param);
}

// 0082f6c0
void FrWorkShopEventDlg::OnBg8Init(int param)
{
	m_pBg[7] = KINDOF_CAST983(FrArea, param);
}

// 0082f730
void FrWorkShopEventDlg::OnBg9Init(int param)
{
	m_pBg[8] = KINDOF_CAST983(FrArea, param);
}

// 0082f7a0
void FrWorkShopEventDlg::OnBg10Init(int param)
{
	m_pBg[9] = KINDOF_CAST983(FrArea, param);
}

// 0082f810
void FrWorkShopEventDlg::OnBg11Init(int param)
{
	m_pBg[10] = KINDOF_CAST983(FrArea, param);
}

// 0082f880
void FrWorkShopEventDlg::OnBg12Init(int param)
{
	m_pBg[11] = KINDOF_CAST983(FrArea, param);
}

// 0082f8f0
void FrWorkShopEventDlg::OnHelpInit(int param)
{
	m_pHelpBtn = KINDOF_CAST983(FrButton, param);
}

// 0082fa60
bool FrWorkShopEventDlg::OnInit()
{
	if (m_pFont == NULL)
		m_pFont = new CClubSetWorkShopEventFont;
	if (m_pCompleteOverlay == NULL)
		m_pCompleteOverlay = g_resrcmng->GetOverlay("event_club_complete-progress.tga", 0);
	if (!m_bGoalDone)
	{
		if (m_pTotalGauge == NULL)
			m_pTotalGauge = new CGaugeBar983;
		m_pTotalGauge->Init("event_club_all-progress_bg.tga", "event_club_all-progress.tga", WRect(580.0f, 113.0f, 0.0f, (float)m_totalGoal),
			0x17, true, 1.0f, false, true);
	}
	Update983(false, 0, 0, 0, 0);
	return true;
}

// 0082fba0
void FrWorkShopEventDlg::OnRandomBoxBtnClick()
{
	if (m_boxCount > 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
		WSendPacket packet((enumClientPacket)0x173);
		packet.Encode4(m_boxCount);
		packet.Send(TO_GAME);
	}
}

// 0082fc70
void FrWorkShopEventDlg::OnHelpClick()
{
	FrBirthdayEventHelpDlg* pHelp = CreateForm<FrBirthdayEventHelpDlg>(g_pFresh->GetManager(), this, "BirthdayEventHelp", NULL);
	if (pHelp)
	{
		pHelp->Open(NULL, 1);
		pHelp->OpenHelpDlg("workshop_event_helpimage_jp.jpg");
	}
}
}
