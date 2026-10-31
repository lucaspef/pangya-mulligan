// JP 983: o polegar da barra de rolagem da Refresh (B161, F3.7.13 item 5; 0045a480–0045a7e0, classe não exportada): um RFButton
// arrastável que, no arrasto, prende a posição do mouse ao retângulo de rolagem da barra (_scrollRect, relativo à barra) e trava
// o outro eixo. A barra (rfscrollbar.cpp) escuta o EventMouseMove dele.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

// 0045a480
const wchar_t* RFThumbButton::ToString() const
{
	return L"RFThumbButton";
}

// 0045a490 (o construtor do RFButton embutido)
RFThumbButton::RFThumbButton()
{
	_horizontal = true;
}

// 0045a550
void RFThumbButton::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFButton::Create(base, parent);
}

// 0045a560
void RFThumbButton::PreInitialize()
{
	RFButton::PreInitialize();
	_dragable = true;
}

// 0045a5b0: e.pos (tela) - ponto do clique = canto do polegar; preso entre o começo e o fim do retângulo de rolagem
void RFThumbButton::OnMouseMove(MouseEventArgs& e)
{
	RFRect rc = _super->GetScreenRect();
	if (_horizontal)
	{
		rc.left = _scrollRect.left + rc.left;
		float width = _scrollRect.right - _scrollRect.left;
		float left = rc.left;
		rc.right = left + width;
		float x = (float)(int)(e.pos.x - _dragPos.x);
		if (x < left)
			e.pos.x = left + _dragPos.x;
		else if (rc.right < x + GetSize().width)
			e.pos.x = _dragPos.x + rc.right - GetSize().width;
		float y = 0.0f;
		for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
			y += wnd->GetPos().y;
		e.pos.y = _dragPos.y + y;
	}
	else
	{
		rc.top = _scrollRect.top + rc.top;
		float height = _scrollRect.bottom - _scrollRect.top;
		float top = rc.top;
		rc.bottom = top + height;
		float y = (float)(int)(e.pos.y - _dragPos.y);
		if (y < top)
			e.pos.y = top + _dragPos.y;
		else if (rc.bottom < y + GetSize().height)
			e.pos.y = _dragPos.y + rc.bottom - GetSize().height;
		float x = 0.0f;
		for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
			x += wnd->GetPos().x;
		e.pos.x = _dragPos.x + x;
	}
	RFWnd::OnMouseMove(e);
	e.handled = true;
}
