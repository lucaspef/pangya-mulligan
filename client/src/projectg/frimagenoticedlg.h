#pragma once

#include "frform.h"

class FrButton;
class FrArea;

class FrImageNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrImageNoticeDlg)

	FrImageNoticeDlg();
	virtual ~FrImageNoticeDlg();

	bool SetOkButton(WRect& rect, bool bRelative);
	bool SetCloseButton(WRect& rect, bool bRelative);
	void SetDlgSize(WRect& rect);
	bool SetBackGroundImage(int x, int y, WRect& rect, const char* image,
		bool bRelative);

protected:
	void OnInitCloseButton(int param);
	void OnInitOkButton(int param);
	void OnLBDownOkButton();
	void OnInitLeftTop(int param);
	void OnInitCenterTop(int param);
	void OnInitRightTop(int param);
	void OnInitLeftMiddle(int param);
	void OnInitCenterMiddle(int param);
	void OnInitRightMiddle(int param);
	void OnInitLeftBottom(int param);
	void OnInitCenterBottom(int param);
	void OnInitRightBottom(int param);

	DECLARE_FRESH_MSGMAP()

private:
	FrButton* m_pCloseButton;
	FrButton* m_pOkButton;
	FrArea* m_pBackGround[3][3];
};
