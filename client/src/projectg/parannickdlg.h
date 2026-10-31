#pragma once

#include "frform.h"

class FrEdit;

class FrParanNickDlg : public FrForm
{
	DECLARE_OBJECT(FrParanNickDlg)
	FrParanNickDlg();

	void CloseDlg();
	void EnableControls(bool bEnable);

protected:
	void OnNickInit(int param);
	void OnOkBtnUp();

	FrEdit* m_pNick;

	DECLARE_FRESH_MSGMAP()
};
