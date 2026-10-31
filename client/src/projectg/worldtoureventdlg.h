#pragma once

#include "frform.h"
#include "../shared/globalgamedefine.h"

class FrWorldTourEventDlg;
class FrWorldTourEventGiftDlg;
class FrWorldTourEventDescDlg;

class FrWorldTourEventDlg : public FrForm
{
	friend class FrWorldTourEventGiftDlg;

	DECLARE_OBJECT(FrWorldTourEventDlg)

	FrWorldTourEventDlg();
	virtual ~FrWorldTourEventDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void ClearVariables();
	void SetMapFlag(unsigned long flag);
	void DrawMapImage();

	bool OnDescDlgResult(int result, FrForm* pForm);
	bool OnGiftDlgResult(int result, FrForm* pForm);

protected:
	void SetMapFlagVector(eMapType type);
	void SetMapImageRect(eMapType type, WRect& rect);
	void SetMapImage(eMapType type, const char* on, const char* off);

	void OnMapIconOwnerDraw();
	void OnInitGiftImageLeftTop(int);
	void OnInitGiftImageRightTop(int);
	void OnInitGiftImageLeftBottom(int);
	void OnInitGiftImageRightBottom(int);
	void OnInitDescButton(int);
	void OnInitCourseButton(int);
	void OnInitGiftButtonCover(int);
	void OnInitGiftButton(int);

	void OnLBDownCourseButton();
	void OnLBDownDescButton();
	void OnLBDownGiftButton();

	static const int MAP_COUNT = 22; // 983: 22 (era 17)

	float m_fElapsed;
	unsigned char m_reserved;
	unsigned long m_mapFlag;
	unsigned long m_mapFlagList[MAP_COUNT];
	WRect m_mapRect[MAP_COUNT];
	const Bitmap* m_mapImage[MAP_COUNT][2];
	WRect m_markRect[MAP_COUNT];
	const Bitmap* m_pMarkImage;
	FrArea* m_pGiftButtonCover;
	FrButton* m_pDescButton;
	FrButton* m_pCourseButton;
	FrButton* m_pGiftButton;
	FrArea* m_pGiftImage[4];
	FrWorldTourEventGiftDlg* m_pGiftDlg;
	FrWorldTourEventDescDlg* m_pDescDlg;

	DECLARE_FRESH_MSGMAP()
};

class FrWorldTourEventDescDlg : public FrForm
{
	DECLARE_OBJECT(FrWorldTourEventDescDlg)

	FrWorldTourEventDescDlg();
	virtual ~FrWorldTourEventDescDlg();

protected:
	void OnInitDescView(int);

	FrViewer* m_pDescView;

	DECLARE_FRESH_MSGMAP()
};

class FrWorldTourEventGiftDlg : public FrForm
{
	DECLARE_OBJECT(FrWorldTourEventGiftDlg)

	FrWorldTourEventGiftDlg();
	virtual ~FrWorldTourEventGiftDlg();

protected:
	void OnInitYesButton(int);
	void OnInitNoButton(int);
	void OnLBDownYesButton();
	void OnLBDownNoButton();

	FrButton* m_pYesButton;
	FrButton* m_pNoButton;

	DECLARE_FRESH_MSGMAP()
};
