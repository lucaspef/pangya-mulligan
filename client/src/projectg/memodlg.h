#pragma once

#include "frform.h"

class FrEdit;

class FrMemoDlg : public FrForm
{
	DECLARE_OBJECT(FrMemoDlg)

	FrMemoDlg()
		: m_pMemo(NULL), m_focusTime(0.0f), m_bFocusSet(false)
	{
	}

	const char* GetMemo();

protected:
	virtual void OnProc(const float deltaTime);

	void OnMemoInit(int param);
	void OnMemoLButtonDown();
	bool OnMemoEnterKey(int param);
	void OnOkBtnUp();

	FrEdit* m_pMemo;
	float m_focusTime;
	bool m_bFocusSet;

	DECLARE_FRESH_MSGMAP()
};
