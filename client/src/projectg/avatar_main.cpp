#include "minatl.h"
#define REBANG_AVATAR_MAIN_CPP

inline bool WRect::IsInRect(float x, float y)
{
	if (x >= this->x && this->x + w >= x && y >= this->y && this->y + h >= y)
		return true;
	return false;
}

inline bool __fastcall IsEqual(float a, float b)
{
	float d = a - b;
	return d < 1e-05f && d > -1e-05f;
}

#include "../shared/s5/specialprize/specialprizeitem.h"
#include "mousecursor.h"
#include "frdesktop.h"
#include "avatar_main.h"
#include "inputmanager.h"
#include "clientsetting.h"
#include "jukeboxdlg.h"
#include "chatmsg.h"
#include "shareddoc.h"
#include "quadtree.h"
#include "fx.h"
#include "soundmanager.h"
#include "shadow.h"
#include "projectg.h"
#include "sea.h"
#include "sky.h"
#include "polysoup.h"
#include "commonutil.h"
#include "lobbymain.h"
#include "uccmanager.h"
#include "taskmanager.h"
#include "avatar_task.h"
#include "wind.h"
#include "messengerdlg.h"
#include "user_info.h"
#include "quitwindow.h"
#include "avataruserlistdlg.h"
#include "avatarchatlogdlg.h"
#include "../../include/engine/wfont.h"
#include "s5/utilities.h"
#include "jp/lobbyjp.h" // CUiPointer983 (00e73e4c)
#include "jp/createcharacteroption.h" // D641: FindSetMotionExact983 (0x199)
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a && offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812); // A737

// unused, but the original emitted these wmath.h constants into .rdata (their users are naked asm here)
static bool IsValidRange(float f)
{
	return f < g_PI && f < g_2_PI && f < g_DEGTORAD && f < g_HUGE &&
		f > g_EPSILON;
}


// HACK: never called; creates Waabb::operator= before GetAABB needs it (emission order)
inline void RbCopyAabb(Waabb& dst, const Waabb& src)
{
	dst = src;
}

// the root node (+4) keeps its bounding box at +0x60
inline void CQuadTree::GetAABB(Waabb& aabb)
{
	aabb = *(const Waabb*)(*(const char**)((const char*)this + 4) + 0x60);
}

// HACK: WPolySoup::FindPoint and CSceneManager::UpdateWind/UpdateFog are not declared in polysoup.h /
// scenemanager.h because both are in the PCH (a new declaration there moves code of other units);
// they are reached through these layout-free views
struct RbPolySoupPoint
{
	unsigned char m_unused0[0x24];
	WVector pos;
};
struct RbPolySoup
{
	RbPolySoupPoint* FindPoint(const char* name);
};
// chatmsg.h is in the PCH as well: CChatMsg::SetToggle is reached through this view
struct RbChatMsg : public CChatMsg
{
	void SetToggle(bool toggle) { m_bToggle = toggle; }
};
struct RbSceneManager
{
	void UpdateWind();
	void UpdateFog(bool enable);
};
// clang-format off
#pragma comment(linker, "/alternatename:?FindPoint@RbPolySoup@@QAEPAURbPolySoupPoint@@PBD@Z=?FindPoint@WPolySoup@@QAEPAUsPoint@1@PBD@Z")
#pragma comment(linker, "/alternatename:?UpdateWind@RbSceneManager@@QAEXXZ=?UpdateWind@CSceneManager@@QAEXXZ")
#pragma comment(linker, "/alternatename:?UpdateFog@RbSceneManager@@QAEX_N@Z=?UpdateFog@CSceneManager@@QAEX_N@Z")
// clang-format on

extern Fresh* g_pFresh;
extern WMatrix g_camera;
#define KEYNAME_UP "\xbb\xf3"
#define KEYNAME_DOWN "\xc7\xcf"
#define KEYNAME_LEFT "\xc1\xc2"
#define KEYNAME_RIGHT "\xbf\xec"
bool g_SALoadingState;

#define RAW(T, p, off) (*(T*)((char*)(p) + (off)))
// D908: g_input +0x60/+0xe4 do VC10 = m_bFixPointer/m_sensitivity; no cl 14.44 andam -4 (BaseObject):
// usar FixPointer()/GetSensitivity() (inline, VC10 sai igual)
int WisZero(const float& v, float e);

#define SAFE_RELEASE_TEX(h) \
	if (g_resrcmng && (h)) \
	{ \
		g_resrcmng->Release(h); \
		(h) = 0; \
	}

void SetPVSIndex(int);

namespace _util
{
	void GenerateRandomTidParts(unsigned long tid, unsigned long* parts,
		unsigned long* auxParts);
}

#define POINT_START_NAME "\xb4\xeb\xc8\xad\xb9\xe6\xbd\xc3\xc0\xdb"
#define POINT_START_NAME2 "\xbd\xc3\xc0\xdb\xc1\xa1"

WMatrix RotMat(WVector rotation)
{
	WMatrix mz(cosf(rotation.z), -sinf(rotation.z), 0.0f, sinf(rotation.z),
		cosf(rotation.z), 0.0f, 0.0f, 0.0f, 1.0f);
	WMatrix my(cosf(rotation.y), 0.0f, -sinf(rotation.y), 0.0f, 1.0f, 0.0f,
		sinf(rotation.y), 0.0f, cosf(rotation.y));
	WMatrix mx(1.0f, 0.0f, 0.0f, 0.0f, cosf(rotation.x), sinf(rotation.x), 0.0f,
		-sinf(rotation.x), cosf(rotation.x));
	return mx * my * mz;
}

// 983 007a9760: com o resultado por referência (ecx = ângulos, edx = saída). A CExhibition::Animate do 983 é um dos 9
// chamadores; os do 645 acima seguem com a forma por valor. Os sinais NÃO são os da forma 645: no 983 cada eixo é o
// RotMat(ângulo, eixo) (zy = -sin x, zx = sin y, yx = sin z); com os do 645 a luz da prévia 3D vinha de baixo (A340).
void __fastcall RotMat(const WVector& rotation, WMatrix& out)
{
	float sx = sinf(rotation.x);
	float cx = cosf(rotation.x);
	float sy = sinf(rotation.y);
	float cy = cosf(rotation.y);
	float sz = sinf(rotation.z);
	float cz = cosf(rotation.z);
	out = WMatrix(1.0f, 0.0f, 0.0f, 0.0f, cx, -sx, 0.0f, sx, cx);
	WMatrix mz(cz, sz, 0.0f, -sz, cz, 0.0f, 0.0f, 0.0f, 1.0f);
	WMatrix my(cy, 0.0f, sy, 0.0f, 1.0f, 0.0f, -sy, 0.0f, cy);
	out = out * (my * mz);
}

// HACK: the destructor is naked asm behind an alias, so the scalar deleting destructor has nothing to wait
// for; this inlined operator delete makes it wait for MakeInstance like the original (emission order)
IObject* ntAvatarChatMainMakeInstance();
inline void ntAvatarChatMain::operator delete(void* p)
{
	if (0)
		ntAvatarChatMainMakeInstance();
	::operator delete(p);
}

// MakeInstance must keep using the global operator delete in its EH cleanup
#define new ::new
IMPLEMENT_ACTOR(ntAvatarChatMain, CTaskMain)
#undef new

ntAvatarChatMain::ntAvatarChatMain()
{
	// A798: 983 007ac230/007af600: m_pTitleFont 0x2e4, 15 retratos 0x2e8, i_male.tga 0x324, [shadow.jpg 0x3a0, m_b454 0x4a8
	C_ASSERT(offsetof(ntAvatarChatMain, m_genderIcons) - offsetof(ntAvatarChatMain, m_pTitleFont) == 0x40);
	C_ASSERT(offsetof(ntAvatarChatMain, m_shadowTexture) - offsetof(ntAvatarChatMain, m_pTitleFont) == 0xbc);
	C_ASSERT(offsetof(ntAvatarChatMain, m_b454) - offsetof(ntAvatarChatMain, m_pTitleFont) == 0x1c4);
	if (0) // HACK: dead refs keep the inlines the naked functions use emitted and this constructor after them (emission order)
	{
		((ntAvatarChatMain*)0)->ntAvatarChatMain::OnLoad();
		((ntAvatarChatMain*)0)->ntAvatarChatMain::OnProcess(0.0f);
		((ntAvatarChatMain*)0)->ntAvatarChatMain::ProcessCamera(0.0f);
		((ntAvatarChatMain*)0)->ntAvatarChatMain::OnDisplay();
		((ntAvatarChatMain*)0)
			->ntAvatarChatMain::HandleMsg(*(const MsgObject*)0);
		ntAvatar* a0 = ntAvatarManager::Instance()->m_pMyAvatar;
		a0->GetMySpItem((eSPAVILITYTYPE)0).getflag();
		WRect rc;
		rc.IsInRect(0.0f, 0.0f);
		IsEqual(0.0f, 0.0f);
		a0->GetMySpItem((eSPAVILITYTYPE)0).changeflag();
		a0->GetMySpItem((eSPAVILITYTYPE)0).setvalue(0.0f);
		a0->IsFollowing();
		a0->IsHoldState();
		a0->GetUID();
		a0->GetIDName();
		a0->GetNickName();
		a0->GetTargetPosition();
		a0->GetMotion();
		a0->GetState();
		a0->GetAction();
		a0->AddState(0);
		a0->SubState(0);
		a0->GetPartTidList();
		SetLocalPVS(GetPVS());
		ntBalloonTextManager::IsInstantiated();
		CChatMsg::Instance()->IsOpenning();
		g_input->FixPointer(false);
		g_input->GetSensitivity();
		Waabb box;
		GetPVS().GetAABB(box);
		std::find(CSharedDoc::Instance()->m_slotList.begin(),
			CSharedDoc::Instance()->m_slotList.end(), 0);
	}
	m_dispPriority = (eDispPriority)4;
	m_pPVS = NULL;
	m_pPVS = &::GetPVS();
	m_curMap = 0;
	m_u4c9 = 0;
	m_time = 0.0f;
	m_v250 = WVector::ZERO;
	m_v244 = WVector(-0.48869219f, 0.0f, 0.0f);
	m_v25c.x = 28.800001f;
	m_v25c.y = 28.800001f;
	m_v25c.z = 28.800001f;
	m_f26c = 4.6f;
	m_f270 = 4.6f;
	m_f274 = 0.0f;
	m_f278 = 0.0f;
	m_f268 = 1.0f;
	m_bTopView = false;
	m_f28c = 256.0f;
	m_b208 = false;
	m_glowBallTex = g_resrcmng->LoadTexture("~GlowBall.jpg", 0, 0, NULL);
	m_b2a0 = false;
	new ntBalloonTextManager();
	ntBalloonTextManager::Instance()->Init();
	WTitleFont* font = g_resrcmng->GetTitleFont();
	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));
	char* fontName;
	info.fonth = 16;
	info.fontw = 16;
	m_pTitleFont = font;
	fontName = "[font_wind.jpg";
	info.filename = &fontName;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:, ";
	font->Create(&info);
	InitUI();
	m_b4ca = false;
	m_i23c = -1;
}

extern "C" void __cdecl jp983_ctxmenu_owner_gone(ntAvatarChatMain* owner);
ntAvatarChatMain::~ntAvatarChatMain()
{
	jp983_ctxmenu_owner_gone(this); // D503: s_menuOwner nao pode apontar para o Main liberado
	g_audio->StopSfx("AmbientSnd");
	ntBalloonTextManager::Instance()->Shutdown();
	delete ntBalloonTextManager::Instance();
	if (g_resrcmng)
	{
		if (m_glowBallTex)
		{
			g_resrcmng->Release(m_glowBallTex);
			m_glowBallTex = 0;
		}
		if (g_resrcmng && m_pTitleFont)
		{
			g_resrcmng->Release(m_pTitleFont);
			m_pTitleFont = NULL;
		}
	}
	for (int i = 0; i < 15; i++) // 983 007af600: 15 retratos em +0x2e8
	{
		ClearTexCache(m_pBitmap[i]);
		delete m_pBitmap[i];
		m_pBitmap[i] = NULL;
	}
}

void ntAvatarChatMain::OnPreLoadInit()
{
	if (0)
		RotMat(*(WVector*)0);
	CTaskMain::OnPreLoadInit();
	new CShadowManager(g_resrcmng, COption::Instance()->vIsShadowEnabled());
	CSharedDoc::Instance()->m_pGolfDoc->MakePlayer(1);
}

void ntAvatarChatMain::OnLoad()
{
	sSlotInfo slotInfo;
	sCharacterInfo charInfo;

	CTaskMain::OnLoad();

	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::eNetUnit(0)))
	{
		SetCurMap(COption::Instance()->dGetMap());

		LoadBg(CSharedDoc::Instance()->m_golfGame.pCourse
				? CSharedDoc::Instance()->m_golfGame.pCourse->Data
				: 0,
			COption::Instance()->dGetStartHole());

		WPolySoup::sCamera* camera =
			CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup->FindCamera(
				"DefaultCamera");
		if (camera)
		{
			g_camera = camera->mat;
			g_view->SetFOV(camera->fov * g_DEGTORAD);
		}

		g_audio->StopBGM(false);
		if (CSharedDoc::Instance()->m_golfGame.pCourse &&
			CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd[0])
		{
			g_audio->StopSfx("AmbientSnd");
			g_audio->PlaySfx(
				CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd, 0, 1,
				"AmbientSnd", 0, 0.5f, 200.0f);
		}

		CSceneManager::Instance()->SetWeather(
			COption::Instance()->dGetWeather(), false);
		ntAvatarManager::Instance()->FreeAllAvatars();

		RbPolySoupPoint* point =
			((RbPolySoup*)CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup)
				->FindPoint(MakeStr(POINT_START_NAME));
		if (!point)
			point =
				((RbPolySoup*)CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup)
					->FindPoint(MakeStr(POINT_START_NAME2));

		WVector pos = GetPVS().GetGroundPoint(point->pos, false, 0, false);

		memset(&slotInfo, 0, sizeof(slotInfo));
		slotInfo.location[2] = Random(0.0f, 6.2831855f);
		slotInfo.bMaster = 1;
		slotInfo.dwGuid = 0;
		slotInfo.dwTitle = 0;
		slotInfo.level = 1;
		rand();
		slotInfo.gender = 0;
		slotInfo.location[0] = pos.x;
		slotInfo.location[1] = pos.z;
		strcpy(slotInfo.sNick, "jufoot");
		slotInfo.tidChar =
			COption::Instance()->dGetDefaultCharacter() | 0x4000000;
		CSharedDoc::Instance()->m_slotList.push_back(slotInfo);

		memset(&charInfo, 0, sizeof(charInfo));
		charInfo.tid = slotInfo.tidChar;
		ntAvatarManager* mgr = ntAvatarManager::Instance();
		mgr->m_pMyAvatar = mgr->CreateAvatar(&slotInfo, &charInfo, this);
		{
			ntAvatarBurden burden(0, 0, charInfo, 0);
			((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
		}

		for (unsigned long i = 1;
			(int)i < COption::Instance()->dGetNumPlayers(); i++)
		{
			point =
				((RbPolySoup*)CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup)
					->FindPoint(MakeStr(POINT_START_NAME));
			if (!point)
				point = ((RbPolySoup*)CSharedDoc::Instance()
							 ->m_pGolfDoc->m_pPolySoup)
							->FindPoint(MakeStr(POINT_START_NAME2));

			float x = point->pos.x;
			float y = point->pos.y;
			float z = point->pos.z;
			x += Random(-16.0f, 16.0f);
			z += Random(-16.0f, 16.0f);
			WVector p(x, y, z);
			pos = GetPVS().GetGroundPoint(p, false, 0, false);

			memset(&slotInfo, 0, sizeof(slotInfo));
			slotInfo.location[2] = Random(0.0f, 6.2831855f);
			slotInfo.bMaster = 0;
			slotInfo.dwTitle = 0;
			slotInfo.level = 1;
			slotInfo.dwGuid = i;
			rand();
			slotInfo.gender = 0;
			slotInfo.location[0] = pos.x;
			slotInfo.location[1] = pos.z;
			strcpy(slotInfo.sNick, MakeStr("dummy%2d", i));
			slotInfo.tidChar = (rand() % 7) | 0x4000000;
			CSharedDoc::Instance()->m_slotList.push_back(slotInfo);

			memset(&charInfo, 0, sizeof(charInfo));
			charInfo.tid = slotInfo.tidChar;
			_util::GenerateRandomTidParts(slotInfo.tidChar, charInfo.tidParts,
				charInfo.tidAuxParts);
			ntAvatarManager::Instance()->CreateAvatar(&slotInfo, &charInfo,
				this);
			{
				ntAvatarBurden burden(i, i, charInfo, 0);
				((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
			}
		}
	}
	else
	{
		SetCurMap(CSharedDoc::Instance()->m_roomInfo.mapType);
		LoadBg(CSharedDoc::Instance()->m_golfGame.pCourse
				? CSharedDoc::Instance()->m_golfGame.pCourse->Data
				: 0,
			CSharedDoc::Instance()->m_roomInfo.nHole);

		WPolySoup::sCamera* camera =
			CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup->FindCamera(
				"DefaultCamera");
		if (camera)
		{
			g_camera = camera->mat;
			g_view->SetFOV(camera->fov * g_DEGTORAD);
		}

		g_audio->StopBGM(false);
		if (CSharedDoc::Instance()->m_golfGame.pCourse &&
			CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd[0])
		{
			g_audio->StopSfx("AmbientSnd");
			g_audio->PlaySfx(
				CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd, 0, 1,
				"AmbientSnd", 0, 0.5f, 200.0f);
		}

		CSceneManager::Instance()->SetWeather(0, false);
		ntAvatarManager::Instance()->FreeAllAvatars();
	}

	CGolfDoc* golfDoc = CSharedDoc::Instance()->m_pGolfDoc;
	golfDoc->m_pSky->GenerateClouds(50, Wind().GetPower983());
	Wind().Init(0xff, 0xff, 0, 1);
	Wind().CheckSpecialWind();
	WVector wind(1.0f, 0.0f, 1.0f);
	Wind().SetWind(wind);
	CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup->UpdateBgFxWind();
	((RbSceneManager*)CSceneManager::Instance())->UpdateWind();

	m_genderIcons[0] = g_resrcmng->GetOverlay("i_male.tga", 0);
	m_genderIcons[1] = g_resrcmng->GetOverlay("i_female.tga", 0);
	m_genderIcons[2] = g_resrcmng->GetOverlay("i_male_02.tga", 0);
	m_genderIcons[3] = g_resrcmng->GetOverlay("i_female_02.tga", 0);
	m_genderIcons[4] = g_resrcmng->GetOverlay("i_male_03.tga", 0);
	m_genderIcons[5] = g_resrcmng->GetOverlay("i_female_03.tga", 0);
	m_mannerIcons[0] = g_resrcmng->GetOverlay("i_male_manner.tga", 0);
	m_mannerIcons[1] = g_resrcmng->GetOverlay("i_female_manner.tga", 0);
	m_angelIcons[0] = g_resrcmng->GetOverlay("i_male_angel.tga", 0);
	m_angelIcons[1] = g_resrcmng->GetOverlay("i_female_angel.tga", 0);
	m_stateIcons[0] = g_resrcmng->GetOverlay("dive.tga", 0);
	m_stateIcons[1] = g_resrcmng->GetOverlay("state_icon_01.tga", 0);
	m_stateIcons[2] = g_resrcmng->GetOverlay("state_icon_02.tga", 0);
	m_stateIcons[3] = g_resrcmng->GetOverlay("state_icon_03.tga", 0);
	m_stateIcons[4] = g_resrcmng->GetOverlay("state_icon_04.tga", 0);
	m_stateIcons[5] = g_resrcmng->GetOverlay("state_icon_05.tga", 0);
	m_stateIcons[6] = g_resrcmng->GetOverlay("state_icon_06.tga", 0);
	m_stateIcons[7] = g_resrcmng->GetOverlay("stateicon_01_n.tga", 0);
	m_stateIcons[8] = g_resrcmng->GetOverlay("stateicon_02_n.tga", 0);
	m_stateIcons[9] = g_resrcmng->GetOverlay("stateicon_03_n.tga", 0);
	m_stateIcons[10] = g_resrcmng->GetOverlay("stateicon_04_n.tga", 0);
	m_emotiObject = g_resrcmng->GetOverlay("[emoti_object.png", 0);

	for (unsigned int i = 0; i < 9; i++)
	{
		m_stateBars[i] =
			g_resrcmng->GetOverlay(MakeStr("statebar_%02d.tga", i + 1), 0);
	}

	m_shadowTexture = g_resrcmng->LoadTexture("[shadow.jpg", 0, 0, 0);

	// 983 007ac230: +0x11c (CSceneManager de 0x124, ctor 009b42a0 poe 1 ali); o 0x128 do 645 escrevia fora do objeto (D791)
	RAW(bool, CSceneManager::Instance(), 0x11c) =
		!RAW(bool, CSceneManager::Instance(), 0x11c);

	if (CMessengerInfo::Instance()->GetMyStatus() != 3)
		CMessengerInfo::Instance()->SetMyStatus((MSN::eStatusType)0);
}

void ntAvatarChatMain::DoReLoad()
{
	SendMsg(this, "Ambient", M983(0x1f4), 0, 0, 0, 0);
	OnLoad();
	if (CTaskManager::Instance())
	{
		CTaskManager::Instance()->GetCurrentTask()->Init(NULL);
	}
}

void ntAvatarChatMain::OnInit()
{
	CTaskMain::OnInit();
	SetBGMNone();
	CalcUIOffset(false);
	if (CMouseCursor::IsInstantiated())
	{
		CMouseCursor::Instance()->SetActive(true);
		CMouseCursor::Instance()->SetMode(2);
	}
	((RbSceneManager*)CSceneManager::Instance())->UpdateFog(true);
	CSceneManager::Instance()->RegisterElement(OBJ_ACTOR, NULL, this,
		"\xbe\xc6\xb9\xd9\xc5\xb8\xb8\xde\xc0\xce", 0, NULL);
	if (CChatMsg::Instance())
	{
		CChatMsg::Instance()->Close(true, true);
		CChatMsg::Instance()->EnableCaretMove(true);
	}
	if (!CChatMsg::Instance()->IsOpened())
	{
		g_pFresh->GetManager()->SetExclusiveKey(false);
	}
	FrJukeBoxDlg::Init();
	FrJukeBoxDlg::PlayWithMap(GetCurMap());
	g_input->Reset();
}

void ntAvatarChatMain::OnDestroy()
{
	CTaskMain::OnDestroy();
	if (CShadowManager::Instance())
	{
		delete CShadowManager::Instance();
	}
	int i;
	for (i = 0; i < 6; i++)
	{
		SAFE_RELEASE_TEX(m_genderIcons[i]);
	}
	for (i = 0; i < 2; i++)
	{
		SAFE_RELEASE_TEX(m_mannerIcons[i]);
	}
	for (i = 0; i < 2; i++)
	{
		SAFE_RELEASE_TEX(m_angelIcons[i]);
	}
	SAFE_RELEASE_TEX(m_stateIcons[0]);
	SAFE_RELEASE_TEX(m_stateIcons[1]);
	SAFE_RELEASE_TEX(m_stateIcons[2]);
	SAFE_RELEASE_TEX(m_stateIcons[3]);
	SAFE_RELEASE_TEX(m_stateIcons[4]);
	SAFE_RELEASE_TEX(m_stateIcons[5]);
	SAFE_RELEASE_TEX(m_stateIcons[6]);
	SAFE_RELEASE_TEX(m_stateIcons[7]);
	SAFE_RELEASE_TEX(m_stateIcons[8]);
	SAFE_RELEASE_TEX(m_stateIcons[9]);
	SAFE_RELEASE_TEX(m_stateIcons[10]);
	SAFE_RELEASE_TEX(m_emotiObject);
	for (i = 0; i < 9; i++)
	{
		SAFE_RELEASE_TEX(m_stateBars[i]);
	}
	SAFE_RELEASE_TEX(m_shadowTexture);

	ntAvatarManager* mgr = ntAvatarManager::Instance();
	int guids[30];
	int n = 0;
	int count = mgr->m_count;
	for (i = 0; i < 30; i += 6)
	{
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 0])
		{
			guids[n++] = mgr->m_avatars[i + 0]->m_guid;
		}
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 1])
		{
			guids[n++] = mgr->m_avatars[i + 1]->m_guid;
		}
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 2])
		{
			guids[n++] = mgr->m_avatars[i + 2]->m_guid;
		}
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 3])
		{
			guids[n++] = mgr->m_avatars[i + 3]->m_guid;
		}
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 4])
		{
			guids[n++] = mgr->m_avatars[i + 4]->m_guid;
		}
		if (n >= count)
		{
			break;
		}
		if (mgr->m_avatars[i + 5])
		{
			guids[n++] = mgr->m_avatars[i + 5]->m_guid;
		}
	}
	for (i = 0; i < n; i++)
	{
		// D787: 983 007a8fb0 manda 0x10d (= M983(0xe5), o caso que apaga o avatar); o 0xe5 cru do 645 nao tinha caso
		// e os avatares ficavam no ntAvatarManager depois de liberar texturas/CShadowManager
		MsgObject msg(this, M983(0xe5), guids[i], 0, 0, 0, 0);
		HandleMsg(msg);
	}
	g_pFresh->GetManager()->SetExclusiveKey(true);
	g_audio->StopBGM(true); // 983 007a900f: [00f02464]->vt+0x18(1)
}

void ntAvatarChatMain::OnPreserveBack(char const* taskName)
{
	if (0)
		((ntAvatarChatMain*)0)
			->ntAvatarChatMain::
				DoReLoad(); // HACK: emission order (compiled after DoReLoad)
	// D790: 983 007a99e0 nao liga os bits 2/4 do estado (o 645 mandava 0x0063 08 ao ir ao My Room/Loja) nem chama
	// FrJukeBoxDlg::Preserve: so Stop, StopSfx, mapa/buraco, SetPVSIndex, ResetFrWndPointers
	ntAvatarManager::Instance()->m_pMyAvatar->Stop();
	g_audio->StopSfx("AmbientSnd");
	m_curMap = GetCurMap();
	m_u4c9 = CSharedDoc::Instance()->m_pGolfDoc->m_currentHole;
	SetPVSIndex(1);
	ResetFrWndPointers();
	CTaskMain::OnPreserveBack(taskName);
}

void ntAvatarChatMain::OnRestorePreserved()
{
	CTaskMain::OnRestorePreserved();

	memcpy(&g_lightset, &m_lightset, sizeof(m_lightset));
	SetPVSIndex(0);
	SetCurMap(m_curMap);
	CSharedDoc::Instance()->m_pGolfDoc->m_currentHole = m_u4c9;

	COption::Instance()->vSetMinimumScreenSize(1, 800, 600);
	COption::Instance()->vApplyGameScreenSize();

	ntAvatarManager::Instance()->m_bRestored = true;

	unsigned int key = *(unsigned int*)&CSharedDoc::Instance()->m_myInfo.userEquip.guidChar;
	std::map<unsigned int, sCharacterInfo>::iterator it =
		CSharedDoc::Instance()->m_charMap.find(key);
	if (it != CSharedDoc::Instance()->m_charMap.end())
	{
		WSendPacket packet(0xc);
		packet.Encode1(4);
		packet.Encode4(it->second.guid);
		packet.Send(TO_GAME);
	}

	ntAvatarManager::Instance()->ResetAvatarPositionAngle();

	if (CSharedDoc::Instance()->m_golfGame.pCourse &&
		CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd[0])
	{
		g_audio->PlaySfx(CSharedDoc::Instance()->m_golfGame.pCourse->AmbientSnd,
			0, 1, "AmbientSnd", 0, 0.5f, 200.0f);
	}

	SetBGMNone();
	CalcUIOffset(false);

	if (CMouseCursor::Instance())
	{
		CMouseCursor::Instance()->SetActive(true);
		CMouseCursor::Instance()->SetMode(2);
	}

	// D790: 983 007ab920 nao tem FrJukeBoxDlg::Restore (unica chamada de audio = PlaySfx do AmbientSnd)
	ntAvatarManager::Instance()->m_pMyAvatar->SetState(
		ntAvatarManager::Instance()->m_pMyAvatar->m_state & 0xfffffff9);
}

void ntAvatarChatMain::LoadBg(char const* name, int hole)
{
	m_str240 = name;
	CSharedDoc::Instance()->m_pGolfDoc->m_currentHole = hole;

	if (CFx::Instance())
		CFx::Instance()->SetActive(false);

	CGolfDoc* doc = CSharedDoc::Instance()->m_pGolfDoc;
	if (!doc->m_pPolySoup->Load(
			MakeStr("%s_%02d.gbin", name, doc->m_currentHole), true))
	{
		g_bQuit = true;
		return;
	}

	memcpy(&m_lightset, &g_lightset, sizeof(m_lightset));

	CSharedDoc::Instance()->m_pGolfDoc->m_waveInfo.Load(
		MakeStr("%s_wave.txt", name));
	CSharedDoc::Instance()->m_pGolfDoc->m_riverInfo.Load(
		MakeStr("%s_river.txt", name));

	CSharedDoc::Instance()->m_pGolfDoc->m_pSky = new CSky;
	CSharedDoc::Instance()->m_pGolfDoc->m_pSky->LoadSky(name, 0.0f);
	CSharedDoc::Instance()->m_pGolfDoc->m_pSky->RegisterCloud(
		MakeStr("%s_cloud.dds", name));

	CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup->MakeDataStructure();

	if (CSharedDoc::Instance()->m_pGolfDoc->m_pSky)
	{
		CSharedDoc::Instance()->m_pGolfDoc->m_pSky->GenerateStars(-1);
		CSharedDoc::Instance()->m_pGolfDoc->m_pSky->GenerateLensFlare();
		CSharedDoc::Instance()->m_pGolfDoc->m_pSky->GenerateMoons();
	}

	CShadowManager::Instance()->UpdateLight("sun", g_lightset);
	CSceneManager::Instance()->Load();
	CSharedDoc::Instance()->m_pGolfDoc->m_pPolySoup->EndLoad(false);

	if (CFx::Instance())
		CFx::Instance()->SetActive(true);
}

void ntAvatarChatMain::CheckUIPosition()
{
	static float s_barY = COption::Instance()->gGetBar_Y();
	if (s_barY != COption::Instance()->gGetBar_Y())
	{
		CalcUIOffset(false);
		s_barY = COption::Instance()->gGetBar_Y();
		g_pFresh->OpenLayout("AVATARCHAT_MAIN", this, false);
	}
}

void ntAvatarChatMain::ToggleTopViewMode()
{
	bool wasTop = m_bTopView;
	m_bTopView = !wasTop;
	if (!wasTop)
	{
		CSharedDoc::Instance()->m_pGolfDoc->m_gameMode = 0x200;
		float* bounds = *(float**)((char*)m_pPVS + 4);
		float minX = bounds[0x60 / 4];
		float minZ = bounds[0x68 / 4];
		float maxX = bounds[0x6c / 4];
		float maxZ = bounds[0x74 / 4];
		ntAvatar* me = ntAvatarManager::Instance()->m_pMyAvatar;
		if (me == NULL)
		{
			m_topViewPos.y = bounds[0x64 / 4];
			m_topViewPos.x = (maxX + minX) * 0.5f;
			m_topViewPos.z = (maxZ + minZ) * 0.5f;
		}
		else
		{
			m_topViewPos = me->m_pos;
		}
		m_f28c = 256.0f;
		*((char*)CSceneManager::Instance() + VC10_OFS_BO(0xb8)) = 0; // B1468: m_bFog (cl 14.44 0xb4: BaseObject)
	}
	else
	{
		CSharedDoc::Instance()->m_pGolfDoc->m_gameMode = 0;
		*((char*)CSceneManager::Instance() + VC10_OFS_BO(0xb8)) = 1; // B1468: m_bFog (cl 14.44 0xb4: BaseObject)
	}
	ntAvatarManager::Instance()->SetFxMode983(m_bTopView); // 983 007aa250: fx das pecas 0x200 na vista de cima, 8 de volta
}

#include "jp/refresh/rflobbydesktop.h"
extern "C" void __cdecl jp983_rastro(const char* fmt, ...); // A819
void ntAvatarChatMain::OnProcess(float dt)
{
	static WVector s_camRight = WVector::UNIT_POS_Z;
	static WVector s_camLeft = WVector::UNIT_POS_X;
	static float s_prevDelta[3];
	static bool s_bRotating;
	static bool s_bFollowing;
	static bool s_bKeyMove;

	CheckUIPosition();
	CTaskMain::OnProcess(dt);
	m_pPVS->PreProcess(dt, false);
	ntBalloonTextManager::Instance()->Process(dt);
	ntAvatarManager* mgr = ntAvatarManager::Instance();
	if (!mgr->m_pMyAvatar)
		return;
	// 983 007aa31f: janela modal do RF aberta = sai antes da entrada, dos avatares, da camera e da UI
	if (RFSceneManager::GetManager983() && RFSceneManager::GetManager983()->IsModalExist())
		return;

	FrDesktop* desktop = g_pFresh->GetManager()->GetDesktop();
	// 983 007aa260: desktop+0x1c bit 3 = m_nFlags & FWF_VIEWFOCUS (o +8 do 645 cai no vptr do
	// FrModalRenderer, cujo bit 3 depende do link: o clique no chao do Lounge nunca era lido)
	if (!desktop->m_nFlags.GetFlag(FWF_VIEWFOCUS))
	{
		if (m_pForm4b8 && m_pForm4b8->IsVisible()) // 983 +0x44 bit 0
			m_pForm4b8->SetVisible(false); // 983 007ab566: vtable+0x1c (Close marcava FWF_DESTROY e liberava o rmenu)
		g_input->FixPointer(false);
		CMouseCursor::Instance()->Toggle(true);
	}
	else
	{
		int left = g_input->GetButton(LEFT_BUTTON);
		int right = g_input->GetButton(RIGHT_BUTTON);
		int middle = g_input->GetButton(MIDDLE_BUTTON);
		WVector delta = g_input->GetMouseDelta();
		WVector mouse = g_input->GetMousePoint();
		float mx = mouse.x;
		float my = mouse.y;
		WVector rayO, rayD;
		g_view->GetScreen2World(mx, my, rayO, rayD);
		if (m_pForm4b8 &&
			!CMouseCursor::Instance()->InArea(m_pForm4b8->GetRect()) &&
			(left == 1 || right == 1))
			m_pForm4b8->SetVisible(false); // 983 007aa414

		float scroll = m_f28c * (1.0f / 960.0f);
		if (scroll <= 0.1f)
			scroll = 0.1f;

		if (!s_bKeyMove)
		{
			s_camLeft = WVector(g_view->GetCamera().xx, g_view->GetCamera().yx,
				g_view->GetCamera().zx);
			s_camLeft = s_camLeft.Normalize();
			s_camRight = WVector(WVector::UNIT_POS_Y.z * s_camLeft.y -
					s_camLeft.z * WVector::UNIT_POS_Y.y,
				s_camLeft.z * WVector::UNIT_POS_Y.x -
					WVector::UNIT_POS_Y.z * s_camLeft.x,
				WVector::UNIT_POS_Y.y * s_camLeft.x -
					s_camLeft.y * WVector::UNIT_POS_Y.x);
			s_camRight = s_camRight.Normalize();
		}
		if (CMouseCursor::Instance()->GetArea() == 0)
		{
			if (!m_bTopView)
			{
				mgr->m_pHoverAvatar = mgr->AvatarByRayIntersect(rayO, rayD);
				mgr->m_pClickAvatar = mgr->CheckClick(mx, my);
				CMouseCursor::Instance()->SetCursor(
					mgr->m_pHoverAvatar || mgr->m_pClickAvatar ? 1 : 0);
			}
			bool onForm = false;
			if (m_pForm4b8 && m_pForm4b8->IsVisible()) // 983 +0x44 bit 0
			{
				WRect rc(m_pForm4b8->GetRect()); // 983 +0x34
				onForm = rc.IsInRect(mx, my);
			}
			// 983 007aa527: clique sobre janela RF salta para o mesmo 007aac16 (sem direito, sem roda); m_name da visao RFWnd = +0xd8 so no VC10
			if (!onForm && left == 1 && RFSceneManager::Instance() && RFSceneManager::Instance()->IsOverWnd983(mx, my))
				onForm = true;
			if (!onForm)
			{
				if (left == 1)
				{
					ntAvatar* picked;
					if (!m_bTopView && (picked = mgr->CheckClick(mx, my)) != 0)
					{
						strcpy(m_szPickNick, picked->m_nickName);
						m_i23c = picked->m_uid;
						if (picked->m_state & 0x80)
						{
							if (picked == mgr->m_pMyAvatar)
								OnOpenTradeShop();
							else
								OnEnterTradeShop();
						}
					}
					else
					{
						WVector hit = m_pPVS->GetRayIntersection(rayO, rayD);
						WVector target = hit;
						if (!(mgr->m_pMyAvatar->m_state & 0x7e))
						{
							int idx;
							target =
								m_pPVS->GetGroundPoint(hit, true, &idx, true);
							if (idx >= 0)
							{
								s_bFollowing = true;
								mgr->m_pMyAvatar->SetTargetPosition(target);
								mgr->m_pMyAvatar->SetAction(0);
								mgr->m_pMyAvatar->Follow(0);
							}
						}
					}
				}
				if (right == 1)
				{
					if (!m_bTopView)
					{
						if (mgr->m_pHoverAvatar)
						{
							strcpy(m_szPickNick,
								mgr->m_pHoverAvatar->m_nickName);
							m_i23c = mgr->m_pHoverAvatar->m_uid;
							s_bRotating = false;
							goto afterRight;
						}
						m_szPickNick[0] = 0;
						m_i23c = -1;
						m_b210 = 0;
					}
					s_bRotating = false;
				}
				else if (right == 3)
				{
					if (!m_bTopView)
					{
						static float s_prevY = 0.0f;
						float dy = (delta.y + s_prevY) * 0.5f;
						m_v244.y -= (s_prevY + delta.x) * 0.5f * 0.008f;
						m_v244.x -= dy * 0.008f;
						s_prevDelta[0] = delta.x;
						s_prevY = delta.y;
						s_prevDelta[2] = delta.z;
						if (m_v244.x < -1.5707964f)
							m_v244.x = -1.5707964f;
						if (0.7853982f < m_v244.x)
							m_v244.x = 0.7853982f;
						m_v27c = m_v244;
						m_f274 = 0.0f;
					}
					if (!WisZero(delta.x, g_EPSILON) ||
						!WisZero(delta.y, g_EPSILON))
						s_bRotating = true;
					g_input->FixPointer(true);
					CMouseCursor::Instance()->Toggle(false);
				}
				else if (right == 2)
				{
					jp983_rastro("A819 dir solto top=%d gira=%d hover=%p pick=%s menu=%p", (int)m_bTopView, (int)s_bRotating,
						mgr->m_pHoverAvatar, m_szPickNick, m_pForm4b8);
					if (!m_bTopView && !s_bRotating && mgr->m_pHoverAvatar)
					{
						if (strcmp(mgr->m_pHoverAvatar->m_nickName,
								m_szPickNick) == 0)
						{
							WVector p1 = g_input->GetMousePoint();
							WVector p2 = g_input->GetMousePoint();
							PopupRightMenu((int)p1.x, (int)p2.y);
						}
					}
					g_input->FixPointer(false);
					CMouseCursor::Instance()->Toggle(true);
				}
				else
				{
					g_input->FixPointer(false);
					if (!CProjectG::Instance()->HideGUI()) // 983 007aaa33: interface escondida mantem o cursor oculto
						CMouseCursor::Instance()->Toggle(true);
				}
afterRight:
				if (g_input->GetButton(LEFT_BUTTON) &&
					g_input->GetButton(RIGHT_BUTTON) && !m_bTopView &&
					!CChatMsg::Instance()->IsOpenning())
				{
					WVector dest = mgr->m_pMyAvatar->m_pos;
					dest.x += s_camRight.x * 4.1f;
					dest.y += s_camRight.y * 4.1f;
					dest.z += s_camRight.z * 4.1f;
					ntAvatar* me = mgr->m_pMyAvatar;
					if (!WisEqual(dest, me->m_pos, 1e-05f) &&
						!(me->m_state & 0x7e))
					{
						s_bFollowing = false;
						me->SetTargetPosition(dest);
						mgr->m_pMyAvatar->SetAction(0);
						mgr->m_pMyAvatar->Follow(0);
					}
				}
				if (middle == 1)
					ToggleTopViewMode();
				FrWndManager* wm = g_pFresh->GetManager();
				if (!wm->GetWheelFocus())
				{
					if (!m_bTopView)
						m_v25c.y = Between<float>(10.88f,
							m_v25c.y - g_input->GetSensitivity() * delta.z * 0.021f,
							64.0f); // 983 007aa260: faixa 10,88..64 (nao 3..10,88: a distancia 28,8 colava a camera)
					else
						m_f28c = Between<float>(25.6f,
							m_f28c - g_input->GetSensitivity() * delta.z * scroll * 0.4f,
							960.0f);
				}
			}
		}
		if (m_bTopView)
		{
			if (g_input->GetMousePoint().x < 5.0f)
				m_topViewPos.x -= scroll * dt * 800.0f;
			if (g_view->GetWidth() - 6.0f < g_input->GetMousePoint().x)
				m_topViewPos.x += scroll * dt * 800.0f;
			if (g_input->GetMousePoint().y < 5.0f)
				m_topViewPos.z += scroll * dt * 800.0f;
			if (g_view->GetHeight() - 6.0f < g_input->GetMousePoint().y)
				m_topViewPos.z -= scroll * dt * 800.0f;
		}
		if (!g_pFresh->GetManager()->IsKeyExclusive())
		{
			if ((g_input->GetDown("ENTER", true) ||
					g_input->GetDown("PADENTER", true)) &&
				CChatMsg::Instance()->IsClosed())
				CChatMsg::Instance()->Open(true, true);
			bool shift =
				g_input->Get("LSHIFT", true) || g_input->Get("RSHIFT", true);
			bool ctrl = g_input->Get("LCONTROL", true) ||
				g_input->Get("RCONTROL", true);
			WVector dest = mgr->m_pMyAvatar->m_pos;
			if (!m_bTopView && !CChatMsg::Instance()->IsOpenning())
			{
				if (ctrl)
				{
					float yaw = 0.0f;
					float pitch = 0.0f;
					if (g_input->Get(KEYNAME_UP, true))
						pitch = dt * -0.5235988f;
					if (g_input->Get(KEYNAME_DOWN, true))
						pitch = dt * 0.5235988f + pitch;
					if (g_input->Get(KEYNAME_LEFT, true))
						yaw = dt * -0.5235988f;
					if (g_input->Get(KEYNAME_RIGHT, true))
						yaw = dt * 0.5235988f + yaw;
					if (!WisZero(yaw, g_EPSILON) || !WisZero(pitch, g_EPSILON))
					{
						m_v244.y -= yaw;
						m_v244.x -= pitch;
						if (m_v244.x < -1.5707964f)
							m_v244.x = -1.5707964f;
						if (0.7853982f < m_v244.x)
							m_v244.x = 0.7853982f;
						m_v27c = m_v244;
						m_f274 = 0.0f;
					}
				}
				else
				{
					if (g_input->Get(KEYNAME_UP, true))
					{
						dest.x += s_camRight.x * 4.1f;
						dest.y += s_camRight.y * 4.1f;
						dest.z += s_camRight.z * 4.1f;
					}
					if (g_input->Get(KEYNAME_DOWN, true))
					{
						dest.x += -s_camRight.x * 4.1f;
						dest.y += -s_camRight.y * 4.1f;
						dest.z += -s_camRight.z * 4.1f;
					}
					if (g_input->Get(KEYNAME_LEFT, true))
					{
						dest.x += -s_camLeft.x * 4.1f;
						dest.y += -s_camLeft.y * 4.1f;
						dest.z += -s_camLeft.z * 4.1f;
					}
					if (g_input->Get(KEYNAME_RIGHT, true))
					{
						dest.x += s_camLeft.x * 4.1f;
						dest.y += s_camLeft.y * 4.1f;
						dest.z += s_camLeft.z * 4.1f;
					}
					int idx;
					dest = m_pPVS->GetGroundPoint(dest, true, &idx, true);
					if (idx < 0)
						dest = mgr->m_pMyAvatar->m_pos;
				}
				if (g_input->Get("HOME", true))
					m_v25c.y = Between<float>(10.88f, m_v25c.y - dt * 16.8f, 64.0f);
				if (g_input->Get("END", true))
					m_v25c.y = Between<float>(10.88f, m_v25c.y + dt * 16.8f, 64.0f);
				ntAvatar* me;
				if (g_input->GetDown("PAGE_DOWN", true) &&
					(me = mgr->m_pMyAvatar, !(me->m_state & 0x7e)) &&
					me->m_action != 2)
				{
					int act;
					if (me->m_action == 1)
					{
						act = 2;
					}
					else
					{
						me->Stop();
						me = mgr->m_pMyAvatar;
						act = 1;
					}
					me->SetAction(act);
				}
				if (g_input->GetDown("PAGE_UP", true) &&
					(me = mgr->m_pMyAvatar, !(me->m_state & 0x7e)))
				{
					if (me->m_action == 1)
						me->SetAction(0);
					else if (me->m_action == 2)
						me->SetAction(1);
				}
			}
			else
			{
				if (g_input->Get(KEYNAME_UP, true) && !shift)
					m_topViewPos.z += scroll * dt * 800.0f;
				if (g_input->Get(KEYNAME_DOWN, true) && !shift)
					m_topViewPos.z -= scroll * dt * 800.0f;
				if (g_input->Get(KEYNAME_LEFT, true))
					m_topViewPos.x -= scroll * dt * 800.0f;
				if (g_input->Get(KEYNAME_RIGHT, true))
					m_topViewPos.x += scroll * dt * 800.0f;
				if (g_input->Get("HOME", true) ||
					(shift && g_input->Get(KEYNAME_UP, true)))
					m_f28c = Between<float>(25.6f, m_f28c - scroll * dt * 1200.0f, 960.0f);
				if (g_input->Get("END", true) ||
					(shift && g_input->Get(KEYNAME_DOWN, true)))
					m_f28c = Between<float>(25.6f, m_f28c + scroll * dt * 1200.0f, 960.0f);
			}
			if (g_input->GetDown("PAD_0", true) ||
				g_input->GetDown("NUM_0", true))
				ToggleTopViewMode();
			ntAvatar* me = mgr->m_pMyAvatar;
			if (!WisEqual(dest, me->m_pos, 1e-05f) && !(me->m_state & 0x7e))
			{
				s_bFollowing = false;
				s_bKeyMove = true;
				me->SetTargetPosition(dest);
				mgr->m_pMyAvatar->SetAction(0);
				mgr->m_pMyAvatar->Follow(0);
			}
			else
			{
				s_bKeyMove = false;
			}
		}
	}

	if (mgr->m_pMyAvatar->IsTargetPositionChanged(false))
	{
		WVector dir;
		ntAvatar* me = mgr->m_pMyAvatar;
		if (!me->IsFollowing() || !me->GetFollowingAvatar())
		{
			if (!s_bKeyMove)
			{
				dir = me->m_targetPos - me->m_pos;
				m_f274 = 0.0f;
			}
			else
			{
				dir = s_camRight;
			}
		}
		else
		{
			ntAvatar* target = me->GetFollowingAvatar();
			dir = target->m_pos - me->m_pos;
			s_bFollowing = true;
		}
		dir.Normalize();
		m_f278 = atan2f(dir.z, dir.x) + 4.712389f;
		m_v27c = m_v244;
	}
	mgr->Process(dt);
	if (mgr->m_pMyAvatar->m_motion == 1)
		m_f274 += dt;
	ntAvatar* me = mgr->m_pMyAvatar;
	float speed;
	if (me->m_motion == 2 || me->m_motion == 3)
		speed = me->GetMySpItem((eSPAVILITYTYPE)0).getvalue() * 1.9f;
	else
		speed = me->GetMySpItem((eSPAVILITYTYPE)0).getvalue() * 4.6f;
	m_f270 = speed;
	float d = mgr->m_pMyAvatar->m_targetPos.x - mgr->m_pMyAvatar->m_pos.x;
	if (d < 1e-05f && -1e-05f < d)
	{
		d = mgr->m_pMyAvatar->m_targetPos.z - mgr->m_pMyAvatar->m_pos.z;
		if (d < 1e-05f && -1e-05f < d)
			s_bFollowing = false;
	}
	m_b208 = s_bFollowing;
	ProcessCamera(dt);
	ProcessUI(dt);
	m_time += dt;
	if (IsLocalContent((localContentType_t)0x58 /*983: era 0x57; medido*/))
		RefreshAllUccClothes();
}

void ntAvatarChatMain::ProcessCamera(float dt)
{
	if (m_b4ca && g_SALoadingState)
	{
		m_v25c.y = ntAvatarManager::Instance()
					   ->m_pMyAvatar->GetMySpItem((eSPAVILITYTYPE)0)
					   .getvalue() *
			28.8f;
		m_b4ca = false;
	}

	WMatrix mat;
	if (!m_bTopView)
	{
		g_view->SetClip(1.0f, 4096.0f, false);
		g_view->SetFOV(1.0471976f);
		g_view->UpdateCamera();

		float diff = m_v25c.x - m_v25c.y;
		float absDiff = diff;
		if (((unsigned int&)diff & 0x80000000) != 0)
			absDiff = -diff;
		if (absDiff >= 0.00001f)
		{
			if (m_v25c.y <= m_v25c.x)
			{
				m_v25c.x = m_v25c.x - dt * 46.0f;
				if (m_v25c.x < m_v25c.y)
					m_v25c.x = m_v25c.y;
			}
			else
			{
				m_v25c.x = dt * 46.0f + m_v25c.x;
				if (m_v25c.y < m_v25c.x)
					m_v25c.x = m_v25c.y;
			}
		}

		// D504: o 983 (007a9a50) chama FUN_007a9760 = RotMat(angulos, saida) com os sinais 983; a forma por valor (645) inverte o pitch
		// (camera colada olhando para cima, raio para dentro do chao)
		WVector rot = m_v244;
		RotMat(rot, mat);

		diff = m_f26c - m_f270;
		absDiff = diff;
		if (((unsigned int&)diff & 0x80000000) != 0)
			absDiff = -diff;
		if (absDiff >= 0.00001f)
		{
			if (m_f270 <= m_f26c)
			{
				m_f26c = m_f26c - dt * 5.0f;
				if (m_f26c < m_f270)
					m_f26c = m_f270;
			}
			else
			{
				m_f26c = dt * 5.0f + m_f26c;
				if (m_f270 < m_f26c)
					m_f26c = m_f270;
			}
		}

		ntAvatar* me = ntAvatarManager::Instance()->m_pMyAvatar;
		WVector origin;
		origin.x = me->m_pos.x;
		origin.y = me->m_pos.y + m_f26c;
		origin.z = me->m_pos.z;

		WVector dir(mat.xz * m_v25c.x, mat.yz * m_v25c.x, mat.zz * m_v25c.x);
		dir = -dir;

		float hit = GetPVS().RayIntersectAll(origin, dir, false);
		float dist;
		if (hit <= m_f268 || (dist = hit * m_v25c.x) <= m_v25c.z)
		{
			m_v25c.z = hit * m_v25c.x;
			m_f268 = hit;
		}
		else
		{
			m_v25c.z = dt * 16.0f + m_v25c.z;
			if (dist <= m_v25c.z)
			{
				m_v25c.z = dist;
				m_f268 = hit;
			}
		}

		dir.Normalize();
		WVector pos(origin.x + dir.x * m_v25c.z, dir.y * m_v25c.z + origin.y,
			origin.z + dir.z * m_v25c.z);
		WVector ground = GetPVS().GetGroundPoint(pos, false, 0, false);
		if (pos.y < ground.y + 3.5f)
			pos.y = ground.y + 3.5f;
		mat.pivot = pos;
	}
	else
	{
		g_view->SetClip(8.0f, 8192.0f, false);
		g_view->SetFOV(1.5707964f);
		g_view->UpdateCamera();

		RotMat(WVector(-1.5707964f, 0.0f, 0.0f), mat);
		WVector ground = GetPVS().GetGroundPoint(m_topViewPos, false, 0, false);
		mat.zm = ground.z;
		mat.ym = ground.y + m_f28c;
		mat.xm = ground.x;
	}
	g_camera = mat;
}

void ntAvatarChatMain::PrintText(float x, float y, char const* text,
	float space, int align, float scale, unsigned long color)
{
	if (text && strlen(text) != 0)
	{
		// D884: era *(char**)((char*)m_pTitleFont + 0xcc) cru (VC10 m_info.pCharSet); no cl 14.44 o WTitleFont anda -4
		// (std::string do BaseObject) e o cru lia m_iTexIndex -> strchr em lixo no Lounge
		const char* charset = m_pTitleFont->GetFontInfo()->pCharSet;
		float oldScale = m_pTitleFont->GetScale();
		m_pTitleFont->SetScale(scale);
		float width = 0.0f;
		if (align == 1)
		{
			for (const char* p = text; *p; p++)
			{
				const char* pos = strchr(charset, *p);
				width += m_pTitleFont->GetCharWidth(g_view, pos - charset) +
					space * scale;
			}
			width = (width - space) * 0.5f;
		}
		else if (align == 2)
		{
			for (const char* p = text; *p; p++)
			{
				const char* pos = strchr(charset, *p);
				width += m_pTitleFont->GetCharWidth(g_view, pos - charset) +
					space * scale;
			}
			width = width - space;
		}
		x -= width;
		while (*text)
		{
			const char* pos = strchr(charset, *text++);
			x += m_pTitleFont->PutChar(g_view, x, y, pos - charset, 0, color) +
				space;
		}
		m_pTitleFont->SetScale(oldScale);
		m_pTitleFont->Flush(g_view);
	}
}

void ntAvatarChatMain::Display()
{
	if (ntAvatarManager::Instance()->m_pMyAvatar)
	{
		ntAvatarManager::Instance()->Render();
		if (m_b208 && m_viewMode != 2)
		{
			WVector dirX = WVector(4.8f, 0.0f, 0.0f);
			WVector dirZ = WVector(0.0f, 0.0f, 4.8f);
			ntAvatar* me = ntAvatarManager::Instance()->m_pMyAvatar;
			if (me->m_followName[0] == '\0')
			{
				m_pPVS->DrawDecal(m_glowBallTex, me->m_targetPos, dirX, dirZ,
					(float)((cos((double)(m_time * 10.0f)) + 1.0f) * 0.05f +
						1.0f),
					1.0f, W_POLY_BLEND_ADD, false);
			}
			else
			{
				ntAvatar* target =
					ntAvatarManager::Instance()->FindAvatarByNickName(
						me->m_followName);
				if (target)
				{
					m_pPVS->DrawDecal(m_glowBallTex, target->m_pos, dirX, dirZ,
						(float)((cos((double)(m_time * 10.0f)) + 1.0f) * 0.05f +
							1.0f),
						1.0f, W_POLY_BLEND_ADD, false);
				}
			}
		}
	}
}

void ntAvatarChatMain::OnDisplay()
{
	ntAvatarManager::Instance()->RenderOverlay();
	if (m_viewMode == 2)
		return;

	if (!CProjectG::Instance()->HidePI())
		ntBalloonTextManager::Instance()->Render();

	if (m_viewMode != 2 && !CProjectG::Instance()->HideGUI() &&
		!CProjectG::Instance()->HidePI())
	{
		FrGraphicInterface* gfx = g_pFresh->GetManager()->GetGDI();
		char* text =
			MakeStr("[%d] %s", CSharedDoc::Instance()->m_roomInfo.roomGuid,
				CSharedDoc::Instance()->m_roomInfo.title);
		gfx->SetTextStyle(2);
		gfx->SetTextColor(0xffffffff, 0xff000000);
		gfx->PrintText(WPoint(20, 48), 0, text, 0);

		if (IsLocalContent((localContentType_t)10) &&
			!(*(unsigned char*)&CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 4))
		{
			std::list<sSlotInfo>::iterator it;
			for (it = CSharedDoc::Instance()->m_slotList.begin();
				it != CSharedDoc::Instance()->m_slotList.end(); ++it)
			{
				sSlotInfo info = *it;
				if (info.dwIdentity & 4)
					CSharedDoc::Instance()->m_briefUserInfoMap.find(
						info.dwGuid);
			}
		}

		// D481: o 983 imprime "%d/%d"; o espaco do "%2d" nao esta no charset da fonte (strchr NULL) e saia como "x" (S18)
		float x = g_view->GetWidth();
		text = MakeStr("%d/%d", CSharedDoc::Instance()->m_roomInfo.nUserNum,
			CSharedDoc::Instance()->m_roomInfo.nUserLimit);
		PrintText(x - 46.0f, 84.0f, text, 1.0f, 2, 1.0f, 0xffffffff);

		// D482 (S17): o botao de pessoas (983: x = largura - 38) nascia com a largura do Init (800 -> 783,159); reposiciona com a largura viva (1583,158)
		PlaceUserListBtn983(x);
	}
}

#define BRIEF_INFO(guid) \
	(CSharedDoc::Instance()->m_briefUserInfoMap.find(guid)->second)
#define TALK_ROOM_FX(pos) \
	do \
	{ \
		CFxSequence* fx = \
			CFx::Instance()->OpenSequence("Talk_Room.seq", true); \
		if (fx) \
			fx->m_pos = \
				WVector((pos).x, (pos).y + 5.0f, (pos).z); \
	} while (0)

#include "jp/refresh/rflobbydesktop.h" // 983 (C199): caso 0 do HandleMsg
#include "jp/taskform.h" // D792: FrTaskForm::OnMsg983 (+0xa0), repasse ao form da widget (My Room aberto no Lounge)
extern int g_lobbyModal983; // 00e78cac (jp/rflobbydesktop.cpp)
void __fastcall Fn0b304b0_983(unsigned long uid, sCharacterInfo* pInfo); // 00b304b0 (jp/pktviews983.cpp): indice + ficha ao Gateway
// 983 007acdb0: UI pointers at 0x4c8..0x4e4 (ResetFrWndPointers 007af9b0); the old 0x46c..0x488 read garbage (AV in the Lounge, d449-3)
void ntAvatarChatMain::HandleMsg(const MsgObject& msg)
{
	CSharedDoc* doc = CSharedDoc::Instance();

	if (g_CurrentTime < msg.time)
	{
		StoreDelayedMsg(msg);
		return;
	}

	ntAvatarManager* mgr;
	ntAvatar* avatar;

	switch (msg.message)
	{
	// D641 (983 007aeb9d): a janela de emocoes do Lounge (FrAvatarMotionDlg FLAG_CHAT) pede o esqueleto do meu boneco
	case 0x198:
		if (ntAvatarManager::Instance()->m_pMyAvatar && ntAvatarManager::Instance()->m_pMyAvatar->GetPetFrame983())
			const_cast<MsgObject&>(msg).param1 = (int)ntAvatarManager::Instance()->m_pMyAvatar->GetPetFrame983();
		break;
	// D641 (983 007aefb2): toca a emocao escolhida; pose basica = tipo 0, conjunto 007a8940 (tipo 5 so no kind 1), senao 4
	case 0x199:
		if (msg.param1 && ntAvatarManager::Instance()->m_pMyAvatar)
		{
			const char* motion = (const char*)msg.param1;
			avatar = ntAvatarManager::Instance()->FindAvatarByNickName(
				ntAvatarManager::Instance()->m_pMyAvatar->GetNickName());
			if (!avatar)
				break;
			unsigned int kind = 0;
			if (!strcmp(motion, "\xb1\xe2\xba\xbb\xc0\xda\xbc\xbc"))
				avatar->ChangeMotion(0, motion, 0.2f);
			else if (CreateCharacterOption::Instance()->FindSetMotionExact983(motion, &kind))
			{
				int type = 0;
				switch (kind)
				{
				case 0:
				case 2:
				case 3:
				case 4:
					type = 4;
					break;
				case 1:
					type = 5;
					break;
				}
				if (!avatar->IsHoldState())
					avatar->ChangeMotion(type, motion, 0.2f);
			}
			else
				avatar->ChangeMotion(4, motion, 0.2f);
		}
		break;
	// D788 (983 007aeb1b): chat com "/" + comando de conjunto (ntAvatarChatTask::OnPacket 007b6375 manda (nome, frase));
	// nome nulo = o proprio jogador; toca o movimento se o avatar veste a peca do conjunto (004d0c40) -> 007a8940
	case 0x115:
		{
			const char* nick = (const char*)msg.param1;
			if (!nick && ntAvatarManager::Instance()->m_pMyAvatar)
				nick = ntAvatarManager::Instance()->m_pMyAvatar->GetNickName();
			avatar = ntAvatarManager::Instance()->FindAvatarByNickName(nick);
			if (!avatar)
				break;
			char motion[0x20] = { 0 };
			unsigned int kind = 0;
			if (CreateCharacterOption::Instance()->FindEquippedSetCommand983(
				(const char*)msg.param2, motion, &kind, &avatar->m_charInfo))
			{
				int type = 0;
				switch (kind)
				{
				case 0:
				case 2:
				case 3:
				case 4:
					type = 4;
					break;
				case 1:
					type = 5;
					break;
				}
				if (!avatar->IsHoldState())
					avatar->ChangeMotion(type, motion, 0.2f);
			}
		}
		break;
	// D789 (983 007ace2c): abre a widget `param1` por cima do Lounge (ex.: rmrMode 1 -> "CMyRoomWidget", "AVATARCHAT_MAIN",
	// taskmain.cpp); "RFLobbyDesktop" (00d08820) nao abre nada; [00e73e60] = CTaskManager -> 009bf6e0
	case 0x88:
		{
			std::string name((const char*)msg.param1);
			if (name != "RFLobbyDesktop")
			{
				std::string loadingMsg((const char*)msg.param2);
				if (CTaskManager::Instance())
					CTaskManager::Instance()->OpenWidget983(name.c_str(), loadingMsg.c_str());
			}
		}
		break;
	case 0:
		// 983 (007ada9d; C199): a sala 3D marca g_lobbyModal983 = 1 (lido pela troca de personagem 005d2ac0 -> 0x000c sub 4 e
		// pelas lojas/ranking com o ntAvatarManager); com a area de trabalho da RF ja aberta nao troca de layout.
		g_lobbyModal983 = 1;
		if (RFLobbyDesktop::Instance983())
			return;
		{
			RFLobbyDesktop* pDesktop = RFLobbyDesktop::Create983(NULL);
			if (pDesktop)
				pDesktop->OnShowDesktop983(true); // vt+0x88
		}
		if (!g_pFresh)
			return;
		g_pFresh->OpenLayout((char*)msg.param1,
			this ? (FrCmdTarget*)(this) : (FrCmdTarget*)0, false);
		return;
	case M983(0x31):
	case M983(0x34):
	case M983(0x3c):
		return;
	case M983(0xf):
	{
		std::list<sSlotInfo>::iterator it;
		for (it = doc->m_slotList.begin(); it != doc->m_slotList.end(); ++it)
		{
			if (it->bMaster & 1)
			{
				m_ul4c4 = it->dwGuid;
				break;
			}
		}
	}
		return;
	case M983(0x16):
		m_b454 = msg.param1 != 0;
		return;
	case M983(0x17):
		*(unsigned int*)msg.param1 = (unsigned int)m_b454;
		return;
	case M983(0x27):
		strcpy(m_roomPassword, (const char*)msg.param1);
		return;
	case M983(0x28):
		*(char**)msg.param1 = m_roomPassword; // 983 0x3a4 (era 0x350 do 645 = sobre ponteiros de icones)
		return;
	case M983(0x37):
		if (CUserInfo::Instance()->IsVisible())
			CUserInfo::Instance()->Close();
		if (CSharedDoc::Instance()->AddWhisperPartner(
				m_pChatTarget, (const char*)msg.param1))
			m_pChatTarget
				->SetLine(1, (const char*)msg.param1, 0, false, 0);
		if (CChatMsg::Instance()->IsClosed())
			CChatMsg::Instance()->Open(true, true);
		if (m_pChatInput)
			m_pChatInput->SetKeyFocus(true);
		return;
	case M983(0x38):
		CSharedDoc::Instance()->AddWhisperPartner(m_pChatTarget,
			(const char*)msg.param1);
		return;
	case M983(0x3b):
		switch (msg.param1)
		{
		case 2:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xc1\xa4\xbf\xf8\xc0\xcc \xc3\xca\xb0\xfa \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 3:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xc1\xb8\xc0\xe7\xc7\xcf\xc1\xf6 \xbe\xca\xb4\xc2 \xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 4:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xba\xf1\xb9\xd0\xb9\xf8\xc8\xa3\xb0\xa1 \xc6\xb2\xb7\xc8\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 5:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb7\xb9\xba\xa7\xbf\xa1\xbc\xad\xb4\xc2 \xb9\xe6\xc0\xbb \xb8\xb8\xb5\xe9\xb0\xc5\xb3\xaa \xb9\xe6\xbf\xa1 \xc0\xd4\xc0\xe5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			return;
		case 7:
		case 9:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xb9\xe6\xb8\xb8\xb5\xe9\xb1\xe2 \xbf\xe4\xc3\xbb\xc0\xcc \xbd\xc7\xc6\xd0\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 8:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1 \xb0\xa1\xc0\xd4\xc7\xd8\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			return;
		case 0xd:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xce \xb9\xe6\xc0\xd4\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 0xf:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xc6\xce\xb9\xe8\xc6\xb2\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9"),
				0, 0, 0, 0);
			return;
		case 0x11:
			(IActor*)this << MsgObject(0, M983(0x23),
				(int)K2L_Compatibility("\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1\xb0\xd4\xc0\xd3\xbf\xa1 \xc7\xca\xbf\xe4\xc7\xd1 \xc6\xce\xc0\xcc \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			return;
		default:
			return;
		}
	case M983(0x3d):
	{
		std::list<sSlotInfo>::iterator it = std::find(doc->m_slotList.begin(),
			doc->m_slotList.end(), msg.param1);
		if (it == doc->m_slotList.end())
			return;
		it->bSleep = (msg.param2 == 1);
	}
		return;
	case M983(0x3f):
		if (m_pChatInput)
			*(const char**)msg.param1 =
				m_pChatInput->GetLine(1, false);
		else
			*(const char**)msg.param1 = 0;
		return;
	case M983(0x47):
		CSharedDoc::Instance()->InitRecentWhisperList(
			m_pChatTarget);
		return;
	case M983(0x49):
	{
		sCharacterInfo* newInfo = (sCharacterInfo*)msg.param2;
		avatar = ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		// D792: 983 007acdb0 caso 0x48 repassa antes de tudo ao form da widget (My Room aberto no Lounge, D789)
		if (FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983())
			pWidgetForm->OnMsg983(msg);
		if (avatar)
		{
			{
				ntAvatarBurden burden(avatar->m_uid, avatar->m_guid,
					avatar->m_charInfo, (CPetFrame*)NULL /* D465: 983 passa [avatar+4] (AvatarBase, nulo); o frame vivo do avatar nao pode ir ao DeleteBurdenPet */);
				((ntAvatarChatTask*)GetTask())->RemoveAvatarBurden(burden);
			}
			avatar->SetCharactorInfo(newInfo);
			Fn0b304b0_983(avatar->m_uid, newInfo); // D910: 983 007acf83 (uid +0x148, ficha nova) entre SetCharactorInfo e AddAvatarBurden
			{
				ntAvatarBurden burden(avatar->m_uid, avatar->m_guid, *newInfo, 0);
				((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
			}
			if (!IsLocalContent((localContentType_t)10) ||
				!BRIEF_INFO(avatar->m_guid).IsIdentity(4) ||
				(BRIEF_INFO(avatar->m_guid).state & 1))
			{
				TALK_ROOM_FX(avatar->m_pos);
			}
			if (avatar == ntAvatarManager::Instance()->m_pMyAvatar)
			{
				doc->m_myInfo.userEquip.guidChar = newInfo->guid; // 983 doc+0xa2a
				ChangePortraitImage(
					ntAvatarManager::Instance()->m_pMyAvatar->m_ec &
					0x3ffffff);
			}
		}
		RefreshMotionUi983(); // D794: 983 007ad0ff roda sempre (fora do teste do avatar)
	}
		return;
	// D792: 983 007acdb0 casos 0x49/0x4a/0x4b/0x5b: do proprio avatar grava o equipamento em m_myInfo.userEquip (doc+0xa26/
	// 0xa2e/0xa32/0xa8e; os 0x628/0x630/0x690 do 645 caiam no CItemManager) e SEMPRE repassa ao form da widget (My Room aberto
	// no Lounge: caddie/taco/bola/mascote selecionados). O nosso so retornava.
	case M983(0x4a):
	case M983(0x4b):
	case M983(0x4c):
	case M983(0x4e):
		avatar = ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (avatar && avatar == ntAvatarManager::Instance()->m_pMyAvatar)
		{
			if (msg.message == M983(0x4a))
				doc->m_myInfo.userEquip.guidCaddie = *(unsigned long*)msg.param2;
			else if (msg.message == M983(0x4b))
				doc->m_myInfo.userEquip.guidClubSet = *(unsigned long*)msg.param2;
			else if (msg.message == M983(0x4c))
				doc->m_myInfo.userEquip.tidBall = msg.param2;
			else
				doc->m_myInfo.userEquip.guidMascot = *(unsigned long*)msg.param2;
		}
		if (msg.message == M983(0x4e))
			doc->SendEquipItemSlot(); // D793: 983 007ad342 chama 0063de40 sempre (fora do teste do proprio avatar)
		if (FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983())
			pWidgetForm->OnMsg983(msg);
		return;
	case M983(0x5b):
		avatar = ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (!avatar || avatar == ntAvatarManager::Instance()->m_pMyAvatar)
			return;
		switch (msg.param2)
		{
		case 4:
		{
			WVector pos(*(float*)msg.param3, 0.0f, *(float*)msg.param4);
			pos = GetPVS().GetGroundPoint(pos, false, 0, false);
			avatar->SetPosition(pos);
			avatar->SetAngle(*(float*)msg.param5);
			if (!IsLocalContent((localContentType_t)10))
			{
				TALK_ROOM_FX(pos);
				return;
			}
			if (!BRIEF_INFO(msg.param1).IsIdentity(4))
				return;
			if (!(BRIEF_INFO(msg.param1).state & 1))
				return;
			TALK_ROOM_FX(avatar->m_pos);
		}
			return;
		case 5:
			avatar->SetAction(msg.param3);
			return;
		case 6:
		{
			WVector pos(avatar->m_targetPos.x + *(float*)msg.param3,
				avatar->m_targetPos.y,
				avatar->m_targetPos.z + *(float*)msg.param4);
			avatar->SetTargetPosition(pos);
			avatar->SetPendingTargetAngle(*(float*)msg.param5);
		}
			return;
		case 7:
			avatar->ChangeMotion(4, (const char*)msg.param3, 0.2f);
			return;
		case 8:
			avatar->SetState(msg.param3);
			return;
		case 9:
			if (doc->m_briefUserInfoMap.find(msg.param1) ==
				doc->m_briefUserInfoMap.end())
				return;
			if (m_pUserListDlg)
				m_pUserListDlg->UpdateUserList();
			return;
		case 10: // 983 007af02c: movimento de conjunto de outro jogador (ChangeMotion 5 -> 0x63 sub 10)
			avatar->ChangeMotion(5, (const char*)msg.param3, 0.2f);
			return;
		default:
			return;
		}
	case M983(0x5f):
		if (msg.param2 != 0xffff)
			return;
		{
			std::list<sSlotInfo>::iterator it;
			for (it = doc->m_slotList.begin(); it != doc->m_slotList.end();
				++it)
			{
				if (it->dwGuid == (unsigned long)msg.param1)
				{
					it->bMaster |= 1;
					it->bReady = 1;
				}
				else
					it->bMaster &= ~1;
			}
		}
		m_ul4c4 = msg.param1;
		return;
	case M983(0x60):
	{
		const char* nick = (const char*)msg.param1;
		const char* text = (const char*)msg.param2;
		if (!nick && ntAvatarManager::Instance()->m_pMyAvatar)
			nick = ntAvatarManager::Instance()->m_pMyAvatar->m_nickName;
		avatar = ntAvatarManager::Instance()->FindAvatarByNickName(nick);
		if (!avatar)
			return;
		if (SpCommand(text))
			return;
		int face = doc->m_chatManager.GetFaceIndex(text);
		if (face != 5)
			avatar->SetChatFacial(face);
		const char* motion =
			doc->m_chatManager.GetMotionName(text, avatar->m_ec); // 983 007aa040: +0x154, indice da tabela; m_uid (+0x148) lia fora dela
		if (!motion)
			return;
		if (avatar->m_state & 0x7e)
			return;
		avatar->ChangeMotion(4, motion, 0.2f);
	}
		return;
	case M983(0x62):
		if (m_pChatFrame)
			m_pChatFrame->SetVisible(msg.param1 != 0);
		if (m_pChatTarget)
			m_pChatTarget->SetVisible(msg.param1 != 0);
		if (m_pChatInput)
		{
			if (msg.param1)
			{
				m_pChatInput->SetVisible(true);
				m_pChatInput->SetKeyFocus(true);
			}
			else
				m_pChatInput->SetVisible(false);
			g_pFresh->GetManager()->SetExclusiveKey(msg.param1 != 0);
		}
		if (m_pEmoticonBtn)
			m_pEmoticonBtn->SetVisible(msg.param1 != 0);
		if (m_pLanguage)
			m_pLanguage->SetVisible(msg.param1 != 0);
		if (m_pReportBtn)
			m_pReportBtn->SetVisible(msg.param1 != 0);
		return;
	case M983(0x63):
		if (!m_pChatView)
			return;
		m_pChatView->ClearLine();
		{
			int n = 8;
			do
			{
				m_pChatView->AddLine("", 0, false);
				n--;
			} while (n);
		}
		return;
	case M983(0x64):
		// 983 007ad9c7/007ad9cd: DelItem no scrollbar de FrWnd+0x90 (GetScrollBar); o +0x7c lia lixo (D697: AV 009b6150 na 1a fala do Lounge, d696-1)
		if (m_pChatView)
		{
			if (m_pChatView->GetLineNum() > 0x32)
			{
				m_pChatView->DeleteFirstLine();
				m_pChatView->GetScrollBar()->DelItem();
			}
			m_pChatView
				->AddLine((const char*)msg.param1, msg.param2, false, 2); // 983 007ad9f7: style 2
			if (m_i4bc > 0)
			{
				m_pChatView->DeleteFirstLine();
				m_pChatView->GetScrollBar()->DelItem();
				m_i4bc--;
			}
		}
		if (!m_pChatLogDlg)
			return;
		m_pChatLogDlg->AddChatMsg((const char*)msg.param1, msg.param2);
		return;
	case M983(0x65):
		if (!m_pChatTarget)
			return;
		if (doc->m_recentWhisperList.empty())
			return;
		m_pChatTarget
			->SetLine(1, doc->m_recentWhisperList.front().c_str(), 0, false, 0);
		return;
	case M983(0x69):
		// D701: 983 007acea1 usa o botao do jukebox (this+0x4b4 = m_pJukeBoxBtn do avatar_mainui, init 007b02e5); m_pJukeBoxBtn nunca era atribuido
		if (!m_pJukeBoxBtn)
			return;
		m_pJukeBoxBtn->SetButtonImg(msg.param1 == 1 ? "btn_jukebox_o"
												: "btn_jukebox_n",
			FrButton::NORMAL);
		return;
	case M983(0xe4):
	{
		sSlotInfo* slot = (sSlotInfo*)msg.param1;
		sCharacterInfo* info = (sCharacterInfo*)msg.param2;
		avatar = ntAvatarManager::Instance()->FindAvatarByNickName(slot->sNick);
		if (!avatar)
		{
			if (m_pUserListDlg)
				m_pUserListDlg->UpdateUserList();
			if (ntAvatarManager::Instance()->m_pMyAvatar)
			{
				if (!IsLocalContent((localContentType_t)10) ||
					!BRIEF_INFO(slot->dwGuid).IsIdentity(4) ||
					(BRIEF_INFO(slot->dwGuid).state & 1))
				{
					std::string text = MakeStr(
						K2L_Compatibility("%s \xb4\xd4\xc0\xcc \xc0\xd4\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
						slot->sNick);
					CChatMsg::Instance()->AddChatMsg(text, 0xffff00, false,
						false);
				}
			}
			Fn0b304b0_983(slot->dwUserUID, info); // D910: 983 007adffb (slot +0x6c, ficha) antes do 007a8680
			avatar =
				ntAvatarManager::Instance()->CreateAvatar(slot, info, this);
			if (!avatar)
				return;
			{
				ntAvatarBurden burden(slot->dwUserUID, slot->dwGuid, *info, 0);
				((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
			}
			if (slot->dwGuid != MyGuid(false))
			{
				WVector pos(slot->location[0], 0.0f, slot->location[1]);
				WVector ground = GetPVS().GetGroundPoint(pos, false, 0, false);
				avatar->SetPosition(ground);
				avatar->SetAngle(slot->location[2]);
			}
			else
			{
				ntAvatarManager::Instance()->m_pMyAvatar = avatar;
				doc->m_myInfo.userEquip.guidChar = info->guid; // 983 007ae0a1: doc+0xa2a
				doc->SendTradeCloseShop();
				RbPolySoupPoint* point =
					((RbPolySoup*)doc->m_pGolfDoc->m_pPolySoup)
						->FindPoint(MakeStr(POINT_START_NAME));
				if (!point)
					point = ((RbPolySoup*)doc->m_pGolfDoc->m_pPolySoup)
								->FindPoint(MakeStr(POINT_START_NAME2));
				WVector pos = point->pos;
				pos.x += Random(-16.0f, 16.0f);
				pos.z += Random(-16.0f, 16.0f);
				pos = GetPVS().GetGroundPoint(pos, false, 0, false);
				if (!IsLocalContent((localContentType_t)10) ||
					!BRIEF_INFO(slot->dwGuid).IsIdentity(4) ||
					(BRIEF_INFO(slot->dwGuid).state & 1))
				{
					TALK_ROOM_FX(pos);
				}
				ntAvatarManager::Instance()->m_pMyAvatar->SetPosition(pos);
				ntAvatarManager::Instance()->m_pMyAvatar->SetAngle(
					Random(0.0f, 6.2831855f));
				ntAvatarManager::Instance()->m_pMyAvatar->Stop();
				WSendPacket packet(99);
				packet.Encode1(4);
				packet.EncodeBuffer(
					&ntAvatarManager::Instance()->m_pMyAvatar->m_pos.x, 4);
				packet.EncodeBuffer(
					&ntAvatarManager::Instance()->m_pMyAvatar->m_pos.z, 4);
				packet.EncodeBuffer(
					&ntAvatarManager::Instance()->m_pMyAvatar->m_angle,
					4);
				packet.Send((eSendTo)0);
				ChangePortraitImage(
					ntAvatarManager::Instance()->m_pMyAvatar->m_ec &
					0x3ffffff);
			}
		}
		WSendPacket ack(0xeb);
		ack.Encode4(slot->dwGuid);
		ack.Send((eSendTo)0);
	}
		return;
	case M983(0xe5):
		avatar = ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (!avatar)
			return;
		if (m_pUserListDlg)
			m_pUserListDlg->UpdateUserList();
		if (ntAvatarManager::Instance()->m_pMyAvatar)
		{
			if (strcmp(avatar->m_idName, // 983 007ae61a: +0x6c
					ntAvatarManager::Instance()->m_pMyAvatar->m_idName) == 0)
			{
				doc->SendTradeCloseShop();
				ntAvatarManager::Instance()->m_pMyAvatar->SubState(0x7e);
				ntAvatarManager::Instance()->m_pMyAvatar = 0;
			}
			if (!IsLocalContent((localContentType_t)10) ||
				!BRIEF_INFO(avatar->m_guid).IsIdentity(4) ||
				(BRIEF_INFO(avatar->m_guid).state & 1))
			{
				std::string text = MakeStr(
					K2L_Compatibility("%s \xb4\xd4\xc0\xcc \xc5\xf0\xc0\xe5\xc7\xdf\xbd\xc0\xb4\xcf\xb4\xd9."),
					avatar->m_nickName);
				CChatMsg::Instance()->AddChatMsg(text, 0xffff00, false, false);
			}
		}
		if (!IsLocalContent((localContentType_t)10) ||
			!BRIEF_INFO(avatar->m_guid).IsIdentity(4) ||
			(BRIEF_INFO(avatar->m_guid).state & 1))
		{
			TALK_ROOM_FX(avatar->m_pos);
		}
		{
			ntAvatarBurden burden(avatar->m_uid, avatar->m_guid,
				avatar->m_charInfo, (CPetFrame*)-1);
			((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
		}
		{
			ntAvatarBurden burden(avatar->m_uid, avatar->m_guid,
				avatar->m_charInfo, (CPetFrame*)NULL /* D465: 983 passa [avatar+4] (AvatarBase, nulo); o frame vivo do avatar nao pode ir ao DeleteBurdenPet */);
			((ntAvatarChatTask*)GetTask())->RemoveAvatarBurden(burden);
		}
		ntAvatarManager::Instance()->DeleteAvatar(avatar->m_idName); // 983 007ae94a: 007a3350 compara +0x6c
		return;
	case M983(0xe6):
		ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		return;
	case M983(0xe7):
		if (!ntAvatarManager::Instance()->m_pMyAvatar)
			return;
		strcpy(ntAvatarManager::Instance()->m_pMyAvatar->m_nickName,
			(const char*)msg.param1);
		return;
	case M983(0xe8):
		avatar = ntAvatarManager::Instance()->FindAvatarByNickName(
			(const char*)msg.param1);
		if (!avatar)
			return;
		switch (msg.param2)
		{
		case 0:
		case 2:
			avatar->SubState(0x180);
			return;
		case 1:
			avatar->AddState(0x80);
			return;
		case 3:
			avatar->SubState(0x80);
			avatar->AddState(0x100);
			return;
		default:
			return;
		}
	case M983(0xe9):
		avatar = ntAvatarManager::Instance()->FindAvatarByIDName(
			(const char*)msg.param1);
		if (!avatar)
			return;
		avatar->SetTradeTitle((const char*)msg.param2);
		return;
	case M983(0x1f9):
	{
		if (CQuitWindow::Instance()->IsActive())
			return;
		bool hide = m_viewMode == 2 || CProjectG::Instance()->HideGUI();
		if (g_pFresh)
			g_pFresh->show = !hide;
		// 983 007aee6f: o mesmo valor vai ao +0x40 do ponteiro da interface (00e73e4c)
		if (CUiPointer983::Instance983())
			CUiPointer983::Instance983()->m_show40 = !hide;
		hide = m_viewMode == 1 || CProjectG::Instance()->HideGUI();
		ShowOverAndUnderBar(!hide);
		hide = (m_viewMode == 2 || CProjectG::Instance()->HidePI()) &&
			!CChatMsg::Instance()->IsClosed();
		((RbChatMsg*)CChatMsg::Instance())->SetToggle(!hide);
		// 983 007aef05: +0xb1 (detalhe do overlay); o +0xb0 desliga o desenho de todos os avatares
		ntAvatarManager::Instance()->m_overlayDetail =
			!(m_viewMode != 0 || CProjectG::Instance()->HidePI());
		hide = m_viewMode == 2 || CProjectG::Instance()->HideGUI() ||
			CProjectG::Instance()->HidePI();
		CMouseCursor::Instance()->Toggle(!hide);
		hide = m_viewMode == 2 || CProjectG::Instance()->HidePI();
		// 983 007aef7c: +0x4c0 = m_pWnd46c = m_pUserListBtn (ResetFrWndPointers 007af9b0) e +0x110 = m_pOverBarPangCookie
		// (OnOverBar_PangCookieInit 00653540); os 0x4c8/0xdc do 645 caiam no form m_pDlg4c8 e no m_pOverBarTitle
		if (m_pWnd46c)
			m_pWnd46c->SetVisible(!hide);
		if (m_pOverBarPangCookie)
			m_pOverBarPangCookie->SetVisible(!hide);
	}
		return;
	case M983(0x211):
		return;
	case M983(0x24e):
	{
		int type = msg.param2;
		float value = (float)msg.param3;
		avatar = ntAvatarManager::Instance()->FindAvatarByGuid(msg.param1);
		if (!avatar)
			return;
		if (type == 0)
		{
			if (ntAvatarManager::Instance()->m_pMyAvatar->m_guid == msg.param1)
			{
				m_b4ca = true;
				g_SALoadingState = false;
			}
		}
		if (type != 3)
		{
			sCharacterInfo info = avatar->m_charInfo;
			{
				ntAvatarBurden burden(avatar->m_uid, avatar->m_guid,
					avatar->m_charInfo, (CPetFrame*)NULL /* D465: 983 passa [avatar+4] (AvatarBase, nulo); o frame vivo do avatar nao pode ir ao DeleteBurdenPet */);
				((ntAvatarChatTask*)GetTask())->RemoveAvatarBurden(burden);
			}
			avatar->SetCharactorInfo(&info);
			{
				ntAvatarBurden burden(avatar->m_uid, avatar->m_guid,
					avatar->m_charInfo, 0);
				((ntAvatarChatTask*)GetTask())->AddAvatarBurden(burden);
			}
		}
		avatar->GetMySpItem((eSPAVILITYTYPE)type).changeflag();
		avatar->GetMySpItem((eSPAVILITYTYPE)type).setvalue(value);
		if (IsLocalContent((localContentType_t)10))
		{
			if (BRIEF_INFO(avatar->m_guid).IsIdentity(4) &&
				!(BRIEF_INFO(avatar->m_guid).state & 1))
				return;
		}
		StartSpEffect(avatar, (eSPAVILITYTYPE)type);
	}
		return;
	default:
	{
		// 983 (007aebe5): a widget da tarefa (My Room aberto no Lounge) trata primeiro; falso -> HandleMsgCommon
		FrTaskForm* pWidgetForm = AfxGetTask()->GetWidgetForm983();
		if (pWidgetForm == NULL || !pWidgetForm->OnMsg983(msg))
			CTaskMain::HandleMsgCommon(msg);
	}
		return;
	}
}
#undef BRIEF_INFO
#undef TALK_ROOM_FX

void ntAvatarChatMain::RefreshAllUccClothes()
{
	sUccRefreshItem item;
	for (int tries = 0; tries < 10; tries++)
	{
		if (!UccManager()->GetRefreshItem(item))
		{
			return;
		}
		int found = 0;
		for (int i = 0; i < 30; i++)
		{
			if (ntAvatarManager::Instance()->m_count <= found)
			{
				break;
			}
			ntAvatar* avatar = ntAvatarManager::Instance()->m_avatars[i];
			if (avatar)
			{
				found++;
				unsigned int slot = (item.typeId >> 13) & 0x1f;
				if (avatar->m_partTids.m_tid[slot] == item.typeId)
				{
					sUccClothes* clothes =
						UccManager()->FindClothes(avatar->m_partTids.m_uid[slot]);
					if (clothes &&
						_strcmpi(clothes->uccIndex, item.uccIndex) == 0)
					{
						avatar->RefreshUccClothes(clothes);
					}
				}
			}
		}
	}
}

void ntAvatarChatMain::StartSpEffect(ntAvatar* avatar, eSPAVILITYTYPE type)
{
	CFxSequence* fx = NULL;
	bool flag = avatar->GetMySpItem(type).getflag();
	switch (type)
	{
	case 0:
		break;
	case 1:
		if (flag)
		{
			if (avatar->GetMySpItem((eSPAVILITYTYPE)0).getflag())
			{
				fx = CFx::Instance()->OpenSequence("change_up-big.seq", true);
			}
			else
			{
				fx =
					CFx::Instance()->OpenSequence("change_up-middle.seq", true);
			}
		}
		else
		{
			if (avatar->GetMySpItem((eSPAVILITYTYPE)0).getflag())
			{
				fx = CFx::Instance()->OpenSequence("change_up-big.seq", true);
			}
			else
			{
				fx = CFx::Instance()->OpenSequence("change_up-small.seq", true);
			}
		}
		goto spawned;
	case 2:
		flag = avatar->GetMySpItem((eSPAVILITYTYPE)0).getflag();
		break;
	case 3:
	{
		float v = avatar->GetMySpItem((eSPAVILITYTYPE)0).getvalue() * 10.0f;
		fx = CFx::Instance()->OpenSequence("BFX_B2.seq", true);
		if (fx)
		{
			fx->Pos() = WVector(avatar->m_pos.x + 1.0f, v + avatar->m_pos.y,
				avatar->m_pos.z + 1.0f);
		}
		fx = CFx::Instance()->OpenSequence("BFX_B3.seq", true);
		if (fx)
		{
			fx->Pos() = WVector(avatar->m_pos.x + 1.0f, v + avatar->m_pos.y,
				avatar->m_pos.z + 1.0f);
		}
		g_audio->PlaySfx("Ball_B", 0, 0, 0, 0, 0.5f, 200.0f);
		return;
	}
	default:
		goto spawned;
	}
	if (flag)
	{
		fx = CFx::Instance()->OpenSequence("change_up-big.seq", true);
	}
	else if (avatar->GetMySpItem((eSPAVILITYTYPE)1).getflag())
	{
		fx = CFx::Instance()->OpenSequence("change_up-middle.seq", true);
	}
	else
	{
		fx = CFx::Instance()->OpenSequence("change_up-small.seq", true);
	}
spawned:
	if (fx)
	{
		fx->Pos() =
			WVector(avatar->m_pos.x, avatar->m_pos.y + 5.0f, avatar->m_pos.z);
	}
}

static const char s_spCommandNames[5][0x40] = {
	"\xb0\xc5\xc0\xce",
	"\xbf\xd5\xb8\xd3\xb8\xae",
	"\xb1\xa4\xbc\xd3",
	"\xb9\xdd\xc2\xa6\xc0\xcc",
	"",
};

bool ntAvatarChatMain::SpCommand(char const* cmd)
{
	volatile bool handled = false;
	static DWORD s_lastTime = timeGetTime() - 3500;

	if (strlen(cmd + 1) == 0 || cmd[0] != '/')
		return false;

	int type = 0;
	for (const char* name = s_spCommandNames[0];
		(int)name < (int)s_spCommandNames[5]; name += 0x40, ++type)
	{
		if (strcmp(cmd + 1, name) != 0)
			continue;

		handled = true;
		// 983 007abec0: espera de 3500 ms vale para TODOS os comandos
		if (timeGetTime() - s_lastTime < 3500)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9")),
				0xffff7878, true, false);
			return true;
		}
		s_lastTime = timeGetTime();
		if (CanUseSPItem((eSPAVILITYTYPE)type))
		{
			ntAvatarManager::Instance()->m_pMyAvatar->UseSPItem(
				(eSPAVILITYTYPE)type);
			return handled;
		}
		CChatMsg::Instance()->AddChatMsg(
			std::string(
				K2L_Compatibility("\xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc2\xf8\xbf\xeb\xc7\xcf\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9")),
			0xffff7878, true, false);
		return handled;
	}
	return handled;
}

bool ntAvatarChatMain::CanUseSPItem(eSPAVILITYTYPE type)
{
	ntAvatar* avatar = ntAvatarManager::Instance()->m_pMyAvatar;
	int i;
	for (i = 0; i < 24; i++)
	{
		unsigned long tid = avatar->m_charInfo.tidParts[i];
		if (tid != 0 &&
			CSharedDoc::Instance()->m_itemManager.FindSPItem(tid, type) != NULL)
		{
			return true;
		}
	}
	for (i = 0; i < 5; i++)
	{
		unsigned long tid = avatar->m_charInfo.tidAuxParts[i];
		if (tid != 0 &&
			CSharedDoc::Instance()->m_itemManager.FindSPItem(tid, type) != NULL)
		{
			return true;
		}
	}
	return false;
}

// JP 983 (s158): nomes que este arquivo cita por uma visao (ou com a assinatura um pouco diferente: ponteiro x referencia,
// const, int x unsigned long, void x bool; ABI igual no x86) e que nao existiam com esse nome -> viravam stub no exe.
// Cada um aponta para a funcao/dado real (lista e motivo em out/F3.5/s158/apelidos.tsv).
#pragma comment(linker, "/alternatename:?AvatarByRayIntersect@ntAvatarManager@@QAEPAVntAvatar@@AAVWVector@@0@Z=?AvatarByRayIntersect@ntAvatarManager@@QBEPAVntAvatar@@ABVWVector@@0@Z") // const/ref const
#pragma comment(linker, "/alternatename:?ChangePortraitImage@ntAvatarChatMain@@AAEXK@Z=?ChangePortraitImage@ntAvatarChatMain@@IAEXH@Z") // unsigned long x int
#pragma comment(linker, "/alternatename:?CheckClick@ntAvatarManager@@QAEPAVntAvatar@@MM@Z=?CheckClick@ntAvatarManager@@QBEPAVntAvatar@@MM@Z") // const
#pragma comment(linker, "/alternatename:?CreateAvatar@ntAvatarManager@@QAEPAVntAvatar@@PAUsSlotInfo@@PAUsCharacterInfo@@PAVntAvatarChatMain@@@Z=?CreateAvatar@ntAvatarManager@@QAEPAVntAvatar@@ABUsSlotInfo@@ABUsCharacterInfo@@PAVntAvatarChatMain@@@Z") // ponteiro x ref
#pragma comment(linker, "/alternatename:?DeleteAvatar@ntAvatarManager@@QAEXPBD@Z=?DeleteAvatar@ntAvatarManager@@QAE_NPBD@Z") // void x bool
#pragma comment(linker, "/alternatename:?GetFollowingAvatar@ntAvatar@@QAEPAV1@XZ=?GetFollowingAvatar@ntAvatar@@QBEPAV1@XZ") // const
#pragma comment(linker, "/alternatename:?SetAction@ntAvatar@@QAEXK@Z=?SetAction@ntAvatar@@QAEXH@Z") // unsigned long x int
#pragma comment(linker, "/alternatename:?SetCharactorInfo@ntAvatar@@QAEXPBUsCharacterInfo@@@Z=?SetCharactorInfo@ntAvatar@@QAEXABUsCharacterInfo@@@Z") // ponteiro x ref
