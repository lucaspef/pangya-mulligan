#pragma once

#include <map>
#include <string>

enum eCaddie
{
	W_BIGBONGDARI = 12,
	R_BIGBONGDARI = 13,
};

class CCaddieBonus
{
public:
	CCaddieBonus();
	~CCaddieBonus();

	void SetTicket();
	__int64 GetBonusPang(eCaddie caddie, __int64 pang);

private:
	bool IsHaveCaddie(eCaddie caddie);
	void Release();
	void RegisterBonus(int caddie, std::string name);
	__int64 GetWBigPontaBonus(int ticket);
	__int64 GetRBigPontaBonus(int ticket, __int64 pang);

	std::map<int, std::string> m_bonus;
};
