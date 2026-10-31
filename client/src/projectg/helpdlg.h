#pragma once

#include "frform.h"

class FrViewer;
class FrArea;

class FrHelpDlg : public FrForm
{
	DECLARE_OBJECT(FrHelpDlg)

	FrHelpDlg()
		: m_pViewer(NULL), m_pTitle(NULL), m_type(0)
	{
	}

protected:
	virtual bool OnInit();

	void OnTitleInit(int param);
	void OnViewInit(int param);
	void OnViewBtnUp();

	FrViewer* m_pViewer;
	FrArea* m_pTitle;
	int m_type;

	DECLARE_FRESH_MSGMAP()
};
