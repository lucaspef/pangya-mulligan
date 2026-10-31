#pragma once

#include "frform.h"
class FrEdit;
class FrReportDlg;
class FrAvatarChatLogDlg : public FrForm
{
	DECLARE_OBJECT(FrAvatarChatLogDlg)

	FrAvatarChatLogDlg();
	virtual ~FrAvatarChatLogDlg();

	void AddChatMsg(const char* msg, int color);

protected:
	void OnChatViewInit(int param);
	void OnHideChatInit(int param);
	void OnShowChatInit(int param);
	void OnBtnHideChatUp();
	void OnBtnShowChatUp();
	void OnBtnSaveChatUp();
	void OnBtnAccuseUp();
	void OnBtnCloseDlgUp();

	void* m_pUnused110;
	FrButton* m_pShowChat;
	FrButton* m_pHideChat;
	void* m_pUnused11c;
	void* m_pUnused120;
	FrEdit* m_pChatView;
	FrReportDlg* m_pReportDlg;

private:
	bool OnReportDlgResult(int result, FrForm* form);

	DECLARE_FRESH_MSGMAP()
};
