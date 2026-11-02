// JP 983: RFTypes.cpp e RFVector2.cpp da Refresh (B153, F3.7.13): operadores e retÃ¢ngulo fora de linha (0041dc50â€“0041de80,
// 004289b0â€“00428b90). Os construtores/acessores embutidos estÃ£o no lib/rftypes.h.
#include <stdio.h>
#include "refresh/lib/rftypes.h"

// 0041dc50
bool RFSize::operator==(const RFSize& o) const
{
	if (width == o.width && height == o.height)
		return true;
	return false;
}

// 0041dc80
bool RFSize::operator!=(const RFSize& o) const
{
	if (width == o.width && height == o.height)
		return false;
	return true;
}

// 0041dcb0
RFSize RFSize::operator-(const RFSize& o) const
{
	return RFSize(width - o.width, height - o.height);
}

// 0041dd10: borda esquerda/de cima fora, direita/de baixo dentro
bool RFRect::PtInRect(float x, float y) const
{
	if (x >= left && x <= right && y >= top && y <= bottom) // 983 0041dd1c: bordas esquerda/superior contam
		return true;
	return false;
}

// 0041dd60
bool RFRect::PtInRect(const RFVector2& pt) const
{
	if (pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom)
		return true;
	return false;
}

// 0041ddc0: interseção; true se sobrar área
bool RFRect::IntersectRect(const RFRect& a, const RFRect& b)
{
	left = (a.left > b.left) ? a.left : b.left;
	top = (a.top > b.top) ? a.top : b.top;
	right = (a.right < b.right) ? a.right : b.right;
	bottom = (a.bottom < b.bottom) ? a.bottom : b.bottom;
	if (left < right && top < bottom)
		return true;
	left = right = top = bottom = 0.0f;
	return false;
}

// 0041de80
RFRect RFRect::MoveRect(float dx, float dy) const
{
	return RFRect(left + dx, top + dy, right + dx, bottom + dy);
}

// 004289b0
RFVector2::RFVector2()
{
	x = 0.0f;
	y = 0.0f;
}

// 004289c0
RFVector2& RFVector2::operator+=(const RFVector2& o)
{
	x = o.x + x;
	y = o.y + y;
	return *this;
}

// 004289e0
RFVector2& RFVector2::operator-=(const RFVector2& o)
{
	x = x - o.x;
	y = y - o.y;
	return *this;
}

// 00428a90
const RFVector2 RFVector2::operator+(const RFVector2& o) const
{
	return RFVector2(o.x + x, o.y + y);
}

// 00428ab0
const RFVector2 RFVector2::operator-(const RFVector2& o) const
{
	return RFVector2(x - o.x, y - o.y);
}

// 00428b60
bool RFVector2::operator==(const RFVector2& o) const
{
	if (x == o.x && y == o.y)
		return true;
	return false;
}

// 00428b90
bool RFVector2::operator!=(const RFVector2& o) const
{
	if (x == o.x && y == o.y)
		return false;
	return true;
}

// 0041dc10
RFPoint RFPoint::operator+(const RFPoint& o) const
{
	return RFPoint(x + o.x, y + o.y);
}

// 0041dc30
RFPoint& RFPoint::operator+=(const RFPoint& o)
{
	x += o.x;
	y += o.y;
	return *this;
}

// 0041dcd0 / 00428bc0: "%f, %f" (o 983 lÃª o texto da propriedade do XML)
RFSize& RFSize::FromString(const std::string& s)
{
	float a, b;
	sscanf(s.c_str(), "%f, %f", &a, &b);
	width = a;
	height = b;
	return *this;
}

// 00428a00
RFVector2& RFVector2::operator*=(const RFVector2& o)
{
	x = o.x * x;
	y = o.y * y;
	return *this;
}

// 00428a20
RFVector2& RFVector2::operator/=(const RFVector2& o)
{
	x = x / o.x;
	y = y / o.y;
	return *this;
}

// 00428a40
RFVector2& RFVector2::operator*=(float s)
{
	x = x * s;
	y = s * y;
	return *this;
}

// 00428a60: multiplica pelo inverso
RFVector2& RFVector2::operator/=(float s)
{
	s = 1.0f / s;
	x = x * s;
	y = s * y;
	return *this;
}

// 00428ad0
RFVector2 RFVector2::operator*(const RFVector2& o) const
{
	return RFVector2(o.x * x, o.y * y);
}

// 00428af0
RFVector2 RFVector2::operator/(const RFVector2& o) const
{
	return RFVector2(x / o.x, y / o.y);
}

// 00428b10
RFVector2 RFVector2::operator*(float s) const
{
	return RFVector2(s * x, s * y);
}

// 00428b30
RFVector2 RFVector2::operator/(float s) const
{
	s = 1.0f / s;
	return RFVector2(s * x, s * y);
}

// 00428bc0
RFVector2& RFVector2::FromString(const std::string& s)
{
	float a, b;
	sscanf(s.c_str(), "%f, %f", &a, &b);
	x = a;
	y = b;
	return *this;
}

// 0041dec0: margens: esquerda/cima entram, direita/baixo saem
void RFRect::ApplyPadding(const RFRect& pad)
{
	left += pad.left;
	top += pad.top;
	right -= pad.right;
	bottom -= pad.bottom;
}

// 0041def0
RFRect RFRect::InflateRect(float dx, float dy) const
{
	return RFRect(left - dx, top - dy, right + dx, bottom + dy);
}

// 0041df30
RFRect RFRect::DeflateRect(float dx, float dy) const
{
	return RFRect(left + dx, top + dy, right - dx, bottom - dy);
}

// 0041df70
RFRect& RFRect::operator+=(const RFRect& o)
{
	left += o.left;
	right += o.right;
	top += o.top;
	bottom += o.bottom;
	return *this;
}

// 0041dfa0
RFRect& RFRect::operator-=(const RFRect& o)
{
	left -= o.left;
	right -= o.right;
	top -= o.top;
	bottom -= o.bottom;
	return *this;
}

// 0041dfd0
RFRect& RFRect::operator*=(float s)
{
	left = s * left;
	right *= s;
	top *= s;
	bottom *= s;
	return *this;
}

// 0041e000
RFRect RFRect::operator+(const RFRect& o) const
{
	RFRect r;
	r.left = o.left + left;
	r.right = o.right + right;
	r.top = o.top + top;
	r.bottom = o.bottom + bottom;
	return r;
}

// 0041e030
RFRect RFRect::operator-(const RFRect& o) const
{
	RFRect r;
	r.left = left - o.left;
	r.right = right - o.right;
	r.top = top - o.top;
	r.bottom = bottom - o.bottom;
	return r;
}

// 0041e060
RFRect RFRect::operator*(const RFRect& o) const
{
	RFRect r;
	r.left = o.left * left;
	r.right = o.right * right;
	r.top = o.top * top;
	r.bottom = o.bottom * bottom;
	return r;
}

// 0041e090
RFRect RFRect::operator/(const RFRect& o) const
{
	RFRect r;
	r.left = left / o.left;
	r.right = right / o.right;
	r.top = top / o.top;
	r.bottom = bottom / o.bottom;
	return r;
}
