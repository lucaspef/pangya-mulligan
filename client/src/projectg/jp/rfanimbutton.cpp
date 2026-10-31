// JP 983: RFAnimButton.cpp da Refresh (B160, F3.7.13 item 5): o botão animado (004344f0–00434fa4). Desenha pelo RFDirector próprio:
// uma cena por estado (NORMAL/DOWN/OVER/DISABLE, carregadas dos caminhos dos ImageSet da <AnimButton>) e as cenas de passagem
// ENTER/LEAVE (mouse entrando/saindo), que tocam até o fim e voltam à cena do estado. Com <Asset Name="Group"> o arquivo do
// _normalImage traz todas as cenas. O LTCG embute o RFDirector::Play(nome)/IsLoop/End/FrameMove/Render: aqui pelas chamadas.
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <assert.h>
#include <string>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfrenderer.h"
#include "refresh/lib/rfmanager.h"
#include "refresh/lib/rfcontrols.h"
#include "refresh/lib/rfdirector.h"
#include "refresh/rfimageset.h"
#include "refresh/rfproperty.h"
#include "refresh/rfwndstruct.h"

extern RFRenderer* g_pRFRenderer983; // 00f1de20 (rfmanager.cpp)

// nomes das cenas por estado (índice = RFBUTTON_STATE; 4 e 5 = passagem); estáticos do RFAnimButton.cpp (00cc79b0–00cc7a50)
static const std::wstring s_animNormal983 = L"NORMAL"; // 00e46f3c
static const std::wstring s_animDown983 = L"DOWN"; // 00e46f58
static const std::wstring s_animOver983 = L"OVER"; // 00e46f74
static const std::wstring s_animDisable983 = L"DISABLE"; // 00e46f90
static const std::wstring s_animEnter983 = L"ENTER"; // 00e46fac
static const std::wstring s_animLeave983 = L"LEAVE"; // 00e46fc8
extern const std::wstring g_rfEmptyString983; // 00e46f04

enum { ANIM_ENTER983 = 4, ANIM_LEAVE983 = 5 };

// 004344f0
RFAnimButton::RFAnimButton()
	: _director(0), _animState983(0), _group(false)
{
}

// 00434560
RFAnimButton::~RFAnimButton()
{
}

// 00434570
void RFAnimButton::Create(WndBaseStruct* base, RFWnd* parent)
{
	RFButton::Create(base, parent);
	AnimButtonBase* button = dynamic_cast<AnimButtonBase*>(base);
	_enterImage = static_cast<ImageSet*>(button->_enterImage);
	_leaveImage = static_cast<ImageSet*>(button->_leaveImage);
	_group = button->_group;
}

// 004345d0
void RFAnimButton::PreInitialize()
{
	RFButton::PreInitialize();
	_director = new RFDirector;
	_animState983 = 0;
}

// 00434650 (o RFButton::OnDestroy embutido)
void RFAnimButton::OnDestroy()
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
	RFButton::OnDestroy();
}

// 004346a0: cenas dos caminhos das imagens (sem pincéis) e a do estado
void RFAnimButton::LoadResource()
{
	if (!_group)
	{
		LoadAnimation_(static_cast<SingleImageSet*>(_normalImage)->_path);
		LoadAnimation_(static_cast<SingleImageSet*>(_downImage)->_path);
		LoadAnimation_(static_cast<SingleImageSet*>(_overImage)->_path);
		LoadAnimation_(static_cast<SingleImageSet*>(_disableImage)->_path);
		LoadAnimation_(static_cast<SingleImageSet*>(_enterImage)->_path);
		LoadAnimation_(static_cast<SingleImageSet*>(_leaveImage)->_path);
	}
	else
		LoadAnimation_(static_cast<SingleImageSet*>(_normalImage)->_path);
	_director->Play(GetAnimName(_state));
}

// 00434770: diretor novo com as cenas de um arquivo
void RFAnimButton::LoadAnimation(const std::wstring& path)
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
	_director = new RFDirector;
	LoadAnimation_(path);
	_director->Play(GetAnimName(_state));
}

// 00434840
void RFAnimButton::UnloadAnimation()
{
	if (_director)
	{
		delete _director;
		_director = 0;
	}
}

// 00434870: o botão passa a ser dono do diretor
R_ERRORTYPE RFAnimButton::SetAnimation(RFDirector* director)
{
	if (!director)
		return R_FAIL983;
	UnloadAnimation();
	_director = director;
	_director->Play(GetAnimName(_state));
	return R_OK983;
}

// 004348e0
void RFAnimButton::SetAnimPos(float x, float y)
{
	_director->SetPosition(RFVector2(x, y));
}

// 00434910
const std::wstring& RFAnimButton::GetAnimName(int state) const
{
	if (state >= 6)
		_wassert(L"state < 6", L"src\\RFAnimButton.cpp", 134);
	switch (state)
	{
	case RFBS_NORMAL983:
		return s_animNormal983;
	case RFBS_DOWN983:
		return s_animDown983;
	case RFBS_OVER983:
		return s_animOver983;
	case RFBS_DISABLE983:
		return s_animDisable983;
	case ANIM_ENTER983:
		return s_animEnter983;
	case ANIM_LEAVE983:
		return s_animLeave983;
	}
	return g_rfEmptyString983;
}

// 004349a0
void RFAnimButton::OnStateChanged()
{
	_director->Play(GetAnimName(_state));
	_animState983 = 0;
}

// 004349f0 (o RFButton::OnLButtonDown embutido)
void RFAnimButton::OnLButtonDown(MouseEventArgs& e)
{
	RFButton::OnLButtonDown(e);
	if (!_pressed)
		_director->Play(GetAnimName(_state));
	_animState983 = 0;
}

// 00434aa0: cena do estado novo; a que não repete termina
void RFAnimButton::OnLButtonUp(MouseEventArgs& e)
{
	RFButton::OnLButtonUp(e);
	if (!_pressed)
	{
		const std::wstring& name = GetAnimName(_state);
		_director->Play(name);
		RFAnimation* animation = _director->GetAnimation(name);
		if (!animation)
			_wassert(L"!\"Animation not found.\"", L"src\\RFDirector.cpp", 301);
		if (!animation || !animation->_repeat983)
			_director->End(name);
	}
	_animState983 = 0;
}

// 00434b60 (o RFButton::OnMouseEnter embutido): ENTER se existir (e o botão não estiver apertado), senão a do estado
void RFAnimButton::OnMouseEnter()
{
	RFButton::OnMouseEnter();
	if (_pressed)
		return;
	_animState983 = ANIM_ENTER983;
	if (_director->GetAnimation(GetAnimName(ANIM_ENTER983)) && _state != RFBS_DOWN983)
		_director->Play(GetAnimName(_animState983));
	else
	{
		_animState983 = 0;
		_director->Play(GetAnimName(_state));
	}
}

// 00434c60: LEAVE se existir (botão habilitado), senão a do estado
void RFAnimButton::OnMouseLeave()
{
	RFButton::OnMouseLeave();
	if (_pressed || !_enabled || (_super && !_super->IsWindowEnabled(false)))
	{
		_animState983 = 0;
		return;
	}
	_animState983 = ANIM_LEAVE983;
	if (_director->GetAnimation(GetAnimName(ANIM_LEAVE983)))
		_director->Play(GetAnimName(_animState983));
	else
	{
		_animState983 = 0;
		_director->Play(GetAnimName(_state));
	}
}

// 00434d10: a cena de passagem que acabou volta à do estado
void RFAnimButton::OnUpdate(UpdateEventArgs& e)
{
	if (_animState983)
	{
		RFAnimation* animation = _director->GetAnimation(GetAnimName(_animState983));
		if (!animation || !animation->_playing983)
		{
			_animState983 = 0;
			_director->Play(GetAnimName(_state));
		}
	}
	_director->FrameMove(e.delta);
	RFWnd::OnUpdate(e);
}

// 00434db0
void RFAnimButton::OnDraw()
{
	RFVector2 pos(0.0f, 0.0f);
	for (RFWnd* wnd = this; wnd; wnd = wnd->_super)
	{
		RFVector2 p = wnd->GetPos();
		pos.x += p.x;
		pos.y += p.y;
	}
	_director->SetPosition(pos);
	RFRect clip;
	clip.IntersectRect(_rc._clipRect, ClientToScreen());
	if (_rc._clip)
		g_pRFRenderer983->PushClip983(clip);
	_director->Render(GetAlpha(false));
	RFRect rect;
	rect.IntersectRect(_rc._clipRect, ClientToScreen());
	_rc._clipRect = rect;
	if (_rc._clip)
		g_pRFRenderer983->PopClip983();
}

// 00434f30
void RFAnimButton::LoadAnimation_(const std::wstring& path)
{
	if (path.size())
		_director->LoadAnimation(path);
}

// 00434f50
void RFAnimButton::OnEnabled(bool enable)
{
	_state = enable ? RFBS_NORMAL983 : RFBS_DISABLE983;
	_director->Play(GetAnimName(_state));
}
