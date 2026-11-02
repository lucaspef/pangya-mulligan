#include "minatl.h"
#include "clientsetting.h"
#include "mousecursor.h"
#include "realmyroomdata.h"
#include "realmyroommain.h"
#include "vc10_layout.h"
#include "ccardbook.h"
#include "mathconsts.h"
#include "realmyroomavatar.h"
#include "shadow.h"
#include "taskmanager.h"
#include "avatar_task.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "rtti.h"
#include "uccmanager.h"
#include "exhibition.h"
#include "partsinfo.h"
#include "parttidlist.h"
#include "ccamerawork.h"
#include "projectg.h"
#include "sky.h"
#include "wind.h"
#include "fx.h"
#include "quadtree.h"
#include "polysoup.h"
#include "scenemanager.h"
#include "inputmanager.h"
#include "chatmsg.h"
#include "messengerdlg.h"
#include "tooltip.h"
#include "standinginline.h"
#include "../shared/sharedtables.h"
#include "s5/utilities.h"
#include "packets/realmyroompackets.h"	// PKT (frente C)
#include "jp/clubsetresetwnd.h" // B89: FrClubSetResetWnd::s_pInstance (00e74254)



// the original reads CTaskManager::m_pCurrentTask (+0x38) inline
#define CURRENT_TASK() (CTaskManager::Instance()->GetPreservedTask()) // VC10 +0x38 (B1466: cl 14.44 +0x34)

extern "C" const char rb_myroomItemBuffName[] = "ItemBuff";
extern "C" const char rb_myroomTaskTitle[] =
	"\xb8\xae\xbe\xf3\xb8\xb6\xc0\xcc\xb7\xeb";

extern bool g_bQuit;
extern Fresh* g_pFresh;
extern WMatrix g_camera;
WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

IMPLEMENT_ACTOR(CRealMyRoomMain, CTaskMain)

// Declarations missing from the shared headers, used only by the C of OnLoad
// (helper types instead of new names in the PCH headers).
struct sPolyPoint
{
	char name[0x24];
	WVector pos;
};


class WPolySoupEx : public WPolySoup
{
public:
	sPolyPoint* FindPoint(const char* name);
};

struct sQuadNode
{
	unsigned char pad[0x60];
	Waabb aabb;
};

class CQuadTreeEx : public CQuadTree
{
public:
	void SetAvatarPosition(const WVector& pos, float radius);
};

// root node of the room quadtree (a macro: an inline member would be emitted out of line
// by the dead copy of the naked OnLoad)
#define MYROOM_QUADROOT() (*(sQuadNode**)((char*)&GetPVS() + 4))

class CWindEx : public CWind
{
public:
	void Init(unsigned char a, unsigned char b, unsigned char c,
		unsigned char d);
	void SetWind(const WVector& wind);
};

class CSceneManagerWind : public CSceneManager
{
public:
	void UpdateWind();
};

class CRealMyRoomTask
{
public:
	void AddAvatarBurden(const CRealMyRoom_AvatarBurden& burden);
};

namespace _util
{
	void GenerateRandomTidParts(unsigned long tid, unsigned long* const parts,
		unsigned long* const auxParts);
}

#define MYROOM_POLYSOUP() ((WPolySoupEx*)GOLFDOC()->m_pPolySoup)
// o 983 le o pCourse duas vezes (cmp/mov); membro direto junta as leituras -> offsetof (anda com o layout do cl novo)
#define MYROOM_COURSE983() \
	(*(char**)((char*)CSharedDoc::Instance() + offsetof(CSharedDoc, m_golfGame.pCourse)))
#define MYROOM_MAPNAME() \
	(MYROOM_COURSE983() \
			? MYROOM_COURSE983() + 0x90 \
			: NULL)

// A734: crus 0x10590/0x8cb/0x10708/0x1048c do CSharedDoc -> membros (no cl novo o STL do VC10 desloca)
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.pCourse) == 0x10590);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwUID) == 0x8cb);
C_ASSERT_VC10(offsetof(CSharedDoc, m_replayTargetUID) == 0x10708);
C_ASSERT_VC10(offsetof(CSharedDoc, m_gameMode) == 0x1048c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip) == 0xa26); // A737

#pragma comment(linker, "/alternatename:@rb_InitMyRoomControls@4=__pg_c_003084")
#pragma comment(linker, "/alternatename:@rb_InitMyRoomDialogs@4=__pg_c_0032ba")
#pragma comment(linker, \
	"/alternatename:@rb_InitMyRoomVariables@4=__pg_c_0032bb")

// m_p210: the right-click popup window (FrWnd: rect at +0x20, style at +0x30, virtual slot 7 hides it)
#define MYROOM_POPUP_VISIBLE(p) (*(unsigned char*)((char*)(p) + 0x30) & 1)
#define MYROOM_POPUP_RECT(p) (*(WRect*)((char*)(p) + 0x20))
class CMyRoomPopupWnd
{
public:
	virtual void V00();
	virtual void V01();
	virtual void V02();
	virtual void V03();
	virtual void V04();
	virtual void V05();
	virtual void V06();
	virtual void Hide(int fade);
};
#define MYROOM_POPUP_HIDE(p) (((CMyRoomPopupWnd*)(p))->Hide(0))
// my own room: CSharedDoc +0x4eb (my uid) == +0x47d4 (room owner uid)
#define MYROOM_IS_OWNER() \
	(*(int*)&CSharedDoc::Instance()->m_myInfo.info.dwUID == \
		*(int*)&CSharedDoc::Instance()->m_replayTargetUID)
// FrDesktop flags (+8): FWF_VIEWFOCUS
#define MYROOM_VIEW_FOCUSED() \
	((*(unsigned long*)((char*)g_pFresh->GetManager()->GetDesktop() + 8) >> \
		 3) & \
		1)

int WisZero(const float& v, float e);
bool IsEqual(float a, float b);

// CChatMsg::Open(bool, bool) is not declared in the shared header
class CChatMsgEx : public CChatMsg
{
public:
	void Open(bool a, bool b);
};

class CQuadTreeWall : public CQuadTree
{
public:
	WVector GetRayIntersectionWall(WVector start, WVector ray);
	void GetAABB(Waabb& aabb);
};

#pragma comment(linker, \
	"/alternatename:?Open@CChatMsgEx@@QAEX_N0@Z=?Open@CChatMsg@@QAEX_N0@Z")
#pragma comment(linker, \
	"/alternatename:?GetAABB@CQuadTreeWall@@QAEXAAVWaabb@@@Z=?GetAABB@CQuadTree@@QAEXAAVWaabb@@@Z")
#pragma comment(linker, \
	"/alternatename:?GetRayIntersectionWall@CQuadTreeWall@@QAE?AVWVector@@V2@0@Z=?GetRayIntersectionWall@CQuadTree@@QAE?AVWVector@@V2@0@Z")

// HACK: the shared scene header omits this member; its body lives on the PCH stand-in
// CSceneManagerFog::SetFog() (stdafx.h), at the original's PCH position.
#pragma comment(linker, \
	"/alternatename:?ToogleNoOverlapCamera@CSceneManager@@QAE_NXZ=?SetFog@CSceneManagerFog@@QAE_NXZ")
#define MYROOM_TOGGLE_NO_OVERLAP_CAMERA() \
	(((CSceneManagerFog*)CSceneManager::Instance())->SetFog())

CRealMyRoomMain::CRealMyRoomMain()
{
	// HACK: retains inline bodies originally referenced by OnProcess; asm uses aliases.
	if (0)
	{
		((CRealMyRoom_Avatar*)0)->IsHoldState();
		((CRealMyRoom_Avatar*)0)->GetPartTidList();
		((CRealMyRoom_Avatar*)0)->GetHide();
		((CRealMyRoom_Avatar*)0)->GetMotion();
		((CRealMyRoom_Avatar*)0)->GetAction();
		((CAnotherHand*)0)->IsWorking(0);
		((CAnotherHand*)0)->Enter();
		((CAnotherHand*)0)->Leave();
		((CIconToolTip*)0)->GetCurTipIndex();
		// same for OnInit (inline naked: its dead copy retains nothing)
		CCameraWork::m_pInstance->SetManualType(0);
		m_mapChar = CSharedDoc::Instance()->m_charMap;
	}
	// HACK: retains the empty camera constructor emitted by the original TU.
	if (0)
		new ((void*)0) WPolySoup::sCamera;
	// HACK: forces storage of the wmath.h constants the original referenced by memory
	(void)g_PI;
	(void)g_DEGTORAD;
	m_dispPriority = DISP_PRIORITY_4;
	m_time270 = 0;
	m_v220 = WVector::ZERO;
	m_v214 = WVector(0.0f, 0.0f, 0.0f);
	m_fovCur = 28.800001f;
	m_fovTarget = 28.800001f;
	m_f234 = 28.800001f;
	m_f244 = 0;
	m_f248 = 0;
	m_f238 = 1.0f;
	m_f23c = 4.6f;
	m_f240 = 4.6f;
	m_v2c8 = WVector::ZERO;
	m_bTopView = false;
	m_topViewDist = 256.0f;
	m_f274 = -40.0f;
	m_f278 = 40.0f;
	m_f27c = -25.0f;
	m_f280 = 25.0f;
	m_f284 = 0;
	m_f7b0 = 0;

	WVector vUnused(0.0f, 0.0f, 0.0f);

	m_pTitleFont2 = g_resrcmng->GetTitleFont();
	m_b209 = false;
	m_glowBallTex = g_resrcmng->LoadTexture("~GlowBall.jpg", 0, 0, NULL);

	char* fontFile = "[font_wind.jpg";
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = &fontFile;
	info.fonth = 16;
	info.fontw = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:, ";
	m_pTitleFont2->Create(&info);

	m_f694 = 0;
	m_fc4c = 0;
	m_pCamMain = NULL;
	m_fc54 = 0;
	m_fc48 = 0;
	m_fc6c = 0;
	m_pFxSeq = NULL;
	m_bCamFollow = false;
	m_fce8 = 1;
	m_clipNear = g_view->GetClipNearValue();
	m_clipFar = g_view->GetClipFarValue();
	m_p210 = NULL;

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		m_fba0 = 0;
		LoadCardImages();
	}

	m_pCardBook = new CCardBook;
	m_pFrListBox_DCC = 0;
	m_cutinImgMap.clear();
	m_f2b8 = 0xffffffff;
}

CRealMyRoomMain::miniBtn_t::miniBtn_t()
{
}

CRealMyRoomMain::~CRealMyRoomMain()
{
	// HACK: emission order only (OnLoad is an inline naked function, see its HACK note)
	if (0)
		CRealMyRoomMain::OnLoad();
	// JP 983: a sequencia de equipar, o HidePet, a parada da mao e a liberacao do CCardBook e das imagens ficam na virtual
	// 006165d0 (FrTaskForm::OnCloseWidget983, pedido C27); enquanto a base for CTaskMain, o destrutor a chama
	OnCloseWidget983();

	this << MsgObject(NULL, M983(0x2), 0, 0, 0, 0, 0);

	if (m_pFxSeq)
		CFx::Instance()->CloseSequence(m_pFxSeq, false);

	if (g_resrcmng)
	{
		if (m_glowBallTex)
		{
			g_resrcmng->Release(m_glowBallTex);
			m_glowBallTex = 0;
		}
		if (g_resrcmng && m_pTitleFont2)
		{
			g_resrcmng->Release(m_pTitleFont2);
			m_pTitleFont2 = NULL;
		}
	}

	CSceneManager::Instance()->ReleaseElement();
}

// RFWnd (biblioteca Refresh, sem cabecalho) e os dois globais que a 006165d0 solta; nomes meus (A s98)
class CRFWnd983
{
public:
	void Close(int closeState); // 00408080 RFWnd::Close(RFWNDCLOSESTATE): void, o 983 empurra 1 (A s159)
};
extern CRFWnd983* g_pMyRoomCardWnd983; // 00e78154 (RFCardIcon 005bccc0, CTaskMain::HandleMsgCommon)

// JP 983 (006165d0): slot 39 da vtable 00d1497c = FrTaskForm::OnCloseWidget983 (+0x9c). Diferencas para o destrutor do
// 645: tipos 9 e 10 do 0x20 depois do Net_EquipMascot; SendEquipItemSlot e DeleteCardImages sem o teste do sistema de
// cartas; solta os dois globais acima; devolve verdadeiro.
bool CRealMyRoomMain::OnCloseWidget983()
{
	Net_EquipDresserParts();
	Net_EquipCharacter();
	Net_EquipCaddie();
	Net_EquipItem();
	Net_EquipGear();
	Net_EquipSkin();
	Net_EquipMascot();
	Net_EquipCutIn983();
	Net_EquipPoster983();
	CSharedDoc::Instance()->SendEquipItemSlot();

	HidePet(true);
	g_resrcmng->UnlockAllByThread(m_hand.GetThreadID());
	m_hand.BreakWorkingThread();

	if (m_pCardBook)
	{
		delete m_pCardBook;
		m_pCardBook = NULL;
	}
	DeleteCutinImages();
	DeleteCardImages();

	if (FrClubSetResetWnd::s_pInstance) // 00e74254 (pedido B89)
	{
		delete FrClubSetResetWnd::s_pInstance;
		FrClubSetResetWnd::s_pInstance = NULL;
	}
	// 983: aqui chama FrTaskForm::OnCloseWidget983 (00523610) da base
	if (g_pMyRoomCardWnd983)
		g_pMyRoomCardWnd983->Close(1);
	return true;
}

void CRealMyRoomMain::OnPreLoadInit()
{
	CTaskMain::OnPreLoadInit();
	if (!IS_EXACTKINDOF(ntAvatarChatTask, CURRENT_TASK()))
	{
		new CShadowManager(g_resrcmng, COption::Instance()->vIsShadowEnabled());
	}
	*(unsigned long*)&CSharedDoc::Instance()->m_gameMode = 0;
	InitMyRoomControls(); // ramo semântico: os rb_InitMyRoom* são apelidos destes 3 métodos
	InitMyRoomDialogs();
	InitMyRoomVariables();
	CSharedDoc::Instance()->m_pGolfDoc->MakePlayer(1);
}

void CRealMyRoomMain::DoReLoad()
{
	SendMsg(this, "Ambient", M983(0x1f4), 0, 0, 0, 0);

	OnLoad();

	AfxGetTask()->Init(NULL);
}

void CRealMyRoomMain::OnInit()
{
	CTaskMain::OnInit();
	SetBGMNone();

	CSceneManager::Instance()->UpdateFog(true);
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, this,
		"\xb8\xae\xbe\xf3\xb8\xb6\xc0\xcc\xb7\xeb", 0, NULL);

	g_input->Reset();

	if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
	{
		m_v220 = CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos;
		m_v220.y += 4.6f;
		m_v220.z -= 46.0f;
	}

	m_mapChar = CSharedDoc::Instance()->m_charMap;
	m_enterInfo = *(sMyRoomEnterInfo*)&CSharedDoc::Instance()->m_myInfo.userEquip;

	if (CMouseCursor::Instance())
	{
		CMouseCursor::Instance()->SetActive(true);
		CMouseCursor::Instance()->SetMode(2);
	}

	CCameraWork::m_pInstance->SelectCamera(200);
	CCameraWork::m_pInstance->SetManualType(0);

	g_camera.Rotate(-0.17453292f, 0);
	m_pCamMain->m_mat = g_camera;
	m_pCamMain->m_mat.pivot.y = g_camera.pivot.y - 5.0f;
	m_pCamMain->m_mat.Rotate(0.34906584f, 1);
	m_pCamMain->m_mat.pivot.x -= 5.0f;
	m_pCamMain->m_fov =
		g_view->GetFOV();

	m_matCam = g_camera;
	m_fovCam = g_view->GetFOV();
	m_matCam.pivot.y -= 5.0f;
	g_camera.pivot.y -= 7.0f;
	m_fd4c = 0;

	g_view->SetFOV(0.7853982f);
}

void CRealMyRoomMain::OnLoad()
{
	sSlotInfo slot;
	sCharacterInfo info;
	char name[256];

	CCameraWork::m_pInstance->Load();

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		// offline: the room with dummy avatars
		SetCurMap((unsigned char)COption::Instance()->dGetMap());
		LoadBg(MYROOM_MAPNAME(), COption::Instance()->dGetStartHole());

		WPolySoup::sCamera* cam =
			MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		if (cam)
		{
			g_camera = cam->mat;
			g_view->SetFOV(cam->fov * g_DEGTORAD);
		}
		m_pCamMain =
			(CCameraWork*)MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		m_fc54 = (unsigned long)MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		m_fovCur = m_fovTarget = cam->fov;
		m_fc48 = (unsigned long)MYROOM_POLYSOUP()->FindCamera(
			"\xc4\xab\xb8\xde\xb6\xf3\xb3\xa1\xc1\xa1");
		m_fc4c = (unsigned long)MYROOM_POLYSOUP()->FindCamera(
			"\xc3\xca\xb1\xe2\xb0\xaa");
		m_fc6c = (unsigned long)MYROOM_POLYSOUP()->FindPoint(
			"\xbd\xc3\xc0\xdb\xc1\xa1");
		for (int i = 0; i < 4; i++)
		{
			sprintf(name, "savecamera%d", i);
			m_pCamList[i] = (CCameraWork*)MYROOM_POLYSOUP()->FindCamera(name);
		}
		m_pCamSel = (CCameraWork*)MYROOM_POLYSOUP()->FindCamera("uicamera0");

		CSceneManager::Instance()->SetWeather(
			(unsigned char)COption::Instance()->dGetWeather(), false);
		CRealMyRoom_AvatarManager::Instance()->FreeAllAvatars();

		sPolyPoint* point = MYROOM_POLYSOUP()->FindPoint(
			MakeStr("\xb8\xb6\xc0\xcc\xb7\xeb\xbd\xc3\xc0\xdb\xc1\xa1"));
		if (!point)
			point = MYROOM_POLYSOUP()->FindPoint(
				MakeStr("\xb4\xeb\xc8\xad\xb9\xe6\xbd\xc3\xc0\xdb"));
		WVector pos = point->pos;
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
		m_v2c8 = pos;
		((CQuadTreeEx&)GetPVS()).SetAvatarPosition(pos, 2.0f);
		m_vCamLook = pos;

		m_pFxSeq = CFx::Instance()->OpenSequence("room_point.seq", true);
		if (m_pFxSeq)
			m_pFxSeq->m_pos = pos; // A844: era visao crua VC10_FXSEQ_POS

		// my avatar
		memset(&slot, 0, sizeof(slot));
		slot.bMaster = 1;
		slot.location[2] = 0.0f;
		slot.dwGuid = 0;
		slot.dwTitle = 0;
		slot.level = 1;
		slot.gender = rand() % 1;
		slot.location[0] = pos.x;
		slot.location[1] = pos.z;
		strcpy(slot.sNick, "bianca");
		slot.tidChar = COption::Instance()->dGetDefaultCharacter() | 0x4000000;
		CSharedDoc::Instance()->m_slotList.push_back(slot);

		memset(&info, 0, sizeof(info));
		info.tid = slot.tidChar;
		_util::GenerateRandomTidParts(info.tid, info.tidParts,
			info.tidAuxParts);
		CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar =
			CRealMyRoom_AvatarManager::Instance()->CreateAvatar(&slot, &info,
				this);
		((CRealMyRoomTask*)GetTask())
			->AddAvatarBurden(CRealMyRoom_AvatarBurden(0, &info, NULL));

		// the dummies
		for (unsigned long guid = 1;
			(int)guid < COption::Instance()->dGetNumPlayers(); guid++)
		{
			point = MYROOM_POLYSOUP()->FindPoint(
				MakeStr("\xb4\xeb\xc8\xad\xb9\xe6\xbd\xc3\xc0\xdb"));
			if (!point)
				point = MYROOM_POLYSOUP()->FindPoint(
					MakeStr("\xbd\xc3\xc0\xdb\xc1\xa1"));
			pos = point->pos;
			pos.x += Random(-16.0f, 16.0f);
			pos.z += Random(-16.0f, 16.0f);
			pos = GetPVS().GetGroundPoint(pos, false, NULL, false);

			memset(&slot, 0, sizeof(slot));
			slot.location[2] = Random(0.0f, 6.2831855f);
			slot.bMaster = 0;
			slot.dwGuid = guid;
			slot.dwTitle = 0;
			slot.level = 1;
			slot.gender = rand() % 1;
			slot.location[0] = pos.x;
			slot.location[1] = pos.z;
			strcpy(slot.sNick, MakeStr("dummy%2d", guid));
			slot.tidChar = rand() % 7 | 0x4000000;
			CSharedDoc::Instance()->m_slotList.push_back(slot);

			memset(&info, 0, sizeof(info));
			info.tid = slot.tidChar;
			_util::GenerateRandomTidParts(info.tid, info.tidParts,
				info.tidAuxParts);
			CRealMyRoom_AvatarManager::Instance()->CreateAvatar(&slot, &info,
				this);
			((CRealMyRoomTask*)GetTask())
				->AddAvatarBurden(CRealMyRoom_AvatarBurden(guid, &info, NULL));
		}

		CRealMyRoomData::Instance()->OnLoad();
	}
	else
	{
		SetCurMap((unsigned char)COption::Instance()->dGetMap());
		LoadBg(MYROOM_MAPNAME(), COption::Instance()->dGetStartHole());

		WPolySoup::sCamera* cam =
			MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		if (cam)
		{
			g_camera = cam->mat;
			g_view->SetFOV(cam->fov * g_DEGTORAD);
		}
		m_pCamMain =
			(CCameraWork*)MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		m_fc54 = (unsigned long)MYROOM_POLYSOUP()->FindCamera("DefaultCamera");
		m_fovCur = m_fovTarget = cam->fov;
		m_fc48 = (unsigned long)MYROOM_POLYSOUP()->FindCamera(
			"\xc4\xab\xb8\xde\xb6\xf3\xb3\xa1\xc1\xa1");
		m_fc4c = (unsigned long)MYROOM_POLYSOUP()->FindCamera(
			"\xc3\xca\xb1\xe2\xb0\xaa");
		m_fc6c = (unsigned long)MYROOM_POLYSOUP()->FindPoint(
			"\xbd\xc3\xc0\xdb\xc1\xa1");
		for (int i = 0; i < 4; i++)
		{
			sprintf(name, "savecamera%d", i);
			m_pCamList[i] = (CCameraWork*)MYROOM_POLYSOUP()->FindCamera(name);
		}
		m_pCamSel = (CCameraWork*)MYROOM_POLYSOUP()->FindCamera("uicamera0");

		sPolyPoint* point = MYROOM_POLYSOUP()->FindPoint(
			MakeStr("\xb8\xb6\xc0\xcc\xb7\xeb\xbd\xc3\xc0\xdb\xc1\xa1"));
		if (!point)
			point = MYROOM_POLYSOUP()->FindPoint(
				MakeStr("\xbd\xc3\xc0\xdb\xc1\xa1"));
		WVector pos = point->pos;
		pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
		((CQuadTreeEx&)GetPVS()).SetAvatarPosition(pos, 2.0f);
		m_v2c8 = pos;

		m_pFxSeq = CFx::Instance()->OpenSequence("room_point.seq", true);
		if (m_pFxSeq)
			m_pFxSeq->m_pos = pos; // A844: era visao crua VC10_FXSEQ_POS
		m_vCamLook = pos;

		CSceneManager::Instance()->SetWeather(0, false);
		CRealMyRoom_AvatarManager::Instance()->FreeAllAvatars();
	}

	m_lightSet = g_lightset;

	GOLFDOC()->m_pSky->GenerateClouds(50, Wind().GetPower983());
	((CWindEx&)Wind()).Init(0xff, 0xff, 0, 1);
	Wind().CheckSpecialWind();
	((CWindEx&)Wind()).SetWind(WVector(1.0f, 0.0f, 1.0f));
	GOLFDOC()->m_pPolySoup->UpdateBgFxWind();
	((CSceneManagerWind*)CSceneManager::Instance())->UpdateWind();

	m_resD = g_resrcmng->LoadTexture("[shadow.jpg", 0, 0, NULL);
	MYROOM_TOGGLE_NO_OVERLAP_CAMERA();

	// guide lines of the room walls (from the bounding box of the map)
	Waabb box = MYROOM_QUADROOT()->aabb;
	WPolySoup::sModel* wall = MYROOM_POLYSOUP()->FindModel("wall_a.pet");
	float top = wall ? wall->aabb.max.y : box.max.y; // A874: era visao sPolyModel +0x70 (aabb do 645); sModel 983 aabb em +0x98
	m_vLineA0 = WVector(box.max.x, top, box.min.z);
	m_vLineA1 = WVector(box.max.x, box.min.y, box.min.z);
	*(WVector*)m_padc90 =
		WVector(box.max.x, (box.min.y + top) * 0.5f, box.min.z);
	m_vLineB0 = WVector(box.min.x, top, box.min.z);
	m_vLineB1 = WVector(box.min.x, box.min.y, box.min.z);
	*(WVector*)&m_padcb4[0] =
		WVector(box.min.x, (box.min.y + top) * 0.5f, box.min.z);
	*(WVector*)&m_padcb4[3] =
		WVector((box.min.x + box.max.x) * 0.5f, top, box.min.z);
	*(WVector*)&m_padcb4[6] =
		WVector((box.min.x + box.max.x) * 0.5f, box.min.y, box.min.z);

	m_resA[0] = g_resrcmng->GetOverlay("i_male.tga", 0);
	m_resA[1] = g_resrcmng->GetOverlay("i_female.tga", 0);
	m_resA[2] = g_resrcmng->GetOverlay("i_male_02.tga", 0);
	m_resA[3] = g_resrcmng->GetOverlay("i_female_02.tga", 0);
	m_resA[4] = g_resrcmng->GetOverlay("i_male_03.tga", 0);
	m_resA[5] = g_resrcmng->GetOverlay("i_female_03.tga", 0);
	m_resB[0] = g_resrcmng->GetOverlay("i_male_manner.tga", 0);
	m_resB[1] = g_resrcmng->GetOverlay("i_female_manner.tga", 0);
	m_resC[0] = g_resrcmng->GetOverlay("i_male_angel.tga", 0);
	m_resC[1] = g_resrcmng->GetOverlay("i_female_angel.tga", 0);

	CTaskMain::OnLoad();
	LoadMyRoomImages();
}

void CRealMyRoomMain::OnDestroy()
{
	CTaskMain::OnDestroy();

	if (!IS_EXACTKINDOF(ntAvatarChatTask, CURRENT_TASK()) &&
		CShadowManager::Instance())
	{
		delete CShadowManager::Instance();
	}

	for (int i = 0; i < 6; i++)
	{
		if (g_resrcmng && m_resA[i])
		{
			g_resrcmng->Release(m_resA[i]);
			m_resA[i] = NULL;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_resB[i])
		{
			g_resrcmng->Release(m_resB[i]);
			m_resB[i] = NULL;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		if (g_resrcmng && m_resC[i])
		{
			g_resrcmng->Release(m_resC[i]);
			m_resC[i] = NULL;
		}
	}

	if (g_resrcmng && m_resD)
	{
		g_resrcmng->Release(m_resD);
		m_resD = 0;
	}

	unsigned long ids[30];
	int count = 0;
	int total = CRealMyRoom_AvatarManager::Instance()->m_avatarCount;
	for (int i = 0; i < 30; i++)
	{
		if (count >= total)
			break;
		if (CRealMyRoom_AvatarManager::Instance()->m_avatars[i])
		{
			ids[count] =
				CRealMyRoom_AvatarManager::Instance()->m_avatars[i]->m_id;
			count++;
		}
	}

	for (int i = 0; i < count; i++)
	{
		this << MsgObject(this, M983(0xe5), ids[i], 0, 0, 0, 0);
	}

	g_pFresh->GetManager()->SetExclusiveKey(true);
}

void CRealMyRoomMain::OnProcess(float dt)
{
	static WVector s_vFront = WVector::UNIT_POS_Z;
	static WVector s_vRight = WVector::UNIT_POS_X;
	static bool s_bKeyMove;
	static bool s_bMoving;
	static bool s_bDragging;
	static bool s_bRotating;

	CTaskMain::OnProcess(dt);
	GetPVS().PreProcess(dt, false);

	if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar == NULL)
		return;

	WVector target;
	if (MYROOM_VIEW_FOCUSED())
	{
		int left = g_input->GetButton(LEFT_BUTTON);
		int right = g_input->GetButton(RIGHT_BUTTON);
		int middle = g_input->GetButton(MIDDLE_BUTTON);
		WVector delta = g_input->GetMouseDelta();
		WVector mouse = g_input->GetMousePoint();
		WVector start, ray;
		g_view->GetScreen2World(mouse.x, mouse.y, start, ray);

		if (m_p210 &&
			!CMouseCursor::Instance()->InArea(MYROOM_POPUP_RECT(m_p210)) &&
			(left == 1 || right == 1))
			MYROOM_POPUP_HIDE(m_p210);

		float zoomRate = m_topViewDist * (1.0f / 960.0f);
		if (zoomRate <= 0.1f)
			zoomRate = 0.1f;

		if (!s_bKeyMove)
		{
			s_vRight = g_view->GetCamera().xa;
			s_vRight = s_vRight.Normalize();
			WVector front(WVector::UNIT_POS_Y.z * s_vRight.y -
					s_vRight.z * WVector::UNIT_POS_Y.y,
				s_vRight.z * WVector::UNIT_POS_Y.x -
					WVector::UNIT_POS_Y.z * s_vRight.x,
				WVector::UNIT_POS_Y.y * s_vRight.x -
					s_vRight.y * WVector::UNIT_POS_Y.x);
			s_vFront = front;
			s_vFront = s_vFront.Normalize();
		}

		if (m_p3c0 && CMouseCursor::Instance()->GetArea() &&
			MYROOM_POPUP_VISIBLE(m_p3c0))
			*((unsigned char*)m_p3c0 + 0x30) &= 0x7f;

		if (CMouseCursor::Instance()->GetArea() == 0)
		{
			if (!m_bTopView)
			{
				CRealMyRoom_AvatarManager::Instance()->m_pRayAvatar =
					CRealMyRoom_AvatarManager::Instance()->AvatarByRayIntersect(
						start, ray);
				CRealMyRoom_AvatarManager::Instance()->m_pClickAvatar =
					CRealMyRoom_AvatarManager::Instance()->CheckClick(mouse.x,
						mouse.y);
				CRealMyRoomData::Instance()->m_pOBBModel =
					CRealMyRoomData::Instance()->ModelByRayIntersectOBB(
						g_camera.pivot);
				CMouseCursor::Instance()->SetCursor(
					CRealMyRoom_AvatarManager::Instance()->m_pRayAvatar ||
							CRealMyRoom_AvatarManager::Instance()
								->m_pClickAvatar
						? 1
						: 0);
			}

			if (m_p210 && MYROOM_POPUP_VISIBLE(m_p210))
			{
				WRect rect(MYROOM_POPUP_RECT(m_p210));
				if (rect.IsInRect(mouse.x, mouse.y))
					goto KEYBOARD;
			}

			int index;
			if (left == 1)
			{
				if (m_bTopView ||
					!CRealMyRoom_AvatarManager::Instance()->CheckClick(mouse.x,
						mouse.y))
				{
					target = GetPVS().GetRayIntersection(start, ray);
					target =
						GetPVS().GetGroundPoint(target, true, &index, true);
					WVector wall = ((CQuadTreeWall&)GetPVS())
									   .GetRayIntersectionWall(start, ray);
					GetPVS().DrawDecal(m_glowBallTex, target,
						WVector(4.8f, 0.0f, 0.0f), WVector(0.0f, 0.0f, 4.8f),
						0.5f, 1.0f, (enumPolyBlend)0x800000, false);

					if (CRealMyRoomData::Instance()->m_pSelModel &&
						!MYROOM_IS_WALL_ITEM(
							CRealMyRoomData::Instance()->m_pSelModel))
						CRealMyRoomData::Instance()->SetTargetPosition(wall, 1);

					if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar &&
						!CRealMyRoom_AvatarManager::Instance()
							->m_pCurAvatar->IsHoldState())
					{
						GetPVS().GetGroundPoint(target, true, &index, true);
						if (index >= 0 &&
							CRealMyRoomData::Instance()->m_pSelModel == NULL)
						{
							s_bMoving = true;
							if (((sPolyPoint*)m_fc6c)->pos.z < target.z)
							{
								CRealMyRoom_AvatarManager::Instance()
									->m_pCurAvatar->SetTargetPosition(target);
								CRealMyRoom_AvatarManager::Instance()
									->m_pCurAvatar->SetAction(0);
								CRealMyRoom_AvatarManager::Instance()
									->m_pCurAvatar->Follow(NULL);
							}
						}
					}

					if (GetFlags(8) &&
						(CRealMyRoomData::Instance()->GetState() == 1 ||
							CRealMyRoomData::Instance()->GetState() == 3) &&
						(CRealMyRoomData::Instance()->m_pSelModel == NULL ||
							!MYROOM_IS_WALL_ITEM(
								CRealMyRoomData::Instance()->m_pSelModel)))
						CRealMyRoomData::Instance()->m_pSelModel =
							CRealMyRoomData::Instance()->ModelByRayIntersect(
								start, ray);
				}
			}
			else if (left == 3)
			{
				if (CRealMyRoomData::Instance()->GetState() == 2)
				{
					s_bDragging = true;
					target = GetPVS().GetRayIntersection(start, ray);
					target =
						GetPVS().GetGroundPoint(target, true, &index, true);
					WVector wall = ((CQuadTreeWall&)GetPVS())
									   .GetRayIntersectionWall(start, ray);
					Waabb aabb(WVector(0.0f, 0.0f, 0.0f),
						WVector(0.0f, 0.0f, 0.0f));
					((CQuadTreeWall&)GetPVS()).GetAABB(aabb);
					if (CRealMyRoomData::Instance()->m_pSelModel && GetFlags(8))
						CRealMyRoomData::Instance()->SetTargetPosition(wall, 3);
					CMouseCursor::Instance()->SetMode(0);
				}
			}
			else if (left == 2)
			{
				if (CRealMyRoomData::Instance()->GetState() == 2 && s_bDragging)
				{
					target = GetPVS().GetRayIntersection(start, ray);
					target =
						GetPVS().GetGroundPoint(target, true, &index, true);
					WVector wall = ((CQuadTreeWall&)GetPVS())
									   .GetRayIntersectionWall(start, ray);
					if (CRealMyRoomData::Instance()->m_pSelModel)
						CRealMyRoomData::Instance()->SetTargetPosition(wall, 2);
					CRealMyRoomData::Instance()->m_pSelModel = NULL;
					CMouseCursor::Instance()->SetMode(2);
					s_bDragging = false;
					CRealMyRoomData::Instance()->SetState(0);
				}
			}
			else
			{
				if (CRealMyRoomData::Instance()->GetState() == 2 &&
					CRealMyRoomData::Instance()->m_pSelModel &&
					MYROOM_IS_WALL_ITEM(
						CRealMyRoomData::Instance()->m_pSelModel))
				{
					s_bDragging = true;
					target = GetPVS().GetRayIntersection(start, ray);
					target =
						GetPVS().GetGroundPoint(target, true, &index, true);
					WVector wall = ((CQuadTreeWall&)GetPVS())
									   .GetRayIntersectionWall(start, ray);
					Waabb aabb(WVector(0.0f, 0.0f, 0.0f),
						WVector(0.0f, 0.0f, 0.0f));
					((CQuadTreeWall&)GetPVS()).GetAABB(aabb);
					if (CRealMyRoomData::Instance()->m_pSelModel && GetFlags(8))
						CRealMyRoomData::Instance()->SetTargetPosition(wall, 3);
					CMouseCursor::Instance()->SetMode(0);
				}
			}

			if (right == 1)
			{
				if (!m_bTopView &&
					(GetFlags(8) || GetFlags(2) ||
						g_pFresh->IsCurrentLayout("REALMYROOMVISITOR")))
					CRealMyRoomData::Instance()->m_pSelModel =
						CRealMyRoomData::Instance()->ModelByRayIntersect(start,
							ray);
				s_bRotating = false;
			}
			else if (right == 3)
			{
				if (!m_bTopView)
				{
					static WVector s_prevDelta(0.0f, 0.0f, 0.0f);
					m_v214.y -= (delta.y + s_prevDelta.y) * 0.5f * 0.008f;
					s_prevDelta = delta;
					if (m_f284 == 0)
					{
						if (m_v214.x < -1.5707964f)
							m_v214.x = -1.5707964f;
						if (m_v214.x > 0.78539819f)
							m_v214.x = 0.78539819f;
					}
					else
					{
						if (m_v214.x < m_f27c * g_DEGTORAD)
							m_v214.x = m_f27c * g_DEGTORAD;
						if (m_v214.x > m_f280 * g_DEGTORAD)
							m_v214.x = m_f280 * g_DEGTORAD;
						if (m_v214.y < m_f274 * g_DEGTORAD)
							m_v214.y = m_f274 * g_DEGTORAD;
						if (m_v214.y > m_f278 * g_DEGTORAD)
							m_v214.y = m_f278 * g_DEGTORAD;
					}
					*(WVector*)m_pad24c = m_v214;
					m_f244 = 0.0f;
				}
				if (!WisZero(delta.x, g_EPSILON) ||
					!WisZero(delta.y, g_EPSILON))
					s_bRotating = true;
				g_input->FixPointer(true);
				CMouseCursor::Instance()->Toggle(false);
			}
			else if (right == 2)
			{
				if (CRealMyRoomData::Instance()->m_pSelModel)
				{
					int x = (int)g_input->GetMousePoint().x;
					PopupRightMenu(x, (int)g_input->GetMousePoint().y);
				}
				g_input->FixPointer(false);
				CMouseCursor::Instance()->Toggle(true);
			}
			else
			{
				g_input->FixPointer(false);
				CMouseCursor::Instance()->Toggle(true);
			}

			if (g_input->GetButton(LEFT_BUTTON) &&
				g_input->GetButton(RIGHT_BUTTON) && !m_bTopView &&
				!CChatMsg::Instance()->IsOpenning() &&
				CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
			{
				target =
					CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos;
				target.x += s_vFront.x * 4.1f;
				target.y += s_vFront.y * 4.1f;
				target.z += s_vFront.z * 4.1f;
				CRealMyRoom_Avatar* avatar =
					CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar;
				if (!WisEqual(target, avatar->m_vPos, 1e-05f) &&
					!avatar->IsHoldState())
				{
					s_bMoving = false;
					avatar->SetTargetPosition(target);
					CRealMyRoom_AvatarManager::Instance()
						->m_pCurAvatar->SetAction(0);
					CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->Follow(
						NULL);
				}
			}

			if (middle == 1)
				ToggleTopViewMode();

			if (g_pFresh->GetManager()->GetWheelFocus() == NULL)
			{
				if (CRealMyRoomData::Instance()->GetState() == 3)
					CRealMyRoomData::Instance()->SetRotate(delta.z);
				else if (!m_bTopView)
					m_fovTarget = Between(33.28f,
						m_fovTarget -
							delta.z * g_input->GetSensitivity() * 0.021f,
						60.8f);
				else
					m_topViewDist = Between(51.2f,
						m_topViewDist -
							delta.z * g_input->GetSensitivity() * zoomRate *
								0.4f,
						80.0f);
			}
		}

KEYBOARD:
		if (!g_pFresh->GetManager()->IsKeyExclusive())
		{
			if ((g_input->GetDown("ENTER", true) ||
					g_input->GetDown("PADENTER", true)) &&
				CChatMsg::Instance()->IsClosed())
				((CChatMsgEx*)CChatMsg::Instance())->Open(true, true);

			bool bShift =
				g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true);
			bool bControl = g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true);

			if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
				target =
					CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos;

			if (CChatMsg::Instance()->IsOpenning())
			{
				if (g_input->Get("HOME", true) ||
					(bShift && g_input->Get("\xbb\xf3", true)))
					m_topViewDist = Between(25.6f,
						m_topViewDist - zoomRate * dt * 1200.0f, 960.0f);
				if (g_input->Get("END", true) ||
					(bShift && g_input->Get("\xc7\xcf", true)))
					m_topViewDist = Between(25.6f,
						m_topViewDist + zoomRate * dt * 1200.0f, 960.0f);
			}
			else
			{
				if (bControl)
				{
					if (!m_bTopView)
					{
						float yaw = 0.0f;
						float pitch = 0.0f;
						if (g_input->Get("\xbb\xf3", true))
							pitch = dt * -0.52359879f;
						if (g_input->Get("\xc7\xcf", true))
							pitch += dt * 0.52359879f;
						if (g_input->Get("\xc1\xc2", true))
							yaw = dt * -0.52359879f;
						if (g_input->Get("\xbf\xec", true))
							yaw += dt * 0.52359879f;
						if (!WisZero(yaw, g_EPSILON) ||
							!WisZero(pitch, g_EPSILON))
						{
							m_v214.y -= yaw;
							m_v214.x -= pitch;
							if (m_v214.x < -1.5707964f)
								m_v214.x = -1.5707964f;
							if (m_v214.x > 0.78539819f)
								m_v214.x = 0.78539819f;
							*(WVector*)m_pad24c = m_v214;
							m_f244 = 0.0f;
						}
					}
					else
					{
						if (g_input->Get("\xbb\xf3", true) &&
							(m_vTopView.z += s_vFront.z) > 3.0f)
							m_vTopView.z = 3.0f;
						if (g_input->Get("\xc7\xcf", true) &&
							(m_vTopView.z += -s_vFront.z) < -3.0f)
							m_vTopView.z = -3.0f;
						if (g_input->Get("\xc1\xc2", true) &&
							(m_vTopView.x += -s_vRight.x) < -3.0f)
							m_vTopView.x = -3.0f;
						if (g_input->Get("\xbf\xec", true) &&
							(m_vTopView.x += s_vRight.x) > 3.0f)
							m_vTopView.x = 3.0f;
					}
				}
				else
				{
					if (g_input->Get("\xbb\xf3", true))
						target.z += s_vFront.z * 4.1f;
					if (g_input->Get("\xc7\xcf", true))
						target.z += s_vFront.z * -4.1f;
					if (g_input->Get("\xc1\xc2", true))
						target.x += s_vRight.x * -4.1f;
					if (g_input->Get("\xbf\xec", true))
						target.x += s_vRight.x * 4.1f;

					if (target.z <= -8.0f)
						target.z = -8.0f;
					else
					{
						int index;
						target =
							GetPVS().GetGroundPoint(target, true, &index, true);
						if (index < 0 &&
							CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
							target = CRealMyRoom_AvatarManager::Instance()
										 ->m_pCurAvatar->m_vPos;
					}
				}

				if (g_input->Get("HOME", true))
					m_fovTarget =
						Between(33.28f, m_fovTarget - dt * 16.8f, 60.8f);
				if (g_input->Get("END", true))
					m_fovTarget =
						Between(33.28f, m_fovTarget + dt * 16.8f, 60.8f);

				CRealMyRoom_Avatar* avatar;
				if (g_input->GetDown("PAGE_DOWN", true) &&
					(avatar = CRealMyRoom_AvatarManager::Instance()
							->m_pCurAvatar) != NULL &&
					!avatar->IsHoldState() && avatar->GetAction() != 2)
				{
					if (avatar->GetAction() == 1)
						avatar->SetAction(2);
					else
					{
						avatar->Stop();
						CRealMyRoom_AvatarManager::Instance()
							->m_pCurAvatar->SetAction(1);
					}
				}
				if (g_input->GetDown("PAGE_UP", true) &&
					(avatar = CRealMyRoom_AvatarManager::Instance()
							->m_pCurAvatar) != NULL &&
					!avatar->IsHoldState())
				{
					if (avatar->GetAction() == 1)
						avatar->SetAction(0);
					else if (avatar->GetAction() == 2)
						avatar->SetAction(1);
				}
			}

			if (g_input->GetDown("PAD_0", true) ||
				g_input->GetDown("NUM_0", true))
				ToggleTopViewMode();

			CRealMyRoom_Avatar* avatar =
				CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar;
			if (avatar == NULL || WisEqual(target, avatar->m_vPos, 1e-05f) ||
				avatar->IsHoldState())
				s_bKeyMove = false;
			else
			{
				s_bMoving = false;
				s_bKeyMove = true;
				if (((sPolyPoint*)m_fc6c)->pos.z < target.z)
				{
					avatar->SetTargetPosition(target);
					CRealMyRoom_AvatarManager::Instance()
						->m_pCurAvatar->SetAction(0);
					CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->Follow(
						NULL);
				}
			}
		}
	}
	else
	{
		if (MYROOM_IS_OWNER() && MYROOM_POPUP_VISIBLE(m_p210))
			MYROOM_POPUP_HIDE(m_p210);
		g_input->FixPointer(false);
		CMouseCursor::Instance()->Toggle(true);
	}

	CRealMyRoom_AvatarManager::Instance()->Process(dt);
	ProcessCamera(dt);
	CRealMyRoomData::Instance()->Process(dt);
	CTransferableItem::Instance()->Process(dt);

	if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
	{
		if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->GetMotion() ==
			1)
			m_f244 += dt;
		if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->GetMotion() ==
				2 ||
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->GetMotion() ==
				3)
			m_f240 = 1.9f;
		else
			m_f240 = 4.6f;

		float dx =
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPosSelf.x -
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos.x;
		if (dx < 1e-05f && dx > -1e-05f &&
			IsEqual(CRealMyRoom_AvatarManager::Instance()
						->m_pCurAvatar->m_vPosSelf.z,
				CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos.z))
			s_bMoving = false;
	}
	m_b209 = s_bMoving;

	if (m_hand.IsWorking(0))
		Sleep(10);
	else if (m_pExhibition)
	{
		m_hand.Enter();
		m_pExhibition->Process(dt, m_roomMode != 1 && m_roomMode != 4);
		m_hand.Leave();
	}

	if (g_input->GetDown("ESCAPE", true) == 1 &&
		!g_pFresh->GetManager()->HasEscKeyWindow())
		CMessengerInfo::Instance()->Close(false);

	if (m_roomMode == 4)
	{
		for (int i = 0; i < 2; i++)
		{
			float speed = i == 0 ? 138.0f : 228.0f;
			if (m_scrollDir[i] == 0)
				m_scrollPos[i] = 0.0f;
			else if (m_scrollDir[i] == 1)
				m_scrollPos[i] -= speed * dt * 0.25f;
			else if (m_scrollDir[i] == 2)
				m_scrollPos[i] += speed * dt * 0.25f;
		}
	}

	switch (CIconToolTip::GetInstance()->GetCurTipIndex())
	{
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x27:
	case 0x3e:
		OnBarHoverOff(0);
		break;
	}

	bool bBuffHover = false;
	SendMsg(this, "ItemBuff", M983(0x246), 2, (int)&bBuffHover, 0, 0);
	if (bBuffHover)
		OnBarHoverOff(0);

	if (IsLocalContent(S4_UCC))
		RefreshAllUccClothes();

	if (MYROOM_IS_OWNER())
	{
		if ((g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true)) &&
			g_input->GetDown("HIDE", true) && !GetFlags(8))
			SelectMenu(0);
		if ((g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true)) &&
			g_input->GetDown("FURNITURE_UI", true))
			SelectMenu(2);
	}

	g_input->GetDown("ENTER", true);
}

void CRealMyRoomMain::Display()
{
	if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar == NULL)
		return;

	CRealMyRoom_AvatarManager::Instance()->Render();
	CRealMyRoomData::Instance()->Render();
	CTransferableItem::Instance()->Render();

	if (m_b209)
	{
		WVector dirX;
		WVector dirZ;
		dirX = WVector(4.8f, 0.0f, 0.0f);
		dirZ = WVector(0.0f, 0.0f, 4.8f);
		CRealMyRoom_Avatar* pAvatar =
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar;
		if (pAvatar->m_nick[0] != '\0')
		{
			pAvatar =
				CRealMyRoom_AvatarManager::Instance()->FindAvatarByNickName(
					pAvatar->m_nick);
			if (pAvatar == NULL)
			{
				DrawMouseCursor();
				return;
			}
			float scale = (cosf(m_time270 * 10.0f) + 1.0f) * 0.05f + 1.0f;
			GetPVS().DrawDecal(m_glowBallTex, pAvatar->m_vPos, dirX, dirZ,
				scale, 1.0f, (enumPolyBlend)0x800000, false);
		}
		else
		{
			float scale = (cosf(m_time270 * 10.0f) + 1.0f) * 0.05f + 1.0f;
			GetPVS().DrawDecal(m_glowBallTex, pAvatar->m_vPosSelf, dirX, dirZ,
				scale, 1.0f, (enumPolyBlend)0x800000, false);
		}
	}

	DrawMouseCursor();
}

void CRealMyRoomMain::DrawMouseCursor()
{
	int cursor = CMouseCursor::Instance()->GetCursor();
	static int s_prevCursor = cursor;

	if (m_p210 && (*(unsigned char*)((char*)m_p210 + 0x30) & 1))
	{
		CMouseCursor::Instance()->SetCursor(s_prevCursor);
		return;
	}
	if (CRealMyRoomData::Instance()->GetState() == 2 &&
		CRealMyRoomData::Instance()->m_pSelModel)
	{
		if (CRealMyRoomData::Instance()->m_bModelDragging)
			CMouseCursor::Instance()->SetCursor(CMouseCursor::RM_MOVE_NO);
		else
			CMouseCursor::Instance()->SetCursor(CMouseCursor::RM_MOVE);
		return;
	}
	if (CRealMyRoomData::Instance()->GetState() == 3 &&
		CRealMyRoomData::Instance()->m_pSelModel)
	{
		if (CRealMyRoomData::Instance()->m_bModelDragging)
			CMouseCursor::Instance()->SetCursor(CMouseCursor::RM_ROTATE_NO);
		else
			CMouseCursor::Instance()->SetCursor(CMouseCursor::RM_ROTATE);
		return;
	}
	if (cursor != s_prevCursor)
		CMouseCursor::Instance()->SetCursor(s_prevCursor);
	s_prevCursor = CMouseCursor::Instance()->GetCursor();
}

void CRealMyRoomMain::OnDisplay()
{
	CRealMyRoom_AvatarManager::Instance()->RenderOverlay();
	g_view->DrawLine(m_vLineA0, 0xffffffff, m_vLineA1, 0xffffffff, 0);
	g_view->DrawLine(m_vLineB0, 0xffffffff, m_vLineB1, 0xffffffff, 0);
}

void CRealMyRoomMain::LoadBg(const char* name, int flag)
{
	m_bgName = name;
	GOLFDOC()->m_currentHole = flag;

	if (CFx::Instance())
		CFx::Instance()->SetActive(false);

	if (!GOLFDOC()->m_pPolySoup->Load("myroom_01_01.gbin", false))
	{
		g_bQuit = true;
		return;
	}

	GOLFDOC()->m_waveInfo.Load(MakeStr("%s_wave.txt", name));
	GOLFDOC()->m_riverInfo.Load(MakeStr("%s_river.txt", name));

	GOLFDOC()->m_pSky = new CSky;
	GOLFDOC()->m_pSky->LoadSky(name, 0.0f);

	GOLFDOC()->m_pPolySoup->MakeDataStructure();

	if (GOLFDOC()->m_pSky)
	{
		GOLFDOC()->m_pSky->GenerateStars(-1);
		GOLFDOC()->m_pSky->GenerateLensFlare();
		GOLFDOC()->m_pSky->GenerateMoons();
	}

	CShadowManager::Instance()->UpdateLight("sun", g_lightset);

	CSceneManager::Instance()->Load();

	GOLFDOC()->m_pPolySoup->EndLoad(false);

	CTransferableItem::Instance()->OnLoad();

	if (CFx::Instance())
		CFx::Instance()->SetActive(true);
}

void CRealMyRoomMain::CheckUIPosition()
{
	static float s_barY = COption::Instance()->gGetBar_Y();

	if (s_barY != COption::Instance()->gGetBar_Y())
	{
		CalcUIOffset(false);
		s_barY = COption::Instance()->gGetBar_Y();
		g_pFresh->OpenLayout("REALMYROOM", this, false);
	}
}

void CRealMyRoomMain::ProcessCamera(float dt)
{
	static int s_camIdx = 1;
	WMatrix mat;
	WVector at;
	float limit = 8.0f;

	g_view->SetClip(1.0f, m_clipFar, false);

	switch (m_fce8)
	{
	case 0:
	{
		mat.Reset();
		mat = g_camera;
		if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
			at = CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos;
		WVector step;
		step = at - m_vCamLook;
		at.y = g_camera.pivot.y;
		float speed = dt * 4.0f;
		m_vCamLook.y = at.y;
		step.x *= speed;
		step.z *= speed;
		float dist = (at - m_vCamLook).Magnitude();
		float dz = Wabs(g_camera.pivot.z - at.z);
		if (dz > 8.0f)
			limit = dz;
		if (dist == 0.0f)
		{
			m_bCamFollow = false;
		}
		else if (dist > 3.0f / limit || m_bCamFollow == true)
		{
			m_bCamFollow = true;
			m_vCamLook.x += step.x;
			m_vCamLook.z += step.z;
		}
		m_vCamLook.y = g_camera.pivot.y;
		if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar &&
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_bOnStair &&
			at.z < m_v2c8.z)
		{
			g_camera.pivot.y = m_matCam.pivot.y - Wabs(at.z - m_v2c8.z);
			m_vCamLook.y = 8.0f;
		}
		if (!WisEqual(m_fovCur, m_fovTarget, 1e-5f))
		{
			float delta = dt * 46.0f;
			if (m_fovTarget > m_fovCur)
			{
				m_fovCur += delta;
				if (m_fovCur > m_fovTarget)
					m_fovCur = m_fovTarget;
			}
			else
			{
				m_fovCur -= delta;
				if (m_fovCur < m_fovTarget)
					m_fovCur = m_fovTarget;
			}
		}
		g_view->SetFOV(m_fovCur * g_DEGTORAD);
		if (m_v214.y < -25.0f)
			m_v214.y = -25.0f;
		else if (m_v214.y > 23.0f)
			m_v214.y = 23.0f;
		m_vCamLook.y += m_v214.y;
		g_camera = MakeCameraMatrix(g_camera.pivot, m_vCamLook);
		g_view->SetCamera(g_camera);
	}
	break;

	case 1:
		g_view->SetCamera(g_camera);
		break;

	case 2:
	{
		WVector dir = m_vCamTarget - g_camera.pivot;
		at = WVector(0.0f, 0.0f, 5.0f);
		if (dir.Magnitude() < 1.0f)
		{
			s_camIdx++;
			if (s_camIdx == 4)
			{
				m_vCamTarget = m_pCamList[0]->m_mat.pivot;
				g_view->UpdateCamera();
				return;
			}
			if (s_camIdx != 5)
			{
				m_vCamTarget = m_pCamList[s_camIdx]->m_mat.pivot;
				g_view->UpdateCamera();
				return;
			}
			SetCameraDisplayType(0);
			g_view->SetClip(m_clipNear, m_clipFar, false);
			s_camIdx = 1;
			g_view->UpdateCamera();
			return;
		}
		CalculationCameraMat(mat, at, dir, g_camera.pivot, m_fCamFov,
			g_view->GetFOV(),
			dt * 8.0f);
		g_camera = mat;
		g_view->SetCamera(g_camera);
	}
	break;

	case 3:
	{
		mat.Reset();
		g_view->SetClip(8.0f, 8192.0f, false);
		g_view->SetFOV(m_topViewDist * g_DEGTORAD);
		mat = RotMat(WVector(-1.5707964f, 0.0f, 0.0f));
		at = m_vTopView;
		WVector eye = m_matCam.pivot;
		eye.y = 25.0f;
		eye.z += 5.0f;
		mat = MakeCameraMatrix(eye, at);
		g_camera = mat;
		g_view->SetCamera(mat);
	}
	break;

	case 4:
	{
		if (CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar)
			at = CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar->m_vPos;
		at.y = m_vCamTarget.y;
		WVector dir = m_vCamTarget - g_camera.pivot;
		float fov =
			g_view->GetFOV();
		if (dir.Magnitude() < 1.0f)
		{
			if (m_fCamFov == fov)
			{
				SetCameraDisplayType(0);
				return;
			}
			CalculationCameraMat(mat, at, WVector::ZERO, g_camera.pivot,
				m_fCamFov, fov, dt * 5.0f);
		}
		else
		{
			// the original passes the current fov as the target too
			CalculationCameraMat(mat, at, dir, g_camera.pivot, fov, fov,
				dt * 5.0f);
		}
		g_camera = mat;
		g_view->SetCamera(g_camera);
	}
	break;

	case 5:
	{
		g_view->SetFOV(m_topViewDist);
		WVector dir = m_vCamTarget - g_camera.pivot;
		at = WVector(0.0f, 0.0f, 5.0f);
		if (dir.Magnitude() < 1.0f)
		{
			mat = MakeCameraMatrix(g_camera.pivot, m_vTopView);
		}
		else
		{
			float fov =
				g_view->GetFOV();
			CalculationCameraMat(mat, at, dir, g_camera.pivot, fov, fov,
				dt * 8.0f);
		}
		g_camera = mat;
		g_view->SetCamera(g_camera);
	}
	break;
	}

	g_view->UpdateCamera();
}

void CRealMyRoomMain::CalculationCameraMat(WMatrix& mat, WVector at,
	WVector dir, WVector eye, float fovTarget, float fovCur, float step)
{
	mat = g_camera;
	float fov = fovCur;
	dir.Normalize();
	eye += WVector(dir.x * step, dir.y * step, dir.z * step);
	float diff = fovCur - fovTarget;
	// HACK: the compiler folds a 1e-5f literal into g_EPSILON's storage; the original kept the literal
	if (!(Wabs(diff) < 1e-5f))
	{
		float delta = step * 46.0f;
		if (fovTarget > fovCur)
		{
			fov = delta + fovCur;
			if (fov > fovTarget)
				fov = fovTarget;
		}
		else
		{
			fov = fovCur - delta;
			if (fov < fovTarget)
				fov = fovTarget;
		}
	}
	g_view->SetFOV(fov);
	mat = MakeCameraMatrix(eye, at);
}

void CRealMyRoomMain::ToggleTopViewMode()
{
	if (!GetFlags(1))
	{
		m_bTopView = !m_bTopView;
		if (m_bTopView)
		{
			GOLFDOC()->m_gameMode = 0x200;
			m_topViewDist = 70.0f;
			m_vTopView = WVector::ZERO;
			m_fce8 = 3;
		}
		else
		{
			GOLFDOC()->m_gameMode = 0;
			SetCameraDisplayType(0);
		}
	}
}

void CRealMyRoomMain::SetCameraDisplayType(int type)
{
	static int s_camSel;
	if (type == 0)
	{
		if (m_fce8 != 4)
			g_camera = m_matCam;
		g_view->SetFOV(m_fovCam);
		m_fce8 = type;
		return;
	}

	{
		switch (type)
		{
		case 1:
			g_camera = m_pCamMain->m_mat;
			g_view->SetFOV(m_pCamMain->m_fov);
			m_fce8 = type;
			return;

		case 2:
			g_camera = m_pCamList[0]->m_mat;
			g_view->SetFOV(0.7330383f);
			m_vCamTarget = m_pCamList[1]->m_mat.pivot;
			m_fce8 = type;
			m_fCamFov = 0.7330383f;
			return;

		case 3:
			m_topViewDist = 70.0f;
			m_vTopView = WVector::ZERO;
			m_fce8 = type;
			return;

		case 4:
			if (m_pCamSel == NULL)
			{
				g_camera = m_matCam;
				g_view->SetFOV(m_fovCam);
			}
			else
			{
				g_camera = m_pCamSel->m_mat;
				g_view->SetFOV(m_pCamMain->m_fov);
			}
			m_vCamTarget = m_matCam.pivot;
			m_fce8 = type;
			m_fCamFov = m_fovCam;
			return;

		case 5:
			if (m_pCamList[s_camSel] == NULL)
				g_camera = m_matCam;
			else
				g_camera = m_pCamList[s_camSel]->m_mat;
			g_view->SetFOV(70.0f);
			m_topViewDist = 70.0f;
			m_vTopView = WVector(0.0f, 0.0f, 5.0f);
			s_camSel++;
			if (s_camSel < 4)
			{
				if (m_pCamList[s_camSel] == NULL)
					m_vCamTarget = m_matCam.pivot;
				else
					m_vCamTarget = m_pCamList[s_camSel]->m_mat.pivot;
				m_fce8 = type;
				return;
			}
			s_camSel = 0;
			SetCameraDisplayType(4);
			return;

		default:
			m_fce8 = type;
			return;
		}
	}
}

void CRealMyRoomMain::RefreshAllUccClothes()
{
	sUccRefreshItem item;

	for (int i = 0; i < 10; i++)
	{
		if (!UccManager()->GetRefreshItem(item))
			return;

		if (m_pExhibition && m_pExhibition->GetPetFrame())
		{
			CPartTidList& partList = CSharedDoc::Instance()->GetMyPartTidList();
			unsigned char index = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (partList.m_tid[index] == item.typeId)
			{
				sUccClothes* pClothes =
					UccManager()->FindClothes(partList.m_uid[index]);
				if (pClothes && !strcmpi(pClothes->uccIndex, item.uccIndex))
				{
					unsigned long id = pClothes->id;
					unsigned long typeId = pClothes->typeId;
					CPetFrame* pFrame = m_pExhibition->GetPetFrame();
					pFrame->SetPart(&CSharedDoc::Instance()->GetMyPartTidList(),
						typeId, id);
					id = pClothes->id;
					typeId = pClothes->typeId;
					pFrame = m_pExhibition->GetPetFrame();
					pFrame->SetPart(&CSharedDoc::Instance()->GetMyPartTidList(),
						typeId, id);
				}
			}
		}

		CRealMyRoom_Avatar* pCur =
			CRealMyRoom_AvatarManager::Instance()->m_pCurAvatar;
		if (pCur)
		{
			unsigned char index = (unsigned char)((item.typeId >> 13) & 0x1f);
			if (pCur->m_tid[index] == item.typeId)
			{
				sUccClothes* pClothes =
					UccManager()->FindClothes(pCur->m_uid[index]);
				if (pClothes && !strcmpi(pClothes->uccIndex, item.uccIndex))
				{
					CRealMyRoom_AvatarManager::Instance()
						->m_pCurAvatar->RefreshUccClothes(pClothes->typeId,
							pClothes->id);
				}
			}
		}

		for (int j = 0;
			j < CRealMyRoom_AvatarManager::Instance()->m_avatarCount; j++)
		{
			CRealMyRoom_Avatar* pAvatar =
				CRealMyRoom_AvatarManager::Instance()->m_avatars[j];
			if (pAvatar)
			{
				unsigned char index =
					(unsigned char)((item.typeId >> 13) & 0x1f);
				if (pAvatar->m_tid[index] == item.typeId)
				{
					sUccClothes* pClothes =
						UccManager()->FindClothes(pAvatar->m_uid[index]);
					if (pClothes && !strcmpi(pClothes->uccIndex, item.uccIndex))
					{
						pAvatar->RefreshUccClothes(pClothes->typeId,
							pClothes->id);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// JP 983 (frente C, pacotes): envios da oficina de tacos (ClubSetWork*). No 983 ficam nesta unidade, 004ce120-004ce4b0;
// declaracao e quem chama em include/packets/clubsetworkpackets.h.
#include "packet.h"
#include "packets/clubsetworkpackets.h"	// PKT (frente C)

// 004ce120
void __fastcall SendClubSetLevelUp983(unsigned long a, unsigned long b, unsigned short c)
{
	WSendPacket packet(0x164);
	packet.Encode4(b);
	packet.Encode2(c);
	packet.Encode4(a);
	packet.Send(TO_GAME);
}

// 004ce1b0
void __fastcall SendClubSetWork165_983(bool bSim)
{
	WSendPacket packet(0x165 + (bSim == false));
	packet.Send(TO_GAME);
}

// 004ce220
void __fastcall SendClubSetRankUp983(unsigned long a, unsigned long b, unsigned short c)
{
	WSendPacket packet(0x167);
	packet.Encode4(b);
	packet.Encode2(c);
	packet.Encode4(a);
	packet.Send(TO_GAME);
}

// 004ce2b0
void __fastcall SendClubSetWork168_983(bool bSim)
{
	WSendPacket packet(0x168 + (bSim == false));
	packet.Send(TO_GAME);
}

// 004ce320
void __fastcall SendClubSetWork16b_983(unsigned long a, unsigned long b)
{
	WSendPacket packet(0x16b);
	packet.Encode4(b);
	packet.Encode4(a);
	packet.Send(TO_GAME);
}

// 004ce3a0
void __fastcall SendClubSetWork16c_983(unsigned long a, unsigned long b, unsigned long typeId, unsigned long d)
{
	WSendPacket packet(0x16c);
	packet.Encode4(typeId);
	packet.Encode4(a);
	packet.Encode4(b);
	packet.Encode4(d);
	packet.Send(TO_GAME);
}

// 004ce440
void __fastcall SendClubSetRandomBox983(unsigned long a, unsigned long b)
{
	WSendPacket packet(0x16d);
	packet.Encode4(b);
	packet.Encode4(a);
	packet.Send(TO_GAME);
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?CreateAvatar@CRealMyRoom_AvatarManager@@QAEPAVCRealMyRoom_Avatar@@PAUsSlotInfo@@PAUsCharacterInfo@@PAVCRealMyRoomMain@@@Z=?CreateAvatar@CRealMyRoom_AvatarManager@@QAEPAVCRealMyRoom_Avatar@@ABUsSlotInfo@@ABUsCharacterInfo@@PAVCRealMyRoomMain@@@Z") // ponteiro x ref
#pragma comment(linker, "/alternatename:?FindPoint@WPolySoupEx@@QAEPAUsPolyPoint@@PBD@Z=?FindPoint@WPolySoup@@QAEPAUsPoint@1@PBD@Z") // visao derivada; struct de visao
#pragma comment(linker, "/alternatename:?SetAvatarPosition@CQuadTreeEx@@QAEXABVWVector@@M@Z=?SetAvatarPosition@CQuadTree@@QAEXABVWVector@@M@Z") // visao derivada
#pragma comment(linker, "/alternatename:?Init@CWindEx@@QAEXEEEE@Z=?Init@CWind@@QAEXEEEE@Z") // visao derivada
#pragma comment(linker, "/alternatename:?SetWind@CWindEx@@QAEXABVWVector@@@Z=?SetWind@CWind@@QAEXABVWVector@@@Z") // visao derivada
#pragma comment(linker, "/alternatename:?UpdateWind@CSceneManagerWind@@QAEXXZ=?UpdateWind@CSceneManager@@QAEXXZ") // visao derivada
#pragma comment(linker, "/alternatename:?g_pMyRoomCardWnd983@@3PAVCRFWnd983@@A=?ms_pInstance@RFCharMasteryMainDlg@@2PAV1@A") // dado, mesma VA 00e78154
#pragma comment(linker, "/alternatename:?Close@CRFWnd983@@QAEXH@Z=?Close@RFWnd@@QAEXW4RFWNDCLOSESTATE@@@Z") // s159: visao; RFCharMasteryMainDlg tem RFWnd em +0 (RFFormModalBase<RFWndEx>)
#pragma comment(linker, "/alternatename:??0CRealMyRoom_AvatarBurden@@QAE@HPAUsCharacterInfo@@PAVCPetFrame@@@Z=??0CRealMyRoom_AvatarBurden@@QAE@KABUsCharacterInfo@@PAVCPetFrame@@@Z") // s159: visao realmyroomavatar.h com o tamanho real (0x2d4); int x unsigned long, ponteiro x ref
#pragma comment(linker, "/alternatename:?m_pInstance@CCameraWork@@2PAV1@A=?m_pInstance@?$WSingleton@VCCameraWork@@@@1PAVCCameraWork@@A") // dado: a visao ccamerawork.h tinha o seu m_pInstance (sempre nulo); o real e o do WSingleton
