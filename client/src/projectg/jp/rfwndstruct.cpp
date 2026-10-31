// JP 983: Refresh — estruturas do leitor de layout (WndBaseStruct e as `*Base`, FrameImageSet, Testing::RF*Exception),
// 00439a80–0043dc30 + 00440290–00440410. F3.7.13 item (2), sessão B146. LTCG: prova pela dll de prova (out/F3.7/s146/
// RELATORIO.md). Sem assert na faixa: o arquivo do 983 não é conhecido (provavelmente cabeçalho do RFXmlLoader).
#include <string.h>
#include <windows.h>
#include "refresh/rfwndstruct.h"

// 00439db0
bool RFPropertyNameLess983::operator()(const std::string& left, const std::string& right) const
{
	if (left.size() == right.size())
		return _stricmp(left.c_str(), right.c_str()) < 0;
	return left.size() < right.size();
}

// 004394c0
WndBaseStruct::WndBaseStruct()
	: _x(0.0f), _y(0.0f)
{
	_isHitTestVisible.Set(true);
	_isHitTestThrough.Set(false);
	_clipChildren.Set(false);

	RF_REGISTER_PROPERTY983(_dragable, "Dragable");
	RF_REGISTER_PROPERTY983(_isHitTestVisible, "IsHitTestVisible");
	RF_REGISTER_PROPERTY983(_isHitTestThrough, "IsHitTestThrough");
	RF_REGISTER_PROPERTY983(_startPosition, "StartPosition");
	RF_REGISTER_PROPERTY983(_escKeyAction, "EscKeyAction");
	RF_REGISTER_PROPERTY983(_anchor, "Anchor");
	RF_REGISTER_PROPERTY983(_clipChildren, "ClipChildren");
}

// 00439a80
WndBaseStruct::~WndBaseStruct()
{
}

// 00439d00
void WndBaseStruct::AddProperty(RFProperty* property)
{
	if (property)
		_properties[property->GetName()] = property;
}

// 00439d80
RFProperty* WndBaseStruct::GetProperty(const std::string& name)
{
	PropertyMap::iterator it = _properties.find(name);
	if (it == _properties.end())
		return 0;
	return it->second;
}

// 0042ab40: wstring -> multibyte; o 983 le a pagina de codigo de um global (DAT_00f1de24, sem dono conhecido): usamos 932
// como WideToMultiByte983 (005e4d60). So o caminho de erro (RF*Exception) chama.
std::string RFWideToMulti983(const std::wstring& text)
{
	int n = WideCharToMultiByte(932, 0, text.c_str(), -1, NULL, 0, "?", NULL);
	std::string out(n > 0 ? n : 1, '\0');
	WideCharToMultiByte(932, 0, text.c_str(), -1, &out[0], n, "?", NULL);
	out.resize(strlen(out.c_str()));
	return out;
}

namespace Testing
{
	// 00439e90
	RFException::RFException(const std::wstring& message)
		: std::exception(RFWideToMulti983(message).c_str())
	{
	}

	// 00439f60
	RFInvalidArgException::RFInvalidArgException(const std::wstring& message)
		: std::exception(RFWideToMulti983(message).c_str())
	{
	}

	// 0043a030
	RFFileException::RFFileException(const std::wstring& message, int code)
		: std::exception(RFWideToMulti983(message).c_str()), _code(code)
	{
	}
}

// 0043a110
void RFThrowException983(const std::wstring& message)
{
	throw Testing::RFException(message);
}

// 0043a1d0
void RFThrowInvalidArgException983(const std::wstring& message)
{
	throw Testing::RFInvalidArgException(message);
}

// 0043a2c0
void RFThrowFileException983(const std::wstring& message, int code)
{
	throw Testing::RFFileException(message, code);
}

// 0043a370
FrameImageSet::FrameImageSet()
{
}

// 0043a440
FrameImageSet::~FrameImageSet()
{
}

// 0043a600
CheckBase::CheckBase()
{
	RF_REGISTER_PROPERTY983(_pushSound, "PushSound");
	_uncheckedImage = 0;
	_checkedImage = 0;
}

// 0043a720
CheckBase::~CheckBase()
{
	RF_SAFE_RELEASE983(_uncheckedImage);
	RF_SAFE_RELEASE983(_checkedImage);
}

// 0043a9b0
WndExBase::~WndExBase()
{
	RF_SAFE_RELEASE983(_background);
}

// 0043aaf0
EditBase::EditBase()
{
	_color.Set(0xffffffff);

	RF_REGISTER_PROPERTY983(_fontName, "FontName");
	RF_REGISTER_PROPERTY983(_fontSize, "FontSize");
	RF_REGISTER_PROPERTY983(_color, "Color");
	RF_REGISTER_PROPERTY983(_shadowColor, "ShadowColor");
	RF_REGISTER_PROPERTY983(_outlineColor, "OutlineColor");
}

// 0043aef0
EditBase::~EditBase()
{
	RF_SAFE_RELEASE983(_caret);
}

// 0043b140
LabelBase::LabelBase()
{
	_color.Set(0xffffffff);
	_textTrimming.Set(0);

	RF_REGISTER_PROPERTY983(_padding, "Padding");
	RF_REGISTER_PROPERTY983(_fontName, "FontName");
	RF_REGISTER_PROPERTY983(_fontSize, "FontSize");
	RF_REGISTER_PROPERTY983(_color, "Color");
	RF_REGISTER_PROPERTY983(_shadowColor, "ShadowColor");
	RF_REGISTER_PROPERTY983(_outlineColor, "OutlineColor");
	RF_REGISTER_PROPERTY983(_hAlign, "HAlign");
	RF_REGISTER_PROPERTY983(_vAlign, "VAlign");
	RF_REGISTER_PROPERTY983(_textTrimming, "TextTrimming");
	RF_REGISTER_PROPERTY983(_fitToContent, "FitToContent");
}

// 0043b930
LabelBase::~LabelBase()
{
}

// 0043bc30
ButtonBase::ButtonBase()
{
	RF_REGISTER_PROPERTY983(_isDefaultButton, "IsDefaultButton");
	_normalImage = 0;
	_pushImage = 0;
	_hoverImage = 0;
	_disableImage = 0;
}

// 0043bd90
ButtonBase::~ButtonBase()
{
	RF_SAFE_RELEASE983(_normalImage);
	RF_SAFE_RELEASE983(_pushImage);
	RF_SAFE_RELEASE983(_hoverImage);
	RF_SAFE_RELEASE983(_disableImage);
}

// 0043bfc0
LabelButtonBase::LabelButtonBase()
{
	_hasNormalColor = true;
	_normalColor.Set(0xffffffff);
	_hoverColor.Set(0xffffffff);
	_pushColor.Set(0xffffffff);
	_disableColor.Set(0xffffffff);

	RF_REGISTER_PROPERTY983(_normalColor, "NormalColor");
	RF_REGISTER_PROPERTY983(_hoverColor, "HoverColor");
	RF_REGISTER_PROPERTY983(_pushColor, "PushColor");
	RF_REGISTER_PROPERTY983(_disableColor, "DisableColor");
}

// 0043c320
LabelButtonBase::~LabelButtonBase()
{
}

// 0043c420
AnimButtonBase::AnimButtonBase()
{
	_enterImage = 0;
	_leaveImage = 0;
}

// 0043c440
AnimButtonBase::~AnimButtonBase()
{
	RF_SAFE_RELEASE983(_enterImage);
	RF_SAFE_RELEASE983(_leaveImage);
}

// 0043c570
StackPanelBase::StackPanelBase()
{
	RF_REGISTER_PROPERTY983(_hAlign, "HAlign");
	RF_REGISTER_PROPERTY983(_vAlign, "VAlign");
	RF_REGISTER_PROPERTY983(_orientation, "Orientation");
}

// 0043c810
StackPanelBase::~StackPanelBase()
{
}

// 0043c8d0
TextAreaBase::TextAreaBase()
{
	_editable.Set(false);

	RF_REGISTER_PROPERTY983(_fontName, "FontName");
	RF_REGISTER_PROPERTY983(_fontSize, "FontSize");
	RF_REGISTER_PROPERTY983(_color, "Color");
	RF_REGISTER_PROPERTY983(_shadowColor, "ShadowColor");
	RF_REGISTER_PROPERTY983(_outlineColor, "OutlineColor");
	RF_REGISTER_PROPERTY983(_editable, "Editable");
	RF_REGISTER_PROPERTY983(_text, "Text");
	RF_REGISTER_PROPERTY983(_fitToContent, "FitToContent");
}

// 0043cec0
TextAreaBase::~TextAreaBase()
{
	RF_SAFE_RELEASE983(_caret);
}

// 0043d1e0
TextAreaSCBase::TextAreaSCBase()
{
	_hScrollbar = 0;
	_vScrollbar = 0;
}

// 0043d270
ListBase::ListBase()
{
	RF_REGISTER_PROPERTY983(_orientation, "Orientation");
}

// 0043d3e0
ListBase::~ListBase()
{
}

// 0043d4a0
ListSCBase::ListSCBase()
{
	_hScrollbar = 0;
	_vScrollbar = 0;
}

// 0043d510
ComboBase::ComboBase()
{
	RF_REGISTER_PROPERTY983(_normalColor, "NormalColor");
	RF_REGISTER_PROPERTY983(_selectColor, "SelectColor");
	RF_REGISTER_PROPERTY983(_fontName, "FontName");
	RF_REGISTER_PROPERTY983(_fontSize, "FontSize");
	_normalColor.Set(0xff0000ff);
	_selectColor.Set(0xffff0000);
}

// 0043d880
ComboBase::~ComboBase()
{
}

// 0043d9b0
PictureBase::~PictureBase()
{
	RF_SAFE_RELEASE983(_image);
}

// 0043daf0
ProgressBase::~ProgressBase()
{
	RF_SAFE_RELEASE983(_image);
}

// 00440290
ScrollbarBase::ScrollbarBase()
{
}

// 004402c0
SliderBase::SliderBase()
{
}

// 00440330
CustomBase::CustomBase()
{
}

// 004403c0 (dentro do ??_G)
CustomBase::~CustomBase()
{
}
