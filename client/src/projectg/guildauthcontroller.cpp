#include "minatl.h"
#include "guildactor.h"
#include "guildauthcontroller.h"
#include "guildmainform.h"
#include "guildinfo.h"
#include "frcontextmenuctrl.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "bitmap.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "shareddoc.h"
#include "user_info.h"
#include "commonutil.h"

extern Fresh* g_pFresh;

static bool IsZero(float f)
{
	return f < g_EPSILON;
}

namespace _guild
{
	CGuildInfo* IGuildAuthController::m_linkInfo;
	CGuildMainForm* IGuildAuthController::m_linkForm;

	IGuildAuthController::IGuildAuthController()
	{
	}

	IGuildAuthController::~IGuildAuthController()
	{
	}

	void IGuildAuthController::SetLinkDepends(CGuildMainForm* pForm,
		CGuildInfo* pInfo)
	{
		m_linkForm = pForm;
		m_linkInfo = pInfo;
	}

	void IGuildAuthController::PopupCommandMenu(float x, float y)
	{
		// HACK: the original does not depend on the WPoint(float, float)
		// constructor here (it would defer the emission of this function and
		// its callers); a plain _WPOINT generates the same code
		_WPOINT pt = { x, y };
		m_linkForm->m_pCommandMenu->MoveWindow(static_cast<WPoint&>(pt));
		m_linkForm->m_pCommandMenu->SetVisible(true);
	}

	void IGuildAuthController::OnGuildJoin_LButtonUp()
	{
		if (m_linkInfo->IsGuildMember() == true)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)m_linkInfo->GuildMsg(0x3f5), 0, 0, 0, 0));
		}
	}

	void CGuildManager::DrawGuildTalk(FrListItem* pItem)
	{
		CGuildMainForm::sMemberItem* pMember =
			(CGuildMainForm::sMemberItem*)pItem->pData;

		if (pMember->bEmpty)
			return;

		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		pGDI->Print(WPoint(m_linkForm->m_talkTextOffset + pItem->pos.x,
						pItem->pos.y + 10.0f),
			0, pMember->pInfo->message);

		const Bitmap* pIcon = m_linkForm->m_pTalkIcon;
		if (pIcon)
		{
			pGDI->DrawTexture(pIcon,
				WRect(m_linkForm->m_talkIconOffset + pItem->pos.x,
					pItem->pos.y + 5.0f, pIcon->Width(), pIcon->Height()),
				0xffffffff, 0);
		}
	}

	void CGuildManager::ProcManagerCommand(const char* command)
	{
		if (m_linkForm->m_pSelectedMember == NULL)
			return;

		const GUILD_USER_LIST* pUser = m_linkForm->m_pCommandUserInfo;

		if (strcmp("\xbd\xc2\xc0\xce(\xbd\xc2\xb1\xde)", command) == 0)
		{
			if (pUser->userUID == MyUID())
				return;

			if (m_linkInfo->IsGuildMember(pUser->classIdx))
			{
				if (pUser->classIdx == 1 || pUser->classIdx == 2)
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)m_linkInfo->GuildMsg(0x3f3), 0, 0, 0, 0));
				}
				else if (pUser->classIdx == 3)
				{
					WSendPacketSem983 packet((enumClientPacket)0x10d); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
					packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
					packet.Encode4(pUser->userUID);
					packet.Encode4(2);
					packet.Send(TO_GAME);

					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
				}
			}
			else
			{
				WSendPacketSem983 packet((enumClientPacket)0x10b); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
				packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
				packet.Encode4(pUser->userUID);
				packet.Send(TO_GAME);

				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			}
		}
		else if (strcmp("\xb0\xad\xb5\xee", command) == 0)
		{
			if (pUser->userUID == MyUID())
				return;

			if (m_linkInfo->IsGuildMember(pUser->classIdx))
			{
				if (pUser->classIdx == 2)
				{
					WSendPacketSem983 packet((enumClientPacket)0x10d); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
					packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
					packet.Encode4(pUser->userUID);
					packet.Encode4(3);
					packet.Send(TO_GAME);

					AfxGetTask()->SendMsgToMainActor(
						MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
				}
				else
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)m_linkInfo->GuildMsg(0x3f4), 0, 0, 0, 0));
				}
			}
			else if (pUser->classIdx == 9)
			{
				WSendPacketSem983 packet((enumClientPacket)0x10c); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
				packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
				packet.Encode4(pUser->userUID);
				packet.Send(TO_GAME);

				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			}
		}
		else if (strcmp("\xb1\xe6\xb5\xe5\xc0\xe5 \xc0\xa7\xc0\xd3", command) ==
			0)
		{
			if (pUser->userUID == CSharedDoc::Instance()->m_myInfo.info.dwUID)
				return;

			if (pUser->classIdx > 2)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)m_linkInfo->GuildMsg(0x40c), 0, 0, 0, 0));
			}
			else if (pUser->classIdx == 2)
			{
				IActor* pActor = AfxGetTask()->GetActor(s_guildActor[1]);
				pActor << MsgObject(NULL, M983(0x263), 0x1e,
					(int)m_linkInfo->GuildMsg(0x408), 1, 0, 0);
			}
		}
		else if (strcmp("\xb1\xe6\xb5\xe5\xbf\xf8 \xc3\xdf\xb9\xe6", command) ==
			0)
		{
			if (pUser->userUID == MyUID())
				return;

			WSendPacketSem983 packet((enumClientPacket)0x111); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
			packet.Encode4(pUser->userUID);
			packet.Send(TO_GAME);

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		else if (strcmp("\xc1\xa4\xba\xb8\xba\xb8\xb1\xe2", command) == 0)
		{
			CUserInfo::Instance()->SetInfo(pUser->userUID, -1, false, true,
				false, true, pUser->nickname);
		}
	}

	void CGuildManager::OnGuildIntroConfirm_LButtonUp()
	{
		std::string text;
		int lineNum = m_linkForm->m_pIntroduceEdit->GetLineNum();
		for (int i = 1; i <= lineNum; i++)
		{
			text += m_linkForm->m_pIntroduceEdit->GetLine(i, false);
			if (i != lineNum)
			{
				text += s_guildLineFeed;
			}
		}

		if (text == m_linkInfo->GetMyGuildInfo().introduce)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)m_linkInfo->GuildMsg(0x424), 0, 0, 0, 0));
			return;
		}

		str_Replace(text, "'", "`");
		int result = CSharedDoc::Instance()->m_chatManager.FilteringGuildString(
			text.c_str(), 2, 100, false);
		if (result != 1)
		{
			if (result > 0)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)m_linkInfo->GuildMsg(result), 0, 0, 0, 0));
			}
			return;
		}

		std::string introduce(text);
		str_Replace(introduce, "`", "'");
		m_linkInfo->SetGuildIntroduce(introduce);

		WSendPacketSem983 packet(0x103); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
		packet.Encode4(MyUID());
		packet.EncodeStr(text);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}

	void CGuildManager::OnGuildNoticeConfirm_LButtonUp()
	{
		std::string text;
		int lineNum = m_linkForm->m_pNoticeEdit->GetLineNum();
		for (int i = 1; i <= lineNum; i++)
		{
			text += m_linkForm->m_pNoticeEdit->GetLine(i, false);
			text += s_guildLineFeed;
		}

		if (text == m_linkInfo->GetMyGuildInfo().notice)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)m_linkInfo->GuildMsg(0x424), 0, 0, 0, 0));
			return;
		}

		str_Replace(text, "'", "`");
		int result = CSharedDoc::Instance()->m_chatManager.FilteringGuildString(
			text.c_str(), 2, 100, false);
		if (result != 1)
		{
			if (result > 0)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
					(int)m_linkInfo->GuildMsg(result), 0, 0, 0, 0));
			}
			return;
		}

		std::string notice(text);
		str_Replace(notice, "`", "'");
		m_linkInfo->SetGuildNotice(notice);

		WSendPacketSem983 packet(0x102); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
		packet.Encode4(MyUID());
		packet.EncodeStr(text);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}

	void CGuildMaster::PopupCommandMenu(float x, float y)
	{
		IGuildAuthController::PopupCommandMenu(x, y);
		m_linkForm->m_pCommandMenu->SetMenuList(m_linkInfo->GuildMsg(0x3f0));
	}

	void CGuildMaster::OnGuildShutDown_LButtonUp()
	{
		IActor* pActor = AfxGetTask()->GetActor(s_guildActor[1]);
		pActor << MsgObject(NULL, M983(0x263), 0x1e, (int)m_linkInfo->GuildMsg(0x400),
			0, 0, 0);
	}

	void CGuildMaster::OnGuildOut_LButtonUp()
	{
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x23), (int)m_linkInfo->GuildMsg(0x3fe), 0, 0, 0, 0));
	}

	void CGuildSubMaster::PopupCommandMenu(float x, float y)
	{
		IGuildAuthController::PopupCommandMenu(x, y);
		m_linkForm->m_pCommandMenu->SetMenuList(m_linkInfo->GuildMsg(0x3f1));
	}

	void CGuildSubMaster::OnGuildOut_LButtonUp()
	{
		IActor* pActor = AfxGetTask()->GetActor(s_guildActor[1]);
		pActor << MsgObject(NULL, M983(0x263), 0x1e, (int)m_linkInfo->GuildMsg(0x417),
			2, 0, 0);
	}

	void CGuildMember::DrawGuildTalk(FrListItem* pItem)
	{
		CGuildMainForm::sMemberItem* pMember =
			(CGuildMainForm::sMemberItem*)pItem->pData;

		if (pMember->bEmpty)
			return;

		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		pGDI->Print(WPoint(m_linkForm->m_talkTextOffset + pItem->pos.x,
						pItem->pos.y + 10.0f),
			0, pMember->pInfo->message);

		if (pMember->pInfo->userUID == MyUID())
		{
			const Bitmap* pIcon = m_linkForm->m_pTalkIcon;
			if (pIcon)
			{
				pGDI->DrawTexture(pIcon,
					WRect(m_linkForm->m_talkIconOffset + pItem->pos.x,
						pItem->pos.y + 5.0f, pIcon->Width(), pIcon->Height()),
					0xffffffff, 0);
			}
		}
	}

	void CGuildMember::PopupCommandMenu(float x, float y)
	{
		IGuildAuthController::PopupCommandMenu(x, y);
		m_linkForm->m_pCommandMenu->SetMenuList(m_linkInfo->GuildMsg(0x3f2));
	}

	void CGuildMember::ProcManagerCommand(const char* command)
	{
		if (m_linkForm->m_pSelectedMember)
		{
			CGuildMainForm::sMemberItem* pMember =
				(CGuildMainForm::sMemberItem*)
					m_linkForm->m_pSelectedMember->pData;
			if (strcmp("\xc1\xa4\xba\xb8\xba\xb8\xb1\xe2", command) == 0)
			{
				CUserInfo::Instance()->SetInfo(pMember->pInfo->userUID, -1,
					false, true, false, true, pMember->pInfo->nickname);
			}
		}
	}

	void CGuildMember::OnGuildOut_LButtonUp()
	{
		IActor* pActor = AfxGetTask()->GetActor(s_guildActor[1]);
		pActor << MsgObject(NULL, M983(0x263), 0x1e, (int)m_linkInfo->GuildMsg(0x417),
			2, 0, 0);
	}

	void CGuildWaitMember::OnGuildJoinAbort_LButtonUp()
	{
		WSendPacketSem983 packet(0x10a); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(m_linkInfo->GetMyGuildInfo().guildUID);
		packet.Send(TO_GAME);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}

	void CGuildWaitMember::OnGuildOut_LButtonUp()
	{
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x23), (int)m_linkInfo->GuildMsg(0x3fe), 0, 0, 0, 0));
	}

	void CGuildNull::OnGuildJoin_LButtonUp()
	{
		IGuildAuthController::OnGuildJoin_LButtonUp();

		IActor* pActor = AfxGetTask()->GetActor(s_guildActor[1]);
		if (pActor)
		{
			pActor << MsgObject(NULL, M983(0x263), 0x1d,
				m_linkForm->m_guildActorParam, 0, 0, 0);
		}
	}
}
