// JP 983: RFLabelButton.cpp da Refresh (B160, F3.7.13 item 5): o botão com rótulo (00425f50–004261cd). O rótulo é um RFLabel
// filho (subcontrole) criado pela fábrica a partir do <control type="LABEL"> da estrutura; a cor do texto acompanha o estado do
// botão (NormalColor/HoverColor/PushColor/DisableColor) quando o XML traz as cores. ToString/GetTypeName em linha no rfcontrols.h.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <assert.h>
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rffactorymanager.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

// 00425f50
RFLabelButton::RFLabelButton()
{
}

// 00425fb0
RFLabelButton::~RFLabelButton()
{
}

// 00425fc0: as cores e o rótulo filho
void RFLabelButton::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFButton::Create(base, parent);
	if (!base)
		return;
	LabelButtonBase* button = static_cast<LabelButtonBase*>(base);
	_1fc = button->_hasNormalColor;
	_normalColor983 = button->_normalColor.Get();
	_overColor983 = button->_hoverColor.Get();
	_downColor983 = button->_pushColor.Get();
	_disableColor983 = button->_disableColor.Get();
	RFManager& manager = RFManager::GetInstance();
	const std::wstring& type = manager.GetCompatibleControlName((WndType)button->_label->GetType());
	RFWindowFactory* factory = manager._factoryManager->GetFactory(type);
	assert(factory);
	RFWnd* wnd = factory->Create();
	wnd->Create(button->_label, this);
	wnd->Initialize();
	wnd->_subControl = true;
	_labelWnd983 = static_cast<RFLabel*>(wnd);
}

// 004260e0
void RFLabelButton::PreInitialize()
{
	RFButton::PreInitialize();
	_1f8 = _state;
	if (_1fc && _labelWnd983)
		_labelWnd983->SetColor(_normalColor983);
}

// 00426120 (o RFLabel::SetLabel do rótulo interno vem embutido)
void RFLabelButton::SetLabel(const std::wstring& label)
{
	RFLabel* labelWnd = _labelWnd983;
	labelWnd->_label.assign(label, 0, std::wstring::npos);
	if (labelWnd->_autoFit)
		labelWnd->FitToContent();
}

// 00426160
const std::wstring& RFLabelButton::GetLabel() const
{
	return _labelWnd983->_label;
}

// 00426170: estado novo do botão = cor nova do texto
void RFLabelButton::OnUpdate(UpdateEventArgs&)
{
	if (!_labelWnd983 || !_1fc || _1f8 == _state)
		return;
	switch (_state)
	{
	case RFBS_NORMAL983:
		_labelWnd983->SetColor(_normalColor983);
		break;
	case RFBS_DOWN983:
		_labelWnd983->SetColor(_downColor983);
		break;
	case RFBS_OVER983:
		_labelWnd983->SetColor(_overColor983);
		break;
	case RFBS_DISABLE983:
		_labelWnd983->SetColor(_disableColor983);
		break;
	}
	_1f8 = _state;
}
