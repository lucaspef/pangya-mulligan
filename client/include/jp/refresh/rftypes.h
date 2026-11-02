#pragma once

// JP 983: tipos básicos da biblioteca Refresh (RF) usados pelas unidades de ligação do ProjectG. Nomes da RF (RTTI/839).

// tamanho de texto/janela (2 floats; a RF devolve por valor)
struct RFSize
{
	RFSize() : width(0.0f), height(0.0f) { }
	RFSize(float w, float h) : width(w), height(h) { }

	float width; // +0
	float height; // +4
};

// ponto (2 floats)
struct RFVector2
{
	RFVector2(); // 004289b0 (zera; fora de linha no rftypes.cpp; B157/C92.3: o corpo inline vazio duplicava o símbolo no link)
	RFVector2(float x_, float y_) : x(x_), y(y_) { }

	float x; // +0
	float y; // +4
};

// retângulo da RF: esquerda, direita, cima, baixo (o RefreshRenderer::PushClip983 tira largura = [1] − [0], altura = [3] − [2])
struct RFRect
{
	RFRect() : left(0.0f), right(0.0f), top(0.0f), bottom(0.0f) { } // B1637: mesmo corpo da lib, o linker fica com um so
	RFRect(float l, float t, float r, float b) : left(l), right(r), top(t), bottom(b) { } // B1637: ordem da lib (004013e0)
	RFRect(const RFRect& o) : left(o.left), right(o.right), top(o.top), bottom(o.bottom) { } // cópia com fld/fstp
	bool PtInRect(float x, float y); // 0041dd10
	float GetWidth() const { return right - left; } // (embutida)
	float GetHeight() const { return bottom - top; } // (embutida)

	float left; // +0x0
	float right; // +0x4
	float top; // +0x8
	float bottom; // +0xc
};
