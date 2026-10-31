#pragma once

#include <map>
#include <vector>
#include "frelement.h"
#include "../shared/globalgamedefine.h"

class FrWnd;
class FrArea;
class FrButton;
class FrEdit;
class FrListBox;

namespace _guild
{
	enum eFormStyle
	{
		FORM_GUILD_INFO = 1,
		FORM_GUILD_MEMBER = 2,
		FORM_GUILD_SEARCH = 4
	};

	enum eAuthority
	{
		AUTHORITY_NULL = 0x1,
		AUTHORITY_MASTER = 0x2,
		AUTHORITY_SUBMASTER = 0x4,
		AUTHORITY_MEMBER = 0x8,
		AUTHORITY_WAIT = 0x10,
		AUTHORITY_ALL = 0x1f,
		AUTHORITY_NONE = 0xffff
	};

	template <typename T>
	enumGuiType GUIConverter(T* pWnd);
	template <>
	inline enumGuiType GUIConverter(FrEdit* pWnd)
	{
		return GI_EDIT;
	}
	template <>
	inline enumGuiType GUIConverter(FrArea* pWnd)
	{
		return GI_AREA;
	}
	template <>
	inline enumGuiType GUIConverter(FrButton* pWnd)
	{
		return GI_BUTTON;
	}
	template <>
	inline enumGuiType GUIConverter(FrListBox* pWnd)
	{
		return GI_LISTBOX;
	}

	int ConvertGuildHistory(GUILD_STATE_IDX state);

	class CPageArrow
	{
	public:
		CPageArrow() { }
		~CPageArrow() { }

		void SetLeftArrow(FrButton* pButton) { m_pLeftArrow = (FrWnd*)pButton; }
		void SetRightArrow(FrButton* pButton)
		{
			m_pRightArrow = (FrWnd*)pButton;
		}
		void InitControl();
		void RefreshArrowButton(int count, int pos);

	protected:
		FrWnd* m_pLeftArrow;
		FrWnd* m_pRightArrow;

		WFlags m_flags;
	};

	class CGuildFormController
	{
	public:
		struct sPostAction
		{
			int type;
			int formStyle;
			int authority;
			FrWnd* pWnd;
			void (CGuildFormController::*pfnAction)(sPostAction* pAction);

			void operator()(CGuildFormController* pController)
			{
				(pController->*pfnAction)(this);
			}
		};

		typedef std::map<sPostAction*, sPostAction*> PostActionMap;
		typedef PostActionMap::iterator PostActionIterator;

		CGuildFormController();
		~CGuildFormController();

		void ControllForm(eFormStyle style, eAuthority authority);
		void InsertControl(FrWnd* pWnd);

		template <typename T>
		void InsertPostAction(T* pWnd, int formStyle, int authority,
			void (CGuildFormController::*pfnAction)(sPostAction* pAction))
		{
			sPostAction* pAction = new sPostAction;
			pAction->type = GUIConverter(pWnd);
			pAction->formStyle = formStyle;
			pAction->authority = authority;
			pAction->pWnd = (FrWnd*)pWnd;
			pAction->pfnAction = pfnAction;
			m_postActions.insert(std::make_pair(pAction, pAction));
		}

		void ReadOnlywindow(sPostAction* pAction);
		void WritableWindow(sPostAction* pAction);
		void EnableWindow(sPostAction* pAction);
		void DisableWindow(sPostAction* pAction);
		void MoveLeftSide(sPostAction* pAction);
		void MoveRightSide(sPostAction* pAction);
		void ClearEdit(sPostAction* pAction);
		void VisibleWindow(sPostAction* pAction);
		void InVisibleWindow(sPostAction* pAction);

	protected:
		std::vector<FrWnd*> m_controls;
		PostActionMap m_postActions;
		eFormStyle m_formStyle;
	};
}
