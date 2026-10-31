#pragma once

#include <exdispid.h>

#include "anotherhand.h"

class WOverlay;

extern ATL::_ATL_FUNC_INFO WindowClosingInfo;
extern ATL::_ATL_FUNC_INFO DownloadInfo;

class CBrowserView : public ATL::CWindowImpl<CBrowserView, ATL::CAxWindow>,
					 public WTL::CMessageFilter,
					 public WTL::CIdleHandler,
					 public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
					 public ATL::IDispEventSimpleImpl<37, CBrowserView,
						 &DIID_DWebBrowserEvents2>
{
public:
	DECLARE_WND_SUPERCLASS(NULL, ATL::CAxWindow::GetWndClassName())

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnIdle();

	BEGIN_MSG_MAP(CBrowserView)
	MESSAGE_HANDLER(WM_DESTROY, OnDestory)
	END_MSG_MAP()

	BEGIN_COM_MAP(CBrowserView)
	END_COM_MAP()

	BEGIN_SINK_MAP(CBrowserView)

	SINK_ENTRY_INFO(37, DIID_DWebBrowserEvents2, DISPID_WINDOWCLOSING,
		OnWindowClosing, &WindowClosingInfo)

	SINK_ENTRY_INFO(37, DIID_DWebBrowserEvents2, DISPID_DOWNLOADBEGIN,
		OnDownloadBegin, &DownloadInfo)
	SINK_ENTRY_INFO(37, DIID_DWebBrowserEvents2, DISPID_DOWNLOADCOMPLETE,
		OnDownloadComplete, &DownloadInfo)
	END_SINK_MAP()

	LRESULT OnDestory(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void __stdcall OnWindowClosing(VARIANT_BOOL bIsChildWindow,
		VARIANT_BOOL* bCancel);
	void __stdcall OnDownloadBegin();
	void __stdcall OnDownloadComplete();

	int m_downloadCount;
};

template <class Base>
class CComObjectStack2 : public ATL::CComObjectStack<Base>
{
public:
	CComObjectStack2() { }
	virtual ~CComObjectStack2() { }

	virtual ULONG __stdcall AddRef() { return 1; }
	virtual ULONG __stdcall Release() { return 1; }
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject)
	{
		return _InternalQueryInterface(iid, ppvObject);
	}
};

// JP 983: layout medido no construtor 00873500 (operator new 0x170 em 00a728ad), destrutor 00872870, Init 008663c0,
// PrepareOpen 00870be0, Process 00872a60, Close 00868de0 (out/F3.5/s103). Sem CHandOwner (o 983 so tem a vptr do BaseObject)
// e sem m_pageType (o tipo de pagina do 983 e o m_webKind983 +0x120).
class CBrowser : public WSingleton<CBrowser>
{
public:
	enum eWebPage
	{
		WEBPAGE_NORMAL,
		WEBPAGE_SELFCERTIFY,
	};

	CBrowser();
	virtual ~CBrowser();

	void Init(HINSTANCE hInstance, HWND hWndParent, const RECT& rect);
	// 645: o 983 so abre pelo OpenWeb983 (sem funcao propria; inline)
	void Open(const char* url, bool bResetScreenSize, bool bWindowed) { InitializePostArgument(); m_bWindowed = bWindowed; PrepareOpen(url, bResetScreenSize); }
	void OpenPost(const char* url, bool bResetScreenSize, bool bWindowed) { m_bWindowed = bWindowed; PrepareOpen(url, bResetScreenSize); }
	void OpenWeb983(const char* url, bool bResetScreenSize, int kind); // 00870d90 (0xa1 com o tipo da pagina)
	void SetPopup983(bool bPopup, bool bSelfCertify, const char* caption,
		const float* size); // 00870e60 (sem chamador direto no 983)
	void Close();
	void Process(float elapsed);
	void Display();
	void Refresh();
	BOOL PreTranslateMessage(MSG* pMsg);
	void DrawCloseButton(tagDRAWITEMSTRUCT* pDis);
	void DrawBgButton(tagDRAWITEMSTRUCT* pDis);
	void InvalidateButtons983(); // 008664f0
	BOOL IsKeyDown983(int key); // 008666b0 (-1 = WM_KEYDOWN guardado)
	bool IsAltDown983(); // 008666f0
	bool IsCtrlDown983(); // 00866730

	void InitializePostArgument();
	void AddPostArgument(const char* key, const char* value);

	// 645: tipo de pagina; no 983 o "SELFCERTIFY" e a janela solta centrada (+0x124/+0x125)
	void SetPagetype(eWebPage type)
	{
		float size[2] = { 0.0f, 0.0f };
		SetPopup983(type == WEBPAGE_SELFCERTIFY, true, NULL, size);
	}
	int GetWebKind983() { return m_webKind983; }
	void SetStyle983(DWORD dwStyle) { m_dwStyle = dwStyle; }

protected:
	void PrepareOpen(const char* url, bool bResetScreenSize);
	std::string GetPostArgument() const;

	HINSTANCE m_hInstance; // +0x28
	HWND m_hWndParent; // +0x2c
	RECT m_rect; // +0x30
	bool m_bOpen; // +0x40
	DWORD m_dwOpenTime; // +0x44
	int m_reserved50; // +0x48
	int m_count; // +0x4c
	std::string m_url; // +0x50
	std::string m_waitMsg; // +0x6c
	int m_oldWindowMode; // +0x88
	int m_oldFillMode; // +0x8c
	HWND m_hWndCloseBtn983; // +0x90 ("closebtn", id 0x89)
	HWND m_hWndBgBtn983; // +0x94 ("bgbtn", id 0x8a)
	HANDLE m_hCloseBmp983; // +0x98 (bitmap 123)
	WOverlay* m_pLogo983; // +0x9c ("Jp_PangYa_Logo.png")
	bool m_bWindowed; // +0xa0
	bool m_bResetScreenSize; // +0xa1

public:
	bool IsOpen() { return m_spBrowser != NULL; }

	void ResetCount() { m_count = 0; }

protected:
	HWND m_hWndBrowser; // +0xa4
	CComObjectStack2<CBrowserView> m_view; // +0xa8

public:
	void SetWaitMsg(const char* msg) { m_waitMsg = msg; }

protected:
	ATL::CComQIPtr<IWebBrowser2> m_spBrowser; // +0x10c
	DWORD m_dwStyle; // +0x110
	std::list<std::pair<std::string, std::string> > m_postArgument; // +0x114
	int m_webKind983; // +0x120 (2..0xe; 0xc = nenhuma)
	bool m_bSelfCertify983; // +0x124
	bool m_bPopup983; // +0x125 (janela solta, sem logo nem botoes)
	std::string m_caption983; // +0x128
	float m_popupW983; // +0x144
	float m_popupH983; // +0x148
	bool m_bVisible983; // +0x14c
	int m_savedBindMouse983; // +0x150 (copia de COption gBindMouseCursor)
	MSG m_lastMsg983; // +0x154 (PreTranslateMessage guarda de message em diante)
};
