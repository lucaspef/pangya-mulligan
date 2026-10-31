#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>
#include "../shared/globalgamedefine.h"
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
// HACK: the original compiled these inline bodies of work.h and taskmanager.h after the ones of
// globalgamedefine.h (precompiled header order decides the emission order in taskmanager.obj)
#include "taskmanager.h"
inline CWork::~CWork()
{
}
inline void CTaskManager::SetWork(CWork* pWork)
{
	m_pWork.reset(pWork);
}
inline void CTaskManager::ReleaseWork()
{
	delete m_pWork.release();
}
#endif
#include "../shared/globalnetworkdefine.h"
typedef std::map<unsigned long, sUserInfoTime>::iterator USERINFOTIME_MAP_IT;
typedef std::map<unsigned long, sBriefUserInfo>::iterator BRIEFUSERINFO_MAP_IT;
typedef std::map<int, GuildMemberInfo_t>::iterator GUILDMEMBER_MAP_IT;
typedef std::list<sMailInfoBrief>::iterator MAIL_LIST_IT;
typedef std::vector<unsigned long>::iterator ACQUIREITEM_LIST_IT;
typedef std::list<unsigned long>::iterator GUID_LIST_IT;
typedef std::map<unsigned long, unsigned char>::iterator INDEX_MAP_IT;
typedef std::auto_ptr<std::string>::element_type AUTOSTRING_ELEM;
#include "polysoup.h"
#include "golfdoc.h"
#include "z_ilfill.h"
ILFILL2A
#include "../shared/classdefine.h"
ILFILL2
#include "../shared/localize.h"
#include "hatmanager.h"
#include "parttidlist.h"
#include "golfrule.h"
struct sItemCount983
{
	unsigned long uid; // +0
	unsigned long typeId; // +4 (chave da busca)
	int count; // +8
};
#include "jp/questitem.h" // 983: elementos das listas +0x3840/+0x384c (B97)
#include "packets/guildlist983.h" // 983: elementos dos vetores +0x10fe8/+0x10ff8

class CExhibition;
class CPeriodContents;
class FrArea;
class FrComboBox;
struct sWinningPrize;
struct sTradeItem;
struct sStall;
struct sSaleItem;
struct sCardStack;
struct sPrizeInfo;

enum eStateTrade
{
	STATE_TRADE_INIT,
	STATE_TRADE_OPEN,
	STATE_TRADE_EDIT,
	STATE_TRADE_OUTOFSTOCK,
	STATE_TRADE_ENTER
};

enum eReservedChatPartner
{
};

struct sRoomSlot
{
	FrArea* pArea;
	CExhibition* pExhibition;
	CPartTidList partTidList;
	sCharacterInfo charInfo;
	unsigned long dwUserUID; // +0x2d4 uid da conta (pacote do lugar dwUserUID; 1o argumento de 00b304b0; bits em +0x2d8, sizeof 0x2dc pelo no do mapa 0050ce40) // 983: +8 além do sCharacterInfo (lugar provisório)

#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: body after sFindRival, as in the original PCH (emission order)
	sRoomSlot();
#else
	sRoomSlot()
	{
		pArea = NULL;
		pExhibition = NULL;
	}
#endif

	unsigned char bAngelWing : 1;
	unsigned char bGachaWing : 1;
};

#pragma pack(push, 1)
struct sGuildMatchup
{
	unsigned char team;
	unsigned long uid[2];
};
#pragma pack(pop)

struct sGameTypeInfo
{
	const char* name;
	unsigned char maxPlayer;
	unsigned char minPlayer;
	unsigned char holes;
};

struct sGolfGame
{
	unsigned char map;
	IFF_STRUCT::sCourse* pCourse;
	unsigned char gameType;
	char gameTypeName[260];
	unsigned char holes;
	unsigned char weather;
	unsigned long shotTimeLimit;
	unsigned long gameTimeLimit;
	unsigned long gpTimeLimit983; // limite do relogio do Grand Prix em Doc+0x106a4 (983) (sRoomInfo::gpStartValue983; inicio em +0x1057c; CScreen 00752660)

	sGolfGame()
		: map(0),
		  pCourse(NULL),
		  gameType(0),
		  weather(0),
		  shotTimeLimit(0),
		  gameTimeLimit(0),
		  gpTimeLimit983(0),
		  holes(18)
	{
		memset(gameTypeName, 0, sizeof(gameTypeName));
	}
};

enum eReplayModeType
{
};

struct sRivalData
{
	unsigned long uid;
	unsigned long oid;
	char nickname[22];
	char guildName[20]; // 983: 21 -> 20 (team em +0x32)
	unsigned char team;
	unsigned short level;
	unsigned char state;
	int holeScore[18];
	unsigned char holeStroke[18];
	__int64 holePang[18];
	__int64 totalPang;
	__int64 totalBonusPang;
	// 983: 0x14 bytes novos (totalScore em +0x14c), do S->C 0x18 do Hole-in-One (CHoleInOneGameHandler::On18); colunas do
	// result_holein_frame: Score | Hole-In | # Shots | H/S Rate | Point | Pang
	__int64 hioScore983; // +0x138: dword baixo = coluna "Hole-In" (acertos; H/S Rate = acertos / holeStroke[0]), alto (+0x13c) = "Point"
	__int64 hioPrizePang983; // +0x140: pang do premio (icone item1_16, item_JackpotPang no modo jackpot)
	int hioTrophyTid983; // +0x148: tid do trofeu (0x2d0a6200/6300/6400 -> m_pMedal[0..2] do resultado)
	int totalScore;
	unsigned char totalStroke;
	unsigned char hole;
	unsigned char rank;
	unsigned char order;
	unsigned char quitOrder;
	unsigned char finishOrder;
	WVector ballPos;
	unsigned long observerState;
	unsigned char observerRank;
	unsigned long titleTypeId;
	unsigned char flag;
	unsigned long guildUID;
	unsigned long guildEmblemVer983; // 983: recebe sSlotInfo::dwEmblemVer983 (guildMark em +0x17c)
	char guildMark[12];
	unsigned long guildPoint;
	unsigned long approachDistance;
	unsigned long approachResultDistance;
	int approachTime;
	unsigned long capability;
	unsigned long mascotTypeId;
	unsigned char pangMastery;
	unsigned char pangNitro;
	unsigned long shotSpecial983; // 983: +0x1a4, dword do 0x6e = "special" do 0x1b do rival (2 Tomahawk, 4 Spike, 8 Cobra; cor em CSpot::GetRivalColor 007282a0)
	unsigned char prevRank983; // 983: +0x1a8, rank guardado antes do buraco (FrGrandPrixRankForm 00782070); seta sobe/desce
	unsigned long tidChar983; // 983: sSlotInfo::tidChar (golfrule30battle) / tipo do NPC (aiplayer); o placar desenha o boneco por ele
	float rankTieBreak983; // 983: sSlotInfo::rankTieBreak983; desempate do CalcRivalRank983 (00636dd0)
	unsigned char bGrandPrixAI983; // 983: 1 = NPC do Grand Prix (aiplayer); placar/resultado pulam essa linha
	unsigned short grandPrixAIId983; // 983: id do NPC (GetNpcId983)

	// 983: função 00544f90 (fora de linha, sem valor de retorno: não é construtor; uid e oid ficam como estão). Nome meu.
	void Clear983()
	{
		memset(nickname, 0, sizeof(nickname));
		memset(guildName, 0, sizeof(guildName));
		team = 0;
		level = 0;
		state = 0;
		memset(holeScore, 0, sizeof(holeScore));
		memset(holeStroke, 0, sizeof(holeStroke));
		memset(holePang, 0, sizeof(holePang));
		totalPang = 0;
		totalBonusPang = 0;
		hioScore983 = 0;
		hioPrizePang983 = 0;
		hioTrophyTid983 = 0;
		totalScore = 0;
		totalStroke = 0;
		hole = 0;
		rank = 0;
		order = 0;
		quitOrder = 0;
		finishOrder = 0;
		ballPos = WVector::ZERO;
		observerState = 0;
		observerRank = 0;
		titleTypeId = 0;
		flag = 0;
		guildUID = 0;
		guildEmblemVer983 = 0;
		memset(guildMark, 0, sizeof(guildMark));
		guildPoint = 0;
		memset(&approachDistance, 0, 8); // 983: os dois campos de distância num memset só
		approachTime = 0;
		capability = 0;
		mascotTypeId = 0;
		memset(&pangMastery, 0, 2);
		rankTieBreak983 = 0.0f;
		shotSpecial983 = 0;
		prevRank983 = 0;
		tidChar983 = 0;
		bGrandPrixAI983 = 0;
		memset(&grandPrixAIId983, 0, sizeof(grandPrixAIId983));
	}
};

#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: shareddoc.h:211 in the original (PCH inline, emission order)
struct sFindRival
{
	sFindRival(unsigned long uid)
		: m_uid(uid)
	{
	}

	bool operator()(sRivalData& rival) { return rival.oid == m_uid; }

	unsigned long m_uid;
};

inline sRoomSlot::sRoomSlot()
{
	pArea = NULL;
	pExhibition = NULL;
}
#endif

#pragma pack(push, 1)
struct sRecordedItemInfo
{
	sItemInfo itemInfo;
	unsigned long course;
	SYSTEMTIME date;
	char fileName[32];
	unsigned char bValid;
};
#pragma pack(pop)

class CGolfDoc;

struct sChatLine
{
	std::string text;
	unsigned long color;
};

#include "../shared/itemmanager.h"
typedef std::map<unsigned long, sRoomSlot>::iterator ROOMSLOT_MAP_IT;
typedef std::list<sItemInfo>::iterator ITEM_LIST_IT;
typedef std::list<sRecordedItemInfo>::iterator RECORDEDITEM_LIST_IT;
typedef std::list<sGiftInfo>::iterator GIFT_LIST_IT;
typedef std::map<unsigned long, sFriend>::iterator FRIEND_MAP_IT;
typedef std::map<unsigned int, std::string>::iterator STRING_MAP_IT;
typedef std::map<unsigned int, sCharacterInfo>::iterator CHAR_MAP_IT;
typedef std::map<unsigned int, sCaddieInfo>::iterator CADDIE_MAP_IT;
typedef std::map<unsigned int, sMascotInfo>::iterator MASCOT_MAP_IT;
typedef std::map<unsigned int, sItemInfo>::iterator CLUBSET_MAP_IT;
typedef std::vector<sRivalData>::iterator RIVAL_LIST_IT;
typedef std::vector<sCaddieReportData>::iterator CADDIEREPORT_LIST_IT;
typedef std::list<std::string>::iterator STRING_LIST_IT;
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
typedef std::list<ranktype::sListInfo>::iterator
	RANK_LIST_IT; // HACK: instantiates list<sListInfo> before list<sNoteInfo> (template COMDAT order of taskmanager)
#endif
typedef std::list<sNoteInfo>::iterator NOTE_LIST_IT;
typedef std::list<sSpecialTrophy>::iterator SPECIALTROPHY_LIST_IT;
typedef std::list<sGuildTrophy>::iterator GUILDTROPHY_LIST_IT;
typedef std::vector<sGuildMatchup>::iterator GUILDMATCHUP_LIST_IT;
typedef std::list<sGuildMatchFlagInfo>::iterator GUILDMATCHFLAG_LIST_IT;
typedef std::list<sQuest>::iterator QUEST_LIST_IT;
typedef std::list<sTradeItem>::iterator TRADEITEM_LIST_IT;
typedef std::list<sStall>::iterator STALL_LIST_IT;
typedef std::list<sSaleItem>::iterator SALEITEM_LIST_IT;
typedef std::list<sFurniture_List>::iterator FURNITURE_LIST_IT;
typedef std::list<sCards>::iterator CARD_LIST_IT;
typedef std::list<sSCardAvilityPeriodInfo>::iterator CARDABILITY_LIST_IT;
typedef std::list<sCardStack>::iterator CARDSTACK_LIST_IT;
typedef std::list<sBurnningSpCard>::iterator BURNNINGSPCARD_LIST_IT;
typedef std::list<sMapEventInfo>::iterator MAPEVENT_LIST_IT;
typedef std::list<CGolfRule::sShotData>::iterator SHOTDATA_LIST_IT;
typedef std::list<sItemAttachCard>::iterator ITEMATTACHCARD_LIST_IT;
typedef std::map<unsigned char, sMapStatistics>::iterator MAPSTATISTICS_MAP_IT;
typedef std::list<unsigned short>::iterator COUNT_LIST_IT;
typedef std::list<sRoomInfo>::iterator ROOM_LIST_IT;
typedef std::list<sChannelInfo>::iterator CHANNEL_LIST_IT;
typedef std::list<sSlotInfo>::iterator SLOT_LIST_IT;
typedef std::list<sGameServerInfo>::iterator GAMESERVER_LIST_IT;
typedef std::vector<sGameServerInfo>::iterator GAMESERVER_VECTOR_IT;
typedef std::list<sRoomUserInfo>::iterator ROOMUSER_LIST_IT;
typedef std::list<sChatLine>::iterator CHATLINE_LIST_IT;
typedef std::map<std::string, std::list<std::string> >::iterator
	VISGROUP_MAP_IT;

// 983: valor do mapa do pacote 0x20e (CSharedDoc+0x113e0, 009d3801; nó de 0x1c): tid + estado de venda (63f280; 3 = esconde comprar/presente)
struct sShopSaleState983
{
	unsigned long typeId; // +0x0: = chave (a ordem fixa da loja lê este campo)
	unsigned long state;  // +0x4
};
// 983: classe nova do Self Design (RTTI .?AVCCommodityList@sd_shop@@, vtable 00d54504, construtor 00b3d270):
// vtable, CRITICAL_SECTION em +4, mapa em +0x20 com nó de 0x25c. Corpo fica para a F3.7 (sistemas novos).
namespace sd_shop {
// valor do mapa de CCommodityList (0x248, gravado em 00b3cd60): cópia da peça do Part.iff + id da mercadoria + índice UCC
struct sCommodity
{
	IFF_STRUCT::sPart part; // +0x000: peça achada pelo typeId do registro
	unsigned long commodityId; // +0x238: chave do mapa e id do sUccClothes
	char uccIndex[0xc]; // +0x23c
};
class CCommodityList
{
public:
	CCommodityList();
	virtual ~CCommodityList();
	CRITICAL_SECTION m_cs;
	int m_listVersion; // +0x1c, sem gravação no construtor: marca do servidor no 0x1b1 (009d3354), devolvida no 0xfe ao abrir a loja (00a86990); 0 = lista vazia
	std::map<unsigned long, sCommodity> m_map;
};
}
class CSharedDoc : public WSingleton<CSharedDoc>
{
public:
	enum eAppGiftDownloadState
	{
	};

	CSharedDoc();
	virtual ~CSharedDoc();

	bool IsCurrentMissionBlind();
	int FindUserInfoTimeUID(unsigned long uid,
		std::map<unsigned long, sUserInfoTime>::iterator& it);
	std::map<unsigned char, sMapStatistics>& GetMyPastMapStat(
		unsigned char season);
	std::string GetLoginAuthKey();
	__int64 IsControlServerService(__int64 service); // 0062ba40
	void ClearUserInfoTimeMap(unsigned long uid);
	void HaveAngelWing(bool bHave) { m_bHaveAngelWing = bHave; }
	void SetIndex(unsigned long uid);
	unsigned char GetIndex(unsigned long guid);
	float GetWtPepPangyaComboGauge(unsigned char player);
	float GetComboGaugeLimit(unsigned char team);
	SYSTEMTIME& GetServerTime();
	int CollapseItemSlot(unsigned char slot);

	const unsigned char* FindMapEvent983(int course);
	bool CanCompound(unsigned long uid);
	bool CanUseRookieChannelMap(unsigned int course);
	__int64 RemainOwnCash(unsigned long provider, __int64 cookie);

#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	char* GetFileName();
#else
	char* GetFileName() { return m_fileName; }
#endif
	const std::list<sItemInfo>& GetConstMyItemList() { return m_myItemList; }
#ifdef REBANG_TASKMAIN_PCH // taskmain_pch.h: defined after the class, in the order of the original header
	eReplayModeType GetPlayingMode();
#else
	eReplayModeType GetPlayingMode() { return m_playingMode; }
#endif

	void ClearCardStack() { m_cardStackList.clear(); }
	void ClearAll();
	void ClearUcc();
	void ClearJapanVars();
	void ClearInventory();
	void ClearGameVars();
	void ClearRivalVars(bool bAll);
	void SetRivalState983(unsigned char state); // 983: 0062fdf0 (so o CGolfRule30Battle::RequestShotReview chama; pedido C31)
	int GetBuffValue(int type, int index);
	void ClearOldSeasonStat();
	void ClearCurrentChannel(bool bAll);
	void ClearRealEquip();
	void LoadItemDb();
	void InitGolfDoc();
	void LoadChatFilter();
	bool AreWePlayingTogether(unsigned long uid);
	bool SetWinningPrize(unsigned long uid, const sWinningPrize& prize);
	bool SetExtPrize(unsigned long uid, int num, sPrizeInfo* prize);
	float GetAuxPartProperty(unsigned char player, unsigned char type);
	unsigned int GetClubSetKind983(unsigned long tid); // JP 983: 00632fd0 (sClubSet +0x100; 0x12 se zero ou sem registro)
	float GetRangeBonus983(unsigned char player); // JP 983: 006362a0 (nova; usa o CItemManager 009d9af0/00a18a80/009d9a70)
	bool FindEquipPart(unsigned char player, unsigned long typeId);
	void AddChar(sCharacterInfo* info);
	// JP 983 (s72): 0062bee0 = indice (0-3) do jogador da sala com este uid, 0xff se nao achar (pedido B15);
	// 0063f3f0 = personagem do m_charMap por guid (-1 = o equipado), NULL se nao achar ou se o tid nao for de personagem
	unsigned char FindPlayerByUid983(unsigned long uid);
	sCharacterInfo* FindChar983(unsigned long guid);
	// JP 983 (s77): 0063f330 / 0063f390 = taco e caddie do proprio jogador por guid (-1 = o equipado), NULL se o tid for de outro tipo
	sItemInfo* FindClubSet983(unsigned long guid);
	sCaddieInfo* FindCaddie983(unsigned long guid);
	// JP 983 (s77): parcelas do alcance do taco (chamadas so por CClub::GetRangeByEquip983 006ad580):
	// 006364d0 = pecas auxiliares (DriveUp + bonus de conjunto); 006403f0 = mascote (DriveUp)
	float GetAuxPartRange983(unsigned char player);
	float GetMascotRange983(unsigned char player);
	void AddCaddie(sCaddieInfo* info);
	void AddMascot(sMascotInfo* info);
	sCardStack* GetCardInfoToUID(unsigned long uid);
	sCardStack* GetCardInfoToTID(unsigned long tid);
	void ReplaceCardCountToTID(unsigned long tid, int count);
	void ReplaceCardCountToUID(unsigned long uid, int count);
	void DeleteCardToPartsAttach(unsigned long uid);
	void AddCardStack(sCardStack card, bool bReplace);
	void DeleteCardStack(unsigned long uid, int count);
	void AddCard(unsigned long uid, unsigned long tid, unsigned short count);
	void AddFurniture(const sFurniture_List& furniture);
	void AddMyPartsList(const sItemInfo& item);
	sMascotInfo* GetMyMascotInfo(unsigned long uid);
	sMascotInfo* GetMyEquippedMascotInfo();
	int GetNumAddItemSlotByMascot(unsigned char player);
	int GetNumAddItemSlotByMascot();
	void SendEquipItemSlotByMascot();
	void SendEquipItemSlot();
	void SetCurEquipInfo(sUserEquip* equip);
	// JP 983 (006332f0): alguma das 24 pecas do personagem tem o campo +0x22a do Part.iff (CaddieSlot) >= 1
	bool HasSeventhCardSlot983(const sCharacterInfo* pInfo);
	void CheckCurrEquipParts();
	void AddMyItem(const sItemInfo& item);
	short DecreaseMyItem(std::list<sItemInfo>::iterator& it); // 983: 006407d0 (pedido C19)
	bool DeleteMyItem(unsigned long uid);
	bool DeleteMyItem(std::list<sItemInfo>::iterator& it);
	sItemInfo* FindMyItemByGuid(unsigned long uid);
	sItemInfo* FindMyItemByTypeId983(unsigned long typeId); // 983: 0063b570 (pedido B65)
	void ClearMyGuildInfo();
	const sPangYaUserInfo& GetPangyaUserInfo();
	void SetEquipCharCaddieInfoFromMyInfo();
	sRivalData* GetRival(unsigned long oid);
	bool HasRival983(unsigned short count); // 983: 00632cc0 (m_rivalList.size() >= count; placar do Grand Prix)
	void CalcRivalRank983();                // 983: 00636dd0 (rank de cada rival no Grand Prix)
	const char* GetDefaultTitle(unsigned char level);
	void Log30sScore();
	void LogGstUpdateHole(unsigned long uid, unsigned char hole, int score,
		__int64 pang);
	void LoadSchoolName(char* const name, unsigned long id);
	bool InitRecentWhisperList(FrComboBox* combo);
	bool InitReservedChatList(FrComboBox* combo, eReservedChatPartner partner);
	bool AddWhisperPartner(FrComboBox* combo, const char* name);
	bool UpdateChatTarget(FrComboBox* combo);
	bool UTIL_SendChatMessage(FrComboBox* combo, const char* msg,
		bool bWhisper);
	const char* GetPrevChatTarget(const char* name);
	const char* GetNextChatTarget(const char* name);
	void CheckAngelWing();
	void SetInitHaveHalloweenItem();
	void SetInitWearHalloweenItem();
	void SortAllMyItemList();
	bool ConfirmInsertItem(unsigned long typeId, unsigned long count) const;
	bool BuildMyPartTidListDefault(unsigned long typeId);
	void BuildMyPartTidList();
	void BuildMyPartTidList(sCharacterInfo& info);
	void BuildTikiReportList();
	CPartTidList& GetMyPartTidList();
	unsigned long* GetMyAuxPartTidList();
	bool IsOverlapPart(unsigned long typeId);
	int GetIndexPangyaLogoImage();
	void SetInitAbilityItem();
	void InitTradeData();
	eStateTrade GetStateTrade() { return m_stateTrade; }
	void ClearListTradeItem() { m_listTradeItem.clear(); }
#ifdef REBANG_PCH_TRADE_ERASE
	// HACK: see stdafx_trade.h
	void HackEraseTradeItem(std::list<sTradeItem>::iterator it)
	{
		m_listTradeItem.erase(it);
	}
#endif
	std::list<sTradeItem>* GetListTradeItem() { return &m_listTradeItem; }
	int GetSizeTradeItem() { return (int)m_listTradeItem.size(); }
	void SetStateTrade(eStateTrade state) { m_stateTrade = state; }
	void AddTradeIncome(__int64 income) { m_tradeIncome += income; }
	void ClearListStallItem() { m_listStallItem.clear(); }
	std::list<sStall>* GetListStallItem() { return &m_listStallItem; }
	void AddListStallItem(sStall item) { m_listStallItem.push_back(item); }
	sStall* GetStallItem() { return &m_stallItem; }
	std::list<sSaleItem>* GetMaketList() { return &m_maketList; }
	void ClearMaketList() { m_maketList.clear(); }
	void AddMaketListItem(sSaleItem item) { m_maketList.push_back(item); }
	bool AddListTradeItem(sTradeItem item);
	bool MinusListTradeItem(sTradeItem item);
	void SetTradeUID(unsigned long uid);
	void ChageListStallitem(sStall* stall);
	bool SendOfflineTradeEditShop();
	bool SendOfflineTradeVisitor(unsigned long uid);
	bool SendTradeOpenShop();
	bool SendTradeCloseShop();
	bool SendTradeEditShop();
	bool SendTradeEnterShop(unsigned long uid);
	bool SendTradeExitShop(unsigned long uid);
	bool SendTradeTitle(std::string title);
	bool SendTradeShowVisitor();
	bool SendTradeIncome();
	bool SendTradeBuyItem(unsigned long uid, sTradeItem item);
	unsigned short GetSecurityKey();
	void SetLoginAuthKey(std::string key);
	void SetSecurityKey(unsigned short key);
	void RefreshItemListFromGuidList(unsigned char mode);
	void UpdateGachaTickets();
	unsigned long GetMyDisconPangPenalty();
	void LoadVisGroup(const char* filename);
	void SetTradeMode(unsigned char mode) { m_tradeMode = mode; }
	void SetFileName(char* const filename);
	void ReplaySavFileCheck();
	std::list<std::string>* GetVisGroup(const char* name);
	bool CheckUnderLevelClubSet(unsigned long typeId);
	void InitMapEvent();
	bool IsMapEventActive(int course);
	unsigned long GetMapEventExpRate(int course);
	void SetIdentity(int identity);
	void AccumulateIdentity(int identity);
	void RefreshAllCookie();
	void SetServerTime(_SYSTEMTIME& time);
	void SetDirectMoveRoomInfo(unsigned char type, unsigned short room,
		unsigned long uid);
	bool GetDirectMoveRoomInfo(unsigned char* type, unsigned short* room,
		unsigned long* uid);
	void ClearDirectMoveRoomInfo();
	int FindUserInfoTimeGUID(unsigned long guid,
		std::map<unsigned long, sUserInfoTime>::iterator& it);
	int FindUserInfo(unsigned long uid,
		std::map<unsigned long, sBriefUserInfo>::iterator& it);
	int FindUserInfoUID(unsigned long uid,
		std::map<unsigned long, sBriefUserInfo>::iterator& it);
	int FindUserInfoTimeUserID(const char* id,
		std::map<unsigned long, sUserInfoTime>::iterator& it);
	int InsertUserInfoTime(unsigned long uid, sUserInfoTime& info);
	CPeriodContents* GetPeriodDoc();
	void SetPeriodDoc();
	int IsEquipParts(unsigned long uid, unsigned long typeId);
	bool IsEquipParts(sCharacterInfo& info, unsigned long uid,
		unsigned long typeId);
	unsigned char GetForceNicknameChange() const
	{
		return m_forceNicknameChange;
	}
	void ClearServerVars(); // publico: 983 CProjectG::OnGameServerResult 00a67600 chama
	void SetFriendGameSvrUID(unsigned long uid) { m_friendGameSvrUID = uid; }
	unsigned long GetFriendGameSvrUID();
	void SetForceNicknameChange(unsigned char);
	void SetTimeBlock(int);

	// HACK: real CSharedDoc methods (PDB) and never-defined pads, declared only for the PCH name count:
	// lobbytask's declarations above shift it, and projectg (SaveScreenShot) / mascotinfodlg (OnInit) only
	// match again with 39..48 extra names (swept; 43 used).
	// fontinfo (two in-class bodies below, fontinfo.h and quadtree.h declarations): nine pads fewer keep
	// SaveScreenShot, MakeStatistics, OnInit and OnFriendListOwnerDraw (swept on the current master:
	// only 39..40 pads match all four; 40 used).
	short GetBsRemainedBonusTimes();
	short GetBsUsableBonusTimes();
	short GetBsUsableTimes();
	int GetCountVisitor();
	unsigned long GetFlagAbilityItem() { return m_flagAbilityItem; }
	unsigned long GetFlagBlock();
	bool GetGoStopMode();
	int GetGoWithFriendUID();
	int GetNowGoWithMode();
	unsigned char GetNumBonusPangItem() { return m_numBonusPangItem; }
	int GetReplayPlayType();
	unsigned char GetRmrMode();
	__int64 GetTradeIncome();
	unsigned char GetTradeMode() { return m_tradeMode; }
	const char* GetTradeTitle();
	unsigned long GetTradeUID();
	void SetBsRemainedBonusTimes(short);
	void SetBsUsableBonusTimes(short);
	void SetBsUsableTimes(short);
	void SetCountVisitor(int);
	void SetFlagBlock(unsigned long);
	void SetGoStopMode(bool);
	void SetPlayingMode(eReplayModeType);
	void SetReplayPlayType(int);
	void SetRmrMode(unsigned char);
	void SetStallItem(sStall);
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	void SetTradeIncome(__int64);
	void SetTradeTitle(std::string);
#endif
	void RbNamePad0();
	void RbNamePad1();
	void RbNamePad2();
	void RbNamePad3();
	void RbNamePad4();
	void RbNamePad5();
	void RbNamePad6();
	void RbNamePad7();
	void RbNamePad8();
	int GetTimeBlock();
	// treasuredlg (frcombobox.h in the PCH): fourteen pads fewer (window 12..16 fewer for
	// SaveScreenShot/mascotinfodlg; polysoup and frienddlg accept 12..22).
	// realmyroommain's PCH inlines (anotherhand.inl, the SetFog() stand-in) need 37 more
	// (window 34..40 for SaveScreenShot/polysoup/mascotinfodlg/frienddlg).
	// privatetradedlg (sItemInfo tid/typeId union, RbStringIteratorAt in stdafx.h): seven pads fewer
	// (window 5..9 for SaveScreenShot/mascotinfodlg; polysoup and frienddlg do not react).
	// realmyroomtask's sItemInfo guid/id union (globalgamedefine.h): eight pads fewer
	// (window 6..10 fewer for SaveScreenShot; full image verified).
	// npc's PCH inline (WPuppet::GetFrameDataTime) needs two or three pads fewer (swept: only
	// those keep SaveScreenShot/polysoup/mascotinfodlg/frienddlg; two fewer used).
	void SetInviteMode(int mode) { m_inviteMode = mode; }
	int IsInviteMode() { return m_inviteMode; }
	void SetGoWithModeStart(int uid) { m_goWithFriendUID = uid; }
	void SetGoWithModeEnd()
	{
		m_goWithMode = 0;
		m_goWithFriendUID = -1;
	}
	void SetGoWithMode(int mode) { m_goWithMode = mode; }

protected:
	void ClearCommonVars();
	void ClearGameMode();
	void ClearStatistics();
	void ClearLobbyStateVars();
	void InitGameType();
	void LoadLevelTable();

public:
	std::string m_password;
	std::string m_gameServerAddr;
	std::string m_unusedString60;
	unsigned long m_gameServerPort;
	unsigned long m_unused80;
	unsigned long m_gameServerUID;
	unsigned long m_unused88;
	unsigned long m_loginServerUID;
	bool m_bSentHoleStat;
	unsigned long m_needParentAgree;
	std::string m_rankServerAddr;
	unsigned long m_rankServerPort;
	// 983 (sessao 83): os 16 bytes de m_unusedB8 do 645 nao existem; m_lobbyLoginTime +0xb8 (005a27c3), m_roomDetail
	// +0xbc (00ac3760, 00ace8ce), tabela em +0xc8 com 22 entradas (0062b3d0, 00502ed0); m_serverTime segue em +0x180.
	unsigned long m_lobbyLoginTime;
	sRoomDetail m_roomDetail;
	sGameTypeInfo m_gameTypeInfo[GAME_TYPE_MAX];
	unsigned char m_unused150[8];
	SYSTEMTIME m_serverTime;
	std::string m_paymentKID983; // +0x190: KID da pagina de pagamento JP (OnUnderBar_CookieUp 006585c5, C28); era m_unusedString168
	unsigned long m_unused184;
	std::string m_unusedString188;
	int m_provType;
	std::string m_commandLineID;
	int m_gachaTicket[2]; // 983: 0x1ec/0x1f0, sem o byte de 0x1c4
	std::list<unsigned long> m_refreshGuidList;
	std::list<unsigned short> m_refreshCountList;
	CItemManager m_itemManager;
	sMyInfo m_myInfo; // 983: 0x7a0..0x36f2; o bloco de 0x400 que vinha depois saiu
	std::map<unsigned int, std::string> m_stringMap;
	std::map<unsigned int, sCharacterInfo> m_charMap;
	std::map<unsigned int, sCaddieInfo> m_caddieMap;
	std::map<unsigned int, sMascotInfo> m_mascotMap;
	std::list<sItemInfo> m_partsList;
	std::map<unsigned int, sItemInfo> m_clubSetMap;
	std::list<sItemInfo> m_ballList;
	std::list<sItemInfo> m_myItemList;
	std::list<sRecordedItemInfo> m_recordedItemList;
	std::list<sItemInfo> m_cadItemList;
	std::list<sItemInfo> m_setItemList;
	// 983: entre m_setItemList (0x3780) e m_specialTrophyList (0x37a4) há só 2 listas, as duas com nó de 0xcc.
	// PROVISÓRIO: quais duas das quatro do 645 ficaram não foi medido; as de presente viram estáticas só para o
	// resto do CSharedDoc cair no lugar (rever em giftalarmdlg/realmyroomtask do 983).
	static std::list<sGiftInfo> m_giftList;
	static std::list<sGiftInfo> m_sendGiftList;
	std::list<sMailInfoBrief> m_mailList;
	std::list<sItemInfo> m_cutinList;
	std::list<sSpecialTrophy> m_specialTrophyList;
	std::list<sGuildTrophy> m_guildTrophyList;
	std::list<sItemInfo> m_auxPartList;
	std::list<sItemInfo> m_questDropList;
	std::list<sFurniture_List> m_furnitureList;
	std::list<sCards> m_cardList;
	// tmpl order: the original instantiates list<sGuildMatchFlagInfo> before list<sQuest> (see lobbytask)
	typedef char RbGmFlagListSize[sizeof(std::list<sGuildMatchFlagInfo>)];
	std::list<sSCardAvilityPeriodInfo> m_cardAbilityList[4];
	std::list<sSCardAvilityPeriodInfo> m_cardPeriodList; // 983: 0x381c (push_back 00641410, CCardManager::CalcCardPeriodAndStatus983 0080a810)
	// B97/C40.1: +0x3840 = contagens das missões (sQuestCount983, 0x10) e +0x384c = as missões (_quest::CQuestItem, 0x108),
	// ver include/jp/questitem.h.
	// 983: 0x3828; contagens de item da categoria 0xb (CItemUpdater::NewItemCount983 0058a880 / AddPart983 0058b600; My Room 0061c482; pacote 0x25d tipos 4/5)
	std::list<sItemCount983> m_itemCounts983;
	std::list<sQuest> m_questList; // 983: 0x3834 (missões do My Room 0061c56b; ClearInventory 00647800)
	std::list<_quest::sQuestCount983> m_questCounts983; // 983: 0x3840; contadores das missões (uid -> contagem; CQuestItem::GetCounterCount983 005e6680, Daily Quest)
	std::list<_quest::CQuestItem> m_quests983; // 983: 0x384c; missões ativas (pacote 0x21e; achadas por GetUid983)
	void AddQuestCount983(const _quest::sQuestCount983& count); // 00641470: m_questCounts983.push_back fora de linha
	void AddQuest983(const _quest::CQuestItem& item); // 00642940 (C42.2): m_quests983.push_back fora de linha (pacote 0x21e)
	__int64 m_cookie;
	__int64 m_pointShopTP983; // 983 +0x3860: saldo de pontos (TP) da Point Shop; pacotes 0x1e8 (009d2ecd) e 0x1e9 (009d2fb0) gravam o dword; zerado junto com m_cookie/m_cash
	__int64 m_cash;
	__int64 m_bonusCash;
	std::list<sItemInfo> m_uccItemList;
	std::list<sItemInfo> m_uccAztecList;
	std::map<unsigned long, sRoomSlot> m_roomSlotMap;
	std::map<unsigned long, unsigned char> m_indexMap;
	sUserInfo m_userInfo[4];
	sPangYaUserStatistics m_holeStatistics[4];
	std::vector<unsigned long> m_acquireItemList[4];
	std::vector<sRivalData> m_rivalList;
	std::vector<sCaddieReportData> m_caddieReportList;
	int m_hioJackpotMode983; // 983 +0x1044c: 1o int do pacote 0x18 do Hole-in-One (0057dce0); 1 = icone item_JackpotPang no resultado (00aea120/00aef160)
	std::vector<sGuildMatchup> m_guildMatchupList;
	std::list<sGuildMatchFlagInfo> m_guildMatchFlagList;
	unsigned char m_guildRefreshData[0x1c];
	bool m_bGameOver;
	int m_gameMode;
	unsigned char m_holeType;
	unsigned long m_roomType;
	unsigned char m_eventType;
	unsigned char m_eventFlag;
	sAwardItem m_awardItem[12];
	unsigned char m_courseMap[18];
	unsigned char m_pinIndex[18];
	unsigned char m_holeOrder[19];
	unsigned long m_holeRandom[18];
	unsigned long m_approachStartTime;
	float m_pangRate;
	int m_expRate;
	bool m_bFastForward;
	sGolfGame m_golfGame;
	unsigned char m_holePar[18];
	struct
	{
		int score;
		__int64 pang;
	} m_teamResult[2];
	short m_guildScore[2];
	unsigned long m_guildRoundScore[2];
	unsigned long m_unused47bc;
	unsigned long m_teamPang[2];
	unsigned long m_unused47c8;
	bool m_bShopBuyDone983; // +0x106fc: 983 so grava 1 (construtor 0064aba3, compra ok 005e22a0); ninguem le (A686)
	unsigned long m_unused47cc;
	unsigned char m_unused47d0;
	unsigned long m_replayTargetUID;
	sRealMyRoomAuthority m_rmrAuthority;
	bool m_bTreasureResult;
	bool m_bTreasureAward;
	sPangYaUserStatistics m_oldSeasonStatistics;
	sTrophyStatistics m_oldSeasonTrophy;
	std::list<sSpecialTrophy> m_oldSpecialTrophyList;
	std::list<sGuildTrophy> m_oldGuildTrophyList;
	std::list<sSpecialTrophy> m_specialTrophyList983; // 983: lista de sSpecialTrophy (pacote, kind 0), nó de 0x14 (tipo provisório)
	sRoomInfo m_roomInfo;
	std::map<unsigned long, sBriefUserInfo> m_briefUserInfoMap;
	std::list<sRoomInfo> m_roomList;
	std::list<sChannelInfo> m_channelList;
	std::list<sSlotInfo> m_slotList;
	std::list<sSlotInfo> m_slotList2;
	std::list<sGameServerInfo> m_gameServerList;
	std::set<unsigned long> m_failedGameServerIds983; // 983: +0x10a20, ids de servidor de jogo que falharam (CProjectG::OnGameServerResult 00a67600 insere m_curGameServer.id); a lista de servidores (pacote 0x2, CLobbyTask) os pula; limpo no login e ao escolher canal
	std::vector<sGameServerInfo> m_gameServerVector;
	std::list<sRoomUserInfo> m_roomUserList;
	const char* m_shopName;
	unsigned long m_underBarMask;
	sChannelInfo m_curChannel;
	sGameServerInfo m_curGameServer;
	std::list<unsigned long> m_guildMemberList;
	std::map<int, GuildMemberInfo_t> m_guildMemberMap;
	bool m_bAutoRefresh;
	bool m_bShowNotice;
	bool m_bBackgroundVideo;
	bool m_bLobbyBgCameraSway983; // 983: +0x10b27; liga o balanço da câmera do fundo do lobby (lido em 00ab5be0); ClearLobbyStateVars 006425e0 = 1, RFLobbyDesktop::OnDestroy 005f7080 = 1, jogador do lobby aparece 00566390 = 0 (+ DefaultCamera)
	bool m_bReturnLobbyNotice;
	bool m_bRebuildUnderBar;
	sChannelInfo m_selChannel;
	std::map<unsigned long, sFriend> m_friendMap;
	sFriend m_selFriend;
	ChatManager m_chatManager;
	std::list<sChatLine> m_chatLineList;
	std::list<std::string> m_chatHistory;
	std::list<std::string> m_ignoreList;
	unsigned long m_lobbyExitReason;
	std::list<std::string> m_recentWhisperList;
	std::list<std::string> m_reservedChatList;
	unsigned long m_maxWhisperPartner;
	std::string m_whisperPartner;
	unsigned long m_chatMode;
	std::list<sNoteInfo> m_noteList;
	std::string m_chatModeName;
	// PROVISÓRIO: no 983 não está mais no CSharedDoc; fica estático só para o resto cair no lugar
	static std::string m_packetNickname;
	static std::string m_nicknameSuffix;
	unsigned long m_unused10d24; // 983: sem uso (o 645 tinha m_controlServerService aqui)
	unsigned __int64 m_controlServerService; // 983: 0x10d28 (máscara de serviços, 0062ba40)
	int m_offlinePlay;
	unsigned long m_pcBang;
	unsigned long m_roomSetupStage;
	unsigned long m_serverProperty;
	unsigned char m_startHole;
	bool m_bTutorialResume;
	unsigned char m_newRecordState;
	bool m_bNewMapEvent;
	unsigned long m_newMapEventMask;
	bool m_bBingoEvent;
	unsigned long m_newRecordCourse;
	unsigned long m_tutorialComplete[3];
	unsigned long m_tutorialStartMode;
	std::string m_giftName;
	int m_giftIndex;
	bool m_bRefreshCamera;
	bool m_bMessengerAlarm;
	bool m_bMessengerInit;
	struct
	{
		unsigned long exp, totalExp;
	} m_levelTable[71];
	struct
	{
		unsigned long exp, pang;
	} m_bonusPangTable[3];
	unsigned long m_teamPlayerGuid[2];
	unsigned long m_guildBonusPang;
	unsigned long m_guildPoint;
	unsigned char m_guildTeam;
	std::vector<sGuildJoinEntry983> m_guildJoinList983; // 983 +0x10fe8: pedidos de entrada (caso 0x116, FrGuildManageDlg)
	std::vector<sGuildInvite983> m_guildInvites983; // 983 +0x10ff8: convites recebidos (0x30/0x127, caso 0x118, mensageiro)
	std::string m_authTicket;
	CGolfDoc* m_pGolfDoc;
	// PROVISÓRIO: no 983 não está mais no CSharedDoc; fica estático só para o resto cair no lugar
	static unsigned long m_cardPackPageNum;
	static unsigned long m_cardPackSelPageNum;
	static unsigned long m_cardPackCurPage;
	static unsigned long m_cardPackMode;
	static bool m_bCardPackRebuild;
	static unsigned char m_cardPackSlot;
	static unsigned char m_cardPackItem;
	static unsigned short m_rmrTabValue;
	static bool m_bRmrTabPending;
	unsigned long m_pendingMenu;
	unsigned long m_unused5060;
	unsigned long m_specialShotMark;
	bool m_bToppageAd;
	unsigned long m_toppageFeature;
	sUserEquip m_realEquip;
	std::list<sMapEventInfo> m_mapEventList;
	std::list<CGolfRule::sShotData> m_shotDataList;
	bool m_bItemStoragePin;
	bool m_bItemStorageLock;
	bool m_bTutorialRedirect;
	CPartTidList m_partTidList;
	unsigned long m_auxPartTidList[5];
	bool m_bHaveAngelWing;
	std::string m_loginAuthKey;
	unsigned short m_securityKey;
	unsigned long m_flagAbilityItem;
	unsigned char m_numBonusPangItem;
	eStateTrade m_stateTrade;
	std::list<sTradeItem> m_listTradeItem;
	std::list<sItemAttachCard> m_listAttachCard;
	unsigned long m_tradeUID;
	std::string m_tradeTitle;
	int m_countVisitor;
	// 983 (sessao 99, C28): sem m_tradeMode; o byte +0x11214 e o m_rmrMode (00651800/00651810 Set/Get, 00657e70 grava 5,
	// 0065c0d0 grava 2, 0065c290 grava 1; ninguem le +0x11215). PROVISORIO: estatico para a loja offline do 645 compilar.
	static unsigned char m_tradeMode;
	// PROVISÓRIO: no 983 não está mais no CSharedDoc; fica estático só para o resto cair no lugar
	static sStall m_stallItem;
	static std::list<sStall> m_listStallItem;
	static std::list<sSaleItem> m_maketList;
	unsigned char m_rmrMode;
	__int64 m_tradeIncome;
	void SetHaveHalloweenItem(bool bHave) { m_bHaveHalloweenItem = bHave; }
	void SetWearHalloweenItem(bool bWear) { m_bWearHalloweenItem = bWear; }
#ifdef REBANG_TASKMAIN_PCH
	bool IsHaveHalloweenItem();
#else
	bool IsHaveHalloweenItem() { return m_bHaveHalloweenItem; }
#endif
	bool IsWearHalloweenItem() { return m_bWearHalloweenItem; }
	bool m_bHaveHalloweenItem;
	bool m_bWearHalloweenItem;
	void SetReplayState(int state) { m_replayState = state; }
	int GetReplayState() { return m_replayState; }
	int m_replayState;
	int m_replayPlayType;
	eReplayModeType m_playingMode;
	char m_fileName[32];
	unsigned long m_bReturnToLogin983; // 983: byte 1 = volta ao servidor de login (msg 0x2f0 do CTaskMain; CLoginUnit manda 0xb, FrLoginDlg zera)
	std::string m_loginToken983; // 983: texto do pacote 3 do login/0x1d4 (00ac23dc, 006481a0) reenviado no login do servidor de jogo (00a519af); construtor 0064a657
	unsigned long m_replayPosition; // 983: o construtor zera 0x11270 com fldz/fstp (talvez float)
	unsigned long m_replayDuration;
	unsigned long m_replayTick;
	int m_replayShotIndex;
	unsigned char m_replayHoleStroke;
	unsigned long m_replayVersion;
	unsigned long m_serverTimeTick; // 983: 0x11288
	CPeriodContents* m_pPeriodDoc;
	int m_goWithMode;
	int m_goWithFriendUID;
	sRoomInfo m_inviteRoomInfo;
	unsigned char m_directMoveType;
	unsigned short m_directMoveRoom;
	unsigned long m_directMoveUID;
	unsigned long m_friendGameSvrUID;
	int m_inviteMode;
	unsigned long m_unused5cb0;
	std::map<unsigned long, sUserInfoTime> m_userInfoTimeMap;
	int m_userInfoMod;
	int GetUserInfoMod() { return m_userInfoMod; }
	void SetUserInfoMod(int mod) { m_userInfoMod = mod; }
	unsigned char m_forceNicknameChange;
	std::map<unsigned char, sMapStatistics> m_pastMapStat[4];
	std::map<unsigned long, sShopSaleState983> m_shopSaleState983; // 983: pacote 0x20e (009d3801), tid -> {tid, estado de venda} (63f280); construtor 00644280, nó de 0x1c (par de 12 B)
	std::map<unsigned long, IFF_ITEM_COMMON> m_promoItems983; // 983: itens do Item.iff com DateFlag 2 (promocao), TypeId -> IFF_ITEM_COMMON; refeito em 00643920 (CProjectG::Ready); construtor 00643410, nó de 0xd4 (par de 196 B)
	std::map<unsigned long, IFF_ITEM_COMMON> m_promoItemsNow983; // 983: os de m_promoItems983 dentro de saleStart..saleEnd na hora do servidor (00642a30); 0063b960 sorteia um (CShopMain msg M983(0xc6) param1 5), 006301d0 = nao vazio
	bool m_bBongdariShopOpen; // 983: loja do bongdari aberta (o m_bBongdariShop2 do CTaskMain do 645): grava 008bb130, zera 006559b0/ClearCommonVars
	bool m_goStopMode; // 983: 0x11411 (construtor 0064ac54)
	std::map<std::string, std::list<std::string> > m_visGroupMap; // 983: 0x11414 (2 votos)
	void SetAppGiftDownloadState(eAppGiftDownloadState state)
	{
		m_appGiftDownloadState = state;
	}

	eAppGiftDownloadState GetAppGiftDownLoadState() const
	{
		return m_appGiftDownloadState;
	}
	bool IsOpenFirstLoginAlarm() const { return m_bNeedFirstLoginAlarm; }
	void SetNeedFirstLoginAlarm(bool bNeed) { m_bNeedFirstLoginAlarm = bNeed; }
	eAppGiftDownloadState m_appGiftDownloadState;
	unsigned char m_unused5d0c;
	std::list<sCardStack> m_cardStackList;
	std::list<sBurnningSpCard> m_burnningSpCardList;
	bool m_bNeedFirstLoginAlarm; // 983: 0x11444
	// 983: cauda nova até 0x11644. Só os tamanhos e os WCrypticValue (construtor 0062bfc0) são medidos.
	unsigned char m_pad983_11445[3]; // 983: alinhamento depois do bool; sem acesso a +0x11445..+0x11447 (out/A839/scan_11440.txt)
	unsigned long m_playerCutinItem983[4]; // 983: 0x11448 (0062be40 zera, 0062be60 grava, 0062be80 le; por jogador)
	bool m_bChannelLogin; // 983: 0x11458; ctor 0064ab19/0064ab4f liga com id de canal (@hg/@gr/@lv) ou "-gpwccp" na linha de comando
	unsigned long m_holePrizeItem983[3]; // 983: 0x1145c; typeid do premio do buraco (pacote 0xab/conteudo 0x92); 00683610: [0] prize.spr, [1] spr do item de evento
	unsigned char m_holePrizeXmas983[3]; // 983: 0x11468; 1 = xmas_prize.spr no premio [i]
	std::map<int, int> m_mapEvent983; // 983: evento por mapa (0063f830; valor = ponteiro do registro, +0xc = taxa de pang em %); construtor 00643490, nó de 0x18 (par de 8 B)
	sd_shop::CCommodityList m_sdCommodityList983; // 983: 0x1147c; mercadorias do Self Design (pacote so do 983 009d3354; loja usa so com m_listVersion != 0); construtor 00b3d270 (0x30 bytes)
	float m_courseScale983[3]; // 983: 0x114ac; 00643830 = 3/0/1; 0046f540 (DigHolecup/HoleProcess): [0] escala no mapa 0xb (taskmanager = tentativas*3), [2] taxa do Grand Prix (realGameType 0x14/0x15)
	void* m_auxPartValues983[4]; // 983: 0x114b8 (0062bea0 grava; vector<sAuxpartValue>* = &CConditionalAuxpartObject::m_applied por jogador)
	WCrypticValue<unsigned char> m_holeOrderCrypt983[19]; // 983: 0x114c8; m_holeOrder (+0x1051e) cifrado, 1 registro por buraco (00636c60 copia; 00633560 le pelo nÂº do buraco 1..19: WPolySoup/treasure/golfrule)
	WCrypticValue<unsigned char> m_fixHole983; // 983: 0x115f8; buraco fixo da sala de repeticao (1..0x12; 0xff = nenhum; 00633580 le, 00636c40 grava; pacote da sala holeType 4)
	unsigned char m_bFixHoleMoveTee983; // 983: 0x11608; buraco fixo (holeType 4): botao Move Tee preso (OnMoveTeeBtnUp 00677250); IsHoleInOne983 006a6f20 le
	unsigned char m_bFixHoleRepeat983; // 983: 0x11609; 1 = repeticao do single no buraco fixo (OnSingleModeStartResult 00abad60); 00abae5b manda Encode4 7 (senao 6)
	unsigned char m_bPremiumEndNotice983; // 983: 0x1160a; aviso de fim do premium pendente (taskmanager grava; CLobbyMain/CGolfRule mostram e zeram)
	unsigned char m_pad983_1160b;
	std::string m_premiumEndNoticeText983; // 983: 0x1160c; texto do aviso (GetGameString 0x13882 com a data local)
	unsigned long m_itemUpdater983[2]; // CItemUpdater (Update983) (construtor 00589b50)
	unsigned long m_roomListKind983; // 983: 0062bf40/0062bf50; 0 = salas comuns "roomlist", 1 = Grand Prix "grandprix_list" (CLobbyMain HandleMsg 0x8b caso 4 00abcb01)
	unsigned long m_roomListRequest983; // 983: 0062bf60/0062bf70; aba pedida no FrRoomListDlg (1 = GP -> pacote 0x176, 0 = salas -> 0x81; CLobbyMain msg 0x84), 2 = nenhum
	unsigned long m_lobbyState983; // 983: 0062bf80/0062bf90 (GetState983); 1 = lista de salas, 2 = lista do GP, 3/4/7 = dentro de sala, 8 = FrGameRoomDlg
	unsigned long m_gpChannel983; // 983: 0062bf20/0062bf30; canal do Grand Prix (0 = comum; 0x63 sem faixa no NormalLobbyRoom)
	bool m_bPangyaBySet983; // 983: 0x11640; 1 = servidor ecoou o 0x199 (PangYa saiu pela correcao do conjunto, CGolfRule::GetBarCondition 00675160); 0x52 zera; sem leitor no 983
	void SetPangyaBySet983(bool value); // 0062bfa0 (C42.1): pacote 0x52 do OnPacketCommon chama com 0
	bool IsPangyaBySet983() const; // 0062bfb0 (sem chamador no 983; o campo so e gravado)
};

#ifdef REBANG_TASKMAIN_PCH
// HACK: taskmain_pch.h only. In-class inline accessors of the original shareddoc.h, in its order
// (taskmain.obj has its own copies); the shared PCH keeps them out of line (IL offsets, canaries).
inline void CSharedDoc::SetRmrMode(unsigned char mode)
{
	m_rmrMode = mode;
}
inline unsigned char CSharedDoc::GetRmrMode()
{
	return m_rmrMode;
}
inline unsigned long CSharedDoc::GetFlagBlock()
{
	return m_myInfo.info.dwFlagBlock;
}
inline bool CSharedDoc::IsHaveHalloweenItem()
{
	return m_bHaveHalloweenItem;
}
inline eReplayModeType CSharedDoc::GetPlayingMode()
{
	return m_playingMode;
}
inline void CSharedDoc::SetPlayingMode(eReplayModeType mode)
{
	m_playingMode = mode;
}
inline int CSharedDoc::GetNowGoWithMode()
{
	return m_goWithMode;
}
inline int CSharedDoc::GetGoWithFriendUID()
{
	return m_goWithFriendUID;
}
#endif

int OnlinePlay();
bool SetCurMap(unsigned char map);
unsigned char GetCurMap();

// HACK: IsMyTurn lives in the PCH (upstream: mousecursor.cpp), before PLAYER and MyGuid, so it
// waits for their wave (mousecursor emission order); placed right before ILFILL3, which pays for
// its IL. golfdoc emits it between its own templates and the PCH-instantiated ones: the dead
// reference makes it wait there for the wave of CGolfDoc::ApproachDataAllClear (no effect in
// other units).
inline CSharedDoc* Doc();
inline CGolfDoc* GOLFDOC();
inline sPlayerData* PLAYER(unsigned char index);
inline unsigned long MyGuid(bool bGallery);

inline bool IsMyTurn(bool check)
{
	if (0)
		GOLFDOC()->ApproachDataAllClear();
	switch (Doc()->m_gameMode)
	{
	case 0:
		if (WSingleton<WNetworkSystem>::Instance()->IsConnected(
				WNetworkSystem::NET_GAME))
		{
			bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
			if (bGM)
				return check;
			return PLAYER(GOLFDOC()->m_currentPlayer)->oid == MyGuid(false);
		}
		break;
	case 5:
		if (PLAYER(GOLFDOC()->m_currentPlayer)->oid == MyGuid(false))
			return true;
		return false;
	}
	return true;
}

ILFILL3

#include "golftask.h"
#include "icontentsdatacontainer.h"
class CContentsDoc;
// HACK!
typedef char CONTENTSDOC_SINGLETON_SIZE[sizeof(WSingleton<CContentsDoc>)];
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
#ifdef REBANG_GOLFRULE_PCH
inline char* CSharedDoc::GetFileName()
{
	return m_fileName;
}
#endif
inline int CSharedDoc::GetReplayPlayType()
{
	return m_replayPlayType; // 645: +0x5b98 cru (s88)
}
inline void CSharedDoc::SetReplayPlayType(int t)
{
	m_replayPlayType = t; // 645: +0x5b98 cru (s88)
}
inline void CSharedDoc::SetGoStopMode(bool b)
{
	m_goStopMode = b; // 645: +0x5d04 cru; 983 +0x11411
}
inline bool CSharedDoc::GetGoStopMode()
{
	return m_goStopMode; // 645: +0x5d04 cru; 983 +0x11411
}
#endif
#include "shareddoc.inl"
