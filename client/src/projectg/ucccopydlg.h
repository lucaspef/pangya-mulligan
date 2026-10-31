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

struct sItemInfo;
class FrUccCopyConfirmDlg;

class FrUccCopyDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccCopyDlg)

	FrUccCopyDlg();
	virtual ~FrUccCopyDlg();

	void Init();
	void SetItem(sItemInfo* pItemInfo);

protected:
	void OnCharIconInit(int param);
	void OnItemListInit(int param);
	void OnItemListOwnerDraw(int param);
	void OnItemListBtnDown();
	void OnOkBtnInit(int param);
	void OnOkBtnUp();
	void OnCancelBtnInit(int param);
	void OnCancelBtnUp();
	void OnEtcOwnerDraw(int param);

	void BuildItemList();
	bool OnUccCopyConfirmDlgResult(int result, FrForm* form);

	const Bitmap* m_pItemBaseN;
	const Bitmap* m_pItemBaseO;
	const Bitmap* m_pItemBaseD;
	sItemInfo* m_pItemInfo;
	FrArea* m_pCharIcon;
	FrListBox* m_pItemList;
	FrButton* m_pOkBtn;
	FrArea* m_pCancelBtn;
	FrUccCopyConfirmDlg* m_pConfirmDlg;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrUccCopyConfirmDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccCopyConfirmDlg)

	FrUccCopyConfirmDlg();
	virtual ~FrUccCopyConfirmDlg();

	void Init(sItemInfo* pTargetItem, sItemInfo* pSourceItem);

protected:
	void OnIconInit(int param);
	void OnOkBtnInit(int param);
	void OnOkBtnUp();
	void OnCancelBtnInit(int param);
	void OnCancelBtnUp();
	void OnEtcOwnerDraw(int param);

	sItemInfo* m_pTargetItem;
	sItemInfo* m_pSourceItem;
	FrArea* m_pIcon;
	FrButton* m_pOkBtn;
	FrButton* m_pCancelBtn;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrUccCopyCompletedDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccCopyCompletedDlg)

	FrUccCopyCompletedDlg();
	virtual ~FrUccCopyCompletedDlg();

	void Init(unsigned long typeID, const char* uccIndex);

protected:
	void OnOkBtnInit(int param);
	void OnOkBtnUp();
	void OnEtcOwnerDraw(int param);

	unsigned long m_typeID;
	char m_uccIndex[9];
	FrButton* m_pOkBtn;

private:
	DECLARE_FRESH_MSGMAP()
};
