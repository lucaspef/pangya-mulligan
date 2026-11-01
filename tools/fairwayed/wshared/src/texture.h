// SPDX-License-Identifier: MIT
// texture.h - image loading for the viewport.
#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

void TextureStartup();
void TextureShutdown();

#include <vector>

struct TexImage
{
	int w, h;
	std::vector<unsigned char> rgba;
};

// One *ptex image: the rectangle it replaces, RGBA, row by row.
struct TexPart
{
	int left, top, width, height;
	bool hasAlpha;
	std::vector<unsigned char> rgba;
};

typedef std::string (*AssetResolver)(const std::string& cp949Name, void* context);

bool LoadTexImage(const std::string& utf8Path, TexImage* out);	// with its _mask, if any
void UploadTexImage(unsigned int texture, const TexImage& image);

// WResourceManager::LoadBitmapPart and GetRectInSourceTexture. why says what failed.
bool LoadTexPart(const std::string& dstName, const std::string& srcName, int dstW, int dstH,
	AssetResolver resolve, void* context, TexPart* out, std::string* why);

// Creates an OpenGL texture from a .jpg .png .bmp .tga or .dds. 0 on failure.
unsigned int LoadGLTexture(const std::string& utf8Path, int* width = 0, int* height = 0);

// The four colours of a DXT block (RGBA each); dxt1 allows its transparent mode.
void DecodeDxtColorBlock(const unsigned char* block, unsigned char colors[4][4], bool dxt1);

#endif // TEXTURE_H
