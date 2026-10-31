#pragma once
#include "frform.h"
class FrUpgradeCaddieDlg : public FrForm
{
	DECLARE_OBJECT(FrUpgradeCaddieDlg)
	FrUpgradeCaddieDlg();

	void SetUpgradeInfo(unsigned long caddieId, int* stat, int oldPrice,
		int newPrice);

protected:
	void OnMessageInit(int param);
	void OnYesBtnUp();
	void OnNoBtnUp();

	FrEdit* m_pMessage;
	int m_stat[5];
	unsigned long m_caddieId;
	int m_oldPrice;
	int m_newPrice;

	DECLARE_FRESH_MSGMAP()
};
