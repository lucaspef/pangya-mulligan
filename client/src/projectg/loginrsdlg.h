#pragma once

#include "frform.h"

class FrButton;

enum eLoginToRs
{
	LOGINRS_NONE,
	LOGINRS_CONNECTING,
	LOGINRS_CONNECTED,
	LOGINRS_DONE,
};

class FrLoginRsDlg : public FrForm
{
	DECLARE_OBJECT(FrLoginRsDlg)

	FrLoginRsDlg();

	static void SetState(eLoginToRs state);
	static eLoginToRs GetState();

protected:
	virtual void OnProc(const float deltaTime);

	void OnCancelInit(int param);

	static eLoginToRs m_connState;

	float m_elapsedTime;
	FrButton* m_pCancel;

	DECLARE_FRESH_MSGMAP()
};
