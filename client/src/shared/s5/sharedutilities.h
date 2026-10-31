#pragma once

#include <string>
#include <list>

struct sHoleInBonus
{
	__int64 holeinone;
	__int64 albatross;
	__int64 eagle;
	__int64 birdie;
	__int64 par;
	__int64 over;
};

namespace S5
{
	namespace CLASSICSRV
	{
		enum
		{
			PANG_LONGPUTT_PER_YARD = 4,
			PANG_MULTIPLY_APPROACH = 2,
			PANG_LONGCHIPIN_PER_YARD = 4
		};

		unsigned long GetClearBonus(unsigned char numPlayer,
			unsigned char mapType);
	}

	namespace RENTALPARTS
	{
		int IsRentableParts(unsigned long typeId);
		int GetRentalPrice(unsigned long typeId);
		void CollectRentalItem(std::list<sItemInfo>& rentalItems,
			const std::list<sItemInfo>& items);
		int IsExistExpiredRentalItem(const std::list<sItemInfo>& items);
	}

	__int64 ExchangeOwnCashToCookie(__int64 cash, __int64 cookie, int rate);
	__int64 ExchangeOwnCookieToCash(__int64 cookie, int rate);

	bool IsSpecialGameStyle(eGameStyle style);
	bool IsSpecialGame(unsigned char gameType, eGameStyle style);

	SYSTEMTIME MakeSysTime(unsigned short year, unsigned short month,
		unsigned short day, unsigned short hour);
}

namespace _util
{
	enum ePwdErrorCode
	{
		PWD_OK,
		PWD_NOT_NUMBER,
		PWD_EMPTY,
		PWD_TOO_SHORT,
		PWD_TOO_LONG,
		PWD_SAME_CHARS,
		PWD_ASCENDING,
		PWD_DESCENDING
	};

	ePwdErrorCode VerifyInputPassword(const std::string& password);
	ePwdErrorCode VerifyLength(const std::string& str, int minLength,
		int maxLength);
	ePwdErrorCode VerifyStringContents_IsNumber(const std::string& str);
	ePwdErrorCode VerifyStringContents_Limit(const std::string& str);
}
