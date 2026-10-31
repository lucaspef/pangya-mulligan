#pragma once

#include <list>
#include "frform.h"
#include "../shared/tikimagicboxtable.h"

class FrButton;
class FrListBox;

class FrTikiMagicBoxDlg : public FrForm
{
	DECLARE_OBJECT(FrTikiMagicBoxDlg)

	FrTikiMagicBoxDlg();
	virtual ~FrTikiMagicBoxDlg();

	int InsertMaterial(const sTikiMagicBoxMtr& mtr);
	int DeleteMaterial(const sTikiMagicBoxMtr& mtr);

protected:
	void DrawBitmapOnDlg(float x, float y, const char* name);
	void PrintTextOnDlg(float x, float y, const char* text,
		unsigned long color);
	void BindButton(FrButton*& pButton, int param);
	void BindListBox(FrListBox*& pListBox, int param);

	void OnBackImageOwnerDraw(int param);
	void OnMyItemListInit(int param);
	void OnMyItemListOwnerDraw(int param);
	void OnMyItemListDBClick();
	void OnMyItemListLBDown();
	void OnMyItemListLBUp();
	void OnMtrListInit(int param);
	void OnMtrListOwnerDraw(int param);
	void OnMtrListDBClick();
	void OnMtrListLBDown();
	void OnMtrListLBUp();
	void OnCategoryListInit(int param);
	void OnCategoryListOwnerDraw(int param);
	void OnCategoryListLBDown();
	void OnCharacterListInit(int param);
	void OnCharacterListOwnerDraw(int param);
	void OnCharacterListLBDown();
	void OnHelpBtnInit(int param);
	void OnHelpBtnLBUp();
	void OnCloseBtnInit(int param);
	void OnCloseBtnLBUp();
	void OnCharTabInit(int param);
	void OnCharTabLBUp();
	void OnItemTabInit(int param);
	void OnItemTabLBUp();
	void OnInsertBtnInit(int param);
	void OnInsertBtnLBUp();
	void OnMixBtnInit(int param);
	void OnMixBtnLBUp();
	void OnCancelInit(int param);
	void OnCancelLBUp();

private:
	static const int MAX_MATERIAL = 4;

	int m_mtrIndex;
	std::list<FrWnd*> m_ctrlList;
	FrListBox* m_pMyItemList;
	FrListBox* m_pMtrList;
	FrListBox* m_pCategoryList;
	FrListBox* m_pCharacterList;
	FrButton* m_pHelpBtn;
	FrButton* m_pCloseBtn;
	FrButton* m_pCharTab;
	FrButton* m_pItemTab;
	FrButton* m_pInsertBtn;
	FrButton* m_pMixBtn;
	FrButton* m_pCancel;

	DECLARE_FRESH_MSGMAP()
};
