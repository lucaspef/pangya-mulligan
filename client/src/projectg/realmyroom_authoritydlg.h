#pragma once

#include <string>
#include "frform.h"

class FrListBox;
class FrEdit;
class FrButton;
class Bitmap;

class CRealMyRoom_AuthorityDlg : public FrForm
{
	DECLARE_OBJECT(CRealMyRoom_AuthorityDlg)

	CRealMyRoom_AuthorityDlg();
	virtual ~CRealMyRoom_AuthorityDlg();

	bool GetAuthSetting();
	void GetPassWord();

protected:
	void OnAuth_KindPowerInit(int param);
	void OnAuth_KindPowerOwnerdraw(int param);
	void OnAuth_KindPowerBtnDown();
	void OnAuth_PasswordInit(int param);
	void OnAuthEditBgInit(int param);
	void OnInitStorageLock(int param);
	void OnLBDownStorageLock();
	void OnInitStorageUnLock(int param);
	void OnLBDownStorageUnLock();
	void OnInitStorageChangePass(int param);
	void OnLBDownStorageChangePass();
	bool OnAuth_PWEnterKey(int param);
	void OnLoadImage();

	virtual void OnOK();
	virtual void OnCancle();

	FrListBox* m_pKindList;
	FrEdit* m_pPassword;
	unsigned long m_reserved;
	const Bitmap* m_pKindBmp[2][3];
	bool m_bAuthSetting;
	bool m_bStorageLock;
	FrButton* m_pStorageLock;
	FrButton* m_pStorageUnLock;
	FrButton* m_pStorageChangePass;

private:
	DECLARE_FRESH_MSGMAP()
};

class CRealMyRoom_EnterSetting : public FrForm
{
	DECLARE_OBJECT(CRealMyRoom_EnterSetting)

	CRealMyRoom_EnterSetting();
	virtual ~CRealMyRoom_EnterSetting();

	std::string GetPassWord();

protected:
	void OnInputPwInit(int param);
	bool OnInputPwEnterKey(int param);

	virtual void OnOK();
	virtual void OnCancle();

	FrEdit* m_pInputPw;
	unsigned long m_reserved;
	std::string m_password;

private:
	DECLARE_FRESH_MSGMAP()
};
