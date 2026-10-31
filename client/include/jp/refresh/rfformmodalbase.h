#pragma once
#include "rfbase.h"
#include "rfmodalrenderer.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"

// JP 983: base das janelas modais da RF, RFFormModalBase<T> (RTTI .?AV?$RFFormModalBase@VRFWndEx@@@@, vtables 00d0e47c / 00d0e470;
// 0x188 com T = RFWndEx): T + RFModalRenderer (+0x180, a pilha das modais e o escurecimento da tela) + "já assinou" (+0x184).
// Levantada na sessão B137 (unidade rfloadingprogressdlg). Cópias fora de linha dos moldes: DoModal 004783e0, OnDrawEvent983
// 004777c0 e OnUpdateEvent983 00477ca0 na unidade do CArinBirthDay2014Handler; ??_G 005b85e0 (+ conversão da 2ª base 005b8780),
// OnDestroy 005b8d00 e OnShow 005b8580 perto da rfcharmasterycardselectdlg. Nomes meus (…983), menos a classe e DoModal.

extern Fresh* g_pFresh; // 00f02340


template<class T> class RFFormModalBase : public T, public RFModalRenderer
{
public:
	RFFormModalBase() : m_bModalReady(false) { }
	virtual ~RFFormModalBase() { } // ??_G 005b85e0

	// 005b8d00: sem outra modal, devolve o foco ao jogo; sai da pilha das modais
	virtual void OnDestroy()
	{
		if (!RFSceneManager::GetManager983()->IsModalExist())
			RFSceneManager::Instance()->SetRFFocus983(false);
		RFSceneManager::Instance()->m_41 = false;
		RFModalRenderer::Remove983();
		T::OnDestroy();
		m_bModalReady = false;
	}
	virtual void OnShow(bool show) { } // 005b8580

	// 004783e0: na 1ª vez assina EventUpdate/EventDraw; depois prioridade -3, ShowModal, entra na pilha e pega o foco
	void DoModal()
	{
		if (!m_bModalReady)
		{
			RegisterTo(GetName983(), RFWnd::EventUpdate, Subscriber(&RFFormModalBase<T>::OnUpdateEvent983, this));
			RegisterTo(GetName983(), RFWnd::EventDraw, Subscriber(&RFFormModalBase<T>::OnDrawEvent983, this));
			m_bModalReady = true;
			SetPreDraw(false);
		}
		RFSceneManager::Instance()->SetRFFocus983(true);
		RFSceneManager::Instance()->m_41 = true;
		SetPriority(-3);
		ShowModal();
		RFModalRenderer::Push983();
		RFSceneManager::GetManager983()->FocusWindow(this);
	}

	// 00477ca0: o foco fica com a RF enquanto o mouse não está sobre uma janela do jogo (as marcadas com 0x4000 tiram o foco)
	void OnUpdateEvent983(EventArgs& e)
	{
		if (g_pFresh && g_pFresh->GetManager())
		{
			WVector* pos = RFSceneManager::Instance()->GetPos983();
			FrWnd* wnd = g_pFresh->GetManager()->FindWndAt983(pos->x, pos->y);
			if (wnd)
			{
				if (wnd->m_nFlags & FWF_UNK4000_983)
				{
					if (RFSceneManager::Instance()->HasRFFocus983())
						RFSceneManager::Instance()->SetRFFocus983(false);
				}
			}
			else if (!RFSceneManager::Instance()->HasRFFocus983())
				RFSceneManager::Instance()->SetRFFocus983(true);
		}
	}
	// 004777c0: escurece a tela atrás (só a modal de cima)
	void OnDrawEvent983(EventArgs& e)
	{
		RFModalRenderer::Draw983(GetAlpha(false));
		e.handled = false;
	}

	bool m_bModalReady; // +0x184
};
