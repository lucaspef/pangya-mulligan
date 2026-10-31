#pragma once

#include "frform.h"

class FrViewer;
class FrListBox;

static const char* g_szArrayName[] = { (0, "\xb4\xa9\xb8\xae"),
	(0, "\xc7\xcf\xb3\xaa"), (0, "\xbe\xc6\xc0\xfa"),
	(0, "\xbc\xbc\xbd\xc7\xb8\xae\xbe\xc6"), (0, "\xb8\xc6\xbd\xba"),
	(0, "\xc4\xed"), (0, "\xbe\xc6\xb8\xb0"), (0, "\xc4\xab\xc1\xee"),
	(0, "\xb7\xe7\xbd\xc3\xbe\xc6"), (0, "\xb3\xda"),

	(0, "\xc5\xac\xb7\xb4"),

	(0, "\xbe\xc6\xc0\xcc\xc5\xdb"),

	(0, "\xb1\xe2\xc5\xb8") };

class CStandingInLine : public FrForm
{
	DECLARE_OBJECT(CStandingInLine)

public:
	CStandingInLine();
	virtual ~CStandingInLine();

	int GetSelectNum() { return m_selectNum; }

protected:
	void OnView_TotalInit(int param);
	void OnView_TotalOwnerDraw(int param);
	void OnView_TotalButtonUp();

	void OnArrayListInit(int param);
	void OnArrayListOwnerDraw(int param);
	void OnArrayListBtnDown();
	void OnArrayListRBtnDown();

	FrViewer* m_pViewTotal;
	FrListBox* m_pArrayList;
	int m_selectNum;

	DECLARE_FRESH_MSGMAP()
};
