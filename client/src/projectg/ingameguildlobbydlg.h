#pragma once

#include <list>
#include <map>
#include <string>
#include "frform.h"
#include "guildauthcontroller.h"

class Bitmap;
class FrArea;
class FrButton;
class FrEdit;
class FrListBox;
class FrContextMenuCtrl;
class WTitleFont;
struct FrListItem;

namespace _guild
{
	enum eFormStyle;
	enum eAuthority;
	class CGuildFormController;
	class CGuildInfo;
	class CPageArrow;

	class CGuildMainForm : public FrForm
	{
		friend class IGuildAuthController;
		friend class CGuildManager;
		friend class CGuildMaster;
		friend class CGuildSubMaster;
		friend class CGuildMember;
		friend class CGuildWaitMember;
		friend class CGuildNull;

	public:
		DECLARE_OBJECT(CGuildMainForm)

		struct sMemberItem
		{
			sMemberItem()
			{
				pInfo = NULL;
				bEdit = false;
			}

			const GUILD_USER_LIST* pInfo;
			bool bEdit;
		};

		typedef std::list<sMemberItem> MemberItemList;
		typedef MemberItemList::iterator MemberItemIterator;
		typedef std::list<const GUILD_LIST*> SearchItemList;
		typedef SearchItemList::iterator SearchItemIterator;

		CGuildMainForm();
		virtual ~CGuildMainForm();

		virtual bool OnInit();
		virtual void OnProc(const float dt);

		void Refresh();
		void RefreshMyGuildInfo();
		void RefreshMyGuildHistory();
		void RefreshGuildMember();
		void RefreshGuildSearch();
		void RefreshMemberPageList();
		void RefreshSearchPageList();
		void RequestMember_Page(int page);
		void RequestSearch_Page(int page);

		void SetLinkGuildInfo(CGuildInfo* pInfo) { m_pLinkGuildInfo = pInfo; }
		void SetCommandUserInfo(const GUILD_USER_LIST* pInfo)
		{
			m_pCommandUserInfo = pInfo;
		}
		void ClearCommandUserInfo() { m_pCommandUserInfo = NULL; }

		void RequestChangeClass(GUILD_CLASS_IDX classIdx);
		void ClearForm();
		void DisableJoinButton();
		void InitSearchMode();
		void LockScrollMemberList();
		void UnLockScrollMemberList();

	private:
		void InitControl();
		void LoadResource();
		void CreateAuthController();
		void PopupMemberCommand(float x, float y);
		void SelectMemberItem(FrListBox* pList, FrListItem* pItem);
		void ProcManagerCommand(const char* command);
		void CheckTalkState();
		void EnableAuthCommand(GUILD_CLASS_IDX classIdx);
		void ResetEditState(void* pData);
		void UnSelectMember();
		eAuthority ConvertGuildClass();
		bool CanExecuteClass(const GUILD_USER_LIST* pInfo);

		CGuildInfo* GuildInfo() { return m_pLinkGuildInfo; }
		CGuildFormController* Controller() { return m_pController; }

		const char* GuildMsg(int id);
		GUILD_CLASS_IDX MyGuildClassIndex();

	protected:
		void OnGuildInfo_ButtonInit(int param);
		void OnGuildInfo_LButtonUp();
		void OnGuildMember_ButtonInit(int param);
		void OnGuildMember_LButtonUp();
		void OnGuildSearch_ButtonInit(int param);
		void OnGuildSearch_LButtonUp();
		void OnGuildDetail_AreaInit(int param);
		void OnGuildDetail_OwnerDraw(int param);
		void OnGuildIntroduce_AreaInit(int param);
		void OnGuildNotice_AreaInit(int param);
		void OnGuildAct_AreaInit(int param);
		void OnEmptyGuild_AreaInit(int param);
		void OnEmptyGuild_AreaOwnerDraw(int param);
		void OnGuildMemberInfoBg_AreaInit(int param);
		void OnGuildMemberInfoBg_OwnerDraw(int param);
		void OnSearchBg1_AreaInit(int param);
		void OnSearchBg2_AreaInit(int param);
		void OnGuildIntroduce_EditInit(int param);
		void OnGuildNotice_EditInit(int param);
		void OnGuildAct_EditInit(int param);
		void OnGuildSearch_EditInit(int param);
		void OnGuildSearch_EnterKey(int param);
		void OnGuildMemberTalk_EditInit(int param);
		void OnGuildForm_CloseInit(int param);
		void OnGuildForm_CloseLButtonUp();
		void OnGuildIntroConfirm_ButtonInit(int param);
		void OnGuildIntroConfirm_LButtonUp();
		void OnGuildNoticeConfirm_ButtonInit(int param);
		void OnGuildNoticeConfirm_LButtonUp();
		void OnGuildJoin_ButtonInit(int param);
		void OnGuildJoin_LButtonUp();
		void OnGuildSearchInfo_ButtonInit(int param);
		void OnGuildSearchInfo_LButtonUp();
		void OnGuildJoinAbort_ButtonInit(int param);
		void OnGuildJoinAbort_LButtonUp();
		void OnGuildMemberTalk_ButtonInit(int param);
		void OnGuildMemberTalk_LButtonUp();
		void OnGuildSearchPrev_ButtonInit(int param);
		void OnGuildSearchPrev_LButtonUp();
		void OnGuildSearchNext_ButtonInit(int param);
		void OnGuildSearchNext_LButtonUp();
		void OnGuildShutDown_ButtonInit(int param);
		void OnGuildShutDown_LButtonUp();
		void OnGuildOut_ButtonInit(int param);
		void OnGuildOut_LButtonUp();
		void OnGuildMemberInfo_ListBoxInit(int param);
		void OnGuildMemberInfo_OwnerDraw(int param);
		void OnGuildMemberInfo_LButtonDown();
		void OnMemberPage_ListBoxInit(int param);
		void OnMemberPage_ListOwnerDraw(int param);
		void OnMemberPage_ListLButtonDown();
		void OnGuildSearchInfo_ListBoxInit(int param);
		void OnGuildSearchInfo_OwnerDraw(int param);
		void OnGuildSearchInfo_LButtonDown();
		void OnGuildSearchPage_ListBoxInit(int param);
		void OnGuildSearchPage_OwnerDraw(int param);
		void OnGuildSearchPage_LButtonDown();
		void OnGuildManagerCmd_ContextInit(int param);
		void OnGuildManagerCmd_LButtonDown();
		void OnGuildManagerCmd_LButtonUp();

		// 0 info, 1 member, 2 search
		FrButton* m_pTab[3];
		// 0 close, 1 intro confirm, 2 notice confirm, 3 join, 4 search,
		// 5 shutdown, 6 join abort, 7 out, 8 talk confirm, 9 prev, 10 next
		FrButton* m_pButton[11];
		FrArea* m_pArea[8];
		// 0 introduce, 1 notice, 2 action, 3 search, 4 talk
		FrEdit* m_pEdit[5];
		// 0 member, 1 search, 2 member page, 3 search page
		FrListBox* m_pListBox[4];
		FrContextMenuCtrl* m_pManagerCmd;
		FrListItem* m_pSelectedMember;
		const Bitmap* m_pBitmap[17];
		WTitleFont* m_pPageFont;
		WTitleFont* m_pCurrentPageFont;
		eFormStyle m_formStyle;
		CGuildFormController* m_pController;
		CGuildInfo* m_pLinkGuildInfo;
		GUILD_LIST m_selectedGuild;
		std::map<GUILD_CLASS_IDX, IGuildAuthController*> m_authControllers;
		float m_talkTextOffset;
		float m_talkIconOffset;
		MemberItemList m_memberItems;
		CPageArrow* m_pSearchArrow;
		const GUILD_USER_LIST* m_pCommandUserInfo;
		SearchItemList m_searchItems;
		int m_memberPage;
		int m_searchPage;
		int m_searchMode;
		std::string m_searchText;

	private:
		DECLARE_FRESH_MSGMAP()

	public:
		static void __cdecl
		RbOninitFunclets(); // HACK: EH funclets of the naked OnInit (nakedify)
	};

	class FrGuildJoin : public FrForm
	{
	public:
		DECLARE_OBJECT(FrGuildJoin)

		FrGuildJoin();
		virtual ~FrGuildJoin();

		virtual bool OnInit();

		void SetGuildNumber(int guildNumber);
		void SetSuccessRequest();

	protected:
		void OnGuildJoin_AreaInit(int param);
		void OnGuildJoin_AreaOwnerDraw(int param);
		void OnJoin_EditInit(int param);
		void OnRequestJoin_ButtonInit(int param);
		void OnRequestJoin_LButtonUp();
		void OnRequestOK_ButtonInit(int param);
		void OnRequestOK_LButtonUp();
		void OnFormClose_ButtonInit(int param);
		void OnFormClose_LButtonUp();

		FrArea* m_pJoinArea;
		FrEdit* m_pJoinEdit;
		FrButton* m_pButton[3];
		const Bitmap* m_pBackground;
		int m_guildNumber;
		bool m_bRequested;

	private:
		DECLARE_FRESH_MSGMAP()
	};
}
