// JP 983: RFLabel.cpp da Refresh: o rótulo inteiro (0041f0c0–0042004f) (o RFLabelButton::SetLabel 00426120 foi para o rflabelbutton.cpp na B160). B153: texto e
// ajuste ao conteúdo (o SetLabel(wstring) era o stub mais perto do WinMain); B156: construtor, Create, PreInitialize, margens e
// o OnDraw (texto pelo RFInterface do RFManager, com "..." no corte e alinhamento por _hAlign/_vAlign).
#include <string>
#include <sstream>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"
#include "vc10_layout.h"

extern RFInterface* g_pRFInterface983; // 00f1de14 (rfmanager.cpp)
extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

#if _MSC_VER > 1600
// D877: cor do texto para a visão do jogo (rfbase.h); no VC10 a visão escreve +0x1c8 direto
void RFLabel::SetColor983(unsigned int color)
{
	_fontInfo.color = color;
}
#endif

// A822: espelho RFTextStyle983 (refresh/rfinterface.h) do RefreshInterface::DrawText983: size, style, color, outline, shadow
C_ASSERT(offsetof(FontInfo, size) == sizeof(std::wstring) && offsetof(FontInfo, style) == sizeof(std::wstring) + 4 && offsetof(FontInfo, shadowColor) == sizeof(std::wstring) + 0x10);

// 0041f0c0 (_hAlign/_vAlign/_textTrimming/_autoFit sem valor: vêm do Create)
RFLabel::RFLabel()
{
	C_ASSERT_VC10(offsetof(RFLabel, _fontInfo) + offsetof(FontInfo, color) == 0x1c8); // D877: visão rfbase.h SetColor983
}

// 0041f140
RFLabel::~RFLabel()
{
}

// 0041f1c0: estilo da <Label>; o texto passa pelo TranslateText do jogo
void RFLabel::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	LabelBase* label = static_cast<LabelBase*>(base);
	_padding = label->_padding.As983<RFRect>();
	_fontInfo.name.assign(label->_fontName.GetRef983(), 0, std::wstring::npos);
	_fontInfo.size = label->_fontSize.Get();
	_fontInfo.color = label->_color.Get();
	_fontInfo.shadowColor = label->_shadowColor.Get();
	_fontInfo.outlineColor = label->_outlineColor.Get();
	_fontInfo.style = label->_style;
	_hAlign = (HorzAlignTypes::Type)label->_hAlign.Get();
	_vAlign = (VertAlignTypes::Type)label->_vAlign.Get();
	_textTrimming = (TextTrimmingTypes::Type)label->_textTrimming.Get();
	_autoFit = label->_fitToContent.Get();
	_label.assign(g_pRFInterface983->TranslateText(label->_text), 0, std::wstring::npos);
	if (_autoFit)
		FitToContent();
}

// 0041f340: tamanho = medida do texto (RFInterface slot 2) + margens; se mudou, avisa o layout (2 = tamanho)
void RFLabel::FitToContent()
{
	if (_label.size() != 0)
	{
		RFSize size = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(_label.c_str()));
		size.width = _padding.right + _padding.left + size.width;
		size.height = _padding.bottom + _padding.top + size.height;
		if (size.width != _localSize.width || size.height != _localSize.height)
		{
			_localSize = size;
			OnLayoutChanged(2);
		}
	}
}

// 0041f490
void RFLabel::AutoFit(bool autoFit)
{
	_autoFit = autoFit;
	if (autoFit)
		FitToContent();
}

// 0041f510: copia os dígitos pondo ',' a cada 3 a partir do fim ("1234567" -> "1,234,567")
void RFLabel::putComma_(const wchar_t* src, wchar_t* dst)
{
	int n = (int)wcslen(src);
	wchar_t c = *src;
	*dst = c;
	while (c != L'\0')
	{
		++dst;
		++src;
		if (--n != 0 && n == n / 3 * 3)
			*dst++ = L',';
		c = *src;
		*dst = c;
	}
}

// 0041f4b0 (o RFWnd::PreInitialize embutido): o rótulo não pega o mouse
void RFLabel::PreInitialize()
{
	RFWnd::PreInitialize();
	_hitTestVisible = false;
}

// 0041f580: número com separador de milhar (o 983 põe as vírgulas sempre; o bool não é lido)
void RFLabel::SetLabel(int value, bool)
{
	std::wostringstream stream;
	stream << value;
	std::wstring digits = stream.str();
	wchar_t text[0x400] = { 0 };
	putComma_(digits.c_str(), text);
	std::wstring label(text); // (SetLabel(wstring) embutida)
	_label.assign(label, 0, std::wstring::npos);
	if (_autoFit)
		FitToContent();
}

// 0041f830
void RFLabel::SetLabel(const std::wstring& label)
{
	_label.assign(label, 0, std::wstring::npos);
	if (_autoFit)
		FitToContent();
}

// 0041f860
void RFLabel::SetPadding(const RFRect& padding)
{
	_padding = padding;
}

// 0041f890
const RFRect& RFLabel::GetPadding() const
{
	return _padding;
}

// 0041f8a0: o texto no retângulo da janela menos as margens; com CharacterEllipsis e texto maior que a largura, corta letra a
// letra (larguras arredondadas) e põe "..."; a cor sai com o alfa da janela multiplicado no alfa da cor
void RFLabel::OnDraw()
{
	if (_label.size() == 0)
		return;
	RFRect rect = ClientToScreen();
	rect.left += _padding.left;
	rect.top += _padding.top;
	rect.right -= _padding.right;
	rect.bottom -= _padding.bottom;
	std::wstring trimmed;
	RFSize extent = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(_label.c_str()));
	const wchar_t* text;
	float width = rect.Width();
	if (_textTrimming == TextTrimmingTypes::CharacterEllipsis && width < extent.width)
	{
		RFSize dots = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(L"..."));
		int total = (int)(dots.width + 0.5);
		for (unsigned int i = 0; i < _label.size(); ++i)
		{
			wchar_t ch[2] = { 0 };
			ch[0] = _label.at(i);
			RFSize size = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(ch));
			if (width < (float)((int)(size.width + 0.5) + total))
				break;
			trimmed.append(ch);
			total += (int)(size.width + 0.5);
		}
		trimmed.append(L"...");
		text = trimmed.c_str();
	}
	else
		text = _label.c_str();
	int areaWidth = (int)rect.Width();
	int areaHeight = (int)rect.Height();
	RFSize size = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(text));
	int x, y;
	switch (_hAlign)
	{
	case HorzAlignTypes::Left:
		x = (int)rect.left;
		break;
	case HorzAlignTypes::Center:
		x = (areaWidth - (int)size.width) / 2 + (int)rect.left;
		break;
	case HorzAlignTypes::Right:
		x = (int)(rect.right - (int)size.width);
		break;
	}
	switch (_vAlign)
	{
	case VertAlignTypes::Top:
		y = (int)rect.top;
		break;
	case VertAlignTypes::Center:
		y = (areaHeight - (int)size.height) / 2 + (int)rect.top;
		break;
	case VertAlignTypes::Bottom:
		y = (int)(rect.bottom - (int)size.height);
		break;
	}
	FontInfo font = _fontInfo;
	float alpha = GetAlpha(false);
	unsigned int color = _fontInfo.color;
	unsigned int a = (unsigned int)((double)(color >> 24) / 255.0 * alpha * 255.0);
	font.color = (((a << 8 | ((color >> 16) & 0xff)) << 8 | ((color >> 8) & 0xff)) << 8) | (color & 0xff);
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, rect);
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	RFManager::GetInstance()._interface->PrintText983(font, x, y, std::wstring(text), _rc._depth);
	RFRect clip2;
	clip2.IntersectRect(_rc._clipRect, rect);
	_rc._clipRect = clip2;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}
