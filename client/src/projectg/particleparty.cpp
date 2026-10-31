#include "minatl.h"

// HACK: in the original operator/ and CGolfDoc::GetDay are PCH inlines that this unit owns; defined
// before particleparty.h they are emitted in the original order (before CParticle::SetCenter).
inline WVector __fastcall operator/(const WVector& vector, float scalar)
{
	float inv = 1.0f / scalar;
	return WVector(inv * vector.x, inv * vector.y, inv * vector.z);
}

inline bool CGolfDoc::GetDay()
{
	if (0)
	{
		(void)(WVector::UNIT_POS_Z / 1.0f);
	}
	return m_bDay;
}

#define REBANG_PARTICLEPARTY_CPP
#include "particleparty.h"
#include "golfball.h"
#include "wview.h"
#include "wresrcmng.h"
#include "wminmax.h"
#include "wutil.h"
#include "commonutil.h"
#include "quadtree.h"
#include "golfdoc.h"
#include "shareddoc.h"
#include "clientsetting.h"
#include "soundmanager.h"
#include "taskmanager.h"
#include "scenemanager.h"
#include "shadow.h"
#include "petbody.h"
#include "projectg.h"
// B1446: 983 le o byte CSharedDoc+0x10594 = m_golfGame.gameType (o cl novo anda o CSharedDoc)
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // B1450: ex-cru Doc()+0x10a99 em OnRender
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


// HACK: 96.0f, 0.99f and 12.566371f are defined in autotarget.cpp, camerawork.cpp and ground_info.cpp
// as function-local statics aliased to their __real@ names; using the literals here would emit
// competing COMDATs in this unit's .rdata.
#pragma comment(linker, \
	"/alternatename:_rb_real_3f7d70a4=?rb_real_3f7d70a4@?1??SetCamera@CCameraWork@@IAEXH_N@Z@4MB")
#pragma comment(linker, \
	"/alternatename:_rb_real_41490fdb=?rb_r_41490fdb@?1??Process@CBase@@QAEXM@Z@4MB")

extern WView* g_view;
extern WResourceManager* g_resrcmng;
extern WMatrix g_camera;

// HACK: CQuadTree::SetBaseVtxColor/SetObjVtxColor and CSceneManager::GetPuppet are not declared
// in their (PCH) headers yet; adding names there moves the PCH canaries, so they are reached
// through local shim classes aliased to the real symbols.
class RbQuadTree
{
public:
	void SetBaseVtxColor(unsigned long color);
	void SetObjVtxColor(unsigned long color);
};
class RbSceneManager
{
public:
	WPuppet* GetPuppet(const char* name);
};
#pragma comment(linker, \
	"/alternatename:?SetBaseVtxColor@RbQuadTree@@QAEXK@Z=?SetBaseVtxColor@CQuadTree@@QAEXK@Z")
#pragma comment(linker, \
	"/alternatename:?SetObjVtxColor@RbQuadTree@@QAEXK@Z=?SetObjVtxColor@CQuadTree@@QAEXK@Z")
#pragma comment(linker, \
	"/alternatename:?GetPuppet@RbSceneManager@@QAEPAVWPuppet@@PBD@Z=?GetPuppet@CSceneManager@@QAEPAVWPuppet@@PBD@Z")

CParticle::CParticle()
	: m_texHandle(0), m_bActive(false), m_pAniSpr(NULL), m_pSpr(NULL)
{
	m_vl[0] = &m_vtx[0];
	m_vl[1] = &m_vtx[1];
	m_vl[2] = &m_vtx[3];
	m_vl[3] = &m_vtx[2];
	m_vl[4] = NULL;
	// HACK: the original emits g_PI and g_HUGE (unused) next to g_EPSILON at the top of .rdata.
	(void)&g_PI;
	(void)&g_HUGE;
}

CParticle::~CParticle()
{
	if (m_pSpr)
	{
		delete m_pSpr;
		m_pSpr = NULL;
	}
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
}

CShotParticle::CShotParticle()
{
	m_numPiece = 100;
	m_pPiece = new sPiece[m_numPiece];
}

CShotParticle::~CShotParticle()
{
	if (m_pPiece)
	{
		delete[] m_pPiece;
		m_pPiece = NULL;
	}
}

void CShotParticle::MakeParticle()
{
	CGolfBall& ball = GolfBall();
	if (ball.m_renderNum >= 3)
	{
		const CGolfBall::sExpected* pExpected = &ball.m_expected[3];
		if (pExpected)
		{
			for (int i = 0; i < m_numPiece; i++)
			{
				m_pPiece[i].active = true;
				m_pPiece[i].pos = ball.m_pos;
				m_pPiece[i].dir = pExpected->vel * 0.1f +
					WVector((float)(rand() % 41) - 20.0f,
						(float)(rand() % 41) - 20.0f,
						(float)(rand() % 41) - 20.0f);
				m_pPiece[i].length = 0.7f;
				m_pPiece[i].life = (float)(rand() % 10) * 0.05f + 0.5f;
				m_pPiece[i].maxLife = m_pPiece[i].life * 0.6f;
			}
			m_count = m_numPiece;
			m_bActive = true;
		}
	}
}

void CShotParticle::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numPiece; i++)
	{
		sPiece* p = &m_pPiece[i];
		if (p->active)
		{
			if (p->life < p->maxLife)
			{
				p->dir *= p->life / p->maxLife;
				p->length *= 0.9f;
			}
			p->pos += p->dir * dt;
			p->life -= dt;
			if (p->life < 0.0f)
			{
				p->active = false;
				m_count--;
			}
		}
	}
	if (m_count == 0)
	{
		m_bActive = false;
	}
}

void CShotParticle::Display()
{
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numPiece; i++)
	{
		if (m_pPiece[i].active)
		{
			WVector dir = m_pPiece[i].dir * -1.0f;
			dir.Normalize();
			dir *= m_pPiece[i].length;
			g_view->DrawLine(m_pPiece[i].pos, 0xffffffff, m_pPiece[i].pos + dir,
				0xffffff, 0);
		}
	}
}

CPowerShot::CPowerShot()
{
	m_numSeg = 16;
	m_radius = 2.0f;
	m_ratio = 0.8f;
	for (int i = 0; i < 4; i++)
	{
		m_vtx[i].tu = (i & 2) ? 1.0f : 0.0f;
		m_vtx[i].tv = (i & 1) ? 1.0f : 0.0f;
	}
}

CPowerShot::~CPowerShot()
{
}

void CPowerShot::MakeParticle()
{
	m_scale = 0.0f;
	m_time = 0.0f;
	m_duration = 0.8f;
	m_speed = 20.0f;
	m_center = GolfBall().m_pos;
	m_center.y += 1.6f;
	m_bActive = true;
}

void CPowerShot::Process(float dt)
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	m_scale += dt * m_speed;
	m_time += dt;
	m_alpha = (int)((1.0f - m_time / m_duration) * 255.0f);
	if (m_time > m_duration)
	{
		m_bActive = false;
	}
}

void CPowerShot::Display()
{
	if (!m_bActive)
	{
		return;
	}
	float step = 6.2831855f / (float)m_numSeg;
	m_vtx[0].diffuse = (m_alpha << 24) | 0xffffff;
	m_vtx[1].diffuse = (m_alpha << 24) | 0xffffff;
	m_vtx[2].diffuse = (m_alpha << 24) | 0xffffff;
	m_vtx[3].diffuse = (m_alpha << 24) | 0xffffff;
	for (int i = 0; i < m_numSeg; i++)
	{
		float angle = (float)i * step;
		for (int j = 0; j < 4; j++)
		{
			float s = sinf(angle + ((j & 2) ? 1.0f : 0.0f) * step);
			float c = cosf(angle + ((j & 2) ? 1.0f : 0.0f) * step);
			float r = (m_radius - ((j & 1) ? 0.0f : 1.0f) * m_ratio) * m_scale;
			m_vtx[j].SetPosition(m_center + WVector(c, 0.0f, s) * r);
		}
		// 983 (006e9178): 0x200, nao o 0x400 do 645
		g_view->DrawPolygonFan(m_vl, (m_texHandle & 0x7ff) | 0xc00000, 0x200,
			false);
	}
}

CLightColumn::CLightColumn()
	: m_texScreen(0)
{
	for (int i = 0; i < 5; i++)
	{
		m_tex[i] = 0;
	}
	m_texLevel983 = 0;
	m_mode983 = 0;
}

CLightColumn::~CLightColumn()
{
	for (int i = 0; i < 5; i++)
	{
		if (g_resrcmng && m_tex[i])
		{
			g_resrcmng->Release(m_tex[i]);
			m_tex[i] = 0;
		}
	}
	if (g_resrcmng && m_texScreen)
	{
		g_resrcmng->Release(m_texScreen);
		m_texScreen = 0;
	}
}

float __fastcall Scale983(int map, unsigned long channelType, float value); // 0046f540 (jp/aiplayer.cpp, frente B)

void CLightColumn::MakeParticle()
{
	// 983 (006eb8f0): modo 0xb so carrega 2 texturas; flag +0x195 do golfdoc troca a 1a pela dourada
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		m_tex[0] = g_resrcmng->LoadTexture("[hole_ef_nomal_10.jpg", 0, 0, NULL);
		m_tex[1] =
			g_resrcmng->LoadTexture("[hole_ef_nomal_gold.jpg", 0, 0, NULL);
	}
	else if (GOLFDOC()->m_b195_983)
	{
		m_tex[0] =
			g_resrcmng->LoadTexture("[hole_ef_nomal_gold.jpg", 0, 0, NULL);
		for (int i = 1; i < 5; i++)
		{
			m_tex[i] = g_resrcmng->LoadTexture(
				MakeStr("[hole_ef_nomal_%d.jpg", i + 10), 0, 0, NULL);
		}
	}
	else
	{
		for (int i = 0; i < 5; i++)
		{
			m_tex[i] = g_resrcmng->LoadTexture(
				MakeStr("[hole_ef_nomal_%d.jpg", i + 10), 0, 0, NULL);
		}
	}
	m_texScreen = g_resrcmng->LoadTexture("hole_scr01.jpg", 0, 0, NULL);
	m_scroll = 0.0f;
	m_fadeState = 0;
	m_height = 1600.0f;
	m_alpha1 = 0xff;
	m_alpha0 = 0xff;
	m_bActive = true;
}

void CLightColumn::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	// 983 (006ee620): raio minimo = 2 * Scale983(0046f540, raio da bola) * 1.6666666f; sem o bloco de conteudo local do 645
	float cup = Scale983(Doc()->m_golfGame.gameType,
		*reinterpret_cast<unsigned long*>(&Doc()->m_curChannel.Type), // B1450: ex-cru Doc()+0x10a99
		GolfBall().m_radius);
	float minRadius = (float)((cup + cup) * 1.6666666269302368);
	WVector pin = GOLFDOC()->GetHoleData().pin;
	WVector diff;
	diff.x = g_camera.xm - pin.x;
	diff.y = 0.0f;
	diff.z = g_camera.zm - pin.z;
	float radius = (float)(sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z) * 0.009999999776482582);
	if (radius > minRadius)
	{
		minRadius = radius;
	}
	m_radius = minRadius;
	m_scroll = m_scroll - (dt + dt);
	bool bDone;
	if (m_fadeState == 1)
	{
		unsigned int a0 = m_alpha0 + 0x14;
		if (a0 > 0xfe)
		{
			a0 = 0xff;
		}
		unsigned int a1 = m_alpha1 + 0x14;
		m_alpha0 = (unsigned char)a0;
		if (a1 > 0xfe)
		{
			a1 = 0xff;
		}
		m_alpha1 = (unsigned char)a1;
		if ((unsigned char)a0 != 0xff)
		{
			return;
		}
		bDone = (unsigned char)a1 == 0x50;
	}
	else
	{
		if (m_fadeState != 2)
		{
			return;
		}
		int a0 = m_alpha0 - 0x14;
		m_alpha0 = a0 < 1 ? 0 : (unsigned char)a0;
		int a1 = m_alpha1 - 0x14;
		m_alpha1 = a1 < 1 ? 0 : (unsigned char)a1;
		if (m_alpha0 != 0 || m_alpha1 != 0)
		{
			return;
		}
		// 983 (006ee80d): com o modo da mensagem 0x1ef a coluna troca de textura (+0x1d0) e volta (fade-in)
		if (m_mode983 != 0)
		{
			if (m_mode983 == 3)
			{
				m_texLevel983 = 0;
			}
			else if (m_mode983 == 4)
			{
				m_texLevel983 = 1;
			}
			m_fadeState = 1;
			return;
		}
		bDone = true;
	}
	if (bDone)
	{
		m_fadeState = 0;
	}
}

void CLightColumn::Display()
{
	if (!m_bActive)
	{
		return;
	}
	CGolfDoc* doc = GOLFDOC();
	if ((doc->m_gameMode & 0x200) != 0) // 983: dword +0x158 (era o byte +0x149 do 645)
	{
		return;
	}
	WVector pin = doc->GetHoleData().pin;
	WVector toCam;
	toCam.x = g_camera.xm - pin.x;
	toCam.y = 0.0f;
	toCam.z = g_camera.zm - pin.z;
	WVector side;
	side.x = WVector::UNIT_POS_Y.z * 0.0f - toCam.z * WVector::UNIT_POS_Y.y;
	side.y = toCam.z * WVector::UNIT_POS_Y.x - toCam.x * WVector::UNIT_POS_Y.z;
	side.z = toCam.x * WVector::UNIT_POS_Y.y - WVector::UNIT_POS_Y.x * 0.0f;
	side.Normalize();
	int level;
	if (Doc()->m_golfGame.gameType == 1)
	{
		sPlayerData* player = doc->GetPlayer(doc->GetCurrentPlayer());
		sTeamData* team = doc->GetTeam(player->team);
		int stroke = team->stroke[doc->m_currentHole - 1];
		level = stroke - doc->GetHoleData().par + 1;
	}
	else if (Doc()->m_golfGame.gameType == 0xb)
	{
		level = m_texLevel983; // 983: this+0x1d0, sem limite
		goto draw;
	}
	else if (doc->m_bNoStat) // 983: byte +0x194 (era o +0x180 do 645)
	{
		level = 4;
		goto draw;
	}
	else
	{
		sPlayerData* player = doc->GetPlayer(doc->GetCurrentPlayer());
		int stroke = player->stroke[doc->m_currentHole - 1];
		level = stroke - doc->GetHoleData().par + 1;
	}
	if (level < 0)
	{
		level = 0;
	}
	else if (level > 3)
	{
		level = 3;
	}
draw:
	WVector fwd;
	fwd.x = toCam.x;
	fwd.y = toCam.y;
	fwd.z = toCam.z;
	fwd = fwd.Normalize();
	fwd.x = fwd.x * 0.5f;
	fwd.y = fwd.y * 0.5f;
	fwd.z = fwd.z * 0.5f;

	WVector base = GOLFDOC()->GetHoleData().pin;
	// 983: a coluna nasce abaixo do pino: 6 x raio da bola (GolfBall().m_radius * 3.0 duas vezes)
	float lift = (float)(GolfBall().m_radius * 3.0 + GolfBall().m_radius * 3.0);
	float org[6];
	org[0] = base.x;
	org[1] = base.y - lift;
	org[2] = base.z;
	org[3] = base.x - fwd.x;
	org[4] = (base.y - fwd.y) - lift;
	org[5] = base.z - fwd.z;

	float side0[8];
	float wide = m_radius * 0.5f;
	float narrow = m_radius * 0.25f;
	side0[0] = 0.0f;
	side0[1] = m_scroll;
	side0[2] = side.x * wide;
	side0[3] = side.y * wide;
	side0[4] = side.z * wide;
	side0[5] = side.x * narrow;
	side0[6] = side.y * narrow;
	side0[7] = side.z * narrow;

	unsigned char alpha[2];
	alpha[0] = m_alpha1;
	alpha[1] = m_alpha0;
	int texFlags[2];
	texFlags[0] = (m_tex[level] & 0x7ff) | 0x400000; // 983: sem o 0x20000000 do 645
	texFlags[1] = (m_texScreen & 0x7ff) | 0xc00000;

	// 983: [0,50] em 10 passos, [50,0.6H] e [0.6H,H]
	int segCount[3] = { 10, 1, 1 };
	float segBase[3];
	float segLen[3];
	segLen[0] = 50.0f;
	segLen[1] = m_height * 0.6f - 50.0f;
	segLen[2] = m_height * 0.4f;
	segBase[0] = 0.0f;
	segBase[1] = 50.0f;
	segBase[2] = m_height * 0.6f;
	float scale[3] = { 2.0f, 400.0f, 0.0f };

	m_vtx[0].tu = 0.0f;
	m_vtx[1].tu = 0.0f;
	m_vtx[2].tu = 1.0f;
	m_vtx[3].tu = 1.0f;

	int layer = 0;
	int off = 0;
	do
	{
		unsigned char a = alpha[layer];
		if (a != 0)
		{
			for (int seg = 0; seg < 3; seg++)
			{
				int count = segCount[seg];
				if (count > 0)
				{
					scale[2] = (float)count;
					for (int n = 0; n < count; n++)
					{
						for (unsigned int k = 0; k < 4; k++)
						{
							int step = (k & 1) + n;
							float h = ((float)step / scale[2]) * segLen[seg] +
								segBase[seg];
							float sign = (k & 2) ? 1.0f : -1.0f;
							float px = org[off / 4] + sign * side0[off / 4 + 2];
							float py = org[off / 4 + 1] + h +
								sign * side0[off / 4 + 3];
							float pz =
								org[off / 4 + 2] + sign * side0[off / 4 + 4];
							m_vtx[k].x = px;
							m_vtx[k].y = py;
							m_vtx[k].z = pz;
							m_vtx[k].tv =
								((m_height - h) / m_height) * scale[layer] +
								side0[layer];
							if (seg < 2)
							{
								m_vtx[k].diffuse =
									(unsigned int)a << 24 | 0xffffff;
							}
							else
							{
								m_vtx[k].diffuse =
									((int)((a * (m_height - h)) /
										(m_height * 0.4f)) & 0xff)
										<< 24 | 0xffffff; // 983: some com a altura
							}
						}
						g_view->DrawPolygonFan(m_vl, texFlags[layer], 0, false);
					}
				}
			}
			if (320.0f < sqrtf(toCam.x * toCam.x + toCam.y * toCam.y + toCam.z * toCam.z))
			{
				return;
			}
		}
		layer++;
		off += 0xc;
	} while (off <= 0x17);
}

CDust::CDust(int num)
	: m_pParticle(NULL)
{
	m_numParticle = num;
	m_pParticle = new sParticle[m_numParticle];
}

CDust::~CDust()
{
	if (m_pParticle)
	{
		delete[] m_pParticle;
		m_pParticle = NULL;
	}
}

void CDust::Process(float dt)
{
	if (m_bActive)
	{
		m_time = m_time - dt / m_interval;
		if (m_time < 0.0f)
		{
			m_time = 1.0f;
			m_bActive = false;
		}
	}
}

void CDust::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numParticle; i++)
	{
		int alpha = (int)(m_time * 128.0f);
		float size =
			(m_pParticle[i].size1 - m_pParticle[i].size0) * (1.0f - m_time) +
			m_pParticle[i].size0;
		m_pSpr->SetPos(m_pParticle[i].pos);
		m_pSpr->SetRect(size, size, size * 0.5f, size * 0.5f);
		m_pSpr->SetColor((alpha << 24) + 0xffffff);
		m_pSpr->Render(g_view, m_pParticle[i].rot * m_time, 0x400000);
	}
}

void CDust::MakeParticle()
{
	if (m_pSpr)
	{
		delete m_pSpr;
		m_pSpr = NULL;
	}
	m_pSpr = new W3dSpr;
	m_pSpr->SetResourceManager(g_resrcmng);
	m_pSpr->AttachTexture(m_texHandle);
	float speed = GolfBall().m_vel.Magnitude() * 0.5f;
	float lo = Min<float>(1.0f, speed); // 983: Min/Max fora de linha (4d1b90/4ad970), uma vez antes do laco
	float hi = Max<float>(1.0f, speed);
	for (int i = 0; i < m_numParticle; i++)
	{
		m_pParticle[i].pos = m_center;
		m_pParticle[i].rot =
			((float)(rand() % 128) / 128.0f - 0.5f) * 1.5707964f;
		m_pParticle[i].size0 = lo;
		m_pParticle[i].size1 = hi;
	}
	m_interval = m_time = 1.0f;
	m_bActive = true;
}

CGrassParticle::CGrassParticle(int num, const WVector& dir)
	: m_numGrass(num)
{
	m_dir = dir;
	m_pGrass = new sGrass[m_numGrass];
}

CGrassParticle::~CGrassParticle()
{
	if (m_pGrass)
	{
		delete[] m_pGrass;
		m_pGrass = NULL;
	}
}

void CGrassParticle::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	bool bActive = false;
	for (int i = 0; i < m_numGrass; i++)
	{
		sGrass* p = &m_pGrass[i];
		if (p->active)
		{
			p->alpha -= dt / p->life * 255.0f;
			p->vel.y -= dt * 34.295296f;
			p->pos += p->vel * dt;
			p->rot += dt * p->rotSpeed;
			if (p->alpha < 0.0f ||
				p->pos.y <
					GetPVS().GetGroundPoint(p->pos, false, NULL, false).y)
			{
				p->active = false;
			}
			else
			{
				bActive = true;
			}
		}
	}
	m_bActive = bActive;
}

void CGrassParticle::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numGrass; i++)
	{
		sGrass* p = &m_pGrass[i];
		if (p->active)
		{
			int color = ((int)p->alpha << 24) | 0xffffff;
			m_pAniSpr->SetPos(p->pos);
			m_pAniSpr->SetColor(color);
			m_pAniSpr->Render(g_view, p->rot, 0x400000, p->frame);
		}
	}
}

void CGrassParticle::MakeParticle()
{
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 64.0f, 64.0f);
	m_pAniSpr->SetRect(0.5f, 0.5f, 0.25f, 0.25f);
	for (int i = 0; i < m_numGrass; i++)
	{
		sGrass* p = &m_pGrass[i];
		p->pos = GolfBall().m_pos;
		p->pos.y -= GolfBall().m_radius;
		p->active = true;
		p->rot = (float)(rand() % 1023) * 0.0009765625 * 2.0 * 3.1415927f;
		p->rotSpeed = ((float)(rand() % 1023) * 0.0009765625f - 0.5f) *
			4.0 * 3.1415927f;
		p->vel.x = (float)(rand() % 1025 - 512) * (m_dir.x * 0.2f + 3.0f) *
				0.001953125f +
			m_dir.x;
		p->vel.y = (float)(rand() % 1025 - 512) * (m_dir.y * 0.2f + 3.0f) *
				0.001953125f +
			m_dir.y;
		p->vel.z = (float)(rand() % 1025 - 512) * (m_dir.z * 0.2f + 3.0f) *
				0.001953125f +
			m_dir.z;
		p->frame = rand() % 4;
		p->life = 1.5f;
		p->alpha = 255.0f;
	}
	m_bActive = true;
}

CBunkerShotParticle::CBunkerShotParticle(int numSmoke, int numParticle)
	: m_pSmoke(NULL),
	  m_pParticle(NULL),
	  m_numSmoke(numSmoke),
	  m_numParticle(numParticle),
	  m_color(0xf8e9ab)
{
	if (numSmoke > 0)
	{
		m_pSmoke = new sSmoke[numSmoke];
	}
	if (numParticle > 0)
	{
		m_pParticle = new sParticle[numParticle];
	}
}

CBunkerShotParticle::~CBunkerShotParticle()
{
	if (m_pSmoke)
	{
		delete[] m_pSmoke;
		m_pSmoke = NULL;
	}
	if (m_pParticle)
	{
		delete[] m_pParticle;
		m_pParticle = NULL;
	}
	m_numSmoke = 0;
	m_numParticle = 0;
}

void CBunkerShotParticle::MakeParticle()
{
	if (m_pSpr)
	{
		delete m_pSpr;
		m_pSpr = NULL;
	}
	m_pSpr = new W3dSpr;
	m_pSpr->SetResourceManager(g_resrcmng);
	m_pSpr->AttachTexture(m_texHandle);
	Generate();
	m_interval = m_time = 1.0f;
	g_audio->PlaySfx("\xba\xa1\xc4\xbf\xb8\xf0\xb7\xa1\xb3\xaf\xb8\xb2", NULL,
		0, NULL, NULL, 0.5f, 200.0f);
	m_bActive = true;
}

void CBunkerShotParticle::Generate()
{
	CGolfBall& ball = GolfBall();
	if (ball.m_renderNum >= 3)
	{
		const CGolfBall::sExpected* pExpected = &ball.m_expected[3];
		if (pExpected)
		{
			int i;
			for (i = 0; i < m_numSmoke; i++)
			{
				m_pSmoke[i].pos = ball.m_pos;
				// 983 006ec0c0: rand x, y, z nesta ordem (argumentos do WVector seriam z primeiro)
				int rx = rand() % 11;
				int ry = rand() % 11;
				int rz = rand() % 11;
				m_pSmoke[i].vel = pExpected->vel * 0.5f +
					WVector((float)rx - 5.0f, (float)ry - 5.0f, (float)rz - 5.0f) *
						2.5f;
			}
			for (i = 0; i < m_numParticle; i++)
			{
				m_pParticle[i].active = true;
				m_pParticle[i].pos = ball.m_pos;
				int rx = rand() % 41;
				int ry = rand() % 41;
				int rz = rand() % 41;
				m_pParticle[i].vel = pExpected->vel * 0.6f +
					WVector((float)rx - 20.0f, (float)ry - 20.0f, (float)rz - 20.0f);
				m_pParticle[i].length = 0.3f;
				// 983 006ec332: r*0.5/10+1.0
				m_pParticle[i].life = (float)(rand() % 10) * 0.5f / 10.0f + 1.0f;
				m_pParticle[i].maxLife = m_pParticle[i].life * 0.5f;
			}
		}
	}
}

void CBunkerShotParticle::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	m_time -= dt / m_interval;
	if (m_time < 0.0f)
	{
		m_bActive = false;
		return;
	}
	for (int i = 0; i < m_numSmoke; i++)
	{
		sSmoke* p = &m_pSmoke[i];
		p->vel.y -= dt * 120.033536f;
		p->pos += m_pSmoke[i].vel * dt;
	}
	for (int j = 0; j < m_numParticle; j++)
	{
		sParticle* p = &m_pParticle[j];
		if (p->active)
		{
			p->vel.x =
				p->vel.x * 0.99f;
			p->vel.y -= dt * 102.885888f;
			p->vel.z =
				p->vel.z * 0.99f;
			p->pos += p->vel * dt;
			p->life -= dt;
			if (p->life < 0.0f)
			{
				p->active = false;
				m_numParticle--;
			}
		}
	}
}

void CBunkerShotParticle::Display()
{
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numSmoke; i++)
	{
		sSmoke* p = &m_pSmoke[i];
		float size = (1.0f - m_time) * 40.0f;
		m_pSpr->SetRect(size, size, size * 0.5f, 0.0f);
		m_pSpr->SetColor(((int)(m_time * 255.0f) << 24) | m_color);
		m_pSpr->SetPos(p->pos);
		m_pSpr->Render(g_view, 0x400000, W3dSpr::CAMERA_X_ALIGN);
	}
	for (int j = 0; j < m_numParticle; j++)
	{
		sParticle* p = &m_pParticle[j];
		if (p->active)
		{
			WVector dir = p->vel * -1.0f;
			dir.Normalize();
			dir *= p->length;
			if (p->length > 0.1f)
			{
				p->length *= 0.95f;
			}
			if (p->life < p->maxLife)
			{
				dir *= p->life / p->maxLife;
			}
			g_view->DrawLine(p->pos, 0xffa08000, p->pos + dir, 0xa08000, 0);
		}
	}
}

CBunkerParticle::CBunkerParticle()
{
}

CBunkerParticle::~CBunkerParticle()
{
}

void CBunkerParticle::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	m_time -= dt / m_interval;
	if (m_time < 0.0f)
	{
		m_bActive = false;
	}
}

void CBunkerParticle::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	m_size = (1.0f - m_time) * 20.0f;
	m_pSpr->SetRect(m_size, m_size, m_size * 0.5f, 0.0f);
	m_alpha = (int)(m_time * 255.0f);
	m_pSpr->SetColor((m_alpha << 24) | 0xffffff);
	m_pSpr->Render(g_view, 0x400000, W3dSpr::CAMERA_X_ALIGN);
}

void CBunkerParticle::MakeParticle()
{
	if (m_pSpr)
	{
		delete m_pSpr;
		m_pSpr = NULL;
	}
	m_pSpr = new W3dSpr;
	m_pSpr->SetResourceManager(g_resrcmng);
	m_pSpr->AttachTexture(m_texHandle);
	m_size = 0.0f;
	m_time = 1.0f;
	m_interval = 0.7f;
	m_pSpr->SetPos(GolfBall().m_pos);
	g_audio->PlaySfx("\xba\xa1\xc4\xbf\xb8\xf0\xb7\xa1\xb3\xaf\xb8\xb2");
	m_bActive = true;
}

CFragment::CFragment()
	: m_pFrag(NULL), m_numFrag(30)
{
	m_pFrag = new sFrag[m_numFrag];
}

CFragment::~CFragment()
{
	if (m_pFrag)
	{
		delete[] m_pFrag;
		m_pFrag = NULL;
	}
}

void CFragment::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	m_alpha -= dt * 96.0f;
	for (int i = 0; i < m_numFrag; i++)
	{
		m_pFrag[i].vel.y -= dt * 17.147648f;
		m_pFrag[i].pos += m_pFrag[i].vel * dt;
	}
	if (m_alpha < 0.0f)
	{
		m_bActive = false;
	}
}

void CFragment::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	m_pAniSpr->SetColor(((int)m_alpha << 24) | 0xffffff);
	for (int i = 0; i < m_numFrag; i++)
	{
		m_pAniSpr->SetPos(m_pFrag[i].pos);
		m_pAniSpr->Render(g_view, 0.0f, 0xc00000, 7);
	}
}

void CFragment::MakeParticle()
{
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 32.0f, 32.0f);

	WVector xa(1.0f, 0.0f, 0.0f);
	WVector ya(0.0f, 1.0f, 0.0f);
	WVector za(0.0f, 0.0f, 1.0f);
	if (!WisEqual(m_normal.y, 1.0f, g_EPSILON))
	{
		ya = m_normal;
		xa = WCrossProduct(ya, WVector::UNIT_POS_Y);
		xa.Normalize();
		za = WCrossProduct(xa, ya);
		za.Normalize();
	}
	for (int i = 0; i < m_numFrag; i++)
	{
		float t = (float)(rand() % 1023) * 0.0009765625f;
		m_pFrag[i].pos = m_center;
		// 983 006ec550: o angulo (2o rand) sai antes do comprimento (3o rand)
		float ang = (float)(rand() % 1023) * 0.0009765625f;
		WMatrix rot = RotMat((ang + ang) * 3.1415927f, 1);
		float len = (float)(rand() % 1023) * 0.0009765625f;
		WVector dir(WVector::UNIT_POS_Z.x * len, WVector::UNIT_POS_Z.y * len,
			WVector::UNIT_POS_Z.z * len);
		m_pFrag[i].vel = dir * rot;
		m_pFrag[i].vel.y = t * 5.0f + (1.0f - t) * 2.0f;
		// 983 006ec92f: abaixo de 2.0 vale 2.0 (Max)
		float speed = Max(GolfBall().m_vel.Magnitude() * 0.1f, 2.0f);
		float vx = m_pFrag[i].vel.x * speed;
		float vy = m_pFrag[i].vel.y * speed;
		float vz = m_pFrag[i].vel.z * speed;
		// 983 006ec96c: vel = vx*xa + vy*ya + vz*za (o nosso trocava x e z)
		m_pFrag[i].vel.x = xa.x * vx + ya.x * vy + za.x * vz;
		m_pFrag[i].vel.y = xa.y * vx + ya.y * vy + za.y * vz;
		m_pFrag[i].vel.z = xa.z * vx + ya.z * vy + za.z * vz;
	}
	m_alpha = 255.0f;
	m_bActive = true;
	float size = Max(Min(GolfBall().m_vel.Magnitude() * 0.01f, 0.5f), 0.2f);
	m_pAniSpr->SetRect(size, size, size * 0.5f, size * 0.5f);
}

CDroplet::CDroplet()
{
	m_numDroplet = 100;
	m_pDroplet = new sDroplet[m_numDroplet];
	m_pSpr = g_resrcmng->Get3DSpr("w-drop.dds", 0);
	m_pSpr->SetRect(0.2f, 0.2f, 0.1f, 0.1f);
}

CDroplet::~CDroplet()
{
	if (m_pSpr)
	{
		g_resrcmng->Release(m_pSpr);
		m_pSpr = NULL;
	}
	if (m_pDroplet)
	{
		delete[] m_pDroplet;
		m_pDroplet = NULL;
	}
}

void CDroplet::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	m_alpha -= dt / m_interval * 255.0f;
	if (m_alpha < 0.0f)
	{
		m_bActive = false;
		return;
	}
	if (m_interval < 0.0f)
	{
		m_bActive = false;
		return;
	}
	for (int i = 0; i < m_numDroplet; i++)
	{
		m_pDroplet[i].vel.y -= dt * 34.295296f;
		m_pDroplet[i].pos += m_pDroplet[i].vel * dt;
	}
}

void CDroplet::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	int color = ((int)m_alpha << 24) | 0xffffff;
	for (int i = 0; i < m_numDroplet; i++)
	{
		m_pSpr->SetPos(m_pDroplet[i].pos);
		m_pSpr->SetColor(color);
		m_pSpr->Render(g_view, 0.0f, 0x400000);
	}
}

void CDroplet::MakeParticle()
{
	for (int i = 0; i < m_numDroplet; i++)
	{
		m_pDroplet[i].pos = GolfBall().m_pos;
		// 983 006ecbb0: o angulo (1o rand) sai antes da velocidade (2o rand)
		float ang = (float)(rand() % 1023) * 0.0009765625f;
		WMatrix rot = RotMat((ang + ang) * 3.1415927f, 1);
		float speed = (float)(rand() % 1023) * 6.0f * 0.0009765625f;
		WVector dir(WVector::UNIT_POS_Z.x * speed, WVector::UNIT_POS_Z.y * speed,
			WVector::UNIT_POS_Z.z * speed);
		m_pDroplet[i].vel = dir * rot;
		float t = (float)(rand() % 1023) * 0.0009765625f;
		m_pDroplet[i].vel.y = (1.0f - t) * 10.0f + t * 40.0f;
	}
	m_alpha = 255.0f;
	m_interval = 1.5f;
	m_bActive = true;
}

CRipple::CRipple()
	: m_pRipple(NULL), m_numRipple(0)
{
	m_pRipple = new sRipple[5];
	AddRipple(15.0f, 0.01f, 255.0f, 255.0f);
	AddRipple(17.0f, 0.2f, 255.0f, 255.0f);
	AddRipple(19.0f, 0.5f, 128.0f, 128.0f);
	m_pDroplet = new CDroplet;
	Init();
}

CRipple::~CRipple()
{
	if (m_pRipple)
	{
		delete[] m_pRipple;
		m_pRipple = NULL;
	}
	if (m_pDroplet)
	{
		delete m_pDroplet;
		m_pDroplet = NULL;
	}
}

void CRipple::Init()
{
	for (int i = 0; i < m_numRipple; i++)
	{
		m_pRipple[i].active = false;
		m_pRipple[i].size = 0.0f;
		m_pRipple[i].alpha = m_pRipple[i].startAlpha;
		m_pRipple[i].wait = 0.0f;
	}
	m_count = m_numRipple;
}

void CRipple::MakeParticle()
{
	Init();
	for (int i = 0; i < m_numRipple; i++)
	{
		m_pRipple[i].wait = m_pRipple[i].delay;
	}
	m_bActive = true;
	m_pDroplet->SetCenter(m_center);
	m_pDroplet->MakeParticle();
}

void CRipple::AddRipple(float sizeSpeed, float delay, float startAlpha,
	float alphaSpeed)
{
	m_pRipple[m_numRipple].sizeSpeed = sizeSpeed;
	m_pRipple[m_numRipple].delay = delay;
	m_pRipple[m_numRipple].startAlpha = startAlpha;
	m_pRipple[m_numRipple].alphaSpeed = alphaSpeed;
	m_numRipple++;
}

void CRipple::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numRipple; i++)
	{
		if (m_pRipple[i].wait > 0.0f)
		{
			m_pRipple[i].wait -= dt;
			if (m_pRipple[i].wait < 0.0f)
			{
				m_pRipple[i].active = true;
			}
		}
		if (m_pRipple[i].active)
		{
			m_pRipple[i].size += dt * m_pRipple[i].sizeSpeed;
			m_pRipple[i].alpha -= dt * m_pRipple[i].alphaSpeed;
			if (m_pRipple[i].alpha < 0.0f)
			{
				m_pRipple[i].active = false;
				m_count--;
				if (m_count == 0)
				{
					m_bActive = false;
				}
			}
		}
	}
	m_pDroplet->Process(dt);
}

void CRipple::Display()
{
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numRipple; i++)
	{
		if (m_pRipple[i].active)
		{
			for (int j = 0; j < 4; j++)
			{
				WVector pos = m_center;
				pos.x += ((j & 2) ? 1.0f : -1.0f) * m_pRipple[i].size * 0.5f;
				pos.z += ((j & 1) ? 1.0f : -1.0f) * m_pRipple[i].size * 0.5f;
				m_vtx[j].SetPosition(pos);
				m_vtx[j].tu = (j & 2) ? 1.0f : 0.0f;
				m_vtx[j].tv = (j & 1) ? 0.0f : 1.0f;
				m_vtx[j].diffuse = ((int)m_pRipple[i].alpha << 24) | 0xffffff;
			}
			// 983 (006e9f7d): 0x200, nao o 0x400 do 645
			g_view->DrawPolygonFan(m_vl, (m_texHandle & 0x7ff) | 0x20800000,
				0x200, false);
		}
	}
	m_pDroplet->Display();
}

CCoin::CCoin(int num, const WVector& pos)
{
	m_pos = pos;
	m_numCoin = num;
	m_pCoin = new sCoin[num];
	m_numSpark = 50;
	m_pSpark = new sSpark[50];
}

CCoin::~CCoin()
{
	for (int i = 0; i < m_numCoin; i++)
	{
		if (m_pCoin[i].pBody)
		{
			delete m_pCoin[i].pBody;
			m_pCoin[i].pBody = NULL;
		}
	}
	if (m_pCoin)
	{
		delete[] m_pCoin;
		m_pCoin = NULL;
	}
	if (m_pSpark)
	{
		delete[] m_pSpark;
		m_pSpark = NULL;
	}
	m_numSpark = m_numCoin = 0;
}

void CCoin::MakeParticle()
{
	m_time = 1.0f;
	m_interval = 3.0f;
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 32.0f, 32.0f);
	m_pAniSpr->SetRect(0.3f, 0.3f, 0.15f, 0.15f);
	m_numDropSound = 0;
	Generate();
	SetActive(true);
}

void CCoin::Generate()
{
	if (m_numCoin > 0)
	{
		sCoin* p = &m_pCoin[0];
		if (p->pBody == NULL)
		{
			if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
			{
				p->pBody = new CPetBody("*pet coin2.pet *shadow 0", false);
			}
			else
			{
				p->pBody = new CPetBody("*pet coin.pet *shadow 0", false);
			}
			p->pBody->GetPet()->CalcBound(NULL, true);
		}
		p->pBody->SetPos(m_pos);
		p->rot = (float)(rand() % 32) * 6.2831855f / 32.0f;
		float y = p->bounce = (float)(rand() % 5 - 2) * 0.1f + 1.5f;
		p->vel = WVector(0.3f, y, 0.0f) *
			RotMat(6.2831855f / (float)(rand() % 32 + 1), 1);
	}
	if (m_numCoin > 1)
	{
		for (int i = 1; i < m_numCoin; i++)
		{
			sCoin* p = &m_pCoin[i];
			if (p->pBody == NULL)
			{
				if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/))
				{
					p->pBody = new CPetBody("*pet coin2.pet *shadow 0", false);
				}
				else
				{
					p->pBody = new CPetBody("*pet coin.pet *shadow 0", false);
				}
				p->pBody->GetPet()->CalcBound(NULL, true);
			}
			p->pBody->SetPos(m_pos);
			p->rot = (float)(rand() % 32) * 6.2831855f / 32.0f;
			float y = p->bounce = (float)(rand() % 5 - 2) * 0.1f + 1.3f;
			float x = (float)(rand() % 51 - 25) * 0.005f + 0.25f;
			p->vel = WVector(x, y, 0.0f) *
				RotMat((float)i * 6.2831855f / (float)(m_numCoin - 1), 1);
		}
	}
	for (int j = 0; j < m_numSpark; j++)
	{
		sSpark* p = &m_pSpark[j];
		p->pos = m_pos;
		// 983 006ea120: sorteia x (rand%5) antes de y (rand%21); vida = r*0.5/10+0.5
		float x = (float)(rand() % 5 - 2) * 0.02f + 1.3f;
		float y = (float)(rand() % 21 - 10) * 0.05f + 2.0f;
		p->vel = WVector(x, y, 0.0f) *
			RotMat((float)j * 6.2831855f / (float)(m_numSpark - 1), 1);
		p->alpha = 255.0f;
		p->life = (float)(rand() % 10) * 0.5f / 10.0f + 0.5f;
		p->maxLife = p->life * 0.8f;
		p->active = true;
	}
}

bool CCoin::ShouldDisplay()
{
	return m_bActive;
}

void CCoin::SetActive(bool bActive)
{
	m_bActive = bActive;
}

void CCoin::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	m_time -= dt / m_interval;
	if (m_time < 0.0f)
	{
		m_time = 1.0f;
		SetActive(false);
	}
	for (int i = 0; i < m_numCoin; i++)
	{
		sCoin* p = &m_pCoin[i];
		int a = (int)(m_time * 1275.0f);
		unsigned char alpha = a < 255 ? (unsigned char)a : 255;
		p->pBody->SetAlpha(alpha);
		p->pBody->SetLight();
		p->pBody->SetPos(p->pBody->GetPos() + p->vel * 15.0f * dt);
		p->pBody->SetRotate(WVector(0.0f, p->rot, 0.0f));
		p->rot += dt * 15.0f;
		if (p->vel.x > dt * 0.1f)
		{
			p->vel.x -= dt * 0.1f;
		}
		else if (p->vel.x < -(dt * 0.1f))
		{
			p->vel.x += dt * 0.1f;
		}
		if (p->vel.z > dt * 0.1f)
		{
			p->vel.z -= dt * 0.1f;
		}
		else if (p->vel.z < -(dt * 0.1f))
		{
			p->vel.z += dt * 0.1f;
		}
		p->vel.y -= dt * 3.0f;
		WVector pos = p->pBody->GetPos();
		float ground = GetPVS().GetGroundPoint(pos, true, NULL, true).y;
		if (pos.y < ground)
		{
			if (m_numDropSound < 4)
			{
				g_audio->PlaySfx("pang_coin_drop");
				m_numDropSound++;
			}
			p->bounce *= 0.5f;
			pos.y = ground;
			p->vel.y = p->bounce;
			p->pBody->SetPos(pos);
		}
		p->pBody->Process(dt);
	}
	for (int j = 0; j < m_numSpark; j++)
	{
		sSpark* p = &m_pSpark[j];
		if (p->active)
		{
			if (p->life < p->maxLife)
			{
				p->vel *= p->life / p->maxLife * 0.8f;
				p->alpha *= p->life / p->maxLife;
			}
			p->pos += p->vel * 7.0f * dt;
			p->life -= dt;
			if (p->life < 0.0f)
			{
				p->active = false;
			}
		}
	}
}

void CCoin::Display()
{
	// HACK: emission order; the original compiles this function one wave later.
	if (0)
	{
		((CShotParticle*)0)->CShotParticle::Process(0.0f);
	}
	if (!m_bActive)
	{
		return;
	}
	for (int i = 0; i < m_numCoin; i++)
	{
		m_pCoin[i].pBody->Display();
	}
	for (int j = 0; j < m_numSpark; j++)
	{
		sSpark* p = &m_pSpark[j];
		m_pAniSpr->SetColor(((int)p->alpha << 24) | 0xffffff);
		m_pAniSpr->SetPos(p->pos);
		m_pAniSpr->Render(g_view, 0.0f, 0xc00000, 5);
	}
}

void CCoin::DisplayShadow()
{
	if (!m_bActive)
	{
		return;
	}
	char name[32];
	sprintf(name, "coin%x", this);
	CShadowManager::Instance()->RegisterCaster_New(name, 16, "sun", g_lightset);
	for (int i = 0; i < m_numCoin; i++)
	{
		m_pCoin[i].pBody->DisplayShadow(name);
	}
}

CEnergyCenter::CEnergyCenter(int num, const WVector& pos, float rotSpeed)
{
	m_rotSpeed = rotSpeed;
	m_numSpark = num;
	m_pos = pos;
	m_coreAlpha = 1.0f;
	m_coreRot = 0.0f;
	m_bgAlpha = 255.0f;
	m_lastBgAlpha = 255.0f;
	m_pGolfBg = AfxGetTask()->GetActor("GolfBg");
	m_pSpark = new sSpark[m_numSpark];
}

CEnergyCenter::~CEnergyCenter()
{
	if (m_pSpark)
	{
		delete[] m_pSpark;
		m_pSpark = NULL;
	}
	m_numSpark = 0;
}

void CEnergyCenter::MakeParticle()
{
	m_numGenerated = 0;
	m_bActive = true;
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 32.0f, 32.0f);
}

void CEnergyCenter::Generate(float dt)
{
	for (int i = 0; m_numGenerated < m_numSpark && i < dt * 35.0f; i++)
	{
		sSpark* p = &m_pSpark[m_numGenerated];
		p->pos = WVector((float)(rand() % 31 - 15) * 0.05f + 8.0f,
					 (float)(rand() % 31 - 15) * 0.05f + 8.0f, 0.0f) *
				RotMat((float)m_numSpark * 6.2831855f /
						(float)(rand() % m_numSpark + 1),
					1) +
			m_pos;
		p->dir = m_pos - p->pos;
		p->dir = p->dir.Normalize();
		p->alpha = 0.0f;
		p->active = true;
		m_numGenerated++;
	}
}

void CEnergyCenter::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	Generate(dt);
	bool bDone = true;
	for (int i = 0; i < m_numGenerated; i++)
	{
		sSpark* p = &m_pSpark[i];
		if (p->active)
		{
			p->alpha += dt * 1000.0;
			if (p->alpha > 255.0)
			{
				p->alpha = 255.0f;
			}
			p->pos += p->dir * 20.0f * dt;
			WVector rel = p->pos - m_pos;
			p->pos = rel * RotMat(m_rotSpeed, 1) + m_pos;
			if (p->pos.y * p->dir.y >= m_pos.y * p->dir.y)
			{
				m_coreAlpha += 255.0f / (float)m_numSpark;
				p->active = false;
			}
			else
			{
				bDone = false;
			}
			p->dir = m_pos - p->pos;
			p->dir = p->dir.Normalize();
		}
	}
	m_coreRot -= dt * 1.5707964f;
	if (bDone)
	{
		m_coreAlpha -= dt * 400.0f;
		if (m_coreAlpha < 0.0f)
		{
			m_bActive = false;
		}
	}
	else
	{
		m_bgAlpha -= dt * 250.0;
		if (m_bgAlpha < 160.0)
		{
			m_bgAlpha = 160.0f;
		}
	}
}

void CEnergyCenter::Display()
{
	if (!m_bActive)
	{
		return;
	}
	if (GOLFDOC()->GetDay() && COption::Instance()->vGetParticleLevel() < 1)
	{
		if (!WisEqual(m_bgAlpha, m_lastBgAlpha, g_EPSILON))
		{
			m_lastBgAlpha = m_bgAlpha;
			unsigned char alpha = (unsigned char)(int)m_bgAlpha;
			unsigned long color =
				(((0xffffff00 | alpha) << 8 | alpha) << 8) | alpha;
			((RbQuadTree&)GetPVS()).SetBaseVtxColor(color);
			((RbQuadTree&)GetPVS()).SetObjVtxColor(color);
			if (m_pGolfBg)
			{
				m_pGolfBg << MsgObject(NULL, M983(0x13f), color, 0, 0, 0, 0);
			}
		}
	}
	m_pAniSpr->SetColor(((int)m_coreAlpha << 24) | 0xffffff);
	m_pAniSpr->SetPos(m_pos);
	m_pAniSpr->SetRect(1.0f, 1.0f, 0.5f, 0.5f);
	m_pAniSpr->Render(g_view, m_coreRot, 0xf00000, 4);
	for (int i = 0; i < m_numGenerated; i++)
	{
		sSpark* p = &m_pSpark[i];
		if (p->active)
		{
			m_pAniSpr->SetColor(((int)p->alpha << 24) | 0xffffff);
			m_pAniSpr->SetPos(p->pos);
			m_pAniSpr->SetRect(0.5f, 0.5f, 0.25f, 0.25f);
			m_pAniSpr->Render(g_view, 0.0f, 0xc00000, 6);
		}
	}
}

CStarDust::CStarDust()
{
	m_genInterval = 0.01f;
	m_numParticle = 400;
	m_pParticle = new sParticle[m_numParticle];
}

CStarDust::~CStarDust()
{
	if (m_pParticle)
	{
		delete[] m_pParticle;
		m_pParticle = NULL;
	}
	m_numParticle = 0;
}

void CStarDust::MakeParticle()
{
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 32.0f, 32.0f);
	m_next = 0;
	m_alpha = 255.0f;
	m_genTime = 0.0f;
	m_rot = 0.0f;
	m_bActive = true;
	for (int i = 0; i < m_numParticle; i++)
	{
		m_pParticle[i].active = false;
	}
}

void CStarDust::Generate()
{
	WVector side = WCrossProduct(GolfBall().m_vel, g_camera.za);
	float radius = GolfBall().m_radius;
	side = side.Normalize() * 2.0f * radius;

	sParticle* p = &m_pParticle[m_next];
	p->active = true;
	p->radius = GolfBall().m_radius;
	p->angleSpeed = (float)(rand() % 1023) * 0.0015339808f + 1.5707964f;
	p->axisX = WVector((float)(rand() % 1024 - 512) * 0.001953125f,
				   (float)(rand() % 1024 - 512) * 0.001953125f,
				   (float)(rand() % 1024 - 512) * 0.001953125f) *
			2.0f +
		g_camera.za;
	p->axisY = side / side.Magnitude() +
		WVector((float)(rand() % 1024 - 512) * 0.001953125f,
			(float)(rand() % 1024 - 512) * 0.001953125f,
			(float)(rand() % 1024 - 512) * 0.001953125f) *
			2.0f;
	p->center = GolfBall().m_pos;
	p->angle = (float)(rand() % 1023) * 0.0061359233f;
	p->life = 1.0f;
}

void CStarDust::BlowAway(float dt, int index)
{
	sParticle* p = &m_pParticle[index];
	if (p->active)
	{
		float speed = dt * 2.0f;
		p->angleSpeed -= speed;
		p->angle -= p->angleSpeed * dt;
		p->pos = (p->axisX * cosf(p->angle) + p->axisY * sinf(p->angle)) *
				p->radius +
			p->center;
		p->radius += speed;
		p->life -= dt * 0.3f;
		if (p->life < 0.0f)
		{
			p->active = false;
		}
	}
}

void CStarDust::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	if (GolfBall().m_vel.y < -1.0f)
	{
		m_genTime += Min(dt, dt * m_alpha * 0.005f);
		m_alpha = Max(0.0f, m_alpha - dt * 127.0f);
		WPuppet* pBall =
			((RbSceneManager*)CSceneManager::Instance())->GetPuppet("\xb0\xf8");
		if (pBall && pBall->GetRootBone())
		{
			pBall->SetAlpha(255 - (unsigned char)(int)m_alpha);
		}
		if (Wabs(m_alpha) < g_EPSILON)
		{
			m_bActive = false;
			m_alpha = 0.0f;
			if (pBall && pBall->GetRootBone())
			{
				pBall->SetAlpha(255);
			}
		}
	}
	else
	{
		m_genTime += dt;
	}
	for (int i = 0; i < m_numParticle; i++)
	{
		BlowAway(dt, i);
	}
	if (m_genTime > m_genInterval)
	{
		int num;
		if (dt > m_genInterval)
		{
			num = (int)(m_genTime / m_genInterval);
		}
		else
		{
			num = 1;
		}
		m_genTime -= num * m_genInterval;
		while (num > 0)
		{
			for (int k = 2; k != 0; k--)
			{
				Generate();
				if (dt > m_genInterval)
				{
					BlowAway(num * m_genInterval + m_genTime, m_next);
				}
				m_next = (m_next + 1) % m_numParticle;
			}
			num--;
		}
	}
	m_rot -= dt * 18.84955644607544;
}

void CStarDust::Display()
{
	if (!m_bActive)
	{
		return;
	}
	int color = ((int)m_alpha << 24) | 0xffffff;
	m_pAniSpr->SetRect(1.0f, 1.0f, 0.5f, 0.5f);
	m_pAniSpr->SetColor(color);
	m_pAniSpr->SetPos(GolfBall().m_pos);
	m_pAniSpr->Render(g_view, m_rot, 0xc00000, 4);
	for (int i = 0; i < m_numParticle; i++)
	{
		sParticle* p = &m_pParticle[i];
		if (p->active)
		{
			float size = p->life * 0.6f;
			m_pAniSpr->SetPos(p->pos);
			m_pAniSpr->SetRect(size, size, size * 0.5f, size * 0.5f);
			m_pAniSpr->SetColor(color);
			m_pAniSpr->Render(g_view, 0.0f, 0xc00000, 5);
		}
	}
}

CStarDust2::CStarDust2()
{
	m_genInterval = 0.1f;
	m_numParticle = 60;
	m_pParticle = new sParticle[m_numParticle];
}

CStarDust2::~CStarDust2()
{
	if (m_pParticle)
	{
		delete[] m_pParticle;
		m_pParticle = NULL;
	}
	m_numParticle = 0;
}

void CStarDust2::MakeParticle()
{
	if (m_pAniSpr)
	{
		delete m_pAniSpr;
		m_pAniSpr = NULL;
	}
	m_pAniSpr = new W3dAniSpr;
	m_pAniSpr->SetResourceManager(g_resrcmng);
	m_pAniSpr->LoadTexture(m_texHandle, 32.0f, 32.0f);
	m_next = 0;
	m_alpha = 255.0f;
	m_genTime = 0.0f;
	m_rot = 0.0f;
	m_bActive = true;
	for (int i = 0; i < m_numParticle; i++)
	{
		m_pParticle[i].active = false;
	}
}

void CStarDust2::Generate()
{
	WVector side = WCrossProduct(GolfBall().m_vel, g_camera.za);
	float radius = GolfBall().m_radius;
	side = side.Normalize() * 2.0f * radius;

	for (int i = 0; i < 4; i++)
	{
		sParticle* p = &m_pParticle[m_next];
		m_next = (m_next + 1) % m_numParticle;
		p->active = true;
		p->radius = GolfBall().m_radius * 4.0f;
		p->angleSpeed = (float)(rand() % 1023) * 0.0015339808f + 1.5707964f;
		p->axisX = WVector((float)(rand() % 1024 - 512) * 0.001953125f,
					   (float)(rand() % 1024 - 512) * 0.001953125f,
					   (float)(rand() % 1024 - 512) * 0.001953125f) *
				2.0f +
			g_camera.za;
		p->axisY = side / side.Magnitude() +
			WVector((float)(rand() % 1024 - 512) * 0.001953125f,
				(float)(rand() % 1024 - 512) * 0.001953125f,
				(float)(rand() % 1024 - 512) * 0.001953125f) *
				2.0f;
		p->center = GolfBall().m_pos;
		p->angle = (float)(rand() % 1023) * 0.0061359233f;
		p->alpha = 255.0f;
	}
}

void CStarDust2::Process(float dt)
{
	if (!m_bActive)
	{
		return;
	}
	if (GolfBall().m_vel.y < 0.0f)
	{
		m_genTime += Min(dt, dt * m_alpha * 0.005f);
		m_alpha = Max(0.0f, m_alpha - dt * 150.0f);
		((RbSceneManager*)CSceneManager::Instance())
			->GetPuppet("\xb0\xf8")
			->SetAlpha(255 - (int)m_alpha);
		if (Wabs(m_alpha) < g_EPSILON)
		{
			m_bActive = false;
			m_alpha = 0.0f;
			((RbSceneManager*)CSceneManager::Instance())
				->GetPuppet("\xb0\xf8")
				->SetAlpha(255);
		}
	}
	else
	{
		m_genTime += dt;
	}
	if (m_genTime > m_genInterval)
	{
		m_genTime -= m_genInterval;
		Generate();
	}
	m_rot -= dt * 18.84955644607544;
	for (int i = 0; i < m_numParticle; i++)
	{
		sParticle* p = &m_pParticle[i];
		if (p->active)
		{
			float speed = dt * 2.0f;
			p->angleSpeed -= speed;
			p->angle -= p->angleSpeed * dt;
			p->pos = (p->axisX * cosf(p->angle) + p->axisY * sinf(p->angle)) *
					p->radius +
				p->center;
			p->radius += speed;
			p->alpha = Max(p->alpha - dt * 100.0f, 0.0f);
			if (GolfBall().m_radius * 0.5f > p->radius)
			{
				p->active = false;
			}
		}
	}
}

void CStarDust2::Display()
{
	if (!m_bActive)
	{
		return;
	}
	int color = ((int)m_alpha << 24) | 0xffffff;
	m_pAniSpr->SetColor(color);
	m_pAniSpr->SetPos(GolfBall().m_pos);
	m_pAniSpr->SetRect(2.0f, 2.0f, 1.0f, 1.0f);
	m_pAniSpr->Render(g_view, m_rot, 0xf00000, 5);
	for (int i = 0; i < m_numParticle; i++)
	{
		sParticle* p = &m_pParticle[i];
		if (p->active)
		{
			m_pAniSpr->SetPos(p->pos);
			m_pAniSpr->SetRect(0.5f, 0.5f, 0.25f, 0.25f);
			m_pAniSpr->SetColor((((int)p->alpha << 24) | 0xffffff) & color);
			m_pAniSpr->Render(g_view, 0.0f, 0xf00000, 6);
		}
	}
}
