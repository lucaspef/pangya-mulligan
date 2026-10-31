#pragma once

#include "frform.h"

struct sGiftInfo;
class FrArea;
class FrEdit;

class FrPangGiftDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrPangGiftDlg)

	FrPangGiftDlg();

	void SetItemInfo(sGiftInfo* info);
	sGiftInfo* GetItemInfo() { return m_pItemInfo; }

protected:
	void OnMyPangInit(int param);
	void OnSendPangInit(int param);
	void OnTotalPangInit(int param);
	void OnItemInit(int param);
	void OnItemOwnerDraw(int param);

	FrArea* m_pItem;
	FrEdit* m_pMyPang;
	FrEdit* m_pSendPang;
	FrEdit* m_pTotalPang;
	sGiftInfo* m_pItemInfo;

	DECLARE_FRESH_MSGMAP()
};
