#pragma once

#include "frform.h"
#include "../shared/globalgamedefine.h"

class FrArea;
class FrEdit;
class FrStatic;

class FrCaddieInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrCaddieInfoDlg)

	FrCaddieInfoDlg();

	void SetCaddieInfo(const sCaddieInfo& info);
	void HideContractControls();
	void EndContract(int result);
	sCaddieInfo GetCaddieInfo() { return m_caddieInfo; }

protected:
	virtual bool OnInit();

	void OnPortraitInit(int param);
	void OnVacationInit(int param);
	void OnLimitInit(int param);
	void OnNameInit(int param);
	void OnContractInit(int param);
	void OnCaddieFeeInit(int param);
	void OnSignUpInit(int param);
	void OnOkInit(int param);
	void OnCancelInit(int param);
	void OnWarningButtonInit(int param);
	void OnWarningTextInit(int param);
	void OnSignUpBtnUp();
	void OnOkBtnUp();
	void OnCancelBtnUp();
	bool OnSignUpConfirmDlgResult(int result, FrForm* form);
	void SendCaddieWarningCheckOption();

	sCaddieInfo m_caddieInfo;
	FrArea* m_pPortrait;
	FrArea* m_pVacation;
	FrArea* m_pLimit;
	FrEdit* m_pName;
	FrEdit* m_pMessage;
	FrStatic* m_pContract;
	FrEdit* m_pCaddieFee;
	FrButton* m_pSignUp;
	FrButton* m_pOk;
	FrButton* m_pCancel;
	FrButton* m_pWarningButton;
	FrStatic* m_pWarningText;
	FrForm* m_pContractDlg;

private:
	DECLARE_FRESH_MSGMAP()
};
