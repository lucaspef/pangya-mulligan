// JP 983: RFCheck.cpp da Refresh (B160, F3.7.13 item 5): a caixa de marcar (00425a80–00425f40). Um pincel por estado (desmarcada,
// marcada) feito dos ImageSet da <CheckBox>; som PUSH ao soltar; troca o estado e manda o EventChanged (CheckChangedEventArgs)
// no soltar dentro da caixa. ToString/GetTypeName em linha no rfcontrols.h; strings no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfbrush.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// 00425a80
RFCheck::RFCheck()
{
}

// 00425ab0
RFCheck::~RFCheck()
{
}

// 00425b00
void RFCheck::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	CheckBase* check = static_cast<CheckBase*>(base);
	_imageSets[0] = static_cast<ImageSet*>(check->_uncheckedImage);
	_imageSets[1] = static_cast<ImageSet*>(check->_checkedImage);
	_sound983.assign(check->_pushSound.GetRef983(), 0, std::wstring::npos);
}

// 00425b50 (o RFWnd::PreInitialize embutido)
void RFCheck::PreInitialize()
{
	RFWnd::PreInitialize();
	_brushes[0] = 0;
	_brushes[1] = 0;
	_check = false;
}

// 00425bc0
void RFCheck::LoadResource()
{
	_brushes[0] = RFBrush::Create(_imageSets[0]);
	_brushes[1] = RFBrush::Create(_imageSets[1]);
}

// 00425bf0
void RFCheck::OnDestroy()
{
	RFBrush** brush = _brushes;
	for (int i = 2; i != 0; --i, ++brush)
	{
		if (*brush)
		{
			delete *brush;
			*brush = 0;
		}
	}
}

// 00425c20
bool RFCheck::GetCheck() const
{
	return _check;
}

// 00425c30
void RFCheck::SetCheck(bool check)
{
	_check = check;
}

// 00425c40
void RFCheck::OnLButtonDown(MouseEventArgs& e)
{
	RFManager::GetInstance()._capturedWnd = this;
	RFWnd::OnLButtonDown(e);
	e.handled = true;
}

// 00425c90: soltar dentro da caixa depois de apertar nela troca o estado
void RFCheck::OnLButtonUp(MouseEventArgs& e)
{
	RFManager::GetInstance()._capturedWnd = 0;
	bool dragging = _dragging;
	RFWnd::OnLButtonUp(e);
	PlaySfx(_sound983);
	e.handled = true;
	if (dragging && HitTest(e.pos, false))
	{
		_check = !_check;
		CheckChangedEventArgs changed(this, _check);
		SendEvent(EventChanged, changed);
	}
}

// 00425d80: o pincel do estado (sem pincel de desabilitada)
void RFCheck::OnDraw()
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
	_brushes[_check != false]->Draw(pos, GetSize(), alpha, depth);
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}
