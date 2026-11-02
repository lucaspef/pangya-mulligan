#include "minatl.h"
#include "fortuneopendlg.h"
#include "frlistbox.h"
#include "frstatic.h"
#include "frbutton.h"
#include "frarea.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fresh.h"
#include "packets/pouchprizelist983.h"

extern Fresh* g_pFresh;
extern WView* g_view; // 00f02458

namespace S5 { bool ISCutinItem(unsigned long typeId); }

IMPLEMENT_OBJECT(FrFortuneOpenDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrFortuneOpenDlg, FrForm)

ON_FRESH_VI("list", FRCMD_INIT, FrFortuneOpenDlg::OnItemListInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrFortuneOpenDlg::OnOkInit)
ON_FRESH_VI("caption1", FRCMD_INIT, FrFortuneOpenDlg::OnCaption1Init)
ON_FRESH_VI("caption1_1", FRCMD_INIT, FrFortuneOpenDlg::OnCaption11Init)
ON_FRESH_VI("caption2", FRCMD_INIT, FrFortuneOpenDlg::OnCaption2Init)
ON_FRESH_VI("list", FRCMD_OWNERDRAW, FrFortuneOpenDlg::OnItemListOwnerDraw)
ON_FRESH_VI("total_price", FRCMD_INIT, FrFortuneOpenDlg::OnTotalPriceInit)
ON_FRESH_VI("caption_img", FRCMD_INIT, FrFortuneOpenDlg::OnCaptionImgInit)

END_FRESH_MSGMAP()

FrFortuneOpenDlg::FrFortuneOpenDlg()
{
	m_typeId = 0;
	m_pItemList = NULL;
	m_pTotalPrice = NULL;
	m_pOk = NULL;
	m_pCaption1 = NULL;
	m_pCaption11 = NULL;
	m_pCaption2 = NULL;
	m_pCaptionImg = NULL;
}

FrFortuneOpenDlg::~FrFortuneOpenDlg()
{
}

void FrFortuneOpenDlg::OnItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrFortuneOpenDlg::OnOkInit(int param)
{
	m_pOk = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrFortuneOpenDlg::OnCaptionImgInit(int param)
{
	m_pCaptionImg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrFortuneOpenDlg::OnCaption1Init(int param)
{
	m_pCaption1 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pCaption1)
		m_pCaption1->SetCaption(K2L_Compatibility("\307\340\277\356\300\307 \272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
}

void FrFortuneOpenDlg::OnCaption11Init(int param)
{
	m_pCaption11 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrFortuneOpenDlg::OnCaption2Init(int param)
{
	m_pCaption2 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pCaption2)
		m_pCaption2->SetCaption(K2L_Compatibility("\310\271\265\346\307\321 \276\306\300\314\305\333\300\272 \270\266\300\314\267\353\277\241\274\255 \310\256\300\316\307\322 \274\366 \300\326\275\300\264\317\264\331."));
}

void FrFortuneOpenDlg::OnTotalPriceInit(int param)
{
	m_pTotalPrice = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pTotalPrice)
		m_pTotalPrice->SetVisible(false);
}

void FrFortuneOpenDlg::OnItemListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (pItem == NULL || pGDI == NULL)
		return;

	sPouchPrize* pPrize = (sPouchPrize*)pItem->pData;
	if (pPrize == NULL)
		return;

	IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(pPrize->dwTid);
	if (pCommon == NULL)
		return;

	const Bitmap* pBitmap = g_pFresh->GetBitmap(pCommon->Icon);
	if (pBitmap)
	{
		WRect rect(pItem->pos.x + 30.0,
			(m_pItemList->GetItemHeight() - pBitmap->Height()) * 0.5 +
				pItem->pos.y + 10.0,
			(float)pBitmap->Width(), (float)pBitmap->Height());
		pGDI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	}

	std::string name(pCommon->Name);
	pGDI->SetTextStyle(1);

	if ((pPrize->dwTid & 0xfc000000) == 0x8000000 &&
		name.find(K2L_Compatibility("("), 0) == std::string::npos &&
		name.find("(", 0, 1) == std::string::npos)
	{
		IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(
			((pPrize->dwTid >> 18) & 0xff) | 0x4000000);
		if (pChar)
		{
			name.append("(", 1);
			name += pChar->c.Name;
			name.append(")", 1);
		}
	}

	pGDI->Print(WPoint(pItem->pos.x + 120.0, pItem->pos.y + 20.0), 0, "%s",
		name.c_str());

	pGDI->SetTextStyle(0);

	if ((pCommon->TypeId & 0xfc000000) == 0x38000000)
	{
		if (!S5::ISCutinItem(pCommon->TypeId))
			pGDI->Print(WPoint(pItem->pos.x + 120.0, pItem->pos.y + 43.0), 0,
				K2L_Compatibility("\273\347\277\353\261\342\260\243 : %d\300\317"), pPrize->iCount);
	}
	else
	{
		pGDI->Print(WPoint(pItem->pos.x + 120.0, pItem->pos.y + 43.0), 0,
			K2L_Compatibility("\260\271   \274\366: %d\260\263"), pPrize->iCount);
	}
}

// 983 00aa95d0: encosta os captions e o botao no rodape da janela
void FrFortuneOpenDlg::UpdateCaptions()
{
	float left = m_rect.x;
	float top = m_rect.y;
	float height = m_rect.h;

	if (m_pOk)
	{
		WRect rect(g_view->GetWidth() * 0.5 - m_pOk->GetRect().w * 0.5,
			(height + top) - 70.0, m_pOk->GetRect().w, m_pOk->GetRect().h);
		m_pOk->SetRect(rect);
	}

	float x = 30.0;
	if (m_pCaption1)
	{
		WRect rect(x + left, (top + height) - 130.0, m_pCaption1->GetRect().w,
			m_pCaption1->GetRect().h);
		m_pCaption1->SetRect(rect);
	}

	if (strlen(m_pCaption11->GetCaption()) == 0)
	{
		if (m_pCaption2)
		{
			WRect rect(x + left, (top + height) - 110.0,
				m_pCaption2->GetRect().w, m_pCaption2->GetRect().h);
			m_pCaption2->SetRect(rect);
		}
	}
	else
	{
		WRect rect(x + left, (top + height) - 110.0, m_pCaption11->GetRect().w,
			m_pCaption11->GetRect().h);
		m_pCaption11->SetRect(rect);

		if (m_pCaption2)
		{
			WRect rect2(x + left, (top + height) - 90.0,
				m_pCaption2->GetRect().w, m_pCaption2->GetRect().h);
			m_pCaption2->SetRect(rect2);
		}
	}
}

// 983 00aa9880: altura pela quantidade de premios (maximo 4 linhas), centraliza e ajusta os captions
void FrFortuneOpenDlg::UpdateLayout()
{
	WRect rect = m_rect;
	int lines = (int)m_list.size();
	if (lines > 4)
		lines = 4;
	rect.h = lines * 96.0 + 180.0;
	float sw = g_view->GetWidth();
	float sh = g_view->GetHeight();
	rect.x = (sw - rect.w) * 0.5;
	rect.y = (sh - rect.h) * 0.5;
	SetRect(rect);
	UpdateCaptions();
}

void FrFortuneOpenDlg::SetItemList(unsigned long* items, int num)
{
	m_list.clear();
	m_pItemList->ClearItem();

	for (int i = 0; i < num; i++)
	{
		std::list<sPouchPrize>::iterator it = m_list.begin();
		while (it != m_list.end() && it->dwTid != items[i])
			++it;

		if (it == m_list.end())
		{
			sPouchPrize prize;
			prize.dwTid = items[i];
			prize.iCount = 1;
			m_list.push_back(prize);
		}
		else
		{
			it->iCount++;
		}
	}

	for (std::list<sPouchPrize>::iterator it = m_list.begin();
		it != m_list.end(); ++it)
		m_pItemList->AddItem(&*it);

	m_pItemList->SetKeyFocus(true);
	UpdateLayout();
}

// 983 00aaa190: nome do recurso do fundo + textos dos captions
struct sFortuneOpenText983
{
	char* name;
	char* cap1;
	char* cap2;

	void Set(const char* res);
};

void sFortuneOpenText983::Set(const char* res)
{
	strcpy_s(name, 0x40, res);
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(
		RbPouchPrizeList983::Instance983()->m_typeId);
	strcpy_s(cap1, 0xc0,
		MakeStr(K2L_Compatibility("%s \300\273(\270\246)"), pItem->c.Name));
	strcpy_s(cap2, 0xc0, K2L_Compatibility("\277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
}

void FrFortuneOpenDlg::SetItemNewList()
{
	m_pItemList->ClearItem();
	m_list.clear();

	RbPouchPrizeList983* pPrizes = RbPouchPrizeList983::Instance983();
	m_list = pPrizes->m_list;

	for (std::list<sPouchPrize>::iterator it = m_list.begin();
		it != m_list.end(); ++it)
		m_pItemList->AddItem(&*it);

	m_pItemList->SetKeyFocus(true);

	char name[0x40];
	char cap1[0xc0];
	char cap2[0xc0];
	name[0] = '\0';
	memset(name + 1, 0, sizeof(name) - 1);
	cap1[0] = '\0';
	memset(cap1 + 1, 0, sizeof(cap1) - 1);
	cap2[0] = '\0';
	memset(cap2 + 1, 0, sizeof(cap2) - 1);

	sFortuneOpenText983 text;
	text.name = name;
	text.cap1 = cap1;
	text.cap2 = cap2;

	switch (RbPouchPrizeList983::Instance983()->m_typeId)
	{
	case 0x1a00004d:
		strcpy(name, "fortune_lucky_ponta_res");
		strcpy(cap1, K2L_Compatibility("3\301\326\263\342\261\342\263\344 \272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000054:
		strcpy(name, "lucky_ponta2008_res");
		strcpy(cap1, K2L_Compatibility("2008\263\342\272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000b5:
		strcpy(name, "lucky_ponta2009_res");
		strcpy(cap1, K2L_Compatibility("2009\263\342\272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000bb:
		strcpy(name, "kooh_present_res");
		strcpy(cap1, K2L_Compatibility("\304\355\300\307\274\261\271\260\273\363\300\332\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000d4:
		strcpy(name, "pangya_box_res");
		strcpy(cap1, K2L_Compatibility("\306\316\276\337\300\307\263\257\261\342\263\344\273\363\300\332\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000d5:
		strcpy(name, "thank_you_res");
		strcpy(cap1, K2L_Compatibility("ThankYou\273\363\300\332\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000e5:
		strcpy(name, "hw_giftbox_res");
		strcpy(cap1, K2L_Compatibility("\307\322\267\316\300\251\261\342\307\301\306\256\271\332\275\272\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000138:
		strcpy(name, "hw_giftbox_res");
		strcpy(cap1, K2L_Compatibility("\307\322\267\316\300\251\261\342\307\301\306\256\271\332\275\272\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000e6:
		strcpy(name, "hw_cosbox_res");
		strcpy(cap1, K2L_Compatibility("\307\322\267\316\300\251\304\332\275\272\303\365\271\332\275\272\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000ec:
		strcpy(name, "5th_box_res");
		strcpy(cap1, K2L_Compatibility("5\301\326\263\342\261\342\263\344\273\363\300\332\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a0000fd:
		strcpy(name, "angel_egg_res");
		strcpy(cap1, K2L_Compatibility("\276\310\301\251\270\256\263\252\300\307\276\313\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a00010f:
		strcpy(name, "lucky_ponta2010_res");
		strcpy(cap1, K2L_Compatibility("2010\263\342\272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000116:
		strcpy(name, "undoukai_box_res");
		strcpy(cap1, K2L_Compatibility("\264\353\277\356\265\277\310\270\271\332\275\272\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000118:
		strcpy(name, "kooh_present2nd_res");
		strcpy(cap1, K2L_Compatibility("\304\355\300\307\274\261\271\260\273\363\300\3322ND\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a00013e:
		strcpy(name, "lucky_ponta2011_res");
		strcpy(cap1, K2L_Compatibility("2011\263\342\272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a00013f:
		strcpy(name, "kooh_selectbox_2011_res");
		strcpy(cap1, K2L_Compatibility("\304\355\300\307 \274\277\267\272\306\256 \271\332\275\272 2012\300\273 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000165:
		strcpy(name, "lucky_ponta2012_res");
		strcpy(cap1, K2L_Compatibility("2012\263\342\272\271\301\326\270\323\264\317\270\246 \277\255\276\356 \264\331\300\275\260\372 \260\260\300\272 \276\306\300\314\305\333\265\351\300\273 \310\271\265\346\307\337\275\300\264\317\264\331."));
		break;
	case 0x1a000185:
		text.Set("fortune_lucky_lucia_res");
		break;
	case 0x1a000148:
	case 0x1a00018a:
		text.Set("swimwear_male_2011_res");
		break;
	case 0x1a000149:
	case 0x1a00018b:
		text.Set("swimwear_female_2011_res");
		break;
	case 0x1a00014c:
	case 0x1a00018c:
		text.Set("pangyabox_2011_res");
		break;
	case 0x1a000195:
	case 0x1a000196:
	case 0x1a000197:
	case 0x1a000198:
	case 0x1a000199:
	case 0x1a00019a:
	case 0x1a00019b:
	case 0x1a00019c:
	case 0x1a00019d:
	case 0x1a00019e:
		text.Set("8th_box_res");
		break;
	case 0x1a0001cf:
		text.Set("fortune_pouch_2013");
		break;
	case 0x1a0001a2:
		text.Set("madoka_magika_pouch_res");
		break;
	case 0x1a0001d6:
		text.Set("kooh_selectbox_2013_res");
		break;
	case 0x1a000212:
		text.Set("vividred_2013_res");
		break;
	case 0x1a00021d:
		text.Set("pangyabox_2013_res");
		break;
	case 0x1a00021e:
		text.Set("male_swimsuit_2013_res");
		break;
	case 0x1a00021f:
		text.Set("female_swimsuit_2013_res");
		break;
	case 0x1a000220:
		text.Set("club_2013_res");
		break;
	case 0x1a00023c:
		text.Set("thai_2013_res");
		break;
	}

	if (m_pCaptionImg && m_pCaption1)
	{
		m_pCaptionImg->SetBgImg(name);
		m_pCaption1->SetCaption(cap1);
		if (cap2[0] != '\0')
			m_pCaption11->SetCaption(cap2);
	}

	UpdateLayout();
}
