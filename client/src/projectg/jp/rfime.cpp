// JP 983: RFIme.cpp e RFInputMethodProvider.cpp da Refresh (B163, PRIORIDADE 3(b)): o IME do RFManager (00450fe0–00453e50 e
// 00455d00–00455d60; o resto da faixa são cópias da STL dos vetores/strings). O RFIme cria uma janela filha invisível
// ("ImeWndClass", WS_CHILD) da janela do jogo; com o IME ligado (SetEnable983) o foco do teclado fica nela: as teclas vão ao
// RFManager (InjectKeyDown/Up) e o texto do IME (WM_CHAR/WM_IME_*) entra na fila _results que o RFManager::UpdateInputMethod
// (00419c40) passa ao InjectChar. O RFCustomIme esconde a janela do IME do Windows e guarda candidatos/cláusula para o jogo
// desenhar. Nomes dos métodos meus (…983); o 983 só tem o RTTI das classes.
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <windows.h>
#include <imm.h>
#include <malloc.h>
#include <string.h>
#include <string>
#include <vector>
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfime.h"

#pragma comment(lib, "imm32.lib")

RFInputMethodProvider* RFSingleton<RFInputMethodProvider>::_singleton; // 00f1de50

// 00451370: janela do IME (o RFIme no GWL 0)
static LRESULT CALLBACK ImeWndProc983(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	RFIme* ime = (RFIme*)GetWindowLongW(hWnd, 0);
	if (!ime)
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	bool result;
	switch (msg)
	{
	case WM_INPUTLANGCHANGE:
		result = ime->OnInputLangChange983(wParam, lParam);
		break;
	case WM_KEYDOWN:
		RFManager::GetInstance().InjectKeyDown(wParam, lParam);
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	case WM_KEYUP:
		RFManager::GetInstance().InjectKeyUp(wParam, lParam);
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	case WM_CHAR:
		result = ime->OnChar983(wParam, lParam);
		ime->FlushStrings983();
		RFManager::GetInstance().UpdateInputMethod();
		break;
	case WM_IME_STARTCOMPOSITION:
		result = ime->OnStartComposition983();
		break;
	case WM_IME_ENDCOMPOSITION:
		result = ime->OnEndComposition983();
		ime->FlushStrings983();
		RFManager::GetInstance().UpdateInputMethod();
		break;
	case WM_IME_COMPOSITION:
		result = ime->OnComposition983(wParam, lParam);
		ime->FlushStrings983();
		RFManager::GetInstance().UpdateInputMethod();
		break;
	case WM_IME_SETCONTEXT:
		result = ime->OnSetContext983(wParam, &lParam);
		break;
	case WM_IME_NOTIFY:
		result = ime->OnNotify983(wParam, lParam);
		break;
	case WM_IME_CHAR:
		result = ime->OnImeChar983(wParam, lParam);
		break;
	default:
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
	if (!result)
		return 0;
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

// 00451130
RFIme::RFIme(HWND parent)
	: _parent(parent), _enabled(true), _contextActive(false), _composing(false), _savedContext(0), _58(0), _imeWnd(0)
{
	WNDCLASSEXW wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = ImeWndProc983;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(RFIme*);
	wc.hInstance = GetModuleHandleW(0);
	wc.hIcon = 0;
	wc.hCursor = LoadCursorW(0, MAKEINTRESOURCEW(32512)); // IDC_ARROW
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"ImeWndClass";
	wc.hIconSm = 0;
	RegisterClassExW(&wc);
	RECT rc;
	GetClientRect(_parent, &rc);
	_imeWnd = CreateWindowExW(0, L"ImeWndClass", 0, WS_CHILD, 0, 0, rc.right, rc.bottom, _parent, 0, GetModuleHandleW(0), 0);
	SetWindowLongW(_imeWnd, 0, (LONG)this);
	UpdateLocale983((DWORD)GetKeyboardLayout(0));
	UpdateStatus983();
}

// 004512c0
RFIme::~RFIme()
{
	DestroyWindow(_imeWnd);
}

// 00451600: ligado, o foco do teclado vai para a janela do IME; desligado, volta à do jogo
void RFIme::SetEnable983(bool enable)
{
	_enabled = enable;
	FinishComposition983(false);
	AssociateContext983(enable);
	_results.clear();
	if (_enabled)
		SetFocus(_imeWnd);
	else
		SetFocus(_parent);
}

// 00451660
void RFIme::Restore983()
{
	if (_enabled)
		SetFocus(_imeWnd);
}

// 00451680
void RFIme::SetNativeMode983(bool native)
{
	RFImmContext983 context(_enabled ? _imeWnd : _parent);
	DWORD conversion, sentence;
	ImmGetConversionStatus(context.himc, &conversion, &sentence);
	ImmSetConversionStatus(context.himc, (conversion & ~(IME_CMODE_NATIVE | IME_CMODE_KATAKANA)) | (native == true ? IME_CMODE_NATIVE : 0),
		sentence);
}

// 004516e0
bool RFIme::IsNativeMode983()
{
	RFImmContext983 context(_enabled ? _imeWnd : _parent);
	DWORD conversion, sentence;
	ImmGetConversionStatus(context.himc, &conversion, &sentence);
	if (conversion & IME_CMODE_NATIVE)
		return true;
	return false;
}

// 00451740
void RFIme::GetStatus983(LocaleIndicatorTypes::Type* locale, unsigned int* status)
{
	*locale = _locale;
	*status = _status;
}

// 00451760
void RFIme::UpdateStatus983()
{
	RFImmContext983 context(_imeWnd);
	DWORD conversion, sentence;
	ImmGetConversionStatus(context.himc, &conversion, &sentence);
	_status = 0;
	if (conversion & IME_CMODE_NATIVE)
		_status = 1;
	if (conversion & IME_CMODE_KATAKANA)
		_status |= 4;
	if (conversion & IME_CMODE_FULLSHAPE)
		_status |= 2;
	RFManager::GetInstance().PostIMStatusEvent(_locale, _status);
}

// 00451870
void RFIme::AssociateContext983(bool associate)
{
	if (associate)
	{
		ImmAssociateContext(_imeWnd, _savedContext);
		return;
	}
	HIMC old = ImmAssociateContext(_imeWnd, 0);
	if (old)
		_savedContext = old;
}

// 004518b0: o 1º texto da fila (com buffer: copia length caracteres e o tira da fila); devolve o tamanho dele
int RFIme::GetResultString983(wchar_t* buffer, int length, bool* composing)
{
	RFImeString983 item = _results.front();
	int size = (int)item.text.size();
	if (buffer)
	{
		memcpy(buffer, item.text.c_str(), length * sizeof(wchar_t));
		*composing = item.composing;
		_results.erase(_results.begin());
	}
	return size;
}

// 00451a20
void RFIme::SetCompositionPos983(int x, int y)
{
	RFImmContext983 context(_imeWnd);
	COMPOSITIONFORM form;
	form.rcArea.left = 0;
	form.rcArea.top = 0;
	form.rcArea.right = 0;
	form.rcArea.bottom = 0;
	form.dwStyle = CFS_POINT;
	form.ptCurrentPos.x = x;
	form.ptCurrentPos.y = y;
	ImmSetCompositionWindow(context.himc, &form);
}

// 00451aa0
void RFIme::SetCompositionWindow983(const RECT& area, int x, int y)
{
	RFImmContext983 context(_imeWnd);
	COMPOSITIONFORM form;
	form.rcArea = area;
	form.dwStyle = CFS_RECT;
	form.ptCurrentPos.x = x;
	form.ptCurrentPos.y = y;
	ImmSetCompositionWindow(context.himc, &form);
}

// 00451b30
void RFIme::SetCandidateWindow983(int x, int y)
{
	RFImmContext983 context(_imeWnd);
	CANDIDATEFORM form = { 0 };
	form.dwStyle = CFS_CANDIDATEPOS;
	form.ptCurrentPos.x = x;
	form.ptCurrentPos.y = y;
	ImmSetCandidateWindow(context.himc, &form);
}

// 00451bb0
void RFIme::FinishComposition983(bool complete)
{
	RFImmContext983 context(_imeWnd);
	if (!complete)
	{
		ImmNotifyIME(context.himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
		_results.clear();
		OnEndComposition983();
	}
	else
		ImmNotifyIME(context.himc, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
}

// 00451f20
int RFIme::GetCompositionString983(wchar_t* buffer, int length)
{
	int size = (int)_composition.size();
	if (buffer && size > 0 && length > 0 && length <= size)
	{
		memcpy(buffer, _composition.c_str(), length * sizeof(wchar_t));
		_composition.clear();
	}
	return size;
}

// 00451c60: em composição vai a composição (mesmo vazia); senão, o resultado que houver
void RFIme::FlushStrings983()
{
	wchar_t composition[0x100] = { 0 };
	wchar_t result[0x100] = { 0 };
	if (_composing)
	{
		if (_composition.size() > 0)
			GetCompositionString983(composition, (int)_composition.size());
		_results.push_back(RFImeString983(std::wstring(composition), true));
	}
	else
	{
		int size = (int)_result.size();
		if (size > 0)
		{
			if (size <= size)
			{
				memcpy(result, _result.c_str(), size * sizeof(wchar_t));
				_result.clear();
			}
			_results.push_back(RFImeString983(std::wstring(result), false));
		}
	}
}

// 00451fc0
void RFIme::ReadCompositionString983(DWORD type)
{
	RFImmContext983 context(_imeWnd);
	if (!context.himc)
		return;
	LONG size = ImmGetCompositionStringW(context.himc, type, 0, 0);
	if (size > 0)
	{
		wchar_t* buffer = (wchar_t*)_alloca(size);
		ImmGetCompositionStringW(context.himc, type, buffer, size);
		if (type & GCS_COMPSTR)
			_composition.assign(buffer, buffer + size / sizeof(wchar_t));
		else
			_result.assign(buffer, buffer + size / sizeof(wchar_t));
	}
}

// 00452090
bool RFIme::OnInputLangChange983(WPARAM wParam, LPARAM lParam)
{
	UpdateLocale983((DWORD)lParam);
	UpdateStatus983();
	return true;
}

// 004520c0
bool RFIme::OnStartComposition983()
{
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CompositionStart983);
	_composing = true;
	return true;
}

// 00452100: só o resultado (a janela do IME do Windows mostra a composição)
bool RFIme::OnComposition983(WPARAM wParam, LPARAM lParam)
{
	if (lParam & GCS_RESULTSTR)
	{
		_composing = false;
		ReadCompositionString983(GCS_RESULTSTR);
	}
	return true;
}

// 00452130
bool RFIme::OnEndComposition983()
{
	_composing = false;
	_composition.erase();
	_result.erase();
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CompositionEnd983);
	return true;
}

// 00452190
bool RFIme::OnNotify983(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IMN_CLOSECANDIDATE:
		OnCloseCandidate983();
		break;
	case IMN_OPENCANDIDATE:
		OnOpenCandidate983();
		return true;
	case IMN_SETCONVERSIONMODE:
		UpdateStatus983();
		return true;
	}
	return true;
}

// 004521d0
void RFIme::OnOpenCandidate983()
{
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CandidateOpen983);
}

// 00452200
void RFIme::OnCloseCandidate983()
{
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CandidateClose983);
}

// 00452240: caractere sem IME (de 1 byte, pela página de código do sistema); Ctrl+tecla e as de controle ficam de fora
bool RFIme::OnChar983(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_BACK:
	case VK_TAB:
	case VK_RETURN:
	case VK_ESCAPE:
		break;
	default:
		if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000))
		{
			wchar_t ch;
			MultiByteToWideChar(CP_ACP, 0, (LPCSTR)&wParam, 1, &ch, 1);
			_result.erase();
			_result.insert(_result.end(), ch);
		}
		break;
	}
	return true;
}

// 00452300: idioma do teclado (LANGID na palavra baixa do HKL) → _locale; devolve o charset (o 983 não usa)
int RFIme::UpdateLocale983(DWORD layout)
{
	_locale = LocaleIndicatorTypes::None983;
	switch (PRIMARYLANGID(LOWORD(layout)))
	{
	case LANG_ARABIC:
		return ARABIC_CHARSET;
	case LANG_BULGARIAN:
	case LANG_RUSSIAN:
	case LANG_UKRAINIAN:
	case LANG_BELARUSIAN:
		return RUSSIAN_CHARSET;
	case LANG_CHINESE:
		switch (SUBLANGID(LOWORD(layout)))
		{
		case SUBLANG_CHINESE_TRADITIONAL:
			_locale = LocaleIndicatorTypes::ChineseTraditional983;
			return CHINESEBIG5_CHARSET;
		case SUBLANG_CHINESE_SIMPLIFIED:
			_locale = LocaleIndicatorTypes::ChineseSimplified983;
			return GB2312_CHARSET;
		}
		break;
	case LANG_CZECH:
	case LANG_HUNGARIAN:
	case LANG_POLISH:
	case LANG_ROMANIAN:
	case LANG_CROATIAN:
	case LANG_SLOVAK:
	case LANG_SLOVENIAN:
	case LANG_MACEDONIAN:
		return EASTEUROPE_CHARSET;
	case LANG_GREEK:
		return GREEK_CHARSET;
	case LANG_HEBREW:
		return HEBREW_CHARSET;
	case LANG_JAPANESE:
		_locale = LocaleIndicatorTypes::Japanese983;
		return SHIFTJIS_CHARSET;
	case LANG_KOREAN:
		_locale = LocaleIndicatorTypes::Korean983;
		return HANGUL_CHARSET;
	case LANG_THAI:
		return THAI_CHARSET;
	case LANG_TURKISH:
		return TURKISH_CHARSET;
	case LANG_ESTONIAN:
	case LANG_LATVIAN:
	case LANG_LITHUANIAN:
		return BALTIC_CHARSET;
	case LANG_VIETNAMESE:
		return VIETNAMESE_CHARSET;
	}
	return ANSI_CHARSET;
}

// 00453830
RFCustomIme::~RFCustomIme()
{
}

void RFCustomIme::ResetCandidate983()
{
	_candidateOpen = false;
	_attrStart = 0;
	_attrEnd = 0;
	_candidateCount = 0;
	_candidates.clear();
}

// 00453930: a cláusula alvo (ATTR_TARGET_*) da composição
void RFCustomIme::ReadCompositionAttr983()
{
	RFImmContext983 context(_imeWnd);
	if (!context.himc)
		return;
	LONG size = ImmGetCompositionStringW(context.himc, GCS_COMPATTR, 0, 0);
	if (size > 0)
	{
		BYTE* attr = (BYTE*)_alloca(size + 1);
		memset(attr, 0, size + 1);
		ImmGetCompositionStringW(context.himc, GCS_COMPATTR, attr, size);
		int start;
		for (start = 0; start < size; start++)
		{
			if (attr[start] == ATTR_TARGET_CONVERTED || attr[start] == ATTR_TARGET_NOTCONVERTED)
				break;
		}
		int end;
		for (end = start; end < size; end++)
		{
			if (attr[end] != attr[start])
				break;
		}
		_attrStart = start;
		_attrEnd = end;
	}
}

// 00453a00: a página atual da lista de candidatos
void RFCustomIme::ReadCandidateList983()
{
	RFImmContext983 context(_imeWnd);
	if (!context.himc)
		return;
	DWORD size = ImmGetCandidateListW(context.himc, 0, 0, 0);
	if ((int)size > 0)
	{
		_candidates.clear();
		CANDIDATELIST* list = (CANDIDATELIST*)_alloca(size);
		ImmGetCandidateListW(context.himc, 0, list, size);
		if (list)
		{
			DWORD i = 0;
			for (DWORD j = (list->dwSelection / list->dwPageSize) * list->dwPageSize; j < list->dwCount && i < list->dwPageSize; i++, j++)
				_candidates.push_back(std::wstring((const wchar_t*)((BYTE*)list + list->dwOffset[j])));
			_candidateCount = i;
		}
	}
}

// 00453bc0: some com a janela do IME do Windows (lParam 0)
bool RFCustomIme::OnSetContext983(WPARAM wParam, LPARAM* lParam)
{
	_contextActive = wParam == TRUE;
	*lParam = 0;
	return true;
}

// 00453be0
bool RFCustomIme::OnStartComposition983()
{
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CompositionStart983);
	_composing = true;
	return false;
}

// 00453c20
bool RFCustomIme::OnComposition983(WPARAM wParam, LPARAM lParam)
{
	if (lParam & GCS_COMPSTR)
	{
		_composing = true;
		ReadCompositionString983(GCS_COMPSTR);
	}
	if (lParam & GCS_RESULTSTR)
	{
		_composing = false;
		ReadCompositionString983(GCS_RESULTSTR);
	}
	if (lParam & GCS_COMPATTR)
		ReadCompositionAttr983();
	return false;
}

// 00453c80
bool RFCustomIme::OnEndComposition983()
{
	ResetCandidate983();
	_composing = false;
	_composition.erase();
	_result.erase();
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CompositionEnd983);
	return false;
}

// 00453d20
bool RFCustomIme::OnNotify983(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IMN_CHANGECANDIDATE:
		break;
	case IMN_CLOSECANDIDATE:
		OnCloseCandidate983();
		return false;
	case IMN_OPENCANDIDATE:
		if (_hideCandidate)
		{
			RFImmContext983 context(_imeWnd);
			ImmNotifyIME(context.himc, NI_CLOSECANDIDATE, 0, 0);
			return false;
		}
		OnOpenCandidate983();
		break;
	case IMN_SETCONVERSIONMODE:
		UpdateStatus983();
		if (_restoreMode)
			SetNativeMode983(_nativeMode983);
		return true;
	default:
		return true;
	}
	_candidateOpen = true;
	ReadCandidateList983();
	return false;
}

// 00453dd0
void RFCustomIme::OnCloseCandidate983()
{
	ResetCandidate983();
	RFManager::GetInstance()._interface->OnInputMethodStateChanged(InputMethodStateTypes::CandidateClose983);
}
