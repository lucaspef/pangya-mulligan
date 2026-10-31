// a engine não é o projeto device: o WVector tem operator= próprio (1005c240)
#undef WANGREAL_DEVICE
#include "wmath.h"
#include <vector>

const WVector2D WVector2D::ONE(1.0f, 1.0f);
const WVector2D WVector2D::UNIT_POS_X(1.0f, 0.0f);
const WVector2D WVector2D::UNIT_POS_Y(0.0f, 1.0f);
const WVector2D WVector2D::UNIT_NEG_X(-1.0f, 0.0f);
const WVector2D WVector2D::UNIT_NEG_Y(0.0f, -1.0f);
const WVector WVector::ONE(1.0f, 1.0f, 1.0f);
const WVector WVector::UNIT_POS_X(1.0f, 0.0f, 0.0f);
const WVector WVector::UNIT_POS_Y(0.0f, 1.0f, 0.0f);
const WVector WVector::UNIT_POS_Z(0.0f, 0.0f, 1.0f);
const WVector WVector::UNIT_NEG_X(-1.0f, 0.0f, 0.0f);
const WVector WVector::UNIT_NEG_Y(0.0f, -1.0f, 0.0f);
const WVector WVector::UNIT_NEG_Z(0.0f, 0.0f, -1.0f);
const WMatrix WMatrix::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	1.0f);
__declspec(align(8)) const WMatrix4 WMatrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
__declspec(align(8)) const WVector4 WVector4::ONE(1.0f, 1.0f, 1.0f, 1.0f);

float* costable = 0;
float* sqrttable = 0;
float* acostable = 0;
const WVector WVector::ZERO(0.0f, 0.0f, 0.0f);
const WMatrix WMatrix::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f);
__declspec(align(8)) const WMatrix4 WMatrix4::ZERO(0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
__declspec(align(8)) const WVector4 WVector4::ZERO(0.0f, 0.0f, 0.0f, 0.0f);

// So no exe: este arquivo tambem entra na wangreal.dll (Makefile BASE_SRC/ENGINE_SRC, link /OPT:NOREF) e a dll do 983 nao tem
// as funcoes abaixo; REBANG_SEMANTIC_ONLY so e definida pelo build do exe (tools/f32/pgbuild.py).
#ifdef REBANG_SEMANTIC_ONLY
// 983 0092ef80 (fora de linha no 983; sem corpo aqui ate a F4: stub fatal no WPolySoup::PostModels983, robo c88-7):
// uniao das caixas, eixo a eixo (std::min/std::max)
Waabb& Waabb::operator+=(const Waabb& box)
{
	min.x = box.min.x < min.x ? box.min.x : min.x;
	max.x = max.x < box.max.x ? box.max.x : max.x;
	min.y = box.min.y < min.y ? box.min.y : min.y;
	max.y = max.y < box.max.y ? box.max.y : max.y;
	min.z = box.min.z < min.z ? box.min.z : min.z;
	max.z = max.z < box.max.z ? box.max.z : max.z;
	return *this;
}

// sobrecarga nao-const que o quadtree/npc escolhem com WVector nao-const (so declarada: stub no exe que devolvia 0 =
// "fora da caixa"); mesmo teste da versao const embutida do wmath.h
bool Waabb::IsInclude(WVector& vector) const
{
	if (min.x <= vector.x && vector.x <= max.x && min.y <= vector.y &&
		vector.y <= max.y && min.z <= vector.z && vector.z <= max.z)
		return true;
	return false;
}
#endif

Waabb& Waabb::operator+=(const WVector& vector)
{
	if (vector.x > 0.0f)
		max.x += vector.x;
	else
		min.x += vector.x;
	if (vector.y > 0.0f)
		max.y += vector.y;
	else
		min.y += vector.y;
	if (vector.z > 0.0f)
		max.z += vector.z;
	else
		min.z += vector.z;
	return *this;
}

void WMatrix::Rotate(float angle, char direct, bool pre)
{
	WMatrix m;
	RotMat(angle, direct, m);
	if (pre)
		*this = m * *this;
	else
		*this *= m;
}

void WMatrix::Rotate(const WVector& rotation)
{
	WMatrix m;
	WMatrix t;
	RotMat(rotation.x, 0, m);
	RotMat(rotation.y, 1, t);
	m = m * t;
	RotMat(rotation.z, 2, t);
	m = m * t;
	*this = *this * m;
}

void WMatrix::Normalize()
{
	xa.Normalize();
	ya.Normalize();
	za.Normalize();
}

void WMatrix::GetRotMatrix(WMatrix* result) const
{
	WVector v(1.0f, 0.0f, 0.0f);
	*result = *this;
	result->xm = result->ym = result->zm = 0.0f;
	v = v * *result;
	float size = 1.0f / (float)sqrt(v.y * v.y + v.x * v.x + v.z * v.z);
	*result *= size;
}

void WMatrix::AxisScale(WVector& scale)
{
	Normalize();
	xx = scale.x * xx;
	yx = scale.x * yx;
	zx = zx * scale.x;
	xy = scale.y * xy;
	yy = yy * scale.y;
	zy = zy * scale.y;
	xz = xz * scale.z;
	yz = scale.z * yz;
	zz = zz * scale.z;
}

void WQuat::Reset()
{
	x = y = z = 0.0f;
	w = 1.0f;
}

void WQuat::Normalize()
{
	float size = 1.0f / (float)sqrt(x * x + y * y + z * z + w * w);
	x *= size;
	y *= size;
	z *= size;
	w *= size;
}

void WQuat::SetFromAxisAngle(const WVector& vector, float theta)
{
	double halfTheta = theta * 0.5;
	float sinValue = sinf(halfTheta);
	x = sinValue * vector.x;
	y = sinValue * vector.y;
	z = sinValue * vector.z;
	w = cosf(halfTheta);
}

void WQuat::ConvertToAxisAngle(WVector* vector, float* theta)
{
	*theta = 2.0f * acosf(w);
	if (Wabs(*theta) < g_EPSILON)
	{
		vector->x = 1.0f;
		vector->y = 0.0f;
		vector->z = 0.0f;
		return;
	}
	float sinValue = 1.0f / (float)sin(*theta * 0.5f);
	vector->x = sinValue * x;
	vector->y = sinValue * y;
	vector->z = sinValue * z;
}

void WQuat::ConvertToRotationMatrix(WMatrix& rotation) const
{
	float fTx = 2.0 * x;
	float fTy = 2.0 * y;
	float fTz = 2.0 * z;
	float fTwx = fTx * w;
	float fTwy = fTy * w;
	float fTwz = fTz * w;
	float fTxx = fTx * x;
	float fTxy = fTy * x;
	float fTxz = fTz * x;
	float fTyy = fTy * y;
	float fTyz = fTz * y;
	float fTzz = fTz * z;

	rotation.xx = 1.0f - (fTyy + fTzz);
	rotation.yx = fTxy - fTwz;
	rotation.zx = fTxz + fTwy;
	rotation.xy = fTxy + fTwz;
	rotation.yy = 1.0f - (fTxx + fTzz);
	rotation.zy = fTyz - fTwx;
	rotation.xz = fTxz - fTwy;
	rotation.yz = fTyz + fTwx;
	rotation.zz = 1.0f - (fTxx + fTyy);
	rotation.xm = rotation.ym = rotation.zm = 0.0f;
}

void WQuat::SetFromAngles(float yaw, float pitch, float roll)
{
	float fSinYaw = sinf(yaw * 0.5);
	float fSinPitch = sinf(pitch * 0.5);
	float fSinRoll = sinf(roll * 0.5);
	float fCosYaw = cosf(yaw * 0.5);
	float fCosPitch = cosf(pitch * 0.5);
	float fCosRoll = cosf(roll * 0.5);
	x = fCosPitch * fSinRoll * fCosYaw - fCosRoll * fSinPitch * fSinYaw;
	y = fCosRoll * fSinPitch * fCosYaw + fCosPitch * fSinRoll * fSinYaw;
	z = fCosRoll * fCosPitch * fSinYaw - fSinPitch * fSinRoll * fCosYaw;
	w = fSinPitch * fSinRoll * fSinYaw + fCosRoll * fCosPitch * fCosYaw;
	Normalize();
}

void WQuat::operator=(const WMatrix& matrix)
{
	float t = matrix.xx + matrix.yy + matrix.zz;
	float len;
	float s;
	if (t > 0.0f)
	{
		len = (float)sqrt((float)(t + 1.0f));
		s = 0.5f / len;
		w = len * 0.5f;
		x = (matrix.yz - matrix.zy) * s;
		y = (matrix.zx - matrix.xz) * s;
		z = (matrix.xy - matrix.yx) * s;
		return;
	}

	switch (matrix.xx > matrix.yy ? (matrix.xx > matrix.zz ? 0 : 2)
								  : (matrix.yy > matrix.zz ? 1 : 2))
	{
	case 0:
		len = (float)sqrt(1.0f + matrix.xx - matrix.yy - matrix.zz);
		s = 0.5f / len;
		x = len * 0.5f;
		y = (matrix.xy + matrix.yx) * s;
		z = (matrix.xz + matrix.zx) * s;
		w = (matrix.yz - matrix.zy) * s;
		break;
	case 1:
		len = (float)sqrt(1.0f - matrix.xx + matrix.yy - matrix.zz);
		s = 0.5f / len;
		x = (matrix.xy + matrix.yx) * s;
		y = len * 0.5f;
		z = (matrix.yz + matrix.zy) * s;
		w = (matrix.zx - matrix.xz) * s;
		break;
	case 2:
		len = (float)sqrt(1.0f - matrix.xx - matrix.yy + matrix.zz);
		s = 0.5f / len;
		x = (matrix.xz + matrix.zx) * s;
		y = (matrix.yz + matrix.zy) * s;
		z = len * 0.5f;
		w = (matrix.xy - matrix.yx) * s;
		break;
	}
}

void WMatrix::operator=(const WQuat& quat)
{
	float _xx = 2.0f * quat.x * quat.x;
	float _yy = 2.0f * quat.y * quat.y;
	float _zz = 2.0f * quat.z * quat.z;
	float _xy = 2.0f * quat.x * quat.y;
	float _xz = 2.0f * quat.x * quat.z;
	float _yz = 2.0f * quat.y * quat.z;
	float _wx = 2.0f * quat.w * quat.x;
	float _wy = 2.0f * quat.w * quat.y;
	float _wz = 2.0f * quat.w * quat.z;

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

WVector __fastcall MinPointLineSegment(WVector& vp, WVector& va, WVector& vb,
	WVector* vt)
{
	WVector vm = vb - va;
	WVector vdiff = vp - va;
	float t = vdiff * vm;
	float mSqr;
	if (t <= 0.0f)
	{
		*vt = va;
	}
	else if (t >= (mSqr = vm.SquareMagnitude()))
	{
		vdiff = vdiff - vm;
		*vt = vb;
	}
	else
	{
		t /= mSqr;
		*vt = va + vm * t;
		vdiff = vp - *vt;
	}
	return vdiff;
}

void __cdecl UninitMath()
{
}

// 983: novas (nomes provisórios). Lado do triângulo p0-p1-p2 em relação a uma normal.
bool __fastcall WIsFrontFace(const WVector& p0, const WVector& p1,
	const WVector& p2, const WVector& normal)
{
	if (WCrossProduct(p1 - p0, p2 - p0) * normal > 0.0f)
		return true;
	return false;
}

bool __fastcall WIsFrontFace(const WVector& p0, const WVector& p1,
	const WVector& p2, int direct)
{
	const WVector* normal;
	switch (direct)
	{
	case 0:
		normal = &WVector::UNIT_POS_X;
		break;
	case 1:
		normal = &WVector::UNIT_POS_Y;
		break;
	case 2:
		normal = &WVector::UNIT_POS_Z;
		break;
	case 3:
		normal = &WVector::UNIT_NEG_X;
		break;
	case 4:
		normal = &WVector::UNIT_NEG_Y;
		break;
	case 5:
		normal = &WVector::UNIT_NEG_Z;
		break;
	default:
		return false;
	}
	return WIsFrontFace(p0, p1, p2, *normal);
}

void WMatrix::GetAngles(WVector* angles) const
{
	WMatrix m = *this;
	m.Normalize();
	if (m.zx < 1.0f)
	{
		if (m.zx > -1.0f)
		{
			angles->y = asinf(m.zx);
			angles->x = atan2f(-m.zy, m.zz);
			angles->z = atan2f(m.yx, m.xx);
		}
		else
		{
			angles->y = -g_PI_DIV_2;
			angles->x = -atan2f(m.xy, m.yy);
			angles->z = 0.0f;
		}
	}
	else
	{
		angles->y = g_PI_DIV_2;
		angles->x = atan2f(m.xy, m.yy);
		angles->z = 0.0f;
	}
}

void __cdecl InitMath()
{
}

float __cdecl CalcDeltaAngle(const WVector& v1, const WVector& v2)
{
	WVector va(v1.x, 0.0f, v1.z);
	WVector vb(v2.x, 0.0f, v2.z);
	float la = va.Magnitude();
	float lb = vb.Magnitude();
	if (Wabs(la) < g_EPSILON || Wabs(lb) < g_EPSILON)
		return 0.0f;
	float angle = acosf(Between<float>(-1.0f, WDotProduct(vb, va) / (la * lb), 1.0f));
	return v1.z * v2.x - v2.z * v1.x > 0.0 ? angle : -angle;
}

// 983: novas (nomes provisórios). `direct` 0..2 = plano perpendicular ao eixo; 3..5 = o mesmo, visto do outro lado.
bool __fastcall WIsInTriangle(const WVector& p, const WVector& a,
	const WVector& b, const WVector& c, int direct)
{
	WVector2D tri[3];
	WVector2D pt;
	int axis;
	if (direct < 3)
		axis = direct;
	else
		axis = direct - 3;
	switch (axis)
	{
	case 0:
		tri[0].x = a.z;
		tri[0].y = a.y;
		tri[1].x = b.z;
		tri[1].y = b.y;
		tri[2].x = c.z;
		tri[2].y = c.y;
		pt.x = p.z;
		pt.y = p.y;
		break;
	case 1:
		tri[0].x = a.x;
		tri[0].y = a.z;
		tri[1].x = b.x;
		tri[1].y = b.z;
		tri[2].x = c.x;
		tri[2].y = c.z;
		pt.x = p.x;
		pt.y = p.z;
		break;
	case 2:
		tri[0].x = a.y;
		tri[0].y = a.x;
		tri[1].x = b.y;
		tri[1].y = b.x;
		tri[2].x = c.y;
		tri[2].y = c.x;
		pt.x = p.y;
		pt.y = p.x;
		break;
	default: // igual ao caso 1 (no 983 os dois dividem o mesmo bloco)
		tri[0].x = a.x;
		tri[0].y = a.z;
		tri[1].x = b.x;
		tri[1].y = b.z;
		tri[2].x = c.x;
		tri[2].y = c.z;
		pt.x = p.x;
		pt.y = p.z;
		break;
	}
	WVector2D e0;
	WVector2D e1;
	e0 = (tri[1] - tri[0]).Normalize();
	e1 = (tri[2] - tri[0]).Normalize();
	float side = e0.x * e1.y - e0.y * e1.x;
	for (int i = 0; i < 3; i++)
	{
		e0 = (tri[(i + 1) % 3] - tri[i]).Normalize();
		e1 = (pt - tri[i]).Normalize();
		float cross = e0.x * e1.y - e0.y * e1.x;
		if (cross * side < 0.0)
			return false;
	}
	return true;
}

bool __fastcall WIsClockwise(const std::vector<WVector>& poly, int direct)
{
	bool cw;
	int axis;
	if (direct < 3)
	{
		cw = true;
		axis = direct;
	}
	else
	{
		cw = false;
		axis = direct - 3;
	}
	float cross = 0.0f;
	int count = (int)poly.size();
	int top = 0;
	for (int i = 1; i < count; i++)
	{
		switch (axis)
		{
		case 0:
			if (poly[i].z <= poly[top].z && poly[i].y < poly[top].y)
				top = i;
			break;
		case 1:
			if (poly[i].x <= poly[top].x && poly[i].z < poly[top].z)
				top = i;
			break;
		case 2:
			if (poly[i].y <= poly[top].y && poly[i].x < poly[top].x)
				top = i;
			break;
		}
	}
	const WVector* v[3];
	v[0] = &poly[(top - 1 + count) % count];
	v[1] = &poly[top];
	v[2] = &poly[(top + 1) % count];
	switch (axis)
	{
	case 0:
		cross = (v[1]->z - v[0]->z) * (v[2]->y - v[0]->y) -
			(v[2]->z - v[0]->z) * (v[1]->y - v[0]->y);
		break;
	case 1:
		cross = (v[1]->x - v[0]->x) * (v[2]->z - v[0]->z) -
			(v[2]->x - v[0]->x) * (v[1]->z - v[0]->z);
		break;
	case 2:
		cross = (v[2]->x - v[0]->x) * (v[1]->y - v[0]->y) -
			(v[1]->x - v[0]->x) * (v[2]->y - v[0]->y);
		break;
	}
	if (cw)
	{
		if (cross < 0.0f)
			return true;
		return false;
	}
	if (cross > 0.0f)
		return true;
	return false;
}

float __fastcall WCollisionTest(const Waabb& origin, const WVector& vec,
	const Waabb& target, WPlane* plane)
{
	Waabb area;
	area = origin;
	area += vec;
	if (!(area & target) || vec == WVector::ZERO)
		return 1.0f;

	int direct = 3;
	float t = 1.0f;
	float t2;
	for (int i = 0; i < 3; i++)
	{
		float absoluteComponent;
		if (*(unsigned long*)&vec.p[i] & 0x80000000)
			absoluteComponent = -vec.p[i];
		else
			absoluteComponent = vec.p[i];
		if (absoluteComponent < g_EPSILON)
			continue;
		if (vec.p[i] > 0.0f)
			t2 = 1.0f - (area.max.p[i] - target.min.p[i]) / vec.p[i];
		else
			t2 = 1.0f - (area.min.p[i] - target.max.p[i]) / vec.p[i];
		if (direct == 3 || t < t2)
		{
			t = t2;
			direct = i;
		}
	}

	if (direct >= 3 || t >= 1.0f)
		return 1.0f;
	float absoluteT;
	if (*(unsigned long*)&t & 0x80000000)
		absoluteT = -t;
	else
		absoluteT = t;
	if (absoluteT < g_EPSILON)
		return 0.0f;

	WVector gap;
	gap = vec * t;
	if (WisEqual(WCollisionTest(origin + gap, vec * (1.0f - t), target, 0),
			1.0f, g_EPSILON))
	{
		return 1.0f;
	}
	if (plane)
	{
		plane->normal = WVector::ZERO;
		plane->normal.p[direct] = vec.p[direct] > 0.0f ? -1.0f : 1.0f;
	}
	return t;
}
