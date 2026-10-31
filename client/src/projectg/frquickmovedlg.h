#pragma once

#include "frform.h"

class FrViewer;
class FrButton;

class FrQuickMoveDlg : public FrForm
{
	DECLARE_OBJECT(FrQuickMoveDlg)

	FrQuickMoveDlg();
	virtual ~FrQuickMoveDlg();

	void SetDescImage(const char* image);

protected:
	void OnInitDescViewer(int param);
	void OnLBDownViewer();
	void OnInitBtnPrev(int param);
	void OnInitBtnNext(int param);
	void OnInitBtnMove(int param);
	void OnLBDownBtnPrev();
	void OnLBDownBtnNext();
	void OnLBDownBtnMove();

	DECLARE_FRESH_MSGMAP()

private:
	FrViewer* m_pDescViewer;
	char m_descImage[64];
	unsigned int m_page;
	FrButton* m_pBtnPrev;
	FrButton* m_pBtnNext;
	FrButton* m_pBtnMove;
};
