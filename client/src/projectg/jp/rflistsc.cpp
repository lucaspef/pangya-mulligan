// JP 983: RFListSC.cpp da Refresh (B162, F3.7.13 item 5): a lista com barras de rolagem (00450080–00450e65; tipo "ListBox"
// do XML). As barras (<control type="HSCROLLBAR"/"VSCROLLBAR">) são sub-controles criados pela fábrica do tipo, com rolagem
// suave; a posição da barra vira −_scrollPos983 da RFList; faixa = tamanho de todos os itens − área cliente (menos o _padding);
// o passo (botões/roda) é o tamanho do <itemrect> ou o SetScrollSize. ToString/GetTypeName/Get?ScrollBar em linha no rfcontrols.h.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"
#include "refresh/rffactorymanager.h"

extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A802 (temporário)

// 00450080
RFListSC::RFListSC()
{
}

// 004500a0
RFListSC::~RFListSC()
{
}

// 004500b0 (o 983 pega a fábrica pelo RFSingleton<RFWindowFactoryManager>; aqui pelo _factoryManager do RFManager, o mesmo objeto)
void RFListSC::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFList::Create(base, parent);
	_hScrollBar = 0;
	_vScrollBar = 0;
	ListSCBase* list = static_cast<ListSCBase*>(base);
	if (list->_hScrollbar)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)list->_hScrollbar->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 45 "src\\RFListSC.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(list->_hScrollbar, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_hScrollBar = static_cast<RFScrollBar*>(wnd);
		_hScrollBar->_smoothScroll = true;
	}
	if (list->_vScrollbar)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)list->_vScrollbar->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 58 "src\\RFListSC.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(list->_vScrollbar, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_vScrollBar = static_cast<RFScrollBar*>(wnd);
		_vScrollBar->_smoothScroll = true;
	}
}

// 00450270: barras começam vazias e desabilitadas (o UpdateScrollbar habilita)
void RFListSC::PreInitialize()
{
	RFList::PreInitialize();
	_scrollSize983 = (unsigned int)-1;
	if (_hScrollBar)
	{
		_hScrollBar->SetScrollRange(0, 0);
		if (_hScrollBar)
			_hScrollBar->EnableWindow(false);
		RegisterTo(_hScrollBar->_name, RFScrollBar::EventPosChanged, Subscriber(&RFListSC::OnHScrollPosChanged, this));
		RegisterTo(_hScrollBar->_name, RFWnd::EventDraw, Subscriber(&RFListSC::OnScrollDraw, this));
	}
	if (_vScrollBar)
	{
		_vScrollBar->SetScrollRange(0, 0);
		if (_vScrollBar)
			_vScrollBar->EnableWindow(false);
		RegisterTo(_vScrollBar->_name, RFScrollBar::EventPosChanged, Subscriber(&RFListSC::OnVScrollPosChanged, this));
		RegisterTo(_vScrollBar->_name, RFWnd::EventDraw, Subscriber(&RFListSC::OnScrollDraw, this));
	}
}

// 004505a0: as barras contam como parte da lista
bool RFListSC::HitTest(const RFVector2& pt, bool a2)
{
	if (!_visible || !_hitTestVisible)
		return false;
	if (!a2 && !_enabled)
		return false;
	if (_vScrollBar && _vScrollBar->HitTest(pt, a2))
		return true;
	if (_hScrollBar && _hScrollBar->HitTest(pt, a2))
		return true;
	return ClientToScreen().PtInRect(pt);
}

// 00450640
void RFListSC::LockScrollbar(RFSCROLLBAR_TYPE type, bool lock)
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

// 004506d0: os itens passam da área cliente (menos o _padding)
bool RFListSC::IsScrollBarAvailable(RFSCROLLBAR_TYPE type) const
{
	RFRect area(0.0f, 0.0f, GetSize().width, GetSize().height);
	area.left += _padding.left;
	area.top += _padding.top;
	area.right -= _padding.right;
	area.bottom -= _padding.bottom;
	if (type == RFSCROLLBAR_HORZ983)
	{
		if (_hScrollBar && GetAllItemsWidth() > area.right - area.left)
			return true;
	}
	else if (type == RFSCROLLBAR_VERT983 && _vScrollBar)
	{
		if (GetAllItemsHeight() > area.bottom - area.top)
			return true;
	}
	return false;
}

// 004507b0
void RFListSC::ScrollToTop()
{
	if (_vScrollBar)
		_vScrollBar->SetScrollPos(_vScrollBar->_range.x);
	if (_hScrollBar)
		_hScrollBar->SetScrollPos(_hScrollBar->_range.x);
}

// 004507f0
void RFListSC::ScrollToBottom()
{
	if (_vScrollBar)
		_vScrollBar->SetScrollPos(_vScrollBar->_range.y);
	if (_hScrollBar)
		_hScrollBar->SetScrollPos(_hScrollBar->_range.y);
}

// 00450830: o item no começo da área cliente
void RFListSC::ScrollToItem(int index)
{
	if (_items.empty() || index < 0 || index >= (int)_items.size())
		return;
	RFRect rect = GetScreenRect();
	rect.left += _padding.left;
	rect.top += _padding.top;
	if (index >= (int)_items.size())
		return;
	if (IsScrollBarAvailable(RFSCROLLBAR_VERT983))
		_vScrollBar->SetScrollPos((int)(_items[index]->GetPos().y - rect.top - _scrollPos983.y));
	if (IsScrollBarAvailable(RFSCROLLBAR_HORZ983))
		_hScrollBar->SetScrollPos((int)(_items[index]->GetPos().x - rect.left - _scrollPos983.x));
}

// 00450930: faixa = todos os itens − área cliente; sem rolagem, a barra desabilita e volta a 0
void RFListSC::UpdateScrollbar()
{
	RFRect area(0.0f, 0.0f, GetSize().width, GetSize().height);
	area.left += _padding.left;
	area.top += _padding.top;
	area.right -= _padding.right;
	area.bottom -= _padding.bottom;
	if (_vScrollBar)
	{
		if (IsScrollBarAvailable(RFSCROLLBAR_VERT983))
		{
			if (_vScrollBar)
				_vScrollBar->EnableWindow(true);
			_vScrollBar->SetScrollRange(0, (int)GetAllItemsHeight() - (int)(area.bottom - area.top));
			_vScrollBar->_size983 = (int)GetScrollSize();
		}
		else
		{
			if (_vScrollBar)
				_vScrollBar->EnableWindow(false);
			_vScrollBar->SetScrollPos(0);
			_scrollPos983.y = 0.0f;
		}
	}
	if (_hScrollBar)
	{
		if (IsScrollBarAvailable(RFSCROLLBAR_HORZ983))
		{
			if (_hScrollBar)
				_hScrollBar->EnableWindow(true);
			_hScrollBar->SetScrollRange(0, (int)GetAllItemsWidth() - (int)(area.right - area.left));
			_hScrollBar->_size983 = (int)GetScrollSize();
		}
		else
		{
			if (_hScrollBar)
				_hScrollBar->EnableWindow(false);
			_hScrollBar->SetScrollPos(0);
			_scrollPos983.x = 0.0f;
		}
	}
}

// 00450b20
void RFListSC::OnItemAdded()
{
	UpdateItemPosition();
	UpdateScrollbar();
}

// 00450b30
void RFListSC::OnItemRemoved()
{
	UpdateItemPosition();
	UpdateScrollbar();
}

// 00450b40
void RFListSC::OnUpdate(UpdateEventArgs& e)
{
	RFWnd::OnUpdate(e);
	UpdateItemPosition();
	UpdateScrollbar();
}

// 00450b70
void RFListSC::SetScrollSize(unsigned int size)
{
	_scrollSize983 = size;
}

// 00450b80: −1 = o tamanho do <itemrect> no sentido da lista (0 sem barra útil)
float RFListSC::GetScrollSize()
{
	float size = 0.0f;
	if (_scrollSize983 == (unsigned int)-1)
	{
		RFRect area(0.0f, 0.0f, GetSize().width, GetSize().height);
		if (_orientation == OrientationTypes::Vertical)
		{
			if (_vScrollBar && IsScrollBarAvailable(RFSCROLLBAR_VERT983))
				return _itemSize.height;
		}
		else if (_hScrollBar && IsScrollBarAvailable(RFSCROLLBAR_HORZ983))
			return _itemSize.width;
	}
	else
		size = (float)_scrollSize983;
	return size;
}

// 00450c30: a roda anda um passo pela rolagem suave da barra (giro < 0 = para baixo/direita)
void RFListSC::OnMouseWheel(MouseEventArgs& e)
{
	RFWnd::OnMouseWheel(e);
	RFScrollBar* bar;
	if (_orientation == OrientationTypes::Vertical)
	{
		if (!_vScrollBar || !IsScrollBarAvailable(RFSCROLLBAR_VERT983))
			return;
		bar = _vScrollBar;
	}
	else
	{
		if (!_hScrollBar || !IsScrollBarAvailable(RFSCROLLBAR_HORZ983))
			return;
		bar = _hScrollBar;
	}
	float step = GetScrollSize();
	int pos = bar->_pos983;
	if (_name == L"card_list")
		jp983_rastro("A802 roda giro=%d pos=%d range=%d..%d passo=%d itens=%d alt=%d", (int)e.wheelDelta983, pos, bar->_range.x,
			bar->_range.y, (int)step, (int)_items.size(), (int)GetAllItemsHeight());
	if (e.wheelDelta983 < 0.0f)
	{
		if (pos >= bar->_range.y)
			return;
		bar->MoveToScroll(pos + (int)step);
	}
	else
	{
		if (pos <= bar->_range.x)
			return;
		bar->MoveToScroll(pos - (int)step);
	}
	e.handled = true;
}

// 00450da0
void RFListSC::OnHScrollPosChanged(EventArgs& e)
{
	if (_hScrollBar && IsScrollBarAvailable(RFSCROLLBAR_HORZ983) && !_items.empty())
		_scrollPos983.x = -(float)static_cast<ScrollBarPosChangedEventArgs&>(e).pos;
}

// 00450de0
void RFListSC::OnVScrollPosChanged(EventArgs& e)
{
	if (_vScrollBar && IsScrollBarAvailable(RFSCROLLBAR_VERT983) && !_items.empty())
		_scrollPos983.y = -(float)static_cast<ScrollBarPosChangedEventArgs&>(e).pos;
}

// 00450e20: a barra desenha com o recorte do pai da lista (fora do recorte dos itens)
void RFListSC::OnScrollDraw(EventArgs& e)
{
	static_cast<WindowEventArgs_<RFWnd>&>(e).window->_rc._clipRect = _super->_rc._clipRect;
	e.handled = true;
}
