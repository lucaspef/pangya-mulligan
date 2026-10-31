#pragma once
#include <assert.h>
#include <list>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfinterface.h"

// JP 983: RFManager.h e RFSingleton.h da Refresh (F3.7.13 item (4); visão da B149 completada na B151 pelo construtor 00417490, o destrutor
// 004177f0 e as cópias fora de linha de 00402e30–004033f0). Sem RF_API na classe de propósito (o construtor de cópia/operator= implícitos
// 004030b0/00403230 ficam para quando o layout estiver todo nomeado); os membros que o 983 exporta levam RF_API um a um.

class RFRenderer;
class RFInputMethodProvider;
class RFWindowFactoryManager;
class RFXmlLoader;
struct WndBaseStruct;

// idioma do teclado do IME (PostIMStatusEvent; RFIme::UpdateLocale983 00452300 pelo LANGID)
struct LocaleIndicatorTypes
{
	enum Type { None983 = 0, Korean983 = 1, Japanese983 = 2, ChineseSimplified983 = 3, ChineseTraditional983 = 4 };
};

// EventHyperLink / EventGlobalHyperLink* (vtable pelo RTTI do 983; 2 textos: alvo e parâmetro do link; nomes meus)
class HyperLinkEventArgs : public WindowEventArgs_<RFWnd>
{
public:
	HyperLinkEventArgs(RFWnd* wnd, const std::wstring& link, const std::wstring& param)
		: WindowEventArgs_<RFWnd>(wnd), link983(link), param983(param) { }

	std::wstring link983; // +0x0c
	std::wstring param983; // +0x28
};

// EventInputMethodStatusChanged
class IMStatusEventArgs : public EventArgs
{
public:
	IMStatusEventArgs(LocaleIndicatorTypes::Type locale, unsigned int status) : locale983(locale), status983(status) { }

	LocaleIndicatorTypes::Type locale983; // +0x08
	unsigned int status983; // +0x0c
};

// e:\program_code\server_client_launcher\trunk\code\client\refresh\include\RFSingleton.h (cópias 00409580–00409600 para o RFManager,
// 0041aa70–0041aae0 para o RFWindowFactoryManager)
template<class T> class RFSingleton
{
public:
	RFSingleton()
	{
		assert(_singleton == NULL);
		_singleton = static_cast<T*>(this);
	}
	~RFSingleton() { _singleton = 0; }
	static T& __fastcall GetInstance() { assert(_singleton); return *_singleton; }
	static T* __fastcall GetInstancePtr() { return _singleton; }

protected:
	static T* _singleton;
};

// base vazia antes do RFSingleton (o static_cast do construtor desconta 0x31, não 0x30: há outra base vazia em +0x30; nome meu)
class RFManagerBase983
{
};

class RFManager : public RFEventHandler, public RFManagerBase983, public RFSingleton<RFManager>
{
public:
	RF_API RFManager(); // 00417490
	RF_API virtual ~RFManager(); // 004177f0

	static RFManager& __fastcall GetInstance(); // 00417450 (assert(_singleton), src\RFManager.cpp 58)
	static RFManager* __fastcall GetInstancePtr(); // 00417480

	RF_API RFWnd* CreateDesktop(const std::wstring& name, RFWnd* wnd); // 00417a20
	RF_API RFWnd* CreateWnd(const std::wstring& name, RFWnd* wnd, RFWnd* parent); // 00417a90 (wnd = janela já alocada ou NULL: fábrica pelo tipo)
	RF_API void FreezeInputs(bool freeze); // 00417e70
	RF_API bool IsInputsFroze() const; // 00417e80
	RF_API void ActivateWindow(RFWnd* wnd); // 004182f0
	RF_API void FocusWindow(RFWnd* wnd); // 00418340
	RF_API R_ERRORTYPE DeleteDesktop(); // 004183b0
	RF_API R_ERRORTYPE DeleteWnd(RFWnd* wnd); // 00418410
	RF_API void DeleteAllWnd(); // 00418460
	RF_API void RegisterDesktop(RFWnd* wnd); // 00418470
	RF_API RFWnd* GetDesktop(); // 004184b0
	RF_API RFWnd* GetWnd(const std::wstring& name); // 004184c0
	RF_API RFWnd* GetInputTargetWindow() const; // 004184d0
	RF_API void SetRenderer(RFRenderer* renderer); // 004184e0
	RF_API void SetRenderer(const std::wstring& module, void* param); // 00418500 (dll com "createRenderer")
	RF_API RFRenderer* GetRenderer(); // 00418630
	RF_API void SetInterface(RFInterface* iface); // 00418640
	RF_API RFInterface* GetInterface(); // 00418660
	RF_API void InitializeInputMethod(struct HWND__* hWnd, bool custom); // 00418670 (HWND)
	RF_API void RestoreInputMethod(); // 004187a0
	RF_API void UpdateInputMethod(); // 00419c40
	RF_API RFInputMethodProvider* GetInputMethod(); // 004187c0
	RF_API void BringWndToTop(RFWnd* wnd); // 004187d0
	RF_API void BringControlToTop(RFWnd* wnd); // 004187e0
	RF_API void SetVisualEffect(RFVisualEffect* effect); // 00418860
	RF_API const wchar_t* GetVersion() const; // 004188a0
	RF_API RFWnd* GetLastActiveModal() const; // 00418950
	RF_API bool IsModalExist() const; // 00418990
	RF_API RFWnd* FindActiveFocusableWnd(RFWnd* wnd); // 004189d0
	RF_API bool InjectLButtonDown(float x, float y); // 00418ad0
	RF_API bool InjectLButtonUp(float x, float y); // 00418be0
	RF_API bool InjectLButtonDblClick(float x, float y); // 00418ce0
	RF_API bool InjectRButtonDown(float x, float y); // 00418df0
	RF_API bool InjectRButtonUp(float x, float y); // 00418f00
	RF_API bool InjectMouseMove(float x, float y); // 00419010
	RF_API bool InjectMouseWheel(int delta, float x, float y); // 00419180
	RF_API bool InjectKeyDown(unsigned int key, unsigned int flags); // 00419280
	RF_API bool InjectKeyUp(unsigned int key, unsigned int flags); // 00419340
	RF_API bool InjectChar(wchar_t* text, bool composing, const RFPoint& pos); // 00419400
	RF_API bool InjectPreDraw(); // 004196d0
	RF_API bool InjectDraw(); // 00419780
	RF_API bool InjectUpdate(float delta); // 00419830
	RF_API const std::wstring& GetCompatibleControlName(WndType type); // 004199f0
	RF_API void NotifyWindowGotFocus(RFWnd* wnd); // 004198d0
	RF_API void NotifyWindowLostFocus(RFWnd* wnd); // 00419900
	RF_API void NotifyWindowActivated(RFWnd* wnd); // 00419920
	RF_API void NotifyWindowDeactivated(RFWnd* wnd); // 00419950
	RF_API void NotifyWindowDestroyed(RFWnd* wnd); // 00419970
	RF_API bool IsUsingCustomInputMethod(); // 00419c30

	// embutidos (cópias fora de linha no RFWnd.cpp, 00402f10–004033f0)
	RF_API RFWnd* GetFocusWnd() const { return _focusWnd; } // 00402f10
	// D856: para as visões do jogo (rfscenemanager.h, lobbyjp.h), que leem a lib por offset do VC10: fora de linha, com o layout real
	// (no toolset novo o STL do RFEventHandler desloca o _root/_focusWnd e muda o sizeof; o 983 lê +0x4c/+0x58 e faz new(0x98))
	RF_API static RFManager* New983();
	RF_API RFWnd* GetRoot983();
	RF_API RFWnd* GetFocusWnd983();
	RF_API void SetCapturedWindow(RFWnd* wnd) { _capturedWnd = wnd; } // 00402f20
	RF_API RFWnd* GetCapturedWindow() const { return _capturedWnd; } // 00402f30
	RF_API void SetGlobalPadding(const RFRect& padding) { _globalPadding = padding; } // 00402f70
	RF_API RFRect GetGlobalPadding() const { return _globalPadding; } // 00402fa0
	RF_API void EnableGlobalPadding(bool enable) { _globalPaddingEnabled = enable; } // 00402fd0
	RF_API bool IsGlobalPaddingEnabled() const { return _globalPaddingEnabled; } // 00402fe0
	RF_API const RFVector2& GetCursorPos() const { return _cursorPos; } // 004033c0
	RF_API void SetCursorPos(const RFVector2& pos) { _cursorPos = pos; } // 004033d0

	// cópias fora de linha / auxiliares do RFWnd.cpp (B268; nomes meus)
	RF_API RFWnd* CreateDesktopA983(const std::wstring& name, RFWnd* wnd); // 00402e30 (= CreateDesktop 00417a20, cópia)
	RF_API RFWnd* CreateDesktopW983(const std::wstring& name, RFWnd* wnd); // 00402ea0 (idem)
	RF_API void SetTopMost983(RFWnd* wnd); // 00402f40 (prioridade -1; não usa this)
	RF_API RFSize& GetResolution983(RFSize& out); // 00402ff0 (tamanho da janela-raiz)
	RF_API void SetResolution983(const RFSize& size); // 00403010
	RF_API void UpdateLayout983(); // 00403060
	RF_API RFWnd* FindWnd983(float x, float y); // 004033f0 (= HitTest2(pt, true, true))
	RF_API RFManager(const RFManager& other); // 004030b0 (cópia; nunca chamada: singleton)
	RF_API RFManager& operator=(const RFManager& other); // 00403230

	RF_API static const std::wstring EventHyperLink; // 00e46c64
	RF_API static const std::wstring EventGlobalHyperLinkEnter; // 00e46c80
	RF_API static const std::wstring EventGlobalHyperLinkLeave; // 00e46c9c
	RF_API static const std::wstring EventInputMethodStatusChanged; // 00e46cb8

protected:
	RF_API void CreateChild(RFXmlLoader& loader, RFWnd* parent, WndBaseStruct* base); // 00417e90
	RF_API void PostHyperLinkEvent(RFWnd* wnd, const std::wstring& link, const std::wstring& param); // 00417fd0
	RF_API void PostGlobalHyperLinkEnterEvent(RFWnd* wnd, const std::wstring& link, const std::wstring& param); // 004180e0
	RF_API void PostGlobalHyperLinkLeaveEvent(RFWnd* wnd); // 004181f0
	friend class RFTextArea; // (B266: os eventos de link do texto)
	friend class RFIme; // (B163: o UpdateStatus983 do IME manda o evento)
	RF_API void PostIMStatusEvent(const LocaleIndicatorTypes::Type& locale, const unsigned int& status); // 00418270
	RF_API RFWnd* HitTest(const RFVector2& pt, bool a2); // 004188b0
	RF_API RFWnd* HitTest2(const RFVector2& pt, bool a2, bool a3); // 004188d0
	RF_API void UpdateWindowContainingMouse(const RFVector2& pt); // 00418a60

public:
	// campos (a biblioteca os declara protegidos; aqui públicos para as visões do jogo; nomes meus menos _singleton)
	void* _rendererModule983; // +0x34: dll do desenhista do SetRenderer(nome, …) (o destrutor pede a função de liberar pelo nome)
	RFRenderer* _renderer; // +0x38 (= 00f1de20)
	RFInterface* _interface; // +0x3c (= 00f1de14)
	RFInputMethodProvider* _inputMethod; // +0x40
	RFWindowFactoryManager* _factoryManager; // +0x44 (criado no construtor com as 14 fábricas padrão)
	bool _inputsFrozen; // +0x48
	RFWnd* _root; // +0x4c: janela-raiz "__refresh_system_layer__" (= 00f1de18; GetWnd procura nela)
	RFWnd* _desktop; // +0x50 (= 00f1de1c; RegisterDesktop)
	RFWnd* _activeWnd; // +0x54
	RFWnd* _focusWnd; // +0x58
	RFWnd* _capturedWnd; // +0x5c
	RFWnd* _mouseOverWnd; // +0x60 (UpdateWindowContainingMouse: OnMouseLeave/OnMouseEnter)
	RFVisualEffect* _visualEffect; // +0x64 (efeito padrão das janelas)
	int _68;
	int _6c;
	RFVector2 _cursorPos; // +0x70
	RFRect _globalPadding; // +0x78
	bool _globalPaddingEnabled; // +0x88
	bool _usingCustomInputMethod; // +0x89 (IsUsingCustomInputMethod)
	std::list<std::wstring> _list8c; // +0x8c (nó 0x24)
};
