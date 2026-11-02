#include "frgraphicinterface.h"
#include "frtext.h"
#include "../texturecache/tcachemanager.h"
#include "wfont.h"
#include "wresrcmng.h"
#include <vector>
#include <stdarg.h>
#include <stdio.h>

extern const float TEXT_TOOLTIP_LINE_INTERVAL;
typedef std::list<FrTextToken> FrTextTokenList;

FrGraphicInterface::FrGraphicInterface(WView* pView)
{
	m_pView = pView;
	m_pCacheManager = 0;
	m_pFont12 = 0;
	m_maxTexCache = 32;
	m_cacheTexSize = 256;
	m_pCacheManager = new TexCacheManager;
	if (m_pCacheManager != 0)
		m_pCacheManager->Init(m_maxTexCache, m_cacheTexSize, m_cacheTexSize, 4,
			4);
	m_pFont12 = g_resrcmng->GetFixedFont("fnt_japan_12.wft" /* 983 (00bec600); no 645: fnt_/xb5/xb8/xbf/xf2/xc3/xbc_12.wft */);
	if (m_pFont12 != 0)
		m_pFont12->SetMode(WFont::MASKED);
	m_pFont11 = g_resrcmng->GetFixedFont("ms_ui_gothic_11_jp.wft" /* 983 (00bec600); no 645: fnt_/xb5/xb8/xbf/xf2/xc3/xbc_11.wft */);
	if (m_pFont11 != 0)
		m_pFont11->SetMode(WFont::MASKED);
	m_textStyle = 0;
	m_textColor = 0xff000000;
	m_textOutlineColor = 0xffffffff;
	m_alpha = 1.0f;
}

FrGraphicInterface::~FrGraphicInterface(void)
{
	if (m_pCacheManager != 0)
	{
		delete m_pCacheManager;
		m_pCacheManager = 0;
	}
	if (g_resrcmng != 0)
	{
		if (m_pFont12 != 0)
		{
			g_resrcmng->Release(m_pFont12);
			m_pFont12 = 0;
		}
		if (g_resrcmng != 0 && m_pFont11 != 0)
		{
			g_resrcmng->Release(m_pFont11);
			m_pFont11 = 0;
		}
	}
}

void FrGraphicInterface::DrawTexture(const Bitmap* pBitmap,
	const WRect& destination, unsigned long color, int flags) const
{
	if (pBitmap == 0)
		return;
	TexCacheManager* pCache = m_pCacheManager;
	const sTexCacheInfo* pInfo = pCache->Draw(*pBitmap);
	if (pInfo == 0)
		return;
	WRect source((float)pInfo->rcPixel.left / m_cacheTexSize,
		(float)pInfo->rcPixel.top / m_cacheTexSize,
		(float)pInfo->rcPixel.Width() / m_cacheTexSize,
		(float)pInfo->rcPixel.Height() / m_cacheTexSize);
	m_pView->Draw2DTexture(source, destination, pInfo->texHandle,
		FrALPHA(color, m_alpha), flags);
}

void FrGraphicInterface::DrawTexture(const Bitmap* pBitmap, const WRect& source,
	const WRect& destination, unsigned long color, int flags) const
{
	if (pBitmap == 0)
		return;
	TexCacheManager* pCache = m_pCacheManager;
	const sTexCacheInfo* pInfo = pCache->Draw(*pBitmap);
	if (pInfo == 0)
		return;
	WRect area(((float)pInfo->rcPixel.left + source.x) / m_cacheTexSize,
		((float)pInfo->rcPixel.top + source.y) / m_cacheTexSize,
		source.w / m_cacheTexSize, source.h / m_cacheTexSize);
	m_pView->Draw2DTexture(area, destination, pInfo->texHandle,
		FrALPHA(color, m_alpha), flags);
}

void FrGraphicInterface::UpdateTextureCacheInfo(const Bitmap* pBitmap)
{
	TexCacheManager* pCache = m_pCacheManager;
	const sTexCacheInfo* pInfo = pCache->Draw(*pBitmap);
	if (pInfo == 0)
		return;
	m_pCacheManager->UpdateTextureCacheInfo(*pInfo);
}

void FrGraphicInterface::RefreshTextureCacheInfo(const Bitmap* pBitmap)
{
	m_pCacheManager->RefreshTexCache(*pBitmap);
}

float FrGraphicInterface::Print(const WPoint& point, unsigned long align,
	const char* format, ...)
{
	char buffer[2048];
	va_list args;
	va_start(args, format);
	if (_vsnprintf(buffer, 0x7ff, format, args) == -1)
		buffer[0x7ff] = 0;
	va_end(args);
	return PrintText(point, align, buffer, 0);
}

float FrGraphicInterface::Print11(const WPoint& point, unsigned long align,
	const char* format, ...)
{
	char buffer[2048];
	va_list args;
	va_start(args, format);
	if (_vsnprintf(buffer, 0x7ff, format, args) == -1)
		buffer[0x7ff] = 0;
	va_end(args);
	return PrintText11(point, align, buffer, 0);
}

float FrGraphicInterface::PrintText(const WPoint& point, unsigned long align,
	const char* text, Bitmap* pBitmap)
{
	WPoint p;
	float w;
	switch (align)
	{
	case 0:
		p = point;
		break;
	case 1:
		w = m_pFont12->GetTextWidthOld983(m_pView, text);
		p.x = point.x - w * 0.5f;
		p.y = point.y;
		break;
	case 2:
		w = m_pFont12->GetTextWidthOld983(m_pView, text);
		p.x = point.x - w;
		p.y = point.y;
		break;
	}
	switch (m_textStyle)
	{
	case 1:
		m_pFont12->Print(m_pView, p.x + 1.0f, p.y, text, 0x80000,
			FrALPHA(m_textColor, m_alpha), pBitmap);
		break;
	case 2:
	{
		static int opix[8][2] = {
			{ 0,  -1 },
            { -1, 0  },
            { 1,  0  },
            { 0,  1  },
            { -1, -1 },
            { 1,  -1 },
			{ -1, 1  },
            { 1,  1  }
		};
		for (int i = 0; i < 8; ++i)
			m_pFont12->Print(m_pView, p.x + opix[i][0], p.y + opix[i][1], text,
				0x80000, FrALPHA(m_textOutlineColor, m_alpha), pBitmap);
	}
	break;
	case 4:
		m_pFont12->Print(m_pView, p.x + 1.0f, p.y + 1.0f, text, 0x80000,
			FrALPHA(m_textOutlineColor, m_alpha), pBitmap);
		break;
	}
	float advance = m_pFont12->Print(m_pView, p.x, p.y, text, 0x80000,
		FrALPHA(m_textColor, m_alpha), pBitmap);
	if (pBitmap == 0)
		m_pFont12->Flush(m_pView);
	return advance;
}

float FrGraphicInterface::PrintText11(const WPoint& point, unsigned long align,
	const char* text, Bitmap* pBitmap)
{
	WPoint p;
	float w;
	switch (align)
	{
	case 0:
		p = point;
		break;
	case 1:
		w = m_pFont11->GetTextWidthOld983(m_pView, text);
		p.x = point.x - w * 0.5f;
		p.y = point.y;
		break;
	case 2:
		w = m_pFont11->GetTextWidthOld983(m_pView, text);
		p.x = point.x - w;
		p.y = point.y;
		break;
	}
	switch (m_textStyle)
	{
	case 1:
		m_pFont11->Print(m_pView, p.x + 1.0f, p.y, text, 0x80000,
			FrALPHA(m_textColor, m_alpha), pBitmap);
		break;
	case 2:
	{
		static int opix[8][2] = {
			{ 0,  -1 },
            { -1, 0  },
            { 1,  0  },
            { 0,  1  },
            { -1, -1 },
            { 1,  -1 },
			{ -1, 1  },
            { 1,  1  }
		};
		for (int i = 0; i < 8; ++i)
			m_pFont11->Print(m_pView, p.x + opix[i][0], p.y + opix[i][1], text,
				0x80000, FrALPHA(m_textOutlineColor, m_alpha), pBitmap);
	}
	break;
	case 4:
		m_pFont11->Print(m_pView, p.x + 1.0f, p.y + 1.0f, text, 0x80000,
			FrALPHA(m_textOutlineColor, m_alpha), pBitmap);
		break;
	}
	float advance = m_pFont11->Print(m_pView, p.x, p.y, text, 0x80000,
		FrALPHA(m_textColor, m_alpha), pBitmap);
	if (pBitmap == 0)
		m_pFont11->Flush(m_pView);
	return advance;
}

void FrGraphicInterface::PrintText(const WPoint& point, const FrTEXT& text)
{
	FrTextTokenList tokens;
	text.GetTokenList(tokens);
	WPoint p;
	p = point;
	for (FrTextTokenList::iterator it = tokens.begin(); it != tokens.end();
		++it)
	{
		if ((*it).IsTag())
			(*it).RunTag(this);
		else if ((*it).Text() == "\n")
		{
			p.x = point.x;
			p.y += GetFontHeight();
		}
		else
			p.x += PrintText(p, 0, (*it).Text().c_str(), 0);
	}
}

void FrGraphicInterface::PrintText11(const WPoint& point, const FrTEXT& text)
{
	FrTextTokenList tokens;
	text.GetTokenList(tokens);
	WPoint p;
	p = point;
	for (FrTextTokenList::iterator it = tokens.begin(); it != tokens.end();
		++it)
	{
		if ((*it).IsTag())
			(*it).RunTag(this);
		else if ((*it).Text() == "\n")
		{
			p.x = point.x;
			p.y += GetFontHeight11() + TEXT_TOOLTIP_LINE_INTERVAL;
		}
		else
			p.x += PrintText11(p, 0, (*it).Text().c_str(), 0);
	}
}

void FrGraphicInterface::PrintText(const WPoint& point, unsigned long align,
	const FrTEXT& text)
{
	FrTextTokenList tokens;
	text.GetTokenList(tokens);
	std::vector<float> widths;
	float length = 0.0f;
	{
		for (FrTextTokenList::iterator it = tokens.begin(); it != tokens.end();
			++it)
		{
			if (!(*it).IsTag())
			{
				if ((*it).Text() == "\n")
				{
					widths.push_back(length);
					length = 0.0f;
				}
				else
					length +=
						m_pFont12->GetTextWidthOld983(m_pView, (*it).Text().c_str());
			}
		}
	}
	widths.push_back(length);
	std::vector<float>::iterator width = widths.begin();
	WPoint p;
	p = point;
	switch (align)
	{
	case 1:
		p.x -= *width * 0.5f;
		break;
	case 2:
		p.x -= *width;
		break;
	}
	FrTextTokenList::iterator it = tokens.begin();
	if (it != tokens.end())
	{
		++width;
		for (; it != tokens.end(); ++it)
		{
			if ((*it).IsTag())
				(*it).RunTag(this);
			else if ((*it).Text() == "\n")
			{
				p.x = point.x;
				switch (align)
				{
				case 1:
					p.x -= *width * 0.5f;
					break;
				case 2:
					p.x -= *width;
					break;
				}
				p.y += GetFontHeight();
				++width;
			}
			else
				p.x += PrintText(p, 0, (*it).Text().c_str(), 0);
		}
	}
}

void FrGraphicInterface::PrintText11(const WPoint& point, unsigned long align,
	const FrTEXT& text)
{
	FrTextTokenList tokens;
	text.GetTokenList(tokens);
	std::vector<float> widths;
	float length = 0.0f;
	{
		for (FrTextTokenList::iterator it = tokens.begin(); it != tokens.end();
			++it)
		{
			if (!(*it).IsTag())
			{
				if ((*it).Text() == "\n")
				{
					widths.push_back(length);
					length = 0.0f;
				}
				else
					length +=
						m_pFont12->GetTextWidthOld983(m_pView, (*it).Text().c_str());
			}
		}
	}
	widths.push_back(length);
	std::vector<float>::iterator width = widths.begin();
	WPoint p;
	p = point;
	switch (align)
	{
	case 1:
		p.x -= *width * 0.5f;
		break;
	case 2:
		p.x -= *width;
		break;
	}
	FrTextTokenList::iterator it = tokens.begin();
	if (it != tokens.end())
	{
		++width;
		for (; it != tokens.end(); ++it)
		{
			if ((*it).IsTag())
				(*it).RunTag(this);
			else if ((*it).Text() == "\n")
			{
				p.x = point.x;
				switch (align)
				{
				case 1:
					p.x -= *width * 0.5f;
					break;
				case 2:
					p.x -= *width;
					break;
				}
				p.y += GetFontHeight();
				++width;
			}
			else
				p.x += PrintText11(p, 0, (*it).Text().c_str(), 0);
		}
	}
}

// 983 (00bec480, 00bec4a0): medida pelo virtual novo do WFont (slot 6). Funções novas; nomes meus.
void FrGraphicInterface::PrintTitleFont983(WTitleFont* font, float x, float y,
	const char* text, float space, int align, float scale, unsigned long color)
{
	if (font == NULL || text == NULL || strlen(text) == 0)
		return;
	float oldScale = font->GetScale();
	const char* charset = font->GetFontInfo()->pCharSet;
	font->SetScale(scale);
	float width = 0.0f;
	const char* p;
	switch (align)
	{
	case 1:
		{
			for (p = text; *p;)
				width += font->GetCharWidth(g_view, strchr(charset, *p++) - charset) +
					space * scale;
			float w = width - space;
			width = w * 0.5;
		}
		break;
	case 2:
		for (p = text; *p;)
			width += font->GetCharWidth(g_view, strchr(charset, *p++) - charset) +
				space * scale;
		width -= space;
		break;
	}
	x -= width;
	for (p = text; *p;)
		x += font->PutChar(m_pView, x, y, strchr(charset, *p++) - charset, 0, color) +
			space;
	font->SetScale(oldScale);
	font->Flush(m_pView);
}

float FrGraphicInterface::GetTextExtendOld983(const char* text)
{
	return m_pFont12->GetTextWidthOld983(m_pView, text);
}

float FrGraphicInterface::GetTextExtendOld11_983(const char* text)
{
	return m_pFont11->GetTextWidthOld983(m_pView, text);
}

float FrGraphicInterface::GetTextExtend(const char* text)
{
	return m_pFont12->GetTextWidth(m_pView, text);
}

float FrGraphicInterface::GetTextExtend11(const char* text)
{
	return m_pFont11->GetTextWidth(m_pView, text);
}

int FrGraphicInterface::GetFontHeight(void)
{
	return m_pFont12->GetFontHeight();
}

int FrGraphicInterface::GetFontHeight11(void)
{
	return m_pFont11->GetFontHeight();
}

void FrGraphicInterface::SetTextColor(unsigned long color,
	unsigned long outline)
{
	m_textColor = color;
	m_textOutlineColor = outline;
}

void FrGraphicInterface::SetTextStyle(unsigned long style)
{
	m_textStyle = style;
}

int FrGraphicInterface::SetSpace(int space)
{
	int previous = m_pFont12->GetSpace();
	m_pFont12->SetSpace(space);
	return previous;
}

int FrGraphicInterface::SetSpace11(int space)
{
	int previous = m_pFont11->GetSpace();
	m_pFont11->SetSpace(space);
	return previous;
}

void FrGraphicInterface::SetClippingArea(WRect* pRect)
{
	m_pFont12->SetClippingArea(pRect);
}

void FrGraphicInterface::SetClippingArea11(WRect* pRect)
{
	m_pFont11->SetClippingArea(pRect);
}

void FrGraphicInterface::SetAlpha(float alpha)
{
	m_alpha = alpha;
}

float FrGraphicInterface::GetAlpha(void)
{
	return m_alpha;
}

void FrGraphicInterface::Reset(void)
{
	if (m_pCacheManager != 0)
		m_pCacheManager->ClearAll();
	m_alpha = 1.0f;
}

void FrGraphicInterface::SetScale(float scale)
{
	m_pFont12->SetScale(scale);
}

void FrGraphicInterface::SetScale11(float scale)
{
	m_pFont11->SetScale(scale);
}

void FrGraphicInterface::RefreshTexCache(const Bitmap& bitmap)
{
	if (m_pCacheManager != 0)
		m_pCacheManager->RefreshTexCache(bitmap);
}

void FrGraphicInterface::InvalidateCache(const Bitmap& bitmap)
{
	if (m_pCacheManager != 0)
		m_pCacheManager->InvalidateCache(bitmap);
}
