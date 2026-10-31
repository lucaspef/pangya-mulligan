#pragma once

#include "frform.h"

class FrAvatarUserListDlg : public FrForm
{
	DECLARE_OBJECT(FrAvatarUserListDlg)

	FrAvatarUserListDlg();
	virtual ~FrAvatarUserListDlg();

	void UpdateUserList();

protected:
	virtual void OnProc(const float delta);

	void OnUserListInit(int param);
	void OnUserListOwnerDraw(int param);
	void OnUserListLBtnDown();
	void OnUserListRBtnUp();

	unsigned long m_selectedUid;
	class FrListBox* m_pUserList;
	float m_alpha;
	const Bitmap* m_pIcon[6];
	const Bitmap* m_pMannerIcon[2];
	const Bitmap* m_pAngelIcon[2];
	const Bitmap* m_pInGameIcon;

	DECLARE_FRESH_MSGMAP()
};
