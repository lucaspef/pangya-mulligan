#pragma once

class Bitmap;

struct sTexturePart
{
	sTexturePart()
	{
		memset(name, 0, sizeof(name));
		memset(texture, 0, sizeof(texture));
		memset(&rect, 0, sizeof(rect));
		pBitmap = NULL;
	}
	~sTexturePart()
	{
		if (pBitmap)
		{
			delete pBitmap;
			pBitmap = NULL;
		}
	}

	char name[0x40];
	char texture[0x40];
	Bitmap* pBitmap;
	tagRECT rect;
};

struct sFindTexPart
{
	sFindTexPart(const char* _texture)
		: texture(_texture)
	{
	}
	bool operator()(const sTexturePart* pPart) const
	{
		return strcmpi(pPart->texture, texture) == 0;
	}

	const char* texture;
};
