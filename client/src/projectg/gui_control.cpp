#include "minatl.h"
#define REBANG_GUI_CONTROL_CPP
#include "projectg.h"
#include "gui_control.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "jp/grandprixartifactrule.h"
#include "jp/holeinonemissiondlg.h"
#include "chatmsg.h"
#include "reportdlg.h"
#include "frbutton.h"
#include "fredit.h"
#include "fresh.h"
#include "messengerdlg.h"
// HACK: wview.h and golfdoc.h (PCH) only declare these three; the unit owns their COMDATs. In the original
// they are PCH inlines past the end of the unit's IL (compiled after the main pass, before wave 1): the dead
// reference to a function of the main pass defers them to the head of wave 1.
inline void WView::ResetClippingArea()
{
	if (0)
		((CMiniScore*)0)->CMiniScore::Flicker();
	WRect rect(0.0f, 0.0f, SCREEN_XS, SCREEN_YS);
	SetClippingArea(rect);
}
#ifdef JP983_VS_DEBUG // D843: letter_effect chama; /Ob1 sem expansao aqui nao emite o COMDAT
void (WView::* const g_jp983DbgResetClippingArea)() = &WView::ResetClippingArea;
#endif

inline void CGolfDoc::SetSaveChatNum(int num)
{
	if (0)
		((CMiniScore*)0)->CMiniScore::Flicker();
	m_saveChatNum = num;
}

inline int CGolfDoc::GetSaveChatNum()
{
	if (0)
		((CMiniScore*)0)->CMiniScore::Flicker();
	return m_saveChatNum;
}

#include "club.h"
#include "fontinfo.h"
#include "mousecursor.h"
#include "simpleoverlay.h"
#include "ground_info.h"
#include "actor.h"
#include "taskmanager.h"
#include "packet.h"
#include "wfont.h"
#include "clientsetting.h"
#include "golfball.h"
#include "quadtree.h"
#include "quitwindow.h"
#include "inputmanager.h"
#include "capturedbg.h"
#include "commonutil.h"
#include "jp/uiframesetmanager.h"
#include "jp/holeeffect.h"
#include "jp/lua983.h"
#include "soundmanager.h"
#include "helpmsg.h"
#include "fxinterpreter.h"
#include "skinnedtheme.h"
#include "wind.h"
#include "itemwindow.h"
#include "powergauge.h"
#include "trainingoptiondlg.h"
#include "headicon.h"
#include "user_info.h"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"
#include "autotarget.h"
#include "golfdocofs983.h" // B1459
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // B1422 temporario

// HACK: golfdoc.h:282 in the original (a per-unit static of the header): its initializer runs
// first but its storage is the last of the unit's bss; a separate ".bss" section keeps the
// initializer order and the image layout (same as shotpreview.cpp).
#pragma bss_seg(".bss")
static float BAR_LENGTH = BAR_END - BAR_START;
#pragma bss_seg()


extern Fresh* g_pFresh;

unsigned char MyIndex();
bool IsMyTurn(bool check);
int IsMassGame();
extern WMatrix g_camera;


// HACK: m_bArea is not public in mousecursor.h (see guidebook.cpp)
struct CMouseCursorAccess : public CMouseCursor
{
	using CMouseCursor::m_bArea;
	using WSingleton<CMouseCursor>::m_pInstance;
};

#define MOUSECURSOR() ((CMouseCursorAccess*)CMouseCursorAccess::m_pInstance)

// HACK: const floats only get storage when an inline of this unit names them.
inline void InstantiateGuiControlConsts()
{
	(void)g_PI;
	(void)g_HUGE;
	(void)BAR_MARGIN;
}

// CGolfDoc +0xcc: hit bar state (golfdoc.h is in the PCH and keeps these as m_unusedcd)
struct sHitBarInfo
{
	unsigned char alpha;
	unsigned char unused1[3];
	float powerPos;
	float unused2[2];
	float hitPos;
	float unused3[3];
	float pangyaPos;
};

#define HITBAR() ((sHitBarInfo*)&GOLFDOC()->m_uiAlpha)

// overlay names shared by OnLoad / DrawApproachInfo / DrawRemainTime
#define OVERLAY_TREASUREBOX "\xc6\xae\xb7\xb9\xc1\xae\xb9\xda\xbd\xba"
#define OVERLAY_APPROACH_INFO "\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1\xc1\xa4\xba\xb8"
#define OVERLAY_TIME_BASE \
	"\xb0\xd4\xc0\xd3\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_base"
#define OVERLAY_TIME_BORDER \
	"\xb0\xd4\xc0\xd3\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_border"
#define OVERLAY_MISSION_FRAME1 \
	"\xb9\xcc\xbc\xc7\xb9\xda\xbd\xba\xc7\xc1\xb7\xb9\xc0\xd3" \
	"1"
#define OVERLAY_MISSION_FRAME2 \
	"\xb9\xcc\xbc\xc7\xb9\xda\xbd\xba\xc7\xc1\xb7\xb9\xc0\xd3" \
	"2"

#define MINISCORE_LOWER_Y 124.0f
#define MINISCORE_PUTT_Y 71.0f

struct sDocFlags
{
	unsigned int flag0 : 1;
	unsigned int flag1 : 1;

	bool Flag1() const { return flag1; }
};


// first member of CPartTidList (no definition in this tree yet)
struct sPartTidHead
{
	unsigned long m_tidChar;
};

#define CLUB_FIELD(type, offset) (*(type*)((char*)&GolfClub() + (offset)))

extern int __fastcall float2int(float);

IMPLEMENT_ACTOR(CScreen, IActor)

CScreen::CScreen()
	: m_pWindPuppet(NULL), m_overlays(0x20, 0x20), m_pFont(NULL)
{
	if (0)
		DrawHitBarInfo(); // HACK: forces the emission of the inline naked DrawHitBarInfo (callers use aliases)
	m_pFontOverlay = NULL;
	m_pGroundInfo = NULL;
	m_pWindInfo = NULL;
	m_pRemainTimeInfo = NULL;
	m_pClubTypeInfo = NULL;
	m_pMiniScore = NULL;
	m_pAuxMenuUI = NULL;
	m_pAccuracyInfo = NULL;
	m_pPangInfo = NULL;
	m_pPowerInfo = NULL;
	m_pCammodeUI = NULL;
	m_pClubLineUI = NULL;
	m_pIconUI = NULL;
	m_pTutoTextUI = NULL;
	m_pTitleFont = NULL;
	m_pTitleFont2983 = NULL;
	m_pMissionUI = NULL;
	for (int i = 0; i < 12; i++)
	{
		m_pQuestMsgUI[i] = NULL;
	}
	m_tutoTime = 0.0f;
	m_tutoScale = 1.0f;
	m_pTutoEscUI = NULL;
	m_pCalipers = NULL;
	m_pHolePoseSkins = NULL;
	m_pExtraHoleInfo = NULL;
	m_clubBoundingBox = -1;
	m_pGuildInfo = NULL;
	m_dispPriority = DISP_PRIORITY_4;
	AddSkipList(0x400);
	// 983 (00751400): campos novos zerados depois do AddSkipList
	m_pHoleInOneScoreInfo983 = NULL;
	m_pHoleinoneMissionDlg983 = NULL;
	m_pHoleEffect983 = NULL;
	m_remainArcStart983 = 0.0f;
	m_pAutoFlag983 = NULL;
	m_pCurveArrow983 = NULL;
	m_remainArcColor983 = 0;
	m_gpExtraElapsedMs983 = 0;
	m_pAutoShotButtons983 = NULL;
	m_pRecoverPangyaShotEffect983 = NULL;
	m_pRecoverPangyaShotRingEffect983 = NULL;
	m_pFxInterpreter983 = new FxInterpreter(NULL, 0); // 983: 00751670
}

void CScreen::OnDestroy()
{
	// 983 (0074d8e0): a janela da missao do hole-in-one e os objetos do hole-in-one antes de tudo
	if (m_pHoleinoneMissionDlg983)
	{
		m_pHoleinoneMissionDlg983->Close((eFormRet)1, true);
	}
	if (m_pHoleEffect983)
	{
		delete m_pHoleEffect983;
		m_pHoleEffect983 = NULL;
	}
	if (m_pHoleInOneScoreInfo983)
	{
		delete m_pHoleInOneScoreInfo983;
		m_pHoleInOneScoreInfo983 = NULL;
	}
	if (m_pFontOverlay)
	{
		delete m_pFontOverlay;
		m_pFontOverlay = NULL;
	}
	if (m_pGroundInfo)
	{
		delete m_pGroundInfo;
		m_pGroundInfo = NULL;
	}
	if (m_pWindInfo)
	{
		delete m_pWindInfo;
		m_pWindInfo = NULL;
	}
	if (m_pRemainTimeInfo)
	{
		delete m_pRemainTimeInfo;
		m_pRemainTimeInfo = NULL;
	}
	if (m_pClubTypeInfo)
	{
		delete m_pClubTypeInfo;
		m_pClubTypeInfo = NULL;
	}
	if (m_pMiniScore)
	{
		delete m_pMiniScore;
		m_pMiniScore = NULL;
	}
	if (m_pAuxMenuUI)
	{
		delete m_pAuxMenuUI;
		m_pAuxMenuUI = NULL;
	}
	if (m_pAccuracyInfo)
	{
		delete m_pAccuracyInfo;
		m_pAccuracyInfo = NULL;
	}
	if (m_pPangInfo)
	{
		delete m_pPangInfo;
		m_pPangInfo = NULL;
	}
	if (m_pPowerInfo)
	{
		delete m_pPowerInfo;
		m_pPowerInfo = NULL;
	}
	if (m_pCammodeUI)
	{
		delete m_pCammodeUI;
		m_pCammodeUI = NULL;
	}
	if (m_pClubLineUI)
	{
		delete m_pClubLineUI;
		m_pClubLineUI = NULL;
	}
	if (m_pIconUI)
	{
		delete m_pIconUI;
		m_pIconUI = NULL;
	}
	if (m_pTutoTextUI)
	{
		delete m_pTutoTextUI;
		m_pTutoTextUI = NULL;
	}
	for (int i = 0; i < 12; i++)
	{
		if (m_pQuestMsgUI[i])
		{
			delete m_pQuestMsgUI[i];
			m_pQuestMsgUI[i] = NULL;
		}
	}
	if (m_pMissionUI)
	{
		delete m_pMissionUI;
		m_pMissionUI = NULL;
	}
	if (m_pTitleFont)
	{
		m_pTitleFont->Erase();
		if (g_resrcmng && m_pTitleFont)
		{
			g_resrcmng->Release(m_pTitleFont);
			m_pTitleFont = NULL;
		}
	}
	if (m_pCalipers)
	{
		delete m_pCalipers;
		m_pCalipers = NULL;
	}
	if (m_pHolePoseSkins)
	{
		delete m_pHolePoseSkins;
		m_pHolePoseSkins = NULL;
	}
	if (m_pExtraHoleInfo)
	{
		delete m_pExtraHoleInfo;
		m_pExtraHoleInfo = NULL;
	}
	if (m_pTutoEscUI)
	{
		delete m_pTutoEscUI;
		m_pTutoEscUI = NULL;
	}
	if (m_pGuildInfo)
	{
		delete m_pGuildInfo;
		m_pGuildInfo = NULL;
	}
	if (m_pAutoFlag983)
	{
		delete m_pAutoFlag983;
		m_pAutoFlag983 = NULL;
	}
	if (m_pAutoShotButtons983)
	{
		delete m_pAutoShotButtons983;
		m_pAutoShotButtons983 = NULL;
	}
	if (m_pCurveArrow983)
	{
		delete m_pCurveArrow983;
		m_pCurveArrow983 = NULL;
	}
	if (m_pRecoverPangyaShotEffect983)
	{
		delete m_pRecoverPangyaShotEffect983;
		m_pRecoverPangyaShotEffect983 = NULL;
	}
	if (m_pRecoverPangyaShotRingEffect983)
	{
		delete m_pRecoverPangyaShotRingEffect983;
		m_pRecoverPangyaShotRingEffect983 = NULL;
	}
	delete CHelpMsg::Instance();
	if (g_resrcmng && m_pWindPuppet)
	{
		g_resrcmng->Release(m_pWindPuppet, false);
		m_pWindPuppet = NULL;
	}
	for (sOverlay* pOverlay = m_overlays.Start(); pOverlay;
		pOverlay = m_overlays.Next())
	{
		if (pOverlay)
		{
			delete pOverlay;
			pOverlay = NULL;
		}
	}
	m_overlays.Reset();
	m_pFxInterpreter983->DetachFx(m_clubBoundingBox);
	// 983 (0074d8e0, fim): solta a fonte do relogio do Grand Prix (+0x2ec)
	if (g_resrcmng && m_pTitleFont2983)
	{
		g_resrcmng->Release(m_pTitleFont2983);
		m_pTitleFont2983 = NULL;
	}
}

// ---- 983: objetos novos do CScreen (0x250..0x26c) ----
CScreenAutoFlag983::CScreenAutoFlag983()
{
	Reset();
}

void CScreenAutoFlag983::Reset()
{
	m_on = false;
}

void CScreenAutoFlag983::Process(float dt)
{
	CClub& club = GolfClub();
	if (~(*(unsigned int*)((char*)&club + 0x34) ^
			*(unsigned int*)((char*)&club + 0x2c)) &&
		m_on)
	{
		m_value = GOLFDOC()->m_uiAlpha; // +0xdc
	}
}

CAutoShotButtons983::CAutoShotButtons983()
{
	Reset();
}

void CAutoShotButtons983::Reset()
{
	memset(&m_on, 0, 4);
}

void CAutoShotButtons983::SetOn(bool on)
{
	m_on = on;
}

void CAutoShotButtons983::SetButton(unsigned char kind, bool on)
{
	// 983 0074aac0: so o botao escolhido acende (on) ou alterna; os outros apagam
	unsigned char z = 0;
	switch (kind)
	{
	case 0x10:
		if (on)
		{
			m_state[0] = 2;
			m_state[1] = z;
			m_state[2] = z;
			return;
		}
		m_state[1] = z;
		m_state[2] = z;
		m_state[0] = (m_state[0] == 2) - 1U & 2;
		return;
	case 0x20:
		if (on)
		{
			m_state[1] = 2;
			m_state[0] = z;
			m_state[2] = z;
			return;
		}
		m_state[0] = z;
		m_state[2] = z;
		m_state[1] = (m_state[1] == 2) - 1U & 2;
		return;
	case 0x40:
		if (on)
		{
			m_state[2] = 2;
			m_state[0] = z;
			m_state[1] = z;
			return;
		}
		m_state[0] = z;
		m_state[1] = z;
		m_state[2] = (m_state[2] == 2) - 1U & 2;
		return;
	default:
		*(unsigned short*)&m_state[0] = z;
		m_state[2] = z;
		return;
	}
}

CCurveArrow983::CCurveArrow983()
{
	m_alpha = 0.0f;
	m_type = 0;
	m_time = 0.0f;
	m_state = 0;
}

CCurveArrow983::~CCurveArrow983()
{
	if (g_resrcmng && m_pArrow[0])
	{
		g_resrcmng->Release(m_pArrow[0]);
		m_pArrow[0] = NULL;
	}
	if (g_resrcmng && m_pArrow[1])
	{
		g_resrcmng->Release(m_pArrow[1]);
		m_pArrow[1] = NULL;
	}
	if (g_resrcmng && m_pArrow[2])
	{
		g_resrcmng->Release(m_pArrow[2]);
		m_pArrow[2] = NULL;
	}
}

void CCurveArrow983::Process(float dt)
{
	if (m_state == 1)
	{
		m_time += dt;
		if (m_time >= 2.0f)
		{
			m_state = 2;
			return;
		}
		if (m_time > 0.5f)
		{
			m_alpha += 10.0;
		}
		if (m_alpha > 255.0)
		{
			m_alpha = 255.0f;
		}
	}
	else if (m_state == 0)
	{
		m_time = 0.0f;
		m_alpha = 0.0f;
	}
	else if (m_state == 2)
	{
		m_alpha -= 10.0;
		if (m_alpha < 0.0f)
		{
			m_alpha = 0.0f;
		}
	}
}

void CCurveArrow983::SetType(unsigned int type)
{
	if (type <= 2)
	{
		m_type = type;
	}
}

void CCurveArrow983::SetState(unsigned int state)
{
	if (state <= 2)
	{
		m_state = state;
	}
}

CRecoverPangyaShotEffect::~CRecoverPangyaShotEffect()
{
}

void CRecoverPangyaShotEffect::Reset()
{
	m_active = false;
	m_time = 0.0f;
	m_frame = 1;
}

CRecoverPangyaShotRingEffect::~CRecoverPangyaShotRingEffect()
{
}

void CRecoverPangyaShotRingEffect::Reset()
{
	m_active = false;
	m_time = 0.0f;
	m_frame = 1;
}

void CCurveArrow983::Load()
{
	m_pArrow[0] = g_resrcmng->GetOverlay("CurveArrow_Straight.png", 0);
	m_pArrow[1] = g_resrcmng->GetOverlay("CurveArrow_Left.png", 0);
	m_pArrow[2] = g_resrcmng->GetOverlay("CurveArrow_Right.png", 0);
	m_pos = WVector2D(g_view->GetWidth() * 0.5, g_view->GetHeight() * 0.5);
}

// 983 (0074b690): recentra a seta do tipo atual na tela (chamada pelo OnInit e pelo HandleMsg)
void CCurveArrow983::Reposition()
{
	m_pos.x = g_view->GetWidth() * 0.5 - m_pArrow[m_type]->GetWidth() * 0.5;
	m_pos.y = g_view->GetHeight() * 0.5 - m_pArrow[m_type]->GetHeight() * 0.5;
}

extern const WRect g_rectUnitW983; // 00e4296c (retangulo de origem padrao)
// 983 (0074e470): seta da curva (reta/esquerda/direita) na vez do jogador; na tacada 0x200 fica no centro (m_pos), nas outras
// acompanha a bola projetada (centrada, acima dela); alpha = m_alpha
void CCurveArrow983::Display()
{
	if (IsMyTurn(false) && (*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x158)) & 0xe0) == 0 && GolfClub().GetType() != 3 &&
		m_pArrow[m_type])
	{
		WOverlay* arrow = m_pArrow[m_type];
		unsigned long color = ((unsigned long)(int)m_alpha << 24) | 0xffffff;
		float w = (float)arrow->GetWidth();
		float h = (float)arrow->GetHeight();
		if (GOLFDOC()->m_gameMode == 0x200)
		{
			arrow->Render(g_view, g_rectUnitW983, WRect(m_pos.x, m_pos.y, w, h), 0, color, 0.0f, 0);
			return;
		}

		WTVertex v;
		g_view->Projection2(&v, GolfBall().m_pos);
		if (v.z >= 0.0f && v.z <= 1.0f)
		{
			float x = v.x - w * 0.5f;
			float y = v.y - (h + h * (g_view->GetHeight() / 480.0f));
			arrow->Render(g_view, g_rectUnitW983, WRect(x, y, w, h), 0, color, 0.0f, 0);
		}
	}
}

// 983 (00751250)
void CRecoverPangyaShotEffect::Display()
{
	if (m_active && m_frame > 0 && m_frame <= m_frameCount)
	{
		CUiFrameSetManager::Instance()->DrawFrameSet(MakeStr("f_footprint_effect_%d", m_frame), g_view, 0,
			((unsigned long)*((unsigned char*)GDOCPTR983(GOLFDOC(), 0xdc)) << 24) | 0xffffff);
	}
}

// 983 (00751340)
void CRecoverPangyaShotRingEffect::Display()
{
	if (m_active && m_soundFrame < m_frame && m_frame <= m_frameCount + m_soundFrame)
	{
		CUiFrameSetManager::Instance()->DrawFrameSet(MakeStr("f_rainbowprint_effect_%d", m_frame - m_soundFrame), g_view,
			0, ((unsigned long)*((unsigned char*)GDOCPTR983(GOLFDOC(), 0xdc)) << 24) | 0xffffff);
	}
}

CRecoverPangyaShotEffect::CRecoverPangyaShotEffect()
{
	m_interval = 0.1f;
	m_frameCount = 4;
	Reset();
}

void CRecoverPangyaShotEffect::Process(float dt)
{
	if (m_active)
	{
		m_time += dt;
		if (m_time >= m_interval)
		{
			if (m_frame < m_frameCount)
			{
				m_time = 0.0f;
				m_frame++;
				return;
			}
			Reset();
		}
	}
}

CRecoverPangyaShotRingEffect::CRecoverPangyaShotRingEffect()
{
	m_interval = 0.1f;
	m_frameCount = 4;
	m_soundFrame = 4;
	Reset();
}

void CRecoverPangyaShotRingEffect::Process(float dt)
{
	if (m_active)
	{
		m_time += dt;
		if (m_time >= m_interval)
		{
			m_time = 0.0f;
			if (m_frame < m_frameCount + m_soundFrame)
			{
				if (++m_frame == m_soundFrame)
				{
					g_audio->PlaySfx("luck_rainbow_print", NULL, 0, 0, 0, 0.5f,
						200.0f);
				}
				return;
			}
			Reset();
		}
	}
}

// 983 (0074bde0): alem dos tres flags, zera +0x4cc, liga o relogio do Grand Prix (realGameType 0x14/0x15) e
// guarda se o artefato da sala (tid em +0x109b9) e cronometrado (CGrandPrixArtifactRule::IsTimed 00546900).
void CScreen::OnPreLoadInit()
{
	m_displayEnabled = true;
	m_spinNotify = false;
	m_tuto11Triggered = false;
	m_gpExtraElapsedMs983 = 0;
	m_grandPrix983 = (unsigned long)(Doc()->m_roomInfo.realGameType - 0x14) <= 1;
	if (m_grandPrix983)
	{
		unsigned long tid = Doc()->m_roomInfo.artifactTid983;
		m_gpTimedHole983 = _grandprix::_private::CGrandPrixArtifactRule::Instance()->IsTimed(tid);
	}
}

// 983 (0074ff30): offsets do relogio de limite de tempo do Grand Prix
bool CScreen::LoadGrandPrixUi983()
{
	lua_system::lua_script* script =
		lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script && script->RunFile("grandprix_ingame_ui.lua"))
	{
	lua_system::lua_table_helper tbl =
		script->GetTable("grandprix_next_hole_limited_time_table");
	m_gpBaseImg983 =
		((LuaHelper983&)tbl).GetChecked983<const char*>("base_ui_img_file_name");
	m_gpCautionImg983 = ((LuaHelper983&)tbl).GetChecked983<const char*>(
		"caution_ui_img_file_name");
	m_gpUiOffsetX983 = LUA983_GET(tbl, float, "hole_limited_time_ui_offset_x");
	m_gpUiOffsetY983 = LUA983_GET(tbl, float, "hole_limited_time_ui_offset_y");
	m_gpTextOffsetX983 =
		LUA983_GET(tbl, float, "hole_limited_time_text_offset_x");
	m_gpTextOffsetY983 =
		LUA983_GET(tbl, float, "hole_limited_time_text_offset_y");
	m_gpColonOffsetX983 =
		LUA983_GET(tbl, float, "remain_time_sign_colon_str_offset_x");
	m_gpSecondOffsetX983 =
		LUA983_GET(tbl, float, "remain_time_second_str_offset_x");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
	return true;
	}
	return false;
}

// 983 (007500a0): relogio do Grand Prix no lugar do "timer.png"
void CScreen::LoadGrandPrixOverlays983()
{
	if (LoadGrandPrixUi983())
	{
		AddOverlay("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae_\xc8\xa6_\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_base",
			m_gpBaseImg983.c_str(), 0);
		AddOverlay("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae_\xc8\xa6_\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_caution",
			m_gpCautionImg983.c_str(), 0);
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(NULL, M983(0x78), 0x200, 0, 0, 0, 0);
	}
}

// 983 (0074c960): modo caca ao tesouro (tipo de jogo 10), com a tabela de campos do 983
void CScreen::LoadTreasureOverlays983()
{
	AddOverlay(OVERLAY_TIME_BORDER, "icon_time.png", 0);
	AddOverlay(OVERLAY_TIME_BASE, "icon_time_02.png", 0);
	AddOverlay(OVERLAY_APPROACH_INFO, "[ground_text_02.jpg", 0);
	AddOverlay(OVERLAY_MISSION_FRAME1, "treasurebox_frm1.tga", 0);
	AddOverlay(OVERLAY_MISSION_FRAME2, "treasurebox_frm2.png", 0);

	switch (GetCurMap())
	{
	case 0:
	case 1:
	case 6:
	case 7:
		AddOverlay(OVERLAY_TREASUREBOX, "treasurebox_b.png", 0);
		break;
	case 2:
	case 3:
	case 11:
	case 16:
		AddOverlay(OVERLAY_TREASUREBOX, "treasurebox_a.png", 0);
		break;
	case 4:
	case 5:
	case 8:
	case 9:
	case 14:
	case 19:
		AddOverlay(OVERLAY_TREASUREBOX, "treasurebox_d.png", 0);
		break;
	case 10:
	case 13:
	case 15:
	case 17:
	case 18:
	case 20:
	case 21:
		AddOverlay(OVERLAY_TREASUREBOX, "treasurebox_c.png", 0);
		break;
	}
}

// 983 (007517b0): so 10 overlays (os do 645 - left_*, middle_*, classic_*, vento do tema, click.png, left_ic_*, keys.png - sairam;
// o 983 desenha por quadros do CUiFrameSetManager) e os objetos novos 0x250..0x26c
void CScreen::OnLoad()
{
	AddOverlay("\xc5\xac\xb7\xb4\xb9\xd9", "{middle_club.png", 0);
	AddOverlay("\xc5\xbe\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb", "[camera_top.jpg",
		0);
	AddOverlay("\xc1\xb6\xc1\xbe\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb",
		"[camera_free.jpg", 0);
	AddOverlay("\xb3\xab\xc7\xcf\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb",
		"[camera_target.jpg", 0);

	if (Doc()->m_golfGame.gameType == 10)
	{
		LoadTreasureOverlays983();
	}
	else if (m_grandPrix983)
	{
		LoadGrandPrixOverlays983();
	}
	else
	{
		AddOverlay("\xb0\xd4\xc0\xd3\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3",
			"timer.png", 0);
	}

	m_pCammodeUI = new CSimpleUI("[camera_mode.jpg");
	m_pCammodeUI->Add("FORWARD_OVER", WRect(0.0f, 0.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("FORWARD_CLICK", WRect(0.0f, 85.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("BACKWARD_OVER", WRect(85.0f, 0.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("BACKWARD_CLICK", WRect(85.0f, 85.0f, 85.0f, 85.0f),
		false);
	m_pCammodeUI->Add("LEFT_OVER", WRect(0.0f, 170.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("LEFT_CLICK", WRect(0.0f, 255.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("RIGHT_OVER", WRect(85.0f, 170.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("RIGHT_CLICK", WRect(85.0f, 255.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("UP_OVER", WRect(0.0f, 340.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("UP_CLICK", WRect(0.0f, 425.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("DOWN_OVER", WRect(85.0f, 340.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("DOWN_CLICK", WRect(85.0f, 425.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("NORMAL", WRect(170.0f, 0.0f, 85.0f, 85.0f), false);
	m_pCammodeUI->Add("INACTIVE", WRect(170.0f, 85.0f, 85.0f, 85.0f), false);

	m_pClubLineUI = new CSimpleUI("[club_line.jpg");
	m_pClubLineUI->Add("1W", WRect(0.0f, 0.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("2W", WRect(0.0f, 44.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("3W", WRect(0.0f, 88.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("2I", WRect(0.0f, 132.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("3I", WRect(0.0f, 176.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("4I", WRect(0.0f, 220.0f, 512.0f, 44.0f), false);
	m_pClubLineUI->Add("5I", WRect(0.0f, 264.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("6I", WRect(256.0f, 264.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("7I", WRect(0.0f, 326.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("8I", WRect(256.0f, 326.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("9I", WRect(0.0f, 388.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("PW", WRect(256.0f, 388.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("SW", WRect(0.0f, 450.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("1PT", WRect(256.0f, 450.0f, 256.0f, 62.0f), false);
	m_pClubLineUI->Add("2PT", WRect(256.0f, 450.0f, 256.0f, 62.0f), false);

	m_pIconUI = new CSimpleUI("[icon01.jpg");
	m_pIconUI->Add("\xc7\xf6\xc0\xe7\xc5\xac\xb7\xb4",
		WRect(288.0f, 0.0f, 48.0f, 48.0f), false);

	if (GOLFDOC()->m_tutorialMode < 15)
	{
		for (int i = 0; i < 12; i++)
		{
			m_pQuestMsgUI[i] =
				new CSimpleUI(MakeStr("quest_message_%d.png", i + 1));
			m_pQuestMsgUI[i]->Add("MissionTitle",
				WRect(0.0f, 0.0f, 512.0f, 16.0f), false);
		}
	}

	m_remainTimeY = COption::Instance()->gGetBar_Y();
	m_pWindPuppet = g_resrcmng->GetPuppet("wind.pet", false, true, false);
	m_pFontOverlay = new CFontOverlay;
	m_pGroundInfo = new CBase;
	if (Doc()->m_golfGame.gameType == 11)
	{
		m_pHoleInOneScoreInfo983 = new CHoleInOneScoreInfo;
		m_pPangInfo = new CPangInfo;
		m_pHoleEffect983 = new CHoleEffect;
	}
	else
	{
		m_pPangInfo = new CPangInfo;
	}
	m_pMiniScore = new CMiniScore;
	m_pWindInfo = new CWindInfo;
	m_pClubTypeInfo = new CClubTypeInfo;
	m_pAccuracyInfo = new CAccuracyInfo;
	m_pPowerInfo = new CPowerInfo;
	m_pAutoFlag983 = new CScreenAutoFlag983;
	m_pAutoShotButtons983 = new CAutoShotButtons983;
	m_pCurveArrow983 = new CCurveArrow983;
	m_pRecoverPangyaShotEffect983 = new CRecoverPangyaShotEffect;
	m_pRecoverPangyaShotRingEffect983 = new CRecoverPangyaShotRingEffect;

	if (Doc()->m_golfGame.gameType == 7)
	{
		m_pHolePoseSkins = new CHolePoseSkins;
		m_pExtraHoleInfo = new CExtraHoleInfo;
	}

	if (GOLFDOC()->m_tutorialMode == 15)
	{
		AddOverlay("\xbf\xac\xbd\xc0\xb8\xf0\xb5\xe5\xbf\xc9\xbc\xc7",
			"train_option_small.png", 0);
		m_pAuxMenuUI = new CAuxMenuUI;
		m_pRemainTimeInfo = new CRemainTimeInfo;
		m_pCalipers = new CCalipers;
		// 983 (00752344): modos 8/9 no tipo de jogo 0xb, senao 4/5
		m_pCalipers->SetMode983(Doc()->m_golfGame.gameType == 0xb ? 4 : 2);
		new CHelpMsg;
	}
	else
	{
		if (GOLFDOC()->m_tutorialMode == 5 || GOLFDOC()->m_tutorialMode == 9)
		{
			m_pAuxMenuUI = new CAuxMenuUI;
			new CHelpMsg;
		}

		AddOverlay("\xc1\xb6\xc1\xd8\xbf\xb5\xbf\xaa", "{sight.png", 0);
		AddOverlay("\xc7\xd9\xbd\xc9\xc6\xf7\xc0\xce\xc6\xae",
			"check_point.png", 0);
		AddOverlay("\xc7\xd9\xbd\xc9\xc6\xf7\xc0\xce\xc6\xaesmall",
			"check_point_small.png", 0);

		if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
		{
			m_tutoStep = 0;
			m_pTutoTextUI = new CSimpleUI("[tuto_opentext3.png");
			for (int i = 0; i <= 14; i++)
			{
				m_pTutoTextUI->Add(
					MakeStr("\xc5\xb8\xc0\xcc\xc6\xb2_%d", i + 1),
					WRect(0.0f, i * 28.0f, 128.0f, 28.0f), true);
			}
		}
		else
		{
			AddOverlay("\xb9\xcc\xbc\xc7\xc1\xa4\xba\xb8_0", "[mission_00.jpg",
				0);
		}

		m_tutoBlinkPeriod = 0.25f;
		m_tutoBlinkOn = 0;
		m_tutoBlinkCount = 2;
		m_tutoBlinkTime = 0;
		m_holeBlinkPeriod = 0.25f;
		m_holeBlinkCount = 3;
		m_tutoTextState = 2;
		m_tutoTextTime = 0;
		m_tutoTextAlpha = 0;
		m_tutoTitleState = 2;
		m_tutoTime = 0.0f;

		m_pTutoEscUI = new CSimpleUI("[tuto_esc_msg2.png");
		m_pTutoEscUI->Add("MSG0", WRect(0.0f, 0.0f, 256.0f, 16.0f), false);
		m_pTutoEscUI->Add("MSG1", WRect(0.0f, 16.0f, 256.0f, 16.0f), false);
	}

	m_pFont = CChatMsg::Instance()->GetMaskedFont();

	for (int i = 0; i < 15; i++)
	{
		if (i < GOLFDOC()->m_tutorialMode)
		{
			m_bHoleShown[i] = true;
		}
		else
		{
			m_bHoleShown[i] = false;
		}
	}

	if (GOLFDOC()->m_tutorialMode < 15 && m_pFontOverlay)
	{
		m_pFontOverlay->FixHoleInfo();
	}

	if (m_pCurveArrow983)
	{
		m_pCurveArrow983->Load();
	}
	m_gpBlinkOn983 = false;
	m_gpBlinkPeriod983 = 0.5f;
	m_gpBlinkTime983 = 0.0f;
}

void CScreen::OnInit()
{
	m_bannerTime = 0.0f;
	m_hitBarCount = 10;
	m_hitBarFrom = 0;
	m_hitBarTime = 0.0f;
	m_selectedIndex = -1;
	m_turnStarted = false;
	m_turnTime = 0;
	m_accumValue = 0.0f;
	m_fadeValue = 0.0f;
	m_blinkTime = 0.0f;
	m_hudVisible = true;
	m_clubAngle = 6.2831855f;
	m_clubSpinDir = 0;
	m_clubSwing = 0.0f;
	m_playerCount = 0;
	m_msgColorTime = 0;
	m_msgColor = 0;
	m_cammodeAlpha = 0.0f;
	m_drawFlags.Enable(2);

	unsigned long tid =
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid;
	sClubSet* pClubSet = GolfClub().FindClubSet(tid);
	WPuppet* pPuppet = NULL;
	if (pClubSet)
	{
		pPuppet = pClubSet->pHeadSub[GolfClub().GetType()];
	}
	m_clubBoundingBox = m_pFxInterpreter983->ParseBoundingBoxScript_fx(pPuppet);

	if (m_pTitleFont == NULL)
	{
		static char* s_windFontFile = "[font_wind.jpg";
		tagWTITLEFONT info;
		memset(&info, 0, sizeof(info));
		info.filename = &s_windFontFile;
		info.fontw = 16;
		info.fonth = 16;
		info.numPages = 1;
		info.texw = 128;
		info.texh = 64;
		info.pCharSet = "1234567890ym%-.?/:";
		m_pTitleFont = g_resrcmng->GetTitleFont();
		m_pTitleFont->Create(&info);
	}

	if (m_pMissionUI == NULL)
	{
		m_pMissionUI = new CSimpleUI("text_mission.png");
		m_pMissionUI->Add("\xb9\xcc\xbc\xc7\xbe\xf8\xc0\xbd",
			WRect(91.0f, 10.0f, 232.0f, 100.0f), false);
		m_pMissionUI->Add("\xb9\xcc\xbc\xc7\xc0\xd6\xc0\xbd",
			WRect(50.0f, 130.0f, 256.0f, 130.0f), false);
	}

	if (Doc()->m_golfGame.gameType == 11)
	{
		m_pHoleinoneMissionDlg983 = CreateForm<_gateway::FrHoleinoneMissionDlg>(
			g_pFresh->GetManager(), this, "holeinone_mission", NULL);
		m_pHoleinoneMissionDlg983->EnableDrag(false);
		m_pHoleinoneMissionDlg983->SetFixed(true);
		m_pHoleinoneMissionDlg983->Show983();
		m_pHoleinoneMissionDlg983->Open(NULL, 0xc);
	}

	m_pCurveArrow983->Reposition();

	if (m_pTitleFont2983 == NULL)
	{
		m_pTitleFont2983 = NULL;
		static char* s_windFontX2File = "[font_wind_x2.jpg";
		tagWTITLEFONT info2;
		memset(&info2, 0, sizeof(info2));
		info2.filename = &s_windFontX2File;
		info2.fontw = 32;
		info2.fonth = 32;
		info2.numPages = 1;
		info2.texw = 256;
		info2.texh = 128;
		info2.pCharSet = "1234567890ym%-.?/:";
		m_pTitleFont2983 = g_resrcmng->GetTitleFont();
		m_pTitleFont2983->Create(&info2);
		m_pTitleFont2983->SetScale(0.8f);
	}
}

// 983 (0074cd20): relogio do tempo limite do buraco no Grand Prix ("GP_Special_Rule_Limited_Time": ultimos 10 s em
// vermelho, com o som em laco); so com a flag +0x4d5 (msg 0x248) e fora dos modos 0x1002
void CScreen::DrawGrandPrixTimer983()
{
	static const char kSound[] = "GP_Special_Rule_Limited_Time";
	static const char kBase[] =
		"\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae_\xc8\xa6_\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_base";
	static const char kCaution[] =
		"\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae_\xc8\xa6_\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3_caution";

	CSharedDoc* sdoc = CSharedDoc::Instance(); // A730: membros (cl novo desloca os crus do VC10)
	unsigned long gameMode = sdoc->m_pGolfDoc->m_gameMode;
	if (!m_gpClockShown983 || (gameMode & 0x1002))
	{
		if (g_audio->IsPlaying(kSound))
		{
			g_audio->StopSfx(kSound);
		}
		return;
	}

	float size[2] = {0.0f, 0.0f};
	sOverlay* baseOv = m_overlays.Find(kBase); // 0074c2e0
	if (baseOv && baseOv->m_pOverlay)
	{
		size[0] = (float)baseOv->m_pOverlay->GetWidth();
		size[1] = (float)baseOv->m_pOverlay->GetHeight();
	}

	float* pos = m_gpUiPos983;
	if (!(gameMode & 0x2000))
	{
		pos[0] = g_view->GetWidth() * 0.5f - size[0] * 0.5f;
		if (m_pMiniScore)
		{
			WRect icon = m_pMiniScore->GetIconRect(0);
			pos[0] = icon.x;
			pos[1] = icon.y + icon.h;
		}
		pos[0] = m_gpUiOffsetX983 + pos[0];
		pos[1] = m_gpUiOffsetY983 + pos[1];
	}
	WRect rect(pos[0], pos[1], size[0], size[1]);
	float textX = m_gpTextOffsetX983 + pos[0];
	float textY = m_gpTextOffsetY983 + pos[1];

	unsigned long limit = sdoc->m_golfGame.gpTimeLimit983;
	unsigned long start = sdoc->m_approachStartTime;
	unsigned long left = limit;
	bool running = true;
	if (start != 0)
	{
		left = start + limit;
		if (g_CurrentTime <= left)
		{
			left -= g_CurrentTime;
		}
		else
		{
			running = false;
		}
	}

	if (running && left != 0)
	{
		unsigned long sec = 0;
		if (limit == 0)
		{
			DrawOverlay2(kBase, rect, 0xffffffff, 0);
		}
		else
		{
			sec = left / 1000;
			if (sec <= 10)
			{
				DrawOverlay(kCaution, rect, 0xff, 0);
				if (!g_audio->IsPlaying(kSound))
				{
					g_audio->PlaySfx(kSound, NULL, 1, kSound, 0, 0.5f, 200.0f);
				}
			}
			else
			{
				DrawOverlay2(kBase, rect, 0xffffffff, 0);
				if (g_audio->IsPlaying(kSound))
				{
					g_audio->StopSfx(kSound);
				}
			}
		}

		char minStr[16];
		char secStr[16];
		if (limit == 0)
		{
			strcpy(minStr, "--");
			strcpy(secStr, "--");
		}
		else
		{
			sprintf(minStr, "%02d", sec / 60);
			sprintf(secStr, "%02d", sec % 60);
		}
		m_pTitleFont2983->Print(g_view, textX, textY, minStr, 0, 0xffffffff, NULL);
		if (m_gpBlinkOn983)
		{
			m_pTitleFont2983->Print(g_view, m_gpColonOffsetX983 + textX, textY, ":", 0, 0xffffffff, NULL);
		}
		m_pTitleFont2983->Print(g_view, m_gpSecondOffsetX983 + textX, textY, secStr, 0, 0xffffffff, NULL);
		m_pTitleFont2983->Flush(g_view);
		return;
	}

	if (g_audio->IsPlaying(kSound))
	{
		g_audio->StopSfx(kSound);
	}
}

void CScreen::OnProcess(float dt)
{
	// 983 (00752660): Grand Prix com gameMode 0x2000 so pisca o relogio (+0x519) e sai
	if (m_grandPrix983 && (GOLFDOC()->m_gameMode & 0x2000))
	{
		if (Doc()->m_golfGame.gpTimeLimit983 == 0)
		{
			m_gpBlinkOn983 = true;
			return;
		}
		m_gpBlinkTime983 += dt;
		if (m_gpBlinkTime983 > m_gpBlinkPeriod983)
		{
			m_gpBlinkTime983 -= m_gpBlinkPeriod983;
			m_gpBlinkOn983 = !m_gpBlinkOn983;
		}
		return;
	}
	if (GOLFDOC()->m_tutorialMode < 15)
	{
		if (m_holeBlinkCount < 3)
		{
			m_holeBlinkTime += dt;
			if (m_holeBlinkTime > m_holeBlinkPeriod)
			{
				m_holeBlinkTime -= m_holeBlinkPeriod;
				m_bHoleShown[GOLFDOC()->m_tutorialMode] =
					!m_bHoleShown[GOLFDOC()->m_tutorialMode];
				if (m_bHoleShown[GOLFDOC()->m_tutorialMode])
				{
					m_holeBlinkCount++;
				}
			}
		}

		if (m_tutoBlinkCount < 2)
		{
			m_tutoBlinkTime += dt;
			if (m_tutoBlinkTime > m_tutoBlinkPeriod)
			{
				m_tutoBlinkTime -= m_tutoBlinkPeriod;
				m_tutoBlinkOn = 1 - m_tutoBlinkOn;
				if (m_tutoBlinkOn)
				{
					m_tutoBlinkCount++;
				}
			}
		}

		float startX = g_view->GetWidth() * 0.5f - 50.0f;
		float startY = g_view->GetHeight() * 0.3f;
		float endX = g_view->GetWidth() - 250.0f;
		switch (m_tutoTextState)
		{
		case 0:
			m_tutoTextTime += dt;
			if (m_tutoTextTime > 1.0f)
			{
				m_tutoTextTime = 0.0f;
				m_tutoTextAlpha = 0xFF;
				m_tutoTextState = 1;
			}
			else
			{
				m_tutoTextAlpha = (unsigned char)(m_tutoTextTime * 255.0f);
			}
			m_tutoTextX = startX;
			m_tutoTextY = startY;
			break;
		case 1:
			m_tutoTextTime += dt;
			if (m_tutoTextTime > 1.0f)
			{
				m_tutoTextTime = 1.0f;
				m_tutoTextState = 2;
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial)
				{
					MsgObject msg(NULL, M983(0x148), 0, 0, 0, 0, 0);
					tutorial << msg;
				}
			}
			m_tutoTextX = (1.0f - m_tutoTextTime) * startX + endX * m_tutoTextTime;
			m_tutoTextY = startY * (1.0f - m_tutoTextTime) + 2.0f * m_tutoTextTime;
			break;
		}
	}

	if (GOLFDOC()->m_tutorialMode >= 0 && GOLFDOC()->m_tutorialMode < 15 &&
		GOLFDOC()->m_tutorialMode != 6 && GOLFDOC()->m_tutorialMode != 7)
	{
		float startX = g_view->GetWidth() * 0.5f;
		float startY = g_view->GetHeight() * 0.33333334f;
		float endX = (float)((int)g_view->GetWidth() / 2);
		if (m_tutoTitleState == 0)
		{
			m_tutoScale = 1.2f;
			m_tutoTime += dt;
			if (m_tutoTime > 1.0f)
			{
				m_tutoTime = 0.0f;
				m_tutoTitleState = 1;
			}
			m_tutoX = startX;
			m_tutoY = startY;
		}
		else if (m_tutoTitleState == 1)
		{
			m_tutoTime += dt;
			m_tutoScale -= dt * 0.2f;
			if (m_tutoTime > 1.0f)
			{
				m_tutoTime = 1.0f;
				m_tutoScale = 1.0f;
				m_tutoTitleState = 2;
			}
			m_tutoX = (1.0f - m_tutoTime) * startX + endX * m_tutoTime;
			m_tutoY = startY * (1.0f - m_tutoTime) + 10.0f * m_tutoTime;
		}
		else if (m_tutoTitleState == 2)
		{
			m_tutoScale = 1.0f;
		}
	}

	if (!CCapturedBg::Instance()->IsCaptured())
	{
		Wind().Process(dt);
		if (m_pGroundInfo)
		{
			m_pGroundInfo->Process(dt);
		}
		if (m_pFontOverlay)
		{
			m_pFontOverlay->Process(dt);
		}
		if (m_pPangInfo)
		{
			m_pPangInfo->Process(dt);
		}
		if (m_pGuildInfo)
		{
			m_pGuildInfo->Process(dt);
		}
		// 983 (00752660): tipo de jogo 0xb -- placar grande do hole-in-one (+0x254) e CHoleEffect (+0x25c)
		if (m_pHoleInOneScoreInfo983)
		{
			m_pHoleInOneScoreInfo983->Process(dt);
		}
		if (m_pHoleEffect983)
		{
			m_pHoleEffect983->Process(dt);
		}

		m_cammodeAlpha = Min(dt * 7.0f + m_cammodeAlpha, 1.0f);

		if ((CLUB_FIELD(unsigned int, 0x40) >> 1) & 1)
		{
			m_pFxInterpreter983->DetachFx(m_clubBoundingBox);
			unsigned char type = GolfClub().GetType();
			sClubSet* pClubSet = GolfClub().FindClubSet( // 983 00752ceb: so procura (B909 tinha trocado por AddClubSet, que carrega no HUD)
				Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid);
			m_clubBoundingBox = m_pFxInterpreter983->ParseBoundingBoxScript_fx(
				pClubSet ? pClubSet->pHeadSub[type] : NULL);
		}

		if (CLUB_FIELD(unsigned char, 0x40) & 1)
		{
			m_clubAngle = 0.0f;
			m_clubSpinDir = 1;
			m_clubSwing = 0.0f;
		}
		else if (m_clubSpinDir == -1)
		{
			m_clubAngle -= dt * 21.991149f;
			if (m_clubAngle < 6.2831855f)
			{
				m_clubAngle = 6.2831855f;
				m_clubSpinDir = 0;
			}
		}
		else if (m_clubSpinDir == 0)
		{
			m_clubSwing += dt + dt;
		}
		else if (m_clubSpinDir == 1)
		{
			m_clubAngle += dt * 21.991149f;
			if (m_clubAngle > 6.8067842f)
			{
				m_clubAngle = 6.8067842f;
				m_clubSpinDir = -1;
			}
		}
	}
	else if (GOLFDOC()->m_shotPhase == 2)
	{
		m_hitBarTime += dt * 100.0f;
		if (m_hitBarTime > 20.0f)
		{
			m_hitBarTime -= 40.0f;
		}
	}

	// 983 (00752dee): o CCalipers escolhe o modo (item, S5, tipo de jogo) no proprio Process
	if (m_pCalipers)
	{
		m_pCalipers->Process(dt);
	}

	if (m_pAccuracyInfo)
	{
		m_pAccuracyInfo->Process(dt);
	}

	if (Doc()->m_golfGame.shotTimeLimit != 0)
	{
		if (GOLFDOC()->m_shotPhase == 0 && IsMyTurn(false))
		{
			if (GOLFDOC()->m_remainTime < 10000)
			{
				m_blinkTime += dt;
				if (m_blinkTime > 0.25f)
				{
					m_blinkTime -= 0.25f;
					m_hudVisible = !m_hudVisible;
				}
			}
			else
			{
				m_hudVisible = true;
			}
		}
		else
		{
			m_hudVisible = true;
			m_blinkTime = 0.0f;
		}
	}

	if (m_pMiniScore && GOLFDOC()->m_tutorialMode > 6)
	{
		m_pMiniScore->Process(dt);
	}

	if (GOLFDOC()->m_gameMode & 0x40300)
	{
		if (m_turnTime == GOLFDOC()->m_gameMode)
		{
			m_bannerTime = Min(dt * 512.0f + m_bannerTime, 255.0f);
		}
		else
		{
			m_turnTime = GOLFDOC()->m_gameMode;
			m_bannerTime = -255.0f;
		}
	}

	if (GolfBall().m_groundType == 2)
	{
		WVector pin = GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].pin;
		WVector dir;
		dir = pin - *(WVector*)&GolfBall();
		float angle = 0.69813168f -
			Abs(CalcDeltaAngle(dir, *(WVector*)GDOCPTR983(GOLFDOC(), 0x13c)));
		if (angle < 0.0f)
		{
			angle = 0.0f;
		}
		unsigned char alpha = (unsigned char)(angle * 1.4323945f * 128.0f);
		if ((CLUB_FIELD(unsigned int, 0x34) ^ CLUB_FIELD(unsigned int, 0x2c)) ==
			0xFFFFFFFF)
		{
			m_hitBarColor = (alpha << 24) | 0xFF96;
		}
		else
		{
			m_hitBarColor = (alpha << 24) | 0xFFF600;
		}
	}

	if (m_turnStarted)
	{
		m_turnStartTime += dt;
		if (m_turnStartTime > 5.0f)
		{
			m_turnStarted = false;
			MOUSECURSOR()->m_bArea[0x21] = true;
		}
	}

	if (!CQuitWindow::Instance()->IsActive())
	{
		switch (MOUSECURSOR()->GetArea())
		{
		case 5:
		{
			int result;
			SendMsg(this, "Item", M983(0x199), (int)&result, 0, 0, 0);
			if (result == 3)
			{
				SendMsg(this, "Player", M983(0x199), (int)&result, 0, 0, 0);
				if (result != 3)
				{
					if (m_pCalipers)
					{
						m_pCalipers->VisiblePowerBar(1);
					}
					if (g_input->GetButton(LEFT_BUTTON) == 1 &&
						((m_drawFlags >> 1) & 1) && IsMyTurn(false) &&
						GOLFDOC()->m_shotPhase == 0 && !GOLFDOC()->m_bPause &&
						!CChatMsg::Instance()->IsActive() &&
						GOLFDOC()->m_tutorialMode > 1 &&
						CLUB_FIELD(int, 0x54) == 2)
					{
						SendMsg(this, "GolfRule", M983(0x76), 0, 0, 0, 0);
					}
				}
			}
			break;
		}
		case 7:
			if (g_input->GetButton(LEFT_BUTTON) == 2 &&
				MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON) &&
				GOLFDOC()->m_tutorialMode == 15)
			{
				if (CChatMsg::Instance()->IsOpened())
				{
					CChatMsg::Instance()->Close(false, true);
				}
				else
				{
					CChatMsg::Instance()->Open(false, true);
				}
				SendMsg(this, "Item", M983(0x195), 0, 0, 0, 0);
			}
			break;
		case 10:
		case 11:
		case 12:
		case 13:
		{
			// 983 (00753331): clique em cima da janela do mensageiro nao vale para o placar
			FrMessengerDlg* pMsnDlg = CMessengerInfo::Instance()->GetMessengerDlg();
			if (pMsnDlg)
			{
				WRect msnRect = pMsnDlg->GetRect();
				if (msnRect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
				{
					break;
				}
			}
			bool busy;
			SendMsg(this, "Score", M983(0x139), (int)&busy, 0, 0, 0);
			if (!busy)
			{
				if (g_input->GetButton(RIGHT_BUTTON))
				{
					IActor* rule = GetActor("GolfRule");
					if (rule)
					{
						if (!CChatMsg::Instance()->IsOpened())
						{
							CChatMsg::Instance()->Open(true, true);
						}
						// 983 007533ad: +0x18 = info.sNick; o inicio do sUserInfo e roomIndex + sID
						MsgObject msg(NULL, 0x37,
							(int)Doc()->m_userInfo[(
								unsigned char)(MOUSECURSOR()->GetArea() - 10)].info.sNick,
							0, 0, 0, 0);
						rule << msg;
					}
				}

				if (g_input->GetButton(LEFT_BUTTON) == 2 &&
					MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON) &&
					OnlinePlay())
				{
					unsigned char index = MOUSECURSOR()->GetArea() - 10;
					if (PLAYER(index)->state != 3)
					{
						g_audio->PlaySfx("play_menu_open");
						MOUSECURSOR()->SetCamMode(false);
						std::map<unsigned long, sBriefUserInfo>::iterator it =
							Doc()->m_briefUserInfoMap.find(PLAYER(index)->oid);
						if (it != Doc()->m_briefUserInfoMap.end())
						{
							CUserInfo::Instance()->SetInfo(it->second.dwUid,
								PLAYER(index)->oid, true, true, false, false,
								it->second.sNick);
						}
					}
				}
			}
			break;
		}
		case 0x1f:
			if (g_input->GetButton(LEFT_BUTTON) == 2 &&
				MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON))
			{
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial)
				{
					MsgObject msg(NULL, M983(0x14b), 0, 0, 0, 0, 0);
					tutorial << msg;
				}
			}
			break;
		case 0x20:
			if (g_input->GetButton(LEFT_BUTTON) == 2 &&
				MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON))
			{
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial)
				{
					MsgObject msg(NULL, M983(0x14c), 0, 0, 0, 0, 0);
					tutorial << msg;
				}
			}
			break;
		case 0x21:
			if (g_input->GetButton(LEFT_BUTTON) == 2 &&
				MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON) &&
				GOLFDOC()->m_tutorialMode < 15 && !m_turnStarted)
			{
				m_turnStarted = true;
				m_turnStartTime = 0.0f;
				MOUSECURSOR()->m_bArea[0x21] = false;
			}
			break;
		case 0x24:
			if (g_input->GetButton(LEFT_BUTTON) == 2 &&
				MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON))
			{
				MOUSECURSOR()->m_bArea[0x24] = false;
				CreateForm<FrTrainingOptionDlg>(g_pFresh->GetManager(), this,
					"training", NULL)
					->Open(
						(FRESH_PFN_RESULT)&CScreen::OnTrainingOptionDlgResult,
						7);
			}
			break;
		}
	}

	if (CHelpMsg::Instance())
	{
		CHelpMsg::Instance()->Process(dt);
	}
	if (CHeadIcon::Instance())
	{
		CHeadIcon::Instance()->Process(dt);
	}

	if (IsMyTurn(true))
	{
		if (GolfClub().GetType() == 3)
		{
			HITBAR()->pangyaPos = BAR_START;
		}
		else
		{
			float power = 0.0f;
			SendMsg(this, "Player", M983(0x113), (int)&power, 0, 0, 0);

			if (GOLFDOC()->IsCurrentPlayerEvent983(6)) // 983 0075374e: evento 6 (o 645 testava o item 0x18000028)
			{
				power = (float)floor(0.0);
			}
			else
			{
				power = (float)floor((double)(power * 0.5f * g_RADTODEG));
			}

			if (Abs(power) < 2.5f)
			{
				power = 0.0f;
			}
			else if (power > 0.0f)
			{
				power -= 2.0f;
			}
			else
			{
				power += 2.0f;
			}

			float area = CPowerGauge::Instance()->GetPangYaArea();
			HITBAR()->pangyaPos =
				Between(area - GolfClub().GetNarrowAccuracy(), power,
					GolfClub().GetNarrowAccuracy() - area) +
				BAR_START;

			if (Abs(HITBAR()->hitPos - HITBAR()->pangyaPos) <
					GolfClub().GetNarrowAccuracy() &&
				GOLFDOC()->m_shotPhase == 2)
			{
				for (int i = 0; i < 3; i++)
				{
					if (Wabs(m_pangyaSparkle[i] - 255.0f) < g_EPSILON)
					{
						m_pangyaSparkle[i] = Random(225.0f, 255.0f); // 983 00753929
					}
				}
			}
			else
			{
				m_pangyaSparkle[0] = 255.0f;
				m_pangyaSparkle[1] = 255.0f;
				m_pangyaSparkle[2] = 255.0f;
			}
		}
	}

	if (m_pHolePoseSkins)
	{
		m_pHolePoseSkins->Process(dt);
	}

	if (m_fadeValue - dt > 0.0f)
	{
		m_fadeValue -= dt;
	}
	else
	{
		m_fadeValue = 0.0f;
	}

	// 983 (00752660): objetos novos 0x250..0x26c processam no fim do OnProcess
	if (m_pAutoFlag983)
	{
		m_pAutoFlag983->Process(dt);
	}
	if (m_pAutoShotButtons983)
	{
		int kind = m_pAutoShotButtons983->Process();
		if (kind)
		{
			SendMsg(this, "GolfRule", 0x329, kind, 0, 0, 0);
		}
	}
	if (m_pCurveArrow983)
	{
		m_pCurveArrow983->Process(dt);
	}
	if (m_pRecoverPangyaShotEffect983)
	{
		m_pRecoverPangyaShotEffect983->Process(dt);
	}
	if (m_pRecoverPangyaShotRingEffect983)
	{
		m_pRecoverPangyaShotRingEffect983->Process(dt);
	}

	// 983: relogio do Grand Prix (+0x519) pisca a cada +0x51c s enquanto ha tempo (+0x106a4)
	if (Doc()->m_golfGame.gpTimeLimit983 == 0)
	{
		m_gpBlinkOn983 = true;
	}
	else
	{
		m_gpBlinkTime983 += dt;
		if (m_gpBlinkTime983 > m_gpBlinkPeriod983)
		{
			m_gpBlinkTime983 -= m_gpBlinkPeriod983;
			m_gpBlinkOn983 = !m_gpBlinkOn983;
		}
	}
}

// 983 (00750b90): botoes "r_automatic_*_button" -- clique em 0x49/0x4a/0x4b manda 0x329 (0x10/0x20/0x40) ao GolfRule
int CAutoShotButtons983::Process()
{
	if (!m_on)
	{
		return 0;
	}
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	if (!ui->FindNamedRect(std::string("r_automatic_toma_button")) ||
		!ui->FindNamedRect(std::string("r_automatic_cobra_button")) ||
		!ui->FindNamedRect(std::string("r_automatic_spike_button")))
	{
		return 0;
	}
	m_value = GOLFDOC()->m_uiAlpha;
	if (!MOUSECURSOR()->IsInButtonDownArea(LEFT_BUTTON) || g_input->GetButton(LEFT_BUTTON) != 2)
	{
		return 0;
	}
	int kind;
	switch (MOUSECURSOR()->GetArea())
	{
	case 0x49:
		kind = 0x10;
		break;
	case 0x4a:
		kind = 0x20;
		break;
	case 0x4b:
		kind = 0x40;
		break;
	default:
		return 0;
	}
	return kind;
}

bool CScreen::OnTrainingOptionDlgResult(int result, FrForm* pForm)
{
	if (0)
		((CMiniScore*)0)->CMiniScore::~CMiniScore();
	MOUSECURSOR()->m_bArea[0x24] = true;
	return true;
}

// 983 (00633450): o jogador da vez usa a bola "ntball" (item 0xcc); escolhe a variante _ntball dos quadros do HUD
static bool IsNtBallItem983()
{
	return (Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall &
			   0x3ffffff) == 0xcc;
}

// 983 (007599xx): quadro do CUiFrameSetManager na tela atual
static __forceinline void DrawHudFrame983(const char* name, unsigned long color = 0xffffffff)
{
	CUiFrameSetManager::Instance()->DrawFrameSet(name, g_view, 0, color);
}

// 983 (00750a50/00753c30): so com a vez do jogador, fora dos modos 0xe0 e com tacada especial (power shot) escolhida
static __forceinline bool CanDrawShotButtons983()
{
	return IsMyTurn(true) && (*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x158)) & 0xe0) == 0 &&
		GolfClub().PowerShotType() != 0 && GolfClub().GetType() != 3;
}

// 983 (00750a50): descricao da tacada especial; so tacada de Tomahawk/Cobra tem a variante "toma_only"
void CScreen::DrawSpecialShotDesc983()
{
	if (!CanDrawShotButtons983() || !m_pAutoFlag983->m_on)
	{
		return;
	}
	unsigned long color = ((unsigned long)m_pAutoFlag983->m_value << 24) | 0xffffff;
	unsigned char type = GolfClub().GetType();
	DrawHudFrame983(type == 2 || type == 1 ? "f_special_shot_desc_toma_only" : "f_special_shot_desc", color);
}

// 983 (00753c30 + 00750e30/00750f90/007510f0): botoes automaticos toma/cobra/spike; estado 0 "n", 1 "o", 2 "d"
// 983 00750e30/00750f90/007510f0: um botao; estado 0 "n", 1 "o", 2 "d" (outro valor nao desenha)
void CAutoShotButtons983::DrawToma() // 00750e30
{
	switch (m_state[0])
	{
	case 0:
		DrawHudFrame983("f_automatic_toma_button_n", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 1:
		DrawHudFrame983("f_automatic_toma_button_o", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 2:
		DrawHudFrame983("f_automatic_toma_button_d", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	}
}

void CAutoShotButtons983::DrawCobra() // 00750f90
{
	switch (m_state[1])
	{
	case 0:
		DrawHudFrame983("f_automatic_cobra_button_n", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 1:
		DrawHudFrame983("f_automatic_cobra_button_o", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 2:
		DrawHudFrame983("f_automatic_cobra_button_d", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	}
}

void CAutoShotButtons983::DrawSpike() // 007510f0
{
	switch (m_state[2])
	{
	case 0:
		DrawHudFrame983("f_automatic_spike_button_n", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 1:
		DrawHudFrame983("f_automatic_spike_button_o", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	case 2:
		DrawHudFrame983("f_automatic_spike_button_d", ((unsigned long)m_value << 24) | 0xffffff);
		break;
	}
}

void CAutoShotButtons983::Draw()
{
	if (!CanDrawShotButtons983() || !m_on)
	{
		return;
	}
	DrawToma();
	unsigned char type = GolfClub().GetType();
	if (type != 2 && type != 1)
	{
		DrawCobra();
		DrawSpike();
	}
}

void CScreen::OnDisplay()
{
	if (!m_displayEnabled)
	{
		return;
	}

	// 983 (00759750): Grand Prix com gameMode 0x2000 so desenha o relogio do tempo limite (0074cd20)
	if (m_grandPrix983 && (GOLFDOC()->m_gameMode & 0x2000))
	{
		DrawGrandPrixTimer983();
		return;
	}

	// 983 (00759750): a HUD sai sem rim light; o estado anterior volta no fim (slots 0x188 IsRimLight / 0x174 SetRimLight)
	WVideoDev* videoDev = g_resrcmng->VideoReference();
	bool rimLight = videoDev->IsRimLight();
	videoDev->SetRimLight(false);

	WRect rect;
	m_remainTimeY = COption::Instance()->gGetBar_Y();

	if (!CCapturedBg::Instance()->IsCaptured())
	{
		rect = WRect(12.0f, m_remainTimeY - 64.0f, 128.0f, 64.0f);
		// 983 (00759750): botao de item/chat pelos quadros "f_item_chat_button_*" (variante _ntball da bola ntball)
		if (GOLFDOC()->m_tutorialMode == 15)
		{
			bool nt = IsNtBallItem983();
			if (CChatMsg::Instance()->IsActive())
			{
				DrawHudFrame983(nt ? "f_item_chat_button_chat_on_ntball"
								   : "f_item_chat_button_chat_on");
			}
			else if (CItemWindow::IsClosed())
			{
				DrawHudFrame983(nt ? "f_item_chat_button_both_off_ntball"
								   : "f_item_chat_button_both_off");
			}
			else
			{
				DrawHudFrame983(nt ? "f_item_chat_button_item_on_ntball"
								   : "f_item_chat_button_item_on");
			}
		}

		DrawWindInfo();
		DrawCammodeInfo();
		DrawClubInfo();

		// 983 (0074d160 / 0074d530): faixa inferior pelo estado do jogo (+0x10594), nao pelo gameType do 645
		if (!m_grandPrix983)
		{
			DrawMatchBand983();
			DrawMatchTime983();
		}
	}

	DrawApproachInfo();

	if (m_bHoleShown[1])
	{
		// 983 (00759750): fundo do relogio analogico pelo quadro "f_analog_timer_bg[_ntball]"
		DrawHudFrame983(IsNtBallItem983() ? "f_analog_timer_bg_ntball"
										  : "f_analog_timer_bg");
	}

	if (m_hudVisible && m_bHoleShown[1] &&
		CSharedDoc::Instance()->m_golfGame.gameType != 0xb) // 983: sem o ponteiro no modo 0xb
	{
		DrawRemainTime();
	}

	if (m_pAutoFlag983)
	{
		DrawSpecialShotDesc983();
	}
	if (m_pAutoShotButtons983)
	{
		m_pAutoShotButtons983->Draw();
	}

	if ((m_drawFlags & 1) && GOLFDOC()->m_tutorialMode >= 2 &&
		m_bHoleShown[2] && (!GOLFDOC()->m_bNoStat || IsMyTurn(true)))
	{
		DrawHitBarInfo();
	}

	if (!(GOLFDOC()->m_gameMode & 0x10))
	{
		if (GOLFDOC()->m_tutorialMode == 15 || GOLFDOC()->m_tutorialMode == 5)
		{
			SendMsg(this, "Item", M983(0x198), 0, 0, 0, 0);
		}
		CChatMsg::Instance()->PrintInScreen();
		CChatMsg::Instance()->DrawChatWindow();
		CChatMsg::Instance()->PrintInChatWindow();
	}

	if (m_bHoleShown[1])
	{
		// 983 (00754dc0): fundo da bola e do tipo de taco pelos quadros "f_ball_bg[_ntball]" e "f_club_type_bg"
		DrawHudFrame983(IsNtBallItem983() ? "f_ball_bg_ntball" : "f_ball_bg");
		DrawHudFrame983("f_club_type_bg");
		DrawClubBallUI();
	}

	if (GOLFDOC()->m_tutorialMode >= 12)
	{
		// 983 (00759750): fundo do relogio digital pelo quadro "f_digital_timer_bg"; o relogio so some no Grand Prix
		// (m_grandPrix983) sem o modo especial (+0x524)
		if (!m_grandPrix983 || m_gpTimedHole983)
		{
			DrawHudFrame983("f_digital_timer_bg");
		}

		if (m_pRemainTimeInfo && ((!m_grandPrix983) || m_gpTimedHole983))
		{
			m_pRemainTimeInfo->Display();
		}

		if (GolfClub().GetType() != 3)
		{
			// 983: o ponto de giro e as setas viram as medidas "d_spin_center_x/y" e "d_auto_spin_center_x/y"
			float x;
			float y;
			unsigned long color;
			CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
			SendMsg(this, "Spin", M983(0x167), (int)&x, (int)&y, (int)&color, 0);
			CUiFrameSetManager::NamedDim* sx = ui->FindNamedDim("d_spin_center_x");
			CUiFrameSetManager::NamedDim* sy = ui->FindNamedDim("d_spin_center_y");
			if (sx != NULL && sy != NULL)
			{
				sx->Set(x, 0);
				sy->Set(y, 0);
				ui->DrawFrameSet("f_spin", g_view, 0, color);
			}
			if (m_spinNotify)
			{
				SendMsg(this, "Spin", M983(0x168), (int)&x, (int)&y, (int)&color, 0);
				CUiFrameSetManager::NamedDim* ax = ui->FindNamedDim("d_auto_spin_center_x");
				CUiFrameSetManager::NamedDim* ay = ui->FindNamedDim("d_auto_spin_center_y");
				if (ax != NULL && ay != NULL)
				{
					ax->Set(x, 0);
					ay->Set(y, 0);
					ui->DrawFrameSet("f_auto_spin", g_view, 0, color);
				}
			}
		}
	}

	if (!CCapturedBg::Instance()->IsCaptured())
	{
		if (m_pFontOverlay)
		{
			m_pFontOverlay->Display();
		}

		if (m_pMiniScore && (m_bHoleShown[7] || m_bHoleShown[3]) &&
			!CProjectG::Instance()->HidePI())
		{
			m_pMiniScore->Display();
		}

		if (GOLFDOC()->m_tutorialMode == 15 && m_bannerTime > 0.0f)
		{
			switch (GOLFDOC()->m_gameMode)
			{
			case 0x40000:
				DrawOverlay("\xb3\xab\xc7\xcf\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb",
					WRect(0.0f, 29.0f, 256.0f, 64.0f),
					(unsigned char)m_bannerTime, 0);
				break;
			case 0x200:
				DrawOverlay("\xc5\xbe\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb",
					WRect(0.0f, 29.0f, 256.0f, 128.0f),
					(unsigned char)m_bannerTime, 0);
				break;
			case 0x100:
				DrawOverlay("\xc1\xb6\xc1\xbe\xba\xe4\xb5\xb5\xbf\xf2\xb8\xbb",
					WRect(0.0f, 29.0f, 256.0f, 128.0f),
					(unsigned char)m_bannerTime, 0);
				break;
			}
		}

		if (GOLFDOC()->m_gameMode != 0x10 && CHelpMsg::Instance())
		{
			CHelpMsg::Instance()->Display();
		}

		if (m_pPangInfo && !CProjectG::Instance()->HidePI())
		{
			m_pPangInfo->Display();
		}

		if (Doc()->m_gameMode == 2)
		{
			DrawOverlay("\xbf\xac\xbd\xc0\xb8\xf0\xb5\xe5\xbf\xc9\xbc\xc7",
				WRect(0.0f, 245.0f, 32.0f, 32.0f), 0xFF, 0);
		}

		// 983 (00759750): tipo 0xb -- placar grande (so com o bit 0x80 do modo) e CHoleEffect
		if (Doc()->m_golfGame.gameType == 0xb)
		{
			if ((GOLFDOC()->m_gameMode & 0x80) && m_pHoleInOneScoreInfo983)
			{
				m_pHoleInOneScoreInfo983->Display();
			}
			if (m_pHoleEffect983)
			{
				m_pHoleEffect983->Display();
			}
		}
	}

	DrawCommandArrow();

	if (m_pAccuracyInfo)
	{
		m_pAccuracyInfo->Display();
	}
	if (m_pHolePoseSkins)
	{
		m_pHolePoseSkins->Display();
	}
	if (m_pExtraHoleInfo)
	{
		m_pExtraHoleInfo->Display();
	}
	if (m_pGuildInfo)
	{
		m_pGuildInfo->Display();
	}

	if (m_playerCount > 0 && (GOLFDOC()->m_gameMode & 2) &&
		GOLFDOC()->m_bTeeShotReady)
	{
		char msg[32];
		sprintf(msg,
			"\xbe\xc6\xc1\xf7 %d\xb8\xed\xc0\xc7 \xc0\xaf\xc0\xfa\xb0\xa1 \xc1\xd8\xba\xf1\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.",
			m_playerCount);
		float width = m_pFont->GetTextWidthOld983(g_view, msg);
		m_pFont->Print(g_view, (g_view->GetWidth() - width) * 0.5f, 200.0f, msg,
			0, 0xFF000000, NULL);
	}

	DrawTutorialUI();

	// 983 (00759750, final): seta da curva e efeitos do "recover pangya shot" por cima de tudo
	if (m_pCurveArrow983)
	{
		m_pCurveArrow983->Display();
	}
	if (m_pRecoverPangyaShotEffect983)
	{
		m_pRecoverPangyaShotEffect983->Display();
	}
	if (m_pRecoverPangyaShotRingEffect983)
	{
		m_pRecoverPangyaShotRingEffect983->Display();
	}

	if (m_grandPrix983)
	{
		DrawGrandPrixTimer983();
	}

	videoDev->SetRimLight(rimLight);
}

void CScreen::AddOverlay(const char* name, const char* file, int flags)
{
	sOverlay* overlay = new sOverlay;
	overlay->m_pOverlay = g_resrcmng->GetOverlay(file, flags);
	if (overlay->m_pOverlay)
	{
		strcpy(overlay->m_name, name);
		m_overlays.AddItem(overlay, overlay->m_name, false);
	}
	else
	{
		delete overlay;
	}
}

void CScreen::DrawOverlay(const char* name, const WRect& rect,
	unsigned char alpha, int flags)
{
	if (alpha)
	{
		sOverlay* overlay = m_overlays.Find(name);
		if (overlay && overlay->m_pOverlay)
		{
			overlay->m_pOverlay->Render(g_view, g_rectUnitW983,
				rect, flags, (alpha << 24) | 0xffffff, 0.0f, 0);
		}
	}
}

void CScreen::DrawOverlay2(const char* name, const WRect& rect, int color,
	int flags)
{
	if (color & 0xff000000)
	{
		sOverlay* overlay = m_overlays.Find(name);
		if (overlay && overlay->m_pOverlay)
		{
			overlay->m_pOverlay->Render(g_view, g_rectUnitW983,
				rect, flags, color, 0.0f, 0);
		}
	}
}

void CScreen::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x64):
		if (m_pAuxMenuUI && m_pAuxMenuUI->GetChatFuncDlg())
		{
			m_pAuxMenuUI->GetChatFuncDlg()->AddChatMsg((const char*)msg.param1,
				msg.param2);
		}
		break;
	case M983(0x4c):
		if (m_pGroundInfo)
		{
			m_pGroundInfo->ChangeBall(msg.param1);
		}
		break;
	case M983(0x49):
		if (m_pMiniScore)
		{
			m_pMiniScore->ChangeCharacter();
		}
		break;
	case M983(0xa0):
		if (m_pCalipers)
		{
			m_pCalipers->Reset();
		}
		break;
	case M983(0x1a0):
		m_hitBarTime = 0.0f;
		if (CHelpMsg::Instance())
		{
			CHelpMsg::Instance()->OnChangeTurn();
		}
		if (m_pPangInfo)
		{
			m_pPangInfo->ChangeTurn();
		}
		if (m_pGroundInfo)
		{
			m_pGroundInfo->ChangeTurn();
		}
		if (m_pAuxMenuUI)
		{
			m_pAuxMenuUI->ChangeTurn();
		}
		if (m_pCalipers)
		{
			m_pCalipers->ResetBar();
		}
		break;
	case M983(0x190):
		m_displayEnabled = msg.param1 == 1;
		break;
	case M983(0xa1):
		if (m_pCalipers)
		{
			m_pCalipers->SetCalipersLimit(*(float*)msg.param1);
		}
		break;
	case M983(0x1a1):
		if (m_pGroundInfo)
		{
			m_accumValue += *(float*)msg.param1;
			m_pGroundInfo->SetAngle(m_accumValue);
		}
		break;
	case M983(0x1a3):
		if (m_pGroundInfo)
		{
			WVector normal = GetPVS().GetMeshNormal(GolfBall().m_meshIndex);
			m_pGroundInfo->SetNormal(normal);
		}
		if (m_pFontOverlay)
		{
			m_pFontOverlay->ResetHoleCupInfo();
		}
		m_hitBarColor = 0x8000FF96;
		m_ballStateFade = GetPVS().GetMaxPowerOnGround();
		if (m_ballStateFade < 1.0f)
		{
			m_ballStateFade -= 0.05f;
		}
		break;
	case 0x35e: // so do 983 (0074f761): o chao da bola mudou sem mudar a normal
		if (m_pGroundInfo)
		{
			m_pGroundInfo->UpdateGroundPenalty983();
		}
		GetPVS().SetCondition();
		m_hitBarColor = 0x8000FF96;
		m_ballStateFade = GetPVS().GetMaxPowerOnGround();
		if (m_ballStateFade < 1.0f)
		{
			m_ballStateFade -= 0.05f;
		}
		break;
	case M983(0x1a4):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->SetActive(CFontOverlay::INFO_HOLECUP,
				msg.param1 != 0);
		}
		break;
	case M983(0x1a5):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->SetActive(CFontOverlay::INFO_FLY, msg.param1 != 0);
		}
		break;
	case M983(0x1a7):
		m_drawFlags.Enable(1);
		break;
	case M983(0x1a8):
		m_drawFlags.Disable(1);
		break;
	case M983(0x1a6):
		m_fadeValue = 1.0f;
		break;
	case M983(0x1a9):
		m_remainArcColor983 = msg.param1; // 983: +0x310 (nao +0x2f4)
		m_remainArcStart983 = *(float*)msg.param2;
		if (m_pRemainTimeInfo)
		{
			m_pRemainTimeInfo->SetColor(m_remainArcColor983);
		}
		break;
	case M983(0x1aa):
		m_hitBarBaseColor = ULblend(0xFF016A96, 0xFFC81E42,
			float2int(*(float*)msg.param1 * 255.0f));
		break;
	case M983(0x1b9):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->ResetTimeOutInfo();
		}
		break;
	case M983(0x1ac):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->SetAccuracy();
		}
		break;
	case M983(0x1ab):
		if (m_pAccuracyInfo)
		{
			m_pAccuracyInfo->SetPower();
		}
		break;
	case M983(0x1ad):
		m_msgColorTime =
			0xFFFFFF - ((int)(*(float*)msg.param1 * -255.0f) << 24);
		m_msgColor = 0xFFFFFF - ((int)(*(float*)msg.param2 * -255.0f) << 24);
		break;
	case M983(0x1ae):
		m_hitBarFrom = msg.param1;
		m_hitBarCount = msg.param2;
		if (msg.param3)
		{
			// 983 (0074eccc): sem m_remainTimeY; a altura das marcas e relativa ao centro da barra (DrawHitBarInfo)
			int i;
			float y;
			float maxY = ((float*)msg.param3)[0];
			// one variable in the original: the lowest mark, then the scale
			float scale = m_hitMarks[0].y = maxY;
			for (i = 1; i < m_hitBarCount + 1; i++)
			{
				y = ((float*)msg.param3)[i];
				m_hitMarks[i].y = y;
				if (y < scale)
				{
					scale = y;
				}
				if (y > maxY)
				{
					maxY = y;
				}
			}

			float range = maxY - scale;
			float middle = 0.5f * range + scale;
			if (range > 1.6f)
			{
				scale = 1.6f / range;
			}
			else
			{
				scale = 1.0f;
			}

			m_hitMarks[0].y = (float)((middle - m_hitMarks[0].y) * 5.0 * scale);
			for (i = 1; i < m_hitBarCount + 1; i++)
			{
				m_hitMarks[i].y = (float)((middle - m_hitMarks[i].y) * 5.0 * scale);
				float slope =
					Between(-8.0f, m_hitMarks[i].y - m_hitMarks[0].y, 8.0f);
				if (slope > 0.0f)
				{
					int c = (int)(255.0f - 31.875f * slope);
					m_hitMarks[i].color = (c << 16) | c | 0xFF00E500;
				}
				else
				{
					int c = (int)(31.875f * slope + 255.0f);
					m_hitMarks[i].color = ((c | 0xFFFFFF00) << 8) | c;
				}
			}
		}
		break;
	case M983(0x1af):
		m_drawFlags.Enable(2);
		break;
	case M983(0x1b0):
		m_drawFlags.Disable(2);
		break;
	case M983(0x1b1):
		m_cammodeAlpha = 0.0f;
		break;
	case M983(0x1b2):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->ResetHoleInfo();
		}
		break;
	case M983(0x1b4):
		if (m_pPangInfo)
		{
			m_pPangInfo->PreCalc(msg.param1 != 0);
		}
		break;
	case M983(0x1b5):
		if (m_pPangInfo)
		{
			m_pPangInfo->Restore();
		}
		break;
	case M983(0x1b6):
		if (m_pPangInfo)
		{
			m_pPangInfo->Skip();
		}
		break;
	case M983(0x1b7):
		if (m_pPangInfo)
		{
			switch (Doc()->m_golfGame.gameType)
			{
			case 7:
				m_pPangInfo->ShowMyPang();
				break;
			case 1:
				m_pPangInfo->PreCalcTeamHoleInBonus();
				break;
			}
			SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
		}
		break;
	case M983(0x1b8):
		if (m_pPangInfo)
		{
			*(bool*)msg.param1 = m_pPangInfo->IsFinalized();
		}
		break;
	case M983(0x1b3):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->FixHoleInfo();
		}
		break;
	case M983(0x1ba):
		m_tutoBlinkOn = 0;
		if (msg.param1)
		{
			m_tutoBlinkTime = 0.0f;
			m_selectedIndex = GOLFDOC()->m_tutorialMode;
			if (GOLFDOC()->m_tutorialMode < 8 &&
				Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
			{
				m_turnStarted = true;
				m_turnStartTime = 0.0f;
				MOUSECURSOR()->m_bArea[0x21] = false;
			}
		}
		break;
	case M983(0x1d0):
		if (msg.param1 == 1)
		{
			GOLFDOC()->m_bDropEnable = 1;
		}
		else
		{
			GOLFDOC()->m_bDropEnable = 0;
		}
		break;
	case M983(0x1cf):
		m_playerCount = (unsigned char)msg.param1;
		break;
	case M983(0x1cb):
		// C751: 983 0074e8f7 so nestes modos de jogo (o 645 nao testava)
		if (m_pAuxMenuUI && GOLFDOC() && (GOLFDOC()->m_gameMode & 0x8108))
		{
			m_pAuxMenuUI->SetUnplayable();
		}
		break;
	case M983(0x1cc):
		if (m_pAuxMenuUI)
		{
			m_pAuxMenuUI->SetPause();
		}
		break;
	case M983(0x1cd):
		if (m_pAuxMenuUI)
		{
			m_pAuxMenuUI->OpenChatRecord();
		}
		break;
	case M983(0x1c9):
		if (m_pExtraHoleInfo)
		{
			m_pExtraHoleInfo->SetShotTime(GOLFDOC()->m_shotTime);
		}
		break;
	case M983(0x1ca):
		if (m_pExtraHoleInfo)
		{
			m_pExtraHoleInfo->UpdateLeader();
		}
		break;
	case M983(0x208):
		m_tutoTitleState = msg.param1 ? 0 : 3;
		break;
	case M983(0x1bb):
		if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC)
		{
			IActor* tutorial = GetActor("Tutorial");
			if (tutorial)
			{
				MsgObject start(NULL, M983(0x148), 0, 0, 0, 0, 0);
				tutorial << start;
			}
		}
		else
		{
			m_tutoTextState = 0;
			m_tutoTextTime = 0.0f;
			m_tutoTextAlpha = 0;
		}
		m_tutoStep = msg.param1;
		break;
	case M983(0x1bc):
	{
		switch (msg.param1)
		{
		case 2:
			m_bHoleShown[GOLFDOC()->m_tutorialMode] = true;
			break;
		case 1:
			m_bHoleShown[GOLFDOC()->m_tutorialMode] = false;
			m_holeBlinkCount = 3;
			break;
		case 0:
			m_bHoleShown[GOLFDOC()->m_tutorialMode] = false;
			m_holeBlinkTime = 0.0f;
			m_holeBlinkCount = 0;
			break;
		}
		for (int i = 0; i < 15; i++)
		{
			if (i < GOLFDOC()->m_tutorialMode)
			{
				m_bHoleShown[i] = true;
			}
		}
		break;
	}
	case M983(0x1bd):
		if (m_pMiniScore)
		{
			m_pMiniScore->Flicker();
		}
		break;
	case M983(0x1be):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->FlickerHoleInfo();
		}
		break;
	case M983(0x1bf):
		if (m_pAuxMenuUI)
		{
			m_pAuxMenuUI->SetActive(true, msg.param1);
		}
		break;
	case M983(0x1c0):
		if (m_pAuxMenuUI)
		{
			m_pAuxMenuUI->ResetSelected();
		}
		break;
	case M983(0x1c1):
		if (m_pGroundInfo)
		{
			*(WVector*)msg.param1 = m_pGroundInfo->GetBaseMatrix().ya;
		}
		break;
	case M983(0x1c2):
		*(float*)msg.param1 = m_ballStateFade;
		break;
	case M983(0x1c3):
		if (m_pMiniScore)
		{
			m_pMiniScore->Show();
		}
		break;
	case M983(0x1c4):
		if (m_pMiniScore)
		{
			m_pMiniScore->Hide();
		}
		break;
	case M983(0x1c5):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->FixHoleCupInfo();
		}
		break;
	case M983(0x1c6):
		if (m_pFontOverlay)
		{
			m_pFontOverlay->GetHoleInfoPos((float*)msg.param1,
				(float*)msg.param2);
		}
		break;
	case M983(0x1c7):
		if (m_pHolePoseSkins)
		{
			m_pHolePoseSkins->SetPose(msg.param1, msg.param2);
		}
		break;
	case M983(0x1c8):
		if (m_pMiniScore)
		{
			m_pMiniScore->StartFloating();
		}
		break;
	case M983(0x1ce):
		if (m_pGuildInfo)
		{
			m_pGuildInfo->SetScore(msg.param1, msg.param2);
		}
		break;
	case M983(0x202):
		jp983_rastro("B1422 Screen 0x27f lc30=%d ground=%p", (int)IsLocalContent((localContentType_t)0x30), m_pGroundInfo); // temporario
		if (IsLocalContent((localContentType_t)0x30 /*983: era 0x2f; interpolado*/))
		{
			// B1621: 983 0074e6d0 zera o bunker (+0x4da); o +0x4ca do 645 caia no meio do gauge cifrado (+0x4c8), o IsSafe falhava e
			// CheckCrypticValue punha g_bQuit: a queda silenciosa da asa
			PLAYER(GOLFDOC()->m_currentPlayer)->bunker = 0;
			m_pGroundInfo->SetNormal(WVector(WVector::UNIT_POS_Y));
			m_hitBarColor = 0x8000FF96;
			m_ballStateFade = GetPVS().GetMaxPowerOnGround();
			if (m_ballStateFade < 1.0f)
			{
				m_ballStateFade -= 0.05f;
			}
			GolfClub().SetAccuracy();
			// 983 0074e6d0 case 0x27f: avisa o Player antes de abrir o efeito; posicao em +0x110c (= CFxSequence::m_pos)
			jp983_rastro("B1422 Screen 0x27f accuracy ok");
			SendMsg(this, "Player", M983(0x203), 1, 0, 0, 0);
			jp983_rastro("B1422 Screen 0x27f player ok");
			CFxSequence* pSeq =
				CFx::Instance()->OpenSequence("safetee_ball.seq", true);
			if (pSeq)
			{
				pSeq->m_pos = GolfBall().m_pos;
			}
			jp983_rastro("B1422 Screen 0x27f fim seq=%p", pSeq);
		}
		break;
	case M983(0x1f0):
	{
		float intensity = (float)msg.param1;
		if (intensity >= 10.0f)
		{
			break;
		}
		Wind().SetIntensity(intensity);
		break;
	}
	case M983(0x1f1):
		Wind().SetDirection((float)msg.param1 * g_DEGTORAD);
		GOLFDOC()->m_pPolySoup->UpdateBgFxWind();
		break;
	case M983(0x1d1):
		m_spinNotify = msg.param1 == 1;
		break;
	case M983(0x20c):
	{
		WPolySoup::sPoint* pPoint =
			GOLFDOC()->m_pPolySoup->FindPoint((const char*)msg.param1);
		float radius = GolfBall().GetRadius();
		const WVector& ground =
			GetPVS().GetGroundPoint(pPoint->pos, false, NULL, false);
		WVector pos(ground.x, radius + ground.y, ground.z);
		GolfBall().m_pos = pos;
		GolfBall().m_meshIndex = GetPVS().GetMeshIndex(GolfBall().m_pos, false);
		WVector normal = GetPVS().GetMeshNormal(GolfBall().m_meshIndex);
		m_pGroundInfo->SetNormal(normal);
		break;
	}
	case 0x271b:
		m_remainTimeY = COption::Instance()->gGetBar_Y();
		if (m_pGroundInfo)
		{
			m_pGroundInfo->ResetUiPosition();
		}
		if (m_pPangInfo)
		{
			m_pPangInfo->ResetUiPosition();
		}
		if (m_pFontOverlay)
		{
			m_pFontOverlay->ResetUiPosition();
		}
		if (m_pMiniScore)
		{
			m_pMiniScore->ResetUiPosition();
		}
		if (m_pCurveArrow983)
		{
			m_pCurveArrow983->Reposition(); // 983 0074f82a
		}
		SendMsg(this, "Grid", M983(0x1d4), 0, 0, 0, 0);
		CHelpMsg::Instance()->ResetUiPosition();
		break;
	// mensagens que so existem no 983 (CScreen::HandleMsg 0074e6d0); numeros do 983 escritos direto
	case 0xbe: // 983 0074e806: le o float do CCalipers (0074e817 -> 00526e80, *this)
		if (m_pCalipers)
		{
			*(float*)msg.param1 = *(float*)m_pCalipers;
		}
		break;
	case 0xc7: // 983 0074e7ee: FxInterpreter::SetMode (0088c460)
		if (m_pFxInterpreter983)
		{
			m_pFxInterpreter983->SetMode((int)msg.param1);
		}
		break;
	case 0x247: // 983 0074f5a9 (0056f080 AddCount983); cai no 0x16f
		if (m_pHoleEffect983)
		{
			m_pHoleEffect983->AddCount983((int)msg.param1);
		}
	// fallthrough
	case 0x16f: // 983 0074f5ae: com p1 == 0 e a janela "holeinone_mission" criada (+0x258), SetCleared983(p2); cai no 0x313
		if (msg.param1 == 0 && m_pHoleinoneMissionDlg983)
		{
			m_pHoleinoneMissionDlg983->SetCleared983(*(const std::vector<unsigned long>*)msg.param2);
		}
	// fallthrough
	case 0x313: // 983 0074f5c7: liga/desliga o CScreenAutoFlag983
		if (m_pAutoFlag983)
		{
			m_pAutoFlag983->m_on = msg.param1 != 0;
		}
		break;
	case 0x245: // 983 0074f57a: CHoleEffect::Reset (vtable +0x10)
		if (m_pHoleEffect983)
		{
			m_pHoleEffect983->Reset();
		}
		break;
	case 0x246: // 983 0074f58b: CHoleEffect::Start983
		if (m_pHoleEffect983)
		{
			m_pHoleEffect983->Start983();
		}
		break;
	case 0x248: // 983 0074f5e7 (0074aa40): liga o relogio do GP
		m_gpClockShown983 = msg.param1 == 1;
		break;
	case 0x249: // 983 0074f60x: posicao do relogio do GP
	{
		const float* v = (const float*)msg.param1;
		m_gpUiPos983[0] = v[0];
		m_gpUiPos983[1] = v[1];
		break;
	}
	case 0x318: // 983: +0x4cc += p1 * 1000 (segundos a descontar do relogio GP)
		m_gpExtraElapsedMs983 += (int)msg.param1 * 1000;
		break;
	case 0x32f: // 983 0074aab0
		if (m_pAutoShotButtons983)
		{
			m_pAutoShotButtons983->SetOn(msg.param1 == 1);
		}
		break;
	case 0x330: // 983 0074aac0
		if (m_pAutoShotButtons983)
		{
			m_pAutoShotButtons983->SetButton((unsigned char)msg.param1, msg.param2 == 1);
		}
		break;
	case 0x331: // 983: CCurveArrow983 SetState(1) + SetType(p1)
		if (m_pCurveArrow983)
		{
			m_pCurveArrow983->SetState(1);
			m_pCurveArrow983->SetType((unsigned int)msg.param1);
		}
		break;
	case 0x332:
		if (m_pCurveArrow983)
		{
			m_pCurveArrow983->SetState(0);
		}
		break;
	case 0x334:
	case 0x35f: // 983 0074f6c0: som do rastro + liga o efeito de pegadas
		g_audio->PlaySfx("luck_foot_print", NULL, 0, 0, 0, 0.5f, 200.0f);
		if (m_pRecoverPangyaShotEffect983)
		{
			m_pRecoverPangyaShotEffect983->m_active = true;
		}
		break;
	case 0x35c: // 983 0074f708: anel do arco-iris: som no quadro 2 e liga
		if (m_pRecoverPangyaShotRingEffect983)
		{
			m_pRecoverPangyaShotRingEffect983->m_soundFrame = 2;
			m_pRecoverPangyaShotRingEffect983->m_active = true;
		}
		break;
	case 0x35d: // 983 0074f728
		g_audio->PlaySfx("caddie_xxx", NULL, 0, 0, 0, 0.5f, 200.0f);
		break;
	case 0x2718:
		if (m_pGroundInfo)
		{
			float angle = *(float*)msg.param1;
			m_accumValue = angle;
			m_pGroundInfo->SetAngle(angle);
		}
		break;
	case M983(0x20e):
		if (GOLFDOC()->m_tutorialMode == 11)
		{
			m_tuto11Triggered = true;
		}
		break;
	}
}

CMiniScore::CMiniScore()
	: m_state(1), m_alpha(0)
{
	if (0)
		((CMiniScore*)0)
			->CMiniScore::Process(
				0.0f); // HACK: emission order (wave 3 in the original)
	m_pInfo = NULL;
	m_pHp = NULL;
	m_pNPlayer = NULL;
	m_stateTime = 0.0f;
	unsigned char i;
	for (i = 0; i < 4; i++)
	{
		m_pCharBitmap[i] = NULL;
	}

	for (i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state == 3)
		{
			continue;
		}

		sPartTidHead& tidList = (sPartTidHead&)GOLFDOC()->GetPartTidList(i);
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(tidList.m_tidChar);
		if (pChar)
		{
			m_pCharBitmap[i] =
				g_pFresh->GetManager()->GetBitmap("ITEMS", pChar->c.Icon);
		}
	}

	// 983 (0074dd70): bitmap golf/premium_score (645: pc_icon_vs); guardado em +0x1c, usado no icone de premium (bit 0x4000) do Display
	m_pVsIcon = g_pFresh->GetManager()->GetBitmap("golf", "premium_score");

	m_pNPlayer = g_resrcmng->GetOverlay("[nplayer.jpg", 0);
	m_pHp = g_resrcmng->GetOverlay("info_hp.jpg", 0);

	m_pBestRecordL = g_resrcmng->GetOverlay("[best_record_l.png", 0);
	m_pBestRecordR = g_resrcmng->GetOverlay("[best_record_r.png", 0);

	if (Doc()->m_golfGame.gameType == 1)
	{
		m_pInfo = g_resrcmng->GetOverlay("[info_2p.jpg", 0);
	}
	else if (Doc()->m_golfGame.gameType == 7)
	{
		m_pInfo = g_resrcmng->GetOverlay("[skins_info_1p.jpg", 0);
	}
	else
	{
		m_pInfo = g_resrcmng->GetOverlay("[info_1p.jpg", 0);
	}

	m_pFont = CChatMsg::Instance()->GetMaskedFont();

	m_pShotCountInfo = new CShotCountInfo;

	m_pHoleOutUI = new CSimpleUI("[hole_out.jpg");
	m_pHoleOutUI->Add("HoleOut", WRect(0.0f, 0.0f, 128.0f, 32.0f), false);
	m_pHoleOutUI->Add("GiveUp", WRect(0.0f, 32.0f, 128.0f, 32.0f), false);
	m_pHoleOutUI->SetAngle(-0.52359879f);

	m_iconUV = WRect(0.0f, 0.0f, 0.46875f, 1.0f);
	m_iconRect.w = 120.0f;
	m_iconRect.h = (Doc()->m_golfGame.gameType == 7 ? 128.0f : 64.0f) *
		(Doc()->m_golfGame.gameType == 1 ? 2.0f : 1.0f);

	m_offsetY = 0.0f;
	m_posX = g_view->GetWidth() - 120.0f;
	m_gameMode = 0;
}

CMiniScore::~CMiniScore()
{
	if (m_pInfo != NULL)
	{
		g_resrcmng->Release(m_pInfo);
		m_pInfo = NULL;
	}

	if (m_pHp != NULL)
	{
		g_resrcmng->Release(m_pHp);
		m_pHp = NULL;
	}

	if (m_pNPlayer != NULL)
	{
		g_resrcmng->Release(m_pNPlayer);
		m_pNPlayer = NULL;
	}

	if (m_pBestRecordL != NULL)
	{
		g_resrcmng->Release(m_pBestRecordL);
		m_pBestRecordL = NULL;
	}

	if (m_pBestRecordR != NULL)
	{
		g_resrcmng->Release(m_pBestRecordR);
		m_pBestRecordL = NULL;
	}

	if (m_pShotCountInfo)
	{
		delete m_pShotCountInfo;
		m_pShotCountInfo = NULL;
	}
	if (m_pHoleOutUI)
	{
		delete m_pHoleOutUI;
		m_pHoleOutUI = NULL;
	}
}

void CMiniScore::ChangeCharacter()
{
	if (0)
		((CMiniScore*)0)
			->CMiniScore::Process(
				0.0f); // HACK: emission order (wave 3 in the original)
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb:
	{
		IFF_STRUCT::sChar* pChar =
			ItemManager()->FindChar(Doc()->m_userInfo[0].charInfo.tid);
		if (pChar)
		{
			m_pCharBitmap[0] =
				g_pFresh->GetManager()->GetBitmap("ITEMS", pChar->c.Icon);
		}
		else
		{
			m_pCharBitmap[0] = NULL;
		}
		break;
	}
	default:
		for (unsigned char i = 0; i < GOLFDOC()->GetPlayerNum(); i++)
		{
			IFF_STRUCT::sChar* pChar =
				ItemManager()->FindChar(Doc()->m_userInfo[i].charInfo.tid);
			if (pChar)
			{
				m_pCharBitmap[i] =
					g_pFresh->GetManager()->GetBitmap("ITEMS", pChar->c.Icon);
			}
			else
			{
				m_pCharBitmap[i] = NULL;
			}
		}
		break;
	}
}

WRect CMiniScore::GetIconRect(int flags)
{
	WRect r;
	r.x = (flags & 2) ? g_view->GetWidth() - 120.0f : 0.0f;
	r.y = (flags & 1) ? 124.0f : 30.0f;
	r.w = m_iconRect.w;
	r.h = m_iconRect.h;
	return r;
}

void CMiniScore::Process(float dt)
{
	if (m_gameMode != GOLFDOC()->m_gameMode)
	{
		if ((m_gameMode & ~0x38) &&
			((GOLFDOC()->m_gameMode & 8) ||
				(Doc()->m_golfGame.gameType == 7 &&
					(GOLFDOC()->m_gameMode & 0x1000))))
		{
			Show(); // 0074b1b0
		}
		else if ((GOLFDOC()->m_gameMode & ~0x3a) && (m_gameMode & 0x2a))
		{
			m_state = 3;
			m_stateTime = 0.0f;
			m_alpha = 255;
		}

		m_gameMode = GOLFDOC()->m_gameMode;
	}

	if (m_state == 0)
	{
		if (m_pShotCountInfo)
		{
			m_pShotCountInfo->Process(dt);
		}
	}

	if (m_state == 0 || m_state == 1)
	{
		return;
	}

	if (m_state == 2)
	{
		m_stateTime += dt * 2.0f;
		if (m_stateTime > 1.0f)
		{
			m_state = 0;
			m_stateTime = 1.0f;
		}

		m_alpha = (unsigned char)(m_stateTime * 255.0f);
	}
	else
	{
		m_stateTime += dt;
		if (m_stateTime > 1.0f)
		{
			m_state = 1;
			m_stateTime = 1.0f;
		}

		m_offsetY -= m_stateTime * 30.0f;
		m_posX += m_stateTime * 30.0f;
	}
}

void CMiniScore::Flicker()
{
	m_state = 0;
	m_stateTime = 1.0f;
	m_alpha = 255;
}

// HACK: the team branch of CMiniScore::Display passes m_currentPlayer to GetPlayer through an unsigned char
// temp (an inline accessor in the original). Any ordinary inline defined here is also emitted out of line
// (an extra COMDAT in the image); dllimport + __forceinline is the only form that is never emitted.
__declspec(dllimport) __forceinline unsigned char CurrentPlayer(CGolfDoc* doc)
{
	return doc->m_currentPlayer;
}

void CMiniScore::Display()
{
	if (0)
		((CAuxMenuUI*)0)
			->CAuxMenuUI::Process(
				0.0f); // HACK: emission order (wave 6 in the original)
	if (0)
		GetIconRect(
			0); // HACK: forces the emission of the inline naked GetIconRect (callers use aliases)
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) && Doc()->m_playingMode == 2)
	{
		GOLFDOC()->GetPlayer(0)->state = 0;
		Flicker();
	}

	if (m_state == 1)
		return;

	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (!device)
		return;

	bool busy = false;
	{
		MsgObject msg(NULL, M983(0x139), (int)&busy, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Score") << msg;
	}

	WRect src;
	WRect rect;
	unsigned char i;

	if (Doc()->m_golfGame.gameType == 1)
	{
		m_iconRect.y = 30.0f;

		for (i = 0; i < 2; i++)
		{
			if (i != 0)
			{
				src.x = 0.5f;
				m_iconRect.x = m_posX;
				m_charRect.x = m_posX;
			}
			else
			{
				src.x = 0.0f;
				m_iconRect.x = m_offsetY;
				m_charRect.x = m_offsetY + 120.0f - 46.0f;
			}

			src.y = 0.0f;
			src.w = m_iconUV.w;
			src.h = m_iconUV.h;
			m_charRect.y = m_iconRect.y + 14.0f;

			unsigned long color;
			if (i == GOLFDOC()->GetPlayer(CurrentPlayer(GOLFDOC()))->team &&
				!busy)
				color = m_alpha << 24;
			else
				color = (m_alpha >> 1) << 24;

			if (m_pInfo)
				m_pInfo->Render(g_view, src, m_iconRect, 0, color | 0xffffff, 0,
					0);

			int count = 0;
			for (unsigned char j = 0; j < GOLFDOC()->m_playerNum; j++)
			{
				if (GOLFDOC()->GetPlayer(j)->state == 3)
					continue;
				if (GOLFDOC()->GetPlayer(j)->team != i)
					continue;

				unsigned long alpha;
				if (j == GOLFDOC()->m_currentPlayer && !busy)
					alpha = m_alpha << 24;
				else
					alpha = (m_alpha >> 1) << 24;

				alpha |= 0xffffff;

				int index = j;
				if (m_pCharBitmap[index])
				{
					m_charRect.w = (float)m_pCharBitmap[index]->Width();
					m_charRect.h = (float)m_pCharBitmap[index]->Height();
					device->DrawTexture(m_pCharBitmap[index], m_charRect, alpha,
						0);
				}

				rect.x = (i ? m_posX : m_offsetY) + 5.0f;
				rect.y = count++ * 112.0f + m_iconRect.y + 2.0f;

				if (!CProjectG::Instance()->HidePrivacy())
				{
					g_pFresh->GetManager()->PrintText(WPoint(rect.x, rect.y), 0,
						Doc()->m_userInfo[j].info.sNick, -1.0f, alpha);
				}

				if (m_pVsIcon && Doc()->m_userInfo[j].info.IsIdentity(0x4000)) // 983: bit 14 do dwIdentity (shr 0xe, 00755770)
				{
					device->DrawTexture(m_pVsIcon,
						WRect(m_iconRect.x + 98.0f,
							m_iconRect.y - 2.0f + (index % 2) * 116.0f,
							(float)m_pVsIcon->Width(),
							(float)m_pVsIcon->Height()),
						alpha, 0);
				}

				if (CMouseCursor::Instance()->GetArea() == j + 10)
				{
					if (i)
						rect.x = m_posX - 256.0f + 6.0f;
					else
						rect.x = m_offsetY + 120.0f - 6.0f;
					rect.y = m_charRect.y;

					if ((&m_pBestRecordL)[i])
					{
						(&m_pBestRecordL)[i]->Render(g_view,
							WRect(0.0f, 0.0f, 1.0f, 1.0f),
							WRect(rect.x, rect.y, 256.0f, 128.0f), 0, alpha, 0,
							0);
					}

					rect.x += i ? 110.0f : 15.0f;
					alpha &= 0xff000000;

					m_pFont->Print(g_view, rect.x, rect.y + 20.0f,
						K2L_Compatibility("\xc8\xb9\xb5\xe6\xc6\xce :"), 0, alpha, NULL);
					m_pFont->Print(g_view, rect.x, rect.y + 37.0f,
						K2L_Compatibility("\xc3\xd6\xb4\xeb\xba\xf1\xb0\xc5\xb8\xae :"), 0, alpha,
						NULL);
					m_pFont->Print(g_view, rect.x, rect.y + 54.0f,
						K2L_Compatibility("\xc3\xd6\xb4\xeb\xc4\xa8\xc0\xce\xb0\xc5\xb8\xae :"), 0,
						alpha, NULL);
					m_pFont->Print(g_view, rect.x, rect.y + MINISCORE_PUTT_Y,
						K2L_Compatibility("\xc3\xd6\xb4\xeb\xc6\xdb\xc6\xc3\xb0\xc5\xb8\xae :"), 0,
						alpha, NULL);

					rect.x += 135.0f;

					char text[32];
					sprintf(text, "%dp",
						(int)GOLFDOC()->GetPlayer(j)->totalPang);
					rect.w = m_pFont->GetTextWidthOld983(g_view, text);
					m_pFont->Print(g_view, rect.x - rect.w, rect.y + 20.0f,
						text, 0, alpha, NULL);

					sprintf(text, "%.2fy", Doc()->m_holeStatistics[j].fLongest);
					rect.w = m_pFont->GetTextWidthOld983(g_view, text);
					m_pFont->Print(g_view, rect.x - rect.w, rect.y + 37.0f,
						text, 0, alpha, NULL);

					sprintf(text, "%.2fy",
						Doc()->m_holeStatistics[j].fLongestChipIn);
					rect.w = m_pFont->GetTextWidthOld983(g_view, text);
					m_pFont->Print(g_view, rect.x - rect.w, rect.y + 54.0f,
						text, 0, alpha, NULL);

					sprintf(text, "%.2fy",
						Doc()->m_holeStatistics[j].fLongestPuttIn);
					rect.w = m_pFont->GetTextWidthOld983(g_view, text);
					m_pFont->Print(g_view, rect.x - rect.w,
						rect.y + MINISCORE_PUTT_Y, text, 0, alpha, NULL);
				}

				m_charRect.y += 49.0f;
			}

			if ((float)GOLFDOC()->GetTeam(i)->gauge > 0.0f)
			{
				rect.y = m_iconRect.y + 61.0f;
				rect.h = 4.0f;

				unsigned long alpha;
				if (i == GOLFDOC()->GetPlayer(CurrentPlayer(GOLFDOC()))->team &&
					!busy)
					alpha = m_alpha << 24;
				else
					alpha = (m_alpha >> 1) << 24;

				unsigned char limit =
					(unsigned char)(Doc()->GetComboGaugeLimit(i) / 33.0f);
				for (unsigned char k = 0; k < limit; k++)
				{
					int width = (int)Min(33.0f,
						(float)GOLFDOC()->GetTeam(i)->gauge - k * 33.0f);
					if (width <= 0)
						break;

					rect.w = (float)width;
					unsigned char column;
					unsigned long color;
					if (k >= 3)
					{
						column = Min(k - 3, 0);
						color = rect.w == 33.0f ? 0xff : 0xc2ff;
					}
					else
					{
						column = k;
						color = rect.w == 33.0f ? 0xff0000 : 0xffc200;
					}

					rect.w *= 20.0f / 33.0f;
					color |= alpha;
					if (i & 1)
					{
						rect.x = column * 21.0f + 1.0f;
						rect.x = rect.x + (m_posX + 46.0f) + 5.0f;
					}
					else
						rect.x = m_offsetY + 120.0f - 46.0f - 5.0f - rect.w -
							(column * 21.0f + 1.0f);

					if (m_pHp)
						m_pHp->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							rect, 0, color, 0, 0);
				}
			}

			unsigned long alpha;
			if (i == GOLFDOC()->GetPlayer(CurrentPlayer(GOLFDOC()))->team &&
				!busy)
				alpha = m_alpha << 24;
			else
				alpha = (m_alpha >> 1) << 24;

			alpha |= 0xffffff;
			rect.x = (i ? m_posX + 46.0f : m_offsetY) + 56.0f;
			if (m_pShotCountInfo)
			{
				m_pShotCountInfo->SetColor(alpha);
				m_pShotCountInfo->Display(rect.x, m_iconRect.y + 69.0f, i);
			}

			m_pFont->Flush(g_view);

			float x;
			if (!i)
				x = m_offsetY + 35.0f;
			else
				x = m_posX + 82.0f;

			if (m_pHoleOutUI)
			{
				m_pHoleOutUI->SetColor(
					((unsigned char)(m_alpha * 0.5f) << 24) | 0xffffff);

				if (GOLFDOC()->GetTeam(i)->state == 1)
				{
					sHoleData& hole = GOLFDOC()->GetHoleData();
					sTeamData* data = GOLFDOC()->GetTeam(i);
					if ((unsigned char)
							data->stroke[GOLFDOC()->m_currentHole - 1] >
						(unsigned char)(hole.par + 4))
						m_pHoleOutUI->Render("GiveUp", x, 80.0f, 1.0f, 0x22);
					else
					{
						sPlayerData* player = GOLFDOC()->GetPlayer(i);
						if ((unsigned char)player
								->stroke[GOLFDOC()->m_currentHole - 1] > 0)
							m_pHoleOutUI->Render("HoleOut", x, 80.0f, 1.0f,
								0x22);
					}
				}
				else if (i ==
						GOLFDOC()->GetPlayer(CurrentPlayer(GOLFDOC()))->team &&
					GOLFDOC()->GetPlayer(CurrentPlayer(GOLFDOC()))->state == 0)
				{
					if (GOLFDOC()->m_gameMode & 0x1000)
						m_pHoleOutUI->Render("HoleOut", x, 80.0f, 1.0f, 0x22);
					else if (GOLFDOC()->m_gameMode & 0x80000)
						m_pHoleOutUI->Render("GiveUp", x, 80.0f, 1.0f, 0x22);
				}
			}
		}
	}
	else if (Doc()->m_golfGame.gameType == 7)
	{
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (GOLFDOC()->GetPlayer(i)->state == 3)
				continue;

			unsigned char lower = i & 1;
			m_iconRect.y = lower ? MINISCORE_LOWER_Y : 30.0f;
			unsigned char right = i & 2;

			if (right)
			{
				src.x = 0.5f;
				m_iconRect.x = m_posX;
				m_charRect.x = m_posX;
			}
			else
			{
				src.x = 0.0f;
				m_iconRect.x = m_offsetY;
				m_charRect.x = m_offsetY + 120.0f - 46.0f;
			}
			m_charRect.y = m_iconRect.y + 14.0f;

			unsigned long alpha;
			if (i == GOLFDOC()->m_currentPlayer && !busy)
				alpha = m_alpha << 24;
			else
				alpha = (m_alpha >> 1) << 24;

			unsigned long diffuse = alpha | 0xffffff;
			src.y = 0.0f;
			src.w = m_iconUV.w;
			src.h = m_iconUV.h;

			if (m_pInfo)
				m_pInfo->Render(g_view, src, m_iconRect, 0, diffuse, 0, 0);

			if (m_pCharBitmap[i])
			{
				m_charRect.w = (float)m_pCharBitmap[i]->Width();
				m_charRect.h = (float)m_pCharBitmap[i]->Height();
				device->DrawTexture(m_pCharBitmap[i], m_charRect, diffuse, 0);
			}

			rect.x = (right ? m_posX : m_offsetY) + 25.0f;
			rect.y = m_iconRect.y + 2.0f;

			if (!CProjectG::Instance()->HidePrivacy())
			{
				g_pFresh->GetManager()->PrintText(WPoint(rect.x, rect.y), 0,
					Doc()->m_userInfo[i].info.sNick, 84.0f, diffuse);
			}

			if (m_pVsIcon && Doc()->m_userInfo[i].info.IsIdentity(0x4000)) // 983: bit 14 do dwIdentity (shr 0xe, 00755770)
			{
				device->DrawTexture(m_pVsIcon,
					WRect(m_iconRect.x + 98.0f, m_iconRect.y - 2.0f,
						(float)m_pVsIcon->Width(), (float)m_pVsIcon->Height()),
					diffuse, 0);
			}

			if ((float)GOLFDOC()->GetPlayer(i)->gauge > 0.0f)
			{
				rect.y = m_iconRect.y + 21.0f;
				rect.h = 4.0f;

				unsigned long color;
				if (i == GOLFDOC()->m_currentPlayer && !busy)
					color = m_alpha << 24;
				else
					color = (m_alpha >> 1) << 24;

				unsigned char limit =
					(unsigned char)(Doc()->GetComboGaugeLimit(i) / 33.0f);
				for (unsigned char k = 0; k < limit; k++)
				{
					int width = (int)Min(33.0f,
						(float)GOLFDOC()->GetPlayer(i)->gauge - k * 33.0f);
					if (width <= 0)
						break;

					rect.w = (float)width;
					unsigned char column;
					unsigned long bar;
					if (k >= 3)
					{
						column = Min(k - 3, 0);
						bar = rect.w == 33.0f ? 0xff : 0xc2ff;
					}
					else
					{
						column = k;
						bar = rect.w == 33.0f ? 0xff0000 : 0xffc200;
					}

					rect.w *= 20.0f / 33.0f;
					bar |= color;
					if (right)
					{
						rect.x = column * 21.0f + 1.0f;
						rect.x = rect.x + (m_posX + 46.0f) + 5.0f;
					}
					else
						rect.x = m_offsetY + 120.0f - 46.0f - 5.0f - rect.w -
							(column * 21.0f + 1.0f);

					if (m_pHp)
						m_pHp->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							rect, 0, bar, 0, 0);
				}
			}

			unsigned long tint;
			if (i == GOLFDOC()->m_currentPlayer && !busy)
				tint = m_alpha << 24;
			else
				tint = (m_alpha >> 1) << 24;
			unsigned long playerColor = GOLFDOC()->GetPlayer(i)->color | tint;

			src.x = lower ? 0.5f : 0.0f;
			src.y = right ? 0.25f : 0.0f;
			src.w = 0.5f;
			src.h = 0.25f;
			rect.x = (right ? m_posX : m_offsetY) - 4.0f;
			rect.w = 32.0f;
			rect.h = 16.0f;
			rect.y = m_iconRect.y - 3.0f;

			if (m_pNPlayer)
				m_pNPlayer->Render(g_view, src, rect, 0, playerColor, 0, 0);

			if (i == GOLFDOC()->m_currentPlayer && !busy)
				alpha = m_alpha << 24;
			else
				alpha = (m_alpha >> 1) << 24;

			alpha |= 0xffffff;
			rect.x = (right ? m_posX + 46.0f : m_offsetY) + 56.0f;
			if (m_pShotCountInfo)
			{
				m_pShotCountInfo->SetColor(alpha);
				m_pShotCountInfo->Display(rect.x, m_iconRect.y + 29.0f, i);
			}

			m_pFont->Flush(g_view);

			float x;
			if (right)
				x = m_posX + 62.0f;
			else
				x = m_offsetY + 51.0f;
			float y = lower ? 157.0f : 62.0f;

			if (m_pHoleOutUI)
			{
				m_pHoleOutUI->SetColor(
					((unsigned char)(m_alpha * 0.5f) << 24) | 0xffffff);

				if (GOLFDOC()->GetPlayer(i)->state == 1)
				{
					sHoleData& hole = GOLFDOC()->GetHoleData();
					sPlayerData* data = GOLFDOC()->GetPlayer(i);
					if ((unsigned char)
							data->stroke[GOLFDOC()->m_currentHole - 1] >
						(unsigned char)(hole.par + 4))
						m_pHoleOutUI->Render("GiveUp", x, y, 1.0f, 0x22);
					else
					{
						sPlayerData* player = GOLFDOC()->GetPlayer(i);
						if ((unsigned char)player
								->stroke[GOLFDOC()->m_currentHole - 1] > 0)
							m_pHoleOutUI->Render("HoleOut", x, y, 1.0f, 0x22);
					}
				}
				else if (i == GOLFDOC()->m_currentPlayer &&
					GOLFDOC()->GetPlayer(i)->state == 0)
				{
					if (GOLFDOC()->m_gameMode & 0x1000)
						m_pHoleOutUI->Render("HoleOut", x, y, 1.0f, 0x22);
					else if (GOLFDOC()->m_gameMode & 0x80000)
						m_pHoleOutUI->Render("GiveUp", x, y, 1.0f, 0x22);
				}
			}
		}
	}
	else
	{
		for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (GOLFDOC()->GetPlayer(i)->state == 3)
				continue;

			unsigned char lower = i & 1;
			m_iconRect.y = lower ? MINISCORE_LOWER_Y : 30.0f;
			unsigned char right = i & 2;

			if (right)
			{
				src.x = 0.5f;
				m_iconRect.x = m_posX;
				m_charRect.x = m_posX;
			}
			else
			{
				src.x = 0.0f;
				m_iconRect.x = m_offsetY;
				m_charRect.x = m_offsetY + 120.0f - 46.0f;
			}
			m_charRect.y = m_iconRect.y + 14.0f;

			unsigned long alpha;
			if (i == GOLFDOC()->m_currentPlayer && !busy)
				alpha = m_alpha << 24;
			else
				alpha = (m_alpha >> 1) << 24;

			unsigned long diffuse = alpha | 0xffffff;
			src.y = 0.0f;
			src.w = m_iconUV.w;
			src.h = m_iconUV.h;

			if (m_pInfo)
				m_pInfo->Render(g_view, src, m_iconRect, 0, diffuse, 0, 0);

			unsigned int index = i;
			if (m_pCharBitmap[index])
			{
				m_charRect.w = (float)m_pCharBitmap[index]->Width();
				m_charRect.h = (float)m_pCharBitmap[index]->Height();
				device->DrawTexture(m_pCharBitmap[index], m_charRect, diffuse,
					0);
			}

			rect.x = (right ? m_posX : m_offsetY) + 25.0f;
			rect.y = m_iconRect.y + 2.0f;

			// 983 (00755770): o apelido some nas salas realGameType 0xc/0xe/0x13
			if (!CProjectG::Instance()->HidePrivacy() &&
				Doc()->m_roomInfo.realGameType != 0xc &&
				Doc()->m_roomInfo.realGameType != 0xe &&
				Doc()->m_roomInfo.realGameType != 0x13)
			{
				g_pFresh->GetManager()->PrintText(WPoint(rect.x, rect.y), 0,
					Doc()->m_userInfo[index].info.sNick, 84.0f, diffuse);
			}

			if (m_pVsIcon && Doc()->m_userInfo[index].info.IsIdentity(0x4000)) // 983: bit 14 do dwIdentity (shr 0xe, 00755770)
			{
				device->DrawTexture(m_pVsIcon,
					WRect(m_iconRect.x + 98.0f, m_iconRect.y - 2.0f,
						(float)m_pVsIcon->Width(), (float)m_pVsIcon->Height()),
					diffuse, 0);
			}

			if ((float)GOLFDOC()->GetPlayer(i)->gauge > 0.0f)
			{
				rect.y = m_iconRect.y + 21.0f;
				rect.h = 4.0f;

				unsigned long color;
				if (i == GOLFDOC()->m_currentPlayer && !busy)
					color = m_alpha << 24;
				else
					color = (m_alpha >> 1) << 24;

				unsigned char limit =
					(unsigned char)(Doc()->GetComboGaugeLimit(i) / 33.0f);
				for (unsigned char k = 0; k < limit; k++)
				{
					int width = (int)Min(33.0f,
						(float)GOLFDOC()->GetPlayer(i)->gauge - k * 33.0f);
					if (width <= 0)
						break;

					rect.w = (float)width;
					unsigned char column;
					unsigned long bar;
					if (k >= 3)
					{
						column = Min(k - 3, 0);
						bar = rect.w == 33.0f ? 0xff : 0xc2ff;
					}
					else
					{
						column = k;
						bar = rect.w == 33.0f ? 0xff0000 : 0xffc200;
					}

					rect.w *= 20.0f / 33.0f;
					bar |= color;
					if (right)
					{
						rect.x = column * 21.0f + 1.0f;
						rect.x = rect.x + (m_posX + 46.0f) + 5.0f;
					}
					else
						rect.x = m_offsetY + 120.0f - 46.0f - 5.0f - rect.w -
							(column * 21.0f + 1.0f);

					if (m_pHp)
						m_pHp->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							rect, 0, bar, 0, 0);
				}
			}

			if (i == MyIndex())
			{
				int active = 0;
				float gauge = 0.0f;
				MsgObject msg(NULL, M983(0x129), (int)&active, (int)&gauge, 0, 0, 0);
				AfxGetTask()->GetActor("Player") << msg;

				if (active)
				{
					rect.y = m_iconRect.y + 21.0f;
					rect.h = 4.0f;

					for (unsigned char k = 0; k < 3; k++)
					{
						float column = k;
						int width = (int)Min(33.0f, gauge - 33.0f * column);
						if (width <= 0)
							break;

						float w = (float)width;
						unsigned long bar;
						if (w == 33.0f)
						{
							if (i == GOLFDOC()->m_currentPlayer && !busy)
								bar = m_alpha << 24;
							else
								bar = (m_alpha >> 1) << 24;
							bar |= 0xff0000;
						}
						else
						{
							if (i == GOLFDOC()->m_currentPlayer && !busy)
								bar = m_alpha << 24;
							else
								bar = (m_alpha >> 1) << 24;
							bar |= 0xffc200;
						}
						rect.w = w * (20.0f / 33.0f);
						if (right)
						{
							rect.x = column * 21.0f + 1.0f;
							rect.x = rect.x + (m_posX + 46.0f) + 5.0f;
						}
						else
							rect.x = m_offsetY + 120.0f - 46.0f - 5.0f -
								rect.w - (column * 21.0f + 1.0f);

						if (m_pHp)
							m_pHp->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
								rect, 0, bar, 0, 0);
					}
				}
			}

			unsigned long tint;
			if (i == GOLFDOC()->m_currentPlayer && !busy)
				tint = m_alpha << 24;
			else
				tint = (m_alpha >> 1) << 24;
			unsigned long playerColor = GOLFDOC()->GetPlayer(i)->color | tint;

			src.x = lower ? 0.5f : 0.0f;
			src.y = right ? 0.25f : 0.0f;
			src.w = 0.5f;
			src.h = 0.25f;
			rect.x = (right ? m_posX : m_offsetY) - 4.0f;
			rect.w = 32.0f;
			rect.h = 16.0f;
			rect.y = m_iconRect.y - 3.0f;

			if (m_pNPlayer)
				m_pNPlayer->Render(g_view, src, rect, 0, playerColor, 0, 0);

			if (i == GOLFDOC()->m_currentPlayer && !busy)
				alpha = m_alpha << 24;
			else
				alpha = (m_alpha >> 1) << 24;

			alpha |= 0xffffff;
			rect.x = (right ? m_posX + 46.0f : m_offsetY) + 56.0f;
			if (m_pShotCountInfo)
			{
				m_pShotCountInfo->SetColor(alpha);
				m_pShotCountInfo->Display(rect.x, m_iconRect.y + 29.0f, i);
			}

			if (CMouseCursor::Instance()->GetArea() == index + 10)
			{
				if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
					Doc()->m_playingMode == 2)
					break;

				WRect board;
				if (right)
					board.x = m_posX - 256.0f + 6.0f;
				else
					board.x = m_offsetY + 120.0f - 6.0f;
				board.y = m_iconRect.y;
				board.w = 256.0f;
				board.h = 128.0f;
				rect.x = board.x;
				rect.y = board.y;

				(&m_pBestRecordL)[(index >> 1) & 1]->Render(g_view,
					WRect(0.0f, 0.0f, 1.0f, 1.0f), board, 0, alpha, 0, 0);

				rect.x += right ? 110.0f : 15.0f;
				alpha &= 0xff000000;

				m_pFont->Print(g_view, rect.x, rect.y + 20.0f,
					K2L_Compatibility("\xc8\xb9\xb5\xe6\xc6\xce :"), 0, alpha, NULL);

				switch (Doc()->m_golfGame.gameType)
				{
				case 4:
				case 5:
				case 6:
				case 9:
				case 10:
					m_pFont->Print(g_view, rect.x, rect.y + 37.0f,
						K2L_Compatibility("\xb4\xa9\xc0\xfb\xba\xf1\xb0\xc5\xb8\xae :"), 0, alpha,
						NULL);
					break;
				default:
					m_pFont->Print(g_view, rect.x, rect.y + 37.0f,
						K2L_Compatibility("\xc3\xd6\xb4\xeb\xba\xf1\xb0\xc5\xb8\xae :"), 0, alpha,
						NULL);
					break;
				}
				m_pFont->Print(g_view, rect.x, rect.y + 54.0f,
					K2L_Compatibility("\xc3\xd6\xb4\xeb\xc4\xa8\xc0\xce\xb0\xc5\xb8\xae :"), 0,
					alpha, NULL);
				m_pFont->Print(g_view, rect.x, rect.y + MINISCORE_PUTT_Y,
					K2L_Compatibility("\xc3\xd6\xb4\xeb\xc6\xdb\xc6\xc3\xb0\xc5\xb8\xae :"), 0,
					alpha, NULL);

				rect.x += 135.0f;

				char text[32];
				sprintf(text, "%dp", (int)GOLFDOC()->GetPlayer(i)->totalPang);
				rect.w = m_pFont->GetTextWidthOld983(g_view, text);
				m_pFont->Print(g_view, rect.x - rect.w, rect.y + 20.0f, text, 0,
					alpha, NULL);

				switch (Doc()->m_golfGame.gameType)
				{
				case 4:
				case 5:
				case 6:
				case 9:
				case 10:
					sprintf(text, "%dy",
						Doc()->m_holeStatistics[index].dwDistance);
					break;
				default:
					sprintf(text, "%.2fy",
						Doc()->m_holeStatistics[index].fLongest);
					break;
				}
				rect.w = m_pFont->GetTextWidthOld983(g_view, text);
				m_pFont->Print(g_view, rect.x - rect.w, rect.y + 37.0f, text, 0,
					alpha, NULL);

				sprintf(text, "%.2fy",
					Doc()->m_holeStatistics[index].fLongestChipIn);
				rect.w = m_pFont->GetTextWidthOld983(g_view, text);
				m_pFont->Print(g_view, rect.x - rect.w, rect.y + 54.0f, text, 0,
					alpha, NULL);

				sprintf(text, "%.2fy",
					Doc()->m_holeStatistics[index].fLongestPuttIn);
				rect.w = m_pFont->GetTextWidthOld983(g_view, text);
				m_pFont->Print(g_view, rect.x - rect.w,
					rect.y + MINISCORE_PUTT_Y, text, 0, alpha, NULL);
			}

			m_pFont->Flush(g_view);

			float x;
			if (right)
				x = m_posX + 62.0f;
			else
				x = m_offsetY + 51.0f;
			float y = lower ? 157.0f : 62.0f;

			if (m_pHoleOutUI)
			{
				m_pHoleOutUI->SetColor(
					((unsigned char)(m_alpha * 0.5f) << 24) | 0xffffff);

				if (GOLFDOC()->GetPlayer(i)->state == 1)
				{
					sHoleData& hole = GOLFDOC()->GetHoleData();
					sPlayerData* data = GOLFDOC()->GetPlayer(i);
					if ((unsigned char)
							data->stroke[GOLFDOC()->m_currentHole - 1] >
						(unsigned char)(hole.par + 4))
						m_pHoleOutUI->Render("GiveUp", x, y, 1.0f, 0x22);
					else
					{
						sPlayerData* player = GOLFDOC()->GetPlayer(i);
						if ((unsigned char)player
								->stroke[GOLFDOC()->m_currentHole - 1] > 0)
							m_pHoleOutUI->Render("HoleOut", x, y, 1.0f, 0x22);
					}
				}
				else if (i == GOLFDOC()->m_currentPlayer &&
					GOLFDOC()->GetPlayer(i)->state == 0)
				{
					if (GOLFDOC()->m_gameMode & 0x1000)
						m_pHoleOutUI->Render("HoleOut", x, y, 1.0f, 0x22);
					else if (GOLFDOC()->m_gameMode & 0x80000)
						m_pHoleOutUI->Render("GiveUp", x, y, 1.0f, 0x22);
				}
			}
		}
	}

	if (CHeadIcon::Instance())
		CHeadIcon::Instance()->DisplayBesideBoard(m_offsetY + 120.0f, m_posX);
}

void CMiniScore::ResetUiPosition()
{
	if (m_state == 0 || m_state == 2)
	{
		m_posX = g_view->GetWidth() - 120.0f;
	}
}

void CMiniScore::Show()
{
	m_state = 2;
	m_stateTime = 0.0f;
	m_offsetY = 0.0f;
	m_posX = g_view->GetWidth() - 120.0f;
	m_alpha = 0;
}

void CMiniScore::Hide()
{
	m_state = 1;
	m_stateTime = 1.0f;
	m_offsetY = -120.0f;
	m_posX = g_view->GetWidth() - 120.0f + 120.0f;
	m_alpha = 255;
}

void CMiniScore::StartFloating()
{
	if (m_pShotCountInfo)
	{
		m_pShotCountInfo->StartFloating();
	}
}

CAuxMenuUI::CAuxMenuUI()
	: m_pBase(NULL), m_dropDlg(0), m_dropFailDlg(0), m_pChatFuncDlg(NULL)
{
	m_pBase = g_resrcmng->GetOverlay("[option_base.png", 0x80000000);
	m_pBlack = g_resrcmng->GetOverlay("[option_black.png", 0x80000000);
	m_pWhite = g_resrcmng->GetOverlay("[option_white.png", 0x80000000);
	m_active = 0;
	m_slide = 0.0f;
	m_pauseCount = 2;
}

CAuxMenuUI::~CAuxMenuUI()
{
	if (g_resrcmng)
	{
		if (m_pBase)
		{
			g_resrcmng->Release(m_pBase);
			m_pBase = NULL;
		}
		if (g_resrcmng)
		{
			if (m_pBlack)
			{
				g_resrcmng->Release(m_pBlack);
				m_pBlack = NULL;
			}
			if (g_resrcmng && m_pWhite)
			{
				g_resrcmng->Release(m_pWhite);
				m_pWhite = NULL;
			}
		}
	}
}

void CAuxMenuUI::Process(float dt)
{
	m_hover = MOUSECURSOR()->GetArea() - 0x27;

	switch (m_active)
	{
	case 0:
		if (MOUSECURSOR()->GetArea() == CMouseCursor::AREA_AUXMENU_OPEN &&
			g_input->GetButton(LEFT_BUTTON) == 2)
		{
			m_active = 1;
		}
		break;
	case 1:
		m_slide += dt * 5.0f;
		if (m_slide > 1.0f)
		{
			m_slide = 1.0f;
			m_active = 2;
			MOUSECURSOR()->m_bArea[0x27] = true;
			MOUSECURSOR()->m_bArea[0x28] = true;
			MOUSECURSOR()->m_bArea[0x29] = true;
			MOUSECURSOR()->m_bArea[0x2a] = true;
			MOUSECURSOR()->m_bArea[0x2b] = true;
			MOUSECURSOR()->m_bArea[0x2c] = true;
		}
		break;
	case 2:
		if (m_hover < 0 || m_hover > 5)
		{
			MOUSECURSOR()->m_bArea[0x27] = false;
			MOUSECURSOR()->m_bArea[0x28] = false;
			MOUSECURSOR()->m_bArea[0x29] = false;
			MOUSECURSOR()->m_bArea[0x2a] = false;
			MOUSECURSOR()->m_bArea[0x2b] = false;
			MOUSECURSOR()->m_bArea[0x2c] = false;
			m_active = 3;
		}
		else if (g_input->GetButton(LEFT_BUTTON) == 2)
		{
			switch (m_hover)
			{
			case 0:
				if (CQuitWindow::Instance())
				{
					CQuitWindow::Instance()->CreateOptionWindow();
				}
				break;
			case 1:
			{
				// 983 0074fae0: 0x15b (= M983(0x12a)); o ator pode nao existir
				MsgObject msg(NULL, M983(0x12a), 0, 0, 0, 0, 0);
				IActor* guide = AfxGetTask()->GetActor("GuideBook");
				if (guide)
				{
					guide << msg;
				}
				break;
			}
			case 2:
				SetUnplayable();
				break;
			case 3:
				SetPause();
				break;
			case 4:
				OpenChatRecord();
				break;
			}
		}
		break;
	case 3:
		m_slide -= dt * 5.0f;
		if (m_slide < 0.0f)
		{
			m_slide = 0.0f;
			m_active = 0;
		}
		break;
	}
}

void CAuxMenuUI::SetUnplayable()
{
	if (GOLFDOC()->m_bDropRequest)
	{
		return;
	}
	if ((bool)(Doc()->m_myInfo.info.dwIdentity >> 1 & 1))
	{
		return;
	}
	if (!IsMyTurn(false))
	{
		return;
	}
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_AUXMENU_UNPLAYABLE] = false;
	if (GetPVS().OverlappedWithObject(GolfBall().m_pos, 0.0f) ||
		(GolfBall().m_groundType != 0 && GolfBall().m_groundType != 1 &&
			GolfBall().m_groundType != 2))
	{
		GOLFDOC()->m_bDropRequest = true;
		if ((GOLFDOC()->m_gameMode & ~0x200) != 0)
		{
			MsgObject msg(NULL, M983(0x78), 0x200, 0, 0, 0, 0);
			AfxGetTask()->GetActor("GolfRule") << msg;
		}
		g_camera.pivot =
			GetPVS().GetGroundPoint(GolfBall().m_pos, false, NULL, false);
		g_camera.pivot.y += 192.0f;
		return;
	}
	if (m_dropFailDlg)
	{
		return;
	}
	m_dropFailDlg =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
	switch (GolfBall().m_groundType)
	{
	case 0:
		m_dropFailDlg->SetMessage(
			K2L_Compatibility("\xc6\xbc\xbc\xa6\xc0\xa7\xc4\xa1\xbf\xa1\xbc\xad\xb4\xc2 \xbe\xf0\xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xba\xed\xc0\xbb \xbc\xb1\xbe\xf0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;
	case 1:
		m_dropFailDlg->SetMessage(
			K2L_Compatibility("\xc6\xe4\xbe\xee\xbf\xfe\xc0\xcc\xbf\xa1\xbc\xad\xb4\xc2 \xbe\xf0\xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xba\xed\xc0\xbb \xbc\xb1\xbe\xf0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;
	case 2:
		m_dropFailDlg->SetMessage(
			K2L_Compatibility("\xb1\xd7\xb8\xb0\xbf\xa1\xbc\xad\xb4\xc2 \xbe\xf0\xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xba\xed\xc0\xbb \xbc\xb1\xbe\xf0 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;
	}
	m_dropFailDlg->Open((FRESH_PFN_RESULT)&CAuxMenuUI::OnDropFailDlgResult, 4);
	m_dropFailDlg->SetTimeLimit(3.0f);
}

void CAuxMenuUI::SetPause()
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb: // 983 0074c4b0: tabela 0074c600 (4,5,6,9,10,0xb saem; 7,8 pausam)
		break;
	default:
		unsigned long limit = Doc()->m_golfGame.shotTimeLimit;
		if (limit != 0)
		{
			MOUSECURSOR()->m_bArea[CMouseCursor::AREA_AUXMENU_PAUSE] = false;
			if (m_pauseCount == 0)
			{
				MsgObject msg(NULL, 0x23,
					(int)K2L_Compatibility("\xb0\xd4\xc0\xd3\xc1\xa4\xc1\xf6\xb4\xc2 \xc7\xd1 \xb0\xd4\xc0\xd3\xbf\xa1 2\xb9\xf8\xb8\xb8 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
					0, 0, 0, 0);
				AfxGetTask()->GetActor("GolfRule") << msg;
			}
			else
			{
				m_pauseCount--;
				if (OnlinePlay())
				{
					WSendPacket packet((enumClientPacket)0x30);
					packet.Encode1(1);
					packet.Send(TO_GAME);
				}
				else
				{
					MsgObject msg(NULL, M983(0x84), 1,
						(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xc0\xcc \xc0\xe1\xbd\xc3 \xc1\xdf\xb4\xdc\xb5\xc8 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9.\n\xb0\xd4\xc0\xd3\xc0\xbb \xb0\xe8\xbc\xd3\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 \xc8\xae\xc0\xce\xc0\xbb \xb4\xad\xb7\xaf\xc1\xd6\xbc\xbc\xbf\xe4."),
						0, 0, 0);
					AfxGetTask()->GetActor("GolfRule") << msg;
				}
			}
		}
		break;
	}
}

void CAuxMenuUI::OpenChatRecord()
{
	if (OnlinePlay() && !m_pChatFuncDlg)
	{
		m_pChatFuncDlg = CreateForm<FrChatFuncDlg>(g_pFresh->GetManager(), this,
			"chat_func", NULL);
		m_pChatFuncDlg->Open((FRESH_PFN_RESULT)&CAuxMenuUI::OnChatFuncDlgResult,
			5);
	}
}

bool CAuxMenuUI::OnDropDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (OnlinePlay())
		{
			WSendPacket packet((enumClientPacket)0x19);
			const WVector& src = GOLFDOC()->m_pAutoTarget->m_dropPos;
			WVector pos;
			pos = src;
			packet.EncodeBuffer(&pos, sizeof(pos));
			packet.Send(TO_GAME);
		}
		else
		{
			const WVector& src = GOLFDOC()->m_pAutoTarget->m_dropPos;
			WVector pos;
			pos = src;
			GolfBall().m_pos = pos;
			MsgObject msg(NULL, M983(0x8d), 0, 0, 0, 0, 0);
			AfxGetTask()->GetActor("GolfRule") << msg;
		}
	}
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_AUXMENU_UNPLAYABLE] = true;
	m_dropDlg = 0;
	return true;
}

bool CAuxMenuUI::OnDropFailDlgResult(int result, FrForm* pForm)
{
	if (0)
		((CMiniScore*)0)->CMiniScore::Hide();
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_AUXMENU_UNPLAYABLE] = true;
	m_dropFailDlg = 0;
	return true;
}

void CAuxMenuUI::Display()
{
	if (MOUSECURSOR()->IsVisible())
	{
		unsigned long color;
		if (m_active == 0)
		{
			switch (MOUSECURSOR()->GetArea())
			{
			case CMouseCursor::AREA_AUXMENU_SHOW:
				color = 0x80ffffff;
				break;
			case CMouseCursor::AREA_AUXMENU_OPEN:
				color = 0xffffffff;
				break;
			default:
				color = 0x80ffffff;
				break;
			}
		}
		else
		{
			color = 0xffffffff;
		}
		if (m_pBase)
		{
			WRect dst(g_view->GetWidth() - 128.0f + (1.0f - m_slide) * 79.0f,
				g_view->GetHeight() * 0.5f - 66.0f, 128.0f, 256.0f);
			// 983 0074b270: origem 00e4296c, flag 0x6000000
			m_pBase->Render(g_view, g_rectUnitW983, dst, 0x6000000, color, 0, 0);
		}
		int i = 0;
		do
		{
			int idx = (i == m_hover || i == m_selected) ? 1 : 0;
			if (*(&m_pBlack + idx))
			{
				idx = (i == m_hover || i == m_selected) ? 1 : 0;
				int row = i - 3;
				WRect dst(g_view->GetWidth() - m_slide * 79.0f,
					g_view->GetHeight() * 0.5f + (float)row * 22.0f, 128.0f,
					22.0f);
				WRect src(0.0f, (float)i * 0.0859375f, 1.0f, 0.0859375f);
				(*(&m_pBlack + idx))
					->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
			}
			i++;
		} while (i < 6);
	}
}

bool CAuxMenuUI::OnChatFuncDlgResult(int result, FrForm* pForm)
{
	m_pChatFuncDlg = NULL;
	return true;
}

void CAuxMenuUI::SetActive(bool active, int index)
{
	if (active)
	{
		m_selected = index;
		m_active = 1;
		MOUSECURSOR()->m_bArea[index + 0x27] = true;
		MOUSECURSOR()->ForceMove(g_view->GetWidth() - 50.0f,
			g_view->GetHeight() * 0.5f + ((float)(index - 3) + 0.5f) * 22.0f,
			-1.0f);
		return;
	}
	m_selected = -1;
}

void CAuxMenuUI::ChangeTurn()
{
	if (0)
		((CAuxMenuUI*)0)->CAuxMenuUI::SetActive(*(bool*)0, *(int*)0);
	m_selected = -1;
	m_active = 0;
	m_slide = 0.0f;
	MOUSECURSOR()->m_bArea[0x27] = false;
	MOUSECURSOR()->m_bArea[0x28] = false;
	MOUSECURSOR()->m_bArea[0x29] = false;
	MOUSECURSOR()->m_bArea[0x2a] = false;
	MOUSECURSOR()->m_bArea[0x2b] = false;
	MOUSECURSOR()->m_bArea[0x2c] = false;
}

CDrivingDistInfo::CDrivingDistInfo()
{
	m_topIdx = 0xff;
	m_prevIdx = 0xff;
	m_pNPlayer = NULL;
	m_pTopDrv = NULL;
	m_bShow = true;
	m_bBlink = false;
	m_topColor = -1;
	m_myColor = -1;
	m_baseDist = 0;
	m_topDist = 0;
	m_myDist = 0;
	m_pNPlayer = g_resrcmng->GetOverlay("[nplayer.jpg", 0);
	m_pTopDrv = g_resrcmng->GetOverlay("[top_drv.jpg", 0);
}

CDrivingDistInfo::~CDrivingDistInfo()
{
	if (g_resrcmng)
	{
		if (m_pNPlayer)
		{
			g_resrcmng->Release(m_pNPlayer);
			m_pNPlayer = NULL;
		}
		if (g_resrcmng && m_pTopDrv)
		{
			g_resrcmng->Release(m_pTopDrv);
			m_pTopDrv = NULL;
		}
	}
}

#if 0 // CDrivingDistInfo::OnChangeTurn/Process/Display: so o 645 (no 983 so existem ctor/dtor 0074a890/0074a8f0; nada os chama)
void CDrivingDistInfo::OnChangeTurn()
{
	m_bShow = true;
	m_myDist = Doc()->m_holeStatistics[MyIndex()].fLongest;
	float top = Wabs(m_topDist);
	if (g_EPSILON <= top)
	{
		m_prevIdx = 0;
		m_topIdx = 0;
		m_topDist = Doc()->m_holeStatistics[0].fLongest;
		unsigned char i = 1;
		while (i < GOLFDOC()->m_playerNum)
		{
			if (m_topDist < Doc()->m_holeStatistics[i].fLongest)
			{
				m_prevIdx = i;
				m_topIdx = i;
				m_topDist = Doc()->m_holeStatistics[i].fLongest;
			}
			i++;
		}
		m_baseDist = m_topDist;
	}
}

void CDrivingDistInfo::Process(float dt)
{
	if (0)
		((CMiniScore*)0)
			->CMiniScore::
				Display(); // HACK: emission order (wave 6 in the original)
	if (m_bShow)
	{
		static float s_blinkTime;
		s_blinkTime += dt;
		if (s_blinkTime > (m_bBlink ? 1.5f : 2.0f))
		{
			s_blinkTime = 0.0f;
			m_bBlink = !m_bBlink;
		}
		m_topColor = m_myColor = 0xffffffff;
		if (GolfBall().m_state == 0 && GolfBall().m_bOB)
		{
			if (m_topDist > m_baseDist)
			{
				if (m_topDist - m_baseDist < 0.05f)
				{
					m_topIdx = m_prevIdx;
					m_topDist = m_baseDist;
					m_topColor = 0xffffffff;
				}
				else
				{
					m_topColor = 0xffff0000;
					m_topDist =
						m_topDist - (m_topDist - m_baseDist) * dt * 5.0f;
				}
			}
			if (GOLFDOC()->m_currentPlayer == MyIndex())
			{
				float& longest =
					Doc()
						->m_holeStatistics[GOLFDOC()->m_currentPlayer]
						.fLongest;
				if (m_myDist > longest)
				{
					float diff = m_myDist - longest;
					if (diff < 0.05f)
					{
						m_myDist = longest;
						m_topColor = 0xffffffff;
					}
					else
					{
						m_myColor = 0xffff0000;
						m_myDist = m_myDist - diff * dt * 5.0f;
					}
				}
			}
		}
		else if (GOLFDOC()->m_gameMode == 0x80)
		{
			if (WisEqual(GolfBall().PreData(0)->vel, WVector::ZERO, 1e-5f))
			{
				m_bShow = false;
				return;
			}
			const CGolfBall::sExpected* data = GolfBall().PreData(0);
			WVector diff;
			diff = GolfBall().m_pos - data->pos;
			float dist = sqrtf(diff.x * diff.x + diff.z * diff.z) * 0.3125f;
			if (m_topDist <= dist)
			{
				m_topIdx = GOLFDOC()->m_currentPlayer;
				m_topDist = dist;
				m_topColor = 0xff00ff97;
			}
			if (GOLFDOC()->m_currentPlayer == MyIndex() && m_myDist <= dist)
			{
				m_myDist = dist;
				m_myColor = 0xff00ff97;
			}
		}
	}
}

void CDrivingDistInfo::Display(WFont* font)
{
	char buf[32];
	float y = COption::Instance()->gGetBar_Y();
	WRect box(27.0f, y + 4.0f, 62.0f, 1.0f);
	WOverlay::DrawBox(g_view, box, 0, 0xffeeeeee, 0.001f);
	WRect src;
	WRect dst;
	if (GOLFDOC()->m_gameMode != 0x80 && m_bBlink)
	{
		dst.y = y - 28.0f;
		src.y = 0.0f;
		src.x = 0.0f;
		src.h = 1.0f;
		src.w = 1.0f;
		dst.x = 27.0f;
		dst.h = 64.0f;
		dst.w = 64.0f;
		m_pTopDrv->Render(g_view, src, dst, 0, 0xffffffff, 0, 0);
		return;
	}
	sprintf(buf, "%.2fy", m_topDist);
	int half = (int)(0.5f * font->GetTextWidthOld983(g_view, buf));
	float x = 57.0f - half;
	font->Print(g_view, x, y - 9.0f, buf, 0, m_topColor, NULL);
	sprintf(buf, "%.2fy", m_myDist);
	half = (int)(0.5f * font->GetTextWidthOld983(g_view, buf));
	x = 57.0f - half;
	font->Print(g_view, x, y + 7.0f, buf, 0, m_myColor, NULL);
	src.w = 0.5f;
	src.h = 0.25f;
	dst.x = 41.0f;
	dst.w = 32.0f;
	dst.h = 16.0f;
	if (m_topIdx != 0xff)
	{
		src.x = (m_topIdx & 1) ? 0.5f : 0.0f;
		src.y = (m_topIdx & 2) ? 0.25f : 0.0f;
		dst.y = y - 25.0f;
		if (m_pNPlayer)
		{
			m_pNPlayer->SetClippingArea(&dst);
			unsigned char idx = m_topIdx;
			sPlayerData* player = GOLFDOC()->GetPlayer(idx);
			m_pNPlayer->Render(g_view, src, dst, 0, player->color | 0xff000000,
				0, 0);
		}
	}
	src.x = (MyIndex() & 1) ? 0.5f : 0.0f;
	src.y = (MyIndex() & 2) ? 0.25f : 0.0f;
	dst.y = y + 17.0f;
	if (m_pNPlayer)
	{
		m_pNPlayer->SetClippingArea(&dst);
		unsigned char idx = MyIndex();
		sPlayerData* player = GOLFDOC()->GetPlayer(idx);
		m_pNPlayer->Render(g_view, src, dst, 0, player->color | 0xff000000, 0,
			0);
	}
	font->Flush(g_view);
}
#endif // 0

IMPLEMENT_OBJECT(FrChatFuncDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrChatFuncDlg, FrForm)

ON_FRESH_VI("chatview", FRCMD_INIT, FrChatFuncDlg::OnChatViewInit)
ON_FRESH_VI("hidechat", FRCMD_INIT, FrChatFuncDlg::OnHideChatInit)
ON_FRESH_VI("showchat", FRCMD_INIT, FrChatFuncDlg::OnShowChatInit)
ON_FRESH_VV("hidechat", FRCMD_LBUTTONUP, FrChatFuncDlg::OnBtnHideChatUp)
ON_FRESH_VV("showchat", FRCMD_LBUTTONUP, FrChatFuncDlg::OnBtnShowChatUp)
ON_FRESH_VV("savechat", FRCMD_LBUTTONUP, FrChatFuncDlg::OnBtnSaveChatUp)
ON_FRESH_VV("accuse", FRCMD_LBUTTONUP, FrChatFuncDlg::OnBtnAccuseUp)
ON_FRESH_VV("closedlg", FRCMD_LBUTTONUP, FrChatFuncDlg::OnBtnCloseDlgUp)

END_FRESH_MSGMAP()

FrChatFuncDlg::FrChatFuncDlg()
{
	m_unused110 = 0;
	m_pBtnShowChat = NULL;
	m_pBtnHideChat = NULL;
	m_unused11c = 0;
	m_unused120 = 0;
	m_pChatView = NULL;
}

FrChatFuncDlg::~FrChatFuncDlg()
{
}

void FrChatFuncDlg::OnBtnShowChatUp()
{
	if (m_pBtnHideChat)
		m_pBtnHideChat->SetVisible(true);
	if (m_pBtnShowChat)
		m_pBtnShowChat->SetVisible(false);

	GOLFDOC()->m_bShowChat = true;
	std::string msg(K2L_Compatibility("\xb0\xd4\xc0\xd3\xb3\xbb \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xc8\xb0\xbc\xba\xc8\xad\xc7\xd5\xb4\xcf\xb4\xd9."));
	CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, true, false);
}

void FrChatFuncDlg::OnBtnHideChatUp()
{
	if (m_pBtnHideChat)
		m_pBtnHideChat->SetVisible(false);
	if (m_pBtnShowChat)
		m_pBtnShowChat->SetVisible(true);

	GOLFDOC()->m_bShowChat = false;
	std::string msg(K2L_Compatibility("\xb0\xd4\xc0\xd3\xb3\xbb \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xbc\xfb\xb1\xe9\xb4\xcf\xb4\xd9."));
	CChatMsg::Instance()->AddChatMsg(msg, 0xffff7878, true, false);
}

void FrChatFuncDlg::OnBtnSaveChatUp()
{
	int num =
		Doc()->m_chatManager.SaveChatHistory(GOLFDOC()->GetSaveChatNum(), 1);
	GOLFDOC()->SetSaveChatNum(num);
}

bool FrChatFuncDlg::OnReportDlgResult(int result, FrForm* pForm)
{
	Close(FrOK, true);
	m_pReportDlg = NULL;
	return true;
}

void FrChatFuncDlg::OnBtnAccuseUp()
{
	m_pReportDlg =
		CreateForm<FrReportDlg>(g_pFresh->GetManager(), this, "report", NULL);
	if (!m_pReportDlg)
		return;

	std::string msg;
	msg =
		K2L_Compatibility("1. \xb4\xeb\xc8\xad\xc3\xa2\xc0\xc7 \xc1\xf6\xb3\xad \xb3\xbb\xbf\xeb\xb5\xb5 \xb8\xf0\xb5\xce \xbc\xad\xb9\xf6\xbf\xa1 \xb1\xe2\xb7\xcf\xb5\xcb\xb4\xcf\xb4\xd9.\n");
	msg +=
		K2L_Compatibility("2. \xc1\xb6\xc0\xdb, \xc7\xe3\xc0\xa7, \xb9\xdd\xba\xb9\xbd\xc5\xb0\xed\xb4\xc2 \xc3\xb3\xb9\xfa\xc0\xbb \xb9\xde\xbd\xc0\xb4\xcf\xb4\xd9.\n");
	msg +=
		K2L_Compatibility("3. \xbd\xc5\xb0\xed\xc7\xcf\xbd\xc5 \xb3\xbb\xbf\xeb\xc0\xba 48\xbd\xc3\xb0\xa3 \xc0\xcc\xb3\xbb\xbf\xa1 \xc3\xb3\xb8\xae\xb5\xcb\xb4\xcf\xb4\xd9.");

	m_pReportDlg->SetMessage(msg.c_str(), false);
	m_pReportDlg->Open((FRESH_PFN_RESULT)&FrChatFuncDlg::OnReportDlgResult, 3);
}

void FrChatFuncDlg::OnBtnCloseDlgUp()
{
	Close(FrOK, true);
}

void FrChatFuncDlg::OnHideChatInit(int param)
{
	m_pBtnHideChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pBtnHideChat)
		return;

	if (GOLFDOC()->m_bShowChat)
	{
		m_pBtnHideChat->SetVisible(true);
	}
	else
	{
		m_pBtnHideChat->SetVisible(false);
	}
}

void FrChatFuncDlg::OnShowChatInit(int param)
{
	m_pBtnShowChat = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!m_pBtnShowChat)
		return;

	if (GOLFDOC()->m_bShowChat)
	{
		m_pBtnShowChat->SetVisible(false);
	}
	else
	{
		m_pBtnShowChat->SetVisible(true);
	}
}

void FrChatFuncDlg::OnChatViewInit(int param)
{
	m_pChatView = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (!m_pChatView)
		return;

	for (std::list<sChatLine>::iterator it = Doc()->m_chatLineList.begin();
		it != Doc()->m_chatLineList.end(); ++it)
	{
		sChatLine& line = *it;
		m_pChatView->AddLine(line.text.c_str(),
			CChatMsg::Instance()->ConvertToUIColor(line.color), false);
	}
}

void FrChatFuncDlg::AddChatMsg(const char* msg, int color)
{
	if (m_pChatView)
		m_pChatView->AddLine(msg, color, false, 2); // 983 0074a9f0: style 2
}

void CScreen::DrawClubInfo()
{
	if (!m_bHoleShown[1] || CCapturedBg::Instance()->IsCaptured())
	{
		return;
	}

	unsigned long tid =
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid;
	sClubSet* pClubSet = GolfClub().FindClubSet(tid); // 983 007501c6: so procura
	if (!pClubSet)
	{
		return;
	}

	float clipNear = g_view->GetClipNearValue();
	if (!(0.0f < clipNear))
	{
		clipNear = 0.0f;
	}
	float clipFar = g_view->GetClipFarValue();

	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedRect* pClipRect =
		ui->FindNamedRect("r_club_pet_clip");
	CUiFrameSetManager::NamedPoint* pCenterPt =
		ui->FindNamedPoint("p_club_pet_center");
	WPuppet* pPuppet = pClubSet->pHeadSub[GolfClub().GetType()];
	if (!pPuppet || !pClipRect || !pCenterPt)
	{
		return;
	}

	WMatrix camera = pClubSet->frame[GolfClub().GetType()];

	// 983 00750150: recorte e centro vem de "r_club_pet_clip"/"p_club_pet_center" do layout (640x480 escalado), a camera
	// recua pela distancia do taco e o FOV e fixo (85 * 0,75) em perspectiva (repostos no fim)
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float fov = g_view->GetFOV();

	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	WRect clip;
	pClipRect->Calc(scl, clip);
	WPoint center;
	pCenterPt->Calc(scl, center);

	float rate = m_clubAngle / 6.2831855f - 1.0f;
	center.x += ScaleByViewHeight983(g_view, rate * 50.0f);
	center.y += ScaleByViewHeight983(g_view, rate * -50.0f);

	float back = (1.0f - ui->ScaleForView983(g_view, 0.8f)) *
		pClubSet->dist983[GolfClub().GetType()];
	camera.xm += back * camera.xz;
	camera.ym += back * camera.yz;
	camera.zm += back * camera.zz;

	g_view->SetFOV(1.1126474f);
	g_view->SetClip(0.1f, 100.0f, false);
	g_view->SetProjectionMode(WView::PERSPECTIVE);
	g_view->SetCamera(camera);
	g_view->SetClippingArea(clip);
	g_view->SetScreenCenter(center.x, center.y);
	g_view->UpdateCamera();

	if (pPuppet)
	{
		pPuppet->SetMatrix(
			RotMat(sinf(m_clubSwing) * 0.08726646f - m_clubAngle, 0));
		pPuppet->SetAlpha(
			(unsigned char)Between(0.0f, m_clubAngle * 40.584511f, 255.0f));
		pPuppet->Transform(NULL, 1.0f, 0);
		pPuppet->UpdateLightSource(&pClubSet->guide[GolfClub().GetType()],
			false, false);
		pPuppet->Render(g_view, true, 1.0f, false, true, false);
	}

	g_view->Flush(1, 1);
	g_view->SetProjectionMode(oldMode);
	g_view->SetCamera(g_camera);
	g_view->SetClip(clipNear, clipFar, false);
	g_view->SetFOV(fov);
	g_view->ResetClippingArea();
	g_view->ResetScreenCenter();
	g_view->UpdateCamera();
}

void CScreen::DrawCammodeInfo()
{
	if (!MOUSECURSOR()->IsCamMode())
	{
		return;
	}

	char text[16];
	char state[8];

	if (g_input->GetButton(LEFT_BUTTON))
	{
		strcpy(state, "CLICK");
	}
	else
	{
		strcpy(state, "OVER");
	}

	switch (MOUSECURSOR()->GetArea())
	{
	case CMouseCursor::AREA_CAMLEFT:
		strcpy(text, MakeStr("LEFT_%s", state));
		break;
	case CMouseCursor::AREA_CAMRIGHT:
		strcpy(text, MakeStr("RIGHT_%s", state));
		break;
	case CMouseCursor::AREA_CAMUP:
		strcpy(text, MakeStr("UP_%s", state));
		break;
	case CMouseCursor::AREA_CAMDOWN:
		strcpy(text, MakeStr("DOWN_%s", state));
		break;
	case CMouseCursor::AREA_CAMFORWARD:
		if (GOLFDOC()->m_gameMode & 0x8000)
		{
			strcpy(text, "INACTIVE");
		}
		else
		{
			strcpy(text, MakeStr("FORWARD_%s", state));
		}
		break;
	case CMouseCursor::AREA_CAMBACKWARD:
		if (GOLFDOC()->m_gameMode & 0x8000)
		{
			strcpy(text, "INACTIVE");
		}
		else
		{
			strcpy(text, MakeStr("BACKWARD_%s", state));
		}
		break;
	default:
		if (GOLFDOC()->m_gameMode == 0x8000)
		{
			strcpy(text, "INACTIVE");
		}
		else
		{
			strcpy(text, "NORMAL");
		}
		break;
	}

	if (m_pCammodeUI)
	{
		m_pCammodeUI->SetColor(
			(MOUSECURSOR()->IsVisible() || m_cammodeAlpha < 1.0f) ? 0xffffffff
																  : 0x80ffffff);
		m_pCammodeUI->Render(text, g_view->GetWidth() * 0.65f,
			g_view->GetHeight() - 150.0f, m_cammodeAlpha, 0x22);
	}
}

// 983 (007506d0): sem os overlays do vento do 645 nem o ramo "classic"; o fundo da direcao e o da velocidade saem
// dos quadros "f_wind_direction_bg"/"f_wind_speed_bg" do CUiFrameSetManager, o cata-vento na posicao
// "d_wind_direction_center_x/y" e o texto da velocidade pelo CWindInfo (m_pWindInfo, +0x220)
void CScreen::DrawMatchBand983() // 983 0074d160
{
	CSharedDoc* sdoc = CSharedDoc::Instance(); // A730: membros (cl novo desloca os crus do VC10)
	unsigned char state = sdoc->m_golfGame.gameType;
	if (state == 4 || state == 5 || state == 6 || state == 9 || state == 0xb)
	{
		DrawOverlay("\xb0\xd4\xc0\xd3\xc1\xa6\xc7\xd1\xbd\xc3\xb0\xa3",
			WRect(g_view->GetWidth() * 0.5f - 87.0f,
				g_view->GetHeight() - 32.0f, 256.0f, 32.0f),
			0xFF, 0);

		const char* text = NULL;
		char buf[80];
		if (state == 4 || state == 5)
		{
			IFF_STRUCT::sMatch* match =
				Doc()->m_itemManager.FindMatch(sdoc->m_roomType);
			if (match)
			{
				if (state == 5)
				{
					sprintf(buf, "%s %s", match->Name,
						K2L_Compatibility("\xc6\xc0\xc0\xfc"));
				}
				else if (sdoc->m_roomInfo.realGameType == 0x12)
				{
					strcpy(buf, K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba"));
				}
				else if (sdoc->m_roomInfo.realGameType == 0x13)
				{
					strcpy(buf, K2L_Compatibility("\xb3\xb2\xc0\xba \xbd\xc3\xb0\xa3"));
				}
				else
				{
					strcpy(buf, match->Name);
				}
				text = buf;
			}
		}
		else if (state == 6)
		{
			text = K2L_Compatibility("\xb1\xe6\xb5\xe5 \xb4\xeb\xc0\xfc");
		}
		else if (state == 0xb)
		{
			text = MakeStr(K2L_Compatibility("\xbd\xc3\xc0\xdb \xc0\xce\xbf\xf8 : %d"),
				(int)(reinterpret_cast<sRivalData* const*>(&sdoc->m_rivalList)[1] - reinterpret_cast<sRivalData* const*>(&sdoc->m_rivalList)[0]) /* begin/end do vector, VC10 e cl novo; size() sai fora de linha no VC10 */);
		}
		if (text)
		{
			float width = m_pFont->GetTextWidthOld983(g_view, text);
			m_pFont->Print(g_view, (g_view->GetWidth() - width) * 0.5f - 25.0f,
				g_view->GetHeight() - 16.0f, text, 0, 0xFFFFFFFF, NULL);
		}
		m_pFont->Flush(g_view);
	}
}

void CScreen::DrawMatchTime983() // 983 0074d530
{
	CSharedDoc* sdoc = CSharedDoc::Instance(); // A730: membros (cl novo desloca os crus do VC10)
	unsigned char state = sdoc->m_golfGame.gameType;
	if (state == 4 || state == 5 || state == 6 || state == 9 || state == 0xb)
	{
		unsigned long limit = sdoc->m_golfGame.gameTimeLimit;
		unsigned long start = sdoc->m_approachStartTime;
		float x = g_view->GetWidth() * 0.5f + 39.0f;
		float y = g_view->GetHeight() - 16.0f;
		if (g_CurrentTime < limit + start)
		{
			// 983 0074d0d0
			unsigned long sec;
			if (state == 0xb)
			{
				if (sdoc->m_pGolfDoc->m_gameMode == 0x2000000)
				{
					sec = limit / 1000;
				}
				else
				{
					sec = (start - m_gpExtraElapsedMs983 - g_CurrentTime + limit) / 1000;
					if (limit < sec)
					{
						sec = limit;
					}
				}
			}
			else
			{
				sec = (limit + start - g_CurrentTime) / 1000;
			}
			m_pFont->Print(g_view, x, y, MakeStr("%02d:%02d", sec / 60, sec % 60), 0,
				sec < 180 ? 0xFFFF0000 : 0xFFFFFFFF, NULL);
		}
		else
		{
			m_pFont->Print(g_view, x, y, "00:00", 0, 0xFFFF0000, NULL);
		}
		m_pFont->Flush(g_view);
	}
}

void CScreen::DrawWindInfo()
{
	bool review = false;
	SendMsg(NULL, "ShotReview", 0x2fd, (int)"IS_REVIEW_CAMERA_MODE",
		(int)&review, 0, 0);

	if (GolfClub().GetType() == 3 && !review)
	{
		return;
	}
	if (!(GOLFDOC()->m_tutorialMode == 15 ||
			(GOLFDOC()->m_tutorialMode == 11 && m_tuto11Triggered)))
	{
		return;
	}

	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	ui->DrawFrameSet("f_wind_direction_bg", g_view, 0, 0xffffffff);
	CUiFrameSetManager::NamedDim* cx =
		ui->FindNamedDim("d_wind_direction_center_x");
	CUiFrameSetManager::NamedDim* cy =
		ui->FindNamedDim("d_wind_direction_center_y");
	if (cx != NULL && cy != NULL)
	{
		CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
			(float)(g_view->GetHeight() / 480.0));
		float y = (float)(cy->Calc(scl) + ScaleByViewHeight983(g_view, 1.0f));
		Wind().Display(m_pWindPuppet, cx->Calc(scl), y);
	}
	ui->DrawFrameSet("f_wind_speed_bg", g_view, 0, 0xffffffff);
	if (m_pWindInfo)
	{
		m_pWindInfo->Display();
	}
}

void CScreen::DrawApproachInfo()
{
	if (Doc()->m_golfGame.gameType != 10)
	{
		return;
	}

	if (Doc()->m_approachStartTime == 0)
	{
		return;
	}

	DrawOverlay(OVERLAY_MISSION_FRAME1,
		WRect(g_view->SCREEN_XS - 270.0f, 46.0f, 256.0f, 32.0f), 0xFF, 0);
	DrawOverlay(OVERLAY_MISSION_FRAME2,
		WRect(g_view->SCREEN_XS - 270.0f, 70.0f, 256.0f, 256.0f), 0xFF, 0);

	if (GOLFDOC()->GetApproachMissionType() == 0)
	{
		m_pMissionUI->Render("\xb9\xcc\xbc\xc7\xbe\xf8\xc0\xbd",
			g_view->SCREEN_XS - 182.0f, 95.0f, 1.0f, 0);
	}
	else
	{
		m_pMissionUI->Render("\xb9\xcc\xbc\xc7\xc0\xd6\xc0\xbd",
			g_view->SCREEN_XS - 220.0f, 75.0f, 1.0f, 0);
	}

	if (GOLFDOC()->GetApproachMissionType() != 0)
	{
		char prize[64] = "\0";
		switch (GOLFDOC()->GetApproachWinnerType())
		{
		case 1:
			sprintf(prize,
				K2L_Compatibility("\xa1\xd8 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8\xbd\xc3 \xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4 %d\xb0\xb3 \xb4\xf5."),
				GOLFDOC()->GetApproachPrizeType());
			break;
		case 2:
			sprintf(prize,
				K2L_Compatibility("\xa1\xd8 \xbc\xba\xb0\xf8\xc7\xd1 \xc0\xaf\xc0\xfa\xbf\xa1\xb0\xd4 %d\xb0\xb3 \xb4\xf5."),
				GOLFDOC()->GetApproachPrizeType());
			break;
		case 3:
			sprintf(prize,
				K2L_Compatibility("\xa1\xd8 \xb9\xcc\xbc\xc7\xbc\xba\xb0\xf8\xbd\xc3 \xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4 %d\xb0\xb3 \xb4\xf5."),
				GOLFDOC()->GetApproachPrizeType());
			break;
		}

		float x = g_view->SCREEN_XS - 205.0f;
		std::string desc(GOLFDOC()->GetApproachMissionDesc());
		std::string lines[4] = { "" };
		lines[3] = prize;

		if (desc.length() >= 33)
		{
			// 983 00753d80: procura o espaco de tras p/ frente; sem espaco corta em 33 bytes sem partir caractere DBCS
			int start = 0;
			for (int n = 0; n < 3; n++)
			{
				if (desc.length() < (unsigned int)(start + 33))
				{
					lines[n] = desc.substr(start);
					break;
				}

				int len = 33;
				do
				{
					if (start + len < (int)desc.length() && desc[start + len] == ' ')
					{
						break;
					}
					len--;
				} while (len > 0);

				if (len == 0)
				{
					int i = start;
					for (; i <= start + 33; i++)
					{
						unsigned char c = (unsigned char)desc[i];
						if ((c > 0x7f && c < 0xa0) || (unsigned char)(c + 0x20) < 0x1d)
						{
							i++;
						}
					}
					len = i - start;
				}

				lines[n] = desc.substr(start, len);
				start += len;
				if (desc.length() <= (unsigned int)start)
				{
					break;
				}
			}
		}
		else
		{
			lines[0] = desc;
		}

		for (int i = 0; i < 4; i++)
		{
			m_pFont->Print(g_view, x, i * 17.0f + 138.0f, lines[i].c_str(), 0,
				0xFFFFFFFF, NULL);
		}
	}

	m_pFont->Flush(g_view);

	DrawOverlay2(OVERLAY_TREASUREBOX,
		WRect(g_view->SCREEN_XS - 60.0f, 47.0f, 32.0f, 32.0f), 0xFFFFFFFF, 0);

	unsigned long total = GOLFDOC()->GetApproachTotalPrize();
	char num[4] = { 0 };
	itoa(total, num, 10);
	float offset = 65.0f;
	if (total > 9)
	{
		offset = 75.0f;
	}
	m_pTitleFont->Print(g_view, g_view->SCREEN_XS - offset, 49.0f, num, 0,
		0xFFFFFFFF, NULL);

	unsigned long remain;
	if (Doc()->m_approachStartTime == 0)
	{
		remain = Doc()->m_golfGame.gameTimeLimit;
	}
	else
	{
		remain = Doc()->m_approachStartTime + Doc()->m_golfGame.gameTimeLimit;
		if (remain < g_CurrentTime)
		{
			remain = 0;
		}
		else
		{
			remain -= g_CurrentTime;
		}
	}

	char time[16];
	sprintf(time, "%d.%03d", remain / 1000, remain % 1000);

	unsigned long color;
	if (remain > 20000)
	{
		color = 0xFF000000;
	}
	else
	{
		color = remain > 10000 ? 0xFFFF8000 : 0xFFFF0000;
	}

	DrawOverlay2(OVERLAY_TIME_BASE, WRect(17.0f, 150.0f, 128.0f, 64.0f), color,
		0);
	DrawOverlay2(OVERLAY_TIME_BORDER, WRect(17.0f, 150.0f, 128.0f, 64.0f),
		0xFFFFFFFF, 0);

	float width = m_pFont->GetTextWidthOld983(g_view, time);
	m_pFont->Print(g_view, 75.0f - width, 161.0f, time, 0, 0xFFFFFFFF, NULL);

	if (GOLFDOC()->m_gameMode & 0x8c0)
	{
		DrawOverlay2(OVERLAY_APPROACH_INFO, WRect(10.0f, 200.0f, 64.0f, 64.0f),
			0xFFFFFFFF, 0);
	}

	m_pFont->Flush(g_view);
}

// escala da tela para o CUiFrameSetManager (tela / 640x480), lida direto da vista em cada desenho da barra do 983
extern const CUiFrameSetManager::Size g_sclOne983; // 00e424f8 (1, 1)

static inline CUiFrameSetManager::Size HitBarScale983()
{
	return CUiFrameSetManager::Size((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
}

// 983 (00754650): cursor do driver ("f_driver_thumb") na posicao da batida; no impacto (fase 3) encosta na borda da
// janela de impacto arredondada quando a batida caiu no pixel de fora/de dentro dela
void CScreen::DrawDriverThumb983(int width, int hitX, unsigned char alpha)
{
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::Size scl = HitBarScale983();
	CUiFrameSetManager::NamedDim* baseX = ui->FindNamedDim("d_default_pangya_x");
	if (baseX == NULL)
	{
		return;
	}
	float x = (float)(ScaleByViewHeight983(g_view, (float)(hitX - 140.0)) + baseX->Calc(scl));
	if (GOLFDOC()->m_shotPhase == 3)
	{
		CUiFrameSetManager::NamedDim* leftX = ui->FindNamedDim("d_driver_impact_left_x");
		CUiFrameSetManager::NamedDim* roundedW = ui->FindNamedDim("d_driver_impact_rounded_w");
		CUiFrameSetManager::NamedDim* thumbW = ui->FindNamedDim("d_thumb_rounded_w");
		if (leftX == NULL || roundedW == NULL || thumbW == NULL)
		{
			return;
		}
		int center = (int)(HITBAR()->pangyaPos + 0.5);
		int rw = (int)(roundedW->Calc(scl) + 0.5);
		float left = leftX->Calc(scl);
		float right = (float)rw + left;
		int tw = (int)(thumbW->Calc(scl) + 0.5);
		float twf = (float)tw;
		float half = twf * 0.5f;
		float thumbLeft = x - half;
		float thumbRight = thumbLeft + twf;
		float hw = (float)(tw / 2 + tw % 2);
		if (hitX < center)
		{
			int edge = center - width / 2 - width % 2;
			float d = thumbRight - left;
			if (hitX - edge == -1)
			{
				if (d > hw - 1.0)
				{
					x = (float)(left + (hw - 1.0) - half - (double)0.00001f);
				}
			}
			else if (hitX == edge && d < hw)
			{
				x = (float)(left + hw - half + (double)0.00001f);
			}
		}
		else if (hitX > center)
		{
			int edge = center + width / 2;
			float d = right - thumbLeft;
			if (hitX == edge)
			{
				if (d < hw)
				{
					x = (float)(right - hw + half - (double)0.00001f);
				}
			}
			else if (hitX - edge == 1 && d > hw - 1.0)
			{
				x = (float)(right - (hw - 1.0) + half + (double)0.00001f);
			}
		}
	}
	CUiFrameSetManager::NamedDim* thumbX = ui->FindNamedDim("d_thumb_x");
	if (thumbX != NULL)
	{
		thumbX->Set(x, 0);
		ui->DrawFrameSet("f_driver_thumb", g_view, 0, ((unsigned long)alpha << 24) | 0xFFFFFF);
	}
}

// 983 (00754bc0): cursor do putter ("f_putter_thumb") na posicao da batida
void CScreen::DrawPutterThumb983()
{
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::Size scl = HitBarScale983();
	CUiFrameSetManager::NamedDim* baseX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* thumbX = ui->FindNamedDim("d_thumb_x");
	if (baseX != NULL && thumbX != NULL)
	{
		double offset = ScaleByViewHeight983(g_view, (float)(HITBAR()->hitPos - 140.0));
		thumbX->Set((float)(baseX->Calc(scl) + offset), 0);
		ui->DrawFrameSet("f_putter_thumb", g_view, 0, (HITBAR()->alpha << 24) | 0xFFFFFF);
	}
}

// 983 (00758290): a barra de forca inteira pelos quadros do CUiFrameSetManager ("f_driver_gauge", "f_thumb_status_*"...);
// as overlays coreanas do 645 ("\xc3\xbc\xb7\xc2\xb0\xd4\xc0\xcc\xc1\xf6"...) sairam. Medidas com nome ("d_*") recebem a
// posicao/largura da vez antes de cada quadro.
void CScreen::DrawHitBarInfo()
{
	CUiFrameSetManager::Size scl = HitBarScale983();
	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::NamedDim* baseX = ui->FindNamedDim("d_default_pangya_x");
	CUiFrameSetManager::NamedDim* gaugeW = ui->FindNamedDim("d_gauge_w");
	float x0 = baseX != NULL ? baseX->Calc(scl) : 0.0f;
	float gw = gaugeW != NULL ? gaugeW->Calc(scl) : 0.0f;
	bool putter = false;
	if (GolfClub().GetType() == 3)
	{
		putter = true;
	}

	unsigned long color = (HITBAR()->alpha << 24) | 0xFFFFFF;
	if (!putter)
	{
		ui->DrawFrameSet("f_driver_gauge_left", g_view, 0, (HITBAR()->alpha << 24) | 0xFFFFFF);
		ui->DrawFrameSet("f_driver_gauge", g_view, 0, m_hitBarBaseColor & color);
	}
	else
	{
		ui->DrawFrameSet("f_putter_gauge", g_view, 0, m_hitBarBaseColor & color);
	}

	// trecho do buraco (green)
	if (GolfBall().m_groundType == 2 && m_hitBarFrom < m_hitBarCount)
	{
		CUiFrameSetManager::NamedDim* sectionX = ui->FindNamedDim("d_holecup_section_x");
		CUiFrameSetManager::NamedDim* sectionW = ui->FindNamedDim("d_holecup_section_w");
		if (baseX != NULL && gaugeW != NULL && sectionX != NULL && sectionW != NULL)
		{
			float step = gw / m_hitBarCount;
			sectionX->Set(m_hitBarFrom * step + x0, 0);
			sectionW->Set((float)(step + 1.0), 0);
			ui->DrawFrameSet(std::string("f_holecup_section"), g_view, 0, m_hitBarColor & color);
		}
	}

	// marcas da inclinacao do green (putter): altura relativa ao centro da barra (HandleMsg 0074eccc)
	if (putter && GolfBall().m_groundType == 2)
	{
		CUiFrameSetManager::NamedDim* gaugeY = ui->FindNamedDim("d_putter_gauge_y");
		CUiFrameSetManager::NamedDim* gaugeH = ui->FindNamedDim("d_putter_gauge_h");
		if (baseX != NULL && gaugeW != NULL && gaugeY != NULL && gaugeH != NULL)
		{
			float h = gaugeH->Calc(scl);
			float centerY = (float)(h * 0.5 + gaugeY->Calc(scl));
			float ratio = h / gaugeH->Calc(g_sclOne983);
			float step = gw / m_hitBarCount;
			WRect rect(0.0f, 0.0f, 3.0f, 3.0f);
			for (int i = 1; i < m_hitBarCount + 1; i++)
			{
				rect.x = (float)(((i - 1) + 0.5) * step + x0 - 1.0);
				rect.y = m_hitMarks[i].y * ratio + centerY - 1.0f;
				WOverlay::DrawBox(g_view, rect, 0, m_hitMarks[i].color & color, 0.001f);
			}
		}
	}

	// forca: a largura da barra cheia vai ate a batida (antes do impacto) ou ate a forca escolhida
	CUiFrameSetManager::NamedDim* powerW = ui->FindNamedDim("d_power_gauge_w");
	std::string power(putter ? "f_putter_power_gauge" : "f_driver_power_gauge");
	if (powerW != NULL)
	{
		if (GOLFDOC()->m_shotPhase < 2)
		{
			powerW->Set((float)(HITBAR()->hitPos - 140.0), 4);
		}
		else
		{
			powerW->Set((float)(HITBAR()->powerPos - 140.0), 4);
		}
		ui->DrawFrameSet(power, g_view, 0, 0xFFFFFFFF);
	}

	if (!putter)
	{
		CUiFrameSetManager::NamedDim* accuracyW = ui->FindNamedDim("d_half_accuracy_w");
		if (accuracyW != NULL)
		{
			accuracyW->Set(GolfClub().GetNarrowAccuracy(), 4);
			ui->DrawFrameSet("f_accuracy", g_view, 0, (HITBAR()->alpha << 24) | 0xFFFFFF);
			ui->DrawFrameSet("f_driver_bar", g_view, 0, color);
		}

		// janela de impacto (pangya) arredondada ao pixel da tela
		float area;
		if (IsMyTurn(true))
		{
			area = CPowerGauge::Instance()->GetPangYaArea();
		}
		else
		{
			area = *(float*)GDOCPTR983(GOLFDOC(), 0x48a); // 983: area do pangya de quem joga (campo sem nome no CGolfDoc)
		}
		float half = 0.0f;
		double width = area + area;
		if ((int)width % 2 == 1)
		{
			half = 0.5f;
		}
		CUiFrameSetManager::NamedDim* impactX = ui->FindNamedDim("d_driver_impact_left_x");
		CUiFrameSetManager::NamedDim* impactW = ui->FindNamedDim("d_driver_impact_rounded_w");
		if (impactX != NULL && impactW != NULL)
		{
			float left = (float)(ScaleByViewHeight983(g_view, (float)(HITBAR()->pangyaPos - 140.0 - area - half)) + x0);
			float rounded = (float)(int)(ScaleByViewHeight983(g_view, (float)width) + 0.5);
			impactX->Set(left, 0);
			impactW->Set(rounded, 0);
			ui->DrawFrameSet("f_driver_impact", g_view, 0, (HITBAR()->alpha << 24) | 0xFFFFFF);
			DrawDriverThumb983((int)(width + (double)0.00001f), (int)(HITBAR()->hitPos + 0.5), HITBAR()->alpha);
		}
	}
	else
	{
		std::string bar;
		if (GolfBall().m_groundType == 2)
		{
			bar = MakeStr("f_putter_%d_bar", m_hitBarCount);
		}
		else
		{
			bar = "f_putter_10_bar";
		}
		ui->DrawFrameSet(bar, g_view, 0, color);
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) && Doc()->m_playingMode == 2)
		{
			if (Doc()->m_replayState != 3)
			{
				HITBAR()->hitPos = BAR_START;
			}
			else
			{
				HITBAR()->hitPos = *(float*)&Doc()->m_replayPosition;
			}
		}
		DrawPutterThumb983();
	}

	std::string status;
	if (GOLFDOC()->m_shotPhase == 0)
	{
		status = "f_thumb_status_idle";
	}
	else if (GOLFDOC()->m_shotPhase == 1)
	{
		status = "f_thumb_status_power";
	}
	else
	{
		status = "f_thumb_status_impact";
	}
	ui->DrawFrameSet(status, g_view, 0, color);

	if (m_pPowerInfo)
	{
		WVector pos;
		SendMsg(this, "Spot", M983(0xf4), (int)&pos, 0, 0, 0);
		m_pPowerInfo->SetColor(color);
		m_pPowerInfo->SetPos(pos);
		m_pPowerInfo->Display();
	}

	// o CCalipers decide pelo modo se e o que desenha
	if (m_pCalipers)
	{
		m_pCalipers->Display();
	}
	if (GolfClub().GetType() != 3 && !reinterpret_cast<const sDocFlags&>(Doc()->m_myInfo.info.dwIdentity).Flag1()) // 983: +0x812 (645: +0x433)
	{
		if (GOLFDOC()->m_shotPhase == 1 && BAR_LENGTH * 0.5 + BAR_START < HITBAR()->hitPos)
		{
			ui->DrawFrameSet(std::string("f_power_click_indicator"), g_view, 0, 0xFFFFFFFF);
		}
		else if (GOLFDOC()->m_shotPhase == 2)
		{
			CUiFrameSetManager::NamedDim* offsetY = ui->FindNamedDim("d_impact_click_indicator_y_offset");
			if (offsetY != NULL)
			{
				offsetY->Set(Abs(m_hitBarTime), 4);
				ui->DrawFrameSet(std::string("f_impact_click_indicator"), g_view, 0, 0xFFFFFFFF);
			}
		}
	}
}

// 983 (00750980): ponteiro do relogio analogico = arco do quadro "f_analog_timer" (CUiFrameSetManager 006b9100);
// angulo inicial em +0x30c e cor em +0x310 (mensagem 0x21b), fim fixo 5.0614548 rad (290 graus)
void CScreen::DrawRemainTime()
{
	if (!CCapturedBg::Instance()->IsCaptured())
	{
		m_remainArcShown983 = m_remainArcStart983; // +0x308
	}

	CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
	CUiFrameSetManager::FrameSet* pSet = ui->FindFrameSet(std::string("f_analog_timer"));
	if (pSet)
	{
		CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0), (float)(g_view->GetHeight() / 480.0));
		pSet->DrawArc(g_view, scl, m_remainArcStart983, 5.0614548f, 0, m_remainArcColor983);
	}
}

// 983 006aea20: posicao do bloco do taco pelo ponto "p_club_window_area" (tela / 640x480, floor);
// sem o ponto: x = 58, y = barra - 6
static void GetClubWindowPos983(float& x, float& y)
{
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager::NamedPoint* pPt =
		CUiFrameSetManager::Instance()->FindNamedPoint("p_club_window_area");
	if (pPt)
	{
		WPoint pt;
		pPt->Calc(scl, pt);
		x = (float)floor((double)pt.x);
		y = (float)floor((double)pt.y);
	}
	else
	{
		x = 58.0f;
		y = COption::Instance()->gGetBar_Y() - 6.0f;
	}
}

// 983 00754dc0: barra/nome do taco no ponto "p_club_window_area"; fundos "f_ball_bg"/"f_club_type_bg" e a
// "embeddedness" da bola pelo CUiFrameSetManager (d_ball_embeddedness_*, escala pela altura de [ball_con.jpg)
void CScreen::DrawClubBallUI()
{
	if (IsMyTurn(false))
	{
		float px, py;
		GetClubWindowPos983(px, py);
		sOverlay* overlay = m_overlays.Find("\xc5\xac\xb7\xb4\xb9\xd9");
		WRect clip;
		clip.x = px;
		clip.y = py;
		clip.w = GolfClub().GetBarWidth();
		clip.h = 32.0f;
		unsigned long color = (GolfClub().GetBarAlpha() << 24) | 0xFFFFFF;

		if (overlay && overlay->m_pOverlay)
		{
			overlay->m_pOverlay->SetClippingArea(&clip);
			overlay->m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(clip.x - 456.0f + clip.w, clip.y, 512.0f, 32.0f), 0,
				color, 0.0f, 0);
		}

		if (GolfClub().BarIs(0) || GolfClub().BarIs(3))
		{
			if (m_pClubLineUI)
			{
				m_pClubLineUI->SetColor(color);
				m_pClubLineUI->Render(GolfClub().GetName(), px + 27.0f, py + 11.0f, 1.0f, 0x40);
			}

			if (m_pIconUI)
			{
				m_pIconUI->SetColor(color);
				m_pIconUI->Render("\xc7\xf6\xc0\xe7\xc5\xac\xb7\xb4",
					(px + 436.0f) - Min<unsigned char>(GolfClub().GetIndex(), 13) * 26.0f,
					py + 11.0f, 1.0f, 0x22);
			}
		}
	}

	DrawHudFrame983(IsNtBallItem983() ? "f_ball_bg_ntball" : "f_ball_bg");
	DrawHudFrame983("f_club_type_bg");

	if (m_pGroundInfo)
	{
		m_pGroundInfo->Display();
	}

	if (GolfBall().m_playFrame < 0)
	{
		unsigned long color;
		switch (GolfBall().m_groundType)
		{
		case 0:
		case 1:
		case 2:
		case 6:
		case 7:
			goto skip;
		case 3:
			color = 0x80E5CA77;
			break;
		case 5:
			color = 0x80FFFFFF;
			break;
		default:
			color = 0x8078E577;
			break;
		}

		CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
		WOverlay* ball = ui->GetOverlay("[ball_con.jpg");
		CUiFrameSetManager::NamedDim* imgY = ui->FindNamedDim("d_ball_embeddedness_img_y");
		CUiFrameSetManager::NamedDim* imgH = ui->FindNamedDim("d_ball_embeddedness_img_h");
		CUiFrameSetManager::NamedDim* offY = ui->FindNamedDim("d_ball_embeddedness_y_offset");
		CUiFrameSetManager::NamedDim* dstH = ui->FindNamedDim("d_ball_embeddedness_h");
		if (ball && imgY && imgH && offY && dstH)
		{
			float height = (float)ball->GetHeight();
			imgY->Set(m_ballStateFade * height, 0);
			imgH->Set((1.0f - m_ballStateFade) * height, 0);
			offY->Set(m_ballStateFade * height, 4);
			dstH->Set((1.0f - m_ballStateFade) * height, 4);
			DrawHudFrame983("f_ball_embeddedness", color);
		}
	}
skip:

	if (m_pClubTypeInfo)
	{
		m_pClubTypeInfo->Display();
	}
}

void CScreen::DrawCommandArrow()
{
	// 983 (00755500): as teclas da tacada saem pelos quadros "UP"/"DOWN"/"LEFT"/"RIGHT"/"INVALID" (+ "_S" se a proxima
	// entrada e 0x10) do CUiFrameSetManager; a posicao vertical vem da medida "d_shot_command_x_offset" (22 por tecla)
	if (GOLFDOC()->m_gameMode & 0x60)
	{
		CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
		CUiFrameSetManager::NamedDim* dim = ui->FindNamedDim("d_shot_command_x_offset");
		float y = 0.0f;
		std::string name;

		std::list<unsigned long>::iterator it = GOLFDOC()->m_inputList983.begin();
		while (it != GOLFDOC()->m_inputList983.end())
		{
			if (*it == 0x10)
			{
				++it;
				continue;
			}

			switch (*it)
			{
			case 1:
				name = "UP";
				break;
			case 2:
				name = "DOWN";
				break;
			case 4:
				name = "LEFT";
				break;
			case 8:
				name = "RIGHT";
				break;
			case 0x40:
				name = "INVALID";
				break;
			case 0x20:
				name = "INVALID";
				y -= 22.0f;
				break;
			}

			++it;
			if (it != GOLFDOC()->m_inputList983.end() && *it == 0x10)
			{
				name += "_S";
			}

			if (dim)
			{
				dim->Set(y, 4);
				ui->DrawFrameSet(name, g_view, 0, 0xffffffff);
			}
			y += 22.0f;
		}
	}
}

void CScreen::DrawTutorialUI()
{
	WRect rect;

	// 983 (00759290): sem saida antecipada para o modo 15; ele so cai no default do switch final
	if (!CCapturedBg::Instance()->IsCaptured())
	{
		if (GOLFDOC()->m_tutorialMode == 0 && m_tutoBlinkOn)
		{
			DrawOverlay("\xc1\xb6\xc1\xd8\xbf\xb5\xbf\xaa",
				WRect(g_view->GetWidth() * 0.5f - 64.0f,
					g_view->GetHeight() * 0.5f - 64.0f, 128.0f, 128.0f),
				0xFF, 0);
		}

		if ((Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
				Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV) &&
			m_pTutoEscUI)
		{
			// 983 (00759290): MSG0/MSG1 saem da base "d_default_pangya_x" (+10 px na escala do tipo 4), 256 px
			// de distancia entre os dois, e 29 px acima do rodape (nao mais em 150/406 fixos)
			CUiFrameSetManager::Size scl(g_view->GetWidth() / 640.0, g_view->GetHeight() / 480.0);
			CUiFrameSetManager::NamedDim* baseX =
				CUiFrameSetManager::Instance()->FindNamedDim("d_default_pangya_x");
			if (baseX)
			{
				float y = (float)(g_view->GetHeight() - 29.0);
				CUiFrameSetManager::Dim off;
				off.Set(10.0f, 4);
				float x = (float)(baseX->Calc(scl) + off.Calc(scl));
				m_pTutoEscUI->Render("MSG0", x, y, 1.0f, 0);
				m_pTutoEscUI->Render("MSG1", (float)(x + 256.0), y, 1.0f, 0);
			}
		}

		if (GOLFDOC()->m_tutorialMode == 5)
		{
			if (CItemWindow::IsClosed())
			{
				DrawOverlay("\xb9\xf6\xc6\xb0\xc3\xa2_normal", rect, 0xFF, 0);
			}
			else
			{
				DrawOverlay("\xb9\xf6\xc6\xb0\xc3\xa2_item", rect, 0xFF, 0);
			}
		}

		if (m_tutoBlinkOn)
		{
			if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
			{
				DrawOverlay(
					MakeStr(
						"\xc4\xf9\xbd\xba\xc6\xae\xb9\xcc\xbc\xc7\xc1\xa4\xba\xb8%d",
						GOLFDOC()->m_tutorialMode),
					WRect(g_view->GetWidth() - 256.0f, 26.0f, 256.0f, 256.0f),
					0xFF, 0);
			}
			else
			{
				DrawOverlay(
					"\xc4\xf9\xbd\xba\xc6\xae\xb9\xcc\xbc\xc7\xc1\xa4\xba\xb8"
					"0",
					WRect(g_view->GetWidth() - 256.0f, 26.0f, 256.0f, 128.0f),
					0xFF, 0);
			}
		}

		if (m_pTutoTextUI)
		{
			m_pTutoTextUI->SetColor((m_tutoTextAlpha << 24) | 0xFFFFFF);
			m_pTutoTextUI->Render(
				MakeStr("\xc5\xb8\xc0\xcc\xc6\xb2_%d", m_tutoStep + 1),
				m_tutoTextX + 20.0f, m_tutoTextY, 1.0f, 0);
		}
	}

	int index;
	switch (GOLFDOC()->m_tutorialMode)
	{
	case 1:
		index = 0;
		break;
	case 2:
		index = 1;
		break;
	case 3:
		index = 2;
		break;
	case 4:
		index = 3;
		break;
	case 5:
		index = 4;
		break;
	case 8:
		index = 5;
		break;
	case 9:
		index = 6;
		break;
	case 10:
		index = 7;
		break;
	case 11:
		index = 8;
		break;
	case 12:
		index = 9;
		break;
	case 13:
		index = 10;
		break;
	case 14:
		index = 11;
		break;
	default:
		return;
	}

	if (m_tutoTitleState != 3)
	{
		if (index == 6)
		{
			m_pQuestMsgUI[6]->Render("MissionTitle", m_tutoX, m_tutoY + 20.0f,
				m_tutoScale, 2);
		}
		else
		{
			m_pQuestMsgUI[index]->Render("MissionTitle", m_tutoX, m_tutoY,
				m_tutoScale, 2);
		}
	}
}
