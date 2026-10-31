#include "minatl.h"
// A357: o 983 sempre tem o pos-efeito (HDR/DOF/FXAA); nao depender so do DEFS do pgbuild (o a353-1 linkou um
// taskmanager.obj compilado sem a chave e perdeu os alvos fp16 / a estrela). JP983_SEM_POSTEFFECT desliga.
#if !defined(JP983_POSTEFFECT) && !defined(JP983_SEM_POSTEFFECT)
#define JP983_POSTEFFECT
#endif
#include "projectg.h"
#include "scenemanager.h"
#include "vc10_layout.h"
#include <stddef.h>
#include "wbone.h"
#include "beach.h"
#include "sea.h"
#include "obmarker.h"
#include "weather.h"
#include "sky.h"
#include "clientsetting.h"
#include "wtextureview.h"
#include "chatmsg.h"
#include "shadow.h"
#include "golfball.h"
#include "quadtree.h"
#include "snow.h"
#include "rain.h"
#include "icesnow.h"
#include "implflutter.h"
#include "jp/abbotpowder.h"
#include "jp/weathereffect.h"
#include "lobbybg.h"
#include "taskmanager.h"
#include "golftask.h"
#include "obmarker.h"
#include "autotarget.h"
#include "pool.h"
#include "river.h"
#include "mathconsts.h"
#ifdef JP983_POSTEFFECT
#include "jp/posteffect983.h"
#endif


// HACK: the original object carries g_PI next to g_HUGE/g_EPSILON at the top of
// .rdata (inlines not recovered yet); this unused helper makes the compiler emit it.
static float GetPi()
{
	return g_PI;
}

extern int g_nDrawPoly;
extern int g_nModel;
extern int g_nDrawModel;

struct RenderOrder
{
	bool operator()(const SceneElement& a, const SceneElement& b);
};

std::list<TransparentElement> CSceneManager::m_transparent3; // 983: não existe como membro (PROVISÓRIO)

CSceneManager::CSceneManager()
	: m_beachList(16, 16),
	  m_waveletList(16, 16),
	  m_poolList(16, 16),
	  m_riverList(16, 16),
	  m_pSea(NULL),
	  m_pMarker(NULL),
	  m_pWeather(NULL),
	  m_pCloud(NULL),
	  m_bFog(true),
	  m_bLod(false),
	  m_bShaderCaps(false),
	  m_bShowTag(false),
	  m_pFont(NULL),
	  m_bTransparent(true),
	  m_bKeepAlpha(false),
	  m_debugDraw()
{
	// D861: jp/scenemanagerjp.cpp le a lista crua (cabeca 0xbc, tamanho 0xc0 no VC10) com VC10_OFS_BO
	C_ASSERT(offsetof(CSceneManager, m_elements) == VC10_OFS_BO(0xbc));
	// D873: visao reduzida (scenemanager.h): m_bShowTag/m_bTransparent com o mesmo nome da completa
	C_ASSERT(offsetof(CSceneManager, m_bShowTag) == VC10_OFS_BO(0xb4) + REBANG_SCENE_PADB4 &&
		offsetof(CSceneManager, m_bTransparent) == offsetof(CSceneManager, m_bShowTag) + 0x13 && sizeof(CSceneManager) == offsetof(CSceneManager, m_bTransparent) + 8);
	m_pMarker = new CMarker;

	int caps;
	g_resrcmng->VideoReference()->Command(WX_VDEV_GET_CAPS, 1, (int)&caps);
	if (caps)
		m_bShaderCaps = true;

	m_bLod = COption::Instance()->vIsLodEnabled();
	m_pFont = CChatMsg::Instance()->GetMaskedFont();
}

CSceneManager::~CSceneManager()
{
	for (std::list<SceneElement>::iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		if ((*it).texView)
		{
			delete (*it).texView;
			(*it).texView = NULL;
		}
	}

	m_elements.clear();
	m_custom1.clear();
	m_custom2.clear();
	m_transparent3.clear();
	m_transparent2.clear();
	m_transparent1.clear();

	Clear();

	if (m_pMarker)
	{
		delete m_pMarker;
		m_pMarker = NULL;
	}
	if (m_pWeather)
	{
		delete m_pWeather;
		m_pWeather = NULL;
	}
	if (m_pCloud)
	{
		delete m_pCloud;
		m_pCloud = NULL;
	}
}

void CSceneManager::RegisterElement(eObjType type, WPuppet* puppet,
	CRenderFuncPtr* func, const char* name, unsigned long flag, Waabb* aabb)
{
	SceneElement elem;
	elem.type = type;
	elem.puppet = puppet;
	elem.func = func;
	elem.aabb = aabb;

	if (name)
		strcpy(elem.name, name);
	else
		elem.name[0] = '\0';

	switch (type)
	{
	case OBJ_FUNC:
	case OBJ_CUSTOM:
	case OBJ_EFFECT:
		flag |= 0xb;
		break;
	case OBJ_MODEL:
		flag |= 6;
		break;
	case OBJ_ACTOR:
		flag |= 7;
		break;
	}

	elem.flag = flag;

	if (m_bLod && !(flag & 2))
	{
		WTextureView* view = new WTextureImposterView(puppet);
		elem.texView = view;
		view->SetResourceManager(g_resrcmng);

		WTextureView::TextureParam param;
		param.m_numRts = 1;
		param.m_rtTexInfo[0].m_texStyle = 0x12000;
		param.m_rtTexInfo[0].m_sizeInfo = WRenderToTextureSizeInfo::SIZE_ABS;
		param.m_rtTexInfo[0].m_needToClear = true;
		param.m_rtTexInfo[0].m_clearClr = 0;
		param.m_depthSurfInfo.m_surfUsage =
			WRenderToTextureParam::DepthSurfInfo::USE_SHARED_SURFACE;
		param.m_depthSurfInfo.m_sizeInfo = WRenderToTextureSizeInfo::SIZE_ABS;
		param.m_depthSurfInfo.m_needToClear = true;
		param.m_depthSurfInfo.m_clearZ = 1.0f;
		view->SetTextureParam(param);
	}

	m_elements.push_back(elem);
	m_bDirty = true;
}

void CSceneManager::RegisterElement(SceneElemRegister& reg)
{
	RegisterElement(reg.type, reg.puppet, reg.func, reg.name, reg.flag,
		reg.aabb);
}

void CSceneManager::ReleaseElement()
{
	std::list<SceneElement>::iterator it = m_elements.begin();
	while (it != m_elements.end())
	{
		if ((*it).flag & 8)
		{
			++it;
			continue;
		}

		if ((*it).texView)
		{
			delete (*it).texView;
			(*it).texView = NULL;
		}
		it = m_elements.erase(it);
	}
}

void CSceneManager::DeleteElement(const char* name)
{
	// 983 009b4130: sem teste de conteudo local
	std::list<SceneElement>::iterator it = m_elements.begin();
	while (it != m_elements.end())
	{
		if (strcmp((*it).name, name) == 0)
		{
			if ((*it).texView)
			{
				delete (*it).texView;
				(*it).texView = NULL;
			}
			it = m_elements.erase(it);
		}
		else
			++it;
	}
}

void CSceneManager::SetLOD(bool bLod, bool bUpdate)
{
	m_bLod = bLod && COption::Instance()->vIsLodEnabled();

	if (!bUpdate)
		return;

	std::list<SceneElement>::iterator it;
	if (m_bLod)
	{
		for (it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			if ((*it).flag & 2)
				continue;
			if ((*it).texView || !(*it).puppet)
				continue;

			(*it).texView = new WTextureImposterView((*it).puppet);
			(*it).texView->SetResourceManager(g_resrcmng);

			WTextureView::TextureParam param;
			param.m_numRts = 1;
			param.m_rtTexInfo[0].m_texStyle = 0x12000;
			param.m_rtTexInfo[0].m_sizeInfo =
				WRenderToTextureSizeInfo::SIZE_ABS;
			param.m_rtTexInfo[0].m_needToClear = true;
			param.m_rtTexInfo[0].m_clearClr = 0;
			param.m_depthSurfInfo.m_surfUsage =
				WRenderToTextureParam::DepthSurfInfo::USE_SHARED_SURFACE;
			param.m_depthSurfInfo.m_sizeInfo =
				WRenderToTextureSizeInfo::SIZE_ABS;
			param.m_depthSurfInfo.m_needToClear = true;
			param.m_depthSurfInfo.m_clearZ = 1.0f;
			(*it).texView->SetTextureParam(param);
		}
	}
	else
	{
		for (it = m_elements.begin(); it != m_elements.end(); ++it)
		{
			if ((*it).texView)
			{
				delete (*it).texView;
				(*it).texView = NULL;
			}
		}
	}
}

#define MAKE_FOGCOLOR(r, g, b) \
	(((((unsigned char)(r) << 8) | (unsigned char)(g)) << 8) | \
		(unsigned char)(b))

unsigned long __fastcall ModifyColor(unsigned long color, float f);

void CSceneManager::SetReflection(bool enable)
{
	for (CPool* pool = m_poolList.Start(); pool; pool = m_poolList.Next())
		pool->SetReflection(enable);
}

void CSceneManager::UpdateSea()
{
	if (m_pSea)
		m_pSea->LoadTexture();
}

void CSceneManager::UpdateWind()
{
	if (m_pWeather)
		m_pWeather->UpdateWind();
}

void CSceneManager::UpdateBright()
{
	// HACK: dead reference; the original compiles this function one wave after
	// UpdateFog (COMDAT order).
	if (0)
		UpdateFog(false);
	g_view->SetFogState(m_fogStart * 3.2f, m_fogEnd * 3.2f, m_fogColor);
	m_bFog = !(GOLFDOC()->m_gameMode & 0x200);
}

void CSceneManager::UpdateFog(bool bLoad)
{
	if (bLoad)
	{
		const char* name =
			Doc()->m_golfGame.pCourse ? Doc()->m_golfGame.pCourse->Data : NULL;

		cFile* file = g_resrcmng->GetCFile(MakeStr("%s_fog.txt", name), -1);
		if (file)
		{
			int c[3];
			file->Scan("%d %d %d\n", &c[0], &c[1], &c[2]);
			m_fogColor = ModifyColor(MAKE_FOGCOLOR(c[0], c[1], c[2]),
				GOLFDOC()->m_bright);
			file->Scan("%f %f\n", &m_fogStart, &m_fogEnd);
			CloseCFile(file);
		}
		else
		{
			m_fogColor = 0xffffff;
			m_fogStart = 100.0f;
			m_fogEnd = 400.0f;
		}
	}

	g_view->SetFogState(m_fogStart * 3.2f, m_fogEnd * 3.2f, m_fogColor);
	m_bFog = !(GOLFDOC()->m_gameMode & 0x200);
}

void CSceneManager::UpdateFog(const char* mapName)
{
	cFile* file = g_resrcmng->GetCFile(MakeStr("%s_fog.txt", mapName), -1);
	if (file)
	{
		int c[3];
		file->Scan("%d %d %d\n", &c[0], &c[1], &c[2]);
		m_fogColor =
			ModifyColor(MAKE_FOGCOLOR(c[0], c[1], c[2]), GOLFDOC()->m_bright);
		file->Scan("%f %f\n", &m_fogStart, &m_fogEnd);
		CloseCFile(file);
	}
	else
	{
		m_fogColor = 0xffffff;
		m_fogStart = 100.0f;
		m_fogEnd = 400.0f;
	}

	g_view->SetFogState(m_fogStart * 3.2f, m_fogEnd * 3.2f, m_fogColor);
	m_bFog = true;
}

WPuppet* CSceneManager::GetPuppet(const char* name)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
			return (*it).puppet;
	}
	return NULL;
}

eObjType CSceneManager::GetType(const char* name)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
			return (*it).type;
	}
	return OBJ_NONE;
}

void CSceneManager::SetAlpha(const char* name, unsigned char alpha)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
			(*it).alpha = alpha;
	}
}

void CSceneManager::SetVisible(const char* name, bool bVisible)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
			(*it).visible = bVisible;
	}
}

void CSceneManager::SetPuppet(const char* name, WPuppet* puppet)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
			(*it).puppet = puppet;
	}
}

void CSceneManager::SetType(const char* name, eObjType type)
{
	if (name)
	{
		std::list<SceneElement>::iterator it =
			std::find(m_elements.begin(), m_elements.end(), name);
		if (it != m_elements.end())
		{
			(*it).type = type;
			m_bDirty = true;
		}
	}
}

void CSceneManager::Process(float dt)
{
	if (GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->Process(dt);

	if (m_pWeather)
		m_pWeather->Process(dt);
	if (m_pCloud)
		m_pCloud->Process(dt);
	if (m_pSea)
		m_pSea->Process(dt);

	for (CPool* pool = m_poolList.Start(); pool; pool = m_poolList.Next())
		pool->Process(dt);
	for (CBeach* beach = m_beachList.Start(); beach; beach = m_beachList.Next())
		beach->Process(dt);
	for (ntRiver* river = m_riverList.Start(); river;
		river = m_riverList.Next())
		river->Process(dt);
	for (CWavelet* wavelet = m_waveletList.Start(); wavelet;
		wavelet = m_waveletList.Next())
		wavelet->Process(dt);
}

void CSceneManager::DropTest()
{
	WPolySoup::sModel* model = GOLFDOC()->m_pPolySoup->FindModel(NULL);

	Waabb box = model->aabb;
	int countX = 1 - (int)((box.max.x - box.min.x) / -16.0f);
	int countZ = 1 - (int)((box.max.z - box.min.z) / -16.0f);
	m_drops.clear();

	WVector pos = box.min;
	for (int z = 0; z < countZ; z++)
	{
		for (int x = 0; x < countX; x++)
		{
			sDrop drop;
			pos.x += 16.0f;

			// golfdoc.h does not expose the CAutoTarget* at +0x10 yet
			switch (GOLFDOC()->m_pAutoTarget->CalcDropPos(pos))
			{
			case 0:
				drop.pos = GolfBall().m_pos;
				drop.color = 0xffff0000;
				m_drops.push_back(drop);
				break;
			case 2:
				drop.pos = GolfBall().m_pos;
				drop.color = 0xff0000ff;
				m_drops.push_back(drop);
				break;
			}
		}
		pos.z += 16.0f;
		pos.x = box.min.x;
	}

	unsigned char current = GOLFDOC()->m_currentPlayer;
	sPlayerData* player = GOLFDOC()->GetPlayer(current);
	GolfBall().m_pos = player->pos;
}

bool RenderOrder::operator()(const SceneElement& a, const SceneElement& b)
{
	return a.type < b.type;
}

void CSceneManager::Prerender(WView* view)
{
	bool bLod = m_bLod;
	bool b129 = m_bKeepAlpha;
	bool b128 = m_bTransparent;
	m_bLod = false;
	m_bKeepAlpha = false;
	m_bTransparent = false;

	for (CPool* pool = m_poolList.Start(); pool; pool = m_poolList.Next())
		pool->Prerender(view);

	m_bLod = bLod;
	m_bKeepAlpha = b129;
	m_bTransparent = b128;
}

void CSceneManager::Render(WView* view, unsigned long mask)
{
	// D905: 983 009b55b0 desliga a neblina tambem com gameMode 0x2000000
	view->SetFogEnable(m_bFog && GOLFDOC()->m_gameMode != 0x200000 && GOLFDOC()->m_gameMode != 0x2000000);
	g_nDrawPoly = 0;
	g_nDrawModel = 0;
	g_nModel = GOLFDOC()->m_pPolySoup->GetModelNum();

	m_custom1.clear();
	m_custom2.clear();
	m_transparent3.clear();
	m_transparent2.clear();
	m_transparent1.clear();

	if (m_bDirty)
	{
		m_elements.sort(RenderOrder());
		m_bDirty = false;
	}

	if ((mask & 0x80) && GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->Render(view);
	if (mask & 0xc)
		Render_BuildImposter(view);
	if (mask & 1)
		Render_BaseAndShadow(view, mask);
	// D904: 983 009b55b0 -> 009af470 (0x100): nuvens do ceu (CSky 00706a50) antes dos elementos
	if ((mask & 0x100) && GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->RenderClouds983(view);
	int count = Render_RenderElement(view, mask);
#ifdef JP983_POSTEFFECT
	// 983 009b55b0 (009b57a0-009b57f6): em CGolfTask com o pos-efeito (+0x68) ativo os efeitos nao saem aqui; o
	// CTask::Display chama Render_RenderEffect depois do EndScene (009bb629)
	CTask* pFxTask = AfxGetTask();
	if ((mask & 0x20) && !(pFxTask && pFxTask->DynamicCast(&CGolfTask::m_RTTI) && pFxTask->GetPostEffect983() &&
		(pFxTask->GetPostEffect983())->IsActive()))
#else
	if (mask & 0x20) // D905: 983 0x20 -> 009b1180 (+0xd4)
#endif
		Render_RenderEffect(view);
	// D904: 983 009ac6f0 Render_RenderOBMarker com 0x400 (0x100 = nuvens)
	if ((mask & 0x400) && m_pMarker)
		m_pMarker->Display();
	// D905: 983 0x800 -> 009ac710 (mar +0xa8) e 009af490 (praia/ondas/piscina/rio); 0x200 so o lens flare
	if ((mask & 0x800) && m_pSea)
		m_pSea->Render();
	if (mask & 0x800)
		Render_RenderWaterStuff(view);
	// A353: 983 009b55b0 desenha billboard (+0xe0) e blend (+0xec) ANTES do Flush(1,0) (passada opaca com Z) e o clima
	// entre Flush(1,0) e Flush(0,1); no 645 iam depois do Flush(1) = FlushOnePass sem ZWRITE (d3dtrace a351-1 x orig-c291-1)
	if (mask & 0xc)
	{
		Render_RenderBillboardGroup(view);
		Render_RenderBlendGroup(view, m_transparent2, 0, count);
	}
	view->Flush(1, 0);
	if (mask & 0x1000)
		Render_RenderWeather(view);
	view->Flush(0, 1);
	// A369: 983 009b55b0 -> 009af5f0 (0x200): manchas 2D do lens flare do ceu (CSky 007060f0)
	if ((mask & 0x200) && GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->RenderLensFlareOverlay983(view);
	if (mask & 0x40) // D905: 983 0x40 -> 009b1220 (+0xc8)
		Render_RenderCustom(view);
	// A439: o 983 (009b55b0) nao tem a lista near do 645 (Render_RenderNearGroup/Flush(2)): objeto a <= 800 da
	// camera com alpha != 0xff so recebe SetAlpha em RenderElement 009b2df0 e nao e desenhado
	if (mask & 0x1000)
		Render_RenderWeatherFullScreenOverlay(view);
	if (mask & 0x2000)
		Render_RenderDropPosition(view);
	view->Flush(0, 1);
	DrawDebugLine(view);
}

void CSceneManager::Render_RenderSky(WView* view)
{
	if (GOLFDOC()->m_pSky)
		GOLFDOC()->m_pSky->Render(view);
}

void CSceneManager::Render_BuildImposter(WView* view)
{
	if (!m_bLod)
		return;

	WxBatchState batch;
	for (std::list<SceneElement>::iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		SceneElement* elem = &*it;
		if (!elem || !(*it).visible || (*it).type == OBJ_CUSTOM ||
			(*it).type == OBJ_EFFECT || (*it).func)
			continue;
		WPuppet* puppet = (*it).puppet;
		if (!puppet || (*it).type != OBJ_STATIC || !puppet->m_rootbone)
			continue;
		WSphere* sph = &puppet->m_boundSphere;
		if (!view->InFrustum(*sph) || !view->InFrustum(puppet->m_bound))
			continue;
		WTextureImposterView* imp = (WTextureImposterView*)(*it).texView;
		if (!imp)
			continue;

		WVector d;
		const WMatrix& cam = view->GetCamera();
		d = cam.pivot - sph->pos;
		d.y = 0.0f;
		float dist = d.Magnitude();
		// 983 009b0f30: so desenha se o Begin (00bb1750) abriu o render-to-texture;
		// sem Flush(0) proprio antes do End (00bb1350)
		if (!(dist <= 800.0f) && imp->NeedToUpdate(cam) &&
			(*it).texView->Begin())
		{
			(*it).puppet->SetAlpha(0xff);
			if (!m_bShaderCaps)
			{
				(*it).puppet->Render((*it).texView, true,
					(*it).puppet->m_mat.xa.Magnitude(), false, false, false);
			}
			else
			{
				(*it).texView->xSetLight(0,
					(*it).puppet->m_rootbone->GetLight());
				(*it).puppet->Transform((*it).texView,
					WVectorLen((*it).puppet->m_mat.xa), 2);
				(*it).puppet->m_rootbone->xRender((*it).texView, &batch, false,
					0, 0);
			}
			(*it).texView->End();
		}
	}
}

void CSceneManager::Render_BaseAndShadow(WView* view, unsigned long mask)
{
	for (std::list<SceneElement>::iterator it = m_elements.begin();
		it != m_elements.end(); it++)
	{
		if (!&*it || !(*it).visible || !(mask & (1 << (*it).type)) ||
			(*it).type != OBJ_BASE)
			continue;
		WPuppet* puppet = (*it).puppet;
		if (!puppet || !puppet->m_rootbone)
			return;
		if (!view->InFrustum(puppet->m_boundSphere) ||
			!view->InFrustum(puppet->m_bound))
			return;
		++g_nDrawModel;
		g_nDrawPoly += (*it).puppet->m_faceNum;
		DrawPuppet(view, (*it).puppet, (*it).aabb);
		view->Flush(1, 1); // B1582: 983 009b2250 push 1, 1
		if (CShadowManager::Instance())
			RenderShadow(view, mask);
		return;
	}
}

int CSceneManager::Render_RenderElement(WView* view, unsigned long mask)
{
	int count = 0;
	for (std::list<SceneElement>::iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		if (!&*it || !(*it).visible || !(mask & (1 << (*it).type)) ||
			(*it).type == OBJ_BASE)
			continue;

		// D905: 983 009b2df0 (009b2e70-009b2edd): tipo 6 sempre na lista +0xc8 (m_custom1, desenhada no fim com 0x40);
		// tipo 5 (particulas etc.) com 0x20 na +0xd4 (m_custom2, antes do Flush(1,0); com pos-efeito vai depois do EndScene)
		if ((*it).type == OBJ_EFFECT)
		{
			m_custom1.push_back(CustomElement((*it).type, (*it).func));
			continue;
		}
		if ((*it).type == OBJ_CUSTOM && (mask & 0x20))
		{
			m_custom2.push_back(CustomElement((*it).type, (*it).func));
			continue;
		}
		if ((*it).func)
		{
			(*it).func->Display();
			continue;
		}

		WPuppet* puppet = (*it).puppet;
		if (!puppet || !puppet->m_rootbone)
			continue;
		if ((*it).type == OBJ_STATIC)
		{
			if (!view->InFrustum(puppet->m_boundSphere) ||
				!view->InFrustum(puppet->m_bound))
				continue;
		}
		else if (!view->InFrustum(puppet->m_boundSphere))
			continue;

		if (!m_bLod || !(*it).texView)
		{
			if ((*it).type == OBJ_STATIC || (*it).type == OBJ_MODEL)
			{
				++g_nDrawModel;
				g_nDrawPoly += (*it).puppet->m_faceNum;
			}
			if ((*it).type == OBJ_ACTOR)
			{
				(*it).alpha = (*it).puppet->m_rootbone->GetAlpha();
			}
			else
			{
				WVector d;
				if ((*it).puppet &&
					(d = view->GetCamera().pivot -
							(*it).puppet->m_boundSphere.pos,
						d.y = 0.0f, d.Magnitude() <= 800.0f))
					(*it).alpha = OverlapCamera(view, *it);
				else
					(*it).alpha = 0xff;
			}
		}
		else
		{
			WVector d;
			d = view->GetCamera().pivot - (*it).puppet->m_boundSphere.pos;
			d.y = 0.0f;
			float dist = d.Magnitude();
			if (dist < 960.0f)
			{
				++g_nDrawModel;
				g_nDrawPoly += (*it).puppet->m_faceNum;
			}
			if (dist <= 800.0f)
			{
				(*it).alpha = OverlapCamera(view, *it);
			}
			else
			{
				(*it).alpha =
					(unsigned char)(Between(0.0f, (960.0f - dist) * 0.00625f,
										1.0f) *
						255.0f);
				if (dist <= 960.0f)
				{
					m_transparent2.push_back(TransparentElement((*it).puppet,
						(*it).aabb, (*it).texView->GetTexture(0),
						(*it).puppet->m_faceNum));
					count += (*it).puppet->m_faceNum;
				}
				else
				{
					m_transparent1.push_back(TransparentElement((*it).puppet,
						(*it).aabb, (*it).texView->GetTexture(0), 0));
				}
			}
		}

		if ((*it).alpha != (*it).puppet->m_rootbone->GetAlpha())
			(*it).puppet->SetAlpha((*it).alpha);
		if (!m_bShaderCaps)
		{
			if ((*it).alpha == 0xff)
				DrawPuppet(view, (*it).puppet, (*it).aabb);
		}
		else if ((*it).type != OBJ_STATIC)
			DrawPuppet(view, (*it).puppet, (*it).aabb);
	}
	return count;
}

void CSceneManager::Render_RenderEffect(WView* view)
{
	std::list<CustomElement>::iterator it;
	for (it = m_custom2.begin(); it != m_custom2.end(); ++it)
	{
		if ((*it).func)
			(*it).func->DisplayAll();
	}
}

void CSceneManager::Render_RenderOBMarker(WView* view)
{
	if (m_pMarker)
		m_pMarker->Display();
}

void CSceneManager::Render_RenderSea(WView* view)
{
	if (m_pSea)
		m_pSea->Render();
}

void CSceneManager::Render_RenderWaterStuff(WView* view)
{
	for (CBeach* beach = m_beachList.Start(); beach; beach = m_beachList.Next())
		beach->Render();
	for (ntRiver* river = m_riverList.Start(); river;
		river = m_riverList.Next())
		river->Render();
	for (CPool* pool = m_poolList.Start(); pool; pool = m_poolList.Next())
		pool->Render(view);
	for (CWavelet* wavelet = m_waveletList.Start(); wavelet;
		wavelet = m_waveletList.Next())
		wavelet->Render();
}

struct Greater
{
	Greater(const WVector& dir);
	bool operator()(const TransparentElement& a, const TransparentElement& b);

	WVector m_dir;
};

unsigned char CSceneManager::OverlapCamera(WView* view,
	const SceneElement& elem)
{
	if (m_bKeepAlpha)
		return elem.alpha;
	if (elem.flag & 1)
		return 0xff;
	if (!m_bTransparent)
		return 0xff;

	Waabb box = elem.puppet->m_bound;
	const WMatrix& cam = view->GetCamera();

	float dist2 = 0.0f;
	float d;
	if (cam.xm < box.min.x)
	{
		d = cam.xm - box.min.x;
		dist2 = d * d;
	}
	else if (cam.xm > box.max.x)
	{
		d = cam.xm - box.max.x;
		dist2 = d * d;
	}
	if (cam.ym < box.min.y)
	{
		d = cam.ym - box.min.y;
		dist2 = d * d + dist2;
	}
	else if (cam.ym > box.max.y)
	{
		d = cam.ym - box.max.y;
		dist2 = d * d + dist2;
	}
	if (cam.zm < box.min.z)
	{
		d = cam.zm - box.min.z;
		dist2 = d * d + dist2;
	}
	else if (cam.zm > box.max.z)
	{
		d = cam.zm - box.max.z;
		dist2 = d * d + dist2;
	}

	float half = sqrtf((box.max.x - box.min.x) * (box.max.x - box.min.x) +
					 (box.max.y - box.min.y) * (box.max.y - box.min.y) +
					 (box.max.z - box.min.z) * (box.max.z - box.min.z)) *
		0.5f;
	if (half > 96.0f)
		half = 96.0f;
	if (half * half < dist2)
		return 0xff;

	float ratio = sqrtf(dist2) / half;
	if (ratio < 0.25f)
		ratio = 0.25f;
	else if (ratio > 1.0f)
		ratio = 1.0f;
	return (unsigned char)(ratio * 255.0f);
}

Greater::Greater(const WVector& dir)
	: m_dir(dir)
{
}

bool Greater::operator()(const TransparentElement& a,
	const TransparentElement& b)
{
	const WVector& pa = a.puppet->GetBoundSphere().pos;
	const WVector& pb = b.puppet->GetBoundSphere().pos;
	return pa.z * m_dir.z + pa.y * m_dir.y + pa.x * m_dir.x >
			pb.z * m_dir.z + pb.y * m_dir.y + pb.x * m_dir.x
		? TRUE
		: FALSE;
}

void CSceneManager::Render_RenderBillboardGroup(WView* view)
{
	if (m_transparent1.size() != 0)
	{
		for (std::list<TransparentElement>::iterator it =
				 m_transparent1.begin();
			it != m_transparent1.end(); ++it)
			DrawImposter(view, *it);

		if (!m_bShaderCaps)
			view->Flush(0, 1); // B1582: 983 009b11ff
	}
}

void CSceneManager::Render_RenderBlendGroup(WView* view,
	std::list<TransparentElement>& elements, int depth, int count)
{
	if (elements.size() == 0)
		return;

	std::list<TransparentElement>::iterator it;
	if (m_bShaderCaps)
	{
		for (it = elements.begin(); it != elements.end(); it++)
			DrawImposter(view, *it);
		return;
	}

	if (elements.size() != 1 && count > 0x3ff)
	{
		bool bSplit = false;
		std::list<TransparentElement> group[2];
		WVector dir;
		if (depth % 2 == 0)
			dir = view->GetCamera().xa;
		else
			dir = view->GetCamera().za;
		elements.sort(Greater(dir));

		float prevLen = -1.0f;
		int count0 = 0;
		int count1 = 0;
		it = elements.begin();
		if (it != elements.end())
		{
			do
			{
				std::list<TransparentElement>::iterator next = it;
				++next;
				if (next == elements.end())
					break;
				if (!bSplit)
				{
					group[0].push_back(*it);
					count0 += (*it).polyNum;
					if (count / 3 <= count0)
					{
						if (count * 2 / 3 < count0)
						{
							bSplit = true;
						}
						else
						{
							WVector d = (*it).puppet->m_boundSphere.pos -
								(*next).puppet->m_boundSphere.pos;
							float dot = dir.x * d.x + dir.z * d.z + dir.y * d.y;
							float len;
							if (prevLen <= 0.0f)
							{
								const Waabb& b = (*it).puppet->m_bound;
								WVector ext;
								ext.z = b.max.z - b.min.z;
								ext.x = b.max.x - b.min.x;
								ext.y = 0.0f;
								len = WVectorLen(ext);
							}
							else
								len = prevLen;
							const Waabb& b = (*next).puppet->m_bound;
							WVector ext;
							ext.z = b.max.z - b.min.z;
							ext.x = b.max.x - b.min.x;
							ext.y = 0.0f;
							prevLen = WVectorLen(ext);
							if (prevLen + len < Abs(dot))
								bSplit = true;
						}
					}
				}
				else
				{
					group[1].push_back(*it);
					count1 += (*it).polyNum;
				}
				it = next;
			} while (it != elements.end());
		}
		group[1].push_back(*it);
		Render_RenderBlendGroup(view, group[0], depth + 1, count0);
		Render_RenderBlendGroup(view, group[1], depth + 1,
			count1 + (*it).polyNum);
	}

	for (it = elements.begin(); it != elements.end(); it++)
	{
		DrawPuppet(view, (*it).puppet, (*it).aabb);
		DrawImposter(view, *it);
	}
	view->Flush(0);
}

void CSceneManager::Render_RenderWeather(WView* view)
{
	if (!AfxGetTask()->GetActor("LobbyBg") && m_pWeather)
		m_pWeather->Display();
	if (m_pCloud)
		m_pCloud->Display();
}

void CSceneManager::Render_RenderCustom(WView* view)
{
	std::list<CustomElement>::iterator it;
	for (it = m_custom1.begin(); it != m_custom1.end(); ++it)
	{
		if ((*it).func)
			(*it).func->DisplayAll();
	}
}

void CSceneManager::Render_RenderNearGroup(WView* view)
{
	if (m_transparent3.size() == 0 || m_bShaderCaps)
		return;
	m_transparent3.sort(Greater(view->GetCamera().za));
	for (std::list<TransparentElement>::iterator it = m_transparent3.begin();
		it != m_transparent3.end(); ++it)
	{
		DrawPuppet(view, (*it).puppet, (*it).aabb);
		view->Flush(2);
	}
}

void CSceneManager::Render_RenderWeatherFullScreenOverlay(WView* view)
{
	if (!AfxGetTask()->GetActor("LobbyBg") && m_pWeather)
		m_pWeather->DisplayFullScreenOverlay();
	if (m_pCloud)
		m_pCloud->DisplayFullScreenOverlay();
}

void CSceneManager::Render_RenderDropPosition(WView* view)
{
	if (!(GOLFDOC()->m_gameMode & 0x200))
		return;

	for (std::list<sDrop>::iterator it = m_drops.begin(); it != m_drops.end();
		++it)
	{
		const WMatrix& cam = view->GetCamera();
		WVector a, b;
		a.x = (cam.xx + cam.xy) * 4.0f + (*it).pos.x;
		a.y = (cam.yy + cam.yx) * 4.0f + (*it).pos.y;
		a.z = (cam.zy + cam.zx) * 4.0f + (*it).pos.z;
		b.x = (*it).pos.x - (cam.xy + cam.xx) * 4.0f;
		b.y = (*it).pos.y - (cam.yy + cam.yx) * 4.0f;
		b.z = (*it).pos.z - (cam.zy + cam.zx) * 4.0f;
		view->DrawLine(b, (*it).color, a, (*it).color, 0x300000);
		a.x = (cam.xx - cam.xy) * 4.0f + (*it).pos.x;
		a.y = (cam.yx - cam.yy) * 4.0f + (*it).pos.y;
		a.z = (cam.zx - cam.zy) * 4.0f + (*it).pos.z;
		b.x = (*it).pos.x - (cam.xx - cam.xy) * 4.0f;
		b.y = (*it).pos.y - (cam.yx - cam.yy) * 4.0f;
		b.z = (*it).pos.z - (cam.zx - cam.zy) * 4.0f;
		view->DrawLine(b, (*it).color, a, (*it).color, 0x300000);
	}

	CGolfDoc* doc = GOLFDOC();
	if (!doc->m_bDropRequest || !doc->m_bDropEnable)
		return;

	WVector pin = doc->GetHoleData().pin;
	CGolfBall& ball = GolfBall();
	WTVertex v[3];
	WTVertex* fan[4] = { &v[0], &v[1], &v[2], NULL };
	v[0].diffuse = 0x66000000;
	v[1].diffuse = 0x66000000;
	v[2].diffuse = 0x66000000;
	v[2].SetPosition(GOLFDOC()->GetHoleData().pin);
	vtr[0].diffuse = 0xff000000;
	vtr[1].diffuse = 0xff000000;
	vtr[2].diffuse = 0xff000000;
	vtr[3].diffuse = 0xff000000;

	float radius = sqrtf((ball.m_pos.z - pin.z) * (ball.m_pos.z - pin.z) +
					   (ball.m_pos.x - pin.x) * (ball.m_pos.x - pin.x)) -
		25.6f;
	for (int i = 0; i < 128; i++)
	{
		int j;
		for (j = 0; j < 2; j++)
		{
			int angle = (i + (j & 1)) % 128;
			float c = cosf(angle * 0.049087387f);
			WVector center = GOLFDOC()->GetHoleData().pin;
			float s = sinf(angle * 0.049087387f);
			v[j].x = c * radius + center.x;
			v[j].y = center.y + radius * 0.0f;
			v[j].z = center.z + s * radius;
		}
		// 983 009b1250: face 0x200 (0x400 do 645 = face oposta no 983, wview.h)
		view->DrawPolygonFan(fan, 0x300000, 0x200, false);

		for (j = 0; j < 4; j++)
		{
			float r = radius;
			if (j & 2)
				r += 1.0f;
			int angle = (i + (j & 1)) % 128;
			float c = cosf(angle * 0.049087387f);
			WVector center = GOLFDOC()->GetHoleData().pin;
			float s = sinf(angle * 0.049087387f);
			vtr[j].x = c * r + center.x;
			vtr[j].y = center.y + 0.0f * r;
			vtr[j].z = center.z + r * s;
		}
		view->DrawPolygonFan(vtx, 0x300000, 0x200, false);
	}
}

WTVertex CSceneManager::vtr[4];
WTVertex* CSceneManager::vtx[5] = { &vtr[0], &vtr[1], &vtr[3], &vtr[2], NULL };

void CSceneManager::DrawImposter(WView* view, const TransparentElement& elem)
{
	float r = elem.puppet->m_boundSphere.radius * -1.0f;
	vtr[0].x = r * view->GetCamera().xx + elem.puppet->m_boundSphere.pos.x;
	vtr[0].z = elem.puppet->m_boundSphere.pos.z + r * view->GetCamera().zx;
	vtr[0].y = r + r * view->GetCamera().yx + elem.puppet->m_boundSphere.pos.y;
	vtr[0].tu = 0.0f;
	vtr[0].tv = 1.0f;
	vtr[0].diffuse =
		((255 - elem.puppet->m_rootbone->GetAlpha()) << 24) | 0xffffff;

	r = elem.puppet->m_boundSphere.radius * -1.0f;
	vtr[1].x = r * view->GetCamera().xx + elem.puppet->m_boundSphere.pos.x;
	vtr[1].z = elem.puppet->m_boundSphere.pos.z + r * view->GetCamera().zx;
	vtr[1].y = r * view->GetCamera().yx + elem.puppet->m_boundSphere.pos.y +
		elem.puppet->m_boundSphere.radius;
	vtr[1].tu = 0.0f;
	vtr[1].tv = 0.0f;
	vtr[1].diffuse =
		((255 - elem.puppet->m_rootbone->GetAlpha()) << 24) | 0xffffff;

	r = elem.puppet->m_boundSphere.radius;
	vtr[2].x = r * view->GetCamera().xx + elem.puppet->m_boundSphere.pos.x;
	vtr[2].z = elem.puppet->m_boundSphere.pos.z + r * view->GetCamera().zx;
	vtr[2].y = (r * view->GetCamera().yx + elem.puppet->m_boundSphere.pos.y) -
		elem.puppet->m_boundSphere.radius * 1.0f;
	vtr[2].tu = 1.0f;
	vtr[2].tv = 1.0f;
	vtr[2].diffuse =
		((255 - elem.puppet->m_rootbone->GetAlpha()) << 24) | 0xffffff;

	vtr[3].y = elem.puppet->m_boundSphere.radius;
	vtr[3].x =
		vtr[3].y * view->GetCamera().xx + elem.puppet->m_boundSphere.pos.x;
	vtr[3].z =
		elem.puppet->m_boundSphere.pos.z + vtr[3].y * view->GetCamera().zx;
	vtr[3].y = vtr[3].y + vtr[3].y * view->GetCamera().yx +
		elem.puppet->m_boundSphere.pos.y;
	vtr[3].diffuse =
		((255 - elem.puppet->m_rootbone->GetAlpha()) << 24) | 0xffffff;
	vtr[3].tu = 1.0f;
	vtr[3].tv = 0.0f;

	view->DrawPolygonFan(vtx, (elem.texHandle & 0x7ff) | 0x10400000, 0, false);
	if (m_debugDraw.bits & 8)
		view->DrawPolygonFan(vtx, 0x8000000, 0, false);
}

void CSceneManager::DrawPuppet(WView* view, WPuppet* puppet, Waabb* aabb)
{
	puppet->Render(view, true, puppet->m_mat.xa.Magnitude(),
		view->InFrustumSafe(puppet->m_bound), false, false);
	if (m_debugDraw.bits & 1)
		view->DrawAABB(puppet->m_bound, 0xffff0000, false, 0);
	if (m_debugDraw.bits & 2)
	{
		w_bound_box* box = puppet->m_bbList.Find("char0");
		if (!box)
		{
			if (aabb)
				view->DrawAABB(*aabb, 0xff00ff00, false, 0);
		}
		else
		{
			int i = 0;
			do
			{
				WBone* bone = box->bone;
				w_bound_box_info* info = box->info;
				float sz = info->aabb.max.z - info->aabb.min.z;
				float hx = (info->aabb.max.x - info->aabb.min.x) * 0.5f +
					WVector::ONE.x * 1.6f;
				float hy = (info->aabb.max.y - info->aabb.min.y) * 0.5f +
					WVector::ONE.y * 1.6f;
				float hz = sz * 0.5f + WVector::ONE.z * 1.6f;
				Wobb obb;
				obb.extend[0].x = bone->m_matrix.xx * hx;
				obb.extend[0].y = bone->m_matrix.yx * hx;
				obb.extend[0].z = bone->m_matrix.zx * hx;
				obb.extend[1].x = bone->m_matrix.xy * hy;
				obb.extend[1].y = bone->m_matrix.yy * hy;
				obb.extend[1].z = bone->m_matrix.zy * hy;
				obb.extend[2].x = bone->m_matrix.xz * hz;
				obb.extend[2].y = bone->m_matrix.yz * hz;
				obb.extend[2].z = bone->m_matrix.zz * hz;
				bone = box->bone;
				info = box->info;
				obb.center.x = info->spherePivot.x * bone->m_matrix.xx +
					bone->m_matrix.xy * info->spherePivot.y +
					bone->m_matrix.xz * info->spherePivot.z + bone->m_matrix.xm;
				obb.center.y = bone->m_matrix.yy * info->spherePivot.y +
					bone->m_matrix.yx * info->spherePivot.x +
					bone->m_matrix.yz * info->spherePivot.z + bone->m_matrix.ym;
				obb.center.z = bone->m_matrix.zy * info->spherePivot.y +
					bone->m_matrix.zx * info->spherePivot.x +
					bone->m_matrix.zz * info->spherePivot.z + bone->m_matrix.zm;
				view->DrawOBB(obb, 0xff00ff00);
				++i;
				box = puppet->m_bbList.Find(MakeStr("char%d", i));
			} while (box);
		}
	}
	if (m_debugDraw.bits & 4)
		view->DrawSphere(puppet->GetBoundSphere(), 0xffffff00, 0x18, 0);
}

void CSceneManager::RenderShadow(WView* view, unsigned long mask)
{
	int count = 0;
	std::list<SceneElement>::iterator it = m_elements.begin();
	while (it != m_elements.end() &&
		(!&*it || !(*it).visible || !(mask & (1 << (*it).type)) ||
			(*it).type != OBJ_BASE))
		it++;
	if (!(it != m_elements.end()))
		return;
	Waabb* bound = &(*it).puppet->m_bound;
	CShadowManager::Instance()->RegisterRecver_New("bg");
	CShadowManager::Instance()->AddRecvPet_New("bg", (*it).puppet,
		bound->min.y);
	unsigned long flag;
	for (it = m_elements.begin(); it != m_elements.end(); it++)
	{
		if (&*it && (*it).visible && (mask & (1 << (*it).type)) &&
			(*it).type != OBJ_BASE && (flag = (*it).flag, flag & 0x30))
		{
			if ((*it).func)
			{
				(*it).func->DisplayShadow();
			}
			else if ((*it).puppet && (*it).puppet->m_rootbone)
			{
				CShadowManager* shadow = CShadowManager::Instance();
				shadow->RegisterCaster_New(MakeStr("caster%d", count),
					(unsigned char)~(unsigned char)flag >> 1 & 0x10, "sun",
					g_lightset);
				shadow = CShadowManager::Instance();
				shadow->AddCastPet_New(MakeStr("caster%d", count), false,
					(*it).puppet);
				++count;
			}
		}
	}
	CShadowManager::Instance()->Flush(view);
	CShadowManager::Instance()->Reset_New();
}

void CSceneManager::Clear()
{
	if (m_pSea)
	{
		delete m_pSea;
		m_pSea = NULL;
	}

	for (CPool* pool = m_poolList.Start(); pool; pool = m_poolList.Next())
		delete pool;
	m_poolList.Reset();

	for (CBeach* beach = m_beachList.Start(); beach; beach = m_beachList.Next())
		delete beach;
	m_beachList.Reset();

	for (CWavelet* wavelet = m_waveletList.Start(); wavelet;
		wavelet = m_waveletList.Next())
		delete wavelet;
	m_waveletList.Reset();

	for (ntRiver* river = m_riverList.Start(); river;
		river = m_riverList.Next())
		delete river;
	m_riverList.Reset();

	if (m_pMarker)
		m_pMarker->Reset();
}

void CSceneManager::Load()
{
	Clear();

	for (std::vector<CNodeContainer>::iterator it =
			 GOLFDOC()->m_pPolySoup->m_node.begin();
		it != GOLFDOC()->m_pPolySoup->m_node.end(); ++it)
	{
		switch ((*it).GetType())
		{
		case CNodeContainer::TYPE_SEA:
			m_pSea = new CSea;
			m_pSea->SetArea((*it).GetList());
			break;
		case CNodeContainer::TYPE_BEACH:
		{
			CBeach* beach = new CBeach;
			beach->SetWavelet((*it).GetList(), (*it).GetName().c_str(), 3);
			m_beachList += beach;
			break;
		}
		case CNodeContainer::TYPE_RIVER:
		{
			ntRiver* river = new CRiver;
			river->Setup((*it).GetList(), (*it).GetName().c_str());
			m_riverList += river;
			break;
		}
		case CNodeContainer::TYPE_RIVEREX:
		{
			ntRiver* river = new CRiver_ex;
			river->Setup((*it).GetList(), (*it).GetName().c_str());
			m_riverList += river;
			break;
		}
		case CNodeContainer::TYPE_LAVA:
		{
			ntRiver* river = new ntRiver_Lava;
			river->Setup((*it).GetList(), (*it).GetName().c_str());
			m_riverList += river;
			break;
		}
		case CNodeContainer::TYPE_WAVE:
		{
			CWavelet* wavelet = new CWavelet;
			wavelet->SetWavelet((*it).GetList(), (*it).GetName().c_str(), 2);
			m_waveletList += wavelet;
			break;
		}
		case CNodeContainer::TYPE_IB:
		case CNodeContainer::TYPE_OB:
			m_pMarker->AddList((*it).GetList());
			break;
		}
	}

	for (int i = 0; i < GOLFDOC()->m_pPolySoup->GetAreaNum(); i++)
	{
		WPolySoup::sArea* area = GOLFDOC()->m_pPolySoup->GetArea(i);
		if (area->type == 2)
		{
			CPool* pool = CPool::CreatePoolByMapType((eMapType)GetCurMap());
			pool->SetArea(area->aabb, area->name);
			m_poolList += pool;
		}
	}

	m_pMarker->GetReady();
}

bool __fastcall IsCurMapSnow();

// 983 009af7c0: nada sem o CShadowManager (o CLobbyMain::OnPreLoadInit chama antes do ntLobbyBg criar o gerente)
void __stdcall EnableShadow(bool bShadow, bool bShadowmap)
{
	if (CShadowManager::Instance())
	{
		if (COption::Instance()->vIsShadowEnabled())
			CShadowManager::Instance()->Enable(bShadow);
		if (COption::Instance()->vIsShadowmapEnabled())
			GOLFDOC()->m_pPolySoup->EnableShadowmap(bShadowmap);
	}
}

void CSceneManager::SetWeather(unsigned char weather, bool force)
{
	if (force)
	{
		if (m_pWeather)
		{
			delete m_pWeather;
			m_pWeather = NULL;
		}
		if (m_pCloud)
		{
			delete m_pCloud;
			m_pCloud = NULL;
		}
	}

	// 983 009b37a0/009ac800: particulas do mapa, sem tempo 2 e 4
	const unsigned char map = GetCurMap();
	if ((map == 0xb || map == 0xd || map == 0x13 || map == 0x14 || map == 0x15) &&
		weather != 2 && weather != 4)
	{
		if (m_pCloud)
			goto cloud_done;

		int num = 0;
		switch (COption::Instance()->vGetParticleLevel())
		{
		case 0:
			num = 900;
			break;
		case 1:
			num = 500;
			break;
		case 2:
			num = 200;
			break;
		}
		if (map == 0x13 && num > 200)
			m_pCloud = new CSnow(num / 20, "~bubble.jpg", 0, 0x80, 0x80);
		else if (map == 0xb)
			m_pCloud = new ntImplFlutter<sPetalNameHolder>(num, "", 0);
		else if (map == 0xd)
			m_pCloud = new ntImplFlutter<sSilicaParticleNameHolder>(num, "", 0);
		else if (map == 0x14)
			m_pCloud = new CAbbotPowder(num, "~glowsapiasmall.jpg", 0);
		else if (map == 0x15)
		{
			CWeatherEffect* effect = new CWeatherEffect;
			effect->AddEffect(100, "~gp_twinkle1_ball.jpg", -1, 32.0f, 32.0f, 0.2f, 1.5f, 1.5f, false, 0.0f, 1.5f, -1, 0);
			effect->AddEffect(100, "~gp_twinkle2_ball.jpg", -1, 32.0f, 32.0f, 0.2f, 1.5f, 1.5f, false, 0.0f, 1.5f, -1, 0);
			effect->AddEffect(0x48, "[2#zodiac_cloud.jpg", 2, 128.0f, 32.0f, 0.0f, 128.0f, 32.0f, true, -0.01f, 4.0f,
				0x20465bfc, 0);
			m_pCloud = effect;
		}
		if (m_pCloud && map != 0x15) // mapa 0x13 com poucas particulas fica sem nuvem
			m_pCloud->SetGravity(0.025f);
	}
	else if (m_pCloud)
	{
		delete m_pCloud;
		m_pCloud = NULL;
	}
cloud_done:

	if (GetCurMap() == 9 && IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (!m_pWeather)
		{
			m_pWeather = new CSnow(0x400, "~GlowGray.jpg", 0, 0x20, 0x20);
			m_pWeather->SetGravity(0.25f);
		}
	}
	else if (IsCurMapSnow() && weather != 2 && weather != 3)
	{
		if (!m_pWeather)
		{
			int num = 0x1000;
			if (map == 0xe || map == 0x11 || map == 0x12)
				num = 0x666;
			m_pWeather = new ntIceSnow(num, "~GlowGraySmall.jpg", 0);
			m_pWeather->SetGravity(0.08f);
		}
	}
	else
	{
		if (Doc()->m_golfGame.weather == weather && !force)
			return;
		if (m_pWeather)
		{
			delete m_pWeather;
			m_pWeather = NULL;
		}
	}

	switch (weather)
	{
	case 0:
		if (m_pWeather && GetCurMap() == 9)
			m_pWeather->Reset(rand() % 512 + 512, 0.5f);
		EnableShadow(true, true);
		break;
	case 1:
		if (!m_pWeather)
			m_pWeather = new CCloud;
		if (GetCurMap() == 9)
			m_pWeather->Reset(rand() % 512 + 512, 0.5f);
		EnableShadow(false, false);
		break;
	case 2:
		if (!m_pWeather)
		{
			if (IsCurMapSnow() && IS_KINDOF(CGolfTask, AfxGetTask()))
			{
				m_pWeather = new CSnow(0x800, "~GlowGray.jpg", 0, 0x20, 0x20);
				m_pWeather->SetGravity(0.25f);
			}
			else
				m_pWeather = new CRain;
		}
		if (GetCurMap() == 9 || IsCurMapSnow())
			m_pWeather->Reset(0x800, 1.0f);
		EnableShadow(false, false);
		break;
	case 3:
		if (!m_pWeather)
		{
			m_pWeather = new CSnow(0x800, "~GlowGray.jpg", 0, 0x20, 0x20);
			m_pWeather->SetGravity(0.25f);
		}
		else if (GetCurMap() == 9 || IsCurMapSnow())
			m_pWeather->Reset(0x800, 1.0f);
		EnableShadow(false, false);
		break;
	}
	Doc()->m_golfGame.weather = weather;
}

// 983 009af820: so liga a sombra e apaga o tempo e as nuvens (as particulas do lobby por LOBBYBG_MAP do 645 sairam)
void CSceneManager::SetLobbyWeather(bool enable)
{
	EnableShadow(true, true);
	if (m_pWeather)
	{
		delete m_pWeather;
		m_pWeather = NULL;
	}
	if (m_pCloud)
	{
		delete m_pCloud;
		m_pCloud = NULL;
	}
}

void CSceneManager::DrawDebugLine(WView* view)
{
	int i;
	if (m_debugDraw.bits & 0x20 /* 983 009b1bd0: shr 5 */)
	{
		WPolySoup::sModel* model = GOLFDOC()->m_pPolySoup->FindModel(NULL);
		for (i = 0; i < 4; i++)
		{
			WVector p;
			p.x = (i & 1) ? model->aabb.max.x : model->aabb.min.x;
			p.z = (i & 2) ? model->aabb.max.z : model->aabb.min.z;
			vtr[i].y = GetPVS().m_groundHeight;
			vtr[i].x = p.x;
			vtr[i].z = p.z;
			vtr[i].diffuse = 0x808080ff;
		}
		view->DrawPolygonFan(vtx, 0, 0, false);
	}
	if (m_debugDraw.bits & 0x40 /* 983 009b1bd0: shr 6 */)
	{
		WPolySoup::sModel* model = GOLFDOC()->m_pPolySoup->FindModel(NULL);
		for (i = 0; i < 4; i++)
		{
			WVector p;
			p.x = (i & 1) ? model->aabb.max.x : model->aabb.min.x;
			p.z = (i & 2) ? model->aabb.max.z : model->aabb.min.z;
			vtr[i].x = p.x;
			vtr[i].y = GetPVS().m_groundHeight + 192.0f;
			vtr[i].z = p.z;
			vtr[i].diffuse = 0x808080ff;
		}
		view->DrawPolygonFan(vtx, 0, 0, false);
	}

	if (!m_bShowTag)
		return;
	for (std::list<SceneElement>::iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		if (!(*it).puppet)
			continue;
		const Waabb& bound = (*it).puppet->m_bound;
		WVector pos;
		pos.x = (bound.min.x + bound.max.x) * 0.5f;
		pos.y = (bound.max.y + bound.min.y) * 0.5f;
		pos.z = (bound.max.z + bound.min.z) * 0.5f;
		pos.y = (*it).puppet->m_bound.max.y;
		const WMatrix& cam = view->GetCamera();
		float dx = pos.x - cam.xm;
		float dy = pos.y - cam.ym;
		float dz = pos.z - cam.zm;
		if (sqrtf(dx * dx + dy * dy + dz * dz) > 192.0f &&
			view->InFrustum((*it).puppet->m_bound))
			continue;
		WVector screen = view->Projection(pos);
		if (screen.z > 0.0f && screen.z < 1.0f)
		{
			m_pFont->Print(view,
				screen.x - m_pFont->GetTextWidthOld983(view, (*it).name) * 0.5f,
				screen.y, (*it).name, 0, 0xffffffff, NULL);
		}
	}
	m_pFont->Flush(view);
}
