#include "minatl.h"
#define REBANG_QUADTREE_CPP
#include "wind.h"
#include "quadtree.h"
#include "projectg.h"
#include "wpuppet.h"
#include "wbone.h"
#include "wmesh.h"
#include "wminmax.h"
#include "club.h"
#include "shareddoc.h"
#include "clientsetting.h"
#include "slicestr.h"
#include "soundmanager.h"
#include "lobbymain.h"
#include "nodecontainer.h"
#include "contentsdoc.h"
#include "fx.h"
#include "voiceitem.h"
#include "lobbytask.h"
#include "rtti.h"
#include <map>
extern "C" void __cdecl jp983_rastro(const char* fmt, ...);

#define PuppetFaceNum(pPuppet) ((pPuppet)->GetFaceNum()) // 983: WPuppet +0xf8 (645: +0xf0)
#define BoneMesh(pBone) ((pBone)->GetMesh()) // 983: WBone::m_mesh +0x4c (645: +4)
#define BoneFlags(pBone) \
	(*reinterpret_cast<unsigned long*>(reinterpret_cast<char*>((pBone)) + 0x10c)) // 983: WBone::m_flag +0x10c (645: +0xf0; 00953e22)
#define QuadObjNum(quad) ((quad).objNum)
#define QuadObjList(quad) ((quad).objIndex)
#define HolderHitCount(pHolder) ((pHolder)->hitCount) // 983: sModel +0x129 (645: +0xe5)
#define BallFirstData(ball) \
	((ball).m_renderNum >= 0 ? &(ball).m_expected[0] : NULL)
// A174: os Shared* eram offsets crus do 645 (C89.2); agora membros (983: gameType +0x10594, CGolfDoc em +0x11024 com
// m_pHoleData +0xd8, m_currentHole +0x17c, m_currentPlayer +0x189, m_bDay +0x160, m_cameraData.clearance +0x10c, m_bNoStat +0x194).
#define SharedGameType() (CSharedDoc::Instance()->m_golfGame.gameType)
#define SharedCameraPos() \
	(reinterpret_cast<const float*>( \
		reinterpret_cast<char*>(GOLFDOC()->m_pHoleData) - 0x2c + GOLFDOC()->m_currentHole * 0x38))
#define PlayerBunkerState(pPlayer) ((pPlayer)->bunker) // 983: sPlayerData +0x4da (645: +0x4ca)
#define SharedApproachFlag() (GOLFDOC()->m_bNoStat) // 983 +0x194 (645 +0x180)
#define SharedTeeData() \
	(*reinterpret_cast<const sTeeData*>( \
		reinterpret_cast<char*>(GOLFDOC()->m_pHoleData) - 0x38 + GOLFDOC()->m_currentHole * 0x38))
#define PlayerPowerItem(pPlayer) ((pPlayer)->item) // 983: sPlayerData +0x4bc (645: +0x4b0)
#define PlayerPowerIndex(pPlayer) ((pPlayer)->bunker) // 983: sPlayerData +0x4da (645: +0x4ca)
#define PropertyPower(pProperty, index, offset) \
	(*reinterpret_cast<const float*>( \
		reinterpret_cast<const char*>((pProperty)) + (index) * 4 + (offset)))
#define PolyDoc(pSoup) \
	(*reinterpret_cast<CGolfDoc**>(reinterpret_cast<char*>((pSoup)) + 0x128)) // 983: m_pDoc +0x128 (645: +0x104)
#define PolyTextureNum(pSoup) \
	(*reinterpret_cast<int*>(reinterpret_cast<char*>((pSoup)) + 0x9c))
#define PolyTextures(pSoup) \
	(*reinterpret_cast<WPolySoup::sTexture**>( \
		reinterpret_cast<char*>((pSoup)) + 0xb0)) // 983: m_texture +0xb0 (645: +0xac)
#define SharedCurPlayerItem() \
	(CSharedDoc::Instance()->m_userInfo[GOLFDOC()->m_currentPlayer].userEquip.tidBall) // 983 HoleIn 00956563: +0x3b42 (645: * 0xb92 + 0x1524)
#define PolyObjNum(pSoup) \
	(*reinterpret_cast<int*>(reinterpret_cast<char*>((pSoup)) + 0xa8)) // 983: m_modelNum +0xa8 (645: +0xa4)
#define PolyStructuredObjNum(pSoup) ((pSoup)->GetRmrModelNum())
#define ModelObjIndex(pModel) ((pModel)->id) // 983: sModel +0x14c (645: +0x108)
#define PolyObjs(pSoup) \
	(*reinterpret_cast<sQuadPuppetHolder***>( \
		reinterpret_cast<char*>((pSoup)) + 0xcc)) // 983: m_model +0xcc (645: +0xc8; +0xc8 = m_attach983)
#define SharedCameraFlag() (GOLFDOC()->m_bDay) // 983 00958bdf: +0x160 (645 +0x150; B181.1, C101.1)
#define SharedCameraHeight() (GOLFDOC()->m_cameraData.clearance) // 983: fsub [doc+0x10c] (645 +0xfc)
#define PolyTexMap(pSoup) \
	(*reinterpret_cast<std::map<int, int>*>( \
		reinterpret_cast<char*>((pSoup)) + 0xf8)) // 983: m_texIndex +0xf8 (645: +0xd8)
#define PolyTexNames(pSoup) \
	(*reinterpret_cast<char**>(reinterpret_cast<char*>((pSoup)) + 0xb0)) // 983: m_texture +0xb0 (645: +0xac)
#if defined(_MSC_VER) && _MSC_VER > 1600
// A728: os crus acima sao do layout do VC10 (std::vector 0x10, std::map 0x10); no cl novo o WPolySoup encolhe
// (m_node, m_model, m_texIndex) e +0xcc caia fora do m_model -> vs2022-2 AV em MakeObjStructure lendo [0x14].
#undef PolyDoc
#undef PolyTextureNum
#undef PolyTextures
#undef PolyObjNum
#undef PolyObjs
#undef PolyTexMap
#undef PolyTexNames
#define PolyDoc(pSoup) ((pSoup)->m_pDoc)
#define PolyTextureNum(pSoup) ((pSoup)->m_header.textureNum)
#define PolyTextures(pSoup) ((pSoup)->m_texture)
#define PolyObjNum(pSoup) ((pSoup)->m_modelNum)
#define PolyObjs(pSoup) ((pSoup)->m_model.data())
#define PolyTexMap(pSoup) ((pSoup)->m_texIndex)
#define PolyTexNames(pSoup) (reinterpret_cast<char*>((pSoup)->m_texture))
#endif
#define PolyFirstPuppet(pSoup) (PolyObjs((pSoup))[0]->pet)
// 983: o raio do copo deixou de ser campo (m_holeCupRadius); cada função chama o 0046f540 (corpo na jp/aiplayer.cpp, frente B)
// com o tipo de jogo (CSharedDoc+0x10594) e as opções da sala (+0x10a99).
float __fastcall Scale983(int map, unsigned long channelType, float value);
#define HOLECUP_RADIUS983() \
	Scale983(Doc()->m_golfGame.gameType, \
		Doc()->m_curChannel.Type, GolfBall().m_radius)


struct Str10
{
	char b[10];
};


namespace intersection
{
	WVector __fastcall closestPointOnLine(const WVector& a, const WVector& b,
		const WVector& p);
	WVector __fastcall closestPointOnTriangle(const WVector& p,
		const WVector& a, const WVector& b, const WVector& c);
	float __fastcall intersectPolygon(const WVector& pos, const WVector& center,
		const WVector& vel, const float& radius);
	float __fastcall IntersectRayTriangle(const WVector& orig,
		const WVector& dir, const WVector* tri);
	bool __fastcall SameSide(const WVector& p1, const WVector& p2,
		const WVector& a, const WVector& b);
	bool __fastcall PointInTriangle(const WVector& p, const WVector& a,
		const WVector& b, const WVector& c);
	float __fastcall IntersectPlane(const WVector& pos, const WVector& vel,
		const WPlane& plane, bool bFront);
}

WVector __fastcall intersection::closestPointOnLine(const WVector& a,
	const WVector& b, const WVector& p)
{
	WVector c = p - a;
	WVector v = (b - a).Normalize();
	float d = WVectorLen(a - b);
	float t = v * c;
	if (t < 0.0f)
	{
		return a;
	}
	if (t > d)
	{
		return b;
	}
	v = v * t;
	return a + v;
}

WVector __fastcall intersection::closestPointOnTriangle(const WVector& p,
	const WVector& a, const WVector& b, const WVector& c)
{
	WVector rab = closestPointOnLine(a, b, p);
	WVector rbc = closestPointOnLine(b, c, p);
	WVector rca = closestPointOnLine(c, a, p);
	float dab = WVectorLen(rab - p);
	float dbc = WVectorLen(rbc - p);
	float dca = WVectorLen(rca - p);
	if (dab < dbc)
	{
		if (dab < dca)
		{
			return rab;
		}
		return rca;
	}
	if (dbc < dca)
	{
		return rbc;
	}
	return rca;
}


float __fastcall intersection::intersectPolygon(const WVector& pos,
	const WVector& center, const WVector& vel, const float& radius)
{
	WVector dir = vel * -1.0f;
	WVector diff = pos - center;
	float a = dir * dir;
	float b = diff * dir;
	float c = diff * diff - radius * radius;
	float disc = b * b - c * a;
	if (disc < 0.0f)
	{
		return 1.0f;
	}
	disc = sqrtf(disc);
	float inv = 1.0f / a;
	float t1 = (b * -1.0f - disc) * inv;
	float t2 = (disc - b) * inv;
	if (t1 < 0.0f)
	{
		if (t2 > 0.0f && t2 < 1.0f)
		{
			return t2;
		}
	}
	else if (t1 < 1.0f)
	{
		return t1;
	}
	return 1.0f;
}

float __fastcall intersection::IntersectRayTriangle(const WVector& orig,
	const WVector& dir, const WVector* tri)
{
	WVector edge1;
	WVector edge2;
	WVector pvec;
	WVector tvec;
	WVector qvec;
	edge1 = tri[1] - tri[0];
	edge2 = tri[2] - tri[0];
	pvec = WCrossProduct(dir, edge2);
	float det = pvec * edge1;
	if (Wabs(det) < g_EPSILON)
	{
		return 1.0f;
	}
	float invDet = 1.0f / det;
	tvec = orig - tri[0];
	float u = (pvec * invDet) * tvec;
	if (u < 0.0f || u > 1.0f)
	{
		return 1.0f;
	}
	qvec = WCrossProduct(tvec, edge1);
	float v = (qvec * invDet) * dir;
	if (v < 0.0f || v > 1.0f)
	{
		return 1.0f;
	}
	return Min((qvec * invDet) * edge2, 1.0f);
}

bool __fastcall intersection::SameSide(const WVector& p1, const WVector& p2,
	const WVector& a, const WVector& b)
{
	WVector cp1;
	WVector cp2;
	cp1 = WCrossProduct(b - a, p1 - a);
	cp2 = WCrossProduct(b - a, p2 - a);
	if (cp1 * cp2 > 0.0f)
	{
		return true;
	}
	if (WisEqual(cp1, WVector::ZERO, g_EPSILON) ||
		WisEqual(cp2, WVector::ZERO, g_EPSILON))
	{
		return true;
	}
	return false;
}

bool __fastcall intersection::PointInTriangle(const WVector& p,
	const WVector& a, const WVector& b, const WVector& c)
{
	if (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b))
	{
		return true;
	}
	return false;
}

float __fastcall intersection::IntersectPlane(const WVector& pos,
	const WVector& vel, const WPlane& plane, bool bFront)
{
	float d1 = plane * pos;
	float d2 = (pos + vel) * plane;
	if (vel * plane.normal > 0.0f)
	{
		return 1.0f;
	}
	if (bFront && d1 < 0.0f)
	{
		return 0.0f;
	}
	if (d1 * d2 <= 0.0f)
	{
		return Abs(d1) / (Abs(d1) + Abs(d2));
	}
	return 1.0f;
}

void __fastcall ReflectVector(const WVector& normal, WVector& v)
{
	float d = normal * v;
	d = d + d;
	v = v - normal * d;
}

// Separating axis test of two oriented boxes.
static bool ObbOverlap(const Wobb& a, const Wobb& b)
{
	double ea[3];
	double eb[3];
	WVector axisA[3];
	WVector axisB[3];
	double c[3][3];
	double absC[3][3];
	double d[3];
	double r, r0, r1;
	ea[0] = a.extend[0].Magnitude();
	ea[1] = a.extend[1].Magnitude();
	ea[2] = a.extend[2].Magnitude();
	eb[0] = b.extend[0].Magnitude();
	eb[1] = b.extend[1].Magnitude();
	eb[2] = b.extend[2].Magnitude();
	bool bParallel = false;
	WVector diff = b.center - a.center;
	int i;
	for (i = 0; i < 3; i++)
	{
		axisA[i] = a.extend[i];
		axisA[i].Normalize();
	}
	for (i = 0; i < 3; i++)
	{
		axisB[i] = b.extend[i];
		axisB[i].Normalize();
	}

	c[0][0] = WDotProduct(axisA[0], axisB[0]);
	absC[0][0] = Wabs((float)c[0][0]);
	if (absC[0][0] > 0.999999)
	{
		bParallel = true;
	}
	c[0][1] = WDotProduct(axisA[0], axisB[1]);
	absC[0][1] = Wabs((float)c[0][1]);
	if (absC[0][1] > 0.999999)
	{
		bParallel = true;
	}
	c[0][2] = WDotProduct(axisA[0], axisB[2]);
	absC[0][2] = Wabs((float)c[0][2]);
	if (absC[0][2] > 0.999999)
	{
		bParallel = true;
	}
	d[0] = axisA[0].y * diff.y + axisA[0].z * diff.z + axisA[0].x * diff.x;
	r = fabs(d[0]);
	r1 = absC[0][0] * eb[0] + absC[0][1] * eb[1] + absC[0][2] * eb[2];
	if (r > r1 + ea[0])
	{
		return false;
	}

	c[1][0] = WDotProduct(axisA[1], axisB[0]);
	absC[1][0] = Wabs((float)c[1][0]);
	if (absC[1][0] > 0.999999)
	{
		bParallel = true;
	}
	c[1][1] = WDotProduct(axisA[1], axisB[1]);
	absC[1][1] = Wabs((float)c[1][1]);
	if (absC[1][1] > 0.999999)
	{
		bParallel = true;
	}
	c[1][2] = WDotProduct(axisA[1], axisB[2]);
	absC[1][2] = Wabs((float)c[1][2]);
	if (absC[1][2] > 0.999999)
	{
		bParallel = true;
	}
	d[1] = axisA[1].y * diff.y + axisA[1].z * diff.z + diff.x * axisA[1].x;
	r = fabs(d[1]);
	r1 = absC[1][0] * eb[0] + absC[1][1] * eb[1] + absC[1][2] * eb[2];
	if (r > r1 + ea[1])
	{
		return false;
	}

	c[2][0] = WDotProduct(axisA[2], axisB[0]);
	absC[2][0] = Wabs((float)c[2][0]);
	if (absC[2][0] > 0.999999)
	{
		bParallel = true;
	}
	c[2][1] = WDotProduct(axisA[2], axisB[1]);
	absC[2][1] = Wabs((float)c[2][1]);
	if (absC[2][1] > 0.999999)
	{
		bParallel = true;
	}
	c[2][2] = WDotProduct(axisA[2], axisB[2]);
	absC[2][2] = Wabs((float)c[2][2]);
	if (absC[2][2] > 0.999999)
	{
		bParallel = true;
	}
	d[2] = diff.x * axisA[2].x + diff.z * axisA[2].z + diff.y * axisA[2].y;
	r = fabs(d[2]);
	r1 = absC[2][0] * eb[0] + absC[2][1] * eb[1] + absC[2][2] * eb[2];
	if (r > r1 + ea[2])
	{
		return false;
	}

	r = fabs(axisB[0].y * diff.y + axisB[0].z * diff.z + axisB[0].x * diff.x);
	r0 = absC[0][0] * ea[0] + absC[1][0] * ea[1] + absC[2][0] * ea[2];
	if (r > r0 + eb[0])
	{
		return false;
	}
	r = fabs(axisB[1].y * diff.y + axisB[1].z * diff.z + axisB[1].x * diff.x);
	r0 = absC[0][1] * ea[0] + absC[1][1] * ea[1] + absC[2][1] * ea[2];
	if (r > r0 + eb[1])
	{
		return false;
	}
	r = fabs(axisB[2].y * diff.y + axisB[2].z * diff.z + axisB[2].x * diff.x);
	r0 = absC[0][2] * ea[0] + absC[1][2] * ea[1] + absC[2][2] * ea[2];
	if (r > r0 + eb[2])
	{
		return false;
	}

	if (bParallel)
	{
		return true;
	}

	r = fabs(c[1][0] * d[2] - c[2][0] * d[1]);
	if (r > absC[0][2] * eb[1] + absC[0][1] * eb[2] + absC[1][0] * ea[2] +
			absC[2][0] * ea[1])
	{
		return false;
	}
	r = fabs(c[1][1] * d[2] - c[2][1] * d[1]);
	if (r > absC[0][2] * eb[0] + absC[0][0] * eb[2] + absC[1][1] * ea[2] +
			absC[2][1] * ea[1])
	{
		return false;
	}
	r = fabs(c[1][2] * d[2] - c[2][2] * d[1]);
	if (r > absC[0][1] * eb[0] + absC[0][0] * eb[1] + absC[1][2] * ea[2] +
			absC[2][2] * ea[1])
	{
		return false;
	}
	r = fabs(c[2][0] * d[0] - c[0][0] * d[2]);
	if (r > absC[1][2] * eb[1] + absC[1][1] * eb[2] + absC[0][0] * ea[2] +
			absC[2][0] * ea[0])
	{
		return false;
	}
	r = fabs(c[2][1] * d[0] - c[0][1] * d[2]);
	if (r > absC[1][2] * eb[0] + absC[1][0] * eb[2] + absC[0][1] * ea[2] +
			absC[2][1] * ea[0])
	{
		return false;
	}
	r = fabs(c[2][2] * d[0] - c[0][2] * d[2]);
	if (r > absC[1][1] * eb[0] + absC[1][0] * eb[1] + absC[0][2] * ea[2] +
			absC[2][2] * ea[0])
	{
		return false;
	}
	// The three axes below use the |c[2][*]| row for the second box, as the original does.
	r = fabs(c[0][0] * d[1] - c[1][0] * d[0]);
	if (r > absC[0][0] * ea[1] + absC[1][0] * ea[0] + absC[2][2] * eb[1] +
			absC[2][1] * eb[2])
	{
		return false;
	}
	r = fabs(c[0][1] * d[1] - c[1][1] * d[0]);
	if (r > absC[0][1] * ea[1] + absC[1][1] * ea[0] + absC[2][2] * eb[0] +
			absC[2][0] * eb[2])
	{
		return false;
	}
	r = fabs(c[0][2] * d[1] - c[1][2] * d[0]);
	if (r > absC[0][2] * ea[1] + absC[1][2] * ea[0] + absC[2][1] * eb[0] +
			absC[2][0] * eb[1])
	{
		return false;
	}
	return true;
}

int CPVSPool::ms_InstanceIdx;

CQuadTree& CPVSPool::GetInstance()
{
	return m_pool[ms_InstanceIdx];
}

void __fastcall CPVSPool::SetInstanceIdx(int idx)
{
	ms_InstanceIdx = idx;
}

void __fastcall SetPVSIndex(int idx)
{
	CPVSPool::ms_InstanceIdx = idx;
}

CQuadTree& __fastcall GetPVS()
{
	static CPVSPool s_pool;
	return s_pool.GetInstance();
}

void CQuadTree::AddWater(Waabb aabb, int type)
{
	sWater water;
	water.aabb = aabb;
	water.type = type;
	m_water.push_back(water);
}

// 983 (00960310): o ponto do green vem do GetGroundPoint (sem objeto, sem água); o resto da limpeza é o ResetVariable
// fora de linha; a posição da câmera está em CSharedDoc+0x11024 (+0xd8, índice em +0x17c).
// A730: membros (cl novo m_pGolfDoc 0x10cc4); +0xd8 - 0x2c + hole*0x38 = m_pHoleData[hole - 1].pin (+0xc)
#define SharedCameraPos983() \
	(reinterpret_cast<const float*>( \
		reinterpret_cast<int>(CSharedDoc::Instance()->m_pGolfDoc->m_pHoleData) - 0x2c + \
		CSharedDoc::Instance()->m_pGolfDoc->m_currentHole * 0x38))
void CQuadTree::ResetHoleData()
{
	m_greenAabb.min.z = 0.0f;
	m_greenAabb.min.y = 0.0f;
	m_greenAabb.min.x = 0.0f;
	m_greenAabb.max.z = 0.0f;
	m_greenAabb.max.y = 0.0f;
	m_greenAabb.max.x = 0.0f;
	for (int i = 0; i < m_pPolySoup->GetAreaNum(); i++)
	{
		if (m_pPolySoup->GetArea(i)->type == 7)
		{
			m_greenAabb = m_pPolySoup->GetArea(i)->aabb;
			break;
		}
	}
	WPolySoup::sPoint* pPoint = m_pPolySoup->FindPoint("\xc1\xdf\xbd\xc9");
	m_greenPoint = pPoint != NULL
		? GetGroundPoint(pPoint->pos, false, NULL, false)
		: GetGroundPoint(*reinterpret_cast<const WVector*>(SharedCameraPos983()), false, NULL, false);
	const float* pCam = SharedCameraPos983();
	m_ballFloorY = pCam[1] - (float)(GolfBall().m_radius * 3.0 * 2.0);
	m_frame = 0;
	m_randSeed = 0;
	m_paintIndex = -1;
	m_frameTime = 0.0f;
	m_gravity = 1.0f;
	ResetVariable();
}

void CQuadTree::ResetVariable()
{
	m_colList.clear();
	g_colNum = 0;
	m_bCollideBallAndCaddie = false;
	m_bounceCount = 0;
	m_cupRimFrames = 0;
	m_bMeshListOverflow = false;
	m_bApplyWind = true;
}

CQuadTree::CQuadTree()
	: m_pPolySoup(NULL), m_pHolder(NULL)
{
	// HACK: the original still emitted these wmath.h constants into .rdata
	// (their uses are folded: 1.6f = 5 * g_CM_TO_WU).
	(void)g_HUGE;
	(void)g_CM_TO_WU;
	m_pQuad = NULL;
	m_vec21d8_983 = WVector::ZERO;
	m_slopeFactor = 1.1f;
	m_paintIndex = -1;
	m_bounceFactor = 1.0f;
	m_rollFactor = 1.0f;
	m_greenFactor = 0.2f;
	g_colNum = 0;
	m_baseColor = WVector(255.0f, 255.0f, 255.0f);
	m_objColor = WVector(255.0f, 255.0f, 255.0f);
	m_vtxColorMode = 0;
	m_gravity = 1.0f;
}

CQuadTree::~CQuadTree()
{
	for (int i = 0; i < m_quadWidth * m_quadHeight; i++)
	{
		if (m_pQuad[i].meshIndex != NULL)
		{
			delete[] m_pQuad[i].meshIndex;
			m_pQuad[i].meshIndex = NULL;
		}
	}
	if (m_pQuad != NULL)
	{
		delete[] m_pQuad;
		m_pQuad = NULL;
	}
	m_water.clear();
	m_colList.clear();
	for (unsigned int j = 0; j < m_aniObjs.size(); j++)
	{
		if (m_aniObjs[j] != NULL)
		{
			delete m_aniObjs[j];
			m_aniObjs[j] = NULL;
		}
	}
	m_aniObjs.clear();
	if (m_pArray2220 != NULL)
	{
		delete[] m_pArray2220;
		m_pArray2220 = NULL;
	}
	if (m_pArray2224 != NULL)
	{
		delete[] m_pArray2224;
		m_pArray2224 = NULL;
	}
	if (m_pArray2224 != NULL)
	{
		delete[] m_pArray2224;
		m_pArray2224 = NULL;
	}
}

void CQuadTree::Reserve(short num)
{
	for (unsigned int i = 0; i < m_aniObjs.size(); i++)
	{
		if (m_aniObjs[i] != NULL)
		{
			delete m_aniObjs[i];
			m_aniObjs[i] = NULL;
		}
	}
	m_aniObjs.clear();
	if (num > 0)
	{
		m_aniObjs.reserve(num);
	}
}

unsigned long CQuadTree::MakeRand(unsigned long limit)
{
	// 983 (0094afb0)
	unsigned long seed = m_randSeed;
	if (seed < limit)
	{
		do
		{
			seed = seed * seed + 1;
		} while (seed < limit);
		m_randSeed = seed;
	}
	m_randSeed = m_randSeed % limit;
	return m_randSeed;
}

void CQuadTree::MakeObjStructure(short objIndex, int option, bool bRebuild)
{
	WPolySoup::sModel* pModel;
	if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		if (option != -1)
		{
			pModel = PolyObjs(m_pPolySoup)[option];
		}
		else
		{
			pModel = PolyObjs(m_pPolySoup)[objIndex];
		}
	}
	else
	{
		pModel = PolyObjs(m_pPolySoup)[objIndex];
	}
	if (pModel->pet == NULL)
	{
		for (unsigned int i = 0; i < m_aniObjs.size(); i++)
		{
			if (m_aniObjs[i] != NULL && m_aniObjs[i]->m_id == pModel->id)
			{
				delete m_aniObjs[i];
				m_aniObjs[i] = NULL;
				return;
			}
		}
		return;
	}
	if (pModel->type != 0)
	{
		CAniObj* pAni = NULL;
		if (bRebuild)
		{
			for (unsigned int i = 0; i < m_aniObjs.size(); i++)
			{
				if (m_aniObjs[i] != NULL && m_aniObjs[i]->m_id == pModel->id)
				{
					pAni = m_aniObjs[i];
					break;
				}
			}
		}
		int randRange;
		if (pModel->pet->GetAnimationLength() > 0.0)
		{
			randRange = (int)(pModel->pet->GetAnimationLength() * 50.0f);
		}
		else
		{
			randRange = 50;
		}
		switch (pModel->type)
		{
		case 1:
		{
			CMoveObj* pMove;
			if (bRebuild)
			{
				pMove = static_cast<CMoveObj*>(pAni);
			}
			else
			{
				pMove = new CMoveObj;
			}
			pMove->m_standTime = 0.0f;
			pMove->m_moveTime = 1.0f;
			pMove->m_startPos = pModel->mat.pivot;
			pMove->m_endPos = pModel->mat.pivot;
			if (pModel->option[0] != '\0')
			{
				char node[0x20];
				WSliceStr slice = pModel->option;
				slice.Get("*node", "%s", node);
				slice.Get("*stand", "%f", &pMove->m_standTime);
				slice.Get("*move", "%f", &pMove->m_moveTime);
				CNodeContainer* pNode = m_pPolySoup->FindNode(node);
				if (pNode != NULL)
				{
					pMove->m_startPos = pNode->GetList()[0];
					pMove->m_endPos = pNode->GetList()[1];
					pModel->mat.pivot = pNode->GetList()[0];
				}
			}
			pAni = pMove;
			break;
		}
		case 2:
		{
			CRotObj* pRot = static_cast<CRotObj*>(pAni);
			if (!bRebuild)
			{
				pRot = new CRotObj;
			}
			float dy = pModel->aabb.max.y - pModel->aabb.min.y;
			float dx = pModel->aabb.max.x - pModel->aabb.min.x;
			float dz = pModel->aabb.max.z - pModel->aabb.min.z;
			pRot->m_invSize = 1.0f / Max(Max(dx, dy), dz);
			pAni = pRot;
			break;
		}
		case 3:
			if (!bRebuild)
			{
				pAni = new CVtxObj;
			}
			break;
		case 4:
			if (!bRebuild)
			{
				pAni = new CTranObj;
			}
			break;
		}
		if (pAni != NULL)
		{
			pAni->m_rand = MakeRand(randRange);
			pAni->m_type = pModel->type;
			pAni->m_id = objIndex;
			if (!bRebuild)
			{
				m_aniObjs.push_back(pAni);
			}
		}
	}
	if (pModel->noCollision == 3)
	{
		return;
	}
	float lo[3] = { 0.0f, 0.0f, 0.0f };
	float hi[3] = { 0.0f, 0.0f, 0.0f };
	if (pModel->type != 1 && pModel->type != 3)
	{
		int minCol = (int)((pModel->aabb.min.x - m_quadOrigin.x) * 0.05f);
		if (minCol < 0)
		{
			minCol = 0;
		}
		else if (minCol > m_quadWidth - 1)
		{
			minCol = m_quadWidth - 1;
		}
		int maxCol = (int)((pModel->aabb.max.x - m_quadOrigin.x) * 0.05f);
		if (maxCol < 0)
		{
			maxCol = 0;
		}
		else if (maxCol > m_quadWidth - 1)
		{
			maxCol = m_quadWidth - 1;
		}
		int minRow = (int)((pModel->aabb.min.z - m_quadOrigin.z) * 0.05f);
		if (minRow < 0)
		{
			minRow = 0;
		}
		else if (minRow > m_quadHeight - 1)
		{
			minRow = m_quadHeight - 1;
		}
		int maxRow = (int)((pModel->aabb.max.z - m_quadOrigin.z) * 0.05f);
		if (maxRow < 0)
		{
			maxRow = 0;
		}
		else if (maxRow > m_quadHeight - 1)
		{
			maxRow = m_quadHeight - 1;
		}
		for (int row = minRow; row <= maxRow; row++)
		{
			for (int col = minCol; col <= maxCol; col++)
			{
				int index = m_quadWidth * row + col;
				if (m_pQuad[index].objNum == 0)
				{
					m_pQuad[index].objIndex = new short[4];
					m_pQuad[index].objIndex[0] = objIndex;
					m_pQuad[index].objNum = 1;
					continue;
				}
				int j;
				for (j = 0; j < m_pQuad[index].objNum; j++)
				{
					if (m_pQuad[index].objIndex[j] == objIndex)
					{
						break;
					}
				}
				if (j < m_pQuad[index].objNum)
				{
					continue;
				}
				if (m_pQuad[index].objNum % 4 == 0)
				{
					short* pTemp = new short[m_pQuad[index].objNum];
					memcpy(pTemp, m_pQuad[index].objIndex,
						m_pQuad[index].objNum * sizeof(short));
					delete[] m_pQuad[index].objIndex;
					m_pQuad[index].objIndex =
						new short[m_pQuad[index].objNum + 4];
					memcpy(m_pQuad[index].objIndex, pTemp,
						m_pQuad[index].objNum * sizeof(short));
					delete[] pTemp;
				}
				m_pQuad[index].objIndex[m_pQuad[index].objNum] = objIndex;
				m_pQuad[index].objNum++;
			}
		}
	}
	if (pModel->noCollision & 2)
	{
		return;
	}
	if (bRebuild)
	{
		pModel->tri.clear();
		if (pModel->quad != NULL)
		{
			delete[] pModel->quad;
		}
	}
	int faceNum = PuppetFaceNum(pModel->pet);
	pModel->tri.resize(faceNum);
	bool bHasAbv = pModel->pet->FindBoneByTail("abv") != NULL;
	int triIndex = 0;
	w_bound_box* pBox;
	for (pBox = pModel->pet->m_bbList.Start(); pBox != NULL;
		pBox = pModel->pet->m_bbList.Next())
	{
		WBone* pBone = pBox->bone;
		if (_stricmp(pBox->info->name, pBone->GetBoneName()) != 0)
		{
			continue;
		}
		if (bHasAbv)
		{
			if (strstr(_strlwr(pBox->info->name), "abv") == NULL)
			{
				continue;
			}
		}
		for (w_mesh* pMesh = BoneMesh(pBone); pMesh != NULL;
			pMesh = pMesh->next)
		{
			// 983 (00963233..00963257): os vertices do mundo vem do WBone::GetWorldVertices983 (00ba6410), uma vez por
			// malha; o 645 multiplicava vecList pela matriz do osso
			std::vector<WVector> worldVtx;
			if (pModel->type == 0 || pModel->type == 4)
			{
				WBone::GetWorldVertices983(pBone, pMesh, worldVtx);
			}
			for (int k = 0; k < pMesh->indexNum; k += 3)
			{
				WPolySoup::sTriangle* pTri = &pModel->tri[triIndex];
				if (pModel->type == 0 || pModel->type == 4)
				{
					for (int j = 0; j < 3; j++)
					{
						pTri->v[j] = worldVtx[pMesh->indexList[k + j]];
					}
					pTri->plane.normal = WCrossProduct(pTri->v[2] - pTri->v[1],
						pTri->v[0] - pTri->v[1]);
					pTri->plane.normal.Normalize();
					pModel->tri[triIndex].plane.d =
						-pModel->tri[triIndex].plane.normal.z * pTri->v[0].z +
						-pModel->tri[triIndex].plane.normal.y * pTri->v[0].y +
						-pModel->tri[triIndex].plane.normal.x * pTri->v[0].x;
				}
				pModel->tri[triIndex].bone = pBone;
				pModel->tri[triIndex].bDoubleSide =
					(pMesh->xiDrawFlag2 >> 9) & 1; // 983: dupla face 0x200
				pModel->tri[triIndex].propIndex = *reinterpret_cast<int*>(
					PolyTexNames(m_pPolySoup) +
					PolyTexMap(m_pPolySoup)[pMesh->texHandle & 0x7ff] * 0x28 +
					0x24);
				pModel->tri[triIndex].texHandle = *reinterpret_cast<int*>(
					PolyTexNames(m_pPolySoup) +
					PolyTexMap(m_pPolySoup)[pMesh->texHandle & 0x7ff] * 0x28 +
					0x20);
				triIndex++;
				if (triIndex >= faceNum)
				{
					goto trianglesDone;
				}
			}
		}
	}
trianglesDone:
	if (PuppetFaceNum(pModel->pet) < 0x200 || pModel->type == 2 ||
		pModel->type == 1 || pModel->type == 3)
	{
		pModel->quad = new WPolySoup::sQuad[1];
		pModel->bSingleQuad = true;
		pModel->quad[0].meshIndex = new int[PuppetFaceNum(pModel->pet)];
		pModel->quad[0].meshNum = (short)PuppetFaceNum(pModel->pet);
		for (int i = 0; i < PuppetFaceNum(pModel->pet); i++)
		{
			pModel->quad[0].meshIndex[i] = i;
		}
		return;
	}
	pModel->quad = new WPolySoup::sQuad[8];
	pModel->bSingleQuad = false;
	pModel->pos.x = 0.0f;
	pModel->pos.y = 0.0f;
	pModel->pos.z = 0.0f;
	int vertexCount = 0;
	for (pBox = pModel->pet->m_bbList.Start(); pBox != NULL;
		pBox = pModel->pet->m_bbList.Next())
	{
		WBone* pBone = pBox->bone;
		if (_stricmp(pBox->info->name, pBone->GetBoneName()) != 0)
		{
			continue;
		}
		for (w_mesh* pMesh = BoneMesh(pBone); pMesh != NULL;
			pMesh = pMesh->next)
		{
			for (int v = 0; v < pMesh->vtxNum; v++)
			{
				WVector pos = pMesh->vecList[v] * pBone->GetMatrix();
				vertexCount++;
				pModel->pos.x += pos.x;
				pModel->pos.y += pos.y;
				pModel->pos.z += pos.z;
			}
		}
	}
	float invCount = 1.0f / vertexCount;
	pModel->pos.x *= invCount;
	pModel->pos.y *= invCount;
	pModel->pos.z *= invCount;
	for (int i = 0; i < PuppetFaceNum(pModel->pet); i++)
	{
		WPolySoup::sTriangle* pTri = &pModel->tri[i];
		lo[0] = Min(Min(pTri->v[0].x, pTri->v[1].x), pTri->v[2].x);
		lo[1] = Min(Min(pTri->v[0].y, pTri->v[1].y), pTri->v[2].y);
		lo[2] = Min(Min(pTri->v[0].z, pTri->v[1].z), pTri->v[2].z);
		hi[0] = Max(Max(pTri->v[0].x, pTri->v[1].x), pTri->v[2].x);
		hi[1] = Max(Max(pTri->v[0].y, pTri->v[1].y), pTri->v[2].y);
		hi[2] = Max(Max(pTri->v[0].z, pTri->v[1].z), pTri->v[2].z);
		int octants[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
		const float* pCenter = &pModel->pos.x;
		for (int axis = 0; axis < 3; axis++)
		{
			int bit = 1 << axis;
			if (hi[axis] < pCenter[axis])
			{
				for (int q = 0; q < 8; q++)
				{
					if ((bit & q) == bit)
					{
						octants[q] = 0;
					}
				}
			}
			else if (lo[axis] > pCenter[axis])
			{
				for (int q = 0; q < 8; q++)
				{
					if (!(q & bit))
					{
						octants[q] = 0;
					}
				}
			}
		}
		for (int q = 0; q < 8; q++)
		{
			if (octants[q] != 1)
			{
				continue;
			}
			if (pModel->quad[q].meshNum == 0)
			{
				pModel->quad[q].meshIndex = new int[0x40];
				pModel->quad[q].meshIndex[0] = i;
				pModel->quad[q].meshNum = 1;
				continue;
			}
			if (pModel->quad[q].meshNum % 0x40 == 0)
			{
				int* pTemp = new int[pModel->quad[q].meshNum];
				memcpy(pTemp, pModel->quad[q].meshIndex,
					pModel->quad[q].meshNum * sizeof(int));
				delete[] pModel->quad[q].meshIndex;
				pModel->quad[q].meshIndex =
					new int[pModel->quad[q].meshNum + 0x40];
				memcpy(pModel->quad[q].meshIndex, pTemp,
					pModel->quad[q].meshNum * sizeof(int));
				delete[] pTemp;
			}
			pModel->quad[q].meshIndex[pModel->quad[q].meshNum] = i;
			pModel->quad[q].meshNum++;
		}
	}
}

unsigned char CQuadTree::GetObjQuad(const WVector& a, const WVector& b)
{
	unsigned char quad = 0;
	if (a.x > b.x)
	{
		quad = 1;
	}
	if (a.y > b.y)
	{
		quad |= 2;
	}
	if (a.z > b.z)
	{
		quad |= 4;
	}
	return quad;
}

CQuadTree::sColResponse::GolfBallClone::GolfBallClone(const CGolfBall& ball)
	: m_pos(ball.m_pos), m_vel(ball.m_vel)
{
	m_colObjIndex = ball.m_colObjIndex;
	m_colTriIndex = ball.m_colTriIndex;
	m_state = ball.m_state;
	m_pBall = const_cast<CGolfBall*>(&ball);
}

bool CQuadTree::ObjColTest(const WVector& dir, short objIndex, WPlane* pPlane,
	float& t, int& triIndex)
{
	sQuadPuppetHolder* pObj = PolyObjs(m_pPolySoup)[objIndex];
	bool bHit = false;
	if (!pObj->bSingleQuad)
	{
		unsigned char from = GetObjQuad(GolfBall().m_pos, pObj->pos);
		unsigned char to = GetObjQuad(GolfBall().m_pos + dir, pObj->pos);
		if (from == to)
		{
			m_meshNum = pObj->quad[from].meshNum;
			memcpy(m_meshList, pObj->quad[from].meshIndex,
				m_meshNum * sizeof(int));
		}
		else
		{
			int i = 0;
			m_meshNum = 0;
			int j = 0;
			if (pObj->quad[from].meshNum != 0)
			{
				while (pObj->quad[to].meshNum != 0)
				{
					if (pObj->quad[from].meshIndex[i] <
						pObj->quad[to].meshIndex[j])
					{
						m_meshList[m_meshNum] = pObj->quad[from].meshIndex[i];
						i++;
					}
					else
					{
						if (pObj->quad[to].meshIndex[j] <
							pObj->quad[from].meshIndex[i])
						{
							m_meshList[m_meshNum] = pObj->quad[to].meshIndex[j];
						}
						else
						{
							m_meshList[m_meshNum] =
								pObj->quad[from].meshIndex[i];
							i++;
						}
						j++;
					}
					m_meshNum++;
					if (i == pObj->quad[from].meshNum)
					{
						if (j != pObj->quad[to].meshNum)
						{
							int num = Min(pObj->quad[to].meshNum - j,
								0x800 - m_meshNum);
							if (num < 0)
							{
								m_bMeshListOverflow = true;
								return false;
							}
							memcpy(&m_meshList[m_meshNum],
								&pObj->quad[to].meshIndex[j],
								num * sizeof(int));
							m_meshNum += num;
						}
						break;
					}
					if (j == pObj->quad[to].meshNum)
					{
						int num = Min(pObj->quad[from].meshNum - i,
							0x800 - m_meshNum);
						if (num < 0)
						{
							m_bMeshListOverflow = true;
							return false;
						}
						memcpy(&m_meshList[m_meshNum],
							&pObj->quad[from].meshIndex[i], num * sizeof(int));
						m_meshNum += num;
						break;
					}
					if (pObj->quad[from].meshNum == 0)
					{
						break;
					}
				}
			}
			if (m_meshNum > 0x800)
			{
				m_bMeshListOverflow = true;
			}
		}
	}
	else
	{
		m_meshNum = pObj->quad[0].meshNum;
		memcpy(m_meshList, pObj->quad[0].meshIndex, m_meshNum * sizeof(int));
	}
	float lo;
	float hi;
	if (dir.y > 0.0f)
	{
		lo = GolfBall().m_pos.y - GolfBall().m_radius;
		hi = GolfBall().m_pos.y + dir.y + GolfBall().m_radius;
	}
	else
	{
		lo = GolfBall().m_pos.y - GolfBall().m_radius + dir.y;
		hi = GolfBall().m_radius + GolfBall().m_pos.y;
	}
	for (int i = 0; i < m_meshNum; i++)
	{
		if (objIndex == GolfBall().m_colObjIndex &&
			i == GolfBall().m_colTriIndex)
		{
			continue;
		}
		int index = m_meshList[i];
		if ((BoneFlags(pObj->tri[index].bone) >> 0x12 & 1) != 0)
		{
			continue;
		}
		if ((m_pPolySoup->GetProperty(pObj->tri[index].propIndex).pass & 1) !=
			0)
		{
			if ((GolfBall().m_special & 0x10) != 0)
			{
				continue;
			}
			if ((GolfBall().m_special & 0x20) != 0 &&
				GolfBall().m_cobraFrame < 0)
			{
				continue;
			}
			if (WisZero(
					m_pPolySoup->GetProperty(pObj->tri[index].propIndex).bound -
					1.0f))
			{
				continue;
			}
			if ((GolfBall().m_special & 0x40) != 0)
			{
				continue;
			}
		}
		int above = 0;
		int below = 0;
		pObj->tri[index].bEdge = false;
		WVector v[3];
		for (int k = 0; k < 3; k++)
		{
			v[k] = pObj->tri[index].v[k];
			if (v[k].y > hi)
			{
				above++;
			}
			else if (v[k].y < lo)
			{
				below++;
			}
			if (GolfBall().m_bounceFrame < 0 && GolfBall().m_nearObjFrame < 0 &&
				(GolfBall().m_pos - v[k]).Magnitude() < 3.0f)
			{
				GolfBall().m_nearObjFrame = GolfBall().m_renderNum;
			}
		}
		if (below == 3 || above == 3)
		{
			continue;
		}
		WPlane plane = pObj->tri[index].plane;
		if (pObj->tri[index].bDoubleSide &&
			plane.x * GolfBall().m_pos.x + plane.z * GolfBall().m_pos.z +
					plane.y * GolfBall().m_pos.y + plane.dis <
				0.0f)
		{
			plane.x = -plane.x;
			plane.y = -plane.y;
			plane.z = -plane.z;
			plane.dis = plane.dis * -1.0f;
		}
		WVector offset = plane.normal * GolfBall().m_radius;
		WVector start = GolfBall().m_pos - offset;
		float hitT = intersection::IntersectPlane(start, dir, plane, false);
		WVector hit = dir * hitT + start;
		if (hitT < t)
		{
			if (intersection::PointInTriangle(hit, v[0], v[1], v[2]))
			{
				t = hitT;
				*pPlane = pObj->tri[index].plane;
				triIndex = index;
				bHit = true;
				if (pObj->tri[index].bDoubleSide &&
					GolfBall().m_pos.x * pObj->tri[index].plane.x +
							GolfBall().m_pos.y * pObj->tri[index].plane.y +
							GolfBall().m_pos.z * pObj->tri[index].plane.z +
							pObj->tri[index].plane.dis <
						0.0f)
				{
					pPlane->x = -pPlane->x;
					pPlane->y = -pPlane->y;
					pPlane->z = -pPlane->z;
					pPlane->dis = pPlane->dis * -1.0f;
				}
			}
			else
			{
				WVector closest =
					intersection::closestPointOnTriangle(hit, v[0], v[1], v[2]);
				float radius = GolfBall().m_radius;
				float edgeT = intersection::intersectPolygon(closest,
					GolfBall().m_pos, dir, radius);
				if (edgeT < t)
				{
					pObj->tri[index].bEdge = true;
					t = edgeT;
					*pPlane = pObj->tri[index].plane;
					triIndex = index;
					bHit = true;
					if (pObj->tri[index].bDoubleSide &&
						pObj->tri[index].plane.x * GolfBall().m_pos.x +
								GolfBall().m_pos.y * pObj->tri[index].plane.y +
								GolfBall().m_pos.z * pObj->tri[index].plane.z +
								pObj->tri[index].plane.dis <
							0.0f)
					{
						pPlane->x = -pPlane->x;
						pPlane->y = -pPlane->y;
						pPlane->z = -pPlane->z;
						pPlane->dis = pPlane->dis * -1.0f;
					}
				}
			}
		}
	}
	return bHit;
}

void CQuadTree::MakeBaseStructure(WPolySoup::sModel* pModel, bool bRebuild)
{
	ResetWater();
	m_pHolder = pModel;
	if (m_pQuad != NULL)
	{
		delete[] m_pQuad;
		m_pQuad = NULL;
	}
	const Waabb& aabb = m_pHolder->aabb;
	m_groundHeight = 100000.0f;
	m_quadOrigin = WVector(aabb.min.x, 0.0f, aabb.min.z);
	m_quadWidth =
		1 - (int)((m_pHolder->aabb.max.x - m_pHolder->aabb.min.x) * -0.05f);
	m_quadHeight =
		1 - (int)((m_pHolder->aabb.max.z - m_pHolder->aabb.min.z) * -0.05f);
	if (bRebuild)
	{
		if (m_pQuad != NULL)
		{
			delete[] m_pQuad;
		}
		m_pQuad = NULL;
	}
	m_pQuad = new WPolySoup::sQuad[m_quadWidth * m_quadHeight];
	for (int i = 0; i < PuppetFaceNum(m_pHolder->pet); i++)
	{
		WPolySoup::sTriangle& tri = m_pHolder->tri[i];
		if (tri.bDoubleSide)
		{
			if (Abs(m_pHolder->tri[i].plane.normal.y) >
				(float)cos(20 * g_DEGTORAD))
			{
				m_groundHeight = Min(Min(m_groundHeight, tri.v[0].y),
					Min(tri.v[1].y, tri.v[2].y));
			}
		}
		else if (m_pHolder->tri[i].plane.normal.y > (float)cos(20 * g_DEGTORAD))
		{
			m_groundHeight = Min(Min(m_groundHeight, tri.v[0].y),
				Min(tri.v[1].y, tri.v[2].y));
		}
		float minX = Min(tri.v[0].x, Min(tri.v[1].x, tri.v[2].x));
		float maxX = Max(tri.v[0].x, Max(tri.v[1].x, tri.v[2].x));
		float minZ = Min(tri.v[0].z, Min(tri.v[1].z, tri.v[2].z));
		float maxZ = Max(tri.v[0].z, Max(tri.v[1].z, tri.v[2].z));
		minX -= m_quadOrigin.x;
		maxX -= m_quadOrigin.x;
		minZ -= m_quadOrigin.z;
		maxZ -= m_quadOrigin.z;
		int minCol = (int)(minX * 0.05f);
		int maxCol = (int)(maxX * 0.05f);
		int minRow = (int)(minZ * 0.05f);
		int maxRow = (int)(maxZ * 0.05f);
		for (int row = minRow; row <= maxRow; row++)
		{
			for (int col = minCol; col <= maxCol; col++)
			{
				int index = row * m_quadWidth + col;
				if (m_pQuad[index].meshNum == 0)
				{
					m_pQuad[index].meshIndex = new int[0x10];
					m_pQuad[index].meshIndex[0] = i;
					m_pQuad[index].meshNum = 1;
					continue;
				}
				int j;
				for (j = 0; j < m_pQuad[index].meshNum; j++)
				{
					if (m_pQuad[index].meshIndex[j] == i)
					{
						break;
					}
				}
				if (j < m_pQuad[index].meshNum)
				{
					continue;
				}
				if (m_pQuad[index].meshNum % 0x10 == 0)
				{
					int* pTemp = new int[m_pQuad[index].meshNum];
					memcpy(pTemp, m_pQuad[index].meshIndex,
						m_pQuad[index].meshNum * sizeof(int));
					delete[] m_pQuad[index].meshIndex;
					m_pQuad[index].meshIndex =
						new int[m_pQuad[index].meshNum + 0x10];
					memcpy(m_pQuad[index].meshIndex, pTemp,
						m_pQuad[index].meshNum * sizeof(int));
					delete[] pTemp;
				}
				m_pQuad[row * m_quadWidth + col]
					.meshIndex[m_pQuad[row * m_quadWidth + col].meshNum] = i;
				m_pQuad[row * m_quadWidth + col].meshNum++;
			}
		}
	}
	m_groundHeight = Max(m_groundHeight, m_pHolder->aabb.min.y) - 1.0f;
}

WVector CQuadTree::ColTest(const WVector& pos, bool bKeepPos, int* pIndex,
	bool bAll)
{
	WVector p;
	p.x = pos.x;
	p.z = pos.z;
	p.y = -100000.0f;
	bool bFound = false;
	for (int i = 0; i < m_meshNum; i++)
	{
		int index = m_meshList[i];
		if (!bAll && m_pHolder->tri[index].bDoubleSide)
			continue;
		const WPolySoup::sTriangle* tris = &m_pHolder->tri[0];
		if (!tris[index].bDoubleSide && tris[index].plane.normal.y < 0.0f)
			continue;
		if (Abs(tris[index].plane.normal.y) < 0.1f)
			continue;
		WVector a = tris[index].v[0];
		a.y = p.y;
		WVector b = tris[index].v[1];
		b.y = p.y;
		WVector c = tris[index].v[2];
		c.y = p.y;
		if (intersection::SameSide(p, a, b, c) &&
			intersection::SameSide(p, b, a, c) &&
			intersection::SameSide(p, c, a, b))
		{
			const WPlane& plane = m_pHolder->tri[index].plane;
			float y =
				((p.x * plane.normal.x + p.z * plane.normal.z + plane.dis) /
					plane.normal.y) *
				-1.0f;
			if (p.y < y)
			{
				bFound = true;
				p.y = y;
				if (pIndex != NULL)
					*pIndex = index;
			}
		}
	}
	if (!bFound)
	{
		if (bKeepPos)
		{
			if (pIndex != NULL)
				*pIndex = -1;
			p.x = pos.x;
			p.y = pos.y;
			p.z = pos.z;
			return p;
		}
		if (GolfBall().m_pos.y < m_pHolder->aabb.min.y)
			p.y = m_pHolder->aabb.min.y;
	}
	return p;
}

WVector CQuadTree::ColObject(const WVector& pos, bool bKeepPos, int* pIndex,
	bool bAll)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return pos;
	}
	WVector p = pos;
	sQuadPuppetHolder* pObj = PolyObjs(m_pPolySoup)[0];
	int num = pObj->quad->meshNum;
	float y = p.y;
	for (int i = 0; i < num; i++)
	{
		int tri = pObj->quad->meshIndex[i];
		if (bAll || !pObj->tri[0].bDoubleSide)
		{
			WPolySoup::sTriangle* pTri = &pObj->tri[tri];
			if (pTri->bDoubleSide || pTri->plane.normal.y >= 0.0f)
			{
				float ny = pTri->plane.normal.y;
				if (ny <= 0.0f)
				{
					ny = -ny;
				}
				if (0.1f <= ny)
				{
					WVector a = pTri->v[0];
					WVector b = pTri->v[1];
					WVector c = pTri->v[2];
					if (intersection::SameSide(p, a, b, c) &&
						intersection::SameSide(p, b, a, c) &&
						intersection::SameSide(p, c, a, b))
					{
						float hit =
							((p.x * pTri->plane.normal.x +
								 p.z * pTri->plane.normal.z + pTri->plane.dis) /
								pTri->plane.normal.y) *
							-1.0f;
						if (p.y < hit)
						{
							y = hit;
							goto done;
						}
					}
				}
			}
		}
	}
	if (bKeepPos)
	{
		if (pIndex != NULL)
		{
			*pIndex = -1;
		}
		return pos;
	}
	y = p.y;
	if (pos.y < m_pHolder->aabb.min.y)
	{
		y = m_pHolder->aabb.min.y;
	}
done:
	return WVector(p.x, y, p.z);
}

WVector CQuadTree::TestCameraCollision(const WVector& pos)
{
	WVector camPos = m_pPolySoup->GetCameraPos(pos, GolfBall().m_pos);
	WVector ground = GetGroundPoint(camPos, true, NULL, true);
	float y = pos.y - SharedCameraHeight();
	if (y <= ground.y)
	{
		y = ground.y;
	}
	return WVector(ground.x, y + SharedCameraHeight(), ground.z);
}

int CQuadTree::GetMeshIndex(const WVector& pos, bool bObject)
{
	int result = -1;
	std::vector<int> candidates;
	float best = 100000.0f;
	GetMeshList(pos);
	for (int i = 0; i < m_meshNum; i++)
	{
		int tri = m_meshList[i];
		WPolySoup::sTriangle* pTri = &m_pHolder->tri[tri];
		if (pTri->bDoubleSide || pTri->plane.normal.y >= 0.0f)
		{
			WVector a(pTri->v[0].x, pos.y, pTri->v[0].z);
			WVector b(pTri->v[1].x, pos.y, pTri->v[1].z);
			WVector c(pTri->v[2].x, pos.y, pTri->v[2].z);
			if (intersection::SameSide(pos, a, b, c) &&
				intersection::SameSide(pos, b, a, c) &&
				intersection::SameSide(pos, c, a, b))
			{
				candidates.push_back(m_meshList[i]);
			}
		}
	}
	int num = (int)candidates.size();
	if (num == 1)
	{
		result = candidates[0];
	}
	else
	{
		for (int i = 0; i < num; i++)
		{
			int tri = candidates[i];
			WPolySoup::sTriangle* pTri = &m_pHolder->tri[tri];
			// 983 (0095e1c0): distancia ate o ponto mais proximo do triangulo,
			// nao a distancia ao plano
			WVector nearPt = closestPointOnTriangle(pos, pTri->v[0],
				pTri->v[1], pTri->v[2]);
			float dx = nearPt.x - pos.x;
			float dy = nearPt.y - pos.y;
			float dz = nearPt.z - pos.z;
			float d = (float)sqrt(dz * dz + dy * dy + dx * dx);
			if (Wabs(d) < Wabs(best))
			{
				result = tri;
				best = d;
			}
		}
	}
	if (bObject)
	{
		for (int i = 0; i < (int)m_water.size(); i++)
		{
			Waabb box = m_water[i].aabb;
			if (box.min.x < pos.x && pos.x < box.max.x && box.min.z < pos.z &&
				pos.z < box.max.z)
			{
				WVector col = ColTest(pos, true, NULL, true);
				if (col.y < box.max.y)
				{
					result = -2;
					break;
				}
			}
		}
	}
	return result;
}

int CQuadTree::GetObjIndex(const WVector& pos, bool bFlag)
{
	int result = -1;
	std::vector<int> candidates;
	float best = 100000.0f;
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return -1;
	}
	GetObjList(pos, pos, 0);
	for (int i = 0; i < m_objNum; i++)
	{
		WPolySoup::sTriangle* pTri = &PolyObjs(m_pPolySoup)[m_objList[i]]->tri[0];
		if (pTri->bDoubleSide || pTri->plane.normal.y >= 0.0f)
		{
			WVector a(pTri->v[0].x, pos.y, pTri->v[0].z);
			WVector b(pTri->v[1].x, pos.y, pTri->v[1].z);
			WVector c(pTri->v[2].x, pos.y, pTri->v[2].z);
			if (intersection::SameSide(pos, a, b, c) &&
				intersection::SameSide(pos, b, a, c) &&
				intersection::SameSide(pos, c, a, b))
			{
				candidates.push_back(m_objList[i]);
			}
		}
	}
	int num = (int)candidates.size();
	if (num == 1)
	{
		result = m_objList[0];
	}
	for (int i = 0; i < num; i++)
	{
		WPolySoup::sTriangle* pTri = &PolyObjs(m_pPolySoup)[candidates[i]]->tri[0];
		float d = pos.x * pTri->plane.normal.x + pTri->plane.normal.y * pos.y +
			pTri->plane.normal.z * pos.z + pTri->plane.dis;
		if (d < 0.0f && pTri->bDoubleSide)
		{
			d = d * -1.0f;
		}
		if (Wabs(d) < Wabs(best))
		{
			result = candidates[i];
			best = d;
		}
	}
	return result;
}

WVector CQuadTree::GetGroundPoint(WVector pos, bool bObject, int* pIndex,
	bool bWater)
{
	GetMeshList(pos);
	int index = -1;
	WVector col = ColTest(pos, bObject, &index, true);
	float x = col.x;
	pos.y = col.y;
	float z = col.z;
	if (bObject && index == -1)
	{
		pos.y = m_pHolder->aabb.min.y;
	}
	if (pIndex != NULL)
	{
		*pIndex = index;
	}
	if (bWater)
	{
		for (int i = 0; i < (int)m_water.size(); i++)
		{
			sWater& water = m_water[i];
			float top = water.aabb.max.y;
			if (water.aabb.min.x < x && x < water.aabb.max.x &&
				water.aabb.min.z < z && z < water.aabb.max.z && pos.y < top)
			{
				pos.y = top;
				if (pIndex != NULL)
				{
					*pIndex = -2;
				}
				break;
			}
		}
	}
	if (pos.y < m_pHolder->aabb.min.y)
	{
		pos.y = m_pHolder->aabb.min.y;
		if (pIndex != NULL)
		{
			int i = *pIndex;
			if (-2 < i)
			{
				i = -1;
			}
			*pIndex = i;
		}
	}
	return WVector(x, pos.y, z);
}

float CQuadTree::GetHighestGround(WVector pos, int* pIndex)
{
	GetMeshList(pos);
	float best = -100000.0f;
	bool bFound = false;
	for (int i = 0; i < m_meshNum; i++)
	{
		int index = m_meshList[i];
		const WPolySoup::sTriangle* tris = &m_pHolder->tri[0];
		if (Abs(tris[index].plane.normal.y) < 0.1f)
			continue;
		if (!tris[index].bDoubleSide && tris[index].plane.normal.y < 0.0f)
			continue;
		WVector a = tris[index].v[0];
		a.y = pos.y;
		WVector b = tris[index].v[1];
		b.y = pos.y;
		WVector c = tris[index].v[2];
		c.y = pos.y;
		if (intersection::SameSide(pos, a, b, c) &&
			intersection::SameSide(pos, b, a, c) &&
			intersection::SameSide(pos, c, a, b))
		{
			const WPlane& plane = m_pHolder->tri[index].plane;
			float y =
				((pos.x * plane.normal.x + pos.z * plane.normal.z + plane.dis) /
					plane.normal.y) *
				-1.0f;
			if (Abs(y - pos.y) < Abs(best - pos.y))
			{
				bFound = true;
				best = y;
				if (pIndex != NULL)
					*pIndex = index;
			}
		}
	}
	if (bFound)
		return best;
	if (pIndex != NULL)
		*pIndex = -1;
	return pos.y;
}

float CQuadTree::GetLowestGround(WVector pos)
{
	GetMeshList(pos);
	float best = 100000.0f;
	bool bFound = false;
	for (int i = 0; i < m_meshNum; i++)
	{
		int index = m_meshList[i];
		const WPolySoup::sTriangle* tris = &m_pHolder->tri[0];
		if (Abs(tris[index].plane.normal.y) < 0.1f)
			continue;
		if (!tris[index].bDoubleSide && tris[index].plane.normal.y < 0.0f)
			continue;
		WVector a = tris[index].v[0];
		a.y = pos.y;
		WVector b = tris[index].v[1];
		b.y = pos.y;
		WVector c = tris[index].v[2];
		c.y = pos.y;
		if (intersection::SameSide(pos, a, b, c) &&
			intersection::SameSide(pos, b, a, c) &&
			intersection::SameSide(pos, c, a, b))
		{
			const WPlane& plane = m_pHolder->tri[index].plane;
			bFound = true;
			float y =
				((pos.x * plane.normal.x + pos.z * plane.normal.z + plane.dis) /
					plane.normal.y) *
				-1.0f;
			if (y < best)
				best = y;
		}
	}
	if (bFound)
		return best;
	return pos.y;
}

void CQuadTree::GetMeshList(WVector pos)
{
	m_meshNum = 0;
	pos.x -= m_quadOrigin.x;
	pos.z -= m_quadOrigin.z;

	int maxRow = m_quadHeight - 1;
	int row = Between(0, (int)(0.05f * pos.z), maxRow);

	int maxCol = m_quadWidth - 1;
	int col = Between(0, (int)(0.05f * pos.x), maxCol);

	for (int i = 0; i < m_pQuad[m_quadWidth * row + col].meshNum; i++)
	{
		int num = m_meshNum;
		int j;
		for (j = 0; j < m_meshNum; j++)
		{
			if (m_pQuad[m_quadWidth * row + col].meshIndex[i] == m_meshList[j])
				break;
		}
		if (j == num)
		{
			m_meshList[num] = m_pQuad[m_quadWidth * row + col].meshIndex[i];
			m_meshNum++;
		}
	}
}

void CQuadTree::GetMeshList(WVector pos, const WVector& dir)
{
	m_meshNum = 0;
	pos.x -= m_quadOrigin.x;
	pos.z -= m_quadOrigin.z;

	int minCol, maxCol, minRow, maxRow;
	int lastCol = m_quadWidth - 1;
	int col = (int)(0.05f * (dir.x > 0.0f ? pos.x : pos.x + dir.x));
	if (col < 0)
		minCol = 0;
	else if (col > lastCol)
		minCol = lastCol;
	else
		minCol = col;

	col = (int)(0.05f * (dir.x >= 0.0f ? pos.x + dir.x : pos.x));
	if (col < 0)
		maxCol = 0;
	else if (col > lastCol)
		maxCol = lastCol;
	else
		maxCol = col;

	int lastRow = m_quadHeight - 1;
	int row = (int)(0.05f * (dir.z > 0.0f ? pos.z : pos.z + dir.z));
	if (row < 0)
		minRow = 0;
	else if (row > lastRow)
		minRow = lastRow;
	else
		minRow = row;

	row = (int)(0.05f * (dir.z >= 0.0f ? pos.z + dir.z : pos.z));
	if (row < 0)
		maxRow = 0;
	else if (row > lastRow)
		maxRow = lastRow;
	else
		maxRow = row;

	int cols = maxCol - minCol + 1;
	int rows = maxRow - minRow + 1;
	int* mask = new int[cols * rows];

	if (Abs(dir.x) < g_EPSILON)
	{
		for (int i = 0; i < rows; i++)
			mask[i] = 1;
	}
	else
	{
		int* edge = new int[cols + 1];
		for (int i = 0; i < cols + 1; i++)
		{
			edge[i] = -(int)(((dir.z / dir.x) *
									 ((float)(minCol + i) * 20.0f - pos.x) +
								 pos.z) *
						  -0.05f) -
				minRow;
		}

		for (int c = 0; c < cols; c++)
		{
			for (int r = 0; r < rows; r++)
			{
				if (r < Min(edge[c], edge[c + 1]) ||
					r > Max(edge[c], edge[c + 1]))
					mask[r * cols + c] = 0;
				else
					mask[r * cols + c] = 1;
			}
		}
		delete[] edge;
	}

	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			if (mask[r * cols + c] == 0)
				continue;

			for (int i = 0;
				i < m_pQuad[m_quadWidth * (minRow + r) + c + minCol].meshNum;
				i++)
			{
				int num = m_meshNum;
				int j;
				for (j = 0; j < m_meshNum; j++)
				{
					if (m_pQuad[m_quadWidth * (minRow + r) + c + minCol]
							.meshIndex[i] == m_meshList[j])
						break;
				}
				if (j == num)
				{
					m_meshList[num] =
						m_pQuad[m_quadWidth * (minRow + r) + c + minCol]
							.meshIndex[i];
					m_meshNum++;
				}
			}
		}
	}
	delete[] mask;
}

void CQuadTree::GetMeshList(Waabb aabb)
{
	m_meshNum = 0;
	aabb.min.x -= m_quadOrigin.x;
	aabb.min.z -= m_quadOrigin.z;
	aabb.max.x -= m_quadOrigin.x;
	aabb.max.z -= m_quadOrigin.z;

	int maxCol = m_quadWidth - 1;
	int col = Between(0, (int)(0.05f * aabb.min.x), maxCol);
	m_minCol = col;

	col = Between(0, (int)(0.05f * aabb.max.x), maxCol);
	m_maxCol = col;

	int maxRow = m_quadHeight - 1;
	int row = Between(0, (int)(0.05f * aabb.min.z), maxRow);
	m_minRow = row;

	row = Between(0, (int)(0.05f * aabb.max.z), maxRow);
	m_maxRow = row;

	for (int r = m_minRow; r <= m_maxRow; r++)
	{
		for (int c = m_minCol; c <= m_maxCol; c++)
		{
			for (int i = 0; i < m_pQuad[m_quadWidth * r + c].meshNum; i++)
			{
				int num = m_meshNum;
				int j;
				for (j = 0; j < m_meshNum; j++)
				{
					if (m_pQuad[m_quadWidth * r + c].meshIndex[i] ==
						m_meshList[j])
						break;
				}
				if (j == num)
				{
					m_meshList[num] = m_pQuad[m_quadWidth * r + c].meshIndex[i];
					m_meshNum++;
				}
			}
		}
	}
}

// 983 0094fbb0: a mascara tira da lista os objetos com noCollision & mask (1 = bola/trace, 2 = decal e IntersectObj,
// 0 = TraceOBB_Object). m_objList tem 32 entradas e m_objNum e byte: sem o filtro, mapa com muitos objetos (Wiz City)
// passava de 32 e escrevia por cima do m_objNum (estado 5 "Unplayable Area" em qualquer lugar).
void CQuadTree::GetObjList(Waabb aabb, unsigned char mask)
{
	m_objNum = 0;
	int maxCol = m_quadWidth - 1;
	int col = (int)(0.05f * (aabb.min.x - m_quadOrigin.x));
	if (col < 0)
		col = 0;
	else if (col > maxCol)
		col = maxCol;
	m_minCol = col;

	col = (int)((aabb.max.x - m_quadOrigin.x) * 0.05f);
	if (col < 0)
		col = 0;
	else if (col > maxCol)
		col = maxCol;
	m_maxCol = col;

	int maxRow = m_quadHeight - 1;
	int row = (int)((aabb.min.z - m_quadOrigin.z) * 0.05f);
	if (row < 0)
		row = 0;
	else if (row > maxRow)
		row = maxRow;
	m_minRow = row;

	int lastRow = (int)((aabb.max.z - m_quadOrigin.z) * 0.05f);
	if (lastRow < 0)
		lastRow = 0;
	else if (lastRow > maxRow)
		lastRow = maxRow;
	m_maxRow = lastRow;

	for (int r = m_minRow; r <= m_maxRow; r++)
	{
		for (int c = m_minCol; c <= m_maxCol; c++)
		{
			WPolySoup::sQuad& quad = m_pQuad[m_quadWidth * r + c];
			if (QuadObjNum(quad) != 0)
			{
				for (int i = 0; i < QuadObjNum(quad); i++)
				{
					int j;
					for (j = 0; j < m_objNum; j++)
					{
						if (QuadObjList(quad)[i] == m_objList[j])
							break;
					}
					if (j == m_objNum)
					{
						short id = QuadObjList(quad)[i];
						sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[id];
						if (pHolder != NULL && (pHolder->noCollision & mask) == 0)
						{
							m_objList[m_objNum] = id;
							m_objNum++;
						}
					}
				}
			}
		}
	}
	for (unsigned int i = 0; i < m_aniObjs.size();)
	{
		CAniObj* pObj = m_aniObjs[i];
		if (pObj != NULL && (pObj->m_type == 1 || pObj->m_type == 3))
		{
			WPolySoup* pSoup = m_pPolySoup;
			// 983: modelo pelo indice (lista cheia) ou pelo id (+0x14c; o 645 lia +0x108), o primeiro que casar
			short k = -1;
			if (PolyObjNum(pSoup) == pSoup->GetRmrModelNum())
			{
				k = (short)pObj->m_id;
			}
			else
			{
				for (int n = 0; n < PolyObjNum(pSoup); n++)
				{
					if ((short)pObj->m_id == PolyObjs(pSoup)[n]->id)
					{
						k = (short)n;
						break;
					}
				}
			}
			if (k >= 0)
			{
				sQuadPuppetHolder* pHolder = PolyObjs(pSoup)[k];
				if (pHolder != NULL && (pHolder->noCollision & mask) == 0)
				{
					m_objList[m_objNum] = k;
					m_objNum++;
				}
			}
			i++;
		}
		else
		{
			i++;
		}
	}
}

// 983 0094fe70: mesma mascara da versao aabb (RayIntersectAll e GetShadowIntensity 00958bb0 passam 0,
// GetRayIntersectionObj 00958f10 passa 2);
// na lista dos aniObjs o modelo vem pelo id (+0x14c), o primeiro que casar (o 645 lia +0x108 cru e somava todos).
void CQuadTree::GetObjList(WVector start, const WVector& ray, unsigned char mask)
{
	m_objNum = 0;
	float fx = start.x - m_quadOrigin.x;
	float fz = start.z - m_quadOrigin.z;
	float v = fx;
	if (ray.x <= 0.0f)
	{
		v = fx + ray.x;
	}
	int maxCol = m_quadWidth - 1;
	int c0 = (int)(0.05f * v);
	int minCol;
	if (c0 < 0)
	{
		minCol = 0;
	}
	else
	{
		minCol = maxCol;
		if (c0 <= maxCol)
		{
			minCol = c0;
		}
	}
	v = fx;
	if (0.0f <= ray.x)
	{
		v = fx + ray.x;
	}
	c0 = (int)(0.05f * v);
	if (c0 < 0)
	{
		maxCol = 0;
	}
	else if (c0 <= maxCol)
	{
		maxCol = c0;
	}
	v = fz;
	if (ray.z <= 0.0f)
	{
		v = fz + ray.z;
	}
	int maxRow = m_quadHeight - 1;
	int r0 = (int)(0.05f * v);
	int minRow;
	if (r0 < 0)
	{
		minRow = 0;
	}
	else
	{
		minRow = maxRow;
		if (r0 <= maxRow)
		{
			minRow = r0;
		}
	}
	v = fz;
	if (0.0f <= ray.z)
	{
		v = fz + ray.z;
	}
	r0 = (int)(0.05f * v);
	if (r0 < 0)
	{
		r0 = 0;
	}
	else if (maxRow < r0)
	{
		r0 = maxRow;
	}
	int cols = (maxCol - minCol) + 1;
	int rows = (r0 - minRow) + 1;
	int* inRay = new int[cols * rows];
	float ax = Wabs(ray.x);
	if (g_EPSILON <= ax)
	{
		int* edge = new int[cols + 1];
		int n = (maxCol - minCol) + 2;
		for (int i = 0; i < n; i++)
		{
			edge[i] =
				-(int)(((ray.z / ray.x) * ((float)(minCol + i) * 20.0f - fx) +
						   fz) *
					-0.05f) -
				minRow;
		}
		if (ray.z / ray.x < 0.0f)
		{
			for (int c = 0; c < cols; c++)
			{
				for (int r = 0; r < rows; r++)
				{
					inRay[r * cols + c] =
						(r < edge[c + 1] || edge[c] < r) ? 0 : 1;
				}
			}
		}
		else
		{
			for (int c = 0; c < cols; c++)
			{
				for (int r = 0; r < rows; r++)
				{
					inRay[r * cols + c] =
						(r < edge[c] || edge[c + 1] < r) ? 0 : 1;
				}
			}
		}
		delete[] edge;
	}
	else
	{
		for (int i = 0; i < rows; i++)
		{
			inRay[i] = 1;
		}
	}
	for (int r = 0; r < rows; r++)
	{
		for (int c = 0; c < cols; c++)
		{
			if (inRay[r * cols + c] != 0)
			{
				WPolySoup::sQuad& quad =
					m_pQuad[m_quadWidth * (minRow + r) + c + minCol];
				if (QuadObjNum(quad) != 0)
				{
					for (int i = 0; i < QuadObjNum(quad); i++)
					{
						unsigned int num = m_objNum;
						unsigned int j = 0;
						if (num != 0)
						{
							do
							{
								if (QuadObjList(quad)[i] == m_objList[j])
									break;
								j++;
							} while ((int)j < m_objNum);
						}
						if (j == num)
						{
							short id = QuadObjList(quad)[i];
							sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[id];
							if (pHolder != NULL && (pHolder->noCollision & mask) == 0)
							{
								m_objList[num] = id;
								m_objNum++;
							}
						}
					}
				}
			}
		}
	}
	delete[] inRay;
	for (unsigned int i = 0; i < m_aniObjs.size();)
	{
		CAniObj* pObj = m_aniObjs[i];
		if (pObj != NULL && (pObj->m_type == 1 || pObj->m_type == 3))
		{
			WPolySoup* pSoup = m_pPolySoup;
			short k = -1;
			if (PolyObjNum(pSoup) == pSoup->GetRmrModelNum())
			{
				k = (short)pObj->m_id;
			}
			else
			{
				for (int n = 0; n < PolyObjNum(pSoup); n++)
				{
					if ((short)pObj->m_id == PolyObjs(pSoup)[n]->id)
					{
						k = (short)n;
						break;
					}
				}
			}
			if (k >= 0)
			{
				sQuadPuppetHolder* pHolder = PolyObjs(pSoup)[k];
				if (pHolder != NULL && (pHolder->noCollision & mask) == 0)
				{
					m_objList[m_objNum] = k;
					m_objNum++;
				}
			}
		}
		i++;
	}
}

float CQuadTree::GetGroundSlope(int index)
{
	if (index < 0)
		return 0.0f;

	const WPolySoup::sTriangle& tri = m_pHolder->tri[index];
	WVector normal = tri.plane.normal;
	if (tri.bDoubleSide && normal.y < 0.0f)
		normal.y = -normal.y;
	return Abs(acosf(Between(-1.0f, normal.y, 1.0f)));
}

WVector CQuadTree::GetMeshNormal(int index)
{
	if (index < 0)
	{
		return GetMeshNormal(GolfBall().m_pos);
	}
	WVector normal;
	const WPlane& plane = m_pHolder->tri[index].plane;
	normal.x = plane.normal.x;
	normal.y = plane.normal.y;
	normal.z = plane.normal.z;
	if (m_pHolder->tri[index].bDoubleSide && normal.y < 0.0f)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;
	}
	return normal;
}

WVector CQuadTree::GetMeshNormal(const WVector& pos)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return WVector::ZERO;
	}
	int index = GetMeshIndex(pos, false);
	if (index < 0)
	{
		return WVector::ZERO;
	}
	WPolySoup::sTriangle* tri = &m_pHolder->tri[index];
	WVector normal = tri->plane.normal;
	if (tri->bDoubleSide && normal.y < 0.0f)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;
	}
	return normal;
}

WVector CQuadTree::GetMeshObjcet(const WVector& pos)
{
	int index = GetObjIndex(pos, false);
	if (index < 0)
	{
		return WVector::ZERO;
	}
	sQuadPuppetHolder* pObj = PolyObjs(m_pPolySoup)[m_objList[index]];
	WVector normal;
	normal.x = pObj->pos.x;
	normal.y = pObj->pos.y;
	normal.z = pObj->pos.z;
	if (PolyObjs(m_pPolySoup)[m_objList[index]]->tri[0].bDoubleSide &&
		normal.y < 0.0f)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;
	}
	return normal;
}

bool CQuadTree::PointInTriangle(const WVector& p, const WVector& a,
	const WVector& b, const WVector& c) const
{
	if (intersection::SameSide(p, a, b, c) &&
		intersection::SameSide(p, b, a, c) &&
		intersection::SameSide(p, c, a, b))
		return true;
	return false;
}

bool CQuadTree::SameSide(const WVector& p1, const WVector& p2, const WVector& a,
	const WVector& b) const
{
	return intersection::SameSide(p1, p2, a, b);
}

float CQuadTree::IntersectPlane(const WVector& pos, const WVector& vel,
	const WPlane& plane, bool bBothSides)
{
	return intersection::IntersectPlane(pos, vel, plane, bBothSides);
}

float CQuadTree::IntersectObj(const WVector& pos, const WVector& dir,
	WPlane* pPlane)
{
	float best = 1.0f;
	short hitObj = -1;
	int hitTri = -1;
	float radius = GolfBall().m_radius;
	float az = Abs(dir.z);
	float ay = Abs(dir.y);
	float ax = Abs(dir.x);
	float ex = WVector::ONE.x * radius + ax;
	float ey = WVector::ONE.y * radius + ay;
	float ez = radius * WVector::ONE.z + az;
	Waabb box;
	box.min.x = pos.x - ex;
	box.min.y = pos.y - ey;
	box.min.z = pos.z - ez;
	box.max.x = ex + pos.x;
	box.max.y = ey + pos.y;
	box.max.z = ez + pos.z;
	GetObjList(box, 2);
	if (m_objNum != 0)
	{
		for (unsigned char i = 0; i < m_objNum; i++)
		{
			sQuadPuppetHolder* pObj = PolyObjs(m_pPolySoup)[m_objList[i]];
			// 983 00960ca0: se o pet tem a caixa "anicollision", ela (movida pelo pivo) vale no lugar do aabb do objeto
			Waabb ob = pObj->aabb;
			if (w_bound_box* ani = pObj->pet->FindBoundBox("anicollision"))
			{
				const Waabb& a = ani->info->aabb;
				ob.min.x = a.min.x + pObj->mat.pivot.x;
				ob.min.y = a.min.y + pObj->mat.pivot.y;
				ob.min.z = a.min.z + pObj->mat.pivot.z;
				ob.max.x = a.max.x + pObj->mat.pivot.x;
				ob.max.y = a.max.y + pObj->mat.pivot.y;
				ob.max.z = a.max.z + pObj->mat.pivot.z;
			}
			if (ob.min.x - ex <= pos.x &&
				pos.x <= ex + ob.max.x &&
				ob.min.y - ey <= pos.y &&
				pos.y <= ey + ob.max.y &&
				ob.min.z - ez <= pos.z &&
				pos.z <= ez + ob.max.z)
			{
				char type = pObj->type;
				if (type == 2 || type == 1 || type == 3)
				{
					MakeCollisionData(m_objList[i]);
				}
				if (ObjColTest(dir, m_objList[i], pPlane, best, hitTri))
				{
					hitObj = m_objList[i];
				}
			}
		}
		if (hitObj >= 0)
		{
			sQuadPuppetHolder* pObj = PolyObjs(m_pPolySoup)[hitObj];
			if ((GolfBall().m_special & 0x10) != 0)
			{
				HolderHitCount(pObj) = HolderHitCount(pObj) + 1;
			}
			if ((GolfBall().m_special & 0x40) != 0)
			{
				if (GolfBall().m_topFrame >= 0 &&
					GolfBall().m_topFrame < GolfBall().m_renderNum)
				{
					HolderHitCount(pObj) = HolderHitCount(pObj) + 1;
				}
			}
			m_colData.objIndex = hitObj;
			m_colData.triIndex = hitTri;
			m_colData.groundType = pObj->type;
			m_colData.frame = GolfBall().m_renderNum;
			WPolySoup* pSoup = m_pPolySoup;
			int texHandle = pObj->tri[hitTri].texHandle & 0x7ff;
			PolyTexMap(pSoup).find((int)pPlane);
			const char* pName =
				PolyTexNames(pSoup) + PolyTexMap(pSoup)[texHandle] * 0x28;
			char* pDst = g_colTex[g_colNum];
			char c;
			do
			{
				c = *pName++;
				*pDst++ = c;
			} while (c != '\0');
			g_colNum++;
			if (g_colNum > 0xf)
			{
				g_colNum = 0xf;
			}
			GolfBall().m_attrIndex = pObj->tri[hitTri].propIndex;
		}
	}
	return best;
}

int CQuadTree::GetTextureHandle(const WVector& pos)
{
	int index = GetMeshIndex(pos, false);
	if (index < 0)
	{
		return 0;
	}
	return m_pHolder->tri[index].texHandle;
}

const char* CQuadTree::GetTextureName(const WVector& pos)
{
	int index = GetMeshIndex(pos, false);
	if (index < 0)
	{
		return NULL;
	}
	int texHandle = m_pHolder->tri[index].texHandle & 0x7ff;
	PolyTexMap(m_pPolySoup).find((int)this);
	return PolyTexNames(m_pPolySoup) +
		PolyTexMap(m_pPolySoup)[texHandle] * 0x28;
}

void CQuadTree::MakeCollisionData(short objIndex)
{
	sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[objIndex];
	WPuppet* pPuppet = pHolder->pet;
	if (pPuppet != NULL)
	{
		int triNum = 0;
		for (w_bound_box* box = pPuppet->m_bbList.Start(); box != NULL;
			box = pPuppet->m_bbList.Next())
		{
			WBone* bone = box->bone;
			if (_strcmpi(box->info->name, bone->GetBoneName()) == 0)
			{
				for (w_mesh* mesh = bone->m_mesh; mesh != NULL;
					mesh = mesh->next)
				{
					for (int i = 0; i < mesh->indexNum; i += 3)
					{
						WPolySoup::sTriangle* tri = &pHolder->tri[triNum];
						tri->v[0] = mesh->vecList[mesh->indexList[i]] *
							bone->GetMatrix();
						tri->v[1] = mesh->vecList[mesh->indexList[i + 1]] *
							bone->GetMatrix();
						tri->v[2] = mesh->vecList[mesh->indexList[i + 2]] *
							bone->GetMatrix();
						WVector normal;
						normal.x = (tri->v[2].y - tri->v[1].y) *
								(tri->v[0].z - tri->v[1].z) -
							(tri->v[2].z - tri->v[1].z) *
								(tri->v[0].y - tri->v[1].y);
						normal.y = (tri->v[2].z - tri->v[1].z) *
								(tri->v[0].x - tri->v[1].x) -
							(tri->v[0].z - tri->v[1].z) *
								(tri->v[2].x - tri->v[1].x);
						normal.z = (tri->v[0].y - tri->v[1].y) *
								(tri->v[2].x - tri->v[1].x) -
							(tri->v[2].y - tri->v[1].y) *
								(tri->v[0].x - tri->v[1].x);
						normal.Normalize();
						tri->plane.normal = normal;
						triNum++;
						pHolder->tri[triNum - 1].plane.d =
							pHolder->tri[triNum - 1].plane.z * -1.0f *
								tri->v[0].z +
							pHolder->tri[triNum - 1].plane.y * -1.0f *
								tri->v[0].y +
							pHolder->tri[triNum - 1].plane.x * -1.0f *
								tri->v[0].x;
					}
				}
			}
		}
	}
}

void CQuadTree::CalcObjMeshData(short objIndex, int triIndex)
{
	sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[objIndex];
	WPuppet* pPuppet = pHolder->pet;
	if (pPuppet != NULL)
	{
		for (w_bound_box* box = pPuppet->m_bbList.Start(); box != NULL;
			box = pPuppet->m_bbList.Next())
		{
			WBone* bone = box->bone;
			if (_strcmpi(box->info->name, bone->GetBoneName()) == 0)
			{
				int triNum = 0;
				for (w_mesh* mesh = bone->m_mesh; mesh != NULL;
					mesh = mesh->next)
				{
					for (int i = 0; i < mesh->indexNum; i += 3)
					{
						if (triNum == triIndex)
						{
							WPolySoup::sTriangle* tri = &pHolder->tri[triNum];
							tri->v[0] = mesh->vecList[mesh->indexList[i]] *
								bone->GetMatrix();
							tri->v[1] = mesh->vecList[mesh->indexList[i + 1]] *
								bone->GetMatrix();
							tri->v[2] = mesh->vecList[mesh->indexList[i + 2]] *
								bone->GetMatrix();
							return;
						}
						triNum++;
					}
				}
			}
		}
	}
}

void CQuadTree::PreProcess(float dt, bool bOneFrame)
{
	if (bOneFrame)
	{
		m_frame++;
	}
	else
	{
		m_frameTime += dt;
		if (m_frameTime < 0.02f)
		{
			return;
		}
		if (Wabs(dt) < g_EPSILON)
		{
			return;
		}
		unsigned long ticks = (unsigned long)(m_frameTime * 50.0f);
		m_frame += ticks;
		m_frameTime -= (float)ticks * 0.02f;
	}
	// 983 (00963e16): nada de vento especial com o modo so nos bits 0x2000/0x20000 ou zerado
	if (Wind().HasSpecialWind() && (GOLFDOC()->m_gameMode & 0xfffddfff) != 0)
	{
		unsigned long phase = (m_frame / 50) % 170;
		if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/))
		{
			if (CSharedDoc::Instance()->GetPlayingMode() != 2)
			{
				Wind().SetSpecialWind(phase >= 80, bOneFrame); // 983 (00963e73): repassa bOneFrame
			}
		}
		else
		{
			Wind().SetSpecialWind(phase >= 80, bOneFrame); // 983 (00963e73): repassa bOneFrame
		}
		if (!bOneFrame)
		{
			if (Wind().IsActiveSpecialWind())
			{
				if (Wind().GetSpWindStartFrame() == 0)
				{
					Wind().SetSpWindStartFrame(m_frame);
				}
			}
			else if (Wind().GetSpWindStartFrame() != 0)
			{
				Wind().SetSpWindStartFrame(0);
			}
			Wind().GenerateFx(phase >= 60);
		}
	}
	for (unsigned int i = 0; i < m_aniObjs.size(); i++)
	{
		CAniObj* pAni = m_aniObjs[i];
		if (pAni == NULL)
		{
			continue;
		}
		WPolySoup::sModel* pModel;
		if (PolyObjNum(m_pPolySoup) == PolyStructuredObjNum(m_pPolySoup))
		{
			pModel = PolyObjs(m_pPolySoup)[pAni->m_id];
		}
		else
		{
			pModel = NULL;
			for (int j = 0; j < PolyObjNum(m_pPolySoup); j++)
			{
				if (PolyObjs(m_pPolySoup)[j]->id == pAni->m_id)
				{
					pModel = PolyObjs(m_pPolySoup)[j];
					break;
				}
			}
		}
		if (pModel == NULL || pModel->pet == NULL)
		{
			continue;
		}
		if (!bOneFrame && pAni->m_sound[0] != 0)
		{
			char sound[8];
			sprintf(sound, "sound%d", i);
			if (g_audio->IsPlaying(sound))
			{
				float dist = (pModel->mat.pivot - g_camera.pivot).Magnitude();
				float volume = Max(1.0f - dist / (pAni->m_value * 3.2f), 0.0f);
				g_audio->SetSFXVolumeWhilePlaying(sound, volume);
			}
			else
			{
				pAni->m_sound[0] = 0;
			}
		}
		if (pModel->type == 4 && bOneFrame)
		{
			continue;
		}
		pAni->m_time = (float)(pAni->m_rand + m_frame) * 0.02f;
		float aniTime = pAni->m_time;
		switch (pModel->type)
		{
		case 2:
		{
			CRotObj* pRot = static_cast<CRotObj*>(pAni);
			// 983 (00964090): no lobby (frameset atual e CLobbyTask) o vento nao conta
			if (IS_KINDOF(CLobbyTask,
					AfxGetTask()))
			{
				pRot->m_angle = aniTime * g_PI * pModel->windFactor * pRot->m_invSize;
			}
			else if (pModel->windFactor > 0.0f)
			{
				pRot->m_angle = Wind().GetPower983() * pRot->m_invSize *
					pModel->windFactor * pAni->m_time * g_PI;
			}
			else
			{
				pRot->m_angle = aniTime * pModel->windFactor * -3.1415927f;
			}
			pModel->pet->SetMatrix(RotMat(pRot->m_angle, 2) * pModel->mat);
			pModel->pet->Transform(NULL, 1.0f, 0);
			continue;
		}
		case 1:
		{
			CMoveObj* pMove = static_cast<CMoveObj*>(pAni);
			float local = pAni->m_time -
				(float)floor(
					aniTime / (pMove->m_standTime + pMove->m_moveTime)) *
					(pMove->m_standTime + pMove->m_moveTime);
			int odd = (int)floor(pAni->m_time /
						  (pMove->m_standTime + pMove->m_moveTime)) %
				2;
			const WVector* pPos = &pMove->m_startPos;
			if (local > pMove->m_moveTime)
			{
				pModel->mat.pivot = pPos[1 - odd];
			}
			else
			{
				WVector delta = pPos[1 - odd] - pPos[odd];
				pModel->mat.pivot =
					delta * (local / pMove->m_moveTime) + pPos[odd];
			}
			break;
		}
		}
		// 983 (009641e4): a matriz entra antes do ScriptProcess
		pModel->pet->SetMatrix(pModel->mat);
		if (pModel->pet->GetAnimationLength() > 0.0f)
		{
			float rem = Max(pAni->m_time -
					(float)floor(
						pAni->m_time / pModel->pet->GetAnimationLength()) *
						pModel->pet->GetAnimationLength(),
				0.0f);
			if (!bOneFrame)
			{
				ScriptProcess(i, rem - dt, rem);
			}
			pModel->pet->Animate983(NULL, NULL, rem, 0, NULL, NULL);
		}
		pModel->pet->Transform(NULL, 1.0f, 0);
	}
}

void CQuadTree::ScriptProcess(int index, float from, float to)
{
	CAniObj* pAni = m_aniObjs[index];
	WPolySoup::sModel* pModel = PolyObjs(m_pPolySoup)[pAni->m_id];
	WPuppet* pPuppet = pModel->pet;
	if (from == to)
	{
		return;
	}
	from = (0.0f < from) ? from : 0.0f;
	from *= 30.0f;
	to *= 30.0f;
	float tolerance;
	if (pPuppet->GetFrameCount())
	{
		tolerance = ((float)pPuppet->m_frame[1].num * (1.0f / 30.0f) -
							(float)pPuppet->m_frame[0].num * (1.0f / 30.0f) <
						0.0f)
			? 0.0f
			: (float)pPuppet->m_frame[1].num * (1.0f / 30.0f) -
				(float)pPuppet->m_frame[0].num * (1.0f / 30.0f);
	}
	else
	{
		tolerance = 0.3f;
	}
	int first = Ceil(from, tolerance);
	int last;
	if ((float)floor(to) == to)
	{
		last = (int)to - 1;
	}
	else
	{
		last = (int)floor(to);
	}
	if (first > last)
	{
		return;
	}
	int lo;
	int hi;
	if (pPuppet->CheckFrameData(first, last, lo, hi))
	{
		for (int f = lo; f <= hi; f++)
		{
			ParsePuppetScript(index, pPuppet->m_frame[f].data);
		}
	}
}

const char* GetStrConst(const char* str, char* out);

void CQuadTree::ParsePuppetScript(int index, const char* script)
{
	static char* cmdList[4] = { "*fx(", "*snd(", "*hide", "*show" };
	static char cmd[0x100];

	char key[16];
	char fx[64];
	char name[64];
	char bone[64];
	char suffix[0x100];
	char arg[0x80];
	WPuppet* pPuppet = PolyObjs(m_pPolySoup)[m_aniObjs[index]->m_id]->pet;

	while (script != NULL)
	{
		const char* p = strchr(script, '*');
		if (p == NULL)
		{
			break;
		}
		int count = 0;
		if (*p == '\0')
		{
			break;
		}
		char c;
		do
		{
			c = p[++count];
			if (c == ' ')
			{
				break;
			}
			if (c == '(')
			{
				count++;
				break;
			}
		} while (c != '\0');
		if (count <= 1)
		{
			break;
		}
		strncpy(cmd, p, count);
		cmd[count] = 0;
		script = p + count;
		for (unsigned int n = 0; n < 4; n++)
		{
			if (strcmpi(cmd, cmdList[n]) != 0)
			{
				continue;
			}
			if (n == 0)
			{
				name[0] = '\0';
				script = GetStrConst(script, name);
				if (strlen(name) != 0 && strlen(name) < 0x40)
				{
					if (name[0] == '@')
					{
						script = GetStrConst(script, fx);
						if (strlen(fx) == 0 || strlen(fx) >= 0x40)
						{
							continue;
						}
					}
					else
					{
						strcpy(fx, name);
					}
					if (pPuppet != NULL)
					{
						WVector pivot = pPuppet->GetPivot(NULL);
						WMatrix mat = pPuppet->GetMatrix();
						unsigned char ground =
							GetGroundType(pivot * mat, false, NULL);
						if ((ground == 10 || ground == 14) &&
							strstr(fx, "dust") != NULL)
						{
							suffix[0] = '\0';
							memset(suffix + 1, 0, sizeof(suffix) - 1);
							WSliceStr slice(fx);
							slice.Get("dust", "%s", suffix);
							std::string strSuffix(suffix);
							std::string strFx(fx);
							strFx =
								strFx.substr(0,
									strFx.length() - 4 - strSuffix.length()) +
								"water" + strSuffix;
							strcpy(fx, strFx.c_str());
						}
					}
					Attacher attacher;
					unsigned int j = 0;
					do
					{
						script = GetStrConst(script, bone);
						if (strlen(bone) == 0)
						{
							break;
						}
						if (strlen(bone) >= 0x40)
						{
							break;
						}
						WBone* pBone = pPuppet->FindBone(bone);
						if (pBone != NULL)
						{
							attacher.mat[j] = &pBone->m_matrix;
						}
						else
						{
							w_bound_box* pBox = pPuppet->m_bbList.Find(bone);
							if (pBox != NULL)
							{
								if (pBox->bone != NULL)
								{
									attacher.mat[j] = &pBox->bone->m_matrix;
								}
								attacher.pos[j] = &pBox->info->spherePivot;
							}
						}
						j++;
					} while (j < 4);
					if (0 < (int)j)
					{
						WSingleton<CFx>::Instance()->Attach(attacher, fx, true,
							NULL);
					}
				}
			}
			else if (n == 1)
			{
				sprintf(key, "sound%d", index);
				script = GetStrConst(script, arg);
				int loop = 0;
				sscanf(arg, "%s %d", m_aniObjs[index]->m_sound, &loop);
				if (g_audio->IsPlaying(key))
				{
					g_audio->StopSfx(key);
				}
				WVector diff =
					PolyObjs(m_pPolySoup)[m_aniObjs[index]->m_id]->mat.pivot -
					g_camera.pivot;
				g_audio->PlaySfx(m_aniObjs[index]->m_sound, NULL, loop != 0,
					key, NULL, 0.5f, 200.0f);
				float volume = Max(1.0f -
						diff.Magnitude() / (m_aniObjs[index]->m_value * 3.2f),
					0.0f);
				g_audio->SetSFXVolumeWhilePlaying(key, volume);
			}
		}
	}
}

void CQuadTree::BallProcess(float dt, bool bCupCapture)
{
	float holeCupRadius = HOLECUP_RADIUS983();
	CGolfBall& ball = GolfBall();
	m_bSpecialSkipHit = false;
	if (ball.m_state != CGolfBall::BALL_SPIN)
	{
		const CGolfBall::sExpected* pLast =
			ball.PreData(Max(ball.m_renderNum - 1, 0));
		const CGolfBall::sExpected* pPrev =
			ball.PreData(Max(ball.m_renderNum - 2, 0));
		WVector delta = pPrev->pos - pLast->pos;
		if (!WisEqual(delta, WVector::ZERO, 0.00001f))
		{
			WVector hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
			delta = ball.m_pos - hole;
			if (sqrtf(delta.x * delta.x + delta.z * delta.z) < holeCupRadius)
			{
				ball.m_stayCount++;
				if (ball.m_stayCount == 100)
				{
					ball.m_pos = GetHoleCupPos();
					ball.m_pos.y = m_ballFloorY + ball.m_radius;
					HoleIn();
					return;
				}
			}
			else
			{
				ball.m_stayCount = 0;
			}
		}
		else
		{
			ball.m_stayCount++;
		}
	}
	ball.m_groundType = GetGroundType(ball.m_pos, true, NULL);
	WVector hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
	WVector toHole(ball.m_pos.x - hole.x, 0.0f, ball.m_pos.z - hole.z);
	if (toHole.Magnitude() < holeCupRadius &&
		ball.m_pos.y < m_ballFloorY + ball.m_radius)
	{
		ball.m_pos.y = m_ballFloorY + ball.m_radius;
		HoleIn();
		return;
	}
	if (ball.m_pos.y < m_pHolder->aabb.min.y)
	{
		ball.m_vel.z = 0.0f;
		ball.m_vel.y = 0.0f;
		ball.m_vel.x = 0.0f;
		ball.m_bOB = true;
		ball.m_state = CGolfBall::BALL_STOP;
		return;
	}
	if (ball.m_renderNum < 100 && m_bCollideBallAndCaddie == 0)
	{
		WVector start(ball.m_pos.x,
			(m_caddieBox.max.y + m_caddieBox.min.y) * 0.5f, ball.m_pos.z);
		WVector ray(dt * ball.m_vel.x, 0.0f, dt * ball.m_vel.z);
		if (RayIntersectAABB(m_caddieBox, start, ray, NULL, 0.0f) < 1.0f)
		{
			if (ball.m_pos.y < m_caddieBox.min.y)
			{
				m_bCollideBallAndCaddie = 2;
			}
			else if (ball.m_pos.y > m_caddieBox.max.y)
			{
				m_bCollideBallAndCaddie = 1;
			}
			else
			{
				m_bCollideBallAndCaddie = 3;
			}
		}
	}
	WVector move = ball.m_vel * dt;
	float cupDepth = GolfBall().m_radius * 6.0f;
	hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
	WVector cupBottom(hole.x, hole.y - cupDepth, hole.z);
	m_holeDist = DistanceBetweenRayNSegment(cupBottom, WVector::UNIT_POS_Y,
		ball.m_pos, move);
	// 983 00966b70 param_3: so Predict passa (m_optionFlag & 1)
	if (bCupCapture && ball.m_cupFrame < 0 && ball.m_rollFrame < 0 &&
		m_holeDist.z < holeCupRadius - ball.m_radius &&
		((ball.m_special & 0x20) == 0 || ball.m_vel.y < 0.0f))
	{
		ball.m_cupFrame = ball.m_renderNum;
		ball.m_pos += ball.m_vel * m_holeDist.y * dt;
		WVector toCup;
		toCup = GetHoleCupPos() - ball.m_pos;
		toCup.y = 0.0f;
		float factor =
			toCup.Magnitude() * 0.4f / (holeCupRadius - ball.m_radius) + 0.1f;
		ball.m_vel = WVector(factor * ball.m_vel.x, Abs(ball.m_vel.y) * -2.0f,
			factor * ball.m_vel.z);
		return;
	}
	hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
	WVector next = ball.m_vel * dt + ball.m_pos;
	WVector diff = next - hole;
	int index;
	if (ball.m_radius + holeCupRadius <= m_holeDist.z)
	{
		m_cupRimFrames = 0;
		if (ball.m_state == CGolfBall::BALL_HOLE)
		{
			WVector pos = next;
			GetMeshList(pos);
			WVector ground = ColTest(pos, false, &index, true);
			if (ground.y < m_pHolder->aabb.min.y)
			{
				ground.y = m_pHolder->aabb.min.y;
			}
			if (next.y <= ground.y + ball.m_radius + 0.01f)
			{
				pos = ball.m_pos;
				GetMeshList(pos);
				ground = ColTest(pos, false, &index, true);
				if (ground.y < m_pHolder->aabb.min.y)
				{
					ground.y = m_pHolder->aabb.min.y;
				}
				ball.m_state = CGolfBall::BALL_ROLL;
				ball.m_pos.y = ground.y + ball.m_radius;
			}
			else
			{
				ball.m_state = CGolfBall::BALL_BOUNCE;
			}
			ball.m_holeOutFrame = ball.m_renderNum;
		}
	}
	else
	{
		if (ball.m_state == CGolfBall::BALL_ROLL &&
			ball.m_vel.Magnitude() > 12.0f)
		{
			diff.y = 0.0f;
			diff = diff * -1.0f;
			if (Abs(CalcDeltaAngle(diff, ball.m_vel)) < 1.0471976f)
			{
				ball.m_spin = 0.0f;
				ball.m_curve = 0.0f;
				ball.m_vel.y = ball.m_vel.Magnitude() * 0.15f;
				ball.m_pos += ball.m_vel * dt;
				ball.m_state = CGolfBall::BALL_BOUNCE;
				ball.m_meshIndex = -1;
				CGolfBall::sExpected* pExpected =
					ball.PreDataToWrite(ball.m_renderNum);
				pExpected->effect = 2;
				pExpected->event = 3;
				pExpected->normal = WVector::UNIT_POS_Y;
				strcpy(pExpected->sound, "\xc3\xe6\xb5\xb9_wood");
				ball.m_holeOutFrame = ball.m_renderNum;
				return;
			}
		}
		WVector pos = next;
		GetMeshList(pos);
		WVector ground = ColTest(pos, false, &index, true);
		if (ground.y < m_pHolder->aabb.min.y)
		{
			ground.y = m_pHolder->aabb.min.y;
		}
		if (next.y <= ground.y + ball.m_radius + 0.01f)
		{
			if (m_holeDist.z < holeCupRadius)
			{
				ball.m_state = CGolfBall::BALL_HOLE;
				if (ball.m_holeFrame < 0)
				{
					ball.m_holeFrame = ball.m_renderNum;
				}
			}
		}
		else
		{
			ball.m_meshIndex = -1;
			ball.m_state = CGolfBall::BALL_BOUNCE;
		}
	}
	switch (ball.m_state)
	{
	case CGolfBall::BALL_BOUNCE:
		BounceProcess(dt);
		break;
	case CGolfBall::BALL_SPIN:
		SpinProcess(dt);
		break;
	case CGolfBall::BALL_ROLL:
		RollProcess(dt);
		break;
	case CGolfBall::BALL_HOLE:
		HoleProcess(dt);
		break;
	}
}

bool CQuadTree::IsValid(const WVector& pos)
{
	if (0)
		((CQuadTree*)0)
			->CQuadTree::IntersectPlane(*(const WVector*)0, *(const WVector*)0,
				*(const WPlane*)0, *(bool*)0);
	return true;
}

WVector CQuadTree::ApplyForce()
{
	WVector force(0.0f, 0.0f, 0.0f);
	if (Wabs(GolfBall().m_curve) >= g_EPSILON && GolfBall().m_cobraFrame < 0)
	{
		WVector side(GolfBall().m_vel.z * -1.0f, 0.0f, GolfBall().m_vel.x);
		force = side.Normalize() *
			(GolfClub().GetCurve() * GolfBall().m_curve * 0.75f);
	}
	else if (Wabs(GolfBall().m_curve) >= g_EPSILON &&
		GolfBall().m_spikeFrame < 0)
	{
		WVector side(GolfBall().m_vel.z * -1.0f, 0.0f, GolfBall().m_vel.x);
		force = side.Normalize() *
			(GolfClub().GetCurve() * GolfBall().m_curve * 0.75f);
	}
	if (((GolfBall().m_special & 0x20) == 0 || GolfBall().m_cobraFrame >= 0) &&
		((GolfBall().m_special & 0x40) == 0 || GolfBall().m_spikeFrame >= 0))
	{
		if (m_bApplyWind)
		{
			if ((GolfBall().m_special & 0x40) != 0)
			{
				force += Wind().GetWind(GolfBall().m_pos) * 0.01f;
			}
			else
			{
				force += Wind().GetWind(GolfBall().m_pos) * 0.02f;
			}
		}
		force.y -= m_gravity * GolfBall().GetMass() * 34.295296f;
		if (Wabs(GolfBall().m_spin) >= g_EPSILON)
		{
			force.y += GolfClub().GetSpin() * GolfBall().m_spin * 3.0f;
		}
		force -= GolfBall().m_vel * (GolfBall().m_vel.Magnitude() * 0.00008f);
	}
	return force;
}

void CQuadTree::SetPenaltyPos(const Waabb& water)
{
	if (GolfBall().m_water == CGolfBall::WATER_SEA)
	{
		m_penaltyPos = BallFirstData(GolfBall())->pos;
		return;
	}
	const CGolfBall::sExpected* pFirst = BallFirstData(GolfBall());
	WVector dir(pFirst->pos.x - GolfBall().m_pos.x, 0.0f,
		pFirst->pos.z - GolfBall().m_pos.z);
	dir.Normalize();
	dir *= 16.0f;
	int found = 0;
	WVector candidate[2];
	int index;
	for (int side = 0; side < 2; side++)
	{
		WVector& cur = candidate[found];
		cur = GolfBall().m_pos;
		bool bFound = false;
		for (;;)
		{
			cur += dir;
			WVector pos = cur;
			GetMeshList(pos);
			WVector ground = ColTest(pos, false, &index, true);
			if (ground.y < m_pHolder->aabb.min.y)
			{
				ground.y = m_pHolder->aabb.min.y;
			}
			if (GetMeshIndex(ground, false) == -1)
			{
				break;
			}
			ColTest(ground, true, NULL, true);
			Waabb waterBox;
			// 983 0095e710: nao usa a agua do argumento, so IsFallenIntoWater
			if (IsFallenIntoWater(ground, &waterBox, true))
			{
				continue;
			}
			if (m_pPolySoup->IsOutOfBound(ground))
			{
				continue;
			}
			float limit = (float)cos(20 * g_DEGTORAD);
			if (GetMeshNormal(ground).y <= limit)
			{
				continue;
			}
			int i = 0;
			float angle = 0.0f;
			do
			{
				WVector around = dir * RotMat(angle, 1) + cur;
				int aroundIndex;
				GetMeshList(around);
				ground = ColTest(around, false, &aroundIndex, true);
				if (ground.y < m_pHolder->aabb.min.y)
				{
					ground.y = m_pHolder->aabb.min.y;
				}
				if (GetMeshNormal(ground).y < limit)
				{
					break;
				}
				angle += 20 * g_DEGTORAD;
				i++;
			} while (i < 18);
			if (OverlappedWithObject(ground, 1.6f) || i != 18)
			{
				continue;
			}
			bFound = true;
			break;
		}
		if (bFound)
		{
			found++;
		}
		dir *= -1.0f;
	}
	switch (found)
	{
	case 0:
		m_penaltyPos = BallFirstData(GolfBall())->pos;
		return;
	case 1:
		m_penaltyPos = candidate[0];
		break;
	case 2:
	{
		WVector hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
		WVector toSecond = hole - candidate[1];
		if ((*reinterpret_cast<const WVector*>(SharedCameraPos()) -
				candidate[0])
				.Magnitude() > toSecond.Magnitude())
		{
			m_penaltyPos = candidate[0];
		}
		else
		{
			m_penaltyPos = candidate[1];
		}
		break;
	}
	default:
		m_penaltyPos = BallFirstData(GolfBall())->pos;
		return;
	}
	WVector pos = m_penaltyPos;
	GetMeshList(pos);
	WVector ground = ColTest(pos, false, &index, true);
	if (ground.y < m_pHolder->aabb.min.y)
	{
		ground.y = m_pHolder->aabb.min.y;
	}
	m_penaltyPos = ground;
	m_penaltyPos.y += GolfBall().m_radius;
}

void CQuadTree::BounceProcess(float dt)
{
	bool bPass = false;
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(ball.m_renderNum);
	if ((GolfBall().m_special & 0x40) != 0 && GolfBall().m_topFrame >= 0 &&
		GolfBall().m_renderNum < GolfBall().m_topFrame + 0x3c)
	{
		return;
	}
	ball.m_vel += ApplyForce() * (1.0f / ball.m_mass) * dt;
	if (ball.m_topFrame == -1)
	{
		ball.m_vel += m_vec21d8_983 * (1.0f / ball.m_mass) * dt;
	}
	if (ball.m_colTriIndex != -1)
	{
		WVector normal = PolyObjs(m_pPolySoup)[ball.m_colObjIndex]
							 ->tri[ball.m_colTriIndex]
							 .plane.normal;
		ball.m_vel -= normal * (normal * ball.m_vel);
		if (ball.m_vel.y > 0.0f)
		{
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			ball.m_state = CGolfBall::BALL_STOP;
		}
	}
	ball.m_curveRot += dt * ball.m_curve * 12.566371f;
	if ((GolfBall().m_special & 3) != 0)
	{
		ball.m_spinRot += dt * ball.m_initSpin * 62.831856f;
	}
	else
	{
		ball.m_spinRot += dt * ball.m_spin * 25.132742f;
	}
	if (ball.m_meshIndex >= 0 &&
		ball.PreDataToWrite(ball.m_renderNum - 1)->normal * ball.m_vel < 0.0f)
	{
		float d =
			ball.m_vel * ball.PreDataToWrite(ball.m_renderNum - 1)->normal;
		ball.m_vel -= ball.PreDataToWrite(ball.m_renderNum - 1)->normal * d;
		int index;
		WVector pos = ball.m_pos;
		CQuadTree& pvs = GetPVS();
		pvs.GetMeshList(pos);
		WVector ground = pvs.ColTest(pos, false, &index, true);
		if (ground.y < pvs.m_pHolder->aabb.min.y)
		{
			ground.y = pvs.m_pHolder->aabb.min.y;
		}
		ball.m_pos = ground;
		ball.m_pos.y += ball.m_radius;
		if ((ball.m_special & 2) != 0)
		{
			MakeAfterBurner();
		}
		else
		{
			ball.m_rollFrame = ball.m_renderNum;
			ball.m_state = CGolfBall::BALL_ROLL;
		}
		const char* sound =
			m_pPolySoup->GetProperty(m_pHolder->tri[ball.m_meshIndex].propIndex)
				.rollSound;
		float speed = (ball.m_vel + ball.m_afterBurner).Magnitude();
		if (sound[0] != '\0' && speed > 20.0f)
		{
			strcpy(pExpected->sound, sound);
		}
		if (GetGroundType(ball.m_pos, true, NULL) == 2 && speed > 20.0f)
		{
			ball.m_greenFrame = ball.m_renderNum;
		}
		if (ball.m_state == CGolfBall::BALL_SPIN)
		{
			SpinProcess(dt);
			return;
		}
		if (ball.m_state == CGolfBall::BALL_ROLL)
		{
			RollProcess(dt);
		}
		return;
	}
	if (PlayerPowerItem(GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)) ==
		0x18000013)
	{
		bPass = true;
	}
	WVector move = ball.m_vel * dt;
	float t = CheckCollision(ball.m_pos, move, &ball.m_meshIndex, false);
	Waabb water;
	WPlane plane;
	WVector next = move + ball.m_pos;
	if (move.y < 0.0f && IsFallenIntoWater(next, &water, false))
	{
		float waterT = (ball.m_pos.y - water.max.y) / move.y * -1.0f;
		if (waterT < t)
		{
			ball.m_pos += move * waterT;
			if (ball.m_bounceFrame < 0)
			{
				ball.m_bounceFrame = ball.m_renderNum;
			}
			ball.m_special &= ~0x20;
			float angle = acosf(Between(0.0f,
				ball.m_vel.y / ball.m_vel.Magnitude() * -1.0f, 1.0f));
			if ((ball.m_special & 0x50) == 0 && angle < 1.3962634f &&
				ball.m_vel.y < -10.0f)
			{
				WVector flat(ball.m_vel.x, 0.0f, ball.m_vel.z);
				if (flat.Magnitude() > 10.0f &&
					(angle > 1.0471976f || (ball.m_special & 1) != 0))
				{
					m_bounceCount++;
					ball.m_water = CGolfBall::WATER_NONE;
					ball.m_vel *= 1.0f - ball.m_initSpin * 0.1f;
					ball.m_vel.y =
						-((ball.m_initSpin * 0.1f + 0.75f) * ball.m_vel.y);
					pExpected->event = 4;
					pExpected->effect = 4;
					pExpected->normal = WVector::UNIT_POS_Y;
					strcpy(pExpected->sound, "\xc3\xe6\xb5\xb9_water");
					return;
				}
			}
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			ball.m_state = CGolfBall::BALL_STOP;
			ball.m_bOB = true;
			pExpected->effect = 4;
			strcpy(pExpected->sound, "\xc3\xe6\xb5\xb9_water");
			SetPenaltyPos(water);
			return;
		}
		GolfBall().m_water = CGolfBall::WATER_NONE;
	}
	if (t >= 1.0f)
	{
		t = IntersectObj(ball.m_pos, move, &plane);
		if (t >= 1.0f || bPass)
		{
			ball.m_pos += move;
			if (ball.m_colTriIndex != -1)
			{
				WVector v[3];
				for (int i = 0; i < 3; i++)
				{
					v[i] = PolyObjs(m_pPolySoup)[ball.m_colObjIndex]
							   ->tri[ball.m_colTriIndex]
							   .v[i];
					v[i].y = ball.m_pos.y;
				}
				if (!PointInTriangle(ball.m_pos, v[0], v[1], v[2]))
				{
					ball.m_colObjIndex = -1;
					ball.m_colTriIndex = -1;
				}
			}
		}
		else
		{
			if (m_colData.objIndex >= 0)
			{
				if (m_pPolySoup->GetProperty(ball.m_attrIndex).ground == 0xc)
				{
					sQuadPuppetHolder* pObj =
						PolyObjs(m_pPolySoup)[m_colData.objIndex];
					ball.m_pos += move;
					pExpected->event = 5;
					float up;
					float forward;
					if (sscanf(pObj->option, "%f %f", &up, &forward) < 2)
					{
						return;
					}
					float z = ball.m_vel.z;
					ball.m_vel = pObj->mat.ya * (Abs(ball.m_vel.y) + up);
					forward = Abs(z) + forward;
					ball.m_vel += pObj->mat.za * forward;
					return;
				}
				if (m_colData.objIndex >= 0 &&
					m_pPolySoup->GetProperty(ball.m_attrIndex).ground == 0xd)
				{
					sQuadPuppetHolder* pObj =
						PolyObjs(m_pPolySoup)[m_colData.objIndex];
					ball.m_pos += move;
					pExpected->event = 5;
					pExpected->hitObj983 = (unsigned long)pObj;
					float up;
					float forward;
					if (sscanf(pObj->option, "%f %f", &up, &forward) < 2)
					{
						return;
					}
					float z = ball.m_vel.z;
					up *= 0.33333334f;
					forward = up + forward;
					if (up < 0.1f)
					{
						up = 1.0f;
					}
					ball.m_vel = pObj->mat.ya * (Abs(ball.m_vel.y) + up);
					forward = Abs(z) + forward;
					ball.m_vel += pObj->mat.za * forward;
					return;
				}
			}
			sColResponse response(GolfBall(), m_colData.pos, m_colData.normal);
			ObjResponse(move, t, plane.normal, response);
			CommitResponse(GolfBall(), response, GolfBall().m_renderNum);
			pExpected->effect = 2;
			pExpected->event = 2;
			ball.m_curve = 0.0f;
			ball.m_spin = 0.0f;
			ball.m_special = 0;
			CVectorSlideTestContainer* pSlide = NULL;
			CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a,
				pSlide);
			if (pSlide != NULL && pSlide->IsNewTypeVectorSlide() &&
				plane.normal * WVector(0.0f, 1.0f, 0.0f) <
					cos(45 * g_DEGTORAD) &&
				ball.m_groundType == 8)
			{
				ball.m_vel.y = 0.0f;
				return;
			}
		}
	}
	else
	{
		WPolySoup::sTriangle* pTri = &m_pHolder->tri[ball.m_meshIndex];
		WVector normal;
		if (!pTri->bEdge)
		{
			normal = pTri->plane.normal;
			if (pTri->bDoubleSide && normal * move > 0.0f)
			{
				normal = -normal;
			}
		}
		else
		{
			WVector hit = move * t + ball.m_pos;
			normal = intersection::closestPointOnTriangle(hit, pTri->v[0],
				pTri->v[1], pTri->v[2]);
			normal = move * t + ball.m_pos - normal;
			normal.Normalize();
		}
		ball.m_attrIndex = pTri->propIndex;
		ball.m_pos += move * t;
		BounceResponse(normal);
		CVectorSlideTestContainer* pSlide =
			static_cast<CVectorSlideTestContainer*>(
				CContentsDoc::Instance()->GetContainer(
					(localContentType_t)0x9a));
		if (pSlide != NULL && pSlide->IsNewTypeVectorSlide() &&
			normal * WVector(0.0f, 1.0f, 0.0f) < cos(45 * g_DEGTORAD) &&
			ball.m_groundType == 8)
		{
			ball.m_vel.y = 0.0f;
		}
		if (PlayerBunkerState(
				GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)) != 0)
		{
			pExpected->effect = 3;
		}
		else
		{
			pExpected->effect = normal.y > cos(45 * g_DEGTORAD) ? 1 : 2;
		}
		pExpected->normal = normal;
		pExpected->event = 1;
		if (m_pPolySoup->GetProperty(ball.m_attrIndex).ground != 8)
		{
			WPolySoup* pSoup = m_pPolySoup;
			int texHandle = m_pHolder->tri[ball.m_meshIndex].texHandle & 0x7ff;
			PolyTexMap(pSoup).find(texHandle);
			const char* pName =
				PolyTexNames(pSoup) + PolyTexMap(pSoup)[texHandle] * 0x28;
			char* pDst = g_colTex[g_colNum];
			char c;
			do
			{
				c = *pName++;
				*pDst++ = c;
			} while (c != '\0');
			g_colNum++;
			if (g_colNum > 0xf)
			{
				g_colNum = 0xf;
			}
			m_colData.frame = ball.m_renderNum;
			m_colData.objIndex = -1;
			m_colData.triIndex = ball.m_meshIndex;
			m_colData.groundType = 0;
			m_colData.pos = ball.m_pos;
			m_colData.normal = ball.m_vel;
			m_colList.push_back(m_colData);
			return;
		}
		float height =
			(m_pPolySoup->GetProperty(GolfBall().m_attrIndex).max[1] > 500.0f
					? 500.0f
					: m_pPolySoup->GetProperty(GolfBall().m_attrIndex).max[1]) *
			0.01f;
		pSlide = static_cast<CVectorSlideTestContainer*>(
			CContentsDoc::Instance()->GetContainer((localContentType_t)0x9a));
		if (pSlide != NULL)
		{
			MakeVectorSlide(height, (float)pSlide->GetLimitSpeed(),
				(float)pSlide->GetBoundHeight(),
				pSlide->GetLimitVectorSlideCount());
			return;
		}
	}
}

float CQuadTree::CheckCollision(const WVector& pos, const WVector& dir,
	int* pIndex, bool bFlag)
{
	float holeCupRadius = HOLECUP_RADIUS983();
	CGolfBall& ball = GolfBall();
	float radius = ball.m_radius;
	WVector aMin;
	aMin.x = (pos.x - Abs(dir.x)) + WVector::ONE.x * radius;
	aMin.y = (pos.y - Abs(dir.y)) + WVector::ONE.y * radius;
	aMin.z = (pos.z - Abs(dir.z)) + radius * WVector::ONE.z;
	radius = ball.m_radius;
	Waabb box;
	box.min = aMin;
	box.max.x = Abs(dir.x) + pos.x + WVector::ONE.x * radius;
	box.max.y = Abs(dir.y) + pos.y + WVector::ONE.y * radius;
	box.max.z = Abs(dir.z) + pos.z + radius * WVector::ONE.z;
	GetMeshList(box);
	float best = 1.0f;
	if (pIndex != NULL)
	{
		*pIndex = -1;
	}
	for (int i = 0; i < m_meshNum; i++)
	{
		int index = m_meshList[i];
		WPolySoup::sTriangle* pTri = &m_pHolder->tri[index];
		WVector a = pTri->v[0];
		WVector b = pTri->v[1];
		WVector c = pTri->v[2];
		pTri->bEdge = false;
		WPlane plane = m_pHolder->tri[index].plane;
		if (m_pHolder->tri[index].bDoubleSide &&
			plane.x * pos.x + plane.z * pos.z + plane.y * pos.y + plane.dis <
				0.0f)
		{
			plane.x = -plane.x;
			plane.y = -plane.y;
			plane.z = -plane.z;
			plane.dis = plane.dis * -1.0f;
		}
		float r = ball.m_radius;
		WVector start;
		start.x = pos.x - plane.x * r;
		start.y = pos.y - r * plane.y;
		start.z = pos.z - plane.z * r;
		const sProperty* pProp =
			&m_pPolySoup->GetProperty(m_pHolder->tri[index].propIndex);
		float t =
			intersection::IntersectPlane(start, dir, plane, pProp->ground == 2);
		if (t < best)
		{
			float dx = t * dir.x;
			float dy = t * dir.y;
			float dz = t * dir.z;
			WVector hit;
			hit.x = dx + start.x;
			hit.y = start.y + dy;
			hit.z = start.z + dz;
			bool bCheck = bFlag;
			if (!bCheck)
			{
				if ((ball.m_special & 0x20) != 0 && ball.m_cobraFrame < 0 &&
					plane.y > cos(20 * g_DEGTORAD))
				{
					m_bSpecialSkipHit = true;
					continue;
				}
				if ((ball.m_special & 0x40) != 0 && ball.m_spikeFrame < 0 &&
					plane.y > cos(20 * g_DEGTORAD))
				{
					m_bSpecialSkipHit = true;
					continue;
				}
				const float* pCam = SharedCameraPos();
				WVector diff;
				diff.x = pCam[0] - (dx + pos.x);
				diff.y = pCam[1] - (dy + pos.y);
				diff.z = pCam[2] - (dz + pos.z);
				bCheck =
					sqrtf(diff.z * diff.z + diff.x * diff.x) >= holeCupRadius;
			}
			if (bCheck)
			{
				if (intersection::SameSide(hit, a, b, c) &&
					intersection::SameSide(hit, b, a, c) &&
					intersection::SameSide(hit, c, a, b))
				{
					best = t;
					if (pIndex != NULL)
					{
						*pIndex = index;
					}
				}
				else
				{
					WVector closest =
						intersection::closestPointOnTriangle(hit, a, b, c);
					float rr = GolfBall().m_radius;
					float t2 =
						intersection::intersectPolygon(closest, pos, dir, rr);
					if (t2 < best)
					{
						pTri->bEdge = true;
						best = t2;
						if (pIndex != NULL)
						{
							*pIndex = index;
						}
					}
				}
			}
		}
	}
	return best;
}

void CQuadTree::CalcGreenPlane()
{
	if (!WisEqual(m_greenAabb.max, m_greenAabb.min, 0.00001f))
	{
		sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		WVector slopeVec((pPlayer->pos.x - m_greenPoint.x) /
				(m_greenAabb.max.x - m_greenAabb.min.x),
			0.0f,
			(pPlayer->pos.z - m_greenPoint.z) /
				(m_greenAabb.max.z - m_greenAabb.min.z));
		m_greenPlane.normal = WVector::UNIT_POS_Y + slopeVec * m_greenFactor;
		m_greenPlane.normal.Normalize();
		m_greenPlane.dis = m_greenPlane.x * -1.0f * m_greenPoint.x +
			m_greenPlane.y * -1.0f * m_greenPoint.y +
			m_greenPlane.z * -1.0f * m_greenPoint.z;
	}
}

float CQuadTree::GetGreenPos(const WVector& pos)
{
	if (!WisEqual(m_greenAabb.max, m_greenAabb.min, 1e-05f) &&
		GetGroundType(pos, true, NULL) == 2)
	{
		return (m_greenPlane.a * pos.x + m_greenPlane.c * pos.z +
				   m_greenPlane.d) /
			m_greenPlane.b * -1.0f;
	}
	return pos.y;
}

WVector CQuadTree::GetFriction()
{
	GolfBall().m_attrIndex = m_pHolder->tri[GolfBall().m_meshIndex].propIndex;
	WVector normal;
	if (!WisEqual(m_greenAabb.max, m_greenAabb.min, 0.00001f) &&
		GetGroundType(GolfBall().m_pos, true, NULL) == 2)
	{
		normal = m_greenPlane.normal;
	}
	else
	{
		normal = m_pHolder->tri[GolfBall().m_meshIndex].plane.normal;
		if (normal.y < 0.0f &&
			m_pHolder->tri[GolfBall().m_meshIndex].bDoubleSide)
		{
			normal = -normal;
		}
	}
	float d = WVector::UNIT_POS_Y * normal;
	WVector slope = WVector::UNIT_NEG_Y + normal * d;
	slope.Normalize();
	slope *= sinf(acosf(Between(-1.0f, d, 1.0f))) * m_slopeFactor *
		GolfBall().GetMass() * m_gravity * 34.295296f;
	WVector friction = GolfBall().m_vel * -1.0f;
	friction.Normalize();
	float roll = m_pPolySoup->GetProperty(GolfBall().m_attrIndex).roll;
	float gravity = m_gravity * 34.295296f;
	float mass = GolfBall().GetMass();
	friction *= GetWeatherRollCoeff(GolfBall().m_attrIndex) * m_rollFactor *
		mass * gravity * roll * d;
	WVector result = friction + slope;
	if (GolfBall().m_vel.Magnitude() <
		friction.Magnitude() / GolfBall().GetMass() * 0.02f)
	{
		GolfBall().m_state = (CGolfBall::eBallState)0;
	}
	return result;
}

void CQuadTree::SpinProcess(float dt)
{
	CGolfBall& ball = GolfBall();
	ball.m_spinRot = dt * ball.m_initSpin * 62.831856f + ball.m_spinRot;
	if (ball.m_renderNum == ball.m_afterBurnerFrame + 50)
	{
		ball.m_vel.x = ball.m_afterBurner.x + ball.m_vel.x;
		ball.m_vel.y = ball.m_afterBurner.y + ball.m_vel.y;
		ball.m_vel.z = ball.m_afterBurner.z + ball.m_vel.z;
		ball.m_rollFrame = ball.m_renderNum;
		ball.m_state = CGolfBall::BALL_ROLL;
		RollProcess(dt);
	}
}

float CQuadTree::IntersectRollingBallGround(const WVector& pos,
	const WVector& vel, const float& dist, int& outIndex, WVector& outNormal,
	bool bFlag1, bool bFlag2, bool bBothSides)
{
	GetMeshList(pos, vel);
	float t = 1.0f;
	for (int i = 0; i < m_meshNum; i++)
	{
		const WPolySoup::sTriangle* tri = &m_pHolder->tri[m_meshList[i]];
		const WVector* v = tri->v;
		WPlane plane = tri->plane;
		if (m_pHolder->tri[m_meshList[i]].bDoubleSide &&
			plane.x * pos.x + plane.y * pos.y + plane.z * pos.z + plane.dis <
				0.0f)
		{
			plane.x = -plane.x;
			plane.y = -plane.y;
			plane.z = -plane.z;
			plane.dis = plane.dis * -1.0f;
		}
		if (bFlag1 && bFlag2)
		{
			if (!(Abs(plane.y) <= 0.5f &&
					plane.x * vel.x + plane.y * vel.y + plane.z * vel.z <=
						0.0f &&
					(pos.y <= v[0].y || pos.y <= v[1].y || pos.y <= v[2].y)))
			{
				continue;
			}
		}
		outNormal.x = plane.x;
		outNormal.y = plane.y;
		outNormal.z = plane.z;
		float d = dist;
		float ox = d * plane.y;
		float oz = plane.z * d;
		WVector start;
		start.x = pos.x - plane.x * d;
		start.y = pos.y - ox;
		start.z = pos.z - oz;
		t = intersection::IntersectPlane(start, vel, plane, bBothSides);
		if (t < 1.0f)
		{
			float dx = t * vel.x;
			float dy = t * vel.y;
			float dz = t * vel.z;
			WVector hit;
			hit.x = dx + start.x;
			hit.y = dy + start.y;
			hit.z = dz + start.z;
			if (intersection::SameSide(hit, v[0], v[1], v[2]) &&
				intersection::SameSide(hit, v[1], v[0], v[2]) &&
				intersection::SameSide(hit, v[2], v[0], v[1]))
			{
				outIndex = m_meshList[i]; // 983 009555f0: indice do triangulo, nao da lista
				return t;
			}
			WVector p2;
			p2.x = dx + start.x;
			p2.y = dy + start.y;
			p2.z = dz + start.z;
			WVector closest =
				intersection::closestPointOnTriangle(p2, v[0], v[1], v[2]);
			float radius = GolfBall().m_radius;
			t = intersection::intersectPolygon(closest, pos, vel, radius);
			if (t < 1.0f)
			{
				outIndex = m_meshList[i];
				return t;
			}
		}
	}
	return t;
}

void CQuadTree::RollingBallGroundResponse(CGolfBall& ball, float dt,
	const WVector& dir, int triIndex, const WVector& normal)
{
	if (IsLocalContent((localContentType_t)0x6b))
	{
		ball.m_spin = 0.0f;
		ball.m_curve = 0.0f;
		ball.m_pos.x = dt * dir.x + ball.m_pos.x;
		ball.m_pos.y = dt * dir.y + ball.m_pos.y;
		ball.m_pos.z = dt * dir.z + ball.m_pos.z;
		ball.m_pos.y =
			GetHighestGround(ball.m_pos, &ball.m_meshIndex) + ball.m_radius;
		ball.m_attrIndex = m_pHolder->tri[triIndex].propIndex;
		ReflectVector(normal, ball.m_vel);
		float bounce = m_pPolySoup->GetProperty(ball.m_attrIndex).bound;
		float coeff = GetWeatherBounceCoeff(ball.m_attrIndex) * bounce;
		ball.m_vel.x = coeff * ball.m_vel.x;
		ball.m_vel.y = coeff * ball.m_vel.y;
		ball.m_vel.z = coeff * ball.m_vel.z;
		CGolfBall::sExpected* pExpected = ball.PreDataToWrite(ball.m_renderNum);
		pExpected->normal = normal;
		return;
	}
	ball.m_spin = 0.0f;
	ball.m_curve = 0.0f;
	ball.m_pos.x = dt * dir.x + ball.m_pos.x;
	ball.m_pos.z = dt * dir.z + ball.m_pos.z;
	ball.m_pos.y = ball.m_radius + ball.m_pos.y + dt * dir.y;
	ball.m_attrIndex = m_pHolder->tri[triIndex].propIndex;
	float bounce = m_pPolySoup->GetProperty(ball.m_attrIndex).bound;
	float coeff = GetWeatherBounceCoeff(ball.m_attrIndex) * bounce * -1.0f;
	ball.m_vel.x = coeff * ball.m_vel.x;
	ball.m_vel.y = coeff * ball.m_vel.y;
	ball.m_vel.z = coeff * ball.m_vel.z;
	*reinterpret_cast<Str10*>(ball.PreDataToWrite(ball.m_renderNum)->sound) =
		*reinterpret_cast<const Str10*>("\xc3\xe6\xb5\xb9_wood");
	ball.PreDataToWrite(ball.m_renderNum)->normal = normal;
}

const WVector& GetHoleCupPos(); // 983: devolve referencia

void CQuadTree::RollProcess(float dt)
{
	float holeCupRadius = HOLECUP_RADIUS983();
	CGolfBall& ball = GolfBall();
	ball.m_meshIndex = GetMeshIndex(ball.m_pos, false);
	if (ball.m_meshIndex == -1)
	{
		ball.m_vel =
			ball.m_vel * -1.0f * (1.0f / ball.m_vel.Magnitude()) * 0.02f;
		ball.m_pos += ball.m_vel * 0.02f;
		if (ball.m_vel.Magnitude() < 0.02f / ball.m_mass)
		{
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			ball.m_bOB = true;
			ball.m_state = CGolfBall::BALL_STOP;
		}
		return;
	}
	float mass = ball.m_mass;
	ball.m_vel += GetFriction() * (1.0f / mass) * dt;
	if (ball.m_state == CGolfBall::BALL_STOP)
	{
		WVector hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
		WVector diff = ball.m_pos - hole;
		if (sqrtf(diff.x * diff.x + diff.z * diff.z) < holeCupRadius)
		{
			ball.m_pos = GetHoleCupPos();
			ball.m_pos.y = m_ballFloorY + ball.m_radius;
			HoleIn();
			return;
		}
		ball.m_vel.z = 0.0f;
		ball.m_vel.y = 0.0f;
		ball.m_vel.x = 0.0f;
		Waabb water;
		if (IsFallenIntoWater(ball.m_pos, &water, false))
		{
			ball.m_bOB = true;
			SetPenaltyPos(water);
			return;
		}
		if ((ball.m_special & 1) != 0)
		{
			MakeAfterBurner();
		}
		return;
	}
	WVector move = ball.m_vel * dt;
	WPlane plane;
	float t = IntersectObj(ball.m_pos, move, &plane);
	float tLimit = 0.0f;
	sColResponse response(GolfBall(), m_colData.pos, m_colData.normal);
	WVector next;
	if (t >= 1.0f)
	{
		next = move + ball.m_pos;
	}
	else
	{
		ObjResponse(move, t, plane.normal, response);
		if (response.m_bBounced)
		{
			tLimit = response.m_ball.GetRadius() / move.Magnitude() + t;
		}
		next = response.m_ball.m_pos;
		move = next - ball.m_pos;
	}
	float ground = GetHighestGround(next, &ball.m_meshIndex);
	if (ball.m_meshIndex == -1)
	{
		next.y = -100000.0f;
	}
	else
	{
		next.y = ground + ball.m_radius;
		ball.m_spinRot -=
			(next - ball.m_pos).Magnitude() / ball.m_radius * 0.3f;
	}
	int triIndex = -1;
	float radius = GolfBall().m_radius;
	WVector normal;
	float tGround = IntersectRollingBallGround(ball.m_pos, move, radius,
		triIndex, normal, true, true, false);
	if (tGround < 1.0f)
	{
		if (response.m_bBounced && t / tLimit < tGround)
		{
			response.m_ball.m_pos = GolfBall().m_pos;
			CommitResponse(GolfBall(), response, GolfBall().m_renderNum);
		}
		RollingBallGroundResponse(ball, tGround, move, triIndex, normal);
		return;
	}
	if (t < 1.0f)
	{
		CommitResponse(GolfBall(), response, GolfBall().m_renderNum);
	}
	if (move.y + ball.m_pos.y <= ball.m_radius * 0.5f + next.y)
	{
		int prev = ball.m_renderNum - 1;
		if (ball.PreData(prev)->state == CGolfBall::BALL_ROLL)
		{
			WVector delta = next - ball.PreData(prev)->pos;
			float len = sqrtf(delta.x * delta.x + delta.z * delta.z);
			if (WisZero(len))
			{
				ball.m_vel.z = 0.0f;
				ball.m_vel.y = 0.0f;
				ball.m_vel.x = 0.0f;
				ball.m_state = CGolfBall::BALL_STOP;
				if ((ball.m_special & 1) != 0)
				{
					MakeAfterBurner();
				}
			}
			else if (Abs(delta.y) / delta.Magnitude() < sin(60 * g_DEGTORAD))
			{
				ball.m_vel.y = sqrtf(ball.m_vel.z * ball.m_vel.z +
								   ball.m_vel.x * ball.m_vel.x) /
					len * delta.y;
			}
		}
		ball.m_pos = next;
	}
	else
	{
		ball.m_spin = 0.0f;
		ball.m_curve = 0.0f;
		ball.m_state = CGolfBall::BALL_BOUNCE;
		ball.m_meshIndex = -1;
		ball.m_pos = move + ball.m_pos;
		if (ball.m_bounceFrame < 0)
		{
			ball.m_bounceFrame = ball.m_renderNum;
		}
	}
	if (m_holeDist.z <= holeCupRadius + ball.m_radius)
	{
		if (m_cupRimFrames < 75.0f)
		{
			m_cupRimFrames++;
			WVector toBall;
			toBall = ball.m_pos - GetHoleCupPos();
			if (toBall.x * ball.m_vel.x + ball.m_vel.y * 0.0f +
					toBall.z * ball.m_vel.z >
				0.0f)
			{
				WVector pull = GetHoleCupPos() - ball.m_pos;
				pull.y = 0.0f;
				pull.Normalize();
				ball.m_vel += pull * 80.0f * dt;
			}
		}
	}
}

void CQuadTree::BounceResponse(const WVector& normal)
{
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(ball.m_renderNum);
	const char* sound = m_pPolySoup->GetProperty(ball.m_attrIndex).boundSound;
	if (sound[0] != '\0')
	{
		// 983 00961550: o som da bola vem da CItemSoundTable (estado 4); sem som do item, vale o da textura/propriedade
		std::string itemSound =
			CItemSoundTable::Instance().GetSoundName(SharedCurPlayerItem(), 4);
		if (!itemSound.empty())
		{
			strcpy(pExpected->sound, itemSound.c_str());
		}
		else
		{
			const char* texName = GetTextureName(ball.m_pos);
			if (texName != NULL &&
				(stricmp(texName, "icegrass07_03.dds") == 0 ||
					stricmp(texName, "igrass03.dds") == 0 ||
					stricmp(texName, "igrass06.dds") == 0))
			{
				const char* iceSound[4] = { "ball_ice\xc0\xfd\xba\xae"
											"3",
					"ball_ice\xc0\xfd\xba\xae"
					"3-1",
					"ball_ice\xc0\xfd\xba\xae"
					"3-2",
					"ball_ice\xc0\xfd\xba\xae"
					"3-3" };
				strcpy(pExpected->sound, iceSound[rand() % 4]);
			}
			else
			{
				strcpy(pExpected->sound, sound);
			}
		}
	}
	bool bBounce = true;
	if (ball.m_bounceFrame < 0)
	{
		ball.m_bounceFrame = ball.m_renderNum;
		ball.m_curve = 0.0f;
		if (normal.y > cos(20 * g_DEGTORAD))
		{
			if ((ball.m_special & 0x10) != 0 || StuckInBunker(normal) ||
				(ball.m_special & 0x40) != 0)
			{
				bBounce = false;
			}
		}
		else if ((ball.m_special & 0x40) != 0 && ball.m_topFrame >= 0 &&
			ball.m_topFrame < ball.m_bounceFrame)
		{
			ball.m_vel *= 0.5f;
		}
		ball.m_special &= ~0x70;
		if (!bBounce)
		{
			if ((ball.m_special & 3) != 0)
			{
				MakeAfterBurner();
				return;
			}
			if (GetGroundType(ball.m_pos, true, NULL) != 3)
			{
				PlayerBunkerState(
					GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)) = 0;
				ball.m_state = (CGolfBall::eBallState)0;
				ball.m_vel.z = 0.0f;
				ball.m_vel.y = 0.0f;
				ball.m_vel.x = 0.0f;
				return;
			}
			if (ball.m_vel.Magnitude() <= 150.0f &&
				(ball.m_initSpecial & 0x50) == 0)
			{
				PlayerBunkerState(PLAYER(GOLFDOC()->m_currentPlayer)) = 1;
				ball.m_state = (CGolfBall::eBallState)0;
				ball.m_vel.z = 0.0f;
				ball.m_vel.y = 0.0f;
				ball.m_vel.x = 0.0f;
				return;
			}
			PlayerBunkerState(
				GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)) = 2;
			ball.m_state = (CGolfBall::eBallState)0;
			ball.m_vel.z = 0.0f;
			ball.m_vel.y = 0.0f;
			ball.m_vel.x = 0.0f;
			return;
		}
	}
	ball.m_spin = Min(ball.m_spin, 0.0f);
	WVector perp = normal * ((normal * ball.m_vel) * -1.0f);
	WVector para = perp + ball.m_vel;
	float spinFactor = ball.m_initSpin * 0.1f + 0.3f;
	WVector bounce = perp *
		(GetWeatherBounceCoeff(ball.m_attrIndex) * m_bounceFactor *
			m_pPolySoup->GetProperty(ball.m_attrIndex).bound);
	float keep = 1.0f -
		(1.0f -
			GetWeatherBounceCoeff(ball.m_attrIndex) * m_bounceFactor *
				m_pPolySoup->GetProperty(ball.m_attrIndex).bound) *
			spinFactor;
	ball.m_vel = para * keep + bounce;
}

bool CQuadTree::StuckInBunker(const WVector& normal)
{
	float d =
		((GolfBall().m_vel * -1.0f) * normal) / GolfBall().m_vel.Magnitude();
	if (GetGroundType(GolfBall().m_pos, true, NULL) == 3 &&
		GolfBall().m_vel.Magnitude() > 50.0f && d > cos(45 * g_DEGTORAD))
	{
		return true;
	}
	if (GetGroundType(GolfBall().m_pos, true, NULL) == 11 &&
		GolfBall().m_vel.Magnitude() > 50.0f && d > cos(45 * g_DEGTORAD))
	{
		return true;
	}
	if (GetGroundType(GolfBall().m_pos, true, NULL) == 14 &&
		GolfBall().m_vel.Magnitude() > 50.0f && d > cos(45 * g_DEGTORAD))
	{
		return true;
	}
	return false;
}

void CQuadTree::ObjResponse(const WVector& dir, float t, const WVector& normal,
	sColResponse& response)
{
	response.m_normal.x = normal.x;
	response.m_normal.y = normal.y;
	response.m_normal.z = normal.z;
	const char* pSrc =
		m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex()).boundSound;
	if (*pSrc != '\0')
	{
		if (SharedCurPlayerItem() == 0x14000004)
		{
			sprintf(response.m_sound, "t_Ex4-%d", rand() % 4 + 1);
		}
		else
		{
			char* pDst = response.m_sound;
			char c;
			do
			{
				c = *pSrc++;
				*pDst++ = c;
			} while (c != '\0');
		}
	}
	if ((m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex()).pass & 1) ==
		0)
	{
		if (response.m_ball.m_state == 3)
		{
			WVector& vel = response.m_ball.m_vel;
			response.m_ball.m_pos.x = t * dir.x + response.m_ball.m_pos.x;
			response.m_ball.m_pos.y = t * dir.y + response.m_ball.m_pos.y;
			response.m_ball.m_pos.z = t * dir.z + response.m_ball.m_pos.z;
			ReflectVector(normal, vel);
			float bounce =
				m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex()).bound;
			vel.x = bounce * vel.x;
			vel.y = bounce * vel.y;
			vel.z = bounce * vel.z;
			response.m_colPos = response.m_ball.m_pos;
			response.m_colNormal = vel;
			response.m_bRolled = true;
			return;
		}
		if (response.m_ball.m_colTriIndex == -1)
		{
			WVector& vel = response.m_ball.m_vel;
			float d = (vel.x * normal.x + vel.y * normal.y + vel.z * normal.z) *
				-1.0f;
			float px = d * normal.x;
			float py = d * normal.y;
			float pz = d * normal.z;
			float spin = response.m_ball.GetInitSpin();
			float vx = vel.x;
			float vy = vel.y;
			float vz = vel.z;
			float bounce =
				m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex()).bound;
			float k = 1.0f -
				(1.0f -
					m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex())
						.bound) *
					(spin * 0.1f + 0.3f);
			vel.x = (px + vx) * k + px * bounce;
			vel.y = (py + vy) * k + py * bounce;
			vel.z = (pz + vz) * k + pz * bounce;
			if ((response.m_ball.GetSpecial() & 0x10) != 0)
			{
				vel.x = vel.x * 0.25f;
				vel.y = vel.y * 0.25f;
				vel.z = vel.z * 0.25f;
			}
			if ((response.m_ball.GetSpecial() & 0x40) != 0 &&
				response.m_ball.GetTopFrame() >= 0 &&
				response.m_ball.GetTopFrame() < response.m_ball.GetRenderNum())
			{
				vel *= 0.25f;
			}
			response.m_ball.m_pos.x = t * dir.x + response.m_ball.m_pos.x;
			response.m_ball.m_pos.y = t * dir.y + response.m_ball.m_pos.y;
			response.m_ball.m_pos.z = t * dir.z + response.m_ball.m_pos.z;
			response.m_colPos = response.m_ball.m_pos;
			response.m_colNormal = vel;
			response.m_bRolled = true;
		}
		else
		{
			response.m_ball.m_colObjIndex = m_colList.back().objIndex;
			response.m_ball.m_colTriIndex = m_colList.back().triIndex;
			WPolySoup::sTriangle* tri =
				&PolyObjs(m_pPolySoup)[response.m_ball.m_colObjIndex]
					 ->tri[response.m_ball.m_colTriIndex];
			if (Wabs(tri->plane.y) >= g_EPSILON)
			{
				float y = ((tri->plane.x * response.m_ball.m_pos.x +
							   tri->plane.z * response.m_ball.m_pos.z +
							   tri->plane.dis) /
							  tri->plane.y) *
					-1.0f;
				response.m_ball.m_pos.y = y;
				response.m_ball.m_pos.y = y + response.m_ball.GetRadius();
			}
			else
			{
				response.m_ball.m_vel.z = 0.0f;
				response.m_ball.m_vel.y = 0.0f;
				response.m_ball.m_vel.x = 0.0f;
				response.m_ball.m_state = (CGolfBall::eBallState)0;
			}
		}
		WVector& vel = response.m_ball.m_vel;
		if (vel.x * normal.x + (vel.y - GetGravity() * 0.02f) * normal.y +
				vel.z * normal.z <
			0.0f)
		{
			if (cos(20 * g_DEGTORAD) < normal.y)
			{
				vel.z = 0.0f;
				vel.y = 0.0f;
				vel.x = 0.0f;
				response.m_ball.m_state = (CGolfBall::eBallState)0;
				return;
			}
			if (0.0f < vel.y)
			{
				vel.x = -vel.x;
				vel.y = -vel.y;
				vel.z = -vel.z;
			}
			// 983 (00964380): ja rolou neste passo -> indices do m_colData (+0x21e4)
			if (response.m_bRolled)
			{
				response.m_ball.m_colObjIndex = m_colData.objIndex;
				response.m_ball.m_colTriIndex = m_colData.triIndex;
				return;
			}
			response.m_ball.m_colObjIndex = m_colList.back().objIndex;
			response.m_ball.m_colTriIndex = m_colList.back().triIndex;
		}
	}
	else if (!WisEqual(dir, WVector::ZERO, 1e-05f))
	{
		response.m_bBounced = true;
		float s = response.m_ball.GetRadius() /
				sqrtf(dir.z * dir.z + dir.y * dir.y + dir.x * dir.x) +
			t;
		response.m_ball.m_pos.x = s * dir.x + response.m_ball.m_pos.x;
		response.m_ball.m_pos.y = s * dir.y + response.m_ball.m_pos.y;
		response.m_ball.m_pos.z = s * dir.z + response.m_ball.m_pos.z;
		float bounce =
			m_pPolySoup->GetProperty(response.m_ball.GetAttrIndex()).bound;
		response.m_ball.m_vel.x = bounce * response.m_ball.m_vel.x;
		response.m_ball.m_vel.y = bounce * response.m_ball.m_vel.y;
		response.m_ball.m_vel.z = bounce * response.m_ball.m_vel.z;
		response.m_colPos = response.m_ball.m_pos;
		response.m_colNormal = response.m_ball.m_vel;
		response.m_bRolled = true;
	}
}

CQuadTree::sColResponse::sColResponse(const CGolfBall& ball,
	const WVector& colPos, const WVector& colNormal)
	: m_ball(ball), m_colPos(colPos), m_colNormal(colNormal)
{
	m_bBounced = false;
	m_bRolled = false;
	m_sound[0] = '\0';
	// B1555: layout do 983 (ctor 0094d860: colPos +0xa0, colNormal +0xac, bools +0xb8; ObjResponse 0095daa0: som +0x48, normal +0x8c)
	C_ASSERT_VC10(offsetof(sColResponse, m_sound) == 0x48);
	C_ASSERT_VC10(offsetof(sColResponse, m_normal) == 0x8c);
	C_ASSERT_VC10(offsetof(sColResponse, m_colPos) == 0xa0);
	C_ASSERT_VC10(offsetof(sColResponse, m_colNormal) == 0xac);
	C_ASSERT_VC10(offsetof(sColResponse, m_bRolled) == 0xb9);
}

void CQuadTree::CommitResponse(CGolfBall& ball, const sColResponse& response,
	int index)
{
	ball.m_pos = response.m_ball.m_pos;
	ball.m_vel = response.m_ball.m_vel;
	ball.m_colObjIndex = response.m_ball.m_colObjIndex;
	ball.m_colTriIndex = response.m_ball.m_colTriIndex;
	ball.m_state = response.m_ball.m_state;
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(index);
	pExpected->normal = response.m_normal;
	const char* src = response.m_sound;
	char* dst = pExpected->sound;
	char c;
	do
	{
		c = *src;
		*dst = c;
		src++;
		dst++;
	} while (c != 0);
	m_colData.pos = response.m_colPos;
	m_colData.normal = response.m_colNormal;
	if (response.m_bRolled)
	{
		m_colList.push_back(m_colData);
	}
}

void __cdecl TextOut(WView* view, float x, float y, const char* format, ...);

void CQuadTree::Display()
{
	WTVertex vtx[3];
	WTVertex* vl[4] = { &vtx[0], &vtx[1], &vtx[2], NULL };
	if (m_paintIndex >= 0)
	{
		vtx[0].SetPosition(m_pHolder->tri[m_paintIndex].v[0]);
		vtx[0].diffuse = 0xffffffff;
		vtx[1].SetPosition(m_pHolder->tri[m_paintIndex].v[1]);
		vtx[1].diffuse = 0xffffffff;
		vtx[2].SetPosition(m_pHolder->tri[m_paintIndex].v[2]);
		vtx[2].diffuse = 0xffffffff;
		// 983 00951330: (vl, 0x8300000, 0x200, 0)
		g_view->DrawPolygonFan(vl, 0x8300000, 0x200, false);
	}
	if (COption::Instance()->dIsVtxClrTestEnabled())
	{
		TextOut(g_view, 470.0f, 60.0f,
			m_vtxColorMode == 0 ? "<<BASE COLOR MODE>>" : "<<OBJ COLOR MODE>>");
		TextOut(g_view, 470.0f, 72.0f,
			MakeStr("Base : %.0f %.0f %.0f", m_baseColor.x, m_baseColor.y,
				m_baseColor.z));
		TextOut(g_view, 470.0f, 84.0f,
			MakeStr("Obj  : %.0f %.0f %.0f", m_objColor.x, m_objColor.y,
				m_objColor.z));
	}
}

WVector CQuadTree::closestPointOnTriangle(const WVector& p, const WVector& a,
	const WVector& b, const WVector& c) const
{
	return intersection::closestPointOnTriangle(p, a, b, c);
}

WVector CQuadTree::closestPointOnLine(const WVector& a, const WVector& b,
	const WVector& p) const
{
	return intersection::closestPointOnLine(a, b, p);
}

float CQuadTree::intersectPolygon(const WVector& pos, const WVector& center,
	const WVector& vel) const
{
	float radius = GolfBall().GetRadius();
	return intersection::intersectPolygon(pos, center, vel, radius);
}

void CQuadTree::HoleProcess(float dt)
{
	float holeCupRadius = HOLECUP_RADIUS983();
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(ball.m_renderNum);
	ball.m_vel.y -= dt * m_gravity * 34.295296f;
	WVector move = ball.m_vel * dt;
	float bottom = ball.m_pos.y - ball.m_radius - m_ballFloorY;
	float nextBottom = move.y + bottom;
	float cross = nextBottom * bottom;
	float t;
	if (cross < 0.0f && ball.m_vel.y < 0.0f && !WisZero(bottom))
	{
		t = bottom / (bottom - nextBottom);
	}
	else
	{
		t = 1.0f;
	}
	move = move * t;
	float colT = CheckCollision(ball.m_pos, move, &ball.m_meshIndex, false);
	if (ball.m_meshIndex >= 0)
	{
		ball.m_attrIndex = m_pHolder->tri[ball.m_meshIndex].propIndex;
	}
	WVector toBall =
		ball.m_pos - *reinterpret_cast<const WVector*>(SharedCameraPos());
	toBall.y = 0.0f;
	WVector hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
	float dist = toBall.Magnitude() - (holeCupRadius - ball.m_radius);
	WVector toNext = move + ball.m_pos - hole;
	WVector flat = toNext;
	flat.y = 0.0f;
	float nextDist = flat.Magnitude() - (holeCupRadius - ball.m_radius);
	if (ball.PreData(ball.m_renderNum - 1)->state == CGolfBall::BALL_BOUNCE &&
		dist > 0.0f)
	{
		if (nextDist > ball.m_radius)
		{
			ball.m_pos += move * colT;
			if (colT < 1.0f)
			{
				BounceResponse(GetMeshNormal(ball.m_meshIndex));
			}
		}
		else
		{
			ball.m_pos += move;
			if (nextDist > 0.0f)
			{
				int index;
				WVector pos = ball.m_pos;
				GetMeshList(pos);
				WVector ground = ColTest(pos, false, &index, true);
				if (ground.y < m_pHolder->aabb.min.y)
				{
					ground.y = m_pHolder->aabb.min.y;
				}
				float depth = ball.m_radius - nextDist;
				float top =
					sqrtf(ball.m_radius * ball.m_radius - depth * depth) +
					ground.y;
				WVector out = ball.m_pos - GetHoleCupPos();
				out.y = 0.0f;
				out.Normalize();
				out = out * holeCupRadius;
				WVector edge = out + GetHoleCupPos();
				WVector normal = (ball.m_pos - edge).Normalize();
				if (ball.m_pos.y < top)
				{
					ball.m_special &= ~0x73;
					ball.m_spin = 0.0f;
					ball.m_curve = 0.0f;
					ball.m_pos.y = top;
					// 983 00961ba0: o impulso e a velocidade contra a normal da borda (|vel.n| * 1.3), nao a altura do topo
					ball.m_vel += normal *
						(Abs(normal.x * ball.m_vel.x + normal.y * ball.m_vel.y +
							 normal.z * ball.m_vel.z) * 1.3f);
					pExpected->effect = 0;
					pExpected->event = 3;
					pExpected->normal = normal;
					strcpy(pExpected->sound, "\xc3\xe6\xb5\xb9_pole");
					return;
				}
			}
		}
	}
	else if (nextDist < 0.0f)
	{
		ball.m_pos += move;
		if (cross < 0.0f && ball.m_vel.y < 0.0f)
		{
			HoleIn();
		}
	}
	else
	{
		float hit = IntersectPoint(move);
		if (hit < colT)
		{
			ball.m_pos += move * colT;
			if (colT < 1.0f)
			{
				BounceResponse(GetMeshNormal(ball.m_meshIndex));
			}
		}
		else if (hit >= 1.0f)
		{
			ball.m_pos += move;
		}
		else
		{
			ball.m_special &= ~0x73;
			ball.m_pos += move * hit;
			WVector normal = GetHoleCupPos() - ball.m_pos;
			normal.y = 0.0f;
			normal.Normalize();
			ball.m_vel = normal * (ball.m_vel.Magnitude() * 0.3f);
			pExpected->effect = 0;
			pExpected->event = 3;
			pExpected->normal = normal;
			strcpy(pExpected->sound, "\xc3\xe6\xb5\xb9_pole");
		}
	}
	hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
	WVector toHole = hole - ball.m_pos;
	flat = toHole;
	flat.y = 0.0f;
	float holeDist = flat.Magnitude();
	if (holeCupRadius - ball.m_radius < holeDist)
	{
		if (ball.m_rollFrame >= 0)
		{
			float y = ball.m_pos.y;
			ball.m_pos = *reinterpret_cast<const WVector*>(SharedCameraPos()) -
				flat.Normalize() * (holeCupRadius - ball.m_radius);
			ball.m_pos.y = y;
			ball.m_vel = flat.Normalize() * (ball.m_vel.Magnitude() * 0.3f);
			return;
		}
		hole = *reinterpret_cast<const WVector*>(SharedCameraPos());
		if (ball.m_pos.y < hole.y)
		{
			if (t >= 1.0f)
			{
				float y = ball.m_pos.y;
				ball.m_pos = GetHoleCupPos() -
					flat.Normalize() * (holeCupRadius - ball.m_radius);
				ball.m_pos.y = y;
				ball.m_vel *= -0.3f;
				ball.m_special &= ~0x73;
				ball.m_vel.y *= -1.0f;
				return;
			}
			ball.m_pos = GetHoleCupPos();
			ball.m_pos.y = m_ballFloorY + ball.m_radius;
			HoleIn();
			return;
		}
		if (ball.m_pos.y < GetHoleCupPos().y + ball.m_radius)
		{
			if (holeDist <= holeCupRadius)
			{
				int index;
				WVector pos = ball.m_pos;
				GetMeshList(pos);
				WVector ground = ColTest(pos, false, &index, true);
				if (ground.y < m_pHolder->aabb.min.y)
				{
					ground.y = m_pHolder->aabb.min.y;
				}
				float top = sqrtf(ball.m_radius * ball.m_radius -
								(holeCupRadius - holeDist) *
									(holeCupRadius - holeDist)) +
					ground.y;
				if (ball.m_pos.y < top)
				{
					WVector dir = ball.m_vel;
					dir.Normalize();
					float up = -dir.y;
					WVector toCup = GetHoleCupPos() - ball.m_pos;
					toCup.y = 0.0f;
					toCup.Normalize();
					dir.y = 0.0f;
					dir = dir + toCup;
					dir.Normalize();
					dir = dir + toCup;
					dir.Normalize();
					dir.y = up;
					dir.Normalize();
					ball.m_pos.y = top;
					ball.m_vel = dir * (ball.m_vel.Magnitude() * 0.3f);
					ball.m_special &= ~0x73;
					return;
				}
			}
			else
			{
				int index;
				WVector pos = ball.m_pos;
				GetMeshList(pos);
				WVector ground = ColTest(pos, false, &index, true);
				if (ground.y < m_pHolder->aabb.min.y)
				{
					ground.y = m_pHolder->aabb.min.y;
				}
				float y = ground.y + ball.m_radius;
				if (ball.m_pos.y < y)
				{
					ball.m_special &= ~0x70;
					ball.m_pos.y = y;
					ball.m_vel.y =
						GetBounceCoeff(
							m_pHolder->tri[GetMeshIndex(ball.m_pos, false)]
								.propIndex) *
						ball.m_vel.y * -1.0f;
					pExpected->effect = 1;
					pExpected->event = 1;
					pExpected->normal = WVector::UNIT_POS_Y;
					const char* sound = GetBounceSound(
						m_pHolder->tri[GetMeshIndex(ball.m_pos, false)]
							.propIndex);
					if (sound[0] != '\0')
					{
						strcpy(pExpected->sound, sound);
					}
				}
			}
		}
	}
}

float CQuadTree::IntersectPoint(WVector dir)
{
	float holeCupRadius = HOLECUP_RADIUS983();
	const float* pHole = SharedCameraPos();
	float hx = pHole[0];
	float hz = pHole[2];
	float dx = GolfBall().m_pos.x - hx;
	float dz = GolfBall().m_pos.z - hz;
	float a = dir.x * dir.x + dir.z * dir.z;
	float b = dx * dir.x + dir.z * dz;
	float c = dx * dx + dz * dz;
	float det = b * b -
		(c -
			(holeCupRadius - GolfBall().m_radius) *
				(holeCupRadius - GolfBall().m_radius)) *
			a;
	if (det < 0.0f)
	{
		return 1.0f;
	}
	det = sqrtf(det);
	float inv = 1.0f / a;
	float t1 = (b * -1.0f - det) * inv;
	float t2 = (det - b) * inv;
	float range = holeCupRadius - GolfBall().m_radius;
	float dist = sqrtf(c);
	if (range == dist)
	{
		return 0.0f;
	}
	if (dist < range)
	{
		if (t2 < 1.0f)
		{
			return t2;
		}
		return 1.0f;
	}
	return t1;
}

float CQuadTree::GetWeatherRollCoeff(int propIndex) const
{
	// 983 00951620: evento 0x20 (Lossless) na chuva = sem efeito do clima (`fld1` em 009516a1)
	if (GOLFDOC()->IsCurrentPlayerEvent983(0x20) == true &&
		(Doc()->m_golfGame.weather >= 2 && Doc()->m_golfGame.weather <= 3))
	{
		return 1.0f;
	}
	if (Doc()->m_golfGame.weather >= 2 && Doc()->m_golfGame.weather <= 3)
	{
		switch (m_pPolySoup->GetProperty(propIndex).ground)
		{
		case 0:
			return 1.3f;
		default:
			return 1.8f;
		case 3:
			return 1.5f;
		case 2:
			return 1.13f;
		case 7:
		case 8:
			break;
		}
	}
	return 1.0f;
}

float CQuadTree::GetWeatherBounceCoeff(int propIndex) const
{
	// 983 009516d0: evento 0x20 (Lossless) na chuva = sem efeito do clima (`fld1` em 00951746)
	if (GOLFDOC()->IsCurrentPlayerEvent983(0x20) == true &&
		(Doc()->m_golfGame.weather >= 2 && Doc()->m_golfGame.weather <= 3))
	{
		return 1.0f;
	}
	if (Doc()->m_golfGame.weather >= 2 && Doc()->m_golfGame.weather <= 3)
	{
		switch (m_pPolySoup->GetProperty(propIndex).ground)
		{
		case 0:
		case 1:
		default:
			return 0.8f;
		case 2:
			return 0.9f;
		case 3:
			return 0.7f;
		case 7:
		case 8:
			break;
		}
	}
	return 1.0f;
}

void CQuadTree::SetCondition()
{
	switch (Doc()->m_golfGame.weather)
	{
	case 0:
	case 1:
		m_condition = 0.0f;
		return;
	case 2:
	case 3:
		switch (GolfBall().m_groundType)
		{
		case 0:
		case 1:
		case 3:
			m_condition = -2.0f;
			return;
		case 2:
			m_condition = 0.0f;
			return;
		case 4:
			m_condition = -3.0f;
			return;
		default:
			m_condition = -3.0f;
			return;
		}
	default:
		return;
	}
}

float CQuadTree::GetRollCoeff(int propIndex) const
{
	return m_pPolySoup->GetProperty(propIndex).roll;
}

float CQuadTree::GetBounceCoeff(int propIndex) const
{
	return m_pPolySoup->GetProperty(propIndex).bound;
}

// tee data of the current hole (defined in another unit; placeholder name until converted there)
struct sTeeData
{
	WVector pos;

	const WVector& GetTeeShotPos();
};
#pragma comment(linker, \
	"/alternatename:?GetTeeShotPos@sTeeData@@QAEABVWVector@@XZ=?GetTeeShotPos@@YI?AVWVector@@XZ")

unsigned char CQuadTree::GetGroundType(WVector pos, bool bCheckTee,
	CGolfBall::eWater* water)
{
	if (bCheckTee && !SharedApproachFlag())
	{
		sTeeData tee = SharedTeeData();
		if (Wabs(pos.x - tee.pos.x) < g_EPSILON &&
			Wabs(pos.z - tee.GetTeeShotPos().z) < g_EPSILON)
		{
			return 0;
		}
	}
	GetMeshList(pos);
	int index;
	pos = ColTest(pos, true, &index, true);
	if (index < 0)
	{
		return 10;
	}
	bool bInWater = false;
	for (unsigned int i = 0; i < m_water.size(); i++)
	{
		sWater& w = m_water[i];
		if (pos.x > w.aabb.min.x && pos.x < w.aabb.max.x &&
			pos.z > w.aabb.min.z && pos.z < w.aabb.max.z &&
			pos.y < w.aabb.max.y)
		{
			if (water != NULL)
			{
				*water = (CGolfBall::eWater)w.type;
			}
			bInWater = true;
		}
	}
	if (bInWater)
	{
		return 10;
	}
	return m_pPolySoup->GetProperty(m_pHolder->tri[index].propIndex).ground;
}

bool CQuadTree::IsOBArea(const WVector& pos)
{
	int index = GetMeshIndex(pos, false);
	if (index < 0)
	{
		return true;
	}
	bool ob = (m_pPolySoup->GetProperty(m_pHolder->tri[index].propIndex).pass &
				  2) != 0;
	return ob;
}

bool CQuadTree::IsPenetrable(int propIndex) const
{
	bool penetrable = (m_pPolySoup->GetProperty(propIndex).pass & 1) != 0;
	return penetrable;
}

float CQuadTree::GetMaxPowerOnGround() const
{
	sPlayerData* pPlayer;
	int propIndex;
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe)) // 983 00956200: evento 0xe (o 645 testava o item 0x18000028)
	{
		pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		propIndex = 1;
	}
	else
	{
		pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		propIndex = m_pHolder->tri[GolfBall().m_meshIndex].propIndex;
	}
	const sProperty* pProperty = &m_pPolySoup->GetProperty(propIndex);
	return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 0x14) +
			   m_condition) *
		0.01f;
}

float CQuadTree::GetMinPowerOnGround()
{
	sPlayerData* pPlayer;
	int propIndex;
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe)) // 983 00956340: evento 0xe (o 645 testava o item 0x18000028)
	{
		pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		propIndex = 1;
	}
	else
	{
		pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		propIndex = m_pHolder->tri[GolfBall().m_meshIndex].propIndex;
	}
	const sProperty* pProperty = &m_pPolySoup->GetProperty(propIndex);
	return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 8) +
			   m_condition) *
		0.01f;
}

float CQuadTree::GetMaxPowerOnGround(const WVector& pos)
{
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe)) // 983 0095ef70: evento 0xe (o 645 testava o item 0x18000028)
	{
		sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		const sProperty* pProperty = &m_pPolySoup->GetProperty(1);
		return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 0x14) +
				   m_condition) *
			0.01f;
	}
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
	int index = GetMeshIndex(pos, false);
	const sProperty* pProperty =
		&m_pPolySoup->GetProperty(m_pHolder->tri[index].propIndex);
	return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 0x14) +
			   m_condition) *
		0.01f;
}

float CQuadTree::GetMinPowerOnGround(const WVector& pos)
{
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe)) // 983 0095f050: evento 0xe (o 645 testava o item 0x18000028)
	{
		sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		const sProperty* pProperty = &m_pPolySoup->GetProperty(1);
		return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 8) +
				   m_condition) *
			0.01f;
	}
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
	int index = GetMeshIndex(pos, false);
	const sProperty* pProperty =
		&m_pPolySoup->GetProperty(m_pHolder->tri[index].propIndex);
	return (PropertyPower(pProperty, PlayerPowerIndex(pPlayer), 8) +
			   m_condition) *
		0.01f;
}

const char* CQuadTree::GetBounceSound(int propIndex) const
{
	return m_pPolySoup->GetProperty(propIndex).boundSound;
}

const char* CQuadTree::GetRollSound(int propIndex) const
{
	return m_pPolySoup->GetProperty(propIndex).rollSound;
}

const char* CQuadTree::GetStepSound(int propIndex) const
{
	return m_pPolySoup->GetProperty(propIndex).stepSound;
}

const char* CQuadTree::GetStepSound(const WVector& pos)
{
	int propIndex = m_pHolder->tri[GetMeshIndex(pos, false)].propIndex;
	return m_pPolySoup->GetProperty(propIndex).stepSound;
}

const char* CQuadTree::FindTexture(int texHandle)
{
	return PolyTexNames(m_pPolySoup) +
		PolyTexMap(m_pPolySoup)[texHandle] * 0x28;
}

void CQuadTree::UpdateProperty()
{
	IFF_STRUCT::sCourse* course = ItemManager()->FindCourse(
		((GetCurMap() > 127 && GetCurMap() != 253) ? GetCurMap() - 128
												   : GetCurMap()) |
		0x28000000);

	if (course && course->TexProp[0])
	{
		PolyDoc(m_pPolySoup)
			->LoadXml(course->TexProp, GOLFDOC()->m_currentHole - 1, true);
	}
	else
	{
		PolyDoc(m_pPolySoup)
			->LoadXml(MakeStr("%s_property.xml",
						  Doc()->m_golfGame.pCourse
							  ? Doc()->m_golfGame.pCourse->Data
							  : NULL),
				GOLFDOC()->m_currentHole - 1, true);
	}

	for (int i = 0; i < PolyTextureNum(m_pPolySoup); i++)
	{
		PolyTextures(m_pPolySoup)[i].propIndex =
			m_pPolySoup->GetPropertyIndex(PolyTextures(m_pPolySoup)[i].name);
	}
}

void CQuadTree::ResetWater()
{
	m_water.clear();
	m_water.reserve(0x20);
}

struct Str8
{
	char b[8];
};

void CQuadTree::HoleIn()
{
	CGolfBall& ball = GolfBall();
	CGolfBall::sExpected* pExpected = ball.PreDataToWrite(ball.m_renderNum);
	ball.m_vel.z = 0.0f;
	ball.m_vel.y = 0.0f;
	ball.m_vel.x = 0.0f;
	ball.m_state = (CGolfBall::eBallState)0;
	ball.SetHoleIn(1);
	pExpected->event = 3;
	pExpected->effect = 0;
	if (SharedCurPlayerItem() != 0x14000005 &&
		SharedCurPlayerItem() != 0x14000080)
	{
		*reinterpret_cast<Str8*>(pExpected->sound) =
			*reinterpret_cast<const Str8*>("\xb0\xf8_\xc8\xa6\xc0\xce");
	}
	else
	{
		*reinterpret_cast<Str10*>(pExpected->sound) =
			*reinterpret_cast<const Str10*>("wb_holein");
	}
}

WVector CQuadTree::GetCollisionPoint(const WVector& dir)
{
	WVector offset = GolfBall().GetRadius() * dir;
	WVector point;
	point = GolfBall().m_pos - offset;
	return point;
}

bool g_rastroNoPlay983 = false; // B1504 temporario: ligado so no SearchGround do jogador

bool CQuadTree::OverlappedWithObject(WVector pos, float radius)
{
	unsigned char n = 0;
	if (m_objNum != 0)
	{
		do
		{
			unsigned int idx = n;
			sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[idx]];
			if ((pHolder->noCollision & 1) == 0)
			{
				int num = 0;
				w_bound_box* box = pHolder->pet->m_bbList.Find("char0");
				if (box == NULL)
				{
					if (pos.y <= pHolder->bbox.max.y &&
						pHolder->bbox.min.y - 10.0f <= pos.y)
					{
						Waabb area;
						area.min.y = pHolder->bbox.min.y;
						area.min.x = pHolder->bbox.min.x - radius;
						area.max.x = pHolder->bbox.max.x;
						area.min.z = pHolder->bbox.min.z - radius;
						area.max.y = pHolder->bbox.max.y;
						area.max.z = radius + pHolder->bbox.max.z;
						if (area.min.x < pos.x && pos.x < radius + area.max.x &&
							area.min.z < pos.z && pos.z < area.max.z)
						{
							if (g_rastroNoPlay983) // B1504 temporario: objeto que deixou a bola em NoPlay
								jp983_rastro("B1504 NoPlay aabb obj=%d nc=%d min=(%.1f,%.1f,%.1f) max=(%.1f,%.1f,%.1f)", (int)m_objList[idx],
									(int)pHolder->noCollision, pHolder->bbox.min.x, pHolder->bbox.min.y, pHolder->bbox.min.z,
									pHolder->bbox.max.x, pHolder->bbox.max.y, pHolder->bbox.max.z);
							return true;
						}
					}
				}
				else
				{
					do
					{
						WBone* bone = box->bone;
						w_bound_box_info* info = box->info;
						Wobb obb;
						float hx =
							(info->aabb.max.x - info->aabb.min.x) * 0.5f +
							WVector::ONE.x * radius;
						float hy =
							(info->aabb.max.y - info->aabb.min.y) * 0.5f +
							WVector::ONE.y * radius;
						float hz =
							(info->aabb.max.z - info->aabb.min.z) * 0.5f +
							WVector::ONE.z * radius;
						obb.extend[0].x = bone->GetMatrix().xx * hx;
						obb.extend[0].y = bone->GetMatrix().yx * hx;
						obb.extend[0].z = bone->GetMatrix().zx * hx;
						obb.extend[1].x = bone->GetMatrix().xy * hy;
						obb.extend[1].y = bone->GetMatrix().yy * hy;
						obb.extend[1].z = bone->GetMatrix().zy * hy;
						obb.extend[2].x = bone->GetMatrix().xz * hz;
						obb.extend[2].y = bone->GetMatrix().yz * hz;
						obb.extend[2].z = bone->GetMatrix().zz * hz;
						obb.center.x =
							bone->GetMatrix().xx * info->spherePivot.x +
							bone->GetMatrix().xz * info->spherePivot.z +
							bone->GetMatrix().xy * info->spherePivot.y +
							bone->GetMatrix().xm;
						obb.center.y =
							bone->GetMatrix().yz * info->spherePivot.z +
							bone->GetMatrix().yx * info->spherePivot.x +
							bone->GetMatrix().yy * info->spherePivot.y +
							bone->GetMatrix().ym;
						obb.center.z =
							bone->GetMatrix().zz * info->spherePivot.z +
							bone->GetMatrix().zx * info->spherePivot.x +
							bone->GetMatrix().zy * info->spherePivot.y +
							bone->GetMatrix().zm;
						if (obb.IsInclude(pos))
						{
							if (g_rastroNoPlay983) // B1504 temporario
								jp983_rastro("B1504 NoPlay obb obj=%d char%d c=(%.1f,%.1f,%.1f)", (int)m_objList[idx], num, obb.center.x,
									obb.center.y, obb.center.z);
							return true;
						}
						num++;
						box = PolyObjs(m_pPolySoup)[m_objList[idx]]->pet->m_bbList.Find(MakeStr("char%d", num));
					} while (box != NULL);
				}
			}
			n = (unsigned char)(n + 1);
		} while (n < m_objNum);
	}
	return false;
}

void CQuadTree::UpdateVtxColor(float dt)
{
	if (g_input->GetDown("\xc5\xd7\xbd\xba\xc6\xae", true))
	{
		m_vtxColorMode = (m_vtxColorMode + 1) % 2;
	}
	WVector color;
	switch (m_vtxColorMode)
	{
	case 0:
		color = m_baseColor;
		break;
	case 1:
		color = m_objColor;
		break;
	}
	int channel[3] = { 0, 0, 0 };
	if (g_input->Get('Q'))
	{
		channel[0] = 1;
	}
	if (g_input->Get('W'))
	{
		channel[1] = 1;
	}
	if (g_input->Get('E'))
	{
		channel[2] = 1;
	}
	for (int i = 0; i < 3; i++)
	{
		if (channel[i])
		{
			if (g_input->Get('='))
			{
				color.p[i] = Between(0.0f, dt * 64.0f + color.p[i], 255.0f);
			}
			if (g_input->Get('-'))
			{
				color.p[i] = Between(0.0f, color.p[i] - dt * 64.0f, 255.0f);
			}
		}
	}
	switch (m_vtxColorMode)
	{
	case 0:
		if (!WisEqual(m_baseColor, color, 0.00001f))
		{
			SetBaseVtxColor(0xff000000 | ((unsigned char)color.x << 16) |
				((unsigned char)color.y << 8) | (unsigned char)color.z);
		}
		break;
	case 1:
		if (!WisEqual(m_objColor, color, 0.00001f))
		{
			SetObjVtxColor(0xff000000 | ((unsigned char)color.x << 16) |
				((unsigned char)color.y << 8) | (unsigned char)color.z);
		}
		break;
	}
}

void CQuadTree::SetBaseVtxColor(unsigned long color)
{
	m_baseColor.x = (float)((color >> 16) & 0xff);
	m_baseColor.y = (float)((color >> 8) & 0xff);
	m_baseColor.z = (float)(color & 0xff);
	if (m_pHolder != NULL && m_pHolder->pet != NULL)
	{
		m_pHolder->pet->SetVtxColor(color);
		m_pHolder->pet->UpdateLightSource(&g_lightset, false, NULL);
	}
}

void CQuadTree::SetObjVtxColor(unsigned long color)
{
	m_objColor.x = (float)((color >> 16) & 0xff);
	m_objColor.y = (float)((color >> 8) & 0xff);
	m_objColor.z = (float)(color & 0xff);
	if (m_pPolySoup != NULL)
	{
		for (int i = 0; i < PolyObjNum(m_pPolySoup); i++)
		{
			sQuadPuppetHolder** ppObj = &PolyObjs(m_pPolySoup)[i];
			if ((*ppObj)->pet != NULL)
			{
				(*ppObj)->pet->SetVtxColor(color);
				ppObj = &PolyObjs(m_pPolySoup)[i];
				(*ppObj)->pet->UpdateLightSource(&g_lightset, false, NULL);
			}
		}
	}
}

WVector CQuadTree::DistanceBetweenRayNSegment(const WVector& rayPos,
	const WVector& rayDir, const WVector& segPos, const WVector& segDir)
{
	WVector w = rayPos - segPos;
	float a = rayDir * rayDir;
	float b = rayDir * segDir;
	float c = segDir * segDir;
	float d = w * rayDir;
	float e = w * segDir;
	float D = a * c - b * b;
	float sN, sD = D;
	float tN, tD = D;

	if (Wabs(D) < g_EPSILON)
	{
		sN = 0.0f;
		tN = e;
	}
	else
	{
		sN = b * e - c * d;
		tN = a * e - b * d;
	}

	if (sN < 0.0f)
	{
		sN = 0.0f;
		tN = e;
		tD = c;
	}

	if (tN < 0.0f)
	{
		tN = 0.0f;
		if (-d < 0.0f)
			sN = 0.0f;
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD)
	{
		tN = tD;
		if (b - d < 0.0f)
			sN = 0.0f;
		else
		{
			sN = b - d;
			sD = a;
		}
	}

	float sc = Wabs(sN) < g_EPSILON ? 0.0f : sN / sD;
	float tc = Wabs(tN) < g_EPSILON ? 0.0f : tN / tD;
	WVector dP = (sc * rayDir + rayPos) - (tc * segDir + segPos);
	return WVector(sc, tc, dP.Magnitude());
}

bool CQuadTree::SimpleColTest(const WVector& start, const WVector& ray)
{
	GetObjList(start, ray, 0);
	for (unsigned char i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder** ppObj = &PolyObjs(m_pPolySoup)[m_objList[i]];
		if (RayIntersectAABB((*ppObj)->aabb, start, ray, NULL, 0.0f) < 1.0f)
		{
			return true;
		}
	}
	return false;
}

float CQuadTree::RayIntersectAll(const WVector& start, const WVector& ray,
	bool bFlag)
{
	float best = 1.0f;
	GetObjList(start, ray, 0);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		// 983 00951fa1: noCollision&2 fica sem tri (MakeObjStructure) e a mascara 0 deixa ele na lista
		if (pHolder->pet != NULL && !pHolder->tri.empty())
		{
			int triNum = pHolder->pet->GetFaceNum();
			for (int j = 0; j < triNum; j++)
			{
				WPolySoup::sTriangle* tri = &pHolder->tri[j];
				if (bFlag ||
					(m_pPolySoup->GetProperty(tri->propIndex).pass & 1) == 0)
				{
					WVector a = tri->v[0];
					WVector b = tri->v[1];
					WVector c = tri->v[2];
					WPlane plane = tri->plane;
					if (tri->bDoubleSide &&
						plane.x * start.x + plane.z * start.z +
								plane.y * start.y + plane.dis <
							0.0f)
					{
						plane.x = -plane.x;
						plane.y = -plane.y;
						plane.z = -plane.z;
						plane.dis = plane.dis * -1.0f;
					}
					float t =
						intersection::IntersectPlane(start, ray, plane, false);
					if (t < best)
					{
						WVector hit;
						hit.x = t * ray.x + start.x;
						hit.y = t * ray.y + start.y;
						hit.z = t * ray.z + start.z;
						if (intersection::SameSide(hit, a, b, c) &&
							intersection::SameSide(hit, b, a, c) &&
							intersection::SameSide(hit, c, a, b))
						{
							best = t;
						}
					}
				}
			}
		}
	}
	GetMeshList(start, ray);
	for (int k = 0; k < m_meshNum; k++)
	{
		int idx = m_meshList[k];
		WPolySoup::sTriangle* tris = &m_pHolder->tri[0];
		if (bFlag ||
			(m_pPolySoup->GetProperty(tris[idx].propIndex).pass & 1) == 0)
		{
			WPlane plane = tris[idx].plane;
			if (tris[idx].bDoubleSide &&
				plane.x * start.x + plane.z * start.z + plane.y * start.y +
						plane.dis <
					0.0f)
			{
				plane.x = -plane.x;
				plane.y = -plane.y;
				plane.z = -plane.z;
				plane.dis = plane.dis * -1.0f;
			}
			float t = intersection::IntersectPlane(start, ray, plane, false);
			if (t < best)
			{
				WVector a = tris[idx].v[0];
				WVector b = tris[idx].v[1];
				WVector c = tris[idx].v[2];
				WVector hit;
				hit.x = t * ray.x + start.x;
				hit.y = t * ray.y + start.y;
				hit.z = t * ray.z + start.z;
				if (intersection::SameSide(hit, a, b, c) &&
					intersection::SameSide(hit, b, a, c) &&
					intersection::SameSide(hit, c, a, b))
				{
					best = t;
				}
			}
		}
	}
	return best;
}

void CQuadTree::ClipDecalPolygon(WVector* pVerts, int num, int plane)
{
	WVector newVerts[1024];
	float dists[1024];
	char sides[1024];
	if (num >= 0x3ff)
	{
		return;
	}
	if (plane == 6)
	{
		if (num > 2)
		{
			if (m_decalVertNum + num >= m_decalVertMax)
			{
				num = m_decalVertMax - m_decalVertNum;
			}
			WVector* pDst = &m_pDecalVerts[m_decalVertNum];
			for (int i = 0; i < num; i++)
			{
				pDst[i] = pVerts[i];
			}
			m_decalVertNum += num;
		}
		return;
	}
	const WPlane& clip = m_decalPlanes[plane];
	bool bFront = false;
	int i;
	for (i = 0; i < num; i++)
	{
		float dist = pVerts[i] * clip.normal - clip.d;
		if (dist > 0.03125f)
		{
			bFront = true;
			sides[i] = 0;
		}
		else if (dist < -0.03125f)
		{
			sides[i] = 1;
		}
		else
		{
			sides[i] = 2;
		}
		dists[i] = dist;
	}
	if (!bFront)
	{
		return;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	pVerts[i] = pVerts[0];
	int newNum = 0;
	for (i = 0; i < num; i++)
	{
		if (sides[i] == 0 || sides[i] == 2)
		{
			newVerts[newNum] = pVerts[i];
			newNum++;
		}
		if (sides[i] == 2 || sides[i + 1] == 2 || sides[i] == sides[i + 1])
		{
			continue;
		}
		float frac = dists[i] / (dists[i] - dists[i + 1]);
		WVector delta = pVerts[i + 1] - pVerts[i];
		newVerts[newNum] = delta * frac + pVerts[i];
		newNum++;
	}
	ClipDecalPolygon(newVerts, newNum, plane + 1);
}

void CQuadTree::ClipDecalTriangle(const WPolySoup::sTriangle* pTri,
	WVector normal)
{
	if (!pTri->bDoubleSide && normal * pTri->plane.normal < 0.01f)
	{
		return;
	}
	WVector verts[4];
	memcpy(verts, pTri->v, sizeof(pTri->v));
	ClipDecalPolygon(verts, 3, 0);
}

static int s_fragmentsCacheIndex = 0;
static fragmentsCache_s s_fragmentsCache[32];

static fragmentsCache_s* FindDecalFragments(const WVector* box,
	const WVector& normal)
{
	int last = s_fragmentsCacheIndex - 1;
	if (last < 0)
	{
		last = 31;
	}
	int i = s_fragmentsCacheIndex;
	while (i != last)
	{
		fragmentsCache_s* pCache = &s_fragmentsCache[i];
		if (pCache->bUsed && WisEqual(pCache->box[0], box[0], 0.15f) &&
			WisEqual(pCache->box[1], box[1], 0.15f) &&
			WisEqual(pCache->box[2], box[2], 0.15f) &&
			WisEqual(pCache->box[3], box[3], 0.15f) &&
			WisEqual(pCache->normal, normal, 0.15f))
		{
			return pCache;
		}
		i++;
		if (i == 32)
		{
			i = 0;
		}
	}
	return NULL;
}

static void SaveDecalFragments(int numVertices, const WVector* box,
	const WVector& normal, const WVector* pVerts, int numIndices,
	const unsigned short* pIndices)
{
	fragmentsCache_s* pCache = &s_fragmentsCache[s_fragmentsCacheIndex++];
	if (s_fragmentsCacheIndex == 32)
	{
		s_fragmentsCacheIndex = 0;
	}
	pCache->bUsed = true;
	pCache->box[0] = box[0];
	pCache->box[1] = box[1];
	pCache->box[2] = box[2];
	pCache->box[3] = box[3];
	pCache->normal = normal;
	pCache->numVertices = numVertices;
	memcpy(pCache->vertices, pVerts, numVertices * sizeof(WVector));
	pCache->numIndices = numIndices;
	memcpy(pCache->indices, pIndices, numIndices * sizeof(unsigned short));
}

void CQuadTree::DecalFragments(const WVector* box, const WVector& normal,
	WVector* pVerts, int* pVertNum, int maxVerts, unsigned short* pIndices,
	int* pIndexNum, int maxIndices, bool bAll)
{
	Waabb aabb;
	fragmentsCache_s* pCache = FindDecalFragments(box, normal);
	if (pCache != NULL)
	{
		memcpy(pVerts, pCache->vertices, pCache->numVertices * sizeof(WVector));
		memcpy(pIndices, pCache->indices,
			pCache->numIndices * sizeof(unsigned short));
		*pVertNum = pCache->numVertices;
		*pIndexNum = pCache->numIndices;
		return;
	}
	int i;
	WVector center(0.0f, 0.0f, 0.0f);
	for (i = 0; i < 4; i++)
	{
		center += box[i];
	}
	center *= 0.25f;
	GetMeshList(center);
	int index;
	center = ColTest(center, false, &index, true);
	if (center.y < m_pHolder->aabb.min.y)
	{
		center.y = m_pHolder->aabb.min.y;
	}
	WVector dir = -normal;
	dir.Normalize();
	WVector axisU = box[0] - box[1];
	axisU.Normalize();
	WVector axisV = WCrossProduct(dir, axisU);
	axisV.Normalize();
	float halfU = (box[1] - box[0]).Magnitude() * 0.5f;
	float halfV = (box[2] - box[1]).Magnitude() * 0.5f;
	float halfMax = Max(halfU, halfV);
	float dot = center * axisU;
	m_decalPlanes[0].normal = axisU;
	m_decalPlanes[0].d = dot - halfU;
	m_decalPlanes[1].normal = -axisU;
	m_decalPlanes[1].d = -dot - halfU;
	dot = center * axisV;
	m_decalPlanes[2].normal = axisV;
	m_decalPlanes[2].d = dot - halfV;
	m_decalPlanes[3].normal = -axisV;
	m_decalPlanes[3].d = -dot - halfV;
	dot = center * dir;
	m_decalPlanes[4].normal = dir;
	m_decalPlanes[4].d = dot - halfMax;
	m_decalPlanes[5].normal = -dir;
	m_decalPlanes[5].d = -dot - halfMax;
	aabb.Clear();
	for (i = 0; i < 4; i++)
	{
		aabb.AddPoint(box[i]);
	}
	aabb.min.y -= halfMax;
	aabb.max.y += halfMax;
	GetMeshList(aabb);
	m_pDecalVerts = pVerts;
	m_decalVertNum = 0;
	m_decalVertMax = maxVerts;
	int indexNum = 0;
	for (i = 0; i < m_meshNum; i++)
	{
		int first = m_decalVertNum;
		ClipDecalTriangle(&m_pHolder->tri[m_meshList[i]], dir);
		int num = m_decalVertNum - first - 2;
		int j;
		for (j = 0; j < num; j++)
		{
			if (indexNum + 3 > maxIndices)
			{
				break;
			}
			pIndices[indexNum + j * 3] = (unsigned short)first;
			pIndices[indexNum + j * 3 + 1] = (unsigned short)(first + j + 1);
			pIndices[indexNum + j * 3 + 2] = (unsigned short)(first + j + 2);
		}
		indexNum += j * 3;
	}
	if (bAll)
	{
		GetObjList(aabb, 2);
		for (i = 0; i < m_objNum; i++)
		{
			WPolySoup::sModel* pModel = PolyObjs(m_pPolySoup)[m_objList[i]];
			for (int k = 0; k < PuppetFaceNum(pModel->pet); k++)
			{
				int first = m_decalVertNum;
				ClipDecalTriangle(&pModel->tri[k], dir);
				int num = m_decalVertNum - first - 2;
				int j;
				for (j = 0; j < num; j++)
				{
					if (indexNum + 3 > maxIndices)
					{
						break;
					}
					pIndices[indexNum + j * 3] = (unsigned short)first;
					pIndices[indexNum + j * 3 + 1] =
						(unsigned short)(first + j + 1);
					pIndices[indexNum + j * 3 + 2] =
						(unsigned short)(first + j + 2);
				}
				indexNum += j * 3;
			}
		}
	}
	SaveDecalFragments(m_decalVertNum, box, normal, pVerts, indexNum, pIndices);
	*pVertNum = m_decalVertNum;
	*pIndexNum = indexNum;
}

void CQuadTree::DrawDecal(int texHandle, const WVector& pos,
	const WVector& dirX, const WVector& dirZ, float width, float height,
	enumPolyBlend blend, bool bFlag)
{
	static WVector vertices[1024];
	static unsigned short indices[1536];
	WTVertex tvs[1024];
	int numVerts;
	int numIndices;
	WVector n = dirX;
	n.Normalize();
	WVector t = dirZ;
	t.Normalize();
	WVector b = WCrossProduct(n, t);
	b.Normalize();
	WVector box[4];
	box[0] = pos - dirX - dirZ;
	box[1] = dirX + pos - dirZ;
	box[2] = dirX + pos + dirZ;
	box[3] = pos - dirX + dirZ;
	DecalFragments(box, b, vertices, &numVerts, 0x400, indices, &numIndices,
		0x600, bFlag);
	float lenN = dirX.Magnitude();
	float lenT = dirZ.Magnitude();
	float half = (lenT + lenN) * 0.5f;
	float scaleU = width / (lenN + lenN);
	float scaleV = width / (lenT + lenT);
	for (int i = 0; i < numVerts; i++)
	{
		tvs[i].x = vertices[i].x;
		tvs[i].y = vertices[i].y;
		tvs[i].z = vertices[i].z;
		WVector d = vertices[i] - pos;
		tvs[i].tu = (d * n) * scaleU + 0.5f;
		tvs[i].tv = 0.5f - (d * t) * scaleV;
		float fade = Min(Wabs(pos.y - vertices[i].y), half);
		tvs[i].diffuse = ((unsigned long)(unsigned char)((half - fade) *
							  (1.0f / half) * height * 255.0f)
							 << 24) |
			0xffffff;
	}
	// 983 00957340: bits de face do flag1 desceram (645 0x400 -> 983 0x200; 0x400 do 983 = face oposta,
	// o decal sumia: sombra do Lounge, D651) e o flag0 ganhou 0x4000000; + 1 coordenada de textura
	g_view->DrawIndexedTriangles(tvs, numVerts, indices, numIndices,
		(texHandle & 0x7ff) | blend | 0x26200000, 0x200, 1, 0);
}

void CQuadTree::TraceAABB(const WVector& start, const WVector& end,
	const Waabb& box, trace_s* pTrace)
{
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	Waabb startBox;
	Waabb endBox;
	Waabb bound;
	if (WisEqual(start, end, 0.00001f))
	{
		pTrace->m_endPos = end;
		return;
	}
	startBox = box + start;
	endBox = box + end;
	bound = ExpandAABB(startBox, endBox);
	bound.min -= WVector::ONE;
	bound.max += WVector::ONE;
	WVector delta = end - start;
	float fraction = 1.0f;
	WPlane plane;
	WPlane hitPlane;
	GetObjList(bound, 1);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		if ((pHolder->noCollision & 1) == 0)
		{
			int num = 0;
			if (pHolder->pet->m_bbList.Find("char1") != NULL)
			{
				w_bound_box* pBox = pHolder->pet->m_bbList.Find("char0");
				while (pBox != NULL)
				{
					num++;
					float t = AabbIntersect(startBox, delta,
						pBox->info->aabb + pHolder->pet->GetMatrix().pivot,
						&plane);
					if (t < fraction)
					{
						fraction = t;
						hitPlane = plane;
					}
					pBox = pHolder->pet->m_bbList.Find(MakeStr("char%d", num));
				}
			}
			else
			{
				float t = AabbIntersect(startBox, delta, pHolder->bbox, &plane);
				if (t < fraction)
				{
					fraction = t;
					hitPlane = plane;
				}
			}
		}
	}
	if (fraction != 1.0f)
	{
		pTrace->m_endPos = start + WVector::UNIT_POS_Y + delta * fraction;
	}
	else
	{
		pTrace->m_endPos = end;
	}
	pTrace->m_fraction = fraction;
	pTrace->m_normal.x = hitPlane.x;
	pTrace->m_normal.y = hitPlane.y;
	pTrace->m_normal.z = hitPlane.z;
	pTrace->m_planeDist = hitPlane.dis;
	pTrace->m_flag = false;
}

void CQuadTree::TraceOBB(const WVector& start, const WVector& end,
	const Waabb& box, trace_s* pTrace)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	Waabb startBox;
	Waabb endBox;
	Waabb bound;
	if (WisEqual(start, end, 0.00001f))
	{
		pTrace->m_endPos = end;
		return;
	}
	startBox = box + start;
	endBox = box + end;
	bound = ExpandAABB(startBox, endBox);
	bound.min -= WVector::ONE;
	bound.max += WVector::ONE;
	WVector delta = end - start;
	float fraction = 1.0f;
	WPlane plane;
	WPlane hitPlane;
	GetObjList(bound, 1);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		if ((pHolder->noCollision & 1) == 0)
		{
			int num = 0;
			if (pHolder->pet->m_bbList.Find("char0") != NULL)
			{
				w_bound_box* pBox = pHolder->pet->m_bbList.Find("char0");
				while (pBox != NULL)
				{
					num++;
					Wobb obb;
					WVector half =
						(pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;
					obb.extend[0] = pBox->bone->GetMatrix().xa * half.x;
					obb.extend[1] = pBox->bone->GetMatrix().ya * half.y;
					obb.extend[2] = pBox->bone->GetMatrix().za * half.z;
					obb.center =
						pBox->info->spherePivot * pBox->bone->GetMatrix();
					float t = ObbIntersect(startBox, delta, obb, &plane);
					if (t < fraction)
					{
						fraction = t;
						hitPlane = plane;
					}
					pBox = pHolder->pet->m_bbList.Find(MakeStr("char%d", num));
				}
			}
		}
	}
	if (fraction != 1.0f)
	{
		pTrace->m_endPos = start + WVector::UNIT_POS_Y + delta * fraction;
	}
	else
	{
		pTrace->m_endPos = end;
	}
	pTrace->m_fraction = fraction;
	pTrace->m_normal.x = hitPlane.x;
	pTrace->m_normal.y = hitPlane.y;
	pTrace->m_normal.z = hitPlane.z;
	pTrace->m_planeDist = hitPlane.dis;
	pTrace->m_flag = false;
}

void CQuadTree::DrawDebugTraceOBB(const WVector& start, const WVector& end,
	const Waabb& box)
{
	Waabb startBox = box + start;
	Waabb endBox = box + end;
	Waabb bound = ExpandAABB(startBox, endBox);
	bound.min -= WVector::ONE;
	bound.max += WVector::ONE;
	GetObjList(bound, 1);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		if ((pHolder->noCollision & 1) == 0 && pHolder->pet != NULL)
		{
			int num = 0;
			w_bound_box* pBox = pHolder->pet->m_bbList.Find("char0");
			while (pBox != NULL)
			{
				num++;
				Wobb obb;
				WVector half =
					(pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;
				obb.extend[0] = pBox->bone->GetMatrix().xa * half.x;
				obb.extend[1] = pBox->bone->GetMatrix().ya * half.y;
				obb.extend[2] = pBox->bone->GetMatrix().za * half.z;
				obb.center = pBox->info->spherePivot * pBox->bone->GetMatrix();
				g_view->DrawOBB(obb, 0xff00ff00);
				pBox = pHolder->pet->m_bbList.Find(MakeStr("char%d", num));
			}
		}
	}
}

void CQuadTree::DrawDebugTraceAABB(const WVector& start, const WVector& end,
	const Waabb& box)
{
	Waabb startBox = box + start;
	Waabb endBox = box + end;
	Waabb bound = ExpandAABB(startBox, endBox);
	bound.min -= WVector::ONE;
	bound.max += WVector::ONE;
	GetObjList(bound, 1);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		if ((pHolder->noCollision & 1) == 0 && pHolder->pet != NULL)
		{
			int num = 0;
			if (pHolder->pet->m_bbList.Find("char1") != NULL)
			{
				w_bound_box* pBox = pHolder->pet->m_bbList.Find("char0");
				while (pBox != NULL)
				{
					num++;
					g_view->DrawAABB(pBox->info->aabb +
							pHolder->pet->GetMatrix().pivot,
						0xffff0000, false, 0);
					pBox = pHolder->pet->m_bbList.Find(MakeStr("char%d", num));
				}
			}
			else
			{
				g_view->DrawAABB(pHolder->bbox, 0xffff0000, false, 0);
			}
		}
	}
}

int CQuadTree::GetObjColNum()
{
	int num = 0;
	std::vector<sColData>::iterator it;
	for (it = m_colList.begin(); it != m_colList.end(); ++it)
	{
		if ((*it).objIndex >= 0)
		{
			num++;
		}
	}
	return num;
}

bool CQuadTree::IsFallenIntoWater(WVector pos, Waabb* pAabb, bool bKeepWater)
{
	bool bFallen = false;
	for (unsigned int i = 0; i < m_water.size(); i++)
	{
		sWater& water = m_water[i];
		if (water.aabb.min.x < pos.x && pos.x < water.aabb.max.x &&
			water.aabb.min.z < pos.z && pos.z < water.aabb.max.z &&
			pos.y < water.aabb.max.y)
		{
			if (pAabb != NULL)
			{
				*pAabb = water.aabb;
			}
			if (!bKeepWater)
			{
				GolfBall().m_water = (CGolfBall::eWater)water.type;
			}
			bFallen = true;
		}
	}
	return bFallen;
}

bool CQuadTree::IsOverWater(WVector pos)
{
	GetMeshList(pos);
	for (unsigned int i = 0; i < m_water.size(); i++)
	{
		sWater& water = m_water[i];
		pos.y = (water.aabb.min.y + water.aabb.max.y) * 0.5f;
		if (water.aabb.min.x <= pos.x && pos.x <= water.aabb.max.x &&
			water.aabb.min.y <= pos.y && pos.y <= water.aabb.max.y &&
			water.aabb.min.z <= pos.z && pos.z <= water.aabb.max.z)
		{
			WVector col = ColTest(pos, true, NULL, true);
			if (col.y < water.aabb.max.y)
			{
				return true;
			}
		}
	}
	return false;
}

float CQuadTree::GetShadowIntensity(const WVector& pos)
{
	if (!COption::Instance()->vIsShadowmapEnabled() || !SharedCameraFlag())
	{
		return 1.0f;
	}
	WVector lightDir = g_lightset.nearOne;
	lightDir.y = 0.0f;
	lightDir.Normalize();
	lightDir *= -320.0f;
	GetObjList(pos, lightDir, 0);	// 983 00958bb0
	float result = 1.0f;
	Waabb box;
	unsigned char ambient = (unsigned char)(g_lightset.ambient >> 17);
	for (int i = 0; i < m_objNum; i++)
	{
		WPolySoup::sModel* pModel;
		if (PolyObjNum(m_pPolySoup) != PolyStructuredObjNum(m_pPolySoup))
		{
			pModel = NULL;
			for (int j = 0; j < PolyObjNum(m_pPolySoup); j++)
			{
				if (ModelObjIndex(PolyObjs(m_pPolySoup)[j]) == m_objList[j])
				{
					pModel = PolyObjs(m_pPolySoup)[j];
				}
			}
		}
		else
		{
			pModel = PolyObjs(m_pPolySoup)[m_objList[i]];
		}
		if (!pModel)
		{
			continue;
		}
		if (pModel->aabb.max.y - pModel->aabb.min.y < 16.0f)
		{
			continue;
		}
		if (pModel->aabb.IsInclude(pos))
		{
			WVector d = pos - pModel->pos;
			if (d.x * lightDir.x + d.y * lightDir.y + d.z * lightDir.z < 0.0f)
			{
				return (float)ambient;
			}
		}
		WVector ext;
		ext = pModel->aabb.max - pModel->pos;
		box.min = WVector(pModel->pos.x - ext.x * 0.75f, pModel->pos.y - ext.y,
			pModel->pos.z - ext.z * 0.75f);
		box.max = WVector(ext.x * 0.75f + pModel->pos.x,
			ext.y * 0.5f + pModel->pos.y, ext.z * 0.75f + pModel->pos.z);
		WVector ray(g_lightset.nearOne.x * -320.0f,
			g_lightset.nearOne.y * -320.0f, g_lightset.nearOne.z * -320.0f);
		float t = RayIntersectAABBSolid(box, pos, ray, NULL, 0.0f);
		if (t < result)
		{
			result = t;
		}
	}
	return result;
}

WVector CQuadTree::GetRayIntersection(WVector start, WVector ray)
{
	ray.Normalize();
	ray.x = ray.x * 1.6f;
	float t = 1.0f;
	ray.y = ray.y * 1.6f;
	ray.z = ray.z * 1.6f;
	for (int step = 0; step < 0x5dc; step++)
	{
		WVector next(start.x + ray.x, start.y + ray.y, start.z + ray.z);
		GetMeshList(next);
		int index = -1;
		WVector col = ColTest(next, true, &index, true);
		float groundY = col.y;
		if (index == -1)
		{
			groundY = m_pHolder->aabb.min.y;
		}
		if (groundY < m_pHolder->aabb.min.y)
		{
			groundY = m_pHolder->aabb.min.y;
		}
		if (next.y < groundY)
		{
			GetMeshList(start, ray);
			if (m_meshNum > 0)
			{
				for (int i = 0; i < m_meshNum; i++)
				{
					float d = intersection::IntersectRayTriangle(start, ray,
						m_pHolder->tri[m_meshList[i]].v);
					if (d <= t)
					{
						t = d;
					}
				}
				if (t < 1.0f)
				{
					break;
				}
			}
		}
		start = next;
	}
	start.x = ray.x * t + start.x;
	start.y = ray.y * t + start.y;
	start.z = ray.z * t + start.z;
	GetMeshList(start);
	int index;
	WVector col = ColTest(start, false, &index, true);
	float y = col.y;
	if (y < m_pHolder->aabb.min.y)
	{
		y = m_pHolder->aabb.min.y;
	}
	return WVector(col.x, y, col.z);
}

WVector CQuadTree::GetRayIntersectionObj(WVector start, WVector ray)
{
	int num = 0;
	bool bHit = false;
	WVector dir = ray;
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return WVector::ONE;
	}
	dir.Normalize();
	dir *= 1.6f;
	float fraction = 1.0f;
	int count = 0;
	do
	{
		GetObjList(start, dir, 2);	// 983 00958f10: mascara 2
		for (int i = 0; i < m_objNum; i++)
		{
			WPuppet* pPuppet = PolyObjs(m_pPolySoup)[m_objList[i]]->pet;
			if (pPuppet != NULL && pPuppet->m_bbList.Find("center") != NULL)
			{
				w_bound_box* pBox = pPuppet->m_bbList.Find("char0");
				while (pBox != NULL)
				{
					num++;
					Wobb obb;
					WVector half =
						(pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;
					obb.extend[0] = pBox->bone->GetMatrix().xa * half.x;
					obb.extend[1] = pBox->bone->GetMatrix().ya * half.y;
					obb.extend[2] = pBox->bone->GetMatrix().za * half.z;
					obb.center =
						pBox->info->spherePivot * pBox->bone->GetMatrix();
					WVector tri[3];
					WVector ext0 = obb.extend[0];
					WVector ext1 = obb.extend[1];
					WVector ext2 = obb.extend[2];
					tri[0] = ext1 + (ext2 * -1.0f - ext0 * 1.0f) + obb.center;
					tri[1] = ext1 * -1.0f + (ext2 * -1.0f - ext0 * 1.0f) +
						obb.center;
					tri[2] = ext2 * -1.0f + ext0 + ext1 * -1.0f + obb.center;
					WVector last = ext2 * -1.0f + ext0 + ext1 + obb.center;
					float t =
						intersection::IntersectRayTriangle(start, dir, tri);
					if (t <= fraction)
					{
						fraction = t;
					}
					if (fraction < 1.0f)
					{
						bHit = true;
					}
					tri[0] = tri[1];
					tri[1] = tri[2];
					tri[2] = last;
					t = intersection::IntersectRayTriangle(start, dir, tri);
					if (t <= fraction)
					{
						fraction = t;
					}
					if (fraction < 1.0f || bHit)
					{
						WPolySoup::sModel* pObj =
							PolyObjs(m_pPolySoup)[m_objList[i]];
						int triNum = pObj->quad->meshNum;
						fraction = 1.0f;
						for (int j = 0; j < triNum; j++)
						{
							t = intersection::IntersectRayTriangle(start, dir,
								pObj->tri[j].v);
							if (t <= fraction)
							{
								fraction = t;
							}
							if (fraction < 1.0f)
							{
								break;
							}
						}
						return dir * fraction + start;
					}
					pBox =
						PolyObjs(m_pPolySoup)[m_objList[i]]->pet->m_bbList.Find(
							MakeStr("char%d", num));
				}
			}
		}
		start += dir;
		count++;
	} while (count < 0x5dc);
	return GetRayIntersection(start, ray);
}

WVector CQuadTree::GetRayIntersectionWall(WVector start, WVector ray)
{
	WVector dir = ray;
	dir.Normalize();
	dir.x = dir.x * 1.6f;
	dir.y = dir.y * 1.6f;
	dir.z = dir.z * 1.6f;
	WVector hit = GetRayIntersection(start, ray);
	Waabb world;
	sQuadPuppetHolder* pWorld = GetPVS().m_pHolder;
	world.min.x = pWorld->aabb.min.x;
	world.min.y = pWorld->aabb.min.y;
	world.min.z = pWorld->aabb.min.z;
	world.max.x = pWorld->aabb.max.x;
	world.max.y = pWorld->aabb.max.y;
	world.max.z = pWorld->aabb.max.z;
	if (world.IsInclude(hit))
	{
		return hit;
	}
	sQuadPuppetHolder** ppHolder = PolyObjs(m_pPolySoup);
	if (ppHolder[0]->pet == NULL)
	{
		return start;
	}
	float t = 1.0f;
	for (int step = 0; step < 0x5dc; step++)
	{
		int num = ppHolder[0]->pet->GetFaceNum();
		if (num > 0)
		{
			char* pTri = reinterpret_cast<char*>(&ppHolder[0]->tri[0]);
			for (int i = 0; i < num; i++)
			{
				float d = intersection::IntersectRayTriangle(start, dir,
					reinterpret_cast<WVector*>(pTri));
				pTri += 0x48;
				if (t < d)
				{
					d = t;
				}
				if (0.0f < d)
				{
					t = d;
				}
			}
			if (t < 1.0f)
			{
				break;
			}
		}
		start.x = start.x + dir.x;
		start.y = start.y + dir.y;
		start.z = start.z + dir.z;
	}
	return WVector(dir.x * t + start.x, dir.y * t + start.y,
		dir.z * t + start.z);
}

float CQuadTree::IntersectRayTriangle(const WVector& orig, const WVector& dir,
	const WVector* tri)
{
	return intersection::IntersectRayTriangle(orig, dir, tri);
}

void CQuadTree::MakeAfterBurner()
{
	CGolfBall& ball = GolfBall();
	const CGolfBall::sExpected* pExpected;
	if (ball.m_renderNum < 0)
	{
		pExpected = NULL;
	}
	else
	{
		// 983 0095282a / 645 naked: bola+0xcc = m_expected[0] (velocidade de SAIDA), sem indice;
		// o [m_renderNum] da traducao em C (tira_naked aab0477ea) pegava a velocidade do quique: bola nao voltava
		pExpected = &ball.m_expected[0];
	}
	float vx = pExpected->vel.x;
	float vz = pExpected->vel.z;
	float scale = GolfBall().m_initSpin * -40.0f;
	float vy = 0.0f * scale;
	float invPower = 1.0f / GolfClub().GetPower();
	CGolfBall& b1 = GolfBall();
	b1.m_afterBurner.x = vx * scale * invPower;
	b1.m_afterBurner.y = vy * invPower;
	b1.m_afterBurner.z = vz * scale * invPower;
	float spin = GolfBall().m_initSpin * 1.5f;
	GolfBall().m_spin = spin;
	GolfBall().m_afterBurnerFrame = GolfBall().m_renderNum;
	GolfBall().m_state = CGolfBall::BALL_SPIN;
	CGolfBall& b2 = GolfBall();
	b2.m_vel.z = 0.0f;
	b2.m_vel.y = 0.0f;
	b2.m_vel.x = 0.0f;
	CGolfBall& b3 = GolfBall();
	b3.m_special = b3.m_special & 0xffffff8c;
}

void CQuadTree::MakeVectorSlide(float scale, float limit, float boost,
	int count)
{
	if ((GolfBall().m_initSpecial & 0x50) != 0)
	{
		return;
	}
	GolfBall().m_state = (CGolfBall::eBallState)1;
	int prevCount = GolfBall().GetVectorSlideCount();
	if (WVectorLen(GolfBall().m_vel) < limit)
	{
		GolfBall().m_vel.x = scale * GolfBall().m_vel.x;
		GolfBall().m_vel.y = scale * GolfBall().m_vel.y;
		GolfBall().m_vel.z = scale * GolfBall().m_vel.z;
	}
	WVector step = GolfBall().m_vel * 0.02f;
	WVector target = step + GolfBall().m_pos;
	target = GetGroundPoint(target, true, NULL, true);
	WVector dir = target - GolfBall().m_pos;
	dir.Normalize();
	dir.y = WVectorLen(GolfBall().m_vel) * dir.y;
	if (prevCount < count)
	{
		GolfBall().m_vel.y = dir.y + boost;
	}
	else
	{
		GolfBall().m_vel.y = dir.y;
	}
	GolfBall().AddVectorSlideCount();
	GolfBall().PreDataToWrite(GolfBall().m_renderNum)->event = 6;
}

int CQuadTree::GetBounceCount()
{
	int count = m_bounceCount;
	int lastFrame = -1;
	for (std::vector<sColData>::iterator it = m_colList.begin();
		it != m_colList.end(); ++it)
	{
		if ((*it).frame - lastFrame > 5)
		{
			count++;
			lastFrame = (*it).frame;
		}
	}
	return count;
}

float CQuadTree::GetBasePosY(WVector pos, float offset)
{
	unsigned int i;
	float top = pos.y;
	for (i = 0; i < m_water.size(); i = (unsigned char)(i + 1))
	{
		sWater& water = m_water[i];
		WVector max = water.aabb.max;
		// 983 00959730: so a agua acima da posicao vale; fora disso top fica pos.y
		if (m_water[i].aabb.min.x < pos.x && pos.x < max.x &&
			water.aabb.min.z < pos.z && pos.z < max.z && pos.y < max.y)
		{
			top = max.y;
			break;
		}
	}
	pos.y = top;
	if (m_bSpecialSkipHit && GolfBall().m_renderNum > 0)
	{
		return GetLowestGround(pos) + offset;
	}
	float hit[3];
	hit[0] = 0.0f;
	hit[1] = 100.0f;
	hit[2] = 0.0f;
	hit[1] = CheckCollision(pos, *reinterpret_cast<WVector*>(hit), NULL, true);
	WVector start;
	start.x = pos.x;
	start.y = pos.y + 100.0f;
	start.z = pos.z;
	WVector down;
	down.x = 0.0f;
	down.y = -100.0f;
	down.z = 0.0f;
	hit[2] = CheckCollision(start, down, NULL, true);
	float below = 1.0f - hit[2];
	if (hit[(below <= hit[1]) + 1] >= 1.0f || !(below <= hit[1]))
	{
		start.x = 0.0f;
		start.y = -100.0f;
		start.z = 0.0f;
		float t = CheckCollision(pos, start, NULL, true);
		if (t < 1.0f)
		{
			pos.y = (pos.y - t * 100.0f) + offset;
		}
	}
	else
	{
		pos.y = below * 100.0f + pos.y + offset;
	}
	if (pos.y < offset + m_groundHeight)
	{
		pos.y = offset + m_groundHeight;
	}
	return pos.y;
}

float CQuadTree::GetGravity()
{
	return m_gravity * 34.295296f;
}

void CQuadTree::GravityModifier(float gravity)
{
	m_gravity = gravity;
}

void CQuadTree::TraceOBB_New(const WVector& start, const WVector& end,
	const Waabb& box, trace_s* pTrace)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return;
	}
	WVector pos = end;
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	Waabb startBox;
	Waabb endBox;
	Waabb bound;
	trace_s trace;
	trace.m_endPos = WVector(0.0f, 0.0f, 0.0f);
	trace.m_fraction = 0.0f;
	trace.m_planeDist = 0.0f;
	bool bHit = false;
	if (WisEqual(start, end, 0.00001f))
	{
		pTrace->m_endPos = end;
		return;
	}
	startBox = box + start;
	endBox = box + end;
	bound = ExpandAABB(startBox, endBox);
	bound.min -= WVector::ONE;
	bound.max += WVector::ONE;
	GetObjList(bound, 1);
	for (int i = 0; i < m_objNum; i++)
	{
		sQuadPuppetHolder* pHolder = PolyObjs(m_pPolySoup)[m_objList[i]];
		if ((pHolder->noCollision & 1) == 0)
		{
			int num = 0;
			if (pHolder->pet != NULL)
			{
				w_bound_box* pBox = pHolder->pet->m_bbList.Find("char0");
				if (pBox == NULL)
				{
					if (pos.y <= pHolder->bbox.max.y &&
						pHolder->bbox.min.y - 10.0f <= pos.y)
					{
						Waabb area;
						area = pHolder->bbox;
						area.min.x -= 1.0f;
						area.min.z -= 1.0f;
						area.max.x += 1.0f;
						area.max.z += 1.0f;
						if (area.min.x < pos.x && pos.x < area.max.x &&
							area.min.z < pos.z && pos.z < area.max.z)
						{
							pTrace->m_endPos = start;
							pTrace->m_fraction = 1.0f;
							pTrace->m_flag = false;
							return;
						}
					}
				}
				else
				{
					do
					{
						Wobb obb(pBox->bone->GetMatrix());
						WVector half =
							(pBox->info->aabb.max - pBox->info->aabb.min) *
								0.5f +
							WVector::ONE * 1.0f;
						obb.extend[0] *= half.x;
						obb.extend[1] *= half.y;
						obb.extend[2] *= half.z;
						obb.center =
							pBox->info->spherePivot * pBox->bone->GetMatrix();
						if (obb.IsInclude(pos))
						{
							bHit = true;
							break;
						}
						num++;
						pBox = PolyObjs(m_pPolySoup)[m_objList[i]]
								   ->pet->m_bbList.Find(MakeStr("char%d", num));
					} while (pBox != NULL);
				}
			}
		}
	}
	if (bHit)
	{
		pTrace->m_endPos = start;
		pTrace->m_fraction = 1.0f;
		pTrace->m_flag = false;
		return;
	}
	pTrace->m_endPos = end;
	pTrace->m_fraction = 1.0f; // 983 00959fc6: tambem sem colisao
	pTrace->m_flag = false;
}

bool CQuadTree::TraceAABB_BackGround(const WVector& start, const WVector& end,
	const Waabb& box, trace_s* pTrace, WPolySoup::sModel* pModel)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return false;
	}
	if (WisEqual(start, end, 0.00001f))
	{
		pTrace->m_endPos = end;
		return false;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	WVector delta = end - start;
	Waabb world = m_pHolder->aabb;
	world.min.x -= 1.0f;
	world.min.y -= 1.0f;
	world.min.z -= 1.0f;
	world.max += WVector::ONE;
	Waabb moved = box;
	moved.min.x += delta.x;
	moved.min.z += delta.z;
	moved.max.x += delta.x;
	moved.max.z += delta.z;
	bool inside = world.IsInclude(moved);
	pTrace->m_fraction = 1.0f;
	pTrace->m_flag = false;
	return inside;
}

int CQuadTree::CheckCamera()
{
	int result = -1;
	int index = 0;
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return -1;
	}
	WPuppet* pPuppet = PolyObjs(m_pPolySoup)[0]->pet;
	if (pPuppet != NULL)
	{
		w_bound_box* pCenter = pPuppet->m_bbList.Find("center");
		if (pCenter != NULL)
		{
			w_bound_box* pBox = pPuppet->m_bbList.Find("camera0");
			while (pBox != NULL)
			{
				const WVector& vMin = pBox->info->aabb.min;
				WVector corners[4] = { vMin, vMin, vMin, vMin };
				const WVector& vMax = pBox->info->aabb.max;
				corners[0].y = vMax.y;
				corners[1].x = vMax.x;
				corners[1].y = vMax.y;
				corners[2].x = vMax.x;
				bool bVisible;
				for (int i = 0; i < 4; i++)
				{
					bVisible = g_view->InFrustum(corners[i]);
					if (bVisible)
					{
						result = index;
						break;
					}
				}
				if (bVisible)
				{
					break;
				}
				index++;
				pBox = PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find(
					MakeStr("camera%d", index));
			}
		}
	}
	return result;
}

void CQuadTree::SetAvatarPosition(const WVector& pos, float radius)
{
	if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		m_aabb2148.min.z = 0.0f;
		m_aabb2148.min.y = 0.0f;
		m_aabb2148.min.x = 0.0f;
		m_aabb2148.max.z = 0.0f;
		m_aabb2148.max.y = 0.0f;
		m_aabb2148.max.x = 0.0f;
		m_aabb2148.min.x = pos.x;
		m_aabb2148.min.y = pos.y;
		m_aabb2148.min.z = pos.z;
		m_aabb2148.max.x = pos.x;
		m_aabb2148.max.y = pos.y;
		m_aabb2148.max.z = pos.z;
		m_aabb2148.min.x = m_aabb2148.min.x - radius;
		m_aabb2148.max.x = radius + m_aabb2148.max.x;
		m_aabb2148.min.y = m_aabb2148.min.y - radius;
		m_aabb2148.max.y = radius + m_aabb2148.max.y;
		m_aabb2148.min.z = m_aabb2148.min.z - radius;
		m_aabb2148.max.z = radius + m_aabb2148.max.z;
		m_aabb2148.min.y = -2.0f;
		m_aabb2148.max.y = 2.0f;
	}
}

void CQuadTree::TraceOBB_Avatar_genPosition(const WVector& start,
	const WVector& end, trace_s* pTrace, WPolySoup::sModel* pModel)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	int num = 0;
	if (WisEqual(start, end, 0.00001f))
	{
		pTrace->m_endPos = start;
		return;
	}
	WVector delta = end - start;
	if (pModel->pet == NULL)
	{
		pTrace->m_endPos = start;
		return;
	}
	w_bound_box* pBox = pModel->pet->m_bbList.Find("center");
	if (pBox != NULL)
	{
		Wobb avatar(pBox->bone->GetMatrix());
		avatar *= (m_aabb2148.max - m_aabb2148.min) * 0.5f;
		avatar.center = (m_aabb2148.max + m_aabb2148.min) * 0.5f;
		pBox = pModel->pet->m_bbList.Find("char0");
		while (pBox != NULL)
		{
			Wobb obb;
			WVector half = (pBox->info->aabb.max - pBox->info->aabb.min) * 0.5f;
			obb.extend[0] = pBox->bone->GetMatrix().xa * half.x;
			obb.extend[1] = pBox->bone->GetMatrix().ya * half.y;
			obb.extend[2] = pBox->bone->GetMatrix().za * half.z;
			obb.center = pBox->info->spherePivot * pBox->bone->GetMatrix();
			delta.y = 0.0f;
			Wobb moved = obb;
			moved.center.x = delta.x + obb.center.x;
			moved.center.z = delta.z + obb.center.z;
			if (ObbOverlap(avatar, moved))
			{
				WVector dir(delta.x, delta.y, delta.z);
				dir.Normalize();
				moved = obb;
				moved.center.x = dir.x + moved.center.x;
				moved.center.z = dir.z + moved.center.z;
				if (!ObbOverlap(avatar, moved))
				{
					pTrace->m_endPos =
						WVector(dir.x + start.x, start.y, dir.z + start.z);
					return;
				}
				pTrace->m_endPos = start;
				return;
			}
			num++;
			pBox = pModel->pet->m_bbList.Find(MakeStr("char%d", num));
		}
	}
	pTrace->m_endPos = end;
}

void CQuadTree::TraceOBB_WallSliding(const WVector& start, const WVector& end,
	trace_s* pTrace, WPolySoup::sModel* pModel, int axis, bool bForce)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	bool bHit = false;
	bool bTested = false;
	Waabb box;
	box.min = WVector(0.0f, 0.0f, 0.0f);
	box.max = WVector(0.0f, 0.0f, 0.0f);
	Waabb wallBox;
	wallBox.min = WVector(0.0f, 0.0f, 0.0f);
	wallBox.max = WVector(0.0f, 0.0f, 0.0f);
	WVector pos = start;
	int wallNum = 0;
	WVector target = end;
	if (WisEqual(start, end, 0.00001f) && !bForce)
	{
		pTrace->m_endPos = start;
		return;
	}
	float fraction = 1.0f;
	WVector delta = end - start;
	if (strcmp(PolyObjs(m_pPolySoup)[0]->name, pModel->name) != 0)
	{
		WPuppet* pRoom = PolyObjs(m_pPolySoup)[0]->pet;
		if (pRoom == NULL)
		{
			pTrace->m_endPos = start;
			return;
		}
		if (pModel->pet != NULL)
		{
			WVector dir = delta;
			if (pModel->pet->m_bbList.Find("center") != NULL)
			{
				w_bound_box* pCenter = pRoom->m_bbList.Find("center");
				w_bound_box* pWindow;
				if (axis == 0 ||
					(pWindow = pRoom->m_bbList.Find("center_window")) != NULL)
				{
					box = pModel->aabb;
					if (pCenter == NULL)
					{
						pTrace->m_endPos = start;
						return;
					}
					w_bound_box* pWallBox =
						PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find("char0");
					while (pWallBox != NULL)
					{
						int num = 0;
						Wobb wall(pWallBox->bone->GetMatrix());
						wall *= (pWallBox->info->aabb.max -
									pWallBox->info->aabb.min) *
							0.5f;
						wall.center = pWallBox->info->spherePivot *
							pWallBox->bone->GetMatrix();
						wallBox = pWallBox->info->aabb +
							PolyObjs(m_pPolySoup)[0]->pet->GetMatrix().pivot;
						w_bound_box* pBox = pModel->pet->m_bbList.Find("char0");
						if (pBox != NULL)
						{
							bTested = true;
							do
							{
								Wobb obb(pBox->bone->GetMatrix());
								WVector half = (pBox->info->aabb.max -
												   pBox->info->aabb.min) *
									0.5f;
								obb.extend[0] *= half.x;
								obb.extend[1] *= half.y;
								obb.extend[2] *= half.z;
								obb.center = pBox->info->spherePivot *
									pBox->bone->GetMatrix();
								box.min = pModel->aabb.min;
								box.max = pModel->aabb.max;
								if (axis <= 0)
								{
									Wobb moved = obb;
									if (axis == 0)
									{
										dir.y = 0.0f;
									}
									else if (axis == 1)
									{
										dir.z = 0.0f;
										WVector center = obb.center + dir;
										box.max = center + pBox->info->aabb.max;
										box.min = center + pBox->info->aabb.min;
										if (pWindow->info->aabb.max.y <
												box.max.y &&
											box.max.y -
													pWindow->info->aabb.max.y >
												0.001f)
										{
											dir.y = end.y -
												(box.max.y -
													pWindow->info->aabb.max.y) -
												start.y;
											target.y = dir.y + start.y;
										}
										if (box.min.y <
											pWindow->info->aabb.min.y)
										{
											dir.y = pWindow->info->aabb.min.y -
												box.min.y + end.y - start.y;
											target.y = dir.y + start.y;
										}
									}
									moved += dir;
									if (ObbOverlap(wall, moved))
									{
										bHit = true;
										float dz = GetObbPos(moved, true, 2) -
											pWallBox->info->aabb.min.z;
										float dx;
										if (target.x > 0.0f)
										{
											dx = GetObbPos(moved, true, 0) -
												pWallBox->info->aabb.min.x;
										}
										else
										{
											dx = GetObbPos(moved, false, 0) -
												pWallBox->info->aabb.max.x;
										}
										if (Abs(pWallBox->info->aabb.max.x -
												pWallBox->info->aabb.min.x) >
											Abs(pWallBox->info->aabb.max.z -
												pWallBox->info->aabb.min.z))
										{
											target.z = target.z - dz;
										}
										else
										{
											if (target.x == 0.0f)
											{
												target.z = wall.center.z - dz;
												target.x = wall.center.x;
											}
											target.x = target.x - dx;
										}
										if (axis == 1)
										{
											dir.x = target.x - start.x;
											dir.z = target.z - start.z;
										}
										else
										{
											delta = target - start;
											dir = delta;
										}
										pos = target;
									}
								}
								else
								{
									WPlane plane;
									float t = AabbIntersect(box, dir, wallBox,
										&plane);
									if (t < fraction)
									{
										fraction = t;
									}
								}
								num++;
								pBox = pModel->pet->m_bbList.Find(
									MakeStr("char%d", num));
							} while (pBox != NULL);
						}
						wallNum++;
						pWallBox = PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find(
							MakeStr("char%d", wallNum));
					}
					if (axis <= 0)
					{
						if (bHit)
						{
							pTrace->m_endPos = pos;
							return;
						}
						if (!bTested)
						{
							pTrace->m_endPos = start;
							return;
						}
						pTrace->m_endPos = end;
						return;
					}
					if (fraction == 1.0f)
					{
						pTrace->m_endPos = end;
						return;
					}
					if (fraction == 0.0f)
					{
						pTrace->m_endPos = WVector::UNIT_POS_Y + start + dir;
					}
					else
					{
						pTrace->m_endPos =
							WVector::UNIT_POS_Y + start + dir * fraction;
					}
					wallBox = pWindow->info->aabb +
						PolyObjs(m_pPolySoup)[0]->pet->GetMatrix().pivot;
					pTrace->m_endPos.y = end.y;
					if (wallBox.max.y <
						(pModel->aabb.max.y - pModel->aabb.min.y) * 0.5f +
							end.y)
					{
						pTrace->m_endPos.y = wallBox.max.y -
							(pModel->aabb.max.y - pModel->aabb.min.y) * 0.5f;
					}
					if (end.y -
							(pModel->aabb.max.y - pModel->aabb.min.y) * 0.5f <
						wallBox.min.y)
					{
						pTrace->m_endPos.y =
							(pModel->aabb.max.y - pModel->aabb.min.y) * 0.5f +
							wallBox.min.y;
					}
					return;
				}
				pTrace->m_endPos = start;
				return;
			}
		}
	}
	pTrace->m_endPos = start;
}

bool CQuadTree::CheckPickRoom(WVector pos)
{
	if (IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		WPolySoup* pSoup = m_pPolySoup;
		sQuadPuppetHolder* pHolder = PolyObjs(pSoup)[0];
		Waabb room;
		room.min.z = 0.0f;
		room.min.y = 0.0f;
		room.min.x = 0.0f;
		room.max.z = 0.0f;
		room.max.y = 0.0f;
		room.max.x = 0.0f;
		if (pHolder->pet != NULL)
		{
			WList<w_bound_box*>& list = pHolder->pet->m_bbList;
			w_bound_box* pBox = list.Find("center_window");
			if (pBox != NULL)
			{
				// 983 0095b610: m_mat.pivot (+0xb8); o 0x74 do 645 caia em m_phybone
				room = pBox->info->aabb +
					PolyObjs(pSoup)[0]->pet->GetMatrix().pivot;
				return room.IsInclude(pos);
			}
		}
	}
	return false;
}

int CQuadTree::Get_WallSlidingAngle(const WVector& pos,
	WPolySoup::sModel* pModel, WVector& out)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return -1;
	}
	WVector dir = pos;
	WVector cand[3];
	cand[0] = WVector(0.0f, 0.0f, 0.0f);
	cand[1] = WVector(0.0f, 0.0f, 0.0f);
	cand[2] = WVector(0.0f, 0.0f, 0.0f);
	WPuppet* pRoom = PolyObjs(m_pPolySoup)[0]->pet;
	int num = 0;
	Waabb wallBox;
	Waabb centerBox;
	WVector lo;
	WVector hi;
	if (pRoom != NULL)
	{
		w_bound_box* pRoomCenter = pRoom->m_bbList.Find("center");
		w_bound_box* pCenter = pModel->pet->m_bbList.Find("center");
		if (pCenter != NULL)
		{
			centerBox = pCenter->info->aabb + pModel->pet->GetMatrix().pivot;
			if (pRoomCenter != NULL)
			{
				w_bound_box* pBox =
					PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find("char_col0");
				while (pBox != NULL)
				{
					wallBox = pBox->info->aabb +
						PolyObjs(m_pPolySoup)[0]->pet->GetMatrix().pivot;
					if (wallBox.IsInclude(pos))
					{
						if (num == 0)
						{
							float hx =
								(centerBox.max.x - centerBox.min.x) * 0.5f;
							float hy =
								(centerBox.max.y - centerBox.min.y) * 0.5f;
							lo = WVector(pos.x - hx, pos.y - hy, pos.z);
							hi = WVector(hx + pos.x, hy + pos.y, pos.z);
						}
						else if (num > 0 && num < 3)
						{
							float hy =
								(centerBox.max.y - centerBox.min.y) * 0.5f;
							float hx =
								(centerBox.max.x - centerBox.min.x) * 0.5f;
							lo = WVector(pos.x, pos.y - hy, pos.z - hx);
							hi = WVector(pos.x, hy + pos.y, hx + pos.z);
						}
						if (wallBox.IsInclude(lo) && wallBox.IsInclude(hi))
						{
							dir.Normalize();
							out = (hi + lo) * 0.5f + dir;
							cand[num] = out;
						}
					}
					num++;
					pBox = PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find(
						MakeStr("char_col%d", num));
				}
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
					{
						if ((&cand[i].x)[j] != 0.0f)
						{
							lo = out + out;
							for (int k = 0; k < 3; k++)
							{
								dir = cand[k];
								float len =
									sqrtf(dir.x * dir.x + dir.z * dir.z);
								if (len < sqrtf(lo.x * lo.x + lo.z * lo.z) &&
									len != 0.0)
								{
									lo = cand[k];
									num = k;
								}
							}
							out = cand[num];
							return num;
						}
					}
				}
			}
		}
	}
	return -1;
}

void CQuadTree::OBBToAABB(Wobb obb, Waabb& aabb, WVector& pos)
{
	WPolySoup::sModel* pWorld = GOLFDOC()->m_pPolySoup->FindModel(NULL);
	WVector vMax = pWorld->aabb.min;
	WVector vMin = pWorld->aabb.max;
	vMin.y = 30.0f;
	WVector center = obb.center;
	WVector ext0 = obb.extend[0];
	WVector ext1 = obb.extend[1];
	WVector ext2 = obb.extend[2];
	vMax.y = 0.0f;
	WVector v[8] = {
		center + ext0 * -1.0f + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 + ext2,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2,
		center + ext0 + ext1 * -1.0f + ext2,
		center + ext0 + ext1 + ext2,
	};
	WVector* pCorner = v;
	for (int count = 8; count != 0; count--, pCorner++)
	{
		if (pCorner->x >= vMax.x)
		{
			vMax.x = pCorner->x;
		}
		if (pCorner->y >= vMax.y)
		{
			vMax.y = pCorner->y;
		}
		if (pCorner->z >= vMax.z)
		{
			vMax.z = pCorner->z;
		}
		if (pCorner->x <= vMin.x)
		{
			vMin.x = pCorner->x;
		}
		if (pCorner->y <= vMin.y)
		{
			vMin.y = pCorner->y;
		}
		if (pCorner->z <= vMin.z)
		{
			vMin.z = pCorner->z;
		}
	}
	aabb.max = vMax;
	aabb.min = vMin;
}

float CQuadTree::GetObbPos(Wobb& obb, bool bMax, int axis)
{
	float result = bMax ? -1000.0f : 1000.0f;
	WVector center = obb.center;
	WVector ext0 = obb.extend[0];
	WVector ext1 = obb.extend[1];
	WVector ext2 = obb.extend[2];
	WVector v[8] = {
		center + ext0 * -1.0f + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 * -1.0f + ext2 * -1.0f,
		center + ext0 + ext1 + ext2 * -1.0f,
		center + ext0 * -1.0f + ext1 + ext2,
		center + ext0 * -1.0f + ext1 * -1.0f + ext2,
		center + ext0 + ext1 * -1.0f + ext2,
		center + ext0 + ext1 + ext2,
	};
	if (bMax)
	{
		if (v[0].p[axis] >= result)
		{
			result = v[0].p[axis];
		}
		if (v[1].p[axis] >= result)
		{
			result = v[1].p[axis];
		}
		if (v[2].p[axis] >= result)
		{
			result = v[2].p[axis];
		}
		if (v[3].p[axis] >= result)
		{
			result = v[3].p[axis];
		}
		if (v[4].p[axis] >= result)
		{
			result = v[4].p[axis];
		}
		if (v[5].p[axis] >= result)
		{
			result = v[5].p[axis];
		}
		if (v[6].p[axis] >= result)
		{
			result = v[6].p[axis];
		}
		if (v[7].p[axis] >= result)
		{
			result = v[7].p[axis];
		}
	}
	else
	{
		if (v[0].p[axis] <= result)
		{
			result = v[0].p[axis];
		}
		if (v[1].p[axis] <= result)
		{
			result = v[1].p[axis];
		}
		if (v[2].p[axis] <= result)
		{
			result = v[2].p[axis];
		}
		if (v[3].p[axis] <= result)
		{
			result = v[3].p[axis];
		}
		if (v[4].p[axis] <= result)
		{
			result = v[4].p[axis];
		}
		if (v[5].p[axis] <= result)
		{
			result = v[5].p[axis];
		}
		if (v[6].p[axis] <= result)
		{
			result = v[6].p[axis];
		}
		if (v[7].p[axis] <= result)
		{
			result = v[7].p[axis];
		}
	}
	return result;
}

void CQuadTree::TraceOBB_Wall(const WVector& start, const WVector& end,
	trace_s* pTrace, WPolySoup::sModel* pModel, int axis, bool bForce)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	bool bHit = false;
	bool bTested = false;
	WVector pos = start;
	int wallNum = 0;
	if (WisEqual(start, end, 0.00001f) && !bForce)
	{
		pTrace->m_endPos = start;
		return;
	}
	WVector delta = end - start;
	if (strcmp(PolyObjs(m_pPolySoup)[0]->name, pModel->name) != 0 &&
		PolyObjs(m_pPolySoup)[0]->pet != NULL && pModel->pet != NULL)
	{
		WPuppet* pRoom = PolyObjs(m_pPolySoup)[0]->pet;
		WVector dir = delta;
		if (pModel->pet->m_bbList.Find("center") != NULL &&
			pRoom->m_bbList.Find("center") != NULL)
		{
			w_bound_box* pWallBox = pRoom->m_bbList.Find("char0");
			if (pWallBox != NULL)
			{
				do
				{
					int num = 0;
					Wobb wall(pWallBox->bone->GetMatrix());
					wall *=
						(pWallBox->info->aabb.max - pWallBox->info->aabb.min) *
						0.5f;
					wall.center = pWallBox->info->spherePivot *
						pWallBox->bone->GetMatrix();
					w_bound_box* pBox = pModel->pet->m_bbList.Find("char0");
					if (pBox != NULL)
					{
						bTested = true;
						do
						{
							Wobb obb(pBox->bone->GetMatrix());
							WVector half =
								(pBox->info->aabb.max - pBox->info->aabb.min) *
								0.5f;
							obb.extend[0] *= half.x;
							obb.extend[1] *= half.y;
							obb.extend[2] *= half.z;
							obb.center = pBox->info->spherePivot *
								pBox->bone->GetMatrix();
							Wobb moved = obb;
							if (axis == 0)
							{
								delta.y = 0.0f;
							}
							else if (axis == 1)
							{
								delta.z = 0.0f;
							}
							moved += delta;
							if (ObbOverlap(wall, moved))
							{
								bHit = true;
								dir = delta;
								dir.Normalize();
								if (axis == 0)
								{
									dir.y = 0.0f;
								}
								else if (axis == 1)
								{
									dir.z = 0.0f;
								}
								moved = obb;
								moved += dir;
								if (!ObbOverlap(wall, moved))
								{
									delta = dir;
									pos = WVector(dir.x + start.x,
										dir.y + start.y, dir.z + start.z);
								}
								else
								{
									pos = start;
								}
								break;
							}
							num++;
							pBox = pModel->pet->m_bbList.Find(
								MakeStr("char%d", num));
						} while (pBox != NULL);
					}
					wallNum++;
					pWallBox = PolyObjs(m_pPolySoup)[0]->pet->m_bbList.Find(
						MakeStr("char%d", wallNum));
				} while (pWallBox != NULL);
				if (bHit)
				{
					pTrace->m_endPos = pos;
					return;
				}
				if (bTested)
				{
					pTrace->m_endPos = end;
					return;
				}
			}
		}
		pTrace->m_endPos = start;
		return;
	}
	pTrace->m_endPos = start;
}

bool CQuadTree::TraceOBB_Object(const WVector& start, const WVector& end,
	const Waabb& box, trace_s* pTrace, WPolySoup::sModel* pModel, bool bAll)
{
	if (!IsLocalContent((localContentType_t)0x57 /*983: era 0x56; medido*/))
	{
		return false;
	}
	pTrace->m_endPos = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_fraction = 0.0f;
	pTrace->m_normal = WVector(0.0f, 0.0f, 0.0f);
	pTrace->m_planeDist = 0.0f;
	pTrace->m_flag = false;
	WVector delta = end - start;
	int num = 0;
	bool bHit = false;
	int wallNum = 0;
	Waabb bound;
	bound.min = WVector(0.0f, 0.0f, 0.0f);
	bound.max = WVector(0.0f, 0.0f, 0.0f);
	Waabb startBox;
	startBox.min = WVector(0.0f, 0.0f, 0.0f);
	startBox.max = WVector(0.0f, 0.0f, 0.0f);
	Waabb endBox;
	endBox.min = WVector(0.0f, 0.0f, 0.0f);
	endBox.max = WVector(0.0f, 0.0f, 0.0f);
	if (PolyObjs(m_pPolySoup)[0]->pet != NULL &&
		strcmp(PolyObjs(m_pPolySoup)[0]->name, pModel->name) != 0)
	{
		WVector dir = delta;
		startBox = box + start;
		endBox = box + end;
		bound = ExpandAABB(startBox, endBox);
		bound.min -= WVector::ONE;
		bound.max += WVector::ONE;
		GetObjList(bound, 0);
		if (pModel->pet == NULL)
		{
			pTrace->m_endPos = start;
			return true;
		}
		if (pModel->pet->m_bbList.Find("center") == NULL)
		{
			pTrace->m_endPos = start;
			return true;
		}
		for (int i = 0; i < m_objNum; i++)
		{
			if (bAll || m_objList[i] > 0)
			{
				WPolySoup::sModel* pObj = PolyObjs(m_pPolySoup)[m_objList[i]];
				if (strcmp(pObj->name, pModel->name) != 0 && pObj->pet != NULL)
				{
					w_bound_box* pCenter = pObj->pet->m_bbList.Find("center");
					char poster[0x100] = "poster_a.pet";
					if (stricmp(
							PolyObjs(m_pPolySoup)[m_objList[i]]->pet->m_petName,
							poster) != 0 &&
						pCenter != NULL)
					{
						w_bound_box* pWallBox =
							PolyObjs(m_pPolySoup)[m_objList[i]]
								->pet->m_bbList.Find("char0");
						while (pWallBox != NULL)
						{
							Wobb wall(pWallBox->bone->GetMatrix());
							wall *= (pWallBox->info->aabb.max -
										pWallBox->info->aabb.min) *
								0.5f;
							wall.center = pWallBox->info->spherePivot *
								pWallBox->bone->GetMatrix();
							w_bound_box* pBox =
								pModel->pet->m_bbList.Find("char0");
							while (pBox != NULL)
							{
								Wobb obb(pBox->bone->GetMatrix());
								WVector half = (pBox->info->aabb.max -
												   pBox->info->aabb.min) *
									0.5f;
								obb.extend[0] *= half.x;
								obb.extend[1] *= half.y;
								obb.extend[2] *= half.z;
								obb.center = pBox->info->spherePivot *
									pBox->bone->GetMatrix();
								num++;
								pBox = pModel->pet->m_bbList.Find(
									MakeStr("char%d", num));
								Wobb moved = obb;
								moved += dir;
								if (ObbOverlap(wall, moved))
								{
									WVector normal = delta;
									normal.Normalize();
									moved = obb;
									moved.center.x = normal.x + moved.center.x;
									moved.center.z = normal.z + moved.center.z;
									if (!ObbOverlap(wall, moved))
									{
										pTrace->m_endPos =
											WVector(normal.x + start.x, start.y,
												normal.z + start.z);
										return false;
									}
									bHit = true;
									goto next;
								}
							}
							if (bHit)
							{
								break;
							}
							wallNum++;
							num = 0;
							pWallBox = PolyObjs(m_pPolySoup)[m_objList[i]]
										   ->pet->m_bbList.Find(
											   MakeStr("char%d", wallNum));
						}
					}
				}
			}
next:;
		}
		if (bHit)
		{
			pTrace->m_endPos = start;
			return bHit;
		}
		if (bAll)
		{
			pTrace->m_endPos = start + end;
		}
		else
		{
			pTrace->m_endPos = end;
		}
		return bHit;
	}
	pTrace->m_endPos = start;
	return false;
}
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // A748: ex-cru Doc()+0x10a99
