#pragma once

#include "frform.h"

class FrArea;
class FrEdit;
class FrButton;
class FrStatic;

class FrCaddieWarningDlg : public FrForm
{
	DECLARE_OBJECT(FrCaddieWarningDlg)

	FrCaddieWarningDlg();

	void SetCaddieInfo(const sCaddieInfo& info);

protected:
	virtual bool OnInit();

	void OnPortraitInit(int param);
	void OnVacationInit(int param);
	void OnLimitInit(int param);
	void OnNameInit(int param);
	void OnMessageInit(int param);
	void OnOkInit(int param);
	void OnWarningButtonInit(int param);
	void OnWarningTextInit(int param);
	void OnOkBtnUp();

	sCaddieInfo m_caddieInfo;
	FrArea* m_pPortrait;
	FrArea* m_pVacation;
	FrArea* m_pLimit;
	FrEdit* m_pName;
	FrEdit* m_pMessage;
	FrButton* m_pOk;
	FrButton* m_pWarningButton;
	FrStatic* m_pWarningText;

	DECLARE_FRESH_MSGMAP()
};
