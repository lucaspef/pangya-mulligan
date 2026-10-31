#pragma once
#include <string>
#include <memory>
#include "engine/singleton.h"
#include "engine/wmath.h"
#include "frgraphicinterface.h"
#include "rfinterface.h"

class WFont;

// JP 983: texto da RF desenhado pelo FrGraphicInterface do jogo. Uma unidade só, rftextmanager.cpp 00602490–006030c0 (o mapa da
// F3.7.1 cortava em 00602d80 "rftextdraw"/"rftextmanager", mas os DrawText983 do gerente, 00602c50/00602c90, e os moldes do
// WSingleton<RFTextManager> e do shared_ptr<RFTextDraw> estão antes do corte).
// RFTextDraw: 0x2c, vtable 00d13d08 (só o destrutor), sem campos próprios.
class RFTextDraw : public FrGraphicInterface
{
public:
	RFTextDraw(WView* view); // 00602490
	virtual ~RFTextDraw(); // 006024f0 (??_G 006025f0)

	float ColorAlpha983(unsigned long color); // 00602500: (color >> 24) / 255
	// texto com sombra (flags bit 5), contorno de 8 pontos (bit 4) e negrito (bit 0); devolve o avanço
	float PrintText983(WFont* font, const RFVector2& pos, const std::wstring& text, unsigned long flags, unsigned long color,
		unsigned long outlineColor, unsigned long shadowColor); // 00602690
	RFSize GetTextExtent983(const RFTextStyle983& style, const std::wstring& text); // 00602a40
	void DrawText983(float x, float y, const std::wstring& text, unsigned long flags, unsigned long color,
		unsigned long outlineColor, unsigned long shadowColor); // 00602bd0 (fonte 12)
	void DrawText11_983(float x, float y, const std::wstring& text, unsigned long flags, unsigned long color,
		unsigned long outlineColor, unsigned long shadowColor); // 00602c10 (fonte 11)
};

// singleton WSingleton<RFTextManager> (instância 00e78c9c, vtable 00d13d2c); criado no CProjectG::Ready (00a69a80)
class RFTextManager : public WSingleton<RFTextManager>
{
public:
	RFTextManager(); // 00602fd0
	virtual ~RFTextManager(); // 00602d80 (??_G 00602f00)

	void DrawText983(float x, float y, const std::wstring& text, unsigned long c0, unsigned long c1, unsigned long c2, unsigned long c3); // 00602c50 (4 dwords do estilo: o 983 empilha um a um, sem FPU)
	void DrawText11_983(float x, float y, const std::wstring& text, unsigned long c0, unsigned long c1, unsigned long c2, unsigned long c3); // 00602c90 (fonte pequena)

	// 005f5020 (cópia fora de linha na refreshinterface)
	RFSize GetTextExtent983(const RFTextStyle983& style, const std::wstring& text)
	{
		if (m_draw)
			return m_draw->GetTextExtent983(style, text);
		return RFSize(0.0f, 0.0f);
	}
	// 005f58a0 (cópia fora de linha na refreshrenderer)
	void SetClippingArea983(WRect* rect)
	{
		if (m_draw)
		{
			m_draw->SetClippingArea(rect);
			m_draw->SetClippingArea11(rect);
		}
	}

	std::tr1::shared_ptr<RFTextDraw> m_draw; // +0x28 (ponteiro), +0x2c (contador)
};
