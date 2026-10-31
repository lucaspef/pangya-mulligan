#pragma once

#include "frform.h"
class FrViewer;
class FrAvatarTipDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrAvatarTipDlg)

	FrAvatarTipDlg();
	virtual ~FrAvatarTipDlg();

protected:
	void OnTipInit(int param);
	void OnPrevInit(int param);
	void OnNextInit(int param);
	void OnCloseInit(int param);
	void OnPrevBtnUp();
	void OnNextBtnUp();
	void OnCloseBtnUp();

	FrViewer* m_pTip;
	FrButton* m_pPrev;
	FrButton* m_pNext;
	FrButton* m_pCancel;
	int m_tipIndex;

	DECLARE_FRESH_MSGMAP()
};
