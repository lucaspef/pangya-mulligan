#include "minatl.h"
#include "uccbase.h"
#include "actor.h"
#include "frarea.h"
#include "inputmanager.h"
#include "taskmanager.h"
#include "uccbrush.h"
#include "uccdrawdlg.h"
#include "wresrcmng.h"


void MixColor(sRGBA& base, sRGBA& overlay, sRGBA& result);
void SetBrightness(sRGBA& color, sRGBA& mask, sRGBA& result);
std::string GenerateTextureName_F(unsigned long typeId, const char* uccIndex);
std::string GenerateTextureName_B(unsigned long typeId, const char* uccIndex);

static int s_drawCount = 0;

void CDrawHistory::Add(Bitmap* before, Bitmap* after, IPoint min, IPoint max)
{
	if (before == NULL || after == NULL)
		return;

	if (before->bi->bmiHeader.biBitCount != after->bi->bmiHeader.biBitCount ||
		before->bi->bmiHeader.biWidth != after->bi->bmiHeader.biWidth ||
		before->bi->bmiHeader.biHeight != after->bi->bmiHeader.biHeight)
		return;

	if (m_history.size() != 0)
	{
		unsigned int i = m_current + 1;
		if (i < m_history.size())
		{
			for (; i < m_history.size(); i++)
			{
				delete m_history[i];
			}
			m_history.resize(m_current + 1);
		}
	}

	min.x = Max(0, min.x - 12);
	min.y = Max(0, min.y - 12);
	int width = before->bi->bmiHeader.biWidth;
	int x1 = Min(width, max.x + 12);
	int height = before->bi->bmiHeader.biHeight;
	max.y = Min(height, max.y + 12);

	sDrawHistory* history = new sDrawHistory;
	history->x = min.x;
	history->y = min.y;
	history->width = x1 - min.x;
	history->height = max.y - min.y;
	history->undo.Create(history->width, history->height,
		before->BitsPerPixel());
	history->redo.Create(history->width, history->height,
		before->BitsPerPixel());
	CopyBitmap(before, &history->undo, history->x, history->y, history->width,
		history->height, 0, 0);
	CopyBitmap(after, &history->redo, history->x, history->y, history->width,
		history->height, 0, 0);
	m_history.push_back(history);
	m_current++;

	unsigned int excess = m_history.size();
	excess -= 30;
	if ((int)excess > 0)
	{
		m_current -= excess;
		for (int i = 0; i < excess; i++)
		{
			delete m_history[i];
		}
		for (unsigned int j = excess; j < m_history.size(); j++)
		{
			m_history[j - excess] = m_history[j];
		}
		for (unsigned int k = 30; k < m_history.size(); k++)
		{
			m_history[k] = NULL;
		}
		m_history.resize(30);
	}
}

void CDrawHistory::Clear()
{
	for (unsigned int i = 0; i < m_history.size(); i++)
	{
		delete m_history[i];
	}
	m_history.clear();
	m_current = -1;
}

void CDrawHistory::Undo(Bitmap* bitmap)
{
	if (bitmap != NULL && m_current >= 0 && m_current < m_history.size() &&
		m_history.size() != 0)
	{
		sDrawHistory* history = m_history[m_current];
		if (history != NULL)
		{
			CopyBitmap(&history->undo, bitmap, 0, 0, history->width,
				history->height, history->x, history->y);
		}
		m_current--;
	}
}

void CDrawHistory::Redo(Bitmap* bitmap)
{
	if (bitmap != NULL && m_current >= -1 &&
		m_current < (int)m_history.size() - 1 && m_history.size() != 0)
	{
		m_current++;
		sDrawHistory* history = m_history[m_current];
		if (history != NULL)
		{
			CopyBitmap(&history->redo, bitmap, 0, 0, history->width,
				history->height, history->x, history->y);
		}
	}
}

bool CDrawHistory::IsUndoPossible()
{
	return m_current >= 0 && m_current < m_history.size() &&
		m_history.size() != 0;
}

bool CDrawHistory::IsRedoPossible()
{
	return m_current >= -1 && m_current < (int)m_history.size() - 1 &&
		m_history.size() != 0;
}

void CDrawHistory::Draw(Bitmap* bitmap, sDrawHistory* history, bool bRedo)
{
	if (bitmap != NULL && history != NULL)
	{
		Bitmap* src = bRedo ? &history->redo : &history->undo;
		CopyBitmap(src, bitmap, 0, 0, history->width, history->height,
			history->x, history->y);
	}
}

void CDrawHistory::CopyBitmap(Bitmap* src, Bitmap* dst, int sx, int sy, int w,
	int h, int dx, int dy)
{
	if (src != NULL && dst != NULL)
	{
		BITMAPINFO* srcInfo = src->bi;
		BITMAPINFO* dstInfo = dst->bi;
		unsigned short bpp = srcInfo->bmiHeader.biBitCount;
		if (bpp == dstInfo->bmiHeader.biBitCount &&
			(unsigned int)(sx + w) <=
				(unsigned int)srcInfo->bmiHeader.biWidth &&
			(unsigned int)(sy + h) <=
				(unsigned int)srcInfo->bmiHeader.biHeight &&
			(unsigned int)(dx + w) <=
				(unsigned int)dstInfo->bmiHeader.biWidth &&
			(unsigned int)(h + dy) <= (unsigned int)dstInfo->bmiHeader.biHeight)
		{
			if (0)
				src->BitsPerPixel();
			unsigned int bytes = bpp >> 3;
			for (int y = sy; y < sy + h; y++)
			{
				memcpy(dst->vram + bytes * dx + (dy - sy + y) * dst->pitch,
					src->vram + bytes * sx + src->pitch * y, bytes * w);
			}
		}
	}
}

CUccBase::CUccBase()
{
	m_pTexture = NULL;
	m_pMask = NULL;
	m_pArea = NULL;
	m_pTexture32 = NULL;
	m_pBackup = NULL;
	m_pBackup32 = NULL;
	m_pDefaultBrush = NULL;
	m_bDragging = false;
	m_refresh = 0;
	m_pZoomed = NULL;
	m_mode = MODE_DRAW;
	m_pParent = NULL;
	m_lastPos.x = -1;
	m_lastPos.y = -1;
	m_zoomRate = 1;
}

CUccBase::~CUccBase()
{
	DestroyVariables();
}

void CUccBase::InitVariables()
{
	m_pTexture = NULL;
	m_pMask = NULL;
	m_pArea = NULL;
	m_pTexture32 = NULL;
	m_pBackup = NULL;
	m_pBackup32 = NULL;
	m_pDefaultBrush = NULL;
	m_bDragging = false;
	m_refresh = 0;
	m_lastPos.x = -1;
	m_lastPos.y = -1;
	m_zoomRate = 1;
	m_pZoomed = NULL;
	m_mode = MODE_DRAW;
	m_pParent = NULL;
}

void CUccBase::DestroyVariables()
{
	ClearTexCache(m_pTexture);
	delete m_pTexture;
	m_pTexture = NULL;
	ClearTexCache(m_pMask);
	delete m_pMask;
	m_pMask = NULL;
	ClearTexCache(m_pTexture32);
	delete m_pTexture32;
	m_pTexture32 = NULL;
	ClearTexCache(m_pBackup);
	delete m_pBackup;
	m_pBackup = NULL;
	ClearTexCache(m_pBackup32);
	delete m_pBackup32;
	m_pBackup32 = NULL;
	ClearTexCache(m_pZoomed);
	delete m_pZoomed;
	m_pZoomed = NULL;
}

void CUccBase::Initialize(FrArea* area, Bitmap* texture, Bitmap* mask)
{
	if (m_pArea == NULL && area != NULL)
		SetArea(area);
	if (m_pTexture == NULL && texture != NULL && m_pArea != NULL)
		SetTexture(texture);
	if (m_pMask == NULL && mask != NULL)
		SetMask(mask);
	m_refresh = 7;
}

void CUccBase::Process()
{
	if (m_refresh & 1)
		Refresh2D();
	if (m_refresh & 2)
		Refresh3D();
	if (m_refresh & 4)
		RefreshCache();
}

void CUccBase::MouseEventHandler(eButton button, int event, const WPoint& pos)
{
	switch (m_mode)
	{
	case MODE_DRAW:
		DrawEventHandler(button, event, pos);
		break;
	case MODE_DRAG:
		MoveEventHandler(button, event, pos);
		break;
	case MODE_SPOID:
		SpoidEventHandler(button, event, pos);
		break;
	case MODE_PAINT:
		PaintEventHandler(button, event, pos);
		break;
	case MODE_ZOOMIN:
	case MODE_ZOOMOUT:
		ZoomEventHandler(button, event, pos);
		break;
	}
}

void CUccBase::Refresh2D()
{
	Convert24to32(m_pTexture, m_pTexture32, m_pMask);
	if (m_pArea != NULL && m_pTexture32 != NULL)
		m_pArea->SetBgImg(m_pTexture32);
	m_refresh &= ~1;
}

void CUccBase::Refresh3D()
{
	m_refresh &= ~2;
}

void CUccBase::RefreshCache()
{
	GetGDI();
	if (m_pTexture32 != NULL)
		RefreshTexCache(m_pTexture32);
	if (m_zoomRate > 1 && m_pZoomed != NULL)
		RefreshTexCache(m_pZoomed);
	m_refresh &= ~4;
}

void CUccBase::Draw(int x, int y, Bitmap* brush, bool bFirst)
{
	if (brush == NULL)
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xba\xea\xb7\xaf\xbd\xac \xc1\xa4\xba\xb8\xb8\xa6 \xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\n\xc6\xce\xbe\xdf \xb0\xd4\xc0\xd3\xc0\xbb \xbf\xcf\xc0\xfc\xc8\xf7 \xc1\xbe\xb7\xe1\xc7\xcf\xbc\xcc\xb4\xd9\xb0\xa1 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
			0, 0, 0, 0));
		return;
	}
	if (!IsInitialized())
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc3\xca\xb1\xe2\xc8\xad\xbf\xa1 \xbd\xc7\xc6\xd0\xc7\xcf\xbf\xa9 \xb1\xd7\xb8\xb1 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \n\n\xc6\xce\xbe\xdf \xb0\xd4\xc0\xd3\xc0\xbb \xbf\xcf\xc0\xfc\xc8\xf7 \xc1\xbe\xb7\xe1\xc7\xcf\xbc\xcc\xb4\xd9\xb0\xa1 \xb4\xd9\xbd\xc3 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
			0, 0, 0, 0));
		return;
	}

	if (m_zoomRate > 1)
	{
		x = (int)((float)(x / m_zoomRate) + m_zoomArea.x);
		y = (int)((float)(y / m_zoomRate) + m_zoomArea.y);
	}

	int minX = x - brush->bi->bmiHeader.biWidth / 2;
	if (minX < m_min.x)
		m_min.x = Max(0, minX);
	int maxX = brush->bi->bmiHeader.biWidth / 2 + x;
	if (maxX > m_max.x)
		m_max.x = Min((int)m_pTexture->bi->bmiHeader.biWidth - 1, maxX);
	int minY = y - brush->bi->bmiHeader.biHeight / 2;
	if (minY < m_min.y)
		m_min.y = Max(0, minY);
	int maxY = brush->bi->bmiHeader.biHeight / 2 + y;
	if (maxY > m_max.y)
		m_max.y = Min((int)m_pTexture->bi->bmiHeader.biHeight - 1, maxY);

	unsigned int width = brush->bi->bmiHeader.biWidth;
	unsigned int height = brush->bi->bmiHeader.biHeight;
	int left = x - (width >> 1);
	int top = y - (height >> 1);
	int bottom = height + top;
	int right = width + left;

	sRGBA color;
	sRGBA brushPixel;
	sRGBA maskPixel;
	sRGBA texPixel;
	sRGBA result;
	sRGBA bright;
	UccCurrentBrush()->GetBrushColor(color.r, color.g, color.b);
	s_drawCount++;
	color.a = 0xff;

	int startX = Max(left, 0);
	int startY = Max(top, 0);
	int brushY = startY - top;
	for (int cy = startY; cy < bottom; cy++)
	{
		if ((unsigned int)cy >= m_pTexture->Height())
			break;
		int brushX = startX - left;
		for (int cx = startX; cx < right; cx++)
		{
			if ((unsigned int)cx >= m_pTexture->Width())
				break;
			if (m_pMask != NULL)
			{
				m_pMask->GetPixel(cx, cy, maskPixel.r, maskPixel.g,
					maskPixel.b);
				if (maskPixel.r < 0x10 && maskPixel.g < 0x10 &&
					maskPixel.b < 0x10)
				{
					brushX++;
					continue;
				}
			}
			else
			{
				maskPixel.r = 0;
				maskPixel.g = 0;
				maskPixel.b = 0;
			}
			brush->GetPixel(brushX, brushY, brushPixel.r, brushPixel.g,
				brushPixel.b, brushPixel.a);
			if (brushPixel.a != 0)
			{
				m_pTexture32->GetPixel(cx, cy, texPixel.r, texPixel.g,
					texPixel.b, texPixel.a);
				sRGBA mixed;
				MixColor(texPixel, brushPixel, mixed);
				SetBrightness(mixed, maskPixel, bright);
				MixColor(texPixel, brushPixel, result);
				if (m_pMask != NULL)
				{
					m_pTexture->SetPixel(cx, cy, bright.r, bright.g, bright.b,
						0xff);
					m_pTexture32->SetPixel(cx, cy, result.r, result.g, result.b,
						result.a);
					if (m_zoomRate > 1 && m_pZoomed != NULL)
					{
						int zx = (int)(((float)cx - m_zoomArea.x) *
							(float)m_zoomRate);
						zx = Max(0, zx);
						int zy = (int)(((float)cy - m_zoomArea.y) *
							(float)m_zoomRate);
						zy = Max(0, zy);
						int zw = m_pZoomed->bi->bmiHeader.biWidth;
						int zh = m_pZoomed->bi->bmiHeader.biHeight;
						int ex = Min(m_zoomRate + zx, zw);
						int ey = Min(m_zoomRate + zy, zh);
						for (int py = zy; py < ey; py++)
						{
							for (int px = zx; px < ex; px++)
							{
								m_pZoomed->SetPixel(px, py, result.r, result.g,
									result.b, result.a);
							}
						}
					}
				}
				else
				{
					m_pTexture->SetPixel(cx, cy, result.r, result.g, result.b,
						0xff);
					m_pTexture32->SetPixel(cx, cy, result.r, result.g, result.b,
						result.a);
					if (m_zoomRate > 1 && m_pZoomed != NULL)
					{
						int zx = (int)(((float)cx - m_zoomArea.x) *
							(float)m_zoomRate);
						zx = Max(0, zx);
						int zy = (int)(((float)cy - m_zoomArea.y) *
							(float)m_zoomRate);
						zy = Max(0, zy);
						int zw = m_pZoomed->bi->bmiHeader.biWidth;
						int zh = m_pZoomed->bi->bmiHeader.biHeight;
						int ex = Min(m_zoomRate + zx, zw);
						int ey = Min(m_zoomRate + zy, zh);
						for (int py = zy; py < ey; py++)
						{
							for (int px = zx; px < ex; px++)
							{
								m_pZoomed->SetPixel(px, py, result.r, result.g,
									result.b, result.a);
							}
						}
					}
				}
			}
			brushX++;
		}
		brushY++;
	}

	if (bFirst)
		m_refresh |= 4;
}

void CUccBase::DrawLine(IPoint from, IPoint to, Bitmap* brush)
{
	if (brush == NULL)
		return;

	int dy = to.y - from.y;
	int ady = abs(dy);
	int dx = to.x - from.x;
	int adx = abs(dx);
	int steps = Max(adx, ady);

	float stepX = (float)dx / (float)steps;
	float stepY = (float)dy / (float)steps;
	float fx = (float)from.x;
	float fy = (float)from.y;
	while (true)
	{
		Draw((int)fx, (int)fy, brush, false);
		if ((steps == adx && fx == (float)to.x) ||
			(steps == ady && fy == (float)to.y))
			break;
		fx += stepX;
		fy += stepY;
	}
	m_refresh |= 4;
}

void CUccBase::Undo()
{
	m_history.Undo(m_pTexture);
	m_history32.Undo(m_pTexture32);
	if (IsZoomed())
		_DrawZoomedArea();
	MustRefresh(7);
}

void CUccBase::Redo()
{
	m_history.Redo(m_pTexture);
	m_history32.Redo(m_pTexture32);
	if (IsZoomed())
		_DrawZoomedArea();
	if (0)
		GetZoomRate();
	MustRefresh(7);
}

bool CUccBase::IsUndoPossible()
{
	return m_history.IsUndoPossible();
}

bool CUccBase::IsRedoPossible()
{
	return m_history.IsRedoPossible();
}

void CUccBase::SetTexture(Bitmap* texture)
{
	ClearTexCache(m_pTexture);
	delete m_pTexture;
	m_pTexture = NULL;
	m_pTexture = new Bitmap;
	*m_pTexture = *texture;
	ClearTexCache(m_pTexture32);
	delete m_pTexture32;
	m_pTexture32 = NULL;
	m_pTexture32 = new Bitmap(m_pTexture->Width(), m_pTexture->Height(), 32);
	m_refresh |= 1;
}

void CUccBase::SetMask(Bitmap* mask)
{
	ClearTexCache(m_pMask);
	delete m_pMask;
	m_pMask = NULL;
	m_pMask = new Bitmap;
	*m_pMask = *mask;
}

void CUccBase::SetDefaultBrush(Bitmap* brush)
{
	ClearTexCache(m_pDefaultBrush);
	delete m_pDefaultBrush;
	m_pDefaultBrush = NULL;
	m_pDefaultBrush = new Bitmap;
	*m_pDefaultBrush = *brush;
}

bool CUccBase::IsMouseOver(const WPoint& pos)
{
	if (m_pArea == NULL)
		return false;
	if (m_pTexture == NULL && m_pTexture32 == NULL)
		return false;
	if (!m_pArea->IsVisible())
		return false;
	if (!m_pArea->IsViewFocused())
		return false;

	WRect rect = m_pArea->GetRect();
	int w = 0;
	int h = 0;
	if (m_zoomRate > 1)
	{
		w = m_pZoomed->bi->bmiHeader.biWidth;
		h = m_pZoomed->bi->bmiHeader.biHeight;
	}
	else if (m_pTexture != NULL)
	{
		w = m_pTexture->bi->bmiHeader.biWidth;
		h = m_pTexture->bi->bmiHeader.biHeight;
	}
	else if (m_pTexture32 != NULL)
	{
		w = m_pTexture32->bi->bmiHeader.biWidth;
		h = m_pTexture32->bi->bmiHeader.biHeight;
	}
	if (rect.x <= pos.x && pos.x < (float)w + rect.x && rect.y <= pos.y &&
		pos.y < (float)h + rect.y)
		return true;
	return false;
}

Bitmap* CUccBase::GetTexture32BPP(bool bOriginal)
{
	if (m_zoomRate > 1 && !bOriginal)
		return m_pZoomed;
	return m_pTexture32;
}

bool CUccBase::ZoomIn(int x, int y)
{
	if (m_zoomRate >= 8)
		return false;
	m_zoomRate++;
	MakeZoomedArea(x, y);
	return true;
}

bool CUccBase::ZoomOut(int x, int y)
{
	if (m_zoomRate <= 1)
		return false;
	m_zoomRate--;
	MakeZoomedArea(x, y);
	return true;
}

void CUccBase::MakeZoomedArea(int x, int y)
{
	int zoomRate = m_zoomRate;
	Bitmap* texture32 = m_pTexture32;
	if (zoomRate == 1)
	{
		if (texture32 != NULL)
		{
			WRect rect = m_pArea->GetRect();
			unsigned int width = texture32->Width();
			unsigned int height = texture32->Height();
			rect.w = (float)width;
			rect.h = (float)height;
			m_pArea->SetRect(rect);
			RefreshTexCache(m_pTexture32);
			m_pArea->SetBgImg(m_pTexture32);
		}
		return;
	}

	int halfW = texture32->bi->bmiHeader.biWidth / 2;
	int halfH = texture32->bi->bmiHeader.biHeight / 2;
	if (x == -1 && y == -1)
	{
		x = (int)(m_zoomArea.w * 0.5f + m_zoomArea.x);
		y = (int)(m_zoomArea.h * 0.5f + m_zoomArea.y);
	}
	else
	{
		if (zoomRate * x < 0x80)
			x = 0x100 / (zoomRate * 2);
		BITMAPINFO* bi = m_pTexture->bi;
		if ((unsigned int)((bi->bmiHeader.biWidth - x) * zoomRate) < 0x80)
			x = bi->bmiHeader.biWidth - 0x100 / (zoomRate * 2);
		if (zoomRate * y < 0x80)
			y = 0x100 / (zoomRate * 2);
		if ((unsigned int)((bi->bmiHeader.biHeight - y) * zoomRate) < 0x80)
			y = bi->bmiHeader.biHeight - 0x100 / (zoomRate * 2);
	}

	int left = x - halfW;
	int top = y - halfH;
	int w = m_pTexture32->bi->bmiHeader.biWidth;
	int h = m_pTexture32->bi->bmiHeader.biHeight;
	int zw = zoomRate * w;
	if (zw <= 0x100)
	{
		m_zoomArea.x = 0;
	}
	else
	{
		int margin = (zw - 0x100) / 2;
		w = (zw - margin * 2) / zoomRate;
		m_zoomArea.x = (float)(margin / zoomRate - 1);
	}
	m_zoomArea.w = (float)w;
	int zh = zoomRate * h;
	if (zh <= 0x100)
	{
		m_zoomArea.y = 0;
	}
	else
	{
		int margin = (zh - 0x100) / 2;
		m_zoomArea.y = (float)(margin / zoomRate - 1);
		h = (zh - margin * 2) / zoomRate;
	}
	m_zoomArea.h = (float)h;

	m_zoomArea.x = (float)left + m_zoomArea.x;
	m_zoomArea.y = (float)top + m_zoomArea.y;
	if (m_zoomArea.x < 0)
		m_zoomArea.x = 0;
	if (m_zoomArea.y < 0)
		m_zoomArea.y = 0;

	float texW = (float)m_pTexture32->Width();
	if (texW < m_zoomArea.w + m_zoomArea.x)
		m_zoomArea.x = texW - m_zoomArea.w;
	float texH = (float)m_pTexture32->Height();
	if (texH < m_zoomArea.h + m_zoomArea.y)
		m_zoomArea.y = texH - m_zoomArea.h;
	_DrawZoomedArea();
}

void CUccBase::_DrawZoomedArea()
{
	int w = (int)((float)m_zoomRate * m_zoomArea.w);
	if (w >= 0x100)
		w = 0x100;
	int rawH = (int)((float)m_zoomRate * m_zoomArea.h);
	int h = 0x100;
	if (rawH < 0x100)
		h = rawH;

	m_pArea->SetBgImg((const Bitmap*)NULL);
	ClearTexCache(m_pZoomed);
	delete m_pZoomed;
	m_pZoomed = NULL;
	m_pZoomed = new Bitmap(w, h, 32);
	if (m_pZoomed != NULL)
	{
		for (int sy = (int)m_zoomArea.y;
			(float)sy < m_zoomArea.h + m_zoomArea.y; sy++)
		{
			for (int sx = (int)m_zoomArea.x;
				(float)sx < m_zoomArea.w + m_zoomArea.x; sx++)
			{
				unsigned char r, g, b, a;
				m_pTexture32->GetPixel(sx, sy, r, g, b, a);
				float fx = (float)sx - m_zoomArea.x;
				int x0 = (int)((float)m_zoomRate * fx);
				int x1 = (int)((1.0f + fx) * (float)m_zoomRate);
				float fy = (float)sy - m_zoomArea.y;
				int y0 = (int)((float)m_zoomRate * fy);
				int y1 = (int)((1.0f + fy) * (float)m_zoomRate);
				for (int py = y0; py < y1; py++)
				{
					for (int px = x0; px < x1; px++)
					{
						m_pZoomed->SetPixel(px, py, r, g, b, a);
					}
				}
			}
		}
		m_pArea->SetRect(WRect(m_pArea->GetRect().x, m_pArea->GetRect().y,
			m_pZoomed->Width(), m_pZoomed->Height()));
		m_pArea->SetBgImg(m_pZoomed);
	}
}

void CUccBase::DrawEventHandler(eButton button, int event, const WPoint& pos)
{
	Bitmap* brush = m_pDefaultBrush;
	if (brush == NULL)
		brush = &UccCurrentBrush()->brush;

	if (button == LEFT_BUTTON)
	{
		switch (event)
		{
		case 3:
			if (m_bDragging)
			{
				if (m_pArea != NULL)
				{
					WRect rect = m_pArea->GetRect();
					IPoint p(pos.x - rect.x, pos.y - rect.y);
					if (!(m_lastPos == p))
					{
						if (m_lastPos.x == -1 && m_lastPos.y == -1)
						{
							Draw(p.x, p.y, brush, true);
							m_lastPos = p;
							return;
						}
						DrawLine(m_lastPos, p, brush);
						m_lastPos = p;
						return;
					}
				}
				else
				{
					Draw((int)pos.x, (int)pos.y, brush, true);
					return;
				}
			}
			break;
		case 2:
			m_bDragging = false;
			m_lastPos.x = -1;
			m_lastPos.y = -1;
			m_history.Add(m_pBackup, m_pTexture, m_min, m_max);
			m_history32.Add(m_pBackup32, m_pTexture32, m_min, m_max);
			m_refresh |= 2;
			m_pParent->SetSaved(false);
			return;
		case 1:
			m_bDragging = true;
			ClearTexCache(m_pBackup);
			delete m_pBackup;
			m_pBackup = NULL;
			m_pBackup = new Bitmap;
			*m_pBackup = *m_pTexture;
			ClearTexCache(m_pBackup32);
			delete m_pBackup32;
			m_pBackup32 = NULL;
			m_pBackup32 = new Bitmap;
			*m_pBackup32 = *m_pTexture32;
			m_min.x = m_pTexture->bi->bmiHeader.biWidth;
			m_min.y = m_pTexture->bi->bmiHeader.biHeight;
			m_max.y = 0;
			m_max.x = 0;
			if (m_pArea == NULL)
			{
				Draw((int)pos.x, (int)pos.y, brush, true);
				m_lastPos.x = -1;
				m_lastPos.y = -1;
			}
			else
			{
				IPoint p(pos.x - m_pArea->GetRect().x,
					pos.y - m_pArea->GetRect().y);
				if (!(m_lastPos == p))
				{
					Draw(p.x, p.y, brush, true);
					m_lastPos = p;
					return;
				}
			}
			break;
		}
	}
}

void CUccBase::MoveEventHandler(eButton button, int event, const WPoint& pos)
{
	if (m_zoomRate > 1)
	{
		if (m_zoomArea.x == 0 && m_zoomArea.w == (float)m_pTexture32->Width() &&
			m_zoomArea.y == 0 && m_zoomArea.h == (float)m_pTexture32->Height())
			return;

		if (button == LEFT_BUTTON)
		{
			switch (event)
			{
			case 3:
			{
				WRect rect = m_pArea->GetRect();
				IPoint p(pos.x - rect.x, pos.y - rect.y);
				if (!(p == m_lastPos) && m_lastPos.x != -1 && m_lastPos.y != -1)
				{
					IPoint delta((m_lastPos.x - p.x) / m_zoomRate,
						(m_lastPos.y - p.y) / m_zoomRate);
					if (delta.x != 0 || delta.y != 0)
					{
						if ((float)delta.x + (m_zoomArea.w + m_zoomArea.x) >
							(float)m_pTexture32->Width())
							delta.x = (int)((float)m_pTexture32->Width() -
								(m_zoomArea.w + m_zoomArea.x));
						if ((float)delta.y + (m_zoomArea.h + m_zoomArea.y) >
							(float)m_pTexture32->Height())
							delta.y = (int)((float)m_pTexture32->Height() -
								(m_zoomArea.h + m_zoomArea.y));
						if ((float)delta.x + m_zoomArea.x < 0)
							delta.x = (int)(-m_zoomArea.x);
						if ((float)delta.y + m_zoomArea.y < 0)
							delta.y = (int)(-m_zoomArea.y);
						m_lastPos = p;
						m_zoomArea.x = (float)delta.x + m_zoomArea.x;
						m_zoomArea.y = (float)delta.y + m_zoomArea.y;
						_DrawZoomedArea();
					}
				}
				break;
			}
			case 2:
				m_bDragging = false;
				m_lastPos.x = -1;
				m_lastPos.y = -1;
				break;
			case 1:
			{
				m_bDragging = true;
				WRect rect = m_pArea->GetRect();
				IPoint p(pos.x - rect.x, pos.y - rect.y);
				if (!(p == m_lastPos))
				{
					m_lastPos = p;
				}
				break;
			}
			}
		}
	}
}

void CUccBase::SpoidEventHandler(eButton button, int event, const WPoint& pos)
{
	if (button == LEFT_BUTTON)
	{
		switch (event)
		{
		case 1:
		case 3:
		{
			WRect rect = m_pArea->GetRect();
			IPoint point(pos.x - rect.x, pos.y - rect.y);
			sRGB color;
			Bitmap* bitmap;
			if (m_zoomRate > 1)
				bitmap = m_pZoomed;
			else
				bitmap = m_pTexture32;
			bitmap->GetPixel(point.x, point.y, color.r, color.g, color.b);
			if (m_pParent != NULL)
				m_pParent->SetCurrentColor(color.r, color.g, color.b);
			break;
		}
		case 2:
			m_mode = MODE_DRAW;
			break;
		}
	}
}

void CUccBase::PaintEventHandler(eButton button, int event, const WPoint& pos)
{
	if (button == LEFT_BUTTON)
	{
		switch (event)
		{
		case 2:
		{
			m_pBackup = new Bitmap;
			*m_pBackup = *m_pTexture;
			ClearTexCache(m_pBackup32);
			delete m_pBackup32;
			m_pBackup32 = NULL;
			m_pBackup32 = new Bitmap;
			*m_pBackup32 = *m_pTexture32;
			WRect rect = m_pArea->GetRect();
			IPoint point(pos.x - rect.x, pos.y - rect.y);
			Paint(point.x, point.y, true, 0, 0, 0);
			if (IsZoomed())
				_DrawZoomedArea();
			IPoint minPoint(0, 0);
			IPoint maxPoint((int)m_pTexture->bi->bmiHeader.biWidth - 1,
				(int)m_pTexture->bi->bmiHeader.biHeight - 1);
			m_history.Add(m_pBackup, m_pTexture, minPoint, maxPoint);
			m_history32.Add(m_pBackup32, m_pTexture32, minPoint, maxPoint);
			MustRefresh(7);
			SetMode(MODE_DRAW);
			m_pParent->SetSaved(false);
			break;
		}
		}
	}
}

void CUccBase::Paint(int x, int y, bool bUseBrushColor, unsigned char r,
	unsigned char g, unsigned char b)
{
	if (bUseBrushColor)
	{
		r = UccCurrentBrush()->r;
		g = UccCurrentBrush()->g;
		b = UccCurrentBrush()->b;
	}
	if (m_zoomRate > 1)
	{
		x = (int)((float)(x / m_zoomRate) + m_zoomArea.x);
		y = (int)((float)(y / m_zoomRate) + m_zoomArea.y);
	}

	sRGBA color(r, g, b);
	std::list<IPoint> queue;
	int dx[4] = { 1, 0, -1, 0 };
	int dy[4] = { 0, 1, 0, -1 };
	queue.push_back(IPoint(x, y));
	char visited[512][512];
	for (unsigned int row = 0; row < m_pTexture->Height(); row++)
	{
		if (m_pTexture->Width() > 0)
		{
			memset(visited[row], 0, m_pTexture->Width());
		}
	}

	while (queue.size() > 0)
	{
		IPoint cur = queue.front();
		queue.pop_front();
		if (visited[cur.y][cur.x] != 1 &&
			m_pTexture32->GetAlpha(cur.x, cur.y) != 0)
		{
			visited[cur.y][cur.x] = 1;
			sRGBA ref;
			m_pTexture32->GetPixel(cur.x, cur.y, ref.r, ref.g, ref.b);
			sRGBA maskPixel;
			sRGBA newColor;
			if (m_pMask != NULL)
			{
				m_pMask->GetPixel(cur.x, cur.y, maskPixel.r, maskPixel.g,
					maskPixel.b);
				SetBrightness(color, maskPixel, newColor);
			}
			else
			{
				newColor = color;
			}
			m_pTexture->SetPixel(cur.x, cur.y, newColor.r, newColor.g,
				newColor.b, 0xff);
			m_pTexture32->SetPixel(cur.x, cur.y, color.r, color.g, color.b,
				0xff);
			for (int i = 0; i < 4; i++)
			{
				int nx = dx[i] + cur.x;
				int ny = dy[i] + cur.y;
				if (nx >= 0 && ny >= 0 &&
					(unsigned int)nx < m_pTexture->Width() &&
					(unsigned int)ny < m_pTexture->Height() &&
					m_pTexture32->GetAlpha(nx, ny) != 0)
				{
					sRGBA neighbor;
					m_pTexture32->GetPixel(nx, ny, neighbor.r, neighbor.g,
						neighbor.b);
					if (visited[ny][nx] == 0 && ref == neighbor)
					{
						queue.push_back(IPoint(nx, ny));
					}
				}
			}
		}
	}
}

void CUccBase::ZoomEventHandler(eButton button, int event, const WPoint& pos)
{
	if (button != LEFT_BUTTON)
		return;

	switch (event)
	{
	case 2:
	{
		WRect rect = m_pArea->GetRect();
		int x = (int)(pos.x - rect.x);
		int y = (int)(pos.y - rect.y);
		if (m_zoomRate > 1)
		{
			x = (int)((float)(x / m_zoomRate) + m_zoomArea.x);
			y = (int)((float)(y / m_zoomRate) + m_zoomArea.y);
		}

		if (m_mode == MODE_ZOOMIN)
		{
			if (m_zoomRate < 8)
			{
				m_zoomRate++;
				MakeZoomedArea(x, y);
			}
			if (m_zoomRate == 8)
				m_mode = MODE_DRAW;
		}
		else if (m_mode == MODE_ZOOMOUT)
		{
			ZoomOut(x, y);
			if (m_zoomRate == 1)
				m_mode = MODE_DRAW;
		}
		if (m_pParent != NULL)
			m_pParent->AfterZoom();
		break;
	}
	}
}

void CUccBaseItemDraw::InitializeItemDraw(int side, FrArea* area,
	Bitmap* texture, Bitmap* mask, unsigned long typeID, const char* uccIndex)
{
	CUccBase::Initialize(area, texture, mask);
	strncpy(m_uccIndex, uccIndex, 9);
	m_typeID = typeID;
	GenerateTextureName(side, typeID, m_uccIndex);
	m_history.Clear();
	m_history32.Clear();
	Convert24to32(m_pTexture, m_pTexture32, m_pMask);
}

CUccBaseItemDraw::CUccBaseItemDraw()
{
	m_textureName[0] = 0;
	m_uccIndex[0] = 0;
	m_typeID = 0;
}

CUccBaseItemDraw::~CUccBaseItemDraw()
{
}

bool CUccBaseItemDraw::IsInitialized()
{
	return strlen(m_uccIndex) > 0 && m_typeID != 0 && CUccBase::IsInitialized();
}

void CUccBaseItemDraw::Refresh2D()
{
	if (m_pArea != NULL)
	{
		Bitmap* bitmap;
		if (m_zoomRate > 1)
			bitmap = m_pZoomed;
		else
			bitmap = m_pTexture32;
		if (bitmap != NULL)
			m_pArea->SetBgImg(bitmap);
	}
	m_refresh &= ~1;
}

void CUccBaseItemDraw::Refresh3D()
{
	w_texlist* tex = g_resrcmng->FindTexture(m_textureName);
	if (tex != NULL)
	{
		if (m_pTexture != NULL)
			g_resrcmng->FixTexture(tex->texhandle, m_pTexture, 0, NULL);
	}
	else
	{
		if (m_textureName != NULL && m_pTexture != NULL)
			g_resrcmng->UploadTexture(m_textureName, m_pTexture, 0, NULL);
	}
	m_refresh &= ~2;
}

void CUccBaseItemDraw::GenerateTextureName(int side, unsigned long typeID,
	const char* uccIndex)
{
	const char* index = uccIndex;
	if (index == NULL)
		index = m_uccIndex;
	if (strlen(index) > 0)
	{
		if (side == 0)
		{
			strcpy(m_textureName,
				GenerateTextureName_F(typeID, uccIndex).c_str());
		}
		else
		{
			strcpy(m_textureName,
				GenerateTextureName_B(typeID, uccIndex).c_str());
		}
	}
}

void CUccBaseItemDraw::Process()
{
	if (m_refresh & 1)
		Refresh2D();
	if (m_refresh & 2)
		Refresh3D();
	if (m_refresh & 4)
		RefreshCache();
}
