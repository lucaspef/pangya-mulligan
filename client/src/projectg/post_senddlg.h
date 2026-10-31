#pragma once

#include <list>
#include <string>
#include "frform.h"
#include "standinginline.h"

class FrFriendDlg;
class FrGraphicInterface;
class FrButton;
class FrWnd;
class FrArea;
class FrListBox;
class FrEdit;
class FrEmoticonDlg;
class CPost_MyItemList;
class CPost_SendDlg;
class WTitleFont;
class Bitmap;
struct FrListItem;
struct sTradeItem;
struct sMailIncludeItem;
struct sPangYaUserInfo;
struct sFRESH_MSGMAP;

class CPost_ItemCountDlg : public FrForm
{
	DECLARE_OBJECT(CPost_ItemCountDlg)

public:
	CPost_ItemCountDlg();
	virtual ~CPost_ItemCountDlg();

	void SetSelectItem(unsigned long item, int maxCount);
	unsigned long GetSelectItem();
	int GetCount();

	void OnPostMinusInit(int param);
	void OnPostPluseInit(int param);
	void OnPostMinusBtnUp();
	void OnPostPluseBtnUp();
	void OnPostEditItemCountInit(int param);
	bool OnPostEditItemCountEnterKey(int param);

protected:
	void SetText();

	FrButton* m_pMinus;
	FrButton* m_pPluse;
	FrEdit* m_pEditCount;
	int m_count;
	int m_maxCount;
	unsigned long m_selectItem;

	DECLARE_FRESH_MSGMAP()
};

class CPost_MyItemList : public FrForm
{
	DECLARE_OBJECT(CPost_MyItemList)

public:
	struct miniBtn_t
	{
		WRect rect;
		const Bitmap* pNormal;
		const Bitmap* pOver;
	};

	CPost_MyItemList();
	virtual ~CPost_MyItemList();

	void SetParent(CPost_SendDlg* pParent);

	void OnPost_ItemListInit(int param);
	void OnPost_ImgSelect(int param);
	void OnPost_SelectListInit(int param);
	void OnPost_EditArrayInit(int param);
	void OnPostBtnArrayInit(int param);
	void OnPost_ItemListMouseMove();
	void OnPost_ItemListDClick();
	void OnPost_ItemListRBtnDown();
	void OnPost_SelectListMouseMove();
	void OnPost_SelectListDClick();
	void OnPost_ItemListOwnerDraw(int param);
	void OnPost_ItemListBtnDown();
	void OnPost_SelectListOwnerDraw(int param);
	void OnPost_SelectListBtnDown();
	void OnPostBtnArrayButtonUp();
	bool OnPostItemCountResult(int result, FrForm* form);
	bool OnPostItemArrayResult(int result, FrForm* form);
	void DrawItem(WPoint pos, sTradeItem* item, unsigned char selected);
	void DrawPang();

protected:
	virtual void OnOK();
	virtual bool OnInit();
	void SetList();
	void DrawPartsSlot(FrGraphicInterface* gdi, FrListItem* item);

	FrListBox* m_pItemList;
	FrListBox* m_pSelectList;
	FrArea* m_pImgSelect;
	FrListItem* m_pSelected;
	FrButton* m_pBtnArray;
	FrEdit* m_pEditArray;
	CPost_SendDlg* m_pParent;
	WTitleFont* m_pFont;
	WTitleFont* m_pFontPang;
	const Bitmap* m_pSelectN;
	const Bitmap* m_pSelectD;
	const Bitmap* m_pLine;
	int m_registerCount; // 983 +0x140: quantidade do item registrado (OnPostItemCountResult)
	const Bitmap* m_pUccOriginal;
	const Bitmap* m_pUccCopied;
	const Bitmap* m_pUccTempSaved;
	CPost_ItemCountDlg* m_pCountDlg;
	CStandingInLine* m_pArrayDlg;
	miniBtn_t m_cancelBtn;
	int m_pad170; // 983: +0x170 nunca acessado
	int m_charFilter; // 983 +0x174: filtro de personagem da lista (-1 = todos, ctor 0080c7ab)

	DECLARE_FRESH_MSGMAP()
};

class CPost_SendDlg : public FrForm
{
	friend class FrFriendDlg;

	DECLARE_OBJECT(CPost_SendDlg)

public:
	CPost_SendDlg();
	virtual ~CPost_SendDlg();

	void SetReceiverInfo(unsigned long uid, const sPangYaUserInfo& info);
	void Reset();
	FrFriendDlg* GetFriendDlg983() const { return m_pFriendDlg; } // D620: 983 +0x110 (CTaskMain 0065e011)
	bool OnEmoticonResult(int result, FrForm* form);
	bool OnSendQuestionResult(int result, FrForm* form);
	bool OnFriendListResult(int result, FrForm* form);
	bool OnPostItemListResutl(int result, FrForm* form);
	std::string GetLetterMsg();
	int GetSendListSize();
	int GetSendServiceCharge();
	bool VerifySendItemList();
	bool VerifyIncludeSameItem();
	std::list<sMailIncludeItem>* GetSendList();
	// 983 (pedido C32, s102): publicos para o CTaskMain::OnPostSendDlgResult983 (0065c2d0)
	unsigned long GetSendUID() { return m_receiverUid; } // +0x168, em linha no 983
	std::string GetReceiverId983() { return m_receiverId; } // 0065bf30 (copia fora de linha no taskmain.obj); +0x14c
#ifdef REBANG_REALMYROOMMAINUI_CPP
	void UpdateFriendList();
#endif

protected:
	virtual void OnCancel();
	virtual void OnOK();

	void OnPostReciveUserInit(int param);
	void OnPostReciveMsgInit(int param);
	void OnPostUserNickInit(int param);
	void OnPostSearchUserInit(int param);
	void OnPostFriendListInit(int param);
	void OnPostItemAddInit(int param);
	void OnPostListInit(int param);
	void OnPostServiceChargeInit(int param);
	void OnEmoticonBtnInit(int param);
	bool OnPostReciveMsgEnterKey(int param);
	bool OnPostReceiveUserEnterKey(int param);
	void OnPostSearchUserBtnUp();
	void OnPostFriendListBtnUp();
	void OnEmoticonBtnUp();
	void OnPostItemAddBtnUp();
	void OnPostListOwnerDraw(int param);
	void DrawPartsSlot(FrGraphicInterface* gdi, FrListItem* item);

	FrFriendDlg* m_pFriendDlg;
	FrButton* m_pFriendList;
	FrButton* m_pEmoticonBtn;
	FrButton* m_pItemAdd;
	FrEmoticonDlg* m_pEmoticonDlg;
	CPost_MyItemList* m_pMyItemList;
	FrListBox* m_pPostList;
	FrEdit* m_pServiceCharge;
	std::string m_receiverNick;
	std::string m_receiverId;
	unsigned long m_receiverUid;
	const Bitmap* m_pUccOriginal;
	const Bitmap* m_pUccCopied;
	const Bitmap* m_pUccTempSaved;
	WTitleFont* m_pTitleFont;
	FrEdit* m_pUserNick;
	FrEdit* m_pReciveUser;
	FrEdit* m_pReciveMsg;
	FrButton* m_pSearchUser;
	bool m_bSearched;
	std::list<sTradeItem> m_listTradeItem;
	std::list<sMailIncludeItem> m_listSend;

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpostsearchuserFunclets(); // HACK: EH funclets of the naked OnPostSearchUserBtnUp (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnpostfriendlistFunclets(); // HACK: EH funclets of the naked OnPostFriendListBtnUp (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnokFunclets(); // HACK: EH funclets of the naked OnOK (nakedify)
#endif
};

#ifdef REBANG_REALMYROOMMAINUI_CPP
#include "frienddlg.h"

inline void CPost_SendDlg::UpdateFriendList()
{
	if (m_pFriendDlg)
		m_pFriendDlg->UpdateFriendList();
}
#endif
