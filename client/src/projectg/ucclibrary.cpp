#include "minatl.h"
#include "ucclibrary.h"
#include <algorithm>
#include <math.h>
#include <ctype.h>
#include "commonutil.h"
#include "fresh.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "shareddoc.h"
#include "uccmanager.h"
#include "wresrcmng.h"
#include "cfile.h"
#include "xunzip.h"
#include "xzip.h"




extern Fresh* g_pFresh;

void MixColor(sRGBA& base, sRGBA& overlay, sRGBA& result);
void SetBrightness(sRGBA& color, sRGBA& mask, sRGBA& result);
std::string GenerateTextureName_F(unsigned long typeId, const char* uccIndex);
std::string GenerateTextureName_B(unsigned long typeId, const char* uccIndex);
const char* GetUccTempFoler();
bool NoDrawArea(const Bitmap* mask, int left, int top, int right, int bottom);
unsigned char HueToRGB(unsigned char a, unsigned char b, unsigned char hue);
void GetUccInfoFromPacket(WReceivedPacket* packet);
bool ZipUccClothes(Bitmap* front, Bitmap* back, Bitmap* icon,
	const char* filename);
bool UnzipUccClothes(unsigned char* data, unsigned long len, Bitmap* front,
	Bitmap* back, Bitmap* icon, sUccHeader* header);
bool LoadWarningClothes(unsigned int id, Bitmap* front, Bitmap* back,
	Bitmap* icon);
float Power(float base, float exponent);
void Convert32to24(Bitmap* src, Bitmap* dst, const Bitmap* mask);
bool IsSameClothes(unsigned long first, unsigned long second);
const Bitmap* GetUccItemIcon(unsigned long typeId, const char* uccIndex);
const char* GetUccItemName(unsigned long typeId, const char* uccIndex);
sItemInfo* GetMyItemInfo(unsigned long guid);
std::string Lower(std::string str);
std::string FilterFilename(std::string name);
std::string GetMaskFilename(const char* filename);
std::string GenerateTextureFullPath(int side, unsigned long typeId,
	const char* uccIndex);
std::string GenerateIconFilename(unsigned long typeId, const char* uccIndex);
sRGBA GetAverageColor(const Bitmap* bitmap, int left, int top, int right,
	int bottom, bool bUseAlpha);
sRGBA GetSimilarColor(sRGBA base, sRGBA first, sRGBA second);
void GetResizedImage(const Bitmap* src, const Bitmap* mask, Bitmap* dst,
	int width, int height);
void RequestUccInfo(sCharacterInfo& info);
void RequestUccInfo(sMailIncludeItem& item);
void RequestUccInfo(sUserInfoTime& info);
void RequestUccInfo(sUserInfo& info);
void RequestUccInfo(sRoomSlot& slot);
void RequestUccInfo(ranktype::sRankUserInfo& info);

namespace ranktype
{
#pragma pack(push, 1)
	struct sRankUserInfo
	{
		unsigned char pad0[0x32]; // visao: so charInfo e lido aqui
		sCharacterInfo charInfo;
	};
#pragma pack(pop)
} // namespace ranktype

const char* GetUccTempFoler()
{
	return "capture";
}

bool IsUccClothes(unsigned long typeId)
{
	if (Doc() != NULL && ItemManager() != NULL &&
		(typeId & 0xfc000000) == 0x8000000)
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
		if (part != NULL &&
			(part->Category == 7 || part->Category == 8 || part->Category == 9))
			return true;
	}
	return false;
}

bool IsSameClothes(IFF_STRUCT::sPart* first, IFF_STRUCT::sPart* second)
{
	if (first == NULL || second == NULL)
		return false;

	bool bSame = strcmpi(first->Data, second->Data) == 0;
	for (int i = 0; i < 3; i++)
	{
		if (bSame && strcmpi(first->Tex[i], second->Tex[i]) == 0 &&
			strcmpi(first->OrgTex[i], second->OrgTex[i]) == 0)
			bSame = true;
		else
			bSame = false;
	}
	return bSame;
}

bool IsSameClothes(unsigned long first, unsigned long second)
{
	if (first == second)
		return true;
	if (Doc() != NULL && ItemManager() != NULL)
	{
		IFF_STRUCT::sPart* firstPart = ItemManager()->FindPart(first);
		IFF_STRUCT::sPart* secondPart = ItemManager()->FindPart(second);
		return IsSameClothes(firstPart, secondPart);
	}
	return false;
}

void CreateItemThumbnail(const Bitmap* texture, const Bitmap* mask,
	Bitmap* thumbnail)
{
	if (texture != NULL && thumbnail != NULL)
	{
		Bitmap resized;
		float offsetX;
		float offsetY;
		float w;
		float h;
		unsigned int size = Max(texture->Width(), texture->Height());
		float scale = 64.0f / (float)size;
		w = (float)texture->Width() * scale;
		h = (float)texture->Height() * scale;
		offsetX = (64.0f - w) * 0.5f;
		offsetY = (84.0f - h) * 0.5f;
		int y = (int)w;
		GetResizedImage(texture, mask, &resized, y, (int)h);
		thumbnail->Create(64, 84, 32);
		for (y = 0; y < 84; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
				if ((float)x < offsetX || (float)x >= offsetX + w ||
					(float)y < offsetY || (float)y >= offsetY + h)
				{
					r = 0xff;
					g = 0xff;
					b = 0xff;
					a = 0;
				}
				else
				{
					int ox = (int)offsetX;
					resized.GetPixel(x - ox, y - (int)offsetY, r, g, b, a);
				}
				thumbnail->SetPixel(x, y, r, g, b, a);
			}
		}
	}
}

const Bitmap* GetUccItemIcon(sItemInfo* item)
{
	if (item == NULL)
		return NULL;

	if (item->status & 1)
	{
		sUccClothes* clothes =
			UccManager()->FindClothes(item->tid, item->UccIndex);
		if (clothes != NULL && UccManager()->IsDownloadComplete(clothes))
			return clothes->icon;
	}
	if (Doc() != NULL && ItemManager() != NULL)
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->tid);
		if (part != NULL)
			return g_resrcmng->LoadBitmap(part->c.Icon, 0, false);
	}
	return NULL;
}

const char* GetUccItemName(sItemInfo* item)
{
	if (Doc() != NULL && ItemManager() != NULL)
	{
		if (!(item->status & 1))
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->tid);
			if (part != NULL)
				return part->c.Name;
		}
		else
		{
			sUccClothes* clothes =
				UccManager()->FindClothes(item->tid, item->UccIndex);
			if (clothes == NULL)
				return "";
			if (UccManager()->IsDownloadComplete(clothes))
				return clothes->uccName;
			if (!NetResourceManager::Instance()->IsDownloadRequested(
					RESOURCE_UCC_CLOTHES, item->tid, item->UccIndex))
				return MakeStr(K2L_Compatibility( // 983 007e9728/007ea711/007ea8d3
					"%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xbd\xc7\xc6\xd0)"),
					clothes->uccName);
			return MakeStr(K2L_Compatibility("%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xc1\xdf)"),
				clothes->uccName);
		}
	}
	return "";
}

sItemInfo* GetMyItemInfo(unsigned long guid)
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_uccItemList.begin(); it != Doc()->m_uccItemList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_uccAztecList.begin(); it != Doc()->m_uccAztecList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_partsList.begin(); it != Doc()->m_partsList.end(); it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end(); it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_cadItemList.begin(); it != Doc()->m_cadItemList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_setItemList.begin(); it != Doc()->m_setItemList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_cutinList.begin(); it != Doc()->m_cutinList.end(); it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_auxPartList.begin(); it != Doc()->m_auxPartList.end();
		it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	for (it = Doc()->m_questDropList.begin();
		it != Doc()->m_questDropList.end(); it++)
	{
		if (it->guid == guid)
			return &*it;
	}
	return NULL;
}

sItemInfo* GetMyItemInfo(unsigned long typeId, const char* uccIndex,
	unsigned short seq)
{
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_uccItemList.begin(); it != Doc()->m_uccItemList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_uccAztecList.begin(); it != Doc()->m_uccAztecList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_partsList.begin(); it != Doc()->m_partsList.end(); it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_ballList.begin(); it != Doc()->m_ballList.end(); it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_cadItemList.begin(); it != Doc()->m_cadItemList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_setItemList.begin(); it != Doc()->m_setItemList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_cutinList.begin(); it != Doc()->m_cutinList.end(); it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_auxPartList.begin(); it != Doc()->m_auxPartList.end();
		it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	for (it = Doc()->m_questDropList.begin();
		it != Doc()->m_questDropList.end(); it++)
	{
		if (strlen(it->UccIndex) > 0 && it->Seq != 0 && it->tid == typeId &&
			strcmp(it->UccIndex, uccIndex) == 0 && (seq == 0 || it->Seq == seq))
			return &*it;
	}
	return NULL;
}

const Bitmap* GetUccItemIcon(unsigned long guid)
{
	if (guid != 0xffffffff)
	{
		sItemInfo* item = GetMyItemInfo(guid);
		if (item != NULL && !(item->status & 1) && ItemManager() != NULL)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->tid);
			if (part != NULL)
				return g_pFresh->GetBitmap(part->c.Icon);
		}
		sUccClothes* clothes = UccManager()->FindClothes(guid);
		if (clothes != NULL && UccManager()->IsDownloadComplete(clothes))
			return clothes->icon;
	}
	return NULL;
}

const char* GetUccItemName(unsigned long guid)
{
	if (guid != 0xffffffff)
	{
		sItemInfo* item = GetMyItemInfo(guid);
		if (item != NULL && !(item->status & 1) && ItemManager() != NULL)
		{
			IFF_STRUCT::sPart* part = ItemManager()->FindPart(item->tid);
			if (part != NULL)
				return part->c.Name;
		}
		sUccClothes* clothes = UccManager()->FindClothes(guid);
		if (clothes != NULL)
		{
			if (UccManager()->IsDownloadComplete(clothes))
				return clothes->uccName;
			if (!NetResourceManager::Instance()->IsDownloadRequested(
					RESOURCE_UCC_CLOTHES, clothes->typeId, clothes->uccIndex))
				return MakeStr(K2L_Compatibility( // 983 007e9728/007ea711/007ea8d3
					"%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xbd\xc7\xc6\xd0)"),
					clothes->uccName);
			return MakeStr(K2L_Compatibility("%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xc1\xdf)"),
				clothes->uccName);
		}
	}
	return "";
}

const Bitmap* GetUccItemIcon(unsigned long typeId, const char* uccIndex)
{
	if (typeId == 0 || uccIndex == NULL)
		return NULL;
	if (strlen(uccIndex) > 0)
	{
		IFF_STRUCT::sPart* part = NULL;
		sItemInfo* item = GetMyItemInfo(typeId, uccIndex, 0);
		if (item == NULL || (item->status & 1) || ItemManager() == NULL ||
			(part = ItemManager()->FindPart(item->tid)) == NULL)
		{
			sUccClothes* clothes = UccManager()->FindClothes(typeId, uccIndex);
			if (clothes != NULL && UccManager()->IsDownloadComplete(clothes))
				return clothes->icon;
			if (ItemManager() == NULL)
				return NULL;
			part = ItemManager()->FindPart(typeId);
			if (part == NULL)
				return NULL;
		}
		return g_pFresh->GetBitmap(part->c.Icon);
	}
	return NULL;
}

const char* GetUccItemName(unsigned long typeId, const char* uccIndex)
{
	if (typeId != 0 && uccIndex != NULL)
	{
		if (strlen(uccIndex) == 0)
			return "";
		IFF_STRUCT::sPart* part = NULL;
		sItemInfo* item = GetMyItemInfo(typeId, uccIndex, 0);
		if (item == NULL || (item->status & 1) || ItemManager() == NULL ||
			(part = ItemManager()->FindPart(item->tid)) == NULL)
		{
			sUccClothes* clothes = UccManager()->FindClothes(typeId, uccIndex);
			if (clothes != NULL)
			{
				if (UccManager()->IsDownloadComplete(clothes))
					return clothes->uccName;
				if (!NetResourceManager::Instance()->IsDownloadRequested(
						RESOURCE_UCC_CLOTHES, clothes->typeId,
						clothes->uccIndex))
					return MakeStr(
						K2L_Compatibility("%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xbd\xc7\xc6\xd0)"),
						clothes->uccName);
				return MakeStr(K2L_Compatibility("%s (\xb4\xd9\xbf\xee\xb7\xce\xb5\xe5 \xc1\xdf)"),
					clothes->uccName);
			}
			if (ItemManager() == NULL ||
				(part = ItemManager()->FindPart(typeId)) == NULL)
				return "";
		}
		return part->c.Name;
	}
	return "";
}

bool ZipUccClothes(Bitmap* front, Bitmap* back, Bitmap* icon,
	const char* filename)
{
	unsigned int frontSize;
	if (front != NULL)
		frontSize =
			front->BitsPerPixel() * front->Height() * front->Width() >> 3;
	else
		frontSize = 0;
	unsigned int backSize;
	if (back != NULL)
		backSize = back->BitsPerPixel() * back->Height() * back->Width() >> 3;
	else
		backSize = 0;
	unsigned int iconSize;
	if (icon != NULL)
		iconSize = icon->BitsPerPixel() * icon->Height() * icon->Width() >> 3;
	else
		iconSize = 0;

	HZIP zip = CreateZipZ((void*)filename, 0, ZIP_FILENAME, ZIP_FILENAME);
	bool bResult = true;
	if (front != NULL)
	{
		bResult =
			ZipAdd(zip, "front", front->vram, frontSize, ZIP_MEMORY) == ZR_OK;
	}
	if (back != NULL)
	{
		if (bResult &&
			ZipAdd(zip, "back", back->vram, backSize, ZIP_MEMORY) == ZR_OK)
			bResult = true;
		else
			bResult = false;
	}
	if (icon != NULL)
	{
		if (bResult &&
			ZipAdd(zip, "icon", icon->vram, iconSize, ZIP_MEMORY) == ZR_OK)
			bResult = true;
		else
			bResult = false;
	}
	if (IsZipHandleU(zip))
		CloseZipU(zip);
	else
		CloseZipZ(zip);
	return bResult;
}

bool UnzipUccClothes(unsigned char* data, unsigned long len, Bitmap* front,
	Bitmap* back, Bitmap* icon, sUccHeader* header)
{
	HZIP zip = OpenZipU(data, len, ZIP_MEMORY);
	bool bResult = true;
	ZIPENTRY entry;
	int index;
	if (header != NULL)
	{
		if (FindZipItemA(zip, "header", true, &index, &entry) == ZR_OK)
			UnzipItem(zip, index, header, entry.unc_size, ZIP_MEMORY);
	}
	if (front != NULL)
	{
		if (FindZipItemA(zip, "front", true, &index, &entry) == ZR_OK)
		{
			if ((front->BitsPerPixel() * front->Height() * front->Width() >>
					3) != entry.unc_size)
			{
				if (IsZipHandleU(zip))
					CloseZipU(zip);
				else
					CloseZipZ(zip);
				return false;
			}
			UnzipItem(zip, index, front->vram, entry.unc_size, ZIP_MEMORY);
			bResult = true;
		}
		else
			bResult = false;
	}
	if (back != NULL)
	{
		if (FindZipItemA(zip, "back", true, &index, &entry) == ZR_OK)
		{
			if ((back->BitsPerPixel() * back->Height() * back->Width() >> 3) !=
				entry.unc_size)
			{
				if (IsZipHandleU(zip))
					CloseZipU(zip);
				else
					CloseZipZ(zip);
				return false;
			}
			UnzipItem(zip, index, back->vram, entry.unc_size, ZIP_MEMORY);
			bResult = bResult != false;
		}
		else
			bResult = false;
	}
	if (icon != NULL)
	{
		if (FindZipItemA(zip, "icon", true, &index, &entry) == ZR_OK)
		{
			// 983 007e8ebc: tamanho errado sai com false e nao descompacta
			if ((icon->BitsPerPixel() * icon->Height() * icon->Width() >> 3) !=
				entry.unc_size)
			{
				if (IsZipHandleU(zip))
					CloseZipU(zip);
				else
					CloseZipZ(zip);
				return false;
			}
			UnzipItem(zip, index, icon->vram, entry.unc_size, ZIP_MEMORY);
		}
		// 983 007e8ea3: icon ausente nao muda o resultado
	}
	if (IsZipHandleU(zip))
		CloseZipU(zip);
	else
		CloseZipZ(zip);
	return bResult;
}

bool LoadWarningClothes(unsigned int id, Bitmap* front, Bitmap* back,
	Bitmap* icon)
{
	if (id == 0 || front == NULL || back == NULL || icon == NULL)
		return false;

	char filename[512] = { 0 };
	sprintf(filename, "data/ucc/warning/%d.jpg", id);
	cFile* file = GetCFile(filename, -1, 0);
	if (file == NULL)
		return false;

	unsigned long len = file->m_nLen;
	unsigned char* data = new unsigned char[len];
	file->Read(data, len);
	if (data == NULL)
	{
		delete[] data;
		CloseCFile(file);
	}
	else
	{
		CloseCFile(file);
		if (!UnzipUccClothes(data, len, front, back, icon, NULL))
		{
			delete[] data;
			return false;
		}
		delete[] data;
		return true;
	}
	return false;
}

FrGraphicInterface* GetGDI()
{
	if (g_pFresh != NULL)
	{
		if (g_pFresh->GetManager() != NULL)
			return g_pFresh->GetManager()->GetGDI();
	}
	return NULL;
}

void RefreshTexCache(Bitmap& bitmap)
{
	if (g_pFresh != NULL)
	{
		if (g_pFresh->GetManager() != NULL)
		{
			FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
			if (gdi != NULL)
				gdi->RefreshTexCache(bitmap);
		}
	}
}

void RefreshTexCache(Bitmap* bitmap)
{
	if (bitmap != NULL)
	{
		if (g_pFresh != NULL)
		{
			if (g_pFresh->GetManager() != NULL)
			{
				FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
				if (gdi != NULL)
					gdi->RefreshTexCache(*bitmap);
			}
		}
	}
}

void ClearTexCache(Bitmap* bitmap)
{
	FrGraphicInterface* gdi = GetGDI();
	if (gdi != NULL && bitmap != NULL)
		gdi->InvalidateCache(*bitmap);
}

float Power(float base, float exponent)
{
	return exp(log(base) * exponent);
}

std::string Lower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

std::string FilterFilename(std::string name)
{
	std::string result;
	for (unsigned int i = 0; i < name.size(); i++)
	{
		char c = name[i];
		if (strchr("[]{}`~!@#$%^&()-+=\\|<>/?", c) == NULL)
			result += c;
	}
	return result;
}

std::string GetMaskFilename(const char* filename)
{
	std::string name = filename;
	const char* ext = strrchr(filename, '.');
	if (ext == NULL)
		return filename;

	name = name.substr(0, ext - filename);
	name += "_mask";
	name += ext;
	return Lower(name);
}

std::string GenerateTextureFullPath(int side, unsigned long typeId,
	const char* uccIndex)
{
	if (side == 0)
		return MakeStr("%s\\%s", "capture",
			GenerateTextureName_F(typeId, uccIndex).c_str());
	return MakeStr("%s\\%s", "capture",
		GenerateTextureName_B(typeId, uccIndex).c_str());
}

std::string GenerateIconFilename(unsigned long typeId, const char* uccIndex)
{
	if (Doc() == NULL || ItemManager() == NULL)
		return "";

	std::string name;
	IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
	if (part == NULL)
		return "";

	name = FilterFilename(part->Tex[0]);
	name = name.substr(0, name.rfind('.'));
	name += "_";
	name += uccIndex;
	name += "_i.jpg";
	return Lower(name);
}

std::string GenerateTextureName_F(unsigned long typeId, const char* uccIndex)
{
	if (Doc() == NULL || ItemManager() == NULL)
		return "";

	std::string name;
	IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
	if (part == NULL)
		return "";

	name = FilterFilename(part->Tex[0]);
	name = name.substr(0, name.rfind('.'));
	name += "_";
	name += uccIndex;
	name += ".jpg";
	return Lower(name);
}

std::string GenerateTextureName_B(unsigned long typeId, const char* uccIndex)
{
	if (Doc() == NULL || ItemManager() == NULL)
		return "";

	std::string name;
	IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
	if (part == NULL)
		return "";

	name = FilterFilename(part->Tex[1]);
	name = name.substr(0, name.rfind('.'));
	name += "_";
	name += uccIndex;
	name += ".jpg";
	return Lower(name);
}

void Convert24to32(Bitmap* src, Bitmap* dst, const Bitmap* mask)
{
	if (src != NULL && dst != NULL)
	{
		if (0)
			src->GetVram(0);
		for (unsigned int y = 0; y < src->Height(); y++)
		{
			for (unsigned int x = 0; x < src->Width(); x++)
			{
				dst->SetPixel(x, y, 0xff, 0xff, 0xff, 0xff);
				if (mask != NULL)
				{
					unsigned char r;
					unsigned char g;
					unsigned char b;
					mask->GetPixel(x, y, r, g, b);
					if (r < 5 && g < 5 && b < 5)
						dst->SetPixel(x, y, 0xff, 0xff, 0xff, 0);
				}
			}
		}
	}
}

void Convert32to24(Bitmap* src, Bitmap* dst, const Bitmap* mask)
{
	if (src != NULL && dst != NULL && mask != NULL)
	{
		dst->Create(src->Width(), dst->Height(), 24);
		sRGBA color;
		sRGBA maskColor;
		sRGBA result;
		for (unsigned int y = 0; y < src->Height(); y++)
		{
			for (unsigned int x = 0; x < src->Width(); x++)
			{
				src->GetPixel(x, y, color.r, color.g, color.b);
				mask->GetPixel(x, y, maskColor.r, maskColor.g, maskColor.b);
				SetBrightness(color, maskColor, result);
				dst->SetPixel(x, y, result.r, result.g, result.b, 0xff);
			}
		}
	}
}

void MixColor(sRGBA& base, sRGBA& overlay, sRGBA& result)
{
	// 983 007e8920: fdiv por 255.0, nao multiplica pelo inverso
	float baseAlpha = (float)base.a / 255.0;
	float overlayAlpha = (float)overlay.a / 255.0;
	float inverseAlpha = 1.0f - overlayAlpha;
	result.r = (unsigned char)((float)overlay.r * overlayAlpha +
		baseAlpha * (float)base.r * inverseAlpha);
	result.g = (unsigned char)((float)overlay.g * overlayAlpha +
		(float)base.g * baseAlpha * inverseAlpha);
	result.a = 0xff;
	result.b = (unsigned char)((float)base.b * baseAlpha * inverseAlpha +
		(float)overlay.b * overlayAlpha);
}

void SetBrightness(sRGBA& color, sRGBA& mask, sRGBA& result)
{
	// 983 007e9180: fdiv por 3.0 e 255.0, nao multiplica pelo inverso
	float avg = (float)(color.g + color.b + color.r) / 3.0;
	float maskAvg = (float)(mask.b + mask.g + mask.r) / 3.0;
	float product = maskAvg * avg / 255.0;
	float factor = product / avg;
	int r = (int)((float)color.r * factor);
	if (r > 0xff)
		r = 0xff;
	result.r = r;
	int g = (int)((float)color.g * factor);
	if (g > 0xff)
		g = 0xff;
	result.g = g;
	int v = (int)((float)color.b * factor);
	result.b = Min(v, 0xff);
	result.a = color.a;
}

void GetResizedImage(const Bitmap* src, const Bitmap* mask, Bitmap* dst,
	int width, int height)
{
	if (0)
		ceil(0.0f);
	if (src == NULL || dst == NULL)
		return;

	float scaleX = (float)src->Width() / (float)width;
	float scaleY = (float)src->Height() / (float)height;
	bool bMaskless;
	if (mask == NULL && src->BitsPerPixel() == 0x20)
		bMaskless = true;
	else
		bMaskless = false;
	dst->Create(width, height, 0x20);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int x0 = (int)floor((double)((float)x * scaleX));
			int y0 = (int)floor((double)((float)y * scaleY));
			int x1 = (int)ceil((double)((float)(x + 1) * scaleX - 1.0f));
			int y1 = (int)ceil((double)((float)(y + 1) * scaleY - 1.0f));
			if (mask != NULL && NoDrawArea(mask, x0, y0, x1, y1))
			{
				dst->SetPixel(x, y, 0, 0, 0, 0);
			}
			else
			{
				const sRGBA& color =
					GetAverageColor(src, x0, y0, x1, y1, bMaskless);
				dst->SetPixel(x, y, color.r, color.g, color.b, color.a);
			}
		}
	}
}

sRGBA GetAverageColor(const Bitmap* bitmap, int left, int top, int right,
	int bottom, bool bUseAlpha)
{
	unsigned int sumR = 0;
	unsigned int sumG = 0;
	unsigned int sumB = 0;
	unsigned int sumA = 0;
	unsigned int weightSum = 0;
	unsigned int countA = 0;
	for (int y = top; y <= bottom; y++)
	{
		if ((unsigned int)y >= bitmap->Height())
			break;
		for (int x = left; x <= right; x++)
		{
			if ((unsigned int)x >= bitmap->Width())
				break;
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
			if (bitmap->bi->bmiHeader.biBitCount == 0x20)
			{
				bitmap->GetPixel(x, y, r, g, b, a);
			}
			else
			{
				a = 0xff;
				bitmap->GetPixel(x, y, r, g, b);
			}
			sRGBA color(r, g, b, a);
			unsigned char value;
			if (bUseAlpha)
				value = a;
			else
				value = color.toHSV().v;
			unsigned char weight = (0xff - value) / 20 + 1;
			sumR += r * weight;
			sumG += g * weight;
			sumB += b * weight;
			sumA += a;
			weightSum += weight;
			if (a > 0)
				countA++;
		}
	}
	if (weightSum != 0)
	{
		sumR /= weightSum;
		sumG /= weightSum;
		sumB /= weightSum;
	}
	if (countA != 0)
		sumA /= countA;
	return sRGBA(sumR, sumG, sumB, sumA);
}

bool NoDrawArea(const Bitmap* mask, int left, int top, int right, int bottom)
{
	for (int y = top; y <= bottom; y++)
	{
		for (int x = left; x <= right; x++)
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			mask->GetPixel(x, y, r, g, b);
			if (r <= 0x10 && g <= 0x10 && b <= 0x10)
				return true;
		}
	}
	return false;
}

sRGBA GetSimilarColor(sRGBA base, sRGBA first, sRGBA second)
{
	sRGBA baseColor = base;
	sRGBA firstColor = first;
	sRGBA secondColor = second;
	baseColor.RemoveAlpha();
	firstColor.RemoveAlpha();
	secondColor.RemoveAlpha();
	sHSV baseHsv;
	baseHsv = baseColor.toHSV();
	sHSV firstHsv;
	firstHsv = firstColor.toHSV();
	sHSV secondHsv;
	secondHsv = secondColor.toHSV();
	int hueFirst =
		Min(abs(baseHsv.h - firstHsv.h), abs(firstHsv.h - baseHsv.h + 360));
	int hueSecond =
		Min(abs(baseHsv.h - secondHsv.h), abs(secondHsv.h - baseHsv.h + 360));
	if (hueFirst < hueSecond)
		return first;
	if (hueFirst > hueSecond)
		return second;
	if (abs(baseHsv.v - firstHsv.v) < abs(baseHsv.v - secondHsv.v))
		return first;
	return second;
}

void RGBToHLS(unsigned char r, unsigned char g, unsigned char b,
	unsigned char& h, unsigned char& l, unsigned char& s)
{
	WORD hue;
	WORD lum;
	WORD sat;
	ColorRGBToHLS(RGB(r, g, b), &hue, &lum, &sat);
	h = (unsigned char)hue;
	l = (unsigned char)lum;
	s = (unsigned char)sat;
}

void HLSToRGB(unsigned char h, unsigned char l, unsigned char s,
	unsigned char& r, unsigned char& g, unsigned char& b)
{
	COLORREF color = ColorHLSToRGB(h, l, s);
	r = GetRValue(color);
	g = GetGValue(color);
	b = GetBValue(color);
}

unsigned char HueToRGB(unsigned char a, unsigned char b, unsigned char hue)
{
	if (hue > 0xf0)
		hue += 0x10;
	if (hue < 0x28)
		return (unsigned char)((int)(((unsigned int)b - (unsigned int)a) * hue +
								   0x14) /
				   0x28) +
			a;
	if (hue < 0x78)
		return b;
	if (hue >= 0xa0)
		return a;
	return (unsigned char)((int)(((unsigned int)b - (unsigned int)a) *
								   (0xa0 - hue) +
							   0x14) /
			   0x28) +
		a;
}

void GetUccInfoFromPacket(WReceivedPacket* packet)
{
	if (packet != NULL)
	{
		for (int i = packet->Decode1(); i > 0; i--)
		{
			unsigned long id = packet->Decode4();
			unsigned long typeId = packet->Decode4();
			char uccIndex[9];
			memset(uccIndex, 0, sizeof(uccIndex));
			strncpy(uccIndex, packet->DecodeStr().c_str(), 9);
			UccManager()->AddClothes(0, id, typeId, uccIndex, false, false);
		}
	}
}

void RequestUccInfo(sUserInfoTime& info)
{
	RequestUccInfo(info.info.charInfo);
}

void RequestUccInfo(sUserInfo& info)
{
	RequestUccInfo(info.charInfo);
}

void RequestUccInfo(sRoomSlot& slot)
{
	RequestUccInfo(slot.charInfo);
}

void RequestUccInfo(ranktype::sRankUserInfo& info)
{
	RequestUccInfo(info.charInfo);
}

void RequestUccInfo(sCharacterInfo& info)
{
	for (int i = 0; i < 24; i++)
	{
		unsigned long typeId = info.tidParts[i];
		unsigned long guid = info.ItemIdList[i];
		// 983 007e9840: guid 0 nao pede nada ao servidor
		if (typeId != 0 && IsUccClothes(typeId) && guid != 0 &&
			UccManager()->FindClothes(guid) == NULL)
		{
			WSendPacket packet((enumClientPacket)0xb9);
			packet.Encode1(1);
			packet.Encode4(guid);
			packet.Encode1(0);
			packet.Send(TO_GAME);
		}
	}
}

void RequestUccInfo(sMailIncludeItem& item)
{
	if (IsUccClothes(item.dwTID))
		UccManager()->AddClothes(0, 0, item.dwTID, item.csUCCIndex, false,
			false);
}

sHSV sRGB::toHSV()
{
	sHSV hsv;
	double minValue = Min(r, g, b);
	double maxValue = Max(r, g, b);
	double delta = maxValue - minValue;
	hsv.v = (unsigned char)maxValue;
	if (delta == 0.0)
	{
		hsv.h = 0;
		hsv.s = 0;
	}
	else
	{
		hsv.s = (unsigned char)(delta / maxValue * 255.0);
		double hue;
		if (r == (int)maxValue)
			hue = (g - b) / delta;
		else if (g == (int)maxValue)
			hue = (b - r) / delta + 2.0;
		else
			hue = (r - g) / delta + 4.0;
		hue *= 60.0;
		if (hue < 0.0)
			hue += 360.0;
		if (hue == 360.0)
			hue = 0.0;
		hsv.h = (unsigned char)hue;
	}
	return hsv;
}

sRGB sHSV::toRGB()
{
	sRGB rgb;
	double value = v;
	double chroma = s * value / 255.0; // 983 007e9294: fdiv, nao multiplica por 1/255
	double low = value - chroma;
	int hue = h;
	double hueValue = hue;
	if (hue <= 300 && h > 60)
	{
		if (h < 180)
		{
			rgb.g = (unsigned char)value;
			if (h < 120)
			{
				rgb.b = (unsigned char)low;
				rgb.r = (unsigned char)(low -
					(hueValue / 60.0 - 2.0) * chroma);
			}
			else
			{
				rgb.r = (unsigned char)low;
				rgb.b =
					(unsigned char)((hueValue / 60.0 - 2.0) * chroma +
						low);
			}
		}
		else
		{
			rgb.b = (unsigned char)value;
			if (h < 240)
			{
				rgb.r = (unsigned char)low;
				rgb.g = (unsigned char)(low -
					(hueValue / 60.0 - 4.0) * chroma);
			}
			else
			{
				rgb.g = (unsigned char)low;
				rgb.r =
					(unsigned char)((hueValue / 60.0 - 4.0) * chroma +
						low);
			}
		}
	}
	else
	{
		rgb.r = (unsigned char)value;
		if (hue > 300)
		{
			rgb.g = (unsigned char)low;
			rgb.b =
				(unsigned char)((((hueValue - 360.0) / 60.0) * chroma -
									low) *
					-1.0);
		}
		else
		{
			rgb.b = (unsigned char)low;
			rgb.g = (unsigned char)(((hueValue / 60.0) * chroma) + low);
		}
	}
	return rgb;
}

void sRGBA::RemoveAlpha()
{
	if (a != 0xff)
	{
		sHSV hsv = toHSV();
		hsv.s = (unsigned char)(a * hsv.s / 255.0); // 983 007e9ae6: fdiv
		sRGB rgb = hsv.toRGB();
		r = rgb.r;
		g = rgb.g;
		b = rgb.b;
		a = 0xff;
	}
}

