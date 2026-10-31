#include "minatl.h"
#include "jp/shareddocjp.h"
#include "vc10_layout.h"
#include "exhibition.h"
#include "contentsdoc.h"
#include "cardsystem.h"
#include "partsinfo.h"
#include "golfball.h"
#include "../shared/sharedtables.h"
#include "lobbytask.h"
#include "shoptask.h"
#include "frcombobox.h"
// HACK: frcombobox.h is in the PCH now (treasuredlg needs FrComboCtlEx::GetCurrentIdx as a PCH
// inline); this pad gives the TU back the IL of its inline bodies, so the PCH inlines
// (std::_Ptr_cat) keep their place among this file's functions.
namespace RbShareddocIlPad
{
	struct Pad
	{
		int v;
		// clang-format off
		void g() { ILFILL16 ILFILL16 ILFILL4 ILFILL4 v += 1; }
		// clang-format on
	};
}
#include "../shared/token.h"
#include "club.h"
#include "../shared/s5/sharedutilities.h"
#include "jp/conditionalauxpartobject.h"
#include "jp/controllerdoc.h"
#include "jp/combogaugeobject.h"
#include "jp/itemupdater.h"
static float BAR_LENGTH = BAR_END - BAR_START;
#include "standinginline.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A803 temporario
#include "mathconsts.h"

extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
char* GetStr(char* str, char* out);
void LineParse(const char* line, std::vector<std::string>& tokens);

// 983: tabelas preenchidas por K2L_Compatibility num inicializador estatico (00cd37b0..)
const char* defaultTitles_vs[] = {
	K2L_Compatibility("\xc6\xce\xbe\xdf~ \xc6\xce\xbe\xdf!"),
	K2L_Compatibility("\xb3\xca\xb6\xfb \xb3\xaa\xb6\xfb \xc6\xce\xbe\xdf~"),
	K2L_Compatibility("\xb9\xd9\xb6\xf7 \xc1\xc1\xc0\xba \xb9\xe6~"),
	K2L_Compatibility("\xc6\xce\xbe\xdf\xbf\xcd \xc7\xd4\xb2\xb2\xb6\xf3\xb8\xe9~!"),
	K2L_Compatibility("\xbf\xb9\xbb\xdb \xbd\xba\xc0\xae, \xb8\xda\xc1\xf8 \xbc\xa6!"),
	K2L_Compatibility("\xb3\xaf\xbe\xc6\xb6\xf3 \xbe\xc6\xc1\xee\xc5\xd8~"),
	K2L_Compatibility("\xc0\xfa \xc7\xcf\xb4\xc3\xc0\xc7 \xc6\xce\xc0\xcc \xb5\xc7\xbe\xee~"),
	K2L_Compatibility("\xc6\xce\xbe\xdf\xb0\xa1 \xc3\xd6\xb0\xed!"),
	K2L_Compatibility("\xb4\xd9\xc7\xd4\xb2\xb2 \xc6\xae\xb7\xb9\xc0\xfa \xc7\xe5\xc6\xae!"),
};

const char* defaultTitles_mass[] = {
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8\xc0\xbb \xb3\xbb \xc7\xb0\xbe\xc8\xbf\xa1"),
	K2L_Compatibility("\xc6\xce\xbe\xdf\xb4\xeb\xc8\xb8\xb7\xce \xb8\xf0\xc0\xcc\xbc\xbc\xbf\xe4~"),
	K2L_Compatibility("\xb8\xf0\xb5\xce\xb8\xf0\xb5\xce \xc6\xce\xbe\xdf!"),
	K2L_Compatibility("\xb1\xe2\xb7\xcf \xb0\xbb\xbd\xc5\xc0\xc7 \xb1\xd7\xb3\xaf\xb1\xee\xc1\xf6!"),
	K2L_Compatibility("1\xb5\xee \xb1\xe2\xb4\xd9\xb7\xc1~"),
	K2L_Compatibility("\xc6\xae\xb7\xce\xc7\xc7\xb4\xc2 \xb3\xbb\xb2\xa8~"),
};

const char* defaultTitles_battle[] = {
	K2L_Compatibility("\xbd\xac\xbe\xee\xb0\xa1\xb4\xc2 \xc7\xd1 \xc6\xc7!"),
	K2L_Compatibility("\xc7\xd1\xb9\xe6\xbd\xc2\xba\xce! \xc6\xce\xb9\xe8\xc6\xb2~"),
	K2L_Compatibility("\xb4\xeb\xb9\xda \xb2\xde, \xbd\xf1\xbe\xc6\xc1\xf6\xb4\xc2 \xc6\xce"),
	K2L_Compatibility("\xbb\xf5\xb7\xce\xbf\xee \xbd\xc3\xc1\xf0 \xc6\xce\xb9\xe8\xc6\xb2 \xc7\xd1 \xc6\xc7!"),
};

const char* defaultTitles_approach[] = {
	K2L_Compatibility("\xbd\xac\xbe\xee\xb0\xa1\xb4\xc2 \xc7\xd1 \xc6\xc7!"),
	K2L_Compatibility("\xc7\xd1\xb9\xe6\xbd\xc2\xba\xce! \xbe\xee\xc7\xc1\xb7\xce\xc4\xa1~"),
	K2L_Compatibility("\xbd\xc3\xc1\xf0 NEW \xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"),
};

const char* defaultTitles_chat[] = {
	K2L_Compatibility("\xbc\xd2\xb3\xe2, \xbc\xd2\xb3\xe0\xb8\xa6 \xb8\xb8\xb3\xaa\xb4\xd9"),
	K2L_Compatibility("\xbc\xf6\xb4\xd9\xc0\xef\xc0\xcc, \xbb\xf5\xc4\xa7\xb6\xbc\xb1\xe2"),
	K2L_Compatibility("\xc0\xce\xbf\xac\xc0\xcc \xb8\xf0\xc0\xcc\xb4\xc2 \xb0\xf7"),
	K2L_Compatibility("\xc6\xce\xbe\xdf! \xc1\xf1\xb0\xc5\xbf\xee \xb8\xb8\xb3\xb2~"),
	K2L_Compatibility("\xc6\xce\xbe\xdf\xbf\xf9\xb5\xe5\xbf\xcd \xc7\xd4\xb2\xb2!"),
	K2L_Compatibility("\xc6\xce\xbe\xdf\xc0\xce\xb5\xe9 \xb4\xd9 \xb8\xf0\xbf\xa9\xb6\xf3~!"),
	K2L_Compatibility("\xc4\xf0 \xb0\xa1\xc0\xcc! \xb3\xaa\xc0\xcc\xbd\xba \xb0\xc9!"),
};

const char* defaultTitles_event[] = {
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8 \xc0\xcc\xba\xa5\xc6\xae(\xc1\xdf\xb1\xde)"),
	K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8 \xc0\xcc\xba\xa5\xc6\xae(\xbb\xf3\xb1\xde)"),
};

std::string CSharedDoc::m_packetNickname; // provisórios (ver shareddoc.h)
std::string CSharedDoc::m_nicknameSuffix("");
unsigned long CSharedDoc::m_cardPackPageNum, CSharedDoc::m_cardPackSelPageNum, CSharedDoc::m_cardPackCurPage, CSharedDoc::m_cardPackMode;
bool CSharedDoc::m_bCardPackRebuild, CSharedDoc::m_bRmrTabPending;
unsigned char CSharedDoc::m_cardPackSlot, CSharedDoc::m_cardPackItem;
unsigned short CSharedDoc::m_rmrTabValue;
sStall CSharedDoc::m_stallItem;
std::list<sStall> CSharedDoc::m_listStallItem;
std::list<sSaleItem> CSharedDoc::m_maketList;
unsigned char CSharedDoc::m_tradeMode;
std::list<sGiftInfo> CSharedDoc::m_giftList; // provisório (ver shareddoc.h)
std::list<sGiftInfo> CSharedDoc::m_sendGiftList;

CSharedDoc::CSharedDoc()
	: m_maxWhisperPartner(8), m_serverProperty(0)
{
	// mapa de layout do 983 (docs/LAYOUTS.md, A193): ClearInventory 00647800, OnPacket 0x48 00ac0ea5, 0062ba40, 0063dca0, 00a806fc
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812); // 983 005fae70 (premium = bit 14)
	// A223: sPangYaUserInfo (jogador) inteiro pelo 983: nomes por conv_ghostdata_to_userinfo 00828130; cauda por Doc+0x7a2+desl
	C_ASSERT(sizeof(sPangYaUserInfo) == 0x12d);
	C_ASSERT(offsetof(sPangYaUserInfo, sNick) == 0x16); C_ASSERT(sizeof(((sPangYaUserInfo*)0)->sNick) == 22); // 00827f30
	C_ASSERT(offsetof(sPangYaUserInfo, sGuild) == 0x2c); C_ASSERT(sizeof(((sPangYaUserInfo*)0)->sGuild) == 20); // 00827f70
	C_ASSERT(offsetof(sPangYaUserInfo, szEmblemName) == 0x40); C_ASSERT(sizeof(((sPangYaUserInfo*)0)->szEmblemName) == 12); // 00827fb0
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.szEmblemName) == 0x7e2); // FrMessengerDlg (guilda online)
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.school) == 0x80e); // user_info/optiondlg 008ce4ce
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwGalleryGuid) == 0x816); // lobbymain 00503912
	C_ASSERT(offsetof(sPangYaUserInfo, iBongdariShopUsableTimes) == 0x93); // bongdarishopdlg 008ba1e8 (word Doc+0x835/37/39)
	C_ASSERT(offsetof(sPangYaUserInfo, dwPointPointEvent) == 0x99); // pointeventdlg 00b0df16 (Doc+0x83b)
	C_ASSERT(offsetof(sPangYaUserInfo, dwFlagBlock) == 0x9d); // taskmain 0065a1c5 (Doc+0x83f)
	C_ASSERT(offsetof(sPangYaUserInfo, iTimeBlock) == 0xa1); // tikimagicboxdlg 00af7ed5 (Doc+0x843)
	C_ASSERT(offsetof(sPangYaUserInfo, sDisplayID) == 0xa9); // lobbymain 00ab617a (lea Doc+0x84b)
	C_ASSERT(offsetof(sPangYaUserInfo, dwUID) == 0x129); // Doc+0x8cb
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo) == 0x7a0 && offsetof(CSharedDoc, m_userInfo) == 0x38b0 && sizeof(sUserInfo) == 0x31c6);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cardPeriodList) == 0x381c && offsetof(CSharedDoc, m_itemCounts983) == 0x3828 && offsetof(CSharedDoc, m_questList) == 0x3834);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_questCounts983) == 0x3840 && offsetof(CSharedDoc, m_quests983) == 0x384c && offsetof(CSharedDoc, m_cookie) == 0x3858);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cash) == 0x3868 && offsetof(CSharedDoc, m_controlServerService) == 0x10d28 && offsetof(CSharedDoc, m_chatManager) == 0x10c24);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) == 0x108f4 && sizeof(sRoomInfo) == 0xdd && offsetof(sRoomInfo, bPublic) == 0x40);
	C_ASSERT(offsetof(sRoomInfo, nUserLimit) == 0x43 && offsetof(sRoomInfo, nHole) == 0x57 && offsetof(sRoomInfo, holeType) == 0x5b && offsetof(sRoomInfo, realGameType) == 0xc4);
	C_ASSERT(offsetof(sRoomInfo, artifactTid983) == 0xc5 && offsetof(sRoomInfo, roomFlags983) == 0xc9 && offsetof(sRoomInfo, gpTypeId983) == 0xcd && offsetof(sRoomInfo, gpTicket983) == 0xd1 && offsetof(sRoomInfo, gpStartValue983) == 0xd5 && offsetof(sRoomInfo, gpRoomFlags983) == 0xd9);
	C_ASSERT(sizeof(sMascotInfo) == 0x3e && offsetof(sMascotInfo, Remain_Date) == 0x2b && offsetof(sMascotInfo, endDate) == 0x2d && offsetof(sMascotInfo, PCBangMascot) == 0x3d);
	m_bNeedFirstLoginAlarm = true;
	m_bShopBuyDone983 = true; // 983 0064aba3
	m_itemUpdater983[0] = (unsigned long)new CItemUpdater; // 983 00589b50
	m_pGolfDoc = NULL;
	ClearAll();

	m_unused47cc = 0;
	m_unused47d0 = 0;

	memset(m_fileName, 0, sizeof(m_fileName));

	m_replayState = 0;
	m_replayPosition = 0;
	m_replayDuration = 0;
	m_replayTick = 0;
	m_replayPlayType = -1;
	m_playingMode = (eReplayModeType)1;
	m_replayShotIndex = 0;
	m_replayHoleStroke = 0;

	m_replayVersion = 0x413d; // 983 0064a84b (0x18f1 = 645: replay gravado pelo original nao aparecia na lista)
	m_rmrMode = 0;

	m_serverTimeTick = GetTickCount();
	m_cookie = 0;
	m_unused5d0c = 0;
	m_goStopMode = false;
	m_cash = 0;
	m_bonusCash = 0;
	m_directMoveType = 0xff;
	m_directMoveRoom = 0xffff;
	m_directMoveUID = 0xffffffff;
	// 983 (0064ac02-0064ac54, fim do construtor); A229: o canal do Grand Prix começa em 1 (o nosso ficava 0 = aba sem horário)
	m_holePrizeXmas983[2] = 0;
	m_holePrizeItem983[0] = m_holePrizeItem983[1] = m_holePrizeItem983[2] = 0;
	m_gpChannel983 = 1;
	m_roomListKind983 = 0;
	m_roomListRequest983 = 2;
	m_inviteMode = 0;
	m_lobbyState983 = 0;
	m_bPangyaBySet983 = false;
}

CSharedDoc::~CSharedDoc()
{
	std::map<unsigned long, sRoomSlot>::iterator it;
	for (it = Doc()->m_roomSlotMap.begin(); it != m_roomSlotMap.end(); ++it)
	{
		if ((*it).second.pExhibition)
		{
			delete (*it).second.pExhibition;
			(*it).second.pExhibition = NULL;
		}
	}
	m_roomSlotMap.clear();

	ClearRivalVars(false);
	UccManager()->Clear();
	delete (CItemUpdater*)m_itemUpdater983[0];
}
void CSharedDoc::ClearAll()
{
	ClearCommonVars();
	ClearInventory();
	ClearGameVars();
	ClearRivalVars(false);
	InitGameType();
	ClearLobbyStateVars();
	InitTradeData();

	ClearOldSeasonStat();
	ClearServerVars();
	ClearJapanVars();
	ClearRealEquip();

	if (IsLocalContent(S4_UCC))
	{
		ClearUcc();
	}

	if (IsLocalContent(S3_MAP_EVENT))
		m_mapEventList.clear();

	if (IsLocalContent(S4_REPLAY_SYSTEM))
		m_shotDataList.clear();

	SetUserInfoMod(0);
}

void CSharedDoc::ClearUcc()
{
	m_uccItemList.clear();
	m_uccAztecList.clear();
	UccManager()->Clear();
}

void CSharedDoc::ClearJapanVars()
{
	m_unused184 = 0;
	m_provType = -1;

	m_gachaTicket[0] = 0;
	m_gachaTicket[1] = 0;

	char cmdLine[260] = { 0 };
	char args[3][260];
	char temp[260];
	char* p;
	int len;

	strcpy(cmdLine, GetCommandLineA());

	for (int i = 0; i < 3; i++)
		args[i][0] = 0;

	if (cmdLine[0] == '"')
		p = GetStr(cmdLine, args[0]);
	else
	{
		sscanf(cmdLine, "%s", args[0]);
		p = cmdLine + strlen(args[0]);
	}

	sscanf(p, "%s %s", args[1], args[2]);

	if (args[1][0] != 0)
	{
		len = strlen(args[1]);

		if (args[1][0] == '"' && args[1][len - 1] == '"')
		{
			strncpy(temp, &args[1][1], len - 2);
			temp[len - 2] = 0;
			strcpy(args[1], temp);
			Doc()->m_commandLineID = temp;
		}
		else
			Doc()->m_commandLineID = args[1];
	}

	len = strlen(args[2]);

	if (args[2][0] == '"' && args[2][len - 1] == '"')
	{
		strncpy(temp, &args[2][1], len - 2);
		temp[len - 2] = 0;
		strcpy(args[2], temp);
		Doc()->m_password = temp;
	}
	else
		Doc()->m_password = args[2];

	// 983 0064aa08..0064ab5f: login de canal so com id e senha na linha de comando e id (minusculo, > 4) com @hg/@gr/@lv,
	// ou "-gpwccp"; senao 0 (A815: o nosso nao gravava -> lixo do heap no cl 14.44 escondia ID/senha do login)
	m_bChannelLogin = false;
	if (args[1][0] != 0 && args[2][0] != 0)
	{
		std::string id = _strlwr(args[1]);
		if (id.length() > 4 && (id.find("@hg", 0, 3) != std::string::npos || id.find("@gr") != std::string::npos || id.find("@lv") != std::string::npos))
			m_bChannelLogin = true;
		if (std::string(cmdLine).find("-gpwccp", 0, 7) != std::string::npos)
			m_bChannelLogin = true;
	}
}

void CSharedDoc::ClearCommonVars()
{
	m_loginServerUID = 0;
	m_bGameOver = false;
	m_holeType = 0;
	ClearGameMode();
	m_roomType = 0;
	m_eventType = 0;
	m_eventFlag = 2;
	m_pangRate = 1.0f;
	m_expRate = 100;
	m_offlinePlay = 0;
	m_teamPang[0] = 0;
	m_teamPang[1] = 0;
	m_newRecordState = 0xff;
	m_pcBang = 0;
	m_roomSetupStage = 0;

	m_bNewMapEvent = false;
	m_newMapEventMask = 0;
	m_bBingoEvent = false;
	m_newRecordCourse = 0xffffffff;

	memset(m_tutorialComplete, 0, sizeof(m_tutorialComplete));
	m_tutorialStartMode = 15;
	m_gameServerPort = 0;
	m_gameServerUID = 0;
	m_unused80 = 0;
	m_unused88 = 0;

	LoadLevelTable();

	unsigned char i;
	for (i = 0; i < 18; i++)
	{
		m_holeRandom[i] = rand();
		m_courseMap[i] = m_holeRandom[i] % 3;
		m_holeOrder[i] = i + 1;
	}
	m_holeOrder[i] = 19;

	// 983 0063d7e0: cifrados = 0xff (nenhum); sem isto GetFixHole983 devolvia ~chave, aleatorio a cada sessao
	for (i = 0; i < 19; i++)
		m_holeOrderCrypt983[i] = 0xff;
	m_fixHole983 = 0xff;

	m_noteList.clear();

	memset(&m_serverTime, 0, sizeof(m_serverTime));

	m_chatMode = 0;

	m_bTutorialResume = false;
	memset(m_auxPartTidList, 0, sizeof(m_auxPartTidList));

	m_guildBonusPang = 0;
	m_guildPoint = 0;
	m_guildTeam = 0;
	m_bHaveHalloweenItem = false;
	m_bWearHalloweenItem = false;
	m_giftName = "";

	m_giftIndex = -1;

	m_bRefreshCamera = false;
	m_bMessengerAlarm = false;
	m_bMessengerInit = false;
	m_friendMap.clear();
	memset(&m_selFriend, 0, sizeof(m_selFriend));

	m_needParentAgree = 0;
	m_controlServerService = 0;
	m_serverProperty = 0;
	m_flagAbilityItem = 0;
	m_numBonusPangItem = 0;
	m_cardPackPageNum = 0;
	m_cardPackSelPageNum = 0;
	m_cardPackCurPage = 0;
	m_cardPackMode = 1;
	m_cardPackSlot = 0;
	m_cardPackItem = 0;
	m_rmrTabValue = 0;
	m_bCardPackRebuild = false;
	m_bRmrTabPending = false;
	m_specialShotMark = 0;
	m_pendingMenu = 0;
	m_bToppageAd = false;
	m_toppageFeature = 0;

	if (IsLocalContent(S3_MAP_EVENT))
	{
	}

	m_appGiftDownloadState = (eAppGiftDownloadState)0;
	m_bItemStoragePin = false;
	m_bTutorialRedirect = false; // 983: a gravação de m_unused5bdc saiu
	m_replayTargetUID = 0xffffffff;

	if (IsLocalContent(S4_NT_NICKNAME_CHANGE))
	{
		m_forceNicknameChange = 0;
	}

	m_pPeriodDoc = NULL;

	// 983 fim de 0063d7e0
	m_bFixHoleRepeat983 = 0;
	memset(m_auxPartValues983, 0, sizeof(m_auxPartValues983));
	m_premiumEndNoticeText983 = "";
	m_bPremiumEndNotice983 = 0;
	m_bBongdariShopOpen = false;
}

void CSharedDoc::ClearInventory()
{
	memset(&m_myInfo, 0, sizeof(m_myInfo));
	m_cookie = 0;
	m_pointShopTP983 = 0;
	m_cash = 0;
	m_bonusCash = 0;

	m_stringMap.clear();
	m_charMap.clear();
	m_caddieMap.clear();
	m_mascotMap.clear();
	m_partsList.clear();
	m_clubSetMap.clear();
	m_ballList.clear();
	m_myItemList.clear();
	m_recordedItemList.clear();
	m_cadItemList.clear();
	m_setItemList.clear();
	m_giftList.clear();
	m_cutinList.clear();
	m_specialTrophyList.clear();
	m_guildTrophyList.clear();
	m_auxPartList.clear();
	m_questDropList.clear();
	m_questList.clear();
	m_furnitureList.clear();
	m_refreshGuidList.clear();
	m_refreshCountList.clear();
	m_questCounts983.clear();
	m_quests983.clear();
	m_itemCounts983.clear();

	ClearUserInfoTimeMap(0xffffffff);
}

void CSharedDoc::ClearGameMode()
{
	m_gameMode = 0;
}

void CSharedDoc::ClearUserInfoTimeMap(unsigned long uid)
{
	// 983: as listas de troféus são std::vector (o destrutor do sUserInfoTime 00644a00 libera); o 645 fazia delete[] de cada uma.
	if (uid == 0xffffffff)
	{
		m_userInfoTimeMap.clear();
	}
	else
	{
		std::map<unsigned long, sUserInfoTime>::iterator it = m_userInfoTimeMap.find(uid);

		if (it != m_userInfoTimeMap.end())
			m_userInfoTimeMap.erase(it);
	}
}

void CSharedDoc::ClearGameVars()
{
	m_bSentHoleStat = false;
	m_bTreasureResult = false;
	m_bTreasureAward = false;
	memset(m_holeStatistics, 0, sizeof(m_holeStatistics));
	memset(m_teamResult, 0, sizeof(m_teamResult));
	memset(m_guildScore, 0, sizeof(m_guildScore));

	memset(m_guildRoundScore, 0, sizeof(m_guildRoundScore));

	m_holeStatistics[3].dwExp = 0xffffffff;
	m_holeStatistics[2].dwExp = 0xffffffff;
	m_holeStatistics[1].dwExp = 0xffffffff;
	m_holeStatistics[0].dwExp = 0xffffffff;

	m_startHole = 0;
	m_eventType = 0;
	m_eventFlag = 2;

	m_indexMap.clear();
	m_bFixHoleMoveTee983 = 0; // 983 fim de ClearGameVars
}

// 983: 0062fdf0 - poe o estado em todos os rivais, menos os de estado 2 e 3
void CSharedDoc::SetRivalState983(unsigned char state)
{
	for (unsigned int i = 0; i < m_rivalList.size(); i++)
	{
		if (m_rivalList[i].state != 3 && m_rivalList[i].state != 2)
			m_rivalList[i].state = state;
	}
}


void CSharedDoc::ClearRivalVars(bool bAll)
{
	if (!bAll)
		memset(m_userInfo, 0, sizeof(m_userInfo));

	memset(m_teamPlayerGuid, 0, sizeof(m_teamPlayerGuid));

	m_rivalList.clear();
	m_guildMatchupList.clear();
	m_guildMatchFlagList.clear();

	// 983 00643830: depois das listas volta 0x114ac..b4 a {3.0 (00cfe8d4), 0, 1.0}
	m_courseScale983[0] = 3.0f;
	m_courseScale983[1] = 0.0f;
	m_courseScale983[2] = 1.0f;
}

int CSharedDoc::GetBuffValue(int type, int index)
{
	if (index != 20 && index != 16)
		return -1;

	return 0;
}

void CSharedDoc::ClearOldSeasonStat()
{
	memset(&m_oldSeasonStatistics, 0, sizeof(m_oldSeasonStatistics));
	memset(&m_oldSeasonTrophy, 0, sizeof(m_oldSeasonTrophy));

	m_oldSpecialTrophyList.clear();
	m_oldGuildTrophyList.clear();
}

void CSharedDoc::ClearStatistics()
{
}

void CSharedDoc::ClearCurrentChannel(bool bAll)
{
	memset(&m_curChannel, 0, sizeof(m_curChannel));
	m_curChannel.Uid = 0xff;
}

void CSharedDoc::ClearServerVars()
{
	ClearCurrentChannel(true);
	memset(&m_curGameServer, 0, sizeof(m_curGameServer));
	m_gameServerList.clear();
	m_channelList.clear();
}

void CSharedDoc::ClearLobbyStateVars()
{
	m_shopName = "";
	m_bAutoRefresh = false;
	m_bShowNotice = true;
	m_underBarMask = 0;
	m_bBackgroundVideo = false;
	m_bLobbyBgCameraSway983 = true;
	m_bReturnLobbyNotice = false;
	m_bRebuildUnderBar = false;

	memset(&m_roomInfo, 0, sizeof(m_roomInfo));

	sRoomInfo info;
	m_roomInfo = info;

	m_briefUserInfoMap.clear();
	m_roomList.clear();
	m_slotList.clear();
	m_slotList2.clear();
	m_roomUserList.clear();
	m_guildMemberMap.clear();
}

void CSharedDoc::ClearRealEquip()
{
	memset(&m_realEquip, 0, sizeof(m_realEquip));
}
const char* __fastcall GetGameString(int key); // 00540a10 (unidade da frente B: include/jp/gamestringmanager.h)
C_ASSERT_VC10(offsetof(CSharedDoc, m_lobbyLoginTime) == 0xb8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_gameTypeInfo) == 0xc8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_serverTime) == 0x180);
// s88: bloco de replay (645: 0x5b94..0x5bd4, +0xb690; +0x20 a partir de m_replayPosition)
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayState) == 0x11224 && offsetof(CSharedDoc, m_replayPlayType) == 0x11228);
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayPosition) == 0x11270 && offsetof(CSharedDoc, m_replayShotIndex) == 0x1127c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayHoleStroke) == 0x11280 && offsetof(CSharedDoc, m_replayVersion) == 0x11284);
// A201 (docrefs.py): convite/"ir junto" (messengerdlg 008fe450..008fe480, CLobbyTask::OnPacket 00ac03a0, 0062bc40..0062bcc0)
C_ASSERT_VC10(offsetof(CSharedDoc, m_serverTimeTick) == 0x11288 && offsetof(CSharedDoc, m_goWithMode) == 0x11290 && offsetof(CSharedDoc, m_goWithFriendUID) == 0x11294);
C_ASSERT_VC10(offsetof(CSharedDoc, m_inviteRoomInfo) == 0x11298 && offsetof(CSharedDoc, m_directMoveType) == 0x11375 && offsetof(CSharedDoc, m_directMoveRoom) == 0x11376);
C_ASSERT_VC10(offsetof(CSharedDoc, m_directMoveUID) == 0x11378 && offsetof(CSharedDoc, m_friendGameSvrUID) == 0x1137c && offsetof(CSharedDoc, m_inviteMode) == 0x11380);
// A201: lobby/chat (009c20a0, RFLobbyDesktop 005f7080, HandleMsgCommon 0065d8e0, CChatMsg::ClearHistory, CGolfTask::Process)
C_ASSERT_VC10(offsetof(CSharedDoc, m_bBackgroundVideo) == 0x10b26 && offsetof(CSharedDoc, m_bLobbyBgCameraSway983) == 0x10b27 && offsetof(CSharedDoc, m_bRebuildUnderBar) == 0x10b29);
C_ASSERT_VC10(offsetof(CSharedDoc, m_selChannel) == 0x10b2a && offsetof(CSharedDoc, m_chatLineList) == 0x10c98 && offsetof(CSharedDoc, m_chatHistory) == 0x10ca4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_ignoreList) == 0x10cb0 && offsetof(CSharedDoc, m_lobbyExitReason) == 0x10cbc && offsetof(CSharedDoc, m_chatModeName) == 0x10d08);
// A202 (docrefs.py 11214 70): rmr (taskmain 00651800/00657e70), renda da loja (taskmanager 009b9350/70, tradedlg 008f2010),
// halloween (0063b680/0063b6d0, fontinfo 00527cd0), playingMode (CGolfRule::ChangeGameMode), +0x11250 (HandleMsgCommon 0x2f0,
// FrLoginDlg::BeginConnect 00adaa51, CLoginUnit), string +0x11254 (CLobbyTask::OnPacket 00ac23dc, 006481a0)
C_ASSERT_VC10(offsetof(CSharedDoc, m_rmrMode) == 0x11214 && offsetof(CSharedDoc, m_tradeIncome) == 0x11218);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bHaveHalloweenItem) == 0x11220 && offsetof(CSharedDoc, m_bWearHalloweenItem) == 0x11221);
C_ASSERT_VC10(offsetof(CSharedDoc, m_playingMode) == 0x1122c && offsetof(CSharedDoc, m_bReturnToLogin983) == 0x11250 && offsetof(CSharedDoc, m_loginToken983) == 0x11254);
// A203 (docrefs.py 10d28 5c / 10fd4 58 / 11008 24): mascara de servicos 0062ba40, offline/pcBang/serverProperty, tutorial (tutorialmain 007d2850..),
// newRecord/newMapEvent (OpenNewRecordForm, FrNewMapEventDlg::OnScore*), gift (giftdlg 00aaf840, giftIndex=-1 em 0063d9a9), mensageiro
// (messengerdlg 00900fd0/00901220), time/guild (CLobbyTask::OnPacket 00ac3e58, scoredlg, resultdlg), vetores 006481a0, authTicket/pGolfDoc
C_ASSERT_VC10(offsetof(CSharedDoc, m_controlServerService) == 0x10d28 && offsetof(CSharedDoc, m_offlinePlay) == 0x10d30 && offsetof(CSharedDoc, m_pcBang) == 0x10d34);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomSetupStage) == 0x10d38 && offsetof(CSharedDoc, m_serverProperty) == 0x10d3c && offsetof(CSharedDoc, m_startHole) == 0x10d40);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bTutorialResume) == 0x10d41 && offsetof(CSharedDoc, m_newRecordState) == 0x10d42 && offsetof(CSharedDoc, m_bNewMapEvent) == 0x10d43);
C_ASSERT_VC10(offsetof(CSharedDoc, m_newMapEventMask) == 0x10d44 && offsetof(CSharedDoc, m_bBingoEvent) == 0x10d48 && offsetof(CSharedDoc, m_newRecordCourse) == 0x10d4c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_tutorialComplete) == 0x10d50 && offsetof(CSharedDoc, m_tutorialStartMode) == 0x10d5c && offsetof(CSharedDoc, m_giftName) == 0x10d60);
C_ASSERT_VC10(offsetof(CSharedDoc, m_giftIndex) == 0x10d7c && offsetof(CSharedDoc, m_bRefreshCamera) == 0x10d80 && offsetof(CSharedDoc, m_bMessengerAlarm) == 0x10d81);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bMessengerInit) == 0x10d82 && offsetof(CSharedDoc, m_levelTable) == 0x10d84 && offsetof(CSharedDoc, m_teamPlayerGuid) == 0x10fd4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_guildBonusPang) == 0x10fdc && offsetof(CSharedDoc, m_guildPoint) == 0x10fe0 && offsetof(CSharedDoc, m_guildTeam) == 0x10fe4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bShopBuyDone983) == 0x106fc);
C_ASSERT_VC10(offsetof(CSharedDoc, m_guildJoinList983) == 0x10fe8 && offsetof(CSharedDoc, m_guildInvites983) == 0x10ff8 && offsetof(CSharedDoc, m_authTicket) == 0x11008);
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && offsetof(CSharedDoc, m_pendingMenu) == 0x11028);
// A204: 0x1102c..0x11210 medidos com `tools/f35/docrefs.py 1102c 1e8` (escritas em 0063d7e0/0062b6d0/00647e70/006481a0,
// 0x1102c gravado por 009c20a0, 0x111c8 word por 00633140/00633150, 0x111cc por 00632f70)
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused5060) == 0x1102c && offsetof(CSharedDoc, m_specialShotMark) == 0x11030 && offsetof(CSharedDoc, m_bToppageAd) == 0x11034);
C_ASSERT_VC10(offsetof(CSharedDoc, m_toppageFeature) == 0x11038 && offsetof(CSharedDoc, m_realEquip) == 0x1103c && offsetof(CSharedDoc, m_mapEventList) == 0x110b0);
C_ASSERT_VC10(offsetof(CSharedDoc, m_shotDataList) == 0x110bc && offsetof(CSharedDoc, m_bItemStoragePin) == 0x110c8 && offsetof(CSharedDoc, m_bTutorialRedirect) == 0x110ca);
C_ASSERT_VC10(offsetof(CSharedDoc, m_partTidList) == 0x110cc && offsetof(CSharedDoc, m_auxPartTidList) == 0x11194 && offsetof(CSharedDoc, m_bHaveAngelWing) == 0x111a8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_loginAuthKey) == 0x111ac && offsetof(CSharedDoc, m_securityKey) == 0x111c8 && offsetof(CSharedDoc, m_flagAbilityItem) == 0x111cc);
C_ASSERT_VC10(offsetof(CSharedDoc, m_numBonusPangItem) == 0x111d0 && offsetof(CSharedDoc, m_stateTrade) == 0x111d4 && offsetof(CSharedDoc, m_listTradeItem) == 0x111d8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_listAttachCard) == 0x111e4 && offsetof(CSharedDoc, m_tradeUID) == 0x111f0 && offsetof(CSharedDoc, m_tradeTitle) == 0x111f4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_countVisitor) == 0x11210);
// A204: cauda 0x11384..0x11640 (`docrefs.py 11390 2b4`: 0062f540 grava 0x11398, 0063f280/0063f2d0/0063f2f0 os mapas 0x113e0/f0/400,
// GetVisGroup 0x11414, 006498d0 0x11444/0x1145c..0x1146a, 0062be40 0x11448..0x11454, 00643830 floats 0x114ac..b4, 00633580 0x115f8)
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused5cb0) == 0x11384 && offsetof(CSharedDoc, m_userInfoTimeMap) == 0x11388 && offsetof(CSharedDoc, m_userInfoMod) == 0x11398);
C_ASSERT_VC10(offsetof(CSharedDoc, m_forceNicknameChange) == 0x1139c && offsetof(CSharedDoc, m_pastMapStat) == 0x113a0 && offsetof(CSharedDoc, m_shopSaleState983) == 0x113e0);
C_ASSERT_VC10(offsetof(CSharedDoc, m_promoItems983) == 0x113f0 && offsetof(CSharedDoc, m_promoItemsNow983) == 0x11400 && offsetof(CSharedDoc, m_bBongdariShopOpen) == 0x11410);
C_ASSERT_VC10(offsetof(CSharedDoc, m_goStopMode) == 0x11411 && offsetof(CSharedDoc, m_visGroupMap) == 0x11414 && offsetof(CSharedDoc, m_appGiftDownloadState) == 0x11424);
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused5d0c) == 0x11428 && offsetof(CSharedDoc, m_cardStackList) == 0x1142c && offsetof(CSharedDoc, m_burnningSpCardList) == 0x11438);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bNeedFirstLoginAlarm) == 0x11444 && offsetof(CSharedDoc, m_playerCutinItem983) == 0x11448 && offsetof(CSharedDoc, m_bChannelLogin) == 0x11458);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holePrizeXmas983) == 0x11468 && offsetof(CSharedDoc, m_mapEvent983) == 0x1146c && offsetof(CSharedDoc, m_sdCommodityList983) == 0x1147c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_courseScale983) == 0x114ac && offsetof(CSharedDoc, m_holeOrderCrypt983) == 0x114c8 && offsetof(CSharedDoc, m_fixHole983) == 0x115f8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bFixHoleMoveTee983) == 0x11608 && offsetof(CSharedDoc, m_bFixHoleRepeat983) == 0x11609 && offsetof(CSharedDoc, m_bPremiumEndNotice983) == 0x1160a && offsetof(CSharedDoc, m_premiumEndNoticeText983) == 0x1160c && offsetof(CSharedDoc, m_itemUpdater983) == 0x11628);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomListKind983) == 0x11630 && offsetof(CSharedDoc, m_gpChannel983) == 0x1163c && offsetof(CSharedDoc, m_bPangyaBySet983) == 0x11640);
C_ASSERT(GAME_TYPE_USEMAX == 18 && GAME_TYPE_MAX == 22);

void CSharedDoc::InitGameType()
{
	m_gameTypeInfo[GAME_TYPE_STROKE].maxPlayer = 4;
	m_gameTypeInfo[GAME_TYPE_STROKE].minPlayer = 2;
	m_gameTypeInfo[GAME_TYPE_STROKE].holes = 3;
	m_gameTypeInfo[GAME_TYPE_STROKE].name =
		K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9");

	m_gameTypeInfo[GAME_TYPE_TEAM].maxPlayer = 4;
	m_gameTypeInfo[GAME_TYPE_TEAM].minPlayer = 2;
	m_gameTypeInfo[GAME_TYPE_TEAM].holes = 6;
	m_gameTypeInfo[GAME_TYPE_TEAM].name = K2L_Compatibility("\xb8\xc5\xc4\xa1");

	// 983 (0062b3d0): o GAME_TYPE_MATCH nao e mais preenchido.
	m_gameTypeInfo[GAME_TYPE_30S].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_30S].minPlayer = 10;
	m_gameTypeInfo[GAME_TYPE_30S].holes = 9;
	m_gameTypeInfo[GAME_TYPE_30S].name =
		K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8");

	m_gameTypeInfo[GAME_TYPE_30S_TEAM].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_30S_TEAM].minPlayer = 10;
	m_gameTypeInfo[GAME_TYPE_30S_TEAM].holes = 9;
	m_gameTypeInfo[GAME_TYPE_30S_TEAM].name =
		K2L_Compatibility("30\xc0\xce \xb4\xeb\xc8\xb8 \xc6\xc0\xc0\xfc");

	m_gameTypeInfo[GAME_TYPE_GUILD_MATCH].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_GUILD_MATCH].minPlayer = 10;
	m_gameTypeInfo[GAME_TYPE_GUILD_MATCH].holes = 9;
	m_gameTypeInfo[GAME_TYPE_GUILD_MATCH].name =
		K2L_Compatibility("\xb1\xe6\xb5\xe5 \xb4\xeb\xc0\xfc");

	m_gameTypeInfo[GAME_TYPE_REALMYROOM].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_REALMYROOM].minPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_REALMYROOM].holes = 1;
	m_gameTypeInfo[GAME_TYPE_REALMYROOM].name =
		K2L_Compatibility("\xb0\xf8\xb0\xb3 \xb8\xb6\xc0\xcc\xb7\xeb");

	m_gameTypeInfo[GAME_TYPE_SKINS].maxPlayer = 4;
	m_gameTypeInfo[GAME_TYPE_SKINS].minPlayer = 2;
	m_gameTypeInfo[GAME_TYPE_SKINS].holes = 6;
	m_gameTypeInfo[GAME_TYPE_SKINS].name =
		K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2");

	m_gameTypeInfo[GAME_TYPE_APPROACH].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_APPROACH].minPlayer = 10;
	m_gameTypeInfo[GAME_TYPE_APPROACH].holes = 1;
	m_gameTypeInfo[GAME_TYPE_APPROACH].name =
		K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1");

	m_gameTypeInfo[GAME_TYPE_NEW_APPROACH].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_NEW_APPROACH].minPlayer = 6;
	m_gameTypeInfo[GAME_TYPE_NEW_APPROACH].holes = 3;
	m_gameTypeInfo[GAME_TYPE_NEW_APPROACH].name =
		K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1");

	m_gameTypeInfo[GAME_TYPE_AVATARCHAT].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_AVATARCHAT].minPlayer = 10;
	m_gameTypeInfo[GAME_TYPE_AVATARCHAT].holes = 1;
	m_gameTypeInfo[GAME_TYPE_AVATARCHAT].name =
		K2L_Compatibility("\xb4\xeb\xc8\xad\xb9\xe6");

	m_gameTypeInfo[GAME_TYPE_HOLEINONE983].maxPlayer = 100; // 983: tipo novo
	m_gameTypeInfo[GAME_TYPE_HOLEINONE983].minPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_HOLEINONE983].holes = 1;
	m_gameTypeInfo[GAME_TYPE_HOLEINONE983].name =
		K2L_Compatibility("\xc8\xa6\xc0\xce\xbf\xf8");

	m_gameTypeInfo[GAME_TYPE_TUTORIAL_BASIC].maxPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_BASIC].minPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_BASIC].holes = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_BASIC].name =
		K2L_Compatibility("\xc3\xb3\xc0\xbd \xbd\xc3\xc0\xdb\xc7\xcf\xb1\xe2");

	m_gameTypeInfo[GAME_TYPE_TUTORIAL_ADV].maxPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_ADV].minPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_ADV].holes = 1;
	m_gameTypeInfo[GAME_TYPE_TUTORIAL_ADV].name =
		K2L_Compatibility("\xc4\xb3\xb5\xf0\xbf\xcd \xb9\xe8\xbf\xec\xb1\xe2");

	m_gameTypeInfo[GAME_TYPE_OFFLINE_GHOST].maxPlayer = 255;
	m_gameTypeInfo[GAME_TYPE_OFFLINE_GHOST].minPlayer = 1;
	m_gameTypeInfo[GAME_TYPE_OFFLINE_GHOST].holes = 18;
	m_gameTypeInfo[GAME_TYPE_OFFLINE_GHOST].name =
		K2L_Compatibility("\xbd\xba\xc6\xae\xb7\xce\xc5\xa9");

	m_gameTypeInfo[GAME_TYPE_USEMAX].maxPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_USEMAX].minPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_USEMAX].holes = 18;
	m_gameTypeInfo[GAME_TYPE_USEMAX].name = K2L_Compatibility(
		"\xba\xd2\xb0\xa1\xbb\xe7\xc0\xc7\xc7\xd1 \xc8\xa5\xb5\xb7");

	m_gameTypeInfo[GAME_TYPE_GRANDPRIX983].maxPlayer = 30; // 983: tipo novo
	m_gameTypeInfo[GAME_TYPE_GRANDPRIX983].minPlayer = 30;
	m_gameTypeInfo[GAME_TYPE_GRANDPRIX983].holes = 18;
	m_gameTypeInfo[GAME_TYPE_GRANDPRIX983].name = GetGameString(0x7543);
}

static const unsigned long s_bonusExp[3] = { 520, 680, 840 };

void CSharedDoc::LoadLevelTable()
{
	for (int i = 0; i < 71; i++)
	{
		m_levelTable[i].exp = g_LevelTable[i].exp;
		m_levelTable[i].totalExp =
			(i == 0 ? 0 : m_levelTable[i - 1].totalExp) + m_levelTable[i].exp;
	}

	m_bonusPangTable[0].exp = s_bonusExp[0];
	m_bonusPangTable[0].pang = s_bonusExp[0];
	m_bonusPangTable[1].exp = s_bonusExp[1];
	m_bonusPangTable[1].pang = s_bonusExp[0] + s_bonusExp[1];
	m_bonusPangTable[2].exp = s_bonusExp[2];
	m_bonusPangTable[2].pang = s_bonusExp[0] + s_bonusExp[1] + s_bonusExp[2];
}

void CSharedDoc::LoadItemDb()
{
	m_itemManager.Load();

	CPartTidList::SetItemManager(&m_itemManager);
}

void CSharedDoc::InitGolfDoc()
{
	SetCurMap(0);
	SetGameType(GAME_TYPE_STROKE);
	Doc()->m_holeType = 0;
	SetHoles(18);
	SetWeather(0);
	Doc()->m_golfGame.shotTimeLimit = 40000;
	Doc()->m_golfGame.gameTimeLimit = 0;
}

void CSharedDoc::LoadChatFilter()
{
	m_chatManager.Load();
}

bool CSharedDoc::AreWePlayingTogether(unsigned long uid)
{
	switch (m_golfGame.gameType)
	{
	case GAME_TYPE_STROKE:
	case GAME_TYPE_TEAM:
	case GAME_TYPE_SKINS:
		if (GOLFDOC() == NULL)
			break;

		for (unsigned char i = 0; i < GetPlayerNum(); i++)
		{
			if (PLAYER(i)->state == 3)
				continue;

			if (PLAYER(i)->oid == uid)
				return true;
		}
		break;

	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
	case GAME_TYPE_APPROACH:
	case GAME_TYPE_NEW_APPROACH:
	case GAME_TYPE_HOLEINONE983: // 983 00635c20: 0xb na lista de rivais; MATCH saiu dos dois
	{
		std::vector<sRivalData>::iterator it;
		for (it = m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
		{
			if (it->oid == uid)
				return it->state != 3;
		}
	}
	break;
	}

	return false;
}

bool CSharedDoc::SetWinningPrize(unsigned long uid, const sWinningPrize& prize)
{
	// 983 00635d24: premio so com o conteudo local 0x92
	if (!IsLocalContent((localContentType_t)0x92) || GOLFDOC() == NULL)
		return false;

	for (unsigned char i = 0; i < GetPlayerNum(); i++)
	{
		if (PLAYER(i)->oid == uid)
		{
			PLAYER(i)->prize = prize;
			return true;
		}
	}

	return false;
}

bool CSharedDoc::SetExtPrize(unsigned long uid, int num, sPrizeInfo* prize)
{
	if (GOLFDOC() == NULL)
		return false;

	for (unsigned char i = 0; i < GetPlayerNum(); i++)
	{
		if (PLAYER(i)->oid == uid)
		{
			if (num > 0)
			{
				int k = 0;
				for (unsigned int j = 0; j < 128; j++)
				{
					if (j >= PLAYER(i)->extPrizeNum && k < num)
					{
						PLAYER(i)->extPrize[j] = prize[k];
						k++;
					}
				}
			}
			PLAYER(i)->extPrizeNum += num;
			return true;
		}
	}

	return false;
}

bool CSharedDoc::CanCompound(unsigned long uid)
{
	IFF_STRUCT::sQuest* pQuest = m_itemManager.FindQuest(uid);
	if (pQuest == NULL)
		return false;

	std::list<sItemInfo>::iterator it;
	for (int i = 0; i < 5; i++)
	{
		unsigned long tid = pQuest->DropTid[i];
		if (tid == 0)
			continue;

		for (it = m_questDropList.begin(); it != m_questDropList.end(); ++it)
		{
			if (tid == (*it).tid)
			{
				if ((*it).Common[0] < pQuest->DropNum[i])
					return false;
				break;
			}
		}

		if (it == m_questDropList.end())
			return false;
	}

	return true;
}

bool CSharedDoc::HasSeventhCardSlot983(const sCharacterInfo* pInfo) // 006332f0
{
	for (int i = 0; i < 24; i++)
	{
		if (pInfo->tidParts[i] == 0)
			continue;

		IFF_STRUCT::sPart* pPart = Doc()->m_itemManager.FindPart(pInfo->tidParts[i]);
		if (pPart && pPart->CaddieSlot >= 1)
			return true;
	}
	return false;
}

// JP 983 (0x114b8 + 4 * jogador): vetor m_applied do CConditionalAuxpartObject do jogador (004ac4c0 refaz 5 elementos,
// 0062bea0 guarda o ponteiro); so o elemento [1] e lido aqui
C_ASSERT(sizeof(sAuxpartValue) == 0x14 && offsetof(sAuxpartValue, m_kind) == 4 && offsetof(sAuxpartValue, m_value) == 8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_auxPartValues983) == 0x114b8);
C_ASSERT(offsetof(IFF_STRUCT::sAuxPart, DriveUp) == 0xd4);
C_ASSERT(offsetof(IFF_STRUCT::sAuxPart, SetDriveUp983) == 0xde);
C_ASSERT(offsetof(IFF_STRUCT::sAuxPart, tidSet983) == 0xe0 && sizeof(IFF_STRUCT::sAuxPart) == 0xe4);
C_ASSERT(offsetof(IFF_STRUCT::sMascot, DriveUp) == 0x11a);
C_ASSERT(offsetof(IFF_STRUCT::sPart, CaddieSlot) == 0x22a);
C_ASSERT(offsetof(sCharacterInfo, tidParts) == 0xc && offsetof(sCharacterInfo, tidAuxParts) == 0x1a4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, charInfo.tidAuxParts) == 0x69a6);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, mascotInfo) == 0x6a38);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidMascot) == 0xa8e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidCaddie) == 0xa26);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidClubSet) == 0xa2e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_caddieMap) == 0x3714 && offsetof(CSharedDoc, m_clubSetMap) == 0x3740);

// JP 983 (00632fd0): campo novo do fim do ClubSet.iff (+0x100) do taco; 0x12 quando nao ha registro ou o campo e zero
unsigned int CSharedDoc::GetClubSetKind983(unsigned long tid)
{
	IFF_STRUCT::sClubSet* pClubSet = m_itemManager.FindClubSet(tid);
	if (pClubSet && pClubSet->nKind983)
		return pClubSet->nKind983;
	return 0x12;
}

// JP 983 (006362a0): bonus de alcance do conjunto (SetEffectTable.iff) do jogador. Monta o pedido no CItemManager
// (pecas auxiliares, taco, caddie e mascote), manda calcular e le o alcance do registro escolhido. O 983 passa o
// mesmo ponteiro (pecas auxiliares) nos dois primeiros argumentos.
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, clubInfo.tid) == 0x6a20);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, caddieInfo.tid) == 0x6a07);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, mascotInfo.tid) == 0x6a3c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) == 0x20c);
float CSharedDoc::GetRangeBonus983(unsigned char player) // 006362a0
{
	if (m_pGolfDoc)
	{
		if (GetPlayerNum() < player)
			return 0.0f;
	}

	unsigned long* pTids;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		pTids = m_userInfo[player].charInfo.tidAuxParts;
	else
		pTids = GetMyAuxPartTidList();

	Doc()->m_itemManager.SetRangeQuery983(pTids, pTids, m_userInfo[player].clubInfo.tid,
		m_userInfo[player].caddieInfo.tid, m_userInfo[player].mascotInfo.tid);
	Doc()->m_itemManager.CalcSetEffect983();
	return (float)Doc()->m_itemManager.GetSetRange983();
}

float CSharedDoc::GetAuxPartRange983(unsigned char player) // 006364d0
{
	if (m_pGolfDoc)
	{
		if (GetPlayerNum() < player)
			return 0.0f;
	}

	bool bInGame = true;
	unsigned long* pTids = NULL;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		pTids = m_userInfo[player].charInfo.tidAuxParts;
	}
	else
	{
		pTids = GetMyAuxPartTidList();
		bInGame = false;
	}

	float value = 0.0f;

	for (int i = 0; i < 5; i++)
	{
		if (pTids[i] == 0)
			continue;

		switch (i)
		{
		case 0:
			{
				IFF_STRUCT::sAuxPart* pAuxPart = Doc()->m_itemManager.FindAuxPart(pTids[0]);
				if (pAuxPart && pAuxPart->DriveUp)
					value += pAuxPart->DriveUp;
			}
			break;
		case 1:
			if (bInGame)
			{
				std::vector<sAuxpartValue>* pEffects =
					(std::vector<sAuxpartValue>*)m_auxPartValues983[player];
				if (pEffects)
				{
					sAuxpartValue& effect = pEffects->at(1);
					if (effect.m_pItem != NULL && effect.m_kind != -1 && effect.m_kind == 0)
						value += effect.m_value;
				}
			}
			else
			{
				IFF_STRUCT::sAuxPart* pAuxPart = Doc()->m_itemManager.FindAuxPart(pTids[1]);
				if (pAuxPart && pAuxPart->c.TypeId == 0x70210005)
				{
					value += pAuxPart->DriveUp;
					if (pAuxPart->tidSet983)
					{
						if ([&pTids](unsigned long tid) -> bool {
								for (int k = 0; k < 5; k++)
								{
									if (tid == pTids[k])
										return true;
								}
								return false;
							}(pAuxPart->tidSet983))
						{
							value += pAuxPart->SetDriveUp983;
						}
					}
				}
			}
			break;
		}
	}

	return value;
}

float CSharedDoc::GetMascotRange983(unsigned char player) // 006403f0
{
	if (m_pGolfDoc)
	{
		if (GetPlayerNum() < player)
			return 0.0f;
	}

	sMascotInfo* pInfo;
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
		pInfo = &Doc()->m_userInfo[player].mascotInfo;
	else
		pInfo = GetMyMascotInfo(Doc()->m_myInfo.userEquip.guidMascot);

	if (pInfo)
	{
		IFF_STRUCT::sMascot* pMascot = Doc()->m_itemManager.FindMascot(pInfo->tid);
		if (pMascot)
			return (float)(pMascot->DriveUp + 0.0);
	}
	return 0.0f;
}

// 983 00636040: so o slot 0 (item auxiliar) e o vetor do slot 1 contam; o laco do 645 somava os 5 slots
float CSharedDoc::GetAuxPartProperty(unsigned char player, unsigned char type)
{
	return ((CSharedDocJp*)this)->GetBonusRate983(player, type);
}

float CSharedDoc::GetWtPepPangyaComboGauge(unsigned char player)
{
	// A910: 983 0062fce0: efeito 16 do cartao do caddie; a tabela de 24 tids (16.0) e do 645
	CCardManager::Instance()->SetPlayerIndex(player);
	return CCardManager::Instance()->GetCaddieCard16_983();
}

bool CSharedDoc::FindEquipPart(unsigned char player, unsigned long typeId)
{
	bool bFind = false;

	if (Doc()->m_golfGame.gameType == GAME_TYPE_TEAM)
	{
		for (int i = 0; i < GetPlayerNum(); i++)
		{
			if (PLAYER(i)->team == player)
			{
				for (int j = 0; j < 24; j++)
				{
					if (typeId == Doc()->m_userInfo[i].charInfo.tidParts[j])
					{
						bFind = true;
						break;
					}
				}
				if (bFind)
					break;
			}
		}
	}
	else
	{
		for (int j = 0; j < 24; j++)
		{
			if (typeId == Doc()->m_userInfo[player].charInfo.tidParts[j])
			{
				bFind = true;
				break;
			}
		}
	}

	return bFind;
}

// 0062fd10 (983): o limite vem da habilidade 8; a lista de tids de pecas do 645 nao existe mais
float CSharedDoc::GetComboGaugeLimit(unsigned char team)
{
	_gateway::_controller::CControllerDoc* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xad, pDoc);
	CComboGaugeLimitIncrease* pLimit = (CComboGaugeLimitIncrease*)pDoc->GetObjectAt(team, eSpecialAbility_ComboGaugeLimitIncrease);
	return pLimit ? pLimit->GetValue(team) + 99.0f : 99.0f;
}

void CSharedDoc::AddChar(sCharacterInfo* pInfo)
{
	CPartTidList tidList;

	if (!tidList.SetTids(pInfo, 0xff) || !tidList.IsComboValid())
	{
		tidList.SetDefaultTids();
		memcpy(pInfo->tidParts, tidList.m_tid, sizeof(pInfo->tidParts));

		// 983 006427b1 (PKT C45): pecas trocadas pelas padrao -> devolve o personagem inteiro ao servidor (0x20 tipo 0)
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(0);
		packet.EncodeBuffer(pInfo, sizeof(sCharacterInfo)); // 0x201
		packet.Send(TO_GAME);
	}

	m_charMap.insert(std::map<unsigned int, sCharacterInfo>::value_type(
		pInfo->guid, *pInfo));
}

void CSharedDoc::AddCaddie(sCaddieInfo* pInfo)
{
	m_caddieMap.insert(
		std::map<unsigned int, sCaddieInfo>::value_type(pInfo->guid, *pInfo));
}

void CSharedDoc::AddMascot(sMascotInfo* pInfo)
{
	std::map<unsigned int, sMascotInfo>::iterator it =
		m_mascotMap.find(pInfo->guid);

	if (it != m_mascotMap.end())
	{
		(*it).second = *pInfo;
	}
	else
	{
		m_mascotMap.insert(std::map<unsigned int, sMascotInfo>::value_type(
			pInfo->guid, *pInfo));
	}
}

sCardStack* CSharedDoc::GetCardInfoToUID(unsigned long uid)
{
	for (std::list<sCardStack>::iterator it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		sCardStack* pCard = &(*it);
		if (uid == pCard->uid && pCard->bValid == 1)
		{
			return pCard;
		}
	}

	return NULL;
}

sCardStack* CSharedDoc::GetCardInfoToTID(unsigned long tid)
{
	for (std::list<sCardStack>::iterator it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		sCardStack* pCard = &(*it);
		if (tid == pCard->typeId && pCard->bValid == 1)
		{
			return pCard;
		}
	}

	return NULL;
}

void CSharedDoc::ReplaceCardCountToTID(unsigned long tid, int count)
{
	for (std::list<sCardStack>::iterator it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		if (tid == (*it).typeId && (*it).bValid == 1)
		{
			(*it).count = count;
			if (count <= 0)
				(*it).bValid = 0;
			return;
		}
	}
}

void CSharedDoc::ReplaceCardCountToUID(unsigned long uid, int count)
{
	for (std::list<sCardStack>::iterator it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		if (uid == (*it).uid && (*it).bValid == 1)
		{
			(*it).count = count;
			if (count <= 0)
				(*it).bValid = 0;
			return;
		}
	}
}

void CSharedDoc::DeleteCardToPartsAttach(unsigned long uid)
{
	std::list<sSCardAvilityPeriodInfo>::iterator it;
	for (it = Doc()->m_cardAbilityList[0].begin();
		it != Doc()->m_cardAbilityList[0].end(); it++)
	{
		if ((*it).partsUid == uid)
		{
			Doc()->m_cardAbilityList[0].erase(it);
			return;
		}
	}
}

void CSharedDoc::AddCardStack(sCardStack card, bool bReplace)
{
	std::list<sCardStack>::iterator it;
	for (it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		if (card.typeId == (*it).typeId && card.bValid == 1)
		{
			if (bReplace == true)
				(*it).count = card.count;
			else
				(*it).count += card.count;

			if ((*it).count <= 0)
				(*it).bValid = 0;
			return;
		}
	}

	if (it == Doc()->m_cardStackList.end())
	{
		Doc()->m_cardStackList.push_back(card);
	}
}

void CSharedDoc::DeleteCardStack(unsigned long uid, int count)
{
	for (std::list<sCardStack>::iterator it = Doc()->m_cardStackList.begin();
		it != Doc()->m_cardStackList.end(); ++it)
	{
		sCardStack* pCard = &(*it);
		if (uid == pCard->uid)
		{
			pCard->count -= count;
			if (pCard->count < 1)
			{
				pCard->count = 0;
				pCard->bValid = 0;
			}
		}
	}
}

void CSharedDoc::AddCard(unsigned long typeId, unsigned long uid,
	unsigned short count)
{
	std::list<sCards>::iterator it;
	for (it = Doc()->m_cardList.begin(); it != Doc()->m_cardList.end(); ++it)
	{
		if ((*it).tid == typeId)
		{
			(*it).cardCount += count;
			return;
		}
	}

	if (it == Doc()->m_cardList.end())
	{
		sCards card;
		memset(&card, 0, sizeof(card));
		card.tid = typeId;
		card.uid = uid;
		card.cardCount = count;
		Doc()->m_cardList.push_back(card);
	}
}

void CSharedDoc::AddFurniture(const sFurniture_List& furniture)
{
	m_furnitureList.push_back(furniture);
}

// 983 00641470 (B97/C40.1): contagem de missão do servidor (pacote 0x25c do OnPacketCommon)
void CSharedDoc::AddQuestCount983(const _quest::sQuestCount983& count)
{
	m_questCounts983.push_back(count);
}

// 983 00642940 (C42.2): missão recebida do servidor (pacote 0x21e do OnPacketCommon)
void CSharedDoc::AddQuest983(const _quest::CQuestItem& item)
{
	m_quests983.push_back(item);
}

// 983 0062bfa0 / 0062bfb0 (C42.1)
void CSharedDoc::SetPangyaBySet983(bool value)
{
	m_bPangyaBySet983 = value;
}

bool CSharedDoc::IsPangyaBySet983() const
{
	return m_bPangyaBySet983;
}

// 983 0046e6d0 (C42.3): fica no vão de funções pequenas do início do exe; aqui só para ter definição
bool __fastcall IsGrandPrixGameType983(int realGameType)
{
	return realGameType >= 0x14 && realGameType <= 0x15;
}

void CSharedDoc::AddMyPartsList(const sItemInfo& item)
{
	m_partsList.push_back(item);
}

sMascotInfo* CSharedDoc::GetMyMascotInfo(unsigned long uid)
{
	if (uid)
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(uid);

		if (it != Doc()->m_mascotMap.end())
		{
			return &(*it).second;
		}
	}

	return NULL;
}

sMascotInfo* CSharedDoc::GetMyEquippedMascotInfo()
{
	if (m_myInfo.userEquip.guidMascot)
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(m_myInfo.userEquip.guidMascot);

		if (it != Doc()->m_mascotMap.end())
		{
			return &(*it).second;
		}
	}

	return NULL;
}

// 983: 00632cc0 (so o placar do Grand Prix chama). Nome meu.
bool CSharedDoc::HasRival983(unsigned short count)
{
	if (Doc()->m_rivalList.size() >= count)
		return true;
	return false;
}

int CSharedDoc::GetNumAddItemSlotByMascot(unsigned char player)
{
	int num = 0;
	unsigned long tid = m_userInfo[player].mascotInfo.tid;
	IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);

	if (pMascot)
	{
		if (pMascot->ItemSlot <= 1)
			num = pMascot->ItemSlot;
	}

	return num;
}

int CSharedDoc::GetNumAddItemSlotByMascot()
{
	int num = 0;
	sMascotInfo* pInfo = GetMyEquippedMascotInfo();

	if (pInfo)
	{
		// 983 0063dcaf: o prazo so vale com Remain_Date; mascote sem prazo tambem da o slot
		if (!pInfo->Remain_Date ||
			ItemManager()->CompareSystemTime(pInfo->endDate,
				Doc()->GetServerTime()) > 0)
		{
			IFF_STRUCT::sMascot* pMascot =
				ItemManager()->FindMascot(pInfo->tid);

			if (pMascot)
			{
				if (pMascot->ItemSlot <= 1)
					num = pMascot->ItemSlot;
			}
		}
	}

	return num;
}

void CSharedDoc::SendEquipItemSlotByMascot()
{
	sMascotInfo* pInfo = GetMyEquippedMascotInfo();

	if (pInfo)
	{
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(pInfo->tid);

		if (pMascot && pMascot->ItemSlot > 0)
		{
			if (ItemManager()->CompareSystemTime(pInfo->endDate,
					Doc()->GetServerTime()) > 0)
				return;
		}
	}

	if (m_myInfo.userEquip.tidItemSlot[9])
	{
		unsigned long slots[10] = { 0 };
		memcpy(slots, Doc()->m_myInfo.userEquip.tidItemSlot, sizeof(slots));
		slots[9] = 0;

		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(2);
		packet.EncodeBuffer(slots, sizeof(slots));
		packet.Send(TO_GAME);
	}
}

void CSharedDoc::SendEquipItemSlot()
{
	sMascotInfo* pInfo = GetMyEquippedMascotInfo();
	CCardManager::Instance()->SetPlayerIndex(0xff);
	CCardManager::Instance()->CalcCardPeriodAndStatus();
	int cardSlot = CCardManager::Instance()->GetCardPeriodSlot();

	// D793: = 983 0063de40. Mascote com slot (+0x124) e cartas de prazo: TimeFlag (+0x82) 0 = nao manda; 1 = so com o prazo
	// valendo; outro valor manda. Senao zera no doc os slots a mais (mascote + cartas, do slot 9 para tras) e manda SEMPRE
	// (o 645 so mandava com o slot 8/9 ocupado e nao zerava).
	if (pInfo)
	{
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(pInfo->tid);

		if (pMascot && pMascot->ItemSlot)
		{
			if (pMascot->c.TimeFlag == 0 ||
				(pMascot->c.TimeFlag == 1 &&
					ItemManager()->CompareSystemTime(pInfo->endDate, Doc()->GetServerTime()) > 0))
			{
				if (cardSlot > 0)
					return;
			}
		}
	}

	int numExtra = GetNumAddItemSlotByMascot() + CCardManager::Instance()->GetCardPeriodSlot();
	for (unsigned long* p = &m_myInfo.userEquip.tidItemSlot[9]; numExtra > 0; --numExtra)
		*p-- = 0;

	unsigned long slots[10];
	memcpy(slots, m_myInfo.userEquip.tidItemSlot, sizeof(slots));

	WSendPacket packet((enumClientPacket)0x20);
	packet.Encode1(2);
	packet.EncodeBuffer(slots, sizeof(slots));
	packet.Send(TO_GAME);
}

void CSharedDoc::SetCurEquipInfo(sUserEquip* pEquip)
{
	m_myInfo.userEquip = *pEquip;
	BuildMyPartTidList();

	if (m_myInfo.userEquip.guidCaddie)
	{
		if (m_myInfo.userEquip.guidCaddie != 0xffffffff)
		{
			if (m_caddieMap.find(m_myInfo.userEquip.guidCaddie) ==
				m_caddieMap.end())
			{
				if (m_caddieMap.size() > 0)
				{
					m_myInfo.userEquip.guidCaddie =
						(*m_caddieMap.begin()).first;
				}
			}
		}
	}

	if (m_myInfo.userEquip.guidMascot)
	{
		if (m_myInfo.userEquip.guidMascot != 0xffffffff)
		{
			if (m_mascotMap.find(m_myInfo.userEquip.guidMascot) ==
				m_mascotMap.end())
			{
				m_myInfo.userEquip.guidMascot = 0;
			}
		}
	}
}

void CSharedDoc::CheckCurrEquipParts()
{
	CPartTidList tidList;
	CPartList partList;
	bool bSendChar = false;
	std::map<unsigned int, sCharacterInfo>::iterator it;

	for (it = m_charMap.begin(); it != m_charMap.end(); ++it)
	{
		bool bChanged = false;
		sCharacterInfo* pInfo = &it->second;

		if (!tidList.SetTids(pInfo, m_myInfo.stat.Level))
		{
			tidList.SetDefaultTids();
			memcpy(pInfo->tidParts, tidList.m_tid, sizeof(pInfo->tidParts));
		}
		else
		{
			partList.Build(&tidList);

			for (int i = 0; i < 24; i++)
			{
				unsigned long tid = pInfo->tidParts[i];
				if (!tid || (tid & 0x600))
					continue;

				std::list<sItemInfo>::iterator itPart;
				for (itPart = m_partsList.begin(); itPart != m_partsList.end();
					itPart++)
				{
					if ((*itPart).tid == tid)
						break;
				}

				if (itPart == m_partsList.end())
				{
					bChanged = true;
					partList.SetPart(&tidList, tid, 0);
				}
			}

			if (!bChanged)
				continue;

			memcpy(pInfo->tidParts, tidList.m_tid, sizeof(pInfo->tidParts));
		}

		if (pInfo->guid == m_myInfo.userEquip.guidChar)
			bSendChar = true;

		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(0);
		packet.EncodeBuffer(pInfo, sizeof(sCharacterInfo));
		packet.Send(TO_GAME);
	}

	if (bSendChar)
	{
		WSendPacket packet((enumClientPacket)0x20);
		packet.Encode1(5);
		packet.Encode4(m_myInfo.userEquip.guidChar);
		jp983_rastro("A803 doc envia 0x20/5 guidChar=%08x", m_myInfo.userEquip.guidChar);
		packet.Send(TO_GAME);
	}
}

bool CSharedDoc::BuildMyPartTidListDefault(unsigned long typeId)
{
	IFF_STRUCT::sChar* pChar = m_itemManager.FindChar(typeId);
	if (pChar)
	{
		m_partTidList.m_charTid = typeId;
		m_partTidList.m_defPartNum = pChar->nParts;
		m_partTidList.m_partNum = pChar->nParts + pChar->nAcsries;
		m_partTidList.m_hairColor = 0;
		m_partTidList.m_shirtsColor = 0;
		m_partTidList.SetDefaultTids();

		memset(m_auxPartTidList, 0, sizeof(m_auxPartTidList));
		return true;
	}

	return false;
}

void CSharedDoc::BuildMyPartTidList()
{
	sCharacterInfo* pInfo;
	sCharacterInfo defInfo;
	std::map<unsigned int, sCharacterInfo>::iterator it;
	it = m_charMap.find(m_myInfo.userEquip.guidChar);

	if (it == m_charMap.end())
	{
		memset(&defInfo, 0, sizeof(defInfo));
		defInfo.tid = 0x4000000 | (Doc()->m_myInfo.info.gender % 2);
		ItemManager()->GetDefCombo(defInfo.tid, defInfo.tidParts);
		pInfo = &defInfo;
	}
	else
		pInfo = &it->second;

	if (!m_partTidList.SetTids(pInfo, Doc()->m_myInfo.stat.Level))
	{
		m_partTidList.SetDefaultTids();
	}

	if (!m_partTidList.IsComboValid())
	{
		m_partTidList.SetDefaultTids();
	}

	memcpy(m_auxPartTidList, pInfo->tidAuxParts, sizeof(m_auxPartTidList));
}

void CSharedDoc::BuildMyPartTidList(sCharacterInfo& info)
{
	if (!m_partTidList.SetTids(&info, Doc()->m_myInfo.stat.Level))
	{
		m_partTidList.SetDefaultTids();
	}

	if (!m_partTidList.IsComboValid())
	{
		m_partTidList.SetDefaultTids();
	}

	memcpy(m_auxPartTidList, info.tidAuxParts, sizeof(m_auxPartTidList));
}

void CSharedDoc::BuildTikiReportList()
{
	if (IsLocalContent(S3_CADDIE_REPORT) && m_caddieReportList.size())
	{
		Doc()->m_rivalList.clear();
		Doc()->m_roomInfo.nUserLimit = 30;
		// 983 00644696: zera os 6 dwords de +0x106e4 (placar de rodada e pang das equipes)
		m_guildRoundScore[0] = 0;
		m_guildRoundScore[1] = 0;
		m_unused47bc = 0;
		m_teamPang[0] = 0;
		m_teamPang[1] = 0;
		m_unused47c8 = 0;

		std::vector<sCaddieReportData>::iterator it;
		for (it = m_caddieReportList.begin(); it != m_caddieReportList.end();
			it++)
		{
			sRivalData rival;
			memset(&rival, 0, sizeof(rival));
			rival.uid = it->uid;
			rival.oid = it->uid;
			strcpy(rival.nickname, it->nickname);
			rival.level = it->level;
			rival.totalScore = it->score;
			rival.totalPang = it->pang;
			rival.totalBonusPang = it->bonusPang;
			rival.guildUID = it->guildUID;
			rival.guildEmblemVer983 = it->guildEmblemVer983; // 983: [esi+0x5d] -> +0x178 (C55.1)
			strcpy(rival.guildMark, it->guildMark);
			rival.mascotTypeId = it->mascotTypeId;
			rival.holeStroke[0] = it->eventType;
			unsigned char team = it->team;
			if (team & 4)
			{
				team ^= 4;
				rival.state = 3;
			}
			rival.team = team;

			if (strcmp(rival.nickname, MyNick()) == 0)
			{
				Doc()->m_eventType = it->eventType;
				Doc()->m_holeStatistics[0].dwExp = it->exp;
				Doc()->m_roomType = it->roomType;
				Doc()->m_golfGame.gameType = it->gameType;
				Doc()->m_eventFlag = it->eventFlag;
			}

			rival.capability = it->premium ? 0x4000 : 0; // 983: 0x4000 (645: 0x80)
			rival.pangMastery = 0;
			rival.pangNitro = 0;

			if (it->awardFlag & 1)
				m_awardItem[0].uid = rival.uid;
			if (it->awardFlag & 2)
				m_awardItem[1].uid = rival.uid;
			if (it->awardFlag & 4)
				m_awardItem[2].uid = rival.uid;
			if (it->awardFlag & 8)
				m_awardItem[3].uid = rival.uid;
			if (it->awardFlag & 0x10)
				m_awardItem[4].uid = rival.uid;
			if (it->awardFlag & 0x20)
				m_awardItem[5].uid = rival.uid;
			// 983 00644870-006448d6: no meu registro, conta os premios que ficaram com o amigo selecionado (+0x10bb1) em 6
			// contadores do meu sUserStat (Doc+0x9b6 = stat+0xe7, passo 4)
			if (m_myInfo.info.dwUID == rival.oid)
			{
				// D879: era (char*)this + 0x9b6 cru; D939: campo nomeado em sPangYaUserStatistics
				C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo) + offsetof(sMyInfo, stat) + offsetof(sPangYaUserStatistics, awardCountFriend983) == 0x9b6);
				for (int i = 0; i < 6; i++)
				{
					if (m_awardItem[i].uid == m_selFriend.Uid)
						m_myInfo.stat.awardCountFriend983[i] += 1;
				}
			}

			switch (it->pangItem)
			{
			case 1:
				rival.pangMastery = 1;
				break;
			case 2:
				rival.pangNitro = 1;
				break;
			case 3:
				rival.pangNitro = 1;
				rival.pangMastery = 1;
				break;
			}

			m_rivalList.push_back(rival);
		}
	}
}

CPartTidList& CSharedDoc::GetMyPartTidList()
{
	return m_partTidList;
}

unsigned long* CSharedDoc::GetMyAuxPartTidList()
{
	return m_auxPartTidList;
}

void CSharedDoc::SetIndex(unsigned long uid)
{
	if (m_indexMap.find(uid) != m_indexMap.end())
		return;

	unsigned char index = (unsigned char)m_indexMap.size();

	if (OnlinePlay())
	{
		if (IsMassGame())
		{
			if (uid == m_myInfo.info.dwGuid)
				m_userInfo[0].info = m_myInfo.info;
		}
	}

	m_indexMap[uid] = index;
}

bool CompareItemInfo(const sItemInfo& a, const sItemInfo& b)
{
	return a.tid < b.tid;
}

void CSharedDoc::SortAllMyItemList()
{
	m_partsList.sort(CompareItemInfo);
	m_ballList.sort(CompareItemInfo);
	m_myItemList.sort(CompareItemInfo);
	m_cadItemList.sort(CompareItemInfo);
	m_setItemList.sort(CompareItemInfo);
}

bool CSharedDoc::ConfirmInsertItem(unsigned long typeId,
	unsigned long count) const
{
	unsigned long category = typeId >> 26;
	if (category >= 5 && category <= 6)
	{
		if (typeId != 0x1a000010)
		{
			for (std::list<sItemInfo>::const_iterator it = m_myItemList.begin();
				it != m_myItemList.end(); ++it)
			{
				if (sItemInfo(*it).tid == typeId)
				{
					if (sItemInfo(*it).Common[0] + count > 20000)
						return false;
					return true;
				}
			}
			return true;
		}
	}

	return true;
}

void CSharedDoc::AddMyItem(const sItemInfo& item)
{
	Doc()->m_myItemList.push_back(item);
}

short CSharedDoc::DecreaseMyItem(std::list<sItemInfo>::iterator& it)
{
	// JP 983 (006407d0): apaga so quando passa de 0 para baixo e devolve a quantidade (-1 quando apagou)
	sItemInfo* pItem = &(*it);
	pItem->Common[0]--;
	short count = pItem->Common[0];
	if (pItem->Common[0] < 0)
	{
		DeleteMyItem(it);
		return -1;
	}
	if (count == -1)
		DeleteMyItem(it);
	return count;
}

bool CSharedDoc::DeleteMyItem(unsigned long typeId)
{
	std::list<sItemInfo>::iterator it;
	for (it = m_myItemList.begin(); it != m_myItemList.end(); it++)
	{
		if ((*it).tid == typeId)
			break;
	}

	return DeleteMyItem(it);
}

bool CSharedDoc::DeleteMyItem(std::list<sItemInfo>::iterator& it)
{
	if (it == m_myItemList.end())
		return false;

	m_myItemList.erase(it);
	return true;
}

sItemInfo* CSharedDoc::FindMyItemByGuid(unsigned long guid)
{
	std::list<sItemInfo>::iterator it;
	for (it = m_myItemList.begin(); it != m_myItemList.end(); it++)
	{
		if ((*it).guid == guid)
			return &(*it);
	}

	return NULL;
}

// 983: 0063b570 - irmã do FindMyItemByGuid, pelo typeid
sItemInfo* CSharedDoc::FindMyItemByTypeId983(unsigned long typeId)
{
	std::list<sItemInfo>::iterator it;
	for (it = m_myItemList.begin(); it != m_myItemList.end(); it++)
	{
		if ((*it).typeId == typeId)
			return &(*it);
	}

	return NULL;
}

void CSharedDoc::ClearMyGuildInfo()
{
	m_myInfo.info.dwGuildId = 0;
	m_myInfo.info.dwEmblemVer = 0;
	memset(m_myInfo.info.sGuild, 0, sizeof(m_myInfo.info.sGuild));
	memset(m_myInfo.info.szEmblemName, 0, sizeof(m_myInfo.info.szEmblemName));
}

void CSharedDoc::SetEquipCharCaddieInfoFromMyInfo()
{
	std::map<unsigned int, sCharacterInfo>::iterator itChar;
	std::map<unsigned int, sCaddieInfo>::iterator itCaddie;
	std::map<unsigned int, sMascotInfo>::iterator itMascot;
	std::map<unsigned int, sItemInfo>::iterator itClub;

	memset(&m_userInfo[0].charInfo, 0, sizeof(sCharacterInfo));
	memset(&m_userInfo[0].caddieInfo, 0, sizeof(sCaddieInfo));
	memset(&m_userInfo[0].clubInfo, 0, sizeof(sClubInfo));
	memset(&m_userInfo[0].mascotInfo, 0, sizeof(sMascotInfo));

	m_userInfo[0].info = m_myInfo.info;
	m_userInfo[0].stat = m_myInfo.stat;
	m_userInfo[0].userEquip = m_myInfo.userEquip;

	itChar = m_charMap.find(m_myInfo.userEquip.guidChar);
	if (itChar != m_charMap.end())
		m_userInfo[0].charInfo = (*itChar).second;

	itCaddie = m_caddieMap.find(m_myInfo.userEquip.guidCaddie);
	if (itCaddie != m_caddieMap.end())
	{
		m_userInfo[0].caddieInfo = (*itCaddie).second;
		IFF_STRUCT::sCaddie* pCaddie =
			ItemManager()->FindCaddie(m_userInfo[0].caddieInfo.tid);
		if ((pCaddie && m_myInfo.stat.Level < pCaddie->c.Level) ||
			(Doc()->m_userInfo[0].caddieInfo.Rent_flag &&
				Doc()->m_userInfo[0].caddieInfo.Remain_Date == 0 &&
				pCaddie->MonthlyFee > 0))
			m_userInfo[0].caddieInfo.tid = 0;
	}

	itMascot = m_mascotMap.find(m_myInfo.userEquip.guidMascot);
	if (itMascot != m_mascotMap.end())
	{
		m_userInfo[0].mascotInfo = (*itMascot).second;
		IFF_STRUCT::sMascot* pMascot =
			ItemManager()->FindMascot(m_userInfo[0].mascotInfo.tid);
	}

	itClub = m_clubSetMap.find(m_myInfo.userEquip.guidClubSet);
	if (itClub != m_clubSetMap.end())
	{
		IFF_STRUCT::sClubSet* pClubSet =
			ItemManager()->FindClubSet((*itClub).second.tid);
		if (pClubSet &&
			((m_myInfo.stat.Level < pClubSet->c.Level &&
				 !pClubSet->c.IsUnderLvl) ||
				(m_myInfo.stat.Level > pClubSet->c.Level &&
					pClubSet->c.IsUnderLvl)))
		{
			std::map<unsigned int, sItemInfo>::iterator it;
			for (it = m_clubSetMap.begin(); it != m_clubSetMap.end(); ++it)
			{
				if ((*it).second.tid == 0x10000000)
				{
					itClub = it;
					break;
				}
			}
		}
		else
			itClub = m_clubSetMap.find(m_myInfo.userEquip.guidClubSet);

		if (itClub != m_clubSetMap.end())
		{
			m_userInfo[0].clubInfo.tid = (*itClub).second.tid;
			m_userInfo[0].clubInfo.guid = (*itClub).second.guid;
			memcpy(m_userInfo[0].clubInfo.PCL, (*itClub).second.Common,
				sizeof(m_userInfo[0].clubInfo.PCL));
			// 983 0063e120: slots do taco (sItemInfo+0xaa) -> clubInfo+0x12
			memcpy(m_userInfo[0].clubInfo.slotUp983, (*itClub).second.slotUp983,
				sizeof(m_userInfo[0].clubInfo.slotUp983));
		}
	}
}

sRivalData* CSharedDoc::GetRival(unsigned long uid)
{
	std::vector<sGuildMatchup>::iterator it;
	for (it = m_guildMatchupList.begin(); it != m_guildMatchupList.end(); ++it)
	{
		if (uid == it->uid[0])
			return &m_rivalList[Doc()->GetIndex(it->uid[1])];

		if (uid == it->uid[1])
			return &m_rivalList[Doc()->GetIndex(it->uid[0])];
	}

	return NULL;
}

int CSharedDoc::CollapseItemSlot(unsigned char player)
{
	if (player >= 4)
		return 0;

	unsigned long* slots = m_userInfo[player].userEquip.tidItemSlot;
	int numSlots = 10;
	if (IsLocalContent(S3_MASCOT))
		numSlots = GetNumAddItemSlotByMascot(player) + 8;

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(player);
		numSlots += CCardManager::Instance()->GetCardPeriodSlot();
	}

	int count = 0;
	for (int i = 0; i < numSlots; i++)
	{
		if (slots[i])
			count++;
	}

	for (int j = 0; j < count; j++)
	{
		if (slots[j] == 0)
		{
			int k;
			for (k = j + 1; k < numSlots; k++)
			{
				if (slots[k])
				{
					slots[j] = slots[k];
					slots[k] = 0;
					break;
				}
			}
			if (k == numSlots)
				break;
		}
	}

	return count;
}

const char* CSharedDoc::GetDefaultTitle(unsigned char gameType)
{
	const char* title;

	switch (gameType)
	{
	case GAME_TYPE_STROKE:
	case GAME_TYPE_TEAM:
		title = defaultTitles_vs[rand() % 9];
		break;
	case GAME_TYPE_30S:
	case GAME_TYPE_30S_TEAM:
	case GAME_TYPE_GUILD_MATCH:
	case GAME_TYPE_USEMAX:
		title = defaultTitles_mass[rand() % 6];
		break;
	case GAME_TYPE_SKINS:
		title = defaultTitles_battle[rand() % 4];
		break;
	case GAME_TYPE_NEW_APPROACH:
		title = defaultTitles_approach[rand() % 3];
		break;
	case GAME_TYPE_AVATARCHAT:
	case GAME_TYPE_REALMYROOM:
		title = defaultTitles_chat[rand() % 7];
		break;
	case GAME_TYPE_HOLEINONE983: // 983: titulos fixos dos eventos de hole in one (00e78d74/00e78d78)
	case GAME_TYPE_UNK983_13:
		title = defaultTitles_event[gameType == GAME_TYPE_UNK983_13];
		break;
	default:
		title = "No Title!";
		break;
	}

	return title;
}

void CSharedDoc::Log30sScore()
{
	if (IsMassGame() || IsGuildGame())
	{
		mkdir("score");

		char filename[128];
		SYSTEMTIME st;
		GetLocalTime(&st);

		sprintf(filename, "score/score_30s_%d%02d%02d_%02d%02d%02d.txt",
			st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		FILE* fp = fopen(filename, "wt");
		if (fp)
		{
			fprintf(fp, "## 30s Game Result ##\n\n");
			fprintf(fp, "* room name : %s\n", Doc()->m_roomInfo.title);
			fprintf(fp, "* map no. : %d\n",
				(unsigned char)(Doc()->m_golfGame.map > 127 &&
							Doc()->m_golfGame.map != 253
						? Doc()->m_golfGame.map - 128
						: Doc()->m_golfGame.map));
			fprintf(fp, "* grade : %d\n",
				(unsigned char)(Doc()->m_roomType >> 16));
			fprintf(fp, "* game finished at : %d-%02d-%02d %02d:%02d\n",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
			fprintf(fp, "* num of players : %d\n\n", m_rivalList.size());

			fprintf(fp,
				"nick / id                                rank  score  pang \n");
			fprintf(fp,
				"-----------------------------------------------------------\n");

			for (int i = 0; i < (int)m_rivalList.size(); i++)
			{
				sRivalData* pRival = &m_rivalList[i];

				if (pRival)
				{
					fprintf(fp, "%18s   %2d    ", pRival->nickname,
						pRival->rank);

					if (pRival->totalScore == 0)
					{
						fprintf(fp, "  0   %I64d\n", pRival->totalPang);
						continue;
					}

					fprintf(fp, "%+2d   %I64d\n", pRival->totalScore,
						pRival->totalPang);
				}
			}

			fclose(fp);
		}
	}
}

void CSharedDoc::LogGstUpdateHole(unsigned long uid, unsigned char hole,
	int score, __int64 pang)
{
}

void CSharedDoc::LoadSchoolName(char* const name, unsigned long id)
{
	char line[8192];
	char code[64];
	cFile* file = g_resrcmng->GetCFile("school.txt", 0xffff);

	if (!file)
	{
		name[0] = 0;
		return;
	}

	while (file->Scan("%n", line))
	{
		int length = strlen(line);
		cTokenV token;
		token.Init(line, length);

		token.GetToken(code, " ", 1);
		token.GetToken(name, "(\r\n", 3);

		if (id == atoi(code))
		{
			CloseCFile(file);
			return;
		}
	}

	CloseCFile(file);
	name[0] = 0;
}

bool CSharedDoc::InitRecentWhisperList(FrComboBox* combo)
{
	m_recentWhisperList.clear();
	return UpdateChatTarget(combo);
}

bool CSharedDoc::InitReservedChatList(FrComboBox* combo,
	eReservedChatPartner partner)
{
	if (!combo)
		return false;

	m_reservedChatList.clear(); // 983 00645bc0: os textos passam pelo K2L_Compatibility
	if (m_whisperPartner.size() > 0)
		combo->SetLine(1, m_whisperPartner.c_str(), 0, false, 0);

	if (partner & 1)
	{
		m_reservedChatList.push_back(K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4"));
	}
	else if (strcmpi(m_whisperPartner.c_str(), K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4")) == 0)
	{
		m_whisperPartner = K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4");
		combo->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, false, 0);
	}

	if (partner & 2)
	{
		m_reservedChatList.push_back(K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"));
	}

	if (Doc()->m_myInfo.info.dwGuildId)
	{
		m_reservedChatList.push_back(K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1\xb0\xd4"));
	}

	if (strlen(combo->GetLine(1, false)) == 0)
		combo->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, false, 0);

	return UpdateChatTarget(combo);
}

bool CSharedDoc::AddWhisperPartner(FrComboBox* combo, const char* name)
{
	if (!combo)
		return false;

	bool bSet = false;
	const char* partner = NULL;

	if (name)
	{
		partner = name;
		bSet = true;
	}
	else if (strlen(combo->GetLine(1, false)) &&
		strcmp(MyNick(), combo->GetLine(1, false)))
	{
		partner = combo->GetLine(1, false);
	}

	if (partner)
	{
		std::list<std::string>::iterator it;

		it = std::find(m_reservedChatList.begin(), m_reservedChatList.end(),
			partner);
		if (it != m_reservedChatList.end())
		{
			if (!bSet)
				m_whisperPartner = partner;
			return true;
		}

		if (strcmpi(K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4"), partner) == 0)
		{
			return false;
		}

		it = std::find(m_recentWhisperList.begin(), m_recentWhisperList.end(),
			partner);
		if (it != m_recentWhisperList.end())
		{
			if (*it == m_recentWhisperList.back())
				return true;

			if (!bSet)
				m_whisperPartner = partner;
			m_recentWhisperList.push_back(partner);
			m_recentWhisperList.erase(it);

			return UpdateChatTarget(combo);
		}

		if (m_recentWhisperList.size() == m_maxWhisperPartner)
		{
			if (!bSet)
				m_whisperPartner = partner;
			m_recentWhisperList.push_back(partner);
			m_recentWhisperList.erase(m_recentWhisperList.begin());
			return UpdateChatTarget(combo);
		}

		if (!bSet)
			m_whisperPartner = partner;
		m_recentWhisperList.push_back(partner);
		return UpdateChatTarget(combo);
	}

	return false;
}

bool CSharedDoc::UpdateChatTarget(FrComboBox* combo)
{
	if (!combo)
		return false;

	std::list<std::string>::iterator it;
	combo->ClearList();

	for (it = m_recentWhisperList.begin(); it != m_recentWhisperList.end();
		++it)
	{
		combo->AddString((*it).c_str());
	}
	for (it = m_reservedChatList.begin(); it != m_reservedChatList.end(); ++it)
	{
		combo->AddString((*it).c_str());
	}

	return true;
}

bool CSharedDoc::UTIL_SendChatMessage(FrComboBox* combo, const char* msg,
	bool bWhisper)
{
	if (!combo)
	{
		return false;
	}

	if (strlen(combo->GetLine(1, false)) == 0)
		combo->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, false, 0);

	if (!AddWhisperPartner(combo, NULL))
		return false;

	const char* target = NULL;
	bool bTeam = false;

	if (strcmpi(combo->GetLine(1, false), K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4")) == 0)
	{
		bTeam = true;
	}
	else if (strcmpi(combo->GetLine(1, false),
				 K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4")) &&
		strcmpi(combo->GetLine(1, false), K2L_Compatibility("\xc6\xc0\xbf\xa1\xb0\xd4")))
	{
		target = combo->GetLine(1, false);
	}

	if (bWhisper)
		m_chatManager.SendChatMessage2(msg, target, bTeam);
	else
		m_chatManager.SendChatMessage(msg, target, bTeam);

	return true;
}

const char* CSharedDoc::GetPrevChatTarget(const char* name)
{
	std::list<std::string>::iterator it =
		std::find(m_reservedChatList.begin(), m_reservedChatList.end(), name);
	if (it != m_reservedChatList.end())
	{
		if (it != m_reservedChatList.begin())
		{
			--it;
			return (*it).c_str();
		}
		if (m_recentWhisperList.size() > 0)
			return m_recentWhisperList.back().c_str();
	}
	else if (m_recentWhisperList.size() > 0)
	{
		it = std::find(m_recentWhisperList.begin(), m_recentWhisperList.end(),
			name);
		if (it != m_recentWhisperList.end())
		{
			if (it != m_recentWhisperList.begin())
			{
				--it;
				return (*it).c_str();
			}
			return m_recentWhisperList.front().c_str();
		}
	}
	return m_reservedChatList.back().c_str();
}

const char* CSharedDoc::GetNextChatTarget(const char* name)
{
	std::list<std::string>::iterator it =
		std::find(m_recentWhisperList.begin(), m_recentWhisperList.end(), name);
	if (it != m_recentWhisperList.end())
	{
		if (++it != m_recentWhisperList.end())
		{
			return (*it).c_str();
		}
		else if (m_reservedChatList.size() > 0)
		{
			return m_reservedChatList.front().c_str();
		}
	}
	else if (m_reservedChatList.size() > 0)
	{
		it = std::find(m_reservedChatList.begin(), m_reservedChatList.end(),
			name);
		if (it != m_reservedChatList.end())
		{
			if (++it != m_reservedChatList.end())
			{
				return (*it).c_str();
			}
		}
	}

	return "";
}

void CSharedDoc::CheckAngelWing()
{
	HaveAngelWing(false);

	std::list<sItemInfo>::iterator it;
	for (it = m_partsList.begin(); it != m_partsList.end(); it++)
	{
		if (IsAngelWing((*it).tid))
		{
			HaveAngelWing(true);
			return;
		}
	}
}

void CSharedDoc::SetInitHaveHalloweenItem()
{
	SetHaveHalloweenItem(false);

	std::list<sItemInfo>::iterator it;
	for (it = m_partsList.begin(); it != m_partsList.end(); it++)
	{
		if (((*it).tid & 0x1800) == 0x1800)
		{
			SetHaveHalloweenItem(true);
			return;
		}
	}
}

void CSharedDoc::SetInitWearHalloweenItem()
{
	SetWearHalloweenItem(false);

	std::map<unsigned int, sCharacterInfo>::iterator it;
	for (it = m_charMap.begin(); it != m_charMap.end(); ++it)
	{
		sCharacterInfo* pInfo = &(*it).second;

		if (pInfo && pInfo->guid == m_myInfo.userEquip.guidChar)
		{
			for (int i = 0; i < 24; i++)
			{
				if ((pInfo->tidParts[i] & 0x1800) == 0x1800)
				{
					SetWearHalloweenItem(true);
					return;
				}
			}
			return;
		}
	}
}

__int64 CSharedDoc::IsControlServerService(__int64 service)
{
	return m_controlServerService & service;
}
void CSharedDoc::InitTradeData()
{
	m_stateTrade = STATE_TRADE_INIT;
	m_listTradeItem.clear();

	m_tradeUID = 0xffffffff;
	m_tradeTitle = "";
	m_countVisitor = 0;
	m_tradeIncome = 0;

	m_tradeMode = 0;
}

void CSharedDoc::SetTradeUID(unsigned long uid)
{
	if (uid != 0xffffffff)
		m_tradeUID = uid;
}

bool CSharedDoc::AddListTradeItem(sTradeItem item)
{
	if (m_listTradeItem.size() >= 6)
		return false;
	if (item.dwGuid == 0)
		return false;
	if (item.dwTid == 0)
		return false;
	if (item.iNum <= 0)
		return false;
	if (item.i64Price <= 0)
		return false;

	std::list<sTradeItem>::iterator it;
	for (it = m_listTradeItem.begin(); it != m_listTradeItem.end(); it++)
	{
		if ((*it).dwGuid == item.dwGuid)
		{
			unsigned long type = item.dwTid >> 26;
			if (type < 5 || (type > 6 && type != 31))
				return false;
		}
	}

	m_listTradeItem.push_back(item);
	if (IsLocalContent(S4_UCC))
	{
		if (strlen(item.UccIndex) > 0 && item.Seq > 0 && (item.status & 1) &&
			IsUccClothes(item.dwTid))
		{
			sUccClothes clothes;
			clothes.id = item.dwGuid;
			clothes.typeId = item.dwTid;
			strncpy(clothes.uccIndex, item.UccIndex, 9);
			strncpy(clothes.uccName,
				K2L_Compatibility("\xc1\xa4\xba\xb8\xb8\xa6 \xb9\xde\xbe\xc6\xbf\xc0\xb4\xc2 "
				"\xc1\xdf..."),
				41);
			clothes.downloadCount = 0;
			UccManager()->AddClothes(0, clothes, true, false);
		}
	}

	return true;
}

bool CSharedDoc::MinusListTradeItem(sTradeItem item)
{
	std::list<sTradeItem>::iterator it;
	for (it = m_listTradeItem.begin(); it != m_listTradeItem.end(); it++)
	{
		if ((*it).iIndex == item.iIndex)
		{
			if ((*it).dwGuid == item.dwGuid && (*it).dwTid == item.dwTid)
			{
				if ((*it).iNum > item.iNum)
				{
					(*it).iNum -= item.iNum;
					return true;
				}
				else if ((*it).iNum == item.iNum)
				{
					m_listTradeItem.erase(it);
					return true;
				}
			}
			break;
		}
	}

	return false;
}

void CSharedDoc::ChageListStallitem(sStall* stall)
{
	std::list<sStall>::iterator it;
	std::list<sStall>* pList = Doc()->GetListStallItem();
	if (stall == NULL)
	{
		sStall* pStall = GetStallItem();
		for (it = pList->begin(); it != pList->end(); it++)
		{
			if (pStall->stallKey == (*it).stallKey)
				*it = *pStall;
		}
	}
	else
	{
		for (it = pList->begin(); it != pList->end(); it++)
		{
			if (stall->stallKey == (*it).stallKey)
				*it = *stall;
		}
	}
}

bool CSharedDoc::SendOfflineTradeEditShop()
{
	if (!IsLocalContent(S4_OFFLINE_SHOP))
		return true;

	if (Doc()->GetStallItem() == NULL || Doc()->GetStallItem()->stallKey == 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)"\xc1\xf8\xbf\xad\xc0\xe5 \xb1\xb8\xb8\xc5\xb8\xa6 "
				 "\xc7\xcf\xc1\xf6\xbe\xca\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf"
				 "\xb4\xd9. \xb1\xb8\xb8\xc5\xc7\xcf\xb0\xed \xc0\xcc\xbf\xeb"
				 "\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4.~",
			0, 0, 0, 0));
		return true;
	}

	Doc()->SetTradeMode(5);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x232), 0, 0, 0, 0, 0));

	return true;
}

bool CSharedDoc::SendOfflineTradeVisitor(unsigned long uid)
{
	if (!IsLocalContent(S4_OFFLINE_SHOP))
		return true;

	if (Doc()->GetStallItem() == NULL || Doc()->GetStallItem()->stallKey == 0)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)"\xb9\xe6\xb9\xae\xc7\xcf\xbd\xc5 \xb9\xe6\xc0\xc7 \xc0\xaf\xc0\xfa"
				 "\xb4\xc2 \xc1\xc2\xc6\xc7\xc0\xbb \xb9\xe8\xc4\xa1\xc7\xcf\xb0\xc5"
				 "\xb3\xaa, \xb1\xb8\xc0\xd4\xc7\xcf\xc1\xf6 \xbe\xca\xc0\xb8\xbc\xcc"
				 "\xbd\xc0\xb4\xcf\xb4\xd9.",
			0, 0, 0, 0));
		return true;
	}

	std::list<sStall>* pList = Doc()->GetListStallItem();
	sStall stall;
	std::list<sStall>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{
		if ((*it).typeID == uid)
		{
			WSendPacketSem983 packet((enumClientPacket)0xcd); // JP 983 (s63): id do KR 645 sem id do 983; nao envia
			packet.Encode4(Doc()->m_replayTargetUID);
			packet.Encode4((*it).stallKey);
			packet.Encode1(1);
			packet.Send(TO_GAME);

			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
			break;
		}
	}

	return true;
}

bool CSharedDoc::SendTradeOpenShop()
{
	if (GetStateTrade() != STATE_TRADE_EDIT)
		return false;
	if (GetListTradeItem()->size() <= 0)
		return false;

	WSendPacket packet((enumClientPacket)0x74);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeCloseShop()
{
	if (GetStateTrade() == STATE_TRADE_INIT)
		return false;

	WSendPacket packet((enumClientPacket)0x75);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeEditShop()
{
	WSendPacket packet((enumClientPacket)0x76);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeEnterShop(unsigned long uid)
{
	if (uid == 0xffffffff)
		return false;

	WSendPacket packet((enumClientPacket)0x77);
	packet.Encode4(uid);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeExitShop(unsigned long uid)
{
	if (uid == 0xffffffff)
		return false;

	WSendPacket packet((enumClientPacket)0x78);
	packet.Encode4(uid);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeTitle(std::string title)
{
	if (title.length() <= 0)
		return false;

	WSendPacket packet((enumClientPacket)0x79);
	packet.EncodeStr(title);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeShowVisitor()
{
	switch (m_stateTrade)
	{
	case STATE_TRADE_OPEN:
	case STATE_TRADE_OUTOFSTOCK:
		break;
	default:
		return false;
	}

	WSendPacket packet((enumClientPacket)0x7a);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeIncome()
{
	switch (m_stateTrade)
	{
	case STATE_TRADE_OPEN:
	case STATE_TRADE_OUTOFSTOCK:
		break;
	default:
		return false;
	}

	WSendPacket packet((enumClientPacket)0x7b);
	packet.Send(TO_GAME);

	return true;
}

bool CSharedDoc::SendTradeBuyItem(unsigned long uid, sTradeItem item)
{
	if (m_stateTrade != STATE_TRADE_ENTER)
		return false;
	if (uid == 0 || uid == 0xffffffff)
		return false;
	if (item.dwGuid == 0)
		return false;
	if (item.dwTid == 0)
		return false;
	if (item.i64Price == 0)
		return false;
	if (item.iNum <= 0)
		return false;

	WSendPacket packet((enumClientPacket)0x7d);
	packet.Encode4(uid);
	packet.EncodeBuffer(&item, sizeof(sTradeItem));
	packet.Send(TO_GAME);

	return true;
}

void CSharedDoc::SetInitAbilityItem()
{
	m_flagAbilityItem = 0;
	m_numBonusPangItem = 0;

	for (int i = 0; i < 24; i++)
	{
		unsigned long flag = ItemManager()->GetIndexAbilityItem(
			m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tidParts[i]);
		m_flagAbilityItem |= flag;

		if (flag & 1)
		{
			m_numBonusPangItem++;
		}
	}
}

// JP 983 (00636930): o numero do "[text_pangya%02d.jpg" vem do ClubSet.iff (+0x100); a tabela por taco do 645 saiu.
// Tacos 0x3f/0x40 so mostram o proprio texto com o personagem 9; com os outros, o 00.
int CSharedDoc::GetIndexPangyaLogoImage()
{
	const sUserInfo& user = m_userInfo[GOLFDOC()->m_currentPlayer];
	if ((user.clubInfo.tid & 0x3ffffff) - 0x3f < 2 && (user.charInfo.tid & 0x3ffffff) != 9)
		return 0;
	return GetClubSetKind983(user.clubInfo.tid);
}

bool CSharedDoc::IsOverlapPart(unsigned long typeId)
{
	std::list<sItemInfo>::iterator it;

	for (it = m_partsList.begin(); it != m_partsList.end(); it++)
	{
		if ((*it).tid == typeId)
		{
			return true;
		}
	}

	return false;
}

int OnlinePlay()
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		return FALSE;

	if (IS_KINDOF(CLobbyTask, AfxGetTask()) ||
		IS_KINDOF(CShopTask, AfxGetTask()))
		return TRUE;

	if (Doc()->m_gameMode != 0 || Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
		Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
		return FALSE;

	return TRUE;
}

unsigned char GetCurMap()
{
	return (Doc()->m_golfGame.map > 127 && Doc()->m_golfGame.map != 0xfd)
		? Doc()->m_golfGame.map - 128
		: Doc()->m_golfGame.map;
}

bool IsCurMapSnow()
{
	unsigned char map = GetCurMap();
	if (map == 8)
		return true;
	if (map == 14)
		return true;
	if (map == 17)
		return true;
	if (map == 9)
		return true;
	return map == 18;  // 983 006330b0: mapas 8/9/14/17/18
}

bool SetCurMap(unsigned char map)
{
	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(
		((map > 127 && map != 0xfd) ? map - 128 : map) | 0x28000000);
	if (pCourse == NULL)
	{
		return false;
	}

	Doc()->m_golfGame.map = map;
	Doc()->m_golfGame.pCourse = pCourse;
	return true;
}

std::string CSharedDoc::GetLoginAuthKey()
{
	return Doc()->m_loginAuthKey;
}

unsigned short CSharedDoc::GetSecurityKey()
{
	return Doc()->m_securityKey;
}

void CSharedDoc::SetLoginAuthKey(std::string key)
{
	Doc()->m_loginAuthKey = key;
}

void CSharedDoc::SetSecurityKey(unsigned short key)
{
	Doc()->m_securityKey = key;
}
void CSharedDoc::RefreshItemListFromGuidList(unsigned char mode)
{
	std::list<sItemInfo>::iterator it;
	std::list<unsigned long>::iterator itGuid;
	std::list<unsigned short>::iterator itCount;

	it = m_myItemList.begin();
	while (it != m_myItemList.end())
	{
		itGuid = m_refreshGuidList.begin();
		itCount = m_refreshCountList.begin();
		for (; itGuid != m_refreshGuidList.end(); ++itGuid, ++itCount)
		{
			if ((*it).guid == *itGuid)
				break;
		}

		if (itGuid == m_refreshGuidList.end())
		{
			it = m_myItemList.erase(it);
			continue;
		}

		if (mode == 2 && (*it).Common[0] != *itCount)
		{
			(*it).Common[0] = *itCount;
		}

		it++;
	}

	if (mode == 2)
	{
		it = m_questDropList.begin();
		while (it != m_questDropList.end())
		{
			itGuid = m_refreshGuidList.begin();
			itCount = m_refreshCountList.begin();
			for (; itGuid != m_refreshGuidList.end(); ++itGuid, ++itCount)
			{
				if ((*it).guid == *itGuid)
					break;
			}

			if (itGuid == m_refreshGuidList.end())
			{
				it = m_questDropList.erase(it);
				continue;
			}

			if ((*it).Common[0] != *itCount)
			{
				(*it).Common[0] = *itCount;
			}

			it++;
		}
	}

	m_refreshGuidList.clear();
	m_refreshCountList.clear();
}

void CSharedDoc::UpdateGachaTickets()
{
	unsigned long typeId[2];
	unsigned short count[2];
	sItemInfo item;
	std::list<sItemInfo>::iterator it;

	typeId[0] = 0x1a000080;
	typeId[1] = 0x1a000083;
	count[0] = m_gachaTicket[0];
	count[1] = m_gachaTicket[1];

	memset(&item, 0, sizeof(sItemInfo));

	for (int i = 0; i < 2; i++)
	{
		for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();)
		{
			if ((*it).tid == typeId[i])
			{
				if (count[i] == 0)
					Doc()->m_myItemList.erase(it);
				else
					(*it).Common[0] = count[i];
				break;
			}

			it++;
		}

		if (count[i] != 0)
		{
			if (it == Doc()->m_myItemList.end())
			{
				item.tid = typeId[i];
				item.Common[0] = count[i];
				Doc()->m_myItemList.push_back(item);
			}
		}
	}
}

unsigned long CSharedDoc::GetMyDisconPangPenalty()
{
	unsigned long penalty = 0;

	switch (Doc()->m_golfGame.gameType)
	{
	case GAME_TYPE_STROKE:
		penalty = 100;
		break;

	case GAME_TYPE_MATCH:
	case GAME_TYPE_SKINS:
		penalty = 350;
		break;

	case GAME_TYPE_TEAM:
		penalty = 300;
		break;

	case GAME_TYPE_30S:
		penalty = 150;
		break;

	case GAME_TYPE_30S_TEAM:
		penalty = 200;
		break;
	}

	if (m_myInfo.stat.Level >= 6)
	{
		penalty += ((m_myInfo.stat.Level - 1) / 5) * 100;

		if (m_myInfo.stat.dwGameCount > 50)
			penalty += ((m_myInfo.stat.dwNoMannerGameCount * 100 /
							m_myInfo.stat.dwGameCount) -
						   10) *
				10;
	}

	return penalty;
}

void CSharedDoc::LoadVisGroup(const char* filename)
{
	cFile* fp = g_resrcmng->GetCFile("visgroup.def", 0xffff);

	if (fp)
	{
		std::vector<std::string> tokens;

		int len = fp->Length();
		char* buf = new char[len + 1];
		fp->Read(buf, len);
		CloseCFile(fp);

		buf[len] = 0;
		char* p = buf;

		for (;;)
		{
			while (*p == '\n' || *p == '\r')
				p++;

			if (*p == 0)
				break;

			char line[256];
			char* q = line;
			while (*p != '\n' && *p != '\r' && *p != 0)
			{
				*q = *p++;
				q++;
			}

			*q = 0;

			LineParse(line, tokens);

			if (tokens.size() > 1)
			{
				std::string& key = tokens[0];

				for (unsigned int i = 1; i < tokens.size(); i++)
					m_visGroupMap[key].push_back(tokens[i]);
			}
		}

		delete[] buf;
	}
}

void CSharedDoc::SetFileName(char* const filename)
{
	strcpy(m_fileName, filename);
}

void CSharedDoc::ReplaySavFileCheck()
{
	Doc()->m_recordedItemList.clear();

	char fileName[32] = { 0 };

	for (int i = 0; i < 100; i++)
	{
		sprintf(fileName, "save/pangya_%03d.sav", i);
		FILE* fp = fopen(fileName, "rb");

		if (fp == NULL)
			continue;

		unsigned long version = 0;
		unsigned long uid = 0xffffffff;
		unsigned char course = 0;
		SYSTEMTIME date;
		int mode = 0;

		fread(&version, 4, 1, fp);
		fread(&uid, 4, 1, fp);
		fread(&course, 1, 1, fp);
		fread(&date, sizeof(SYSTEMTIME), 1, fp);
		fread(&mode, 4, 1, fp);

		if (version == m_replayVersion && uid == MyUID())
		{
			sRecordedItemInfo info;

			if (mode == 1)
				info.itemInfo.tid = 0x1a000043;
			else if (mode == 2)
				info.itemInfo.tid = 0x1a000050;

			info.itemInfo.Common[0] = 1;
			info.course = course;
			info.date = date;
			info.bValid = 1;
			strcpy(info.fileName, fileName);

			Doc()->m_recordedItemList.push_back(info);
		}

		fclose(fp);

		if (Doc()->m_recordedItemList.size() >= 10)
			break;
	}
}

std::list<std::string>* CSharedDoc::GetVisGroup(const char* name)
{
	std::map<std::string, std::list<std::string> >::iterator it =
		m_visGroupMap.find(name);

	if (it == m_visGroupMap.end())
		return NULL;

	return &it->second;
}

bool CSharedDoc::CheckUnderLevelClubSet(unsigned long typeId)
{
	bool bRet = true;
	IFF_STRUCT::sClubSet* pClubSet = Doc()->m_itemManager.FindClubSet(typeId);

	if (pClubSet->c.IsUnderLvl)
	{
		if (pClubSet->c.Level < m_myInfo.stat.Level)
			bRet = false;
	}

	return bRet;
}

void CSharedDoc::InitMapEvent()
{
}

// 983 0063f830: registro do mapa no 0x1ac (0xe bytes) ou NULL
const unsigned char* CSharedDoc::FindMapEvent983(int course)
{
	std::map<int, int>::iterator it = m_mapEvent983.find(course);
	return it != m_mapEvent983.end() ? (const unsigned char*)it->second : NULL;
}

// 983 00697545: registro com dword 0 == 1
bool CSharedDoc::IsMapEventActive(int course)
{
	const int* pEvent = (const int*)FindMapEvent983(course);
	return pEvent != NULL && *pEvent == 1;
}

unsigned long CSharedDoc::GetMapEventExpRate(int course)
{
	return 100;
}

void CSharedDoc::SetIdentity(int identity)
{
	m_myInfo.info.dwIdentity = identity;
}

void CSharedDoc::AccumulateIdentity(int identity)
{
	m_myInfo.info.dwIdentity |= identity;
}

__int64 CSharedDoc::RemainOwnCash(unsigned long provider, __int64 cookie)
{
	__int64 remain = Doc()->m_cookie;

	// A911: 983 00633260: so os tipos 8 e 9 nao descontam (tabela 006332a0); o 645 so descontava em 2 e 4
	if (provider != 8 && provider != 9 && provider <= 0xd)
		remain -= cookie;

	return remain;
}

void CSharedDoc::RefreshAllCookie()
{
	Doc()->m_cookie =
		S5::ExchangeOwnCashToCookie(Doc()->m_cash, Doc()->m_bonusCash, 100);
}

bool CSharedDoc::CanUseRookieChannelMap(unsigned int course)
{
	bool bRet = true;

	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(
		(course > 127 && course != 0xfd ? course - 128 : course) | 0x28000000);
	if (pCourse)
	{
		if (pCourse->Difficulty > 3)
			return false;
	}

	if (course == 8 || course == 1 || course == 9 || course == 14 ||
		course == 6)
		bRet = false;

	return bRet;
}

void CSharedDoc::SetServerTime(_SYSTEMTIME& time)
{
	Doc()->m_serverTime = time;

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		if (CCardManager::IsInstantiated())
			CCardManager::Instance()->ResetSpecialBuffTick();
	}

	Doc()->m_serverTimeTick = GetTickCount();
}

SYSTEMTIME& CSharedDoc::GetServerTime()
{
	return Doc()->m_serverTime;
}

void CSharedDoc::SetDirectMoveRoomInfo(unsigned char type, unsigned short room,
	unsigned long uid)
{
	m_directMoveType = type;
	m_directMoveRoom = room;
	m_directMoveUID = uid;
}

bool CSharedDoc::GetDirectMoveRoomInfo(unsigned char* type,
	unsigned short* room, unsigned long* uid)
{
	if (m_directMoveUID == 0xffffffff || m_directMoveRoom == 0xffff)
		return false;

	*type = m_directMoveType;
	*room = m_directMoveRoom;
	*uid = m_directMoveUID;

	return true;
}

void CSharedDoc::ClearDirectMoveRoomInfo()
{
	m_directMoveType = 0xff;
	m_directMoveRoom = 0xffff;
	m_directMoveUID = 0xffffffff;
}

int CSharedDoc::FindUserInfoTimeUID(unsigned long uid,
	std::map<unsigned long, sUserInfoTime>::iterator& it)
{
	it = m_userInfoTimeMap.find(uid);
	return it != m_userInfoTimeMap.end() ? TRUE : FALSE;
}

int CSharedDoc::FindUserInfoTimeGUID(unsigned long guid,
	std::map<unsigned long, sUserInfoTime>::iterator& it)
{
	if (!m_userInfoTimeMap.empty())
	{
		for (std::map<unsigned long, sUserInfoTime>::iterator i =
				 m_userInfoTimeMap.begin();
			i != m_userInfoTimeMap.end(); ++i)
		{
			if ((*i).second.info.info.dwGuid == guid)
			{
				it = i;
				return TRUE;
			}
		}
	}

	it = m_userInfoTimeMap.end();
	return FALSE;
}

int CSharedDoc::FindUserInfo(unsigned long uid,
	std::map<unsigned long, sBriefUserInfo>::iterator& it)
{
	if (!m_briefUserInfoMap.empty())
	{
		std::map<unsigned long, sBriefUserInfo>::iterator i =
			m_briefUserInfoMap.find(uid);
		if (i != m_briefUserInfoMap.end())
		{
			it = i;
			return TRUE;
		}
	}

	return FALSE;
}

int CSharedDoc::FindUserInfoUID(unsigned long uid,
	std::map<unsigned long, sBriefUserInfo>::iterator& it)
{
	if (m_briefUserInfoMap.empty())
		return FALSE;

	for (std::map<unsigned long, sBriefUserInfo>::iterator i =
			 m_briefUserInfoMap.begin();
		i != m_briefUserInfoMap.end(); ++i)
	{
		if (uid == (*i).second.dwUid)
		{
			it = i;
			return TRUE;
		}
	}

	return FALSE;
}

int CSharedDoc::FindUserInfoTimeUserID(const char* id,
	std::map<unsigned long, sUserInfoTime>::iterator& it)
{
	if (!m_userInfoTimeMap.empty())
	{
		char* lwrId = strlwr((char*)id);

		for (std::map<unsigned long, sUserInfoTime>::iterator i =
				 m_userInfoTimeMap.begin();
			i != m_userInfoTimeMap.end(); ++i)
		{
			char* lwr = strlwr((*i).second.info.info.sID);
			if (strcmp(lwrId, lwr) == 0)
			{
				it = i;
				return TRUE;
			}
		}
	}

	it = m_userInfoTimeMap.end();
	return FALSE;
}

int CSharedDoc::InsertUserInfoTime(unsigned long uid, sUserInfoTime& info)
{
	m_userInfoTimeMap[uid] = info;
	return TRUE;
}

CPeriodContents* CSharedDoc::GetPeriodDoc()
{
	return m_pPeriodDoc;
}

void CSharedDoc::SetPeriodDoc()
{
	m_pPeriodDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0x92,
		m_pPeriodDoc);
}

const eApproachMissionType s_blindMissions[2] = {
	(eApproachMissionType)4,
	(eApproachMissionType)21,
};

bool CSharedDoc::IsCurrentMissionBlind()
{
	bool bRet = false;

	if (Doc()->m_golfGame.gameType == GAME_TYPE_NEW_APPROACH)
	{
		for (int i = 0;
			i < sizeof(s_blindMissions) / sizeof(s_blindMissions[0]); i++)
		{
			if (s_blindMissions[i] == GOLFDOC()->GetApproachMissionType())
			{
				bRet = true;
				break;
			}
		}
	}

	return bRet;
}

int CSharedDoc::IsEquipParts(unsigned long uid, unsigned long typeId)
{
	std::map<unsigned int, sCharacterInfo>::iterator it;

	if (typeId >> 26 == 2)
	{
		for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			for (int i = 0; i < 24; i++)
			{
				if ((*it).second.tidParts[i] &&
					uid == (*it).second.ItemIdList[i])
					return TRUE;
			}
		}
	}
	else if (typeId >> 26 == 28)
	{
		for (it = Doc()->m_charMap.begin(); it != Doc()->m_charMap.end(); ++it)
		{
			for (int i = 0; i < 5; i++)
			{
				if ((*it).second.tidAuxParts[i] &&
					typeId == (*it).second.tidAuxParts[i])
					return TRUE;
			}
		}
	}

	return FALSE;
}

bool CSharedDoc::IsEquipParts(sCharacterInfo& info, unsigned long uid,
	unsigned long typeId)
{
	if (typeId >> 26 == 2)
	{
		for (int i = 0; i < 24; i++)
		{
			if (info.tidParts[i])
			{
				if (uid == info.ItemIdList[i])
					return true;
			}
		}
	}
	else if (typeId >> 26 == 28)
	{
		for (int i = 0; i < 5; i++)
		{
			if (info.tidAuxParts[i])
			{
				if (typeId == info.tidAuxParts[i])
					return true;
			}
		}
	}

	return false;
}

std::map<unsigned char, sMapStatistics>& CSharedDoc::GetMyPastMapStat(
	unsigned char season)
{
	unsigned char index = 0;

	switch (season)
	{
	case 0:
		index = 0;
		break;
	case 10:
		index = 1;
		break;
	case 51:
		index = 2;
		break;
	case 5:
		index = 3;
		break;
	}

	return m_pastMapStat[index];
}

float CalcPowerBarPosition()
{
	WVector cup = GetHoleCupPos();
	WVector diff = GolfBall().m_pos - cup;
	float range = GolfClub().GetRange() * 3.2f;

	diff.y = 0.0f;
	if (diff.Magnitude() > range)
		return BAR_END;

	char buf[32] = { 0 };
	sPlayerData* pPlayer = PLAYER(GOLFDOC()->m_currentPlayer);
	cup = GOLFDOC()->GetHoleData().pin;
	WVector dist2 = cup - pPlayer->pos;
	dist2.y = 0.0f;
	float dist = sqrtf(dist2 * dist2) * 0.3125f;

	if (dist < 100.0f)
		sprintf(buf, "%.1f", dist);
	else
		sprintf(buf, "%.0f", dist);

	float value = (float)atof(buf);
	value *= 3.2f;

	return (value / range) * BAR_LENGTH + BAR_START;
}

unsigned char CSharedDoc::FindPlayerByUid983(unsigned long uid) // 0062bee0
{
	for (int i = 0; i < 4; i++)
	{
		if (uid == m_userInfo[i].info.dwUID)
			return (unsigned char)i;
	}
	return 0xff;
}

sItemInfo* CSharedDoc::FindClubSet983(unsigned long guid) // 0063f330
{
	if (guid == (unsigned long)-1)
		guid = m_myInfo.userEquip.guidClubSet;

	std::map<unsigned int, sItemInfo>::iterator it = m_clubSetMap.find(guid);
	if (it != m_clubSetMap.end())
	{
		sItemInfo* pInfo = &(*it).second;
		if ((pInfo->tid & 0xfc000000) == 0x10000000)
			return pInfo;
	}
	return NULL;
}

sCaddieInfo* CSharedDoc::FindCaddie983(unsigned long guid) // 0063f390
{
	if (guid == (unsigned long)-1)
		guid = m_myInfo.userEquip.guidCaddie;

	std::map<unsigned int, sCaddieInfo>::iterator it = m_caddieMap.find(guid);
	if (it != m_caddieMap.end())
	{
		sCaddieInfo* pInfo = &(*it).second;
		if ((pInfo->tid & 0xfc000000) == 0x1c000000)
			return pInfo;
	}
	return NULL;
}

sCharacterInfo* CSharedDoc::FindChar983(unsigned long guid) // 0063f3f0
{
	if (guid == (unsigned long)-1)
		guid = m_myInfo.userEquip.guidChar;

	CHAR_MAP_IT it = m_charMap.find(guid);
	if (it != m_charMap.end())
	{
		sCharacterInfo* pInfo = &(*it).second;
		if ((pInfo->tid & 0xfc000000) == 0x04000000)
			return pInfo;
	}
	return NULL;
}

void PerformPowerBarSetting()
{
	// HACK: these seem to be DCE'd dependencies in the original
	if (0)
	{
		WFlags flag;
		flag.Set(0);
		flag.Reset();
	}

	float pos = CalcPowerBarPosition();

	CTask* pTask = AfxGetTask();
	IActor* pScreen = pTask->GetActor("Screen");
	if (pScreen)
		pScreen << MsgObject(NULL, M983(0xa1), (int)&pos, 0, 0, 0, 0);
}

// 983: 00636dd0 (so o placar do Grand Prix chama). Ordem do Grand Prix: quem saiu (state 3) fica atras pela ordem de saida;
// os outros por placar, pang, marca +0x1b4 com a ordem e o valor +0x1b0. Nome meu.
void CSharedDoc::CalcRivalRank983()
{
	std::vector<sRivalData>::iterator it;

	for (it = Doc()->m_rivalList.begin(); it != Doc()->m_rivalList.end(); ++it)
	{
		if ((*it).state == 3)
		{
			(*it).rank = (unsigned char)Doc()->m_rivalList.size() - (*it).quitOrder;
			continue;
		}

		if ((*it).state != 1 && (*it).state != 2)
			continue;

		(*it).rank = 1;
		std::vector<sRivalData>::iterator it2;
		for (it2 = Doc()->m_rivalList.begin(); it2 != Doc()->m_rivalList.end(); ++it2)
		{
			if (it == it2 || (*it2).state == 3)
				continue;
			if ((*it2).state != 1 && (*it2).state != 2)
				continue;

			if ((*it).totalScore < (*it2).totalScore)
			{
			}
			else if ((*it).totalScore > (*it2).totalScore)
				(*it).rank++;
			else if ((*it).totalScore == (*it2).totalScore)
			{
				if ((*it).totalPang > (*it2).totalPang)
				{
				}
				else if ((*it).totalPang < (*it2).totalPang)
					(*it).rank++;
				else if (!(*it).bGrandPrixAI983 && (*it2).bGrandPrixAI983)
				{
				}
				else if ((*it).bGrandPrixAI983 && (*it2).bGrandPrixAI983)
				{
					if ((*it).order <= (*it2).order)
					{
						if ((*it).rankTieBreak983 > (*it2).rankTieBreak983)
							(*it).rank++;
					}
					else
						(*it).rank++;
				}
				else if ((*it).bGrandPrixAI983 && !(*it2).bGrandPrixAI983)
					(*it).rank++;
				else if ((*it).rankTieBreak983 > (*it2).rankTieBreak983)
					(*it).rank++;
			}
		}
	}
}

ILFILLTU_SHAREDDOC
