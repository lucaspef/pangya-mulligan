#pragma once

#include "frform.h"
#include "../shared/classdefine.h"

class FrListBox;
class FrEdit;

class FrRepayDlg : public FrForm
{
	DECLARE_OBJECT(FrRepayDlg)

	FrRepayDlg();

	void Repay(const IFF_ITEM_COMMON& item, unsigned long guid, __int64 myPang,
		__int64 myCookie);

	unsigned long GetTid() { return m_item.TypeId; }
	unsigned long GetGuid() { return m_guid; }

protected:
	void OnCartInit(int param);
	void OnCartOwnerDraw(int param);
	void OnPriceInit(int param);
	void OnMyMoneyInit(int param);
	void OnMoneyLeftInit(int param);
	void OnPrice_CookieInit(int param);
	void OnMyMoney_CookieInit(int param);
	void OnMoneyLeft_CookieInit(int param);

	FrListBox* m_pCart;
	FrEdit* m_pPrice;
	FrEdit* m_pMyMoney;
	FrEdit* m_pMoneyLeft;
	FrEdit* m_pPriceCookie;
	FrEdit* m_pMyMoneyCookie;
	FrEdit* m_pMoneyLeftCookie;
	IFF_ITEM_COMMON m_item;
	unsigned long m_guid;
	__int64 m_pang;
	__int64 m_cookie;

	DECLARE_FRESH_MSGMAP()
};
