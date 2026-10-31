#pragma once

#include "frform.h"

class FrFindEbortEventDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrFindEbortEventDlg)

	FrFindEbortEventDlg();
	virtual ~FrFindEbortEventDlg();

	virtual bool OnInit();

protected:
	void OnCurrentItemOwnerDraw(int param);

	int m_ebortCount[4];
	WPoint m_countPos[4];
	unsigned long m_reserved;

	DECLARE_FRESH_MSGMAP()
};
