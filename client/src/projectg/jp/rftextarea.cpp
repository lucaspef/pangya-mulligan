// JP 983: RFTextArea.cpp da Refresh (B175, F3.7.13): a area de texto (00426580-00428300), 1o lote: ctor/dtor, Create,
// PreInitialize/OnDestroy/LoadResource, layout, Enabled, SetText/AddText/GetText/ClearText e os acessores do RFText (+0x180).
// B266 (2o lote): mouse (OnLButtonDown/RButton/MouseMove/MouseLeave, HitTestHyperLink), OnDraw, Invaildate, FitToContent,
// SendDrawImageEvent (00426e00-00428220; asm em out/F3.7/s266/ta.asm).
#include <windows.h>
#undef CreateDesktop
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

// 00426580 (o resto vem dos construtores dos membros)
RFTextArea::RFTextArea()
{
	_text983 = 0;
	_caret983 = 0;
	_1a0 = false;
	_textLimit983 = 0;
	_lineLimit983 = 0;
	_1e4 = 0;
	_1e8 = 0.0f;
	_1ec = 0.0f;
	_200 = false;
	_220 = 0;
	_hAlign983 = (HorzAlignTypes::Type)0;
	_268 = 0;
	_26c = false;
	_26d = false;
	_270 = 0;
}

// 00426690
RFTextArea::~RFTextArea()
{
}

// 004267b0: estilo da <TextArea>; o texto inicial troca L"\\n" por quebra de linha; _220 = 1,30 x altura de um "A"
void RFTextArea::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	TextAreaBase* ta = static_cast<TextAreaBase*>(base);
	_fontInfo.name.assign(ta->_fontName.GetRef983(), 0, std::wstring::npos);
	_fontInfo.size = ta->_fontSize.Get();
	_fontInfo.color = ta->_color.Get();
	_fontInfo.shadowColor = ta->_shadowColor.Get();
	_fontInfo.outlineColor = ta->_outlineColor.Get();
	_fontInfo.style = ta->_style;
	_hyperLinkFont.name.assign(_fontInfo.name, 0, std::wstring::npos);
	_hyperLinkFont.size = _fontInfo.size;
	_hyperLinkFont.style = _fontInfo.style;
	_hyperLinkFont.color = _fontInfo.color;
	_hyperLinkFont.outlineColor = _fontInfo.outlineColor;
	_hyperLinkFont.shadowColor = _fontInfo.shadowColor;
	_padding.left = ta->_padding.left;
	_padding.right = ta->_padding.right;
	_padding.top = ta->_padding.top;
	_padding.bottom = ta->_padding.bottom;
	_hAlign983 = (HorzAlignTypes::Type)ta->_halign;
	_268 = 0;
	_26c = ta->_editable.Get();
	_270 = (int)ta->_caret;
	_26d = ta->_fitToContent.Get();
	std::wstring text(ta->_text.GetRef983(), 0, std::wstring::npos);
	RFRect rect = ClientToScreen();
	ApplyGlobalPadding(rect);
	_text983 = new RFText983((int)(rect.right - rect.left), (int)(rect.bottom - rect.top), (const int*)&_hAlign983,
		&_268, (int)&_fontInfo, (int)&_hyperLinkFont, (int)this);
	if (text.size() != 0)
	{
		std::wstring::size_type pos;
		while ((pos = text.find(L"\\n", 0, 2)) != std::wstring::npos)
			text.replace(pos, 2, L"\n", 1);
		SetText(text);
	}
	RFSize size = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(L"A"));
	int h = (int)(size.height + 0.5f);
	_220 = (int)(0.5 + (double)(h * 0x82) / 100.0);
}

// 00426b20 (o RFWnd::PreInitialize embutido)
void RFTextArea::PreInitialize()
{
	RFWnd::PreInitialize();
	_1a0 = false;
	_caret983 = 0;
	_200 = false;
}

// 00426b90: apaga o que o LoadResource/Create criaram (caret em +0x1b0, RFText em +0x180, pincel em +0x178)
void RFTextArea::OnDestroy()
{
	if (_caret983)
	{
		delete _caret983;
		_caret983 = 0;
	}
	if (_text983)
	{
		delete _text983;
		_text983 = 0;
	}
	if (_brush)
	{
		delete _brush;
		_brush = 0;
	}
}

// 00426bf0: pincel do fundo + cursor de texto (RFCaret 0x1c; o pincel do cursor, feito do ImageSet de +0x270, fica em
// caret+0x18 e o tamanho dele vai para caret+8/+0xc)
void RFTextArea::LoadResource()
{
	_brush = RFBrush::Create(_imageSet);
	RFCaret983* caret = new RFCaret983;
	_caret983 = caret;
	RFBrush* brush = RFBrush::Create(static_cast<ImageSet*>((RFReferenceCount*)_270));
	caret->_painter = (RFCaretPainter983*)brush;
	RFSize sz = brush->GetImageSize();
	caret->_w = (int)sz.width;
	caret->_h = (int)sz.height;
}

// 00426c90: (2 = tamanho) ancora; (tamanho ou 0x10000000) refaz a largura/altura do RFText pela area com margens
void RFTextArea::OnLayoutChanged(unsigned int flags)
{
	if (!(flags & 2) && !(flags & 0x10000000))
		return;
	if (flags & 2)
		ApplyAnchor();
	if (_text983)
	{
		RFRect rect = ClientToScreen();
		ApplyGlobalPadding(rect);
		_text983->SetWidth983((int)(rect.right - rect.left));
		_text983->_p30 = (int)(rect.bottom - rect.top);
	}
}

// 00426d90: habilitada e editavel: reinicia o piscar do caret e liga/desliga o IME
void RFTextArea::OnEnabled(bool enabled)
{
	if (!_1a0)
		return;
	if (enabled && _26c && _caret983)
		_caret983->ResetBlink983();
	RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
	if (ime)
		ime->SetEnable983(enabled);
}

// 00427bc0
void RFTextArea::SetText(const std::wstring& text)
{
	ClearText();
	AddText(text);
}

// 00427bf0
void RFTextArea::AddText(const std::wstring& text)
{
	_text983->AddText983(text.c_str(), true);
	if (_26d)
		FitToContent(false);
}

// 00427c30
void RFTextArea::SetHAlign(HorzAlignTypes::Type align)
{
	_hAlign983 = align;
	_text983->_p44 = align;
}

// 00427c50
void RFTextArea::AddPlainText(const std::wstring& text)
{
	_text983->AddText983(text.c_str(), false);
	if (_26d)
		FitToContent(false);
}

// 00427c90: monta o texto de todos os paragrafos em _text (+0x184) e devolve
const std::wstring& RFTextArea::GetText()
{
	std::wstring tmp;
	_text983->GetText983(tmp);
	_text.assign(tmp, 0, std::wstring::npos);
	return _text;
}

// 00427d10
void RFTextArea::ClearText()
{
	_textLimit983 = 0;
	_lineLimit983 = 0;
	_text983->Reset983();
}

// 00427d30
void RFTextArea::SetTextLimit(unsigned int limit)
{
	_text983->_p3c = (int)limit;
}

// 00427d50
unsigned int RFTextArea::GetTextLimit() const
{
	return (unsigned int)_text983->_p3c;
}

// 00427d60
unsigned int RFTextArea::GetLineLimit() const
{
	return (unsigned int)_text983->_p38;
}

// 00428130: margens do controle + margens globais do RFManager (se ligadas)
void RFTextArea::ApplyGlobalPadding(RFRect& r) const
{
	RFManager* m = &RFManager::GetInstance();
	if (m->_globalPaddingEnabled)
	{
		r.left = r.left + _padding.left + m->_globalPadding.left;
		r.top = r.top + _padding.top + m->_globalPadding.top;
		r.right = r.right - (_padding.right + m->_globalPadding.right);
		r.bottom = r.bottom - (_padding.bottom + m->_globalPadding.bottom);
		return;
	}
	r.left = _padding.left + r.left;
	r.top = _padding.top + r.top;
	r.right = r.right - _padding.right;
	r.bottom = r.bottom - _padding.bottom;
}

// 00427b30 (B176): anda o relogio do caret e inverte a fase do piscar quando passa do periodo
void RFTextArea::OnUpdate(UpdateEventArgs& e)
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

// 00427b80 (mesmo corpo do RFWnd::OnMouseWheel: so repassa o evento)
void RFTextArea::OnMouseWheel(MouseEventArgs& e)
{
	SendEvent(EventMouseWheel, e);
}

// 004275f0: repassa o evento; editavel e habilitada (ESC fica de fora): o caret volta a aparecer e a tecla vira comando do RFText
// (cima 0, baixo 1, esquerda 2, direita 3, home 4, end 5, delete 8, backspace 9; Enter insere "\n"); a tecla e consumida.
void RFTextArea::OnKeyDown(KeyEventArgs& e)
{
	SendEvent(EventKeyDown, e);
	if (!_enabled || !_26c || e.key == VK_ESCAPE)
		return;
	if (_caret983)
	{
		_caret983->_blinkOn = true;
		_caret983->_timer = 0.0f;
	}
	int cmd;
	switch (e.key)
	{
	case VK_BACK: cmd = 9; break;
	case VK_RETURN: _text983->AddText983(L"\n", false); e.handled = true; return;
	case VK_END: cmd = 5; break;
	case VK_HOME: cmd = 4; break;
	case VK_LEFT: cmd = 2; break;
	case VK_UP: cmd = 0; break;
	case VK_RIGHT: cmd = 3; break;
	case VK_DOWN: cmd = 1; break;
	case VK_DELETE: cmd = 8; break;
	default: e.handled = true; return;
	}
	_text983->HandleCursor983(cmd);
	e.handled = true;
}

// 00427810: com o texto editavel, troca o trecho em composicao do IME (RFText 00456ab0). O ramo do clique pendente (+0x1e4)
// posiciona o cursor por 00457550 depois do ClientToScreen/padding (so o OnLButtonDown liga o +0x1e4: clique durante a composicao).
void RFTextArea::OnChar(CharEventArgs& e)
{
	if (!_enabled || !_26c)
		return;
	_text983->SetComposition983(e.text, e.composing983);
	if (_1e4)
	{
		RFRect rect = ClientToScreen();
		ApplyGlobalPadding(rect);
		_text983->SetCursorAtPoint983((int)_1e8, (int)_1ec, (int)(_1e8 - rect.left));
		_1e4 = 0;
		_1e8 = 0.0f;
		_1ec = 0.0f;
	}
	e.handled = true;
}

// 004278d0: foco; editavel e habilitada: caret volta a aparecer, IME e teclado da interface ligados
void RFTextArea::OnSetFocus()
{
	RFWnd::OnSetFocus();
	_1a0 = true;
	if (_enabled && _26c)
	{
		ResetCaretBlink();
		RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->SetEnable983(true);
		g_pRFInterface983->OnKeyboardAcquired(true);
	}
}

// 00427950: o inverso do OnSetFocus (interface primeiro, depois o IME)
void RFTextArea::OnKillFocus()
{
	RFWnd::OnKillFocus();
	_1a0 = false;
	if (_enabled && _26c)
	{
		g_pRFInterface983->OnKeyboardAcquired(false);
		RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->SetEnable983(false);
	}
}

// 004283a0
void RFTextArea::ResetCaretBlink()
{
	if (_caret983)
		_caret983->ResetBlink983();
}

// 00427f90
void RFTextArea::AutoFit(bool fit)
{
	_26d = fit;
	if (fit)
		FitToContent(false);
}

// 00428300
int RFTextArea::GetNumOfLine() const
{
	return (int)_text983->TotalLines983();
}

// 00428220: altura (arredondada) do "A" na fonte do controle
int RFTextArea::GetTextHeight()
{
	RFSize sz = g_pRFInterface983->GetTextExtent983(_fontInfo, std::wstring(L"A"));
	return (int)((double)sz.height * 130.0 / 100.0 + 0.5); // 00428220: altura da linha = 130% da fonte
}

// 00428310: caret na posicao do cursor do paragrafo corrente (+0x28/+0x2c); avisa o IME onde fica a janela de composicao
void RFTextArea::DrawCaret(float time)
{
	if (_1a0 && _enabled && _26c && _caret983)
	{
		RFParagraph983* para = _text983->_current;
		int x = para->_f28;
		int y = para->_f2c;
		float pos[2] = { (float)x, (float)y };
		_caret983->Draw983(pos, GetAlpha(false), time);
		RFInputMethodService* ime = RFInputMethodProvider::GetInstance()._ime;
		if (ime)
			ime->SetCompositionPos983(x, y);
	}
}

// 00426e00: clique esquerdo: link primeiro; senao (editavel) o cursor vai para o ponto, ou, com a composicao do IME ativa
// (+0x4c do RFText), o clique fica pendente (+0x1e4) para o OnChar
void RFTextArea::OnLButtonDown(MouseEventArgs& e)
{
	RFWnd::OnLButtonDown(e);
	e.handled = true;
	if (!_enabled)
		return;
	if (HitTestHyperLink(e, true))
		return;
	if (!_26c)
		return;
	if (_text983->_p4c)
	{
		_1e4 = 1;
		_1e8 = e.pos.x;
		_1ec = e.pos.y;
		return;
	}
	RFRect rect = ClientToScreen();
	ApplyGlobalPadding(rect);
	_text983->SetCursorAtPoint983((int)e.pos.x, (int)e.pos.y, (int)(e.pos.x - rect.left));
	if (_caret983)
		_caret983->ResetBlink983();
}

// 00426ee0 (o RFWnd::OnRButtonDown embutido: evento 00e46a50)
void RFTextArea::OnRButtonDown(MouseEventArgs& e)
{
	RFWnd::OnRButtonDown(e);
	if (_enabled && !HitTestHyperLink(e, true))
		e.handled = true;
}

// 00426f30
void RFTextArea::OnRButtonUp(MouseEventArgs& e)
{
	RFWnd::OnRButtonUp(e);
	if (_enabled && !HitTestHyperLink(e, false))
		e.handled = true;
}

// 00426f60: link sob o mouse? botao esquerdo (down = 2; sem down o modo nao e inicializado no 983: 0 aqui), direito (modo = !down);
// outros botoes nao. Achado: evento HyperLinkClicked (texto do link) na janela e o global do RFManager (+ o nome da janela)
bool RFTextArea::HitTestHyperLink(MouseEventArgs& e, bool down)
{
	int mode;
	if (e._14 == 0)
		mode = down ? 2 : 0;
	else if (e._14 == 1)
		mode = down ? 0 : 1;
	else
		return false;
	std::wstring link;
	if (!_text983->HitTestLink983((int)e.pos.x, (int)e.pos.y, mode, &link))
		return false;
	HyperLinkTextEventArgs983 args(this, link);
	SendEvent(EventHyperLinkClicked, args);
	RFManager::GetInstance().PostHyperLinkEvent(this, _name, link);
	return true;
}

// 00427170: sem hover nos trechos e (modo 3 = qualquer link) o link sob o mouse: entrando (ou trocando de link) manda o
// HyperLinkEnter + o global (janela, nome, link); saindo (+0x200 ligado) manda o HyperLinkLeave + o global Leave
void RFTextArea::OnMouseMove(MouseEventArgs& e)
{
	RFWnd::OnMouseMove(e);
	_text983->ResetHover983();
	std::wstring link;
	if (_text983->HitTestLink983((int)e.pos.x, (int)e.pos.y, 3, &link))
	{
		bool enter = false;
		if (!_200)
		{
			_200 = true;
			_204.assign(link, 0, std::wstring::npos);
			enter = true;
		}
		else if (_204.compare(0, _204.size(), link.c_str(), link.size()) != 0)
		{
			_204.assign(link, 0, std::wstring::npos);
			enter = true;
		}
		if (enter)
		{
			HyperLinkTextEventArgs983 args(this, link);
			SendEvent(EventHyperLinkEnter, args);
			RFManager::GetInstance().PostGlobalHyperLinkEnterEvent(this, _name, link);
		}
	}
	else if (_200)
	{
		_200 = false;
		WindowEventArgs_<RFWnd> args(this);
		SendEvent(EventHyperLinkLeave, args);
		RFManager::GetInstance().PostGlobalHyperLinkLeaveEvent(this);
	}
}

// 004274e0 (o RFWnd::OnMouseLeave embutido: evento 00e46ac0)
void RFTextArea::OnMouseLeave()
{
	RFWnd::OnMouseLeave();
	_text983->ResetHover983();
	if (_200)
	{
		_200 = false;
		WindowEventArgs_<RFWnd> args(this);
		SendEvent(EventHyperLinkLeave, args);
		RFManager::GetInstance().PostGlobalHyperLinkLeaveEvent(this);
	}
}

// 004279c0: fundo (RFWndEx), posiciona o texto (Invaildate), desenha o texto com o alfa da janela e o caret, tudo recortado
// pela area cliente menos o _padding (sem a margem global); area negativa nao desenha
void RFTextArea::OnDraw()
{
	RFWndEx::OnDraw();
	RFRect rect = ClientToScreen();
	rect.left += _padding.left;
	rect.top += _padding.top;
	rect.right -= _padding.right;
	rect.bottom -= _padding.bottom;
	if (rect.right - rect.left < 0.0f)
		return;
	if (rect.bottom - rect.top < 0.0f)
		return;
	Invaildate();
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, rect);
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	_text983->Draw983(GetAlpha(false));
	DrawCaret(_rc._depth);
	RFRect clip2;
	clip2.IntersectRect(_rc._clipRect, rect);
	_rc._clipRect = clip2;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 00427d70: tamanho = (largura do texto, se fitWidth) e altura do texto + margens. O 983 soma _padding.top e .bottom (+ globais)
// as DUAS dimensoes (a largura tambem; conferido no asm: [esp+0x30]/[esp+0x34])
void RFTextArea::FitToContent(bool fitWidth)
{
	RFSize size = GetSize();
	RFManager* m = &RFManager::GetInstance();
	float top = _padding.top;
	float bottom = _padding.bottom;
	if (m->_globalPaddingEnabled)
	{
		top = top + m->_globalPadding.top;
		bottom = bottom + m->_globalPadding.bottom;
	}
	if (fitWidth)
		size.width = (float)_text983->MaxWidth983() + top + bottom;
	size.height = (float)(unsigned int)_text983->TotalHeight983() + top + bottom;
	if (size.width != _localSize.width || size.height != _localSize.height)
	{
		_localSize = size;
		OnLayoutChanged(2);
	}
}

// 00427fb0: posiciona os paragrafos a partir do canto da area cliente (margens) menos a rolagem (+0x1a8)
void RFTextArea::Invaildate()
{
	RFRect rect = ClientToScreen();
	ApplyGlobalPadding(rect);
	_text983->Position983((int)rect.left, _lineLimit983 + (int)rect.top);
}

// 00428040: HyperLink do texto de imagem (estilo 8): evento RichTextDrawImage com a posicao e o nome da imagem
void RFTextArea::SendDrawImageEvent(const RFVector2& pos, const std::wstring& name)
{
	RichTextDrawImageEventArgs983 args(this, pos, name);
	SendEvent(EventRichTextDrawImage, args);
}

// D857: acessos da visão do jogo (rfbase.h) com o layout real da lib
int RFTextArea::GetFontSize983()
{
	return _fontInfo.size;
}

void RFTextArea::SetFontColor983(unsigned int color)
{
	_fontInfo.color = color;
}

float RFTextArea::GetPaddingWidth983()
{
	return _padding.right - _padding.left;
}

float RFTextArea::GetPaddingHeight983()
{
	return _padding.bottom - _padding.top;
}
