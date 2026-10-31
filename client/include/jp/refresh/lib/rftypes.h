#pragma once
#include <string>
#include "refresh/lib/rfapi.h"

// JP 983: RFTypes.h da Refresh (00401280–00401540; nomes e assinaturas pela exportação do 983)

class RF_API RFVector2
{
public:
	RFVector2(); // 004289b0 (zera; fora de linha no RFVector2.cpp)
	RFVector2(float x_, float y_) { x = x_; y = y_; } // 00401280
	void set(float x_, float y_) { x = x_; y = y_; } // 004012a0
	RFVector2& operator+=(const RFVector2& o); // 004289c0
	RFVector2& operator-=(const RFVector2& o); // 004289e0
	const RFVector2 operator+(const RFVector2& o) const; // 00428a90
	const RFVector2 operator-(const RFVector2& o) const; // 00428ab0
	RFVector2& operator*=(const RFVector2& o); // 00428a00
	RFVector2& operator/=(const RFVector2& o); // 00428a20
	RFVector2& operator*=(float s); // 00428a40
	RFVector2& operator/=(float s); // 00428a60
	RFVector2 operator*(const RFVector2& o) const; // 00428ad0
	RFVector2 operator/(const RFVector2& o) const; // 00428af0
	RFVector2 operator*(float s) const; // 00428b10
	RFVector2 operator/(float s) const; // 00428b30
	RFVector2& FromString(const std::string& s); // 00428bc0 ("%f, %f")
	bool operator==(const RFVector2& o) const; // 00428b60
	bool operator!=(const RFVector2& o) const; // 00428b90

	static const RFVector2 ZERO; // 00f1de44

	float x;
	float y;
};

class RF_API RFPoint
{
public:
	RFPoint() : x(0), y(0) { } // 004012e0
	RFPoint(int x_, int y_) : x(x_), y(y_) { } // 004012f0
	RFPoint(const RFPoint& o) : x(o.x), y(o.y) { } // 00401310
	void set(int x_, int y_) { x = x_; y = y_; } // 00401330
	RFPoint operator+(const RFPoint& o) const; // 0041dc10
	RFPoint& operator+=(const RFPoint& o); // 0041dc30
	int Length() const { return y - x; } // 00401350

	int x;
	int y;
};

class RF_API RFSize
{
public:
	RFSize() : width(0.0f), height(0.0f) { } // 00401380
	RFSize(float w, float h) { width = w; height = h; } // 00401390
	bool operator==(const RFSize& o) const; // 0041dc50
	bool operator!=(const RFSize& o) const; // 0041dc80
	RFSize operator-(const RFSize& o) const; // 0041dcb0
	RFSize& FromString(const std::string& s); // 0041dcd0 ("%f, %f")

	float width;
	float height;
};

// retângulo: esquerda, direita, cima, baixo (o construtor recebe esquerda, cima, direita, baixo)
class RF_API RFRect
{
public:
	RFRect() : left(0.0f), right(0.0f), top(0.0f), bottom(0.0f) { } // 004013d0
	RFRect(float l, float t, float r, float b) { left = l; right = r; top = t; bottom = b; } // 004013e0
	RFRect(const RFRect& o) { left = o.left; right = o.right; top = o.top; bottom = o.bottom; } // 00401400
	RFVector2 LT() const { return RFVector2(left, top); } // 00401430
	RFVector2 RB() const { return RFVector2(right, bottom); } // 00401450
	RFVector2 XY() const { return RFVector2(left, top); } // 00401470
	RFVector2 WH() const { return RFVector2(right - left, bottom - top); } // 00401490
	void set(float l, float t, float r, float b) { left = l; right = r; top = t; bottom = b; } // 004014b0
	float Width() const { return right - left; } // 004014d0
	float Height() const { return bottom - top; } // 004014f0
	bool PtInRect(float x, float y) const; // 0041dd10
	bool PtInRect(const RFVector2& pt) const; // 0041dd60
	bool IntersectRect(const RFRect& a, const RFRect& b); // 0041ddc0
	RFRect MoveRect(float dx, float dy) const; // 0041de80
	void ApplyPadding(const RFRect& pad); // 0041dec0
	RFRect InflateRect(float dx, float dy) const; // 0041def0
	RFRect DeflateRect(float dx, float dy) const; // 0041df30
	RFRect& operator+=(const RFRect& o); // 0041df70
	RFRect& operator-=(const RFRect& o); // 0041dfa0
	RFRect& operator*=(float s); // 0041dfd0
	RFRect operator+(const RFRect& o) const; // 0041e000
	RFRect operator-(const RFRect& o) const; // 0041e030
	RFRect operator*(const RFRect& o) const; // 0041e060
	RFRect operator/(const RFRect& o) const; // 0041e090

	static const RFRect ZERO; // 00f1de34

	float left;
	float right;
	float top;
	float bottom;
};
