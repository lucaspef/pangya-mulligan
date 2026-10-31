#include "rectcache.h"

RectCache::RectCache()
{
	m_Width = m_Height = m_BlockWidth = m_BlockHeight = 0;
	m_BlockCountW = 0;
	m_BlockCountH = 0;

	m_ppTable = NULL;
	m_EmptyArea = 0;
}

RectCache::~RectCache()
{
	if (m_ppTable)
	{
		delete[] m_ppTable;
		m_ppTable = NULL;
	}
}

void RectCache::Init(int w, int h, int bw, int bh)
{
	m_Width = w;
	m_Height = h;
	m_BlockWidth = bw;
	m_BlockHeight = bh;

	m_BlockCountW = m_Width / m_BlockWidth;
	m_BlockCountH = m_Height / m_BlockHeight;

	if (m_ppTable)
	{
		delete[] m_ppTable;
		m_ppTable = NULL;
	}

	m_ppTable = new LPRECTINFO[m_BlockCountW * m_BlockCountH];
	Clear();
}

void RectCache::Clear()
{
	for (int i = 0; i < m_BlockCountW * m_BlockCountH; ++i)
		m_ppTable[i] = NULL;

	m_EmptyArea = m_BlockCountW * m_BlockCountH;
}

bool RectCache::Add(RectInfo& info, int width, int height)
{
	const int wInBlock = PixelToBlock_W(width);
	const int hInBlock = PixelToBlock_H(height);

	const int blockArea = wInBlock * hInBlock;
	if (blockArea > m_EmptyArea)
	{
		return false;
	}

	const int xMax = m_BlockCountW - wInBlock + 1;
	const int yMax = m_BlockCountH - hInBlock + 1;

	for (int y = 0; y < yMax;)
	{
		int x = 0;
		LPRECTINFO pMinHeightInfo = NULL;
		while (x < xMax)
		{
			LPRECTINFO pInfo = NULL;
			eCRRes res =
				CheckRect(x, y, wInBlock, hInBlock, pInfo, pMinHeightInfo);
			switch (res)
			{
			case eCRRes_RectOnX:
				if (x >= pInfo->rcBlock.right)
				{
					return false;
				}

				x = pInfo->rcBlock.right;
				break;

			case eCRRes_RectOnY:
				x = SkipX(x + wInBlock, y, min(pInfo->rcBlock.right, xMax),
					pMinHeightInfo);
				break;

			case eCRRes_Empty:
				info.rcBlock.left = x;
				info.rcBlock.top = y;
				info.rcBlock.right = x + wInBlock;
				info.rcBlock.bottom = y + hInBlock;

				info.rcPixel.left = BlockToPixel_X(x);
				info.rcPixel.top = BlockToPixel_Y(y);
				info.rcPixel.right = info.rcPixel.left + width;
				info.rcPixel.bottom = info.rcPixel.top + height;

				FillRect(info);

				m_EmptyArea -= blockArea;

				return true;

			case eCRRes_Error:
				return false;
			}
		}

		if (pMinHeightInfo)
		{
			if (pMinHeightInfo->rcBlock.bottom > y)
				y = pMinHeightInfo->rcBlock.bottom;
			else
				++y;
		}
		else
		{
			++y;
		}
	}

	return false;
}
