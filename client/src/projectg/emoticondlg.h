#pragma once

#include "frform.h"

class FrListBox;

class FrEmoticonDlg : public FrForm
{
	DECLARE_OBJECT(FrEmoticonDlg)

	FrEmoticonDlg();
	virtual ~FrEmoticonDlg();

	const char* GetSelectedIcon() { return m_selectedIcon; }

protected:
	virtual bool OnInit();
	virtual void OnOK();

	void OnListInit(int param);
	void OnListOwnerDraw(int param);
	void OnListBtnUp();

	FrListBox* m_pList;
	char m_selectedIcon[64];
	unsigned long m_startTime;

	DECLARE_FRESH_MSGMAP()
};
