#include "minatl.h"
#include "userlistdlg.h"
#include "shareddoc.h"
#include "user_info.h"
#include "projectg.h"
#include "netresourcemanager.h"
#include "golftask.h"
#include "actor.h"
#include "frscrollbar.h"
#include "frwndinl.h"
#include "packet.h"
#include "../shared/localize.h"

inline unsigned long CSharedDoc::GetFriendGameSvrUID()
{
	return m_friendGameSvrUID;
}
extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrUserListDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrUserListDlg, FrForm)

ON_FRESH_VI("invite", FRCMD_INIT, FrUserListDlg::OnInviteBtnInit)
ON_FRESH_VV("invite", FRCMD_LBUTTONUP, FrUserListDlg::OnInviteBtnUp)
ON_FRESH_VI("whisper", FRCMD_INIT, FrUserListDlg::OnWhisperBtnInit)
ON_FRESH_VV("whisper", FRCMD_LBUTTONUP, FrUserListDlg::OnWhisperBtnUp)
ON_FRESH_VV("addfriend", FRCMD_LBUTTONUP, FrUserListDlg::OnFriendBtnUp)
ON_FRESH_VI("sex", FRCMD_INIT, FrUserListDlg::OnSortBtnInit)
ON_FRESH_VV("sex", FRCMD_LBUTTONUP, FrUserListDlg::OnSexBtnUp)
ON_FRESH_VI("level", FRCMD_INIT, FrUserListDlg::OnSortBtnInit)
ON_FRESH_VV("level", FRCMD_LBUTTONUP, FrUserListDlg::OnLevelBtnUp)
ON_FRESH_VI("guild", FRCMD_INIT, FrUserListDlg::OnSortBtnInit)
ON_FRESH_VV("guild", FRCMD_LBUTTONUP, FrUserListDlg::OnGuildBtnUp)
ON_FRESH_VI("id", FRCMD_INIT, FrUserListDlg::OnSortBtnInit)
ON_FRESH_VV("id", FRCMD_LBUTTONUP, FrUserListDlg::OnIdBtnUp)
ON_FRESH_VI("user", FRCMD_INIT, FrUserListDlg::OnUserListInit)
ON_FRESH_VI("user", FRCMD_OWNERDRAW, FrUserListDlg::OnUserListOwnerDraw)
ON_FRESH_VV("user", FRCMD_LBUTTONDOWN, FrUserListDlg::OnUserListLBtnDown)
ON_FRESH_VV("user", FRCMD_RBUTTONUP, FrUserListDlg::OnUserListRBtnUp)
ON_FRESH_VV("user", FRCMD_DBLCLICK, FrUserListDlg::OnUserListDClick)

END_FRESH_MSGMAP()

FrUserListDlg::FrUserListDlg()
{
	m_pWhisperBtn = NULL;
	m_pInviteBtn = NULL;
	m_pUserList = NULL;
	m_selOid = -1;
	m_selUid = -1;
	m_pInviteDelayDlg = NULL;
	m_bSortReverse = false;
	m_pAddFriendDlg = NULL;
	m_sortType = 3;
}

void FrUserListDlg::OnProc(const float dt)
{
	if (m_pInviteDelayDlg)
	{
		m_inviteDelay -= dt;
		if (m_inviteDelay < 0)
		{
			m_pInviteDelayDlg->Close(true);
			m_pInviteDelayDlg = NULL;
		}
	}
}

bool UserList_UserSexCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return (a->gender % 2) > (b->gender % 2);
}

bool UserList_UserSexRevCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return (a->gender % 2) < (b->gender % 2);
}

bool UserList_UserLevelCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	// 983 00af92b0/00af92c0: so o nivel, sem o ramo do ladder (canal 0x80)
	return a->level > b->level;
}

bool UserList_UserLevelRevCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	// 983 00af92b0/00af92c0: so o nivel, sem o ramo do ladder (canal 0x80)
	return a->level < b->level;
}

bool UserList_UserNickCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return strcmpi(a->sNick, b->sNick) < 0;
}

bool UserList_UserNickRevCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return strcmpi(a->sNick, b->sNick) > 0;
}

bool UserList_UserGuildRevCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return a->m_GuildId < b->m_GuildId;
}

bool UserList_UserGuildCompare(const void* left, const void* right)
{
	const sBriefUserInfo* a = (const sBriefUserInfo*)left;
	const sBriefUserInfo* b = (const sBriefUserInfo*)right;
	return a->m_GuildId > b->m_GuildId;
}

void FrUserListDlg::OnSortBtnInit(int param)
{
	FrButton* btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	btn->SetPushDelay(0);
	btn->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrUserListDlg::OnSexBtnUp()
{
	if (m_sortType != 0)
		m_sortType = 0;
	else
		m_bSortReverse = !m_bSortReverse;
	SortUserList();
	if (m_pUserList && m_pUserList->GetScrollBar())
		m_pUserList->GetScrollBar()->ScrollToFirst();
}

void FrUserListDlg::OnLevelBtnUp()
{
	if (m_sortType != 1)
		m_sortType = 1;
	else
		m_bSortReverse = !m_bSortReverse;
	SortUserList();
	if (m_pUserList && m_pUserList->GetScrollBar())
		m_pUserList->GetScrollBar()->ScrollToFirst();
}

void FrUserListDlg::OnGuildBtnUp()
{
	if (m_sortType != 2)
		m_sortType = 2;
	else
		m_bSortReverse = !m_bSortReverse;
	SortUserList();
	if (m_pUserList && m_pUserList->GetScrollBar())
		m_pUserList->GetScrollBar()->ScrollToFirst();
}

void FrUserListDlg::OnIdBtnUp()
{
	if (m_sortType != 3)
		m_sortType = 3;
	else
		m_bSortReverse = !m_bSortReverse;
	SortUserList();
	if (m_pUserList && m_pUserList->GetScrollBar())
		m_pUserList->GetScrollBar()->ScrollToFirst();
}

void FrUserListDlg::SortUserList()
{
	if (m_pUserList)
	{
		switch (m_sortType)
		{
		case 0:
			if (m_bSortReverse)
				m_pUserList->SortItem(UserList_UserSexRevCompare);
			else
				m_pUserList->SortItem(UserList_UserSexCompare);
			break;
		case 1:
			if (m_bSortReverse)
				m_pUserList->SortItem(UserList_UserLevelRevCompare);
			else
				m_pUserList->SortItem(UserList_UserLevelCompare);
			break;
		case 3:
			if (m_bSortReverse)
				m_pUserList->SortItem(UserList_UserNickRevCompare);
			else
				m_pUserList->SortItem(UserList_UserNickCompare);
			break;
		case 2:
			if (m_bSortReverse)
				m_pUserList->SortItem(UserList_UserGuildRevCompare);
			else
				m_pUserList->SortItem(UserList_UserGuildCompare);
			break;
		}
	}
}

void FrUserListDlg::EnableRightButton(bool enable)
{
	m_pUserList->UseRightButton(enable ? true : false);
}

void FrUserListDlg::OnUserListInit(int param)
{
	m_pUserList = (FrListBox*)param;
	if (m_pUserList)
	{
		m_pUserList->UseRightButton(true);
		m_pSexIcon[0] = m_pUserList->GetBitmap("i_male");
		m_pSexIcon[1] = m_pUserList->GetBitmap("i_female");
		m_pSexIcon[2] = m_pUserList->GetBitmap("i_male_02");
		m_pSexIcon[3] = m_pUserList->GetBitmap("i_female_02");
		m_pSexIcon[4] = m_pUserList->GetBitmap("i_male_03");
		m_pSexIcon[5] = m_pUserList->GetBitmap("i_female_03");
		m_pMannerIcon[0] = m_pUserList->GetBitmap("i_male_manner");
		m_pMannerIcon[1] = m_pUserList->GetBitmap("i_female_manner");
		m_pAngelIcon[0] = m_pUserList->GetBitmap("i_male_angel");
		m_pAngelIcon[1] = m_pUserList->GetBitmap("i_female_angel");
		m_pInGameIcon = m_pUserList->GetBitmap("ingame");
	}
}

void FrUserListDlg::OnUserListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	sBriefUserInfo* user = (sBriefUserInfo*)item->pData;
	if (!user)
		return;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* icon;
	if (user->angelicWings)
		icon = m_pAngelIcon[user->gender % 2];
	else if (user->manner)
		icon = m_pMannerIcon[user->gender % 2];
	else
		icon = m_pSexIcon[user->gender];
	WRect dest(item->pos.x, item->pos.y, (float)icon->Width(),
		(float)icon->Height());
	if (user->dwGuid == m_selOid)
		gdi->Box(WRect(dest.x, dest.y, (float)m_pUserList->GetItemWidth(),
					 (float)m_pUserList->GetItemHeight()),
			0x999b9dff, 0, 0);
	gdi->DrawTexture(icon, dest, 0xffffffff, 0);
	if (user->roomIndex != 0xffff)
		gdi->DrawTexture(m_pInGameIcon,
			WRect(dest.x, dest.y + 3, (float)m_pInGameIcon->Width(),
				(float)m_pInGameIcon->Height()),
			0xffffffff, 0);
	static const char s_levelFmt[] = "level_%03d"; // 983: um literal so (00d00470)
	const Bitmap* level;
	if (user->dwTitle)
	{
		IFF_STRUCT::sSkin* skin = ItemManager()->FindSkin(user->dwTitle);
		if (skin)
			level = g_pFresh->GetBitmap(skin->c.Icon);
		else
			level = g_pFresh->GetBitmap(MakeStr(s_levelFmt, user->level + 1));
	}
	else
		level = g_pFresh->GetBitmap(MakeStr(s_levelFmt, user->level + 1));
	if (user->dwIdentity & 0x8000) // 983
		level = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");
	if (level)
		gdi->DrawTexture(level,
			WRect(item->pos.x + 50 - (int)(float)(level->Width() * 0.5f),
				item->pos.y + 13 - (int)(float)(level->Height() * 0.5f),
				(float)level->Width(), (float)level->Height()),
			0xffffffff, 0);
	if (user->m_GuildId && !(user->dwIdentity & 0x14))
	{
		const Bitmap* emblem =
			NetResourceManager::Instance()->GetEmblem983(user->m_GuildId, user->dwEmblemVer983);
		if (emblem)
			gdi->DrawTexture(emblem,
				WRect(item->pos.x + 85, item->pos.y + 7 - 6,
					(float)emblem->Width(), (float)emblem->Height()),
				0xffffffff, 0);
	}
	if (!CProjectG::Instance()->HidePrivacy())
	{
		unsigned char identity = (unsigned char)user->dwIdentity;
		unsigned long color = (identity & 0x14) ? 0x80ffffff : 0xffffffff;
		if (item->underCursor)
		{
			gdi->SetTextColor(0xffffffff, 0xff808080);
			gdi->SetTextStyle(2);
		}
		else
		{
			gdi->SetTextColor(0xff000000, 0xffffffff);
			gdi->SetTextStyle(0);
		}
		g_pFresh->GetManager()->PrintText(
			WPoint(item->pos.x + 112, item->pos.y + 7), 0, user->sNick, -1,
			color);
	}
}

void FrUserListDlg::OnUserListLBtnDown()
{
	// HACK
	if (0)
		OnSexBtnUp();
	FrListItem* item = m_pUserList->GetItemUnderCursor();
	if (item)
	{
		sBriefUserInfo* user = (sBriefUserInfo*)item->pData;
		m_selOid = user->dwGuid;
		m_selUid = user->dwUid;
		// 983 00af9cf0: sem o teste do canal (Type & 0x80) do 645
		if (m_pInviteBtn)
			m_pInviteBtn->Enable(m_selOid != MyGuid(false) &&
				user->roomIndex == 0xffff);
		if (m_pWhisperBtn)
			m_pWhisperBtn->Enable(m_selOid != MyGuid(false));
		if (Doc()->m_roomInfo.gameType == 6)
			m_pInviteBtn->Enable(false);
	}
}

void FrUserListDlg::MakeUserList(std::map<unsigned long, sBriefUserInfo>& users)
{
	// 983 00af9da0: aqui MyGuid e chamada fora de linha (0047dd30); nas outras funcoes vem em linha
	unsigned long (__fastcall* const myGuid)(bool) = MyGuid;
	m_pUserList->ClearItem();
	for (std::map<unsigned long, sBriefUserInfo>::iterator it = users.begin();
		it != users.end(); ++it)
	{
		if ((*it).second.dwGuid != myGuid(false) &&
			(bool((Doc()->m_myInfo.info.dwIdentity >> 2) & 1) ||
				!((*it).second.dwIdentity & 0x14)))
			m_pUserList->AddItem(&(*it).second);
	}
}

void FrUserListDlg::OnUserListRBtnUp()
{
	FrListItem* item = m_pUserList->GetItemUnderCursor();
	if (item)
	{
		sBriefUserInfo* user = (sBriefUserInfo*)item->pData;
		if (user)
		{
			m_selOid = user->dwGuid;
			m_selUid = user->dwUid;
			// 983 00af9e70: sem o teste do canal (Type & 0x80) do 645
			if (m_pInviteBtn)
				m_pInviteBtn->Enable(
					!bool((Doc()->m_myInfo.info.dwIdentity >> 1) & 1) &&
					user->roomIndex == 0xffff);
			if (m_pWhisperBtn)
				m_pWhisperBtn->Enable(m_selOid != MyGuid(false));
			if (CUserInfo::Instance())
			{
				std::map<unsigned long, sBriefUserInfo>::iterator it =
					Doc()->m_briefUserInfoMap.find(m_selOid);
				if (it != Doc()->m_briefUserInfoMap.end())
					CUserInfo::Instance()->SetInfo((*it).second.dwUid, m_selOid,
						true, true, false, false,
						std::string((*it).second.sNick));
			}
			if (Doc()->m_roomInfo.gameType == 6)
				m_pInviteBtn->Enable(false);
		}
	}
}

void FrUserListDlg::OnUserListDClick()
{
	if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
		OnInviteBtnUp();
}

void FrUserListDlg::OnInviteBtnInit(int param)
{
	m_pInviteBtn = (FrButton*)param;
	if (m_pInviteBtn)
		m_pInviteBtn->Enable(false);
}

void FrUserListDlg::OnInviteBtnUp()
{
	if (m_selOid != -1 && m_selUid != -1)
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(m_selOid);
		if (it != Doc()->m_briefUserInfoMap.end())
		{
			if (Doc()->IsInviteMode())
				AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\303\312\264\353 \277\344\303\273\301\337\277\241 \300\326\275\300\264\317\264\331. \300\341\275\303\310\304 \264\331\275\303 \300\314\277\353\307\317\274\274\277\344."),
					0xff000000, 0, 0, 0);
			else
			{
				m_inviteDelay = 3.0f;
				if (!m_pInviteDelayDlg)
				{
					m_pInviteDelayDlg = CreateForm<FrForm>(
						g_pFresh->GetManager(), this, "wait", NULL);
					m_pInviteDelayDlg->SetMessage(
						MakeStr(
							K2L_Compatibility("%s\264\324\277\241\260\324 \303\312\264\353 \270\336\275\303\301\366\270\246 \272\270\263\302\275\300\264\317\264\331"),
							(*it).second.sNick),
						false);
					m_pInviteDelayDlg->Open(NULL, 0);
					FrButton* cancel = DYNAMIC_CAST(FrButton,
						m_pInviteDelayDlg->FindChildByName("cancel"));
					if (cancel)
						cancel->SetVisible(false);
				}
				if (IsLocalContent(S4_INVITE_FRIEND))
				{
					Doc()->SetFriendGameSvrUID(Doc()->m_gameServerUID);
					Doc()->SetInviteMode(1);
					WSendPacket packet((enumClientPacket)0xba);
					packet.EncodeStr((*it).second.sNick);
					packet.Encode4((*it).second.dwUid);
					packet.Send(TO_GAME);
				}
				else
				{
					WSendPacket packet((enumClientPacket)41);
					packet.Encode4((*it).second.dwUid);
					packet.Send(TO_GAME);
				}
			}
		}
	}
}

void FrUserListDlg::OnFriendBtnUp()
{
	FrAddFriendDlg* dialog = CreateForm<FrAddFriendDlg>(g_pFresh->GetManager(),
		this, "addfriend", NULL);
	m_pAddFriendDlg = dialog;
	if (m_selOid != -1 && m_selOid != Doc()->m_myInfo.info.dwGuid)
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(m_selOid);
		if (it != Doc()->m_briefUserInfoMap.end())
			dialog->SetNick((*it).second.sNick);
	}
	// 983 00afa210: devolve o resultado em OnAddFriendDlgResult (zera m_pAddFriendDlg)
	m_pAddFriendDlg->Open((FRESH_PFN_RESULT)&FrUserListDlg::OnAddFriendDlgResult, 1);
}

void FrUserListDlg::OnWhisperBtnInit(int param)
{
	m_pWhisperBtn = (FrButton*)param;
	m_pWhisperBtn->Enable(false);
}

void FrUserListDlg::OnWhisperBtnUp()
{
	if (m_selOid == -1)
	{
		if (m_pWhisperBtn)
			m_pWhisperBtn->Enable(false);
	}
	else
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(m_selOid);
		if (it != Doc()->m_briefUserInfoMap.end())
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, M983(0x37), (int)(*it).second.sNick, 0, 0, 0, 0);
	}
}

void FrUserListDlg::AddUser(void* data)
{
	if (m_pUserList)
		m_pUserList->AddItem(data);
}

void FrUserListDlg::DelUser(void* data, int index)
{
	if (m_pUserList)
		m_pUserList->DelItem(data, index);
}

void FrUserListDlg::SetAddFriendDlgNick(const char* nick, const char*)
{
	if (m_pAddFriendDlg)
		m_pAddFriendDlg->SetNick(nick);
}

bool FrUserListDlg::OnAddFriendDlgResult(int, FrForm*)
{
	m_pAddFriendDlg = NULL;
	return true;
}

void FrUserListDlg::CloseInviteDelayDlg()
{
	if (IsLocalContent(S4_INVITE_FRIEND) && m_pInviteDelayDlg)
	{
		m_pInviteDelayDlg->Close(true);
		m_pInviteDelayDlg = NULL;
	}
}

void FrUserListDlg::SetRequestFriendData(const char* nick, unsigned long uid)
{
	if (nick && m_pAddFriendDlg)
	{
		m_pAddFriendDlg->SetNick(nick);
		m_pAddFriendDlg->SetFindUID(uid);
	}
}
