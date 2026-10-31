// JP 983: RFButton.cpp da Refresh (B156, F3.7.13 item 5): o botão (00420e10–00421b6d). Um pincel por estado (RFBUTTON_STATE:
// normal, apertado, mouse em cima, desabilitado) feito dos ImageSet da <Button>; sons DOWN/OVER/RELEASE; EventClicked no
// soltar dentro do botão ou no Enter do botão padrão. O RFInterface slot 3 recebe (botão, estado do cursor) — a
// RefreshInterface do jogo não faz nada nele. ToString/GetTypeName/SetState em linha no rfcontrols.h; strings no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

extern RFInterface* g_pRFInterface983; // 00f1de14 (rfmanager.cpp)
extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

#if _MSC_VER > 1600
// D976: para a visão do jogo (rfbase.h); no VC10 a visão grava +0x178/+0x17c direto
void RFButton::SetState983(int state)
{
	SetState((RFBUTTON_STATE)state);
}

int RFButton::GetState983()
{
	return _state;
}
#endif

// 00420e10
RFButton::RFButton()
{
}

// 00420e70
RFButton::~RFButton()
{
}

// 00420f30
void RFButton::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	ButtonBase* button = static_cast<ButtonBase*>(base);
	_normalImage = static_cast<ImageSet*>(button->_normalImage);
	_downImage = static_cast<ImageSet*>(button->_pushImage);
	_overImage = static_cast<ImageSet*>(button->_hoverImage);
	_disableImage = static_cast<ImageSet*>(button->_disableImage);
	_pushSound.assign(button->_pushSound, 0, std::wstring::npos);
	_overSound.assign(button->_hoverSound, 0, std::wstring::npos);
	_releaseSound.assign(button->_clickSound, 0, std::wstring::npos);
	_17e = button->_isDefaultButton.Get();
}

// 00420fd0 (o RFWnd::PreInitialize e o RegisterTo embutidos): o botão padrão escuta o EventKeyUp da janela de cima
void RFButton::PreInitialize()
{
	RFWnd::PreInitialize();
	_state = RFBS_NORMAL983;
	_pressed = false;
	_17d = false;
	_normalBrush = 0;
	_downBrush = 0;
	_overBrush = 0;
	_disableBrush = 0;
	if (_17e)
		RegisterTo(L"", EventKeyUp, Subscriber(&RFButton::DefaultButtonEventHandler, this));
}

// 004211c0
void RFButton::LoadResource()
{
	_normalBrush = RFBrush::Create(_normalImage);
	_downBrush = RFBrush::Create(_downImage);
	_overBrush = RFBrush::Create(_overImage);
	_disableBrush = RFBrush::Create(_disableImage);
}

// 00421210
void RFButton::OnDestroy()
{
	RFBrush** brush = &_normalBrush;
	for (int i = 4; i != 0; --i, ++brush)
	{
		if (*brush)
		{
			delete *brush;
			*brush = 0;
		}
	}
}

// 00421240
void RFButton::ButtonReset()
{
	_dragging = false;
	_state = RFBS_NORMAL983;
	_pressed = false;
}

// 00421260: desabilitado (ele ou um pai) desenha o pincel 3; senão o do estado
void RFButton::OnDraw()
{
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, ClientToScreen());
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	RFBUTTON_STATE state;
	if (!_enabled || (_super && !_super->IsWindowEnabled(false)))
		state = RFBS_DISABLE983;
	else
		state = _state;
	float depth = _rc._depth;
	float alpha = GetAlpha(false);
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	(&_normalBrush)[state]->Draw(pos, GetSize(), alpha, depth);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 00421420
void RFButton::OnEnabled(bool enabled)
{
	_state = enabled ? RFBS_NORMAL983 : RFBS_DISABLE983;
}

// 00421440 (o RFWnd::OnShow embutido)
void RFButton::OnShow(bool show)
{
	VisibleChangedArgs e(this, _visible);
	SendEvent(EventVisibleChanged, e);
	if (!show)
		_dragging = false;
}

// 004214d0
void RFButton::OnLButtonDown(MouseEventArgs& e)
{
	RFManager::GetInstance()._capturedWnd = this;
	RFWnd::OnLButtonDown(e);
	PlaySfx(_pushSound);
	_state = RFBS_DOWN983;
	g_pRFInterface983->Slot3_983((int)this, 2);
	e.handled = true;
}

// 00421540: clique = soltar dentro do botão depois de apertar nele
void RFButton::OnLButtonUp(MouseEventArgs& e)
{
	RFManager::GetInstance()._capturedWnd = 0;
	bool dragging = _dragging;
	RFWnd::OnLButtonUp(e);
	e.handled = true;
	if (dragging)
	{
		if (HitTest(e.pos, false))
		{
			if (!_pressed)
				_state = RFBS_OVER983;
			g_pRFInterface983->Slot3_983((int)this, 1);
			PlaySfx(_releaseSound);
			WindowEventArgs_<RFButton> clicked(this);
			SendEvent(EventClicked, clicked);
		}
		else
		{
			g_pRFInterface983->Slot3_983((int)this, 0);
			if (!_pressed)
				_state = RFBS_NORMAL983;
		}
	}
}

// 00421690 (o RFWnd::OnMouseEnter embutido)
void RFButton::OnMouseEnter()
{
	MouseEnter_Impl();
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventMouseEnter, e);
}

// 00421710 (MouseLeave_Impl e RFWnd::OnMouseLeave embutidos)
void RFButton::OnMouseLeave()
{
	MouseLeave_Impl();
	WindowEventArgs_<RFWnd> e(this);
	SendEvent(EventMouseLeave, e);
}

// 004217c0: com o botão apertado (arrastando de volta para dentro) volta a DOWN, sem som
void RFButton::MouseEnter_Impl()
{
	_17d = true;
	if (!_pressed)
		_state = RFBS_OVER983;
	if (_dragging)
	{
		_state = RFBS_DOWN983;
		return;
	}
	g_pRFInterface983->Slot3_983((int)this, _state == RFBS_DISABLE983 ? 3 : 1);
	PlaySfx(_overSound);
}

// 00421820
void RFButton::MouseLeave_Impl()
{
	_17d = false;
	if (!_dragging)
		g_pRFInterface983->Slot3_983((int)this, 0);
	if (!_pressed && _enabled)
	{
		if (_super && !_super->IsWindowEnabled(false))
			return;
		_state = RFBS_NORMAL983;
	}
}

// 00421870: Enter (VK_RETURN) no botão padrão = clique
void RFButton::DefaultButtonEventHandler(EventArgs& e)
{
	if (static_cast<KeyEventArgs&>(e).key == VK_RETURN)
	{
		e.handled = true;
		WindowEventArgs_<RFButton> clicked(this);
		SendEvent(EventClicked, clicked);
	}
}

// 004218f0: o caminho novo vai para o ImageSet da estrutura (sem ImageSet, o pincel sai nulo)
void RFButton::ChangeNormalButtonImage(std::wstring path)
{
	if (_normalBrush)
	{
		delete _normalBrush;
		_normalBrush = 0;
	}
	if (_normalImage)
		static_cast<SingleImageSet*>(_normalImage)->_path.assign(path, 0, std::wstring::npos);
	_normalBrush = RFBrush::Create(_normalImage);
}

// 00421990
void RFButton::ChangeOverButtonImage(std::wstring path)
{
	if (_overBrush)
	{
		delete _overBrush;
		_overBrush = 0;
	}
	if (_overImage)
		static_cast<SingleImageSet*>(_overImage)->_path.assign(path, 0, std::wstring::npos);
	_overBrush = RFBrush::Create(_overImage);
}

// 00421a30
void RFButton::ChangeDownButtonImage(std::wstring path)
{
	if (_downBrush)
	{
		delete _downBrush;
		_downBrush = 0;
	}
	if (_downImage)
		static_cast<SingleImageSet*>(_downImage)->_path.assign(path, 0, std::wstring::npos);
	_downBrush = RFBrush::Create(_downImage);
}

// 00421ad0
void RFButton::ChangeDisableButtonImage(std::wstring path)
{
	if (_disableBrush)
	{
		delete _disableBrush;
		_disableBrush = 0;
	}
	if (_disableImage)
		static_cast<SingleImageSet*>(_disableImage)->_path.assign(path, 0, std::wstring::npos);
	_disableBrush = RFBrush::Create(_disableImage);
}
