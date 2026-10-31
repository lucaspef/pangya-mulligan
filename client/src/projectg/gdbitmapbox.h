#pragma once

#include <map>
#include "gamedatadb.h"

struct sGD_BITMAP
{
	sGD_BITMAP() { memset(fileName, 0, sizeof(fileName)); }

	char fileName[64];
	WRect rect;
};

class CGDBitmapBox : public IGameData
{
public:
	DECLARE_GAMEDATA(CGDBitmapBox, GAMEDATA_BITMAPBOX, "bitmapbox")

	CGDBitmapBox();
	virtual ~CGDBitmapBox();

	virtual void Initialize(TiXmlNode* pNode);
	const char* GetFileName(char* key);
	void GetRect(const char* key, _WRECT& rect);

private:
	std::map<std::string, sGD_BITMAP> m_bitmapMap;
};
