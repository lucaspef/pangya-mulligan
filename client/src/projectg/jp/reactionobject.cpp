// JP 983: faixa 005f01d0–005f35b2 (antes da RefreshInputHandler 005f35c0): os objetos que reagem à bola da Abbot Mine —
// CReactionObject / CAbbotCrystal (sessão B117), CReactionObjectArea e o ator CReactionObjectMng (sessão B118;
// IMPLEMENT_ACTOR em 00cd2170/00cd2190). Classes novas do 983; nomes de método meus. Ver include/jp/reactionobject.h.
#include "minatl.h"
#include "projectg.h"
#include "wresrcmng.h"
#include "wview.h"
#include "fx.h"
#include "fxspray.h"
#include "soundmanager.h"
#include "commonutil.h"
#include "clientsetting.h"
#include "golfball.h"
#include "golfdoc.h"
#include "vc10_layout.h"
#include "shareddoc.h"
#include "polysoup.h"
#include "quadtree.h"
#include "slicestr.h"
#include "lobbyjp.h"
#include "reactionobject.h"

// CSharedDoc+0x11024 = m_pGolfDoc; CGolfDoc+0x08 = m_pPolySoup (deslocamentos do 983); A746: membros (no VS 2022 o Doc anda)
#define POLYSOUP983() (::Doc()->m_pGolfDoc->m_pPolySoup)
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && offsetof(CGolfDoc, m_pPolySoup) == 0x08);

// WPolySoup::sArea do 983 tem 0x80 bytes (passo do vetor no OnInit 005f34a0)
C_ASSERT_VC10(sizeof(WPolySoup::sArea) == 0x80 && offsetof(WPolySoup::sArea, aabb) == 0x44);

static int s_crystalHits983; // 00e78c88: número do som de explosão (1…8)

// 005f0230
CReactionObject::~CReactionObject()
{
	if (g_resrcmng && m_pPuppet)
	{
		g_resrcmng->Release(m_pPuppet, false);
		m_pPuppet = NULL;
	}
}

// 005f0260
void CReactionObject::SetPuppet(WPuppet* pPuppet)
{
	m_pPuppet = pPuppet;
	if (pPuppet)
	{
		m_hitDelay = 0.8f;
		m_bHit = false;
		Reset();
	}
}

// 005f0290
CAbbotCrystal::CAbbotCrystal(WPuppet* pPuppet)
{
	SetPuppet(pPuppet);
}

// quadrante de pos em relação ao vértice (inline nas duas chamadas)
static inline int Quadrant983(const WVector& v, const WVector& pos)
{
	if (v.x > pos.x)
		return v.z > pos.z ? 1 : 4;
	return v.z > pos.z ? 2 : 3;
}

// 005f08e0: ponto dentro do polígono (no plano xz) pela soma dos quadrantes percorridos
bool CReactionObjectArea::IsInPolygon983(const std::vector<WVector>& poly, const WVector& pos)
{
	int num = (int)poly.size();
	if (num < 3)
		return false;

	int winding = 0;
	for (int i = 0; i < num; ++i)
	{
		WVector a = poly[i];
		WVector b = poly[(i + 1) % num];
		int qa = Quadrant983(a, pos);
		int q = Quadrant983(b, pos) - qa;
		switch (q)
		{
		case -2:
		case 2:
			if (b.x - (a.x - b.x) * (b.z - pos.z) / (a.z - b.z) > pos.x)
				q = -q;
			break;
		case 3:
			q = -1;
			break;
		case -3:
			q = 1;
			break;
		}
		winding += q;
	}
	return abs(winding) == 4;
}

// 005f0a60
CReactionObject::CReactionObject(const char* name)
{
	SetPuppet(g_resrcmng->GetPuppet(name, false, true, false));
}

// 005f0a90
CReactionObject::CReactionObject(WPuppet* pPuppet)
{
	SetPuppet(pPuppet);
}

// 005f0ab0
void CReactionObject::Display()
{
	if (!m_pPuppet)
		return;

	m_pPuppet->SetMatrix(m_matrix);
	m_pPuppet->Transform(g_view, 1.0f, 0);
	m_pPuppet->UpdateBSphere();
	if (!g_view->InFrustum(m_pPuppet->GetBoundSphere()))
		return;

	m_pPuppet->UpdateLightSource(&g_lightset, false, NULL);
	WPuppet* pPuppet = m_pPuppet;
	WView* pView = g_view;
	pPuppet->Render(pView, true, 1.0f, pView->InFrustumSafe(pPuppet->GetBoundBox()), false, true);
}

// 005f0b50
void CReactionObject::SetPosition(const WVector& pos, bool)
{
	m_matrix.pivot = pos;
}

// 005f0b70: volta para a identidade e gira ao acaso em torno do eixo 1
void CAbbotCrystal::Reset()
{
	m_matrix = WMatrix::IDENTITY;
	m_matrix.Rotate(Random(-180.0f, 180.0f) * g_DEGTORAD, 1);
}

// 005f0c20: some, solta a faísca e toca o som (uma vez só)
void CAbbotCrystal::OnHit()
{
	if (m_bHit == true)
		return;

	if (s_crystalHits983 < 8)
		++s_crystalHits983;

	m_pPuppet->SetAlpha(0);

	CFxSpray* pSpray = CFx::Instance()->OpenSpray("crystal_bomb01.spr");
	if (pSpray)
		pSpray->m_pos = m_matrix.pivot;

	g_audio->PlaySfx(MakeStr("funobj01_hit_%02d", s_crystalHits983));
	m_bHit = true;
}

// 005f10b0: a esfera do boneco encosta na bola (só uma vez)
bool CReactionObject::CheckHit983()
{
	if (m_bHit == true)
		return false;

	float radius = m_pPuppet->GetBoundSphere().radius;
	float ballRadius = GolfBall().GetRadius();
	WVector d = m_pPuppet->GetBoundSphere().pos - GolfBall().m_pos;
	float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
	return dist - (ballRadius + radius) <= 0.0f;
}

// 005f1350
void CReactionObjectArea::Display()
{
	if (!COption::Instance()->vIsNpcEnabled())
		return;

	for (std::vector<CReactionObject*>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
		(*it)->Display();
}

// 005f1390
void CReactionObjectArea::DebugDisplay()
{
	g_view->DrawAABB(m_box, 0xffff0000, false, 0);
	for (std::vector<CReactionObject*>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
		g_view->DrawSphere((*it)->GetPuppet983()->GetBoundSphere(), 0xff00ff00, 0x14, 0);
}

// centro da caixa, por componente (como o GetCenter do pool.cpp)
static inline WVector Center983(const Waabb& box)
{
	WVector c((box.max.x + box.min.x) * 0.5f, (box.max.y + box.min.y) * 0.5f, (box.max.z + box.min.z) * 0.5f);
	return c;
}

static inline float Length983(const WVector& v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// 005f13e0: a bola dentro da esfera que envolve a área → testa cada objeto
void CReactionObjectArea::CheckHit983()
{
	WVector half = Center983(m_box) - m_box.min;
	WVector d = Center983(m_box) - GolfBall().m_pos;
	float radius = Length983(half);
	float dist = Length983(d);
	if (radius >= dist - GolfBall().GetRadius())
	{
		for (std::vector<CReactionObject*>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
		{
			if ((*it)->CheckHit983())
				(*it)->OnHit();
		}
	}
}

// 005f1790
void CReactionObjectArea::Process983(float)
{
	if (COption::Instance()->vIsNpcEnabled())
		CheckHit983();
}

// 005f19d0
CReactionObjectArea::~CReactionObjectArea()
{
	for (std::vector<CReactionObject*>::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_objects.clear();
}

// 005f1aa0: "funobj_%02d.pet" (o tipo 1 usa o 02)
WPuppet* CReactionObjectArea::LoadPuppet983(int type)
{
	std::string name("");
	if (type != 0)
	{
		if (type == 1)
			name = "funobj_02.pet";
		else
			name = MakeStr("funobj_%02d.pet", type);
	}

	WPuppet* pPuppet = g_resrcmng->GetPuppet(name.c_str(), false, true, false);
	if (pPuppet)
		pPuppet->CalcBound(NULL, true);
	return pPuppet;
}

// 005f1b90: tipo 1 = cristal com um dos dois bonecos ao acaso; 2 e 3 = cristal com o boneco da área
CReactionObject* CReactionObjectArea::CreateObject983(int type, WPuppet* pPuppet)
{
	CReactionObject* pObject = NULL;
	if (type == 1)
	{
		std::string name(rand() % 2 ? "funobj_02.pet" : "funobj_01.pet");
		WPuppet* pRandom = g_resrcmng->GetPuppet(name.c_str(), false, true, false);
		if (pRandom)
			pRandom->CalcBound(NULL, true);
		pObject = new CAbbotCrystal(pRandom);
	}
	else if (type > 1 && type < 4)
	{
		pObject = new CAbbotCrystal(pPuppet);
	}
	return pObject;
}

// 005f1de0: grade de pontos no chão da caixa, célula = caixa do boneco × densidade, com deslocamento ao acaso
// (15 valores em x e 12 em z, repetidos em ciclo)
void CReactionObjectArea::MakeGrid983(WPuppet* pPuppet, float density, std::vector<WVector>* pOut)
{
	Waabb box = pPuppet->GetBoundBox();
	float cellX = (box.max.x - box.min.x) * density;
	float cellZ = (box.max.z - box.min.z) * density;
	float sizeX = m_box.max.x - m_box.min.x;
	float sizeZ = m_box.max.z - m_box.min.z;
	int numX = (int)(sizeX / cellX);
	int numZ = (int)(sizeZ / cellZ);
	float offX = (sizeX - numX * cellX) * 0.5;
	float offZ = (sizeZ - numZ * cellZ) * 0.5;

	std::vector<float> jitterX;
	std::vector<float> jitterZ;
	int i;
	for (i = 0; i < m_jitterX; ++i)
	{
		float half = cellX * 0.5;
		jitterX.push_back(Random(-half, half));
	}
	for (i = 0; i < m_jitterZ; ++i)
	{
		float half = cellZ * 0.5;
		jitterZ.push_back(Random(-half, half));
	}

	std::vector<float>::iterator itX = jitterX.begin();
	std::vector<float>::iterator itZ = jitterZ.begin();
	for (int x = 0; x < numX; ++x)
	{
		for (int z = 0; z < numZ; ++z)
		{
			WVector pos = WVector::UNIT_POS_Y;
			pos.x = m_box.min.x + x * cellX + cellX * 0.5 + offX;
			pos.z = z * cellZ + m_box.min.z + cellZ * 0.5 + offZ;
			pos.x += *itX++;
			pos.z += *itZ;
			if (itX == jitterX.end())
				itX = jitterX.begin();
			if (++itZ == jitterZ.end())
				itZ = jitterZ.begin();

			pos = GetPVS().GetGroundPoint(pos, false, NULL, false);
			pOut->push_back(pos);
		}
	}
}

// 005f23b0
void CReactionObjectArea::MakeGridInPolygon983(const std::vector<WVector>& poly, WPuppet* pPuppet, float density,
	std::vector<WVector>* pOut)
{
	std::vector<WVector> grid;
	MakeGrid983(pPuppet, density, &grid);
	for (std::vector<WVector>::iterator it = grid.begin(); it != grid.end(); ++it)
	{
		if (IsInPolygon983(poly, *it))
			pOut->push_back(*it);
	}
}

// 005f2470: área de caixa (área tipo 8 do mapa)
void CReactionObjectArea::InitBox983(const Waabb& box, int type, float density, bool)
{
	WPuppet* pPuppet;
	if (density > 0.0f && (pPuppet = LoadPuppet983(type)) != NULL)
	{
		m_box = box;

		std::vector<WVector> grid;
		MakeGrid983(pPuppet, density, &grid);
		for (std::vector<WVector>::iterator it = grid.begin(); it != grid.end(); ++it)
		{
			CReactionObject* pObject = CreateObject983(type, pPuppet);
			pObject->SetPosition(*it, false);
			m_objects.push_back(pObject);
		}

		CSceneManagerJp::Instance983()->AddElement983(4, 0, (int)this, "\xbf\xc0\xba\xea\xc1\xa7\xc6\xae \xc3\xa4\xbf\xf2 \xbf\xb5\xbf\xaa", 0, 0); // "오브젝트 채움 영역" (CP949)
	}
}

// 005f2590: área de polígono (nó "fill" do mapa)
void CReactionObjectArea::InitPolygon983(const std::vector<WVector>& poly, int type, float density, bool)
{
	WPuppet* pPuppet;
	if (density > 0.0f && poly.size() > 2 && (pPuppet = LoadPuppet983(type)) != NULL)
	{
		m_box.Clear();
		for (std::vector<WVector>::const_iterator it = poly.begin(); it != poly.end(); ++it)
		{
			WVector pos = *it;
			m_box.AddPoint(pos);
		}

		std::vector<WVector> grid;
		MakeGridInPolygon983(poly, pPuppet, density, &grid);
		for (std::vector<WVector>::iterator it = grid.begin(); it != grid.end(); ++it)
		{
			CReactionObject* pObject = CreateObject983(type, pPuppet);
			pObject->SetPosition(*it, false);
			m_objects.push_back(pObject);
		}

		CSceneManagerJp::Instance983()->AddElement983(4, 0, (int)this, "\xbf\xc0\xba\xea\xc1\xa7\xc6\xae \xc3\xa4\xbf\xf2 \xbf\xb5\xbf\xaa", 0, 0); // "오브젝트 채움 영역" (CP949)
	}
}

// 005f27e0: 0x212 zera o contador do som
void CReactionObjectMng::HandleMsg(const MsgObject& msg)
{
	if (msg.message == 0x212)
		s_crystalHits983 = 0;
}

// 005f2ee0
void CReactionObjectMng::OnProcess(float delta)
{
	for (std::vector<CReactionObjectArea*>::iterator it = m_areas.begin(); it != m_areas.end(); ++it)
		(*it)->Process983(delta);
}

// 005f3010
CReactionObjectMng::~CReactionObjectMng()
{
}

IMPLEMENT_ACTOR(CReactionObjectMng, IActor)

// 005f31f0
void CReactionObjectMng::Clear983()
{
	for (std::vector<CReactionObjectArea*>::iterator it = m_areas.begin(); it != m_areas.end(); ++it)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_areas.clear();
}

// 005f32d0: "*t" = tipo do objeto, "*w" = densidade (tamanho da célula em caixas do boneco)
void CReactionObjectMng::AddBoxArea983(const Waabb& box, const char* name)
{
	float density = 0.0f;
	int type = 0;
	WSliceStr slice(name);
	slice.Get("*t", "%d", &type);
	slice.Get("*w", "%f", &density);
	if (type != 0)
	{
		CReactionObjectArea* pArea = new CReactionObjectArea;
		pArea->InitBox983(box, type, density, false);
		m_areas.push_back(pArea);
	}
}

// 005f33b0
void CReactionObjectMng::AddPolygonArea983(const std::vector<WVector>& poly, const char* name)
{
	if (strstr(name, "fill"))
	{
		float density = 1.0f;
		int type = 0;
		WSliceStr slice(name);
		slice.Get("*t", "%d", &type);
		slice.Get("*w", "%f", &density);
		CReactionObjectArea* pArea = new CReactionObjectArea;
		pArea->InitPolygon983(poly, type, density, false);
		if (pArea)
			m_areas.push_back(pArea);
	}
}

// 005f34a0: áreas do tipo 8 e nós do tipo 0xe do mapa
void CReactionObjectMng::OnInit()
{
	Clear983();

	for (int i = 0; i < POLYSOUP983()->GetAreaNum(); ++i)
	{
		WPolySoup::sArea* pArea = POLYSOUP983()->GetArea(i);
		if (pArea->type == 8)
			AddBoxArea983(pArea->aabb, pArea->name);
	}

	for (std::vector<CNodeContainer>::iterator it = POLYSOUP983()->m_node.begin(); it != POLYSOUP983()->m_node.end(); ++it)
	{
		if (it->GetType() == 0xe)
			AddPolygonArea983(it->GetList(), it->GetName().c_str());
	}
}
