#pragma once

#include "frform.h"

class FrArea;
class FrViewer;

class CMissionEvent;
class FrMissionEventDlg;
class FrMissionDetail;

class FrMissionDetail : public FrForm
{
public:
	DECLARE_OBJECT(FrMissionDetail)

	FrMissionDetail();
	virtual ~FrMissionDetail();

protected:
	void OnInitExplaneViewer(int param);

	FrViewer* m_pExplaneViewer;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrMissionEventDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrMissionEventDlg)

	FrMissionEventDlg();
	virtual ~FrMissionEventDlg();

	virtual bool OnInit();

	bool OnDetailDlgResult(int result, FrForm* form);

protected:
	void OnInitDetailButton(int param);
	void OnLButtonDownDetailButton();
	void OnInitDayGiftButton(int param);
	void OnLButtonDownDayGiftButton();
	void OnInitTermGiftButton(int param);
	void OnLButtonDownTermGiftButton();
	void OnInitMissionClearArea(int param);
	void OnOwnerDrawMissionClearArea(int param);
	void OnInitCourseClearArea(int param);
	void OnOwnerDrawCourseClearArea(int param);
	void OnHoverOnCourseClearArea(int param);
	void OnHoverOffCourseClearArea(int param);

private:
	void CheckMissionComplete();
	void CheckCourseComplete();
	void RequestGift(int type);
	void CalcOffset(int index, float& x, float& y);

	FrButton* m_pButton[3];
	FrArea* m_pCourseClearArea;
	FrArea* m_pMissionClearArea;
	const Bitmap* m_pBitmap[3];
	FrMissionDetail* m_pDetailDlg;
	CMissionEvent* m_pMissionEvent;
	unsigned char m_pad983_138[4];	// +0x138: sem leitura/escrita no 983 (00b09630..00b0a650; ctor 00b09900 nao zera)
	WRect m_courseSrcRect;
	WPoint m_missionPos[7];
	WPoint m_textPos[5];
	bool m_bMissionComplete[7];
	bool m_bCourseHover;
	int m_bCourseComplete[15];
	int m_completeCourseNum;

	DECLARE_FRESH_MSGMAP()
};
