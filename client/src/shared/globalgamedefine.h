#pragma once
#include "vc10_layout.h"

enum eGameType
{
	GAME_TYPE_STROKE,
	GAME_TYPE_TEAM,
	GAME_TYPE_AVATARCHAT,
	GAME_TYPE_MATCH,
	GAME_TYPE_30S,
	GAME_TYPE_30S_TEAM,
	GAME_TYPE_GUILD_MATCH,
	GAME_TYPE_SKINS,
	GAME_TYPE_REALMYROOM,
	GAME_TYPE_APPROACH,
	GAME_TYPE_NEW_APPROACH,
	// JP 983: 22 tipos (GetGameTypeName 00502ed0: `cmp cl, 0x16`); numeros pela ordem de gravacao do
	// CSharedDoc::InitGameType 0062b3d0 (tabela em +0xc8, passo 8). No 645 o tutorial era 11/12, o fantasma 13 e
	// o USEMAX 14. Os *_UNK983_* nao tem entrada na tabela (nome meu).
	GAME_TYPE_HOLEINONE983, // 11: texto "hole in one" (00d0ac34), 100 jogadores, 1 buraco
	GAME_TYPE_UNK983_12,
	GAME_TYPE_UNK983_13,
	GAME_TYPE_UNK983_14,
	GAME_TYPE_TUTORIAL_BASIC, // 15
	GAME_TYPE_TUTORIAL_ADV, // 16
	GAME_TYPE_OFFLINE_GHOST, // 17
	GAME_TYPE_USEMAX, // 18
	GAME_TYPE_UNK983_19,
	GAME_TYPE_GRANDPRIX983, // 20: texto GetGameString(0x7543); 0046e6d0 trata 0x14 e 0x15 como Grand Prix
	GAME_TYPE_GRANDPRIX983_2, // 21
	GAME_TYPE_MAX // 22
};

// 983 0046e6d0 (C42.3): tipo real de sala 0x14..0x15 = Grand Prix (fora de linha; 10 chamadas: golftask, lobbytask, taskmanager)
bool __fastcall IsGrandPrixGameType983(int realGameType);

enum eGameStyle
{
	GAME_STYLE_NORMAL,
	GAME_STYLE_SPECIAL1,
	GAME_STYLE_SPECIAL2
};

enum eLevel
{
	ROOKIE_F,
	ROOKIE_E,
	ROOKIE_D,
	ROOKIE_C,
	ROOKIE_B,
	ROOKIE_A,
	BEGINNER_E,
	BEGINNER_D,
	BEGINNER_C,
	BEGINNER_B,
	BEGINNER_A,
	JUNIOR_E,
	JUNIOR_D,
	JUNIOR_C,
	JUNIOR_B,
	JUNIOR_A,
	SENIOR_E,
	SENIOR_D,
	SENIOR_C,
	SENIOR_B,
	SENIOR_A,
	AMATUER_E,
	AMATUER_D,
	AMATUER_C,
	AMATUER_B,
	AMATUER_A,
	SEMI_PRO_E,
	SEMI_PRO_D,
	SEMI_PRO_C,
	SEMI_PRO_B,
	SEMI_PRO_A,
	PRO_E,
	PRO_D,
	PRO_C,
	PRO_B,
	PRO_A,
	NATIONAL_PRO_E,
	NATIONAL_PRO_D,
	NATIONAL_PRO_C,
	NATIONAL_PRO_B,
	NATIONAL_PRO_A,
	WORLD_PRO_E,
	WORLD_PRO_D,
	WORLD_PRO_C,
	WORLD_PRO_B,
	WORLD_PRO_A,
	MASTER_E,
	MASTER_D,
	MASTER_C,
	MASTER_B,
	MASTER_A,
	TOP_MASTER_E,
	TOP_MASTER_D,
	TOP_MASTER_C,
	TOP_MASTER_B,
	TOP_MASTER_A,
	WORLD_MASTER_E,
	WORLD_MASTER_D,
	WORLD_MASTER_C,
	WORLD_MASTER_B,
	WORLD_MASTER_A,
	LEGEND_E,
	LEGEND_D,
	LEGEND_C,
	LEGEND_B,
	LEGEND_A,
	INFINITY_LEGEND_E,
	INFINITY_LEGEND_D,
	INFINITY_LEGEND_C,
	INFINITY_LEGEND_B,
	INFINITY_LEGEND_A,
	MAX_LEVELS,
};

enum eTitle
{
	TITLE_LEVEL,
	TITLE_JJANG,
	TITLE_OLD_JJANG,
	TITLE_CYBER_CHAMPIONSHIP,
	TITLE_WCG_CHAMPIONSHIP,
	TITLE_PCBANG_CHAMPIONSHIP,
	TITLE_SQUAD,
	TITLE_OFFICIAL,
	TITLE_CHICK = 15,
	TITLE_PANGYASHOT = 21,
	TITLE_STRAIGHTROAD,
	TITLE_GREENMASTER,
	TITLE_COURSEMASTER,
	TITLE_GOLD,
	TITLE_SILVER,
	TITLE_BRONZE,
	TITLE_MANNERANGEL
};

enum GUILD_CLASS_IDX
{
};

enum GUILD_STATE_IDX
{
};

#pragma pack(push, 1)
struct GUILD_HISTORY
{
	int index;
	unsigned long guildUID;
	char guildName[21];
	GUILD_STATE_IDX state;
	_SYSTEMTIME time;

	GUILD_HISTORY()
		: index(-1), guildUID(0), state((GUILD_STATE_IDX)0)
	{
		memset(guildName, 0, sizeof(guildName));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GUILD_USER_INFO
{
	unsigned long guildUID;
	char guildName[21];
	int guildPang;
	int guildPoint;
	int memberCount;
	char guildMark[12];
	char notice[101];
	char introduce[101];
	GUILD_CLASS_IDX classIdx;
	unsigned long masterUID;
	char masterNickname[22];

	GUILD_USER_INFO()
		: guildUID(0),
		  guildPang(0),
		  guildPoint(0),
		  memberCount(0),
		  classIdx((GUILD_CLASS_IDX)0),
		  masterUID(-1)
	{
		memset(guildName, 0, sizeof(guildName));
		memset(guildMark, 0, sizeof(guildMark));
		memset(notice, 0, sizeof(notice));
		memset(introduce, 0, sizeof(introduce));
		memset(masterNickname, 0, sizeof(masterNickname));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GUILD_INFO : public GUILD_USER_INFO
{
	_SYSTEMTIME createTime;

	GUILD_INFO() { }
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GUILD_LIST
{
	unsigned long guildUID;
	char guildName[21];
	int guildPang;
	int guildPoint;
	int memberCount;
	_SYSTEMTIME createTime;
	char introduce[101];
	unsigned long srvDword9a_645; // +0x9a do pacote 645 0x1bb (lista de guildas, sem par no 983); sem leitor no nosso
	unsigned long masterUID;
	char masterNickname[22];
	char guildMark[12];

	GUILD_LIST()
		: guildUID(0),
		  guildPang(0),
		  guildPoint(0),
		  memberCount(0),
		  srvDword9a_645(0),
		  masterUID(-1)
	{
		memset(guildName, 0, sizeof(guildName));
		memset(guildMark, 0, sizeof(guildMark));
		memset(introduce, 0, sizeof(introduce));
		memset(masterNickname, 0, sizeof(masterNickname));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GUILD_USER_LIST
{
	unsigned long guildUID;
	unsigned long userUID;
	GUILD_CLASS_IDX classIdx;
	char message[25];
	char guildName[21];
	char nickname[22];
	unsigned char online;

	GUILD_USER_LIST()
		: guildUID(0), userUID(-1), classIdx((GUILD_CLASS_IDX)0), online(0)
	{
		memset(nickname, 0, sizeof(nickname));
		memset(guildName, 0, sizeof(guildName));
		memset(message, 0, sizeof(message));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct GUILD_USER_MSN_LIST
{
	unsigned long userUID;
	unsigned long guildUID;
	unsigned char sex;
	char userID[22];
	char nickname[22];
	unsigned char srvByte35_645;  // +0x35 do pacote 645 0x38/0x39 (amigos da guilda, sem par no 983); sem leitor no nosso
	unsigned char online;

	GUILD_USER_MSN_LIST()
		: userUID(-1), guildUID(0), sex(0), srvByte35_645(0), online(0)
	{
		memset(nickname, 0, sizeof(nickname));
		memset(userID, 0, sizeof(userID));
	}
};
#pragma pack(pop)

struct Delete_Object
{
	template <typename T>
	void operator()(T ptr) const
	{
		delete ptr;
		ptr = NULL;
	}
};

struct Delete_SecondObject
{
	template <typename T>
	void operator()(T& pair) const
	{
		delete pair.second;
		pair.second = NULL;
	}
};

namespace GlobalEnum
{
#pragma pack(push, 1)
	struct sMissionInfo
	{
		int condition;
		bool complete;

		sMissionInfo()
			: condition(0), complete(false)
		{
		}
	};
#pragma pack(pop)
}

namespace GlobalEnum
{
	struct sBingoInfo
	{
		unsigned long type;
		int tryCount;
		unsigned long aimedPoint;
		unsigned long completeLine;
	};
}

#pragma pack(push, 1)
struct sTreasureHunt
{
	unsigned long dwUID;
	unsigned long dwTid;
	unsigned short wCount;
	unsigned char byItemType;

	sTreasureHunt()
		: dwUID(0), dwTid(0), wCount(0), byItemType(0)
	{
	}

	sTreasureHunt(const sTreasureHunt& rhs)
	{
		dwUID = rhs.dwUID;
		dwTid = rhs.dwTid;
		wCount = rhs.wCount;
		byItemType = rhs.byItemType;
	}

	void operator=(const sTreasureHunt& rhs)
	{
		dwUID = rhs.dwUID;
		dwTid = rhs.dwTid;
		wCount = rhs.wCount;
		byItemType = rhs.byItemType;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sTreasureGift
{
	unsigned long dwUID;
	unsigned long dwTid;
	unsigned long dwItemGuid;
	unsigned short wCount;
	unsigned char byItemType;
	int iHourRemain;
	unsigned short reserved;

	sTreasureGift()
		: dwUID(0),
		  dwTid(0),
		  dwItemGuid(0),
		  wCount(0),
		  byItemType(0),
		  iHourRemain(0),
		  reserved(0)
	{
	}

	void operator=(const sTreasureGift& rhs)
	{
		dwUID = rhs.dwUID;
		dwTid = rhs.dwTid;
		dwItemGuid = rhs.dwItemGuid;
		wCount = rhs.wCount;
		byItemType = rhs.byItemType;
		iHourRemain = rhs.iHourRemain;
		reserved = rhs.reserved;
	}

	void Reset()
	{
		dwUID = 0;
		dwTid = 0;
		dwItemGuid = 0;
		wCount = 0;
		byItemType = 0;
		iHourRemain = 0;
		reserved = 0;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sTreasureItem
{
	unsigned char byMapIndex;
	unsigned long dwGauge;

	sTreasureItem()
		: byMapIndex(0), dwGauge(0)
	{
	}
};
#pragma pack(pop)

struct stGameClass
{
	eGameStyle style;
	unsigned char gameType;

	stGameClass(eGameStyle s, unsigned char type)
	{
		style = s;
		gameType = type;
	}
};

enum eMapType
{
	MAP_BLUE_LAGOON = 0,
	MAP_BLUE_WATER = 1,
	MAP_SEPIA_WIND = 2,
	MAP_WIND_HILL = 3,
	MAP_WIZ_WIZ = 4,
	MAP_WEST_WIZ = 5,
	MAP_BLUE_MOON = 6,
	MAP_SILVIA_CANNON = 7,
	MAP_ICE_CANNON = 8,
	MAP_WHITE_WIZ = 9,
	MAP_SHINING_SAND = 10,
	MAP_PINK_WIND = 11,
	MAP_MAP_12 = 12,
	MAP_DEEP_INFERNO = 13,
	MAP_ICE_SPA = 14,
	MAP_LOST_SEAWAY = 15,
	MAP_EASTERN_VALLEY = 16,
	MAP_MAP_17 = 17,
	MAP_MAP_18 = 18,
	MAP_WIZ_CITY = 19,
	MAP_MAP_20 = 20,
	MAP_MAP_21 = 21,
	MAP_RANDOM = 127,
};

struct EncryptKey
{
	unsigned char m_byKey[16];

	EncryptKey()
	{
		memset(m_byKey, 0, sizeof(m_byKey));
		for (int i = 0; i < 16; i++)
			m_byKey[i] = (unsigned char)(rand() % 255);
	}
	~EncryptKey() { }

	unsigned char GetKey(int index) { return m_byKey[index]; }
};

#pragma pack(push, 1)

struct sRoomDetail
{
	unsigned char holes;
	unsigned long gameTime;
	unsigned char map;
	unsigned char gameType;
	unsigned char holeOrder;
	unsigned long matchTypeId;
};

struct sPangYaUserInfo
{
	char sID[22];
	char sNick[22];
	char sGuild[20]; // 983: 20 (conv_ghostdata_to_userinfo 00828130: py_strcpy 00827f70 zera 0x14 em info+0x2c)
	char szEmblemName[12]; // 983: info+0x40 (00827fb0 em ebx+0x42; FrMessengerDlg le Doc+0x7e2)
	unsigned char pad983_4c[32]; // 983: +32 entre szEmblemName e school; vem do servidor na copia do info (0x12d bytes, 0063e1a4/006ac191) mas nada le: Doc+0x7ee/+0x38fe e sMyInfo+0x4e sem acesso (D942)
	unsigned long school;
	unsigned long dwIdentity;
	unsigned long dwGalleryGuid;
	unsigned long dwGuid;
	unsigned long dwRank[3];
	unsigned long dwGuildId;
	unsigned long dwEmblemVer;
	// 983: 2 bits a mais no inicio (bit 3 = gender 00ac207c, bits 5..7 = asas 00856de4, conv_ghostdata 00828130)
	unsigned char guildGrade : 2; // 983 00901a95/009c83a3: & 3 == 1 ou 2
	unsigned char DoTutorial : 1;
	unsigned char gender : 1;
	unsigned char manner : 1;
	unsigned char angelicWings : 1;
	unsigned char angelicWingsEffect : 1;
	unsigned char devilWings : 1;
	unsigned char RookieFEvent : 2;
	unsigned char NewYearEvent : 1;
	unsigned char Reserved : 5;
	unsigned char pad983_92; // 983: +1 entre o bitfield (+0x90) e iBongdariShop (+0x93); vem do servidor na copia do info, mas nada le info+0x91/+0x92 (Doc+0x833/0x834, +0x3943/0x3944; ghost 00828130 so +0x90) (D943)
	short iBongdariShopUsableTimes;
	short iBongdariShopBonusTimes;
	short iBongdariShopRemainedBonus;
	unsigned long dwPointPointEvent;
	unsigned long dwFlagBlock;
	int iTimeBlock;
	int nChannelingFlag;
	char sDisplayID[128];
	unsigned long dwUID;

	bool IsIdentity(unsigned long identity)
	{
		return (dwIdentity & identity) ? true : false;
	}
};

struct sPangYaUserStatistics
{
	unsigned long dwDrive;
	unsigned long dwPutt;
	unsigned long dwVisitTime;
	unsigned long dwShotTime;
	float fLongest;
	unsigned long dwPangya;
	unsigned long dwTimeOut;
	unsigned long dwOB;
	unsigned long dwDistance;
	unsigned long dwHole;
	unsigned long dwMatchHole;
	unsigned long wHoleInOne; // 983: 4 bytes (006a69d0: add dword em +0x2c)
	unsigned short wBunker;
	unsigned long dwFairway;
	unsigned long wAlbatross; // 983: 4 bytes (006a69d0: add dword em +0x36)
	unsigned long dwHoleIn;
	unsigned long dwPuttIn;
	float fLongestPuttIn;
	float fLongestChipIn;
	unsigned long dwExp;
	unsigned char Level;
	__int64 i64Pang;
	int iTotalScore;
	char cBestScore[6];
	__int64 i64MaxPang[6];
	unsigned long dwGameCount;
	unsigned long dwDisconnectCount;
	unsigned long dwTeamWin;
	unsigned long dwTeamGames;
	unsigned long dwLadderPoint;
	unsigned long dwLadderWin;
	unsigned long dwLadderLose;
	unsigned long dwLadderDraw;
	unsigned long dwLadderHoles;
	unsigned int nMannerCombo;
	unsigned int nMaxMannerCombo;
	unsigned long dwNoMannerGameCount;
	__int64 i64TotalSkinsPang;
	unsigned long dwSkinsWin;
	unsigned long dwSkinsLose;
	unsigned long dwSkinsGames;
	int iStrikePoint;
	unsigned short iAllInCount;
	unsigned long eventValue;
	unsigned long eventFlag;
	unsigned long dwSeasonCount;
	// 983 00644610 (CSharedDoc::BuildTikiReportList): 6 contadores de premios (awardFlag bits 0..5) que ficaram com o
	// amigo selecionado (m_selFriend), stat+0xe7 passo 4; era i64SumPang + unk983_tail[26] (sem leitor no nosso nem no 983)
	unsigned long awardCountFriend983[6];
	unsigned char pad983_ff[10]; // 983: stat+0xff..0x108 sem acesso (docrefs 9ce a); vem no pacote, servidor pode mandar 0
};

struct sTrophyStatistics
{
	unsigned short Trophy[13][3];
};

struct sUserEquip
{
	unsigned long guidCaddie;
	unsigned long guidChar;
	unsigned long guidClubSet;
	unsigned long tidBall;
	unsigned long tidItemSlot[10];
	unsigned long guidSkin[6];
	unsigned long tidSkin[6];
	unsigned long guidMascot;
	unsigned long poster[2]; // 983: +8 depois de guidMascot (nome provisório)
};

struct sMapStatistics
{
	unsigned char bMap;
	unsigned long dwDrive;
	unsigned long dwPutt;
	unsigned long dwHole;
	unsigned long dwFairway;
	unsigned long dwHoleIn;
	unsigned long dwPuttIn;
	int iTotalScore;
	char cBestScore;
	__int64 i64MaxPang;
	unsigned long tidChar;
	unsigned char eventScore;

	sMapStatistics() { bMap = 0xff; }
};

struct sCharacterInfo
{
	// 983: 513 bytes (nó do m_charMap = 0x218); tidParts em +12 e tidAuxParts em +420 (votos). A ordem dos campos
	// sem voto (os 4 bytes de +8, ItemIdList/UccIndexList, cutIn, PCL, mastery, cards) é provisória.
	unsigned long tid;
	unsigned long guid;
	unsigned char hairClr; // +8
	// 983 (s114): +9 e +0xa sao bytes inteiros (645: shirtsClr:4/gift_flag:4 num byte so). 00abbb90 grava +9 = cor da camisa
	// (byte alto da dword +4 do CPartTidList) e +0xa = 1; o construtor 00500230 zera +8..+0xb com uma dword.
	unsigned char shirtsClr; // +9
	unsigned char gift_flag; // +0xa
	char Purchase; // +0xb
	unsigned long tidParts[24];
	unsigned long ItemIdList[24];
	char UccIndexList[24][9];
	unsigned long tidAuxParts[5];
	unsigned long cutIn[4];
	char PCL[5];
	unsigned long mastery;
	unsigned long card[3][4];

	sCharacterInfo()
	{
		tid = 0;
		guid = 0;
		hairClr = 0;
		shirtsClr = 0;
		gift_flag = 0;
		Purchase = 0;
		memset(tidParts, 0, sizeof(tidParts));
		memset(PCL, 0, sizeof(PCL));
		memset(tidAuxParts, 0, sizeof(tidAuxParts));
		memset(ItemIdList, 0, sizeof(ItemIdList));
		memset(UccIndexList, 0, sizeof(UccIndexList));
		memset(cutIn, 0, sizeof(cutIn)); // 983 00500230
		mastery = 0;
		memset(card, 0, sizeof(card));
	}
};

struct sMyInfo
{
	unsigned short roomIndex;
	sPangYaUserInfo info;
	sPangYaUserStatistics stat;
	sTrophyStatistics trophy;
	sUserEquip userEquip;
	sMapStatistics mapStat[22]; // 983: 12 vetores de 22 mapas seguidos (009d16a6, 009c4304); indice por 00731d00
	sMapStatistics classicMapStat[22];
	sMapStatistics mapStat983[10][22]; // 983: vetores 2..11 de 00731d00 (tipo, lista): 2=0x33/1a 3=0xa/1a 4=5/2a 5=0/2a 6=0x33/2a 7=0xa/2a 8=0x34/1a 9=0xb/1a 10=0x34/2a 11=0xb/2a (mapStat = 5/1a, classicMapStat = 0/1a); 264 iniciados num laco so (00649a00)
};

struct sCaddieInfo
{
	unsigned long guid;
	unsigned long tid;
	unsigned long tidPart;
	unsigned char Level;
	unsigned long Exp;
	unsigned char gift_flag : 1;
	unsigned char Rent_flag : 1;
	unsigned short Remain_Date;
	unsigned short Remain_Partdate;
	char Purchase;
	unsigned char byCheckCaddieWarning;
	bool PCBangCaddie;
};

struct sClubInfo
{
	unsigned long guid;
	unsigned long tid;
	short PCL[5];
	short slotUp983[5]; // +0x12: nivel de cada slot do taco (983 0063e120 copia sItemInfo+0xaa; param.clubSlotUp em golfdoc/ghostmain, PlayerDlg)
};

struct sMascotInfo
{
	unsigned long guid;
	unsigned long tid;
	unsigned char Level;
	unsigned long Exp;
	char szMsg[30];
	unsigned char Remain_Date; // 983: 0x2b, 1 byte = tem prazo (GetNumAddItemSlotByMascot 0063dca0; o 645 tinha short)
	char Purchase;
	_SYSTEMTIME endDate; // 983: 0x2d
	bool PCBangMascot; // 983: 0x3d (FrMascotInfoDlg 00a806fc)
};

#define REBANG_983_PCBANG_MASCOT(info) false // 983: rever FrMascotInfoDlg no 983

struct sUserInfo : public sMyInfo
{
	sCharacterInfo charInfo;
	sCaddieInfo caddieInfo;
	sClubInfo clubInfo;
	sMascotInfo mascotInfo;
};

struct sSpecialTrophy
{
	unsigned long guid;
	unsigned long tid;
	unsigned long count;
};

struct sGuildTrophy
{
	unsigned long TypeID;
};

// 983: 0x8eb (construtor 006443b0, destrutor 00644460); 22 mapas e as listas de troféus em std::vector (o 645 tinha 20 mapas e
// contagem + ponteiro). A 3ª lista é nova no 983 (pacote 0x257, tipo 0).
struct sSeasonRecord
{
	sPangYaUserStatistics userStat;
	sMapStatistics mapStat[22];
	sMapStatistics classicMapStat[22];
	sTrophyStatistics trophyStat;
	std::vector<sSpecialTrophy> trophies;
	std::vector<sGuildTrophy> guildTrophies;
	std::vector<sSpecialTrophy> trophies983;

	sSeasonRecord()
	{
		memset(&userStat, 0, sizeof(userStat));
		memset(mapStat, 0, sizeof(mapStat));
		memset(classicMapStat, 0, sizeof(classicMapStat));
		memset(&trophyStat, 0, sizeof(trophyStat));
	}
};
C_ASSERT_VC10(sizeof(sSeasonRecord) == 0x8eb);
C_ASSERT(offsetof(sSeasonRecord, classicMapStat) == 0x4bb);
C_ASSERT(offsetof(sSeasonRecord, trophyStat) == 0x86d);
C_ASSERT(offsetof(sSeasonRecord, trophies) == 0x8bb);
C_ASSERT_VC10(offsetof(sSeasonRecord, guildTrophies) == 0x8cb);
C_ASSERT_VC10(offsetof(sSeasonRecord, trophies983) == 0x8db);

#pragma pack(pop)

#pragma pack(push, 1)
struct sUserInfoExt : public sUserInfo
{
	sUserInfoExt()
	{
		memset(&charInfo, 0, sizeof(charInfo));
		memset(&caddieInfo, 0, sizeof(caddieInfo));
		memset(&clubInfo, 0, sizeof(clubInfo));
		memset(&mascotInfo, 0, sizeof(mascotInfo));
		memset(&info, 0, sizeof(info));
		memset(&stat, 0, sizeof(stat));
		memset(&trophy, 0, sizeof(trophy));
		memset(&userEquip, 0, sizeof(userEquip));
		memset(mapStat, 0, sizeof(mapStat));
		memset(ext, 0, sizeof(ext));
	}

	unsigned char ext[0x400];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sUserMatchHistory
{
	int nGender;
	char szNickName[22];
	char szLoginID[22];
	unsigned long dwUID;

	sUserMatchHistory()
		: nGender(0), dwUID(0)
	{
		memset(szNickName, 0, sizeof(szNickName));
		memset(szLoginID, 0, sizeof(szLoginID));
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sLevelUpDone
{
	unsigned char bDone;
	unsigned char bLevel;
	unsigned char bItemType;

	sLevelUpDone()
		: bDone(0), bLevel(0), bItemType(0)
	{
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sBriefUserInfo
{
	unsigned long dwUid;
	unsigned long dwGuid;
	unsigned short roomIndex;
	char sNick[22];
	unsigned char level;
	unsigned long dwIdentity;
	unsigned long dwTitle;
	unsigned long dwLadderPoint;
	unsigned char bSleep : 1;
	unsigned char gender : 3;
	unsigned char manner : 1;
	unsigned char angelicWings : 1;
	unsigned char Reserve : 2;
	unsigned long m_GuildId;
	unsigned long dwEmblemVer983; // 983: numero do emblema, logo depois de m_GuildId (00af9bd4); state em +0x42
	char szEmblemName[12];
	unsigned short state;
	int nChannelingFlag;
	unsigned char reserved[0x80];

	bool IsIdentity(unsigned long identity)
	{
		return (dwIdentity & identity) ? true : false;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sUserPosition
{
	unsigned short iRoomIdx;
	int iRoomType;
	int iServerGUID;
	unsigned char iChannelUid;
	char csChannelName[64];

	sUserPosition()
	{
		iRoomIdx = 0xffff;
		iRoomType = -1;
		iServerGUID = -1;
		iChannelUid = 0xff;
		memset(csChannelName, 0, sizeof(csChannelName));
	}
};

// 983: 0x3b3c (nó do map<ulong,sUserInfoTime> = push 0x3b50 em 00632547; construtor 00644f10, destrutor 00644a00). As listas de
// troféus saíram do meio (o 645 tinha contagem + ponteiro depois do info) e viraram std::vector no fim; a 3ª é nova (0x257, tipo 5).
struct sUserInfoTime
{
	unsigned long dwUpdate;
	sUserInfo info;
	unsigned long dwGuildPang;
	unsigned long dwGuildPoint;
	sSeasonRecord oldSeason;
	sUserPosition userPosition;
	unsigned long hasSeasonData;
	std::vector<sSpecialTrophy> trophies;
	std::vector<sGuildTrophy> guildTrophies;
	std::vector<sSpecialTrophy> trophies983;

	sUserInfoTime()
	{
		dwUpdate = 0;
		dwGuildPang = 0;
		dwGuildPoint = 0;
		hasSeasonData = 0;
		memset(&info, 0, sizeof(info));
		memset(&userPosition, 0, sizeof(userPosition));
	}
};
C_ASSERT_VC10(sizeof(sUserInfoTime) == 0x3b3c);
C_ASSERT(offsetof(sUserInfoTime, dwGuildPang) == 0x31ca);
C_ASSERT(offsetof(sUserInfoTime, oldSeason) == 0x31d2);
C_ASSERT_VC10(offsetof(sUserInfoTime, userPosition) == 0x3abd);
C_ASSERT_VC10(offsetof(sUserInfoTime, hasSeasonData) == 0x3b08);
C_ASSERT_VC10(offsetof(sUserInfoTime, trophies) == 0x3b0c);
C_ASSERT_VC10(offsetof(sUserInfoTime, trophies983) == 0x3b2c);

struct sItemInfo
{
	union
	{
		unsigned long guid;
		unsigned long id;
	};
	// _FindItem (wtemplate.h) names the type id typeId.
	union
	{
		unsigned long tid;
		unsigned long typeId;
	};
	int HourRemain;
	short Common[5];
	char Purchase;
	unsigned char gift_flag : 1;
	unsigned char Expired : 1;
	unsigned char ItemFlag : 2;
	unsigned char ItemType : 3;
	_SYSTEMTIME ItemDate;
	unsigned char Reserve : 1;
	unsigned char IsValid : 1;
	char ItemName[41];
	char UccIndex[9];
	unsigned char status;
	unsigned short Seq;
	char CopierNick[22];
	unsigned long attachCard[12];
	unsigned short CharacterSlotNum;
	unsigned short CaddieSlotNum;
	unsigned short NpcSlotNum983; // +0xa8: 3a contagem de slots (983 007ff6c0: sPart+0x22c)
	short slotUp983[5]; // +0xaa: nivel ganho em cada slot no Club Workshop (0063e120 -> sClubInfo::slotUp983; 0x216 sub 0xcc)
	int clubExp983; // +0xb4: experiencia do taco (FrClubSetWorkshopSelector; 0x216 sub 0xcc)
	int recoveryUsed983; // +0xb8: cancelamentos ja usados ("%d/%d" do Club Workshop; 0x216 sub 0xcc)
	int workshopSrvByte983; // +0xbc: byte do servidor (0x216 sub 0xcc, Decode1 -> workshop.srvByte983); ctor 00478d60 poe -1; o 983 so copia, nao le (D951)
	int workshopSrvInt983; // +0xc0: dword do servidor (0x216 sub 0xcc, Decode4 -> workshop.srvInt983); ctor 00478d60 poe 0; o 983 so copia, nao le (D951)

	sItemInfo()
	{
		gift_flag = 0;
		Expired = 0;
		ItemFlag = 0;
		ItemType = 0;
		Reserve = 0;
		IsValid = 0;
		guid = 0;
		tid = 0;
		HourRemain = 0;
		Purchase = 0;
		memset(Common, 0, sizeof(Common));
		memset(&ItemDate, 0, sizeof(ItemDate));
		memset(ItemName, 0, sizeof(ItemName));
		memset(UccIndex, 0, sizeof(UccIndex));
		memset(CopierNick, 0, sizeof(CopierNick));
		memset(attachCard, 0, sizeof(attachCard));
		CharacterSlotNum = 0;
		CaddieSlotNum = 0;
		status = 0;
		Seq = 0;
		NpcSlotNum983 = 0; // 983 00478d60
		memset(slotUp983, 0, sizeof(slotUp983));
		clubExp983 = 0;
		recoveryUsed983 = 0;
		workshopSrvByte983 = -1;
		workshopSrvInt983 = 0;
	}

	bool operator==(const unsigned long& type) const { return tid == type; }
};

#pragma pack(pop)

#pragma pack(push, 1)
struct sGuildRoomInfo
{
	unsigned long nGuildID[2];
	char szEmblemName[2][12]; // 983: emblemas antes dos nomes (Reset(i) em 005092a5: +8/+0x14)
	short nEmblemVer983[2]; // 983: número (versão?) do emblema de cada time, 2º argumento de GetEmblem983
	char szName[2][20]; // 983: +0x24/+0x38; o total segue 76 (Reset zera 76 bytes só com dwords)

	void Reset()
	{
		memset(nGuildID, 0, sizeof(nGuildID));

		memset(szEmblemName, 0, sizeof(szEmblemName));
		memset(nEmblemVer983, 0, sizeof(nEmblemVer983));
		memset(szName, 0, sizeof(szName));
	}

	void Reset(int index)
	{
		if (index >= 2)
			return;

		nGuildID[index] = 0;
		nEmblemVer983[index] = 0;
		szName[index][0] = 0;
		szEmblemName[index][0] = 0;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sRoomInfo
{
	char title[64]; // 983: 0x40 sem senha (CLobbyTask::OnPacket 0x48, 00ac0ea5: memset 0x40, strncpy 0x3f)
	bool bPublic;
	bool bAvailable;
	bool bIntrusion;
	unsigned char nUserLimit;
	unsigned char nUserNum;
	EncryptKey RoomKey;
	unsigned char nGalleryNum;
	unsigned char nGalleryLimit;
	unsigned char nHole;
	unsigned char gameType;
	unsigned short roomGuid;
	unsigned char holeType;
	unsigned char mapType;
	unsigned long shotTimeLimit;
	unsigned long gameTimeLimit;
	unsigned long tidMatch;
	bool bSleep;
	bool bAdminMaster;
	sGuildRoomInfo GuildInfo;
	unsigned long pangMultiply;
	unsigned long expMultiply;
	unsigned char gpHost983; // +0xbf: anfitriao do Grand Prix (icone gp_host, sala da lista)
	int masterUID; // 983: +0xc0 (CLobbyTask::OnPacket le +0x109b4; nosso estava em +0xbf)
	unsigned char realGameType;
	// 983: 24 bytes zerados no construtor. Nomes medidos no 983 (A197, docs/LAYOUTS.md); ext983[i] = os mesmos 6 dwords (so para zerar).
	union
	{
		unsigned long ext983[6];
		struct
		{
			unsigned long artifactTid983;   // +0xc5: artefato da sala (CGrandPrixArtifactRule::IsTimed 00546900; inicio do jogo 009c2913)
			unsigned long roomFlags983;      // +0xc9: bit 0 classico, bit 1 shortgame (00b30560/00b30570; OnPacket 0x48 00ac0dd6)
			unsigned long gpTypeId983;       // +0xcd: tipo do Grand Prix (CTaskMain::HandleMsgCommon 0065d995; FrGrandPrixAwardDlg 0054d1cd)
			unsigned long gpTicket983;       // +0xd1: ticket do GP (HasGrandPrixSpecialHole983 em 00511da7 e 0072f32f)
			unsigned long gpStartValue983;   // +0xd5: copiado para m_golfGame.gpTimeLimit983 (Doc+0x106a4) no inicio do jogo (009c2958)
			unsigned long gpRoomFlags983;     // +0xd9: bit 0 = sala de Grand Prix (CGrandPrixDocument::Open983 00559700 filtra m_roomList por node+0xe1 & 1)
		};
	};

	sRoomInfo()
	{
		title[0] = 0;

		bAvailable = false;
		tidMatch = 0;
		bSleep = false;
		bAdminMaster = false;

		realGameType = 0;
		memset(ext983, 0, sizeof(ext983));
		roomGuid = 0xffff;
		bPublic = true;

		GuildInfo.Reset();
	}

	bool operator==(const unsigned long& number) const
	{
		return roomGuid == number;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sSlotInfo
{
	unsigned long dwGuid;
	char sNick[22];
	char sGuild[20]; // 983: 21 -> 20 (connectionRank em +0x2e)
	unsigned char connectionRank;
	unsigned long dwIdentity;
	unsigned long dwTitle;
	unsigned long tidChar;
	unsigned long tidSkin[6];
	unsigned char bTeam : 2;
	unsigned char bSleep : 1;
	unsigned char bMaster : 2;
	unsigned char gender : 3;
	unsigned char manner : 1;
	unsigned char bReady : 1;
	unsigned char stateReserved : 6;
	unsigned char level;
	unsigned char angelicWings : 1;
	unsigned char angelicWingsEffect : 1;
	unsigned char Reserved : 6;
	unsigned char ladderGrade;
	unsigned long GuildId;
	char szEmblemName[12];
	unsigned long dwEmblemVer983; // 983: 4 bytes novos (dwUserUID em +0x6c)
	unsigned long dwUserUID;
	unsigned long state;
	unsigned short subRoomIndex;
	unsigned long action;
	float location[3];
	unsigned long iStateTrade;
	char strTradeTitle[64];
	unsigned long tidMascot;
	unsigned char pangma[2];
	unsigned long nChannelingFlag;
	char sDisplayID[128];
	unsigned char IsInvite : 1;
	unsigned char guestReserved : 7;
	float rankTieBreak983;     // 983: construtor 00500360; vai p/ sRivalData+0x1b0 (golfrule30battle) e o único leitor é o desempate do rank (shareddoc: menor fica na frente)
	unsigned char bGrandPrixAI983;   // 983: slot de NPC do Grand Prix (gameroomdlg AddAI: 1)
	unsigned short grandPrixAIId983; // 983: id do NPC (GetNpcId983 / FindGrandPrixAI983)

	sSlotInfo()
	{
		dwUserUID = 0;

		dwGuid = 0;

		level = 0;
		tidChar = 0;
		connectionRank = 0;

		bTeam = 0;
		bSleep = 0;
		bMaster = 0;
		gender = 0;
		dwIdentity = 0;

		manner = 0;
		nChannelingFlag = 0;
		dwTitle = 0x30000000;
		IsInvite = 0;

		memset(sDisplayID, 0, sizeof(sDisplayID));

		memset(szEmblemName, 0, sizeof(szEmblemName));

		memset(tidSkin, 0, sizeof(tidSkin));

		rankTieBreak983 = 0.0f; // 983
		bGrandPrixAI983 = 0;
		grandPrixAIId983 = 0;
	}

	bool operator==(const unsigned long& id) const { return dwGuid == id; }

	bool IsIdentity(unsigned long flag)
	{
		return dwIdentity & flag ? true : false;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sChannelInfo
{
	char Name[64];
	unsigned short Max_Num;
	unsigned short Current_Num;
	unsigned char Uid;
	unsigned long Type;
	int Property;
	unsigned long MinLevel; // 983: +0x4d, canal fechado abaixo deste nivel (DrawChannelButton 00af633a)
	unsigned long MaxLevel; // 983: +0x51, canal fechado acima deste nivel

	bool operator==(const unsigned char& channel) const
	{
		return Uid == channel;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sGameServerInfo
{
	char name[40];
	unsigned long id;
	int maxUser;
	int curUser;
	char addr[18];
	int port;
	unsigned long property;
	int angelicWingsNum;
	unsigned long eventFlags;
	unsigned long eventValue;
	unsigned short iconIndex;

	sGameServerInfo()
	{
		name[0] = 0;
		addr[0] = 0;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sFriend
{
	char NickName[22];
	char szAlias[11];
	unsigned long Uid;
	unsigned long Guid;
	unsigned long Server;
	unsigned long dwGuildId;
	unsigned long emblemVer983; // 983: versao do emblema da guilda (GetEmblem983; zerado no Clear 0062f457)
	char szEmblemName[12];
	sUserPosition userPosition;

	unsigned char State;
	unsigned char Channel;
	unsigned char GameLevel;
	unsigned char Gender : 1;
	unsigned char IsLogOn : 1;
	unsigned char IsAccept : 1;
	unsigned char IsAgree : 1;
	unsigned char IsBlock : 1;
	unsigned char IsPlay : 1;
	unsigned char IsDive : 1;
	unsigned char IsBusy : 1;
	unsigned char PangyaFriend : 1;
	unsigned char GuildFriend : 1;
	unsigned char IsBlocked : 1;
	unsigned char Reserved : 5;

	sFriend() { Clear(); }

	void Clear()
	{
		memset(NickName, 0, sizeof(NickName));
		memset(szAlias, 0, sizeof(szAlias));
		Uid = 0xffffffff;

		Guid = 0xffffffff;
		Server = 0;

		dwGuildId = 0xffffffff;
		emblemVer983 = 0;

		memset(szEmblemName, 0, sizeof(szEmblemName));

		memset(userPosition.csChannelName, 0,
			sizeof(userPosition.csChannelName));
		userPosition.iChannelUid = 0xff;

		userPosition.iRoomType = 0xffffffff;
		userPosition.iServerGUID = 0xffffffff;

		GameLevel = 0;

		Gender = 0;
		IsLogOn = 0;
		IsAccept = 0;
		IsAgree = 0;
		IsBlock = 0;
		IsPlay = 0;
		IsDive = 0;
		IsBusy = 0;

		PangyaFriend = 0;
		GuildFriend = 0;
		IsBlocked = 0;
		Reserved = 0;
		userPosition.iRoomIdx = 0xffff;
		State = 5;
		Channel = 0xff;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sNoteInfo
{
	unsigned long uid;
	unsigned short reserved;
	char sNick[22];
	char sNote[64];
	char sTime[16];
	bool bReply;

	sNoteInfo() { clear(); }

	void clear()
	{
		uid = 0xffffffff;
		reserved = 0;
		memset(sNick, 0, sizeof(sNick));
		memset(sNote, 0, sizeof(sNote));
		memset(sTime, 0, sizeof(sTime));
		bReply = false;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sMailIncludeItem
{
	unsigned long dwIDX;
	unsigned long dwTID;
	unsigned char btItemType;
	int iCount;
	int iTimeCount;
	short soCom[5];
	unsigned short CharacterSlotNum;
	unsigned short CaddieSlotNum;
	unsigned short NpcSlotNum983; // 983: +0x1f, de sPart::NpcSlot983 (005b6801/005b5a5f); sizeof 0x37 como o 983
	unsigned long dwSetTID;
	int iSetItemCount;
	char csUCCIndex[9];
	unsigned long reserved30;
	unsigned char reserved34;

	sMailIncludeItem()
	{
		dwIDX = 0xffffffff;
		dwTID = 0xffffffff;
		btItemType = 0; // 983 005b33d9

		iCount = 0;
		iTimeCount = 0;
		soCom[0] = soCom[1] = soCom[2] = soCom[3] = soCom[4] = 0;
		CharacterSlotNum = 0;
		CaddieSlotNum = 0;
		NpcSlotNum983 = 0;
		dwSetTID = 0xffffffff;
		iSetItemCount = 0;
		memset(csUCCIndex, 0, sizeof(csUCCIndex));
		reserved30 = 0;
		reserved34 = 0;
	}
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sMailInfoBrief
{
	unsigned long id;
	char sender[22];
	char reserved[0x6e];
	bool bRead;
	int itemCount;
	sMailIncludeItem item;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sGiftInfo
{
	unsigned long guid;
	unsigned long tid;
	unsigned short Arg0;
	char sFromID[128];
	char sMsg[80];
	char sDate[32];
	unsigned char ItemType;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sMailInfo
{
	unsigned long dwIndex;
	char szFromNick[22];
	char szDate[32];
	char szContent[800];
	bool bHaveItem;
	std::list<sMailIncludeItem> m_lMailItemList;
	bool bRead;

	sMailInfo()
	{
		dwIndex = 0xffffffff;
		memset(szFromNick, 0, sizeof(szFromNick));
		memset(szDate, 0, sizeof(szDate));
		memset(szContent, 0, sizeof(szContent));
		bHaveItem = false;
		m_lMailItemList.clear();
	}
};
#pragma pack(pop)

class CMapTypeAdapter
{
public:
	enum eMapIndex
	{
		MI_NONE = -1,
		MI_BLUE_LAGOON = 0,
		MI_BLUE_WATER = 1,
		MI_SEPIA_WIND = 2,
		MI_WIND_HILL = 3,
		MI_WIZ_WIZ = 4,
		MI_WEST_WIZ = 5,
		MI_BLUE_MOON = 6,
		MI_SILVIA_CANNON = 7,
		MI_ICE_CANNON = 8,
		MI_WHITE_WIZ = 9,
		MI_SHINING_SAND = 10,
		MI_PINK_WIND = 11,
		MI_MAP_12 = 12,
		MI_DEEP_INFERNO = 13,
		MI_ICE_SPA = 14,
		MI_LOST_SEAWAY = 15,
		MI_EASTERN_VALLEY = 16,
		MI_MAP_18 = 18,
		MI_WIZ_CITY = 19,
		MI_MAP_20 = 20,
		MI_MAP_21 = 21,
		MI_RANDOM = 127,
	};

	enum eMapFlag
	{
		MF_NONE = 0,
		MF_BLUE_LAGOON = 0x1,
		MF_BLUE_WATER = 0x2,
		MF_SEPIA_WIND = 0x4,
		MF_WIND_HILL = 0x8,
		MF_WIZ_WIZ = 0x10,
		MF_WEST_WIZ = 0x20,
		MF_BLUE_MOON = 0x40,
		MF_SILVIA_CANNON = 0x80,
		MF_ICE_CANNON = 0x100,
		MF_WHITE_WIZ = 0x200,
		MF_SHINING_SAND = 0x400,
		MF_PINK_WIND = 0x800,
		MF_DEEP_INFERNO = 0x1000,
		MF_ICE_SPA = 0x2000,
		MF_LOST_SEAWAY = 0x4000,
		MF_EASTERN_VALLEY = 0x8000,
		MF_WIZ_CITY = 0x40000,
	};

	static eMapFlag GetMapFlag(eMapType type)
	{
		eMapFlag flag;
		switch (type)
		{
		case MAP_BLUE_LAGOON:
			flag = MF_BLUE_LAGOON;
			break;
		case MAP_BLUE_WATER:
			flag = MF_BLUE_WATER;
			break;
		case MAP_SEPIA_WIND:
			flag = MF_SEPIA_WIND;
			break;
		case MAP_WIND_HILL:
			flag = MF_WIND_HILL;
			break;
		case MAP_WIZ_WIZ:
			flag = MF_WIZ_WIZ;
			break;
		case MAP_WEST_WIZ:
			flag = MF_WEST_WIZ;
			break;
		case MAP_BLUE_MOON:
			flag = MF_BLUE_MOON;
			break;
		case MAP_SILVIA_CANNON:
			flag = MF_SILVIA_CANNON;
			break;
		case MAP_ICE_CANNON:
			flag = MF_ICE_CANNON;
			break;
		case MAP_WHITE_WIZ:
			flag = MF_WHITE_WIZ;
			break;
		case MAP_SHINING_SAND:
			flag = MF_SHINING_SAND;
			break;
		case MAP_PINK_WIND:
			flag = MF_PINK_WIND;
			break;
		case MAP_DEEP_INFERNO:
			flag = MF_DEEP_INFERNO;
			break;
		case MAP_ICE_SPA:
			flag = MF_ICE_SPA;
			break;
		case MAP_LOST_SEAWAY:
			flag = MF_LOST_SEAWAY;
			break;
		case MAP_EASTERN_VALLEY:
			flag = MF_EASTERN_VALLEY;
			break;
		case MAP_WIZ_CITY:
			flag = MF_WIZ_CITY;
			break;
		}

		return flag;
	}

	static eMapFlag GetMapFlag(eMapIndex index)
	{
		eMapFlag flag = MF_NONE;

		switch (index)
		{
		case MI_BLUE_LAGOON:
			flag = MF_BLUE_LAGOON;
			break;
		case MI_BLUE_WATER:
			flag = MF_BLUE_WATER;
			break;
		case MI_SEPIA_WIND:
			flag = MF_SEPIA_WIND;
			break;
		case MI_WIND_HILL:
			flag = MF_WIND_HILL;
			break;
		case MI_WIZ_WIZ:
			flag = MF_WIZ_WIZ;
			break;
		case MI_WEST_WIZ:
			flag = MF_WEST_WIZ;
			break;
		case MI_BLUE_MOON:
			flag = MF_BLUE_MOON;
			break;
		case MI_SILVIA_CANNON:
			flag = MF_SILVIA_CANNON;
			break;
		case MI_ICE_CANNON:
			flag = MF_ICE_CANNON;
			break;
		case MI_WHITE_WIZ:
			flag = MF_WHITE_WIZ;
			break;
		case MI_SHINING_SAND:
			flag = MF_SHINING_SAND;
			break;
		case MI_PINK_WIND:
			flag = MF_PINK_WIND;
			break;
		case MI_MAP_12:
			flag = MF_NONE;
			break;
		case MI_DEEP_INFERNO:
			flag = MF_DEEP_INFERNO;
			break;
		case MI_ICE_SPA:
			flag = MF_ICE_SPA;
			break;
		case MI_LOST_SEAWAY:
			flag = MF_LOST_SEAWAY;
			break;
		case MI_EASTERN_VALLEY:
			flag = MF_EASTERN_VALLEY;
			break;

		case MI_WIZ_CITY:
			flag = MF_WIZ_CITY;
			break;
		}

		return flag;
	}

	static eMapIndex GetMapIndex(eMapType type)
	{
		switch (type)
		{
		case MAP_BLUE_LAGOON:
			return MI_BLUE_LAGOON;
		case MAP_BLUE_WATER:
			return MI_BLUE_WATER;
		case MAP_SEPIA_WIND:
			return MI_SEPIA_WIND;
		case MAP_WIND_HILL:
			return MI_WIND_HILL;
		case MAP_WIZ_WIZ:
			return MI_WIZ_WIZ;
		case MAP_WEST_WIZ:
			return MI_WEST_WIZ;
		case MAP_BLUE_MOON:
			return MI_BLUE_MOON;
		case MAP_SILVIA_CANNON:
			return MI_SILVIA_CANNON;
		case MAP_ICE_CANNON:
			return MI_ICE_CANNON;
		case MAP_WHITE_WIZ:
			return MI_WHITE_WIZ;
		case MAP_SHINING_SAND:
			return MI_SHINING_SAND;
		case MAP_PINK_WIND:
			return MI_PINK_WIND;
		case MAP_MAP_12:
			return MI_MAP_12;
		case MAP_DEEP_INFERNO:
			return MI_DEEP_INFERNO;
		case MAP_ICE_SPA:
			return MI_ICE_SPA;
		case MAP_LOST_SEAWAY:
			return MI_LOST_SEAWAY;
		case MAP_EASTERN_VALLEY:
			return MI_EASTERN_VALLEY;
		// 983 008381e0: 0x12, 0x14 e 0x15 tambem sao identidade (0x11 nao)
		case MAP_MAP_18:
			return MI_MAP_18;
		case MAP_WIZ_CITY:
			return MI_WIZ_CITY;
		case MAP_MAP_20:
			return MI_MAP_20;
		case MAP_MAP_21:
			return MI_MAP_21;
		case MAP_RANDOM:
			return MI_RANDOM;
		default:
			return MI_NONE;
		}
	}
};

struct sPouchPrize
{
	unsigned long dwTid;
	unsigned long dwGuid;
	int iCount;
	unsigned long dwFlag1;
	unsigned long dwFlag2;

	sPouchPrize() { }
};

#pragma pack(push, 1)
struct sApproachResultData
{
	bool bExit;
	unsigned long dwGUID;
	unsigned long dwRealUID;
	unsigned char byRank;
	unsigned long dwPrizeCount;
	unsigned long dwRemainDistance;
	unsigned long dwRemainTime;
	unsigned char byRankPrize;
	unsigned char byLuckPrize;

	sApproachResultData& operator=(sApproachResultData& rhs)
	{
		bExit = rhs.bExit;
		dwGUID = rhs.dwGUID;
		dwRealUID = rhs.dwRealUID;
		byRank = rhs.byRank;
		dwPrizeCount = rhs.dwPrizeCount;
		dwRemainDistance = rhs.dwRemainDistance;
		dwRemainTime = rhs.dwRemainTime;
		byRankPrize = rhs.byRankPrize;
		byLuckPrize = rhs.byLuckPrize;
		return *this;
	}
};
#pragma pack(pop)

enum eCUTINANISTLYE
{
	CUTIN_NONE,
	CUTIN_TORIGHT,
	CUTIN_TOLEFT,
	CUTIN_ZOOM_IN,
	CUTIN_ZOOM_OUT,
	CUTIN_FADE_IN,
	CUTIN_FADE_OUT,
	CUTIN_ROTATE,
	CUTIN_UNTIL,
	CUTIN_REVERSE,
};

enum eCUTINANI_DRAWPOS
{
	CUTIN_DP_NONE,
	CUTIN_DP_CENTER,
	CUTIN_DP_RIGHT,
	CUTIN_DP_LEFT,
	CUTIN_DP_TOP,
	CUTIN_DP_BOTTOM,
};

struct CUTIN_ANI_STYPEINFO
{
	eCUTINANI_DRAWPOS PosType;
	eCUTINANISTLYE CharType;
	eCUTINANISTLYE BgType;
	eCUTINANISTLYE PatternType;
	eCUTINANISTLYE TextType;
	eCUTINANISTLYE OutType;
	char szCharTex[40];
	char szBgTex[40];
	char szPattern[40];
	char szText[40];

	CUTIN_ANI_STYPEINFO() { Reset(); }

	void Reset()
	{
		PosType = CUTIN_DP_NONE;
		CharType = CUTIN_NONE;
		BgType = CUTIN_NONE;
		PatternType = CUTIN_NONE;
		TextType = CUTIN_NONE;
		OutType = CUTIN_NONE;

		memset(szCharTex, 0, sizeof(szCharTex));
		memset(szBgTex, 0, sizeof(szBgTex));
		memset(szPattern, 0, sizeof(szPattern));
		memset(szText, 0, sizeof(szText));
	}
};

enum GIMMICK_ITEM_TYPE
{
	GIMMICK_ITEM_ANY = -1,
	GIMMICK_ITEM_PANG,
	GIMMICK_ITEM_BOX,
};

#ifdef REBANG_GOLFRULE_PCH
// Extra prize kinds 3..5 get no acquisition effect (golfrule.obj keeps the
// out-of-line body of this helper between sItemInfo and FieldItem).
static inline bool IsSilentPrizeType(int type)
{
	if (type >= 3 && type <= 5)
		return true;
	return false;
}
#endif

namespace FieldItem
{
	enum eDisposeTextureType
	{
		DISPOSE_TEXTURE_COIN, // 983: ct_green_b = 0 (NeedToMakeTriPtArray 00907860)
		DISPOSE_TEXTURE_BOOSTER,
	};

	struct AcquiredItemInfo
	{
		GIMMICK_ITEM_TYPE type;
		unsigned long index;
		unsigned long count;
		eDisposeTextureType textureType;

		void EncodeTo(WSendPacket& packet) const

		{
			packet.Encode1((unsigned char)type);
			packet.Encode4(index);
			packet.Encode1((unsigned char)count);
			packet.Encode1((unsigned char)textureType);
		}
	};
}

struct GimmickDispositionInformation
{
	enum eFlag
	{
		GIMMICK_UNUSED,
		GIMMICK_USED,
		GIMMICK_ANY,
	};

	// 983: sizeof 0x24 (CGimmickContainer::LoadFromPacket 009103e0: DecodeBuffer de 0x24; o 645 tinha 0x14)
	GIMMICK_ITEM_TYPE type;      // +0x00
	unsigned long index;         // +0x04
	eFlag flag;                  // +0x08
	unsigned long srvDword0c_983;  // +0x0c do servidor; 983 sem leitor (D955: so o default 0x16 em 009103e0/009106bd; o 645 punha 20)
	unsigned char srvByte10_983; // +0x10 do servidor; 983 sem leitor (D953: LoadFromPacket 009103e0 copia, 0085d6c0/0085e450/0085d780 nao leem)
	unsigned char hole;          // +0x11
	float posX983;               // +0x14 posicao do item (983 0085d6c0; 3 floats soltos: com array o cl poe cookie /GS no LoadFromPacket)
	float posY983;               // +0x18
	float posZ983;               // +0x1c
	unsigned long texType983;    // +0x20 FieldItem::eDisposeTextureType (983 0085d6c0 -> PlaceGroundItem; 4 = registro ignorado no LoadFromPacket)

	GimmickDispositionInformation()
		: type(GIMMICK_ITEM_ANY),
		  index(0),
		  flag(GIMMICK_UNUSED),
		  srvDword0c_983(0x16),
		  srvByte10_983(0),
		  hole(0),
		  posX983(0.0f),
		  posY983(0.0f),
		  posZ983(0.0f),
		  texType983(4)
	{
	}
};

#pragma pack(push, 1)
struct sSCardAvilityPeriodInfo
{
	unsigned long uid;
	unsigned long tid;
	unsigned long partsTid;
	unsigned long partsUid;
	int Avility;
	unsigned int AvilityValue;
	int slotNum;
	_SYSTEMTIME useStartTime;
	_SYSTEMTIME useEndTime;
	int cardType;
	unsigned char valid;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sCards
{
	unsigned long uid;
	unsigned long tid;
	unsigned long partTid;
	unsigned long partUid;
	int selectedCardSlotNum;
	int cardCount;
	_SYSTEMTIME useStartTime;
	_SYSTEMTIME useEndTime;
	unsigned char valid;
	unsigned char bUseDT;
};

namespace IFF_STRUCT
{
	struct sQuest;
}

struct sQuest
{
	unsigned long typeId;
	unsigned long count;
	IFF_STRUCT::sQuest* pQuest;
	unsigned char flag;
};

struct sItemAttachCard
{
	unsigned long typeId;
	unsigned long id;
};

struct sCaddieReportData
{
	unsigned long uid;
	__int64 pang;
	__int64 bonusPang;
	unsigned long roomType;
	unsigned long exp;
	unsigned long mascotTypeId;
	unsigned char premium;
	char pangItem;
	unsigned short level;
	unsigned char reserved24[2];
	char score;
	unsigned char awardFlag;
	unsigned char eventType;
	unsigned char reserved29[22];
	char nickname[22];
	unsigned char reserved55[4];
	unsigned long guildUID;
	unsigned long guildEmblemVer983; // 983: +0x5d, versao do emblema da guilda (BuildTikiReportList 00644610 -> sRivalData::guildEmblemVer983; C600)
	char guildMark[12]; // 983: +0x61
	unsigned char gameType; // 983: +0x6d
	unsigned char reserved6a[3];
	unsigned char team; // 983: +0x71
	unsigned char eventFlag; // 983: +0x72
	unsigned char reserved6f[16];
};
C_ASSERT(sizeof(sCaddieReportData) == 0x83); // 983: push_back 005a82e0 (passo 0x83), caso 0x11a 005a9270 (DecodeBuffer 0x83)

struct sRoomUserInfo
{
	unsigned long uid;
	unsigned char level;
	unsigned char hole : 5;
	unsigned char reserved05 : 3;
	unsigned long capability;
	unsigned char reserved0a[4];
	unsigned long ladderPoint;
};

struct sGuildMatchFlagInfo
{
	unsigned char reserved[0x32]; // 983: nó de 0x3c
};

struct sMapEventInfo
{
	unsigned char reserved[0x28];
};

struct sBurnningSpCard
{
	unsigned char reserved[0x14];
};

struct GuildMemberInfo_t
{
	unsigned char reserved[0x40];
};

struct sFurniture_List
{
	unsigned long id;
	unsigned long typeId;
	unsigned short reserved;
	float x;
	float y;
	float z;
	float r;
	unsigned char bArrange : 1;
};

struct sAwardItem
{
	unsigned long uid;
	int pang;
};

struct sRealMyRoomAuthority
{
	unsigned long ownerUID;
	unsigned short serverValue983; // +0x04: vem do 0x12b (983 009d1adf) e so volta ao servidor no bloco inteiro do 0xb8 (00610a8d); sem outro leitor
	bool bPrivate;
	char password[100];
};
#pragma pack(pop)
