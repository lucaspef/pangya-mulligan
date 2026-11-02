#include "minatl.h"
#define REBANG_GHOSTHANDLER_CPP
#include "ghosthandler.h"
// HACK: ghostdocument.h names its game data header GHOST_GAME_BRIEF, which clashes
// with the list element of ghosttypes.h
#define GHOST_GAME_BRIEF GHOST_GAME_DATA_HEADER
#include "ghostdocument.h"
#undef GHOST_GAME_BRIEF
#include "ghostmanager.h"
#include "ghostgamedlg.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "taskmanager.h"
#include "packet.h"
#include "mousecursor.h"
#include "projectg.h"
#include "../shared/localize.h"
#include "tinyxml.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "frform.h"


extern Fresh* g_pFresh;

unsigned char MyIndex();

IMPLEMENT_ACTOR(CGhostRecorder, IActor)

CGhostRecorder::CGhostRecorder()
	: m_pDocument(NULL)
{
}

bool CGhostRecorder::IsSaveAvailable()
{
	if (Doc()->m_golfGame.gameType != 4 && Doc()->m_golfGame.gameType != 0)
		return false;
	if (Doc()->m_holeType == 4)
		return false;
	if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX)
		return false;

	if (Doc()->m_golfGame.holes == 18)
		return true;
	return false;
}

void CGhostRecorder::HandleMsg(const MsgObject& msg)
{
	if (m_pDocument)
	{
		switch (msg.message)
		{
		case M983(0xb):
			CGhostManager::Instance()->ReleaseDocument();
			break;
		default:
			m_pDocument->RecordHoleData(msg);
			break;
		}
	}
}

void CGhostRecorder::OnLoad()
{
	CGhostManager::Instance()->ReleaseDocument();
	if (OnlinePlay() && IsSaveAvailable())
	{
		m_pDocument = CGhostManager::Instance()->GetDocument();
		m_pDocument->RecordUserData();
		m_pDocument->RecordGameData();
	}
}

void CGhostRecorder::OnReload()
{
}

void CGhostRecorder::OnInit()
{
}

void CGhostRecorder::OnDestroy()
{
}

CGhostLoader::CGhostLoader()
{
}

bool CGhostLoader::LoadFromMemory(unsigned char* data, unsigned int size)
{
	CGhostManager::Instance()->ReleaseDocument();
	CGhostDocument* pDocument = CGhostManager::Instance()->GetDocument();
	return pDocument->Import(data, size);
}

bool CGhostLoader::LoadFromFile(const std::string& path)
{
	CGhostManager::Instance()->ReleaseDocument();
	CGhostDocument* pDocument = CGhostManager::Instance()->GetDocument();
	return pDocument->Import(path);
}

namespace
{

	const char* NOTIFY_MSG_SERVICE_STOP = true
		? K2L_Compatibility("\xb0\xed\xbd\xba\xc6\xae \xbc\xad\xba\xf1\xbd\xba \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xcc\xbf\xeb\xbf\xa1 \xba\xd2\xc6\xed\xc0\xbb \xb5\xe5\xb7\xc1\xbc\xad \xc1\xcb\xbc\xdb\xc7\xd5\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* RECORD_DLG_LOADING_FAIL = true
		? K2L_Compatibility("\xb7\xce\xb5\xf9\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* CANNOT_UNDERSTAND_MSG = true
		? K2L_Compatibility("\xb4\xd4\xc0\xc7 \xb0\xed\xbd\xba\xc6\xae\xb0\xa1 \xbe\xee\xb8\xae\xb5\xd5\xc0\xfd\xc7\xd8 \xc7\xd5\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_GHOST_NOSAVE = true
		? K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc0\xaf\xc0\xfa\xbf\xa1\xb0\xd4 \xc0\xfa\xc0\xe5\xb5\xc8 \xb0\xed\xbd\xba\xc6\xae\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_GHOST_NODATA = true
		? K2L_Compatibility("\xb0\xed\xbd\xba\xc6\xae \xb5\xa5\xc0\xcc\xc5\xcd\xb0\xa1 \xc1\xb8\xc0\xe7 \xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_GHOST_SAVED = true
		? K2L_Compatibility("\xb0\xed\xbd\xba\xc6\xae \xb5\xa5\xc0\xcc\xc5\xcd\xb0\xa1 \xb0\xbb\xbd\xc5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_UNKNOWN_ERROR = true
		? K2L_Compatibility("\xbd\xc3\xbd\xba\xc5\xdb \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_DATA_ERROR = true
		? K2L_Compatibility("\xbc\xd5\xbb\xf3\xb5\xc8 \xb5\xa5\xc0\xcc\xc5\xcd\xb0\xa1 \xb9\xdf\xb0\xdf\xb5\xc7\xbe\xee \xb4\xf5\xc0\xcc\xbb\xf3 \xb0\xd4\xc0\xd3\xc0\xbb \xc1\xf8\xc7\xe0\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_NETWAIT_OVER = true
		? K2L_Compatibility("\xbc\xad\xb9\xf6\xbf\xa1 \xbf\xe4\xc3\xbb\xc0\xcc \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3 \xc8\xc4 \xb4\xd9\xbd\xc3 \xc0\xcc\xbf\xeb\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.")
		: NULL;
	const char* GHOST_ROOM_JOIN_WELCOME = true
		? K2L_Compatibility("(\xbe\xcb\xb8\xb2)\\c0xffffffff\\c%s\\c0xffff7878\\c\xb4\xd4\xc0\xcc \xc0\xd4\xc0\xe5\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;
	const char* NOTIFY_MSG_DISABLE_GAME = true
		? K2L_Compatibility("\xbc\xd5\xbb\xf3\xb5\xc8 \xb5\xa5\xc0\xcc\xc5\xcd\xbf\xcd\xb4\xc2 \xc0\xe7\xb4\xeb\xb0\xe1 \xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xb8\xa5 \xb0\xed\xbd\xba\xc6\xae\xbf\xcd \xb4\xeb\xc0\xfc\xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4.")
		: NULL;
	const char* NOTIFY_MSG_NOT_DEFINED = true
		? K2L_Compatibility("\xc1\xa4\xc0\xc7\xb5\xc8 \xbf\xc0\xb7\xf9 \xb8\xde\xbc\xbc\xc1\xf6\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")
		: NULL;

	class CEventGhostXmlFile
	{
	public:
		bool Load()
		{
			m_gameList.clear();
			TiXmlDocument doc;
			if (!doc.LoadFileEx("event_ghost.xml"))
				return false;

			std::string uid;
			std::string file;
			TiXmlNode* pActive = doc.FirstChild("active");
			if (!pActive)
				return false;

			TiXmlElement* pActiveElem = pActive->ToElement();
			uid = pActiveElem->Attribute("uid");
			if (uid == "-1")
				return false;

			TiXmlNode* pNode = pActive->FirstChild("element");
			while (pNode)
			{
				TiXmlElement* pElem = pNode->ToElement();
				if (uid == pElem->Attribute("uid"))
				{
					m_nick = pElem->Attribute("nick");
					TiXmlNode* pParam = pNode->FirstChild("param");

					while (pParam)
					{
						TiXmlElement* pParamElem = pParam->ToElement();
						file = pParamElem->Attribute("file");

						if (!file.empty())
						{
							GHOST_GAME_BRIEF brief;
							memset(&brief, 0, sizeof(brief));
							GetGhostGameBriefFromFileName(&brief, file.c_str());

							brief.active = 1;

							m_gameList.push_back(brief);
						}
						pParam = pParam->NextSibling();
					}
					return !m_gameList.empty();
				}

				pNode = pNode->NextSibling();
			}
			return false;
		}

		const std::string& GetNick() const { return m_nick; }
		const std::list<GHOST_GAME_BRIEF>& GetGameList() const
		{
			return m_gameList;
		}

	private:
		std::string m_nick;
		std::list<GHOST_GAME_BRIEF> m_gameList;
	};

}

CGhostHandler::CGhostHandler()
{
}

CGhostHandler::~CGhostHandler()
{
}

bool CGhostHandler::IsAvailableGhostSystem()
{
	if (!IsLocalContent(S4_GHOST))
		return false;

	bool bStop = (Doc()->m_curChannel.Type >> 11) & 1;
	bStop = !bStop;
	return bStop;
}

void CGhostHandler::OpenNotifyDlg(int type)
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);

	pForm->SetTopmost(true);
	pForm->FindNextTopFocus(false);
	pForm->SetTimeLimit(7.0f);
	switch (type)
	{
	case 7:
		pForm->SetMessage(NOTIFY_MSG_SERVICE_STOP, false);
		break;
	case 0:
		pForm->SetMessage(NOTIFY_MSG_GHOST_NOSAVE, false);
		break;
	case 1:
		pForm->SetMessage(NOTIFY_MSG_GHOST_NODATA, false);
		break;
	case 2:
		pForm->SetMessage(RECORD_DLG_LOADING_FAIL, false);
		break;
	case 4:
		pForm->SetMessage(NOTIFY_MSG_UNKNOWN_ERROR, false);
		break;
	case 5:
		pForm->SetMessage(NOTIFY_MSG_DATA_ERROR, false);
		break;
	case 6:
		pForm->SetMessage(NOTIFY_MSG_NETWAIT_OVER, false);
		break;
	case 8:
		pForm->SetMessage(NOTIFY_MSG_DISABLE_GAME, false);
		break;
	default:
		pForm->SetMessage(NOTIFY_MSG_NOT_DEFINED, false);
		break;
	}

	pForm->Open(NULL, 0);
}

bool CGhostHandler::IsExistEventGhost()
{
	CEventGhostXmlFile file;
	return file.Load();
}

bool CGhostHandler::OpenEventGhostGameDlg()
{
	CEventGhostXmlFile file;
	if (!file.Load())
		return false;

	OpenGhostGameDlg(0xffffffff, file.GetNick(), file.GetGameList(),
		(FRESH_PFN_RESULT)&CGhostHandler::OnResultEventGhostGameDlg);
	return true;
}

void CGhostHandler::OpenGhostGameDlg(unsigned long uid, const std::string& nick,
	const std::list<GHOST_GAME_BRIEF>& gameList, FRESH_PFN_RESULT pFnResult)
{
	if (g_pFresh->GetManager()->GetDesktop()->FindChildForm("GHOST_GAMEMAP") ==
		NULL)
	{
		FrGhostGameMapDlg* pDlg = CreateForm<FrGhostGameMapDlg>(
			g_pFresh->GetManager(), this, "GHOST_GAMEMAP", NULL);

		pDlg->SetTopmost(true);
		pDlg->FindNextTopFocus(false);
		pDlg->Init(uid, nick, gameList);
		pDlg->EnableDrag(false);

		pDlg->SetFlag(8);
		pDlg->Open(pFnResult, 1);
	}
}

void CGhostHandler::RecordComplete()
{
	// HACK: dead reference; the original emits this function in the last wave,
	// after IsExistEventGhost
	if (0)
		IsExistEventGhost();

	if (GOLFDOC())
	{
		int score = PLAYER(MyIndex())->score;
		int pang = PLAYER(MyIndex())->totalPang;

		score += (unsigned char)PLAYER(MyIndex())
					 ->stroke[GOLFDOC()->m_currentHole - 1] -
			GOLFDOC()->GetHoleData().par;

		unsigned char character =
			(unsigned char)Doc()->m_userInfo[MyIndex()].charInfo.tid;

		if (score < 0)
		{
			WSendPacket packet((enumClientPacket)0xc2);
			packet.Encode1(4);
			packet.Encode1(GetCurMap());
			packet.Encode1(character);
			packet.Encode4(abs(score));
			packet.Encode4(pang);
			packet.Send((eSendTo)0);
			return;
		}
	}

	CGhostManager::Instance()->ReleaseDocument();
}

void CGhostHandler::LoadingComplete(bool success)
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
	if (success)
	{
		CMouseCursor::Instance()->SetActive(false);
		CTaskManager::Instance()->ChangeTask("CGhostTask", "",
			CTaskManager::Instance()->GetPreservedTask() != NULL);
		CTaskManager::Instance()->PostMsg(NULL, "Ghost", 0, (int)"GHOST", 0, 0,
			0);
	}
	else
	{
		OpenNotifyDlg(2);
		SendErrorReport("GHOST FILE LOAD FAILED",
			CGhostManager::Instance()->GetLatestFileName());
	}
}

void CGhostHandler::UploadComplete(bool success, const std::string& fileName)
{
	ClearLocalGhostFiles();
	if (success)
	{
		WSendPacket packet((enumClientPacket)0xc2);
		packet.Encode1(5);
		packet.EncodeStr(fileName);
		packet.Send((eSendTo)0);
	}
	else
	{
		SendErrorReport("GHOST FILE UPLOAD FAILED.", fileName);
	}
}

void CGhostHandler::OnGhostPacket(WReceivedPacket& packet)
{
	if (!IsAvailableGhostSystem()) // 983 00821f80 chama 00820f00
		return;

	switch (packet.Decode1())
	{
	case 12:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		int count = packet.Decode4();
		unsigned long uid = packet.Decode4();

		if (count == 0)
		{
			OpenNotifyDlg(0);
		}
		else
		{
			std::list<GHOST_GAME_BRIEF> gameList;
			std::string nick;
			for (int i = 0; i < count; i++)
			{
				int info[5] = { 0 };
				for (int j = 0; j < 5; j++)
					info[j] = packet.Decode4();
				packet.DecodeStr(nick);
				int active = packet.Decode4();

				GHOST_GAME_BRIEF brief;
				brief.ownerUid = uid;
				brief.id = info[0];
				brief.ball = info[1];
				brief.pang = info[2];
				brief.score = info[3];
				brief.recordId = info[4];
				brief.active = active;
				gameList.push_back(brief);
			}

			OpenGhostGameDlg(uid, nick, gameList,
				(FRESH_PFN_RESULT)&CGhostHandler::OnResultGhostGameDlg);
		}
		break;
	}
	case 13:
	{
		std::string path;
		packet.DecodeStr(path);
		CGhostManager::Instance()->SaveDocument(path);
		break;
	}
	case 14:
		CGhostManager::Instance()->ReleaseDocument();
		ClearLocalGhostFiles();
		break;
	case 16:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		switch (packet.Decode1())
		{
		case 28:
			OpenNotifyDlg(7);
			break;
		case 25:
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
			OpenNotifyDlg(1);
			break;
		case 26:
			OpenNotifyDlg(6);
			break;
		}
		break;
	}
	case 19:
	{
		packet.Decode1();
		unsigned long typeId = packet.Decode4();
		unsigned long id = packet.Decode4();
		packet.Decode1();

		if (!(typeId & 0x2000000))
		{
			AfxGetTask()->GetActor("Item")
				<< MsgObject(NULL, M983(0x19f), -1, typeId, id, 0, 0);
		}
		break;
	}
	case 22:
	{
		FrRecentMatch* pDlg = (FrRecentMatch*)g_pFresh->GetManager()
								  ->GetDesktop()
								  ->FindChildByName("GHOST_HISTORY");
		if (pDlg == NULL)
		{
			__rtti_obj = pDlg;
			break;
		}

		unsigned char result = packet.Decode1();
		if (result != 0xff)
		{
			unsigned long count = packet.Decode4();
			for (unsigned long i = 0; i < count; i++)
			{
				FrRecentMatch::sRecentMatchForm form;
				form.gender = packet.Decode1();
				form.name = packet.DecodeStr();
				form.win = packet.Decode4();
				packet.DecodeBuffer(&form.time, sizeof(form.time));
				form.score = packet.Decode4();
				form.grade = packet.Decode1();
				pDlg->AddHistory(form);
			}
			pDlg->AddFinish();
		}
		else
		{
			pDlg->Exception();
		}
		break;
	}
	}
}

bool CGhostHandler::OnResultGhostGameDlg(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrGhostGameMapDlg* pDlg = DYNAMIC_CAST(FrGhostGameMapDlg, pForm);
		GHOST_GAME_BRIEF* pBrief = (GHOST_GAME_BRIEF*)pDlg->GetSelectItem();
		if (pBrief)
		{
			std::string fileName(MakeGhostGameFileName(pBrief));

			CGhostManager::Instance()->LoadDocument(fileName);
			CGhostManager::Instance()->SetRecentSelectedGhostNick(
				pDlg->GetGhostNick());

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		}
		else
		{
			OpenNotifyDlg(4);
		}
	}

	return true;
}

bool CGhostHandler::OnResultEventGhostGameDlg(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrGhostGameMapDlg* pDlg = DYNAMIC_CAST(FrGhostGameMapDlg, pForm);
		GHOST_GAME_BRIEF* pBrief = (GHOST_GAME_BRIEF*)pDlg->GetSelectItem();
		if (pBrief)
		{
			std::string fileName(MakeGhostGameFileName(pBrief));
			CGhostManager::Instance()->SetRecentSelectedGhostNick(
				pDlg->GetGhostNick());
			CGhostManager::Instance()->LoadDocumentFromLocalFile(fileName);
		}
		else
		{
			OpenNotifyDlg(4);
		}
	}

	return true;
}

void CGhostHandler::ClearLocalGhostFiles()
{
	WIN32_FIND_DATA findData;

	HANDLE hFind = FindFirstFile(".\\temporary_save\\*.GST", &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			DeleteFile(findData.cFileName);
		} while (FindNextFile(hFind, &findData));

		FindClose(hFind);
	}
}

void CGhostHandler::SendErrorReport(const std::string& error,
	const std::string& fileName)
{
	WSendPacket packet((enumClientPacket)0xc2);
	packet.Encode1(8);
	packet.EncodeStr(fileName);
	packet.EncodeStr(error);
	packet.Send((eSendTo)0);
}

void CGhostHandler::ExceptionFinishGame()
{
	CGhostManager::Instance()->ReleaseDocument();
	CProjectG::Instance()->FromGolfToLobbySettings();
	OpenNotifyDlg(5);
	CTaskManager::Instance()->ChangeTask("CGhostTask", "", false);
	CTaskManager::Instance()->PostMsg(NULL, "Ghost", 0, (int)"GHOST", 1, 0, 0);
}

void CGhostHandler::RequestGhostGameList(unsigned long uid)
{
	if (IsLocalContent(S4_GHOST) && !(Doc()->m_curChannel.Type & 0x800))
	{
		if (IsLocalContent(S4_MATCHING_SYSTEM))
		{
			WSendPacket packet((enumClientPacket)0xc1);
			packet.Encode1(2);
			packet.Send((eSendTo)0);
		}

		WSendPacket packet((enumClientPacket)0xc2);
		packet.Encode1(0);
		packet.Encode4(uid);
		packet.Send((eSendTo)0);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
	}
}
