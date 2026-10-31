#pragma once
#include "msg983.h"

#include <map>
#include <list>
#include <string>
#include "taskmain.h"
#include "jp/taskform.h"
#include "anotherhand.h"
#include "exhibition.h"

class FrGaugeBar;
class FrComboBox;
class FrTextButton;
class FrListBox;
class FrViewer;
class FrStatic;
struct FrListItem;
class FrGraphicInterface;
class Bitmap;
class FrForm;
class MsgObject;
class CPartTidList;
struct IFF_ITEM_COMMON;
struct sCaddieInfo;
class WOverlay;
class FrCheckoutDlg;
class FrGiftDlg;
class FrOptionDlg;
class FrKbdManualDlg;
class FrServerDlg;
class FrUpgradeCaddieDlg;

char const* __fastcall MakeStatusString(int a, int b, int c);   // 983 b2e010
char const* __fastcall MakeStatusString2(int a, int b, int c);  // 983 b2e180
char const* __fastcall MakeCardStatusString(int a, int b);      // 983 b2e260

unsigned long __fastcall ShopCategoryFlag983(int cat); // 983 00b2e310

class CShopMain : public FrTaskForm, public CHandOwner
{
	DECLARE_OBJECT(CShopMain)

public:
	CShopMain();
	virtual ~CShopMain();
	virtual bool OnOpenWidget983(); // 00a868e0 (+0x98)
	virtual bool OnCloseWidget983(); // 00a86960 (+0x9c)
	virtual bool OnMsg983(const MsgObject& p1); // 00a971f0 (+0xa0)
	void SearchItems983(std::list<struct sItemSearchResult983>& results, int group, const std::string& word); // 00a96f00 (0x2d3)
	virtual void OnProc(const float p1); // 00a91950 (slot 9)
	virtual bool OnInit(); // 00a93300 (slot 32)
	virtual void Enable(bool enable); // 00a84790 (slot 6): FrWnd::Enable + SetVisible, repassa ao m_pStatBox

	enum statTab_t
	{
		STATTAB_0
	};

	enum mainTab_t
	{
		MAINTAB_0
	};

	enum subTab_t // numeracao do 983 (BuildDisplayStand 00a923c0); o 645 tinha tudo -1 a partir da aba 1
	{
		SUBTAB_PACKAGE = 0,
		SUBTAB_SET = 1, // nova no 983 (Build 00a91d20)
		SUBTAB_HAIR = 6,
		SUBTAB_CLUBSET = 7,
		SUBTAB_AZTEC = 8,
		SUBTAB_EQUIP = 9,
		SUBTAB_NOEQUIP = 0xa,
		SUBTAB_CADDIE = 0xb,
		SUBTAB_MASCOT = 0xd,
		SUBTAB_SKIN = 0xe,
		SUBTAB_CARD = 0x1c,
		SUBTAB_FURNITURE = 0x20,
		SUBTAB_CUTIN = 0x22
	};

	struct miniBtn_t
	{
		miniBtn_t();
	};

	// 983: 8 botoes da linha da lista {WRect, N, O, bool} de 0x1c a partir de +0x2e8; a msg 0xc0 liga/desliga
	// os 8 bools juntos (00a97249). [BUY] = buy_btn (+0x304), [GIFT] = gf_btn (+0x320), [STATE] = state_view (+0x3ac);
	// os outros 5 o nosso nao usa.
	enum { ROWBTN983_BUY = 1, ROWBTN983_GIFT = 2, ROWBTN983_STATE = 7, ROWBTN983_N = 8 };
	struct sRowBtn983
	{
		_WRECT rc; // _WRECT (sem ctor): o ctor 00a91ec0 nao percorre o vetor
		WRect& Rc() { return static_cast<WRect&>(rc); }
		Bitmap* pBmpN;
		Bitmap* pBmpO;
		bool bOn;
	};

	struct oldItem_t
	{
		unsigned long tid;
		unsigned long oldTid;
		unsigned long oldUid;
	};

protected:
	void SendShopRefresh983(); // 983: 00a87980 (pacote 0x140 + mensagem 1 ao ator principal; nao le o this; pedido C30)
	void SafeLoadPet(unsigned long* p1, bool p2);
	void OnItemDescBgOwnerDraw(int p1);
	void OnItemDescInit(int p1);
	void DrawItemPrice(FrGraphicInterface* p1, float p2, float p3,
		IFF_ITEM_COMMON& p4, FrListItem* p5);
	void OnTabMainInit(int p1);
	void OnItemListInit(int p1);
	void DrawPartsSlot(FrGraphicInterface* p1, FrListItem* p2);
	bool OnSellDlgResult(int p1, FrForm* p2);
	void OpenLayout(const char* p1);
	void LoadShopImages();
	void Net_StartPeriodItem(unsigned long p1);
	void DrawSkinFrame();
	void DrawSkinSlot();
	void ShowItemDesc(unsigned long p1);
	void DrawRentalPrice(float p1, float p2, IFF_ITEM_COMMON& p3,
		FrListItem* p4);
	void OnItemSlotOwnerDraw(int p1);
	void SetPetCtrl(unsigned long p1);
	void ShowPet(unsigned long p1);
	void OpenInformation(unsigned long p1, unsigned long uccGuid, const char* uccIndex);
	void AddSaleInfo983(FrEdit* pEdit, IFF_ITEM_COMMON* pItem);
	void SendGift(const IFF_ITEM_COMMON& p1);
	void Repay(const IFF_ITEM_COMMON& p1, unsigned long p2);
	void ChangeSkin(unsigned long p1);
	void BuildMyItemSlot(bool p1);
	virtual void OnAnotherHand(int p1, void* p2);
	void SetLevelBar();
	void ResetItemList(bool p1);
	std::list<FrListItem*>::iterator DeleteItemFromCartList(unsigned long p1);
	std::list<FrListItem*>::iterator DeleteItemFromAuxCartList(
		unsigned long p1);
	bool UnselectItemFromItemList(unsigned long p1);
	bool IsItemInCart(FrListItem* p1);
	void RestoreShoppersOldItem(unsigned long p1, CPartTidList& p2);
	unsigned long ShopSaleState983(unsigned long tid);
	bool ShopItemLocked983(const IFF_ITEM_COMMON* item);
	void PutOnNBackupShoppersOldItem(unsigned long p1, CPartTidList& p2, unsigned long p3);
	void ReleaseExhibition(bool bWait);
	void SetMyItemSlot();
	void SetAutoScroll();
	void ScrollToTid983(unsigned long tid); // 00a88d60 (0x2d7)
	void ShowMainModel(CPartTidList& p1);
	void TurnOnItemStatus();
	void TurnOffItemStatus();
	bool OnReemployeeResult(int p1, FrForm* p2);
	void OpenCaddieInfo(const sCaddieInfo& p1);
	int GetItemNumInMyPocket(unsigned long p1, bool p2);
	void ClearCartList(bool p1, CPartTidList* p2);
	void ChangeMainTab(mainTab_t p1, bool p2);
	void UpgradeCaddie(unsigned long p1);
	WOverlay* GetCutinImg(unsigned long p1);
	void OnPartCartListOwnerDraw(int p1);
	void OnAuxPartCartListOwnerDraw(int p1);
	void PlacePartCartLists983(); // 00a84da0 (+ consulta 0x194 de a8aa50/a8b040)
	bool OnBuyDlgResult(int p1, FrForm* p2);
	void Checkout();
	const Bitmap* GetItemIcon(unsigned long p1);
	void LoadCardImages();
	void InitRookieLevel983(); // 00a876f0
	WOverlay* GetCardImg(unsigned long p1);
	void DrawCard();
	void InitalizeUpgradeCaddieInfo();
	void BuildVisibleCaddieList();
	void DrawCutinFrame();
	void OnPetOwnerDraw(int p1);
	void OnItemListOwnerDraw(int p1);
	void OnBuyAllUp();
	void BuildDisplayRental(subTab_t p1);
	bool SelectItemList(FrListItem* p1);
	bool SelectItemList(unsigned long p1);
	void UnselectItemList(unsigned long p1);
	void OnItemListBtnDown();
	void OnItemListRBtnDown();
	void OnSideTabRentalUp();
	void BuildDisplayStand(subTab_t p1);
	void ChangeSubTab(subTab_t p1, bool p2);
	void ChangeCharacter(unsigned long p1);
	mainTab_t MainTabOfTid983(unsigned long tid); // 00a82730
	subTab_t SubTabOfTid983(unsigned long tid); // 00a87720
	void ChangeCaddie(unsigned long p1);
	void ChangeMascot(unsigned long p1);
	void ChangeClubset(unsigned long p1);
	void ChangeAztec(unsigned long p1);
	void OnTabMainButtonUp();
	void OnTabSubButtonUp();
	void OnBuyResetUp();
	void OnSideTabBuyUp();
	void OnExportSelectListInit(int param);
	void OnExportSelectListUp();
	void OnMainBg1Init(int param);
	void OnMainBg2Init(int param);
	void OnMainBg3Init(int param);
	void OnMainBg4Init(int param);
	void OnItemSearchBtnInit(int param);
	void OnItemSearchBtnUp();
	void OnCloseWndUp();
	void OnStatArrowInit(int param);
	void OnSortOptionComboDown();
	void OnSearchGoodsLButtonDown();
	void InitShopDialogs();
	void InitShopControls();
	void SetItemDescRect();
	void DrawStatus(FrGraphicInterface* gfx, FrListItem* item, short* const a,
		short* const b, float y);
	void DrawStatus2(FrGraphicInterface* gfx, FrListItem* item, short* const a,
		short* const b, float y);
	void DrawStatView983(FrGraphicInterface* gfx, FrListItem* item,
		const short* stats); // 983 00a89380: botao "Stats" do item
	void DrawStatusCard(FrGraphicInterface* gfx, FrListItem* item,
		short* const a, float y);
	void SetVisibleSideTab(bool visible);
	void DrawStatus3(FrGraphicInterface* gfx, float x, float y, short* const a,
		short* const b);
	virtual void CloseAllDialogs();
	void CheckAuxCartScroll();
	void OnScrollUpUp();
	void OnScrollDownUp();
	void OnBuyHoverOn(int param);
	void OnBuyHoverOff(int param);
	void DeleteCutinImages();
	void DeleteCardImages();
	bool HaveCharacter(unsigned long tid);
	void TurnOffAuxCartScroll();
	void TurnOnAuxCartScroll();
	void HidePet(bool wait);
	void SetPetAreaRect();
	void ChangeStatTab(statTab_t tab);
	void TurnOnItemSelect();
	void TurnOffItemSelect();
	void DrawSelArrow(FrGraphicInterface* gfx);
	bool IsSelectedRentalTab();
	void OnPetInit(int param);
	void OnSkinPetInit(int param);
	void OnSkinPet2Init(int param);
	void OnItemDescBgInit(int param);
	void OnStatBoxInit(int param);
	void OnTabTxt2Init(int param);
	void OnTabTxt3Init(int param);
	void OnTabSubInit(int param);
	void OnPartCartListInit(int param);
	void OnAuxPartCartListInit(int param);
	void OnPowerEditInit(int param);
	void OnControlEditInit(int param);
	void OnImpactEditInit(int param);
	void OnSpinEditInit(int param);
	void OnCurveEditInit(int param);
	void OnSortOptionComboInit(int param);
	void OnSearchGoodsButtonInit(int param);
	void OnScrollUpInit(int param);
	void OnScrollDownInit(int param);
	void OnItemSlotFrame1Init(int param);
	void OnItemSlotFrame2Init(int param);
	void OnBuyAllInit(int param);
	void OnBuyResetInit(int param);
	void OnSideTabBuyInit(int param);
	void OnSideTabRentalInit(int param);
	void OnItemSlotInit(int param);

	// Layout do 983 (sizeof 0x6c0): FrTaskForm 0x120 + CHandOwner em 0x120; os campos vem dos
	// handlers Init do mapa (00a82db0...) e do LoadShopImages 00a847e0. m_pad<off> = sem par ainda.
	unsigned long m_n124;
	float m_f128; // -10.0f
	unsigned long m_n12c;
	FrTabButton* m_pTabMain; // 0x130
	FrTabButton* m_pTabSub;
	FrButton* m_pBuyAll;
	FrButton* m_pBuyReset;
	FrListBox* m_pItemList;
	FrListBox* m_pPartCartList;
	FrListBox* m_pAuxPartCartList;
	FrArea* m_pTabTxt2;
	FrArea* m_pTabTxt3;
	unsigned long m_n154;
	FrArea* m_pPetArea; // 0x158
	FrGroupBox* m_pStatBox; // 0x15c
	FrArea* m_pStatArrow;
	FrStatic* m_pStatEdit[5];
	FrButton* m_pScrollUp; // 0x178
	FrButton* m_pScrollDown;
	FrArea* m_pItemDescBg;
	FrEdit* m_pItemDesc;
	FrArea* m_pSkinPet;
	FrArea* m_pSkinPet2;
	FrArea* m_pPet;
	FrArea* m_pItemSlotFrame1;
	FrArea* m_pItemSlotFrame2;
	FrListBox* m_pItemSlot;
	FrButton* m_pSideTabBuy;
	FrButton* m_pSideTabRental;
	FrCheckoutDlg* m_pCheckoutDlg; // 0x1a8
	FrUpgradeCaddieDlg* m_pUpgradeDlg;
	std::map<unsigned long, unsigned long> m_upgradeMap; // 0x1b0
	std::map<unsigned int, IFF_STRUCT::sCaddie> m_caddieIffMap;
	Bitmap* m_pItemBase[5]; // 0x1d0 item_base_n/o/d/empty/award
	char m_pad983_1e4[8]; // E425: 983 sem leitura/escrita (CShopMain 00a82300..00a9a000, ctor 00a82310/00a823e8 nao zera)
	Bitmap* m_pBmpMarkNew; // 0x1ec
	Bitmap* m_pBmpMarkHot;
	Bitmap* m_pBmpMarkLim;
	Bitmap* m_pBmpMarkLimited;
	Bitmap* m_pBmp1fc[3];
	Bitmap* m_pBmpMarkSurprise; // 0x208
	Bitmap* m_pBmpMarkBegin;
	char m_pad983_210[0xc]; // E425: 983 sem leitura/escrita (CShopMain 00a82300..00a9a000, ctor 00a82310/00a823e8 nao zera)
	Bitmap* m_pStatBmp[5][2]; // 0x21c
	Bitmap* m_pSelectBmp[6]; // 0x244 select_up..cursor0
	Bitmap* m_pStatCharBmp[9]; // 0x25c stat_char_n..stat_caddie_d
	Bitmap* m_pStatAllBmp[3]; // 0x280
	Bitmap* m_pSiconBmp[5]; // 0x28c
	Bitmap* m_pBmpFa02; // 0x2a0 it_fa02
	Bitmap* m_pBmpCancelN;
	Bitmap* m_pBmpCancelO;
	char m_pad983_2ac[4]; // E425: 983 sem leitura/escrita (CShopMain 00a82300..00a9a000, ctor 00a82310/00a823e8 nao zera)
	Bitmap* m_pBmpMale; // 0x2b0
	Bitmap* m_pBmpFemale;
	Bitmap* m_pBmpMasterSign;
	Bitmap* m_pBmpTeamBase;
	Bitmap* m_pBmpItemSlotN; // 0x2c0
	Bitmap* m_pBmpSelArrow;
	Bitmap* m_pBmpItemDesc1;
	Bitmap* m_pBmpItemDesc2;
	char m_pad983_2d0[0x10]; // E425: 983 sem leitura/escrita (CShopMain 00a82300..00a9a000, ctor 00a82310/00a823e8 nao zera)
	Bitmap* m_pBmpSelectMark; // 0x2e0 SelectItem_Mark
	unsigned long m_n2e4; // zerado no ctor
	sRowBtn983 m_rowBtn983[ROWBTN983_N]; // 0x2e8 (8 x 0x1c; bools +0x300..+0x3c4)
	std::map<unsigned long, const Bitmap*> m_itemBmpMap; // 0x3c8
	std::map<unsigned int, WOverlay*> m_cardImgMap;
	std::map<unsigned int, WOverlay*> m_cutinImgMap;
	char m_pad983_3f8[0xc]; // E425: 983 sem leitura/escrita (CShopMain 00a82300..00a9a000, ctor 00a82310/00a823e8 nao zera)
	char* m_pCharPCL; // 0x404
	unsigned long* m_pParts;
	unsigned long m_partGuid983[24]; // 0x40c guid de cada peca provada (par de m_pParts; 983 ctor 00a823e8 zera, 0xb6 00a97d71 grava, 00a976bf le)
	unsigned long* m_pAuxParts; // 0x46c
	char m_b520;
	char m_pad471[3];
	unsigned long m_u524; // 0x474
	unsigned long m_u528;
	unsigned long m_mascotTid;
	unsigned long m_aztecTid;
	unsigned long m_clubsetTid;
	statTab_t m_statTab; // 0x488
	mainTab_t m_mainTab; // 0x48c
	mainTab_t m_prevMainTab; // 0x490 (983: copia de m_mainTab no fim de ChangeMainTab)
	subTab_t m_subTab; // 0x494
	float m_f544;
	float m_f548;
	WPoint m_descOffset;
	float m_f554;
	float m_f558;
	unsigned long m_u55c;
	CExhibition* m_pExhibition; // 0x4b4
	CAnotherHand m_hand; // 0x4b8
	std::list<unsigned long> m_tidList; // 0x640
	std::list<oldItem_t> m_oldItems;
	unsigned long m_skinTid[6]; // 0x658
	float m_f71c; // 0x670
	float m_f720;
	unsigned long m_u724;
	unsigned long m_u728;
	unsigned long m_u72c;
	FrComboBox* m_pSortOptionCombo; // 0x684
	FrTextButton* m_pSearchGoodsButton;
	FrButton* m_pItemSearchBtn; // 0x68c item_search_btn
	FrArea* m_pMainBg[4]; // 0x690 main_bg1..4
	unsigned long m_nRookieLv1; // 0x6a0: 983 00a876f0 = CSharedDoc +0x91d + 1
	unsigned long m_nRookieLv; // 0x6a4
	int m_n6a8; // -1 no ctor
	WTitleFont* m_pTitleFont; // 0x6ac: WTITLEFONT 16x16, criada no ctor
	WRect m_rc6b0; // copia do WRect zerado do .bss 00f14668

private:
	// 983 00b2e310: global fastcall (ShopCategoryFlag983, shopmain.cpp), chamada das Build_*/BuildDisplayRental
	static __forceinline unsigned long GetCategoryFlag(int cat)
	{
		return ShopCategoryFlag983(cat);
	}
	void TakeOff_AuxPartNCartList(CPartTidList& p1, unsigned long p2);
	void TakeOff_PartNCartList(CPartTidList& p1, unsigned long p2);
	void Build_Hair(subTab_t p1, int p2);
	void Build_Set(subTab_t p1, int p2);
	void Build_Parts(subTab_t p1, int p2);
	void Build_Clubset(subTab_t p1, int p2);
	void Build_Aztec(subTab_t p1, int p2);
	void Build_Equip(subTab_t p1, int p2);
	void Build_NoEquip(subTab_t p1, int p2);
	void Build_Card(subTab_t p1, int p2);
	void Build_Interior(subTab_t p1, int p2);
	void Build_Furniture(subTab_t p1, int p2);
	void Build_Decorate(subTab_t p1, int p2);
	void Build_Cutin(subTab_t p1, int p2);
	void Build_Caddie(subTab_t p1, int p2);
	void Build_Mascot(subTab_t p1, int p2);
	void Build_CaddieItem(int p1, int p2);
	void Build_Package(subTab_t p1, int p2);

protected:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOnitemlistbtndowFunclets(); // HACK: EH funclets of the naked OnItemListBtnDown (nakedify)
#endif
};
