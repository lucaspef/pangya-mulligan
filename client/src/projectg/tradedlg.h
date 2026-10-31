#pragma once

#include "frform.h"
#include "tooltipmanager.h"
#include "../shared/globalnetworkdefine.h"

class FrArea;
class FrButton;
class FrEdit;
class FrListBox;
struct FrListItem;
class FrStatic;
class WTitleFont;

enum eResultTrade
{
	RESULT_TRADE_OK = 1
};

enum eTypeSalesTrade
{
	SALES_TRADE_NORMAL,
	SALES_TRADE_ONE,
	SALES_TRADE_PACKAGE
};

struct sPartEquipInfo
{
	sPartEquipInfo()
	{
		m_part = 0;
		m_tid = 0;
	}

	unsigned long m_part;
	unsigned long m_tid;
};

void __fastcall SetTradeResultCode(int code, int type, sTradeItem* item);

class FrTradeDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeDlg)

	FrTradeDlg();
	virtual ~FrTradeDlg();

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

	void EnableSendingServer(bool enable);
	void ShowStaticOutOfStock(bool show);
	void EnableHideBn(bool enable);
	void DrawPang();
	void OpenTradeShop(unsigned long uid);
	void UpdateTradeTitle();
	void UpdateCountVisitor();
	void SetListTradeItem(bool bEmpty);
	bool OnTradeBuyDlgResult(int result, FrForm* form);

protected:
	virtual bool OnInit();
	virtual void OnCancel(); // 983: 008f6af0 (posicao 34 da vtable 00d3b0f4)

	void OnTradeDlgInit(int param);
	void OnVisitorBackInit(int param);
	void OnVisitorBackOwnerDraw();
	void OnMyPangInit(int param);
	void OnMyPangOwnerDraw();
	void OnTradeItemListInit(int param);
	void OnTradeItemListLClick();
	void OnTradeItemListRClick();
	void OnTradeItemListOwnerDraw(int param);
	void OnNoItem01Init(int param);
	void OnNoItem02Init(int param);
	void OnVisitorInit(int param);
	void OnUserIdInit(int param);
	void OnTitleInit(int param);
	void OnEditBnInit(int param);
	void OnEditBnUp();
	void OnCloseBnInit(int param);
	void OnCloseBnUp();
	void OnHideBnInit(int param);
	void OnHideBnUp();
	void OnExitBnInit(int param);
	void OnExitBnUp();

	FrArea* m_pVisitorBack;
	FrListBox* m_pTradeItemList;
	FrStatic* m_pNoItem01;
	FrStatic* m_pNoItem02;
	FrEdit* m_pVisitor;
	FrEdit* m_pUserId;
	FrEdit* m_pTitle;
	FrButton* m_pEditBn;
	FrButton* m_pCloseBn;
	FrButton* m_pHideBn;
	FrButton* m_pExitBn;
	const Bitmap* m_pItemBase[4];
	const Bitmap* m_pUccState[3];
	WTitleFont* m_pFontTime;
	WTitleFont* m_pFontPrice;
	bool m_bOwner;
	unsigned long m_tradeUID;
	// 983: 0x168..0x184 e 0x18c so zerados no ctor (008f0fa0); nenhuma funcao do 983 le
	unsigned long m_unused168[8];
	bool m_bSendingServer;
	float m_unused18c; // 983: so gravado com fstp (008f11d1)
	// 983 (008f0fa0, 0x210): sem m_pPackagePurchaseBn, m_unknown194, m_totalPrice e m_packageSale
	// (a venda em pacote do 645 nao existe); m_buyBtn em +0x190
	MiniBtn m_buyBtn[3];
	MiniBtn m_deleteIcon;
	MiniBtn m_clockIcon;
	int m_pad208; // 983: nunca acessado (sizeof 0x210)

private:
	DECLARE_FRESH_MSGMAP()
};

class FrTradeBuyDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeBuyDlg)

	FrTradeBuyDlg();
	virtual ~FrTradeBuyDlg();

#ifdef REBANG_TASKMANAGER_PCH // HACK: taskmanager_pch.h
	sTradeItem* GetBuyItem() { return (sTradeItem*)m_pBuyItem; }
#endif
	bool IsClosing() { return m_bClosing; }
	void SetClosing(bool closing) { m_bClosing = closing; }
	void SetBuyItem(sTradeItem* item);
	void SetControlsBuyItem(sTradeItem* item);
	void UpdatePrice();
	bool OnConfirmResult(int result, FrForm* form);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float deltaTime);

	void OnTradeBuyDlgInit(int param);
	void OnOkBnInit(int param);
	void OnOkBnUp();
	void OnBuyItemIconInit(int param);
	void OnBuyItemIconOwnerDraw();
	void OnBuyItemNameInit(int param);
	void OnBuyItemAbilityInit(int param);
	void OnMyPangInit(int param);
	void OnPriceInit(int param);
	void OnPangLeftInit(int param);
	void OnNumberInit(int param);
	void OnNumberNextBnInit(int param);
	void OnNumberNextBnUp();
	void OnNumberPrevBnInit(int param);
	void OnNumberPrevBnUp();

	FrArea* m_pBuyItemIcon;
	FrEdit* m_pBuyItemName;
	FrEdit* m_pBuyItemAbility;
	FrEdit* m_pMyPang;
	FrEdit* m_pPrice;
	FrEdit* m_pPangLeft;
	FrButton* m_pNumberNext;
	FrButton* m_pNumberPrev;
	FrEdit* m_pNumber;
	// 983 (008eed50, 0x200): sem m_pPurchaseList e m_packageSale; m_buyItem em +0x134
	sTradeItem m_buyItem;
	const sTradeItem* m_pBuyItem;
	bool m_bClosing;
	const Bitmap* m_pUccState[3];
	WTitleFont* m_pFont;
	int m_count;
	int m_pad1fc; // 983: nunca acessado (ctor 008eed50, sizeof 0x200)

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnconfirmresultFunclets(); // HACK: EH funclets of the naked OnConfirmResult (nakedify)
#endif
};

class FrTradeResultDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeResultDlg)

	FrTradeResultDlg();
	virtual ~FrTradeResultDlg();

	void SetResultItem(sTradeItem* item);

protected:
	virtual bool OnInit();

	void OnTradeResultDlgInit(int param);
	void OnResultItemIconInit(int param);
	void OnResultItemIconOwnerDraw();
	void OnResultItemNameInit(int param);
	void OnResultItemAbilityInit(int param);

	FrArea* m_pResultItemIcon;
	FrEdit* m_pResultItemName;
	FrEdit* m_pResultItemAbility;
	const Bitmap* m_pUccState[3];
	sTradeItem m_resultItem;
	// 983 (008ef300, 0x1e0): fonte em +0x1d4 logo depois do item de 0xac; sem m_list e m_pPurchaseList
	WTitleFont* m_pFont;
	unsigned long m_pad1d8; // 983: nunca acessado (ctor 008ef300, sizeof 0x1e0)

private:
	DECLARE_FRESH_MSGMAP()
};

// NOTE: the members from 0x1e8 on are only partly named
class FrTradeEditDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeEditDlg)

	FrTradeEditDlg();
	virtual ~FrTradeEditDlg();

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

	// 983: byte em +0x1e0 no lugar do m_packageSale do 645. Ligado quando o titulo vai ao servidor antes de abrir a
	// loja; a resposta (CTask::OnPacketCommon, 009cd32d) chama OnOpenBnUp e desliga. Nomes meus (s84).
	void SetOpenAfterTitle983(bool b); // 008ee290
	bool IsOpenAfterTitle983(); // 008ee2a0
	void CancelSelectedTradeItem();
	int GetNumEquipItem(unsigned long guid);
	int GetNumTradingItem(unsigned long guid);
	void SetListTradeItem();
	bool IsEquipParts(unsigned long part, unsigned long tid);
	void UpdateTradeTitle();
	void UpdateTradeTitleEdit();
	void SetListTradingItem();
	void SetListMyItem();
	void DelListTradeItem(sTradeItem* item);
	void DelListMyItem(sTradeItem* item);
	void SetListEquipParts();
	void SetListEnableTradingMyItem();
	void DrawPang();
	void DrawItem(WPoint pos, sTradeItem* item, unsigned char flag);
	void AddListTradeItem(sTradeItem* item);
	void AddListMyItem(sTradeItem* item);
	void SetTradeMyItem(sTradeItem* item);
	void SetControlsTradeMyItem(sTradeItem* item, bool enable);
	int CheckTradeEdit();
	eResultTrade SendTradeTitle();
	void SetControlsButton(bool bOpen, bool bClose);

protected:
	virtual bool OnInit();
	virtual void OnCancel(); // 983: 008f3890 (posicao 34)

	void OnTradeEditDlgInit(int param);
	void OnMyItemListInit(int param);
	void OnMyItemListLClick();
	void OnMyItemListRClick();
	void OnMyItemListOwnerDraw(int param);
	void OnTradeItemListInit(int param);
	void OnTradeItemListLClick();
	void OnTradeItemListOwnerDraw(int param);
	void OnVisitorInit(int param);
	void OnUserIdInit(int param);
	void OnTitleInit(int param);
	void OnTitleEditInit(int param);
	bool OnTitleEditEnterKey(int param);
	void OnTitleEditBnInit(int param);
	void OnTitleEditBnUp();
	void OnTradeMyItemIconInit(int param);
	void OnTradeMyItemIconOwnerDraw();
	void OnTradeMyItemNameInit(int param);
	void OnTradeMyItemAbilityInit(int param);
	void OnTradeMyItemNumberInit(int param);
	void OnTradeMyItemPriceInit(int param);
	void OnTradeMyItemPriceClick();
	void OnTradeMyItemOkBnInit(int param);
	void OnTradeMyItemOkBnUp();
	void OnTradeMyItemCancelBnInit(int param);
	void OnTradeMyItemCancelBnUp();
	void OnOpenBnInit(int param);
	void OnOpenBnUp();
	void OnCloseBnInit(int param);
	void OnCloseBnUp();
	void OnExitBnInit(int param);
	void OnExitBnUp();
public:
	void OnOpenBnUp983() { OnOpenBnUp(); } // 008f94a0 (thunk chamado pelo CTask::OnPacketCommon)
protected:

	FrListBox* m_pTradeItemList;
	FrListBox* m_pMyItemList;
	FrEdit* m_pVisitor;
	FrEdit* m_pUserId;
	FrEdit* m_pTitle;
	FrEdit* m_pTitleEdit;
	FrArea* m_pMyItemIcon;
	FrEdit* m_pMyItemName;
	FrEdit* m_pMyItemAbility;
	FrEdit* m_pMyItemNumber;
	FrEdit* m_pMyItemPrice;
	FrButton* m_pMyItemOkBn;
	FrButton* m_pMyItemCancelBn;
	FrButton* m_pOpenBn;
	FrButton* m_pCloseBn;
	FrButton* m_pExitBn;
	// 983 (008f6ba0, 0x280): sem m_pPackageSaleBn, m_pTooltipArea e m_toolTip; m_pBase em +0x150
	const Bitmap* m_pBase[8];
	WTitleFont* m_pFontTime;
	WTitleFont* m_pFontPrice;
	unsigned long m_unused190[8]; // 983: so zerado no ctor (008f6ba0)
	char m_title[0x20];
	std::list<sTradeItem> m_listMyItem;
	std::list<sTradeItem> m_listTradeItem;
	const sTradeItem* m_pBuyItem;
	std::list<sPartEquipInfo> m_listEquipParts;
	bool m_bOpenAfterTitle983;
	MiniBtn m_btn[6];
	int m_pad274; // 983: nunca acessado (sizeof 0x280)

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSendtradetitleFunclets(); // HACK: EH funclets of the naked SendTradeTitle (nakedify)
#endif
};

class FrTradeFinishEditDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeFinishEditDlg)
	friend class FrTradeEditDlg;

	FrTradeFinishEditDlg();
	virtual ~FrTradeFinishEditDlg();

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

	void SetTradeUserId();
	void SetTradeTitle(char* title);
	void SetListTradeItem(std::list<sTradeItem>* list);

protected:
	virtual bool OnInit();

	void OnTradeFinishEditDlgInit(int param);
	void OnTradeItemListInit(int param);
	void OnTradeItemListOwnerDraw(int param);
	void OnUserIdInit(int param);
	void OnTitleInit(int param);
	void OnOkBnInit(int param);
	void OnOkBnUp();

	FrListBox* m_pTradeItemList;
	FrEdit* m_pUserId;
	FrEdit* m_pTitle;
	const Bitmap* m_pBase[7];
	WTitleFont* m_pFont;
	char m_title[0x20];
	std::list<sTradeItem>* m_pList;
	unsigned char m_titleSource;
	// 983 (008f4200): sem m_packageSale; m_deleteIcon em +0x164 (o tamanho 0x1a0 nao muda por causa do alinhamento)
	MiniBtn m_deleteIcon;
	MiniBtn m_clockIcon;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrTradeWarningDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeWarningDlg)

	FrTradeWarningDlg();
	virtual ~FrTradeWarningDlg();

	void SetControls(int type);
	void SetWarningMessageType(int type, unsigned char flag);
	void SetWarningMessageForPackage(int type);

protected:
	void OnTradeWarningDlgInit(int param);
	void OnMsg01Init(int param);
	void OnMsg02Init(int param);
	void OnOkBnInit(int param);
	void OnOkBnUp();
	void OnCancelBnInit(int param);
	void OnCancelBnUp();

	FrStatic* m_pMsg01;
	FrStatic* m_pMsg02;
	FrButton* m_pOkBn;
	FrButton* m_pCancelBn;
	int m_type;
	int m_pad124; // 983: nunca acessado (ctor 008ee3e0, SetWarningMessageType 008f0180, SetControls 008f00d0; sizeof 0x130)

private:
	DECLARE_FRESH_MSGMAP()
};

class FrTradeLevelLimitDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTradeLevelLimitDlg)

	FrTradeLevelLimitDlg();
	virtual ~FrTradeLevelLimitDlg();

protected:
	virtual bool OnInit();

	void OnOkBnInit(int param);
	void OnOkBnUp();

private:
	DECLARE_FRESH_MSGMAP()
};
