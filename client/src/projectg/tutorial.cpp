#include "minatl.h"
#include "projectg.h"
#include "tutorial.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "golfdoc.h"
#include "wind.h"
#include "chatmsg.h"
#include "quadtree.h"
#include "quizdlg.h"
#include "taskmanager.h"
#include "polysoup.h"
#include "club.h"
#include "golfball.h"
#include "commonutil.h"
#include "networksystem.h"
#include "contentsdoc.h"
#include "packets/tutorialcontainer.h"	// PKT (frente C)
#include "capturedbg.h"
#include "jp/uiframesetmanager.h"

// HACK: literals shared between C code and a naked function that owned them in the original are
// defined as statics of that (inline) naked function, so they keep their .rdata slot; the C users
// reach them through the rb_ alias. RB_LIT(alias, literal) is the plain literal in the C-only build.
#define RB_LIT(sym, lit) lit
#define RB_DBL(sym, lit) lit
#define RB_FLT(sym, lit) lit

extern Fresh* g_pFresh;
extern bool g_bQuit;
extern WMatrix g_camera;

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

AnimatedOverlay::AnimatedOverlay()
{
	m_src = WRect(0.0f, 0.0f, 1.0f, 1.0f);
	m_dest = WRect(0.0f, 0.0f, 1.0f, 1.0f);
	m_frameTime = 0.0f;
	m_elapsed = 0.0f;
	m_frame = 0;
	m_frameCount = 0;
	m_bTarget = false;
	m_bShow = false;
	m_bLoop = false;
	m_fade = 0;
	m_alpha = 255.0f;
	m_pOverlay = NULL;
	m_cols = 0;
}

AnimatedOverlay::~AnimatedOverlay()
{
	for (std::vector<WOverlay*>::iterator it = m_overlays.begin();
		it != m_overlays.end(); ++it)
	{
		if (g_resrcmng && *it)
		{
			g_resrcmng->Release(*it);
			*it = NULL;
		}
	}
}

void AnimatedOverlay::Process(float delta)
{
	if ((m_bShow || m_bTarget) && m_frameCount >= 1)
	{
		if (m_fade == 0)
		{
			if (m_frameTime > 0.0f)
			{
				m_elapsed += delta;
				if (m_elapsed > m_frameTime)
				{
					if (m_frameCount == 1)
						m_bShow = !m_bShow;
					m_frame++;
					m_elapsed -= m_frameTime;
					if (!m_bTarget || !m_bLoop)
					{
						if (m_frame == m_frameCount)
						{
							m_frame = 0;
							m_elapsed = 0.0f;
							m_frameTime = 0.0f;
							m_bShow = m_bTarget;
						}
					}
					else
						m_frame %= m_frameCount;
				}
			}
		}
		else
		{
			m_elapsed += delta * 3.0f;
			m_alpha = (float)m_fade * m_elapsed * 255.0f +
				(float)(1 - m_fade) * 128.0f;
			if (m_alpha < 0.0f || m_alpha > 255.0f)
			{
				m_alpha = Between(0.0f, m_alpha, 255.0f);
				m_fade = 0;
				m_elapsed = 0.0f;
				m_bShow = m_bTarget;
			}
		}
		m_src.x = (float)(m_frame % m_cols) * m_src.w;
		m_src.y = (float)(m_frame / m_cols) * m_src.h;
	}
}

void AnimatedOverlay::Activate(bool active, bool fade, bool loop)
{
	m_bTarget = active;
	m_bLoop = loop;
	if (active)
	{
		m_frame = 0;
		m_elapsed = 0.0f;
		m_alpha = 255.0f;
		m_bShow = active;
	}
	if (fade)
		m_fade = active ? 1 : -1;
}

void AnimatedOverlay::Render()
{
	if (m_bShow && m_pOverlay)
		m_pOverlay->Render(g_view, m_src, m_dest, 0,
			((int)m_alpha << 24) | 0xffffff, 0, 0);
}

void AnimatedOverlay::SetSourceOverlay(const char* filename, int cols, int rows,
	int mode, float width, float height)
{
	if (!m_overlays.empty())
	{
		std::vector<WOverlay*>::iterator it = m_overlays.begin();
		if (g_resrcmng && *it)
		{
			g_resrcmng->Release(*it);
			*it = NULL;
		}
		m_overlays.erase(it);
	}
	WOverlay* overlay = g_resrcmng->GetOverlay(filename, 0);
	if (overlay)
	{
		m_overlays.push_back(overlay);
		m_cols = cols;
		m_pOverlay = overlay;
		m_frameCount = cols * rows;
		if (mode == 1)
		{
			m_src.w = width / (float)overlay->GetWidth();
			m_src.h = height / (float)overlay->GetHeight();
			return;
		}
		m_src.w = 1.0f / (float)cols;
		m_src.h = 1.0f / (float)rows;
	}
}

void AnimatedOverlay::GetNextOverlay()
{
	m_src.x = (float)(m_frame % m_cols) * m_src.w;
	m_src.y = (float)(m_frame / m_cols) * m_src.h;
}

struct CMouseCursorAccess : public CMouseCursor
{
	using CMouseCursor::m_bArea;
	using WSingleton<CMouseCursor>::m_pInstance;
};

struct FrWndAccess : public FrWnd
{
	using FrWnd::m_dwStyle;
};

#define FRWND_STYLE(wnd) (((FrWndAccess*)(FrWnd*)(wnd))->m_dwStyle)

#define MOUSECURSOR() ((CMouseCursorAccess*)CMouseCursorAccess::m_pInstance)


__int64 g_tutorialModeValue[20][6] = {
	{ 50,  50,  30,  12,  3,  0 },
	{ 100, 100, 60,  30,  11, 0 },
	{ 110, 110, 65,  32,  12, 0 },
	{ 180, 180, 98,  59,  20, 0 },
	{ 150, 150, 85,  43,  16, 0 },
	{ 55,  55,  36,  17,  5,  0 },
	{ 100, 100, 60,  30,  11, 0 },
	{ 150, 150, 85,  43,  16, 0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 110, 110, 65,  32,  12, 0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 0,   0,   0,   0,   0,  0 },
	{ 180, 180, 98,  59,  20, 0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 50,  50,  30,  12,  3,  0 },
	{ 80,  80,  52,  26,  9,  0 },
	{ 800, 800, 520, 260, 90, 0 },
	{ 0,   0,   0,   0,   0,  0 },
	{ 80,  80,  52,  26,  9,  0 },
};

struct sTutorialModeInfo
{
	int mode;
	int msg;
	int count;
};

struct sTutorialRData
{
	sTutorialModeInfo modeInfo[20];
	char answers[3][8];
	char facials[9][32];
	char missionImages[14][32];
	int missionTextX[14];
	float degToRad;
	float epsilon;
	float boxOffset[2];
};

static const sTutorialRData s_data = {
	{
     { 0x1, 0x1a000002, 10 },
     { 0x2, 0x1800000b, 3 },
     { 0x4, 0x18000025, 3 },
     { 0x8, 0x18000005, 3 },
     { 0x10, 0x18000004, 3 },
     { 0x20, 0x1800000a, 3 },
     { 0x40, 0x18000000, 3 },
     { 0x80, 0x1a000010, 1000 },
     { 0x100, 0x1a000002, 10 },
     { 0x200, 0x18000028, 1 },
     { 0x400, 0x18000006, 1 },
     { 0x800, 0x1800000a, 3 },
     { 0x1000, 0x18000000, 4 },
     { 0x2000, 0x18000001, 4 },
     { 0x8000, 0x18000005, 3 },
     { 0x10000, 0x18000005, 3 },
     { 0x20000, 0x18000005, 3 },
     { 0x40000, 0x18000005, 3 },
     { 0x80000, 0x18000005, 3 },
     { 0x80000, 0x18000005, 3 },
	 },
	{ "ok", "yes", "keep" },
	{
     "dummy", "pipin_facial_smile01",
     "pipin_facial_smile03", "pipin_facial_play",
     "pipin_facial_cry", "pipin_facial_difficulty",
     "pipin_facial_angry", "pipin_facial_surprise",
     "pipin_facial_embarras", },
	{
     "[mission01.png", "[mission02.png",
     "[mission03.png", "[mission04.png",
     "[mission05.png", "[mission06.png",
     "[mission07.png", "[mission08.png",
     "[mission09.png", "[mission10.png",
     "[mission11.png", "[mission12.png",
     "[mission13.png", "[mission14.png",
	 },
	{
     44, 73,
     161, 44,
     155, 100,
     101, 155,
     143, 100,
     100, 100,
     100, 93,
	 },
	0.017453293f,
	1e-5f,
	{ 500.0f, 140.0f },
};

IMPLEMENT_ACTOR(CTutorial, CTaskMain)

CTutorial::CTutorial()
{
	m_pForm248 = NULL;
	m_pNotifyForm = NULL;
	m_pReviewForm = NULL;
	m_pTipForm = NULL;
	m_pTipCaddieWnd = NULL;
	m_2dc = 0;
	m_pForm264 = NULL;
	m_pOverlay = NULL;
	m_pQuizDlg = NULL;
	m_270 = 0;
	m_b2d4 = false;
	m_b21c = false;
	memset(m_vals, 0, sizeof(m_vals));
	m_dispPriority = (eDispPriority)4;
	AddSkipList(0x10);
	m_bInputBlocked = false;
	m_pTutorialDoc983 = NULL; // 983 007cdc98
}

CTutorial::~CTutorial()
{
	if (m_bInputBlocked)
	{
		FreeInput();
	}
}

void CTutorial::OnInit()
{
	if (GOLFDOC()->m_tutorialMode != 15)
	{
		m_packed.a = 0;
		m_packed.b = 0;
		m_packed.c = 0;
		m_packed.d = 0;
		m_packed.e = false;
		// 983 007ccf20 (pedido C58.1): documento 0xa4 (CTutorialContainer) para o 0xae do MoveNextStep
		IContentsDataContainer* pDoc =
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4);
		if (pDoc != NULL)
			m_pTutorialDoc983 = (CTutorialContainer983*)pDoc;
		Wind().CheckSpecialWind();
		_ReadWriteBarrier();
		m_mode = -1;
		m_2b8 = -1;
		m_b29c = false;
		m_2a0 = 0;
		m_pNotifyForm = NULL;
		m_228 = 0;
		m_22c = 0;
		m_b2b4 = false;
		m_2cc = 1.5f;
		m_2d0 = 0;
		m_b2d5 = false;
		m_2d8 = 0;
		m_b2a4 = false;
		m_2bc = (g_view->GetWidth() - 370.0f) * 0.5f;
		m_2c0 = 50.0f;
		m_2c4 = 370.0f;
		m_2c8 = 125.0f;
		IsPractice();
	}
}

void CTutorial::OnLoad()
{
	if (GOLFDOC()->m_tutorialMode != 15)
	{
		cFile* file = g_resrcmng->GetCFile("tutorial_text.txt", -1);
		if (file)
		{
			int size = file->m_nLen;
			char* buffer = new char[size + 1];
			memset(buffer, 0, size + 1);
			file->Read(buffer, size);
			char* p = buffer;
			while (p)
			{
				int len = strlen(p);
				if (len == 0)
				{
					break;
				}
				int key;
				char line[1024];
				sscanf(p, "%d", &key);
				p = GetLine(p, line);
				m_lines[key] = line;
			}
			CloseCFile(file);
			delete[] buffer;
		}
		GOLFDOC()->m_pCameraTargetPuppet =
			g_resrcmng->GetPuppet("camera_target.pet", false, true, false);
		OnImageLoad();
	}
}

void CTutorial::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode == 15)
	{
		return;
	}
	if (m_pad20c[0])
	{
		g_input->SetFakeInput(m_pad20c, m_b21c);
		m_b21c = false;
	}
	if (m_macroWait >= 0)
	{
		m_macroWait -= (int)(delta * 1000.0f);
		if (m_macroWait < 0)
		{
			if (m_pTipForm == NULL || m_pTipCaddieWnd != NULL)
			{
				PopMacro();
			}
			else
			{
				m_pTipForm->Close(true);
			}
		}
	}
	if (m_state == 1)
	{
		if (m_pForm264 == NULL && !CCapturedBg::Instance()->IsCaptured())
		{
			OpenMissionForm();
		}
		if (m_state == 1 && m_b2b4)
		{
			m_2b8 += (int)(delta * -1000.0f);
			if (m_2b8 < 0)
			{
				m_2b8 = 24000;
				if (GOLFDOC()->m_gameMode & 0x208)
				{
					Again_Msg(false);
				}
			}
			else if (m_2b8 < 15000)
			{
				Again_Msg(true);
			}
		}
	}
	int i;
	for (i = 0; i < 26; i++)
	{
		m_overlays[i].Process(delta);
	}
	m_overlay3.Process(delta);
	if (m_overlay3.IsActivate())
	{
		if (m_overlay3.m_dest.y > 20.0f)
		{
			m_overlay3.m_dest.y -= 2.0f;
		}
		if (m_2cc > 1.0f)
		{
			m_2cc -= 0.004f;
		}
		m_overlay3.m_dest.w = m_2cc * 262.0f;
		m_overlay3.m_dest.h = m_2cc * 44.0f;
		m_overlay3.m_dest.x = (g_view->GetWidth() - m_overlay3.m_dest.w) * 0.5f;
	}
	for (i = 0; i < 3; i++)
	{
		m_overlays2[i].Process(delta);
	}
	switch (m_state)
	{
	case 1:
		if (GOLFDOC()->m_tutorialMode == 12)
		{
			return;
		}
		if (!CheckMissionComplete())
		{
			return;
		}
		break;
	case 4:
		m_state = 5;
		BlockInput();
		OpenMissionForm();
		OpenQuizForm(m_270);
		return;
	case 6:
		if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
		{
			return;
		}
		if (m_pForm264)
		{
			m_pForm264->Close(true);
		}
		if (m_bQuizCorrect)
		{
			MissionComplate();
		}
		else
		{
			HandleMsg(MsgObject(NULL, M983(0x154), 0, 0, 0, 0, 0));
		}
		return;
	default:
		return;
	}
	MissionComplate();
}

void CTutorial::OnDisplay()
{
	if (IsLocalContent(S3_TUTORIAL_RENEWAL))
	{
		for (int i = 0; i < 26; i++)
		{
			m_overlays[i].Render();
		}
		m_overlay3.Render();
		ChanceOverlay();
		for (int i = 0; i < 3; i++)
		{
			m_overlays2[i].Render();
		}
		if (!CCapturedBg::Instance()->IsCaptured() && m_pOverlay)
		{
			m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(g_view->GetWidth() - (float)m_pOverlay->GetWidth() -
						10.0f,
					40.0f, (float)m_pOverlay->GetWidth(),
					(float)m_pOverlay->GetHeight()),
				0, 0xffffffff, 0, 0);
		}
	}
}

void CTutorial::OnDestroy()
{
	if (CSharedDoc::Instance()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
		CSharedDoc::Instance()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
	{
		SetCurMap(0);
		CSharedDoc::Instance()->m_golfGame.gameType = 0;
		strcpy(CSharedDoc::Instance()->m_golfGame.gameTypeName,
			CSharedDoc::Instance()->m_gameTypeInfo[0].name);
		g_input->FreeKey();
		g_input->FreeMouse();
		ClearMacro();
		m_lines.clear();
		if (g_resrcmng)
		{
			if (GOLFDOC()->m_pCameraTargetPuppet)
			{
				g_resrcmng->Release(GOLFDOC()->m_pCameraTargetPuppet, false);
				GOLFDOC()->m_pCameraTargetPuppet = NULL;
			}
			if (g_resrcmng && m_pOverlay)
			{
				g_resrcmng->Release(m_pOverlay);
				m_pOverlay = NULL;
			}
		}
	}
}

void CTutorial::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x148):
		PopMacro();
		return;
	case M983(0x14b):
		if (m_pNotifyForm == NULL &&
			!((bool*)&m_packed)[GOLFDOC()->m_tutorialMode])
		{
			m_b29c = true;
		}
		SkipMission();
		return;
	case M983(0x149):
		if (m_state == 1)
		{
			if (GOLFDOC()->m_tutorialMode == 12)
			{
				CheckMissionComplete();
				return;
			}
			MissionComplate();
			return;
		}
		PopMacro();
		return;
	case M983(0x154):
		if (IsLocalContent(S3_TUTORIAL_RENEWAL))
		{
			if (m_pReviewForm)
			{
				m_pReviewForm->Close(FrCANCEL, true);
			}
			BlockInput();
			g_input->BlockKey("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9");
			if (m_state == 1 || m_state == 6)
			{
				m_25c = 4;
				m_260 = 2;
				m_state = 7;
				switch (GOLFDOC()->m_tutorialMode)
				{
				case 2:
					m_25c = 1;
					OpenTipCaddieForm(m_lines[0x133].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 4:
				{
					int count = m_228;
					if (count >= 2)
					{
						m_228 = count + 1;
						OpenTipCaddieForm(m_lines[0x199].c_str(),
							(FRESH_PFN_RESULT)&CTutorial::
								OnTipCaddieNoneResult);
						OpenMissionFailedForm();
						return;
					}
					m_state = 1;
					m_228 = count + 1;
					FreeInput();
					break;
				}
				case 3:
					OpenTipCaddieForm(m_lines[608].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 5:
					OpenTipCaddieForm(m_lines[810].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 8:
					SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
					OpenTipCaddieForm(m_lines[907].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 6:
					OpenTipCaddieForm(m_lines[710].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 7:
					OpenTipCaddieForm(m_lines[510].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 9:
					OpenTipCaddieForm(m_lines[1107].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 10:
					OpenTipCaddieForm(m_lines[1208].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 12:
					while (!m_macros.empty())
					{
						m_macros.pop();
					}
					PushMacroHitPointMission();
					m_22c = 0;
					SendMsg(this, "GolfRule", M983(0x89),
						(int)"\xbd\xc3\xc0\xdb\xc1\xa1",
						(int)"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
							 "3",
						0, 0);
					PopMacro();
					OpenTipCaddieForm(m_lines[1309].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 11:
					OpenTipCaddieForm(m_lines[1010].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 13:
					OpenTipCaddieForm(m_lines[1413].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				case 14:
					OpenTipCaddieForm(m_lines[1513].c_str(),
						(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
					OpenMissionFailedForm();
					return;
				default:
					return;
				}
			}
			SendMsg(this, "Screen", M983(0x1ba), 1, 0, 0, 0);
		}
		break;
	case M983(0x14c):
		ReviewTutorial();
		return;
	case M983(0x14a):
	{
		float* values = (float*)msg.param1;
		m_240 = values[0];
		m_244 = values[1];
	}
		return;
	case M983(0x152):
		*(int**)msg.param1 = m_vals;
		return;
	case M983(0x155):
	{
		float value = (float)msg.param1;
		m_2d8 = value;
		if (value < (float)GOLFDOC()->m_tutorialMinGauge983)
		{
			GOLFDOC()->m_bar.gauge = (float)GOLFDOC()->m_tutorialMinGauge983;
			SendMsg(NULL, "GolfRule", M983(0xa2), 0, 0, 0, 0);
		}
	}
	break;
	case M983(0x156):
		PopSpecial();
		return;
	case M983(0x20f):
		switch (msg.param1)
		{
		case 0:
			m_b2d4 = true;
			g_input->BlockMouse(LEFT_BUTTON);
			BlockInput();
			return;
		case 1:
			m_b2d4 = false;
			g_input->FreeMouse(LEFT_BUTTON);
			return;
		case 2:
		{
			float value = *(float*)msg.param2;
			GOLFDOC()->m_bar.gauge = value;
			if (msg.param3 == 1)
			{
				GOLFDOC()->m_bar.dir = 1;
			}
			else if (msg.param3 == 2)
			{
				GOLFDOC()->m_bar.gaugeStep = value;
				return;
			}
		}
		break;
		}
		break;
	}
}

bool CTutorial::CheckMissionComplete()
{
	switch (GOLFDOC()->m_tutorialMode)
	{
	case 0:
		return CheckCameraTarget();
	case 1:
		return CheckClubTarget();
	case 12:
		m_22c++;
		if (m_22c == 4)
		{
			MissionComplate();
		}
		BlockInput();
		MOUSECURSOR()->SetActive(true);
		if (m_pForm248)
		{
			m_pForm248->Close(FrCANCEL, true);
		}
		PopMacro();
		return true;
	}
	return false;
}

void CTutorial::BlockInput()
{
	g_input->BlockKey();
	g_input->FreeKey("ESCAPE");
	g_input->FreeKey("ENTER");
	g_input->FreeKey("PADENTER");
	g_input->FreeKey("FUNC_11");
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_BAR] = false;
	g_input->BlockMouse(MIDDLE_BUTTON);
	g_input->BlockMouse(RIGHT_BUTTON);
	m_bInputBlocked = true;
}

void CTutorial::FreeInput()
{
	g_input->FreeKey();
	g_input->FreeMouse();
	g_input->FreeMouse(LEFT_BUTTON);
	g_input->FreeMouse(MIDDLE_BUTTON);
	g_input->FreeMouse(RIGHT_BUTTON);
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_BAR] = true;
	m_bInputBlocked = false;
}

char* CTutorial::GetLine(char* src, char* dest)
{
	char c = *src;
	while (c != '"')
	{
		if (c == '\0')
		{
			return NULL;
		}
		src++;
		c = *src;
	}
	if (*src != '\0')
	{
		src++;
		c = *src;
		if (c != '\0')
		{
			while (c != '"')
			{
				if (c == '\\' && src[1] == 'n')
				{
					*dest = '\n';
					dest++;
					src += 2;
				}
				else
				{
					*dest = c;
					dest++;
					src++;
				}
				c = *src;
				if (c == '\0')
				{
					return NULL;
				}
			}
			if (*src != '\0')
			{
				*dest = '\0';
				return src + 1;
			}
		}
	}
	return NULL;
}

// 983 (007cc830): modo e licao vem do documento 0xa4 (+0xdac), nao do ator "TutorialMain" do 645 (M983(0x14f))
void CTutorial::IsPractice()
{
	if (m_pTutorialDoc983)
	{
		int mode = m_pTutorialDoc983->m_mode;
		m_mode = m_pTutorialDoc983->m_index;
		GOLFDOC()->m_tutorialMode = mode;
		m_2dc = s_data.modeInfo[m_mode].mode;
	}
	m_pNotifyForm = NULL;
	MOUSECURSOR()->Store();
	for (int i = 0; i < 26; i++)
		m_overlays[i].Activate(false, true, true);
	BlockInput();
	SendMsg(this, "Screen", M983(0x1bc), 2, 0, 0, 0);
	InitTutorialMode(GOLFDOC()->m_tutorialMode);
}

void CTutorial::ReviewTutorial()
{
	if (m_pReviewForm == NULL)
	{
		BlockInput();
		MOUSECURSOR()->SetActive(true);
		if (m_pForm248)
		{
			m_pForm248->Close(FrCANCEL, true);
		}
		m_state = 3;
		m_pReviewForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		m_pReviewForm->SetMessage(
			K2L_Compatibility("\xbc\xb3\xb8\xed\xc0\xbb \xb4\xd9\xbd\xc3 \xb5\xe9\xc0\xb8\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		m_pReviewForm->EnableDrag(false);
		m_pReviewForm->Open((FRESH_PFN_RESULT)&CTutorial::OnReviewFormResult,
			WPoint((g_view->GetWidth() - m_pReviewForm->GetRect().w) * 0.5f,
				80.0f),
			2);
		m_pReviewForm->MoveCursor("ok");
	}
}

void CTutorial::MoveNextStep()
{
	if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
	{
		if (m_pNotifyForm)
		{
			return;
		}
		if (m_pForm264)
		{
			m_pForm264->Close((eFormRet)0, true);
		}
		if (!m_b29c)
		{
			((bool*)&m_packed)[GOLFDOC()->m_tutorialMode] = true;
		}
		BlockInput();
		MOUSECURSOR()->SetActive(true);
		if (m_pForm248)
		{
			m_pForm248->Close(FrCANCEL, true);
		}
		m_state = 3;
	}
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		// PKT (frente C, lote 51) - 983 (007cd2b0): o 0xae sai pelo documento 0xa4 (CTutorialContainer, guardado em
		// +0xdac pelo OnInit) e a volta e para o lobby (TOPPAGE + 0x55 "tutorial"), nao para o CTutorialTask/TutorialMain.
		CProjectG::Instance()->FromGolfToLobbySettings();
		m_pTutorialDoc983->SendTutorialResult983(m_mode, m_2dc, 1,
			m_2dc == 0x80);
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x55 /*983; sem par no 645*/,
			(int)"tutorial", 0, 0, 0);
	}
	if (GOLFDOC()->m_tutorialMode < 15)
	{
		m_pNotifyForm = CreateForm<FrForm>(g_pFresh->GetManager(),
			(FrCmdTarget*)this, "tutorial", NULL);
		m_pNotifyForm->SetMessage(
			K2L_Compatibility("\xb9\xcc\xbc\xc7\xc0\xbb \xbf\xcf\xb7\xe1\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9. \n\n"
			"\xc3\xe0\xc7\xcf\xc7\xd5\xb4\xcf\xb4\xd9! "),
			false);
	}
	else
	{
		m_pNotifyForm = CreateForm<FrForm>(g_pFresh->GetManager(),
			(FrCmdTarget*)this, "tutorial", NULL);
		m_pNotifyForm->SetMessage(
			K2L_Compatibility("\xc6\xce\xbe\xdf\xb9\xe8\xbf\xec\xb1\xe2\xb8\xa6 \xb8\xb6\xc4\xa1\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9. "
			"\xc3\xe0\xc7\xcf\xc7\xd5\xb4\xcf\xb4\xd9."),
			false);
	}
	if (m_pNotifyForm)
	{
		m_pNotifyForm->EnableDrag(false);
		m_pNotifyForm->Open((FRESH_PFN_RESULT)&CTutorial::OnCompleteFormResult,
			WPoint((g_view->GetWidth() - m_pNotifyForm->GetRect().w) * 0.5f,
				80.0f),
			2);
		m_pNotifyForm->MoveCursor("ok");
	}
}

void CTutorial::MissionComplate()
{
	if (m_pNotifyForm == NULL)
	{
		if (m_pForm264)
		{
			m_pForm264->Close((eFormRet)0, true);
		}
		if (!m_b29c)
		{
			((bool*)&m_packed)[GOLFDOC()->m_tutorialMode] = true;
		}
		BlockInput();
		MOUSECURSOR()->SetActive(true);
		if (m_pForm248)
		{
			m_pForm248->Close(FrCANCEL, true);
		}
		m_state = 3;
		m_overlay3.m_dest.w = 393.0f;
		m_overlay3.m_dest.h = 66.0f;
		m_overlay3.m_dest.x = (g_view->GetWidth() - 393.0f) * 0.5f;
		m_overlay3.m_dest.y = (g_view->GetHeight() - 66.0f) * 0.5f;
		m_overlay3.m_frameTime = 0.0f;
		m_overlay3.Activate(true, true, true);
		m_b2b4 = false;
		PopMacro();
	}
}

void CTutorial::PrepareMission()
{
	m_228 = 0;
	int mode = GOLFDOC()->m_tutorialMode;
	if (mode == 3)
	{
		GOLFDOC()->GetPlayer(0)->gauge = 33.0f;
	}
	else if (mode == 5)
	{
		m_vals[0] = 0x18000004;
		m_vals[1] = 0x18000004;
		m_vals[2] = 0x18000004;
		GOLFDOC()->GetPlayer(0)->itemNum = 3;
	}
	else if (mode == 8)
	{
		GOLFDOC()->GetPlayer(0)->totalPang = 0;
		GOLFDOC()->GetPlayer(0)->bunker = 0;
		sPlayerData* player = GOLFDOC()->GetPlayer(0);
		player->stroke[GOLFDOC()->m_currentHole - 1] = 1;
		SendMsg(this, "GolfRule", M983(0x89), (int)"\xbd\xc3\xc0\xdb\xc1\xa1", 0, 0,
			0);
	}
	switch (GOLFDOC()->m_tutorialMode)
	{
	case 2:
	case 3:
	case 5:
	case 11:
	case 13:
	case 14:
		SendMsg(this, "GolfRule", M983(0x89), (int)"\xbd\xc3\xc0\xdb\xc1\xa1",
			(int)"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
				 "3",
			0, 0);
		break;
	}
}

void CTutorial::SkipMission()
{
	if (m_pNotifyForm == NULL)
	{
		BlockInput();
		MOUSECURSOR()->SetActive(true);
		if (m_pForm248)
		{
			m_pForm248->Close(FrCANCEL, true);
		}
		m_state = 3;
		m_pNotifyForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		m_pNotifyForm->SetMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce \xc4\xf9\xbd\xba\xc6\xae\xb8\xa6 "
			"\xc6\xf7\xb1\xe2\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		m_pNotifyForm->EnableDrag(false);
		m_pNotifyForm->Open(
			(FRESH_PFN_RESULT)&CTutorial::OnSkipMissionFormResult,
			WPoint((g_view->GetWidth() - m_pNotifyForm->GetRect().w) * 0.5f,
				80.0f),
			2);
		m_pNotifyForm->MoveCursor("ok");
	}
}

void CTutorial::OnImageLoad()
{
	char filename[256];
	m_overlays[0].SetSourceOverlay("tuto_point.tga", 4, 1, 0, 1.0f, 1.0f);
	m_overlay3.SetSourceOverlay("text_questclear.png", 1, 1, 0, 1.0f, 1.0f);
	m_overlays2[0].SetSourceOverlay("tutorial_icon_chance.png", 1, 1, 0, 1.0f,
		1.0f);
	m_overlays2[1].SetSourceOverlay("tutorial_icon_chance.png", 1, 1, 0, 1.0f,
		1.0f);
	m_overlays2[2].SetSourceOverlay("tutorial_icon_chance.png", 1, 1, 0, 1.0f,
		1.0f);
	for (int i = 1; i < 26; i++)
	{
		if (i < 20)
		{
			sprintf(filename, "tutorial_keyanmaion%02d.png", i);
		}
		else
		{
			sprintf(filename, "tutorial_ground%02d.png", i - 20);
		}
		switch (i)
		{
		case 1:
			m_overlays[1].SetSourceOverlay(filename, 3, 1, 1, 128.0f, 88.0f);
			break;
		case 3:
			m_overlays[3].SetSourceOverlay(filename, 2, 1, 1, 128.0f, 88.0f);
			break;
		case 4:
			m_overlays[4].SetSourceOverlay(filename, 3, 1, 1, 136.0f, 88.0f);
			break;
		case 5:
			m_overlays[5].SetSourceOverlay(filename, 2, 1, 1, 136.0f, 88.0f);
			break;
		case 0:
		case 2:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 16:
		case 17:
		case 18:
		case 19:
			m_overlays[i].SetSourceOverlay(filename, 2, 1, 1, 128.0f, 88.0f);
			break;
		case 12:
			m_overlays[12].SetSourceOverlay(filename, 2, 4, 1, 136.0f, 105.0f);
			break;
		case 13:
		case 14:
			m_overlays[i].SetSourceOverlay(filename, 2, 1, 1, 136.0f, 105.0f);
			break;
		case 15:
			m_overlays[15].SetSourceOverlay(filename, 2, 2, 1, 136.0f, 105.0f);
			break;
		case 20:
		case 21:
		case 22:
			m_overlays[i].SetSourceOverlay(filename, 1, 1, 0, 1.0f, 1.0f);
			break;
		case 23:
			m_overlays[23].SetSourceOverlay(filename, 2, 1, 1, 128.0f, 88.0f);
			break;
		case 24:
			sprintf(filename, "tu_icon_click.tga");
			m_overlays[24].SetSourceOverlay(filename, 1, 1, 0, 1.0f, 1.0f);
			break;
		case 25:
			sprintf(filename, "tu_arrow_click.tga");
			m_overlays[25].SetSourceOverlay(filename, 1, 1, 0, 1.0f, 1.0f);
			break;
		}
	}
	m_overlays[23].SetSourceOverlay("tutorial_preview.png", 1, 1, 0, 1.0f,
		1.0f);
}

void CTutorial::Again_Msg(bool close)
{
	if (m_state == 1)
	{
		if (close)
		{
			if (m_pTipForm)
			{
				m_pTipForm->Close(true);
			}
		}
		else
		{
			m_25c = 1;
			m_260 = 0;
			int line = 5;
			switch (GOLFDOC()->m_tutorialMode)
			{
			case 1:
				line = 213;
				break;
			case 2:
				line = 308;
				break;
			case 4:
				line = 410;
				break;
			case 3:
				line = 607;
				break;
			case 5:
				line = 809;
				break;
			case 9:
				line = 1107;
				break;
			case 10:
				line = 1209;
				break;
			case 11:
				line = 1011;
				break;
			case 12:
				line = 1309;
				break;
			case 13:
				line = 1414;
				break;
			case 14:
				line = 1514;
				break;
			}
			if (m_pTipForm == NULL)
			{
				OpenTipCaddieForm(m_lines[line].c_str(),
					(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieNoneResult);
			}
			if (m_pTipForm)
			{
				m_pTipForm->EnableFlag(2);
			}
			if (m_pTipCaddieWnd)
			{
				m_pTipCaddieWnd->Enable(true);
			}
			MOUSECURSOR()->ForceMove(g_view->GetWidth() * 0.5f,
				g_view->GetHeight() * 0.5f, -1.0f);
		}
	}
}

void CTutorial::ChanceOverlay()
{
	if (m_state == 1 && GOLFDOC()->m_tutorialMode == 4)
	{
		for (int i = 0; i < 3; i++)
		{
			if (i < 3 - m_228)
			{
				m_overlays2[i].m_dest.y = 160.0f;
				m_overlays2[i].m_dest.w = 58.0f;
				m_overlays2[i].m_dest.h = 53.0f;
				m_overlays2[i].m_dest.x =
					g_view->GetWidth() - (float)(i + 1) * 62.0f - 20.0f;
				m_overlays2[i].Activate(true, true, true);
			}
			else
			{
				m_overlays2[i].m_bTarget = false;
				m_overlays2[i].m_fade = -1;
			}
			m_overlays2[i].m_bLoop = true;
		}
		return;
	}
	for (int i = 0; i < 3; i++)
	{
		m_overlays2[i].m_bTarget = false;
		m_overlays2[i].m_bLoop = true;
		m_overlays2[i].m_fade = -1;
	}
}

void CTutorial::PriviewOverlay()
{
	PushMacro(MACRO_13,
		MakeStr("%d %.1f %.1f", 23, (g_view->GetWidth() - 128.0f) * 0.5f,
			g_view->GetHeight() * 0.25f),
		0, 1);
	PushMacro(MACRO_10, MakeStr("%d %.1f %.1f %.1f", 23, 128.0, 64.0, 0.0), 0,
		1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_12, MakeStr("%d", 23), 500, 1);
}

bool CTutorial::OnTipFormResult(int result, FrForm*)
{
	m_pForm248 = NULL;
	if (result != 2)
	{
		if (m_b2d4)
		{
			g_input->BlockMouse(LEFT_BUTTON);
		}
		else
		{
			FreeInput();
		}
		MOUSECURSOR()->Store();
		if (m_missionState == 6)
		{
			m_missionState = 0;
			MissionComplate();
			return true;
		}
		PopMacro();
	}
	return true;
}

bool CTutorial::OnCompleteFormResult(int result, FrForm*)
{
	m_pNotifyForm = NULL;
	MOUSECURSOR()->Store();
	if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
	{
		if (CSharedDoc::Instance()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
			GOLFDOC()->m_tutorialMode == 7)
		{
			if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_GAME))
			{
				CProjectG::Instance()->FromGolfToLobbySettings();
				CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
					false);
				CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0,
					(int)"TOPPAGE", 0, 0, 0);
			}
			else
			{
				g_bQuit = true;
			}
		}
		else
		{
			if (m_b29c && result == 1)
			{
				m_2a0++;
			}
			m_b29c = false;
		}
		SendMsg(this, "Screen", M983(0x1bc), result == 1 ? 2 : 1, 0, 0, 0);
		InitTutorialMode(GOLFDOC()->m_tutorialMode + (result == 1));
		return true;
	}
	if (result != 0 &&
		!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		g_bQuit = true;
	}
	return true;
}

bool CTutorial::OnReviewFormResult(int result, FrForm*)
{
	m_pReviewForm = NULL;
	MOUSECURSOR()->Store();
	if (result == 1)
	{
		SendMsg(this, "Screen", M983(0x208), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1bc), 1, 0, 0, 0);
		InitTutorialMode(GOLFDOC()->m_tutorialMode);
		return true;
	}
	if (result == 2)
	{
		if (GOLFDOC()->m_tutorialMode != 6 && GOLFDOC()->m_tutorialMode != 7)
		{
			FreeInput();
			m_state = 1;
			switch (GOLFDOC()->m_tutorialMode)
			{
			case 2:
			case 3:
			case 4:
			case 5:
			case 8:
			case 10:
			case 11:
			case 13:
			case 14:
				break;
			default:
				OpenMissionForm();
				SendMsg(this, "Screen", M983(0x1bc), 2, 0, 0, 0);
				SendMsg(this, "Screen", M983(0x1ba), 1, 0, 0, 0);
				break;
			}
			return true;
		}
		m_state = 4;
	}
	return true;
}

bool CTutorial::OnSkipMissionFormResult(int result, FrForm*)
{
	m_pNotifyForm = NULL;
	MOUSECURSOR()->Store();
	if (m_b29c == true && result == 1)
	{
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			// 983 (007c1f20): volta ao lobby e abre o widget do tutorial (sem CTutorialTask nem FromGolfToLobbySettings)
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
				(int)"CTutorialWidget", (int)"TOPPAGE", 0, 0);
			return true;
		}
	}
	else
	{
		if (GOLFDOC()->m_tutorialMode != 6 && GOLFDOC()->m_tutorialMode != 7)
		{
			m_state = 1;
			FreeInput();
			OpenMissionForm();
			SendMsg(this, "Screen", M983(0x1bc), 2, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1ba), 1, 0, 0, 0);
			return true;
		}
		m_state = 4;
	}
	return true;
}

bool CTutorial::OnPracticeFormResult(int result, FrForm*)
{
	m_pNotifyForm = NULL;
	MOUSECURSOR()->Store();
	if (result == 1)
	{
		m_b2a4 = true;
	}
	for (int i = 0; i < 26; i++)
	{
		m_overlays[i].Activate(false, true, true);
	}
	BlockInput();
	InitTutorialMode(GOLFDOC()->m_tutorialMode);
	return true;
}

// 983 007c2a51/007c3014: o texto do form abre no topo (ScrollToFirst da barra do edit +0xe8)
struct RbFormMsg983 : public FrForm
{
	void ScrollMsgToFirst() { m_pMsgEdit->GetScrollBar()->ScrollToFirst(); }
};

void CTutorial::OpenTipCaddieForm(const char* text, FRESH_PFN_RESULT pfnResult)
{
	if (m_pReviewForm)
	{
		m_pReviewForm->Close(FrCANCEL, true);
	}
	if (m_pTipForm == NULL && g_view)
	{
		std::string str(text);
		const char* name;
		FrWndManager* manager;
		if (str.size() <= 75)
		{
			manager = g_pFresh->GetManager();
			name = "tip_imageBox";
		}
		else if (str.size() <= 100)
		{
			manager = g_pFresh->GetManager();
			name = "tip_MimageBox";
		}
		else
		{
			manager = g_pFresh->GetManager();
			name = "tip_BimageBox";
		}
		m_pTipForm =
			CreateForm<FrForm>(manager, (FrCmdTarget*)this, name, NULL);
		m_pTipForm->SetMessage(text, false);
		((RbFormMsg983*)m_pTipForm)->ScrollMsgToFirst();
		m_pTipForm->EnableDrag(false);
		FrArea* pArea = DYNAMIC_CAST(FrArea,
			(FrWnd*)m_pTipForm->FindChildByName("pipin_face"));
		if (pArea)
		{
			pArea->SetBgImg(s_data.facials[m_25c]);
		}
		for (unsigned int i = 0; i < 3; i++)
		{
			FrButton* pButton = DYNAMIC_CAST(FrButton,
				(FrWnd*)m_pTipForm->FindChildByName(s_data.answers[i]));
			if (pButton)
			{
				bool focus = i == m_260;
				if (focus)
				{
					m_pTipCaddieWnd = pButton;
				}
				pButton->SetVisible(focus);
				pButton->Enable(false);
			}
		}
		m_2c0 = 50.0f;
		m_2bc = (g_view->GetWidth() - m_pTipForm->GetRect().w) * 0.5f;
		m_2c4 = m_pTipForm->GetRect().w;
		m_2c8 = m_pTipForm->GetRect().h;
		// 983 (007c2f30): grava 0x80 em +0x44 (o 645 usava FWS_FIXED = 0x100)
		FRWND_STYLE(m_pTipForm).Enable((eFrStyle)0x80);
		m_pTipForm->Open(pfnResult,
			WPoint((g_view->GetWidth() - m_pTipForm->GetRect().w) * 0.5f,
				50.0f),
			4);
		m_pTipForm->MoveCursor(s_data.answers[m_260]);
	}
}

bool CTutorial::OnTipCaddieFormResult(int result, FrForm*)
{
	m_pTipForm = NULL;
	m_pTipCaddieWnd = NULL;
	if (result != 2 && result != 0)
	{
		MOUSECURSOR()->Store();
		if (m_missionState == 6)
		{
			m_missionState = 0;
			MissionComplate();
			return true;
		}
		PopMacro();
	}
	return true;
}

bool CTutorial::OnTipCaddieReopenQuizResult(int, FrForm*)
{
	m_pTipForm = NULL;
	m_pTipCaddieWnd = NULL;
	m_state = 4;
	return true;
}

bool CTutorial::OnTipCaddieNoneResult(int, FrForm*)
{
	m_pTipForm = NULL;
	m_pTipCaddieWnd = NULL;
	return true;
}

void CTutorial::OpenMissionForm()
{
	m_25c = 1;
	m_260 = 2;
	if (m_pOverlay == NULL && m_mode >= 0 &&
		m_mode < sizeof(s_data.missionImages) / sizeof(s_data.missionImages[0]))
	{
		m_pOverlay = g_resrcmng->GetOverlay(s_data.missionImages[m_mode], 0);
	}
	if (m_pForm264 == NULL)
	{
		m_pForm264 = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"tuto_mission2", NULL);
		float y = 25.0f;
		float x = g_view->GetWidth() - m_pForm264->GetRect().w - 10.0f;
		if (m_pOverlay)
		{
			x = g_view->GetWidth() - (float)m_pOverlay->GetWidth() + 20.0f;
			y = (float)m_pOverlay->GetHeight() + 25.0f -
				s_data.missionTextX[m_mode];
		}
		FRWND_STYLE(m_pForm264).Enable(FWS_FIXED);
		m_pForm264->SetKeyEvent(true);
		m_pForm264->Open((FRESH_PFN_RESULT)&CTutorial::OnMissionFormResult,
			WPoint(x, y), 6);
	}
	MOUSECURSOR()->ForceMove(g_view->GetWidth() * 0.5f,
		g_view->GetHeight() * 0.5f, -1.0f);
}

bool CTutorial::OnMissionFormResult(int result, FrForm*)
{
	if (m_pTipForm)
	{
		m_pTipForm->Close(true);
	}
	if (m_pQuizDlg)
	{
		m_pQuizDlg->Close(true);
	}
	if (g_resrcmng && m_pOverlay)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = NULL;
	}
	m_pForm264 = NULL;
	if (result == 1)
	{
		HandleMsg(MsgObject(NULL, M983(0x14c), 0, 0, 0, 0, 0));
		return true;
	}
	if (result == 2)
	{
		HandleMsg(MsgObject(NULL, M983(0x14b), 0, 0, 0, 0, 0));
	}
	return true;
}

void CTutorial::OpenQuizForm(int quiz)
{
	if (m_pQuizDlg == NULL)
	{
		if (quiz == 1)
		{
			m_pQuizDlg = CreateForm<FrQuizDlg>(g_pFresh->GetManager(), this,
				"tuto_quiz_ground", NULL);
		}
		else
		{
			m_pQuizDlg = CreateForm<FrQuizDlg>(g_pFresh->GetManager(), this,
				"tuto_quiz_score", NULL);
		}
		m_pQuizDlg->SetupQuiz(quiz);
		m_25c = 1;
		m_260 = 2;
		m_pQuizDlg->SetKeyEvent(true);
		FRWND_STYLE(m_pQuizDlg).Enable(FWS_FIXED);
		m_pQuizDlg->EnableDrag(false);
		m_pQuizDlg->Open((FRESH_PFN_RESULT)&CTutorial::OnQuizFormResult,
			WPoint((g_view->GetWidth() - m_pQuizDlg->GetRect().w) * 0.5f,
				(g_view->GetHeight() - m_pQuizDlg->GetRect().h) * 0.5f + 10.0f),
			3);
		m_pQuizDlg->MoveCursor("cases");
		m_state = 5;
		m_bQuizCorrect = false;
	}
}

bool CTutorial::OnQuizFormResult(int result, FrForm*)
{
	if (m_pTipForm)
	{
		m_pTipForm->Close(true);
	}
	if (m_pQuizDlg)
	{
		m_bQuizCorrect = m_pQuizDlg->IsCorrectAnswer();
	}
	m_pQuizDlg = NULL;
	m_state = 6;
	if (result == 2)
	{
		m_pForm264->Close(FrCANCEL, true);
	}
	return true;
}

void CTutorial::OpenMissionFailedForm()
{
	if (m_pNotifyForm == NULL)
	{
		m_pNotifyForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		m_pNotifyForm->SetMessage(
			K2L_Compatibility("\xb4\xd9\xbd\xc3 \xb5\xb5\xc0\xfc\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?\n\n"
			"(\xbe\xc6\xb4\xcf\xbf\xc0\xb8\xa6 \xb4\xa9\xb8\xa3\xb8\xe9 \xb9\xcc\xbc\xc7\xc0\xbb "
			"\xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed\n\xb9\xdb\xc0\xb8\xb7\xce \xb3\xaa\xb0\xa9\xb4\xcf\xb4\xd9)"),
			false);
		((RbFormMsg983*)m_pNotifyForm)->ScrollMsgToFirst();
		m_pNotifyForm->EnableDrag(false);
		m_pNotifyForm->Open(
			(FRESH_PFN_RESULT)&CTutorial::OnMissionFailedFormResult,
			WPoint((g_view->GetWidth() - m_pNotifyForm->GetRect().w) * 0.5f,
				180.0f),
			2);
		m_pNotifyForm->MoveCursor("ok");
	}
}

bool CTutorial::OnMissionFailedFormResult(int result, FrForm*)
{
	m_pNotifyForm = NULL;
	MOUSECURSOR()->Store();
	if (m_pTipForm)
	{
		m_pTipForm->Close(true);
	}
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (result == 1)
		{
			m_state = 1;
			if (GOLFDOC()->m_tutorialMode == 7 ||
				GOLFDOC()->m_tutorialMode == 6)
			{
				m_state = 4;
				return true;
			}
			PrepareMission();
			OpenMissionForm();
			SendMsg(this, "Screen", M983(0x1bc), 2, 0, 0, 0);
		}
		else
		{
			if (result != 2)
			{
				return true;
			}
			// 983 (007c2030): sem o M983(0x150) ao "TutorialMain" e sem FromGolfToLobbySettings; volta ao widget do lobby
			m_b2b4 = false;
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
				(int)"CTutorialWidget", (int)"TOPPAGE", 0, 0);
			FreeInput();
			return true;
		}
	}
	SendMsg(this, "Screen", M983(0x1ba), 1, 0, 0, 0);
	FreeInput();
	return true;
}

void CTutorial::PushMacroCamera()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "100", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "101", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_1")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_1"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_1",
		0, 1);
	PushMacro(MACRO_08, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "102", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_2")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_2"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_2",
		-1, 1);
	PushMacro(MACRO_0E, "4 ok", 0, 1);
	PushMacro(MACRO_09, "103", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_3")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_3"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_3",
		-1, 1);
	PushMacro(MACRO_0E, "5 ok", 0, 1);
	PushMacro(MACRO_09, "104", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_4")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_4"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_4",
		-1, 1);
	PushMacro(MACRO_0E, "6 ok", 0, 1);
	PushMacro(MACRO_09, "105", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_5")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_5"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_5",
		-1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "106", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d 1", M983(0x1d8),
			GetPoint("\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
					 "1_6")),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_6"),
		0, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"1_6",
		-1, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "107", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
}

void CTutorial::PushMacroClub()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "200", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "201", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "202", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "203", -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Screen %d %d %d", 568, (int)&m_240, (int)&m_244), 0, 1);
	PushMacro(MACRO_11,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
			RB_DBL(rb_real_4050000000000000, 64.0),
			RB_DBL(rb_real_4050000000000000, 64.0), 1.0),
		0, 1);
	PushMacro(MACRO_00, NULL, 1500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 30, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "204", -1, 1);
	float w = g_view->GetWidth();
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
			(double)(w * 0.5f - 32.0f),
			(double)(g_view->GetHeight() * 0.48333333f - 32.0f)),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
			RB_DBL(rb_real_4050000000000000, 64.0),
			RB_DBL(rb_real_4050000000000000, 64.0), 1.0),
		0, 1);
	PushMacro(MACRO_00, NULL, 1500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "205", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "206", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 10,
			(double)(m_2bc + m_2c4 - 128.0f), (double)(m_2c0 + m_2c8 + 20.0f)),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 10,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		0, 1);
	PushMacro(MACRO_01, "PAGE_DOWN", 200, 5);
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_01, "PAGE_UP", 200, 5);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 10), 300, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "207", -1, 1);
	PushMacro(MACRO_00, NULL, 30, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "208", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 9,
			m_2bc + m_2c4 + 20.0f, m_2c0 + m_2c8 - 88.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 9,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_01, "NUM_0", 0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "209", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 1,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 1,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		200, 1);
	PushMacro(MACRO_00, NULL, 1500, 1);
	PushMacro(MACRO_02, "LSHIFT", 0, 1);
	PushMacro(MACRO_01,
		"\xbb"
		"\xf3"
		"",
		1, 50);
	PushMacro(MACRO_00, NULL, 800, 1);
	PushMacro(MACRO_01,
		"\xc7"
		"\xcf"
		"",
		1, 50);
	PushMacro(MACRO_03, NULL, 500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 9), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 1), 300, 1);
	PushMacro(MACRO_01, "NUM_0", 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "210", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xc1"
		"\xc2"
		"",
		250, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "211", -1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d %d", 643, 1), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr("Caddie %d %d", M983(0x1d7),
			"\xc4\xb3\xb5\xf0"
			"1_6"),
		0, 1);
	PushMacro(MACRO_15, "\xb3\xa1\xc1\xa1", 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "212", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroShot()
{
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "300", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"3",
		0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "301", -1, 1);
	PushMacro(MACRO_00, NULL, 1500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "302", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 3,
			m_2c4 + m_2bc - 128.0f, m_2c8 + m_2c0 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 3,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d %d", 162, 490, 140), -1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d %d", 165, (int)&m_240), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1", 163), 0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "303", -1, 1);
	PushMacro(MACRO_11,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
			(double)ScaleByViewHeight983(g_view, 64.0f),
			(double)ScaleByViewHeight983(g_view, 64.0f), 1.0),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 0", 163), 0, 1);
	PushMacro(MACRO_00, NULL, -1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1", 163), 0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "304", -1, 1);
	// 983: posicao/tamanho do indicador pela medida do medidor (d_default_pangya_x, d_driver_gauge_y/_h)
	{
		CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
			(float)(g_view->GetHeight() / 480.0));
		CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
		CUiFrameSetManager::NamedDim* dimX = ui->FindNamedDim("d_default_pangya_x");
		CUiFrameSetManager::NamedDim* dimY = ui->FindNamedDim("d_driver_gauge_y");
		CUiFrameSetManager::NamedDim* dimH = ui->FindNamedDim("d_driver_gauge_h");
		if (dimX != NULL && dimY != NULL && dimH != NULL)
		{
			float x = (float)dimX->Calc(scl);
			float y = (float)dimY->Calc(scl);
			float h = (float)dimH->Calc(scl);
			float a = ScaleByViewHeight983(g_view, 64.0f);
			float b = ScaleByViewHeight983(g_view, 64.0f);
			PushMacro(MACRO_13,
				MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
					x - 0.5 * b * a, y + h * 0.5 - b),
				0, 1);
			PushMacro(MACRO_10,
				MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
					(double)a, (double)b, 0.5),
				0, 1);
		}
	}
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 3), 0, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 0", 163), 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 0", 162), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "305", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "306", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroGreen()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "400", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "401", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "402", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 4,
			m_2c4 + m_2bc - 128.0f, m_2c8 + m_2c0 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 4,
			RB_DBL(rb_real_4060000000000000, 128.0),
			RB_DBL(rb_real_4056000000000000, 88.0), 0.5),
		0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "403", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 4), 300, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 1", 162), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d", 162, 350), -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 0", 162), 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "404", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "405", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 1", 644), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "406", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xbf"
		"\xec"
		"",
		295, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 0", 644), 0, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 1", 162), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d", 162, 350), -1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_16, MakeStr("Grid %d 0", 162), 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "407", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xbf"
		"\xec"
		"",
		100, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "408", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

// HACK: the original inlines CGolfDoc::GetCurrentPlayer() here without emitting its body, ours
// emitted an out-of-line copy (0x10 bytes); a dllimport __forceinline accessor is never emitted.
__declspec(dllimport) __forceinline unsigned char RbCurrentPlayer(CGolfDoc* doc)
{
	return doc->m_currentPlayer;
}

void CTutorial::PushMacroGauge()
{
	GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->gauge = 30.0f;
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "600", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "601", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 559), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, 3.0, 45.0), 0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 76.0, 22.0,
			0.5),
		0, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "602", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_HLJLLFBF, "GolfRule %d 1 %d %d"), 162, 470, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "603", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "604", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 5,
			m_2c4 + m_2bc - 128.0f, m_2c8 + m_2c0 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 5,
			RB_DBL(rb_real_4060000000000000, 128.0),
			RB_DBL(rb_real_4056000000000000, 88.0), 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 5), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr("Player %d", M983(0x11d)), 2000, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_HLJLLFBF, "GolfRule %d 1 %d %d"), 162, 495, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "605", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "606", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroGround()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "700", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "701", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "702", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 9,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 30.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 9,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 9), 300, 1);
	PushMacro(MACRO_01, "NUM_0", 500, 1);
	PushMacro(MACRO_05,
		MakeStr("%f %f", g_view->GetWidth() * 0.5f - 50.0f,
			g_view->GetHeight() * 0.5f - 30.0f),
		500, 1);
	PushMacro(MACRO_04, "0", 500, 1);
	PushMacro(MACRO_05,
		MakeStr("%f %f", g_view->GetWidth() * 0.5f + 50.0f,
			g_view->GetHeight() * 0.5f + 50.0f),
		500, 1);
	PushMacro(MACRO_04, "0", 500, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "703", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 22,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 22, 256.0,
			RB_DBL(rb_real_4060000000000000, 128.0), 0.0),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 22), 300, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "704", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 20,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 20, 256.0,
			RB_DBL(rb_real_4060000000000000, 128.0), 0.0),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 20), 300, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "705", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 21,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 21, 256.0,
			RB_DBL(rb_real_4060000000000000, 128.0), 0.0),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 21), 300, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "706", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 21,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 21, 256.0,
			RB_DBL(rb_real_4060000000000000, 128.0), 0.0),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 21), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "707", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_01, "NUM_0", 500, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "708", -1, 1);
	PushMacro(MACRO_14, "1", -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "709", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroItemUse()
{
	m_vals[0] = 0x18000004;
	m_vals[1] = 0x18000004;
	m_vals[2] = 0x18000004;
	GOLFDOC()->GetPlayer(0)->itemNum = 3;
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "800", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "801", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "802", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 6,
			m_2c4 + m_2bc - 128.0f, m_2c8 + m_2c0 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 6,
			RB_DBL(rb_real_4060000000000000, 128.0),
			RB_DBL(rb_real_4056000000000000, 88.0), 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 6), 300, 1);
	PushMacro(MACRO_01, "ITEMSLOT", 0, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "803", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 7, 80.0,
			g_view->GetHeight() - 208.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 7,
			RB_DBL(rb_real_4060000000000000, 128.0),
			RB_DBL(rb_real_4056000000000000, 88.0), 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 7), 300, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_01,
		"\xbf"
		"\xec"
		"",
		500, 1);
	PushMacro(MACRO_01,
		"\xbf"
		"\xec"
		"",
		500, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "804", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 8, 80.0,
			g_view->GetHeight() - 208.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 8,
			RB_DBL(rb_real_4060000000000000, 128.0),
			RB_DBL(rb_real_4056000000000000, 88.0), 0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 8), 300, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		2000, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "805", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_HLJLLFBF, "GolfRule %d 1 %d %d"), 162, 470, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "806", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "807", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "808", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroScore()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "500", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "501", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 560), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "502", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, 165.0, 8.0), 0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 50.0, 20.0,
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	PushMacro(MACRO_00, NULL, 300, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "503", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "504", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "505", -1, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "506", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 559), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, 2.0, 55.0), 0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 70.0, 38.0,
			0.5),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "507", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "508", -1, 1);
	PushMacro(MACRO_14, "0", -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "509", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroHoleIn()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "900", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "901", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "902", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 560), 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, 94.0, 8.0), 0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 130.0, 20.0,
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "903", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 559), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 558), 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, 4.0, 56.0), 0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 65.0, 40.0,
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "905", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, -1, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "906", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroWindReading()
{
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"3",
		0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1000", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1001", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 650), 0, 1);
	// 983: centro do indicador de vento pelas medidas do layout (d_wind_direction_center_x/_y)
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* dimX = ui->FindNamedDim("d_wind_direction_center_x");
	CUiFrameSetManager::NamedDim* dimY = ui->FindNamedDim("d_wind_direction_center_y");
	float cx = 0.0f;
	float cy = 0.0f;
	if (dimX != NULL && dimY != NULL)
	{
		cx = dimX->Calc(scl);
		cy = dimY->Calc(scl);
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1002", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1003", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 617, 1), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 618, 0), 0, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	{
		float a = ScaleByViewHeight983(g_view, 80.0f);
		float b = ScaleByViewHeight983(g_view, 80.0f);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 500, 1);
	int i;
	for (i = 0; i <= 360; i += 5)
	{
		PushMacro(MACRO_16, MakeStr("Screen %d %d", 618, i), 100, 1);
	}
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1004", -1, 1);
	CUiFrameSetManager::NamedPoint* pSpeed = ui->FindNamedPoint("p_wind_speed_center");
	if (pSpeed != NULL)
	{
		WPoint pt;
		pSpeed->Calc(scl, pt);
		float a = ScaleByViewHeight983(g_view, 40.0f);
		float b = ScaleByViewHeight983(g_view, 40.0f);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				pt.x - 0.5 * a, pt.y - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	for (i = 0; i < 9; i++)
	{
		PushMacro(MACRO_16, MakeStr("Screen %d %d", 617, i + 1), 600, 1);
	}
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1005", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 618, 90), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1006", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_HLJLLFBF, "GolfRule %d 1 %d %d"), 162, 470, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1007", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1008", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1009", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroBallMove()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1100", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1101", -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbb"
		"\xf3"
		"",
		800, 1);
	PushMacro(MACRO_02,
		"\xc1"
		"\xc2"
		"",
		800, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1102", -1, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d", 649), 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
			g_view->GetWidth() - 400.0f, 1.0),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, 40.0, 30.0,
			0.5),
		0, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_07, MakeStr("unplayable %d", M983(0x24a)), 0, 1);
	PushMacro(MACRO_1E, "0", 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 24,
			g_view->GetWidth() - 395.0f, 35.0),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24, 73.0, 33.0,
			0.0),
		300, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24,
			80.30000305175781, 36.29999923706055, 0.0),
		100, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24, 73.0, 33.0,
			0.0),
		1000, 1);
	PushMacro(MACRO_12, MakeStr("%d", 24), 0, 1);
	PushMacro(MACRO_1F, NULL, 0, 1);
	PushMacro(MACRO_04, "0", 0, 1);
	PushMacro(MACRO_1E, "0", 800, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1103", -1, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 1), 300, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 1), 300, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 578, 1), 300, 1);
	PushMacro(MACRO_1F, NULL, 0, 1);
	PushMacro(MACRO_1E, "2", 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1E, "0", 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1104", -1, 1);
	PushMacro(MACRO_1F, NULL, 0, 1);
	PushMacro(MACRO_1E, "2", 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 11,
			m_2c4 + m_2bc - 128.0f, m_2c8 + m_2c0 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 11,
			RB_DBL(rb_real_4060000000000000, 128.0), 88.0, 0.5),
		500, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 11), 100, 1);
	PushMacro(MACRO_06,
		MakeStr("%f %f",
			(double)(g_view->GetWidth() - g_view->GetWidth() * 0.125f),
			(double)(g_view->GetHeight() * 0.5f + 30.0f)),
		0, 1);
	PushMacro(MACRO_1E, "0", 0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 24,
			(double)(g_view->GetWidth() - g_view->GetWidth() * 0.125f),
			(double)(g_view->GetHeight() * 0.5f + 45.0f)),
		0, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 25,
			g_view->GetWidth() - g_view->GetWidth() * 0.125f - 245.0f,
			g_view->GetHeight() * 0.5f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 25, 195.0, 106.0,
			0.0),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24, 73.0, 33.0,
			0.0),
		300, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24,
			80.30000305175781, 36.29999923706055, 0.0),
		100, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 24, 73.0, 33.0,
			0.0),
		1000, 1);
	PushMacro(MACRO_12, MakeStr("%d", 24), 0, 1);
	PushMacro(MACRO_12, MakeStr("%d", 25), 0, 1);
	PushMacro(MACRO_02, "LSHIFT", 100, 1); // 983 007c7540: LSHIFT, botao direito travado na demonstracao
	PushMacro(MACRO_1F, NULL, 0, 1);
	PushMacro(MACRO_1E, "2", 0, 1);
	PushMacro(MACRO_04, "0", 100, 1);
	PushMacro(MACRO_04, "0", 300, 1);
	PushMacro(MACRO_03, NULL, 500, 1);
	PushMacro(MACRO_1E, "0", 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1105", -1, 1);
	PushMacro(MACRO_1F, NULL, 0, 1);
	PushMacro(MACRO_1E, "2", 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16,
		MakeStr("GolfRule %d %d", 164, "\xbd\xc3\xc0\xdb\xc1\xa1_Unplayable",
			0),
		0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d", 649), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1106", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroBallSlope()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1200", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"3",
		0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1201", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1202", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1203", -1, 1);
	// 983: centro da bola pelas medidas do layout (d_ball_center_x/_y)
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* dimX = ui->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* dimY = ui->FindNamedDim("d_ball_center_y");
	float cx = 0.0f;
	float cy = 0.0f;
	float a = ScaleByViewHeight983(g_view, 103.0f);
	float b = ScaleByViewHeight983(g_view, 103.0f);
	if (dimX != NULL && dimY != NULL)
	{
		cx = dimX->Calc(scl);
		cy = dimY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, cx - 0.5 * a,
				cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, (double)a,
				(double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d %d", 164, "Slope_Right", 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1204", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, cx - 0.5 * a,
			cy - 0.5 * b),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, (double)a,
			(double)b, 0.5),
		0, 1);
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d %d", 162, 470, 143), -1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 0", 162), 0, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d %d", 164, "Slope_Left", 0), 0, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1205", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0, cx - 0.5 * a,
			cy - 0.5 * b),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0, (double)a,
			(double)b, 0.5),
		0, 1);
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d %d", 162, 470, 135), -1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 0", 162), 0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1206", -1, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16, MakeStr("Player %d %d %d %d %d", 450, 5, -1, -1), 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d %d", 164, "Slope_Right", 0), 300, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_16, MakeStr("Player %d %d %d", 330, 0x10000000, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1207", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroHitPoint()
{
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1300", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1301", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1302", -1, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 1), 500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 0), 500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 1), 500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 0), 500, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 1), 500, 1);
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* dimBX = ui->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* dimBY = ui->FindNamedDim("d_ball_center_y");
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1303", -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d", M983(0x16a)), 0, 1);
	PushMacro(MACRO_13,
		MakeStr("%d %.1f %.1f", 12, m_2bc + m_2c4 - 128.0f,
			m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10, MakeStr("%d %.1f %.1f %.1f", 12, 137.0, 105.0, 1.0), 0,
		1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x16b), 2, 3, 1), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 1), 0, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 12), 300, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1304", -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x16b), 1, 0, 0), 300, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 579, 0), 500, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, -0x14), 0, 1);
	PushMacro(MACRO_13,
		MakeStr("%d %.1f %.1f", 13, m_2bc + m_2c4 - 128.0f,
			m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10, MakeStr("%d %.1f %.1f %.1f", 13, 137.0, 105.0, 1.0), 0,
		1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 13), 300, 1);
	PushMacro(MACRO_0E, "7 ok", 0, 1);
	PushMacro(MACRO_09, "1305", -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, 20), 0, 1);
	PushMacro(MACRO_13,
		MakeStr("%d %.1f %.1f", 14, m_2bc + m_2c4 - 128.0f,
			m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10, MakeStr("%d %.1f %.1f %.1f", 14, 137.0, 105.0, 1.0), 0,
		1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 14), 300, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1306", -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 0), 0, 1);
	PushMacro(MACRO_13,
		MakeStr("%d %.1f %.1f", 15, m_2bc + m_2c4 - 128.0f,
			m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10, MakeStr("%d %.1f %.1f %.1f", 15, 137.0, 105.0, 1.0), 0,
		1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, -0x14, 0), 1000, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 20, 0), 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 15), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1307", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, 0), 0, 1);
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacroHitPointMission();
}

void CTutorial::PushMacroHitPointMission()
{
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 579, 1), 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 2, 0, -0x14), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 2), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 1), 300, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 2, -0x14, 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 0), 1000, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 2), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 1), 300, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 2, 0, 20), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 0), 1000, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 2), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 1), 300, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 2, 20, 0), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 0), 1000, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 2), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1308", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroPowerSpin()
{
	PushMacro(MACRO_15,
		"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
		"3",
		0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 538), 0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1400", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1401", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1402", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 537), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 3), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, -0x14), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1403", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 13,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 13, 137.0, 105.0,
			1.0),
		0, 1);
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* dimBX = ui->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* dimBY = ui->FindNamedDim("d_ball_center_y");
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 13), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "310", 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d %d ", 162, 490, 140), -1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1404", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xbb"
		"\xf3"
		"",
		0, 1);
	PushMacro(MACRO_1C, "142", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_KLEMCLIN, "GolfRule %d 1"), 163),
		0, 1);
	CUiFrameSetManager::NamedDim* dimGX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* dimGY = ui->FindNamedDim("d_driver_gauge_y");
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 53.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 123.0f),
				(double)ScaleByViewHeight983(g_view, 45.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 300, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 16,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 16, 137.0, 105.0,
			1.0),
		0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192),
		50, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1405", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 16), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1406", -1, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 58.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 120.0f),
				(double)ScaleByViewHeight983(g_view, 103.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 100, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 163),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		300, 1);
	PushMacro(MACRO_03, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		100, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 3), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 0, 20), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1407", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 14,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 14, 137.0, 105.0,
			1.0),
		0, 1);
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 14), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "310", 0, 1);
	PushMacro(MACRO_16, MakeStr("GolfRule %d 1 %d %d ", 162, 490, 140), -1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1408", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xc7"
		"\xcf"
		"",
		0, 1);
	PushMacro(MACRO_1C, "142", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_KLEMCLIN, "GolfRule %d 1"), 163),
		0, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 53.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 123.0f),
				(double)ScaleByViewHeight983(g_view, 45.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 300, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 17,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 17, 137.0, 105.0,
			1.0),
		0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192),
		50, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1409", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 17), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1410", -1, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 58.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 120.0f),
				(double)ScaleByViewHeight983(g_view, 103.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 100, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 163),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		300, 1);
	PushMacro(MACRO_03, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		1000, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 4), 0, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1411", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_GNCGCIEM, "Player %d %d %d %d %d"), M983(0x157), -1, 28,
			-1),
		0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d %d", 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1412", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::PushMacroPowerCurve()
{
	PushMacro(MACRO_15, "\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1_curve", 0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 538), 0, 1);
	PushMacro(MACRO_0E, "1 ok", 0, 1);
	PushMacro(MACRO_09, "1500", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1501", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1502", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_18, "0", 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_GNCGCIEM, "Player %d %d %d %d %d"), M983(0x157), -1, -1,
			7),
		0, 1);
	PushMacro(MACRO_16, MakeStr("Screen %d", 537), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 3), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, 7, 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1503", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 15,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 15,
			RB_DBL(rb_real_4061200000000000, 137.0),
			RB_DBL(rb_real_405a400000000000, 105.0),
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* dimBX = ui->FindNamedDim("d_ball_center_x");
	CUiFrameSetManager::NamedDim* dimBY = ui->FindNamedDim("d_ball_center_y");
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 15), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, RB_LIT(rb_C_03MGMKMOPI_310_AA, "310"), 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_PICCFLPM, "GolfRule %d 1 %d %d "), 162, 490, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1504", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xbf"
		"\xec"
		"",
		0, 1);
	PushMacro(MACRO_1C, RB_LIT(rb_C_03FIDOKGBK_142_AA, "142"), 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_KLEMCLIN, "GolfRule %d 1"), 163),
		0, 1);
	CUiFrameSetManager::NamedDim* dimGX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* dimGY = ui->FindNamedDim("d_driver_gauge_y");
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 53.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 123.0f),
				(double)ScaleByViewHeight983(g_view, 45.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 300, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 18,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 18,
			RB_DBL(rb_real_4061200000000000, 137.0),
			RB_DBL(rb_real_405a400000000000, 105.0),
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192),
		50, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1505", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 18), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1506", -1, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 58.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 120.0f),
				(double)ScaleByViewHeight983(g_view, 103.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 100, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 163),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		300, 1);
	PushMacro(MACRO_03, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		100, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 3), 0, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d %d %d", M983(0x165), 1, -7, 0), 0, 1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1507", -1, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 15,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 15,
			RB_DBL(rb_real_4061200000000000, 137.0),
			RB_DBL(rb_real_405a400000000000, 105.0),
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	if (dimBX != NULL && dimBY != NULL)
	{
		float a = ScaleByViewHeight983(g_view, 103.0f);
		float b = ScaleByViewHeight983(g_view, 103.0f);
		float cx = dimBX->Calc(scl);
		float cy = dimBY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				cx - 0.5 * a, cy - 0.5 * b),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)a, (double)b, 0.5),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 1000, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 15), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, RB_LIT(rb_C_03MGMKMOPI_310_AA, "310"), 0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_PICCFLPM, "GolfRule %d 1 %d %d "), 162, 490, 140),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		-1, 1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_0E, "2 ok", 0, 1);
	PushMacro(MACRO_09, "1508", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_02,
		"\xc1"
		"\xc2"
		"",
		0, 1);
	PushMacro(MACRO_1C, RB_LIT(rb_C_03FIDOKGBK_142_AA, "142"), 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_1B, NULL, 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_KLEMCLIN, "GolfRule %d 1"), 163),
		0, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 53.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 123.0f),
				(double)ScaleByViewHeight983(g_view, 45.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 300, 1);
	PushMacro(MACRO_13,
		MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 19,
			m_2bc + m_2c4 - 128.0f, m_2c0 + m_2c8 + 20.0f),
		0, 1);
	PushMacro(MACRO_10,
		MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 19,
			RB_DBL(rb_real_4061200000000000, 137.0),
			RB_DBL(rb_real_405a400000000000, 105.0),
			RB_DBL(rb_real_3ff0000000000000, 1.0)),
		0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192),
		50, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1509", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 19), 300, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1510", -1, 1);
	if (dimGX != NULL && dimGY != NULL)
	{
		float gx = dimGX->Calc(scl);
		float gy = dimGY->Calc(scl);
		PushMacro(MACRO_13,
			MakeStr(RB_LIT(rb_str_OJDKBMGL, "%d %.1f %.1f"), 0,
				gx - ScaleByViewHeight983(g_view, 30.0f),
				gy - ScaleByViewHeight983(g_view, 58.0f)),
			0, 1);
		PushMacro(MACRO_10,
			MakeStr(RB_LIT(rb_str_KADLIFB, "%d %.1f %.1f %.1f"), 0,
				(double)ScaleByViewHeight983(g_view, 120.0f),
				(double)ScaleByViewHeight983(g_view, 103.0f), 1.0),
			0, 1);
	}
	PushMacro(MACRO_00, NULL, 100, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_12, MakeStr("%d", 0), 300, 1);
	PushMacro(MACRO_1C, "0", 0, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 191), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_PPMNKAHM, "GolfRule %d"), 192), 0,
		1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 163),
		-1, 1);
	PushMacro(MACRO_1D, NULL, 0, 1);
	PushMacro(MACRO_01,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		1, 1);
	PushMacro(MACRO_17, "0", 0, 1);
	PushMacro(MACRO_02,
		"\xbd"
		"\xba"
		"\xc6"
		"\xe4"
		"\xc0"
		"\xcc"
		"\xbd"
		"\xba"
		"\xb9"
		"\xd9"
		"",
		300, 1);
	PushMacro(MACRO_03, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr(RB_LIT(rb_str_LCFHBKMM, "GolfRule %d 0"), 162),
		1000, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1511", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_16, MakeStr("Spin %d %d", M983(0x169), 4), 0, 1);
	PushMacro(MACRO_03, NULL, 0, 1);
	PushMacro(MACRO_00, NULL, 500, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_GNCGCIEM, "Player %d %d %d %d %d"), M983(0x157), -1, 28,
			7),
		0, 1);
	PushMacro(MACRO_16,
		MakeStr(RB_LIT(rb_str_DCFJPHFM, "Screen %d %d"), 645, 1), 0, 1);
	PushMacro(MACRO_08, NULL, 0x4e20, 1);
	PushMacro(MACRO_00, NULL, 2000, 1);
	PushMacro(MACRO_0E, "3 ok", 0, 1);
	PushMacro(MACRO_09, "1512", -1, 1);
	PushMacro(MACRO_0F, NULL, -1, 1);
	PushMacro(MACRO_1A, NULL, -1, 1);
}

void CTutorial::InitTutorialMode(int mode)
{
	char start[32];
	char end[32];

	ClearMacro();
	GetPVS().ResetHoleData();
	m_25c = 1;
	m_260 = 0;
	m_b2d4 = false;
	m_b21c = false;
	m_state = 7;
	m_macroWait = -1;
	m_pad20c[0] = 0;
	m_b2b4 = false;
	m_228 = 0;
	m_22c = 0;
	start[0] = '\0';
	end[0] = '\0';
	GOLFDOC()->GetPlayer(0)->totalPang = 0;
	m_2cc = 1.5f;
	GOLFDOC()->GetPlayer(0)->bunker = 0;
	sPlayerData* player = GOLFDOC()->GetPlayer(0);
	player->stroke[GOLFDOC()->m_currentHole - 1] = 1;
	GOLFDOC()->m_bTutorialPin983 = 0;
	memset(m_vals, 0, sizeof(m_vals));
	GOLFDOC()->GetPlayer(0)->itemNum = 0;
	{
		sPlayerData* p0 = GOLFDOC()->GetPlayer(0);
		p0->item = 0;
		p0->windAbility983 = 0;
	}
	GOLFDOC()->GetPlayer(0)->gauge = 0.0f;
	SendMsg(this, "Screen", M983(0x208), 0, 0, 0, 0);
	PriviewOverlay();
	GOLFDOC()->m_tutorialMode = mode;
	switch (GOLFDOC()->m_tutorialMode)
	{
	case 0:
		m_missionState = 0;
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		PushMacroCamera();
		break;
	case 1:
		strcpy(start,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"2");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"1_6");
		PushMacroClub();
		break;
	case 2:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroShot();
		break;
	case 6:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroGround();
		break;
	case 3:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroGauge();
		break;
	case 4:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1_\xb1\xd7\xb8\xb0");
		PushMacroGreen();
		break;
	case 5:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroItemUse();
		break;
	case 7:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroScore();
		break;
	case 8:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroHoleIn();
		break;
	case 11:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		Wind().Init(7, 0xcc, 0, 1);
		Wind().SetIntensity(0.0f);
		PushMacroWindReading();
		break;
	case 9:
		// HACK: strcpy of the literal (inline dword copies), spelled as memcpy so it can read the shared static (RB_LIT)
		memcpy(start,
			RB_LIT(rb_str_GNKGMOGB, "\xbd\xc3\xc0\xdb\xc1\xa1_Unplayable"), 18);
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroBallMove();
		break;
	case 10:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroBallSlope();
		break;
	case 12:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroHitPoint();
		break;
	case 13:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end,
			"\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1"
			"3");
		PushMacroPowerSpin();
		break;
	case 14:
		strcpy(start, "\xbd\xc3\xc0\xdb\xc1\xa1");
		strcpy(end, "\xb3\xab\xc7\xcf\xc1\xf6\xc1\xa1_curve");
		PushMacroPowerCurve();
		break;
	}
	BlockInput();
	GOLFDOC()->m_bTutorialPin983 = 0;
	MOUSECURSOR()->SetActive(false);
	SendMsg(this, "Screen", M983(0x1ba), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1bb), m_mode + 1, 0, 0, 0);
	SendMsg(this, "GolfRule", M983(0x89), (int)start, end[0] != '\0' ? (int)end : 0,
		0, 0);
	CChatMsg::Instance()->ClearHistory();
}

// HACK: 96.0f is defined in autotarget.cpp as a function-local static aliased to
// __real@42c00000 (see giftalarmdlg.cpp); the literal here would emit a competing COMDAT.
extern "C" __declspec(selectany) const float rb_real_42c00000 = 96.0f; // C64.4: o estático de função do rebang não existe mais

bool CTutorial::CheckCameraTarget()
{
	WVector d = GOLFDOC()->m_tutorialPin983 - g_camera.pivot;
	d.y += 6.0f;
	float dot = g_camera.yz * d.y + g_camera.xz * d.x + g_camera.zz * d.z;
	if (!((*(unsigned int*)&dot & 0x80000000 ? -dot : dot) < s_data.epsilon) &&
		dot > rb_real_42c00000 && dot < 288.0f)
	{
		float angle = acosf(Between(-1.0f, dot / d.Magnitude(), 1.0f));
		if (Abs(angle) < 0.05235988f)
		{
			m_missionState++;
			PopMacro();
		}
	}
	return false;
}

bool CTutorial::CheckClubTarget()
{
	WVector& dir = GOLFDOC()->m_cameraData.mat.za;
	float range = GolfClub().GetRange() * 3.2f;
	WVector offset;
	offset.x = range * dir.x;
	offset.z = range * dir.z;
	CGolfBall& ball = GolfBall();
	WVector target;
	target.x = offset.x + ball.m_pos.x;
	target.z = offset.z + ball.m_pos.z;
	WVector& pin = GOLFDOC()->m_tutorialPin983;
	WVector diff;
	diff.x = pin.x - target.x;
	diff.z = pin.z - target.z;
	if (sqrtf(diff.z * diff.z + diff.x * diff.x) < 16.0f)
	{
		return true;
	}
	return false;
}

WVector* CTutorial::GetPoint(const char* name)
{
	return &GOLFDOC()->m_pPolySoup->FindPoint(name)->pos;
}

void CTutorial::PushMacro(eMacroType type, const char* str, int value,
	int count)
{
	sMacro macro;
	macro.type = type;
	macro.value = value;
	if (str == NULL)
	{
		macro.str[0] = '\0';
	}
	else
	{
		strcpy(macro.str, str);
	}
	for (int i = 0; i < count; i++)
	{
		m_macros.push(macro);
	}
}

void CTutorial::PopSpecial()
{
	if (!m_macros.empty() && m_macros.front().type == MACRO_1B)
	{
		m_state = 7;
		m_macroWait = m_macros.front().value;
		m_macros.pop();
		if (!m_macros.empty() && m_macroWait == 0)
		{
			PopMacro();
		}
	}
}

void CTutorial::PopMacro()
{
	if (m_macros.empty())
	{
		return;
	}
	if (m_pNotifyForm != NULL)
	{
		return;
	}
	m_state = 7;
	m_macroWait = m_macros.front().value;
	if (m_macros.front().type == MACRO_1B)
	{
		return;
	}
	switch (m_macros.front().type)
	{
	case MACRO_01:
		g_input->SetFakeInput(m_macros.front().str, m_b21c);
		m_b21c = false;
		break;
	case MACRO_02:
	{
		const char* src = m_macros.front().str;
		char* dst = m_pad20c;
		char c;
		do
		{
			c = *src++;
			*dst++ = c;
		} while (c != '\0');
		g_input->SetFakeInput(m_pad20c, m_b21c);
		m_b21c = false;
	}
	break;
	case MACRO_03:
		m_pad20c[0] = '\0';
		break;
	case MACRO_04:
	{
		int button;
		sscanf(m_macros.front().str, "%d", &button);
		g_input->SetFakeInput((eButton)button);
	}
	break;
	case MACRO_05:
	{
		float x, y;
		sscanf(m_macros.front().str, "%f %f", &x, &y);
		MOUSECURSOR()->ForceMove(x, y, (float)m_macroWait * 0.001f);
	}
	break;
	case MACRO_06:
	{
		float x, y;
		sscanf(m_macros.front().str, "%f %f", &x, &y);
		MOUSECURSOR()->ForceMove(x, y, -1.0f);
	}
	break;
	case MACRO_07:
	{
		char name[32];
		float* pos;
		memset(name, 0, sizeof(name));
		sscanf(m_macros.front().str, "%s", name);
		pos = NULL;
		SendMsg(this, "GolfRule", M983(0x24a), (int)name, (int)&pos, 0, 0);
		if (pos != NULL)
		{
			MOUSECURSOR()->ForceMove(((FrWnd*)pos)->GetRect().x + 5.0f, ((FrWnd*)pos)->GetRect().y + 5.0f, -1.0f); // 983: FrWnd::m_rect (+0x34 no VC10; B1471: 0x30 no cl 14.44)
		}
	}
	break;
	case MACRO_08:
		if (m_macroWait > 0)
		{
			m_b2b4 = true;
			m_2b8 = m_macroWait + 4000;
			m_macroWait = -1;
		}
		m_state = 1;
		SendMsg(this, "Screen", M983(0x1ba), 1, 0, 0, 0);
		PrepareMission();
		OpenMissionForm();
		break;
	case MACRO_09:
	{
		int index;
		m_state = 0;
		MOUSECURSOR()->SetActive(true);
		sscanf(m_macros.front().str, "%d", &index);
		if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
		{
			m_pForm248 =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "tip", NULL);
			m_pForm248->SetMessage(m_lines[index].c_str(), false);
			((RbFormMsg983*)m_pForm248)->ScrollMsgToFirst();
			m_pForm248->EnableDrag(false);
			m_pForm248->Open((FRESH_PFN_RESULT)&CTutorial::OnTipFormResult,
				WPoint((g_view->GetWidth() - m_pForm248->GetRect().w) * 0.5f,
					80.0f),
				FrENTER | FrMODALESS);
			m_pForm248->MoveCursor("ok");
			FrWnd* pArrow = m_pForm248->FindChildByName("harrow");
			if (pArrow)
			{
				pArrow->SetVisible(false);
			}
			pArrow = m_pForm248->FindChildByName("varrow");
			if (pArrow)
			{
				pArrow->SetVisible(false);
			}
		}
		else
		{
			m_macroWait = 0;
			OpenTipCaddieForm(m_lines[index].c_str(),
				(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieFormResult);
		}
	}
	break;
	case MACRO_0A:
	{
		std::string text;
		int index;
		float x, y, w, h;
		m_state = 0;
		MOUSECURSOR()->SetActive(true);
		sscanf(m_macros.front().str, "%d %f %f %f %f", &index, &x, &y, &w, &h);
		m_macroWait = 0;
		OpenTipCaddieForm(m_lines[index].c_str(),
			(FRESH_PFN_RESULT)&CTutorial::OnTipCaddieFormResult);
	}
	break;
	case MACRO_0B:
	{
		int index;
		float x, y, w, h;
		m_state = 0;
		MOUSECURSOR()->SetActive(true);
		sscanf(m_macros.front().str, "%d %f %f %f %f", &index, &x, &y, &w, &h);
		m_pForm248 =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "tip", NULL);
		m_pForm248->SetMessage(m_lines[index].c_str(), false);
		((RbFormMsg983*)m_pForm248)->ScrollMsgToFirst();
		m_pForm248->EnableDrag(false);
		m_pForm248->Open((FRESH_PFN_RESULT)&CTutorial::OnTipFormResult,
			WPoint(x, y), FrENTER | FrMODALESS);
		m_pForm248->MoveCursor("ok");
		FrWnd* pArrow = m_pForm248->FindChildByName("harrow");
		if (pArrow)
		{
			pArrow->SetClientRect(WRect(w, h, 19.0f, 14.0f));
		}
		pArrow = m_pForm248->FindChildByName("varrow");
		if (pArrow)
		{
			pArrow->SetVisible(false);
		}
	}
	break;
	case MACRO_0C:
		m_state = 0;
		MOUSECURSOR()->SetActive(true);
		break;
	case MACRO_0D:
	{
		int index;
		float width, height;
		int type;
		m_state = 0;
		MOUSECURSOR()->SetActive(true);
		sscanf(m_macros.front().str, "%d %f %f %d", &index, &width, &height, &type);
		m_pForm248 =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "tip_img", NULL);
		m_pForm248->EnableDrag(false);
		// 983 007ce350 caso 0xd: largura e altura do script, y fixo 200 (slot +0x8c = SetRect)
		m_pForm248->SetRect(WRect((g_view->GetWidth() - width) * 0.5f, 200.0f, width, height));
		FrWnd* pOk = m_pForm248->FindChildByName("ok");
		if (pOk)
		{
			WRect rect;
			m_pForm248->GetClientRect(rect);
			pOk->MoveWindow(
				WPoint(rect.x + rect.w - 50.0f, rect.y + rect.h - 25.0f));
		}
		if (type == 0)
		{
			FrWnd* pChild = m_pForm248->FindChildByName("bong");
			if (pChild)
			{
				pChild->SetVisible(false);
			}
			pChild = m_pForm248->FindChildByName("message");
			if (pChild)
			{
				pChild->SetClientRect(WRect(10.0f, 30.0f, 273.0f, 102.0f));
			}
		}
		else if (type == 1)
		{
			FrWnd* pChild = m_pForm248->FindChildByName("score");
			if (pChild)
			{
				pChild->SetVisible(false);
			}
			pChild = m_pForm248->FindChildByName("message");
			if (pChild)
			{
				pChild->SetClientRect(WRect(65.0f, 55.0f, 218.0f, 80.0f));
			}
		}
		m_pForm248->SetMessage(m_lines[index].c_str(), false);
		((RbFormMsg983*)m_pForm248)->ScrollMsgToFirst();
		m_pForm248->Open((FRESH_PFN_RESULT)&CTutorial::OnTipFormResult, FrENTER | FrMODALESS);
		m_pForm248->MoveCursor("ok");
	}
	break;
	case MACRO_0E:
	{
		// 983 007ce350 caso 0xe: o numero do script e a face do caddie (m_25c); o local do 645 a perdia
		char answer[32];
		sscanf(m_macros.front().str, "%d %s", &m_25c, answer);
		if ((unsigned int)m_25c > 8)
		{
			m_25c = 1;
		}
		m_260 = 0;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (_stricmp(s_data.answers[i], answer) == 0)
			{
				m_260 = i;
				break;
			}
		}
	}
	break;
	case MACRO_0F:
		// 983 caso 0xf: libera o botao do caddie (criado desligado) e o Enter da caixa
		if (m_pTipCaddieWnd != NULL && m_pTipForm != NULL)
		{
			m_pTipCaddieWnd->Enable(true);
			m_pTipForm->EnableFlag(FrENTER);
			if (m_b2d4)
			{
				g_input->FreeMouse(LEFT_BUTTON);
			}
		}
		break;
	case MACRO_10:
	{
		int index = 0;
		float w, h, frameTime;
		sscanf(m_macros.front().str, "%d %f %f %f", &index, &w, &h, &frameTime);
		m_overlays[index].m_dest.w = w;
		m_overlays[index].m_dest.h = h;
		m_overlays[index].m_frameTime = frameTime;
		m_overlays[index].m_bTarget = true;
		m_overlays[index].m_bLoop = true;
		m_overlays[index].m_elapsed = 0.0f;
		m_overlays[index].m_frame = 0;
		m_overlays[index].m_alpha = 255.0f;
		m_overlays[index].m_bShow = true;
		m_overlays[index].m_fade = 1;
	}
	break;
	case MACRO_11:
	{
		int index = 0;
		float w, h, frameTime;
		sscanf(m_macros.front().str, "%d %f %f %f", &index, &w, &h, &frameTime);
		m_overlays[index].m_dest.w = w;
		m_overlays[index].m_dest.h = h;
		m_overlays[index].m_frameTime = frameTime;
		m_overlays[index].m_dest.x = m_240 - w * 0.5f;
		m_overlays[index].m_dest.y = m_244 - h * 0.5f;
		m_overlays[index].m_bTarget = true;
		m_overlays[index].m_bLoop = true;
		m_overlays[index].m_elapsed = 0.0f;
		m_overlays[index].m_frame = 0;
		m_overlays[index].m_alpha = 255.0f;
		m_overlays[index].m_bShow = true;
		m_overlays[index].m_fade = 1;
	}
	break;
	case MACRO_12:
	{
		int index = 0;
		sscanf(m_macros.front().str, "%d", &index);
		m_overlays[index].m_bTarget = false;
		m_overlays[index].m_bLoop = true;
		m_overlays[index].m_fade = -1;
	}
	break;
	case MACRO_13:
	{
		int index = 0;
		float x, y;
		sscanf(m_macros.front().str, "%d %f %f", &index, &x, &y);
		m_overlays[index].m_dest.x = x;
		m_overlays[index].m_dest.y = y;
	}
	break;
	case MACRO_14:
	{
		// 983 caso 0x14: o numero do script e a prova que o OnProcess abre (+0x2b4 = m_270)
		m_state = 4;
		sscanf(m_macros.front().str, "%d", &m_270);
	}
	break;
	case MACRO_15:
	{
		m_state = 1;
		GOLFDOC()->m_bTutorialPin983 = 1;
		WPolySoup::sPoint* pPoint =
			GOLFDOC()->m_pPolySoup->FindPoint(m_macros.front().str);
		GOLFDOC()->m_tutorialPin983 = GetPVS().GetGroundPoint(pPoint->pos, false, NULL, true);
	}
	break;
	case MACRO_16:
	{
		char name[32];
		int msg = 0;
		int a = 0;
		int b = 0;
		int c = 0;
		memset(name, 0, sizeof(name));
		sscanf(m_macros.front().str, "%s %d %d %d %d", name, &msg, &a, &b, &c);
		SendMsg(this, name, msg, a, b, c, 0);
	}
	break;
	case MACRO_17:
	{
		int value;
		sscanf(m_macros.front().str, "%d", &value);
		MOUSECURSOR()->SetActive(value != 0);
	}
	break;
	case MACRO_18:
	{
		int value;
		sscanf(m_macros.front().str, "%d", &value);
		if (value != 0)
		{
			g_input->BlockMouse(LEFT_BUTTON);
			g_input->BlockMouse();
		}
		BlockInput();
	}
	break;
	case MACRO_19:
		FreeInput();
		break;
	case MACRO_1A:
		MoveNextStep();
		break;
	case MACRO_1C:
		// 983 caso 0x1c: minimo da barra lido em M983(0x155)
		sscanf(m_macros.front().str, "%d", &GOLFDOC()->m_tutorialMinGauge983);
		break;
	case MACRO_1D:
		m_b21c = true;
		break;
	case MACRO_1E:
	{
		int value;
		sscanf(m_macros.front().str, "%d", &value);
		if (value == 0)
		{
			g_input->BlockMouse();
		}
		else if (value == 1)
		{
			g_input->BlockMouse(LEFT_BUTTON);
		}
		else if (value == 2)
		{
			g_input->BlockMouse(RIGHT_BUTTON);
		}
	}
	break;
	case MACRO_1F:
		g_input->FreeMouse();
		break;
	}
	if (!m_macros.empty())
	{
		m_macros.pop();
	}
	if (GOLFDOC()->m_tutorialMode == 7 && m_macros.empty())
	{
		return;
	}
	if (m_state == 1)
	{
		FreeInput();
	}
	else if (m_state == 0 || m_state == 5)
	{
		BlockInput();
	}
	if (m_macroWait == 0)
	{
		PopMacro();
	}
}

void CTutorial::ClearMacro()
{
	while (!m_macros.empty())
	{
		m_macros.pop();
	}
}
