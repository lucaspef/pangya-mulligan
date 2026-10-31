// JP 983: RFScrollBar.cpp da Refresh (B161, F3.7.13 item 5): a barra de rolagem (00428c00–0042a290). Dois botões (LT/RB) e o
// polegar (RFThumbButton, rfthumbbutton.cpp) criados pelo <Scrollbar>; posição inteira em [_range.x, _range.y]; com
// _smoothScroll os botões/clique/roda andam pela RFSmoothScroll983 (curva de Bézier, 400–1000 ms) no OnUpdate.
// ToString/GetTypeName/GetScrollRange/EnableSmoothScroll em linha no rfcontrols.h; strings no rfcontrols.cpp.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include <functional>
#include <stdlib.h>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

// A802 (temporário): rastro da rolagem da lista de cartas (revisor 17:35, scroll da janela Equip)
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
static bool IsCardListBar983(RFWnd* bar)
{
	return bar->GetSuper(false) && bar->GetSuper(false)->GetName() == L"card_list";
}

// 00428c00
RFSmoothScroll983::RFSmoothScroll983()
{
	elapsed = 0.0f;
	active = false;
	t = 0.0f;
	duration = 600;
	from = 0;
	to = 0;
	curve = new RFBezierSpline(0x400, RFVector2(0.3f, 0.7f), RFVector2(0.5f, 1.0f));
}

// (embutido no 00428ec0)
RFSmoothScroll983::~RFSmoothScroll983()
{
	if (curve)
	{
		delete curve;
		curve = 0;
	}
}

// 00428cf0: o tempo é contado em ms inteiros antes de somar o quadro (como o 983)
int RFSmoothScroll983::Step(float dt)
{
	unsigned int ms = (unsigned int)(elapsed * 1000.0 + 0.5);
	elapsed += dt;
	float tt = (float)((ms - 0.0) / duration);
	t = tt;
	if (tt < 1.0f)
	{
		int start = from;
		float value = curve->GetValue(tt) * (to - start) + start;
		return (int)(value + 0.5);
	}
	active = false;
	return to;
}

// 00428e30
RFScrollBar::RFScrollBar()
{
}

// 00428ec0
RFScrollBar::~RFScrollBar()
{
}

// 00428f30: botões e polegar pela fábrica fixa (RFButton/RFThumbButton), não pelo RFManager
void RFScrollBar::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFWnd::Create(base, parent);
	_ltButton983 = 0;
	_rbButton983 = 0;
	_thumb983 = 0;
	ScrollbarBase* bar = dynamic_cast<ScrollbarBase*>(base);
	_scrollRect983.left = bar->_scrollRect.left;
	_scrollRect983.right = bar->_scrollRect.right;
	_scrollRect983.top = bar->_scrollRect.top;
	_scrollRect983.bottom = bar->_scrollRect.bottom;
	_horizontal983 = bar->_horizontal;

	RFButton* button = new RFButton;
	button->Create(bar->_buttonLT, this);
	button->Initialize();
	button->_subControl = true;
	_ltButton983 = button;

	button = new RFButton;
	button->Create(bar->_buttonRB, this);
	button->Initialize();
	button->_subControl = true;
	_rbButton983 = button;

	RFThumbButton* thumb = new RFThumbButton;
	thumb->Create(bar->_thumb, this);
	thumb->Initialize();
	thumb->_subControl = true;
	_thumb983 = thumb;
	_thumbSize983 = thumb->GetSize();
}

// 00429110 (o RFWnd::PreInitialize embutido)
void RFScrollBar::PreInitialize()
{
	RFWnd::PreInitialize();
	_clipChildren = true;
	_range.x = 0;
	_range.y = 100;
	_pos983 = 0;
	_size983 = 1;
	_smoothScroll = false;
	_thumb983->_horizontal = _horizontal983;
	_thumb983->_scrollRect = _scrollRect983;
	RegisterTo(_ltButton983->_name, RFButton::EventClicked, Subscriber(&RFScrollBar::OnLTButClicked, this));
	RegisterTo(_rbButton983->_name, RFButton::EventClicked, Subscriber(&RFScrollBar::OnRBButClicked, this));
	RegisterTo(_thumb983->_name, RFWnd::EventMouseMove, Subscriber(&RFScrollBar::OnThumbMouseMove, this));
}

// 00429390: a margem do fim do retângulo de rolagem repete a do começo
void RFScrollBar::OnLayoutChanged(unsigned int flags)
{
	if (flags & 2)
		ApplyAnchor();
	RFSize size = GetSize();
	if (flags & 2)
	{
		if (!_horizontal983)
			_scrollRect983.bottom = size.height - _scrollRect983.top;
		else
			_scrollRect983.right = size.width - _scrollRect983.left;
		_thumb983->_horizontal = _horizontal983;
		_thumb983->_scrollRect = _scrollRect983;
	}
}

// 00429430
void RFScrollBar::OnEnabled(bool enable)
{
	_thumb983->ShowWindow(enable);
}

// 00429440
void RFScrollBar::OnLButtonUp(MouseEventArgs& e)
{
	RFWnd::OnLButtonUp(e);
	e.handled = true;
}

// 00429460
void RFScrollBar::OnRButtonUp(MouseEventArgs& e)
{
	RFWnd::OnRButtonUp(e);
	e.handled = true;
}

// 00429480: clique na trilha = o centro do polegar vai para o ponto
void RFScrollBar::OnLButtonDown(MouseEventArgs& e)
{
	RFWnd::OnLButtonDown(e);
	RFVector2 pos;
	pos.y = e.pos.y;
	pos.x = (float)(e.pos.x - (_thumb983->GetSize().width - 0.0) * 0.5);
	pos.y = (float)(pos.y - (_thumb983->GetSize().height - 0.0) * 0.5);
	int old = _pos983;
	PositionToLevel(pos);
	if (IsCardListBar983(this))
		jp983_rastro("A802 trilho y=%d old=%d alvo=%d range=%d..%d smooth=%d", (int)e.pos.y, old, _pos983, _range.x, _range.y,
			(int)_smoothScroll);
	_smooth983.Reset();
	if (!_smoothScroll)
	{
		LevelToThumbPosition();
		ScrollBarPosChangedEventArgs changed(this, _pos983);
		SendEvent(EventPosChanged, changed);
	}
	else
	{
		int target = _pos983;
		_pos983 = old;
		MoveToScroll(target);
	}
	e.handled = true;
}

// 00429620 (não chama o RFWnd::OnUpdate)
void RFScrollBar::OnUpdate(UpdateEventArgs& e)
{
	if (_smooth983.active)
		SetScrollPos(_smooth983.Step(e.delta));
}

// 00429660
void RFScrollBar::SetScrollSize(int size)
{
	_size983 = size;
}

// 00429670: polegar NinePatch (vertical) encolhe com a faixa, até o tamanho do Create
void RFScrollBar::SetScrollRange(int min, int max)
{
	_range.y = max;
	_range.x = min;
	SetScrollPos(_pos983);
	if (_thumb983->_normalImage->GetType() == 3)
	{
		float height = (_scrollRect983.bottom - _scrollRect983.top) - (float)(_range.y - _range.x);
		if (height < _thumbSize983.height)
			height = _thumbSize983.height;
		_thumb983->SetSize(RFSize(_thumb983->GetSize().width, height));
	}
}

// 00429770
void RFScrollBar::SetScrollPos(int pos)
{
	_pos983 = pos;
	if (pos < _range.x)
		_pos983 = _range.x;
	else if (pos > _range.y)
		_pos983 = _range.y;
	LevelToThumbPosition();
	ScrollBarPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 00429830
int RFScrollBar::GetScrollPos() const
{
	return _pos983;
}

// 00429840
void RFScrollBar::Next()
{
	EventArgs e;
	OnRBButClicked(e);
}

// 004298a0
void RFScrollBar::Prev()
{
	EventArgs e;
	OnLTButClicked(e);
}

// 00429900
void RFScrollBar::ScrollToTop()
{
	SetScrollPos(_range.x);
}

// 00429910
void RFScrollBar::ScrollToBottom()
{
	SetScrollPos(_range.y);
}

// 00429920
bool RFScrollBar::IsBottom()
{
	return _pos983 == _range.y;
}

// 00429930: cliques durante a rolagem suave aceleram (até 5x o salto)
void RFScrollBar::MoveToScroll(int target)
{
	if (!_smooth983.active)
	{
		_accel983 = 1;
		_smooth983.Reset();
		_smooth983.from = _pos983;
		_smooth983.active = true;
		_smooth983.to = target;
		unsigned int duration = abs(_pos983 - target) * 4;
		if (duration < 400)
		{
			_smooth983.duration = 400;
			return;
		}
		if (duration > 1000)
			duration = 1000;
		_smooth983.duration = duration;
		return;
	}
	if (++_accel983 > 5)
		_accel983 = 5;
	int pos = _pos983;
	_smooth983.from = pos;
	if (pos < target)
	{
		_smooth983.elapsed = 0.0f;
		_smooth983.to = (target - pos) * _accel983 + target;
		return;
	}
	_smooth983.elapsed = 0.0f;
	_smooth983.to = target - (pos - target) * _accel983;
}

// 00429a30: manda o EventMouseWheel e rola um passo (sem o RFWnd::OnMouseWheel)
void RFScrollBar::OnMouseWheel(MouseEventArgs& e)
{
	SendEvent(EventMouseWheel, e);
	if (e.wheelDelta983 < 0.0f)
		OnRBButClicked(e);
	else
		OnLTButClicked(e);
}

// 00429a90
void RFScrollBar::OnThumbMouseMove(EventArgs&)
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
	int old = _pos983;
	PositionToLevel(pos);
	if (IsCardListBar983(this))
		jp983_rastro("A818 polegar soma=%d,%d tela=%d,%d top=%d bottom=%d alt=%d old=%d novo=%d", (int)pos.x, (int)pos.y,
			(int)GetScreenPos().x, (int)GetScreenPos().y, (int)_scrollRect983.top, (int)_scrollRect983.bottom,
			(int)_thumb983->GetSize().height, old, _pos983);
	ScrollBarPosChangedEventArgs changed(this, _pos983);
	SendEvent(EventPosChanged, changed);
}

// 00429b80
void RFScrollBar::OnLTButClicked(EventArgs& e)
{
	if (IsCardListBar983(this))
		jp983_rastro("A802 seta-cima pos=%d range=%d..%d passo=%d smooth=%d ativo=%d", _pos983, _range.x, _range.y,
			_size983, (int)_smoothScroll, (int)_smooth983.active);
	if (_pos983 > _range.x)
	{
		int pos = _pos983 - _size983;
		if (_smoothScroll)
			MoveToScroll(pos);
		else
			SetScrollPos(pos);
		e.handled = true;
	}
}

// 00429bd0
void RFScrollBar::OnRBButClicked(EventArgs& e)
{
	if (IsCardListBar983(this))
		jp983_rastro("A802 seta-baixo pos=%d range=%d..%d passo=%d smooth=%d ativo=%d", _pos983, _range.x, _range.y,
			_size983, (int)_smoothScroll, (int)_smooth983.active);
	if (_pos983 < _range.y)
	{
		int pos = _size983 + _pos983;
		if (_smoothScroll)
			MoveToScroll(pos);
		else
			SetScrollPos(pos);
		e.handled = true;
	}
}

// 00429c20: canto do polegar (tela) -> nível
void RFScrollBar::PositionToLevel(const RFVector2& pos)
{
	if (_range.y - _range.x <= 0)
		return;
	float offset;
	float length;
	if (_horizontal983)
	{
		offset = (float)((double)pos.x - GetScreenPos().x) - _scrollRect983.left;
		float track = _scrollRect983.right - _scrollRect983.left;
		float thumb = (float)(_thumb983->GetSize().width - 0.0);
		length = track - thumb;
	}
	else
	{
		offset = (float)((double)pos.y - GetScreenPos().y) - _scrollRect983.top;
		float track = _scrollRect983.bottom - _scrollRect983.top;
		float thumb = (float)(_thumb983->GetSize().height - 0.0);
		length = track - thumb;
	}
	int min = _range.x;
	float unit = length / (float)(_range.y - min);
	float level = offset / unit + 0.5;
	int value = (int)level + min;
	if (value < min)
		value = min;
	else if (value > _range.y)
		value = _range.y;
	_pos983 = value;
}

// 00429da0: nível -> posição do polegar na tela (a conta da proporção é uma lambda, 0042a040)
void RFScrollBar::LevelToThumbPosition()
{
	if (_range.y - _range.x <= 0)
		return;
	float offset;
	std::tr1::function<void (float)> calc = [&offset, this](float length)
	{
		offset = length / (float)(_range.y - _range.x) * (float)(_pos983 - _range.x) + 0.5;
	};
	float x, y;
	if (_horizontal983)
	{
		float track = _scrollRect983.right - _scrollRect983.left;
		float thumb = (float)(_thumb983->GetSize().width - 0.0);
		calc(track - thumb);
		float screenX = GetScreenPos().x;
		float thumbY = (float)((double)_thumb983->GetPos().y + GetScreenPos().y);
		x = _scrollRect983.left + screenX + offset;
		y = _scrollRect983.top + thumbY;
	}
	else
	{
		float track = _scrollRect983.bottom - _scrollRect983.top;
		float thumb = (float)(_thumb983->GetSize().height - 0.0);
		calc(track - thumb);
		float thumbX = (float)((double)_thumb983->GetPos().x + GetScreenPos().x);
		float screenY = GetScreenPos().y;
		x = _scrollRect983.left + thumbX;
		y = _scrollRect983.top + screenY + offset;
	}
	_thumb983->SetScreenPos(x, y);
}

// 0042a090
void RFScrollBar::SmoothScrolling(float dt)
{
	if (_smooth983.active)
		SetScrollPos(_smooth983.Step(dt));
}

// D857: visão do jogo (rfbase.h m_bWheel983 +0x19c do VC10) com o layout real da lib
void RFScrollBar::EnableSmoothScroll983(bool enable)
{
	_smoothScroll = enable;
}
