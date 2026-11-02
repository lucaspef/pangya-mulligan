// wime.cpp do JP 983. Base: device/wime/wime.cpp do 645; o IME foi refeito para o japonês (cadeia de composição
// com atributos e cláusulas, lista de candidatos, MSIMEService/MSIMEKeyMap).
#include "winput.inl"
#include "wime.h"

#include <string.h>
#include <new>

#include <mmsystem.h>

// MSIME (msime.h do IME japonês da Microsoft): mapa de teclas
#define RWM_SERVICE "MSIMEService"
#define RWM_KEYMAP "MSIMEKeyMap"
#define FID_MSIME_KMS_VERSION 1
#define FID_MSIME_KMS_INIT 2
#define FID_MSIME_KMS_TERM 3
#define FID_MSIME_KMS_SETKEYMAP 4
#define VERSION_ID_IME98 0x1000980
#define VERSION_ID_IME98A 0x100098a

struct IMEKMSINIT
{
	INT cbSize;
	HWND hWnd;
};
struct IMEKMSKEY
{
	DWORD dwStatus;
	DWORD dwCompStatus;
	DWORD dwVKEY;
	DWORD dwControl;
	WCHAR* pwszDscr;
};
struct IMEKMSKMP
{
	INT cbSize;
	HIMC hIMC;
	DWORD cMap;
	IMEKMSKEY* pMap;
};

LONG g_oldImeWndProc;

char* WIme::GetCompStr()
{
	return this->m_compStr;
}

char* WIme::GetCompAttr()
{
	return this->m_compAttr;
}

DWORD* WIme::GetCompClause()
{
	return this->m_compClause;
}

inline int WIme::IME_Enter()
{
	if (this->m_hWnd)
	{
		this->m_hIMC = ImmGetContext(this->m_hWnd);
		return this->m_hIMC != NULL;
	}
	return 0;
}

inline void WIme::Leave()
{
	ImmReleaseContext(this->m_hWnd, this->m_hIMC);
}

inline int WIme::Check()
{
	return !(this->m_property & IME_PROP_SPECIAL_UI) &&
		(this->m_property & IME_PROP_AT_CARET);
}

// byte inicial de Shift-JIS (0x80..0x9f, 0xe0..0xfc)
inline bool IsLeadByte(unsigned char c)
{
	if ((c >= 0x80 && c < 0xA0) || (c >= 0xE0 && c < 0xFD))
		return true;
	return false;
}

void WIme::ReleaseKeyMap()
{
	if (g_oldImeWndProc)
	{
		HWND hwndIme = ImmGetDefaultIMEWnd(this->m_hWnd);
		if (hwndIme)
		{
			if (this->m_msgKeyMap)
				SendMessageA(hwndIme, this->m_msgKeyMap, FID_MSIME_KMS_TERM, 0);
			SetWindowLongA(hwndIme, GWL_WNDPROC, g_oldImeWndProc);
		}
	}
	g_oldImeWndProc = 0;
	this->m_imeVersion = 0;
	this->m_msgKeyMap = 0;
}

static LRESULT CALLBACK ImeWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return CallWindowProcA((WNDPROC)g_oldImeWndProc, hwnd, msg, wparam, lparam);
}

void WIme::SetKeyMap(DWORD compStat)
{
	if (this->m_imeVersion == VERSION_ID_IME98 || this->m_imeVersion == VERSION_ID_IME98A)
	{
		HWND hwndIme = ImmGetDefaultIMEWnd(this->m_hWnd);
		if (hwndIme && this->m_msgKeyMap && IME_Enter())
		{
			IMEKMSKEY key[4];
			IMEKMSKMP kmp;
			key[0].dwStatus = 0;
			key[0].dwCompStatus = compStat;
			key[0].dwVKEY = VK_F5;
			kmp.cbSize = sizeof(kmp);
			kmp.hIMC = this->m_hIMC;
			kmp.cMap = 1;
			kmp.pMap = key;
			SendMessageA(hwndIme, this->m_msgKeyMap, FID_MSIME_KMS_SETKEYMAP, (LPARAM)&kmp);
			ImmReleaseContext(this->m_hWnd, this->m_hIMC);
		}
	}
}

void WIme::ClearData()
{
	this->m_nState &= 0xc;
	this->m_compStr[0] = 0;
	this->m_compAttr[0] = 0;
	this->m_compClause[0] = 0;
	this->m_compStrLen = 0;
	this->m_compAttrLen = 0;
	this->m_compClauseLen = 0;
	this->old_current = 0;
	this->current = 0;
	this->pos = 0;
}

void WIme::OnStartComposition(unsigned int dwCommand, long dwData)
{
	if (Check())
	{
		m_nState |= 1;
		current = 0;
		pos = 0;
	}
}

int WIme::OnSetContext(unsigned int dwCommand, long dwData)
{
	if (this->m_property & IME_PROP_AT_CARET)
	{
		dwData &= ~0x80000001;
	}

	return DefWindowProcA(this->m_hWnd, WM_IME_SETCONTEXT, dwCommand,
		dwData & ~(ISC_SHOWUICOMPOSITIONWINDOW | ISC_SHOWUIGUIDELINE | ISC_SHOWUIALLCANDIDATEWINDOW));
}

void WIme::OnEndComposition(unsigned int dwCommand, long dwData)
{
	if (Check())
	{
		this->m_compStr[0] = 0;
		this->m_compAttr[0] = 0;
		this->m_compClause[0] = 0;
		this->m_compStrLen = 0;
		this->m_compAttrLen = 0;
		this->m_compClauseLen = 0;
		this->m_nState = this->m_nState & ~1 | 4;
	}
}

inline void WIme::OnCompositionFull(unsigned int dwCommand, long dwData)
{
}

int WIme::OnNotify(unsigned int dwCommand, long dwData)
{
	switch (dwCommand)
	{
	case IMN_OPENCANDIDATE:
		SetKeyMap(6);
	case IMN_CHANGECANDIDATE:
		if (IME_Enter())
		{
			DWORD size = ImmGetCandidateListA(this->m_hIMC, 0, NULL, 0);
			if (size)
			{
				if (size > this->m_candSize)
				{
					if (this->m_candList)
					{
						delete[] this->m_candList;
						this->m_candList = NULL;
					}
					this->m_candList = (CANDIDATELIST*)new char[size];
					this->m_candSize = size;
				}
				ImmGetCandidateListA(this->m_hIMC, 0, this->m_candList, size);
				if (this->m_candList->dwSelection < this->m_candList->dwPageStart ||
					this->m_candList->dwSelection >= this->m_candList->dwPageStart + this->m_candList->dwPageSize)
				{
					this->m_candList->dwPageStart = this->m_candList->dwSelection / 9 * 9;
					if (this->m_candList->dwPageStart + 9 <= this->m_candList->dwCount)
						this->m_candList->dwPageSize = 9;
					else
						this->m_candList->dwPageSize = this->m_candList->dwCount - this->m_candList->dwPageStart;
				}
			}
			else
			{
				memset(this->m_candList, 0, sizeof(CANDIDATELIST));
			}
			ImmReleaseContext(this->m_hWnd, this->m_hIMC);
		}
		this->m_nState |= 2;
		return 1;
	case IMN_CLOSECANDIDATE:
		memset(this->m_candList, 0, sizeof(CANDIDATELIST));
		this->m_nState &= ~2;
		SetKeyMap(1);
		return 1;
	case IMN_SETOPENSTATUS:
		IME_Enter();
		if (ImmGetOpenStatus(this->m_hIMC))
			this->m_nState |= 8;
		else
			this->m_nState &= ~8;
		ImmReleaseContext(this->m_hWnd, this->m_hIMC);
		break;
	case IMN_SETCONVERSIONMODE:
		DWORD dwSenMode;
		IME_Enter();
		ImmGetOpenStatus(this->m_hIMC);
		ImmGetConversionStatus(this->m_hIMC, &this->m_dwConvMode, &dwSenMode);
		ImmReleaseContext(this->m_hWnd, this->m_hIMC);
		break;
	default:
		if (dwCommand >= 0x100 && dwCommand < 0x120 && (dwCommand & 0x1f) == IMN_PRIVATE)
			this->m_nState |= 4;
		break;
	}
	return 0;
}

inline void WIme::OnControl(unsigned int dwCommand, long dwData)
{
}

void WIme::PutString(char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
	{
		if (str[i] == '\\')
			continue;
		this->ringBuf[this->current++ & 0x3FF] = str[i];
		if (IsLeadByte(str[i]))
		{
			++i;
			this->ringBuf[this->current++ & 0x3FF] = str[i];
		}
	}
}

int WIme::GetResultString()
{
	static char str[128];
	DWORD dwBuf;
	if (IME_Enter())
	{
		dwBuf = ImmGetCompositionStringA(m_hIMC, GCS_RESULTSTR, NULL, 0);
		if (dwBuf > 0)
		{
			if (dwBuf > 127)
				dwBuf = 127;
			ImmGetCompositionStringA(m_hIMC, GCS_RESULTSTR, str, dwBuf);
			str[dwBuf] = 0;
			ProcessResultString(str);
		}
		Leave();
	}
	return 1;
}

int WIme::GetCompString(long flag)
{
	if (!IME_Enter())
	{
		return 0;
	}

	if (flag & GCS_COMPSTR)
	{
		this->m_compStrLen = ImmGetCompositionStringA(this->m_hIMC, GCS_COMPSTR, this->m_compStr,
			sizeof(this->m_compStr));
		this->m_compStr[this->m_compStrLen] = 0;
	}
	else
	{
		this->m_compStrLen = 0;
		this->m_compStr[0] = 0;
	}
	if (flag & GCS_COMPATTR)
	{
		this->m_compAttrLen = ImmGetCompositionStringA(this->m_hIMC, GCS_COMPATTR, this->m_compAttr,
			sizeof(this->m_compAttr));
	}
	else
	{
		this->m_compAttrLen = 0;
		this->m_compAttr[0] = 0;
	}
	if (flag & GCS_COMPCLAUSE)
	{
		this->m_compClauseLen = ImmGetCompositionStringA(this->m_hIMC, GCS_COMPCLAUSE, this->m_compClause,
			sizeof(this->m_compClause));
	}
	else
	{
		this->m_compClauseLen = 0;
		this->m_compClause[0] = 0;
		this->m_compClause[1] = 0;
	}
	ImmReleaseContext(this->m_hWnd, this->m_hIMC);

	return 1;
}

void WIme::ProcessResultString(char* str)
{
	this->PutString(str);
}

int WIme::GetState(int sort, int n)
{
	char* str = NULL;

	switch (sort)
	{
	case 3:
		switch (n)
		{
		case -1:
			return 1;
		case -2:
			if (Check())
				str = this->m_compStr;
			return (int)str;
		case -3:
			if (Check())
				str = this->m_compAttr;
			return (int)str;
		case -4:
			if (Check())
				str = (char*)this->m_compClause;
			return (int)str;
		case -5:
			return this->m_compAttrLen;
		case -6:
			return this->m_compClauseLen;
		}
		{
			int start = pos;
			if (start < current)
			{
				ringBuf[current] = 0;
				str = (char*)&ringBuf[start];
				current = 0;
				pos = 0;
				return (int)str;
			}
		}
		break;
	case 5:
		return (this->m_nState >> 1) & 1;
	case 4:
		return this->m_nState & 1;
	case 6:
		return (this->m_nState >> 2) & 1;
	case 7:
		return (this->m_nState >> 3) & 1;
	case 8:
		return (this->m_nState >> 4) & 1;
	case 9:
		return (int)this->m_candList;
	case 10:
		return this->m_dwConvMode;
	case 11:
		return HIWORD(this->m_property) & 1; // IME_PROP_AT_CARET
	}
	return 0;
}

void WIme::SetAlphaNumericMode()
{
	DWORD dwSenMode;

	IME_Enter();
	ImmGetConversionStatus(this->m_hIMC, &this->m_dwConvMode, &dwSenMode);
	ImmSetConversionStatus(this->m_hIMC, 0, dwSenMode);
	ImmGetConversionStatus(this->m_hIMC, &this->m_dwConvMode, &dwSenMode);
	ImmReleaseContext(this->m_hWnd, this->m_hIMC);
}

void WIme::SetOpenStatus(bool open)
{
	if (IME_Enter())
	{
		ImmSetOpenStatus(this->m_hIMC, open);
		this->m_nState &= ~8;
		ImmReleaseContext(this->m_hWnd, this->m_hIMC);
	}
}

bool WIme::IsAlphaNumericMode()
{
	DWORD dwSenMode;

	if (!this->m_hIMC)
	{
		IME_Enter();

		ImmGetConversionStatus(this->m_hIMC, &this->m_dwConvMode, &dwSenMode);
		ImmReleaseContext(this->m_hWnd, this->m_hIMC);
	}

	return this->m_dwConvMode == 0;
}

void WIme::Reset()
{
	this->SetActive(true);
	IME_Enter();
	ImmNotifyIME(this->m_hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
	ImmReleaseContext(this->m_hWnd, this->m_hIMC);
	this->ClearData();
}

void WIme::Update(unsigned long timeStamp)
{
	if (current != old_current)
	{
		old_current = current;
		bUpdated = true;
		dwLastInputTime = timeGetTime();
	}
	else
	{
		bUpdated = false;
	}
}

WIme::WIme()
{
	g_oldImeWndProc = 0;
	this->m_imeVersion = 0;
	this->m_msgKeyMap = 0;
	this->m_candList = 0;
	this->m_candSize = 0;
}

WIme::~WIme()
{
	if (this->m_candList)
	{
		delete this->m_candList;
		this->m_candList = NULL;
	}
	ReleaseKeyMap();
}

void WIme::InitKeyMap()
{
	HWND hwndIme = ImmGetDefaultIMEWnd(this->m_hWnd);
	if (hwndIme)
	{
		g_oldImeWndProc = SetWindowLongA(hwndIme, GWL_WNDPROC, (LONG)ImeWndProc);
		UINT msgService = RegisterWindowMessageA(RWM_SERVICE);
		this->m_msgKeyMap = RegisterWindowMessageA(RWM_KEYMAP);
		if (!msgService || !this->m_msgKeyMap)
		{
			SetWindowLongA(hwndIme, GWL_WNDPROC, g_oldImeWndProc);
			g_oldImeWndProc = 0;
			return;
		}
		this->m_imeVersion = SendMessageA(hwndIme, msgService, 0, 0);
		if (this->m_imeVersion != VERSION_ID_IME98 && this->m_imeVersion != VERSION_ID_IME98A)
		{
			SetWindowLongA(hwndIme, GWL_WNDPROC, g_oldImeWndProc);
			g_oldImeWndProc = 0;
			return;
		}
		IMEKMSINIT init;
		init.cbSize = sizeof(init);
		init.hWnd = this->m_hWnd;
		if (SendMessageA(hwndIme, this->m_msgKeyMap, FID_MSIME_KMS_INIT, (LPARAM)&init))
			SetKeyMap(1);
	}
}

void WIme::OnComposition(unsigned int dwCommand, long dwData)
{
	if (Check())
	{
		if (dwData & GCS_RESULTSTR)
		{
			this->GetResultString();
		}
		this->GetCompString(dwData);
	}
}

void WIme::OnChar(unsigned int nChar)
{
	switch (nChar)
	{
	case '\b':
		this->ringBuf[this->current++ & 0x3FF] = '\b';
		break;
	case '\t':
		this->ringBuf[this->current++ & 0x3FF] = '\t';
		break;
	case '\r':
		this->ringBuf[this->current++ & 0x3FF] = '\r';
		break;
	case 0x1B:
		this->ringBuf[this->current++ & 0x3FF] = 0x1B;
		break;
	case 0xA1:
		this->ringBuf[this->current++ & 0x3FF] = 2;
		break;
	case 0xA2:
		this->ringBuf[this->current++ & 0x3FF] = 3;
		break;
	case 0xA3:
		this->ringBuf[this->current++ & 0x3FF] = 0x1A;
		break;
	case 0xA4:
		this->ringBuf[this->current++ & 0x3FF] = 0x0C;
		break;
	case 0xA5:
		this->ringBuf[this->current++ & 0x3FF] = 0x04;
		break;
	case 0xA7:
		this->ringBuf[this->current++ & 0x3FF] = 0x05;
		break;
	case 0xA6:
		this->ringBuf[this->current++ & 0x3FF] = 0x06;
		return;
	case 0xA8u:
		this->ringBuf[this->current++ & 0x3FF] = 0x07;
		return;
	case 0xAEu:
		this->ringBuf[this->current++ & 0x3FF] = 0x01;
		return;
	default:
		break;
	}

	if (nChar >= 0x20 && nChar <= 0x7E && nChar != '\\')
	{
		char str[2];
		str[0] = nChar;
		str[1] = 0;
		this->PutString(str);
	}
}

void WIme::SetState(int type, int value)
{
	switch (type)
	{
	case 12:
		InitKeyMap();
		break;
	case 13:
		ReleaseKeyMap();
		break;
	}
}

void WIme::InitIme(HWND hwnd_)
{
	if (hwnd_)
	{
		this->m_hWnd = hwnd_;
	}

	this->m_hKeyLayout = GetKeyboardLayout(0);
	this->m_property = ImmGetProperty(this->m_hKeyLayout, IGP_PROPERTY);
	this->ClearData();
	if (LOWORD(this->m_hKeyLayout) == 0x411)
		InitKeyMap();
}

void WIme::OnInputLangChange(unsigned int dwCommand, long dwData)
{
	int i;
	CANDIDATEFORM Candidate;

	if (ImmIsIME(this->m_hKeyLayout) && this->m_property & IME_PROP_AT_CARET)
	{
		this->ClearData();
	}

	InitIme(NULL);
	if (IME_Enter())
	{
		{
			i = 0;
			do
			{
				if (!(this->m_property & IME_PROP_AT_CARET) &&
					ImmGetCandidateWindow(this->m_hIMC, i, &Candidate))
				{
					if (Candidate.dwStyle)
					{
						Candidate.dwStyle = 0;
						ImmSetCandidateWindow(this->m_hIMC, &Candidate);
					}
				}
				++i;
			} while (i < 32);
			ImmReleaseContext(this->m_hWnd, this->m_hIMC);
		}
	}
}

WIme::WIme(HWND hwnd_)
{
	this->m_hWnd = NULL;
	this->m_bActive = false;
	this->m_hKeyLayout = NULL;
	this->m_nState = 0;
	this->m_property = 0;
	this->m_charWidth = 0;
	this->m_charHeight = 0;
	this->m_hIMC = NULL;
	this->m_dwConvMode = 0;
	memset(this->m_compStr, 0, sizeof(this->m_compStr));
	memset(this->m_compAttr, 0, sizeof(this->m_compAttr));
	memset(this->m_compClause, 0, sizeof(this->m_compClause));
	this->m_compStrLen = 0;
	this->m_compAttrLen = 0;
	this->m_compClauseLen = 0;
	this->old_current = 0;
	this->current = 0;
	this->pos = 0;
	this->bUpdated = false;
	this->dwLastInputTime = timeGetTime();
	this->m_candList = new CANDIDATELIST;
	memset(this->m_candList, 0, sizeof(CANDIDATELIST));
	this->m_candSize = 0;
	g_oldImeWndProc = 0;
	this->m_imeVersion = 0;
	this->m_msgKeyMap = 0;
	InitIme(hwnd_);
}

int WIme::WinProc(UINT msg, unsigned long wparam, unsigned long lparam)
{
	if (!m_bActive)
		return 0;
	switch (msg)
	{
	case WM_IME_STARTCOMPOSITION:
		OnStartComposition(wparam, lparam);
		m_nState &= ~4;
		return 1;
	case WM_IME_ENDCOMPOSITION:
		OnEndComposition(wparam, lparam);
		return 1;
	case WM_IME_COMPOSITION:
		OnComposition(wparam, lparam);
		return 1;
	case WM_IME_SETCONTEXT:
		return OnSetContext(wparam, lparam);
	case WM_IME_NOTIFY:
		return OnNotify(wparam, lparam);
	case WM_INPUTLANGCHANGE:
		OnInputLangChange(wparam, lparam);
		m_nState &= ~4;
		break;
	case WM_IME_COMPOSITIONFULL:
		OnCompositionFull(wparam, lparam);
		return 1;
	case WM_IME_CHAR:
		OnControl(wparam, lparam);
		return 1;
	case WM_KEYDOWN:
		if (LOWORD(wparam) >= 0x21 && LOWORD(wparam) <= 0x2E)
		{
			OnChar(wparam | 0x80);
			m_nState &= ~4;
		}
		return 1;
	case WM_CHAR:
		OnChar(wparam);
		if (wparam != '\r' || (m_nState & 4))
			m_nState &= ~4;
		break;
	}
	return 0;
}

WInputDev* WIme::MakeClone(char* modeName, HWND hwnd)
{
	return new WIme(hwnd);
}

#include "wproc.inl"
