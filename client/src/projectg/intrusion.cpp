#define REBANG_INTRUSION // HACK: exposes CGimmickContainer::LoadFromPacket
#include "minatl.h"
#include "intrusion.h"
#include "gatewayactor.h"
#include "treasure.h"
#include "fresh.h"
#include "frdesktop.h"
#include "frform.h"
#include "actor.h"
#include "taskmanager.h"
#include "messengerdlg.h"
#include "chatmsg.h"
#include "cardsystem.h"
#include "mousecursor.h"
#include "clientsetting.h"
#include "contentsdoc.h"
#include "user_info.h"
#include "../shared/localize.h"
#include "powergauge.h"

#pragma pack(push, 1)
struct sIntrusionHoleScore
{
	unsigned char holeStroke[18];
	int holeScore[18];
	__int64 holePang[18];
};

struct sIntrusionScore
{
	sIntrusionHoleScore hole;
	unsigned long uid;
	unsigned long sender;
};
#pragma pack(pop)

struct sFindRival
{
	unsigned long oid;

	sFindRival(unsigned long _oid)
		: oid(_oid)
	{
	}
	bool operator()(sRivalData& rival) { return rival.oid == oid; }
};

extern Fresh* g_pFresh;

static bool IsZero(float f)
{
	return f < g_EPSILON;
}

// HACK: shareddoc.inl's inline SetGameType would emit a GetGameTypeInfo COMDAT
// here that the linker keeps; call shareddoc.obj's out-of-line body instead.
#define RB_SET_GAME_TYPE SetGameType

class FrBongdariShopDlg : public FrForm
{
public:
	void Clear();
};

unsigned char CIntrusion::GetCalcExpMemberSize()
{
	return m_calcExpMemberSize;
}

void CIntrusion::FinishGame()
{
	m_calcExpMemberSize = 0;
}

CIntrusion::CIntrusion()
	: m_pDlg(NULL), m_roomGuid(0), m_calcExpMemberSize(0)
{
}

CIntrusion::~CIntrusion()
{
}

unsigned long CIntrusion::GetActiveColor()
{
	return 0xc3e5ffd8;
}

bool CIntrusion::OnAccessRoom(sRoomInfo& room)
{
	if (room.bIntrusion == true && room.bAvailable == false)
	{
		room.bAvailable = true;
		return true;
	}

	return false;
}

#pragma pack(push, 1)
struct sIntrusionEnterInfo
{
	unsigned short roomGuid;
	unsigned long gameType;
	unsigned long serverUID;
	unsigned char channel;
	char channelName[64];
};
#pragma pack(pop)

void CIntrusion::DoIntrusionAccess(WReceivedPacket& packet,
	eIntrusionPermitCode code)
{
	switch (code)
	{
	case INTRUSION_PERMIT_ENTER:
	{
		Doc()->m_myInfo.info.dwIdentity &= ~2;
		Doc()->m_myInfo.info.dwGalleryGuid = 0xffffffff;
		Doc()->m_slotList.clear();
		Doc()->m_rivalList.clear();
		Doc()->m_guildMatchupList.clear();
		Doc()->m_bGameOver = false;

		packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));

		Doc()->m_chatManager.FilteringHack(Doc()->m_roomInfo.title, false);
		Doc()->m_myInfo.roomIndex = Doc()->m_roomInfo.roomGuid;

		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_MSN) &&
			CMessengerInfo::Instance()->GetComplete() == 4)
		{
			sIntrusionEnterInfo info;
			memset(&info, 0, sizeof(info));

			info.serverUID = Doc()->m_gameServerUID;
			info.channel = Doc()->m_curChannel.Uid;
			info.roomGuid = Doc()->m_roomInfo.roomGuid;
			info.gameType = Doc()->m_roomInfo.gameType;
			wsprintf(info.channelName, "%s", Doc()->m_curChannel.Name);

			WSendPacket sendPacket(0x23);
			sendPacket.EncodeBuffer(&info, sizeof(info));
			sendPacket.Send(TO_MSN);
		}
		break;
	}

	case INTRUSION_PERMIT_PLAYERS:
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x2e), 0, 0, 0, 0, 0);

		Doc()->m_chatLineList.clear();

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;
		slotList.clear();

		int num = packet.Decode1();
		for (int i = 0; i < num; i++)
		{
			sSlotInfo slot;
			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));

			if (slot.bMaster)
				slot.bReady = 1;
			else
				slot.bReady = 0;

			slotList.push_back(slot);
		}
		break;
	}

	case INTRUSION_PERMIT_UNKNOWN2:
		Doc()->m_expRate = packet.Decode4();
		break;

	case INTRUSION_PERMIT_RIVALS:
	{
		Doc()->ClearRivalVars(false);

		{
			// 983 00856e..: tres Decode1 nos bits 5, 6 e 7 do byte de flags (+0x832 do doc);
			// no struct do 645 os bits 3..5 (angelicWings/Effect/devilWings) nao batem
			unsigned char* flags = reinterpret_cast<unsigned char*>(
				&Doc()->m_myInfo.info.dwEmblemVer + 1);
			*flags ^= (unsigned char)((packet.Decode1() << 5) ^ *flags) & 0x20;
			*flags ^= (unsigned char)((packet.Decode1() << 6) ^ *flags) & 0x40;
			*flags = (unsigned char)((*flags & 0x7f) | (packet.Decode1() << 7));
		}

		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			SYSTEMTIME serverTime;
			packet.DecodeBuffer(&serverTime, sizeof(serverTime));
			Doc()->SetServerTime(serverTime);

			CCardManager::Instance()->m_playerSlot[0] = 0;

			std::map<unsigned int, sCharacterInfo>::iterator it =
				Doc()->m_charMap.find(Doc()->m_myInfo.userEquip.guidChar);
			if (it != Doc()->m_charMap.end())
				CCardManager::Instance()->m_charInfo[0] = (*it).second;
		}

		Doc()->SetEquipCharCaddieInfoFromMyInfo();
		CPowerGauge::Instance()->UpdateClubArea983(); // 983: 00856ec2
		AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0, 0, 4, 0, 0); // 983: aviso ao Gateway
		break;
	}

	case INTRUSION_PERMIT_START:
	{
		FrWnd* pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"bongdari_shop");
		FrBongdariShopDlg* pShop = NULL;
		if (pWnd)
			pShop = (FrBongdariShopDlg*)pWnd;
		else
			__rtti_obj = NULL;
		if (pShop)
			pShop->Clear();

		if (CUserInfo::Instance())
			CUserInfo::Instance()->Clear();

		SetCurMap(packet.Decode1());
		RB_SET_GAME_TYPE(packet.Decode1());

		unsigned char holeType = packet.Decode1();
		Doc()->m_holeType = holeType;
		unsigned char holes = packet.Decode1();
		Doc()->m_golfGame.holes = holes;
		Doc()->m_roomType = packet.Decode4();
		unsigned long shotTimeLimit = packet.Decode4();
		Doc()->m_golfGame.shotTimeLimit = shotTimeLimit;
		unsigned long gameTimeLimit = packet.Decode4();
		Doc()->m_golfGame.gameTimeLimit = gameTimeLimit;

		int myIndex = 0;
		if (!IsMassGame())
		{
			for (myIndex = 0; myIndex < 4; myIndex++)
			{
				if (Doc()->m_userInfo[myIndex].info.dwGuid == MyGuid(false))
					break;
			}
		}

		for (int i = 0; i < 18; i++)
		{
			Doc()->m_holeRandom[i] = packet.Decode4();
			Doc()->m_courseMap[i] = packet.Decode1();	// 983 0085705e: o mapa do buraco vem no pacote (era % 3 no 645; lote 9 da C-PKT)
			Doc()->m_holeOrder[i] = packet.Decode1();
		}

		if (IsLocalContent((localContentType_t)0x52)) // 983 00856..
		{
			CCardManager::Instance()->SetPlayerIndex(0xff);
			CCardManager::Instance()->CalcCardPeriodAndStatus();
		}

		CGimmickContainer* pGimmick = NULL;
		CContentsDoc::Instance()->GetContainer((localContentType_t)0x9b,
			pGimmick);
		pGimmick->LoadFromPacket(packet);

		memset(Doc()->m_holePar, 0, 0x12);
		Doc()->m_roomSetupStage = 0;
		Doc()->LogGstUpdateHole(0xffffffff, 0xff, -1, -1);

		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
			Doc()->SetInitWearHalloweenItem();

		if (CMessengerInfo::Instance()->GetMyStatus() != 3)
			CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)0);

		Doc()->m_bGameOver = false;

		CMouseCursor::Instance()->SetMode(1);
		CMouseCursor::Instance()->SetActive(false);

		COption::Instance()->vSetMinimumScreenSize(0, 800, 600);

		CTaskManager::Instance()->ChangeTask("CGolfTask", "CGolfDoc", false);

		// 983: mesmas limpezas do CTask::OnPacket do taskmanager
		Doc()->m_appGiftDownloadState = (CSharedDoc::eAppGiftDownloadState)0; // +0x11424
		CTHunter::Instance()->SetReceiveGiftPacket(false); // [00e74e10]+0x5e
		Doc()->m_bTreasureResult = false; // +0x10777
		Doc()->m_bTreasureAward = false; // +0x10778

		IActor* pActor = AfxGetTask()->GetActor("Score");
		if (pActor)
		{
			pActor << MsgObject(NULL, M983(0x131), 0, 0, 0, 0, 0);
			pActor << MsgObject(NULL, M983(0x2a), 0, 0, 0, 0, 0);
		}
		break;
	}
	}
}

void CIntrusion::PacketAnalysis(WReceivedPacket& packet)
{
	if (m_pDlg != NULL)
		return;

	unsigned char type = packet.Decode1();
	unsigned char code = packet.Decode1();

	switch (type)
	{
	case 3:
	{
		m_roomGuid = packet.Decode2();
		unsigned long elapsed = packet.Decode4();
		unsigned long limit = packet.Decode4();

		packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));
		Doc()->m_chatManager.FilteringHack(Doc()->m_roomInfo.title, false);
		Doc()->m_myInfo.roomIndex = Doc()->m_roomInfo.roomGuid;

		m_pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"IntrusionAlarm", NULL);
		m_pDlg->SetCaption(
			K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4\xc0"
			"\xe5 \xc8\xae\xc0\xce"));
		m_pDlg->SetDesc(K2L_Compatibility(" \xa1\xd8 '\xbf\xb9'\xb8\xa6 \xb4\xa9\xb8\xa3\xb8\xe9 "
						"\xc1\xef\xbd\xc3 \xb0\xd4\xc0\xd3\xc0\xcc \xbd\xc3\xc0"
						"\xdb\xb5\xcb\xb4\xcf\xb4\xd9."));

		sInfoTime info;
		GetInfoTime(&info, elapsed, limit);

		// 983: 10+ tiki reports (tid 0x1a000042) in the item list adds a warning
		unsigned int reportCount = 0;
		for (std::list<sItemInfo>::const_iterator it = Doc()->GetConstMyItemList().begin();
			it != Doc()->GetConstMyItemList().end(); ++it)
		{
			sItemInfo item = *it; // 983 00856..: copia o item (0xc4 bytes, rep movsd)
			if (item.tid == 0x1a000042)
				reportCount++;
		}

		const char* msg;
		if (reportCount > 9)
			msg = MakeStr(K2L_Compatibility("\xc7\xd8\xb4\xe7 \xb0\xd4\xc0\xd3\xc0\xba \xb0\xd4\xc0\xd3 \xbd\xc3\xc0\xdb \xc8\xc4\n\\c0xffff0000\\c'%2d\xba\xd0%2d\xc3\xca'\\c0xff000000\\c\xb0\xa1 \xc1\xf6\xb3\xb5\xb0\xed \\c0xffff0000\\c'%2d\xba\xd0%2d\xc3\xca'\\c0xff000000\\c\xb0\xa1\n\xb3\xb2\xbe\xc6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n%s"),
				info.elapsedMin, info.elapsedSec, info.remainMin, info.remainSec,
				K2L_Compatibility("\xb9\xcc\xc8\xae\xc0\xce \xc6\xbc\xc5\xb0\xb8\xae\xc6\xf7\xc6\xae\xc0\xc7 \xba\xb8\xc0\xaf\xb7\xae\xc0\xcc 10\xb0\xb3\xb8\xa6 \xc3\xca\xb0\xfa \xc7\xcf\xbf\xb4\xc0\xb8\xb9\xc7\xb7\xce \xb4\xf5\xc0\xcc\xbb\xf3 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9 \xc6\xbc\xc5\xb0\xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbd\xc3\xb1\xe2 \xc0\xa7\xc7\xd8\xbc\xad\xb4\xc2 \xba\xb8\xc0\xaf\xc7\xcf\xb0\xed \xc0\xd6\xb4\xc2 \xb8\xae\xc6\xf7\xc6\xae\xb8\xa6 \xc8\xae\xc0\xce\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.\n\n\xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce \xb9\xe6\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"));
		else
			msg = MakeStr(
			K2L_Compatibility("\xc7\xd8\xb4\xe7 \xb0\xd4\xc0\xd3\xc0\xba \xb0\xd4\xc0\xd3 \xbd\xc3"
			"\xc0\xdb \xc8\xc4\n\\c0xffff0000\\c'%2d\xba\xd0%2d\xc3\xca'"
			"\\c0xff000000\\c\xb0\xa1 \xc1\xf6\xb3\xb5\xb0\xed "
			"\\c0xffff0000\\c'%2d\xba\xd0%2d\xc3\xca'\\c0xff000000\\c\xb0\xa1"
			"\n\xb3\xb2\xbe\xc6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\n\xc1\xf8"
			"\xc7\xe0\xc1\xdf\xc0\xce \xb9\xe6\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xcf"
			"\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			info.elapsedMin, info.elapsedSec, info.remainMin, info.remainSec);
		m_pDlg->SetMessage(msg, false);
		m_pDlg->Open((FRESH_PFN_RESULT)&CIntrusion::OnAlarmDlgResult, 1);
		break;
	}

	case 7:
	{
		std::string nick = packet.DecodeStr();
		CChatMsg::Instance()->AddChatMsg(
			MakeStr(K2L_Compatibility("(\xbe\xcb\xb8\xb2) \\c0xffffffff\\c%s\\c0xffff7878\\c"
					"\xb4\xd4\xc0\xcc \xb0\xd4\xc0\xd3\xc1\xdf\xbf\xa1 \xc0\xd4"
					"\xc0\xe5\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
				nick.c_str()),
			0xffff7878, true, false);

		packet.DecodeBuffer(&Doc()->m_roomInfo, sizeof(sRoomInfo));
		Doc()->m_chatManager.FilteringHack(Doc()->m_roomInfo.title, false);

		unsigned long tidMatch = Doc()->m_roomInfo.tidMatch;
		if (Doc()->m_roomType != tidMatch)
		{
			IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(tidMatch);
			if (pMatch)
			{
				Doc()->m_roomType = tidMatch;
				CChatMsg::Instance()->AddChatMsg(
					MakeStr(
						K2L_Compatibility("(\xbe\xcb\xb8\xb2) \xb4\xeb\xc8\xb8 \xb5\xee\xb1"
						"\xde\xc0\xcc \\c0xffffffff\\c[ %s ]\\c0xffff7878\\c"
						"\xc0\xb8\xb7\xce \xba\xaf\xb0\xe6 \xb5\xc7\xbe\xfa"
						"\xbd\xc0\xb4\xcf\xb4\xd9."),
						pMatch->Name),
					0xffff7878, true, false);
			}
		}

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;
		slotList.clear();

		int num = packet.Decode1();
		int bonus = packet.Decode4();

		for (int i = 0; i < num; i++)
		{
			sSlotInfo slot;
			packet.DecodeBuffer(&slot, sizeof(sSlotInfo));

			if (slot.bMaster)
				slot.bReady = 1;
			else
				slot.bReady = 0;

			slotList.push_back(slot);
		}

		PlayersUpdate(bonus);
		break;
	}

	case 4:
		DoIntrusionAccess(packet, (eIntrusionPermitCode)code);
		break;

	case 5:
		m_pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"IntrusionAlarm", NULL);
		m_pDlg->SetCaption(
			K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4\xc0"
			"\xe5"));
		m_pDlg->SetDesc(K2L_Compatibility(" \xa1\xd8 '\xbf\xb9'\xb8\xa6 \xb4\xa9\xb8\xa3\xb8\xe9 "
						"\xb4\xd9\xbd\xc3 \xc0\xd4\xc0\xe5\xc0\xbb \xbd\xc3\xb5"
						"\xb5\xc7\xd5\xb4\xcf\xb4\xd9."));
		m_pDlg->SetMessage(K2L_Compatibility("\xc0\xaf\xc0\xfa\xb0\xa1 \xb8\xb9\xbe\xc6 \xc0\xd4"
						   "\xc0\xe5\xbf\xa1 \xbd\xc7\xc6\xd0 \xc7\xcf\xbf\xb4"
						   "\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3"
						   "\xb5\xb5 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf"
						   "\xb1\xee?"),
			false);
		m_pDlg->Open((FRESH_PFN_RESULT)&CIntrusion::OnDelayDlgResult, 1);
		break;

	case 6:
		DenyAnlaysis((eIntrusionDenyCode)code);
		break;

	case 9:
	{
		CPowerGauge::Instance()->ResetRandTable983(); // 983: 008577ac
		unsigned long uid = packet.Decode4();
		unsigned long quitOrder = packet.Decode4();
		unsigned long myGuid = Doc()->m_myInfo.info.dwGuid;

		if (uid == myGuid)
		{
			for (std::vector<sRivalData>::iterator it =
					 Doc()->m_rivalList.begin();
				it != Doc()->m_rivalList.end(); ++it)
			{
				(*it).quitOrder = quitOrder;
			}
			break;
		}

		sIntrusionScore score;
		memset(&score, 0, sizeof(score));

		std::vector<sRivalData>::iterator it =
			std::find_if(Doc()->m_rivalList.begin(), Doc()->m_rivalList.end(),
				sFindRival(myGuid));
		if (it != Doc()->m_rivalList.end())
		{
			sIntrusionHoleScore hole;
			memset(&hole, 0, sizeof(hole));

			memcpy(hole.holeScore, (*it).holeScore, sizeof(hole.holeScore));
			memcpy(hole.holeStroke, (*it).holeStroke, sizeof(hole.holeStroke));
			memcpy(hole.holePang, (*it).holePang, sizeof(hole.holePang));

			score.hole = hole;
			score.uid = uid;
			score.sender = Doc()->m_myInfo.info.dwGuid;
		}
		else
		{
			score.uid = uid;
			score.sender = 0xffffffff;
		}

		WSendPacket sendPacket((enumClientPacket)0x9d);
		sendPacket.Encode1(2);
		sendPacket.EncodeBuffer(&score, sizeof(score));
		sendPacket.Send(TO_GAME);
		break;
	}

	case 10:
		RivalDataUpdate(packet);
		break;

	case 8:
		m_calcExpMemberSize = packet.Decode1();
		break;
	}
}

void CIntrusion::RivalDataUpdate(WReceivedPacket& packet)
{
	unsigned long oid = packet.Decode4();
	unsigned char totalStroke = packet.Decode1();
	unsigned char hole = packet.Decode1();
	int totalScore = packet.Decode4();
	__int64 totalPang = packet.Decode8();

	sIntrusionScore score;
	memset(&score, 0, sizeof(score));
	packet.DecodeBuffer(&score, sizeof(score));

	if (oid == Doc()->m_myInfo.info.dwGuid)
		return;

	std::vector<sRivalData>::iterator it = std::find_if(
		Doc()->m_rivalList.begin(), Doc()->m_rivalList.end(), sFindRival(oid));
	if (it == Doc()->m_rivalList.end())
		return;

	(*it).totalStroke = totalStroke;
	(*it).hole = hole;
	(*it).totalScore = totalScore;
	(*it).totalPang = totalPang;

	memcpy((*it).holeStroke, score.hole.holeStroke, sizeof((*it).holeStroke));
	memcpy((*it).holeScore, score.hole.holeScore, sizeof((*it).holeScore));
	memcpy((*it).holePang, score.hole.holePang, sizeof((*it).holePang));

	IActor* pActor = AfxGetTask()->GetActor("Score");
	if (pActor)
		pActor << MsgObject(NULL, 0x16e/*983*/, 0, 0, 0, 0, 0);

	pActor = AfxGetTask()->GetActor("Spot");
	if (pActor)
		pActor << MsgObject(NULL, 0x125/*983*/, 0, 0, 0, 0, 0);
}

void CIntrusion::PlayersUpdate(int bonus)
{
	std::list<sSlotInfo>& slotList = Doc()->m_slotList;
	std::vector<sRivalData>& rivalList = Doc()->m_rivalList;

	for (std::list<sSlotInfo>::iterator it = slotList.begin();
		it != slotList.end(); ++it)
	{
		std::vector<sRivalData>::iterator rival = std::find_if(
			rivalList.begin(), rivalList.end(), sFindRival((*it).dwGuid));
		if (rival != rivalList.end())
		{
			(*rival).quitOrder = bonus;
			continue;
		}

		if ((*it).dwGuid == -1)
		{
			GOLFDOC()->GetPlayer(0)->state = 3;
			continue;
		}

		Doc()->SetIndex((*it).dwGuid);

		sRivalData data;
		data.Clear983();
		strcpy(data.nickname, (*it).sNick);
		strcpy(data.guildName, (*it).sGuild);

		data.order = bonus;
		data.quitOrder = bonus;
		data.uid = (*it).dwUserUID;
		data.capability = (*it).dwIdentity;
		data.guildEmblemVer983 = (*it).dwEmblemVer983; // 983

		strcpy(data.guildMark, (*it).szEmblemName);

		data.finishOrder = slotList.size();
		data.flag = (*it).gender;
		data.oid = (*it).dwGuid;
		data.guildUID = (*it).GuildId;
		data.hole = Doc()->m_holeOrder[0];
		data.level = (*it).level;
		data.pangMastery = (*it).pangma[0];
		data.pangNitro = (*it).pangma[1];
		data.ballPos = GOLFDOC()->GetHoleData().tee;
		data.team = (*it).bTeam;
		data.observerState = 1;
		data.rank = 1;
		data.observerRank = 1;
		data.state = 0;
		data.mascotTypeId = (*it).tidMascot;
		data.titleTypeId = (*it).dwTitle;

		rivalList.push_back(data);
	}

	IActor* pActor = AfxGetTask()->GetActor("Score");
	if (pActor)
		pActor << MsgObject(NULL, 0x16e/*983*/, 0, 0, 0, 0, 0);

	pActor = AfxGetTask()->GetActor("Spot");
	if (pActor)
		pActor << MsgObject(NULL, 0x125/*983*/, 0, 0, 0, 0, 0);
}

void CIntrusion::GetInfoTime(sInfoTime* pInfo, unsigned long elapsed,
	unsigned long limit)
{
	int elapsedSec = elapsed / 1000;
	int limitSec = limit / 1000;

	int remainSec = limitSec - elapsedSec;
	pInfo->elapsedMin = elapsedSec / 60;
	pInfo->elapsedSec = elapsedSec - (elapsedSec / 60) * 60;
	pInfo->remainMin = remainSec / 60;
	pInfo->remainSec = remainSec - (remainSec / 60) * 60;
}

struct sIntrusionEquip
{
	unsigned long guidChar;
	unsigned long guidCaddie;
	unsigned long guidClubSet;
	unsigned long tidBall;
};

bool CIntrusion::OnAlarmDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		WSendPacket packet((enumClientPacket)0x9d);
		packet.Encode1(1);
		packet.Encode2(m_roomGuid);
		packet.Send(TO_GAME);

		unsigned long guidCaddie = Doc()->m_myInfo.userEquip.guidCaddie;
		std::map<unsigned int, sCaddieInfo>& caddieMap = Doc()->m_caddieMap;
		std::map<unsigned int, sCaddieInfo>::iterator it =
			caddieMap.find(guidCaddie);
		if (it != caddieMap.end())
		{
			IFF_STRUCT::sCaddie* pCaddie =
				ItemManager()->FindCaddie((*it).second.tid);
			if ((*it).second.Rent_flag && (*it).second.Remain_Date == 0 &&
				pCaddie && pCaddie->MonthlyFee > 0)
				guidCaddie = 0;
		}

		sIntrusionEquip equip;
		memset(&equip, 0, sizeof(equip));
		equip.guidChar = Doc()->m_myInfo.userEquip.guidChar;
		Doc()->m_myInfo.userEquip.guidCaddie = guidCaddie;
		equip.guidCaddie = guidCaddie;
		equip.guidClubSet = Doc()->m_myInfo.userEquip.guidClubSet;
		equip.tidBall = Doc()->m_myInfo.userEquip.tidBall;

		WSendPacket equipPacket((enumClientPacket)0xc);
		equipPacket.Encode1(7);
		equipPacket.EncodeBuffer(&equip, sizeof(equip));
		equipPacket.Send(TO_GAME);
	}
	else
	{
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x3b), 0, 0, 0, 0, 0);
		Doc()->m_myInfo.roomIndex = 0xffff;
	}

	m_pDlg = NULL;
	return true;
}

bool CIntrusion::OnDelayDlgResult(int result, FrForm* pForm)
{
	if (result != 1)
		AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x3b), 0, 0, 0, 0, 0);

	m_pDlg = NULL;
	return true;
}

bool CIntrusion::OnDenyDlgResult(int result, FrForm* pForm)
{
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, M983(0x3b), 0, 0, 0, 0, 0);

	m_pDlg = NULL;
	return true;
}

void CIntrusion::DenyAnlaysis(eIntrusionDenyCode code)
{
	if (m_pDlg != NULL)
		return;

	m_pDlg =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "IntrusionDeny", NULL);
	m_pDlg->SetCaption(
		K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4\xc0\xe5"));
	m_pDlg->SetDesc(
		K2L_Compatibility(" \xa1\xd8 \xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4\xc0\xe5 \xb0\xe1\xb0\xfa"));

	switch (code)
	{
	case INTRUSION_DENY_FULL:
		m_pDlg->SetMessage(
			K2L_Compatibility("\xc0\xce\xbf\xf8\xc0\xcc \xb0\xa1\xb5\xe6 \xc2\xf7"
			"\xbc\xad \xb9\xe6\xbf\xa1 \xc0\xd4\xc0\xe5 \xc7\xcf"
			"\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_TIMEOUT:
		m_pDlg->SetMessage(
			K2L_Compatibility("\xb0\xd4\xc0\xd3 \xbd\xc3\xc0\xdb \xc8\xc4 5\xba"
			"\xd0\xc0\xcc \xc1\xf6\xb3\xad \xb9\xe6\xbf\xa1\xb4"
			"\xc2 \xc0\xd4\xc0\xe5 \xc7\xcf\xbd\xc7 \xbc\xf6 \xbe"
			"\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_PRIVATE:
		m_pDlg->SetMessage(
			K2L_Compatibility("\xba\xf1\xb0\xf8\xb0\xb3 \xb9\xe6\xbf\xa1\xb4\xc2 "
			"\xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4\xc0\xe5 \xc7\xcf"
			"\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_ALREADY_ENTERED:
		m_pDlg->SetMessage(K2L_Compatibility("\xc0\xcc\xc0\xfc\xbf\xa1 \xb0\xd4\xc0\xd3\xc1\xdf "
						   "\xc0\xd4\xc0\xe5\xc7\xd1 \xb1\xe2\xb7\xcf\xc0\xcc "
						   "\xc0\xd6\xbe\xee\xbc\xad \xb4\xe7\xba\xd0\xb0\xa3 "
						   "\xc0\xd4\xc0\xe5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8"
						   "\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_NOT_ALLOWED:
		m_pDlg->SetMessage(K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xc1\xdf \xc0\xd4"
						   "\xc0\xe5\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd1 "
						   "\xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_ALREADY_PLAYED:
		m_pDlg->SetMessage(K2L_Compatibility("\xc7\xd8\xb4\xe7 \xb9\xe6\xbf\xa1\xbc\xad \xc7\xc3"
						   "\xb7\xb9\xc0\xcc\xc7\xd1 \xb1\xe2\xb7\xcf\xc0\xcc "
						   "\xc0\xd6\xbe\xee\xbc\xad \xb0\xd4\xc0\xd3\xc1\xdf "
						   "\xc0\xd4\xc0\xe5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8"
						   "\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		break;

	case INTRUSION_DENY_KICKED:
		m_pDlg->SetMessage(K2L_Compatibility("\xc7\xd8\xb4\xe7 \xb9\xe6\xbf\xa1\xbc\xad \xb0\xad"
						   "\xc1\xa6 \xc5\xf0\xc0\xe5\xb4\xe7\xc7\xd1 \xb1\xe2"
						   "\xb7\xcf\xc0\xcc \xc0\xd6\xbe\xee\xbc\xad \xc0\xd4"
						   "\xc0\xe5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0"
						   "\xb4\xcf\xb4\xd9."),
			false);
		break;
	}

	m_pDlg->Open((FRESH_PFN_RESULT)&CIntrusion::OnDenyDlgResult, 3);
}

bool CIntrusion::IsIntrusionGame(unsigned short roomGuid)
{
	bool bIntrusion = false;
	for (std::list<sRoomInfo>::iterator it = Doc()->m_roomList.begin();
		it != Doc()->m_roomList.end(); it++)
	{
		if ((*it).roomGuid == roomGuid)
		{
			bIntrusion = (*it).bIntrusion;
			break;
		}
	}
	return bIntrusion;
}

void CIntrusion::DoJoinRoom(unsigned short roomGuid)
{
	m_roomGuid = roomGuid;

	WSendPacket packet((enumClientPacket)0x9d);
	packet.Encode1(0);
	packet.Encode2(m_roomGuid);
	packet.Send(TO_GAME);
}
