#pragma once

#include <string>
#include "frform.h"
#include "tooltipmanager.h"
#include "tradedlg.h"
#include "../shared/globalnetworkdefine.h"

class FrComboBox;
class FrEdit;
class FrButton;
class FrViewer;
class FrListBox;
class FrArea;

class CStandingInLine;
struct FrListItem;

class FrOfflineTradeFinishEditDlg : public FrTradeFinishEditDlg
{
public:
	DECLARE_OBJECT(FrOfflineTradeFinishEditDlg)

	FrOfflineTradeFinishEditDlg();
	virtual ~FrOfflineTradeFinishEditDlg();
	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		DWORD color;
	};

	void SetListTradeItem(std::list<sTradeItem>* list);
	void GetSelectSlotInfo(int& index, int& valid);

protected:
	void OnTradeItemListOwnerDraw(int param);
	void OnTradeItemListLBtnUp();
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	FrListItem* m_pSelectedSlot;
	FrArea* m_pToolTip;
	CToolTipManager m_toolTipMgr;
	MiniBtn m_extendBtnN;
	MiniBtn m_extendBtnO;
	MiniBtn m_returnN;
	MiniBtn m_returnO;
	MiniBtn m_buyBtnN;
	MiniBtn m_buyBtnO;
	MiniBtn m_itemUploadN;
	MiniBtn m_itemUploadO;
	MiniBtn m_deleteBtnN;
	MiniBtn m_deleteBtnO;
	MiniBtn m_delete;
	MiniBtn m_tradeState1;
	MiniBtn m_tradeState2;
	MiniBtn m_tradeState3;

private:
	DECLARE_FRESH_MSGMAP()
};

class CBuyingSlotDlg : public FrForm
{
public:
	DECLARE_OBJECT(CBuyingSlotDlg)

	CBuyingSlotDlg();
	virtual ~CBuyingSlotDlg();

	void SetSlotNum(int num) { m_slotNum = num; }
	void RecalculatePang();

protected:
	void OnDaysInit(int param);
	void OnMyMoneyInit(int param);
	void OnMoneyLeftInit(int param);
	void OnPriceInit(int param);
	void OnCheckOutInit(int param);
	void OnSlotPriceInit(int param);
	void OnDaysBtnDown();
	void OnCheckOutBtnDown();

	FrComboBox* m_pDays;
	FrEdit* m_pMyMoney;
	FrEdit* m_pMoneyLeft;
	FrEdit* m_pPrice;
	FrButton* m_pCheckOut;
	FrEdit* m_pSlotPrice;
	__int64 m_price;
	int m_slotNum;
	int m_pad645_134; // 645: sem leitura/escrita (645 e nosso); o 983 refez a classe (0x400 B, ctor 00aa1af0)

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOncheckoutbtndowFunclets(); // HACK: EH funclets of the naked OnCheckOutBtnDown (nakedify)
#endif
};

class COfflineTradeDlg : public FrForm
{
public:
	DECLARE_OBJECT(COfflineTradeDlg)

	COfflineTradeDlg();
	virtual ~COfflineTradeDlg();

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		DWORD color;
	};

	bool SetSearchString(std::string search);
	void SetPageData(int curPage, int maxPage);
	void RequestMaketList();
	void SetListItem();
	bool OnStandingInLineDlgResult(int result, FrForm* form);

protected:
	void OnAreaSubNickInit(int param);
	void OnAreaSubNickOwnerDraw(int param);
	void OnAreaSubNickLBtnUP();
	void OnAreaSubPriceInit(int param);
	void OnAreaSubPriceOwnerDraw(int param);
	void OnAreaSubPriceLBtnUP();
	void OnAreaSubItemNameInit(int param);
	void OnAreaSubItemNameOwnerDraw(int param);
	void OnAreaSubItemNameLBtnUP();
	void OnItemSearchInit(int param);
	bool OnItemSearchEnterKey(int param);
	void OnItemSearchLBtnUp();
	void OnBtnMainSearchInit(int param);
	void OnBtnMainSearchUp();
	void OnEditMaxPageInit(int param);
	void OnEditCurrentPageInit(int param);
	bool OnEditCurrentPageEnterKey(int param);
	void OnBtnPrevInit(int param);
	void OnBtnPrevBtnUp();
	void OnBtnNextInit(int param);
	void OnBtnNextBtnUp();
	void OnBtnGoPageInit(int param);
	void OnBtnGoPageBtnUp();
	void OnBtnRefreshInit(int param);
	void OnBtnRefreshBtnUp();
	void OnBtnDetailInit(int param);
	void OnBtnDetailBtnUp();
	void OnMaketListInit(int param);
	void OnMaketListOwnerDraw(int param);
	void OnMaketListBtnDown();
	void OnEditMsgInit(int param);
	void OnEditMsgLBtnUp();
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	FrViewer* m_pAreaSubNick;
	FrViewer* m_pAreaSubPrice;
	FrViewer* m_pAreaSubItemName;
	FrEdit* m_pItemSearch;
	FrEdit* m_pMaxPage;
	FrEdit* m_pCurPage;
	FrListBox* m_pMaketList;
	FrButton* m_pBtnPrev;
	FrButton* m_pBtnNext;
	FrButton* m_pBtnGoPage;
	FrButton* m_pBtnMainSearch;
	FrButton* m_pBtnRefresh;
	FrButton* m_pBtnDetail;
	FrEdit* m_pEditMsg;
	FrArea* m_pToolTip;
	CToolTipManager m_toolTipMgr;
	CStandingInLine* m_pStandingInLine;
	MiniBtn m_moveBtn;
	MiniBtn m_moveOverBtn;
	int m_maxPage;
	int m_curPage;
	int m_sortType;
	int m_searchOrder;
	bool m_searchFlag645; // 645: zerado com a busca (ctor e Refresh), nunca lido; o 983 nao tem esta janela (pacotes 0xc5..0xca sem id)
	int m_tradeMode;
	std::string m_searchString;

private:
	DECLARE_FRESH_MSGMAP()
};

class COfflineShopEditDlg : public FrTradeEditDlg
{
public:
	DECLARE_OBJECT(COfflineShopEditDlg)

	COfflineShopEditDlg();
	virtual ~COfflineShopEditDlg();
	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		DWORD color;
	};

	void SetSelectData(unsigned long idx, unsigned long valid);
	void SetListTradeItem();
	void SetSlotData();
	bool IsCheckCardPats(unsigned long partsUid, unsigned long partsTid);
	bool OnOfflineEquitCancelDlgResult(int result, FrForm* form);
	bool OnOfflineTradeItemOnDlgResult(int result, FrForm* form);
	void InitAllUI();
	void DelListMyItem(sTradeItem* item, unsigned long guid,
		unsigned char flag);
	bool OnOfflineTradeBuyDlgResult(int result, FrForm* form);

protected:
	void OnAreaBg01Init(int param);
	void OnAreaBg02Init(int param);
	void OnAreaBg03Init(int param);
	void OnAreaBg04Init(int param);
	void OnAreaBg05Init(int param);
	void OnStallSlotNumInit(int param);
	void OnSlotNumInit(int param);
	void OnTradeItemListInit(int param);
	void OnTradeItemListLClick();
	void OnTradeItemListOwnerDraw(int param);
	void OnVisitorInit(int param);
	void OnTradeMyItemOkBnUp();
	void OnOpenBnInit(int param);
	void OnCloseBnInit(int param);
	void OnExitBnUp();
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	virtual bool OnInit();
	virtual void OnProc(const float dt);

	FrListBox* m_pShopItemList;
	FrEdit* m_pVisitor;
	FrButton* m_pShopOpenBn;
	FrButton* m_pShopCloseBn;
	int m_pad645_2a0; // 645: sem leitura/escrita (645 e nosso)
	FrArea* m_pAreaBg01;
	FrArea* m_pAreaBg02;
	FrArea* m_pAreaBg03;
	FrArea* m_pAreaBg04;
	FrArea* m_pAreaBg05;
	FrEdit* m_pStallSlotNum;
	FrEdit* m_pSlotNum;
	FrArea* m_pToolTip;
	CToolTipManager m_toolTipMgr;
	FrOfflineTradeFinishEditDlg* m_pFinishEditDlg;
	MiniBtn m_buyBtnN;
	MiniBtn m_buyBtnO;
	MiniBtn m_tradeItemDelBnNormal;
	MiniBtn m_tradeItemDelBnOver;
	int m_usedSlotCount;
	int m_closing;
	unsigned long m_selectData[2];

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnofflinetradeitFunclets(); // HACK: EH funclets of the naked OnOfflineTradeItemOnDlgResult (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOntrademyitemokbFunclets(); // HACK: EH funclets of the naked OnTradeMyItemOkBnUp (nakedify)
#endif
};

class COfflineShopDlg : public FrTradeDlg
{
public:
	DECLARE_OBJECT(COfflineShopDlg)

	COfflineShopDlg();
	virtual ~COfflineShopDlg();
	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		DWORD color;
	};

	bool OnOfflineShopEditDlgCreateResult(int result, FrForm* form);
	void SetListTradeItem(bool clear);

protected:
	void OnVisitorBackOwnerDraw();
	void OnVisitorInit(int param);
	void OnCloseBnUp();
	void OnVisitorBackInit(int param);
	void OnTradeItemListOwnerDraw(int param);
	void OnTradeItemListLClick();
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	virtual void OnProc(const float dt);

	FrArea* m_pVisitorBack;
	FrEdit* m_pVisitor;
	FrArea* m_pToolTip;
	CToolTipManager m_toolTipMgr;
	MiniBtn m_extendBtnN;
	MiniBtn m_extendBtnO;
	MiniBtn m_returnN;
	MiniBtn m_returnO;
	MiniBtn m_buyBtnN;
	MiniBtn m_buyBtnO;
	MiniBtn m_itemUploadN;
	MiniBtn m_itemUploadO;
	MiniBtn m_deleteBtnN;
	MiniBtn m_deleteBtnO;
	MiniBtn m_delete;
	MiniBtn m_tradeState1;
	MiniBtn m_tradeState2;
	MiniBtn m_tradeState3;
	int m_closing;
	FrListItem* m_pSelectedSlot;
	int m_pad645_394; // 645: sem leitura/escrita (645 e nosso)

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOntradeitemlistl2Funclets(); // HACK: EH funclets of the naked OnTradeItemListLClick (nakedify)
#endif
};

class COfflineShopVisitorDlg : public FrTradeDlg
{
public:
	DECLARE_OBJECT(COfflineShopVisitorDlg)

	COfflineShopVisitorDlg();
	virtual ~COfflineShopVisitorDlg();
	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		DWORD color;
	};

	bool OnOfflineShopEditDlgCreateResult(int result, FrForm* form);
	void SetListTradeItem(bool clear);

protected:
	void OnEditBnUp();
	void OnVisitorBackOwnerDraw();
	void OnCloseBnUp();
	void OnVisitorInit(int param);
	void OnVisitorBackInit(int param);
	void OnTradeItemListOwnerDraw(int param);
	void OnTradeItemListLClick();
	void OnToolTipInit(int param);
	void OnToolTipOnwerDraw(int param);

	virtual void OnProc(const float dt);

	FrEdit* m_pVisitor;
	FrArea* m_pVisitorBack;
	FrArea* m_pToolTip;
	CToolTipManager m_toolTipMgr;
	MiniBtn m_buyBtnN;
	MiniBtn m_buyBtnO;
	MiniBtn m_pad645_26c; // 645: sem leitura/escrita (645 e nosso); bitmap nunca carregado
	MiniBtn m_tradeState1;
	MiniBtn m_tradeState2;
	MiniBtn m_tradeState3;
	int m_pad645_2cc; // 645: sem leitura/escrita (645 e nosso)

private:
	DECLARE_FRESH_MSGMAP()
};

class COfflineBuyDlg : public FrTradeBuyDlg
{
public:
	DECLARE_OBJECT(COfflineBuyDlg)

	COfflineBuyDlg();
	virtual ~COfflineBuyDlg();

	void SetBuyitemData(sStallSlot slot) { m_slot = slot; }
	bool OnConfirmResult(int result, FrForm* form);

protected:
	void OnOkBnUp();

	sStallSlot m_slot;
	int m_count;

private:
	DECLARE_FRESH_MSGMAP()
};
