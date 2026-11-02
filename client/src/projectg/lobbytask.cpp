#include "minatl.h"
#include "vc10_layout.h"
#include "lobbytask.h"
#include "taskmain.h"
#include "actor.h"
#include "fresh.h"
#include "golfdoc.h"
#include "s5/utilities.h"
#include "guildactor.h"
#include "gatewayactor.h"
#include "taskmanager.h"
#include "packet.h"
#include "networksystem.h"
#include "logininfo.h"
#include "clientsetting.h"
#include "messengerdlg.h"
#include "mousecursor.h"
#include "user_info.h"
#include "matchingsystem.h"
#include "frticketexchangedlg.h"
#include "contentsdoc.h"
#include "chatmsg.h"
#include "hatmanager.h"
#include "intrusion.h"
#include "jp/refresh/rflobbydesktop.h"	// 005fbe40 no caso 0x96
#include "packets/lsloginackpostprocessor.h"	// PKT (frente C)

// HACK: tmpl order: the original instantiates vector<sGuildMatchup>'s iterators before list<sGuildMatchFlagInfo>'s.
typedef char
	RbGuildMatchupIterSize[sizeof(std::vector<sGuildMatchup>::iterator)];

extern Fresh* g_pFresh;
extern int g_iRecvPacketType;
extern HWND g_hwnd;

int __fastcall IsMassGame(unsigned char gameType);
int __fastcall IsMassGame();
// HACK: SharedDoc's original IsMassGame(void) is named IsTourney in the delinked object.
#pragma comment(linker, "/alternatename:?IsMassGame@@YIHXZ=?IsTourney@@YIHXZ")
void __fastcall RequestUccInfo(sCharacterInfo& info);
void __fastcall RequestUccInfo(sUserInfo& info);
unsigned char __fastcall GetEndHole();

// HACK: retain the epsilon constant emitted by the original math inlines.
static bool IsWithinEpsilon(float value)
{
	return Wabs(value) < g_EPSILON;
}

// Same definition as in intrusion.cpp: the original's COMDATs of this unit are discarded as its duplicates.
struct sFindRival
{
	unsigned long oid;

	sFindRival(unsigned long _oid)
		: oid(_oid)
	{
	}
	bool operator()(sRivalData& rival) { return rival.oid == oid; }
};

void __fastcall RecvRSS(int);

// HACK: PCH inlines in the original (emitted after the main pass); the dead references make them wait one wave.
inline void CSharedDoc::SetTimeBlock(int timeBlock)
{
	if (0)
		RecvRSS(0);
	m_myInfo.info.iTimeBlock = timeBlock;
}

inline unsigned long CSharedDoc::GetFriendGameSvrUID()
{
	if (0)
		RecvRSS(0);
	return m_friendGameSvrUID;
}

inline void CSharedDoc::SetForceNicknameChange(unsigned char change)
{
	if (0)
		((CSharedDoc*)0)->CSharedDoc::GetFriendGameSvrUID();
	m_forceNicknameChange = change;
}

void __fastcall RecvRSS(int)
{
}

IMPLEMENT_OBJECT(CLobbyTask, CTask)

CLobbyTask::CLobbyTask()
{
	m_bSyncLoad = true;
	IActor::m_defaultSkipLayer = 0;
	IActor::m_pLayer = NULL;
}

CLobbyTask::~CLobbyTask()
{
}

void CLobbyTask::Init(const char* wallPaper)
{
	CTask::Init(NULL);

	SendMessage(g_hwnd, 0xadb4, 0, 0);
}

void CLobbyTask::Register()
{
	// 983 00abfda0: o boneco, o caddie e o mascote do lobby sao atores (frente B, jp/lobby*.cpp); o LobbyPlayer avisa o
	// "Lobby" (0x57) ao carregar e o RFLobbyDesktop fecha o "Loading..."; sem o ator de guilda do 645
	SetMainActor(AddActor("CLobbyMain", "Lobby", 0, ""));
	AddActor("LobbyPlayer", "LobbyAvatar", 0, "");
	AddActor("LobbyCaddie", "LobbyCaddie", 0, "");
	AddActor("LobbyMascot", "LobbyMascot", 0, "");
	AddActor("ntLobbyBg", "LobbyBg", 0, "");
	AddActor("CItemBuff", "ItemBuff", 0, "");
	GOLFDOC()->m_currentHole = 0x13;
	AddActor("CNPC", "NPC", 0, "");
	AddActor(s_gatewayActor[0], s_gatewayActor[1], 0, GetRTTI()->GetName());
}

void CLobbyTask::Load()
{
	SetProgressBar(false);
	SetTip(false);

	CTask::Load();
}

void CLobbyTask::Process(float delta)
{
	g_pFresh->OnProcess(delta);
	CTask::Process(delta);
}

void CLobbyTask::Display()
{
	if (!Doc()->m_bBackgroundVideo)
		CTask::Display();

	g_pFresh->OnDisplay(Doc()->m_bBackgroundVideo != 0);
}

int CLobbyTask::OnPacket(WReceivedPacket& packet)
{
	unsigned short type = packet.Decode2();

	g_iRecvPacketType = type;

	// HACK: dead calls preserve the original MsgObject ctor inlining budget.
	// Removing this IL padding expands additional ctor call sites.
	if (0)
	{
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
		AfxGetTask();
	}

	switch (type)
	{
	case 0xc4:
	{
		unsigned long uid = packet.Decode4();
		unsigned char sub = packet.Decode1();

		if (uid == Doc()->m_myInfo.info.dwGuid)
			break;

		switch (sub)
		{
		case 0:
		{
			unsigned long value;

			packet.DecodeBuffer(&value, 4);
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x5b), uid, sub, (int)&value, 0, 0);
			break;
		}

		case 1:
		{
			std::string name;
			packet.DecodeStr(name);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x5b), uid, sub, (int)name.c_str(), 0, 0);
			break;
		}

		case 2:
		case 3:
			GetActor("Lobby") << MsgObject(NULL, M983(0x5b), uid, sub, 0, 0, 0);
			break;

		default:
			break;
		}

		break;
	}

	case PKT645_SEM_983(0x11b):
	{
		unsigned char idx = packet.Decode1();

		packet.Decode1();

		if (idx == 3)
		{
			packet.DecodeBuffer(Doc()->m_tutorialComplete, 0xc);
			break;
		}

		if (idx > 2)
			break;

		Doc()->m_tutorialComplete[idx] = packet.Decode4();
		break;
	}

	case 0xb2:
	{
		// 983 (00ac05d5): a lista vem em partes: total, quantos ja vieram, quantos neste pacote (Decode4);
		// so limpa a lista quando este pacote fecha o total.
		unsigned long total = packet.Decode4();
		unsigned long received = packet.Decode4();
		unsigned long count = packet.Decode4();

		if (total == count + received)
			Doc()->m_noteList.clear();

		for (unsigned long i = count; i != 0; i--)
		{
			sNoteInfo note;

			packet.DecodeBuffer(&note, 0x6d);
			Doc()->m_chatManager.FilteringHack(note.sNote, false);
			Doc()->m_noteList.push_back(note);
		}

		break;
	}

	case 0x84:
	{
		unsigned char flags = packet.Decode1();
		unsigned char isGM = flags >> 7;
		flags &= 0x7f;
		bool isSelf = flags == 0;
		unsigned long color = 0xffd2a2ff;
		if (isGM)
			color = 0xff6eecfb;

		std::string from;
		std::string msg;
		std::string text;

		packet.DecodeStr(from);
		packet.DecodeStr(msg);

		Doc()->m_chatManager.FilteringHack(msg.c_str(), false);

		if (isSelf)
		{
			text = Doc()->m_myInfo.info.sNick; // 983 00ac03a0: +0x7b8, nao o login +0x7a2
			text += ">";
			text += from;
		}
		else
		{
			if (!COption::Instance()->gIsWhisperAllowed())
				break;

			std::list<std::string>::iterator it = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), from);
			if (it != Doc()->m_ignoreList.end())
				break;

			const char* chatLine;
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x3f), (int)&chatLine, 0, 0, 0, 0);
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x38), (int)from.c_str(), 0, 0, 0, 0);

			text = from + ">" + Doc()->m_myInfo.info.sNick; // 983 00ac03a0: +0x7b8
		}

		text += " : \b'";
		text += msg;
		text += "'";

		if (AfxGetTask()->IsRecieveWhisper())
		{
			std::string plain;
			CChatMsg::Instance()->RemoveColor(text, plain);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x3), (int)plain.c_str(), color, 0, 0, 0);
		}
		else
		{
			WSendPacket sendPacket((enumClientPacket)0xde);
			sendPacket.EncodeStr(from);
			sendPacket.Send((eSendTo)0);
		}

		break;
	}

	case 0xb0:
	{
		std::string from;
		std::string msg;
		std::string text;

		packet.DecodeStr(from);
		packet.DecodeStr(msg);

		if (COption::Instance()->gIsWhisperAllowed())
		{
			std::list<std::string>::iterator it = std::find(
				Doc()->m_ignoreList.begin(), Doc()->m_ignoreList.end(), from);
			if (it != Doc()->m_ignoreList.end())
				break;

			Doc()->m_chatManager.FilteringHack(msg.c_str(), false);

			text = from + ">" + K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4");
			text += " : ";
			text += msg;

			std::string plain;
			CChatMsg::Instance()->RemoveColor(text, plain);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x3), (int)plain.c_str(), 0xff00c6ff, 0, 0, 0);
		}

		break;
	}

	case 0x4a:
	{
		unsigned short roomGuid = packet.Decode2();
		std::string title;

		Doc()->m_roomInfo.gameType = packet.Decode1();
		Doc()->m_roomInfo.mapType = packet.Decode1();
		Doc()->m_roomInfo.nHole = packet.Decode1();
		Doc()->m_roomInfo.holeType = packet.Decode1();

		// 983 (00ac0d34): o tipo de buraco 4 traz um byte (guardado cifrado em CSharedDoc+0x115f8) e um valor que
		// vai para o lobby na mensagem 0x31c (nova no 983, sem par no 645)
		if (Doc()->m_roomInfo.holeType == 4)
		{
			unsigned char repeatHole = packet.Decode1();
			unsigned long repeatValue = packet.Decode4();

			Doc()->m_fixHole983 = repeatHole;
			GetActor("Lobby") << MsgObject(NULL, 0x31c /* 983; sem par no 645 */, repeatValue, 0, 0, 0, 0);
		}

		Doc()->m_roomInfo.roomFlags983 = packet.Decode4();	// 983: sRoomInfo+0xc9 (00ac0dd4)
		Doc()->m_roomInfo.nUserLimit = packet.Decode1();
		Doc()->m_roomInfo.nGalleryLimit = packet.Decode1();
		Doc()->m_roomInfo.bSleep = packet.Decode1() == 1;
		Doc()->m_roomInfo.shotTimeLimit = packet.Decode4();
		Doc()->m_roomInfo.gameTimeLimit = packet.Decode4();
		Doc()->m_roomInfo.tidMatch = packet.Decode4();
		Doc()->m_roomInfo.bPublic = packet.Decode1() == 1;

		// 983 (00ac0e72): a senha nao vem mais; o titulo ocupa os 0x40 bytes do comeco da sRoomInfo
		// (title + password do nosso cabecalho; pedido C -> A)
		packet.DecodeStr(title);

		Doc()->m_chatManager.FilteringHack(title.c_str(), false);

		memset(Doc()->m_roomInfo.title, 0, 0x40);
		strncpy(Doc()->m_roomInfo.title, title.c_str(), 0x3f);

		if (strlen(Doc()->m_roomInfo.title) < strlen(title.c_str()))
		{
			CProjectG::Instance()->ReportError(6,
				"[%d_%d]_%d_%d_%d_%d_%d_%d(%d_%d)", strlen(title.c_str()),
				Doc()->m_roomInfo.bPublic, roomGuid, Doc()->m_roomInfo.gameType,
				Doc()->m_roomInfo.realGameType, Doc()->m_roomInfo.mapType,
				Doc()->m_roomInfo.nHole, Doc()->m_roomInfo.nUserLimit,
				Doc()->m_roomInfo.roomGuid, Doc()->m_myInfo.info.dwGuid);
		}

		int lobbyState;
		GetActor("Lobby") << MsgObject(NULL, M983(0x17), (int)&lobbyState, 0, 0, 0,
			0);

		// 983 (00ac0fec): no lugar do nome do layout, o estado de CSharedDoc+0x11638 (leitor 0062bf90)
		unsigned long roomState = Doc()->m_lobbyState983;

		if (roomState == 3 || roomState == 4 || roomState == 7 || lobbyState == 0)
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
			GetActor("Lobby") << MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0);
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x11), Doc()->m_roomInfo.mapType, 0, 0, 0, 0);
			GetActor("Lobby") << MsgObject(NULL, M983(0x12),
				Doc()->m_roomInfo.holeType, 0, 0, 0, 0);
			GetActor("Lobby") << MsgObject(NULL, M983(0x2f), 0, 0, 0, 0, 0);
		}

		break;
	}

	case 0x49:
	{
		CIntrusion::Instance()->FinishGame();

		GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
			Doc()->m_inviteMode = 0;

		unsigned char result = packet.Decode1();

		if (result == 0)
		{
			if (packet.Decode1() == 0)
			{
				Doc()->m_myInfo.info.dwIdentity &= ~2;
				Doc()->m_myInfo.info.dwGalleryGuid = 0xffffffff;
			}
			else
			{
				Doc()->m_myInfo.info.dwIdentity |= 2;
				Doc()->m_myInfo.info.dwGalleryGuid = packet.Decode4();
			}

			Doc()->m_slotList.clear();
			Doc()->m_rivalList.clear();
			Doc()->m_guildMatchupList.clear();
			Doc()->m_bGameOver = false;

			packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));	// 983: 0xdd (era 0xb2 do 645)
			Doc()->m_chatManager.FilteringHack(Doc()->m_roomInfo.title, false);

			Doc()->m_myInfo.roomIndex = Doc()->m_roomInfo.roomGuid;

			std::map<unsigned long, sUserInfoTime>::iterator userIt;
			if (Doc()->FindUserInfoTimeGUID(Doc()->m_myInfo.info.dwGuid,
					userIt))
				userIt->second.info.roomIndex = Doc()->m_roomInfo.roomGuid;

			Doc()->m_chatLineList.clear();

			if (WNetworkSystem::Instance()->IsConnected(
					(WNetworkSystem::eNetUnit)1) &&
				(int)CMessengerInfo::Instance()->GetComplete() == 4)
			{
				sUserPosition position;
				position.iServerGUID = Doc()->m_gameServerUID;
				position.iChannelUid = Doc()->m_curChannel.Uid;
				wsprintf(position.csChannelName, "%s",
					Doc()->m_curChannel.Name);
				position.iRoomIdx = Doc()->m_roomInfo.roomGuid;
				position.iRoomType = Doc()->m_roomInfo.gameType;

				WSendPacket sendPacket(0x23);
				sendPacket.EncodeBuffer(&position, 0x4b);
				sendPacket.Send((eSendTo)1);
			}

			if (Doc()->m_roomInfo.gameType == 2)
			{
				CMouseCursor::Instance()->SetActive(false);
				COption::Instance()->vSetMinimumScreenSize(1, 800, 600);
				CTaskManager::Instance()->ChangeTask("ntAvatarChatTask",
					"CGolfDoc", false);
				CTaskManager::Instance()->PostMsg(NULL, "AvatarChat", 0,
					(int)"AVATARCHAT_MAIN", 0, 0, 0);
				break;
			}

			if (Doc()->m_roomInfo.realGameType == GAME_TYPE_USEMAX &&
				Doc()->m_roomInfo.masterUID == (int)Doc()->m_myInfo.info.dwUID)
			{
				std::list<sItemInfo>::iterator it;

				for (it = Doc()->m_myItemList.begin();
					it != Doc()->m_myItemList.end(); ++it)
				{
					if (it->tid == 0x1a0000f7 && it->Common[0] > 0)
					{
						it->Common[0]--;

						if (it->Common[0] < 1)
							Doc()->m_myItemList.erase(it);

						break;
					}
				}
			}

			// 983 00ac145c: sem o layout "GAMEROOM"/"GAMEROOM_EXT" do 645 (que trocava a tela e destruía o RFLobbyDesktop, C101.1);
			// a janela da sala abre por cima do lobby pela msg 0x8b (CLobbyMain: 6 = normal -> OpenGameRoomDlg983(3), 7 = mass,
			// 10 = Grand Prix, 0xb = mass com realGameType 0xc/0xe/0x13 [00500200] + msg 0x2f = M983(0x2e))
			int roomDlgType = 6;
			bool bMassExt = false;
			if (IsMassGame(Doc()->m_roomInfo.gameType))
			{
				unsigned char realType = Doc()->m_roomInfo.realGameType;
				if (realType == 0xc || realType == 0xe || realType == 0x13)
				{
					roomDlgType = 0xb;
					bMassExt = true;
				}
				else
					roomDlgType = IsGrandPrixGameType983(realType) ? 10 : 7;
			}
			GetActor("Lobby") << MsgObject(NULL, 0x8b, roomDlgType, 0, 0, 0, 0);
			if (bMassExt)
				GetActor("Lobby") << MsgObject(NULL, M983(0x2e), 0, 0, 0, 0, 0); // 983 0x2f
		}
		else
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x3b), result, 0, 0, 0, 0);
		}

		break;
	}

	case 0x82:
	{
		std::list<sSlotInfo>& slots = Doc()->m_slotList;
		std::list<sSlotInfo>::iterator it;
		unsigned long guid = packet.Decode4();

		if (guid != 0xffffffff)
		{
			if (guid == Doc()->m_myInfo.info.dwGuid)
			{
				// 983 00ac156f: expulso/saiu: fecha a sala pelo 0x8b 4 (o layout ROOMLIST do 645 nao existe no 983)
				AfxPostMsg(NULL, "Lobby", M983(0x2), 0, 0, 0, 0);
				AfxPostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
				AfxPostMsg(NULL, "Lobby", 0x8b, 4, 0, 0, 0);
			}
			else
			{
				it = std::find(slots.begin(), slots.end(), guid);

				if (it != slots.end())
				{
					GetActor("Lobby")
						<< MsgObject(NULL, M983(0x8), (int)&*it, 0, 0, 0, 0);
					slots.erase(it);
				}
			}
		}
		break;
	}

	case 0xa8:
	{
		unsigned long guid = packet.Decode4();
		std::list<sSlotInfo>& slots = Doc()->m_slotList;
		std::list<sSlotInfo>::iterator it = slots.begin();

		while (it != slots.end())
		{
			if (it->dwGuid == guid || (bool)((it->dwIdentity >> 2) & 1))
			{
				++it;
			}
			else
			{
				GetActor("Lobby") << MsgObject(NULL, M983(0x8), (int)&*it, 0, 0, 0, 0);
				it = slots.erase(it);
			}
		}

		Doc()->m_roomInfo.nUserNum = slots.size();

		GetActor("Lobby") << MsgObject(NULL, M983(0x2f), 0, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0);
		break;
	}

	case 0xae:
	{
		std::list<sSlotInfo>& slots = Doc()->m_slotList2;
		unsigned char sub = packet.Decode1();

		switch (sub)
		{
		case 2:
		{
			unsigned long guid = packet.Decode4();
			std::list<sSlotInfo>::iterator it;
			it = std::find(slots.begin(), slots.end(), guid);

			if (it != slots.end())
				slots.erase(it);
			break;
		}

		case 1:
		{
			sSlotInfo slot;

			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)

			std::list<sSlotInfo>::iterator it;
			it = std::find(slots.begin(), slots.end(), slot.dwGuid);

			if (it == slots.end())
				slots.push_back(slot);
			break;
		}

		case 0:
		{
			slots.clear();

			unsigned char count = packet.Decode1();
			sSlotInfo slot;

			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
				slots.push_back(slot);
			}

			GetActor("Lobby") << MsgObject(NULL, M983(0x2f), 0, 0, 0, 0, 0);
			GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);

			if (Doc()->m_slotList.size() == 0)
				GetActor("Lobby") << MsgObject(NULL, M983(0xe), 0, 0, 0, 0, 0);
			break;
		}
		}

		GetActor("Lobby") << MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0);
		break;
	}

	case 0x48:
	{
		std::list<sSlotInfo>* slots = &Doc()->m_slotList;
		unsigned char sub = packet.Decode1();
		packet.Decode2();

		switch (sub)
		{
		case 0:
		case 5:
		{
			if (sub == 0)
			{
				slots->clear();
				GetActor("Lobby") << MsgObject(NULL, M983(0x59), 0, 0, 0, 0, 0);
			}

			unsigned char count = packet.Decode1();
			sSlotInfo slot;
			sCharacterInfo charInfo;

			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
				slots->push_back(slot);

				if (!IsMassGame(Doc()->m_roomInfo.gameType))
				{
					packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));	// 983: 0x201 (era 0x1bc do 645)

					if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
						RequestUccInfo(charInfo);

					GetActor("Lobby") << MsgObject(NULL, M983(0x5a), (int)&slot,
						(int)&charInfo, 0, 0, 0);
				}
			}

			if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
			{
				unsigned char guests = packet.Decode1();

				for (int i = 0; i < guests; i++)
				{
					packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)
					slots->push_back(slot);
				}
			}

			if (sub == 0)
			{
				std::map<unsigned int, sCaddieInfo>& caddies =
					Doc()->m_caddieMap;
				std::map<unsigned int, sCaddieInfo>::iterator caddieIt;
				caddieIt = caddies.find(Doc()->m_myInfo.userEquip.guidCaddie);
				if (caddieIt == caddies.end())
					GetActor("Lobby") << MsgObject(NULL, M983(0x13), 0, 0, 0, 0, 0);
				else
					GetActor("Lobby") << MsgObject(NULL, M983(0x13),
						caddieIt->second.tid, 1, 0, 0, 0);
				GetActor("Lobby") << MsgObject(NULL, M983(0x14),
					Doc()->m_myInfo.userEquip.guidClubSet, 0, 0, 0, 0);
				GetActor("Lobby") << MsgObject(NULL, M983(0x15),
					Doc()->m_myInfo.userEquip.tidBall, 0, 0, 0, 0);
			}

			GetActor("Lobby") << MsgObject(NULL, M983(0xe), 0, 0, 0, 0, 0);
			break;
		}
		case 1:
		{
			sSlotInfo slot;
			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)

			if (std::find(slots->begin(), slots->end(), slot.dwGuid) ==
				slots->end())
			{
				slots->push_back(slot);
				std::list<sSlotInfo>::iterator last = slots->end();
				--last;
				sSlotInfo* added = &*last;

				if (IsMassGame(Doc()->m_roomInfo.gameType))
				{
					GetActor("Lobby")
						<< MsgObject(NULL, M983(0x7), (int)added, 0, 0, 0, 0);
				}
				else
				{
					sCharacterInfo charInfo;
					charInfo.guid = slot.dwGuid;

					if (!slot.IsInvite)
					{
						packet.DecodeBuffer(&charInfo, sizeof(sCharacterInfo));	// 983: 0x201 (era 0x1bc do 645)

						if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
							RequestUccInfo(charInfo);
					}

					GetActor("Lobby") << MsgObject(NULL, M983(0x7), (int)added,
						(int)&charInfo, 0, 0, 0);
				}
			}
			break;
		}
		case 2:
		{
			unsigned long guid = packet.Decode4();
			std::list<sSlotInfo>::iterator it;
			it = std::find(slots->begin(), slots->end(), guid);

			if (it != slots->end())
			{
				sSlotInfo* slot = &*it;

				if (slot == NULL)
				{
					slots->erase(it);
				}
				else
				{
					guid = slot->dwGuid;
					unsigned char team = slot->bTeam;
					unsigned char rank = slot->connectionRank;

					slots->erase(it);

					for (it = slots->begin(); it != slots->end(); ++it)
					{
						if (rank < it->connectionRank)
							it->connectionRank--;
					}

					GetActor("Lobby")
						<< MsgObject(NULL, M983(0x8), (int)slot, guid, team, 0, 0);
				}
			}
			break;
		}
		case 3:
		{
			sSlotInfo slot;
			unsigned long guid = packet.Decode4();
			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c (era 0x152 do 645)

			if (std::find(slots->begin(), slots->end(), guid) != slots->end())
				GetActor("Lobby") << MsgObject(NULL, M983(0x9), (int)&slot, 0, 0, 0, 0);
			break;
		}
		}

		Doc()->m_roomInfo.nUserNum = slots->size();

		GetActor("Lobby") << MsgObject(NULL, M983(0x2f), 0, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0);

		if (Doc()->m_roomInfo.gameType != 6)
			break;

		GetActor("Lobby") << MsgObject(NULL, M983(0x71), 0, 0, 0, 0, 0);
		break;
	}

	case 1:
	{
		// PKT 983: 00ac1e9a - a resposta do login saiu daqui para a CLSLoginAckPostProcessor (corpo no fim do arquivo)
		CLSLoginAckPostProcessor ack(this);
		int error = ack.Process(packet);

		if (error)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /* 983; sem par no 645 */, error, 0, 0, 0, 0));

		return 1;
	}

	case 0xb:
		packet.DecodeStr(Doc()->m_authTicket);
		break;

	case 0x11:
	{
		// PKT 983: 00ac1f2e
		unsigned char result = packet.Decode1();

		if (result == 0)
			break;

		if (result != 0xe2)
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc4\xb3\xb8\xaf\xc5\xcd \xbc\xb3\xc1\xa4\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe7\xc1\xa2\xbc\xd3\xc7\xcf\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		int error = packet.Decode4();

		if (error)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /* 983; sem par no 645 */, error, 0, 0, 0, 0));
		break;
	}

	case PKT645_SEM_983(0x1cd):
	{
		GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		unsigned long result = packet.Decode4();
		IActor* gateway = GetActor(s_gatewayActor[1]);

		if (result == 0)
		{
			gateway << MsgObject(NULL, M983(0x26d), 8, 0, 0, 0, 0);
			gateway << MsgObject(NULL, M983(0x26d), 4, result, 0, 0, 0);
		}
		else
		{
			gateway << MsgObject(NULL, M983(0x26d), 1, result, 0, 0, 0);
		}

		break;
	}

	case PKT645_SEM_983(0x1ce):
	{
		GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		unsigned long result = packet.Decode4();
		IActor* gateway = GetActor(s_gatewayActor[1]);

		if (result == 0)
		{
			gateway << MsgObject(NULL, M983(0x26d), 8, 0, 0, 0, 0);
			gateway << MsgObject(NULL, M983(0x26d), 0xe, result, 0, 0, 0);
		}
		else
		{
			gateway << MsgObject(NULL, M983(0x26d), 0xb, result, 0, 0, 0);
		}

		break;
	}

	case 0x10:
	{
		std::string key = packet.DecodeStr();

		Doc()->SetLoginAuthKey(key);
		break;
	}

	case 0xf:
	{
		Doc()->m_bTutorialResume = true;
		Doc()->m_bTutorialRedirect = true;
		Doc()->m_myInfo.info.gender = packet.Decode1();

		std::string name;
		packet.DecodeStr(name);

		if (stricmp(name.c_str(), Doc()->m_myInfo.info.sID) != 0)
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xc, 0, 0, 0, 0);
		}
		else
		{
			// PKT 983: 00ac212e - os mesmos quatro campos de bloqueio da resposta do login (005a27e3)
			Doc()->m_provType = packet.Decode4();
			Doc()->m_unused184 = packet.Decode4();
			Doc()->m_unusedString188 = packet.DecodeStr();
			packet.DecodeStr(Doc()->m_paymentKID983);
		}
		break;
	}

	case 2:
	{
		GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		// C98: 983 00ac21f0 sem a pergunta 0x26d do 645 ao Gateway (senha secundaria, nao existe no 983: o flag ficava false e
		// a lista era descartada = "Loading Server List" parado). Na volta ao servidor de login (+0x11250) limpa as variaveis e
		// so abre o SERVERLIST se o Lobby nao tiver a janela de servidores (CTaskMain::m_pServerDlg, +0x15c no 983).
		bool serverList = true;
		if (*(bool*)&Doc()->m_bReturnToLogin983)
		{
			struct CSharedDocAcc983 : public CSharedDoc { using CSharedDoc::ClearServerVars; }; // protegido (983 0063b020)
			((CSharedDocAcc983*)Doc())->ClearServerVars();
			// D894: +0x15c cru do VC10 -> membro (no cl 14.44 o IActor/FrCmdTarget tem STL e o membro anda)
			struct CTaskMainAcc983 : public CTaskMain { using CTaskMain::m_pServerDlg; }; // protegido
			C_ASSERT_VC10(offsetof(CTaskMainAcc983, m_pServerDlg) == 0x15c);
			serverList = ((CTaskMainAcc983*)static_cast<CTaskMain*>(GetActor("Lobby")))->m_pServerDlg == NULL;
		}
		if (serverList)
			AfxGetTask()->GetActor("Lobby")
				<< MsgObject(NULL, 0, (int)"SERVERLIST", 0, 0, 0, 0);

		Doc()->m_gameServerList.clear();

		unsigned char count = packet.Decode1();
		sGameServerInfo info;

		for (int i = 0; i < count; i++)
		{
			memset(&info, 0, sizeof(info));
			packet.DecodeBuffer(&info, 0x5c);
			if (Doc()->m_failedGameServerIds983.find(info.id) == Doc()->m_failedGameServerIds983.end()) // 983: servidores do conjunto +0x10a20 ficam fora
				Doc()->m_gameServerList.push_back(info);
		}

		GetActor("Lobby") << MsgObject(NULL, M983(0x19), 0, 0, 0, 0, 0);
		break;
	}

	case 9:
	{
		CMessengerInfo::Instance()->m_serverList.clear();

		MSNServerInfo info;
		unsigned char count = packet.Decode1();

		for (int i = 0; i < count; i++)
		{
			memset(&info, 0, sizeof(info));
			packet.DecodeBuffer(&info, 0x5c);
			CMessengerInfo::Instance()->m_serverList.push_back(info);
		}
		break;
	}

	case 4:
		Doc()->m_needParentAgree = packet.Decode1();
		break;

	case 3:
		// PKT 983: 00ac23b7 - resultado (0 = ok) + texto que vai para CSharedDoc+0x11254; avisa o ator principal com a mensagem 0x1b2
		if (packet.Decode4() != 0)
			break;

		Doc()->m_loginToken983 = packet.DecodeStr();

		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_LOGIN))
			WNetworkSystem::Instance()->ForceShutDown(WNetworkSystem::NET_LOGIN);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1b2 /* 983; sem par no 645 */, 0, 0, 0, 0, 0));
		break;

	case 5:
	{
		if (packet.Decode1() != 3)
			break;

		unsigned long count = packet.Decode4();
		unsigned long tid = packet.Decode4();

		if (count == 0)
			break;

		if (tid == 0)
			break;

		char msg[0x400] = { 0 };
		IFF_STRUCT::sItem* item = Doc()->m_itemManager.FindItem(tid);

		if (item != NULL)
			sprintf(msg,
				K2L_Compatibility(" PC\xb9\xe6 \xc3\xe2\xbc\xae \xc0\xcc\xba\xa5\xc6\xae\n\n \\cT\\c\\c0xffff0000\\c%d \\c0xff000000\\c\xb9\xf8 \xc0\xc0\xb8\xf0\n \\cT\\c\\c0xffff0000\\c%s \\c0xff000000\\c\xbe\xc6\xc0\xcc\xc5\xdb \xc1\xf6\xb1\xde"),
				count, item->c.Name);
		else
			sprintf(msg,
				K2L_Compatibility(" PC\xb9\xe6 \xc3\xe2\xbc\xae \xc0\xcc\xba\xa5\xc6\xae\n\n \\cT\\c\\c0xffff0000\\c%d \\c0xff000000\\c\xb9\xf8 \xc0\xc0\xb8\xf0\n \\cT\\c\\c0xffff0000\\c\xbe\xc6\xc0\xcc\xc5\xdb \\c0xff000000\\c \xc1\xf6\xb1\xde"),
				count);

		GetActor("Lobby") << MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0);
		break;
	}

	case 6:
	{
		sOption option;
		char macroBuf[0x240];

		packet.DecodeBuffer(macroBuf, 0x240);
		COption::Instance()->mSetPCBangMascotMsg(macroBuf + 0x200);

		if (COption::Instance()->mGetSaveMacros() != 0)
		{
			int i = 0;
			char* p = macroBuf;

			do
			{
				COption::Instance()->mSetMacroText(i, p);
				i++;
				p += 0x40;
			} while (i < 8);

			COption::Instance()->mSetPCBangMascotMsg(macroBuf + i * 0x40);
		}
		break;
	}

	case 8:
	{
		CLoginInfo* loginInfo = CLoginInfo::Instance();
		loginInfo->EnablePcBang(packet.Decode4() == 1);
		break;
	}

	case 0xd:
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		// PKT 983: 00ac25c2 - codigo 0xc novo (numero de erro do servidor); textos pela tabela de traducao
		std::string msg;
		unsigned long code = packet.Decode4();

		if (code == 0xc)
		{
			int error = packet.Decode4();

			if (error)
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /* 983; sem par no 645 */, error, 0, 0, 0, 0));
			break;
		}

		switch (code)
		{
		case 0:
		{
			std::string name;
			packet.DecodeStr(name);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x249), 0, (int)name.c_str(), 0, 0, 0);
			break;
		}

		case 1:
			msg =
				K2L_Compatibility("\xb4\xeb\xc8\xad\xb8\xed \xbc\xb3\xc1\xa4\xc7\xcf\xb4\xc2 \xb0\xfa\xc1\xa4\xbf\xa1\xbc\xad \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 2:
			msg =
				K2L_Compatibility("\xc0\xcc\xb9\xcc \xbb\xe7\xbf\xeb\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb4\xeb\xc8\xad\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		case 3:
			msg =
				K2L_Compatibility("\xb1\xe6\xc0\xcc \xb6\xc7\xb4\xc2 \xc7\xfc\xbd\xc4\xc0\xcc \xb8\xc2\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 4:
			msg =
				K2L_Compatibility("\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc5\xb0\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 5:
			msg =
				K2L_Compatibility("\xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb4\xdc\xbe\xee\xb0\xa1 \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9..");
			break;
		case 6:
			msg =
				K2L_Compatibility("\xc7\xf6\xc0\xe7 DB\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 10:
			msg =
				K2L_Compatibility("\xb5\xee\xb7\xcf\xc7\xcf\xb7\xc1\xb4\xc2 \xb4\xd0\xb3\xd7\xc0\xd3\xc0\xcc \xc0\xcc\xb9\xcc \xc3\xbc\xc5\xa9\xc7\xd1 \xb4\xd0\xb3\xd7\xc0\xd3\xb0\xfa \xb5\xbf\xc0\xcf\xc7\xcf\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.");
			break;
		default:
		{
			char codeBuf[8] = { 0 };

			msg =
				K2L_Compatibility("\xbe\xcb \xbc\xf6 \xbe\xf8\xb4\xc2 \xc0\xcc\xc0\xaf\xb7\xce \xba\xaf\xb0\xe6\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. - Code : ");
			itoa(code, codeBuf, 10);
			msg += codeBuf;
			break;
		}
		}

		if (code != 0)
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x23), (int)msg.c_str(), 0, 0, 0, 0);
		break;
	}

	case 0xe:
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

		// PKT 983: 00ac285b - codigo 0xc novo (numero de erro do servidor); textos pela tabela de traducao
		std::string msg;
		unsigned long code = packet.Decode4();

		if (code == 0xc)
		{
			int error = packet.Decode4();

			if (error == 0)
				break;

			if (error != 5100065)
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /* 983; sem par no 645 */, error, 0, 0, 0, 0));
				break;
			}

			msg = K2L_Compatibility("\xc0\xcc\xb9\xcc \xbb\xe7\xbf\xeb\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb4\xeb\xc8\xad\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9.");
			GetActor("Lobby") << MsgObject(NULL, M983(0x23), (int)msg.c_str(), 0, 0, 0, 0);
			break;
		}

		switch (code)
		{
		case 0:
		{
			std::string name;
			packet.DecodeStr(name);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x249), 1, (int)name.c_str(), 0, 0, 0);

			if (IsLocalContent((localContentType_t)0x7e /*983: era 0x81; medido*/))
				GetActor("Lobby") << MsgObject(NULL, M983(0x6e), 0, 0, 0, 0, 0);

			msg =
				K2L_Compatibility("\xbb\xe7\xbf\xeb \xb0\xa1\xb4\xc9\xc7\xd1 \xb4\xeb\xc8\xad\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		}

		case 1:
			msg =
				K2L_Compatibility("\xb4\xeb\xc8\xad\xb8\xed\xc0\xbb \xb0\xcb\xbb\xe7\xc7\xcf\xb4\xc2 \xb0\xfa\xc1\xa4\xbf\xa1\xbc\xad \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 2:
			msg =
				K2L_Compatibility("\xc0\xcc\xb9\xcc \xbb\xe7\xbf\xeb\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb4\xeb\xc8\xad\xb8\xed\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		case 3:
			msg =
				K2L_Compatibility("\xb1\xe6\xc0\xcc \xb6\xc7\xb4\xc2 \xc7\xfc\xbd\xc4\xc0\xcc \xb8\xc2\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 4:
			msg =
				K2L_Compatibility("\xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xc4\xed\xc5\xb0\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 5:
			msg =
				K2L_Compatibility("\xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb4\xdc\xbe\xee\xb0\xa1 \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9..");
			break;
		case 6:
			msg =
				K2L_Compatibility("\xc7\xf6\xc0\xe7 DB\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 8:
			if (!IsLocalContent((localContentType_t)0x7e /*983: era 0x81; medido*/))
				break;

			GetActor("Lobby") << MsgObject(NULL, M983(0x6e), 0, 8, 0, 0, 0);
			msg =
				K2L_Compatibility("\xb5\xbf\xc0\xcf\xc7\xd1 \xb4\xd0\xb3\xd7\xc0\xd3\xc0\xbb \xb1\xd7\xb4\xeb\xb7\xce \xbb\xe7\xbf\xeb\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		default:
			break;
		}

		GetActor("Lobby") << MsgObject(NULL, M983(0x23), (int)msg.c_str(), 0, 0, 0,
			0);
		break;
	}

	case 7:	// PKT 983: 00ac2b0b (o mesmo corpo atende 0x7 e 0xd4)
	case 0xd4:
	{
		char buf[0x12c];
		memset(buf, 0, sizeof(buf));

		int count = packet.Decode4();

		for (int i = 0; i < count; i++)
			packet.DecodeBuffer(buf + i * 0x19, 0x19);

		if (count > 0)
			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x61), count, (int)buf, 0, 0, 0);
		break;
	}

	case 0x78:
	{
		unsigned long arg = packet.Decode4();
		unsigned char flag = packet.Decode1();

		GetActor("Lobby") << MsgObject(NULL, M983(0x29), arg, flag, 0, 0, 0);
		break;
	}

	case 0x6c:
	{
		if (!IsMassGame(Doc()->m_roomInfo.gameType))
			break;

		unsigned long uid = packet.Decode4();
		unsigned char state = packet.Decode1();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		switch (state)
		{
		case 3:
		{
			for (std::vector<sRivalData>::iterator rival =
					 Doc()->m_rivalList.begin();
				rival != Doc()->m_rivalList.end(); ++rival)
			{
				if (rival->state != 3)
					Doc()->m_rivalList[Doc()->GetIndex(uid)].quitOrder--;
			}

			std::list<sSlotInfo>::iterator it = std::find(
				Doc()->m_slotList.begin(), Doc()->m_slotList.end(), uid);

			if (it != Doc()->m_slotList.end() &&
				Doc()->m_rivalList[Doc()->GetIndex(uid)].state == 2)
				GetActor("Lobby") << MsgObject(NULL, M983(0x8), (int)&*it, 0, 0, 0, 0);
			break;
		}

		case 2:
		{
			std::list<sSlotInfo>::iterator it = std::find(
				Doc()->m_slotList.begin(), Doc()->m_slotList.end(), uid);

			if (it != Doc()->m_slotList.end())
				GetActor("Lobby") << MsgObject(NULL, M983(0x7), (int)&*it, 0, 0, 0, 0);
			break;
		}
		}

		Doc()->m_rivalList[Doc()->GetIndex(uid)].state = state;

		GetActor("Lobby") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		break;
	}

	case 0x6e:
	{
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		packet.Decode1();

		float pos[2];
		packet.DecodeBuffer(pos, 8);
		packet.Decode4();	// 983: campo novo, descartado (00ac2d99)

		unsigned long approachDistance;
		int approachTime;

		if (Doc()->m_roomInfo.gameType == 0xa)
		{
			approachDistance = packet.Decode4();
			approachTime = packet.Decode4();
		}

		packet.Decode2();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		unsigned char index = Doc()->GetIndex(uid);

		if (WisEqual(Doc()->m_rivalList[index].ballPos.x, pos[0], 0.00001f))
			break;

		if (WisEqual(Doc()->m_rivalList[index].ballPos.z, pos[1], 0.00001f))
			break;

		Doc()->m_rivalList[index].ballPos = WVector(pos[0], 0.0f, pos[1]);

		if (Doc()->m_roomInfo.gameType != 0xa)
			break;

		Doc()->m_rivalList[index].approachDistance = approachDistance;
		Doc()->m_rivalList[index].approachResultDistance = approachDistance;
		Doc()->m_rivalList[index].approachTime = approachTime;
		break;
	}

	case 0x6d:
	{
		if (!IsMassGame())
			break;

		unsigned long uid = packet.Decode4();
		unsigned char hole = packet.Decode1();
		unsigned char stroke = packet.Decode1();
		int score = packet.Decode4();
		__int64 totalPang = packet.Decode8();
		__int64 bonusPang = packet.Decode8();
		int flag = packet.Decode1();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		Doc()->LogGstUpdateHole(uid, hole, score, totalPang);

		std::vector<sRivalData>::iterator rival =
			std::find_if(Doc()->m_rivalList.begin(), Doc()->m_rivalList.end(),
				sFindRival(uid));
		if (rival == Doc()->m_rivalList.end())
			break;

		if (hole != rival->hole)
			break;

		if (flag == 1)
		{
			rival->holeStroke[hole - 1] = stroke - rival->totalStroke;
			rival->holeScore[hole - 1] = score - rival->totalScore;
			rival->holePang[hole - 1] = totalPang - rival->totalPang;
			rival->totalScore = score;
			rival->totalStroke = stroke;
			rival->hole = Doc()->m_holeOrder[GetHoleIndex(hole)];
		}

		rival->totalPang = totalPang;
		rival->totalBonusPang = bonusPang;

		if (Doc()->m_golfGame.gameType == 6)
			GetActor("Lobby") << MsgObject(NULL, M983(0x5e), uid, hole, 0, 0, 0);

		GetActor("Lobby") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		break;
	}

	case 0x79:
		if (Doc()->m_holeStatistics[0].dwExp != -1)
			break;

		Doc()->m_holeStatistics[0].dwExp = packet.Decode4();
		Doc()->m_roomType = packet.Decode4();
		Doc()->m_eventType = packet.Decode1();
		Doc()->m_eventFlag = packet.Decode1();

		if (Doc()->m_roomInfo.gameType == 6)
		{
			Doc()->m_guildBonusPang = packet.Decode4();
			Doc()->m_guildPoint = packet.Decode4();
		}

		packet.DecodeBuffer(Doc()->m_awardItem, 0x60);

		// 983 (00ac3171): o placar por equipe deixou de vir em dois Decode4 do modo 6 e passou a ser um bloco de
		// 0x18 bytes em CSharedDoc+0x106e4 (m_guildRoundScore ... m_unused47c8), fora dos modos 6 e 0xa.
		if (Doc()->m_golfGame.gameType != 6 && Doc()->m_golfGame.gameType != 0xa)
			packet.DecodeBuffer(Doc()->m_guildRoundScore, 0x18);

		GetActor("Lobby") << MsgObject(NULL, M983(0x2d), 0, 0, 0, 0, 0);
		break;

	case 0x7c:
	{
		std::list<sSlotInfo>& slots = Doc()->m_slotList;
		unsigned long guid = packet.Decode4();
		packet.Decode2();

		for (std::list<sSlotInfo>::iterator it = slots.begin();
			it != slots.end(); ++it)
		{
			if (it->dwGuid == guid)
			{
				it->bMaster = 1;
				it->bReady = 1;
			}
			else
			{
				it->bMaster = 0;
			}
		}

		GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0x5f), guid, 0, 0, 0, 0);
		break;
	}

	case 0x7d:
	{
		std::list<sSlotInfo>& slots = Doc()->m_slotList;
		unsigned long guid = packet.Decode4();
		unsigned char team = packet.Decode1();

		std::list<sSlotInfo>::iterator it;
		it = std::find(slots.begin(), slots.end(), guid);

		if (it == slots.end())
			break;

		it->bTeam = team;

		GetActor("Lobby") << MsgObject(NULL, M983(0x46), guid, team, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
		break;
	}

	case 0x7f:
	{
		unsigned char reason = packet.Decode1();
		std::string text;

		switch (reason)
		{
		case 1:
			text =
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xbd\xc3\xc0\xdb\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8 \xc7\xca\xbf\xe4\xc7\xd1 \xc0\xaf\xc0\xfa\xc0\xc7 \xbc\xf6\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 2:
			text =
				K2L_Compatibility("\xc6\xc0\xc0\xc7 \xc0\xce\xbf\xf8\xbc\xf6\xb0\xa1 \xbc\xad\xb7\xce \xb8\xc2\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 3:
			text =
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xbd\xc3\xc0\xdb\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8 \xc7\xca\xbf\xe4\xc7\xd1 Ready\xb5\xc8 \xc0\xaf\xc0\xfa\xc0\xc7 \xbc\xf6\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 4:
			text =
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xbd\xc3\xc0\xdb\xc7\xcf\xb1\xe2\xbf\xa1 \xc0\xce\xbf\xf8\xc0\xcc \xb3\xca\xb9\xab \xb8\xb9\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		default:
			break;
		case 6:
			text =
				K2L_Compatibility("\xc3\xb3\xc0\xbd\xbf\xa1 \xb0\xb6\xb7\xaf\xb8\xae\xc7\xd2 \xc0\xaf\xc0\xfa\xb0\xa1 \xbc\xb1\xc5\xc3\xb5\xc7\xbe\xee \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 7:
		case 8:
			text =
				K2L_Compatibility("\xb0\xd4\xc0\xd3 \xbd\xba\xc5\xb8\xc6\xae\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 9:
			text =
				K2L_Compatibility("\xc3\xd6\xbd\xc5 \xb9\xf6\xc1\xaf\xc0\xb8\xb7\xce \xbe\xf7\xb5\xa5\xc0\xcc\xc6\xae\xb0\xa1 \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9.");
			break;
		case 10:
			text =
				K2L_Compatibility("\xb4\xeb\xc8\xb8 \xbc\xad\xb9\xf6\xbf\xa1\xbc\xad \xc7\xc3\xb7\xb9\xc0\xcc \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xc4\xda\xbd\xba\xc0\xd4\xb4\xcf\xb4\xd9.");
			break;
		case 12:
			text =
				K2L_Compatibility("\xbb\xf3\xb4\xeb \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xbf\xcd \xb8\xc5\xc4\xa1 \xc7\xc3\xb7\xb9\xc0\xcc\xb8\xa6 \xb4\xf5 \xc0\xcc\xbb\xf3 \xc7\xd2\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 13:
			text =
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xbd\xc3\xc0\xdb\xc7\xcf\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd1 \xc0\xaf\xc0\xfa\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		case 14:
		{
			// 983 (00ac33bb): o motivo 0xe traz o numero do erro do servidor
			unsigned long error = packet.Decode4();

			switch (error)
			{
			case 3800056:
			case 3800057:
			case 3800059:
			case 3800084:
			case 3800085:
				text =
					K2L_Compatibility("\xc4\xa8\xc0\xce \xbf\xac\xbd\xc0 \xc6\xbc\xc4\xcf\xc0\xcc \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9.");
				break;
			case 3800060:
				text =
					K2L_Compatibility("\xbd\xc3\xbd\xba\xc5\xdb\xc0\xcc \xbb\xfd\xbc\xba\xc7\xd1 \xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9. \xbb\xe7\xbf\xeb\xc0\xda\xb0\xa1 \xbd\xc3\xc0\xdb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;
			default:
				text = MakeStr(
					K2L_Compatibility("\xbf\xb9\xbb\xf3\xc4\xa1 \xb8\xf8\xc7\xd1 \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9(\xc4\xda\xb5\xe5:%d)"),
					error);
				break;
			}
			break;
		}
		case 15:
			text =
				K2L_Compatibility("\xbf\xac\xbd\xc0\xb8\xf0\xb5\xe5\xbf\xa1 \xb5\xce\xb8\xed\xc0\xcc\xbb\xf3 \xc0\xd4\xc0\xe5\xc7\xcf\xbf\xa9 \xb0\xd4\xc0\xd3\xc0\xbb \xbd\xc3\xc0\xdb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
			break;
		}

		std::list<sSlotInfo>::iterator it = std::find(Doc()->m_slotList.begin(),
			Doc()->m_slotList.end(), Doc()->m_myInfo.info.dwGuid);

		if (it != Doc()->m_slotList.end())
		{
			if (it->bMaster != 0)
			{
				GetActor("Lobby")
					<< MsgObject(NULL, M983(0x23), (int)text.c_str(), 0, 0, 0, 0);
				GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
			}
			else if (Doc()->m_roomInfo.gameType == 6)
			{
				GetActor("Lobby")
					<< MsgObject(NULL, M983(0x23), (int)text.c_str(), 0, 0, 0, 0);
			}
			else
			{
				GetActor("Lobby") << MsgObject(NULL, M983(0x2b), 0, 0, 0, 0, 0);
			}
		}

		break;
	}

	case 0x81:
	{
		std::string name;

		packet.DecodeStr(name);
		name +=
			K2L_Compatibility("\xb4\xd4\xc0\xbb \xb0\xad\xc1\xa6 \xc5\xf0\xc0\xe5\xbd\xc3\xc5\xb0\xb7\xc1 \xc7\xd5\xb4\xcf\xb4\xd9.\n\xc2\xf9\xbc\xba\xc7\xcf\xbd\xca\xb4\xcf\xb1\xee?");

		GetActor("Lobby") << MsgObject(NULL, M983(0x31), (int)name.c_str(),
			packet.Decode4(), 0, 0, 0);
		break;
	}

	case 0x37:

	case 0x83:
	{
		if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
		{
			if (packet.Decode2() != 0)
				break;

			unsigned long uid = packet.Decode4();
			unsigned char dmType = packet.Decode1();
			unsigned short dmRoom = packet.Decode2();
			unsigned long arg = packet.Decode4();

			std::string name = packet.DecodeStr();

			unsigned long sendValue = packet.Decode4();

			Doc()->SetDirectMoveRoomInfo(dmType, dmRoom, uid);

			WSendPacket sendPacket(0x24);
			sendPacket.Encode4(sendValue);
			sendPacket.Send((eSendTo)1);

			GetActor("Lobby")
				<< MsgObject(NULL, M983(0x30), uid, dmType, dmRoom, arg, (int)&name);
			break;
		}

		unsigned short s = packet.Decode2();
		unsigned long l = packet.Decode4();

		GetActor("Lobby") << MsgObject(NULL, M983(0x30), s, l, 0, 0, 0);
		break;
	}

	case 0x86:
	{

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		// 983 (00ac3733): a contagem passou a Decode4; -1 = a sala nao existe mais
		int count = packet.Decode4();

		if (count == -1)
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xb4\xc2 \xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			break;
		}

		sRoomDetail detail;

		packet.DecodeBuffer(&detail, sizeof(sRoomDetail));
		Doc()->m_roomDetail = detail;

		std::list<sRoomUserInfo>& users = Doc()->m_roomUserList;
		users.clear();

		// D747: o servidor manda o 0x86 no formato do 645 (0x84: contagem Decode1; 31 B = 1 + 12 + 18); o 983 le Decode4 ->
		// contagem 0x601 = 1537 registros de 0x12 B copiados (memcpy sem limite) ~27 KB alem do pacote -> queda intermitente
		// no direito numa sala da lista. Com o pacote do 983 o laco e o mesmo; so nao le alem do fim do pacote.
		for (int i = count; i > 0 && packet.GetRemainLength() >= (int)sizeof(sRoomUserInfo); i--)
		{
			sRoomUserInfo user;
			memset(&user, 0, sizeof(sRoomUserInfo));

			packet.DecodeBuffer(&user, sizeof(sRoomUserInfo));
			users.push_back(user);
		}

		// 983 (00ac37df): o argumento vem de CSharedDoc+0x11630 (leitor 0062bf50)
		if (Doc()->m_roomListKind983 != 0)
			GetActor("Lobby") << MsgObject(NULL, M983(0x33), 1, 0, 0, 0, 0);
		else
			GetActor("Lobby") << MsgObject(NULL, M983(0x33), 0, 0, 0, 0, 0);
		break;
	}

	case 0x88:
	{
		unsigned long count = packet.Decode4();

		if (count == 0)
			break;

		do
		{
			unsigned long guid = packet.Decode4();
			unsigned short roomIndex = packet.Decode2();
			std::map<unsigned long, sBriefUserInfo>::iterator it =
				Doc()->m_briefUserInfoMap.find(guid);

			if (it != Doc()->m_briefUserInfoMap.end())
				it->second.roomIndex = roomIndex;

			count--;
		} while (count != 0);

		break;
	}

	case 0x8c:
	{
		unsigned char finishOrder = (unsigned char)Doc()->m_rivalList.size();

		for (std::vector<sRivalData>::iterator rival =
				 Doc()->m_rivalList.begin();
			rival != Doc()->m_rivalList.end(); ++rival)
		{
			if (rival->state == 0)
			{
				unsigned char hole = rival->hole;

				rival->finishOrder = finishOrder;
				rival->state = 2;
				finishOrder--;
				rival->holePang[hole - 1] = 0;
				rival->holeStroke[rival->hole - 1] =
					Doc()->m_holePar[rival->hole - 1] + 5;
				rival->holeScore[rival->hole - 1] = 5;
				rival->totalScore += 5;

				while (rival->hole != GetEndHole())
				{
					rival->hole = Doc()->m_holeOrder[GetHoleIndex(rival->hole)];
					rival->holePang[rival->hole - 1] = 0;
					rival->holeStroke[rival->hole - 1] =
						Doc()->m_holePar[rival->hole - 1] + 5;
					rival->holeScore[rival->hole - 1] = 5;
					rival->totalScore += 5;
				}
			}
		}

		GetActor("Lobby") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		break;
	}

	case 0x8e:
	{
		unsigned long arg = packet.Decode4();
		unsigned char flag = packet.Decode1();

		GetActor("Lobby") << MsgObject(NULL, M983(0x3d), arg, flag, 0, 0, 0);
		break;
	}

	case 0x96:
		// 983 00ac3a35: saldo (+0x3870), cookie, RFLobbyDesktop::Refresh983 (005fbe40, dinheiro do lobby) e msg 2 ao "Lobby".
		// O 645 ainda trocava para a CShopTask se m_shopName estivesse preenchido (nunca e; o 983 nao tem esse ramo).
		Doc()->m_bonusCash = packet.Decode8();
		Doc()->RefreshAllCookie();

		if (RFLobbyDesktop::Instance983())
			RFLobbyDesktop::Instance983()->Refresh983();

		GetActor("Lobby") << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);
		break;

	case 0x98:
	{
		if (!GalleryMode())
			break;

		packet.DecodeBuffer(&Doc()->m_userInfo[0], sizeof(sUserInfo));	// 983: 0x31c6 (era 0xb92 do 645)
		Doc()->m_myInfo.info.dwGalleryGuid = Doc()->m_userInfo[0].info.dwGuid;

		if (!IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
			break;

		RequestUccInfo(Doc()->m_userInfo[0]);
		break;
	}

	case 0x97:
		Doc()->m_roomType = packet.Decode4();
		break;

	case 0x1f6:
		// 983 (00ac5208): o lobby aceita o pacote e nao le nada
		break;

	case 0x24b:
	{
		// 983 (00ac51a5): numero de erro do servidor para todas as tarefas (mensagem 0x27, nova no 983)
		unsigned long error = packet.Decode4();

		if (error != 0)
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x27 /* 983; sem par no 645 */, error, 0, 0, 0, 0));
		break;
	}

	case 0xbc:
	{
		unsigned char type = packet.Decode1();

		if (type == 0xb)
		{
			GetActor("Lobby") << MsgObject(NULL, M983(0x3),
				(int)K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xb5\xd1\xb7\xaf\xba\xb8\xb1\xe2\xb0\xa1 \xb0\xa1\xb4\xc9\xc7\xd1 \xb0\xd4\xc0\xd3\xc1\xdf\xc0\xce \xb9\xe6\xc0\xbb \xc3\xa3\xc1\xf6 \xb8\xf8\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xffff7878, 0, 0, 0);
			break;
		}

		if (type == 0x10)
		{
			CChatMsg::Instance()->AddChatMsg(
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xb7\xce\xb5\xf9\xb5\xc7\xb1\xe2 \xc0\xfc\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xcc \xc1\xbe\xb7\xe1\xb5\xc9 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xffff7878, false, false);
			break;
		}

		packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));	// 983: 0xdd
		Doc()->m_expRate = packet.Decode4();

		unsigned char count = packet.Decode1();

		Doc()->m_slotList.clear();

		for (unsigned char i = count; i != 0; i--)
		{
			sSlotInfo slot;

			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));	// 983: 0x15c
			Doc()->m_slotList.push_back(slot);
		}

		if (IsMassGame(Doc()->m_roomInfo.gameType))
		{
			packet.DecodeBuffer(&Doc()->m_userInfo[0], sizeof(sUserInfo));	// 983: 0x31c6

			if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
				RequestUccInfo(Doc()->m_userInfo[0]);

			Doc()->m_myInfo.info.dwGalleryGuid =
				Doc()->m_userInfo[0].info.dwGuid;
		}
		else
		{
			if (count > 3)
				break;

			for (int i = 0; i < count; i++)
			{
				packet.DecodeBuffer(&Doc()->m_userInfo[i], sizeof(sUserInfo));	// 983: 0x31c6

				if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
					RequestUccInfo(Doc()->m_userInfo[i]);
			}
		}

		SetCurMap(packet.Decode1());
		SetGameType(packet.Decode1());
		unsigned char holeType = packet.Decode1();
		Doc()->m_holeType = holeType;
		unsigned char holes = packet.Decode1();
		Doc()->m_golfGame.holes = holes;
		Doc()->m_roomType = packet.Decode4();
		unsigned long shotTimeLimit = packet.Decode4();
		Doc()->m_golfGame.shotTimeLimit = shotTimeLimit;
		unsigned long gameTimeLimit = packet.Decode4();
		Doc()->m_golfGame.gameTimeLimit = gameTimeLimit;

		for (int i = 0; i < Doc()->m_golfGame.holes; i++)
		{
			Doc()->m_holeRandom[i] = packet.Decode4();
			Doc()->m_courseMap[i] = packet.Decode1();	// 983: vem no pacote (00ac3d93); era m_holeRandom % 3
			Doc()->m_holeOrder[i] = packet.Decode1();
		}

		Doc()->m_startHole = packet.Decode1();
		Doc()->m_roomSetupStage = 0;

		GetActor("Lobby") << MsgObject(NULL, M983(0x18), 0, 0, 0, 0, 0);
		break;
	}

	case 0xbf:
	{
		unsigned char count = packet.Decode1();

		for (unsigned char i = 0; i < count; i++)
		{
			sGuildMatchup matchup;

			packet.DecodeBuffer(&matchup, 9);

			if (Doc()->m_myInfo.info.dwGuid == matchup.uid[0])
			{
				Doc()->m_teamPlayerGuid[0] = matchup.uid[0];
				Doc()->m_teamPlayerGuid[1] = matchup.uid[1];
				Doc()->m_guildTeam = matchup.team;
			}
			else if (Doc()->m_myInfo.info.dwGuid == matchup.uid[1])
			{
				Doc()->m_teamPlayerGuid[0] = matchup.uid[1];
				Doc()->m_teamPlayerGuid[1] = matchup.uid[0];
				Doc()->m_guildTeam = matchup.team;
			}

			Doc()->m_guildMatchupList.push_back(matchup);
		}

		break;
	}

	case 0xc0:
	{
		packet.DecodeBuffer(Doc()->m_guildRefreshData, 0x1c);
		CUserInfo::Instance()->Refresh();
		break;
	}

	case 0xc1:
	{
		unsigned long guildId = packet.Decode4();
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(CUserInfo::Instance()->GetGuid());

		if (it != Doc()->m_briefUserInfoMap.end())
			break;

		if (guildId != it->second.m_GuildId)
			break;

		unsigned short count = packet.Decode2();

		if (count > 10)
			count = 10;

		for (int i = count; i != 0; i--)
		{
			sGuildMatchFlagInfo info;

			packet.DecodeBuffer(&info, sizeof(sGuildMatchFlagInfo));	// 983: 0x32 (era 0x2e do 645)
			Doc()->m_guildMatchFlagList.push_back(info);
		}

		CUserInfo::Instance()->Refresh();
		break;
	}

	case 0xc2:
	{
		unsigned long uid = packet.Decode4();

		Doc()->m_guildScore[0] = packet.Decode2();
		Doc()->m_guildScore[1] = packet.Decode2();

		unsigned char scoreA = packet.Decode1();
		unsigned char scoreB = packet.Decode1();
		sRivalData* rivalA = &Doc()->m_rivalList[Doc()->GetIndex(uid)];
		sRivalData* rivalB = Doc()->GetRival(uid);

		if (rivalA == NULL)
			break;

		if (rivalB == NULL)
			break;

		GetActor("Lobby") << MsgObject(NULL, M983(0x57), Doc()->m_guildScore[0],
			Doc()->m_guildScore[1], 0, 0, 0);
		rivalA->guildPoint = scoreA;
		rivalB->guildPoint = scoreB;
		break;
	}

	case 0xc3:
	{
		GetActor("Lobby") << MsgObject(NULL, M983(0x2b), 1, 0, 0, 0, 0);
		GetActor("Lobby") << MsgObject(NULL, M983(0x58), 0, 0, 0, 0, 0);
		break;
	}

	case 0xcf:
	{
		unsigned short count = packet.Decode2();

		Doc()->m_questList.clear();

		for (int i = 0; i < count; i++)
		{
			sQuest quest;

			quest.flag = 0;
			quest.typeId = packet.Decode4();
			quest.count = packet.Decode1();
			quest.pQuest = Doc()->m_itemManager.FindQuest(quest.typeId);
			Doc()->m_questList.push_back(quest);
		}

		break;
	}

	case 0x11b:
	{
		if (!IsLocalContent((localContentType_t)0x14))
			break;

		unsigned long uid = packet.Decode4();

		if (!Doc()->AreWePlayingTogether(uid))
			break;

		Doc()->m_rivalList[Doc()->GetIndex(uid)].state = 2;

		GetActor("Lobby") << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		break;
	}

	case 0x12f:
	{
		if (!IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
			break;

		GetActor("Lobby") << MsgObject(NULL, M983(0x230), 0, 0, 0, 0, 0);

		unsigned short result = packet.Decode2();

		if (result == 0)
		{
			unsigned long guid = packet.Decode4();
			unsigned char type = packet.Decode1();
			unsigned short room = packet.Decode2();
			unsigned long uid = packet.Decode4();
			std::string name = packet.DecodeStr();

			unsigned long svrUID = packet.Decode4();

			if (Doc()->m_gameServerUID == Doc()->GetFriendGameSvrUID())
			{
				WSendPacket sendPacket((enumClientPacket)0x29);

				sendPacket.Encode4(svrUID);
				sendPacket.Send((eSendTo)0);
			}
			else if (WNetworkSystem::Instance()->IsConnected(
						 (WNetworkSystem::eNetUnit)1) &&
				uid == Doc()->m_myInfo.info.dwUID &&
				WNetworkSystem::Instance()->IsConnected(
					(WNetworkSystem::eNetUnit)1))
			{
				WSendPacket sendPacket(0x28); // 983 00ac4338 (645: 0x26)

				sendPacket.Encode4(guid);
				sendPacket.Encode1(type);
				sendPacket.Encode2(room);
				sendPacket.Encode4(uid);
				sendPacket.EncodeStr(name);
				sendPacket.Encode4(svrUID);
				sendPacket.Send((eSendTo)1);
			}

			break;
		}

		// 983 00ac43dc: codigos do 645 + 1 (0x15..0x1a) e 0x13 novo
		if (result == 6)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xd4\xc0\xd3\xb9\xe6\xbf\xa1\xc0\xd4\xc0\xe5\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x15)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc0\xaf\xc0\xfa\xb4\xc2 \xc3\xca\xb4\xeb\xc7\xd2\xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x16)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc0\xaf\xc0\xfa\xb4\xc2 \xb1\xe6\xb5\xe5 \xb0\xa1\xc0\xd4\xc0\xbb \xc7\xcf\xc1\xf6 \xbe\xca\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x17)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xe7\xc5\xb0\xb7\xb9\xba\xa7\xb8\xb8 \xc0\xd4\xc0\xe5\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x18)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc7\xd8\xb4\xe7 \xc0\xaf\xc0\xfa\xb4\xc2 \xc3\xca\xb4\xeb\xc7\xd2\xbc\xf6 \xc0\xd6\xb4\xc2 \xc0\xa7\xc4\xa1\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 2)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb9\xe6\xbf\xa1 \xc0\xce\xbf\xf8\xc0\xcc \xb2\xcb\xc3\xa1\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x19)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xa4\xb3\xce\xc0\xc7 \xc1\xa4\xbf\xf8\xbc\xf6\xb0\xa1 \xc3\xca\xb0\xfa\xb5\xc7\xbe\xee \xc3\xca\xb4\xeb\xc7\xd2\xbc\xf6\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x1a)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xca\xb4\xeb \xb4\xeb\xbb\xf3\xc0\xba \xc3\xa4\xb3\xce\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xd2\xbc\xf6 \xbe\xf8\xb4\xc2 \xb7\xb9\xba\xa7\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else if (result == 0x13)
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb7\xe7\xc5\xb0" "F \xc0\xaf\xc0\xfa\xb4\xc2 \xc3\xca\xb4\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
		else
			GetActor("Lobby") << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc3\xca\xb4\xeb\xc7\xcf\xb1\xe2\xb8\xa6 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);

		Doc()->m_inviteMode = 0;
		break;
	}

	case 0x130:
	{
		if (!IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
			break;

		std::list<sSlotInfo>& slots = Doc()->m_slotList;
		unsigned long uid = packet.Decode4();

		Doc()->m_inviteMode = 0;

		std::list<sSlotInfo>::iterator it;

		for (it = slots.begin(); it != slots.end(); ++it)
		{
			if (uid == it->dwUserUID)
			{
				sSlotInfo* pSlot = &*it;
				unsigned long guid = it->dwGuid;
				unsigned char team = it->bTeam;
				unsigned char rank = it->connectionRank;

				slots.erase(it);

				for (it = slots.begin(); it != slots.end(); ++it)
				{
					if (rank < it->connectionRank)
						it->connectionRank--;
				}

				GetActor("Lobby")
					<< MsgObject(NULL, M983(0x8), (int)pSlot, guid, team, 0, 0);

				Doc()->m_roomInfo.nUserNum = slots.size();

				GetActor("Lobby") << MsgObject(NULL, M983(0x2f), 0, 0, 0, 0, 0);
				GetActor("Lobby") << MsgObject(NULL, M983(0xf), 0, 0, 0, 0, 0);
				GetActor("Lobby") << MsgObject(NULL, M983(0x10), 0, 0, 0, 0, 0);
				break;
			}
		}
		break;
	}

	case 0x13a:
	{
		if (!IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
			break;

		unsigned long uid = 0;

		uid = packet.Decode4();
		std::string name = packet.DecodeStr();

		unsigned char flag = packet.Decode1();

		GetActor("Lobby") << MsgObject(NULL, M983(0x21c), (int)&name, (int)&flag,
			(int)&uid, 0, 0);
		break;
	}

	case 0x13c:
	{
		if (!IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
			break;

		packet.Decode1();
		packet.Decode1();
		break;
	}

	case 0x13b:
	{
		if (!IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
			break;

		// PKT (frente C, lote 11): sub-codigos na numeracao do 983 (00ac4718; textos 00d49a48, 00d49a28, 00d499d0, 00d49988,
		// 00d49954): 1/2/3 e 5/6 trocados em relacao ao 645
		switch (packet.Decode1())
		{
		case 0:
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 1, 0, 0, 0, 0);
			break;
		case 2: // PKT: era 1 (645)
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xb8\xc5\xc4\xaa \xb4\xeb\xbb\xf3\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.\n\xc0\xe1\xbd\xc3\xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		case 3: // PKT: era 2 (645)
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xb4\xeb\xbb\xf3\xc0\xcc \xb8\xc5\xc4\xaa\xc0\xbb \xb0\xc5\xc0\xfd\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		case 1: // PKT: era 3 (645)
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xb8\xf0\xb5\xe7 \xb4\xeb\xbb\xf3\xc0\xcc \xb8\xc5\xc4\xaa\xc0\xbb \xb0\xc5\xc0\xfd\xc7\xcf\xbf\xa9 \xb8\xc5\xc4\xaa\xc0\xbb \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.\n \xc0\xe1\xbd\xc3\xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		case 4:
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xc8\xaf\xbb\xf3\xc0\xc7 \xc6\xce\xbe\xdf\xbc\xb6\xbf\xa1\xbc\xad\xb4\xc2 \xb8\xc5\xc4\xaa\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		case 6: // PKT: era 5 (645)
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xb8\xc5\xc4\xaa\xc0\xbb \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		case 5: // PKT: era 6 (645)
			GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
				(int)MakeStr(
					K2L_Compatibility("\xb8\xc5\xc4\xaa\xc0\xbb \xbf\xe4\xc3\xbb\xc7\xd1 \xbb\xf3\xb4\xeb\xb9\xe6\xc0\xcc \xb8\xc5\xc4\xaa \xb0\xa1\xb4\xc9\xc7\xd1 \xc0\xa7\xc4\xa1\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
				0, 0, 0);
			break;
		default:
			break;
		}

		break;
	}

	case 0x13d:
	{
		if (!IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
			break;

		unsigned long uid = packet.Decode4();
		unsigned char type = packet.Decode1();
		unsigned short room = packet.Decode2();

		if (uid == Doc()->m_gameServerUID)
		{
			WSendPacket sendPacket((enumClientPacket)0xb4);

			sendPacket.Encode1(type);
			sendPacket.Encode2(room);
			sendPacket.Send((eSendTo)0);
		}
		else
		{
			SendMsgToMainActor(MsgObject(NULL, M983(0xdf), 0, 0, 0, 0, 0)); // 983 00ac490f: sem teste; o form do My Room recebe pelo Lobby

			Doc()->m_gameServerList.clear();
			Doc()->m_channelList.clear();

			WSendPacket sendPacket((enumClientPacket)0x43);

			sendPacket.Send((eSendTo)0);
			Doc()->m_goWithMode = 3;
			Doc()->SetDirectMoveRoomInfo(type, room, uid);
		}

		break;
	}

	case 0x13e:
	{
		if (!IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
			break;

		GetActor("Lobby") << MsgObject(NULL, M983(0x21d), 0,
			(int)MakeStr(
				K2L_Compatibility("\xb8\xc5\xc4\xaa\xc0\xcc \xc3\xeb\xbc\xd2\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3\xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.")),
			0, 0, 0);
		break;
	}

	case 0x13f:
	{
		CMatchingSystem* matching = CMatchingSystem::Instance();
		if (matching == NULL)
			break;

		unsigned char count = packet.Decode1();

		matching->Clear();

		for (unsigned char i = 0; i < count; i++)
		{
			unsigned char intrest = packet.Decode1();

			matching->SetMyIntrest(intrest);
		}
		break;
	}

	case 0x17b:
	{
		if (!IsLocalContent((localContentType_t)0x71 /*983: era 0x72; medido*/))
			break;

		if (packet.Decode2() != 1)
			break;

		GetActor("Lobby") << MsgObject(NULL, M983(0x240), 0, 0, 0, 0, 0);
		break;
	}

	case 0x17d:
	{
		unsigned long flagBlock = packet.Decode4();
		Doc()->m_myInfo.info.dwFlagBlock = flagBlock;
		break;
	}

	case 0x187:
	{
		if (!IsLocalContent((localContentType_t)0x77 /*983: era 0x7a; medido 00ac03a0*/))
			break;

		if (!TicketInfo::IsInstantiated())
			break;

		unsigned char state = packet.Decode1();
		unsigned long flags = packet.Decode4();

		if (flags == 0x1000000)
		{
			TicketInfo::Instance()->m_ticketCount = packet.Decode4();
		}
		else if (flags == 0x10000)
		{
			unsigned long count = packet.Decode4();
			unsigned long tid = packet.Decode4();
			unsigned long itemCount = packet.Decode4();
			unsigned long itemRemain = packet.Decode4();

			TicketInfo::Instance()->m_ticketCount = count;

			std::map<unsigned long, TicketInfo::sTicketItem>::iterator it =
				TicketInfo::Instance()->m_itemMap.find(tid);
			if (it != TicketInfo::Instance()->m_itemMap.end())
			{
				it->second.count = itemCount;
				it->second.remain = itemRemain;
			}
		}
		else if (flags == 0x100000)
		{
			unsigned long count = packet.Decode4();
			unsigned long tid = packet.Decode4();
			unsigned long itemRemain = packet.Decode4();

			TicketInfo::Instance()->m_ticketCount = count;

			std::map<unsigned long, TicketInfo::sTicketItem>::iterator it =
				TicketInfo::Instance()->m_itemMap.find(tid);
			if (it != TicketInfo::Instance()->m_itemMap.end())
			{
				it->second.remain = itemRemain;
			}
		}
		else if (flags == 0x11110000)
		{
			TicketInfo::Instance()->m_ticketCount = packet.Decode4();
			TicketInfo::Instance()->m_state = packet.Decode1();

			unsigned int n =
				(unsigned int)TicketInfo::Instance()->m_itemMap.size();
			while (n != 0)
			{
				unsigned long tid = packet.Decode4();
				unsigned long itemCount = packet.Decode4();
				unsigned long itemRemain = packet.Decode4();

				std::map<unsigned long, TicketInfo::sTicketItem>::iterator it =
					TicketInfo::Instance()->m_itemMap.find(tid);
				if (it != TicketInfo::Instance()->m_itemMap.end())
				{
					it->second.count = itemCount;
					it->second.remain = itemRemain;
				}
				n--;
			}
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x6d), flags, state, 0, 0, 0));
		break;
	}

	case 0x189:
	{
		if (!IsLocalContent((localContentType_t)0x80 /*983: era 0x83; medido*/))
			break;

		CMissionEvent* missionEvent;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x80,
			missionEvent);
		if (missionEvent == NULL)
			break;

		missionEvent->DecodeMissionInfo(packet);

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x6f), 0, 0, 0, 0, 0));
		break;
	}

	case 0x18a:
	{
		if (!IsLocalContent((localContentType_t)0x80 /*983: era 0x83; medido*/))
			break;

		unsigned long result = packet.Decode4();

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));
		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x6f), 1, result, 0, 0, 0));
		break;
	}

	case 0x18b:
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		if (!IsLocalContent((localContentType_t)0x82 /*983: era 0x85; medido*/))
			break;

		unsigned long result = packet.Decode4();
		if (result != 0)
		{
			char msg[0x100] = { 0 };

			switch (result)
			{
			case 10:
				wsprintf(msg,
					"\xc3\xdf\xc3\xb7\xb1\xc7\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
				break;

			case 9:
				wsprintf(msg,
					"\xc0\xdf\xb8\xf8\xb5\xc8 \xc0\xaf\xc0\xfa\xc1\xa4\xba\xb8\xc0\xd4\xb4\xcf\xb4\xd9. \xc0\xe7\xc1\xa2\xbc\xd3 \xc7\xd8\xc1\xd6\xbc\xbc\xbf\xe4 : %d",
					result);
				break;

			case 1:
				wsprintf(msg,
					"\xc0\xdf\xb8\xf8\xb5\xc8 \xbc\xfd\xc0\xda\xb0\xa1 \xc3\xdf\xc3\xb7\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \xbf\xa1\xb7\xaf\xc4\xda\xb5\xe5 : %d",
					result);
				break;
			}

			AfxGetTask()->SendMsgToMainActor(
				MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0));
		}
		else
		{
			unsigned long guid[4];
			packet.DecodeBuffer(guid, 0x10);

			unsigned long value = 0;
			if (guid[0] == 1)
				value = packet.Decode4();

			int count = packet.Decode4();
			unsigned long first = 0;
			if (count > 0)
				first = packet.Decode4();

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x70), guid[0],
				(int)guid, value, count, (int)&first));
		}
		break;
	}

	case 0x18c:
	{
		if (!IsLocalContent((localContentType_t)0x82 /*983: era 0x85; medido*/))
			break;

		unsigned long value = packet.Decode4();
		unsigned long count = packet.Decode4();
		if (count == 0)
			break;

		std::vector<unsigned long> list;
		for (int i = 0; i < (int)count; i++)
		{
			list.push_back(packet.Decode4());
		}

		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x70), 2, (int)&list, (int)&value, 0, 0));
		break;
	}

	case 0x198:
	{
		unsigned long tid = packet.Decode4();
		unsigned long count = packet.Decode4();
		if (tid == 0)
			break;

		char msg[0x100] = { 0 };
		sprintf(msg,
			K2L_Compatibility("\xb9\xe6\xc0\xe5\xbf\xa1 \xb4\xeb\xc7\xd1 \xba\xb8\xbb\xf3\xc0\xb8\xb7\xce \\c0xffff0000\\c%s(%d)\\c0xff000000\\c\xc0\xcc \xc1\xf6\xb1\xde \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			Doc()->m_itemManager.GetItemName(tid), count);

		AfxGetTask()->SendMsgToMainActor(
			MsgObject(NULL, M983(0x26), (int)msg, tid, 0, 0, 0));
		break;
	}

	case PKT645_SEM_983(0x1b6):
	{
		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		GUILD_INFO info;

		packet.DecodeBuffer(&info, 0x129);
		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 4, (int)&info, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1b7):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		GetActor(s_guildActor[1])
			<< MsgObject(NULL, M983(0x263), 0x1a, 0x403, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1b8):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		GetActor(s_guildActor[1])
			<< MsgObject(NULL, M983(0x263), 0x1a, 0x402, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1bc):
	{
		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		std::list<GUILD_HISTORY> historyList;
		int count = packet.Decode2();

		for (int i = 0; i < count; i++)
		{
			GUILD_HISTORY history;

			packet.DecodeBuffer(&history, 0x31);
			historyList.push_back(history);
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 9, (int)&historyList, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1bd):
	{
		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		GUILD_USER_INFO userInfo;

		packet.DecodeBuffer(&userInfo, 0x119);
		GetActor(s_guildActor[1])
			<< MsgObject(NULL, M983(0x263), 5, (int)&userInfo, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c4):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		unsigned long param1 = packet.Decode4();
		unsigned long param2 = packet.Decode4();
		int count = packet.Decode2();
		std::list<GUILD_USER_LIST> userList;

		for (int i = 0; i < count; i++)
		{
			GUILD_USER_LIST user;

			packet.DecodeBuffer(&user, 0x51);
			userList.push_back(user);
		}

		if (count != 0)
		{
			IActor* guild = GetActor(s_guildActor[1]);
			guild << MsgObject(NULL, M983(0x263), 7, (int)&userList, param1, param2,
				0);
		}

		break;
	}

	case PKT645_SEM_983(0x1bf):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0xa, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c3):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0x12, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1ba):

	case PKT645_SEM_983(0x1bb):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		unsigned long param1 = packet.Decode4();
		unsigned long param2 = packet.Decode4();
		int count = packet.Decode2();
		std::list<GUILD_LIST> guildList;

		for (int i = 0; i < count; i++)
		{
			GUILD_LIST guild;

			packet.DecodeBuffer(&guild, 0xc4);
			guildList.push_back(guild);
		}

		GetActor(s_guildActor[1])
			<< MsgObject(NULL, M983(0x263), 8, (int)&guildList, param1, param2, 0);
		break;
	}

	case PKT645_SEM_983(0x1c6):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0xd, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1be):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x11, result, 0, 0, 0);
			break;
		}

		GetActor(s_guildActor[1]) << MsgObject(NULL, M983(0x263), 0xe, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c0):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		GetActor(s_guildActor[1]) << MsgObject(NULL, M983(0x263), 0xf, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c1):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		GetActor(s_guildActor[1]) << MsgObject(NULL, M983(0x263), 0x10, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c2):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		unsigned long param = packet.Decode4();

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0x13, param, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1c5):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 26, result, 0, 0, 0);
			break;
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0xc, 0, 0, 0, 0);
		break;
	}

	case PKT645_SEM_983(0x1b9):
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0));

		unsigned long result = packet.Decode4();

		if (result != 1)
		{
			GetActor(s_guildActor[1])
				<< MsgObject(NULL, M983(0x263), 0x1a, result, 0, 0, 0);
			break;
		}

		IActor* guild = GetActor(s_guildActor[1]);
		guild << MsgObject(NULL, M983(0x263), 0xb, 0, 0, 0, 0);
		break;
	}

	default:
		return OnPacketCommon(packet);
	}

	return 1;
}
// ---------------------------------------------------------------------------------------------------------------
// PKT (frente C): CLSLoginAckPostProcessor - resposta do servidor de login (pacote 0x1), JP 983 005a11f0..005a2920.
// No 983 estas funcoes ficam em outra unidade (vao 005a11f0, junto do lobbymain); estao aqui porque sao o corpo do
// antigo `case 1` do OnPacket. Declaracao em include/packets/lsloginackpostprocessor.h.
// ---------------------------------------------------------------------------------------------------------------
CLSLoginAckPostProcessor::CLSLoginAckPostProcessor(CTask* pTask)
	: m_pTask(pTask)
{
}

CLSLoginAckPostProcessor::~CLSLoginAckPostProcessor()
{
}

bool __stdcall CLSLoginAckPostProcessor::IsNicknameCode(unsigned char code)
{
	return code >= 0xd8 && code <= 0xd9;
}

bool __stdcall CLSLoginAckPostProcessor::IsSecondPwdCode(unsigned char code)
{
	return code >= 0xdc && code <= 0xde;
}

bool __stdcall CLSLoginAckPostProcessor::IsPlainCode(unsigned char code)
{
	if (code == 0xe2)
		return false;
	if (IsNicknameCode(code) == true)
		return false;
	return IsSecondPwdCode(code) != true;
}

// 983: bloqueio da conta. Os quatro campos sao novos no CSharedDoc do 983 (+0x1cc, +0x1ac, +0x1b0, +0x190);
// o +0x1cc e o mesmo campo em que o login grava o ProvType.
__forceinline void CLSLoginAckPostProcessor::ReadBlockInfo(WReceivedPacket& packet)
{
	Doc()->m_provType = packet.Decode4();
	Doc()->m_unused184 = packet.Decode4();
	Doc()->m_unusedString188 = packet.DecodeStr();
	packet.DecodeStr(Doc()->m_paymentKID983);
}

int CLSLoginAckPostProcessor::OnPlainCode(unsigned char code, WReceivedPacket& packet)
{
	// JP 983 005a1630: ordem dos casos (e dos corpos) como no 983
	switch (code)
	{
	case 0xd1:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x1b, 0, 0, 0, 0);
		break;
	case 0xd8:
		if (packet.Decode4() != 0xffffffff)
		{
			Doc()->m_forceNicknameChange = 2;
			m_pTask->GetActor("Lobby") << MsgObject(NULL, 0, (int)"BLANK", 0, 0, 0, 0);
		}
		else
		{
			Doc()->m_forceNicknameChange = 1;

			if (g_pFresh->IsCurrentLayout("LOGIN") == true)
				m_pTask->GetActor("Lobby") << MsgObject(NULL, 0, (int)"BLANK", 0, 0, 0, 0);
		}
		return 0;
	case 0xd9:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xd, 0, 0, 0, 0);
		break;
	case 2:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 6, 0, 0, 0, 0);
		break;
	case 6:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 7, 0, 0, 0, 0);
		break;
	case 4:
		ReadBlockInfo(packet);
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 8, 0, 0, 0, 0);
		break;
	case 0x12:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x16, 0, 0, 0, 0);
		break;
	case 5:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 9, 0, 0, 0, 0);
		break;
	case 0x13:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x17, 0, 0, 0, 0);
		break;
	case 7:
		Doc()->m_chatMode = packet.Decode4();
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xa, 0, 0, 0, 0);
		break;
	case 8:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xb, 0, 0, 0, 0);
		break;
	case 12:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x10, 0, 0, 0, 0);
		break;
	case 10:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xf, 0, 0, 0, 0);
		break;
	case 9:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xe, 0, 0, 0, 0);
		break;
	case 13:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x11, 0, 0, 0, 0);
		break;
	case 14:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x12, 0, 0, 0, 0);
		break;
	case 15:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x13, 0, 0, 0, 0);
		break;
	case 16:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x14, 0, 0, 0, 0);
		break;
	case 0xd4:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x18, 0, 0, 0, 0);
		break;
	case 0xcf:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x1a, 0, 0, 0, 0);
		break;
	default:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xc, 0, 0, 0, 0);
		break;
	case 200:
		return 0;
	}

	return 0;
}

int CLSLoginAckPostProcessor::OnNicknameCode(unsigned char code, WReceivedPacket& packet)
{
	switch (code)
	{
	case 0xd8:
		if (packet.Decode4() == 0xffffffff)
		{
			Doc()->m_forceNicknameChange = 1;

			if (g_pFresh->IsCurrentLayout("LOGIN") == true)
				m_pTask->GetActor("Lobby") << MsgObject(NULL, 0, (int)"BLANK", 0, 0, 0, 0);
		}
		else
		{
			Doc()->m_forceNicknameChange = 2;
			m_pTask->GetActor("Lobby") << MsgObject(NULL, 0, (int)"BLANK", 0, 0, 0, 0);
		}
		return 0;

	case 0xd9:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xd, 0, 0, 0, 0);
		return 0;
	}

	return 5100151;
}

// codigos 0xe2 e 0xdc..0xde: o servidor manda um numero de erro (5100013...) no lugar do sub-codigo
int CLSLoginAckPostProcessor::OnServerError(unsigned char code, WReceivedPacket& packet)
{
	unsigned long error = packet.Decode4();
	switch (error)
	{
	case 5100013:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x14, 0, 0, 0, 0);
		break;
	case 5100018:
	case 5100104:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x13, 0, 0, 0, 0);
		break;
	case 5100019:
	case 5100023:
		ReadBlockInfo(packet);
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 8, 0, 0, 0, 0);
		break;
	case 5100020:
	case 5100118:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 9, 0, 0, 0, 0);
		break;
	case 5100021:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x17, 0, 0, 0, 0);
		break;
	case 5100022:
		Doc()->m_chatMode = packet.Decode4();
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xa, 0, 0, 0, 0);
		break;
	case 5100106:
	case 5100108:
	case 5100114:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 6, 0, 0, 0, 0);
		break;
	case 5100109:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x18, 0, 0, 0, 0);
		break;
	case 5100115:
	case 5100123:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 7, 0, 0, 0, 0);
		break;
	case 5100116:
	case 5100119:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x11, 0, 0, 0, 0);
		break;
	case 5100117:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x16, 0, 0, 0, 0);
		break;
	case 5100120:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x10, 0, 0, 0, 0);
		break;
	default:
		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0x1c, error, 0, 0, 0);
		break;
	}

	return 0;
}

int CLSLoginAckPostProcessor::OnFail(unsigned char code, WReceivedPacket& packet)
{
	if (IsPlainCode(code) == true)
		return OnPlainCode(code, packet);

	if (IsNicknameCode(code) == true)
		return OnNicknameCode(code, packet);

	return OnServerError(code, packet);
}

int CLSLoginAckPostProcessor::Process(WReceivedPacket& packet)
{
	unsigned char code = packet.Decode1();

	if (code != 0)
		return OnFail(code, packet);

	std::string name;
	packet.DecodeStr(name);

	if (stricmp(name.c_str(), Doc()->m_myInfo.info.sID) != 0)
	{
		CProjectG::Instance()->ReportError(7, "ID_Server:%s,ID_Client:%s", name.c_str(), Doc()->m_myInfo.info.sID);

		m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 0xc, 0, 0, 0, 0);
		return 5100150;
	}

	Doc()->m_myInfo.info.dwUID = packet.Decode4();
	Doc()->m_myInfo.info.dwIdentity = packet.Decode4();
	strcpy(Doc()->m_myInfo.info.sID, name.c_str());
	Doc()->m_provType = CLoginInfo::Instance()->ProvType();
	Doc()->m_myInfo.stat.Level = packet.Decode1();
	Doc()->m_lobbyLoginTime = timeGetTime();	// 983: +0xb8 (005a27c3)
	Doc()->m_pcBang = packet.Decode1() == 1;

	ReadBlockInfo(packet);
	packet.Decode4();
	packet.Decode4();

	if (IsLocalContent((localContentType_t)0x7e))
	{
		std::string displayId;
		packet.DecodeStr(displayId);
		strcpy(Doc()->m_myInfo.info.sNick, displayId.c_str()); // 983 005a28a2: +0x7b8 = nick, nao sDisplayID (+0x84b)
	}

	m_pTask->GetActor("Lobby") << MsgObject(NULL, M983(0x1e), 4, 0, 0, 0, 0);
	return 0;
}
