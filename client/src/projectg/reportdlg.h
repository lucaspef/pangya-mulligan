#pragma once

#include "frform.h"

class FrEmoticonDlg;
class FrEdit;
class FrComboBox;
class FrButton;

class FrReportDlg : public FrForm
{
	DECLARE_OBJECT(FrReportDlg)

	FrReportDlg();
	virtual ~FrReportDlg();

protected:
	virtual void OnProc(const float deltaTime);
	virtual void OnOK();

	void OnIdInit(int param);
	bool OnIdEnterKey(int param);
	void OnReasonInit(int param);
	void OnOKInit(int param);
	void OnEmoticonBtnUp();

	bool OnEmoticonResult(int result, FrForm* pForm);

	FrEdit* m_pId;
	FrComboBox* m_pReason;
	FrButton* m_pOK;
	FrEmoticonDlg* m_pEmoticonDlg;

	DECLARE_FRESH_MSGMAP()
};
