// JP 983: RFWndEx.cpp da Refresh (B154, F3.7.13): a janela com fundo (0041d980–0041dc06), base das janelas do jogo criadas por XML
// (RFFormBase<RFWndEx>, RFFormModalBase<RFWndEx>, RFFormDesktopBase<RFWndEx>). O fundo é um pincel (rfbrush.cpp) feito do
// ImageSet do <WndEx>. ToString/GetTypeName em linha no rfcontrols.h.
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// 00e46cd4 (inicializador 00cc76f0)
const std::wstring RFWndEx::TypeName = L"Window";

// 0041d980
RFWndEx::RFWndEx()
	: _brush(0), _imageSet(0)
{
}

// D857: visão do jogo (rfscenemanager.cpp IsOtherLayer983 006037a0 lê +0x17c como int)
int RFWndEx::GetImageSet983()
{
	return reinterpret_cast<int>(_imageSet);
}

// 0041d9a0
RFWndEx::~RFWndEx()
{
}

// 0041d9b0
void RFWndEx::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_imageSet = static_cast<ImageSet*>(static_cast<WndExBase*>(base)->_background);
}

// 0041d9e0 (o RFWnd::PreInitialize embutido)
void RFWndEx::PreInitialize()
{
	RFWnd::PreInitialize();
}

// 0041da40
void RFWndEx::LoadResource()
{
	_brush = RFBrush::Create(_imageSet);
}

// 0041da60
void RFWndEx::OnDestroy()
{
	if (_brush)
	{
		delete _brush;
		_brush = 0;
	}
}

// 0041da90: o fundo no retângulo da janela, com o recorte do contexto (a posição soma a de todos os pais)
void RFWndEx::OnDraw()
{
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, ClientToScreen());
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	float depth = _rc._depth;
	float alpha = GetAlpha(false);
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	_brush->Draw(pos, GetSize(), alpha, depth);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}
