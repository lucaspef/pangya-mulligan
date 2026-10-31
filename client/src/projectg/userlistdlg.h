#pragma once

#include <map>
#include "frform.h"

class FrButton;
class FrListBox;
#include "../shared/globalgamedefine.h"
#include "addfrienddlg.h"

class FrUserListDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrUserListDlg)
	FrUserListDlg();

	void MakeUserList(std::map<unsigned long, sBriefUserInfo>& userList);
	void SortUserList();
	void EnableRightButton(bool bEnable);
	void AddUser(void* pData);
	void DelUser(void* pData, int index);
	void SetAddFriendDlgNick(const char* nick, const char* msg);
	bool OnAddFriendDlgResult(int result, FrForm* form);
	void CloseInviteDelayDlg();
	void SetRequestFriendData(const char* nick, unsigned long uid);

	bool IsOpenAddFriendDlg() const { return m_pAddFriendDlg ? true : false; }

protected:
	virtual void OnProc(const float dt);

	void OnUserListInit(int param);
	void OnUserListOwnerDraw(int param);
	void OnUserListLBtnDown();
	void OnUserListRBtnUp();
	void OnUserListDClick();
	void OnSortBtnInit(int param);
	void OnSexBtnUp();
	void OnLevelBtnUp();
	void OnGuildBtnUp();
	void OnIdBtnUp();
	void OnInviteBtnInit(int param);
	void OnInviteBtnUp();
	void OnFriendBtnUp();
	void OnWhisperBtnInit(int param);
	void OnWhisperBtnUp();

	float m_inviteDelay;
	FrForm* m_pInviteDelayDlg;
	FrAddFriendDlg* m_pAddFriendDlg;
	FrButton* m_pWhisperBtn;
	FrButton* m_pInviteBtn;
	FrListBox* m_pUserList;
	unsigned long m_selOid;
	unsigned long m_selUid;
	const Bitmap* m_pSexIcon[6];
	const Bitmap* m_pMannerIcon[2];
	const Bitmap* m_pAngelIcon[2];
	const Bitmap* m_pInGameIcon;
	int m_sortType;
	bool m_bSortReverse;

private:
	DECLARE_FRESH_MSGMAP()
};
