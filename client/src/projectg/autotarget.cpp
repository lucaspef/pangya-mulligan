#include "minatl.h"
#include "autotarget.h"
#include "club.h"
#include "golfball.h"
#include "quadtree.h"
#include "polysoup.h"

CAutoTarget::~CAutoTarget()
{
	Reset();
}

void CAutoTarget::Reset()
{
	m_pointList.clear();
}

float CAutoTarget::GetAngle()
{
	return CalcDeltaAngle(GetPoint(NULL) - GolfBall().m_pos,
		WVector::UNIT_POS_Z);
}

float CAutoTarget::InArea(const WVector& a, const WVector& b,
	float radius) const
{
	return sqrtf((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z)) -
		radius;
}

int CAutoTarget::CalcDropPos(const WVector& pos)
{
	unsigned char type = GetPVS().GetGroundType(pos, true, NULL);
	if (type < 3)
	{
		m_dropPos = PLAYER(GOLFDOC()->m_currentPlayer)->startPos;
		return 3;
	}

	if (GOLFDOC()->m_pPolySoup->IsOutOfBound(pos))
	{
		m_dropPos = PLAYER(GOLFDOC()->m_currentPlayer)->startPos;
		return 4;
	}

	float dist = 3200.0f;
	WVector dir;
	for (unsigned char i = 0; i < m_pointList.size() - 1; i++)
	{
		WVector nearest;
		nearest = GetNearest(i);
		float len = (nearest - pos).Magnitude();
		if (len < dist)
		{
			dist = len;
			dir = nearest - pos;
			dir.y = 0.0f;
		}
	}

	dist = 32.0f;
	dir.Normalize();

	do
	{
		type = GetPVS().GetGroundType(pos + dir * dist, true, NULL);
		if (type == 10)
		{
			m_dropPos = PLAYER(GOLFDOC()->m_currentPlayer)->startPos;
			return 2;
		}
		if (type == 1)
		{
			m_dropPos =
				GetPVS().GetGroundPoint(pos + dir * dist, false, NULL, false);
			m_dropPos.y += GolfBall().GetRadius();
			return 1;
		}
		dist += 32.0f;
	} while (dist < 192.0f);

	m_dropPos = PLAYER(GOLFDOC()->m_currentPlayer)->startPos;
	return 0;
}

WVector CAutoTarget::GetNearest(unsigned char index)
{
	float t = 0.0f;
	float nearest = 1.0f;
	float minDist = 3200.0f;
	WVector d = m_pointList[index + 1] - m_pointList[index];
	float step = 1.0f /
		(float)ceil((double)(sqrtf(d.x * d.x + d.z * d.z + d.y * d.y) / 16.0f));

	do
	{
		WVector pos;
		pos = m_pointList[index] * (1.0f - t) + m_pointList[index + 1] * t;
		if (GetPVS().GetGroundType(pos, true, NULL) == 1)
		{
			float dist = (GolfBall().m_pos - pos).Magnitude();
			if (dist < minDist)
			{
				minDist = dist;
				nearest = t;
			}
		}
		t += step;
	} while (t < 1.0f);

	return m_pointList[index] * (1.0f - nearest) +
		m_pointList[index + 1] * nearest;
}

float CAutoTarget::GetDistance(const WVector& a, const WVector& b)
{
	WVector d = a - b;
	d.y = 0.0f;
	return d.Magnitude();
}

WVector CAutoTarget::GetPoint(const char* club)
{
	WVector v[2];
	float range = (float)(GolfClub().GetRange(club) * (double)3.2f); // 983 006cd730: fmul qword 00cfe438 = 3.2000000476837 ((double)3.2f)

	if (club)
	{
		if (GolfClub().GetApproachMode() < 4 ||
			InArea(GolfBall().m_pos, GetHoleCupPos(), range) < 96.0f)
			return GetHoleCupPos();
	}
	else
	{
		if ((GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 0 ||
				(GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) == 0 &&
					GOLFDOC()->GetHoleData().par < 5)) &&
			(GolfClub().GetApproachMode() < 4 ||
				InArea(GolfBall().m_pos, GetHoleCupPos(), range) < 32.0f))
			return GetHoleCupPos();
	}

	for (int i = m_pointList.size() - 2; i >= 0; i--)
	{
		if (InArea(GolfBall().m_pos, m_pointList[i], range) *
				InArea(GolfBall().m_pos, m_pointList[i + 1], range) <
			0.0f)
		{
			WVector pos = SolveEquation(i, range) + GolfBall().m_pos;
			v[0] = pos - GolfBall().m_pos;
			v[1] = GetHoleCupPos() - GolfBall().m_pos;
			if (v[0].x * v[1].x + v[0].z * v[1].z > 0.0f &&
				GetDistance(v[1], v[0]) <
					sqrtf(v[1].x * v[1].x + v[1].z * v[1].z))
			{
				pos.y = 0.0f;
				return pos;
			}
		}
	}

	return GetHoleCupPos();
}

WVector CAutoTarget::SolveEquation(unsigned char index, float dist)
{
	WVector v[2];
	v[0] = m_pointList[index] - GolfBall().m_pos;
	v[1] = m_pointList[index + 1] - GolfBall().m_pos;
	float x[2], z[2];

	if (WisEqual(v[0].x, v[1].x, g_EPSILON))
	{
		x[0] = x[1] = v[0].x;
		z[0] = sqrtf(dist * dist - v[0].x * v[0].x);
		z[1] = z[0] * -1.0f;
	}
	else
	{
		float k = (v[1].z - v[0].z) / (v[1].x - v[0].x);
		float c = v[0].z - k * v[0].x;
		float det = (k * k + 1.0f) * dist * dist - c * c;
		x[0] = (sqrtf(det) - k * c) / (k * k + 1.0f);
		x[1] = (k * c * -1.0f - sqrtf(det)) / (k * k + 1.0f);
		z[0] = k * x[0] + c;
		z[1] = k * x[1] + c;
	}

	WVector result;
	if (x[0] >= Min(v[0].x, v[1].x) && x[0] <= Max(v[0].x, v[1].x))
	{
		result.x = x[0];
		result.z = z[0];
	}
	else
	{
		result.x = x[1];
		result.z = z[1];
	}
	return result;
}
