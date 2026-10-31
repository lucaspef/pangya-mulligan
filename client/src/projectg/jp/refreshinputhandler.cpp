// JP 983: unidade 005f35c0–005f4ea0 — a RefreshInputHandler (entrada do jogo para a biblioteca Refresh). O WinProc do jogo empilha
// as mensagens de teclado (RFSceneManager::OnWndMsg983 → PushMessage983) e o Update983 do frame as repassa ao RFSceneManager junto
// com o mouse lido do CInputManager (botões, roda com espera de 0,2 s, movimento). O resto da faixa são cópias fora de linha do
// std::map<unsigned int, sRFKeyMessage983> (moldes) e de embutidas sem chamador (005f35c0, 005f35d0).
#include "refresh/rfscenemanager.h"
#include "inputmanager.h"

extern const WVector g_vZero983_f041c4; // 00f041c4

// 005f3600
void RefreshInputHandler::Initialize983()
{
}

// 005f38e0
void sRFMouseState983::Reset983()
{
	pos = g_vZero983_f041c4;
	wheel = 0.0f;
}

// 005f3900
unsigned int sRFMouseState983::Update983(CInputManager* pInput)
{
	unsigned int flags = 0;
	if (pInput)
	{
		bool bDouble = pInput->GetDoubleClick(LEFT_BUTTON);
		switch (pInput->GetButton(LEFT_BUTTON))
		{
		case 1:
			flags = bDouble ? RF_MOUSE_LDBLCLK983 : RF_MOUSE_LDOWN983;
			break;
		case 2:
			flags = RF_MOUSE_LUP983;
			break;
		}
		switch (pInput->GetButton(RIGHT_BUTTON))
		{
		case 1:
			flags |= RF_MOUSE_RDOWN983;
			break;
		case 2:
			flags |= RF_MOUSE_RUP983;
			break;
		}
		switch (pInput->GetButton(MIDDLE_BUTTON))
		{
		case 1:
			flags |= RF_MOUSE_MDOWN983;
			break;
		case 2:
			flags |= RF_MOUSE_MUP983;
			break;
		}
		float z = pInput->GetMouseDelta().z;
		if (wheel != z)
		{
			wheel = z;
			flags |= RF_MOUSE_WHEEL983;
		}
		if (flags == 0)
		{
			WVector point = pInput->GetMousePoint();
			if (!WisEqual(pos, point, 1e-05f))
				pos = point;
			return RF_MOUSE_MOVE983;
		}
	}
	return flags;
}

// 005f3a00
void RefreshInputHandler::ProcessMouse983(float delta)
{
	if (m_bBlocked)
	{
		m_mouse.Update983(g_input);
		return;
	}
	unsigned int flags = m_mouse.Update983(g_input);
	if (flags & RF_MOUSE_LDOWN983)
		RFSceneManager::Instance()->InjectLButtonDown(m_mouse.pos.x, m_mouse.pos.y);
	else if (flags & RF_MOUSE_LUP983)
		RFSceneManager::Instance()->InjectLButtonUp(m_mouse.pos.x, m_mouse.pos.y);
	else if (flags & RF_MOUSE_LDBLCLK983)
		RFSceneManager::Instance()->InjectLButtonDblClick(m_mouse.pos.x, m_mouse.pos.y);
	else if (flags & RF_MOUSE_RDOWN983)
		RFSceneManager::Instance()->InjectRButtonDown(m_mouse.pos.x, m_mouse.pos.y);
	else if (flags & RF_MOUSE_RUP983)
		RFSceneManager::Instance()->InjectRButtonUp(m_mouse.pos.x, m_mouse.pos.y);
	else if (flags & RF_MOUSE_WHEEL983)
	{
		int wheel = (int)m_mouse.GetWheel983(); // o 983 passa o float por [ebp-4] antes do _ftol2 (D: 6 instruções)
		if (wheel && m_wheelWait == 0.0f)
		{
			RFSceneManager::Instance()->InjectMouseWheel(wheel, m_mouse.pos.x, m_mouse.pos.y);
			m_wheelWait = m_wheelDelay;
		}
	}
	if (flags & RF_MOUSE_MOVE983)
		RFSceneManager::Instance()->InjectMouseMove(m_mouse.pos.x, m_mouse.pos.y);
	if (m_wheelWait > 0.0f)
	{
		m_wheelWait -= delta;
		if (m_wheelWait < 0.0f)
			m_wheelWait = 0.0f;
	}
}

// 005f3b90
int RefreshInputHandler::GetButtons983()
{
	if (g_input)
		return m_mouse.Update983(g_input);
	return 0;
}

// 005f3f20
sRFMouseState983::sRFMouseState983()
{
	Reset983();
}

// 005f3f30
sRFMouseState983::~sRFMouseState983()
{
	Reset983();
}

// 005f4870
void RefreshInputHandler::ProcessKeys983()
{
	if (m_bBlocked)
		return;
	for (std::map<unsigned int, sRFKeyMessage983>::iterator it = m_messages.begin(); it != m_messages.end(); ++it)
	{
		switch (it->first)
		{
		case WM_KEYDOWN:
			RFSceneManager::Instance()->InjectKeyDown(it->second.wParam, it->second.lParam);
			break;
		case WM_KEYUP:
			RFSceneManager::Instance()->InjectKeyUp(it->second.wParam, it->second.lParam);
			break;
		}
	}
}

// 005f4b00
void RefreshInputHandler::Update983(float delta)
{
	if (RFSceneManager::Instance())
	{
		ProcessKeys983();
		ProcessMouse983(delta);
		m_messages.clear();
		m_bClosed = false;
	}
}

// 005f4c30
RefreshInputHandler::~RefreshInputHandler()
{
}

// 005f4da0
RefreshInputHandler::RefreshInputHandler()
	: m_bClosed(false)
{
	m_wheelWait = m_wheelDelay = 0.2f;
	m_bBlocked = false;
}

// 005f4e60
void RefreshInputHandler::PushMessage983(unsigned int msg, WPARAM wParam, LPARAM lParam)
{
	m_cs.Enter();
	if (!m_bClosed)
		m_messages[msg] = sRFKeyMessage983(wParam, lParam);
	m_cs.Leave();
}
