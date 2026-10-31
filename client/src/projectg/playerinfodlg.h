#pragma once

#include "frform.h"

class FrEdit;

class FrPlayerInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrPlayerInfoDlg)

	FrPlayerInfoDlg();
	virtual ~FrPlayerInfoDlg();

	void WritePlayerInfoDlg();

protected:
	void OnUserInfoInit(int param);
	void OnCharInfoInit(int param);
	void OnCadInfoInit(int param);

	FrEdit* m_pUserInfo;
	FrEdit* m_pCharInfo;
	FrEdit* m_pCadInfo;

	DECLARE_FRESH_MSGMAP()
};
