#include "minatl.h"
#include "projectg.h"
#include "golfdoc.h"
#include "golfball.h"
#include "quadtree.h"
#include "shareddoc.h"
#include "scenemanager.h"
#include "taskmanager.h"
#include "clientsetting.h"
#include "commonutil.h"
#include "polysoup.h"
#include "soundmanager.h"
#include "fxspray.h"
#include "fx.h"
#include "wind.h"
#include "wpuppet.h"
#include "w3danispr.h"
#include "particle.h"
#include "particleparty.h"

// HACK: scenemanager.h and quadtree.h are part of the PCH, so the members used
// here are declared on helper classes and aliased to the real symbols (SetFog is
// the PCH stand-in CSceneManagerFog, see stdafx.h).
class CSceneManagerEx : public CSceneManager
{
public:
	WPuppet* GetPuppet(const char* name);
};


class CQuadTreeEx : public CQuadTree
{
public:
	void SetBaseVtxColor(unsigned long color);
	void SetObjVtxColor(unsigned long color);
};

#pragma comment(linker, \
	"/alternatename:?SetFog@CSceneManager@@QAEX_N@Z=?SetFog@CSceneManagerFog@@QAEX_N@Z")
#pragma comment(linker, \
	"/alternatename:?GetPuppet@CSceneManagerEx@@QAEPAVWPuppet@@PBD@Z=?GetPuppet@CSceneManager@@QAEPAVWPuppet@@PBD@Z")
#pragma comment(linker, \
	"/alternatename:?SetBaseVtxColor@CQuadTreeEx@@QAEXK@Z=?SetBaseVtxColor@CQuadTree@@QAEXK@Z")
#pragma comment(linker, \
	"/alternatename:?SetObjVtxColor@CQuadTreeEx@@QAEXK@Z=?SetObjVtxColor@CQuadTree@@QAEXK@Z")

// HACK: forces the three-float constant table (PI, HUGE, EPSILON) that the
// original unit emitted; the inline functions that used it are unknown.
inline void rb_UseConsts()
{
	(void)g_PI;
	(void)g_HUGE;
}

#define SCENEMGR() ((CSceneManagerEx*)CSceneManager::Instance())
#define SCENEFOG() ((CSceneManagerFog*)CSceneManager::Instance())
#define PVS() ((CQuadTreeEx*)&GetPVS())

extern Fresh* g_pFresh;

unsigned char GetCurMap();
const WVector& GetHoleCupPos(); // 983: devolve referencia
WVector operator*(const WVector& v, float f);
WVector operator*(float f, const WVector& v);
WVector operator*(const WVector& v, const WMatrix& m);

inline CTaskDoc* AfxGetDocument()
{
	if (0)
		((CParticleEffect*)0)->CParticleEffect::DisplayShadow();
	if (CTaskManager::Instance() != NULL)
	{
		CTask* pTask = CTaskManager::Instance()->GetCurrentTask();
		if (pTask != NULL)
			return pTask->GetDocument();
	}
	return NULL;
}

#define SAFE_DELETE(p) \
	{ \
		if (p) \
		{ \
			delete (p); \
			(p) = NULL; \
		} \
	}

IMPLEMENT_ACTOR(CParticleEffect, IActor)

CParticleEffect::CParticleEffect()
	: m_recvList(16, 16), m_pManager(NULL), m_bgList(16, 16), m_pLight(NULL)
{
	m_spFxInterpreter983 = std::tr1::shared_ptr<FxInterpreter>(new FxInterpreter(NULL, 0)); // 983: 00779c87
}

void CParticleEffect::OnDestroy()
{
	SAFE_DELETE(m_pManager);
	Clear();
}

void CParticleEffect::Clear()
{
	CParticleRecv::Disconnect();
	for (CParticleRecv* pRecv = m_recvList.Start(); pRecv;
		pRecv = m_recvList.Next())
	{
		delete pRecv;
	}
	m_recvList.Reset();
}

void CParticleEffect::OnPreLoadInit()
{
	m_pManager = new CParticleManager;
	m_pLight = new CLightColumn;
	m_pManager->MakeParticle(m_pLight, NULL, true);
	CSceneManager::Instance()->RegisterElement(OBJ_CUSTOM, NULL, // 983 00778cb0: OBJ_CUSTOM (5), nao OBJ_EFFECT
		(CRenderFuncPtr*)this, "\xc0\xcc\xc6\xe5\xc6\xae", 0x10, NULL);
}

void CParticleEffect::OnInit()
{
	Clear();
}

void CParticleEffect::OnProcess(float delta)
{
	for (CParticleRecv* pRecv = m_recvList.Start(); pRecv;
		pRecv = m_recvList.Next())
	{
		if (pRecv->IsActive())
			pRecv->Process(delta);
	}
	m_pManager->Process(delta);
}

void CParticleEffect::Display()
{
	for (CParticleRecv* pRecv = m_recvList.Start(); pRecv;
		pRecv = m_recvList.Next())
	{
		if (pRecv->IsActive())
			pRecv->Display();
	}
	m_pManager->Display();
}

void CParticleEffect::DisplayShadow()
{
	if (m_pManager)
		m_pManager->DisplayShadow();
}

// 983: 00778d70. As mensagens foram renumeradas (0x16c.. -> 0x1d7..), cada caso chama o MakeParticle por conta propria, e ha
// casos novos: 0x1d9/0x1da/0x1db (sequencia de fx da tacada pelo FxInterpreter do ator), 0x1ef (modo da coluna de luz) e
// 0x1f0 (fx e som de mudanca de vento, que passa mesmo com as particulas desligadas).
C_ASSERT_VC10(offsetof(CGolfDoc, m_gameMode) == 0x158 && offsetof(CGolfDoc, m_bDay) == 0x160); // A749: ex-crus m_pGolfDoc+0x158/+0x160

void CParticleEffect::HandleMsg(const MsgObject& msg)
{
	if (COption::Instance()->vGetParticleLevel() == 2 && msg.message != 0x1f0/*983*/)
		return;

	switch (msg.message)
	{
	case 0x1d7/*983*/:
		m_pManager->MakeParticle(new CPowerShot, "\xc6\xc4\xbf\xf6\xbc\xa6", false);
		return;
	case 0x1d9/*983*/:
		m_spFxInterpreter983->PlayFx983("pangya_shot.seq", &GolfBall().m_pos);
		return;
	case 0x1da/*983*/:
	case 0x1db/*983*/:
		m_spFxInterpreter983->PlayFx983("nomal_shot.seq", &GolfBall().m_pos);
		return;
	case 0x1dc/*983*/:
		m_pManager->MakeParticle(new CGrassParticle(msg.param1, *(WVector*)msg.param2), "\xc0\xdc\xb5\xf0", false);
		return;
	case 0x1dd/*983*/:
		if (CheckExtShotEffect(0x1dd))
			return;
		m_pManager->MakeParticle(new CBunkerShotParticle(4, 100), "\xba\xa1\xc4\xbf", false);
		return;
	case 0x1de/*983*/:
		if (CheckExtShotEffect(0x1e0))
			return;
		m_pManager->MakeParticle(new CBunkerParticle, "\xba\xa1\xc4\xbf", false);
		return;
	case 0x1e0/*983*/:
	{
		if (CheckExtShotEffect(0x1e0))
			return;
		CParticle* pParticle = new CDust(1);
		float radius = GolfBall().m_radius;
		WVector pos = GolfBall().m_pos - WVector(0.0f, radius, 0.0f);
		pParticle->m_center = pos;
		m_pManager->MakeParticle(pParticle, "\xb8\xd5\xc1\xf6", false);
		return;
	}
	case 0x1e1/*983*/:
	{
		WVector normal;
		normal = GolfBall().PreData(msg.param1)->normal;
		float radius = GolfBall().m_radius;
		WVector offset(normal.x * radius, normal.y * radius, normal.z * radius);
		const CGolfBall::sExpected* pData = GolfBall().PreData(msg.param1);
		WVector pos(pData->pos.x - offset.x, pData->pos.y - offset.y,
			pData->pos.z - offset.z);
		CParticle* pParticle = new CFragment;
		pParticle->SetNormal(normal);
		pParticle->m_center = pos;
		m_pManager->MakeParticle(pParticle, "\xc0\xd4\xc0\xda", false);
		return;
	}
	case 0x1e3/*983*/:
	{
		WVector pos = PVS()->GetGroundPoint(GolfBall().m_pos, true, NULL, true);
		float radius = GolfBall().m_radius;
		pos.y = radius + pos.y;
		CParticle* pParticle = new CRipple;
		pParticle->m_center = pos;
		m_pManager->MakeParticle(pParticle, "\xc6\xc4\xb9\xae", false);
		return;
	}
	case 0x1df/*983*/:
	{
		CParticleRecv* pRecv = m_recvList.Find((const char*)msg.param1);
		if (pRecv == NULL)
		{
			pRecv = new CParticleRecv(MakeStr("%s.eft", msg.param1));
			m_recvList.AddItem(pRecv, (const char*)msg.param1, false);
		}
		m_recv = *(sRecv*)msg.param2;
		pRecv->SetActive(true);
		pRecv->SetCenter(m_recv.center);
		pRecv->SetGround(m_recv.ground);
		pRecv->SetDuration(m_recv.duration);
		return;
	}
	case 0x1e2/*983*/:
		if (msg.param1)
		{
			CParticleRecv* pRecv = m_recvList.Find((const char*)msg.param1);
			pRecv->SetActive(false);
			return;
		}
		for (CParticleRecv* pRecv = m_recvList.Start(); pRecv;
			pRecv = m_recvList.Next())
		{
			pRecv->SetActive(false);
		}
		return;
	case 0x1e4/*983*/:
		m_pManager->RemoveAll();
		return;
	case 0x1e5/*983*/:
		if (m_pLight == NULL)
			return;
		m_pLight->FadeIn();
		return;
	case 0x1e6/*983*/:
		if (m_pLight == NULL)
			return;
		m_pLight->FadeOut();
		return;
	case 0x1e7/*983*/:
		if (m_pLight == NULL)
			return;
		m_pLight->SetActive(true);
		return;
	case 0x1e8/*983*/:
		if (m_pLight == NULL)
			return;
		m_pLight->SetActive(false);
		return;
	case 0x1ef/*983*/:
		if (m_pLight == NULL)
			return;
		switch (msg.param1)
		{
		case 0:
			m_pLight->SetMode983(3);
			break;
		case 1:
			m_pLight->SetMode983(4);
			break;
		}
		m_pLight->FadeOut();
		return;
	case 0x1ea/*983*/:
	{
		WVector pos = msg.param2 == 0 ? GetHoleCupPos() : GolfBall().m_pos;
		CParticle* pParticle = new CCoin(msg.param1, pos);
		g_audio->PlaySfx("pang_coin_emit");
		m_pManager->MakeParticle(pParticle, "\xc8\xa6\xc4\xc5", false);
		return;
	}
	case 0x1eb/*983*/:
	{
		if (COption::Instance()->vGetParticleLevel() < 1)
			SCENEFOG()->SetFog(false);
		WVector pos = msg.param1 == 0 ? GetHoleCupPos() : GolfBall().m_pos;
		m_pManager->MakeParticle(new CEnergyCenter(30, pos, (float)msg.param2 * 0.1f), "\xc8\xa6\xc4\xc5", false);
		return;
	}
	case 0x1ec/*983*/:
	{
		if (CSharedDoc::Instance()->m_pGolfDoc->m_bDay == 0) // 983: m_bDay em 0x160
			return;
		if (COption::Instance()->vGetParticleLevel() >= 1)
			return;
		SCENEFOG()->SetFog(true);
		IActor* pActor = AfxGetTask()->GetActor("GolfBg");
		if (pActor)
		{
			MsgObject fog;
			fog.sender = NULL;
			fog.time = 0;
			fog.message = 0x17b; // 983: era 0x13f
			fog.param1 = -1;
			pActor << fog;
		}
		PVS()->SetBaseVtxColor(0xffffffff);
		PVS()->SetObjVtxColor(0xffffffff);
		return;
	}
	case 0x1ed/*983*/:
	{
		WPuppet* pPuppet = SCENEMGR()->GetPuppet("\xb0\xf8");
		if (pPuppet && pPuppet->m_rootbone)
			pPuppet->SetAlpha(0);
		m_pManager->MakeParticle(new CStarDust, "\xc8\xa6\xc4\xc5", false);
		return;
	}
	case 0x1ee/*983*/:
	{
		WPuppet* pPuppet = SCENEMGR()->GetPuppet("\xb0\xf8");
		if (pPuppet && pPuppet->m_rootbone)
			pPuppet->SetAlpha(0);
		m_pManager->MakeParticle(new CStarDust2, "\xc8\xa6\xc4\xc5", false);
		return;
	}
	case 0x1f0/*983*/:
	{
		float intensity = Wind().GetPower983();
		float dist = (CSharedDoc::Instance()->m_pGolfDoc->m_gameMode & 0x200) ? 100.0f : 500.0f; // 983: m_gameMode em 0x158
		WVector pos = dist * g_camera.za + g_camera.pivot;
		if (intensity > 6.0f)
			m_spFxInterpreter983->PlayFx983("wind_change9.spr", &pos);
		else if (intensity > 3.0f)
			m_spFxInterpreter983->PlayFx983("wind_change4.spr", &pos);
		else
			m_spFxInterpreter983->PlayFx983("wind_change1.spr", &pos);
		int r = rand() % 3;
		if (r == 0)
			g_audio->PlaySfx("wind_change01");
		else if (r == 1)
			g_audio->PlaySfx("wind_change02");
		else
			g_audio->PlaySfx("wind_change03");
		return;
	}
	default:
		return;
	}
}

// 983: 007783e0. Recebe a mensagem (0x1da e 0x1dd = tacada); alem do mapa 0x13 (moedas) trata o mapa 0x14 (cristal).
bool CParticleEffect::CheckExtShotEffect(int message)
{
	WVector pos = GolfBall().m_pos;
	unsigned char type = GetPVS().GetGroundType(pos, true, NULL);
	std::string spray;
	std::string sound;
	bool bShot = false;
	switch (message)
	{
	case 0x1da:
	case 0x1dd:
		bShot = true;
		break;
	}
	if (type == 3)
	{
		switch (GetCurMap())
		{
		case 0x13:
			if (bShot)
				CallSprayParticle("Bunker_coin.spr", "wiz_Moneybunkerin", &pos);
			else
				CallSprayParticle("Bunker_coinFall.spr", "wiz_Moneybunkerin", &pos);
			return true;
		case 0x14:
			if (bShot)
			{
				m_pManager->MakeParticle(new CShotParticle, NULL, false);
			}
			else
			{
				CParticle* pParticle = new CDust(1);
				float radius = GolfBall().m_radius;
				WVector center = GolfBall().m_pos - WVector(0.0f, radius, 0.0f);
				pParticle->m_center = center;
				m_pManager->MakeParticle(pParticle, "\xb8\xd5\xc1\xf6", false);
			}
			g_audio->PlaySfx("bunker_crystal");
			return true;
		}
	}
	return false;
}

void CParticleEffect::CallSprayParticle(const char* spray, const char* sound,
	const WVector* pPos)
{
	if (spray)
	{
		CFxSpray* pSpray = CFx::Instance()->OpenSpray(spray);
		if (pSpray && pPos)
		{
			pSpray->Pos() = *pPos;
		}
		if (sound)
			g_audio->PlaySfx(sound);
	}
}

CParticleManager::CParticleManager()
	: m_texList(16, 8)
{
	m_count = 0;
	m_keep = 0;
	m_last = -1;
	RegisterTexture("\xc6\xc4\xbf\xf6\xbc\xa6", "shockwave.jpg");
	RegisterTexture("\xc8\xa6\xc4\xc5", "~particle_all03.jpg");
	RegisterTexture("\xb8\xd5\xc1\xf6", "[dust.jpg");
	RegisterTexture("\xc0\xdc\xb5\xf0", "[eft01.jpg");
	RegisterTexture("\xba\xa1\xc4\xbf", "[dust01.jpg");
	RegisterTexture("\xc0\xd4\xc0\xda", "~particle_all02.jpg");
	RegisterTexture("\xbf\xac\xb1\xe2", "[dust.jpg");
	RegisterTexture("\xc6\xc4\xb9\xae", "sea-eft.jpg");
	for (int i = 0; i < 20; i++)
		m_pParticle[i] = NULL;
}

void CParticleManager::RegisterTexture(const char* key, const char* file)
{
	sTexture* pTex = new sTexture;
	pTex->handle = g_resrcmng->LoadTexture(file, 0, 0, NULL);
	strcpy(pTex->name, file);
	m_texList.AddItem(pTex, key, false);
}

CParticleManager::~CParticleManager()
{
	for (int i = 0; i < 20; i++)
		SAFE_DELETE(m_pParticle[i]);

	for (sTexture* pTex = m_texList.Start(); pTex; pTex = m_texList.Next())
	{
		if (pTex->handle)
		{
			g_resrcmng->Release(pTex->handle);
			pTex->handle = 0;
		}
		delete pTex;
	}
}

void CParticleManager::Process(float elapsed)
{
	for (int i = 0; i <= m_last; i++)
	{
		if (m_pParticle[i])
		{
			m_pParticle[i]->Process(elapsed);
			if (i >= m_keep && !m_pParticle[i]->IsActive())
			{
				m_count--;
				SAFE_DELETE(m_pParticle[i]);
				if (i == m_last)
				{
					while (m_last > 0 && m_pParticle[m_last] == NULL)
						m_last--;
				}
			}
		}
	}
}

void CParticleManager::Display()
{
	for (int i = 0; i <= m_last; i++)
	{
		if (m_pParticle[i])
			m_pParticle[i]->Display();
	}
}

void CParticleManager::DisplayShadow()
{
	for (int i = 0; i <= m_last; i++)
	{
		if (m_pParticle[i])
			m_pParticle[i]->DisplayShadow();
	}
}

void CParticleManager::MakeParticle(CParticle* pParticle, const char* texName,
	bool bKeep)
{
	if (m_count == 20)
	{
		if (pParticle)
			delete pParticle;
		return;
	}

	int index = 0;
	while (m_pParticle[index])
		index++;
	m_last = Max(index, m_last);
	m_count++;
	if (bKeep)
		m_keep++;
	m_pParticle[index] = pParticle;
	if (texName)
		pParticle->LoadTexture(m_texList.Find(texName)->handle);
	m_pParticle[index]->MakeParticle();
}

void CParticleManager::RemoveAll()
{
	for (int i = m_keep; i <= m_last; i++)
	{
		if (m_pParticle[i])
		{
			m_pParticle[i]->SetActive(false);
			if (m_pParticle[i])
				delete m_pParticle[i];
			m_pParticle[i] = NULL;
		}
	}
	m_count = m_keep;
}

void CParticleManager::Remove(const char* texName)
{
	w_texlist* pTex = g_resrcmng->FindTexture(texName);
	for (int i = m_keep; i <= m_last; i++)
	{
		if (m_pParticle[i]->GetTexHandle() == pTex->texhandle)
		{
			m_pParticle[i]->SetActive(false);
			SAFE_DELETE(m_pParticle[i]);
			m_count--;
		}
	}
}

CGolfDoc* CParticleRecv::m_pDoc;

CParticleRecv::CParticleRecv(const char* file)
	: m_pSpr(NULL)
{
	cFile* pFile = g_resrcmng->GetCFile(file, -1);
	if (pFile)
	{
		pFile->Read(&m_alphaStart, 4);
		pFile->Read(&m_alphaEnd, 4);
		pFile->Read(&m_sizeStart, 4);
		pFile->Read(&m_sizeEnd, 4);
		pFile->Read(&m_velYMin, 4);
		pFile->Read(&m_velYMax, 4);
		pFile->Read(&m_spinMin, 4);
		pFile->Read(&m_spinMax, 4);
		pFile->Read(&m_lifeMin, 4);
		pFile->Read(&m_lifeMax, 4);
		pFile->Read(&m_gravity, 4);
		pFile->Read(&m_interval, 4);
		pFile->Read(&m_frameTime, 4);
		pFile->Read(&m_duration, 4);
		pFile->Read(&m_bounce, 4);
		pFile->Read(&m_accel, 12);
		pFile->Read(&m_renderFlag, 4);
		pFile->Read(&m_emitCount, 4);
		pFile->Read(&m_type, 4);
		pFile->Read(&m_speedMin, 4);
		pFile->Read(&m_speedMax, 4);
		pFile->Read(&m_box, 24);
		pFile->Read(&m_radius, 4);
		pFile->Read(&m_sprSize, 4);
		pFile->Read(&m_frame, 4);
		pFile->Read(&m_bRandFrame, 1);
		pFile->Read(m_sprName, 0x20);
		CloseCFile(pFile);

		m_poolSize = (int)(m_lifeMax / m_interval) * m_emitCount;
		m_pFrag = new sFrag[m_poolSize];
		for (int i = 0; i < m_poolSize; i++)
			m_pFrag[i].bActive = false;

		m_matrix.Reset();
		m_rot.Reset();
		m_pSpr = g_resrcmng->Get3DAniSpr(m_sprName, 0, m_sprSize, m_sprSize);
		m_accum = 0.0f;
		m_head = 0;
		m_liveCount = 0;
		m_bActive = true;
		m_bVisible = true;
		m_objIndex = 0xffff;
		m_objAlpha = 1.0f;
		if (m_pDoc == NULL)
			m_pDoc = (CGolfDoc*)AfxGetDocument();
		m_pTarget = &m_center;
	}
}

CParticleRecv::~CParticleRecv()
{
	Destroy();
}

void CParticleRecv::Destroy()
{
	if (m_pSpr)
	{
		g_resrcmng->Release(m_pSpr);
		m_pSpr = NULL;
	}
	if (m_pFrag)
	{
		delete[] m_pFrag;
		m_pFrag = NULL;
	}
}

void CParticleRecv::SetTarget(WVector* pTarget)
{
	if (pTarget == NULL)
		pTarget = &m_center;
	m_pTarget = pTarget;
}

void CParticleRecv::Process(float elapsed)
{
	if (m_bActive)
	{
		float interval = m_interval;
		for (int i = 0; i < m_poolSize; i++)
		{
			if (m_pFrag[i].bActive)
				BlowAway(elapsed, i);
		}

		float accum = elapsed + m_accum;
		m_accum = accum;
		if (m_duration > 0.0f)
			m_duration -= elapsed;
		if (accum > interval && m_duration >= 0.0f)
		{
			int count;
			if (elapsed > interval)
				count = (int)(accum / interval);
			else
				count = 1;
			m_accum = accum - (float)count * interval;
			for (; count > 0; count--)
			{
				for (int i = 0; i < m_emitCount; i++)
				{
					Generate();
					if (elapsed > interval)
						BlowAway((float)count * interval + m_accum, m_head);
					m_liveCount++;
					m_head = (m_head + 1) % m_poolSize;
				}
			}
		}
		m_objAlpha =
			(float)m_pDoc->m_pPolySoup->GetObjAlpha(m_objIndex) * 0.003921569f;
	}
}

void CParticleRecv::Display()
{
	if (m_bActive && m_bVisible)
	{
		for (int i = 0; i < m_poolSize; i++)
		{
			if (m_pFrag[i].bActive)
			{
				float alpha = m_pFrag[i].alpha;
				float size = m_pFrag[i].size;
				float objAlpha = m_objAlpha;
				int color = (int)(alpha * objAlpha) * 0x1000000 + 0xffffff;
				m_pSpr->SetRect(size, size, size * 0.5f, size * 0.5f);
				m_pSpr->SetPos(m_pFrag[i].pos);
				m_pSpr->SetColor(color);
				m_pSpr->Render(g_view, m_pFrag[i].angle,
					m_renderFlag | 0x400000, m_pFrag[i].frame);
			}
		}
	}
}

void CParticleRecv::Generate()
{
	m_pFrag[m_head].bActive = true;
	m_pFrag[m_head].alpha = m_alphaStart;
	m_pFrag[m_head].size = m_sizeStart;
	int n = rand() % 0x3ff;
	m_pFrag[m_head].life =
		(m_lifeMax - m_lifeMin) * (float)n * 0.0009765625f + m_lifeMin;
	m_pFrag[m_head].remain = m_pFrag[m_head].life;
	m_pFrag[m_head].frameTime = m_frameTime;
	if (m_bRandFrame)
		m_pFrag[m_head].frame = rand() % m_pSpr->GetSpriteNum();
	else
		m_pFrag[m_head].frame = m_frame;
	float dir = 1.0f;
	if (rand() % 2 == 0)
		dir = -1.0f;
	n = rand() % 0x3ff;
	m_pFrag[m_head].angleSpeed =
		((m_spinMax - m_spinMin) * (float)n * 0.0009765625f + m_spinMin) * dir;
	m_pFrag[m_head].angle = 0.0f;
	m_pFrag[m_head].pos.x = m_center.x;
	m_pFrag[m_head].pos.y = m_center.y;
	m_pFrag[m_head].pos.z = m_center.z;
	if (m_type == 0)
	{
		n = rand() % 0x3ff;
		m_pFrag[m_head].pos.x =
			(m_box.max.x - m_box.min.x) * (float)n * 0.0009765625f +
			m_pFrag[m_head].pos.x + m_box.min.x;
		n = rand() % 0x3ff;
		m_pFrag[m_head].pos.y =
			(m_box.max.y - m_box.min.y) * (float)n * 0.0009765625f +
			m_pFrag[m_head].pos.y + m_box.min.y;
		n = rand() % 0x3ff;
		m_pFrag[m_head].pos.z =
			(m_box.max.z - m_box.min.z) * (float)n * 0.0009765625f +
			m_pFrag[m_head].pos.z + m_box.min.z;
		n = rand() % 0x3ff;
		float speed =
			(m_speedMax - m_speedMin) * (float)n * 0.0009765625f + m_speedMin;
		WVector vel;
		vel.x = WVector::UNIT_POS_Z.x * speed;
		vel.y = WVector::UNIT_POS_Z.y * speed;
		vel.z = WVector::UNIT_POS_Z.z * speed;
		n = rand() % 0x3ff;
		WVector rotated =
			vel * RotMat((float)n * 0.0009765625f * 6.28318530717958647692f, 1);
		m_pFrag[m_head].vel.x = rotated.x;
		m_pFrag[m_head].vel.y = rotated.y;
		m_pFrag[m_head].vel.z = rotated.z;
		n = rand() % 0x3ff;
		m_pFrag[m_head].vel.y =
			(m_velYMax - m_velYMin) * (float)n * 0.0009765625f + m_velYMin;
		WVector world = m_pFrag[m_head].vel * m_matrix;
		m_pFrag[m_head].vel.x = world.x;
		m_pFrag[m_head].vel.y = world.y;
		m_pFrag[m_head].vel.z = world.z;
	}
	else if (m_type == 1)
	{
		if (g_EPSILON <= Wabs(m_radius))
		{
			float radius = m_radius;
			WVector offset;
			offset.x = WVector::UNIT_POS_Z.x * radius;
			offset.y = WVector::UNIT_POS_Z.y * radius;
			offset.z = radius * WVector::UNIT_POS_Z.z;
			n = rand() % 0x3ff;
			WMatrix rotX =
				RotMat((float)n * 0.0009765625f * 6.28318530717958647692f, 1);
			n = rand() % 0x3ff;
			WMatrix rotY =
				RotMat((float)n * 0.0009765625f * 6.28318530717958647692f, 0);
			WVector result = (offset * rotY) * rotX;
			m_pFrag[m_head].pos.x = m_pFrag[m_head].pos.x + result.x;
			m_pFrag[m_head].pos.y = result.y + m_pFrag[m_head].pos.y;
			m_pFrag[m_head].pos.z = result.z + m_pFrag[m_head].pos.z;
			float dx = m_pFrag[m_head].pos.x - m_center.x;
			float dy = m_pFrag[m_head].pos.y - m_center.y;
			float dz = m_pFrag[m_head].pos.z - m_center.z;
			float inv = 1.0f / (float)sqrt(dx * dx + dy * dy + dz * dz);
			m_pFrag[m_head].vel.x = inv * dx;
			m_pFrag[m_head].vel.y = dy * inv;
			m_pFrag[m_head].vel.z = dz * inv;
		}
		else
		{
			n = rand() % 0x3ff;
			WMatrix rotX =
				RotMat((float)n * 0.0009765625f * 6.28318530717958647692f, 1);
			n = rand() % 0x3ff;
			WMatrix rotY =
				RotMat((float)n * 0.0009765625f * 6.28318530717958647692f, 0);
			WVector result = (WVector::UNIT_POS_Z * rotY) * rotX;
			m_pFrag[m_head].vel.x = result.x;
			m_pFrag[m_head].vel.y = result.y;
			m_pFrag[m_head].vel.z = result.z;
		}
		float scale =
			(m_velYMax - m_velYMin) * (float)(rand() % 0x3ff) * 0.0009765625f +
			m_velYMin;
		m_pFrag[m_head].vel.x = scale * m_pFrag[m_head].vel.x;
		m_pFrag[m_head].vel.y = scale * m_pFrag[m_head].vel.y;
		m_pFrag[m_head].vel.z = scale * m_pFrag[m_head].vel.z;
	}
}

void CParticleRecv::BlowAway(float elapsed, int index)
{
	m_pFrag[index].remain = m_pFrag[index].remain - elapsed;
	if (m_frameTime > 0.0f)
		m_pFrag[index].frameTime = m_pFrag[index].frameTime - elapsed;
	sFrag* pFrag = &m_pFrag[index];
	if (m_pFrag[index].remain < 0.0f || pFrag->alpha < 0.0f ||
		pFrag->size < 0.0f)
	{
		pFrag->bActive = false;
		m_liveCount--;
		return;
	}

	pFrag->size =
		((m_sizeEnd - m_sizeStart) / pFrag->life) * elapsed + pFrag->size;
	m_pFrag[index].angle =
		elapsed * m_pFrag[index].angleSpeed + m_pFrag[index].angle;
	m_pFrag[index].alpha =
		((m_alphaEnd - m_alphaStart) / m_pFrag[index].life) * elapsed +
		m_pFrag[index].alpha;

	const WVector& wind = Wind().GetWind(m_pFrag[index].pos);
	float factor = m_pDoc->m_pPolySoup->GetObjWindFactor(m_objIndex);
	float* pVel = &m_pFrag[index].vel.x;
	pVel[0] = factor * wind.x * elapsed + pVel[0];
	pVel[1] = factor * wind.y * elapsed + pVel[1];
	pVel[2] = factor * wind.z * elapsed + pVel[2];
	pVel = &m_pFrag[index].vel.x;
	pVel[0] = elapsed * m_accel.x + pVel[0];
	pVel[1] = elapsed * m_accel.y + pVel[1];
	pVel[2] = elapsed * m_accel.z + pVel[2];
	pVel = &m_pFrag[index].vel.y;
	*pVel = *pVel - elapsed * m_gravity * 34.31f;

	if (m_bounce > 0.0f)
	{
		sFrag* pBounce = &m_pFrag[index];
		float bottom = pBounce->pos.y - pBounce->size;
		if (m_ground < bottom)
		{
			float step = elapsed * pBounce->vel.y;
			if ((step + pBounce->pos.y) - pBounce->size < m_ground)
			{
				pBounce->pos += (pBounce->vel * (bottom / step)) * elapsed;
				m_pFrag[index].vel.y = m_bounce * m_pFrag[index].vel.y * -1.0f;
				goto frame;
			}
		}
	}
	{
		float* pPos = &m_pFrag[index].pos.x;
		pPos[0] = elapsed * m_pFrag[index].vel.x + pPos[0];
		pPos[1] = elapsed * pPos[4] + pPos[1];
		pPos[2] = elapsed * pPos[5] + pPos[2];
	}

frame:
	if (m_pFrag[index].frameTime >= 0.0f)
		return;
	m_pFrag[index].frameTime = m_frameTime;
	m_pFrag[index].frame = (m_pFrag[index].frame + 1) % m_pSpr->GetSpriteNum();
}

void CParticleRecv::SetBox(const Waabb& box)
{
	float z = Max(m_box.max.z, 1.0f);
	float y = Max(m_box.max.y, 1.0f);
	float x = Max(m_box.max.x, 1.0f);
	float scale = Max(box.max.x / x, box.max.y / y);
	scale = Max(scale, box.max.z / z);
	m_scale = scale;
	m_box.min = box.min;
	m_box.max = box.max;
	m_sizeStart = m_scale * m_sizeStart;
	m_sizeEnd = m_sizeEnd * m_scale;
	m_velYMin = m_velYMin * m_scale;
	m_velYMax = m_velYMax * m_scale;
	m_speedMin = m_speedMin * m_scale;
	m_speedMax = m_speedMax * m_scale;
}

void CParticleRecv::ApplyScale()
{
	m_sizeStart *= m_scale;
	m_sizeEnd *= m_scale;
	m_velYMin *= m_scale;
	m_velYMax *= m_scale;
	m_speedMin *= m_scale;
	m_speedMax *= m_scale;
}

void CParticleRecv::Disconnect()
{
	m_pDoc = NULL;
}

CParticle2D::~CParticle2D()
{
	Release();
}

int CParticle2D::Initialize(const std::list<std::string>& files,
	const WRect& rect, unsigned int count, eSTYLE style, float minSpeed,
	float maxSpeed, const WVector2D& velMin, const WVector2D& velMax)
{
	m_rect = rect;
	m_maxSpeed = maxSpeed;
	m_style = style;
	m_minSpeed = minSpeed;
	m_count = count;
	m_velMin.x = velMin.x;
	m_velMin.y = velMin.y;
	m_velMax.x = velMax.x;
	m_velMax.y = velMax.y;

	m_bitmaps.reserve(files.size());
	for (std::list<std::string>::const_iterator it = files.begin();
		it != files.end(); ++it)
	{
		const Bitmap* pBitmap = g_pFresh->GetBitmap((*it).c_str());
		if (pBitmap)
			m_bitmaps.push_back(pBitmap);
	}

	if (m_bitmaps.empty())
		return 0;

	for (unsigned int i = 0; i < count; i++)
	{
		sParticle* pParticle = new sParticle;
		memset(pParticle, 0, sizeof(sParticle));
		pParticle->bAlive = 1;
		pParticle->pBitmap = m_bitmaps[rand() % m_bitmaps.size()];
		pParticle->pos.x = (float)(rand() % (int)m_rect.w) + m_rect.x;
		pParticle->pos.y = (float)(rand() % (int)m_rect.h) + m_rect.y;
		pParticle->speed =
			(float)(rand() % (int)(maxSpeed - minSpeed)) + minSpeed;
		pParticle->vel.x = velMin.x;
		pParticle->vel.y = velMin.y;
		float dx = velMax.x - velMin.x;
		if (g_EPSILON <= Wabs(dx))
			pParticle->vel.x = (float)(rand() % (int)Wabs(dx)) + velMin.x;
		float dy = velMax.y - velMin.y;
		if (g_EPSILON <= Wabs(dy))
			pParticle->vel.y = (float)(rand() % (int)Wabs(dy)) + velMin.y;
		if (m_style == STYLE_BLINK)
		{
			pParticle->alphaDir = rand() % 2;
			pParticle->alpha = (float)(rand() % 0x7d) + 125.0f;
		}
		else
		{
			pParticle->alphaDir = 0;
			pParticle->alpha = 255.0f;
		}
		MakeDisplayRect(pParticle);
		m_list.push_back(pParticle);
	}
	return !m_list.empty();
}

void CParticle2D::MakeDisplayRect(sParticle* pParticle)
{
	pParticle->src.x = 0;
	pParticle->src.y = 0;
	const Bitmap* pBitmap = pParticle->pBitmap;
	pParticle->src.w = (float)pBitmap->Width();
	pParticle->src.h = (float)pBitmap->Height();
	pParticle->dst.x = pParticle->pos.x - (float)pBitmap->Width() * 0.5f;
	pParticle->dst.y = pParticle->pos.y - (float)pBitmap->Height() * 0.5f;
	pParticle->dst.w = (float)pBitmap->Width();
	pParticle->dst.h = (float)pBitmap->Height();

	if (m_rect.x > pParticle->dst.x &&
		m_rect.x < pParticle->dst.w + pParticle->dst.x)
	{
		float left = m_rect.x;
		pParticle->src.x = left - pParticle->dst.x;
		pParticle->src.w = pParticle->src.w - (left - pParticle->dst.x);
		pParticle->dst.x = m_rect.x;
		pParticle->dst.w = pParticle->src.w;
	}
	if (m_rect.y > pParticle->dst.y &&
		m_rect.y < pParticle->dst.h + pParticle->dst.y)
	{
		float top = m_rect.y;
		pParticle->src.y = top - pParticle->dst.y;
		pParticle->src.h = pParticle->src.h - (top - pParticle->dst.y);
		pParticle->dst.y = m_rect.y;
		pParticle->dst.h = pParticle->src.h;
	}
	float right = (m_rect.w + m_rect.x);
	if (right > pParticle->dst.x && right < pParticle->dst.w + pParticle->dst.x)
	{
		right =
			pParticle->src.w - ((pParticle->dst.w + pParticle->dst.x) - right);
		pParticle->src.w = right;
		pParticle->dst.w = right;
	}
	float bottom = (m_rect.h + m_rect.y);
	if (bottom > pParticle->dst.y &&
		bottom < pParticle->dst.h + pParticle->dst.y)
	{
		bottom =
			pParticle->src.h - ((pParticle->dst.h + pParticle->dst.y) - bottom);
		pParticle->src.h = bottom;
		pParticle->dst.h = bottom;
	}

	pParticle->src.x = floor(pParticle->src.x);
	pParticle->src.y = floor(pParticle->src.y);
	pParticle->src.w = floor(pParticle->src.w);
	pParticle->src.h = floor(pParticle->src.h);
	pParticle->dst.x = floor(pParticle->dst.x);
	pParticle->dst.y = floor(pParticle->dst.y);
	pParticle->dst.w = floor(pParticle->dst.w);
	pParticle->dst.h = floor(pParticle->dst.h);
}

void CParticle2D::Process(float elapsed)
{
	for (std::list<sParticle*>::iterator it = m_list.begin();
		it != m_list.end();)
	{
		sParticle* pParticle = *it;
		float step = elapsed * pParticle->speed;
		float dy = step * pParticle->vel.y;
		pParticle->pos.x = step * pParticle->vel.x + pParticle->pos.x;
		pParticle->pos.y = dy + pParticle->pos.y;
		if (m_style == STYLE_BLINK)
		{
			if (pParticle->alphaDir != 0)
			{
				if (SAFE_ADDVALUE(pParticle->alpha, 255.0f, elapsed * 125.0f) >=
					255.0f)
					pParticle->alphaDir = 0;
			}
			else
			{
				if (SAFE_SUBVALUE(pParticle->alpha, 125.0f, elapsed * 125.0f) <=
					125.0f)
					pParticle->alphaDir = 1;
			}
		}
		MakeDisplayRect(pParticle);
		if (m_rect.Right() <= pParticle->dst.x)
			pParticle->bAlive = 0;
		if (m_rect.Bottom() <= pParticle->dst.y)
			pParticle->bAlive = 0;
		if (pParticle->bAlive == 0)
		{
			delete pParticle;
			it = m_list.erase(it);
		}
		else
			++it;
	}

	for (int n = m_count - m_list.size(); n != 0; n--)
	{
		sParticle* pParticle = new sParticle;
		memset(pParticle, 0, sizeof(sParticle));
		pParticle->bAlive = 1;
		pParticle->pBitmap = m_bitmaps[rand() % m_bitmaps.size()];
		pParticle->pos.x = (float)(rand() % (int)m_rect.w) + m_rect.x;
		pParticle->pos.y = m_rect.y;
		pParticle->speed =
			(float)(rand() % (int)(m_maxSpeed - m_minSpeed)) + m_minSpeed;
		pParticle->vel.x = m_velMin.x;
		pParticle->vel.y = m_velMin.y;
		float dx = m_velMax.x - m_velMin.x;
		if (g_EPSILON <= Wabs(dx))
			pParticle->vel.x = (float)(rand() % (int)Wabs(dx)) + m_velMin.x;
		float dy = m_velMax.y - m_velMin.y;
		if (g_EPSILON <= Wabs(dy))
			pParticle->vel.y = (float)(rand() % (int)Wabs(dy)) + m_velMin.y;
		if (m_style == STYLE_BLINK)
		{
			pParticle->alphaDir = rand() % 2;
			pParticle->alpha = (float)(rand() % 0x7d) + 125.0f;
		}
		else
		{
			pParticle->alphaDir = 0;
			pParticle->alpha = 255.0f;
		}
		m_list.push_back(pParticle);
	}
}

void CParticle2D::Display()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	for (std::list<sParticle*>::iterator it = m_list.begin();
		it != m_list.end(); ++it)
	{
		sParticle* pParticle = *it;
		pGDI->DrawTexture(pParticle->pBitmap, pParticle->src, pParticle->dst,
			(unsigned char)pParticle->alpha << 24 | 0xffffff, 0);
	}
}

void CParticle2D::Release()
{
	for (std::list<sParticle*>::iterator it = m_list.begin();
		it != m_list.end();)
	{
		if (*it)
		{
			delete *it;
			it = m_list.erase(it);
		}
		else
			++it;
	}
	m_list.clear();
}
