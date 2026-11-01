// SPDX-License-Identifier: MIT
// wmath.h - vector/matrix/quaternion helpers.
//
// Same conventions as the game's Wangreal engine: row vectors (v' = v * M),
// matrices concatenate child * parent, and the members are named the way the
// BONE chunk stores them (xx yx zx / xy yy zy / xz yz zz / xm ym zm).
#ifndef WMATH_H
#define WMATH_H

#include <math.h>

struct WVector
{
	float x, y, z;

	WVector() : x(0.0f), y(0.0f), z(0.0f) { }
	WVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }

	WVector operator+(const WVector& o) const { return WVector(x + o.x, y + o.y, z + o.z); }
	WVector operator-(const WVector& o) const { return WVector(x - o.x, y - o.y, z - o.z); }
	WVector operator*(float f) const { return WVector(x * f, y * f, z * f); }
	float Dot(const WVector& o) const { return x * o.x + y * o.y + z * o.z; }
	float Length() const { return (float)sqrt(x * x + y * y + z * z); }
	WVector Cross(const WVector& o) const
	{
		return WVector(y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x);
	}
	void Normalize()
	{
		float len = Length();
		if (len > 0.000001f)
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}
};

struct WQuat
{
	float x, y, z, w;

	WQuat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) { }
	WQuat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) { }

	WQuat operator*(const WQuat& b) const
	{
		return WQuat(w * b.x + x * b.w + y * b.z - z * b.y,
			w * b.y - x * b.z + y * b.w + z * b.x,
			w * b.z + x * b.y - y * b.x + z * b.w,
			w * b.w - x * b.x - y * b.y - z * b.z);
	}
	void Normalize()
	{
		float len = (float)sqrt(x * x + y * y + z * z + w * w);
		if (len > 0.000001f)
		{
			x /= len;
			y /= len;
			z /= len;
			w /= len;
		}
	}
};

inline WQuat WSlerp(const WQuat& a, const WQuat& b, float t)
{
	float cosom = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float sign = 1.0f;
	if (cosom < 0.0f)
	{
		cosom = -cosom;
		sign = -1.0f;
	}
	float s0 = 1.0f - t;
	float s1 = t;
	if (cosom < 0.9995f)
	{
		float om = (float)acos(cosom);
		float sinom = (float)sin(om);
		s0 = (float)sin((1.0f - t) * om) / sinom;
		s1 = (float)sin(t * om) / sinom;
	}
	s1 *= sign;
	WQuat q(a.x * s0 + b.x * s1, a.y * s0 + b.y * s1, a.z * s0 + b.z * s1, a.w * s0 + b.w * s1);
	q.Normalize();
	return q;
}

struct WMatrix
{
	float xx, yx, zx;
	float xy, yy, zy;
	float xz, yz, zz;
	float xm, ym, zm;

	WMatrix() { Reset(); }

	void Reset()
	{
		xx = 1.0f; yx = 0.0f; zx = 0.0f;
		xy = 0.0f; yy = 1.0f; zy = 0.0f;
		xz = 0.0f; yz = 0.0f; zz = 1.0f;
		xm = 0.0f; ym = 0.0f; zm = 0.0f;
	}

	// wmath.cpp: WMatrix::operator=(const WQuat&)
	void SetRotation(const WQuat& q)
	{
		float _xx = 2.0f * q.x * q.x;
		float _yy = 2.0f * q.y * q.y;
		float _zz = 2.0f * q.z * q.z;
		float _xy = 2.0f * q.x * q.y;
		float _xz = 2.0f * q.x * q.z;
		float _yz = 2.0f * q.y * q.z;
		float _wx = 2.0f * q.w * q.x;
		float _wy = 2.0f * q.w * q.y;
		float _wz = 2.0f * q.w * q.z;
		xx = 1.0f - (_yy + _zz);
		yx = _xy - _wz;
		zx = _xz + _wy;
		xy = _xy + _wz;
		yy = 1.0f - (_xx + _zz);
		zy = _yz - _wx;
		xz = _xz - _wy;
		yz = _yz + _wx;
		zz = 1.0f - (_xx + _yy);
	}

	WVector Pivot() const { return WVector(xm, ym, zm); }
	void SetPivot(const WVector& v)
	{
		xm = v.x;
		ym = v.y;
		zm = v.z;
	}

	// Rotation-only transform (RotVec in the engine).
	WVector Rotate(const WVector& v) const
	{
		return WVector(v.x * xx + v.y * xy + v.z * xz,
			v.x * yx + v.y * yy + v.z * yz,
			v.x * zx + v.y * zy + v.z * zz);
	}

	WMatrix Inverse() const
	{
		WMatrix r;
		float cx = yy * zz - yz * zy;
		float cy = xz * zy - zz * xy;
		float cz = yz * xy - yy * xz;
		float det = xx * cx + yx * cy + zx * cz;
		float inv = det != 0.0f ? 1.0f / det : 1.0f;
		r.xx = cx * inv;
		r.yx = (yz * zx - yx * zz) * inv;
		r.zx = (yx * zy - zx * yy) * inv;
		r.xy = cy * inv;
		r.yy = (xx * zz - zx * xz) * inv;
		r.zy = (zx * xy - xx * zy) * inv;
		r.xz = cz * inv;
		r.yz = (yx * xz - yz * xx) * inv;
		r.zz = (yy * xx - yx * xy) * inv;
		r.xm = -(r.xx * xm + r.xy * ym + r.xz * zm);
		r.ym = -(r.yx * xm + r.yy * ym + r.yz * zm);
		r.zm = -(r.zx * xm + r.zy * ym + r.zz * zm);
		return r;
	}
};

inline WVector operator*(const WVector& v, const WMatrix& m)
{
	return WVector(v.x * m.xx + v.y * m.xy + v.z * m.xz + m.xm,
		v.x * m.yx + v.y * m.yy + v.z * m.yz + m.ym,
		v.x * m.zx + v.y * m.zy + v.z * m.zz + m.zm);
}

inline WMatrix operator*(const WMatrix& l, const WMatrix& r)
{
	WMatrix m;
	m.xx = l.xx * r.xx + l.yx * r.xy + l.zx * r.xz;
	m.yx = l.xx * r.yx + l.yx * r.yy + l.zx * r.yz;
	m.zx = l.xx * r.zx + l.yx * r.zy + l.zx * r.zz;
	m.xy = l.xy * r.xx + l.yy * r.xy + l.zy * r.xz;
	m.yy = l.xy * r.yx + l.yy * r.yy + l.zy * r.yz;
	m.zy = l.xy * r.zx + l.yy * r.zy + l.zy * r.zz;
	m.xz = l.xz * r.xx + l.yz * r.xy + l.zz * r.xz;
	m.yz = l.xz * r.yx + l.yz * r.yy + l.zz * r.yz;
	m.zz = l.xz * r.zx + l.yz * r.zy + l.zz * r.zz;
	m.xm = l.xm * r.xx + l.ym * r.xy + l.zm * r.xz + r.xm;
	m.ym = l.xm * r.yx + l.ym * r.yy + l.zm * r.yz + r.ym;
	m.zm = l.xm * r.zx + l.ym * r.zy + l.zm * r.zz + r.zm;
	return m;
}

#endif // WMATH_H
