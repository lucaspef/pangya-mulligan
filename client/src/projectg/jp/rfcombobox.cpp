// JP 983: RFComboBox.cpp da Refresh (B265, F3.7.13 item 5): a caixa de combinacao (00454950-00455b60; tipo "ComboBox" do XML),
// sem fonte no 645/839: escrita pelo asm do 983 (out/F3.7/s265/combo.asm). Tres filhos criados pela fabrica do tipo no Create:
// o botao (+0x178; abre/fecha a lista; _priority = -1), o TextAreaSC (+0x17c; mostra a selecao) e a ListSC (+0x180; itens = RFListItem
// com std::wstring* em GetData, alocadas aqui). Lista aberta: _priority -1 (desenha por cima) e _super->_priorityInvalid.
// O item sob o mouse/selecionado usa _selectColor983; o desenho do item e o proprio (OnListDrawItem, PrintText983 + recorte).
// ToString/GetTypeName/EnableMWheelSelect/GetListArea em linha no rfcontrols.h.
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
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"
#include "refresh/rffactorymanager.h"

extern const std::wstring g_rfEmptyString983; // 00e46f04 (rfwnd.cpp)
extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// 00454950 (so a string da fonte; o resto vem do Create/PreInitialize)
RFComboBox::RFComboBox()
{
}

// 00454980
RFComboBox::~RFComboBox()
{
}

// 004549d0 (a fabrica pelo _factoryManager do RFManager, o mesmo RFSingleton<RFWindowFactoryManager> do 983)
void RFComboBox::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_button = 0;
	_textArea = 0;
	_listArea = 0;
	ComboBase* combo = static_cast<ComboBase*>(base);
	_normalColor983 = combo->_normalColor.Get();
	_selectColor983 = combo->_selectColor.Get();
	_fontName983 = combo->_fontName.Get();
	_fontSize983 = combo->_fontSize.Get();
	if (combo->_textArea)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)combo->_textArea->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 53 "src\\RFComboBox.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(combo->_textArea, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_textArea = static_cast<RFTextAreaSC*>(wnd);
	}
	if (combo->_listArea)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)combo->_listArea->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 65 "src\\RFComboBox.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(combo->_listArea, this);
		wnd->Initialize();
		wnd->_subControl = true;
		_listArea = static_cast<RFListSC*>(wnd);
		_listArea->_reselectable = true;
	}
	if (combo->_button)
	{
		RFManager& manager = RFManager::GetInstance();
		const std::wstring& type = manager.GetCompatibleControlName((WndType)combo->_button->GetType());
		RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
#line 78 "src\\RFComboBox.cpp"
		assert(factory);
		RFWnd* wnd = factory->Create();
		wnd->Create(combo->_button, this);
		wnd->Initialize();
		wnd->_subControl = true;
		wnd->_priority = -1;
		if (wnd->_super)
			wnd->_super->_priorityInvalid = true;
		_button = static_cast<RFButton*>(wnd);
	}
}

// 00454ca0
void RFComboBox::Destroy()
{
	DeleteAllItems();
	RFWnd::Destroy();
}

// 00454cb0: o corpo do RFWnd::PreInitialize + a lista nasce escondida; liga o botao/texto (abre/fecha) e os 4 eventos da lista
void RFComboBox::PreInitialize()
{
	RFWnd::PreInitialize();
	_listShown = false;
	_curSel983 = -1;
	_mWheelSelect = true;
	_listArea->ShowWindow_(false);
	RegisterTo(_button->_name, RFButton::EventClicked, Subscriber(&RFComboBox::OnCBButClicked, this));
	RegisterTo(_textArea->_name, RFWnd::EventLButtonUp, Subscriber(&RFComboBox::OnCBButClicked, this));
	RegisterTo(_listArea->_name, RFList::EventSelChanged, Subscriber(&RFComboBox::OnListSelChanged, this));
	RegisterTo(_listArea->_name, RFList::EventBeginDrawItem, Subscriber(&RFComboBox::OnListBeginDrawItem, this));
	RegisterTo(_listArea->_name, RFList::EventEndDrawItem, Subscriber(&RFComboBox::OnListEndDrawItem, this));
	RegisterTo(_listArea->_name, RFList::EventDrawItem, Subscriber(&RFComboBox::OnListDrawItem, this));
}

// 00455080: o evento VisibleChanged (corpo do RFWnd::OnShow) e a lista fecha
void RFComboBox::OnShow(bool show)
{
	RFWnd::OnShow(show);
	_listShown = false;
	_priority = 0;
	if (_super)
		_super->_priorityInvalid = true;
	_listArea->ShowWindow(_listShown);
}

// 00455140: com toggle inverte; sem toggle fecha. Aberta = _priority -1 (por cima dos irmaos)
void RFComboBox::ShowListArea(bool toggle)
{
	if (toggle)
		_listShown = !_listShown;
	else
		_listShown = false;
	_priority = _listShown ? -1 : 0;
	if (_super)
		_super->_priorityInvalid = true;
	_listArea->ShowWindow(_listShown);
}

// 004551a0: a lista (aberta, fora do retangulo) conta como parte do controle
bool RFComboBox::HitTest(const RFVector2& pt, bool a2)
{
	if (!_visible || !_hitTestVisible)
		return false;
	if (!a2 && !_enabled)
		return false;
	if (_listArea->HitTest(pt, a2))
		return true;
	return ClientToScreen().PtInRect(pt);
}

// 00455220
void RFComboBox::AddText(const std::wstring& text)
{
	std::wstring* data = new std::wstring(text);
	_listArea->InsertItem(data);
}

// 004552b0: a selecao vira o texto do TextArea; a lista rola ate o item e o marca (so uma selecionada)
void RFComboBox::SetCurSel(int index)
{
	if (index < (int)_listArea->_items.size() && index >= 0 && _curSel983 != index)
	{
		_curSel983 = index;
		_textArea->ClearText();
		_textArea->AddPlainText(GetText(_curSel983));
		_listArea->RelaseAllSelectedItem();
		_listArea->ScrollToItem(_curSel983);
		_listArea->SelectItem(_curSel983, true);
	}
}

// 00455360
int RFComboBox::GetCurSel() const
{
	return _curSel983;
}

// 00455370 (o dado de cada item e o std::wstring* alocado no AddText)
const std::wstring& RFComboBox::GetText(int index) const
{
	if (index < (int)_listArea->_items.size() && index >= 0)
	{
		assert(index >= 0 && index < (int)_listArea->_items.size());
		return *(const std::wstring*)_listArea->_items[index]->GetData();
	}
	return g_rfEmptyString983;
}

// 004553f0
const std::wstring& RFComboBox::GetCurText() const
{
	return GetText(_curSel983);
}

// 00455460
int RFComboBox::GetItemCount() const
{
	return (int)_listArea->_items.size();
}

// 00455480: apaga os std::wstring* dos itens e esvazia a lista
void RFComboBox::DeleteAllItems()
{
	_curSel983 = -1;
	_textArea->ClearText();
	for (int i = 0; i < (int)_listArea->_items.size(); ++i)
	{
		assert(i >= 0 && i < (int)_listArea->_items.size());
		std::wstring* data = (std::wstring*)_listArea->_items[i]->GetData();
		delete data;
	}
	_listArea->DeleteAllItems();
}

// 00455570
bool RFComboBox::IsValidItemidx(int index) const
{
	return index >= 0 && index < (int)_listArea->_items.size();
}

// 004555a0: lista fechada: a roda troca a selecao (giro >= 0 = anterior); lista aberta: a propria lista rola, so marca tratado
void RFComboBox::OnMouseWheel(MouseEventArgs& e)
{
	if (!_mWheelSelect || _curSel983 == -1)
		return;
	RFListSC* list = _listArea;
	bool closed = list->_hiding || !list->_visible;
	if (!closed)
	{
		RFWnd* super = list->_super;
		if (!super || super->IsWindowVisible(false))
		{
			e.handled = true;
			return;
		}
	}
	if (0.0f <= e.wheelDelta983)
	{
		int prev = _curSel983 - 1;
		if (prev < 0)
			return;
		list->UnselectItem(_curSel983, false);
		list->ScrollToItem(_curSel983 - 1);
		list->SelectItem(_curSel983 - 1, true);
	}
	else
	{
		if (!IsValidItemidx(_curSel983 + 1))
			return;
		list->UnselectItem(_curSel983, false);
		list->ScrollToItem(_curSel983 + 1);
		list->SelectItem(_curSel983 + 1, true);
	}
	e.handled = true;
}

// 00455680
void RFComboBox::OnKillFocus()
{
	RFWnd::OnKillFocus();
	_listShown = false;
	_priority = 0;
	if (_super)
		_super->_priorityInvalid = true;
	_listArea->ShowWindow(_listShown);
}

// 004556c0 (corpo do ShowListArea(true))
void RFComboBox::OnCBButClicked(EventArgs&)
{
	_listShown = !_listShown;
	_priority = _listShown ? -1 : 0;
	if (_super)
		_super->_priorityInvalid = true;
	_listArea->ShowWindow(_listShown);
}

// 00455710: item marcado: vira _curSel, o texto do TextArea, a lista fecha e avisa o EventSelChanged
void RFComboBox::OnListSelChanged(EventArgs& e)
{
	RFListItemBase* item = static_cast<ListSelChangedEventArgs&>(e).item;
	std::wstring* data = (std::wstring*)item->GetData();
	if (item->IsSelected())
	{
		_curSel983 = item->GetIndex();
		_textArea->ClearText();
		std::wstring text(*data);
		_textArea->AddPlainText(text);
		_listShown = false;
		_priority = 0;
		if (_super)
			_super->_priorityInvalid = true;
		_listArea->ShowWindow(_listShown);
		WindowEventArgs_<RFComboBox> args(this);
		SendEvent(EventSelChanged, args);
	}
}

// 00455880
void RFComboBox::OnListBeginDrawItem(EventArgs&)
{
}

// 00455890: texto do item (fonte do combo; cor de selecao se sob o mouse/selecionado) na posicao do item, recortado pela area
// cliente da lista (menos o _padding); o recorte da lista passa a ser a intersecao
void RFComboBox::OnListDrawItem(EventArgs& e)
{
	ListDrawItemEventArgs& args = static_cast<ListDrawItemEventArgs&>(e);
	RFListItemBase* item = args.item;
	FontInfo font;
	font.name = _fontName983;
	font.size = _fontSize983;
	font.style = 0;
	font.outlineColor = 0;
	font.shadowColor = 0;
	if (!item->IsOver983() && !item->IsSelected())
		font.color = _normalColor983;
	else
		font.color = _selectColor983;
	const std::wstring* data = (const std::wstring*)item->GetData();
	RFListSC* list = _listArea;
	RFRect r = list->GetScreenRect();
	r.left += list->_padding.left;
	r.top += list->_padding.top;
	r.right -= list->_padding.right;
	r.bottom -= list->_padding.bottom;
	RFRect clip;
	clip.IntersectRect(list->_rc._clipRect, r);
	if (list->_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	std::wstring text(*data);
	int x = (int)args.pos.x;
	int y = (int)args.pos.y;
	RFManager::GetInstance()._interface->PrintText983(font, x, y, text, _rc._depth);
	RFRect clip2;
	clip2.IntersectRect(list->_rc._clipRect, r);
	list->_rc._clipRect = clip2;
	if (list->_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 00455b60
void RFComboBox::OnListEndDrawItem(EventArgs&)
{
}
