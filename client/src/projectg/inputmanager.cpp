#include "minatl.h"

#include <vector>
#include "projectg.h"
#include "clientsetting.h"

// user32 do Vista+, importados pelo 983 em 00cfc9bc/00cfca14; o user32.lib do SDK de 2002 do link nao os tem
BOOL GetPhysicalCursorPos983(LPPOINT pt)
{
	typedef BOOL (__stdcall *PFN)(LPPOINT);
	static PFN pfn = (PFN)GetProcAddress(GetModuleHandleA("user32.dll"), "GetPhysicalCursorPos");
	return pfn ? pfn(pt) : GetCursorPos(pt);
}
static BOOL SetPhysicalCursorPos983(int x, int y)
{
	typedef BOOL (__stdcall *PFN)(int, int);
	static PFN pfn = (PFN)GetProcAddress(GetModuleHandleA("user32.dll"), "SetPhysicalCursorPos");
	return pfn ? pfn(x, y) : SetCursorPos(x, y);
}
extern HWND g_hwnd;

struct CInputManager::KeyPair
{
	const char* name;
	std::vector<int> keys;
	int type;
	unsigned long flag;
	int state;
	int oldState;
	unsigned long eventTime;
	bool bFake;
};
const char* aliasList[] = { "\xbb\xf3", "\xc7\xcf", "\xc1\xc2", "\xbf\xec",
	"HOME", "END", "ITEMSLOT", "NUM_1", "PAD_1", "CALIPERS_RIGHT",
	"CALIPERS_LEFT" };
CInputManager* g_input = NULL;
extern WInputDev* g_keyboard;
extern WInputDev* g_mouse;
CInputManager::~CInputManager()
{
	// D860: crus +0x54/+0x58 (jp/lobbyavataractor, fontinfo) usam VC10_OFS_BO
	C_ASSERT(offsetof(CInputManager, m_mousePoint) == VC10_OFS_BO(0x54));
	EraseAllList();
}

void CInputManager::Reset()
{
	KeyPair* pKey = m_keyList.Start();
	while (pKey)
	{
		pKey->oldState = pKey->state = 0;
		pKey->bFake = false;
		pKey = m_keyList.Next();
	}

	m_mouseDelta.x = m_mouseDelta.y = m_mouseDelta.z = 0.0f;

	m_mousePoint.x = g_view->GetWidth() * 0.625f;
	m_mousePoint.y = g_view->GetHeight() * 0.5f;
	m_mousePoint.z = 0.0f;
	m_bFixPointer = false;

	for (int i = 0; i < 3; ++i)
	{
		m_button[i].state = 0;
		m_button[i].downPos = WVector::ZERO;
		m_button[i].interval = 0;
		m_button[i].lastTime = timeGetTime();
		m_button[i].bFake = false;
		m_button[i].bBlock = false;
	}

	m_mouseFlag = 0;
	m_inputMode = INPUT_GAME;
	m_bActive = true;
}

void CInputManager::SetMousePoint(float x, float y)
{
	float screenW = 800.0f;
	float screenH = 600.0f;
	if (g_view)
	{
		screenW = g_view->GetWidth();
		screenH = g_view->GetHeight();
	}

	// 983 009a66f0: em janela o Update le o cursor do Windows; sem mover o cursor o ponto voltaria no quadro seguinte
	const sOption* pOpt = COption::Instance()->GetOption();
	if (pOpt->gBindMouseCursor != 1 && COption::Instance()->vIsWindowed())
	{
		if (pOpt->gBindMouseCursor == 0)
		{
			m_mousePoint.x = x;
			m_mousePoint.y = y;
			POINT pt;
			pt.x = (int)x;
			pt.y = (int)y;
			ClientToScreen(g_hwnd, &pt);
			SetPhysicalCursorPos983(pt.x, pt.y);
		}
		return;
	}

	m_mousePoint.x = Between(2.0f, x, screenW - 2.0f);
	m_mousePoint.y = Between(2.0f, y, screenH - 2.0f);
}

void CInputManager::SetMousePointToLoginBox()
{
	SetMousePoint(500.0f, 300.0f);
}

void CInputManager::Update()
{
	if (!m_bActive)
		return;

	KeyPair* pKey = m_keyList.Start();
	while (pKey)
	{
		std::vector<int>::iterator it;
		pKey->oldState = pKey->state & 1;
		pKey->state = 0;

		for (it = pKey->keys.begin(); it != pKey->keys.end(); ++it)
		{
			if (g_keyboard->GetState(pKey->type, *it))
			{
				pKey->state = 1;
				break;
			}
		}

		if (pKey->bFake)
		{
			pKey->bFake = false;
			pKey->state = 1;
		}
		else
		{
			if (pKey->flag & 6)
				pKey->state = 0;
			if (pKey->type == 3 && pKey->state)
				pKey->eventTime = g_keyboard->GetEventTime(pKey->keys[0]);
		}

		pKey = m_keyList.Next();
	}

	if (m_mouseFlag & 6)
	{
		m_mouseDelta.x = 0.0f;
		m_mouseDelta.y = 0.0f;
	}
	else
	{
		m_mouseDelta.x = g_mouse->GetState(0, 1) * m_sensitivity;
		m_mouseDelta.y = g_mouse->GetState(0, 2) * m_sensitivity;
	}

	if (m_button[MIDDLE_BUTTON].bBlock)
		m_mouseDelta.z = 0.0f;
	else
		m_mouseDelta.z = (float)g_mouse->GetState(0, 3);

	for (int i = 0; i < 3; ++i)
	{
		int state = g_mouse->GetState(1, i);

		if (m_button[i].bFake)
		{
			m_button[i].bFake = false;
			m_button[i].state = ((m_button[i].state & 1) << 1) + 1;
		}
		else
		{
			m_button[i].state = ((m_button[i].state & 1) << 1) +
				(m_button[i].bBlock ? 0 : (state != 0));
		}
	}

	// 983 009a722d: em janela (gBindMouseCursor != 1) o ponteiro segue o cursor do Windows (posicao absoluta,
	// GetPhysicalCursorPos + ScreenToClient); o 645 so somava os deltas do DirectInput (o robo do PC move o
	// cursor com SetCursorPos, sem delta, e o ponteiro do jogo ficava parado: C96)
	const sOption* pOpt = COption::Instance()->GetOption();
	if (pOpt->gBindMouseCursor != 1 && COption::Instance()->vIsWindowed())
	{
		if (pOpt->gBindMouseCursor == 0 && !(m_mouseFlag & 2))
		{
			POINT pt;
			GetPhysicalCursorPos983(&pt);
			ScreenToClient(g_hwnd, &pt);
			m_mousePoint.x = (float)pt.x;
			m_mousePoint.y = (float)pt.y;
			ClientToScreen(g_hwnd, &pt);
			SetPhysicalCursorPos983(pt.x, pt.y);
		}
	}
	else if (!m_bFixPointer)
	{
		m_mousePoint.x = Between(2.0f, m_mousePoint.x + m_mouseDelta.x,
			g_view->GetWidth() - 2.0f);
		m_mousePoint.y = Between(2.0f, m_mousePoint.y + m_mouseDelta.y,
			g_view->GetHeight() - 2.0f);
	}

	DWORD time = timeGetTime();

	if (m_button[LEFT_BUTTON].state == 1)
	{
		m_button[LEFT_BUTTON].downPos = m_mousePoint;
		m_button[LEFT_BUTTON].interval = time - m_button[LEFT_BUTTON].lastTime;
		m_button[LEFT_BUTTON].lastTime = time;
	}
	else if (m_button[LEFT_BUTTON].state == 2)
		m_button[LEFT_BUTTON].upPos = m_mousePoint;

	if (m_button[RIGHT_BUTTON].state == 1)
	{
		m_button[RIGHT_BUTTON].downPos = m_mousePoint;
		m_button[RIGHT_BUTTON].interval =
			time - m_button[RIGHT_BUTTON].lastTime;
		m_button[RIGHT_BUTTON].lastTime = time;
	}
	else if (m_button[RIGHT_BUTTON].state == 2)
		m_button[RIGHT_BUTTON].upPos = m_mousePoint;

	if (m_button[MIDDLE_BUTTON].state == 1)
	{
		m_button[MIDDLE_BUTTON].downPos = m_mousePoint;
		m_button[MIDDLE_BUTTON].interval =
			time - m_button[MIDDLE_BUTTON].lastTime;
		m_button[MIDDLE_BUTTON].lastTime = time;
	}
	else if (m_button[MIDDLE_BUTTON].state == 2)
		m_button[MIDDLE_BUTTON].upPos = m_mousePoint;
}

unsigned long CInputManager::GetEventTime(const char* name)
{
	KeyPair* pKey = m_keyList.Find(name);

	unsigned long time = pKey ? pKey->eventTime : 0;

	if (m_inputMode == INPUT_CHAT && !(pKey->flag & 1))
		time = 0;

	return time;
}

unsigned long CInputManager::GetEventTime(eButton button)
{
	return g_mouse->GetEventTime(button);
}

bool CInputManager::GetDoubleClick(eButton button)
{
	if (!m_bActive)
		return false;

	if (m_bSwapButton)
	{
		if (button == LEFT_BUTTON)
			button = RIGHT_BUTTON;
		else if (button == RIGHT_BUTTON)
			button = LEFT_BUTTON;
	}

	if (m_button[button].state == 1 && m_button[button].interval < 300)
		return true;

	return false;
}

int CInputManager::GetButton(eButton button)
{
	if (!m_bActive)
		return 0;

	if (m_bSwapButton)
	{
		if (button == LEFT_BUTTON)
			button = RIGHT_BUTTON;
		else if (button == RIGHT_BUTTON)
			button = LEFT_BUTTON;
	}

	return m_button[button].state;
}

int CInputManager::Get(const char* name, bool bCheckMode)
{
	if (!m_bActive)
		return 0;

	KeyPair* pKey = m_keyList.Find(name);

	int state = pKey ? pKey->state : 0;

	if (bCheckMode && m_inputMode == INPUT_CHAT && state && !(pKey->flag & 1))
		state = 0;

	return state & 1;
}

int CInputManager::GetList()
{
	for (int i = 0; i < sizeof(aliasList) / sizeof(aliasList[0]); ++i)
	{
		if (Get(aliasList[i], true))
			return 1;
	}

	return 0;
}

int CInputManager::Get(int key)
{
	if (!m_bActive)
		return 0;

	return g_keyboard->GetState(2, key);
}

int CInputManager::GetDown(const char* name, bool bCheckMode)
{
	if (!m_bActive)
		return 0;

	KeyPair* pKey = m_keyList.Find(name);

	int down = pKey ? pKey->state & (pKey->state ^ pKey->oldState) : 0;

	// o 983 (009a6d22) lê pKey->flag sem testar; aqui testa (AV a174-6/c103-2 no lobby em modo chat, B217.1)
	if (bCheckMode && m_inputMode == INPUT_CHAT && pKey && !(pKey->flag & 1))
		return 0;

	return down;
}

void CInputManager::ExclusiveGetDownUseDone(const char* name)
{
	if (!m_bActive)
		return;

	KeyPair* pKey = m_keyList.Find(name);

	if (pKey && pKey->state == 1)
		pKey->state = 3;
}

int CInputManager::GetUp(const char* name)
{
	if (!m_bActive)
		return 0;

	KeyPair* pKey = m_keyList.Find(name);

	int up = pKey ? pKey->oldState & (pKey->oldState ^ pKey->state) : 0;

	if (m_inputMode == INPUT_CHAT && !(pKey->flag & 1))
		return 0;

	return up;
}

WVector CInputManager::GetMouseDelta()
{
	return m_mouseDelta;
}

void CInputManager::Register(const char* name, int key, unsigned long flag,
	int type)
{
	KeyPair* pKey = m_keyList.Find(name);

	if (!pKey)
	{
		pKey = new KeyPair;
		pKey->name = name;
		pKey->keys.push_back(key);
		pKey->oldState = 0;
		pKey->state = 0;
		pKey->flag = flag;
		pKey->type = type;
		pKey->eventTime = 0;
		pKey->bFake = false;
		m_keyList.AddItem(pKey, name, false);
	}
	else
	{
		if (pKey->type != 3)
		{
			pKey->keys.push_back(key);
		}
	}
}

void CInputManager::IncludeKey(const char* name)
{
	if (name)
	{
		KeyPair* pKey = m_keyList.Find(name);

		if (pKey)
			pKey->flag &= ~4;
	}
	else
	{
		KeyPair* pKey = m_keyList.Start();
		while (pKey)
		{
			pKey->flag &= ~4;
			pKey = m_keyList.Next();
		}
	}
}

void CInputManager::ExcludeKey(const char* name)
{
	KeyPair* pKey = m_keyList.Find(name);

	if (pKey)
		pKey->flag |= 4;
}

void CInputManager::BlockKey(const char* name)
{
	KeyPair* pKey = m_keyList.Find(name);

	if (pKey)
		pKey->flag |= 2;
}

void CInputManager::FreeKey(const char* name)
{
	KeyPair* pKey = m_keyList.Find(name);

	if (pKey)
		pKey->flag &= ~2;
}

void CInputManager::BlockKey()
{
	KeyPair* pKey = m_keyList.Start();
	while (pKey)
	{
		pKey->flag |= 2;
		pKey = m_keyList.Next();
	}
}

void CInputManager::FreeKey()
{
	KeyPair* pKey = m_keyList.Start();
	while (pKey)
	{
		pKey->flag &= ~2;
		pKey = m_keyList.Next();
	}
}

void CInputManager::BlockMouse()
{
	m_mouseFlag |= 2;

	for (int i = 0; i < 3; ++i)
		m_button[i].bBlock = true;
}

void CInputManager::FreeMouse()
{
	m_mouseFlag &= ~2;

	for (int i = 0; i < 3; ++i)
		m_button[i].bBlock = false;
}

void CInputManager::EraseAllList()
{
	KeyPair* pKey = m_keyList.Start();

	while (pKey)
	{
		delete pKey;
		pKey = m_keyList.Next();
	}
	m_keyList.Reset();
}

void CInputManager::SetFakeInput(eButton button)
{
	if (m_bSwapButton)
	{
		if (button == LEFT_BUTTON)
			button = RIGHT_BUTTON;
		else if (button == RIGHT_BUTTON)
			button = LEFT_BUTTON;
	}

	m_button[button].bFake = true;
}

void CInputManager::SetFakeInput(const char* name, bool bDown)
{
	KeyPair* pKey = m_keyList.Find(name);

	if (pKey)
	{
		pKey->bFake = true;

		if (bDown)
		{
			pKey->state = 1;
			pKey->oldState = 0;
		}

		if (pKey->type == 3)
			pKey->eventTime = timeGetTime();
	}
}

void CInputManager::SwapMouseButton(bool bSwap)
{
	m_bSwapButton = bSwap;
}
