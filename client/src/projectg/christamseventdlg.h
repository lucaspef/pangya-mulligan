#pragma once

#include <vector>
#include "frform.h"

class FrButton;
class FrArea;
class WTitleFont;

class FrChristmasEventDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrChristmasEventDlg)

	FrChristmasEventDlg();
	virtual ~FrChristmasEventDlg();

private:
	virtual bool OnInit();
	virtual void OnProc(const float delta);
	virtual void OnDraw();

protected:
	void OnCloseBtnInit(int param);
	void OnCloseBtnUp();
	void OnCaptionInit(int param);
	void OnXmasBtn1Init(int param);
	void OnXmasBtn2Init(int param);
	void OnXmasBtn3Init(int param);
	void OnXmasEx1Init(int param);
	void OnXmasEx2Init(int param);
	void OnXmasEx3Init(int param);
	void OnSocks1AreaInit(int param);
	void OnSocks2AreaInit(int param);
	void OnSnowFallDraw();

	tagWTITLEFONT m_titleFontInfo;
	WTitleFont* m_pTitleFont;
	FrButton* m_pXmasBtn[3];
	FrArea* m_pXmasEx[3];
	bool m_bXmasExShown[3];

private:
	DECLARE_FRESH_MSGMAP()
};
