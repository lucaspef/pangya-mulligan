#pragma once

#include <string>
#include <vector>

#include "../shared/globalgamedefine.h"

class CTMapInfo;

class CTHunter : public WSingleton<CTHunter>
{
public:
	CTHunter();
	virtual ~CTHunter();

	void Initialize(int count);
	void AddTreasure(unsigned char index, unsigned long gauge);
	bool DeleteTreasure(unsigned char index);
	unsigned char FindTreasure(unsigned char index);
	void SetTHunterGauge(unsigned char index, unsigned long gauge);
	unsigned long GetTHunterGauge(eMapType mapType);
	unsigned char GetTHunterMark(eMapType mapType);
	unsigned char GetTreasureMapSize();
	void SetTreasurePoint(unsigned long point);
	unsigned long GetTreasurePoint() const;
	void ResetTGift();
	void DecideTMark();
	void ReceiveAllGift(std::vector<sTreasureGift>& gifts);
	void UpdateItemList(std::string str);
	void ReceiveRepository(std::vector<sTreasureHunt>& repository);
	sTreasureHunt GetGiftInfo(unsigned char index);
	void ReAdjustRepository();

	unsigned char GetRepositorySize()
	{
		return (unsigned char)m_vecRepository.size();
	}
	unsigned char GetGiftSize() { return (unsigned char)m_vecGift.size(); }

	bool IsReturnLobby() { return m_bReturnLobby; }
	void SetReturnLobby(bool b) { m_bReturnLobby = b; }

	bool IsReceiveGiftPacket() { return m_bReceiveGiftPacket; }
	void SetReceiveGiftPacket(bool b) { m_bReceiveGiftPacket = b; }

	bool IsUsedTikiReport() { return m_bUsedTikiReport; }
	void SetUsedTikiReport(bool b) { m_bUsedTikiReport = b; }

private:
	std::vector<CTMapInfo*> m_vecTMap;
	std::vector<sTreasureHunt> m_vecRepository;
	std::vector<sTreasureGift> m_vecGift;
	unsigned long m_treasurePoint;
	unsigned char m_giftReserveSize;
	bool m_bReturnLobby;
	bool m_bReceiveGiftPacket;
	bool m_bUsedTikiReport;
};
