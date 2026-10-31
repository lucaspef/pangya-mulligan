#pragma once

#include <bitset>
#include "encrypttypes.h"
#include "parttidlist.h"
#include <map>
#include <list>
#include "objectfactory.h"

class WPolySoup;
class CAutoTarget;
#include "../shared/globalgamedefine.h"
class CSky;
class WPuppet;

struct sProperty
{
	float bound;
	float roll;
	float min[3];
	float max[3];
	unsigned char pass;
	unsigned char ground;

	sProperty()
	{
		boundSound[0] = 0;
		rollSound[0] = 0;
	}
	~sProperty() { }

	char boundSound[64];
	char rollSound[64];
	char stepSound[64];
};

class CPropertyInfo
{
public:
	CPropertyInfo() { m_defaultIndex = -1; }
	~CPropertyInfo();

	void Init(const TiXmlDocument& doc);
	int GetPropertyIndex(const char* texture);
	const sProperty& GetProperty(int index);

private:
	std::vector<sProperty> m_property;
	std::map<std::string, int> m_textureIndex;
	int m_defaultIndex;
#ifndef REBANG_SEMANTIC_ONLY
public:
	// HACK: never defined; kept for the PCH name-count canaries (EH funclet holder of a former naked Init)
	static void __cdecl RbInitFunclets();
#endif
};

struct sWave
{
	char texture[32];
	float width;
	float freq;
};

class CWaveInfo
{
public:
	CWaveInfo() { }
	~CWaveInfo() { m_wave.clear(); }

	void Load(const char* filename);

	char* GetTexture(int index) { return m_wave[index].texture; }
	float GetWidth(int index) { return m_wave[index].width; }
	float GetFreq(int index) { return m_wave[index].freq; }

private:
	std::map<int, sWave> m_wave;
};

struct sRiver
{
	float scrollVel;
	float tileHead;
	float tileSide;
	float depth;
};

class CRiverInfo
{
public:
	CRiverInfo() { }
	~CRiverInfo() { m_river.clear(); }

	void Load(const char* filename);

	float GetScrollVelocity(int index) { return m_river[index].scrollVel; }
	float GetHeadTile(int index) { return m_river[index].tileHead; }
	float GetSideTile(int index) { return m_river[index].tileSide; }
	float GetDepth(int index) { return m_river[index].depth; }

private:
	std::map<int, sRiver> m_river;
};

class CMapInfo
{
public:
	void Init(const TiXmlDocument& doc, unsigned char index);
	const char* FindItem(const std::string& key);

private:
	std::map<std::string, std::string> m_item;
};

#ifdef REBANG_GOLFRULE_PCH
const float BAR_MIN = 105.0f; // golfrule: emitted before BAR_END
#endif
const float BAR_MIDDLE = 105.0f;
const float BAR_END = 500.0f;
const float BAR_MARGIN = 8.0f;
const float BAR_START = 140.0f;

struct sBar
{
	float power;
	float impact;
	float gauge;
	float gaugeStep;
	int grade;
	bool bFullPower;
	int dir;
	float center;
	float calipers;

	void Reset();
#ifdef REBANG_GOLFRULE_PCH
	float Persent() const;
	float Delta() const;
#endif
};

#pragma pack(push, 1)
struct sWinningPrize
{
	unsigned long holeInOne;
	unsigned long albatross;
	unsigned long eagle;
	unsigned long birdie;
	unsigned long par;
	unsigned long bogey;
	unsigned long pad983_18; // 983: 7 dwords (pacote 0xab copia 0x1c em 00635daa; AddPlayer 006a7470 e AddPrize 00683e0d zeram); +0x18 sem leitor
};

struct sPrizeInfo
{
	unsigned long typeId;
	unsigned short pad983_4; // sem leitor no nosso; copiado com o registro de 0x10
	unsigned short quantity;
	int type;
	unsigned long pad983_c; // 983: 0x10 por registro (pacote 0xcc copia em 00635eab); +0xc (disp 0x596) sem leitor no 983
};
#pragma pack(pop)

struct sPlayerData
{
	int state;
	unsigned char index;
	WVector startPos;
	WVector pos;
	WVector shotFromPos983; // 983 +0x20: copia de pos de onde saiu a tacada (ChangeGameMode 0068e7fd, apos o push do m_backUp); nenhum leitor no 983
	WCrypticValue<unsigned char> stroke[18];
	WCrypticValue<unsigned char> putt[18];
	WCrypticValue<int> pang[18];
	WCrypticValue<int> skinsPang[18];
	unsigned long color;
	unsigned long uid;
	unsigned long oid;
	unsigned char itemNum;
	unsigned long item;
	unsigned long windAbility983; // 983 +0x4c0: 1 = vento 1m (CWindVolumeObject 00b45f20 grava; CWind 006c9550 le); AddPlayer 006a7553 zera
	unsigned char teeOrder;
	WCrypticValue<float> gauge;
	unsigned short unusedWord;
	unsigned char bunker;
	unsigned char timeout;
	unsigned char giveUp;
	unsigned long shotFlag;
	// HACK: std::bitset<32> stays instantiated in the PCH (its names keep the PCH name count of the
	// canaries SaveScreenShot/mascotinfodlg); the original field zeroes inline, no bitset COMDATs
	typedef char rb_bitset32[sizeof(std::bitset<32>)];
	WFlags event;
	unsigned char flag;
	unsigned char team;
	unsigned char teammate;
	unsigned long shotTime;
	unsigned char order : 4;
	unsigned char rank : 4;
	unsigned char totalStroke;
	int score;
	WCrypticValue<int> totalPang;
	WCrypticValue<int> bonusPang;
	unsigned long holeNum;
	WCrypticValue<int> power;
	WCrypticValue<int> accuracy;
	WCrypticValue<int> control;
	WCrypticValue<int> spin;
	WCrypticValue<int> curve;
	unsigned char chatBlock;
	sWinningPrize prize;
	unsigned char extPrizeNum;
	sPrizeInfo extPrize[128];
	unsigned char holeInPose;
	unsigned char holeInCollision;
	__int64 skinsTotalPang;
	__int64 skinsSumPang;
	__int64 skinsHolePang;
	unsigned char level;
};

struct sTeamData
{
	int state;
	WCrypticValue<unsigned char> stroke[18];
	WCrypticValue<unsigned char> putt[18];
	WCrypticValue<int> pang[18];
	unsigned char totalStroke;
	int score;
	WCrypticValue<int> totalPang;
	WCrypticValue<int> bonusPang;
	WVector pos;
	unsigned long color;
	WCrypticValue<float> gauge;
	unsigned char bunker;
	unsigned char shooter;
	unsigned char club;
	unsigned char turn;
	unsigned char teeShooter;
	unsigned char holeInPose;
	unsigned char holeInCollision;
};

enum eApproachMissionType
{
};

enum eApproachPrizeType
{
};

enum eApproachWinnerType
{
};

struct sHoleData
{
	WVector tee;
	WVector pin;
	unsigned char par;
	unsigned char playerNum;
	unsigned char rank[4];
	unsigned char result;
	unsigned char carry;
	__int64 pang;
	__int64 basePang;
	unsigned long checkSum;
	unsigned long reserved;
};

struct sCameraData
{
	float distance;
	float height;
	float clearance;
	WVector target;
	float angle;
	float angleOffset;
	WMatrix mat;
	float pitch;
};

#pragma pack(push, 1)
struct sShotResultMsg
{
	unsigned long unused0;
	float pos[3];
	unsigned char unused10;
	unsigned char flag;
	unsigned char unused12[0x14]; // 983: sizeof 0x26 (era 0x25)
};

struct sHoleResult983 // 983 CGolfDoc +0x340 [4]: resultado do buraco por jogador (golftask, pacote do fim do buraco)
{
	unsigned long uid;
	unsigned char rankBits; // +0x04: nibble alto do jogador +0x4f0 (GolfRule 0x80)
	char score; // +0x05: jogador +0x4f4
	unsigned char byte6; // +0x06: jogador +0x4f1
	short exp; // +0x07: scoreboard (proximo nivel)
	unsigned long totalPang; // +0x09: jogador +0x4f8 (Team totalPang)
	unsigned long totalPangHi; // +0x0d: parte alta do __int64 do jogador (+0x4fc); so a baixa e lida
	unsigned long bonusPang; // +0x11: jogador +0x508 (Team bonusPang)
	unsigned long bonusPangHi; // +0x15: parte alta do __int64 do jogador (+0x50c); so a baixa e lida
	unsigned long skins19; // +0x19: Skins, jogador +0xd90
	unsigned long skins1d; // +0x1d: Skins, jogador +0xd94
};

struct sRemoteGauge983 // 983 CGolfDoc +0x48e: pacote 0x57 de outro jogador -> GolfRule M983(0x7b)
{
	unsigned char state; // 1 = para o som e volta a fase 1; 2 = valor da barra
	unsigned long value; // +0x48f -> m_bar (+0xe8/+0xec)
};
#pragma pack(pop)

class CTaskDoc : public IObject
{
public:
	CTaskDoc();
	virtual ~CTaskDoc();

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	CTask* GetTask() const;

#endif
protected:
#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	CTask* m_pTask;

	friend class CTaskManager;
#else
	unsigned long m_unused4;
#endif
};

#if defined(REBANG_GOLFRULE_PCH) || defined(REBANG_GOLFPLAYER_CPP)
enum eTeamName
{
	TEAM_RED,
	TEAM_BLUE
};

#endif
class CPangyaCorrectionObject;
class CPangyaAddCorrectionObj;
class CControlledObject;
class CPangyaCorrectionSetObject;

class CGolfDoc : public CTaskDoc
{
	DECLARE_OBJECT(CGolfDoc)

protected:
	CGolfDoc();

public:
	struct sTarget
	{
		sTarget()
		{
			m_unused0 = 0;
			m_flag = 0;
		}

		unsigned long m_unused0;
		unsigned char m_unused4[0xc];
		unsigned char m_flag;
	};

	virtual ~CGolfDoc();

	sPlayerData* GetPlayer(unsigned char index);
	sTeamData* GetTeam(unsigned char index);
	bool IsCurrentPlayerEvent983(unsigned long flag); // JP 983: 00663870
	void MakePlayer(unsigned char num);
	void MakeTeam();
	void ResetTeamVars();
	void LoadXml(const char* filename, unsigned char index, bool reload);
	void RefreshPlayerNum();
	void SetPlayerLevel(unsigned char index);
	CPartTidList& GetPartTidList(unsigned char index);
	void SetPartTidList(unsigned char index, unsigned long charTid,
		unsigned char hairColor, unsigned char shirtColor,
		unsigned long* partTid, unsigned long* partId);
	void SetApproachCurHoleResult(sApproachResultData& result);
	void SetApproachGameResult(sApproachResultData& result);
	void ApproachDataAllClear();
	void ClearApproachCurHoleResult();
	void ClearApproachGameResult();
	sApproachResultData* GetAppMyCurHoleResult();
	sApproachResultData* GetAppMyGameResult();
	void MakeHoleData();
	void ClearShotResultMsg();
	void InitShotResultMsg();
	void LogStrokeScore();
	void UpdateBright(float bright);
	void SendReport();
	WMatrix MakeTopViewCameraMatrix(bool check);
	WMatrix GetDefaultCamera();
	void MakeDefaultCamera(bool apply);
	sCameraData& GetCameraData() { return m_cameraData; }
	void SetApproachMissionDesc(const char* desc);
	char* GetApproachMissionDesc();
	void SetApproachMissionType(eApproachMissionType type);
	void SetApproachPrizeType(eApproachPrizeType type);
	void SetApproachWinnerType(eApproachWinnerType type);
	eApproachMissionType GetApproachMissionType() const;
	eApproachPrizeType GetApproachPrizeType() const;
	eApproachWinnerType GetApproachWinnerType() const;
	std::list<sApproachResultData*>& GetApproachCurHoleResult();
	std::list<sApproachResultData*>& GetApproachGameResult();
	void AddApproachTotalPrize(unsigned long prize);
	unsigned long GetApproachTotalPrize() const;
	WMatrix* MakeTopViewCameraMatrix(WMatrix& out, bool bTop);

	unsigned char GetCurrentPlayer() { return m_currentPlayer; }
#ifdef REBANG_GOLFRULE_PCH
	unsigned char& GetUIAlpha();
	sBar& GetBar();
	unsigned long& GameMode();
	void NetGameEnd();
	bool IsNetGameEnd();
	void SetTeeShotReady(bool);
	bool IsTeeShotReady();
	void SetTeamGameWinner(eTeamName);
	void SetStatUpdated(bool);
	bool GetStatUpdated() const;
	bool GetApproachReloaded();
	void SetApproachReloaded(bool);
#endif
#ifdef REBANG_GOLFPLAYER_CPP // golfplayer.cpp defines it (emission order); other includers keep their names
	eTeamName GetTeamGameWinner();
#endif
	unsigned char GetCurrentPlayerNum();
	void SetCurrentPlayerNum(int num);
	unsigned char GetPlayerNum() { return m_playerNum; }
	bool IsPause();
	void SetPause(bool bPause);
	bool GetDay();
	void SetDay(bool bDay) { m_bDay = bDay; }
	bool IsReported();
	void SetReport(bool bReport);
	bool IsShowChat();
	void ShowChat(bool bShow);
	int GetSaveChatNum();
	void SetSaveChatNum(int num);
	sHoleData& GetHoleData() { return m_pHoleData[m_currentHole - 1]; }
	void SetHoleRange983(unsigned char hole, float range); // JP 983: 00661eb0 (tee e pin do buraco pelo alcance; modo 0xb)
	void SetHoleTees983(unsigned char hole, std::vector<WVector2D>* pTees); // JP 983: 00661f30 (pin + todos os tees; o 1o vira o tee)
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: body in golfrule_pch_inl.h / at the original PCH position (emission order)
	sHoleData& GetHoleData(unsigned char hole);
#else
	sHoleData& GetHoleData(unsigned char hole) { return m_pHoleData[hole - 1]; }
#endif

	WPolySoup* m_pPolySoup;
	CSky* m_pSky;
	CAutoTarget* m_pAutoTarget;
	CPropertyInfo m_propertyInfo;
	CMapInfo m_mapInfo;
	CWaveInfo m_waveInfo;
	CRiverInfo m_riverInfo;
	WList<char*> m_modelNameList;
	int m_tutorialMode;
	std::list<unsigned long> m_inputList983; // 983 +0x8c (head +0x8c, size +0x90): teclas da regua (CGolfRule 00685980/00675dd0/CalcInputCommand)
	WPuppet* m_pCameraTargetPuppet; // 983 +0x98: "camera_target.pet" do CTutorial (OnLoad 007ce020 grava, OnDestroy 007cd030 solta)
	WVector m_tutorialPin983; // 983 +0x9c: pin do tutorial (macro 15 do CTutorial: ponto do PolySoup no chao)
	unsigned char m_bTutorialPin983; // 983 +0xa8: 1 = pin do tutorial posto (macro 15); zerado no reinicio do tutorial
	unsigned char m_unused99[3];
	std::string m_xmlFile;
	sPlayerData* m_pPlayer;
	sTeamData* m_pTeam;
	int m_teamNum;
	unsigned char m_currentPlayerNum;
	unsigned char m_playerNum;
	unsigned char m_unusedc6[2];
	sHoleData* m_pHoleData;
	unsigned char m_uiAlpha; // 983 +0xdc: alfa da UI da partida (0xff normal, 0x80 esmaecida; GetUIAlpha 005298f0)
	unsigned char m_unusedcd[3];
	sBar m_bar; // 983: +0xe0 (Bar() 004ae160: [[0xe47e2c]+0x11024] + 0xe0; o 645 tinha a barra em +0xd0); pedido C23
	sCameraData m_cameraData;
	unsigned long m_gameMode;
	unsigned char m_bNetGameEnd; // 983 +0x15c: NetGameEnd/IsNetGameEnd (fim da partida online)
	bool m_bPause;
	unsigned char m_bTeeShotReady; // 983 +0x15e: SetTeeShotReady/IsTeeShotReady
	bool m_bReport;
	bool m_bDay;
	bool m_bShowChat;
	unsigned char m_unused152[2];
	int m_saveChatNum;
	unsigned char m_bReplaying; // 983 +0x168: Replay liga (0067b5e0), ChangeTurn desliga (0067c730); HelpMsg le
	unsigned char m_bDropEnable; // 983 +0x169: M983(0x1d0) do tutorial liga/desliga; cena so desenha o drop com ele (9b15d1)
	unsigned char m_bTutorialGaugeMute; // 983 +0x16a: M983(0xa3) (macro "GolfRule 192" do CTutorial) alterna; com 0 o DriveGauge manda a barra ao Tutorial (0x155, 0069b2c6)
	unsigned char m_unused15b;
	int m_tutorialMinGauge983; // 983 +0x16c: CTutorial M983(0x155): barra abaixo disto volta a ele (007cf9a0)
	unsigned char m_bGodMode; // 983 +0x170: console 0x32 ("You are the God now!"), PerformPowerBarSetting
	unsigned char m_unused161[3];
	unsigned long m_reserved174_983; // 983: 0x174, o construtor não grava; nome meu
	float m_debugBarValue983; // 983 +0x178: comando de console "bar" N (2..9 -> 138.0..142.0; 00721420), 0.0 no construtor (0066d240); nenhum fld dele achado no 983
	unsigned char m_currentHole;
	unsigned char m_unused169[3];
	unsigned long m_remainTime;
	unsigned long m_shotTime;
	unsigned char m_shotPhase;
	unsigned char m_currentPlayer;
	unsigned char m_unused176[2];
	float m_bright;
	bool m_bDropRequest;
	bool m_bStopRequest;
	unsigned char m_maxGrade;
	bool m_bStopDlg;
	bool m_bNoStat;
	bool m_b195_983; // 983: 0x195, zerado junto com m_bNoStat (word); nome meu
	unsigned char m_unused181[2];
	float m_impactScale; // 983 +0x198: console 0x41 "impact" (GolfClub().SetAccuracy)
	float m_controlScale; // 983 +0x19c: console 0x41 "control"
	int m_skinsHoleWinner; // 983 +0x1a0: Skins (gameType 7) uid do vencedor do buraco (0x65); -1 = carry
	int m_skinsPenaltyUid; // 983 +0x1a4: Skins, uid do 0x66/0x8f/0x91/0xb3; esse perde 5% do pang no fim
	unsigned short m_skinsNextTurnWord983; // 983 +0x1a8: Skins, word do 0x63 (PM_NET_NEXT_TURN, M983(0x7d)); so escrito
	unsigned char m_unused196[2];
	__int64 m_skinsHolePot; // 983 +0x1b0: Skins, pote do buraco (base * mult)
	__int64 m_skinsPotBase; // 983 +0x1b8: Skins, soma dos pangs-base acumulados
	unsigned long m_skinsPotMult; // 983 +0x1c0: Skins, multiplicador do carry (1,2,4,8)
	unsigned char m_skinsCarryCount; // 983 +0x1c4: Skins, buracos seguidos em carry (pose/camera)
	unsigned char m_unused1ad[3];
	__int64 m_skinsTotalPang; // 983 +0x1c8: Skins, pang ainda em jogo
	__int64 m_skinsWinnerBonus983; // 983 +0x1d0: Skins, somado ao pang do vencedor (CalcWinnersPang); so zerado (CalcSkinsHolePang/ctor)
	unsigned char m_unused1c0[0x168];
	sHoleResult983 m_holeResult983[4]; // 983 +0x340
	sShotResultMsg m_shotResultMsg[4];
	unsigned char m_shotSync983[0x32]; // 983 +0x45c: bloco do 0x7a (sShotSyncData) de outro jogador; memset de 0x32
	sRemoteGauge983 m_remoteGauge983; // 983 +0x48e
	unsigned char m_unused473;
	CPartTidList m_partTidList[4];
	bool m_bApproach794;
	bool m_bApproach795;
	char m_approachMissionDesc[0x82];
	eApproachMissionType m_approachMissionType;
	eApproachPrizeType m_approachPrizeType;
	eApproachWinnerType m_approachWinnerType;
	std::list<sApproachResultData*> m_approachCurHoleResult;
	std::list<sApproachResultData*> m_approachGameResult;
	unsigned long m_approachTotalPrize;
	// 983 (pedido C26): objetos de correcao do PangYa por jogador (indice = m_currentPlayer); o construtor (0066d240) nao os toca
	CPangyaCorrectionObject* m_pPangyaCorrection983[4];       // +0x860: 0065fe70 (habilidade 0x0a)
	CPangyaAddCorrectionObj* m_pPangyaAddCorrection983[4];    // +0x870: 0065fec0 (0x16)
	CPangyaCorrectionSetObject* m_pPangyaCorrectionSet983[4]; // +0x880: 0065fef0; 0065fed0 zera, 00662200 refaz (conteudo 0xad, 0x23)
	CControlledObject* m_pPowerShotItem983[4];                // +0x890: 00662240 (0x17); 0073edb0 +1 power shot
	CControlledObject* m_pPlusYardItem983[4];                 // +0x8a0: 00662280 (0x19)
	CControlledObject* m_pComboGaugeEveryshot983[4];          // +0x8b0: 006622c0 (0x1a); sizeof 0x8c0
	static std::stack<CGolfDoc*, std::list<CGolfDoc*> > m_GolfDocInstanceStack;
	// HACK: instantiates stack<CGolfDoc*> (and its list) in the PCH (COMDAT order); reuses the name (PCH name-count canaries)
	typedef char
		rb_bitset32[sizeof(std::stack<CGolfDoc*, std::list<CGolfDoc*> >)];
};

#ifdef REBANG_STDAFX_STATICS
// HACK: header statics that the original emitted only in the units that compiled stdafx.h without the
// PCH (stdafx.cpp itself and videoin.cpp). /Yu units never emit unreferenced header statics, so they are
// kept behind this macro (defined by videoin.cpp) and the PCH stays unchanged.
// golfdoc.h:282-283 in the original line table.
static float BAR_LEN = BAR_END - BAR_START;
static float BAR_MIDPOINT = (BAR_END + BAR_START) * 0.5f;
// The original header of these is unknown; only their .data bytes are known.
static const char* s_taskName[5] = { "CLobbyTask", "CShopTask",
	"CRealMyRoomTask", "ntAvatarChatTask", "CGolfTask" };
static int rb_rdataFillInt = 0xff;
static char rb_rdataFillChar = (char)0xff;
// s_courseOrder comes after BAR_END/BAR_START in the original .rdata.
#include "courseorder.h"
#endif

#ifdef REBANG_GOLFRULE_PCH
#ifdef REBANG_GOLFRULE_PCH // golfrule.cpp: inline bodies the original PCH has here (emission order)
#ifndef REBANG_STDAFX_STATICS
static float BAR_LEN =
	BAR_END - BAR_START; // golfdoc.h:282 in the original line table
#endif
inline void CTask::SetPause(bool bPause)
{
	m_bPause = bPause;
}
inline float sBar::Persent() const
{
	return (power - BAR_START) / BAR_LEN;
}

inline float sBar::Delta() const
{
	return impact - center;
}

inline sHoleData& CGolfDoc::GetHoleData(unsigned char hole)
{
	return m_pHoleData[hole - 1];
}
inline unsigned char& CGolfDoc::GetUIAlpha()
{
	return m_uiAlpha; // 983: +0xdc (005298f0); o 645 lia +0xcc cru
}
inline sBar& CGolfDoc::GetBar()
{
	return m_bar; // 983: +0xe0 (004ae160); o 645 lia +0xd0 cru
}
inline unsigned long& CGolfDoc::GameMode()
{
	return m_gameMode;
}
inline void CGolfDoc::NetGameEnd()
{
	m_bNetGameEnd = 1; // 983 +0x15c (era o +0x14c cru do 645)
}
inline bool CGolfDoc::IsNetGameEnd()
{
	return m_bNetGameEnd != 0;
}
inline void CGolfDoc::SetTeeShotReady(bool b)
{
	m_bTeeShotReady = b; // 983 +0x15e (era o +0x14e cru do 645)
}
inline bool CGolfDoc::IsTeeShotReady()
{
	return m_bTeeShotReady != 0;
}
inline void CGolfDoc::SetPause(bool b)
{
	m_bPause = b;
}
inline bool CGolfDoc::IsPause()
{
	return m_bPause;
}
inline void CGolfDoc::SetTeamGameWinner(eTeamName t)
{
	*(eTeamName*)&m_teamNum = t; // 983 +0xd0 (era o +0xc0 cru do 645)
}
inline void CGolfDoc::SetReport(bool b)
{
	m_bReport = b; // 983 +0x15f (era o +0x14f cru do 645)
}
inline void CGolfDoc::ShowChat(bool b)
{
	m_bShowChat = b;
}
inline bool CGolfDoc::IsShowChat()
{
	return m_bShowChat;
}
inline void CGolfDoc::SetStatUpdated(bool b)
{
	m_bApproach794 = b; // 983 +0x7b4 (era o +0x794 cru do 645)
}
inline bool CGolfDoc::GetStatUpdated() const
{
	return m_bApproach794; // 983 +0x7b4 (era o +0x794 cru do 645)
}
inline bool CGolfDoc::GetApproachReloaded()
{
	return m_bApproach795; // 983 +0x7b5 (era o +0x795 cru do 645)
}
inline void CGolfDoc::SetApproachReloaded(bool b)
{
	m_bApproach795 = b; // 983 +0x7b5 (era o +0x795 cru do 645)
}
#endif

#endif

#ifdef REBANG_GOLFTASK_PCH // golftask.cpp: golfdoc.h:546 in the original (PCH inline, emission order)
inline bool CGolfDoc::IsReported()
{
	return m_bReport;
}
#endif
