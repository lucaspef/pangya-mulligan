#include "minatl.h"
#include "vc10_layout.h"
#include "jp/uiframesetmanager.h"
#include "jp/lobbyjp.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfdocofs983.h"
#include "golfrulebase.h"
#include "commonutil.h"
#include "itemcursor983.h"
#include "jp/shareddocjp.h" // SetFixHole983 (983 006775f0)
#define RB_HOLEINBONUS_LINKAGE extern "C"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"
#include "fx.h"
#include "fxspray.h"
#include "golfball.h"
#include "quadtree.h"
#include "club.h"
#include "mousecursor.h"
#include "emoticondlg.h"
// HACK: unreferenced inline body = IL only (no code); calibrates the PCH inline merge (golfrule_pch.h).
#define RB_ILPAD(n, body) \
	struct RbIlPad##n \
	{ \
		int v; \
		void g() { body } \
	};

// atlcore.h (PCH) GetResourceInstance lands between emoticondlg.h and wind.h in the original
RB_ILPAD(H,
	GRFILL1K ILFILL256 ILFILL64 ILFILL64 ILFILL64 ILFILL16 ILFILL4 ILFILL4
		ILFILL4)
#include "wind.h"
#include "headicon.h"
#include "autotarget.h"
#include "capturedbg.h"
#include "quitwindow.h"
#include "giftalarmdlg.h"
#include "hackingmanager.h"
#include "tooltip.h"
#include "registry.h"
#include "chatmsg.h"
#include "inputmanager.h"
#include "user_info.h"
#include "projectg.h"
#include "frcombobox.h"
#include "../shared/localize.h"
#include "onelinereqdlg.h"
#include "approachresultdlg.h"
#include "shadow.h"
#include "frdesktop.h"
#include "voiceitem.h"
#include "itemwindow.h"
#include "codetinker.h"
#include "kbdmanualdlg.h"
#include "shotpreview.h" // JP983_DEV_PREVIEW (Ctrl+R da conta GM)
#include <algorithm>


extern "C" void _ReadWriteBarrier(void);
#pragma intrinsic(_ReadWriteBarrier)

extern Fresh* g_pFresh;

extern WMatrix g_camera;
extern int g_iSendPacketType;
extern int g_iRecvPacketType;
extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
extern "C" __declspec(dllimport) int __cdecl _access(const char*, int);
bool IsMyTurn(bool check);
std::string GetMyComputerName();
unsigned char GetCurMap();
unsigned char MyIndex();
const WVector& GetHoleCupPos(); // 983: devolve referencia
sPlayerData* PLAYER(unsigned char index);
extern bool g_bQuit;
int __fastcall WisZero(const float& f, float epsilon);
int g_iRecvPacketGR;
int g_iSendPacketGR;

class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	void SetHelpMsg(int index, char* msg);
	void CheckReplayAvailable();
};

class CRival : public WSingleton<CRival>
{
public:
	CRival();
	virtual ~CRival();
	void Shot();
	void SetResult();
	WVector GetPos() { return m_pos; }

	WVector m_pos;
	unsigned char m_unused34[0x5c - 0x34];
};

class CCameraWork : public WSingleton<CCameraWork>
{
public:
	using WSingleton<CCameraWork>::m_pInstance;

	virtual ~CCameraWork();
	void Process(float time, float);
	void StopBallSound();
	void InputProcess(float time);
	void SetCaddieChase(bool chase);
	void SelectCamera(int type);
	int GetCameraType() { return m_cameraType; }
	void Lock() { m_bLock = true; }
	void SetReplayCount(int count);
	void SetAbsAnim();
	void SetCamera(WMatrix& camera) { m_camera = camera; }
	void Unlock()
	{
		if (m_bLock)
		{
			m_bLock = false;
			m_input.Reset();
		}
	}
	struct sInput
	{
		int x;
		int y;
		int z;
		unsigned char button;
		void Reset()
		{
			// HACK: dead reference to a PCH inline: defers the emission to the end of wave 0, as in the original
			if (0)
			{
				WVector* rb_v = NULL;
				*rb_v = *rb_v;
			}
			button = 0;
			x = 0;
			z = 0;
			y = 0;
		}
	};
	bool Finished();
	void UninstallCamera();
	void SetInstalledCamera(const WVector* pos);
	enum eCamMode
	{
		eCamMode0,
		eCamMode1,
		eCamMode2
	};
	float LoadCamera(eCamMode mode, char* name, const WVector& pos);
	void Load();

	sInput m_input;
	unsigned long m_unused38[(0x7c - 0x38) / 4];
	// B1453: os 3 vector<int> de camerawork.h (m_camCandidates/m_vecA/m_vecB) com o tipo real, para a copia andar junto
	// no cl novo (vector 0xc x 0x10 do VC10; medido out/D859/ofs.py: m_i8c 0x84, m_f190 0x1b8, m_f264 0x290, m_i2a4 0x2c8)
	std::vector<int> m_camCandidates983; // 983 0x7c
	int m_cameraType;
	unsigned long m_unused90[(0xf0 - 0x90) / 4];
	WMatrix m_camera;
	// 983 (s101): a copia local estava no layout do 645 (sizeof 0x2b4) e o `new CCameraWork` do OnPreLoadInit reservava
	// 0x2b4 bytes para o construtor do 983 (camerawork.cpp), que grava ate 0x2ec. Deslocamentos de camerawork.h
	// (tools/f34/offs.py CCameraWork): m_b1dc 0x20c (Unlock 00672390), m_b1de 0x20e, m_i2a4 0x2d8, m_b2b1 0x2e5.
	unsigned char m_unused120[0x1c0 - 0x120];
	float m_f190983;            // 983 0x1c0 (m_f190 de camerawork.h; Finished)
	unsigned char m_unused1c4[0x20c - 0x1c4];
	bool m_bLock;               // 983 0x20c (645 0x1dc)
	unsigned char m_unused20d;
	bool m_bCaddieChase;        // 983 0x20e (645 0x1de)
	unsigned char m_unused20f[0x298 - 0x20f];
	float m_f264983;            // 983 0x298 (m_f264 de camerawork.h; Finished)
	unsigned char m_unused29c[0x2a4 - 0x29c];
	std::vector<int> m_vecA983; // 983 0x2a4
	unsigned char m_unused2b4[0x2b8 - 0x2b4];
	std::vector<int> m_vecB983; // 983 0x2b8
	unsigned char m_unused2c8[0x2d8 - 0x2c8];
	int m_replayCount;          // 983 0x2d8 (645 0x2a4)
	unsigned char m_unused2dc[0x2e0 - 0x2dc];
	int m_i2ac983;              // 983 0x2e0 (m_i2ac de camerawork.h; B1465)
	unsigned char m_unused2e4;
	bool m_bAbsAnim;            // 983 0x2e5 (645 0x2b1)
	unsigned char m_unused2e6[0x2ec - 0x2e6];

	CCameraWork();
};
// B1477: crus do sPlayerData que ficam (team/Pang cifrado/Skins em dwords); medidos iguais no cl 14.44 (out/B1477)
C_ASSERT(offsetof(sPlayerData, team) == 0x4e9 && offsetof(sPlayerData, totalPang) == 0x4f8 && offsetof(sPlayerData, bonusPang) == 0x508 && offsetof(sPlayerData, skinsTotalPang) == 0xd90);
C_ASSERT_VC10(sizeof(CCameraWork) == 0x2ec);
C_ASSERT_VC10(offsetof(CCameraWork, m_cameraType) == 0x8c);
C_ASSERT_VC10(offsetof(sPlayerData, bunker) == 0x4da && offsetof(sPlayerData, team) == 0x4e9 && offsetof(sPlayerData, totalStroke) == 0x4f1); // B1531 (sonda VC10)
C_ASSERT_VC10(offsetof(sPlayerData, pos) == 0x14 && offsetof(sPlayerData, item) == 0x4bc && offsetof(sPlayerData, windAbility983) == 0x4c0 && offsetof(sPlayerData, shotFlag) == 0x4e0 && offsetof(sPlayerData, teammate) == 0x4ea && offsetof(sPlayerData, shotTime) == 0x4ec); // B1562
C_ASSERT_VC10(offsetof(sPlayerData, gauge) == 0x4c8); // B1563
C_ASSERT_VC10(sizeof(CGolfBall::sExpected) == 0x78 && offsetof(CGolfBall::sExpected, curveRot) == 0x18 && offsetof(CGolfBall::sExpected, sound) == 0x20 && offsetof(CGolfBall::sExpected, event) == 0x60 && offsetof(CGolfBall::sExpected, state) == 0x70); // B1564
C_ASSERT_VC10(offsetof(CGolfBall, m_state) == 0x18 && offsetof(CGolfBall, m_curveRot) == 0x2c && offsetof(CGolfBall, m_spinRot) == 0x30); // B1564
C_ASSERT_VC10(offsetof(CGolfBall, m_meshIndex) == 0x28 && offsetof(CGolfBall, m_initCurve) == 0x34 && offsetof(CGolfBall, m_initSpecial) == 0x44 && offsetof(CGolfBall, m_special) == 0x48 && offsetof(CGolfBall, m_bOB) == 0x58 && offsetof(CGolfBall, m_groundType) == 0x59 && offsetof(CGolfBall, m_water) == 0x5c && offsetof(CGolfBall, m_renderNum) == 0x64 && offsetof(CGolfBall, m_spikeEndFrame) == 0xa4 && offsetof(CGolfBall, m_totalTime) == 0xa8 && offsetof(CGolfBall, m_standGround) == 0xb0 && offsetof(CGolfBall, m_prevColTriIndex) == 0xb4 && offsetof(CGolfBall, m_colTriIndex) == 0xbc && offsetof(CGolfBall, m_vectorSlideCount) == 0xc0 && offsetof(CGolfBall, m_radius) == 0xc8); // B1565
C_ASSERT_VC10(offsetof(sTeamData, pos) == 0x38c && offsetof(sTeamData, gauge) == 0x39c && offsetof(sTeamData, bunker) == 0x3ac); // B1562
C_ASSERT_VC10(offsetof(sPlayerData, score) == 0x4f4 && offsetof(sPlayerData, totalPang) == 0x4f8 && offsetof(sPlayerData, bonusPang) == 0x508);
C_ASSERT_VC10(offsetof(CQuadTree, m_frame) == 0x21bc && offsetof(CQuadTree, m_slopeFactor) == 0x21d4);
C_ASSERT_VC10(offsetof(CCameraWork, m_camera) == 0xf0);
C_ASSERT_VC10(offsetof(CCameraWork, m_bLock) == 0x20c);
C_ASSERT_VC10(offsetof(CCameraWork, m_camCandidates983) == 0x7c && offsetof(CCameraWork, m_f190983) == 0x1c0);
C_ASSERT_VC10(offsetof(CCameraWork, m_f264983) == 0x298 && offsetof(CCameraWork, m_vecA983) == 0x2a4);
C_ASSERT_VC10(offsetof(CCameraWork, m_vecB983) == 0x2b8 && offsetof(CCameraWork, m_replayCount) == 0x2d8);
C_ASSERT_VC10(offsetof(CClub, m_pClub) == 0x4 && offsetof(CClub, m_powerShot) == 0x2c && offsetof(CClub::sClub, type) == 0x22); // B1532
C_ASSERT_VC10(offsetof(CCameraWork, m_i2ac983) == 0x2e0 && offsetof(CCameraWork, m_bAbsAnim) == 0x2e5);

// 983: sub-regra do CGolfRule (+0x63c) e regra do hole-in-one, classes da frente B (include/jp/subgolfrule.h,
// include/jp/golfruleholeinone.h; corpos em src/projectg/jp/). Copias locais so com o que este arquivo usa (como as de cima):
// a ordem das virtuais e o sizeof sao os da B (vtables 00d16980/00d169c4/00d169dc; CGolfRuleHoleInOne 0x28, vtable 00d09774).
class CSubGolfRuleBase
{
public:
	CSubGolfRuleBase();
	virtual ~CSubGolfRuleBase(); // [0]
	virtual void Init();          // [1] +0x04: OnReload (00678a5e)
	virtual void Process(float dt); // [2]
	virtual void On3();           // [3]
	virtual void OnShotEnd();     // [4] +0x10: DoFlyBall_Stopped (006924aa)

protected:
	bool ShotDataLoad(const char* filename);
	CGolfRule* m_pRule;
};
C_ASSERT(sizeof(CSubGolfRuleBase) == 8);

class CSubGolfRuleGame : public CSubGolfRuleBase
{
public:
	CSubGolfRuleGame(CGolfRule* pRule); // 00650540
	virtual ~CSubGolfRuleGame();
	virtual void Init();
};

class CSubGolfRuleReplayTape : public CSubGolfRuleBase
{
public:
	CSubGolfRuleReplayTape(CGolfRule* pRule); // 006506e0
	virtual ~CSubGolfRuleReplayTape();
	virtual void Init();
	virtual void Process(float dt);
	virtual void OnShotEnd();
};

// A B deriva a CGolfRuleHoleInOne da CGolfRuleBaseJp (copia da CGolfRuleBase com as 17 virtuais; pedido B21/B22 ainda
// aberto): aqui so o construtor e o tamanho; o ponteiro vai para o m_pRuleBase com cast (mesma tabela virtual).
class CGolfRuleHoleInOne
{
public:
	CGolfRuleHoleInOne(CGolfRule* pGolfRule); // 00546400
	void OnShotEnd(); // 00545d20 (jp/golfruleholeinone.cpp; chamada pelo HandleMsg case 0xc6)
	unsigned char m_data[0x28];
};

// Emission order of the original camerawork.h: GetCameraType, Lock, Finished, SetCaddieChase, SetReplayCount, SetAbsAnim.
inline bool CCameraWork::Finished()
{
	float t = m_f264983;
	return 0.0f < t && t < m_f190983;
}

inline void CCameraWork::SetCaddieChase(bool chase)
{
	m_bCaddieChase = chase;
}

inline void CCameraWork::SetReplayCount(int count)
{
	m_replayCount = count;
}

inline void CCameraWork::SetAbsAnim()
{
	m_bAbsAnim = true;
}

// HACK: include position = emission order of these headers' inlines (after rival/camerawork in the original)
#include "randomhouse.h"
#include "netresourcemanager.h"
#include "clientsetting.h"
#include "slicestr.h"
#include "onelineboard.h"
#include "rankingdlg.h"
#include "messengerdlg.h"
#include "shufflebonus.h"
#include "replaydlg.h"
#include "effectmanager.h"
#include "powergauge.h"

// HACK: same helper as trajectory.cpp (declaring it in scenemanager.h shifts other units)
class CSceneManagerEx : public CSceneManager
{
public:
	WPuppet* GetPuppet(const char* name);
	void UpdateWind();
	void UpdateFog(bool);
};

#pragma comment(linker, \
	"/alternatename:?GetPuppet@CSceneManagerEx@@QAEPAVWPuppet@@PBD@Z=?GetPuppet@CSceneManager@@QAEPAVWPuppet@@PBD@Z")

#pragma comment(linker, \
	"/alternatename:?UpdateWind@CSceneManagerEx@@QAEXXZ=?UpdateWind@CSceneManager@@QAEXXZ")

#pragma comment(linker, \
	"/alternatename:?UpdateFog@CSceneManagerEx@@QAEX_N@Z=?UpdateFog@CSceneManager@@QAEX_N@Z")

// HACK: functions of units still delinked: bound to the original placeholders
#pragma comment(linker, \
	"/alternatename:?CalcUIOffset@CTaskMain@@IAEX_N@Z=__pg_c_0009ea")
#pragma comment(linker, \
	"/alternatename:?SetBGMNone@CTaskMain@@IAEXXZ=__pg_c_000988")
#pragma comment(linker, \
	"/alternatename:?gIsChatLineEntended@COption@@QAE_NXZ=__pg_c_00002a")
#pragma comment(linker, \
	"/alternatename:?AlarmBtnUp@CMessengerInfo@@QAEXK@Z=__pg_c_0055f1")
#pragma comment(linker, \
	"/alternatename:?CreateOptionWindow@CQuitWindow@@QAEXXZ=__pg_c_001686")
#pragma comment(linker, \
	"/alternatename:?Enable@CShadowManager@@QAEX_N@Z=__pg_c_006223")

#define SCENEMGR() ((CSceneManagerEx*)CSceneManager::Instance())

// HACK: local declarations (adding them to the PCH headers shifts other units)
class CChatMsgEx : public CChatMsg
{
public:
	void ClearHistory();
};

#pragma comment(linker, \
	"/alternatename:?ClearHistory@CChatMsgEx@@QAEXXZ=?ClearHistory@CChatMsg@@QAEXXZ")

// JP 983 (s158): os apelidos abaixo apontavam para __pg_c_* (nomes do golfrule.obj do 645, que nao existem neste build) e
// viravam stub no exe (efeitos de tacada, MakeHoleData, Close do chat, SetLOD); agora apontam para os metodos reais
// (mesma assinatura; ponteiro x referencia e int x void/unsigned long tem o mesmo ABI no x86).
class CGolfDocEx : public CGolfDoc
{
public:
	void ClearShotResultMsg();
	void MakeHoleData();
	void InitShotResultMsg();
};

#pragma comment(linker, \
	"/alternatename:?InitShotResultMsg@CGolfDocEx@@QAEXXZ=?InitShotResultMsg@CGolfDoc@@QAEXXZ")

#pragma comment(linker, \
	"/alternatename:?MakeHoleData@CGolfDocEx@@QAEXXZ=?MakeHoleData@CGolfDoc@@QAEXXZ")

#pragma comment(linker, \
	"/alternatename:?ClearShotResultMsg@CGolfDocEx@@QAEXXZ=?ClearShotResultMsg@CGolfDoc@@QAEXXZ")

// HACK: same accessor as guidebook.cpp (the original header exposed m_bArea)
struct CMouseCursorAccess : public CMouseCursor
{
	using CMouseCursor::m_bArea;
	using CMouseCursor::m_mode;
	using WSingleton<CMouseCursor>::m_pInstance;
};

#define MOUSECURSOR() ((CMouseCursorAccess*)CMouseCursorAccess::m_pInstance)

struct sLadderItem
{
	unsigned long tid;
	unsigned long count;
};

// BAR_LEN (golfdoc.h:282 in the original) comes from the golfrule PCH variant: see golfdoc.h.
#define g_fBarRange BAR_LEN

// B1447: crus do CSharedDoc copiados do 983 (VC10) ancorados no membro (o cl novo anda o CSharedDoc); `off` constante
#include "jp/docofs983.h"
// m_userInfo[n] + (off - 0x38b0): crus do 983 `Doc + n * 0x31c6 + off` (sUserInfo pack 1, layout fixo)
// m_holeOrder[n - 1] (n = 1..19) na forma do 983 (`Doc + (n + 0x1051d)`): o VC10 gera o mesmo codigo que o cru
#define HOLEORDER1_983(n) (*((unsigned char*)Doc() + ((n) + (offsetof(CSharedDoc, m_holeOrder) - 1))))
#define USERINFO983(n, off) ((char*)&Doc()->m_userInfo[n] + ((off) - 0x38b0))
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0 && offsetof(CSharedDoc, m_courseMap) == 0x104fa);
C_ASSERT_VC10(offsetof(CSharedDoc, m_gameMode) == 0x1048c && offsetof(CSharedDoc, m_holeOrder) == 0x1051e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && offsetof(sRivalData, observerState) == 0x164);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, clubInfo.tid) == 0x6a20 && sizeof(sUserInfo) == 0x31c6);
C_ASSERT_VC10(offsetof(CSharedDoc, m_reservedChatList) + 4 == 0x10cd0 && offsetof(CSharedDoc, m_bFastForward) == 0x10588);
C_ASSERT_VC10(offsetof(CSharedDoc, m_recentWhisperList) == 0x10cc0);
C_ASSERT_VC10(offsetof(CSharedDoc, m_pinIndex) == 0x1050c && offsetof(CSharedDoc, m_pGolfDoc) == 0x11024);

#define SHAREDDOC_ULONG(off) (*(unsigned long*)DOCPTR983(off))
#define SHAREDDOC_UCHAR(off) (*(unsigned char*)DOCPTR983(off))

// HACK: raw CGolfDoc fields still hidden in the PCH header's padding
#define GOLFDOC_FLOAT(off) (*(float*)GDOCPTR983(GOLFDOC(), off)) // B1457: offset do 983 segue o membro (cl novo)
#define GOLFDOC_ULONG(off) (*(unsigned long*)GDOCPTR983(GOLFDOC(), off)) // B1457: offset do 983 segue o membro (cl novo)

int GetMyClubSlotSum983(unsigned long guid);	// jp/clubsetstat.cpp, 0049c770
// 0068c2f0: param3 do "PangyaLogo" (Letter 0x1fe) = 2; 3 (logo animado) se o grau do taco (00497f60) do jogador = 5
// B1544 (006900a2): sala normal soma clubInfo.slotUp983[0..4] do jogador da vez (vem do servidor, vale p/ os outros);
// so jogo em massa (00502f20) e realGameType 0xc/0xe/0x13 (00500200) usam o inventario proprio
int IsMassGame();
static int PangyaLogoParam983()
{
	int sum;
	const unsigned char realType = Doc()->m_roomInfo.realGameType;
	if (!IsMassGame() && realType != 0xc && realType != 0xe && realType != 0x13)
	{
		const short* slot = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.slotUp983;
		sum = slot[4] + slot[3] + slot[2] + slot[1] + slot[0];
	}
	else
		sum = GetMyClubSlotSum983(*(unsigned long*)(USERINFO983(GOLFDOC()->m_currentPlayer, 0x6a1c)));
	return (sum >= 30 && sum <= 59 && (sum - 30) / 5 == 5) ? 3 : 2;
}
#define GOLFDOC_UCHAR(off) (*(unsigned char*)GDOCPTR983(GOLFDOC(), off)) // B1457: offset do 983 segue o membro (cl novo)

#pragma pack(push, 1)
struct sCancelShotData
{
	unsigned char flag;
	unsigned long value;
};
#pragma pack(pop)

#define SHAREDDOC_ULONG(off) (*(unsigned long*)DOCPTR983(off))

WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

#define TEAM_HOLE_STROKE(t, h) \
	((unsigned char)~(((unsigned char*)(t))[(h) * 0x10 - 0xc + 0xe] ^ \
		((unsigned char*)(t))[(h) * 0x10 - 0xc]))

// HACK: dllimport + __forceinline = inlined without an out-of-line COMDAT (the
// original obj has no body for these two; their real form is not known yet)
__declspec(dllimport) __forceinline bool IsSharedFlag433Bit1()
{
	return (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
}
__declspec(dllimport) __forceinline unsigned long SharedShotMode()
{
	return Doc()->m_playingMode;
}

// HACK: the original calls PLAYER out of line here (it is inline in shareddoc.inl)
sPlayerData* __fastcall RbPlayerCall(unsigned char index);
#pragma comment(linker, \
	"/alternatename:?RbPlayerCall@@YIPAUsPlayerData@@E@Z=?PLAYER@@YIPAUsPlayerData@@E@Z")

static __forceinline bool IsTeammateTurn(CGolfDoc* doc)
{
	sPlayerData* me = RbPlayerCall(MyIndex());
	unsigned char cur = doc->m_currentPlayer;
	return cur == me->teammate;
}

#define CAMERAWORK_FLOAT(off) \
	(*(float*)((char*)CCameraWork::Instance() + (off)))

int IsMassGame();

// HACK: IsGlobalWind_Compare is not declared in wind.h (PCH canaries)
class CWindEx : public CWind
{
public:
	bool IsGlobalWind_Compare();
	void SetIntensity(float);
	void SetDirection(float);
};

#pragma comment(linker, \
	"/alternatename:?IsGlobalWind_Compare@CWindEx@@QAE_NXZ=?IsGlobalWind_Compare@CWind@@QAE_NXZ")
#pragma comment(linker, \
	"/alternatename:?SetIntensity@CWindEx@@QAEXM@Z=?SetIntensity@CWind@@QAEXM@Z")
#pragma comment(linker, \
	"/alternatename:?SetDirection@CWindEx@@QAEXM@Z=?SetDirection@CWind@@QAEXM@Z")

#define WINDEX() ((CWindEx&)Wind())

class WPolySoupEx2 : public WPolySoup
{
public:
	struct sArea
	{
		char type;
		char name[1];
	};

	struct sPoint
	{
		char unused[0x24];
		WVector pos;
	};

	sArea* GetAreaByPos(const WVector& pos);
	sCamera* GetCameraByName(const char* name);
	sPoint* FindPoint(const char* name);
};

#pragma comment(linker, \
	"/alternatename:?FindPoint@WPolySoupEx2@@QAEPAUsPoint@1@PBD@Z=?FindPoint@WPolySoup@@QAEPAUsPoint@1@PBD@Z")

#pragma comment(linker, \
	"/alternatename:?GetAreaByPos@WPolySoupEx2@@QAEPAUsArea@1@ABVWVector@@@Z=?GetAreaByPos@WPolySoup@@QAEPAUsArea@1@ABVWVector@@@Z")
#pragma comment(linker, \
	"/alternatename:?GetCameraByName@WPolySoupEx2@@QAEPAUsCamera@WPolySoup@@PBD@Z=?GetCameraByName@WPolySoup@@QAEPAUsCamera@1@PBD@Z")

struct CQuadTreeEx : public CQuadTree
{
	void CalcGreenPlane();
};

struct CClubEx : public CClub
{
	void SetAccuracy();
	float GetNarrowAccuracy();
	float GetWideAccuracy();
	void SetClub(char* name);
	void SetClub(unsigned char type);
	void Process(float);
};

#pragma comment(linker, \
	"/alternatename:?CalcGreenPlane@CQuadTreeEx@@QAEXXZ=?CalcGreenPlane@CQuadTree@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetAccuracy@CClubEx@@QAEXXZ=?SetAccuracy@CClub@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?GetNarrowAccuracy@CClubEx@@QAEMXZ=?GetNarrowAccuracy@CClub@@QAEMXZ")
#pragma comment(linker, \
	"/alternatename:?GetWideAccuracy@CClubEx@@QAEMXZ=?GetWideAccuracy@CClub@@QAEMXZ")
#pragma comment(linker, \
	"/alternatename:?SetClub@CClubEx@@QAEXPAD@Z=?SetClub@CClub@@QAEXPBD@Z")
#pragma comment(linker, \
	"/alternatename:?SetClub@CClubEx@@QAEXE@Z=?SetClub@CClub@@QAEXE@Z")
#pragma comment(linker, \
	"/alternatename:?Process@CClubEx@@QAEXM@Z=?Process@CClub@@QAEXM@Z")

void __fastcall RequestUccInfo(sUserInfo& info);

// B1452: offsets crus do CGolfRule copiados do 983 (= VC10) -> ancorados por trecho entre membros STL. No cl novo (VS 2022)
// deque/stack/vector menores andam o resto da classe para tras (medido: 0x218..0x4c8 -> 0x210..0x4b0); dentro de cada trecho o
// layout nao muda. No VC10 a conta dobra para o mesmo offset (C_ASSERT_VC10) e o obj nao muda. `off` constante; so dentro de CGolfRule.
#define GROFS983_R(off, lo, hi, m) ((unsigned)((off) - (lo)) < (unsigned)((hi) - (lo))) ? (offsetof(CGolfRule, m) + ((off) - (lo))) :
#define GROFS983(off) ( \
	GROFS983_R(off, 0x218, 0x224, m_inputCmdHead) \
	GROFS983_R(off, 0x23c, 0x2d8, m_cmdQueueTime) \
	GROFS983_R(off, 0x2f0, 0x3fc, m_backUp) \
	GROFS983_R(off, 0x40c, 0x4b8, m_curTeam) \
	GROFS983_R(off, 0x4c8, 0x650, m_pChatTarget) \
	(off))
#define GRPTR983(off) ((char*)this + GROFS983(off))
C_ASSERT_VC10(offsetof(CGolfRule, m_inputCmdHead) == 0x218 && offsetof(CGolfRule, m_cmdQueue) == 0x224);
C_ASSERT_VC10(offsetof(CGolfRule, m_cmdQueueTime) == 0x23c && offsetof(CGolfRule, m_backUpStack) == 0x2d8);
C_ASSERT_VC10(offsetof(CGolfRule, m_backUp) == 0x2f0 && offsetof(CGolfRule, m_bgPosList) == 0x3fc);
C_ASSERT_VC10(offsetof(CGolfRule, m_curTeam) == 0x40c && offsetof(CGolfRule, m_bgmList) == 0x4b8);
C_ASSERT_VC10(offsetof(CGolfRule, m_pChatTarget) == 0x4c8 && sizeof(CGolfRule) == 0x650);
// trechos com o mesmo tamanho nos dois compiladores
C_ASSERT(offsetof(CGolfRule, m_cmdQueue) - offsetof(CGolfRule, m_inputCmdHead) == 0xc);
C_ASSERT(offsetof(CGolfRule, m_backUpStack) - offsetof(CGolfRule, m_cmdQueueTime) == 0x9c);
C_ASSERT(offsetof(CGolfRule, m_bgPosList) - offsetof(CGolfRule, m_backUp) == 0x10c);
C_ASSERT(offsetof(CGolfRule, m_bgmList) - offsetof(CGolfRule, m_curTeam) == 0xac);
C_ASSERT(sizeof(CGolfRule) - offsetof(CGolfRule, m_pChatTarget) == 0x188);
#define CAPTURE_TILES() (*(int*)(GRPTR983(0x5d0))) // 983 (645: 0x54c)

#define SHAREDDOC_UCHAR(off) (*(unsigned char*)DOCPTR983(off))

// free helpers the original keeps in a header (emitted as inline COMDATs)

inline unsigned char MyIndex()
{
	if (IsSharedFlag433Bit1())
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 0053a260 (MyIndex) inclui 0xb
			return Doc()->GetIndex(Doc()->m_myInfo.info.dwGalleryGuid);
		default:
			return GOLFDOC()->m_currentPlayer;
		}
	}
	return Doc()->GetIndex(Doc()->m_myInfo.info.dwGuid);
}

static __forceinline sPlayerData* HcPlayer(unsigned char who)
{
	return PLAYER(who);
}

static __forceinline sPlayerData* HcDocPlayer(CGolfDoc* doc, unsigned char who)
{
	return doc->GetPlayer(who);
}

static __forceinline float HcCurGauge(CGolfDoc* doc, float combo)
{
	float gauge = HcDocPlayer(doc, doc->m_currentPlayer)->gauge;
	float limit = Doc()->GetComboGaugeLimit(doc->m_currentPlayer);
	float value = gauge + combo;
	return value < 0.0f ? 0.0f : (value > limit ? limit : value);
}

static __forceinline char* HcUser(unsigned char who)
{
	return (char*)&Doc()->m_userInfo[who].charInfo;
}

static __forceinline sTeamData* HcTeam(unsigned char who)
{
	return TEAM(who);
}

// B1481: crus do 983 pelo GROFS983 (cl 14.44: m_shotExtra983/m_bSpecialCamDone983 caiam no m_backUpStack -> heap corrompido no push)
#define GR_ULONG(off) (*(unsigned long*)GRPTR983(off))
#define GR_UCHAR(off) (*(unsigned char*)GRPTR983(off))
#define GR_FLOAT(off) (*(float*)GRPTR983(off))

struct CClubEx2 : public CClub
{
	void SelectBestClub(const WVector& pos);
};

class CRivalEx : public WSingleton<CRivalEx>
{
public:
	void SetVariable(CGolfDoc* doc);
};

#pragma comment(linker, \
	"/alternatename:?SelectBestClub@CClubEx2@@QAEXABVWVector@@@Z=?SelectBestClub@CClub@@QAEXABVWVector@@@Z")
#pragma comment(linker, \
	"/alternatename:?SetVariable@CRivalEx@@QAEXPAVCGolfDoc@@@Z=?SetVariable@CRival@@QAEXPAVCGolfDoc@@@Z")
#pragma comment(linker, \
	"/alternatename:?m_pInstance@?$WSingleton@VCRivalEx@@@@2PAVCRivalEx@@A=?m_pInstance@?$WSingleton@VCRival@@@@2PAVCRival@@A")

// HACK: local declarations (adding them to the PCH headers shifts other units)
class CGolfDocEx2 : public CGolfDoc
{
public:
	WMatrix GetDefaultCamera();
};

#pragma comment(linker, \
	"/alternatename:?ReplayCamera@CCameraWorkEx@@QAEX_N@Z=?ReplayCamera@CCameraWork@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?OnChangeTurn@CHelpMsgEx@@QAEXXZ=?OnChangeTurn@CHelpMsg@@QAEXXZ")
class CCameraWorkEx : public CCameraWork
{
public:
	void ReplayCamera(bool);
	void SetCaddieCamera(WMatrix*,
		WVector*); // HACK: placeholder not resolved yet
};

class CHelpMsgEx : public CHelpMsg
{
public:
	void OnChangeTurn();
};

#pragma comment(linker, \
	"/alternatename:?GetDefaultCamera@CGolfDocEx2@@QAE?AVWMatrix@@XZ=?GetDefaultCamera@CGolfDoc@@QAE?AVWMatrix@@XZ")

class CChatMsgEx2 : public CChatMsg
{
public:
	void Close(bool, bool);
};

#pragma comment(linker, \
	"/alternatename:?Close@CChatMsgEx2@@QAEX_N0@Z=?Close@CChatMsg@@QAEX_N0@Z")

struct sReplayNode
{
	sReplayNode* next;
	sReplayNode* prev;
	unsigned long unused;
	unsigned long key;
};

// 983: m_holeStatistics[jogador] (sPangYaUserStatistics 0x109; o 645 tinha 0xeb e os campos depois de dwMatchHole +4/+8)
#define SHOTSTAT(type, f) (Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].f)

struct sObserverUser
{
	char data[0x1b8]; // 983: sRivalData 0x1b8 (645: 400)
};

#define OBS_UINT(e, off) (*(unsigned long*)((e) + (off)))
#define OBS_INT(e, off) (*(int*)((e) + (off)))
#define OBS_UCHAR(e, off) (*(unsigned char*)((e) + (off)))

namespace S5
{
	bool ISCutinItem(unsigned long typeId);
	namespace UTIL
	{
		void SendCutInPacket(unsigned long, short, unsigned long,
			unsigned long, bool); // 983: 5o argumento (s5/utilities.h)
	}
}

// B1475: base m_shotResultMsg (+0x3c4 do 983) segue o membro no cl novo; `off` pode ser variavel
#define SHOTRES(type, off) \
	(*(type*)(GDOCPTR983(GOLFDOC(), 0x3c4) + GOLFDOC()->m_currentPlayer * 0x26 + ((off) - 0x3c4)))

#pragma pack(push, 1)
struct sShotResult
{
	unsigned long oid;
	unsigned long pos[3];
	unsigned char state;
	unsigned char bunker;
	unsigned char replayFlag;
	unsigned long totalPang;
	unsigned long bonusPang;
	unsigned long itemTick;
	unsigned long special;
	unsigned short renderNum;
	unsigned char gpArtifact; // 983: +0x25 (1 = artefato do Grand Prix mexeu no placar; 006847a0)

	// 983 (00684be5): o registro nasce com oid = -1 e o resto zerado
	sShotResult()
		: oid(0xffffffff), state(0), bunker(0), replayFlag(0), totalPang(0), bonusPang(0), itemTick(0), special(0),
		  renderNum(0), gpArtifact(0)
	{
		pos[0] = pos[1] = pos[2] = 0;
	}
};
#pragma pack(pop)
C_ASSERT(sizeof(sShotResult) == 0x26); // 983: EncodeBuffer(&res, 0x26) em 00684c66

// HACK: the static-local guard of CEncryption::Instance is $S34 in the original; the guard number counts the
// guarded statics declared before it, so two unreferenced
// statics of class type (guard + atexit name each; no code, no data) restore the numbering.
#define RB_GUARDPAD(n) \
	static RbGuardPad& f##n() \
	{ \
		static RbGuardPad s; \
		return s; \
	}
struct RbGuardPad
{
	RbGuardPad() { }
	~RbGuardPad() { }
	RB_GUARDPAD(0)
	RB_GUARDPAD(1)
};

class CEncryption
{
private:
	CEncryption() { }
	~CEncryption() { }

public:
	static CEncryption* __fastcall Instance()
	{
		static CEncryption _Instance;
		return &_Instance;
	}
	template <class T>
	void Encrypt(T* data, EncryptKey& key)
	{
		T copy = *data;
		unsigned char* buf = (unsigned char*)&copy;
		int k = 0;
		int i = 0;
		do
		{
			buf[i] ^= key.GetKey(k);
			k++;
			if (k >= 16)
				k = 0;
			i++;
		} while (i < (int)sizeof(T));
		*data = copy;
	}
};

// HACK: include position = emission order (after encryption.h in the original)
#include "treasure.h"
// tmpl order: the original instantiates these const_iterator classes before allocator<AcquiredItemInfo>
typedef char RbShotIterSize[sizeof(std::list<CGolfRule::sShotData>::iterator)];
typedef char RbGuidCIterSize[sizeof(std::list<unsigned long>::const_iterator)];
typedef char RbGuidIterSize[sizeof(std::list<unsigned long>::iterator)];
#include "gatewayactor.h"
typedef char RbAcqCIterSize[sizeof(
	std::vector<FieldItem::AcquiredItemInfo>::const_iterator)];

const char* g_bgmPutting[2] = { "putting.mp3", "putting2.mp3" };
const char* g_plSound[8] = { "shot_best_timing", "shot_best_timing",
	"shot_good_timing", "shot_normal_timing", "shot_bad_timing",
	"\xb0\xb6\xb7\xaf\xb8\xae_\xb9\xda\xbc\xf6", "\xc6\xce\xbe\xdf",
	"pangya_mike" };
const char* g_szCommonCharMotion[8] = {
	"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee"
};
const char* g_szLuciaCharMotion[8] = {
	"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
	"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xb4\xf5\xba\xed\xba\xb8\xb1\xe2\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
	"\xb4\xf5\xba\xed\xba\xb8\xb1\xe2\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee"
};
sLadderItem g_ladderItem[6] = {
	{ 0x00, 15 },
	{ 0x01, 13 },
	{ 0x0a, 18 },
	{ 0x0b, 12 },
	{ 0x04, 18 },
	{ 0x0c, 24 },
};
#include "cardsystem.h"
#include "logininfo.h"
#include "simpleoverlay.h"
#include "jp/replayextension.h"
#include "jp/shottrajectoryformassgame.h" // sMassGameShot (C-PKT lote 16)
#include "jp/grandprixartifactrule.h" // CGrandPrixArtifactRule (C-PKT lote 17)
#include "jp/controllerdoc.h" // CControllerDoc::GetObjectAt (C-PKT lote 19)
#include "jp/pangyacorrectionobject.h" // GetBarCondition do 983 (C-PKT lote 19)
#include "jp/pangyaaddcorrectionobj.h"
#include "jp/pangyacorrectionsetobject.h"
#include "jp/combogaugeeveryshotobject.h" // IsHit/GetPercent do 983 (ChangeGameMode 0068ee8e)
#include "jp/gamestringmanager.h" // GetGameString 00540a10
#include "jp/grandprixawarddlg.h" // FrGrandPrixAwardDlg::s_pInstance (B72)

#define CURPLAYER_ULONG(off) \
	(*(unsigned long*)((char*)PLAYER(GOLFDOC()->m_currentPlayer) + (off)))
#define CURPLAYER_UCHAR(off) \
	(*(unsigned char*)((char*)PLAYER(GOLFDOC()->m_currentPlayer) + (off)))

#define PLAYER_NEXT_STROKE(p, h) /* 983: sPlayerData::stroke[] em +0x2c (645 +0x20) */ \
	((unsigned char)~(((unsigned char*)(p))[((h) + 1) * 0x10 + 0xc + 0xe] ^ \
		((unsigned char*)(p))[((h) + 1) * 0x10 + 0xc]))

#pragma pack(push, 1)
struct sHitShotInfo
{
	unsigned long field0;
	unsigned long field1;
	float spinX;
	float spinY;
	unsigned char type;
	unsigned long ballFlags;
	unsigned long mesh;
	unsigned long player;
	int time;
	unsigned long field3f;
	unsigned char clubType;
	unsigned long clubValue;
	unsigned long extra;
};
#pragma pack(pop)

#define BALL_FLOAT(off) (*(float*)((char*)&GolfBall() + (off)))
#define BALL_ULONG(off) (*(unsigned long*)((char*)&GolfBall() + (off)))

#define HITSHOT_ITEMS() (*(std::list<unsigned long>*)GDOCPTR983(GOLFDOC(), 0x7c))

#define HitShotSend(self, name, msg) \
	do \
	{ \
		const MsgObject& rb_msg = msg; \
		IActor* rb_actor = (self)->GetActor(name); \
		if (rb_actor != NULL) \
			rb_actor << rb_msg; \
	} while (0)

// HACK: not declared in scenemanager.h (same trick as UpdateWind above)
class CSceneManagerLod : public CSceneManager
{
public:
	void SetLOD(bool, bool);
};

#pragma comment(linker, \
	"/alternatename:?SetLOD@CSceneManagerLod@@QAEX_N0@Z=?SetLOD@CSceneManager@@QAEX_N0@Z")

extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
void SaveBMP(const char* filename, const Bitmap* bitmap);
extern int g_last_screen_width;
extern int g_last_screen_height;

#define CAPTURE_WIND_HIDE() \
	do \
	{ \
		unsigned char* w = (unsigned char*)&Wind(); \
		w[0xb8] = ~w[0xc6]; \
		w[0xb9] = ~w[0xc6] ^ (unsigned char)(w + 0xb8); \
	} while (0)

#define CAPTURE_WIND_SHOW() \
	do \
	{ \
		unsigned char* w = (unsigned char*)&Wind(); \
		w[0xb8] = w[0xc6] ^ 0xfe; \
		w[0xb9] = w[0xc6] ^ 0xfe ^ (unsigned char)(w + 0xb8); \
	} while (0)

// HACK: the int overload is dead; it only emits std::find<..., int> (COMDAT owned by this unit)
#define HasMyItem(tid) \
	((0 ? std::find(Doc()->m_myItemList.begin(), Doc()->m_myItemList.end(), \
			  (int)(tid)) \
		: std::find(Doc()->m_myItemList.begin(), Doc()->m_myItemList.end(), \
			  (unsigned long)(tid))) != Doc()->m_myItemList.end())

#define GR_BYTE(off) (*(unsigned char*)GRPTR983(off))
#define GR_ULONG(off) (*(unsigned long*)GRPTR983(off))

#define GB_ULONG(off) (*(unsigned long*)((char*)&GolfBall() + (off)))

#define DcPlayer(who) PLAYER((unsigned char)(who))

#define DC_BALL_ULONG(off) (*(unsigned long*)((char*)&ball + (off)))

#define DcIsPowerMode(mode) \
	((mode) == 2 || (mode) == 6 || (mode) == 4 || (mode) == 10)

template <class T>
T __fastcall Abs(T value);

#define GD_PLAYER_ITEM(p) ((int)(p)->item) // 983: sPlayerData::item (645: 0x4b0)

static __forceinline sPlayerData* DgPlayer(unsigned char who)
{
	return PLAYER(who);
}

static __forceinline int DgItem(unsigned char who)
{
	return GD_PLAYER_ITEM(PLAYER(who));
}

// HACK: CFx members not declared in the PCH header (fx.obj placeholders)
class CFxEx : public CFx
{
public:
	void MakeBoosterWarp(CGolfBall* ball, char type);
	void* FindControl(unsigned long id);
	unsigned long MakePowerSpin(CGolfBall* ball);
};

#pragma comment(linker, \
	"/alternatename:?MakeBoosterWarp@CFxEx@@QAEXPAVCGolfBall@@D@Z=?MakeBoosterWarp@CFx@@QAEHAAVCGolfBall@@E@Z")
#pragma comment(linker, \
	"/alternatename:?FindControl@CFxEx@@QAEPAXK@Z=?FindControl@CFx@@QAEPAVCFxControl@@H@Z")
#pragma comment(linker, \
	"/alternatename:?MakePowerSpin@CFxEx@@QAEKPAVCGolfBall@@@Z=?MakePowerSpin@CFx@@QAEHAAVCGolfBall@@@Z")

// HACK: CTask is not declared; slot 7 of its vtable steps the task
class RbTask
{
public:
	virtual void s0() = 0;
	virtual void s1() = 0;
	virtual void s2() = 0;
	virtual void s3() = 0;
	virtual void s4() = 0;
	virtual void s5() = 0;
	virtual void s6() = 0;
	virtual void Step(int type, float dt, int frame) = 0;
};

#define DBG_FLOAT(off) (*(float*)GRPTR983(off)) // B1488: era (char*)this + off (0x484..0x498 andam no cl 14.44)
#define DBG_EYE m_dbgEye983 // 983 +0x248 (645: 0x1ec)

#define HP_POS m_holePos983 // 983 +0x2b4 (645: 0x258; lea [esi+0x2b4] em 00687c91)

#define HpPlayer(who) PLAYER((unsigned char)(who))

#define HpTeam(who) TEAM((unsigned char)(who))

#define RrPlayer(who) PLAYER((int)(who))

#define RrTeam(who) TEAM((int)(who))

#define GR_FLOAT2(off) (*(float*)GRPTR983(off))

extern int g_iSendFailType;
extern int g_iRecvDisconCode;

// HACK: ProcessObserver is a naked/hybrid function ending in __assume(0); a direct call would make the compiler drop the
// rest of OnProcess as unreachable, so call it through an alias the compiler cannot see through.
#pragma comment(linker, \
	"/alternatename:@rb_ProcessObserver_call@4=?ProcessObserver@CGolfRule@@AAEXXZ")

static __forceinline void OpNotice(const char* text)
{
	CChatMsg::Instance()->AddChatMsg(std::string(text), 0xffff7878, true,
		false);
}

// HACK: S5::UTIL::GetHoleinCoinCount is not declared (utilities.obj placeholder)
namespace S5
{
	namespace UTIL
	{
		int __fastcall GetHoleinCoinCount(int coin, int type);
	}
}

#pragma comment(linker, \
	"/alternatename:?GetHoleinCoinCount@UTIL@S5@@YIHHH@Z=__pg_c_009d31")

// HACK: local declarations (the dialog lives in bongdarishopdlg, its header is not reconstructed yet)
struct sBonusBall;
class FrClosedBonusBallResultDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrClosedBonusBallResultDlg)

	FrClosedBonusBallResultDlg();
	virtual ~FrClosedBonusBallResultDlg();

	void SetClosedBonusBall(sBonusBall* ball);

	unsigned char m_unused[0x250 - sizeof(FrForm)];
};

// HACK: m_RTTI of the dialog lives inside a still delinked object (bongdarishopdlg): local/gr_fix_bongdari.py names it there

// B1489: era (char*)this + (off) cru; no cl 14.44 o CTaskMain anda -4 (BaseObject) e o resto do CGolfRule anda pelo GROFS983.
// Os tres offsets do CTaskMain usados aqui viram membros (C_ASSERT_VC10 no TopViewClick); `off` constante.
#define GR_WND(off) (*(FrWnd**)((off) == 0x48 ? (char*)&m_pOverBarSpecialCard : (off) == 0x128 ? (char*)&m_pMessengerBubbleEdit : \
	(off) == 0x13c ? (char*)&m_pMessengerBtn : GRPTR983(off)))
#define GR_WND983(off) (*(FrWnd**)GRPTR983(off)) // B1481: offsets >= 0x218 (andam no cl 14.44)
#define GR_WND_S(m) (*(FrWnd**)&(m)) // 983: botao que saiu do CTaskMain (estatico NULL no taskmain.h)
#define SHOTRES_ULONG(off) \
	(*(unsigned long*)(GDOCPTR983(GOLFDOC(), 0x3c4) + GOLFDOC()->m_currentPlayer * 0x26 + ((off) - 0x3c4)))
#define SHOTRES_UCHAR(off) \
	(*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x3c4) + GOLFDOC()->m_currentPlayer * 0x26 + ((off) - 0x3c4)))
#define ROW_UCHAR(p, off) (*((unsigned char*)(p) + (off)))
// A175: o 983 le FrEdit+0x90 (m_pScrBar) -> +0xe0 (m_rowCapacity) (HandleMsg 0069913d); os +0x7c/+0xe4 eram do 645 (AV no robo a175-1)
#define CHATVIEW_LINES() (m_pChatView->GetScrollBar()->GetRowCapacity())

// HACK: .rdata blob __pg_c_00d622 (clear bonus rates + g_PI, g_DEGTORAD, g_HUGE), one object. The compiler emits
// g_EPSILON, BAR_MIN, BAR_END and BAR_START right after it (same section), which is where the asm expects them
// (__pg_c_00d622 + 0x5c..0x68).
extern "C" const int _pg_c_00d622[23];
extern "C" const int _pg_c_00d622[23] = { 0x14, 0x32, 0x37, 0x50, 0x41, 0x18,
	0x32, 0x46, 0x28, 0x37, 0x28, 0x14, 0, 0x50, 0x14, 0x14, 0x28, 0x28, 0,
	0x28, 0x40490fdb, 0x3c8efa35, 0x7f7fffff };
// 983 00832f20 (std::map global 00e78e60, mapa fora dela = 0): o blob acima e do 645, com 0 no mapa 18 e sem 20/21
int ClearBonusRate983(unsigned char map)
{
	static const int s_rate[22] = { 0x14, 0x32, 0x37, 0x50, 0x41, 0x18, 0x32, 0x46, 0x28, 0x37, 0x28,
		0x14, 0, 0x50, 0x14, 0x14, 0x28, 0x28, 0x46, 0x28, 0x28, 0x28 };
	return map < 22 ? s_rate[map] : 0;
}

// ChangeGameMode (run 43): cases 0x400000/0x800000/0x200000/0x80000/0x40000/0x4000/0x1000/0x2000/0x800/0x20/0x10/0x200/0x400/0x100
// are written; cases 1, 2, 4, 0x40, 0x80, 0x10000, 0x20000, 0x100000 are still missing.
#define CAMERAWORK_ULONG(off) \
	(*(unsigned long*)((char*)CCameraWork::Instance() + (off)))
unsigned char GetEndHole();
#define GR_BALL_FLOAT(b, off) (*(float*)((char*)(b) + (off)))
#define GR_BALL_ULONG(b, off) (*(unsigned long*)((char*)(b) + (off)))
#define GR_PD_ULONG(p, off) (*(unsigned long*)((char*)(p) + (off)))
#define GOLFDOC_U64(off) (*(unsigned __int64*)GDOCPTR983(GOLFDOC(), off)) // B1457: offset do 983 segue o membro (cl novo)

static __forceinline WVector CgLiftPos(const WVector& pos, float lift)
{
	return WVector(pos.x, lift + pos.y, pos.z);
}
// HACK: CWind/CQuadTree/util members not declared in their headers (PCH canaries)
class CWindEx2 : public CWind
{
public:
	void Init(unsigned char, unsigned char, unsigned char, unsigned char);
	void MakeWindList();
};

#define WINDEX2() ((CWindEx2&)Wind())
#pragma comment(linker, \
	"/alternatename:?Init@CWindEx2@@QAEXEEEE@Z=?Init@CWind@@QAEXEEEE@Z")
#pragma comment(linker, \
	"/alternatename:?MakeWindList@CWindEx2@@QAEXXZ=?MakeWindList@CWind@@QAEXXZ")

bool IsCurMapSnow();
int GalleryMode();

// HACK: CFx/CCameraWork/Bar helpers of the shot-effect mode (0x80); placeholders matched by call order
class CFxEx2 : public CFx
{
public:
	unsigned long MakeShotImpact(CGolfBall* ball);
	void MakeClubImpact(CGolfBall* ball, unsigned long id);
	void MakeCobra(CGolfBall* ball);
	void MakeSpike(CGolfBall* ball, bool power);
	void MakeTomahawk(CGolfBall* ball, bool power, unsigned long itemId);
	void MakePowerShot2(CGolfBall* ball);
};

#pragma comment(linker, \
	"/alternatename:?MakeShotImpact@CFxEx2@@QAEKPAVCGolfBall@@@Z=?MakeShotImpact@CFx@@QAEHAAVCGolfBall@@@Z")
#pragma comment(linker, \
	"/alternatename:?MakeClubImpact@CFxEx2@@QAEXPAVCGolfBall@@K@Z=?MakeClubImpact@CFx@@QAEHAAVCGolfBall@@K@Z")
#pragma comment(linker, \
	"/alternatename:?MakeCobra@CFxEx2@@QAEXPAVCGolfBall@@@Z=?MakeCobra@CFx@@QAEHABVCGolfBall@@@Z")
#pragma comment(linker, \
	"/alternatename:?MakeSpike@CFxEx2@@QAEXPAVCGolfBall@@_N@Z=?MakeSpike@CFx@@QAEHABVCGolfBall@@_N@Z")
#pragma comment(linker, \
	"/alternatename:?MakeTomahawk@CFxEx2@@QAEXPAVCGolfBall@@_NK@Z=?MakeTomahawk@CFx@@QAEHABVCGolfBall@@_NK@Z")
#pragma comment(linker, \
	"/alternatename:?MakePowerShot2@CFxEx2@@QAEXPAVCGolfBall@@@Z=?MakePowerShot2@CFx@@QAEHAAVCGolfBall@@@Z")

class CCameraWorkEx2 : public CCameraWork
{
public:
	void Quake();
};

#pragma comment(linker, \
	"/alternatename:?Quake@CCameraWorkEx2@@QAEXXZ=?Quake@CCameraWork@@QAEXXZ")

sBar& __fastcall Bar();

// The current player is read through a reference in ChangeGameMode (the doc pointer is
// loaded before the right-hand side and the byte after it); everywhere else by value.
inline unsigned char& CgmCurrentPlayer(CGolfDoc* doc)
{
	return doc->m_currentPlayer;
}
#define CGM_CUR() CgmCurrentPlayer(GOLFDOC())

// HACK: the original never expands Bar(), CGolfBall::PreData(), PreDataToWrite() and CQuadTree::SetFrame() inside
// ChangeGameMode (its inliner budget is spent there); bodiless aliases keep them as calls.
sBar& __fastcall CgmBar();
#pragma comment(linker, \
	"/alternatename:?CgmBar@@YIAAUsBar@@XZ=?Bar@@YIAAUsBar@@XZ")
#ifdef JP983_VS_DEBUG // D843: /Od nao emite Bar() (alvo do alias); o endereco obriga o COMDAT
extern sBar& (__fastcall* const g_jp983DbgBar)() = &Bar;
#endif
struct CgmGolfBall
{
	CGolfBall::sExpected* PreDataToWrite(int index);
	const CGolfBall::sExpected* PreData(int index) const;
};
#define CGM_BALL(b) (*(CgmGolfBall*)&(b))
struct CgmQuadTree
{
	void SetFrame(unsigned long frame);
};
#define CGM_PVS() (*(CgmQuadTree*)&GetPVS())
#pragma comment(linker, \
	"/alternatename:?SetFrame@CgmQuadTree@@QAEXK@Z=?SetFrame@CQuadTree@@QAEXK@Z")
#pragma comment(linker, \
	"/alternatename:?PreDataToWrite@CgmGolfBall@@QAEPAUsExpected@CGolfBall@@H@Z=?PreDataToWrite@CGolfBall@@QAEPAUsExpected@1@H@Z")
#pragma comment(linker, \
	"/alternatename:?PreData@CgmGolfBall@@QBEPBUsExpected@CGolfBall@@H@Z=?PreData@CGolfBall@@QBEPBUsExpected@1@H@Z")

#define CLUB_TYPE() (GolfClub().m_pClub->type) // CClub+4 m_pClub, sClub+0x22 type (B1532)

static const unsigned long s_impactFxDummy = 0;

#define GR_WND_SHOW(off, v) \
	do \
	{ \
		if (GR_WND(off)) \
			GR_WND(off)->SetVisible(v); \
	} while (0)

// HACK: helpers of the shot mode (0x40); placeholders not resolved yet
#define GR_BALL_UCHAR(b, off) (*(unsigned char*)((char*)(b) + (off)))
// CClub+0x2c m_powerShot: WCrypticValue m_key32 (+8, privado) XOR m_data (+0) = ~tipo (B1532)
#define CLUB_XOR() \
	(*(unsigned int*)((char*)&GolfClub().m_powerShot + 8) ^ *(unsigned int*)&GolfClub().m_powerShot)
#define CLUB_PS() ((int)GolfClub().m_powerShot)
#define PTR_AUTOTARGET(p) (p)

class CQuadTreeEx2 : public CQuadTree
{
public:
	int GetObjColNum();
	void ResetVariable();
	float GetBasePosY(WVector pos, float radius);
	bool IsOverWater(WVector pos);
};

class CClubEx3 : public CClub
{
public:
	float GetLie(char* name);
	void SetApproachMode();
};

#pragma comment(linker, \
	"/alternatename:?GetObjColNum@CQuadTreeEx2@@QAEHXZ=?GetObjColNum@CQuadTree@@QAEHXZ")
#pragma comment(linker, \
	"/alternatename:?ResetVariable@CQuadTreeEx2@@QAEXXZ=?ResetVariable@CQuadTree@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?GetBasePosY@CQuadTreeEx2@@QAEMVWVector@@M@Z=?GetBasePosY@CQuadTree@@QAEMVWVector@@M@Z")
#pragma comment(linker, \
	"/alternatename:?IsOverWater@CQuadTreeEx2@@QAE_NVWVector@@@Z=?IsOverWater@CQuadTree@@QAE_NVWVector@@@Z")
#pragma comment(linker, \
	"/alternatename:?GetLie@CClubEx3@@QAEMPAD@Z=?GetLie@CClub@@QAEMPBD@Z")
#pragma comment(linker, \
	"/alternatename:?SetApproachMode@CClubEx3@@QAEXXZ=?SetApproachMode@CClub@@QAEXXZ")
#pragma comment(linker, \
	"/alternatename:?SetCaddieCamera@CCameraWorkEx@@QAEXPAVWMatrix@@PAVWVector@@@Z=?SetCaddieCamera@CCameraWork@@QAEXABVWMatrix@@ABVWVector@@@Z")

class CQuadTreeEx3 : public CQuadTree
{
public:
	float GetMinPowerOnGround();
	float GetMaxPowerOnGround();
};

// (these helpers were in an anonymous namespace in the original; its hash depends on the build
// directory, so the naked functions could not name them through an alias)
struct BallSpeedFactor
{
	float f0;
	float f4;
	float f8;
	float fc;
	WVector vel;
	BallSpeedFactor(unsigned long item, CGolfBall* ball, float scale);
};

int __fastcall GetBarClickTiming();
CGolfBall::eBallState __fastcall GetInitBallState();
float __fastcall GetBarClickPowerFactor();
#define CGM_CUR_ULONG(off) \
	(*(unsigned long*)((char*)PLAYER(GOLFDOC()->m_currentPlayer) + (off)))
#define CGM_CUR_UCHAR(off) \
	(*(unsigned char*)((char*)PLAYER(GOLFDOC()->m_currentPlayer) + (off)))
#define CGM_CUR_PD() PLAYER(GOLFDOC()->m_currentPlayer)
static __forceinline unsigned char CgLevelGrade(unsigned char level)
{
	if (level < 6)
		return 1;
	return (level - 1) / 5 + 1;
}
// 983 0067e200: item da vez (sPlayerData+0x4bc) e um dos 5 itens de forca (sem par no 645)
static bool IsPowerItem983()
{
	unsigned long item = CGM_CUR_PD()->item;
	return item == 0x1800000a || item == 0x18000007 || item == 0x18000002 ||
		item == 0x18000012 || item == 0x1800002b;
}

// HACK: lives in an anonymous namespace in the original (its hash depends on the build directory)
struct sDetermineWhetherToEnableFiledItem
{
	~sDetermineWhetherToEnableFiledItem();
};

#define EXTPRIZE(player, field) \
	(((sPrizeInfo*)((char*)(player)->extPrize + off))->field)

// The motion grade of a shot: power shot 4, else 2 for the second flag, plus
// one for the bonus flag (at most 5), looked up in the special or normal table.
int __stdcall GetShotMotionGrade(bool special, bool powerShot, bool secondFlag,
	bool bonus)
{
	int grade = 0;
	if (powerShot)
		grade = 4;
	else if (secondFlag)
		grade = 2;
	if (bonus)
		grade++;
	if (grade >= 5)
		grade = 5;
	if (special)
	{
		int grades[6] = { 12, 3, 14, 3, 3, 3 };
		return grades[grade];
	}
	else
	{
		int grades[6] = { 4, 3, 4, 3, 3, 3 };
		return grades[grade];
	}
}

// The character motion for a hole result: hole in one, -3..+3 against par, or
// the "other" entry. Lucia (type 8) has her own set.
const char* __fastcall GetScoreCharMotion(int diff, int charType, int holeIn) // 983 0066e1a0: 3 parametros (ecx, edx, pilha)
{
	int index = 0;
	if (holeIn != 1 && (diff < -3 || diff > 3 || (index = diff + 4) >= 7))
		index = 7;
	if (charType == 8 || charType == 10) // 983 0066e1a0: tid 8 e 10
		return g_szLuciaCharMotion[index];
	return g_szCommonCharMotion[index];
}

// The ball state a shot starts with, from the club type, the character and
// which side of the bar's centre the impact fell on (only for a good grade).
CGolfBall::eBallState __fastcall GetInitBallState()
{
	char clubType = CLUB_TYPE();
	if (clubType == 3)
		return (CGolfBall::eBallState)3;
	unsigned int charType =
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
		0x3ffffff;
	if (GOLFDOC()->GetBar().grade < 2)
	{
		CGolfDoc* doc = GOLFDOC();
		bool over = doc->GetBar().impact > doc->GetBar().center;
		if (clubType == 2)
			return (CGolfBall::eBallState)(over ? 3 : 1);
		if (clubType == 0)
		{
			if (over || charType == 1 || charType == 5 || charType == 6)
				return (CGolfBall::eBallState)0;
			return (CGolfBall::eBallState)3;
		}
		if (clubType == 1)
		{
			if (over)
				return (CGolfBall::eBallState)3;
			if (charType == 5 || charType == 6)
				return (CGolfBall::eBallState)0;
		}
	}
	return (CGolfBall::eBallState)1;
}

int __fastcall GetBarClickTiming()
{
	CGolfDoc* doc = GOLFDOC();
	switch (doc->GetBar().grade)
	{
	case 3:
		return doc->GetBar().impact - doc->GetBar().center > 0.0f ? 1 : -1;
	case 2:
		return doc->GetBar().impact > BAR_START ? 2 : -2;
	case 1:
	case 0:
		return doc->GetBar().impact - doc->GetBar().center > 0.0f ? 3 : -3;
	}
	return 0;
}

// The power factor of a bar click: full power for a putter or an unknown
// grade, else the root of the ground power blended by the click's error.
float __fastcall GetBarClickPowerFactor()
{
	if (CLUB_TYPE() != 3)
	{
		CGolfDoc* doc = GOLFDOC();
		switch (doc->GetBar().grade)
		{
		case 4:
			return sqrtf(GetPVS().GetMaxPowerOnGround());
		case 3:
		{
			float delta = doc->GetBar().impact - doc->GetBar().center;
			float ratio;
			if (delta > 0.0f)
				ratio = delta /
					(((CClubEx*)&GolfClub())->GetNarrowAccuracy() + BAR_START -
						doc->GetBar().center);
			else
				ratio = delta /
					(doc->GetBar().center -
						(BAR_START -
							((CClubEx*)&GolfClub())->GetNarrowAccuracy()));
			return sqrtf(GetPVS().GetMaxPowerOnGround() * (1.0f - Abs(ratio)) +
				GetPVS().GetMinPowerOnGround() * Abs(ratio));
		}
		case 2:
		{
			float ratio = (Abs(doc->GetBar().impact - BAR_START) -
							  ((CClubEx*)&GolfClub())->GetNarrowAccuracy()) /
				(((CClubEx*)&GolfClub())->GetWideAccuracy() -
					((CClubEx*)&GolfClub())->GetNarrowAccuracy());
			return sqrtf(GetPVS().GetMinPowerOnGround() * (1.0f - ratio) +
				GetPVS().GetMinPowerOnGround() * ratio);
		}
		}
	}
	return 1.0f;
}

// The spin/curve factors of a shot from the bar click: the grade of the click,
// the side of the impact, the club type and the character's special shots.
BallSpeedFactor::BallSpeedFactor(unsigned long item, CGolfBall* ball,
	float scale)
	: f0(0.0f),
	  f4(0.0f),
	  f8(ball->m_initCurve),
	  fc(ball->m_initSpin)
{
	vel.Reset();
	AfxGetTask()->GetMainActor()->SendMsg(NULL, "Player", M983(0x110), (int)&f4, 0, 0,
		0);
	f4 = scale + f4;
	if (CLUB_TYPE() == 3)
		return;
	if (GOLFDOC()->GetBar().grade == 4)
		f0 -= f8 * 0.3490658476948738;
	if (GOLFDOC()->GetBar().grade == 3)
	{
		CGolfDoc* doc = GOLFDOC();
		float delta = doc->GetBar().impact - doc->GetBar().center;
		float ratio;
		if (delta > 0.0f)
			ratio = delta /
				(((CClubEx*)&GolfClub())->GetNarrowAccuracy() + BAR_START -
					doc->GetBar().center);
		else
			ratio = delta /
				(doc->GetBar().center -
					(BAR_START - ((CClubEx*)&GolfClub())->GetNarrowAccuracy()));
		f0 -= f8 * 0.3490658476948738;
		if (!IsLocalContent((localContentType_t)0x51 /*983: era 0x50; medido*/) ||
			ball->m_initSpecial == 0)
			f8 -= ratio * 0.2f;
	}
	if (GOLFDOC()->GetBar().grade == 2)
	{
		float ratio = (Abs(GOLFDOC()->GetBar().impact - BAR_START) -
						  ((CClubEx*)&GolfClub())->GetNarrowAccuracy()) /
			(((CClubEx*)&GolfClub())->GetWideAccuracy() -
				((CClubEx*)&GolfClub())->GetNarrowAccuracy());
		if (!IsLocalContent((localContentType_t)0x51 /*983: era 0x50; medido*/) ||
			ball->m_initSpecial == 0)
		{
			f8 = 0.2f;
			fc = 0.0f;
		}
		if (GOLFDOC()->GetBar().impact > BAR_START)
		{
			f8 *= -1.0f;
			f0 -= ratio * 0.1745329238474369;
		}
		else
			f0 += ratio * 0.1745329238474369;
	}
	if (GOLFDOC()->GetBar().grade == 0 || GOLFDOC()->GetBar().grade == 1)
	{
		f8 = 0.0f;
		fc = 0.0f;
		switch (CLUB_TYPE())
		{
		case 2:
			if (GOLFDOC()->GetBar().impact > GOLFDOC()->GetBar().center)
				vel = WVector(0.0f, 0.0f, 5.0f);
			else
				vel = RotMat(1.3962634f, 0).za * 20.0f;
			break;
		case 1:
			if (GOLFDOC()->GetBar().impact > GOLFDOC()->GetBar().center)
				vel = WVector(0.0f, 0.0f, 20.0f);
			else
				vel = RotMat(1.3962634f, 0).za * 50.0f;
			if (item == 0x4000006 || item == 0x4000005)
				vel.Reset();
			break;
		case 0:
			if (GOLFDOC()->GetBar().impact > GOLFDOC()->GetBar().center ||
				item == 0x4000001 || item == 0x4000006 || item == 0x4000005)
				vel.Reset();
			else
			{
				vel = WVector(0.0f, 0.0f, 15.0f);
				f0 += 0.6981316953897476;
			}
			break;
		}
	}
	if (GOLFDOC()->GetBar().grade <= 1)
		return;
	if (GOLFDOC()->GetBar().grade != 4 || !GOLFDOC()->GetBar().bFullPower ||
		CLUB_PS() == 0 || (*((unsigned char*)ball + 0x44) & 0x70) != 0 ||
		!GOLFDOC()->IsCurrentPlayerEvent983(6)) // 983 00672700 -> 00663870(6); o 645 testava item 0x18000028
	{
		WVector screen;
		AfxGetTask()->GetMainActor()->SendMsg(NULL, "Screen", M983(0x1c1),
			(int)&screen, 0, 0, 0);
		if (*((unsigned char*)ball + 0x44) & 0x40)
			f8 -= screen.x * 0.15f;
		else
			f8 -= screen.x * 0.5f;
	}
}

// When field items are on the ground: tells the item manager whether they stay
// enabled (strokes of this hole still under par) and announces when they go.
sDetermineWhetherToEnableFiledItem::~sDetermineWhetherToEnableFiledItem()
{
	int count = 0;
	AfxGetTask()->GetActor("GroundItemMan")
		<< MsgObject(NULL, 0x2f7, (int)&count, 0, 0, 0, 0);
	if (count > 0)
	{
		int stroke;
		unsigned char index;
		unsigned char gameType = Doc()->m_golfGame.gameType;
		if (gameType == 1)
		{
			index = PLAYER(GOLFDOC()->m_currentPlayer)->team;
			stroke = TEAM(index)->stroke[GOLFDOC()->m_currentHole - 1];
		}
		else
		{
			index = GOLFDOC()->m_currentPlayer;
			stroke = PLAYER(index)->stroke[GOLFDOC()->m_currentHole - 1];
		}
		int enable = 1;
		if (stroke - GOLFDOC()->GetHoleData().par >= 0)
		{
			// 983 (00681600): so avisa na virada (consulta 0x2f6 = ainda ligado)
			char wasEnabled = 0;
			enable = 0;
			AfxGetTask()->GetActor("GroundItemMan")
				<< MsgObject(NULL, 0x2f6, index, (int)&wasEnabled, 0, 0, 0);
			if (wasEnabled)
				CChatMsg::Instance()->AddChatMsg(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2) \xb1\xd4\xc1\xa4 \xc5\xb8\xbc\xf6 \xc0\xcc\xbb\xf3\xc0\xcc \xb5\xc7\xbe\xee \xc7\xca\xb5\xe5 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbb\xe7\xb6\xf3\xc1\xfd\xb4\xcf\xb4\xd9."),
					0xffff7878, false, false);
		}
		AfxGetTask()->GetActor("GroundItemMan")
			<< MsgObject(NULL, M983(0x27b), enable, index, 0, 0, 0);
	}
}

IMPLEMENT_ACTOR(CGolfRule, CTaskMain)

BEGIN_FRESH_MSGMAP(CGolfRule, CTaskMain)
ON_FRESH_VI("MAIN.chatview", FRCMD_INIT, CGolfRule::OnChatViewInit)
ON_FRESH_VI("MAIN.emoticon", FRCMD_INIT, CGolfRule::OnEmoticonInit)
ON_FRESH_VV("MAIN.emoticon", FRCMD_LBUTTONUP, CGolfRule::OnEmoticonBtnUp)
ON_FRESH_VI("MAIN.language", FRCMD_INIT, CGolfRule::OnLanguageInit)
ON_FRESH_VI("MAIN.report", FRCMD_INIT, CGolfRule::OnReportInit)
ON_FRESH_VV("MAIN.report", FRCMD_LBUTTONUP, CGolfRule::OnReportBtnUp)
ON_FRESH_VI("MAIN.chattarget", FRCMD_INIT, CGolfRule::OnChatTargetInit)
ON_FRESH_VI("MAIN.chattarget", FRCMD_LBUTTONDOWN,
	CGolfRule::OnChatTargetBtnDown)
ON_FRESH_BI("MAIN.chattarget", FRCMD_ENTERKEY, CGolfRule::OnChatTargetEnterKey)
ON_FRESH_VI("MAIN.chatinput", FRCMD_INIT, CGolfRule::OnChatInputInit)
ON_FRESH_BI("MAIN.chatinput", FRCMD_ENTERKEY, CGolfRule::OnChatInputEnterKey)
ON_FRESH_VI("MAIN.spcard", FRCMD_INIT, CGolfRule::OnSpecialCardInit)
ON_FRESH_VV("MAIN.spcard", FRCMD_LBUTTONUP, CGolfRule::OnSpecialCardBtnUp)
ON_FRESH_VI("MAIN.spcard", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.spcard", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.myinfo", FRCMD_INIT, CGolfRule::OnMyInfoInit)
ON_FRESH_VV("MAIN.myinfo", FRCMD_LBUTTONUP, CGolfRule::OnMyInfoBtnUp)
ON_FRESH_VI("MAIN.myinfo", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.myinfo", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.option", FRCMD_INIT, CGolfRule::OnOptionInit)
ON_FRESH_VV("MAIN.option", FRCMD_LBUTTONUP, CGolfRule::OnOptionBtnUp)
ON_FRESH_VI("MAIN.option", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.option", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.caddiebook", FRCMD_INIT, CGolfRule::OnCaddieBookInit)
ON_FRESH_VV("MAIN.caddiebook", FRCMD_LBUTTONUP, CGolfRule::OnCaddieBookBtnUp)
ON_FRESH_VI("MAIN.caddiebook", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.caddiebook", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.unplayable", FRCMD_INIT, CGolfRule::OnUnplayableInit)
ON_FRESH_VV("MAIN.unplayable", FRCMD_LBUTTONUP, CGolfRule::OnUnplayableBtnUp)
ON_FRESH_VI("MAIN.unplayable", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.unplayable", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.pause", FRCMD_INIT, CGolfRule::OnPauseInit)
ON_FRESH_VV("MAIN.pause", FRCMD_LBUTTONUP, CGolfRule::OnPauseBtnUp)
ON_FRESH_VI("MAIN.pause", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.pause", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.record", FRCMD_INIT, CGolfRule::OnChatRecordInit)
ON_FRESH_VV("MAIN.record", FRCMD_LBUTTONUP, CGolfRule::OnChatRecordBtnUp)
ON_FRESH_VI("MAIN.record", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.record", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.onelineview", FRCMD_INIT, CGolfRule::OnOnelineViewInit)
ON_FRESH_VI("MAIN.onelineview", FRCMD_OWNERDRAW,
	CGolfRule::OnOnelineViewOwnerDraw)
ON_FRESH_VI("MAIN.onelinebtn", FRCMD_INIT, CGolfRule::OnOnelineBtnInit)
ON_FRESH_VV("MAIN.onelinebtn", FRCMD_LBUTTONUP, CGolfRule::OnOnelineBtnUp)
ON_FRESH_VI("MAIN.tooltip", FRCMD_OWNERDRAW, CTaskMain::OnBarTooltipOwnerDraw)
ON_FRESH_VI("MAIN.messenger", FRCMD_INIT, CGolfRule::OnMessengerBtnInit)
ON_FRESH_VV("MAIN.messenger", FRCMD_LBUTTONUP, CGolfRule::OnMessengerBtnUp)
ON_FRESH_VI("MAIN.messenger", FRCMD_HOVERON, CGolfRule::OnTopBarHoverOn)
ON_FRESH_VI("MAIN.messenger", FRCMD_HOVEROFF, CGolfRule::OnTopBarHoverOff)
ON_FRESH_VI("MAIN.messenger_bubble", FRCMD_INIT,
	CTaskMain::OnMessengerBubbleInit)
ON_FRESH_VI("MAIN.messenger_bubble", FRCMD_OWNERDRAW,
	CTaskMain::OnMessengerBubbleOwnerDraw)
ON_FRESH_VI("MAIN.messenger_desc", FRCMD_INIT,
	CGolfRule::OnMessengerBubbleEditInit)
ON_FRESH_VI("MAIN.messenger_alarm", FRCMD_INIT,
	CGolfRule::OnMessenger_AlarmBtnInit)
ON_FRESH_VV("MAIN.messenger_alarm", FRCMD_LBUTTONUP,
	CGolfRule::OnMessenger_AlarmBtnUp)
ON_FRESH_VI("MAIN.gift_alarm", FRCMD_INIT,
	CGolfRule::OnMessenger_GiftAlarmBtnInit)
ON_FRESH_VV("MAIN.gift_alarm", FRCMD_LBUTTONUP,
	CGolfRule::OnMessenger_GiftAlarmBtnUp)
ON_FRESH_VI("MAIN.replay_status", FRCMD_INIT, CGolfRule::OnReplayStatusInit)
ON_FRESH_VV("MAIN.replay_status", FRCMD_LBUTTONUP, CGolfRule::ReplayBoxOpen)
// 983 (msgmap 00e15160): HOVERON 00671250 so regrava +0x51c e HOVEROFF 0066e860 e vazio -> sem entrada aqui
ON_FRESH_VI("MAIN.move_tee", FRCMD_INIT, CGolfRule::OnMoveTeeInit)
ON_FRESH_VV("MAIN.move_tee", FRCMD_LBUTTONUP, CGolfRule::OnMoveTeeBtnUp)
ON_FRESH_VI("MAIN.predict_normal", FRCMD_INIT,
	CGolfRule::OnRadioPredictNormalShotInit)
ON_FRESH_VI("MAIN.predict_tomahwak", FRCMD_INIT,
	CGolfRule::OnRadioPredictTomahwakShotInit)
ON_FRESH_VI("MAIN.predict_cobra", FRCMD_INIT,
	CGolfRule::OnRadioPredictCobraShotInit)
ON_FRESH_VI("MAIN.predict_spike", FRCMD_INIT,
	CGolfRule::OnRadioPredictSpikeShotInit)
ON_FRESH_VV("MAIN.predict_normal", FRCMD_LBUTTONDOWN,
	CGolfRule::OnRadioPredictNormalShotDown)
ON_FRESH_VV("MAIN.predict_tomahwak", FRCMD_LBUTTONDOWN,
	CGolfRule::OnRadioPredictTomahwakShotDown)
ON_FRESH_VV("MAIN.predict_cobra", FRCMD_LBUTTONDOWN,
	CGolfRule::OnRadioPredictCobraShotDown)
ON_FRESH_VV("MAIN.predict_spike", FRCMD_LBUTTONDOWN,
	CGolfRule::OnRadioPredictSpikeShotDown)
ON_FRESH_VI("MAIN.predict_normal", FRCMD_OWNERDRAW,
	CGolfRule::OnRadioPredictNormalShotDraw)
ON_FRESH_VI("MAIN.predict_tomahwak", FRCMD_OWNERDRAW,
	CGolfRule::OnRadioPredictTomahwakShotDraw)
ON_FRESH_VI("MAIN.predict_cobra", FRCMD_OWNERDRAW,
	CGolfRule::OnRadioPredictCobraShotDraw)
ON_FRESH_VI("MAIN.predict_spike", FRCMD_OWNERDRAW,
	CGolfRule::OnRadioPredictSpikeShotDraw)
ON_FRESH_VV("MAIN", FRCMD_FINISH, CGolfRule::OnMainFinish)
ON_FRESH_VI("MAIN.shot_review_button_pre", FRCMD_INIT, CGolfRule::OnShotReviewPreInit)
ON_FRESH_VV("MAIN.shot_review_button_pre", FRCMD_LBUTTONUP, CGolfRule::OnShotReviewPreBtnUp)
ON_FRESH_VI("MAIN.shot_review_button_next", FRCMD_INIT, CGolfRule::OnShotReviewNextInit)
ON_FRESH_VV("MAIN.shot_review_button_next", FRCMD_LBUTTONUP, CGolfRule::OnShotReviewNextBtnUp)
ON_FRESH_VI("MAIN.premium_service_end_notice", FRCMD_INIT, CGolfRule::OnPremiumServiceEndNoticeInit)
ON_FRESH_VI("MAIN.premium_service_end_notice", FRCMD_OWNERDRAW, CGolfRule::OnPremiumServiceEndNoticeOwnerDraw)
END_FRESH_MSGMAP()

const char* ball_stop_replay_data_name = "save/pangya_bug_report.sav";

void CGolfRule::OnChatViewInit(int param)
{
	m_pChatView = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pChatView)
	{
		m_pChatView->HidePrivacy(true);
		WRect rect = m_pChatView->GetRect();
		int lines;
		// JP 983 (00685270): 5 linhas se !extendido e (+0x1094c == 0xb || +0x1095e == 0 || +0x10937 < 100)
		unsigned char* d983 = (unsigned char*)Doc();
		if (COption::Instance()->gIsChatLineEntended() ||
			!(d983[DOCOFS983(0x1094c)] == 0xb || d983[DOCOFS983(0x1095e)] == 0 || d983[DOCOFS983(0x10937)] < 100))
			lines = 10;
		else
			lines = 5;
		// 983 (00685270): altura = linhas * 19.0 + 8.0 em double (dc 0d 30 16 d1 00 = fmul [00d11630] = 19.0)
		rect.h = (float)(lines * 19.0 + 8.0);
		// JP 983 (00684cb0): base y do chat = medida nomeada d_chat_output_base_y (Calc(W/640, H/480)); sem ela, BarY - 100
		float baseY = COption::Instance()->gGetBar_Y() - 100.0f;
		CUiFrameSetManager::NamedDim* pBase =
			CUiFrameSetManager::Instance()->FindNamedDim("d_chat_output_base_y");
		if (pBase)
		{
			CUiFrameSetManager::Size scl(g_view->GetWidth() / 640.0,
				g_view->GetHeight() / 480.0);
			baseY = pBase->Calc(scl);
		}
		rect.y = baseY - rect.h;
		m_pChatView->SetRect(rect);
		m_pChatView->GetScrollBar()->Resize(lines, 1, false);
		m_pChatView->GetScrollBar()->SetVisible(false);
		this << MsgObject(NULL, M983(0x63), 0, 0, 0, 0, 0);
	}
}

// 983 009a0930: origem da area do chat = ponto p_chat_window_area (arredondado para baixo); sem ele x = 91 e y = BarY + 9
static void GetChatWindowOrigin983(float& cx, float& cy)
{
	cx = 91.0f;
	cy = COption::Instance()->gGetBar_Y() + 9.0f;
	CUiFrameSetManager::NamedPoint* pChat = CUiFrameSetManager::Instance()->FindNamedPoint("p_chat_window_area");
	if (pChat)
	{
		float gw = g_view->GetWidth();
		float gh = g_view->GetHeight();
		WPoint cp;
		pChat->Calc(CUiFrameSetManager::Size(gw / 640.0, gh / 480.0), cp);
		cx = (float)floor(cp.x);
		cy = (float)floor(cp.y);
	}
}

void CGolfRule::OnEmoticonInit(int param)
{
	m_pEmoticonBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pEmoticonBtn)
	{
		m_pEmoticonBtn->SetVisible(false);
		float cx, cy;
		GetChatWindowOrigin983(cx, cy);
		WPoint pos;
		pos.x = (float)(cx + 381.0);
		pos.y = (float)(cy + 7.0);
		m_pEmoticonBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnEmoticonBtnUp()
{
	if (m_pEmoticonDlg == NULL)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this,
			"emoticon", NULL);
		if (m_pEmoticonDlg)
		{
			WPoint pos;
			pos.x = 0.0f;
			pos.y = 0.0f;
			if (m_pEmoticonBtn)
			{
				pos.x = m_pEmoticonBtn->GetRect().x - 200.0f;
				pos.y = m_pEmoticonBtn->GetRect().y - 260.0f;
			}
			if (!g_pFresh->GetManager()->IsKeyExclusive())
				g_pFresh->GetManager()->SetExclusiveKey(true);
			m_pEmoticonDlg->Open((FRESH_PFN_RESULT)&CGolfRule::OnEmoticonResult,
				pos, 3);
		}
	}
	else
		m_pEmoticonDlg->Close((eFormRet)2, true);
}

void CGolfRule::OnLanguageInit(int param)
{
	m_pLanguage = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pLanguage)
	{
		m_pLanguage->SetVisible(false);
		if (!CChatMsg::Instance()->IME_IsImeOpened()) // 983 0066fc35 (0099f050 = GetState(7,0)), B1615
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");
		float cx, cy;
		GetChatWindowOrigin983(cx, cy);
		WPoint pos;
		pos.x = (float)(cx + 401.0);
		pos.y = (float)(cy + 7.0);
		m_pLanguage->MoveWindow(pos);
	}
}

void CGolfRule::OnReportInit(int param)
{
	m_pReportBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pReportBtn)
	{
		m_pReportBtn->SetVisible(false);
		float cx, cy;
		GetChatWindowOrigin983(cx, cy);
		WPoint pos;
		pos.x = (float)(cx + 420.0);
		pos.y = (float)(cy + 7.0);
		m_pReportBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnReportBtnUp()
{
	if (GOLFDOC()->m_bStopRequest)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb0\xd4\xc0\xd3\xbf\xa1 \xb4\xeb\xc7\xd8\xbc\xad \xc0\xcc\xb9\xcc "
			"\xbd\xc5\xb0\xed\xb0\xa1 \xc1\xa2\xbc\xf6\xb5\xc7\xbe\xee \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9."),
			false);
		pForm->Open(NULL, 0);
		return;
	}
	if (m_pReportDlg == NULL)
	{
		m_pReportDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		m_pReportDlg->SetMessage(
			K2L_Compatibility("\xbd\xc5\xb0\xed\xb8\xa6 \xc7\xcf\xbd\xc3\xb8\xe9 \xc7\xf6\xc0\xe7\xb0\xd4\xc0\xd3\xc0\xc7 "
			"\xb4\xeb\xc8\xad \xb3\xbb\xbf\xeb\xc0\xcc \xb8\xf0\xb5\xce \xbf\xee\xbf\xb5\xc0\xda\xbf\xa1\xb0\xd4 "
			"\xbd\xc5\xb0\xed\xb0\xa1 \xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb4\xeb\xc8\xad \xb3\xbb\xbf\xeb \xc1\xdf "
			"\xc3\xb3\xb9\xfa\xc7\xd8\xb4\xe7\xbb\xe7\xc7\xd7\xc0\xcc \xc0\xd6\xb4\xc2 \\c0xffff0000\\c\xb8\xf0\xb5\xe7 "
			"\xbe\xc6\xc0\xcc\xb5\xf0\\c0xff000000\\c\xb4\xc2 \xba\xed\xb7\xb0 \xc3\xb3\xb8\xae\xb8\xa6 "
			"\xb4\xe7\xc7\xcf\xb0\xd4 \xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xbd\xc5\xb0\xed \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		m_pReportDlg->Open((FRESH_PFN_RESULT)&CGolfRule::OnReportResult, 0);
	}
}

void CGolfRule::OnChatTargetInit(int param)
{
	m_pChatTarget = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
	if (m_pChatTarget)
	{
		m_pChatTarget->SetVisible(false);
		eReservedChatPartner partner;
		if (Doc()->m_golfGame.gameType == 5 || Doc()->m_golfGame.gameType == 6)
			partner = (eReservedChatPartner)3;
		else
			partner = (eReservedChatPartner)2;
		Doc()->InitReservedChatList(m_pChatTarget, partner);
		float cx, cy;
		GetChatWindowOrigin983(cx, cy);
		WPoint pos;
		pos.x = (float)(cx + 21.0);
		pos.y = (float)(cy + 5.0);
		m_pChatTarget->MoveWindow(pos);
	}
}

void CGolfRule::OnChatTargetBtnDown(int)
{
	if (m_pChatEdit)
		m_pChatEdit->SetKeyFocus(true);
}

bool CGolfRule::OnChatTargetEnterKey(int)
{
	// HACK: dead reference (0 bytes) for the emission wave; the original's fully inlined callee is unknown
	if (0)
		m_pChatEdit->HasKeyFocus();
	if (m_pChatTarget)
	{
		if (strlen(m_pChatTarget->GetLine(1, false)) == 0)
			m_pChatTarget->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, // 983 0066e240: texto via K2L_Compatibility (00b4ec60)
				false, 0);
	}
	if (m_pChatEdit)
		m_pChatEdit->SetKeyFocus(true);
	return false;
}

void CGolfRule::OnChatInputInit(int param)
{
	m_pChatEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pChatEdit)
	{
		m_pChatEdit->SetVisible(false);
		float cx, cy;
		GetChatWindowOrigin983(cx, cy);
		WPoint pos;
		pos.x = (float)(cx + 139.0);
		pos.y = (float)(cy + 5.0);
		m_pChatEdit->MoveWindow(pos);
		m_pChatEdit->SetCharLimit(0x2e, true);
		WRect rc = m_pChatEdit->GetRect();
		rc.w -= 45.0f;
		m_pChatEdit->SetRect(rc);
	}
}

bool CGolfRule::OnChatInputEnterKey(int param)
{
	const char* msg = (const char*)param;
	if (strlen(msg) == 0)
	{
		((CChatMsgEx2*)CChatMsg::Instance())->Close(true, true);
		return true;
	}
	if (GOLFDOC()->m_bShowChat == 0)
	{
		std::string notice(
			K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xbc\xfb\xb1\xe4 \xbb\xf3\xc5\xc2\xc0\xd4\xb4\xcf\xb4\xd9."));
		CChatMsg::Instance()->AddChatMsg(notice, 0xffff7878, true, false);
	}
	else
	{
		if (!Doc()->UTIL_SendChatMessage(m_pChatTarget, msg, false))
		{
			m_pChatTarget->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0,
				false, 0);
			std::string err(
				K2L_Compatibility("\xb1\xd3\xb8\xbb \xbb\xf3\xb4\xeb\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9. \"\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4\"\xb7\xce \xba\xaf\xb0\xe6\xc7\xd5\xb4\xcf\xb4\xd9."));
			CChatMsg::Instance()->AddChatMsg(err, 0xffff7878, true, false);
		}
		if (OnlinePlay())
			CHeadIcon::Instance()->SendIcon(0xffff);
		else
		{
			std::string target(m_pChatTarget->GetLine(1, false));
			if (target == K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"))
			{
				std::string text(
					USERINFO983(GOLFDOC()->m_currentPlayer, 0x38c8));
				text += " : \b";
				text += msg;
				if (Doc()->m_gameMode == 5)
				{
					char buf[0x400] = { 0 };
					strncpy(buf, msg,
						strlen(msg) < 0x400 ? strlen(msg) : 0x3ff);
					if (Doc()->m_chatManager.Filtering(buf))
					{
						std::string warn(
							K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xbf\xe5\xbc\xb3\xc0\xba \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
						CChatMsg::Instance()->AddChatMsg(warn, 0xffff8000,
							false, false);
						goto done;
					}
				}
				CChatMsg::Instance()->AddChatMsg(text, 0xffffcc00, false,
					false);
done:;
			}
		}
	}
	return true;
}

void CGolfRule::OnSpecialCardInit(int param)
{
	m_pOverBarSpecialCard = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOverBarSpecialCard)
	{
		m_pOverBarSpecialCard->SetVisible(false);
		m_pOverBarSpecialCard->EnableHover(true, 0.0f);
		m_pOverBarSpecialCard->EnableBlink(
			CCardManager::Instance()->ShowSPCardAlarm());
		WPoint pos;
		pos.x = g_view->GetWidth() - 106.0f;
		pos.y = m_pOverBarSpecialCard->GetRect().y;
		m_pOverBarSpecialCard->MoveWindow(pos);
	}
}

void CGolfRule::OnSpecialCardBtnUp()
{
	OnOverBar_SpecialCardUp();
}

void CGolfRule::OnMyInfoInit(int param)
{
	m_pMyInfoBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMyInfoBtn)
	{
		m_pMyInfoBtn->SetVisible(false);
		m_pMyInfoBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 54.0f;
		pos.y = m_pMyInfoBtn->GetRect().y;
		m_pMyInfoBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnMyInfoBtnUp()
{
	if (CUserInfo::Instance())
		CUserInfo::Instance()->SetInfo(MyUID(), MyGuid(false), true, true,
			false, false, MyNick());
	OnBarHoverOff(0);
}

void CGolfRule::OnOptionInit(int param)
{
	m_pOptionBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOptionBtn)
	{
		m_pOptionBtn->SetVisible(false);
		m_pOptionBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 29.0f;
		pos.y = m_pMyInfoBtn->GetRect().y;
		m_pOptionBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnOptionBtnUp()
{
	if (CQuitWindow::Instance())
		CQuitWindow::Instance()->CreateOptionWindow();
	OnBarHoverOff(0);
}

void CGolfRule::OnCaddieBookInit(int param)
{
	m_pCaddieBookBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pCaddieBookBtn)
	{
		m_pCaddieBookBtn->SetVisible(false);
		m_pCaddieBookBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 423.0f;
		pos.y = m_pCaddieBookBtn->GetRect().y;
		m_pCaddieBookBtn->MoveWindow(pos);
		ResetUiPos_Board();
	}
}

void CGolfRule::OnCaddieBookBtnUp()
{
	SendMsg(this, "GuideBook", M983(0x12a), 0, 0, 0, 0);
}

void CGolfRule::OnUnplayableInit(int param)
{
	m_pUnplayableBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pUnplayableBtn)
	{
		m_pUnplayableBtn->SetVisible(false);
		m_pUnplayableBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 398.0f;
		pos.y = m_pUnplayableBtn->GetRect().y;
		m_pUnplayableBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnUnplayableBtnUp()
{
	if (GOLFDOC()->m_bNoStat)
	{
		if (CHelpMsg::Instance())
			CHelpMsg::Instance()->SetHelpMsg(0x17, NULL);
	}
	else if (GOLFDOC()->m_gameMode & 0x48108)
	{
		SendMsg(this, "Screen", M983(0x1cb), 0, 0, 0, 0);
	}
}

void CGolfRule::OnPauseInit(int param)
{
	m_pPauseBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pPauseBtn)
	{
		m_pPauseBtn->SetVisible(false);
		m_pPauseBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 373.0f;
		pos.y = m_pPauseBtn->GetRect().y;
		m_pPauseBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnPauseBtnUp()
{
	if (Doc()->m_golfGame.gameType != 9)
	{
		if (GOLFDOC()->m_gameMode & 0x48108)
			SendMsg(this, "Screen", M983(0x1cc), 0, 0, 0, 0);
	}
	else if (CHelpMsg::Instance())
	{
		CHelpMsg::Instance()->SetHelpMsg(0x18, NULL);
	}
}

void CGolfRule::OnChatRecordInit(int param)
{
	m_pChatRecordBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pChatRecordBtn)
	{
		m_pChatRecordBtn->SetVisible(false);
		m_pChatRecordBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 348.0f;
		pos.y = m_pChatRecordBtn->GetRect().y;
		m_pChatRecordBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnChatRecordBtnUp()
{
	SendMsg(this, "Screen", M983(0x1cd), 0, 0, 0, 0);
}

void CGolfRule::OnOnelineViewInit(int param)
{
	m_pOnelineView = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_pOnelineView->SetVisible(false);
	COneLineBoard::Instance()->SetRect(m_pOnelineView->GetRect());
	COneLineBoard::Instance()->LoadBg();
}

void CGolfRule::OnOnelineViewOwnerDraw(int)
{
	COneLineBoard::Instance()->Display();
}

void CGolfRule::OnOnelineBtnInit(int param)
{
	m_pOnelineBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	WRect rect = m_pOnelineBtn->GetRect();
	rect.x = g_view->GetWidth() - 135.0f;
	m_pOnelineBtn->SetRect(rect);
	m_pOnelineBtn->EnableHover(true, 0.0f);
	m_pOnelineBtn->SetVisible(false);
	COneLineBoard::Instance()->ShowBg(false);
}

bool CGolfRule::OnOnelineReqChkOutDlgResult(int result, FrForm*)
{
	if (result == 1)
	{
		WSendPacket packet((enumClientPacket)0x66);
		packet.EncodeStr(COneLineBoard::Instance()->GetMessageA());
		packet.Send(TO_GAME);
	}
	return true;
}

bool CGolfRule::OnOnelineReqDlgResult(int result, FrForm* pForm)
{
	if (!CChatMsg::Instance()->IsOpened())
		g_pFresh->GetManager()->SetExclusiveKey(false);
	if (result == 1)
	{
		char* text =
			(char*)((FrOnelineReqDlg*)pForm)->m_pChatInput->GetLine(1, false);
		if (Doc()->m_chatManager.Filtering(text))
		{
			FrForm* pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			pNotify->SetMessage(
				K2L_Compatibility("\xc0\xfb\xb4\xe7\xc7\xcf\xc1\xf6\xbe\xca\xc0\xba \xb4\xdc\xbe\xee\xb0\xa1 \xc6\xf7\xc7\xd4\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pNotify->Open(NULL, 3);
			m_pOnelineReqDlg = NULL;
			return true;
		}
		if (*text == '\0')
		{
			FrForm* pNotify = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify", NULL);
			// 983 00681a45: este texto (00d17174) tambem passa pelo K2L (00b4ec60)
			pNotify->SetMessage(
				K2L_Compatibility("\xba\xb8\xb3\xbe \xb3\xbb\xbf\xeb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pNotify->Open(NULL, 3);
			m_pOnelineReqDlg = NULL;
			return true;
		}
		COneLineBoard::Instance()->GetMessageA() = text;
		WSendPacket packet((enumClientPacket)0x67);
		packet.Send(TO_GAME);
	}
	m_pOnelineReqDlg = NULL;
	return true;
}

void CGolfRule::OnOnelineBtnUp()
{
	if (Doc()->IsControlServerService(0x20000))
	{
		this << MsgObject(this, M983(0x23),
			(int)K2L_Compatibility("\xc0\xfc\xb1\xa4\xc6\xc7 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}
	m_pOnelineReqDlg = CreateForm<FrOnelineReqDlg>(g_pFresh->GetManager(), this,
		"oneline_req", NULL);
	if (m_pOnelineReqDlg)
	{
		m_pOnelineReqDlg->Open(
			(FRESH_PFN_RESULT)&CGolfRule::OnOnelineReqDlgResult, 0x11);
		if (!g_pFresh->GetManager()->IsKeyExclusive())
			g_pFresh->GetManager()->SetExclusiveKey(true);
		m_pOnelineReqDlg->SetIconRect(m_pOnelineBtn->GetRect());
	}
}

void CGolfRule::OnTopBarHoverOn(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_hoverBtn = (int)button;
	if (button)
	{
		if (button == m_pMessengerBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_MSN);
		else if (button == m_pMyInfoBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_MYINFO);
		else if (button == m_pOptionBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_OPTION);
		else if (button == m_pOverBarSpecialCard)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_SPECIAL_CARD);
		else if (button == m_pCaddieBookBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_CADDIEBOOK);
		else if (button == m_pUnplayableBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_UNPLAYABLE);
		else if (button == m_pPauseBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_PAUSE);
		else if (button == m_pChatRecordBtn)
			CIconToolTip::GetInstance()->SetToolTip(TOOLTIP_CHATREC);
	}
}

void CGolfRule::OnTopBarHoverOff(int param)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button && (int)button != m_hoverBtn)
		return;
	m_hoverBtn = 0;
	CIconToolTip::GetInstance()->ClearToolTip();
}

void CGolfRule::InitRadioPredictShotButton(FrButton** ppButton, int param,
	float y)
{
	FrButton* button = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (button)
	{
		button->MoveWindow(
			WPoint(g_view->GetWidth() - 180.0f, g_view->GetHeight() - y));
		button->SetPushDelay(0.1f);
		button->SetVisible(false); // 983 00670530: nasce escondido
	}
	*ppButton = button;
}

void CGolfRule::SetRadioPredictShotPress(FrButton* button)
{
	if (m_pRadioPredictNormalShot)
		m_pRadioPredictNormalShot->SetStatus(FrButton::NORMAL);
	if (m_pRadioPredictTomahwakShot)
		m_pRadioPredictTomahwakShot->SetStatus(FrButton::NORMAL);
	if (m_pRadioPredictCobraShot)
		m_pRadioPredictCobraShot->SetStatus(FrButton::NORMAL);
	if (m_pRadioPredictSpikeShot)
		m_pRadioPredictSpikeShot->SetStatus(FrButton::NORMAL);
	if (button)
		button->SetStatus(FrButton::PRESSED);

	int type = 0;
	if (button == m_pRadioPredictTomahwakShot)
		type = 1;
	if (button == m_pRadioPredictCobraShot)
		type = 2;
	if (button == m_pRadioPredictSpikeShot)
		type = 3;
	SendMsg(this, "ShotPreview", 0x2fd, (int)"CHANGE_PREDICT_TYPE", type, 0, 0);
}

void CGolfRule::UpdateRadioPredictShotButtonStatus()
{
#ifdef JP983_DEV_PREVIEW
	bool visible = DevPreviewOn983() && GolfClub().GetType() != 3 && IsMyTurn(false);
#else
	bool visible = GolfClub().GetType() != 3;
#endif
	if (m_pRadioPredictNormalShot)
		m_pRadioPredictNormalShot->SetVisible(visible);
	if (m_pRadioPredictTomahwakShot)
		m_pRadioPredictTomahwakShot->SetVisible(visible);
	if (m_pRadioPredictCobraShot)
		m_pRadioPredictCobraShot->SetVisible(visible);
	if (m_pRadioPredictSpikeShot)
		m_pRadioPredictSpikeShot->SetVisible(visible);
}

void CGolfRule::OnRadioPredictNormalShotInit(int param)
{
	InitRadioPredictShotButton(&m_pRadioPredictNormalShot, param, 100.0f);
	if (m_pRadioPredictNormalShot)
		m_pRadioPredictNormalShot->SetStatus(FrButton::PRESSED);
}

void CGolfRule::OnRadioPredictTomahwakShotInit(int param)
{
	InitRadioPredictShotButton(&m_pRadioPredictTomahwakShot, param, 80.0f);
}

void CGolfRule::OnRadioPredictCobraShotInit(int param)
{
	InitRadioPredictShotButton(&m_pRadioPredictCobraShot, param, 60.0f);
}

void CGolfRule::OnRadioPredictSpikeShotInit(int param)
{
	InitRadioPredictShotButton(&m_pRadioPredictSpikeShot, param, 40.0f);
}

void CGolfRule::OnRadioPredictNormalShotDown()
{
	if (m_pRadioPredictNormalShot &&
		m_pRadioPredictNormalShot->GetStatus() != FrButton::PRESSED)
		SetRadioPredictShotPress(m_pRadioPredictNormalShot);
}

void CGolfRule::OnRadioPredictTomahwakShotDown()
{
	if (m_pRadioPredictTomahwakShot)
	{
		if (GolfClub().PowerShotType() != 0)
			SetRadioPredictShotPress(m_pRadioPredictTomahwakShot);
		else
			m_pRadioPredictTomahwakShot->SetStatus(FrButton::NORMAL);
	}
}

void CGolfRule::OnRadioPredictCobraShotDown()
{
	if (m_pRadioPredictCobraShot)
	{
		if (GolfClub().PowerShotType() != 0)
			SetRadioPredictShotPress(m_pRadioPredictCobraShot);
		else
			m_pRadioPredictCobraShot->SetStatus(FrButton::NORMAL);
	}
}

void CGolfRule::OnRadioPredictSpikeShotDown()
{
	if (m_pRadioPredictSpikeShot)
	{
		if (GolfClub().PowerShotType() != 0)
			SetRadioPredictShotPress(m_pRadioPredictSpikeShot);
		else
			m_pRadioPredictSpikeShot->SetStatus(FrButton::NORMAL);
	}
}

void CGolfRule::OnRadioPredictNormalShotDraw(int)
{
	FrButton* button = m_pRadioPredictNormalShot;
	if (button)
	{
		button->MoveWindow(
			WPoint(g_view->GetWidth() - 180.0f, g_view->GetHeight() - 100.0f));
		WRect rect = button->GetRect();
		float x = rect.x + rect.w - 35.0f;
		float y = rect.y;
		if (button->GetStatus() == FrButton::PRESSED)
			m_pPredictShotUI->SetColor(0xffffffff);
		else
			m_pPredictShotUI->SetColor(0x80808080);
		m_pPredictShotUI->Render("Pangya", x, y, 1.0f, 0);
	}
}

void CGolfRule::OnRadioPredictTomahwakShotDraw(int)
{
	FrButton* button = m_pRadioPredictTomahwakShot;
	if (button)
	{
		if (GolfClub().PowerShotType() == 0)
			SetRadioPredictShotPress(m_pRadioPredictNormalShot);
		button->MoveWindow(
			WPoint(g_view->GetWidth() - 180.0f, g_view->GetHeight() - 80.0f));
		button->Enable(GolfClub().PowerShotType() != 0);
		WRect rect = button->GetRect();
		float x = rect.x + rect.w - 23.0f;
		float y = rect.y;
		if (button->GetStatus() == FrButton::PRESSED)
			m_pPredictShotUI->SetColor(0xffffffff);
		else
			m_pPredictShotUI->SetColor(0x80808080);
		m_pPredictShotUI->Render("Tomahawk", x, y, 1.0f, 0);
	}
}

void CGolfRule::OnRadioPredictCobraShotDraw(int)
{
	FrButton* button = m_pRadioPredictCobraShot;
	if (button)
	{
		if (GolfClub().PowerShotType() == 0)
			SetRadioPredictShotPress(m_pRadioPredictNormalShot);
		button->MoveWindow(
			WPoint(g_view->GetWidth() - 180.0f, g_view->GetHeight() - 60.0f));
		button->Enable(GolfClub().PowerShotType() != 0);
		WRect rect = button->GetRect();
		float x = rect.x + rect.w - 40.0f;
		float y = rect.y;
		if (button->GetStatus() == FrButton::PRESSED)
			m_pPredictShotUI->SetColor(0xffffffff);
		else
			m_pPredictShotUI->SetColor(0x80808080);
		m_pPredictShotUI->Render("Cobra", x, y, 1.0f, 0);
	}
}

void CGolfRule::OnRadioPredictSpikeShotDraw(int)
{
	FrButton* button = m_pRadioPredictSpikeShot;
	if (button)
	{
		if (GolfClub().PowerShotType() == 0)
			SetRadioPredictShotPress(m_pRadioPredictNormalShot);
		button->MoveWindow(
			WPoint(g_view->GetWidth() - 180.0f, g_view->GetHeight() - 40.0f));
		button->Enable(GolfClub().PowerShotType() != 0);
		WRect rect = button->GetRect();
		float x = rect.x + rect.w - 40.0f;
		float y = rect.y;
		if (button->GetStatus() == FrButton::PRESSED)
			m_pPredictShotUI->SetColor(0xffffffff);
		else
			m_pPredictShotUI->SetColor(0x80808080);
		m_pPredictShotUI->Render("Spike", x, y, 1.0f, 0);
	}
}

CGolfRule::CGolfRule()
{
	// 983 (0068aea0): mesma lista do 645 em outra ordem, mais os campos novos
	m_forgetDlgTime = 0.0f;
	m_dispPriority = DISP_PRIORITY_0;
	m_pChatView = NULL;
	m_pEmoticonBtn = NULL;
	m_pLanguage = NULL;
	m_pReportBtn = NULL;
	m_pChatTarget = NULL;
	m_pChatEdit = NULL;
	m_pEmoticonDlg = NULL;
	m_pReportDlg = NULL;
	m_pMyInfoBtn = NULL;
	m_pOptionBtn = NULL;
	m_pCaddieBookBtn = NULL;
	m_pUnplayableBtn = NULL;
	m_pPauseBtn = NULL;
	m_pChatRecordBtn = NULL;
	m_pOnelineView = NULL;
	m_pOnelineBtn = NULL;
	m_pForgetDlg = NULL;
	m_bCancelShotAsk = false;
	m_cancelItemSlot = 0x18000031; // 983 (era 0x1800002d)
	m_unused983_474 = 0;
	m_pOnelineReqDlg = NULL;
	m_pApproachResultDlg = NULL;
	m_pApproachEndDlg = NULL;
	m_bClosedBonusBallPending = false;
	memset(m_closedBonusBall, 0, sizeof(m_closedBonusBall));
	m_pRuleBase = NULL;
	m_pSubRule983 = NULL;

	if (IsLocalContent(S4_SPECIALSHOT_RESTRICTION_MODIFY))
	{
		m_inputCmdTail = 0;
		m_inputCmdHead = 0;
		m_specialShotGroup = 0;
	}

	if (IsLocalContent(S4_REPLAY_SYSTEM))
	{
		m_pReplayControlDlg = NULL;
		m_pRecordAviDlg = NULL;
		CReplayControlManager::Instance()->Init();
		m_replayBasePang = 0;
		m_replayBaseBonusPang = 0;
	}

	Doc()->m_goStopMode = false;
	CItemSoundTable::Instance(); // 983 0068b0fb: carrega a tabela de sons na carga da partida, nao na 1a tacada

	m_pPredictShotUI = new CSimpleUI("[impact.png");
	m_pPredictShotUI->Add("Pangya", WRect(0.0f, 18.0f, 128.0f, 18.0f), false);
	m_pPredictShotUI->Add("Tomahawk", WRect(0.0f, 72.0f, 128.0f, 18.0f), false);
	m_pPredictShotUI->Add("Cobra", WRect(0.0f, 126.0f, 128.0f, 18.0f), false);
	m_pPredictShotUI->Add("Spike", WRect(0.0f, 144.0f, 128.0f, 18.0f), false);

	m_pRadioPredictNormalShot = NULL;
	m_pRadioPredictCobraShot = NULL;
	m_pRadioPredictSpikeShot = NULL;
	m_pRadioPredictTomahwakShot = NULL;
	m_topViewMode = 0;
	m_pMoveTeeBtn = NULL;
	m_turnStartTime = 0;
}

CGolfRule::~CGolfRule()
{
	// HACK: OnDisplay is naked + inline (its literals are external COMDATs), so only the vtable would pull it in;
	// this dead non-virtual reference makes it compile at its definition point, as in the original.
	if (0)
		CGolfRule::OnDisplay();
	if (m_pPredictShotUI)
	{
		delete m_pPredictShotUI;
		m_pPredictShotUI = NULL;
	}
	if (m_pSubRule983) // 983 (0068b313)
	{
		delete m_pSubRule983;
		m_pSubRule983 = NULL;
	}
	if (m_pRuleBase)
	{
		delete m_pRuleBase;
		m_pRuleBase = NULL;
	}
}

void CGolfRule::SendHoleData()
{
	// 983 (006735c0): so envia online (00633000 = OnlinePlay)
	if (!OnlinePlay())
		return;
#pragma pack(push, 1)
	struct sHoleInfo
	{
		unsigned long reserved;
		unsigned long checkSum;
		unsigned char par;
		float teeX;
		float teeZ;
		float pinX;
		float pinZ;
	};
#pragma pack(pop)

	float teeX = GOLFDOC()->GetHoleData().tee.x;
	float teeZ = GOLFDOC()->GetHoleData().tee.z;
	float pinX = GOLFDOC()->GetHoleData().pin.x;
	float pinZ = GOLFDOC()->GetHoleData().pin.z;
	unsigned char par = GOLFDOC()->GetHoleData().par;
	sHoleInfo info;
	info.teeX = teeX;
	info.teeZ = teeZ;
	info.pinX = pinX;
	info.pinZ = pinZ;
	info.par = par;
	info.checkSum = GOLFDOC()->GetHoleData().checkSum;
	info.reserved = 0;

	WSendPacket packet((enumClientPacket)0x1a);
	packet.Encode1(GOLFDOC()->m_currentHole);
	packet.EncodeBuffer(&info, sizeof(info));
	packet.Send(TO_GAME);
}

// JP 983 (0068bdb0). Corpo anterior (645 com retoques) em out/F3.5/s101/OnPreLoadInit_645.txt. Mudancas do 983: sem o laco
// do CSharedDoc +0x1049a, sem os placares de equipe e sem o ramo offline (COption); a regra e a sub-regra nascem aqui
// (SetCurrentGolfRule, que no 645 era do OnReload) e a taxa de pang foi para a regra (CGolfRuleBase::V7_983, slot [7]);
// sem o laco do GetHolePang; matriz da camera do CGolfDoc em +0x124 (o 645 gravava +0x114 cru); campos novos zerados;
// no fim, mensagem 0x7532 ao "Gateway" com a quantidade do item 0x1a0000c7.
void CGolfRule::OnPreLoadInit()
{
	// 983: abre antes o registro de depuracao "gameLog" (00b48c30("gameLog", true, 0xb); so com a chave de [0xf03fdc]): nao reproduzido
	CTaskMain::OnPreLoadInit();
	m_shotData.pvsFrame = 0;
	((CGolfDocEx*)GOLFDOC())->MakeHoleData();
	CHeadIcon::Instance()->SetPlayerNum983(-1);
	g_view->SetFOV(0.9599311f);
	float farClip;
	if (GetCurMap() == 13)
		farClip = 26214.4f;
	else
		farClip = 1920.0f;
	g_view->SetClip(0.1f, farClip, true);
	g_camera.zz = 1.0f;
	g_camera.yy = 1.0f;
	g_camera.xx = 1.0f;
	g_camera.zm = 0.0f;
	g_camera.ym = 0.0f;
	g_camera.xm = 0.0f;
	g_camera.yz = 0.0f;
	g_camera.xz = 0.0f;
	g_camera.zy = 0.0f;
	g_camera.xy = 0.0f;
	g_camera.zx = 0.0f;
	g_camera.yx = 0.0f;
	WMatrix& mat = GOLFDOC()->m_cameraData.mat; // 983: +0x124
	mat.zz = 1.0f;
	mat.yy = 1.0f;
	mat.xx = 1.0f;
	mat.zm = 0.0f;
	mat.ym = 0.0f;
	mat.xm = 0.0f;
	mat.yz = 0.0f;
	mat.xz = 0.0f;
	mat.zy = 0.0f;
	mat.xy = 0.0f;
	mat.zx = 0.0f;
	mat.yx = 0.0f;
	m_shotExtra983 = 0.0f;
	m_replayFlags = 0;
	m_dbgCam[0] = -1.0f;
	m_unused983_41d = 0;
	m_bContinueDlg = 0;
	m_bNetMsgWait = 0;
	m_dbgCam[1] = 100.0f;
	m_pNetMsgWait = 0;
	m_pNetMsgRecv = 0;
	m_netMsgSendTick = 0;
	m_netMsgRecvTick = 0;
	m_unused983_48c = 0.0f;
	m_netMsgWaitType = -1;
	m_serverHoldTime983 = 0.0f;
	m_bGameUI = true;
	m_holeInCoinFxTime = 0.0f;
	m_bgmIndex = 0;
	m_cmdQueueTime = 0.0f;
	m_pendingInitSpecial = 0;
	m_observerReqTick = 0;
	m_premiumNoticeAlpha = 1.0f;
	m_captureIndex = 0;
	m_captureTiles = 1;
	m_premiumNoticeFadeRate = 0.2f;
	m_captureMode = 0;
	m_bGaugePaused = 0;
	m_bTutorialShot983 = 0;
	m_bRemoteHitPending = false;
	m_nextTurn = 0;
	m_pPauseDlg = 0;
	m_pGoStopDlg = 0;
	m_pCancelDropDlg = 0;
	m_bServerHold983 = 0;
	m_bChatIconSent983 = 0;
	m_bNotifyPending983 = 0;
	m_bSkinsNextHole = false;
	m_holeInCoinFxCount = 0;
	m_holeInCoinFxId = 0;
	*(unsigned short*)&m_screenLockMode = 0;
	m_autoShotCount = 0;
	m_pPremiumNotice = 0;
	m_b983_59c = 0;
	m_pPremiumBmp[0] = 0;
	m_pPremiumBmp[1] = 0;
	m_pPremiumBmp[2] = 0;
	m_pPremiumBmp[3] = 0;
	char path[512];
	strcpy(path, g_executeDirectory);
	sprintf(path, "%s/data/Sound/bgm", path);
	CheckUserBGM(path);
	for (int i = 0; i < 8; i++)
		g_audio->LoadSfx(g_plSound[i], 0, 0, 0, 0, 0.5f, 200.0f);
	GOLFDOC()->m_bTutorialGaugeMute = 0;
	GOLFDOC()->m_tutorialMode = 0xf;
	if (IsLocalContent((localContentType_t)0x59 /*983: era 0x58; medido em 0068c0c0*/))
		CTHunter::Instance()->SetTreasurePoint(0);
	CMouseCursor::Instance()->SetAutoHide(true);
	new CCameraWork;
	new CRival;
	new CRandomHouse;
	if (Doc()->m_startHole == 0)
		GOLFDOC()->m_currentHole = Doc()->m_holeOrder[0];
	else
		GOLFDOC()->m_currentHole = Doc()->m_startHole;
	SetCurrentGolfRule();
	if (!m_pRuleBase)
		ReturnLobby(false);
	m_pRuleBase->V7_983();
	PostMsg(this, "GolfRule", 0x274 /*so do 983*/, 0, 0, 0, 0);
	int count = 0;
	std::list<sItemInfo>& items = Doc()->m_myItemList;
	std::list<sItemInfo>::iterator it;
	for (it = items.begin(); it != items.end(); ++it)
	{
		if ((*it).typeId == 0x1a0000c7)
			break;
	}
	if (it != items.end())
		count = (*it).Common[0];
	IActor* gateway = AfxGetTask()->GetActor(s_gatewayActor[1]);
	if (gateway)
		gateway << MsgObject(NULL, 0x7532 /*so do 983*/, 0xd, count, 0, 0, 0);
}

// 983 00686840 (chamada no fim de CGolfRule::OnLoad 00686a70): mapa global 00e78e44, chave = tid do par do artefato
// (CItemManager 00a192e0); valor {tid do par, item do artefato se o jogador o tem na lista +0x375c (senao 0), item do par}.
// AddExtPrize (684210): pPair != NULL -> item do premio = pPair e AF_Cluster.seq; chat "[%s] mana recuperada" com o
// nome de pOwned, e sem chat quando o jogador nao tem o artefato (pOwned == NULL). Chave ausente = valor zerado (00e78e3c).
struct sArtifactPrize983
{
	IFF_ITEM_COMMON* pOwned; // +0x00 (local_24)
	IFF_ITEM_COMMON* pPair; // +0x04 (local_20)
};
static std::map<unsigned long, sArtifactPrize983> s_artifactPrize983;

static void FillArtifactPrizeMap983()
{
	s_artifactPrize983.clear();
	CItemCursor983 cursor(DOCPTR983(0x375c));
	CItemManager& mgr = Doc()->m_itemManager;
	std::vector<unsigned long> tids;
	{
		std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator it;
		for (it = mgr.m_ArtifactManaInfoMap983.begin(); it != mgr.m_ArtifactManaInfoMap983.end(); ++it)
			tids.push_back(*(unsigned long*)((char*)&(*it).second + 0x04)); // 00a23780
	}
	for (size_t i = 0; i < tids.size(); ++i)
	{
		std::map<unsigned int, IFF_STRUCT::sItem>::iterator itItem = mgr.m_ItemMap.find(tids[i]);
		if (itItem == mgr.m_ItemMap.end())
			continue;
		std::map<unsigned int, IFF983::sArtifactManaInfo>::iterator itArt = mgr.m_ArtifactManaInfoMap983.find(tids[i]);
		unsigned long pairTid = itArt == mgr.m_ArtifactManaInfoMap983.end() ? 0 : *(unsigned long*)((char*)&(*itArt).second + 0x08); // 00a192e0
		std::map<unsigned int, IFF_STRUCT::sItem>::iterator itPair = mgr.m_ItemMap.find(pairTid);
		if (itPair == mgr.m_ItemMap.end())
			continue;
		sArtifactPrize983 v;
		v.pOwned = cursor.Find(&tids[i]) ? &(*itItem).second.c : NULL;
		v.pPair = &(*itPair).second.c;
		s_artifactPrize983.insert(std::make_pair(pairTid, v));
	}
}

void CGolfRule::OnLoad()
{
	CTaskMain::OnLoad();
	// 983 00686a70 -> 00636c60(shared+0x1051e, 1): copia os 19 bytes de buraco para m_holeOrderCrypt983 (cifrados). Se o buraco
	// repetido (m_fixHole983, 1..0x12) e valido e o tipo de buraco (+0x1094f) != 4, zera-o (0xff) antes; se ainda valido,
	// os 19 recebem o proprio valor, senao vem do vetor de origem.
	{
		CSharedDoc* sd = Doc();
		const unsigned char* src = sd->m_holeOrder;
		unsigned char rh = (unsigned char)sd->m_fixHole983;
		if (rh >= 1 && rh <= 0x12 && *((unsigned char*)sd + DOCOFS983(0x1094f)) != 4)
			sd->m_fixHole983 = (unsigned char)0xff;
		rh = (unsigned char)sd->m_fixHole983;
		bool valid = rh >= 1 && rh <= 0x12;
		for (int i = 0; i < 19; ++i)
			sd->m_holeOrderCrypt983[i] = valid ? rh : src[i];
	}
	OnReload();
	CTaskMain::OnInit();
	g_pFresh->OpenLayout("MAIN", this, true);
	// 983 00686a70 fim: 0065fe80 zera as 3 primeiras correcoes de PangYa do CGolfDoc (+0x860..0x868)
	FillArtifactPrizeMap983(); // 983 00686840
	{
		CGolfDoc* gd = GOLFDOC();
		gd->m_pPangyaCorrection983[0] = NULL;
		gd->m_pPangyaCorrection983[1] = NULL;
		gd->m_pPangyaCorrection983[2] = NULL;
		gd->m_pPangyaCorrection983[3] = NULL;
		memset(gd->m_pPangyaAddCorrection983, 0, sizeof(gd->m_pPangyaAddCorrection983)); // 0065fe80 zera +0x860..+0x87c (C112.1)
		// 983 00686a70: depois de zerar, refaz pelo CControllerDoc (conteudo 0xad) +0x860/+0x870 (0x0a/0x16: 006621b0),
		// +0x8b0 (0x1a: 006622c0), +0x8a0 (0x19: 00662280), +0x890 (0x17: 00662240); 0065ff40/ff20/ff00 zeram antes
		memset(gd->m_pPowerShotItem983, 0, 0x30); // +0x890..+0x8bc (3 x 4)
		IContentsDataContainer* container = CContentsDoc::Instance()->GetContainer((localContentType_t)0xad);
		if (container)
		{
			_gateway::_controller::CControllerDoc* cd = (_gateway::_controller::CControllerDoc*)container;
			CControlledObject** items = gd->m_pPowerShotItem983; // +0x890/+0x8a0/+0x8b0 seguidos (um ponteiro so, como o 983)
			for (int i = 0; i < 4; i++)
			{
				gd->m_pPangyaCorrection983[i] = (CPangyaCorrectionObject*)cd->GetObjectAt(i, 0x0a);
				gd->m_pPangyaAddCorrection983[i] = (CPangyaAddCorrectionObj*)cd->GetObjectAt(i, 0x16);
				items[i] = cd->GetObjectAt(i, eSpecialAbility_PowerShotItem);
				items[4 + i] = cd->GetObjectAt(i, eSpecialAbility_PlusYardItem); // m_pPlusYardItem983[i]
				items[8 + i] = cd->GetObjectAt(i, eSpecialAbility_ComboGaugeEveryshot); // m_pComboGaugeEveryshot983[i]
			}
		}
	}
}

struct sType9Area983
{
	float c[3];
	float half;
	float pow;
	sType9Area983() : half(1.0f), pow(100.0f) { c[0] = c[1] = c[2] = 0.0f; }
};
// 004852e0: a lista estatica (e73ec4) e global do arquivo; consumida no laco de simulacao da tacada (0068dd50, ChangeGameMode)
static std::vector<sType9Area983*> s_areas983;
// 004846a0 (+004845c0 Contains): bola dentro da esfera -> velocidade += normalizado(centro - pos) * pow * dt
static void ProcessType9Areas983(char* ball, float dt)
{
	for (size_t k = 0; k < s_areas983.size(); k++)
	{
		const sType9Area983* a = s_areas983[k];
		float* pos = (float*)ball;
		float d[3] = { a->c[0] - pos[0], a->c[1] - pos[1], a->c[2] - pos[2] };
		float len = sqrtf(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
		if (len > a->half)
			continue;
		float inv = 0.0f;
		if (len > 1e-6f)
			inv = 1.0f / len;
		float* vel = (float*)(ball + 0xc);
		for (int i = 0; i < 3; i++)
			vel[i] += d[i] * inv * a->pow * dt;
	}
}

// B1394: a previsao GM (shotpreview, so JP983_DEV_PREVIEW) aplica a mesma forca das areas tipo 9 que o laco da tacada real
// (0068dd50); a previsao do 983 (008451d0) nao chama, por isso a bola real saia da linha prevista nesses buracos
void ShotPreviewType9Areas983(float dt)
{
	ProcessType9Areas983((char*)&GolfBall(), dt);
}

// JP 983 (00685860). Diferencas para o 645: o fator de inclinacao vem de pCourse+0x13c (PVS+0x21d4); mensagens "Score" 0x160/0x164
// (acumulado e dos tiros); zera o par +0x4f0/+0x4f4 (replay/AVI) e o flag +0x640/0x641; chama o slot 8 da regra-base;
// limpa a fila deque +0x224 (00681f60/00681fc0, msg "Screen" 0x330) e recalcula o flag +0x241 (modo 11, bit 14 da identidade,
// ou a ficha 0x1a00019f da lista +0x375c do doc compartilhado); "Screen" 0x32f. Lista estatica 00e73ec4 (004852e0/00485110) no fim.
void CGolfRule::OnInit()
{
	if (Doc()->m_golfGame.pCourse)
		// B1501: era (char*)&GetPVS() + 0x21d4 cru; no cl 14.44 m_water/m_aniObjs (STL) encolhem e o cru caia em m_vec21d8_983.y
		*(unsigned long*)&GetPVS().m_slopeFactor = *(unsigned long*)((char*)Doc()->m_golfGame.pCourse + 0x13c);
	SCENEMGR()->UpdateWind();
	m_bTrajectoryMsgSent = false;
	CalcUIOffset(false);
	SendMsg(this, "Score", 0x160, 0, 0, 0, 0);
	SendMsg(this, "Score", 0x164, 0, 0, 0, 0);
	m_pReplayControlDlg = NULL;
	m_pRecordAviDlg = NULL;
	*(unsigned short*)&m_bHoleIntroEntered = 0;
	(*(void(__thiscall***)(void*))m_pRuleBase)[8](m_pRuleBase);
	// 00681fc0
	*(unsigned long*)&m_cmdQueueTime = 0;
	while (!m_cmdQueue.empty())
		m_cmdQueue.pop_front();
	m_screenLockMode = 0;
	SendMsg(this, "Screen", 0x330, 0, 0, 0, 0);
	if (Doc()->m_golfGame.gameType == 0xb || ((SHAREDDOC_ULONG(0x812) >> 0xe) & 1))
		m_bAutoShot = 1;
	else
	{
		int* head = (int*)SHAREDDOC_ULONG(0x375c);
		if (head)
		{
			int* n = (int*)*head;
			while (n != head && n[3] != 0x1a00019f)
				n = (int*)*n;
			if (n != head)
				m_bAutoShot = (short)n[5] > 0;
		}
	}
	SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
	// 004852e0 + 00485110: lista estatica (e73ec4) com um registro de 0x14 B por buraco tipo 9 do GolfDoc+8 -> +0xc0 (passo 0x80, n em
	// +0x98): centro e meia largura da caixa em +0x44 (6 floats) e o "*pow" (padrao 100.0) lido do nome em +1 (WSliceStr::Get).
	{
		for (size_t k = 0; k < s_areas983.size(); k++)
			delete s_areas983[k];
		s_areas983.clear();
		// B1456: era GolfDoc+8 / +0x98 / +0xc0 cru (VC10); no cl 14.44 o WPolySoup anda (STL antes do m_header) -> membros
		C_ASSERT_VC10(offsetof(CGolfDoc, m_pPolySoup) == 8);
		C_ASSERT_VC10(offsetof(WPolySoup, m_header.areaNum) == 0x98);
		C_ASSERT_VC10(offsetof(WPolySoup, m_area) == 0xc0);
		C_ASSERT(sizeof(WPolySoup::sArea) == 0x80);
		C_ASSERT(offsetof(WPolySoup::sArea, aabb) == 0x44);
		WPolySoup* soup = GOLFDOC()->m_pPolySoup;
		int n = soup->m_header.areaNum;
		for (int k = 0; k < n; k++)
		{
			WPolySoup::sArea* hole = &soup->m_area[k];
			if (hole->type != 9)
				continue;
			sType9Area983* a = new sType9Area983;
			const float* box = (const float*)&hole->aabb;
			a->c[0] = (box[0] + box[3]) * 0.5f;
			a->c[1] = (box[4] + box[1]) * 0.5f;
			a->c[2] = (box[5] + box[2]) * 0.5f;
			a->half = (box[3] - box[0]) * 0.5f;
			WSliceStr(hole->name).Get("*pow", "%f", &a->pow);
			s_areas983.push_back(a);
		}
	}
}

// JP 983 (00678a30): a regra passou a nascer no OnPreLoadInit; a carga dos buracos (jogo) e da fita (replay) foi para a sub-regra
// (CSubGolfRuleGame/CSubGolfRuleReplayTape::Init, frente B). Corpo do 645 em out/F3.5/s101/OnReload_645.txt.
void CGolfRule::OnReload()
{
	// 983: `if (0066e850()) +0x634 = 1`, mas o 0066e850 e `return 0` (o nosso GalleryMode() le o bit 1 da identidade): sem efeito
	Doc()->ClearGameVars();
	((CGolfDocEx*)GOLFDOC())->ClearShotResultMsg();
	m_pSubRule983->Init();
	SetPlayer();
	SetGrade();
	if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
	{
		RecordPlayerList();
		for (sRivalData* it = &*Doc()->m_rivalList.begin(); it != &*Doc()->m_rivalList.end(); it++)
			it->observerState = 0;
	}
	ToggleGameUI();
	CRandomHouse::Instance()->SetRandomSeed(g_CurrentTime);
	CCameraWork::Instance()->Load();
	((CChatMsgEx*)CChatMsg::Instance())->ClearHistory();
	CMouseCursor::Instance()->Reset(true);
	MOUSECURSOR()->m_mode = 1;
	COneLineBoard::Instance()->Clear();
	CMessengerInfo::Instance()->SetGameFocus(true);
}

void CGolfRule::SetPlayer()
{
	if (HackingManager::Instance())
		HackingManager::Instance()->ClearCrypticList();
	if (m_pRuleBase)
		m_pRuleBase->SetPlayer();
}

void CGolfRule::SetGrade()
{
	if (m_pRuleBase)
		m_pRuleBase->SetGrade();
}

void CGolfRule::SetReady()
{
	if (Doc()->m_playingMode == (eReplayModeType)2)
		ChangeGameMode(0x10000);
	else if (m_pRuleBase)
		m_pRuleBase->SetReady();
}

void CGolfRule::RecordPlayerList()
{
	mkdir("history");
	mkdir("history/ongamenet");
	SYSTEMTIME time;
	GetLocalTime(&time);
	char buf[256] = "";
	FILE* fp =
		fopen(MakeStr("history/ongamenet/player(%02d\xbd\xc3%02d\xba\xd0).txt",
				  time.wHour, time.wMinute),
			"wt");
	if (fp)
	{
		sprintf(buf, "\xbe\xc6\xc0\xcc\xb5\xf0 : \xb4\xeb\xc8\xad\xb8\xed\n");
		fwrite(buf, strlen(buf), 1, fp);
		sprintf(buf,
			"------------------------------------------------------------\n");
		fwrite(buf, strlen(buf), 1, fp);
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 00b2f180 (como IsMassGame 00502f20)
		{
			for (sRivalData* rival = &*Doc()->m_rivalList.begin();
				rival != &*Doc()->m_rivalList.end(); rival++)
			{
				sprintf(buf, "%s\n", rival->nickname);
				fwrite(buf, strlen(buf), 1, fp);
			}
			break;
		}
		default:
			for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
			{
				sprintf(buf, "%s\n", Doc()->m_userInfo[i].info.sNick);
				fwrite(buf, strlen(buf), 1, fp);
			}
			break;
		}
		fclose(fp);
	}
}

void CGolfRule::SetVisibleUpperBtns(bool visible)
{
	if (m_pChatView)
		m_pChatView->SetVisible(visible);
	if (m_pOnelineBtn)
		m_pOnelineBtn->SetVisible(visible);
	if (m_pMyInfoBtn)
		m_pMyInfoBtn->SetVisible(visible);
	if (m_pOptionBtn)
		m_pOptionBtn->SetVisible(visible);
	if (m_pMessengerBtn)
		m_pMessengerBtn->SetVisible(visible);
}

// 983 (00686b40): offsets dos valores cripto registrados no HackingManager
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar) == 0xe0);
C_ASSERT(offsetof(sBar, grade) == 0x10 && offsetof(sBar, dir) == 0x18 && offsetof(sBar, center) == 0x1c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHolePot) == 0x1b0);
C_ASSERT_VC10(offsetof(CGolfDoc, m_cameraData) + offsetof(sCameraData, angle) == 0x11c);
C_ASSERT(offsetof(sPlayerData, totalPang) == 0x4f8);
C_ASSERT(offsetof(sPlayerData, bonusPang) == 0x508);
C_ASSERT(offsetof(sPlayerData, power) == 0x51c);
C_ASSERT(offsetof(sPlayerData, accuracy) == 0x52c);
C_ASSERT(offsetof(sPlayerData, control) == 0x53c);
C_ASSERT(offsetof(sPlayerData, spin) == 0x54c);
C_ASSERT(offsetof(sPlayerData, curve) == 0x55c);

void CGolfRule::AddPlayer(unsigned char index, const char* name,
	unsigned long uid, unsigned long oid, unsigned long caddie)
{
	if (m_pRuleBase)
	{
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
			m_pRuleBase->AddPlayer(index, name, uid, oid, caddie);
		else
		{
			m_pRuleBase->AddPlayerOffline(index, name, oid, caddie);
			// 983 (00686b40): offline grava 0x8000 no flag do jogador (+0x3922) logo apos o AddPlayerOffline
			*(unsigned long*)(USERINFO983(index, 0x3922)) = 0x8000;
		}
	}
	if (OnlinePlay() && HackingManager::Instance())
	{
		GolfBall().AddCrypticValue();
		GolfClub().AddCrypticValue();
		int i = 0;
		int left = 18;
		do
		{
			HackingManager::Instance()->AddCrypticByteValue(
				&PLAYER(index)->stroke[i]);
			HackingManager::Instance()->AddCrypticByteValue(
				&PLAYER(index)->putt[i]);
			HackingManager::Instance()->AddCrypticIntValue(
				&PLAYER(index)->pang[i]);
			i++;
			left--;
		} while (left != 0);
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->totalPang);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->bonusPang);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticFloatValue(&player->gauge);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->power);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->control);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->accuracy);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->spin);
		}
		{
			sPlayerData* player = PLAYER(index);
			HackingManager::Instance()->AddCrypticIntValue(&player->curve);
		}
	}
}

void CGolfRule::OnDestroy()
{
	CTaskMain::OnDestroy();

	if (CCameraWork::Instance())
		delete CCameraWork::Instance();
	if (CRival::Instance())
		delete CRival::Instance();
	if (CRandomHouse::Instance())
		delete CRandomHouse::Instance();
	if (CCapturedBg::Instance())
		delete CCapturedBg::Instance();

	while (!m_backUpStack.empty())
		m_backUpStack.pop();

	// 983 (00681c10): termina com g_audio->StopBGM(true) (vtable +0x18)
	g_audio->StopBGM(true);
}

// 983 (00673740): near/far do clip pelo jogador/camera; far = 1920 (26214.4 no mapa 13) ou o maior z (no espaco da camera) dos
// 8 cantos da caixa e0 do WPolySoup. Flag 0x200 do modo: near fixo 3.2. Sem o early-return e o 0x5d8 do 645.
void CGolfRule::OnDisplay()
{
	if ((GOLFDOC()->m_gameMode & 0x20) != 0 && CCapturedBg::Instance() != NULL)
		CCapturedBg::Instance()->Render();
	WPolySoup* soup = GOLFDOC()->m_pPolySoup;
	C_ASSERT_VC10(offsetof(WPolySoup, m_aabb_e0_983) == 0xe0); // B1456: era soup+0xe0 cru (VC10)
	const Waabb& box = soup->m_aabb_e0_983;
	float nearClip;
	if ((GOLFDOC()->m_gameMode & 0x200) != 0)
		nearClip = 3.2f;
	else
	{
		int index;
		nearClip = g_camera.ym -
			GetPVS()
				.GetGroundPoint(WVector(g_camera.xm, g_camera.ym, g_camera.zm),
					true, &index, false)
				.y;
		if (index == -1)
			nearClip = g_camera.ym - box.min.y;
		bool useBall = GOLFDOC()->m_gameMode == 0x80;
		if (!useBall)
		{
			WSphere sphere(GolfBall().m_pos, GolfBall().m_radius);
			useBall = g_view->InFrustum(sphere);
		}
		if (useBall)
		{
			float dx = g_camera.xm - GolfBall().m_pos.x;
			float dy = g_camera.ym - GolfBall().m_pos.y;
			float dz = g_camera.zm - GolfBall().m_pos.z;
			float dist = sqrtf(dx * dx + dy * dy + dz * dz);
			if (dist < nearClip)
				nearClip = dist;
		}
		if (0.1 <= nearClip) // 983 006739a2: fcom qword [00cff040] = 0.1 (double)
		{
			if (1.0f < nearClip)
				nearClip = 1.0f;
		}
		else
			nearClip = 0.1f;
	}
	// 983 00673740: no mapa 13 o far fica fixo (os 8 cantos da caixa nao entram)
	float farClip = GetCurMap() == 13 ? 26214.4f : 1920.0f;
	const WVector& mn = box.min;
	const WVector& mx = box.max;
	WMatrix inv = ~g_camera;
	for (int i = 0; GetCurMap() != 13 && i < 8; i++)
	{
		float x = (i & 1) ? mx.x : mn.x;
		float y = (i & 4) ? mx.y : mn.y;
		float z = (i & 2) ? mx.z : mn.z;
		float zc = x * inv.zx + y * inv.zy + z * inv.zz + inv.zm;
		if (farClip < zc)
			farClip = zc;
	}
	g_view->SetClip(nearClip, farClip, true);
	g_view->UpdateCamera();
}

void CGolfRule::OnProcess(float time)
{
	static bool s_bLastImeOpened = false;
	static bool s_bImeInit = false;

#if defined(JP983_SHOT_PREDICTION) || defined(JP983_DEV_PREVIEW) // previsao do 645 (GM); o 983 (0069bb50) nao chama
	UpdateRadioPredictShotButtonStatus();
#endif
	// 983 (0069bb50): sem o relatorio de replay FUNC_8 (B595)
	// 983: o bloco do replay (teclas e replay_status) saiu daqui para o HandleMsg 0x274
	// 983 0069bb8c: grava em FrWnd+0x80 (m_wndAlpha; +0x84 = alpha2, 00be882d); o +0x6c do 645 cai no m_wndName
	if (m_pOnelineBtn != NULL)
		*(float*)((char*)m_pOnelineBtn + VC10_FRWND_ALPHA) = // B1471: 0x74 no cl 14.44
			COneLineBoard::Instance()->GetFadeFactor();
	if (!(GOLFDOC()->m_gameMode & 0x30) &&
		CCapturedBg::Instance()->IsCaptured())
		CCapturedBg::Instance()->Reset();
	if (m_bServerHold983 != 0)
	{
		m_serverHoldTime983 = time + m_serverHoldTime983;
		if (m_serverHoldTime983 > 5.0f)
			m_bServerHold983 = 0;
	}
	ProcessObserver();
	if (IsSharedFlag433Bit1() &&
		(GOLFDOC()->m_shotPhase == 1 || GOLFDOC()->m_shotPhase == 2))
		DoPowerGauge(time);
	if (GOLFDOC()->m_gameMode != 0x200 && g_input->GetDown("FUNC_12", true))
	{
		if (m_pKbdManualDlg != NULL)
			m_pKbdManualDlg->Close(true);
		else
		{
			m_pKbdManualDlg = CreateForm<FrKbdManualDlg>(g_pFresh->GetManager(),
				this, "kbd_manual", NULL);
			if (m_pKbdManualDlg != NULL)
			{
				m_pKbdManualDlg->Open(
					(FRESH_PFN_RESULT)&CGolfRule::OnKbdManualDlgResult, 1);
				m_pKbdManualDlg->EnableDrag(false);
			}
		}
	}
	if (!CQuitWindow::Instance()->IsActive())
	{
		if (((g_input->Get("LCONTROL", true) ||
				 g_input->Get("RCONTROL", true)) &&
				g_input->GetDown("HIDE", true)) ||
			(!m_bGameUI &&
				(g_input->GetDown("ENTER", true) ||
					g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
						true))))
		{
			m_bGameUI = !m_bGameUI;
			ToggleGameUI();
		}
	}
	else if (!m_bGameUI)
	{
		m_bGameUI = true;
		ToggleGameUI();
	}
	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("EMOTICON", false) &&
		CRankingInfo::Instance()->GetDlg() == NULL &&
		!CMessengerInfo::Instance()->IsVisible())
		OnEmoticonBtnUp();
	if (m_pLanguage != NULL)
	{
		// 983 (0069bb50): a estatica nasce com o estado atual do IME (DAT_00e78f9f = 0099f050, GetState(7,0)), B1615
		if (!s_bImeInit)
		{
			s_bImeInit = true;
			s_bLastImeOpened = CChatMsg::Instance()->IME_IsImeOpened();
		}
	}
	if (m_pLanguage != NULL &&
		CChatMsg::Instance()->IME_IsImeOpened() != s_bLastImeOpened)
	{
		s_bLastImeOpened = CChatMsg::Instance()->IME_IsImeOpened();
		if (!s_bLastImeOpened)
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");
	}
	if (m_pChatEdit != NULL && m_pChatTarget != NULL)
	{
		if (!CChatMsg::Instance()->IsClosed())
		{
			if (g_input->GetDown("PAGE_UP", true))
			{
				if (m_pChatEdit->m_nFlags.GetFlag(0x10) &&
					Doc()->m_chatHistory.size() > 0)
				{
					std::list<std::string>::iterator it =
						std::find(Doc()->m_chatHistory.begin(),
							Doc()->m_chatHistory.end(),
							m_pChatEdit->GetLine(1, false));
					if (it != Doc()->m_chatHistory.end())
					{
						if (it != Doc()->m_chatHistory.begin())
						{
							--it;
							CChatMsg::Instance()->SetChatText(it->c_str(),
								false);
						}
					}
					else
						CChatMsg::Instance()->SetChatText(
							Doc()->m_chatHistory.back().c_str(), false);
				}
				else if (m_pChatTarget->m_nFlags.GetFlag(0x10) &&
					Doc()->m_reservedChatList.size() > 0)
				{
					CChatMsg::Instance()->SetChatText(
						Doc()->GetPrevChatTarget(
							m_pChatTarget->GetLine(1, false)),
						false);
				}
			}
			else if (g_input->GetDown("PAGE_DOWN", true))
			{
				if (m_pChatEdit->m_nFlags.GetFlag(0x10) &&
					Doc()->m_chatHistory.size() > 0)
				{
					std::list<std::string>::iterator it =
						std::find(Doc()->m_chatHistory.begin(),
							Doc()->m_chatHistory.end(),
							m_pChatEdit->GetLine(1, false));
					if (it != Doc()->m_chatHistory.end())
					{
						++it;
						if (it != Doc()->m_chatHistory.end())
							CChatMsg::Instance()->SetChatText(it->c_str(),
								false);
						else
							CChatMsg::Instance()->SetChatText("", false);
					}
				}
				else if (m_pChatTarget->m_nFlags.GetFlag(0x10) &&
					Doc()->m_reservedChatList.size() > 0)
				{
					CChatMsg::Instance()->SetChatText(
						Doc()->GetNextChatTarget(
							m_pChatTarget->GetLine(1, false)),
						false);
				}
			}
		}
		if (m_pChatEdit->IsVisible() && g_input->GetDown("ENTER", true) &&
			!CMessengerInfo::Instance()->IsVisible())
			m_pChatEdit->SetKeyFocus(true);
		if (OnlinePlay())
		{
			const char* line = m_pChatEdit->GetLine(1, false);
			if (line != NULL && strlen(line) != 0)
			{
				if (m_bChatIconSent983 == 0)
				{
					m_bChatIconSent983 = 1;
					CHeadIcon::Instance()->SendIcon(1);
				}
				if (strlen(m_pChatTarget->GetLine(1, false)) == 0 ||
					!strcmp(m_pChatTarget->GetLine(1, false), // 983 0069c0e3: K2L antes do strcmp (D737)
						K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4")))
					m_pChatEdit->SetFontColor(0xffffffff);
				else if (strlen(m_pChatTarget->GetLine(1, false)) == 0 ||
					!strcmp(m_pChatTarget->GetLine(1, false), // 983 0069c14b
						K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1\xb0\xd4")))
					m_pChatEdit->SetFontColor(0xff5bffb0);
				else
					m_pChatEdit->SetFontColor(0xffcf67ff);
			}
			else if (m_bChatIconSent983 == 1)
			{
				m_bChatIconSent983 = 0;
				CHeadIcon::Instance()->SendIcon(0xffff);
			}
		}
	}
	if (m_bNetMsgWait)
	{
		m_netMsgWaitTime = m_netMsgWaitTime - time;
		if (m_netMsgWaitTime < 0.0f)
		{
			unsigned long elapsed = GetTickCount() - m_netMsgSendTick;
			if (elapsed >
				(unsigned long)((m_pNetMsgWait != NULL &&
									!strcmpi(m_pNetMsgWait, "DoScoreBoard"))
						? 40000
						: 20000))
			{
				const char* name = m_pNetMsgWait;
				if (name == NULL)
					name = "UnknownLock";
				CProjectG::Instance()->ReportError(1, "%s_%d_%d_%d_%d(%d_%d)",
					name, g_iSendPacketGR, g_iRecvPacketGR, g_iSendFailType,
					WSAGetLastError(),
					(unsigned int)*(unsigned short*)(DOCPTR983(0x1094d)),
					Doc()->m_myInfo.info.dwGuid);
				g_iSendPacketGR = 0;
				g_iRecvPacketGR = 0;
				g_iRecvDisconCode = 0;
				SendReportError(1);
				m_bNetMsgWait = false;
				m_pNetMsgRecv = "";
				m_netMsgRecvTick = GetTickCount();
				g_iRecvPacketGR = g_iRecvPacketType;
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				// B1362: 983 0069c2fd: texto via K2L_Compatibility (00b4ec60); cru saia em EUC-KR no JP (Notice ilegivel do dono7)
				pForm->SetMessage(K2L_Compatibility(
					"\xbc\xad\xb9\xf6\xbf\xcd\xc0\xc7 \xbf\xac\xb0\xe1\xc0\xcc \xb2\xf7\xb0\xe5\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				pForm->Open(
					(FRESH_PFN_RESULT)&CGolfRule::ReturnToLobbyDlgResult,
					0x203);
			}
		}
	}
	if (m_bNotifyPending983 != 0)
	{
		m_bNotifyPending983 = 0;
		m_bNetMsgWait = false;
		m_pNetMsgRecv = "";
		m_netMsgRecvTick = GetTickCount();
		g_iRecvPacketGR = g_iRecvPacketType;
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
		pForm->SetMessage(K2L_Compatibility( // B1362: 983 0069c391 via K2L_Compatibility
			"\xc6\xc4\xc0\xcf\xc0\xcc \xbc\xd5\xbb\xf3\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
			false);
		pForm->Open((FRESH_PFN_RESULT)&CGolfRule::ReturnToLobbyDlgResult, 3);
	}
	if (GOLFDOC()->m_gameMode != 0x20 && m_captureMode == 0)
		GetPVS().PreProcess(time, false);
	if ((GOLFDOC()->m_gameMode & 0x48308) && m_bServerHold983 == 0)
		((CClubEx*)&GolfClub())->Process(time);
	// 983 (0069bb50): o tempo da Approach saiu daqui para a regra [10] (CGolfRuleNewApproach 006a9790)
	if (SHAREDDOC_UCHAR(0x10594) == 10)
		m_pRuleBase->V10_983(time);
	if ((GOLFDOC()->m_gameMode & 8) &&
		GolfClub().m_powerShot == 0 &&
		(g_input->Get("LALT", true) || g_input->Get("RALT", true)))
		return;
	g_input->GetDown("SKIP", true);
	// 983 (B574): so nos tipos 0/4/5/6/0xb; velocidade 3.0 se 0x10588 ou serverProperty bit 14
	unsigned char ffType = SHAREDDOC_UCHAR(0x10594);
	if ((ffType == 4 || ffType == 5 || ffType == 6 || ffType == 0 || ffType == 0xb) &&
		g_input->GetDown("FASTFORWARD", true) && m_bFastForwardReady != 0 &&
		(GOLFDOC()->m_gameMode & 0xc0))
	{
		if (!OnlinePlay() || !IsMyTurn(false))
			m_bFastForwardReady = 0;
		else if (SHAREDDOC_UCHAR(0x10588) != 0 || ((SHAREDDOC_ULONG(0x812) >> 0xe) & 1))
		{
			m_bFastForwardReady = 0;
			float speed = 3.0f;
			WSendPacket packet((enumClientPacket)0x65);
			packet.EncodeBuffer(&speed, 4);
			packet.Send(TO_GAME);
		}
	}
	if (Doc()->m_playingMode == 1 &&
		(OnlinePlay() || SHAREDDOC_ULONG(0x1048c) == 5))
	{
		if (!WisEqual(CProjectG::Instance()->GetGameSpeed(), 1.0f,
				g_EPSILON) &&
			(GOLFDOC()->m_gameMode & 0xfffff73f))
			CProjectG::Instance()->SetGameSpeed(1.0f);
	}
#ifdef JP983_DEV_PREVIEW
	// Ctrl+P da conta GM no Practice (realGameType 0x13, pedido do dono 28/10; nao existe no 983): enche a carga do Power Shot
	// como o comando de console "combo" do 645 (console.cpp case 0xc: gauge + valor ate GetComboGaugeLimit; Team = do time)
	if (DevPreviewCtrlR983() && g_input->GetDown("PEN", true) && Doc()->m_roomInfo.realGameType == 0x13 &&
		IsMyTurn(false))
	{
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		if (Doc()->m_golfGame.gameType == 1)
			TEAM(PLAYER(cur)->team)->gauge = Doc()->GetComboGaugeLimit(PLAYER(cur)->team);
		else
			PLAYER(cur)->gauge = Doc()->GetComboGaugeLimit(cur);
	}
#endif
	if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
	{
		// 983 (B574): REPLAY fora do conteudo local = Replay(shift) ou fita
#ifdef JP983_DEV_PREVIEW
		if (DevPreviewCtrlR983()) // Ctrl+R da conta GM = previsao (ShotPreview::OnProcess), nao replay
			;
		else
#endif
		if (g_input->GetDown("REPLAY", true) && m_bReplayReady != 0 && IsMyTurn(false) &&
			GOLFDOC()->m_tutorialMode == 0xf && SHAREDDOC_ULONG(0x1048c) == 0)
		{
			if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
				Replay(g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true));
			else if (GOLFDOC()->m_bReplaying == 0)
				ReplayTapeCheckAndUse();
		}
	}
	else if (
		Doc()->m_replayPlayType == 4 && g_input->GetDown("REPLAY", true) &&
		IsMyTurn(false) && GOLFDOC()->m_tutorialMode == 0xf &&
		SHAREDDOC_ULONG(0x1048c) == 0 && GetCurMap() != 7)
	{
		Doc()->m_replayPlayType = 2;
		ReplayTapeCheckAndUse();
	}
	// 983: OnProcess nao fecha o ReplayBox aqui (bloco do 645 removido; B571)
	m_modeTime = time + m_modeTime;
	// B1321: 983 0069c7b1..c7f2 = sprintf(buf, K2L_Compatibility(fmt), 10, (int)t). O texto tem 43 bytes; o buf[20] do 983
	// (ebp-0x44) estoura por sorte num std::string ainda livre (ebp-0x30); o nosso buf[20] + double 10.0 no %d (numero de
	// 10 digitos) corrompia a pilha a cada quadro com o Go/Stop aberto (PM_NET_ASK_GOSTOP) -> queda.
	char buf[64];
	if (m_pGoStopDlg != NULL)
	{
		m_goStopTime983 = time + m_goStopTime983;
		sprintf(buf,
			K2L_Compatibility("%d\xc3\xca \xc8\xc4\xbf\xa1 \xc0\xda\xb5\xbf\xc0\xb8\xb7\xce \xb0\xd4\xc0\xd3\xbf\xa1\xbc\xad \xb3\xaa\xb0\xa9\xb4\xcf\xb4\xd9. %d\xc3\xca"),
			10, (int)m_goStopTime983);
		m_pGoStopDlg->SetDesc(buf);
	}
	switch (GOLFDOC()->m_gameMode)
	{
	case 1:
		DoWaitForStart(time);
		break;
	case 2:
		DoPlayerPreview(time);
		break;
	case 4:
		DoCloseupCaddie(time);
		break;
	case 8:
		DoStand(time);
		break;
	case 0x10:
		DoGetReadyToCaptureBG(time);
		break;
	case 0x20:
		DoPowerGauge(time);
		break;
	case 0x40:
		DoDrive(time);
		break;
	case 0x80:
		DoFlyBall(time);
		break;
	case 0x100:
		DoNavigateCamera(time);
		break;
	case 0x200:
		DoTopViewCamera(time);
		break;
	case 0x400:
		DoDebugCamera(time);
		break;
	case 0x800:
		DoToDefaultCamera(time);
		break;
	case 0x1000:
		DoHoleInPose(time);
		break;
	case 0x2000:
		// B576: 983 (0066e540) so chama ShowScore da regra; o DoScoreBoard virou o caso 0x33b do HandleMsg
		if (m_pRuleBase)
			m_pRuleBase->ShowScore(*(int*)&time);
		else
			DoScoreBoard(time);
		break;
	case 0x4000:
		DoTimeOut(time);
		break;
	case 0x8000:
		DoFixedCamera(time);
		break;
	case 0x20000:
		DoGameOver(time);
		break;
	case 0x40000:
		DoSpotCamera(time);
		break;
	case 0x80000:
		DoDespairPose(time);
		break;
	case 0x200000:
		DoCoursePreview(time);
		break;
	case 0x800000:
		DoApproachNextHole(time);
		break;
	case 0x2000000: // 983 (00670b40)
		CCameraWork::Instance()->Process(time, 0.0f);
		break;
	}
	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("CANCEL", false))
	{
		// 983 0069ca61: ligado -> texto "esconde" e desliga; desligado -> texto "ativa" e liga (o nosso tinha os textos trocados), B1615
		if (GOLFDOC()->m_bShowChat != 0)
		{
			OpNotice(
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xb3\xbb \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xbc\xfb\xb1\xe9\xb4\xcf\xb4\xd9."));
			GOLFDOC()->m_bShowChat = 0;
		}
		else
		{
			OpNotice(
				K2L_Compatibility("\xb0\xd4\xc0\xd3\xb3\xbb \xb4\xeb\xc8\xad\xc3\xa2\xc0\xbb \xc8\xb0\xbc\xba\xc8\xad\xc7\xd5\xb4\xcf\xb4\xd9."));
			GOLFDOC()->m_bShowChat = 1;
		}
	}
	if (m_holeInCoinFxTime != 0.0f)
	{
		m_holeInCoinFxTime = m_holeInCoinFxTime - time;
		if (m_holeInCoinFxTime < 0.0f)
		{
			m_holeInCoinFxCount = m_holeInCoinFxCount - 1;
			m_holeInCoinFxTime = 0.0f;
			if (m_holeInCoinFxCount > 0)
				m_holeInCoinFxTime = 0.1f;
			SendMsg(this, "Effect", M983(0x17d), m_holeInCoinFxId, 0, 0, 0);
		}
	}
	CMouseCursor::Instance()->SetCursor(0);
	if (m_b983_59c) // 983 (0069cbf0)
	{
		m_premiumNoticeAlpha = m_premiumNoticeAlpha - m_premiumNoticeFadeRate * time;
		if (m_premiumNoticeAlpha < 0.0f)
			m_premiumNoticeAlpha = 0.0f;
	}
}

void CGolfRule::DoFixedCamera(float)
{
	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
	{
		CMouseCursor::Instance()->Store();
		ChangeGameMode(8);
		return;
	}
	if (TopViewClick())
		ChangeGameMode(0x200);
}

void CGolfRule::DoSpotCamera(float)
{
	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
	{
		CMouseCursor::Instance()->Store();
		ChangeGameMode(8);
		return;
	}
	if (TopViewClick())
		ChangeGameMode(0x200);
}

bool CGolfRule::CheckDropOut()
{
	if (m_pRuleBase && m_pRuleBase->GetNumTimeout() >= 3)
	{
		if (!OnlinePlay())
		{
			unsigned char cur = GOLFDOC()->m_currentPlayer;
			sPlayerData* player = PLAYER(cur);
			SendMsg(this, "GolfRule", M983(0x82), player->oid, 0, 0, 0);
		}
		return true;
	}
	return false;
}

bool CGolfRule::CheckGiveUp()
{
	if (m_pRuleBase)
		return m_pRuleBase->CheckGiveUp();
	return false;
}

void CGolfRule::DoDespairPose(float time)
{
	if (m_modeTime < m_modeDuration)
	{
		CCameraWork::Instance()->Process(time, 0.0f);
		return;
	}
	if (OnlinePlay() && !m_bTimeOutSent)
	{
		m_bTimeOutSent = true;
		if (!IsMassGame())
			SetNetMsgWait(true, 0xdb6, "DoDespairPose", 20.0f);
		// 983 (00695440): o pacote 0x1c so sai sem o dialogo Go/Stop (+0x46c); o 645 testava SharedDoc+0x11411
		if (m_pGoStopDlg == NULL)
		{
			WSendPacket packet((enumClientPacket)0x1c);
			EncodePacket((enumClientPacket)0x1c, packet);
			packet.Send(TO_GAME);
		}
		CheckNetMsgWait_Send("DoDespairPose");
	}
	if (m_bContinueDlg)
		return;
	if (m_bNetMsgWait)
		return;
	sPlayerData* cur = PLAYER(GOLFDOC()->m_currentPlayer);
	unsigned long id = ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1) == 0
		? Doc()->m_myInfo.info.dwGuid
		: Doc()->m_myInfo.info.dwGalleryGuid;
	unsigned long mode;
	if (cur->oid == id)
	{
		if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			unsigned long step = GOLFDOC()->m_tutorialMode;
			if (step == 8)
			{
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial == NULL)
					return;
				tutorial << MsgObject(NULL, M983(0x154), 0, 0, 0, 0, 0);
				return;
			}
			if (step == 7)
			{
				ReturnLobby(true);
				return;
			}
			if (step == 0xf && Doc()->m_gameMode == 0)
			{
				if (PLAYER(GOLFDOC()->m_currentPlayer)->state == 3)
					return;
				if (Doc()->m_golfGame.gameType == 1)
				{
					unsigned char self = GOLFDOC()->m_currentPlayer;
					unsigned char mate =
						*((unsigned char*)PLAYER(self) + 0x4ea);
					if (2 <
						(unsigned int)*((unsigned char*)PLAYER(mate) + 0x4db) +
							(unsigned int)*(
								(unsigned char*)PLAYER(self) + 0x4db))
						return;
					unsigned int limit = (Doc()->m_golfGame.holes + 1) / 2;
					if (limit < 4)
						limit = 3;
					if (limit <=
						(unsigned int)*((unsigned char*)PLAYER(mate) + 0x4dc) +
							(unsigned int)*(
								(unsigned char*)PLAYER(self) + 0x4dc))
						return;
				}
				else if ((unsigned char)(*((unsigned char*)DOCPTR983(0x109b8)) - 0x14) > 1) // 983 (00695440): so fora dos modos 0x14/0x15
				{
					if (2 <
						*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4db))
						return;
					unsigned int limit = (Doc()->m_golfGame.holes + 1) / 2;
					if (limit < 4)
						limit = 3;
					if (limit <=
						*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4dc))
						return;
				}
			}
			goto next;
		}
	}
	else
	{
next:
		if (!CanMoveToNextHole())
		{
			SendMsg(this, "Score", M983(0x12f), 0, 0, 0, 0);
			SendMsg(this, "Score", 0x164, 0, 0, 0, 0);
			if (g_pFresh != NULL)
				g_pFresh->GetManager()->CloseForm(false);
			ChangeGameMode(0x800);
			return;
		}
		if (GOLFDOC()->m_tutorialMode == 8)
		{
			IActor* tutorial = GetActor("Tutorial");
			if (tutorial == NULL)
				return;
			tutorial << MsgObject(NULL, M983(0x14b), 0, 0, 0, 0, 0);
			return;
		}
		if (GOLFDOC()->m_tutorialMode == 7)
		{
			ReturnLobby(false);
			return;
		}
		if (OnlinePlay() && !IsMassGame())
			return;
	}
	ChangeGameMode(0x2000);
}

void CGolfRule::DoTimeOut(float time)
{
	if (!(m_modeTime > 3.0f))
		return;
	if (OnlinePlay() && !m_bTimeOutSent)
	{
		m_bTimeOutSent = true;
		if (!IsMassGame())
			SetNetMsgWait(true, 0xe34, "DoTimeOut", 20.0f);
		WSendPacket packet((enumClientPacket)0x1c);
		EncodePacket((enumClientPacket)0x1c, packet);
		packet.Send(TO_GAME);
		CheckNetMsgWait_Send("DoTimeOut");
	}
	if (!m_bContinueDlg && !m_bNetMsgWait && m_pGoStopDlg == NULL)
	{
		m_bTimeOutSent = false;
		if (m_nextTurn == GOLFDOC()->m_currentPlayer)
		{
			PLAYER(GOLFDOC()->m_currentPlayer)
				->stroke[GOLFDOC()->m_currentHole - 1] += 1;
			if (Doc()->m_golfGame.gameType == 1)
			{
				TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
					->stroke[GOLFDOC()->m_currentHole - 1] += 1;
			}
			SendMsg(this, "Spot", M983(0x163), 0, 0, 0, 0);
			SendMsg(this, "Grid", M983(0x163), 0, 0, 0, 0);
			SendMsg(this, "Clock", M983(0x1e9), 0, 0, 0, 0);
			SendMsg(this, "Player", M983(0x10f), 0, 0, 0, 0);
			SendMsg(this, "Caddie", 0x2716, (int)"\xb1\xe2\xba\xbb", 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x1db), 0, 0, 0, 0);
			ChangeGameMode(8);
			return;
		}
		GOLFDOC()->m_currentPlayer = m_nextTurn;
		PLAYER(GOLFDOC()->m_currentPlayer)
			->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		if (Doc()->m_golfGame.gameType == 1)
		{
			TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
				->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		}
		SetEnvironmentVariableA();
		SetTurnChangeEvent();
		ReadyForNextTurn();
	}
	// keep: inline COMDATs the original emitted out of line (never called live)
	if (0)
	{
		IsWebLogin();
		CHeadIcon::IsInstantiated();
		CHelpMsg::IsInstantiated();
		CRival::IsInstantiated();
		CCameraWork::IsInstantiated();
		CHeadIcon::Instance()->Show();
		CHeadIcon::Instance()->Hide();
		CHeadIcon::Instance()->GetVisible();
		CClub* club = NULL;
		club->BarIs(0);
		CMouseCursor::Instance()->GetArea();
		CMouseCursor::Instance()->SetMode(0);
		COption::Instance()->vGetTnLMode();
		CMessengerInfo::Instance()->IsGameFocus();
		CTHunter::Instance()->SetReceiveGiftPacket(false);
		CTHunter::IsInstantiated();
		CRandomHouse::IsInstantiated();
		CGolfBall* ball = NULL;
		ball->ResetVectorSlideCount();
		ball->NewPreData();
		CQuadTree* qt = NULL;
		qt->GetLowest();
		CMainFrame* mf = NULL;
		mf->GetGameSpeed();
		FrScrollBar* sb = NULL;
		sb->GetRowCapacity();
		GetCurMapProp();
		GetHoles();
		GetStartHole();
		GetWeather();
		GetShotTimeLimit();
		UIAlpha();
		SetGameOver(false);
		IsGameOver();
		GetGameTimeLimit();
		IsObserver(0);
		IsOthersTurn(false);
		WView* view = NULL;
		WSphere sph(WVector(), 1.0f);
		view->InFrustum(sph);
		WPuppet* pup = NULL;
		pup->GetRootBone();
		CTask* task = NULL;
		task->SetPause(false);
		WVector kv;
		kv.Reset();
		FrWnd* fw = NULL;
		fw->HasKeyFocus();
		fw->SetAlpha(1.0f);
		CFxSpray* fx = NULL;
		fx->Vel();
		club->GetName();
		club->GetIndex();
		club->GetID();
		club->GetBarSpeed();
		club->SetRandomPower(0.0f);
		club->GetRandomPower();
		club->SetCalcRandomPower(false);
		WCrypticValue<int>* cv = NULL;
		*cv = *cv;
		CreateForm<FrClosedBonusBallResultDlg>(NULL, NULL, NULL, NULL);
		CreateForm<FrApproachResultDlg>(NULL, NULL, NULL, NULL);
		// only the naked OnProcess/ChangeGameMode/SetHoleInPose/DriveGauge reference these now
		DoFlyBall(0.0f);
		DoFlyBall_Flying(0.0f);
		DoDebugCamera(0.0f);
		GetBarCondition();
		SetHoleInPose_Skins();
		Bar();
		GetEndHole();
		GetPVS().SetFrame(0);
		GetPVS().GetPenaltyPos();
		GolfBall().GetPowerFactor();
		SetFlagCutin(GetFlagCutin());
		CAutoTarget* at = NULL;
		at->GetDropPos();
		qt->GetFrame();
		WSingleton<CSoundItemManager>::Instance();
		WSingleton<CShuffleBonus>::Instance();
		kv += kv;
		WMatrix* km = NULL;
		km->Reset();
		WFlags* kf = NULL;
		kf->Enable(0);
		CChatMsg* kc = NULL;
		kc->IsClosed();
		FrEdit* ke = NULL;
		ke->SetFontColor(0);
		WCrypticValue<float>* cvf = NULL;
		*cvf + 0.0f;
		std::stack<sBackUp>* backupStack = NULL;
		backupStack->push(*(sBackUp*)NULL);
		std::list<sShotData>* shots = NULL;
		shots->std::list<sShotData>::list(*shots);
		*shots = *shots;
		CMouseCursor::Instance()->Enable(0, false);
		Wind().IsActiveSpecialWind();
		Wind().SetActive(false);
		CRival::Instance()->GetPos();
		CCameraWork* cw = CCameraWork::Instance();
		cw->GetCameraType();
		cw->Lock();
		cw->SetReplayCount(0);
		cw->SetAbsAnim();
		WMatrix cm;
		cw->SetCamera(cm);
		CCameraWork::sInput* cin = NULL;
		cin->Reset();
		RANKINGDLG();
		sBar* bar = NULL;
		bar->Persent();
		bar->Delta();
		mf->SetGameSpeed(1.0f);
		EncryptKey* ek = NULL;
		ek->GetKey(0);
		sBriefUserInfo* bi = NULL;
		bi->IsIdentity(0);
		Fresh* fresh = NULL;
		fresh->Toggle(false);
		CGolfDoc* gd = NULL;
		gd->GetUIAlpha();
		gd->GetBar();
		gd->GameMode();
		gd->NetGameEnd();
		gd->IsNetGameEnd();
		gd->SetTeeShotReady(false);
		gd->IsTeeShotReady();
		gd->SetPause(false);
		gd->IsPause();
		gd->SetTeamGameWinner(TEAM_RED);
		gd->SetReport(false);
		gd->ShowChat(false);
		gd->IsShowChat();
		gd->SetStatUpdated(false);
		gd->GetStatUpdated();
		gd->GetApproachReloaded();
		gd->SetApproachReloaded(false);
		CSharedDoc* sd = NULL;
		sd->SetReplayState(0);
		sd->GetReplayState();
		sd->GetFileName();
		sd->GetReplayPlayType();
		sd->SetReplayPlayType(0);
		sd->SetGoStopMode(false);
		sd->GetGoStopMode();
		WCrypticValue<unsigned char>* cve = NULL;
		*cve >= (unsigned char)0;
		*cve + (unsigned char)0;
		WCrypticValue<int>* cvh = NULL;
		*cvh > 0;
		WVector2D v2a(0.0f, 0.0f);
		WVector2D v2b;
		v2b.Reset();
		v2a = v2b;
		v2a = 1.0f * v2b;
		v2a = v2b * 1.0f;
		v2a = v2a + v2b;
		kv -= kv;
		std::list<sShotData>* shotList = NULL;
		shotList->size();
		std::list<sShotData>::iterator sit = shotList->begin();
		sit++;
		const std::list<sItemInfo>* itemList = NULL;
		std::list<sItemInfo>::const_iterator iit = itemList->begin();
		*iit;
		std::list<unsigned long>* klistPtr = NULL;
		std::list<unsigned long>& klist = *klistPtr;
		std::list<unsigned long>::iterator kit = klist.begin();
		kit++;
		std::list<unsigned long>::const_iterator kcit = klist.begin();
		klist.back();
		Min<int>(0, 1);
		Min<float>(0.0f, 1.0f);
		atanf(0.5f);
		floorf(0.5f);
		std::map<unsigned long, sBriefUserInfo>* briefMap = NULL;
		std::pair<const unsigned long, sBriefUserInfo>* briefPair = NULL;
		(*briefMap)[0];
		std::vector<std::string>* strVec = NULL;
		std::string* str = NULL;
		strVec->push_back(*str);
		strVec->clear();
		std::list<sSCardAvilityPeriodInfo>* cardList = NULL;
		cardList->size();
	}
}

void CGolfRule::DoGameOver(float time)
{
	if (m_modeTime < m_modeDuration)
	{
		CCameraWork::Instance()->Process(time, 0.0f);
		return;
	}
	if (!(m_modeTime < 10.0f))
	{
		if (Doc()->m_golfGame.gameType <= 1 && OnlinePlay() &&
			!CTHunter::Instance()->IsReturnLobby())
			return;
		if (!IsNetWaiting983() ||
			(unsigned char)(*((unsigned char*)DOCPTR983(0x109b8)) - 0x14) < 2)
		{
			if (OnlinePlay())
			{
				// 983 (0067f1d7): modo fora de 0x14/0x15 sempre avisa o servidor; nos modos 0x14/0x15 so com o GP AwardDlg terminado
				if ((unsigned char)(*((unsigned char*)DOCPTR983(0x109b8)) - 0x14) > 1)
				{
					SetNetMsgWait(true, 0xe98, "DoGameOver", 20.0f);
					this << MsgObject(NULL, M983(0x8f), 1, 0, 0, 0, 0); // B1362: 983 0067f21d manda 0xaa do 983 = M983(0x8f) (volta a sala), nao M983(0xaa) (= 0xce, mensageiro)
					CheckNetMsgWait_Send("DoGameOver");
					return;
				}
				if (FrGrandPrixAwardDlg::s_pInstance &&
					FrGrandPrixAwardDlg::s_pInstance->IsEnd983()) // B1483: m_bEnd (VC10 +0x170; no cl 14.44 o membro fica em 0x14c)
				{
					SetNetMsgWait(true, 0xea8, "DoGameOver", 20.0f);
					this << MsgObject(NULL, M983(0x8f), 1, 0, 0, 0, 0); // B1362: 983 0067f21d manda 0xaa do 983 = M983(0x8f) (volta a sala), nao M983(0xaa) (= 0xce, mensageiro)
					CheckNetMsgWait_Send("DoGameOver");
					return;
				}
			}
			else if (WNetworkSystem::Instance()->IsConnected(
					WNetworkSystem::NET_GAME))
			{
				switch (Doc()->m_gameMode)
				{
				case 2:
				{
					m_bContinueDlg = true;
					FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "notify_yesno", NULL);
					pDlg->Open((FRESH_PFN_RESULT)&CGolfRule::ContinueDlgResult,
						0); // 983 0067f130: flag 0 (0xffffffe8 e o ajuste de this do ponteiro-membro de 16 bytes, nao flag)
					pDlg->SetMessage(
						K2L_Compatibility("\xbf\xac\xbd\xc0\xc0\xbb \xb0\xe8\xbc\xd3 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"), // 983 0067f130: texto via K2L_Compatibility (00b4ec60)
						false);
					pDlg->EnableDrag(false);
					break;
				}
				case 5:
					Doc()->m_bGameOver = true;
					CProjectG::Instance()->FromGolfToLobbySettings();
					CTaskManager::Instance()->ChangeTask("CGhostTask", "",
						CTaskManager::Instance()->GetPreservedTask() != NULL);
					CTaskManager::Instance()->PostMsg(NULL, "Ghost", 0,
						(int)"GHOST", 0, 0, 0);
					break;
				case 1:
					Doc()->m_bGameOver = true;
					CProjectG::Instance()->FromGolfToLobbySettings();
					CTaskManager::Instance()->ChangeTask("CFamilyTask", "",
						false);
					PostMsg(this, "Family", 0, (int)"FAMILY", 0, 0, 0);
					break;
				}
			}
		}
	}
}

bool CGolfRule::HaveTeeShots()
{
	if (Doc()->m_golfGame.gameType == 1)
	{
		unsigned char index = 1 - m_curTeam;
		sTeamData* team = TEAM(index);
		if (team->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)0)
			return true;
		return false;
	}
	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (PLAYER(i)->state == 0)
		{
			sPlayerData* player = PLAYER(i);
			if (player->stroke[GOLFDOC()->m_currentHole - 1] ==
				(unsigned char)0)
				return true;
		}
	}
	return false;
}

void CGolfRule::RecordPlayerData(unsigned char index)
{
	// 983 00678c90: sem o laco de ordem de buraco/holeNum do 645; holeNum sempre sobe
	if (PLAYER(index)->state == 3)
		return;
	PLAYER(index)->item = 0;
	PLAYER(index)->windAbility983 = 0;
	unsigned char mode = *((unsigned char*)DOCPTR983(0x109b8));
	bool modeFlag = mode == 0xc || mode == 0xe || mode == 0x13;
	if (GOLFDOC()->m_bNetGameEnd == 0 || modeFlag)
	{
		CGolfDoc* doc = Doc()->m_pGolfDoc;
		unsigned char* totalStroke = &doc->GetPlayer(index)->totalStroke;
		sPlayerData* strokes = PLAYER(index);
		*totalStroke += strokes->stroke[GOLFDOC()->m_currentHole - 1];
		unsigned char gameType = Doc()->m_golfGame.gameType;
		if (gameType == 1)
		{
			sTeamData* team = TEAM(PLAYER(index)->team);
			PLAYER(index)->score = team->score;
		}
		else if (gameType != 7)
		{
			int* score = &PLAYER(index)->score;
			CGolfDoc* scoreDoc = Doc()->m_pGolfDoc;
			unsigned char stroke =
				PLAYER(index)->stroke[scoreDoc->m_currentHole - 1];
			*score += stroke - GOLFDOC()->GetHoleData().par;
		}
	}
	unsigned long* holeNum = &PLAYER(index)->holeNum;
	++*holeNum;
	PLAYER(index)->state = 1;
}

void CGolfRule::RecordTeamData(unsigned char team)
{
	unsigned char& total = TEAM(team)->totalStroke;
	sTeamData* data = TEAM(team);
	total += data->stroke[GOLFDOC()->m_currentHole - 1];
	TEAM(team)->state = 1;
}

void CGolfRule::ResetRank()
{
	unsigned char i;
	unsigned char j;
	switch (Doc()->m_golfGame.gameType)
	{
	case 7:
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state != 3)
			{
				PLAYER(i)->rank = 1;
				PLAYER(i)->order = 1;
				for (j = 0; j < GOLFDOC()->m_playerNum; j++)
				{
					if (i != j && PLAYER(j)->state != 3)
					{
						if ((int)RrPlayer(j)->skinsSumPang >
							(int)RrPlayer(i)->skinsSumPang)
						{
							PLAYER(i)->rank++;
							PLAYER(i)->order++;
						}
						else if ((int)RrPlayer(j)->skinsSumPang ==
							(int)RrPlayer(i)->skinsSumPang)
						{
							if (RrPlayer(i)->index > RrPlayer(j)->index)
								PLAYER(i)->order++;
						}
					}
				}
			}
		}
		break;
	case 6:
		if (GOLFDOC()->m_currentHole >= 1 && GOLFDOC()->m_currentHole <= 18)
		{
			sRivalData* left = GUILDMATCHUP(0);
			sRivalData* right = GUILDMATCHUP(1);
			for (int i = 0; i < GOLFDOC()->m_currentHole; i++)
			{
				if (left->holeStroke[i] == 0 || right->holeStroke[i] == 0)
					break;
				if (left->holeStroke[i] < right->holeStroke[i])
					GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).result = 0;
				else if (left->holeStroke[i] > right->holeStroke[i])
					GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).result = 1;
				else
					GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).result = 2;
			}
		}
		break;
	case 0:
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state != 3)
			{
				PLAYER(i)->order = 1;
				PLAYER(i)->rank = 1;
				for (j = 0; j < GOLFDOC()->m_playerNum; j++)
				{
					if (i != j && PLAYER(j)->state != 3)
					{
						if (RrPlayer(i)->score > RrPlayer(j)->score ||
							RrPlayer(i)->score == RrPlayer(j)->score &&
								RrPlayer(i)->totalPang < RrPlayer(j)->totalPang)
						{
							PLAYER(i)->order++;
							PLAYER(i)->rank++;
						}
						if (RrPlayer(i)->score == RrPlayer(j)->score &&
							RrPlayer(i)->totalPang == RrPlayer(j)->totalPang)
						{
							if (RrPlayer(i)->index > RrPlayer(j)->index)
								PLAYER(i)->order++;
						}
					}
				}
			}
		}
		break;
	case 1:
		if (GOLFDOC()->m_currentHole >= 1 && GOLFDOC()->m_currentHole <= 18)
		{
			if (TEAM(0)->state != 3 && TEAM(1)->state != 3)
			{
				unsigned char stroke1 =
					TEAM(1)->stroke[GOLFDOC()->m_currentHole - 1];
				if (TEAM(0)->stroke[GOLFDOC()->m_currentHole - 1] < stroke1)
				{
					GOLFDOC()->GetHoleData().result = 0;
					if (GOLFDOC()->m_bNetGameEnd)
						break;
					TEAM(0)->score++;
				}
				else
				{
					unsigned char stroke1b =
						TEAM(1)->stroke[GOLFDOC()->m_currentHole - 1];
					if (TEAM(0)->stroke[GOLFDOC()->m_currentHole - 1] >
						stroke1b)
					{
						GOLFDOC()->GetHoleData().result = 1;
						if (GOLFDOC()->m_bNetGameEnd)
							break;
						TEAM(1)->score++;
					}
					else
						GOLFDOC()->GetHoleData().result = 2;
				}
			}
			else
			{
				int lost = TEAM(0)->state;
				GOLFDOC()->GetHoleData().result = lost == 3;
				if (GOLFDOC()->m_bNetGameEnd)
					break;
				PLAYER(lost == 3)->score++;
			}
		}
		break;
	}
	SendMsg(this, "Score", M983(0x131), 0, 0, 0, 0);
	if (!OnlinePlay())
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 1:
			if (HpTeam(m_curTeam)->stroke[GOLFDOC()->m_currentHole - 1] >
				HpTeam(1 - m_curTeam)->stroke[GOLFDOC()->m_currentHole - 1])
				m_curTeam = 1 - m_curTeam;
			for (unsigned char t = 0; t < 2; t++)
			{
				RrTeam(t)->teeShooter = 1 - RrTeam(t)->teeShooter;
				RrTeam(t)->turn = RrTeam(t)->teeShooter;
			}
			break;
		case 0:
			for (i = 0; i < GOLFDOC()->m_playerNum; i++)
			{
				if (PLAYER(i)->state == 3)
					GOLFDOC()->GetHoleData().rank[i] = GOLFDOC()->m_playerNum;
				else
				{
					GOLFDOC()->GetHoleData().rank[i] = 1;
					for (j = 0; j < GOLFDOC()->m_playerNum; j++)
					{
						if (i != j && PLAYER(j)->state != 3)
						{
							if (RrPlayer(i)->stroke[GOLFDOC()->m_currentHole -
									1] > RrPlayer(j)
											 ->stroke[GOLFDOC()->m_currentHole -
												 1] ||
								RrPlayer(i)->stroke[GOLFDOC()->m_currentHole -
									1] ==
										RrPlayer(j)
											->stroke[GOLFDOC()->m_currentHole -
												1] &&
									RrPlayer(i)->teeOrder >
										RrPlayer(j)->teeOrder)
								GOLFDOC()->GetHoleData().rank[i]++;
						}
					}
				}
			}
			for (i = 0; i < GOLFDOC()->m_playerNum; i++)
			{
				sHoleData& hole = GOLFDOC()->GetHoleData();
				sPlayerData* player = PLAYER(i);
				player->teeOrder = hole.rank[i] - 1;
			}
			break;
		}
	}
}

bool CGolfRule::CanMoveToNextHole()
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	{
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state == 0)
				return false;
			if (PLAYER(i)->state == 2)
			{
				sPlayerData* player = PLAYER(i);
				if (player->stroke[GOLFDOC()->m_currentHole - 1] ==
					(unsigned char)0)
					return false;
			}
		}
		ResetRank();
		return true;
	}
	case 7: // 983 (00689b10): sem o gameType 3 do 645 (cai no default = false)
		if (PLAYER(0)->state == 1 && PLAYER(1)->state == 1)
			return true;
		if (PLAYER(0)->state == 1)
		{
			sPlayerData* player1 = PLAYER(1);
			unsigned char stroke1 =
				player1->stroke[GOLFDOC()->m_currentHole - 1];
			sPlayerData* player0 = PLAYER(0);
			if (player0->stroke[GOLFDOC()->m_currentHole - 1] <= stroke1)
				return true;
		}
		else if (PLAYER(1)->state == 1)
		{
			unsigned char stroke1 =
				PLAYER(1)->stroke[GOLFDOC()->m_currentHole - 1];
			if (PLAYER(0)->stroke[GOLFDOC()->m_currentHole - 1] >= stroke1)
				return true;
		}
		return false;
	case 1:
	{
		bool win[2];
		win[0] = TEAM(0)->state == 1;
		win[1] = TEAM(1)->state == 1;
		if (win[0] && win[1])
			return true;
		sTeamData* team0 = TEAM(0);
		unsigned char stroke0 = team0->stroke[GOLFDOC()->m_currentHole - 1];
		sTeamData* team1 = TEAM(1);
		unsigned char stroke1 = team1->stroke[GOLFDOC()->m_currentHole - 1];
		if (win[0])
		{
			if (stroke0 <= stroke1)
				return true;
		}
		else if (win[1] && stroke0 >= stroke1)
			return true;
		break;
	}
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	{
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		return PLAYER(cur)->state == 1;
	}
	}
	return false;
}

void CGolfRule::RecordGiveUp(unsigned char index)
{
	if (index < GOLFDOC()->m_playerNum)
	{
		unsigned char maxStroke = GOLFDOC()->GetHoleData().par + 4;
		if (Doc()->m_golfGame.gameType == 1)
		{
			unsigned char team = PLAYER(index)->team;
			sTeamData* pTeam = TEAM(team);
			pTeam->stroke[GOLFDOC()->m_currentHole - 1] = maxStroke + 1;
		}
		else
		{
			sPlayerData* pPlayer = PLAYER(index);
			pPlayer->stroke[GOLFDOC()->m_currentHole - 1] = maxStroke + 1;
		}
		PLAYER(index)->giveUp++;
	}
}

void CGolfRule::RecordScoreByPlayer(unsigned char index)
{
	if (index < GOLFDOC()->m_playerNum)
	{
		if (Doc()->m_golfGame.gameType == 1)
			RecordTeamData(PLAYER(index)->team);
		RecordPlayerData(index);
		if (Doc()->m_golfGame.gameType == 1)
		{
			if (PLAYER(index)->teammate != 0xff)
				RecordPlayerData(PLAYER(index)->teammate);
		}
		SendHoleStat();
	}
}

void CGolfRule::RecordScoreByTeam(unsigned char team)
{
	if (Doc()->m_golfGame.gameType == 1 && team < 2)
	{
		for (int i = 0; i < 4; i++)
		{
			if (PLAYER(i)->team == team)
			{
				RecordScoreByPlayer(i);
				return;
			}
		}
	}
}

void CGolfRule::ResetData(const WVector& pos)
{
	unsigned char count = 0;
	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (PLAYER(i)->state == 3)
			continue;
		PLAYER(i)->event = 0;
		PLAYER(i)->flag = 0;
		PLAYER(i)->startPos = pos;
		PLAYER(i)->pos = pos;
		PLAYER(i)->timeout = 0;
		PLAYER(i)->holeInPose = 0;
		PLAYER(i)->holeInCollision = 0;
		PLAYER(i)->bunker =
			Doc()->m_playingMode == 2 ? m_shotData.bunker : 0;
		if (GOLFDOC()->m_skinsHolePot == 0)
		{
			PLAYER(i)->skinsHolePang = 0;
		}
		count++;
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		unsigned char t = 0;
		do
		{
			TEAM(t)->pos = pos;
			TEAM(t)->holeInPose = 0;
			TEAM(t)->holeInCollision = 0;
			TEAM(t)->bunker = 0;
			t++;
		} while (t < 2);
	}
	GOLFDOC()->GetHoleData().playerNum = count;
	m_bShotInFlight = 0;
	m_bTimeOutSent = false;
	m_bReplayReady = 0;
	m_bTurnStarted983 = 0;
	m_bFastForwardReady = 0;
	while (!m_backUpStack.empty())
		m_backUpStack.pop();
	SendMsg(this, "Ball", M983(0x1eb), 0, 0, 0, 0);
}

unsigned long CGolfRule::GetHolePang(unsigned char hole)
{
	unsigned long type = Doc()->m_curChannel.Type;
	float rate;
	if (type & 0x800)
		rate = 1.0f;
	else if (type & 0x1000)
		rate = 2.5f;
	else if (type & 0x2000)
		rate = 5.0f;
	else
		rate = 1.0f;

	float pang = rate * 20.0f * ((hole - 1) / 3 * 0.5f + 1.0f); // ordem do 983 00672240
	if (hole == Doc()->m_golfGame.holes)
		pang += pang;
	return (unsigned long)pang;
}

void CGolfRule::DoScoreBoard(float time)
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	{
		bool done = false;
		SendMsg(this, "Score", M983(0x138), (int)&done, 0, 0, 0);
		if (!done)
			return;
		break;
	}
	default:
		if (m_modeTime < 3.0f)
			return;
	}
	int online = OnlinePlay();
	CSharedDoc* shared = Doc();
	if (online)
	{
		// B1318: campos com nome (983 +0x15c/+0x7b4, como o ShowScore983 006a8130); os +0x14c/+0x794 crus eram do 645
		if (*(unsigned char*)&shared->m_bGameOver != 0 ||
			GOLFDOC()->m_bNetGameEnd != 0)
		{
			if (IsMassGame())
			{
				if (!GOLFDOC()->m_bApproach794)
					UpdateShotResultStat();
				ReturnLobby(false);
				return;
			}
			goto changeMode;
		}
		if (*((unsigned char*)GRPTR983(0x41e)) != 0)
			return;
		if (*((unsigned char*)GRPTR983(0x41f)) != 0)
			return;
		GolfBall().SetHoleIn(0);
		shared = Doc();
		char* doc = (char*)shared->m_pGolfDoc;
		unsigned char hole = *(unsigned char*)GDOCPTR983(doc, 0x17c);
		if (GetHoleIndex(hole) == *((unsigned char*)shared + DOCOFS983(0x10699)))
		{
			*(unsigned char*)GDOCPTR983(doc, 0x17c) = 1;
			GOLFDOC()->m_bNoStat = 1;
		}
		else
		{
			if (*((unsigned char*)shared + DOCOFS983(0x109b8)) == 0xe)
			{
				unsigned char index = GetHoleIndex(hole);
				SetCurMap(shared->m_pinIndex[index]);
				shared = Doc();
			}
			doc = (char*)shared->m_pGolfDoc;
			unsigned char index = GetHoleIndex(*(unsigned char*)GDOCPTR983(doc, 0x17c));
			*(unsigned char*)GDOCPTR983(doc, 0x17c) =
				shared->m_holeOrder[index];
		}
		GOLFDOC()->m_pPolySoup->LoadMapCheckData(GOLFDOC()->m_currentHole);
		SetNetMsgWait(true, 0x47f, "DoScoreBoard", 40.0f); // 983 (006a6f90): 0x47f (0x1079 do 645)
		SendHoleData();
		CheckNetMsgWait_Send("DoScoreBoard");
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
		CheckNetMsgWait_Send("DoScoreBoard");
		{
			unsigned char type = Doc()->m_golfGame.gameType;
			if (type == 0 || type == 1 || type == 4 || type == 5 || type == 6)
				SendMsg(this, "Score", M983(0x12f), 2, 0, 0, 0);
		}
	}
	else
	{
		if (*(unsigned char*)&shared->m_bGameOver != 0)
		{
changeMode:
			ChangeGameMode(0x20000);
			return;
		}
		char* doc = (char*)shared->m_pGolfDoc;
		if (GetHoleIndex(*(unsigned char*)GDOCPTR983(doc, 0x17c)) ==
			*((unsigned char*)shared + DOCOFS983(0x10699)))
		{
			*(unsigned char*)GDOCPTR983(doc, 0x17c) = 1;
			GOLFDOC()->m_bNoStat = 1;
			GOLFDOC()->m_pPolySoup->LoadMapCheckData(GOLFDOC()->m_currentHole);
		}
		else
		{
			_ReadWriteBarrier();
			doc = (char*)shared->m_pGolfDoc;
			unsigned char index = GetHoleIndex(*(unsigned char*)GDOCPTR983(doc, 0x17c));
			*(unsigned char*)GDOCPTR983(doc, 0x17c) =
				shared->m_holeOrder[index];
		}
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
		{
			unsigned char type = Doc()->m_golfGame.gameType;
			if (type == 0 || type == 1 || type == 4 || type == 5 || type == 6)
				SendMsg(this, "Score", M983(0x12f), 2, 0, 0, 0);
		}
	}
}

void CGolfRule::DoHoleInPose(float time)
{
	if (m_modeTime < m_modeDuration)
	{
		CCameraWork::Instance()->Process(time, 0.0f);
		return;
	}
	if (OnlinePlay() && !m_bTimeOutSent && Doc()->m_playingMode == 1)
	{
		m_bTimeOutSent = true;
		if (!IsMassGame())
			SetNetMsgWait(true, 0x10d4, "DoHoleInPose", 20.0f);
		WSendPacket packet((enumClientPacket)0x1c);
		EncodePacket((enumClientPacket)0x1c, packet);
		packet.Send(TO_GAME);
		CheckNetMsgWait_Send("DoHoleInPose");
	}
	if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_playingMode == 2)
	{
		m_bContinueDlg = false;
		m_bNetMsgWait = false;
		if (Doc()->m_replayState == 7)
		{
			Doc()->m_replayState = 9;
			g_input->SetFakeInput("FUNC_11", false);
		}
	}
	if (!m_bContinueDlg && !m_bNetMsgWait)
	{
		if ((unsigned long)GOLFDOC()->m_tutorialMode < 0xf)
		{
			SendMsg(this, "GolfRule", M983(0x88), 1, 0, 0, 0);
			IActor* tutorial = GetActor("Tutorial");
			if (tutorial)
			{
				if (GOLFDOC()->m_tutorialMode != 8)
				{
					if (GOLFDOC()->m_tutorialMode != 0xc)
					{
						tutorial << MsgObject(NULL, M983(0x148), 0, 0, 0, 0, 0);
						return;
					}
					tutorial << MsgObject(NULL, M983(0x154), 0, 0, 0, 0, 0);
					return;
				}
				if ((unsigned char)PLAYER(0)
						->stroke[GOLFDOC()->m_currentHole - 1] > 4)
				{
					tutorial << MsgObject(NULL, M983(0x154), 0, 0, 0, 0, 0);
					return;
				}
				tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
				return;
			}
		}
		else
		{
			unsigned long mode;
			if (Doc()->m_golfGame.gameType == 7)
			{
				if (!m_bSkinsNextHole)
					return;
				mode = 0x2000;
			}
			else if (CanMoveToNextHole())
			{
				if (OnlinePlay() && !IsMassGame())
					return;
				mode = 0x2000;
			}
			else
			{
				if (IsLocalContent(S4_REPLAY_SYSTEM) &&
					Doc()->m_playingMode == 2)
					return;
				SendMsg(this, "Score", M983(0x12f), 0, 0, 0, 0);
				SendMsg(this, "Score", 0x164, 0, 0, 0, 0); // 983 00695c20 (placar do Grand Prix)
				if (g_pFresh != NULL)
					g_pFresh->GetManager()->CloseForm(false);
				mode = 0x800;
			}
			ChangeGameMode(mode);
		}
	}
}

void CGolfRule::DoWaitForStart(float time)
{
	FlyBy(time);
	m_bWaitForStart = true;
	if (GOLFDOC()->m_currentHole != Doc()->m_holeOrder[0])
		ChangeGameMode(2);
	// B723: 983 00695f90 so volta ao lobby fora dos tipos 11..14 e 0x13
	// 983 (00695f90): o tipo vem de +0x109b8 (sRoomInfo::realGameType), nao de m_golfGame.gameType (Practice = 0x13 so la)
	unsigned char type = Doc()->m_roomInfo.realGameType;
	if (Doc()->m_slotList.size() == 1 && (type < 11 || type > 14) && type != 0x13)
		ReturnLobby(false);
}

// JP 983 (0066e870): espera de dialogo ou de resposta da rede
bool CGolfRule::IsWaiting983()
{
	return m_bContinueDlg || m_bNetMsgWait || m_pGoStopDlg != NULL;
}

// JP 983 (0066e8c0)
bool CGolfRule::IsModeTimeOver983()
{
	return !(m_modeTime < 3.0f);
}

// 983: sem funcao propria (a CGolfRuleBase grava +0x2c8 direto em 006a8817)
C_ASSERT(offsetof(CGolfRule, m_modeDuration) == GOLFRULE_OFS_MODEDUR983); // B1463: golfrule30battle.cpp le cru; nos dois compiladores
C_ASSERT_VC10(offsetof(CGolfRule, m_bgPosList) == 0x3fc); // D50: FlyBy 00670bb0
void CGolfRule::SetModeDuration983(float time)
{
	m_modeDuration = time;
}

// JP 983 (0066e8a0)
bool CGolfRule::IsNetWaiting983()
{
	return m_bContinueDlg || m_bNetMsgWait;
}

// JP 983 (00671310). Pedido B72: retorno do aviso "notify_ok2" que a FrGrandPrixAwardDlg abre (00549f29).
bool CGolfRule::OnGPAwardNotifyResult983(int, FrForm*)
{
	if (FrGrandPrixAwardDlg::s_pInstance)
		FrGrandPrixAwardDlg::s_pInstance->OnNotifyOk983();
	return true;
}

// JP 983 (00671330)
void CGolfRule::SendMsg983(const char* target, const MsgObject& msg)
{
	GetActor(target) << msg;
}

C_ASSERT_VC10(offsetof(CGolfRule, m_modeTime) == 0x260); // B718: IsModeTimeOver983 (0066e8c0) le +0x260
C_ASSERT_VC10(offsetof(CGolfRule, m_bContinueDlg) == 0x41e);
C_ASSERT_VC10(offsetof(CGolfRule, m_bNetMsgWait) == 0x41f);
C_ASSERT_VC10(offsetof(CGolfRule, m_bServerHold983) == 0x455);
C_ASSERT_VC10(offsetof(CGolfRule, m_serverHoldTime983) == 0x458);
C_ASSERT_VC10(offsetof(CGolfRule, m_bChatIconSent983) == 0x464);
C_ASSERT_VC10(offsetof(CGolfRule, m_bNotifyPending983) == 0x634);
C_ASSERT_VC10(offsetof(CGolfRule, m_bTutorialShot983) == 0x439); // B1398: crus 0x439/0x43c/0x444/0x450 nomeados
C_ASSERT_VC10(offsetof(CGolfRule, m_tutorialGauge983) == 0x43c);
C_ASSERT_VC10(offsetof(CGolfRule, m_bPuttTutorial983) == 0x444);
C_ASSERT_VC10(offsetof(CGolfRule, m_cmdTick) == 0x450);
C_ASSERT_VC10(offsetof(CGolfRule, m_replayFlags) == 0x62c); // B725: DoFlyBall 0069a5e0 le +0x62c/+0x630
C_ASSERT_VC10(offsetof(CGolfRule, m_replayState) == 0x630);
C_ASSERT_VC10(offsetof(CGolfRule, m_pGoStopDlg) == 0x46c);
C_ASSERT(offsetof(CGolfRule, m_turnStartTime) == GOLFRULE_OFS_TURNSTART983); // B1322/B1463: golfrule30battle.cpp grava cru (fora do pch); nos dois compiladores
C_ASSERT(offsetof(CGolfRule, m_autoShotCount) == GOLFRULE_OFS_AUTOSHOT983); // B1463: SetMyTicket983 (golfrulebase.h) grava cru
C_ASSERT_VC10(sizeof(CGolfRule) == 0x650); // 983: sizeof 0x650 (golfrule.h)
C_ASSERT_VC10(offsetof(CSharedDoc, m_bFixHoleMoveTee983) == 0x11608);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.holes) == 0x10699);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_approachStartTime) == 0x1057c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_playingMode) == 0x1122c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) == 0x38b0);
C_ASSERT(offsetof(sUserInfo, mascotInfo) + offsetof(sMascotInfo, tid) == 0x318c);
C_ASSERT(sizeof(sUserInfo) == 0x31c6);
C_ASSERT(offsetof(sUserInfo, info.dwUID) == 0x12b);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwUID) == 0x8cb);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bTeeShotReady) == 0x15e);
// DriveGauge 983 0069ab60: campos do doc lidos por deslocamento
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar.power) == 0xe0);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar.gauge) == 0xe8);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar.gaugeStep) == 0xec);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bar.calipers) == 0x100);
C_ASSERT_VC10(offsetof(CGolfDoc, m_shotPhase) == 0x188);
C_ASSERT_VC10(offsetof(CGolfDoc, m_tutorialMode) == 0x88); // 983 0068c0c0: +0x88 = 0xf (fim do tutorial)
C_ASSERT_VC10(offsetof(CGolfDoc, m_inputList983) == 0x8c); // 983 00685980/CalcInputCommand: lista da regua em +0x8c (head) / +0x90 (size)
C_ASSERT_VC10(offsetof(CGolfDoc, m_bTutorialGaugeMute) == 0x16a);
C_ASSERT_VC10(offsetof(CGolfRule, m_b983_59c) == 0x59c);
C_ASSERT_VC10(offsetof(CGolfRule, m_pPremiumNotice) == 0x590 && offsetof(CGolfRule, m_premiumNoticeAlpha) == 0x594 && offsetof(CGolfRule, m_premiumNoticeFadeRate) == 0x598); // B1569
C_ASSERT_VC10(offsetof(CGolfRule, m_pChatView) == 0x4dc && offsetof(CGolfRule, m_pMyInfoBtn) == 0x4fc && offsetof(CGolfRule, m_pMoveTeeBtn) == 0x51c && offsetof(CGolfRule, m_sfxName) == 0x3dc); // B1569
C_ASSERT_VC10(offsetof(CGolfRule, m_bTrajectoryMsgSent) == 0x480 && offsetof(CGolfRule, m_bRainbowFx983) == 0x635); // B1569
C_ASSERT_VC10(offsetof(CGolfRule, m_bSkinsNextHole) == 0x4a8); // B1391: crus 0x4a8..0x4b7 do 983 nomeados
C_ASSERT_VC10(offsetof(CGolfRule, m_holeInCoinFxTime) == 0x4ac);
C_ASSERT_VC10(offsetof(CGolfRule, m_holeInCoinFxId) == 0x4b4);
C_ASSERT_VC10(offsetof(CGolfRule, m_bgmList) == 0x4b8);
C_ASSERT_VC10(offsetof(CGolfRule, m_pPredictShotUI) == 0x5cc);
C_ASSERT_VC10(offsetof(CGolfRule, m_pReplayControlDlg) == 0x4f0);
C_ASSERT_VC10(offsetof(CGolfRule, m_forgetDlgTime) == 0x4a0); // B1347: 00691f60 acumula o tempo da janela flowerofforgetting em +0x4a0
C_ASSERT_VC10(offsetof(CGolfRule, m_pForgetDlg) == 0x4e8); // B1347: 00691f60 guarda a janela em +0x4e8
C_ASSERT_VC10(offsetof(CGolfRule, m_pReplayBox) == 0x4f8);
C_ASSERT_VC10(offsetof(CGolfRule, m_shotData) == 0x354 && offsetof(CGolfRule, m_saveFileName) == 0x5dd); // s88: 006884f0
// B1566: crus 0x354..0x3cc do 983 = m_shotData (sem STL: mesmo layout nos dois compiladores); 0x350 = m_nextTurn
C_ASSERT(offsetof(CGolfRule::sShotData, wind) == 0x4 && offsetof(CGolfRule::sShotData, bar) == 0x14 && offsetof(CGolfRule::sShotData, pos) == 0x38);
C_ASSERT(offsetof(CGolfRule::sShotData, pvsFrame) == 0x50 && offsetof(CGolfRule::sShotData, club) == 0x5c && offsetof(CGolfRule::sShotData, item) == 0x64);
C_ASSERT(offsetof(CGolfRule::sShotData, holeStroke) == 0x6c && offsetof(CGolfRule::sShotData, gauge) == 0x78 && sizeof(CGolfRule::sShotData) == 0x88);
C_ASSERT_VC10(offsetof(CGolfRule, m_nextTurn) == 0x350);
C_ASSERT(offsetof(CGolfRule, m_shotData) == GOLFRULE_OFS_SHOTDATA); // A727: = visao parcial do golfrule.h, nos dois compiladores
C_ASSERT(offsetof(CGolfRule, m_bShotInFlight) == GOLFRULE_OFS_SHOTINFLIGHT983); // B1537: CGolfRuleJp::SetFlag2c0_983 (subgolfrule.h), nos dois compiladores
C_ASSERT_VC10(offsetof(CSharedDoc, m_cardAbilityList) == 0x37ec && sizeof(sSCardAvilityPeriodInfo) == 0x41);

// JP 983 (00696010). Diferencas para o 645: o pedido do pacote 0x34 passou a virtual [13] da regra; ao fim avisa o
// mascote (0x307) e, na vez do jogador local, o item de jogo (0x7532, sub 0x10).
void CGolfRule::DoPlayerPreview(float time)
{
	if (!m_b983_59c && Doc()->m_bPremiumEndNotice983)
		m_b983_59c = true;

	CCameraWork::Instance()->Process(time, 0.0f);
	// B1465: era CAMERAWORK_FLOAT(0x298)/(0x1c0) cru; no cl 14.44 o CCameraWork anda -8 (vector 0xc): lia m_b26c/m_f198 e o 0x34 nao saia
	if (CCameraWork::Instance()->m_f264983 > 0.0f &&
		CCameraWork::Instance()->m_f190983 > CCameraWork::Instance()->m_f264983)
	{
		if (OnlinePlay())
		{
			m_pRuleBase->RequestShotReview(time);
			CSharedDoc* shared = Doc();
			unsigned char gameType = shared->m_golfGame.gameType;
			if ((gameType == 10 || !IsMassGame()) && IsWaiting983())
				return;
			if (gameType == 10)
				shared->m_approachStartTime = g_CurrentTime;
		}
		else if (IsLocalContent((localContentType_t)0x50) &&
			Doc()->GetPlayingMode() == 2)
		{
			ChangeGameMode(8);
			if (GOLFDOC()->m_bTeeShotReady == 0)
				GOLFDOC()->m_bTeeShotReady = 1;
			return;
		}

		SetTurnChangeEvent();
		ReadyForNextTurn();
		if (GOLFDOC()->m_currentHole ==
			Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1])
			SendMsg(this, "Score", M983(0x135), 0, 0, 0, 0);
		// +0x3170 do sUserInfo: id da voz do personagem (645: 0x1e15 + n * 0xb92)
		CVoiceItem::Instance()->PlaySoundA(
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].clubInfo.tid,
			(CVoiceItem::eWaveState)0x20, -1);
		GetActor("Mascot") << MsgObject(NULL, 0x307 /*so do 983*/, 0, 0, 0, 0, 0);

		IActor* pGateway = AfxGetTask()->GetActor(s_gatewayActor[1]);
		if (Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].info.dwUID ==
			Doc()->m_myInfo.info.dwUID)
			pGateway << MsgObject(NULL, M983(0x26f), 0x10, 0, 0, 0, 0);
	}
}

void CGolfRule::DoCloseupCaddie(float time)
{
	if (m_modeTime < 3.0f)
	{
		CCameraWork::Instance()->Process(time, 0.0f);
		return;
	}
	CMouseCursor::Instance()->Reset(true);
	SendMsg(this, "Clock", M983(0x1e9), 0, 0, 0, 0);
	g_audio->PlaySfx("play_my_turn");
	ChangeGameMode(8);
}

void CGolfRule::ReadyForNextTurn()
{
	CMouseCursor::Instance()->Reset(true);
	SendMsg(this, "Clock", M983(0x1e9), 0, 0, 0, 0);
	ChangeTurn();
	GOLFDOC()->MakeDefaultCamera(true);
	WPuppet* ball = SCENEMGR()->GetPuppet("\xb0\xf8");
	if (ball && ball->m_rootbone)
		ball->SetAlpha(0xff);
	g_audio->StopSfx("shot_dist_count");
	g_audio->PlaySfx("play_my_turn");
	CCameraWork::Instance()->StopBallSound();
	// 983 (0067f410): inicio do turno (g_CurrentTime) em +0x648; o CGrandPrixArtifactRule::Check do resultado da tacada le
	m_turnStartTime = g_CurrentTime;
	ChangeGameMode(8);
}

void CGolfRule::DoDrive(float time)
{
	unsigned char shotType = (unsigned char)GolfClub().PowerShotType();
	if (CCameraWork::Instance()->m_cameraType >= 200 || // B1465: era +0x8c cru
		(shotType == 2 && m_bDriveCamStarted983 != 0) ||
		(shotType == 3 && m_bDriveCamStarted983 != 0))
		CCameraWork::Instance()->Process(time, 0.0f);
	if (m_bContinueDlg || m_bNetMsgWait)
		return;
	if (m_bDriveCamStarted983 == 0)
	{
		if ((m_replayFlags & 2) == 0 &&
			g_input->Get("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) == 0)
			CCameraWork::Instance()->Unlock();
		if (m_modeTime > 1.0f)
		{
			m_bDriveCamStarted983 = 1;
			SendMsg(this, "Player", M983(0x10e), 0, 0, 0, 0);
			if ((shotType == 2 || shotType == 3) &&
				*(char*)(*(int*)((char*)&GolfClub() + 4) + 0x22) == 0)
			{
				WVector pos;
				AfxGetTask()->GetActor("Player")
					<< MsgObject(NULL, M983(0x127), (int)&pos, 0, 0, 0, 0);
				int number = rand() % 4;
				const char* name =
					MakeStr("\xbf\xec\xb5\xe5\xc6\xc4\xbf\xf6\xbc\xa6"
							"2_%02d",
						number + 1);
				m_modeDuration =
					CCameraWork::Instance()->LoadCamera(
						(CCameraWork::eCamMode)2, (char*)name, pos);
				m_bSpecialCamDone983 = 0;
				return;
			}
		}
	}
	else if (m_modeTime > 11.0f)
		ChangeGameMode(0x80);
}

void CGolfRule::HandleMsg(const MsgObject& msg)
{
	CGolfBall& ball = GolfBall();
	CSceneManager* sceneMgr = CSceneManager::Instance();
	COption* option = COption::Instance();
	int id = msg.message;
	// 983 (006999e0/00699ab8): 0x274 (postado no OnPreLoadInit) e 0x275; no 645 isto rodava a cada quadro no OnProcess
	if (id == 0x274)
	{
		if (Doc()->m_playingMode == 2)
		{
			static const char* const keys[] = { "\xc1\xc2", "\xbf\xec", "\xbb\xf3", "\xc7\xcf", "W", "A", "S", "D", "ENTER" };
			for (int i = 0; i < 9; ++i)
				g_input->BlockKey(keys[i]);
			*(unsigned long*)((char*)m_pReplayBox + VC10_FRWND_STYLE) |= FWS_FIXED; // m_dwStyle (protegido; B1471: 0x40 no cl 14.44, +0x44 = m_wndText)
		}
		else
		{
			m_pReplayBox->SetVisible(false);
			m_pReplayBox->Enable(false);
		}
		return;
	}
	if (id == 0x275)
	{
		if (m_pReplayBox)
			m_pReplayBox->SetVisible(true);
		return;
	}
	// S68: era "if (id < 0x8e)" (arvore do switch do 645); os blocos sao disjuntos e a faixa nao vale com M983
	{
		if (id == M983(0x8d))
		{
			Drop(*(WVector*)msg.param1);
			return;
		}
		switch (id)
		{
		case M983(0x29):
		{
			SetReady();
			if (!IsLocalContent((localContentType_t)1))
				return;
			if (m_bClosedBonusBallPending)
			{
				m_bClosedBonusBallPending = false;
				FrClosedBonusBallResultDlg* dlg =
					CreateForm<FrClosedBonusBallResultDlg>(
						g_pFresh->GetManager(), this, "closed_bonusball", NULL);
				if (dlg)
				{
					dlg->Open(NULL, 3);
					dlg->SetClosedBonusBall((sBonusBall*)m_closedBonusBall);
					dlg->SetFixed(true); // 983 006965f3: m_dwStyle |= FWS_FIXED (nao m_accHoverTime)
				}
			}
			return;
		}
		case M983(0x37):
			if (Doc()->AddWhisperPartner(m_pChatTarget, (char*)msg.param1))
				m_pChatTarget->SetLine(1, (char*)msg.param1, 0, false, 0);
			if (CUserInfo::Instance()->IsVisible())
				CUserInfo::Instance()->Close();
			if (m_pChatEdit)
				m_pChatEdit->SetKeyFocus(true);
			return;
		case M983(0x38):
			Doc()->AddWhisperPartner(m_pChatTarget, (char*)msg.param1);
			return;
		case M983(0x3f):
			if (m_pChatEdit == NULL)
				*(int*)msg.param1 = 0;
			else
				*(const char**)msg.param1 = m_pChatEdit->GetLine(1, false);
			return;
		case M983(0x47):
			Doc()->InitRecentWhisperList(m_pChatTarget);
			return;
		case M983(0x5e):
		{
			if (Doc()->m_teamPlayerGuid[0] != (unsigned long)msg.param1 &&
				Doc()->m_teamPlayerGuid[1] != (unsigned long)msg.param1)
				return;
			sRivalData* left = GUILDMATCHUP(0);
			sRivalData* right = GUILDMATCHUP(1);
			if (left == NULL || right == NULL)
				return;
			int idx = msg.param2;
			unsigned char a = ROW_UCHAR(left, idx + 0x7f);
			if (a == 0)
				return;
			unsigned char b = ROW_UCHAR(right, idx + 0x7f);
			if (b == 0)
				return;
			char buf[1024];
			if (a < b)
				sprintf(buf, K2L_Compatibility(
					"%s\xb4\xd4\xc0\xcc %d\xc8\xa6\xbf\xa1\xbc\xad \xbd\xc2\xb8\xae\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9"),
					(char*)left + 8, idx);
			else if (b < a)
				sprintf(buf, K2L_Compatibility(
					"%s\xb4\xd4\xc0\xba %d\xc8\xa6\xbf\xa1\xbc\xad \xc6\xd0\xb9\xe8\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9"),
					(char*)left + 8, idx);
			else
				sprintf(buf, K2L_Compatibility(
					"%d\xc8\xa6\xc0\xba \xb9\xab\xbd\xc2\xba\xce\xb0\xa1 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
					idx);
			CChatMsg::Instance()->AddChatMsg(std::string(buf), 0xffff7878, true,
				false);
			if (ROW_UCHAR(right, idx + 0x7f) > ROW_UCHAR(left, idx + 0x7f))
				GOLFDOC()->GetHoleData(Doc()->m_holeOrder[idx - 1]).result =
					0;
			else if (ROW_UCHAR(left, idx + 0x7f) > ROW_UCHAR(right, idx + 0x7f))
				GOLFDOC()->GetHoleData(Doc()->m_holeOrder[idx - 1]).result =
					1;
			else
				GOLFDOC()->GetHoleData(Doc()->m_holeOrder[idx - 1]).result =
					2;
			return;
		}
		case M983(0x62):
			if (m_pEmoticonBtn)
				m_pEmoticonBtn->SetVisible(msg.param1 != 0);
			if (m_pLanguage)
				m_pLanguage->SetVisible(msg.param1 != 0);
			if (m_pReportBtn)
				m_pReportBtn->SetVisible(msg.param1 != 0);
			if (m_pChatTarget)
				m_pChatTarget->SetVisible(msg.param1 != 0);
			if (m_pChatEdit)
			{
				if (msg.param1 == 0)
					m_pChatEdit->SetVisible(false);
				else
				{
					m_pChatEdit->SetVisible(true);
					m_pChatEdit->SetKeyFocus(true);
				}
				g_pFresh->GetManager()->SetExclusiveKey(msg.param1 != 0);
			}
			return;
		case M983(0x63):
			if (m_pChatView)
			{
				int lines = CHATVIEW_LINES();
				int i = 0;
				if (0 < lines)
				{
					do
					{
						i++;
						m_pChatView->SetLine(i, "", 0, false, 0);
					} while (i < lines);
				}
			}
			return;
		case M983(0x64):
			if (m_pChatView)
			{
				int lines = CHATVIEW_LINES();
				int i = 1;
				if (1 < lines)
				{
					do
					{
						int next = i + 1;
						const char* text = m_pChatView->GetLine(next, false);
						if (text)
						{
							unsigned long anim = m_pChatView->GetAnimTime(next);
							m_pChatView->SetLine(i, text, 0, false, anim, 2); // 983 (case 0x71): textStyle 2 = contorno
						}
						i = next;
					} while (i < lines);
				}
				unsigned long now = timeGetTime();
				m_pChatView->SetLine(lines, (char*)msg.param1, msg.param2,
					false, now, 2);
			}
			return;
		case M983(0x65):
			if (Doc()->m_recentWhisperList.size() != 0)
				m_pChatTarget->SetLine(1, Doc()->m_recentWhisperList.back().c_str(),
					0, false, 0);
			return;
		case M983(0x69):
		{
			unsigned long mode = GOLFDOC()->m_gameMode;
			if (mode == 0x1000)
			{
				sPlayerData* p;
				if (Doc()->m_golfGame.gameType == 1)
				{
					sPlayerData* cur = PLAYER(GOLFDOC()->m_currentPlayer);
					sTeamData* team = TEAM(*((unsigned char*)cur + 0x4e9));
					p = (sPlayerData*)((unsigned char*)team +
						GOLFDOC()->m_currentHole * 0x10 - 0xc);
				}
				else
				{
					sPlayerData* cur = PLAYER(GOLFDOC()->m_currentPlayer);
					p = (sPlayerData*)((unsigned char*)cur +
						(GOLFDOC()->m_currentHole + 1) * 0x10 + 0xc); // 983: stroke[] +0x2c
				}
				unsigned char s4 = *((unsigned char*)p + 0xe);
				unsigned char s5 = *(unsigned char*)p;
				unsigned char par =
					*((unsigned char*)&GOLFDOC()->GetHoleData() + 0x18);
				unsigned char score = (unsigned char)~(s4 ^ s5);
				if (score != par && -1 < (int)(score - par))
					SetConstantBGM("bgm_over_par.mp3", true, true);
				else
					SetConstantBGM("bgm_under_par.mp3", true, true);
				return;
			}
			if (mode == 0x2000)
			{
				SetConstantBGM("bgm_scoreboard.mp3", false, true);
				return;
			}
			if (mode != 0x20000)
			{
				PlayBGM();
				return;
			}
			SetConstantBGM("bgm_scoreboard.mp3", true, true);
			return;
		}
		case M983(0x74):
			m_holePos983 = *(WVector*)msg.param1;
			return;
		case M983(0x76):
		{
			unsigned long mode = GOLFDOC()->m_gameMode;
			if (mode != 8)
			{
				if ((mode & 0x300) != 0)
				{
					// B578: 983 006964c0 case 0x91 manda RELEASE_REVIEW_CAMERA ao ShotReview
					SendMsg(this, "ShotReview", 0x2fd, (int)"RELEASE_REVIEW_CAMERA", 0, 0, 0);
					g_camera = ((CGolfDocEx2*)GOLFDOC())->GetDefaultCamera();
					CMouseCursor::Instance()->Store();
					ChangeGameMode(8);
				}
				return;
			}
			if (m_bServerHold983 == 0)
			{
				if (m_bUnplayable983 != 0)
				{
					ChangeGameMode(0x10);
					return;
				}
				g_audio->PlaySfx("play_unplayable", NULL, 0, NULL, NULL, 0.5f,
					200.0f); // 983 (006964c0): maxDistance = 00d00d90 (200), nao 0
			}
			return;
		}
		case M983(0x77):
			m_bUnplayable983 = msg.param1 != 0;
			return;
		case M983(0x78):
			ChangeGameMode(msg.param1);
			return;
		case M983(0x79):
		{
			if ((GOLFDOC()->m_gameMode & 0x40) == 0)
				return;
			if (OnlinePlay() == 0)
			{
				m_bNetMsgWait = false;
				m_pNetMsgRecv = "PM_NET_SHOT_RESULT";
				m_netMsgRecvTick = GetTickCount();
				g_iRecvPacketGR = g_iRecvPacketType;
			}
			else
				SendMsg(this, "Cutin", 0x30c, 0x10, 0, 0, 0); // 983 (00697eaf): 0x30c
			m_replayFlags &= SHOTRES_UCHAR(0x3d6);
			if (Doc()->m_golfGame.gameType == 0 &&
				(SHOTRES_UCHAR(0x3e0) & 0x20) != 0)
			{
				int classic = S5::CLASSICSRV::IsClassicGame983(); // 983 (00697f36)
				CSharedDoc* shared = Doc();
				if (classic == 0)
				{
					unsigned char two = 0;
					unsigned char three = 0;
					unsigned char four = 0;
					if (Doc()->m_golfGame.holes != 0)
					{
						unsigned long count = Doc()->m_golfGame.holes;
						unsigned char* hole = shared->m_holeOrder;
						do
						{
							unsigned char kind =
								ROW_UCHAR(&GOLFDOC()->GetHoleData(*hole), 0x19);
							if (kind == 2)
								two++;
							else if (kind == 3)
								three++;
							else if (kind == 4)
								four++;
							hole++;
							count--;
						} while (count != 0);
					}
					unsigned char map = GetCurMap();
					int* bonus = &SHOTRES(int, 0x3db); // 983 0069804b: bonusPang (0x3c3 + cur * 0x25 era o 645)
					*bonus = *bonus +
						(four * 3 + three * 2 + two) * ClearBonusRate983(map);
				}
				else
				{
					unsigned char i = 0;
					unsigned long bonus = 0;
					if (Doc()->m_golfGame.holes != 0)
					{
						do
						{
							unsigned char map = GetCurMap();
							bonus = S5::CLASSICSRV::GetClearBonus(
								ROW_UCHAR(&GOLFDOC()->GetHoleData(
											  Doc()->m_holeOrder[i]),
									0x19),
								map);
							i++;
						} while (i < Doc()->m_golfGame.holes);
					}
					int* dst = &SHOTRES(int, 0x3db); // 983 00697fa5
					*dst = *dst + bonus;
				}
			}
			m_bShotInFlight = 1;
			m_bFastForwardReady = GolfBall().m_groundType != 2;
			CorrectShotResult();
			SendMsg(this, "Player", M983(0x77), 0, 0, 0, 0);
			return;
		}
		case M983(0x7a):
		{
			if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
			{
				sceneMgr->SetVisible(MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer),
					true);
				CSceneManager::Instance()->SetVisible("\xc5\xac\xb7\xb4", true);
				CSceneManager::Instance()->SetVisible("\xc4\xb3\xb5\xf0", true);
				CShadowManager::Instance()->Enable(
					COption::Instance()->vIsShadowEnabled());
				SendMsg(this, "Score", M983(0x13b), 0, 0, 0, 0);
			}
			if (IsSharedFlag433Bit1())
				SendMsg(this, "ItemWindow", M983(0x195), 0, 0, 0, 0);
			GOLFDOC()->m_shotPhase = 3;
			g_audio->StopSfx("\xc5\xb8\xc0\xd3");
			CMouseCursor::Instance()->SetCamMode(false);
			// B733: 983 006964c0 case 0x95 manda RELEASE_REVIEW_CAMERA ao ShotReview entre SetCamMode e Camera
			SendMsg(this, "ShotReview", 0x2fd, (int)"RELEASE_REVIEW_CAMERA", 0, 0, 0);
			SendMsg(this, "Camera", M983(0x19b), 0, 0, 0, 0);
			SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a7), 0, 0, 0, 0); // 983 (0x95): sem teste de gameType 3
			GOLFDOC()->m_uiAlpha = 0x80;
			CChatMsg::Instance()->ReplaceBackupUiAlpha();
			// B1266: o bloco do 0x7a (sShotSyncData, golftask 0069f67c) fica em m_shotSync983 = 983 +0x45c; o 645 lia
			// GOLFDOC()+0x440.. (dentro do m_shotResultMsg). 983 006964c0 case 0x7a: 696f34..6970c3 = +0x464..+0x486.
			unsigned char* src = GOLFDOC()->m_shotSync983;
			*(unsigned long*)((char*)&ball + 0x34) =
				*(unsigned long*)(src + 0x08);
			*(unsigned long*)((char*)&ball + 0x38) =
				*(unsigned long*)(src + 0x0c);
			*(unsigned long*)((char*)&ball + 0x44) =
				*(unsigned long*)(src + 0x11);
			GOLFDOC_ULONG(0xe0) = *(unsigned long*)(src + 0x00);
			GOLFDOC_ULONG(0xe4) = *(unsigned long*)(src + 0x04);
			GOLFDOC_ULONG(0xec) = *(unsigned long*)(src + 0x04);
			GOLFDOC_ULONG(0xe8) = *(unsigned long*)(src + 0x04);
			GOLFDOC()->m_bar.grade = *(unsigned char*)(src + 0x10);
			GOLFDOC_UCHAR(0xf4) = // m_bar +0xe0..+0x11c cru neste bloco: com os membros (power/impact/gauge/gaugeStep/bFullPower/center, m_cameraData.angle) um mov do HandleMsg troca base/indice
				WisEqual(GOLFDOC()->m_bar.power, BAR_END, 0.00001f) != 0;
			GOLFDOC_ULONG(0xfc) = *(unsigned long*)(src + 0x21);
			GOLFDOC()->m_shotTime = *(unsigned long*)(src + 0x1d);
			unsigned long limit = Doc()->m_golfGame.shotTimeLimit;
			if (limit != 0)
			{
				if (GOLFDOC()->m_shotTime < limit)
					GOLFDOC()->m_remainTime = limit - GOLFDOC()->m_shotTime;
				else
					GOLFDOC()->m_remainTime = 0;
			}
			GOLFDOC_ULONG(0x11c) = *(unsigned long*)(src + 0x19);
			unsigned long frame = *(unsigned long*)(src + 0x15);
			CQuadTree& pvs = GetPVS();
			pvs.m_frame = frame;
			*(unsigned long*)&pvs.m_frameTime = 0;
			((CClubEx*)&GolfClub())->SetClub(*(unsigned char*)(src + 0x25));
			float power = *(float*)(src + 0x26);
			*(float*)((char*)&GolfClub() + 100) = power;
			*(unsigned long*)&m_shotExtra983 = *(unsigned long*)(src + 0x2a);
			{
				// B1266: 983 6970cf..6970fa manda (int)ball+0x34 e (int)ball+0x38 = m_initCurve/m_initSpin (era m_initSpecial)
				int spinY = (int)GolfBall().m_initCurve;
				int spinX = (int)GolfBall().m_initSpin;
				SendMsg(this, "Spin", M983(0x165), 0, spinY, spinX, 0);
			}
			SetAngle(GOLFDOC()->m_cameraData.angle);
			SendMsg(this, "Player", 0x2718, (int)&GOLFDOC()->m_cameraData.angle, 0, 0,
				0);
			if (GOLFDOC()->m_bNoStat != 0)
				SendMsg(this, "Screen", M983(0x1c9), 0, 0, 0, 0);
			WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
			m_targetPos = *(WVector*)&ball + WVector(rot.xz, rot.yz, rot.zz);
			GOLFDOC()->MakeDefaultCamera(true);
			ChangeGameMode(0x40);
			return;
		}
		case M983(0x7b):
		{
			if (GOLFDOC()->m_gameMode == 0x200000)
				return;
			if (GOLFDOC()->m_remoteGauge983.state == 1)
			{
				g_audio->StopSfx("\xc5\xb8\xc0\xd3");
				GOLFDOC()->m_shotPhase = 1;
				unsigned long mode = GOLFDOC()->m_gameMode;
				ChangeGameMode(0x20);
				GOLFDOC()->m_gameMode = mode;
				return;
			}
			if (GOLFDOC()->m_remoteGauge983.state != 2)
				return;
			if (GOLFDOC()->m_shotPhase == 1)
			{
				m_bRemoteHitPending = true;
				GOLFDOC_ULONG(0xe8) = GOLFDOC()->m_remoteGauge983.value;
				GOLFDOC_ULONG(0xec) = GOLFDOC()->m_remoteGauge983.value;
			}
			return;
		}
		case M983(0x7c):
		{
			WVector dir = m_targetPos - *(WVector*)&GolfBall();
			GOLFDOC()->m_cameraData.angle = CalcDeltaAngle(dir, WVector::UNIT_POS_Z);
			GOLFDOC()->MakeDefaultCamera(true);
			SetAngle(GOLFDOC()->m_cameraData.angle);
			SendMsg(this, "Player", 0x2718, (int)&GOLFDOC()->m_cameraData.angle, 0, 0,
				0);
			if (IsMassGame() == 0)
				m_nextTurn = Doc()->GetIndex(msg.param1);
			else
				m_nextTurn = 0;
			ChangeGameMode(0x4000);
			return;
		}
		case M983(0x7d):
			m_bNetMsgWait = false;
			m_pNetMsgRecv = "PM_NET_NEXT_TURN";
			m_netMsgRecvTick = GetTickCount();
			g_iRecvPacketGR = g_iRecvPacketType;
			if (IsMassGame() == 0)
				m_nextTurn = Doc()->GetIndex(msg.param1);
			else
				m_nextTurn = 0;
			ChangeGameMode(0x800);
			return; // 983 (case 0x98, B734): nao zera GolfClub+0x68 (so o NEXT_HOLE)
		case M983(0x7e):
			m_bNetMsgWait = false;
			m_pNetMsgRecv = "PM_NET_NEXT_HOLE";
			m_netMsgRecvTick = GetTickCount();
			g_iRecvPacketGR = g_iRecvPacketType;
			if (Doc()->m_golfGame.gameType == 7)
			{
				m_bSkinsNextHole = true;
				ChangeGameMode(0x1000);
				CalcSkinsHolePang();
			}
			else if (Doc()->m_golfGame.gameType == 10)
			{
				if (m_pApproachResultDlg == NULL)
				{
					Doc()->m_approachStartTime = 0;
					m_pApproachResultDlg = CreateForm<FrApproachResultDlg>(
						g_pFresh->GetManager(), this, "approach_result", NULL);
					m_pApproachResultDlg->EnableDrag(false);
					WRect rect = m_pApproachResultDlg->GetRect();
					WPoint pos;
					pos.x = g_view->GetWidth() * 0.5f - rect.w * 0.5f;
					pos.y = 35.0f;
					m_pApproachResultDlg->Open(
						(FRESH_PFN_RESULT)&CGolfRule::OnApproachDlgResult, pos,
						0);
					m_pApproachResultDlg->SetTimeLimit(8.0f);
					rect.w = rect.w + 30.0f; // 983 (00697882/00697e24: fadd qword [00cfe8e0] = 30.0; o 645 somava 120)
					m_pApproachResultDlg->SetRect(rect);
					CProjectG::Instance()->SetGameSpeed(1.0f);
				}
			}
			else
				ChangeGameMode(0x2000);
			((CGolfDocEx*)GOLFDOC())->ClearShotResultMsg();
			*((unsigned char*)&GolfClub() + 0x68) = 0;
			return;
		case M983(0x80):
		{
			if (GOLFDOC()->m_bNetGameEnd == 0)
			{
				GOLFDOC()->m_bNetGameEnd = 1;
				GOLFDOC()->RefreshPlayerNum();
				for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; i++)
				{
					if (*(int*)PLAYER(i) != 3)
					{
						unsigned long off = i * 0x21;
						unsigned char* row = (unsigned char*)GDOCPTR983(GOLFDOC(), 0x340) - 0x340 + off; // B1475: base segue o membro (cl novo). A717: row + 0x340 = m_holeResult983[i] (sHoleResult983; +4 rankBits, +5 score, +6 byte6, +9 totalPang, +0x11 bonusPang, +0x19/+0x1d skins)
						unsigned char& bits =
							*((unsigned char*)PLAYER(i) + 0x4f0);
						bits = (unsigned char)(row[0x344] << 4 | (bits & 0xf));
						char score = row[0x345];
						PLAYER(i)->score = (int)score;
						PLAYER(i)->totalStroke = row[0x346];
						PLAYER(i)->totalPang =
							*(unsigned long*)(row + 0x349);
						PLAYER(i)->bonusPang =
							*(unsigned long*)(row + 0x351); // 983: m_holeResult983 em +0x340 (s94)
						if (Doc()->m_golfGame.gameType == 1)
						{
							TEAM(PLAYER(i)->team)
								->totalPang = PLAYER(i)->totalPang;
							TEAM(PLAYER(i)->team)
								->bonusPang = PLAYER(i)->bonusPang;
						}
						else if (Doc()->m_golfGame.gameType == 7)
						{
							const sHoleResult983* hr = GOLFDOC()->m_holeResult983; // B1561: ex-cru doc (B1475)
							sPlayerData* p = PLAYER(i);
							*(unsigned long*)((char*)p + 0xd90) =
								*(const unsigned long*)((const char*)&hr->skins19 + off); // B1561: ex-cru doc + 0x359 + off (off = i * 0x21)
							*(unsigned long*)((char*)p + 0xd94) =
								*(const unsigned long*)((const char*)&hr->skins1d + off); // B1561: ex-cru doc + 0x35d + off (off = i * 0x21)
						}
					}
				}
				m_bNetMsgWait = false;
				m_pNetMsgRecv = "PM_NET_GAME_END";
				m_netMsgRecvTick = GetTickCount();
				g_iRecvPacketGR = g_iRecvPacketType;
				g_view->SetFOV(0.9599311f);
				SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
				SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
				SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
				SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
				SendMsg(this, "Trajectory", M983(0x160), 0, 0, 0, 0);
				CFx::Instance()->ClearAll(true);
				if (Doc()->m_golfGame.gameType == 7)
				{
					if ((unsigned long)GOLFDOC()->m_skinsPenaltyUid == Doc()->m_myInfo.info.dwGuid)
					{
						sPlayerData* p = PLAYER(MyIndex());
						__int64& pang = *(__int64*)((char*)p + 0xd90);
						if (pang > 0)
						{
							p = PLAYER(MyIndex());
							int delta =
								(int)((float)*(__int64*)((char*)p + 0xd90) *
									0.05f);
							p = PLAYER(MyIndex());
							*(__int64*)((char*)p + 0xd90) -= delta;
							CChatMsg::Instance()->AddChatMsg(
								std::string(MakeStr(
									K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xc6\xce\xb9\xe8\xc6\xb2 \xc8\xb9\xb5\xe6\xc6\xce\xbf\xa1\xbc\xad 5%%(%I64d\xc6\xce)\xb8\xa6 \xb1\xe2\xba\xce \xc7\xd5\xb4\xcf\xb4\xd9."),
									(__int64)delta)),
								0xffff7878, false, false);
						}
					}
					CalcSkinsHolePang();
				}
				ChangeGameMode(0x2000);
			}
			if (m_pGoStopDlg)
				m_pGoStopDlg->Close((eFormRet)0, true);
			if (m_pCancelDropDlg)
				m_pCancelDropDlg->Close((eFormRet)2, true);
			if (CTHunter::Instance())
				CTHunter::Instance()->SetReturnLobby(true);
			SHAREDDOC_UCHAR(0x10488) = 1;
			return;
		}
		case M983(0x81):
		{
			m_bNetMsgWait = false;
			m_pNetMsgRecv = "PM_NET_GAMESYNC_START";
			m_netMsgRecvTick = GetTickCount();
			g_iRecvPacketGR = g_iRecvPacketType;
			GOLFDOC()->m_currentPlayer = (unsigned char)msg.param1;
			SendMsg(this, "Score", M983(0x130), 0, 0, 0, 0);
			((CGolfDocEx*)GOLFDOC())->InitShotResultMsg();
			unsigned long mode;
			if (OnlinePlay() == 1 &&
				((GOLFDOC()->m_bNoStat == 0 &&
					 GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0]) ||
					Doc()->m_golfGame.gameType == 10))
				mode = 2;
			else
				mode = 1;
			ChangeGameMode(mode);
			// 983 00697534: sem IsLocalContent na frente
			CSharedDoc* shared = Doc();
			if (shared->IsMapEventActive(GetCurMap()) &&
				Doc()->m_holeOrder[0] == GOLFDOC()->m_currentHole)
			{
				std::string notice(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xb8\xca\xc0\xcc\xba\xa5\xc6\xae\xb0\xa1 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.\xb0\xd4\xc0\xd3\xbf\xcf\xc1\xd6\xbd\xc3 \xba\xb8\xb3\xca\xbd\xba\xb0\xa1 \xc1\xd6\xbe\xee\xc1\xfd\xb4\xcf\xb4\xd9."));
				CChatMsg::Instance()->AddChatMsg(notice, 0xffff7878, true,
					false);
			}
			CShuffleBonus::Instance()->ShowNoticeMsg();
			SendMsg(this, "TutorGuide", 0x311, 0, 0, 0, 0); // B735: 983 006964c0 case 0x9c termina com SendMsg TutorGuide 0x311
			return;
		}
		case M983(0x82):
		{
			if (GOLFDOC()->m_bNetGameEnd != 0)
				return;
			unsigned char who = Doc()->GetIndex(msg.param1);
			*(int*)PLAYER(who) = 3;
			if (Doc()->m_golfGame.gameType == 1)
			{
				bool allOut = true;
				who = Doc()->GetIndex(msg.param1);
				unsigned char team = *((unsigned char*)PLAYER(who) + 0x4e9);
				int i = 0;
				if (GOLFDOC()->m_playerNum != 0)
				{
					do
					{
						if (team ==
								*((unsigned char*)PLAYER((unsigned char)i) +
									0x4e9) &&
							*(int*)PLAYER((unsigned char)i) != 3)
							allOut = false;
						i++;
					} while (i < (int)GOLFDOC()->m_playerNum);
				}
				if (!allOut)
					goto refresh;
				*(int*)TEAM(team) = 3;
			}
refresh:
			GOLFDOC()->RefreshPlayerNum();
			unsigned char num = GOLFDOC()->GetCurrentPlayerNum();
			unsigned char idx = Doc()->GetIndex(msg.param1);
			unsigned char* bits = (unsigned char*)PLAYER(idx) + 0x4f0;
			*bits = (unsigned char)(*bits ^ ((num + 1) ^ *bits) & 0xf);
			idx = Doc()->GetIndex(msg.param1);
			*((unsigned char*)PLAYER(idx) + 0x4f0) =
				(unsigned char)(*bits << 4 |
					(*((unsigned char*)PLAYER(idx) + 0x4f0) & 0xf));
			ResetRank();
			if (Doc()->m_golfGame.gameType != 0)
			{
				if (Doc()->m_golfGame.gameType == 7)
				{
					unsigned char who2 = Doc()->GetIndex(msg.param1);
					unsigned char hole;
					if (GOLFDOC()->m_bNoStat == 0)
						hole = GOLFDOC()->m_currentHole;
					else
						hole =
							Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1];
					unsigned long cur = hole;
					unsigned long lo =
						*(unsigned long*)(USERINFO983(who2, 0x3a2e));
					int hi = *(int*)(USERINFO983(who2, 0x3a32));
					unsigned long baseLo = ((unsigned long*)&GOLFDOC()->m_skinsHolePot)[0];
					int baseHi = ((unsigned long*)&GOLFDOC()->m_skinsHolePot)[1];
					if (hi <= baseHi && (hi < baseHi || lo < baseLo))
					{
						baseLo = lo;
						baseHi = hi;
					}
					*(WCrypticValue<int>*)((char*)PLAYER(who2) +
						(cur + 0x37) * 0x10 + 0xc) -= baseLo; // 983: skinsPang[] +0x38c
					__int64& pang = *(__int64*)((char*)PLAYER(who2) + 0xd98);
					pang -= (__int64)baseLo | ((__int64)baseHi << 32);
					if (pang > 0)
						pang = 0;
					*(__int64*)((char*)PLAYER(who2) + 0xd90) = pang;
					__int64& total = *(__int64*)GDOCPTR983(GOLFDOC(), 0x1d0);
					total += (__int64)baseLo | ((__int64)baseHi << 32);
					sPlayerData* p = PLAYER(who2);
					unsigned char* row = (unsigned char*)p + 0x1c + cur * 0x10; // 983 (0069832x): registro do buraco em 0x1c + hole*0x10 (o 645 usava (hole+1)*0x10)
					row[0] = row[0xe];
					row[1] = (unsigned char)(row[0xe] ^ (unsigned char)(unsigned long)row); // 983 0069853a: verificador = valor ^ byte baixo do endereco
				}
				return;
			}
			GOLFDOC()->m_maxGrade = 0;
			int i = 0;
			if (GOLFDOC()->m_playerNum != 0)
			{
				int off = 0;
				do
				{
					if (*(int*)PLAYER((unsigned char)i) != 3)
					{
						unsigned char level;
						unsigned char stat = *((unsigned char*)&Doc()->m_userInfo[0] + 0x17d + off);
						if (stat < 6)
							level = 1;
						else
							level = (unsigned char)((stat - 1) / 5 + 1);
						if (GOLFDOC()->m_maxGrade < level)
							GOLFDOC()->m_maxGrade = level;
					}
					i++;
					off += 0x31c6;
				} while (i < (int)GOLFDOC()->m_playerNum);
			}
			return;
		}
		case M983(0x83):
			if (m_bShotInFlight != 0)
				Skip();
			return;
		case M983(0x84):
			if (msg.param1 == 0)
			{
				GOLFDOC()->m_bPause = 0;
				if (m_pPauseDlg)
					m_pPauseDlg->Close((eFormRet)0, true);
				return;
			}
			if (m_pPauseDlg == NULL)
			{
				m_pPauseDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				m_pPauseDlg->SetMessage((const char*)msg.param2, false);
				m_pPauseDlg->Open((FRESH_PFN_RESULT)&CGolfRule::PauseDlgResult,
					0);
				m_pPauseDlg->EnableDrag(false);
				GOLFDOC()->m_bPause = 1;
				MOUSECURSOR()->m_bArea[0x2a] = false;
			}
			return;
		case M983(0x85):
			QuickSave((char*)msg.param1);
			return;
		case M983(0x86):
			QuickLoad((char*)msg.param1);
			return;
		case M983(0x87):
		{
			int kind = msg.param1;
			if (kind == 0)
				m_bTutorialShot983 = 0;
			else if (kind == 1 || kind == 2)
			{
				m_bTutorialShot983 = 1;
				g_input->SetFakeInput(
					"\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", false);
				m_tutorialGauge983[0] = (float)msg.param2;
				m_tutorialGauge983[1] = (float)msg.param3;
			}
			IActor* tutorial = GetActor("Tutorial");
			if (m_bTutorialShot983 == 0)
			{
				if (tutorial)
					tutorial << MsgObject(NULL, M983(0x20f), 1, 0, 0, 0, 0);
				return;
			}
			if (tutorial)
				tutorial << MsgObject(NULL, M983(0x20f), 0, 0, 0, 0, 0);
			return;
		}
		case M983(0x88):
			m_bContinueDlg = msg.param1 != 0;
			return;
		case M983(0x89):
		{
			*(int*)PLAYER(0) = 0;
			WPolySoupEx2* soup = *(WPolySoupEx2**)((char*)GOLFDOC() + 8);
			WPolySoupEx2::sPoint* point = soup->FindPoint((char*)msg.param1);
			float radius = *(float*)((char*)&GolfBall() + 200);
			WVector at = point->pos;
			WVector ground = GetPVS().GetGroundPoint(at, false, NULL, false);
			WVector pos(ground.x, radius + ground.y, ground.z);
			GolfBall().m_pos = pos;
			GolfBall().m_meshIndex =
				GetPVS().GetMeshIndex(GolfBall().m_pos, false);
			GolfBall().m_groundType =
				GetPVS().GetGroundType(GolfBall().m_pos, true, NULL);
			PLAYER(GOLFDOC()->m_currentPlayer)->pos = GolfBall().m_pos;
			if ((char*)msg.param2 == NULL)
				m_targetPos =
					(*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))->GetPoint(NULL);
			else
				m_targetPos = soup->FindPoint((char*)msg.param2)->pos;
			ChangeTurn();
			SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
			SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
			SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
			CFx::Instance()->ClearAll(true);
			if ((unsigned long)GOLFDOC()->m_tutorialMode < 0xf && GOLFDOC()->m_tutorialMode != 0xb)
			{
				WINDEX().SetIntensity(0.0f);
				WINDEX().SetDirection(0.0f);
			}
			(*(WPolySoup**)((char*)GOLFDOC() + 8))->UpdateBgFxWind();
			SendMsg(this, "GolfRule", M983(0x88), 0, 0, 0, 0);
			SetEnvironmentVariableA();
			GOLFDOC()->MakeDefaultCamera(true);
			ChangeGameMode(8);
			return;
		}
		case M983(0x8a):
		{
			// 983 00696980: posicao/tamanho do medidor pelas medidas nomeadas da interface (nao pelo bar do 645)
			CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
				(float)(g_view->GetHeight() / 480.0));
			CUiFrameSetManager* ui = CUiFrameSetManager::Instance();
			CUiFrameSetManager::NamedDim* dimX = ui->FindNamedDim("d_default_pangya_x");
			CUiFrameSetManager::NamedDim* dimY = ui->FindNamedDim("d_driver_gauge_y");
			if (dimX != NULL && dimY != NULL)
			{
				float* out = (float*)msg.param1;
				float base = ScaleByViewHeight983(g_view, *(float*)GDOCPTR983(GOLFDOC(), 0xec) - 140.0f);
				out[0] = (float)((double)dimX->Calc(scl) + (double)base);
				out[1] = (float)dimY->Calc(scl);
			}
			return;
		}
		case M983(0x8b):
			if (GOLFDOC()->m_bar.grade < 3)
			{
				*(float*)msg.param1 = 0.01f;
				return;
			}
			*(float*)msg.param1 =
				(GOLFDOC()->m_bar.power - BAR_START) / g_fBarRange;
			return;
		case M983(0x8c):
			m_captureTiles = msg.param1;
			return;
		}
	}
	// S68: era "else if (id < 0xb2)" (arvore do switch do 645); os blocos sao disjuntos e a faixa nao vale com M983
	{
		if (id == M983(0xb1))
		{
			unsigned long* src = (unsigned long*)msg.param1;
			if (src != NULL)
			{
				m_bClosedBonusBallPending = true;
				memcpy(m_closedBonusBall, src, 100);
			}
			return;
		}
		switch (id)
		{
		case 0xc5:
			// A469: 983 006964c0 caso 0xc5 (sem par 645): o CGolfTask o manda no pacote 0x1f3 (golftask.cpp, 983 006a4329);
			// o nosso nao tinha o caso e a mensagem se perdia. = fim do 0x9b sem o placar: fecha GoStop/CancelDrop e volta ao lobby
			if (GOLFDOC()->m_bNetGameEnd == 0)
			{
				GOLFDOC()->m_bNetGameEnd = 1;
				GOLFDOC()->RefreshPlayerNum();
				SetNetMsgWait(false, 0x172b, "PM_NET_GAME_END", 20.0f);
				g_view->SetFOV(0.9599311f);
				SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
				SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
				SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
				SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
				SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
				SendMsg(this, "Trajectory", M983(0x160), 0, 0, 0, 0);
				CFx::Instance()->ClearAll(true);
			}
			if (m_pGoStopDlg)
				m_pGoStopDlg->Close((eFormRet)0, true);
			if (m_pCancelDropDlg)
				m_pCancelDropDlg->Close((eFormRet)2, true);
			if (CTHunter::Instance())
				CTHunter::Instance()->SetReturnLobby(true);
			SHAREDDOC_UCHAR(0x10488) = 1;
			if (OnlinePlay())
			{
				CProjectG::Instance()->FromGolfToLobbySettings();
				CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
				PostMsg(this, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
				unsigned char realType = Doc()->m_roomInfo.realGameType;
				PostMsg(NULL, "Lobby", 0x8b, (realType == 0xc || realType == 0xe) ? 0xb : 9, 0, 0, 0);
				PostMsg(this, "Lobby", M983(0x1a), 0, 0, 0, 0);
				if (CSceneManager::Instance())
					SendMsg(this, "GolfBg", M983(0x142), 0, 0, 0, 0);
			}
			return;
		case 0xc8:
			// 983 00699513 (sem par 645): o CGolfTask posta 0xc8 no pacote 0x259 com resultado 0 = desistencia do jogador da vez
			SendMsg(this, "GuideLine", M983(0x1a2), 0, 0, 0, 0);
			RecordGiveUp(GOLFDOC()->m_currentPlayer);
			RecordScoreByPlayer(GOLFDOC()->m_currentPlayer);
			ChangeGameMode(0x80000);
			AfxGetTask()->GetActor("GolfRule") << MsgObject(NULL, 0xc9, 0, 0, 0, 0, 0);
			return;
		case 0xc9:
		{
			// 983 (caso 0xc9 do 006964c0): cada rival ativo (+0x36 != 1) avisa o Score (0x171/0x177); o AIPlayer (0x34d) so com +0x1b4
			C_ASSERT_VC10(offsetof(CSharedDoc, m_rivalList) == 0x1042c && sizeof(sRivalData) == 0x1b8);
			C_ASSERT_VC10(offsetof(sRivalData, state) == 0x36 && offsetof(sRivalData, bGrandPrixAI983) == 0x1b4);
			int r = 0;
			for (RIVAL_LIST_IT it = Doc()->m_rivalList.begin();
				it != Doc()->m_rivalList.end(); ++it, r++)
			{
				if (it->state == 1)
					continue;
				if (it->bGrandPrixAI983 != 0) // 983 0xc9: so o AIPlayer depende do +0x1b4; o Score vale para todo rival ativo
					AfxGetTask()->GetActor("AIPlayer") << MsgObject(NULL, 0x34d, r & 0xffff, 0, 0, 0, 0);
				AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x171, r & 0xffff, 0, 0, 0, 0);
				AfxGetTask()->GetActor("Score") << MsgObject(NULL, 0x177, r & 0xffff, 0, 0, 0, 0);
			}
			return;
		}
		case M983(0x8e):
			Warp(*(WVector*)msg.param1);
			return;
		case M983(0x8f):
			// 983 (00698bcf): tutorial avançado = 0x10 (era 0xc no 645); sem o 0xf do tipo 3 do 645; aviso por K2L
			if ((unsigned long)GOLFDOC()->m_tutorialMode < 0xf)
			{
				if (msg.param1 == 0 && Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
				{
					WSendPacket packet((enumClientPacket)0x2e);
					packet.Send(TO_GAME);
				}
			}
			else
			{
				if (msg.param1 != 1)
				{
					if (msg.param1 == 2)
					{
						FrForm* form = CreateForm<FrForm>(
							g_pFresh->GetManager(), this, "notify", NULL);
						form->SetMessage(K2L_Compatibility(
							"\xbb\xf3\xb4\xeb\xb0\xa1 \xb0\xd4\xc0\xd3\xbf\xa1\xbc\xad \xb3\xaa\xb0\xac\xbd\xc0\xb4\xcf\xb4\xd9"),
							false);
						form->Open(
							(FRESH_PFN_RESULT)&CGolfRule::OnReturnLobbyResult,
							3);
					}
					return;
				}
			}
			ReturnLobby(false);
			return;
		case M983(0x90):
		{
			unsigned long* flags = (unsigned long*)USERINFO983((unsigned char)msg.param1, 0x3922);
			if ((*flags & 1) == 0)
			{
				*flags |= 1;
				SendMsg(this, "Letter", M983(0x18d), (int)"AI", 0, 0, 0);
				ChangeGameMode(8);
				SetEnvironmentVariableA();
				return;
			}
			*flags &= 0xfffffffe;
			SendMsg(this, "Letter", M983(0x18c), (int)"AI", 0, 0, 0);
			return;
		}
		case M983(0x91):
			m_pendingInitSpecial |= msg.param1;
			return;
		case M983(0x92):
			memcpy(GRPTR983(0x320), DOCPTR983(0x3b46), 0x28);
			return;
		case M983(0x93):
			SendShotResult();
			return;
		case M983(0x94):
			m_bNetMsgWait = false;
			m_pNetMsgRecv = "PM_NET_TEESHOT_READY_ACK";
			m_netMsgRecvTick = GetTickCount();
			g_iRecvPacketGR = g_iRecvPacketType;
			GOLFDOC()->m_bApproach795 = 0;
			return;
		case M983(0x95):
		{
			SetNetMsgWait(false, 0x1826, "PM_NET_ASK_GOSTOP", 0.0f); // 983 0066e680(0,0x1826,..)
			GOLFDOC()->m_bStopDlg = 1;
			if (m_pGoStopDlg == NULL)
			{
				*(unsigned long*)&m_goStopTime983 = 0;
				m_pGoStopDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify_yesno", NULL);
				m_pGoStopDlg->Open(
					(FRESH_PFN_RESULT)&CGolfRule::GoStopDlgResult, 0); // 983 0066e680 b0: flag 0 (0xffffffe8 = ajuste de this do ponteiro-membro)
				m_pGoStopDlg->EnableDrag(false);
			}
			int exp;
			SendMsg(this, "Score", M983(0x13a), (int)&exp, 0, 0, 0);
			sPlayerData* p = PLAYER(MyIndex());
			char text[1024];
			sprintf(text,
				K2L_Compatibility("\xbb\xf3\xb4\xeb\xb0\xa1 \xc5\xf0\xc0\xe5\xc7\xd8\xbc\xad \xb4\xf5 \xc0\xcc\xbb\xf3\xc0\xc7 \xb4\xeb\xc0\xfc\xc0\xba \xba\xd2\xb0\xa1\xb4\xc9\xc7\xcf\xb3\xaa, \xb0\xb3\xc0\xce \xb1\xe2\xb7\xcf \xbc\xf6\xb8\xb3\xc0\xbb \xc0\xa7\xc7\xd8\xbc\xad \xc7\xf6\xc0\xe7\xb1\xee\xc1\xf6 \xbe\xf2\xc0\xba\n\\c0xffff0000\\c\xb0\xe6\xc7\xe8\xc4\xa1 %d\\c0xff000000\\c\xbf\xcd \\c0xffff0000\\c%d \xc6\xce\\c0xff000000\\c\xb8\xb8\xc0\xbb \xb9\xde\xb0\xed \xb0\xd4\xc0\xd3\xc0\xbb \xb0\xe8\xbc\xd3 \xc1\xf8\xc7\xe0 \xc7\xd2 \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\xb1\xd7\xb8\xae\xb0\xed, \xb0\xd4\xc0\xd3\xc0\xbb \xbf\xcf\xc1\xd6\xc7\xcf\xb8\xe9 \xc5\xac\xb8\xae\xbe\xee \xba\xb8\xb3\xca\xbd\xba \xc6\xce\xc0\xbb \xb9\xde\xc0\xbb \xbc\xf6 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n\xb0\xe8\xbc\xd3 \xc1\xf8\xc7\xe0 \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
				exp,
				~(*(unsigned int*)((char*)p + 0x500) ^
					*(unsigned int*)((char*)p + 0x4f8)));
			m_pGoStopDlg->SetMessage(text, false);
			m_pGoStopDlg->SetTimeLimit(10.0f);
			return;
		}
		case M983(0x96):
		{
			if (m_pChatView == NULL)
				return;
			int lines = CHATVIEW_LINES();
			int i = 0;
			if (0 < lines)
			{
				while (true)
				{
					int next = i + 1;
					const char* text = m_pChatView->GetLine(next, false);
					if (text != NULL && *text != '\0')
						break;
					i = next;
					if (lines <= next)
						return;
				}
				m_pChatView->SetLine(i + 1, "", 0, false, 0);
			}
			return;
		}
		case M983(0x97):
			if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			{
				Replay(false);
				CHelpMsg::Instance()->CheckReplayAvailable();
				return;
			}
			if (Doc()->m_replayPlayType != 2)
				CHelpMsg::Instance()->CheckReplayAvailable();
			return;
		case M983(0x98):
		{
			if (m_pChatView == NULL)
				return;
			int extra = (-(int)option->gIsChatLineEntended() & 5) + 5;
			WPoint pos;
			pos.x = m_pChatView->GetRect().x;
			// 983 0xb3: base y do chat = d_chat_output_base_y (00684cb0); sem a medida, BarY - 100
			float baseY = option->gGetBar_Y() - 100.0f;
			CUiFrameSetManager::NamedDim* pBase =
				CUiFrameSetManager::Instance()->FindNamedDim("d_chat_output_base_y");
			if (pBase)
				baseY = pBase->Calc(
					CUiFrameSetManager::Size(g_view->GetWidth() / 640.0, g_view->GetHeight() / 480.0));
			pos.y = baseY - ((float)extra * 19.0f + 8.0f);
			m_pChatView->MoveWindow(pos);
			m_pChatView->GetScrollBar()->Resize(extra, 1, false); // A175: +0x90 no 983 (era o +0x7c do 645)
			int n = 5;
			if (!option->gIsChatLineEntended())
			{
				int i = 6;
				do
				{
					const char* text = m_pChatView->GetLine(i, false);
					if (text != NULL)
						m_pChatView->SetLine(i - 5, text, 0, false, 0);
					m_pChatView->SetLine(i, "", 0, false, 0);
					i++;
					n--;
				} while (n != 0);
				return;
			}
			int i = 1;
			do
			{
				const char* text = m_pChatView->GetLine(i, false);
				if (text != NULL)
					m_pChatView->SetLine(i + 5, text, 0, false, 0);
				m_pChatView->SetLine(i, "", 0, false, 0);
				i++;
				n--;
			} while (n != 0);
			return;
		}
		case M983(0x99):
		{
			*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
			GOLFDOC()->m_bTeeShotReady = 0;
			float radius = *(float*)((char*)&GolfBall() + 200);
			WVector tee = GetTeeShotPos();
			WVector pos(tee.x, radius + tee.y, tee.z);
			GolfBall().Reset(pos);
			GolfBall().m_meshIndex =
				GetPVS().GetMeshIndex(GolfBall().m_pos, false);
			ResetData(GolfBall().m_pos);
			return;
		}
		case M983(0x9a):
			m_bPuttTutorial983 = 1;
			return;
		case M983(0x9b):
			m_dbgCam[0] = (float)msg.param1;
			return;
		case M983(0x9c):
			m_bNetMsgWait = false;
			m_pNetMsgRecv = "PM_NET_CANCEL_SHOT";
			m_netMsgRecvTick = GetTickCount();
			g_iRecvPacketGR = g_iRecvPacketType;
			Cancel();
			return;
		case M983(0x9d):
			m_dbgCam[1] = (float)msg.param1;
			return;
		case M983(0x9e):
		{
			int on = msg.param1;
			m_bServerHold983 = on == 1;
			if (on == 1)
				m_serverHoldTime983 = 0.0f;
			return;
		}
		case M983(0x9f):
			if (msg.param1 == (int)Doc()->m_myInfo.info.dwGuid)
				ChangeGameMode(0x2000);
			return;
		case M983(0xa2):
			if (m_bGaugePaused != 1)
			{
				m_bGaugePaused = 1;
				return;
			}
			m_bGaugePaused = 0;
			m_gaugeTick = m_gaugePauseTick;
			return;
		case M983(0xa3):
			GOLFDOC()->m_bTutorialGaugeMute = GOLFDOC()->m_bTutorialGaugeMute == 0;
			return;
		case M983(0xa4):
			if (GOLFDOC()->m_gameMode == 0x80)
			{
				BALL_ULONG(0x18) = 0;
				return;
			}
			BALL_ULONG(0x18) = 0;
			GOLFDOC()->m_gameMode = 0x80;
			if (Doc()->m_replayPlayType == 1)
			{
				*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x508) = 0;
				*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x4f8) = 0;
			}
			return;
		case M983(0xa6):
			if (Doc()->m_golfGame.gameType != 10)
				return;
			if (m_pApproachResultDlg == NULL)
			{
				Doc()->m_approachStartTime = 0;
				m_pApproachResultDlg = CreateForm<FrApproachResultDlg>(
					g_pFresh->GetManager(), this, "approach_result", NULL);
				m_pApproachResultDlg->SetApproachEnd(true);
				m_pApproachResultDlg->EnableDrag(false);
				WRect rect = m_pApproachResultDlg->GetRect();
				WPoint pos;
				pos.x = g_view->GetWidth() * 0.5f - rect.w * 0.5f;
				pos.y = 35.0f;
				m_pApproachResultDlg->Open(
					(FRESH_PFN_RESULT)&CGolfRule::OnApproachDlgResult, pos, 0);
				m_pApproachResultDlg->SetTimeLimit(8.0f);
				rect.w = rect.w + 30.0f; // 983 (00697882/00697e24: fadd qword [00cfe8e0] = 30.0; o 645 somava 120)
				m_pApproachResultDlg->SetRect(rect);
				CProjectG::Instance()->SetGameSpeed(1.0f);
			}
			return;
		case M983(0xa7):
			ForcedMoveBall(*(WVector*)msg.param1);
			return;
		case M983(0xa9):
		case M983(0xaa):
		case M983(0xac):
			// 983 (006994f5): botao do mensageiro do golfe (+0x13c) pelo auxiliar do taskmain (00657eb0); nao cai no HandleMsgCommon
			UpdateMessengerBtn983(m_pMessengerBtn, id, msg.param1 != 0); // B1488: era GR_WND(0x13c) cru (cl 14.44 anda -4: BaseObject)
			return;
		}
	}
	// S68: era "else if (id < 0x244)" (arvore do switch do 645); os blocos sao disjuntos e a faixa nao vale com M983
	{
		if (id == M983(0x243))
		{
			HoleComboGaugeSetting();
			return;
		}
		switch (id)
		{
		case M983(0x1f9):
		{
			if (CQuitWindow::Instance()->IsActive())
				return;
			bool show = !CProjectG::Instance()->HideGUI() && m_bGameUI;
			SendMsg(this, "Screen", M983(0x190), show, 0, 0, 0);
			SendMsg(this, "Letter", M983(0x190), show, 0, 0, 0);
			if (g_pFresh != NULL)
				g_pFresh->show = show;
			// 983 (00696555 case 0x273): o mesmo valor vai ao +0x40 do ponteiro da interface (00e73e4c)
			if (CUiPointer983::Instance983())
				CUiPointer983::Instance983()->m_show40 = show;
			show = !CProjectG::Instance()->HidePI() && m_bGameUI;
			CChatMsg::Instance()->Toggle(show);
			if (!CProjectG::Instance()->HideGUI() &&
				!CProjectG::Instance()->HidePI() && m_bGameUI)
			{
				CMouseCursor::Instance()->Toggle(true);
				return;
			}
			CMouseCursor::Instance()->Toggle(false);
			return;
		}
		case M983(0x206):
			if (!IsLocalContent((localContentType_t)0x39 /*983: era 0x38; medido*/))
				return;
			((CClubEx*)&GolfClub())->SetClub((unsigned char)msg.param1);
			return;
		case M983(0x20d):
			if (!IsLocalContent((localContentType_t)0x39 /*983: era 0x38; medido*/))
				return;
			ToggleCommandBtns(true);
			return;
		case M983(0x21e):
			if (!IsLocalContent((localContentType_t)0x59 /*983: era 0x58; interpolado*/))
				return;
			m_modeTime = 0;
			return;
		}
	}
	// S68: era "else if (id < 0x271b)" (arvore do switch do 645); os blocos sao disjuntos e a faixa nao vale com M983
	{
		if (id == 0x271a)
		{
			OnDestroy();
			return;
		}
		if (id == M983(0x24a))
		{
			FrWnd* found =
				g_pFresh->GetManager()->GetDesktop()->FindChildByName(
					(char*)msg.param1);
			// 983 (006964c0 case 0x2c6): sem achar, grava 0 em param2 (o 645 deixava o valor antigo)
			*(FrWnd**)msg.param2 = found;
			return;
		}
		if (id == M983(0x24d))
		{
			bool send = false;
			if (msg.param1 == 1)
				send = true;
			SetNetMsgWait(send, 0x1971, (char*)msg.param2, 20.0f);
			return;
		}
	}
	if (id == 0x271b)
	{
		// B694: 983 006964c0 case 0x271b (mudanca de resolucao): posicoes pela medida nomeada do chat (009a0930), larguras 983
		if (CChatMsg::m_pInstance)
			CChatMsg::m_pInstance->ResetUiPosition();
		float gw = g_view->GetWidth();
		float gh = g_view->GetHeight();
		if (m_pChatView)
		{
			const bool bigChat = option->gIsChatLineEntended() ||
				(*((unsigned char*)DOCPTR983(0x1094c)) != 0xb && *((unsigned char*)DOCPTR983(0x1095e)) != 0 &&
					*((unsigned char*)DOCPTR983(0x10937)) > 99);
			float baseY = option->gGetBar_Y() - 100.0f; // 00684cb0: d_chat_output_base_y
			CUiFrameSetManager::NamedDim* pBase =
				CUiFrameSetManager::Instance()->FindNamedDim("d_chat_output_base_y");
			if (pBase)
				baseY = pBase->Calc(CUiFrameSetManager::Size(gw / 640.0, gh / 480.0));
			WPoint pos;
			pos.x = m_pChatView->GetRect().x;
			pos.y = (float)(baseY - ((double)(bigChat ? 10 : 5) * 19.0 + 8.0));
			m_pChatView->MoveWindow(pos);
		}
		// 009a0930: ponto p_chat_window_area (arredondado para baixo); sem ele x = 91 e y = BarY + 9
		float cx = 91.0f;
		float cy = option->gGetBar_Y() + 9.0f;
		CUiFrameSetManager::NamedPoint* pChat =
			CUiFrameSetManager::Instance()->FindNamedPoint("p_chat_window_area");
		if (pChat)
		{
			WPoint cp;
			pChat->Calc(CUiFrameSetManager::Size(gw / 640.0, gh / 480.0), cp);
			cx = (float)floor(cp.x);
			cy = (float)floor(cp.y);
		}
		struct sChatMove
		{
			FrWnd* wnd;
			double dx;
			double dy;
		};
		sChatMove chat[5] = { { m_pChatTarget, 21.0, 5.0 }, { m_pChatEdit, 139.0, 5.0 }, { m_pEmoticonBtn, 381.0, 7.0 },
			{ m_pLanguage, 401.0, 7.0 }, { m_pReportBtn, 420.0, 7.0 } };
		for (int i = 0; i < 5; i++)
		{
			if (chat[i].wnd)
			{
				WPoint pos;
				pos.x = (float)(cx + chat[i].dx);
				pos.y = (float)(cy + chat[i].dy);
				chat[i].wnd->MoveWindow(pos);
			}
		}
		// botoes do topo/direita: x = largura - n, y mantido (983 0069a049..0069a28e; offsets reais do 983)
		static const struct
		{
			unsigned long off;
			double sub;
		} s_right[] = { { 0x48, 106.0 }, { 0x4fc, 54.0 }, { 0x13c, 81.0 }, { 0x128, 167.0 }, { 0x500, 29.0 },
			{ 0x504, 423.0 }, { 0x508, 398.0 }, { 0x50c, 373.0 }, { 0x510, 348.0 } };
		for (int i = 0; i < 9; i++)
		{
			FrWnd* wnd = GR_WND(s_right[i].off);
			if (wnd)
			{
				WPoint pos;
				pos.x = (float)(gw - s_right[i].sub);
				pos.y = (s_right[i].off == 0x500 && m_pMyInfoBtn) ? m_pMyInfoBtn->GetRect().y : wnd->GetRect().y;
				wnd->MoveWindow(pos);
				if (s_right[i].off == 0x13c)
					CMessengerInfo::Instance()->m_alarmRect = wnd->GetRect(); // +0x68 (0069a0e3)
			}
		}
		FrWnd* moveTee = m_pMoveTeeBtn;
		if (moveTee)
		{
			WPoint pos;
			pos.x = (float)(gw * 0.5 + 90.0);
			pos.y = gh - moveTee->GetRect().h;
			moveTee->MoveWindow(pos);
		}
		SendMsg(this, "ShotReview", 0x2fd, (int)"RESET_UI", 0, 0, 0);
		SendMsg(this, "ShotPreview", 0x2fd, (int)"RESET_UI", 0, 0, 0);
		ResetUiPos_Board();
		return;
	}
	// B575: mensagens novas do 983 (006964c0 casos 0x2ff/0x32b-0x32e/0x33c); os emissores (club, shotreview, taskmanager) ja existiam sem receptor
	switch (id)
	{
	case 0x2fe:
		// B579: 983 006964c0 case 0x2fe - Player 0x2718 com o angulo da camera + SetAngle (emissor: shotreview.cpp)
		SendMsg(this, "Player", 0x2718, (int)&GOLFDOC()->m_cameraData.angle, 0, 0, 0);
		SetAngle(GOLFDOC()->m_cameraData.angle);
		return;
	case 0x2ff:
		if (*(int*)GDOCPTR983(GOLFDOC(), 0x158) == 8)
		{
			m_topViewMode = 1;
			ChangeGameMode(0x200);
		}
		return;
	case 0x32b:
		if (m_bAutoShot != 0)
		{
			unsigned char v = (m_screenLockMode != 0x10) ? 0 : 0x10;
			m_screenLockMode = v;
			SendMsg(this, "Screen", 0x330, v, 1, 0, 0);
		}
		return;
	case 0x32c:
		*(unsigned long*)msg.param1 = m_bAutoShot;
		return;
	case 0x32d:
		if (((SHAREDDOC_ULONG(0x812) >> 0xe) & 1) == 0)
			m_bAutoShot = (int)m_autoShotCount > 0; // 983: comparacao com sinal (0 < param_1[0x91])
		else
			m_bAutoShot = 1;
		return;
	case 0x32e:
		if (((SHAREDDOC_ULONG(0x812) >> 0xe) & 1) == 0)
		{
			m_autoShotCount = m_autoShotCount - 1;
			if ((int)m_autoShotCount < 1)
				m_bAutoShot = 0;
		}
		return;
	case 0x316:
		// B577: 983 006964c0 case 0x316 - o mouse esta sobre o botao move_tee (+0x51c)? (emissor: helpmsg.cpp)
		*(bool*)msg.param1 = false;
		if (m_pMoveTeeBtn)
		{
			WVector mouse = g_input->GetMousePoint();
			WRect rect = m_pMoveTeeBtn->GetRect();
			WPoint point(mouse.x, mouse.y);
			if (rect.IsInRect(point))
				*(bool*)msg.param1 = true;
		}
		return;
	case 0x329:
		// B577: 983 FUN_00675d40 - trava de tela (+0x240); tipos de taco 1/2 so aceitam 0x10, tipo 3 nao faz nada, >3 so manda a msg
		if (m_bAutoShot != 0 && CLUB_XOR() != 0xffffffff)
		{
			unsigned char want = (unsigned char)msg.param1;
			bool set = (msg.param2 == 1);
			unsigned char clubType = CLUB_TYPE();
			if (clubType == 3)
				return;
			if ((clubType == 1 || clubType == 2) && want != 0x10)
				return;
			if (clubType <= 2)
			{
				if (!set && m_screenLockMode == want)
					want = 0;
				m_screenLockMode = want;
			}
			SendMsg(this, "Screen", 0x330, m_screenLockMode, set ? 1 : 0, 0, 0);
		}
		return;
	case 0x32a:
		// B577: 983 FUN_00681fc0 - esvazia a fila e solta a trava de tela
		*(unsigned long*)&m_cmdQueueTime = 0;
		while (!m_cmdQueue.empty())
			m_cmdQueue.pop_front();
		m_screenLockMode = 0;
		SendMsg(this, "Screen", 0x330, 0, 0, 0, 0);
		return;
	case 0x33b:
		// B576: o DoScoreBoard do 645 virou mensagem no 983 (006964c0 case 0x33b), disparada pelo timer do placar
		if (m_pRuleBase)
		{
			m_pRuleBase->NextHole983();
			m_pRuleBase->ShowHoleBg983(Doc()->m_golfGame.gameType);
		}
		CheckNetMsgWait_Send("DoScoreBoard");
		SendMsg983("Score", MsgObject(this, 0x176, 0, 0, 0, 0, 0));
		SendMsg983("Score", MsgObject(this, 0x172, 0, 0, 0, 0, 0));
		SendMsg983("Player", MsgObject(this, 0x1ce, 0, 0, 0, 0, 0));
		SendMsg983("Mascot", MsgObject(this, 0x1ce, 0, 0, 0, 0, 0));
		SendMsg983("Caddie", MsgObject(this, 0x1ce, 0, 0, 0, 0, 0));
		return;
	case 0xc6:
	{
		// B764: 983 006964c0 case 0xc6 (emissor: CHoleInOneGameHandler::SetHolePos 0057bad0, so ao ator principal; no 645 nao existe).
		// Poe a bola no tee do buraco (+ raio) e refaz o turno do Hole-in-One. FUN_004d1740(angulo) = RotMat(angulo, 1) (B775).
		bool bHoleIn = msg.param1 == 1;
		bool bMission = msg.param2 == 1;
		char* ball = (char*)&GolfBall();
		float radius = *(float*)(ball + 0xc8);
		char* hole = *(char**)GDOCPTR983(GOLFDOC(), 0xd8) + GOLFDOC()->m_currentHole * 0x38;
		WVector tee = *(WVector*)(hole - 0x38);
		WVector pos(tee.x + 0.0f, tee.y + radius, tee.z + 0.0f);
		*(WVector*)ball = pos;
		sPlayerData* p0 = PLAYER(0);
		*((unsigned char*)p0 + 0x4da) = 0;
		*(unsigned long*)p0 = 0;
		*(WVector*)((char*)p0 + 0x14) = pos;
		if (*(int*)(ball + 0x64) >= 0)
			*(WVector*)(ball + 0xcc) = pos;
		*(int*)(ball + 0x28) = GetPVS().GetMeshIndex(*(WVector*)ball, false);
		*(int*)(ball + 0x68) = -1;
		g_audio->StopSfx("shot_dist_count");
		if (!bHoleIn)
			GOLFDOC()->m_cameraData.angle = m_shotData.angle;
		else
		{
			WVector dir = *(WVector*)(hole - 0x2c) - *(WVector*)ball;
			GOLFDOC()->m_cameraData.angle = CalcDeltaAngle(dir, WVector::UNIT_POS_Z);
		}
		SetAngle(GOLFDOC()->m_cameraData.angle);
		// 983 (006964c0 caso 0xc6): FUN_004d1740(angulo) = RotMat(angulo, 1); alvo = bola + za
		m_targetPos = *(WVector*)ball + RotMat(GOLFDOC()->m_cameraData.angle, 1).za;
		SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
		GolfClub().SetPowerShot(0);
		SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
		SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0); // 983 00698a36
		float zero = 0.0f;
		SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
		SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
		CFx::Instance()->ClearAll(true);
		SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
		if (m_pRuleBase && Doc()->m_golfGame.gameType == 0xb)
			((CGolfRuleHoleInOne*)(void*)m_pRuleBase)->OnShotEnd();
		SetTurnChangeEvent();
		ReadyForNextTurn();
		float v1 = 0.0f, v2 = 0.0f;
		if ((unsigned long)(SHAREDDOC_UCHAR(0x109b8) - 0xd) > 1 || !bHoleIn || !bMission) // 983 00698b10: ja (sem sinal)
			AfxGetTask()->GetActor("Gateway") << MsgObject(NULL, 0x7535, 7, (int)&v1, (int)&v2, 0, 0);
		*(float*)(ball + 0x34) = v1;
		*(float*)(ball + 0x38) = v2;
		SendMsg(this, "Spin", M983(0x165), 0, 0, 0, 0);
		*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
		m_bFastForwardReady = 0;
		m_bShotInFlight = 0;
		m_bReplayReady = 0;
		AfxGetTask()->GetActor("Gateway") << MsgObject(NULL, 0x7535, 0x15, 0, 0, 0, 0);
		return;
	}
	case 0x33c:
		ChangeGameMode(0x20000);
		return;
	}
	CTaskMain::HandleMsgCommon(msg);
}

void CGolfRule::DispatchMatchItems()
{
	unsigned char rounds = GetHoles() / 9;
	unsigned char hole = 0;
	bool found = false;
	do
	{
		if (Doc()->m_holeOrder[hole++] == Doc()->m_pGolfDoc->m_currentHole)
		{
			found = true;
			break;
		}
	} while (hole < 0x12);
	if (!found)
		hole = 0x13;
	if (rounds == 0)
		rounds = 1;
	if (hole > 0)
	{
		if ((int)hole % (int)rounds == 0 && Doc()->m_golfGame.gameType == 3)
		{
			unsigned long item = 0;
			unsigned int sum = 0;
			for (unsigned int n = 0; n < 6; n++)
			{
				sum += g_ladderItem[n].count;
				if (Doc()->m_holeRandom[hole / rounds] % 100 < sum)
				{
					item = g_ladderItem[n].tid | 0x18000000;
					break;
				}
			}
			int slots = 10;
			if (IsLocalContent((localContentType_t)3))
				slots = 8;
			int offset = 0;
			for (int p = 0; p < GOLFDOC()->m_playerNum; p++)
			{
				if (PLAYER(p)->state != 3)
				{
					for (int j = 0; j < slots; j++)
					{
						if (*(unsigned long*)((char*)&Doc()->m_userInfo[0] + (0x3b46 - 0x38b0) + offset + j * 4) ==
							0x1800000d)
						{
							*(unsigned long*)((char*)&Doc()->m_userInfo[0] + (0x3b46 - 0x38b0) + offset + j * 4) = item;
							break;
						}
					}
				}
				offset += 0x31c6;
			}
		}
	}
}

// 983 0068c2f0 (casos 0/1/2 do modo 0x80): mensagem do Effect pelo grau da barra (FUN_004ae160+0x10): 2 -> 0x1db, 3 -> 0x1da,
// 4 (so sem tacada especial, init&0x70==0) -> 0x1d9 (CParticleEffect: pangya_shot.seq / nomal_shot.seq). 0 = nenhuma.
static int ShotGradeEffectMsg983(int grade, unsigned long initSpecial)
{
	if (grade == 2)
		return 0x1db;
	if (grade == 3)
		return 0x1da;
	if (grade == 4 && (initSpecial & 0x70) == 0)
		return 0x1d9;
	return 0;
}

float __fastcall MascotProperty983(unsigned char player, unsigned char type); // 00635ef0, definida abaixo

void CGolfRule::ChangeGameMode(unsigned long mode)
{
	static unsigned long s_shotImpactFx = 0xffffffff;

	unsigned long cur = GOLFDOC()->m_gameMode;
	if (cur == 0x200 && GOLFDOC()->m_bDropRequest != 0)
	{
		if (mode == 0x4000)
		{
			GOLFDOC()->m_bDropRequest = 0;
			if (m_pCancelDropDlg)
				m_pCancelDropDlg->Close(FrNONE, false);
		}
		else
		{
			if (m_pCancelDropDlg != NULL)
				return;
			m_pCancelDropDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			m_pCancelDropDlg->Open(
				(FRESH_PFN_RESULT)&CGolfRule::CancelDropDlgResult, 0);
			m_pCancelDropDlg->SetMessage(
				K2L_Compatibility("\xb0\xf8 \xbf\xc5\xb1\xe2\xb1\xe2 \xb8\xf0\xb5\xe5\xb8\xa6 \xc3\xeb\xbc\xd2\xc7\xd5\xb4\xcf\xb4\xd9."),
				false);
			m_pCancelDropDlg->EnableDrag(false);
			return;
		}
	}
	GOLFDOC()->m_gameMode = mode;
	*(unsigned long*)&m_modeTime = 0;
	// 983 (0068c383): modo novo aos interpretadores de fx (0xc7 -> FxInterpreter::SetMode); sem isto
	// os efeitos com regra de modo (AddModeRule, scythe_girl.seq) nunca trocam
	SendMsg(this, "Player", 0xc7, mode, 0, 0, 0);
	SendMsg(this, "Caddie", 0xc7, mode, 0, 0, 0);
	SendMsg(this, "Mascot", 0xc7, mode, 0, 0, 0);
	SendMsg(this, "Screen", 0xc7, mode, 0, 0, 0);
	CGolfBall& ball = GolfBall();
	if (0)
	{
		// keeps the out-of-line copies the aliases above need
		Bar();
		GetPVS().SetFrame(0);
	}
	switch (mode)
	{
	case 0x200000:
	{
		GOLFDOC()->m_shotPhase = 0;
		m_modeDuration = Random(0.0f, 36.0f);
		m_camPivot =
			(GetHoleCupPos() + GetTeeShotPos()) * 0.5f;
		g_view->SetClip(32.0f, 3200.0f, true);
		SCENEMGR()->SetVisible(MakeStr("PLAYER_%d", (unsigned int)CGM_CUR()),
			false);
		SCENEMGR()->SetVisible("\xc5\xac\xb7\xb4", false);
		SCENEMGR()->SetVisible("\xc4\xb3\xb5\xf0", false);
		CShadowManager::Instance()->Enable(false);
		SendMsg(this, "Score", M983(0x13b), 1, 0, 0, 0);
		break;
	}
	case 0x40000:
	{
		UIAlpha() = 0x80;
		CChatMsg::Instance()->ReplaceBackupUiAlpha();
		break;
	}
	case 1:
	{
		// 983 (0068c2f0, caso 1): sem o aviso ao Gateway do 645; o 0x7532 sub 0xa so sai do OnInitFinished (0069d847)
		GetPVS().ResetHoleData();
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		SendMsg(this, "Score", M983(0x12e), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
		m_modeDuration = 0.0f;
		GOLFDOC()->m_bTeeShotReady = 0;
		ball.Reset(
			WVector(0.0f, ball.m_radius, 0.0f) + GetTeeShotPos());
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		ResetData(ball.m_pos);
		// 983: sem zerar CTHunter+0x5e e doc+0x10777/0x10778 por buraco (so inicio de partida 008571c3/009c2aa8; os flags do tesouro duram ate o resultado)
		// 983 (0068c5b9): so o tipo 0xb online; o 645 testava buraco inicial e +0x194 (sai na carga do Practice)
		// 983 (0068c5b9) 2o ramo: online, buraco atual (+0x17c) == buraco inicial (shared+0x1051e) e (tipo 0xa ou +0x194 == 0)
		if ((Doc()->m_golfGame.gameType == 0xb && OnlinePlay()) ||
			(OnlinePlay() && GOLFDOC()->m_currentHole == SHAREDDOC_UCHAR(0x1051e) &&
				(Doc()->m_golfGame.gameType == 0xa || GOLFDOC()->m_bNoStat == 0)))
		{
			SendMsg(this, "GolfBg", M983(0x1e5), (int)&m_bgPosList, 0, 0, 0);
			break;
		}
		ChangeGameMode(2);
		break;
	}
	case 0x10000:
	{
		if (SHAREDDOC_ULONG(0x10590) != 0 &&
			*(const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8) != '\0')
			g_audio->PlaySfx((const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8),
				NULL, 1, NULL, NULL, 0.5f, 0.0f);
		SendMsg(this, "GolfBg", M983(0x142), m_shotData.weather, 0, 0, 0);
		WINDEX2().Init(0xff, 0xff, 0, 1);
		Wind().SetWind(m_shotData.wind);
		(*(WPolySoup**)((char*)GOLFDOC() + 8))->UpdateBgFxWind();
		SCENEMGR()->UpdateWind();
		GetPVS().ResetHoleData();
		WVector ground = GetPVS().GetGroundPoint(
			m_shotData.pos, false, NULL, false);
		m_shotData.pos = ground;
		m_shotData.pos.y = ball.m_pos.y + m_shotData.pos.y;
		ball.Reset(m_shotData.pos);
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		ResetData(ball.m_pos);
		*((unsigned char*)PLAYER(0) + 0x4da) = m_shotData.bunker;
		CQuadTree& pvs = GetPVS();
		pvs.m_frame = 0;
		*(unsigned long*)&pvs.m_frameTime = 0;
		SetEnvironmentVariableA();
		float powerShot = m_shotData.clubPower == 0 ? 0.0f : 1.0f;
		SendMsg(this, "Screen", M983(0x1aa), (int)&powerShot, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1b3), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1c5), 0, 0, 0, 0);
		SendMsg(this, "Logo", M983(0x18b), 0, 0, 0, 0);
		SendMsg(this, "Item", M983(0x19b), 0, 0, 0, 0);
		ChangeTurn();
		GOLFDOC()->MakeDefaultCamera(false);
		CShadowManager::Instance()->Enable(true);
		GOLFDOC()->m_currentPlayer = 0;
		if (Doc()->m_replayPlayType == 2 ||
			(Doc()->m_replayPlayType == 1 && Doc()->m_replayShotIndex == 0))
			ChangeGameMode(2);
		else
			ChangeGameMode(8);
		break;
	}
	case 0x100000:
	{
		char path[512];
		strcpy(path, g_executeDirectory);
		sprintf(path, "%s/data/Sound/bgm", path);
		CheckUserBGM(path);
		m_bgmIndex = (unsigned char)(rand() % m_bgmList.size());
		g_audio->StopSfx(NULL);
		// 983: msg 0x26e do HandleMsg (mandada aqui) = som ambiente por clima; o 645 tocava so o bgm do mapa
		if (GetWeather() == 1)
		{
			if (GetCurMap() != 9)
			{
				char rain[32];
				sprintf(rain, "\xb9\xf8\xb0\xb3\xc1\xd8\xba\xf1%d", rand() % 2 + 1);
				g_audio->PlaySfx(rain, NULL, 0, NULL, NULL, 0.5f, 200.0f); // 983 006997fb: sem repeticao
			}
		}
		// 983 0069984a: chuva cai no som do mapa abaixo; so a neve (2) sai direto
		if (GetWeather() == 2)
		{
			if (!IsCurMapSnow())
				g_audio->PlaySfx("\xba\xf1", NULL, 1, "AmbientSound", NULL, 0.5f, 200.0f);
		}
		else if (SHAREDDOC_ULONG(0x10590) == 0 ||
			*(const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8) == '\0')
			g_audio->StopSfx("AmbientSound");
		else
			g_audio->PlaySfx((const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8), NULL, 1,
				"AmbientSound", NULL, 0.5f, 200.0f);
		GOLFDOC()->m_shotPhase = 0;
		GetPVS().ResetHoleData();
		ball.Reset(PLAYER(CGM_CUR())->pos);
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		m_bShotInFlight = 0;
		m_bTimeOutSent = 0;
		m_bReplayReady = 0;
		m_bTurnStarted983 = 0;
		m_bFastForwardReady = 0;
		SendMsg(this, "Ball", M983(0x1eb), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
		SendMsg(this, "Clock", M983(0x1e9), 0, 0, 0, 0);
		SendMsg(this, "GolfBg", M983(0x13d), GetWeather(), 0, 0, 0);
		m_targetPos = ball.m_pos + RotMat(GOLFDOC()->m_cameraData.angle, 1).za;
		SetAngle(GOLFDOC()->m_cameraData.angle);
		SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
		SendMsg(this, "Caddie", 0x359, 0, 0, 0, 0); // 983 (0068c2f0, caso 0x100000)
		SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
		float powerShot = 1.0f;
		if (CLUB_PS() == 0)
			powerShot = 0.0f;
		SendMsg(this, "Screen", M983(0x1aa), (int)&powerShot, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1b3), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1c5), 0, 0, 0, 0);
		SendMsg(this, "Logo", M983(0x18b), 0, 0, 0, 0);
		SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x17f), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
		SendMsg(this, "Score", M983(0x12f), 0, 0, 0, 0);
		SendMsg(this, "Score", 0x164, 0, 0, 0, 0); // 983 (0068c2f0, caso 0x100000): 2a mensagem ao placar
		ChangeTurn();
		GOLFDOC()->MakeDefaultCamera(false);
		(&GOLFDOC()->m_inputList983)->clear();
		WPuppet* puppet = SCENEMGR()->GetPuppet("\xb0\xf8");
		if (puppet && puppet->m_rootbone)
			puppet->SetAlpha(0xff);
		CShadowManager::Instance()->Enable(true);
		CFx::Instance()->ClearAll(true);
		ChangeGameMode(8);
		WINDEX2().MakeWindList();
		WINDEX2().CheckSpecialWind();
		(*(WPolySoup**)((char*)GOLFDOC() + 8))->UpdateBgFxWind();
		break;
	}
	case 2:
	{
		if (Doc()->m_playingMode == 1)
		{
			if (cur == 0x400000)
				m_bHoleIntroEntered = 1;
			if (m_bHoleIntroEntered == 0)
			{
				ChangeGameMode(0x400000);
				break;
			}
			if (cur != 0x2000000)
			{
				Doc()->m_shotDataList.clear(); // 983: CSharedDoc+0x110bc (std::list; anda no cl novo)
				m_replayBasePang =
					*(WCrypticValue<int>*)((char*)PLAYER(MyIndex()) + 0x4f8);
				m_replayBaseBonusPang =
					*(WCrypticValue<int>*)((char*)PLAYER(MyIndex()) + 0x508);
			}
		}
		else if (Doc()->m_playingMode == 2)
		{
			*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x508) = 0;
			*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x4f8) = 0;
			Doc()->m_replayState = 1;
			GetShotCntData(); // 983 (0068c72a): avanca o indice e carrega a 1a tacada da fita; o indice nao e gravado aqui
		}
		// 983 (0068c72f): o icone/nome sobre a cabeca some nos tipos de pratica (00500200: realGameType 0xc/0xe/0x13)
		if (CHeadIcon::IsInstantiated())
		{
			const unsigned char realType = Doc()->m_roomInfo.realGameType;
			if (realType == 0xc || realType == 0xe || realType == 0x13)
				CHeadIcon::Instance()->Hide();
			else
				CHeadIcon::Instance()->Show();
		}
		// 983 (0068c72f..): voltando do modo 0x2000000 (jogo tipo 0xb) o resto da preparacao nao se repete
		if (cur != 0x2000000)
		{
		switch (GetWeather())
		{
		case 2: // 983 HandleMsg 0x26e: so IsCurMapSnow; o teste do mapa 9 vale so para a chuva
			if (!IsCurMapSnow())
				g_audio->PlaySfx("\xba\xf1", NULL, 1, "AmbientSound", NULL,
					0.5f, 200.0f);
			break;
		case 1:
			if (GetCurMap() != 9)
				g_audio->PlaySfx(MakeStr("\xb9\xf8\xb0\xb3\xc1\xd8\xba\xf1%d",
									 rand() % 2 + 1),
					NULL, 0, NULL, NULL, 0.5f, 200.0f);
		default:
			if (SHAREDDOC_ULONG(0x10590) != 0 &&
				*(const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8) != '\0')
				g_audio->PlaySfx((const char*)(SHAREDDOC_ULONG(0x10590) + 0xe8),
					NULL, 1, "AmbientSound", NULL, 0.5f, 200.0f);
			else
				g_audio->StopSfx("AmbientSound");
			break;
		}
		m_bgmIndex = (unsigned char)(rand() % m_bgmList.size());
		SendMsg(this, "GolfBg", M983(0x13d), GetWeather(), 0, 0, 0);
		SendMsg(this, "Effect", M983(0x17a), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
		// B1522, 983 0068c82c: o periodo/status dos cards e refeito no inicio do buraco (modo 2 = so o jogador 0)
		if (Doc()->GetPlayingMode() == 2)
			CCardManager::Instance()->CalcCardPeriodAndStatus983();
		else
			CCardManager::Instance()->CalcCardPeriodAndStatus();
		if (!OnlinePlay())
			this << MsgObject(NULL, M983(0x243), 0, 0, 0, 0, 0);
		for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER((unsigned char)i)->state != 3)
			{
				if (GOLFDOC()->m_bNoStat == 0)
					Doc()->m_holeStatistics[i].dwHole += 1;
				PLAYER((unsigned char)i)->state = 0;
				PLAYER((unsigned char)i)->pos =
					WVector(0.0f, GolfBall().m_radius, 0.0f) +
					GetTeeShotPos();
			}
		}
		GOLFDOC()->m_currentPlayer = GetFirstTurn();
		CQuadTree& pvs = GetPVS();
		pvs.m_frame = 0;
		*(unsigned long*)&pvs.m_frameTime = 0;
		SetEnvironmentVariableA();
		if (!IsMyTurn(false))
			SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1b2), 0, 0, 0, 0);
		SendMsg(this, "Logo", M983(0x18b), 0, 0, 0, 0);
		SendMsg(this, "NPC", M983(0x18a), 0, 0, 0, 0);
		// 983 0068c2f0: a entrada do jogador (0x2716 + camera 0xcb) passou para a regra (ReadyNextShot, slot 12); o -3 do 645
		// (jogo de massa fora do 1o buraco) cortava o movimento de entrada do Practice para os 3 s finais
		GOLFDOC()->MakeDefaultCamera(false);
		m_pRuleBase->ReadyNextShot();
		ChangeTurn();
		PLAYER(CGM_CUR())->stroke[GOLFDOC()->m_currentHole - 1] = 1;
		// 983 0068c9fd: o pote do Skins (+0x1b0..+0x1c8) e o time de quem joga (stroke 1, +0x3ad, +0x3ae) sairam daqui para o
		// ReadyNextShot da regra (Skins 006aa290 A434, Team 006aab70 A435), chamado logo acima; com os dois, o pote dobrava
		CShadowManager::Instance()->Enable(true);
		WINDEX2().MakeWindList();
		WINDEX2().CheckSpecialWind();
		if (*(WPolySoup**)((char*)GOLFDOC() + 8))
			(*(WPolySoup**)((char*)GOLFDOC() + 8))->UpdateBgFxWind();
		}
		// 983 (0068c72f): tipo 0xb vindo do modo 0x400000 entra no modo 0x2000000
		if (Doc()->m_golfGame.gameType == 0xb && cur == 0x400000)
			ChangeGameMode(0x2000000);
		break;
	}
	case 4:
	{
		g_view->SetFOV(0.9599311f);
		SendMsg(this, "Screen", M983(0x1a4), 1, 0, 0, 0);
		SendMsg(this, "Caddie", 0x2716, (int)"\xc2\xf9\xbd\xba", 0, 0, 0);
		if (Doc()->m_golfGame.gameType != 1)
			SendMsg(this, "Caddie", M983(0x1dc), 0x22, 0, 0, 0);
		WVector caddiePos; // never initialised in the original either
		SendMsg(this, "Caddie", M983(0xf4), (int)&caddiePos, 0, 0, 0);
		((CCameraWorkEx*)CCameraWork::Instance())
			->SetCaddieCamera(&g_camera, &caddiePos);
		break;
	}
	case 8:
		// 983 (0068cc..): vindo do modo 4 (closeup do caddie) o inicio do turno (+0x648) e recarregado
		if (cur == 4)
			m_turnStartTime = g_CurrentTime;
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
		{
			if (Doc()->m_playingMode == 1)
			{
				Doc()->m_replayPlayType = 0;
				unsigned char player = CGM_CUR();
				*(unsigned long*)&m_replayGauge = Doc()->m_golfGame.gameType == 1
					? *(WCrypticValue<int>*)&TEAM(PLAYER(player)->team)->gauge
					: *(WCrypticValue<int>*)&PLAYER(player)->gauge;
			}
			else if (Doc()->m_playingMode == 2)
				SetEnvironmentVariableA();
		}
		if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
		{
			// B1522, 983 0068cbf5..0068cc32: no inicio do turno o 983 refaz os status do jogador (SetPlayerLevel 00664c30)
			// no jogo normal; o CalcCardPeriodAndStatus do 645 aqui foi para o inicio do buraco (0068c848). Sem isto a troca
			// de equipamento no Praticar nao mudava Forca/Controle (dono16)
			CCardManager::Instance()->SetPlayerIndex(CGM_CUR());
			if (GOLFDOC()->m_tutorialMode == 0xf)
				GOLFDOC()->SetPlayerLevel(CGM_CUR());
		}
		_ImeReset();
		if (GOLFDOC()->m_tutorialMode == 0xf)
		{
			GR_WND_SHOW(0x48, true);
			GR_WND_SHOW(0x4fc, true);
			GR_WND_SHOW(0x500, true);
			ToggleCommandBtns(true);
			GR_WND_SHOW(0x518, true);
			GR_WND_SHOW(0x514, true);
			COneLineBoard::Instance()->ShowBg(true);
			GR_WND_SHOW(0x13c, true);
			// 983 (0068ccda): os alarmes do mensageiro (645 +0x10c/+0x110) sairam; no lugar, o botao novo +0x51c aparece na
			// sala de tipo real 0xe ou de buracos tipo 4
			if (m_pMoveTeeBtn)
			{
				if (Doc()->m_roomInfo.realGameType == 0xe)
					m_pMoveTeeBtn->SetVisible(true);
				else if (Doc()->m_roomInfo.holeType == 4)
					m_pMoveTeeBtn->SetVisible(true);
			}
		}
		else if (GOLFDOC()->m_tutorialMode == 9)
			ToggleCommandBtns(true);
		m_bTurnStarted983 = 1;
		if (*((unsigned char*)&ball + 0x58))
		{
			*((unsigned char*)&ball + 0x58) = 0;
			GOLFDOC()->MakeDefaultCamera(true);
			*(unsigned long*)((char*)&ball + 0x5c) = 0;
		}
		else
			g_camera = ((CGolfDocEx2*)GOLFDOC())->GetDefaultCamera();
		if (IsMyTurn(true) ||
			(IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
				Doc()->m_playingMode == 2))
		{
			SendMsg(this, "Spin", M983(0x163), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a7), 0, 0, 0, 0);
			UIAlpha() = 0xff;
			CChatMsg::Instance()->ReplaceBackupUiAlpha();
		}
		else
			SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		if (((*(unsigned char*)USERINFO983(CGM_CUR(), 0x3922)) & 1) == 0)
			SendMsg(this, "Camera", M983(0x19a), 0, 0, 0, 0);
		SendMsg(this, "Camera", M983(0xee), 0, 0, 0, 0);
		{
			// 983: so manda 0x216 (M983 0x1a4) se nao for tee do 1o buraco com stroke 0xfe
			CGolfDoc* gd = GOLFDOC();
			const unsigned char* sv = (const unsigned char*)gd->GetPlayer(gd->m_currentPlayer) + 0x1c + gd->m_currentHole * 0x10;
			if (gd->m_b195_983 == 0 /* B1467: era +0x195 cru */ || gd->m_currentHole != Doc()->m_holeOrder[0] ||
				(unsigned char)(sv[0xe] ^ sv[0]) != 0xfe)
				SendMsg(this, "Screen", M983(0x1a4), 1, 0, 0, 0);
		}
		SendMsg(this, "Spot", M983(0x163), 0, 0, 0, 0);
		g_view->SetFOV(0.9599311f);
		if (Doc()->m_replayState == 1)
		{
			Doc()->m_replayState = 0;
			g_input->SetFakeInput("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				false);
		}
		else if (Doc()->m_replayState == 7)
			g_input->SetFakeInput("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				false);
		break;
	case 0x10:
	{
		g_audio->StopSfx("\xc5\xb8\xc0\xd3");
		CMouseCursor::Instance()->SetActive(false);
		CMouseCursor::Instance()->SetCamMode(false);
		SendMsg(this, "Score", M983(0x12f), 0, 0, 0, 0);
		SendMsg(this, "Score", 0x164, 0, 0, 0, 0); // 983 (0068c2f0, caso 0x10): 2a mensagem ao placar
		if (g_pFresh)
			g_pFresh->GetManager()->CloseForm(false);
		SendMsg(this, "Camera", M983(0x19b), 0, 0, 0, 0);
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		GOLFDOC()->m_shotPhase += 1;
		SendMsg(this, "Spin", M983(0x161), (int)((char*)&GolfBall() + 0x34),
			(int)((char*)&GolfBall() + 0x38), 0, 0);
		SendMsg(this, "Spot", M983(0xfa), (int)&m_shotExtra983, 0, 0, 0);
		*(unsigned long*)((char*)&GolfBall() + 0x44) = 0;
		(&GOLFDOC()->m_inputList983)->clear();
		m_cmdTick = 0;
		// 983 (0068c2f0, caso 0x10): auto flag, auto shot e seta de curva
		SendMsg(this, "Screen", 0x313, 0, 0, 0, 0);
		SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
		SendMsg(this, "Screen", 0x332, 0, 0, 0, 0);
		if (!OnlinePlay())
			break;
#pragma pack(push, 1)
		struct
		{
			unsigned char type;
			unsigned long value;
		} data;
#pragma pack(pop)
		data.type = 1;
		data.value = *(unsigned long*)&GOLFDOC()->m_bar.center;
		WSendPacket packet((enumClientPacket)0x14);
		packet.EncodeBuffer(&data, 5);
		packet.Send(TO_GAME);
		break;
	}
	case 0x20:
	{
		if (GR_WND983(0x4dc) && CCapturedBg::Instance()->IsCaptured()) // m_pChatView: cru p/ VC10 igual (B1569: pelo membro troca 2 instrucoes do ChangeGameMode)
			GR_WND983(0x4dc)->SetVisible(false);
		GOLFDOC()->m_bar.dir = 1;
		m_bTurnStarted983 = 0;
		m_gaugeTick = 0;
		m_gaugeStartTick = 0;
		m_gaugePauseTick = 0;
		if (!IsMyTurn(false))
			break;
		timeBeginPeriod(1);
		CFx::Instance()->SetActive(false);
		break;
	}
	case 0x100:
	{
		UIAlpha() = 0x80;
		CChatMsg::Instance()->ReplaceBackupUiAlpha();
		break;
	}
	case 0x400:
	{
		m_savedCamera983 = g_camera;
		m_dbgEye983 = g_camera.pivot;
		WVector at(g_camera.xz + m_dbgEye983.x, g_camera.yz + m_dbgEye983.y,
			g_camera.zz + m_dbgEye983.z);
		(m_camPivot) = at;
		SendMsg(this, "Camera", M983(0x19b), 0, 0, 0, 0);
		GR_WND_SHOW(0x48, false);
		GR_WND_SHOW(0x4fc, false);
		GR_WND_SHOW(0x500, false);
		ToggleCommandBtns(false);
		GR_WND_SHOW(0x13c, false);
		if (m_pMoveTeeBtn) // 983 (0068f8c8/00690920): no lugar dos alarmes do mensageiro do 645
			m_pMoveTeeBtn->SetVisible(false);
		// 983 (00690932): o 0x400 segue para o mesmo trecho do 0x800
	}
	// fallthrough
	case 0x800:
	{
		if (GOLFDOC()->m_bNetGameEnd != 0)
			break;
		if ((unsigned long)CCameraWork::Instance()->m_cameraType == 0x15) // B1465: era CAMERAWORK_ULONG(0x8c)
			CCameraWork::Instance()->UninstallCamera();
		*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
		if (PLAYER(CGM_CUR())->state == 3)
		{
			m_bTimeOutSent = 1;
		}
		// 983 (00690932): sem a msg Screen 0xbc do 645 aqui
		break;
	}
	case 0x200:
	{
		GR_WND_SHOW(0x48, false);
		GR_WND_SHOW(0x4fc, false);
		GR_WND_SHOW(0x500, false);
		ToggleCommandBtns(false);
		GR_WND_SHOW(0x13c, false);
		if (m_pMoveTeeBtn) // 983 (0068f8c8/00690920): no lugar dos alarmes do mensageiro do 645
			m_pMoveTeeBtn->SetVisible(false);
		UIAlpha() = 0x80;
		CChatMsg::Instance()->ReplaceBackupUiAlpha();
		g_view->SetFOV(1.0471976f); // 983 (0068f.. modo 0x200): so SetFOV(60 graus); sem SetClip (o 645 punha 3.2/3200)
		g_camera = GOLFDOC()->MakeTopViewCameraMatrix(false);
		SendMsg(this, "Camera", M983(0xf1), 0, 0, 0, 0);
		SCENEMGR()->UpdateFog(false);
		if (GR_WND_S(m_pKbdManualDlg)) // B1488: era GR_WND(0x158) cru (cl 14.44 anda -4: BaseObject)
			(*(void(__fastcall**)(FrWnd*, int, int))(*(int*)GR_WND_S(m_pKbdManualDlg) + 8))(
				GR_WND_S(m_pKbdManualDlg), 0, 1);
		break;
	}
	case 0x40:
	{
		if (CCapturedBg::Instance() != NULL)
			CCapturedBg::Instance()->Reset();
		if (CLUB_TYPE() != 3)
		{
			int timing = GetBarClickTiming();
			SendMsg(this, "Screen", M983(0x1ac), 0, 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x1df), timing, 0, 0, 0);
			if (CLUB_PS() != 0)
			{
				g_audio->PlaySfx("powershot_ball_effect", NULL, 0, NULL, NULL,
					0.5f, 200.0f);
				SendMsg(this, "Effect", M983(0x17e), 1, 0, 0, 0);
			}
			else
				SendMsg(this, "Player", M983(0x10f), 0, 0, 0, 0);
		}
		SendMsg(this, "Clock", M983(0x1ea), 0, 0, 0, 0);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		if (IsMyTurn(false)) // 983 0068c2f0: FUN_00530c00() guarda o ShotReview
			SendMsg(this, "ShotReview", 0x2fd, (int)"SAVE_SPOT_INDICATOR", 0, 0,
				0); // HACK: raw .rdata pointer
		if (CLUB_TYPE() != 3 && CgmBar().grade == 4)
			SHOTSTAT(int, dwPangya)++;
		CGM_CUR_PD()->shotTime += GOLFDOC()->m_shotTime;
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
			Doc()->m_playingMode == 2)
			GolfBall().m_initSpecial = m_shotData.special;
		float t = (CgmBar().power - BAR_START) / g_fBarRange;
		float sq = sqrtf(t);
		float vel = GolfClub().GetPower() * sq;
		vel = GolfBall().GetPowerFactor() * vel;
		vel = GetBarClickPowerFactor() * vel;
		CGolfBall::sExpected* pre0 = CGM_BALL(ball).PreDataToWrite(0);
		pre0->event = 0;
		pre0->pos = *(WVector*)&ball;
		pre0->state = GetInitBallState();
		unsigned long item = Doc()->m_userInfo[CGM_CUR()].charInfo.tid;
		BallSpeedFactor bsf(item, &GolfBall(), m_shotExtra983);
		WVector* launch = &pre0->vel;
		*launch = bsf.vel;
		ball.m_initCurve = bsf.f8;
		ball.m_initSpin = bsf.fc;
		float spinA = bsf.f0;
		float spinB = bsf.f4;
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
			Doc()->m_playingMode == 2)
		{
			GR_BALL_ULONG(&ball, 0x34) = *(unsigned long*)&m_shotData.curve;
			spinB = m_shotData.angle;
			spinA = m_shotData.deviation;
		}
		if (CgmBar().grade > 1)
		{
			*launch =
				(((Abs<float>(ball.m_initCurve) * 0.1f + 1.0f) *
					 vel) *
					RotMat(((CClubEx3*)&GolfClub())->GetLie(NULL), 0).za) *
				RotMat(spinB + spinA, 1);
		}
		switch (Doc()->m_playingMode)
		{
		case 1:
			if (!WNetworkSystem::Instance()->IsConnected(
					(WNetworkSystem::eNetUnit)0) ||
				IsMyTurn(false))
			{
				memset(&m_shotData, 0, 0x88);
				m_shotData.hole = GOLFDOC()->m_currentHole;
				m_shotData.wind = Wind().GetGlobalWind();
				m_shotData.weather = GetWeather();
				m_shotData.stroke = PLAYER(CGM_CUR())->stroke[GOLFDOC()->m_currentHole - 1]; // 983: +0x1c + hole*0x10 (o 645 era +0x10)
				m_shotData.angle = spinB;
				m_shotData.pvsFrame = GetPVS().GetFrame();
				m_shotData.pos = *(WVector*)&ball;
				memcpy(&m_shotData.bar, &CgmBar(), 9 * 4);
				*(unsigned long*)&m_shotData.curve = GR_BALL_ULONG(&ball, 0x34);
				*(unsigned long*)&m_shotData.spin = GR_BALL_ULONG(&ball, 0x38);
				m_shotData.special = ball.m_initSpecial;
				m_shotData.clubPower = CLUB_PS();
				m_shotData.bunker = CGM_CUR_PD()->bunker;
				strcpy(m_shotData.club,
					(const char*)(*(int*)((char*)&GolfClub() + 4) +
						0x54)); // HACK: orig copies byte by byte into 0x350
				m_shotData.item = CGM_CUR_PD()->item;
				m_shotData.deviation = spinA;
				WCrypticValue<unsigned char>* stroke;
				if (Doc()->m_golfGame.gameType == 1)
					stroke =
						&((WCrypticValue<unsigned char>*)((char*)TEAM(
															  CGM_CUR_UCHAR(
																  0x4e9)) -
							0xc))[GOLFDOC()->m_currentHole];
				else
					stroke = &PLAYER(CGM_CUR())->stroke[GOLFDOC()->m_currentHole - 1];
				m_shotData.holeStroke = *stroke;
				int base = m_replayBasePang;
				m_shotData.pang =
					(int)*(
						WCrypticValue<int>*)((char*)PLAYER(CGM_CUR()) + 0x4f8) -
					base;
				base = m_replayBaseBonusPang;
				m_shotData.bonusPang =
					(int)*(
						WCrypticValue<int>*)((char*)PLAYER(CGM_CUR()) + 0x508) -
					base;
				m_shotData.gauge = m_replayGauge;
				Doc()->m_shotDataList.push_back(m_shotData);
			}
			break;
		case 2:
			GR_BALL_ULONG(&GolfBall(), 0x34) = *(unsigned long*)&m_shotData.curve;
			GR_BALL_ULONG(&GolfBall(), 0x38) = *(unsigned long*)&m_shotData.spin;
			GolfBall().m_initSpecial = m_shotData.special;
			CGM_PVS().SetFrame(m_shotData.pvsFrame);
			break;
		}
		float shotVal;
		SendMsg(this, "Player", M983(0x126), (int)&shotVal, 0, 0, 0);
		if (!OnlinePlay() || IsMassGame())
		{
			SendMsg(this, "Player", M983(0x77), 0, 0, 0, 0);
			m_bShotInFlight = 1;
			m_bFastForwardReady = ball.m_groundType != 2;
		}
		else if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
			Doc()->m_playingMode == 2)
			SendMsg(this, "Player", M983(0x77), 0, 0, 0, 0);
		GR_ULONG(0x5b0) = GetPVS().GetFrame(); // m_pvsFrame[0..2]: crus no ChangeGameMode todo p/ VC10 igual (B1568: pelo membro troca 2 instrucoes)
		GR_ULONG(0x5b4) = GR_ULONG(0x5b0) + 0x32;
		GR_ULONG(0x5b8) = GR_ULONG(0x5b4) + (int)(shotVal * 50.0); /*983: fmul +50 (d05020), add*/
		CGM_PVS().SetFrame(GR_ULONG(0x5b8));
		*(WVector*)((char*)&ball + 0xc) = *launch;
		ball.m_state = pre0->state;
		ball.m_special = ball.m_initSpecial;
		m_bSpecialCamDone983 = 1;
		m_bDriveCamStarted983 = 0;
		m_replayState = 0;
		ball.m_water = CGolfBall::WATER_NONE;
		ball.SetHoleIn(0);
		*(unsigned short*)((char*)PLAYER(CGM_CUR()) + 0x4d8) = 0xffff;
		CGM_CUR_PD()->shotFlag = 0;
		if (GolfBall().m_state != 0)
			CGM_CUR_PD()->bunker = 0;
		((CQuadTreeEx2*)&GetPVS())->ResetVariable();
		ball.ClearBuffer();
		ball.m_vectorSlideCount = 0;
		ball.m_renderNum = 0;
		ball.m_bounceFrame = -1;
		ball.m_topFrame = -1;
		ball.m_nearObjFrame = -1;
		ball.m_holeFrame = -1;
		ball.m_holeOutFrame = -1;
		ball.m_greenFrame = -1;
		ball.m_cupFrame = -1;
		ball.m_afterBurnerFrame = -1;
		ball.m_rollFrame =
			ball.m_state != 3 ? -1 : 0;
		ball.m_stayCount = 0;
		ball.m_standGround = 0;
		ball.m_prevColTriIndex = -1;
		*(short*)((char*)&ball + 0xb2) = -1;
		ball.m_colTriIndex = -1;
		*(short*)((char*)&ball + 0xb8) = -1;
		ball.m_meshIndex = -1;
		ball.m_cobraFrame = -1;
		ball.m_tomahawkFrame = -1;
		GR_BALL_ULONG(&ball, 0x54) = 0;
		GR_BALL_ULONG(&ball, 0x50) = 0;
		GR_BALL_ULONG(&ball, 0x4c) = 0;
		ball.m_spikeFrame = -1;
		ball.m_spikeDiveFrame = -1;
		ball.m_spikeEndFrame = -1;
		ball.m_vectorSlideFrame = -1;
		unsigned char savedFlag59 = ball.m_groundType;
		if (CgmBar().grade == 1)
		{
			GR_BALL_ULONG(&ball, 0x40) = 0;
			GR_BALL_ULONG(&ball, 0x3c) = 0;
		}
		else
		{
			if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
				Doc()->m_playingMode == 2)
				((CClubEx3*)&GolfClub())->SetApproachMode();
			if (*(unsigned char*)((char*)&GolfClub() + 0x3c) == 4)
			{
				// B1389, 983 0068db5f..0068dc52: aux + bonus de alcance + mascote, cada um *0.5, e o card pelo
				// GetTotalRange (inclui periodo do card especial e o redutor) tambem *0.5; o ramo de buff 0x70 do 645 saiu
				float range = Doc()->GetAuxPartProperty(CGM_CUR(), 0) * 0.5f;
				range = Doc()->GetRangeBonus983(CGM_CUR()) * 0.5f + range;
				range = MascotProperty983(CGM_CUR(), 0) * 0.5f + range;
				if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
				{
					CCardManager::Instance()->SetPlayerIndex(CGM_CUR());
					range = CCardManager::Instance()->GetTotalRange(CLUB_PS() != 0) * 0.5f + range;
				}
				float add = (float)(int)*(
					WCrypticValue<int>*)((char*)PLAYER(CGM_CUR()) + 0x51c);
				ball.m_spin =
					(Max<float>(0.0f, (float)((add + range) / 170.0)) + 1.5f) *
					t * t;
			}
			else
				GR_BALL_ULONG(&ball, 0x40) = 0;
			ball.m_curve = t * ball.m_initCurve;
		}
		GetPVS().m_vec21d8_983 = WVector::ZERO; // 983 0068dcd0: 0066f9b0(vetor nulo 00f041c4); a previsao e o console gravam este campo
		WVector* bv = (WVector*)((char*)&ball + 0xc);
		while (ball.m_state != 0)
		{
			ball.m_renderNum++;
			CGolfBall::sExpected* cur =
				CGM_BALL(ball).PreDataToWrite(ball.m_renderNum);
			cur->sound[0] = 0;
			cur->event = 0;
			cur->effect = 0;
			ProcessType9Areas983((char*)&ball, 0.02f); // 0068dd50 (lista tipo 9)
			GetPVS().PreProcess(0.02f, true);
			GetPVS().BallProcess(0.02f);
			// 983 (0068c2f0 ~0068de00): tambem entra quando a bola ja tem o estado 0x20/0x40, mesmo sem taco tipo 4
			if (*(unsigned char*)((char*)&GolfClub() + 0x3c) == 4 ||
				(ball.m_special & 0x60) != 0)
			{
				if ((ball.m_special & 0x20) != 0)
				{
					if (ball.m_cobraFrame < 0)
						ball.m_pos.y =
							Max<float>(ball.m_pos.y,
								((CQuadTreeEx2*)&GetPVS())
									->GetBasePosY(*(WVector*)&ball,
										ball.m_radius +
											ball.m_radius));
					else
						ball.m_spin =
							ball.m_spin - 0.02f;
				}
				else if ((ball.m_special & 0x40) != 0)
				{
					if (ball.m_spikeFrame < 0)
						ball.m_pos.y =
							Max<float>(ball.m_pos.y,
								((CQuadTreeEx2*)&GetPVS())
									->GetBasePosY(*(WVector*)&ball,
										ball.m_radius +
											ball.m_radius));
					else
						ball.m_spin =
							ball.m_spin - 0.02f;
				}
				else if (ball.m_bounceFrame < 0 &&
					((CQuadTreeEx2*)&GetPVS())->GetObjColNum() == 0)
					ball.m_spin = ball.m_spin -
						(0.5f - ball.m_initSpin * 0.1f) * 0.02f;
				else
					ball.m_spin =
						ball.m_spin - 0.002f;
			}
			bool resetBall = false;
			if (ball.m_renderNum >= 3000)
				resetBall = true;
			else if (ball.m_renderNum >= 0x5dc)
			{
				resetBall = true;
				if (0.1f <= bv->Magnitude())
				{
					if (((CQuadTreeEx2*)&GetPVS())->GetObjColNum() < 10)
					{
						if (GetPVS().GetMeshIndex(*(WVector*)&ball, false) !=
							-1)
							resetBall = false;
					}
				}
			}
			if (resetBall)
			{
				bv->z = 0.0f;
				bv->y = 0.0f;
				bv->x = 0.0f;
				*(WVector*)&ball = GetPVS().GetGroundPoint(*(WVector*)&ball,
					false, NULL, false);
				ball.m_pos.y =
					ball.m_radius + ball.m_pos.y;
				ball.m_state = CGolfBall::BALL_STOP;
			}
			cur->pos = *(WVector*)&ball;
			cur->vel = *bv;
			*(unsigned long*)&cur->curveRot = *(unsigned long*)&ball.m_curveRot; // 983: mov dword (sem fld)
			*(unsigned long*)&cur->spinRot = *(unsigned long*)&ball.m_spinRot;
			cur->state = ball.m_state;
			if (ball.m_state == 0)
				ball.m_special &= 0xffffff8f;
			if (ball.m_state == 1 &&
				ball.m_vel.y < 0.0f &&
				ball.m_topFrame < 0)
				ball.m_topFrame = ball.m_renderNum;
			if (ball.m_tomahawkFrame < 0 &&
				(GR_BALL_UCHAR(&ball, 0x44) & 0x10) != 0 &&
				(GR_BALL_UCHAR(&ball, 0x48) & 0x10) == 0)
				ball.m_tomahawkFrame = ball.m_renderNum;
			if ((GR_BALL_UCHAR(&ball, 0x48) & 0x20) != 0)
			{
				if (ball.m_renderNum == 1)
				{
					GR_BALL_ULONG(&ball, 0x10) = 0;
					*bv = vel * bv->Normalize();
				}
				WVector moved = *(WVector*)&ball -
					*(WVector*)(ball.m_renderNum < 0
							? NULL
							: (char*)&ball + 0xcc);
				moved.y = 0.0f;
				if (ball.m_cobraFrame < 0)
				{
					float dist = moved.Magnitude();
					if ((GolfClub().GetRange(NULL) * sq - 100.0f) * 3.2f < dist)
					{
						ball.m_cobraFrame = ball.m_renderNum;
						bv->Normalize();
						unsigned char clubKind =
							*(unsigned char*)(*(int*)((char*)&GolfClub() + 4) +
								0x20);
						if (clubKind == 0)
							*bv *= 74.0f;
						else if (clubKind == 1)
							*bv *= 76.0f;
						else if (clubKind == 2)
							*bv *= 80.0f;
						*bv *= sq;
						ball.m_spin = 2.5f;
					}
				}
			}
			if (ball.m_spikeEndFrame < 0 &&
				(GR_BALL_UCHAR(&ball, 0x44) & 0x40) != 0 &&
				(GR_BALL_UCHAR(&ball, 0x48) & 0x40) == 0)
			{
				if (0 < ball.m_spikeDiveFrame)
					ball.m_spikeEndFrame = ball.m_renderNum;
			}
			if ((GR_BALL_UCHAR(&ball, 0x48) & 0x40) != 0)
			{
				if (ball.m_renderNum == 1)
				{
					GR_BALL_ULONG(&ball, 0x10) = 0;
					*bv = vel * bv->Normalize();
				}
				WVector moved = *(WVector*)&ball -
					*(WVector*)(ball.m_renderNum < 0
							? NULL
							: (char*)&ball + 0xcc);
				moved.y = 0.0f;
				if (ball.m_spikeFrame < 0 &&
					0.0f < moved.Magnitude())
				{
					ball.m_spikeFrame = ball.m_renderNum;
					bv->Normalize();
					*bv *= 72.5f;
					*bv *= sq + sq;
					ball.m_spin = 3.1f;
				}
				if (ball.m_topFrame >= 0 &&
					ball.m_renderNum >
						ball.m_topFrame + 0x3c &&
					ball.m_spikeDiveFrame < 0)
				{
					ball.m_spikeDiveFrame = ball.m_renderNum;
					unsigned char clubKind = *(
						unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x20);
					float base;
					switch (clubKind)
					{
					case 0:
						base = GolfClub().GetPower() * sq < 344.0f
							? GolfClub().GetPower() * sq
							: 344.0f;
						ball.m_vel.y =
							-8.0f - (344.0 - base) / 112.0 * 21.5;
						break;
					case 1:
						base = GolfClub().GetPower() * sq < 306.0f
							? GolfClub().GetPower() * sq
							: 306.0f;
						ball.m_vel.y =
							-10.299999f - (306.0 - base) / 105.0 * 20.5;
						break;
					case 2:
						base = GolfClub().GetPower() * sq < 273.0f
							? GolfClub().GetPower() * sq
							: 273.0f;
						ball.m_vel.y =
							-10.799999f - (273.0 - base) / 100.0 * 20.2f;
						break;
					}
					*bv *= sq * 7.0f;
					GR_BALL_ULONG(&ball, 0x40) = GR_BALL_ULONG(&ball, 0x38);
				}
			}
			if (cur->event == 6 &&
				ball.m_vectorSlideFrame == -1)
				ball.m_vectorSlideFrame = ball.m_renderNum;
		}
		ball.m_groundType = savedFlag59;
		ball.m_special = ball.m_initSpecial;
		// 983 (0068c2f0): sem o corte do jogo em massa do 645 (GR_BYTE(0x5b2) + pacote 0xe5 tipo 8 + "Cutin"); no 983 o
		// unico 0xe5 sai do HitShot (00694110) e a trajetoria vai pelo pacote 0x12f (0067e140)
		ball.DbgTraceOutput(
			(const char*)(*(int*)((char*)&GolfClub() + 4) + 0x54),
			GolfClub().GetRange(NULL));
		SendMsg(this, "NPC", M983(0x186), 0, 0, 0, 0);
		if (0 < ((CQuadTreeEx2*)&GetPVS())->GetObjColNum())
			ball.m_nearObjFrame = -1;
		CGM_PVS().SetFrame(GR_ULONG(0x5b0) - 1); // m_pvsFrame[0]: cru p/ VC10 igual (B1568, ver 0x5b8 abaixo)
		GetPVS().PreProcess(0.02f, true);
		ball.m_totalTime =
			(float)ball.m_renderNum * 0.02f;
		GR_BALL_ULONG(&ball, 0x3c) = GR_BALL_ULONG(&ball, 0x34);
		GR_BALL_ULONG(&ball, 0x40) = GR_BALL_ULONG(&ball, 0x38);
		if (ball.m_bOB == 0)
			ball.m_bOB =
				GOLFDOC()->m_pPolySoup->IsOutOfBound(*(WVector*)&ball);
		if (Doc()->m_playingMode == 1)
			AutoReplay();
		SendMsg(this, "Player", M983(0x115),
			(int)CGM_BALL(ball).PreData(ball.m_renderNum), 0, 0, 0);
		// backup of the player state before the shot (pushed on the undo stack)
		m_backUp.pos = *(WVector*)((char*)PLAYER(0) + 0x14);
		m_backUp.cameraAngle = spinB;
		m_backUp.bOB = ball.m_bOB;
		m_backUp.bunker = PLAYER(0)->bunker;
		m_backUp.gauge = PLAYER(0)->gauge;
		m_backUp.totalPang = (int)*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x4f8);
		m_backUp.bonusPang = (int)*(WCrypticValue<int>*)((char*)PLAYER(0) + 0x508);
		m_backUp.holePang = (int)PLAYER(0)->pang[GOLFDOC()->m_currentHole - 1]; // 983: +0x25c + hole*0x10
		m_backUp.holeStroke = PLAYER(0)->stroke[GOLFDOC()->m_currentHole - 1]; // 983: +0x1c + hole*0x10
		m_backUp.flag = PLAYER(0)->flag;
		strcpy(m_backUp.club,
			(const char*)(*(int*)((char*)&GolfClub() + 4) +
				0x54)); // HACK: orig copies byte by byte
		m_backUpStack.push(m_backUp);
		if (ball.m_bOB != 0)
		{
			*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14) =
				ball.m_water != 0
				? GetPVS().GetPenaltyPos()
				: *(const WVector*)CGM_BALL(ball).PreData(0);
			CGM_CUR_PD()->bunker = 0;
		}
		else if (!GolfBall().IsHoleIn() &&
			!(GolfBall().m_standGround > 0))
		{
			(*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))
				->CalcDropPos(*(WVector*)CGM_BALL(ball).PreData(
					ball.m_renderNum));
			*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14) =
				(*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))->GetDropPos();
			CGM_CUR_PD()->bunker = 0;
		}
		else
		{
			*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14) =
				*(WVector*)CGM_BALL(ball).PreData(ball.m_renderNum);
			unsigned char gt = GetPVS().GetGroundType(
				*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14), true, NULL);
			if (gt <= 7 || gt == 0xb)
				*(WVector*)((char*)PLAYER(CGM_CUR()) + 8) =
					*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14);
		}
		if (ball.m_bOB == 0)
		{
			if (((CQuadTreeEx2*)&GetPVS())
					->IsOverWater(*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14)))
			{
				ball.m_bOB = true;
				ball.m_water = CGolfBall::WATER_NONE;
				*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14) =
					*(WVector*)CGM_BALL(ball).PreData(0);
				CGM_CUR_PD()->bunker = 0;
			}
		}
		if (Doc()->m_golfGame.gameType == 0xa)
			*(WVector*)((char*)PLAYER(CGM_CUR()) + 0x14) =
				*(WVector*)CGM_BALL(ball).PreData(ball.m_renderNum);
		if (Doc()->m_golfGame.gameType == 1)
		{
			if (CGM_CUR_PD()->teammate != 0xff)
			{
				PLAYER(CGM_CUR_PD()->teammate)->pos =
					PLAYER(CGM_CUR())->pos;
				PLAYER(CGM_CUR_PD()->teammate)->bunker =
					CGM_CUR_PD()->bunker;
			}
			TEAM(CGM_CUR_PD()->team)->pos =
				PLAYER(CGM_CUR())->pos;
			TEAM(CGM_CUR_PD()->team)->bunker =
				CGM_CUR_PD()->bunker;
		}
		else if (Doc()->m_golfGame.gameType == 0xb)
			SendMsg(this, s_gatewayActor[1], 0x7535, 5, 0, 0, 0); // 983 (0068c2f0, caso 0x40): OnReadScreenValue do Hole-in-One
		// 983 (0068c2f0, caso 0x40): online, foto dos controladores do jogador da vez (OnSnapshot)
		if (OnlinePlay() == 1)
			GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 7, CGM_CUR(), 0, 0, 0);
		if (GOLFDOC()->m_tutorialMode == 0xf || GOLFDOC()->m_tutorialMode == 8)
			SendMsg(this, "Screen", M983(0x1b4), 0, 0, 0, 0);
		m_bCancelShotAsk = false;
		*(unsigned long*)&m_forgetDlgTime = 0; // 983: mov dword [esi+0x4a0],0 (nao fldz)
		if (ball.m_bOB != 0 && OnlinePlay() && IsMassGame() &&
			Doc()->m_golfGame.gameType != 0xa && CanCancelShot())
			m_bCancelShotAsk = true;
		if (!m_bCancelShotAsk)
			SendShotResult();
		*(WVector*)&ball = pre0->pos;
		*bv = *launch;
		ball.m_meshIndex =
			GetPVS().GetMeshIndex(*(WVector*)&ball, false);
		unsigned long* shotCount;
		if (CLUB_TYPE() == 3)
		{
			if (ball.m_groundType == 2)
				*(WCrypticValue<unsigned char>*)((char*)PLAYER(CGM_CUR()) +
					(GOLFDOC()->m_currentHole + 0x13) * 0x10 + 0xc) += 1; // 983: putt[] +0x14c
			shotCount = &SHOTSTAT(unsigned long, dwPutt);
		}
		else
			shotCount = &SHOTSTAT(unsigned long, dwDrive);
		(*shotCount)++;
		// 983 (0068c2f0, caso 0x40): depois da tacada os controladores reavaliam (OnFire09 -> IController::On09)
		GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 8, 0, 0, 0, 0);
		if (CLUB_TYPE() != 3)
		{
			int combo = 0;
			float aux = Doc()->GetAuxPartProperty(CGM_CUR(), 4);
			// 983 0068c2f0: o tipo do mascote (00635ef0, tipo 4) tambem da combo, e o item Everyshot soma % ao medidor e vale como anel 11
			float mascotType = MascotProperty983(CGM_CUR(), 4);
			float everyshot = 0.0f;
			CComboGaugeEveryshotObject* pEveryshot =
				(CComboGaugeEveryshotObject*)GOLFDOC()->m_pComboGaugeEveryshot983[GOLFDOC()->m_currentPlayer];
			if (pEveryshot && pEveryshot->IsHit())
			{
				everyshot = (float)pEveryshot->GetPercent();
				aux = 11.0f;
			}
			switch (CgmBar().grade)
			{
			case 4:
			{
				bool equipped = false;
				// 983 0067e200 (chamado so aqui): os 5 itens de forca, inclusive 0x1800002b
				if (IsPowerItem983())
					equipped = true;
				if (IsLocalContent((localContentType_t)0x4c /*983: era 0x4b; medido*/) && aux == 11.0f)
				{
					bool hit = false;
					if ((GR_BALL_UCHAR(&GolfBall(), 0x44) & 3) != 0 ||
						(GR_BALL_UCHAR(&GolfBall(), 0x44) & 0xc) != 0)
						hit = true;
					combo =
						GetShotMotionGrade(true, CLUB_PS() != 0, hit, equipped);
				}
				else if (CLUB_PS() == 0 && !equipped)
				{
					if ((GR_BALL_UCHAR(&GolfBall(), 0x44) & 3) != 0)
						combo = (aux == 9.0f ? 1 : 0) + 14;
					else if ((GR_BALL_UCHAR(&GolfBall(), 0x44) & 0xc) != 0)
						combo = (aux == 9.0f ? 1 : 0) + 14;
					else
						combo = (aux == 9.0f ? 1 : 0) + 12;
				}
				switch ((int)mascotType) // 983 00677a60
				{
				case 0x1f:
				case 0x2f:
					combo += (CLUB_PS() != 0 || equipped) ? 3 : 0; // 0066e180
					break;
				case 0x20:
				case 0x33:
					if (CLUB_PS() == 0 && !equipped)
						combo += 1;
					break;
				}
				if (CLUB_PS() == 0 && !equipped)
				{
					if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
					{
						CCardManager::Instance()->SetPlayerIndex(CGM_CUR());
						// B1389: 983 0068ef4c soma GetTotalComboGauge983 (caddie + periodo do card especial)
						combo +=
							CCardManager::Instance()->GetTotalComboGauge983();
					}
					if (!IsMassGame() && Doc()->m_golfGame.gameType != 3)
					{
						unsigned char need = 0;
						if (Doc()->m_golfGame.gameType == 0 ||
							Doc()->m_golfGame.gameType == 7)
							need = CgLevelGrade(
								Doc()->m_userInfo[CGM_CUR()].stat.Level);
						else if (Doc()->m_golfGame.gameType == 1)
						{
							need = CgLevelGrade(
								Doc()->m_userInfo[CGM_CUR()].stat.Level);
							if (CGM_CUR_PD()->teammate != 0xff)
							{
								if (Doc()->m_userInfo[CGM_CUR_PD()->teammate].stat.Level <
									6)
									need += 1;
							}
							else
								need += (Doc()->m_userInfo[CGM_CUR()].stat.Level -
											1) /
										5 +
									1;
							need >>= 1;
						}
						if (need < GOLFDOC()->m_maxGrade)
							combo += (GOLFDOC()->m_maxGrade - need) * 2;
					}
				}
				break;
			}
			case 3:
			{
				bool equipped = false;
				// 983 0068c2f0 caso 3: mesma lista do 0067e200 (0a/07/02/12/2b; o nosso trocava o 07 pelo 2b)
				if (IsPowerItem983())
					equipped = true;
				if (IsLocalContent((localContentType_t)0x4c /*983: era 0x4b; medido*/) && aux == 11.0f)
					combo = GetShotMotionGrade(false, CLUB_PS() != 0, false,
						equipped);
				else if (CLUB_PS() == 0)
					combo = aux == 9.0f ? 7 : 4;
				if (mascotType == 31.0f || mascotType == 47.0f) // 983 0068ee4a
					combo += (CLUB_PS() != 0 || equipped) ? 3 : 0; // 0066e180
				break;
			}
			case 2:
				if (CLUB_PS() == 0)
					combo = -4;
				break;
			case 0:
			case 1:
				if (CLUB_PS() == 0)
					combo = -17;
				break;
			}
			if (Doc()->m_golfGame.gameType == 3 || Doc()->m_golfGame.gameType == 7)
			{
				if (*(WCrypticValue<unsigned char>*)((char*)PLAYER(CGM_CUR()) +
						(GOLFDOC()->m_currentHole + 1) * 0x10 + 0xc) > // 983: stroke[] +0x2c
						GOLFDOC()->GetHoleData().par &&
					0 < combo)
					combo = 0;
			}
			// 983 0068f460: o desconto so vale sem item de tacada (+0x70); com o bonus +0x6c = 1 cai para 0x21 e so nos niveis 2/3
			if (CLUB_PS() != 0 && GolfClub().m_powerShotItemType983 == 0)
			{
				bool localPs = IsLocalContent((localContentType_t)0x32 /*983: era 0x31; medido*/);
				if (GolfClub().m_powerShotBonus983 == 0)
				{
					if (CGM_CUR_PD()->item != 0x18000004 &&
						CGM_CUR_PD()->item != 0x18000010 &&
						CGM_CUR_PD()->item != 0x18000022 &&
						CGM_CUR_PD()->item != 0x18000027 &&
						CGM_CUR_PD()->item != 0x1800002e &&
						CGM_CUR_PD()->item != 0x1800002f)
					{
						if (CLUB_PS() == 1)
							combo -= 0x21;
						else if (CLUB_PS() == 2 || (localPs && CLUB_PS() == 3))
							combo -= 0x42;
					}
				}
				else if (GolfClub().m_powerShotBonus983 == 1)
				{
					if (CLUB_PS() == 2 || (localPs && CLUB_PS() == 3))
						combo -= 0x21;
				}
			}
			if ((SHAREDDOC_UCHAR(0x10a99) & 8) != 0)
				combo = 0;
			if ((GOLFDOC()->m_tutorialMode == 0xf || GOLFDOC()->m_tutorialMode == 3) &&
				combo != 0)
			{
				float delta;
				if (combo < 1)
					delta = (float)combo;
				else
					delta =
						Doc()->GetAuxPartProperty(CGM_CUR(), 2) * (float)combo;
				if (Doc()->m_golfGame.gameType == 1)
				{
					// 983 0068f5f0: limite por jogador (0062fd10), o maior da dupla (004ad970)
					float limit = Doc()->GetComboGaugeLimit(CGM_CUR());
					if (CGM_CUR_PD()->teammate != 0xff)
						limit = Max<float>(limit, Doc()->GetComboGaugeLimit(CGM_CUR_PD()->teammate));
					TEAM(CGM_CUR_PD()->team)->gauge = Between(0.0f,
						TEAM(CGM_CUR_PD()->team)->gauge + delta + everyshot, limit);
				}
				else
				{
					PLAYER(CGM_CUR())->gauge =
						Between(0.0f,
							PLAYER(CGM_CUR())->gauge + // 983 0068f70d: +0x4c8 (o 645 lia +0x4b8)
								delta + everyshot,
							Doc()->GetComboGaugeLimit(CGM_CUR()));
				}
			}
		}
		m_nextTurn = GetNextTurn();
		m_bTimeOutSent = 0;
		m_bTurnStarted983 = 1;
		m_bPuttTutorial983 = 0;
		if ((m_replayFlags & 2) == 0)
		{
			ReactionForShot();
			CCameraWork::Instance()->m_replayCount = 0; // 983 0068f795 +0x2d8 (645 0x2a4); B1465: era CAMERAWORK_ULONG(0x2d8)
			bool keepCamera = false;
			if (IsLocalContent((localContentType_t)0x32 /*983: era 0x31; medido*/))
			{
				if ((CLUB_PS() == 2 || CLUB_PS() == 3) && CLUB_TYPE() == 0)
					keepCamera = true;
			}
			else if (CLUB_PS() == 2 && CLUB_TYPE() == 0)
				keepCamera = true;
			if (!keepCamera)
				CCameraWork::Instance()->SelectCamera(-1);
		}
		CMouseCursor::Instance()->SetActive(true);
		if (Doc()->m_golfGame.gameType == 0xa)
			SendMsg(this, "Screen", M983(0x1a5), 1, 0, 0, 0);
		break;
	}
	case 0x80:
	{
		m_pCamArea983 = NULL;
		m_bRainbowFx983 = 0;
		SendMsg(this, "Effect", M983(0x17f), 0, 0, 0, 0);
		if (m_replayState == 0)
		{
			SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a4), 0, 0, 0, 0);
			SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
		}
		(&GOLFDOC()->m_inputList983)->clear();
		const CGolfBall::sExpected* first = CGM_BALL(ball).PreData(0);
		ball.m_playFrame = 0;
		ball.m_state = *(CGolfBall::eBallState*)((char*)first + 0x70);
		GR_BALL_ULONG(&ball, 0x2c) = 0;
		ball.m_spinRot =
			CalcDeltaAngle(WVector::UNIT_POS_Z, ball.m_vel);
		unsigned long frame = GR_ULONG(0x5b8); // m_pvsFrame[2]: pelo membro o VC10 troca 2 instrucoes de ChangeGameMode (B1568)
		*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
		CGM_PVS().SetFrame(frame);
		if (0 < ball.m_rollFrame)
			SendMsg(this, "Ball", M983(0x1ec), 0, 0, 0, 0);
		if ((m_replayFlags & 2) == 0 || m_replayState == 2)
		{
			SendMsg(this, "Screen", M983(0x1a5), 1, 0, 0, 0);
			if (!g_audio->IsPlaying("shot_dist_count"))
				g_audio->PlaySfx("shot_dist_count", NULL, 1, "shot_dist_count",
					NULL, 0.5f, 0.0f);
		}
		if (ball.m_state != 1)
		{
			if (ball.m_state == 3)
				SendMsg(this, "Trajectory", M983(0x15d), 0, 0, 0, 0);
			break;
		}
		float power = (CgmBar().power - BAR_START) / g_fBarRange;
		unsigned char ground = GetPVS().GetGroundType(ball.m_pos, false, NULL);
		if (CLUB_TYPE() != 3 && power > 0.8f && ground != 1 && ground != 6 &&
			((CFxEx*)CFx::Instance())->FindControl(s_shotImpactFx) == NULL)
			s_shotImpactFx = ((CFxEx2*)CFx::Instance())->MakeShotImpact(&ball);
		((CFxEx2*)CFx::Instance())
			->MakeClubImpact(&ball,
				Doc()->m_userInfo[CGM_CUR()].clubInfo.tid);
		WVector travel =
			*(WVector*)CGM_BALL(ball).PreData(ball.m_renderNum) -
			*(WVector*)first;
		m_sfxName[0] = 0;
		char* sfx = m_sfxName;
		travel.y = 0.0f;
		// 983 0068c2f0: o som de distancia vem da tabela de sons do item (00b3ac70; estado 0x4000/0x2000/0x1000 pela
		// distancia 640/480/320); sem som na tabela, o padrao e "\xb0\xf8\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2" 1/2/3
		unsigned long item = *(unsigned long*)(USERINFO983(CGM_CUR(), 0x3b42));
		float dist = travel.Magnitude();
		int dstate = dist > 640.0f ? 0x4000 : dist > 480.0f ? 0x2000 : dist > 320.0f ? 0x1000 : 0;
		if (dstate != 0)
		{
			std::string name = CItemSoundTable::Instance().GetSoundName(item, dstate);
			if (name.empty())
				name = dstate == 0x4000 ? "\xb0\xf8\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2" "1"
					: dstate == 0x2000 ? "\xb0\xf8\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2" "2"
					: "\xb0\xf8\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2" "3";
			strncpy(sfx, name.c_str(), 0x1f);
			sfx[0x1f] = '\0';
		}
		if (*sfx != '\0')
		{
			g_audio->StopSfx(sfx);
			g_audio->PlaySfx(sfx, NULL, 0, sfx, NULL, 0.5f, 200.0f);
		}
		unsigned long powerShot = CLUB_PS();
		switch (CLUB_TYPE())
		{
		case 0:
		case 1:
			if (power > 0.8f)
			{
				if ((ball.m_initSpecial & 0x70) != 0)
				{
					// 983 (0068c2f0, caso 0x80): a tacada especial toca o som do item, sem os CVoiceItem 8/4 do 645
					CSoundItemManager::Instance()->PlayCurrentSoundItem();
					if ((ball.m_initSpecial & 0x10) != 0)
					{
						bool special = (unsigned char)powerShot == 2 ||
							(unsigned char)powerShot == 3;
						// 983: o 3o argumento e o id do item (+0x3170 do sUserInfo) que escolhe a pluma
						((CFxEx2*)CFx::Instance())
							->MakeTomahawk(&ball, special,
								(*(unsigned long*)USERINFO983(CGM_CUR(), 0x6a20)));
					}
					else if ((ball.m_initSpecial & 0x20) != 0)
					{
						((CFxEx2*)CFx::Instance())->MakeCobra(&ball);
						if ((unsigned char)powerShot == 2 ||
							(unsigned char)powerShot == 3)
							AfxGetTask()->GetActor("Trajectory")
								<< MsgObject(NULL, M983(0x15e), 0, 0, 0, 0, 0);
					}
					else if ((ball.m_initSpecial & 0x40) != 0)
					{
						bool special = (unsigned char)powerShot == 2 ||
							(unsigned char)powerShot == 3;
						((CFxEx2*)CFx::Instance())->MakeSpike(&ball, special);
					}
				}
				else
				{
					// 983 (0068c2f0, caso 0x80): o Trajectory 0x1c4 sai sempre, antes do resto (como no caso 2)
					SendMsg(this, "Trajectory", M983(0x159), 0, 0, 0, 0);
					if (CgmBar().grade == 4)
					{
						if (CLUB_PS() != 0)
						{
							SendMsg(this, "Trajectory", M983(0x158), 0, 0, 0, 0);
							SendMsg(this, "Effect", M983(0x181), 0, 0, 0, 0);
							g_audio->StopSfx("\xc7\xfd\xbc\xba");
							g_audio->PlaySfx(
								"\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2_\xc7\xfd\xbc\xba",
								NULL, 0, "\xc7\xfd\xbc\xba", NULL, 0.5f,
								200.0f);
						}
						else
						{
							SendMsg(this, "Effect", M983(0x180), 0, 0, 0, 0);
							g_audio->StopSfx("\xba\xb0\xb0\xa1\xb7\xe7");
							g_audio->PlaySfx(
								"\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2_\xba\xb0\xb0\xa1\xb7\xe7",
								NULL, 0, "\xba\xb0\xb0\xa1\xb7\xe7", NULL, 0.5f,
								200.0f);
						}
						if ((m_replayFlags & 2) == 0 || m_replayState == 2)
						{
							char text[64] = "";
							float dur = 2.0f;
							sprintf(text, "\xc6\xce\xbe\xdf%02d",
								Doc()->GetIndexPangyaLogoImage());
							SendMsg(this, "Letter", M983(0x18d), (int)text, (int)&dur,
								PangyaLogoParam983(), 0);
							SendMsg(this, "Gallery", M983(0xff), 0, 0, 0, 0);
							CSoundItemManager::Instance()
								->PlayCurrentSoundItem();
						}
					}
					else
					{
						if (Wind().GetGlobalWind().Magnitude() > 4.0f)
						{
							SendMsg(this, "Trajectory", M983(0x15b), 0, 0, 0, 0);
							g_audio->StopSfx("\xb9\xd9\xb6\xf7");
							g_audio->PlaySfx(
								"\xb3\xaf\xbe\xc6\xb0\xa1\xb1\xe2_\xb9\xd9\xb6\xf7",
								NULL, 0, "\xb9\xd9\xb6\xf7", NULL, 0.5f,
								200.0f);
						}
					}
					if (CLUB_PS() != 0)
					{
						if (CLUB_PS() == 1)
							SendMsg(this, "Effect", M983(0x16c), 0, 0, 0, 0);
						else if ((unsigned char)powerShot == 2 ||
							(unsigned char)powerShot == 3)
							((CFxEx2*)CFx::Instance())->MakePowerShot2(&ball);
						((CCameraWorkEx2*)CCameraWork::Instance())->Quake();
						g_audio->PlaySfx(
							"\xc6\xc4\xbf\xf6\xbc\xa6_\xc0\xcc\xc6\xe5\xc6\xae",
							NULL, 0, NULL, NULL, 0.5f, 200.0f);
					}
				}
				// 983: grau 2/3/4 manda o Effect 0x1db/0x1da/0x1d9
				int gradeMsg = ShotGradeEffectMsg983(CgmBar().grade, ball.m_initSpecial);
				if (gradeMsg != 0)
					SendMsg(this, "Effect", gradeMsg, 0, 0, 0, 0);
			}
			else
				SendMsg(this, "Trajectory", M983(0x159), 0, 0, 0, 0);
			// 983 (006903d9): o Ball 0x267 (efeitos de tacada/voo da bola) sai SEMPRE nos casos 0/1, depois do Effect ou
			// do Trajectory (B610 o tinha posto so no "else" do grau: bola sem efeito no ar)
			SendMsg(this, "Ball", M983(0x1ef), 0, 0, 0, 0);
			break;
		case 2:
			if (power > 0.8f)
			{
				if ((ball.m_initSpecial & 0x10) != 0)
				{
					bool special = (unsigned char)powerShot == 2 ||
						(unsigned char)powerShot == 3;
					// 983 (caso 2, 0068c2f0): equipamento 0x140000d6/c8/ce/cf toca o som do item (00b3b1e0 -> 00b3acb0, estado 0x8000)
					unsigned long equip = *(unsigned long*)(USERINFO983(CGM_CUR(), 0x3b42));
					if (equip == 0x140000d6 || equip == 0x140000c8 ||
						equip == 0x140000ce || equip == 0x140000cf)
						CItemSoundTable::Instance().PlaySound(equip, 0x8000, -1);
					((CFxEx2*)CFx::Instance())->MakeTomahawk(&ball, special,
						(*(unsigned long*)USERINFO983(CGM_CUR(), 0x6a20)));
				}
				else
				{
					// 983 (caso 2): o Trajectory 0x1c4 sai sempre; o Effect pelo grau (sem Ball)
					SendMsg(this, "Trajectory", M983(0x159), 0, 0, 0, 0);
					if (CgmBar().grade == 4)
					{
						if ((m_replayFlags & 2) == 0 || m_replayState == 2)
						{
							char text[64] = "";
							float dur = 2.0f;
							sprintf(text, "\xc6\xce\xbe\xdf%02d",
								Doc()->GetIndexPangyaLogoImage());
							SendMsg(this, "Letter", M983(0x18d), (int)text, (int)&dur,
								PangyaLogoParam983(), 0);
							SendMsg(this, "Gallery", M983(0xff), 0, 0, 0, 0);
							CSoundItemManager::Instance()
								->PlayCurrentSoundItem();
						}
					}
					int gradeMsg = ShotGradeEffectMsg983(CgmBar().grade, ball.m_initSpecial);
					if (gradeMsg != 0)
						SendMsg(this, "Effect", gradeMsg, 0, 0, 0, 0);
				}
			}
			else
				SendMsg(this, "Trajectory", M983(0x159), 0, 0, 0, 0);
			break;
		case 3:
			SendMsg(this, "Trajectory", M983(0x15d), 0, 0, 0, 0);
			break;
		}
		if ((ball.m_initSpecial & 3) != 0 &&
			0 < ball.m_afterBurnerFrame)
			SendMsg(this, "Trajectory", M983(0x15a), 0, 0, 0, 0);
		// 983 (0068c2f0, fim do 0x80): tipo de jogo 0xb avisa o Gateway (0x7535 sub 0x11) com o grau do power shot
		if (*((unsigned char*)DOCPTR983(0x10594)) == 0xb)
			GetActor(s_gatewayActor[1])
				<< MsgObject(NULL, 0x7535, 0x11, (int)(unsigned char)CLUB_PS(), 0, 0, 0);
		break;
	}
	case 0x2000:
	{
		// 983 0068c2f0 caso 0x2000: GP (0x14/0x15) com o meu jogador em estado 1 -> 0067b4a0 (camera 983 210 = nossa 209)
		if ((unsigned char)(*((unsigned char*)DOCPTR983(0x109b8)) - 0x14) < 2 &&
			DcPlayer(MyIndex())->state == 1)
		{
			SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
			SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
			SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
			SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
			CFx::Instance()->ClearAll(true);
			g_view->SetFOV(0.9599311f);
			DcPlayer(GOLFDOC()->m_currentPlayer)->item = 0;
			CCameraWork::Instance()->SelectCamera(0xd1);
			SendMsg(this, "Player", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Mascot", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Bubble", M983(0x164), 0, 0, 0, 0);
			CProjectG::Instance()->SetGameSpeed(1.0f);
		}
		// 983 (0068c2f0 caso 0x2000): retangulo vazio ao placar do GP (0x178) e ao Screen (0x249); sem DispatchMatchItems;
		// Score 0x164 (GP); Spot so fora do GP; fim de partida fora do GP; contador +0x10700; Gateway 0x7532 sub 0xe
		float rcEmpty[5] = {0, 0, 0, 0, 0};
		SendMsg(this, "Score", 0x178, (int)rcEmpty, 0, 0, 0);
		SendMsg(this, "Screen", 0x249, (int)rcEmpty, 0, 0, 0);
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			Doc()->m_replayPlayType = 0;
		ResetRank();
		m_bTurnStarted983 = 0;
		m_bSkinsNextHole = false;
		SetConstantBGM("bgm_scoreboard.mp3", false, true);
		g_audio->StopSfx(NULL);
		SendMsg(this, "Score", M983(0x12f), 1, 0, 0, 0);
		SendMsg(this, "Score", 0x164, 1, 0, 0, 0);
		bool gpRoom = (unsigned char)(*((unsigned char*)DOCPTR983(0x109b8)) - 0x14) < 2;
		// 983 (FUN_006881e0): "Spot" so sai quando NAO e (sala GP e meu jogador em estado 1)
		if (!(gpRoom && DcPlayer(MyIndex())->state == 1))
			SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		if ((!OnlinePlay() || IsMassGame()) && !gpRoom)
			SetGameOver(GOLFDOC()->m_currentHole == GetEndHole());
		if (*(int*)(DOCPTR983(0x10700)) == 0)
			*((unsigned char*)DOCPTR983(0x10704)) = 0;
		else
			*(int*)(DOCPTR983(0x10700)) -= 1;
		// 983 (0068c2f0): fora do tipo de sala 1 e do jogo 0xa, baixa 1 uso do item 0x1a000136 da lista +0x375c (std::find por tid)
		if (Doc()->m_gameMode != 1 && *((unsigned char*)DOCPTR983(0x10594)) != 0xa)
		{
			int* head = (int*)SHAREDDOC_ULONG(0x375c);
			int* n = (int*)*head;
			while (n != head && n[3] != 0x1a000136)
				n = (int*)*n;
			if (n != head && (short)n[5] > 0)
				n[5] = (n[5] & 0xffff0000) | (unsigned short)((short)n[5] - 1);
		}
		GetActor(s_gatewayActor[1]) << MsgObject(NULL, M983(0x26f), 0xe, 0, 0, 0, 0);
		AfxGetTask()->GetActor("Screen")
			<< MsgObject(NULL, M983(0xa0), 0, 0, 0, 0, 0);
		break;
	}
	case 0x400000:
	{
		if (m_bWaitForStart)
			break;
		WVector eye = GetTeeShotPos();
		eye = GetPVS().GetGroundPoint(eye, false, NULL, false);
		eye.y = eye.y + 100.0f;
		WVector dir = GetHoleCupPos() - eye;
		dir.Normalize();
		g_camera = MakeCameraMatrix(eye, dir);
		break;
	}
	case 0x1000:
	{
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
		{
			if (Doc()->m_playingMode == 1)
				Doc()->m_replayPlayType = 4;
			else if (Doc()->m_playingMode == 2 &&
				Doc()->m_replayState != 9 && Doc()->m_replayState != 7)
			{
				Doc()->m_replayShotIndex = Doc()->m_replayShotIndex + 1;
				Doc()->m_replayState = 0;
			}
		}
		m_bShotInFlight = 0;
		m_bReplayReady = 0;
		m_bFastForwardReady = 0;
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) ||
			Doc()->m_playingMode != 2)
			SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		// 983 (0068c2f0): desliga o auto flag (0x313) e os botoes de auto shot (0x32f)
		SendMsg(this, "Screen", 0x313, 0, 0, 0, 0);
		SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
		SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
		CFx::Instance()->ClearAll(true);
		unsigned char gameType = Doc()->m_golfGame.gameType;
		if (gameType == 1)
		{
			if (!SetHoleInPose_Team())
			{
				ChangeGameMode(0x800);
				break;
			}
		}
		// 983 (0068c2f0): sem SetHoleInPose_Match; gameType 3 cai no SetHoleInPose(0)
		else if (gameType == 7)
		{
			if (m_bSkinsNextHole == 0)
			{
				ChangeGameMode(0x800);
				break;
			}
			SetHoleInPose_Skins();
		}
		else
			SetHoleInPose(false);
		g_view->SetFOV(0.9599311f);
		break;
	}
	case 0x4000:
	{
		// 983 (0068c2f0, case 0x4000): sem o aviso ao Player do 645; zera item (+0x4bc) e +0x4c0
		PLAYER(CGM_CUR())->shotTime += GOLFDOC()->m_shotTime;
		PLAYER(CGM_CUR())->item = 0;
		PLAYER(CGM_CUR())->windAbility983 = 0;
		if (Doc()->m_golfGame.gameType == 1)
			TEAM(PLAYER(CGM_CUR())->team)->gauge =
				Max<float>(0.0f, TEAM(PLAYER(CGM_CUR())->team)->gauge + -30.0f);
		else
			PLAYER(CGM_CUR())->gauge =
				Max<float>(0.0f, PLAYER(CGM_CUR())->gauge + -30.0f);
		g_audio->StopSfx("\xc5\xb8\xc0\xd3");
		g_audio->PlaySfx("\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6", NULL, 0,
			"\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6", NULL, 0.5f, 200.0f);
		*((unsigned char*)PLAYER(CGM_CUR()) + 0x4db) += 1;
		Doc()->m_holeStatistics[CGM_CUR()].dwTimeOut += 1;
		int zero = 0;
		GolfClub().SetPowerShot(0);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
		SendMsg(this, "Clock", 0x271a, 0, 0, 0, 0);
		SendMsg(this, "GuideLine", M983(0x1a2), 0, 0, 0, 0);
		if (IsMyTurn(true))
		{
			UIAlpha() = 0xff;
			CChatMsg::Instance()->ReplaceBackupUiAlpha();
		}
		m_nextTurn = GetNextTurn();
		if (!CheckDropOut() && !CheckGiveUp())
		{
			if (GOLFDOC()->m_bNoStat == 0)
			{
				float dur = 1.5f;
				SendMsg(this, "Letter", M983(0x18d),
					(int)"\xc5\xb8\xc0\xd3\xbe\xc6\xbf\xf4", (int)&dur, 0, 0);
				SendMsg(this, "Screen", M983(0x1b9), 0, 0, 0, 0);
			}
			g_camera = ((CGolfDocEx2*)GOLFDOC())->GetDefaultCamera();
			SendMsg(this, "Player", 0x2716,
				(int)"\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				0, 0, 0);
			// 983 (0068c2f0 caso 0x4000): texto de 00d18318, nao o endereco fixo 0x9e9cf4 do 645
			SendMsg(this, "Caddie", 0x2716, (int)"\xba\xa1\xc4\xbf", 0, 0, 0);
			break;
		}
		RecordGiveUp(CGM_CUR());
		if (!CheckDropOut())
			RecordScoreByPlayer(CGM_CUR());
		if (Doc()->m_golfGame.gameType == 1)
		{
			ChangeGameMode(0x1000);
			break;
		}
		ChangeGameMode(0x80000);
		break;
	}
	case 0x20000:
	{
		// JP 983 (00690e6d): o resto do case do 645 (camera da premiacao, time vencedor, pacote 6) passou para a
		// posicao [14] da CGolfRuleBase (006a8440); a musica do placar saiu daqui
		CCapturedBg::Instance()->Reset();
		ResetRank();
		m_pRuleBase->V14_983();
		break;
	}
	case 0x80000:
	{
		g_view->SetFOV(0.9599311f);
		if (Doc()->m_golfGame.gameType != 1 &&
			(GOLFDOC()->m_tutorialMode == 0xf || GOLFDOC()->m_tutorialMode == 8))
		{
			SendMsg(this, "Screen", M983(0x1b4), 1, 0, 0, 0);
			SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
		}
		SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		// 983 (0068c2f0): desliga o auto flag (0x313) e os botoes de auto shot (0x32f)
		SendMsg(this, "Screen", 0x313, 0, 0, 0, 0);
		SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
		SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
		CFx::Instance()->ClearAll(true);
		SetHoleInPose(false);
		m_nextTurn = GetNextTurn();
		break;
	}
	// 983 (0068c2f0 caso 0x4000000): resto do DoToDefaultCamera (00682d90 / modo 0xf: 0067ca80 + 006828c0)
	case 0x4000000:
		DoToDefaultCameraNext983();
		break;
	// 983 (00672390): destrava a camera
	case 0x1000000:
		CCameraWork::Instance()->Unlock();
		break;
	// 983 (00673d90): camera tipo 4 sobre o copo; avisa o Gateway (0x7535 sub 10) com o tempo devolvido
	case 0x2000000:
	{
		float t = CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)4, NULL, GetHoleCupPos());
		CCameraWork::Instance()->m_i2ac983 = 0; // B1465: era +0x2e0 cru
		SendMsg(this, s_gatewayActor[1], 0x7535, 10, (int)&t, 0, 0);
		break;
	}
	case 0x800000:
	{
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			Doc()->m_replayPlayType = 0;
		m_bTurnStarted983 = 0;
		ResetRank();
		SetConstantBGM("bgm_scoreboard.mp3", false, true);
		g_audio->StopSfx(NULL);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Player", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Mascot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Bubble", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a4), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		// 983 (0068c2f0): desliga o auto flag (0x313) e os botoes de auto shot (0x32f)
		SendMsg(this, "Screen", 0x313, 0, 0, 0, 0);
		SendMsg(this, "Screen", 0x32f, 0, 0, 0, 0);
		// 983 (0068c2f0 caso 0x800000): mesmo teste FUN_00a7a8d0 do inicio do caso (0x50)
		if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
			break;
		AfxGetTask()->GetActor("Screen")
			<< MsgObject(NULL, M983(0xa0), 0, 0, 0, 0, 0);
		break;
	}
	default:
		break;
	}
}

// A random one of the four camera works of a character.
const char* __stdcall GetRandomCameraName(const char* name)
{
	return MakeStr("%s_%02d", name, rand() % 4 + 1);
}

void CGolfRule::SetHoleInPose(bool replay)
{
	char pose[0x20];
	char caddie[0x20];
	char text[0x20];
	if (!replay)
	{
		WVector& pos = HP_POS;
		float angle = 3.1415927f;
		WVector ground = pos;
		SendMsg(this, "Player", 0x2719, (int)&pos, 0, 0, 0);
		SendMsg(this, "Player", 0x2718, (int)&angle, 0, 0, 0);
		SendMsg(this, "Player", M983(0x114), 0, 0, 0, 0);
		angle = 5.4977875f;
		ground = GetPVS().GetGroundPoint(
			WVector(pos.x + 10.0f, pos.y, pos.z + 10.0f), false, NULL, false);
		SendMsg(this, "Caddie", M983(0x1d9), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x1e2), (int)&angle, 0, 0, 0);
		SendMsg(this, "Caddie", 0x2719, (int)&ground, 0, 0, 0);
	}
	int diff;
	if (Doc()->m_golfGame.gameType != 1)
	{
		unsigned char stroke = HpPlayer(GOLFDOC()->m_currentPlayer)
								   ->stroke[GOLFDOC()->m_currentHole - 1];
		diff = stroke - GOLFDOC()->GetHoleData().par;
	}
	else
	{
		unsigned char stroke =
			HpTeam(HpPlayer(GOLFDOC()->m_currentPlayer)->team)
				->stroke[GOLFDOC()->m_currentHole - 1];
		diff = stroke - GOLFDOC()->GetHoleData().par;
	}
	int holeIn = Doc()->m_golfGame.gameType == 1
		? (unsigned char)HpTeam(HpPlayer(GOLFDOC()->m_currentPlayer)->team)
			  ->stroke[GOLFDOC()->m_currentHole - 1]
		: (unsigned char)HpPlayer(GOLFDOC()->m_currentPlayer)
			  ->stroke[GOLFDOC()->m_currentHole - 1];
	if (IsLocalContent((localContentType_t)0x6d))
	{
		int charType =
			Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
			0x3ffffff;
		int index = 0;
		// 983 00678fb0: character 14 with a birdie switches to the 3W when the club type is non-zero
		if (charType == 0xe && diff == -1 && GolfClub().GetType() != 0)
			GolfClub().SetClub("3W");
		if (holeIn != 1 && (diff < -3 || diff > 3 || (index = diff + 4) >= 7))
			index = 7;
		strcpy(pose,
			(charType == 8 || charType == 10) ? g_szLuciaCharMotion[index]
						  : g_szCommonCharMotion[index]);
		if (holeIn == 1)
		{
			strcpy(caddie, "\xc8\xa6\xc0\xce\xbf\xf8");
			strcpy(text, "\xc8\xa6\xc0\xce\xbf\xf8");
		}
		else
		{
			switch (diff)
			{
			case -3:
				strcpy(caddie, "\xc8\xa6\xc0\xce\xbf\xf8");
				strcpy(text, "\xbe\xcb\xb9\xd9\xc6\xae\xb7\xce\xbd\xba");
				break;
			case -2:
				strcpy(caddie, "\xc8\xa6\xc0\xce");
				strcpy(text, "\xc0\xcc\xb1\xdb");
				break;
			case -1:
				strcpy(caddie, "\xc8\xa6\xc0\xce");
				strcpy(text, "\xb9\xf6\xb5\xf0");
				break;
			case 0:
				strcpy(caddie, "\xc8\xa6\xc0\xce");
				strcpy(text, "\xc6\xc4");
				break;
			case 1:
				strcpy(caddie, "\xbd\xc7\xb0\xdd");
				strcpy(text, "\xba\xb8\xb1\xe2");
				break;
			case 2:
				strcpy(caddie, "\xbd\xc7\xb0\xdd");
				strcpy(text, "\xb4\xf5\xba\xed\xba\xb8\xb1\xe2");
				break;
			case 3:
				strcpy(caddie, "\xbd\xc7\xb0\xdd");
				strcpy(text, "\xc6\xae\xb8\xae\xc7\xc3\xba\xb8\xb1\xe2");
				break;
			default:
				strcpy(caddie, "\xbd\xc7\xb0\xdd");
				if (GolfBall().IsHoleIn())
					sprintf(text, "+%d", diff);
				else
					strcpy(text, "\xc6\xf7\xb1\xe2");
			}
		}
	}
	else if (holeIn == 1)
	{
		strcpy(pose,
			"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
		strcpy(caddie, "\xc8\xa6\xc0\xce\xbf\xf8");
		strcpy(text, "\xc8\xa6\xc0\xce\xbf\xf8");
	}
	else
	{
		switch (diff)
		{
		case -3:
			strcpy(pose,
				"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xc8\xa6\xc0\xce\xbf\xf8");
			strcpy(text, "\xbe\xcb\xb9\xd9\xc6\xae\xb7\xce\xbd\xba");
			break;
		case -2:
			strcpy(pose, "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xc8\xa6\xc0\xce");
			strcpy(text, "\xc0\xcc\xb1\xdb");
			break;
		case -1:
			strcpy(pose, "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xc8\xa6\xc0\xce");
			strcpy(text, "\xb9\xf6\xb5\xf0");
			break;
		case 0:
			strcpy(pose,
				"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xc8\xa6\xc0\xce");
			strcpy(text, "\xc6\xc4");
			break;
		case 1:
			strcpy(pose,
				"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xbd\xc7\xb0\xdd");
			strcpy(text, "\xba\xb8\xb1\xe2");
			break;
		case 2:
			// 983 00679110: tid 10 usa a pose de double bogey
			if ((Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
					0x3ffffff) == 10)
				strcpy(pose,
					"\xb4\xf5\xba\xed\xba\xb8\xb1\xe2\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			else
				strcpy(pose,
					"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xbd\xc7\xb0\xdd");
			strcpy(text, "\xb4\xf5\xba\xed\xba\xb8\xb1\xe2");
			break;
		case 3:
			// 983 00679110: tid 10 usa a pose de double bogey
			if ((Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
					0x3ffffff) == 10)
				strcpy(pose,
					"\xb4\xf5\xba\xed\xba\xb8\xb1\xe2\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			else
				strcpy(pose,
					"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xbd\xc7\xb0\xdd");
			strcpy(text, "\xc6\xae\xb8\xae\xc7\xc3\xba\xb8\xb1\xe2");
			break;
		default:
			strcpy(pose,
				"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(caddie, "\xbd\xc7\xb0\xdd");
			if (GolfBall().IsHoleIn())
				sprintf(text, "+%d", diff);
			else
				strcpy(text, "\xc6\xf7\xb1\xe2");
		}
	}
	if (diff <= 0)
		SetConstantBGM("bgm_under_par.mp3", true, true);
	else
		SetConstantBGM("bgm_over_par.mp3", true, true);
	if (!replay)
	{
		SendMsg(this, "Player", 0x2716, (int)pose, 0, 0, 0);
		SendMsg(this, "Player", 0x2717, (int)"", 0, 0, 0);
		SendMsg(this, "Caddie", 0x2716, (int)caddie, 0, 0, 0);
		char camera[0x40] = "";
		bool hasItem = false;
		const char* name = NULL;
		SendMsg(this, "Player", 0x2715, (int)&name, 0, 0, 0);
		if (strstr(name, "_item") != NULL)
			hasItem = true;
		const char* str;
		if ((Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid &
				0x3ffffff) == 0)
			str = MakeStr("%s_%02d", name, rand() % 2 + 3);
		else
			str = MakeStr("%s_%02d", name, rand() % 4 + 1);
		strncpy(camera, str, 0x3f);
		CCameraWork* cw = CCameraWork::Instance();
		WVector& pos = HP_POS;
		m_modeDuration = cw->LoadCamera((CCameraWork::eCamMode)0, (char*)camera, pos);
		if (hasItem == true)
			CCameraWork::Instance()->m_bAbsAnim = hasItem; // 983 0067999e +0x2e5 (645 0x2b1); B1465: era cru
		if (m_modeDuration < 0.0f)
		{
			WVector eye;
			WVector at;
			eye = WVector(pos.x, pos.y + 6.4f, pos.z - 32.0f);
			at = WVector(pos.x, pos.y + 6.4f, pos.z);
			g_camera = MakeCameraMatrix(eye, at);
			m_modeDuration = 3.5f;
		}
	}
	SendMsg(this, "Letter", M983(0x18d), (int)text, 0, 0, 0);
}

bool CGolfRule::SetHoleInPose_Team()
{
	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType != 1)
		return true;
	unsigned char my = MyIndex();
	unsigned char myTeam = PLAYER(my)->team;
	bool in0 = TEAM(0)->state == 1;
	bool in1 = TEAM(1)->state == 1;
	if (!in0 && !in1)
		return false;
	if (in1 ^ in0)
	{
		unsigned char winner;
		if (in0)
		{
			sTeamData* ta = TEAM(1);
			unsigned char stroke1 =
				TEAM_HOLE_STROKE(ta, GOLFDOC()->m_currentHole);
			sTeamData* tb = TEAM(0);
			if (TEAM_HOLE_STROKE(tb, GOLFDOC()->m_currentHole) > stroke1)
			{
				sTeamData* tc = TEAM(0);
				unsigned char stroke0 =
					TEAM_HOLE_STROKE(tc, GOLFDOC()->m_currentHole);
				sTeamData* td = TEAM(1);
				if (stroke0 !=
					(unsigned char)((unsigned char)TEAM_HOLE_STROKE(td,
										GOLFDOC()->m_currentHole) +
						1))
					return false;
				unsigned char holes = Doc()->m_golfGame.holes;
				int done = GetHoleIndex(GOLFDOC()->m_currentHole);
				int need = TEAM(1)->score + holes - done;
				if (TEAM(0)->score <= need)
					return false;
			}
			winner = 1;
		}
		else
		{
			sTeamData* ta = TEAM(1);
			unsigned char stroke1 =
				TEAM_HOLE_STROKE(ta, GOLFDOC()->m_currentHole);
			sTeamData* tb = TEAM(0);
			if (TEAM_HOLE_STROKE(tb, GOLFDOC()->m_currentHole) < stroke1)
			{
				sTeamData* tc = TEAM(1);
				unsigned char stroke1b =
					TEAM_HOLE_STROKE(tc, GOLFDOC()->m_currentHole);
				sTeamData* td = TEAM(0);
				if (stroke1b !=
					(unsigned char)((unsigned char)TEAM_HOLE_STROKE(td,
										GOLFDOC()->m_currentHole) +
						1))
					return false;
				unsigned char holes = Doc()->m_golfGame.holes;
				int done = GetHoleIndex(GOLFDOC()->m_currentHole);
				int need = TEAM(0)->score + holes - done;
				if (TEAM(1)->score <= need)
					return false;
			}
			winner = 0;
		}
		sTeamData* team = TEAM(winner);
		team->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		RecordScoreByTeam(winner);
	}
	SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
	char pose[0x20];
	char other[0x20];
	char camera[0x20];
	char text[0x20];
	bool in[2];
	int win;
	in[0] = TEAM(0)->state == 1;
	in[1] = TEAM(1)->state == 1;
	unsigned char opp;
	if (in[0] && in[1])
	{
		opp = 1 - myTeam;
		unsigned char oppStroke =
			TEAM_HOLE_STROKE(TEAM(opp), GOLFDOC()->m_currentHole);
		unsigned char myStroke =
			TEAM_HOLE_STROKE(TEAM(myTeam), GOLFDOC()->m_currentHole);
		if (oppStroke < myStroke)
		{
lose:
			strcpy(pose,
				"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(other, "3\xb5\xee\xb8\xf0\xbc\xc7");
			strcpy(camera,
				"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
			strcpy(text, "lose");
			win = -1;
			goto send;
		}
		unsigned char oppStroke2 =
			TEAM_HOLE_STROKE(TEAM(opp), GOLFDOC()->m_currentHole);
		win = TEAM_HOLE_STROKE(TEAM(myTeam), GOLFDOC()->m_currentHole) !=
			oppStroke2;
	}
	else
	{
		if (in[myTeam] == false)
			goto lose;
		win = 1;
	}
	if (TEAM_HOLE_STROKE(TEAM(myTeam), GOLFDOC()->m_currentHole) ==
		(unsigned char)1)
	{
		strcpy(pose,
			"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
	}
	else
	{
		int diff = TEAM_HOLE_STROKE(TEAM(myTeam), GOLFDOC()->m_currentHole) -
			GOLFDOC()->GetHoleData().par;
		if (diff == -3)
			strcpy(pose,
				"\xbe\xcb\xb9\xd9\xc8\xa6\xc0\xce\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
		else if (diff == -2)
			strcpy(pose, "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
		else if (diff == -1)
			strcpy(pose, "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
		else
			strcpy(pose,
				"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
	}
	strcpy(other, "1\xb5\xee\xb8\xf0\xbc\xc7");
	if (win == 0)
		strcpy(other, "2\xb5\xee\xb8\xf0\xbc\xc7");
	strcpy(camera, pose);
	if (win == 0)
		strcpy(text, "draw");
	else
		strcpy(text, "win");
send:
	float* pos = &m_holePos983.x;
	if (*((unsigned char*)TEAM(myTeam) + 0x3ad) == MyIndex())
		SendMsg(this, "Player", M983(0x120), (int)pose, (int)other, (int)pos, 0);
	else
		SendMsg(this, "Player", M983(0x120), (int)other, (int)pose, (int)pos, 0);
	m_modeDuration = CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)0,
		(char*)camera, *(WVector*)pos);
	if (m_modeDuration < 0.0f)
	{
		WVector at(pos[0], pos[1] + 6.4f, pos[2]);
		WVector eye(pos[0], pos[1] + 6.4f, pos[2] - 32.0f);
		g_camera = MakeCameraMatrix(eye, at);
		m_modeDuration = 3.5f;
	}
	SendMsg(this, "Letter", M983(0x18d), (int)text, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1b7), 0, 0, 0, 0);
	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) &&
		(win == 0 || win == 1))
	{
		WSendPacket packet((enumClientPacket)0x35);
		packet.Encode1(
			*((unsigned char*)TEAM(PLAYER(MyIndex())->team) + 0x3b1));
		packet.Encode1(
			*((unsigned char*)TEAM(PLAYER(MyIndex())->team) + 0x3b2));
		packet.Send((eSendTo)0);
	}
	return true;
}

__int64 CGolfRule::CalcWinnersPang(unsigned long oid)
{
	__int64 pang = 0;
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (PLAYER(i)->state != 3 && PLAYER(i)->oid != oid)
		{
			__int64 bet = GOLFDOC()->m_skinsHolePot;
			__int64 userPang = Doc()->m_userInfo[i].stat.i64Pang;
			pang += bet < userPang ? bet : userPang;
		}
	}
	return GOLFDOC()->m_skinsWinnerBonus983 + pang;
}

void CGolfRule::CalcSkinsHolePang()
{
	unsigned long winner = (unsigned long)GOLFDOC()->m_skinsHoleWinner;
	if (winner == (unsigned long)-1)
	{
		int i = 0;
		if (GOLFDOC()->m_playerNum > 0)
		{
			do
			{
				unsigned char index = (unsigned char)i;
				if (PLAYER(index)->state != 3)
				{
					PLAYER(index)->skinsPang[GOLFDOC()->m_currentHole - 1] = -1;
					PLAYER(index)->skinsHolePang = 0;
					if (PLAYER(index)->state == 0)
						RecordScoreByPlayer(index);
				}
				i++;
			} while (i < GOLFDOC()->m_playerNum);
		}
		if (!GOLFDOC()->m_bNoStat)
		{
			GOLFDOC()->m_skinsCarryCount += 1;
			GOLFDOC()->m_pHoleData[GOLFDOC()->m_currentHole - 1].carry = 1;
			if (GOLFDOC()->m_currentHole !=
				HOLEORDER1_983(Doc()->m_golfGame.holes))
			{
				GOLFDOC()->GetHoleData().pang =
					GOLFDOC()->GetHoleData().basePang;
				unsigned char n = 0;
				unsigned char hole;
				do
				{
					hole = n + 1;
					if (*((unsigned char*)Doc() + (offsetof(CSharedDoc, m_holeOrder) + n)) ==
						(unsigned char)(GOLFDOC()->m_currentHole + 1))
						goto found;
					n = hole;
				} while (hole < 0x12);
				hole = 0x13;
found:
				__int64 base =
					GetHolePang(hole) + *(__int64*)GDOCPTR983(GOLFDOC(), 0x1b8);
				unsigned long mult = GOLFDOC()->m_skinsPotMult * 2;
				if (mult >= 8)
					mult = 8;
				GOLFDOC()
					->m_pHoleData[(unsigned char)(GOLFDOC()->m_currentHole +
									  1) -
						1]
					.carry = 1;
				GOLFDOC()
					->m_pHoleData[(unsigned char)(GOLFDOC()->m_currentHole +
									  1) -
						1]
					.pang = (__int64)mult * base;
			}
		}
	}
	else
	{
		__int64 pang = CalcWinnersPang(winner);
		int i = 0;
		if (GOLFDOC()->m_playerNum > 0)
		{
			do
			{
				unsigned char index = (unsigned char)i;
				if (PLAYER(index)->state != 3)
				{
					unsigned char hole = GOLFDOC()->m_bNoStat == 0
						? GOLFDOC()->m_currentHole
						: HOLEORDER1_983(Doc()->m_golfGame.holes);
					if (PLAYER(index)->oid == winner)
					{
						*(int*)((char*)PLAYER(index) + 0x4f4) += 1;
						PLAYER(index)->skinsHolePang = pang;
						PLAYER(index)->skinsPang[hole - 1] = (int)pang;
						PLAYER(index)->skinsSumPang += pang;
						Doc()->m_userInfo[i].stat.i64Pang += pang;
					}
					else
					{
						PLAYER(index)->stroke[GOLFDOC()->m_currentHole - 1] +=
							1;
						__int64 lost = Doc()->m_userInfo[i].stat.i64Pang;
						if (GOLFDOC()->m_skinsHolePot < lost)
							lost = GOLFDOC()->m_skinsHolePot;
						PLAYER(index)->skinsHolePang = -lost;
						PLAYER(index)->skinsPang[hole - 1] = (int)-lost;
						PLAYER(index)->skinsSumPang -= lost;
						Doc()->m_userInfo[i].stat.i64Pang -= lost;
					}
					if (PLAYER(index)->state == 0)
						RecordScoreByPlayer(index);
				}
				i++;
			} while (i < GOLFDOC()->m_playerNum);
		}
		GOLFDOC()->m_skinsCarryCount = 0;
		GOLFDOC()->m_skinsHolePot = 0;
		GOLFDOC()->m_skinsWinnerBonus983 = 0;
	}
}

bool CGolfRule::SetHoleInPose_Skins()
{
	char pose[0x20];
	char caddie[0x20];
	char camera[0x20];
	unsigned long flag;
	int result;
	const char* bgm;
	if (Doc()->m_golfGame.gameType == 7)
	{
		unsigned long winner = (unsigned long)GOLFDOC()->m_skinsHoleWinner;
		flag = 1;
		if (winner == (unsigned long)-1)
		{
			strcpy(camera,
				"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			result = 0;
			if (GOLFDOC()->m_skinsCarryCount < 3)
				strcpy(pose,
					"\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
			else
			{
				strcpy(pose, "2\xb5\xee\xb8\xf0\xbc\xc7");
				flag = 0;
			}
			strcpy(caddie, "\xc8\xa6\xc0\xce");
			bgm = "skins_draw.mp3";
		}
		else
		{
			unsigned char me = MyIndex();
			result = me;
			if (winner == PLAYER(me)->oid)
			{
				result = 1;
				if (GOLFDOC()->m_skinsCarryCount < 2)
					strcpy(pose,
						"\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
				else if (GOLFDOC()->m_skinsCarryCount < 4)
				{
					strcpy(pose, "1\xb5\xee\xb8\xf0\xbc\xc7");
					flag = 0;
				}
				else
					strcpy(pose,
						"\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
				if (GOLFDOC()->m_skinsCarryCount < 3)
					strcpy(caddie, "\xc8\xa6\xc0\xce");
				else
					strcpy(caddie, "\xc8\xa6\xc0\xce\xbf\xf8");
				if (GOLFDOC()->m_skinsCarryCount < 3)
					strcpy(camera,
						"\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
				else
					strcpy(camera,
						"\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee");
				bgm = "skins_win.mp3";
			}
			else
			{
				result = -1;
				if (GOLFDOC()->m_skinsCarryCount < 3)
					strcpy(pose,
						"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
				else
				{
					strcpy(pose, "3\xb5\xee\xb8\xf0\xbc\xc7");
					flag = 0;
				}
				if (GOLFDOC()->m_skinsCarryCount < 3)
					strcpy(caddie, "\xbd\xc7\xb0\xdd");
				else
					strcpy(caddie, "\xba\xa1\xc4\xbf");
				strcpy(camera,
					"\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee");
				bgm = "skins_lose.mp3";
			}
		}
		SetConstantBGM(bgm, true, true);
		float* pos = &m_holePos983.x;
		SendMsg(this, "Player", M983(0x122), (int)pos, (int)pose, flag, 0);
		WVector ground = GetPVS().GetGroundPoint(
			WVector(pos[0] + 10.0f, pos[1], pos[2] + 10.0f), false, NULL,
			false);
		SendMsg(this, "Caddie", M983(0x121), (int)&ground, (int)caddie, 0, 0);
		m_modeDuration = CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)0,
			(char*)camera, *(WVector*)pos);
		if (m_modeDuration < 0.0f)
		{
			WVector at(pos[0], pos[1] + 6.4f, pos[2]);
			WVector eye(pos[0], pos[1] + 6.4f, pos[2] - 32.0f);
			g_camera = MakeCameraMatrix(eye, at);
			m_modeDuration = 3.5f;
		}
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		__int64 pang = CalcWinnersPang(Doc()->m_myInfo.info.dwGuid);
		SendMsg(this, "Screen", M983(0x1c7), result, (int)pang, 0, 0);
		SendMsg(this, "Screen", M983(0x1c8), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1b7), 0, 0, 0, 0);
	}
	return true;
}

void CGolfRule::DoFlyBall(float time)
{
	if (m_bContinueDlg || m_bNetMsgWait)
		return;
	unsigned char shotType = (unsigned char)GolfClub().PowerShotType();
	if (shotType == 2 || shotType == 3)
	{
		if (*(char*)(*(int*)((char*)&GolfClub() + 4) + 0x22) == 0 &&
			m_bSpecialCamDone983 == 0)
		{
			CCameraWork* camera = CCameraWork::Instance();
			if (camera->Finished())
			{
				camera->SelectCamera(-1);
				m_bSpecialCamDone983 = 1;
			}
		}
	}
	if ((m_replayFlags & 2) == 0)
	{
		if (g_input->Get("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) ==
				0 &&
			m_replayState == 0)
			CCameraWork::Instance()->Unlock();
	}
	WPolySoupEx2* soup = (WPolySoupEx2*)GOLFDOC()->m_pPolySoup;
	CCameraWork* work = CCameraWork::Instance();
	WPolySoupEx2::sArea* area;
	area = soup->GetAreaByPos(GolfBall().m_pos);
	if (area != NULL && m_pCamArea983 != area)
	{
		char camName[64];
		WSliceStr slice(area->name);
		slice.Get("*cam", "%s", camName);
		WPolySoup::sCamera* cam = soup->GetCameraByName(camName);
		if (cam != NULL)
			work->SetInstalledCamera((const WVector*)((char*)cam + 0x44));
		m_pCamArea983 = area;
	}
	else
	{
		area = soup->GetAreaByPos(GolfBall().m_pos);
		if (area == NULL)
		{
			m_pCamArea983 = NULL;
			if (work->m_cameraType == 0x15) // B1467: era +0x8c cru
				work->UninstallCamera();
		}
	}
	GolfBall();
	if (GolfBall().m_state != 0)
		DoFlyBall_Flying(time);
	else
		DoFlyBall_Stopped(time);
}

void CGolfRule::DoFlyBall_Flying(float dt)
{
	static unsigned long s_powerSpinFx = 0xffffffff;

	CGolfBall& ball = GolfBall();
	m_modeDuration = dt + m_modeDuration;
	// 983: sem o bloco do corte do 645 (m_bFlagCutin + "S5::mode_flyBall" + 0x20 ao "Cutin"): a string nao existe no exe e
	// a flag nao e mais ligada (C23)
	if (0.02f <= m_modeDuration)
	{
		int steps = (int)(m_modeDuration * 50.0f);
		if (ball.m_renderNum < steps + ball.m_playFrame)
			steps = ball.m_renderNum - ball.m_playFrame;
		bool first = true;
		for (int i = 0; i < steps; i++)
		{
			((RbTask*)GetTask())->Step(1, 0.02f, ball.m_playFrame + 1 + i);
			int index = ball.m_playFrame + 1 + i;
			const CGolfBall::sExpected* e = ball.PreData(index);
			if (IsMyTurn(false) && e->event != 0)
				SendMsg(this, "ShotReview", 0x2fd, (int)"RECORD_REAL_BOUNCE_BALL",
					(int)e, 0, 0);
			if (e->event != 0 && e->event == 6)
			{
				SendMsg(this, "MagicCarpetHelper", M983(0x280), 0, 0, 0, 0);
				((CFxEx*)CFx::Instance())->MakeBoosterWarp(&ball, 6);
			}
			if (first && (first = false, e->event != 0))
			{
				ball.m_special &= 0xffffff8f;
				int msg = 0;
				int arg = 0;
				switch (e->effect)
				{
				case 1:
					msg = M983(0x173);
					break;
				case 2:
					if (30.0f < sqrtf(e->vel.z * e->vel.z +
									e->vel.y * e->vel.y + e->vel.x * e->vel.x))
					{
						arg = ball.m_playFrame + 1 + i;
						msg = M983(0x174);
					}
					break;
				case 3:
					msg = M983(0x171);
					break;
				case 4:
					msg = M983(0x176);
					break;
				}
				if (msg != 0)
					SendMsg(this, "Effect", msg, arg, 0, 0, 0);
				if (e->event == 2)
					SendMsg(this, "Gallery", M983(0x102), 0, 0, 0, 0);
				if (e->event == 5)
				{
					((CFxEx*)CFx::Instance())->MakeBoosterWarp(&ball, 0);
					if (Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall == 0x14000005)
						SendMsg(this, "Ball", M983(0x1ee), 0, 0, 0, 0);
				}
				if (ball.m_playFrame + 1 + i == ball.m_greenFrame)
					SendMsg(this, "Gallery", M983(0x101), 0, 0, 0, 0);
				if (*((unsigned char*)GRPTR983(0x3dc)) != 0)
					g_audio->StopSfx(GRPTR983(0x3dc));
				const char* sound = e->sound;
				if (*sound != 0 && !g_audio->IsPlaying(sound))
				{
					WVector pos(ball.m_pos.x - g_camera.xm,
						ball.m_pos.y - g_camera.ym, ball.m_pos.z - g_camera.zm);
					g_audio->PlaySfx(sound, &pos, 5, sound, NULL, 0.5f, 200.0f);
				}
			}
			if (ball.m_playFrame + 1 + i == ball.m_nearObjFrame)
				SendMsg(this, "Gallery", M983(0x10d), 0, 0, 0, 0);
			if (!ball.IsHoleIn() &&
				ball.m_playFrame + 1 + i == ball.m_holeOutFrame)
				SendMsg(this, "Gallery", M983(0x109), 0, 0, 0, 0);
			int slide = ball.m_vectorSlideFrame;
			int back = slide;
			if (slide > 0x4f)
				back = 0x50;
			if (slide - back == ball.m_playFrame + 1 + i)
				SendMsg(this, "MagicCarpetHelper", M983(0x27e), 0, 0, 0, 0);
			if (ball.m_playFrame + 1 + i == ball.m_rollFrame)
				SendMsg(this, "MagicCarpetHelper", M983(0x27f), 0, 0, 0, 0);
			if (ball.m_vectorSlideFrame == ball.m_playFrame + 1 + i)
				SendMsg(this, "MagicCarpetHelper", M983(0x27d), 0, 0, 0, 0);
		}
		int next = steps + ball.m_playFrame;
		if (ball.m_renderNum < next)
			next = ball.m_renderNum;
		ball.m_playFrame = next;
		m_modeDuration = m_modeDuration - (float)steps * 0.02f;
		if (ball.m_state == 3)
		{
			const CGolfBall::sExpected* cur = ball.PreData(ball.m_playFrame);
			unsigned char ground = GetPVS().GetGroundType(cur->pos, true, NULL);
			if (ground == 2)
			{
				int after = steps + ball.m_playFrame;
				if (ball.m_renderNum < after)
					after = ball.m_renderNum;
				const CGolfBall::sExpected* nx = ball.PreData(after);
				ground = GetPVS().GetGroundType(nx->pos, true, NULL);
				if (ground != 2)
					SendMsg(this, "Gallery", M983(0x10a), 0, 0, 0, 0);
			}
		}
		if (ball.m_afterBurnerFrame > 0 &&
			ball.m_afterBurnerFrame < ball.m_playFrame &&
			ball.m_playFrame < ball.m_afterBurnerFrame + 0x32)
		{
			g_audio->PlaySfx("spin_super");
			if (((CFxEx*)CFx::Instance())->FindControl(s_powerSpinFx) == NULL)
			{
				CItemSoundTable::Instance().PlaySound(
					Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall, 1, -1);
				s_powerSpinFx = ((CFxEx*)CFx::Instance())->MakePowerSpin(&ball);
			}
		}
		if (ball.m_playFrame == ball.m_renderNum)
		{
			if (ball.m_water != 0)
			{
				if (GetCurMap() == 13)
				{
					CFxSequence* seq =
						CFx::Instance()->OpenSequence("hell_ob.seq", true);
					if (seq != NULL)
						seq->m_pos = GolfBall().m_pos; // B1556: era visao crua VC10_FXSEQ_POS
				}
				else
					SendMsg(this, "Effect", M983(0x176), 0, 0, 0, 0);
			}
			m_modeDuration = 0.0f;
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			ball.m_state = CGolfBall::BALL_STOP;
			g_audio->StopSfx("shot_dist_count");
			SendMsg(this, "Screen", M983(0x1ca), 0, 0, 0, 0);
			if (IsMyTurn(false))
				SendMsg(this, "ShotReview", 0x2fd, (int)"SAVE_BALL_TRACE",
					(int)&GolfBall(), 0, 0);
		}
		if ((m_replayFlags & 2) != 0 && 15.000001f < (float)ball.m_playFrame)
		{
			m_replayState = m_replayState + 1;
			if (m_replayState == 3)
			{
				m_replayFlags = m_replayFlags & 0xfffffffd;
				m_replayState = 0;
				ReactionForShot();
				if (g_input->Get("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
						true) != 0)
					g_camera = ((CGolfDocEx2*)GOLFDOC())->GetDefaultCamera();
				CCameraWork::Instance()->SelectCamera(-1);
			}
			else
				SwingReplay();
		}
	}
	if (*(char*)(*(int*)((char*)&GolfClub() + 4) + 0x22) != 3 &&
		(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall & 0x3ffffff) ==
			0x20 &&
		m_bRainbowFx983 == 0 &&
		ball.m_topFrame < ball.m_playFrame)
	{
		CFxSequence* seq =
			CFx::Instance()->OpenSequence("rainbow_002.seq", true);
		if (seq != NULL)
			seq->m_pos = ball.m_pos; // B1556: era visao crua VC10_FXSEQ_POS
		m_bRainbowFx983 = 1;
	}
	int frame = ball.m_playFrame;
	float t;
	if (frame < ball.m_renderNum && GOLFDOC()->m_gameMode == 0x80)
	{
		if (m_modeDuration <= 0.0f)
			t = 1.0f;
		else
			t = (0.02f - m_modeDuration) * 50.0f;
		float a = 1.0f - t;
		const CGolfBall::sExpected* p1 = ball.PreData(frame + 1);
		WVector pn(a * p1->pos.x, a * p1->pos.y, a * p1->pos.z);
		const CGolfBall::sExpected* p0 = ball.PreData(frame);
		ball.m_pos.x = t * p0->pos.x + pn.x;
		ball.m_pos.y = t * p0->pos.y + pn.y;
		ball.m_pos.z = t * p0->pos.z + pn.z;
		p1 = ball.PreData(frame + 1);
		WVector vn(a * p1->vel.x, a * p1->vel.y, a * p1->vel.z);
		p0 = ball.PreData(frame);
		ball.m_vel.x = t * p0->vel.x + vn.x;
		ball.m_vel.y = t * p0->vel.y + vn.y;
		ball.m_vel.z = t * p0->vel.z + vn.z;
		frame = ball.m_playFrame;
		p1 = ball.PreData(frame + 1);
		float cr = a * p1->curveRot;
		float sr = a * p1->spinRot;
		p0 = ball.PreData(frame);
		ball.m_curveRot = t * p0->curveRot + cr;
		ball.m_spinRot = t * p0->spinRot + sr;
	}
	else
	{
		t = 0.0f;
		const CGolfBall::sExpected* e = ball.PreData(frame);
		ball.m_pos = e->pos;
		e = ball.PreData(frame);
		ball.m_vel = e->vel;
		e = ball.PreData(ball.m_playFrame);
		ball.m_curveRot = e->curveRot;
		ball.m_spinRot = e->spinRot;
	}
	if (*(char*)(*(int*)((char*)&GolfClub() + 4) + 0x22) != 3 &&
		GetCurMap() == 10 && !m_bTrajectoryMsgSent && ball.m_rollFrame <= ball.m_playFrame)
	{
		m_bTrajectoryMsgSent = true;
		SendMsg(this, "Trajectory", M983(0x15d), 0, 0, 0, 0);
	}
	if ((m_replayFlags & 2) == 0 || m_replayState != 0)
		CCameraWork::Instance()->Process(dt, 1.0f - t);
}

bool CGolfRule::GetShotCntData()
{
	CSharedDoc* doc = Doc(); // B1475: +0x110c0/+0x11224/+0x1127c crus do 983 -> membros (andam no cl novo)
	int count = (int)doc->m_shotDataList.size();
	int i = 1;
	if (doc->m_replayState == 0xe)
	{
		doc->m_replayShotIndex = 0;
		return true;
	}
	if (count < doc->m_replayShotIndex)
		doc->m_replayShotIndex = count;
	else if (doc->m_replayShotIndex != count)
		doc->m_replayShotIndex = doc->m_replayShotIndex + 1;
	std::list<sShotData>& list = Doc()->m_shotDataList;
	for (std::list<sShotData>::iterator it = list.begin(); it != list.end();
		it++, i++)
	{
		if (i != Doc()->m_replayShotIndex)
			continue;
		memcpy(&m_shotData, &*it, sizeof(sShotData));
		Wind().SetWind(m_shotData.wind);
		Doc()->m_replayHoleStroke = m_shotData.holeStroke;
		sPlayerData* player = PLAYER(0);
		player->gauge = m_shotData.gauge;
		unsigned long pvsFrame = m_shotData.pvsFrame;
		CQuadTree& pvs = GetPVS();
		pvs.m_frame = pvsFrame;
		*(unsigned long*)&pvs.m_frameTime = 0;
		if (Doc()->m_replayShotIndex == 0)
		{
			PLAYER(0)->bonusPang = 0;
			PLAYER(0)->totalPang = 0;
		}
		else
		{
			PLAYER(0)->bonusPang = m_shotData.bonusPang;
			PLAYER(0)->totalPang = m_shotData.pang;
		}
		// B1349: 983 (0068045f..00680494) avisa o "Gateway" (0x753b, sub 0xc) com o indice da tacada - 1
		AfxGetTask()->GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0xc, i - 1, 0, 0, 0);
		break;
	}
	return true;
}

// JP 983 00673e10 (fastcall: ecx = tacadas, edx = tacadas - par): som do buraco. Primeiro a tabela de sons da bola
// (CItemSoundTable, codigo 0x20 se foi 1 tacada, senao por diff: -3 0x10, -2 0x800, -1 8, 0 0x40, 1..4 0x80), depois a voz do
// taco (CVoiceItem) e so entao o efeito "Birdie"/"par"/"Bogey". O 983 nao tem o t_Ex7 (bola 0x14000004) do 645.
static void HoleInSound983(int stroke, int diff)
{
	int code = 0;
	unsigned long wave = 0;
	if (stroke == 1)
	{
		code = 0x20;
		wave = 0x400;
	}
	else
	{
		switch (diff)
		{
		case -3: code = 0x10;  wave = 0x800;   break;
		case -2: code = 0x800; wave = 0x1000;  break;
		case -1: code = 8;     wave = 0x2000;  break;
		case 0:  code = 0x40;  wave = 0x4000;  break;
		case 1:  code = 0x80;  wave = 0x8000;  break;
		case 2:  code = 0x80;  wave = 0x10000; break;
		case 3:  code = 0x80;  wave = 0x20000; break;
		case 4:  code = 0x80;  wave = 0x40000; break;
		}
	}
	unsigned char player = GOLFDOC()->m_currentPlayer;
	if (CItemSoundTable::Instance().PlaySound(Doc()->m_userInfo[player].userEquip.tidBall, code, -1))
		return;
	if (CVoiceItem::Instance()->PlaySoundA(Doc()->m_userInfo[player].clubInfo.tid, (CVoiceItem::eWaveState)wave, -1))
		return;
	if (diff < 0)
		g_audio->PlaySfx("Birdie", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	else if (diff == 0)
		g_audio->PlaySfx("par", NULL, 0, NULL, NULL, 0.5f, 200.0f);
	else
		g_audio->PlaySfx("Bogey", NULL, 0, NULL, NULL, 0.5f, 200.0f);
}

// JP 983 00674000 (fastcall: ecx = codigo na tabela de sons da bola, edx = onda da voz do taco): primeiro o CItemSoundTable da
// bola; se nao tocou, a voz do taco (CVoiceItem).
static void BallSound983(int code, unsigned long wave)
{
	unsigned char player = GOLFDOC()->m_currentPlayer;
	if (CItemSoundTable::Instance().PlaySound(Doc()->m_userInfo[player].userEquip.tidBall, code, -1))
		return;
	CVoiceItem::Instance()->PlaySoundA(Doc()->m_userInfo[player].clubInfo.tid, (CVoiceItem::eWaveState)wave, -1);
}

void CGolfRule::DoFlyBall_Stopped(float dt)
{
	m_bTrajectoryMsgSent = false;
	m_pSubRule983->OnShotEnd(); // 983 (006924aa); a ReplayTape marca o +0x2c0
	// JP 983 (00692583..0069262c): no tipo 0xb a bola parada so anda a camera; passados 2 s, estatistica, pacote 0x1c
	// ao servidor de jogo e modo 0x1000000. Este e o terceiro 0x1c do 983 (os outros dois sao os do jogo em massa abaixo).
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		CCameraWork::Instance()->Process(dt, 0.0f);
		dt = m_modeDuration + dt;
		m_modeDuration = dt;
		if (dt > 2.0f)
		{
			m_modeDuration = 0.0f;
			UpdateShotResultStat(); // 983: 00675e00
			WSendPacket packet((enumClientPacket)0x1c);
			EncodePacket((enumClientPacket)0x1c, packet);
			packet.Send(TO_GAME);
			ChangeGameMode(0x1000000);
		}
		return;
	}
	CGolfBall& ball = GolfBall();
	m_bReplayReady = 1;
	// 983 (00692480): sem o bloco de estados da fita aqui; ele vive so em CSubGolfRuleReplayTape::OnShotEnd (m_pSubRule983, acima)
	unsigned char player = GOLFDOC()->m_currentPlayer;
	int stroke;
	if (Doc()->m_golfGame.gameType == 1)
	{
		player = GOLFDOC()->m_currentPlayer;
		sPlayerData* p = PLAYER(player);
		player = *((unsigned char*)p + 0x4e9);
		sTeamData* team = TEAM(player);
		stroke = TEAM_HOLE_STROKE(team, GOLFDOC()->m_currentHole);
	}
	else
	{
		player = GOLFDOC()->m_currentPlayer;
		sPlayerData* p = PLAYER(player);
		stroke = PLAYER_NEXT_STROKE(p, GOLFDOC()->m_currentHole);
	}
	bool overMode = !((unsigned long)GOLFDOC()->m_tutorialMode < 8 && GOLFDOC()->m_tutorialMode != 4 &&
		GOLFDOC()->m_tutorialMode != 10);
	if (ball.IsHoleIn() && overMode && Doc()->m_golfGame.gameType != 10)
	{
		if (m_bShotInFlight != 0)
		{
			m_bShotInFlight = 0;
			WPuppet* puppet = SCENEMGR()->GetPuppet("\xb0\xf8");
			if (puppet && puppet->m_rootbone)
				puppet->SetAlpha(0xff);
			CCameraWork::Instance()->StopBallSound();
			SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
			g_audio->StopSfx("shot_dist_count");
			if (m_bPuttTutorial983 != 0)
				SendMsg(this, "Player", 0x2716,
					(int)"\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", 0, 0, 0);
			// 983 00692480: sem o efeito asteca da bola (o 645 testava 0x55; o 983 nunca chama o PlayEffect)
			if ((m_replayFlags & 1) == 0)
			{
				if (IsMassGame() != 0 &&
					GOLFDOC()->m_currentHole ==
						Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] &&
					m_bTimeOutSent == 0)
				{
					m_bTimeOutSent = 1;
					WSendPacket packet((enumClientPacket)0x1c);
					EncodePacket((enumClientPacket)0x1c, packet);
					packet.Send(TO_GAME);
				}
				SendMsg(this, "Effect", M983(0x179), 0, 0, 0, 0);
				int diff = stroke - GOLFDOC()->GetHoleData().par;
				GetWinningPrize(diff);
				if (diff <= 0)
				{
					int coin = ((diff >= 0) - 1 & 7) + 6;
					if ((Doc()->m_serverProperty >> 7 & 1) != 0)
						coin = S5::UTIL::GetHoleinCoinCount(coin, 2);
					if (Doc()->m_golfGame.gameType == 7 &&
						*(short*)GDOCPTR983(GOLFDOC(), 0x1a8) == 2)
					{
						m_holeInCoinFxId = 0x1e;
						m_holeInCoinFxTime = 0.1f;
						m_holeInCoinFxCount = (int)GOLFDOC()->m_skinsCarryCount;
					}
					else
						SendMsg(this, "Effect", M983(0x17d), coin, 0, 0, 0);
				}
				HoleInSound983(stroke, diff); // 983 00673e10
				if (Doc()->m_golfGame.gameType == 1) // 983 (00692990): so o tipo 1; o 645 tambem tratava o 3
					SetHoleInPose(true);
			}
			else
			{
				m_replayState = m_replayState + 1;
				int state = m_replayState;
				m_bShotInFlight = 1;
				if (state == 1)
				{
					CCameraWork::Instance()->m_bLock = 1; // B1465: era +0x20c cru
					if (IsMassGame() != 0 &&
						GOLFDOC()->m_currentHole ==
							Doc()->m_holeOrder[Doc()->m_golfGame.holes - 1] &&
						m_bTimeOutSent == 0)
					{
						m_bTimeOutSent = 1;
						WSendPacket packet((enumClientPacket)0x1c);
						EncodePacket((enumClientPacket)0x1c, packet);
						packet.Send(TO_GAME);
					}
					HoleReplay();
					return;
				}
				if (state == 2)
				{
					HoleReplay();
					return;
				}
				if (state == 3)
				{
					m_replayFlags &= 0xfffffffe;
					CCameraWork::Instance()->Unlock();
				}
			}
		}
		float timer = m_modeDuration;
		m_modeDuration = dt + timer;
		float limit = 2.0f;
		if (Doc()->m_golfGame.gameType == 1)
			limit = 3.0f;
		if (dt + timer <= limit)
			goto done;
		char ready;
		if (GOLFDOC()->m_tutorialMode == 0xf)
		{
			SendMsg(this, "Screen", M983(0x1b8), (int)&ready, 0, 0, 0);
			if (ready == 0)
			{
				float wait = 6.0f;
				if (Doc()->m_golfGame.gameType == 1)
					wait = 7.0f;
				if (m_modeDuration <= wait)
					goto done;
			}
		}
		else
			ready = 1;
		if ((m_replayFlags & 4) != 0)
		{
			m_replayFlags &= 0xfffffffb;
			Replay(false);
			goto done; // 983 (006926f0..0069301f): 00692260 volta ao fim comum, que roda CCameraWork::Process
		}
		*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
		UpdateShotResultStat();
		if (GOLFDOC()->m_tutorialMode == 4)
		{
			if (m_bTutorialShot983 == 0)
			{
				SendMsg(this, "GolfRule", M983(0x88), 1, 0, 0, 0);
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial)
					tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
			}
			else
				ChangeGameMode(0x800);
			goto done;
		}
		if (!IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) ||
			Doc()->m_playingMode != 2)
			RecordScoreByPlayer(GOLFDOC()->m_currentPlayer);
	}
	else
	{
		if (m_bShotInFlight != 0)
		{
			m_bShotInFlight = 0;
			WPuppet* puppet = SCENEMGR()->GetPuppet("\xb0\xf8");
			if (puppet && puppet->m_rootbone)
				puppet->SetAlpha(0xff);
			g_audio->StopSfx("shot_dist_count");
			SendMsg(this, "NPC", M983(0x183), 0, 0, 0, 0);
			if (m_bPuttTutorial983 != 0)
				SendMsg(this, "Player", 0x2716,
					(int)"\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", 0, 0, 0);
			WVector pin = GOLFDOC()->GetHoleData().pin;
			const WVector* rest = ball.m_renderNum < 0
				? NULL
				: (const WVector*)((char*)&ball + 0xcc);
			WVector toPin(rest->x - pin.x, 0.0f, rest->z - pin.z);
			player = GOLFDOC()->m_currentPlayer;
			sPlayerData* p = PLAYER(player);
			if ((p->shotFlag & 1) == 0 && !GolfBall().IsHoleIn() &&
				m_bPuttTutorial983 == 0)
			{
				const WVector* at = ball.m_renderNum < 0
					? NULL
					: (const WVector*)((char*)&ball + 0xcc);
				unsigned char ground = GetPVS().GetGroundType(*at, true, NULL);
				if (ground == 2 &&
					(GOLFDOC()->m_remainTime < 10000 ||
						toPin.Magnitude() < 32.0f ||
						(unsigned char)PLAYER_NEXT_STROKE(
							PLAYER(GOLFDOC()->m_currentPlayer),
							GOLFDOC()->m_currentHole + 0x12) > 1))
				{
					sPlayerData* q = PLAYER(GOLFDOC()->m_currentPlayer);
					q->shotFlag |= 1;
					SendMsg(this, "Player", 0x2717,
						(int)"\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
						0, 0, 0);
				}
			}
			WVector holePos = GOLFDOC()->GetHoleData().pin;
			WVector delta(holePos.x - ball.m_pos.x, holePos.y - ball.m_pos.y,
				holePos.z - ball.m_pos.z);
			float dist = sqrtf(delta.z * delta.z + delta.x * delta.x);
			unsigned char ground =
				GetPVS().GetGroundType(ball.m_pos, true, NULL);
			if ((ground == 3 || ground == 0xe) && !ball.m_bOB) // 983 00692c49: so se a bola nao saiu
			{
				SendMsg(this, "Gallery", M983(0x106), 0, 0, 0, 0);
				BallSound983(0x200, 0x80);
			}
			else if (stroke < 3)
			{
				ground = GetPVS().GetGroundType(ball.m_pos, true, NULL);
				if (ground == 2)
				{
					if (dist <= 22.4f)
						SendMsg(this, "Gallery", M983(0x10b), 0, 0, 0, 0);
					else
						SendMsg(this, "Gallery", M983(0x100), 0, 0, 0, 0);
				}
			}
			if (*((char*)(*(int*)((char*)&GolfClub() + 4)) + 0x22) == 3 &&
				dist < 3.2f && ball.m_holeFrame < 0)
				SendMsg(this, "Gallery", M983(0x104), 0, 0, 0, 0);
			if (!ball.m_bOB)
			{
				if (ball.m_standGround == 0)
				{
					m_replayFlags &= 0xfffffffb;
					if (GOLFDOC()->m_bReplaying == 0)
					{
						player = GOLFDOC()->m_currentPlayer;
						PLAYER(player)->flag++; // 983 00692e4c/00692d60: +0x4e8 (o nosso contava no +0x4dc da desistencia)
						player = GOLFDOC()->m_currentPlayer;
						sPlayerData* q = PLAYER(player);
						q->stroke[GOLFDOC()->m_currentHole - 1] += 1; // 983 00692480: +0x1c + buraco*0x10
						if (Doc()->m_golfGame.gameType == 1)
						{
							sPlayerData* r = PLAYER(GOLFDOC()->m_currentPlayer);
							TEAM(*((unsigned char*)r + 0x4e9))
								->stroke[GOLFDOC()->m_currentHole - 1] += 1;
						}
					}
					g_audio->PlaySfx("ball_OB_area", NULL, 0, NULL, NULL, 0.5f,
						200.0f);
					BallSound983(0x100, 0x10); // 983 00674070
					if (Doc()->m_golfGame.gameType != 10)
						SendMsg(this, "Letter", M983(0x18d), (int)"Retry", 0, 0, 0);
				}
				else
				{
					player = GOLFDOC()->m_currentPlayer;
					PLAYER(player)->flag = 0; // 983 00692f0a: +0x4e8
				}
			}
			else
			{
				if (GOLFDOC()->m_bReplaying == 0)
				{
					player = GOLFDOC()->m_currentPlayer;
					PLAYER(player)->flag++; // 983 00692e4c/00692d60: +0x4e8 (o nosso contava no +0x4dc da desistencia)
					player = GOLFDOC()->m_currentPlayer;
					sPlayerData* q = PLAYER(player);
					q->stroke[GOLFDOC()->m_currentHole - 1] += 1;
					if (Doc()->m_golfGame.gameType == 1)
					{
						player = GOLFDOC()->m_currentPlayer;
						sPlayerData* r = PLAYER(player);
						TEAM(*((unsigned char*)r + 0x4e9))
							->stroke[GOLFDOC()->m_currentHole - 1] += 1; // 983 00692dc6: mesmo indice do ramo sem OB
					}
				}
				if (GolfBall().m_water == 3 || GolfBall().m_water == 2)
				{
					SendMsg(this, "Letter", M983(0x18d),
						(int)"\xbf\xf6\xc5\xcd\xc7\xd8\xc0\xfa\xb5\xe5", 0, 0,
						0);
					g_audio->PlaySfx("ball_fall_into_water", NULL, 0, NULL,
						NULL, 0.5f, 200.0f);
					BallSound983(0x400, 0x100); // 983 00679dd3
				}
				else
				{
					SendMsg(this, "Letter", M983(0x18d), (int)"OB", 0, 0, 0);
					if (GolfBall().m_water == 1)
					{
						g_audio->PlaySfx("ball_fall_into_water", NULL, 0, NULL,
							NULL, 0.5f, 200.0f);
						BallSound983(0x400, 0x100);
					}
					else
					{
						g_audio->PlaySfx("ball_OB_area", NULL, 0, NULL, NULL,
							0.5f, 200.0f);
						BallSound983(0x100, 0x10);
					}
				}
				// 983 00679c50: no mapa 0x12, bola abaixo do fundo do modelo base (aabb.min.y) = 0x30d (buraco negro)
				if (GetCurMap() == 0x12 && GetPVS().m_pHolder->aabb.min.y > ball.m_pos.y)
					SendMsg(this, "Gallery", 0x30d, 0, 0, 0, 0);
				else
					SendMsg(this, "Gallery", M983(0x105), 0, 0, 0, 0);
			}
			if (m_pRuleBase != NULL && m_pRuleBase->CheckGiveUp() &&
				Doc()->m_golfGame.gameType == 1) // 983 00692f21: so o tipo 1 (o 645 tambem tratava o 3)
				SetHoleInPose(true);
			if (Doc()->m_golfGame.gameType == 10)
			{
				unsigned char a = MyIndex();
				unsigned char b = MyIndex();
				*(unsigned long*)(SHAREDDOC_ULONG(0x1042c) + b * 0x1b8 + 0x18c) =
					*(unsigned long*)(SHAREDDOC_ULONG(0x1042c) + a * 0x1b8 +
						0x190); // 983 00692f69
				SendMsg(this, "Score", M983(0x2a), 1, 0, 0, 0);
			}
		}
		if (ball.m_water != 0)
		{
			float radius = *(float*)((char*)&ball + 200);
			float drop = dt * 3.2f;
			float below = ball.m_pos.y - drop;
			WVector ground =
				GetPVS().GetGroundPoint(ball.m_pos, true, NULL, false);
			if (radius + ground.y < below)
				ball.m_pos.y = ball.m_pos.y - drop;
		}
		float elapsed = dt + m_modeDuration;
		m_modeDuration = elapsed;
		if (!ball.m_bOB)
		{
			if (elapsed <= 3.5f)
				goto done;
		}
		else if (elapsed <= 2.0f)
		{
			if (!ball.m_bOB && elapsed <= 3.5f)
				goto done;
			goto done;
		}
		char ready;
		if (GOLFDOC()->m_tutorialMode == 0xf)
		{
			SendMsg(this, "Screen", M983(0x1b8), (int)&ready, 0, 0, 0);
			if (ready == 0)
			{
				float wait = 7.5f;
				if (ball.m_bOB)
					wait = 6.0f;
				if (m_modeDuration <= wait)
					goto done;
			}
		}
		else
			ready = 1;
		if ((m_replayFlags & 4) != 0)
		{
			m_replayFlags &= 0xfffffffb;
			Replay(false);
			goto done; // 983 00691f60 retorna ao 00692480, que roda CCameraWork::Process (0069301f)
		}
		if (m_bCancelShotAsk)
		{
			if (m_pForgetDlg == NULL)
			{
				m_pForgetDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"flowerofforgetting", NULL);
				((FrForm*)m_pForgetDlg)
					->SetMessage( // 983 00691f60: texto via K2L_Compatibility (00b4ec60)
						K2L_Compatibility("\xb4\xe7\xbd\xc5\xc0\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb8\xc1\xb0\xa2\xc8\xad\xb4\xc2 \\c0xffff0000\\c"
						"\xb3\xaa\xbb\xdb \xb1\xe2\xbe\xef\xc0\xbb \xc1\xf6\xbf\xef \xbc\xf6 \\c0xff000000\\c\xc0\xd6\xb4\xc2 "
						"\xb4\xc9\xb7\xc2\xc0\xcc \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9. \\c0xffff0000\\c\xb8\xc1\xb0\xa2\xc8\xad"
						"\\c0xff000000\\c\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9 \xc0\xdf\xb8\xf8\xb5\xc8 \xbc\xa6\xc0\xbb "
						"\xb5\xc7\xb5\xb9\xb8\xae\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
						false);
				WPoint pos;
				pos.x = 0.0f;
				pos.y = 0.0f;
				((FrForm*)m_pForgetDlg)
					->Open((FRESH_PFN_RESULT)&CGolfRule::OnForgetDlgResult, pos,
						3);
				m_forgetDlgTime = 0;
				SendMsg(this, "Item", M983(0x196), 0, 0, 0, 0);
			}
			float acc = dt / CProjectG::Instance()->GetGameSpeed() +
				m_forgetDlgTime;
			m_forgetDlgTime = acc;
			// 983 00691f60: os tres retornos voltam ao 00692480, que ainda roda CCameraWork::Process
			// (o nosso saia direto e a camera parava com a janela do esquecimento aberta)
			if (acc > 10.0f && m_pForgetDlg != NULL)
				((FrForm*)m_pForgetDlg)->Close((eFormRet)2, true);
			goto done;
		}
		UpdateShotResultStat();
		m_bReplayReady = 0;
		*(unsigned long*)&m_modeDuration = 0; // 983: dword 0 (sem fldz)
		if (m_pRuleBase == NULL || !m_pRuleBase->CheckGiveUp())
		{
			if (Doc()->m_golfGame.gameType != 1) // 983 00692241: so o tipo 1 (o 645 tambem tratava o 3)
			{
				ChangeGameMode(0x800);
				goto done;
			}
		}
		else
		{
			SendMsg(this, "GuideLine", M983(0x1a2), 0, 0, 0, 0);
			RecordGiveUp(GOLFDOC()->m_currentPlayer);
			RecordScoreByPlayer(GOLFDOC()->m_currentPlayer);
			if (Doc()->m_golfGame.gameType != 1)
			{
				ChangeGameMode(0x80000);
				goto done;
			}
		}
	}
	ChangeGameMode(0x1000);
done:
	CCameraWork::Instance()->Process(dt, 0.0f);
}

void CGolfRule::CaptureProcess()
{
	static bool s_captureHeadIcon = true;
	static bool s_captureGameUI = true;
	static int s_captureFov = 0x37;
	static int s_captureIndex = 1;

	if (m_captureMode == 0)
	{
		if ((g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true)) &&
			g_input->GetDown("FUNC_10", true) &&
			(*(unsigned char*)GDOCPTR983(Doc()->m_pGolfDoc, 0x159) & 4) /* 983 0067a... dword +0x158 & 0x400 */)
		{
			mkdir("capture");
			mkdir("capture/cinematic");
			m_captureMode = 2;
			m_captureIndex = 1;
			s_captureIndex = 1;
			s_captureFov =
				(int)ceilf(g_view->GetFOV() / g_DEGTORAD); // 983 00679f70
			const char* name =
				MakeStr("capture/cinematic/fov(%d)_%03d_frame(1).bmp",
					s_captureFov, s_captureIndex);
			while (_access(name, 0) != -1)
			{
				s_captureIndex++;
				name = MakeStr("capture/cinematic/fov(%d)_%03d_frame(1).bmp",
					s_captureFov, s_captureIndex);
			}
			SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Player", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x190), 0, 0, 0, 0);
			SendMsg(this, "Effect", M983(0x17b), 0, 0, 0, 0);
			CShadowManager::Instance()->Enable(false);
			CAPTURE_WIND_HIDE();
			((CSceneManagerLod*)CSceneManager::Instance())
				->SetLOD(false, false);
			GetTask()->SetPause(true); // B1474: 983 00679e90 grava task+0x200 (m_bPause); era +0x1f5 do 645
		}
		if (g_input->Get("LCONTROL", true))
			return;
		if (g_input->Get("RCONTROL", true))
			return;
		if (!g_input->GetDown("FUNC_10", true))
			return;
		if (COption::Instance()->vGetTnLMode() == 0 /* 645 +0x3e4 */)
		{
			MessageBoxA(NULL,
				"HW TnL \xb0\xa1\xbc\xd3 \xb8\xf0\xb5\xe5\xbf\xa1\xbc\xad\xb8\xb8 \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9.", // 983 00d17d54
				"\xc1\xd6\xc0\xc7", 0);
			return;
		}
		mkdir("capture");
		mkdir("capture/high quality");
		m_captureMode = 1;
		m_captureIndex = 0;
		s_captureIndex = 1;
		const char* name = MakeStr("capture/high quality/shot%03d.bmp", 1);
		while (_access(name, 0) != -1)
		{
			s_captureIndex++;
			name = MakeStr("capture/high quality/shot%03d.bmp", s_captureIndex);
		}
		s_captureGameUI = m_bGameUI;
		m_bGameUI = false;
		ToggleGameUI();
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "ShotPreview", 0x2fd, (int)"ENABLE_DISPLAY", 0, 0, 0);
		if (CHeadIcon::Instance() != NULL)
		{
			s_captureHeadIcon = CHeadIcon::Instance()->GetVisible();
			CHeadIcon::Instance()->Hide();
		}
		CAPTURE_WIND_HIDE();
		((CSceneManagerLod*)CSceneManager::Instance())->SetLOD(false, false);
		GetTask()->SetPause(true); // B1474: 983 00679e90 grava task+0x200 (m_bPause); era +0x1f5 do 645
		return;
	}
	Bitmap* image = new Bitmap(g_last_screen_width, g_last_screen_height, 24);
	char path[260];
	if (m_captureMode == 1)
	{
		if (m_captureIndex == 0)
			sprintf(path, "capture/high quality/shot%03d_origin.bmp",
				s_captureIndex);
		else
			sprintf(path, "capture/high quality/shot%03d_%03d.bmp",
				s_captureIndex, m_captureIndex);
		if (g_resrcmng->VideoReference()->Command(W_VDEV_CAPTURE_SCREEN, 0,
				(int)image) == 1)
		{
			SaveBMP(path, image);
			if (m_captureIndex < m_captureTiles * m_captureTiles)
			{
				int index = m_captureIndex;
				m_captureIndex = index + 1;
				g_view->xScaleProjMat(index, m_captureTiles);
				delete image;
				return;
			}
		}
		else
			MessageBoxA(NULL, // 983 0067a3af: textos pelo K2L
				MakeStr(K2L_Compatibility("%s \xbd\xba\xc5\xa9\xb8\xb0 \xc4\xb8\xc3\xc4\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					strrchr(path, '/')),
				K2L_Compatibility("\xb0\xe6\xb0\xed"), 0);
		m_bGameUI = s_captureGameUI;
		ToggleGameUI();
		SendMsg(this, "Spot", M983(0x163), 0, 0, 0, 0);
		if (CHeadIcon::Instance() != NULL && s_captureHeadIcon)
			CHeadIcon::Instance()->Show();
		CAPTURE_WIND_SHOW();
		((CSceneManagerLod*)CSceneManager::Instance())->SetLOD(true, false);
		GetTask()->SetPause(false); // B1474: 983 task+0x200 (m_bPause); era +0x1f5 do 645
		g_view->xScaleProjMat(0, 1);
		m_captureMode = 0;
		if (m_captureTiles * m_captureTiles <= m_captureIndex)
			IntergrateFile(MakeStr("capture/high quality/shot%03d"));
		for (int i = 1; i <= m_captureTiles * m_captureTiles; i++)
			DeleteFileA(MakeStr("capture/high quality/shot%03d_%03d.bmp",
				s_captureIndex, i));
		m_captureIndex = -1;
	}
	else if (m_captureMode == 2)
	{
		sprintf(path, "capture/cinematic/fov(%d)_%03d_frame(%d).bmp",
			s_captureFov, s_captureIndex, m_captureIndex);
		if (g_resrcmng->VideoReference()->Command(W_VDEV_CAPTURE_SCREEN, 0,
				(int)image) == 1)
		{
			SaveBMP(path, image);
			if (m_captureIndex * s_captureFov < 360)
			{
				float angle = g_view->GetFOV(); // 983 0067a346
				WMatrix rot = RotMat(angle, 1);
				g_camera *= rot;
				g_camera.xm = m_dbgEye983.x;
				g_camera.ym = m_dbgEye983.y;
				g_camera.zm = m_dbgEye983.z;
				m_captureIndex++;
				delete image;
				return;
			}
		}
		else
			MessageBoxA(NULL, // 983 0067a3af: textos pelo K2L
				MakeStr(K2L_Compatibility("%s \xbd\xba\xc5\xa9\xb8\xb0 \xc4\xb8\xc3\xc4\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					strrchr(path, '/')),
				K2L_Compatibility("\xb0\xe6\xb0\xed"), 0);
		SendMsg(this, "Spot", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Player", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Ball", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x190), 1, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x17a), 0, 0, 0, 0);
		CShadowManager::Instance()->Enable(true);
		CAPTURE_WIND_SHOW();
		((CSceneManagerLod*)CSceneManager::Instance())->SetLOD(true, false);
		GetTask()->SetPause(false); // B1474: 983 task+0x200 (m_bPause); era +0x1f5 do 645
		m_captureMode = 0;
		float x = g_camera.xz + m_dbgEye983.x;
		float y = g_camera.yz + m_dbgEye983.y;
		float z = g_camera.zz + m_dbgEye983.z;
		m_camPivot.x = x;
		m_camPivot.y = y;
		m_camPivot.z = z;
		m_bGameUI = s_captureGameUI;
		ToggleGameUI();
		SendMsg(this, "Spot", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "ShotPreview", 0x2fd, (int)"ENABLE_DISPLAY", 1, 0, 0);
		if (CHeadIcon::Instance() != NULL && s_captureHeadIcon)
			CHeadIcon::Instance()->Show();
		m_captureIndex = -1;
	}
	delete image;
}

void CGolfRule::IntergrateFile(const char* filename)
{
	BITMAPINFOHEADER info;
	unsigned char header[0xe];
	info.biSize = 0x28;
	info.biWidth = 0;
	info.biHeight = 0;
	info.biPlanes = 1;
	info.biBitCount = 0x18;
	info.biCompression = 0;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;
	info.biClrUsed = 0;
	info.biClrImportant = 0;
	header[0] = 0x42;
	header[1] = 0x4d;
	*(unsigned long*)(header + 2) = 0xe;
	*(unsigned short*)(header + 6) = 0;
	*(unsigned short*)(header + 8) = 0;
	*(unsigned long*)(header + 10) = 0x36;
	FILE* fp = fopen(MakeStr("%s.bmp", filename), "wb");
	if (fp != NULL)
	{
		Bitmap** bitmaps = new Bitmap*[CAPTURE_TILES()];
		info.biWidth = (int)((float)CAPTURE_TILES() * g_view->GetWidth());
		info.biHeight = (int)((float)CAPTURE_TILES() * g_view->GetHeight());
		fwrite(header, 1, 0xe, fp);
		fwrite(&info, 1, 0x28, fp);
		int tiles = CAPTURE_TILES();
		int row = 0;
		if (0 < tiles)
		{
			do
			{
				int col = 0;
				if (0 < tiles)
				{
					do
					{
						bitmaps[col] =
							g_resrcmng->LoadBMP(MakeStr("%s_%03d.bmp", filename,
								(CAPTURE_TILES() - row - 1) * CAPTURE_TILES() +
									1 + col));
						if (bitmaps[col] == NULL)
							goto done;
						col++;
					} while (col < CAPTURE_TILES());
				}
				int y = bitmaps[0]->Height();
				while (--y >= 0)
				{
					Bitmap** tile = bitmaps;
					int n = 0;
					if (0 < CAPTURE_TILES())
					{
						do
						{
							int x = 0;
							if (0 < (int)(*tile)->Width())
							{
								int offset = 0;
								do
								{
									unsigned char* pixel =
										(*tile)->GetVram(y) + offset;
									fputc(pixel[0], fp);
									fputc(pixel[1], fp);
									fputc(pixel[2], fp);
									offset += 3;
									x++;
								} while (x < (int)(*tile)->Width());
							}
							n++;
							tile++;
						} while (n < CAPTURE_TILES());
					}
				}
				if (0 < CAPTURE_TILES())
				{
					int k = 0;
					do
					{
						if (bitmaps[k] != NULL)
							delete bitmaps[k];
						k++;
					} while (k < CAPTURE_TILES());
				}
				tiles = CAPTURE_TILES();
				row++;
			} while (row < tiles);
		}
done:
		delete[] bitmaps;
		fclose(fp);
	}
}

void CGolfRule::DoDebugCamera(float dt)
{
	static float s_dbgFwd;
	static float s_dbgRight;
	static float s_dbgUp;

	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) != 0)
	{
		CFxSequence* seq = CFx::Instance()->OpenSequence("bullet.seq", true);
		if (seq != NULL)
		{
			WVector pos(g_camera.xz * 10.0f + g_camera.xm - g_camera.xy * 3.0f,
				g_camera.yz * 10.0f + g_camera.ym - g_camera.yy * 3.0f,
				g_camera.zm + g_camera.zz * 10.0f - g_camera.zy * 3.0f);
			seq->m_pos = pos; // B1556: era visao crua VC10_FXSEQ_POS
			*(WMatrix*)((char*)seq + VC10_FXSEQ_ACTIVE - 0x38) =
				RotMat((float)atan2(0.009374999441206455, 1.0), 0) * g_camera;
		}
	}
	if (g_input->GetDown("PARTICLE_2", true) != 0)
	{
		CFxSpray* spray = CFx::Instance()->OpenSpray("bullet.spr");
		if (spray != NULL)
		{
			WVector pos(g_camera.xz * 10.0f + g_camera.xm,
				g_camera.ym + g_camera.yz * 10.0f,
				g_camera.zm + g_camera.zz * 10.0f);
			*(WVector*)((char*)spray + VC10_FXSPRAY_POS) = pos;
			*(WVector*)((char*)spray + VC10_FXSPRAY_POS + 0x18) =
				RotVec(*(WVector*)((char*)spray + VC10_FXSPRAY_POS + 0x18), g_camera);
		}
	}
	if (g_input->GetDown("\xb5\xf0\xb9\xf6\xb1\xd7\xc5\xb0", true) == 0)
	{
		WVector mouse = g_input->GetMouseDelta();
		if (WisEqual(mouse, WVector::ZERO, g_EPSILON) == 0)
		{
			float dx = g_camera.xx * mouse.x - g_camera.xy * mouse.y;
			float dy = (g_camera.yx * mouse.x - g_camera.yy * mouse.y) * 0.01f;
			float dz = (g_camera.zx * mouse.x - g_camera.zy * mouse.y) * 0.01f;
			m_camPivot.x = dx * 0.01f + m_camPivot.x;
			m_camPivot.y = dy + m_camPivot.y;
			m_camPivot.z = dz + m_camPivot.z;
			WVector dir(m_camPivot.x - DBG_EYE.x, m_camPivot.y - DBG_EYE.y,
				m_camPivot.z - DBG_EYE.z);
			WVector n = dir.Normalize();
			m_camPivot =
				WVector(n.x + DBG_EYE.x, n.y + DBG_EYE.y, n.z + DBG_EYE.z);
		}
		float step = dt;
		if (Wabs(step) < g_EPSILON)
			step = 0.03f;
		float speed = step * 300.0f;
		float slow = g_input->Get("LALT", true) != 0 ? 0.05f : 1.0f;
		if (g_input->Get(0x57) != 0 && g_input->Get(0x53) == 0)
			s_dbgFwd = slow * speed + s_dbgFwd;
		else if (g_input->Get(0x53) != 0 && g_input->Get(0x57) == 0)
			s_dbgFwd = s_dbgFwd - slow * speed;
		else
			s_dbgFwd = 0.0f;
		if (g_input->Get(0x41) != 0 && g_input->Get(0x44) == 0)
			s_dbgRight = s_dbgRight - slow * speed;
		else if (g_input->Get(0x44) != 0 && g_input->Get(0x41) == 0)
			s_dbgRight = slow * speed + s_dbgRight;
		else
			s_dbgRight = 0.0f;
		if (g_input->Get(0x5a) != 0 && g_input->Get(0x58) == 0)
			s_dbgUp = slow * speed + s_dbgUp;
		else if (g_input->Get(0x58) != 0 && g_input->Get(0x5a) == 0)
			s_dbgUp = s_dbgUp - slow * speed;
		else
			s_dbgUp = 0.0f;
		float limit = DBG_FLOAT(0x484);
		if (0.0f < limit)
		{
			if (s_dbgFwd < -limit)
				s_dbgFwd = -limit;
			else if (limit < s_dbgFwd)
				s_dbgFwd = limit;
			if (s_dbgRight < -limit)
				s_dbgRight = -limit;
			else if (limit < s_dbgRight)
				s_dbgRight = limit;
			if (s_dbgUp < -limit)
				s_dbgUp = -limit;
			else if (limit < s_dbgUp)
				s_dbgUp = limit;
		}
		if (g_input->Get("ROTATE_CW", true) != 0)
		{
			if (g_input->GetDown("ROTATE_CW", true) != 0 &&
				g_input->GetDown("ROTATE_CCW", true) == 0)
			{
				WVector fwd(g_camera.xz, 0.0f, g_camera.zz);
				WVector n = fwd.Normalize();
				float dist = DBG_FLOAT(0x488);
				DBG_FLOAT(0x490) = dist * n.x + g_camera.xm;
				DBG_FLOAT(0x494) = dist * n.y + g_camera.ym;
				DBG_FLOAT(0x498) = dist * n.z + g_camera.zm;
			}
			WVector rel(g_camera.xm - DBG_FLOAT(0x490),
				g_camera.ym - DBG_FLOAT(0x494), g_camera.zm - DBG_FLOAT(0x498));
			WVector r =
				rel * RotMat(-((DBG_FLOAT(0x484) / DBG_FLOAT(0x488)) * dt), 1);
			DBG_EYE.x = DBG_FLOAT(0x490) + r.x;
			DBG_EYE.y = DBG_FLOAT(0x494) + r.y;
			DBG_EYE.z = DBG_FLOAT(0x498) + r.z;
			WVector dir(DBG_FLOAT(0x490) - DBG_EYE.x,
				(DBG_FLOAT(0x488) / sqrtf(1.0f - g_camera.yz * g_camera.yz)) *
					g_camera.yz,
				DBG_FLOAT(0x498) - DBG_EYE.z);
			WVector n = dir.Normalize();
			m_camPivot =
				WVector(DBG_EYE.x + n.x, DBG_EYE.y + n.y, DBG_EYE.z + n.z);
		}
		if (g_input->Get("ROTATE_CCW", true) != 0)
		{
			if (g_input->GetDown("ROTATE_CCW", true) != 0 &&
				g_input->GetDown("ROTATE_CW", true) == 0)
			{
				WVector fwd(g_camera.xz, 0.0f, g_camera.zz);
				WVector n = fwd.Normalize();
				float dist = DBG_FLOAT(0x488);
				DBG_FLOAT(0x490) = dist * n.x + g_camera.xm;
				DBG_FLOAT(0x494) = dist * n.y + g_camera.ym;
				DBG_FLOAT(0x498) = dist * n.z + g_camera.zm;
			}
			WVector rel(g_camera.xm - DBG_FLOAT(0x490),
				g_camera.ym - DBG_FLOAT(0x494), g_camera.zm - DBG_FLOAT(0x498));
			WVector r =
				rel * RotMat((DBG_FLOAT(0x484) / DBG_FLOAT(0x488)) * dt, 1);
			DBG_EYE.x = DBG_FLOAT(0x490) + r.x;
			DBG_EYE.y = DBG_FLOAT(0x494) + r.y;
			DBG_EYE.z = DBG_FLOAT(0x498) + r.z;
			WVector dir(DBG_FLOAT(0x490) - DBG_EYE.x,
				(DBG_FLOAT(0x488) / sqrtf(1.0f - g_camera.yz * g_camera.yz)) *
					g_camera.yz,
				DBG_FLOAT(0x498) - DBG_EYE.z);
			WVector n = dir.Normalize();
			m_camPivot =
				WVector(DBG_EYE.x + n.x, DBG_EYE.y + n.y, DBG_EYE.z + n.z);
		}
		if (g_EPSILON <= Wabs(s_dbgFwd))
		{
			DBG_EYE.x = g_camera.xz * s_dbgFwd * dt + DBG_EYE.x;
			DBG_EYE.y = g_camera.yz * s_dbgFwd * dt + DBG_EYE.y;
			DBG_EYE.z = g_camera.zz * s_dbgFwd * dt + DBG_EYE.z;
			m_camPivot.x = g_camera.xz * s_dbgFwd * dt + m_camPivot.x;
			m_camPivot.y = g_camera.yz * s_dbgFwd * dt + m_camPivot.y;
			m_camPivot.z = g_camera.zz * s_dbgFwd * dt + m_camPivot.z;
		}
		if (g_EPSILON <= Wabs(s_dbgRight))
		{
			DBG_EYE.x = g_camera.xx * s_dbgRight * dt + DBG_EYE.x;
			DBG_EYE.y = g_camera.yx * s_dbgRight * dt + DBG_EYE.y;
			DBG_EYE.z = g_camera.zx * s_dbgRight * dt + DBG_EYE.z;
			m_camPivot.x = g_camera.xx * s_dbgRight * dt + m_camPivot.x;
			m_camPivot.y = g_camera.yx * s_dbgRight * dt + m_camPivot.y;
			m_camPivot.z = g_camera.zx * s_dbgRight * dt + m_camPivot.z;
		}
		if (g_EPSILON <= Wabs(s_dbgUp))
		{
			DBG_EYE.y = s_dbgUp * dt + DBG_EYE.y;
			m_camPivot.y = s_dbgUp * dt + m_camPivot.y;
		}
		g_camera = MakeCameraMatrix(DBG_EYE, m_camPivot);
	}
	else
	{
		GOLFDOC()->m_gameMode = m_savedGameMode983;
		g_camera = m_savedCamera983;
		SendMsg(this, "Camera", M983(0x19a), 0, 0, 0, 0);
		s_dbgUp = 0.0f;
		s_dbgRight = 0.0f;
		s_dbgFwd = 0.0f;
		if (m_pOverBarSpecialCard != NULL)
			m_pOverBarSpecialCard->SetVisible(true);
		if (*(FrWnd**)(GRPTR983(0x4fc)) != NULL)
			(*(FrWnd**)(GRPTR983(0x4fc)))->SetVisible(true);
		if (*(FrWnd**)(GRPTR983(0x500)) != NULL)
			(*(FrWnd**)(GRPTR983(0x500)))->SetVisible(true);
		ToggleCommandBtns(true);
		if (m_pMessengerBtn != NULL)
			m_pMessengerBtn->SetVisible(true);
		if (SHAREDDOC_UCHAR(0x10d81) != 0 &&
			*(FrWnd**)((char*)&m_pMessengerAlarmBtn) != NULL)
		{
			(*(FrWnd**)((char*)&m_pMessengerAlarmBtn))->SetTopmost(true);
			(*(FrWnd**)((char*)&m_pMessengerAlarmBtn))->SetVisible(true);
		}
		if (SHAREDDOC_UCHAR(0x10d80) != 0 &&
			*(FrWnd**)((char*)&m_pMessengerGiftAlarmBtn) != NULL)
		{
			(*(FrWnd**)((char*)&m_pMessengerGiftAlarmBtn))->SetTopmost(true);
			(*(FrWnd**)((char*)&m_pMessengerGiftAlarmBtn))->SetVisible(true);
		}
	}
}

void CGolfRule::DoStand(float time)
{
	if (((*(unsigned char*)USERINFO983(GOLFDOC()->m_currentPlayer, 0x3922)) & 1) != 0)
	{
		CRival::Instance()->Shot();
		HitShot();
		CRival::Instance()->SetResult();
		return;
	}
	if (TopViewClick()) // 983 0069a8c4: sem o bloco dos botoes de alarme do 645 (B1615; mesmo caso do B597)
	{
		ChangeGameMode(0x200);
		return;
	}
	if (Doc()->m_playingMode != 2 && !IsMyTurn(false))
		return;
	bool quit = CQuitWindow::Instance()->IsActive();
	bool ok = !quit;
	if ((unsigned long)GOLFDOC()->m_tutorialMode < 0xf &&
		(*((unsigned char*)GRPTR983(0x439)) != 0 || (ok = false, !quit)))
		ok = true;
	if (!g_input->Get("LSHIFT", true) && !g_input->Get("RSHIFT", true) &&
		g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) &&
		ok && GOLFDOC()->m_bPause == 0 && !CChatMsg::Instance()->IsActive() &&
		(int)GOLFDOC()->m_tutorialMode > 1 &&
		*(int*)((char*)&GolfClub() + 0x54) == 2 &&
		*((unsigned char*)GRPTR983(0x455)) == 0)
	{
		_ImeReset();
		if (m_bUnplayable983 == 0)
		{
			g_audio->PlaySfx("play_unplayable");
			return;
		}
		int result;
		SendMsg(this, "Item", M983(0x199), (int)&result, 0, 0, 0);
		if (result == 3)
		{
			SendMsg(this, "Player", M983(0x199), (int)&result, 0, 0, 0);
			if (result != 3)
			{
				if (Doc()->m_playingMode == 1)
					ChangeGameMode(0x10);
				else if (Doc()->m_playingMode == 2)
				{
					memcpy(GDOCPTR983(GOLFDOC(), 0xe0), &m_shotData.bar, 0x24);
					SetEnvironmentVariableA();
					GOLFDOC()->MakeDefaultCamera(true);
					HitShot();
				}
			}
		}
	}
	// B724: 983 0069a860 - fim: teclas AUTO_TOMAHAWK/COBRA/SPIKE mandam a msg 0x329 (trava de tela) com o taco que tem item
	if ((CLUB_XOR() != 0xffffffff) && m_bAutoShot != 0)
	{
		if (g_input->GetDown("AUTO_TOMAHAWK", true))
			SendMsg(this, "GolfRule", 0x329, 0x10, 0, 0, 0);
		else if (g_input->GetDown("AUTO_COBRA", true))
			SendMsg(this, "GolfRule", 0x329, 0x20, 0, 0, 0);
		else if (g_input->GetDown("AUTO_SPIKE", true))
			SendMsg(this, "GolfRule", 0x329, 0x40, 0, 0, 0);
	}
}

void CGolfRule::DoCoursePreview(float time)
{
	if (!g_input->GetDown("NUM_9", true) && !g_input->GetDown("PAD_9", true))
	{
		g_camera = RotMat(-0.34906584f, 0) *
			RotMat((m_modeDuration + m_modeTime) * 0.17453292f, 1);
		WVector pin;
		WVector tee = GOLFDOC()->GetHoleData().tee;
		pin = GOLFDOC()->GetHoleData().pin;
		float dist = (pin - tee).Magnitude() * 0.5f + 640.0f;
		g_camera.pivot = m_camPivot - g_camera.za * dist;
		return;
	}
	SCENEMGR()->SetVisible(MakeStr("PLAYER_%d", GOLFDOC()->m_currentPlayer),
		true);
	SCENEMGR()->SetVisible("\xc5\xac\xb7\xb4", true);
	SCENEMGR()->SetVisible("\xc4\xb3\xb5\xf0", true);
	CShadowManager::Instance()->Enable(COption::Instance()->vIsShadowEnabled());
	SendMsg(this, "Score", M983(0x13b), 0, 0, 0, 0);
	ChangeGameMode(8);
}

void CGolfRule::DoGetReadyToCaptureBG(float)
{
	if (CCapturedBg::Instance()->IsCaptured())
	{
		if (COption::Instance()->dGetClickSystem() &&
			(g_input->GetButton(LEFT_BUTTON) == 2) ^
				g_input->GetUp("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9"))
		{
			CancelShot();
			return;
		}
		// B726: 983 00685ca0 (antes do ChangeGameMode): com a trava de tela (+0x240) e taco valido, esvazia a fila (00681f60) e
		// reenfileira (00680f20) as teclas conforme o byte +0x240: 0x10 -> 1,2; 0x20 -> 8,1; 0x40 -> 8,2
		if (m_screenLockMode != 0 && CLUB_XOR() != 0xffffffff)
		{
			unsigned char key = m_screenLockMode;
			*(unsigned long*)&m_cmdQueueTime = 0;
			while (!m_cmdQueue.empty())
				m_cmdQueue.pop_front();
			if (key == 0x10)
			{
				m_cmdQueue.push_back((void*)1);
				m_cmdQueue.push_back((void*)2);
			}
			else if (key == 0x20)
			{
				m_cmdQueue.push_back((void*)8);
				m_cmdQueue.push_back((void*)1);
			}
			else if (key == 0x40)
			{
				m_cmdQueue.push_back((void*)8);
				m_cmdQueue.push_back((void*)2);
			}
		}
		ChangeGameMode(0x20);
	}
}

// JP 983 (00685b90, chamada so do DoPowerGauge): mesmas guardas do CommandBox (fase >= 2, regua ate a metade, forca >= 80%,
// sem tecla de comando pendente); soma o tempo em +0x23c e, passados 0.1 s com a fila deque +0x224 nao vazia, passa o primeiro
// comando da fila para a lista de comandos do doc (+0x8c) e zera o tempo. O 645 nao tinha essa fila.
static void FlushCommandQueue983(CGolfRule* rule, float dt)
{
	CGolfDoc* doc = GOLFDOC();
	if (doc->m_shotPhase < 2)
		return;
	if (doc->m_bar.gauge > g_fBarRange * 0.5f + BAR_START)
		return;
	if (doc->m_bar.power < g_fBarRange * 0.8f + BAR_START)
		return;
	if (doc->m_inputList983.size() > 0 &&
		(doc->m_inputList983.back() & 0x70) != 0)
		return;
	float* acc = &rule->m_cmdQueueTime;
	*acc = *acc + dt;
	if (!rule->m_cmdQueue.empty() && *acc >= 0.1)
	{
		unsigned long cmd = (unsigned long)rule->m_cmdQueue.front();
		(&GOLFDOC()->m_inputList983)->push_back(cmd);
		rule->m_cmdQueue.pop_front();
		*acc = 0.0f;
	}
}
void CGolfRule::DoPowerGauge(float dt)
{
	bool hit;
	if (m_bGaugePaused)
	{
		m_gaugePauseTick = timeGetTime();
		IActor* tutorial = GetActor("Tutorial");
		if (!tutorial)
			return;
		tutorial << MsgObject(NULL, M983(0x156), 0, 0, 0, 0, 0);
		return;
	}
	if (m_bContinueDlg)
		return;
	if (m_bNetMsgWait)
		return;
	if (!COption::Instance()->dGetClickSystem())
	{
		hit = g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				  true) != (g_input->GetButton(LEFT_BUTTON) == 1);
	}
	else if (GOLFDOC()->m_shotPhase == 1 &&
		(g_input->GetButton(LEFT_BUTTON) == 2) !=
			g_input->GetUp("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9"))
	{
		hit = true;
	}
	else
	{
		if (GOLFDOC()->m_shotPhase == 2 &&
			g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				true) != (g_input->GetButton(LEFT_BUTTON) == 1))
			hit = true;
		else
			hit = false;
	}
	if ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
		hit = false;
	if (!IsMyTurn(false))
	{
		if (m_bRemoteHitPending) // 983 0069b9da
		{
			m_bRemoteHitPending = false;
			hit = true;
		}
	}
	else
	{
		float narrow = 10.0f;
		if (((CClub::sClub*)*(char**)((char*)&GolfClub() + 4))->type != 3)
			narrow = ((CClubEx*)&GolfClub())->GetNarrowAccuracy();
		if (GOLFDOC()->m_shotPhase == 1 && hit &&
			GOLFDOC()->m_bar.gauge < narrow + BAR_START) // 983 0069b790: fadd 140.0 (d06f48)
		{
			CancelShot();
			return;
		}
		// 983 0069b7c0..0069b9b7: o hit so cai se a trava (+0x439) esta ligada e o botao do speedbar nao esta apertado;
		// depois a tecla "BANISH" tem de bater com o botao direito, senao cancela a tacada (nao existia no 645)
		if (m_bTutorialShot983 &&
			!g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
			hit = false;
		if (g_input->GetDown("BANISH", true) != g_input->GetButton(RIGHT_BUTTON))
		{
			CancelShot();
			return;
		}
		if (hit)
		{
			if (OnlinePlay())
			{
				unsigned char data[5];
				*(unsigned long*)&data[1] = *(unsigned long*)&GOLFDOC()->m_bar.gaugeStep;
				data[0] = GOLFDOC()->m_shotPhase + 1;
				if (((CClub::sClub*)*(char**)((char*)&GolfClub() + 4))->type ==
						3 &&
					data[0] == 2)
					data[0] = 3;
				WSendPacket packet((enumClientPacket)0x14);
				packet.EncodeBuffer(data, 5);
				packet.Send(TO_GAME);
			}
		}
		else if (m_bTutorialShot983)
		{
			if (((GOLFDOC()->m_shotPhase == 1) &&
					(0.0f < GOLFDOC()->m_bar.gauge - m_tutorialGauge983[0])) ||
				((GOLFDOC()->m_shotPhase == 2) &&
					(GOLFDOC()->m_bar.gauge - m_tutorialGauge983[1] < 0.0f)))
			{
				IActor* tutorial = GetActor("Tutorial");
				if (tutorial)
				{
					// 983: o ponteiro e this+0x43c+(fase-1)*4 (o 0x3e8 era do 645)
					tutorial << MsgObject(NULL, M983(0x20f), 2,
						(unsigned long)&m_tutorialGauge983[(unsigned char)(GOLFDOC()->m_shotPhase - 1)],
						GOLFDOC()->m_shotPhase, 0, 0);
					tutorial << MsgObject(NULL, M983(0x148), 0, 0, 0, 0, 0);
					if (g_input->GetDown(
							"\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
							true) == 1)
						hit = true;
					if (m_bContinueDlg)
						return;
				}
			}
		}
		FlushCommandQueue983(this, dt); // 983 00685b90
		if ((unsigned long)GOLFDOC()->m_tutorialMode > 0xc)
			CommandBox();
	}
	if (m_gaugeTick == 0)
	{
		unsigned long now = timeGetTime();
		m_gaugeTick = now;
		if (m_gaugeStartTick == 0)
			m_gaugeStartTick = now;
	}
	unsigned long now;
	int elapsed;
	if (!hit)
	{
		now = timeGetTime();
		elapsed = now - m_gaugeTick;
	}
	else
	{
		if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
			g_input->GetEventTime("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9");
		else
			g_input->GetEventTime(LEFT_BUTTON);
		now = timeGetTime();
		elapsed = now - m_gaugeTick;
	}
	float step = (float)elapsed;
	if (elapsed < 0)
		step = step + 4294967296.0f; // 983 0069b5c0: volta do timeGetTime (d66cb0)
	step = step * 0.001f;
	m_gaugeTick = now;
	if (!IsMyTurn(false))
		step = step * 0.85f;
	if (((CClub::sClub*)*(char**)((char*)&GolfClub() + 4))->type == 3)
		PuttingGauge(step, hit);
	else
		DriveGauge(step, hit);
	double bar = (double)GOLFDOC()->m_bar.gauge;
	if (GOLFDOC()->m_bar.dir == 1)
		GOLFDOC()->m_bar.gaugeStep = (float)floor(bar);
	else
		GOLFDOC()->m_bar.gaugeStep = ceilf(bar);
}

void CGolfRule::DoApproachNextHole(float delta)
{
	if (OnlinePlay() && GOLFDOC()->m_bApproach795 != 1)
	{
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
		CSharedDoc* shared = Doc();
		char* doc = (char*)shared->m_pGolfDoc;
		if (GetHoleIndex(*(unsigned char*)GDOCPTR983(doc, 0x17c)) ==
			*((unsigned char*)shared + DOCOFS983(0x10699)))
		{
			*(unsigned char*)GDOCPTR983(doc, 0x17c) = 1;
			GOLFDOC()->m_bNoStat = 1;
		}
		else
		{
			doc = (char*)shared->m_pGolfDoc;
			unsigned char index = GetHoleIndex(*(unsigned char*)GDOCPTR983(doc, 0x17c));
			*(unsigned char*)GDOCPTR983(doc, 0x17c) =
				shared->m_holeOrder[index];
			GOLFDOC()->m_bNoStat = 1;
		}
		GOLFDOC()->m_pPolySoup->LoadMapCheckData(GOLFDOC()->m_currentHole);
		*((unsigned char*)GRPTR983(0x41e)) = 0;
		*((unsigned char*)GRPTR983(0x41f)) = 0;
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
		m_modeDuration = 0;
		Doc()->m_approachStartTime = 0;
		GolfBall().SetHoleIn(0);
		SendMsg(this, "Player", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Mascot", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x163), 0, 0, 0, 0);
		SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a4), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
		GOLFDOC()->RefreshPlayerNum();
		SendHoleData();
		AfxGetTask()->GetActor("GolfRule")
			<< MsgObject(NULL, M983(0x99), 0, 0, 0, 0, 0);
		GOLFDOC()->m_bApproach795 = 1;
	}
}

void CGolfRule::CommandBox()
{
	CGolfDoc* doc = GOLFDOC();
	if (doc->m_shotPhase < 2)
		return;
	if (g_fBarRange * 0.5f + BAR_START < doc->m_bar.gauge)
		return;
	if (doc->m_bar.power < g_fBarRange * 0.8f + BAR_START)
		return;
	if (doc->m_inputList983.size() > 0 &&
		(doc->m_inputList983.back() & 0x70) != 0)
		return;
	// 983 00675dd0: com a trava (+0x240) so passa se a regua esta acima do limite e +0x234 == 0
	if (m_screenLockMode && !(m_cmdQueueTime > 0.0f && m_cmdQueue.empty())) // B1482: ex-GR_ULONG(0x234) = _Mysize do deque VC10 (cru no cl 14.44)
		return;
	int count = 0;
	unsigned long cmd = 0;
	if (g_input->Get("\xbb\xf3", true))
	{
		count = 1;
		cmd = 1;
	}
	if (g_input->Get("\xc7\xcf", true))
	{
		count++;
		cmd |= 2;
	}
	if (g_input->Get("\xc1\xc2", true))
	{
		count++;
		cmd |= 4;
	}
	if (g_input->Get("\xbf\xec", true))
	{
		count++;
		cmd |= 8;
	}
	if (count == 0)
	{
		m_cmdTick = 0;
		return;
	}
	if (count == 1)
	{
		if (m_cmdTick > 0)
		{
			if (GOLFDOC()->m_tutorialMode > 12 &&
				GOLFDOC()->m_tutorialMode < 15)
			{
				if (timeGetTime() - m_cmdTick == 0)
					return;
				cmd = 0x10;
			}
			else
			{
				if (timeGetTime() - m_cmdTick <= 100)
					return;
				cmd = 0x10;
				m_cmdQueue.push_back((void*)cmd);
				return;
			}
		}
		else
		{
			m_cmdTick = timeGetTime();
			(&GOLFDOC()->m_inputList983)
				->push_back(cmd);
			return;
		}
	}
	else
		cmd = 0x40;
	m_cmdQueue.push_back((void*)cmd);
}

void CGolfRule::ToggleCommandBtns(bool visible)
{
	if (m_pCaddieBookBtn)
		m_pCaddieBookBtn->SetVisible(visible);
	if (m_pUnplayableBtn)
		m_pUnplayableBtn->SetVisible(visible);
	if (m_pPauseBtn)
		m_pPauseBtn->SetVisible(visible);
	if (m_pChatRecordBtn)
		m_pChatRecordBtn->SetVisible(visible);
}

// 983 00675090: o acerto caiu na area de PangYa (|impacto - centro| < area + 0,5)
static bool IsPangYaImpact983(float area)
{
	float offset = GOLFDOC()->m_bar.impact - GOLFDOC()->m_bar.center;
	if (offset <= 0.0f)
		offset = -offset;
	return offset < area + 0.5;
}

// 983 006750f0: com a area impar (meio ponto tirado), o lado esquerdo ganha um ponto a mais
static bool IsLuckyPangYaImpact983(float area)
{
	return (area + 0.5 + 1.0) * -1.0 < GOLFDOC()->m_bar.impact - GOLFDOC()->m_bar.center &&
		GOLFDOC()->m_bar.impact - GOLFDOC()->m_bar.center < 0.0;
}

// 983: objetos de correcao do PangYa do jogador da vez, guardados no CGolfDoc (m_pPangyaCorrection*983, A s97): CGolfDoc 0065fe70 (+0x860,
// CPangyaCorrectionObject, habilidade 0x0a), 0065fec0 (+0x870, CPangyaAddCorrectionObj, 0x16), 0065fef0 (+0x880,
// CPangyaCorrectionSetObject, 0x23); 0065fed0 zera os 4 de +0x880 e 00662200 os refaz pelo CControllerDoc (conteudo 0xad).
static CPangyaCorrectionObject* PangyaCorrection983(CGolfDoc* doc)
{
	return doc->m_pPangyaCorrection983[doc->m_currentPlayer];
}

static CPangyaAddCorrectionObj* PangyaAddCorrection983(CGolfDoc* doc)
{
	return doc->m_pPangyaAddCorrection983[doc->m_currentPlayer];
}

static CPangyaCorrectionSetObject* PangyaCorrectionSet983(CGolfDoc* doc)
{
	return doc->m_pPangyaCorrectionSet983[doc->m_currentPlayer];
}

static void ResetPangyaCorrectionSet983(CGolfDoc* doc)
{
	for (int i = 0; i < 4; i++)
		doc->m_pPangyaCorrectionSet983[i] = NULL;
}

static void LoadPangyaCorrectionSet983(CGolfDoc* doc)
{
	IContentsDataContainer* container = CContentsDoc::Instance()->GetContainer((localContentType_t)0xad);
	if (container)
	{
		for (int i = 0; i < 4; i++)
			doc->m_pPangyaCorrectionSet983[i] = (CPangyaCorrectionSetObject*)((_gateway::_controller::CControllerDoc*)container)->GetObjectAt(i, 0x23);
	}
}

// 983 00675160: sem o IsLocalContent(0x6e) do 645 (o ponto da area impar vale sempre); sem PangYa, os objetos de correcao
// do jogador (cartas/itens) alargam a area (+3, ou +4 com a correcao extra) e, se o PangYa sair por eles, avisam o
// servidor (0x15c correcao, 0x199 conjunto, 0x196 extra). O som sai uma vez no fim.
bool CGolfRule::GetBarCondition()
{
	GOLFDOC()->m_bar.impact = GOLFDOC()->m_bar.gaugeStep;
	GOLFDOC()->m_bar.dir = 0;
	float delta = GOLFDOC()->m_bar.impact - BAR_START;
	if (delta <= 0.0f)
		delta = -delta;
	float area = CPowerGauge::Instance()->GetPangYaArea();
	bool lucky = false;
	if ((int)(area + area) % 2 == 1)
	{
		lucky = true;
		area = area - 0.5;
	}
	const char* sound;
	if (IsPangYaImpact983(area) || (lucky == true && IsLuckyPangYaImpact983(area)))
	{
		GOLFDOC()->m_bar.grade = 4;
		sound = g_plSound[1];
	}
	else if (delta < ((CClubEx*)&GolfClub())->GetNarrowAccuracy() + 0.5)
	{
		GOLFDOC()->m_bar.grade = 3;
		sound = g_plSound[2];
	}
	else if (delta < ((CClubEx*)&GolfClub())->GetWideAccuracy() + 0.5)
	{
		GOLFDOC()->m_bar.grade = 2;
		sound = g_plSound[3];
	}
	else
	{
		GOLFDOC()->m_bar.grade = 1;
		sound = g_plSound[4];
	}
	switch ((unsigned long)GOLFDOC()->m_bar.grade)
	{
	case 1:
	case 2:
	case 3:
	{
		bool corr = false, add = false, set = false;
		if (CPangyaAddCorrectionObj* p = PangyaAddCorrection983(GOLFDOC()))
			add = p->IsHit();
		if (CPangyaCorrectionObject* p = PangyaCorrection983(GOLFDOC()))
			corr = p->IsHit();
		ResetPangyaCorrectionSet983(GOLFDOC());
		LoadPangyaCorrectionSet983(GOLFDOC());
		if (CPangyaCorrectionSetObject* p = PangyaCorrectionSet983(GOLFDOC()))
			set = p->IsHit();
		if (corr == true)
		{
			if (add == true || set == true)
				area = area + 4.0;
			else
				area = area + 3.0;
		}
		else if (add == true || set == true)
			area = area + 4.0;
		else
			break;
		lucky = false;
		if ((int)(area + area) % 2 == 1)
		{
			lucky = true;
			area = area - 0.5;
		}
		if (IsPangYaImpact983(area) || (lucky == true && IsLuckyPangYaImpact983(area)))
		{
			GOLFDOC()->m_bar.grade = 4;
			sound = g_plSound[1];
			if (corr == true)
			{
				WSendPacket packet((enumClientPacket)0x15c);
				packet.Send(TO_GAME);
			}
			if (set == true)
			{
				WSendPacket packet((enumClientPacket)0x199);
				packet.Send(TO_GAME);
			}
			if (add == true)
			{
				WSendPacket packet((enumClientPacket)0x196);
				packet.Send(TO_GAME);
			}
		}
		break;
	}
	}
	if (sound)
		g_audio->PlaySfx(sound);
	return GOLFDOC()->m_bar.grade == 4 || GOLFDOC()->m_bar.grade == 3 ||
		GOLFDOC()->m_bar.grade == 2;
}

void CGolfRule::CalcInputCommand()
{
	int index = 0;
	m_inputCmdTail = 0;
	m_inputCmdHead = 0;
	CGolfDoc* doc = GOLFDOC();
	if (0)
		doc->m_inputList983.back();
	if (doc->m_inputList983.size() > 0)
	{
		std::list<unsigned long>::iterator end = doc->m_inputList983.end();
		std::list<unsigned long>::iterator node = end;
		if ((*--node & 0x40) != 0)
			return;
		node = end;
		if (++node != end)
		{
			do
			{
				if (index < 2)
				{
					if (m_inputCmdHead > 0 && *node != 0x10)
						m_inputCmdHead <<= 8;
					m_inputCmdHead |= *node;
				}
				else if (index > 1)
				{
					if (m_inputCmdTail > 0 && *node != 0x10)
						m_inputCmdTail <<= 8;
					m_inputCmdTail |= *node;
				}
				++node;
				index++;
			} while (node != GOLFDOC()->m_inputList983.end());
		}
	}
}

void CGolfRule::SetSpecialShot()
{
	unsigned long v = 0x10;
	m_specialShotGroup = 0;
	switch (m_inputCmdHead)
	{
	case 0x11:
		if (GOLFDOC()->m_bar.grade != 4)
			return;
		GolfBall().m_initSpecial = 1;
		return;
	case 0x12:
		if (GOLFDOC()->m_bar.grade != 4)
			return;
		GolfBall().m_initSpecial = 2;
		return;
	case 0x14:
		v = 4;
		if (GOLFDOC()->m_bar.grade != 4)
			return;
		break;
	case 0x18:
		if (GOLFDOC()->m_bar.grade != 4)
			return;
		GolfBall().m_initSpecial = 8;
		return;
	case 0x102:
	case 0x112:
	case 0x1102:
	case 0x1112:
		m_specialShotGroup = 0x10;
		v = 0x10;
		if (GOLFDOC()->m_bar.grade == 4)
		{
			if (m_inputCmdTail == 0x11)
			{
				GolfBall().m_initSpecial = 0x11;
				return;
			}
			if (m_inputCmdTail == 0x12)
			{
				GolfBall().m_initSpecial = 0x12;
				return;
			}
			if (m_inputCmdTail == 0x14)
			{
				GolfBall().m_initSpecial = 0x14;
				return;
			}
			if (m_inputCmdTail == 0x18)
			{
				GolfBall().m_initSpecial = 0x18;
				return;
			}
		}
		break;
	case 0x801:
	case 0x811:
	case 0x1801:
	case 0x1811:
		m_specialShotGroup = 0x20;
		v = 0x20;
		if (GOLFDOC()->m_bar.grade == 4)
		{
			if (m_inputCmdTail == 0x11)
			{
				GolfBall().m_initSpecial = 0x21;
				return;
			}
			if (m_inputCmdTail == 0x12)
			{
				GolfBall().m_initSpecial = 0x22;
				return;
			}
			if (m_inputCmdTail == 0x14)
			{
				GolfBall().m_initSpecial = 0x24;
				return;
			}
			if (m_inputCmdTail == 0x18)
			{
				GolfBall().m_initSpecial = 0x28;
				return;
			}
		}
		break;
	case 0x802:
	case 0x812:
	case 0x1802:
	case 0x1812:
		m_specialShotGroup = 0x40;
		v = 0x40;
		if (GOLFDOC()->m_bar.grade == 4)
		{
			if (m_inputCmdTail == 0x11)
			{
				GolfBall().m_initSpecial = 0x41;
				return;
			}
			if (m_inputCmdTail == 0x12)
			{
				GolfBall().m_initSpecial = 0x42;
				return;
			}
			if (m_inputCmdTail == 0x14)
			{
				GolfBall().m_initSpecial = 0x44;
				return;
			}
			if (m_inputCmdTail == 0x18)
			{
				GolfBall().m_initSpecial = 0x48;
				return;
			}
		}
		break;
	default:
		return;
	}
	GolfBall().m_initSpecial = v;
}

void CGolfRule::CheckSpecialShotSpinAndCurve()
{
	if (GolfBall().m_initSpecial != 0)
	{
		float spinX;
		float spinY;
		if ((GolfBall().m_initSpecial & 0x70) != 0)
		{
			if (GolfClub().PowerShotType() == 0 ||
				((GolfBall().m_initSpecial & 0x20) != 0 &&
					GolfClub().GetType() != 0))
				GolfBall().m_initSpecial = 0;
			if (GolfClub().PowerShotType() == 0 ||
				((GolfBall().m_initSpecial & 0x40) != 0 &&
					GolfClub().GetType() != 0))
				GolfBall().m_initSpecial = 0;
		}
		SendMsg(this, "Spin", M983(0x162), (int)&spinX, (int)&spinY, 0, 0);
		if ((GolfBall().m_initSpecial & 1) != 0)
		{
			if (spinY > 0.0f || Wabs(spinY) < g_EPSILON)
				GolfBall().m_initSpecial = m_specialShotGroup;
		}
		else if ((GolfBall().m_initSpecial & 2) != 0)
		{
			if (spinY < 0.0f || WisZero(spinY, g_EPSILON))
				GolfBall().m_initSpecial = m_specialShotGroup;
		}
		else if ((GolfBall().m_initSpecial & 4) != 0)
		{
			float one = -1.0f;
			if (!WisEqual(spinX, one, g_EPSILON))
				GolfBall().m_initSpecial = m_specialShotGroup;
		}
		else if ((GolfBall().m_initSpecial & 8) != 0)
		{
			float one = 1.0f;
			if (!WisEqual(spinX, one, g_EPSILON))
				GolfBall().m_initSpecial = m_specialShotGroup;
		}
		if ((GolfBall().m_initSpecial & 4) != 0)
			GolfBall().m_initCurve = -1.5f;
		else if ((GolfBall().m_initSpecial & 8) != 0)
			GolfBall().m_initCurve = 1.5f;
	}
	GolfBall().m_initSpecial |= m_pendingInitSpecial;
	m_pendingInitSpecial = 0;
}

void CGolfRule::DriveGauge(float dt, bool hit)
{
	float speed = ((CClub::sClub*)*(char**)((char*)&GolfClub() + 4))->barSpeed;
	// 983 0069ab60 le sPlayerData::control em +0x53c (o 0x52c antigo era accuracy)
	C_ASSERT(offsetof(sPlayerData, control) == 0x53c);
	float rate = speed -
		(float)(int)DgPlayer(GOLFDOC()->m_currentPlayer)->control * 10.0f;
	unsigned char ground =
		GetPVS().GetGroundType(GolfBall().m_pos, false, NULL);
	if (!GOLFDOC()->IsCurrentPlayerEvent983(0xe)) // 983 00663870 (flags +0x4e4); o 645 testava item 0x18000028
	{
		unsigned char club;
		if (ground == 3 || ground == 14)
		{
			club = *(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
			switch (club)
			{
			case 0:
				rate = rate * 1.5f; // 983 sem a variante gameType 3 (1.7/1.5/1.3 do 645)
				break;
			case 1:
				rate = rate * 1.3f;
				break;
			case 2:
				rate = rate * 1.2f;
				break;
			}
		}
		else if (ground == 7)
		{
			club = *(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
			switch (club)
			{
			case 0:
				rate = rate * 1.3f;
				break;
			case 1:
				rate = rate * 1.2f;
				break;
			case 2:
				break;
			}
		}
	}
	if (GOLFDOC()->m_bar.dir == 1)
	{
		if (COption::Instance()->dGetClickSystem() != 0)
		{
			float t = (GOLFDOC()->m_bar.gauge - BAR_MIN) * (1.0f / 395.0f);
			if (t > 0.5f)
				t = 0.5f;
			rate = (t * t + t * t + 0.5f) * rate;
		}
	}
	else if (GOLFDOC()->m_shotPhase > 1)
	{
		if (GOLFDOC()->m_bar.bFullPower != 0)
			rate = rate * 0.9375f;
		else
		{
			float norm = (GOLFDOC()->m_bar.power - BAR_START) / g_fBarRange;
			if (norm < 0.5f)
				rate = rate * 0.875f;
			else
			{
				norm = norm - 0.5f;
				rate = (norm * norm + 1.75f) * rate * 0.5f;
			}
		}
	}
	if (DgItem(GOLFDOC()->m_currentPlayer) == 0x1800000b ||
		DgItem(GOLFDOC()->m_currentPlayer) == 0x18000008 ||
		DgItem(GOLFDOC()->m_currentPlayer) == 0x18000003 ||
		DgItem(GOLFDOC()->m_currentPlayer) == 0x1800000f ||
		DgItem(GOLFDOC()->m_currentPlayer) == 0x18000010 ||
		GD_PLAYER_ITEM(PLAYER(GOLFDOC()->m_currentPlayer)) == 0x18000011 ||
		GD_PLAYER_ITEM(PLAYER(GOLFDOC()->m_currentPlayer)) == 0x18000012 ||
		GD_PLAYER_ITEM(PLAYER(GOLFDOC()->m_currentPlayer)) == 0x1800002c ||
		GD_PLAYER_ITEM(PLAYER(GOLFDOC()->m_currentPlayer)) == 0x1800002a ||
		GD_PLAYER_ITEM(PLAYER(GOLFDOC()->m_currentPlayer)) == 0x1800001f)
		rate = rate * 0.5f;
	if (*((unsigned char*)GRPTR983(0x439)) != 0 && GOLFDOC()->m_tutorialMode != 4)
		rate = rate * 0.5f;
	if (hit)
	{
		switch (GOLFDOC()->m_shotPhase)
		{
		case 2:
		{
			if (GOLFDOC()->m_bar.dir != 0xffffffff)
				return;
			CGolfDoc* doc = GOLFDOC();
			float limit = doc->m_bar.power;
			if (limit > 240.0f)
				limit = 240.0f;
			if (!(limit > doc->m_bar.gauge))
				return;
			GOLFDOC()->m_shotPhase += 1;
			// 983 0069ab60: so dispara a tacada; accuracy/efeito ficam em CalcInputCommand/SetSpecialShot
			if (GetBarCondition() == true)
			{
				CalcInputCommand();
				SetSpecialShot();
				CheckSpecialShotSpinAndCurve();
			}
			HitShot();
			return;
		}
		case 1:
		{
			GOLFDOC()->m_shotPhase += 1;
			if (GOLFDOC()->m_bar.calipers > BAR_START)
			{
				GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.calipers;
				GOLFDOC()->m_bar.gauge = GOLFDOC()->m_bar.calipers;
				GOLFDOC()->m_bar.dir =
					(WisEqual(GOLFDOC()->m_bar.calipers, BAR_END, 0.00001f) != 0) - 1;
			}
			GOLFDOC()->m_bar.power = GOLFDOC()->m_bar.gaugeStep;
			if (GOLFDOC()->m_bar.dir == 0)
			{
				GOLFDOC()->m_bar.dir = 0xffffffff;
				g_audio->PlaySfx(g_plSound[0]);
				GOLFDOC()->m_bar.bFullPower = 1;
				SendMsg(this, "Screen", M983(0x1ab), 0, 0, 0, 0);
			}
			else
				g_audio->PlaySfx(g_plSound[2]);
			break;
		}
		}
	}
	CGolfDoc* gauge = GOLFDOC();
	switch (*(int*)GDOCPTR983(gauge, 0xf8))
	{
	case 1:
		*(float*)GDOCPTR983(gauge, 0xe8) =
			rate * dt + *(float*)GDOCPTR983(gauge, 0xe8);
		if (GOLFDOC()->m_bar.gauge > BAR_END)
		{
			GOLFDOC()->m_bar.gauge = 500.0f;
			m_modeDuration = 0.0f;
			GOLFDOC()->m_bar.dir = 0;
		}
		break;
	case 0:
		m_modeDuration = dt + m_modeDuration;
		if (GOLFDOC()->m_shotPhase > 0 && GOLFDOC()->m_shotPhase < 3)
		{
			float limit =
				COption::Instance()->GetOption()->gPowerGauge != 0 /* 645 +0xcc */
				? 0.03f
				: 0.02f;
			if (limit < m_modeDuration)
				GOLFDOC()->m_bar.dir = 0xffffffff;
		}
		break;
	case -1:
		*(float*)GDOCPTR983(gauge, 0xe8) =
			*(float*)GDOCPTR983(gauge, 0xe8) - rate * dt;
		if (GOLFDOC()->m_bar.gauge < BAR_MIN)
			CancelShot();
		break;
	}
	if (*((unsigned char*)GRPTR983(0x439)) != 0 &&
		(int)GOLFDOC()->m_tutorialMode >= 13 && GOLFDOC()->m_bar.dir == 0xffffffff &&
		GOLFDOC()->m_bTutorialGaugeMute == 0)
	{
		IActor* tutorial = GetActor("Tutorial");
		if (tutorial)
			tutorial << MsgObject(NULL, M983(0x155), (int)GOLFDOC()->m_bar.gauge, 0, 0,
				0, 0);
	}
}

void CGolfRule::PuttingGauge(float dt, bool hit)
{
	CClub& club = GolfClub();
	float speed = ((CClub::sClub*)*(char**)((char*)&club + 4))->barSpeed;
	float norm = (GOLFDOC()->m_bar.gauge - BAR_START) / g_fBarRange;
	float rate = (norm * norm + 1.0f) * speed * 0.5f;
	if (hit && (int)GOLFDOC()->m_shotPhase - 1 == 0)
	{
		if (GOLFDOC()->m_bar.calipers > BAR_START)
		{
			GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.calipers;
			GOLFDOC()->m_bar.gauge = GOLFDOC()->m_bar.calipers;
			float diff = GOLFDOC()->m_bar.calipers - BAR_END;
			if (*(unsigned long*)&diff & 0x80000000)
				diff = -diff;
			if (diff < 0.00001f)
				GOLFDOC()->m_bar.dir = 0;
		}
		g_audio->PlaySfx(
			GOLFDOC()->m_bar.dir == 0 ? g_plSound[0] : g_plSound[3]);
		GOLFDOC()->m_shotPhase = GOLFDOC()->m_shotPhase + 1;
		GOLFDOC()->m_bar.power = GOLFDOC()->m_bar.gaugeStep;
		GOLFDOC()->m_bar.dir = 0;
		GOLFDOC()->m_bar.grade = 4;
		HitShot();
		return;
	}
	switch (GOLFDOC()->m_bar.dir)
	{
	case 0:
		if (GOLFDOC()->m_shotPhase > 0)
		{
			m_modeDuration = dt + m_modeDuration;
			float limit =
				COption::Instance()->GetOption()->gPowerGauge != 0 /* 645 +0xcc */
				? 0.03f
				: 0.02f;
			if (limit < m_modeDuration)
				CancelShot();
		}
		break;
	case 1:
		GOLFDOC()->m_bar.gauge = rate * dt + GOLFDOC()->m_bar.gauge;
		if (GOLFDOC()->m_bar.gauge > BAR_END)
		{
			GOLFDOC()->m_bar.gauge = 500.0f;
			GOLFDOC()->m_bar.dir = 0;
			m_modeDuration = 0.0f;
		}
		break;
	}
}

void CGolfRule::DoNavigateCamera(float)
{
	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
	{
		CMouseCursor::Instance()->Store();
		ChangeGameMode(8);
		SendMsg(this, "Camera", M983(0xf0), 0, 0, 0, 0);
	}
	if (TopViewClick())
		ChangeGameMode(0x200);
}

void CGolfRule::DoTopViewCamera(float)
{
	if (!TopViewClick())
		return;
	if (m_topViewMode)
	{
		g_camera = GOLFDOC()->MakeTopViewCameraMatrix(false);
		return;
	}
	// B597: 983 006933c0 nao tem o bloco dos botoes de alarme (645)
	ChangeGameMode(8);
	SCENEMGR()->UpdateFog(false);
}

bool CGolfRule::StandViewClick()
{
	return g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true)
		? true
		: false;
}

bool CGolfRule::TopViewClick()
{
	// A759: crus this+0x48/+0x13c/+0x158 do CTaskMain -> membros (protected: C_ASSERT no escopo de membro)
	C_ASSERT_VC10(offsetof(CGolfRule, m_pOverBarSpecialCard) == 0x48 && offsetof(CGolfRule, m_pMessengerBtn) == 0x13c && offsetof(CGolfRule, m_pKbdManualDlg) == 0x158);
	C_ASSERT_VC10(offsetof(CGolfRule, m_pMessengerBubbleEdit) == 0x128); // B1489: GR_WND(0x128) do ResetUiPos (s_right)
	if (CQuitWindow::Instance()->IsActive() || GOLFDOC()->m_tutorialMode < 1)
		return false;
	if (!CItemWindow::IsClosed())
		return false;
	if (CUserInfo::Instance()->IsVisible())
		return false;
	if (GOLFDOC()->m_tutorialMode == 15 &&
		g_pFresh->GetManager()->HasValidWindow() &&
		m_pKbdManualDlg == NULL)
		return false;
	if (!g_input->GetDown("PAD_0", true) && !g_input->GetDown("NUM_0", true) &&
		(!g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) ||
			(GOLFDOC_UCHAR(0x159) & 2) == 0)) // m_gameMode 0x200; o 983 testa o byte +0x159
	{
		if (CMouseCursor::Instance()->GetArea() != 0)
			return false;
		if (g_input->GetButton(RIGHT_BUTTON) != 1)
			return false;
		if (CMessengerInfo::Instance()->IsGameFocus() != true) // 983 +0xa1 (645: +0x7d; C25)
			return false;
	}
	int prev = m_topViewMode; // B597: 983 nao tem o desvio 0x10a99/HelpMsg 0x16 (645)
	if (g_input->GetDown("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true) &&
		(GOLFDOC_UCHAR(0x159) & 2) != 0) // m_gameMode 0x200 (byte, como o 983)
	{
		m_topViewMode = 0;
	}
	else if (g_input->Get("LCONTROL", true) || g_input->Get("RCONTROL", true))
	{
		m_topViewMode = 3;
	}
	else if ((g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true) ||
				 g_input->Get(1)) &&
		IsMyTurn(true))
	{
		m_topViewMode = 2;
	}
	else if (g_input->GetDown("PAD_0", true) || g_input->GetDown("NUM_0", true))
	{
		m_topViewMode = 1;
	}
	else
	{
		m_topViewMode = 0;
	}
	if (m_topViewMode == prev || (m_topViewMode == 1 && prev >= 2 && prev <= 3))
		m_topViewMode = 0;
	if (m_topViewMode == 0) // B597: 983 00680560
		SendMsg(this, "ShotReview", 0x2fd, (int)"RELEASE_REVIEW_CAMERA", 0, 0, 0);
	return true;
}

// JP 983 (00675e00): as estatisticas da tacada sairam daqui para a regra (CGolfRuleBase::OnHoleIn, slot [9], 006a69d0);
// sobrou a guarda (sem estatistica / replay). Corpo do 645 em out/F3.5/s101/UpdateShotResultStat_645.txt.
void CGolfRule::UpdateShotResultStat()
{
	if (GOLFDOC()->m_bNoStat == 0 &&
		!(IsLocalContent((localContentType_t)0x50) && Doc()->m_playingMode == 2))
		m_pRuleBase->OnHoleIn();
}

void CGolfRule::SendHoleStat()
{
	if (!OnlinePlay())
		return;
	if (IsSharedFlag433Bit1())
		return;
	bool send;
	unsigned char index;
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
	case 7: // 983 (00688010): o tipo 3 saiu da lista (cai no default e nao manda o 0x31); o 645 tinha 0, 3 e 7
	{
		CGolfDoc* doc = Doc()->m_pGolfDoc;
		unsigned char my = MyIndex();
		send = doc->m_currentPlayer == my;
		CGolfDoc* cur = Doc()->m_pGolfDoc;
		index = cur->m_currentPlayer;
		break;
	}
	case 1:
	{
		CGolfDoc* doc = Doc()->m_pGolfDoc;
		unsigned char my = MyIndex();
		send = doc->m_currentPlayer == my ||
			(PLAYER(MyIndex())->teammate != 0xff &&
				IsTeammateTurn(Doc()->m_pGolfDoc));
		index = MyIndex();
		break;
	}
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
		index = 0;
		send = true;
		break;
	default:
		return;
	}
	if (send)
	{
		WSendPacket packet((enumClientPacket)0x31);
		CSharedDoc* shared = Doc();
		sPlayerData* player = shared->m_pGolfDoc->GetPlayer(index);
		Doc()->m_holeStatistics[index].dwShotTime = player->shotTime / 1000;
		packet.EncodeBuffer(&Doc()->m_holeStatistics[index],
			sizeof(sPangYaUserStatistics));
		packet.Send((eSendTo)0);
	}
}

void CGolfRule::SetNetMsgWait(bool bSend, int type, char* msg, float time)
{
	if (bSend)
	{
		m_bNetMsgWait = true;
		m_netMsgWaitType = type;
		m_netMsgWaitTime = time;
		m_pNetMsgWait = msg;
		m_netMsgSendTick = GetTickCount();
		g_iSendPacketGR = g_iSendPacketType;
		g_iRecvPacketGR = 0;
		g_iRecvPacketType = 0;
	}
	else
	{
		m_bNetMsgWait = false;
		m_pNetMsgRecv = msg;
		m_netMsgRecvTick = GetTickCount();
		g_iRecvPacketGR = g_iRecvPacketType;
	}
}

void CGolfRule::CheckNetMsgWait_Send(char* msg)
{
	if (m_pNetMsgWait)
	{
		if (strcmpi(m_pNetMsgWait, msg) == 0)
			g_iSendPacketGR = g_iSendPacketType;
	}
}

void CGolfRule::CheckNetMsgWait_Recv(char* msg)
{
	if (m_pNetMsgWait)
	{
		if (strcmpi(m_pNetMsgWait, msg) == 0)
			g_iRecvPacketGR = g_iRecvPacketType;
	}
}

void CGolfRule::DoToDefaultCamera(float time)
{
	if (OnlinePlay() && !m_bTimeOutSent && Doc()->m_playingMode == 1)
	{
		m_bTimeOutSent = true;
		if (!IsMassGame())
			SetNetMsgWait(true, 0x3440, "DoToDefaultCamera", 20.0f); // 983 (00693490): 0x3440 (0x31ba do 645)
		WSendPacket packet((enumClientPacket)0x1c);
		EncodePacket((enumClientPacket)0x1c, packet);
		packet.Send(TO_GAME);
		CheckNetMsgWait_Send("DoToDefaultCamera");
	}
	unsigned char type = Doc()->m_golfGame.gameType;
	if (type == 10 && !m_bContinueDlg)
	{
		m_bContinueDlg = true;
		m_bShotInFlight = 0;
		m_bTimeOutSent = 0;
		m_bReplayReady = 0;
		m_bFastForwardReady = 0;
		SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
		SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
		SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
		SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
		CFx::Instance()->ClearAll(true);
		g_view->SetFOV(0.9599311f);
		// 983 00693620: zera item (+0x4bc) e +0x4c0 do jogador, sem o aviso ao Player do 645 (antes +0x4b0 = outro campo)
		DcPlayer(GOLFDOC()->m_currentPlayer)->item = 0;
		DcPlayer(GOLFDOC()->m_currentPlayer)->windAbility983 = 0;
		CCameraWork::Instance()->SelectCamera(0xd0); // B1374: 983 00693655 push 0xd1 = ExtraTimeCamera (tabela 006dacb0: 0xd1 -> 006dab27); nosso switch deslocado de 1 (208 = ExtraTime, 209 = ExtraTimeSlow983)
		SendMsg(this, "Player", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Mascot", M983(0x164), 0, 0, 0, 0);
		SendMsg(this, "Bubble", M983(0x164), 0, 0, 0, 0);
		CHeadIcon::Instance()->Hide();
		CProjectG::Instance()->SetGameSpeed(1.0f);
	}
	else
		CHeadIcon::Instance()->Show();
	if (Doc()->m_golfGame.gameType == 10)
		CCameraWork::Instance()->Process(time, 0.0f);
	else
		CCameraWork::Instance()->InputProcess(time);
	if (m_bContinueDlg || m_bNetMsgWait || m_pGoStopDlg != NULL)
		return;
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
		Doc()->m_playingMode == 2)
	{
		int state = Doc()->m_replayState;
		if (state == 7)
			g_input->SetFakeInput("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				false);
		else if (state == 0xc)
		{
			if (!GolfBall().IsHoleIn())
			{
				Doc()->m_replayState = 0;
				g_input->SetFakeInput(
					"\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", false);
			}
		}
		else if (state == 10)
		{
			Doc()->m_replayState = 0;
			g_input->SetFakeInput("\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9",
				false);
		}
		else if (state != 0xe)
			return;
	}
	// 983 (00693620): fora do replay local, Screen 0x227 (modo < 0xf) e ChangeGameMode(0x4000000); o resto e o caso 0x4000000
	if (!(IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
		Doc()->m_playingMode == 2))
	{
		if ((int)GOLFDOC()->m_tutorialMode < 0xf)
			SendMsg(this, "Screen", M983(0x1b5), 0, 0, 0, 0);
		ChangeGameMode(0x4000000);
		return;
	}
	m_bShotInFlight = 0; // 983: short +0x2c0 so no caminho do replay local
	m_bTimeOutSent = 0;
	m_bReplayReady = 0;
	m_bFastForwardReady = 0;
	SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
	SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
	SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
	CFx::Instance()->ClearAll(true);
	g_view->SetFOV(0.9599311f);
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(0);
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
		Doc()->m_playingMode == 2)
	{
		DC_BALL_ULONG(0x68) = 0xffffffff;
		pExpected->pos = m_shotData.pos;
		ball.m_pos = m_shotData.pos;
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		PLAYER(0)->bunker = m_shotData.bunker;
		SendMsg(this, "Ball", M983(0x1ed), 0, 0, 0, 0);
		GOLFDOC()->MakeDefaultCamera(true);
		if (Doc()->m_replayShotIndex == 0)
		{
			ChangeGameMode(2);
			return;
		}
		ChangeGameMode(8);
		return;
	}
}

// 983 00682d90 (chamada por ChangeGameMode(0x4000000)): vai ao proximo turno/tacada depois da tacada
void CGolfRule::DoToDefaultCameraNext983()
{
	*(unsigned short*)&m_bShotInFlight = 0;
	m_bReplayReady = 0;
	m_bFastForwardReady = 0;
	SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
	SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
	SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
	CFx::Instance()->ClearAll(true);
	g_view->SetFOV(0.9599311f);
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(0);
	// 983 00682d90: le/zera o item em +0x4bc (era +0x4b0 do 645) e nao manda msg ao Player aqui
	int item = (int)DcPlayer(GOLFDOC()->m_currentPlayer)->item;
	DcPlayer(GOLFDOC()->m_currentPlayer)->item = 0;
	DC_BALL_ULONG(0x68) = 0xffffffff;
	DC_BALL_ULONG(0x64) = 0;
	DC_BALL_ULONG(0x34) = 0;
	DC_BALL_ULONG(0x38) = 0;
	DC_BALL_ULONG(0x30) = 0;
	DC_BALL_ULONG(0x2c) = 0;
	int mode = GOLFDOC()->m_tutorialMode;
	if (mode < 0xf && mode != 8)
	{
		if (*((unsigned char*)GRPTR983(0x439)) != 0)
		{
			IActor* tutorial = GetActor("Tutorial");
			if (tutorial)
				tutorial << MsgObject(NULL, M983(0x148), 0, 0, 0, 0, 0);
		}
		else
		{
			WVector dist = *(WVector*)GDOCPTR983(GOLFDOC(), 0x9c) - ball.m_pos;
			dist.y = 0.0f;
			float length = dist.Magnitude();
			IActor* tutorial = GetActor("Tutorial");
			if (!tutorial)
				return;
			mode = GOLFDOC()->m_tutorialMode;
			if (mode == 2 || mode == 10 || mode == 0xb || mode == 0xd ||
				mode == 0xe)
			{
				float scale = GOLFDOC()->m_tutorialMode == 2 ? 2.0f : 1.0f;
				if (length < 44.8f * scale)
				{
					SendMsg(this, "GolfRule", M983(0x88), 1, 0, 0, 0);
					switch ((unsigned long)GOLFDOC()->m_tutorialMode)
					{
					case 2:
					case 10:
					case 0xb:
						tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
						return;
					case 0xd:
						if ((GB_ULONG(0x44) & 1) != 0 ||
							(GB_ULONG(0x44) & 2) != 0)
						{
							tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
							return;
						}
						break;
					case 0xe:
						if ((GB_ULONG(0x44) & 4) != 0 ||
							(GB_ULONG(0x44) & 8) != 0)
						{
							tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
							return;
						}
						break;
					}
				}
			}
			else if (mode == 3)
			{
				if (GolfClub().m_powerShot == 1 &&
					(int)GOLFDOC()->m_bar.grade > 1)
				{
					SendMsg(this, "GolfRule", M983(0x88), 1, 0, 0, 0);
					tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
					return;
				}
			}
			else if (mode == 5 && item == 0x18000004)
			{
				if ((int)GOLFDOC()->m_bar.grade > 1)
				{
					SendMsg(this, "GolfRule", M983(0x88), 1, 0, 0, 0);
					tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
					return;
				}
			}
		}
		*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4da) = 0;
		PLAYER(GOLFDOC()->m_currentPlayer)->pos =
			((const CGolfBall&)ball).PreData(0)->pos;
		ball.m_pos = ((const CGolfBall&)ball).PreData(0)->pos;
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		int mode = GOLFDOC()->m_tutorialMode;
		bool bPower = DcIsPowerMode(mode);
		if (bPower)
		{
			SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
			GOLFDOC()->m_cameraData.angle = m_shotData.angle;
			GolfClub().SetPowerShot(0);
			SetAngle(GOLFDOC()->m_cameraData.angle);
			SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
			if (GOLFDOC()->m_tutorialMode == 2)
				SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
			float zero = 0.0f;
			SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
		}
		else
			SetEnvironmentVariableA();
		CMouseCursor::Instance()->Reset(true);
		ChangeTurn();
		ChangeGameMode(8);
	}
	else if (mode == 0xf)
	{
		// 983 modo 0xf = 0067ca80 + (jogador da vez = +0x350) + 006828c0: zera efeitos, avisa Mascot (fora do tipo de chao 2),
		// vai ao proximo turno e termina com Screen 0x217 + Gateway 0x753b (sem Tutorial 0x154)
		SendMsg(this, "Screen", M983(0xa0), 0, 0, 0, 0);
		*((unsigned char*)&GolfClub() + 0x68) = 0;
		*(unsigned short*)&m_bShotInFlight = 0;
		m_bReplayReady = 0;
		m_bFastForwardReady = 0;
		SendMsg(this, "Clock", 0x271a, 0, 0, 0, 0);
		DcPlayer(GOLFDOC()->m_currentPlayer)->item = 0;
		if (Doc()->m_golfGame.gameType == 1)
		{
			CGolfDoc* doc = GOLFDOC();
			TEAM(PLAYER(doc->m_currentPlayer)->team)->shooter = doc->m_currentPlayer;
			TEAM(PLAYER(doc->m_currentPlayer)->team)->club =
				*(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
		}
		GOLFDOC()->m_currentPlayer = m_nextTurn;
		// 983 (006828c0): abre com +1 tacada do jogador da vez no buraco (sem isto o Practice ficava na tacada 1)
		PLAYER(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		const WVector ppos = DcPlayer(GOLFDOC()->m_currentPlayer)->pos;
		// 983 (006828c0): antes do Mascot zera curva/giro e o indice do PreData (renderNum) e poe playFrame = -1
		ball.m_curveRot = 0.0f;
		ball.m_spinRot = 0.0f;
		ball.m_initCurve = 0.0f;
		ball.m_initSpin = 0.0f;
		ball.m_playFrame = -1;
		ball.m_renderNum = 0;
		if (GetPVS().GetGroundType(ppos, true, NULL) != 2)
			SendMsg(this, "Mascot", 0x307, 0, 0, 0, 0);
		ball.PreDataToWrite(0)->pos = ppos;
		ball.m_pos = ppos;
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		// 983 (006828c0): no jogo de time (tipo 1) o +1 tambem vai a tacada do time
		if (Doc()->m_golfGame.gameType == 1)
			TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		SetEnvironmentVariableA();
		if (Doc()->m_golfGame.gameType == 1)
		{
			// 983 (006828c0): repete shooter/club para o jogador da vez ja trocado
			CGolfDoc* doc = GOLFDOC();
			TEAM(PLAYER(doc->m_currentPlayer)->team)->shooter = doc->m_currentPlayer;
			TEAM(PLAYER(doc->m_currentPlayer)->team)->club =
				*(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
		}
		if (SetTurnChangeEvent())
		{
			ChangeTurn();
			GOLFDOC()->MakeDefaultCamera(true);
			ChangeGameMode(4);
			unsigned char gt = Doc()->m_golfGame.gameType;
			if (gt == 4 || gt == 5 || gt == 6 || gt == 9 || gt == 10 || gt == 0xb)
			{
				CGolfDoc* doc = GOLFDOC();
				unsigned char strokes = PLAYER(doc->m_currentPlayer)->stroke[doc->m_currentHole - 1];
				SendMsg(this, "Score", M983(0x134), strokes - GOLFDOC()->GetHoleData().par, 0, 0, 0);
			}
		}
		else
			ReadyForNextTurn();
		SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
		unsigned char gt2 = Doc()->m_golfGame.gameType;
		unsigned long arg = (gt2 == 4 || gt2 == 5 || gt2 == 6 || gt2 == 9 || gt2 == 10 || gt2 == 0xb) ? 0 : m_nextTurn;
		GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 4, arg, 0, 0, 0);
		return;
	}
	else
	{
		SendMsg(this, "Clock", 0x271a, 0, 0, 0, 0);
		if (Doc()->m_golfGame.gameType == 1)
		{
			CGolfDoc* doc = GOLFDOC();
			TEAM(PLAYER(doc->m_currentPlayer)->team)->shooter =
				doc->m_currentPlayer;
			unsigned char club =
				*(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
			TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->club = club;
		}
		GOLFDOC()->m_currentPlayer = m_nextTurn;
		DcPlayer(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] +=
			1;
		pExpected->pos = DcPlayer(GOLFDOC()->m_currentPlayer)->pos;
		ball.m_pos = DcPlayer(GOLFDOC()->m_currentPlayer)->pos;
		ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
		SetEnvironmentVariableA();
		if (SetTurnChangeEvent())
		{
			ChangeTurn();
			GOLFDOC()->MakeDefaultCamera(true);
			ChangeGameMode(4);
			unsigned char gt = Doc()->m_golfGame.gameType;
			if (gt == 4 || gt == 5 || gt == 6 || gt == 9 || gt == 10 || gt == 0xb)
			{
				CGolfDoc* doc = GOLFDOC();
				unsigned char strokes = PLAYER(doc->m_currentPlayer)
											->stroke[doc->m_currentHole - 1];
				SendMsg(this, "Score", M983(0x134),
					strokes - GOLFDOC()->GetHoleData().par, 0, 0, 0);
			}
		}
		else
			ReadyForNextTurn();
	}
	if (*((unsigned char*)GRPTR983(0x439)) != 0)
		return;
	IActor* tutorial = GetActor("Tutorial");
	if (!tutorial || GOLFDOC()->m_tutorialMode == 8)
		return;
	tutorial << MsgObject(NULL, M983(0x154), 0, 0, 0, 0, 0);
}

// JP 983 (00681ff0). O 983 tirou os avisos de matchshot/winningshot por modo do 645: a regra-base ([11], aviso de chance) manda
// primeiro; so se nao avisou vem o "Letter" do turno (Wait / diferenca / Approach 30-60 / AI) e o placar (0x160/0x164).
// Falta o bloco final (tee do buraco inicial, msg 0x268 ao "Ball", FUN_00671c60).
bool CGolfRule::SetTurnChangeEvent()
{
	// 983 00681ff0: o destrutor local (00681600) roda em todo caminho de saida
	sDetermineWhetherToEnableFiledItem fieldItemGuard;
	bool result = false;
	if (!m_pRuleBase->V11_983(&result))
		return result;
	float arg = 2.0f;
	if (Doc()->m_playingMode == 1)
	{
		if (!IsMyTurn(true))
		{
			SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Letter", M983(0x18d), (int)"Wait", 0, 0, 0);
		}
		else
		{
			if (((*(unsigned char*)USERINFO983(GOLFDOC()->m_currentPlayer, 0x3922)) & 1) == 0)
			{
				bool turnMark = false;
				if (*((unsigned char*)&GolfBall() + 0x59) == 2 || (*((unsigned char*)&GolfClub() + 0x3c)) == 4)
					turnMark = GOLFDOC()->m_tutorialMode != 9;
				// 983 00681ff0: teste bruto do registro do buraco (0x1c + hole*0x10): (byte[0xe] ^ byte[0]) == 0xfe, o mesmo do bloco final (nao stroke == 1 do 645)
				const unsigned char* sv0 = (const unsigned char*)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer) + 0x1c + GOLFDOC()->m_currentHole * 0x10;
				bool teeFirst = (unsigned char)(sv0[0xe] ^ sv0[0]) == 0xfe;
				if (*(char*)GDOCPTR983(GOLFDOC(), 0x195) == 0 || GOLFDOC()->m_currentHole != Doc()->m_holeOrder[0] || !teeFirst)
				{
					if (turnMark)
						SendMsg(this, "Letter", M983(0x18d), (int)"\xc2\xf7\xb7\xca\xc7\xa5\xbd\xc3", (int)&arg, 0, 0);
					else if ((*((unsigned char*)&GolfClub() + 0x3c)) < 3)
						SendMsg(this, "Letter", M983(0x18d), (int)"\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
																"30", (int)&arg, 0, 0);
					else if ((*((unsigned char*)&GolfClub() + 0x3c)) == 3)
						SendMsg(this, "Letter", M983(0x18d), (int)"\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1"
																"60", (int)&arg, 0, 0);
				}
			}
			else
				SendMsg(this, "Letter", M983(0x18d), (int)"AI", 0, 0, 0);
			SendMsg(this, "Score", 0x160, 0, 0, 0, 0);
			SendMsg(this, "Score", 0x164, 0, 0, 0, 0);
			if (g_pFresh != 0)
				g_pFresh->GetManager()->CloseForm(false);
		}
	}
	// B583: bloco final do 983 (00681ff0): tee do buraco inicial com stroke 0xfe e valor do jogador na tabela 00e15168 -> msg 0x268 ao Ball
	if (GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0])
	{
		const unsigned char* sv = (const unsigned char*)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer) + 0x1c + GOLFDOC()->m_currentHole * 0x10;
		if ((unsigned char)(sv[0xe] ^ sv[0]) == 0xfe)
		{
			static const unsigned int tbl[14] = {0x08000885, 0x082d6000, 0x0805a81c, 0x08300803, 0x08080832, 0x080d0836, 0x08390005,
												 0x08100038, 0x0815a047, 0x0818e048, 0x081d881e, 0x0821203c, 0x08252013, 0x0829200e};
			const unsigned int* v = (const unsigned int*)(USERINFO983(GOLFDOC()->m_currentPlayer, 0x680e));
			bool found = false;
			for (int i = 0; i < 24 && !found; i++)
				for (int j = 0; j < 14; j++)
					if (v[i] == tbl[j])
					{
						found = true;
						break;
					}
			if (found)
				SendMsg(this, "Ball", 0x268, 0, 0, 0, 0);
		}
	}
	return result;
}

void CGolfRule::CorrectShotResult()
{
	sPlayerData* player = PLAYER(GOLFDOC()->m_currentPlayer);
	if (player->state == 3 || SHOTRES(unsigned char, 0x3d4) == 0)
	{
		CProjectG::Instance()->ReportError(1,
			"CorrectShotResult_%d_%d_%d(%d_%d)", g_iSendPacketGR,
			g_iRecvPacketGR, GOLFDOC()->m_gameMode,
			(unsigned int)*(unsigned short*)(DOCPTR983(0x1094d)),
			Doc()->m_myInfo.info.dwGuid);
		return;
	}
	for (int off = 0x14; off != 0x20; off += 4)
		*(int*)((char*)PLAYER(GOLFDOC()->m_currentPlayer) + off) =
			SHOTRES(int, off + 0x3b4);
	PLAYER(GOLFDOC()->m_currentPlayer)->bunker = SHOTRES(unsigned char, 0x3d5);
	{
		sPlayerData* p = PLAYER(GOLFDOC()->m_currentPlayer);
		p->totalPang = SHOTRES(int, 0x3d7);
	}
	{
		sPlayerData* p = PLAYER(GOLFDOC()->m_currentPlayer);
		p->bonusPang = SHOTRES(int, 0x3db);
	}
	if (Doc()->m_golfGame.gameType == 1)
	{
		sPlayerData* cur = PLAYER(GOLFDOC()->m_currentPlayer);
		if (*((unsigned char*)cur + 0x4ea) != 0xff)
		{
			sPlayerData* mate =
				PLAYER(*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4ea));
			mate->pos = cur->pos;
			sPlayerData* a = PLAYER(GOLFDOC()->m_currentPlayer);
			PLAYER(*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4ea))
				->bunker = a->bunker;
		}
		sPlayerData* b = PLAYER(GOLFDOC()->m_currentPlayer);
		sTeamData* team =
			TEAM(*((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4e9));
		team->pos = b->pos;
		sPlayerData* c = PLAYER(GOLFDOC()->m_currentPlayer);
		sPlayerData* d = PLAYER(GOLFDOC()->m_currentPlayer);
		TEAM(*((unsigned char*)d + 0x4e9))->bunker = c->bunker;
	}
	switch (SHOTRES(unsigned char, 0x3d4))
	{
	case 3:
		*((bool*)&GolfBall() + 0x58) = true;
		GolfBall().SetHoleIn(0);
		break;
	case 4:
		GolfBall().SetHoleIn(1);
		break;
	case 5:
		*((unsigned char*)&GolfBall() + 0xb0) = 0;
	case 2:
		GolfBall().SetHoleIn(0);
		break;
	default:
		GolfBall().m_holeIn = 0;
		break;
	}
	// 983 (0067b060): so a chamada ao OnlinePlay() sobrou; o corte do 645 (m_bFlagCutin, pacote 0xe5 tipo 8, "Cutin")
	// saiu -- no 983 o corte vai no HitShot (00694110)
	OnlinePlay();
}

void CGolfRule::SetEnvironmentVariableA()
{
	SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
		Doc()->m_playingMode == 2)
	{
		GOLFDOC()->m_cameraData.angle = m_shotData.angle;
		WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
		CGolfBall& ball = GolfBall();
		WVector target;
		target.x = ball.m_pos.x + rot.xz;
		target.y = ball.m_pos.y + rot.yz;
		target.z = ball.m_pos.z + rot.zz;
		m_targetPos = target;
		GolfClub().SetPowerShot(m_shotData.clubPower);
		GolfClub().SelectBestClub();
		((CClubEx*)&GolfClub())->SetClub(m_shotData.club);
	}
	else
	{
		CGolfDoc* doc = GOLFDOC();
		if (((*(unsigned char*)USERINFO983(doc->m_currentPlayer, 0x3922)) & 1) == 0)
		{
			if (GOLFDOC()->m_tutorialMode == 2 || GOLFDOC()->m_tutorialMode == 6)
				((CClubEx2*)&GolfClub())->SelectBestClub(m_targetPos);
			else
				GolfClub().SelectBestClub();
			if (GOLFDOC()->m_tutorialMode == 0xf || GOLFDOC()->m_tutorialMode == 8)
			{
				m_targetPos =
					(*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))->GetPoint(NULL);
			}
		}
		else
		{
			((CRivalEx*)CRival::Instance())->SetVariable(doc);
			m_targetPos = CRival::Instance()->m_pos;
		}
		WVector dir = m_targetPos - GolfBall().m_pos;
		GOLFDOC()->m_cameraData.angle = CalcDeltaAngle(dir, WVector::UNIT_POS_Z);
		GolfClub().m_powerShot = 0;
	}
	SetAngle(GOLFDOC()->m_cameraData.angle);
	SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	float zoom = 0.0f;
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) &&
		Doc()->m_playingMode == 2 &&
		(m_shotData.clubPower == 1 || m_shotData.clubPower == 2))
		zoom = 1.0f;
	SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1aa), (int)&zoom, 0, 0, 0);
}

void CGolfRule::SetAngle(float angle)
{
	SendMsg(this, "Camera", 0x2718, (int)&angle, 0, 0, 0);
	SendMsg(this, "Spot", 0x2718, (int)&angle, 0, 0, 0);
	SendMsg(this, "Screen", 0x2718, (int)&angle, 0, 0, 0);
	SendMsg(this, "Grid", 0x2718, (int)&angle, 0, 0, 0);
	SendMsg(this, "Caddie", 0x2718, (int)&angle, 0, 0, 0);
}

WMatrix CGolfRule::BankCamera(float angle)
{
	return RotMat(angle, 2) * g_camera;
}

void CGolfRule::Replay(bool param_1)
{
	CGolfBall& ball = GolfBall();
	const CGolfBall::sExpected* start = ball.PreData(0);
	m_bDriveCamStarted983 = 0;
	m_bShotInFlight = 1;
	ball.m_playFrame = 0;
	ball.m_pos = start->pos;
	ball.m_vel = start->vel;
	ball.m_state = start->state;
	ball.m_special = ball.m_initSpecial;
	SendMsg(this, "ShotPreview", 0x2fd, (int)"PM_REPLAY", 0, 0, 0);
	SendMsg(this, "NPC", M983(0x185), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1b5), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	ReactionForShot();
	// 983 (0067b5e0): Screen 0x219 sempre, sem o ramo 0x21a do 645
	SendMsg(this, "Screen", M983(0x1a7), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1c4), 0, 0, 0, 0);
	SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
	SendMsg(this, "Letter", M983(0x18c), 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
	CFx::Instance()->ClearAll(true);
	SendMsg(this, "Ball", M983(0x1ee), 0, 0, 0, 0);
	g_view->SetFOV(0.9599311f);
	g_camera = ((CGolfDocEx2*)GOLFDOC())->GetDefaultCamera();
	g_audio->StopSfx("shot_dist_count");
	SendMsg(this, "Letter", M983(0x192), 0, 0, 0, 0);
	SendMsg(this, "Player", M983(0x10f), 0, 0, 0, 0);
	m_modeTime = 1.0f;
	GOLFDOC()->m_gameMode = 0x40;
	unsigned long frame = m_pvsFrame[1];
	CQuadTree& pvs = GetPVS();
	pvs.m_frame = frame;
	*(unsigned long*)&pvs.m_frameTime = 0;
	((CCameraWorkEx*)CCameraWork::Instance())->ReplayCamera(param_1);
	GOLFDOC()->m_bReplaying = 1;
	if (CHelpMsg::Instance() != NULL)
		((CHelpMsgEx*)CHelpMsg::Instance())->OnChangeTurn();
}

void CGolfRule::Skip()
{
	m_bShotInFlight = 0;
	m_bReplayReady = 0;
	m_bFastForwardReady = 0;
	SendMsg(this, "Screen", M983(0x1b6), 0, 0, 0, 0);
	SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
	SendMsg(this, "NPC", M983(0x182), 0, 0, 0, 0);
	CGolfBall& ball = GolfBall();
	if (ball.m_bOB && OnlinePlay())
	{
		// 983 00693970 (switchD_00693a5b): switch no tipo 4/5/6/9/10/0xb (nao o IsMassGame do 645)
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb:
			if (CanCancelShot())
				SendShotResult();
		}
	}
	CSharedDoc* shared = Doc();
	if (*((unsigned char*)shared + DOCOFS983(0x10594)) == 10)
	{
		unsigned char me = MyIndex();
		char* src = (char*)*(int*)((char*)shared + DOCOFS983(0x1042c)) + me * 0x1b8;
		shared = Doc();
		unsigned char me2 = MyIndex();
		char* dst = (char*)*(int*)((char*)shared + DOCOFS983(0x1042c)) + me2 * 0x1b8;
		*(unsigned long*)(dst + 0x18c) = *(unsigned long*)(src + 0x190);
		SendMsg(this, "Score", M983(0x2a), 1, 0, 0, 0);
	}
	if (GOLFDOC()->m_gameMode == 0x40)
	{
		if (ball.m_rollFrame > 0)
			SendMsg(this, "Ball", M983(0x1ec), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
		SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
	}
	WPuppet* puppet = SCENEMGR()->GetPuppet("\xb0\xf8");
	if (puppet && puppet->m_rootbone)
		puppet->SetAlpha(0xff);
	g_audio->StopSfx("shot_dist_count");
	CCameraWork::Instance()->StopBallSound();
	UpdateShotResultStat();
	if (ball.m_holeIn == 1)
	{
		RecordScoreByPlayer(GOLFDOC()->m_currentPlayer);
		ChangeGameMode(0x1000);
		CGolfDoc* doc = GOLFDOC();
		WCrypticValue<unsigned char>* stroke;
		if (Doc()->m_golfGame.gameType == 1)
		{
			sPlayerData* player = doc->GetPlayer(doc->m_currentPlayer);
			sTeamData* team = TEAM(player->team);
			doc = GOLFDOC();
			stroke = &team->stroke[doc->m_currentHole - 1];
		}
		else
		{
			sPlayerData* player = doc->GetPlayer(doc->m_currentPlayer);
			doc = GOLFDOC();
			stroke = &player->stroke[doc->m_currentHole - 1];
		}
		AddPrize((unsigned char)*stroke - doc->GetHoleData().par);
		AddExtPrize();
		return;
	}
	if (!ball.m_bOB)
	{
		if (ball.m_holeIn != 1 && ball.m_standGround == 0)
		{
			// 983 00693970: +1 (o 645 descontava)
			PLAYER(GOLFDOC()->m_currentPlayer)
				->stroke[GOLFDOC()->m_currentHole - 1] += 1;
			if (Doc()->m_golfGame.gameType == 1)
				TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
					->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		}
		PLAYER(GOLFDOC()->m_currentPlayer)->flag = 0; // 983 00693e98: +0x4e8 (nao o bunker +0x4da)
	}
	else
	{
		PLAYER(GOLFDOC()->m_currentPlayer)->flag += 1; // 983 00693cc1: +0x4e8 (nao o bunker +0x4da)
		// 983 00693970: strokes +1 (o 645 descontava)
		PLAYER(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] += 1;
		if (Doc()->m_golfGame.gameType == 1)
			TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
				->stroke[GOLFDOC()->m_currentHole - 1] += 1;
	}
	if (m_pRuleBase && m_pRuleBase->CheckGiveUp())
	{
		SendMsg(this, "GuideLine", M983(0x1a2), 0, 0, 0, 0);
		RecordGiveUp(GOLFDOC()->m_currentPlayer);
		RecordScoreByPlayer(GOLFDOC()->m_currentPlayer);
		if (Doc()->m_golfGame.gameType != 1)
		{
			ChangeGameMode(0x80000);
			return;
		}
		ChangeGameMode(0x1000);
		return;
	}
	if (Doc()->m_golfGame.gameType != 1)
	{
		ChangeGameMode(0x800);
		return;
	}
	ChangeGameMode(0x1000);
}

void CGolfRule::Drop(const WVector& pos)
{
	if (GOLFDOC()->m_tutorialMode == 9)
	{
		IActor* tutorial = GetActor("Tutorial");
		if (tutorial)
			tutorial << MsgObject(NULL, M983(0x149), 0, 0, 0, 0, 0);
	}
	CGolfBall::sExpected* pExpected = GolfBall().PreDataToWrite(0);
	PLAYER(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] +=
		1;
	if (Doc()->m_golfGame.gameType == 1)
		TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
			->stroke[GOLFDOC()->m_currentHole - 1] += 1;
	GolfBall().m_pos = pos;
	PLAYER(GOLFDOC()->m_currentPlayer)->pos = pos;
	PLAYER(GOLFDOC()->m_currentPlayer)->bunker = 0;
	pExpected->pos = pos;
	GolfBall().m_meshIndex = GetPVS().GetMeshIndex(pos, false);
	SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
	GolfClub().SelectBestClub();
	m_targetPos = (*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))->GetPoint(NULL);
	WVector dir = m_targetPos - pos;
	GOLFDOC()->m_cameraData.angle = CalcDeltaAngle(dir, WVector::UNIT_POS_Z);
	GolfClub().m_powerShot = 0;
	SetAngle(GOLFDOC()->m_cameraData.angle);
	SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	int zero = 0;
	SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
	SendMsg(this, "Clock", 0x271a, 0, 0, 0, 0);
	SetTurnChangeEvent();
	ReadyForNextTurn();
}

void CGolfRule::ForcedMoveBall(const WVector& pos)
{
	CGolfBall::sExpected* pExpected = GolfBall().PreDataToWrite(0);
	GolfBall().m_pos = pos;
	PLAYER(GOLFDOC()->m_currentPlayer)->pos = pos;
	PLAYER(GOLFDOC()->m_currentPlayer)->bunker = 0;
	pExpected->pos = pos;
	GolfBall().m_meshIndex = GetPVS().GetMeshIndex(pos, false);
	SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
	GolfClub().SelectBestClub();
	m_targetPos = (*(CAutoTarget**)GDOCPTR983(GOLFDOC(), 0x10))->GetPoint(NULL);
	SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	int zero = 0;
	SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
}

void CGolfRule::Cancel()
{
	if (m_backUpStack.empty())
		return;
	CCameraWork* camera = CCameraWork::Instance();
	// B1467: era +0x20c/+0x34/+0x28/+0x30/+0x2c cru (Unlock inline do 983); no cl 14.44 o CCameraWork anda -4/-8
	if (camera->m_bLock)
	{
		camera->m_bLock = 0;
		camera->m_input.button = 0;
		camera->m_input.x = 0;
		camera->m_input.z = 0;
		camera->m_input.y = 0;
	}
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(0);
	sBackUp* back = &m_backUpStack.top();
	sPlayerData* player = PLAYER(0);
	player->pos = back->pos;
	PLAYER(0)->bunker = m_backUpStack.top().bunker;
	back = &m_backUpStack.top();
	PLAYER(0)->gauge = back->gauge;
	back = &m_backUpStack.top();
	PLAYER(0)->totalPang = (int)back->totalPang;
	back = &m_backUpStack.top();
	PLAYER(0)->bonusPang = (int)back->bonusPang;
	back = &m_backUpStack.top();
	PLAYER(0)->pang[GOLFDOC()->m_currentHole - 1] = (int)back->holePang;
	back = &m_backUpStack.top();
	PLAYER(0)->stroke[GOLFDOC()->m_currentHole - 1] = back->holeStroke;
	PLAYER(0)->flag = m_backUpStack.top().flag;
	*(unsigned long*)PLAYER(0) = 0;
	PLAYER(0)->itemNum = 0;
	for (int i = 0x3b46; i < 0x3b6e; i += 4)
	{
		if (*(int*)((char*)&Doc()->m_userInfo[0] + (i - 0x38b0)) != 0)
			PLAYER(0)->itemNum += 1;
	}
	g_audio->StopSfx("shot_dist_count"); // B865: 983 00687050 chama o slot +0x10 (StopSfx), nao PlaySfx
	pExpected->pos = PLAYER(0)->pos;
	ball.m_pos = PLAYER(0)->pos;
	ball.m_meshIndex = GetPVS().GetMeshIndex(ball.m_pos, false);
	ball.m_renderNum = -1;
	GOLFDOC()->m_cameraData.angle = m_backUpStack.top().cameraAngle;
	SendMsg(this, "Screen", M983(0x1a3), 0, 0, 0, 0);
	GolfClub().m_powerShot = 0;
	GolfClub().SelectBestClub();
	((CClubEx*)&GolfClub())->SetClub(m_backUpStack.top().club);
	m_backUpStack.pop();
	WMatrix rot = RotMat(GOLFDOC()->m_cameraData.angle, 1);
	CGolfBall& ball2 = GolfBall();
	WVector target;
	target.x = ball2.m_pos.x + rot.xz;
	target.y = ball2.m_pos.y + rot.yz;
	target.z = ball2.m_pos.z + rot.zz;
	m_targetPos = target;
	SetAngle(GOLFDOC()->m_cameraData.angle);
	SendMsg(this, "Player", M983(0xe2), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	int zero = 0;
	SendMsg(this, "Spot", M983(0xf5), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1aa), (int)&zero, 0, 0, 0);
	SendMsg(this, "Clock", 0x271a, 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
	SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
	CFx::Instance()->ClearAll(true);
	SendMsg(this, "Score", M983(0x132), 0, 0, 0, 0);
	SendMsg(this, "ShotPreview", 0x2fd, (int)"POP_BACK_ITEM", 0, 0, 0);
	SendMsg(this, "ShotReview", 0x2fd, (int)"POP_BACK_ITEM", 0, 0, 0);
	SendMsg(this, "MagicCarpetHelper", M983(0x27f), 0, 0, 0, 0);
	SetTurnChangeEvent();
	ReadyForNextTurn();
	m_modeDuration = 0;
	m_bFastForwardReady = 0;
	m_bShotInFlight = 0;
	m_bReplayReady = 0;
}

void CGolfRule::Warp(const WVector& pos)
{
	int mesh = GetPVS().GetMeshIndex(pos, true);
	if (mesh >= 0 && !GOLFDOC()->m_pPolySoup->IsOutOfBound(pos))
	{
		PLAYER(GOLFDOC()->m_currentPlayer)->pos = pos;
		PLAYER(GOLFDOC()->m_currentPlayer)->bunker = 0;
		GolfBall().m_pos = pos;
		GolfBall().m_meshIndex = mesh;
		SetEnvironmentVariableA();
		SetTurnChangeEvent();
		ReadyForNextTurn();
	}
}

void CGolfRule::FlyBy(float time)
{
	static unsigned int g_flyByIndex;

	unsigned int idx = g_flyByIndex;
	float t = time + m_modeDuration;
	m_modeDuration = t;
	if (5.0f < t)
	{
		idx += 2;
		g_flyByIndex = idx;
		m_modeDuration = t - 5.0;
		if (m_bgPosList.size() - 1U <= idx)
		{
			g_flyByIndex = 0;
			SendMsg(this, "GolfBg", M983(0x1e5), (int)&m_bgPosList, 0, 0, 0);
			idx = g_flyByIndex;
		}
	}
	const WVector* pts = &m_bgPosList[0];
	float u = m_modeDuration / 5.0;
	unsigned int next = idx + 2;
	const WVector* p2 = pts + next;
	float uu = u * u;
	float inv = 1.0f - u;
	float inv2 = inv * inv;
	const WVector* p0 = pts + idx;
	float mid = inv * u + inv * u;
	WVector eye;
	eye.x = inv2 * p0[0].x + mid * p0[1].x + uu * p2->x;
	eye.y = inv2 * p0[0].y + mid * p0[1].y + uu * p2->y;
	eye.z = inv2 * p0[0].z + mid * p0[1].z + uu * p2->z;
	if (m_bgPosList.size() - 1U <= next)
		next = 0;
	const WVector* q0 = pts + next;
	WVector nxt;
	nxt.x = inv2 * q0[0].x + mid * q0[1].x + uu * q0[2].x;
	nxt.y = inv2 * q0[0].y + mid * q0[1].y + uu * q0[2].y;
	nxt.z = inv2 * q0[0].z + mid * q0[1].z + uu * q0[2].z;
	WVector dir(nxt.x - eye.x, nxt.y - eye.y, nxt.z - eye.z);
	float dx = dir.Normalize().x;
	float dz = dir.z;
	float dy = -0.1f;
	unsigned int half = (m_bgPosList.size() - 1U) >> 1;
	if (idx == 0 || idx == half - 4 || idx == half)
	{
		float v = (double)0.8f - m_modeDuration / 6.25;
		v = -0.1 - v;
		dy = -0.9f;
		if (-0.9f <= v)
		{
			dy = v;
			if (-0.1f < v)
				dy = -0.1f;
		}
	}
	else if (idx == half - 6 || idx == half - 2 ||
		idx == m_bgPosList.size() - 3)
	{
		float v = m_modeDuration / 6.25; // 983 00670e3a: fdiv qword 6.25 (d178c0)
		v = -0.1 - v;
		dy = -0.9f;
		if (-0.9f <= v)
		{
			dy = v;
			if (-0.1f < v)
				dy = -0.1f;
		}
	}
	WVector at(eye.x + dx, eye.y + dy, eye.z + dz);
	g_camera = MakeCameraMatrix(eye, at);
}

unsigned char CGolfRule::GetFirstTurn()
{
	if (OnlinePlay() || Doc()->m_playingMode == 2) // 983 0067bb90: replay (playingMode 2) also takes the current player
	{
		if (Doc()->m_golfGame.gameType == 1)
			m_curTeam = PLAYER(GOLFDOC()->m_currentPlayer)->team;
		return GOLFDOC()->m_currentPlayer;
	}
	if (Doc()->m_golfGame.gameType == 1)
		return TEAM(m_curTeam)->teeShooter + m_curTeam * 2;
	unsigned char i;
	for (i = 0; i < 4; i++)
	{
		if (PLAYER(i)->teeOrder == 0)
			return i;
	}
	return i;
}

unsigned char CGolfRule::GetNextTurn()
{
	if (OnlinePlay())
		return m_nextTurn; // 983 0067bc90: devolve o byte +0x350 (turno do servidor), nao o campo 0x2f0 do 645
	WVector pin;
	WVector dist[2];
	unsigned char result = 0;
	unsigned char i = 0;
	if (Doc()->m_golfGame.gameType == 1)
	{
		if (HaveTeeShots())
		{
			unsigned char other = 1 - m_curTeam;
			return other * 2 + TEAM(other)->teeShooter;
		}
		{
			dist[0] = GetHoleCupPos() - TEAM(0)->pos;
			dist[0].y = 0.0f;
			dist[1] = GetHoleCupPos() - TEAM(1)->pos;
			dist[1].y = 0.0f;
			unsigned char next =
				sqrtf(dist[0].z * dist[0].z + dist[0].x * dist[0].x +
					dist[0].y * dist[0].y) <=
				sqrtf(dist[1].z * dist[1].z + dist[1].x * dist[1].x);
			sTeamData* t1 = TEAM(next);
			sTeamData* t2 = TEAM(next);
			t2->turn = 1 - t1->turn;
			return next * 2 + TEAM(next)->turn;
		}
	}
	if (GOLFDOC()->m_playerNum == 1)
		return 0;
	if (HaveTeeShots())
	{
		unsigned char order = PLAYER(GOLFDOC()->m_currentPlayer)->teeOrder;
		if (GOLFDOC()->m_playerNum > 0)
		{
			do
			{
				if (PLAYER(i)->state != 3 && PLAYER(i)->teeOrder == order + 1)
				{
					if (PLAYER(i)->state == 0)
						return result;
					order++;
					result = 0xff;
				}
				result++;
				i = result;
			} while (i < GOLFDOC()->m_playerNum);
			return result;
		}
	}
	else
	{
		if (GOLFDOC()->m_playerNum > 0)
		{
			do
			{
				if (PLAYER(i)->state == 0)
					break;
				result++;
				i = result;
			} while (i < GOLFDOC()->m_playerNum);
		}
		if (result == GOLFDOC()->m_playerNum)
			return GOLFDOC()->m_currentPlayer;
		sPlayerData* player = PLAYER(i);
		pin = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
			GOLFDOC()->m_currentHole * 0x38 - 0x2c);
		i = 0;
		float best = sqrtf((pin.x - player->pos.x) * (pin.x - player->pos.x) +
			(pin.z - player->pos.z) * (pin.z - player->pos.z));
		if (GOLFDOC()->m_playerNum > 0)
		{
			do
			{
				if (PLAYER(i)->state == 0 && i != result)
				{
					sPlayerData* p = PLAYER(i);
					pin = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
						GOLFDOC()->m_currentHole * 0x38 - 0x2c);
					WVector d = pin - p->pos;
					float len = sqrtf(d.x * d.x + d.z * d.z);
					if (best < len)
					{
						best = len;
						result = i;
					}
				}
				i++;
			} while (i < GOLFDOC()->m_playerNum);
		}
	}
	return result;
}

void CGolfRule::AllReplay()
{
	m_replayFlags &= ~4;
	Replay(false);
}

void CGolfRule::HoleReplay()
{
	CGolfBall& ball = GolfBall();
	int frame = ball.m_renderNum - 50;
	ball.m_playFrame = frame;
	ball.m_pos = ball.PreData(frame)->pos;
	ball.m_vel = ball.PreData(frame)->vel;
	ball.m_state = ball.PreData(ball.m_playFrame)->state;
	CQuadTree& pvs = GetPVS();
	int pvsFrame = pvs.m_frame;
	CQuadTree& pvs2 = GetPVS();
	pvs2.m_frame = pvsFrame - 50;
	*(unsigned long*)&pvs2.m_frameTime = 0;
	const WVector* ballPos =
		ball.m_renderNum < 0 ? NULL : (const WVector*)((char*)&ball + 0xcc);
	WVector pin = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
		GOLFDOC()->m_currentHole * 0x38 - 0x2c);
	WVector delta = pin - *ballPos;
	WVector dir(delta.x, 0.0f, delta.z);
	dir.Normalize();
	switch (m_replayState)
	{
	case 1:
	{
		WVector off = dir * 16.0f;
		WVector target = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
			GOLFDOC()->m_currentHole * 0x38 - 0x2c);
		WVector eye = target + off * RotMat(0.7853982f, 1); // 983 00676..6761ca: fld [00d179b0] = +pi/4 (so o caso 2 e -pi/4)
		eye.y = target.y + 9.6f;
		target = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
			GOLFDOC()->m_currentHole * 0x38 - 0x2c);
		g_camera = MakeCameraMatrix(eye, target);
		break;
	}
	case 2:
	{
		WVector off = dir * 9.6f;
		WVector target = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
			GOLFDOC()->m_currentHole * 0x38 - 0x2c);
		WVector eye = target + off * RotMat(-0.7853982f, 1);
		target = *(WVector*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) +
			GOLFDOC()->m_currentHole * 0x38 - 0x2c);
		g_camera = MakeCameraMatrix(eye, target);
		g_camera.ym += 1.0f;
		break;
	}
	default:
		return;
	}
	CCameraWork::Instance()->m_camera = g_camera; // B1465: era +0xf0 cru
}

void CGolfRule::SwingReplay()
{
	CGolfBall& ball = GolfBall();
	const CGolfBall::sExpected* start = ball.PreData(0);
	SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
	SendMsg(this, "Effect", M983(0x178), 0, 0, 0, 0);
	SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
	CFx::Instance()->ClearAll(true);
	if (Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall == 0x14000005)
		SendMsg(this, "Ball", M983(0x1ee), 0, 0, 0, 0);
	SendMsg(this, "Player", M983(0x10f), 0, 0, 0, 0);
	m_bDriveCamStarted983 = 0;
	m_modeTime = 1.0f;
	GOLFDOC()->m_gameMode = 0x40;
	unsigned long frame = m_pvsFrame[1];
	CQuadTree& pvs = GetPVS();
	pvs.m_frame = frame;
	*(unsigned long*)&pvs.m_frameTime = 0;
	ball.m_playFrame = 0;
	ball.m_pos = start->pos;
	ball.m_vel = start->vel;
	ball.m_state = start->state;
	CCameraWork::Instance()->m_replayCount = m_replayState; // 983 00676535 +0x2d8 (645 0x2a4); B1465: era cru
	CCameraWork::Instance()->SelectCamera(0xce);
}

void CGolfRule::AutoReplay()
{
	m_replayFlags = 0;
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb: // 983 00676560: tipo 0xb tambem fica sem replay automatico
		break;
	default:
		if (GetCurMap() != 7 &&
			(GOLFDOC()->m_tutorialMode >= 15 || GOLFDOC()->m_tutorialMode == 8))
		{
			const CGolfBall::sExpected* start = GolfBall().PreData(0);
			WVector pin = GOLFDOC()->GetHoleData().pin;
			float dx = pin.x - start->pos.x;
			float dz = pin.z - start->pos.z;
			float dist = sqrtf(dx * dx + dz * dz);
			if ((GolfBall().m_initSpecial & 0x70) != 0)
				m_replayFlags |= 2;
			if (GolfClub().PowerShotType() != 0 && GOLFDOC()->m_bar.grade == 4 &&
				GOLFDOC()->m_bar.bFullPower != 0)
				m_replayFlags |= 2;
			if (GolfBall().IsHoleIn())
			{
				if (GolfClub().GetType() == 3 && dist > 64.0f)
					m_replayFlags |= 4;
				if (dist > 320.0f)
					m_replayFlags |= 4;
				if ((m_replayFlags & 4) == 0)
				{
					if (dist > 121.6f && dist < 320.0f)
						m_replayFlags |= 1;
					if ((GolfBall().m_special & 3) != 0)
						m_replayFlags |= 1;
				}
			}
			if (GolfBall().m_bOB)
				m_replayFlags &= ~4;
			unsigned char shotType = (unsigned char)GolfClub().PowerShotType();
			if (shotType == 2 || shotType == 3 ||
				(float)GolfBall().m_renderNum < 15.000001f)
				m_replayFlags &= ~2;
		}
		break;
	}
}

void CGolfRule::SaveShot(const char* filename)
{
	if (m_shotData.pvsFrame == 0)
		return;
	if (filename == NULL)
	{
		mkdir("save");
		int i = 0;
		do
		{
			sprintf(m_saveFileName, "save/pangya_%03d.sav", i);
			i++;
		} while (_access(m_saveFileName, 0) != -1);
	}
	else
	{
		strcpy(m_saveFileName, filename);
	}
	FILE* fp = fopen(m_saveFileName, "wb");
	if (fp != NULL)
	{
		unsigned char map;
		unsigned char reserved[0x20] = { 0 };
		SYSTEMTIME time;
		GetLocalTime(&time);
		unsigned char index;
		if (IsMassGame())
			index = 0;
		else
			index = MyIndex();
		CPartTidList* parts = &GOLFDOC()->GetPartTidList(index);
		unsigned long gameId = Doc()->m_replayPlayType;
		fwrite(reserved, 0x20, 1, fp);
		map = GetCurMap();
		fwrite(&map, 1, 1, fp);
		fwrite(&time, 0x10, 1, fp);
		fwrite(&gameId, 4, 1, fp);
		fwrite(USERINFO983(index, 0x38b0), 0x31c6, 1, fp);
		fwrite(parts, 200, 1, fp);
		fwrite(PLAYER(index), 0xdb0, 1, fp); // 983 (00688230): sPlayerData 0xdb0 (645: 0xb98)
		fwrite(&m_shotData, 0x88, 1, fp);
		fwrite(&Doc()->m_courseMap[GetHoleIndex(m_shotData.hole) - 1], 1, 1, fp);
		fwrite(DOCPTR983(0x10a99), 4, 1, fp);
		fclose(fp);
		CChatMsg::Instance()->AddChatMsg(
			MakeStr(
				"%s\xb4\xd4\xc0\xc7 %d\xc8\xa6 %d\xb9\xf8\xc2\xb0 \xbc\xa6 \xc1\xa4\xba\xb8\xb0\xa1 '\xb1\xe2\xb7\xcf\xb5\xc8 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xc5\xd7\xc0\xcc\xc7\xc1'\xbf\xa1 \xc0\xfa\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.",
				DOCPTR983(0x7b8), m_shotData.hole,
				m_shotData.stroke),
			0xffff7878, true, false);
	}
}

void CGolfRule::LoadShot(const char* filename)
{
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		MessageBoxA(NULL,
			MakeStr(
				"\xbc\xbc\xc0\xcc\xba\xea \xc6\xc4\xc0\xcf(%s)\xc0\xbb \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.",
				filename),
			"\xb0\xe6\xb0\xed", 0); // 983 0067c137: titulo "aviso" (o 645 usava "erro")
		g_bQuit = true;
		return;
	}
	unsigned char reserved[0x20] = { 0 };
	unsigned char map = 0;
	CPartTidList parts;
	SYSTEMTIME time;
	unsigned long gameId = 0;
	fread(reserved, 0x20, 1, fp);
	fread(&map, 1, 1, fp);
	fread(&time, 0x10, 1, fp);
	fread(&gameId, 4, 1, fp);
	Doc()->m_replayPlayType = gameId;
	fread(DOCPTR983(0x38b0), 0x31c6, 1, fp);
	fread(&parts, 200, 1, fp);
	fread(PLAYER(0), 0xdb0, 1, fp); // 983: sPlayerData 0xdb0, par do SaveShot
	fread(&m_shotData, 0x88, 1, fp);
	fread(&Doc()->m_courseMap[GetHoleIndex(m_shotData.hole) - 1], 1, 1, fp);
	fread(DOCPTR983(0x10a99), 4, 1, fp);
	fclose(fp);
	SetCurMap(map);
	*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x17c)) = m_shotData.hole;
	*((unsigned char*)DOCPTR983(0x1051e)) = m_shotData.hole;
	((CClubEx*)&GolfClub())->SetClub(m_shotData.club);
	GOLFDOC()->SetPartTidList(0, parts.m_charTid, parts.m_hairColor,
		parts.m_shirtsColor, parts.m_tid, parts.m_uid);
	*((unsigned char*)DOCPTR983(0x10594)) = 0;
	const char* src = *(char**)(DOCPTR983(0xc8));
	char* dst = DOCPTR983(0x10595);
	char c;
	do
	{
		c = *src;
		*dst = c;
		src++;
		dst++;
	} while (c != 0);
	Doc()->m_replayPosition = *(unsigned long*)&m_shotData.bar.gaugeStep;
	Doc()->m_replayPlayType = gameId;
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		RequestUccInfo(*(sUserInfo*)(DOCPTR983(0x38b0)));
}

// 983: o 7o cartao, quando e de caddie, so conta com a peca certa (0066f7e0; copia do cardsystem.cpp 00809950)
static bool IsSeventhCaddieCardGr983(unsigned long tid, int number)
{
	if (number == 7 && (tid & 0x3c00000) == 0x400000)
		return true;
	return false;
}

// JP 983 (006884f0). Diferencas para o 645: sem o modo anonimo (1 argumento), a ficha inteira do jogador
// (sUserInfo, 0x31c6), cartas ativas do personagem antes das de periodo, aviso 0x753b ao Gateway com o arquivo
// aberto e o bloco "extension" da fita (ReplayWriteExtension983, frente B) antes do fclose.
int CGolfRule::ShotDataSave(char const* filename)
{
	if (*(int*)((char*)&m_shotData + 0x50) == 0) // 983: +0x3a4 (645: +0x344)
		return 0;
	if (filename == NULL)
	{
		mkdir("save");
		int i = 0;
		do
		{
			if (i >= 100)
				return -1;
			sprintf(m_saveFileName, "save/pangya_%03d.sav", i);
			i++;
		} while (_access(m_saveFileName, 0) != -1);
	}
	else
	{
		strcpy(m_saveFileName, filename);
	}
	FILE* fp = fopen(m_saveFileName, "wb");
	if (fp == NULL)
		return 0;
	unsigned char map = GetCurMap();
	SYSTEMTIME time;
	GetLocalTime(&time);
	unsigned char index;
	if (IsMassGame())
		index = 0;
	else
		index = MyIndex();
	CPartTidList* parts = &GOLFDOC()->GetPartTidList(index);
	CSharedDoc* shared = Doc();
	int shotCount = (int)shared->m_shotDataList.size();
	int playType = shared->m_replayPlayType;
	unsigned char hole = *((unsigned char*)shared + (offsetof(CSharedDoc, m_courseMap) - 1) + // 983 (645: 0x45cd); nome nao levantado
		GetHoleIndex(*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x17c)))); // 983: CGolfDoc+0x17c (645: +0x168)
	unsigned long uid = shared->m_myInfo.info.dwUID;
	fwrite(&shared->m_replayVersion, 4, 1, fp);
	fwrite(&uid, 4, 1, fp);
	fwrite(&map, 1, 1, fp);
	fwrite(&time, 0x10, 1, fp);
	fwrite(&playType, 4, 1, fp);
	fwrite(&Doc()->m_userInfo[index], sizeof(sUserInfo), 1, fp);
	fwrite(parts, 200, 1, fp);
	fwrite(&hole, 1, 1, fp);
	fwrite(GOLFDOC()->GetPlayer(index), 0xdb0, 1, fp); // 983: sPlayerData 0xdb0 (645: 0xb98)
	fwrite(DOCPTR983(0x10a99), 4, 1, fp); // 983 (645: 0x4afd); nome nao levantado
	if (playType == 1)
	{
		fwrite(&m_shotData, 0x88, 1, fp);
	}
	else if (playType == 2)
	{
		fwrite(&shotCount, 4, 1, fp);
		std::list<sShotData>& list = Doc()->m_shotDataList;
		for (std::list<sShotData>::iterator it = list.begin(); it != list.end();
			it++)
			fwrite(&*it, 0x88, 1, fp);
		Doc()->m_shotDataList.clear();
	}
	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
	{
		std::list<sSCardAvilityPeriodInfo> cards;
		unsigned long* pTid = &Doc()->m_userInfo[index].charInfo.card[0][0];
		for (int k = 0; k < 12; ++k, ++pTid)
		{
			if (*pTid == 0)
				continue;
			IFF_STRUCT::sCard* pCard = Doc()->m_itemManager.FindCard(*pTid);
			if (pCard == NULL)
				continue;
			bool bCaddie7 = IsSeventhCaddieCardGr983(pCard->c.TypeId, k + 1);
			bool bSlot7 = Doc()->HasSeventhCardSlot983(&Doc()->m_userInfo[index].charInfo);
			if (bCaddie7 && !bSlot7)
				continue;
			sSCardAvilityPeriodInfo info;
			memset(&info, 0, sizeof(info));
			info.tid = pCard->c.TypeId;
			info.cardType = (pCard->c.TypeId >> 22) & 0xf;
			info.slotNum = k;
			info.Avility = pCard->Avility;
			info.AvilityValue = pCard->AvilityValue;
			cards.push_back(info);
		}
		int count = (int)cards.size() + (int)Doc()->m_cardAbilityList[0].size();
		fwrite(&count, 4, 1, fp);
		if (count > 0)
		{
			for (std::list<sSCardAvilityPeriodInfo>::iterator it = cards.begin(); it != cards.end(); ++it)
				fwrite(&*it, sizeof(sSCardAvilityPeriodInfo), 1, fp);
			for (CARDABILITY_LIST_IT it = Doc()->m_cardAbilityList[0].begin();
				it != Doc()->m_cardAbilityList[0].end(); ++it)
				fwrite(&*it, sizeof(sSCardAvilityPeriodInfo), 1, fp);
		}
	}
	GetActor(s_gatewayActor[1]) << MsgObject(NULL, 0x753b, 0xa, (int)fp, 0, 0, 0);
	ReplayWriteExtension983(fp, 4, &time);
	fclose(fp);
	return 1;
}

bool CGolfRule::ShotDataLoad(char const* filename)
{
	size_t len = strlen(filename);
	FILE* fp = fopen(len == 0 ? "D:/work/ProjectG_S4/bin/save/pangya_003.sav"
							  : filename,
		"rb");
	if (fp == NULL)
	{
		g_bQuit = true;
		return false;
	}
	unsigned long tag = 0;
	unsigned char map = 0;
	CPartTidList parts;
	int shotCount = 0;
	unsigned char hole = 0;
	int gameId = 0;
	unsigned long gameTag = 0xffffffff;
	sShotData data;
	memset(&m_shotData, 0, sizeof(m_shotData));
	Doc()->m_shotDataList.clear();
	Doc()->m_replayShotIndex = 0;
	SYSTEMTIME time;
	fread(&tag, 4, 1, fp);
	fread(&gameTag, 4, 1, fp);
	fread(&map, 1, 1, fp);
	fread(&time, 0x10, 1, fp);
	fread(&gameId, 4, 1, fp);
	Doc()->m_replayPlayType = gameId;
	fread(DOCPTR983(0x38b0), 0x31c6, 1, fp);
	fread(&parts, 200, 1, fp);
	fread(&hole, 1, 1, fp);
	fread(PLAYER(0), 0xdb0, 1, fp); // 983 0064fee5: sPlayerData 0xdb0, par do ShotDataSave (645: 0xb98)
	fread(DOCPTR983(0x10a99), 4, 1, fp);
	if (gameId == 1)
	{
		fread(&m_shotData, 0x88, 1, fp);
		Doc()->m_replayShotIndex = m_shotData.holeStroke;
	}
	else if (gameId == 2)
	{
		fread(&shotCount, 4, 1, fp);
		int i = 0;
		if (shotCount > 0)
		{
			do
			{
				fread(&data, 0x88, 1, fp);
				if (i == 0)
				{
					memcpy(&m_shotData, &data, sizeof(sShotData));
					Doc()->m_replayShotIndex = 0;
					Doc()->m_replayHoleStroke =
						m_shotData.holeStroke;
					PLAYER(0)->gauge =
						m_shotData.gauge;
					unsigned long pvsFrame =
						m_shotData.pvsFrame;
					CQuadTree& pvs = GetPVS();
					pvs.m_frame = pvsFrame;
					*(unsigned long*)&pvs.m_frameTime = 0;
				}
				Doc()->m_shotDataList.push_back(data);
				i++;
			} while (i < shotCount);
		}
	}
	if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
	{
		int count = 0;
		fread(&count, 4, 1, fp);
		if (count > 0)
		{
			std::list<sSCardAvilityPeriodInfo>& list =
				Doc()->m_cardAbilityList[2]; // 983: CSharedDoc+0x3804 = [2] (list 0xc no VC10)
			list.clear();
			int i = 0;
			if (count > 0)
			{
				do
				{
					sSCardAvilityPeriodInfo info;
					fread(&info, 0x41, 1, fp);
					list.push_back(info);
					i++;
				} while (i < count);
			}
		}
		// B1467: +0x3a5 cru andava -4 no cl 14.44 (std::string do BaseObject); fica dentro de m_charInfo (0x2d..0xa32)
		C_ASSERT_VC10(offsetof(CCardManager, m_charInfo) == 0x2d);
		memcpy((char*)CCardManager::Instance()->m_charInfo + (0x3a5 - 0x2d), DOCPTR983(0x6802),
			0x6f * 4);
		CCardManager::Instance()->CalcCardPeriodAndStatus();
		CCardManager::Instance()->SetPlayerIndex(2);
	}
	fclose(fp);
	SetCurMap(map);
	*((unsigned char*)DOCPTR983(0x1051e)) = m_shotData.hole;
	unsigned int idx = 0;
	unsigned int next;
	unsigned char n;
	do
	{
		n = (unsigned char)idx + 1;
		next = n;
		if (Doc()->m_holeOrder[idx] == m_shotData.hole)
			goto found;
		idx = next;
	} while (n < 0x12);
	next = 0x13;
found:
	Doc()->m_courseMap[next - 1] = hole;
	*((unsigned char*)GDOCPTR983(GOLFDOC(), 0x17c)) = m_shotData.hole;
	GOLFDOC()->SetPartTidList(0, parts.m_charTid, parts.m_hairColor,
		parts.m_shirtsColor, parts.m_tid, parts.m_uid);
	*((unsigned char*)DOCPTR983(0x10594)) = 0;
	const char* src = *(char**)(DOCPTR983(0xc8));
	char* dst = DOCPTR983(0x10595);
	char c;
	do
	{
		c = *src;
		*dst = c;
		src++;
		dst++;
	} while (c != 0);
	Doc()->m_replayPosition = *(unsigned long*)&m_shotData.bar.gaugeStep;
	Doc()->m_replayPlayType = gameId;
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		RequestUccInfo(*(sUserInfo*)(DOCPTR983(0x38b0)));
	return true;
}

void CGolfRule::SaveGame(int type)
{
	if (type == 0)
	{
		mkdir("save");
		int i = 0;
		do
		{
			sprintf(m_saveFileName, "save/save_%03d.dat", i++);
		} while (_access(m_saveFileName, 0) != -1);
	}
	FILE* fp = fopen(m_saveFileName, "ab");
	if (fp)
	{
		switch (type)
		{
		case 0:
		{
			unsigned char map = GetCurMap();
			fwrite(&map, 1, 1, fp);
			int holeOrder = Doc()->m_holeOrder[0];
			fwrite(&holeOrder, 4, 1, fp);
			unsigned char playerNum = GOLFDOC()->m_playerNum;
			fwrite(&playerNum, 1, 1, fp);
			sPlayerData* players = PLAYER(0);
			fwrite(players, sizeof(sPlayerData), playerNum, fp);
			break;
		}
		case 1:
			fwrite(&Wind().GetGlobalWind(), sizeof(WVector), 1, fp);
			break;
		case 2:
			fwrite(&m_shotData, sizeof(sShotData), 1, fp);
			break;
		}
		fclose(fp);
	}
}

void CGolfRule::LoadGame(int type)
{
	static long s_loadOffset;

	FILE* fp = fopen("save/save_000.dat", "rb");
	if (fp == NULL)
	{
		g_bQuit = true;
		return;
	}
	fseek(fp, s_loadOffset, SEEK_SET);
	switch (type)
	{
	case 0:
	{
		unsigned char map;
		fread(&map, 1, 1, fp);
		SetCurMap(map);
		unsigned char playerNum = GOLFDOC()->m_playerNum;
		fread(&GOLFDOC()->m_currentHole, 1, 1, fp);
		fread(&playerNum, 1, 1, fp);
		sPlayerData* players = PLAYER(0);
		fread(players, sizeof(sPlayerData), playerNum, fp);
		break;
	}
	case 1:
	{
		WVector wind; // 983 (0067c5a5): local; o CGolfRule do 983 não tem m_wind
		fread(&wind, sizeof(WVector), 1, fp);
		Wind().SetWind(wind);
		break;
	}
	case 2:
		fread(&m_shotData, sizeof(sShotData), 1, fp);
		break;
	}
	s_loadOffset = ftell(fp);
	fseek(fp, 0, SEEK_END);
	ftell(fp);
	fclose(fp);
}

void CGolfRule::QuickSave(const char* name)
{
	FILE* fp = fopen(MakeStr("%s.sav", name), "wb");
	unsigned char map = GetCurMap();
	fwrite(&map, 1, 1, fp);
	fwrite(&GOLFDOC()->m_currentHole, 1, 1, fp);
	unsigned char playerNum = GOLFDOC()->m_playerNum;
	fwrite(&playerNum, 1, 1, fp);
	sPlayerData* players = PLAYER(0);
	fwrite(players, sizeof(sPlayerData), playerNum, fp);
	fwrite(&GOLFDOC()->m_currentPlayer, 1, 1, fp);
	fclose(fp);
}

void CGolfRule::QuickLoad(char* fileName)
{
	FILE* file = fopen(MakeStr("%s.sav", fileName), "rb");
	if (file == NULL)
		return;
	unsigned char head[4];
	fread(&head[1], 1, 1, file);
	fread(&head[2], 1, 1, file);
	head[3] = GOLFDOC()->m_playerNum;
	fread(&head[3], 1, 1, file);
	sPlayerData* players = PLAYER(0);
	fread(players, 0xdb0, head[3], file); // 983 00693fea: sPlayerData 0xdb0, par do QuickSave (645: 0xb98)
	fread(&GOLFDOC()->m_currentPlayer, 1, 1, file);
	fclose(file);
	if (head[1] != GetCurMap())
	{
		SetCurMap(head[1]);
		GOLFDOC()->m_currentHole = head[2];
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
		SendMsg(this, "GolfBg", M983(0x13e), 0, 0, 0, 0);
	}
	else if (head[2] != GOLFDOC()->m_currentHole)
	{
		GOLFDOC()->m_currentHole = head[2];
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
	}
	SendMsg(this, "Player", M983(0x116), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x116), 0, 0, 0, 0);
	SetEnvironmentVariableA();
	GOLFDOC()->MakeDefaultCamera(true);
	ChangeGameMode(8);
	delete[] fileName;
}

// JP 983: auxiliares do CSharedDoc usados no corte do HitShot. No 983 sao metodos do CSharedDoc (unidade da A); ficam
// estaticos aqui ate a A declarar (pedido C -> A em out/PEDIDOS.md).
// 0062be00: personagem do jogador da sala (so se o tid for de personagem, 0x04000000)
// B1373: 0062be00 = idx * 0x31c6 + 0x6802, mascara 0xfc000000 == 0x4000000, corte em > 4
C_ASSERT_VC10(sizeof(sUserInfo) == 0x31c6 && offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, charInfo) == 0x6802);
static sCharacterInfo* CharOfPlayer983(short player)
{
	if (player > 4)
		return NULL;
	sCharacterInfo* ch = &Doc()->m_userInfo[player].charInfo;
	return (ch->tid & 0xfc000000) == 0x4000000 ? ch : NULL;
}
// 0063bb30: tid do item de corte com esse guid (lista m_cutinList)
static unsigned long CutinTidOfGuid983(unsigned long guid)
{
	if (guid == 0xffffffff)
		return 0;
	std::list<sItemInfo>& list = Doc()->m_cutinList;
	for (std::list<sItemInfo>::iterator it = list.begin(); it != list.end(); ++it)
	{
		if (S5::ISCutinItem(it->tid) && guid == it->guid)
			return it->tid;
	}
	return 0;
}
// 0063f6c0: o personagem tem carta de corte para esse tipo de tacada?
static bool HasCharCutin983(short type, unsigned char player)
{
	sCharacterInfo* ch = player == 0xff ? Doc()->FindChar983(0xffffffff) : CharOfPlayer983(player);
	if (ch == NULL)
		return false;
	for (unsigned int i = 0; i < 4; i++)
	{
		IFF_STRUCT::S5::sCutinInformation* info = ItemManager()->FindCutinInformation(CutinTidOfGuid983(ch->cutIn[i]));
		if (info != NULL && (ch->tid & 0x3ffffff) == info->CharIndex && (info->Condition & type) != 0)
			return true;
	}
	return false;
}
// 0062be80: item de corte escolhido pelo jogador (CSharedDoc+0x11448 = m_playerCutinItem983)
static unsigned long PlayerCutinItem983(unsigned char player)
{
	return Doc()->m_playerCutinItem983[player];
}

// Campos do CGolfDoc do 983 ainda sem nome (dentro de m_unusedcd, 0xdd..0x103; o 645 lia 0xd0/0xd4/0xe0/0xec)
C_ASSERT_VC10(offsetof(CGolfDoc, m_unusedcd) == 0xdd);
#define GOLFDOC983_SHOTF0 (GOLFDOC()->m_bar.power) // +0xe0 (s94: o GetBar() ja e o do 983)
#define GOLFDOC983_SHOTF1 (GOLFDOC()->m_bar.impact) // +0xe4
#define GOLFDOC983_SHOTTYPE (*(unsigned long*)&GOLFDOC()->m_bar.grade) // +0xf0
#define GOLFDOC983_SHOTF3F (GOLFDOC()->m_bar.center) // +0xfc

#pragma pack(push, 1)
// JP 983 (00694474..006945f9): bloco de 0x32 do pacote 0x12 (o 645 mandava 0x2e)
struct sHitShotInfo983
{
	float field0;           // +0x00 CGolfDoc+0xe0
	float field1;           // +0x04 CGolfDoc+0xe4
	float spinX;            // +0x08 bola +0x34
	float spinY;            // +0x0c bola +0x38
	unsigned char type;     // +0x10 CGolfDoc+0xf0
	unsigned long ballFlags; // +0x11 bola +0x44
	unsigned long mesh;     // +0x15 CQuadTree+0x21bc
	unsigned long player;   // +0x19 preenchido pelo ator "Player" (0x13e)
	int time;               // +0x1d
	float field3f;          // +0x21 CGolfDoc+0xfc
	unsigned char clubType; // +0x25 m_pClub+0x21
	float clubValue;        // +0x26 CClub+0x64
	float extra;            // +0x2a CGolfRule+0x2d4
	float pangyaArea;       // +0x2e novo no 983 (00694d6: area de PangYa da barra)
};
#pragma pack(pop)
C_ASSERT(sizeof(sHitShotInfo983) == 0x32);
C_ASSERT_VC10(offsetof(CGolfRule, m_shotData) == 0x354 && offsetof(CGolfRule, m_shotExtra983) == 0x2d4);
C_ASSERT_VC10(offsetof(CGolfRule, m_dbgEye983) == 0x248 && offsetof(CGolfRule, m_camPivot) == 0x254 && offsetof(CGolfRule, m_goStopTime983) == 0x264);
C_ASSERT_VC10(offsetof(CGolfRule, m_savedGameMode983) == 0x268 && offsetof(CGolfRule, m_savedCamera983) == 0x26c);
C_ASSERT_VC10(offsetof(CGolfRule, m_bDriveCamStarted983) == 0x2a4 && offsetof(CGolfRule, m_bUnplayable983) == 0x2a5);
C_ASSERT_VC10(offsetof(CGolfRule, m_holePos983) == 0x2b4 && offsetof(CGolfRule, m_pCamArea983) == 0x2cc && offsetof(CGolfRule, m_bSpecialCamDone983) == 0x2d0);
C_ASSERT_VC10(offsetof(CGolfRule, m_pPremiumBmp) == 0x5a0 && offsetof(CGolfRule, m_pvsFrame) == 0x5b0);
C_ASSERT(sizeof(sMassGameShot) == 0x57);

// JP 983 (00635ef0): propriedade do mascote do jogador -- 0 = DriveUp (somado), 1..3 = ItemDropUp/ComboUp/PangUp em %
// (multiplicados), 4 = tid sem o tipo. Metodo do CSharedDoc no 983; estatico aqui ate a A declarar em shareddoc.h.
float __fastcall MascotProperty983(unsigned char player, unsigned char type)
{
	if (Doc()->m_pGolfDoc && GOLFDOC()->m_playerNum <= player)
	{
		if (type == 0)
			return 0.0f;
		return 1.0f;
	}
	float value = (type == 0) ? 0.0f : 1.0f;
	IFF_STRUCT::sMascot* pMascot = Doc()->m_itemManager.FindMascot(Doc()->m_userInfo[player].mascotInfo.tid);
	if (pMascot == NULL)
		return value;
	switch (type)
	{
	case 0:
		return pMascot->DriveUp + value;
	case 1:
		if (pMascot->ItemDropUp != 0)
			return (float)(pMascot->ItemDropUp / 100.0 * value);
		break;
	case 2:
		if (pMascot->ComboUp != 0)
			return (float)(pMascot->ComboUp / 100.0 * value);
		break;
	case 3:
		if (pMascot->PangUp != 0)
			return (float)(pMascot->PangUp / 100.0 * value);
		break;
	case 4:
		value = (float)(pMascot->c.TypeId & 0x3ffffff);
		break;
	}
	return value;
}

// JP 983 (0067d970): registro de 0x57 da tacada para o jogo em massa (sMassGameShot). Refaz na bola global o lancamento
// do ChangeGameMode(0x40) -- forca, BallSpeedFactor, curva e efeito --, copia o resultado e devolve a bola como estava.
// Metodo da regra no 983; estatico aqui ate a A declarar em golfrule.h. A barra do 983 fica em CGolfDoc+0xe0
// (GOLFDOC983_SHOTF0 = power, GOLFDOC983_SHOTTYPE = grade; desde a s94 sao membros do m_bar, +0xe0 no 983).
static sMassGameShot* MakeMassGameShot983(CGolfRule* rule, sMassGameShot* shot)
{
	memset(shot, 0, sizeof(sMassGameShot));
	CGolfBall saved;
	WVector pos;
	saved.Reset(pos);
	saved = GolfBall();
	float t = (GOLFDOC983_SHOTF0 - BAR_START) / g_fBarRange;
	shot->ratio = t;
	float vel = GolfClub().GetPower() * sqrtf(t);
	vel = GetBarClickPowerFactor() * (GolfBall().GetPowerFactor() * vel);
	shot->power = vel;
	shot->pos.x = GolfBall().m_pos.x;
	shot->pos.y = GolfBall().m_pos.y;
	shot->pos.z = GolfBall().m_pos.z;
	shot->state = (unsigned char)GetInitBallState();
	shot->clubPower = GolfClub().GetPower();
	shot->clubRange = GolfClub().GetRange(NULL);
	shot->clubId = GolfClub().m_pClub->id;
	BallSpeedFactor bsf(Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid, &GolfBall(), rule->m_shotExtra983);
	GolfBall().m_initCurve = bsf.f8;
	GolfBall().m_initSpin = bsf.fc;
	WVector launch = bsf.vel;
	if ((int)GOLFDOC983_SHOTTYPE > 1)
	{
		launch = (((Abs<float>(GolfBall().m_initCurve) * 0.1f + 1.0f) * vel) *
					 RotMat(GolfClub().GetLie(NULL), 0).za) *
			RotMat(bsf.f4 + bsf.f0, 1);
	}
	CGolfBall& ball = GolfBall();
	ball.m_vel = launch;
	ball.m_state = (CGolfBall::eBallState)shot->state;
	ball.m_special = ball.m_initSpecial;
	if (GOLFDOC983_SHOTTYPE == 1)
	{
		ball.m_spin = 0.0f;
		ball.m_curve = 0.0f;
	}
	else
	{
		if (GolfClub().GetApproachMode() == 4)
		{
			// 983: pecas auxiliares + bonus de conjunto (006362a0) + mascote, cada um pela metade; cartoes (sem o ramo 0x70
			// do 645)
			float range = Doc()->GetAuxPartProperty(GOLFDOC()->m_currentPlayer, 0) * 0.5f;
			range = Doc()->GetRangeBonus983(GOLFDOC()->m_currentPlayer) * 0.5f + range;
			range = MascotProperty983(GOLFDOC()->m_currentPlayer, 0) * 0.5f + range;
			if (IsLocalContent((localContentType_t)0x52 /*983: era 0x51; medido*/))
			{
				CCardManager::Instance()->SetPlayerIndex(GOLFDOC()->m_currentPlayer);
				float bonus = (float)(CCardManager::Instance()->GetCardRangeUp() + 0.0);
				if (CLUB_PS() != 0)
					bonus = CCardManager::Instance()->GetCardPowerRangeUp() + bonus;
				range = bonus + range;
			}
			float add = (float)(int)GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->power;
			ball.m_spin = (Max<float>(0.0f, (float)((add + range) / 170.0)) + 1.5f) * t * t;
		}
		else
			ball.m_spin = 0.0f;
		ball.m_curve = t * ball.m_initCurve;
	}
	shot->vel.x = ball.m_vel.x;
	shot->vel.y = ball.m_vel.y;
	shot->vel.z = ball.m_vel.z;
	shot->initCurve = ball.m_initCurve;
	shot->initSpin = ball.m_initSpin;
	shot->special = ball.m_initSpecial;
	shot->spin = ball.m_spin;
	shot->curve = ball.m_curve;
	shot->wind.x = Wind().GetGlobalWind().x;
	shot->wind.y = Wind().GetGlobalWind().y;
	shot->wind.z = Wind().GetGlobalWind().z;
	// 983: CQuadTree+0x21bc (ver HitShot)
	shot->frame = GetPVS().m_frame; // B1501: ex-cru &GetPVS()+0x21bc (anda -4 no cl 14.44: m_water STL antes)
	// 983: CClub+0x80 = bloco de 0x6c de SetShotOverride983 (006ade10); sem ele, o taco escolhido
	CClub::sClub* over = GolfClub().m_pShotOverride983;
	shot->clubB = over != NULL ? (float)over->spin : (float)GolfClub().m_pClub->spin;
	over = GolfClub().m_pShotOverride983;
	shot->clubA = over != NULL ? (float)over->curve : (float)GolfClub().m_pClub->curve;
	GolfBall() = saved;
	return shot;
}

// JP 983 (0067e140): pacote 0x12f com o registro de 0x57 nos jogos em massa (tipos 4, 5, 6, 9, 10 e 11). Metodo da regra
// no 983; estatico aqui como o de cima.
static void SendMassGameShot983(CGolfRule* rule)
{
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
	{
		sMassGameShot shot;
		MakeMassGameShot983(rule, &shot);
		WSendPacket packet((enumClientPacket)0x12f);
		packet.EncodeBuffer(&shot, sizeof(sMassGameShot));
		packet.Send(TO_GAME);
		break;
	}
	}
}

// JP 983 (00694110)
void CGolfRule::HitShot()
{
	CSharedDoc* shared = Doc();
	if (shared->m_playingMode != 2 && !IsMyTurn(false))
		return;
	timeEndPeriod(1);
	CFx::Instance()->SetActive(true);
	// 983: no tipo de jogo 0xb avisa o ator "Gateway" (0x7535, sub 8)
	if (Doc()->m_golfGame.gameType == 0xb)
		AfxGetTask()->GetActor("Gateway") << MsgObject(NULL, 0x7535, 8, 0, 0, 0, 0);
	// 983 (00677760): desvio aleatorio do efeito; sem o IsLocalContent do 645
	if (GolfBall().m_initSpecial > 0 && GOLFDOC983_SHOTTYPE != 4 && GolfClub().GetType() != 3)
	{
		float wobble = (float)(rand() % 3 + 2);
		if (rand() % 2 == 0)
			wobble = wobble * -1.0f;
		GolfBall().m_initSpin = wobble / 10.0f;
		wobble = (float)(rand() % 3 + 2);
		if (rand() % 2 == 0)
			wobble = wobble * -1.0f;
		GolfBall().m_initCurve = wobble / 10.0f;
		SendMsg(this, "Spin", M983(0x165), 0, (int)GolfBall().m_initCurve, (int)GolfBall().m_initSpin, 0);
	}
	// 983 (00694239): corte da tacada (pacote 0xe5 com o 5o argumento)
	short cutType = 0;
	if (Doc()->m_playingMode == 1 && OnlinePlay())
	{
		unsigned long param = Doc()->m_gameMode;
		if (GOLFDOC()->m_tutorialMode < 0xf)
			param = 6;
		if (GolfClub().PowerShotType() != 0 && GolfBall().m_initSpecial != 0)
		{
			if ((GolfBall().m_initSpecial & 0x70) != 0 && GOLFDOC983_SHOTTYPE == 4)
			{
				if (GolfClub().PowerShotType() == 2 || (GolfClub().PowerShotType() == 3 && GolfClub().GetType() == 0))
					cutType = 2;
				else
					cutType = 1;
			}
			else if ((GolfBall().m_initSpecial & 3) != 0 || (GolfBall().m_initSpecial & 0xc) != 0)
				goto cutin_done;
			else
				cutType = 4;
			unsigned char cur = GOLFDOC()->m_currentPlayer;
			if (HasCharCutin983(cutType, cur))
			{
				S5::UTIL::SendCutInPacket(Doc()->m_myInfo.info.dwUID, cutType, param,
					Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].charInfo.tid, true);
			}
			else
			{
				unsigned long item = PlayerCutinItem983(GOLFDOC()->m_currentPlayer);
				if (item != 0 && cutType != 4)
					S5::UTIL::SendCutInPacket(Doc()->m_myInfo.info.dwUID, cutType, param, item, false);
				else
					cutType = 0;
			}
		}
	}
cutin_done:
	if (!OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0);
		SendMsg(this, "GroundItemMan", M983(0x278), (int)&packet, 0, 0, 0);
	}
	// 983 (006943f9): fora da rede a trajetoria vai ao ator "ShotTrajectoryForMassGame" (0x312) com o bloco de 0x57
	if (!OnlinePlay())
	{
		sMassGameShot shot;
		MakeMassGameShot983(this, &shot);
		GetActor("ShotTrajectoryForMassGame") << MsgObject(NULL, 0x312, (int)&shot, 0, 0, 0, 0);
	}
	if (OnlinePlay() && Doc()->m_playingMode == 1)
	{
		sHitShotInfo983 info;
		info.spinX = GolfBall().m_initCurve;
		info.spinY = GolfBall().m_initSpin;
		info.ballFlags = GolfBall().m_initSpecial;
		info.field1 = GOLFDOC983_SHOTF1;
		info.field0 = GOLFDOC983_SHOTF0;
		info.type = (unsigned char)GOLFDOC983_SHOTTYPE;
		info.field3f = GOLFDOC983_SHOTF3F;
		// 983 (0085cad0): no tipo 0xb soma o valor +0x40 da barra
		info.pangyaArea = Doc()->m_golfGame.gameType == 0xb
			? CPowerGauge::Instance()->GetPangYaArea() + CPowerGauge::Instance()->GetValue40_983()
			: CPowerGauge::Instance()->GetPangYaArea();
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 11:
			SendMassGameShot983(this); // 983 (0067e140): pacote 0x12f
			break;
		}
		if (Doc()->m_golfGame.gameType == 10)
			info.time = (Doc()->m_approachStartTime - g_CurrentTime) + Doc()->m_golfGame.gameTimeLimit;
		else
			info.time = GOLFDOC()->m_shotTime;
		// 983: CQuadTree+0x21bc (o 645 lia +0x21c0, m_frame no nosso cabecalho; layout do CQuadTree nao conferido)
		info.mesh = GetPVS().m_frame; // 983 +0x21bc
		SendMsg(this, "Player", M983(0x110), (int)&info.player, 0, 0, 0);
		info.clubType = GolfClub().m_pClub->index;
		info.clubValue = *(float*)((char*)&GolfClub() + 0x64);
		*((unsigned char*)&GolfClub() + 0x68) = 0;
		info.extra = m_shotExtra983;
		if (IsLocalContent((localContentType_t)0x84 /*983: era 0x87; medido*/))
		{
			std::list<sItemInfo>& balls = Doc()->m_ballList;
			sItemInfo* found = NULL;
			for (std::list<sItemInfo>::iterator it = balls.begin(); it != balls.end(); ++it)
			{
				if (it->tid == Doc()->m_myInfo.userEquip.tidBall)
				{
					found = &*it;
					break;
				}
			}
			if (found == NULL)
				Doc()->m_myInfo.userEquip.tidBall = 0x14000000;
		}
		WSendPacket packet((enumClientPacket)0x12);
		AddSomeInfo(packet);
		packet.EncodeBuffer(&info, sizeof(sHitShotInfo983));
		SendMsg(this, "ClassicWind", 0x358, (int)&packet, 0, 0, 0); // 983: novo (CClassicWind, include/jp)
		SendMsg(this, "GroundItemMan", M983(0x278), (int)&packet, 0, 0, 0);
		packet.Send(TO_GAME);
		if (OnlinePlay())
			SendMsg(this, "Cutin", 0x30c, 0, 0, 0, 0); // 983: 0x30c (o 645 mandava M983(0x78))
		if (GOLFDOC()->m_inputList983.size() > 0)
		{
			WSendPacket packet2((enumClientPacket)0x42);
			packet2.Encode1((unsigned char)GOLFDOC()->m_inputList983.size());
			for (std::list<unsigned long>::iterator it = GOLFDOC()->m_inputList983.begin(); it != GOLFDOC()->m_inputList983.end(); ++it)
				packet2.Encode4(*it);
			packet2.Send(TO_GAME);
		}
		if (OnlinePlay())
		{
			switch (Doc()->m_golfGame.gameType)
			{
			case 4:
			case 5:
			case 6:
			case 9:
			case 10:
			case 11:
				SendMsg(this, "Cutin", 0x30c, 0x10, 0, 0, 0);
				if (cutType != 0)
					SetNetMsgWait(true, 0x3b43, "modeDriveMotion_MassGame_1", 20.0f);
				ChangeGameMode(0x40);
				break;
			default:
				if (cutType != 0)
					SetNetMsgWait(true, 0x3b4b, "HitShot_1", 20.0f);
				break;
			}
		}
		else
		{
			switch (Doc()->m_golfGame.gameType)
			{
			case 4:
			case 5:
			case 6:
			case 9:
			case 10:
			case 11:
				ChangeGameMode(0x40);
				break;
			}
		}
	}
	else
	{
		if (Doc()->m_golfGame.gameType == 10)
		{
			Doc()->m_rivalList[MyIndex()].approachTime =
				(Doc()->m_approachStartTime - g_CurrentTime) + Doc()->m_golfGame.gameTimeLimit;
		}
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) && Doc()->m_playingMode == 2)
		{
			if (Doc()->m_replayState != 7)
				Doc()->m_replayState = 3;
			GolfBall().m_initCurve = m_shotData.curve;
			GolfBall().m_initSpin = m_shotData.spin;
			SendMsg(this, "Spin", M983(0x165), 0, (int)GolfBall().m_initCurve, (int)GolfBall().m_initSpin, 0);
		}
		if (OnlinePlay() && GOLFDOC()->m_tutorialMode == 0xf && Doc()->m_playingMode != 2)
		{
			SendMsg(this, "Cutin", 0x30c, 0x10, 0, 0, 0);
			SetNetMsgWait(true, 0x3b6c, "modeDriveMotion_MassGame_2", 20.0f);
		}
		ChangeGameMode(0x40);
	}
	if (m_pChatView != NULL)
		m_pChatView->SetVisible(true);
	// 983 (00694929): com a trava de tela (+0x240) ligada, esvazia a fila (00681fc0, "Screen" 0x330) e, fora do tipo 0xb,
	// manda o pacote 0x156 vazio
	if (m_screenLockMode)
	{
		m_cmdQueueTime = 0.0f;
		while (!m_cmdQueue.empty())
			m_cmdQueue.pop_front();
		m_screenLockMode = 0;
		SendMsg(this, "Screen", 0x330, 0, 0, 0, 0);
		if (Doc()->m_golfGame.gameType != 0xb)
		{
			WSendPacket packet((enumClientPacket)0x156);
			packet.Send(TO_GAME);
		}
	}
}

void CGolfRule::ReactionForShot()
{
	SendMsg(this, "Caddie", M983(0x1d6), 0, 0, 0, 0);
	CGolfBall& ball = GolfBall();
	const CGolfBall::sExpected* start =
		ball.m_renderNum < 0 ? NULL : ball.m_expected;
	int last = GolfBall().m_renderNum;
	const CGolfBall::sExpected* end = GolfBall().PreData(last);
	WVector dist = end->pos - start->pos;
	if (!GolfBall().m_bOB && sqrtf(dist.z * dist.z + dist.x * dist.x) > 416.0f)
		SendMsg(this, "Caddie", M983(0x1d7), 0, 0, 0, 0);
	else
		CCameraWork::Instance()->SetCaddieChase(false);
}

void CGolfRule::ChangeTurn()
{
	SendMsg(this, "Spin", 0x271a, 0, 0, 0, 0);
	SendMsg(this, "Grid", M983(0x163), 0, 0, 0, 0);
	SendMsg(this, "Ball", M983(0x163), 0, 0, 0, 0);
	SendMsg(this, "Ball", M983(0x1eb), 0, 0, 0, 0);
	SendMsg(this, "Ball", M983(0x1ed), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1a0), 0, 0, 0, 0);
	SendMsg(this, "Item", M983(0x1a0), 0, 0, 0, 0);
	SendMsg(this, "Ball", M983(0x1a0), 0, 0, 0, 0);
	SendMsg(this, "Score", M983(0x1a0), 0, 0, 0, 0);
	SendMsg(this, "GuideLine", M983(0x1a0), 0, 0, 0, 0);
	SendMsg(this, "NPC", M983(0x182), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x13), 0, 0, 0, 0);
	// 983 (0067c730): caddie do m_userInfo do jogador da vez (o 645 lia 0x1dfc/0x1e00 + jogador * 0xb92), 0x359 ao "Caddie",
	// 0x308 ao "Mascot", 0x212 ao "ReactionObject" e 0x30b ao "Cutin" (zera o estado do corte a cada vez; C23); barra em +0xe0
	SendMsg(this, "Caddie", M983(0x1e3),
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid,
		Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tidPart, 0, 0);
	SendMsg(this, "Caddie", 0x359, 0, 0, 0, 0);
	SendMsg(this, "Mascot", 0x308, 0, 0, 0, 0);
	SendMsg(this, "ReactionObject", 0x212, 0, 0, 0, 0);
	SendMsg(this, "Cutin", 0x30b, 0, 0, 0, 0);
	GOLFDOC()->m_shotPhase = 0;
	GOLFDOC()->m_bar.Reset();
	GOLFDOC()->m_bar.gauge = GolfClub().GetType() == 3 ? BAR_START : BAR_MIN;
	GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.gauge;
	GolfBall().SetHoleIn(0);
	((CQuadTreeEx*)&GetPVS())->CalcGreenPlane();
	((CClubEx*)&GolfClub())->SetAccuracy();
	PlayBGM();
	GOLFDOC()->m_bReplaying = 0;
}

void CGolfRule::GetWinningPrize(int prize)
{
	AddPrize(prize);
	AddExtPrize();
}

void CGolfRule::AddPrize(int score)
{
	unsigned long tid;
	switch (score)
	{
	case -3:
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.albatross;
		break;
	case -2:
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.eagle;
		break;
	case -1:
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.birdie;
		break;
	case 0:
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.par;
		break;
	case 1:
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.bogey;
		break;
	default:
		tid = 0;
		break;
	}
	if (GOLFDOC()->GetHoleData().par + score == 1)
		tid = PLAYER(GOLFDOC()->m_currentPlayer)->prize.holeInOne;
	CItemManager& itemManager = Doc()->m_itemManager;
	if (itemManager.FindCommonItem(tid))
	{
		SendMsg(this, "Caddie", M983(0x1de), 0x23, 0, 0, 0);
		CFxSpray* pSpray = CFx::Instance()->OpenSpray("prize.spr");
		if (pSpray)
		{
			// 983 00683d9f: DAT_00e47e30 = GolfBall().m_pos inline (ctor 006b1030, guarda 00e73e34)
			pSpray->Pos() = GolfBall().m_pos;
		}
	}
	// 983: 7 dwords (0x56d..0x589)
	for (int offset = 0; offset < 0x1c; offset += 4)
		*(unsigned long*)((char*)&PLAYER(GOLFDOC()->m_currentPlayer)->prize +
			offset) = 0;
	// 983: jogador da vez com 0x40000006 em userInfo+0x300e -> buloon.seq
	if (*(int*)USERINFO983(GOLFDOC()->m_currentPlayer, 0x6a3c) == 0x40000006)
	{
		CFxSequence* seq = CFx::Instance()->OpenSequence("buloon.seq", true);
		if (seq)
			seq->Pos() = GolfBall().m_pos;
	}
	// 983: item de evento do CSharedDoc+0x11460 (spr por item, posicao do buraco)
	unsigned long evt = *(unsigned long*)(DOCPTR983(0x11460));
	if (Doc()->m_itemManager.FindCommonItem(evt))
	{
		const char* spr = "xmas_prize.spr";
		switch (evt)
		{
		case 0x1a000046: spr = "aeboat-03.spr"; break;
		case 0x1a000047: spr = "aeboat-04.spr"; break;
		case 0x1a0000c3: spr = "aeboat-02.spr"; break;
		case 0x1a0000c4:
		case 0x1a000104: spr = "aeboat-01.spr"; break;
		case 0x1a0000df: spr = "magic_stone_show.spr"; break;
		case 0x1a0000ed: spr = "coin_medal01.spr"; break;
		case 0x1a0000ee: spr = "coin_medal02.spr"; break;
		case 0x1a0000ef: spr = "coin_medal03.spr"; break;
		case 0x1a00010b: spr = "lightstone.spr"; break;
		case 0x1a000134: spr = "Event_Flower01.spr"; break;
		}
		float* hp = (float*)(*(char**)GDOCPTR983(GOLFDOC(), 0xd8) - 0x2c +
			*(unsigned char*)GDOCPTR983(GOLFDOC(), 0x17c) * 0x38);
		WVector hpos;
		hpos.x = hp[0];
		hpos.y = hp[1];
		hpos.z = hp[2];
		CFxSpray* s = CFx::Instance()->OpenSpray(spr);
		if (s)
			s->Pos() = hpos;
		CFxSequence* seq = CFx::Instance()->OpenSequence("thanxgiving.seq", true);
		if (seq)
			seq->Pos() = hpos;
		// 983 (00684034): so quem joga a vez (IsMyTurn(true)) e fora do replay (+0x62c) ve o nome do premio no chat
		if (IsMyTurn(true) && m_replayFlags == 0)
		{
			IFF_ITEM_COMMON* evtItem = Doc()->m_itemManager.FindCommonItem(evt);
			CChatMsg::Instance()->AddChatMsg(
				std::string(MakeStr(K2L_Compatibility(evtItem->Name))), 0xffff7878,
				false, false);
		}
	}
}

void CGolfRule::AddExtPrize()
{
	bool bEffect = true;
	PLAYER(GOLFDOC()->m_currentPlayer);
	PLAYER(GOLFDOC()->m_currentPlayer);
	PLAYER(GOLFDOC()->m_currentPlayer);
	int i = 0;
	int off = 0;
	if (PLAYER(GOLFDOC()->m_currentPlayer)->extPrizeNum > 0)
	{
		do
		{
			sPlayerData* player = PLAYER(GOLFDOC()->m_currentPlayer);
			if (!IsSilentPrizeType(EXTPRIZE(player, type)))
			{
				unsigned long tid =
					EXTPRIZE(PLAYER(GOLFDOC()->m_currentPlayer), typeId);
				IFF_ITEM_COMMON* item;
				std::map<unsigned long, sArtifactPrize983>::iterator itArt = s_artifactPrize983.find(tid); // 006815c0
				sArtifactPrize983 art = { NULL, NULL };
				if (itArt != s_artifactPrize983.end())
					art = itArt->second;
				if (tid != 0 &&
					((item = art.pPair) != NULL || (item = Doc()->m_itemManager.FindCommonItem(tid)) != NULL))
				{
					const char* fx;
					{
						fx = "thanxgiving.seq";
						if (tid == 0x1a000010)
						{
							sPlayerData* p = PLAYER(GOLFDOC()->m_currentPlayer);
							int level;
							if (EXTPRIZE(p, quantity) < 11)
								level = 10;
							else
								level = EXTPRIZE(p, quantity) > 100 ? 30 : 20;
							SendMsg(this, "Effect", M983(0x17d), level, 0, 0, 0);
							bEffect = false;
						}
						else if (art.pPair) // 983 006843ef: item par de um artefato
						{
							fx = "AF_Cluster.seq";
							bEffect = false;
						}
						else if (tid == 0x1a0003c2) // 983 00684210
						{
							fx = "spika_theme_research_reward.seq";
							bEffect = false;
						}
						else if (tid == 0x1a000254)
						{
							fx = "ebort_logo.seq";
							bEffect = false;
						}
						else
						{
							// 983 00684428 (find 00680d40): HoleCupDrop.iff (Doc+0x690) tid -> nome do efeito (+0x04 do registro)
							std::map<unsigned int, IFF983::sHoleCupDropItem>& drop = Doc()->m_itemManager.m_HoleCupDropItemMap983;
							std::map<unsigned int, IFF983::sHoleCupDropItem>::iterator itDrop = drop.find(tid);
							if (itDrop != drop.end())
							{
								fx = (const char*)(*itDrop).second.raw;
								bEffect = false;
							}
						}
					}
					const char* ext = strrchr(fx, '.');
					if (ext && strlen(ext + 1) > 2)
					{
						GolfBall();
						if (_stricmp(ext + 1, "spr") == 0)
						{
							CFxSpray* spray = CFx::Instance()->OpenSpray(fx);
							if (spray)
								spray->Pos() = GetHoleCupPos();
						}
						else
						{
							CFxSequence* seq =
								CFx::Instance()->OpenSequence(fx, true);
							if (seq)
								seq->Pos() = GetHoleCupPos();
						}
						if (bEffect)
						{
							CFxSpray* spray =
								CFx::Instance()->OpenSpray("xmas_prize.spr"); // 983: nome fixo (sem GetHoleDropEffectName)
							if (spray)
								spray->Pos() = GolfBall().m_pos;
						}
						SendMsg(this, "Caddie", M983(0x1de), 0x23, 0, 0, 0);
						if (IsMyTurn(true))
						{
							char msg[0x80] = { 0 };
							if (tid == 0x1a000010)
								sprintf(msg, "%d %s",
									EXTPRIZE(PLAYER(GOLFDOC()->m_currentPlayer),
										quantity) * 500, // 983: quantidade * 500
									item->Name);
							else if (art.pOwned) // 983 00684659: nome do artefato (d1811c)
								sprintf(msg, "%s", art.pOwned->Name);
							else
								sprintf(msg, "%s", item->Name);
							if (!art.pPair || art.pOwned) // 983 006846a3: par sem o artefato = sem chat
							{
								const char* fmt = art.pOwned
									? K2L_Compatibility("[%s]\xc0\xc7 \xb8\xb6\xb7\xc2\xc0\xcc \xc8\xb8\xba\xb9\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.")
									: K2L_Compatibility("\xc3\xe0! %s \xc8\xb9\xb5\xe6");
								std::string text(MakeStr(fmt, msg));
								CChatMsg::Instance()->AddChatMsg(text, 0xffff7878,
									false, false);
							}
						}
					}
				}
			}
next:
			i++;
			off += 0x10; // 983 0068473x: registro de 0x10 (sPrizeInfo)
		} while (i < PLAYER(GOLFDOC()->m_currentPlayer)->extPrizeNum);
	}
	PLAYER(GOLFDOC()->m_currentPlayer)->extPrizeNum = 0;
}

void CGolfRule::CancelShot()
{
	timeEndPeriod(1);
	CFx::Instance()->SetActive(true);
	if (OnlinePlay() && IsMyTurn(false))
	{
		sCancelShotData data;
		data.flag = 0;
		data.value = *(unsigned long*)&GOLFDOC()->m_bar.center;
		WSendPacket packet((enumClientPacket)0x14);
		packet.EncodeBuffer(&data, 5);
		packet.Send(TO_GAME);
	}
	m_bRemoteHitPending = false;
	GOLFDOC()->m_shotPhase = 0;
	GOLFDOC()->m_bar.gauge = GolfClub().GetType() == 3 ? BAR_START : BAR_MIN;
	GOLFDOC()->m_bar.gaugeStep = GOLFDOC()->m_bar.gauge;
	GOLFDOC()->m_bar.power = 140.0f;
	GOLFDOC()->m_bar.dir = 0;
	GOLFDOC()->m_bar.bFullPower = 0;
	CMouseCursor::Instance()->SetActive(true);
	if (CCapturedBg::Instance())
		CCapturedBg::Instance()->Reset();
	SendMsg(this, "Camera", M983(0x19a), 0, 0, 0, 0);
	GOLFDOC()->m_gameMode = 8;
	if (m_pChatView)
		m_pChatView->SetVisible(true);
	// B1268: 983 (00680975): com o power shot armado (taco +0x2c/+0x34), religa o indicador (0x313) e, com o auto shot
	// (+0x241), os botoes (0x32f) que o modo 0x10 apagou
	if (CLUB_XOR() != 0xffffffff)
	{
		SendMsg(this, "Screen", 0x313, 1, 0, 0, 0);
		if (m_bAutoShot)
			SendMsg(this, "Screen", 0x32f, 1, 0, 0, 0);
	}
}

void CGolfRule::SaveErrorFile()
{
	if (IsMyTurn(false))
	{
		mkdir("error");
		int i = 0;
		do
		{
			sprintf(m_saveFileName, "error/%s_%03d.rep",
				GetMyComputerName().c_str(), i++);
		} while (_access(m_saveFileName, 0) != -1);
		SaveShot(m_saveFileName);
	}
}

void CGolfRule::ReturnLobby(bool toRoomList)
{
	if (Doc()->m_golfGame.gameType != 10)
	{
		Doc()->m_bGameOver = true;
		if (CTHunter::Instance())
			CTHunter::Instance()->SetReturnLobby(true);
	}
	if (!OnlinePlay())
	{
		if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		{
			g_bQuit = true;
			return;
		}
		if (m_pRuleBase)
			CProjectG::Instance()->FromGolfToLobbySettings();
		if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
			Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
		{
			// 983 (0067cfe9): sem CTutorialTask - volta ao CLobbyTask e abre o widget do tutorial (0x88 so do 983)
			Doc()->m_gameMode = 0;
			CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
			CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88 /*so do 983*/,
				(int)"CTutorialWidget", (int)"TOPPAGE", 0, 0);
		}
		else if (Doc()->m_gameMode == 1)
		{
			CTaskManager::Instance()->ChangeTask("CFamilyTask", "", false);
			PostMsg(this, "Family", 0, (int)"FAMILY", 0, 0, 0);
		}
		else if (Doc()->m_gameMode == 5)
		{
			CTaskManager::Instance()->ChangeTask("CGhostTask", "",
				CTaskManager::Instance()->GetPreservedTask() != NULL);
			CTaskManager::Instance()->PostMsg(NULL, "Ghost", 0, (int)"GHOST", 0,
				0, 0);
		}
		return;
	}
	if (m_pRuleBase)
	{
		if (IsMassGame())
			Doc()->m_holeStatistics[0].dwShotTime =
				*(unsigned long*)((char*)PLAYER(0) + 0x4ec) / 1000;
		CProjectG::Instance()->FromGolfToLobbySettings();
	}
	CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);
	// 983 (0067cc70): sempre "TOPPAGE" e a janela da sala abre por cima pela msg 0x8b do Lobby (6 normal, 7 mass,
	// 9/10/0xb pelo realGameType, 4 = lista de salas); sem os layouts GAMEROOM/ROOMLIST do 645 (destroem o RFLobbyDesktop)
	PostMsg(this, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
	if (toRoomList)
		PostMsg(this, "Lobby", 0x8b, 4,
			(unsigned)(Doc()->m_roomInfo.realGameType - 0x14) < 2 ? 1 : 0, 0, 0);
	else
	{
		int roomMsg = 6;
		bool bExtMsg = false;
		if (IsMassGame())
		{
			if (Doc()->m_golfGame.gameType == 10)
				roomMsg = 7;
			else
			{
				unsigned char realType = Doc()->m_roomInfo.realGameType;
				if (realType == 0xc || realType == 0xe || realType == 0x13)
				{
					roomMsg = 0xb;
					bExtMsg = true;
				}
				else if ((unsigned)(realType - 0x14) < 2)
				{
					roomMsg = 10;
					bExtMsg = true;
				}
				else
					roomMsg = 9;
			}
		}
		PostMsg(this, "Lobby", 0x8b, roomMsg, 0, 0, 0);
		if (bExtMsg)
			PostMsg(this, "Lobby", M983(0x2d), 0, 0, 0, 0); // 983 0x2e
		PostMsg(this, "Lobby", M983(0x2f), 0, 0, 0, 0);
		PostMsg(this, "Lobby", M983(0xf), 0, 0, 0, 0);
		PostMsg(this, "Lobby", M983(0xe), 0, 0, 0, 0);
		if (Doc()->m_golfGame.gameType == 6 &&
			Doc()->m_holeStatistics[0].dwExp != (unsigned long)-1)
			PostMsg(this, "Lobby", M983(0x2d), 0, 0, 0, 0);
	}
	PostMsg(this, "Lobby", M983(0x1a), 0, 0, 0, 0);
	if (CSceneManager::Instance())
		SendMsg(this, "GolfBg", M983(0x142), 0, 0, 0, 0);
}

bool CGolfRule::ReturnToLobbyDlgResult(int, FrForm*)
{
	if (CLoginInfo::Instance()->IsWebLogin())
	{
		g_bQuit = true;
		return true;
	}
	// 983 (0067d060): so web login (sai) ou ReturnLobby(1); o m_teamPang = totalPang do 645 nao existe
	ReturnLobby(true);
	return true;
}

void CGolfRule::ToggleGameUI()
{
	SendMsg(this, "Screen", M983(0x190), m_bGameUI != false, 0, 0, 0);
	SendMsg(this, "Letter", M983(0x190), m_bGameUI != false, 0, 0, 0);
	SendMsg(this, "VideoRecorder", M983(0x190), m_bGameUI != false, 0, 0, 0);
	if (g_pFresh)
		g_pFresh->show = m_bGameUI;
	// JP 983 (00670f50): o mesmo valor vai ao +0x40 do ponteiro da interface (00e73e4c), ao +0x125 do cursor (m_bShow) e ao chat (+0x1cf6)
	if (CUiPointer983::Instance983())
		CUiPointer983::Instance983()->m_show40 = m_bGameUI;
	CMouseCursor::Instance()->Toggle(m_bGameUI);
	CChatMsg::Instance()->Toggle(m_bGameUI);
}

bool CGolfRule::CheckAllHoleOut()
{
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		if (PLAYER(i)->state == 0)
			return false;
	}
	return true;
}

bool CGolfRule::PauseDlgResult(int result, FrForm*)
{
	MOUSECURSOR()->m_bArea[CMouseCursor::AREA_AUXMENU_PAUSE] = true;
	m_pPauseDlg = NULL;
	if (OnlinePlay())
	{
		if (result)
		{
			WSendPacket packet((enumClientPacket)0x30);
			packet.Encode1(0);
			packet.Send(TO_GAME);
		}
	}
	else
		GOLFDOC()->m_bPause = false;
	return true;
}

bool CGolfRule::ContinueDlgResult(int result, FrForm*)
{
	*((unsigned char*)GRPTR983(0x41e)) = 0;
	if (result == 1)
	{
		PLAYER(0)->state = 2;
		PLAYER(0)->gauge = 0.0f;
		PLAYER(0)->totalStroke = 0;
		PLAYER(0)->shotTime = 0;
		PLAYER(0)->rank = 1;
		PLAYER(0)->gauge = 0.0f;
		PLAYER(0)->score = 0;
		PLAYER(0)->totalPang = 0;
		PLAYER(0)->bonusPang = 0;
		PLAYER(0)->holeNum = 0;
		for (int i = 0; i < 18; i++)
		{
			PLAYER(0)->stroke[i] = 0;
			PLAYER(0)->putt[i] = 0;
			PLAYER(0)->pang[i] = 0;
		}
		if (Doc()->m_gameMode == 2)
		{
			memcpy(DOCPTR983(0x3b46), DOCPTR983(0xa36), 0x28);
			sPlayerData* player = PLAYER(0);
			player->itemNum = Doc()->CollapseItemSlot(0);
		}
		GOLFDOC()->m_currentHole = Doc()->m_holeOrder[0];
		SendMsg(this, "GolfBg", M983(0x140), 0, 0, 0, 0);
		return true;
	}
	if (result == 2)
		ReturnLobby(false);
	return true;
}

bool CGolfRule::CancelDropDlgResult(int result, FrForm*)
{
	m_pCancelDropDlg = NULL;
	if (result == 1)
	{
		GOLFDOC()->m_bDropRequest = 0;
		ChangeGameMode(8);
	}
	return true;
}

bool CGolfRule::GoStopDlgResult(int result, FrForm*)
{
	m_pGoStopDlg = NULL;
	if (result)
	{
		WSendPacket packet((enumClientPacket)0x36);
		packet.Encode1(result == 1);
		packet.Send(TO_GAME);
		GOLFDOC()->m_bStopDlg = false;
		m_bTimeOutSent = true;
		SetNetMsgWait(true, 0x40b2, "GoStopDlgResult", 20.0f); // 983 (00676b80): 0x40b2 (0x3deb do 645)
	}
	return true;
}

bool CGolfRule::OnEmoticonResult(int result, FrForm* form)
{
	if (!CChatMsg::Instance()->IsOpened())
		g_pFresh->GetManager()->SetExclusiveKey(false);
	if (m_pEmoticonDlg && result == 1)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, form);
		if (pDlg)
		{
			const char* icon = pDlg->GetSelectedIcon();
			if (icon)
			{
				if (!m_pChatEdit || !m_pChatEdit->IsVisible())
				{
					Doc()->m_chatManager.SendChatMessage(icon, NULL, false);
					if (!OnlinePlay())
					{
						std::string text(USERINFO983(GOLFDOC()->m_currentPlayer, 0x38c8));
						// 983 (00685d90): no modo 5 entra a string do doc+0x7b8 antes do " : "
						if (*((unsigned char*)DOCPTR983(0x10594)) == 5)
							text += DOCPTR983(0x7b8);
						text += " : \b";
						text += icon;
						CChatMsg::Instance()->AddChatMsg(text, 0xffffcc00,
							false, false);
					}
				}
				else
				{
					const char* front = m_pChatEdit->GetEditText_Front();
					const char* comp = m_pChatEdit->GetEditText_Comp();
					const char* end = m_pChatEdit->GetEditText_End();
					if (CChatMsg::Instance()->GetMaskedFont()->GetTextWidthOld983(
							g_view,
							MakeStr("%s%s%s%s", front, comp, icon, end)) <
						m_pChatEdit->GetWidthLimit())
						m_pChatEdit->SetLine(1,
							MakeStr("%s%s%s%s", front, comp, icon, end), 0,
							false, 0);
					m_pChatEdit->SetKeyFocus(true);
				}
			}
		}
	}
	m_pEmoticonDlg = NULL;
	return true;
}

bool CGolfRule::OnReportResult(int result, FrForm*)
{
	if (result == 1)
	{
		GOLFDOC()->m_bReport = 1;
		WSendPacket packet((enumClientPacket)0x3a);
		packet.Send(TO_GAME);
	}
	m_pReportDlg = NULL;
	return true;
}

bool CGolfRule::OnReturnLobbyResult(int, FrForm*)
{
	ReturnLobby(false);
	return true;
}

bool CGolfRule::OnForgetDlgResult(int result, FrForm*)
{
	if (result == 1)
	{
		// 983 (00685fe0): com a carta do periodo (CCardManager 0080a560) o cancelamento vai direto ao servidor com o
		// item 0x1800000e (pacote 0x17); sem ela, pelo ator "Item" como no 645
		if (CCardManager::Instance()->GetPeriodFlag20_983())
		{
			if (OnlinePlay())
			{
				WSendPacket packet((enumClientPacket)0x17);
				packet.Encode4(0x1800000e);
				packet.Send(TO_GAME);
			}
		}
		else
			SendMsg(this, "Item", M983(0x19e), m_cancelItemSlot, 0, 0, 0);
		SetNetMsgWait(true, 0x4112, "PM_NET_CANCEL_SHOT", 20.0f);
	}
	else if (result == 2)
	{
		SendShotResult();
		SendMsg(this, "Item", M983(0x195), 0, 0, 0, 0);
		SendMsg(this, "Screen", M983(0x1af), 0, 0, 0, 0);
	}
	m_bCancelShotAsk = false;
	m_forgetDlgTime = 0;
	m_pForgetDlg = NULL;
	return true;
}

bool CGolfRule::OnApproachEndDlgResult(int, FrForm*)
{
	if (m_pApproachEndDlg)
	{
		if (!Doc()->m_bSentHoleStat)
			Doc()->m_bSentHoleStat = true;
		if (!GOLFDOC()->m_bNetGameEnd)
		{
			SendMsg(this, "Ball", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
			SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
			SendMsg(this, "Effect", M983(0x177), 0, 0, 0, 0);
			SendMsg(this, "Trajectory", M983(0x18c), 0, 0, 0, 0);
			SendMsg(this, "Trajectory", M983(0x160), 0, 0, 0, 0);
			CFx::Instance()->ClearAll(true);
			ChangeGameMode(0x2000);
		}
		if (CTHunter::Instance())
			CTHunter::Instance()->SetReturnLobby(true);
		Doc()->m_bGameOver = true;
		m_pApproachEndDlg = NULL;
	}
	return true;
}

bool CGolfRule::OnApproachDlgResult(int, FrForm*)
{
	if (m_pApproachResultDlg)
	{
		if (m_pApproachResultDlg->GetApproachEnd())
		{
			if (m_pApproachEndDlg == NULL)
			{
				m_pApproachEndDlg = CreateForm<FrApproachEndDlg>(
					g_pFresh->GetManager(), this, "approach_end", NULL);
				m_pApproachEndDlg->EnableDrag(false);
				WPoint pos;
				float width = m_pApproachEndDlg->GetRect().w;
				pos.x = g_view->GetWidth() * 0.5f - width * 0.5f;
				pos.y = 35.0f;
				m_pApproachEndDlg->Open(
					(FRESH_PFN_RESULT)&CGolfRule::OnApproachEndDlgResult, pos,
					0);
				m_pApproachEndDlg->SetTimeLimit(15.0f);
				m_pApproachEndDlg->SetCaption(
					K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1 \xc3\xd6\xc1\xbe\xb0\xe1\xb0\xfa"));
			}
		}
		else
			ChangeGameMode(0x800000);
		m_pApproachResultDlg = NULL;
	}
	return true;
}

// B755: o CheckUserBGM do 983 (0068b4f0) tem casos novos (0x12, 0x14, 0x15, 0x40): versao em C
void CGolfRule::CheckUserBGM(char const* name)
{
	m_bgmList.clear();
	switch (GetCurMap())
	{
	case 4:
	case 5:
	{
		std::string s1("shiny.mp3");
		m_bgmList.push_back(s1);
	}
		{
			std::string s2("bunny.mp3");
			m_bgmList.push_back(s2);
		}
		break;
	case 7:
	{
		std::string s3("navy_blue.mp3");
		m_bgmList.push_back(s3);
	}
		{
			std::string s4("rising_sun.mp3");
			m_bgmList.push_back(s4);
		}
		break;
	case 8:
	{
		std::string s5("happy flight.mp3");
		m_bgmList.push_back(s5);
	}
		{
			std::string s6("crystal waver.mp3");
			m_bgmList.push_back(s6);
		}
		break;
	case 9:
	{
		std::string s7("snowscape.mp3");
		m_bgmList.push_back(s7);
	}
		{
			std::string s8("winter_ride.mp3");
			m_bgmList.push_back(s8);
		}
		break;
	case 10:
	{
		std::string s9("somewhere.mp3");
		m_bgmList.push_back(s9);
	}
		{
			std::string s10("nowhere.mp3");
			m_bgmList.push_back(s10);
		}
		break;
	case 13:
	{
		std::string s11("volcano.mp3");
		m_bgmList.push_back(s11);
	}
		{
			std::string s12("vermilion.mp3");
			m_bgmList.push_back(s12);
		}
		break;
	case 14:
	{
		std::string s13("crystal lake.mp3");
		m_bgmList.push_back(s13);
	}
		{
			std::string s14("fade into white.mp3");
			m_bgmList.push_back(s14);
		}
		break;
	case 15:
	{
		std::string s15("the mystery of the lost seaway.mp3");
		m_bgmList.push_back(s15);
	}
		{
			std::string s16("voyage the sky.mp3");
			m_bgmList.push_back(s16);
		}
		break;
	case 16:
	{
		std::string s17("eastern_valley.mp3");
		m_bgmList.push_back(s17);
	}
		{
			std::string s18("river.mp3");
			m_bgmList.push_back(s18);
		}
		break;
	case 17:
	{
		std::string s19("volcano.mp3");
		m_bgmList.push_back(s19);
	}
		{
			std::string s20("river.mp3");
			m_bgmList.push_back(s20);
		}
		break;
	case 18:
	{
		std::string t18_0("cyansunset.mp3");
		m_bgmList.push_back(t18_0);
	}
		{
		std::string t18_1("orbitofdarkness.mp3");
		m_bgmList.push_back(t18_1);
	}
		break;
	case 19:
	{
		std::string s21("SecretWish.mp3");
		m_bgmList.push_back(s21);
	}
		{
			std::string s22("ADayInTheWizCity.mp3");
			m_bgmList.push_back(s22);
		}
		break;
	case 20:
	{
		std::string t20_0("beautiful_ruins.mp3");
		m_bgmList.push_back(t20_0);
	}
		{
		std::string t20_1("skyrider.mp3");
		m_bgmList.push_back(t20_1);
	}
		break;
	case 21:
	{
		std::string t21_0("dear memory.mp3");
		m_bgmList.push_back(t21_0);
	}
		{
		std::string t21_1("oracle.mp3");
		m_bgmList.push_back(t21_1);
	}
		break;
	case 64:
	{
		std::string t64_0("cyansunset.mp3");
		m_bgmList.push_back(t64_0);
	}
		{
		std::string t64_1("orbitofdarkness.mp3");
		m_bgmList.push_back(t64_1);
	}
		{
		std::string t64_2("GrandSkyscraper.mp3");
		m_bgmList.push_back(t64_2);
	}
		break;
	default:
	{
		std::string s23("samba3.mp3");
		m_bgmList.push_back(s23);
	}
		{
			std::string s24("daydream.mp3");
			m_bgmList.push_back(s24);
		}
		{
			std::string s25("frog.mp3");
			m_bgmList.push_back(s25);
		}
		{
			std::string s26("spring.mp3");
			m_bgmList.push_back(s26);
		}
		break;
	}
}

bool CGolfRule::CanCancelShot()
{
	if (CCardManager::Instance()->GetPeriodFlag20_983())
		return true; // 983 00676ce0: com o periodo ativo retorna true sem escolher o slot
	for (int i = 0; i < 10; i++)
	{
		if (Doc()->m_userInfo[0].userEquip.tidItemSlot[i] == 0x1800000e)
		{
			m_cancelItemSlot = i;
			return true;
		}
	}
	return false;
}

// JP 983 (006847a0): monta o resultado da tacada (registro de 0x26). Metodo proprio no 983; aqui fica estatico ate a A
// declarar em golfrule.h. Deslocamentos do 983 conferidos com mede.py (oid 0x4b4, bunker 0x4da, totalPang 0x4f8,
// bonusPang 0x508, shotFlag 0x4e0, windAbility983; m_replayFlags 0x62c; m_inputList983 0x8c).
static void MakeShotResult983(CGolfRule* rule, sShotResult* res)
{
	CGolfBall& ball = GolfBall();
	res->oid = PLAYER(GOLFDOC()->m_currentPlayer)->oid;
	for (int i = 0; i < 3; i++)
		*(float*)&res->pos[i] = (&PLAYER(GOLFDOC()->m_currentPlayer)->pos.x)[i];
	res->bunker = PLAYER(GOLFDOC()->m_currentPlayer)->bunker;
	if (ball.m_bOB)
		res->state = 3;
	else if (ball.m_holeIn == 1)
		res->state = 4;
	else
		res->state = ball.m_standGround != 0 ? 2 : 5;
	res->replayFlag = (unsigned char)rule->m_replayFlags;
	res->totalPang = (int)PLAYER(GOLFDOC()->m_currentPlayer)->totalPang;
	res->bonusPang = (int)PLAYER(GOLFDOC()->m_currentPlayer)->bonusPang;
	res->itemTick = PLAYER(GOLFDOC()->m_currentPlayer)->shotFlag;
	res->renderNum = (unsigned short)ball.m_renderNum;
	res->special = 0;
	if (ball.m_special & 0x10)
		res->special = 2;
	else if (ball.m_special & 0x40)
		res->special = 4;
	else if (ball.m_special & 0x20)
		res->special = 8;
	if (ball.m_special & 1)
		res->special |= 0x10;
	else if (ball.m_special & 2)
		res->special |= 0x20;
	if (ball.m_special & 4)
		res->special |= 0x40;
	else if (ball.m_special & 8)
		res->special |= 0x80;
	// 983: bits novos -- sem itens de tacada, tipo de tacada forte e tipo do taco
	if (GOLFDOC()->m_inputList983.size() == 0)
		res->special |= 0x800;
	int powerShot = GolfClub().PowerShotType();
	if (powerShot == 1)
		res->special |= 0x1000;
	else if (powerShot > 1 && powerShot <= 3)
		res->special |= 0x2000;
	switch (GolfClub().GetType())
	{
	case 0:
		res->special |= 0x100000;
		break;
	case 1:
		res->special |= 0x200000;
		break;
	case 2:
		res->special |= 0x400000;
		break;
	case 3:
		res->special |= 0x800000;
		break;
	}
	if (PLAYER(GOLFDOC()->m_currentPlayer)->windAbility983 != 0)
		res->special |= 0x4000000;
	// 983 (00684a68): no Grand Prix (realGameType 0x14/0x15) o artefato da sala (m_roomInfo.artifactTid983) passa por
	// CGrandPrixArtifactRule::Check (00546a60, singleton em 0xe75194; include/jp da B65) com o inicio rule->m_turnStartTime;
	// se valer: gpArtifact = 1, +1 tacada no buraco do jogador da vez e texto 0x754f no bate-papo.
	if ((unsigned int)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
	{
		bool n = _grandprix::_private::CGrandPrixArtifactRule::Instance()->Check(Doc()->m_roomInfo.artifactTid983,
			(const unsigned char*)res, rule->m_turnStartTime);
		if (n)
		{
			res->gpArtifact = 1;
			PLAYER(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] += n;
			CChatMsg::Instance()->AddChatMsg(std::string(GetGameString(0x754f)), 0xffff7878, true, false);
		}
	}
}

// JP 983 (00695270): fim do tempo da Approach (chamada pela CGolfRuleNewApproach [10])
void CGolfRule::ApproachTimeOut983()
{
	SendMsg(this, "Spot", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Grid", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Spin", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Player", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Caddie", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Mascot", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Bubble", M983(0x164), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1a4), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1a5), 0, 0, 0, 0);
	SendMsg(this, "Screen", M983(0x1a8), 0, 0, 0, 0);
	ChangeGameMode(0x800);
}

// JP 983 (00684ba0): um bloco de 0x26 cifrado com a chave da sala (m_roomInfo.RoomKey, +0x10939); a chave nao vai mais
// no pacote (o 645 mandava 0x25 + a chave crua de 0x10 bytes).
void CGolfRule::SendShotResult()
{
	if (OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0x1b);
		sShotResult res;
		MakeShotResult983(this, &res);
		if (IsLocalContent((localContentType_t)0x2e /*983: era 0x2d; medido em 00684c1f*/))
			CEncryption::Instance()->Encrypt<sShotResult>(&res, Doc()->m_roomInfo.RoomKey);
		packet.EncodeBuffer(&res, sizeof(sShotResult));
		packet.Send(TO_GAME);
	}
}

void CGolfRule::SendReportError(unsigned char type)
{
	WSendPacket packet(0x33);
	if (m_pNetMsgWait == NULL)
		m_pNetMsgWait = "UnknownLock";
	if (m_pNetMsgRecv == NULL)
		m_pNetMsgRecv = "UnknownUnlock";
	std::string text;
	char buf[128];
	text = m_pNetMsgWait;
	sprintf(buf, "(%d) / ", m_netMsgSendTick);
	text += buf;
	text += m_pNetMsgRecv;
	sprintf(buf, "(%d)", m_netMsgRecvTick);
	text += buf;
	sprintf(buf, "/ ET(%d)", GetTickCount() - m_netMsgSendTick);
	text += buf;
	// 983 (00686110): depois do ET vem " / TO(%f)" com o tempo de espera (+0x434)
	sprintf(buf, " / TO(%f)", (double)m_netMsgWaitTime);
	text += buf;
	if (type == 3)
	{
		sprintf(buf, "[Pang Hacker]");
		text += buf;
	}
	packet.Encode1(type);
	packet.EncodeStr(text);
	packet.Send(TO_GAME);
}

void CGolfRule::ResetUiPos_Board()
{
	WRect rect = m_pOnelineBtn->GetRect();
	rect.x = g_view->GetWidth() - BAR_START;
	m_pOnelineBtn->SetRect(rect);
	rect = m_pOnelineView->GetRect();
	rect.x = g_view->GetWidth() - 317.0f;
	m_pOnelineView->SetRect(rect);
	COneLineBoard::Instance()->SetRect(m_pOnelineView->GetRect());
}

// visao local: o 983 le o nome da musica atual do CSoundManager (+0x24, m_bgmName protegido) sem metodo no nosso cabecalho
struct CSoundManagerBgm983 : public CSoundManager
{
	static const char* BgmName(CSoundManager* p) { return static_cast<CSoundManagerBgm983*>(p)->m_bgmName; }
};

void CGolfRule::PlayBGM()
{
	// 983 (00676d20): no shortgame de 1 buraco (CGolfDoc+0x195) fora do green toca "shortgame 1hole nogreen BGM.mp3" direto no
	// g_audio (se ainda nao for a atual); no mapa 0x40 (sem o bit 0x80 do mapType) sorteia e as vezes nao troca a musica
	CGolfDoc* doc = GOLFDOC();
	if (doc->m_b195_983 && doc->m_currentHole == Doc()->m_holeOrder[0] && GolfBall().m_groundType != 2)
	{
		if (stricmp(CSoundManagerBgm983::BgmName(g_audio), "shortgame 1hole nogreen BGM.mp3") != 0)
			g_audio->PlayBGM("shortgame 1hole nogreen BGM.mp3", false, true);
		return;
	}
	const char* name;
	if (GolfBall().m_groundType == 2)
		name = g_bgmPutting[rand() % 2];
	else
	{
		unsigned char map = Doc()->m_roomInfo.mapType;
		int course = (map > 0x7f && map != 0xfd) ? map - 0x80 : map;
		if (course == 0x40 && rand() % m_bgmList.size() + 1 == m_bgmList.size())
			return;
		name = m_bgmList[m_bgmIndex].c_str();
	}
	SetConstantBGM(name, false, true);
}

void CGolfRule::OnMessengerBtnInit(int param)
{
	m_pMessengerBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMessengerBtn)
	{
		if (!Doc()->m_bMessengerInit)
			m_pMessengerBtn->EnableBlink(false);
		m_pMessengerBtn->SetVisible(false);
		m_pMessengerBtn->EnableHover(true, 0.0f);
		WPoint pos;
		pos.x = g_view->GetWidth() - 81.0f;
		pos.y = m_pMessengerBtn->GetRect().y;
		m_pMessengerBtn->MoveWindow(pos);
		CMessengerInfo::Instance()->m_alarmRect = m_pMessengerBtn->GetRect();
	}
}

void CGolfRule::OnMessengerBtnUp()
{
	CMessengerInfo::Instance()->Open(true);
	OnBarHoverOff(0);
}

void CGolfRule::OnMessengerBubbleEditInit(int param)
{
	m_pMessengerBubbleEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMessengerBubbleEdit)
	{
		m_pMessengerBubbleEdit->SetVisible(false);
		WPoint pos;
		pos.x = g_view->GetWidth() - 167.0f;
		pos.y = m_pMessengerBubbleEdit->GetRect().y;
		m_pMessengerBubbleEdit->MoveWindow(pos);
	}
}

void CGolfRule::OnMessenger_AlarmBtnInit(int param)
{
	m_pMessengerAlarmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMessengerAlarmBtn)
	{
		m_pMessengerAlarmBtn->SetTopmost(true);
		if (m_pMessengerAlarmBtn && !Doc()->m_bMessengerAlarm)
		{
			m_pMessengerAlarmBtn->SetVisible(false);
			m_pMessengerAlarmBtn->SetTopmost(false);
		}
		WPoint pos;
		pos.x = g_view->GetWidth() - 110.0f;
		pos.y = m_pMessengerAlarmBtn->GetRect().y;
		m_pMessengerAlarmBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnMessenger_AlarmBtnUp()
{
	CMessengerInfo::Instance()->AlarmBtnUp(0xffffffff);
}

void CGolfRule::OnMessenger_GiftAlarmBtnInit(int param)
{
	m_pMessengerGiftAlarmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMessengerGiftAlarmBtn)
	{
		m_pMessengerGiftAlarmBtn->SetVisible(false);
		m_pMessengerGiftAlarmBtn->SetTopmost(false);
		WPoint pos;
		pos.x = g_view->GetWidth() - 75.0f;
		pos.y = m_pMessengerGiftAlarmBtn->GetRect().y;
		m_pMessengerGiftAlarmBtn->MoveWindow(pos);
	}
}

void CGolfRule::OnMessenger_GiftAlarmBtnUp()
{
	m_pMessengerGiftAlarmBtn->SetVisible(false);
	m_pMessengerGiftAlarmBtn->SetTopmost(false);
	FrGiftAlarmDlg* pDlg = CreateForm<FrGiftAlarmDlg>(g_pFresh->GetManager(),
		this, "gift_alarm", NULL);
	pDlg->Open((FRESH_PFN_RESULT)&CGolfRule::OnNotifyDlgResult, 0x11);
	pDlg->SetTopmost(true);
	pDlg->FindNextTopFocus(false);
	pDlg->SetIconRect(m_pMessengerGiftAlarmBtn->GetRect());
}

// JP 983 (00671190): FrButton em +0x4f8, ancorado a direita como no 645 (sem o teste de S4_REPLAY_SYSTEM).
void CGolfRule::OnReplayStatusInit(int param)
{
	m_pReplayBox = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pReplayBox)
	{
		WRect rect = m_pReplayBox->GetRect();
		rect.x = g_view->GetWidth() - (800.0f - rect.x);
		m_pReplayBox->SetRect(rect);
	}
}

// JP 983 (006707c0/00670830): as setas da revisao da tacada comecam escondidas (sem isso o ">" ficava solto na partida).
void CGolfRule::OnShotReviewPreInit(int param)
{
	m_pShotReviewPreBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pShotReviewPreBtn->SetVisible(false);
}

void CGolfRule::OnShotReviewNextInit(int param)
{
	m_pShotReviewNextBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pShotReviewNextBtn->SetVisible(false);
}

// JP 983 (0066e430/0066e450): camera da revisao, 0 = anterior, 1 = proxima.
void CGolfRule::OnShotReviewPreBtnUp()
{
	SendMsg(this, "ShotReview", 0x2fd, (int)"SET_REVIEW_CAMERA", 0, 0, 0);
}

void CGolfRule::OnShotReviewNextBtnUp()
{
	SendMsg(this, "ShotReview", 0x2fd, (int)"SET_REVIEW_CAMERA", 1, 0, 0);
}

// JP 983 (00673530): FINISH do layout MAIN. No tipo de jogo 0xb desliga caddiebook/unplayable/pause (+0x504..+0x50c).
void CGolfRule::OnMainFinish()
{
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		m_pCaddieBookBtn->Enable(false);
		m_pUnplayableBtn->Enable(false);
		m_pPauseBtn->Enable(false);
	}
	if (SHAREDDOC_UCHAR(0x10d80) != 0)
		HandleMsg(MsgObject(NULL, M983(0xaa), 1, 0, 0, 0, 0));
}

// JP 983 (00672e10): aviso de fim do servico premium, visivel e ativo so com CSharedDoc+0x1160a.
// O OWNERDRAW (00678620) esta logo abaixo.
void CGolfRule::OnPremiumServiceEndNoticeInit(int param)
{
	m_pPremiumNotice = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (!m_pPremiumNotice)
		return;
	m_premiumNoticeAlpha = 1.0f;
	m_premiumNoticeFadeRate = 0.2f;
	bool premium = Doc()->m_bPremiumEndNotice983 != 0;
	m_pPremiumNotice->SetVisible(premium);
	m_pPremiumNotice->Enable(premium);
	static const char* const names[4] = { "Premium service_close_00", "Premium service_close_01",
		"Premium service_close_02", "Premium service_base" };
	for (int i = 0; i < 4; ++i)
		m_pPremiumBmp[i] = g_pFresh->GetBitmap(names[i]);
}

// JP 983 (00678620): desenha o aviso (faixa base + 3 bitmaps lado a lado, centrados, escala 0.5) com alpha +0x594 e o texto do doc (+0x1160c)
// duas vezes (estilo 2 e 0); com alpha <= 0 desliga o aviso (doc+0x1160a, janela e +0x59c).
void CGolfRule::OnPremiumServiceEndNoticeOwnerDraw(int)
{
	if (!m_pPremiumNotice || !Doc()->m_bPremiumEndNotice983)
		return;
	const Bitmap* b0 = m_pPremiumBmp[0];
	const Bitmap* b1 = m_pPremiumBmp[1];
	const Bitmap* b2 = m_pPremiumBmp[2];
	const Bitmap* base = m_pPremiumBmp[3];
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (b0 && b1 && b2 && base)
	{
		unsigned long color = ((int)(m_premiumNoticeAlpha * 255.0) << 24) | 0xFFFFFF;
		float w = g_view->GetWidth();
		float h = g_view->GetHeight();
		float w0 = b0->Width(), w1 = b1->Width(), w2 = b2->Width();
		float h0 = b0->Height();
		float x = w * 0.5 - (w2 + w1 + w0) * 0.5;
		float y = h * 0.5 - h0 * 0.5;
		pGDI->DrawTexture(base, WRect(0.0f, y - 20.0, w, h0 + 40.0), color, 0);
		pGDI->DrawTexture(b0, WRect(x, y, w0, h0), color, 0);
		pGDI->DrawTexture(b1, WRect(w0 + x, y, w1, b1->Height()), color, 0);
		pGDI->DrawTexture(b2, WRect(w1 + w0 + x, y, w2, b2->Height()), color, 0);
		const char* text = Doc()->m_premiumEndNoticeText983.c_str();
		pGDI->SetTextStyle(2);
		pGDI->SetTextColor(FrALPHA(0xFFFFFFFF, m_premiumNoticeAlpha), 0xFFFFFFFF);
		pGDI->Print(WPoint(w * 0.5, h0 * 1.1 + y), 1, text);
		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(FrALPHA(0xFFFFFFFF, m_premiumNoticeAlpha), 0xFFFFFFFF);
		pGDI->Print(WPoint(w * 0.5, h0 * 1.1 + y), 1, text);
	}
	if (m_premiumNoticeAlpha <= 0.0f)
	{
		Doc()->m_bPremiumEndNotice983 = 0;
		m_pPremiumNotice->SetVisible(false);
		m_pPremiumNotice->Enable(false);
		m_b983_59c = false;
	}
}

// JP 983 (00677250): buraco tipo 4 guarda o estado do botao em CSharedDoc+0x11608; senao avisa o Gateway (0x7535, sub 0x13).
void CGolfRule::OnMoveTeeBtnUp()
{
	if (*((unsigned char*)DOCPTR983(0x1094f)) == 4)
		Doc()->m_bFixHoleMoveTee983 = m_pMoveTeeBtn->GetStatus() == FrButton::PRESSED;
	else
		SendMsg(this, "Gateway", 0x7535, 0x13, 0, 0, 0);
}

// JP 983 (006770d0): "MAIN.move_tee", FrButton em +0x51c (o A175.1 o tinha posto no replay_status por engano).
// gameType 0xb: realGameType 0xb..0xd esconde, 0xe posiciona; senao, tipo de buraco (+0x1094f) == 4: imagens environment_btn_* + posicao.
void CGolfRule::OnMoveTeeInit(int param)
{
	FrButton* btn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pMoveTeeBtn = btn;
	if (!btn)
		return;
	unsigned char* doc = (unsigned char*)Doc();
	bool place = false;
	if (doc[DOCOFS983(0x10594)] == 0xb)
	{
		unsigned char rt = doc[DOCOFS983(0x109b8)];
		if (rt > 10 && rt < 0xe)
		{
			btn->SetVisible(false);
			return;
		}
		if (rt == 0xe)
			place = true;
	}
	else
	{
		if (doc[DOCOFS983(0x1094f)] == 4)
		{
			btn->SetButtonImg("environment_btn_Normal", FrButton::NORMAL);
			btn->SetButtonImg("environment_btn_Hover", FrButton::OVER);
			btn->SetButtonImg("environment_btn_on_Hover", FrButton::PRESSED);
			place = true;
		}
		else
			btn->SetVisible(false);
	}
	if (place)
	{
		WPoint pos((float)(g_view->GetWidth() * 0.5 + 90.0), g_view->GetHeight() - btn->GetRect().h);
		btn->MoveWindow(pos);
		btn->EnableHover(true, 0.0f);
	}
}

// JP 983 (00677290): chamado pelo OnPreLoadInit (no 645, pelo OnReload). Sem as regras Match (3) e Approach (9); hole-in-one (0xb)
// novo; cria tambem a sub-regra (+0x63c) pelo m_playingMode. Corpo do 645 em out/F3.5/s101/SetCurrentGolfRule_645.txt.
void CGolfRule::SetCurrentGolfRule()
{
	if (m_pRuleBase)
	{
		delete m_pRuleBase;
		m_pRuleBase = NULL;
	}
	switch (Doc()->m_golfGame.gameType)
	{
	case 0:
		m_pRuleBase = new CGolfRuleStroke(this);
		break;
	case 1:
		m_pRuleBase = new CGolfRuleTeam(this);
		break;
	case 7:
		m_pRuleBase = new CGolfRuleSkins(this);
		break;
	case 10:
		m_pRuleBase = new CGolfRuleNewApproach(this);
		break;
	case 6:
		m_pRuleBase = new CGolfRuleGuildMatch(this);
		break;
	case 4:
	case 5:
		m_pRuleBase = new CGolfRule30Battle(this);
		break;
	case 0xb:
		m_pRuleBase = (CGolfRuleBase*)(void*)new CGolfRuleHoleInOne(this);
		break;
	case GAME_TYPE_TUTORIAL_BASIC:
	case GAME_TYPE_TUTORIAL_ADV:
		m_pRuleBase = new CGolfRuleTutorial(this);
		if (Doc()->m_tutorialStartMode == 0xe)
			GOLFDOC()->m_currentHole = 2;
		else
			GOLFDOC()->m_currentHole = 1;
		break;
	default:
		m_pRuleBase = NULL;
		break;
	}
	if (Doc()->m_playingMode == 1)
		m_pSubRule983 = new CSubGolfRuleGame(this);
	else if (Doc()->m_playingMode == 2)
		m_pSubRule983 = new CSubGolfRuleReplayTape(this);
	else
		m_pSubRule983 = NULL;
}

bool CGolfRule::IsMaxWithHGParts(unsigned char index)
{
	static unsigned long s_hgPartTids[4] = { 0x08104011, 0x0810a00c, 0x08100008,
		0x08112005 };

	CPartTidList* parts = &GOLFDOC()->GetPartTidList(index);
	if (!parts)
		return false;
	bool bFind = false;
	for (unsigned int i = 0; i < 4; i++)
	{
		if (parts->IsEquipped(s_hgPartTids[i], 0))
		{
			bFind = true;
			break;
		}
	}
	return bFind;
}

void CGolfRule::_ImeReset()
{
	if (IsLocalContent((localContentType_t)0x2b /*983: era 0x2a; interpolado*/) &&
		!CChatMsg::Instance()->IsActive())
	{
		g_ime->SetActive(true);
		g_ime->SetActive(false);
		g_ime->Reset();
	}
}

bool CGolfRule::OnPlayerInfoDlgResult(int, FrForm*)
{
	m_unused983_474 = 0;
	return true;
}

RB_ILPAD(A,
	ILFILL256 ILFILL256 ILFILL64 ILFILL64 ILFILL64 ILFILL16 ILFILL16 ILFILL4)
int CGolfRule::HoleReplay2()
{
	return 0;
}

RB_ILPAD(B,
	ILFILL256 ILFILL64 ILFILL16 ILFILL16 ILFILL16 ILFILL4 ILFILL4 ILFILL4)
void CGolfRule::ReplayBoxClose()
{
	if (m_pReplayControlDlg)
		m_pReplayControlDlg->Close(true);
	if (m_pReplayBox)
		m_pReplayBox->SetVisible(true);
}

RB_ILPAD(C, ILFILL256 ILFILL64 ILFILL16 ILFILL4 ILFILL4 ILFILL4 v += 1; v += 1;)
bool CGolfRule::IsLastShotRelplay()
{
	return (m_replayState == 2) ? true : false;
}

void CGolfRule::ReplayBoxOpen()
{
	if (m_pReplayControlDlg == NULL)
	{
		WPoint pos;
		pos.x = 571.0f - (800.0f - g_view->GetWidth());
		m_pReplayControlDlg = CreateForm<FrReplayControlDlg>(
			g_pFresh->GetManager(), this, "replaycontroldlg", NULL);
		m_pReplayControlDlg->Open(
			(FRESH_PFN_RESULT)&CGolfRule::OnReplayModeDlgResult,
			WPoint(pos.x, 55.0f), 0); // 983 0067d4f0: flag 0 (o 645 passava 0x11 = ESC fecha + sem icone)
	}
	else
		m_pReplayControlDlg->SetVisible(true);
	if (m_pReplayBox)
		m_pReplayBox->SetVisible(false);
}

// JP 983 (006775f0): resultado 1 zera o buraco fixo da repeticao (0xff); volta ao Lobby por ChangeTask + as msgs TOPPAGE/
// CMyRoomWidget(0x88)/myroom|char(0x55)/0xeb=2 (o 645/A469 mandava a msg 0xc6 ao RealMyRoom, que o 983 nao emite).
bool CGolfRule::OnReplayModeDlgResult(int result, FrForm*)
{
	if (Doc()->m_playingMode == (eReplayModeType)2 && Doc()->m_replayState == 5)
	{
		Doc()->m_replayState = 0;
		if (result == 1)
			((CSharedDocJp*)Doc())->SetFixHole983(0xff);
		Doc()->m_rmrMode = 1;
		Doc()->m_replayTargetUID = Doc()->m_myInfo.info.dwUID;
		Doc()->m_playingMode = (eReplayModeType)3;
		SetBGMNone();
		CProjectG::Instance()->FromGolfToLobbySettings();
		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
			CTaskManager::Instance()->GetPreservedTask() != NULL);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"TOPPAGE", 0, 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88, (int)"CMyRoomWidget", (int)"TOPPAGE", 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x55, (int)"myroom|char", 0, 0, 0);
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0xeb, 2, 0, 0, 0);
	}
	m_pReplayControlDlg = NULL;
	return true;
}

// JP 983 (00b2eac0, sem nome): a fita de replay nao vale nos tipos de jogo 10/11 nem no realGameType 0x12 (o 645 lia
// os bytes crus 0x4668 == 10 e 0x4a49 == 14).
static bool CanUseReplayTape983()
{
	if ((unsigned char)(Doc()->m_golfGame.gameType - 10) <= 1)
		return false;
	return Doc()->m_roomInfo.realGameType != 0x12;
}

// JP 983 (00688af0): a fita e o item 0x1a000000 de m_myItemList (+0x375c; o 645 lia a lista crua em +0x11a4) e o
// limite de 10 e o tamanho de m_recordedItemList (+0x376c; 645 +0x11b4). Novos no 983: sem fita, o tipo 0x1a000000 so
// avisa (nao zera m_replayPlayType); com fita, Grand Prix (realGameType 0x14/0x15) e shortgame nao gravam.
void CGolfRule::ReplayTapeCheckAndUse()
{
	if (!CanUseReplayTape983())
	{
		if (CHelpMsg::Instance() != NULL)
			CHelpMsg::Instance()->SetHelpMsg(0x19, NULL);
		return;
	}
	unsigned long key;
	if (IsLocalContent(S4_REPLAY_SYSTEM))
		key = (Doc()->m_replayPlayType != 2) - 1 & 0x1a000000;
	else
		key = 0x1a000000;
	const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it;
	for (it = items.begin(); it != items.end() && it->typeId != key; ++it)
	{
	}
	Doc()->ReplaySavFileCheck();
	if (it == Doc()->GetConstMyItemList().end())
	{
		if (IsLocalContent(S4_REPLAY_SYSTEM))
		{
			switch (key)
			{
			case 0x1a00004f:
				Doc()->m_replayPlayType = 0;
				CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xcc\xc7\xc1\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
				break;
			case 0x1a000000:
				Doc()->m_replayPlayType = 0;
				CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xcc\xc7\xc1\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
				break;
			}
		}
		else
		{
			CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xcc\xc7\xc1\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
		}
	}
	else
	{
		if ((unsigned int)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)
		{
			CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae \xb4\xeb\xc8\xb8\xbf\xa1\xbc\xad\xb4\xc2 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xb8\xa6 \xc0\xfa\xc0\xe5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
		}
		else if (S5::CLASSICSRV::IsShortGame983())
		{
			CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xf4\xb0\xd4\xc0\xd3 \xb8\xf0\xb5\xe5\xbf\xa1\xbc\xad\xb4\xc2 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xb8\xa6 \xc0\xfa\xc0\xe5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
		}
		else if (IsLocalContent(S4_REPLAY_SYSTEM) && Doc()->m_recordedItemList.size() >= 10)
		{
			Doc()->m_replayPlayType = 0;
			CChatMsg::Instance()->AddChatMsg(K2L_Compatibility("\xb1\xe2\xb7\xcf\xb5\xc8 \xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xcc\xc7\xc1\xb8\xa6 10\xb0\xb3 \xb3\xd1\xb0\xd4 \xbb\xfd\xbc\xba \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 0xffff7878, true, false);
		}
		else if (OnlinePlay())
		{
			if (IsLocalContent(S4_REPLAY_SYSTEM))
			{
				int ret;
				switch (Doc()->m_golfGame.gameType)
				{
				case 0:
				case 1:
				case 4:
				case 5:
				case 6:
				case 7:
					ret = ShotDataSave(NULL);
					break;
				default:
					ret = 0;
					break;
				}
				if (ret == 1)
				{
					WSendPacket packet((enumClientPacket)0x4a);
					packet.Encode4(key);
					packet.Send(TO_GAME);
				}
				else if (ret == -1)
				{
					CChatMsg::Instance()->AddChatMsg(
						K2L_Compatibility(MakeStr(
							"\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc6\xc4\xc0\xcf\xc0\xcc 100\xb0\xb3\xb8\xa6 \xc3\xca\xb0\xfa\xc7\xcf\xbf\xa9 \xc0\xfa\xc0\xe5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xbb\xe8\xc1\xa6 \xc8\xc4 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.")),
						0xffff7878, true, false);
				}
				else if (ret == 0)
				{
					CChatMsg::Instance()->AddChatMsg(
						K2L_Compatibility(MakeStr(
							"\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc\xb8\xa6 \xc0\xfa\xc0\xe5\xc7\xcf\xb4\xc2 \xb5\xb5\xc1\xdf \xb9\xae\xc1\xa6\xb0\xa1 \xbf\xc0\xb7\xf9\xb0\xa1 \xb9\xdf\xbb\xfd \xc7\xcf\xbf\xb4\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xcf\xbf\xa9 \xc1\xd6\xbd\xc3\xb1\xe2 \xb9\xd9\xb6\xf8\xb4\xcf\xb4\xd9.")),
						0xffff7878, true, false);
				}
			}
			else
			{
				WSendPacket packet((enumClientPacket)0x4a);
				packet.Encode4(key);
				packet.Send(TO_GAME);
			}
		}
		else
		{
			WSendPacket packet((enumClientPacket)0x49);
			packet.Send(TO_GAME);
		}
	}
}

void CGolfRule::OnRecordAviBtnUp()
{
	if (m_pRecordAviDlg == NULL)
	{
		const char* dir = GetPangyaInstallDic();
		char msg[255] = "";
		m_pRecordAviDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		sprintf(msg,
			"\xb8\xae\xc7\xc3\xb7\xb9\xc0\xcc \xc5\xd7\xc0\xd9\xc0\xc7 \xb3\xbb\xbf\xeb\xc0\xbb\n"
			"\xb5\xbf\xbf\xb5\xbb\xf3 \xc6\xc4\xc0\xcf\xb7\xce \xc0\xfa\xc0\xe5\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?\n\n"
			"\t\t\t\t\t\t   \xc0\xfa\xc0\xe5 \xb0\xe6\xb7\xce\n"
			"\t\t\t\t\t\t   %s/replay",
			dir);
		m_pRecordAviDlg->SetMessage(msg, false);
		m_pRecordAviDlg->Open((FRESH_PFN_RESULT)&CGolfRule::OnRecordAviResult,
			0);
	}
}

bool CGolfRule::OnRecordAviResult(int result, FrForm*)
{
	if (result == 1)
		Doc()->m_replayState = 9;
	m_pRecordAviDlg = NULL;
	return true;
}

char* CGolfRule::GetPangyaInstallDic()
{
	CRegistry registry;
	registry.SetRootKey(HKEY_LOCAL_MACHINE);
	registry.SetKey("Software\\Ntreev\\Pangya_Jp", true);
	WTL::CString directory = "";
	directory = registry.ReadString("Install_Dir", "");
	static char path[255];
	sprintf(path, "%s", directory);
	return path;
}

void CGolfRule::AddSomeInfo(WSendPacket& packet)
{
	if ((unsigned char)(rand() % 100) < 0x1e)
	{
		packet.Encode2(1);
		CCardManager* cardMgr = CCardManager::Instance();
		struct
		{
			unsigned long tid;
			unsigned long avility;
		} entry;
		std::list<unsigned long>& cards = cardMgr->GetActiveCardList();
		unsigned long count = cards.size();
		unsigned int offset = packet.GetLengthData();
		packet.Encode4(count);
		for (std::list<unsigned long>::iterator it = cards.begin();
			it != cards.end(); ++it)
		{
			IFF_STRUCT::sCard* card = Doc()->m_itemManager.FindCard(*it);
			if (card == NULL)
			{
				count--;
			}
			else
			{
				entry.tid = card->c.TypeId;
				entry.avility = *(unsigned long*)&card->Avility;
				packet.EncodeBuffer(&entry, 8);
			}
		}
		if (count != cards.size())
			packet.EncodeData(offset, (char*)&count, 4);
		// 983 (006809e0): sem IsGlobalWind_Compare (m_netWind nao existe); item 5 casos -> (1,1); intensidade = GetPower983 (fistp);
		// flag do vento em Wind+0x170; sem o ramo HACKINGTOOL o 983 grava so a intensidade.
		if (IsLocalContent(S4_HACKINGTOOL))
		{
			unsigned long item = PLAYER(GOLFDOC()->m_currentPlayer)->item;
			if (item == 0x18000006 || item == 0x18000020 || item == 0x1800002c ||
				item == 0x1800002d || item == 0x1800002f)
			{
				packet.Encode1(1);
				packet.Encode4(1);
			}
			else
			{
				int intensity = (int)floorf(Wind().GetPower983() + 0.5f);
				if (Wind().IsSpecial983() != 0) // 983 +0x170 = m_bSpecial (cl 14.44: +0x16c)
				{
					packet.Encode1(2);
					packet.Encode4(intensity);
				}
				else
				{
					packet.Encode1(0);
					packet.Encode4(intensity - 1 == 0 ? intensity : intensity - 1);
				}
			}
		}
		else
		{
			packet.Encode4((int)floorf(Wind().GetPower983() + 0.5f));
		}
	}
	else
	{
		packet.Encode2(0);
	}
}

void CGolfRule::ProcessObserver()
{
	if ((Doc()->m_myInfo.info.dwIdentity & 0xe) != 0xe)
		return;
	if ((g_input->GetDown("NUM_9", true) || g_input->GetDown("PAD_9", true)) &&
		(GOLFDOC()->m_gameMode & 0x8338))
	{
		ChangeGameMode(0x200000);
		return;
	}
	if (Doc()->m_golfGame.gameType == 0 &&
		(g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("INSERT", true))
	{
		WSendPacket packet((enumClientPacket)0x5d);
		packet.Send(TO_GAME);
	}
	if (!IsMassGame())
		return;
	if (GOLFDOC()->m_gameMode == 0x2000)
		return;
	// 983 (00694c30): o relogio do observador fica em this+0x460 (m_observerReqTick); o 645 lia this+0x40c
	if (g_CurrentTime - m_observerReqTick <= 3000)
		return;
	int key = 1;
	unsigned long guid = 0xffffffff;
	while (true)
	{
		if (g_input->GetDown(MakeStr("PAD_%d", key), true))
			break;
		if (g_input->GetDown(MakeStr("NUM_%d", key), true))
			break;
		key++;
		if (key > 8)
			return;
	}
	unsigned char count = 0;
	CSharedDoc* shared = Doc();
	for (char* e = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
		e != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430)); e += 0x1b8)
	{
		if (OBS_UCHAR(e, 0x36) == 0)
		{
			OBS_UCHAR(e, 0x168) = 1;
			count++;
			for (char* f = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				f != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430)); f += 0x1b8)
			{
				if (OBS_UCHAR(f, 0x36) == 0 && e != f)
				{
					if (OBS_INT(e, 0x14c) > OBS_INT(f, 0x14c) ||
						(OBS_INT(e, 0x14c) == OBS_INT(f, 0x14c) &&
							(*(__int64*)(e + 0x128) < *(__int64*)(f + 0x128) ||
								(*(__int64*)(e + 0x128) ==
										*(__int64*)(f + 0x128) &&
									OBS_UCHAR(e, 0x153) <
										OBS_UCHAR(f, 0x153)))))
						OBS_UCHAR(e, 0x168)++;
				}
			}
		}
		else
			OBS_UCHAR(e, 0x168) = 0;
	}
	char* it;
	switch (key)
	{
	case 1:
		for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
			it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430)); it += 0x1b8)
		{
			// 983 (00694c30): FUN_00687980 = map[guid]; testa dwIdentity (+0x21) bit 4; o 645 lia +0x84
			if ((Doc()->m_briefUserInfoMap[OBS_UINT(it, 4)].dwIdentity >> 4) & 1)
			{
				guid = OBS_UINT(it, 4);
				break;
			}
		}
		break;
	case 2:
	{
		unsigned char pick = rand() % count;
		for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
			it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430)); it += 0x1b8)
		{
			if (OBS_UINT(it, 4) != Doc()->m_myInfo.info.dwGalleryGuid &&
				OBS_INT(it, 0x164) != 1 && OBS_UCHAR(it, 0x36) == 0 &&
				!((Doc()->m_briefUserInfoMap[OBS_UINT(it, 4)].dwIdentity >> 4) & 1) &&
				OBS_UINT(it, 0x16c) != 0x30000001 &&
				OBS_UCHAR(it, 0x168) != 1 && OBS_UCHAR(it, 0x168) != 2 &&
				OBS_UCHAR(it, 0x168) != 3 && pick == 0)
			{
				guid = OBS_UINT(it, 4);
				break;
			}
			pick--;
		}
		break;
	}
	case 3:
		if (count != 0)
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UCHAR(it, 0x168) == 1)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	case 4:
		if (count > 1)
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UCHAR(it, 0x168) == 2)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	case 5:
		if (count > 2)
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UCHAR(it, 0x168) == 3)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	case 6:
		if (count != 0)
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UCHAR(it, 0x168) == count)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	case 7:
		if (count > 1)
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UCHAR(it, 0x168) == count - 1)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	case 8:
		if (IsLocalCountry(1))
		{
			for (it = (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c));
				it != (char*)*(unsigned long*)((char*)shared + DOCOFS983(0x10430));
				it += 0x1b8)
				if (OBS_UINT(it, 0x16c) == 0x30000001)
				{
					guid = OBS_UINT(it, 4);
					break;
				}
		}
		break;
	}
	shared = Doc();
	unsigned char index = shared->GetIndex(guid);
	sObserverUser user =
		*(sObserverUser*)((char*)*(unsigned long*)((char*)shared + DOCOFS983(0x1042c)) +
			index * 0x1b8);
	if (guid != 0xffffffff && guid != Doc()->m_myInfo.info.dwGalleryGuid &&
		OBS_INT(user.data, 0x164) == 0 && OBS_UCHAR(user.data, 0x36) == 0)
	{
		m_observerReqTick = g_CurrentTime;
		WSendPacket packet((enumClientPacket)0x3f);
		packet.Encode4(guid);
		packet.Send(TO_GAME);
	}
}

void CGolfRule::HoleComboGaugeSetting()
{
	// JP 983 (0067d5a0): combo = +1 por aux 0x70010010 e +1 por 0x70010012 (charInfo.tidAuxParts[0]), +1 pelo mascote
	// 0x40000015 ou 0x40000017 (mascotInfo.tid, +0x318c do sUserInfo); a barra e limitada a [0, limite do jogador]
	// e, no modo 1 (equipes), grava na barra da equipe com o maior limite entre o jogador e o parceiro.
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 0xb:
	{
		unsigned char who = GOLFDOC()->m_currentPlayer;
		sUserInfo& user = Doc()->m_userInfo[who];
		float combo = 0.0f;
		if (user.charInfo.tidAuxParts[0] == 0x70010010)
			combo = 1.0f;
		if (user.charInfo.tidAuxParts[0] == 0x70010012)
			combo = combo + 1.0f;
		if (user.mascotInfo.tid == 0x40000015)
			combo = combo + 1.0f;
		else if (user.mascotInfo.tid == 0x40000017)
			combo = combo + 1.0f;
		unsigned char cur = GOLFDOC()->m_currentPlayer;
		combo = PLAYER(cur)->gauge + combo;
		float limit = Doc()->GetComboGaugeLimit(cur);
		float result = 0.0f;
		if (0.0f <= combo)
			result = combo > limit ? limit : combo;
		PLAYER(GOLFDOC()->m_currentPlayer)->gauge = result;
		return;
	}
	}
	for (int i = 0; i < GOLFDOC()->m_playerNum; i++)
	{
		sUserInfo& user = Doc()->m_userInfo[i];
		float combo = 0.0f;
		if (user.charInfo.tidAuxParts[0] == 0x70010010)
			combo = 1.0f;
		if (user.charInfo.tidAuxParts[0] == 0x70010012)
			combo = combo + 1.0f;
		if (user.mascotInfo.tid == 0x40000015)
			combo = combo + 1.0f;
		else if (user.mascotInfo.tid == 0x40000017)
			combo = combo + 1.0f;
		float limit;
		float value;
		float result = 0.0f;
		if (Doc()->m_golfGame.gameType == 1)
		{
			unsigned char mate = PLAYER(i)->teammate;
			limit = Doc()->GetComboGaugeLimit((unsigned char)i);
			if (mate != 0xff)
			{
				float other = Doc()->GetComboGaugeLimit(mate);
				if (limit < other)
					limit = other;
			}
			value = TEAM(PLAYER(i)->team)->gauge + combo;
			if (0.0f <= value)
				result = value > limit ? limit : value;
			TEAM(PLAYER(i)->team)->gauge = result;
		}
		else
		{
			value = PLAYER(i)->gauge + combo;
			limit = Doc()->GetComboGaugeLimit((unsigned char)i);
			if (0.0f <= value)
				result = value > limit ? limit : value;
			PLAYER(i)->gauge = result;
		}
	}
}

void CGolfRule::CollectBallStopData()
{
	int playType = Doc()->m_replayPlayType;
	Doc()->m_replayPlayType = 2;
	if (ShotDataSave(ball_stop_replay_data_name) == 1)
		NetResManager()->UploadBugReportData(ball_stop_replay_data_name, 1,
			(GolfBall().m_rollFrame != -1) + 1, true);
	Doc()->m_replayPlayType = playType;
}

void CGolfRule::EncodePacket(enumClientPacket type, WSendPacket& packet)
{
	if (type == 0x1c)
	{
		sPlayerData* current = PLAYER(GOLFDOC()->m_currentPlayer);
		unsigned char myIndex = MyIndex();
		sPlayerData* mine = PLAYER(myIndex);
		if (current->oid == mine->oid)
			packet.Encode1(1);
		else
			packet.Encode1(0);
		std::vector<FieldItem::AcquiredItemInfo>* items = NULL;
		SendMsg(this, "GroundItemMan", M983(0x277), (int)&items, 0, 0, 0);
		if (items)
		{
			packet.Encode1((unsigned char)items->size());
			for (std::vector<FieldItem::AcquiredItemInfo>::const_iterator it =
					 items->begin();
				it != items->end(); ++it)
			{
				// JP 983 (00688f31, C-PKT lote 10): so tipo e indice por item (elemento de 8 bytes no exe).
				packet.Encode1((unsigned char)(*it).type);
				packet.Encode4((*it).index);
			}
			items->clear();
		}
		else
			packet.Encode1(0);
	}
}

#if _MSC_VER == 1600 // D740: especializacoes do STL do VC10 (o STL novo nao tem)
template <>
inline size_t std::allocator<sCards>::max_size() const;
template <>
inline void std::list<sRoomInfo>::clear();
template <>
inline size_t std::allocator<sRivalData>::max_size() const;
template <>
inline void std::list<COption::sDisplayMode>::clear();
#endif
RB_ILPAD(D, ILFILL256 ILFILL256 ILFILL256 ILFILL64)

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?SetInstalledCamera@CCameraWork@@QAEXPBVWVector@@@Z=?SetInstalledCamera@CCameraWork@@QAEXABVWVector@@@Z") // ponteiro x ref
