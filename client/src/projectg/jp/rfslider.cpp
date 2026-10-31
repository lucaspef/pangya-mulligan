// JP 983: RFSlider.cpp da Refresh (B213, F3.7.13): o controle deslizante (0044ed60-0044fcb0), primo menor da RFScrollBar
// (rfscrollbar.cpp): dois botoes (LT/RB) e o polegar (RFThumbButton) do <Slider>; posicao inteira em [_range983.x, _range983.y]
// (Create: 0..5, passo 1), sem rolagem suave. Sem fonte no 645/839: escrito pelo asm do 983. O clique/arrasto na trilha leva o
// centro do polegar ao ponto e prende o mouse no controle (RFManager +0x5c) ate o OnLButtonUp.
// ToString/GetTypeName em linha no rfcontrols.h; strings no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um metodo com esse nome)
#include <string>
#include <functional>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

// 0044ed60
RFSlider::RFSlider()
{
	_range983.x = 0;
	_range983.y = 0;
	_slideRect983.left = 0.0f;
	_slideRect983.right = 0.0f;
	_slideRect983.top = 0.0f;
	_slideRect983.bottom = 0.0f;
	_dragging983 = false;
}

// 0044eda0
RFSlider::~RFSlider()
{
}

// 0044edb0: botoes e polegar pela fabrica fixa (RFButton/RFThumbButton), como a RFScrollBar
void RFSlider::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_ltButton983 = 0;
	_rbButton983 = 0;
	_thumb983 = 0;
	SliderBase* slider = dynamic_cast<SliderBase*>(base);
	_slideRect983.left = slider->_slideRect.left;
	_slideRect983.right = slider->_slideRect.right;
	_slideRect983.top = slider->_slideRect.top;
	_slideRect983.bottom = slider->_slideRect.bottom;
	_horizontal983 = slider->_horizontal;

	RFButton* button = new RFButton;
	button->Create(slider->_buttonLT, this);
	button->Initialize();
	button->_subControl = true;
	_ltButton983 = button;

	button = new RFButton;
	button->Create(slider->_buttonRB, this);
	button->Initialize();
	button->_subControl = true;
	_rbButton983 = button;

	RFThumbButton* thumb = new RFThumbButton;
	thumb->Create(slider->_thumb, this);
	thumb->Initialize();
	thumb->_subControl = true;
	_thumb983 = thumb;
}

// 0044ef60 (o RFWnd::PreInitialize embutido)
void RFSlider::PreInitialize()
{
	RFWnd::PreInitialize();
	_clipChildren = true;
	_range983.x = 0;
	_range983.y = 5;
	_pos983 = 0;
	_size983 = 1;
	_thumb983->_horizontal = _horizontal983;
	_thumb983->_scrollRect = _slideRect983;
	RegisterTo(_ltButton983->_name, RFButton::EventClicked, Subscriber(&RFSlider::OnLTButClicked, this));
	RegisterTo(_rbButton983->_name, RFButton::EventClicked, Subscriber(&RFSlider::OnRBButClicked, this));
	RegisterTo(_thumb983->_name, RFWnd::EventMouseMove, Subscriber(&RFSlider::OnThumbMouseMove, this));
}

// 0044f1e0: habilita/desabilita o polegar e os dois botoes (EnableWindow embutido: perde o "active" ao desabilitar)
void RFSlider::OnEnabled(bool enable)
{
	RFWnd* wnds[3] = { _thumb983, _ltButton983, _rbButton983 };
	for (int i = 0; i < 3; i++)
		wnds[i]->EnableWindow(enable);
}

// 0044f270: clique na trilha = o centro do polegar vai para o ponto; as pontas valem o minimo/maximo
void RFSlider::OnLButtonDown(MouseEventArgs& e)
{
	RFWnd::OnLButtonDown(e);
	RFVector2 pos;
	pos.y = e.pos.y;
	pos.x = (float)(e.pos.x - (_thumb983->GetSize().width - 0.0) * 0.5);
	pos.y = (float)(pos.y - (_thumb983->GetSize().height - 0.0) * 0.5);
	PositionToLevel(pos);
	int half = _size983 / 2;
	if (_pos983 <= half)
		_pos983 = _range983.x;
	else if (_pos983 + half >= _range983.y)
		_pos983 = _range983.y;
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
	SetCapture();
	_dragging983 = true;
	e.handled = true;
}

// 0044f420
void RFSlider::OnLButtonUp(MouseEventArgs& e)
{
	RFWnd::OnLButtonUp(e);
	_dragging983 = false;
	ReleaseCapture();
	e.handled = true;
}

// 0044f470
void RFSlider::SetSlideSize(int size)
{
	_size983 = size;
}

// 0044f480
void RFSlider::SetSlideRange(int min, int max)
{
	_range983.y = max;
	_range983.x = min;
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 0044f530
void RFSlider::SetSlidePos(int pos)
{
	_pos983 = pos;
	if (pos < _range983.x)
		_pos983 = _range983.x;
	else if (pos > _range983.y)
		_pos983 = _range983.y;
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 0044f5f0
int RFSlider::GetSlidePos() const
{
	return _pos983;
}

// 0044f600
void RFSlider::Next()
{
	EventArgs e;
	OnRBButClicked(e);
}

// 0044f660
void RFSlider::Prev()
{
	EventArgs e;
	OnLTButClicked(e);
}

// 0044f6c0
void RFSlider::SlideToTop()
{
	SetSlidePos(_range983.x);
}

// 0044f6d0
void RFSlider::SlideToBottom()
{
	SetSlidePos(_range983.y);
}

// 0044f6e0: durante o arrasto (clique na trilha) a conta e a do OnLButtonDown
void RFSlider::OnMouseMove(MouseEventArgs& e)
{
	RFWnd::OnMouseMove(e);
	if (_dragging983)
	{
		RFVector2 pos;
		pos.y = e.pos.y;
		pos.x = (float)(e.pos.x - (_thumb983->GetSize().width - 0.0) * 0.5);
		pos.y = (float)(pos.y - (_thumb983->GetSize().height - 0.0) * 0.5);
		PositionToLevel(pos);
		int half = _size983 / 2;
		if (_pos983 <= half)
			_pos983 = _range983.x;
		else if (_pos983 + half >= _range983.y)
			_pos983 = _range983.y;
		LevelToThumbPosition();
		SliderPosChangedEventArgs changed(this, _pos983);
		SendEvent(EventPosChanged, changed);
	}
	e.handled = true;
}

// 0044f870: manda o EventMouseWheel e anda um passo (delta >= 0 = LT, < 0 = RB; sem o RFWnd::OnMouseWheel)
void RFSlider::OnMouseWheel(MouseEventArgs& e)
{
	SendEvent(EventMouseWheel, e);
	if (0.0f <= e.wheelDelta983)
		OnLTButClicked(e);
	else
		OnRBButClicked(e);
	e.handled = true;
}

// 0044f8e0: o polegar foi arrastado (so se o polegar esta com o mouse preso)
void RFSlider::OnThumbMouseMove(EventArgs&)
{
	RFWnd* wnd = _thumb983;
	if (!wnd->_dragging)
		return;
	RFVector2 pos(0.0f, 0.0f);
	for (; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	PositionToLevel(pos);
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 0044f9d0 (sem o e.handled: o 983 so mexe na posicao)
void RFSlider::OnLTButClicked(EventArgs&)
{
	_pos983 -= _size983;
	if (_pos983 < _range983.x)
		_pos983 = _range983.x;
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 0044fa80
void RFSlider::OnRBButClicked(EventArgs&)
{
	_pos983 += _size983;
	if (_pos983 > _range983.y)
		_pos983 = _range983.y;
	LevelToThumbPosition();
	SliderPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 0044fb30: canto do polegar (tela) -> nivel
void RFSlider::PositionToLevel(const RFVector2& pos)
{
	if (_range983.y - _range983.x <= 0)
		return;
	float offset;
	float length;
	if (_horizontal983)
	{
		offset = (float)((double)pos.x - GetScreenPos().x) - _slideRect983.left;
		float track = _slideRect983.right - _slideRect983.left;
		float thumb = (float)(_thumb983->GetSize().width - 0.0);
		length = track - thumb;
	}
	else
	{
		offset = (float)((double)pos.y - GetScreenPos().y) - _slideRect983.top;
		float track = _slideRect983.bottom - _slideRect983.top;
		float thumb = (float)(_thumb983->GetSize().height - 0.0);
		length = track - thumb;
	}
	int min = _range983.x;
	float unit = length / (float)(_range983.y - min);
	float level = offset / unit + 0.5;
	int value = (int)level + min;
	if (value < min)
		value = min;
	else if (value > _range983.y)
		value = _range983.y;
	_pos983 = value;
}

// 0044fcb0: nivel -> posicao do polegar na tela
void RFSlider::LevelToThumbPosition()
{
	if (_range983.y - _range983.x <= 0)
		return;
	float offset;
	float x, y;
	if (_horizontal983)
	{
		float track = _slideRect983.right - _slideRect983.left;
		float thumb = (float)(_thumb983->GetSize().width - 0.0);
		float length = track - thumb;
		offset = length / (float)(_range983.y - _range983.x) * (float)(_pos983 - _range983.x) + 0.5;
		float screenX = GetScreenPos().x;
		float thumbY = (float)((double)_thumb983->GetPos().y + GetScreenPos().y);
		x = _slideRect983.left + screenX + offset;
		y = _slideRect983.top + thumbY;
	}
	else
	{
		float track = _slideRect983.bottom - _slideRect983.top;
		float thumb = (float)(_thumb983->GetSize().height - 0.0);
		float length = track - thumb;
		offset = length / (float)(_range983.y - _range983.x) * (float)(_pos983 - _range983.x) + 0.5;
		float thumbX = (float)((double)_thumb983->GetPos().x + GetScreenPos().x);
		float screenY = GetScreenPos().y;
		x = _slideRect983.left + thumbX;
		y = _slideRect983.top + screenY + offset;
	}
	_thumb983->SetScreenPos(x, y);
}
