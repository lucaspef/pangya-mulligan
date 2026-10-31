#include "browser.h"
#include "vc10_layout.h"
#include "clientsetting.h"
#include "projectg.h"
#include "../shared/localize.h"
#include "mathconsts.h"

ILFILLB1

extern HWND g_hwnd;
extern WInputDev* g_mouse;
extern WInputDev* g_ime;
extern WInputDev* g_keyboard;

#include "../../include/packets/browserpackets.h"
#include "lobbytask.h"
#include "msg983.h"
#include "mousecursor.h"

#include "woverlay.h"
#include "wfont.h"
#include "chatmsg.h"

// JP 983: layout do CBrowser medido (sizeof 0x170; out/F3.5/s103)
C_ASSERT_VC10(sizeof(CBrowser) == 0x170);

/*
 *
 * It was really, really hard to make the IL balance line up for this file.
 * It turns out that the line numbers have a subtle impact on the IL offsets.
 * So this comment is, uhh, "load-bearing", to borrow a word from our AI overlords.
 *
 */

BOOL CBrowserView::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return FALSE;

	return (BOOL)SendMessage(WM_FORWARDMSG, 0, (LPARAM)pMsg);
}

BOOL CBrowserView::OnIdle()
{
	return FALSE;
}

LRESULT CBrowserView::OnDestory(UINT uMsg, WPARAM wParam, LPARAM lParam,
	BOOL& bHandled)
{
	if (CBrowser::Instance()->IsOpen())
	{
		CBrowser::Instance()->Close();
	}

	return 1;
}

void __stdcall CBrowserView::OnWindowClosing(VARIANT_BOOL bIsChildWindow,
	VARIANT_BOOL* bCancel)
{
	USES_CONVERSION;

	*bCancel = VARIANT_TRUE;
	if (CBrowser::Instance()->IsOpen())
	{
		CBrowser::Instance()->Close();
	}
}

void __stdcall CBrowserView::OnDownloadBegin()
{
	m_downloadCount++;
}

void __stdcall CBrowserView::OnDownloadComplete()
{
	if (m_downloadCount > 0)
		m_downloadCount--;
}

// 00873500
CBrowser::CBrowser()
{
	m_bOpen = false;
	m_dwOpenTime = 0xffffffff;
	m_reserved50 = 0;
	m_count = 0;
	m_hInstance = NULL;
	m_hWndParent = NULL;

	m_hWndCloseBtn983 = NULL;
	m_hWndBgBtn983 = NULL;
	m_pLogo983 = NULL;
	m_hWndBrowser = NULL;

	m_dwStyle = 0;

	sOption opt = *COption::Instance()->GetOption();

	m_popupW983 = 460.0f;
	m_popupH983 = 470.0f;
	m_oldWindowMode = opt.vWindowMode;
	m_oldFillMode = opt.vFillMode;
	m_bWindowed = true;
}

// 00872870
CBrowser::~CBrowser()
{
	if (m_spBrowser)
	{
		m_view.DispEventUnadvise(m_spBrowser);

		m_spBrowser.Release();
		if (IsOpen() || m_bOpen == true)
		{
			m_view.DestroyWindow();
		}
	}

	if (m_hWndCloseBtn983)
		DestroyWindow(m_hWndCloseBtn983);
	if (m_hWndBgBtn983)
		DestroyWindow(m_hWndBgBtn983);
	if (m_hCloseBmp983)
		DeleteObject(m_hCloseBmp983);
	if (g_resrcmng && m_pLogo983)
	{
		g_resrcmng->Release(m_pLogo983);
		m_pLogo983 = NULL;
	}
}

// 008663c0: no 983 o navegador e filho da janela do jogo, com uma barra embaixo (botao de fechar + fundo)
void CBrowser::Init(HINSTANCE hInstance, HWND hWndParent, const RECT& rect)
{
	m_hInstance = hInstance;
	m_hWndParent = hWndParent;
	m_rect = rect;
	m_bSelfCertify983 = false;
	m_bPopup983 = false;
	m_bVisible983 = true;

	if (!m_hWndCloseBtn983)
	{
		m_hWndCloseBtn983 = CreateWindowEx(0, "button", "closebtn",
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BS_OWNERDRAW,
			m_rect.right - 0x7a, m_rect.bottom - 0x2f, 0x7a, 0x2a, hWndParent,
			(HMENU)0x89, hInstance, NULL);
		m_hCloseBmp983 = LoadImage(m_hInstance, MAKEINTRESOURCE(123),
			IMAGE_BITMAP, 0x7a, 0x2a, LR_SHARED);
	}
	if (!m_hWndBgBtn983)
	{
		m_hWndBgBtn983 = CreateWindowEx(0, "button", "bgbtn",
			WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | BS_OWNERDRAW,
			m_rect.left, m_rect.bottom - 0x2f, m_rect.right - m_rect.left, 0x2f,
			m_hWndParent, (HMENU)0x8a, m_hInstance, NULL);
	}
	m_savedBindMouse983 = 0;
}

// 00868de0
void CBrowser::Close()
{
	m_dwStyle = 0;
	if (m_spBrowser)
	{
		sOption opt = *COption::Instance()->GetOption();

		// o 983 grava direto no COption+0x1e8 (gBindMouseCursor); GetOption() devolve o proprio m_opt
		sOption* pLive = const_cast<sOption*>(COption::Instance()->GetOption());
		if (m_savedBindMouse983 == 0)
			pLive->gBindMouseCursor = 0;

		if (m_bResetScreenSize)
		{
			COption::Instance()->vApplyGameScreenSize();
		}

		if (m_bWindowed &&
			(opt.vWindowMode != m_oldWindowMode ||
				opt.vFillMode != m_oldFillMode))
		{
			opt.vWindowMode = m_oldWindowMode;
			opt.vFillMode = m_oldFillMode;
			COption::Instance()->ApplyChange(opt, m_bResetScreenSize);
		}

		g_input->SetActive(true);

		if (pLive->gBindMouseCursor == 1 || m_oldWindowMode == 0)
			g_mouse->InitDevice(m_hWndParent, true);
		else
			g_mouse->InitDevice(m_hWndParent, false);

		// 983: esconde o cursor do Windows de novo, com o contador em CMouseCursorHelper+0x34
		CMouseCursorHelper* cursor = g_pMouseCursorHelper983;
		cursor->m_showCount = ShowCursor(FALSE);
		if (cursor->m_showCount < 0)
		{
			if (cursor->m_showCount < -1)
				cursor->HideCountFix(1);
		}
		else
		{
			do
			{
				cursor->m_showCount = ShowCursor(FALSE);
			} while (cursor->m_showCount != -1);
		}

		m_view.DispEventUnadvise(m_spBrowser);

		m_spBrowser.Release();
		if (m_bOpen == true)
		{
			m_view.DestroyWindow();
		}

		m_hWndBrowser = NULL;
		m_bOpen = false;
		m_dwOpenTime = 0xffffffff;

		SetFocus(m_hWndParent);
	}

	m_bSelfCertify983 = false;
	m_bPopup983 = false;
	if (m_hWndCloseBtn983)
		SetWindowLong(m_hWndCloseBtn983, GWL_STYLE,
			GetWindowLong(m_hWndCloseBtn983, GWL_STYLE) & ~WS_VISIBLE);
	if (m_hWndBgBtn983)
		SetWindowLong(m_hWndBgBtn983, GWL_STYLE,
			GetWindowLong(m_hWndBgBtn983, GWL_STYLE) & ~WS_VISIBLE);

	g_audio->SetBGMVolumeInBGMArea(1.0f);

	SetCurrentDirectory(g_executeDirectory);

	// 983: primeiro o envio do tipo de pagina (00868320), depois o tipo vira 0xc e vao 0x3d sempre e 0x8c com o
	// conteudo 0x16; o 0x72 do 645 (raspadinha) nao existe no 983
	SendWebPageClose983(m_webKind983);
	m_webKind983 = 0xc;
	{
		WSendPacket packet((enumClientPacket)0x3d);
		packet.Send(TO_GAME);

		if (IsLocalContent((localContentType_t)0x16 /*983: era 0x15; medido*/))
		{
			WSendPacket packet((enumClientPacket)0x8c);
			packet.Send(TO_GAME);
		}
	}
}

void CBrowser::Refresh()
{
}



// 00866530: guarda a mensagem (de message em diante) para IsKeyDown983/IsAltDown983/IsCtrlDown983 da janela solta
BOOL CBrowser::PreTranslateMessage(MSG* pMsg)
{
	if (!m_bOpen)
		return FALSE;

	m_lastMsg983.message = pMsg->message;
	m_lastMsg983.wParam = pMsg->wParam;
	m_lastMsg983.lParam = pMsg->lParam;
	m_lastMsg983.time = pMsg->time;
	m_lastMsg983.pt = pMsg->pt;

	return ((WTL::CMessageFilter&)m_view).PreTranslateMessage(pMsg);
}

// 008666b0
BOOL CBrowser::IsKeyDown983(int key)
{
	if (key == -1)
		return m_lastMsg983.message == WM_KEYDOWN;

	return g_keyboard->GetState(2, key);
}

// 008666f0
bool CBrowser::IsAltDown983()
{
	if (m_bPopup983)
		return m_lastMsg983.message == WM_SYSKEYDOWN;

	return IsKeyDown983(0x1c) || IsKeyDown983(0x1d);
}

// 00866730
bool CBrowser::IsCtrlDown983()
{
	if (m_bPopup983)
	{
		if (!IsKeyDown983(-1))
			return false;
		return m_lastMsg983.wParam == VK_CONTROL;
	}

	return IsKeyDown983(0x1e) || IsKeyDown983(0x1f);
}

// 008664f0
void CBrowser::InvalidateButtons983()
{
	if (m_bPopup983)
		return;

	if (m_hWndCloseBtn983)
		InvalidateRect(m_hWndCloseBtn983, NULL, FALSE);
	if (m_hWndBgBtn983)
		InvalidateRect(m_hWndBgBtn983, NULL, FALSE);
}

// 00872a60: no 983 o navegador nasce filho da janela do jogo (WS_CHILD) acima da barra de 0x2f com os dois botoes; a janela
// solta do 645 (WS_POPUP) so fica para o modo m_bPopup983
void CBrowser::Process(float elapsed)
{
	if (!m_bOpen)
		return;

	if (m_dwOpenTime < 0xffffffff)
	{
		WVideoDev* video = g_view->GetVideoDevice();

		if (video && !(video->GetRenderCount() & 1) &&
			GetTickCount() - m_dwOpenTime > 200)
		{
			m_dwOpenTime = 0xffffffff;

			if (m_spBrowser == NULL)
			{
				RECT rc = { 0, 0, (int)g_view->GetWidth(), (int)g_view->GetHeight() };

				if (m_bPopup983)
				{
					m_hWndBrowser = m_view.Create(m_hWndParent, rc, "about:blank",
						m_dwStyle | WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
							WS_CAPTION | WS_SYSMENU);
					m_view.SetWindowText(K2L_Compatibility(m_caption983.c_str()));

					RECT rcView;
					m_view.GetWindowRect(&rcView);
					RECT rcMain;
					GetWindowRect(g_hwnd, &rcMain);

					UINT flags = 0;
					int width = (int)m_popupW983;
					int height = (int)m_popupH983;
					HWND hWndInsertAfter = HWND_TOP;
					if (m_bSelfCertify983)
					{
						width = (int)g_view->GetWidth();
						height = (int)g_view->GetHeight();
						hWndInsertAfter = HWND_NOTOPMOST;
						flags = SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;
					}

					rcView.left = rcMain.left + (rcMain.right - rcMain.left) / 2 -
						(width - rcView.left) / 2;
					rcView.top = rcMain.top + (rcMain.bottom - rcMain.top) / 2 -
						(height - rcView.top) / 2;
					rcView.right = rcView.left + width;
					rcView.bottom = rcView.top + height;

					m_view.SetWindowPos(hWndInsertAfter, &rcView, flags);
				}
				else
				{
					int offX = (int)((g_view->GetWidth() - 800.0) * 0.5);
					int offY = (int)((g_view->GetHeight() - 600.0) * 0.5);

					if (m_hWndCloseBtn983)
					{
						MoveWindow(m_hWndCloseBtn983, offX + 0x2a6, offY + 0x229, 0x7a, 0x2a, TRUE);
						SetWindowLong(m_hWndCloseBtn983, GWL_STYLE,
							GetWindowLong(m_hWndCloseBtn983, GWL_STYLE) | WS_VISIBLE);
						InvalidateRect(m_hWndCloseBtn983, NULL, FALSE);
						UpdateWindow(m_hWndCloseBtn983);
					}
					if (m_hWndBgBtn983)
					{
						MoveWindow(m_hWndBgBtn983, m_rect.left + offX, m_rect.bottom - 0x2f + offY,
							m_rect.right - m_rect.left, 0x2f, TRUE);
						SetWindowLong(m_hWndBgBtn983, GWL_STYLE,
							GetWindowLong(m_hWndBgBtn983, GWL_STYLE) | WS_VISIBLE);
						InvalidateRect(m_hWndBgBtn983, NULL, FALSE);
						UpdateWindow(m_hWndBgBtn983);
					}

					rc.bottom -= 0x2f;
					m_hWndBrowser = m_view.Create(m_hWndParent, rc, "about:blank",
						m_dwStyle | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
							WS_CLIPCHILDREN);
				}

				m_view.m_downloadCount = 0;
				m_view.QueryControl(&m_spBrowser);
				m_view.DispEventAdvise(m_spBrowser);

				CComVariant vEmpty;

				if (!m_postArgument.empty())
				{
					VARIANT vHeaders = { 0 };
					VARIANT vPostData = { 0 };
					CString strHeaders(
						"Content-Type: application/x-www-form-urlencoded\r\n");
					vHeaders.vt = VT_BSTR;
					vHeaders.bstrVal = strHeaders.AllocSysString();
					std::string postData = GetPostArgument();
					SAFEARRAY* psa =
						SafeArrayCreateVector(VT_UI1, 0, postData.length());
					if (psa)
					{
						void* pData;
						SafeArrayAccessData(psa, &pData);
						memcpy(pData, postData.c_str(), postData.length());
						SafeArrayUnaccessData(psa);

						vPostData.vt = VT_ARRAY | VT_UI1;
						vPostData.parray = psa;
					}

					m_spBrowser->Navigate(CComBSTR(m_url.c_str()), &vEmpty, &vEmpty,
						&vPostData, &vHeaders);
					SafeArrayDestroy(psa);
					SysFreeString(vHeaders.bstrVal);
				}
				else
				{
					m_spBrowser->Navigate(CComBSTR(m_url.c_str()), &vEmpty, &vEmpty,
						&vEmpty, &vEmpty);
				}

				g_input->SetActive(false);
				g_mouse->InitDevice(m_hWndParent, false);
				ShowCursor(TRUE);
			}
		}
	}

	if (!m_bPopup983 && g_keyboard->GetState(2, 0xe) && m_hWndParent)
		InvalidateButtons983();

	if (m_view.m_hWnd)
		ShowWindow(m_view.m_hWnd, m_bVisible983 ? SW_SHOW : SW_HIDE);
}

// 00867760: fundo branco, logo do JP no centro e o aviso do F1 (a janela solta do 645 fica com a caixa escura)
void CBrowser::Display()
{
	if (m_bOpen && m_dwOpenTime != 0xffffffff)
	{
		WRect rect(0, 0, g_view->GetWidth(), g_view->GetHeight());

		if (m_bPopup983)
		{
			WOverlay::DrawBox(g_view, rect, 0, 0x88404040, 0.001f);
			return;
		}

		WOverlay::DrawBox(g_view, rect, 0, 0xffffffff, 0.001f);

		if (m_pLogo983)
		{
			float w = (float)m_pLogo983->GetWidth();
			float h = (float)m_pLogo983->GetHeight();
			WRect dst(g_view->GetWidth() * 0.5 - w * 0.5,
				g_view->GetHeight() * 0.5 - h * 0.5, w, h);
			m_pLogo983->Render(g_view, WRect(0, 0, 1.0f, 1.0f), dst, 0, 0xffffffff, 0.5f, 0);
		}

		// 00d33458: "a tela nao aparece? aperte F1" (coreano)
		const char* text = K2L_Compatibility("\310\255\270\351\300\314 \263\252\277\300\301\366 \276\312\300\273 \266\247\264\302 F1 \305\260\270\246 \264\255\267\257\301\326\274\274\277\344");
		WFont* pFont = CChatMsg::Instance()->GetMaskedFont(); // 983: CChatMsg+0x34
		float textWidth = pFont->GetTextWidthOld983(g_view, text);
		pFont->Print(g_view, (float)(400.0 - textWidth * 0.5), 450.0f, text, 0x80000, 0xff0000ff, 0);
		pFont->Flush(g_view);
	}
}

// 00866590 (WM_DRAWITEM do "closebtn")
void CBrowser::DrawCloseButton(tagDRAWITEMSTRUCT* pDis)
{
	if (m_bPopup983 || !m_hWndBrowser || !pDis || !pDis->hDC ||
		pDis->hwndItem != m_hWndCloseBtn983 ||
		!(GetWindowLong(m_hWndCloseBtn983, GWL_STYLE) & WS_VISIBLE))
		return;

	HDC hdc = CreateCompatibleDC(pDis->hDC);
	if (!hdc)
		return;

	HGDIOBJ hOld = SelectObject(hdc, m_hCloseBmp983);
	if (hOld)
	{
		BitBlt(pDis->hDC, pDis->rcItem.left, pDis->rcItem.top, pDis->rcItem.right,
			pDis->rcItem.bottom, hdc, 0, 0, SRCCOPY);
		SelectObject(hdc, hOld);
	}
	DeleteDC(hdc);
}

// 00866640 (WM_DRAWITEM do "bgbtn")
void CBrowser::DrawBgButton(tagDRAWITEMSTRUCT* pDis)
{
	if (m_bPopup983 || !m_hWndBrowser || !pDis || !pDis->hDC ||
		pDis->hwndItem != m_hWndBgBtn983 ||
		!(GetWindowLong(m_hWndBgBtn983, GWL_STYLE) & WS_VISIBLE))
		return;

	HBRUSH hbr = CreateSolidBrush(0xb4b9f3);
	if (!hbr)
		return;

	FillRect(pDis->hDC, &pDis->rcItem, hbr);
	DeleteObject(hbr);
}

// 00870be0
void CBrowser::PrepareOpen(const char* url, bool bResetScreenSize)
{
	sOption* pLive = const_cast<sOption*>(COption::Instance()->GetOption());
	m_savedBindMouse983 = pLive->gBindMouseCursor;
	if (!pLive->gBindMouseCursor || !COption::Instance()->vIsWindowed())
		pLive->gBindMouseCursor = 1;

	if (!m_bPopup983 && !m_pLogo983)
		m_pLogo983 = g_resrcmng->GetOverlay("Jp_PangYa_Logo.png", 0);

	m_bResetScreenSize = bResetScreenSize;
	if (m_bOpen)
		return;

	sOption opt = *COption::Instance()->GetOption();
	m_oldFillMode = opt.vFillMode;
	m_oldWindowMode = opt.vWindowMode;
	if (m_bWindowed)
	{
		if (!opt.vWindowMode || opt.vFillMode)
		{
			opt.vWindowMode = 1;
			opt.vFillMode = 0;
			COption::Instance()->ApplyChange(opt, false);
		}
	}

	m_bOpen = true;
	m_url = url;
	m_dwOpenTime = GetTickCount();
	SetWaitMsg(K2L_Compatibility("\267\316\265\371 \301\337\300\324\264\317\264\331..."));
}

// 00870d90
void CBrowser::OpenWeb983(const char* url, bool bResetScreenSize, int kind)
{
	g_input->SetMousePoint(400.0f, 325.0f);
	g_ime->SetState(0xd, 0);

	WSendPacket packet((enumClientPacket)0xa1);
	packet.Encode1((unsigned char)kind);
	packet.Send(TO_GAME);

	m_webKind983 = kind;
	m_bVisible983 = true;
	PrepareOpen(url, bResetScreenSize);
}

// 00870e60
void CBrowser::SetPopup983(bool bPopup, bool bSelfCertify, const char* caption,
	const float* size)
{
	m_bPopup983 = bPopup;
	if (size[0] == 0.0f && size[1] == 0.0f)
	{
		m_popupW983 = 460.0f;
		m_popupH983 = 470.0f;
	}
	else
	{
		m_popupW983 = size[0];
		m_popupH983 = size[1];
	}
	m_bSelfCertify983 = bPopup && bSelfCertify;
	if (caption)
		m_caption983 = caption;
	else
		m_caption983 = "\306\316\276\337";
}

void CBrowser::InitializePostArgument()
{
	m_postArgument.clear();
}

void CBrowser::AddPostArgument(const char* key, const char* value)
{
	m_postArgument.push_back(std::pair<std::string, std::string>(
		std::string(key), std::string(value)));
}

std::string CBrowser::GetPostArgument() const
{
	std::string result;
	std::list<std::pair<std::string, std::string> >::const_iterator it =
		m_postArgument.begin();
	while (it != m_postArgument.end())
	{
		result += (*it).first;
		result += "=";
		result += (*it).second;

		if (++it != m_postArgument.end())
			result += "&";
	}

	return result;
}

ILFILLB2

// ---- frente C (C33): envios do navegador do 983 por tipo de pagina (include/packets/browserpackets.h) ----

// 00870d90 (parte de rede)
void SendWebPageOpen983(int kind)
{
	g_input->SetMousePoint(400.0f, 325.0f);
	g_ime->SetState(0xd, 0);

	WSendPacket packet((enumClientPacket)0xa1);
	packet.Encode1((unsigned char)kind);
	packet.Send(TO_GAME);
}

// 00870d90
void OpenWebPage983(const char* url, bool bResetScreenSize, int kind)
{
	CBrowser::Instance()->OpenWeb983(url, bResetScreenSize, kind);
}

// 00868320
void SendWebPageClose983(int kind)
{
	const char* text = NULL;
	bool bRefresh = false;

	switch (kind)
	{
	case 2:
		{
			bRefresh = true;
			WSendPacket packet((enumClientPacket)0x9e);
			packet.Send(TO_GAME);
		}
		break;
	case 3:
		text = K2L_Compatibility("\306\316\276\337 \306\320\305\260\301\366\277\353 \276\306\300\314\305\333\300\272 \274\261\271\260\307\324\277\241\274\255 \310\256\300\316\307\330\301\326\274\274\277\344");
		bRefresh = true;
		break;
	case 4:
		{
			text = K2L_Compatibility("\276\306\300\314\305\333 \260\271\274\366\264\302 \276\306\300\314\305\333 \300\345\302\370 \310\255\270\351 \266\307\264\302 \276\306\300\314\305\333 \230\336\277\241\274\255 \310\256\300\316\307\330 \301\326\274\274\277\344");
			bRefresh = true;
			WSendPacket packet((enumClientPacket)0xa0);
			packet.Encode1(1);
			packet.Send(TO_GAME);
		}
		break;
	case 5:
		{
			bRefresh = true;
			WSendPacket packet((enumClientPacket)0xa2);
			packet.Send(TO_GAME);
		}
		break;
	case 6:
		{
			bRefresh = true;
			WSendPacket packet((enumClientPacket)0xa4);
			packet.Send(TO_GAME);
			WSendPacket packet2((enumClientPacket)0xa2);
			packet2.Send(TO_GAME);
		}
		break;
	case 8:
	case 9:
	case 10:
	case 0xe:
		bRefresh = true;
		break;
	case 0xb:
		{
			WSendPacket packet((enumClientPacket)0x3d);
			packet.Send(TO_GAME);
		}
		bRefresh = true;
		break;
	case 0xd:
		return;
	default:
		break;
	}

	if (!IS_KINDOF(CLobbyTask, AfxGetTask()))
		return;

	if (bRefresh)
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
	if (text)
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23), (int)text, 0xff000000, 0, 0, 0);
}
