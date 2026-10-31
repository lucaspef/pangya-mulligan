#pragma once

#include "frform.h"

class FrListBox;

class FrMapSelectDlg : public FrForm
{
	DECLARE_OBJECT(FrMapSelectDlg)

	FrMapSelectDlg();
	virtual ~FrMapSelectDlg();

	void SetSelectMap(int map);

protected:
	void OnSelectInit(int param);
	void OnSelectOwnerDraw(int param);
	void OnSelectBtnUp();

	FrListBox* m_pMapList;
	int m_selectMap;
	bool m_bSendChange;
	bool m_bNoRandomMap;

	DECLARE_FRESH_MSGMAP()
};
