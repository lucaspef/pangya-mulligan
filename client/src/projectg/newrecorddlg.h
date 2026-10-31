#pragma once

#include "frform.h"

class FrListBox;

class FrNewRecordDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrNewRecordDlg)

protected:
	void OnCourseListInit(int param);
	void OnCourseListOwnerDraw(int param);

	FrListBox* m_pCourseList;
	int m_unused114;

	DECLARE_FRESH_MSGMAP()
};
