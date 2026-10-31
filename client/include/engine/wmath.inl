inline int WisEqual(const WVector& left, const WVector& right, float epsilon)
{
	if (WisEqual(left.x, right.x, epsilon) &&
		WisEqual(left.y, right.y, epsilon) &&
		WisEqual(left.z, right.z, epsilon))
		return 1;
	else
		return 0;
}

inline WVector WVector::operator-() const
{
	return WVector(-x, -y, -z);
}

inline WVector operator+(const WVector& left, const WVector& right)
{
	return WVector(left.x + right.x, left.y + right.y, left.z + right.z);
}

inline WVector operator-(const WVector& left, const WVector& right)
{
	return WVector(left.x - right.x, left.y - right.y, left.z - right.z);
}

inline WVector operator*(const WVector& vector, float scalar)
{
	return WVector(vector.x * scalar, vector.y * scalar, vector.z * scalar);
}

inline WVector operator*(float scalar, const WVector& vector)
{
	return WVector(scalar * vector.x, scalar * vector.y, scalar * vector.z);
}

inline float operator*(const WVector& left, const WVector& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

inline WVector WCrossProduct(const WVector& left, const WVector& right)
{
	return WVector(left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x);
}

inline float WVectorLen(const WVector& vector)
{
	return sqrtf(
		vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

#if defined(REBANG_LEGACY_CPP) && !defined(WANGREAL_DEVICE)
inline WVector& WVector::operator=(const WVector& other)
{
	memcpy(this, &other, sizeof(WVector));
	return *this;
}
#endif

inline void WVector::operator+=(const WVector& right)
{
	x = right.x + x;
	y = right.y + y;
	z = right.z + z;
}

inline void WVector::operator-=(const WVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
}

inline int operator==(const WVector& left, const WVector& right)
{
	return WisEqual(left, right, g_EPSILON);
}

#ifdef _ATL_STATIC_REGISTRY // só no pch do ProjectG (como no rebang); texto do rebang, não lido no 983
inline int operator!=(const WVector& left, const WVector& right)
{
	return !WisEqual(left, right, 1.0e-5f);
}
#endif

inline void WVector::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

inline void WVector::operator/=(float scalar)
{
	*this *= 1.0f / scalar;
}

inline void WVector::Set(float x_, float y_, float z_)
{
	x = x_;
	y = y_;
	z = z_;
}
inline void WVector::Reset()
{
	x = y = z = 0.0f;
}

inline float WVector::SquareMagnitude() const
{
	return x * x + y * y + z * z;
}

inline float WVector::Magnitude() const
{
	return WVectorLen(*this);
}

inline WVector& WVector::Normalize()
{
	if (WisEqual(*this, ZERO, g_EPSILON))
		*this *= 0.0f;
	else
		*this *= (float)((1.0f / (Magnitude())));
	return *this;
}

inline WVector operator*(const WVector& vector, const WMatrix& matrix)
{
	return WVector(vector.x * matrix.xx + vector.y * matrix.xy +
			vector.z * matrix.xz + matrix.xm,
		vector.x * matrix.yx + vector.y * matrix.yy + vector.z * matrix.yz +
			matrix.ym,
		vector.x * matrix.zx + vector.y * matrix.zy + vector.z * matrix.zz +
			matrix.zm);
}

inline WMatrix operator*(const WMatrix& left, const WMatrix& right)
{
	WMatrix m;
	m.xx = left.xx * right.xx + left.yx * right.xy + left.zx * right.xz;
	m.yx = left.xx * right.yx + left.yx * right.yy + left.zx * right.yz;
	m.zx = left.xx * right.zx + left.yx * right.zy + left.zx * right.zz;
	m.xy = left.xy * right.xx + left.yy * right.xy + left.zy * right.xz;
	m.yy = left.xy * right.yx + left.yy * right.yy + left.zy * right.yz;
	m.zy = left.xy * right.zx + left.yy * right.zy + left.zy * right.zz;
	m.xz = left.xz * right.xx + left.yz * right.xy + left.zz * right.xz;
	m.yz = left.xz * right.yx + left.yz * right.yy + left.zz * right.yz;
	m.zz = left.xz * right.zx + left.yz * right.zy + left.zz * right.zz;
	m.xm =
		left.xm * right.xx + left.ym * right.xy + left.zm * right.xz + right.xm;
	m.ym =
		left.xm * right.yx + left.ym * right.yy + left.zm * right.yz + right.ym;
	m.zm =
		left.xm * right.zx + left.ym * right.zy + left.zm * right.zz + right.zm;
	return m;
}

// 983: rotaÃ§Ã£o em torno de um eixo (0 x, 1 y, 2 z; 3..5 = sentido contrÃ¡rio)
inline void __fastcall RotMat(float angle, char direct, WMatrix& out)
{
	if (direct >= 3)
	{
		direct -= 3;
		angle *= -1.0;
	}
	float c = cosf(angle);
	float s = sinf(angle);
	switch (direct)
	{
	case 0:
		out.xx = 1.0f;
		out.yx = 0.0f;
		out.zx = 0.0f;
		out.xy = 0.0f;
		out.yy = c;
		out.zy = -s;
		out.xz = 0.0f;
		out.yz = s;
		out.zz = c;
		break;
	case 1:
		out.xx = c;
		out.yx = 0.0f;
		out.zx = s;
		out.xy = 0.0f;
		out.yy = 1.0f;
		out.zy = 0.0f;
		out.xz = -s;
		out.yz = 0.0f;
		out.zz = c;
		break;
	case 2:
		out.xx = c;
		out.yx = s;
		out.zx = 0.0f;
		out.xy = -s;
		out.yy = c;
		out.zy = 0.0f;
		out.xz = 0.0f;
		out.yz = 0.0f;
		out.zz = 1.0f;
		break;
	}
	out.pivot.Reset();
}
inline WMatrix RotMat(float angle, char direct)
{
	WMatrix m;
	RotMat(angle, direct, m);
	return m;
}
inline void WMatrix::operator*=(const WMatrix& right)
{
	*this = *this * right;
}
inline WVector RotVec(const WVector& vector, const WMatrix& matrix)
{
	return WVector(vector.x * matrix.xx + vector.y * matrix.xy +
			vector.z * matrix.xz,
		vector.x * matrix.yx + vector.y * matrix.yy + vector.z * matrix.yz,
		vector.x * matrix.zx + vector.y * matrix.zy + vector.z * matrix.zz);
}

inline float __fastcall TransformZ(float x, float y, float z, const WMatrix& m)
{
	return x * m.zx + y * m.zy + z * m.zz + m.zm;
}

inline float __fastcall TransformZ(const WVector& v, const WMatrix& m)
{
	return (v.z * m.zz) + (v.x * m.zx) + (v.y * m.zy) + m.zm;
}

inline WMatrix& WMatrix::operator=(const WMatrix& other)
{
	memcpy(&xa, &other.xa, sizeof(WVector));
	memcpy(&ya, &other.ya, sizeof(WVector));
	memcpy(&za, &other.za, sizeof(WVector));
	memcpy(&pivot, &other.pivot, sizeof(WVector));
	return *this;
}

#ifdef _ATL_STATIC_REGISTRY
inline WMatrix::WMatrix(float xx_, float yx_, float zx_, float xy_, float yy_,
	float zy_, float xz_, float yz_, float zz_)
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
	pivot.x = pivot.y = pivot.z = 0.0f;
}
#endif

inline void WMatrix::operator*=(float scalar)
{
	xa *= scalar;
	ya *= scalar;
	za *= scalar;
}

#if defined(REBANG_PCH_POOL) && !defined(REBANG_SEMANTIC_ONLY)
extern "C" int rb_real_3f800000;
extern "C" int rb_real_00000000;
#pragma comment(linker, "/alternatename:_rb_real_3f800000=__real@3f800000")
#pragma comment(linker, "/alternatename:_rb_real_00000000=__real@00000000")

// HACK: the original loads the operands of six of the products in the
// other order (e.g. zz * xy as fld zz; fmul xy). The compiler picks that
// order itself and no C form tried changes it (operand order, xa/ya/za,
// p[], this->, float locals, declaration counts). pool.obj holds the copy
// the linker keeps. The C equivalent is in the #else.
// clang-format off
inline __declspec(naked) WMatrix WMatrix::operator~() const
{
	__asm _emit 0x55 // push ebp
	__asm _emit 0x8b __asm _emit 0xec // mov ebp, esp
	__asm _emit 0x83 __asm _emit 0xec __asm _emit 0x40 // sub esp, 0x40
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x10 // fld [ecx + 0x10]
	__asm _emit 0x56 // push esi
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x20 // fmul [ecx + 0x20]
	__asm _emit 0x57 // push edi
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x1c // fld [ecx + 0x1c]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x14 // fmul [ecx + 0x14]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x18 // fld [ecx + 0x18]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x14 // fmul [ecx + 0x14]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x20 // fld [ecx + 0x20]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x0c // fmul [ecx + 0xc]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x1c // fld [ecx + 0x1c]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x0c // fmul [ecx + 0xc]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x10 // fld [ecx + 0x10]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x18 // fmul [ecx + 0x18]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd9 __asm _emit 0x55 __asm _emit 0xf8 // fst [ebp - 8]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x08 // fmul [ecx + 8]
	__asm _emit 0xd9 __asm _emit 0xc1 // fld st(1)
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x04 // fmul [ecx + 4]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0xc2 // fld st(2)
	__asm _emit 0xd8 __asm _emit 0x09 // fmul [ecx]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm fdivr dword ptr [rb_real_3f800000]
	__asm _emit 0xd9 __asm _emit 0x55 __asm _emit 0xfc // fst [ebp - 4]
	__asm _emit 0xd8 __asm _emit 0xca // fmul st(2)
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xc0 // fstp [ebp - 0x40]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x1c // fld [ecx + 0x1c]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x08 // fmul [ecx + 8]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x04 // fld [ecx + 4]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x20 // fmul [ecx + 0x20]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xc4 // fstp [ebp - 0x3c]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x04 // fld [ecx + 4]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x14 // fmul [ecx + 0x14]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x08 // fld [ecx + 8]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x10 // fmul [ecx + 0x10]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xc8 // fstp [ebp - 0x38]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xfc // fld [ebp - 4]
	__asm _emit 0xd8 __asm _emit 0xc9 // fmul st(1)
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xcc // fstp [ebp - 0x34]
	__asm _emit 0xdd __asm _emit 0xd8 // fstp st(0)
	__asm _emit 0xdd __asm _emit 0xd8 // fstp st(0)
	__asm _emit 0xd9 __asm _emit 0x01 // fld [ecx]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x20 // fmul [ecx + 0x20]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x08 // fld [ecx + 8]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x18 // fmul [ecx + 0x18]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xd0 // fstp [ebp - 0x30]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x08 // fld [ecx + 8]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x0c // fmul [ecx + 0xc]
	__asm _emit 0xd9 __asm _emit 0x01 // fld [ecx]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x14 // fmul [ecx + 0x14]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xd4 // fstp [ebp - 0x2c]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xfc // fld [ebp - 4]
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xf8 // fmul [ebp - 8]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xd8 // fstp [ebp - 0x28]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x04 // fld [ecx + 4]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x18 // fmul [ecx + 0x18]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x1c // fld [ecx + 0x1c]
	__asm _emit 0xd8 __asm _emit 0x09 // fmul [ecx]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xdc // fstp [ebp - 0x24]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x10 // fld [ecx + 0x10]
	__asm _emit 0x8b __asm _emit 0x45 __asm _emit 0x08 // mov eax, [ebp + 8]
	__asm _emit 0xd8 __asm _emit 0x09 // fmul [ecx]
	__asm _emit 0x8d __asm _emit 0x75 __asm _emit 0xc0 // lea esi, [ebp - 0x40]
	__asm _emit 0xd9 __asm _emit 0x41 __asm _emit 0x04 // fld [ecx + 4]
	__asm _emit 0x8b __asm _emit 0xf8 // mov edi, eax
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x0c // fmul [ecx + 0xc]
	__asm _emit 0xde __asm _emit 0xe9 // fsubp st(1)
	__asm _emit 0xd8 __asm _emit 0x4d __asm _emit 0xfc // fmul [ebp - 4]
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xe0 // fstp [ebp - 0x20]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xc0 // fld [ebp - 0x40]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x24 // fmul [ecx + 0x24]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xd8 // fld [ebp - 0x28]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x2c // fmul [ecx + 0x2c]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xcc // fld [ebp - 0x34]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x28 // fmul [ecx + 0x28]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0xe0 // fchs
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xe4 // fstp [ebp - 0x1c]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xc4 // fld [ebp - 0x3c]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x24 // fmul [ecx + 0x24]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xdc // fld [ebp - 0x24]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x2c // fmul [ecx + 0x2c]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xd0 // fld [ebp - 0x30]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x28 // fmul [ecx + 0x28]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0xe0 // fchs
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xe8 // fstp [ebp - 0x18]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xc8 // fld [ebp - 0x38]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x24 // fmul [ecx + 0x24]
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xe0 // fld [ebp - 0x20]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x2c // fmul [ecx + 0x2c]
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0x45 __asm _emit 0xd4 // fld [ebp - 0x2c]
	__asm _emit 0xd8 __asm _emit 0x49 __asm _emit 0x28 // fmul [ecx + 0x28]
	__asm _emit 0xb9 __asm _emit 0x0c __asm _emit 0x00 __asm _emit 0x00 __asm _emit 0x00 // mov ecx, 0xc
	__asm _emit 0xde __asm _emit 0xc1 // faddp st(1)
	__asm _emit 0xd9 __asm _emit 0xe0 // fchs
	__asm _emit 0xd9 __asm _emit 0x5d __asm _emit 0xec // fstp [ebp - 0x14]
	__asm _emit 0xf3 __asm _emit 0xa5 // rep movsd es:[edi], [esi]
	__asm _emit 0x5f // pop edi
	__asm _emit 0x5e // pop esi
	__asm _emit 0x8b __asm _emit 0xe5 // mov esp, ebp
	__asm _emit 0x5d // pop ebp
	__asm _emit 0xc2 __asm _emit 0x04 __asm _emit 0x00 // ret 4
}
// clang-format on
#else
inline WMatrix WMatrix::operator~() const
{
	WMatrix r;
	WVector c;
	c.x = yy * zz - yz * zy;
	c.y = xz * zy - zz * xy;
	c.z = yz * xy - yy * xz;
	float inv = 1.0f / (xx * c.x + yx * c.y + zx * c.z);
	r.xx = c.x * inv;
	r.yx = (yz * zx - yx * zz) * inv;
	r.zx = (yx * zy - zx * yy) * inv;
	r.xy = c.y * inv;
	r.yy = (xx * zz - zx * xz) * inv;
	r.zy = (zx * xy - xx * zy) * inv;
	r.xz = c.z * inv;
	r.yz = (yx * xz - yz * xx) * inv;
	r.zz = (yy * xx - yx * xy) * inv;
	r.xm = -(r.xx * xm + r.xy * ym + r.xz * zm);
	r.ym = -(r.yx * xm + r.yy * ym + r.yz * zm);
	r.zm = -(r.zx * xm + r.zy * ym + r.zz * zm);
	return r;
}
#endif

#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
inline WMatrix::WMatrix(const WPlane& plane)
{
	WPlane p = plane;
	p.Normalize();
	xx = 1.0f - p.x * p.x * 2.0f;
	yx = p.y * p.x * -2.0f;
	zx = p.z * p.x * -2.0f;
	xy = p.x * p.y * -2.0f;
	yy = 1.0f - p.y * p.y * 2.0f;
	zy = p.z * p.y * -2.0f;
	xz = p.x * p.z * -2.0f;
	yz = p.y * p.z * -2.0f;
	zz = 1.0f - p.z * p.z * 2.0f;
	xm = p.x * p.dis * -2.0f;
	ym = p.y * p.dis * -2.0f;
	zm = p.z * p.dis * -2.0f;
}
#endif

inline bool operator==(const WMatrix& left, const WMatrix& right)
{
	return WisEqual(left.xa, right.xa, 1.0e-5f) &&
		WisEqual(left.ya, right.ya, 1.0e-5f) &&
		WisEqual(left.za, right.za, 1.0e-5f) &&
		WisEqual(left.pivot, right.pivot, 1.0e-5f);
}

inline bool operator!=(const WMatrix& left, const WMatrix& right)
{
	return !(left == right);
}

inline void __fastcall SetWMatrix4FromWMatrix(WMatrix4& d, const WMatrix& s)
{
	for (int i = 0; i < 3; ++i)
	{
		*(WVector*)d.m[i] = (&s.xa)[i];
		d.m[i][3] = 0.0f;
	}
	*(WVector*)d.m[3] = s.pivot;
	d.m[3][3] = 1.0f;
}

inline WMatrix4::WMatrix4(float p0, float p1, float p2, float p3, float p4,
	float p5, float p6, float p7, float p8, float p9, float p10, float p11,
	float p12, float p13, float p14, float p15)
{
	p[0] = p0;
	p[1] = p1;
	p[2] = p2;
	p[3] = p3;
	p[4] = p4;
	p[5] = p5;
	p[6] = p6;
	p[7] = p7;
	p[8] = p8;
	p[9] = p9;
	p[10] = p10;
	p[11] = p11;
	p[12] = p12;
	p[13] = p13;
	p[14] = p14;
	p[15] = p15;
}

#ifdef WANGREAL_DEVICE
inline void WMatrix::Reset()
{
	xx = yy = zz = 1.0f;
	yx = zx = xy = zy = xz = yz = xm = ym = zm = 0.0f;
}
#endif

inline WMatrix4& WMatrix4::operator=(const WMatrix4& other)
{
	memcpy(this, &other, sizeof(WMatrix4));
	return *this;
}

inline WMatrix4 operator*(const WMatrix4& a, const WMatrix4& b)
{
	WMatrix4 m;
	m.m[0][0] = a.m[0][0] * b.m[0][0] + a.m[0][1] * b.m[1][0] +
		a.m[0][2] * b.m[2][0] + a.m[0][3] * b.m[3][0];
	m.m[0][1] = a.m[0][0] * b.m[0][1] + a.m[0][1] * b.m[1][1] +
		a.m[0][2] * b.m[2][1] + a.m[0][3] * b.m[3][1];
	m.m[0][2] = a.m[0][0] * b.m[0][2] + a.m[0][1] * b.m[1][2] +
		a.m[0][2] * b.m[2][2] + a.m[0][3] * b.m[3][2];
	m.m[0][3] = a.m[0][0] * b.m[0][3] + a.m[0][1] * b.m[1][3] +
		a.m[0][2] * b.m[2][3] + a.m[0][3] * b.m[3][3];

	m.m[1][0] = a.m[1][0] * b.m[0][0] + a.m[1][1] * b.m[1][0] +
		a.m[1][2] * b.m[2][0] + a.m[1][3] * b.m[3][0];
	m.m[1][1] = a.m[1][0] * b.m[0][1] + a.m[1][1] * b.m[1][1] +
		a.m[1][2] * b.m[2][1] + a.m[1][3] * b.m[3][1];
	m.m[1][2] = a.m[1][0] * b.m[0][2] + a.m[1][1] * b.m[1][2] +
		a.m[1][2] * b.m[2][2] + a.m[1][3] * b.m[3][2];
	m.m[1][3] = a.m[1][0] * b.m[0][3] + a.m[1][1] * b.m[1][3] +
		a.m[1][2] * b.m[2][3] + a.m[1][3] * b.m[3][3];

	m.m[2][0] = a.m[2][0] * b.m[0][0] + a.m[2][1] * b.m[1][0] +
		a.m[2][2] * b.m[2][0] + a.m[2][3] * b.m[3][0];
	m.m[2][1] = a.m[2][0] * b.m[0][1] + a.m[2][1] * b.m[1][1] +
		a.m[2][2] * b.m[2][1] + a.m[2][3] * b.m[3][1];
	m.m[2][2] = a.m[2][0] * b.m[0][2] + a.m[2][1] * b.m[1][2] +
		a.m[2][2] * b.m[2][2] + a.m[2][3] * b.m[3][2];
	m.m[2][3] = a.m[2][0] * b.m[0][3] + a.m[2][1] * b.m[1][3] +
		a.m[2][2] * b.m[2][3] + a.m[2][3] * b.m[3][3];

	m.m[3][0] = a.m[3][0] * b.m[0][0] + a.m[3][1] * b.m[1][0] +
		a.m[3][2] * b.m[2][0] + a.m[3][3] * b.m[3][0];
	m.m[3][1] = a.m[3][0] * b.m[0][1] + a.m[3][1] * b.m[1][1] +
		a.m[3][2] * b.m[2][1] + a.m[3][3] * b.m[3][1];
	m.m[3][2] = a.m[3][0] * b.m[0][2] + a.m[3][1] * b.m[1][2] +
		a.m[3][2] * b.m[2][2] + a.m[3][3] * b.m[3][2];
	m.m[3][3] = a.m[3][0] * b.m[0][3] + a.m[3][1] * b.m[1][3] +
		a.m[3][2] * b.m[2][3] + a.m[3][3] * b.m[3][3];
	return m;
}

inline WVector2D operator*(float scale, const WVector2D& v)
{
	return WVector2D(scale * v.x, scale * v.y);
}

inline WVector2D operator*(const WVector2D& v, float scale)
{
	return WVector2D(v.x * scale, v.y * scale);
}

inline WVector2D operator+(const WVector2D& a, const WVector2D& b)
{
	return WVector2D(a.x + b.x, a.y + b.y);
}

inline WVector2D operator-(const WVector2D& a, const WVector2D& b)
{
	return WVector2D(a.x - b.x, a.y - b.y);
}

inline void WVector2D::Reset()
{
	x = y = 0.0f;
}

inline float WVector2D::Magnitude() const
{
	return sqrtf(SquareMagnitude());
}

inline float WVector2D::SquareMagnitude() const
{
	return x * x + y * y;
}

#ifdef REBANG_LEGACY_CPP
inline WVector2D& WVector2D::operator=(const WVector2D& other)
{
	x = other.x;
	y = other.y;
	return *this;
}
#endif

inline WVector2D& WVector2D::Normalize()
{
	if (WisZero(x, g_EPSILON) && WisZero(y, g_EPSILON))
		*this *= 0.0f;
	else
		*this *= 1.0f / Magnitude();
	return *this;
}
inline void WVector2D::operator+=(const WVector2D& v)
{
	x += v.x;
	y += v.y;
}

inline void WVector2D::operator*=(float f)
{
	x *= f;
	y *= f;
}

inline void WVector2D::operator/=(float f)
{
	float inv = 1.0f / f;
	x *= inv;
	y *= inv;
}

inline float operator*(const WPlane& plane, const WVector& vector)
{
	return plane.normal * vector + plane.dis;
}

inline float operator*(const WVector& vector, const WPlane& plane)
{
	const WVector& value = vector;
	return plane.normal * value + plane.dis;
}

inline WPlane::WPlane(const WVector& normal_, const WVector& point)
	: normal(normal_)
{
	dis = -(normal * point);
}

inline WPlane& WPlane::operator=(const WPlane& other)
{
	normal = other.normal;
	dis = other.dis;
	return *this;
}

inline float WDotProduct(const WVector& left, const WVector& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

inline float WDotProduct(const WQuat& left, const WQuat& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z +
		left.w * right.w;
}

inline WQuat& WQuat::operator=(const WQuat& other)
{
	memcpy(this, &other, sizeof(WQuat));
	return *this;
}

#ifdef REBANG_SEMANTIC_ONLY
inline WMatrix __fastcall RotMat(const WVector& axis, float angle)
{
	WVector a = axis;
	float c = (float)cos(angle);
	float s = (float)sin(angle);
	a.Normalize();
	float k = 1.0f - c;
	WMatrix m;
	m.xm = 0.0f;
	m.ym = 0.0f;
	m.zm = 0.0f;
	m.xx = a.x * a.x * k + c;
	float xy = a.x * a.y * k;
	m.yx = xy - a.z * s;
	float xz = a.x * a.z * k;
	m.zx = a.y * s + xz;
	m.xy = a.z * s + xy;
	m.yy = a.y * a.y * k + c;
	float yz = a.y * a.z * k;
	m.zy = yz - a.x * s;
	m.xz = xz - a.y * s;
	m.yz = a.x * s + yz;
	m.zz = a.z * a.z * k + c;
	return m;
}
#elif defined(REBANG_PROJECTG_PCH)
// HACK: the body exists only in the ProjectG PCH (camerawork owns the COMDAT); elsewhere the dead
// reference in WQuaternionSlerp must not find a body, or Slerp waits one wave (wboneset order).
#pragma comment(linker, \
	"/alternatename:?rb_wmath_real_3f800000@@3HA=__real@3f800000")
// HACK: RotMat only differs in register allocation / stack layout from every C variant tried;
// byte-exact naked asm generated by local/tools/nakedify.py from the original object.
// The C version is in the #else. No dead C copy and no labels (PCH canaries, see stdafx.h RotMat).
inline __declspec(naked) WMatrix __fastcall RotMat(const WVector& axis,
	float angle)
{
	// block-scope alias: a namespace-scope name here would change the PCH name count (canaries)
	extern int rb_wmath_real_3f800000;
	// clang-format off
	__asm {
		push ebp
		mov ebp, esp
		sub esp, 18h
		fld dword ptr [ebp+8]
		mov eax, dword ptr [edx]
		fcos
		push esi
		mov esi, ecx
		mov ecx, dword ptr [edx+4]
		mov edx, dword ptr [edx+8]
		mov dword ptr [ebp-14h], ecx
		lea ecx, [ebp-18h]
		mov dword ptr [ebp-18h], eax
		mov dword ptr [ebp-10h], edx
		fstp dword ptr [ebp-4]
		fld dword ptr [ebp+8]
		fsin
		fstp dword ptr [ebp+8]
		call WVector::Normalize
		fld dword ptr [rb_wmath_real_3f800000]
		fsub dword ptr [ebp-4]
		xor eax, eax
		fld dword ptr [ebp-18h]
		mov dword ptr [esi+2Ch], eax
		fmul dword ptr [ebp-18h]
		mov dword ptr [esi+28h], eax
		mov dword ptr [esi+24h], eax
		mov eax, esi
		fmul st, st(1)
		fadd dword ptr [ebp-4]
		fstp dword ptr [esi]
		fld dword ptr [ebp-18h]
		fmul dword ptr [ebp-14h]
		fmul st, st(1)
		fld dword ptr [ebp-10h]
		fmul dword ptr [ebp+8]
		fld st(1)
		fsub st, st(1)
		fstp dword ptr [esi+4]
		fld dword ptr [ebp-18h]
		fmul dword ptr [ebp-10h]
		fmul st, st(3)
		fstp dword ptr [ebp-8]
		fld dword ptr [ebp-14h]
		fmul dword ptr [ebp+8]
		fst dword ptr [ebp-0Ch]
		fadd dword ptr [ebp-8]
		fstp dword ptr [esi+8]
		fadd st, st(1)
		fstp dword ptr [esi+0Ch]
		fstp st(0)
		fld dword ptr [ebp-14h]
		fmul dword ptr [ebp-14h]
		fmul st, st(1)
		fadd dword ptr [ebp-4]
		fstp dword ptr [esi+10h]
		fld dword ptr [ebp-14h]
		fmul dword ptr [ebp-10h]
		fmul st, st(1)
		fld dword ptr [ebp-18h]
		fmul dword ptr [ebp+8]
		fld st(1)
		fsub st, st(1)
		fstp dword ptr [esi+14h]
		fld dword ptr [ebp-8]
		fsub dword ptr [ebp-0Ch]
		fstp dword ptr [esi+18h]
		fadd st, st(1)
		fstp dword ptr [esi+1Ch]
		fstp st(0)
		fld dword ptr [ebp-10h]
		fmul dword ptr [ebp-10h]
		fmul st, st(1)
		fadd dword ptr [ebp-4]
		fstp dword ptr [esi+20h]
		fstp st(0)
		pop esi
		mov esp, ebp
		pop ebp
		ret 4
	}
	// clang-format on
	// dead reference to a template instantiation: RotMat waits one wave, as in the original (emission order)
	if (0) Max(angle, angle);
}
#else
WMatrix __fastcall RotMat(const WVector& axis, float angle);
#endif

inline WQuat WQuaternionSlerp(const WQuat& from, const WQuat& to, float t)
{
	WQuat from1;
	float omega, cosom, sinom, scale0, scale1;

	// HACK: dead reference; in the ProjectG PCH Slerp is compiled right after RotMat, one wave late,
	// as in the original (emission order)
	if (0)
		RotMat(*(const WVector*)0, t);

	cosom = WDotProduct(from, to);

	if (cosom < 0.0f)
	{
		from1 = WQuat(-from.x, -from.y, -from.z, -from.w);
		cosom = -cosom;
	}
	else
		from1 = from;

	if ((1.0f + cosom) > 0.05f)
	{
		if ((1.0f - cosom) < 0.05f)
		{
			scale0 = 1.0f - t;
			scale1 = t;
		}
		else
		{
			omega = acosf(cosom);
			sinom = sinf(omega);

			scale0 = sinf((1.0f - t) * omega) / sinom;
			scale1 = sinf(t * omega) / sinom;
		}

		return WQuat(scale0 * from1.x + scale1 * to.x,
			scale0 * from1.y + scale1 * to.y, scale0 * from1.z + scale1 * to.z,
			scale0 * from1.w + scale1 * to.w);
	}
	else
	{
		scale0 = sinf((1.0f - t) * g_PI_DIV_2);
		scale1 = sinf(t * g_PI_DIV_2);

		return WQuat(scale0 * from1.x - scale1 * from1.y,
			scale0 * from1.y + scale1 * from1.x,
			scale0 * from1.z - scale1 * from1.w,
			scale0 * from1.w + scale1 * from1.z);
	}
}

inline int operator&(const Waabb& left, const Waabb& right)
{
	for (int i = 0; i < 3; i++)
	{
		if (left.min.p[i] > right.max.p[i] || left.max.p[i] < right.min.p[i])
			return 0;
	}
	return 1;
}

inline Waabb& Waabb::operator=(const Waabb& other)
{
	min = other.min;
	max = other.max;
	return *this;
}

#if 1 // 983
inline Waabb operator+(const Waabb& box, const WVector& vector)
{
	Waabb result = box;
	result.max.x += vector.x;
	result.min.x += vector.x;
	result.max.y += vector.y;
	result.min.y += vector.y;
	result.max.z += vector.z;
	result.min.z += vector.z;
	return result;
}
#endif

#ifdef REBANG_PCH_REALMYROOMDATA // HACK: realmyroomdata PCH only (original wmath.inl:1050), other units keep their IL
inline void Wobb::operator*=(const WVector& scale)
{
	extend[0] *= scale.x;
	extend[1] *= scale.y;
	extend[2] *= scale.z;
}
#endif

inline Wobb& Wobb::operator=(const Wobb& obb)
{
	center = obb.center;
	extend[0] = obb.extend[0];
	extend[1] = obb.extend[1];
	extend[2] = obb.extend[2];
	return *this;
}

#ifdef REBANG_PCH_REALMYROOMDATA // HACK: realmyroomdata PCH only (original wmath.inl:1075), other units keep their IL
inline Wobb& Wobb::operator=(const WMatrix& mat)
{
	center = mat.pivot;
	extend[0] = mat.xa;
	extend[1] = mat.ya;
	extend[2] = mat.za;
	return *this;
}
#endif

#ifndef REBANG_PCH_POOL // HACK: the pool PCH has its own copy (the bytes pool.obj emits), other units keep their IL
inline WVector Waabb::GetCenter() const
{
	return (min + max) * 0.5f;
}
#endif

inline Wobb& Wobb::operator=(const Waabb& box)
{
	center = box.GetCenter();
	extend[0] = WVector(box.max.x - box.min.x, 0.0f, 0.0f) * 0.5f;
	extend[1] = WVector(0.0f, box.max.y - box.min.y, 0.0f) * 0.5f;
	extend[2] = WVector(0.0f, 0.0f, box.max.z - box.min.z) * 0.5f;
	return *this;
}

inline bool Wobb::IsInclude(const WVector& point) const
{
	WVector direction = point - center;
#ifdef _ATL_STATIC_REGISTRY // ProjectG PCH only
	if (0) // HACK: dead reference: Max<float> instantiated in the PCH before Abs<float>, as in the original (COMDAT order)
		(void)Max(direction.x, direction.x);
#endif
	return bool(Abs(direction * extend[0]) <= (extend[0] * extend[0]) &&
		Abs(direction * extend[1]) <= (extend[1] * extend[1]) &&
		Abs(direction * extend[2]) <= (extend[2] * extend[2]));
}

#ifdef REBANG_PCH_QUADTREE // HACK: only in the quadtree PCH, other units keep their IL (emission order)
inline Waabb __fastcall ExpandAABB(const Waabb& a, const Waabb& b)
{
	return Waabb(WVector(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y),
					 Min(a.min.z, b.min.z)),
		WVector(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y),
			Max(a.max.z, b.max.z)));
}

inline void Wobb::operator+=(const WVector& v)
{
	center += v;
}
#endif

#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
inline void WPlane::Normalize()
{
	float inv = 1.0f / sqrtf(x * x + y * y + z * z);
	x *= inv;
	y *= inv;
	z *= inv;
	dis *= inv;
}
#endif

inline WVector4::WVector4(float x_, float y_, float z_, float w_)
	: x(x_), y(y_), z(z_), w(w_)
{
}

inline void WVector::Negate()
{
	x = -x;
	y = -y;
	z = -z;
}
