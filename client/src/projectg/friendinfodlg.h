#pragma once

#include "frform.h"

struct sFriend;
class FrEdit;
class FrButton;
class FrFriendInfoDlg : public FrForm
{
	DECLARE_OBJECT(FrFriendInfoDlg)

	FrFriendInfoDlg()
		: m_pID(NULL),
		  m_pNick(NULL),
		  m_pSex(NULL),
		  m_pServer(NULL),
		  m_pChannel(NULL),
		  m_pIgnore(NULL)
	{
	}

	void SetInfo(sFriend* pFriend);

protected:
	virtual bool OnInit();

	void OnIDInit(int param);
	void OnNickInit(int param);
	void OnSexInit(int param);
	void OnServerInit(int param);
	void OnChannelInit(int param);
	void OnIgnoreInit(int param);
	void OnWhisperBtnUp();
	void OnIgnoreBtnUp();
	void OnCloseBtnUp();

	DECLARE_FRESH_MSGMAP()

private:
	FrEdit* m_pID;
	FrEdit* m_pNick;
	FrEdit* m_pSex;
	FrEdit* m_pServer;
	FrEdit* m_pChannel;
	FrButton* m_pIgnore;
};
