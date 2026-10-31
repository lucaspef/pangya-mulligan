#include "jp/shareddocjp.h"
#include "minatl.h"
#include "frcombobox.h"
#define REBANG_TREASUREDLG_CPP
#include "treasuredlg.h"
#include "treasure.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frgaugebar.h"
#include "frlistbox.h"
#include "frbutton.h"
#include "frviewer.h"
#include "projectg.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "sea.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "netresourcemanager.h"
#include "simpleoverlay.h"
#include "commonutil.h"
#include "soundmanager.h"
#include <time.h>

// HACK: the linker keeps the copy of -30.0f of golfplayer.obj (earlier in link order), which is a local static of a
// naked function there, so this unit must not emit its own copy: name that one.
#define RB_REAL_M30 -30.0f


extern Fresh* g_pFresh;

unsigned char GetNextHole();

namespace nsTHunter
{
	bool __fastcall IsWndSizeMinimum()
	{
		WRect rect = g_pFresh->GetManager()->GetDesktop()->GetRect();
		if (rect.w == 640.0f && rect.h == 480.0f)
			return true;
		return false;
	}

	inline unsigned long __fastcall CalcGauge(unsigned long gauge)
	{
		if (gauge < 701)
			return 0;
		return gauge - 700;
	}
}

bool __fastcall CalculateUVPos(WRect& rect, int texWidth, int texHeight,
	float cellWidth, float cellHeight, int index)
{
	float fTexWidth = (float)texWidth;
	if (!(fTexWidth < cellWidth))
	{
		float fTexHeight = (float)texHeight;
		if (!(fTexHeight < cellHeight))
		{
			int cols = (int)(fTexHeight / cellHeight);
			if ((int)(fTexWidth / cellWidth) * cols < index)
				return false;
			// 983 0083be90: divide pelo tamanho da textura (o 645 multiplicava pelo inverso)
			float x = (float)(index / cols) * cellWidth;
			rect.x = x / fTexWidth;
			float y = (float)(index % cols) * cellHeight;
			rect.y = y / fTexHeight;
			rect.w = cellWidth / fTexWidth;
			rect.h = cellHeight / fTexHeight;
			return true;
		}
	}
	return false;
}

IMPLEMENT_OBJECT(FrNextHole, FrForm)

BEGIN_FRESH_MSGMAP(FrNextHole, FrForm)
ON_FRESH_VI("next", FRCMD_INIT, FrNextHole::OnAreaNextHole_Init)
ON_FRESH_VI("next", FRCMD_OWNERDRAW, FrNextHole::OnAreaNextHole_OwnerDraw)
ON_FRESH_VI("outback1", FRCMD_INIT, FrNextHole::OnArea_OutFrame_Init)
ON_FRESH_VI("outback2", FRCMD_INIT, FrNextHole::OnArea_OutFrame_Init)
ON_FRESH_VI("outback3", FRCMD_INIT, FrNextHole::OnArea_OutFrame_Init)
END_FRESH_MSGMAP()

FrNextHole::FrNextHole()
{
	m_pHoleOverlay[0] = NULL;
	m_pHoleOverlay[1] = NULL;
	m_pNextHoleArea = NULL;
	m_overlayIndex = 0;
	m_state = (eState)0;
	m_pHoleOverlay[2] = NULL;
}

FrNextHole::~FrNextHole()
{
	if (g_resrcmng)
	{
		if (m_pHoleOverlay[1])
		{
			g_resrcmng->Release(m_pHoleOverlay[1]);
			m_pHoleOverlay[1] = NULL;
		}
		if (g_resrcmng)
		{
			if (m_pHoleOverlay[0])
			{
				g_resrcmng->Release(m_pHoleOverlay[0]);
				m_pHoleOverlay[0] = NULL;
			}
			if (g_resrcmng && m_pHoleOverlay[2])
			{
				g_resrcmng->Release(m_pHoleOverlay[2]);
				m_pHoleOverlay[2] = NULL;
			}
		}
	}
}

void FrNextHole::OnAreaNextHole_Init(int param)
{
	m_pNextHoleArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

bool FrNextHole::OnInit()
{
	FrForm::OnInit();

	unsigned int map;
	if (GetCurMap() <= 0x7f || GetCurMap() == 0xfd)
		map = GetCurMap();
	else
		map = GetCurMap() - 0x80;

	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(map | 0x28000000);
	if (!course)
		return false;

	m_pHoleOverlay[0] =
		g_resrcmng->GetOverlay(MakeStr("%s_1.png", course->c.Name), 0);
	m_pHoleOverlay[1] =
		g_resrcmng->GetOverlay(MakeStr("%s_2.png", course->c.Name), 0);
	m_pHoleOverlay[2] = g_resrcmng->GetOverlay("next_chaos.tga", 0);
	// 983 008404b7: modo caos mede next_chaos.tga; senao precisa das 2 imagens
	if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
	{
		m_texWidth = m_pHoleOverlay[2]->GetWidth();
		m_texHeight = m_pHoleOverlay[2]->GetHeight();
	}
	else if (m_pHoleOverlay[0] && m_pHoleOverlay[1])
	{
		m_texWidth = m_pHoleOverlay[0]->GetWidth();
		m_texHeight = m_pHoleOverlay[0]->GetHeight();
	}
	m_outFrames.reserve(3);
	MoveWindow(GetCalcPoint());
	return true;
}

void FrNextHole::SetNextHole(int hole)
{
	if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
	{
		m_overlayIndex = 2;
		m_uvRect.x = 0.0f;
		m_uvRect.y = 0.0f;
		m_uvRect.w = 1.0f;
		m_uvRect.h = 1.0f;
		return;
	}

	if (m_pHoleOverlay[0] && m_pHoleOverlay[1])
	{
		if (hole <= 10 && hole != 19 &&
			Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] !=
				GOLFDOC()->m_currentHole)
		{
			m_overlayIndex = 0;
		}
		else
		{
			m_overlayIndex = 1;
			hole -= 10;
		}

		if (hole == 19 ||
			Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] ==
				GOLFDOC()->m_currentHole)
		{
			CalculateUVPos(m_uvRect, m_texWidth, m_texHeight, 140.0f, 93.0f, 8);
		}
		else
		{
			CalculateUVPos(m_uvRect, m_texWidth, m_texHeight, 140.0f, 93.0f,
				hole - 1);
		}
	}
}

void FrNextHole::OnAreaNextHole_OwnerDraw(int param)
{
	if (m_state != 1)
	{
		WRect rect(m_pNextHoleArea->GetRect().TopLeft(), WSize(140.0f, 93.0f));
		if (m_pHoleOverlay[m_overlayIndex])
			m_pHoleOverlay[m_overlayIndex]->Render(g_view, m_uvRect, rect, 0,
				0xffffffff, 0.0f, 0);
	}
}

void FrNextHole::OnArea_OutFrame_Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
		m_outFrames.push_back(pArea);
}

void FrNextHole::SetState(eState state)
{
	if (m_state == state)
	{
		if (IsEnabled() && m_state == 1)
		{
			Enable(false);
			SetVisible(false);
		}
	}
	else
	{
		m_state = state;
		if (state == 1)
		{
			Enable(false);
			SetVisible(false);
		}
		else
		{
			Enable(true);
			MoveWindow(GetCalcPoint());
			SetVisible(true);
			SetOutFrameVisible(!nsTHunter::IsWndSizeMinimum());
			SetNextHole(GetNextHole());
		}
	}
}

WPoint FrNextHole::GetCalcPoint()
{
	WRect rect = m_rect;
	WRect desk = g_pFresh->GetManager()->GetDesktop()->GetRect();
	float offset = (desk.w == 640.0f && desk.h == 480.0f) ? 6.0f : -38.0f;
	WPoint pos;
	int x = (int)((g_view->GetWidth() - rect.w) * 0.5f);
	pos.x = (float)x + 210.0f;
	pos.y = (float)(int)((g_view->GetHeight() - rect.h) + offset);
	return pos;
}

void FrNextHole::SetOutFrameVisible(bool bVisible)
{
	int count = m_outFrames.size();
	for (int i = 0; i < count; i++)
	{
		if (m_outFrames[i])
			m_outFrames[i]->SetVisible(bVisible);
	}
}

IMPLEMENT_OBJECT(FrTreasureGauge, FrForm)

BEGIN_FRESH_MSGMAP(FrTreasureGauge, FrForm)
ON_FRESH_VI("pointgauge", FRCMD_INIT, FrTreasureGauge::OnImageBarInit)
ON_FRESH_VI("gaugearrow", FRCMD_INIT, FrTreasureGauge::OnArea_Arrow_Init)
ON_FRESH_VI("gaugearrow", FRCMD_OWNERDRAW,
	FrTreasureGauge::OnArea_Arrow_OwnerDraw)
ON_FRESH_VI("outback1", FRCMD_INIT, FrTreasureGauge::OnArea_OutFrame_Init)
ON_FRESH_VI("outback2", FRCMD_INIT, FrTreasureGauge::OnArea_OutFrame_Init)
ON_FRESH_VI("outback3", FRCMD_INIT, FrTreasureGauge::OnArea_OutFrame_Init)
END_FRESH_MSGMAP()

FrTreasureGauge::FrTreasureGauge()
{
	m_pImageBar = NULL;
	m_pArrowArea = NULL;
	m_state = (eState)0;
}

FrTreasureGauge::~FrTreasureGauge()
{
}

void FrTreasureGauge::OnImageBarInit(int param)
{
	m_pImageBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pImageBar)
	{
		m_pImageBar->SetRange(0, 1000, 0);
		m_pImageBar->SetBarSpeed(250.0f);
	}
}

bool FrTreasureGauge::OnInit()
{
	m_pArrowBmp = g_pFresh->RegisterBitmap("th_gage_arrow");
	m_outFrames.reserve(3);
	MoveWindow(GetCalcPoint());
	return true;
}

void FrTreasureGauge::SetTPoint()
{
	unsigned long point = CTHunter::Instance()->GetTreasurePoint();
	if (m_pImageBar)
		m_pImageBar->SetDestPos(point, false);
}

void FrTreasureGauge::SetState(eState state)
{
	// 983 (0083ec40): nos tipos de jogo 0x14/0x15 o gauge fica sempre escondido
	if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
	{
		Enable(false);
		SetVisible(false);
		return;
	}

	if (m_state != state)
	{
		m_state = state;
		if (state == 1)
		{
			Enable(false);
			SetVisible(false);
		}
		else
		{
			Enable(true);
			MoveWindow(GetCalcPoint());
			SetVisible(true);
			SetOutFrameVisible(!nsTHunter::IsWndSizeMinimum());
			if (state == 3)
				SetTPoint();
		}
	}
}

WPoint FrTreasureGauge::GetCalcPoint()
{
	WRect rect = m_rect;
	WRect desk = g_pFresh->GetManager()->GetDesktop()->GetRect();
	float offset = (desk.w == 640.0f && desk.h == 480.0f) ? 14.0f : RB_REAL_M30;
	WPoint pos;
	int x = (int)((g_view->GetWidth() - rect.w) * 0.5f);
	pos.x = (float)x - 90.0f;
	pos.y = (float)(int)((g_view->GetHeight() - rect.h) + offset);
	return pos;
}

void FrTreasureGauge::SetOutFrameVisible(bool bVisible)
{
	int count = m_outFrames.size();
	for (int i = 0; i < count; i++)
	{
		if (m_outFrames[i])
			m_outFrames[i]->SetVisible(bVisible);
	}
}

void FrTreasureGauge::OnArea_Arrow_Init(int param)
{
	m_pArrowArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTreasureGauge::OnArea_Arrow_OwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	static float s_pixelPerHeight = (float)m_pImageBar->GetPixelPerHeight();
	if (m_pArrowBmp)
	{
		WRect barRect = m_pImageBar->GetRect();
		s_pixelPerHeight = (float)m_pImageBar->GetPixelPerHeight();
		pGI->DrawTexture(m_pArrowBmp,
			WRect((s_pixelPerHeight + barRect.x) - 7.0f, barRect.y - 15.0f,
				(float)m_pArrowBmp->Width(), (float)m_pArrowBmp->Height()),
			0xffffffff, 0);
	}
}

void FrTreasureGauge::OnArea_OutFrame_Init(int param)
{
	FrArea* pArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (pArea)
		m_outFrames.push_back(pArea);
}

IMPLEMENT_OBJECT(FrTreasureCourse, FrForm)

BEGIN_FRESH_MSGMAP(FrTreasureCourse, FrForm)
ON_FRESH_VI("selectmap", FRCMD_INIT, FrTreasureCourse::OnAreaSelectMapInit)
ON_FRESH_VI("bluelagoon", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("bluelagoon", FRCMD_LBUTTONUP,
	FrTreasureCourse::OnBtnMap_Lagoon_Down)
ON_FRESH_VI("bluelagoon", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Lagoon_OwnerDraw)
ON_FRESH_VI("bluewater", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("bluewater", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Water_Down)
ON_FRESH_VI("bluewater", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Water_OwnerDraw)
ON_FRESH_VI("sepiawind", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("sepiawind", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Sepia_Down)
ON_FRESH_VI("sepiawind", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Sepia_OwnerDraw)
ON_FRESH_VI("windhill", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("windhill", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Hill_Down)
ON_FRESH_VI("windhill", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Hill_OwnerDraw)
ON_FRESH_VI("wizwiz", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("wizwiz", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_WizWiz_Down)
ON_FRESH_VI("wizwiz", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_WizWiz_OwnerDraw)
ON_FRESH_VI("westwiz", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("westwiz", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_West_Down)
ON_FRESH_VI("westwiz", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_West_OwnerDraw)
ON_FRESH_VI("bluemoon", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("bluemoon", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Moon_Down)
ON_FRESH_VI("bluemoon", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Moon_OwnerDraw)
ON_FRESH_VI("silvia", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("silvia", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Silvia_Down)
ON_FRESH_VI("silvia", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Silvia_OwnerDraw)
ON_FRESH_VI("icecannon", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("icecannon", FRCMD_LBUTTONUP,
	FrTreasureCourse::OnBtnMap_Cannon_Down)
ON_FRESH_VI("icecannon", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Cannon_OwnerDraw)
ON_FRESH_VI("whitewiz", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("whitewiz", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_White_Down)
ON_FRESH_VI("whitewiz", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_White_OwnerDraw)
ON_FRESH_VI("shining", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("shining", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Shining_Down)
ON_FRESH_VI("shining", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Shining_OwnerDraw)
ON_FRESH_VI("pinkwind", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("pinkwind", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Pink_Down)
ON_FRESH_VI("pinkwind", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Pink_OwnerDraw)
ON_FRESH_VI("inferno", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("inferno", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Inferno_Down)
ON_FRESH_VI("inferno", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Inferno_OwnerDraw)
ON_FRESH_VI("icespa", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("icespa", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_IceSpa_Down)
ON_FRESH_VI("icespa", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_IceSpa_OwnerDraw)
ON_FRESH_VI("seaway", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("seaway", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_SeaWay_Down)
ON_FRESH_VI("seaway", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_SeaWay_OwnerDraw)
ON_FRESH_VI("panda", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("panda", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Panda_Down)
ON_FRESH_VI("panda", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Panda_OwnerDraw)
ON_FRESH_VI("wizcity", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("wizcity", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Wizcity_Down)
ON_FRESH_VI("wizcity", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Wizcity_OwnerDraw)
ON_FRESH_VI("demon_land_map", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("demon_land_map", FRCMD_LBUTTONUP,
	FrTreasureCourse::OnBtnMap_IceInferno_Down)
ON_FRESH_VI("demon_land_map", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_IceInferno_OwnerDraw)
ON_FRESH_VI("abbot_mine", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("abbot_mine", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_AbbotMine_Down)
ON_FRESH_VI("abbot_mine", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_AbbotMine_OwnerDraw)
ON_FRESH_VI("luna", FRCMD_INIT, FrTreasureCourse::OnBtnMap_ALL_Init)
ON_FRESH_VV("luna", FRCMD_LBUTTONUP, FrTreasureCourse::OnBtnMap_Luna_Down)
ON_FRESH_VI("luna", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnBtnMap_Luna_OwnerDraw)
ON_FRESH_VI("random", FRCMD_INIT, FrTreasureCourse::OnBtnMap_Random_Init)
ON_FRESH_VV("random", FRCMD_LBUTTONDOWN, FrTreasureCourse::OnBtnMap_Random_Down)
ON_FRESH_VI("f1", FRCMD_INIT, FrTreasureCourse::OnViewerF1)
ON_FRESH_VI("f2", FRCMD_INIT, FrTreasureCourse::OnViewerF2)
ON_FRESH_VI("gauge1", FRCMD_INIT, FrTreasureCourse::OnImageBar)
ON_FRESH_VI("gauge2", FRCMD_INIT, FrTreasureCourse::OnGauge_SmallBar)
ON_FRESH_VI("gauge2", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnGauge_SmallBar_OwnerDraw)
ON_FRESH_VI("currentmap", FRCMD_INIT, FrTreasureCourse::OnViewer_CurrMap_Init)
ON_FRESH_VI("explane", FRCMD_INIT, FrTreasureCourse::OnArea_Explane_Init)
ON_FRESH_VI("explane", FRCMD_OWNERDRAW,
	FrTreasureCourse::OnArea_Explane_OwnerDraw)
ON_FRESH_VI("cloud", FRCMD_INIT, FrTreasureCourse::OnInit_Cloud_Area)
ON_FRESH_VI("cloud", FRCMD_OWNERDRAW, FrTreasureCourse::OnOwnerDraw_Cloud_Area)
END_FRESH_MSGMAP()

FrTreasureCourse::FrTreasureCourse()
{
	FrButton* pNull = NULL;
	m_mapBtns.reserve(22);
	m_mapBtns.insert(m_mapBtns.begin(), 22, pNull);
	for (int i = 0; i < 22; i++)
	{
		m_pMapBmp[i] = NULL;
		m_gauge[i] = 0;
	}
	m_pRandomSmallBmp = NULL;
	m_unused26c = 0;
	m_pRandomBtn = NULL;
	m_pCloudOverlay = NULL;
	m_pSelectMapArea = NULL;
	m_pViewerF1 = NULL;
	m_pViewerF2 = NULL;
	m_curMapIdx = 0;
	m_unused1fc = 0;
	m_unused284 = 0.0f;
	m_unused288 = 0.0f;
	m_btnIndex = 0;
	m_unused1f9 = 0;
	m_pImageBar = NULL;
	m_pSmallBar = NULL;
	m_gaugeIndex = -1;
	m_bMapIndexChanged = true;
}

FrTreasureCourse::~FrTreasureCourse()
{
	if (g_resrcmng && m_pCloudOverlay)
	{
		g_resrcmng->Release(m_pCloudOverlay);
		m_pCloudOverlay = NULL;
	}
}

bool FrTreasureCourse::OnInit()
{
	m_gaugePos[0] = WPoint(291.0f, 390.0f);
	m_gaugePos[1] = WPoint(129.0f, 391.0f);
	m_gaugePos[2] = WPoint(98.0f, 261.0f);
	m_gaugePos[3] = WPoint(177.0f, 226.0f);
	m_gaugePos[4] = WPoint(234.0f, 146.0f);
	m_gaugePos[5] = WPoint(170.0f, 160.0f);
	m_gaugePos[6] = WPoint(53.0f, 349.0f);
	m_gaugePos[7] = WPoint(230.0f, 457.0f);
	m_gaugePos[8] = WPoint(25.0f, 231.0f);
	m_gaugePos[9] = WPoint(298.0f, 138.0f);
	m_gaugePos[10] = WPoint(493.0f, 278.0f);
	m_gaugePos[11] = WPoint(187.0f, 305.0f);
	m_gaugePos[12] = WPoint(0.0f, 0.0f);
	m_gaugePos[13] = WPoint(329.0f, 239.0f);
	m_gaugePos[14] = WPoint(52.0f, 151.0f);
	m_gaugePos[15] = WPoint(406.0f, 351.0f);
	m_gaugePos[16] = WPoint(339.0f, 329.0f);
	m_gaugePos[18] = WPoint(378.0f, 163.0f);
	m_gaugePos[19] = WPoint(257.0f, 224.0f);
	m_gaugePos[20] = WPoint(461.0f, 189.0f);
	m_gaugePos[21] = WPoint(490.0f, 362.0f);

	WPoint pos = m_rect.TopLeft();
	for (int i = 0; i < 22; i++)
	{
		if (i != 12 && i != 17)
		{
			m_gaugePos[i].x += pos.x;
			m_gaugePos[i].y += pos.y;
		}
	}

	m_pRandomSmallBmp = g_pFresh->RegisterBitmap("map_random_small");
	m_pCloudBmp = g_pFresh->RegisterBitmap("th_cloud");
	m_pCloudOverlay = g_resrcmng->GetOverlay("th_cloud.tga", 0);
	m_pGaugeTipBmp = g_pFresh->RegisterBitmap("th_gage_tip");
	m_pGaugeHighBmp = g_pFresh->RegisterBitmap("gauge_high");
	m_pGaugeLowBmp = g_pFresh->RegisterBitmap("gauge_low");
	m_pMapLockBmp = g_pFresh->RegisterBitmap("map_lock_small2");
	m_orgRect = m_rect;

	CTHunter::Instance()->DecideTMark();
	SetMapGauge();
	InitCloud();

	if (Doc()->m_curChannel.Type & 0x800)
	{
		unsigned char num = (unsigned char)m_mapBtns.size();
		for (unsigned char i = 0; i < num; i++)
		{
			if (i == 17 || i == 18)
				continue;

			unsigned int tid;
			if (i <= 0x7f || i == 0xfd)
				tid = i;
			else
				tid = i - 0x80;

			IFF_STRUCT::sCourse* course =
				ItemManager()->FindCourse(tid | 0x28000000);
			if (!course)
				continue;

			if (course->Difficulty > 3)
			{
				m_mapBtns[i]->Enable(false);
			}
			else
			{
				switch (i)
				{
				case MAP_BLUE_WATER:
				case MAP_BLUE_MOON:
				case MAP_ICE_CANNON:
				case MAP_WHITE_WIZ:
				case MAP_ICE_SPA:
					m_mapBtns[i]->Enable(false);
					break;
				}
			}
		}
		if (m_pRandomBtn)
			m_pRandomBtn->Enable(false);
	}
	return true;
}

void FrTreasureCourse::OnProc(const float dt)
{
	ProcCloud(dt);
}

void FrTreasureCourse::InitCloud()
{
	unsigned int num[2] = { 3, 4 };
	srand((unsigned int)time(NULL));
	unsigned int count = num[rand() % 2];
	sCloud cloud;
	m_clouds.reserve(count);
	for (unsigned int i = 0; i < count; i++)
		m_clouds.push_back(cloud);
	m_cloudNum = m_clouds.size();
	for (int i = 0; i < m_cloudNum; i++)
		ReInitCloud(i);
}

void FrTreasureCourse::ReInitCloud(int index)
{
	float scales[3] = { 0.25f, 0.5f, 1.0f };
	float rnd[10] = { 0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
		9.0f };
	float areaY = m_pCloudArea->GetRect().y;
	float areaH = m_pCloudArea->GetRect().h;
	float step = m_rect.h / (float)m_cloudNum;
	float y = (float)index * step;
	float minY = y + areaY;
	float maxY = (areaH + areaY) - (areaY + 20.0);

	y = step / rnd[rand() % 10] + y;
	if (y < minY)
		y = minY;
	else if (y > maxY)
		y = maxY;

	sCloud cloud;
	WRect rect(0.0f, 0.0f,
		(float)(unsigned int)m_pCloudBmp->bi->bmiHeader.biWidth,
		(float)(unsigned int)m_pCloudBmp->bi->bmiHeader.biHeight);

	cloud.speed = Random(1.0f, 10.0f);
	if (cloud.speed < 5.0f)
		cloud.speed = 5.0f;
	else if (cloud.speed > 10.0f)
		cloud.speed = 10.0f;

	// The original indexes one past the table (rand() % 4 over three scales).
	cloud.scale = scales[rand() % 4];
	rect.x = m_pCloudArea->GetRect().w / 10.0 * rnd[rand() % 10];
	if (1.0f - cloud.scale >= (double)0.0001f)
	{
		rect.w *= cloud.scale;
		rect.h = cloud.scale * rect.h;
		if (cloud.scale != 1.0f)
		{
			rect.w += 0.5;
			rect.h += 0.5;
		}
	}
	if (maxY < rect.w + y)
		y -= rect.w;
	rect.y = y;

	cloud.rect = rect;
	cloud.alpha = 0;
	if (m_pCloudArea->GetRect().x < rect.x)
	{
		cloud.bEntered = true;
		cloud.alpha = 0xff;
	}
	m_clouds[index] = cloud;
}

void FrTreasureCourse::ProcCloud(float dt)
{
	unsigned char i = 0;
	while ((int)i < m_cloudNum)
	{
		CheckArea(i, dt);
		sCloud& cloud = m_clouds[i];
		i++;
		cloud.rect.x = dt * cloud.speed + cloud.rect.x;
	}
}

void FrTreasureCourse::CheckArea(int index, float dt)
{
	// 983 copies both rects into locals.
	WRect area = m_pCloudArea->GetRect();
	WRect cloud = m_clouds[index].rect;
	float areaR = area.x + area.w;
	float cloudR = cloud.x + cloud.w;
	float limit = cloud.w * 2 + area.x;
	if (!(cloud.x >= area.x) || !(limit > cloudR))
	{
		if (cloudR > areaR && cloudR + areaR > cloud.x)
			ShiftAlpha(index, dt, eALPHA_DEC);
	}
	else
		ShiftAlpha(index, dt, eALPHA_INC);
	if (areaR < cloud.x)
		ReInitCloud(index);
}

void FrTreasureCourse::ShiftAlpha(int index, float dt, eALPHA mode)
{
	if (mode == eALPHA_INC)
	{
		sCloud& cloud = m_clouds[index];
		if (!cloud.bEntered)
		{
			if (cloud.alpha < 0xff)
				cloud.alpha++;
			else
				cloud.alpha = 0xff;
		}
	}
	else if (mode == eALPHA_DEC)
	{
		sCloud& cloud = m_clouds[index];
		if (cloud.alpha > 0)
			cloud.alpha -= 5;
		else
			cloud.alpha = 0;
	}
}

void FrTreasureCourse::OnAreaSelectMapInit(int param)
{
	m_pSelectMapArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTreasureCourse::OnBtnMap_ALL_Init(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (pBtn)
	{
		// 983 0083d1b1: pula so 12 e 17; o 18 do 645 deixava m_mapBtns[0x12] (Ice Inferno) nulo
		if (m_btnIndex == 12 || m_btnIndex == 17)
			m_btnIndex++;
		m_mapBtns[m_btnIndex++] = pBtn;
		if (m_btnIndex == (int)m_mapBtns.size())
			m_btnIndex = 0;
	}
}

void FrTreasureCourse::OnBtnMap_Lagoon_Down()
{
	SelectMapBtn(MAP_BLUE_LAGOON);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Lagoon_OwnerDraw(int param)
{
	DrawMapBtn(MAP_BLUE_LAGOON);
}

void FrTreasureCourse::OnBtnMap_Water_Down()
{
	SelectMapBtn(MAP_BLUE_WATER);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Water_OwnerDraw(int param)
{
	DrawMapBtn(MAP_BLUE_WATER);
}

void FrTreasureCourse::OnBtnMap_Sepia_Down()
{
	SelectMapBtn(MAP_SEPIA_WIND);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Sepia_OwnerDraw(int param)
{
	DrawMapBtn(MAP_SEPIA_WIND);
}

void FrTreasureCourse::OnBtnMap_Hill_Down()
{
	SelectMapBtn(MAP_WIND_HILL);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Hill_OwnerDraw(int param)
{
	DrawMapBtn(MAP_WIND_HILL);
}

void FrTreasureCourse::OnBtnMap_WizWiz_Down()
{
	SelectMapBtn(MAP_WIZ_WIZ);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_WizWiz_OwnerDraw(int param)
{
	DrawMapBtn(MAP_WIZ_WIZ);
}

void FrTreasureCourse::OnBtnMap_West_Down()
{
	SelectMapBtn(MAP_WEST_WIZ);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_West_OwnerDraw(int param)
{
	DrawMapBtn(MAP_WEST_WIZ);
}

void FrTreasureCourse::OnBtnMap_Moon_Down()
{
	SelectMapBtn(MAP_BLUE_MOON);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Moon_OwnerDraw(int param)
{
	DrawMapBtn(MAP_BLUE_MOON);
}

void FrTreasureCourse::OnBtnMap_Silvia_Down()
{
	SelectMapBtn(MAP_SILVIA_CANNON);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Silvia_OwnerDraw(int param)
{
	DrawMapBtn(MAP_SILVIA_CANNON);
}

void FrTreasureCourse::OnBtnMap_Cannon_Down()
{
	SelectMapBtn(MAP_ICE_CANNON);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Cannon_OwnerDraw(int param)
{
	DrawMapBtn(MAP_ICE_CANNON);
}

void FrTreasureCourse::OnBtnMap_White_Down()
{
	SelectMapBtn(MAP_WHITE_WIZ);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_White_OwnerDraw(int param)
{
	DrawMapBtn(MAP_WHITE_WIZ);
}

void FrTreasureCourse::OnBtnMap_Shining_Down()
{
	SelectMapBtn(MAP_SHINING_SAND);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Shining_OwnerDraw(int param)
{
	DrawMapBtn(MAP_SHINING_SAND);
}

void FrTreasureCourse::OnBtnMap_Pink_Down()
{
	SelectMapBtn(MAP_PINK_WIND);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Pink_OwnerDraw(int param)
{
	DrawMapBtn(MAP_PINK_WIND);
}

void FrTreasureCourse::OnBtnMap_Inferno_Down()
{
	SelectMapBtn(MAP_DEEP_INFERNO);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Inferno_OwnerDraw(int param)
{
	DrawMapBtn(MAP_DEEP_INFERNO);
}

void FrTreasureCourse::OnBtnMap_IceSpa_Down()
{
	SelectMapBtn(MAP_ICE_SPA);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_IceSpa_OwnerDraw(int param)
{
	DrawMapBtn(MAP_ICE_SPA);
}

void FrTreasureCourse::OnBtnMap_SeaWay_Down()
{
	SelectMapBtn(MAP_LOST_SEAWAY);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_SeaWay_OwnerDraw(int param)
{
	DrawMapBtn(MAP_LOST_SEAWAY);
}

void FrTreasureCourse::OnBtnMap_Panda_Down()
{
	SelectMapBtn(MAP_EASTERN_VALLEY);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Panda_OwnerDraw(int param)
{
	DrawMapBtn(MAP_EASTERN_VALLEY);
}

void FrTreasureCourse::OnBtnMap_Wizcity_Down()
{
	SelectMapBtn(MAP_WIZ_CITY);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Wizcity_OwnerDraw(int param)
{
	DrawMapBtn(MAP_WIZ_CITY);
}

void FrTreasureCourse::OnBtnMap_IceInferno_Down()
{
	SelectMapBtn(MAP_MAP_18);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_IceInferno_OwnerDraw(int param)
{
	DrawMapBtn(MAP_MAP_18);
}

void FrTreasureCourse::OnBtnMap_AbbotMine_Down()
{
	SelectMapBtn(MAP_MAP_20);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_AbbotMine_OwnerDraw(int param)
{
	DrawMapBtn(MAP_MAP_20);
}

void FrTreasureCourse::OnBtnMap_Luna_Down()
{
	SelectMapBtn(MAP_MAP_21);
	Close(true);
}

void FrTreasureCourse::OnBtnMap_Luna_OwnerDraw(int param)
{
	DrawMapBtn(MAP_MAP_21);
}

void FrTreasureCourse::OnBtnMap_Random_Init(int param)
{
	m_pRandomBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTreasureCourse::OnBtnMap_Random_Down()
{
	SelectMapBtn(MAP_RANDOM);
	Close(true);
}

void FrTreasureCourse::OnViewerF1(int param)
{
	m_pViewerF1 = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_pViewerF1->SetMouseEvent(false);
	m_pViewerF1->Open("th_quick_frm1.tga");
}

void FrTreasureCourse::OnViewerF2(int param)
{
	m_pViewerF2 = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	m_pViewerF2->SetMouseEvent(false);
	m_pViewerF2->Open("th_quick_frm2.tga");
}

void FrTreasureCourse::OnImageBar(int param)
{
	m_pImageBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pImageBar)
	{
		m_pImageBar->SetRange(0, 300, 0);
		m_pImageBar->SetBarSpeed(200.0f);
		m_pImageBar->SetVisible(false);
	}
}

void FrTreasureCourse::OnGauge_SmallBar(int param)
{
	m_pSmallBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pSmallBar)
	{
		m_pSmallBar->SetRange(0, 300, 0);
		m_pSmallBar->SetBarSpeed(200.0f);
	}
}

// 983 (0083d4b0): sem SetPos/ResetGauge; 22 barras; a parte "cheia" (skin 1) e desenhada com src/dst proporcionais
void FrTreasureCourse::OnGauge_SmallBar_OwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;

	const Bitmap* pLow = m_pSmallBar->GetGaugeSkin(0);
	const Bitmap* pHigh = m_pSmallBar->GetGaugeSkin(1);
	if (!pLow || !pHigh)
		return;

	for (int i = 0; i < 22; i++)
	{
		if (i == 12 || i == 17 || m_gaugeIndex == i)
			continue;

		unsigned long calc = nsTHunter::CalcGauge(m_gauge[i]);
		float ratio = calc ? (float)((double)calc / 300.0) : 0.0f;

		float w = (float)(unsigned int)pHigh->bi->bmiHeader.biWidth;
		float h = (float)(unsigned int)pHigh->bi->bmiHeader.biHeight;
		WRect src(w * ratio, 0.0f, w - w * ratio, h);
		WRect dst(m_gaugePos[i].x + w * ratio, m_gaugePos[i].y, w - w * ratio, h);
		WRect rect(m_gaugePos[i].x, m_gaugePos[i].y,
			(float)(unsigned int)pLow->bi->bmiHeader.biWidth,
			(float)(unsigned int)pLow->bi->bmiHeader.biHeight);
		pGI->DrawTexture(pLow, rect, 0xffffffff, 0);
		pGI->DrawTexture(pHigh, src, dst, 0xffffffff, 0);
	}
}

void FrTreasureCourse::OnViewer_CurrMap_Init(int param)
{
	m_pCurrMapViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrTreasureCourse::OnArea_Explane_Init(int param)
{
	m_pExplaneArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTreasureCourse::OnArea_Explane_OwnerDraw(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (pGI && m_pExplaneArea && m_pGaugeTipBmp)
	{
		// 983 reuses the area rect as the draw rect.
		WRect rect = m_pExplaneArea->GetRect();
		if (rect.IsInRect(g_input->GetMousePoint().x, g_input->GetMousePoint().y))
		{
			unsigned int h = m_pGaugeTipBmp->bi->bmiHeader.biHeight;
			unsigned int w = m_pGaugeTipBmp->bi->bmiHeader.biWidth;
			rect.x = 610.0f;
			rect.y = 100.0f;
			rect.w = (float)w;
			rect.h = (float)h;
			pGI->DrawTexture(m_pGaugeTipBmp, rect, 0xffffffff, 0);
		}
	}
}

void FrTreasureCourse::OnInit_Cloud_Area(int param)
{
	m_pCloudArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTreasureCourse::OnOwnerDraw_Cloud_Area(int param)
{
	if (m_pCloudOverlay)
	{
		for (int i = 0; i < m_cloudNum; i++)
		{
			WRect uv(0.0f, 0.0f, 1.0f, 1.0f);
			m_pCloudOverlay->Render(g_view, uv, m_clouds[i].rect, 0,
				(m_clouds[i].alpha << 24) | 0xffffff, 0.0f, 0);
		}
	}
}

void FrTreasureCourse::OpenSelectedMap(unsigned char map)
{
}

void FrTreasureCourse::DrawMapBtn(eMapType map)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;

	WRect rect = m_mapBtns[map]->GetRect();
	float x = rect.w * 0.5f - (float)(m_pMapLockBmp->Width() >> 1);
	float y = rect.h * 0.5f - (float)(m_pMapLockBmp->Height() >> 1);

	if (Doc()->m_curChannel.Type & 0x800)
	{
		x = (float)floor(x);

		unsigned int tid;
		if (map <= 0x7f || map == 0xfd)
			tid = map;
		else
			tid = map - 0x80;

		IFF_STRUCT::sCourse* course =
			ItemManager()->FindCourse(tid | 0x28000000);
		if (!course)
			return;

		if (course->Difficulty > 3)
		{
			pGI->DrawTexture(m_pMapLockBmp,
				WRect(rect.x + x, rect.y + y, (float)m_pMapLockBmp->Width(),
					(float)m_pMapLockBmp->Height()),
				0xffffffff, 0);
		}
		else
		{
			switch (map)
			{
			case MAP_BLUE_WATER:
			case MAP_BLUE_MOON:
			case MAP_ICE_CANNON:
			case MAP_WHITE_WIZ:
			case MAP_ICE_SPA:
				pGI->DrawTexture(m_pMapLockBmp,
					WRect(rect.x + x, rect.y + y, (float)m_pMapLockBmp->Width(),
						(float)m_pMapLockBmp->Height()),
					0xffffffff, 0);
				break;
			}
		}
	}

	if (m_mapBtns[map]->GetStatus() == 1)
	{
		WPoint gaugePos(m_gaugePos[map].x - 15.0f, m_gaugePos[map].y);
		OpenGauge(gaugePos, CMapTypeAdapter::GetMapIndex(map));
	}
}

void FrTreasureCourse::DrawCurrentMap(FrGraphicInterface* pGI, int map)
{
	float y = m_rect.y;
	float x = m_rect.w - 5.0f;
	if (map != 0x7f)
	{
		pGI->DrawTexture(m_pMapBmp[map],
			WRect(x, y + 47.0f, (float)m_pMapBmp[map]->Width(),
				(float)m_pMapBmp[map]->Height()),
			0xffffffff, 0);
		return;
	}
	pGI->DrawTexture(m_pRandomSmallBmp,
		WRect(x, y + 47.0f, (float)m_pRandomSmallBmp->Width(),
			(float)m_pRandomSmallBmp->Height()),
		0xffffffff, 0);
}

void FrTreasureCourse::SetMapGauge()
{
	for (int i = 0; i < 22; i++)
	{
		unsigned long gauge =
			CTHunter::Instance()->GetTHunterGauge((eMapType)i);
		if (gauge != 9999)
			m_gauge[i] = gauge;
	}
}

bool FrTreasureCourse::OpenGauge(WPoint& pos, int index)
{
	if (index != -1)
	{
		if (m_gaugeIndex == index)
			return false;
		m_gaugeIndex = index;
		m_pImageBar->MoveWindow(pos);
		m_pImageBar->ResetGauge();
		m_pImageBar->SetRange(0, 300, 0);
		m_pImageBar->SetDestPos(nsTHunter::CalcGauge(m_gauge[index]), false);
		m_pImageBar->SetVisible(true);
		return true;
	}
	return false;
}

void FrTreasureCourse::OpenCurrentMap(int map)
{
	if (m_gaugeIndex != map)
	{
		unsigned int tid;
		if (map <= 0x7f || map == 0xfd)
			tid = map;
		else
			tid = map - 0x80;

		IFF_STRUCT::sCourse* course =
			ItemManager()->FindCourse(tid | 0x28000000);
		if (course)
		{
			if (map <= 0x7e)
				m_pCurrMapViewer->Open(MakeStr("mid_%s.tga", course->c.Icon));
			else
				m_pCurrMapViewer->Open("map_random_small.tga");
		}
	}
}

void FrTreasureCourse::SelectMapBtn(unsigned char map)
{
	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(
		((map > 0x7f && map != 0xfd) ? map - 0x80 : map) | 0x28000000);
	if (!course)
		return;

	if (m_curMapIdx == map)
		m_bMapIndexChanged = false;
	else
		m_bMapIndexChanged = true;

	if (m_bMapIndexChanged)
		m_pCurrMapViewer->Open(MakeStr("mid_%s", course->c.Icon));

	// 983 (0083f980): o estado da sala (CSharedDoc+0x11638, 0062bf90) decide; o nome do layout so vale p/ AVATARCHAT_MAIN
	int state = DOCJP()->GetState983();
	bool bSend = false;
	if (m_bMapIndexChanged)
	{
		switch (state)
		{
		case 3:
		case 4:
		case 5:
		case 7:
			bSend = true;
			break;
		case 8:
			if (DOCJP()->GetRoomListKind983() == 0)
				bSend = true;
			break;
		}
	}

	if (bSend || state == 6 ||
		(state == 8 && DOCJP()->GetRoomListKind983() == 1) ||
		!stricmp(g_pFresh->GetManager()->GetLayoutID(), "AVATARCHAT_MAIN"))
	{
		WSendPacket packet((enumClientPacket)10);
		packet.Encode2(0xffff);
		packet.Encode1(1);
		packet.Encode1(3);
		packet.Encode1((unsigned char)course->c.TypeId);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));

		m_curMapIdx = CMapTypeAdapter::GetMapIndex(
			(eMapType)(course->c.TypeId & 0x3ffffff));
	}
	else
	{
		m_curMapIdx = CMapTypeAdapter::GetMapIndex(
			(eMapType)(course->c.TypeId & 0x3ffffff));
	}
}

void FrTreasureCourse::SetSelectMap(unsigned char map)
{
	m_curMapIdx = CMapTypeAdapter::GetMapIndex((eMapType)map);
	OpenCurrentMap(map);
}

void FrTreasureCourse::SetCurMapIdx(unsigned char idx)
{
	m_curMapIdx = idx;
}

unsigned char FrTreasureCourse::GetCurMapIdx()
{
	return m_curMapIdx;
}

bool FrTreasureCourse::GetMapIndexChanged() const
{
	return m_bMapIndexChanged;
}

void FrTreasureCourse::SetMapIndexChanged(bool bChanged)
{
	m_bMapIndexChanged = bChanged;
}

IMPLEMENT_OBJECT(FrTreasureScore, FrForm)

BEGIN_FRESH_MSGMAP(FrTreasureScore, FrForm)
ON_FRESH_VI("imagebar", FRCMD_INIT, FrTreasureScore::OnGBarImage_ImageBar_Init)
ON_FRESH_VI("back", FRCMD_INIT, FrTreasureScore::OnView_Backgound_Init)
ON_FRESH_VI("team", FRCMD_INIT, FrTreasureScore::OnArea_Team_Init)
END_FRESH_MSGMAP()

FrTreasureScore::FrTreasureScore()
{
	m_pImageBar = NULL;
	m_pBackground = NULL;
	m_pTeamArea = NULL;
	m_pTeamBlueBmp = NULL;
	m_pTeamRedBmp = NULL;
}

FrTreasureScore::~FrTreasureScore()
{
}

bool FrTreasureScore::OnInit()
{
	char buf[128];
	MoveWindow(GetCalcPoint());
	IFF_STRUCT::sCourse* pCourse = Doc()->m_golfGame.pCourse;
	sprintf(buf, "th_result_%s.tga", pCourse ? pCourse->Data : NULL);
	if (m_pBackground)
		m_pBackground->Open(buf);
	m_pTeamBlueBmp = g_pFresh->RegisterBitmap("th_team_blue");
	m_pTeamRedBmp = g_pFresh->RegisterBitmap("th_team_red");
	IsTeamMode();
	SetTPoint();
	return true;
}

void FrTreasureScore::SetTPoint()
{
	unsigned long point = CTHunter::Instance()->GetTreasurePoint();
	if (m_pImageBar)
		m_pImageBar->SetDestPos(point, false);
}

void FrTreasureScore::SetState(bool bShow)
{
	if (m_bShow != bShow)
	{
		m_bShow = bShow;
		if (!bShow)
		{
			SetVisible(false);
			return;
		}
		MoveWindow(GetCalcPoint());
		SetVisible(true);
	}
}

WPoint FrTreasureScore::GetCalcPoint()
{
	WRect rect = m_rect;
	WPoint pos;
	int x = (int)((g_view->GetWidth() - rect.w) * 0.5f);
	float y = (float)(int)((g_view->GetHeight() - rect.h) * 0.5f) - 20.0f;
	pos.x = (float)x;
	pos.y = y;
	return pos;
}

void FrTreasureScore::IsTeamMode()
{
	// HACK: emission order (the original waits one more wave, probably on an inline
	// accessor that this tree does not have)
	if (0)
		((FrTreasureGift*)0)->FrTreasureGift::OnOwnerDraw_Box_Area(0);
	if (Doc()->m_golfGame.gameType != 1)
	{
		m_pTeamArea->SetVisible(false);
		return;
	}
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		unsigned long guid = Doc()->m_myInfo.info.dwGuid;
		if (guid == GOLFDOC()->GetPlayer(i)->oid)
		{
			const Bitmap* pBmp = GOLFDOC()->GetPlayer(i)->team == 0
				? m_pTeamBlueBmp
				: m_pTeamRedBmp;
			if (pBmp)
				m_pTeamArea->SetBgImg(pBmp);
			return;
		}
	}
}

void FrTreasureScore::OnGBarImage_ImageBar_Init(int param)
{
	m_pImageBar = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pImageBar)
	{
		m_pImageBar->SetRange(0, 1000, 0);
		m_pImageBar->SetBarSpeed(600.0f);
	}
}

void FrTreasureScore::OnView_Backgound_Init(int param)
{
	m_pBackground = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
	if (m_pBackground)
		m_pBackground->SetMouseEvent(false);
}

void FrTreasureScore::OnArea_Team_Init(int param)
{
	m_pTeamArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

CTFrame::CTFrame()
	: m_frameCount(3)
{
	m_frames.reserve(m_frameCount);
}

CTFrame::CTFrame(unsigned char frameCount)
	: m_frameCount(frameCount)
{
	m_frames.reserve(frameCount);
}

CTFrame::~CTFrame()
{
	Reset();
}

void CTFrame::AddFrame(FrArea* pFrame)
{
	if (pFrame)
		m_frames.push_back(pFrame);
}

bool CTFrame::SetFrameWidth(float width)
{
	unsigned char start;
	unsigned char end;
	if (0.0f == width)
		return false;

	unsigned char count = (unsigned char)m_frames.size();
	if (count == 3)
	{
		start = 3;
		end = 6;
	}
	else if (count == 9)
	{
		start = 0;
		end = 3;
	}
	else
		return false;
	for (unsigned char i = start; i < end; i++)
	{
		// 983 copies the rect as raw dwords (no WRect copy ctor).
		WRect rect;
		memcpy(&rect, &m_frames[i]->GetRect(), sizeof(rect));
		rect.w += width;
		m_frames[i]->SetRect(rect);
	}
	return true;
}

bool CTFrame::SetFrameHeight(float height)
{
	if (0.0f == height || (unsigned char)m_frames.size() != 6)
		return false;

	for (unsigned char i = 0; i < 3; i++)
	{
		WRect rect;
		memcpy(&rect, &m_frames[i]->GetRect(), sizeof(rect));
		rect.h += height;
		m_frames[i]->SetRect(rect);
	}
	for (unsigned char i = 3; i < 6; i++)
	{
		WRect rect;
		memcpy(&rect, &m_frames[i]->GetRect(), sizeof(rect));
		rect.y += height;
		m_frames[i]->SetRect(rect);
	}
	return true;
}

IMPLEMENT_OBJECT(FrTreasureGift, FrForm)

BEGIN_FRESH_MSGMAP(FrTreasureGift, FrForm)
ON_FRESH_VI("outback3", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("outback4", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("outback5", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("outback6", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("outback7", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("outback8", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_Out)
ON_FRESH_VI("inback3", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("inback4", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("inback5", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("inback6", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("inback7", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("inback8", FRCMD_INIT, FrTreasureGift::OnInit_RangeArea_In)
ON_FRESH_VI("grid", FRCMD_INIT, FrTreasureGift::OnInit_GridArea)
ON_FRESH_VI("grid", FRCMD_OWNERDRAW, FrTreasureGift::OnOwnerDraw_GridArea)
ON_FRESH_VI("giftlist", FRCMD_INIT, FrTreasureGift::OnInit_Gift_List)
ON_FRESH_VI("giftlist", FRCMD_OWNERDRAW, FrTreasureGift::OnOwnerDraw_Gift_List)
ON_FRESH_VI("box", FRCMD_INIT, FrTreasureGift::OnInit_Box_Area)
ON_FRESH_VI("box", FRCMD_OWNERDRAW, FrTreasureGift::OnOwnerDraw_Box_Area)
END_FRESH_MSGMAP()

FrTreasureGift::FrTreasureGift()
	: m_colNum(8)
{
	m_rowNum = 1;
	m_bShow = true;
}

FrTreasureGift::~FrTreasureGift()
{
}

bool FrTreasureGift::OnInit()
{
	m_boxOpener.OnInit();
	MoveWindow(GetCalcPoint());
	CTHunter::Instance()->GetTreasurePoint();
	SetPointByFrameHeight();
	m_rowNum = (unsigned char)(m_boxOpener.m_slotNum / 8);
	return true;
}

void FrTreasureGift::SetPointByFrameHeight()
{
	float outHeight;
	float inHeight;
	int giftNum = m_boxOpener.m_giftNum;
	if (giftNum > 8 && giftNum < 17)
	{
		outHeight = 65.0f;
		inHeight = 64.0f;
	}
	else if (giftNum > 16 && giftNum < 25)
	{
		outHeight = 129.0f;
		inHeight = 128.0f;
	}
	else
	{
		return;
	}
	m_outRange.SetFrameHeight(outHeight);
	m_inRange.SetFrameHeight(inHeight);
}

WPoint FrTreasureGift::GetCalcPoint()
{
	WRect rect = m_rect;
	WPoint pos;
	int x = (int)((g_view->GetWidth() - rect.w) * 0.5f);
	float y = (float)(int)((g_view->GetHeight() - rect.h) * 0.5f) + 50.0f;
	pos.x = (float)x;
	pos.y = y;
	return pos;
}

void FrTreasureGift::SetState(bool bShow)
{
	if (m_bShow != bShow)
	{
		m_bShow = bShow;
		if (!bShow)
		{
			SetVisible(false);
			return;
		}
		MoveWindow(GetCalcPoint());
		SetVisible(true);
	}
}

void FrTreasureGift::OnOwnerDraw_Gift_List(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem)
		m_boxOpener.DrawGiftList(pItem);
}

void FrTreasureGift::OnProc(const float dt)
{
	m_boxOpener.OnProc(dt);
	FrForm::OnProc(dt);
}

void FrTreasureGift::OnInit_RangeArea_Out(int param)
{
	m_outRange.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrTreasureGift::OnInit_RangeArea_In(int param)
{
	m_inRange.AddFrame(DYNAMIC_CAST(FrArea, (FrWnd*)param));
}

void FrTreasureGift::OnInit_Gift_List(int param)
{
	m_boxOpener.m_pGiftList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrTreasureGift::OnInit_GridArea(int param)
{
	m_pGridArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrTreasureGift::OnOwnerDraw_GridArea(int param)
{
	static unsigned long color = 0xffa2d7fd;
	WRect rect = m_boxOpener.m_pGiftList->GetRect();
	float x = rect.x + 25.0f;
	float w = rect.w - 65.0f;
	WOverlay::DrawBox(g_view, WRect(x, rect.y + 30.0f, w, 1.0f), 0, color,
		0.0f);
	if (m_rowNum == 2)
		WOverlay::DrawBox(g_view, WRect(x, rect.y + 98.0f, w, 1.0f), 0, color,
			0.0f);
	else if (m_rowNum == 3)
		WOverlay::DrawBox(g_view, WRect(x, rect.y + 166.0f, w, 1.0f), 0, color,
			0.0f);
	for (int i = 0; i < 8; i++)
	{
		WOverlay::DrawBox(g_view,
			WRect(rect.x + i * 65.0f + 33.0f, rect.y + 35.0f, 1.0f,
				rect.h - 60.0f),
			0, color, 0.0f);
	}
}

void FrTreasureGift::OnInit_Box_Area(int param)
{
	m_boxOpener.m_pBoxArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_boxOpener.m_pBoxArea)
	{
		WRect rect = m_boxOpener.m_pBoxArea->GetRect();
		rect.y = m_rect.y - 10.0f;
		m_boxOpener.m_pBoxArea->SetRect(rect);
	}
}

void FrTreasureGift::OnOwnerDraw_Box_Area(int param)
{
	m_boxOpener.DrawBoxArea();
}

IMPLEMENT_OBJECT(FrTestDlg_combo, FrForm)

BEGIN_FRESH_MSGMAP(FrTestDlg_combo, FrForm)
END_FRESH_MSGMAP()

FrTestDlg_combo::FrTestDlg_combo()
{
	m_pCombo = NULL;
}

FrTestDlg_combo::~FrTestDlg_combo()
{
}

bool FrTestDlg_combo::OnInit()
{
	return true;
}

CBoxOpener::CBoxOpener()
{
	m_boxType = 0;
	m_progress = 0;
	m_giftNum = 0;
	m_slotNum = 0;
	m_pGiftList = NULL;
	m_reserveSeq = eSEQ_NONE;
	m_nextSeq = eSEQ_NONE;
	m_curSeq = eSEQ_NONE;
	m_bApproach = false;
	m_delay = 2000;

	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	char* filename = "[font_wind.jpg";
	info.filename = &filename;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

CBoxOpener::~CBoxOpener()
{
	if (m_pOpenAni)
	{
		delete m_pOpenAni;
		m_pOpenAni = NULL;
	}
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
	m_bApproach = false;
}

bool CBoxOpener::OnInit()
{
	if (m_bApproach == true)
		CTHunter::Instance()->ReAdjustRepository();

	m_giftNum = CTHunter::Instance()->GetRepositorySize();
	if (m_giftNum < 9)
		m_slotNum = 8;
	else if (m_giftNum > 8 && m_giftNum < 17)
		m_slotNum = 16;
	else if (m_giftNum > 16 && m_giftNum < 25)
		m_slotNum = 24;
	else
		m_slotNum = 8;

	m_gifts.reserve(m_slotNum);

	sTGift gift;
	for (unsigned char i = 0; i < m_slotNum; i++)
	{
		if (i < m_giftNum)
		{
			sTreasureHunt info;
			info = CTHunter::Instance()->GetGiftInfo(i);
			gift.index = i;
			gift.quantity = info.wCount;
			gift.typeId = info.dwTid;
			gift.bMine = info.dwUID == Doc()->m_myInfo.info.dwUID;
		}
		else
		{
			gift.Reset();
		}
		m_gifts.push_back(gift);
		m_pGiftList->AddItem(&m_gifts[i]);
	}

	WRect rect = m_pGiftList->GetRect();
	rect.h = (float)(m_slotNum / 8) * 68.0f;
	m_pGiftList->SetRect(rect);

	m_pBoxCloseBmp = g_pFresh->RegisterBitmap("th_item_box_n");
	m_pBoxOpenBmp = g_pFresh->RegisterBitmap("th_item_box_o");
	m_pMark1Bmp = g_pFresh->RegisterBitmap("th_item_mark1");
	m_pMark2Bmp = g_pFresh->RegisterBitmap("th_item_mark2");
	m_pMsgBmp = g_pFresh->RegisterBitmap("th_msg1");
	m_pBoxBmp[0] = g_pFresh->RegisterBitmap("th_box_frm");
	for (int i = 1; i < 5; i++)
		m_pBoxBmp[i] = g_pFresh->RegisterBitmap(MakeStr("th_box_%d", i));

	m_pOpenAni = new CSimpleUI("[emoti_object.png");
	m_pOpenAni->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
	m_pOpenAni->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);

	m_boxType = DecideTRBox();
	if (m_boxType > 5)
		m_boxType = 1;

	m_lastTime = timeGetTime();
	m_pfnProc = &CBoxOpener::ProcInit;
	CTHunter::Instance()->SetReturnLobby(false);
	m_nextSeq = eSEQ_NONE;
	m_curSeq = eSEQ_INIT;
	return true;
}

void CBoxOpener::OnProc(const float dt)
{
	m_dt = dt;
	if (m_nextSeq != eSEQ_NONE)
		ChangeSequence(m_nextSeq);
	if (m_pfnProc)
		(this->*m_pfnProc)();
}

void CBoxOpener::ChangeSequence(eSeqProc seq)
{
	switch (seq)
	{
	case eSEQ_INIT:
		m_lastTime = timeGetTime();
		m_pfnProc = &CBoxOpener::ProcInit;
		CTHunter::Instance()->SetReturnLobby(false);
		break;
	case eSEQ_CREATE:
		m_lastTime = timeGetTime();
		m_delay = 250;
		m_pfnProc = &CBoxOpener::ProcCreate;
		m_reserveSeq = eSEQ_MYITEM_OPEN;
		break;
	case eSEQ_MYITEM_OPEN:
		m_lastTime = timeGetTime();
		m_pfnProc = &CBoxOpener::ProcMyItemOpen;
		m_reserveSeq = eSEQ_COMPLETE;
		for (unsigned char i = 0; i < m_giftNum; i++)
			GetGiftItem(i);
		m_progress = 0;
		m_delay = 200;
		break;
	case eSEQ_COMPLETE:
		m_lastTime = timeGetTime();
		m_pfnProc = &CBoxOpener::ProcComplete;
		m_delay = 1500;
		break;
	case eSEQ_BOX_NONE:
		break;
	case eSEQ_END:
		m_lastTime = timeGetTime();
		m_pfnProc = &CBoxOpener::ProcEnd;
		m_delay = 3000;
		break;
	case eSEQ_FINISH:
		CTHunter::Instance()->SetReturnLobby(true);
		break;
	}
	m_curSeq = seq;
	m_nextSeq = eSEQ_NONE;
}

void CBoxOpener::ProcInit()
{
	if (timeGetTime() - m_lastTime > 1000)
	{
		for (unsigned char i = 0; i < m_pGiftList->GetCurrentItemSize(true);
			i++)
			GetGiftItem(i)->state = 1;
		m_nextSeq = eSEQ_CREATE;
	}
}

void CBoxOpener::ProcCreate()
{
	if (timeGetTime() - m_lastTime <= m_delay)
		return;

	if (m_progress < m_giftNum)
	{
		sTGift* pGift = GetGiftItem(m_progress++);
		if (pGift && pGift->typeId)
		{
			pGift->bItem = true;
			pGift->state = 3;
			g_audio->PlaySfx("play_fail_item", NULL, 0, false, 0, 0.5f, 200.0f);
		}
	}
	else if (m_progress < m_slotNum)
	{
		sTGift* pGift = GetGiftItem(m_progress++);
		pGift->state = 2;
	}
	else
	{
		m_progress++;
	}
	m_lastTime = timeGetTime();
	if (m_progress == m_giftNum)
		m_delay = 50;
	if (m_progress == m_slotNum)
		m_nextSeq = eSEQ_MYITEM_OPEN;
}

void CBoxOpener::ProcMyItemOpen()
{
	if (m_progress < m_giftNum && timeGetTime() - m_lastTime > 250)
	{
		if (m_progress < m_giftNum)
		{
			sTGift* pGift = GetGiftItem(m_progress++);
			if (pGift && pGift->typeId)
			{
				pGift->state = 4;
				pGift->frame = 0.0f;
				g_audio->PlaySfx("play_fail_item", NULL, 0, false, 0, 0.5f,
					200.0f);
			}
			else
			{
				m_progress++;
			}
			m_lastTime = timeGetTime();
		}
	}
	if (m_progress == m_giftNum && timeGetTime() - m_lastTime > m_delay)
		m_nextSeq = eSEQ_COMPLETE;
}

void CBoxOpener::ProcComplete()
{
	if (timeGetTime() - m_lastTime > m_delay && m_curSeq == eSEQ_COMPLETE)
		m_nextSeq = eSEQ_FINISH;
}

void CBoxOpener::ProcBoxNone()
{
}

void CBoxOpener::ProcEnd()
{
	if (timeGetTime() - m_lastTime > m_delay && m_curSeq == eSEQ_END)
		m_nextSeq = eSEQ_FINISH;
}

CBoxOpener::sTGift* CBoxOpener::GetGiftItem(int index)
{
	FrListItem* pItem = m_pGiftList->GetItem(index);
	if (!pItem)
		return NULL;
	return (sTGift*)pItem->pData;
}

void CBoxOpener::DrawGiftList(FrListItem* pItem)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	sTGift* pGift = (sTGift*)pItem->pData;
	if (!pGI)
		return;

	WPoint offset = m_pGiftList->GetRect().TopLeft();

	const Bitmap* pBoxBmp;
	if (m_curSeq >= eSEQ_COMPLETE && pGift->bMine)
		pBoxBmp = m_pBoxOpenBmp;
	else
		pBoxBmp = m_pBoxCloseBmp;
	pGI->DrawTexture(pBoxBmp,
		WRect(pItem->pos.x, pItem->pos.y - 5.0f, (float)pBoxBmp->Width(),
			(float)pBoxBmp->Height()),
		0xffffffff, 0);

	unsigned long color = 0xffffffff;
	const Bitmap* pBmp = NULL;
	GetItemOffset(offset, pGift->typeId, true);

	switch (pGift->state)
	{
	case 0:
		return;
	case 1:
		pBmp = m_pMark1Bmp;
		offset.x = 27.0f;
		offset.y = 18.0f;
		break;
	case 2:
		pBmp = m_pMark2Bmp;
		offset.x = 27.0f;
		offset.y = 28.0f;
		break;
	case 3:
		if (pGift->bItem == true)
		{
			IFF_ITEM_COMMON* pCommon =
				ItemManager()->FindCommonItem(pGift->typeId);
			if (pCommon)
			{
				if (pGift->typeId != 0x1a000010)
					pBmp = g_pFresh->GetManager()->GetBitmap("ITEMS",
						pCommon->Icon);
				else
					pBmp =
						g_pFresh->GetManager()->GetBitmap("ITEMS", "Item_Pang");
				color = 0xa0202020;
				offset.x += 13.0f;
				offset.y += 7.0f;
			}
		}
		break;
	case 4:
		color = pGift->bMine == true ? 0xffffffff : 0x20202020;
		if (pGift->bItem == true)
		{
			IFF_ITEM_COMMON* pCommon =
				ItemManager()->FindCommonItem(pGift->typeId);
			if (pCommon)
			{
				if (pGift->typeId != 0x1a000010)
					pBmp = g_pFresh->GetManager()->GetBitmap("ITEMS",
						pCommon->Icon);
				else
					pBmp =
						g_pFresh->GetManager()->GetBitmap("ITEMS", "Item_Pang");
				offset.x += 13.0f;
				offset.y += 7.0f;
			}
		}
		break;
	}

	if (pBmp)
	{
		pGI->DrawTexture(pBmp,
			WRect(offset.x + pItem->pos.x, offset.y + pItem->pos.y,
				(float)pBmp->Width(), (float)pBmp->Height()),
			color, 0);
		if (m_pFont && pGift->state == 4 && pGift->quantity)
		{
			WPoint numPos = pItem->pos;
			GetItemOffset(numPos, pGift->typeId, false);
			char buf[16] = { 0 };
			itoa(pGift->quantity, buf, 10);
			if (pGift->typeId == 0x1a000010)
				numPos.x -= 15.0f;
			m_pFont->Print(g_view, numPos.x + pItem->pos.x + offset.x,
				numPos.y + offset.y + pItem->pos.y, buf, 0, 0xffffffff, NULL);
		}
	}

	if (pGift->state >= 3 && pGift->frame >= 0.0f)
	{
		if ((int)pGift->frame < 8 && m_pOpenAni)
		{
			m_pOpenAni->Render(MakeStr("%d", (int)pGift->frame),
				pItem->pos.x - 10.0f, pItem->pos.y - 25.0f, 1.5f, 0);
			pGift->frame += m_dt * 15.0f;
		}
	}
}

void CBoxOpener::DrawBoxArea()
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;

	WRect rect = m_pBoxArea->GetRect();
	if (m_pBoxBmp[0])
	{
		pGI->DrawTexture(m_pBoxBmp[0],
			WRect(rect.x, rect.y, (float)m_pBoxBmp[0]->Width(),
				(float)m_pBoxBmp[0]->Height()),
			0xffffffff, 0);
	}
	if (m_pBoxBmp[m_boxType])
	{
		pGI->DrawTexture(m_pBoxBmp[m_boxType],
			WRect(rect.x + 20.0f, rect.y + 22.0f,
				(float)m_pBoxBmp[m_boxType]->Width(),
				(float)m_pBoxBmp[m_boxType]->Height()),
			0xffffffff, 0);
	}
	if (m_bApproach == true && CTHunter::Instance()->GetGiftSize() > 0)
	{
		pGI->SetTextColor(0xffff0000, 0xffffffff);
		pGI->Print(WPoint(rect.x + 230.0f, rect.y + 134.0f), 0,
			K2L_Compatibility("\xb0\xb0\xc0\xba\xc1\xbe\xb7\xf9\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xba \xc7\xd1 \xbd\xbd\xb7\xd4\xbf\xa1 \xc7\xd5\xbb\xea\xc7\xa5\xb1\xe2 \xb5\xcb\xb4\xcf\xb4\xd9."));
		pGI->SetTextColor(0xffffffff, 0xffffffff);
	}
}

void CBoxOpener::GetItemOffset(WPoint& pos, unsigned long typeId, bool bIcon)
{
	if (bIcon == true)
	{
		switch (typeId >> 26)
		{
		case 5:
		case 6:
			if (ItemManager()->GetCouponKind(typeId) == 0)
				pos = WPoint(0.0f, 0.0f);
			else
				pos = WPoint(0.0f, 0.0f);
			break;
		case 0x1c:
			pos = WPoint(-10.0f, -20.0f);
			break;
		case 0x1f:
			pos = WPoint(0.0f, -10.0f);
			break;
		}
	}
	else
	{
		switch (typeId >> 26)
		{
		case 5:
		case 6:
			if (ItemManager()->GetCouponKind(typeId) == 0)
				pos = WPoint(40.0f, 33.0f);
			else
				pos = WPoint(40.0f, 33.0f);
			break;
		case 0x1c:
			pos = WPoint(48.0f, 50.0f);
			break;
		case 0x1f:
			pos = WPoint(45.0f, 43.0f);
			break;
		}
	}
}

unsigned char CBoxOpener::DecideTRBox()
{
	unsigned char count[4];
	unsigned char maps[15];
	count[0] = 3;
	count[1] = 4;
	count[2] = 3;
	count[3] = 5;
	maps[0] = 2;
	maps[1] = 3;
	maps[2] = 11;
	maps[3] = 0;
	maps[4] = 1;
	maps[5] = 6;
	maps[6] = 7;
	maps[7] = 10;
	maps[8] = 13;
	maps[9] = 15;
	maps[10] = 4;
	maps[11] = 5;
	maps[12] = 8;
	maps[13] = 9;
	maps[14] = 14;

	unsigned char map = Doc()->m_roomInfo.mapType;
	unsigned char sum = 0;
	if (map >= 0x80 && map != 0xfd)
		map -= 0x80;

	unsigned char k = 0;
	for (unsigned char i = 0; i < 4; i++)
	{
		sum += count[i];
		for (; k < sum; k++)
		{
			if (maps[k] == map)
				return i + 1;
		}
	}
	return 1;
}
