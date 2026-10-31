#pragma once

#include <list>
#include <string>
#include "frform.h"

class FrGiftDlg;
class CPost_SendDlg;
class FrListBox;
class Bitmap;

class FrFriendDlg : public FrForm
{
	DECLARE_OBJECT(FrFriendDlg)

	FrFriendDlg();

	void SetParent(FrGiftDlg* pParent);
	void SetParent2(CPost_SendDlg* pParent);
	unsigned long GetUid() const { return m_uid; }
	bool IsSelectPerson();
	bool HaveFriends();
	void UpdateFriendList();

protected:
	void OnFriendListInit(int param);
	void OnFriendListOwnerDraw(int param);
	void OnFriendListLBtnUp();
	void OnFriendListRBtnUp();
	void OnFriendListDblClick();
	void OnCancelBtnUp();

	DECLARE_FRESH_MSGMAP()

private:
	CPost_SendDlg* m_pPostSendDlg;
	FrGiftDlg* m_pGiftDlg;
	FrListBox* m_pFriendList;
	std::list<sFriend> m_friendList;
	unsigned long m_uid;
	const Bitmap* m_pSexIcon[2];
	std::string m_prevNick;
	std::string m_prevConfirmNick;
};
