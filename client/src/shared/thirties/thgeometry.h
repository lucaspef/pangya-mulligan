#pragma once

#include "gamath.h"

struct ThSphere
{
	gaV3 pos;
	float r;

	ThSphere() { }

	void Set(const gaV3& c, float radius) { Set(c[0], c[1], c[2], radius); }

	void Set(float x, float y, float z, float radius)
	{
		SetPos(x, y, z);
		r = radius;
	}

	void SetPos(float x, float y, float z)
	{
		pos.x = x;
		pos.y = y;
		pos.z = z;
	}

	bool TestIntersection(const ThSphere& other) const
	{
		gaV3 d = pos - other.pos;
		float rr = other.r + r;

		return d.SquaredLength() <= rr * rr;
	}
};
