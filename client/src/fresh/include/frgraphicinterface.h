#pragma once
#include "../../../include/engine/wtypes.h"
#include "../../../include/engine/woverlay.h"

class Bitmap;
class TexCacheManager;
class WFont;
class WTitleFont;
class WView;
class FrTEXT;

extern WView* g_view;

class FrGraphicInterface
{
public:
	FrGraphicInterface(WView* view);
	virtual ~FrGraphicInterface();
	void Box(const _WRECT& rc, unsigned long color, unsigned long z = 0,
		float alpha = 0.0f) const
	{
		WOverlay::DrawBox(g_view, rc, z, color, alpha);
	}
	void LineBox(const _WRECT& rc, unsigned long color, unsigned long z = 0,
		float alpha = 0.0f) const
	{
		WOverlay::DrawLineBox(g_view, rc, z, color);
	}
	void DrawTexture(const Bitmap* bitmap, const WRect& dst,
		unsigned long color, int flags) const;
	void DrawTexture(const Bitmap* bitmap, const WRect& src, const WRect& dst,
		unsigned long color, int flags) const;
	void InvalidateCache(const Bitmap& bitmap);
	void RefreshTextureCacheInfo(const Bitmap* bitmap);
	void RainbowBox(const _WRECT& rect, unsigned long* const colors,
		unsigned long type = 0, float depth = 0.0f) const
	{
		WOverlay::DrawRainbowBox(g_view, rect, colors, type, depth);
	}
	void SetClippingArea(WRect* rect);
	void SetAlpha(float alpha);
	float GetAlpha();
	int GetFontHeight();
	float PrintText(const WPoint& point, unsigned long style, const char* text,
		Bitmap* bitmap);
	float PrintText11(const WPoint& point, unsigned long style,
		const char* text, Bitmap* bitmap);
	float GetTextExtend11(const char* text);
	void Reset();
	void Line(const WPoint& start, const WPoint& end, unsigned long first,
		unsigned long second, unsigned long type = 0) const
	{
		g_view->DrawLine2D(start, end, first, second, type);
	}
	float GetTextExtendOld983(const char* text); // 983: 00bec480, WFont slot 6 (medida antiga); usada pela lista do IME. Nome meu.
	float GetTextExtendOld11_983(const char* text); // 983: 00bec4a0
	float GetTextExtend(const char* text);
	unsigned long GetTextColor() const { return m_textColor; }
	unsigned long GetTextOutlineColor() const { return m_textOutlineColor; }
	void SetTextColor(unsigned long color, unsigned long outlineColor);
	void SetTextStyle(unsigned long style);
	float Print(const WPoint& point, unsigned long style, const char* format,
		...);
	float GetViewWidth();
	float GetViewHeight();

	void UpdateTextureCacheInfo(const Bitmap* bitmap);
	void RefreshTexCache(const Bitmap& bitmap);
	void SetClippingArea11(WRect* rect);
	int GetFontHeight11();
	int SetSpace(int space);
	int SetSpace11(int space);
	void SetScale(float scale);
	void SetScale11(float scale);
	float Print11(const WPoint& point, unsigned long align, const char* format,
		...);
	void PrintText(const WPoint& point, const FrTEXT& text);
	void PrintText11(const WPoint& point, const FrTEXT& text);
	void PrintText(const WPoint& point, unsigned long align,
		const FrTEXT& text);
	void PrintText11(const WPoint& point, unsigned long align,
		const FrTEXT& text);
	// 983 00bed170: texto com WTitleFont (letras pelo charset da fonte), align 0/1/2 = esquerda/centro/direita; nome meu
	void PrintTitleFont983(WTitleFont* font, float x, float y, const char* text,
		float space, int align, float scale, unsigned long color);

protected:
	WView* m_pView;
	TexCacheManager* m_pCacheManager;
	float m_alpha;
	unsigned long m_textColor;
	unsigned long m_textOutlineColor;
	unsigned long m_textStyle;
	WFont* m_pFont12;
	WFont* m_pFont11;
	int m_maxTexCache;
	int m_cacheTexSize;
};

inline float FrGraphicInterface::GetViewWidth()
{
	return m_pView ? m_pView->GetWidth() : 0.0f;
}

inline float FrGraphicInterface::GetViewHeight()
{
	return m_pView ? m_pView->GetHeight() : 0.0f;
}

inline unsigned long FrALPHA(unsigned long color, float alpha)
{
	return ((int)((color >> 24) * alpha) << 24) | (color & 0x00ffffff);
}
