#pragma once

#include "frform.h"

class FrButton;
class FrGaugeBar;
class FrEdit;
class FrArea;

class FrEnchantDlg : public FrForm
{
	DECLARE_OBJECT(FrEnchantDlg)

	FrEnchantDlg();

	void SetType(unsigned char type, unsigned char stat);

	void SetResultCode(unsigned char code) { m_resultCode = code; }

	virtual void OnProc(const float dt);

protected:
	void OnTitleInit(int param);
	void OnBaseInit(int param);
	void OnYesInit(int param);
	void OnNoInit(int param);
	void OnOKInit(int param);
	void OnGaugeBarInit(int param);
	void OnTextEditInit(int param);
	void OnYesBtnUp();
	void OnNoBtnUp();
	void OnOKBtnUp();

	FrButton* m_pYesBtn;
	FrButton* m_pNoBtn;
	FrButton* m_pOKBtn;
	FrGaugeBar* m_pGaugeBar;
	FrEdit* m_pText;
	FrArea* m_pTitle;
	FrArea* m_pBase;
	unsigned char m_type;
	unsigned char m_stat;
	unsigned long m_itemId;
	unsigned long m_statColor;
	const char* m_statName;
	__int64 m_price;
	bool m_bResultShown;
	unsigned char m_resultCode;

	DECLARE_FRESH_MSGMAP()
};
