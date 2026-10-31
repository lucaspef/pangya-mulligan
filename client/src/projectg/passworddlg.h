#pragma once

#include <string>
#include "frform.h"

class FrEdit;

class FrPasswordDlg : public FrForm
{
	DECLARE_OBJECT(FrPasswordDlg)

	FrPasswordDlg();

	void InitContent();

protected:
	void OnPasswordInit(int param);
	bool OnPasswordEnterKey(int param);
	void OnOkBtnUp();

	std::string m_password;
	FrEdit* m_pPassword;

	DECLARE_FRESH_MSGMAP()
};
