#pragma once

#include "frform.h"

class FrUccDrawDlg;
class FrArea;
class FrEdit;

class FrUccSetInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUccSetInfoDlg)

	FrUccSetInfoDlg();

	void SetParent(FrUccDrawDlg* pParent);

protected:
	void OnNameInit(int param);
	bool OnNameEnterKey(int param);
	void OnOkBtnInit(int param);
	void OnOkBtnUp();
	void OnCancelBtnInit(int param);
	void OnCancelBtnUp();
	void OnIconAreaInit(int param);
	void OnIconAreaOwnerDraw(int param);

	FrEdit* m_pName;
	FrButton* m_pOkBtn;
	FrButton* m_pCancelBtn;
	FrArea* m_pIconArea;
	FrUccDrawDlg* m_pParent;
	Bitmap m_thumbnail;

	DECLARE_FRESH_MSGMAP()
};
