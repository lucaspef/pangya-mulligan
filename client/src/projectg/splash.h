#pragma once

typedef BOOL(WINAPI* lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey,
	BYTE bAlpha, DWORD dwFlags);

class CSplash : public WSingleton<CSplash>
{
public:
	CSplash();
	CSplash(LPCTSTR lpszFileName, COLORREF colTrans);
	virtual ~CSplash();

	void ShowSplash();
	void HideSplash();
	int DoLoop();
	int CloseSplash();
	void SetText(char* text);
	DWORD SetBitmap(LPCTSTR lpszFileName);
	DWORD SetBitmap(HBITMAP hBitmap);
	bool SetTransparentColor(COLORREF col);
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam);

private:
	void Init();
	void OnPaint(HWND hwnd);
	bool MakeTransparent();
	HWND RegAndCreateWindow();
	void FreeResources();

	COLORREF m_colTrans;
	DWORD m_dwWidth;
	DWORD m_dwHeight;
	HBITMAP m_hBitmap;
	HWND m_hwnd;
	LPCTSTR m_lpszClassName;
	char m_szText[128];
	int m_textX;
	int m_textY;
	lpfnSetLayeredWindowAttributes m_pSetLayeredWindowAttributes;
	DWORD m_reserved;
};
