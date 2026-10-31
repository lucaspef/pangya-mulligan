#pragma once
#include "rebang.h"
#include "wtypes.h"
#include <math.h>
#include "wutil.h"
#include <string.h>

const float g_PI = 3.14159265358979323846f;
const float g_2_PI = 6.28318530717958647692f;
const float g_PI_DIV_2 = 1.57079632679489661923f;
const float g_DEGTORAD = 0.01745329251994329577f;
const float g_RADTODEG = 57.2957795130823208768f;
const float g_HUGE = 3.402823466e+38f;
const float g_EPSILON = 0.00001f;
const float g_CM_TO_WU = 0.32f;

namespace
{
	// This is a placeholder; there were probably inlines that used these
	// constants, but we have not recovered them yet.
	inline void InstantiateWangrealMathInlines()
	{
		(void)g_PI;
		(void)g_2_PI;
		(void)g_PI_DIV_2;
		(void)g_HUGE;
		(void)g_EPSILON;
		(void)g_CM_TO_WU;
	}
}

#ifdef REBANG_PCH_MATH_HELPERS
inline float Wabs(const float& value);
inline int WisEqual(const float& left, const float& right, float epsilon);
#else
#ifdef _ATL_STATIC_REGISTRY // como no rebang: s� no pch do ProjectG
inline float __fastcall RadianNormalize2PI(float angle)
{
	if (angle >= g_2_PI || angle < 0.0f)
		angle -= (float)floor(angle * 0.15915494f) * g_2_PI;
	return angle;
}

inline float __fastcall RadianNormalizePI(float angle)
{
	angle = RadianNormalize2PI(angle);
	if (angle > g_PI)
		angle -= g_2_PI;
	return angle;
}

inline float __fastcall RadianDelta(float a, float b)
{
	return RadianNormalizePI(a - b);
}
#endif

inline float Wabs(const float& value)
{
	return (*(const unsigned long*)&value & 0x80000000) ? -value : value;
}

inline int WisEqual(const float& left, const float& right, float epsilon)
{
	return Wabs(left - right) < epsilon;
}
#endif

class WVector2D
{
public:
	WVector2D() { }
	WVector2D(float x_, float y_)
		: x(x_), y(y_)
	{
	}
	void Reset();
	void operator+=(const WVector2D& v);
	float SquareMagnitude() const;
	float Magnitude() const;
	WVector2D& Normalize();
	void operator*=(float f);
	void operator/=(float f);
#ifdef REBANG_LEGACY_CPP
	WVector2D& operator=(const WVector2D& other);
#endif
	float x;
	float y;

	// 983: 5 constantes de 2 floats na .data antes de WVector::ONE, sem nenhuma referência (F2.10); nomes pelos do WVector
	static const WVector2D ONE;
	static const WVector2D UNIT_POS_X;
	static const WVector2D UNIT_POS_Y;
	static const WVector2D UNIT_NEG_X;
	static const WVector2D UNIT_NEG_Y;
};

class WMatrix;
class WQuat;

class WVector
{
public:
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		float p[3];
	};

	WVector() DEFAULT_IMPL;

	WVector(const float x_, const float y_, const float z_)
	{
		x = x_;
		y = y_;
		z = z_;
	}

	WVector(const float* v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
	}

#ifdef REBANG_LEGACY_CPP
#ifndef WANGREAL_DEVICE
	WVector& operator=(const WVector& other);
#endif
#else
	WVector& operator=(const WVector& other) = default;
#endif

	void Reset();
	void Set(float x_, float y_, float z_); // 983: nome provisório
	WVector operator-() const;
	void Negate();
	WVector& Normalize();
	void operator+=(const WVector& right);

	float Magnitude() const;

	float SquareMagnitude() const;

	void operator*=(float scalar);
	void operator/=(float scalar);
	void operator-=(const WVector& other);

	static const WVector ONE;
	static const WVector ZERO;
	static const WVector UNIT_POS_X;
	static const WVector UNIT_POS_Y;
	static const WVector UNIT_POS_Z;
	static const WVector UNIT_NEG_X;
	static const WVector UNIT_NEG_Y;
	static const WVector UNIT_NEG_Z;
};

class WVector4
{
public:
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
		float p[4];
	};

	WVector4() { }

	WVector4(float x_, float y_, float z_, float w_);

	static const WVector4 ONE;
	static const WVector4 ZERO;
};

class Waabb
{
public:
	Waabb() { Reset(); }
	Waabb(const WVector& minimum, const WVector& maximum)
	{
		min = minimum;
		max = maximum;
	}
	Waabb(short* minimum, short* maximum);

	WVector min;
	WVector max;

	Waabb& operator=(const Waabb& other);
	Waabb& operator+=(const Waabb& box);
	Waabb& operator+=(const WVector& vector);
	void Reset()
	{
		min.Reset();
		max.Reset();
	}
	void Default();
#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL (emission order)
	bool IsInclude(const Waabb& box) const
	{
		if (box.min.x >= min.x && box.max.x <= max.x && box.min.y >= min.y &&
			box.max.y <= max.y && box.min.z >= min.z && box.max.z <= max.z)
		{
			return true;
		}
		return false;
	}
#else
	bool IsInclude(const Waabb& box) const;
#endif
	bool IsInclude(const WVector& vector) const
	{
		if (min.x <= vector.x && vector.x <= max.x && min.y <= vector.y &&
			vector.y <= max.y && min.z <= vector.z && vector.z <= max.z)
			return true;
		return false;
	}
	bool IsInclude(WVector& vector) const;
	void Clear()
	{
		min = WVector(3.402823466e+38f, 3.402823466e+38f, 3.402823466e+38f);
		max = WVector(-3.402823466e+38f, -3.402823466e+38f, -3.402823466e+38f);
	}
	void AddPoint(const WVector& vector)
	{
		if (vector.x > max.x)
			max.x = vector.x;
		if (vector.x < min.x)
			min.x = vector.x;
		if (vector.y > max.y)
			max.y = vector.y;
		if (vector.y < min.y)
			min.y = vector.y;
		if (vector.z > max.z)
			max.z = vector.z;
		if (vector.z < min.z)
			min.z = vector.z;
	}
	WVector GetCenter() const;
};

class Wobb
{
public:
#ifdef WANGREAL_DEVICE
	Wobb();
#else
	Wobb() { }
#endif
#ifndef WANGREAL_DEVICE
	Wobb(const Wobb& obb) { *this = obb; }
#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL and name count
	Wobb(const WMatrix& mat) { *this = mat; }
#endif
#endif

	WVector center;
	WVector extend[3];

#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL and name count
	void operator+=(const WVector& v);
#endif
	void operator*=(const WVector& scale);
	Wobb& operator=(const WMatrix& mat);
	Wobb& operator=(const Wobb& obb);
	Wobb& operator=(const Waabb& box);
	bool IsInclude(const WVector& point) const;
};

class WMatrix
{
public:
	union
	{
		struct
		{
			WVector xa;
			WVector ya;
			WVector za;
			WVector pivot;
		};
		struct
		{
			float xx, yx, zx;
			float xy, yy, zy;
			float xz, yz, zz;
			float xm, ym, zm;
		};
		float p[12];
	};

	WMatrix() { }
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
	WMatrix(const class WPlane& plane);
#endif

#ifdef _ATL_STATIC_REGISTRY // HACK: only the ProjectG PCH defines it: the ctor is out of class there (wmath.inl), Fresh/Wangreal keep it in class (their COMDAT order)
	WMatrix(float xx_, float yx_, float zx_, float xy_, float yy_, float zy_,
		float xz_, float yz_, float zz_);
#else
	WMatrix(float xx_, float yx_, float zx_, float xy_, float yy_, float zy_,
		float xz_, float yz_, float zz_)
		: xx(xx_),
		  yx(yx_),
		  zx(zx_),
		  xy(xy_),
		  yy(yy_),
		  zy(zy_),
		  xz(xz_),
		  yz(yz_),
		  zz(zz_)
	{
		pivot.Reset();
	}
#endif

	WMatrix& operator=(const WMatrix& other);
	void operator=(const WQuat& quat);
	void Normalize();
#ifdef WANGREAL_DEVICE
	void Reset();
#else
	void Reset()
	{
		xx = yy = zz = 1.0f;
		yx = zx = xy = zy = xz = yz = xm = ym = zm = 0.0f;
	}
#endif
	void GetRotMatrix(WMatrix* result) const;
	WMatrix operator~() const;
	void operator*=(float right);
	void AxisScale(WVector& scale);
	void Rotate(float angle, char direct, bool pre = false); // 983: 3º argumento (multiplica pela esquerda); padrão false: as chamadas de 2 argumentos do 645 empilham 0 no 983 (sky 0070614a)
	void Rotate(const WVector& rotation);
	void operator*=(const WMatrix& right); // 983
	void GetAngles(WVector* angles) const; // 983: nome provisório

	static const WMatrix IDENTITY;
	static const WMatrix ZERO;
};

class WMatrix4
{
public:
	union
	{
		struct
		{
			WVector xa;
			float wx;
			WVector ya;
			float wy;
			WVector za;
			float wz;
			WVector pivot;
			float wm;
		};
		struct
		{
			float xx, yx, zx;
			unsigned char gapC[4];
			float xy, yy, zy;
			unsigned char gap1C[4];
			float xz, yz, zz;
			unsigned char gap2C[4];
			float xm, ym, zm;
		};

		float m[4][4];
		float p[16];
	};

	WMatrix4() { }

	WMatrix4(float p0, float p1, float p2, float p3, float p4, float p5,
		float p6, float p7, float p8, float p9, float p10, float p11, float p12,
		float p13, float p14, float p15);

	WMatrix4& operator=(const WMatrix4& other);

	static const WMatrix4 IDENTITY;
	static const WMatrix4 ZERO;
};

#ifdef WANGREAL_DEVICE
inline Wobb::Wobb()
{
}
#endif

class WPlane
{
public:
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float dis;
		};
		struct
		{
			float a;
			float b;
			float c;
			float d;
		};
		struct
		{
			WVector normal;
		};
	};

	WPlane() { }

	WPlane(const WVector& normal_, const WVector& point);

	WPlane(const WVector& normal_, float dis_)
	{
		x = normal_.x;
		y = normal_.y;
		z = normal_.z;
		dis = dis_;
	}

	WPlane(float x_, float y_, float z_, float dis_)
	{
		x = x_;
		y = y_;
		z = z_;
		dis = dis_;
	}

	WPlane& operator=(const WPlane& other);
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
	void Normalize();
#endif
};

class WSphere
{
public:
	WSphere() { }
	WSphere(const WVector& position, float length)
	{
		pos = position;
		radius = length;
	}

	WVector pos;
	float radius;
};

class WQuat
{
public:
	float x;
	float y;
	float z;
	float w;

	WQuat() DEFAULT_IMPL;

	WQuat(float x_, float y_, float z_, float w_)
	{
		x = x_;
		y = y_;
		z = z_;
		w = w_;
	}

	WQuat& operator=(const WQuat& other);
	void Reset();
	void Normalize();
	void ConvertToAxisAngle(WVector* vector, float* theta);
	void ConvertToRotationMatrix(WMatrix& rotation) const;
	void SetFromAngles(float yaw, float pitch, float roll);
	void operator=(const WMatrix& matrix);

private:
	void SetFromAxisAngle(const WVector& vector, float theta);
};

// 983: novos (unidade wd3d: 10012580 e 100127e0); WisZero é nome provisório
inline int WisZero(const float& value, float epsilon = g_EPSILON)
{
	return Wabs(value) < epsilon;
}

int WisEqual(const WVector& left, const WVector& right, float epsilon);

inline int WisEqual(const WMatrix4& left, const WMatrix4& right, float epsilon)
{
	return WisEqual(left.xa, right.xa, epsilon) &&
		WisEqual(left.wx, right.wx, epsilon) &&
		WisEqual(left.ya, right.ya, epsilon) &&
		WisEqual(left.wy, right.wy, epsilon) &&
		WisEqual(left.za, right.za, epsilon) &&
		WisEqual(left.wz, right.wz, epsilon) &&
		WisEqual(left.pivot, right.pivot, epsilon) &&
		WisEqual(left.wm, right.wm, epsilon);
}
Waabb operator+(const Waabb& box, const WVector& vector);
WMatrix operator*(const WMatrix& left, const WMatrix& right);
void __fastcall RotMat(float angle, char direct, WMatrix& out); // 983
WMatrix RotMat(float angle, char direct);
WMatrix RotMat(WVector rotation);
void __fastcall RotMat(const WVector& rotation, WMatrix& out); // 983 007a9760 (avatar_main.cpp; 9 chamadores)
WVector MinPointLineSegment(WVector& vp, WVector& va, WVector& vb, WVector* vt);
float WCollisionTest(const Waabb& origin, const WVector& vec,
	const Waabb& target, WPlane* plane);
float __cdecl CalcDeltaAngle(const WVector& v1, const WVector& v2);
void __cdecl InitMath();
void __cdecl UninitMath();

class WView;

void __cdecl MakePlaneEq(WPlane* out_plane, const Waabb& aabb,
	const WMatrix* matrix);
bool __fastcall PlaneFromVecs(WPlane* result, const WVector& first,
	const WVector& second, const WVector& point);
float __cdecl RayIntersect(const WVector& pivot, const WVector& vec,
	WPlane* plane, WPlane* out, float rlen);
float __cdecl ObbIntersect(const Waabb& src_aabb, const WVector& direction,
	const Wobb& dest_obb, WPlane* out_plane);
float __fastcall RayIntersectAABB(const Waabb& box, const WVector& start,
	const WVector& ray, WPlane* result, float radius);

float __cdecl RayIntersectAABB(const Waabb& box, const WVector& start,
	const WVector& ray, WMatrix* matrix, WPlane* result, float radius);
float __cdecl AabbIntersect(const Waabb& src_aabb, const WVector& direction,
	const Waabb& dest_aabb, WPlane* out_plane);
float __cdecl DotContact(const WVector& vec, const WPlane* plane);
WMatrix __cdecl Make3rdCamMatrix(const WVector& position,
	const WVector& target);
WRect __cdecl ScaleRect(WView* view, WRect* rectangle);
void __cdecl ScalePoint(WView* view, WPoint* point);

float WVectorLen(const WVector& vector);
WVector RotVec(const WVector& vector, const WMatrix& matrix);

#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
inline WVector Waabb::GetCenter() const
{
	WVector c((max.x + min.x) * 0.5f, (max.y + min.y) * 0.5f,
		(max.z + min.z) * 0.5f);
	return c;
}
#endif

#include "wmath.inl"

inline float __fastcall GetBaseAngle(float angle)
{
	if (Abs(angle) > g_2_PI)
		angle -= floorf(angle * (1.0f / g_2_PI)) * g_2_PI;
	if (angle > g_PI)
		return angle - g_2_PI;
	if (angle < -g_PI)
		angle += g_2_PI;
	return angle;
}
