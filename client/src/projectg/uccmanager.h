#pragma once

#include <string.h>
#include <string>
#include <list>
#include <map>

#include "../shared/classdefine.h"

struct IFF_ITEM_COMMON;

// HACK: completes map<std::string, Bitmap*> here, before the list members of
// CUccManager, so its template members are emitted in the original order
typedef char UccIconMapSize[sizeof(std::map<std::string, Bitmap*>)];

struct sUccItem
{
	unsigned long id;
	char uccIndex[9];
	unsigned long typeId;
	char uccName[41];
	unsigned long unused40;
	unsigned long unused44;
	unsigned long unused48;
	_SYSTEMTIME date;
	Bitmap* icon;
	char downloadCount;

	sUccItem()
	{
		typeId = 0;
		unused40 = 0;
		unused44 = 0;
		unused48 = 0;
		downloadCount = 0;
		icon = NULL;
		uccIndex[0] = 0;
		uccName[0] = 0;
	}
};
struct sUccClothes : public sUccItem
{
	sUccClothes() { }
};

struct sUccAztec : public sUccItem
{
};

struct sUccRefreshItem
{
	unsigned long typeId;
	char uccIndex[9];

#ifdef REBANG_GOLFPLAYER_CPP
	// golfplayer.cpp defines it out of line (emission order); the IL of the other includers is unchanged
	sUccRefreshItem();
#else
	sUccRefreshItem()
	{
		typeId = 0;
		memset(uccIndex, 0, sizeof(uccIndex));
	}
#endif
};

struct sUccTempClothes
{
	unsigned long typeId;
	char uccIndex[9];
	Bitmap* front;
	Bitmap* back;

	sUccTempClothes()
	{
		typeId = 0;
		memset(uccIndex, 0, sizeof(uccIndex));
		front = new Bitmap;
		back = new Bitmap;
	}
};

struct sDisplayUccClothes
{
	IFF_STRUCT::sPart part;
	unsigned long partTypeId;
	unsigned long itemId;
	std::string name;
	int rank;

	sDisplayUccClothes()
	{
		partTypeId = 0;
		itemId = 0;
		rank = 0;

		part.Category = 2;
		part.PosMask = 0;
		part.HideMask = 0;
		part.CharacterSlot = 0;
		part.CaddieSlot = 0;
		memset(part.Attr, 0, sizeof(part.Attr));
		memset(part.Slot, 0, sizeof(part.Slot));

		memset(&part, 0, sizeof(IFF_ITEM_COMMON));
		strcpy(part.c.Icon, "UCCDISPLAY");
		part.c.Final = true;
		part.c.TypeId = 0;
		part.c.IsCash = 1;
		part.c.InStock = 4;
		part.c.Price = 1000000;
		part.c.New = 0;
		part.c.Hit = 0;
	}
};

class CUccManager
{
public:
	CUccManager();
	~CUccManager();
	void Clear();
	void ClearTexture();
	void ClearIcon();
	void ClearTempClothes();
	void AddClothes(unsigned char slot, unsigned long id, unsigned long typeId,
		const char* uccIndex, bool bSend, bool bForce);
	void AddClothes(unsigned char slot, sUccClothes& clothes, bool bSend,
		bool bForce);
	void AddAztec(sUccAztec& aztec, bool bForce);
	sUccClothes* FindClothes(unsigned long id);
	sUccClothes* FindClothes(unsigned long typeId, const char* uccIndex);
	sUccAztec* FindAztec(unsigned long id);
	sUccAztec* FindAztec(unsigned long typeId, const char* uccIndex);
	void ClearClothes() { m_clothes.clear(); }
	void ClearAztec() { m_aztec.clear(); }
	bool IsDownloadComplete(unsigned long typeId, const char* uccIndex);
	bool IsDownloadComplete(sUccClothes* clothes);
	bool IsDownloadComplete(sUccAztec* aztec);
	bool SetDownloadComplete(unsigned long typeId, const char* uccIndex);
	bool SetDownloadComplete(sUccClothes* clothes);
	bool SetDownloadComplete(sUccAztec* aztec);
	void AddRefreshItem(unsigned long typeId, const char* uccIndex);
	bool GetRefreshItem(sUccRefreshItem& item);
	void SetUccTempClothes(unsigned long typeId, const char* uccIndex,
		Bitmap* front, Bitmap* back);
	bool GetUccTempClothes(unsigned long typeId, const char* uccIndex,
		Bitmap& front, Bitmap& back);
	void AddDisplayClothes(unsigned long partTypeId, unsigned long itemId,
		std::string name, int rank);
	void BuildDisplayClothesSet();
	bool IsDisplayUccClothes(const IFF_ITEM_COMMON* item);
	unsigned long GetWinnerCharacterTypeId();

private:
	void RequestClothes(unsigned long typeId, const char* uccIndex,
		bool bForce);
	void RequestAztec(unsigned long typeId, const char* uccIndex, bool bForce);
	sUccClothes* _GetClothes(unsigned long id);
	sUccAztec* _GetAztec(unsigned long id);

public:
	std::list<sUccClothes> m_clothes;
	std::list<sUccAztec> m_aztec;
	std::map<std::string, Bitmap*> m_icon;
	std::list<sUccTempClothes> m_tempClothes;
	std::list<sDisplayUccClothes> m_displayClothes;
	std::list<sUccRefreshItem> m_refreshItem;
	_RTL_CRITICAL_SECTION m_csRefresh;
};

CUccManager* UccManager();

struct sItemInfo;

sItemInfo* GetMyItemInfo(unsigned long guid);
std::string GenerateTextureName_F(unsigned long typeId, const char* uccIndex);
std::string GenerateTextureName_B(unsigned long typeId, const char* uccIndex);
