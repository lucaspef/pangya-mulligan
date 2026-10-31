#pragma once

#include "frform.h"

class FrEdit;
class FrButton;
class FrEmoticonDlg;

class FrAddFriendDlg : public FrForm
{
	DECLARE_OBJECT(FrAddFriendDlg)

	FrAddFriendDlg()
		: m_pEditNick(NULL),
		  m_pDispNick(NULL),
		  m_findUID(0xffffffff),
		  m_elapsed(0),
		  m_bFocused(false),
		  m_pEmoticonDlg(NULL)
	{
	}

	void SetNick(const char* nick);
	void SetTargetNick(const char* nick);
	void SetFindUID(unsigned long uid);
	void CheckRequestNickname(const char* nick);

protected:
	virtual bool OnInit(); // 983: 00ac57b0
	virtual void OnProc(const float delta);
	virtual void OnCancel();

	void OnEditNiclInit(int param);
	void OnDispNickInit(int param);
	void OnEmoticonInit(int param);
	bool OnEditNickEnterKey(int param);
	void OnEditNickDown();
	void OnCheckBtnUp();
	void OnOKBtnUp();
	void OnEmoticonUp();

	FrEdit* m_pEditNick;
	FrEdit* m_pDispNick;
	unsigned long m_findUID;
	float m_elapsed;
	bool m_bFocused;
	FrEmoticonDlg* m_pEmoticonDlg;
	FrButton* m_pEmoticon;

private:
	bool OnEmoticonResult(int result, FrForm* pForm);

	DECLARE_FRESH_MSGMAP()
};
