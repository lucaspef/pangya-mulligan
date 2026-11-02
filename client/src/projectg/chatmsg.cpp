#include "minatl.h"
#include "chatmsg.h"
#include "golftask.h"
#include "lobbytask.h"
#include "avatar_task.h"
#include "clientsetting.h"
#include "actor.h"
#include "fresh.h"
#include "frdesktop.h"
#include "frwndmanager.h"

// projectg.cpp (983 00a62d40 / 00a62cb0)
bool ConsumeClipboardPaste983();
const char* GetClipboardText983();
#include "jp/uiframesetmanager.h"
#include "wfont.h"
#include "wresrcmng.h"
#include "../shared/token.h"
#include "../shared/localize.h"

// HACK: 91.0f is defined in tutorialmain.cpp as a function-local static aliased to
// __real@42b60000; using the literal here would emit a competing COMDAT that all the
// other objects would bind to.
extern "C" const float rb_real_42b60000;
#pragma comment(linker, "/alternatename:_rb_real_42b60000=__real@42b60000")
#pragma comment(linker, "/include:__real@42b60000")

extern Fresh* g_pFresh;
extern WView* g_view;
extern WResourceManager* g_resrcmng;
extern unsigned long g_CurrentTime;

class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

bool IsMyTurn(bool check);
extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

// HACK: the original has an unreferenced float with a dynamic initializer
// (.bss + .CRT$XCU storing 10.0f); a const float built from another const
// float is not a constant expression for this compiler and reproduces it.
static const float s_fUnused = BAR_START - 130.0f;

// 983 (0099fc50, 009a052d, 009a0d8b): alfa da UI = GolfDoc+0xdc (m_uiAlpha); +0xcc do 645 cai no m_pTeam do 983

CChatMsg::CChatMsg()
	: m_bPrinted(false),
	  m_bCaretMove(true),
	  m_nMaxBytes(0),
	  m_fMaxWidth(1000.0f),
	  m_pMaskedFont(NULL),
	  m_bToggle(true)
{
	for (int i = 0; i < 3; i++)
		m_pChatWindow[i] = NULL;
	m_iState = 0;
	Init();
}

CChatMsg::~CChatMsg()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_pChatWindow[i])
		{
			g_resrcmng->Release(m_pChatWindow[i]);
			m_pChatWindow[i] = NULL;
		}
	}
	if (g_resrcmng && m_pMaskedFont)
	{
		g_resrcmng->Release(m_pMaskedFont);
		m_pMaskedFont = NULL;
	}
}

void CChatMsg::Init()
{
	InitFont();
	m_bCompInProgress = false;
	m_bActive = false;
	m_bInsert = true;
	m_iPrevCode = 0;
	m_iCurChar = 0;
	m_iNumChars = 0;
	m_iCurByte = 0;
	m_iNumBytes = 0;
	m_iNumCopyChars = 0;
	m_iNumCopyBytes = 0;
	m_iNumSelBytes = 0;
	m_szChat[0] = 0;
	m_szComp[0] = 0;
	m_pChatWindow[0] = g_resrcmng->GetOverlay("{chat_l.png", 0);
	m_pChatWindow[1] = g_resrcmng->GetOverlay("{chat_m.png", 0);
	m_pChatWindow[2] = g_resrcmng->GetOverlay("{chat_r.png", 0);
	m_rcChatWinSrc[0] = WRect(0.0f, 0.0f, 0.54296875f, 1.0f);
	m_rcChatWinSrc[1] = WRect(0.0f, 0.0f, 1.0f, 1.0f);
	m_fVel = 3000.0f;
	m_rcChatWinSrc[2] = WRect(0.0f, 0.0f, 1.0f, 1.0f);
	m_bBackupUiAlpha = 0xff;
	m_bShowCaret = true;
	m_fCaretTime = 0.0f;
	ResetUiPosition();
}

void CChatMsg::ResetUiPosition()
{
	float x, y;
	WRect rcClip;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		x = 70.0f;
		y = g_view->GetHeight() - 106.0f;
		rcClip.x = x;
		rcClip.y = y;
	}
	else
	{
		// 983 009a0930: ponto p_chat_window_area (arredondado para baixo); sem ele x = 91 e y = BarY + 9
		x = 91.0f;
		y = COption::Instance()->gGetBar_Y() + 9.0f;
		// o CChatMsg nasce antes do CUiFrameSetManager (login): sem gerente, cai no x/y fixos (d477-1 caiu aqui com this = NULL)
		CUiFrameSetManager* pUi = CUiFrameSetManager::Instance();
		CUiFrameSetManager::NamedPoint* pt = pUi ? pUi->FindNamedPoint("p_chat_window_area") : NULL;
		if (pt)
		{
			WPoint pos;
			pt->Calc(CUiFrameSetManager::Size(g_view->GetWidth() / 640.0, g_view->GetHeight() / 480.0), pos);
			x = (float)floor(pos.x);
			y = (float)floor(pos.y);
		}
		rcClip.x = x;
		rcClip.y = y;
	}
	// HACK: no code, only stops the scheduler from sinking the stores below
	_ReadWriteBarrier();
	rcClip.w = 482.0f;
	rcClip.h = 32.0f;
	for (int i = 0; i < 3; i++)
	{
		if (m_pChatWindow[i])
			m_pChatWindow[i]->SetClippingArea(&rcClip);
	}
	m_rcChatWinDst[0] = WRect(m_iState == 3 ? x : x - 462.0f, y, 139.0f, 32.0f);
	m_rcChatWinDst[2] = WRect(0.0f, y, 64.0f, 32.0f);
	m_rcChatWinDst[1] = WRect(m_rcChatWinDst[0].w + m_rcChatWinDst[0].x, y,
		462.0f - m_rcChatWinDst[0].w - m_rcChatWinDst[2].w, 32.0f);
	m_rcChatWinDst[2].x = m_rcChatWinDst[1].w + m_rcChatWinDst[1].x;
}

void CChatMsg::Reset()
{
	g_ime->Reset();
	m_bCompInProgress = false;
	m_iPrevCode = 0;
	m_iCurChar = 0;
	m_iNumChars = 0;
	m_iCurByte = 0;
	m_iNumBytes = 0;
	m_szComp[0] = 0;
	m_szChat[0] = 0;
}

void CChatMsg::ClearHistory()
{
	Doc()->m_chatLineList.clear();
	Doc()->m_chatHistory.clear();
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(NULL, M983(0x63), 0, 0, 0, 0, 0);
	if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(NULL, M983(0x63), 0, 0, 0, 0, 0);
}

void CChatMsg::InitFont()
{
	// 983 0099f1a0: "fnt_japan_12.wft" (00d268cc); o 645 carregava fnt_<dotum>_12.wft (outra fonte, maior)
	m_pMaskedFont = g_resrcmng->GetFixedFont("fnt_japan_12.wft");
	if (m_pMaskedFont)
		m_pMaskedFont->SetMode(WFont::MASKED);
}

void CChatMsg::SetAlwaysActive()
{
	// HACK: dead dependency on a later-compiled function so that the emission
	// order matches the original
	if (0)
		ProcessResultString();
	g_ime->SetActive(true);
	CInputManager::Instance()->SetInputMode(CInputManager::INPUT_CHAT);
	// HACK: m_rcChatWinDst[1].x = m_rcChatWinDst[0].w + m_rcChatWinDst[0].x (91.0f). The constant is owned
	// by tutorialmain.cpp (see rb_real_42b60000); a global cannot be hoisted above the stores below and
	// is loaded first by the x87 codegen, so the sum is computed here through a volatile read.
	float x1 = *(volatile float*)&m_rcChatWinDst[0].w + rb_real_42b60000;
	m_bActive = true;
	m_bShowCaret = true;
	m_fCaretTime = 0.0f;
	m_iState = 3;
	m_rcChatWinDst[0].x = BAR_START - 49.0f;
	m_rcChatWinDst[1].x = x1;
	m_rcChatWinDst[2].x = m_rcChatWinDst[1].x + m_rcChatWinDst[1].w;
	IActor* pActor = AfxGetTask()->GetActor("GolfRule");
	if (pActor)
		pActor << MsgObject(NULL, M983(0x62), 1, 0, 0, 0, 0);
	pActor = AfxGetTask()->GetActor("AvatarChat");
	if (pActor)
		pActor << MsgObject(NULL, M983(0x62), 1, 0, 0, 0, 0);
	if (GOLFDOC())
		m_bBackupUiAlpha = GOLFDOC()->m_uiAlpha;
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (GOLFDOC() && IsMyTurn(true))
			GOLFDOC()->m_uiAlpha = 0x80;
	}
	else if (GOLFDOC())
		GOLFDOC()->m_uiAlpha = 0x80;
}

bool CChatMsg::HasBeenPrinted()
{
	return !m_bToggle || m_bPrinted;
}

void CChatMsg::ReplaceBackupUiAlpha()
{
	if (GOLFDOC())
		m_bBackupUiAlpha = GOLFDOC()->m_uiAlpha;
}

void CChatMsg::SetActive(bool active, bool b2, bool b3)
{
	if (m_bActive != active)
	{
		m_bActive = active;
		g_ime->SetActive(active);
		CInputManager::Instance()->SetInputMode(
			active ? CInputManager::INPUT_CHAT : CInputManager::INPUT_GAME);
		if (!m_bActive && b2)
			Reset();
	}
}

void CChatMsg::ToggleInsertMode()
{
	m_bInsert = !m_bInsert;
}

void CChatMsg::Process(const float deltaTime)
{
	// 983 (009a03b0): o IME do JP entrega cadeias (GetState(3, 0) = texto, nao codigo de tecla): so as versoes _JP;
	// as coreanas ficam sem chamador (inseriam o ponteiro como caractere: lixo no campo ID do login)
	if (m_bActive)
	{
		ProcessResultString_JP();
		ProcessCompString_JP();
		if (m_iState == 3)
		{
			m_fCaretTime += 0.01f;
			if (m_fCaretTime > (m_bShowCaret ? 0.4f : 0.1f))
			{
				m_bShowCaret = !m_bShowCaret;
				m_fCaretTime = 0.0f;
			}
		}
		// 983: Ctrl+V cola o texto da area de transferencia do Windows caractere a caractere (ate 0x3ff)
		if (m_bActive && ConsumeClipboardPaste983())
		{
			const char* text = GetClipboardText983();
			unsigned int len = strlen(text);
			if (m_iCurByte + len > 0x3ff)
				len = 0x3ff - m_iCurByte;
			for (unsigned int i = 0; i < len; i++)
				WriteCharacter(text[i]);
		}
	}
}

void CChatMsg::ProcessWindow(const float deltaTime)
{
	// D732: 983 009a0db0 = fim do deslize no x do ponto p_chat_window_area (009a0930, o mesmo do ResetUiPosition);
	// o 91 fixo do 645 cortava a barra a esquerda do recorte em resolucao maior (dono: chat cortado no Practice 1600x900)
	float x;
	if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		x = BAR_START - 70.0f;
	else
	{
		x = BAR_START - 49.0f;
		CUiFrameSetManager* pUi = CUiFrameSetManager::Instance();
		CUiFrameSetManager::NamedPoint* pt = pUi ? pUi->FindNamedPoint("p_chat_window_area") : NULL;
		if (pt)
		{
			WPoint pos;
			pt->Calc(CUiFrameSetManager::Size(g_view->GetWidth() / 640.0, g_view->GetHeight() / 480.0), pos);
			x = (float)floor(pos.x);
		}
	}
	if (m_iState == 2 || m_iState == 1)
	{
		if (m_iState == 2)
		{
			m_rcChatWinDst[0].x -= deltaTime * 3000.0f;
			float limit = x - 462.0f;
			if (m_rcChatWinDst[0].x < limit)
			{
				m_rcChatWinDst[0].x = limit;
				m_iState = 0;
			}
		}
		else if (m_iState == 1)
		{
			m_rcChatWinDst[0].x += deltaTime * m_fVel;
			if (m_rcChatWinDst[0].x > x)
			{
				m_fVel -= deltaTime * 105000.0f;
				if (m_rcChatWinDst[0].x > x + 20.0f)
					m_rcChatWinDst[0].x = x + 20.0f;
			}
			else if (m_fVel < 0.0f)
			{
				m_rcChatWinDst[0].x = x;
				m_iState = 3;
				IActor* pActor = AfxGetTask()->GetActor("GolfRule");
				if (pActor)
					pActor << MsgObject(NULL, M983(0x62), 1, 0, 0, 0, 0);
				pActor = AfxGetTask()->GetActor("AvatarChat");
				if (pActor)
					pActor << MsgObject(NULL, M983(0x62), 1, 0, 0, 0, 0);
			}
		}
		m_rcChatWinDst[1].x = m_rcChatWinDst[0].w + m_rcChatWinDst[0].x;
		m_rcChatWinDst[2].x = m_rcChatWinDst[1].x + m_rcChatWinDst[1].w;
	}
	m_bPrinted = false;
	if (m_bActive)
		g_input->GetButton(LEFT_BUTTON);
}

void CChatMsg::ProcessMacro(const float deltaTime)
{
	// 983 009a1a10: teste do IME, nao CRealMyRoomTask (nao existe no 983)
	if (g_ime->GetState(4, 0) == 0 &&
		!g_input->Get("LALT", true) && !g_input->Get("RALT", true) &&
		OnlinePlay())
	{
		std::string str;
		CTask* task = AfxGetTask();
		for (int i = 0; i < 8; i++)
		{
			if (g_input->GetDown(MakeStr("FUNC_%d", i + 1), true))
			{
				if (IS_KINDOF(CGolfTask, task) ||
					IS_KINDOF(ntAvatarChatTask, task))
				{
					Doc()->m_chatManager.SendChatMessage(
						COption::Instance()->mGetMacroText(i), NULL, false);
					if (!WNetworkSystem::Instance()->IsConnected(
							WNetworkSystem::NET_GAME))
					{
						str = Doc()
								  ->m_userInfo[GOLFDOC()->m_currentPlayer]
								  .info.sNick;
						str += " : ";
						str += COption::Instance()->mGetMacroText(i);
						AddChatMsg(str, 0xffffcc00, false, false);
					}
				}
				else
				{
					Doc()->m_chatManager.SendChatMessage2(
						COption::Instance()->mGetMacroText(i), NULL, false);
					if (!WNetworkSystem::Instance()->IsConnected(
							WNetworkSystem::NET_GAME))
					{
						str = Doc()->m_myInfo.info.sNick;
						str += " : ";
						str += COption::Instance()->mGetMacroText(i);
						AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x3),
							(int)str.c_str(), 0xffffcc00, 0, 0, 0);
					}
				}
			}
		}
	}
}

void CChatMsg::ProcessResultString()
{
	m_iConsolCode = 0;
	int code = g_ime->GetState(3, 0);
	if (code == 0x5c)
		return;
	if (Custom_IsLeadByte((unsigned char)code)) // 983 0099fde0: Shift-JIS (o 645: bit 7)
	{
		if (!Custom_IsLeadByte((unsigned char)g_ime->GetState(3, -2)))
			m_bCompInProgress = false;
		InsertWCharacter(code);
		m_iNumSelChars = 0;
		m_iNumSelBytes = 0;
		m_fCaretTime = 0.0f;
		m_bShowCaret = true;
		return;
	}
	if (code == 0)
		return;
	switch (code)
	{
	case 8:
		m_iConsolCode = code;
		OnBackSpace();
		break;
	case 4:
		m_iConsolCode = code;
		if (m_szChat[0])
			OnMoveLeft();
		else if (m_iState == 3 &&
			!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
			Close(true, true);
		break;
	case 5:
		m_iConsolCode = code;
		if (m_szChat[0])
			OnMoveRight();
		else if (m_iState == 3 &&
			!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
			Close(true, true);
		break;
	case 6:
		m_iConsolCode = code;
		if (!m_szChat[0] && m_iState == 3 &&
			!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
			Close(true, true);
		break;
	case 7:
		m_iConsolCode = code;
		if (!m_szChat[0] && m_iState == 3 &&
			!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
			Close(true, true);
		break;
	case 26:
		OnMoveEnd();
		break;
	case 12:
		OnMoveHome();
		break;
	case 1:
		m_iConsolCode = code;
		OnDelete();
		break;
	default:
		if (code >= 0x20 && code <= 0x7f)
		{
			if (m_iNumSelBytes)
				DeleteSelected();
			WriteCharacter(code);
		}
		break;
	}
	m_fCaretTime = 0.0f;
	m_bShowCaret = true;
}

void CChatMsg::ProcessResultString_JP()
{
	m_iConsolCode = 0;
	const char* str = (const char*)g_ime->GetState(3, 0);
	if (!str)
		return;
	int len = strlen(str);
	int i = 0;
	while (i < len)
	{
		char c = str[i];
		// 983 (009a0117): byte inicial de Shift-JIS; o katakana de meia largura (0xa0..0xdf) vai para o WriteCharacter
		if (Custom_IsLeadByte(c))
		{
			if (*(const char*)g_ime->GetState(3, -2) == 0)
				m_bCompInProgress = false;
			int w = MAKEWORD(str[i], str[i + 1]);
			i += 1;
			InsertWCharacter(w);
			m_iNumSelChars = 0;
			m_iNumSelBytes = 0;
			m_fCaretTime = 0.0f;
			m_bShowCaret = true;
		}
		else if (c == 0)
		{
		}
		else if (c == '\\')
			continue;
		else
		{
			switch (c)
			{
			case 8:
				m_iConsolCode = c;
				OnBackSpace();
				break;
			case 4:
				m_iConsolCode = c;
				if (m_szChat[0])
					OnMoveLeft();
				else if (m_iState == 3 &&
					!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
					Close(true, true);
				break;
			case 5:
				m_iConsolCode = c;
				if (m_szChat[0])
					OnMoveRight();
				else if (m_iState == 3 &&
					!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
					Close(true, true);
				break;
			case 6:
				m_iConsolCode = c;
				if (!m_szChat[0] && m_iState == 3 &&
					!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
					Close(true, true);
				break;
			case 7:
				m_iConsolCode = c;
				if (!m_szChat[0] && m_iState == 3 &&
					!g_pFresh->manager->GetDesktop()->FindChildForm("emoticon"))
					Close(true, true);
				break;
			case 26:
				OnMoveEnd();
				break;
			case 12:
				OnMoveHome();
				break;
			case 1:
				m_iConsolCode = c;
				OnDelete();
				break;
			default:
			{
				int uc = (unsigned char)c;
				if ((uc >= 0x20 && uc < 0x80) || (uc >= 0xa0 && uc < 0xe0))
				{
					if (m_iNumSelBytes)
						DeleteSelected();
					WriteCharacter(uc);
				}
				break;
			}
			}
			m_fCaretTime = 0.0f;
			m_bShowCaret = true;
		}
		i++;
	}
}

void CChatMsg::ProcessCompString()
{
	m_iCompCode = g_ime->GetState(3, -2);
	if (m_iCompCode == 0x5c)
	{
		m_iCompCode = 0;
		return;
	}
	if (m_iCompCode != 0)
	{
		if (!(m_pMaskedFont->GetTextWidthOld983(g_view, m_szChat) > m_fMaxWidth) &&
			m_iNumBytes - m_iNumSelBytes <= m_nMaxBytes - 2)
		{
			if (m_iCompCode != m_iPrevCode)
			{
				m_iPrevCode = m_iCompCode;
				m_szComp[0] = (char)m_iCompCode;
				m_szComp[1] = (char)(m_iCompCode >> 8);
				m_szComp[2] = 0;
				if (!m_bCompInProgress)
				{
					m_bCompInProgress = true;
					if (m_iNumSelBytes)
						DeleteSelected();
					if (!m_bInsert && m_iCurChar < m_iNumChars)
						DeleteCharacter();
				}
				m_fCaretTime = 0.0f;
				m_bShowCaret = true;
			}
		}
		else
		{
			g_ime->Reset();
			if (m_iNumSelBytes)
				DeleteSelected();
			m_fCaretTime = 0.0f;
			m_bShowCaret = true;
		}
	}
	else
	{
		if (m_bCompInProgress)
		{
			memset(m_szComp, 0, 2);
			m_bCompInProgress = false;
		}
		m_iPrevCode = 0;
	}
}

void CChatMsg::ProcessCompString_JP()
{
	const char* comp = (const char*)g_ime->GetState(3, -2);
	if (comp)
	{
		strcpy(m_szComp, comp);
		if (m_szComp[0])
		{
			void* pAttr = (void*)g_ime->GetState(3, -3);
			void* pCls = (void*)g_ime->GetState(3, -4);
			memcpy(m_bCompAttr, pAttr, sizeof(m_bCompAttr));
			memcpy(m_dwCompCls, pCls, sizeof(m_dwCompCls));
			m_dwCompAttrLen = g_ime->GetState(3, -5);
			m_dwCompClsLen = g_ime->GetState(3, -6);
			m_pMaskedFont->GetTextWidthOld983(g_view, m_szChat);
			m_pMaskedFont->GetTextWidthOld983(g_view, m_szComp);
			if (!m_bCompInProgress)
			{
				m_bCompInProgress = true;
				if (m_iNumSelBytes)
					DeleteSelected();
				if (!m_bInsert && m_iCurChar < m_iNumChars)
					DeleteCharacter();
			}
			m_bShowCaret = true;
			m_fCaretTime = 0.0f;
		}
		else if (m_bCompInProgress)
			m_bCompInProgress = false;
	}
}

void CChatMsg::ProcessEditKey(const float deltaTime)
{
	if (m_bCaretMove)
	{
		if (g_input->GetDown("HOME", true))
		{
			if (g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true))
			{
				m_iSelByteS = m_iCurByte;
				m_iSelByteE = 0;
				m_iNumSelBytes = m_iSelByteS - m_iSelByteE;
				m_iSelCharS = m_iCurChar;
				m_iSelCharE = 0;
				m_iNumSelChars = m_iSelCharS - m_iSelCharE;
			}
			m_iCurByte = 0;
			m_iCurChar = 0;
		}
		if (g_input->GetDown("END", true))
		{
			if (g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true))
			{
				m_iSelByteS = m_iCurByte;
				m_iSelByteE = m_iNumBytes;
				m_iNumSelBytes = m_iSelByteE - m_iSelByteS;
				m_iSelCharS = m_iCurChar;
				m_iSelCharE = m_iNumChars;
				m_iNumSelChars = m_iSelCharE - m_iSelCharS;
			}
			m_iCurByte = m_iNumBytes;
			m_iCurChar = m_iNumChars;
		}
		if (g_input->Get("LCONTROL", true))
		{
			if (g_input->GetDown("COPY", true))
			{
				if (m_iNumSelBytes)
					CopySelected();
			}
			else if (g_input->GetDown("CUT", true))
			{
				if (m_iNumSelBytes)
				{
					CopySelected();
					DeleteSelected();
				}
			}
			else if (g_input->GetDown("PASTE", true))
			{
				if (m_iNumSelBytes)
					DeleteSelected();
				PasteCopied();
			}
		}
	}
}

void CChatMsg::OnMoveLeft()
{
	if (m_bCaretMove && m_iCurChar > 0)
	{
		m_iCurByte -= m_abWide[m_iCurChar - 1] + 1;
		m_iCurChar--;
	}
}

void CChatMsg::OnMoveRight()
{
	if (m_bCaretMove && m_iCurChar < m_iNumChars)
	{
		m_iCurByte += m_abWide[m_iCurChar] + 1;
		m_iCurChar++;
	}
}

void CChatMsg::OnMoveEnd()
{
	m_iCurByte = m_iNumBytes;
	m_iCurChar = m_iNumChars;
}

void CChatMsg::OnMoveHome()
{
	m_iCurByte = 0;
	m_iCurChar = 0;
}

void CChatMsg::OnDelete()
{
	if (m_iNumSelBytes)
		DeleteSelected();
	else if (m_iCurChar < m_iNumChars)
		DeleteCharacter();
}

void CChatMsg::OnBackSpace()
{
	if (m_iNumSelBytes)
	{
		DeleteSelected();
		return;
	}
	if (m_iCurChar)
	{
		int len = m_abWide[m_iCurChar - 1] + 1;
		memmove(&m_szChat[m_iCurByte - len], &m_szChat[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		memmove(&m_szExt[m_iCurByte - len], &m_szExt[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		m_iNumBytes -= len;
		m_iCurByte -= len;
		memmove(&m_abWide[m_iCurChar - 1], &m_abWide[m_iCurChar],
			m_iNumChars - m_iCurChar);
		m_iNumChars--;
		m_iCurChar--;
	}
}

void CChatMsg::DeleteCharacter()
{
	int len = m_abWide[m_iCurChar] + 1;
	memmove(&m_szChat[m_iCurByte], &m_szChat[m_iCurByte + len],
		m_iNumBytes - m_iCurByte - len);
	memmove(&m_szExt[m_iCurByte], &m_szExt[m_iCurByte + len],
		m_iNumBytes - m_iCurByte - len);
	m_iNumBytes -= len;
	memmove(&m_abWide[m_iCurChar], &m_abWide[m_iCurChar + 1],
		m_iNumChars - m_iCurChar - 1);
	m_iNumChars--;
}

void CChatMsg::DeleteSelected()
{
	m_iNumBytes -= m_iNumSelBytes;
	m_iNumChars -= m_iNumSelChars;
	if (m_iSelByteS < m_iSelByteE)
	{
		memmove(&m_szChat[m_iSelByteS], &m_szChat[m_iSelByteE], m_iNumSelBytes);
		memmove(&m_szExt[m_iSelByteS], &m_szExt[m_iSelByteE], m_iNumSelBytes);
		memmove(&m_abWide[m_iSelCharS], &m_abWide[m_iSelCharE], m_iNumSelChars);
		m_iCurByte = m_iSelByteS;
		m_iCurChar = m_iSelCharS;
		m_iNumSelChars = 0;
		m_iNumSelBytes = 0;
	}
	else
	{
		memmove(&m_szChat[m_iSelByteE], &m_szChat[m_iSelByteS], m_iNumSelBytes);
		memmove(&m_szExt[m_iSelByteE], &m_szExt[m_iSelByteS], m_iNumSelBytes);
		memmove(&m_abWide[m_iSelCharE], &m_abWide[m_iSelCharS], m_iNumSelChars);
		m_iCurByte = m_iSelByteE;
		m_iCurChar = m_iSelCharE;
		m_iNumSelChars = 0;
		m_iNumSelBytes = 0;
	}
}

void CChatMsg::CopySelected()
{
	if (m_iSelByteS < m_iSelByteE)
	{
		memmove(m_szCopied, &m_szChat[m_iSelByteS], m_iNumSelBytes);
		memmove(m_szCopiedExt, &m_szExt[m_iSelByteS], m_iNumSelBytes);
		memmove(m_abCopied, &m_abWide[m_iSelCharS], m_iNumSelChars);
	}
	else
	{
		memmove(m_szCopied, &m_szChat[m_iSelByteE], m_iNumSelBytes);
		memmove(m_szCopiedExt, &m_szExt[m_iSelByteE], m_iNumSelBytes);
		memmove(m_abCopied, &m_abWide[m_iSelCharE], m_iNumSelChars);
	}
	m_iNumCopyBytes = m_iNumSelBytes;
	m_iNumCopyChars = m_iNumSelChars;
}

void CChatMsg::PasteCopied()
{
	float w1 = m_pMaskedFont->GetTextWidthOld983(g_view, m_szChat);
	float w2 = m_pMaskedFont->GetTextWidthOld983(g_view, m_szCopied);
	if (!(w1 + w2 > m_fMaxWidth) && m_iNumBytes + m_iNumCopyBytes < m_nMaxBytes)
	{
		memmove(&m_szChat[m_iCurByte + m_iNumCopyBytes], &m_szChat[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		memmove(&m_szExt[m_iCurByte + m_iNumCopyBytes], &m_szExt[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		memmove(&m_abWide[m_iCurChar + m_iNumCopyChars], &m_abWide[m_iCurChar],
			m_iNumChars - m_iCurChar);
		memmove(&m_szChat[m_iCurByte], m_szCopied, m_iNumCopyBytes);
		memmove(&m_szExt[m_iCurByte], m_szCopiedExt, m_iNumCopyBytes);
		memmove(&m_abWide[m_iCurChar], m_abCopied, m_iNumCopyChars);
		m_iNumBytes += m_iNumCopyBytes;
		m_iNumChars += m_iNumCopyChars;
		m_iCurByte += m_iNumCopyBytes;
		m_iCurChar += m_iNumCopyChars;
	}
}

void CChatMsg::InsertWCharacter(int code)
{
	if (!(m_pMaskedFont->GetTextWidthOld983(g_view, m_szChat) > m_fMaxWidth) &&
		m_iNumBytes < m_nMaxBytes)
	{
		memmove(&m_szChat[m_iCurByte + 2], &m_szChat[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		memmove(&m_szExt[m_iCurByte + 2], &m_szExt[m_iCurByte],
			m_iNumBytes - m_iCurByte);
		memmove(&m_abWide[m_iCurChar + 1], &m_abWide[m_iCurChar],
			m_iNumChars - m_iCurChar);
		m_szChat[m_iCurByte] = (char)code;
		m_szChat[m_iCurByte + 1] = (char)(code >> 8);
		m_szExt[m_iCurByte] = 1;
		m_szExt[m_iCurByte + 1] = 0;
		m_abWide[m_iCurChar] = 1;
		m_iNumChars++;
		m_iNumBytes += 2;
		m_iCurByte += 2;
		m_iCurChar++;
		m_szChat[m_iNumBytes] = 0;
	}
}

void CChatMsg::WriteCharacter(int code)
{
	if (!(m_pMaskedFont->GetTextWidthOld983(g_view, m_szChat) > m_fMaxWidth) &&
		m_iNumBytes < m_nMaxBytes)
	{
		if (m_bInsert)
		{
			memmove(&m_szChat[m_iCurByte + 1], &m_szChat[m_iCurByte],
				m_iNumBytes - m_iCurByte);
			memmove(&m_szExt[m_iCurByte + 1], &m_szExt[m_iCurByte],
				m_iNumBytes - m_iCurByte);
			memmove(&m_abWide[m_iCurChar + 1], &m_abWide[m_iCurChar],
				m_iNumChars - m_iCurChar);
			m_iNumBytes++;
			m_iNumChars++;
		}
		else
		{
			if (m_iNumSelBytes)
			{
				memmove(&m_szChat[m_iCurByte + 1], &m_szChat[m_iCurByte],
					m_iNumBytes - m_iCurByte);
				memmove(&m_szExt[m_iCurByte + 1], &m_szExt[m_iCurByte],
					m_iNumBytes - m_iCurByte);
				memmove(&m_abWide[m_iCurChar + 1], &m_abWide[m_iCurChar],
					m_iNumChars - m_iCurChar);
				m_iNumBytes++;
				m_iNumChars++;
			}
			if (m_iCurChar == m_iNumChars)
			{
				m_iNumBytes++;
				m_iNumChars++;
			}
			else if (m_abWide[m_iCurChar])
			{
				memmove(&m_szChat[m_iCurByte + 1], &m_szChat[m_iCurByte + 2],
					m_iNumBytes - m_iCurByte - 2);
				memmove(&m_szExt[m_iCurByte + 1], &m_szExt[m_iCurByte + 2],
					m_iNumBytes - m_iCurByte - 2);
				m_iNumBytes--;
			}
		}
		m_szChat[m_iCurByte] = (char)code;
		m_szExt[m_iCurByte] = 0;
		m_abWide[m_iCurChar] = 0;
		m_iCurByte++;
		m_iCurChar++;
		m_szChat[m_iNumBytes] = 0;
	}
}

void CChatMsg::PrintInChatWindow()
{
}

unsigned long CChatMsg::ConvertToUIColor(unsigned long color)
{
	switch (color)
	{
	case 0xff5bffb0:
		return 0xff009900;
	case 0xff00ff00:
		return 0xff00a117;
	case 0xff00c6ff:
		return 0xff0054ff;
	case 0xffd2a2ff:
		return 0xff8400ff;
	case 0xffff7878:
		return 0xffff0000;
	case 0xffffcc00:
		return 0xffbf5100;
	case 0xffffffff:
		return 0xff000000;
	}
	return color;
}

void CChatMsg::RemoveColor(const std::string& src, std::string& dst)
{
	char buf[1024];
	strncpy(buf, src.c_str(), 1023);
	cTokenV tok;
	tok.Init(buf, strlen(buf));
	char str[64] = "";
	strncpy(str, buf, 63);
	dst = "";
	char* p = NULL;
	bool ok;
	do
	{
		ok = tok.GetTokenFullSep(&p, "\\c", 2);
		dst += p;
		if (ok)
			tok.GetTokenFullSep((char**)NULL, "\\c", 2);
	} while (ok);
}

void CChatMsg::AddChatMsg(const std::string& msg, unsigned long color, bool b1,
	bool b2)
{
	m_lastMsgTime = g_CurrentTime;
	std::string str;
	if (b2)
		RemoveColor(msg, str);
	else
		str = msg;
	if (IS_KINDOF(CLobbyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(NULL, M983(0x3), (int)str.c_str(), color, 0, 0, 0);
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x60),
			(int)m_chatFacialUser.c_str(), (int)str.c_str(), 0, 0, 0);
	}
	else if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		bool bShowChat = GOLFDOC()->m_bShowChat;
		if (bShowChat || b1)
		{
			AfxGetTask()->GetActor("GolfRule")
				<< MsgObject(NULL, M983(0x64), (int)str.c_str(), color, 0, 0, 0);
		}
		AfxGetTask()->GetActor("Screen") << MsgObject(NULL, M983(0x64),
			(int)str.c_str(), ConvertToUIColor(color), 0, 0, 0);
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		if (GOLFDOC()->m_bShowChat || b1)
		{
			AfxGetTask()->GetActor("AvatarChat")
				<< MsgObject(NULL, M983(0x64), (int)str.c_str(), color, b1, 0, 0);
		}
	}
	sChatLine line;
	line.text = str;
	line.color = color;
	Doc()->m_chatLineList.push_back(line);
}

void CChatMsg::GetChatText_JP(std::string& front, std::string& comp,
	std::string& end, int maxBytes)
{
	// HACK: dead reference to an inline defined by an earlier object; it only
	// serves to reproduce the original function emission order
	if (0)
		IsActive();
	m_szChat[m_iNumBytes] = 0;
	// 983 (009a0550): byte inicial de Shift-JIS (Custom_IsLeadByte), nao o bit 7
	if (maxBytes == -1)
	{
		if (m_iCurByte < m_iNumBytes)
		{
			char c = m_szChat[m_iCurByte];
			m_szChat[m_iCurByte] = 0;
			front = m_szChat;
			m_szChat[m_iCurByte] = c;
			end = &m_szChat[m_iCurByte];
		}
		else
		{
			front = m_szChat;
			end = "";
		}
		comp = m_bCompInProgress ? m_szComp : "";
		return;
	}
	int i = 0;
	if (m_iCurByte < m_iNumBytes)
	{
		for (i = 0; i < maxBytes && i < m_iCurByte; i++)
		{
			if (Custom_IsLeadByte(m_szChat[i]))
				i++;
		}
	}
	else
	{
		for (i = 0; i < maxBytes && i < m_iNumBytes; i++)
		{
			if (Custom_IsLeadByte(m_szChat[i]))
				i++;
		}
	}
	char c = m_szChat[i];
	m_szChat[i] = 0;
	front = m_szChat;
	maxBytes -= i;
	m_szChat[i] = c;
	if (m_bCompInProgress && maxBytes > 0)
	{
		int len = strlen(m_szComp);
		int j = 0;
		for (j = 0; j < len; j++)
		{
			if (Custom_IsLeadByte(m_szComp[j]))
				j++;
		}
		c = m_szComp[j];
		m_szComp[j] = 0;
		comp = m_szComp;
		m_szComp[j] = c;
	}
	else
		comp = "";
	i = m_iCurByte;
	if (i < m_iNumBytes && maxBytes > 0)
	{
		int limit = maxBytes + i;
		for (; i < limit && i < m_iNumBytes; i++)
		{
			if (Custom_IsLeadByte(m_szChat[i]))
				i++;
		}
		c = m_szChat[i];
		m_szChat[i] = 0;
		end = &m_szChat[m_iCurByte];
		m_szChat[i] = c;
	}
	else
		end = "";
}

void CChatMsg::GetChatText(std::string& front, std::string& comp,
	std::string& end)
{
	m_szChat[m_iNumBytes] = 0;
	if (m_iCurByte < m_iNumBytes)
	{
		char c = m_szChat[m_iCurByte];
		m_szChat[m_iCurByte] = 0;
		front = m_szChat;
		m_szChat[m_iCurByte] = c;
		end = &m_szChat[m_iCurByte];
	}
	else
	{
		front = m_szChat;
		end = "";
	}
	if (m_bCompInProgress)
		comp = m_szComp;
	else
		comp = "";
}

void CChatMsg::GetCompositionString(sCompStr* pCompStr)
{
	pCompStr->pCompStr = m_szComp;
	pCompStr->pCompAttr = m_bCompAttr;
	pCompStr->pCompCls = m_dwCompCls;
	pCompStr->dwCompAttrLen = m_dwCompAttrLen;
	pCompStr->dwCompClsLen = m_dwCompClsLen;
}

void CChatMsg::SetChatText(const char* text, bool bKeepCaret)
{
	// HACK: dead reference to an inline defined by an earlier object; it only
	// serves to reproduce the original function emission order
	if (0)
		IsActive();
	if ((int)strlen(text) > m_nMaxBytes)
	{
		strcpy(m_szChat, text);
		m_nMaxBytes = m_iNumBytes = strlen(text);
	}
	else
	{
		strcpy(m_szChat, text);
		m_iNumBytes = strlen(m_szChat);
	}
	int byte = m_iCurByte;
	int chr = m_iCurChar;
	int nChars = 0;
	if (m_iNumBytes)
	{
		int i = 0;
		do
		{
			if (Custom_IsLeadByte(m_szChat[i])) // 983 0099f832: Shift-JIS (o 645: bit 7)
			{
				m_abWide[nChars] = 1;
				i++;
			}
			else
				m_abWide[nChars] = 0;
			nChars++;
			if (bKeepCaret && m_iCurByte > i)
			{
				byte = i;
				chr = nChars;
			}
			i++;
		} while (i < m_iNumBytes);
	}
	m_iNumChars = nChars;
	if (bKeepCaret)
	{
		m_iCurByte = m_iCurByte ? byte + 1 : 0;
		if (m_iCurByte > m_iNumBytes)
			m_iCurByte = m_iNumBytes;
		m_iCurChar = chr;
		if (m_iCurChar > nChars)
			m_iCurChar = nChars;
	}
	else
	{
		m_iCurChar = nChars;
		m_iCurByte = m_iNumBytes;
	}
}

void CChatMsg::DrawChatWindow()
{
	if (!IsClosed() && g_pFresh->show)
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_pChatWindow[i])
				m_pChatWindow[i]->Render(g_view, m_rcChatWinSrc[i],
					m_rcChatWinDst[i], 0, -1, 0.0f, 0);
		}
	}
}

void CChatMsg::PrintInScreen()
{
	m_bPrinted = true;
	if (g_CurrentTime - m_lastMsgTime > 10000)
	{
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
			AfxGetTask()->GetActor("GolfRule")
				<< MsgObject(NULL, M983(0x96), 0, 0, 0, 0, 0);
		m_lastMsgTime = g_CurrentTime;
	}
}

void CChatMsg::PrintOnChatRoom()
{
}

std::string CChatMsg::GetIncomingChatMsg() const
{
	std::string str = m_szChat;
	if (m_szComp[0]) // 983 009a12c3 (o 645: m_iCompCode)
		str += m_szComp;
	for (size_t i = 0; i < str.length(); i++)
	{
		if (str[i] != ' ')
			return str;
	}
	return str = "";
}

const char* CChatMsg::MakeShortID(const char* id, float width)
{
	static char buf[256];
	float ellipsis = GetMaskedFont()->GetTextWidthOld983(g_view, K2L_Compatibility("\xa1\xa6"));
	if (GetMaskedFont()->GetTextWidthOld983(g_view, id) > width)
	{
		float cur = 0.0f;
		int i = 0;
		for (;;)
		{
			if (Custom_IsLeadByte(id[i])) // 983 0099f940: Shift-JIS (o 645: bit 7)
			{
				static float wideWidth =
					GetMaskedFont()->GetTextWidthOld983(g_view, K2L_Compatibility("\xb0\xa1"));
				cur += wideWidth;
				if (width - ellipsis < cur)
					break;
				buf[i] = id[i];
				buf[i + 1] = id[i + 1];
				i += 2;
			}
			else
			{
				cur += GetMaskedFont()->GetTextWidthOld983(g_view, "A");
				if (width - ellipsis < cur)
					break;
				buf[i] = id[i];
				i++;
			}
		}
		buf[i] = 0;
		strcat(buf, K2L_Compatibility("\xa1\xa6"));
	}
	else
		strcpy(buf, id);
	return buf;
}

void CChatMsg::Open(bool b1, bool b2)
{
	// HACK: dead dependency (see SetAlwaysActive)
	if (0)
		Process(0.0f);
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->GetPlayingMode() == 2)
		return;
	SetActive(true, b1, b2);
	m_bShowCaret = true;
	m_fCaretTime = 0.0f;
	m_iState = 1;
	ReplaceBackupUiAlpha();
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		if (GOLFDOC() && IsMyTurn(true))
			GOLFDOC()->m_uiAlpha = 0x80;
	}
	else if (GOLFDOC())
		GOLFDOC()->m_uiAlpha = 0x80;
}

void CChatMsg::Close(bool b1, bool b2)
{
	SetActive(false, b1, b2);
	if (m_iState)
	{
		m_iState = 2;
		m_fVel = 3000.0f;
	}
	if (GOLFDOC())
		GOLFDOC()->m_uiAlpha = m_bBackupUiAlpha;
	IActor* pActor = AfxGetTask()->GetActor("GolfRule");
	if (pActor)
		pActor << MsgObject(NULL, M983(0x62), 0, 0, 0, 0, 0);
	pActor = AfxGetTask()->GetActor("AvatarChat");
	if (pActor)
		pActor << MsgObject(NULL, M983(0x62), 0, 0, 0, 0, 0);
}

bool CChatMsg::IME_IsCandWinOpened()
{
	return g_ime->GetState(5, 0) ? true : false;
}

bool CChatMsg::IME_IsCompDone()
{
	return g_ime->GetState(6, 0) ? true : false;
}

bool CChatMsg::IME_IsImeOpened()
{
	return g_ime->GetState(7, 0) ? true : false;
}

bool CChatMsg::IME_IsCompInProgress()
{
	return g_ime->GetState(4, 0) ? true : false;
}

bool CChatMsg::IME_IsAlphaNumericMode()
{
	return g_ime->GetState(10, 0) ? false : true;
}

void CChatMsg::IME_SetAlphaNumericMode()
{
	g_ime->SetAlphaNumericMode();
}

void CChatMsg::IME_SetOpenStatus(bool open)
{
	g_ime->SetOpenStatus(open);
}

tagCANDIDATELIST* const CChatMsg::IME_GetCandList()
{
	return (tagCANDIDATELIST*)g_ime->GetState(9, 0);
}
