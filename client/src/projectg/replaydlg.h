#pragma once

#include "frform.h"
#include "singleton.h"

class FrReplayControlDlg : public FrForm
{
	DECLARE_OBJECT(FrReplayControlDlg)

	static void Init();

	FrReplayControlDlg();
	virtual ~FrReplayControlDlg();

	void TogglePlayButton();

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	void OnPlayInit(int param);
	void OnPlayBtnUp();
	void OnBeforeShotBtnInit(int param);
	void OnBeforeShotBtnUp();
	void OnNextShotBtnInit(int param);
	void OnNextShotBtnUp();
	void OnExitBtnInit(int param);
	void OnExitBtnUp();
	void OnMinimizeBtnInit(int param);
	void OnMinimizeBtnUp();
	void OnRecordBtnInit(int param);
	void OnRecordBtnUp();

	FrButton* m_pBeforeShot;
	FrButton* m_pNextShot;
	FrButton* m_pExit;
	FrButton* m_pMinimize;
	FrButton* m_pRecord;
	FrButton* m_pPlay;
	unsigned long m_reserved[3];
	bool m_bPlay;
	float m_alpha;
	unsigned long m_unused13c;

	DECLARE_FRESH_MSGMAP()
};

class CReplayControlManager : public WSingleton<CReplayControlManager>
{
public:
	CReplayControlManager();
	virtual ~CReplayControlManager();

	void Init();
	bool CalcReplayControlPos();

	int GetFadeType() { return m_fadeType; }
	void SetFadeType(int type) { m_fadeType = type; }

	WRect GetControlRect() { return m_controlRect; }
	void SetControlRect(WRect rect) { m_controlRect = rect; }

protected:
	float m_ratio;
	unsigned long m_reserved;
	int m_fadeType;
	WRect m_controlRect;
};
