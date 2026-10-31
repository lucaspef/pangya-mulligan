#include "minatl.h"
#include "vc10_layout.h"
#include "checkoutdlg.h"
#include "../../include/packets/browserpackets.h" // C33: OnChargeResult
#include "bitmap.h"
#include "frbutton.h"
#include "frcombobox.h"
#include "fredit.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "commonutil.h"
#include "binstr.h"
#include "rtti.h"
#include "caddieinfodlg.h"
#include "frdesktop.h"
#include "projectg.h"
#include "../shared/itemmanager.h"
#include "browser.h"
#include "logininfo.h"
#include "taskmanager.h"
#include "actor.h"
#include "inputmanager.h"
#include "chatmsg.h"
#include "jp/shareddocjp.h"
#include "jp/gamestringmanager.h"
#include "packet.h"
#include "cardremove.h"			// PKT (frente C)
#include "packets/giftpackets.h"	// PKT (frente C)
#include "../shared/localize.h"
#include "../shared/titlemanager.h"
#include "titles_client.h"
#include <wininet.h>


namespace S5
{
	bool ISCutinItem(unsigned long typeId);
	__int64 ExchangeOwnCookieToCash(__int64 cookie, int rate);
	namespace RENTALPARTS
	{
		int IsRentableParts(unsigned long typeId);
		int GetRentalPrice(unsigned long typeId);
	}
}

int float2int(float f);

// unused, but the original emitted g_HUGE/g_EPSILON into .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern Fresh* g_pFresh;

// HACK: base64 encode/decode tables from a header-defined static of the
// original build (unreferenced here, but emitted at the start of .data)
char g_base64Chars[64] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
	'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
	'3', '4', '5', '6', '7', '8', '9', '+', '/' };
int g_base64Decode[256] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54,
	55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1,
	-1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

enum
{
	MAX_CHILD = 5
};

IMPLEMENT_OBJECT(FrCheckoutDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrCheckoutDlg, FrForm)
ON_FRESH_VI("checkout", FRCMD_INIT, FrCheckoutDlg::OnCheckoutInit)
ON_FRESH_VV("checkout", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCheckoutLBtnUp)
ON_FRESH_VI("cart", FRCMD_INIT, FrCheckoutDlg::OnCartInit)
ON_FRESH_VI("cart", FRCMD_OWNERDRAW, FrCheckoutDlg::OnCartOwnerDraw)
ON_FRESH_VI("price", FRCMD_INIT, FrCheckoutDlg::OnPriceInit)
ON_FRESH_VI("my_money", FRCMD_INIT, FrCheckoutDlg::OnMyMoneyInit)
ON_FRESH_VI("money_left", FRCMD_INIT, FrCheckoutDlg::OnMoneyLeftInit)
ON_FRESH_VI("price_cookie", FRCMD_INIT, FrCheckoutDlg::OnPrice_CookieInit)
ON_FRESH_VI("my_money_cookie", FRCMD_INIT, FrCheckoutDlg::OnMyMoney_CookieInit)
ON_FRESH_VI("money_left_cookie", FRCMD_INIT,
	FrCheckoutDlg::OnMoneyLeft_CookieInit)
ON_FRESH_VI("num_static", FRCMD_INIT, FrCheckoutDlg::OnNumStaticInit)
ON_FRESH_VI("explane", FRCMD_INIT, FrCheckoutDlg::OnExplaneStaticInit)
ON_FRESH_VI("num", FRCMD_INIT, FrCheckoutDlg::OnNumInit)
ON_FRESH_BI("num", FRCMD_ENTERKEY, FrCheckoutDlg::OnNumEnterKey)
ON_FRESH_VI("num_prev", FRCMD_INIT, FrCheckoutDlg::OnNumPrevInit)
ON_FRESH_VV("num_prev", FRCMD_LBUTTONUP, FrCheckoutDlg::OnNumPrevLBtnUp)
ON_FRESH_VI("num_next", FRCMD_INIT, FrCheckoutDlg::OnNumNextInit)
ON_FRESH_VV("num_next", FRCMD_LBUTTONUP, FrCheckoutDlg::OnNumNextLBtnUp)
ON_FRESH_VI("days", FRCMD_INIT, FrCheckoutDlg::OnDaysInit)
ON_FRESH_VI("days2", FRCMD_INIT, FrCheckoutDlg::OnDays2Init)
ON_FRESH_VV("days", FRCMD_LBUTTONDOWN, FrCheckoutDlg::OnDaysBtnDown)
ON_FRESH_VV("days2", FRCMD_LBUTTONDOWN, FrCheckoutDlg::OnDays2BtnDown)
ON_FRESH_VI("coupon_pang", FRCMD_INIT, FrCheckoutDlg::OnCouponPangInit)
ON_FRESH_VI("coupon_cookie", FRCMD_INIT, FrCheckoutDlg::OnCouponCookieInit)
ON_FRESH_VI("coupon_icon_01", FRCMD_INIT, FrCheckoutDlg::OnCouponIcon1Init)
ON_FRESH_VV("coupon_icon_01", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponIcon1LBtnUp)
ON_FRESH_VV("coupon_icon_01", FRCMD_RBUTTONUP, FrCheckoutDlg::OnCouponIcon1RBtnUp)
ON_FRESH_VI("coupon_icon_02", FRCMD_INIT, FrCheckoutDlg::OnCouponIcon2Init)
ON_FRESH_VV("coupon_icon_02", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponIcon2LBtnUp)
ON_FRESH_VV("coupon_icon_02", FRCMD_RBUTTONUP, FrCheckoutDlg::OnCouponIcon2RBtnUp)
ON_FRESH_VI("coupon_icon_03", FRCMD_INIT, FrCheckoutDlg::OnCouponIcon3Init)
ON_FRESH_VV("coupon_icon_03", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponIcon3LBtnUp)
ON_FRESH_VV("coupon_icon_03", FRCMD_RBUTTONUP, FrCheckoutDlg::OnCouponIcon3RBtnUp)
ON_FRESH_VI("coupon_icon_04", FRCMD_INIT, FrCheckoutDlg::OnCouponIcon4Init)
ON_FRESH_VV("coupon_icon_04", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponIcon4LBtnUp)
ON_FRESH_VV("coupon_icon_04", FRCMD_RBUTTONUP, FrCheckoutDlg::OnCouponIcon4RBtnUp)
ON_FRESH_VI("coupon_icon_prev", FRCMD_INIT, FrCheckoutDlg::OnCouponPrevInit)
ON_FRESH_VV("coupon_icon_prev", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponPrevLBtnUp)
ON_FRESH_VI("coupon_icon_next", FRCMD_INIT, FrCheckoutDlg::OnCouponNextInit)
ON_FRESH_VV("coupon_icon_next", FRCMD_LBUTTONUP, FrCheckoutDlg::OnCouponNextLBtnUp)
END_FRESH_MSGMAP()

struct sDayInfo
{
	const char* name;
	int days;
};

#ifndef JP983_VS_DEBUG // D843: /Od nao expande o dllimport inline = __imp_ inexistente
__declspec(dllimport)
#endif
inline int DayCount(int d)
{
	return d;
}

// C583: nomes passam por K2L no inicializador como o 983 (00ceb350)
static sDayInfo days[6] = {
	{ K2L_Compatibility("1\xc0\xcf"),                DayCount(1)   },
	{ K2L_Compatibility("7\xc0\xcf"),                DayCount(7)   },
	{ K2L_Compatibility("15\xc0\xcf"),               DayCount(15)  },
	{ K2L_Compatibility("30\xc0\xcf"),               DayCount(30)  },
	{ K2L_Compatibility("365\xc0\xcf"),              DayCount(365) },
	{ K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), DayCount(0)   }
};

FrCheckoutDlg::FrCheckoutDlg()
{
	m_pCheckoutBtn = NULL;
	m_pCart = NULL;
	m_pPrice = NULL;
	m_pMyMoney = NULL;
	m_pMoneyLeft = NULL;
	m_pPriceCookie = NULL;
	m_pMyMoneyCookie = NULL;
	m_pMoneyLeftCookie = NULL;
	m_pNumStatic = NULL;
	m_pExplane = NULL;
	m_pNum = NULL;
	m_pNumPrev = NULL;
	m_pNumNext = NULL;
	m_pCouponPang = NULL;
	m_pCouponCookie = NULL;
	for (int i = 0; i < 4; i++)
		m_pCouponIcon[i] = NULL;
	m_pCouponPrev = NULL;
	m_pCouponNext = NULL;
	m_bHasOption = false;
	m_bUnderLevel = false;
	m_bOverLevel = false;
	m_bMoneyOk = false;
	m_bCookieOk = false;
	m_bCookie = false;
	m_buyConfirmKind = 0;
	m_count = 1;
	m_money = 0;
	m_cookie = 0;
	m_cashRemain = 0;
	m_bCashDue = false;
	m_coupons.clear();
	m_bHasCoupons = true;
	m_selCoupon = -1;
	m_couponPage = 0;
	m_couponPang = 0;
	m_couponCookie = 0;
	m_couponType = 0;
	m_childCount = 0;
	memset(m_child, 0, sizeof(m_child));
	m_pDaysItem = NULL;
	m_pDays2Item = NULL;
	m_bBuySent = false;
	// 983: construtor 00aa1af0, operator new(0x400) em 00aa2520, GetGuidSelectedCoupon 00a9f680
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_pNum) == 0x138);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_cart) == 0x17c);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_bCookie) == 0x18b);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_coupons) == 0x1cc);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_bHasCoupons) == 0x1dc);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_selCoupon) == 0x1e0);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_couponPang) == 0x1e8);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_bBuySent) == 0x1fc);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_childCount) == 0x2a8);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_caddieIffMap) == 0x3f0);
	C_ASSERT_VC10(offsetof(FrCheckoutDlg, m_child) == 0x2b0);
	C_ASSERT_VC10(sizeof(FrCheckoutDlg) == 0x400);
}

inline FrCheckoutDlg::MiniBtn::MiniBtn()
{
}

std::list<FrCheckoutDlg::CartItem>& FrCheckoutDlg::Cart()
{
	return m_cart;
}

void FrCheckoutDlg::Process()
{
	if (m_pNum)
	{
		int n = atoi(m_pNum->GetLine(1, false));
		if (n != m_count)
		{
			SetPrice(n);
			UpdateCouponPrice(); // 983 00a9db40: jmp 00a9ac60
		}
	}
}

void FrCheckoutDlg::RecalculateTotalCookie(__int64 money, __int64 cookie,
	bool bCookie)
{
	char local[32];

	m_price = 0;
	m_priceCookie = 0;
	for (std::list<CartItem>::iterator it = m_cart.begin(); it != m_cart.end();
		++it)
	{
		CartItem& item = *it;
		__int64 price = 0xffffffff;

		if (m_bCookie && (item.pItem->c.TypeId & 0xfc000000) == 0x8000000)
		{
			IFF_STRUCT::sPart* pPart =
				ItemManager()->FindPart(item.pItem->c.TypeId);
			if (pPart && pPart->RentalPrice > 0)
				m_price += pPart->RentalPrice;
			continue;
		}

		// 983 00a9e360: skin 5/6 e o resto sem MiniBtn = GetItemSalePrice (00a1f680)
		int commonCount = 0;
		unsigned long tid = item.pItem->c.TypeId;
		unsigned int type = tid >> 26;
		if (type == 0xe)
		{
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(tid);
			unsigned int sub = tid >> 22 & 0xf;
			if (sub == 5 || sub == 6)
				commonCount = 1;
			else if (!pSkin->c.IsCash)
				m_price += pSkin->COM[item.option];
			else
				m_priceCookie += pSkin->COM[item.option];
		}
		else if (type == 8)
		{
			IFF_STRUCT::sCadItem* pCad = ItemManager()->FindCadItem(tid);
			if (!pCad->c.IsCash)
				m_price += pCad->COM[item.option];
			else
				m_priceCookie += pCad->COM[item.option];
		}
		else if (type == 0x10)
		{
			// 983: mascote de cookie sem TimeFlag (+0x82) cobra o Price (+0x74)
			IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
			if (!pMascot->c.IsCash)
				m_price += pMascot->COM[item.option];
			else if (pMascot->c.TimeFlag == 0) // 983 00a9e50f: cmp byte [+0x82]
				m_priceCookie += pMascot->c.Price;
			else
				m_priceCookie += pMascot->COM[item.option];
		}
		else if (type == 6 && (tid & 0x1f00000) == 0x100000)
		{
			IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem(tid);
			if (!pIffItem->c.IsCash)
				m_price += pIffItem->COM[item.option];
			else
				m_priceCookie += pIffItem->COM[item.option];
		}
		else if (m_childCount < 1)
		{
			commonCount = 1;
			if (type == 6)
			{
				IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem(tid);
				if (pIffItem && pIffItem->COM[0] > 0)
					commonCount = pIffItem->COM[0];
			}
		}
		else
		{
			MiniBtn& btn = m_child[item.option];
			if (btn.type1 == 0)
			{
				if (m_child[item.option].type0 != 0)
					price =
						(unsigned int)btn.value0 | (__int64)btn.value1 << 32;
			}
			else
				price = (unsigned int)btn.value2;
			if (!btn.flag)
				m_price += price;
			else
				m_priceCookie += price;
		}

		if (commonCount)
		{
			unsigned int p = ItemManager()->GetItemSalePrice(tid, commonCount, 0);
			if (!item.pItem->c.IsCash)
				m_price += p;
			else
				m_priceCookie += p;
		}
	}

	if (bCookie)
	{
		bool bMoneyDue = false;
		bool bCookieDue = false;
		if (m_pPrice)
		{
			__int64 need = m_price - m_couponPang;
			if (need < 0)
				need = 0;
			MakePriceStrA(local, need);
			strcat(local, K2L_Compatibility("\xc6\xce"));
			m_pPrice->SetLine(1, local, 0, false, 0);
			if (m_price != 0)
				bMoneyDue = true;
		}
		if (m_pPriceCookie)
		{
			__int64 diff = m_priceCookie - m_couponCookie;
			__int64 need = diff;
			if (need < 0)
				need = 0;
			sprintf(local, K2L_Compatibility("%I64d\xc4\xed\xc5\xb0"), need);
			m_pPriceCookie->SetLine(1, local, 0, false, 0);
			if (diff != 0)
			{
				bCookieDue = true;
				m_cashRemain = Doc()->RemainOwnCash(Doc()->m_provType, diff);
			}
		}
		if (bMoneyDue && !bCookieDue)
		{
			m_bCashDue = false;
			m_cashRemain = 0;
		}
		else
			m_bCashDue = true;
		if (m_pMyMoney)
		{
			MakePriceStrA(local, money);
			strcat(local, K2L_Compatibility("\xc6\xce"));
			m_pMyMoney->SetLine(1, local, 0, false, 0);
		}
		if (m_pMyMoneyCookie)
		{
			sprintf(local, K2L_Compatibility("%I64d\xc4\xed\xc5\xb0"), cookie);
			m_pMyMoneyCookie->SetLine(1, local, 0, false, 0);
		}
		UpdateCouponPrice(); // 983 00a9e8d9: so com bCookie
	}
	SetPrice(1);
}

void FrCheckoutDlg::Checkout(__int64 money, __int64 cookie, bool bCookie)
{
	m_bCookie = bCookie;
	m_bHasOption = false;
	m_bUnderLevel = false;
	m_bOverLevel = false;
	m_bMoneyOk = false;
	m_bCookieOk = false;
	m_childCount = 0;
	memset(m_child, 0, sizeof(m_child));
	if (m_pCart)
	{
		m_pCart->ClearItem();
		for (std::list<CartItem>::iterator it = m_cart.begin();
			it != m_cart.end(); ++it)
		{
			CartItem& item = *it;
			FixCartItem(item);
			m_pCart->AddItem(&item);
			if (item.pItem->c.InStock)
			{
				if (item.pItem->c.IsUnderLvl &&
					item.pItem->c.Level < Doc()->m_myInfo.stat.Level)
					m_bOverLevel = true;
				else if (!item.pItem->c.IsUnderLvl &&
					item.pItem->c.Level > Doc()->m_myInfo.stat.Level)
					m_bUnderLevel = true;
			}
			if (item.count != 0)
				m_bHasOption = true;
		}
	}
	m_bDaysFlag = true;
	m_pDays->SetVisible(false);
	m_bDays2Flag = true;
	m_pDays2->SetVisible(false);
	m_money = money;
	m_cookie = cookie;
	RecalculateTotalCookie(money, cookie, true);

	std::list<CartItem>::iterator first = m_cart.begin();
	bool bNum = false;
	if (((*first).pItem->c.TypeId & 0xfc000000) == 0x18000000 &&
		!((*first).pItem->c.TypeId & 0x2000000))
	{
		IFF_STRUCT::sItem* pItem =
			ItemManager()->FindItem((*first).pItem->c.TypeId);
		if (pItem->COM[0] > 0)
		{
			bNum = true;
			if (m_pNum)
				m_pNum->SetLine(1, MakeStr("%d", pItem->COM[0]), 0, false, 0);
		}
	}
	if ((!((*first).pItem->c.TypeId & 0x2000000) &&
			((*first).pItem->c.TypeId & 0xfc000000) == 0x18000000 && !bNum &&
			m_cart.size() <= 1) ||
		(*first).pItem->c.TypeId == 0x1a000041 ||
		(*first).pItem->c.TypeId == 0x1a0000cc ||
		(*first).pItem->c.TypeId == 0x1a00020f ||
		// 983 00aa3415..00aa342c: esses itens tambem ficam com a quantidade
		FrCardRemove::IsCardRemoveItem((*first).pItem->c.TypeId) ||
		(*first).pItem->c.TypeId == 0x1a0000ab)
	{
		if (m_pNum)
		{
			WRect rect = m_pNum->GetRect();
			m_pNum->MoveWindow(WPoint(
				rect.x + (float)((*first).pItem->c.IsCash ? 50 : -50), rect.y));
		}
		if (m_pNumPrev)
		{
			WRect rect = m_pNumPrev->GetRect();
			m_pNumPrev->MoveWindow(WPoint(
				rect.x + (float)((*first).pItem->c.IsCash ? 50 : -50), rect.y));
		}
		if (m_pNumNext)
		{
			WRect rect = m_pNumNext->GetRect();
			m_pNumNext->MoveWindow(WPoint(
				rect.x + (float)((*first).pItem->c.IsCash ? 50 : -50), rect.y));
		}
	}
	else
	{
		if (m_pNumStatic)
			m_pNumStatic->SetVisible(false);
		if (m_pNum)
			m_pNum->SetVisible(false);
		if (m_pNumPrev)
			m_pNumPrev->SetVisible(false);
		if (m_pNumNext)
			m_pNumNext->SetVisible(false);
	}
	// 983 00aa3566: fim do Open
	BuildCouponList();
	ShowCouponControls();
	UpdateCouponIcons();
	UpdateCouponPrice();
}

void FrCheckoutDlg::FixCartItem(CartItem& item)
{
	// 983 00a9c950: item tipo 6 sub 0x100000 (pacote) escolhe a 1a opção com sItem.COM (+0xec)
	unsigned long tid = item.pItem->c.TypeId;
	if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x100000)
	{
		IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem(tid);
		if (pIffItem)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pIffItem->COM[i])
				{
					item.option = i;
					return;
				}
			}
			return;
		}
	}
	else switch (tid >> 26)
	{
	case 2:
	{
		IFF_STRUCT::sPart* pPart =
			ItemManager()->FindPart(item.pItem->c.TypeId);
		if (pPart && m_bCookie && pPart->RentalPrice > 0)
		{
			item.option = 7;
			return;
		}
		break;
	}
	case 0xe:
	{
		IFF_STRUCT::sSkin* pSkin =
			ItemManager()->FindSkin(item.pItem->c.TypeId);
		if (pSkin && !S5::ISCutinItem(item.pItem->c.TypeId))
		{
			for (int i = 0; i < 5; i++)
			{
				if (pSkin->COM[i])
				{
					item.option = i;
					return;
				}
			}
		}
		break;
	}
	case 8:
	{
		IFF_STRUCT::sCadItem* pCad =
			ItemManager()->FindCadItem(item.pItem->c.TypeId);
		if (pCad)
		{
			for (int i = 0; i < 5; i++)
			{
				if (pCad->COM[i])
				{
					item.option = i;
					return;
				}
			}
		}
		break;
	}
	case 0x10:
	{
		IFF_STRUCT::sMascot* pMascot =
			ItemManager()->FindMascot(item.pItem->c.TypeId);
		// 983 00a9cadb: só mascote de pang ou de cookie com TimeFlag (+0x82) tem opção
		if (pMascot && (!pMascot->c.IsCash || pMascot->c.TimeFlag != 0))
		{
			for (int i = 0; i < 5; i++)
			{
				if (pMascot->COM[i])
				{
					item.option = i;
					return;
				}
			}
		}
		break;
	}
	default:
	{
		m_childCount = ItemManager()->FindChildItemNumber(item.pItem->c.TypeId);
		if (m_childCount > 0)
		{
			memset(m_child, 0, sizeof(m_child));
			if (m_childCount > MAX_CHILD)
				m_childCount = MAX_CHILD;
			// the button pointer is set up only after the first bound check (as the original does)
			int i = 0;
			if (i < m_childCount)
				for (MiniBtn* pBtn = m_child; i < m_childCount; i++, pBtn++)
				{
					IFF_STRUCT::sChildItem* pChild =
						ItemManager()->FindChildItem(item.pItem->c.TypeId, i);
					if (!pChild || !pChild->IsDisplay)
						continue;
					MiniBtn& btn = *pBtn;
					if (pChild->NumberFlag)
					{
						sprintf(btn.text, "%d\xb0\xb3", pChild->Number);
						btn.flag = pChild->IsCash;
						btn.value0 = pChild->NumberPrice;
						btn.value1 = 0;
						btn.type0 = pChild->NumberFlag;
						btn.num0 = pChild->Number;
					}
					else if (pChild->TimeFlag)
					{
						switch (pChild->TimeFlag)
						{
						case 0:
							break;
						case 3:
							sprintf(btn.text, "%d\xbd\xc3\xb0\xa3",
								pChild->Time);
							break;
						case 4:
							sprintf(btn.text, "%d\xc0\xcf", pChild->Time);
							break;
						case 5:
							sprintf(btn.text, "%d\xb0\xb3\xbf\xf9",
								pChild->Time);
							break;
						default:
							sprintf(btn.text, "%d\xc0\xcf", pChild->Time);
							break;
						}
						btn.flag = pChild->IsCash;
						btn.value2 = pChild->TimePrice;
						btn.type1 = pChild->TimeFlag;
						btn.num1 = pChild->Time;
					}
				}
		}
		break;
	}
	}
}

void FrCheckoutDlg::SetPrice(int count)
{
	char buf[32];

	m_bCookieOk = m_bMoneyOk = true;
	if (count < 1)
		count = 1;
	else if (count > 99)
		count = 99;
	m_count = count;

	if (m_pPrice && m_pMyMoney && m_price != 0)
	{
		__int64 need = (__int64)count * m_price - m_couponPang;
		if (need < 0)
			need = 0;
		MakePriceStrA(buf, need);
		strcat(buf, K2L_Compatibility("\xc6\xce"));
		m_pPrice->SetLine(1, buf, 0, false, 0);

		__int64 fee = Doc()->m_myInfo.stat.i64Pang;
		__int64 total = (__int64)m_count * m_price;
		if (m_couponPang - total + fee < 0)
		{
			if (m_pMoneyLeft)
			{
				MakePriceStrA(buf, total - m_couponPang - fee);
				strcat(buf, K2L_Compatibility("\xc6\xce \xba\xce\xc1\xb7"));
				m_pMoneyLeft->SetLine(1, buf, 0xffff0000, false, 0);
			}
			m_bMoneyOk = false;
		}
		else if (m_pMoneyLeft)
		{
			__int64 have = m_couponPang;
			if (have > total)
				have = total;
			MakePriceStrA(buf, have - total + fee);
			strcat(buf, K2L_Compatibility("\xc6\xce"));
			m_pMoneyLeft->SetLine(1, buf, 0, false, 0);
		}
	}
	else if (m_pMoneyLeft)
	{
		m_pMoneyLeft->SetLine(1, m_pMyMoney->GetLine(1, false), 0, false, 0);
	}

	if (m_pPriceCookie && m_pMyMoneyCookie)
	{
		__int64 need = m_priceCookie - m_couponCookie;
		if (need < 0)
			need = 0;
		sprintf(buf, K2L_Compatibility("%I64d\xc4\xed\xc5\xb0"), need);
		m_pPriceCookie->SetLine(1, buf, 0, false, 0);

		__int64 fee = Doc()->m_cookie;
		__int64 left = m_couponCookie - m_priceCookie + fee;
		if (left < 0)
		{
			if (m_pMoneyLeftCookie)
			{
				MakePriceStrA(buf, m_priceCookie - m_couponCookie - fee);
				strcat(buf, K2L_Compatibility("\xc4\xed\xc5\xb0 \xba\xce\xc1\xb7"));
				m_pMoneyLeftCookie->SetLine(1, buf, 0xffff0000, false, 0);
			}
			m_bCookieOk = false;
		}
		else if (m_pMoneyLeftCookie)
		{
			__int64 have = m_couponCookie;
			if (have > m_priceCookie)
				have = m_priceCookie;
			sprintf(buf, K2L_Compatibility("%d\xc4\xed\xc5\xb0"), have - m_priceCookie + fee);
			m_pMoneyLeftCookie->SetLine(1, buf, 0, false, 0);
		}
	}
	else if (m_pMoneyLeftCookie)
	{
		m_pMoneyLeftCookie->SetLine(1, m_pMyMoneyCookie->GetLine(1, false), 0,
			false, 0);
	}

	if (m_pNum)
	{
		// JP 983 (00a9d10f): o texto do chat (CChatMsg 00e73efc) acompanha a quantidade antes do campo.
		CChatMsg::Instance()->SetChatText(MakeStr("%d", m_count), false);
		m_pNum->SetLine(1, MakeStr("%d", m_count), 0, false, 0);
	}
}

bool FrCheckoutDlg::OnChargeResult(int result, FrForm* form)
{
	// JP 983 (00a9d180): pagina de pagamento do JP com o id do jogador e o KID que o login mandou (CSharedDoc+0x190), aberta
	// pelo navegador do 983 com o tipo 0xc (pacote 0xa1); a loja e ator do CLobbyTask, a mensagem 0xe4 vai ao ator principal.
	// O 983 grava WS_VSCROLL (0x200000) em CBrowser+0x110 (m_dwStyle) antes de abrir (s103: CBrowser no layout do 983).
	if (result == 1)
	{
		char url[0x400] = { 0 };
		switch (Doc()->m_provType)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
		case 10:
		case 0xb:
		case 0xc:
		case 0xd:
			sprintf(url, "https://www3.pangya.jp/payment/login.aspx?ID=%s&KID=%s", Doc()->m_myInfo.info.sID,
				Doc()->m_paymentKID983.c_str());
			CBrowser::Instance()->SetStyle983(WS_VSCROLL);
			CBrowser::Instance()->OpenWeb983(url, false, 0xc);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xc2), 0, 0, 0, 0, 0));
			g_input->Reset();
			Close(FrOK, true);
			break;
		}
	}
	return true;
}

bool FrCheckoutDlg::OnConfirmResult(int result, FrForm* form)
{
	// JP 983 (00aa25f0; frente C, lote 14): cada item vai como registro de 0x25 bytes (sShopBuyItem983) com o preco
	// (CItemManager 00a1f680 = GetItemSalePrice; aluguel de peca com cookie: S5::RENTALPARTS::GetRentalPrice) e o pacote
	// termina com o cupom escolhido (00a9f680) em dword; o KR 645 mandava 0x10 bytes e nao tinha o cupom. Avisos ao ator
	// principal da tarefa (a loja e ator do CLobbyTask) e textos por K2L_Compatibility.
	if (result == 1 && !m_bBuySent)
	{
		FrListBox::ITEM_LIST& items = m_pCart->m_itemList;
		result = atoi(m_pNum->GetLine(1, false));
		sShopBuyItem983 buf;
		memset(&buf, 0, sizeof(buf));
		buf.guid = (unsigned long)-1;
		CartItem* first = (CartItem*)(*items.begin())->pData;
		if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
			Doc()->m_bHaveHalloweenItem &&
			(first->pItem->c.TypeId & 0xfc000000) == 0x8000000 &&
			(first->pItem->c.TypeId & 0x1800) == 0x1800)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb1\xb8\xb8\xc5\xc7\xd1 \xb9\xb0\xb0\xc7\xc0\xd4\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			return true;
		}
		if (m_childCount > 0)
		{
			if ((first->pItem->c.TypeId & 0xfc000000) == 0x18000000)
			{
				WSendPacket packet((enumClientPacket)0x1d);
				IFF_STRUCT::sItem* pItem =
					ItemManager()->FindItem(first->pItem->c.TypeId);
				if (m_child[first->option].type0)
					result = m_child[first->option].num0;
				buf.tid = first->pItem->c.TypeId;
				buf.days = m_child[first->option].num1;
				buf.option = (unsigned short)first->option;
				buf.count = result;
				if (first->pItem->c.IsCash)
				{
					buf.cookie = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
					buf.pang = 0;
				}
				else
				{
					buf.cookie = 0;
					buf.pang = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
				}
				packet.Encode1(0);
				packet.Encode2(1);
				packet.EncodeBuffer(&buf, sizeof(buf));
				packet.Encode4(GetGuidSelectedCoupon());
				packet.Send((eSendTo)0);
			}
			else
			{
				unsigned short count = (unsigned short)items.size();
				WSendPacket packet((enumClientPacket)0x1d);
				packet.Encode1(0);
				packet.Encode2(count);
				for (FrListBox::ITEM_LIST::iterator it = items.begin();
					it != items.end(); it++)
				{
					*first = *(CartItem*)(*it)->pData;
					buf.tid = first->pItem->c.TypeId;
					buf.days = m_child[first->option].num1;
					buf.option = (unsigned short)first->option;
					buf.count = result;
					if (first->pItem->c.IsCash)
					{
						buf.cookie = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
						buf.pang = 0;
					}
					else
					{
						buf.cookie = 0;
						buf.pang = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
					}
					packet.EncodeBuffer(&buf, sizeof(buf));
				}
				packet.Encode4(GetGuidSelectedCoupon());
				packet.Send((eSendTo)0);
			}
		}
		else
		{
			unsigned long typeId = first->pItem->c.TypeId;
			if (typeId >> 26 == 5)
			{
				IFF_STRUCT::sBall* pBall = ItemManager()->FindBall(typeId);
				if (pBall)
					result = pBall->COM[0];
			}
			else if (typeId >> 26 == 6 && (typeId & 0x2000000) &&
				typeId != 0x1a000041 && typeId != 0x1a0000cc &&
				typeId != 0x1a00020f && !FrCardRemove::IsCardRemoveItem(typeId) &&
				first->pItem->c.TypeId != 0x1a0000ab)
			{
				// 983: 0x1a00020f, os itens de tirar carta e 0x1a0000ab tambem ficam com a quantidade digitada
				IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(typeId);
				if (pItem)
					result = pItem->COM[0];
			}
			if (!Doc()->ConfirmInsertItem(first->pItem->c.TypeId, result))
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
					"notify", NULL);
				WRect rect = pForm->GetRect();
				pForm->SetRect(WRect(rect.x, rect.y, rect.w, rect.h));
				pForm->SetTimeLimit(12.0f);
				pForm->SetMessage(K2L_Compatibility(
					"\xc7\xf6\xc0\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xba\xb8\xc0\xaf\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb1\xb8\xc0\xd4 \xb6\xc7\xb4\xc2 \xbf\xc5\xb1\xe6 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					false);
				pForm->Open(NULL, 0x800); // 983: 0x800
				return true;
			}
			if (m_bCookie)
			{
				for (FrListBox::ITEM_LIST::iterator it = items.begin();
					it != items.end(); it++)
				{
					CartItem* pCart = (CartItem*)(*it)->pData;
					if (!S5::RENTALPARTS::IsRentableParts(
							pCart->pItem->c.TypeId))
					{
						AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23),
							(int)K2L_Compatibility("\xb1\xb8\xb8\xc5 \xc3\xb3\xb8\xae \xbf\xc0\xb7\xf9. \xc3\xb3\xc0\xbd\xba\xce\xc5\xcd \xb4\xd9\xbd\xc3 \xbd\xc3\xc0\xdb\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
							0, 0, 0, 0));
						return true;
					}
				}
			}
			if (items.size() > 1)
			{
				std::list<unsigned long> ids;
				for (FrListBox::ITEM_LIST::iterator it = items.begin();
					it != items.end(); it++)
				{
					CartItem* pCart = (CartItem*)(*it)->pData;
					ids.push_back(pCart->pItem->c.TypeId);
				}
				ids.sort();
				ids.unique();
				if (ids.size() != items.size())
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "notify", NULL);
					WRect rect = pForm->GetRect();
					pForm->SetRect(WRect(rect.x, rect.y, rect.w, rect.h));
					pForm->SetTimeLimit(12.0f);
					pForm->SetMessage(K2L_Compatibility(
						"\xc4\xab\xc6\xae \xb1\xb8\xb8\xc5 \xb0\xfa\xc1\xa4\xbf\xa1 \xb9\xae\xc1\xa6\xb0\xa1 \xb9\xdf\xbb\xfd\xc7\xcf\xbf\xa9 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xbb\xec \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xdc\xc7\xb0\xc0\xb8\xb7\xce \xb1\xb8\xb8\xc5\xb8\xa6 \xbd\xc3\xb5\xb5\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
						false);
					pForm->Open(NULL, 0);
					return true;
				}
			}
			// 983 (00aa2dd6): nas 35 pecas de 00a9b500 copia pItem+0x238 (dword) e strcpy de +0x23c, alem do sPart
			// (0x238 = registro do Part.iff): no no do std::map (0x24c, 00a21588) isso e _Color/_Isnil e lixo do heap.
			// Sem sentido para o servidor: vao zerados (o strcpy do 983 ainda pode estourar a pilha).
			unsigned short count = (unsigned short)items.size();
			WSendPacket packet((enumClientPacket)0x1d);
			packet.Encode1(m_bCookie);
			packet.Encode2(count);
			for (FrListBox::ITEM_LIST::iterator it = items.begin();
				it != items.end(); it++)
			{
				CartItem* pCart = (CartItem*)(*it)->pData;
				buf.tid = pCart->pItem->c.TypeId;
				buf.days = (unsigned short)days[pCart->option].days;
				buf.option = 0xffff;
				buf.count = result;
				if (pCart->pItem->c.IsCash)
				{
					if (m_bCookie)
					{
						buf.cookie = 0;
						buf.pang = S5::RENTALPARTS::GetRentalPrice(buf.tid);
					}
					else
					{
						buf.cookie = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
						buf.pang = 0;
					}
				}
				else
				{
					buf.cookie = 0;
					buf.pang = ItemManager()->GetItemSalePrice(buf.tid, result, buf.days);
				}
				packet.EncodeBuffer(&buf, sizeof(buf));
			}
			packet.Encode4(GetGuidSelectedCoupon());
			packet.Send((eSendTo)0);
		}
		m_bBuySent = true;
		IActor* pMain = AfxGetTask()->GetMainActor();
		pMain << MsgObject(NULL, M983(0xc0), 0, 0, 0, 0, 0);
		pMain << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
		Close(FrOK, true);
	}
	return true;
}

bool __fastcall CompareUsableCoupon(const FrCheckoutDlg::sCoupon& a,
	const FrCheckoutDlg::sCoupon& b)
{
	return a.usable > b.usable;
}

bool __fastcall ComparePeriodCoupon(const FrCheckoutDlg::sCoupon& a,
	const FrCheckoutDlg::sCoupon& b)
{
	return a.period < b.period;
}

unsigned long FrCheckoutDlg::GetGuidSelectedCoupon()
{
	// 983 (00a9f680): guid do cupom escolhido (o 645 zerava m_bHasCoupons e devolvia 0)
	if (m_bHasCoupons)
	{
		std::map<unsigned int, sCoupon>::iterator it;
		it = m_coupons.find(m_selCoupon);
		if (it != m_coupons.end())
		{
			sCoupon* p = &(*it).second;
			if (p != NULL)
				return p->guid;
		}
	}
	return 0;
}

void FrCheckoutDlg::OnCheckoutInit(int obj)
{
	m_pCheckoutBtn = DYNAMIC_CAST(FrButton, (FrWnd*)obj);
	m_pClock = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockRect =
		WRect(40.0f, 35.0f, (unsigned int)m_pClock->bi->bmiHeader.biWidth,
			(unsigned int)m_pClock->bi->bmiHeader.biHeight);
}


void FrCheckoutDlg::OnCheckoutLBtnUp()
{
	if (m_bUnderLevel || m_bOverLevel)
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb7\xb9\xba\xa7\xb7\xce\xb4\xc2 \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0);
		return;
	}

	FrListBox::ITEM_LIST& items = m_pCart->m_itemList;
	unsigned long characterId = 0;
	unsigned long hairStyleId = 0;
	for (FrListBox::ITEM_LIST::iterator it = items.begin(); it != items.end();
		++it)
	{
		CartItem* pCart = (CartItem*)(*it)->pData;
		// 983 00aa3696: caddie fora da lista visivel da loja (m_caddieIffMap) = "item nao existe"
		if ((pCart->pItem->c.TypeId >> 26) == 7 && m_caddieIffMap.size() != 0 &&
			m_caddieIffMap.find(pCart->pItem->c.TypeId) == m_caddieIffMap.end())
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xb1\xb8\xc0\xd4\xc7\xcf\xb7\xc1\xb0\xed \xc7\xcf\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0);
			return;
		}
		if ((pCart->pItem->c.TypeId & 0xfc000000) == 0x20000000)
		{
			IFF_STRUCT::sCadItem* pCad =
				ItemManager()->FindCadItem(pCart->pItem->c.TypeId);
			std::map<unsigned int, sCaddieInfo>::const_iterator ci;
			for (ci = Doc()->m_caddieMap.begin();
				ci != Doc()->m_caddieMap.end(); ++ci)
			{
				// 983 00aa3700: indice = bits 21..25 + bits 16..20
				if ((*ci).second.tid ==
					(((pCad->c.TypeId >> 21) & 0x1f) +
						((pCad->c.TypeId >> 16) & 0x1f) | 0x1c000000))
					break;
			}
			if (ci == Doc()->m_caddieMap.end())
			{
				AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xb2\xd9\xb9\xcc\xb1\xe2\xc7\xd2 \xc4\xb3\xb5\xf0\xb8\xa6 \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."),
					0, 0, 0, 0);
				return;
			}
			IFF_STRUCT::sCaddie* pCaddie =
				ItemManager()->FindCaddie((*ci).second.tid);
			if ((*ci).second.Rent_flag && (*ci).second.Remain_Date == 0 &&
				pCaddie && pCaddie->MonthlyFee != 0)
			{
				OpenCaddieInfo((*ci).second);
				return;
			}
		}
		eTitle title = (eTitle)pCart->pItem->c.TypeId;
		if ((title & 0xfc000000) == 0x38000000 &&
			(title & 0x3c00000) == 0x1800000)
		{
			Title* pTitle =
				g_TitleManager->GetTitle((eTitle)(title & 0x3fffff));
			if (pTitle && !pTitle->CanEquip())
			{
				AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xb1\xb8\xc0\xd4\xb0\xa1\xb4\xc9\xc7\xd1 \xc1\xb6\xb0\xc7\xc0\xbb \xb0\xae\xc3\xdf\xc1\xf6 \xbe\xca\xbe\xd2\xbd\xc0\xb4\xcf\xb4\xd9"),
					0, 0, 0, 0);
				return;
			}
		}
		if (pCart->count != 0)
		{
			unsigned long tid = pCart->pItem->c.TypeId;
			if ((tid >> 26) == 6 || (tid >> 26) == 5 ||
				((tid >> 26) == 9 &&
					(((tid >> 21) & 0x1f) == 6 || ((tid >> 21) & 0x1f) == 5)))
			{
				if (pCart->count >= 100000)
				{
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0);
					return;
				}
				if (m_childCount > 0)
				{
					if (m_child[pCart->option].type1)
					{
						AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
							(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
							0, 0, 0, 0);
						return;
					}
				}
				else if (ItemManager()->IsDisableDuplicateItem(tid))
				{
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0);
					return;
				}
			}
			else if ((tid >> 26) == 2)
			{
				// 983 00aa3c8e: roupa ja possuida nunca e comprada (o case 4 = "ja possui, comprar?" do 645
				// nao e alcancado no 983); cash com m_bCookie (+0x18b) = aviso de aluguel, o resto = d467b0
				if (pCart->pItem->c.IsCash && m_bCookie)
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb4\xeb\xbf\xa9\xc1\xdf\xc0\xce \xc0\xc7\xbb\xf3\xc0\xba \xc3\xdf\xb0\xa1\xb7\xce \xb4\xeb\xbf\xa9\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0);
				else
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0);
				return;
			}
			else if ((tid >> 26) != 0xf)
			{
				AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
					(int)K2L_Compatibility("\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xb9\xb0\xb0\xc7\xc0\xbb \xb1\xb8\xc0\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
					0, 0, 0, 0);
				return;
			}
		}
		if ((pCart->pItem->c.TypeId >> 26) == 0xf)
		{
			IFF_STRUCT::sHairStyle* pHair =
				ItemManager()->FindHairStyle(pCart->pItem->c.TypeId);
			m_buyConfirmKind = 1;
			std::map<unsigned int, sCharacterInfo>::const_iterator chi;
			for (chi = Doc()->m_charMap.begin(); chi != Doc()->m_charMap.end();
				++chi)
			{
				if ((chi->second.tid & 0x3ffffff) == pHair->cCharID &&
					chi->second.hairClr == pHair->cHairID)
				{
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xc7\xd8\xb4\xe7\xc4\xb3\xb8\xaf\xc5\xcd\xb4\xc2 \xbc\xb1\xc5\xc3\xc7\xd1 \xbf\xb0\xbb\xf6\xbe\xe0\xb0\xfa \xb0\xb0\xc0\xba \xbb\xf6\xc0\xc7 \xbf\xb0\xbb\xf6\xc0\xbb \xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9"),
						0, 0, 0, 0);
					return;
				}
			}
		}
		else if ((pCart->pItem->c.TypeId >> 26) == 8)
		{
			ItemManager()->FindCadItem(pCart->pItem->c.TypeId);
			m_buyConfirmKind = 2;
			std::map<unsigned int, sCaddieInfo>::const_iterator ci;
			for (ci = Doc()->m_caddieMap.begin();
				ci != Doc()->m_caddieMap.end(); ++ci)
			{
				if ((*ci).second.tid ==
					(((pCart->pItem->c.TypeId >> 21) & 0x1f) +
						((pCart->pItem->c.TypeId >> 16) & 0x1f) | 0x1c000000))
					m_buyConfirmKind = 3;
				if ((*ci).second.Remain_Partdate >= 0x4470)
				{
					AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
						(int)K2L_Compatibility("\xb4\xf5\xc0\xcc\xbb\xf3 \xb1\xb8\xc0\xd4\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
						0, 0, 0, 0);
					return;
				}
			}
		}
		else if ((pCart->pItem->c.TypeId >> 26) == 0x10)
		{
			std::map<unsigned int, sMascotInfo>::iterator mi;
			for (mi = Doc()->m_mascotMap.begin();
				mi != Doc()->m_mascotMap.end(); ++mi)
			{
				if ((*mi).second.tid == pCart->pItem->c.TypeId)
				{
					m_buyConfirmKind = 3;
					// 983 00aa3a8f: mascote com prazo (TimeFlag +0x82) e mais de 0x4470 horas restantes
					IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot((*mi).second.tid);
					__int64 remain;
					if (pMascot && pMascot->c.TimeFlag &&
						(remain = DOCJP()->GetRemainHours983(&(*mi).second.endDate)) != 0 &&
						remain >= 0x4470)
					{
						AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
							(int)K2L_Compatibility("\xb4\xf5\xc0\xcc\xbb\xf3 \xb1\xb8\xc0\xd4\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
							0, 0, 0, 0);
						return;
					}
					break;
				}
			}
		}
		// 983 00aa3afa: item com DateFlag == 2 passa fora do periodo de venda
		if (!ItemManager()->IsInSalePeriod(pCart->pItem->c.TypeId,
				Doc()->GetServerTime()) && pCart->pItem->c.DateFlag != 2)
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3\xc0\xbb \xc8\xae\xc0\xce\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
				0, 0, 0, 0);
			return;
		}
		if ((pCart->pItem->c.TypeId >> 26) == 1)
			characterId = pCart->pItem->c.TypeId;
		else if ((pCart->pItem->c.TypeId >> 26) == 0xf)
			hairStyleId = pCart->pItem->c.TypeId;
	}
	if (characterId && hairStyleId)
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc4\xb3\xb8\xaf\xc5\xcd\xbf\xcd \xbf\xb0\xbb\xf6\xbe\xe0\xc0\xba \xb5\xbf\xbd\xc3\xbf\xa1 \xb1\xb8\xb8\xc5\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0, 0, 0, 0);
		return;
	}

	if (!m_bMoneyOk)
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)K2L_Compatibility("\xc6\xce\xc0\xcc \xb8\xf0\xc0\xda\xb6\xf8\xb4\xcf\xb4\xd9.\n\xb0\xd4\xc0\xd3\xbf\xa1 \xc2\xfc\xbf\xa9\xc7\xd8\xbc\xad \xc6\xce\xc0\xbb \xb8\xf0\xc0\xb8\xbc\xbc\xbf\xe4."),
			0, 0, 0, 0);
		return;
	}
	if (!m_bCookieOk)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_okcancel");
		pForm->SetMessage(
			K2L_Compatibility("\\c0xffff0000\\c\xc4\xed\xc5\xb0\\c0xff000000\\c\xb0\xa1 \xb8\xf0\xc0\xda\xb6\xf8\xb4\xcf\xb4\xd9.\n\xc1\xf6\xb1\xdd \xc4\xed\xc5\xb0\xb8\xa6 \xc3\xe6\xc0\xfc\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			false);
		pForm->Open((FRESH_PFN_RESULT)&FrCheckoutDlg::OnChargeResult, 3);
		return;
	}

	std::string msg;
	// 983 00aa3fe0: texto do cupom em buffer de 0x104 (ebp-0x128); 128 estourava com nome de 64 + texto + preco
	char desc[0x104];
	if (m_bHasCoupons && m_selCoupon >= 0)
	{
		CartItem* first = (CartItem*)(*items.begin())->pData;
		char price[16] = "";
		if (m_price > 0)
		{
			__int64 total = (__int64)m_count * m_price;
			sprintf(price, K2L_Compatibility("%I64d\xc6\xce"),
				total > m_couponPang ? total - m_couponPang : 0);
		}
		else if (m_priceCookie > 0)
		{
			sprintf(price, K2L_Compatibility("%I64d\xc4\xed\xc5\xb0"),
				m_priceCookie > m_couponCookie ? m_priceCookie - m_couponCookie : 0);
		}
		else
			sprintf(price, K2L_Compatibility("0\xc4\xed\xc5\xb0"));
		sprintf(desc,
			K2L_Compatibility("\\c0xffff0000\\c%s\xc0\xbb(\xb8\xa6) \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb4\xc2\xb5\xa5 %s\xc0\xcc(\xb0\xa1) \xbc\xd2\xbf\xe4\xb5\xcb\xb4\xcf\xb4\xd9.\n\xc1\xa4\xb8\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
			first->pItem->c.Name, price);
		msg = desc;
	}
	else
	{
		switch (m_buyConfirmKind)
		{
		case 1:
			msg =
				K2L_Compatibility("\\c0xffff0000\\c\xbf\xb0\xbb\xf6\xbe\xe0\xc0\xba \xb1\xb8\xb8\xc5\xbf\xcd \xb5\xbf\xbd\xc3\xbf\xa1 \xbb\xe7\xbf\xeb\xc0\xcc \xb5\xcb\xb4\xcf\xb4\xd9.\n\xc7\xd8\xb4\xe7 \xc4\xb3\xb8\xaf\xc5\xcd\xc0\xc7 \xb8\xd3\xb8\xae\xbb\xf6\xc0\xbb \xbc\xb1\xc5\xc3\xc7\xd1 \xbb\xf6\xc0\xb8\xb7\xce \xbf\xb0\xbb\xf6\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?");
			break;
		case 2:
			msg =
				K2L_Compatibility("\\c0xffff0000\\c\xb1\xe2\xc1\xb8\xc0\xc7 \xc4\xb3\xb5\xf0\xb2\xd9\xb9\xcc\xb1\xe2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc0\xd6\xb4\xd9\xb8\xe9 \xb5\xa4\xbe\xee\xbe\xb2\xb0\xd4 \xb5\xcb\xb4\xcf\xb4\xd9.\n \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?");
			break;
		case 3:
			msg =
				K2L_Compatibility("\\c0xffff0000\\c\xbc\xb1\xc5\xc3\xc7\xcf\xbd\xc5 \xc0\xcf\xbc\xf6\xb8\xb8\xc5\xad \xb1\xe2\xb0\xa3\xc0\xcc \xbf\xac\xc0\xe5\xb5\xcb\xb4\xcf\xb4\xd9.\n \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?");
			break;
		case 4:
			msg =
				K2L_Compatibility("\\c0xffff0000\\c\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf6\xb0\xed \xc0\xd6\xb4\xc2 \xc0\xc7\xbb\xf3\xc0\xd4\xb4\xcf\xb4\xd9.\n \xc1\xa4\xb8\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?");
			break;
		default:
			msg =
				K2L_Compatibility("\\c0xffff0000\\c\xc1\xa4\xb8\xbb \xb1\xb8\xc0\xd4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?");
			break;
		}
	}
	// 983 00aa3620: o aviso final e o "notify_okcancel" pequeno (e2-41: orig-e241-2 mostra so "Are you sure you want to make this purchase?");
	// o 645 usava notify_okcancel_big com um rodape sobre prazo/quantidade que o 983 nao tem
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_okcancel");
	pForm->SetMessage(msg.c_str(), false);
	// 983 00aa425f: com o item 0x1a100002 no carrinho o OK abre uma 2a confirmacao (00aa3590)
	FRESH_PFN_RESULT pfn = (FRESH_PFN_RESULT)&FrCheckoutDlg::OnConfirmResult;
	for (FrListBox::ITEM_LIST::iterator it = items.begin(); it != items.end(); ++it)
	{
		if (((CartItem*)(*it)->pData)->pItem->c.TypeId == 0x1a100002)
		{
			pfn = (FRESH_PFN_RESULT)&FrCheckoutDlg::OnConfirm1a100002Result983;
			break;
		}
	}
	// 983 00aa4298: push 3 (o 0x803 e do 00aa3590)
	pForm->Open(pfn, 3);
}

bool FrCheckoutDlg::OnConfirm1a100002Result983(int result, FrForm* form)
{
	// 983 00aa3590: OK -> "notify_okcancel" com a string 0x13885 e Open(OnConfirmResult, 0x803)
	if (result == 1 && !m_bBuySent)
	{
		FrForm* pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_okcancel");
		pForm->SetMessage(GetGameString(0x13885), false);
		pForm->Open((FRESH_PFN_RESULT)&FrCheckoutDlg::OnConfirmResult, 0x803);
	}
	return true;
}

void FrCheckoutDlg::SetCaddieIffMap(
	const std::map<unsigned int, IFF_STRUCT::sCaddie>& m)
{
	// 983 00aa25b0: so copia se a origem tem algo
	if (m.size() != 0 && &m_caddieIffMap != &m)
		m_caddieIffMap = m;
}

void FrCheckoutDlg::OpenCaddieInfo(const sCaddieInfo& info)
{
	FrWnd* pWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("caddie_info");
	if (pWnd)
		return;

	__rtti_obj = NULL;
	FrCaddieInfoDlg* pDlg = CreateForm<FrCaddieInfoDlg>(g_pFresh->GetManager(),
		this, "caddie_info");
	if (pDlg)
	{
		pDlg->SetCaddieInfo(info);
		pDlg->Open(NULL, FrESCAPE | FrENTER);
	}
}

void FrCheckoutDlg::OnDaysInit(int obj)
{
	m_pDays = DYNAMIC_CAST(FrComboBox, (FrWnd*)obj);
}

void FrCheckoutDlg::OnDays2Init(int obj)
{
	m_pDays2 = DYNAMIC_CAST(FrComboBox, (FrWnd*)obj);
}

void FrCheckoutDlg::OnDaysBtnDown()
{
	if (m_childCount > 0)
	{
		const char* text = m_pDays->GetLine(1, false);
		for (int i = 0; i < m_childCount; i++)
		{
			if (_stricmp(m_child[i].text, text) == 0)
				m_pDaysItem->option = i;
		}
	}
	else
	{
		const char* text = m_pDays->GetLine(1, false);
		for (int i = 0; i < 5; i++)
		{
			if (_stricmp(days[i].name, text) == 0)
				m_pDaysItem->option = i;
		}
	}
	RecalculateTotalCookie(m_money, m_cookie, true);
}

void FrCheckoutDlg::OnDays2BtnDown()
{
	if (m_childCount > 0)
	{
		const char* text = m_pDays2->GetLine(1, false);
		for (int i = 0; i < m_childCount; i++)
		{
			if (_stricmp(m_child[i].text, text) == 0)
				m_pDays2Item->option = i;
		}
	}
	else
	{
		const char* text = m_pDays2->GetLine(1, false);
		for (int i = 0; i < 5; i++)
		{
			if (_stricmp(days[i].name, text) == 0)
				m_pDays2Item->option = i;
		}
	}
	RecalculateTotalCookie(m_money, m_cookie, true);
}

void FrCheckoutDlg::OnCartInit(int obj)
{
	m_pCart = DYNAMIC_CAST(FrListBox, (FrWnd*)obj);
}

// 983 00a9b500: as 35 pecas da tabela 00d46118 (sem o aviso de copia de UCC no carrinho)
static bool IsListedUccPart(unsigned long typeId)
{
	static const unsigned long parts[35] = {0x8006055, 0x800a035, 0x8044085,
		0x8048025, 0x80440b3, 0x8084044, 0x808a025, 0x80c408b, 0x80ca023,
		0x80c40b2, 0x810404f, 0x8108030, 0x8144084, 0x814a02c, 0x81440bf,
		0x8184087, 0x8188020, 0x81840c7, 0x81c404b, 0x81ca01f, 0x820404a,
		0x820a00e, 0x8204091, 0x8244064, 0x824a004, 0x824408f, 0x8284026,
		0x8284027, 0x828a012, 0x830401b, 0x830401e, 0x830a00c, 0x838400e,
		0x8384011, 0x838a006};
	return std::find(parts, parts + 35, typeId) != parts + 35;
}

void FrCheckoutDlg::OnCartOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!item)
		return;
	FrGraphicInterface* device = m_pCart->WndManager()->GetGDI();
	if (!device)
		return;
	CartItem* cart = (CartItem*)item->pData;
	const Bitmap* icon;
	unsigned int kind = cart->pItem->c.TypeId >> 26;
	if (kind == 0xe)
		icon = m_pCart->WndManager()->GetBitmap("SKINS", cart->pItem->c.Icon);
	else if (kind == 0xf)
		icon = m_pCart->WndManager()->GetBitmap("HAIRS", cart->pItem->c.Icon);
	else
	{
		icon = m_pCart->WndManager()->GetBitmap("ITEMS", cart->pItem->c.Icon);
		if (!icon)
			icon = m_pCart->WndManager()->GetBitmap("ITEMS_FASHION",
				cart->pItem->c.Icon);
	}
	if (icon)
	{
		WRect rect(item->pos.x, item->pos.y, (float)icon->Width(),
			(float)icon->Height());
		if ((cart->pItem->c.TypeId & 0xfc000000) != 0x8000000)
		{
			rect.x += 10.0f;
			rect.y += 20.0f;
		}
		device->DrawTexture(icon, rect, 0xffffffff, 0);
		if (m_childCount > 0 && m_child[cart->option].type1)
		{
			WPoint pos(item->pos.x, item->pos.y + 10.0f);
			device->DrawTexture(m_pClock, (WRect&)m_clockRect + pos, 0xffffffff,
				0);
		}
	}
	device->SetTextColor(0xff000000, 0xffffffff);
	device->SetTextStyle(1);
	float width =
		device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 14.0f), 0,
			"%s%s", cart->pItem->c.Name,
			cart->pItem->c.InStock == 3 ? K2L_Compatibility("(\xb1\xb8\xc0\xd4\xc0\xfc\xbf\xeb)")
										: "");
	// 983 00a9f8e5: o aviso de fim de venda so vale para item sem DateFlag == 2
	if (!ItemManager()->IsInSalePeriod(cart->pItem->c.TypeId,
			Doc()->GetServerTime()) &&
		cart->pItem->c.DateFlag != 2)
	{
		device->SetTextColor(0xffff0000, 0xffffffff);
		device->Print(
			WPoint((int)width + 90.0f + item->pos.x, item->pos.y + 14.0f), 0,
			K2L_Compatibility("(\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3 \xc1\xbe\xb7\xe1)"));
		device->SetTextColor(0xff000000, 0xffffffff);
	}
	device->SetTextStyle(0);

	// price label + unit
	int labelWidth;
	char unit[8];
	if (m_childCount > 0)
	{
		if (!m_bCookie && m_child[cart->option].flag)
		{
			labelWidth = float2int(
				device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 34.0f),
					0, K2L_Compatibility("\xb0\xa1    \xb0\xdd:   ")));
			strcpy(unit, K2L_Compatibility("\xc4\xed\xc5\xb0"));
		}
		else
		{
			labelWidth = float2int(
				device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 34.0f),
					0, K2L_Compatibility("\xb0\xa1    \xb0\xdd:   ")));
			strcpy(unit, K2L_Compatibility("\xc6\xce"));
		}
	}
	else
	{
		if (!m_bCookie && cart->pItem->c.IsCash)
		{
			labelWidth = float2int(
				device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 34.0f),
					0, K2L_Compatibility("\xb0\xa1    \xb0\xdd:   ")));
			strcpy(unit, K2L_Compatibility("\xc4\xed\xc5\xb0"));
		}
		else
		{
			labelWidth = float2int(
				device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 34.0f),
					0, K2L_Compatibility("\xb0\xa1    \xb0\xdd:   ")));
			strcpy(unit, K2L_Compatibility("\xc6\xce"));
		}
	}

	// price
	unsigned long tid = cart->pItem->c.TypeId;
	switch (tid >> 26)
	{
	case 0xe:
		if (S5::ISCutinItem(tid))
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(cart->pItem->c.TypeId);
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f),
				0, "%d%s", pSkin->c.Price, unit);
		}
		else if ((cart->pItem->c.TypeId & 0x3c00000) == 0x1800000)
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(cart->pItem->c.TypeId);
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f),
				0, "%d%s", pSkin->c.Price, unit);
		}
		else
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(cart->pItem->c.TypeId);
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f),
				0, "%d%s", (int)pSkin->COM[cart->option], unit);
		}
		break;
	case 8:
	{
		IFF_STRUCT::sCadItem* pCad = ItemManager()->FindCadItem(tid);
		device->Print(
			WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f), 0,
			"%d%s", (int)pCad->COM[cart->option], unit);
		break;
	}
	case 0x10:
	{
		IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(tid);
		// 983: mascote de cash sem prazo (+0x82 == 0) mostra c.Price (+0x74); as outras, COM[opcao]
		int mascotPrice = (pMascot->c.IsCash && pMascot->c.TimeFlag == 0) // 983 00a9fc5f
			? (int)pMascot->c.Price : (int)pMascot->COM[cart->option];
		device->Print(
			WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f), 0,
			"%d%s", mascotPrice, unit);
		break;
	}
	default:
		// 983 00a9fcbe: tipo 6 sub 0x100000 (pacote) = sItem.COM[opcao], antes dos botoes de compra
		if ((tid >> 26) == 6 && (tid & 0x1f00000) == 0x100000)
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f), 0,
				"%d%s", (int)ItemManager()->FindItem(tid)->COM[cart->option], unit);
		else if (m_childCount > 0)
		{
			__int64 price = 0;
			MiniBtn& btn = m_child[cart->option];
			if (btn.type0)
				price = (unsigned int)btn.value0 | (__int64)btn.value1 << 32;
			else if (btn.type1)
				price = (unsigned int)btn.value2;
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f),
				0, "%I64d%s", price, unit);
		}
		else if (m_bCookie)
		{
			device->Print(
				WPoint(labelWidth + 90.0f + item->pos.x, item->pos.y + 34.0f),
				0, "%d%s", S5::RENTALPARTS::GetRentalPrice(tid), unit);
		}
		else
		{
			float x = (float)labelWidth;
			float priceX = x + 90.0f;
			int priceWidth = float2int(
				device->Print(WPoint(priceX + item->pos.x, item->pos.y + 34.0f),
					0, "%d%s", cart->pItem->c.Price, unit));
			// 983 00a9fe82: item com DateFlag == 2 fora do periodo de venda nao mostra o preco riscado
			if (cart->pItem->c.SalePrice &&
				cart->pItem->c.SalePrice < cart->pItem->c.Price &&
				!(cart->pItem->c.DateFlag == 2 &&
					!ItemManager()->IsInSalePeriod(cart->pItem->c.TypeId,
						Doc()->GetServerTime())))
			{
				// strike the regular price through and print the sale price after it
				x = priceWidth + x;
				float endX = x + 92.0f;
				WPoint end(endX + item->pos.x,
					float2int(device->GetFontHeight() * 0.5f) + 34.0f +
						item->pos.y);
				float startX = priceX - 2.0f;
				device->Line(WPoint(startX + item->pos.x,
								 float2int(device->GetFontHeight() * 0.5f) +
									 34.0f + item->pos.y),
					end, 0xff000000, 0xff000000);
				device->SetTextColor(0xffff0000, 0xffffffff);
				device->Print(
					WPoint(x + 90.0f + item->pos.x, item->pos.y + 34.0f), 0,
					" %d%s", cart->pItem->c.SalePrice, unit);
				device->SetTextColor(0xff000000, 0xffffffff);
			}
		}
		break;
	}

	// sale period
	float y = 54.0f;
	_SYSTEMTIME* saleStart = NULL;
	_SYSTEMTIME* saleEnd = NULL;
	if (ItemManager()->HasSalePeriod(cart->pItem->c.TypeId, &saleStart,
			&saleEnd) &&
		saleStart->wYear && saleEnd->wYear)
	{
		device->SetTextColor(0xffff0000, 0xffffffff);
		device->Print(WPoint(item->pos.x + 90.0f, item->pos.y + 54.0f), 0,
			MakeStr(
				K2L_Compatibility("\xc6\xc7\xb8\xc5\xb1\xe2\xb0\xa3: %d\xbf\xf9 %d\xc0\xcf ~ %d\xbf\xf9 %d\xc0\xcf"),
				saleStart->wMonth, saleStart->wDay, saleEnd->wMonth,
				saleEnd->wDay));
		device->SetTextColor(0xff000000, 0xffffffff);
		y = 74.0f;
		DaysCombo* combo = (DaysCombo*)&m_pDays;
		for (int i = 2; i != 0; i--, combo++)
		{
			static WRect rect = combo->pCombo->GetRect();
			// 983 00aa0115: x = janela + 170, y = linha do carrinho + 74 + 18 (nao a posicao guardada)
			combo->pCombo->SetRect(WRect(GetRect().x + 170.0, item->pos.y + 74.0 + 18.0,
				rect.w, rect.h));
		}
	}

	// level
	const Bitmap* level = g_pFresh->GetManager()->GetBitmap("LEVELS",
		MakeStr("level_%03d", cart->pItem->c.Level + 1));
	if (level)
	{
		WRect rect(device->GetTextExtendOld983(K2L_Compatibility("\xb7\xb9    \xba\xa7:")) +
				item->pos.x + 102.0f,
			y + item->pos.y - 3.0f, (float)level->Width(),
			(float)level->Height());
		device->DrawTexture(level, rect, 0xffffffff, 0);
		device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
			K2L_Compatibility("\xb7\xb9    \xba\xa7:"));
		bool bUnder = cart->pItem->c.IsUnderLvl;
		const char* cond = bUnder ? K2L_Compatibility("\xc0\xcc\xc7\xcf") : K2L_Compatibility("\xc0\xcc\xbb\xf3");
		if ((!bUnder && cart->pItem->c.Level > Doc()->m_myInfo.stat.Level) ||
			(bUnder && cart->pItem->c.Level < Doc()->m_myInfo.stat.Level))
		{
			device->Print(WPoint(rect.w + rect.x + 5.0f, y + item->pos.y), 0,
				cond);
			device->SetTextColor(0xffff0000, 0xffffffff);
			float textY = y + item->pos.y;
			device->Print(
				WPoint(device->GetTextExtendOld983(cond) + rect.w + rect.x + 10.0f,
					textY),
				0, K2L_Compatibility("(\xb1\xb8\xc0\xd4\xba\xd2\xb0\xa1)"));
		}
		else
			device->Print(WPoint(rect.w + rect.x + 5.0f, y + item->pos.y), 0,
				cond);
	}
	y += 20.0f;
	// 983: volta ao preto depois do bloco de nivel (o aviso vermelho de compra bloqueada nao passa adiante)
	device->SetTextColor(0xff000000, 0xffffffff);

	// period / purchase option
	switch (cart->pItem->c.TypeId >> 26)
	{
	case 7:
	{
		device->SetTextColor(0xff000000, 0xffffffff);
		device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
			K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:"));
		device->SetTextColor(0xffff0000, 0xffffffff);
		IFF_STRUCT::sCaddie* pCaddie =
			ItemManager()->FindCaddie(cart->pItem->c.TypeId);
		if (pCaddie && pCaddie->MonthlyFee == 0)
			device->Print(WPoint(device->GetTextExtendOld983(
									 K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:")) +
								  95.0f + item->pos.x,
							  y + item->pos.y),
				0, K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"));
		else
			device->Print(WPoint(device->GetTextExtendOld983(
									 K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:")) +
								  95.0f + item->pos.x,
							  y + item->pos.y),
				0,
				K2L_Compatibility("\xb1\xb8\xc0\xd4\xc0\xcf\xb7\xce\xba\xce\xc5\xcd %d\xc0\xcf \xc8\xc4 \xc0\xe7\xb0\xed\xbf\xeb"),
				30);
		if (cart->count)
		{
			device->SetTextColor(0xffff0000, 0xffffffff);
			float countY = y;
			if ((cart->pItem->c.TypeId & 0xfc000000) == 0x1c000000)
				countY += 20.0f;
			device->Print(WPoint(item->pos.x + 90.0f, countY + item->pos.y), 0,
				K2L_Compatibility(" (\xc0\xcc\xb9\xcc \xb0\xa1\xc1\xf8 \xbe\xc6\xc0\xcc\xc5\xdb \xbc\xf6 %d)"),
				cart->count);
			device->SetTextColor(0xff000000, 0xffffffff);
		}
		break;
	}
	case 8:
	{
		device->SetTextColor(0xff000000, 0xffffffff);
		device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
			K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:"));
		device->SetTextColor(0xffff0000, 0xffffffff);
		if (cart->count)
		{
			device->SetTextColor(0xffff0000, 0xffffffff);
			device->Print(WPoint(item->pos.x + 230.0f, y + item->pos.y), 0,
				K2L_Compatibility("(\xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc1\xdf)"));
			device->SetTextColor(0xff000000, 0xffffffff);
		}
		IFF_STRUCT::sCadItem* pCad =
			ItemManager()->FindCadItem(cart->pItem->c.TypeId);
		cart->pCombo = &((DaysCombo*)&m_pDays)[item->no % 2 != 0];
		if (cart->pCombo->bDirty)
		{
			cart->pCombo->pCombo->ClearList();
			short* period = pCad->COM;
			for (sDayInfo* day = days; day < &days[4]; day++, period++)
			{
				if (*period && day != &days[1])
					cart->pCombo->pCombo->AddString(day->name);
			}
			cart->pCombo->bDirty = false;
			cart->option = 0;
			cart->pCombo->pCombo->SetLine(1, days[0].name, 0, false, 0);
			cart->pCombo->pCombo->SetVisible(true);
			RecalculateTotalCookie(0, 0, false);
		}
		cart->pCombo->pItem = cart;
		break;
	}
	case 0xe:
	{
		if (!S5::ISCutinItem(cart->pItem->c.TypeId) &&
			(cart->pItem->c.TypeId & 0x3c00000) != 0x1800000)
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
				K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:"));
			device->SetTextColor(0xffff0000, 0xffffffff);
			if (cart->count)
			{
				device->SetTextColor(0xffff0000, 0xffffffff);
				device->Print(WPoint(item->pos.x + 230.0f, y + item->pos.y), 0,
					K2L_Compatibility("(\xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc1\xdf)"));
				device->SetTextColor(0xff000000, 0xffffffff);
			}
		}
		IFF_STRUCT::sSkin* pSkin =
			ItemManager()->FindSkin(cart->pItem->c.TypeId);
		DaysCombo& combo = ((DaysCombo*)&m_pDays)[item->no % 2 != 0];
		if (combo.pItem)
			combo.bDirty =
				cart->pItem->c.TypeId != combo.pItem->pItem->c.TypeId;
		else
			combo.bDirty = true;
		cart->pCombo = &combo;
		if (cart->pCombo->bDirty)
		{
			cart->pCombo->pCombo->ClearList();
			short* period = pSkin->COM;
			for (sDayInfo* day = days; day < &days[5]; day++, period++)
			{
				if (*period)
					cart->pCombo->pCombo->AddString(day->name);
			}
			cart->pCombo->bDirty = false;
		}
		cart->pCombo->pCombo->SetLine(1, days[cart->option].name, 0, false, 0);
		if (S5::ISCutinItem(cart->pItem->c.TypeId))
		{
			cart->pCombo->pCombo->SetVisible(false);
			cart->pCombo->pItem = cart;
		}
		else if ((cart->pItem->c.TypeId & 0x3c00000) == 0x1800000)
		{
			cart->pCombo->pCombo->SetVisible(false);
			cart->pCombo->pItem = cart;
		}
		else
		{
			cart->pCombo->pCombo->SetVisible(true);
			cart->pCombo->pItem = cart;
		}
		break;
	}
	case 0x10:
	{
		device->SetTextColor(0xff000000, 0xffffffff);
		device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
			K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:"));
		device->SetTextColor(0xffff0000, 0xffffffff);
		if (cart->count)
		{
			device->SetTextColor(0xffff0000, 0xffffffff);
			device->Print(WPoint(item->pos.x + 230.0f, y + item->pos.y), 0,
				K2L_Compatibility("(\xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc1\xdf)"));
			device->SetTextColor(0xff000000, 0xffffffff);
		}
		IFF_STRUCT::sMascot* pMascot =
			ItemManager()->FindMascot(cart->pItem->c.TypeId);
		cart->pCombo = &((DaysCombo*)&m_pDays)[item->no % 2 != 0];
		if (cart->pCombo->bDirty)
		{
			cart->pCombo->pCombo->ClearList();
			// 983 00aa093a: mascote de cash sem prazo = so "sem limite"; SetLine so quando a lista e refeita
			if (pMascot->c.IsCash && pMascot->c.TimeFlag == 0) // 983 00aa0959: +0x82
			{
				cart->pCombo->pCombo->AddString(K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"));
				cart->pCombo->pCombo->SetLine(1, K2L_Compatibility("\xb9\xab\xc1\xa6\xc7\xd1"), 0,
					false, 0);
			}
			else
			{
				char* period = pMascot->COM;
				for (sDayInfo* day = days; day < &days[5]; day++, period++)
				{
					if (*period)
						cart->pCombo->pCombo->AddString(day->name);
				}
				cart->pCombo->pCombo->SetLine(1, days[cart->option].name, 0, false, 0);
			}
			cart->pCombo->bDirty = false;
		}
		cart->pCombo->pCombo->SetVisible(true);
		cart->pCombo->pItem = cart;
		break;
	}
	default:
		// 983 00aa0a1f: tipo 6 sub 0x100000 (pacote) = combo de periodo pelo sItem.COM (+0xec)
		if ((cart->pItem->c.TypeId >> 26) == 6 &&
			(cart->pItem->c.TypeId & 0x1f00000) == 0x100000)
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
				K2L_Compatibility("\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3:"));
			device->SetTextColor(0xffff0000, 0xffffffff);
			if (cart->count)
			{
				device->SetTextColor(0xffff0000, 0xffffffff);
				device->Print(WPoint(item->pos.x + 230.0f, y + item->pos.y), 0,
					K2L_Compatibility("(\xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc1\xdf)"));
				device->SetTextColor(0xff000000, 0xffffffff);
			}
			IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem(cart->pItem->c.TypeId);
			cart->pCombo = &((DaysCombo*)&m_pDays)[item->no % 2 != 0];
			if (cart->pCombo->bDirty)
			{
				cart->pCombo->pCombo->ClearList();
				short* period = pIffItem->COM;
				for (sDayInfo* day = days; day < &days[5]; day++, period++)
				{
					if (*period)
						cart->pCombo->pCombo->AddString(day->name);
				}
				cart->pCombo->pCombo->SetLine(1, days[cart->option].name, 0, false, 0);
				cart->pCombo->bDirty = false;
			}
			cart->pCombo->pCombo->SetVisible(true);
			cart->pCombo->pItem = cart;
		}
		else if (m_childCount > 0)
		{
			device->SetTextColor(0xff000000, 0xffffffff);
			device->Print(WPoint(item->pos.x + 90.0f, y + item->pos.y), 0,
				K2L_Compatibility("\xb1\xb8\xc0\xd4\xb9\xe6\xb9\xfd:"));
			device->SetTextColor(0xffff0000, 0xffffffff);
			if (cart->count)
			{
				device->SetTextColor(0xffff0000, 0xffffffff);
				device->Print(WPoint(item->pos.x + 230.0f, y + item->pos.y), 0,
					K2L_Compatibility("(\xc0\xcc\xb9\xcc \xbc\xd2\xc0\xaf\xc1\xdf)"));
				device->SetTextColor(0xff000000, 0xffffffff);
			}
			cart->pCombo = &((DaysCombo*)&m_pDays)[item->no % 2 != 0];
			if (cart->pCombo->bDirty)
			{
				cart->pCombo->pCombo->ClearLine();
				for (int i = 0; i < m_childCount; i++)
					cart->pCombo->pCombo->AddString(m_child[i].text);
				cart->pCombo->bDirty = false;
			}
			cart->pCombo->pCombo->SetLine(1, m_child[cart->option].text, 0,
				false, 0);
			cart->pCombo->pCombo->SetVisible(true);
			cart->pCombo->pItem = cart;
		}
		break;
	}

	// 983 00a9f6d0: sem o saldo depois da compra do 645 (provedores coreanos 2/4)
	if (IsLocalContent((localContentType_t)0x18 /*983: era 0x17; medido*/) &&
		!Doc()->m_bHaveHalloweenItem &&
		(cart->pItem->c.TypeId & 0xfc000000) == 0x8000000 &&
		(cart->pItem->c.TypeId & 0x1800) == 0x1800)
	{
		device->SetTextColor(0xffff0000, 0xffffffff);
		// 983 00aa0d5a: duas linhas (+310 e +330); a chave K2L da 2a termina em ".2" (00d46258)
		device->Print(WPoint(item->pos.x + 20.0f, item->pos.y + 310.0f), 0,
			K2L_Compatibility("*\xc8\xa3\xb9\xda\xb8\xf0\xc0\xda\xb4\xc2 \xb0\xe8\xc1\xa4\xb4\xe7 \xc7\xd1\xb9\xf8 \xb9\xdb\xbf\xa1 \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."));
		device->Print(WPoint(item->pos.x + 20.0f, item->pos.y + 330.0f), 0,
			K2L_Compatibility("*\xc8\xa3\xb9\xda\xb8\xf0\xc0\xda\xb4\xc2 \xb0\xe8\xc1\xa4\xb4\xe7 \xc7\xd1\xb9\xf8 \xb9\xdb\xbf\xa1 \xb1\xb8\xb8\xc5\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.2"));
		device->SetTextColor(0xff000000, 0xffffffff);
	}

	// 983 00aa0dd0: aviso de direitos da roupa UCC, na posicao da janela (+110; 2a linha +125), sem voltar ao preto
	if (IsLocalContent((localContentType_t)0x58) &&
		IsUccClothes(cart->pItem->c.TypeId) &&
		!IsListedUccPart(cart->pItem->c.TypeId))
	{
		WPoint pos(GetRect().x + 110.0, GetRect().y + 110.0);
		device->SetTextColor(0xffff0000, 0xffffffff);
		device->Print(pos, 0,
			K2L_Compatibility("\xc1\xa6""3\xc0\xda\xc0\xc7 \xb1\xc7\xb8\xae\xb8\xa6 \xc4\xa7\xc7\xd8/\xb9\xcc\xc7\xb3\xbe\xe7\xbd\xc4\xc0\xbb"));
		IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(cart->pItem->c.TypeId);
		pos.y = GetRect().y + 125.0;
		device->Print(pos, 0,
			pPart && (pPart->Category == 7 || pPart->Category == 9)
				? K2L_Compatibility("\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce\xc0\xc7 \xba\xb9\xbb\xe7 \xb9\xd7 \xb1\xb8\xc0\xd4\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0")
				: K2L_Compatibility("\xc7\xd8\xc4\xa5 \xbf\xec\xb7\xc1\xb0\xa1 \xc0\xd6\xb4\xc2 \xb5\xf0\xc0\xda\xc0\xce\xc0\xba \xc7\xc7\xc7\xd8 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0"));
	}

	// 983 00aa0ee9/00aa0ef0: traco e icones do cupom no fim de cada linha desenhada
	DrawCouponLine();
	DrawCouponIcons();
}

// 983 00a9b810: traco preto acima dos cupons (janela.y + 148, de x ate x + w - 10)
void FrCheckoutDlg::DrawCouponLine()
{
	if (!m_bHasCoupons || !m_pCart->WndManager()->GetGDI())
		return;
	const WRect& rect = GetRect();
	g_view->DrawLine2D(WPoint(rect.x, rect.y + 148.0f),
		WPoint(rect.x + rect.w - 10.0f, rect.y + 148.0f), 0xff000000, 0xff000000, 0);
}

// 983 00a9f410: icone do item de cada cupom visivel nos 4 botoes (+13,+15; +1 apertado);
// botao desligado = icone 0xffcccccc com shop_icon_disable em (-6,+22)
void FrCheckoutDlg::DrawCouponIcons()
{
	FrGraphicInterface* device;
	if (!m_bHasCoupons || !(device = m_pCart->WndManager()->GetGDI()))
		return;
	int n = m_coupons.size() < 4 ? (int)m_coupons.size() : 4;
	for (int i = 0; i < n; i++)
	{
		std::map<unsigned int, sCoupon>::iterator it = m_coupons.find(m_couponPage + i);
		if (it == m_coupons.end())
			continue;
		FrButton* btn = m_pCouponIcon[i];
		const WRect& bRect = btn->GetRect();
		unsigned long color = btn->IsEnabled() ? 0xffffffff : 0xffcccccc;
		float pressed = btn->GetStatus() == FrButton::PRESSED ? 1.0f : 0.0f;
		IFF_STRUCT::sItem* pIffItem = ItemManager()->FindItem((*it).second.itemId);
		if (!pIffItem)
			continue;
		const Bitmap* icon = m_pCart->WndManager()->GetBitmap("ITEMS", pIffItem->c.Icon);
		if (!icon)
			continue;
		device->DrawTexture(icon, WRect(pressed + bRect.x + 13.0f, bRect.y + 15.0f + pressed,
			(float)icon->Width(), (float)icon->Height()), color, 0);
		if (color != 0xffffffff)
		{
			const Bitmap* disable = m_pCart->WndManager()->GetBitmap("ITEMS", "shop_icon_disable");
			if (disable)
				device->DrawTexture(disable, WRect(bRect.x - 6.0f, bRect.y + 22.0f,
					(float)disable->Width(), (float)disable->Height()), color, 0);
		}
	}
}

void FrCheckoutDlg::OnPriceInit(int obj)
{
	m_pPrice = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnMyMoneyInit(int obj)
{
	m_pMyMoney = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnMoneyLeftInit(int obj)
{
	m_pMoneyLeft = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnPrice_CookieInit(int obj)
{
	m_pPriceCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnMyMoney_CookieInit(int obj)
{
	m_pMyMoneyCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnMoneyLeft_CookieInit(int obj)
{
	m_pMoneyLeftCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnNumStaticInit(int obj)
{
	m_pNumStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)obj);
}

void FrCheckoutDlg::OnExplaneStaticInit(int obj)
{
	m_pExplane = DYNAMIC_CAST(FrStatic, (FrWnd*)obj);
}

void FrCheckoutDlg::OnNumInit(int obj)
{
	m_pNum = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

bool FrCheckoutDlg::OnNumEnterKey(int)
{
	if (0)
	{
		IsUnderLevel();
		IsOverLevel();
	}
	return false;
}

void FrCheckoutDlg::OnNumPrevInit(int obj)
{
	m_pNumPrev = DYNAMIC_CAST(FrButton, (FrWnd*)obj);
	if (m_pNumPrev)
	{
		m_pNumPrev->SetPushDelay(0.0f);
		m_pNumPrev->UseDblClick(false); // 983 00a9bed0: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrCheckoutDlg::OnNumPrevLBtnUp()
{
	if (m_pNum)
	{
		int n = atoi(m_pNum->GetLine(1, false));
		if (n > 1)
		{
			SetPrice(n - 1);
			UpdateCouponPrice(); // 983 00a9d380/00a9d3c0: preco antes do cupom
		}
	}
}

void FrCheckoutDlg::OnNumNextInit(int obj)
{
	m_pNumNext = DYNAMIC_CAST(FrButton, (FrWnd*)obj);
	if (m_pNumNext)
	{
		m_pNumNext->SetPushDelay(0.0f);
		m_pNumNext->UseDblClick(false); // 983 00a9bf60: +0x44 |= 0x200 (FWS_NODBLCLICK)
	}
}

void FrCheckoutDlg::OnNumNextLBtnUp()
{
	if (m_pNum)
	{
		int n = atoi(m_pNum->GetLine(1, false));
		if (n < 99)
		{
			SetPrice(n + 1);
			UpdateCouponPrice(); // 983 00a9d380/00a9d3c0: preco antes do cupom
		}
	}
}

// 983: coupon_* (INIT 00a9bff0..00a9c360). O 645 nao tem cupom.
void FrCheckoutDlg::OnCouponPangInit(int obj)
{
	m_pCouponPang = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

void FrCheckoutDlg::OnCouponCookieInit(int obj)
{
	m_pCouponCookie = DYNAMIC_CAST(FrEdit, (FrWnd*)obj);
}

// 983 00a9c0d0..00a9c250: o direito abre o information do cupom
#define COUPON_ICON_INIT(n) \
void FrCheckoutDlg::OnCouponIcon##n##Init(int obj) \
{ \
	m_pCouponIcon[n - 1] = DYNAMIC_CAST(FrButton, (FrWnd*)obj); \
	if (m_pCouponIcon[n - 1]) \
		m_pCouponIcon[n - 1]->UseRightButton(true); \
} \
void FrCheckoutDlg::OnCouponIcon##n##LBtnUp() \
{ \
	SelectCoupon(n - 1); \
} \
void FrCheckoutDlg::OnCouponIcon##n##RBtnUp() \
{ \
	SelectCoupon(n - 1); \
	std::map<unsigned int, sCoupon>::iterator it = \
		m_coupons.find(m_couponPage + (n - 1)); \
	if (it != m_coupons.end()) \
		OpenCouponInformation(&it->second); \
}
COUPON_ICON_INIT(1)
COUPON_ICON_INIT(2)
COUPON_ICON_INIT(3)
COUPON_ICON_INIT(4)
#undef COUPON_ICON_INIT

void FrCheckoutDlg::OnCouponPrevInit(int obj)
{
	m_pCouponPrev = DYNAMIC_CAST(FrButton, (FrWnd*)obj);
	if (m_pCouponPrev)
	{
		m_pCouponPrev->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
		m_pCouponPrev->SetPushDelay(0.0f);
	}
}

void FrCheckoutDlg::OnCouponNextInit(int obj)
{
	m_pCouponNext = DYNAMIC_CAST(FrButton, (FrWnd*)obj);
	if (m_pCouponNext)
	{
		m_pCouponNext->UseDblClick(false); // 983: +0x44 |= 0x200 (FWS_NODBLCLICK)
		m_pCouponNext->SetPushDelay(0.0f);
	}
}

bool FrCheckoutDlg::HasCouponControls()
{
	return m_pCouponPang && m_pCouponCookie && m_pCouponPrev && m_pCouponNext &&
		m_pCouponIcon[0] && m_pCouponIcon[1] && m_pCouponIcon[2] && m_pCouponIcon[3];
}

// 983 00aa0f10/00aa0fe0/00aa10b0/00aa1180: icone apertado = cupom m_couponPage + icone, solto = nenhum
void FrCheckoutDlg::SelectCoupon(int icon)
{
	for (int i = 0; i < 4; i++)
		if (i != icon)
			m_pCouponIcon[i]->SetStatus(FrButton::NORMAL);
	if (m_pCouponIcon[icon]->GetStatus() == FrButton::PRESSED)
		m_selCoupon = m_couponPage + icon;
	else
		m_selCoupon = -1;
	SetCouponDiscount(m_selCoupon);
	UpdateCouponPrice();
	SetPrice(m_count);
}

// 983 00a9e900: direito num cupom abre a "information" dele (nome/nivel, tempo restante, descricao)
void FrCheckoutDlg::OpenCouponInformation(const sCoupon* pCoupon)
{
	if (!pCoupon)
		return;
	if (g_pFresh->GetManager()->GetDesktop()->FindChildByName("information"))
		return;

	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "information", NULL);
	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pCoupon->itemId);

	FrArea* pPortrait = DYNAMIC_CAST(FrArea, pForm->FindChildByName("portrait"));
	FrButton* pBtn = DYNAMIC_CAST(FrButton, pForm->FindChildByName("reset"));
	if (pBtn)
		pBtn->SetVisible(false);
	pBtn = DYNAMIC_CAST(FrButton, pForm->FindChildByName("icon_clubsetworkshop"));
	if (pBtn)
		pBtn->SetVisible(false);

	if (pPortrait)
		pPortrait->SetBgImg(pCommon ? pCommon->Icon : "hide");

	if (pCommon)
	{
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		FrArea* pLevel = DYNAMIC_CAST(FrArea, pForm->FindChildByName("level"));
		if (pName && pLevel)
		{
			// 983 d461e8 ("\xc0\xcc\xbb\xf3") / d46210 (IsUnderLvl: "\xc0\xcc\xc7\xcf")
			pName->AddText(MakeStr(K2L_Compatibility(pCommon->IsUnderLvl
				? "\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf\n"
				: "\xc0\xcc\xb8\xa7: %s\n\n\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3\n"),
				pCommon->Name), 0, true);
			pLevel->SetBgImg(MakeStr("level_%03d", pCommon->Level + 1));

			// tempo restante do cupom no inventario (983: guid = +4 do cupom)
			const std::list<sItemInfo>& inv = Doc()->GetConstMyItemList();
			for (std::list<sItemInfo>::const_iterator it = inv.begin(); it != inv.end(); ++it)
			{
				if ((*it).guid != pCoupon->guid)
					continue;
				if ((*it).ItemType)
				{
					int days = (*it).HourRemain / 24;
					int hours = (*it).HourRemain % 24;
					const char* text;
					if ((*it).Expired)
						// 983 d005e8
						text = K2L_Compatibility("\\c0xffff0000\\c\xbb\xe7\xbf\xeb\xb1\xe2\xb0\xa3\\c0xff000000\\c\xc0\xcc "
							"\\c0xffff0000\\c\xb8\xb8\xb7\xe1\\c0xff000000\\c\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.\n");
					else if (days == 0 && hours == 0)
						// 983 d005b0
						text = K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: 1\xbd\xc3\xb0\xa3\xb9\xcc\xb8\xb8\n");
					else
						// 983 d00598
						text = MakeStr(K2L_Compatibility("\xb3\xb2\xc0\xba\xbd\xc3\xb0\xa3: %d\xc0\xcf %d\xbd\xc3\xb0\xa3\n"),
							days, hours);
					pName->AddText(text, 0, true);
				}
				break;
			}
		}

		IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pCommon->TypeId);
		if (pDesc)
			pForm->SetMessage(pDesc->Desc, false);
	}
	else
	{
		// 983 d16c68 / d16c4c: item desconhecido
		FrEdit* pName = DYNAMIC_CAST(FrEdit, pForm->FindChildByName("name"));
		if (pName)
			pName->AddText(K2L_Compatibility(
				"\xc0\xcc\xb8\xa7: \xba\xd2\xb8\xed\n\n\xb7\xb9\xba\xa7: \xba\xd2\xb8\xed"), 0, true);
		pForm->SetMessage(K2L_Compatibility(
			"\xba\xa3\xc0\xcf\xbf\xa1 \xbd\xce\xc0\xce \xc1\xa4\xc3\xbc\xba\xd2\xb8\xed\xc0\xc7 \xc1\xb8\xc0\xe7"), false);
	}
	pForm->Open(NULL, 3);
}

// 983 00aa1250
void FrCheckoutDlg::OnCouponPrevLBtnUp()
{
	m_couponPage--;
	m_pCouponNext->Enable(true);
	if (m_couponPage < 1)
	{
		m_couponPage = 0;
		m_pCouponPrev->Enable(false);
	}
	UpdateCouponIcons();
}

// 983 00aa12a0
void FrCheckoutDlg::OnCouponNextLBtnUp()
{
	m_couponPage++;
	m_pCouponPrev->Enable(true);
	if ((unsigned int)m_couponPage >= m_coupons.size() - 4)
	{
		m_couponPage = (int)m_coupons.size() - 4;
		m_pCouponNext->Enable(false);
	}
	UpdateCouponIcons();
}

// 983 00a9f270: desconto do cupom escolhido
void FrCheckoutDlg::SetCouponDiscount(int key)
{
	if (!m_bHasCoupons)
		return;
	m_couponPang = 0;
	m_couponCookie = 0;
	m_couponType = 0;
	if (key < 0 || (unsigned int)key >= m_coupons.size())
		return;
	std::map<unsigned int, sCoupon>::iterator it = m_coupons.find(key);
	if (it == m_coupons.end() || !(*it).second.usable)
		return;
	switch ((*it).second.itemId & 0xfffff)
	{
	case 0x15:
	case 0x1d:
	case 0x1e:
		m_couponCookie = 5;
		m_couponType = 3;
		break;
	case 0x16:
	case 0x2b:
	case 0x2c:
		m_couponCookie = 10;
		m_couponType = 3;
		break;
	case 0x17:
	case 0x2d:
	case 0x2e:
		m_couponCookie = 0;
		m_couponPang = 0;
		m_couponType = 1;
		break;
	case 0x3a:
		m_couponCookie = 50000;
		m_couponType = 4;
		break;
	case 0x3c:
	case 0x4e:
		m_couponCookie = 20000;
		m_couponType = 4;
		break;
	}
}

// 983 00a9ac60: valor do desconto e textos coupon_pang/coupon_cookie
void FrCheckoutDlg::UpdateCouponPrice()
{
	if (!m_bHasCoupons)
		return;
	if (m_couponType == 1)
	{
		m_couponPang = (__int64)m_count * m_price;
		m_couponCookie = m_priceCookie;
	}
	else if (m_couponType == 4)
	{
		if (m_couponCookie == 20000)
			m_couponCookie = m_priceCookie * 20 / 100;
		else if (m_couponCookie == 50000)
			m_couponCookie = (m_priceCookie + 1) / 2;
		else
			m_couponCookie = 0;
	}
	if (m_pCouponPang)
	{
		char text[32] = {0};
		if (m_couponPang <= 0)
			sprintf(text, K2L_Compatibility("0\xc6\xce"));
		else
		{
			char price[0x20] = {0};
			MakePriceStrA(price, m_couponPang);
			sprintf(text, K2L_Compatibility("- %s\xc6\xce"), price);
		}
		m_pCouponPang->SetLine(1, text, 0, false, 0);
	}
	if (m_pCouponCookie)
	{
		char text[32] = {0};
		if (m_couponCookie <= 0)
			sprintf(text, K2L_Compatibility("0\xc4\xed\xc5\xb0"));
		else
			sprintf(text, K2L_Compatibility("- %I64d\xc4\xed\xc5\xb0"), m_couponCookie);
		m_pCouponCookie->SetLine(1, text, 0, false, 0);
	}
}

// 983 00a9f0c0: os 4 icones da pagina; o escolhido fica apertado
void FrCheckoutDlg::UpdateCouponIcons()
{
	if (!HasCouponControls() || !m_bHasCoupons)
		return;
	for (int i = 0; i < 4; i++)
	{
		std::map<unsigned int, sCoupon>::iterator it = m_coupons.find(m_couponPage + i);
		if (it == m_coupons.end())
			continue;
		sCoupon* c = &(*it).second;
		if (!c)
			continue;
		FrButton* pIcon;
		switch (i)
		{
		case 0: pIcon = m_pCouponIcon[0]; break;
		case 1: pIcon = m_pCouponIcon[1]; break;
		case 2: pIcon = m_pCouponIcon[2]; break;
		case 3: pIcon = m_pCouponIcon[3]; break;
		default: continue;
		}
		pIcon->Enable(c->usable != 0);
	}
	m_pCouponIcon[0]->SetStatus(FrButton::NORMAL);
	m_pCouponIcon[1]->SetStatus(FrButton::NORMAL);
	m_pCouponIcon[2]->SetStatus(FrButton::NORMAL);
	m_pCouponIcon[3]->SetStatus(FrButton::NORMAL);
	if (m_couponPage <= m_selCoupon && m_selCoupon < m_couponPage + 4)
	{
		switch (m_selCoupon - m_couponPage)
		{
		case 0: m_pCouponIcon[0]->SetStatus(FrButton::PRESSED); return;
		case 1: m_pCouponIcon[1]->SetStatus(FrButton::PRESSED); return;
		case 2: m_pCouponIcon[2]->SetStatus(FrButton::PRESSED); return;
		case 3: m_pCouponIcon[3]->SetStatus(FrButton::PRESSED); return;
		}
	}
}

// 983 00aa2f30: cupons do inventario (0x18xxxxxx com 0x2000000), um por unidade, ordem por validade
void FrCheckoutDlg::BuildCouponList()
{
	if (!HasCouponControls() || m_cart.size() > 1)
	{
		m_bHasCoupons = false;
		return;
	}
	std::list<sCoupon> found;
	FrListBox::ITEM_LIST& items = m_pCart->m_itemList;
	CartItem* first = (CartItem*)(*items.begin())->pData;
	const std::list<sItemInfo>& inv = Doc()->GetConstMyItemList();
	for (std::list<sItemInfo>::const_iterator it = inv.begin(); it != inv.end(); ++it)
	{
		unsigned long typeId = (*it).typeId;
		if ((typeId & 0xfc000000) != 0x18000000 || (typeId & 0x1f00000) ||
			!(typeId & 0x2000000))
			continue;
		sCoupon c;
		c.itemId = typeId;
		c.guid = (*it).guid;
		c.usable = 0;
		c.period = (*it).HourRemain;
		bool bCoupon = true;
		switch (typeId & 0xfffff)
		{
		case 0x15:
		case 0x16:
		case 0x1d:
		case 0x1e:
		case 0x2b:
		case 0x2c:
		case 0x3a:
		case 0x3c:
		case 0x4e:
			c.usable = first->pItem->c.IsCash ? 1 : 0;
			break;
		case 0x17:
		case 0x2d:
		case 0x2e:
		{
			unsigned long kind = first->pItem->c.TypeId >> 26;
			c.usable = (kind == 4 || kind == 9) ? 0 : 1;
			break;
		}
		default:
			bCoupon = false;
			break;
		}
		if (m_bCookie)
			c.usable = 0;
		if (bCoupon)
			for (int n = 0; n < (*it).Common[0]; n++)
				found.push_back(c);
	}
	if (found.empty())
	{
		m_bHasCoupons = false;
		return;
	}
	found.sort(CompareUsableCoupon);
	found.sort(ComparePeriodCoupon);
	m_coupons.clear();
	unsigned int key = 0;
	for (std::list<sCoupon>::iterator it = found.begin(); it != found.end(); ++it, ++key)
		m_coupons.insert(std::make_pair(key, *it));
}

// 983 00a9b530: sem cupom os icones e as setas somem e os textos ficam apagados (alpha 0.2)
void FrCheckoutDlg::ShowCouponControls()
{
	if (!HasCouponControls())
		return;
	if (!m_bHasCoupons)
	{
		for (int i = 0; i < 4; i++)
			m_pCouponIcon[i]->SetVisible(false);
		m_pCouponPrev->SetVisible(false);
		m_pCouponNext->SetVisible(false);
		m_pCouponPang->SetAlpha(0.2f);
		m_pCouponCookie->SetAlpha(0.2f);
		return;
	}
	if (m_pCart)
	{
		WRect rect = m_pCart->GetRect();
		rect.h -= 70.0f;
		m_pCart->SetRect(rect);
	}
	if (m_coupons.size() > 4)
	{
		for (int i = 0; i < 4; i++)
			m_pCouponIcon[i]->SetVisible(true);
		m_pCouponPrev->Enable(false);
	}
	else
	{
		for (int i = 0; i < 4; i++)
			m_pCouponIcon[i]->SetVisible(false);
		m_pCouponPrev->Enable(false);
		m_pCouponNext->Enable(false);
		for (int i = 0; i < (int)m_coupons.size() && i < 4; i++)
			m_pCouponIcon[i]->SetVisible(true);
		if (m_coupons.size() == 0)
		{
			m_pCouponPrev->SetVisible(false);
			m_pCouponNext->SetVisible(false);
		}
	}
	m_pCouponPang->SetAlpha(1.0f);
	m_pCouponCookie->SetAlpha(1.0f);
}
