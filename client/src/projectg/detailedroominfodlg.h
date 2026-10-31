#pragma once

#include "frform.h"

struct sRoomUserInfo;
struct sGPInfo983;
#include <list>

#include "../shared/globalgamedefine.h"

class FrDtRoomDlg : public FrForm
{
	DECLARE_OBJECT(FrDtRoomDlg)

	enum eHeader
	{
		eHEADER_LEVEL,
		eHEADER_ID,
		eHEADER_STATUS,
	};

	FrDtRoomDlg();

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void SetRoomInfo(const sRoomInfo& info);
	void EnableJoinBtn(bool bEnable);
	void SetGrandPrix983(unsigned long typeId); // 00ace830

protected:
	void OnGameDescInit(int param);
	void UpdateGameDesc983(); // 00ace870: as 4 linhas da descricao (OnGameDescInit do 645 em diante)
	void ShowGrandPrix983(); // 00acfcf0: descricao do Grand Prix (m_pGrandPrix983)
	void OnJoinBtnInit(int param);
	void OnJoinBtnUp();
	void OnUserInfoBtnInit(int param);
	void OnUserInfoBtnUp();
	void OnGalleryBtnInit(int param);
	void OnGalleryBtnUp();
	void OnLevelHdrInit(int param);
	void OnLevelHdrDown();
	void OnIdHdrInit(int param);
	void OnIdHdrDown();
	void OnStatusHdrInit(int param);
	void OnStatusHdrDown();
	void OnHeaderLBtnDown(eHeader header);
	void OnUserListInit(int param);
	void OnUserListOwnerDraw(int param);
	void OnUserListLBtnDown();
	void OnUserListRBtnUp();
	bool OnPasswordDlgResult(int result, FrForm* form);

	std::list<void*> m_delList;
	FrEdit* m_pGameDesc;
	FrButton* m_pJoinBtn;
	FrButton* m_pUserInfoBtn;
	FrButton* m_pGalleryBtn;
	FrButton* m_pLevelHdr;
	FrButton* m_pIdHdr;
	FrButton* m_pStatusHdr;
	FrListBox* m_pUserList;
	sRoomInfo m_roomInfo;
	sRoomUserInfo* m_pSelUser;
	eHeader m_sortHeader;
	const Bitmap* m_pIcon[10];
	bool m_bGrandPrix983; // +0x24c
	sGPInfo983* m_pGrandPrix983; // +0x250

	DECLARE_FRESH_MSGMAP()
public:
	char m_pad983_254[0xc]; // 983: sem leitura/escrita (ctor 00acf350 so ate +0x250; vtables d4ac84/d4ac74/d4ac6c e 00acd000..00ad0500 sem +0x254..+0x25f); sizeof 0x260 (push em 00ad0383)
};
