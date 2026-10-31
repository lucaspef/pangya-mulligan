#pragma once

#include <string>
#include "frcmdtarget.h"

class FrUserInfoForm;

class CUserInfo : public WSingleton<CUserInfo>, public FrCmdTarget
{
	friend class FrUserInfoForm;
	friend class FrAIInfoForm;

public:
	CUserInfo();
	virtual ~CUserInfo();

	void SetInfo(unsigned long uid, unsigned long guid, bool bCheckTime,
		bool bOption, bool bMsnControl, bool bControlOff, std::string nickname,
		unsigned char aiType = 0, unsigned short aiId = 0);
	void Process(float delta);
	void Clear();
	void Refresh();
	void Close();
	bool IsVisible();
	bool IsRecvedData(unsigned char season);
	bool OnUserInfoFormResult(int result, FrForm* pForm);
	bool OnAiInfoFormResult(int result, FrForm* pForm);

	unsigned long GetGuid() { return m_guid; }

	FrUserInfoForm* GetDlg() { return m_pDlg; }
	FrForm* GetAiInfoDlg() { return m_pAiInfoDlg; }

protected:
	FrUserInfoForm* m_pDlg;
	FrForm* m_pAiInfoDlg;	// +0x30: FrAIInfoForm (983 008a36b0)
	unsigned long m_guid;
	unsigned long m_uid;
	bool m_bCheckTime;
	bool m_bRequest;
	std::string m_nickname;
};
