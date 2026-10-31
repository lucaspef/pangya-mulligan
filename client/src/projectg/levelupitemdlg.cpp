#include "minatl.h"
#include "levelupitemdlg.h"
#include "projectg.h"
#include "wfont.h"
#include "wresrcmng.h"
#include "../shared/sharedtables.h"
#include "vc10_layout.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrLevelupItemForm, FrForm)

BEGIN_FRESH_MSGMAP(FrLevelupItemForm, FrForm)

ON_FRESH_VI("levelupitem", FRCMD_INIT, FrLevelupItemForm::OnLevelUpItemInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrLevelupItemForm::OnCloseUp)
ON_FRESH_VI("level", FRCMD_INIT, FrLevelupItemForm::OnLevelIcon)
ON_FRESH_VI("frame2", FRCMD_INIT, FrLevelupItemForm::OnFrame2Init)
ON_FRESH_VI("frame3", FRCMD_INIT, FrLevelupItemForm::OnFrame3Init)
ON_FRESH_VI("itemsend", FRCMD_INIT, FrLevelupItemForm::OnItemSendInit)
ON_FRESH_VI("leveluplist", FRCMD_INIT, FrLevelupItemForm::OnItemListInit)
ON_FRESH_VI("leveluplist", FRCMD_OWNERDRAW,
	FrLevelupItemForm::OnItemListOwnerDraw)
ON_FRESH_VI("itemexplane", FRCMD_INIT, FrLevelupItemForm::OnItemExplaneInit)

END_FRESH_MSGMAP()

FrLevelupItemForm::FrLevelupItemForm()
{
	C_ASSERT_VC10(sizeof(FrLevelupItemForm) == 0x210 && offsetof(FrLevelupItemForm, m_bResult) == 0x200); // 983: push 0x210 em 00ad9043
	m_pLevelIcon = NULL;
	m_pFrame2 = NULL;
	m_pFrame3 = NULL;
	m_pItemSend = NULL;
	m_pItemList = NULL;
	m_pItemExplane = NULL;
	m_level = 0;
	m_state = 0;
	m_itemTypeId = 0;
	m_itemTypeId2 = 1;
	m_itemCount[0] = 0; // D720: 983 00ad8b68/00ad8bae (+0x1f8 = 0, +0x1fc = 1)
	m_itemCount[1] = 1;
	m_bResult = false;
	static char* s_fontFile[] = { "[font_wind.jpg" };
	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_fontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

FrLevelupItemForm::~FrLevelupItemForm()
{
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrLevelupItemForm::OnLevelUpItemInit(int param)
{
}

void FrLevelupItemForm::OnCloseUp()
{
	Close(FrNONE, true);
}

bool FrLevelupItemForm::Close(eFormRet ret, bool bSound)
{
	return FrForm::Close(ret, bSound);
}

void FrLevelupItemForm::OnLevelIcon(int param)
{
	m_pLevelIcon = DYNAMIC_CAST(FrArea, param);
}

void FrLevelupItemForm::OnFrame2Init(int param)
{
	m_pFrame2 = DYNAMIC_CAST(FrArea, param);
}

void FrLevelupItemForm::OnFrame3Init(int param)
{
	m_pFrame3 = DYNAMIC_CAST(FrArea, param);
}

void FrLevelupItemForm::OnItemSendInit(int param)
{
	m_pItemSend = DYNAMIC_CAST(FrEdit, param);
}

void FrLevelupItemForm::OnItemListInit(int param)
{
	m_pItemList = DYNAMIC_CAST(FrListBox, param);
	C_ASSERT_VC10(offsetof(FrLevelupItemForm, m_deleteIcon) == 0x174 && offsetof(FrLevelupItemForm, m_clockIcon) == 0x18c); // E432
	m_clockIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockIcon.rect = WRect(50.0f, 35.0f, (float)m_clockIcon.pBitmap->Width(),
		(float)m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "delete");
	m_deleteIcon.rect =
		WRect(50.0f, 35.0f, (float)m_deleteIcon.pBitmap->Width(),
			(float)m_deleteIcon.pBitmap->Height());
}

void FrLevelupItemForm::OnItemListOwnerDraw(int param)
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;
	unsigned long* pTid = (unsigned long*)pItem->pData;
	if (!*pTid)
		return;
	IFF_ITEM_COMMON* pInfo = ItemManager()->FindCommonItem(*pTid);
	if (!pInfo)
		return;
	unsigned char timeFlag = pInfo->TimeFlag;
	const Bitmap* pBitmap =
		g_pFresh->GetManager()->GetBitmap("ITEMS", pInfo->Icon);
	if (!pBitmap)
		return;
	ItemManager()->GetCouponKind(*pTid);
	bool wide = m_pItemList->GetCurrentItemSize(true) == 1;
	float x = wide ? 73.0f : 35.0f, y;
	switch (*pTid >> 26)
	{
	case 0x10:
		y = 0.0f;
		break;
	case 0x1c:
		x = wide ? 53.0f : 15.0f;
		y = 0.0f;
		break;
	case 0x1f:
		y = 15.0f;
		break;
	default:
		y = 30.0f;
		break;
	}
	WRect rect(x + pItem->pos.x + 18.0f, y + pItem->pos.y + 18.0f,
		(float)pBitmap->Width(), (float)pBitmap->Height());
	pGDI->DrawTexture(pBitmap, rect, 0xffffffff, 0);
	pGDI->SetTextColor(0xff000000, 0xffffffff);
	pGDI->SetTextStyle(4);
	if (timeFlag >= 2)
	{
		pGDI->DrawTexture(m_clockIcon.pBitmap,
			m_clockIcon.rect +
				WPoint(pItem->pos.x + 15.0f, pItem->pos.y + 15.0f),
			0xffffffff, 0);
		return;
	}
	int i = 0;
	while (*pTid != (&m_itemTypeId)[i])
	{
		if (++i > 1)
			return;
	}
	int count = m_itemCount[i];
	if (count <= 1)
		return;
	const char* text = MakeStr("%d", count);
	float textX = pItem->pos.x + (wide ? 90.0f : 52.0f);
	for (unsigned k = 0; k < strlen(text); ++k)
		textX -= m_pFont->GetCharWidth(g_view, text[k] - '0');
	m_pFont->Print(g_view, textX + 55.0f, pItem->pos.y + 75.0f, text, 0,
		0xffffffff, NULL);
}

void FrLevelupItemForm::OnItemExplaneInit(int param)
{
	m_pItemExplane = DYNAMIC_CAST(FrEdit, param);
}

bool FrLevelupItemForm::OnInit()
{
	if (m_pLevelIcon)
	{
		char name[32];
		sprintf(name, "level_%03d", m_level + 1);
		m_pLevelIcon->SetBgImg(name);
	}
	SetItemInfo();
	// JP 983 (00ad90b0; PKT lote 31): sem o pacote 0x96 (0x93 do KR 645) -- o 983 nao manda 0x96 em lugar nenhum
	return true;
}

// JP 983 (00ad8f70): textos e premios vem da tabela LevelUpPrizeItem (FindLevelUpPrizeItem983), nao de tabela fixa
void FrLevelupItemForm::SetItemInfo()
{
	IFF983::sLevelUpPrizeItem* pPrize = ItemManager()->FindLevelUpPrizeItem983(m_level);
	if (pPrize == NULL)
		return;
	m_pItemSend->AddText(pPrize->szPrize, false, true);
	m_pItemExplane->AddText((const char*)pPrize + 0x3c, false, true);
	for (int i = 0; i < 2; ++i)
	{
		if (pPrize->tidPrize[i] != 0)
		{
			(&m_itemTypeId)[i] = pPrize->tidPrize[i];
			m_itemCount[i] = pPrize->nPrize[i];
			m_pItemList->AddItem(&(&m_itemTypeId)[i]);
		}
	}
	SetDesc("     \310\271\265\346 \276\306\300\314\305\333\300\272 \274\261\271\260\307\324\277\241\274\255 \310\256\300\316\307\317\274\274\277\344.");
}

void FrLevelupItemForm::OnProc(const float delta)
{
}

bool FrLevelupItemForm::SetData(const sLevelUpDone* pData)
{
	m_level = pData->bLevel;
	m_bResult = pData->bDone != 0;
	return true;
}
