#pragma once

#include "frform.h"

class FrButton;
class FrGaugeBar;
class FrComboBox;
class FrArea;
class FrEdit;
class FrListBox;
class FrStatic;
class FrViewer;
class WTitleFont;
#include "../shared/globalgamedefine.h"

class FrHalloweenEventGiftDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrHalloweenEventGiftDlg)

	FrHalloweenEventGiftDlg();
	virtual ~FrHalloweenEventGiftDlg();
	virtual bool OnInit();

protected:
	void OnInitFrame01(int);
	void OnInitFrameCover(int);
	DECLARE_FRESH_MSGMAP()

private:
	FrArea* m_pFrame01;
	FrArea* m_pFrameCover;
};

class FrHalloweenEventDescDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrHalloweenEventDescDlg)

	FrHalloweenEventDescDlg();
	virtual ~FrHalloweenEventDescDlg();
	virtual bool OnInit();

protected:
	void OnInitDescView(int);
	DECLARE_FRESH_MSGMAP()

private:
	bool m_bScroll;
	FrViewer* m_pDescView;
};

class FrHalloweenEventDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrHalloweenEventDlg)

	FrHalloweenEventDlg();
	virtual ~FrHalloweenEventDlg();
	virtual bool OnInit();

	void SetMapImage(eMapType type, const char* on, const char* off);
	void SetMapImageRect(eMapType type, WRect& rect);
	void SetMapFlagVector(eMapType type);
	void DrawMapImage();

	bool OnDescDlgResult(int result, FrForm* form);

protected:
	void OnInitDescButton(int);
	void OnLBDownDescButton();
	void OnMapIconOwnerDraw();
	void OnCurrentItemOwnerDraw(int);

	DECLARE_FRESH_MSGMAP()

private:
	FrButton* m_pDescButton;
	unsigned long m_reserved;
	const Bitmap* m_pTicketFrame;
	FrHalloweenEventDescDlg* m_pDescDlg;
	const Bitmap* m_pMark;
	WRect m_mapImageRect[21]; // 983: 21
	const Bitmap* m_pMapImage[21][2]; // 983: 21
	WRect m_mapMarkRect[21]; // 983: 21
	unsigned long m_mapFlagVector[21]; // 983: 21
	unsigned long m_mapFlag;
	int m_itemCount[4];
	WPoint m_itemCountPos[4];
	WPoint m_reservedPos;
};
