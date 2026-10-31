// JP 983: RFVisualEffect.cpp da Refresh (B163, PRIORIDADE 3(b)): os 3 efeitos prontos de abrir/fechar janela (0042cbc0–0042d084).
// O RFWnd::ShowWindow põe _visualState em Showing983/Hiding983 e chama Begin983; o RFWnd::OnUpdate chama Update983 a cada
// quadro até o efeito pôr Visible983/Hidden983. 0 = sem efeito, 1 = esmaecer (alpha em 0,4 s), 2 = cenas "opening"/"closing"
// do diretor de animação da janela (RFWnd::LoadUIAnimation). Nomes dos métodos meus (só o RTTI tem os nomes das classes).
#undef NDEBUG	// a Refresh foi compilada sem NDEBUG (_wassert no 983)
#include <assert.h>
#include <windows.h>
#undef CreateDesktop // (o RFManager tem um método com esse nome)
#include <string>
#include <vector>
#include "refresh/lib/rfwnd.h"
#include "refresh/lib/rfwindowmodifier.h"
#include "refresh/lib/rfdirector.h"

// sem efeito (vtable 00d64cd4)
class RFVisualEffectDefault : public RFVisualEffect
{
public:
	virtual RFVisualEffect* Clone983() { return new RFVisualEffectDefault; } // 0042cbc0
	virtual void Begin983(RFWnd* wnd); // 0042cbe0
	virtual void Update983(RFWnd* wnd, float delta) { } // 0042cc10
};

// esmaecer (vtable 00d64ce8)
class RFVisualEffectFade : public RFVisualEffect
{
public:
	RFVisualEffectFade() : _time(0.0f) { } // 0042cc70
	virtual RFVisualEffect* Clone983() { return new RFVisualEffectFade; } // 0042cc80
	virtual void Begin983(RFWnd* wnd); // 0042cca0
	virtual void Update983(RFWnd* wnd, float delta); // 0042ccc0

	float _time; // +0x04
};

// cenas do diretor da janela (vtable 00d64cfc)
class RFVisualEffectAnim : public RFVisualEffect
{
public:
	virtual RFVisualEffect* Clone983() { return new RFVisualEffectAnim; } // 0042ce10
	virtual void Begin983(RFWnd* wnd); // 0042ce30
	virtual void Update983(RFWnd* wnd, float delta); // 0042cef0
};

// 0042cbe0
void RFVisualEffectDefault::Begin983(RFWnd* wnd)
{
	switch (wnd->_visualState)
	{
	case VisualStateTypes::Showing983:
		wnd->SetVisualState(VisualStateTypes::Visible983);
		break;
	case VisualStateTypes::Hiding983:
		wnd->SetVisualState(VisualStateTypes::Hidden983);
		break;
	}
}

// 0042cca0
void RFVisualEffectFade::Begin983(RFWnd* wnd)
{
	if (wnd->_visualState == VisualStateTypes::Showing983)
		wnd->_alpha = 0.0f;
	_time = 0.0f;
}

static const float FADE_TIME983 = 0.4f;

static inline float Clamp01983(const float& value)
{
	const float zero = 0.0f;
	const float one = 1.0f;
	const float& low = value > 0.0 ? value : zero;
	return low > 1.0 ? one : low;
}

// 0042ccc0
void RFVisualEffectFade::Update983(RFWnd* wnd, float delta)
{
	_time += delta;
	if (_time > (double)0.4f)
		_time = FADE_TIME983;
	switch (wnd->_visualState)
	{
	case VisualStateTypes::Showing983:
	{
		float alpha = (float)(_time / (double)0.4f);
		wnd->_alpha = Clamp01983(alpha);
		if (alpha >= 1.0f)
			wnd->SetVisualState(VisualStateTypes::Visible983);
		break;
	}
	case VisualStateTypes::Hiding983:
	{
		float alpha = (float)(1.0 - _time / (double)0.4f);
		wnd->_alpha = Clamp01983(alpha);
		if (alpha <= 0.0f)
			wnd->SetVisualState(VisualStateTypes::Hidden983);
		break;
	}
	}
}

// 0042ce30: toca a cena (sem Stop, como o 983); sem a cena, termina na hora
void RFVisualEffectAnim::Begin983(RFWnd* wnd)
{
	RFDirector* director = wnd->_animation;
	switch (wnd->_visualState)
	{
	case VisualStateTypes::Showing983:
		if (director->GetAnimation(RFWnd::AnimOpening))
		{
			if (RFAnimation* animation = director->GetAnimation(RFWnd::AnimOpening))
			{
				director->_playing = animation;
				animation->Play983();
			}
			director->FrameMove(wnd, 0.0f);
		}
		else
			wnd->SetVisualState(VisualStateTypes::Visible983);
		break;
	case VisualStateTypes::Hiding983:
		if (director->GetAnimation(RFWnd::AnimClosing))
		{
			if (RFAnimation* animation = director->GetAnimation(RFWnd::AnimClosing))
			{
				director->_playing = animation;
				animation->Play983();
			}
		}
		else
			wnd->SetVisualState(VisualStateTypes::Hidden983);
		break;
	}
}

// 0042cef0: termina quando a cena para
void RFVisualEffectAnim::Update983(RFWnd* wnd, float delta)
{
	switch (wnd->_visualState)
	{
	case VisualStateTypes::Showing983:
		if (!wnd->_animation->_playing || !wnd->_animation->_playing->_playing983)
			wnd->SetVisualState(VisualStateTypes::Visible983);
		break;
	case VisualStateTypes::Hiding983:
		if (!wnd->_animation->_playing || !wnd->_animation->_playing->_playing983)
			wnd->SetVisualState(VisualStateTypes::Hidden983);
		break;
	}
}

// 0042cfa0: efeito pronto pelo número (os estáticos de função são protótipos que o 983 não usa)
RFVisualEffect* __stdcall CreateVisualEffect983(int type)
{
	static RFVisualEffectDefault defaultEffect;
	static RFVisualEffectFade fadeEffect;
	static RFVisualEffectAnim animEffect;
	switch (type)
	{
	case 0:
		return new RFVisualEffectDefault;
	case 1:
		return new RFVisualEffectFade;
	case 2:
		return new RFVisualEffectAnim;
	}
	return 0;
}
