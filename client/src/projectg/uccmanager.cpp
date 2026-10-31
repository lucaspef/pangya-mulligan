#include "minatl.h"
#include "uccmanager.h"
#include "ucclibrary.h"
#include "shareddoc.h"
#include "packet.h"
#include "netresourcemanager.h"
#include "wresrcmng.h"

CUccManager* UccManager()
{
	static CUccManager manager;
	return &manager;
}

CUccManager::CUccManager()
{
	InitializeCriticalSection(&m_csRefresh);
}

CUccManager::~CUccManager()
{
	DeleteCriticalSection(&m_csRefresh);
	ClearClothes();
	ClearAztec();
	m_refreshItem.clear();
	ClearTempClothes();
}

void CUccManager::AddClothes(unsigned char slot, unsigned long id,
	unsigned long typeId, const char* uccIndex, bool bSend, bool bForce)
{
	sUccClothes clothes;
	clothes.id = id;
	clothes.typeId = typeId;
	strncpy(clothes.uccIndex, uccIndex, 9);
	AddClothes(slot, clothes, bSend, bForce);
}

void CUccManager::AddClothes(unsigned char slot, sUccClothes& clothes,
	bool bSend, bool bForce)
{
	sUccClothes* existing = FindClothes(clothes.typeId, clothes.uccIndex);
	if (!existing)
	{
		m_clothes.push_back(clothes);
		if (clothes.downloadCount == 0)
			RequestClothes(clothes.typeId, clothes.uccIndex, bForce);
	}
	else if (clothes.id != 0xffffffff)
	{
		if (!FindClothes(clothes.id))
		{
			sUccClothes copy = *existing;
			copy.typeId = clothes.typeId;
			copy.id = clothes.id;
			m_clothes.push_back(copy);
			AddRefreshItem(copy.typeId, copy.uccIndex);
		}
	}
	if (bSend)
	{
		WSendPacket packet((enumClientPacket)0xb9);
		packet.Encode1(1);
		packet.Encode4(clothes.id);
		packet.Encode1(slot);
		packet.Send((eSendTo)0);
	}
}

void CUccManager::AddAztec(sUccAztec& aztec, bool bForce)
{
	if (!FindAztec(aztec.typeId, aztec.uccIndex))
		m_aztec.push_back(aztec);
}

sUccClothes* CUccManager::FindClothes(unsigned long typeId,
	const char* uccIndex)
{
	std::list<sUccClothes>::iterator it;
	for (it = m_clothes.begin(); it != m_clothes.end(); it++)
	{
		sUccClothes* p = &*it;
		if (p->typeId == typeId && strcmp(p->uccIndex, uccIndex) == 0)
			return p;
	}
	for (it = m_clothes.begin(); it != m_clothes.end(); it++)
	{
		sUccClothes* p = &*it;
		if (strcmp(p->uccIndex, uccIndex) == 0 &&
			IsSameClothes(typeId, p->typeId))
			return p;
	}
	return NULL;
}

sUccAztec* CUccManager::FindAztec(unsigned long typeId, const char* uccIndex)
{
	std::list<sUccAztec>::iterator it;
	for (it = m_aztec.begin(); it != m_aztec.end(); it++)
	{
		sUccAztec* p = &*it;
		if (p->typeId == typeId && strcmp(p->uccIndex, uccIndex) == 0)
			return p;
	}
	return NULL;
}

sUccClothes* CUccManager::FindClothes(unsigned long id)
{
	sItemInfo* info = GetMyItemInfo(id);
	if (info)
	{
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_uccItemList.begin();
			it != Doc()->m_uccItemList.end(); ++it)
		{
			if ((*it).tid == info->tid &&
				strcmp((*it).UccIndex, info->UccIndex) == 0)
			{
				sUccClothes* found = _GetClothes((*it).guid);
				if (found)
					return found;
			}
		}
		for (it = Doc()->m_uccItemList.begin();
			it != Doc()->m_uccItemList.end(); ++it)
		{
			if (strcmp((*it).UccIndex, info->UccIndex) == 0 &&
				IsSameClothes(info->tid, (*it).tid))
			{
				sUccClothes* found = _GetClothes((*it).guid);
				if (found)
					return found;
			}
		}
		return NULL;
	}
	return _GetClothes(id);
}

sUccAztec* CUccManager::FindAztec(unsigned long id)
{
	sItemInfo* info = GetMyItemInfo(id);
	if (!info)
		return _GetAztec(id);
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_uccAztecList.begin(); it != Doc()->m_uccAztecList.end();
		++it)
	{
		if ((*it).tid == info->tid &&
			strcmp((*it).UccIndex, info->UccIndex) == 0)
		{
			sUccAztec* found = _GetAztec((*it).guid);
			if (found)
				return found;
		}
	}
	return NULL;
}

sUccClothes* CUccManager::_GetClothes(unsigned long id)
{
	std::list<sUccClothes>::iterator it;
	for (it = m_clothes.begin(); it != m_clothes.end(); it++)
	{
		sUccClothes* p = &*it;
		if (p->id == id)
			return p;
	}
	return NULL;
}

sUccAztec* CUccManager::_GetAztec(unsigned long id)
{
	std::list<sUccAztec>::iterator it;
	for (it = m_aztec.begin(); it != m_aztec.end(); it++)
	{
		sUccAztec* p = &*it;
		if (p->id == id)
			return p;
	}
	return NULL;
}

bool CUccManager::IsDownloadComplete(unsigned long typeId, const char* uccIndex)
{
	sUccClothes* clothes = FindClothes(typeId, uccIndex);
	sUccAztec* aztec = FindAztec(typeId, uccIndex);
	if (clothes)
	{
		if (clothes->downloadCount == 1)
			return true;
		return false;
	}
	if (aztec)
	{
		if (aztec->downloadCount == 1)
			return true;
		return false;
	}
	return false;
}

bool CUccManager::SetDownloadComplete(unsigned long typeId,
	const char* uccIndex)
{
	sUccClothes* clothes = FindClothes(typeId, uccIndex);
	sUccAztec* aztec = FindAztec(typeId, uccIndex);
	if (clothes)
		return SetDownloadComplete(clothes);
	if (aztec)
	{
		aztec->downloadCount++;
		if (aztec->downloadCount == 1)
			return true;
		return false;
	}
	return false;
}

bool CUccManager::IsDownloadComplete(sUccClothes* clothes)
{
	if (clothes && clothes->downloadCount == 1)
		return true;
	return false;
}

bool CUccManager::IsDownloadComplete(sUccAztec* aztec)
{
	if (aztec && aztec->downloadCount == 1)
		return true;
	return false;
}

bool CUccManager::SetDownloadComplete(sUccClothes* clothes)
{
	if (clothes)
	{
		clothes->downloadCount++;
		if (clothes->downloadCount == 1)
		{
			std::list<sUccClothes>::iterator it;
			for (it = m_clothes.begin(); it != m_clothes.end(); it++)
			{
				if (IsSameClothes(clothes->typeId, (*it).typeId) &&
					strcmpi(clothes->uccIndex, (*it).uccIndex) == 0)
				{
					(*it).downloadCount = clothes->downloadCount;
					(*it).icon = clothes->icon;
					strncpy((*it).uccName, clothes->uccName, 41);
				}
			}
			return true;
		}
	}
	return false;
}

bool CUccManager::SetDownloadComplete(sUccAztec* aztec)
{
	if (aztec)
	{
		aztec->downloadCount++;
		return aztec->downloadCount == 1 ? true : false;
	}
	return false;
}

void CUccManager::RequestClothes(unsigned long typeId, const char* uccIndex,
	bool bForce)
{
	NetResourceManager::Instance()->DownloadUccClothes(typeId, uccIndex, bForce,
		false);
}

void CUccManager::RequestAztec(unsigned long typeId, const char* uccIndex,
	bool bForce)
{
}

void CUccManager::Clear()
{
	ClearTexture();
	ClearClothes();
	ClearAztec();
	ClearIcon();
	m_refreshItem.clear();
	m_displayClothes.clear();
	ClearTempClothes();
}

void CUccManager::ClearTexture()
{
	std::list<sUccClothes>::iterator it;
	for (it = m_clothes.begin(); it != m_clothes.end();)
	{
		for (int i = 0; i < 2; ++i)
		{
			std::string name;
			if (i == 0)
				name = GenerateTextureName_F((*it).typeId, (*it).uccIndex);
			else if (i == 1)
				name = GenerateTextureName_B((*it).typeId, (*it).uccIndex);
			w_texlist* tex = g_resrcmng->FindTexture(name.c_str());
			if (tex)
				g_resrcmng->Release(tex->texhandle);
		}
		it = m_clothes.erase(it);
	}
}

void CUccManager::ClearIcon()
{
	std::map<std::string, Bitmap*>::iterator it;
	for (it = m_icon.begin(); it != m_icon.end(); it++)
	{
		Bitmap* bitmap = (*it).second;
		ClearTexCache(bitmap);
		delete bitmap;
	}
	m_icon.clear();
}

void CUccManager::ClearTempClothes()
{
	std::list<sUccTempClothes>::iterator it;
	for (it = m_tempClothes.begin(); it != m_tempClothes.end(); it++)
	{
		ClearTexCache((*it).front);
		delete (*it).front;
		(*it).front = NULL;
		ClearTexCache((*it).back);
		delete (*it).back;
		(*it).back = NULL;
	}
	m_tempClothes.clear();
}

void CUccManager::AddRefreshItem(unsigned long typeId, const char* uccIndex)
{
	sUccRefreshItem item;
	item.typeId = typeId;
	strncpy(item.uccIndex, uccIndex, 9);
	LPCRITICAL_SECTION cs = &m_csRefresh;
	EnterCriticalSection(cs);
	m_refreshItem.push_back(item);
	LeaveCriticalSection(cs);

	IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
	std::map<unsigned int, IFF_STRUCT::sPart>::iterator it;
	for (it = ItemManager()->m_PartMap.begin();
		it != ItemManager()->m_PartMap.end(); ++it)
	{
		IFF_STRUCT::sPart* other = &(*it).second;
		if (typeId != other->c.TypeId && IsSameClothes(part, other))
		{
			sUccRefreshItem otherItem;
			otherItem.typeId = other->c.TypeId;
			strncpy(otherItem.uccIndex, uccIndex, 9);
			EnterCriticalSection(cs);
			m_refreshItem.push_back(otherItem);
			LeaveCriticalSection(cs);
		}
	}
}

bool CUccManager::GetRefreshItem(sUccRefreshItem& item)
{
	EnterCriticalSection(&m_csRefresh);
	int count = m_refreshItem.size();
	if (count > 0)
	{
		item = m_refreshItem.front();
		m_refreshItem.pop_front();
	}
	LeaveCriticalSection(&m_csRefresh);
	return count > 0;
}

void CUccManager::SetUccTempClothes(unsigned long typeId, const char* uccIndex,
	Bitmap* front, Bitmap* back)
{
	std::list<sUccTempClothes>::iterator it;
	for (it = m_tempClothes.begin(); it != m_tempClothes.end(); it++)
	{
		if ((*it).typeId == typeId && strcmpi((*it).uccIndex, uccIndex) == 0)
		{
			*(*it).front = *front;
			*(*it).back = *back;
			return;
		}
	}
	sUccTempClothes temp;
	temp.typeId = typeId;
	strncpy(temp.uccIndex, uccIndex, 9);
	*temp.front = *front;
	*temp.back = *back;
	m_tempClothes.push_back(temp);
}

bool CUccManager::GetUccTempClothes(unsigned long typeId, const char* uccIndex,
	Bitmap& front, Bitmap& back)
{
	std::list<sUccTempClothes>::iterator it;
	for (it = m_tempClothes.begin(); it != m_tempClothes.end(); it++)
	{
		if ((*it).typeId == typeId && strcmpi((*it).uccIndex, uccIndex) == 0)
		{
			front = *(*it).front;
			back = *(*it).back;
			return true;
		}
	}
	return false;
}

void CUccManager::BuildDisplayClothesSet()
{
	// HACK: the original is empty; this dead reference preserves template emission order.
	if (0)
		m_displayClothes.size();
}

void CUccManager::AddDisplayClothes(unsigned long partTypeId,
	unsigned long itemId, std::string name, int rank)
{
	sDisplayUccClothes display;
	display.partTypeId = partTypeId;
	display.itemId = itemId;
	display.name = name;
	display.part.c.TypeId = partTypeId;
	display.rank = rank;
	m_displayClothes.push_back(display);

	WSendPacket packet((enumClientPacket)0xb9);
	packet.Encode1(1);
	packet.Encode4(itemId);
	packet.Encode1(0);
	packet.Send((eSendTo)0);
}

bool CUccManager::IsDisplayUccClothes(const IFF_ITEM_COMMON* item)
{
	return strcmpi(item->Icon, "UCCDISPLAY") == 0;
}

unsigned long CUccManager::GetWinnerCharacterTypeId()
{
	unsigned long typeId = 0;
	int bestRank = 999999;
	std::list<sDisplayUccClothes>::iterator it;
	for (it = m_displayClothes.begin(); it != m_displayClothes.end(); it++)
	{
		if ((*it).rank < bestRank)
		{
			typeId = (*it).partTypeId;
			bestRank = (*it).rank;
		}
	}
	return ((typeId >> 18) & 0xff) | 0x4000000;
}
