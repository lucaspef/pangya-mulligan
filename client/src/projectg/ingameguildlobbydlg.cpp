#define REBANG_GUILDINFO_INLINES // see guildinfo.h
#include "minatl.h"
#include "ingameguildlobbydlg.h"
#include "guildformcontroller.h"
#include "guildinfo.h"

extern Fresh* g_pFresh;

#include "actor.h"
#include "guildactor.h"
#include "guilddefine.h"
#include "s5/utilities.h"
#include "netresourcemanager.h"
#include "frcontextmenuctrl.h"
#include "frscrollbar.h"
#include "inputmanager.h"
#include "mousecursor.h"
#include "commonutil.h"
#include "wresrcmng.h"
#include "wtemplate.h"


extern WResourceManager* g_resrcmng;
extern WView* g_view;

// HACK: 96.0f, 107.0f and 89.0f are function-local statics of autotarget.cpp /
// tutorialmain.cpp aliased to their __real@ names; using the literals here
// would emit competing COMDATs (see giftalarmdlg.cpp).

static bool IsZero(float f)
{
	return f < g_EPSILON;
}

namespace _guild
{

	IMPLEMENT_OBJECT(CGuildMainForm, FrForm)

	BEGIN_FRESH_MSGMAP(CGuildMainForm, FrForm)

	ON_FRESH_VI("set_info", FRCMD_INIT, CGuildMainForm::OnGuildInfo_ButtonInit)
	ON_FRESH_VV("set_info", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildInfo_LButtonUp)
	ON_FRESH_VI("set_member", FRCMD_INIT,
		CGuildMainForm::OnGuildMember_ButtonInit)
	ON_FRESH_VV("set_member", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildMember_LButtonUp)
	ON_FRESH_VI("set_search", FRCMD_INIT,
		CGuildMainForm::OnGuildSearch_ButtonInit)
	ON_FRESH_VV("set_search", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildSearch_LButtonUp)
	ON_FRESH_VI("guild_info_detail", FRCMD_INIT,
		CGuildMainForm::OnGuildDetail_AreaInit)
	ON_FRESH_VI("guild_info_detail", FRCMD_OWNERDRAW,
		CGuildMainForm::OnGuildDetail_OwnerDraw)
	ON_FRESH_VI("guild_info_introduce", FRCMD_INIT,
		CGuildMainForm::OnGuildIntroduce_AreaInit)
	ON_FRESH_VI("guild_info_notice", FRCMD_INIT,
		CGuildMainForm::OnGuildNotice_AreaInit)
	ON_FRESH_VI("guild_info_action", FRCMD_INIT,
		CGuildMainForm::OnGuildAct_AreaInit)
	ON_FRESH_VI("guild_info_empty", FRCMD_INIT,
		CGuildMainForm::OnEmptyGuild_AreaInit)
	ON_FRESH_VI("guild_info_empty", FRCMD_OWNERDRAW,
		CGuildMainForm::OnEmptyGuild_AreaOwnerDraw)
	ON_FRESH_VI("member_info_bg", FRCMD_INIT,
		CGuildMainForm::OnGuildMemberInfoBg_AreaInit)
	ON_FRESH_VI("member_info_bg", FRCMD_OWNERDRAW,
		CGuildMainForm::OnGuildMemberInfoBg_OwnerDraw)
	ON_FRESH_VI("search_bg1", FRCMD_INIT, CGuildMainForm::OnSearchBg1_AreaInit)
	ON_FRESH_VI("search_bg2", FRCMD_INIT, CGuildMainForm::OnSearchBg2_AreaInit)
	ON_FRESH_VI("guild_intro_edit", FRCMD_INIT,
		CGuildMainForm::OnGuildIntroduce_EditInit)
	ON_FRESH_VI("guild_notice_edit", FRCMD_INIT,
		CGuildMainForm::OnGuildNotice_EditInit)
	ON_FRESH_VI("guild_action_edit", FRCMD_INIT,
		CGuildMainForm::OnGuildAct_EditInit)
	ON_FRESH_VI("guild_search_edit", FRCMD_INIT,
		CGuildMainForm::OnGuildSearch_EditInit)
	ON_FRESH_VI("guild_search_edit", FRCMD_ENTERKEY,
		CGuildMainForm::OnGuildSearch_EnterKey)
	ON_FRESH_VI("guild_talk_edit", FRCMD_INIT,
		CGuildMainForm::OnGuildMemberTalk_EditInit)
	ON_FRESH_VI("form_close", FRCMD_INIT, CGuildMainForm::OnGuildForm_CloseInit)
	ON_FRESH_VV("form_close", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildForm_CloseLButtonUp)
	ON_FRESH_VI("guild_intro_confirm", FRCMD_INIT,
		CGuildMainForm::OnGuildIntroConfirm_ButtonInit)
	ON_FRESH_VV("guild_intro_confirm", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildIntroConfirm_LButtonUp)
	ON_FRESH_VI("guild_notice_confirm", FRCMD_INIT,
		CGuildMainForm::OnGuildNoticeConfirm_ButtonInit)
	ON_FRESH_VV("guild_notice_confirm", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildNoticeConfirm_LButtonUp)
	ON_FRESH_VI("guild_search_btn", FRCMD_INIT,
		CGuildMainForm::OnGuildSearchInfo_ButtonInit)
	ON_FRESH_VV("guild_search_btn", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildSearchInfo_LButtonUp)
	ON_FRESH_VI("guild_join", FRCMD_INIT,
		CGuildMainForm::OnGuildJoin_ButtonInit)
	ON_FRESH_VV("guild_join", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildJoin_LButtonUp)
	ON_FRESH_VI("guild_shutdown", FRCMD_INIT,
		CGuildMainForm::OnGuildShutDown_ButtonInit)
	ON_FRESH_VV("guild_shutdown", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildShutDown_LButtonUp)
	ON_FRESH_VI("guild_out", FRCMD_INIT, CGuildMainForm::OnGuildOut_ButtonInit)
	ON_FRESH_VV("guild_out", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildOut_LButtonUp)
	ON_FRESH_VI("join_abort", FRCMD_INIT,
		CGuildMainForm::OnGuildJoinAbort_ButtonInit)
	ON_FRESH_VV("join_abort", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildJoinAbort_LButtonUp)
	ON_FRESH_VI("talk_confirm", FRCMD_INIT,
		CGuildMainForm::OnGuildMemberTalk_ButtonInit)
	ON_FRESH_VV("talk_confirm", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildMemberTalk_LButtonUp)
	ON_FRESH_VI("search_prev", FRCMD_INIT,
		CGuildMainForm::OnGuildSearchPrev_ButtonInit)
	ON_FRESH_VV("search_prev", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildSearchPrev_LButtonUp)
	ON_FRESH_VI("search_next", FRCMD_INIT,
		CGuildMainForm::OnGuildSearchNext_ButtonInit)
	ON_FRESH_VV("search_next", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildSearchNext_LButtonUp)
	ON_FRESH_VI("guild_member_list", FRCMD_INIT,
		CGuildMainForm::OnGuildMemberInfo_ListBoxInit)
	ON_FRESH_VI("guild_member_list", FRCMD_OWNERDRAW,
		CGuildMainForm::OnGuildMemberInfo_OwnerDraw)
	ON_FRESH_VV("guild_member_list", FRCMD_LBUTTONUP,
		CGuildMainForm::OnGuildMemberInfo_LButtonDown)
	ON_FRESH_VI("member_page_listbox", FRCMD_INIT,
		CGuildMainForm::OnMemberPage_ListBoxInit)
	ON_FRESH_VI("member_page_listbox", FRCMD_OWNERDRAW,
		CGuildMainForm::OnMemberPage_ListOwnerDraw)
	ON_FRESH_VV("member_page_listbox", FRCMD_LBUTTONDOWN,
		CGuildMainForm::OnMemberPage_ListLButtonDown)
	ON_FRESH_VI("guild_search_list", FRCMD_INIT,
		CGuildMainForm::OnGuildSearchInfo_ListBoxInit)
	ON_FRESH_VI("guild_search_list", FRCMD_OWNERDRAW,
		CGuildMainForm::OnGuildSearchInfo_OwnerDraw)
	ON_FRESH_VV("guild_search_list", FRCMD_LBUTTONDOWN,
		CGuildMainForm::OnGuildSearchInfo_LButtonDown)
	ON_FRESH_VI("search_page_listbox", FRCMD_INIT,
		CGuildMainForm::OnGuildSearchPage_ListBoxInit)
	ON_FRESH_VI("search_page_listbox", FRCMD_OWNERDRAW,
		CGuildMainForm::OnGuildSearchPage_OwnerDraw)
	ON_FRESH_VV("search_page_listbox", FRCMD_LBUTTONDOWN,
		CGuildMainForm::OnGuildSearchPage_LButtonDown)
	ON_FRESH_VI("cmd_rmenu", FRCMD_INIT,
		CGuildMainForm::OnGuildManagerCmd_ContextInit)
	ON_FRESH_VV("cmd_rmenu", FRCMD_LBUTTONDOWN,
		CGuildMainForm::OnGuildManagerCmd_LButtonDown)
	ON_FRESH_VV("cmd_rmenu", FRCMD_RBUTTONDOWN,
		CGuildMainForm::OnGuildManagerCmd_LButtonUp)

	END_FRESH_MSGMAP()

	CGuildMainForm::CGuildMainForm()
	{
		InitControl();
	}

	CGuildMainForm::~CGuildMainForm()
	{
		if (g_resrcmng && m_pCurrentPageFont)
		{
			g_resrcmng->Release(m_pCurrentPageFont);
			m_pCurrentPageFont = NULL;
		}
		if (g_resrcmng && m_pPageFont)
		{
			g_resrcmng->Release(m_pPageFont);
			m_pPageFont = NULL;
		}
		std::for_each(m_authControllers.begin(), m_authControllers.end(),
			Delete_SecondObject());
		if (m_pSearchArrow)
		{
			delete m_pSearchArrow;
			m_pSearchArrow = NULL;
		}
		if (m_pController)
		{
			delete m_pController;
			m_pController = NULL;
		}
	}

	bool CGuildMainForm::OnInit()
	{
		static char* s_guildNormalFont[] = { "guild_normal_font.tga" };
		static char* s_guildOverFont[] = { "guild_over_font.tga" };

		LoadResource();

		WTITLEFONT info;
		memset(&info, 0, sizeof(info));
		info.filename = s_guildNormalFont;
		info.fontw = 12;
		info.fonth = 14;
		info.numPages = 1;
		info.texw = 120;
		info.texh = 14;
		info.pCharSet = "0123456789";
		m_pPageFont = g_resrcmng->GetTitleFont();
		m_pPageFont->Create(&info);

		memset(&info, 0, sizeof(info));
		info.filename = s_guildOverFont;
		info.fontw = 13;
		info.fonth = 16;
		info.numPages = 1;
		info.texw = 130;
		info.texh = 16;
		info.pCharSet = "0123456789";
		m_pCurrentPageFont = g_resrcmng->GetTitleFont();
		m_pCurrentPageFont->Create(&info);

		m_formStyle = FORM_GUILD_INFO;
		Controller()->ControllForm(m_formStyle, ConvertGuildClass());
		m_pTab[0]->SetStatus(FrButton::PRESSED);

		CreateAuthController();

		const WRect& listRect = m_pListBox[0]->GetRect();
		m_talkTextOffset = listRect.w * 0.5f + 6.0f;
		m_talkIconOffset = listRect.w - m_pButton[8]->GetRect().w * 1.4f - 1.0f;

		int guildUID = GuildInfo()->GetMyGuildInfo().guildUID;
		if (guildUID != 0)
		{
			WSendPacketSem983 packet((enumClientPacket)257); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(guildUID);
			packet.Send(TO_GAME);
		}

		{
			WSendPacketSem983 packet((enumClientPacket)261); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(1);
			packet.Send(TO_GAME);
		}

		return FrForm::OnInit();
	}


	void CGuildMainForm::OnProc(const float dt)
	{
		if (m_formStyle != FORM_GUILD_MEMBER)
			return;

		if (g_input->GetButton(LEFT_BUTTON) || g_input->GetButton(RIGHT_BUTTON))
		{
			if (CMouseCursor::Instance()->InArea(m_pListBox[0]->GetRect()))
				return;

			if (m_pManagerCmd)
				m_pManagerCmd->SetVisible(false);
		}
	}

	void CGuildMainForm::Refresh()
	{
		switch (m_formStyle)
		{
		case FORM_GUILD_INFO:
			RefreshMyGuildInfo();
			RefreshMyGuildHistory();
			break;
		case FORM_GUILD_MEMBER:
			RefreshGuildMember();
			break;
		case FORM_GUILD_SEARCH:
			RefreshGuildSearch();
			break;
		}
	}

	void CGuildMainForm::RefreshMyGuildInfo()
	{
		const GUILD_INFO& info = GuildInfo()->GetMyGuildInfo();

		if (info.classIdx == 9)
			Controller()->ControllForm(FORM_GUILD_INFO, AUTHORITY_WAIT);
		else if (info.classIdx == 3)
			Controller()->ControllForm(FORM_GUILD_INFO, AUTHORITY_MEMBER);

		m_pEdit[0]->ClearLine();
		m_pEdit[0]->AddText(info.introduce, false, true);
		m_pEdit[0]->ResetKeyFocus();
		m_pEdit[0]->GetScrollBar()->ScrollToFirst();

		m_pEdit[1]->ClearLine();
		m_pEdit[1]->AddText(info.notice, false, true);
		m_pEdit[1]->GetScrollBar()->ScrollToFirst();

		m_pEdit[2]->ClearLine();
	}

	void CGuildMainForm::RefreshMyGuildHistory()
	{
		CGuildInfo* pGuildInfo = GuildInfo();

		std::string history;

		std::list<GUILD_HISTORY>::const_iterator it =
			pGuildInfo->GetHistoryList().begin();
		for (; it != pGuildInfo->GetHistoryList().end(); ++it)
		{
			char line[256];
			memset(line, 0, sizeof(line));

			sprintf(line, s_guildJoinDateFormat, (*it).time.wYear,
				(*it).time.wMonth, (*it).time.wDay,
				GuildInfo()->GuildMsg(ConvertGuildHistory((*it).state)));

			history += line;
		}

		m_pEdit[2]->AddText(history.c_str(), false, true);
	}

	void CGuildMainForm::RefreshGuildMember()
	{
		if (m_pListBox[0]->GetCurrentItemSize(true) > 0)
			m_pListBox[0]->ClearItem();

		if (!m_memberItems.empty())
			m_memberItems.clear();

		m_pSelectedMember = NULL;
		const std::list<GUILD_USER_LIST>* pMemberList =
			GuildInfo()->GetMemberList(m_memberPage);
		if (!pMemberList)
			return;

		std::list<GUILD_USER_LIST>::const_iterator it = pMemberList->begin();
		for (; it != pMemberList->end(); ++it)
		{
			sMemberItem item;
			item.pInfo = &*it;
			m_memberItems.push_back(item);
		}

		std::list<sMemberItem>::iterator item = m_memberItems.begin();
		for (; item != m_memberItems.end(); ++item)
			m_pListBox[0]->AddItem(&*item);

		RefreshMemberPageList();

		ClearCommandUserInfo();
		if (m_pManagerCmd)
			m_pManagerCmd->SetVisible(false);
	}

	void CGuildMainForm::RefreshGuildSearch()
	{
		if (m_pListBox[1]->GetCurrentItemSize(true) > 0)
			m_pListBox[1]->ClearItem();

		if (!m_searchItems.empty())
			m_searchItems.clear();

		const std::list<GUILD_LIST>* pSearchList =
			GuildInfo()->GetSearchList(m_searchPage);
		if (!pSearchList)
			return;

		std::list<GUILD_LIST>::const_iterator it = pSearchList->begin();
		for (; it != pSearchList->end(); ++it)
			m_searchItems.push_back(&*it);

		std::list<const GUILD_LIST*>::iterator item = m_searchItems.begin();
		for (; item != m_searchItems.end(); ++item)
			m_pListBox[1]->AddItem((void*)*item);

		RefreshSearchPageList();
	}

	static const unsigned int s_pageGroupSize = 5;

	void CGuildMainForm::RefreshMemberPageList()
	{
		m_pListBox[2]->ClearItem();
		int maxPage = GuildInfo()->GetMaxMemberPage();

		for (int i = 0; i < maxPage; i++)
			m_pListBox[2]->AddItem(NULL);

		FrListBox* pList = m_pListBox[2];
		int count = pList->GetCurrentItemSize(true);
		int width = m_pListBox[2]->GetItemWidth();

		float center = (m_rect.w + m_rect.x) * 0.5f;

		WRect rect = pList->GetRect();
		rect.x = center - (float)width * (float)count + (float)count * 10.0f;
		m_pListBox[2]->SetRect(rect);
	}

	void CGuildMainForm::RefreshSearchPageList()
	{
		m_pListBox[3]->ClearItem();
		int maxPage = GuildInfo()->GetMaxSearchPage();

		m_pListBox[1]->GetCurrentItemSize(true);

		unsigned int group = (m_searchPage - 1) / s_pageGroupSize;
		int first = group * s_pageGroupSize;

		int last = 0;
		if (m_searchPage < maxPage)
		{
			if (group == (maxPage - 1) / s_pageGroupSize)
				last = maxPage;
			else
				last = first + s_pageGroupSize;
		}
		else if (m_searchPage == maxPage)
		{
			last = m_searchPage;
		}

		for (int i = first; i < last; i++)
			m_pListBox[3]->AddItem((void*)i);

		m_pSearchArrow->RefreshArrowButton(maxPage, m_searchPage);

		int count = m_pListBox[3]->GetCurrentItemSize(true);
		const WRect& bg = m_pArea[7]->GetRect();
		float center = bg.w * 0.5f + bg.x;
		float x =
			center - (float)m_pListBox[3]->GetItemWidth() * count * 0.5f + 6.0f;

		WRect rect = m_pListBox[3]->GetRect();
		rect.x = x;
		m_pListBox[3]->SetRect(rect);
	}

	void CGuildMainForm::RequestMember_Page(int page)
	{
		GUILD_CLASS_IDX classIdx = MyGuildClassIndex();
		if (classIdx == 0 || classIdx == 9)
			return;

		WSendPacketSem983 packet((enumClientPacket)271); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		unsigned long guildUID = GuildInfo()->GetMyGuildInfo().guildUID;
		packet.Encode4(guildUID);
		packet.Encode4(page != 0 ? page : m_memberPage);
		packet.Send(TO_GAME);
	}

	void CGuildMainForm::RequestSearch_Page(int page)
	{
		if (m_searchMode == 0)
		{
			m_searchPage = page;

			WSendPacketSem983 packet((enumClientPacket)261); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(page != 0 ? page : 1);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		else if (m_searchMode == 1)
		{
			m_searchPage = page;

			WSendPacketSem983 packet((enumClientPacket)262); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(page);
			packet.EncodeStr(m_searchText);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
	}

	void CGuildMainForm::RequestChangeClass(GUILD_CLASS_IDX classIdx)
	{
		if (!m_pCommandUserInfo)
			return;

		WSendPacketSem983 packet((enumClientPacket)269); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(GuildInfo()->GetMyGuildInfo().guildUID);
		packet.Encode4(m_pCommandUserInfo->userUID);
		packet.Encode4(classIdx);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}

	void CGuildMainForm::ClearForm()
	{
		m_pEdit[0]->ClearLine();
		m_pEdit[1]->ClearLine();
		m_pEdit[2]->ClearLine();
		m_pListBox[0]->ClearItem();
		m_memberItems.clear();

		Controller()->ControllForm(FORM_GUILD_INFO, ConvertGuildClass());
		m_formStyle = FORM_GUILD_INFO;

		m_pTab[0]->SetStatus(FrButton::PRESSED);
		m_pTab[1]->SetStatus(FrButton::NORMAL);
		m_pTab[2]->SetStatus(FrButton::NORMAL);
	}

	void CGuildMainForm::DisableJoinButton()
	{
		FrButton* pButton = m_pButton[3];
		if (pButton)
		{
			pButton->SetVisible(false);
			pButton->Enable(false);
		}
	}

	void CGuildMainForm::InitSearchMode()
	{
		m_searchMode = 0;
	}

	void CGuildMainForm::LockScrollMemberList()
	{
		if (m_pListBox[0])
			m_pListBox[0]->EnableScrollBar(false);
	}

	void CGuildMainForm::UnLockScrollMemberList()
	{
		FrListBox* pList = m_pListBox[0];
		if (pList)
		{
			if (pList->GetSelected())
				pList->EnableScrollBar(true);
		}
	}

	void CGuildMainForm::InitControl()
	{
		memset(m_pTab, 0, sizeof(m_pTab));
		memset(m_pArea, 0, sizeof(m_pArea));
		memset(m_pEdit, 0, sizeof(m_pEdit));
		memset(m_pButton, 0, sizeof(m_pButton));
		memset(m_pListBox, 0, sizeof(m_pListBox));
		memset(m_pBitmap, 0, sizeof(m_pBitmap));
		memset(&m_selectedGuild, 0, sizeof(m_selectedGuild));

		m_pSelectedMember = NULL;
		m_pController = _util::NewCreator<CGuildFormController>::CreateObject();
		m_pSearchArrow = _util::NewCreator<CPageArrow>::CreateObject();

		m_pPageFont = NULL;
		m_pCurrentPageFont = NULL;
		m_pLinkGuildInfo = NULL;

		m_formStyle = (eFormStyle)0;

		m_talkTextOffset = 0.0f;
		m_talkIconOffset = 0.0f;
		m_pCommandUserInfo = NULL;

		m_memberPage = 1;
		m_searchPage = 1;

		InitSearchMode();
	}

	void CGuildMainForm::LoadResource()
	{
		const char* names[17] = { "guild_con_member_bg01",
			"guild_con_member_bg02", "guild_con_member_bg03",
			"guild_con_member_bg04", "guild_icon_manager",
			"guild_icon_assistant", "guild_icon_member", "guild_icon_waiting",
			"guild_not_join_bg1", "guild_not_join_bg2", "guild_wating_bg",
			"guild_member_on", "guild_member_off", "guild_select_line_o",
			"guild_select_line_d", "guild_btn_modification_n",
			"guild_btn_ok_n" };

		for (int i = 0; i < 17; i++)
			m_pBitmap[i] = g_pFresh->RegisterBitmap(names[i]);
	}

	void CGuildMainForm::CreateAuthController()
	{
		m_authControllers.insert(std::make_pair((GUILD_CLASS_IDX)1,
			_util::NewCreator<CGuildMaster>::CreateObject()));
		m_authControllers.insert(std::make_pair((GUILD_CLASS_IDX)2,
			_util::NewCreator<CGuildSubMaster>::CreateObject()));
		m_authControllers.insert(std::make_pair((GUILD_CLASS_IDX)3,
			_util::NewCreator<CGuildMember>::CreateObject()));
		m_authControllers.insert(std::make_pair((GUILD_CLASS_IDX)9,
			_util::NewCreator<CGuildWaitMember>::CreateObject()));
		m_authControllers.insert(std::make_pair((GUILD_CLASS_IDX)0,
			_util::NewCreator<CGuildNull>::CreateObject()));

		m_authControllers[(GUILD_CLASS_IDX)1]->SetLinkDepends(this,
			m_pLinkGuildInfo);
	}

	void CGuildMainForm::PopupMemberCommand(float x, float y)
	{
		if (!m_pSelectedMember)
			return;

		sMemberItem* pItem = (sMemberItem*)m_pSelectedMember->pData;

		ClearCommandUserInfo();
		SetCommandUserInfo(pItem->pInfo);
		m_authControllers[MyGuildClassIndex()]->PopupCommandMenu(x, y);
		EnableAuthCommand(MyGuildClassIndex());
	}

	void CGuildMainForm::SelectMemberItem(FrListBox* pList, FrListItem* pItem)
	{
		if (m_pSelectedMember)
		{
			if (((sMemberItem*)m_pSelectedMember->pData)->bEdit == true)
			{
				m_pEdit[4]->ClearLine();
				FrWnd* pEdit = m_pEdit[4];
				pEdit->SetVisible(false);
				pEdit->Enable(false);
				FrWnd* pButton = m_pButton[8];
				pButton->SetVisible(false);
				pButton->Enable(false);
				pList->OrderItem(
					_util::MakeMemFun1(*this, &CGuildMainForm::ResetEditState));
			}
		}

		pList->UnselectItem(m_pSelectedMember);
		pList->SelectItem(pItem, true);
		m_pSelectedMember = pItem;
	}

	void CGuildMainForm::ProcManagerCommand(const char* command)
	{
		if (strlen(command) < 1)
			return;

		m_authControllers[MyGuildClassIndex()]->ProcManagerCommand(command);
	}

	void CGuildMainForm::CheckTalkState()
	{
		FrWnd* pButton = m_pButton[8];
		pButton->SetVisible(true);
		pButton->Enable(true);
		FrWnd* pEdit = m_pEdit[4];
		pEdit->SetVisible(true);
		pEdit->Enable(true);

		if (!m_pSelectedMember)
		{
			m_pSelectedMember = m_pListBox[0]->GetSelected();
			if (!m_pSelectedMember)
				return;
		}

		const WPoint& pos = m_pSelectedMember->pos;

		WRect rect(0.0f, pos.y + 5.0f, 0.0f, m_pListBox[0]->GetItemHeight());

		rect.x = m_talkTextOffset + pos.x;
		rect.w = m_pEdit[4]->GetRect().w;
		m_pEdit[4]->SetRect(rect);

		rect.x = m_talkIconOffset + pos.x;
		rect.w = m_pButton[8]->GetRect().w;
		rect.h = m_pButton[8]->GetRect().h;
		m_pButton[8]->SetRect(rect);

		sMemberItem* pMember = (sMemberItem*)m_pSelectedMember->pData;

		int length = strlen(pMember->pInfo->message);
		if (length > 0)
		{
			if (length > 25)
				return;

			m_pEdit[4]->ClearLine();
			m_pEdit[4]->ForcedEdit(pMember->pInfo->message, 0);
		}

		LockScrollMemberList();
	}

	void CGuildMainForm::EnableAuthCommand(GUILD_CLASS_IDX classIdx)
	{
		int targetClass =
			((sMemberItem*)m_pSelectedMember->pData)->pInfo->classIdx;

		switch (classIdx)
		{
		case 1:
			if (targetClass == 1)
				break;

			switch (targetClass)
			{
			case 2:
				m_pManagerCmd->UnableMenuItem(GuildMsg(1081));
				break;

			case 3:
			case 9:
				m_pManagerCmd->UnableMenuItem(GuildMsg(1082));
				break;
			}
			break;

		case 2:
			switch (targetClass)
			{
			case 1:
			case 2:
				m_pManagerCmd->UnableMenuItem(GuildMsg(1081));
				m_pManagerCmd->UnableMenuItem(GuildMsg(1082));
				m_pManagerCmd->UnableMenuItem(GuildMsg(1083));
				break;

			case 3:
			case 9:
				m_pManagerCmd->UnableMenuItem(GuildMsg(1082));
				break;
			}
			break;
		}
	}

	void CGuildMainForm::ResetEditState(void* pData)
	{
		sMemberItem* pItem = (sMemberItem*)pData;
		pItem->bEdit = false;
	}

	void CGuildMainForm::UnSelectMember()
	{
		FrListItem* pItem = m_pListBox[0]->GetSelected();
		if (!pItem)
			return;

		((sMemberItem*)pItem->pData)->bEdit = false;
		UnLockScrollMemberList();
		m_pListBox[0]->UnselectItem(pItem);
	}

	eAuthority CGuildMainForm::ConvertGuildClass()
	{
		eAuthority authority = AUTHORITY_NONE;

		switch (GuildInfo()->GetMyGuildInfo().classIdx)
		{
		case 0:
			authority = AUTHORITY_NULL;
			break;
		case 1:
			authority = AUTHORITY_MASTER;
			break;
		case 2:
			authority = AUTHORITY_SUBMASTER;
			break;
		case 3:
			authority = AUTHORITY_MEMBER;
			break;
		case 9:
			authority = AUTHORITY_WAIT;
			break;
		}

		return authority;
	}

	bool CGuildMainForm::CanExecuteClass(const GUILD_USER_LIST* pInfo)
	{
		switch (MyGuildClassIndex())
		{
		case 1:
		case 2:
			return true;
		case 3:
			if (pInfo->userUID == MyUID())
				return true;
			break;
		}

		return false;
	}

	const char* CGuildMainForm::GuildMsg(int id)
	{
		return m_pLinkGuildInfo->GuildMsg(id);
	}

	GUILD_CLASS_IDX CGuildMainForm::MyGuildClassIndex()
	{
		return m_pLinkGuildInfo->GetClassIndex();
	}

	void CGuildMainForm::OnGuildInfo_ButtonInit(int param)
	{
		m_pTab[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void CGuildMainForm::OnGuildInfo_LButtonUp()
	{
		if (m_formStyle == FORM_GUILD_INFO)
			return;

		m_pEdit[0]->ClearLine();
		m_pEdit[0]->AddText(GuildInfo()->GetMyGuildInfo().introduce, false,
			true);
		m_pEdit[0]->GetScrollBar()->ScrollToFirst();

		Controller()->ControllForm(FORM_GUILD_INFO, ConvertGuildClass());
		m_formStyle = FORM_GUILD_INFO;

		if (m_pTab[1]->GetStatus() == FrButton::PRESSED)
			UnSelectMember();

		m_pTab[0]->SetStatus(FrButton::PRESSED);
		m_pTab[1]->SetStatus(FrButton::NORMAL);
		m_pTab[2]->SetStatus(FrButton::NORMAL);
	}

	void CGuildMainForm::OnGuildMember_ButtonInit(int param)
	{
		m_pTab[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void CGuildMainForm::OnGuildMember_LButtonUp()
	{
		if (m_formStyle == FORM_GUILD_MEMBER)
			return;

		Controller()->ControllForm(FORM_GUILD_MEMBER, ConvertGuildClass());
		m_formStyle = FORM_GUILD_MEMBER;

		m_pTab[0]->SetStatus(FrButton::NORMAL);
		m_pTab[1]->SetStatus(FrButton::PRESSED);
		m_pTab[2]->SetStatus(FrButton::NORMAL);
	}

	void CGuildMainForm::OnGuildSearch_ButtonInit(int param)
	{
		m_pTab[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void CGuildMainForm::OnGuildSearch_LButtonUp()
	{
		if (m_formStyle == FORM_GUILD_SEARCH)
			return;

		Controller()->ControllForm(FORM_GUILD_SEARCH, ConvertGuildClass());
		m_formStyle = FORM_GUILD_SEARCH;

		int maxPage = GuildInfo()->GetMaxSearchPage();
		m_pSearchArrow->RefreshArrowButton(maxPage, m_searchPage);

		if (m_pTab[1]->GetStatus() == FrButton::PRESSED)
			UnSelectMember();

		m_pTab[0]->SetStatus(FrButton::NORMAL);
		m_pTab[1]->SetStatus(FrButton::NORMAL);
		m_pTab[2]->SetStatus(FrButton::PRESSED);
	}

	void CGuildMainForm::OnGuildDetail_AreaInit(int param)
	{
		m_pArea[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[0]);
		Controller()->InsertPostAction(m_pArea[0], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pArea[0], FORM_GUILD_INFO,
			AUTHORITY_ALL, &CGuildFormController::MoveLeftSide);
		Controller()->InsertPostAction(m_pArea[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pArea[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::MoveRightSide);
	}

	void CGuildMainForm::OnGuildDetail_OwnerDraw(int param)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		const WRect& rect = m_pArea[0]->GetRect();
		float x = rect.x + 86.0f;
		float y = rect.y + 14.0f;
		static const float s_detailLineHeight = 28.0f;

		if (m_formStyle == FORM_GUILD_INFO)
		{
			if (GuildInfo()->IsGuildMember() == true)
			{
				pGDI->SetTextStyle(1);
				pGDI->SetTextColor(0xff33365f, 0xffffffff);

				const GUILD_INFO& info = GuildInfo()->GetMyGuildInfo();
				const Bitmap* pEmblem =
					NetResourceManager::Instance()->GetEmblemByName(
						info.guildMark);
				if (pEmblem)
				{
					pGDI->DrawTexture(pEmblem,
						WRect(x + 99.0f, rect.y + 3.0f, (float)pEmblem->Width(),
							(float)pEmblem->Height()),
						0xffffffff, 0);
				}

				pGDI->Print11(WPoint(x, y), 0, info.guildName);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, info.masterNickname);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, "%d \xb8\xed", info.memberCount);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, "%d \xc6\xce", info.guildPang);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0,
					"- \xb7\xb9\xba\xa7(%d \xc1\xa1)", info.guildPoint);
			}
		}
		else if (m_formStyle == FORM_GUILD_SEARCH)
		{
			if (m_selectedGuild.guildUID != 0)
			{
				pGDI->SetTextStyle(1);
				pGDI->SetTextColor(0xff33365f, 0xffffffff);

				const Bitmap* pEmblem =
					NetResourceManager::Instance()->GetEmblemByName(
						m_selectedGuild.guildMark);
				if (pEmblem)
				{
					pGDI->DrawTexture(pEmblem,
						WRect(x + 99.0f, rect.y + 3.0f, (float)pEmblem->Width(),
							(float)pEmblem->Height()),
						0xffffffff, 0);
				}

				pGDI->Print11(WPoint(x, y), 0, m_selectedGuild.guildName);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, m_selectedGuild.masterNickname);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, "%d \xb8\xed",
					m_selectedGuild.memberCount);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0, "%d \xc6\xce",
					m_selectedGuild.guildPang);
				y += s_detailLineHeight;
				pGDI->Print11(WPoint(x, y), 0,
					"- \xb7\xb9\xba\xa7(%d \xc1\xa1)",
					m_selectedGuild.guildPoint);
			}
			else
				return;
		}

		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(0, 0xffffffff);
	}

	void CGuildMainForm::OnGuildIntroduce_AreaInit(int param)
	{
		m_pArea[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[1]);
		Controller()->InsertPostAction(m_pArea[1], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pArea[1], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::MoveLeftSide);
		Controller()->InsertPostAction(m_pArea[1], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pArea[1], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::MoveRightSide);
	}

	void CGuildMainForm::OnGuildNotice_AreaInit(int param)
	{
		m_pArea[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[2]);
		Controller()->InsertPostAction(m_pArea[2], FORM_GUILD_INFO,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildAct_AreaInit(int param)
	{
		m_pArea[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[3]);
		Controller()->InsertPostAction(m_pArea[3], FORM_GUILD_INFO,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnEmptyGuild_AreaInit(int param)
	{
		m_pArea[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[4]);
		Controller()->InsertPostAction(m_pArea[4], FORM_GUILD_INFO,
			AUTHORITY_NULL | AUTHORITY_WAIT,
			&CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnEmptyGuild_AreaOwnerDraw(int param)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		if (GuildInfo()->IsWaitingJoin())
		{
			const WRect& rect = m_pArea[4]->GetRect();

			pGDI->DrawTexture(m_pBitmap[10],
				WRect(rect.x + 15.0f, rect.y + 57.0f, m_pBitmap[10]->Width(),
					m_pBitmap[10]->Height()),
				0xffffffff, 0);

			pGDI->SetTextStyle(1);
			pGDI->SetTextColor(0xff33365f, 0xffffffff);

			pGDI->Print(WPoint(rect.x + 107.0f, rect.y + 89.0f), 1,
				GuildInfo()->GetMyGuildInfo().guildName);

			pGDI->SetTextStyle(0);
			pGDI->SetTextColor(0, 0xffffffff);
		}
		else if (!GuildInfo()->IsGuildMember())
		{
			const WRect& rect = m_pArea[4]->GetRect();

			pGDI->DrawTexture(m_pBitmap[8],
				WRect(rect.x, rect.y, m_pBitmap[8]->Width(),
					m_pBitmap[8]->Height()),
				0xffffffff, 0);
			pGDI->DrawTexture(m_pBitmap[9],
				WRect(rect.x, rect.y + 256.0f, m_pBitmap[8]->Width(),
					m_pBitmap[9]->Height()),
				0xffffffff, 0);
		}
	}

	void CGuildMainForm::OnGuildMemberInfoBg_AreaInit(int param)
	{
		m_pArea[5] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[5]);
		Controller()->InsertPostAction(m_pArea[5], FORM_GUILD_MEMBER,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildMemberInfoBg_OwnerDraw(int param)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		pGDI->DrawTexture(m_pBitmap[0],
			WRect(m_rect.x + 68.0f, m_rect.y + 83.0f, m_pBitmap[0]->Width(),
				m_pBitmap[0]->Height()),
			0xffffffff, 0);
		pGDI->DrawTexture(m_pBitmap[1],
			WRect(m_rect.x + 324.0f, m_rect.y + 83.0f, m_pBitmap[1]->Width(),
				m_pBitmap[1]->Height()),
			0xffffffff, 0);
		pGDI->DrawTexture(m_pBitmap[2],
			WRect(m_rect.x + 68.0f, m_rect.y + 339.0f, m_pBitmap[2]->Width(),
				m_pBitmap[2]->Height()),
			0xffffffff, 0);
		pGDI->DrawTexture(m_pBitmap[3],
			WRect(m_rect.x + 324.0f, m_rect.y + 339.0f, m_pBitmap[3]->Width(),
				m_pBitmap[3]->Height()),
			0xffffffff, 0);
	}

	void CGuildMainForm::OnSearchBg1_AreaInit(int param)
	{
		m_pArea[6] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[6]);
		Controller()->InsertPostAction(m_pArea[6], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnSearchBg2_AreaInit(int param)
	{
		m_pArea[7] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
		Controller()->InsertControl(m_pArea[7]);
		Controller()->InsertPostAction(m_pArea[7], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildIntroduce_EditInit(int param)
	{
		m_pEdit[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		Controller()->InsertControl(m_pEdit[0]);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::MoveLeftSide);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER,
			&CGuildFormController::WritableWindow);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_INFO,
			AUTHORITY_MEMBER, &CGuildFormController::ReadOnlywindow);

		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::MoveRightSide);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::ClearEdit);
		Controller()->InsertPostAction(m_pEdit[0], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::ReadOnlywindow);
	}

	void CGuildMainForm::OnGuildNotice_EditInit(int param)
	{
		m_pEdit[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		Controller()->InsertControl(m_pEdit[1]);
		Controller()->InsertPostAction(m_pEdit[1], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pEdit[1], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER,
			&CGuildFormController::WritableWindow);
		Controller()->InsertPostAction(m_pEdit[1], FORM_GUILD_INFO,
			AUTHORITY_NULL | AUTHORITY_MEMBER | AUTHORITY_WAIT,
			&CGuildFormController::ReadOnlywindow);
	}

	void CGuildMainForm::OnGuildAct_EditInit(int param)
	{
		m_pEdit[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		Controller()->InsertControl(m_pEdit[2]);
		Controller()->InsertPostAction(m_pEdit[2], FORM_GUILD_INFO,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
		Controller()->InsertPostAction(m_pEdit[2], FORM_GUILD_INFO,
			AUTHORITY_ALL, &CGuildFormController::ReadOnlywindow);
	}

	void CGuildMainForm::OnGuildSearch_EditInit(int param)
	{
		m_pEdit[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		Controller()->InsertControl(m_pEdit[3]);
		Controller()->InsertPostAction(m_pEdit[3], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildSearch_EnterKey(int param)
	{
		OnGuildSearchInfo_LButtonUp();
	}

	void CGuildMainForm::OnGuildMemberTalk_EditInit(int param)
	{
		m_pEdit[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
		Controller()->InsertControl(m_pEdit[4]);
	}

	void CGuildMainForm::OnGuildForm_CloseInit(int param)
	{
		m_pButton[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void CGuildMainForm::OnGuildForm_CloseLButtonUp()
	{
		Close(true);
	}

	void CGuildMainForm::OnGuildIntroConfirm_ButtonInit(int param)
	{
		m_pButton[1] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[1]);
		Controller()->InsertPostAction(m_pButton[1], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER,
			&CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildIntroConfirm_LButtonUp()
	{
		m_authControllers[MyGuildClassIndex()]->OnGuildIntroConfirm_LButtonUp();
	}

	void CGuildMainForm::OnGuildNoticeConfirm_ButtonInit(int param)
	{
		m_pButton[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[2]);
		Controller()->InsertPostAction(m_pButton[2], FORM_GUILD_INFO,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER,
			&CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildNoticeConfirm_LButtonUp()
	{
		m_authControllers[MyGuildClassIndex()]
			->OnGuildNoticeConfirm_LButtonUp();
	}

	void CGuildMainForm::OnGuildJoin_ButtonInit(int param)
	{
		m_pButton[3] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[3]);
		Controller()->InsertPostAction(m_pButton[3], FORM_GUILD_SEARCH,
			AUTHORITY_NULL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildJoin_LButtonUp()
	{
		if (m_selectedGuild.guildUID == 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)GuildInfo()->GuildMsg(1038), 0, 0, 0, 0));
			return;
		}

		m_authControllers[MyGuildClassIndex()]->OnGuildJoin_LButtonUp();
	}

	void CGuildMainForm::OnGuildSearchInfo_ButtonInit(int param)
	{
		m_pButton[4] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[4]);
		Controller()->InsertPostAction(m_pButton[4], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildSearchInfo_LButtonUp()
	{
		m_searchText = m_pEdit[3]->GetLine(1, false);
		if (m_searchText.empty())
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)GuildInfo()->GuildMsg(1037), 0, 0, 0, 0));
			return;
		}

		if (m_searchText.length() < 4)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)MakeStr(
					K2L_Compatibility(
						"\\c0xffff0000\\c\xbf\xb5\xb9\xae %d\xb1\xdb\xc0\xda"
						"\\c0xff000000\\c \xb6\xc7\xb4\xc2 \\c0xffff0000\\c"
						"\xc7\xd1\xb1\xdb %d\xb1\xdb\xc0\xda\\c0xff000000\\c"
						"\xc0\xcc\xbb\xf3 \xc0\xd4\xb7\xc2\xc7\xcf\xbf\xa9\xbe\xdf "
						"\xc7\xd5\xb4\xcf\xb4\xd9."),
					4, 2),
				0, 0, 0, 0));
			return;
		}

		if (m_searchText.find("%%") == std::string::npos)
		{
			m_searchMode = 1;
			RequestSearch_Page(1);
		}
	}

	void CGuildMainForm::OnGuildJoinAbort_ButtonInit(int param)
	{
		m_pButton[6] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[6]);
		Controller()->InsertPostAction(m_pButton[6],
			FORM_GUILD_INFO | FORM_GUILD_MEMBER | FORM_GUILD_SEARCH,
			AUTHORITY_WAIT, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildJoinAbort_LButtonUp()
	{
		m_authControllers[MyGuildClassIndex()]->OnGuildJoinAbort_LButtonUp();
	}

	void CGuildMainForm::OnGuildMemberTalk_ButtonInit(int param)
	{
		m_pButton[8] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[8]);
	}

	void CGuildMainForm::OnGuildMemberTalk_LButtonUp()
	{
		if (!m_pSelectedMember)
		{
			m_pSelectedMember = m_pListBox[0]->GetSelected();
			if (!m_pSelectedMember)
			{
				Close(FrCANCEL, true);
				return;
			}
		}

		sMemberItem* pItem = (sMemberItem*)m_pSelectedMember->pData;
		pItem->bEdit = false;

		bool bSend = false;
		std::string text(m_pEdit[4]->GetLine(1, false));
		str_Replace(text, "'", "`");

		int result = Doc()->m_chatManager.FilteringGuildString(text.c_str(), 0,
			20, false);
		if (result != 1 && result > 0)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)GuildInfo()->GuildMsg(result), 0, 0, 0, 0));
		}
		else
		{
			if (strcmp(pItem->pInfo->message, text.c_str()) != 0)
			{
				WSendPacketSem983 packet((enumClientPacket)270); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
				packet.Encode4(GuildInfo()->GetMyGuildInfo().guildUID);
				packet.Encode4(pItem->pInfo->userUID);
				packet.EncodeStr(text);
				packet.Send(TO_GAME);
				bSend = true;
			}

			FrWnd* pButton = m_pButton[8];
			pButton->SetVisible(false);
			pButton->Enable(false);
			FrWnd* pEdit = m_pEdit[4];
			pEdit->SetVisible(false);
			pEdit->Enable(false);

			if (bSend == true)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			}
		}
	}

	void CGuildMainForm::OnGuildSearchPrev_ButtonInit(int param)
	{
		m_pButton[9] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[9]);
		Controller()->InsertPostAction(m_pButton[9], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::VisibleWindow);

		m_pSearchArrow->SetLeftArrow(m_pButton[9]);
	}

	void CGuildMainForm::OnGuildSearchPrev_LButtonUp()
	{
		unsigned int group = (m_searchPage - 1) / s_pageGroupSize;
		int page = group * s_pageGroupSize;
		RequestSearch_Page(page);
	}

	void CGuildMainForm::OnGuildSearchNext_ButtonInit(int param)
	{
		m_pButton[10] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[10]);
		Controller()->InsertPostAction(m_pButton[10], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::VisibleWindow);

		m_pSearchArrow->SetRightArrow(m_pButton[10]);
	}

	void CGuildMainForm::OnGuildSearchNext_LButtonUp()
	{
		unsigned int group = (m_searchPage - 1) / s_pageGroupSize;
		int page = group * s_pageGroupSize + s_pageGroupSize + 1;
		RequestSearch_Page(page);
	}

	void CGuildMainForm::OnGuildShutDown_ButtonInit(int param)
	{
		m_pButton[5] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[5]);
		Controller()->InsertPostAction(m_pButton[5],
			FORM_GUILD_INFO | FORM_GUILD_MEMBER | FORM_GUILD_SEARCH,
			AUTHORITY_MASTER, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildShutDown_LButtonUp()
	{
		m_authControllers[MyGuildClassIndex()]->OnGuildShutDown_LButtonUp();
	}

	void CGuildMainForm::OnGuildOut_ButtonInit(int param)
	{
		m_pButton[7] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		Controller()->InsertControl(m_pButton[7]);
		Controller()->InsertPostAction(m_pButton[7],
			FORM_GUILD_INFO | FORM_GUILD_MEMBER | FORM_GUILD_SEARCH,
			AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildOut_LButtonUp()
	{
		m_authControllers[MyGuildClassIndex()]->OnGuildOut_LButtonUp();
	}

	void CGuildMainForm::OnGuildMemberInfo_ListBoxInit(int param)
	{
		m_pListBox[0] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
		Controller()->InsertControl(m_pListBox[0]);
		Controller()->InsertPostAction(m_pListBox[0], FORM_GUILD_MEMBER,
			AUTHORITY_MASTER | AUTHORITY_SUBMASTER | AUTHORITY_MEMBER,
			&CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildMemberInfo_OwnerDraw(int param)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (!pItem)
			return;

		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
		sMemberItem* pMember = (sMemberItem*)pItem->pData;

		if (pItem->selected == true)
		{
			pGDI->DrawTexture(m_pBitmap[14],
				WRect(pItem->pos.x + 1.0f, pItem->pos.y,
					m_pListBox[0]->GetItemWidth() + 2.0f,
					m_pListBox[0]->GetItemHeight()),
				0xffffffff, 0);
		}

		const Bitmap* pIcon = NULL;
		switch (pMember->pInfo->classIdx)
		{
		case 1:
			pIcon = m_pBitmap[4];
			break;
		case 2:
			pIcon = m_pBitmap[5];
			break;
		case 3:
			pIcon = m_pBitmap[6];
			break;
		case 9:
			pIcon = m_pBitmap[7];
			break;
		default:
			return;
		}

		if (pIcon)
		{
			pGDI->DrawTexture(pIcon,
				WRect(pItem->pos.x + 20.0f, pItem->pos.y + 7.0f,
					(float)pIcon->Width(), (float)pIcon->Height()),
				0xffffffff, 0);
		}

		const Bitmap* pOnline = NULL;
		switch (pMember->pInfo->online)
		{
		case 0:
			pOnline = m_pBitmap[12];
			break;
		case 1:
			pOnline = m_pBitmap[11];
			break;
		}

		pGDI->DrawTexture(pOnline,
			WRect(pItem->pos.x + 96.0f,
				pItem->pos.y + 9.0f, pOnline->Width(), pOnline->Height()),
			0xffffffff, 0);

		pGDI->SetTextStyle(1);
		pGDI->SetTextColor(0xff333333, 0xffffffff);

		pGDI->Print(WPoint(pItem->pos.x + 120.0f, pItem->pos.y + 9.0f), 0,
			pMember->pInfo->nickname);

		pGDI->SetTextColor(0xff376f90, 0xffffffff);
		m_authControllers[MyGuildClassIndex()]->DrawGuildTalk(pItem);

		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(0, 0xffffffff);
	}

	void CGuildMainForm::OnGuildMemberInfo_LButtonDown()
	{
		FrListBox* pList = m_pListBox[0];

		FrListItem* pItem = pList->GetItemUnderCursor();
		if (!pItem)
		{
			if (m_pManagerCmd)
				m_pManagerCmd->SetVisible(false);
			return;
		}

		SelectMemberItem(pList, pItem);

		WRect rect(pItem->pos.x, pItem->pos.y, 0, 0);
		rect.w = pList->GetRect().w * 0.5f;
		rect.h = (float)pList->GetItemHeight();
		if (CMouseCursor::Instance()->InArea(rect))
		{
			WVector pt = g_input->GetMousePoint();
			PopupMemberCommand(pt.x, pt.y);
			return;
		}

		rect.x = m_talkIconOffset + pItem->pos.x;
		if (CMouseCursor::Instance()->InArea(rect))
		{
			sMemberItem* pMember = (sMemberItem*)pItem->pData;
			if (CanExecuteClass(pMember->pInfo) && pMember->bEdit == false)
			{
				pMember->bEdit = true;
				CheckTalkState();
				return;
			}

			if (m_pManagerCmd->IsVisible())
				m_pManagerCmd->SetVisible(false);
			return;
		}

		m_pManagerCmd->SetVisible(false);
	}

	void CGuildMainForm::OnMemberPage_ListBoxInit(int param)
	{
		m_pListBox[2] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
		Controller()->InsertControl(m_pListBox[2]);
		Controller()->InsertPostAction(m_pListBox[2], FORM_GUILD_MEMBER,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);

		m_pListBox[2]->EnableScrollBar(false);
	}

	void CGuildMainForm::OnMemberPage_ListOwnerDraw(int param)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (!pItem)
			return;

		static char szPage[8];
		itoa(pItem->idx + 1, szPage, 10);

		int page = pItem->idx + 1;

		if (m_memberPage == page)
		{
			m_pCurrentPageFont->Print(g_view, pItem->pos.x, pItem->pos.y,
				szPage, 0, 0xffffffff, 0);
		}
		else
		{
			m_pPageFont->Print(g_view, pItem->pos.x, pItem->pos.y, szPage, 0,
				0xffffffff, 0);
		}
	}

	void CGuildMainForm::OnMemberPage_ListLButtonDown()
	{
		FrListItem* pItem = m_pListBox[2]->GetSelected();
		if (!pItem)
			return;

		int page = pItem->idx + 1;
		if (page == m_memberPage)
			return;

		m_memberPage = page;
		RequestMember_Page(page);
	}

	void CGuildMainForm::OnGuildSearchInfo_ListBoxInit(int param)
	{
		m_pListBox[1] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
		Controller()->InsertControl(m_pListBox[1]);
		Controller()->InsertPostAction(m_pListBox[1], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildSearchInfo_OwnerDraw(int param)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (!pItem)
			return;

		const GUILD_LIST* pGuild = (const GUILD_LIST*)pItem->pData;
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		if (pItem->selected == true)
		{
			pGDI->DrawTexture(m_pBitmap[14],
				WRect(pItem->pos.x, pItem->pos.y, m_pListBox[1]->GetItemWidth(),
					m_pListBox[1]->GetItemHeight()),
				0xffffffff, 0);
		}

		pGDI->SetTextStyle(1);
		pGDI->SetTextColor(0xff333333, 0xffffffff);

		pGDI->Print(WPoint(pItem->pos.x + 38.0f, pItem->pos.y + 10.0f), 0,
			pGuild->guildName);

		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(0, 0xffffffff);
	}

	void CGuildMainForm::OnGuildSearchInfo_LButtonDown()
	{
		FrListItem* pItem = m_pListBox[1]->GetSelected();

		if (!pItem)
		{
			memset(&m_selectedGuild, 0, sizeof(m_selectedGuild));
			m_pEdit[0]->ClearLine();
			return;
		}

		const GUILD_LIST* pGuild = (const GUILD_LIST*)pItem->pData;
		if (m_selectedGuild.guildUID != pGuild->guildUID)
		{
			m_selectedGuild = *pGuild;

			m_pEdit[0]->ClearLine();
			m_pEdit[0]->AddText(m_selectedGuild.introduce, false, true);
			m_pEdit[0]->GetScrollBar()->ScrollToFirst();
		}
	}

	void CGuildMainForm::OnGuildSearchPage_ListBoxInit(int param)
	{
		m_pListBox[3] = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
		Controller()->InsertControl(m_pListBox[3]);
		Controller()->InsertPostAction(m_pListBox[3], FORM_GUILD_SEARCH,
			AUTHORITY_ALL, &CGuildFormController::EnableWindow);
	}

	void CGuildMainForm::OnGuildSearchPage_OwnerDraw(int param)
	{
		FrListItem* pItem = (FrListItem*)param;
		if (!pItem)
			return;

		static char szPage[8];
		memset(szPage, 0, sizeof(szPage));

		unsigned int page = (int)pItem->pData + 1;
		itoa(page, szPage, 10);

		if (m_searchPage == page)
		{
			m_pCurrentPageFont->Print(g_view, pItem->pos.x, pItem->pos.y,
				szPage, 0, 0xffffffff, 0);
		}
		else
		{
			m_pPageFont->Print(g_view, pItem->pos.x, pItem->pos.y, szPage, 0,
				0xffffffff, 0);
		}
	}

	void CGuildMainForm::OnGuildSearchPage_LButtonDown()
	{
		FrListItem* pItem = m_pListBox[3]->GetSelected();
		if (!pItem)
			return;

		int page = (int)pItem->pData + 1;
		if (page == m_searchPage)
			return;

		m_searchPage = page;
		RequestSearch_Page(page);
	}

	void CGuildMainForm::OnGuildManagerCmd_ContextInit(int param)
	{
		m_pManagerCmd = DYNAMIC_CAST(FrContextMenuCtrl, (FrWnd*)param);
		if (!m_pManagerCmd)
			return;

		m_pManagerCmd->SetVisible(false);
		m_pManagerCmd->SetFixed(true);
	}

	void CGuildMainForm::OnGuildManagerCmd_LButtonDown()
	{
		const char* command = m_pManagerCmd->GetSelectMenuText();
		m_pManagerCmd->SetVisible(false);

		ProcManagerCommand(command);
	}

	void CGuildMainForm::OnGuildManagerCmd_LButtonUp()
	{
		m_pManagerCmd->SetVisible(false);
	}

	IMPLEMENT_OBJECT(FrGuildJoin, FrForm)

	BEGIN_FRESH_MSGMAP(FrGuildJoin, FrForm)

	ON_FRESH_VI("request_paper", FRCMD_INIT, FrGuildJoin::OnGuildJoin_AreaInit)
	ON_FRESH_VI("request_paper", FRCMD_OWNERDRAW,
		FrGuildJoin::OnGuildJoin_AreaOwnerDraw)
	ON_FRESH_VI("story", FRCMD_INIT, FrGuildJoin::OnJoin_EditInit)
	ON_FRESH_VI("request_join", FRCMD_INIT,
		FrGuildJoin::OnRequestJoin_ButtonInit)
	ON_FRESH_VV("request_join", FRCMD_LBUTTONUP,
		FrGuildJoin::OnRequestJoin_LButtonUp)
	ON_FRESH_VI("request_ok", FRCMD_INIT, FrGuildJoin::OnRequestOK_ButtonInit)
	ON_FRESH_VV("request_ok", FRCMD_LBUTTONUP,
		FrGuildJoin::OnRequestOK_LButtonUp)
	ON_FRESH_VI("form_close", FRCMD_INIT, FrGuildJoin::OnFormClose_ButtonInit)
	ON_FRESH_VV("form_close", FRCMD_LBUTTONUP,
		FrGuildJoin::OnFormClose_LButtonUp)

	END_FRESH_MSGMAP()

	FrGuildJoin::FrGuildJoin()
		: m_pJoinArea(NULL),
		  m_pJoinEdit(NULL),
		  m_pBackground(NULL),
		  m_guildNumber(0),
		  m_bRequested(false)
	{
		memset(m_pButton, 0, sizeof(m_pButton));
	}

	FrGuildJoin::~FrGuildJoin()
	{
	}

	bool FrGuildJoin::OnInit()
	{
		m_pBackground = g_pFresh->RegisterBitmap("guild_popup_write_bg");

		return FrForm::OnInit();
	}

	void FrGuildJoin::SetGuildNumber(int guildNumber)
	{
		m_guildNumber = guildNumber;
	}

	void FrGuildJoin::SetSuccessRequest()
	{
		m_bRequested = true;
		m_pJoinEdit->SetReadOnly(true);
		m_pJoinEdit->ClearLine();
		m_pJoinEdit->SetFontColor(0xff33365f);
		m_pJoinEdit->AddText(K2L_Compatibility(
								 "\xb0\xa1\xc0\xd4 \xbd\xc5\xc3\xbb\xc7\xcf"
								 "\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
			false, true);

		FrButton* pRequestJoin = m_pButton[0];
		pRequestJoin->SetVisible(false);
		pRequestJoin->Enable(false);
		FrButton* pRequestOK = m_pButton[1];
		pRequestOK->SetVisible(true);
		pRequestOK->Enable(true);
	}

	void FrGuildJoin::OnGuildJoin_AreaInit(int param)
	{
		m_pJoinArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	}

	void FrGuildJoin::OnGuildJoin_AreaOwnerDraw(int param)
	{
		if (m_bRequested == true)
			return;

		g_pFresh->GetManager()->GetGDI()->DrawTexture(m_pBackground,
			WRect(m_rect.x + 33.0f, m_rect.y - 20.0f, m_pBackground->Width(),
				m_pBackground->Height()),
			0xffffffff, 0);
	}

	void FrGuildJoin::OnJoin_EditInit(int param)
	{
		m_pJoinEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	}

	void FrGuildJoin::OnRequestJoin_ButtonInit(int param)
	{
		m_pButton[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void FrGuildJoin::OnRequestJoin_LButtonUp()
	{
		if (!m_pJoinEdit)
			return;

		std::string text;

		int i = 1, lineNum = m_pJoinEdit->GetLineNum();
		for (; i <= lineNum; i++)
			text += m_pJoinEdit->GetLine(i, false);

		if (text.empty())
		{
			AfxGetTask()->GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, 1014, 0, 0, 0);
			return;
		}

		WSendPacketSem983 packet((enumClientPacket)265); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(m_guildNumber);
		packet.EncodeStr(text);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}

	void FrGuildJoin::OnRequestOK_ButtonInit(int param)
	{
		FrButton* pButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
		m_pButton[1] = pButton;
		if (pButton)
		{
			pButton->SetVisible(false);
			pButton->Enable(false);
		}
	}

	void FrGuildJoin::OnRequestOK_LButtonUp()
	{
		Close(FrOK, true);
	}

	void FrGuildJoin::OnFormClose_ButtonInit(int param)
	{
		m_pButton[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	}

	void FrGuildJoin::OnFormClose_LButtonUp()
	{
		Close(FrNO, true);
	}
}
