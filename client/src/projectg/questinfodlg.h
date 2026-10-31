#pragma once

#include "frform.h"
class FrListBox;
class FrEdit;
class FrArea;
class FrButton;
class WTitleFont;

class FrQuestInfoDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrQuestInfoDlg)
	FrQuestInfoDlg();
	virtual ~FrQuestInfoDlg();

	void SetQuestExplane(int index);

protected:
	void OnQuestInfoInit(int param);
	void OnRewardItemListInit(int param);
	void OnRewardItemListOwnerDraw(int param);
	void OnQuestNameEditInit(int param);
	void OnQuestMessageEditInit(int param);
	void OnQuestExplaneEditInit(int param);

	FrListBox* m_pRewardItemList;
	const Bitmap* m_pItemFrame;
	FrEdit* m_pQuestName;
	FrEdit* m_pQuestMessage;
	FrEdit* m_pQuestExplane;
	bool m_bReward;
	unsigned char m_questIndex;
	WTitleFont* m_pTitleFont;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrQuestGift : public FrForm
{
public:
	DECLARE_OBJECT(FrQuestGift)
	FrQuestGift();
	virtual ~FrQuestGift();

	bool OnNotifyResult(int result, FrForm* pForm);
	void SetQuestExplane(int index);
	void SetData(int type);
	void SetSize();

protected:
	void OnGiftListInit(int param);
	void OnGiftListOwnerDraw(int param);
	void OnGiftListLBtnDown();
	void OnSelectInit(int param);
	void OnItemBack4(int param);
	void OnItemBack5(int param);
	void OnItemBack6(int param);
	void OnItemBack7(int param);
	void OnItemBack8(int param);
	void OnItemBack9(int param);
	void OnOkInit(int param);

	FrForm* m_pNotify;
	FrListBox* m_pGiftList;
	FrArea* m_pSelect;
	FrArea* m_pItemBack[6];
	FrButton* m_pOk;
	WTitleFont* m_pTitleFont;
	unsigned char m_questIndex;
	const Bitmap* m_pItemFrame;
	bool m_bDataSet;

private:
	DECLARE_FRESH_MSGMAP()
};
