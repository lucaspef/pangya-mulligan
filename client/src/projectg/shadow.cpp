#include "minatl.h"
// HACK: instantiates std::list<sShadowRecver*> before CShadowManager (class-instantiation order)
#define REBANG_SHADOW_CPP
#include "shadow.h"
#include <set>
#include <string>
#include <math.h>
#include "wresrcmng.h"
#include "wshadowview.h"
#include "wvideo.h"
#include "wview.h"
#include "wpuppet.h"
#include "cfile.h"
#include "wminmax.h"

namespace
{
	// HACK: the original owns storage for these wmath.h constants (.rdata, declaration order);
	// an inline that references them makes the compiler emit it
	inline float RbShadowMathConsts()
	{
		return g_DEGTORAD + g_HUGE + g_EPSILON;
	}
}

extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;

WMatrix __cdecl MakeCameraMatrix(const WVector& eye, const WVector& at);

namespace rb_shadow
{
	class DisableShadowPet
	{
	public:
		static DisableShadowPet* __fastcall Instance()
		{
			static DisableShadowPet singleton;
			return &singleton;
		}

		bool Exist(const char* name) const
		{
			if (!name)
				return false;
			std::string str(name);
			std::string::size_type pos = str.rfind('.');
			if (pos != std::string::npos)
				str.erase(pos, str.length() - pos);
			return m_names.end() != m_names.find(str);
		}

	private:
		DisableShadowPet();

		std::set<std::string> m_names;

	public:
		~DisableShadowPet();
		static void __cdecl
		RbDisableshadowpetFunclets(); // HACK: EH funclets of the naked DisableShadowPet (nakedify)
	};

	DisableShadowPet::DisableShadowPet()
	{
		cFile* pFile = g_resrcmng->GetCFile("club_noshadow.txt", 0xffff);
		if (!pFile)
			return;
		char* buf = new char[pFile->m_nLen];
		memset(buf, 0, pFile->m_nLen);
		pFile->Read(buf, pFile->m_nLen);
		std::string line;
		for (int i = 0; i < pFile->m_nLen; ++i)
		{
			if (buf[i] == '\n' || buf[i] == '\r')
			{
				if (!line.empty())
				{
					m_names.insert(line);
					line.clear();
				}
			}
			else
			{
				line += buf[i];
			}
		}
		if (!line.empty())
			m_names.insert(line);
		delete[] buf;
		CloseCFile(pFile);
	}

	// HACK: the original dtor is implicit; this explicit one (same code) is emitted after the naked ctor
	// impl like the original's. The dead call keeps it after the impl.
	DisableShadowPet::~DisableShadowPet()
	{
	}
}
using namespace rb_shadow;

CShadow::CShadow(WResourceManager* pResMgr, eShadowMode mode)
	: m_pShadowView(NULL), m_pRecvView(NULL), m_type(SHADOW_PROJECTION)
{
	switch (mode)
	{
	case SHADOW_MODE_SW:
		m_pShadowView = new WShadowViewSw(0x60);
		m_pRecvView = new WRecvViewSw;
		break;
	case SHADOW_MODE_HW:
		m_pShadowView = new WShadowViewHw(0x60);
		m_pRecvView = new WRecvViewHw;
		break;
	default:
		m_pShadowView = new WShadowViewHw(0x60);
		m_pRecvView = new WRecvViewHw;
		break;
	}
	m_pShadowView->SetResourceManager(pResMgr);
	// 983 009681d0 (D129): sem consulta de caps; 0x80 no modo SW, 0x200 nos demais
	if (mode == SHADOW_MODE_SW)
		m_pShadowView->SetTextureSize(0x80, 0x80);
	else
		m_pShadowView->SetTextureSize(0x200, 0x200);
	m_pRecvView->SetResourceManager(pResMgr);
}

CShadow::~CShadow()
{
	if (m_pShadowView)
	{
		delete m_pShadowView;
		m_pShadowView = NULL;
	}
	if (m_pRecvView)
	{
		delete m_pRecvView;
		m_pRecvView = NULL;
	}
}

void CShadow::SetShadowType(ShadowType type)
{
	m_type = type;
}

void CShadow::RenderCaster(WView* view, const LightSet& light,
	const WSphere& sphere, std::list<std::list<WPuppet*>*>& pets,
	sShadowRecver* recver, bool bRenderPets)
{
	if (m_type != SHADOW_NONE)
	{
		switch (m_type)
		{
		case SHADOW_PROJECTION:
			RenderProjectionShadow(view, light, sphere, pets, recver,
				bRenderPets);
			break;
		}
		view->SetCamera(view->camera);
		view->SetClip(view->GetClipNearValue(), view->GetClipFarValue(), false);
		view->UpdateCamera();
	}
}

void CShadow::RenderProjectionShadow(WView* view, const LightSet& light,
	const WSphere& sphere, std::list<std::list<WPuppet*>*>& pets,
	sShadowRecver* recver, bool bRenderPets)
{
	// 983 0096c470 (D129): pets = lista de grupos (CShadowCaster::m_groups983); Render com 4o arg true
	if (Setup(view, light, sphere, recver) && bRenderPets && m_pShadowView)
	{
		for (std::list<std::list<WPuppet*>*>::iterator g = pets.begin();
			g != pets.end(); g++)
		{
			std::list<WPuppet*>* grp = *g;
			if (!grp)
				continue;
			for (std::list<WPuppet*>::iterator it = grp->begin();
				it != grp->end(); it++)
			{
				WPuppet* pPet = *it;
				if (pPet)
				{
					if (!DisableShadowPet::Instance()->Exist(
							pPet->GetPuppetName()))
						m_pShadowView->Render(pPet, true, 1.0f, true, false);
				}
			}
		}
		m_pShadowView->Render();
	}
}

// 983 00969030 (D129): tan(fov*0.5) arredondado para float; 8 cantos do tronco pela camera; caixa min/max
bool CShadow::CheckFrustum(WView* view, const WMatrix& camera, float fov,
	float nearDist, float farDist)
{
	float t = (float)tan((double)(float)(fov * 0.5));
	float hn = nearDist * t;
	float hf = t * farDist;
	WVector c[8];
	c[0] = WVector(-hn, -hn, nearDist) * camera;
	c[1] = WVector(hn, -hn, nearDist) * camera;
	c[2] = WVector(-hn, hn, nearDist) * camera;
	c[3] = WVector(hn, hn, nearDist) * camera;
	c[4] = WVector(-hf, -hf, farDist) * camera;
	c[5] = WVector(hf, -hf, farDist) * camera;
	c[6] = WVector(-hf, hf, farDist) * camera;
	c[7] = WVector(hf, hf, farDist) * camera;
	Waabb box(c[0], c[0]);
	for (int i = 1; i < 8; ++i)
		box.AddPoint(c[i]);
	return view->InFrustum(box);
}

bool CShadow::Setup(WView* view, const LightSet& light, const WSphere& sphere,
	sShadowRecver* recver)
{
	// 983 00969460 (D129): perto fixo 1e6, longe = plano do chao do recver ou far da view + dist
	WMatrix camera;
	float dist;
	if (light.type == 2)
	{
		dist = (float)(sphere.radius + 1000000.0);
		WVector pos = sphere.pos - light.nearOne * dist;
		camera = MakeCameraMatrix(pos, sphere.pos);
	}
	else
	{
		camera = MakeCameraMatrix(light.nearOne, sphere.pos);
		dist = WVector::UNIT_POS_Z * (sphere.pos * ~camera);
	}
	float down = WVector::UNIT_NEG_Y * camera.za;
	float farDist;
	if (down > 0.001f)
		farDist = (camera.pivot.y - recver->height) / down;
	else
		farDist = view->GetClipFarValue() + dist;
	float a = (float)asin((double)(sphere.radius / dist));
	float fov = a + a;
	const float nearDist = 1000000.0f;
	if (!(1000000.0 <= farDist))
		return false;
	if (!CheckFrustum(view, camera, fov, nearDist, farDist))
		return false;
	m_pRecvView->SetRenderView(view);
	float fov2 = fov * 640.0f / 480.0f;
	m_pRecvView->SetFOV(fov2);
	m_pRecvView->SetCamera(camera);
	m_pRecvView->SetClip(nearDist, farDist, false);
	m_pRecvView->BeginScene();
	m_pRecvView->Clear(0, 6);
	m_pRecvView->CollectRecvTris(recver->pet, true, 1.0f, false, false);
	m_pRecvView->EndScene();
	if (!m_pRecvView->HasRecver())
		return false;
	m_pShadowView->SetFOV(fov2);
	m_pShadowView->SetCamera(camera);
	m_pShadowView->SetClip(nearDist, farDist, false);
	m_pShadowView->BeginScene();
	m_pShadowView->Clear(0, 6);
	return true;
}

void CShadow::RenderRecver(sShadowRecver* recver)
{
	m_pRecvView->Render(m_pShadowView, recver->pet);
}

CShadowCaster::CShadowCaster(const char* name, int state,
	WResourceManager* pResMgr, eShadowMode mode)
{
	strcpy(m_name, name);
	m_state = state | 2;
	m_pShadow = new CShadow(pResMgr, mode);
}

CShadowCaster::~CShadowCaster()
{
	if (m_pShadow)
	{
		delete m_pShadow;
		m_pShadow = NULL;
	}
}

const char* CShadowCaster::GetName() const
{
	return m_name;
}

const char* CShadowCaster::GetLightName() const
{
	return m_lightName;
}

void CShadowCaster::Reset()
{
	// 983 (0096aa20): alem de limpar os bits, refaz m_groups983 com {&m_pets}
	m_state &= ~0xc;
	m_groups983.clear();
	m_groups983.push_back(&m_pets);
}

int CShadowCaster::GetState() const
{
	return m_state;
}

void CShadowCaster::SetState_New(int state)
{
	m_state = state;
}

void CShadowCaster::ClearCaster()
{
	m_pets.clear();
	m_state |= 2;
}

void CShadowCaster::ClearCaster_New()
{
	m_pets.clear();
	m_state = 2;
}

void CShadowCaster::SetLight(const char* lightName, LightSet& light)
{
	if (lightName)
		strcpy(m_lightName, lightName);
	memcpy(&m_light, &light, sizeof(LightSet));
	m_state |= 1;
	// HACK: dead use of an inline from the PCH (wscene.h): the original emits
	// SetLight in the group of functions that touch PCH inlines
	if (0)
		LightSet();
}

void CShadowCaster::AddPet(WPuppet* pPet)
{
	// obsolete (AddPet_New replaced it); the dead push_back keeps the
	// original emission order
	if (0)
		m_pets.push_back(pPet);
}

void CShadowCaster::AddPet_New(WPuppet* pPet)
{
	m_pets.push_back(pPet);
	m_state &= ~2;
}

void CShadowCaster::DeletePet(WPuppet* pPet)
{
	if (0)
		((CShadowCaster*)0)->CShadowCaster::~CShadowCaster();
	for (std::list<WPuppet*>::iterator it = m_pets.begin(); it != m_pets.end();
		it++)
	{
		if (*it == pPet &&
			_stricmp((*it)->GetPuppetName(), pPet->GetPuppetName()) == 0)
		{
			m_pets.erase(it);
			if (m_pets.size() == 0)
				m_state |= 2;
			return;
		}
	}
}

void CShadowCaster::ReplacePets(WPuppet* pOld, WPuppet* pNew, bool bShow)
{
	if (pOld == NULL)
	{
		if (m_pets.size() == 0)
		{
			if (pNew != NULL)
			{
				m_pets.push_back(pNew);
				m_state &= ~2;
			}
		}
		else if (m_pets.size() == 1)
		{
			std::list<WPuppet*>::iterator first = m_pets.begin();
			if (pNew != NULL)
			{
				*first = pNew;
			}
			else
			{
				m_pets.erase(first);
				m_state |= 2;
			}
		}
	}
	else
	{
		std::list<WPuppet*>::iterator it;
		for (it = m_pets.begin(); it != m_pets.end(); it++)
		{
			if (*it == pOld &&
				_stricmp((*it)->GetPuppetName(), pOld->GetPuppetName()) == 0)
			{
				if (pNew != NULL)
				{
					*it = pNew;
				}
				else
				{
					m_pets.erase(it);
					if (m_pets.size() == 0)
						m_state |= 2;
				}
				break;
			}
		}
	}
	m_state &= ~0x80;
	ShowCaster(bShow);
}

// 983 00969ed0 (D129): sem argumentos. Esfera = bsphere do 1o pet; com 2 ou mais pets, junta as esferas dos
// demais no plano da luz (MakeCameraMatrix(luz, centro) e a inversa), z = min, raio final * 1.1.
void CShadowCaster::UpdateBSphere()
{
	if ((m_state & 0x22) != 0)
		return;
	std::list<WPuppet*>::iterator it = m_pets.begin();
	const WSphere& s0 = (*it)->GetBoundSphere();
	m_bsphere.pos = s0.pos;
	m_bsphere.radius = s0.radius;
	int type = m_light.type;
	if (type == 2)
	{
		// 983 00969ed0: distancia ate o pivo da camera (g_camera+0x24 = 0xf024c4), nao ate a origem;
		// com ZERO o boneco do lobby (longe da origem) ficava com 0x100 e sem sombra (C185.1).
		// A313: religado (A272 desligou pelo lobby azulado; a causa era o macro 0x40000 do receptor,
		// pos-efeito, trocado por 0x200000 = 983 00bc62cc em wshadowview.cpp, B517)
		float dx = m_bsphere.pos.x - g_camera.pivot.x;
		float dz = m_bsphere.pos.z - g_camera.pivot.z;
		float dist = (float)sqrt(dz * dz + dx * dx + 0.0f);
		if (dist <= 300.0f)
			m_state &= ~0x100;
		else
			m_state |= 0x100;
	}
	if (m_pets.size() == 1)
		return;
	WVector eye;
	const WVector* pEye = &m_light.nearOne;
	if (type == 2)
	{
		float k = 1000.0f;
		eye.x = m_bsphere.pos.x - m_light.nearOne.x * k;
		eye.y = m_bsphere.pos.y - m_light.nearOne.y * k;
		eye.z = m_bsphere.pos.z - m_light.nearOne.z * k;
		pEye = &eye;
	}
	WMatrix cam = MakeCameraMatrix(*pEye, m_bsphere.pos);
	WMatrix inv = ~cam;
	WVector p = m_bsphere.pos;
	m_bsphere.pos.x = p.z * inv.p[6] + inv.p[3] * p.y + p.x * inv.p[0] + inv.p[9];
	m_bsphere.pos.y = p.z * inv.p[7] + p.x * inv.p[1] + inv.p[4] * p.y + inv.p[10];
	m_bsphere.pos.z = p.z * inv.p[8] + p.x * inv.p[2] + inv.p[5] * p.y + inv.p[11];
	for (++it; it != m_pets.end(); ++it)
	{
		WPuppet* pet = *it;
		if (pet == NULL)
			continue;
		const WSphere& ps = pet->GetBoundSphere();
		float cx = ps.pos.y * inv.p[3] + ps.pos.x * inv.p[0] + ps.pos.z * inv.p[6] + inv.p[9];
		float cy = inv.p[7] * ps.pos.z + inv.p[1] * ps.pos.x + inv.p[4] * ps.pos.y + inv.p[10];
		float cz = ps.pos.z * inv.p[8] + inv.p[2] * ps.pos.x + inv.p[5] * ps.pos.y + inv.p[11];
		float r = ps.radius;
		float dx = cx - m_bsphere.pos.x;
		float dy = cy - m_bsphere.pos.y;
		float d = (float)sqrt(dx * dx + dy * dy);
		float R = m_bsphere.radius;
		float newR;
		if (R <= r)
		{
			float t = d + R;
			if (t <= r)
			{
				m_bsphere.radius = r;
				m_bsphere.pos.x = cx;
				m_bsphere.pos.y = cy;
				m_bsphere.pos.z = cz;
				continue;
			}
			newR = (t + r) * 0.5f;
		}
		else if (R < d + r)
			newR = (d + R + r) * 0.5f;
		else
			continue;
		float shift = newR - R;
		m_bsphere.pos.x = (dx / d) * shift + m_bsphere.pos.x;
		m_bsphere.pos.y = m_bsphere.pos.y + shift * (dy / d);
		if (!(m_bsphere.pos.z <= cz))
			m_bsphere.pos.z = cz;
		m_bsphere.radius = newR;
	}
	p = m_bsphere.pos;
	m_bsphere.pos.x = p.z * cam.p[6] + p.x * cam.p[0] + cam.p[3] * p.y + cam.p[9];
	m_bsphere.pos.y = p.z * cam.p[7] + p.x * cam.p[1] + cam.p[4] * p.y + cam.p[10];
	m_bsphere.pos.z = p.z * cam.p[8] + p.x * cam.p[2] + cam.p[5] * p.y + cam.p[11];
	m_bsphere.radius = m_bsphere.radius * 1.1f;
}

// 983 0096ab30 (D129): funde a esfera de `other` na minha (plano da luz, 2D) se nao estiver contida e
// anexa o grupo de pets de `other` (m_groups983); other fica marcado com 8 (nao renderiza sozinho)
void CShadowCaster::MergeCaster983(CShadowCaster* other)
{
	if ((m_state & 0xe) != 0 || (other->m_state & 0xe) != 0)
		return;
	WSphere os = other->m_bsphere;
	WVector eye;
	const WVector* pEye = &m_light.nearOne;
	if (m_light.type == 2)
	{
		float k = 1000000.0f;
		eye.x = m_bsphere.pos.x - m_light.nearOne.x * k;
		eye.y = m_bsphere.pos.y - m_light.nearOne.y * k;
		eye.z = m_bsphere.pos.z - m_light.nearOne.z * k;
		pEye = &eye;
	}
	WMatrix cam = MakeCameraMatrix(*pEye, m_bsphere.pos);
	WMatrix inv = ~cam;
	WVector p = m_bsphere.pos;
	m_bsphere.pos.x = inv.p[9] + inv.p[6] * p.z + inv.p[3] * p.y + inv.p[0] * p.x;
	m_bsphere.pos.y = inv.p[7] * p.z + p.x * inv.p[1] + inv.p[4] * p.y + inv.p[10];
	m_bsphere.pos.z = inv.p[8] * p.z + p.x * inv.p[2] + inv.p[5] * p.y + inv.p[11];
	float cx = os.pos.y * inv.p[3] + os.pos.x * inv.p[0] + os.pos.z * inv.p[6] + inv.p[9];
	float cy = inv.p[10] + inv.p[7] * os.pos.z + inv.p[1] * os.pos.x + os.pos.y * inv.p[4];
	float cz = os.pos.z * inv.p[8] + os.pos.x * inv.p[2] + inv.p[5] * os.pos.y + inv.p[11];
	float dx = cx - m_bsphere.pos.x;
	float dy = cy - m_bsphere.pos.y;
	float d = (float)sqrt(dx * dx + dy * dy);
	float R = m_bsphere.radius;
	float r = os.radius;
	if (!(r + R <= d))
	{
		float t;
		if (R <= r)
		{
			t = R + d;
			if (t <= r)
			{
				m_bsphere.radius = r;
				m_bsphere.pos.x = cx;
				m_bsphere.pos.y = cy;
				m_bsphere.pos.z = cz;
				goto attach;
			}
		}
		else if (R <= r + d)
			t = R + d;
		else
			goto attach;
		{
			float newR = (t + r) * 0.5f;
			float shift = newR - R;
			m_bsphere.pos.x = (dx / d) * shift + m_bsphere.pos.x;
			m_bsphere.pos.y = shift * (dy / d) + m_bsphere.pos.y;
			m_bsphere.pos.z = Min(m_bsphere.pos.z, cz);
			m_bsphere.radius = newR;
		}
	attach:
		m_groups983.push_back(&other->m_pets);
		other->m_state |= 8;
	}
	p = m_bsphere.pos;
	m_bsphere.pos.x = cam.p[3] * p.y + p.x * cam.p[0] + p.z * cam.p[6] + cam.p[9];
	m_bsphere.pos.y = cam.p[7] * p.z + p.x * cam.p[1] + cam.p[4] * p.y + cam.p[10];
	m_bsphere.pos.z = p.z * cam.p[8] + p.x * cam.p[2] + cam.p[5] * p.y + cam.p[11];
}

extern "C" void __cdecl jp983_rastro(const char* fmt, ...);
// B1420 temporario (item 19 do dono: faixa de sombra gigante que gira com o radar): esfera do caster a cada 30 quadros
static void B1420_Rastro(const char* name, int state, const WSphere& s, std::list<WPuppet*>& pets,
	size_t groups, sShadowRecver* recver)
{
	static unsigned int n = 0;
	if (n >= 9000 || (n++ % 30) != 0)
		return;
	WPuppet* p = pets.empty() ? NULL : pets.front();
	jp983_rastro("B1420 caster %s st=%x r=%.1f pos=%.1f,%.1f,%.1f pets=%d grp=%d pet0=%s recv_h=%.1f", name, state,
		s.radius, s.pos.x, s.pos.y, s.pos.z, (int)pets.size(), (int)groups,
		p ? p->GetPuppetName() : "-", recver ? recver->height : 0.0f);
}

void CShadowCaster::RenderCaster(WView* view, sShadowRecver* recver)
{
	B1420_Rastro(m_name, m_state, m_bsphere, m_pets, m_groups983.size(), recver); // B1420 temporario
	int state = m_state;
	if ((state & 0x40) == 0 && m_bsphere.radius < 8.0f)
		m_bsphere.radius = 8.0f;
	if ((state & 0x20) && (state & 0x80))
	{
		m_pShadow->RenderCaster(view, m_light, m_bsphere, m_groups983, recver,
			false);
	}
	else
	{
		m_state = state | 0x80;
		m_pShadow->RenderCaster(view, m_light, m_bsphere, m_groups983, recver, true);
	}
}

void CShadowCaster::RenderRecver(sShadowRecver* recver)
{
	m_pShadow->RenderRecver(recver);
}

void CShadowCaster::ShowCaster(bool bShow)
{
	if (bShow)
		m_state &= ~0x20;
	else
		m_state |= 0x20;
}

const char* g_theCasterName = "theCaster";

CShadowManager::CShadowManager(WResourceManager* pResMgr, bool bEnable)
	: m_bEnable(bEnable), m_bActive(bEnable), m_pResMgr(pResMgr)
{
	int caps;
	pResMgr->VideoReference()->Command(WX_VDEV_GET_CAPS, 1, (int)&caps);
	m_minFov = 1.5707964f;
	m_maxDist = 200.0f;
	m_bFlag = true;
	m_mode = (eShadowMode)(caps != 0);
}

CShadowManager::~CShadowManager()
{
	ClearCasters();
	ClearRecvers();
}

void CShadowManager::ClearCasters()
{
	std::list<CShadowCaster*>::iterator it;
	for (it = m_casters.begin(); it != m_casters.end(); it++)
	{
		CShadowCaster* pCaster = *it;
		if (pCaster)
			delete pCaster;
	}
	m_casters.clear();
}

void CShadowManager::ClearRecvers()
{
	std::list<sShadowRecver*>::iterator it;
	for (it = m_recvers.begin(); it != m_recvers.end(); it++)
	{
		delete *it;
		*it = NULL;
	}
	m_recvers.clear();
}

void CShadowManager::ClearCasters_New()
{
	std::list<CShadowCaster*>::iterator it;
	for (it = m_casters.begin(); it != m_casters.end(); it++)
	{
		(*it)->Reset();
		(*it)->ClearCaster_New();
	}
}

void CShadowManager::ClearRecvers_New()
{
	std::list<sShadowRecver*>::iterator it;
	for (it = m_recvers.begin(); it != m_recvers.end(); it++)
		(*it)->ClearRecver();
}

bool CShadowManager::IsEnabled() const
{
	return m_bEnable;
}

void CShadowManager::Enable(bool bEnable)
{
	m_bEnable = bEnable;
}

CShadowCaster* CShadowManager::RegisterCaster_New(const char* name, int state,
	const char* lightName, LightSet& light)
{
	// 983 0096b510 (D129): usa o nome e o estado recebidos (o 645 fixava "theCaster"/0)
	CShadowCaster* pCaster = FindCaster(name);
	if (pCaster == NULL)
	{
		pCaster = new CShadowCaster(name, state, m_pResMgr, m_mode);
		pCaster->SetLight(lightName, light);
		m_casters.push_back(pCaster);
	}
	else
	{
		pCaster->SetLight(lightName, light);
		pCaster->SetState_New(pCaster->GetState() | state);
	}
	return pCaster;
}

CShadowCaster* CShadowManager::FindCaster(const char* name)
{
	std::list<CShadowCaster*>::iterator it;
	for (it = m_casters.begin(); it != m_casters.end(); it++)
	{
		if (strcmp((*it)->GetName(), name) == 0)
			return *it;
	}
	return NULL;
}

void CShadowManager::AddCastPet_New(const char* name, bool bClear,
	WPuppet* pPet)
{
	CShadowCaster* pCaster = FindCaster(name);
	if (pCaster)
	{
		if (bClear)
			pCaster->ClearCaster();
		pCaster->AddPet_New(pPet);
	}
}

sShadowRecver* CShadowManager::RegisterRecver_New(const char* name)
{
	sShadowRecver* pRecver = FindCommonRecver(name);
	if (pRecver == NULL)
	{
		pRecver = new sShadowRecver;
		strcpy(pRecver->name, name);
		m_recvers.push_back(pRecver);
	}
	return pRecver;
}

void CShadowManager::AddRecvPet_New(const char* name, WPuppet* pPet,
	float height)
{
	sShadowRecver* pRecver = FindCommonRecver(name);
	if (pRecver)
	{
		pRecver->pet = pPet;
		pRecver->height = height;
	}
}

sShadowRecver* CShadowManager::FindCommonRecver(const char* name)
{
	std::list<sShadowRecver*>::iterator it;
	for (it = m_recvers.begin(); it != m_recvers.end(); it++)
	{
		if (strcmp((*it)->name, name) == 0)
			return *it;
	}
	return NULL;
}

void CShadowManager::Flush(WView* view)
{
	if (m_bEnable && m_bActive)
	{
		std::list<CShadowCaster*>::iterator it;
		for (it = m_casters.begin(); it != m_casters.end(); it++)
			(*it)->Reset();
		UpdateBVolume();
		MergeCasters983();
		for (it = m_casters.begin(); it != m_casters.end(); it++)
		{
			if (((*it)->GetState() & 0x2e) == 0 &&
				((*it)->GetState() & 0x100) == 0)
			{
				std::list<sShadowRecver*>::iterator rit;
				for (rit = m_recvers.begin(); rit != m_recvers.end(); rit++)
				{
					(*it)->RenderCaster(view, *rit);
					(*it)->RenderRecver(*rit);
				}
			}
		}
		view->UpdateCamera();
	}
}

void CShadowManager::SetActive(bool bActive)
{
	m_bActive = bActive;
}

void CShadowManager::UpdateBVolume()
{
	// 983 0096a4d0: o limite (m_maxDist/WisEqual) sumiu; so manda cada caster recalcular a esfera
	std::list<CShadowCaster*>::iterator it;
	for (it = m_casters.begin(); it != m_casters.end(); it++)
		(*it)->UpdateBSphere();
}

// 983 0096b030: para cada par de casters ativos e longe, funde o 2o no 1o (CShadowCaster::MergeCaster983)
void CShadowManager::MergeCasters983()
{
	if (m_casters.size() > 1)
	{
		std::list<CShadowCaster*>::iterator a, b;
		for (a = m_casters.begin(); a != m_casters.end(); ++a)
		{
			CShadowCaster* ca = *a;
			if ((ca->GetState() & 0x2e) != 0 || (ca->GetState() & 0x100) != 0)
				continue;
			for (b = m_casters.begin(); b != m_casters.end(); ++b)
			{
				CShadowCaster* cb = *b;
				if (b != a && (cb->GetState() & 0x3e) == 0 &&
					(cb->GetState() & 0x100) == 0)
					ca->MergeCaster983(cb);
			}
		}
	}
}

void CShadowManager::UpdateLight(const char* lightName, LightSet& light)
{
	std::list<CShadowCaster*>::iterator it;
	for (it = m_casters.begin(); it != m_casters.end(); it++)
	{
		if (strcmp((*it)->GetLightName(), lightName) == 0)
			(*it)->SetLight(NULL, light);
	}
}

void CShadowManager::Reset()
{
	ClearCasters();
	ClearRecvers();
}

void CShadowManager::Reset_New()
{
	ClearCasters_New();
	ClearRecvers_New();
}

