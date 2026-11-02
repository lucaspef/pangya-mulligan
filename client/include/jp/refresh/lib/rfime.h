#pragma once
#include <windows.h>
#include <imm.h>
#include <string>
#include <vector>
#include "refresh/lib/rfmanager.h"

// JP 983: RFIme.cpp e RFInputMethodProvider.cpp da Refresh (B163). O RFManager::InitializeInputMethod (00418670) cria o
// RFInputMethodProvider (singleton 00f1de50) e, nele, o RFIme (IME do Windows numa janela filha invisível "ImeWndClass") ou o
// RFCustomIme (o jogo desenha a composição e os candidatos; janela do IME escondida). Nomes dos métodos meus (só o RTTI tem nomes).

// interface do IME (vtable 00d65c24: 11 slots, só o 2 com corpo; destrutor no slot 11)
class RF_API RFInputMethodService
{
public:
	virtual void SetEnable983(bool enable) = 0; // slot 0
	virtual bool IsEnabled983() = 0; // slot 1
	virtual void Restore983() { } // slot 2 (00450fe0; RFManager::RestoreInputMethod)
	virtual void SetNativeMode983(bool native) = 0; // slot 3 (IME_CMODE_NATIVE)
	virtual bool IsNativeMode983() = 0; // slot 4
	virtual void GetStatus983(LocaleIndicatorTypes::Type* locale, unsigned int* status) = 0; // slot 5
	virtual void AssociateContext983(bool associate) = 0; // slot 6
	virtual void SetCompositionPos983(int x, int y) = 0; // slot 7
	virtual void FinishComposition983(bool complete) = 0; // slot 8 (false = cancela)
	virtual int GetResultString983(wchar_t* buffer, int length, bool* composing) = 0; // slot 9 (NULL: só o tamanho do 1º)
	virtual int GetResultCount983() = 0; // slot 10
	virtual ~RFInputMethodService() { } // 00451000 (slot 11)
};

// HIMC da janela, devolvido no fim (00451030/00451050)
struct RFImmContext983
{
	RFImmContext983(HWND wnd) : himc(0), hwnd(wnd) { himc = ImmGetContext(wnd); }
	~RFImmContext983() { if (himc) ImmReleaseContext(hwnd, himc); }

	HIMC himc;
	HWND hwnd;
};

// texto pronto do IME para o RFManager::UpdateInputMethod (0x20; construtor 004510d0)
struct RFImeString983
{
	RFImeString983(const std::wstring& text, bool composing) : text(text), composing(composing) { }

	std::wstring text; // +0x00
	bool composing; // +0x1c (texto em composição, não o resultado)
};

// IME do Windows (0x68; vtable 00d65c5c; construtor 00451130, destrutor 004512c0)
class RF_API RFIme : public RFInputMethodService
{
public:
	RFIme(HWND parent);
	virtual ~RFIme(); // 00451280

	virtual void SetEnable983(bool enable); // 00451600
	virtual bool IsEnabled983() { return _enabled; } // 00451090
	virtual void Restore983(); // 00451660
	virtual void SetNativeMode983(bool native); // 00451680
	virtual bool IsNativeMode983(); // 004516e0
	virtual void GetStatus983(LocaleIndicatorTypes::Type* locale, unsigned int* status); // 00451740
	virtual void AssociateContext983(bool associate); // 00451870
	virtual void SetCompositionPos983(int x, int y); // 00451a20
	virtual void FinishComposition983(bool complete); // 00451bb0
	virtual int GetResultString983(wchar_t* buffer, int length, bool* composing); // 004518b0
	virtual int GetResultCount983() { return (int)_results.size(); } // 00451a10

	// mensagens da janela do IME (ImeWndProc983 00451370); false = tratada (devolve 0), true = DefWindowProc
	virtual bool OnInputLangChange983(WPARAM wParam, LPARAM lParam); // slot 12 (00452090; WM_INPUTLANGCHANGE)
	virtual bool OnSetContext983(WPARAM wParam, LPARAM* lParam) { return false; } // slot 13 (004520b0; WM_IME_SETCONTEXT)
	virtual bool OnStartComposition983(); // slot 14 (004520c0)
	virtual bool OnComposition983(WPARAM wParam, LPARAM lParam); // slot 15 (00452100)
	virtual bool OnEndComposition983(); // slot 16 (00452130)
	virtual bool OnNotify983(WPARAM wParam, LPARAM lParam); // slot 17 (00452190; WM_IME_NOTIFY)
	virtual void OnOpenCandidate983(); // slot 18 (004521d0)
	virtual void OnCloseCandidate983(); // slot 19 (00452200)
	virtual bool OnImeChar983(WPARAM wParam, LPARAM lParam) { return false; } // slot 20 (00452230; WM_IME_CHAR)
	virtual bool OnChar983(WPARAM wParam, LPARAM lParam); // slot 21 (00452240; WM_CHAR)

	void SetCompositionWindow983(const RECT& area, int x, int y); // 00451aa0 (sem chamador no 983)
	void SetCandidateWindow983(int x, int y); // 00451b30 (sem chamador no 983)
	void UpdateStatus983(); // 00451760 (EventInputMethodStatusChanged)
	int UpdateLocale983(DWORD layout); // 00452300 (devolve o charset; o 983 não usa)
	void FlushStrings983(); // 00451c60 (composição ou resultado → _results)
	int GetCompositionString983(wchar_t* buffer, int length); // 00451f20
	void ReadCompositionString983(DWORD type); // 00451fc0 (GCS_COMPSTR → _composition, GCS_RESULTSTR → _result)

	HWND _parent; // +0x04
	bool _enabled; // +0x08
	bool _contextActive; // +0x09 (WM_IME_SETCONTEXT do RFCustomIme)
	bool _composing; // +0x0a
	std::vector<RFImeString983> _results; // +0x0c
	std::wstring _composition; // +0x1c
	std::wstring _result; // +0x38
	HIMC _savedContext; // +0x54 (AssociateContext983(false))
	int _58; // +0x58
	HWND _imeWnd; // +0x5c
	LocaleIndicatorTypes::Type _locale; // +0x60
	unsigned int _status; // +0x64 (1 nativo, 2 katakana, 4 full shape)
};

// IME desenhado pelo jogo (0x8c; vtable 00d65cbc; o construtor é embutido no InitializeInputMethod; destrutor 00453890)
class RF_API RFCustomIme : public RFIme
{
public:
	RFCustomIme(HWND parent) : RFIme(parent), _candidateOpen(false), _hideCandidate(true), _candidateCount(0),
		_restoreMode(false), _nativeMode983(false), _attrStart(0), _attrEnd(0) { }
	virtual ~RFCustomIme(); // 00453830

	virtual bool OnSetContext983(WPARAM wParam, LPARAM* lParam); // 00453bc0
	virtual bool OnStartComposition983(); // 00453be0
	virtual bool OnComposition983(WPARAM wParam, LPARAM lParam); // 00453c20
	virtual bool OnEndComposition983(); // 00453c80
	virtual bool OnNotify983(WPARAM wParam, LPARAM lParam); // 00453d20
	virtual void OnCloseCandidate983(); // 00453dd0
	virtual bool OnImeChar983(WPARAM wParam, LPARAM lParam) { return true; } // 00453e50

	void ReadCompositionAttr983(); // 00453930 (cláusula alvo da composição)
	void ReadCandidateList983(); // 00453a00
	void ResetCandidate983();

	bool _candidateOpen; // +0x68
	bool _hideCandidate; // +0x69 (fecha a lista do Windows no IMN_OPENCANDIDATE)
	int _candidateCount; // +0x6c
	bool _restoreMode; // +0x70
	bool _nativeMode983; // +0x71
	int _attrStart; // +0x74
	int _attrEnd; // +0x78
	std::vector<std::wstring> _candidates; // +0x7c
};

// dono do IME (0x8; singleton 00f1de50; construtor 00455d00, destrutor 00455d60)
class RF_API RFInputMethodProvider : public RFSingleton<RFInputMethodProvider>
{
public:
	RFInputMethodProvider() : _ime(0) { }
	virtual ~RFInputMethodProvider() { }

	RFInputMethodService* _ime; // +0x04
};
