// JP 983: RFList.cpp da Refresh (B162, F3.7.13 item 5): a lista (00422660–00424dd9) e os itens RFListItem/RFListItemWnd
// (00421cf0–004225a0). Itens em grade de _columnCount por linha (vertical) ou coluna (horizontal), posicionados a cada quadro
// no OnUpdate a partir da posição na tela + _scrollPos983; desenho com recorte na área cliente menos o _padding e
// EventBeginDrawItem/EventDrawItem (só os itens que cruzam o recorte)/EventEndDrawItem. Itens janela (InsertWindowItem) são
// filhos da lista e trocam de animação (_Normal_/_Selected_/_Over_/_Disabled_).
// Acessores em linha no rfcontrols.h; strings (eventos e TypeName dos itens) no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <string>
#include <vector>
#include <algorithm>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// ---- RFListItemWnd ----

// 00421e10
bool RFListItemWnd::HitTest(const RFVector2& pt)
{
	if (IsVisible983())
		return _wnd->HitTest(pt, false);
	return false;
}

// 00421e40
void RFListItemWnd::Show983(bool show)
{
	_wnd->ShowWindow(show);
}

// 00421e50
bool RFListItemWnd::IsVisible983() const
{
	return _wnd->IsWindowVisible(false);
}

// 00421e80
void RFListItemWnd::Select(bool select)
{
	_selected = select;
	if (select)
		_wnd->PlayAnimation(L"_Selected_");
	else
		_wnd->PlayAnimation(L"_Normal_");
}

// 00421f60: desabilitado não muda; selecionado fica com a animação de seleção
void RFListItemWnd::SetOver983(bool over)
{
	if (IsDisabled983())
		return;
	_over983 = over;
	if (over)
	{
		if (!IsSelected())
			_wnd->PlayAnimation(L"_Over_");
	}
	else
	{
		if (!IsSelected())
			_wnd->PlayAnimation(L"_Normal_");
	}
}

// 00422070
void RFListItemWnd::Disable983(bool disable)
{
	_disabled983 = disable;
	if (disable)
		_wnd->PlayAnimation(L"_Disabled_");
	else
		_wnd->PlayAnimation(L"_Normal_");
	_wnd->EnableWindow(!disable);
}

// 004221a0
void RFListItemWnd::SetPos(const RFVector2& pos)
{
	_pos = pos;
	_wnd->SetScreenPos(pos.x, pos.y);
}

// 004221d0: a posição na tela da janela (soma a de todos os pais)
const RFVector2& RFListItemWnd::GetPos()
{
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = _wnd; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	_pos = pos;
	return _pos;
}

// 00422230
void RFListItemWnd::SetSize(const RFSize& size)
{
	_size = size;
	_wnd->SetSize(size);
}

// 00422290
const RFSize& RFListItemWnd::GetSize()
{
	_size = _wnd->GetSize();
	return _size;
}

// 004222c0
void RFListItemWnd::SetAlpha(float alpha)
{
	_wnd->SetAlpha(alpha);
}

// 00422320
float RFListItemWnd::GetAlpha() const
{
	return _wnd->GetAlpha(false);
}

// ---- RFListItem ----

// 00422420
bool RFListItem::HitTest(const RFVector2& pt)
{
	float right = _pos.x + _size.width;
	float bottom = _pos.y + _size.height;
	return _pos.x <= pt.x && pt.x <= right && _pos.y <= pt.y && pt.y <= bottom;
}

// ---- RFList ----

// predicado do find_if (00424a30; o find_if é o 00425600): item visível sob o ponto
struct ItemOnPosition983
{
	explicit ItemOnPosition983(const RFVector2& pt_) : pt(pt_) { }
	bool operator()(RFListItemBase* item) const { return item->IsVisible983() && item->HitTest(pt); }

	RFVector2 pt;
};

// predicado do DeleteItem(void*) (00424c80)
struct ItemData983
{
	explicit ItemData983(void* data_) : data(data_) { }
	bool operator()(RFListItemBase* item) const { return item->GetData() == data; }

	void* data;
};

// functor do for_each do OnDraw (construtor 00423900, chamada 00423930; o for_each é o 00425120)
struct DrawItem983
{
	DrawItem983(int* index_, RFList* list_, const RFRect& clip_) : index(index_), list(list_), clip(clip_) { }
	void operator()(RFListItemBase* item)
	{
		float x = item->GetPos().x;
		float y = item->GetPos().y;
		RFRect rect(x, y, x + item->GetSize().width, y + item->GetSize().height);
		RFRect out;
		if (out.IntersectRect(clip, rect))
		{
			item->SetDrawIndex983((*index)++);
			ListDrawItemEventArgs e(list);
			e.pos = item->GetPos();
			e.item = item;
			list->SendEvent(RFList::EventDrawItem, e);
		}
	}

	int* index;
	RFList* list;
	RFRect clip;
};

// functor do for_each do DeleteAllItems (00424d50; o for_each é o 00425690)
struct DeleteItem983
{
	explicit DeleteItem983(RFList* list_) : list(list_) { }
	void operator()(RFListItemBase* item)
	{
		if (item->GetTypeName() == RFListItemWnd::TypeName)
			list->DeleteChild(static_cast<RFWnd*>(item->GetData()));
		delete item;
	}

	RFList* list;
};

// janela de item pelo nome do XML (004251a0: RFWndEx novo; o CreateWnd registra no pai)
template<class T> static T* CreateWnd983(RFManager* manager, const std::wstring& name, RFWnd* parent)
{
	T* wnd = new T;
	if (!manager->CreateWnd(name, wnd, parent))
	{
		delete wnd;
		return 0;
	}
	return wnd;
}

// 00422660 (o RFWndEx::RFWndEx embutido)
RFList::RFList() : _scrollPos983(0.0f, 0.0f)
{
}

// 004226e0
RFList::~RFList()
{
}

// 00422750 (o RFWndEx::Create embutido)
void RFList::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWndEx::Create(base, parent);
	ListBase* list = static_cast<ListBase*>(base);
	_padding.left = list->_padding.left;
	_padding.right = list->_padding.right;
	_padding.top = list->_padding.top;
	_padding.bottom = list->_padding.bottom;
	_columnCount = list->_column;
	_itemSize.width = list->_itemrect.right - list->_itemrect.left;
	_itemSize.height = list->_itemrect.bottom - list->_itemrect.top;
	_selectionMode = (ListSelectionTypes::Type)list->_selection;
	_deselectable = list->_deselectable;
	_reselectable = false;
	_rClickAction = (RFRCLICKACTION)list->_rclickaction;
	_orientation = (OrientationTypes::Type)list->_orientation.Get();
	_firstDrawItem = -1;
	_itemClip = true;
}

// 00422830 (o RFWndEx::PreInitialize embutido)
void RFList::PreInitialize()
{
	RFWndEx::PreInitialize();
}

// 00422890: apaga os itens (as janelas de item são filhas e saem com a lista) e o pincel (RFWndEx::OnDestroy embutido)
void RFList::OnDestroy()
{
	_selItems.clear();
	for (std::vector<RFListItemBase*>::iterator itor = _items.begin(); itor != _items.end(); ++itor)
		delete *itor;
	_items.clear();
	RFWndEx::OnDestroy();
}

// 00422950
void RFList::OnLayoutChanged(unsigned int flags)
{
	if (flags & 2)
		ApplyAnchor();
}

// 00422970
void RFList::EnableMultiSelect(bool enable)
{
	_selectionMode = (ListSelectionTypes::Type)enable;
}

// 00422990
void RFList::RelaseAllSelectedItem()
{
	for (unsigned int i = 0; i < _items.size(); ++i)
	{
		RFListItemBase* item = _items[i];
		if (item->IsSelected())
		{
			item->Select(false);
			ListSelChangedEventArgs e(this, item);
			SendEvent(EventSelChanged, e);
		}
	}
	_selItems.clear();
}

// 00422ac0
void RFList::RelaseAllOverItem()
{
	for (unsigned int i = 0; i < _items.size(); ++i)
		_items[i]->SetOver983(false);
}

// 00422b10
void RFList::OnItemAdded()
{
	UpdateItemPosition();
}

// 00422b20
void RFList::OnItemRemoved()
{
	UpdateItemPosition();
}

// 00422b30
void RFList::OnLButtonDown(MouseEventArgs& e)
{
	RFWnd::OnLButtonDown(e);
	e.handled = true;
	if (!_items.empty())
		ButtonDown_Impl(e);
}

// 00422b70
void RFList::OnRButtonDown(MouseEventArgs& e)
{
	RFWnd::OnRButtonDown(e);
	e.handled = true;
	if (!_items.empty())
		ButtonDown_Impl(e);
}

// 00422bc0
void RFList::OnMouseEnter()
{
	RFWnd::OnMouseEnter();
}

// 00422c30
void RFList::OnMouseLeave()
{
	RFWnd::OnMouseLeave();
	RelaseAllOverItem();
}

// 00422cc0: só um item "sobre" por vez
void RFList::OnMouseMove(MouseEventArgs& e)
{
	RFWnd::OnMouseMove(e);
	e.handled = true;
	if (_items.empty())
		return;
	std::vector<RFListItemBase*>::iterator itor = std::find_if(_items.begin(), _items.end(), ItemOnPosition983(e.pos));
	RFListItemBase* item;
	if (itor == _items.end() || !(item = *itor))
	{
		RelaseAllOverItem();
		return;
	}
	if (!item->IsOver983())
	{
		RelaseAllOverItem();
		item->SetOver983(true);
	}
}

// 00422d40
void RFList::ReindexItems()
{
	int index = 0;
	for (std::vector<RFListItemBase*>::iterator itor = _items.begin(); itor != _items.end(); ++itor)
		(*itor)->SetIndex(index++);
}

// 00422da0
void RFList::DeleteFromSelList(RFListItemBase* item)
{
	std::vector<RFListItemBase*>::iterator itor = std::find(_selItems.begin(), _selItems.end(), item);
	if (itor != _selItems.end())
		_selItems.erase(itor);
}

// 00422e00: botão direito com _rClickAction 1 não seleciona; EventSelChanged se mudou (ou se reselecionável)
void RFList::ButtonDown_Impl(MouseEventArgs& e)
{
	if (e._14 == 1 && _rClickAction == RFRCLICKACTION_NONE983)
		return;
	std::vector<RFListItemBase*>::iterator itor = std::find_if(_items.begin(), _items.end(), ItemOnPosition983(e.pos));
	if (itor == _items.end())
		return;
	RFListItemBase* item = *itor;
	if (!item)
		return;
	bool selected = item->IsSelected();
	if (_selectionMode == ListSelectionTypes::Multiple983)
		MultipleSelectionProc(item, e);
	else
		SingleSelectionProc(item, e);
	UpdateSelectList();
	if (selected == item->IsSelected())
	{
		if (!item->IsSelected())
			return;
		if (!_reselectable)
			return;
	}
	ListSelChangedEventArgs changed(this, item);
	SendEvent(EventSelChanged, changed);
}

// 00422f50
void RFList::MultipleSelectionProc(RFListItemBase* item, MouseEventArgs& e)
{
	if (!item->IsSelected())
		item->Select(true);
	else if (e._14 == 0)
		item->Select(false);
}

// 00422f90
void RFList::SingleSelectionProc(RFListItemBase* item, MouseEventArgs& e)
{
	if (_deselectable && e._14 == 0)
	{
		if (item->IsSelected())
		{
			item->Select(false);
			return;
		}
	}
	else if (item->IsSelected())
		return;
	RelaseAllSelectedItem();
	item->Select(true);
}

// 00423000
void RFList::UpdateSelectList()
{
	_selItems.clear();
	for (unsigned int i = 0; i < _items.size(); ++i)
	{
		if (_items[i]->IsSelected())
			_selItems.push_back(_items[i]);
	}
}

// 00423180
void RFList::OnUpdate(UpdateEventArgs& e)
{
	RFWnd::OnUpdate(e);
	UpdateItemPosition();
}

// 004231a0
void RFList::UpdateItemPosition()
{
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	pos.x = _scrollPos983.x + pos.x;
	pos.y = _scrollPos983.y + pos.y;
	if (_orientation == OrientationTypes::Vertical)
		VericalAlignment(pos);
	else if (_orientation == OrientationTypes::Horizontal)
		HorizontalAlignment(pos);
}

// 00423240: linhas de _columnCount itens; a altura da linha é a do maior item; _firstDrawItem = 1º que cruza a área cliente
// (com _columnCount <= 0 o 983 fica preso no laço)
void RFList::VericalAlignment(const RFVector2& pos)
{
	RFRect client = ClientToScreen();
	client.left += _padding.left;
	client.top += _padding.top;
	client.right -= _padding.right;
	client.bottom -= _padding.bottom;
	int first = -1;
	float y = 0.0f;
	for (int row = 0; ; ++row)
	{
		float x = 0.0f;
		float height = 0.0f;
		for (int col = 0; col < _columnCount; ++col)
		{
			int index = _columnCount * row + col;
			if (index >= (int)_items.size())
				return;
			RFListItemBase* item = _items[index];
			item->SetAlpha(GetAlpha(false));
			item->SetPos(RFVector2(pos.x + _padding.left + x, _padding.top + pos.y + y));
			x += item->GetSize().width;
			if (height < item->GetSize().height)
				height = item->GetSize().height;
			if (first == -1)
			{
				float left = item->GetPos().x;
				float top = item->GetPos().y;
				RFRect rect(left, top, left + item->GetSize().width, top + item->GetSize().height);
				RFRect out;
				if (out.IntersectRect(client, rect))
				{
					first = index;
					_firstDrawItem = index;
				}
			}
		}
		y += height;
	}
}

// 004234a0: colunas de _columnCount itens; a largura da coluna é a do maior item
void RFList::HorizontalAlignment(const RFVector2& pos)
{
	RFRect client = ClientToScreen();
	client.left += _padding.left;
	client.top += _padding.top;
	client.right -= _padding.right;
	client.bottom -= _padding.bottom;
	int first = -1;
	float x = 0.0f;
	for (int column = 0; ; ++column)
	{
		float y = 0.0f;
		float width = 0.0f;
		for (int row = 0; row < _columnCount; ++row)
		{
			int index = _columnCount * column + row;
			if (index >= (int)_items.size())
				return;
			RFListItemBase* item = _items[index];
			item->SetAlpha(GetAlpha(false));
			item->SetPos(RFVector2(pos.x + _padding.left + x, _padding.top + pos.y + y));
			y += item->GetSize().height;
			if (item->GetSize().width > width)
				width = item->GetSize().width;
			if (first == -1)
			{
				float left = item->GetPos().x;
				float top = item->GetPos().y;
				RFRect rect(left, top, left + item->GetSize().width, top + item->GetSize().height);
				RFRect out;
				if (out.IntersectRect(client, rect))
				{
					first = index;
					_firstDrawItem = index;
				}
			}
		}
		x += width;
	}
}

// 00423700: recorte na área cliente menos o _padding (se _itemClip)
void RFList::OnDraw()
{
	RFWndEx::OnDraw();
	if (_items.empty())
		return;
	RFRect client = ClientToScreen();
	client.left += _padding.left;
	client.top += _padding.top;
	client.right -= _padding.right;
	client.bottom -= _padding.bottom;
	_rc._clip = _itemClip;
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, client);
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	WindowEventArgs_<RFList> e(this);
	SendEvent(EventBeginDrawItem, e);
	int index = 0;
	std::for_each(_items.begin(), _items.end(), DrawItem983(&index, this, client));
	SendEvent(EventEndDrawItem, e);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, client);
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
	_rc._clip = !_itemClip;
}

// 00423a60
void RFList::OnItemMouseLeave(EventArgs& e)
{
	RelaseAllOverItem();
}

// 00423a70: vertical = a linha mais larga; horizontal = soma das larguras das colunas
float RFList::GetAllItemsWidth() const
{
	float total = 0.0f;
	int columns = _columnCount;
	if (_orientation == OrientationTypes::Vertical)
	{
		for (int i = 0; ; i += columns)
		{
			float width = 0.0f;
			for (int j = 0, k = i; j < columns; ++j, ++k)
			{
				if (k >= (int)_items.size())
					return width > total ? width : total;
				width = _items[k]->GetSize().width + width;
			}
			if (width > total)
				total = width;
		}
	}
	else
	{
		for (int i = 0; ; i += columns)
		{
			float width = 0.0f;
			for (int j = 0, k = i; j < columns; ++j, ++k)
			{
				if (k >= (int)_items.size())
					return width + total;
				RFListItemBase* item = _items[k];
				if (width < item->GetSize().width)
					width = item->GetSize().width;
			}
			total = width + total;
		}
	}
}

// 00423bc0: retângulo do item (coluna, linha) relativo ao deslocamento de rolagem
RFRect RFList::GetItemRect(int column, int row, bool update)
{
	RFRect rect;
	int index = _columnCount * row + column;
	if (index >= 0 && index < (int)_items.size())
	{
		if (update)
			UpdateItemPosition();
		rect.left = _items[index]->GetPos().x - _scrollPos983.x;
		rect.top = _items[index]->GetPos().y - _scrollPos983.y;
		rect.right = _items[index]->GetSize().width + rect.left;
		rect.bottom = _items[index]->GetSize().height + rect.top;
	}
	return rect;
}

// 00423c80: vertical = soma das alturas das linhas; horizontal = a coluna mais alta
float RFList::GetAllItemsHeight() const
{
	float total = 0.0f;
	int columns = _columnCount;
	if (_orientation == OrientationTypes::Vertical)
	{
		for (int i = 0; ; i += columns)
		{
			float height = 0.0f;
			for (int j = 0, k = i; j < columns; ++j, ++k)
			{
				if (k >= (int)_items.size())
					return height + total;
				RFListItemBase* item = _items[k];
				if (height < item->GetSize().height)
					height = item->GetSize().height;
			}
			total = height + total;
		}
	}
	else
	{
		for (int i = 0; ; i += columns)
		{
			float height = 0.0f;
			for (int j = 0, k = i; j < columns; ++j, ++k)
			{
				if (k >= (int)_items.size())
					return height > total ? height : total;
				height = _items[k]->GetSize().height + height;
			}
			if (height > total)
				total = height;
		}
	}
}

// 00423dd0
OrientationTypes::Type RFList::GetOrientation() const
{
	return _orientation;
}

// 00423de0
void RFList::SetOrientation(OrientationTypes::Type orientation)
{
	_orientation = orientation;
}

// 00423df0: item de dados do tamanho do <itemrect>
void RFList::InsertItem(void* data)
{
	RFListItemBase* item = new RFListItem;
	item->SetData(data);
	item->SetIndex(_items.size());
	item->Show983(true);
	item->Select(false);
	RFSize size = _itemSize;
	item->SetSize(size);
	item->SetAlpha(GetAlpha(false));
	_items.push_back(item);
	OnItemAdded();
}

// 00423f70
RFWnd* RFList::InsertWindowItem(const std::wstring& name)
{
	RFWnd* wnd = CreateWnd983<RFWndEx>(RFSingleton<RFManager>::GetInstancePtr(), name, this);
	InsertWindowItem(wnd);
	return wnd;
}

// 00423fa0
RFWnd* RFList::InsertWindowItem(RFWnd* wnd)
{
	RFListItemBase* item = InsertWindowItem_Impl(wnd);
	_items.push_back(item);
	OnItemAdded();
	return wnd;
}

// 00424040
RFWnd* RFList::InsertWindowItem(const std::wstring& name, unsigned int index)
{
	if (index > _items.size())
		return 0;
	RFWnd* wnd = CreateWnd983<RFWndEx>(RFSingleton<RFManager>::GetInstancePtr(), name, this);
	InsertWindowItem(wnd, index);
	return wnd;
}

// 00424090: entra no fim e roda até o índice (os índices dos itens não são refeitos, como no 983)
RFWnd* RFList::InsertWindowItem(RFWnd* wnd, unsigned int index)
{
	RFListItemBase* item = InsertWindowItem_Impl(wnd);
	_items.push_back(item);
	std::rotate(_items.begin() + index, _items.end() - 1, _items.end());
	OnItemAdded();
	return wnd;
}

// 00424100: a janela vira filha (sub-controle, recorte da lista) e avisa a lista ao perder o mouse
RFListItemWnd* RFList::InsertWindowItem_Impl(RFWnd* wnd)
{
	if (wnd->_super != this)
	{
		// C294.1: coluna criada com o dialogo como pai (Status/Tile da maestria) e depois inserida aqui ficava nos _children dos dois
		// e era destruida duas vezes ao fechar (AV em DeleteAllChildren); passa a ter um dono so (a lista)
		if (wnd->_super)
			wnd->_super->EjectChild_(wnd);
		RegisterChild(wnd);
	}
	wnd->_subControl = true;
	wnd->_rc._clip = _itemClip;
	wnd->RegisterTo(L"", RFWnd::EventMouseLeave, Subscriber(&RFList::OnItemMouseLeave, this));
	RFListItemWnd* item = new RFListItemWnd;
	item->SetData(wnd);
	item->SetIndex(_items.size());
	item->Select(false);
	item->SetSize(wnd->GetSize());
	item->SetAlpha(GetAlpha(false));
	item->Show983(true);
	return item;
}

// 00424340
void RFList::SetItemData(int index, void* data)
{
	if (IsValidIndex(index))
	{
		_items[index]->SetData(data);
		return;
	}
#line 659 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
}

// 004243a0
void* RFList::GetItemData(int index)
{
	if (IsValidIndex(index))
		return _items[index]->GetData();
#line 670 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
	return 0;
}

// 004243f0
RFListItemBase* RFList::GetListItem(int index)
{
	if (IsValidIndex(index))
		return _items[index];
#line 681 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
	return 0;
}

// 00424440
void RFList::SelectItem(int index, bool notify)
{
	if (IsValidIndex(index))
	{
		RFListItemBase* item = _items[index];
		if (!item->IsSelected())
		{
			item->Select(true);
			_selItems.push_back(item);
			if (notify)
			{
				ListSelChangedEventArgs e(this, item);
				SendEvent(EventSelChanged, e);
			}
		}
		return;
	}
#line 692 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
}

// 00424540: todos os itens com o dado
void RFList::SelectItem(void* data, bool notify)
{
	for (int i = 0; i < (int)_items.size(); ++i)
	{
		RFListItemBase* item = _items[i];
		if (item->GetData() == data && !item->IsSelected())
		{
			item->Select(true);
			_selItems.push_back(item);
			if (notify)
			{
				ListSelChangedEventArgs e(this, item);
				SendEvent(EventSelChanged, e);
			}
		}
	}
}

// 004246e0
void RFList::UnselectItem(int index, bool notify)
{
	if (IsValidIndex(index))
	{
		RFListItemBase* item = _items[index];
		if (item->IsSelected())
		{
			item->Select(false);
			DeleteFromSelList(item);
			if (notify)
			{
				ListSelChangedEventArgs e(this, item);
				SendEvent(EventSelChanged, e);
			}
		}
		return;
	}
#line 736 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
}

// 004247d0
void RFList::UnselectItem(void* data, bool notify)
{
	for (unsigned int i = 0; i < _items.size(); ++i)
	{
		RFListItemBase* item = _items[i];
		if (item->GetData() == data && item->IsSelected())
		{
			item->Select(false);
			DeleteFromSelList(item);
			if (notify)
			{
				ListSelChangedEventArgs e(this, item);
				SendEvent(EventSelChanged, e);
			}
		}
	}
}

// 00424920: o 983 compara o próprio item (não o dado) com o argumento
void RFList::EnableItem(void* item, bool enable)
{
	for (unsigned int i = 0; i < _items.size(); ++i)
	{
		if (_items[i] == item)
		{
			_items[i]->Disable983(!enable);
			return;
		}
	}
}

// 00424970
RFListItemBase* RFList::GetItemUnderCursor()
{
	return GetItemOnPosition_Impl(RFManager::GetInstance().GetCursorPos());
}

// 004249d0
RFListItemBase* RFList::GetItemOnPosition_Impl(const RFVector2& pos)
{
	std::vector<RFListItemBase*>::iterator itor = std::find_if(_items.begin(), _items.end(), ItemOnPosition983(pos));
	if (itor != _items.end())
		return *itor;
	return 0;
}

// 00424a60: item janela leva a janela junto
void RFList::DeleteItem(int index)
{
	if (IsValidIndex(index))
	{
		std::vector<RFListItemBase*>::iterator itor = _items.begin() + index;
		DeleteFromSelList(*itor);
		if ((*itor)->GetTypeName() == RFListItemWnd::TypeName)
			DeleteChild(static_cast<RFWnd*>((*itor)->GetData()));
		if (*itor)
		{
			delete *itor;
			*itor = 0;
		}
		_items.erase(itor);
		ReindexItems();
		OnItemRemoved();
		return;
	}
#line 814 "src\\RFList.cpp"
	assert(!"Parameter 'index' out of range.");
}

// 00424b70
void RFList::DeleteItem(void* data)
{
	std::vector<RFListItemBase*>::iterator itor = std::find_if(_items.begin(), _items.end(), ItemData983(data));
	if (itor == _items.end())
		return;
	DeleteFromSelList(*itor);
	if ((*itor)->GetTypeName() == RFListItemWnd::TypeName)
		DeleteChild(static_cast<RFWnd*>((*itor)->GetData()));
	if (*itor)
	{
		delete *itor;
		*itor = 0;
	}
	_items.erase(itor);
	ReindexItems();
	OnItemRemoved();
}

// 00424ca0
void RFList::DeleteAllItems()
{
	_selItems.clear();
	std::for_each(_items.begin(), _items.end(), DeleteItem983(this));
	_items.clear();
	OnItemRemoved();
}

// 00424db0
bool RFList::IsValidIndex(int index) const
{
	if (index >= 0 && index < (int)_items.size())
		return true;
	return false;
}

#if _MSC_VER > 1600
// C731: visão do jogo (rfbase.h m_items983 +0x180 do VC10) com o layout real da lib
int RFList::GetItemCount983()
{
	return (int)_items.size();
}
#endif
