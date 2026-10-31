// JP 983: RFEdit.cpp da Refresh (B214, F3.7.14): a caixa de edicao de uma linha (00435000-004371c0), sem fonte no 645/839:
// escrita pelo asm do 983. Campos: texto em +0x180, foco +0x19c, limite +0x1a0, caret +0x1ac, selecao do IME +0x1b0/b4/b8,
// selecao com Shift +0x1c0/c4 (so com copiar/colar ligado), cursor piscante +0x1bc (RFCaret983).
// ToString/GetTypeName/acessores em linha no rfcontrols.h; strings no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um metodo com esse nome)
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfinterface.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rftext.h"
#include "refresh/lib/rfime.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/rfimageset.h"
#include "refresh/rfreferencecount.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

extern RFInterface* g_pRFInterface983; // 00f1de14 (rfmanager.cpp)
extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// validador do texto digitado (nao exportado; so o slot 1 e usado pelo RFEdit: devolve false para recusar)
class RFValidator
{
public:
	virtual ~RFValidator() { }
	virtual bool Validate983(const std::wstring& text) = 0; // slot 1
};

// 00464e50 (so a copia de CF_UNICODETEXT; o 983 devolve false sem texto na area de transferencia)
static bool GetClipboardText983(std::wstring& out)
{
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		return false;
	if (!OpenClipboard(0))
		return false;
	HANDLE data = GetClipboardData(CF_UNICODETEXT);
	const wchar_t* text = (const wchar_t*)GlobalLock(data);
	out.assign(text, wcslen(text));
	GlobalUnlock(data);
	CloseClipboard();
	return true;
}

// 00464ec0
static bool SetClipboardText983(const std::wstring& text)
{
	if (!OpenClipboard(0))
		return false;
	HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * 2);
	wchar_t* dst = (wchar_t*)GlobalLock(mem);
	memcpy(dst, text.c_str(), (text.size() + 1) * 2);
	GlobalUnlock(mem);
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, mem);
	CloseClipboard();
	return true;
}

static bool IsKeyDown983(int vk)
{
	return (GetAsyncKeyState(vk) & 0x8000) != 0; // 983 004358a0, 00436af0, 00436dc0: o 983 nao importa GetKeyState
}

// 00435000 (o RFWndEx::RFWndEx embutido)
RFEdit::RFEdit() : _scrollX(0), _scrollY(0), _selecting(false), _selStart(0), _selEnd(0), _shiftBegin(0), _shiftEnd(0)
{
	_padding.left = 0.0f;
	_padding.right = 0.0f;
	_padding.top = 0.0f;
	_padding.bottom = 0.0f;
}

// 004350c0
RFEdit::~RFEdit()
{
	CleanupPasswordText();
}

// 00435150 (o RFWndEx::Create embutido)
void RFEdit::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWndEx::Create(base, parent);
	EditBase* edit = static_cast<EditBase*>(base);
	_fontInfo.name.assign(edit->_fontName.GetRef983(), 0, std::wstring::npos);
	_fontInfo.size = edit->_fontSize.Get();
	_fontInfo.color = edit->_color.Get();
	_padding.left = edit->_padding.left;
	_padding.right = edit->_padding.right;
	_padding.top = edit->_padding.top;
	_padding.bottom = edit->_padding.bottom;
	_fontInfo.outlineColor = edit->_outlineColor.Get();
	_fontInfo.shadowColor = edit->_shadowColor.Get();
	_fontInfo.style = edit->_style;
	_hAlign983 = (HorzAlignTypes::Type)edit->_halign;
	_vAlign983 = (VertAlignTypes::Type)edit->_valign;
	_passwordChar = edit->_passwordChar;
	_caretImageSet = (ImageSet*)edit->_caret;
}

// 00435230 (o RFWndEx::PreInitialize embutido)
void RFEdit::PreInitialize()
{
	RFWndEx::PreInitialize();
	_caretLocate = 0;
	_1c8 = false;
	_validator = 0;
	_caret983 = 0;
	_focused = false;
	_copyAndPaste = true;
	_limitText = 0x28;
	_selColor = 0xfaadd6ff;
	_selBgColor = 0xfaa0a0a0;
	_rc._clip = true;
}

// 004352d0 (o resto e o RFWndEx::OnDestroy embutido: apaga o pincel)
void RFEdit::OnDestroy()
{
	delete _caret983;
	_caret983 = 0;
	RFWndEx::OnDestroy();
}

// 00435310: o cursor so existe se o <Edit> tem a imagem dele; tamanho = o da imagem
void RFEdit::LoadResource()
{
	_brush = RFBrush::Create(_imageSet);
	if (_caretImageSet)
	{
		RFCaret983* caret = new RFCaret983;
		_caret983 = caret;
		RFBrush* brush = RFBrush::Create(_caretImageSet);
		caret->_painter = (RFCaretPainter983*)brush;
		RFSize size = brush->GetImageSize();
		memcpy(&caret->_w, &size, sizeof(size)); // (o 983 copia os 2 floats crus para _w/_h)
	}
}

// 00435390
void RFEdit::SetText(wchar_t const* text)
{
	_text.assign(text, wcslen(text));
	_caretLocate = (int)_text.size();
}

// 004353e0
wchar_t const* RFEdit::GetText() const
{
	return _text.c_str();
}

// 00435400: encerra a composicao do IME, zera selecao e texto
void RFEdit::ClearText()
{
	RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
	if (ime)
		ime->FinishComposition983(false);
	_selEnd = _caretLocate;
	_selStart = _caretLocate;
	_selecting = false;
	CleanupPasswordText();
	_text.erase(0, std::wstring::npos);
	_caretLocate = (int)_text.size();
	_shiftBegin = 0;
	_shiftEnd = 0;
}

// 004354a0
void RFEdit::OnEnabled(bool enabled)
{
	if (!_focused)
		return;
	if (enabled && _caret983)
		_caret983->ResetBlink983();
	RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
	if (ime)
		ime->SetEnable983(enabled);
	g_pRFInterface983->OnKeyboardAcquired(enabled);
}

// 00435510
void RFEdit::OnSetFocus()
{
	RFWnd::OnSetFocus();
	_focused = true;
	if (!_enabled)
		return;
	if (_caret983)
		_caret983->ResetBlink983();
	RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
	if (ime)
		ime->SetEnable983(true);
	g_pRFInterface983->OnKeyboardAcquired(true);
	if (_passwordChar != 0)
	{
		ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->AssociateContext983(false);
	}
}

// 004355d0: perde o foco com trecho de composicao selecionado: o caret vai para depois dele
void RFEdit::OnKillFocus()
{
	RFWnd::OnKillFocus();
	_focused = false;
	if (!_enabled)
		return;
	if (_selecting)
	{
		_caretLocate += _selEnd - _selStart;
		if (_caretLocate < 0)
			_caretLocate = 0;
		else if (_caretLocate > (int)_text.size())
			_caretLocate = (int)_text.size();
		_selEnd = _caretLocate;
		_selStart = _caretLocate;
		_selecting = false;
	}
	g_pRFInterface983->OnKeyboardAcquired(false);
	RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
	if (ime)
		ime->SetEnable983(false);
}

// 00435690: o caret vai para a borda do caractere mais perto do clique
void RFEdit::OnLButtonDown(MouseEventArgs& e)
{
	RFWnd::OnLButtonDown(e);
	e.handled = true;
	if (!_enabled || _selecting)
		return;
	if (_caret983)
		_caret983->ResetBlink983();
	RFRect rect = ClientToScreen();
	float left = rect.left + _padding.left;
	int mouseX = (int)e.pos.x;
	int acc = 0;
	int prev = 0;
	if ((int)_text.size() <= 0)
		return;
	int base = (int)left;
	for (int i = 0; i < (int)_text.size(); ++i)
	{
		wchar_t ch[2] = { 0 };
		ch[0] = _text.c_str()[i];
		RFSize size = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(ch));
		acc += (int)size.width;
		int edgeL = prev + (base + _scrollX);
		int edgeR = (base + _scrollX) + acc;
		if (edgeL > mouseX || mouseX > edgeR)
		{
			prev = acc;
			continue;
		}
		int next = i;
		if (mouseX - edgeL >= edgeR - mouseX)
			next = i + 1;
		if (next < 0)
			_caretLocate = 0;
		else if (next > (int)_text.size())
			_caretLocate = (int)_text.size();
		else
			_caretLocate = next;
		return;
	}
}

// 004358a0: a tecla vai primeiro aos assinantes (EventKeyDown); depois edicao/navegacao/copia
void RFEdit::OnKeyDown(KeyEventArgs& e)
{
	RFWnd::OnKeyDown(e);
	if (!_enabled || e.key == VK_ESCAPE)
		return;
	if (_copyAndPaste && IsKeyDown983(VK_SHIFT) && _shiftEnd - _shiftBegin == 0)
		_shiftBegin = _caretLocate;
	switch (e.key)
	{
	case VK_BACK:
		KeyDownDeleteChar(true);
		break;
	case VK_RETURN:
		ResetSelection();
		KeyDownReturn(e);
		break;
	case VK_END:
		ResetSelection();
		KeyDownEnd();
		break;
	case VK_HOME:
		ResetSelection();
		KeyDownHome();
		break;
	case VK_LEFT:
		ResetSelection();
		KeyDownArrow(-1);
		break;
	case VK_UP:
	case VK_DOWN:
		ResetSelection();
		break;
	case VK_RIGHT:
		ResetSelection();
		KeyDownArrow(1);
		break;
	case VK_DELETE:
		KeyDownDeleteChar(false);
		break;
	case 0x43: // 'C'
	case 0x56: // 'V'
	case 0x63: // 'c'
	case 0x76: // 'v'
		KeyDownCopyAndPaste(e.key);
		break;
	}
	if (_copyAndPaste && IsKeyDown983(VK_SHIFT))
		_shiftEnd = _caretLocate;
	if (_caret983)
		_caret983->ResetBlink983();
	e.handled = true;
}

// 00435ab0: o validador pode recusar; apaga o trecho de composicao e a selecao, insere e anda o caret (composicao do
// IME deixa o texto novo selecionado em vez de andar)
void RFEdit::OnChar(CharEventArgs& e)
{
	if (!_enabled)
		return;
	std::wstring input(e.text);
	if (_validator && !_validator->Validate983(input))
		return;
	if ((int)_text.size() > 0 && _selecting)
		DeleteCharAtCaretPos();
	int range = _shiftEnd - _shiftBegin;
	if (range != 0)
	{
		if (range > 0)
			AddCaretLocate(_shiftBegin - _shiftEnd);
		DeleteSelectionChar();
	}
	std::wstring result = InsertStringAtCaretPos(input);
	if (result.size() != 0)
	{
		_text.assign(result, 0, std::wstring::npos);
		if (e.composing983)
		{
			int end = _caretLocate + (int)input.size();
			_selStart = _caretLocate;
			_selEnd = end;
			_selecting = _caretLocate != end;
		}
		else
			AddCaretLocate((int)input.size());
	}
	else if (RFManager::GetInstance()._usingCustomInputMethod)
	{
		RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->FinishComposition983(false);
		_selecting = false;
		_selStart = _caretLocate;
		_selEnd = _caretLocate;
	}
	e.handled = true;
}

// 00435cd0: fundo (RFWndEx), selecao, texto (ou o caractere de senha repetido) e cursor, tudo recortado pela area menos margens
void RFEdit::OnDraw()
{
	RFWndEx::OnDraw();
	RFRect rect = ClientToScreen();
	rect.left += _padding.left;
	rect.top += _padding.top;
	rect.right -= _padding.right;
	rect.bottom -= _padding.bottom;
	std::wstring text;
	text.assign(_text, 0, std::wstring::npos);
	if (_passwordChar != 0)
	{
		int count = (int)text.size();
		std::wstring masked(count, _passwordChar);
		text.assign(masked.c_str(), masked.size());
	}
	int x = GetTextHAlignPos(rect, text);
	int y = GetTextVAlignPos(rect, text);
	int caretX = GetCaretPos(text, true);
	FontInfo font = _fontInfo;
	float alpha = GetAlpha(false);
	unsigned int color = _fontInfo.color;
	unsigned int a = (unsigned int)((double)(color >> 24) / 255.0 * alpha * 255.0);
	font.color = (((a << 8 | ((color >> 16) & 0xff)) << 8 | ((color >> 8) & 0xff)) << 8) | (color & 0xff);
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, rect);
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	RFVector2 selPos((float)x, (float)y);
	DrawSelection(selPos, _rc._depth);
	RFManager::GetInstance()._interface->PrintText983(font, x, y, std::wstring(text.c_str()), _rc._depth);
	if (_focused)
	{
		RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->SetCompositionPos983(caretX + x, y);
	}
	RFVector2 caretPos((float)(caretX + x), (float)y);
	DrawCaret(caretPos, _rc._depth);
	RFRect clip2;
	clip2.IntersectRect(_rc._clipRect, rect);
	_rc._clipRect = clip2;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 004361c0: so anda o relogio do piscar (nao chama o RFWnd::OnUpdate)
void RFEdit::OnUpdate(UpdateEventArgs& e)
{
	RFCaret983* caret = _caret983;
	if (!caret || !caret->_visible)
		return;
	caret->_timer = e.delta + caret->_timer;
	if (caret->_period < caret->_timer)
	{
		caret->_timer = 0.0f;
		caret->_blinkOn = !caret->_blinkOn;
	}
}

// 00436210: devolve o texto com a entrada no caret, ou vazio se passar do limite (so a entrada se nao ha texto)
std::wstring RFEdit::InsertStringAtCaretPos(const std::wstring& input)
{
	int length = (int)_text.size();
	if (length <= 0)
		return std::wstring(input);
	if ((int)input.size() + length > _limitText)
		return std::wstring(L"");
	if (_caretLocate <= 0)
	{
		_caretLocate = 0;
		return input + _text;
	}
	if (_caretLocate >= length)
	{
		_caretLocate = length;
		return _text + input;
	}
	std::wstring result(_text);
	result.insert(_caretLocate, input);
	return result;
}

// 004363b0: apaga o trecho de composicao (ou 1 caractere) no caret
void RFEdit::DeleteCharAtCaretPos()
{
	int count = 1;
	if (_selecting)
		count = _selEnd - _selStart;
	DeleteChar(_caretLocate, count);
	_selEnd = _caretLocate;
	_selStart = _caretLocate;
	_selecting = false;
}

// 00436400: apaga a selecao com Shift e leva o caret ao fim dela
void RFEdit::DeleteSelectionChar()
{
	if (_shiftEnd - _shiftBegin == 0)
		return;
	int lo = _shiftEnd < _shiftBegin ? _shiftEnd : _shiftBegin;
	int hi = _shiftBegin < _shiftEnd ? _shiftEnd : _shiftBegin;
	DeleteChar(lo, hi - lo);
	_shiftBegin = _caretLocate;
	_shiftEnd = _caretLocate;
}

// 00436460
void RFEdit::DeleteChar(int pos, int count)
{
	if (count <= 0 || pos >= (int)_text.size())
		return;
	for (int i = 0; i < count; ++i)
	{
		if (pos == (int)_text.size())
			break;
		_text.erase(pos, 1);
	}
}

// 004364e0: sobrescreve o texto de senha com zeros (o tamanho fica)
void RFEdit::CleanupPasswordText()
{
	if (_passwordChar == 0)
		return;
	for (std::wstring::iterator itor = _text.begin(); itor != _text.end(); ++itor)
		*itor = 0;
}

// 00436530: x do texto conforme o alinhamento; se o caret sai da area, desloca o texto (1/4 da largura de folga)
int RFEdit::GetTextHAlignPos(const RFRect& rect, const std::wstring& text)
{
	int areaWidth = (int)(rect.right - rect.left);
	int caretWidth = GetCaretPos(text, true);
	RFSize extent = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(text.c_str()));
	_scrollX += (int)rect.left;
	float caretEdge = (float)(_scrollX + caretWidth);
	if (!(rect.left > caretEdge) && !(rect.right < caretEdge))
	{
		if (_hAlign983 != 0)
		{
			if (!((float)areaWidth <= extent.width) && _hAlign983 == HorzAlignTypes::Center)
				_scrollX = areaWidth / 2 - (int)extent.width / 2 + (int)rect.left;
			else
			{
				if (_scrollX > (int)rect.left)
					_scrollX = (int)rect.right - (int)extent.width - _scrollX;
				if ((float)(int)rect.right > (float)_scrollX + extent.width)
					_scrollX = (int)rect.right - (int)extent.width;
			}
		}
	}
	else
	{
		int slack = 0;
		if (!(rect.right >= caretEdge))
		{
			int limit = _selecting ? _selStart - _selEnd + (int)_text.size() : (int)_text.size();
			if (_caretLocate < limit)
				slack = areaWidth >> 2;
			_scrollX = (int)rect.right - slack - caretWidth;
		}
		else if (!(rect.left <= caretEdge))
		{
			if (_caretLocate > 0)
				slack = areaWidth >> 2;
			_scrollX = (int)rect.left - caretWidth + slack;
			if (_scrollX > (int)rect.left)
				_scrollX = (int)rect.left;
		}
	}
	int result = _scrollX;
	_scrollX = result - (int)rect.left;
	return result;
}

// 004367c0
int RFEdit::GetTextVAlignPos(const RFRect& rect, const std::wstring& text)
{
	int areaHeight = (int)(rect.bottom - rect.top);
	RFSize extent = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(text.c_str()));
	_scrollY += (int)rect.top;
	switch (_vAlign983)
	{
	case VertAlignTypes::Top:
		_scrollY = (int)rect.top;
		break;
	case VertAlignTypes::Center:
		_scrollY = (areaHeight - (int)extent.height) / 2 + (int)rect.top;
		break;
	case VertAlignTypes::Bottom:
		_scrollY = (int)rect.bottom - (int)extent.height;
		break;
	}
	int result = _scrollY;
	_scrollY = result - (int)rect.top;
	return result;
}

// 00436930: largura do texto ate o caret (com a composicao do IME se includeSelection)
int RFEdit::GetCaretPos(const std::wstring& text, bool includeSelection)
{
	int count = _caretLocate;
	if (includeSelection && _selecting)
		count += _selEnd - _selStart;
	std::wstring head = text.substr(0, count);
	RFSize extent = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(head.c_str()));
	return (int)extent.width;
}

// 00436a50
void RFEdit::EnableCopyAndPaste(bool enable)
{
	_copyAndPaste = enable;
	_shiftBegin = 0;
	_shiftEnd = 0;
}

// 00436a70
void RFEdit::DrawCaret(const RFVector2& pos, float depth)
{
	if (!_focused || !_enabled)
		return;
	RFCaret983* caret = _caret983;
	if (!caret)
		return;
	float alpha = GetAlpha(false);
	if (!caret->_visible || !caret->_blinkOn)
		return;
	float p[2] = { pos.x, pos.y };
	caret->_painter->DrawCaret983(p, &caret->_w, alpha, depth);
}

// 00436ad0
void RFEdit::ResetCaretBlink()
{
	if (_caret983)
		_caret983->ResetBlink983();
}

// 00436af0: sem Shift apertado a selecao some
void RFEdit::ResetSelection()
{
	if (!IsKeyDown983(VK_SHIFT))
	{
		_shiftBegin = 0;
		_shiftEnd = 0;
	}
}

// 00436b20: retangulo da selecao com Shift; cor muda com o foco
void RFEdit::DrawSelection(const RFVector2& pos, float depth)
{
	if (_text.size() == 0 || !_copyAndPaste)
		return;
	if (_shiftEnd - _shiftBegin == 0)
		return;
	int lo = _shiftEnd < _shiftBegin ? _shiftEnd : _shiftBegin;
	int hi = _shiftBegin < _shiftEnd ? _shiftEnd : _shiftBegin;
	if (lo < 0 || lo >= (int)_text.size() || hi > (int)_text.size())
		return;
	std::wstring head = _text.substr(0, lo);
	std::wstring selected;
	if (hi >= (int)_text.size())
		selected = _text.substr(lo);
	else
		selected = _text.substr(lo, hi - lo);
	RFSize headSize = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(head.c_str()));
	RFSize selSize = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(selected.c_str()));
	unsigned int color = _focused ? _selColor : _selBgColor;
	g_pRFRenderer983->FillRectangle(color, pos.x + headSize.width, pos.y, selSize.width, selSize.height, depth);
}

// 00436d40
void RFEdit::SetCaretLocate(int locate)
{
	if (locate < 0)
		_caretLocate = 0;
	else if (locate > (int)_text.size())
		_caretLocate = (int)_text.size();
	else
		_caretLocate = locate;
}

// 00436d80
void RFEdit::AddCaretLocate(int delta)
{
	_caretLocate += delta;
	if (_caretLocate < 0)
		_caretLocate = 0;
	else if (_caretLocate > (int)_text.size())
		_caretLocate = (int)_text.size();
}

// 00436dc0: Ctrl+C copia a selecao (nao em campo de senha); Ctrl+V cola ate o limite, pela mesma via do OnChar
bool RFEdit::KeyDownCopyAndPaste(unsigned int key)
{
	if (!_copyAndPaste)
		return false;
	if (key == 0x76 || key == 0x56)
	{
		if (!IsKeyDown983(VK_CONTROL))
			return false;
		if (_passwordChar == 0)
		{
			std::wstring clip;
			GetClipboardText983(clip);
			clip = clip.substr(0, _limitText);
			CharEventArgs args(this, clip.c_str(), false, RFPoint());
			OnChar(args);
		}
		return true;
	}
	if (key == 0x63 || key == 0x43)
	{
		if (!IsKeyDown983(VK_CONTROL))
			return false;
		if (_passwordChar == 0 && _shiftEnd - _shiftBegin != 0)
		{
			int lo = _shiftEnd < _shiftBegin ? _shiftEnd : _shiftBegin;
			int hi = _shiftBegin < _shiftEnd ? _shiftEnd : _shiftBegin;
			std::wstring copied;
			if (hi >= (int)_text.size())
				copied = _text.substr(lo);
			else
				copied = _text.substr(lo, hi - lo);
			SetClipboardText983(copied);
		}
		return true;
	}
	return false;
}

// 00436ff0
void RFEdit::KeyDownSelectStart()
{
	if (_copyAndPaste && IsKeyDown983(VK_SHIFT) && _shiftEnd - _shiftBegin == 0)
		_shiftBegin = _caretLocate;
}

// 00437030
void RFEdit::KeyDownSelectEnd()
{
	if (_copyAndPaste && IsKeyDown983(VK_SHIFT))
		_shiftEnd = _caretLocate;
}

// 00437060
void RFEdit::KeyDownHome()
{
	int length = (int)_text.size();
	_caretLocate = length >= 0 ? 0 : length;
}

// 00437080
void RFEdit::KeyDownEnd()
{
	int length = (int)_text.size();
	_caretLocate = length < 0 ? 0 : length;
}

// 004370a0: Backspace (back) / Delete: apaga a composicao, a selecao com Shift ou o caractere ao lado do caret
void RFEdit::KeyDownDeleteChar(bool back)
{
	if (_selecting)
		return;
	int range = _shiftEnd - _shiftBegin;
	if (range == 0)
	{
		if (back)
		{
			if (_caretLocate > 0)
			{
				AddCaretLocate(-1);
				DeleteCharAtCaretPos();
			}
		}
		else if ((int)_text.size() > _caretLocate)
			DeleteCharAtCaretPos();
		return;
	}
	if (range > 0)
		AddCaretLocate(_shiftBegin - _shiftEnd);
	DeleteSelectionChar();
}

// 00437140
void RFEdit::KeyDownArrow(int delta)
{
	_caretLocate += delta;
	if (_caretLocate < 0)
		_caretLocate = 0;
	else if (_caretLocate > (int)_text.size())
		_caretLocate = (int)_text.size();
}

// 00437180: Enter avisa os assinantes do EnterKeyDown
void RFEdit::KeyDownReturn(KeyEventArgs& e)
{
	SendEvent(EventEnterKeyDown, e);
}
