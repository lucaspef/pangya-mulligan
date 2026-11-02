#pragma once

#include "frform.h"
#include <string>

enum eLoginState;

class FrLoginDlg : public FrForm
{
	DECLARE_OBJECT(FrLoginDlg)

	FrLoginDlg();
	virtual ~FrLoginDlg();

	void SetNoticeMsg(const char* msg);
	static void SetState(eLoginState state);
	static eLoginState GetState();
	static void SetErrorState983(int code); // 983 00ada7d0: estado 0x1c com o texto do ErrorCodeInfo
	static const char* GetErrorText983(); // 983 00ad9880
	void EnableLoginControls(bool bEnable);
	void BeginConnect();
	void Login();

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);

	bool OnJoinDlgResult(int result, FrForm* form);
	bool OnFindDlgResult(int result, FrForm* form);
	bool OnDisconPrevSession(int result, FrForm* form);

	void OnIdInit(int param);
	void OnPasswordInit(int param);
	bool OnEnterKey(int param);
	void OnAutoIdInit(int param);
	void OnAutoIdBtnUp();
	void OnViewInit(int param);
	const char* QueryWelcomeMessage();
	void OnLoginInit(int param);
	void OnLoginBtnUp();
	void OnJoinBtnInit(int param);
	void OnJoinJpBtnInit(int param);
	void OnQuitBtnInit(int param);
	void OnJoinBtnUp();
	void OnQuitBtnUp();
	void ResetInput(const char* msg);
	void OnHideStaticInit(int param);
	void OnParanAreaInit(int param);
	void OnHideAreaInit(int param);
	void OnHideAreaDraw();
	void OnLoginNoticeInit(int param);
	void OnHideBtnInit(int param);
	void OnLoginFbInit(int param);
	void OnLoginFbBtnUp();
	void OnLoginImgInit(int param);

	static eLoginState m_connState;

	std::string m_id;
	std::string m_passwd;
	FrEdit* m_pId;
	FrEdit* m_pPassword;
	FrEdit* m_pView;
	FrButton* m_pLogin;
	FrButton* m_pLoginFb; // 983 +0x158 (loginfb, 00ad9600)
	FrButton* m_pAutoId; // 983 +0x15c (auto_id, 00ad9e10)
	unsigned long m_reserved160; // 983: sem uso
	unsigned long m_reserved164; // 983 ctor 00ada4fe: = 0, sem outro uso
	unsigned long m_reserved168; // 983 ctor 00ada504: = 0, sem outro uso
	FrButton* m_pJoin; // 983 +0x16c (OnJoinBtnInit 00ad96c9)
	int m_failCount; // 983 +0x170 (inc 00adb323, cmp 4 00adb371)
	float m_reserved174; // 983 ctor 00ada4f8: fldz, sem outro uso
	unsigned long m_reserved178[2]; // 983 new 0x180 (005953e0)

private:
	bool PrepareWebLogin();
	void FinishLogin(); // 983 00ad98a0
	void BeginClientLogin();

	static sFRESH_ENTRY _MsgEntries[];

protected:
	static sFRESH_MSGMAP _MsgMap;
	virtual const sFRESH_MSGMAP* GetMessageMap() const;
};

namespace
{
	struct _ValidId
	{
		bool bInvalid;

		_ValidId()
			: bInvalid(false)
		{
		}
		void operator()(char c)
		{
			if (!isalnum(c))
				bInvalid = true;
		}
	};

	class cNTAUTHPARAM
	{
	public:
		cNTAUTHPARAM(const std::string& _id, const std::string& _pwd)
			: id(_id), pwd(_pwd)
		{
		}

		std::string id;
		std::string pwd;
	};
}
