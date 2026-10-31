#pragma once

#include <string>
#include "frform.h"
#include "../shared/globalgamedefine.h"

class FrArea;
class FrEdit;
class FrStatic;

class FrMascotInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrMascotInfoDlg)
	FrMascotInfoDlg();

	void SetMascotInfo(const sMascotInfo& info, WPuppet* pPuppet);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void OnPortraitInit(int param);
	void OnVacationInit(int param);
	void OnLimitInit(int param);
	void OnNameInit(int param);
	void OnTextMessageInit(int param);
	void OnMascotMsgInit(int param);
	void OnOkInit(int param);
	void OnOkBtnUp();
	void OnCancelInit(int param);
	void OnCancelBtnUp();
	bool OnChangeMsgConfirmDlgResult(int result, FrForm* pForm);

	sMascotInfo m_mascotInfo;
	FrArea* m_pPortrait;
	FrArea* m_pVacation;
	FrArea* m_pLimit;
	FrEdit* m_pName;
	FrStatic* m_pTextMessage;
	FrEdit* m_pMascotMsg;
	FrButton* m_pOk;
	FrButton* m_pCancel;
	FrForm* m_pConfirmDlg;
	std::string m_message;
	WPuppet* m_pPuppet;

private:
	DECLARE_FRESH_MSGMAP()
};
