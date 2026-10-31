#include "minatl.h"
#include "guildformcontroller.h"
#include "guildactor.h"
#include "guilddefine.h"
#include "s5/utilities.h"
#include "fredit.h"

namespace S5
{
	inline void DisableFreshWindow(FrWnd* wnd)
	{
		wnd->SetVisible(false);
		wnd->Enable(false);
	}

	inline void EnableFreshWindow(FrWnd* wnd, bool enable)
	{
		wnd->SetVisible(enable);
		wnd->Enable(enable);
	}
}

namespace _guild
{
	CGuildFormController::CGuildFormController()
		: m_formStyle((eFormStyle)0)
	{
	}

	CGuildFormController::~CGuildFormController()
	{
		std::for_each(m_postActions.begin(), m_postActions.end(),
			Delete_SecondObject());
	}

	void CGuildFormController::ControllForm(eFormStyle style,
		eAuthority authority)
	{
		m_formStyle = style;

		std::for_each(m_controls.begin(), m_controls.end(),
			S5::DisableFreshWindow);

		PostActionIterator it = m_postActions.begin();
		for (; it != m_postActions.end(); ++it)
		{
			sPostAction* pAction = (*it).second;

			if (pAction->formStyle & style)
			{
				if (pAction->authority & authority)
				{
					(*pAction)(this);
				}
			}
		}
	}

	void CGuildFormController::InsertControl(FrWnd* pWnd)
	{
		m_controls.push_back(pWnd);
	}

	void CGuildFormController::ReadOnlywindow(sPostAction* pAction)
	{
		PostActionIterator it = m_postActions.find(pAction);
		if (it == m_postActions.end())
			return;

		if ((*it).second->type == 4)
			((FrEdit*)pAction->pWnd)->SetReadOnly(true);
	}

	void CGuildFormController::WritableWindow(sPostAction* pAction)
	{
		PostActionIterator it = m_postActions.find(pAction);
		if (it == m_postActions.end())
			return;

		if ((*it).second->type == 4)
			((FrEdit*)pAction->pWnd)->SetReadOnly(false);
	}

	void CGuildFormController::EnableWindow(sPostAction* pAction)
	{
		S5::EnableFreshWindow(pAction->pWnd, true);
	}

	void CGuildFormController::DisableWindow(sPostAction* pAction)
	{
		S5::EnableFreshWindow(pAction->pWnd, false);
	}

	void CGuildFormController::MoveLeftSide(sPostAction* pAction)
	{
		FrWnd* pWnd = pAction->pWnd;

		WRect rect = pWnd->GetRect();

		rect.x = 145.0f;
		pWnd->SetRect(rect);
	}

	void CGuildFormController::MoveRightSide(sPostAction* pAction)
	{
		FrWnd* pWnd = pAction->pWnd;

		WRect rect = pWnd->GetRect();

		rect.x = 375.0f;
		pWnd->SetRect(rect);
	}

	void CGuildFormController::ClearEdit(sPostAction* pAction)
	{
		PostActionIterator it = m_postActions.find(pAction);
		if (it == m_postActions.end())
			return;

		if ((*it).second->type == 4)
			((FrEdit*)pAction->pWnd)->ClearLine();
	}

	void CGuildFormController::VisibleWindow(sPostAction* pAction)
	{
		pAction->pWnd->SetVisible(true);
	}

	void CGuildFormController::InVisibleWindow(sPostAction* pAction)
	{
		pAction->pWnd->SetVisible(false);
	}

	static const unsigned int ARROW_PAGE_SIZE = 5;

	void CPageArrow::RefreshArrowButton(int count, int pos)
	{
		InitControl();

		if (pos > ARROW_PAGE_SIZE)
			m_flags.Enable(1);

		if ((pos - 1) / ARROW_PAGE_SIZE < (count - 1) / ARROW_PAGE_SIZE)
			m_flags.Enable(2);

		if (m_flags.GetFlag(1))
			m_pLeftArrow->Enable(true);

		if (m_flags.GetFlag(2))
			m_pRightArrow->Enable(true);
	}

	void CPageArrow::InitControl()
	{
		m_flags.Reset();
		m_pLeftArrow->Enable(false);
		m_pRightArrow->Enable(false);
	}

	int ConvertGuildHistory(GUILD_STATE_IDX state)
	{
		switch (state)
		{
		case 0:
			return 0;
		case 1:
			return 0x425;
		case 2:
			return 0x426;
		case 3:
			return 0x427;
		case 4:
			return 0x428;
		case 5:
			return 0x429;
		case 6:
			return 0x42a;
		case 7:
			return 0x42b;
		case 8:
			return 0x42c;
		case 9:
			return 0x42d;
		case 10:
			return 0x42e;
		case 11:
			return 0x42f;
		case 12:
			return 0x430;
		case 13:
			return 0x431;
		case 51:
			return 0x432;
		case 52:
			return 0x433;
		case 53:
			return 0x434;
		}

		return 0;
	}
}
