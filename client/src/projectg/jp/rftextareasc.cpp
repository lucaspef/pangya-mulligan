// JP 983: RFTextAreaSC.cpp da Refresh (B265, F3.7.13 item 5): a area de texto com barras (0044d6a0-0044ed60; tipo "TextArea"
// do XML com <control type="HSCROLLBAR"/"VSCROLLBAR">), sem fonte no 645/839: escrita pelo asm do 983 (out/F3.7/s265/sc.asm).
// So a barra vertical rola o texto (a horizontal so e criada/escondida); a posicao da barra vira _lineLimit983 (+0x1a8 = -pos) e
// a faixa = soma das alturas dos paragrafos (RFText983::TotalHeight983) - altura da area cliente (menos o _padding).
// <showalways>: a barra fica visivel e so desabilita sem rolagem; <autovisible>: aparece/some e o retangulo do controle
// cresce/encolhe de _autoVisibleRect. ToString/GetTypeName/Get?ScrollBar/StickToBottom em linha no rfcontrols.h.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um metodo com esse nome)
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rftext.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"
#include "refresh/rffactorymanager.h"

// 0044d6a0
RFTextAreaSC::RFTextAreaSC()
{
	_hScrollBar = 0;
	_vScrollBar = 0;
}

// 0044d6e0
RFTextAreaSC::~RFTextAreaSC()
{
}

// 0044d6f0 (o 983 pega a fabrica pelo RFSingleton<RFWindowFactoryManager>; aqui pelo _factoryManager do RFManager, o mesmo objeto)
void RFTextAreaSC::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFTextArea::Create(base, parent);
	_hScrollBar = 0;
	_vScrollBar = 0;
	TextAreaSCBase* ta = static_cast<TextAreaSCBase*>(base);
	_showAlwaysH = ta->_showAlwaysH;
	_showAlwaysV = ta->_showAlwaysV;
	_autoVisibleH = ta->_autoVisibleH;
	_autoVisibleV = ta->_autoVisibleV;
	memcpy(&_autoVisibleRect, &ta->_autoVisibleRect, sizeof(RFRect));
	if (ta->_hScrollbar)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)ta->_hScrollbar->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 55 "src\\RFTextAreaSC.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(ta->_hScrollbar, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_hScrollBar = static_cast<RFScrollBar*>(wnd);
	}
	if (ta->_vScrollbar)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)ta->_vScrollbar->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 68 "src\\RFTextAreaSC.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(ta->_vScrollbar, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_vScrollBar = static_cast<RFScrollBar*>(wnd);
		_vScrollBar->_smoothScroll = true;
	}
}

// 0044d910: sem <showalways> a barra horizontal some; a vertical so fica habilitada/visivel com rolagem
void RFTextAreaSC::PreInitialize()
{
	RFTextArea::PreInitialize();
	_stickToBottom = true;
	if (_hScrollBar)
	{
		if (!_showAlwaysH)
			_hScrollBar->ShowWindow(false);
		RegisterTo(_hScrollBar->_name, RFScrollBar::EventPosChanged, Subscriber(&RFTextAreaSC::OnHScrollPosChanged, this));
		RegisterTo(_hScrollBar->_name, RFWnd::EventDraw, Subscriber(&RFTextAreaSC::OnScrollDraw, this));
	}
	if (_vScrollBar)
	{
		if (_showAlwaysV)
		{
			if (!IsScrollBarAvailable(RFSCROLLBAR_VERT983) && _vScrollBar && _vScrollBar->_enabled)
				_vScrollBar->EnableWindow(false);
		}
		else if (_autoVisibleV)
		{
			if (!IsScrollBarAvailable(RFSCROLLBAR_VERT983))
				_vScrollBar->ShowWindow(false);
		}
		else
			_vScrollBar->ShowWindow(false);
		RegisterTo(_vScrollBar->_name, RFScrollBar::EventPosChanged, Subscriber(&RFTextAreaSC::OnVScrollPosChanged, this));
		RegisterTo(_vScrollBar->_name, RFWnd::EventDraw, Subscriber(&RFTextAreaSC::OnScrollDraw, this));
	}
}

// 0044dc10: so a barra vertical conta como parte do controle
bool RFTextAreaSC::HitTest(const RFVector2& pt, bool a2)
{
	if (!_visible || !_hitTestVisible)
		return false;
	if (!a2 && !_enabled)
		return false;
	if (_vScrollBar && _vScrollBar->HitTest(pt, a2))
		return true;
	return ClientToScreen().PtInRect(pt);
}

// 0044dc90
void RFTextAreaSC::OnLayoutChanged(unsigned int flags)
{
	RFTextArea::OnLayoutChanged(flags);
	if ((flags & 2) || (flags & 0x10000000))
		UpdateScrollbar();
}

// 0044dcc0
void RFTextAreaSC::OnChar(CharEventArgs& e)
{
	RFTextArea::OnChar(e);
	StickToBottomProcess();
}

// 0044dce0: editavel: o cursor (+0x2c do paragrafo corrente) tem que ficar dentro da area cliente; a barra anda o que faltar
void RFTextAreaSC::OnKeyDown(KeyEventArgs& e)
{
	RFTextArea::OnKeyDown(e);
	if (!_26c)
		return;
	StickToBottomProcess();
	if (!_vScrollBar)
		return;
	float cur = (float)_text983->_current->_f2c;
	int pos = _vScrollBar->_pos983;
	RFRect r = ClientToScreen();
	float top = r.top + _padding.top;
	float bottom = r.bottom - _padding.bottom;
	if (cur < top)
		pos += (int)(cur - top);
	else if (cur >= bottom)
		pos += (int)((cur - bottom) + (float)_220);
	_vScrollBar->SetScrollPos(pos);
}

// 0044dde0 (mesmo corpo do RFTextArea::OnUpdate: o piscar do caret)
void RFTextAreaSC::OnUpdate(UpdateEventArgs& e)
{
	RFWnd::OnUpdate(e);
	RFCaret983* caret = _caret983;
	if (caret && caret->_visible)
	{
		caret->_timer = e.delta + caret->_timer;
		if (caret->_period < caret->_timer)
		{
			caret->_timer = 0.0f;
			caret->_blinkOn = !caret->_blinkOn;
		}
	}
}

// 0044de30: repassa o evento e anda 2 linhas (2 * _220) pela rolagem suave (giro >= 0 = para cima)
void RFTextAreaSC::OnMouseWheel(MouseEventArgs& e)
{
	if (!_vScrollBar || !IsScrollBarAvailable(RFSCROLLBAR_VERT983))
		return;
	SendEvent(EventMouseWheel, e);
	if (!_vScrollBar || !IsScrollBarAvailable(RFSCROLLBAR_VERT983))
		return;
	int step = (int)(2.0f * (float)_220);
	int pos = _vScrollBar->_pos983;
	if (0.0f <= e.wheelDelta983)
	{
		if (pos <= _vScrollBar->_range.x)
			return;
		_vScrollBar->MoveToScroll(pos - step);
	}
	else
	{
		if (pos >= _vScrollBar->_range.y)
			return;
		_vScrollBar->MoveToScroll(pos + step);
	}
	e.handled = true;
}

// 0044df00 (slots ClearText 0x98 e AddText 0x8c)
void RFTextAreaSC::SetText(const std::wstring& text)
{
	ClearText();
	AddText(text);
	StickToBottomProcess();
}

// 0044df30
void RFTextAreaSC::AddText(const std::wstring& text)
{
	_text983->AddText983(text.c_str(), true);
	if (_26d)
		FitToContent(false);
	StickToBottomProcess();
}

// 0044df70
void RFTextAreaSC::AddPlainText(const std::wstring& text)
{
	_text983->AddText983(text.c_str(), false);
	if (_26d)
		FitToContent(false);
	StickToBottomProcess();
}

// 0044dfb0
void RFTextAreaSC::ClearText()
{
	_textLimit983 = 0;
	_lineLimit983 = 0;
	_text983->Reset983();
	UpdateScrollbar();
}

// 0044dfe0
void RFTextAreaSC::LockScrollbar(RFSCROLLBAR_TYPE type, bool lock)
{
	switch (type)
	{
	case RFSCROLLBAR_HORZ983:
		if (_hScrollBar)
			_hScrollBar->EnableWindow(!lock);
		break;
	case RFSCROLLBAR_VERT983:
		if (_vScrollBar)
			_vScrollBar->EnableWindow(!lock);
		break;
	case RFSCROLLBAR_BOTH983:
		if (_hScrollBar)
			_hScrollBar->EnableWindow(!lock);
		if (_vScrollBar)
			_vScrollBar->EnableWindow(!lock);
		break;
	}
}

// 0044e070: so a vertical; o texto (todos os paragrafos) passa da area cliente (menos o _padding)
bool RFTextAreaSC::IsScrollBarAvailable(RFSCROLLBAR_TYPE type)
{
	if (type != RFSCROLLBAR_VERT983)
		return false;
	RFRect r = ClientToScreen();
	float top = r.top + _padding.top;
	float bottom = r.bottom - _padding.bottom;
	int total = _text983->TotalHeight983();
	return (float)(unsigned int)total > bottom - top;
}

// 0044e130
void RFTextAreaSC::ScrollToTop()
{
	if (_vScrollBar)
		_vScrollBar->SetScrollPos(_vScrollBar->_range.x);
}

// 0044e150
void RFTextAreaSC::ScrollToBottom()
{
	if (_vScrollBar)
		_vScrollBar->SetScrollPos(_vScrollBar->_range.y);
}

// 0044e170: com <stickToBottom> e a barra no fim (ou sem rolagem), o texto novo mantem a barra no fim; se o RFText descartou
// linhas velhas (total >= limite), a posicao anda de volta uma linha (_220)
void RFTextAreaSC::StickToBottomProcess()
{
	if (!_stickToBottom || !_vScrollBar)
	{
		UpdateScrollbar();
		return;
	}
	bool atBottom = false;
	if (IsScrollBarAvailable(RFSCROLLBAR_VERT983))
	{
		if (_vScrollBar->_pos983 == _vScrollBar->_range.y)
			atBottom = true;
	}
	else
		atBottom = true;
	UpdateScrollbar();
	if (atBottom)
	{
		_vScrollBar->SetScrollPos(_vScrollBar->_range.y);
		return;
	}
	if ((unsigned int)_text983->_p38 > _text983->TotalLines983())
		return;
	_vScrollBar->SetScrollPos(_vScrollBar->_pos983 - _220);
}

// 0044e220
void RFTextAreaSC::UpdateScrollbar()
{
	if (!_vScrollBar)
		return;
	if (IsScrollBarAvailable(RFSCROLLBAR_VERT983))
	{
		if (_showAlwaysV)
		{
			if (_vScrollBar && !_vScrollBar->_enabled)
				_vScrollBar->EnableWindow(true);
		}
		else if (_autoVisibleV)
		{
			RFScrollBar* bar = _vScrollBar;
			bool doBlock = true;
			if (!bar->_hiding && bar->_visible)
			{
				RFWnd* super = bar->_super;
				if (!super || super->IsWindowVisible(false))
					doBlock = false;
			}
			if (doBlock)
			{
				RFRect r = GetRect();
				RFRect a;
				a.left = _autoVisibleRect.left + r.left;
				a.right = _autoVisibleRect.right + r.right;
				a.top = _autoVisibleRect.top + r.top;
				a.bottom = _autoVisibleRect.bottom + r.bottom;
				SetRect(a);
				_vScrollBar->ShowWindow(true);
			}
		}
		RFRect r = ClientToScreen();
		float top = r.top + _padding.top;
		float bottom = r.bottom - _padding.bottom;
		int total = _text983->TotalHeight983();
		_vScrollBar->SetScrollRange(0, total - (int)(bottom - top));
		_vScrollBar->_size983 = _220 * 2;
	}
	else
	{
		if (_showAlwaysV)
		{
			if (_vScrollBar && _vScrollBar->_enabled)
				_vScrollBar->EnableWindow(false);
		}
		else if (_autoVisibleV)
		{
			RFScrollBar* bar = _vScrollBar;
			if (!bar->_hiding && bar->_visible)
			{
				RFWnd* super = bar->_super;
				if (!super || super->IsWindowVisible(false))
				{
					RFRect r = GetRect();
					RFRect a;
					a.left = r.left - _autoVisibleRect.left;
					a.right = r.right - _autoVisibleRect.right;
					a.top = r.top - _autoVisibleRect.top;
					a.bottom = r.bottom - _autoVisibleRect.bottom;
					SetRect(a);
					_vScrollBar->ShowWindow(false);
				}
			}
		}
		_vScrollBar->SetScrollPos(0);
		_lineLimit983 = 0;
	}
}

// 0044e460
void RFTextAreaSC::OnHScrollPosChanged(EventArgs&)
{
}

// 0044e470: a posicao da barra e o deslocamento do texto (+0x1a8 = -pos)
void RFTextAreaSC::OnVScrollPosChanged(EventArgs& e)
{
	if (_vScrollBar && IsScrollBarAvailable(RFSCROLLBAR_VERT983))
	{
		_lineLimit983 = -static_cast<ScrollBarPosChangedEventArgs&>(e).pos;
		Invaildate();
	}
}

// 0044e4b0: a barra desenha com o recorte do pai do controle
void RFTextAreaSC::OnScrollDraw(EventArgs& e)
{
	static_cast<WindowEventArgs_<RFWnd>&>(e).window->_rc._clipRect = _super->_rc._clipRect;
	e.handled = true;
}
