#pragma once

#include "frform.h"

class FrButton;

class FrReplayModeDlg : public FrForm
{
	DECLARE_OBJECT(FrReplayModeDlg)

	FrReplayModeDlg();

	void SetReplayMode(int mode);
	int GetReplayMode();

protected:
	void OnOkInit(int param);
	void OnOkBtnUp();
	void OnCancelInit(int param);
	void OnCancelBtnUp();

	FrButton* m_pOk;
	FrButton* m_pCancel;
	int m_replayMode;

	DECLARE_FRESH_MSGMAP()
};
