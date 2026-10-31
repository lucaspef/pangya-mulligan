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
class CExhibition;
class CPartTidList;
class FrApproachResultDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrApproachResultDlg)

	FrApproachResultDlg();
	virtual ~FrApproachResultDlg();

	virtual void OnProc(const float delta);

	void ClearVariables();
	void ShowPet();
	void SetApproachEnd(bool bEnd);
	bool GetApproachEnd() const;

protected:
	virtual bool OnInit();

	void OnLeftAreaInit(int param);
	void OnLeftAreaOwnerDraw(int param);
	void OnLeftListInit(int param);
	void OnLeftListOwnerDraw(int param);
	void OnRightAreaInit(int param);
	void OnRightAreaOwnerDraw(int param);
	void OnRightListInit(int param);
	void OnRightListOwnerDraw(int param);

	FrArea* m_pLeftArea;
	FrListBox* m_pLeftList;
	const Bitmap* m_pBack;
	const Bitmap* m_pTab1;
	const Bitmap* m_pTab2;
	CExhibition* m_pExhibition;
	CPartTidList* m_pPartTidList;
	FrListBox* m_pRightList;
	FrArea* m_pRightArea;
	const Bitmap* m_pTab3;
	const Bitmap* m_pTab4;
	WTitleFont* m_pBigFont;
	WTitleFont* m_pSmallFont;
	const Bitmap* m_pTreasureTitle;
	const Bitmap* m_pRankGold;
	const Bitmap* m_pRankSilver;
	const Bitmap* m_pRankBronze;
	const Bitmap* m_pTreasureBox;
	const Bitmap* m_pGoodLuck;
	bool m_bApproachEnd;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrAppTreasureGiftDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrAppTreasureGiftDlg)

	FrAppTreasureGiftDlg();
	virtual ~FrAppTreasureGiftDlg();

private:
	DECLARE_FRESH_MSGMAP()
};

class FrApproachEndDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrApproachEndDlg)

	FrApproachEndDlg();
	virtual ~FrApproachEndDlg();

protected:
	void OnDrawAreaInit(int param);
	void OnDrawAreaOwnerDraw(int param);
	void OnListInit(int param);
	void OnListOwnerDraw(int param);

	FrArea* m_pDrawArea;
	FrListBox* m_pList;
	const Bitmap* m_pResult1;
	const Bitmap* m_pResult2;
	const Bitmap* m_pResult3;
	const Bitmap* m_pResult4;

private:
	DECLARE_FRESH_MSGMAP()
};
