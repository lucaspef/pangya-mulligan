// JP 983: unidade 00602490–006030c0 — RFTextDraw (o FrGraphicInterface da RF: sombra, contorno e negrito com a alfa da cor) e o
// RFTextManager (WSingleton que guarda o RFTextDraw num shared_ptr). O mapa da F3.7.1 cortava em 00602d80 em duas
// (rftextdraw.cpp / rftextmanager.cpp), mas os DrawText983 do gerente (00602c50/00602c90) e os moldes do
// WSingleton<RFTextManager> e do shared_ptr<RFTextDraw> estão dos dois lados do corte: é uma unidade só.
#include "refresh/rftextmanager.h"
#include "engine/wfont.h"
#include "engine/wview.h"

extern WView* g_pMainView983; // 00f02458

unsigned long __fastcall FrALPHA983(unsigned long color, float alpha); // 00510250 (o FrALPHA fora de linha)

// contorno de 8 pontos (00e13328)
static int s_outline983[8][2] = {
	{ 0, -1 },
	{ -1, 0 },
	{ 1, 0 },
	{ 0, 1 },
	{ -1, -1 },
	{ 1, -1 },
	{ -1, 1 },
	{ 1, 1 }
};

// 00602490
RFTextDraw::RFTextDraw(WView* view) : FrGraphicInterface(view)
{
	Reset();
}

// 006024f0
RFTextDraw::~RFTextDraw()
{
}

// 00602500
float RFTextDraw::ColorAlpha983(unsigned long color)
{
	return (float)((int)(color >> 24) / 255.0);
}

// 00602690
float RFTextDraw::PrintText983(WFont* font, const RFVector2& pos, const std::wstring& text, unsigned long flags,
	unsigned long color, unsigned long outlineColor, unsigned long shadowColor)
{
	if (!font)
		return 0.0f;
	RFVector2 p = pos;
	float x = p.x;
	float y = p.y;
	bool bBold = (flags & 1) != 0;
	bool bOutline = ((flags >> 4) & 1) != 0;
	m_alpha = ColorAlpha983(color);
	if ((flags >> 5) & 1)
	{
		float shadowAlpha = (float)((int)(shadowColor >> 24) / 255.0);
		float offset = 1.0f;
		if (bOutline)
			offset = 1.5f;
		if (bBold)
			offset = (float)(offset + 0.5);
		if (m_alpha > 0.0f && shadowAlpha > 0.0f)
		{
			if (shadowAlpha < m_alpha)
				font->PrintW983(m_pView, offset + x, offset + y, text.c_str(), 0x80000, FrALPHA983(shadowColor, shadowAlpha / m_alpha), 0);
			else
				font->PrintW983(m_pView, offset + x, offset + y, text.c_str(), 0x80000, FrALPHA983(shadowColor, m_alpha / shadowAlpha), 0);
		}
	}
	if (bOutline)
	{
		float outlineAlpha = (float)((int)(outlineColor >> 24) / 255.0);
		if (m_alpha > 0.0f && outlineAlpha > 0.0f)
		{
			for (int i = 0; i < 8; ++i)
			{
				int bold = 0;
				if (bBold && s_outline983[i][0] == 1)
					bold = 1;
				if (m_alpha > outlineAlpha)
					font->PrintW983(m_pView, s_outline983[i][0] + x + bold, s_outline983[i][1] + y, text.c_str(), 0x80000,
						FrALPHA983(outlineColor, outlineAlpha / m_alpha), 0);
				else
					font->PrintW983(m_pView, s_outline983[i][0] + x + bold, s_outline983[i][1] + y, text.c_str(), 0x80000,
						FrALPHA983(outlineColor, m_alpha / outlineAlpha), 0);
			}
		}
	}
	if (bBold)
		font->PrintW983(m_pView, (float)(x + 1.0), y, text.c_str(), 0x80000, FrALPHA(color, m_alpha), 0);
	float advance = font->PrintW983(m_pView, x, y, text.c_str(), 0x80000, FrALPHA(color, m_alpha), 0);
	font->Flush(m_pView);
	return advance;
}

// 00602a40
RFSize RFTextDraw::GetTextExtent983(const RFTextStyle983& style, const std::wstring& text)
{
	float w;
	if (style.size < 12)
		w = GetTextExtend11((const char*)text.c_str());
	else
		w = GetTextExtend((const char*)text.c_str());
	return RFSize(w, (float)GetFontHeight());
}

// 00602bd0
void RFTextDraw::DrawText983(float x, float y, const std::wstring& text, unsigned long flags, unsigned long color,
	unsigned long outlineColor, unsigned long shadowColor)
{
	RFVector2 pos;
	pos.x = x;
	pos.y = y;
	PrintText983(m_pFont12, pos, text, flags, color, outlineColor, shadowColor);
}

// 00602c10
void RFTextDraw::DrawText11_983(float x, float y, const std::wstring& text, unsigned long flags, unsigned long color,
	unsigned long outlineColor, unsigned long shadowColor)
{
	RFVector2 pos;
	pos.x = x;
	pos.y = y;
	PrintText983(m_pFont11, pos, text, flags, color, outlineColor, shadowColor);
}

// 00602c50
void RFTextManager::DrawText983(float x, float y, const std::wstring& text, unsigned long c0, unsigned long c1, unsigned long c2, unsigned long c3)
{
	if (m_draw)
		m_draw->DrawText983(x, y, text, c0, c1, c2, c3);
}

// 00602c90
void RFTextManager::DrawText11_983(float x, float y, const std::wstring& text, unsigned long c0, unsigned long c1, unsigned long c2, unsigned long c3)
{
	if (m_draw)
		m_draw->DrawText11_983(x, y, text, c0, c1, c2, c3);
}

// 00602fd0
RFTextManager::RFTextManager()
{
	m_draw = std::tr1::shared_ptr<RFTextDraw>(new RFTextDraw(g_pMainView983));
}

// 00602d80
RFTextManager::~RFTextManager()
{
}
