#pragma once
#include <string>
#include <vector>
#include "rfscenemanager.h"

// JP 983: peças da biblioteca Refresh (RF, F3.7.13) que as janelas RF do jogo usam: eventos (EventArgs, EventFunctorBase,
// MemberFunctionFunctor<T>, Subscriber), a base RFFormBase<T> e os controles RFTextArea/RFPicture. Levantado na sessão B130
// (rfcommontooltip.cpp). Classes pela RTTI; métodos pelos nomes da RF que o mapa do 983 traz; o resto é meu (…983).

// argumentos de evento: +4 = tratado; +0xc = tecla (OnKeyDown) / botões (EventUpdate). Layout só do que o código lê.
// Com vtable (o lobby monta um vazio na pilha: vtable 00d113c8 + handled = false, 005d5701).
class EventArgs
{
public:
	EventArgs() : handled(false) { }
	virtual ~EventArgs() { }

	bool handled; // +0x4
	char m_05[3];
	union
	{
		RFWnd* m_pWnd; // +0x8: a janela do evento
		float m_time983; // +0x8: EventUpdate (tempo do quadro)
	};
	union
	{
		int m_0c; // +0xc
		float m_x983; // +0xc: eventos de mouse (posição na tela)
	};
};

// JP 983: argumentos dos eventos de mouse (posição na tela em +0xc/+0x10; nome meu)
class MouseEventArgs983 : public EventArgs
{
public:
	float m_y983; // +0x10
};

// JP 983: argumentos do EventMouseWheel (giro em +0x18: < 0 = para baixo; nome meu; RFSceneManager::InjectMouseWheel 00419180)
class MouseWheelEventArgs983 : public MouseEventArgs983
{
public:
	char m_14[4];
	float m_delta983; // +0x18
};

// functor de evento com contagem de referências (vtable 00d0e8d0…; ~EventFunctorBase 00401670)
class EventFunctorBase
{
public:
	EventFunctorBase() { } // 00401680
	virtual ~EventFunctorBase() { } // slot 0
	virtual bool operator()(EventArgs& e) = 0; // slot 1
	virtual void AddRef() = 0; // slot 2
	virtual void Release() = 0; // slot 3
	virtual int GetRefCount() = 0; // slot 4
};

// functor de método (0x10): +4 método, +8 objeto, +0xc referências. Moldes por classe (MemberFunctionFunctor<RFTooltip>
// 005ce2c0…005ce750 na rfcommontooltip.cpp)
template<class T> class MemberFunctionFunctor : public EventFunctorBase
{
public:
	typedef void (T::*Function)(EventArgs& e);

	MemberFunctionFunctor(Function fn, T* obj) : m_fn(fn), m_obj(obj), m_ref(0) { }
	virtual bool operator()(EventArgs& e) { (m_obj->*m_fn)(e); return true; }
	virtual void AddRef() { ++m_ref; }
	virtual void Release() { --m_ref; }
	virtual int GetRefCount() { return m_ref; }

	Function m_fn; // +0x4
	T* m_obj; // +0x8
	int m_ref; // +0xc
};

// assinante: guarda o functor com uma referência (o construtor de molde 005ce6f0 é a cópia fora de linha do RFTooltip)
class Subscriber
{
public:
	template<class T> Subscriber(void (T::*fn)(EventArgs& e), T* obj)
		: m_p(new MemberFunctionFunctor<T>(fn, obj))
	{
		m_p->AddRef();
	}
	~Subscriber()
	{
		if (m_p)
		{
			m_p->Release();
			if (m_p->GetRefCount() == 0 && m_p)
				delete m_p;
		}
	}

	EventFunctorBase* m_p;
};

// JP 983: base dos formulários RF não modais (vtable 00d0e3e4; construtor e destrutor embutidos; destrutor de apagar 005b85b0).
// +0x180 = já assinou o EventUpdate (o construtor não o zera).
template<class T> class RFFormBase : public T
{
public:
	RFFormBase() { }
	virtual ~RFFormBase() { } // 005ce150 (cópia fora de linha)

	// 005b8cb0: devolve o foco ao jogo se não há modal
	virtual void OnDestroy()
	{
		if (!RFSceneManager::GetManager983()->IsModalExist() && RFSceneManager::Instance()->HasRFFocus983())
			RFSceneManager::Instance()->SetRFFocus983(false);
		RFSceneManager::Instance()->m_41 = false;
		T::OnDestroy();
	}
	virtual void OnShow(bool show) { } // 005b8490

	void OnUpdateEvent983(EventArgs& e); // 005b8d60: o mouse está sobre a janela visível
	// 005ce780: na 1ª vez assina o EventUpdate com o OnUpdateEvent983; muda a visibilidade e avisa o OnShow
	void SetVisible983(bool show);
	void Show983(bool show); // 005ce880
	// 005b9cd0 (cópia na unidade rfcharmasterycardselectdlg): como o SetVisible983, mas mostra pelo RFWnd::ShowWindow
	void ShowWindow983(bool show);

	bool m_bSubscribed; // +0x180
};

// JP 983: botão da RF (RTTI .?AVRFButton@@, vtable 00d62f1c; base RFWnd). Slot novo 34 = OnStateChanged.
class RFButton : public RFWnd
{
public:
	virtual void OnStateChanged(); // 00402ab0 (slot 34, +0x88)

	// D976: _state/_pressed VC10 +0x178/+0x17c; no cl 14.44 +0x14c/+0x150 (+0x17c = tamanho do _overSound) -> pela lib
#if _MSC_VER > 1600
	void SetState(int state) { SetState983(state); }
	void SetState983(int state); // rfbutton.cpp
	int GetState983(); // rfbutton.cpp
	void ResetState983() { SetState983(0); }
#else
	int GetState983() { return _state; } // 00402aa0
	// 00402da0 (3 = desabilitado: EnableWindow(false), embutido; senao grava estado e avisa OnStateChanged)
	void SetState(int state)
	{
		if (state == 3)
			EnableWindow(false);
		else
		{
			_state = state;
			_pressed = (state == 1);
			OnStateChanged();
		}
	}
	// embutida (005fb137): volta ao estado normal
	__forceinline void ResetState983() { _state = 0; _pressed = false; OnStateChanged(); }
#endif
	void ChangeNormalButtonImage(std::wstring image); // 004218f0
	void ChangeOverButtonImage(std::wstring image); // 00421990
	void ChangeDownButtonImage(std::wstring image); // 00421a30
	void ChangeDisableButtonImage(std::wstring image); // 00421ad0

	static const std::wstring EventClicked; // 00e46d28

#if _MSC_VER <= 1600
	int _state; // +0x178 (nomes da lib desde B159; era m_state)
	bool _pressed; // +0x17c (nome da lib desde B159)
#endif
};

// JP 983: botão com rótulo da RF (RTTI .?AVRFLabelButton@@; base RFButton). Sessão B140.
class RFLabelButton : public RFButton
{
public:
	void SetLabel(const std::wstring& text); // 00426120
};

// JP 983: rótulo da RF (RTTI .?AVRFLabel@@; base RFWnd)
class RFLabel : public RFWnd
{
public:
	void SetLabel(const std::wstring& text); // 0041f830
	void SetLabel(int value, bool a2); // 0041f580 (número)
	// D877: _fontInfo.color (VC10 +0x1c8 = 0x1a4 + 0x24; no cl 14.44 o wstring do FontInfo encolhe e o campo anda) -> pela lib
#if _MSC_VER > 1600
	void SetColor983(unsigned int color); // rflabel.cpp
#else
	void SetColor983(unsigned int color) { *(unsigned int*)((char*)this + 0x1c8) = color; } // = RFLabel::SetColor 00414670
#endif
};

// JP 983: área de texto da RF (vtable 00d63eec; base RFWndEx). Slots novos 34–38.
class RFTextArea : public RFWndEx
{
public:
	virtual void SetText(const std::wstring& text); // 00427bc0 (slot 34, +0x88)
	virtual void AddText(const std::wstring& text); // 00427bf0
	virtual void AddPlainText(const std::wstring& text); // 00427c50
	virtual const std::wstring& GetText(); // 00427c90 (+0x94)
	virtual void ClearText(); // 00427d10

	void FitToContent(bool a1); // 00427d70
	int GetTextHeight(); // 00428220

	// D857: os campos abaixo só valem no VC10; o código lê pela lib (rftextarea.cpp) para valer em qualquer toolset
	int GetFontSize983(); // +0x240 (_fontInfo.size)
	void SetFontColor983(unsigned int color); // +0x248 (_fontInfo.color)
	float GetPaddingWidth983(); // m_258 - m_254 (_padding.Width())
	float GetPaddingHeight983(); // m_260 - m_25c (_padding.Height())
	char m_180[0xc0];
	int m_fontSize983; // +0x240
	int m_244;
	int m_248; // RFMasteryToolTipWnd::SetTextStyle983 005d7300 (sentido não visto)
	char m_24c[8];
	// margens do texto (RFMasteryToolTipWnd::FitSize983 005d7490: largura += m_258 − m_254, altura += m_260 − m_25c)
	float m_254;
	float m_258;
	float m_25c;
	float m_260;
};

// JP 983: imagem da RF (vtable 00d63c9c)
class RFPicture : public RFWndEx
{
public:
	void SetPicture(const std::wstring& name); // 0041ed00
	void FitToContent(); // 0041ef30
	RFSize GetTextureSize(); // 0041eef0
};

// JP 983: barra de rolagem da RF (RTTI .?AVRFScrollBar@@). Só o que as janelas da B usam; campos de 00429660…004298a0
// (nomes meus, menos os métodos com nome no mapa). Sessão B138 (rfcommoneventaboutdlg).
class RFScrollBar : public RFWnd
{
public:
	void SetScrollSize(int size); // 00429660: passo de Next/Prev
	void SetScrollRange(int min, int max); // 00429670
	void SetScrollPos(int pos); // 00429770
	int GetScrollPos() const; // 00429830 (export QBE)
	void Next(); // 00429840
	void Prev(); // 004298a0

	static const std::wstring EventPosChanged; // 00e46ecc ("PosChanged"; posição nova em EventArgs::m_0c)

	char m_178[8];
	RFWnd* m_pThumb983; // +0x180 (o "polegar"; GetSize e +0xfc/+0x100 no SetScrollRange)
	char m_184[4];
	float m_188_983; // +0x188
	int m_min983; // +0x18c
	int m_max983; // +0x190
	int m_pos983; // +0x194
	int m_size983; // +0x198
	void EnableSmoothScroll983(bool enable); // D857: m_bWheel983 pela lib (rfscrollbar.cpp; _smoothScroll)
	bool m_bWheel983; // +0x19c (o RFCommonEventAboutDlg liga antes de mostrar; sentido não visto)
};

// JP 983: lista da RF (RTTI .?AVRFList@@). Só o que as janelas da B usam (sessão B139).
class RFList : public RFWndEx
{
public:
	RFWnd* InsertWindowItem(RFWnd* wnd); // 00423fa0 (até a B161 a visão punha este nome em 00423f70, a versão por nome do XML)
	void* GetItemData(int index); // 004243a0
	// C731: m_items983 +0x180 só vale no VC10; no cl 14.44 o RFWnd da lib encolhe -> pela lib
#if _MSC_VER > 1600
	int GetItemCount983(); // rflist.cpp
#else
	int GetItemCount983() { return (int)(m_items983.size()); } // embutida (+0x180/+0x184)
#endif

	std::vector<void*> m_items983; // +0x180 (itens; só o tamanho é lido)
};

// JP 983: lista com rolagem da RF (RTTI .?AVRFListSC@@; base RFList). Sessão B142 (só o tipo do GetChild).
class RFListSC : public RFList
{
};

// JP 983: caixa de marcar da RF (RTTI .?AVRFCheck@@)
class RFCheck : public RFWndEx
{
public:
	bool GetCheck() const; // 00425c20
	void SetCheck(bool check); // 00425c30

	static const std::wstring EventChanged; // 00e46e08
};

// (fora da classe: não embutidas, como no 983)
template<class T> void RFFormBase<T>::OnUpdateEvent983(EventArgs& e)
{
	if (IsWindowVisible(false))
	{
		if (e.m_0c)
		{
			RFRect rect = GetScreenRect();
			WVector* pos = RFSceneManager::Instance()->GetPos983();
			bool over = rect.PtInRect(pos->x, pos->y);
			RFSceneManager::Instance()->m_41 = over;
		}
		e.handled = false;
	}
}

template<class T> void RFFormBase<T>::SetVisible983(bool show)
{
	if (!m_bSubscribed)
	{
		RegisterTo(GetName983(), RFWnd::EventUpdate, Subscriber(&RFFormBase<T>::OnUpdateEvent983, this));
		m_bSubscribed = true;
	}
	if (IsVisible983() != show)
	{
		SetVisibleState983(show);
		OnShow(show);
	}
}

template<class T> void RFFormBase<T>::ShowWindow983(bool show)
{
	if (!m_bSubscribed)
	{
		RegisterTo(GetName983(), RFWnd::EventUpdate, Subscriber(&RFFormBase<T>::OnUpdateEvent983, this));
		m_bSubscribed = true;
	}
	ShowWindow(show);
}

template<class T> void RFFormBase<T>::Show983(bool show)
{
	if (show)
	{
		Activate();
		SetVisible983(true);
	}
	else
	{
		Deactivate();
		SetVisible983(false);
	}
}
