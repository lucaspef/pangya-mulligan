#pragma once

#include "frform.h"
#include "../shared/globalgamedefine.h"
#include <list>
#include <map>

class FrButton;
class FrComboBox;
class FrEdit;
class FrListBox;
class FrStatic;
namespace IFF_STRUCT
{
	struct sItem;
}
class Bitmap;

class FrCheckoutDlg : public FrForm
{
	DECLARE_OBJECT(FrCheckoutDlg)

public:
	struct DaysCombo;

	struct CartItem
	{
		IFF_STRUCT::sItem* pItem;
		int count;
		int reserved;
		int option;
		DaysCombo* pCombo;
	};

	// m_pDays/m_pDaysItem/m_bDaysFlag and the days2 triple seen as an array
	struct DaysCombo
	{
		FrComboBox* pCombo;
		CartItem* pItem;
		bool bDirty;
	};

	struct sCoupon
	{
		unsigned long itemId; // 983: o guid devolvido pelo GetGuidSelectedCoupon (00a9f680) e o dword +4
		unsigned long guid;
		unsigned char usable;
		unsigned char pad[3];
		int period;
	};

	// one entry of the period/option combo boxes
	struct MiniBtn
	{
		char text[0x20];
		bool flag;
		char pad0[7];
		int value0;
		int value1;
		unsigned char type0;
		unsigned char pad1;
		unsigned short num0;
		int value2;
		unsigned char type1;
		unsigned char pad2;
		unsigned short num1;
		char pad3[4];

		MiniBtn();
	};

	FrCheckoutDlg();

	void Checkout(__int64 money, __int64 cookie, bool bCookie);
	void Process();

	bool IsUnderLevel() { return m_bUnderLevel; }
	bool IsOverLevel() { return m_bOverLevel; }
	std::list<CartItem>& Cart();

protected:
	void OnCheckoutInit(int);
	void OnDaysInit(int);
	void OnDays2Init(int);
	void OnCouponPangInit(int);
	void OnCouponCookieInit(int);
	void OnCouponIcon1Init(int);
	void OnCouponIcon2Init(int);
	void OnCouponIcon3Init(int);
	void OnCouponIcon4Init(int);
	void OnCouponPrevInit(int);
	void OnCouponNextInit(int);
	void OnCartInit(int);
	void OnPriceInit(int);
	void OnMyMoneyInit(int);
	void OnMoneyLeftInit(int);
	void OnPrice_CookieInit(int);
	void OnMyMoney_CookieInit(int);
	void OnMoneyLeft_CookieInit(int);
	void OnNumStaticInit(int);
	void OnExplaneStaticInit(int);
	void OnNumInit(int);
	void OnNumPrevInit(int);
	void OnNumNextInit(int);

	void OnCartOwnerDraw(int);
	bool OnNumEnterKey(int);
	void OnNumPrevLBtnUp();
	void OnNumNextLBtnUp();
	void OnDaysBtnDown();
	void OnDays2BtnDown();
	void OnCheckoutLBtnUp();
	void OnCouponIcon1LBtnUp();
	void OnCouponIcon2LBtnUp();
	void OnCouponIcon3LBtnUp();
	void OnCouponIcon4LBtnUp();
	void OnCouponIcon1RBtnUp();
	void OnCouponIcon2RBtnUp();
	void OnCouponIcon3RBtnUp();
	void OnCouponIcon4RBtnUp();
	void OnCouponPrevLBtnUp();
	void OnCouponNextLBtnUp();
	bool OnConfirmResult(int result, FrForm* form);
	bool OnChargeResult(int result, FrForm* form);
	bool OnConfirm1a100002Result983(int result, FrForm* form); // 983 00aa3590 (nome meu)

	void FixCartItem(CartItem& item);
	void SetPrice(int count);
	void RecalculateTotalCookie(__int64 money, __int64 cookie, bool bCookie);
	void OpenCaddieInfo(const sCaddieInfo& info);
	unsigned long GetGuidSelectedCoupon();
	void SelectCoupon(int icon);         // 983 00aa0f10..00aa1180
	void SetCouponDiscount(int key);     // 983 00a9f270
	void UpdateCouponPrice();            // 983 00a9ac60
	void UpdateCouponIcons();            // 983 00a9f0c0
	void BuildCouponList();              // 983 00aa2f30
	void ShowCouponControls();           // 983 00a9b530
	void OpenCouponInformation(const sCoupon* pCoupon); // 983 00a9e900
	bool HasCouponControls();
	void DrawCouponLine();               // 983 00a9b810
	void DrawCouponIcons();              // 983 00a9f410

	FrButton* m_pCheckoutBtn;                  // 0x110
	FrListBox* m_pCart;                        // 0x114
	FrEdit* m_pPrice;                          // 0x118
	FrEdit* m_pMyMoney;                        // 0x11c
	FrEdit* m_pMoneyLeft;                      // 0x120
	FrEdit* m_pPriceCookie;                    // 0x124
	FrEdit* m_pMyMoneyCookie;                  // 0x128
	FrEdit* m_pMoneyLeftCookie;                // 0x12c
	FrStatic* m_pNumStatic;                    // 0x130
	FrStatic* m_pExplane;                      // 0x134
	FrEdit* m_pNum;                            // 0x138
	FrButton* m_pNumPrev;                      // 0x13c
	FrButton* m_pNumNext;                      // 0x140
	FrComboBox* m_pDays;                       // 0x144
	CartItem* m_pDaysItem;                     // 0x148
	bool m_bDaysFlag;                          // 0x14c
	FrComboBox* m_pDays2;                      // 0x150
	CartItem* m_pDays2Item;                    // 0x154
	bool m_bDays2Flag;                         // 0x158
	// 983 (construtor 00aa1af0; INIT 00a9bff0..00a9c360): controles do cupom
	FrEdit* m_pCouponPang;                     // 0x15c
	FrEdit* m_pCouponCookie;                   // 0x160
	FrButton* m_pCouponIcon[4];                // 0x164
	FrButton* m_pCouponPrev;                   // 0x174
	FrButton* m_pCouponNext;                   // 0x178
	std::list<CartItem> m_cart;                // 0x17c
	bool m_bUnderLevel;                        // 0x188
	bool m_bOverLevel;                         // 0x189
	bool m_bHasOption;                         // 0x18a
	bool m_bCookie;                            // 0x18b
	__int64 m_price;                           // 0x190
	__int64 m_priceCookie;                     // 0x198
	bool m_bMoneyOk;                           // 0x1a0
	bool m_bCookieOk;                          // 0x1a1
	int m_buyConfirmKind;                      // 0x1a4 (aviso do OK: 1 tintura, 2 caddie sobrescreve, 3 prorroga prazo, 4 ja tem)
	int m_count;                               // 0x1a8
	__int64 m_money;                           // 0x1b0
	__int64 m_cookie;                          // 0x1b8
	__int64 m_cashRemain;                      // 0x1c0 (RemainOwnCash da falta em cookie)
	bool m_bCashDue;                           // 0x1c8 (falta cookie, nao so pang)
	std::map<unsigned int, sCoupon> m_coupons; // 0x1cc (0x10)
	bool m_bHasCoupons;                        // 0x1dc (ha cupons na lista)
	int m_selCoupon;                           // 0x1e0 (chave do cupom escolhido)
	int m_couponPage;                          // 0x1e4 (primeiro cupom dos 4 icones)
	__int64 m_couponPang;                      // 0x1e8 (desconto em pang)
	__int64 m_couponCookie;                    // 0x1f0 (desconto em cookie)
	int m_couponType;                          // 0x1f8 (983 00a9f270: 1 preco do item, 3 fixo, 4 parte do preco)
	bool m_bBuySent;                           // 0x1fc (pacote de compra ja enviado)
	char m_pad983_200[0x90]; // 0x200 E426: 983 sem leitura/escrita (FrCheckoutDlg 00a9ac20..00aa4370, ctor 00aa1af0 nao zera)
	_WRECT m_clockRect;                        // 0x290
	const Bitmap* m_pClock;                    // 0x2a0
	char m_pad983_2a4[4]; // 0x2a4 E426: 983 sem leitura/escrita (FrCheckoutDlg 00a9ac20..00aa4370, ctor 00aa1af0 nao zera)
	int m_childCount;                          // 0x2a8
	char m_pad983_2ac[4]; // 0x2ac E426: 983 sem leitura/escrita (FrCheckoutDlg 00a9ac20..00aa4370, ctor 00aa1af0 nao zera)
	MiniBtn m_child[5];                        // 0x2b0
	// 983: copia do CShopMain::m_caddieIffMap (00aa25b0, chamado do Checkout 00a8c570); o
	// OnCheckoutLBtnUp 00aa3620 recusa caddie que nao esta nele. Ctor 00aa1af0, dtor 00aa1df0.
	std::map<unsigned int, IFF_STRUCT::sCaddie> m_caddieIffMap; // 0x3f0
	char m_pad983_3fc[0x400 - 0x3f0 - sizeof(std::map<unsigned int, int>)]; // 0x3fc E426: 983 sem acesso (ctor so faz o map +0x3f0..0x3fb)

public:
	void SetCaddieIffMap(const std::map<unsigned int, IFF_STRUCT::sCaddie>& m); // 983 00aa25b0

	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOncheckoutlbtnupFunclets(); // HACK: EH funclets of the naked OnCheckoutLBtnUp (nakedify)
#endif
};

bool __fastcall CompareUsableCoupon(const FrCheckoutDlg::sCoupon& a,
	const FrCheckoutDlg::sCoupon& b);
bool __fastcall ComparePeriodCoupon(const FrCheckoutDlg::sCoupon& a,
	const FrCheckoutDlg::sCoupon& b);
