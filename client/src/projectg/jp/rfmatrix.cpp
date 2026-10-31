// JP 983: produto de matrizes da Refresh (B158; 004620b0/00462100; o resto do RFMatrix é embutido no rfrenderer.h)
#include <string.h>
#include <math.h>
#include "refresh/lib/rfrenderer.h"

// 004620b0
RFMatrix& RFMatrix::operator*=(const RFMatrix& o)
{
	*this = *this * o;
	return *this;
}

// 00462100
RFMatrix RFMatrix::operator*(const RFMatrix& o) const
{
	RFMatrix r;
	memset(&r, 0, sizeof(r));
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			for (int k = 0; k < 4; ++k)
				r.m[i][j] += m[i][k] * o.m[k][j];
	return r;
}

// 00462190
RFMatrix& RFMatrix::Identity()
{
	memset(m, 0, sizeof(m));
	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	return *this;
}

// 004621d0
RFMatrix& RFMatrix::Translation(float x, float y, float z)
{
	Identity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
	return *this;
}

// 00462220: a rotação em Z (m[0][0] = cos, m[0][1] = sin, m[1][0] = -sin, m[1][1] = cos); o resto é a identidade
RFMatrix& RFMatrix::RotationZ(float angle)
{
	Identity();
	float s = sinf(angle);
	float c = cosf(angle);
	m[0][0] = c;
	m[0][1] = s;
	m[1][0] = -s;
	m[1][1] = c;
	return *this;
}

// 004622a0
RFMatrix& RFMatrix::Scaling(float x, float y, float z)
{
	Identity();
	m[0][0] = x;
	m[1][1] = y;
	m[2][2] = z;
	return *this;
}

// 004622f0: T(-centro) * escala * rotação Z (só se |rot| > 0.01) * T(+centro); sem centro = identidade nos extremos
RFMatrix& RFMatrix::Transformation2D(const RFVector2* center, const RFVector2* scale, float rotation)
{
	RFMatrix to, back, t;
	if (center)
	{
		to.Translation(-center->x, -center->y, 0.0f);
		back.Translation(center->x, center->y, 0.0f);
		*this = to;
	}
	else
		Identity();
	if (scale)
	{
		t.Scaling(scale->x, scale->y, 1.0f);
		*this = *this * t;
	}
	if (fabsf(rotation) > 0.01f)
	{
		t.RotationZ(rotation);
		*this = *this * t;
	}
	if (center)
		*this = *this * back;
	return *this;
}

// 00462510 (z = 0)
RFVector2 RFMatrix::Transform2DPoint(const RFVector2* pt) const
{
	RFVector2 r(0.0f, 0.0f);
	if (pt)
	{
		r.x = m[1][0] * pt->y + pt->x * m[0][0] + m[2][0] * 0.0f + m[3][0];
		r.y = m[0][1] * pt->x + m[1][1] * pt->y + m[2][1] * 0.0f + m[3][1];
	}
	return r;
}
