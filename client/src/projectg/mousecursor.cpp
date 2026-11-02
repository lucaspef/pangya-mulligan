#include "minatl.h"
#include "mousecursor.h"
#include "club.h"
#include "quitwindow.h"
#include "simpleoverlay.h"
#include "quadtree.h"
#include "messengerdlg.h"
#include "frgroupbox.h"
#include "frdesktop.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "../shared/localize.h"
#include "jp/uiframesetmanager.h"

extern WMatrix g_camera;
extern Fresh* g_pFresh;
bool IsMyTurn(bool check);
int float2int(float value);
#if _MSC_VER <= 1600 // VC10: floorf do CRT em __fastcall (codegen do 983); toolset novo usa o da UCRT (math.h)
extern "C" float __fastcall floorf(float value);
#endif

inline bool WRect::IsInRect(float x, float y)
{
	if (x >= this->x && this->x + w >= x && y >= this->y && this->y + h >= y)
		return true;
	return false;
}

class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

class CGroundPropt
{
public:
	CGroundPropt(const char* name);
	~CGroundPropt();
	void DrawGroundTip();
	void SelectGroundTip();
	void SetGroundType(const char* type) { strcpy(m_groundtype, type); }
	CSimpleUI* GetGroundTip() { return m_groundtip; }

private:
	char m_groundtype[16];
	CSimpleUI* m_groundtip;
};

#include "clientsetting.h"
#include "scoreboard.h"
#include "jp/docofs983.h" // D864: crus do CSharedDoc ancorados no membro (VS 2022)

extern WInputDev* g_mouse; // 983 00f02450 (vtbl+0x10 = InitDevice)
extern HWND__* g_hWnd983; // 00f0245c
CMouseCursorHelper* g_pMouseCursorHelper983; // 00e7ba5c
extern bool g_bFocus; // 983 00e33794 (WM_SETFOCUS/WM_KILLFOCUS/Paint)
bool IsCursorInClient983(); // 00890ea0
BOOL GetPhysicalCursorPos983(LPPOINT pt); // inputmanager.cpp
void CMouseCursorHelper::Restore()
{
	m_state = 0;
	SetVisible(true);
}

void CMouseCursorHelper::SetState(int state)
{
	m_state = state;
	if (state == 1)
	{
		SetVisible(false);
		return;
	}
	SetVisible(true);
	if (state == 2)
		SetCursor(m_hCursor);
}

// 983: mostrar = contador 0 com m_hCursor; esconder = contador -1
void CMouseCursorHelper::SetVisible(bool show)
{
	if (show)
	{
		SetCursor(m_hCursor);
		m_showCount = ShowCursor(TRUE);
		if (m_showCount > 0)
		{
			do
				m_showCount = ShowCursor(FALSE);
			while (m_showCount != 0);
		}
		else if (m_showCount < 0)
			ShowCountFix(1);
		return;
	}
	m_showCount = ShowCursor(FALSE);
	if (m_showCount >= 0)
	{
		do
			m_showCount = ShowCursor(FALSE);
		while (m_showCount != -1);
	}
	else if (m_showCount < -1)
		HideCountFix(1);
}

void CMouseCursorHelper::ShowCountFix(int a)
{
	while (m_showCount != 0)
		m_showCount = ShowCursor(a);
}

// 983 007e2010 (A312: era stub no exe; chamado pela janela de opcoes): contador do ShowCursor ate -1
void CMouseCursorHelper::HideCountFix(int a)
{
	while (m_showCount != -1)
		m_showCount = ShowCursor(a);
}

// 983 00890ea0: cursor do sistema dentro da area cliente (bordas inclusas)
bool IsCursorInClient983()
{
	RECT rc;
	POINT pt;
	GetClientRect(g_hWnd983, &rc);
	GetPhysicalCursorPos983(&pt);
	ScreenToClient(g_hWnd983, &pt);
	return rc.left <= pt.x && pt.x <= rc.right && rc.top <= pt.y && pt.y <= rc.bottom;
}

// HACK: 89.0f is defined in tutorialmain.cpp as a function-local static aliased to
// __real@42b20000; using the literal here would emit a competing COMDAT that all the
// other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_42b20000 = 89.0f; // C64.4: o estático de função do rebang não existe mais

CMouseCursor::CMouseCursor()
	: m_overlay(NULL), m_bound(NULL)
{
	m_pTextTip = NULL;
	m_overlay = new sPointer[MOUSE_MAX_NUM];
	const char* names[MOUSE_MAX_NUM] = { "1", "2", "3", "4", "_up", "_down",
		"_zoom", "_both", "_move", "_move_no", "_rotate", "_rotate_no",
		"_caret", "_context", "_rotate2" }; // 983 (00893bd0): 15 cursores
	for (int i = 0; i < MOUSE_MAX_NUM; ++i)
	{
		std::string name;
		name = "[s_pointer";
		name += names[i];
		name += ".png";
		m_overlay[i].overlay = g_resrcmng->GetOverlay(name.c_str(), 0);
		m_overlay[i].w = (float)m_overlay[i].overlay->GetWidth();
		m_overlay[i].h = (float)m_overlay[i].overlay->GetHeight();
		m_overlay[i].len = 32.0f;
		m_overlay[i].num = (int)(m_overlay[i].w / 32.0f);
	}
	m_frequency = 0.35f;
	m_frame = 0;
	m_holecup.x = 0;
	m_holecup.y = 0;
	m_autoHide = false;
	m_bShow = true;
	if (g_view)
	{
		m_pointer.x = g_view->GetWidth();
		m_pointer.y = g_view->GetHeight();
	}
	else
	{
		m_pointer.x = 800;
		m_pointer.y = 600;
	}
	SetBoundArea(NULL);
	Reset(false);
	m_mode = SCREEN_LOBBY;
	m_pointer.x = (g_view ? g_view->GetWidth() : 800.0f) * 0.625;
	m_pointer.y = (g_view ? g_view->GetHeight() : 600.0f) * 0.5;
	m_pGroundPt = new CGroundPropt("[ground_2.jpg");
	m_pGroundPt->GetGroundTip()->Add("Fairway", WRect(0, 0, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Rough", WRect(0, 25, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Green", WRect(0, 50, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Bunker", WRect(0, 75, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("OB", WRect(0, 100, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Ice", WRect(0, 125, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Sand", WRect(0, 150, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("Water", WRect(0, 175, 90, 25), false);
	m_pGroundPt->GetGroundTip()->Add("WaterHazard", WRect(0, 200, 90, 25),
		false);
	m_pGroundPt->GetGroundTip()->Add("Vector", WRect(0, 225, 90, 25), false);
	m_pGroundPt->SetGroundType("");
	m_bTerrainTooltip = COption::Instance()->gGetTerrainTooltip() != 0;
	if (BindMouseCursor983() != 1 && COption::Instance()->vIsWindowed() &&
		BindMouseCursor983() == 0 && g_pMouseCursorHelper983 && g_mouse)
		g_mouse->InitDevice(g_hWnd983, false);
	m_bLocked983 = false;
	m_cursorOwner983 = 0;
}

CMouseCursor::~CMouseCursor()
{
	for (int i = 0; i < MOUSE_MAX_NUM; ++i)
	{
		if (m_overlay[i].overlay)
		{
			g_resrcmng->Release(m_overlay[i].overlay);
			m_overlay[i].overlay = NULL;
		}
	}
	if (m_overlay)
	{
		delete[] m_overlay;
		m_overlay = NULL;
	}
	if (m_bound)
	{
		delete m_bound;
		m_bound = NULL;
	}
	if (m_pGroundPt)
	{
		delete m_pGroundPt;
		m_pGroundPt = NULL;
	}
}

void CMouseCursor::Reset(bool active)
{
	m_oldPointer = m_pointer;
	m_stopDuration = 0;
	g_input->SetMousePoint(m_pointer.x, m_pointer.y);
	m_active = active;
	m_tempActive = false;
	m_show = true;
	m_ignoreButtonDownArea = false;
	m_dt = 0;
	m_td4Tooltip = 0;
	m_area = AREA_NORMAL;
	m_cursor = UNSELECT;
	m_camMode = false;
	m_forceMoving = 0;
	m_layer = 0;
	m_itemWindow = false;
	m_bRightButton = false;
	m_activeArea.clear();
	for (int i = 0; i < 3; ++i)
		m_buttonDownArea[i] = AREA_NORMAL;
	memset(m_bArea, true, sizeof(m_bArea));
}

void CMouseCursor::Process(float dt)
{
	// 983: o cursor do sistema acompanha a janela (modo janela, nao preso)
	if (BindMouseCursor983() != 1 && COption::Instance()->vIsWindowed() &&
		BindMouseCursor983() == 0 && g_pMouseCursorHelper983)
	{
		if (!g_bFocus ||
			(g_pMouseCursorHelper983->m_state != 1 && !IsCursorInClient983()))
			g_pMouseCursorHelper983->m_bInside = false;
		else
		{
			g_pMouseCursorHelper983->m_bInside = true;
			if (!IsCursorInClient983())
				g_pMouseCursorHelper983->Restore();
		}
	}
	if (!m_bShow)
		return;
	if (!m_active && !m_tempActive)
		return;
	if (m_forceMoving > 0)
	{
		m_forceMoving -= dt * m_forceFactor;
		if (m_forceMoving < 0)
		{
			m_forceMoving = 0;
			m_oldPointer = m_target;
		}
		m_pointer = m_target + m_forceMoving * m_delta;
		g_input->SetMousePoint(m_pointer.x, m_pointer.y);
		return;
	}
	bool inView = true;
	bool appActive = true;
	if (BindMouseCursor983() != 1 && COption::Instance()->vIsWindowed() &&
		BindMouseCursor983() == 0)
	{
		inView = IsInclude(0, 0, g_view->GetWidth(), g_view->GetHeight());
		appActive = g_bFocus;
	}
	if (m_autoHide &&
		WisEqual(g_input->GetMouseDelta(), WVector::ZERO, g_EPSILON) &&
		!g_input->GetButton(LEFT_BUTTON) && !g_input->GetButton(RIGHT_BUTTON) &&
		!g_input->GetButton(MIDDLE_BUTTON) && inView && appActive)
	{
		m_stopDuration += dt;
		if (m_stopDuration > 3)
			m_show = false;
	}
	else
	{
		m_show = true;
		m_stopDuration = 0;
	}
	if (m_overlay[m_cursor].num > 0)
	{
		m_dt += dt;
		if (m_dt > m_frequency / m_overlay[m_cursor].num)
		{
			m_dt -= m_frequency / m_overlay[m_cursor].num;
			m_frame = (m_frame + 1) % m_overlay[m_cursor].num;
		}
	}
	m_pointer.x = g_input->GetMousePoint().x;
	m_pointer.y = g_input->GetMousePoint().y;
	if (m_mode == SCREEN_TEST)
		return;
	if (m_show)
		m_cursor = CheckArea();
	if (!m_bArea[m_area])
	{
		m_area = AREA_NORMAL;
		m_cursor = UNSELECT;
	}
	for (int i = 0; i < 3; ++i)
		if (g_input->GetButton((eButton)i) == 1)
			m_buttonDownArea[i] = m_area;
	if (m_show && GOLFDOC() && Doc()->m_golfGame.gameType != 2 &&
		GOLFDOC()->m_gameMode == 0x200 && m_bTerrainTooltip &&
		(m_area == AREA_NORMAL || m_area == AREA_RANK))
	{
		m_td4Tooltip += dt;
		if (m_td4Tooltip > 0.2f)
		{
			m_td4Tooltip = 0;
			if (m_pGroundPt)
				m_pGroundPt->SelectGroundTip();
		}
	}
	switch (m_cursor)
	{
	case UNSELECT:
		if (g_input->GetButton(LEFT_BUTTON) == 3 &&
			Abs(g_input->GetButtonDownPos(LEFT_BUTTON).x - m_pointer.x) +
					Abs(g_input->GetButtonDownPos(LEFT_BUTTON).y -
						m_pointer.y) >
				0)
			m_cursor = WRONG;
		break;
	case ROLL_OVER:
		if (g_input->GetButton(LEFT_BUTTON))
		{
			if (m_buttonDownArea[0] == m_area || m_ignoreButtonDownArea)
				m_cursor = m_validButton[0] ? SELECT : WRONG;
			else
				m_cursor = UNSELECT;
		}
		else if (g_input->GetButton(RIGHT_BUTTON))
		{
			if (m_buttonDownArea[1] == m_area || m_ignoreButtonDownArea)
				m_cursor = m_validButton[1] ? SELECT : WRONG;
			else
				m_cursor = UNSELECT;
		}
	}
}

int CMouseCursor::CheckArea()
{
	int mode = m_mode;
	m_validButton[0] = true;
	m_validButton[1] = false; // 983: word [this+0x70] = 0x0001
	switch (mode)
	{
	case SCREEN_GAME:
		return GameCheck();
	case SCREEN_CHAT:
		return ChatCheck();
	case SCREEN_TEST:
		return TestCheck();
	case SCREEN_LOBBY:
		return m_cursor; // 983: mov eax,[this+0x6c]
	}
	return UNSELECT;
}

void CMouseCursor::SetBoundArea(WRect* bound)
{
	if (!bound)
	{
		if (m_bound)
		{
			delete m_bound;
			m_bound = NULL;
		}
	}
	else
	{
		if (!m_bound)
			m_bound = new WRect;
		m_bound->x = bound->x;
		m_bound->y = bound->y;
		m_bound->y = bound->y;
		m_bound->y = bound->y;
	}
}

int CMouseCursor::TestCheck()
{
	m_validButton[0] = true;
	m_validButton[1] = true;
	m_area = AREA_NORMAL;
	return UNSELECT;
}

bool CMouseCursor::IsInclude(float x, float y, float w, float h, bool relative)
{
	if (m_pointer.x >= x && m_pointer.x <= (relative ? 0 : x) + w &&
		m_pointer.y >= y && m_pointer.y <= (relative ? 0 : y) + h)
		return true;
	return false;
}

bool CMouseCursor::SetArea(int area)
{
	if (m_bArea[area])
	{
		m_area = area;
		return true;
	}
	return false;
}

int CMouseCursor::GetCursor()
{
	return m_bRightButton ? BOTH : m_cursor;
}

// 983 00891020
void CMouseCursor::SetCursor983(int cursor, int owner)
{
	if (m_bLocked983)
		return;
	if (m_cursorOwner983 != owner && m_cursor != UNSELECT)
		return;
	m_cursorOwner983 = owner;
	if (cursor < MOUSE_MAX_NUM)
	{
		m_bRightButton = false;
		m_cursor = cursor;
	}
	else
	{
		m_bRightButton = true;
		m_cursor = ROLL_OVER;
	}
}

CGroundPropt::CGroundPropt(const char* name)
{
	m_groundtip = new CSimpleUI(name);
}

bool CMouseCursor::ChildWindowCallback(FrWnd* wnd, void* param)
{
	std::string name;
	if (wnd->IsVisible())
	{
		if (IS_KINDOF(FrGroupBox, wnd))
		{
			wnd->EnumerateChildWindow(ChildWindowCallback, param);
			if (*(int*)param == ROLL_OVER)
				return false;
			return true;
		}
		WRect rect;
		rect = wnd->GetRect();
		if (Instance()->IsInclude(rect.x, rect.y, rect.w, rect.h))
		{
			wnd->GetWindowName(name);
			if (name == "chatview")
			{
				FrWnd* scrollbar = wnd->FindChild("scrollbar");
				if (scrollbar)
				{
					WRect rect;
					rect = scrollbar->GetRect();
					if (Instance()->IsInclude(rect.x, rect.y, rect.w, rect.h))
					{
						goto hit;
					}
				}
			}
			else
			{
hit:
				Instance()->m_area = AREA_RESERVE_GAME;
				*(int*)param = ROLL_OVER;
				return false;
			}
		}
	}
	Instance()->m_area = AREA_NORMAL;
	*(int*)param = UNSELECT;
	return true;
}

float CMouseCursor::RayTrace4Ground()
{
	WVector start;
	g_view->GetScreen2World(m_pointer.x, m_pointer.y, start, m_ray);
	m_ray.Normalize();
	m_ray *= 300.0f;
	if (Wabs(m_ray.y) < g_EPSILON)
		return 1;
	m_rayRatio = g_camera.pivot.y / m_ray.y * -1.0f;
	WVector point = g_camera.pivot + m_ray * m_rayRatio;
	if (point.x > m_groundArea.x && point.x < m_groundArea.w &&
		point.z > m_groundArea.y && point.z < m_groundArea.h)
		return m_rayRatio;
	return 1;
}

void CMouseCursor::SetActive(bool active)
{
	if (active)
	{
		m_pointer = m_oldPointer;
		m_stopDuration = 0;
		m_cursor = UNSELECT;
		g_input->SetMousePoint(m_pointer.x, m_pointer.y);
		m_show = true;
	}
	else
	{
		m_oldPointer = m_pointer;
		m_area = AREA_NORMAL;
		m_cursor = UNSELECT;
	}
	m_active = active;
}

void CMouseCursor::Store()
{
	m_oldPointer = m_pointer;
}

bool CMouseCursor::InArea(const WRect& rect)
{
	if (CQuitWindow::IsInstantiated() && CQuitWindow::Instance()->IsActive())
		return false;
	if (m_pointer.x > rect.x && m_pointer.x < rect.x + rect.w &&
		m_pointer.y > rect.y && m_pointer.y < rect.y + rect.h)
	{
		m_cursor = ROLL_OVER;
		return true;
	}
	return false;
}

void CMouseCursor::Display()
{
	if (!m_bShow)
		return;
	// 983: janela sem cursor preso e cursor do sistema fora da area: nao desenha o proprio
	if (BindMouseCursor983() == 0 && !g_pMouseCursorHelper983->m_bInside &&
		COption::Instance()->vIsWindowed() == true)
		return;
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2 &&
		Doc()->m_replayState == 7)
		return;
	if (!m_active && !m_tempActive)
		return;
	if (!m_show)
		return;
	if (m_mode == SCREEN_GAME && GOLFDOC() && GOLFDOC()->m_gameMode == 0x400)
		return;
	WRect dest(m_pointer.x, m_pointer.y, m_overlay[m_cursor].len,
		m_overlay[m_cursor].len);
	switch (m_cursor)
	{
	case UP:
		dest.y -= 4;
		break;
	case DOWN:
		dest.y -= 28;
		break;
	}
	if (GOLFDOC() && Doc()->m_golfGame.gameType != 2 &&
		GOLFDOC()->m_gameMode == 0x200 && m_bTerrainTooltip &&
		(m_area == AREA_NORMAL || m_area == AREA_RANK))
	{
		bool draw = true;
		if (CQuitWindow::Instance()->IsActive())
			draw = false;
		// 983 009b79c0/009bac50: [00e73e60] e o CTaskManager; GetCurrent = tarefa atual, FindNamed = CTask::GetActor
		CTask* task = AfxGetTask();
		CScore* score = task ? static_cast<CScore*>(task->GetActor("Score")) : NULL;
		if ((!score || !score->IsScoreShown983()) && draw)
			m_pGroundPt->DrawGroundTip();
	}
	m_overlay[m_cursor].overlay->Render(g_view,
		WRect(dest.w * m_frame / m_overlay[m_cursor].w, 0.0f,
			dest.w / m_overlay[m_cursor].w, dest.h / m_overlay[m_cursor].h),
		dest, 0x6080000, 0xffffffff, 0, 0);
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (m_bRightButton && MESSENGER()->IsVisible() &&
		MESSENGER()->m_dlgRect.IsInRect(m_pointer.x, m_pointer.y))
	{
		const Bitmap* bitmap =
			g_pFresh->GetManager()->GetBitmap("MESSENGER", "mouse_right_btn");
		// 983: com a lista da guilda do messenger visivel nao desenha (m_pMessengerDlg +0x94 -> m_pGuildList +0x114 ->
		// m_dwStyle +0x44 & FWS_VISIBLE; offsets do VC10, no cl novo 0x7c/0x104/0x40)
		if ((IsLocalContent((localContentType_t)0x15) &&
				*(int*)DOCPTR983(0x82a) == 0 &&
				MESSENGER()->m_pMessengerDlg &&
				MESSENGER()->m_pMessengerDlg->m_pGuildList &&
				MESSENGER()->m_pMessengerDlg->m_pGuildList->IsVisible()) ||
			!bitmap)
			return;
		device->DrawTexture(bitmap,
			WRect(m_pointer.x + 2, m_pointer.y + 12, (float)bitmap->Width(),
				(float)bitmap->Height()),
			0xffffffff, 0);
	}
	else if (m_bRightButton)
	{
		const Bitmap* bitmap =
			g_pFresh->GetManager()->GetBitmap("NEW_ICON", "mouse_right_button");
		if (bitmap)
			device->DrawTexture(bitmap,
				WRect(m_pointer.x + 2, m_pointer.y + 12, (float)bitmap->Width(),
					(float)bitmap->Height()),
				0xffffffff, 0);
	}
}

void CMouseCursor::SetCamMode(bool mode)
{
	m_camMode = mode;
	if (mode)
	{
		ForceMove(g_view->GetWidth() * 0.65f, g_view->GetHeight() - 150, 0.13f);
		m_show = true;
		m_stopDuration = 0;
		m_cursor = UNSELECT;
	}
}

void CMouseCursor::ForceMove(float x, float y, float duration)
{
	m_show = true;
	m_stopDuration = 0;
	if (duration < 0)
	{
		g_input->SetMousePoint(x, y);
		m_pointer.x = x;
		m_pointer.y = y;
		m_cursor = UNSELECT;
		m_area = AREA_NORMAL;
	}
	else
	{
		m_target.x = x;
		m_target.y = y;
		m_delta = m_pointer - m_target;
		m_cursor = UNSELECT;
		m_area = AREA_NORMAL;
		m_forceMoving = 1;
		m_forceFactor = 1 / duration;
	}
}

CGroundPropt::~CGroundPropt()
{
	if (m_groundtip)
	{
		delete m_groundtip;
		m_groundtip = NULL;
	}
}

void CGroundPropt::DrawGroundTip()
{
	if (m_groundtip)
	{
		m_groundtip->SetColor(0xffffffff);
		float x = g_input->GetMousePoint().x + 3;
		float y = g_input->GetMousePoint().y + 10;
		if (x >= g_view->GetWidth() - 90)
			x = g_view->GetWidth() - 90;
		if (y >= g_view->GetHeight() - 25)
			y = g_view->GetHeight() - 25;
		m_groundtip->Render(m_groundtype, x, y, 1, 0);
	}
}

void CGroundPropt::SelectGroundTip()
{
	WVector start, ray;
	WVector mouse = g_input->GetMousePoint();
	g_view->GetScreen2World(mouse.x, mouse.y, start, ray);
	WVector point;
	point = GetPVS().GetRayIntersection(start, ray);
	if (GOLFDOC()->m_pPolySoup->IsOutOfBound(point))
	{
		strcpy(m_groundtype, "OB");
		return;
	}
	CGolfBall::eWater water = (CGolfBall::eWater)0;
	switch (GetPVS().GetGroundType(point, false, &water))
	{
	case 1:
		strcpy(m_groundtype, "Fairway");
		break;
	case 4:
		strcpy(m_groundtype, "Rough");
		break;
	case 6:
		strcpy(m_groundtype, "Rough");
		break;
	case 2:
		strcpy(m_groundtype, "Green");
		break;
	case 3:
		strcpy(m_groundtype, "Bunker");
		break;
	case 13:
		strcpy(m_groundtype, "Rough");
		break;
	case 7:
		strcpy(m_groundtype, "Ice");
		break;
	case 8:
		strcpy(m_groundtype, "Vector");
		break;
	case 9:
		strcpy(m_groundtype, "Water");
		break;
	case 10:
		if (water == 0 || water == 1)
			strcpy(m_groundtype, "OB");
		else
			strcpy(m_groundtype, "WaterHazard");
		break;
	case 11:
		strcpy(m_groundtype, "Sand");
		break;
	case 12:
		strcpy(m_groundtype, "Rough");
		break;
	case 14:
		strcpy(m_groundtype, "Water");
		break;
	default:
		strcpy(m_groundtype, "Rough");
		break;
	}
}

int CMouseCursor::ChatCheck()
{
	if (CQuitWindow::IsInstantiated() && CQuitWindow::Instance()->IsActive())
	{
		m_area = AREA_TRAINING_OPTION;
		return UNSELECT;
	}
	std::list<FrWnd*>& windows = g_pFresh->GetManager()->GetTopWndList();
	for (std::list<FrWnd*>::iterator it = windows.begin(); it != windows.end();
		++it)
	{
		FrWnd* wnd = *it;
		if (wnd->IsVisible())
		{
			WRect rect = wnd->GetRect();
			if (IsInclude(rect.x, rect.y, rect.w, rect.h))
			{
				m_area = AREA_RESERVE_GAME;
				return ROLL_OVER;
			}
		}
	}
	int cursor = UNSELECT;
	g_pFresh->GetManager()->GetDesktop()->EnumerateChildWindow(
		ChildWindowCallback, &cursor);
	if (cursor)
		return cursor;
	m_area = AREA_NORMAL;
	return UNSELECT;
}

int CMouseCursor::GameCheck()
{
	if (!GOLFDOC() ||
		(CQuitWindow::IsInstantiated() && CQuitWindow::Instance()->IsActive()))
		return UNSELECT;
	// 983: as areas vem do layout nomeado (CUiFrameSetManager), escala = tela / 640x480
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	m_validButton[0] = true;
	m_validButton[1] = false;
	if (m_bound)
	{
		m_pointer.x = Between(m_bound->x, m_pointer.x, m_bound->x + m_bound->w);
		m_pointer.y = Between(m_bound->y, m_pointer.y, m_bound->y + m_bound->h);
		g_input->SetMousePoint(m_pointer.x, m_pointer.y);
	}
	if (OnlinePlay())
	{
		CGolfDoc* golf = GOLFDOC();
		if (!(golf->m_gameMode & 0x300))
		{
			if (Doc()->m_golfGame.gameType == 1)
			{
				WRect rect;
				rect.y = 20;
				rect.w = 118;
				rect.h = 127;
				for (int i = 0; i < 2; ++i)
				{
					rect.x = (i & 1) ? g_view->GetWidth() - 118 : 0;
					if (rect.IsInRect(m_pointer.x, m_pointer.y))
					{
						rect.x += (i & 1) ? 49.0f : 4.0f;
						rect.y += 59;
						rect.w = 65;
						rect.h = 7;
						if (rect.IsInRect(m_pointer.x, m_pointer.y))
						{
							m_area = AREA_COMBO1 + i;
							return ROLL_OVER;
						}
						rect.x = (i & 1) ? g_view->GetWidth() - 118 : 74;
						int j = 0;
						rect.w = 43;
						rect.h = 47;
						for (; j < golf->m_playerNum; ++j, golf = GOLFDOC())
						{
							if (golf->GetPlayer(j)->team == i)
							{
								if (GOLFDOC()->GetPlayer(j)->teammate == 0xff)
									rect.y = 45;
								else
									rect.y =
										j < GOLFDOC()->GetPlayer(j)->teammate
										? 45.0f
										: 94.0f;
								if (rect.IsInRect(m_pointer.x, m_pointer.y))
								{
									m_area = AREA_USERINFO1 + j;
									return ROLL_OVER;
								}
							}
						}
					}
				}
			}
			else
			{
				for (unsigned char i = 0; i < golf->m_playerNum;
					++i, golf = GOLFDOC())
				{
					if (golf->GetPlayer(i)->state == 3)
						continue;
					WRect rect;
					rect.x = (i & 2) ? g_view->GetWidth() - 118 : 0;
					if (i & 1)
						rect.y = 123;
					else
						rect.y = 30;
					rect.w = 118;
					rect.h = 64;
					if (rect.IsInRect(m_pointer.x, m_pointer.y))
					{
						rect.x += (i & 2) ? 49.0f : 4.0f;
						rect.y += 19;
						rect.w = 65;
						rect.h = 7;
						if (rect.IsInRect(m_pointer.x, m_pointer.y))
							m_area = AREA_COMBO1 + i;
						else
							m_area = AREA_USERINFO1 + i;
						return ROLL_OVER;
					}
				}
			}
		}
	}
	else
	{
		if (GOLFDOC()->m_tutorialMode < 15)
		{
			if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV && IsInclude(0, 245, 30, 30))
			{
				m_area = AREA_CHECKPOINT;
				return ROLL_OVER;
			}
		}
		else if (Doc()->m_gameMode == 2 && IsInclude(0, 245, 30, 30))
		{
			m_area = AREA_TRAINING_OPTION;
			return ROLL_OVER;
		}
	}

	if ((Doc()->m_golfGame.gameType == 4 || Doc()->m_golfGame.gameType == 5) &&
		IsInclude(g_view->GetWidth() - 240, 5, 240, 160))
	{
		m_area = AREA_RANK;
		return UNSELECT;
	}
	if (m_pointer.y == 2)
	{
		m_area = AREA_TOP;
		return UP;
	}
	if (m_pointer.y == g_view->GetHeight() - 2)
	{
		m_area = AREA_BOTTOM;
		return DOWN;
	}
	if (g_input->GetButton(LEFT_BUTTON))
	{
		WVector point = g_input->GetButtonDownPos(LEFT_BUTTON) -
			WVector(g_view->GetWidth() * 0.65f, g_view->GetHeight() - 150, 0);
		if (sqrtf(point.x * point.x + point.y * point.y + point.z * point.z) <
			42)
			m_ignoreButtonDownArea = true;
	}
	if (m_camMode &&
		(m_ignoreButtonDownArea || !g_input->GetButton(LEFT_BUTTON)))
	{
		float dy = (float)((double)m_pointer.y - (g_view->GetHeight() - 150));
		float dx = (float)((double)m_pointer.x - g_view->GetWidth() * 0.65f);
		float length = sqrtf(dx * dx + dy * dy);
		if (length < 15)
		{
			m_area = AREA_CAMRESET;
			return ROLL_OVER;
		}
		if (length < 42)
		{
			WVector direction(m_pointer.x - g_view->GetWidth() * 0.65f, 0,
				m_pointer.y - (g_view->GetHeight() - 150));
			float angle =
				CalcDeltaAngle(WVector::UNIT_POS_Z, direction) * g_RADTODEG;
			switch (float2int(floorf(angle / 60.0f)))
			{
			case -2:
				m_area = AREA_CAMBACKWARD;
				break;
			case -1:
				m_area = AREA_CAMLEFT;
				break;
			case 0:
				m_area = AREA_CAMRIGHT;
				break;
			case 1:
				m_area = AREA_CAMDOWN;
				break;
			case 2:
				m_area = AREA_CAMUP;
				break;
			case -3:
				m_area = AREA_CAMFORWARD;
				break;
			}
			if (GOLFDOC()->m_gameMode == 0x8000 &&
				(m_area == AREA_CAMFORWARD || m_area == AREA_CAMBACKWARD))
			{
				m_area = AREA_NORMAL;
				return UNSELECT;
			}
			return ROLL_OVER;
		}
	}
	CUiFrameSetManager::NamedRect* rc =
		CUiFrameSetManager::Instance()->FindNamedRect("r_club_pet_area");
	if (rc && rc->HitTest(scl, WPoint(m_pointer.x, m_pointer.y)))
	{
		m_validButton[1] = true;
		m_area = AREA_CLUB;
		return ROLL_OVER;
	}
	{
		CUiFrameSetManager::NamedPoint* c =
			CUiFrameSetManager::Instance()->FindNamedPoint("p_club_type_center");
		CUiFrameSetManager::NamedDim* r =
			CUiFrameSetManager::Instance()->FindNamedDim("d_club_type_area_r");
		if (c && r)
		{
			WPoint pt;
			c->Calc(scl, pt);
			float dx = m_pointer.x - pt.x;
			float dy = m_pointer.y - pt.y;
			if (sqrtf(dx * dx + dy * dy) < r->Calc(scl))
			{
				m_area = AREA_CLUB_CIRCLE;
				return ROLL_OVER;
			}
		}
	}
	if (GolfClub().GetType() != 3)
	{
		CUiFrameSetManager::NamedDim* cx =
			CUiFrameSetManager::Instance()->FindNamedDim("d_wind_direction_center_x");
		CUiFrameSetManager::NamedDim* cy =
			CUiFrameSetManager::Instance()->FindNamedDim("d_wind_direction_center_y");
		CUiFrameSetManager::NamedDim* r =
			CUiFrameSetManager::Instance()->FindNamedDim("d_wind_direction_area_r");
		if (cx && cy && r)
		{
			float dx = (float)((double)m_pointer.x - cx->Calc(scl));
			float dy = (float)((double)m_pointer.y - cy->Calc(scl));
			if (sqrtf(dx * dx + dy * dy) < r->Calc(scl))
			{
				m_area = AREA_WIND;
				return ROLL_OVER;
			}
		}
		CUiFrameSetManager::NamedPoint* c =
			CUiFrameSetManager::Instance()->FindNamedPoint("p_wind_speed_center");
		r = CUiFrameSetManager::Instance()->FindNamedDim("d_wind_speed_area_r");
		if (c && r)
		{
			WPoint pt;
			c->Calc(scl, pt);
			float dx = m_pointer.x - pt.x;
			float dy = m_pointer.y - pt.y;
			if (sqrtf(dx * dx + dy * dy) < r->Calc(scl))
			{
				m_area = AREA_WIND;
				return ROLL_OVER;
			}
		}
	}
	if (Doc()->m_golfGame.shotTimeLimit)
	{
		CUiFrameSetManager::NamedPoint* c =
			CUiFrameSetManager::Instance()->FindNamedPoint("p_digital_timer_center");
		CUiFrameSetManager::NamedDim* r =
			CUiFrameSetManager::Instance()->FindNamedDim("d_digital_timer_area_r");
		if (c && r)
		{
			WPoint pt;
			c->Calc(scl, pt);
			float dx = m_pointer.x - pt.x;
			float dy = m_pointer.y - pt.y;
			if (sqrtf(dx * dx + dy * dy) < r->Calc(scl))
			{
				m_area = AREA_TIMER;
				return ROLL_OVER;
			}
		}
	}
	if (CChatMsg::Instance()->IsActive())
	{
		rc = CUiFrameSetManager::Instance()->FindNamedRect("r_chat_input_window_area");
		if (rc && rc->HitTest(scl, WPoint(m_pointer.x, m_pointer.y)))
		{
			m_area = AREA_CHAT_WINDOW;
			return ROLL_OVER;
		}
	}
	if (!m_itemWindow && !CChatMsg::Instance()->IsActive() &&
		GolfClub().BarIs(2))
	{
		bool onBar = false;
		if (GolfClub().GetType() != 3)
			rc = CUiFrameSetManager::Instance()->FindNamedRect("r_clickable_driver_bar");
		else
			rc = CUiFrameSetManager::Instance()->FindNamedRect("r_clickable_putter_bar");
		if (rc)
		{
			WRect rect;
			rc->Calc(scl, rect);
			onBar = IsInclude(rect.x, rect.y, rect.w, rect.h);
		}
		if (!onBar)
		{
			rc = CUiFrameSetManager::Instance()->FindNamedRect("r_clickable_thumb_status");
			if (rc)
			{
				WRect rect;
				rc->Calc(scl, rect);
				onBar = IsInclude(rect.x, rect.y, rect.w, rect.h);
			}
		}
		if (onBar)
		{
			if (!IsMyTurn(false))
				return UNSELECT;
			m_area = AREA_BAR;
			return ROLL_OVER;
		}
	}
	if (IsMyTurn(false) && GolfClub().GetType() != 3)
	{
		CUiFrameSetManager::NamedDim* cx =
			CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
		CUiFrameSetManager::NamedDim* cy =
			CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
		CUiFrameSetManager::NamedDim* rc1 =
			CUiFrameSetManager::Instance()->FindNamedDim("d_spin_control_area_r");
		CUiFrameSetManager::NamedDim* rc2 = CUiFrameSetManager::Instance()->FindNamedDim("d_spin_area_r");
		if (cx && cy && rc1 && rc2)
		{
			float dx = (float)((double)m_pointer.x - cx->Calc(scl));
			float dy = (float)((double)m_pointer.y - cy->Calc(scl));
			float length = sqrtf(dx * dx + dy * dy);
			if (length < rc1->Calc(scl) && g_input->GetButton(LEFT_BUTTON) > 0)
			{
				m_area = AREA_SPIN;
				return SELECT;
			}
			if (length < rc2->Calc(scl) && !g_input->GetButton(LEFT_BUTTON))
			{
				m_area = AREA_SPIN;
				return ROLL_OVER;
			}
		}
	}
	if (m_itemWindow)
	{
		// 983 0075cf30: p_item_window_area (sem ele: x = 65, y = barra - 25)
		float ax, ay;
		CUiFrameSetManager::NamedPoint* c =
			CUiFrameSetManager::Instance()->FindNamedPoint("p_item_window_area");
		if (c)
		{
			WPoint pt;
			c->Calc(scl, pt);
			ax = (float)floor((double)pt.x);
			ay = (float)floor((double)pt.y);
		}
		else
		{
			ax = 65.0f;
			ay = COption::Instance()->gGetBar_Y() - 25.0f;
		}
		if (ax + 49 < m_pointer.x &&
			(double)m_pointer.x <
				(double)ax + 49 +
					(double)PLAYER(GOLFDOC()->m_currentPlayer)->itemNum * 45 +
					15 &&
			ay < m_pointer.y && m_pointer.y < ay + 50)
		{
			m_validButton[1] = true;
			m_area = AREA_ITEMSLOT;
			return ROLL_OVER;
		}
	}
	{
		CUiFrameSetManager::NamedDim* cx =
			CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
		CUiFrameSetManager::NamedDim* cy =
			CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_y");
		CUiFrameSetManager::NamedDim* rmin =
			CUiFrameSetManager::Instance()->FindNamedDim("d_item_chat_button_area_min_r");
		CUiFrameSetManager::NamedDim* rmax =
			CUiFrameSetManager::Instance()->FindNamedDim("d_item_chat_button_area_max_r");
		if (cx && cy && rmin && rmax)
		{
			float x = (float)((double)m_pointer.x - cx->Calc(scl));
			float y = (float)(cy->Calc(scl) - m_pointer.y);
			float angle = atanf(y / x) * g_RADTODEG;
			float distance = sqrtf(x * x + y * y);
			if (x > 0 && rmin->Calc(scl) < distance &&
				distance < rmax->Calc(scl))
			{
				if (angle > 40 && angle < 75)
				{
					m_area = AREA_ITEM;
					return ROLL_OVER;
				}
				if (angle > 10 && angle < 40)
				{
					m_area = AREA_CHAT;
					return ROLL_OVER;
				}
			}
		}
	}
	if (!g_input->Get("LCONTROL", true))
		g_input->Get("RCONTROL", true);
	if (IsInclude(m_holecup.x - 32, m_holecup.y - 47, 64, 64))
	{
		m_validButton[0] = false;
		m_area = AREA_HOLEINFO;
		return UNSELECT;
	}
	{
		static const char* const autoNames[3] = {
			"r_automatic_toma_button", "r_automatic_cobra_button",
			"r_automatic_spike_button"};
		CUiFrameSetManager::NamedRect* autos[3];
		for (int i = 0; i < 3; ++i)
			autos[i] = CUiFrameSetManager::Instance()->FindNamedRect(autoNames[i]);
		for (int i = 0; i < 3; ++i)
		{
			if (autos[i] && autos[i]->HitTest(scl, WPoint(m_pointer.x, m_pointer.y)))
			{
				m_area = 0x49 + i;
				return ROLL_OVER;
			}
		}
	}
	m_validButton[0] = false;
	m_validButton[1] = false;
	m_area = AREA_NORMAL;
	return UNSELECT;
}
