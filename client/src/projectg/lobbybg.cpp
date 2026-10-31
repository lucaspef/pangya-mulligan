#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "golfdoc.h"
#include "sky.h"
#include "shadow.h"
#include "wind.h"
#include "fx.h"
#include "quadtree.h"
#include "polysoup.h"
#include "scenemanager.h"
#include "clientsetting.h"
#include "lobbybg.h"
#include "mathconsts.h"

#include <math.h>
#include "jp/lobbyjp.h"
#include "jp/refresh/rflobbydesktop.h"
#include "wpuppet.h"

extern bool g_bQuit;

// 983: LOBBYBG_MAP (00f02f9c) e LOBBYBG_MAPNAME (00e36afc) so mudam pela opcao de depuracao do CProjectG::Ready; o
// cenario do lobby vem do arquivo fixo abaixo e do nome do mapa no ator (msg 0x17a; "west" pelo CLobbyMain::OnPreLoadInit).
int LOBBYBG_MAP = 0;
char* LOBBYBG_MAPNAME = "blue";
static std::string s_lobbyGbin983 = "lobby_west.gbin";                         // 00e36b00
static std::string s_posterTex983[2] = {"+2#_poster01.jpg", "+2#_poster02.jpg"}; // 00e36b1c, 00e36b38

C_ASSERT_VC10(sizeof(ntLobbyBg) == 0x70);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.poster) == 0xa92); // 00ab2640 caso 0x182
C_ASSERT(offsetof(IFF_STRUCT::sFurniture, Tex) == 0x104);            // 00ab1bf0 "%s.jpg"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CameraMovingHelper (983 00485320-00486700)
extern const WVector g_vZero983_f041c4; // 00f041c4
extern const WVector g_vOne983_e42520;  // 00e42520 (1, 1, 1)

// 00485360: despacha o tipo (0..0x15) para as curvas de Penner do 983 (004db270-004dba80, uma funcao por curva)
static const float HALF_PI983 = 1.5707964f;
// 004db270
__declspec(noinline) static float EaseLinear983(float b, float e, float t, float d)
{
	return (e - b) * t / d + b;
}

// 004db2a0
__declspec(noinline) static float EaseQuadIn983(float b, float e, float t, float d)
{
	float u = t / d;
	float c = e - b;
	t = c * u * u + b;
	return t;
}

// 004db2e0
__declspec(noinline) static float EaseQuadOut983(float b, float e, float t, float d)
{
	float u = t / d;
	float c = e - b;
	t = b - c * u * (u - 2.0f);
	return t;
}

// 004db320
__declspec(noinline) static float EaseQuadInOut983(float b, float e, float t, float d)
{
	t /= d * 0.5;
	float c = e - b;
	if (t < 1.0f)
	{
		t = c * 0.5 * t * t + b;
		return t;
	}
	t -= 1.0f;
	t = -c * 0.5 * (t * (t - 2.0f) - 1.0f) + b;
	return t;
}

// 004db790
__declspec(noinline) static float EaseSineIn983(float b, float e, float t, float d)
{
	const float c = e - b;
	return (c - cosf(t / d * HALF_PI983) * c) + b;
}

// 004db7e0
__declspec(noinline) static float EaseSineOut983(float b, float e, float t, float d)
{
	const float c = e - b;
	return b + c * sinf(t / d * HALF_PI983);
}

// 004db820
__declspec(noinline) static float EaseSineInOut983(float b, float e, float t, float d)
{
	const float c = e - b;
	return b - c * 0.5f * (cosf(3.1415927f * t / d) - 1.0f);
}

// 004db880
__declspec(noinline) static float EaseExpoIn983(float b, float e, float t, float d)
{
	const float c = e - b;
	return b + c * powf(2.0f, 10.0f * (t / d - 1.0f));
}

// 004db8d0
__declspec(noinline) static float EaseExpoOut983(float b, float e, float t, float d)
{
	float c = e - b;
	return b + c * (1.0f - powf(2.0f, t / d * -10.0));
}

// 004db920
__declspec(noinline) static float EaseExpoInOut983(float b, float e, float t, float d)
{
	float u = t / (d * 0.5);
	float c = e - b;
	if (u < 1.0f)
	{
		b = c * 0.5 * powf(2.0f, 10.0 * (u - 1.0)) + b;
		return b;
	}
	b = c * 0.5 * (2.0 - powf(2.0f, -10.0 * (u - 1.0))) + b;
	return b;
}

// 004db9d0
__declspec(noinline) static float EaseCircIn983(float b, float e, float t, float d)
{
	const float c = e - b;
	t /= d; return b - c * (sqrtf(1.0f - t * t) - 1.0f);
}

// 004dba30
__declspec(noinline) static float EaseCircOut983(float b, float e, float t, float d)
{
	t /= d;
	t -= 1.0f;
	t = sqrtf(1.0f - t * t);
	float c = e - b;
	t = c * t + b;
	return t;
}

// 004dba80
__declspec(noinline) static float EaseCircInOut983(float b, float e, float t, float d)
{
	t = t / (d * 0.5);
	float c = e - b;
	if (t < 1.0f)
	{
		b = b - c * 0.5 * (sqrtf(1.0f - t * t) - 1.0);
		return b;
	}
	t -= 2.0f;
	b = c * 0.5 * (sqrtf(1.0f - t * t) + 1.0) + b;
	return b;
}

// 004db3a0
__declspec(noinline) static float EaseCubicIn983(float b, float e, float t, float d)
{
	float u = t / d;
	float c = e - b;
	t = c * u * u * u + b;
	return t;
}

// 004db3e0
__declspec(noinline) static float EaseCubicOut983(float b, float e, float t, float d)
{
	t /= d;
	t -= 1.0f;
	float c = e - b;
	t = c * (t * t * t + 1.0f) + b;
	return t;
}

// 004db430
__declspec(noinline) static float EaseCubicInOut983(float b, float e, float t, float d)
{
	t /= d * 0.5;
	float c = e - b;
	if (t < 1.0f)
	{
		t = c * 0.5 * t * t * t + b;
		return t;
	}
	t -= 2.0f;
	t = c * 0.5 * (t * t * t + 2.0f) + b;
	return t;
}

// 004db4b0
__declspec(noinline) static float EaseQuartIn983(float b, float e, float t, float d)
{
	float u = t / d;
	float c = e - b;
	t = c * u * u * u * u + b;
	return t;
}

// 004db4f0
__declspec(noinline) static float EaseQuartOut983(float b, float e, float t, float d)
{
	t /= d;
	t -= 1.0f;
	float c = e - b;
	t = b - c * (t * t * t * t - 1.0f);
	return t;
}

// 004db540
__declspec(noinline) static float EaseQuartInOut983(float b, float e, float t, float d)
{
	t /= d * 0.5;
	float c = e - b;
	if (t < 1.0f)
	{
		t = c * 0.5 * t * t * t * t + b;
		return t;
	}
	t -= 2.0f;
	t = -c * 0.5 * (t * t * t * t - 2.0f) + b;
	return t;
}

// 004db5d0
__declspec(noinline) static float EaseQuintIn983(float b, float e, float t, float d)
{
	float u = t / d;
	float c = e - b;
	t = c * u * u * u * u * u + b;
	return t;
}

// 004db610
__declspec(noinline) static float EaseQuintOut983(float b, float e, float t, float d)
{
	t /= d;
	t -= 1.0f;
	float c = e - b;
	t = c * (t * t * t * t * t + 1.0f) + b;
	return t;
}

// 004db660
__declspec(noinline) static float EaseQuintInOut983(float b, float e, float t, float d)
{
	t /= d * 0.5;
	float c = e - b;
	if (t < 1.0f)
	{
		t = c * 0.5 * t * t * t * t * t + b;
		return t;
	}
	t -= 2.0f;
	t = c * 0.5 * (t * t * t * t * t + 2.0f) + b;
	return t;
}

float CameraEase983(float b, float e, float t, float d, int type)
{
	switch (type)
	{
	case 0: return EaseLinear983(b, e, t, d);
	case 1: return EaseQuadIn983(b, e, t, d);
	case 2: return EaseQuadOut983(b, e, t, d);
	case 3: return EaseQuadInOut983(b, e, t, d);
	case 4: return EaseSineIn983(b, e, t, d);
	case 5: return EaseSineOut983(b, e, t, d);
	case 6: return EaseSineInOut983(b, e, t, d);
	case 7: return EaseExpoIn983(b, e, t, d);
	case 8: return EaseExpoOut983(b, e, t, d);
	case 9: return EaseExpoInOut983(b, e, t, d);
	case 10: return EaseCircIn983(b, e, t, d);
	case 11: return EaseCircOut983(b, e, t, d);
	case 12: return EaseCircInOut983(b, e, t, d);
	case 13: return EaseCubicIn983(b, e, t, d);
	case 14: return EaseCubicOut983(b, e, t, d);
	case 15: return EaseCubicInOut983(b, e, t, d);
	case 16: return EaseQuartIn983(b, e, t, d);
	case 17: return EaseQuartOut983(b, e, t, d);
	case 18: return EaseQuartInOut983(b, e, t, d);
	case 19: return EaseQuintIn983(b, e, t, d);
	case 20: return EaseQuintOut983(b, e, t, d);
	case 21: return EaseQuintInOut983(b, e, t, d);
	}
	return 0.0f;
}

// 00485960: chegou ao alvo (|e - v| < 1e-5; os dois ramos finais do 983 so dao falso fora da tolerancia)
static bool CameraReached983(float v, float e)
{
	float d = e - v;
	if (Wabs(d) < 9.99999974737875e-06f)
		return true;
	if (e > v)
		return d <= 0.0f;
	return (v - e) <= 0.0;
}

void CameraMovingHelper::sCameraMoving::Reset()
{
	from = g_vZero983_f041c4;
	to = g_vZero983_f041c4;
	duration = g_vOne983_e42520;
	cur = g_vZero983_f041c4;
	elapsed = g_vZero983_f041c4;
	ease[0] = 0;
	ease[1] = 0;
	ease[2] = 0;
	active[0] = false;
	active[1] = false;
	active[2] = false;
}

bool CameraMovingHelper::sCameraMoving::IsDone() const
{
	return !active[0] && !active[1] && !active[2];
}

void CameraMovingHelper::sCameraMoving::SetFromTo(const WVector& f, const WVector& t)
{
	from = f;
	to = t;
}

void CameraMovingHelper::sCameraMoving::SetEase(int x, int y, int z)
{
	ease[0] = x;
	ease[1] = y;
	ease[2] = z;
}

void CameraMovingHelper::sCameraMoving::SetDuration(float d)
{
	duration.x = d;
	duration.y = d;
	duration.z = d;
}

void CameraMovingHelper::sCameraMoving::Start()
{
	active[0] = true;
	active[1] = active[2] = true;
	elapsed = g_vZero983_f041c4;
}

void CameraMovingHelper::sCameraMoving::Update(const float& dt)
{
	if (IsDone())
		return;

	if (active[0])
	{
		cur.x = CameraEase983(from.x, to.x, elapsed.x, duration.x, ease[0]);
		elapsed.x += dt;
		active[0] = !CameraReached983(cur.x, to.x) && elapsed.x < duration.x + 0.2f;
		if (!active[0])
			cur.x = to.x;
	}
	if (active[1])
	{
		cur.y = CameraEase983(from.y, to.y, elapsed.y, duration.y, ease[1]);
		elapsed.y += dt;
		active[1] = !CameraReached983(cur.y, to.y) && elapsed.y < duration.y + 0.2f;
		if (!active[1])
			cur.y = to.y;
	}
	if (active[2])
	{
		cur.z = CameraEase983(from.z, to.z, elapsed.z, duration.z, ease[2]);
		elapsed.z += dt;
		active[2] = !CameraReached983(cur.z, to.z) && elapsed.z < duration.z + 0.2f;
		if (!active[2])
			cur.z = to.z;
	}
}

CameraMovingHelper::CameraMovingHelper()
{
	m_xa = std::tr1::shared_ptr<sCameraMoving>(new sCameraMoving);
	m_ya = std::tr1::shared_ptr<sCameraMoving>(new sCameraMoving);
	m_za = std::tr1::shared_ptr<sCameraMoving>(new sCameraMoving);
	m_pivot = std::tr1::shared_ptr<sCameraMoving>(new sCameraMoving);
}

bool CameraMovingHelper::SetFromTo(const WMatrix* from, const WMatrix* to)
{
	if (m_xa)
		m_xa->SetFromTo(from->xa, to->xa);
	if (m_ya)
		m_ya->SetFromTo(from->ya, to->ya);
	if (m_za)
		m_za->SetFromTo(from->za, to->za);
	if (m_pivot)
		m_pivot->SetFromTo(from->pivot, to->pivot);
	return true;
}

void CameraMovingHelper::SetEase(int x, int y, int z)
{
	if (m_xa)
		m_xa->SetEase(x, y, z);
	if (m_ya)
		m_ya->SetEase(x, y, z);
	if (m_za)
		m_za->SetEase(x, y, z);
	if (m_pivot)
		m_pivot->SetEase(x, y, z);
}

void CameraMovingHelper::SetDuration(float d)
{
	if (m_xa)
		m_xa->SetDuration(d);
	if (m_ya)
		m_ya->SetDuration(d);
	if (m_za)
		m_za->SetDuration(d);
	if (m_pivot)
		m_pivot->SetDuration(d);
}

void CameraMovingHelper::Start()
{
	if (m_pivot)
		m_pivot->Start();
}

void CameraMovingHelper::ApplyXa()
{
	if (m_xa)
		g_camera.xa = m_xa->cur;
}

void CameraMovingHelper::ApplyYa()
{
	if (m_ya)
		g_camera.ya = m_ya->cur;
}

void CameraMovingHelper::ApplyZa()
{
	if (m_za)
		g_camera.za = m_za->cur;
}

void CameraMovingHelper::ApplyPivot()
{
	if (m_pivot)
		g_camera.pivot = m_pivot->cur;
}

void CameraMovingHelper::Update(const float& dt)
{
	if (m_xa && !m_xa->IsDone())
	{
		m_xa->Update(dt);
		ApplyXa();
	}
	if (m_ya && !m_ya->IsDone())
	{
		m_ya->Update(dt);
		ApplyYa();
	}
	if (m_za && !m_za->IsDone())
	{
		m_za->Update(dt);
		ApplyZa();
	}
	if (m_pivot && !m_pivot->IsDone())
	{
		m_pivot->Update(dt);
		ApplyPivot();
	}
}

bool CameraMovingHelper::IsDone() const
{
	bool bDone = true;
	if (m_xa && !m_xa->IsDone())
		bDone = false;
	if (m_ya && !m_ya->IsDone())
		bDone = false;
	if (m_za && !m_za->IsDone())
		bDone = false;
	if (m_pivot && !m_pivot->IsDone())
		bDone = false;
	return bDone;
}

void CameraMovingHelper::Stop()
{
	if (m_xa)
		m_xa->Reset();
	if (m_ya)
		m_ya->Reset();
	if (m_za)
		m_za->Reset();
	if (m_pivot)
		m_pivot->Reset();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ntLobbyBg

IMPLEMENT_ACTOR(ntLobbyBg, IActor)

void ntLobbyBg::OnPreLoadInit()
{
	m_pPoster983[0] = NULL;
	m_posterId983[0] = 0;
	m_pPoster983[1] = NULL;
	m_posterId983[1] = 0;

	new CShadowManager(g_resrcmng, COption::Instance()->vIsShadowEnabled());

	if (LOBBYBG_MAP == 13)
		g_view->SetClip(0.1f, 26214.4f, true);
}

void ntLobbyBg::OnLoad()
{
	if (CFx::Instance())
		CFx::Instance()->SetActive(false);

	if (!GOLFDOC()->m_pPolySoup->Load(s_lobbyGbin983.c_str(), false))
	{
		g_bQuit = true;
		return;
	}

	LoadMapData983();

	m_pCamHelper983 = std::tr1::shared_ptr<CameraMovingHelper>(new CameraMovingHelper);

	GOLFDOC()->m_pPolySoup->EndLoad(false);

	if (CFx::Instance())
		CFx::Instance()->SetActive(true);

	WPolySoup::sModel* pModel = GOLFDOC()->m_pPolySoup->FindModel("lobby_post01.pet");
	m_pPoster983[0] = pModel ? pModel->pet : NULL;
	pModel = GOLFDOC()->m_pPolySoup->FindModel("lobby_post02.pet");
	m_pPoster983[1] = pModel ? pModel->pet : NULL;

	m_bLoaded983 = true;
}

void ntLobbyBg::LoadMapData983()
{
	if (!GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky = new CSky;

	GOLFDOC()->m_waveInfo.Load(MakeStr("%s_wave.txt", m_mapName983.c_str()));
	GOLFDOC()->m_riverInfo.Load(MakeStr("%s_river.txt", m_mapName983.c_str()));

	if (GOLFDOC()->m_pSky)
	{
		GOLFDOC()->m_pSky->LoadSky(m_mapName983.c_str(), 0.0f);
		GOLFDOC()->m_pSky->RegisterCloud(MakeStr("%s_cloud.dds", m_mapName983.c_str()));
		GOLFDOC()->m_pPolySoup->MakeDataStructure();
		GOLFDOC()->m_pSky->GenerateStars(m_mapNum983);
		GOLFDOC()->m_pSky->GenerateMoons();
		GOLFDOC()->m_pSky->GenerateLensFlare();
	}

	CShadowManager::Instance()->UpdateLight("sun", g_lightset);

	CSceneManager::Instance()->Load();
}

void ntLobbyBg::OnInit()
{
	CSceneManager::Instance()->UpdateFog(LOBBYBG_MAPNAME);

	Wind().CheckSpecialWind();

	m_bCamMoving983 = false;
}

void ntLobbyBg::OnDestroy()
{
	if (CShadowManager::Instance())
		delete CShadowManager::Instance();

	m_bCamMoving983 = false;
}

void ntLobbyBg::HandleMsg(const MsgObject& msg)
{
	if (msg.time > g_CurrentTime)
	{
		StoreDelayedMsg(msg);
		return;
	}

	switch (msg.message)
	{
	case M983(0x118): // 0x146
		GOLFDOC()->m_pPolySoup->SetRenderMode(msg.param1);
		break;

	case M983(0x13d): // 0x179
		GOLFDOC()->m_pSky->GenerateClouds(50, Wind().GetPower983());
		break;

	// 983 0x17a: troca o mapa do cenario (param1 = numero das estrelas, param2 = nome); param3 != 0 recarrega agora
	case M983(0x13e):
		m_mapNum983 = msg.param1;
		m_mapName983 = (const char*)msg.param2;
		if (msg.param3)
			LoadMapData983();
		break;

	case M983(0x13f): // 0x17b
		GOLFDOC()->m_pSky->m_color = msg.param1;
		break;

	case M983(0x140): // 0x17c
		DoReLoad();
		break;

	case M983(0x141): // 0x17d
		if (GOLFDOC()->m_pPolySoup)
			GOLFDOC()->m_pPolySoup->UpdateBgFxWind();
		break;

	// 983 0x17f: leva a camera para a camera do mapa de nome param1 (vinda de param2, ou da atual); param3 = animado,
	// param4 = int* tipo da curva (padrao 15); ao terminar manda 0x180 a si mesmo
	case 0x17f:
	{
		std::string to = msg.param1 ? (const char*)msg.param1 : "";
		std::string from = msg.param2 ? (const char*)msg.param2 : "";
		bool bAnimate = msg.param3 != 0;
		m_bCamMoving983 = false;
		int ease = msg.param4 ? *(int*)msg.param4 : 15;

		if (to == from)
		{
			this << MsgObject(NULL, 0x180, 0, 0, 0, 0, 0);
			break;
		}

		WPolySoup::sCamera* pTo = GOLFDOC()->m_pPolySoup->FindCamera(to.c_str());
		WPolySoup::sCamera* pFrom = GOLFDOC()->m_pPolySoup->FindCamera(from.c_str());
		if (!pTo)
			break;

		if (!bAnimate)
		{
			g_camera = pTo->mat;
			m_curCamera983 = to;
			g_view->SetFOV(pTo->fov * g_DEGTORAD);
			this << MsgObject(NULL, 0x180, 0, 0, 0, 0, 0);
		}
		else if (m_pCamHelper983)
		{
			m_pCamHelper983->SetFromTo(pFrom ? &pFrom->mat : &g_camera, &pTo->mat);
			m_pCamHelper983->SetEase(ease, ease, ease);
			m_pCamHelper983->SetDuration(2.0f);
			m_pCamHelper983->Start();
			m_bCamMoving983 = true;
			m_curCamera983 = to;
			g_view->SetFOV(pTo->fov * g_DEGTORAD);
		}
	}
	break;

	// 983 0x180: a camera chegou; avisa o ator principal da tarefa (o "Lobby")
	case 0x180:
		m_bCamMoving983 = false;
		AfxGetTask()->GetMainActor() << MsgObject(NULL, 0x180, 0, 0, 0, 0, 0);
		break;

	// 983 0x182: cartazes do lobby = os dois do equipamento (sUserEquip::poster); 0x183 tira os dois
	case 0x182:
		if (m_posterId983[0] != Doc()->m_myInfo.userEquip.poster[0])
			SetPoster983(0, Doc()->m_myInfo.userEquip.poster[0]);
		if (m_posterId983[1] != Doc()->m_myInfo.userEquip.poster[1])
			SetPoster983(1, Doc()->m_myInfo.userEquip.poster[1]);
		break;

	case 0x183:
		SetPoster983(0, 0);
		SetPoster983(1, 0);
		break;
	}
}

ntLobbyBg::ntLobbyBg()
	: m_mapNum983(0), m_mapName983("blue"), m_bCamMoving983(false), m_bLoaded983(false)
{
	m_dispPriority = DISP_PRIORITY_1;
}

void ntLobbyBg::DoReLoad()
{
	SendMsg(this, "Ambient", M983(0x1f4), 0, 0, 0, 0);

	OnLoad();

	AfxGetTask()->Init(NULL);
}

void ntLobbyBg::SetPoster983(int slot, unsigned long id)
{
	int orgTex = g_resrcmng->LoadTexture(s_posterTex983[slot].c_str(), 0, 0, NULL);
	if (!m_pPoster983[slot]) // (o 983 nao confere: o cenario sempre tem os dois cartazes)
		return;
	if (id == 0 && orgTex)
	{
		m_pPoster983[slot]->ChangeTextureByHandle(orgTex, orgTex, NULL);
		m_posterId983[slot] = 0;
		return;
	}

	IFF_STRUCT::sFurniture* pFurniture = Doc()->m_itemManager.FindFurniture(id);
	if (pFurniture)
	{
		int tex = g_resrcmng->LoadTexture(MakeStr("%s.jpg", pFurniture->Tex[0]), 0, 0, NULL);
		if (orgTex && tex)
		{
			m_pPoster983[slot]->ChangeTextureByHandle(tex, orgTex, NULL);
			m_posterId983[slot] = id;
		}
	}
}

void ntLobbyBg::SetSkyColor(unsigned long color)
{
	if (GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->m_color = color;
}

// 983 00ab1e70: alem do PVS, anda a camera (CameraMovingHelper) enquanto o RFLobbyDesktop esta visivel
void ntLobbyBg::OnProcess(float delta)
{
	GetPVS().PreProcess(delta, false);

	RFLobbyDesktop* pDesktop = RFLobbyDesktop::Instance983();
	if (pDesktop && m_pCamHelper983)
	{
		if (pDesktop->IsWindowVisible(false))
		{
			m_pCamHelper983->Update(delta);
			if (m_bCamMoving983 && m_pCamHelper983->IsDone())
				this << MsgObject(NULL, 0x180, 0, 0, 0, 0, 0);
		}
		else
			m_pCamHelper983->Stop();
	}
}
