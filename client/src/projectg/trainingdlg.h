#pragma once

#include "frform.h"

class FrListBox;
class FrNewTrainingOptionDlg;

class FrTrainingDlg : public FrForm
{
	DECLARE_OBJECT(FrTrainingDlg)

	FrTrainingDlg()
		: m_pTraining(NULL)
	{
	}

protected:
	void OnTrainingInit(int param);
	void OnTrainingBtnUp();
	void OnTrainingOwnerDraw(int param);
	void OnTrainingCloseBtnUp();
	bool OnNewTrainingOptionDlgResult(int result, FrForm* form);

	FrListBox* m_pTraining;
	FrNewTrainingOptionDlg* m_pOptionDlg;

	DECLARE_FRESH_MSGMAP()
};

struct sButtonInfo
{
	unsigned char type;
	char icon[4];
	const char* text;
};
