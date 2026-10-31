#pragma once

#include "frform.h"

class FrListBox;
class FrFinalRankDlg : public FrForm
{
	DECLARE_OBJECT(FrFinalRankDlg)

	FrFinalRankDlg();

protected:
	void OnRankLeftInit(int param);
	void OnRankLeftOwnerDraw(int param);
	void OnRankRightInit(int param);
	void OnRankRightOwnerDraw(int param);

	FrListBox* m_pRankLeft;
	FrListBox* m_pRankRight;

	DECLARE_FRESH_MSGMAP()
};
