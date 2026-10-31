#include "minatl.h"
#include "vc10_layout.h"
#include "messengerdlg.h"
#include "packets/messengerpackets.h"	// PKT (frente C)
#include "messengerchatdlg.h"
#include "addfrienddlg.h"
#include "notedlg.h"
#include "memodlg.h"
#include "guildnoticedlg.h"
#include "newsreader.h"
#include "buddymanager.h"
#include "user_info.h"
#include "netresourcemanager.h"
#include "networksystem.h"
#include "clientsocket.h"
#include "packet.h"
#include "inputmanager.h"
#include "golftask.h"
#include "lobbytask.h"
#include "shoptask.h"
#include "familytask.h"
#include "avatar_task.h"
#include "chatmsg.h"
#include "projectg.h"
#include "jp/lobbyjp.h" // CLobbyState983 (00e78ca8)
#include "packets/guildpackets.h" // RbMessengerDlgGuild983 (00900ba0)

C_ASSERT_VC10(sizeof(CMessengerInfo) == 0xdc); // 983: construtor 00906bf0 (m_bEnableInvite em +0xd8)

extern Fresh* g_pFresh;

// realmyroomtask has no header yet.
class CRealMyRoomTask
{
public:
	static const WRTTI m_RTTI;
};

// B1470/D882: os crus CSharedDoc+0x10d80/81/82 sao membros (andam no cl 14.44).
C_ASSERT_VC10(offsetof(CSharedDoc, m_bRefreshCamera) == 0x10d80 && offsetof(CSharedDoc, m_bMessengerAlarm) == 0x10d81);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bMessengerInit) == 0x10d82);

// messengerdlg.cpp:48
bool FriendCompare(const void* a, const void* b)
{
	const sFriend* pA = (const sFriend*)a;
	const sFriend* pB = (const sFriend*)b;
	if (pA == NULL)
		return false;

	if (pB == NULL)
		return true;

	if (pA->Uid == 0xFFFFFFFF || pB->Uid == 0xFFFFFFFF)
		return false;

	if (pA->IsLogOn == 0 && (pB->IsLogOn))
		return false;
	if (pA->IsLogOn == 1 && !(pB->IsLogOn))
		return true;

	if (pA->IsBlock == 0 && (pB->IsBlock))
		return true;
	if (pA->IsBlock == 1 && !(pB->IsBlock))
		return false;

	if (pA->IsAccept == 0 && (pB->IsAccept))
		return false;
	if (pA->IsAccept == 1 && !(pB->IsAccept))
		return true;

	if (pA->IsAccept == 0 && !(pB->IsAccept))
	{
		if (pA->IsAgree == 1 && !(pB->IsAgree))
			return true;

		if (pA->IsAgree == 0 && (pB->IsAgree))
			return true;
	}

	return false;
}

IMPLEMENT_OBJECT(FrMessengerDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrMessengerDlg, FrForm)

ON_FRESH_VI("friendlist", FRCMD_INIT,
	FrMessengerDlg::OnMessenger_FriendListInit)
ON_FRESH_VI("friendlist", FRCMD_OWNERDRAW,
	FrMessengerDlg::OnMessenger_FriendListOwnerDraw)
ON_FRESH_VV("friendlist", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_FriendListLBtnDown)
ON_FRESH_VV("friendlist", FRCMD_RBUTTONDOWN,
	FrMessengerDlg::OnMessenger_FriendListRBtnDown)
ON_FRESH_VV("friendlist", FRCMD_DBLCLICK,
	FrMessengerDlg::OnMessenger_FriendListDBtnDown)
ON_FRESH_VI("guildlist", FRCMD_INIT, FrMessengerDlg::OnMessenger_GuildListInit)
ON_FRESH_VI("guildlist", FRCMD_OWNERDRAW,
	FrMessengerDlg::OnMessenger_GuildListOwnerDraw)
ON_FRESH_VV("guildlist", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_GuildListLBtnDown)
ON_FRESH_VV("guildlist", FRCMD_RBUTTONDOWN,
	FrMessengerDlg::OnMessenger_GuildListRBtnDown)
ON_FRESH_VV("guildlist", FRCMD_DBLCLICK,
	FrMessengerDlg::OnMessenger_GuildListDBtnDown)
ON_FRESH_VV("cancel", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_CloseLBtnDown)
ON_FRESH_VI("myinfo", FRCMD_INIT, FrMessengerDlg::OnMessenger_MyInfoInit)
ON_FRESH_VI("myinfo", FRCMD_OWNERDRAW,
	FrMessengerDlg::OnMessenger_MyInfoOwnerDraw)
ON_FRESH_VI("friend_btn", FRCMD_INIT, FrMessengerDlg::OnMessenger_FriendBtnInit)
ON_FRESH_VV("friend_btn", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_FriendLBtnDown)
ON_FRESH_VI("guild_btn", FRCMD_INIT, FrMessengerDlg::OnMessenger_GuildBtnInit)
ON_FRESH_VV("guild_btn", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_GuildLBtnDown)
ON_FRESH_VI("notice", FRCMD_INIT,
	FrMessengerDlg::OnMessenger_GuildNoticeBtnInit)
ON_FRESH_VV("notice", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_GuildNoticeLBtnDown)
ON_FRESH_VV("find_btn", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_FindLBtnDown)
// D853: botao novo do 983 (messenger.xml "guild_manage" 147 50); init 008fef70, clique 008fde30 (so ret)
ON_FRESH_VI("guild_manage", FRCMD_INIT,
	FrMessengerDlg::OnMessenger_GuildManageBtnInit983)
ON_FRESH_VV("guild_manage", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_GuildManageLBtnDown983)
ON_FRESH_VI("rmenu", FRCMD_INIT, FrMessengerDlg::OnMessenger_RightMenuInit)
ON_FRESH_VV("rmenu", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_RightMenuLBtnDown)
ON_FRESH_VI("state", FRCMD_INIT, FrMessengerDlg::OnMessenger_StateBtnInit)
ON_FRESH_VV("state", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_StateLBtnDown)
ON_FRESH_VI("state_menu", FRCMD_INIT, FrMessengerDlg::OnMessenger_StateMenuInit)
ON_FRESH_VV("state_menu", FRCMD_LBUTTONDOWN,
	FrMessengerDlg::OnMessenger_StateMenuLBtnDown)

END_FRESH_MSGMAP()

// messengerdlg.cpp:160
FrMessengerDlg::FrMessengerDlg()
	: m_pRightMenu(0),
	  m_pStateMenu(0),
	  m_pMyInfo(0),
	  m_pNoteDlg(0),
	  m_pAddFriendDlg(0),
	  m_pGuildManageDlg983(0),
	  m_pMemoDlg(0)
{
	memset(m_groupRow, 0, sizeof(m_groupRow));

	m_groupRow[0].IsLogOn = 1;
	m_groupRow[0].State = CMessengerInfo::Instance()->m_bFriendOnExpand;
	m_groupRow[0].Uid = 0xFFFFFFFF;

	m_groupRow[1].IsLogOn = 0;
	m_groupRow[1].State = CMessengerInfo::Instance()->m_bFriendOffExpand;
	m_groupRow[1].Uid = 0xFFFFFFFF;

	m_groupRow[2].GuildFriend = 0;
	*((unsigned char*)&m_groupRow[2].GameLevel + 1) &= 0x81;
	m_groupRow[2].State = 5;
	m_groupRow[2].Uid = 0xFFFFFFFF;

	m_pFriendList = 0;
	m_pGuildList = 0;
	m_pNoticeBtn = 0;
	m_pStateBtn = 0;
	m_pGuildManageBtn983 = 0;
	// leituras cruas de fora (taskmanager 0x116 +0x18c, pktviews983 +0x114/+0x190) = layout do 983
	C_ASSERT_VC10(offsetof(FrMessengerDlg, m_pGuildList) == 0x114 && offsetof(FrMessengerDlg, m_pGuildManageBtn983) == 0x168);
	C_ASSERT_VC10(offsetof(FrMessengerDlg, m_pGuildManageDlg983) == 0x18c && offsetof(FrMessengerDlg, m_guildTitleRow983) == 0x190 && offsetof(FrMessengerDlg, m_pMemoDlg) == 0x1c4);
	m_friendCount = 0;
	m_friendOnCount = 0;
	m_guildCount = 0;
	m_guildOnCount = 0;
	// D720: o 983 (00902780) tambem zera +0x160 (aba guilda, lida no OnMessenger_MyInfoOwnerDraw), +0x1ac..+0x1b4 e +0x1bc
	m_pTabBtn[2] = 0;
	m_unused1bc983 = 0;
	if (IsLocalContent((localContentType_t)0x15)) // 00902a61: +0x194 = -1, +0x188 = 0
	{
		m_guildTitleRow983.dw4 = 0xFFFFFFFF;
		m_pNoticeBtn = 0;
	}

	if (IsLocalContent(S4_INVITE_FRIEND))
		m_bEnableInvite = false;
}

// messengerdlg.cpp:230
bool FrMessengerDlg::Close(bool bDestroy)
{
	std::list<sChatSlot>::iterator it;
	for (it = CMessengerInfo::Instance()->m_chatSlotList.begin();
		it != CMessengerInfo::Instance()->m_chatSlotList.end(); ++it)
	{
		if ((*it).pDlg != NULL)
		{
			(*it).pDlg->FindNextTopFocus(false);
			break;
		}
	}

	FrForm::Close(true);

	return true;
}

// messengerdlg.cpp:262
void FrMessengerDlg::SetAddFriendNick(const char* nick, unsigned long uid)
{
	if (m_pAddFriendDlg != NULL)
	{
		m_pAddFriendDlg->SetNick(nick);
		m_pAddFriendDlg->SetFindUID(uid);
	}
}

// messengerdlg.cpp:271
void FrMessengerDlg::SetTargetNick(const char* nick)
{
	if (m_pAddFriendDlg != NULL)
	{
		m_pAddFriendDlg->SetTargetNick(nick);
	}
}

// messengerdlg.cpp:279
void FrMessengerDlg::OpenAddFriendNickDlg()
{
	OnMessenger_FindLBtnDown();
}

// messengerdlg.cpp:293
bool FrMessengerDlg::OnLButtonDown(const WPoint& pt)
{
	m_pRightMenu->SetVisible(false);

	m_pStateMenu->SetVisible(false);

	FrForm::OnLButtonDown(pt);

	return false;
}

// messengerdlg.cpp:314
void FrMessengerDlg::OnMessenger_MyInfoInit(int param)
{
	m_pMyInfo = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// messengerdlg.cpp:329
void FrMessengerDlg::OnMessenger_MyInfoOwnerDraw(int param)
{
	// 983 008ff7f0
	if (m_pMyInfo == NULL)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	float alpha = m_wndAlpha2 * m_wndAlpha;
	unsigned long color = ((int)(255.0 * alpha) << 24) | 0xFFFFFF;
	pGDI->SetAlpha(alpha);

	float x = m_pMyInfo->GetRect().x;
	float y = m_pMyInfo->GetRect().y;

	// 983: o emblema aparece com a aba da guilda visivel (test [btn+0x44],1), nao so apertada
	if (m_pTabBtn[2]->IsVisible())
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_myInfo.info.dwGuildId, Doc()->m_myInfo.info.dwEmblemVer);
		if (pEmblem)
		{
			WRect rect(x + 1.0f, y, (float)pEmblem->Width(), (float)pEmblem->Height());
			pGDI->DrawTexture(pEmblem, rect, color, 0);
		}
	}

	const char* text;
	if (!m_pStateBtn->IsVisible() || !m_pStateBtn)
	{
		text = Doc()->m_myInfo.info.sGuild;
	}
	else
	{
		// 983: o sexo e o bit 3 do byte logo depois de dwEmblemVer (Doc+0x832), nao o bit 1
		const Bitmap* pSex = m_pSexIcon[0][(((unsigned char*)&Doc()->m_myInfo.info.dwEmblemVer)[4] >> 3) & 1];
		if (pSex)
		{
			WRect rcSex(x + 1.0f, y - 3.0f, (float)pSex->Width(), (float)pSex->Height());
			pGDI->DrawTexture(pSex, rcSex, color, 0);

			MSN::eStatusType status = MESSENGER()->GetMyStatus();
			if (status != MSN::STATUS_OFFLINE && m_pStatusIcon[status])
			{
				WRect rcStatus(x + 1.0f, y, (float)m_pStatusIcon[status]->Width(),
					(float)m_pStatusIcon[status]->Height());
				pGDI->DrawTexture(m_pStatusIcon[status], rcStatus, color, 0);
			}
		}
		text = Doc()->m_myInfo.info.sNick;
	}

	g_pFresh->GetManager()->PrintText(WPoint(x + 30.0f, y + 6.0f), 0, text, 90.0f,
		0xFFFFFFFF);

	pGDI->SetAlpha(1.0f);
}

// messengerdlg.cpp:441
void FrMessengerDlg::OnMessenger_FriendListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (pFriend->Uid == 0xFFFFFFFF)
	{
		DrawFriendGroup(param);
		return;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha2 * m_wndAlpha);

	const Bitmap* pSex = m_pSexIcon[0][pFriend->Gender];
	WRect rect(pItem->pos.x + 1.0f, pItem->pos.y, pSex->Width(),
		pSex->Height());

	if (pFriend->Uid == m_selectUID)
	{
		pGDI->Box(WRect(rect.x, rect.y, m_pFriendList->GetItemWidth(),
					  m_pFriendList->GetItemHeight()),
			((int)(m_wndAlpha * 0.59765625 * m_wndAlpha2 * 255.0) << 24) |
				0x9B9DFF,
			0, 0.0f);
	}

	float iconAlpha;
	if (pFriend->IsAccept && pFriend->IsLogOn)
		iconAlpha = 255.0f;
	else
		iconAlpha = 80.0f;

	pGDI->DrawTexture(pSex, rect,
		((int)(m_wndAlpha2 * m_wndAlpha * iconAlpha) << 24) | 0xFFFFFF, 0);

	MSN::eStatusType status = GetBuddyStatus(pFriend);
	if (status != MSN::STATUS_OFFLINE)
	{
		WRect rcStatus(rect.x, rect.y + 3.0f,
			(float)m_pStatusIcon[status]->Width(),
			(float)m_pStatusIcon[status]->Height());
		pGDI->DrawTexture(m_pStatusIcon[status], rcStatus,
			FrALPHA(0xFFFFFFFF, m_wndAlpha2 * m_wndAlpha), 0);
	}

	if (pItem->underCursor)
	{
		pGDI->SetTextColor(0xFFFFFFFF, 0xFF808080);
		pGDI->SetTextStyle(2);
	}
	else
	{
		pGDI->SetTextColor(0xFF4C4C4C, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);
	}

	g_pFresh->GetManager()->PrintText(
		WPoint(pItem->pos.x + 20.0f, pItem->pos.y + 7.0f), 0, pFriend->szAlias,
		-1.0f, 0xFFFFFFFF);

	if (pItem->underCursor)
	{
		pGDI->SetTextColor(0xFFFFFFFF, 0xFF808080);
		pGDI->SetTextStyle(2);
	}
	else
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);
	}

	if (pFriend->dwGuildId != 0)
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			pFriend->dwGuildId, pFriend->emblemVer983);
		if (pEmblem)
		{
			WRect rcEmblem(pItem->pos.x + 70.0f, pItem->pos.y + 0.3f,
				(float)pEmblem->Width(), (float)pEmblem->Height());
			pGDI->DrawTexture(pEmblem, rcEmblem,
				FrALPHA(0xFFFFFFFF, m_wndAlpha2 * m_wndAlpha), 0);
		}
	}

	g_pFresh->GetManager()->PrintText(
		WPoint(pItem->pos.x + 90.0f, pItem->pos.y + 7.0f), 0, pFriend->NickName,
		90.0f, 0xFFFFFFFF);

	pGDI->SetAlpha(1.0f);
	pGDI->SetTextStyle(0);
}

// messengerdlg.cpp:398
void FrMessengerDlg::OnMessenger_FriendListInit(int param)
{
	m_pFriendList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pFriendList == NULL)
		return;

	m_pFriendList->UseRightButton(true);

	m_pGroupIcon[0] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "down_btn_n");
	m_pGroupIcon[1] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "up_btn_n");

	m_pSexIcon[0][0] = m_pFriendList->GetBitmap("i_male");
	m_pSexIcon[0][1] = m_pFriendList->GetBitmap("i_female");
	m_pSexIcon[1][0] = m_pFriendList->GetBitmap("i_male_02");
	m_pSexIcon[1][1] = m_pFriendList->GetBitmap("i_female_02");
	m_pSexIcon[2][0] = m_pFriendList->GetBitmap("i_male_03");
	m_pSexIcon[2][1] = m_pFriendList->GetBitmap("i_female_03");

	m_pStatusIcon[0] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "play_icon");
	m_pStatusIcon[1] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "dive_icon");
	m_pStatusIcon[2] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "block_icon");
	m_pStatusIcon[3] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "busy_icon");
	m_pStatusIcon[4] = NULL;
	m_selectUID = 0xFFFFFFFF;
	UpdateFriendList();
}

// messengerdlg.cpp:540
void FrMessengerDlg::OnMessenger_FriendListLBtnDown()
{
	FindNextTopFocus(false);

	m_pRightMenu->SetVisible(false);
	m_pStateMenu->SetVisible(false);

	FrListItem* pItem = m_pFriendList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (pFriend->Uid == 0xFFFFFFFF)
	{
		if (pFriend->IsLogOn)
		{
			CMessengerInfo::Instance()->m_bFriendOnExpand =
				!CMessengerInfo::Instance()->m_bFriendOnExpand;
			m_groupRow[0].State = CMessengerInfo::Instance()->m_bFriendOnExpand;
		}
		else
		{
			CMessengerInfo::Instance()->m_bFriendOffExpand =
				!CMessengerInfo::Instance()->m_bFriendOffExpand;
			m_groupRow[1].State =
				CMessengerInfo::Instance()->m_bFriendOffExpand;
		}

		UpdateFriendList();
		return;
	}

	m_selectUID = pFriend->Uid;
}

// messengerdlg.cpp:600
void FrMessengerDlg::OnMessenger_FriendListRBtnDown()
{
	OnMessenger_FriendListLBtnDown();

	FrListItem* pItem = m_pFriendList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
	m_pRightMenu->MoveWindow(pt);

	if (pFriend->Uid == 0xFFFFFFFF)
		return;

	m_pRightMenu->SetVisible(true);

	if (pFriend->IsLogOn && pFriend->IsAccept)
	{
		// "send a note" becomes "chat"
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2"),
			K2L_Compatibility("\xB4\xEB\xC8\xAD\xC7\xCF\xB1\xE2"));

		if (IsLocalContent(S4_INVITE_FRIEND) ||
			IsLocalContent(S4_GOWITH_FRIEND))
		{
			if (pFriend->IsPlay || pFriend->IsBusy)
			{
				m_pRightMenu->UnableMenuItem(
					K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2")); // "invite"
				m_pRightMenu->UnableMenuItem(
					K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2")); // "go with"
			}
		}
	}
	else
	{
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xB4\xEB\xC8\xAD\xC7\xCF\xB1\xE2"),
			K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2"));

		if (!pFriend->IsAccept)
			m_pRightMenu->UnableMenuItem(
				K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2"));

		if (IsLocalContent(S4_INVITE_FRIEND) ||
			IsLocalContent(S4_GOWITH_FRIEND))
		{
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2"));
		}
	}

	if (pFriend->IsAccept)
	{
		m_pRightMenu->UnableMenuItem(
			K2L_Compatibility("\xC4\xA3\xB1\xB8\xC7\xE3\xB0\xA1")); // "accept friend"
	}
	else
	{
		m_pRightMenu->UnableMenuItem(
			K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2")); // "block"
		m_pRightMenu->UnableMenuItem(
			K2L_Compatibility("\xBA\xB0\xB8\xED\xC0\xD4\xB7\xC2")); // "enter alias"
	}

	if (pFriend->IsAgree && !pFriend->IsAccept)
	{
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC4\xA3\xB1\xB8\xC7\xE3\xB0\xA1"));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2"));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xBA\xB0\xB8\xED\xC0\xD4\xB7\xC2"));
	}

	if (pFriend->IsBlock)
	{
		// "block" becomes "unblock"
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2"),
			K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xD8\xC1\xA6"));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2"));
	}
	else
	{
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xD8\xC1\xA6"),
			K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2"));
	}

	if (IsLocalContent(S4_INVITE_FRIEND))
	{
		if (!m_bEnableInvite)
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
	}

	// 983 00903d2e: "invite" and "visit" are always disabled in the friend menu
	m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
	m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xB9\xE6\xB9\xAE\xC7\xCF\xB1\xE2"));
}

// messengerdlg.cpp:703
void FrMessengerDlg::OnMessenger_FriendListDBtnDown()
{
	FrListItem* pItem = m_pFriendList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (pFriend->Uid == 0xFFFFFFFF)
		return;

	m_selectUID = pFriend->Uid;

	if ((pFriend->IsLogOn) && (pFriend->IsAccept))
	{
		OnChatToFriend();
	}
	else
	{
		OnNoteToFriend();
	}
}

// messengerdlg.cpp:738
void FrMessengerDlg::OnMessenger_GuildListInit(int param)
{
	m_pGuildList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pGuildList == NULL)
		return;

	m_pGuildList->UseRightButton(true);

	m_pSexIcon[0][0] = m_pGuildList->GetBitmap("i_male");
	m_pSexIcon[0][1] = m_pGuildList->GetBitmap("i_female");
	m_pSexIcon[1][0] = m_pGuildList->GetBitmap("i_male_02");
	m_pSexIcon[1][1] = m_pGuildList->GetBitmap("i_female_02");
	m_pSexIcon[2][0] = m_pGuildList->GetBitmap("i_male_03");
	m_pSexIcon[2][1] = m_pGuildList->GetBitmap("i_female_03");

	m_pStatusIcon[0] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "play_icon");
	m_pStatusIcon[1] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "dive_icon");
	m_pStatusIcon[2] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "block_icon");
	m_pStatusIcon[3] =
		g_pFresh->GetManager()->GetBitmap("MESSENGER", "busy_icon");
	m_pStatusIcon[4] = NULL;
	m_selectUID = 0xFFFFFFFF;
	UpdateFriendList();

	m_pGuildList->SetVisible(false);

	// 983 00903d60: sem guilda (convites pendentes) o ITEM da lista e 190x30 (+0xe0/+0xe4 = m_itemSize, inteiros)
	if (IsLocalContent((localContentType_t)0x15) && Doc()->m_myInfo.info.dwGuildId == 0)
		m_pGuildList->SetItemSize(0xbe, 0x1e);
}

// messengerdlg.cpp:790
void FrMessengerDlg::OnMessenger_GuildListOwnerDraw(int param)
{
	// 983 008ffe70
	FrListItem* pItem = (FrListItem*)param;
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD) && pFriend->Uid == 0xFFFFFFFF)
	{
		DrawGuildGroup(param);
		return;
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha2 * m_wndAlpha);

	if (IsLocalContent((localContentType_t)0x15))
	{
		if (Doc()->m_myInfo.info.dwGuildId == 0)
		{
			// 983: sem guilda a lista mostra os convites recebidos (sGuildInvite983)
			sGuildInvite983* pInvite = (sGuildInvite983*)pItem->pData;
			if (pInvite->dw4 == 0xFFFFFFFF)
			{
				// "guilds that invited me"
				WRect rcIcon(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f,
					(float)m_pGroupIcon[1]->Width(), (float)m_pGroupIcon[1]->Height());
				pGDI->DrawTexture(m_pGroupIcon[1], rcIcon,
					FrALPHA(0xFFFFFFFF, m_wndAlpha2 * m_wndAlpha), 0);
				pGDI->SetTextStyle(1);
				g_pFresh->GetManager()->PrintText(
					WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0,
					K2L_Compatibility("\xB3\xAA\xB8\xA6 \xC3\xCA\xB4\xEB\xC7\xD1 \xB1\xE6\xB5\xE5"),
					-1.0f, 0xFFFFFFFF);
				pGDI->SetTextStyle(0);
			}
			else
			{
				const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
					pInvite->dw4, pInvite->dw28);
				if (pEmblem != NULL)
				{
					WRect rcEmblem(pItem->pos.x + 32.0f, pItem->pos.y - 1.0f,
						(float)pEmblem->Width(), (float)pEmblem->Height());
					pGDI->DrawTexture(pEmblem, rcEmblem, 0xFFFFFFFF, 0);
				}
				g_pFresh->GetManager()->PrintText(
					WPoint(pItem->pos.x + 60.0f, pItem->pos.y + 4.0f), 0,
					pInvite->szName, 90.0f, 0xFFFFFFFF);
			}

			if (pInvite->dw4 == m_selectUID && pInvite->dw4 != 0xFFFFFFFF)
			{
				pGDI->Box(WRect(pItem->pos.x + 1.0f, pItem->pos.y - 5.0f,
							  (float)m_pGuildList->GetItemWidth(),
							  (float)m_pGuildList->GetItemHeight()),
					0x999B9DFF, 0, 0.0f);
			}
			pGDI->SetAlpha(1.0f);
			return;
		}
		m_pGuildList->SetItemSize(0xbe, 0x14);
	}

	if (!IsLocalContent(S4NT_VIEW_OFFLINE_GUILD) && pFriend->Uid == 0xFFFFFFFF)
	{
		pGDI->SetTextColor(0xFF4C4C4C, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);
		// "no guild friend is connected"
		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 30.0f, pItem->pos.y + 10.0f), 0,
			K2L_Compatibility("\xC1\xA2\xBC\xD3\xC7\xD1 \xB1\xE6\xB5\xE5\xC4\xA3\xB1\xB8\xB0\xA1 \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9"),
			-1.0f, 0xFFFFFFFF);
		pGDI->SetAlpha(1.0f);
		return;
	}

	const Bitmap* pSex = m_pSexIcon[0][pFriend->Gender];
	WRect rect(pItem->pos.x + 1.0f, pItem->pos.y, (float)pSex->Width(),
		(float)pSex->Height());

	if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD))
	{
		if (pFriend->Uid == m_selectUID)
		{
			pGDI->Box(WRect(rect.x, rect.y, (float)m_pGuildList->GetItemWidth(),
						  (float)m_pGuildList->GetItemHeight()),
				((int)(m_wndAlpha * 0.59765625 * m_wndAlpha2 * 255.0) << 24) |
					0x9B9DFF,
				0, 0.0f);
		}
	}
	else if (pFriend->Uid == m_selectUID)
	{
		pGDI->Box(WRect(rect.x, rect.y, (float)m_pGuildList->GetItemWidth(),
					  (float)m_pGuildList->GetItemHeight()),
			0x999B9DFF, 0, 0.0f);
	}

	float iconAlpha = 255.0f;
	if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD) &&
		(!pFriend->IsAccept || !pFriend->IsLogOn))
		iconAlpha = 80.0f;

	pGDI->DrawTexture(pSex, rect,
		((int)(m_wndAlpha2 * m_wndAlpha * iconAlpha) << 24) | 0xFFFFFF, 0);

	MSN::eStatusType status = GetBuddyStatus(pFriend);
	if (status != MSN::STATUS_OFFLINE &&
		(IsLocalContent(S4NT_VIEW_OFFLINE_GUILD) || status != MSN::STATUS_AWAY))
	{
		WRect rcStatus(rect.x, rect.y + 3.0f,
			(float)m_pStatusIcon[status]->Width(),
			(float)m_pStatusIcon[status]->Height());
		pGDI->DrawTexture(m_pStatusIcon[status], rcStatus,
			FrALPHA(0xFFFFFFFF, m_wndAlpha2 * m_wndAlpha), 0);
	}

	if (pItem->underCursor)
	{
		pGDI->SetTextColor(0xFFFFFFFF, 0xFF808080);
		pGDI->SetTextStyle(2);
	}
	else
	{
		if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD))
			pGDI->SetTextColor(0xFF4C4C4C, 0xFFFFFFFF);
		else
			pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);
	}

	// 983: emblema da guilda do amigo (x+20, y+0.3) antes do nome
	const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
		pFriend->dwGuildId, pFriend->emblemVer983);
	if (pEmblem != NULL)
	{
		WRect rcEmblem(pItem->pos.x + 20.0f, pItem->pos.y + 0.3f,
			(float)pEmblem->Width(), (float)pEmblem->Height());
		pGDI->DrawTexture(pEmblem, rcEmblem,
			FrALPHA(0xFFFFFFFF, m_wndAlpha2 * m_wndAlpha), 0);
	}

	g_pFresh->GetManager()->PrintText(
		WPoint(pItem->pos.x + 45.0f, pItem->pos.y + 7.0f), 0, pFriend->NickName,
		90.0f, 0xFFFFFFFF);

	pGDI->SetAlpha(1.0f);

	if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD))
		pGDI->SetTextStyle(0);
}

// messengerdlg.cpp:956
void FrMessengerDlg::OnMessenger_GuildListLBtnDown()
{
	FindNextTopFocus(false);

	m_pRightMenu->SetVisible(false);
	m_pStateMenu->SetVisible(false);

	FrListItem* pItem = m_pGuildList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	// 983 00903f40: sem guilda e com convites pendentes a lista mostra os convites; clicar so seleciona (dw4)
	if (IsLocalContent((localContentType_t)0x15) && Doc()->m_myInfo.info.dwGuildId == 0 &&
		Doc()->m_guildInvites983.size() != 0)
	{
		sGuildInvite983* pInvite = (sGuildInvite983*)pItem->pData;
		if (pInvite == NULL)
			return;
		m_selectUID = pInvite->dw4;
		return;
	}

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (pFriend->Uid == 0xFFFFFFFF)
	{
		if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD))
		{
			if (pFriend->IsLogOn)
			{
				CMessengerInfo::Instance()->m_bFriendOnExpand =
					!CMessengerInfo::Instance()->m_bFriendOnExpand;
				m_groupRow[0].State =
					CMessengerInfo::Instance()->m_bFriendOnExpand;
			}
			else
			{
				CMessengerInfo::Instance()->m_bFriendOffExpand =
					!CMessengerInfo::Instance()->m_bFriendOffExpand;
				m_groupRow[1].State =
					CMessengerInfo::Instance()->m_bFriendOffExpand;
			}
		}

		UpdateFriendList();
		return;
	}

	m_selectUID = pFriend->Uid;
}

// messengerdlg.cpp:1033
void FrMessengerDlg::OnMessenger_GuildListRBtnDown()
{
	OnMessenger_GuildListLBtnDown();

	// 983 00904060: sem guilda (a lista mostra os convites) nao ha menu direito
	if (IsLocalContent((localContentType_t)0x15) && Doc()->m_myInfo.info.dwGuildId == 0)
		return;

	FrListItem* pItem = m_pGuildList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	WPoint pt(g_input->GetMousePoint().x, g_input->GetMousePoint().y);
	m_pRightMenu->MoveWindow(pt);

	if (pFriend->Uid == 0xFFFFFFFF)
		return;

	m_pRightMenu->SetVisible(true);

	m_pRightMenu->UnableMenuItem(
		K2L_Compatibility("\xC4\xA3\xB1\xB8\xC7\xE3\xB0\xA1")); // "accept friend"
	m_pRightMenu->UnableMenuItem(
		K2L_Compatibility("\xBA\xB0\xB8\xED\xC0\xD4\xB7\xC2")); // "enter alias"
	m_pRightMenu->UnableMenuItem(
		K2L_Compatibility("\xB4\xEB\xC8\xAD\xBB\xF3\xB4\xEB\xBB\xE8\xC1\xA6")); // "remove buddy"
	m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2")); // "block"

	if (pFriend->IsLogOn)
	{
		// "send a note" becomes "chat"
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2"),
			K2L_Compatibility("\xB4\xEB\xC8\xAD\xC7\xCF\xB1\xE2"));

		if (IsLocalContent(S4_INVITE_FRIEND) ||
			IsLocalContent(S4_GOWITH_FRIEND))
		{
			if (pFriend->IsPlay || pFriend->IsBusy)
			{
				m_pRightMenu->UnableMenuItem(
					K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2")); // "invite"
				m_pRightMenu->UnableMenuItem(
					K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2")); // "go with"
			}
		}
	}
	else
	{
		m_pRightMenu->ToggleMenuItem(K2L_Compatibility("\xB4\xEB\xC8\xAD\xC7\xCF\xB1\xE2"),
			K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2"));

		if (IsLocalContent(S4_INVITE_FRIEND) ||
			IsLocalContent(S4_GOWITH_FRIEND))
		{
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2"));
		}
	}

	if (pFriend->IsBlock)
	{
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
		m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2"));
	}

	if (IsLocalContent(S4_INVITE_FRIEND))
	{
		if (!m_bEnableInvite)
			m_pRightMenu->UnableMenuItem(K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2"));
	}
}

// messengerdlg.cpp:1120
void FrMessengerDlg::OnMessenger_GuildListDBtnDown()
{
	FrListItem* pItem = m_pGuildList->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	// 983 00906810 (pedido C34): sem guilda e com convites pendentes, a lista da guilda mostra os convites;
	// o duplo clique pergunta se aceita (AskGuildInvite983 -> pacote 0xa8)
	if (IsLocalContent((localContentType_t)0x15) && Doc()->m_myInfo.info.dwGuildId == 0 &&
		Doc()->m_guildInvites983.size() != 0)
	{
		sGuildInvite983* pInvite = (sGuildInvite983*)pItem->pData;
		if (pInvite == NULL)
			return;
		if (pInvite->dw4 == 0xFFFFFFFF)
			return;
		CMessengerInfo::Instance()->AskGuildInvite983(pInvite->szName, pInvite->dw4, false);
		return;
	}

	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pFriend == NULL)
		return;

	if (pFriend->Uid == 0xFFFFFFFF)
		return;

	m_selectUID = pFriend->Uid;

	if (pFriend->IsLogOn)
	{
		OnChatToFriend();
	}
	else
	{
		OnNoteToFriend();
	}
}

// messengerdlg.cpp:1176
void FrMessengerDlg::OnMessenger_RightMenuInit(int param)
{
	m_pRightMenu = (FrContextMenuCtrl*)DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	if (m_pRightMenu != NULL)
	{
		m_pRightMenu->SetVisible(false);
		m_pRightMenu->SetFixed(true);
	}
}

// messengerdlg.cpp:1198
void FrMessengerDlg::OnMessenger_RightMenuLBtnDown()
{
	m_pRightMenu->SetVisible(false);
	FindNextTopFocus(false);

	const char* text = m_pRightMenu->GetSelectMenuText();

	if (strcmp(text, K2L_Compatibility("\xC4\xA3\xB1\xB8\xC7\xE3\xB0\xA1")) == 0)
	{
		OnAcceptFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xB4\xEB\xC8\xAD\xBB\xF3\xB4\xEB\xBB\xE8\xC1\xA6")) ==
		0)
	{
		OnRemoveFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xCF\xB1\xE2")) == 0)
	{
		OnBlockFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xD8\xC1\xA6")) == 0)
	{
		OnBlockCancelFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xBA\xB0\xB8\xED\xC0\xD4\xB7\xC2")) == 0)
	{
		OnAliasToFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xB4\xEB\xC8\xAD\xC7\xCF\xB1\xE2")) == 0)
	{
		OnChatToFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xC2\xCA\xC1\xF6\xBA\xB8\xB3\xBB\xB1\xE2")) == 0)
	{
		OnNoteToFriend();
	}
	else if (strcmp(text, K2L_Compatibility("\xC1\xA4\xBA\xB8\xBA\xB8\xB1\xE2")) == 0)
	{
		OnInfoFriend();
	}
	// PKT (frente C, lote 37): item do 983 (009068d0; texto 00d263e0) logo depois do de informacoes
	else if (strcmp(text, K2L_Compatibility("\xBE\xF7\xC0\xFB\xBA\xB8\xB1\xE2")) == 0)
	{
		OnAchievementFriend983();
	}
	else if (IsLocalContent(S4_GOWITH_FRIEND) &&
		strcmp(text, K2L_Compatibility("\xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2")) == 0)
	{
		// "The go-with-friend feature is being prepared."
		CMessengerInfo::Instance()->Notify1(
			K2L_Compatibility("\xC4\xA3\xB1\xB8 \xB5\xFB\xB6\xF3\xB0\xA1\xB1\xE2 \xB1\xE2\xB4\xC9\xC0\xBA \xC1\xD8\xBA\xF1\xC1\xDF\xC0\xD4\xB4\xCF\xB4\xD9."));
	}
	else if (IsLocalContent(S4_INVITE_FRIEND) &&
		strcmp(text, K2L_Compatibility("\xC3\xCA\xB4\xEB\xC7\xCF\xB1\xE2")) == 0)
	{
		OnInviteFriend();
	}
}

// messengerdlg.cpp:1296
void FrMessengerDlg::OnMessenger_StateMenuInit(int param)
{
	m_pStateMenu = (FrContextMenuCtrl*)DYNAMIC_CAST(FrWnd, (FrWnd*)param);
	if (m_pStateMenu != NULL)
	{
		m_pStateMenu->SetVisible(false);
		m_pStateMenu->SetFixed(true);
	}
}

// messengerdlg.cpp:1316
void FrMessengerDlg::OnMessenger_StateMenuLBtnDown()
{
	FindNextTopFocus(false);

	const char* text = m_pStateMenu->GetSelectMenuText();

	if (strcmp(text, K2L_Compatibility("\xBF\xC2\xB6\xF3\xC0\xCE")) == 0)
	{
		CMessengerInfo::Instance()->SetMyStatus(MSN::STATUS_ONLINE);
	}
	else if (strcmp(text, K2L_Compatibility("\xB4\xD9\xB8\xA5\xBF\xEB\xB9\xAB\xC1\xDF")) == 0)
	{
		CMessengerInfo::Instance()->SetMyStatus(MSN::STATUS_BUSY);
	}
}

// messengerdlg.cpp:1347
void FrMessengerDlg::OnInfoFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL)
		return;

	if (CUserInfo::Instance() != NULL)
	{
		CUserInfo::Instance()->SetInfo(pFriend->Uid, 0, true, true, true, true,
			pFriend->NickName);
	}
}

// PKT (frente C, lote 37): 983 00900c50 - pede as conquistas do amigo selecionado (pacote 0x157 com o UID) e manda a mensagem
// 1 ao ator principal
void FrMessengerDlg::OnAchievementFriend983()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL)
		return;

	WSendPacket packet((enumClientPacket)0x157);
	packet.Encode4(pFriend->Uid);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// messengerdlg.cpp:1379
void FrMessengerDlg::OnNoteToFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	if (Doc()->m_myInfo.stat.i64Pang < 10)
	{
		// "Sending a note needs %d pang."
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", this);
		pForm->SetMessage(
			MakeStr(
				K2L_Compatibility("\xC2\xCA\xC1\xF6\xB8\xA6 \xBA\xB8\xB3\xBB\xB1\xE2 \xC0\xA7\xC7\xD8\xBC\xAD\xB4\xC2 %d\xC6\xCE\xC0\xCC \xC7\xCA\xBF\xE4\xC7\xD5\xB4\xCF\xB4\xD9."),
				10),
			false);
		pForm->SetTimeLimit(5.0f);
		pForm->SetFixed(true);
		pForm->Open((FRESH_PFN_RESULT)0, pt, 3);
		return;
	}

	if (m_pNoteDlg != NULL)
		return;

	m_pNoteDlg =
		CreateForm<FrNoteDlg>(g_pFresh->GetManager(), this, "note", this);
	m_pNoteDlg->SetNick(pFriend->NickName, 0);
	m_pNoteDlg->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	m_pNoteDlg->Open((FRESH_PFN_RESULT)&FrMessengerDlg::OnNoteDlgResult, pt, 1);

	WRect rect = m_pNoteDlg->GetRect();
	m_pNoteDlg->Adjust(rect);
	m_pNoteDlg->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:1423
void FrMessengerDlg::OnChatToFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL)
		return;

	if (pFriend->IsBlock)
	{
		// "You cannot chat with a blocked buddy. Unblock them and try again."
		CMessengerInfo::Instance()->Notify1(
			K2L_Compatibility("\xC2\xF7\xB4\xDC\xC7\xD1 \xBB\xF3\xB4\xEB\xBF\xCD\xB4\xC2 \xB4\xEB\xC8\xAD\xC7\xD2 \xBC\xF6 \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9. \xC2\xF7\xB4\xDC\xC0\xBB \xC7\xD8\xC1\xA6\xC7\xCF\xBD\xC3\xB0\xED \xB4\xD9\xBD\xC3 \xBD\xC3\xB5\xB5\xC7\xCF\xBD\xC3\xB1\xE2 \xB9\xD9\xB6\xF8\xB4\xCF\xB4\xD9."));
		return;
	}

	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;
	int count = chatList.size();

	if (count < 10)
	{
		std::list<sChatSlot>::iterator it;
		for (it = chatList.begin(); it != chatList.end(); ++it)
		{
			if ((*it).pDlg == NULL)
			{
				if (m_selectUID == (*it).uid)
				{
					CMessengerInfo::Instance()->AlarmBtnUp(m_selectUID);
				}
			}

			if ((*it).uid == pFriend->Uid && (*it).pDlg != NULL)
			{
				(*it).pDlg->FindNextTopFocus(false);
				return;
			}
		}

		sChatSlot slot;
		slot.uid = pFriend->Uid;

		slot.pDlg = CreateForm<FrMessengerChatDlg>(g_pFresh->GetManager(), this,
			"messenger_chat", 0);
		slot.pDlg->SetFixed(true);
		slot.pDlg->Open((FRESH_PFN_RESULT)0, 0xC);
		slot.pDlg->SetTopmost(true);
		slot.pDlg->SetUID(slot.uid);
		slot.pDlg->FindNextTopFocus(false);
		chatList.push_back(slot);

		CMessengerInfo::Instance()->RepositionPopupDlg(slot.pDlg, count);
	}
	else
	{
		// "The chat slots are already full."
		CMessengerInfo::Instance()->Notify1(
			K2L_Compatibility("\xC0\xCC\xB9\xCC \xB4\xEB\xC8\xAD \xBD\xBD\xB7\xB5\xC0\xCC \xB2\xCB \xC3\xA1\xBD\xC0\xB4\xCF\xB4\xD9."));
	}
}

// messengerdlg.cpp:1497
void FrMessengerDlg::OnAcceptFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend != NULL)
	{
		WSendPacket packet(0x19);
		packet.Encode4(pFriend->Uid);
		packet.Send(TO_MSN);
	}
}

// messengerdlg.cpp:1525
void FrMessengerDlg::OnRemoveFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);

	if (pFriend == NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", this);
	pForm->SetMessage(
		MakeStr(
			K2L_Compatibility("\\c0xffff0000\\c%s\xB4\xD4\\c0xff000000\\c\xC0\xBB \xC4\xA3\xB1\xB8 \xB8\xAE\xBD\xBA\xC6\xAE\xBF\xA1\xBC\xAD \\c0xffff0000\\c\xBB\xE8\xC1\xA6\\c0xff000000\\c\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			pFriend->NickName),
		false);
	pForm->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	pForm->Open((FRESH_PFN_RESULT)&FrMessengerDlg::OnRemoveFriendDlgResult, pt,
		0);

	WRect rect = pForm->GetRect();
	pForm->Adjust(rect);
	pForm->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:1557
void FrMessengerDlg::OnBlockFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);

	if (pFriend == NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", this);
	pForm->SetMessage(
		MakeStr(
			K2L_Compatibility("\\c0xffff0000\\c%s\xB4\xD4\\c0xff000000\\c\xC0\xBB \xC4\xA3\xB1\xB8 \xB8\xAE\xBD\xBA\xC6\xAE\xBF\xA1\xBC\xAD \\c0xffff0000\\c\xC2\xF7\xB4\xDC\\c0xff000000\\c\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			pFriend->NickName),
		false);
	pForm->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	pForm->Open((FRESH_PFN_RESULT)&FrMessengerDlg::OnBlockFriendDlgResult, pt,
		0);

	WRect rect = pForm->GetRect();
	pForm->Adjust(rect);
	pForm->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:1589
void FrMessengerDlg::OnBlockCancelFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);

	if (pFriend == NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", this);
	pForm->SetMessage(
		MakeStr(
			K2L_Compatibility("\\c0xffff0000\\c%s\xB4\xD4\\c0xff000000\\c\xC0\xC7 \xC2\xF7\xB4\xDC\xC0\xBB \\c0xffff0000\\c\xC7\xD8\xC1\xA6\\c0xff00000\\c\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			pFriend->NickName),
		false);
	pForm->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	pForm->Open((FRESH_PFN_RESULT)&FrMessengerDlg::OnBlockCancelFriendDlgResult,
		pt, 0);

	WRect rect = pForm->GetRect();
	pForm->Adjust(rect);
	pForm->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:1621
void FrMessengerDlg::OnAliasToFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);

	if (pFriend == NULL)
		return;

	if (m_pMemoDlg != NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	m_pMemoDlg =
		CreateForm<FrMemoDlg>(g_pFresh->GetManager(), this, "memo", this);

	m_pMemoDlg->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	m_pMemoDlg->Open((FRESH_PFN_RESULT)&FrMessengerDlg::OnMemoDlgResult, pt, 1);

	WRect rect = m_pMemoDlg->GetRect();
	m_pMemoDlg->Adjust(rect);
	m_pMemoDlg->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:1655
void FrMessengerDlg::DrawFriendGroup(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha * m_wndAlpha2);

	if (pFriend->State == 5)
	{
		pGDI->SetTextColor(0xFF4C4C4C, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);

		char szText[128] = "\0";
		switch (MESSENGER()->m_complete)
		{
		case 0:
			// "not connected"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xC1\xA2\xBC\xD3\xC7\xCF\xC1\xF6 \xBE\xCA\xC0\xBD"));
			break;
		case 1:
			// "login requested"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xB7\xCE\xB1\xD7\xC0\xCE\xC0\xBB \xBF\xE4\xC3\xBB\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9"));
			break;
		case 4:
			wsprintf(szText, "%s", K2L_Compatibility("\xC4\xA3\xB1\xB8 \xBE\xF8\xC0\xBD"));
			break;
		case 3:
			wsprintf(szText, "%s",
				K2L_Compatibility("\xC4\xA3\xB1\xB8\xB8\xAE\xBD\xBA\xC6\xAE \xB9\xDE\xBE\xC6\xBF\xC0\xB4\xC2 \xC1\xDF"));
			break;
		case 2:
			// "login succeeded"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xB7\xCE\xB1\xD7\xC0\xCE\xBF\xA1 \xBC\xBA\xB0\xF8\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9"));
			break;
		}

		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 40.0f, pItem->pos.y + 7.0f), 0, szText, -1.0f,
			0xFFFFFFFF);
	}
	else
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(1);

		unsigned char expand;
		if (pFriend->IsLogOn)
		{
			expand = m_groupRow[0].State;
			// "online(%d)"
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0,
				MakeStr(K2L_Compatibility("\xBF\xC2\xB6\xF3\xC0\xCE(%d)"), m_friendOnCount), -1.0f,
				0xFFFFFFFF);
		}
		else
		{
			expand = m_groupRow[1].State;
			// "offline(%d)"
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0,
				MakeStr(K2L_Compatibility("\xBF\xC0\xC7\xC1\xB6\xF3\xC0\xCE(%d)"),
					m_friendCount - m_friendOnCount),
				-1.0f, 0xFFFFFFFF);
		}

		pGDI->DrawTexture(m_pGroupIcon[expand],
			WRect(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f,
				m_pGroupIcon[expand]->Width(), m_pGroupIcon[expand]->Height()),
			FrALPHA(0xFFFFFFFF, m_wndAlpha * m_wndAlpha2), 0);
	}

	pGDI->SetAlpha(1.0f);
}

// messengerdlg.cpp:1737
void FrMessengerDlg::DrawGuildGroup(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrListItem* pItem = (FrListItem*)param;
	sFriend* pFriend = (sFriend*)pItem->pData;
	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(m_wndAlpha * m_wndAlpha2);

	if (pFriend->State == 5)
	{
		pGDI->SetTextColor(0xFF4C4C4C, 0xFFFFFFFF);
		pGDI->SetTextStyle(0);

		char szText[128] = "\0";
		switch (MESSENGER()->m_complete)
		{
		case 0:
			// "not connected"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xC1\xA2\xBC\xD3\xC7\xCF\xC1\xF6 \xBE\xCA\xC0\xBD"));
			break;
		case 1:
			// "login requested"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xB7\xCE\xB1\xD7\xC0\xCE\xC0\xBB \xBF\xE4\xC3\xBB\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9"));
			break;
		case 4:
			wsprintf(szText, "%s", K2L_Compatibility("\xB1\xE6\xB5\xE5\xBF\xF8 \xBE\xF8\xC0\xBD"));
			break;
		case 3:
			wsprintf(szText, "%s",
				K2L_Compatibility("\xB1\xE6\xB5\xE5\xB8\xAE\xBD\xBA\xC6\xAE \xB9\xDE\xBE\xC6\xBF\xC0\xB4\xC2 \xC1\xDF"));
			break;
		case 2:
			// "login succeeded"
			wsprintf(szText, "%s",
				K2L_Compatibility("\xB7\xCE\xB1\xD7\xC0\xCE\xBF\xA1 \xBC\xBA\xB0\xF8\xC7\xDF\xBD\xC0\xB4\xCF\xB4\xD9"));
			break;
		}

		g_pFresh->GetManager()->PrintText(
			WPoint(pItem->pos.x + 40.0f, pItem->pos.y + 7.0f), 0, szText, -1.0f,
			0xFFFFFFFF);
	}
	else
	{
		pGDI->SetTextColor(0xFF000000, 0xFFFFFFFF);
		pGDI->SetTextStyle(1);

		unsigned char expand;
		if (pFriend->IsLogOn)
		{
			expand = m_groupRow[0].State;
			// "online(%d)"
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0,
				MakeStr(K2L_Compatibility("\xBF\xC2\xB6\xF3\xC0\xCE(%d)"), m_guildOnCount), -1.0f,
				0xFFFFFFFF);
		}
		else
		{
			expand = m_groupRow[1].State;
			// "offline(%d)"
			g_pFresh->GetManager()->PrintText(
				WPoint(pItem->pos.x + 25.0f, pItem->pos.y + 7.0f), 0,
				MakeStr(K2L_Compatibility("\xBF\xC0\xC7\xC1\xB6\xF3\xC0\xCE(%d)"),
					m_guildCount - m_guildOnCount),
				-1.0f, 0xFFFFFFFF);
		}

		pGDI->DrawTexture(m_pGroupIcon[expand],
			WRect(pItem->pos.x + 5.0f, pItem->pos.y + 5.0f,
				m_pGroupIcon[expand]->Width(), m_pGroupIcon[expand]->Height()),
			FrALPHA(0xFFFFFFFF, m_wndAlpha * m_wndAlpha2), 0);
	}

	pGDI->SetAlpha(1.0f);
}

// messengerdlg.cpp:1967
void FrMessengerDlg::OnMessenger_CloseLBtnDown()
{
	GetClientRect(CMessengerInfo::Instance()->m_dlgRect);

	Close(true);

	CMessengerInfo::Instance()->SetGameFocus(true);
}

// messengerdlg.cpp:1827
void FrMessengerDlg::UpdateFriendList()
{
	if (m_pFriendList == NULL)
		return;
	if (m_pGuildList == NULL)
		return;

	std::map<unsigned long, sFriend>* pList = Buddy()->GetBuddyList();
	if (pList == NULL)
		return;

	m_friendOnList.clear();
	m_friendOffList.clear();
	m_guildOnList.clear();
	m_guildOffList.clear();
	m_guildList.clear();

	m_pFriendList->ClearItem();
	m_pGuildList->ClearItem();

	std::map<unsigned long, sFriend>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		sFriend& buddy = it->second;

		if (buddy.IsBlocked)
			buddy.IsLogOn = 0;

		if (buddy.PangyaFriend && buddy.IsAccept && buddy.IsLogOn)
		{
			m_friendOnList.push_back(&buddy);
		}
		else if (buddy.PangyaFriend)
		{
			if (!buddy.IsAccept && !buddy.IsAgree)
				strcpy(buddy.szAlias, K2L_Compatibility("(\xB4\xEB\xB1\xE2\xC1\xDF)"));

			if (buddy.IsAgree && !buddy.IsAccept)
				strcpy(buddy.szAlias, K2L_Compatibility("(\xBF\xE4\xC3\xBB\xC1\xDF)"));

			m_friendOffList.push_back(&buddy);
		}

		if (buddy.GuildFriend && buddy.IsLogOn)
		{
			buddy.dwGuildId = Doc()->m_myInfo.info.dwGuildId;
			buddy.emblemVer983 = Doc()->m_myInfo.info.dwEmblemVer;
			strcpy(buddy.szEmblemName, Doc()->m_myInfo.info.szEmblemName); // 983: Doc+0x7e2
			m_guildList.push_back(&buddy);
			m_guildOnList.push_back(&buddy);
		}
		else if (buddy.GuildFriend && !buddy.IsLogOn)
		{
			m_guildOffList.push_back(&buddy);
		}
	}

	std::list<sFriend*>::iterator li;

	m_pFriendList->AddItem(&m_groupRow[0]);
	if (m_groupRow[0].State)
	{
		for (li = m_friendOnList.begin(); li != m_friendOnList.end(); ++li)
			m_pFriendList->AddItem(*li);

		if (m_friendOnList.size() == 0)
			m_pFriendList->AddItem(&m_groupRow[2]);
	}

	m_pFriendList->AddItem(&m_groupRow[1]);
	if (m_groupRow[1].State)
	{
		for (li = m_friendOffList.begin(); li != m_friendOffList.end(); ++li)
			m_pFriendList->AddItem(*li);

		if (m_friendOffList.size() == 0)
			m_pFriendList->AddItem(&m_groupRow[2]);
	}

	if (IsLocalContent(S4NT_VIEW_OFFLINE_GUILD))
	{
		m_pGuildList->AddItem(&m_groupRow[0]);
		if (m_groupRow[0].State)
		{
			for (li = m_guildOnList.begin(); li != m_guildOnList.end(); ++li)
				m_pGuildList->AddItem(*li);

			if (m_guildOnList.size() == 0)
				m_pGuildList->AddItem(&m_groupRow[2]);
		}

		m_pGuildList->AddItem(&m_groupRow[1]);
		if (m_groupRow[1].State)
		{
			for (li = m_guildOffList.begin(); li != m_guildOffList.end(); ++li)
				m_pGuildList->AddItem(*li);

			if (m_guildOffList.size() == 0)
				m_pGuildList->AddItem(&m_groupRow[2]);
		}
	}
	else
	{
		for (li = m_guildList.begin(); li != m_guildList.end(); ++li)
			m_pGuildList->AddItem(*li);

		if (m_guildList.size() == 0)
			m_pGuildList->AddItem(&m_groupRow[2]);
	}

	m_pFriendList->SortItem(FriendCompare);
	SetCount();
}

// messengerdlg.cpp:1985
void FrMessengerDlg::OnMessenger_FriendBtnInit(int param)
{
	m_pTabBtn[0] = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pTabBtn[0] != NULL)
	{
		m_pTabBtn[0]->SetStatus(FrButton::PRESSED);
	}
}

// messengerdlg.cpp:2005
void FrMessengerDlg::OnMessenger_FriendLBtnDown()
{
	FindNextTopFocus(false);
	RestoreOther(0);

	m_pFriendList->SetVisible(true);
	m_pStateBtn->SetVisible(true);
	m_pGuildList->SetVisible(false);

	m_pNoticeBtn->SetVisible(false);

	// 983 008fedef
	if (IsLocalContent((localContentType_t)0x15) && m_pGuildManageBtn983 != NULL)
		m_pGuildManageBtn983->SetVisible(false);

	m_pRightMenu->SetVisible(false);

	m_pStateMenu->SetVisible(false);
}

// messengerdlg.cpp:2047
void FrMessengerDlg::OnMessenger_GuildBtnInit(int param)
{
	m_pTabBtn[2] = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (Doc()->m_myInfo.info.dwGuildId == 0)
	{
		m_pTabBtn[2]->Enable(false);
	}
}

// messengerdlg.cpp:2066
void FrMessengerDlg::OnMessenger_GuildLBtnDown()
{
	FindNextTopFocus(false);

	// 983 00901960: sem guilda a aba refaz e mostra os convites recebidos, sem notice/guild_manage
	if (IsLocalContent((localContentType_t)0x15))
	{
		if (Doc()->m_myInfo.info.dwGuildId == 0)
			((RbMessengerDlgGuild983*)this)->RefreshGuildInvite983();

		if (Doc()->m_myInfo.info.dwGuildId == 0 && Doc()->m_guildInvites983.size() != 0)
		{
			RestoreOther(1);

			m_pFriendList->SetVisible(false);
			m_pStateBtn->SetVisible(false);
			m_pGuildList->SetVisible(true);
			m_pNoticeBtn->SetVisible(false);

			if (m_pGuildManageBtn983 != NULL)
				m_pGuildManageBtn983->SetVisible(false);

			m_pRightMenu->SetVisible(false);
			m_pStateMenu->SetVisible(false);
			return;
		}
	}

	if (Doc()->m_myInfo.info.dwGuildId != 0)
	{
		RestoreOther(1);

		m_pFriendList->SetVisible(false);
		m_pStateBtn->SetVisible(false);
		m_pGuildList->SetVisible(true);

		// 983 (00901960): o botao de aviso da guilda aparece na aba da guilda
		m_pNoticeBtn->SetVisible(true);

		// 983 00901a82: guild_manage so para grau 1/2 (bits 0-1 do byte depois de dwEmblemVer, Doc+0x832)
		if (IsLocalContent((localContentType_t)0x15) && m_pGuildManageBtn983 != NULL)
		{
			unsigned char grade = ((unsigned char*)&Doc()->m_myInfo.info.dwEmblemVer)[4] & 3;
			if (grade == 1 || grade == 2)
				m_pGuildManageBtn983->SetVisible(true);
		}

		m_pRightMenu->SetVisible(false);

		m_pStateMenu->SetVisible(false);
	}
	else
	{
		RestoreOther(0);
	}

	m_pRightMenu->SetVisible(false);

	m_pStateMenu->SetVisible(false);
}

// messengerdlg.cpp:2156
void FrMessengerDlg::OnMessenger_StateBtnInit(int param)
{
	m_pStateBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// messengerdlg.cpp:2171
void FrMessengerDlg::OnMessenger_StateLBtnDown()
{
	m_pStateMenu->SetVisible(true);
}

// messengerdlg.cpp:2187
void FrMessengerDlg::OnMessenger_GuildNoticeBtnInit(int param)
{
	m_pNoticeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pNoticeBtn != NULL)
	{
		// 983 (008feeb0): so esconde; sem Enable(false)
		m_pNoticeBtn->SetVisible(false);
	}
}

// 983 008fef70: Enable(false) e esconde (a aba guilda mostra para grau 1/2, mas continua desabilitado)
void FrMessengerDlg::OnMessenger_GuildManageBtnInit983(int param)
{
	m_pGuildManageBtn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pGuildManageBtn983 != NULL)
	{
		m_pGuildManageBtn983->Enable(false);
		m_pGuildManageBtn983->SetVisible(false);
	}
}

// 983 008fde30: vazio (so ret)
void FrMessengerDlg::OnMessenger_GuildManageLBtnDown983()
{
}

// messengerdlg.cpp:2210
void FrMessengerDlg::OnMessenger_GuildNoticeLBtnDown()
{
	if (CMessengerInfo::Instance()->m_pGuildNoticeDlg != NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	CMessengerInfo::Instance()->m_pNewsReader = new NewsReader;

	if (NetResourceManager::Instance()->IsRSSDownLoadComplete())
	{
		// the guild news the RSS download saved
		CMessengerInfo::Instance()->m_pNewsReader->Load("guildnews.xml", 5);
	}

	FrMessengerDlg* pOwner = this;
	CMessengerInfo::Instance()->m_pGuildNoticeDlg =
		CreateForm<FrGuildNoticeDlg>(g_pFresh->GetManager(), pOwner,
			"guild_notice", NULL);
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->DataBind(
		CMessengerInfo::Instance()->m_pNewsReader, 0);
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->SetFixed(true);
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->SetTopmost(true);
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->FindNextTopFocus(false);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->Open(
		(FRESH_PFN_RESULT)&FrMessengerDlg::OnGuildNoticeDlgResult, pt, 1);

	WRect rect = CMessengerInfo::Instance()->m_pGuildNoticeDlg->GetRect();
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->Adjust(rect);
	CMessengerInfo::Instance()->m_pGuildNoticeDlg->MoveWindow(
		WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:2253
void FrMessengerDlg::RestoreOther(unsigned long index)
{
	for (unsigned long i = 0; i < 2; i++)
	{
		if (i == index)
		{
			m_pTabBtn[i * 2]->SetStatus(FrButton::PRESSED);
		}
		else
		{
			m_pTabBtn[i * 2]->SetStatus(FrButton::NORMAL);
		}
	}
}

// messengerdlg.cpp:2277
void FrMessengerDlg::OnMessenger_FindLBtnDown()
{
	if (m_pAddFriendDlg != NULL)
		return;

	WPoint pt;
	pt.x = m_rect.x + 60.0f;
	pt.y = m_rect.y + 60.0f;

	m_pAddFriendDlg = CreateForm<FrAddFriendDlg>(g_pFresh->GetManager(), this,
		"addfriend", this);

	m_pAddFriendDlg->SetFixed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950
	m_pAddFriendDlg->Open(
		(FRESH_PFN_RESULT)&FrMessengerDlg::OnAddFriendDlgResult, pt, 1);

	WRect rect = m_pAddFriendDlg->GetRect();
	m_pAddFriendDlg->Adjust(rect);
	m_pAddFriendDlg->MoveWindow(WPoint(rect.x, rect.y));
}

// messengerdlg.cpp:2336
bool FrMessengerDlg::OnNoteDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
		if (pFriend != NULL)
		{
			WSendPacket packet((enumClientPacket)0x3C);
			packet.Encode2(0x111);
			packet.Encode4(pFriend->Uid);
			packet.EncodeStr(std::string(m_pNoteDlg->GetNote()));
			packet.Encode1(0);
			packet.Send(TO_GAME);
		}
	}

	m_pNoteDlg = 0;

	return true;
}

// messengerdlg.cpp:2367
bool FrMessengerDlg::OnAddFriendDlgResult(int ret, FrForm* pForm)
{
	m_pAddFriendDlg = 0;
	return true;
}

// messengerdlg.cpp:2383
// messengerdlg.cpp:2399
bool FrMessengerDlg::OnMemoDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
		if (pFriend != NULL)
		{
			WSendPacket packet(0x1F);
			packet.Encode4(pFriend->Uid);

			packet.EncodeStr(std::string(m_pMemoDlg->GetMemo()));
			packet.Send(TO_MSN);
		}
	}

	m_pMemoDlg = 0;

	return true;
}

// messengerdlg.cpp:2433
bool FrMessengerDlg::OnRemoveFriendDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
		if (pFriend != NULL)
		{
			WSendPacket packet(0x1C);
			packet.Encode4(pFriend->Uid);
			packet.EncodeStr(std::string(pFriend->NickName));

			packet.Send(TO_MSN);
		}

		m_selectUID = 0xFFFFFFFF;
	}

	return true;
}

// messengerdlg.cpp:2467
bool FrMessengerDlg::OnBlockFriendDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
		if (pFriend != NULL)
		{
			WSendPacket packet(0x1A);
			packet.Encode4(pFriend->Uid);
			packet.Send(TO_MSN);
		}
		m_selectUID = 0xFFFFFFFF;
	}

	return true;
}

// messengerdlg.cpp:2500
bool FrMessengerDlg::OnBlockCancelFriendDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
		if (pFriend != NULL)
		{
			WSendPacket packet(0x1B);
			packet.Encode4(pFriend->Uid);
			packet.Send(TO_MSN);
		}
		m_selectUID = 0xFFFFFFFF;
	}

	return true;
}

// messengerdlg.cpp:2533
bool FrMessengerDlg::OnGuildNoticeDlgResult(int ret, FrForm* pForm)
{
	CMessengerInfo::Instance()->m_pGuildNoticeDlg = 0;
	if (CMessengerInfo::Instance()->m_pNewsReader)
	{
		delete CMessengerInfo::Instance()->m_pNewsReader;
		CMessengerInfo::Instance()->m_pNewsReader = 0;
	}

	return true;
}

// messengerdlg.cpp:2562
MSN::eStatusType FrMessengerDlg::GetBuddyStatus(sFriend* pFriend)
{
	if (pFriend->IsBlock)
		return MSN::STATUS_AWAY;

	if (pFriend->IsBusy)
		return MSN::STATUS_BUSY;

	if ((pFriend->IsPlay) && (pFriend->IsLogOn))
		return MSN::STATUS_ACTIVE;

	if ((pFriend->IsDive) && (pFriend->IsLogOn))
		return MSN::STATUS_PLAYING;

	return MSN::STATUS_OFFLINE;
}

// messengerdlg.cpp:2693
void FrMessengerDlg::OnGoWithFriend()
{
	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL)
		return;

	if (pFriend->userPosition.iRoomIdx == 0xFFFF)
	{
		char szText[256] = "\0";
		// "%s is not in a game room, so you cannot follow."
		wsprintf(szText,
			K2L_Compatibility("%s\xB4\xD4\xC0\xCC \xB0\xD4\xC0\xD3\xB9\xE6\xBF\xA1 \xB5\xE9\xBE\xEE\xB0\xA1 \xC0\xD6\xC1\xF6 \xBE\xCA\xBE\xC6 \xB5\xFB\xB6\xF3\xB0\xA5 \xBC\xF6\xB0\xA1 \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9."),
			pFriend->NickName);
		CMessengerInfo::Instance()->Notify1(szText);
	}

	if (pFriend->userPosition.iServerGUID == Doc()->m_gameServerUID)
	{
		WSendPacket packet((enumClientPacket)0xb4);
		packet.Encode1(pFriend->userPosition.iChannelUid);
		packet.Encode2(pFriend->userPosition.iRoomIdx);
		packet.Send(TO_GAME);
	}
	else
	{
		// 983: sem teste de layout; manda 0x104 ao ator principal da task corrente
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xdf), 0, 0, 0, 0, 0));

		Doc()->m_gameServerList.clear();
		Doc()->m_channelList.clear();

		WSendPacket packet((enumClientPacket)0x43);
		packet.Send(TO_GAME);

		CSharedDoc::Instance()->SetGoWithModeStart(m_selectUID);
	}
}

// messengerdlg.cpp:2590
void FrMessengerDlg::SetCount()
{
	std::map<unsigned long, sFriend>* pList = Buddy()->GetBuddyList();

	if (pList == NULL)
		return;

	m_friendCount = 0;
	m_friendOnCount = 0;
	m_guildCount = 0;
	m_guildOnCount = 0;

	std::map<unsigned long, sFriend>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		if (it->second.PangyaFriend)
			m_friendCount++;
		if (it->second.GuildFriend)
			m_guildCount++;

		if (it->second.IsLogOn)
		{
			if ((it->second.PangyaFriend) && (it->second.IsAccept))
				m_friendOnCount++;
			if (it->second.GuildFriend)
				m_guildOnCount++;
		}
	}
}

void FrMessengerDlg::OnInviteFriend()
{
	if (!IsLocalContent(S4_INVITE_FRIEND))
		return;

	sFriend* pFriend = Buddy()->GetBuddy(m_selectUID);
	if (pFriend == NULL || !pFriend->IsLogOn || !pFriend->IsAccept)
		return;

	if (!CSharedDoc::Instance()->IsInviteMode())
	{
		CSharedDoc::Instance()->SetFriendGameSvrUID(
			pFriend->userPosition.iServerGUID);
		CSharedDoc::Instance()->SetInviteMode(1);

		WSendPacket packet((enumClientPacket)0xba);
		packet.EncodeStr(pFriend->NickName);
		packet.Encode4(pFriend->Uid);
		packet.Send(TO_GAME);
	}
	else
	{
		// "An invitation is already pending. Please try again later."
		AfxGetTask()->GetMainActor() << MsgObject(0, M983(0x23),
			(int)K2L_Compatibility("\xC3\xCA\xB4\xEB \xBF\xE4\xC3\xBB\xC1\xDF\xBF\xA1 \xC0\xD6\xBD\xC0\xB4\xCF\xB4\xD9. \xC0\xE1\xBD\xC3\xC8\xC4 \xB4\xD9\xBD\xC3 \xC0\xCC\xBF\xEB\xC7\xCF\xBC\xBC\xBF\xE4."),
			0xff000000, 0, 0, 0);
	}
}

// messengerdlg.cpp:2803
CMessengerInfo::CMessengerInfo()
	: m_pMessengerDlg(0), m_notifyTime(0), m_bNotify(false)
{
	C_ASSERT_VC10(offsetof(CMessengerInfo, m_pMessengerDlg) == 0x94); // 983 CMouseCursor::Display
	m_dlgRect.x = 200.0f;
	m_dlgRect.y = 100.0f;
	m_dlgRect.w = 220.0f;
	m_dlgRect.h = 300.0f;

	m_alarmRect.x = 663.0f;
	m_alarmRect.y = 0.0f;
	m_alarmRect.w = 38.0f;
	m_alarmRect.h = 32.0f;

	m_myStatus = MSN::STATUS_ONLINE;
	m_pGuildNoticeDlg = 0;
	m_pNewsReader = 0;
	m_checkTime = 0;

	memset(&m_bFriendOnExpand, 1, 3);

	m_popupIndex = 0;
	m_complete = (eConnectStatus)0;
	m_bAutoAway = false;

	m_chatSlotList.clear();
	m_focusList.clear();

	m_serverList.clear();

	m_bWaitServerList = false;
	m_curServerIndex = 0;
	if (IsLocalContent(S4_GOWITH_FRIEND))
	{
		CSharedDoc::Instance()->SetGoWithModeEnd();
	}

	if (IsLocalContent(S4_INVITE_FRIEND))
		m_bEnableInvite = false;
}

// The last messenger connection attempts (CMessengerInfo::Open allows five per
// minute). No symbols in the PDB; the names are invented.
static int s_connectCount;
static unsigned long s_connectHistory[5]; // GetTickCount of each attempt

// messengerdlg.cpp:2852
void CMessengerInfo::Open(bool)
{
	if (WNetworkSystem::Instance()
				->GetSocket(WNetworkSystem::NET_MSN)
				->m_bConnected != 0 &&
		WNetworkSystem::Instance()->IsConnectionComplete(
			WNetworkSystem::NET_MSN))
	{
		Toggle();
		return;
	}

	// "Connecting to the messenger server...."
	Notify2(
		K2L_Compatibility("\xB8\xDE\xBD\xC5\xC0\xFA\xBC\xAD\xB9\xF6\xBF\xA1 \xC1\xA2\xBC\xD3\xC1\xDF...."));

	unsigned long now = GetTickCount();

	if (s_connectCount >= 5)
	{
		if (now - s_connectHistory[4] > 90000)
		{
			s_connectCount = 0;
			s_connectHistory[0] = 0;
		}
		else
		{
			// "Please try again later."
			Notify2(
				K2L_Compatibility("\xC0\xE1\xBD\xC3 \xC8\xC4\xBF\xA1 \xC0\xCC\xBF\xEB\xC7\xD8 \xC1\xD6\xBD\xCA\xBD\xC3\xBF\xC0."));
			return;
		}
	}

	if ((!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
			!WNetworkSystem::Instance()->IsConnectionComplete(
				WNetworkSystem::NET_MSN)) ||
		WNetworkSystem::Instance()->IsConnectionComplete(
			WNetworkSystem::NET_MSN) == 1)
	{
		WNetworkSystem::Instance()->Init(WNetworkSystem::NET_MSN);
		WNetworkSystem::Instance()->SendMessage(WNetworkSystem::NET_MSN,
			NetworkUnit::eEVENT_NEXT);

		if (now - s_connectHistory[0] < 60000)
		{
			s_connectHistory[s_connectCount] = now;
			s_connectCount++;
			return;
		}

		int i;
		for (i = 0; i < s_connectCount; i++)
		{
			if (now - s_connectHistory[i] < 60000)
				break;
		}

		if (i < s_connectCount)
		{
			int count = s_connectCount - i;

			for (int j = 0; j < count; j++)
				s_connectHistory[j] = s_connectHistory[i + j];

			s_connectHistory[count] = now;
			s_connectCount = count + 1;
		}
		else
		{
			s_connectHistory[0] = now;
			s_connectCount = 1;
		}
	}
}

// messengerdlg.cpp:2984
bool CMessengerInfo::Close(bool bDestroy)
{
	if (m_pGuildNoticeDlg != NULL)
		m_pGuildNoticeDlg->Close(true);

	if (!bDestroy && !IsVisible())
		return true;

	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;
	std::list<sChatSlot>::iterator it;
	for (it = chatList.begin(); it != chatList.end();)
	{
		if ((*it).pDlg != NULL)
		{
			if (!(*it).pDlg->IsTopFocus())
			{
				++it;
				continue;
			}

			if (!(*it).pDlg->Close(bDestroy))
				return false;
		}

		if ((*it).bAlarm && !bDestroy)
		{
			++it;
			continue;
		}

		it = chatList.erase(it);

		if (!bDestroy)
			return false;
	}

	if (m_pMessengerDlg != NULL)
	{
		m_pMessengerDlg->GetClientRect(m_dlgRect);
		m_pMessengerDlg->Adjust(m_dlgRect);
		m_pMessengerDlg->SetFadeout(true);

		return false;
	}

	if (bDestroy)
	{
		CSharedDoc::Instance()->m_bMessengerAlarm = false;
		CSharedDoc::Instance()->m_bRefreshCamera = false;
	}

	return true;
}

// messengerdlg.cpp:3067
void CMessengerInfo::CloseWindow()
{
	SaveCurrentFocus();

	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;
	std::list<sChatSlot>::iterator it;
	for (it = chatList.begin(); it != chatList.end(); ++it)
	{
		if ((*it).pDlg != NULL)
		{
			(*it).pDlg->GetClientRect((*it).rect);
			(*it).pDlg->Adjust((*it).rect);
			(*it).pDlg->Close(true);
			(*it).pDlg = 0;
		}
	}

	if (m_pMessengerDlg != NULL)
	{
		m_pMessengerDlg->GetClientRect(m_dlgRect);
		m_pMessengerDlg->Adjust(m_dlgRect);
		m_pMessengerDlg->Close(true);
		m_pMessengerDlg = 0;
	}
}

// messengerdlg.cpp:3108
void CMessengerInfo::Toggle()
{
	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;

	bool bVisible = IsVisible() ? true : false;
	bool bChatRestored = false;

	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		SetGameFocus(true);

	if (bVisible == true)
	{
		SaveCurrentFocus();

		std::list<sChatSlot>::iterator it;
		for (it = chatList.begin(); it != chatList.end(); ++it)
		{
			sChatSlot& slot = *it;
			if (slot.pDlg != NULL)
			{
				slot.pDlg->GetClientRect(slot.rect);
				slot.pDlg->SaveTempChat(slot);
				slot.pDlg->SetFadeout(true);
			}
		}

		if (m_pMessengerDlg != NULL)
		{
			m_pMessengerDlg->GetClientRect(m_dlgRect);
			m_pMessengerDlg->SetFadeout(true);
		}
	}
	else
	{
		Notify4(false);

		FrWnd* pLastChat = NULL;

		if (m_focusList.size() > 0)
		{
			std::list<unsigned long>::iterator itFocus;
			for (itFocus = m_focusList.begin(); itFocus != m_focusList.end();
				++itFocus)
			{
				unsigned long uid = *itFocus;

				if (uid == 0xFFFFFFFF)
				{
					if (m_pMessengerDlg == NULL)
					{
						m_pMessengerDlg = CreateForm<FrMessengerDlg>(
							g_pFresh->GetManager(), this, "messenger", 0);
						if (m_pMessengerDlg != NULL)
						{
							if (!IS_KINDOF(CGolfTask, AfxGetTask()))
								m_pMessengerDlg->Adjust(m_dlgRect);

							m_pMessengerDlg->SetFixed(true);
							m_pMessengerDlg->Open(
								(FRESH_PFN_RESULT)&CMessengerInfo::
									OnMessengerDlgResult,
								m_dlgRect.TopLeft(), 0xC);
							m_pMessengerDlg->SetTopmost(true);
							m_pMessengerDlg->SetFadeout(bVisible);

							if (IsLocalContent(S4_INVITE_FRIEND))
								m_pMessengerDlg->EnableInvite(m_bEnableInvite);
						}
					}
				}
				else
				{
					std::list<sChatSlot>::iterator it;
					for (it = chatList.begin(); it != chatList.end(); ++it)
					{
						sChatSlot& slot = *it;
						if (!slot.bAlarm && slot.pDlg == NULL &&
							slot.uid == uid)
						{
							slot.pDlg = CreateForm<FrMessengerChatDlg>(
								g_pFresh->GetManager(), this, "messenger_chat",
								0);

							if (!IS_KINDOF(CGolfTask, AfxGetTask()))
								slot.pDlg->Adjust(slot.rect);

							slot.pDlg->SetFixed(true);
							slot.pDlg->Open((FRESH_PFN_RESULT)0,
								slot.rect.TopLeft(), 0xC);
							slot.pDlg->SetUID(slot.uid);
							slot.pDlg->SetTopmost(true);
							slot.pDlg->SetFadeout(bVisible);

							bChatRestored = true;
							slot.pDlg->RestoreChat(slot);

							pLastChat = slot.pDlg;
						}
					}
				}
			}

			if (m_pMessengerDlg == NULL)
			{
				m_pMessengerDlg = CreateForm<FrMessengerDlg>(
					g_pFresh->GetManager(), this, "messenger", 0);
				if (m_pMessengerDlg != NULL)
				{
					if (!IS_KINDOF(CGolfTask, AfxGetTask()))
						m_pMessengerDlg->Adjust(m_dlgRect);

					m_pMessengerDlg->SetFixed(true);
					m_pMessengerDlg->Open(
						(FRESH_PFN_RESULT)&CMessengerInfo::OnMessengerDlgResult,
						m_dlgRect.TopLeft(), 0xC);
					m_pMessengerDlg->SetTopmost(true);
					m_pMessengerDlg->SetFadeout(bVisible);
					m_pMessengerDlg->GetClientRect(m_dlgRect);

					if (IsLocalContent(S4_INVITE_FRIEND))
						m_pMessengerDlg->EnableInvite(m_bEnableInvite);
				}
			}

			if (pLastChat != NULL)
				pLastChat->FindNextTopFocus(true);
		}
		else
		{
			if (m_pMessengerDlg == NULL)
			{
				m_pMessengerDlg = CreateForm<FrMessengerDlg>(
					g_pFresh->GetManager(), this, "messenger", 0);
				if (m_pMessengerDlg != NULL)
				{
					if (!IS_KINDOF(CGolfTask, AfxGetTask()))
						m_pMessengerDlg->Adjust(m_dlgRect);

					m_pMessengerDlg->SetFixed(true);
					m_pMessengerDlg->Open(
						(FRESH_PFN_RESULT)&CMessengerInfo::OnMessengerDlgResult,
						m_dlgRect.TopLeft(), 0xC);
					m_pMessengerDlg->SetTopmost(true);
					m_pMessengerDlg->SetFadeout(bVisible);
					m_pMessengerDlg->GetClientRect(m_dlgRect);

					if (m_pMessengerDlg != NULL)
						m_pMessengerDlg->UpdateFriendList();

					if (IsLocalContent(S4_INVITE_FRIEND))
						m_pMessengerDlg->EnableInvite(m_bEnableInvite);
				}
			}
		}

		m_focusList.clear();
	}

	if (IS_KINDOF(CGolfTask, AfxGetTask()) && !CChatMsg::Instance()->IsOpened())
	{
		g_pFresh->GetManager()->SetExclusiveKey(!bVisible && bChatRestored);
	}

	g_audio->PlaySfx("ui_icon_boing2", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// messengerdlg.cpp:3321
void CMessengerInfo::SaveCurrentFocus()
{
	std::string name;

	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;
	FrWndManager* pManager = g_pFresh->GetManager();

	std::list<FrWnd*>::iterator it;
	for (it = pManager->GetTopWndList().begin();
		it != pManager->GetTopWndList().end(); ++it)
	{
		FrWnd* pWnd = *it;

		if (IS_KINDOF(FrForm, pWnd))
		{
			FrForm* pForm = (FrForm*)pWnd;
			pForm->GetWindowName(name);

			if (strcmp(name.c_str(), "messenger") == 0)
			{
				m_focusList.push_back((unsigned long)-1);
			}
			else if (strcmp(name.c_str(), "messenger_chat") == 0)
			{
				std::list<sChatSlot>::iterator itSlot;
				for (itSlot = chatList.begin(); itSlot != chatList.end();
					++itSlot)
				{
					if ((*itSlot).pDlg == pForm)
						m_focusList.push_back((*itSlot).uid);
				}
			}
		}
	}
}

// messengerdlg.cpp:3372
void CMessengerInfo::ApplySavedFocus()
{
	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;

	std::list<unsigned long>::iterator it;
	for (it = m_focusList.begin(); it != m_focusList.end(); ++it)
	{
		unsigned long uid = *it;

		if (uid == 0xFFFFFFFF)
		{
			if (m_pMessengerDlg != NULL)
			{
				m_pMessengerDlg->FindNextTopFocus(false);
			}
		}
		else
		{
			std::list<sChatSlot>::iterator chat;
			for (chat = chatList.begin(); chat != chatList.end(); ++chat)
			{
				if ((*chat).uid == uid)
				{
					if ((*chat).pDlg != NULL)
					{
						(*chat).pDlg->FindNextTopFocus(false);
					}
				}
			}
		}
	}
}

// messengerdlg.cpp:3417
void CMessengerInfo::Process(float dt)
{
	if (CMessengerInfo::Instance()->m_complete != 4)
	{
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
			WNetworkSystem::Instance()->IsConnectionComplete(
				WNetworkSystem::NET_MSN) &&
			WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			m_checkTime += dt;

			if (m_checkTime > 60.0f)
			{
				m_checkTime = 0.0f;

				WNetworkSystem::Instance()->ForceShutDown(
					WNetworkSystem::NET_MSN);
				Close(true);
				WNetworkSystem::Instance()->Init(WNetworkSystem::NET_MSN);
				WNetworkSystem::Instance()->SendMessage(WNetworkSystem::NET_MSN,
					NetworkUnit::eEVENT_NEXT);
			}
		}
		else if (!WNetworkSystem::Instance()->IsConnected(
					 WNetworkSystem::NET_MSN))
		{
			m_checkTime = 0.0f;
		}
	}

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
		WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
		CMessengerInfo::Instance()->GetMyStatus() != MSN::STATUS_BUSY)
	{
		bool bChanged = false;

		if (m_bAutoAway)
		{
			if (g_mouse->IsUpdated() || g_ime->IsUpdated())
			{
				m_bAutoAway = false;
				bChanged = true;
			}
		}
		else if (timeGetTime() - g_mouse->GetLastInputTime() > 120000 &&
			timeGetTime() - g_ime->GetLastInputTime() > 120000)
		{
			m_bAutoAway = true;
			bChanged = true;
		}

		if (bChanged)
		{
			if (m_bAutoAway)
				CMessengerInfo::Instance()->SetMyStatus(MSN::STATUS_PLAYING);
			else
				CMessengerInfo::Instance()->SetMyStatus(MSN::STATUS_ONLINE);
		}
	}

	if (IS_KINDOF(CGolfTask, AfxGetTask()) &&
		WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN))
	{
		if (g_input->GetButton(LEFT_BUTTON) == 1 &&
			!CChatMsg::Instance()->IsOpened())
		{
			g_pFresh->GetManager()->SetExclusiveKey(false);
			SetGameFocus(true);
		}

		if (g_input->GetButton(RIGHT_BUTTON) == 1 ||
			g_input->GetButton(LEFT_BUTTON) == 1)
		{
			const WPoint& pt = g_pFresh->GetManager()->GetMousePos();
			WRect rc;

			bool bOutside = true;
			std::list<sChatSlot>::iterator it;
			for (it = CMessengerInfo::Instance()->m_chatSlotList.begin();
				it != CMessengerInfo::Instance()->m_chatSlotList.end(); ++it)
			{
				if ((*it).pDlg != NULL)
				{
					(*it).pDlg->GetClientRect(rc);

					if (rc.IsInRect(pt))
						bOutside = false;

					if ((*it).pDlg->GetEmoticonClientRect(rc))
					{
						if (rc.IsInRect(pt))
							bOutside = false;
					}
				}
			}

			if (m_pMessengerDlg != NULL)
			{
				m_pMessengerDlg->GetClientRect(rc);

				if (rc.IsInRect(pt))
					bOutside = false;
			}

			if (bOutside)
			{
				SetGameFocus(true);
			}
			else
			{
				SetGameFocus(false);
			}
		}
	}

	if (m_bNotify)
	{
		m_notifyTime += dt;

		if (m_notifyTime > 4.0f)
		{
			if (IS_KINDOF(CLobbyTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("Lobby")
					<< MsgObject(0, M983(0xa8), 0, 0, 0, 0, 0);
			}
			else if (IS_KINDOF(CFamilyTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("Family")
					<< MsgObject(0, M983(0xa8), 0, 0, 0, 0, 0);
			}
			else if (IS_KINDOF(CGolfTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("GolfRule")
					<< MsgObject(0, M983(0xa8), 0, 0, 0, 0, 0);
			}
			else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
			{
				AfxGetTask()->GetActor("AvatarChat")
					<< MsgObject(0, M983(0xa8), 0, 0, 0, 0, 0);
			}

			m_bNotify = false;
			m_notifyTime = 0.0f;
		}
	}
}

// messengerdlg.cpp:3609
bool CMessengerInfo::OnMessengerDlgResult(int ret, FrForm* pForm)
{
	m_pMessengerDlg = 0;
	return true;
}

// messengerdlg.cpp:3625
bool CMessengerInfo::OnMessengerChatDlgResult(int ret, FrForm* pForm)
{
	std::list<sChatSlot>::iterator it;
	for (it = m_chatSlotList.begin(); it != m_chatSlotList.end();)
	{
		if ((*it).pDlg == pForm)
		{
			(*it).pDlg = 0;
		}
		++it;
	}

	return true;
}

// 983 009048b0 (pedido C30; declaracao da A s101): pergunta se aceita o convite da guilda "name" (bApply: texto de pedido
// de entrada). O resultado vai ao OnGuildInviteDlgResult983, que manda o pacote 0xa8.
void CMessengerInfo::AskGuildInvite983(std::string name, unsigned long key, bool bApply)
{
	m_inviteGuildName983 = name;
	m_inviteKey983 = key;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", m_pMessengerDlg);
	if (!bApply)
	{
		// "a guilda %s convidou voce como membro. Aceita?"
		pForm->SetMessage(MakeStr(K2L_Compatibility("%s \xB1\xE6\xB5\xE5\xBF\xA1\xBC\xAD \xB1\xE6\xB5\xE5\xBF\xF8\xC0\xB8\xB7\xCE \xC3\xCA\xB4\xEB\xC7\xCF\xBF\xB4\xBD\xC0\xB4\xCF\xB4\xD9.\n\n\xBD\xC2\xB3\xAB\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			name.c_str()), false);
	}
	else
	{
		// "aceita o convite da guilda %s?"
		pForm->SetMessage(MakeStr(K2L_Compatibility("%s \xB1\xE6\xB5\xE5\xC0\xC7 \xC3\xCA\xB4\xEB\xBF\xA1 \xC0\xC0\xC7\xCF\xBD\xC3\xB0\xDA\xBD\xC0\xB4\xCF\xB1\xEE?"),
			name.c_str()), false);
	}
	pForm->SetFixed(true);

	WPoint pt;
	pt.x = 0.0f;
	pt.y = 0.0f;
	FrMessengerDlg* pDlg = m_pMessengerDlg;
	if (pDlg != NULL)
	{
		pt.x = pDlg->GetRect().x + 60.0;
		pt.y = pDlg->GetRect().y + 60.0;
	}
	else
	{
		pt.x = 100.0f;
		pt.y = 60.0f;
	}
	pForm->Open((FRESH_PFN_RESULT)&CMessengerInfo::OnGuildInviteDlgResult983, pt, 1);
}

// 983 00901b60: resposta ao convite (AskGuildInvite983) -> pacote 0xa8 (chave, id do convite, aceitou)
bool CMessengerInfo::OnGuildInviteDlgResult983(int result, FrForm* pForm)
{
	bool bAccept = (result == 1);

	std::vector<sGuildInvite983>& inviteList = Doc()->m_guildInvites983;
	std::vector<sGuildInvite983>::iterator it;
	for (it = inviteList.begin(); it != inviteList.end(); ++it)
	{
		if ((*it).dw4 == m_inviteKey983)
			break;
	}
	if (it == inviteList.end())
		return false;

	unsigned long inviteId = (*it).dw0;
	if (inviteId == 0xffffffff)
		return false;

	WSendPacket packet((enumClientPacket)0xa8);
	packet.Encode4(m_inviteKey983);
	packet.Encode4(inviteId);
	packet.Encode1(bAccept);
	packet.Send(TO_GAME);

	return true;
}

// messengerdlg.cpp:3680
void CMessengerInfo::Notify1(const char* msg)
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
	if (pForm != NULL)
	{
		pForm->SetMessage(msg, false);
		pForm->Open((FRESH_PFN_RESULT)0, 3);
		pForm->SetTopmost(true);
		pForm->FindNextTopFocus(false);
	}
}

// messengerdlg.cpp:3702
void CMessengerInfo::Notify2(const char* msg)
{
	if (IS_KINDOF(CLobbyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(0, M983(0xa8), (intptr_t)msg, 1, 0, 0, 0);
	}
	else if (IS_KINDOF(CFamilyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Family")
			<< MsgObject(0, M983(0xa8), (intptr_t)msg, 1, 0, 0, 0);
	}
	else if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(0, M983(0xa8), (intptr_t)msg, 1, 0, 0, 0);
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(0, M983(0xa8), (intptr_t)msg, 1, 0, 0, 0);
	}

	m_notifyTime = 0.0f;
	m_bNotify = true;
}

// messengerdlg.cpp:3747
void CMessengerInfo::Notify3()
{
	// 983 00900fd0: aviso em CSharedDoc+0x10d81; so age se ainda desligado
	if (CSharedDoc::Instance()->m_bMessengerAlarm)
		return;
	CSharedDoc::Instance()->m_bMessengerAlarm = true;

	if (IS_KINDOF(CLobbyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(0, M983(0xa9), 1, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CFamilyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Family") << MsgObject(0, M983(0xa9), 1, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("GolfRule") << MsgObject(0, M983(0xa9), 1, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(0, M983(0xa9), 1, 0, 0, 0, 0);
	}

	g_audio->PlaySfx("notice_dingdong");
}

// messengerdlg.cpp:3797
void CMessengerInfo::Notify4(bool bAlarm)
{
	// 983 00901220: CSharedDoc+0x10d82; sem mudanca, nada a fazer
	if (CSharedDoc::Instance()->m_bMessengerInit == bAlarm)
		return;
	CSharedDoc::Instance()->m_bMessengerInit = bAlarm;

	if (IS_KINDOF(CLobbyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Lobby")
			<< MsgObject(0, M983(0xac), bAlarm, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CFamilyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Family")
			<< MsgObject(0, M983(0xac), bAlarm, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(0, M983(0xac), bAlarm, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(0, M983(0xac), bAlarm, 0, 0, 0, 0);
	}
}

// messengerdlg.cpp:3837
void CMessengerInfo::AlarmBtnUp(unsigned long uid)
{
	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;
	int count = chatList.size();

	std::list<sChatSlot>::iterator it;
	for (it = chatList.begin(); it != chatList.end(); ++it)
	{
		// 983 (00902340): campos lidos pela referencia do slot
		sChatSlot& slot = *it;
		if (slot.pDlg == NULL && slot.bAlarm &&
			(uid == 0xFFFFFFFF || uid == slot.uid))
		{
			slot.bAlarm = false;

			slot.pDlg = CreateForm<FrMessengerChatDlg>(g_pFresh->GetManager(),
				this, "messenger_chat", 0);
			slot.pDlg->SetFixed(true);
			slot.pDlg->Open((FRESH_PFN_RESULT)0, 0xC);
			slot.pDlg->SetUID(slot.uid);
			slot.pDlg->SetTopmost(true);
			slot.pDlg->FindNextTopFocus(false);
			slot.pDlg->RestoreChat(slot);

			RepositionPopupDlg(slot.pDlg, count);
		}
	}

	CSharedDoc::Instance()->m_bMessengerAlarm = false;

	if (IS_KINDOF(CLobbyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(0, M983(0xa9), 0, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CFamilyTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("Family") << MsgObject(0, M983(0xa9), 0, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("GolfRule") << MsgObject(0, M983(0xa9), 0, 0, 0, 0, 0);
	}
	else if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		AfxGetTask()->GetActor("AvatarChat")
			<< MsgObject(0, M983(0xa9), 0, 0, 0, 0, 0);
	}
}

// messengerdlg.cpp:3909
void CMessengerInfo::RecvChat(sFriend* pFriend, std::string msg)
{
	std::list<sChatSlot>& chatList = CMessengerInfo::Instance()->m_chatSlotList;

	if ((int)chatList.size() < 10)
	{
		sMsnChat chat;

		std::list<sChatSlot>::iterator it;
		for (it = chatList.begin(); it != chatList.end(); ++it)
		{
			if ((*it).uid == pFriend->Uid)
			{
				chat.nick = pFriend->NickName;
				chat.msg = msg;
				(*it).chatList.push_back(chat);

				if ((*it).pDlg)
				{
					(*it).pDlg->AddChatLine(chat.nick, chat.msg);
					if (!(*it).pDlg->IsTopFocus())
						(*it).pDlg->Vibration(1.0f);
				}

				if (!(*it).bAlarm)
				{
					if ((*it).pDlg == NULL)
						Notify4(true);
				}
				else if ((*it).pDlg == NULL && CLobbyState983::Instance983() != NULL)
				{
					// 983 (00906569): com o lobby ligado, avisa o ator "Lobby" (msg 0xd1, param1 = 1)
					AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0xac), 1, 0, 0, 0, 0);
				}

				// 983 (009061e0): achou = sai direto; o fim do laco cai no slot novo sem testar end()
				return;
			}
		}

		{
			sChatSlot slot;
			sMsnChat newChat;
			slot.uid = pFriend->Uid;
			slot.bAlarm = true;
			newChat.nick = pFriend->NickName;
			newChat.msg = msg;
			slot.chatList.push_back(newChat);
			chatList.push_back(slot);

			Notify3();
		}
	}
	else
	{
		// "The other user cannot answer." -- 983 (ramo >= 10 de 009061e0): texto passa por K2L_Compatibility
		WSendPacket packet(0x1E);
		packet.Encode4(pFriend->Uid);
		packet.EncodeStr(std::string(K2L_Compatibility(
			"\xBB\xF3\xB4\xEB\xB9\xE6\xC0\xCC \xC0\xC0\xB4\xE4\xC7\xD2 \xBC\xF6 \xBE\xF8\xBD\xC0\xB4\xCF\xB4\xD9.")));
		packet.Send(TO_MSN);
	}
}

// messengerdlg.cpp:3994
void CMessengerInfo::SetOfflineUserNick(const char* nick, unsigned long uid)
{
	if (m_pMessengerDlg != NULL)
	{
		m_pMessengerDlg->SetAddFriendNick(nick, uid);
	}
}

// messengerdlg.cpp:4016
void CMessengerInfo::SetTargetNick(const char* nick)
{
	if (m_pMessengerDlg != NULL)
	{
		m_pMessengerDlg->SetTargetNick(nick);
	}
}

// messengerdlg.cpp:4035
void CMessengerInfo::Refresh()
{
	if (m_pMessengerDlg != NULL)
	{
		m_pMessengerDlg->UpdateFriendList();
	}
}

// messengerdlg.cpp:4053
void CMessengerInfo::SetMyStatus(MSN::eStatusType status)
{
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
		CMessengerInfo::Instance()->m_complete == 4)
	{
		m_myStatus = status;
		WSendPacket packet(0x1D);
		packet.Encode1((unsigned char)m_myStatus);
		packet.Send(TO_MSN);
	}
}

// messengerdlg.cpp:4078
bool CMessengerInfo::IsVisible()
{
	std::list<sChatSlot>::iterator it;
	for (it = CMessengerInfo::Instance()->m_chatSlotList.begin();
		it != CMessengerInfo::Instance()->m_chatSlotList.end(); ++it)
	{
		if ((*it).pDlg != NULL)
		{
			return true;
		}
	}

	return m_pMessengerDlg ? true : false;
}

// messengerdlg.cpp:4113
void CMessengerInfo::RepositionPopupDlg(FrMessengerChatDlg* pDlg, int index)
{
	if (pDlg == NULL)
		return;

	if (index == 0)
	{
		m_popupIndex = 1;
		return;
	}

	if (index <= 0)
		return;

	WRect rc = pDlg->GetRect();

	// 983 (008ff0a0): 0 comparado primeiro (fldz) e a largura livre recalculada no produto
	if (0.0f >= g_view->GetWidth() - rc.w)
		return;
	if (0.0f >= g_view->GetHeight() - rc.h)
		return;

	float offset = (float)(m_popupIndex * 20);

	WPoint pt(rc.x + offset * (g_view->GetWidth() - rc.w) / (g_view->GetHeight() - rc.h), offset + rc.y);
	if ((int)(g_view->GetWidth() - rc.w) > 0)
		pt.x = (float)((int)pt.x % (int)(g_view->GetWidth() - rc.w));
	if ((int)(g_view->GetHeight() - rc.h) > 0)
		pt.y = (float)((int)pt.y % (int)(g_view->GetHeight() - rc.h));
	pDlg->MoveWindow(pt);
	m_popupIndex++;
}

// messengerdlg.cpp:4151
void CMessengerInfo::SetGameFocus(bool focus)
{
	m_bGameFocus = focus;

	if (focus)
	{
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
		{
			g_pFresh->GetManager()->ResetTopFocus();
		}
	}
}

// messengerdlg.cpp:4165
void CMessengerInfo::InvalidateCurrentServer()
{
	int index = 0;
	std::list<MSNServerInfo>::iterator it;
	for (it = m_serverList.begin(); it != m_serverList.end(); it++, ++index)
	{
		if (index == m_curServerIndex)
		{
			(*it).bInvalid = true;
			return;
		}
	}
}
