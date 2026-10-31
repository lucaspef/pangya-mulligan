#pragma once

#include "frform.h"

class FrArea;
class FrEdit;

class FrDeleteItemDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrDeleteItemDlg)

	FrDeleteItemDlg();

	void SetDelItem(unsigned long typeId, unsigned long count);

protected:
	void OnItemInit(int param);
	void OnDescInit(int param);
	void OnWarnInit(int param);
	void OnItemNumInit(int param);
	void OnNumInit(int param);
	void OnNumPrevInit(int param);
	void OnNumNextInit(int param);
	void OnItemOwnerDraw(int param);
	bool OnNumEnterKey(int param);
	void OnNumPrevBtnUp();
	void OnNumNextBtnUp();
	void OnCheckBtnUp();
	bool OnConfirmResult(int result, FrForm* form);

	FrArea* m_pItem;
	FrEdit* m_pDesc;
	FrEdit* m_pItemNum;
	FrEdit* m_pNum;
	FrButton* m_pNumPrev;
	FrButton* m_pNumNext;
	unsigned long m_typeId;
	unsigned long m_count;

	DECLARE_FRESH_MSGMAP()
};
