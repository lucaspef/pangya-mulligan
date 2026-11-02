#include "minatl.h"
#include "golfdoc.h"
#include "golfball.h"
#include "club.h"
#include "quadtree.h"
#include "wind.h"
#include "autotarget.h"
#include "rival.h"

// HACK: CQuadTree methods not declared in quadtree.h are reached through a
// derived helper + /alternatename, because more names in quadtree.h change
// the code generated for polysoup.cpp (WPolySoup::MakeStatistics).
class CQuadTreeRival : public CQuadTree
{
public:
	WVector GetMeshNormal(const WVector& pos);
	float GetMaxPowerOnGround(const WVector& pos);
	float RayIntersectAll(const WVector& start, const WVector& ray,
		bool bObject);
};
#pragma comment(linker, \
	"/alternatename:?GetMeshNormal@CQuadTreeRival@@QAE?AVWVector@@ABV2@@Z=?GetMeshNormal@CQuadTree@@QAE?AVWVector@@ABV2@@Z")
#pragma comment(linker, \
	"/alternatename:?GetMaxPowerOnGround@CQuadTreeRival@@QAEMABVWVector@@@Z=?GetMaxPowerOnGround@CQuadTree@@QAEMABVWVector@@@Z")
#pragma comment(linker, \
	"/alternatename:?RayIntersectAll@CQuadTreeRival@@QAEMABVWVector@@0_N@Z=?RayIntersectAll@CQuadTree@@QAEMABVWVector@@0_N@Z")
#define PVSEX() (static_cast<CQuadTreeRival&>(GetPVS()))

// HACK: CGolfDoc::GetCurrentPlayerNum is a real function of golfdoc.cpp (it returns m_currentPlayerNum);
// the original inlines the current player read here without emitting a body, and a dllimport
// __forceinline accessor is never emitted (same as wind.cpp).
__declspec(dllimport) __forceinline unsigned char RbCurrentPlayer(CGolfDoc* doc)
{
	return doc->m_currentPlayer;
}

const float BAR_LENGTH = BAR_END - BAR_START;

#define YARD_TO_POS(yard) ((yard) * 3.2f)


extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

// HACK: view of the CGolfDoc gauge fields at +0xd0, because naming them in
// golfdoc.h changes the code generated for projectg.cpp (SaveScreenShot).
struct sGolfDocGauge
{
	float m_gaugePower;
	float m_gaugeImpact;
	unsigned char m_unused8[8];
	int m_impactResult;
	bool m_bMaxPower;
	unsigned char m_unused15[7];
	float m_impactCenter;
};

#define GAUGE() ((sGolfDocGauge*)(GOLFDOC()->m_unusedcd + 3))

CRival::CRival()
	: m_reserved(0)
{
	// HACK: compiler-only memory barrier (no code). Without it the scheduler
	// sinks the vptr store below the member zeroing; the original stores it
	// right after "xor eax, eax".
	_ReadWriteBarrier();
	m_curve = 0.0f;
	m_spin = 0.0f;
	m_mode = 0;
}

CRival::~CRival()
{
}

void CRival::Shot()
{
	if (WisEqual(m_power, 1.0f, g_EPSILON))
		GAUGE()->m_bMaxPower = true;

	GAUGE()->m_gaugePower = BAR_LENGTH * m_power + BAR_START;
	GAUGE()->m_gaugeImpact = GAUGE()->m_impactCenter + Random(-10.0f, 10.0f);

	float diff = GAUGE()->m_gaugeImpact - BAR_START;
	float dist = diff > 0.0f ? diff : -diff;

	if (dist < 2.0f)
		GAUGE()->m_impactResult = 4;
	else if (dist < GolfClub().GetNarrowAccuracy())
		GAUGE()->m_impactResult = 3;
	else if (dist < GolfClub().GetWideAccuracy())
		GAUGE()->m_impactResult = 2;
	else
		GAUGE()->m_impactResult = 1;
}

void CRival::SetVariable(CGolfDoc* pDoc)
{
	WVector vec;
	float factor;
	int index;
	// HACK: unused locals and DCE'd code that emit nothing, but shift the IL
	// temporary numbering that decides the operand order of commutative FP ops
	// further down (putter/AIM retarget sums, third club-search retarget).
	WVector ilfill0;
	WVector ilfill1;
	float rate = PVSEX().GetMaxPowerOnGround(GolfBall().m_pos);

	if (0)
		factor = vec.Magnitude();

	if (m_mode)
	{
		GolfClub().SetAccuracy();

		vec = m_expected - m_target;

		GolfClub().SetClub(m_club);

		if ((unsigned char)PLAYER(RbCurrentPlayer(GOLFDOC()))
				->stroke[GOLFDOC()->m_currentHole - 1] == 3)
		{
			if (m_mode != 1)
				goto AIM;

			m_club = Min(m_club + 2, 12);
			GolfClub().SetClub(m_club);

			m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
			m_target = GolfBall().m_pos + (m_target - GolfBall().m_pos) * rate;
			return;
		}

		switch (m_mode)
		{
		case 1:
			if (PLAYER(GOLFDOC()->m_currentPlayer)->flag > 1)
				m_club = Min(m_club + 3, 12);
			else
				m_club = Min(m_club + 2, 12);

			GolfClub().SetClub(m_club);

			m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
			m_target = GolfBall().m_pos + rate * (m_target - GolfBall().m_pos);
			return;

		case 2:
			if (PLAYER(GOLFDOC()->m_currentPlayer)->flag == 1)
				goto SELECT;
			break;

		case 3:
			if (PLAYER(GOLFDOC()->m_currentPlayer)->flag == 1)
				goto AIM;
			break;

		case 4:
			m_angle = CalcDeltaAngle(m_target - GolfBall().m_pos - vec,
				WVector::UNIT_POS_Z);
			m_target = GolfBall().m_pos +
				WVector::UNIT_POS_Z * YARD_TO_POS(GolfClub().GetRange()) *
					RotMat(m_angle, 1);
			break;
		}

		if (GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->flag > 1)
		{
			m_club = Min(m_club + 3, 12);
			GolfClub().SetClub(m_club);

			m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
			m_target = GolfBall().m_pos + (m_target - GolfBall().m_pos) * rate;
		}
		return;
	}

SELECT:
	GolfClub().ClearPointFactor();
	GolfClub().SelectBestClub();
	m_club = GolfClub().GetIndex();

	while (true)
	{
		m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
		m_target = GolfBall().m_pos +
			(m_target - GolfBall().m_pos) *
				(GolfClub().GetType() == 3 ? 1.0f : rate);

		GetPVS().GetGroundPoint(m_target, true, &index, true);

		if (index < 0)
			GolfClub().SetPointFactor(0.0f);
		else if (GetPVS().GetGroundType(m_target, true, NULL) == 4)
			GolfClub().SetPointFactor(0.8f);
		else if (GetPVS().GetGroundType(m_target, true, NULL) == 3)
			GolfClub().SetPointFactor(0.6f);
		else
			break;

		GolfClub().SelectBestClub();

		if (GolfClub().GetIndex() == m_club)
			break;

		m_club = GolfClub().GetIndex();

		if (GolfClub().GetIndex() == 12)
		{
			m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
			m_target = GolfBall().m_pos +
				(m_target - GolfBall().m_pos) *
					(GolfClub().GetType() == 3 ? 1.0f : rate);

			GetPVS().GetGroundPoint(m_target, true, &index, true);

			if (index < 0)
			{
				GolfClub().SetPointFactor(0.0f);
				GolfClub().SelectBestClub();

				m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
				m_target = GolfBall().m_pos +
					(m_target - GolfBall().m_pos) *
						(GolfClub().GetType() == 3 ? 1.0f : rate);

				m_club = GolfClub().GetIndex();
			}
			break;
		}
	}

	GolfClub().ClearPointFactor();

AIM:
	WVector dir = m_target - GolfBall().m_pos;
	dir.y = 0.0f;
	dir.Normalize();

	if (GolfClub().GetType() == 3)
	{
		WVector slope = GetGreenSlope();

		m_angle = CalcDeltaAngle(m_target - GolfBall().m_pos - slope * 5.0f,
			WVector::UNIT_POS_Z);
		m_target = GolfBall().m_pos + WVector::UNIT_POS_Z * RotMat(m_angle, 1);

		vec = GetHoleCupPos() - GolfBall().m_pos - slope * 5.0f;
		vec.y = 0.0f;
		m_power =
			Min(vec.Magnitude() / YARD_TO_POS(GolfClub().GetRange()), 1.0f);
		return;
	}

	if (GolfClub().GetType() == 2)
	{
		if (GolfClub().GetApproachMode() <= 2)
			factor = 1.0f;
		else
			factor = m_power * 3.0f;
	}
	else
		factor = 3.0f;

	m_angle = CalcDeltaAngle(m_target - GolfBall().m_pos -
			Wind().GetGlobalWind() * factor,
		WVector::UNIT_POS_Z);
	m_target = GolfBall().m_pos +
		WVector::UNIT_POS_Z * YARD_TO_POS(GolfClub().GetRange()) *
			RotMat(m_angle, 1);

	if ((unsigned char)PLAYER(RbCurrentPlayer(GOLFDOC()))
			->stroke[GOLFDOC()->m_currentHole - 1] == 1)
	{
		m_power = 1.0f;
	}
	else if (m_mode == 3)
	{
		if ((m_expected - GolfBall().m_pos).Magnitude() <
			(m_target - GolfBall().m_pos).Magnitude())
		{
			m_power = 0.9f;
			GolfBall().m_initSpin = 0.0f;
		}
		else
		{
			m_power = 1.0f;
			GolfBall().m_initSpin = 1.0f;
		}
	}
	else
	{
		vec = GetHoleCupPos() - GolfBall().m_pos -
			Wind().GetGlobalWind() * factor;
		vec.y = 0.0f;
		m_power =
			Min(vec.Magnitude() / YARD_TO_POS(GolfClub().GetRange()), 1.0f);
	}

	float tanLoft = (float)tan((double)GolfClub().GetLie());
	vec = WVector(dir.x, tanLoft, dir.z) *
		(0.5f * YARD_TO_POS(GolfClub().GetRange()));
	vec.y -= 32.0f;

	factor = PVSEX().RayIntersectAll(GolfBall().m_pos, vec, true);
	if (factor < 1.0f)
	{
		if (factor > 0.5f && WisZero(GolfBall().m_initSpin) &&
			WisZero(m_curve) && PLAYER(GOLFDOC()->m_currentPlayer)->flag == 0)
		{
			if (PVSEX().RayIntersectAll(GolfBall().m_pos,
					vec * RotMat(10.0f * g_DEGTORAD, 1), true) < 1.0f)
			{
				GolfBall().m_initCurve = 1.0f;
				return;
			}
			else if (PVSEX().RayIntersectAll(GolfBall().m_pos,
						 vec * RotMat(-10.0f * g_DEGTORAD, 1), true) < 1.0f)
			{
				GolfBall().m_initCurve = -1.0f;
				return;
			}
			else
			{
				GolfBall().m_initSpin = 1.0f;
				return;
			}
		}
		else if (factor < 0.3f)
		{
			m_club = Min(m_club + 3, 13);
			GolfClub().SetClub(m_club);

			m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
			m_target = GolfBall().m_pos + (m_target - GolfBall().m_pos) * rate;

			dir = m_target - GolfBall().m_pos;
			dir.y = 0.0f;
			dir.Normalize();

			tanLoft = (float)tan((double)GolfClub().GetLie());
			vec = WVector(dir.x, tanLoft, dir.z) *
				(0.5f * YARD_TO_POS(GolfClub().GetRange()));
			vec.y -= 32.0f;

			float step =
				(GOLFDOC()->GetPlayer(RbCurrentPlayer(GOLFDOC()))->flag + 1) *
				(5.0f * g_DEGTORAD);

			if (PVSEX().RayIntersectAll(GolfBall().m_pos, vec, true) < 1.0f)
				return;

			if (PVSEX().RayIntersectAll(GolfBall().m_pos, vec * RotMat(step, 1),
					true) < 1.0f)
			{
				m_target = GolfBall().m_pos +
					rate * 0.8f * (m_target - GolfBall().m_pos) *
						RotMat(step * -1.0f, 1);
			}
			else if (PVSEX().RayIntersectAll(GolfBall().m_pos,
						 vec * RotMat(step * -1.0f, 1), true) < 1.0f)
			{
				m_target = GolfBall().m_pos +
					rate * 0.8f * (m_target - GolfBall().m_pos) *
						RotMat(step, 1);
			}
		}
	}
	else if (PVSEX().RayIntersectAll(GolfBall().m_pos + vec, vec, true) < 1.0f)
	{
		m_club = Min(m_club + 1, 12);
		GolfClub().SetClub(m_club);

		m_target = pDoc->m_pAutoTarget->GetPoint(NULL);
		m_target = GolfBall().m_pos + (m_target - GolfBall().m_pos) * rate;
	}
}

void CRival::SetResult()
{
	m_expected = GolfBall().PreData(GolfBall().m_renderNum)->pos;

	if (GolfBall().m_bOB || GolfBall().m_standGround <= 4)
	{
		if (GetPVS().GetObjColNum() > 0)
			m_mode = 1;
		else if (GolfBall().m_water == CGolfBall::WATER_RIVER ||
			GolfBall().m_water == CGolfBall::WATER_POOL)
			m_mode = 2;
		else if (GolfBall().m_water == CGolfBall::WATER_SEA)
			m_mode = 3;
		else
			m_mode = 4;

		m_curve = GolfBall().m_initCurve;
		m_spin = GolfBall().m_initSpin;
	}
	else
	{
		m_mode = 0;
		m_curve = 0.0f;
		m_spin = 0.0f;
	}
}

WVector CRival::GetGreenSlope()
{
	WVector dir;
	WVector pos;
	WVector slope;
	WVector normal;
	int num;

	dir = GetHoleCupPos() - GolfBall().m_pos;
	dir.y = 0.0f;
	num = (int)(dir.Magnitude() * 0.3125f);
	dir.Normalize();

	slope.Reset();
	pos = GolfBall().m_pos;

	for (int i = 0; i < num; i++)
	{
		pos += dir * 3.2f;
		normal = PVSEX().GetMeshNormal(pos);
		normal.y = 0.0f;
		slope += normal;
	}

	return slope;
}
