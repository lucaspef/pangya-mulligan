#include "minatl.h"
#include "guildactor.h"
#include "guildmainform.h"
#include "guildinfo.h"
#include "guildforms.h"
#include "contentsdoc.h"
#include "shareddoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "commonutil.h"

extern Fresh* g_pFresh;

static bool IsZero(float f)
{
	return f < g_EPSILON;
}

// HACK: declaring CSharedDoc::FindUserInfo in shareddoc.h changes the codegen
// of other PCH users (its iterator parameter); it is declared on a helper
// class instead and aliased to the real symbol
class CSharedDocUserFinder : public CSharedDoc
{
public:
	int FindUserInfo(unsigned long uid,
		std::map<unsigned long, sBriefUserInfo>::iterator& it);
};

#pragma comment(linker, \
	"/alternatename:" \
	"?FindUserInfo@CSharedDocUserFinder@@QAEHKAAViterator@?$_Tree@V?$" \
	"_Tmap_traits@KUsBriefUserInfo@@U?$less@K@std@@V?$allocator@U?$pair@$$" \
	"CBKUsBriefUserInfo@@@std@@@3@$0A@@std@@@std@@@Z" \
	"=?FindUserInfo@CSharedDoc@@QAEHKAAViterator@?$_Tree@V?$_Tmap_traits@" \
	"KUsBriefUserInfo@@U?$less@K@std@@V?$allocator@U?$pair@$$CBKUsBriefUser" \
	"Info@@@std@@@3@$0A@@std@@@std@@@Z")

// NOTE: inline in shareddoc.h in the original; an inline body in the PCH
// changes the COMDAT order of other units (itemmanager)
inline const sPangYaUserInfo& CSharedDoc::GetPangyaUserInfo()
{
	// HACK: dead dependency, emits this COMDAT where the PCH inline would be
	if (0)
		Instance();

	return m_myInfo.info;
}

namespace _guild
{
	IMPLEMENT_ACTOR(CGuildActor, IActor)

	CGuildActor::CGuildActor()
	{
		InitControl();
	}

	CGuildActor::~CGuildActor()
	{
	}

	void CGuildActor::InitControl()
	{
		m_pCreateGuildForm = NULL;
		m_pRegisterGuildMarkForm = NULL;
		m_pChangeGuildNameForm = NULL;
		m_pGuildJoinForm = NULL;
		m_pMainForm = NULL;
		m_pGuildInfo = NULL;
	}

	// HACK: never called; instantiates list::size ahead of std::pair, as in
	// the original COMDAT order
	static unsigned int GuildListSize(const std::list<GUILD_LIST>& list)
	{
		return list.size();
	}

	void CGuildActor::OnInit()
	{
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x97,
			m_pGuildInfo);
	}

	void CGuildActor::HandleMsg(const MsgObject& msg)
	{
		switch (msg.message)
		{
		case M983(0x263):
			ExecGuildMessage(msg);
			break;
		}
	}

	void CGuildActor::RequestGuildUserList(int page)
	{
		if (page < 1)
			return;

		if (!m_pGuildInfo->IsGuildMember())
			return;

		WSendPacketSem983 packet((enumClientPacket)0x10f); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
		packet.Encode4(m_pGuildInfo->GetMyGuildInfo().guildUID);
		packet.Encode4(page);
		packet.Send(TO_GAME);
	}

	void CGuildActor::ExecGuildMessage(const MsgObject& msg)
	{
		switch (msg.param1)
		{
		case 1:
			OpenGuileMainForm();
			break;

		case 27:
			OpenGuildItemForm(msg.param2);
			break;

		case 29:
			OpenGuildJoinForm(msg.param2);
			break;

		case 30:
			OpenGuildQuestionForm((const char*)msg.param2, msg.param3);
			break;

		case 3:
		{
			std::string* pName = (std::string*)msg.param2;
			if (m_pCreateGuildForm)
				m_pCreateGuildForm->SetConfirmGuildName(pName->c_str());
			if (m_pChangeGuildNameForm)
				m_pChangeGuildNameForm->SetConfirmedName(pName->c_str());
			break;
		}

		case 4:
		{
			m_pGuildInfo->ClearAllData();
			GUILD_INFO* pInfo = (GUILD_INFO*)msg.param2;
			m_pGuildInfo->SetMyGuildInfo(*pInfo);
			ReInitMyGuildInfo_SharedDoc(pInfo);
			if (m_pMainForm)
			{
				{
					WSendPacketSem983 packet((enumClientPacket)0x107); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
					packet.Send(TO_GAME);
				}
				m_pMainForm->Refresh();
				m_pMainForm->RequestMember_Page(0);
			}
			break;
		}

		case 5:
		{
			GUILD_USER_INFO* pInfo = (GUILD_USER_INFO*)msg.param2;
			std::pair<bool, bool> state = CheckMyGuildState(pInfo);
			if (state.first)
			{
				m_pGuildInfo->ClearAllData();
				Doc()->ClearMyGuildInfo();
				if (m_pMainForm)
					m_pMainForm->ClearForm();
			}
			if (state.second)
			{
				m_pGuildInfo->ClearMemberPage();
				m_pGuildInfo->SetMyGuildUserInfo(*pInfo);
				ReInitMyGuildInfo_SharedDoc(pInfo);
				if (m_pMainForm)
				{
					m_pMainForm->Refresh();
					m_pMainForm->RequestMember_Page(0);
				}
			}
			break;
		}

		case 11:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x401), 0, 0, 0, 0));
			break;

		case 12:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x3fd), 0, 0, 0, 0));
			break;

		case 6:
		{
			m_pGuildInfo->ClearAllData();
			GUILD_USER_INFO* pInfo = (GUILD_USER_INFO*)msg.param2;
			m_pGuildInfo->SetMyGuildUserInfo(*pInfo);
			ReInitMyGuildInfo_SharedDoc(pInfo);
			break;
		}

		case 7:
		{
			std::list<GUILD_USER_LIST>* pList =
				(std::list<GUILD_USER_LIST>*)msg.param2;
			int page = msg.param3;
			int count = msg.param4;
			m_pGuildInfo->SetMemberPage(page, pList);
			m_pGuildInfo->SetAllMemberCount(count);
			if (m_pMainForm)
				m_pMainForm->RefreshGuildMember();
			break;
		}

		case 8:
		{
			std::list<GUILD_LIST>* pList = (std::list<GUILD_LIST>*)msg.param2;
			int page = msg.param3;
			int count = msg.param4;
			if (pList->size() == 0)
			{
				AfxGetTask()->SendMsgToMainActor(
					MsgObject(NULL, M983(0x23), (int)SystemMsg(0x40b), 0, 0, 0, 0));
				if (m_pMainForm)
					m_pMainForm->InitSearchMode();
			}
			else
			{
				m_pGuildInfo->SetSearchList(page, pList);
				m_pGuildInfo->SetAllGuildCount(count);
				if (m_pMainForm)
					m_pMainForm->RefreshGuildSearch();
			}
			break;
		}

		case 9:
			m_pGuildInfo->SetHistoryList((std::list<GUILD_HISTORY>*)msg.param2);
			if (m_pMainForm)
				m_pMainForm->RefreshMyGuildHistory();
			break;

		case 10:
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x3f8), 0, 0, 0, 0));
			break;

		case 14:
			if (m_pMainForm)
				m_pMainForm->DisableJoinButton();
			if (m_pGuildJoinForm)
				m_pGuildJoinForm->SetSuccessRequest();
			break;

		case 15:
			if (m_pMainForm)
				m_pMainForm->RequestMember_Page(0);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x3fb), 0, 0, 0, 0));
			break;

		case 16:
			if (m_pMainForm)
				m_pMainForm->RequestMember_Page(0);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x3fa), 0, 0, 0, 0));
			break;

		case 17:
		{
			if (m_pMainForm == NULL)
				break;
			if (m_pGuildJoinForm == NULL)
				break;
			m_pGuildJoinForm->Close(FrCANCEL, true);
			int id = msg.param2;
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(id), 0, 0, 0, 0));
			break;
		}

		case 13:
			if (m_pMainForm)
				m_pMainForm->RequestMember_Page(0);
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x3f9), 0, 0, 0, 0));
			break;

		case 18:
			if (m_pMainForm)
			{
				m_pMainForm->RequestMember_Page(0);
				m_pMainForm->UnLockScrollMemberList();
			}
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(0x406), 0, 0, 0, 0));
			break;

		case 19:
		{
			if (m_pMainForm)
				m_pMainForm->RequestMember_Page(0);

			const char* text;
			switch (msg.param2)
			{
			case 1:
				text = MakeStr(SystemMsg(0x409), SystemMsg(0x435),
					SystemMsg(0x437));
				break;
			case 2:
				text = MakeStr(SystemMsg(0x40a), SystemMsg(0x436));
				break;
			case 3:
				text = MakeStr(SystemMsg(0x40a), SystemMsg(0x437));
				break;
			default:
				return;
			}

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)text, 0, 0, 0, 0));
			break;
		}

		case 32:
		{
			unsigned long* pGuildUID = (unsigned long*)msg.param2;
			if (m_pGuildInfo->IsGuildMember())
				*pGuildUID = m_pGuildInfo->GetMyGuildInfo().guildUID;
			else
				*pGuildUID = 0;
			break;
		}

		case 33:
		{
			const GUILD_INFO** ppInfo = (const GUILD_INFO**)msg.param2;
			*ppInfo = &m_pGuildInfo->GetMyGuildInfo();
			break;
		}

		case 34:
		{
			int* pClassIdx = (int*)msg.param2;
			*pClassIdx = m_pGuildInfo->GetClassIndex();
			break;
		}

		case 21:
			if (m_pRegisterGuildMarkForm)
			{
				m_pRegisterGuildMarkForm->RequestUpload(msg.param2,
					*(std::string*)msg.param3);
			}
			break;

		case 22:
			ChangeGuildEmblem((GUILD_USER_INFO*)msg.param2);
			break;

		case 23:
			ChangeGuildName((GUILD_INFO*)msg.param2);
			break;

		case 24:
		{
			WSendPacket packet(0x2a);
			packet.Encode4(m_pGuildInfo->GetGuildNumber());
			packet.Send(TO_MSN);
			break;
		}

		case 25:
		{
			WSendPacket packet(0x2b);
			packet.Encode4(m_pGuildInfo->GetGuildNumber());
			packet.Send(TO_MSN);
			break;
		}

		case 26:
		{
			int id = msg.param2;
			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)SystemMsg(id), 0, 0, 0, 0));
			break;
		}
		}
	}

	void CGuildActor::OpenGuileMainForm()
	{
		if (m_pMainForm == NULL)
		{
			m_pMainForm = CreateForm<CGuildMainForm>(g_pFresh->GetManager(),
				this, "guildmain", NULL);
			m_pMainForm->EnableDrag(false);
			m_pMainForm->SetLinkGuildInfo(m_pGuildInfo);
			m_pMainForm->Open(
				(FRESH_PFN_RESULT)&CGuildActor::OnMainFormCloseResult, 1);
		}
	}

	void CGuildActor::OpenGuildItemForm(unsigned long typeId)
	{
		switch (typeId)
		{
		case 0x1a00012f:
			if (m_pCreateGuildForm == NULL)
			{
				m_pCreateGuildForm = CreateForm<FrCreateGuildKit>(
					g_pFresh->GetManager(), this, "create_guild", NULL);
				m_pCreateGuildForm->EnableDrag(false);
				m_pCreateGuildForm->SetCaption(SystemMsg(0x3ed));
				m_pCreateGuildForm->Open(
					(FRESH_PFN_RESULT)&CGuildActor::OnCreateGuildFormResult, 1);
			}
			break;

		case 0x1a000130:
			if (m_pRegisterGuildMarkForm == NULL)
			{
				if (!m_pGuildInfo->IsManager())
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)SystemMsg(0xd4eb), 0, 0, 0, 0));
					return;
				}

				m_pRegisterGuildMarkForm = CreateForm<FrRegisterGuildMark>(
					g_pFresh->GetManager(), this, "register_guild_mark", NULL);
				m_pRegisterGuildMarkForm->EnableDrag(false);
				m_pRegisterGuildMarkForm->SetCaption(SystemMsg(0x3ee));
				m_pRegisterGuildMarkForm->Open(
					(FRESH_PFN_RESULT)&CGuildActor::OnRegisterGuildMarkResult,
					1);
			}
			break;

		case 0x1a000131:
			if (m_pChangeGuildNameForm == NULL)
			{
				if (!m_pGuildInfo->IsManager())
				{
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
						(int)SystemMsg(0xd4e9), 0, 0, 0, 0));
					return;
				}

				m_pChangeGuildNameForm = CreateForm<FrChangeGuildName>(
					g_pFresh->GetManager(), this, "change_guild_name", NULL);
				if (m_pChangeGuildNameForm->IsHaveGuild())
				{
					m_pChangeGuildNameForm->EnableDrag(false);
					m_pChangeGuildNameForm->SetCaption(SystemMsg(0x3ef));
					m_pChangeGuildNameForm->Open(
						(FRESH_PFN_RESULT)&CGuildActor::OnChangeGuildNameResult,
						1);
				}
			}
			break;
		}
	}

	void CGuildActor::OpenGuildJoinForm(int guildNumber)
	{
		if (m_pGuildJoinForm == NULL)
		{
			m_pGuildJoinForm = CreateForm<FrGuildJoin>(g_pFresh->GetManager(),
				this, "requestjoinform", NULL);
			m_pGuildJoinForm->SetFixed(true);
			m_pGuildJoinForm->EnableDrag(false);
			m_pGuildJoinForm->SetGuildNumber(guildNumber);
			m_pGuildJoinForm->Open(
				(FRESH_PFN_RESULT)&CGuildActor::OnGuildJoinFormResult, 1);
		}
	}

	void CGuildActor::OpenGuildQuestionForm(const char* message, int type)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);

		pForm->SetMessage(message, false);
		pForm->SetTopmost(true);
		pForm->FindNextTopFocus(false);

		FRESH_PFN_RESULT pfn = NULL;
		switch (type)
		{
		case 0:
			pfn = (FRESH_PFN_RESULT)&CGuildActor::OnGuildShutDownResult;
			break;
		case 1:
			pfn = (FRESH_PFN_RESULT)&CGuildActor::OnGuildMasterDelegate;
			break;
		case 2:
			pfn = (FRESH_PFN_RESULT)&CGuildActor::OnGuildOutResult;
			break;
		}

		if (pfn)
			pForm->Open(pfn, 1);
	}

	void CGuildActor::ReInitMyGuildInfo_SharedDoc(GUILD_USER_INFO* pInfo)
	{
		sPangYaUserInfo& info = (sPangYaUserInfo&)Doc()->GetPangyaUserInfo();
		info.dwGuildId = pInfo->guildUID;
		info.dwEmblemVer = pInfo->guildPang;
		strncpy(info.sGuild, pInfo->guildName, 20);
		strncpy(info.szEmblemName, pInfo->guildMark, 11);
	}

	std::pair<bool, bool> CGuildActor::CheckMyGuildState(GUILD_USER_INFO* pInfo)
	{
		if (m_pGuildInfo == NULL)
			return std::pair<bool, bool>(true, false);

		GUILD_CLASS_IDX myClass = m_pGuildInfo->GetClassIndex();
		switch (pInfo->classIdx)
		{
		case 0:
			if (myClass == 1 || myClass == 3 || myClass == 2)
				return std::pair<bool, bool>(true, false);
			else if (myClass == 9)
				return std::pair<bool, bool>(false, true);
			break;

		case 9:
			if (myClass == 0)
				return std::pair<bool, bool>(false, true);
			break;

		case 3:
			if (myClass == 9)
			{
				ChangeGuildEmblem(pInfo);
				return std::pair<bool, bool>(false, true);
			}
			else if (myClass == 2)
				return std::pair<bool, bool>(false, true);
			else if (myClass == 1)
				return std::pair<bool, bool>(false, true);
			break;

		case 2:
			if (myClass == 3)
				return std::pair<bool, bool>(false, true);
			break;

		case 1:
			if (myClass == 2)
				return std::pair<bool, bool>(false, true);
			else if (myClass == 0)
				return std::pair<bool, bool>(false, true);
			break;
		}

		return std::pair<bool, bool>(false, false);
	}

	void CGuildActor::ChangeGuildEmblem(GUILD_USER_INFO* pInfo)
	{
		if (pInfo->guildUID == 0)
			return;

		m_pGuildInfo->SetGuildEmblem(pInfo);

		strncpy((char*)Doc()->GetPangyaUserInfo().szEmblemName,
			pInfo->guildMark, 11);

		std::map<unsigned long, sBriefUserInfo>::iterator it;
		// C65: chamada direta; o apelido CSharedDocUserFinder (truque de PCH do rebang, VC7.1) tem o nome decorado do VC7.1
		// (iterator@?$_Tree) e no VC10 o símbolo citado (_Tree_iterator) ficava sem corpo no link (out/V6/link/falta.tsv)
		if (Doc()->FindUserInfo(MyGuid(false), it) == 1)
		{
			strncpy((*it).second.szEmblemName, pInfo->guildMark, 11);
		}

		strncpy((char*)Doc()->GetPangyaUserInfo().szEmblemName,
			pInfo->guildMark, 11);
	}

	void CGuildActor::ChangeGuildName(GUILD_INFO* pInfo)
	{
		if (pInfo->guildUID == 0)
			return;

		m_pGuildInfo->SetGuildName(pInfo);

		strncpy((char*)Doc()->GetPangyaUserInfo().sGuild, pInfo->guildName, 20);
	}

	const char* CGuildActor::SystemMsg(int id)
	{
		// HACK: dead dependency, keeps the original emission order
		if (0)
			CContentsDoc::Instance();

		const char* msg = m_pGuildInfo->GuildMsg(id);
		if (msg == NULL)
			msg = "\xc1\xa4\xc0\xc7 \xb5\xc7\xc1\xf6 \xbe\xca\xc0\xba "
				  "\xbf\xa1\xb7\xaf \xb8\xde\xbd\xc3\xc1\xf6\xc0\xd4\xb4\xcf"
				  "\xb4\xd9.";
		return msg;
	}

	bool CGuildActor::OnCreateGuildFormResult(int result, FrForm* pForm)
	{
		m_pCreateGuildForm = NULL;
		return true;
	}

	bool CGuildActor::OnRegisterGuildMarkResult(int result, FrForm* pForm)
	{
		m_pRegisterGuildMarkForm = NULL;
		return true;
	}

	bool CGuildActor::OnChangeGuildNameResult(int result, FrForm* pForm)
	{
		m_pChangeGuildNameForm = NULL;
		return true;
	}

	bool CGuildActor::OnGuildJoinFormResult(int result, FrForm* pForm)
	{
		m_pGuildJoinForm = NULL;
		return true;
	}

	bool CGuildActor::OnMainFormCloseResult(int result, FrForm* pForm)
	{
		m_pMainForm = NULL;
		return true;
	}

	bool CGuildActor::OnGuildShutDownResult(int result, FrForm* pForm)
	{
		if (result == FrOK)
		{
			WSendPacketSem983 packet((enumClientPacket)0x104); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(m_pGuildInfo->GetMyGuildInfo().guildUID);
			packet.Send(TO_GAME);

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		return true;
	}

	bool CGuildActor::OnGuildMasterDelegate(int result, FrForm* pForm)
	{
		if (result == FrOK)
		{
			if (m_pMainForm)
				m_pMainForm->RequestChangeClass((GUILD_CLASS_IDX)1);
		}
		return true;
	}

	bool CGuildActor::OnGuildOutResult(int result, FrForm* pForm)
	{
		if (result == FrOK)
		{
			WSendPacketSem983 packet((enumClientPacket)0x110); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(m_pGuildInfo->GetMyGuildInfo().guildUID);
			packet.Send(TO_GAME);

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		return true;
	}
}
