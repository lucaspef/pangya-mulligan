#pragma once
#include <string>
#include "refresh/lib/rftypes.h"

// JP 983: RFRenderer.h / RFTexture / RFMatrix / RFInterpolation da Refresh como a biblioteca os declara (F3.7.13 item (3), B151).
// Classes exportadas: cópias fora de linha no RFWnd.cpp (00403420–00403830). Assinaturas pela exportação do 983. A visão do jogo
// (refresh/rfrenderer.h, usada pela RefreshRenderer) tem os mesmos slots; unificar no fim do item (5).

struct TileModeTypes { enum Type { Stretch = 0, Tile = 1 }; }; // valores pelo SingleImageSet::_tileMode

// textura (só a vtable 00d13d34; destrutor de apagar 004034b0 = push 4)
class RF_API RFTexture
{
public:
	RFTexture() { } // 00403450
	virtual ~RFTexture() { } // 00403420
	virtual unsigned int GetWidth() const { return 0; } // 00403430
	virtual unsigned int GetHeight() const { return 0; } // 00403440
};

// matriz 4×4 (0x40, sem vtable; operator= com rep movsd 0x10)
class RF_API RFMatrix
{
public:
	RFMatrix() { } // 00403750
	float& operator()(unsigned int row, unsigned int col) { return m[row][col]; } // 00403760
	float operator()(unsigned int row, unsigned int col) const { return m[row][col]; } // 00403780
	operator float*() { return &m[0][0]; } // 004037a0
	operator const float*() const { return &m[0][0]; } // 004037b0
	RFMatrix& operator*=(const RFMatrix& o); // 004620b0
	RFMatrix operator*(const RFMatrix& o) const; // 00462100
	RFMatrix& Identity(); // 00462190
	RFMatrix& Translation(float x, float y, float z); // 004621d0
	RFMatrix& RotationZ(float angle); // 00462220
	RFMatrix& Scaling(float x, float y, float z); // 004622a0
	RFMatrix& Transformation2D(const RFVector2* center, const RFVector2* scale, float rotation); // 004622f0
	RFVector2 Transform2DPoint(const RFVector2* pt) const; // 00462510

	float m[4][4];
};

// desenhista (vtable 00d1304c; o jogo deriva a RefreshRenderer). Os três Draw são sobrecargas: o MSVC as põe na vtable em ordem
// inversa à da declaração (slot 2 = Draw(matriz), 3 = Draw(10 argumentos), 4 = Draw(6 argumentos)).
class RF_API RFRenderer
{
public:
	RFRenderer() { } // 004035a0
	virtual RFTexture* CreateTexture(const std::wstring& name) { return 0; } // 00403510
	virtual void DeleteTexture(RFTexture* texture) { } // 00403520
	virtual void Draw(RFTexture* texture, const RFVector2& pos, const RFSize& size, float alpha, float depth, const TileModeTypes::Type& tile) { } // 00403530
	virtual void Draw(RFTexture* texture, float u0, float v0, float u1, float v1, const RFVector2& pos, const RFSize& size, float alpha, float depth,
		const TileModeTypes::Type& tile) { } // 00403540
	virtual void Draw(RFTexture* texture, float alpha, const RFMatrix& matrix) { } // 00403550
	virtual void PushClip983(const RFRect& rect) = 0; // slot 5 (nome meu)
	virtual void PopClip983() = 0; // slot 6 (nome meu)
	virtual void FillRectangle(unsigned int color, float x, float y, float w, float h, float depth) { } // 00403560
	virtual void OnPreResetDevice() { } // 00403570
	virtual void OnPostResetDevice() { } // 00403580
	virtual ~RFRenderer() { } // 00403590
};

// interpolação (vtable 00d630d8; GetValue linear = devolve t; a RFBezierSpline deriva)
class RF_API RFInterpolation
{
public:
	RFInterpolation() { } // 00403800
	virtual ~RFInterpolation() { } // 004037e0
	virtual float GetValue(float t) const { return t; } // 004037f0
};

static_assert(sizeof(RFMatrix) == 0x40, "RFMatrix");
