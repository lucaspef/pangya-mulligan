#include "wdxtc.h"
#include "wmath.h"
#include <stdio.h>
#include <new>
#include <string.h>

struct sClr32
{
	union
	{
		unsigned int clr;
		struct
		{
			unsigned char b, g, r, a;
		};
	};
};
struct sDXTC_ClrBlk
{
	unsigned short color0, color1;
	unsigned char row[4];
};
struct sDXTC_ExpAlphaBlk
{
	unsigned short row[4];
};
struct sDXTC_LinearAlphaBlk3Bit
{
	unsigned char a0, a1, stuff[6];
};

const DWORD g_dxt1FourCc = MAKEFOURCC('D', 'X', 'T', '1');
const DWORD g_dxt3FourCc = MAKEFOURCC('D', 'X', 'T', '3');
const DWORD g_dxt5FourCc = MAKEFOURCC('D', 'X', 'T', '5');

WDXTC::WDXTC()
{
	this->m_pCompData = 0;
	this->m_pDecompData = 0;
}

WDXTC::~WDXTC()
{
	if (this->m_pDecompData)
	{
		delete[] this->m_pDecompData;
		this->m_pDecompData = 0;
	}
	if (this->m_pCompData)
	{
		delete[] this->m_pCompData;
		this->m_pCompData = 0;
	}
}

LPDDSURFACEDESC2 WDXTC::GetDdsd2()
{
	return &this->m_DDSD2;
}

unsigned char* WDXTC::GetDecompData()
{
	return this->m_pDecompData;
}

void WDXTC::GetColorFromBlock(sClr32* aClr32, const sDXTC_ClrBlk* pBlock)
{
	aClr32[0].a = -1;
	aClr32[0].r = 8 * (pBlock->color0 >> 11);
	aClr32[0].g = 4 * (pBlock->color0 >> 5);
	aClr32[0].b = 8 * pBlock->color0;
	aClr32[1].a = -1;
	aClr32[1].r = 8 * (pBlock->color1 >> 11);
	aClr32[1].g = 4 * (pBlock->color1 >> 5);
	aClr32[1].b = 8 * pBlock->color1;
	if (pBlock->color0 > pBlock->color1)
	{
		aClr32[2].r = (aClr32[1].r + 2 * aClr32[0].r) / 3;
		aClr32[2].g = (aClr32[1].g + 2 * aClr32[0].g) / 3;
		aClr32[2].b = (aClr32[1].b + 2 * aClr32[0].b) / 3;
		aClr32[2].b += aClr32[2].b < 0;
		aClr32[2].a = -1;
		aClr32[3].r = (aClr32[0].r + 2 * aClr32[1].r) / 3;
		aClr32[3].r += aClr32[3].r < 0;
		aClr32[3].g = (aClr32[0].g + 2 * static_cast<BYTE>(aClr32[1].g)) / 3;
		aClr32[3].b = (aClr32[0].b + 2 * aClr32[1].b) / 3;
		aClr32[3].b += aClr32[3].b < 0;
		aClr32[3].a = -1;
	}
	else
	{
		aClr32[2].r =
			(static_cast<BYTE>(aClr32[0].r) + static_cast<BYTE>(aClr32[1].r)) /
			2;
		aClr32[2].g =
			(static_cast<BYTE>(aClr32[0].g) + static_cast<BYTE>(aClr32[1].g)) /
			2;
		aClr32[2].b =
			(static_cast<BYTE>(aClr32[1].b) + static_cast<BYTE>(aClr32[0].b)) /
			2;
		aClr32[2].a = -1;
		aClr32[3].r = 0;
		aClr32[3].g = -1;
		aClr32[3].b = -1;
		aClr32[3].a = 0;
	}
}

void WDXTC::DecodeColor(sClr32* pDecomp, const sDXTC_ClrBlk* pBlock,
	sClr32* aClr32)
{
	const BYTE uiMasks[4] = { 0x03, 0x0C, 0x30, 0xC0 };

	for (int i = 0; i < 4; i++)
	{
		for (int mask = 0; mask < 4; pDecomp++, mask++)
		{
			switch ((pBlock->row[i] & uiMasks[mask]) >> (mask * 2))
			{
			case 0:
				pDecomp->clr = aClr32[0].clr;
				break;
			case 1:
				pDecomp->clr = aClr32[1].clr;
				break;
			case 2:
				pDecomp->clr = aClr32[2].clr;
				break;
			case 3:
				pDecomp->clr = aClr32[3].clr;
				break;
			default:
				break;
			}
		}
		pDecomp += this->m_DDSD2.dwWidth - 4;
	}
}

void WDXTC::DecodeExpAlphaBlock(sClr32* pDecomp,
	const sDXTC_ExpAlphaBlk* pAlpha)
{
	unsigned short usAlpha;
	int i = 0;
	do
	{
		usAlpha = pAlpha->row[i];
		for (int j = 0; j < 4; ++j)
		{
			BYTE alpha = usAlpha & 0xf;
			usAlpha >>= 4;
			(pDecomp++)->a = alpha | (alpha << 4);
		}
		pDecomp += m_DDSD2.dwWidth - 4;
	} while (++i < 4);
}

void WDXTC::DecodeLinearAlphaBlock3Bit(sClr32* pDecomp,
	const sDXTC_LinearAlphaBlk3Bit* pAlpha)
{
	sClr32 gACol[4][4];
	WORD gAlphas[8];
	BYTE gBits[4][4];

	gAlphas[0] = pAlpha->a0;
	gAlphas[1] = pAlpha->a1;

	if (gAlphas[0] > gAlphas[1])
	{
		gAlphas[2] = (6 * gAlphas[0] + gAlphas[1]) / 7;
		gAlphas[3] = (5 * gAlphas[0] + 2 * gAlphas[1]) / 7;
		gAlphas[4] = (4 * gAlphas[0] + 3 * gAlphas[1]) / 7;
		gAlphas[5] = (3 * gAlphas[0] + 4 * gAlphas[1]) / 7;
		gAlphas[6] = (2 * gAlphas[0] + 5 * gAlphas[1]) / 7;
		gAlphas[7] = (gAlphas[0] + 6 * gAlphas[1]) / 7;
	}
	else
	{
		gAlphas[2] = (4 * gAlphas[0] + gAlphas[1]) / 5;
		gAlphas[3] = (3 * gAlphas[0] + 2 * gAlphas[1]) / 5;
		gAlphas[4] = (2 * gAlphas[0] + 3 * gAlphas[1]) / 5;
		gAlphas[5] = (gAlphas[0] + 4 * gAlphas[1]) / 5;
		gAlphas[6] = 0;
		gAlphas[7] = 255;
	}

	DWORD stuff = *reinterpret_cast<const DWORD*>(&pAlpha->stuff[0]);
	gBits[0][0] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[0][1] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[0][2] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[0][3] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[1][0] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[1][1] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[1][2] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[1][3] = BYTE(stuff & 7);
	stuff = *reinterpret_cast<const DWORD*>(&pAlpha->stuff[3]);
	gBits[2][0] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[2][1] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[2][2] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[2][3] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[3][0] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[3][1] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[3][2] = BYTE(stuff & 7);
	stuff >>= 3;
	gBits[3][3] = BYTE(stuff & 7);

	int row, pix;
	for (row = 0; row < 4; row++)
	{
		for (pix = 0; pix < 4; pix++)
			gACol[row][pix].a = BYTE(gAlphas[gBits[row][pix]]);
	}
	for (row = 0; row < 4; row++, pDecomp += m_DDSD2.dwWidth - 4)
	{
		for (pix = 0; pix < 4; pix++)
		{
			pDecomp->a = gACol[row][pix].a;
			pDecomp++;
		}
	}
}

void WDXTC::SaveAsBmp(char* filename) const
{
	FILE* pfRgb;
	FILE* pfAlpha;
	unsigned char BGR[3];
	unsigned char Alpha[3];
	char strBuf[64];

	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;
	bfh.bfType = 0x4D42;
	bih.biCompression = 0;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = 54;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biPlanes = 1;
	bih.biBitCount = 24;
	bfh.bfSize = 54 + 3 * m_DDSD2.dwWidth * m_DDSD2.dwHeight;
	bih.biWidth = m_DDSD2.dwWidth;
	bih.biHeight = m_DDSD2.dwHeight;

	sprintf(strBuf, "%s_RGB.bmp", filename);
	pfRgb = fopen(strBuf, "wb");
	sprintf(strBuf, "%s_Alpha.bmp", filename);
	pfAlpha = fopen(strBuf, "wb");
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, pfRgb);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, pfRgb);
	fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, pfAlpha);
	fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, pfAlpha);

	for (int y = static_cast<int>(this->m_DDSD2.dwHeight - 1); y >= 0; y--)
	{
		unsigned char* ptr = &this->m_pDecompData[4 * y * m_DDSD2.dwWidth];
		for (UINT dw = 0; dw < m_DDSD2.dwWidth; dw++)
		{
			BGR[2] = ptr[0];
			BGR[1] = ptr[1];
			BGR[0] = ptr[2];
			Alpha[2] = ptr[3];
			Alpha[1] = Alpha[2];
			Alpha[0] = Alpha[2];
			fwrite(BGR, 3, 1, pfRgb);
			fwrite(Alpha, 3, 1, pfAlpha);
			ptr += 4;
		}
	}

	fclose(pfAlpha);
	fclose(pfRgb);
}

void WDXTC::DecompBlock_DXT1(unsigned char* pDecomp, const sDXTC_ClrBlk* pBlock)
{
	sClr32 aClr[4];
	this->GetColorFromBlock(aClr, pBlock);
	this->DecodeColor(reinterpret_cast<sClr32*>(pDecomp), pBlock, aClr);
}

void WDXTC::DecompBlock_DXT3(unsigned char* pDecomp, const sDXTC_ClrBlk* pBlock)
{
	sClr32 aClr[4];
	this->GetColorFromBlock(aClr, pBlock + 1);
	this->DecodeColor(reinterpret_cast<sClr32*>(pDecomp), pBlock + 1, aClr);
	this->DecodeExpAlphaBlock(reinterpret_cast<sClr32*>(pDecomp),
		reinterpret_cast<const sDXTC_ExpAlphaBlk*>(pBlock));
}

void WDXTC::DecompBlock_DXT5(unsigned char* pDecomp, const sDXTC_ClrBlk* pBlock)
{
	sClr32 aClr[4];
	this->GetColorFromBlock(aClr, pBlock + 1);
	this->DecodeColor(reinterpret_cast<sClr32*>(pDecomp), pBlock + 1, aClr);
	this->DecodeLinearAlphaBlock3Bit(reinterpret_cast<sClr32*>(pDecomp),
		reinterpret_cast<const sDXTC_LinearAlphaBlk3Bit*>(pBlock));
}

void WDXTC::Decomp_DXT1()
{
	int nXBlocks = m_DDSD2.dwWidth >> 2;
	int nYBlocks = m_DDSD2.dwHeight >> 2;
	for (int y = 0; y < nYBlocks; ++y)
	{
		const sDXTC_ClrBlk* pIn = reinterpret_cast<const sDXTC_ClrBlk*>(
			m_pCompData + 8 * nXBlocks * y);
		unsigned char* pOut = m_pDecompData + 16 * y * m_DDSD2.dwWidth;
		for (int x = 0; x < nXBlocks; ++x)
		{
			DecompBlock_DXT1(pOut, pIn);
			pIn += 1;
			pOut += 16;
		}
	}
}

void WDXTC::Decomp_DXT3()
{
	int nXBlocks = m_DDSD2.dwWidth >> 2;
	int nYBlocks = m_DDSD2.dwHeight >> 2;
	for (int y = 0; y < nYBlocks; ++y)
	{
		const sDXTC_ClrBlk* pIn = reinterpret_cast<const sDXTC_ClrBlk*>(
			m_pCompData + 16 * nXBlocks * y);
		unsigned char* pOut = m_pDecompData + 16 * y * m_DDSD2.dwWidth;
		for (int x = 0; x < nXBlocks; ++x)
		{
			DecompBlock_DXT3(pOut, pIn);
			pIn += 2;
			pOut += 16;
		}
	}
}

void WDXTC::Decomp_DXT5()
{
	int nXBlocks = m_DDSD2.dwWidth >> 2;
	int nYBlocks = m_DDSD2.dwHeight >> 2;
	for (int y = 0; y < nYBlocks; ++y)
	{
		const sDXTC_ClrBlk* pIn = reinterpret_cast<const sDXTC_ClrBlk*>(
			m_pCompData + 16 * nXBlocks * y);
		unsigned char* pOut = m_pDecompData + 16 * y * m_DDSD2.dwWidth;
		for (int x = 0; x < nXBlocks; ++x)
		{
			DecompBlock_DXT5(pOut, pIn);
			pIn += 2;
			pOut += 16;
		}
	}
}

bool WDXTC::DecompDXTC()
{
	if (this->m_pDecompData)
	{
		delete[] this->m_pDecompData;
		this->m_pDecompData = 0;
	}
	this->m_pDecompData =
		new BYTE[4 * this->m_DDSD2.dwHeight * this->m_DDSD2.dwWidth];
	switch (this->m_DDSD2.ddpfPixelFormat.dwFourCC)
	{
	case g_dxt1FourCc:
		this->Decomp_DXT1();
		return true;
	case g_dxt3FourCc:
		this->Decomp_DXT3();
		return true;
	case g_dxt5FourCc:
		this->Decomp_DXT5();
		return true;
	default:
		return false;
	}
}

bool WDXTC::Load(const unsigned char* pData, int iSize)
{
	if (pData[0] != 'D' || pData[1] != 'D' || pData[2] != 'S')
		return false;
	pData += 4;
	memcpy(&m_DDSD2, pData, sizeof(m_DDSD2));
	if (m_DDSD2.dwFlags & DDSD_LINEARSIZE)
	{
		m_pCompData = new unsigned char[m_DDSD2.dwLinearSize];
		memcpy(m_pCompData, pData + sizeof(m_DDSD2), m_DDSD2.dwLinearSize);
		return DecompDXTC();
	}
	return false;
}
