#pragma once

#include "frform.h"

class FrButton;
class FrOptionDlg;

class FrQuitForm : public FrForm
{
	DECLARE_OBJECT(FrQuitForm)

public:
	FrQuitForm();

	void EnableOptionBtn(bool enable);
	WRect GetOptionBtnRect() { return m_pOptionBtn->GetRect(); }

protected:
	virtual void OnDraw();
	virtual void OnProc(const float dt);

	void OnOptionBtnInit(int param);
	void OnOptionBtnUp();
	void OnNextBtnInit(int param);
	void OnNextBtnUp();
	void OnToLobbyBtnInit(int param);
	void OnToLobbyBtnUp();
	void OnExitBtnInit(int param);
	void OnExitBtnUp();
	void OnCloseBtnUp();
	bool OnToLobbyConfirmResult(int result, FrForm* pForm);
	bool OnExitConfirmResult(int result, FrForm* pForm);

	FrButton* m_pOptionBtn;
	FrButton* m_pNextBtn;
	FrButton* m_pToLobbyBtn;
	FrButton* m_pExitBtn;
	FrButton* m_pDescBtn;
	float m_time;

	DECLARE_FRESH_MSGMAP()
};

class CQuitWindow : public WSingleton<CQuitWindow>, public FrCmdTarget
{
public:
	using WSingleton<CQuitWindow>::m_pInstance;
	CQuitWindow();
	virtual ~CQuitWindow();

	void Process();
	bool IsActive();
	bool HasOptionWindow() { return m_pOptionForm ? true : false; }
	void CreateOptionWindow();
	void SetBlockedModes983(int mask) { m_blockedModes983 = mask ? mask : 0x2030; } // JP 983: 006efb00
	void CenterQuitForm983(); // JP 983: 006ef2d0 (recentra a janela de sair na tela nova)

protected:
	bool OnQuitFormResult(int result, FrForm* pForm);
	bool OnOptionResult(int result, FrForm* pForm);

	FrQuitForm* m_pQuitForm;
	FrOptionDlg* m_pOptionForm;
	int m_blockedModes983; // 983 006ef170: dword zerado entre m_pOptionForm e m_bMouseActive (sizeof 0x3c)
	bool m_bMouseActive;
};
