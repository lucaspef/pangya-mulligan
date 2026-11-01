// SPDX-License-Identifier: MIT
// texture.cpp - GDI+ decodes the JPG/PNG/BMP files; TGA and DDS are read by hand.
#include "texture.h"
#include <windows.h>
#include <objidl.h>
#include <algorithm>
#include <gdiplus.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include "wcommon.h"

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

typedef unsigned char u8;

static ULONG_PTR g_gdiplusToken;

void TextureStartup()
{
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup(&g_gdiplusToken, &input, 0);
}

void TextureShutdown()
{
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
}

typedef TexImage Image;

static bool LoadGdiplus(const std::string& path, Image* img)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, 0, 0);
	std::wstring w(n, L' ');
	MultiByteToWideChar(CP_UTF8, 0, path.c_str(), -1, &w[0], n);
	Gdiplus::Bitmap bmp(w.c_str());
	if (bmp.GetLastStatus() != Gdiplus::Ok)
		return false;
	img->w = bmp.GetWidth();
	img->h = bmp.GetHeight();
	if (img->w <= 0 || img->h <= 0)
		return false;
	Gdiplus::Rect rect(0, 0, img->w, img->h);
	Gdiplus::BitmapData data;
	if (bmp.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
		return false;
	img->rgba.resize(img->w * img->h * 4);
	for (int y = 0; y < img->h; ++y)
	{
		const u8* src = (const u8*)data.Scan0 + y * data.Stride;
		u8* dst = &img->rgba[y * img->w * 4];
		for (int x = 0; x < img->w; ++x)
		{
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += 4;
		}
	}
	bmp.UnlockBits(&data);
	return true;
}

static bool LoadTga(const std::string& path, Image* img)
{
	std::vector<u8> d;
	if (!ReadWholeFile(path, &d) || d.size() < 18)
		return false;
	int idLen = d[0], type = d[2], bpp = d[16], desc = d[17];
	int w = d[12] | (d[13] << 8), h = d[14] | (d[15] << 8);
	if ((type != 2 && type != 10) || (bpp != 24 && bpp != 32) || w <= 0 || h <= 0)
		return false;
	size_t pos = 18 + idLen;
	int bytes = bpp / 8;
	img->w = w;
	img->h = h;
	img->rgba.resize(w * h * 4);
	int count = w * h;
	int written = 0;
	u8 px[4] = { 0, 0, 0, 255 };
	while (written < count && pos < d.size())
	{
		int run = 1;
		bool literal = true;
		if (type == 10)
		{
			u8 head = d[pos++];
			run = (head & 0x7f) + 1;
			literal = !(head & 0x80);
			if (!literal)
			{
				if (pos + bytes > d.size())
					break;
				memcpy(px, &d[pos], bytes);
				pos += bytes;
			}
		}
		for (int i = 0; i < run && written < count; ++i, ++written)
		{
			if (literal)
			{
				if (pos + bytes > d.size())
					return false;
				memcpy(px, &d[pos], bytes);
				pos += bytes;
			}
			int row = written / w;
			int col = written % w;
			if (!(desc & 0x20))
				row = h - 1 - row;
			u8* dst = &img->rgba[(row * w + col) * 4];
			dst[0] = px[2];
			dst[1] = px[1];
			dst[2] = px[0];
			dst[3] = bytes == 4 ? px[3] : 255;
		}
	}
	return written == count;
}

static void Rgb565(unsigned short c, u8* out)
{
	out[0] = (u8)(((c >> 11) & 31) * 255 / 31);
	out[1] = (u8)(((c >> 5) & 63) * 255 / 63);
	out[2] = (u8)((c & 31) * 255 / 31);
	out[3] = 255;
}

void DecodeDxtColorBlock(const unsigned char* b, unsigned char colors[4][4], bool dxt1)
{
	unsigned short c0 = (unsigned short)(b[0] | (b[1] << 8));
	unsigned short c1 = (unsigned short)(b[2] | (b[3] << 8));
	Rgb565(c0, colors[0]);
	Rgb565(c1, colors[1]);
	if (!dxt1 || c0 > c1)
	{
		for (int i = 0; i < 3; ++i)
		{
			colors[2][i] = (u8)((2 * colors[0][i] + colors[1][i]) / 3);
			colors[3][i] = (u8)((colors[0][i] + 2 * colors[1][i]) / 3);
		}
		colors[2][3] = colors[3][3] = 255;
	}
	else
	{
		for (int i = 0; i < 3; ++i)
		{
			colors[2][i] = (u8)((colors[0][i] + colors[1][i]) / 2);
			colors[3][i] = 0;
		}
		colors[2][3] = 255;
		colors[3][3] = 0;
	}
}

static bool LoadDds(const std::string& path, Image* img)
{
	std::vector<u8> d;
	if (!ReadWholeFile(path, &d) || d.size() < 128 || memcmp(&d[0], "DDS ", 4))
		return false;
	int h, w;
	memcpy(&h, &d[12], 4);
	memcpy(&w, &d[16], 4);
	unsigned int pfFlags, bits, rMask, aMask;
	memcpy(&pfFlags, &d[80], 4);
	memcpy(&bits, &d[88], 4);
	memcpy(&rMask, &d[92], 4);
	memcpy(&aMask, &d[104], 4);
	if (w <= 0 || h <= 0)
		return false;
	img->w = w;
	img->h = h;
	img->rgba.assign(w * h * 4, 255);
	const u8* src = &d[128];
	const u8* end = &d[0] + d.size();

	if (pfFlags & 4)	// FOURCC
	{
		int dxt = 0;
		if (!memcmp(&d[84], "DXT1", 4))
			dxt = 1;
		else if (!memcmp(&d[84], "DXT3", 4))
			dxt = 3;
		else if (!memcmp(&d[84], "DXT5", 4))
			dxt = 5;
		if (!dxt)
			return false;
		int blockSize = dxt == 1 ? 8 : 16;
		for (int by = 0; by < (h + 3) / 4; ++by)
		{
			for (int bx = 0; bx < (w + 3) / 4; ++bx)
			{
				if (src + blockSize > end)
					return true;
				const u8* block = src;
				src += blockSize;
				u8 alpha[16];
				memset(alpha, 255, sizeof(alpha));
				if (dxt == 3)
				{
					for (int i = 0; i < 16; ++i)
					{
						int v = (block[i / 2] >> ((i & 1) * 4)) & 15;
						alpha[i] = (u8)(v * 17);
					}
				}
				else if (dxt == 5)
				{
					u8 a[8];
					a[0] = block[0];
					a[1] = block[1];
					if (a[0] > a[1])
						for (int i = 0; i < 6; ++i)
							a[2 + i] = (u8)(((6 - i) * a[0] + (1 + i) * a[1]) / 7);
					else
					{
						for (int i = 0; i < 4; ++i)
							a[2 + i] = (u8)(((4 - i) * a[0] + (1 + i) * a[1]) / 5);
						a[6] = 0;
						a[7] = 255;
					}
					unsigned long long bitsA = 0;
					for (int i = 0; i < 6; ++i)
						bitsA |= (unsigned long long)block[2 + i] << (8 * i);
					for (int i = 0; i < 16; ++i)
						alpha[i] = a[(bitsA >> (3 * i)) & 7];
				}
				const u8* cb = dxt == 1 ? block : block + 8;
				u8 colors[4][4];
				DecodeDxtColorBlock(cb, colors, dxt == 1);
				unsigned int index = cb[4] | (cb[5] << 8) | (cb[6] << 16) | ((unsigned int)cb[7] << 24);
				for (int y = 0; y < 4; ++y)
				{
					for (int x = 0; x < 4; ++x)
					{
						int px = bx * 4 + x;
						int py = by * 4 + y;
						if (px >= w || py >= h)
							continue;
						int i = y * 4 + x;
						const u8* c = colors[(index >> (2 * i)) & 3];
						u8* dst = &img->rgba[(py * w + px) * 4];
						dst[0] = c[0];
						dst[1] = c[1];
						dst[2] = c[2];
						dst[3] = dxt == 1 ? c[3] : alpha[i];
					}
				}
			}
		}
		return true;
	}

	if (bits == 32 || bits == 24)
	{
		int bytes = bits / 8;
		bool rgbOrder = rMask == 0x000000ff;
		for (int i = 0; i < w * h; ++i)
		{
			if (src + bytes > end)
				break;
			u8* dst = &img->rgba[i * 4];
			dst[0] = rgbOrder ? src[0] : src[2];
			dst[1] = src[1];
			dst[2] = rgbOrder ? src[2] : src[0];
			dst[3] = bytes == 4 && (pfFlags & 1) ? src[3] : 255;
			src += bytes;
		}
		return true;
	}
	return false;
}

static bool LoadRaw(const std::string& path, Image* img)
{
	size_t dot = path.rfind('.');
	std::string ext = dot == std::string::npos ? std::string() : path.substr(dot);
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
	if (ext == ".dds")
		return LoadDds(path, img);
	if (ext == ".tga")
		return LoadTga(path, img);
	return LoadGdiplus(path, img);
}

bool LoadTexImage(const std::string& path, TexImage* out)
{
	Image& img = *out;
	if (!LoadRaw(path, &img))
		return false;
	size_t dot = path.rfind('.');

	// WResourceManager::CheckMaskTexture: '[' and '+' textures take their alpha from a
	// grey "<name>_mask<ext>" beside them. A mask of another size is ignored.
	size_t slash = path.find_last_of("/\\");
	std::string base = slash == std::string::npos ? path : path.substr(slash + 1);
	if (dot != std::string::npos && (base.find('[') != std::string::npos || base.find('+') != std::string::npos))
	{
		std::string maskPath = path.substr(0, dot) + "_mask" + path.substr(dot);
		Image mask;
		if (LoadRaw(maskPath, &mask) && mask.w == img.w && mask.h == img.h)
			for (int i = 0; i < img.w * img.h; ++i)
				img.rgba[i * 4 + 3] = mask.rgba[i * 4];
	}
	return true;
}

void UploadTexImage(unsigned int texture, const TexImage& img)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, &img.rgba[0]);
}

static bool IsGreen(const u8* p)
{
	return p[0] == 0 && p[1] == 0xff && p[2] == 0;
}

static bool HasMaskFlag(const std::string& name)
{
	return name.find('[') != std::string::npos || name.find('+') != std::string::npos;
}

// "a.png" -> "a_mask.png"
static std::string MaskName(const std::string& name)
{
	size_t dot = name.rfind('.');
	return dot == std::string::npos ? name : name.substr(0, dot) + "_mask" + name.substr(dot);
}

bool LoadTexPart(const std::string& dstName, const std::string& srcName, int dstW, int dstH,
	AssetResolver resolve, void* context, TexPart* out, std::string* why)
{
	std::string path = resolve(srcName, context);
	Image src;
	if (path.empty() || !LoadRaw(path, &src))
	{
		*why = "image not found";
		return false;
	}
	if (src.w != dstW || src.h != dstH)
	{
		*why = "image and texture differ in size";
		return false;
	}
	out->hasAlpha = false;
	if (dstName[0] == '[' || dstName[0] == '+')
	{
		// CheckMaskTexture tries the part's own mask, then the texture's; with neither
		// LoadBitmapPart gives up on the part.
		Image mask;
		bool ok = false;
		for (int i = 0; i < 2 && !ok; ++i)
		{
			const std::string& name = i == 0 ? srcName : dstName;
			if (!HasMaskFlag(name))
				continue;
			std::string maskPath = resolve(MaskName(name), context);
			ok = !maskPath.empty() && LoadRaw(maskPath, &mask) && mask.w == src.w && mask.h == src.h;
		}
		if (!ok)
		{
			*why = "no mask for a masked texture";
			return false;
		}
		for (int i = 0; i < src.w * src.h; ++i)
			src.rgba[i * 4 + 3] = mask.rgba[i * 4];
		out->hasAlpha = true;
	}

	// GetRectInSourceTexture: pure green is "unchanged". The first other pixel is the
	// corner, and the runs right and down from it give the size.
	int x = 0, y = 0;
	bool found = false;
	for (y = 0; y < src.h && !found; ++y)
		for (x = 0; x < src.w; ++x)
			if (!IsGreen(&src.rgba[(y * src.w + x) * 4]))
			{
				found = true;
				break;
			}
	if (!found)
	{
		*why = "image is entirely green";
		return false;
	}
	--y;
	int right = x + 1, bottom = y + 1;
	while (right < src.w && !IsGreen(&src.rgba[(y * src.w + right) * 4]))
		++right;
	while (bottom < src.h && !IsGreen(&src.rgba[(bottom * src.w + x) * 4]))
		++bottom;
	out->left = x;
	out->top = y;
	out->width = right - x;
	out->height = bottom - y;
	out->rgba.resize(out->width * out->height * 4);
	for (int r = 0; r < out->height; ++r)
		memcpy(&out->rgba[r * out->width * 4], &src.rgba[((y + r) * src.w + x) * 4], out->width * 4);
	return true;
}

unsigned int LoadGLTexture(const std::string& path, int* width, int* height)
{
	Image img;
	if (!LoadTexImage(path, &img))
		return 0;
	if (width)
		*width = img.w;
	if (height)
		*height = img.h;
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.w, img.h, GL_RGBA, GL_UNSIGNED_BYTE, &img.rgba[0]);
	return tex;
}
