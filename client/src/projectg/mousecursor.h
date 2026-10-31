#pragma once
#include <vector>
#include "baseobject.h"
#include "singleton.h"

class CGroundPropt;
class CTextToolTip;
class FrWnd;

class CMouseCursor : public WSingleton<CMouseCursor>
{
	// 983 00716890: CCamera::ProcessInput chama o IsInclude (privado no 983)
	friend class CCamera;
public:
	using WSingleton<CMouseCursor>::m_pInstance;
	void SetHoleCup(float x, float y)
	{
		m_holecup.x = x;
		m_holecup.y = y;
	}
	CMouseCursor();
	virtual ~CMouseCursor();
	void Process(float dt);
	void Display();
	void Store();
	void SetBoundArea(WRect* bound);
	void SetCamMode(bool mode);
	float RayTrace4Ground();

	enum
	{
		UNSELECT,
		ROLL_OVER,
		SELECT,
		WRONG,
		UP,
		DOWN,
		ZOOM,
		BOTH,
		RM_MOVE,
		RM_MOVE_NO,
		RM_ROTATE,
		RM_ROTATE_NO,
		CARET983,	// 983 (ctor 00893bd0: "_caret", "_context", "_rotate2"; new 0x12c = 15 x 0x14)
		CONTEXT983,
		ROTATE2_983,
		MOUSE_MAX_NUM
	};

	enum eMode
	{
		SCREEN_LOBBY,
		SCREEN_GAME,
		SCREEN_CHAT,
		SCREEN_TEST
	};

	enum
	{
		AREA_NORMAL = 0,
		AREA_CLUB = 1,
		AREA_CLUB_CIRCLE = 2,
		AREA_SPIN = 3,
		AREA_CHAT_WINDOW = 4,
		AREA_BAR = 5,
		AREA_ITEM = 6,
		AREA_CHAT = 7,
		AREA_TOP = 8,
		AREA_BOTTOM = 9,
		AREA_USERINFO1 = 10,
		AREA_USERINFO2 = 11,
		AREA_USERINFO3 = 12,
		AREA_USERINFO4 = 13,
		AREA_COMBO1 = 14,
		AREA_COMBO2 = 15,
		AREA_COMBO3 = 16,
		AREA_COMBO4 = 17,
		AREA_ITEMSLOT = 18,
		AREA_WIND = 19,
		AREA_TIMER = 20,
		AREA_HOLEINFO = 21,
		AREA_CAMRESET = 22,
		AREA_CAMLEFT = 23,
		AREA_CAMRIGHT = 24,
		AREA_CAMUP = 25,
		AREA_CAMDOWN = 26,
		AREA_CAMFORWARD = 27,
		AREA_CAMBACKWARD = 28,
		AREA_EXIT_YES = 29,
		AREA_EXIT_NO = 30,
		AREA_TUTORIAL_NEXT = 31,
		AREA_TUTORIAL_REVIEW = 32,
		AREA_CHECKPOINT = 33,
		AREA_RANK = 34,
		AREA_CHAT_SCROLLBAR = 35,
		AREA_TRAINING_OPTION = 36,
		AREA_AUXMENU_SHOW = 37,
		AREA_AUXMENU_OPEN = 38,
		AREA_AUXMENU_OPTION = 39,
		AREA_AUXMENU_TIP = 40,
		AREA_AUXMENU_UNPLAYABLE = 41,
		AREA_AUXMENU_PAUSE = 42,
		AREA_AUXMENU_REPORT = 43,
		AREA_AUXMENU_USERLIST = 44,
		AREA_EMOTUI_WEATHER = 45,
		AREA_EMOTUI_ITEM = 46,
		AREA_EMOTUI_ACTION = 47,
		AREA_EMOTUI_EXPRESSION0 = 48,
		AREA_EMOTUI_EXPRESSION1 = 49,
		AREA_EMOTUI_CLOSE = 50,
		AREA_EMOTUI_EMOTICON_00 = 51,
		AREA_EMOTUI_EMOTICON_01 = 52,
		AREA_EMOTUI_EMOTICON_02 = 53,
		AREA_EMOTUI_EMOTICON_03 = 54,
		AREA_EMOTUI_EMOTICON_04 = 55,
		AREA_EMOTUI_EMOTICON_05 = 56,
		AREA_EMOTUI_EMOTICON_06 = 57,
		AREA_EMOTUI_EMOTICON_07 = 58,
		AREA_EMOTUI_EMOTICON_08 = 59,
		AREA_EMOTUI_EMOTICON_09 = 60,
		AREA_EMOTUI_EMOTICON_10 = 61,
		AREA_EMOTUI_EMOTICON_11 = 62,
		AREA_RAY_CHECK = 63,
		AREA_RESERVE_GAME = 64,
		AREA_MESSANGER = 65,
		AREA_TRY_ON = 66,
		AREA_OPTION = 67,
		AREA_CANCEL_RESERVE = 68,
		AREA_GAME_START = 69,
		AREA_LEFT = 70,
		AREA_RIGHT = 71,
		AREA_EXIT_TO_LOBBY = 72,
		AREA_MAX = 73
	};

	bool IsActive() { return m_active; }
	void Toggle(bool show) { m_bShow = show; }
	bool IsVisible() { return m_show; }
	void SetAutoHide(bool autoHide) { m_autoHide = autoHide; }

#ifdef REBANG_GOLFTASK_CPP // HACK: other includers keep their IL (emission order)
	void SetTempActive(bool active)
	{
		m_tempActive = active;
		if (m_tempActive)
		{
			m_stopDuration = 0.0f;
			m_show = true;
		}
	}
#endif

	void Reset(bool active);
	void SetActive(bool active);
	void SetCursor(int cursor) { SetCursor983(cursor, 0); } // 983: so existe SetCursor983 (00891020); chamadores 645 passam dono 0
	int GetCursor();
	// 983 00891020: SetCursor com dono (a RF passa 1): nao troca travado (m_bLocked983) nem cursor de outro dono fora do 0
	void SetCursor983(int cursor, int owner);
	bool InArea(const WRect& rect);
	void ForceMove(float x, float y, float duration);

	int GetArea() { return m_area; }
	bool IsInButtonDownArea(eButton button)
	{
		return m_buttonDownArea[button] == m_area;
	}
	bool IsCamMode() { return m_camMode; }
	void SetMode(int mode) { m_mode = mode; }
#ifdef REBANG_GOLFRULE_PCH
	void Enable(int area, bool enable) { m_bArea[area] = enable; }
#endif
	void SetItemWindow(bool itemWindow) { m_itemWindow = itemWindow; }
	void SetTerrainToolTip(bool tooltip) { m_bTerrainTooltip = tooltip; }

private:
	int CheckArea();
	int GameCheck();
	int ChatCheck();
	int TestCheck();
	static bool ChildWindowCallback(FrWnd* wnd, void* param);
	bool IsInclude(float x, float y, float w, float h, bool relative = false);
	bool SetArea(int area);

protected:
	WVector2D m_pointer;
	WVector2D m_oldPointer;
	WVector2D m_mouse;
	WVector2D m_target;
	WVector2D m_delta;
	int m_mode;

public:
	struct sPointer
	{
		WOverlay* overlay;
		float w;
		float h;
		float len;
		int num;
	};

protected:
	sPointer* m_overlay;
	float m_stopDuration;
	float m_frequency;
	int m_frame;
	bool m_show;
	bool m_active;
	int m_area;
	int m_cursor;
	bool m_validButton[3];
	float m_dt;
	WRect* m_bound;
	bool m_camMode;
	float m_forceMoving;
	float m_forceFactor;
	int m_layer;
	std::vector<int> m_activeArea;
	bool m_itemWindow;
	int m_buttonDownArea[3];
	bool m_ignoreButtonDownArea;
	bool m_tempActive;
	WVector2D m_holecup;
	WRect m_groundArea;
	WVector m_ray;
	float m_rayRatio;
	bool m_autoHide;

public:
	bool m_bArea[76]; // 983: +0xd9, 0x4c bytes (memset do Reset 00892248); o 645 tinha 73

protected:
	bool m_bShow;
	bool m_bRightButton;
	CGroundPropt* m_pGroundPt;
	bool m_bTerrainTooltip;
	float m_td4Tooltip;
	CTextToolTip* m_pTextTip;

public:
	bool m_bLocked983; // +0x138 (983; RFSceneManager::SetCursor983 trava/destrava; visao da B: CMouseCursorJp983)
	int m_cursorOwner983; // +0x13c (983 SetCursor983)
};

class CMouseCursorHelper;
// 983: COption::m_opt.gBindMouseCursor (+0x1e8)
#define BindMouseCursor983() (COption::Instance()->Opt983().gBindMouseCursor) // B1466: cru +0x1e8 andava -4 no cl 14.44
extern CMouseCursorHelper* g_pMouseCursorHelper983; // 00e7ba5c
// 983: objeto do cursor do sistema (global 00e7ba5c, criado no InitSingletonClasses 00a670c9; +0x28 estado, +0x2c HCURSOR, +0x30 dentro, +0x34 contador do ShowCursor)
class CMouseCursorHelper
{
public:
	CMouseCursorHelper() : m_state(0), m_hCursor(LoadCursor(NULL, IDC_ARROW)), m_bInside(true), m_showCount(0)
	{
		g_pMouseCursorHelper983 = this;
	}
	void Restore(); // 008916b0
	void SetState(int state); // 00891180 (WM_SETCURSOR: 1 = cliente, esconde)
	void SetVisible(bool show); // 007e2040
	void ShowCountFix(int a); // 007e1fe0 (contador negativo)
	void HideCountFix(int a); // 007e2010
	char m_pad[0x28];
	int m_state;
	HCURSOR m_hCursor;
	bool m_bInside;
	int m_showCount;
};
