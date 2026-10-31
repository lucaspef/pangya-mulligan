#pragma once
#include <map>
#include <string>
#include <vector>
#include "refresh/lib/rftypes.h"
#include "refresh/lib/rfeventhandler.h"

// JP 983: RFWnd.h da Refresh (F3.7.13 item 3, sessão B149). Classe exportada pelo 983 (vtable 00d62fbc, 0x178 bytes). Nomes e
// assinaturas pela exportação; métodos embutidos com as cópias fora de linha de 00401dc0–00402a80; `_children` pelo assert
// "itor != _children.end()" (src\RFWnd.cpp 1520). Os outros nomes de campo são meus, tirados dos acessores (GetX → _x).

struct WndBaseStruct;
class RFWnd;
struct FontInfo;
class RFDirector;
class RFRenderer;
class RFInterface;
class RFManager;
class RFWindowModifier;
class RFVisualEffect;
class MouseEventArgs;

// argumentos de tecla (não exportada; destrutor de apagar 00413ee0; montada pelo RFManager::InjectKeyDown/Up 00419280/00419340)
class KeyEventArgs : public EventArgs
{
public:
	KeyEventArgs(RFWnd* wnd, unsigned int key_, unsigned int flags) : window983(wnd), key(key_), flags983(flags) { }

	RFWnd* window983; // +0x08
	unsigned int key; // +0x0c (VK_*)
	unsigned int flags983; // +0x10 (lParam da mensagem)
};
// argumentos de caractere (não exportada; montada pelo RFManager::InjectChar 00419400)
class CharEventArgs : public EventArgs
{
public:
	CharEventArgs(RFWnd* wnd, const wchar_t* text_, bool composing, const RFPoint& pos)
		: window983(wnd), text(text_), composing983(composing), pos983(pos) { }

	RFWnd* window983; // +0x08
	const wchar_t* text; // +0x0c: texto sem os caracteres de controle (< 0x20 e 0x7f)
	bool composing983; // +0x10 (2º argumento do InjectChar; o UpdateInputMethod passa o estado do IME)
	RFPoint pos983; // +0x14
};

// argumentos dos eventos da janela (não exportados; nomes pela RTTI do 983; cópias fora de linha 00401d10–00401dc0 e 00408240–004082d7)
enum RFWNDCLOSESTATE
{
	RFWNDCLOSESTATE_NONE983 = 0,
	RFWNDCLOSESTATE_ESC983 = 2, // KeyDownEventHandler (Esc)
};

// EventClose (vtable 00d63070)
class WindowCloseEventArgs : public WindowEventArgs_<RFWnd>
{
public:
	WindowCloseEventArgs(RFWnd* wnd, RFWNDCLOSESTATE state) : WindowEventArgs_<RFWnd>(wnd), closeState(state) { }

	RFWNDCLOSESTATE closeState; // +0x0c
};

// EventVisibleChanged (vtable 00d63078; construtor 00401d60)
class VisibleChangedArgs : public WindowEventArgs_<RFWnd>
{
public:
	VisibleChangedArgs(RFWnd* wnd, bool visible) : WindowEventArgs_<RFWnd>(wnd), visible(visible) { }

	bool visible; // +0x0c
};

// EventPlayUIAnimation (vtable 00d630b0, 0x24; construtor 00408240, destrutores 00408260/00408290)
class AnimationEventArgs : public EventArgs
{
public:
	std::wstring name; // +0x08
};

// eventos de mouse (não exportada; posição na tela em +0xc; giro do EventMouseWheel em +0x18, ver refresh/rfbase.h)
class MouseEventArgs : public WindowEventArgs_<RFWnd>
{
public:
	// vtable 00d640c8; botão pelos RFManager::Inject* (0 = esquerdo … 3 = nenhum no InjectMouseMove; nomes meus)
	MouseEventArgs(RFWnd* wnd, const RFVector2& pos_, int button, float wheel)
		: WindowEventArgs_<RFWnd>(wnd), pos(pos_), _14(button), wheelDelta983(wheel) { }

	RFVector2 pos; // +0x0c
	int _14; // +0x14: botão
	float wheelDelta983; // +0x18
};

// EventUpdate (tempo do quadro em +0x8)
class UpdateEventArgs : public EventArgs
{
public:
	float delta; // +0x08
};

enum WndType
{
	WND_TYPE_WND983 = 0,
};

struct StartPositionTypes
{
	enum Type { Manual = 0, CenterParent = 1, CenterScreen = 2 };
};

struct EscKeyActionTypes
{
	enum Type { None = 0, Close = 1 };
};

// estado visual (com efeito visual: 1 abrindo e 3 fechando até o efeito terminar; nomes meus)
struct VisualStateTypes
{
	enum Type { Hidden983 = 0, Showing983 = 1, Visible983 = 2, Hiding983 = 3 };
};

enum RFCWPFlags
{
	RFCWP_ALL983 = 0,
	RFCWP_SKIPINVISIBLE983 = 1,
	RFCWP_SKIPDISABLED983 = 2,
};

// contexto de desenho (não exportado; vtable 00d63060, destrutor de apagar 00402910)
class RFRenderContext
{
public:
	RFRenderContext() : _clip(false), _depth(-1.0f) { }
	virtual ~RFRenderContext() { }
	// contexto do filho a partir do do pai (embutido no Draw/PreDraw; nome meu)
	void Inherit983(bool clip, const RFRect& clipRect, float depth)
	{
		if (clip)
			_clip = true;
		_clipRect = clipRect;
		_depth = depth + 1.0;
	}

	bool _clip; // +0x04
	RFRect _clipRect; // +0x08
	float _depth; // +0x18: profundidade do desenho (o Draw soma 1 por nível; −1 = fora da árvore; vai ao PrintText)
};

class RF_API RFWnd : public RFEventHandler
{
public:
	// par de iteradores devolvido por GetWindowList/GetReverseWindowList
	template<class It> class IteratorBase
	{
	public:
		IteratorBase(It begin, It end) : _cur(begin), _end(end) { }

		It _cur;
		It _end;
	};

	RFWnd(); // 00404840
	virtual const wchar_t* ToString() const { return L"RFWnd"; } // 00401dc0 (slot 0)
	virtual ~RFWnd(); // 00404a40
	virtual const std::wstring& GetTypeName(); // 00404830
	virtual void Initialize(); // 00404c20
	virtual void Create(WndBaseStruct* base, RFWnd* parent); // 00404d30
	virtual void Destroy(); // 00404e50
	virtual bool HitTest(const RFVector2& pt, bool a2); // 00407290
	virtual bool IsFocusable() const { return false; } // 00401ee0 (slot 7)

	const std::wstring& GetName() const { return _name; } // 00401dd0
	const RFVector2& GetInitialPos() const { return _initialPos; } // 00401de0
	const RFSize& GetLocalSize() const { return _localSize; } // 00401df0
	const RFSize& GetInitialSize() const { return _initialSize; } // 00401e00
	void SetStartPosition(StartPositionTypes::Type type) { _startPosition = type; } // 00401e10
	StartPositionTypes::Type GetStartPosition() const { return _startPosition; } // 00401e20
	void SetEscKeyAction(EscKeyActionTypes::Type type) { _escKeyAction = type; } // 00401e30
	EscKeyActionTypes::Type GetEscKeyAction() const { return _escKeyAction; } // 00401e40
	void SetDragable(bool dragable) { _dragable = dragable; } // 00401e50
	bool IsDragable() const { return _dragable; } // 00401e60
	void SetAnchor(unsigned int anchor) { _anchor = anchor; } // 00401e70
	unsigned int GetAnchor() const { return _anchor; } // 00401e80
	const RFRect& GetAnchorValue() const { return _anchorValue; } // 00401e90
	WndType GetType() const { return _type; } // 00401ea0
	void SetSuper(RFWnd* super) { _super = super; } // 00401eb0
	void SetSubControl(bool subControl) { _subControl = subControl; } // 00401ec0
	bool IsSubControl() const { return _subControl; } // 00401ed0
	void SetInheritsAlpha(bool inherits) { _inheritsAlpha = inherits; } // 00401ef0
	bool InheritsAlpha() const { return _inheritsAlpha; } // 00401f00
	void _ApplyAnchor() { ApplyAnchor(); } // 00401f10
	// 00401f20: muda a visibilidade sem animação
	void ShowWindow_(bool show)
	{
		if (_visible != show)
		{
			_visualState = show ? VisualStateTypes::Visible983 : VisualStateTypes::Hidden983;
			_visible = show;
			OnShow(show);
		}
	}
	bool IsModal() const { return _modal; } // 00401f50
	void SetInputLock(bool lock) { _inputLock = lock; } // 00401f60
	bool IsInputLocked() const { return _inputLock; } // 00401f70
	bool IsPreDraw() { return _preDraw; } // 00401f80
	VisualStateTypes::Type GetVisualState() const { return _visualState; } // 00401f90
	void SetVisualState(VisualStateTypes::Type state) { if (_visualEffect) _visualState = state; } // 00401fa0
	RFWNDCLOSESTATE GetCloseState() const { return _closeState; } // 00401fc0
	RFDirector* GetAnimation() { return _animation; } // 00401fd0
	RFRenderContext* GetRC() { return &_rc; } // 00401fe0
	void ClearUserStrings() { _userStrings.clear(); } // 00401ff0
	int GetPriority() const { return _priority; } // 00402030
	void SetOverlapped(bool overlapped) { _overlapped = overlapped; } // 00402040
	bool IsOverlapped() const { return _overlapped; } // 00402050
	void SetClipChildren(bool clip) { _clipChildren = clip; } // 00402060
	bool IsClipChildren() const { return _clipChildren; } // 00402070

	RFWnd* FindWnd(float x, float y) { return ChildWindowFromPoint(x, y, RFCWP_SKIPINVISIBLE983); } // 00402940
	// 00402960 (top = subir até o fim: o laço do 983 devolve sempre NULL nesse caso)
	RFWnd* GetSuper(bool top)
	{
		if (top)
		{
			RFWnd* wnd = this;
			while (wnd)
				wnd = const_cast<RFWnd*>(static_cast<const RFWnd*>(wnd)->GetSuper(false));
			return wnd;
		}
		return _super;
	}
#line 678 RF_INCLUDE_RFWND_H983
	RFWnd* GetChild(unsigned int index) // 00402990
	{
		if (index >= _children.size())
		{
			assert(!"Parameter 'index' out of range.");
			return 0;
		}
		return _children[index];
	}
	void SetHitTestVisible(bool visible) { _hitTestVisible = visible; } // 004029e0
	bool IsHitTestVisible() const { return _hitTestVisible; } // 004029f0
	void SetHitTestThrough(bool through) { _hitTestThrough = through; } // 00402a00
	bool IsHitTestThrough() const { return _hitTestThrough; } // 00402a10
	// D857: para a visão do jogo (rfscenemanager.h), que lia estes campos por offset do VC10: fora de linha, com o layout real
	// (no toolset novo o STL do RFEventHandler/_modifiers/_children desloca tudo; o 983 lê +0x61/+0x65/+0x78/+0xd4/+0xd8/+0x13c)
	int GetType983();
	bool IsDragable983();
	const std::wstring& GetName983();
	bool IsVisible983();
	void SetVisibleState983(bool show); // _visible + _visualState, sem OnShow
	void SetHitTestVisible983(bool visible);
	unsigned int GetChildCount() const { return _children.size(); } // 00402a20
	unsigned int GetAnimationMask() const { return _animationMask; } // 00402a30
	void RemoveAnimationMask(unsigned int mask) { if (mask & _animationMask) _animationMask ^= mask; } // 00402a40
	void AddAnimationMask(unsigned int mask) { if (!(mask & _animationMask)) _animationMask |= mask; } // 00402a60

	void DeleteChild(RFWnd* child); // 00404f40
	void DeleteAllChildren(); // 00404fc0
	void EjectChild_(RFWnd* child); // 00405020
	void UpdateLayout(); // 00405140
	const RFWnd* GetSuper(bool top) const; // 00405790
	RFWnd* GetControlSuper(); // 004057c0
	RFWnd* GetOverlapped(); // 004057f0
	RFWnd* GetActiveChild(); // 00405810
	bool IsActive() const; // 00405880
	RFWnd* GetChild(const std::wstring& name); // 004058a0
	void RegisterChild(RFWnd* child); // 00405960
	void CenterWindow(RFWnd* parent); // 004059c0
	void CenterWindow_Horizontal(RFWnd* parent); // 00405b20
	void CenterWindow_Vertical(RFWnd* parent); // 00405c20
	RFWnd* GetWindow_(const std::wstring& path); // 00405d20
	void EnableWindow(bool enable); // 00405fc0
	bool IsWindowEnabled(bool recursive) const; // 00405ff0
	void Activate(); // 00406030
	void Deactivate(); // 00406090
	void SetFocus(); // 004060e0
	void KillFocus(); // 004060f0
	void SetCapture(); // 00406100
	void ReleaseCapture(); // 00406130
	void AddModifier(RFWindowModifier* modifier); // 00406160
	void RemoveModifier(RFWindowModifier* modifier); // 004061f0
	RFWindowModifier* GetModifier(const std::wstring& name) const; // 00406240
	void LoadUIAnimation(const std::wstring& name); // 004062b0
	void RegisterTo(const std::wstring& name, const std::wstring& event, const Subscriber& subscriber); // 00406430
	void SetPos(float x, float y); // 004064c0
	RFVector2 GetPos() const; // 00406550
	const RFVector2& GetLocalPos() const; // 004065c0
	void SetScreenPos(float x, float y); // 004065d0
	RFVector2 GetScreenPos() const; // 00406670
	void SetSize(const RFSize& size); // 00406700
	RFSize GetSize() const; // 00406750
	void SetRect(const RFRect& rect); // 004067b0
	RFRect GetRect() const; // 00406810
	void SetScreenRect(const RFRect& rect); // 00406860
	RFRect GetScreenRect() const; // 004068c0
	void Draw(const RFRenderContext& rc); // 00406960
	void Draw2(); // 00406af0
	void PreDraw(const RFRenderContext& rc); // 00406c00
	bool IsWindowVisible(bool recursive) const; // 00406d90
	void ShowWindow(bool show); // 00406de0
	void ShowModal(); // 00407070
	void BringChildWndToTop(RFWnd* child); // 00407110
	void SetPreDraw(bool preDraw); // 004071f0
	void SetVisualEffect(RFVisualEffect* effect); // 00407240
	RFWnd* HitTestChild(const RFVector2& pt, bool a2, bool a3); // 00407340
	RFWnd* ChildWindowFromPoint(float x, float y, RFCWPFlags flags); // 00407420
	bool IsMouseOver(); // 00407520
	RFManager* GetMgr(); // 00407560
	void SetAlpha(float alpha); // 00407570
	float GetAlpha(bool recursive) const; // 004075c0
	void Close(RFWNDCLOSESTATE state); // 00408080
	void PlayAnimation(const std::wstring& name); // 00408150
	void StopAnimation(); // 004082e0
	bool IsAnimationPlaying() const; // 00408300
	void SetUserString(const std::wstring& key, const std::wstring& value); // 00408320
	const std::wstring& GetUserString(const std::wstring& key) const; // 00408350
	bool IsUserStringDefined(const std::wstring& key) const; // 004083d0
	void SetPriority(int priority); // 00408450

	static const unsigned int AnimMask_Visible; // 00d629e0
	static const std::wstring TypeName; // 00e4698c
	static const std::wstring EventClose; // 00e469a8
	static const std::wstring EventFinalize; // 00e469c4
	static const std::wstring EventLButtonUp; // 00e469e0
	static const std::wstring EventLButtonDown; // 00e469fc
	static const std::wstring EventLButtonDblClick; // 00e46a18
	static const std::wstring EventRButtonUp; // 00e46a34
	static const std::wstring EventRButtonDown; // 00e46a50
	static const std::wstring EventMouseMove; // 00e46a6c
	static const std::wstring EventMouseWheel; // 00e46a88
	static const std::wstring EventMouseEnter; // 00e46aa4
	static const std::wstring EventMouseLeave; // 00e46ac0
	static const std::wstring EventContextMenu; // 00e46adc
	static const std::wstring EventKeyUp; // 00e46af8
	static const std::wstring EventKeyDown; // 00e46b14
	static const std::wstring EventChar; // 00e46b30
	static const std::wstring EventActivated; // 00e46b4c
	static const std::wstring EventDeactivated; // 00e46b68
	static const std::wstring EventSetFocus; // 00e46b84
	static const std::wstring EventKillFocus; // 00e46ba0
	static const std::wstring EventUpdate; // 00e46bbc
	static const std::wstring EventDraw; // 00e46bd8
	static const std::wstring EventPlayUIAnimation; // 00e46bf4
	static const std::wstring EventVisibleChanged; // 00e46c10
	static const std::wstring AnimOpening; // 00e46c2c
	static const std::wstring AnimClosing; // 00e46c48

	friend class RFManager; // os Inject*/Notify* chamam os On* protegidos (B151)
	friend class RFDirector; // o FrameMove(RFWnd*) chama OnShow/OnLayoutChanged das janelas animadas (B157)
	friend class RFStackPanel; // o StackUp chama OnLayoutChanged/ApplyAnchor dos filhos (B209)
protected:
	void SetName(const std::wstring& name) { _name.assign(name, 0, std::wstring::npos); } // 00402080
	IteratorBase<std::vector<RFWnd*>::iterator> GetWindowList() // 004020a0
	{
		return IteratorBase<std::vector<RFWnd*>::iterator>(_children.begin(), _children.end());
	}
	IteratorBase<std::vector<RFWnd*>::reverse_iterator> GetReverseWindowList() // 004020c0
	{
		return IteratorBase<std::vector<RFWnd*>::reverse_iterator>(_children.rbegin(), _children.rend());
	}
	virtual void LoadResource() { } // 004020e0 (slot 8)
	virtual void PreInitialize(); // 00404bc0
	virtual void OnInitialUpdate() { } // 004020f0
	virtual void OnDestroy() { } // 00402100
	virtual void OnEnabled(bool enable) { } // 00402110
	virtual void OnShow(bool show); // 00407650
	virtual void OnLayoutChanged(unsigned int flags); // 00407630
	virtual void OnLButtonDown(MouseEventArgs& e); // 004076d0
	virtual void OnLButtonUp(MouseEventArgs& e); // 00407760
	virtual void OnLButtonDblClick(MouseEventArgs& e); // 004077d0
	virtual void OnRButtonDown(MouseEventArgs& e); // 00407810
	virtual void OnRButtonUp(MouseEventArgs& e); // 00407850
	virtual void OnMouseMove(MouseEventArgs& e); // 004078c0
	virtual void OnMouseWheel(MouseEventArgs& e); // 00407940
	virtual void OnMouseEnter(); // 00407980
	virtual void OnMouseLeave(); // 004079f0
	virtual void OnKeyDown(KeyEventArgs& e); // 00407a60
	virtual void OnKeyUp(KeyEventArgs& e); // 00407aa0
	virtual void OnChar(CharEventArgs& e); // 00407ae0
	virtual void OnActivated(); // 00407b20
	virtual void OnDeactivated(); // 00407b90
	virtual void OnSetFocus(); // 00407c00
	virtual void OnKillFocus(); // 00407d30
	virtual void OnUpdate(UpdateEventArgs& e); // 00407de0
	virtual void OnDraw(); // 00408060
	virtual void OnAnimFinished(EventArgs& e); // 00408070 (slot 33)

	void ValidatePriority(); // 00405080
	void InvalidatePriority(); // 00405130
	RFRenderer* GetRenderer(); // 004051d0
	RFInterface* GetInterface(); // 004051e0
	void ApplyAnchor(); // 004051f0
	void UpdateAnchorValue(); // 004053f0
	RFRect ClientToScreen() const; // 00405470
	RFVector2 ScreenToClient(const RFVector2& pt) const; // 00405540
	RFVector2 ClientToScreen(const RFVector2& pt) const; // 004055b0
	void PlaySfx(const std::wstring& name); // 00405620
	void PrintText(const FontInfo& font, int x, int y, const std::wstring& text); // 00405700
	void KeyDownEventHandler(EventArgs& e); // 00405760
	RFVector2 GetScreenPos_Impl() const; // 004066c0

public: // (campos: a biblioteca os declara protegidos; aqui públicos para as visões do jogo)
	WndBaseStruct* _base; // +0x30 (estrutura do XML; apagada no Destroy)
	RFWnd* _super; // +0x34
	bool _inheritsAlpha; // +0x38
	float _alpha; // +0x3c
	RFRect _anchorValue; // +0x40
	std::vector<RFWindowModifier*> _modifiers; // +0x50
	bool _active; // +0x60
	bool _visible; // +0x61
	bool _hiding; // +0x62: escondendo com efeito visual (ShowWindow(false); o OnUpdate apaga _visible quando o estado chega a Hidden)
	bool _enabled; // +0x63
	bool _overlapped; // +0x64
	bool _hitTestVisible; // +0x65
	bool _hitTestThrough; // +0x66
	bool _67;
	RFVector2 _dragPos; // +0x68 (ponto do OnLButtonDown)
	bool _dragging; // +0x70
	bool _71;
	bool _inputLock; // +0x72
	bool _73;
	bool _clipChildren; // +0x74
	VisualStateTypes::Type _visualState; // +0x78
	RFVisualEffect* _visualEffect; // +0x7c
	RFWNDCLOSESTATE _closeState; // +0x80
	bool _modal; // +0x84
	int _88;
	std::vector<RFWnd*> _children; // +0x8c
	std::vector<RFWnd*> _updateList; // +0x9c (cópia dos filhos no OnUpdate)
	bool _priorityInvalid; // +0xac (InvalidatePriority; ValidatePriority reordena os filhos)
	std::map<std::wstring, std::wstring> _userStrings; // +0xb0
	int _priority; // +0xc0
	RFVector2 _initialPos; // +0xc4
	RFSize _initialSize; // +0xcc
	WndType _type; // +0xd4
	std::wstring _name; // +0xd8
	RFVector2 _localPos; // +0xf4
	RFSize _localSize; // +0xfc
	std::wstring _showSound; // +0x104
	std::wstring _hideSound; // +0x120
	bool _dragable; // +0x13c
	StartPositionTypes::Type _startPosition; // +0x140
	EscKeyActionTypes::Type _escKeyAction; // +0x144
	unsigned int _anchor; // +0x148
	bool _subControl; // +0x14c
	RFDirector* _animation; // +0x150
	unsigned int _animationMask; // +0x154
	RFRenderContext _rc; // +0x158
	bool _preDraw; // +0x174
};
