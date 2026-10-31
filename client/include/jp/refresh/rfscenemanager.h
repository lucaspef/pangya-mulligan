#pragma once
#include "vc10_layout.h"
#include <windows.h>
#include <memory>
#include <string>
#include <map>
#include "criticalsection.h"
#include "engine/singleton.h"
#include "engine/wmath.h"
#include "rftypes.h"

#undef CreateDesktop // windows.h (CreateDesktopA); aqui é o RFManager::CreateDesktop da RF

class RefreshInterface;
class RefreshRenderer;

class Subscriber;
class EventArgs;
struct WndBaseStruct;

// JP 983: janela da RF (0x178 bytes, construtor 00404840, vtable 00d62fbc; nomes da RF pelos símbolos do 983). Os slots 0–7
// têm os nomes antigos (unidades de ligação); 8–33 com os nomes da RF (argumentos medidos pelo `ret`; tipos dos eventos ainda
// sem layout, ver refresh/rfbase.h). Tipos de GetType vistos: 3/4/5 (botões: o cursor segue IsWindowVisible), 7 (sem cursor),
// 0xb (campo de texto: cursor 0xc).
class RFWnd
{
public:
	virtual const wchar_t* ToString() const; // 00401dc0 (slot 0; assinatura da lib, B154)
	virtual ~RFWnd(); // 004027f0 (slot 1, destrutor de apagar)
	virtual const std::wstring& GetTypeName(); // 00404830
	virtual void Initialize983(); // 00404c20 (RFWnd::Initialize)
	virtual void Create(WndBaseStruct* base, RFWnd* parent); // 00404d30
	virtual void Destroy983(); // 00404e50 (RFWnd::Destroy)
	virtual bool HitTest(const RFVector2& pt, bool a2); // 00407290 (struct RFVector2 aqui: liga pelo rfapelidos983.cpp)
	virtual bool IsFocusable(); // 00401ee0 (slot 7)
	virtual void LoadResource(); // 004020e0 (slot 8)
	virtual void PreInitialize(); // 00404bc0
	virtual void OnInitialUpdate(); // 004020f0 (slot 10, +0x28)
	virtual void OnDestroy(); // 00402100 (+0x2c)
	virtual void OnEnabled(bool enable); // 00402110 (+0x30)
	virtual void OnShow(bool show); // 00407650 (+0x34)
	virtual void OnLayoutChanged(EventArgs& e); // 00407630
	virtual void OnLButtonDown(EventArgs& e); // 004076d0
	virtual void OnLButtonUp(EventArgs& e); // 00407760
	virtual void OnLButtonDblClick(EventArgs& e); // 004077d0
	virtual void OnRButtonDown(EventArgs& e); // 00407810
	virtual void OnRButtonUp(EventArgs& e); // 00407850
	virtual void OnMouseMove(EventArgs& e); // 004078c0
	virtual void OnMouseWheel(EventArgs& e); // 00407940
	virtual void OnMouseEnter(); // 00407980
	virtual void OnMouseLeave(); // 004079f0
	virtual void OnKeyDown(EventArgs& e); // 00407a60
	virtual void OnKeyUp(EventArgs& e); // 00407aa0
	virtual void OnChar(EventArgs& e); // 00407ae0
	virtual void OnActivated(); // 00407b20
	virtual void OnDeactivated(); // 00407b90
	virtual void OnSetFocus(); // 00407c00
	virtual void OnKillFocus(); // 00407d30
	virtual void OnUpdate(EventArgs& e); // 00407de0 (+0x7c; tempo do quadro em EventArgs::m_time983)
	virtual void OnDraw(); // 00408060 (+0x80)
	virtual void OnAnimFinished(EventArgs& e); // 00408070 (slot 33, +0x84)

	RFSize GetSize(); // 00406750
	void SetSize(const RFSize& size); // 00406700
	RFVector2 GetPos(); // 00406550
	void SetPos(float x, float y); // 004064c0
	RFVector2 GetScreenPos(); // 00406670
	void SetScreenPos(float x, float y); // 004065d0
	RFRect GetRect(); // 00406810
	RFRect GetScreenRect(); // 004068c0
	RFWnd* GetChild(const std::wstring& name); // 004058a0
	// filha pelo nome já convertida (molde fora de linha: RFButton 005b8740)
	template<class T> T* GetChild983(const std::wstring& name) { return dynamic_cast<T*>(GetChild(name)); }
	void PlayAnimation(const std::wstring& name); // 00408150 (avisa o EventAnimationPlay983)
	void StopAnimation(); // 004082e0
	bool IsAnimationPlaying(); // 00408300
	void ShowWindow(bool show); // 00406de0
	void SetRect(const RFRect& rect); // 004067b0
	void SetPreDraw(bool preDraw); // 004071f0
	void Activate(); // 00406030
	void Deactivate(); // 00406090
	void Close(int code); // 00408080
	void CenterWindow(RFWnd* parent); // 004059c0
	void ShowModal(); // 00407070 (RFFormModalBase::DoModal)
	void SetPriority(int priority); // 00408450 (as modais: -3)
	float GetAlpha(bool recursive); // 004075c0
	void CenterWindow_Horizontal(RFWnd* parent); // 00405b20
	// assina o evento `event` da janela `name` (vazio = esta)
	void RegisterTo(const std::wstring& name, const std::wstring& event, const Subscriber& subscriber); // 00406430
	void UpdateLayout(); // 00405140
	bool IsWindowVisible(bool recursive); // 00406d90
	int GetType() { return GetType983(); } // 00401ea0 (embutida)
	bool IsDragable() { return IsDragable983(); } // 00401e60 (embutida)
	// D857: os campos abaixo só valem no VC10; o código lê pela lib (rfwnd.cpp) para valer em qualquer toolset
	int GetType983(); // +0xd4
	bool IsDragable983(); // +0x13c
	const std::wstring& GetName983(); // +0xd8
	bool IsVisible983(); // +0x61
	void SetVisibleState983(bool show); // +0x61 e +0x78 (2 = visível, 0 = escondida)
	void SetHitTestVisible983(bool visible); // +0x65 (_hitTestVisible da lib)

	void EnableWindow(bool enable); // 00405fc0
	void DeleteChild(RFWnd* child); // 00404f40
	void BringChildWndToTop(RFWnd* child); // 00407110
	RFWnd* GetSuper(bool a1); // 00405790 (o pai)
	bool IsWindowEnabled(bool recursive); // 00405ff0

	static const std::wstring EventFinalize; // 00e469c4
	static const std::wstring EventLButtonDown; // 00e469fc
	static const std::wstring EventLButtonDblClick; // 00e46a18 (OnLButtonDblClick 004077d0)
	static const std::wstring EventMouseWheel; // 00e46a88 ("MouseWheel"; argumentos MouseWheelEventArgs983)
	static const std::wstring EventMouseEnter; // 00e46aa4
	static const std::wstring EventMouseLeave; // 00e46ac0
	static const std::wstring EventKeyDown; // 00e46b14
	static const std::wstring EventKillFocus; // 00e46ba0
	static const std::wstring EventUpdate; // 00e46bbc
	static const std::wstring EventDraw; // 00e46bd8
	static const std::wstring EventAnimationPlay983; // 00e46bf4 (PlayAnimation 00408150; nome meu)

	char m_04[0x5d];
	bool m_bVisible; // +0x61
	char m_62[3];
	bool m_bClipChildren983; // +0x65 (o lobby zera nas áreas da seleção de personagem e do balão; nome provisório)
	char m_66[0x12];
	int m_showState; // +0x78 (2 = visível; o RFFormBase o grava junto com m_bVisible)
	char m_7c[0x58];
	int m_type; // +0xd4
	std::wstring m_name; // +0xd8 (GetName 00401dd0 devolve o endereço)
	char m_f4[0x48];
	bool m_bDragable; // +0x13c
	char m_13d[0x3b];
};

// JP 983: RFWndEx (0x180; construtor 0041d980 zera +0x178/+0x17c, vtable 00d6403c). A base das janelas criadas por XML.
class RFWndEx : public RFWnd
{
public:
	RFWndEx(); // 0041d980
	virtual ~RFWndEx(); // 0041d9a0
	virtual const wchar_t* ToString() const; // 004141b0 (assinaturas da lib, B154)
	virtual const std::wstring& GetTypeName(); // 004141c0
	virtual void Create(WndBaseStruct* base, RFWnd* parent); // 0041d9b0
	virtual void LoadResource(); // 0041da40
	virtual void PreInitialize(); // 0041d9e0
	virtual void OnDestroy(); // 0041da60
	virtual void OnDraw(); // 0041da90

	void* _brush; // +0x178 (lib: RFBrush*; nomes da lib desde B159)
	int _imageSet; // +0x17c (lib: ImageSet*; a função 006037a0 lê como int com sinal e compara com o +0xac da tela; era m_layer)
	int GetImageSet983(); // D857: o _imageSet pela lib (rfwndex.cpp); o campo acima só vale no VC10
};

// JP 983: gerente da RF (0x98 bytes, RFManager::RFManager 00417490; destrutor de apagar no slot 1)
class RFManager
{
public:
	RFManager(); // 00417490
	virtual void Slot0_983(); // slot 0
	virtual ~RFManager(); // slot 1 (004177f0)

	static RFManager* GetInstance(); // 00417450
	RFWnd* CreateWnd(const std::wstring& xml, RFWnd* wnd, RFWnd* parent); // 00417a90
	RFWnd* CreateDesktop(const std::wstring& xml, RFWnd* wnd); // 00417a20
	// moldes: cria a janela T pelo XML (filha de `parent` / como área de trabalho); falhou = apaga
	template<class T> T* CreateWnd(const std::wstring& xml, RFWnd* parent)
	{
		T* p = new T;
		RFWnd* wnd = CreateWnd(xml, p, parent);
		if (!wnd)
		{
			if (p)
				delete p;
			return NULL;
		}
		return static_cast<T*>(wnd);
	}
	template<class T> T* CreateDesktop(const std::wstring& xml)
	{
		T* p = new T;
		RFWnd* wnd = CreateDesktop(xml, p);
		if (!wnd)
		{
			if (p)
				delete p;
			return NULL;
		}
		return static_cast<T*>(wnd);
	}
	RFWnd* GetDesktop(); // 004184b0
	void RegisterDesktop(RFWnd* wnd); // 00418470
	void DeleteDesktop(); // 004183b0
	void SetRenderer(RefreshRenderer* renderer); // 004184e0
	void SetInterface(RefreshInterface* iface); // 00418640
	void InitializeInputMethod(HWND hWnd, bool enable); // 00418670
	void RestoreInputMethod(); // 004187a0
	void SetVisualEffect(bool enable); // 00418860
	RFWnd* HitTest2(const RFVector2& pos, bool a2, bool a3); // 004188d0
	bool IsModalExist() const; // 00418990
	void FreezeInputs(bool freeze); // 00417e70
	void FocusWindow(RFWnd* wnd); // 00418340
	bool InjectLButtonDown(float x, float y); // 00418ad0
	bool InjectLButtonUp(float x, float y); // 00418be0
	bool InjectLButtonDblClick(float x, float y); // 00418ce0
	bool InjectRButtonDown(float x, float y); // 00418df0
	bool InjectRButtonUp(float x, float y); // 00418f00
	bool InjectMouseMove(float x, float y); // 00419010
	bool InjectMouseWheel(int delta, float x, float y); // 00419180
	bool InjectKeyDown(unsigned int key, unsigned int flags); // 00419280
	bool InjectKeyUp(unsigned int key, unsigned int flags); // 00419340
	bool InjectPreDraw(); // 004196d0 (bool como na lib, B155)
	bool InjectDraw(); // 00419780
	bool InjectUpdate(float delta); // 00419830
	RFWnd* GetFocusWnd() { return GetFocusWnd983(); } // 00402f10 (embutida)
	// D856: a visão abaixo só vale no VC10; o código lê pela lib (rfmanager.cpp) para valer em qualquer toolset
	static RFManager* New983(); // new RFManager com o sizeof da lib
	RFWnd* GetRoot983(); // _root (+0x4c no VC10)
	RFWnd* GetFocusWnd983(); // _focusWnd (+0x58 no VC10)

	char m_04[0x48];
	RFWnd* m_pRoot; // +0x4c (desktop)
	char m_50[8];
	RFWnd* m_pFocus; // +0x58
	char m_5c[0x3c];
};

class CInputManager;

// JP 983: botões que o sRFMouseState983::Update983 devolve (005f3900)
enum eRFMouseEvent983
{
	RF_MOUSE_LDOWN983 = 0x1,
	RF_MOUSE_LUP983 = 0x2,
	RF_MOUSE_LDBLCLK983 = 0x4,
	RF_MOUSE_RDOWN983 = 0x8,
	RF_MOUSE_RUP983 = 0x10,
	RF_MOUSE_MDOWN983 = 0x20,
	RF_MOUSE_MUP983 = 0x40,
	RF_MOUSE_WHEEL983 = 0x80,
	RF_MOUSE_MOVE983 = 0x100,
};

// JP 983: posição do mouse da RefreshInputHandler (+0x2c; zerada por 005f38e0 com o g_vZero983_f041c4)
struct sRFMouseState983
{
	sRFMouseState983(); // 005f3f20
	~sRFMouseState983(); // 005f3f30 (jmp para o Reset983)
	void Reset983(); // 005f38e0
	unsigned int Update983(CInputManager* pInput); // 005f3900: botões do frame (eRFMouseEvent983) e posição
	WVector* GetPos983() { return &pos; } // 005f35c0 (cópia fora de linha)
	float GetWheel983() { return wheel; } // 005f35d0 (cópia fora de linha)

	WVector pos; // +0x0
	float wheel; // +0xc
};

// JP 983: parâmetros de uma mensagem de teclado guardada pela RefreshInputHandler (valor do map; os 0 do operator[] 005f4d20
// saem de registradores = construtor com argumentos por valor)
struct sRFKeyMessage983
{
	sRFKeyMessage983(WPARAM w = 0, LPARAM l = 0) : wParam(w), lParam(l) {} // 005f35e0 (cópia fora de linha)

	WPARAM wParam;
	LPARAM lParam;
};

// JP 983: entrada (teclado/mouse) do jogo para a RF (0x48 bytes, new no RFSceneManager::Init983 00604160). Unidade
// refreshinputhandler.cpp (005f35c0–005f4ea0, o "semclasse_005f35b2" da F3.7.1). O WinProc empilha WM_KEYDOWN/WM_KEYUP/
// WM_LBUTTONUP (PushMessage983, sob o lock) num map por mensagem; o Update983 do frame repassa as teclas e o mouse (lido do
// CInputManager) ao RFSceneManager e esvazia o map.
class RefreshInputHandler
{
public:
	RefreshInputHandler(); // 005f4da0
	~RefreshInputHandler(); // 005f4c30
	void Initialize983(); // 005f3600 (vazia)
	int GetButtons983(); // 005f3b90 (lê o g_input)
	void Update983(float delta); // 005f4b00
	void PushMessage983(unsigned int msg, WPARAM wParam, LPARAM lParam); // 005f4e60

	void ProcessMouse983(float delta); // 005f3a00
	void ProcessKeys983(); // 005f4870

	// embutidas (cópias fora de linha no rfscenemanager.cpp)
	void SetBlocked983(bool blocked) { m_bBlocked = blocked; m_mouse.Reset983(); } // 00603220
	bool IsBlocked983() { return m_bBlocked; } // 00603240
	sRFMouseState983* GetMouse983() { return &m_mouse; } // 00603250
	float GetWheel983() { return m_mouse.wheel; } // 00603260

	CCriticalSection m_cs; // +0x00
	bool m_bClosed; // +0x18: PushMessage983 não guarda nada (o Update983 zera)
	std::map<unsigned int, sRFKeyMessage983> m_messages; // +0x1c (cabeça +0x20, tamanho +0x24)
	sRFMouseState983 m_mouse; // +0x2c
	float m_wheelWait; // +0x3c: espera até a próxima roda (s)
	float m_wheelDelay; // +0x40 (0,2)
	bool m_bBlocked; // +0x44
};

// JP 983: gerente de cena da RF (WSingleton<RFSceneManager>, instância 00e73e4c, 0x48 bytes — `push 0x48` do 00a65757; vtable
// 00d13d70). Unidade rfscenemanager.cpp (006030c0–006044a0, com a RefreshTexture no começo). A frente A o vê como
// CUiPointer983 em include/jp/lobbyjp.h. Cria a interface, o desenhista e o RFManager da RF (Init983 00604160).
class RFSceneManager : public WSingleton<RFSceneManager>
{
public:
	RFSceneManager(); // 00604090
	virtual ~RFSceneManager(); // 00603d90 (destrutor de apagar 00603fa0)

	bool Init983(); // 00604160
	void Update983(float delta); // 00603e80
	void PreDraw983(); // 00603290
	void Draw983(); // 006032b0
	bool InjectLButtonDown(float x, float y); // 006032d0
	bool InjectLButtonUp(float x, float y); // 00603300
	bool InjectLButtonDblClick(float x, float y); // 00603330
	bool InjectRButtonDown(float x, float y); // 00603360
	bool InjectRButtonUp(float x, float y); // 00603390
	bool InjectMouseMove(float x, float y); // 006033c0
	bool InjectMouseWheel(int delta, float x, float y); // 006033f0
	bool InjectKeyDown(unsigned int key, unsigned int flags); // 00603420
	bool InjectKeyUp(unsigned int key, unsigned int flags); // 00603440
	void SetResolution983(float w, float h); // 006035b0
	void SetRFFocus983(bool focus); // 00603610: as duas marcas da tela do jogo (+0xaa e +0xa9)
	void SetRFKeyFocus983(bool focus); // 00603640 (+0xaa)
	void SetRFMouseFocus983(bool focus); // 00603660 (+0xa9)
	bool HasRFFocus983(); // 00603680
	bool HasRFKeyFocus983(); // 006036b0
	bool HasRFMouseFocus983(); // 006036d0
	bool IsCursorLocked983(); // 006036f0
	void SetCursor983(int cursor, bool lock); // 00603710
	int GetCursor983(); // 00603750
	void OnWndMsg983(unsigned int msg, WPARAM wParam, LPARAM lParam); // 00603850 (CProjectG::WinProc)
	WVector* GetPos983(); // 006038b0
	float GetWheel983(); // 006038d0
	int GetButtons983(); // 006038e0
	void SetInputBlocked983(bool blocked); // 006038f0
	bool IsInputBlocked983(); // 00603920
	bool IsTyping983(); // 00603940 (CProjectG::WinProc: o foco está num campo de texto da RF)
	void UpdateFocus983(); // 006039a0
	void UpdateCursor983(); // 00603b90
	void SetFocusAll983(bool focus); // 00603ca0
	BOOL IsFocusBlocked983(); // 00603cc0
	bool IsOverWnd983(float x, float y); // 00604320

	static RFManager* GetManager983() { return Instance() ? Instance()->m_pManager : NULL; }
	static RefreshRenderer* GetRenderer983() { return Instance() ? Instance()->m_pRenderer : NULL; } // cópia 005f53c0

	RFManager* m_pManager; // +0x28 (new 0x98)
	RefreshInterface* m_pInterface; // +0x2c (new 4)
	RefreshRenderer* m_pRenderer; // +0x30 (new 0x2c)
	std::tr1::shared_ptr<RefreshInputHandler> m_pInput; // +0x34 (new 0x48)
	int m_3c; // +0x3c
	bool m_bDraw; // +0x40 (PreDraw/Draw só com ela)
	bool m_41; // +0x41
	char m_42[2];
	bool m_bCursorSet; // +0x44: último `lock` do SetCursor983
};
C_ASSERT_VC10(sizeof(RFSceneManager) == 0x48);
C_ASSERT_VC10(sizeof(RefreshInputHandler) == 0x48);
C_ASSERT(offsetof(RefreshInputHandler, m_messages) == 0x1c);
C_ASSERT_VC10(offsetof(RefreshInputHandler, m_mouse) == 0x2c);
C_ASSERT_VC10(offsetof(RefreshInputHandler, m_bBlocked) == 0x44);
