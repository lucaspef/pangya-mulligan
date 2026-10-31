#include "minatl.h"
#include <algorithm>
#include <map>
#include "treasuremap.h"

typedef std::vector<CTMapInfo*>::iterator TMapIter;

#include "treasure.h"
#include "shareddoc.h"

CTMapInfo::CTMapInfo()
	: m_TMapIndex(0), m_TMark(2), m_gauge(0)
{
}

CTMapInfo::~CTMapInfo()
{
}

unsigned char CTMapInfo::GetTMapIndex() const
{
	return m_TMapIndex;
}

void CTMapInfo::SetTMapIndex(unsigned char index)
{
	m_TMapIndex = index;
}

unsigned char CTMapInfo::GetTMark() const
{
	return m_TMark;
}

void CTMapInfo::SetTMark(unsigned char mark)
{
	m_TMark = mark;
}

unsigned long CTMapInfo::GetGauge() const
{
	return m_gauge;
}

void CTMapInfo::SetGauge(unsigned long gauge)
{
	m_gauge = gauge;
}

CTHunter::CTHunter()
	: m_treasurePoint(0),
	  m_bReturnLobby(false),
	  m_bReceiveGiftPacket(false),
	  m_bUsedTikiReport(false),
	  m_giftReserveSize(24)
{
}

CTHunter::~CTHunter()
{
	std::vector<CTMapInfo*>::iterator it;
	for (it = m_vecTMap.begin(); it != m_vecTMap.end(); it++)
	{
		if (*it)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_vecTMap.clear();
}

void CTHunter::Initialize(int count)
{
	for (int i = 0; i < count; i++)
	{
		AddTreasure(i, 0);
	}

	m_vecGift.reserve(m_giftReserveSize);
}

void CTHunter::AddTreasure(unsigned char index, unsigned long gauge)
{
	CTMapInfo* pInfo = new CTMapInfo;
	pInfo->SetTMapIndex(index);
	pInfo->SetGauge(gauge);
	m_vecTMap.push_back(pInfo);
}

bool CTHunter::DeleteTreasure(unsigned char index)
{
	std::vector<CTMapInfo*>::iterator it = m_vecTMap.begin();
	while (it != m_vecTMap.end())
	{
		if (index == (*it)->GetTMapIndex())
		{
			delete *it;
			m_vecTMap.erase(it);
			return true;
		}
		it++;
	}

	return false;
}

unsigned char CTHunter::FindTreasure(unsigned char index)
{
	std::vector<CTMapInfo*>::iterator it =
		std::find_if(m_vecTMap.begin(), m_vecTMap.end(), THunterFinder(index));
	return (*it)->GetTMapIndex();
}

void CTHunter::SetTHunterGauge(unsigned char index, unsigned long gauge)
{
	if (m_vecTMap.size() > index)
	{
		m_vecTMap[index]->SetGauge(gauge);
	}
}

unsigned long CTHunter::GetTHunterGauge(eMapType mapType)
{
	CMapTypeAdapter::eMapIndex index = CMapTypeAdapter::GetMapIndex(mapType);
	if (index == -1)
	{
		return 9999;
	}

	return m_vecTMap[index]->GetGauge();
}

unsigned char CTHunter::GetTHunterMark(eMapType mapType)
{
	CMapTypeAdapter::eMapIndex index = CMapTypeAdapter::GetMapIndex(mapType);
	if (index == -1)
	{
		return 2;
	}

	return m_vecTMap[index]->GetTMark();
}

unsigned char CTHunter::GetTreasureMapSize()
{
	unsigned char size = (unsigned char)m_vecTMap.size();
	return size;
}

void CTHunter::SetTreasurePoint(unsigned long point)
{
	if (point >= 1000)
	{
		m_treasurePoint = 1000;
	}
	else
	{
		m_treasurePoint = point;
	}
}

unsigned long CTHunter::GetTreasurePoint() const
{
	return m_treasurePoint;
}

void CTHunter::ResetTGift()
{
	m_vecGift.clear();
}

void CTHunter::DecideTMark()
{
	unsigned long gauge = m_vecTMap[0]->GetGauge();
	int count = (int)m_vecTMap.size();

	std::vector<unsigned char> vecMark;
	int i;
	for (i = 1; i < count; i++)
	{
		unsigned long cur = m_vecTMap[i]->GetGauge();
		if (max(cur, gauge) != 0)
		{
			gauge = cur;
		}
	}

	for (i = 0; i < count; i++)
	{
		if (m_vecTMap[i]->GetGauge() == gauge)
		{
			vecMark.push_back(i);
		}
	}

	unsigned int k;
	for (k = 0; k < vecMark.size(); k++)
	{
		m_vecTMap[vecMark[k]]->SetTMark(1);
	}

	vecMark.clear();

	gauge = m_vecTMap[0]->GetGauge();
	for (i = 1; i < count; i++)
	{
		unsigned long cur = m_vecTMap[i]->GetGauge();
		if (min(cur, gauge) != 0)
		{
			gauge = cur;
		}
	}

	for (i = 0; i < count; i++)
	{
		if (m_vecTMap[i]->GetGauge() == gauge)
		{
			vecMark.push_back(i);
		}
	}

	for (k = 0; k < vecMark.size(); k++)
	{
		m_vecTMap[vecMark[k]]->SetTMark(0);
	}
}

void CTHunter::ReceiveAllGift(std::vector<sTreasureGift>& gifts)
{
	if (!m_vecGift.empty())
	{
		m_vecGift.clear();
	}

	m_vecGift.swap(gifts);
}

void CTHunter::UpdateItemList(std::string str)
{
	int count = (int)m_vecGift.size();
	for (int i = 0; i < count; i++)
	{
		sTreasureGift gift;
		gift.Reset();
		gift = m_vecGift[i];

		sItemInfo item;
		memset(&item, 0, sizeof(item));

		switch (gift.dwTid >> 26)
		{
		case 0x1f:
			CSharedDoc::Instance()->AddCard(gift.dwTid, gift.dwItemGuid,
				gift.wCount);
			break;

		case 0x1c:
		{
			std::list<sItemInfo>::iterator it;
			for (it = CSharedDoc::Instance()->m_auxPartList.begin();
				it != CSharedDoc::Instance()->m_auxPartList.end(); ++it)
			{
				if (it->guid == gift.dwItemGuid)
				{
					it->Common[0] += gift.wCount;
					break;
				}
			}

			if (it == CSharedDoc::Instance()->m_auxPartList.end())
			{
				item.tid = gift.dwTid;
				item.guid = gift.dwItemGuid;
				item.Common[0] = max(1, gift.wCount);
				item.ItemType = gift.byItemType;
				item.gift_flag = 0;
				item.Expired = 0;
				memset(&item.ItemDate, 0, sizeof(item.ItemDate));
				item.IsValid = 1;
				item.Purchase = 0;
				CSharedDoc::Instance()->m_auxPartList.push_back(item);
			}
			break;
		}

		case 5:
		case 6:
			// 983: 0x1a000083 e Pang (0x1a000010) nao entram na lista
			if (gift.dwTid != 0x1a000083 && gift.dwTid != 0x1a000010)
			{
				std::list<sItemInfo>& items = (gift.dwTid >> 26) == 5
					? CSharedDoc::Instance()->m_ballList : CSharedDoc::Instance()->m_myItemList;
				std::list<sItemInfo>::iterator it;
				for (it = items.begin(); it != items.end(); ++it)
				{
					if (it->guid == gift.dwItemGuid)
					{
						it->Common[0] += gift.wCount;
						break;
					}
				}

				if (it == items.end())
				{
					item.tid = gift.dwTid;
					item.guid = gift.dwItemGuid;
					item.Common[0] = max(1, gift.wCount);
					item.ItemType = gift.byItemType;
					item.gift_flag = 0;
					item.Expired = 0;
					item.HourRemain = gift.iHourRemain;
					memset(&item.ItemDate, 0, sizeof(item.ItemDate));
					item.IsValid = 1;
					item.Purchase = 0;
					items.push_back(item);
				}
			}
			break;
		}
	}
}

void CTHunter::ReceiveRepository(std::vector<sTreasureHunt>& repository)
{
	if (!m_vecRepository.empty())
	{
		m_vecRepository.clear();
	}

	m_vecRepository.swap(repository);
}

sTreasureHunt CTHunter::GetGiftInfo(unsigned char index)
{
	return m_vecRepository[index];
}

void CTHunter::ReAdjustRepository()
{
	std::map<unsigned long, sTreasureHunt> mapItem;
	std::map<unsigned long, sTreasureHunt>::iterator mit;
	sTreasureHunt item;

	for (std::vector<sTreasureHunt>::iterator it = m_vecRepository.begin();
		it != m_vecRepository.end(); ++it)
	{
		item = *it;

		mit = mapItem.find(item.dwTid);
		if (mit == mapItem.end())
		{
			mapItem.insert(std::make_pair(item.dwTid, item));
		}
		else
		{
			(*mit).second.wCount += item.wCount;
		}
	}

	int size = (int)mapItem.size();
	m_vecRepository.clear();
	m_vecRepository.reserve(size);

	for (mit = mapItem.begin(); mit != mapItem.end(); ++mit)
	{
		m_vecRepository.push_back((*mit).second);
	}

	mapItem.clear();
}
