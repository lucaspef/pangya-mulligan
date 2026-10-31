// JP 983: RFProgress.cpp da Refresh (B211, F3.7.13): a barra de progresso (0044d2b0-0044d6a0). Sem fonte no 645/839: escrito
// pelo asm do 983. O pincel (RFBrush::Create do ImageSet da <ProgressBar>) e desenhado so ate a fracao (progresso-min)/(max-min):
// na horizontal a largura vezes a fracao; na vertical a altura vezes a fracao, de baixo para cima (y soma (1-fracao)*altura).
// ToString/GetTypeName/SetHorizontal/IsHorizontal em linha no rfcontrols.h; TypeName no rfcontrols.cpp.
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/rfimageset.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// 0044d2b0
RFProgress::RFProgress()
{
}

// 0044d2d0
RFProgress::~RFProgress()
{
}

// 0044d2e0
void RFProgress::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_imageSet983 = static_cast<ImageSet*>(static_cast<ProgressBase*>(base)->_image);
}

// 0044d310 (o RFWnd::PreInitialize embutido; depois os campos da barra: horizontal, 0..100, em 50)
void RFProgress::PreInitialize()
{
	RFWnd::PreInitialize();
	_brush983 = 0;
	_min983 = 0;
	_horizontal = true;
	_max983 = 100;
	_progress983 = 50;
	_188 = true;
}

// 0044d3a0
void RFProgress::LoadResource()
{
	_brush983 = RFBrush::Create(_imageSet983);
}

// 0044d3c0
void RFProgress::OnDestroy()
{
	if (_brush983)
	{
		delete _brush983;
		_brush983 = 0;
	}
}

// 0044d3f0 (o 983 chama GetSize duas vezes e soma GetPos dos pais em laco proprio; mesma conta)
void RFProgress::OnDraw()
{
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, ClientToScreen());
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	RFSize full = GetSize();
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	float ratio = (float)(_progress983 - _min983) / (float)(_max983 - _min983);
	RFSize size;
	if (_horizontal)
	{
		size.width = full.width * ratio;
		size.height = full.height;
	}
	else
	{
		pos.y += (1.0f - ratio) * full.height;
		size.width = full.width;
		size.height = full.height * ratio;
	}
	float alpha = GetAlpha(false);
	if (_brush983)
		_brush983->Draw(pos, size, alpha, _rc._depth);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 0044d620: o progresso e puxado para dentro da nova faixa
void RFProgress::SetRange(int minValue, int maxValue)
{
	_min983 = minValue;
	_max983 = maxValue;
	if (_progress983 < minValue)
		_progress983 = minValue;
	else if (_progress983 > maxValue)
		_progress983 = maxValue;
}

// 0044d660
void RFProgress::SetProgress(int value)
{
	_progress983 = value;
	if (value < _min983)
		_progress983 = _min983;
	else if (value > _max983)
		_progress983 = _max983;
}

// 0044d690
int RFProgress::GetProgress() const
{
	return _progress983;
}
