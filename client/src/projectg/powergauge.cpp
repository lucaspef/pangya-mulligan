#include "minatl.h"
#include "vc10_layout.h"
#include "club.h"
#include "golfdoc.h"
#include "cardsystem.h"
#include "wind.h"
#include "../shared/localize.h"
#include "powergauge.h"
#include "shareddoc.h"
#include "jp/impactzoneobject.h"

// JP 983: unidade 0085b760-0085ce70 (ver powergauge.h)
C_ASSERT_VC10(sizeof(CPowerGauge) == 0x58);

const float LUCKY_PANGYA_AREA = 2.0f;


float CPowerGauge::GetClubArea983() // 0085b760
{
	return m_clubArea983;
}

void CPowerGauge::SetDebugAirNightLuckyPangYaArea(float area) // 0085b7a0
{
	m_debugAirNightLuckyPangYaArea = area;
}

// 0085b7a0 + 0085bc50 pelo CMascot (00762f60): area de cada mascote
float CPowerGauge::GetMascotArea983(unsigned long typeId) // 0085b7b0
{
	float area = 0.0f;
	switch (typeId)
	{
	case 0x40000007:
	case 0x40000023:
		return LUCKY_PANGYA_AREA;
	case 0x40000014:
	case 0x40000015:
	case 0x40000016:
	case 0x40000017:
	case 0x4000001e:
	case 0x4000001f:
	case 0x40000020:
	case 0x40000021:
	case 0x4000002f:
	case 0x40000030:
	case 0x40000031:
	case 0x40000032:
	case 0x40000033:
		area = 1.0f;
		break;
	}
	return area;
}

float CPowerGauge::GetDebugAirNightLuckyPangYaArea() // 0085b830
{
	return m_debugAirNightLuckyPangYaArea;
}

void CPowerGauge::SetValue40_983(float value) // 0085b840
{
	m_value40_983 = value;
}

float CPowerGauge::GetValue40_983() // 0085b850
{
	return m_value40_983;
}

// 645 (console); o 983 nao tem o campo
void CPowerGauge::SetDebugPangYaArea(float area)
{
}

float CPowerGauge::GetAddCardArea() // 0085bc30
{
	if (GolfClub().GetType() == 3)
	{
		return 0.0f;
	}
	else
	{
		float area = CCardManager::Instance()->GetTotalPangyaZone();
		return area;
	}
}

int CPowerGauge::GetNextRand983() // 0085bc50
{
	if (++m_randIndex983 >= 100)
		m_randIndex983 = 0;
	return m_randTable983[m_randIndex983];
}

float CPowerGauge::GetClubSetArea983(int playerIndex) // 0085beb0
{
	unsigned char level = Doc()->m_userInfo[playerIndex].stat.Level;
	IFF_STRUCT::sClubSet* pClub = ItemManager()->FindClubSet(0x10000014);
	if (pClub && level <= pClub->c.Level && Doc()->m_userInfo[playerIndex].clubInfo.tid == 0x10000014)
	{
		float area = 0.0f;
		switch (level)
		{
		case 0:
		case 1:
			return 4.0f;
		case 2:
		case 3:
			return 3.0f;
		case 4:
		case 5:
			return 2.5f;
		case 6:
		case 7:
			return 2.0f;
		case 8:
		case 9:
		case 10:
			return 1.5f;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			area = 0.5f;
			break;
		}
		return area;
	}
	return 0.0f;
}

float CPowerGauge::GetLuckyNecklaceArea(int playerIndex) // 0085bfc0
{
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(0x1a000006);
	float area = 0.0f;
	unsigned char level = Doc()->m_userInfo[playerIndex].stat.Level;
	if (pItem && level <= pItem->c.Level)
	{
		if (level < 21)
			return 1.0f;
		if ((unsigned char)(level - 21) < 10)
			area = 0.5f;
	}
	return area;
}

float CPowerGauge::GetImpactZoneArea983() // 0085c190
{
	float area = 0.0f;
	for (std::vector<CImpactZoneObject*>::iterator it = m_impactZones983.begin(); it != m_impactZones983.end(); ++it)
		area += (*it)->GetZone(); // B1534: +0x64 cru = VC10 (vector de 0x10); no cl 14.44 o m_zone fica em +0x60
	return area;
}

float CPowerGauge::GetAddLuckyPangYaArea() // 0085c1c0: caddie da sorte
{
	switch (GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->item)
	{
	case 0x18000002:
	case 0x18000007:
	case 0x1800000a:
	case 0x18000012:
	case 0x18000029:
	case 0x1800002b:
		return LUCKY_PANGYA_AREA;
	}
	return 0.0f;
}

float CPowerGauge::GetPangYaAreaBase983() // 0085c470
{
	float area = GetImpactZoneArea983() + 2.0;
	area += GetAddLuckyPangYaArea();
	area += GetAddCardArea();
	area += GetClubArea983();
	area += GetDebugAirNightLuckyPangYaArea();
	return area;
}

float CPowerGauge::GetPangYaAreaItem983() // 0085c4d0
{
	float area = GetPangYaAreaBase983();
	area += GetValue40_983();
	if (area < 3.0f)
		return 3.0;
	return area + 0.5;
}

CPowerGauge::~CPowerGauge() // 0085c660
{
}

void CPowerGauge::UpdateClubArea983() // 0085c710
{
	m_impactZones983.clear();
	if (OnlinePlay())
	{
		int i;
		for (i = 0; i < 4; i++)
		{
			if (Doc()->m_userInfo[i].info.dwUID == Doc()->m_myInfo.info.dwUID)
				break;
		}
		float area = GetClubSetArea983(i) + 0.0;
		area += GetLuckyNecklaceArea(i);
		float bonus;
		if (Doc()->m_userInfo[i].clubInfo.tid == 0x10000057)
			bonus = 0.5f;
		else
			bonus = 0.0f;
		m_clubArea983 = bonus + area;
	}
}

void CPowerGauge::ResetRandTable983() // 0085c7e0
{
	m_randIndex983 = 0;
	m_randTable983.clear();
	m_randTable983.reserve(100);
	for (int i = 0; i < 100; i++)
		m_randTable983.push_back(rand());
}

bool CPowerGauge::Init() // 0085ca10
{
	ResetRandTable983();
	return true;
}

CPowerGauge::CPowerGauge() // 0085ca20
	: m_debugAirNightLuckyPangYaArea(0.0f), m_randIndex983(0), m_value40_983(0.0f), m_clubArea983(0.0f)
{
	Init();
}

float CPowerGauge::GetPangYaArea() // 0085cad0
{
	if (Doc()->m_golfGame.gameType != 0xb)
		return GetPangYaAreaBase983();
	return GetPangYaAreaItem983();
}

void CPowerGauge::AddImpactZone983(int playerIndex, CImpactZoneObject* pObject) // 0085caf0
{
	if (std::find(m_impactZones983.begin(), m_impactZones983.end(), pObject) != m_impactZones983.end())
		return;
	if (pObject && Doc()->m_myInfo.info.dwUID == Doc()->m_userInfo[playerIndex].info.dwUID)
		m_impactZones983.push_back(pObject);
}

void CPowerGauge::SetRandTable983(const std::vector<int>& table) // 0085cdd0
{
	m_randIndex983 = 0;
	m_randTable983.clear();
	m_randTable983.reserve(table.size());
	m_randTable983.assign(table.begin(), table.end());
}
