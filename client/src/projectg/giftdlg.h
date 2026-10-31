#pragma once

#include <string>
#include "frform.h"
#include "../shared/classdefine.h"

class FrButton;
class FrListBox;
class FrStatic;
class FrEdit;
class FrComboBox;
class FrFriendDlg;
class FrEmoticonDlg;
class Bitmap;

class FrGiftDlg : public FrForm
{
	friend class FrFriendDlg;

	DECLARE_OBJECT(FrGiftDlg)

public:
	FrGiftDlg();

	void SetReceiverInfo(unsigned long uid, const sPangYaUserInfo& info);
#ifdef REBANG_TASKMAIN_PCH // inline in the original header (taskmain.obj has its own copy); other includers keep their IL
	void UpdateFriendList()
	{
		if (m_pFriendDlg)
			m_pFriendDlg->UpdateFriendList();
	}
#endif
	void SendGift(const IFF_ITEM_COMMON& item, __int64 myPang,
		__int64 myCookie);
	void GiftProgress();

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

	struct sChild
	{
		char name[0x20];
		bool bCash;
		__int64 numberPrice;
		unsigned char numberFlag;
		unsigned short number;
		unsigned long timePrice;
		unsigned char timeFlag;
		unsigned short time;
	};

protected:
	void SetPrice(int count);
	void RecalculateTotalCookie(__int64 pang, __int64 cookie, bool bUpdate);

	bool OnFriendListResult(int result, FrForm* form);
	bool OnEmoticonResult(int result, FrForm* form);
	bool OnChargeResult(int result, FrForm* form);
	bool OnConfirmResult(int result, FrForm* form);

	void OnCheckoutInit(int param);
	void OnCheckoutBtnUp();
	void OnFriendBtnInit(int param);
	void OnFriendBtnUp();
	void OnCartInit(int param);
	void OnCartOwnerDraw(int param);
	void OnSxIdInit(int param);
	void OnEdIdInit(int param);
	bool OnEdIdEnterKey(int param);
	void OnEdIdDown();
	void OnBtIdCheckInit(int param);
	void OnBtIdCheckUp();
	void OnSxNickInit(int param);
	void OnEdNickInit(int param);
	bool OnEdNickEnterKey(int param);
	void OnEdNickDown();
	void OnEmoticonBtnInit(int param);
	void OnEmoticonBtnUp();
	void OnBtNickCheckInit(int param);
	void OnBtNickCheckUp();
	void OnMsgInit(int param);
	bool OnMsgEnterKey(int param);
	void OnEdConfirmInit(int param);
	void OnPriceInit(int param);
	void OnMyMoneyInit(int param);
	void OnMoneyLeftInit(int param);
	void OnPrice_CookieInit(int param);
	void OnMyMoney_CookieInit(int param);
	void OnMoneyLeft_CookieInit(int param);
	void OnNumStaticInit(int param);
	void OnNumInit(int param);
	void OnNumPrevInit(int param);
	void OnNumPrevBtnUp();
	void OnNumNextInit(int param);
	void OnNumNextBtnUp();
	void OnDaysInit(int param);
	void OnDaysBtnDown();
	void OnWarnInit(int param);
	void OnExplaneStaticInit(int param);

	FrButton* m_pCheckout;
	FrButton* m_pFriendBtn;
	FrListBox* m_pCart;
	FrStatic* m_pSxId;
	FrEdit* m_pEdId;
	FrButton* m_pBtIdCheck;
	FrStatic* m_pSxNick;
	FrEdit* m_pEdNick;
	FrButton* m_pBtNickCheck;
	FrEdit* m_pEdConfirm;
	FrEdit* m_pEdMsg;
	FrEdit* m_pPrice;
	FrEdit* m_pMyMoney;
	FrEdit* m_pMoneyLeft;
	FrEdit* m_pPriceCookie;
	FrEdit* m_pMyMoneyCookie;
	FrEdit* m_pMoneyLeftCookie;
	FrStatic* m_pNumStatic;
	FrEdit* m_pNum;
	FrButton* m_pNumPrev;
	FrButton* m_pNumNext;
	FrStatic* m_pWarn;
	FrStatic* m_pExplane;
	FrButton* m_pEmoticonBtn;
	FrEmoticonDlg* m_pEmoticonDlg;
	FrComboBox* m_pDays;
	int m_dayIndex;
	bool m_bDaysChanged;
	IFF_ITEM_COMMON m_item;
	__int64 m_pang;
	__int64 m_cookie;
	std::string m_receiverId;
	unsigned long m_receiverUid;
	bool m_bEnoughPang;
	bool m_bEnoughCookie;
	FrFriendDlg* m_pFriendDlg;
	__int64 m_myPang;
	__int64 m_myCookie;
	__int64 m_remainCash;
	bool m_bCanBuy;
	// E431: 983 (giftdlg.cpp 00aab900..00ab0500) nao acessa +0x294..+0x324; so o clock +0x324 (OnCartInit 00aabb80)
	MiniBtn m_pad983_294[3];
	MiniBtn m_pad983_2dc;
	MiniBtn m_pad983_2f4[2];
	MiniBtn m_clockIcon;
	int m_childCount;
	sChild m_child[5];
	std::string m_myNick;
	int m_mode;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbGiftprogressFunclets(); // HACK: EH funclets of the naked GiftProgress (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOncheckoutbtnupFunclets(); // HACK: EH funclets of the naked OnCheckoutBtnUp (nakedify)
#endif
};
