#include "minatl.h"
#include "vc10_layout.h"
#include "../shared/recycletable.h"
#include "recycledlg.h"
#include "networksystem.h"
#include "packet.h"
#include "shareddoc.h"
#include "uccmanager.h"
#include "objectfactory.h"
#include "commonutil.h"
#include "frarea.h"
#include "frbutton.h"
#include "frdesktop.h"
#include "fredit.h"
#include "fresh.h"
#include "frlistbox.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "bitmap.h"
#include "../shared/localize.h"
#include "../shared/itemmanager.h"
#include "simpleoverlay.h"
#include "exhibition.h"
#include "setitemdlg.h"
#include "wresrcmng.h"
#include "wfont.h"
#include "wview.h"
#include "actor.h"
#include "taskmanager.h"
#include "binstr.h"
#include "mathconsts.h"
#include "jp/gamestringmanager.h"

extern int g_lobbyModal983; // 00e78cac (ver jp/descimageonlyicon.cpp)

// 983 OnInit 008e31f0: estado do proprio avatar (+0x158 = m_state983) e manager +0xa4/+0xb0, pela visao do avatar_avatar.h
#include "avatar_avatar.h"

#define STR_SFX_ITEM_GET_RARE \
	"\xbe\xc6\xc0\xcc\xc5\xdb\xc8\xb9\xb5\xe6(\xb7\xb9\xbe\xee)"


extern Fresh* g_pFresh;

// The caddie's greeting of the recycle ("magic box") dialog: the recipe
// hint, and the Valentine's Day event variant (no symbols).
std::string g_cadieMessage(K2L_Compatibility(
	"\xc4\xab\xb5\xf0\xbf\xa1\xc0\xc7 \xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda\xb7\xce \xbf\xc0\xbd\xc5\xb0\xc9 \xc8\xaf\xbf\xb5\xc7\xd5\xb4\xcf\xb4\xd9.\n"
	"\xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda\xbf\xa1\xbc\xad\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb8\xb8\xb5\xe9 \xbc\xf6 \xc0\xd6\xb4\xc2 \xb7\xb9\xbd\xc3\xc7\xc7\xb0\xa1 \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.\n"
	"\xb7\xb9\xbd\xc3\xc7\xc7\xb8\xa6 \xb5\xfb\xb6\xf3\xbc\xad \xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb8\xb8\xb5\xe9\xbe\xee \xba\xb8\xbc\xbc\xbf\xe4."));
std::string g_cadieValentineMessage(K2L_Compatibility(
	"\xc4\xab\xb5\xf0\xbf\xa1\xc0\xc7 \xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda\xb7\xce \xbf\xc0\xbd\xc5\xb0\xc9 \xc8\xaf\xbf\xb5\xc7\xd5\xb4\xcf\xb4\xd9.\n"
	"\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce\xb5\xa5\xc0\xcc \xc0\xcc\xba\xa5\xc6\xae\xb0\xa1 \xc1\xf8\xc7\xe0\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.\n"
	"\xc0\xcc\xba\xa5\xc6\xae \xb1\xe2\xb0\xa3 \xb5\xbf\xbe\xc8\xc0\xba \\c0xffff0000\\c\xb3\xb2\xbc\xba \xc0\xaf\xc0\xfa\\c0xff000000\\c\xb8\xb8 \xc1\xb6\xc7\xd5\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."));

// HACK: 215.0f and 410.0f are defined in cardremove.cpp, and 89.0f in
// tutorialmain.cpp, as function-local statics aliased to __real@43570000 /
// __real@43cd0000 / __real@42b20000; using the literals here would emit
// competing COMDATs that all the other objects would bind to.
extern "C" __declspec(selectany) const float rb_real_42b20000 = 89.0f; // C64.4: o estático de função do rebang não existe mais
extern "C" __declspec(selectany) const float rb_real_43570000 = 215.0f; // C64.4: o estático de função do rebang não existe mais

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

IMPLEMENT_OBJECT(FrRecycleDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrRecycleDlg, FrForm)
// 983 (s102): mapa 00e2a930 (inicializador 00ce3fa0) comeca pelos 5 controles novos; "card_slot" saiu
ON_FRESH_VI("MagicBox", FRCMD_INIT, FrRecycleDlg::OnMagicBoxBnInit983)
ON_FRESH_VV("MagicBox", FRCMD_LBUTTONUP, FrRecycleDlg::OnMagicBoxBnUp983)
ON_FRESH_VI("CardCompose", FRCMD_INIT, FrRecycleDlg::OnCardComposeBnInit983)
ON_FRESH_VV("CardCompose", FRCMD_LBUTTONUP, FrRecycleDlg::OnCardComposeBnUp983)
ON_FRESH_VI("Exchange", FRCMD_INIT, FrRecycleDlg::OnExchangeBnInit983)
ON_FRESH_VV("Exchange", FRCMD_LBUTTONUP, FrRecycleDlg::OnExchangeBnUp983)
ON_FRESH_VI("back_center_01", FRCMD_INIT, FrRecycleDlg::OnBackCenterInit983)
ON_FRESH_VI("back_bottom_01", FRCMD_INIT, FrRecycleDlg::OnBackBottomInit983)
ON_FRESH_VI("recycledlg", FRCMD_INIT, FrRecycleDlg::OnRecycleDlgInit)
ON_FRESH_VI("about", FRCMD_INIT, FrRecycleDlg::OnAboutBnInit)
ON_FRESH_VV("about", FRCMD_LBUTTONUP, FrRecycleDlg::OnAboutBnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrRecycleDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrRecycleDlg::OnCloseBnUp)
ON_FRESH_VI("level_low", FRCMD_INIT, FrRecycleDlg::OnLevelLowBnInit)
ON_FRESH_VV("level_low", FRCMD_LBUTTONUP, FrRecycleDlg::OnLevelLowBnUp)
ON_FRESH_VI("level_mid", FRCMD_INIT, FrRecycleDlg::OnLevelMidBnInit)
ON_FRESH_VV("level_mid", FRCMD_LBUTTONUP, FrRecycleDlg::OnLevelMidBnUp)
ON_FRESH_VI("level_high", FRCMD_INIT, FrRecycleDlg::OnLevelHighBnInit)
ON_FRESH_VV("level_high", FRCMD_LBUTTONUP, FrRecycleDlg::OnLevelHighBnUp)
ON_FRESH_VI("level_special", FRCMD_INIT, FrRecycleDlg::OnLevelSpecialBnInit)
ON_FRESH_VV("level_special", FRCMD_LBUTTONUP, FrRecycleDlg::OnLevelSpecialBnUp)
ON_FRESH_VI("level_event", FRCMD_INIT, FrRecycleDlg::OnLevelEventBnInit)
ON_FRESH_VV("level_event", FRCMD_LBUTTONUP, FrRecycleDlg::OnLevelEventBnUp)
ON_FRESH_VI("recycle_char_list", FRCMD_INIT,
	FrRecycleDlg::OnCharacterSelectListInit)
ON_FRESH_VV("recycle_char_list", FRCMD_LBUTTONUP,
	FrRecycleDlg::OnCharacterSelectListLBnUp)
ON_FRESH_VI("recycle_char_list", FRCMD_OWNERDRAW,
	FrRecycleDlg::OnCharacterSelectListOwnerDraw)
ON_FRESH_VI("recycle_item_list", FRCMD_INIT,
	FrRecycleDlg::OnRecycleItemListInit)
ON_FRESH_VV("recycle_item_list", FRCMD_LBUTTONUP,
	FrRecycleDlg::OnRecycleItemListLBnUp)
ON_FRESH_VV("recycle_item_list", FRCMD_RBUTTONUP,
	FrRecycleDlg::OnRecycleItemListRBnUp)
ON_FRESH_VI("recycle_item_list", FRCMD_OWNERDRAW,
	FrRecycleDlg::OnRecycleItemListOwnerDraw)
ON_FRESH_VI("recycle_number", FRCMD_INIT, FrRecycleDlg::OnRecycleNumberInit)
ON_FRESH_BI("recycle_number", FRCMD_ENTERKEY,
	FrRecycleDlg::OnRecycleNumberEnterKey)
ON_FRESH_VI("mix", FRCMD_INIT, FrRecycleDlg::OnMixBnInit)
ON_FRESH_VV("mix", FRCMD_LBUTTONUP, FrRecycleDlg::OnMixBnUp)
ON_FRESH_VI("arrow_minimum", FRCMD_INIT, FrRecycleDlg::OnArrowMinimumBnInit)
ON_FRESH_VV("arrow_minimum", FRCMD_LBUTTONDOWN,
	FrRecycleDlg::OnArrowMinimumBnDown)
ON_FRESH_VI("arrow_left", FRCMD_INIT, FrRecycleDlg::OnArrowLeftBnInit)
ON_FRESH_VV("arrow_left", FRCMD_LBUTTONDOWN, FrRecycleDlg::OnArrowLeftBnDown)
ON_FRESH_VV("arrow_left", FRCMD_LBUTTONUP, FrRecycleDlg::OnArrowLeftBnUp)
ON_FRESH_VI("arrow_right", FRCMD_INIT, FrRecycleDlg::OnArrowRightBnInit)
ON_FRESH_VV("arrow_right", FRCMD_LBUTTONDOWN, FrRecycleDlg::OnArrowRightBnDown)
ON_FRESH_VV("arrow_right", FRCMD_LBUTTONUP, FrRecycleDlg::OnArrowRightBnUp)
ON_FRESH_VI("arrow_maximum", FRCMD_INIT, FrRecycleDlg::OnArrowMaximumBnInit)
ON_FRESH_VV("arrow_maximum", FRCMD_LBUTTONDOWN,
	FrRecycleDlg::OnArrowMaximumBnDown)
ON_FRESH_VI("area_picture", FRCMD_INIT, FrRecycleDlg::OnAreaPictureInit)
ON_FRESH_VV("area_picture", FRCMD_OWNERDRAW,
	FrRecycleDlg::OnAreaPictureOwnerDraw)
ON_FRESH_VI("recycle_item_name", FRCMD_INIT,
	FrRecycleDlg::OnRecycleItemNameInit)
ON_FRESH_VI("cadie_message", FRCMD_INIT, FrRecycleDlg::OnCadieMessageInit)
ON_FRESH_VI("select_part", FRCMD_INIT, FrRecycleDlg::OnSelectPartInit)
ON_FRESH_VV("select_part", FRCMD_LBUTTONDOWN, FrRecycleDlg::OnSelectPartLBDown)
ON_FRESH_VI("card_slotresult", FRCMD_INIT, FrRecycleDlg::OnCardSlotResultInit)
END_FRESH_MSGMAP()

// recycledlg.h: no line record (empty constructor of a nested helper class)
FrRecycleDlg::MiniBtn::MiniBtn()
{
}

// 983: WRect zerado do .bss (00f14668) que varias unidades copiam como "retangulo vazio"; dono e nome ainda nao achados
static WRect s_rcEmpty983;

// 983 (s102): construtor dos 5 rotulos (embutido no 008e15f0: retangulo vazio, sem bitmap, cor preta)
inline FrRecycleDlg::sLabel983::sLabel983()
	: rect(s_rcEmpty983), pBitmap(NULL), color(0xff000000)
{
}

FrRecycleDlg::FrRecycleDlg()
{
	// The title font's texture (one page).
	static char* s_titleFontFile[1] = { "[font_wind.jpg" };

	tagWTITLEFONT info;
	memset(&info, 0, sizeof(info));

	m_pCloseBn = NULL;
	m_pLevelLowBn = NULL;
	m_pLevelMidBn = NULL;
	m_pLevelHighBn = NULL;
	m_pLevelSpecialBn = NULL;
	m_pLevelEventBn = NULL;
	m_pCharacterSelectList = NULL;
	m_pRecycleItemList = NULL;
	m_pRecycleNumber = NULL;
	m_pMixBn = NULL;
	m_pArrowMinimumBn = NULL;
	m_pArrowLeftBn = NULL;
	m_pArrowRightBn = NULL;
	m_pArrowMaximumBn = NULL;
	m_pRecycleItemName = NULL;
	m_pCadieMsg = NULL;
	info.filename = s_titleFontFile;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:";
	m_pTitleFont = g_resrcmng->GetTitleFont();
	m_pTitleFont->Create(&info);

	m_pItemBnNormal = g_pFresh->GetManager()->GetBitmap("RECYCLEDLG",
		"recycle_item_bn_normal");
	m_pItemBnClick = g_pFresh->GetManager()->GetBitmap("RECYCLEDLG",
		"recycle_item_bn_click");

	m_pArrowUI = NULL;
	memset(m_characterTid, 0, sizeof(m_characterTid));
	m_selectedIdx = -1;
	m_characterIdx = 0;
	memset(&m_resultItem, 0, sizeof(m_resultItem));
	memset(m_needItem, 0, sizeof(m_needItem));
	memset(m_haveItem, 0, sizeof(m_haveItem));
	m_recycleNumber = 0;
	m_maxEnableRecycle = 0;

	memset(&m_mixedItem, 0, sizeof(m_mixedItem));
	m_arrowLeftDown = false;
	m_arrowRightDown = false;
	m_arrowHoldTime = 0;
	m_mixing = false;
	m_arrowState[0] = 0;
	m_arrowState[1] = 0;
	m_arrowBlinkTime[0] = 0;
	m_arrowBlinkTime[1] = 0;
	m_mixSendDelay = 0;
	m_mixResultTime = 0;
	m_cadieMsgTime = 0;
	m_selectedTid = 0;
	m_selectedPart = 0;
	m_partSelected = false;
	m_unused545 = false;
	m_unused548 = 0;
	m_unused54C = 0;
	m_resultColor983 = 0xffffffff;
	m_pSelectPartBn = NULL;
	m_pSelectPartDlg = NULL;
	InitDataCharacterSelect();
	InitIndexRecycleItem();

	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		WSendPacket packet((enumClientPacket)0xc1);
		packet.Encode1(2);
		packet.Send(TO_GAME);
	}

	// 983 (s102): controles novos, zerados depois do pacote (008e1af8)
	m_pMagicBoxBn983 = NULL;
	m_pCardComposeBn983 = NULL;
	m_pExchangeBn983 = NULL;
	m_pBackCenter983 = NULL;
	m_pBackBottom983 = NULL;

	C_ASSERT_VC10(offsetof(FrRecycleDlg, m_resultItem) == 0x1a8 && offsetof(FrRecycleDlg, m_itemPos) == 0x610);
	C_ASSERT_VC10(offsetof(FrRecycleDlg, m_arrowLeftDown) == 0x660 && offsetof(FrRecycleDlg, m_pSelectPartDlg) == 0x6a8);
	C_ASSERT_VC10(offsetof(FrRecycleDlg, m_deleteBtn) == 0x708 && offsetof(FrRecycleDlg, m_haveItemMap) == 0x780);
	C_ASSERT_VC10(offsetof(FrRecycleDlg, m_label983) == 0x790 && offsetof(FrRecycleDlg, m_unused54C) == 0x698);
}

FrRecycleDlg::~FrRecycleDlg()
{
	if (IsLocalContent((localContentType_t)0x5a /*983: era 0x59; medido*/))
	{
		if (WSingleton<WNetworkSystem>::IsInstantiated())
		{
			WSendPacket packet((enumClientPacket)0xc1);
			packet.Encode1(1);
			packet.Send(TO_GAME);
		}
	}

	if (m_pArrowUI)
	{
		delete m_pArrowUI;
		m_pArrowUI = 0;
	}
	if (g_resrcmng && m_pTitleFont)
	{
		g_resrcmng->Release(m_pTitleFont);
		m_pTitleFont = 0;
	}
}

void FrRecycleDlg::OnRecycleDlgInit(int)
{
}

void FrRecycleDlg::OnAboutBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnAboutBnUp()
{
	FrRecycleAboutDlg* pDlg = CreateForm<FrRecycleAboutDlg>(
		g_pFresh->GetManager(), this, "recycle_about", 0);

	if (pDlg)
	{
		pDlg->Open((FRESH_PFN_RESULT)0, 3);
	}
}

void FrRecycleDlg::OnCloseBnInit(int param)
{
	m_pCloseBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnCloseBnUp()
{
	OnFreshOkay();
}

void FrRecycleDlg::OnLevelLowBnInit(int param)
{
	m_pLevelLowBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pLevelLowBn)
	{
		m_pLevelLowBn->SetStatus(FrButton::PRESSED);
	}
}

void FrRecycleDlg::OnLevelLowBnUp()
{
	SetListRecycleItem(m_level = 0, 0);

	if (m_pLevelLowBn)
		m_pLevelLowBn->SetStatus(FrButton::PRESSED);
	if (m_pLevelMidBn)
		m_pLevelMidBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelHighBn)
		m_pLevelHighBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelEventBn)
		m_pLevelEventBn->SetStatus(FrButton::NORMAL);
	if (m_pCharacterSelectList)
		m_pCharacterSelectList->SetVisible(false);
}

void FrRecycleDlg::OnLevelMidBnInit(int param)
{
	m_pLevelMidBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnLevelMidBnUp()
{
	SetListRecycleItem(m_level = 1, 0);

	if (m_pLevelLowBn)
		m_pLevelLowBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelMidBn)
		m_pLevelMidBn->SetStatus(FrButton::PRESSED);
	if (m_pLevelHighBn)
		m_pLevelHighBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelEventBn)
		m_pLevelEventBn->SetStatus(FrButton::NORMAL);
	if (m_pCharacterSelectList)
		m_pCharacterSelectList->SetVisible(false);
}

void FrRecycleDlg::OnLevelHighBnInit(int param)
{
	m_pLevelHighBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnLevelHighBnUp()
{
	SetListRecycleItem(m_level = 2, 0);

	if (m_pLevelLowBn)
		m_pLevelLowBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelMidBn)
		m_pLevelMidBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelHighBn)
		m_pLevelHighBn->SetStatus(FrButton::PRESSED);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelEventBn)
		m_pLevelEventBn->SetStatus(FrButton::NORMAL);
	if (m_pCharacterSelectList)
		m_pCharacterSelectList->SetVisible(false);
}

void FrRecycleDlg::OnLevelSpecialBnInit(int param)
{
	m_pLevelSpecialBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pLevelSpecialBn)
	{
		if (IsLocalContent((localContentType_t)0x64 /*983: era 0x63; interpolado*/))
		{
			m_pLevelSpecialBn->SetVisible(true);
		}
		else
		{
			m_pLevelSpecialBn->SetVisible(false);
		}
	}
}

void FrRecycleDlg::OnLevelSpecialBnUp()
{
	SetListRecycleItem(m_level = 3, m_characterIdx);

	if (m_pLevelLowBn)
		m_pLevelLowBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelMidBn)
		m_pLevelMidBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelHighBn)
		m_pLevelHighBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->SetStatus(FrButton::PRESSED);
	if (m_pLevelEventBn)
		m_pLevelEventBn->SetStatus(FrButton::NORMAL);
	if (m_pCharacterSelectList)
		m_pCharacterSelectList->SetVisible(true);
}

void FrRecycleDlg::OnLevelEventBnInit(int param)
{
	m_pLevelEventBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (g_NumEventRecycleItem > 0)
	{
		m_pLevelEventBn->SetVisible(true);
	}
	else
	{
		m_pLevelEventBn->SetVisible(false);
	}
}

void FrRecycleDlg::OnLevelEventBnUp()
{
	SetListRecycleItem(m_level = 4, m_characterIdx);

	if (m_pLevelLowBn)
		m_pLevelLowBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelMidBn)
		m_pLevelMidBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelHighBn)
		m_pLevelHighBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->SetStatus(FrButton::NORMAL);
	if (m_pLevelEventBn)
		m_pLevelEventBn->SetStatus(FrButton::PRESSED);

	if (m_pCharacterSelectList)
		m_pCharacterSelectList->SetVisible(false);
}

// 983 (008e3600)
void FrRecycleDlg::SelectLevel983(int level)
{
	switch (level)
	{
	case 0:
		OnLevelLowBnUp();
		break;
	case 1:
		OnLevelMidBnUp();
		break;
	case 2:
		OnLevelHighBnUp();
		break;
	case 3:
		OnLevelSpecialBnUp();
		break;
	case 4:
		OnLevelEventBnUp();
		break;
	}
}

void FrRecycleDlg::OnCharacterSelectListInit(int param)
{
	m_pCharacterSelectList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pCharacterSelectList)
	{
		m_pCharacterSelectList->SetVisible(false);
	}
}

void FrRecycleDlg::OnCharacterSelectListLBnUp()
{
	FrListItem* pItem = m_pCharacterSelectList->GetSelected();

	if (pItem)
	{
		m_characterIdx = *(unsigned long*)pItem->pData & 0x3ffffff; // 983 (008e31b0): indice pelo tid do personagem

		SetListRecycleItem(m_level, m_characterIdx);
	}
}

void FrRecycleDlg::OnCharacterSelectListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;

	if (pItem)
	{
		unsigned long* pTid = (unsigned long*)pItem->pData;

		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(*pTid);

		if (pCommon)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap(MakeStr("s_%s", pCommon->Icon));

			if (pBitmap)
			{
				FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

				if (pGDI)
				{
					unsigned long color = 0xFF808080;

					if (pItem->selected)
					{
						color = 0xFFFFFFFF;
					}
					else if (pItem->underCursor)
					{
						color = 0xFFC0C0C0;
					}

					pGDI->DrawTexture(pBitmap,
						WRect(pItem->pos.x + 4.0f, pItem->pos.y + 4.0f,
							pBitmap->Width() * 0.8f, pBitmap->Height() * 0.8f),
						color, 0);
				}
			}
		}
	}
}

void FrRecycleDlg::ReInitializeHaveItem()
{
	m_haveItemMap.clear();
}

static bool IsHaveItem983(unsigned long tid);

// 983 008e25d0: status first, then Set (tail jmp)
void FrRecycleDlg::AddHaveItemMap(const sRecycleItem* pItem)
{
	sRecycleStatus983& status = m_haveItemMap[pItem];
	status.Set(pItem);
}

// 983 00e7e034: the shared empty status (global, built at startup)
static const sRecycleStatus983 s_noRecycleStatus983;

// 983 008e0260: the status of a list recipe (the shared empty status when it was not added)
const sRecycleStatus983& FrRecycleDlg::GetRecycleStatus983(
	const sRecycleItem* pItem) const
{
	std::map<const sRecycleItem*, sRecycleStatus983>::const_iterator it =
		m_haveItemMap.find(pItem);
	if (it != m_haveItemMap.end())
		return it->second;

	return s_noRecycleStatus983;
}

void FrRecycleDlg::OnRecycleItemListInit(int param)
{
	m_pRecycleItemList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);

	if (m_pRecycleItemList)
	{
		m_pRecycleItemList->UseRightButton(2); // 983 008dcb06: direito nao seleciona
	}

	m_clockBtn.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockBtn.rect = WRect(50.0f, 35.0f, m_clockBtn.pBitmap->Width(),
		m_clockBtn.pBitmap->Height());
	m_deleteBtn.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "delete");
	m_deleteBtn.rect = WRect(50.0f, 35.0f, m_deleteBtn.pBitmap->Width(),
		m_deleteBtn.pBitmap->Height());
}

void FrRecycleDlg::OnRecycleItemListLBnUp()
{
	ClickRecycleItem();
}

void FrRecycleDlg::OnRecycleItemListRBnUp()
{
	// 983 (008e1270): o item sob o cursor, sem selecionar
	FrListItem* pItem = m_pRecycleItemList->GetItemUnderCursor();

	if (pItem)
	{
		sRecycleItem* pRecycleItem = (sRecycleItem*)pItem->pData;

		OpenInformation(pRecycleItem);
	}
}

// One recipe of the recycle list: the result item on its button, the
// "random" mark, the character's small icon for a part of the event level,
// and the reason the recipe cannot be made (level too low / already owned).
void FrRecycleDlg::OnRecycleItemListOwnerDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (pItem)
	{
		sRecycleItem* pRecycleItem = (sRecycleItem*)pItem->pData;
		WPoint pos(pItem->pos.x + 3.0f, pItem->pos.y + 3.0f);
		int button = 0;
		unsigned long color = 0xFFFFFFFF;

		const sRecycleStatus983& status = GetRecycleStatus983(pRecycleItem);
		if (!status.IsKnown())
		{
			return;
		}

		if (status.IsEnabled())
		{
			if (pItem->selected)
			{
				button = 1;
			}
		}
		else
		{
			color = 0xFF909090;
		}

		DrawItem(pos, pRecycleItem->dwTid, pRecycleItem->iNum, 0,
			(&m_pItemBnNormal)[button], color);

		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();

		if (pRecycleItem->dwRandSeq > 0)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap("recycle_random_mark");
			if (pBitmap)
			{
				WRect rect(pos.x + 2.0f, pos.y, (float)pBitmap->Width(),
					(float)pBitmap->Height());
				pGI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
			}
		}

		if ((pRecycleItem->dwTid & 0xFC000000) == 0x8000000 &&
			pRecycleItem->iIndex >= g_NumLowRecycleItem +
					g_NumMidRecycleItem + g_NumHighRecycleItem +
					g_NumSpecialRecycleItem)
		{
			IFF_ITEM_COMMON* pCharacter = ItemManager()->FindCommonItem(
				((pRecycleItem->dwTid >> 18) & 0xFF) | 0x4000000);
			if (pCharacter)
			{
				const Bitmap* pBitmap =
					g_pFresh->GetBitmap(MakeStr("s_%s", pCharacter->Icon));
				if (pBitmap)
				{
					WRect rect(pItem->pos.x + 3.0f, pItem->pos.y + 3.0f,
						(float)pBitmap->Width(), (float)pBitmap->Height());
					pGI->DrawTexture(pBitmap, rect, 0xEEFFFFFF, 0);
				}
			}
		}

		// the "level_%03d" label is this recipe's own, centred on the item
		if (status.Test(1))
		{
			const Bitmap* pLevel = g_pFresh->GetBitmap(
				MakeStr("level_%03d", pRecycleItem->byLevel + 1));
			m_label983[1].pBitmap = pLevel;
			m_label983[1].rect.x = (float)(m_pRecycleItemList->GetItemWidth() *
					0.5 -
				(int)(float)(pLevel->Width() * 0.5));
			m_label983[1].rect.y = (float)(m_pRecycleItemList->GetItemHeight() *
					0.5 -
				(int)(float)(pLevel->Height() * 0.5));
			m_label983[1].rect.w = (float)pLevel->Width();
			m_label983[1].rect.h = (float)pLevel->Height();
		}

		for (int i = 0; i < 5; i++)
		{
			if (status.Test(i) && m_label983[i].pBitmap)
			{
				WRect rect = m_label983[i].rect;
				rect.x += pItem->pos.x;
				rect.y += pItem->pos.y;
				pGI->DrawTexture(m_label983[i].pBitmap, rect,
					m_label983[i].color, 0);
			}
		}
	}
}

void FrRecycleDlg::OnRecycleNumberInit(int param)
{
	m_pRecycleNumber = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	ControlRecycleNumber(-1);
}

bool FrRecycleDlg::OnRecycleNumberEnterKey(int)
{
	return false;
}

void FrRecycleDlg::OnMixBnInit(int param)
{
	m_pMixBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnMixBnUp()
{
	if (m_recycleNumber > 0)
	{
		if (!CSharedDoc::Instance()->ConfirmInsertItem(m_selectedTid,
				m_recycleNumber))
		{
			FrForm* pForm =
				CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", 0);
			WRect rect = pForm->GetRect();
			pForm->SetRect(WRect(rect.x, rect.y, rect.w, rect.h));
			pForm->SetTimeLimit(12.0f);
			// "You already own 20,000 of this item: it cannot be mixed."
			pForm->SetMessage(K2L_Compatibility(
				"\xc7\xf6\xc0\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 \xba\xb8\xc0\xaf\xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc1\xb6\xc7\xd5 \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
			pForm->Open((FRESH_PFN_RESULT)0, 0);
			return;
		}

		// 983: um taco entre os ingredientes pede confirmacao (o "magic box" zera reforma/rank/maestria do taco)
		for (int i = 0; i < 4; i++)
		{
			if (ItemManager()->FindClubSet(m_haveItem[i].dwTid))
			{
				FrForm* pForm = CreateForm<FrForm>(
					g_pFresh->GetManager(), this, "notify_okcancel", 0);
				if (!pForm)
					return;
				pForm->SetMessage(K2L_Compatibility(
					"\xc5\xac\xb7\xb4\xbc\xbc\xc6\xae\xb8\xa6 \xb8\xb6\xb9\xfd\xbb\xf3\xc0\xda\xb7\xce \xc1\xb6\xc7\xd5\xc7\xd2 \xb0\xe6\xbf\xec,\n\\c0xffff0000\\c\xc5\xac\xb7\xb4\xc0\xc7 \xb0\xb3\xc1\xb6/\xb7\xa9\xc5\xa9\xbe\xf7 \xbb\xf3\xc5\xc2, \xbc\xf7\xb7\xc3\xb5\xb5\xb0\xa1 \xb8\xf0\xb5\xce \xc3\xca\xb1\xe2\xc8\xad\\c0xff000000\\c\xb5\xcb\xb4\xcf\xb4\xd9.\n\xc1\xb6\xc7\xd5\xc0\xbb \xc1\xf8\xc7\xe0\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
					false);
				pForm->Open((FRESH_PFN_RESULT)&FrRecycleDlg::OnClubSetMixConfirmResult983, 2);
				return;
			}
		}

		if ((m_selectedTid & 0xFC000000) == 0x1C000000)
		{
			FrForm* pForm = CreateForm<FrForm>(
				g_pFresh->GetManager(), this, "notify_okcancel", 0);
			pForm->SetMessage(GetGameString(0x1731b), false);
			pForm->Open((FRESH_PFN_RESULT)&FrRecycleDlg::OnMixConfirmResult983, 1);
		}
		else
			OnMixConfirmResult983(1, 0);
	}
}

void FrRecycleDlg::StartMix983()
{
	m_mixing = true;
	EnableTotalControls(false);

	m_mixSendDelay = 1.5f;
	m_mixResultTime = 0;
	memset(&m_mixedItem, 0, sizeof(m_mixedItem));
}

bool FrRecycleDlg::OnMixConfirmResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
		StartMix983();
	return true;
}

bool FrRecycleDlg::OnClubSetMixConfirmResult983(int ret, FrForm* pForm)
{
	if (ret == 1)
		StartMix983();
	return true;
}

void FrRecycleDlg::OnArrowMinimumBnInit(int param)
{
	m_pArrowMinimumBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnArrowMinimumBnDown()
{
	ControlRecycleNumber(0);
}

void FrRecycleDlg::OnArrowLeftBnInit(int param)
{
	m_pArrowLeftBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnArrowLeftBnDown()
{
	if ((m_haveItem[0].dwTid & 0xFC000000) == 0x08000000 &&
		m_haveItem[0].iNum > 1)
	{
		if (!m_partSelected)
		{
			return;
		}

		if (!m_pSelectPartBn->IsVisible())
		{
			m_pSelectPartBn->SetVisible(true);
		}
	}

	m_arrowLeftDown = true;
	m_arrowHoldTime = 0.0f;
	ControlRecycleNumber(1);
}

void FrRecycleDlg::OnArrowLeftBnUp()
{
	m_arrowLeftDown = false;
}

void FrRecycleDlg::OnArrowRightBnInit(int param)
{
	m_pArrowRightBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnArrowRightBnDown()
{
	if ((m_haveItem[0].dwTid & 0xFC000000) == 0x08000000 &&
		m_haveItem[0].iNum > 1)
	{
		if (!m_partSelected)
		{
			return;
		}

		if (m_pSelectPartBn->IsVisible())
		{
			m_pSelectPartBn->SetVisible(false);
		}
	}

	m_arrowRightDown = true;
	m_arrowHoldTime = 0.0f;
	ControlRecycleNumber(2);
}

void FrRecycleDlg::OnArrowRightBnUp()
{
	m_arrowRightDown = false;
}

void FrRecycleDlg::OnArrowMaximumBnInit(int param)
{
	m_pArrowMaximumBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleDlg::OnArrowMaximumBnDown()
{
	ControlRecycleNumber(3);
}

void FrRecycleDlg::OnAreaPictureInit(int param)
{
	DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrRecycleDlg::OnAreaPictureOwnerDraw()
{
	DrawRecycleItem();
	DrawArrow();
}

void FrRecycleDlg::OnRecycleItemNameInit(int param)
{
	m_pRecycleItemName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrRecycleDlg::OnCadieMessageInit(int param)
{
	m_pCadieMsg = DYNAMIC_CAST(FrEdit, (FrWnd*)param);

	if (m_pCadieMsg)
	{
		SetCadieMessage(GetGameString(0x1731a)); // 983 (008dd060): texto do caddie pela tabela de textos
	}
}

bool FrRecycleDlg::OnInit()
{
	m_pMagicBoxBn983->SetStatus((FrButton::eButMode)2);
	EnableDrag(false);
	InitPositionItemData();
	SetListRecycleItem(0, 0);
	InitListCharacterSelect();
	EnableMixControls(false);

	// 983: sem o modelo do caddie ("shop_cadie") do 645
	m_pArrowUI = new CSimpleUI("keys.png");
	m_pArrowUI->Add("RIGHT_S", WRect(40.0f, 0.0f, 20.0f, 20.0f), false);
	m_pArrowUI->Add("RIGHT", WRect(0.0f, 40.0f, 20.0f, 20.0f), false);
	m_pArrowUI->Add("INVALID", WRect(40.0f, 40.0f, 20.0f, 20.0f), false);

	SetLabels983(m_pRecycleItemList->GetItemWidth(), m_pRecycleItemList->GetItemHeight());

	CTaskManager* pTaskManager = CTaskManager::Instance();
	(!pTaskManager ? 0 : pTaskManager->GetCurrentTask())
		->SendMsgToMainActor(MsgObject(0, M983(0xdf), 0, 0, 0, 0, 0));

	// 983: no lobby de avatares marca o proprio avatar (estado 0x20, como o avatar_mainui faz com a "recycledlg")
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 | 0x20);
		ntAvatarManager::m_pInstance->m_bActive = false;
	}

	return true;
}

void FrRecycleDlg::OnProc(const float dt)
{
	if (m_arrowLeftDown)
	{
		m_arrowHoldTime += dt;

		if (m_arrowHoldTime > 0.2f)
		{
			m_arrowHoldTime = 0.0f;
			ControlRecycleNumber(1);
		}
	}
	else if (m_arrowRightDown)
	{
		m_arrowHoldTime += dt;

		if (m_arrowHoldTime > 0.2f)
		{
			m_arrowHoldTime = 0.0f;
			ControlRecycleNumber(2);
		}
	}

	if (m_maxEnableRecycle > 0 && m_selectedIdx >= 0)
	{
		if (!m_mixing)
		{
			m_arrowBlinkTime[0] += dt;
			m_arrowState[1] = 0;

			if (m_arrowBlinkTime[0] > 0.5f)
			{
				m_arrowBlinkTime[0] = 0.0f;
				m_arrowState[0] = m_arrowState[0] == 0;
			}
		}
		else
		{
			m_arrowBlinkTime[1] += dt;
			m_arrowState[0] = 1;

			if (m_arrowBlinkTime[1] > 0.5f)
			{
				m_arrowBlinkTime[1] = 0.0f;
				m_arrowState[1] = m_arrowState[1] == 0;
			}
		}
	}
	else
	{
		m_arrowState[0] = -1;
		m_arrowState[1] = -1;
	}

	if (m_mixSendDelay > 0.0f)
	{
		m_mixSendDelay -= dt;

		if (m_mixSendDelay <= 0.0f)
		{
			SendDataRecycleItem(m_selectedIdx, m_recycleNumber);
		}
	}

	if (m_mixResultTime > 0.0f)
	{
		m_mixResultTime -= dt;

		if (m_mixResultTime <= 0.0f)
		{
			memset(&m_mixedItem, 0, sizeof(m_mixedItem));
		}
	}

	if (m_cadieMsgTime > 0.0f)
	{
		m_cadieMsgTime -= dt;

		if (m_cadieMsgTime <= 0.0f)
			SetCadieMessage(GetGameString(0x1731a)); // 983 (008e3577)
	}
}

void FrRecycleDlg::OnCancel()
{
	if (!m_mixing) // 983 (008db720)
		FrForm::Close(true);
}

void FrRecycleDlg::InitPositionItemData()
{
	// JP 983 (008dd0e0): relativo a "back_center_01" (resultado e 4 receitas)
	// e a "back_bottom_01" (4 do inventario e o ultimo).
	m_itemPos[0] = WPoint(m_pBackCenter983->GetRect().x + 190.0f,
		m_pBackCenter983->GetRect().y + 35.0f);

	// 983: 1a coluna soma 0.0 (fldz)
	m_itemPos[1] = WPoint(m_pBackCenter983->GetRect().x + 178.0 + 0.0,
		m_pBackCenter983->GetRect().y + 110.0f);
	m_itemPos[2] = WPoint(m_pBackCenter983->GetRect().x + 178.0 + 50.0,
		m_pBackCenter983->GetRect().y + 110.0f);
	m_itemPos[3] = WPoint(m_pBackCenter983->GetRect().x + 178.0 + 100.0,
		m_pBackCenter983->GetRect().y + 110.0f);
	m_itemPos[4] = WPoint(m_pBackCenter983->GetRect().x + 178.0 + 150.0,
		m_pBackCenter983->GetRect().y + 110.0f);

	m_itemPos[5] = WPoint(m_pBackBottom983->GetRect().x - 5.0 + 0.0,
		m_pBackBottom983->GetRect().y + 20.0f);
	m_itemPos[6] = WPoint(m_pBackBottom983->GetRect().x - 5.0 + 63.0,
		m_pBackBottom983->GetRect().y + 20.0f);
	m_itemPos[7] = WPoint(m_pBackBottom983->GetRect().x - 5.0 + 126.0,
		m_pBackBottom983->GetRect().y + 20.0f);
	m_itemPos[8] = WPoint(m_pBackBottom983->GetRect().x - 5.0 + 189.0,
		m_pBackBottom983->GetRect().y + 20.0f);

	m_itemPos[9] = WPoint(m_pBackBottom983->GetRect().x + 395.0f,
		m_pBackBottom983->GetRect().y + 25.0f);
}

void FrRecycleDlg::SetListRecycleItem(int level, int character)
{
	m_haveItemMap.clear();

	memset(&m_resultItem, 0, sizeof(m_resultItem));
	memset(m_needItem, 0, sizeof(m_needItem));
	memset(m_haveItem, 0, sizeof(m_haveItem));

	if (m_pSelectPartBn)
	{
		m_pSelectPartBn->SetVisible(false);
	}

	EnableMixControls(false);

	m_maxEnableRecycle = 0;
	m_pRecycleItemList->ClearItem();

	m_selectedIdx = -1;
	m_pRecycleItemName->ClearLine();

	int start = 0;
	int end = 0;

	switch (level)
	{
	case 0:

		start = 0;
		end = g_NumLowRecycleItem;
		break;
	case 1:
		start = g_NumLowRecycleItem;
		end = start + g_NumMidRecycleItem;
		break;
	case 2:
		start = g_NumLowRecycleItem + g_NumMidRecycleItem;
		end = start + g_NumHighRecycleItem;
		break;
	case 3:
		start =
			g_NumLowRecycleItem + g_NumMidRecycleItem + g_NumHighRecycleItem;
		end = start + g_NumSpecialRecycleItem;
		break;
	case 4:
		start = g_NumLowRecycleItem + g_NumMidRecycleItem +
			g_NumHighRecycleItem + g_NumSpecialRecycleItem;
		end = start + g_NumEventRecycleItem;
		break;
	}

	bool byCharacter = level == 3 || level == 4;

	for (int i = start; i < end; i++)
	{
		if (g_ArrayRecycleItem[i].dwTid &&
			ItemManager()->FindCommonItem(g_ArrayRecycleItem[i].dwTid))
		{
			if (!byCharacter || g_ArrayRecycleItem[i].iChar == character ||
				g_ArrayRecycleItem[i].iChar == -1 ||
				g_ArrayRecycleItem[i].iChar == -2)
			{
				m_pRecycleItemList->AddItem(&g_ArrayRecycleItem[i]);
				AddHaveItemMap(&g_ArrayRecycleItem[i]);
			}
		}
	}
}

void FrRecycleDlg::ClickRecycleItem()
{
	FrListItem* pItem = m_pRecycleItemList->GetSelected();

	if (pItem)
	{
		sRecycleItem* pRecycleItem = (sRecycleItem*)pItem->pData;

		if (GetRecycleStatus983(pRecycleItem).IsEnabled())
		{
			if (m_mixedItem.dwTid)
			{
				m_mixResultTime = 0.01f;
			}

			m_resultColor983 = 0xFFFFFFFF;
			m_selectedIdx = pRecycleItem->iIndex;
			SetDataTotalItem(m_selectedIdx);
			SetMaxEnableRecycle(m_selectedIdx);
			ControlRecycleNumber(0);
			if (m_maxEnableRecycle > 0)
				EnableMixControls(true);
			else
				EnableMixControls(false);
		}
		else
		{
			m_resultColor983 = 0xFF707070;
			SetDataTotalItem(pRecycleItem->iIndex);
			m_maxEnableRecycle = 0;
			ControlRecycleNumber(-1);
			EnableMixControls(false);
		}

		m_selectedTid = pRecycleItem->dwTid;
		m_partSelected = false;
		m_selectedPart = 0;

		if ((m_haveItem[0].dwTid & 0xFC000000) == 0x08000000)
		{
			// Counts the owned parts of the first ingredient's type that no
			// character wears.
			unsigned int count = 0;

			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_partsList.begin();
				it != CSharedDoc::Instance()->m_partsList.end(); ++it)
			{
				if (it->tid == m_haveItem[0].dwTid)
				{
					std::map<unsigned int, sCharacterInfo>::iterator end =
						CSharedDoc::Instance()->m_charMap.end();

					for (std::map<unsigned int, sCharacterInfo>::iterator ci =
							 CSharedDoc::Instance()->m_charMap.begin();
						ci != end; ++ci)
					{
						for (int i = 0; i < 24; i++)
						{
							if (it->guid == ci->second.ItemIdList[i])
								goto nextPart;
						}
					}

					count++;
				}
nextPart:;
			}

			if (m_pSelectPartBn && count > 1)
			{
				m_pSelectPartBn->SetVisible(true);
			}
			else
			{
				m_pSelectPartBn->SetVisible(false);
				m_partSelected = false;
			}
		}
		else
		{
			m_pSelectPartBn->SetVisible(false);
			m_partSelected = false;
		}
	}
	else
	{
		m_selectedIdx = -1;
		m_selectedTid = 0;
		SetDataTotalItem(m_selectedIdx);
		SetMaxEnableRecycle(m_selectedIdx);
		ControlRecycleNumber(-1);
		EnableMixControls(false);

		if (m_pSelectPartBn)
		{
			m_pSelectPartBn->SetVisible(false);
		}

		m_partSelected = false;
		m_selectedPart = 0;
	}
}

void FrRecycleDlg::InitDataCharacterSelect()
{
	for (int i = 0; i < 15; i++) // 983 (008db740): 15 personagens
		m_characterTid[i] = i | 0x4000000;
}

void FrRecycleDlg::InitListCharacterSelect()
{
	std::map<unsigned int, sCharacterInfo>::iterator it =
		CSharedDoc::Instance()->m_charMap.find(
			CSharedDoc::Instance()->m_myInfo.userEquip.guidChar);

	if (it != CSharedDoc::Instance()->m_charMap.end())
	{
		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem(it->second.tid);

		if (pCommon)
		{
			m_characterIdx = pCommon->TypeId & 0x1FF;
		}
	}

	m_pCharacterSelectList->ClearItem();

	// 983 (008e0a50): 15 personagens, o 13 fica fora da lista (o 14 vira o item 13)
	for (int i = 0; i < 15; i++)
	{
		if (i == 13)
			continue;

		m_pCharacterSelectList->AddItem(&m_characterTid[i]);

		if (i == m_characterIdx)
		{
			FrListItem* pItem;
			if (m_characterIdx == 14)
				pItem = m_pCharacterSelectList->GetItem(13);
			else
				pItem = m_pCharacterSelectList->GetItem(i);

			m_pCharacterSelectList->SelectItem(pItem, true);
		}
	}
}

// Draws one item slot: the button under it, the item's icon (placed per
// item kind), then either the clock icon (a timed item) or the item count.
// JP 983 008de8a0 (o exe roda este; antes so havia a copia morta no naked -> stub sem corpo)
void FrRecycleDlg::DrawItem(WPoint pos, unsigned long tid,
	int num, unsigned char type, const Bitmap* pButton, unsigned long color)
{
		// 983: pega o GDI sem testar NULL
		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
		if (!tid)
			return;

		IFF_ITEM_COMMON* pCommon = ItemManager()->FindCommonItem(tid);
		if (!pCommon)
			return;

		bool shrink = false;
		float offsetX = 0.0f;
		float offsetY = 0.0f;
		const Bitmap* pBitmap = g_pFresh->GetBitmap(pCommon->Icon);

		// JP 983 (008de8a0): deslocamento do icone por tipo do item (tid >> 26).
	switch (tid >> 26)
	{
	case 2:
		offsetX = -2.0f;
		shrink = true;
		offsetY = -15.0f;
		break;
	case 4:
		offsetX = 1.0f;
		shrink = true;
		offsetY = 2.0f;
		break;
	case 9:
		offsetX = -10.0f;
		offsetY = -21.0f;
		break;
	case 0x1F:
		offsetX = -2.0f;
		shrink = true;
		offsetY = -5.0f;
		break;
	case 0x10:
		// 983: relogio (type) desligado so para estes tids; segue no 0x1C
		switch (tid)
		{
		case 0x40000007:
		case 0x40000014: case 0x40000015: case 0x40000016: case 0x40000017:
		case 0x40000019: case 0x4000001A: case 0x4000001B: case 0x4000001C:
		case 0x4000001D: case 0x4000001E: case 0x4000001F: case 0x40000020:
		case 0x40000021: case 0x40000023:
		case 0x4000002F: case 0x40000030: case 0x40000031: case 0x40000032:
		case 0x40000033: case 0x40000034: case 0x40000035: case 0x40000036:
		case 0x40000037: case 0x40000038: case 0x40000039: case 0x4000003A:
			type = 0;
			break;
		default:
			type = 1;
			break;
		}
		// fall through
	case 0x1C:
		offsetX = -12.0f;
		offsetY = -20.0f;
		break;
	case 0x12:
		offsetX = -4.5f;
		offsetY = -9.0f;
		break;
	case 6:
		if (ItemManager()->GetCouponKind(tid))
		{
			offsetX = -5.0f;
			offsetY = 3.0f;
		}
		else if (ItemManager()->GetCouponKind(tid) == 3)
		{
			offsetX = -7.0f;
			offsetY = 3.0f;
		}
		break;
	}

	if (!pBitmap)
		return;

	if (pButton)
	{
		WRect rect(pos.x, pos.y, (float)pButton->Width(),
			(float)pButton->Height());
		pGI->DrawTexture(pButton, rect, color, 0);
	}

	float w = (float)pBitmap->Width();
	float h = (float)pBitmap->Height();

	if (shrink)
	{
		w *= 0.8f;
		h *= 0.8f;
	}

	if ((tid >> 26) != 7)
	{
		WRect rect(pos.x + 14.0f + offsetX, pos.y + 11.0f + offsetY, w, h);
		pGI->DrawTexture(pBitmap, rect, color, 0);
	}
	else
	{
		w -= 6.0f;
		h -= 6.0f;
		WRect dst(pos.x + 14.0f, pos.y + 11.0f, w, h);
		WRect src(4.0f, 2.0f, w, h);
		pGI->DrawTexture(pBitmap, src, dst, color, 0);
	}

	pGI->SetTextColor(0xFF000000, 0xFFFFFFFF);
	pGI->SetTextStyle(4);

	if (type)
	{
		pos.x -= 4.0f;
		pos.y += 8.0f;
		// 983: soma do rect inline (sem WRect::operator+)
		WRect rect(m_clockBtn.rect.x + pos.x, m_clockBtn.rect.y + pos.y,
			m_clockBtn.rect.w, m_clockBtn.rect.h);
		pGI->DrawTexture(m_clockBtn.pBitmap, rect, 0xFFFFFFFF, 0);
		return;
	}

	if (num > 1)
	{
		const char* text = MakeStr("%d", num);
		float textX = pos.x + 54.0f;

		for (int i = 0; i < strlen(text); ++i)
		{
			textX -= m_pTitleFont->GetCharWidth(g_view, text[i] - '0');
		}

		m_pTitleFont->Print(g_view, textX, pos.y + 42.0f, text, 0, color, NULL);
	}
}

// Draws the item slots of the picture area: the result, the four needed
// ingredients, what is left of the four owned ones, and the mixed item.
void FrRecycleDlg::DrawRecycleItem()
{
	// 983 (008dfc50): GDI lido uma vez no topo, sem teste de NULL
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();

	if (m_resultItem.dwRandSeq)
	{
		const Bitmap* pBitmap = g_pFresh->GetBitmap("recycle_random_out");
		if (pBitmap)
		{
			WRect rect(m_itemPos[0].x + 2.0f, m_itemPos[0].y,
				(float)pBitmap->Width(), (float)pBitmap->Height());
			pGI->DrawTexture(pBitmap, rect, m_resultColor983, 0);
		}
	}
	else
	{
		DrawItem(m_itemPos[0], m_resultItem.dwTid, m_resultItem.iNum, 0, NULL,
			m_resultColor983);
	}

	// 983: ponteiros da posicao e do item, contador descendo
	WPoint* pPos = &m_itemPos[1];
	sRecycleItem* pNeed = m_needItem;
	for (int n = 4; n != 0; n--, pPos++, pNeed++)
	{
		DrawItem(*pPos, pNeed->dwTid, pNeed->iNum, 0, NULL, m_resultColor983);
	}

	int i;

	for (i = 0; i < 4; i++)
	{
		int num = m_haveItem[i].iNum - m_recycleNumber * m_needItem[i].iNum;
		unsigned long color = 0xFFFFFFFF;

		if (num < m_needItem[i].iNum || num <= 0)
		{
			color = 0xFF909090;
		}

		// 983: num < 1 primeiro; uma so chamada de DrawItem no fim (goto)
		if (num < 1)
		{
			if ((m_haveItem[i].dwTid & 0xFC000000) == 0x08000000)
			{
				m_pSelectPartBn->SetVisible(false);
			}
		}
		else if ((m_haveItem[i].dwTid & 0xFC000000) == 0x08000000)
		{
			// Counts the owned parts of the first ingredient's type that no
			// character wears.
			unsigned int count = 0;

			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_partsList.begin();
				it != CSharedDoc::Instance()->m_partsList.end(); ++it)
			{
				if (it->tid == m_haveItem[0].dwTid)
				{
					std::map<unsigned int, sCharacterInfo>::iterator end =
						CSharedDoc::Instance()->m_charMap.end();

					for (std::map<unsigned int, sCharacterInfo>::iterator
							 ci = CSharedDoc::Instance()->m_charMap.begin();
						ci != end; ++ci)
					{
						for (int j = 0; j < 24; j++)
						{
							if (it->guid == ci->second.ItemIdList[j])
								goto nextPart;
						}
					}

					count++;
				}
nextPart:;
			}

			IFF_ITEM_COMMON* pCommon =
				ItemManager()->FindCommonItem(m_haveItem[0].dwTid);

			if (m_pSelectPartBn && count > 1 && m_partSelected == false)
			{
				if (pCommon)
				{
					m_pSelectPartBn->SetButtonImg("select_on",
						FrButton::NORMAL);
					m_pSelectPartBn->SetButtonImg("select_on",
						FrButton::OVER);
					m_pSelectPartBn->SetButtonImg("select_on",
						FrButton::PRESSED);
					WRect rect;
					// 983 (008dfc50): +240 / -85 do layout novo da janela
					rect.x = (float)(GetRect().x + 41.0 + 240.0);
					rect.y = (float)(GetRect().y + 387.0 - 85.0);
					rect.w = 40.0f;
					rect.h = 41.0f;
					m_pSelectPartBn->SetRect(rect);
				}
			}
			else if (m_partSelected != true)
			{
				goto drawHave;
			}
			else
			{
				if (m_pSelectPartBn)
				{
					if (strlen(pCommon->Icon))
					{
						m_pSelectPartBn->SetButtonImg(pCommon->Icon,
							FrButton::NORMAL);
						m_pSelectPartBn->SetButtonImg(pCommon->Icon,
							FrButton::OVER);
						m_pSelectPartBn->SetButtonImg(pCommon->Icon,
							FrButton::PRESSED);
					}

					WRect rect = m_pSelectPartBn->GetRect();
					rect.x = (float)(GetRect().x + 39.0 + 240.0);
					rect.y = (float)(GetRect().y + 372.0 - 85.0);
					rect.w *= 0.8f;
					rect.h *= 0.8f;
					m_pSelectPartBn->SetRect(rect);
				}
			}
		}
		else
		{
drawHave:
			DrawItem(m_itemPos[5 + i], m_haveItem[i].dwTid, num, 0, NULL,
				color);
		}
	}

	DrawItem(m_itemPos[9], m_mixedItem.dwTid, m_mixedItem.iNum, 0, NULL,
		0xFFFFFFFF);

	if (m_mixedItem.dwRandSeq && m_mixedItem.dwTid &&
		(m_mixedItem.dwTid & 0xFC000000) == 0x08000000)
	{
		IFF_ITEM_COMMON* pCharacter = ItemManager()->FindCommonItem(
			((m_mixedItem.dwTid >> 18) & 0xFF) | 0x4000000);
		if (pCharacter)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap(MakeStr("s_%s", pCharacter->Icon));
			if (pBitmap)
			{
				WRect rect(m_itemPos[9].x + 6.0f, m_itemPos[9].y + 2.0f,
					(float)pBitmap->Width(), (float)pBitmap->Height());
				pGI->DrawTexture(pBitmap, rect, 0xFFFFFFFF, 0);
			}
		}
	}
}

void FrRecycleDlg::DrawArrow()
{
	if (m_pArrowUI)
	{
		std::string arrowName[2];
		for (int i = 0; i < 2; i++)
		{
			switch (m_arrowState[i])
			{
			case 0:
				arrowName[i] = "RIGHT";
				break;
			case 1:
				arrowName[i] = "RIGHT_S";
				break;
			default:
				arrowName[i] = "INVALID";
				break;
			}
		}

		// 983 (008e0b10): setas relativas ao "back_bottom_01" (x+253 / x+381, y+38)
		m_pArrowUI->Render(arrowName[0].c_str(),
			(float)(m_pBackBottom983->GetRect().x + 253.0),
			(float)(m_pBackBottom983->GetRect().y + 38.0), 1.0f, 0);
		m_pArrowUI->Render(arrowName[1].c_str(),
			(float)(m_pBackBottom983->GetRect().x + 381.0),
			(float)(m_pBackBottom983->GetRect().y + 38.0), 1.0f, 0);
	}
}

// 983 008db940 (C54.1): chamado pelo caso 0x22f do OnPacketCommon quando o servidor recusa a reciclagem
void FrRecycleDlg::ResetMix983()
{
	m_mixResultTime = 3.0f;
	m_mixing = false;
	EnableTotalControls(true);
}

void FrRecycleDlg::EnableTotalControls(bool enable)
{
	if (m_pCloseBn)
		m_pCloseBn->Enable(enable);
	if (m_pLevelLowBn)
		m_pLevelLowBn->Enable(enable);
	if (m_pLevelMidBn)
		m_pLevelMidBn->Enable(enable);
	if (m_pLevelHighBn)
		m_pLevelHighBn->Enable(enable);
	if (m_pLevelSpecialBn)
		m_pLevelSpecialBn->Enable(enable);
	if (m_pLevelEventBn)
		m_pLevelEventBn->Enable(enable);
	if (m_pCharacterSelectList)
		m_pCharacterSelectList->Enable(enable);
	if (m_pRecycleItemList)
		m_pRecycleItemList->Enable(enable);
	if (m_pMixBn)
		m_pMixBn->Enable(enable);
	if (m_pArrowMinimumBn)
		m_pArrowMinimumBn->Enable(enable);
	if (m_pArrowLeftBn)
		m_pArrowLeftBn->Enable(enable);
	if (m_pArrowRightBn)
		m_pArrowRightBn->Enable(enable);
	if (m_pArrowMaximumBn)
		m_pArrowMaximumBn->Enable(enable);
}

void FrRecycleDlg::EnableMixControls(bool enable)
{
	if (m_pMixBn)
		m_pMixBn->Enable(enable);
	if (m_pArrowMinimumBn)
		m_pArrowMinimumBn->Enable(enable);
	if (m_pArrowLeftBn)
		m_pArrowLeftBn->Enable(enable);
	if (m_pArrowRightBn)
		m_pArrowRightBn->Enable(enable);
	if (m_pArrowMaximumBn)
		m_pArrowMaximumBn->Enable(enable);
}

bool FrRecycleDlg::IsUsableParts(sItemInfo& item)
{
	// 983 (008e01e0): o personagem dono da peca sai do tid da peca, nao da lista selecionada;
	// calculado antes do laco (o 983 faz o shr/and/or antes do 1o cmp)
	const unsigned long charTid = ((item.tid >> 18) & 0xff) | 0x4000000;
	std::map<unsigned int, sCharacterInfo>& charMap =
		WSingleton<CSharedDoc>::Instance()->m_charMap;

	for (std::map<unsigned int, sCharacterInfo>::iterator it = charMap.begin();
		it != charMap.end(); ++it)
	{
		if (it->second.tid == charTid)
		{
			if (WSingleton<CSharedDoc>::Instance()->IsEquipParts(it->second, item.guid, item.tid))
			{
				return false;
			}
			break;
		}
	}

	return true;
}

// Fills the result item, the needed ingredients and how many of each the player owns.
void FrRecycleDlg::SetDataTotalItem(int index)
{
	const sRecycleItem* pItem = GetRecycleItem(index);
	if (pItem)
	{
		m_resultItem.dwTid = pItem->dwTid;
		m_resultItem.iNum = pItem->iNum;
		m_resultItem.dwRandSeq = pItem->dwRandSeq;
		strcpy(m_resultItem.szRandName, pItem->szRandName);
	}
	else
	{
		memset(&m_resultItem, 0, sizeof(m_resultItem));
	}

	if (m_pRecycleItemName)
	{
		m_pRecycleItemName->ClearLine();

		IFF_ITEM_COMMON* pCommon =
			ItemManager()->FindCommonItem(m_resultItem.dwTid);
		if (pCommon)
		{
			std::string name(pCommon->Name);
			if (m_resultItem.dwRandSeq)
			{
				name = m_resultItem.szRandName;
			}

			if (name.length() > 20)
			{
				char buf[32] = { 0 };
				mbscut(buf, name.c_str(), 17);
				name = buf;
				name += "...";
			}

			m_pRecycleItemName->AddText(
				MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), name.c_str()), false, true);
		}
	}

	int i;
	for (i = 0; i < 4; i++)
	{
		m_needItem[i].dwTid = GetTidMixItem(index, i);
		m_needItem[i].iNum = GetNumMixItem(index, i);
		m_haveItem[i].dwTid = m_needItem[i].dwTid;
		m_haveItem[i].iNum = 0;
	}

	for (i = 0; i < 4; i++)
	{
		unsigned long tid = m_haveItem[i].dwTid;
		if (!tid)
		{
			break;
		}

		switch (tid >> 26)
		{
		case 6:
		{
			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_myItemList.begin();
				it != CSharedDoc::Instance()->m_myItemList.end(); ++it)
			{
				if (it->tid == tid)
				{
					m_haveItem[i].dwItemid = it->guid;
					m_haveItem[i].iNum += it->Common[0];
				}
			}
		}
		break;
		case 5:
		{
			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_ballList.begin();
				it != CSharedDoc::Instance()->m_ballList.end(); ++it)
			{
				if (it->tid == tid)
				{
					m_haveItem[i].dwItemid = it->guid;
					m_haveItem[i].iNum += it->Common[0];
				}
			}
		}
		break;
		case 0x1C:
		{
			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_auxPartList.begin();
				it != CSharedDoc::Instance()->m_auxPartList.end(); ++it)
			{
				if (it->tid == tid)
				{
					m_haveItem[i].dwItemid = it->guid;
					m_haveItem[i].iNum += it->Common[0];
				}
			}
		}
		break;
		case 2:
		{
			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_partsList.begin();
				it != CSharedDoc::Instance()->m_partsList.end(); ++it)
			{
				if (it->tid == m_haveItem[i].dwTid)
				{
					if (IsUsableParts(*it))
					{
						m_haveItem[i].dwItemid = it->guid;
						m_haveItem[i].iNum++;
					}
				}
			}
		}
		break;
		case 7:
		{
			for (std::map<unsigned int, sCaddieInfo>::iterator it =
					 CSharedDoc::Instance()->m_caddieMap.begin();
				it != CSharedDoc::Instance()->m_caddieMap.end(); ++it)
			{
				if (it->second.tid == tid)
				{
					m_haveItem[i].dwItemid = it->second.guid;
					m_haveItem[i].iNum++;
				}
			}
		}
		break;
		case 9:
		{
			IFF_STRUCT::sSetItem* pSetItem = ItemManager()->FindSetItem(tid);
			if (pSetItem)
			{
				int count[10] = { 0 };

				int j;
				for (j = 0; j < pSetItem->nElems; j++)
				{
					for (std::list<sItemInfo>::iterator it =
							 CSharedDoc::Instance()->m_partsList.begin();
						it != CSharedDoc::Instance()->m_partsList.end(); ++it)
					{
						if (it->tid == pSetItem->ElemIds[j])
						{
							count[j]++;
						}
					}

					sCharacterInfo& character =
						CSharedDoc::Instance()->m_charMap[CSharedDoc::Instance()
								->m_myInfo.userEquip.guidChar];
					for (int k = 0; k < 24; k++)
					{
						if (pSetItem->ElemIds[j] == character.tidParts[k])
						{
							count[j]--;
							break;
						}
					}
				}

				int minCount = 0x7FFFFFFF;
				for (j = 0; j < pSetItem->nElems; j++)
				{
					if (count[j] < minCount)
					{
						minCount = count[j];
					}
				}

				m_haveItem[i].iNum = minCount;
			}
		}
		break;
		case 4:
		{
			for (std::map<unsigned int, sItemInfo>::const_iterator it =
					 CSharedDoc::Instance()->m_clubSetMap.begin();
				it != CSharedDoc::Instance()->m_clubSetMap.end(); ++it)
			{
				if ((*it).second.tid == tid &&
					CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet !=
						(*it).second.guid)
				{
					m_haveItem[i].dwItemid = (*it).second.guid;
					m_haveItem[i].iNum++;
					break;
				}
			}
		}
		break;
		case 0x1F:
		{
			for (std::list<sCards>::iterator it =
					 CSharedDoc::Instance()->m_cardList.begin();
				it != CSharedDoc::Instance()->m_cardList.end(); ++it)
			{
				if ((*it).tid == tid)
				{
					m_haveItem[i].dwItemid = (*it).uid;
					m_haveItem[i].iNum += (*it).cardCount;
				}
			}
		}
		break;
		case 0x10:
		{
			for (std::map<unsigned int, sMascotInfo>::iterator it =
					 CSharedDoc::Instance()->m_mascotMap.begin();
				it != CSharedDoc::Instance()->m_mascotMap.end(); ++it)
			{
				if (it->second.tid == tid)
				{
					m_haveItem[i].dwItemid = it->second.guid;
					m_haveItem[i].iNum++;
				}
			}
		}
		break;
		}

		// The equipped items cannot be used as ingredients.
		unsigned long haveTid = m_haveItem[i].dwTid;
		switch (haveTid >> 26)
		{
		case 6:
		{
			for (int k = 0; k < 10; k++)
			{
				if (haveTid ==
					CSharedDoc::Instance()->m_myInfo.userEquip.tidItemSlot[k])
				{
					m_haveItem[i].iNum--;
				}
			}
		}
		break;
		case 7:
		{
			std::map<unsigned int, sCaddieInfo>::iterator it =
				CSharedDoc::Instance()->m_caddieMap.find(
					CSharedDoc::Instance()->m_myInfo.userEquip.guidCaddie);
			// 983 (008e1b80): so com caddie equipado achado no mapa (o end() nao e lido)
			if (it != CSharedDoc::Instance()->m_caddieMap.end() && haveTid == it->second.tid)
			{
				m_haveItem[i].iNum--;
			}
		}
		break;
		case 4:
		{
			std::map<unsigned int, sItemInfo>::iterator it =
				CSharedDoc::Instance()->m_clubSetMap.find(
					CSharedDoc::Instance()->m_myInfo.userEquip.guidClubSet);
			if (haveTid == it->second.tid && m_haveItem[i].iNum != 0)
			{
				m_haveItem[i].iNum--;
			}
		}
		break;
		case 0x10:
		{
			std::map<unsigned int, sMascotInfo>::iterator it =
				CSharedDoc::Instance()->m_mascotMap.find(
					CSharedDoc::Instance()->m_myInfo.userEquip.guidMascot);
			// 983 (008e1b80): idem para o mascote equipado
			if (it != CSharedDoc::Instance()->m_mascotMap.end() && haveTid == it->second.tid &&
				m_haveItem[i].iNum != 0)
			{
				m_haveItem[i].iNum--;
			}
		}
		break;
		}
	}
}

void FrRecycleDlg::ControlRecycleNumber(int mode)
{
	switch (mode)
	{
	case 1:
		m_recycleNumber--;
		break;
	case 2:
		m_recycleNumber++;
		break;
	case 3:
		m_recycleNumber = m_maxEnableRecycle;
		break;
	case 0:
		m_recycleNumber = 0;
		break;
	default:
		m_recycleNumber = 0;
		break;
	}

	if (m_maxEnableRecycle > 0)
	{
		if (m_recycleNumber >= m_maxEnableRecycle)
		{
			m_recycleNumber = m_maxEnableRecycle;
		}
		else if (m_recycleNumber < 0)
		{
			m_recycleNumber = 0;
		}
	}
	else
	{
		m_recycleNumber = 0;
	}

	// 983: only parts (5/6) and type 0x1C items other than subtype 1 can be
	// made more than once
	unsigned long type = m_resultItem.dwTid >> 26;
	if ((type < 5 || (type > 6 && (type != 0x1C ||
			(m_resultItem.dwTid & 0x1F0000) == 0x10000))) &&
		m_recycleNumber > 1)
	{
		m_recycleNumber = 1;
	}

	if (m_resultItem.dwRandSeq > 0)
	{
		if (m_recycleNumber > 1)
		{
			m_recycleNumber = 1;
		}
	}

	if (m_pRecycleNumber)
	{
		if (m_recycleNumber > 99)
		{
			m_recycleNumber = 99;
		}
		m_pRecycleNumber->SetLine(1, MakeStr("%d", m_recycleNumber), 0, false,
			0);
	}
}

void FrRecycleDlg::SetMaxEnableRecycle(int)
{
	int maxCount = 0x7FFFFFFF;

	m_maxEnableRecycle = 0;

	for (int i = 0; i < 4; i++)
	{
		if (m_needItem[i].dwTid == 0)
			break;
		if (m_needItem[i].iNum <= 0)
			break;

		if (m_haveItem[i].iNum >= m_needItem[i].iNum)
		{
			int count = m_haveItem[i].iNum / m_needItem[i].iNum;

			if (count < maxCount)
			{
				maxCount = count;
			}
		}
		else
		{
			maxCount = 0;
			break;
		}
	}

	m_maxEnableRecycle = maxCount;
}

// JP 983 (00b2de00): tids dos itens raros (tabela 0xe41728, 10 entradas).
static bool IsRareRecycleTid983(unsigned long tid)
{
	static const unsigned long rare[10] = {
		0x0800088d, 0x08060812, 0x0809a828, 0x080e0010, 0x0811a824,
		0x08168017, 0x0819c018, 0x081d8826, 0x08220023, 0x08252016
	};

	for (int i = 0; i < 10; i++)
	{
		if (rare[i] == tid)
			return true;
	}

	return false;
}

#define SendMainActorText983(text) \
	do { \
		CTaskManager* pTaskManager = CTaskManager::Instance(); \
		(!pTaskManager ? 0 : pTaskManager->GetCurrentTask()) \
			->SendMsgToMainActor(MsgObject(0, M983(0x23), (int)(text), 0, 0, 0, 0)); \
	} while (0)

// JP 983 008ded50: "<nome da peca>! acquired" for rare parts.
void ShowRareGetMsg983(unsigned long tid)
{
	IFF_STRUCT::sPart* part = ItemManager()->FindPart(tid);
	SendMainActorText983(MakeStr(K2L_Compatibility("\xc3\xe0! %s \xc8\xb9\xb5\xe6"),
		part->c.Name));
}

// JP 983 008dede0: "%s acquired (Japan only)" for the part type 0x2 (tid & 0xfc000000 == 0x8000000).
void ShowJpGetMsg983(unsigned long tid)
{
	IFF_STRUCT::sPart* part = ItemManager()->FindPart(tid);
	SendMainActorText983(MakeStr(K2L_Compatibility(
		"%s \xc8\xb9\xb5\xe6 (\xc0\xcf\xba\xbb\xc0\xfc\xbf\xeb)"),
		part->c.Name));
}

// The server's answer to the mix request (SendDataRecycleItem).
void FrRecycleDlg::SetResultRecycleItem(bool success,
	std::list<sRecycleItem>& items, short index)
{
	if (success)
	{
		sRecycleItem result;
		memset(&result, 0, sizeof(result));

		// 983 008e2ac0: GetTidRecycleItem/GetRecycleItem/GetNumRecycleItem inline
		unsigned long tid = GetTidRecycleItem(index);

		if ((tid & 0xFC000000) == 0x24000000)
		{
			// JP 983: set item = a linha da tabela, sem conferir os elementos.
			const sRecycleItem* pItem = GetRecycleItem(index);
			if (pItem)
			{
				result = *pItem;
			}
		}
		else
		{
			if (items.size() == 1)
			{
				const sRecycleItem& item = *items.begin();
				result = item;
			}
			else
			{
				SendMainActorText983(K2L_Compatibility(
					"\xc1\xb6\xb0\xc7\xc0\xcc \xbf\xc3\xb9\xd9\xb8\xa3\xc1\xf6 \xbe\xca\xbd\xc0\xb4\xcf\xb4\xd9."));
			}
		}

		if (m_level == 3 || m_level == 4)
		{
			SetListRecycleItem(m_level, m_characterIdx);
		}

		m_resultItem = result;
		m_mixedItem = result;
		if (m_mixedItem.dwTid != 0x1a000010)
		{
			m_mixedItem.iNum = m_recycleNumber * GetNumRecycleItem((short)m_selectedIdx);
		}

		if (IsRareRecycleTid983(m_mixedItem.dwTid))
		{
			ShowRareGetMsg983(m_mixedItem.dwTid);
		}

		if ((m_mixedItem.dwTid & 0xFC000000) == 0x8000000)
		{
			ShowJpGetMsg983(m_mixedItem.dwTid);
		}

		if (m_pSelectPartBn)
		{
			m_pSelectPartBn->SetVisible(false);
		}
	}
	else
	{
		memset(&m_mixedItem, 0, sizeof(m_mixedItem));
	}

	ResetMix983(); // 983 008e2cb3
	SetDataTotalItem(m_selectedIdx);
	SetMaxEnableRecycle(m_selectedIdx);
	// 983 008e2ce1: +0x5c4 = 1 sempre; o DrawRecycleItem (008dfc50) so poe o icone do personagem na peca misturada com ele != 0
	m_mixedItem.dwRandSeq = 1;

	g_audio->PlaySfx(STR_SFX_ITEM_GET_RARE);

	// 983: so sai com os controles ligados se ainda pode misturar e nao tem o item
	if (m_selectedIdx >= 0)
	{
		ControlRecycleNumber(0);
		if (m_maxEnableRecycle <= 0)
		{
			EnableMixControls(false);
			return;
		}
		if (!IsHaveItem983(m_mixedItem.dwTid))
		{
			EnableMixControls(true);
			return;
		}
	}
	ControlRecycleNumber(-1);
	EnableMixControls(false);
}

void FrRecycleDlg::SendDataRecycleItem(int index, int number)
{
	if (index >= 0 && number > 0)
	{
		// JP 983 (008e2db0, C-PKT lote 10): quantidade em dword, marca antes do byte da contagem e
		// cada ingrediente como dois dwords (tid, serial); a contagem volta pela marca.
		char itemCount = 0;
		WSendPacket packet((enumClientPacket)0x158); // JP 983 (s63): 0x7e no KR 645
		packet.Encode2(index);
		packet.Encode4(number);
		int mark = packet.GetMark();
		packet.Encode1(0);

		for (int i = 0; i < 4; i++)
		{
			if (m_haveItem[i].dwTid)
			{
				packet.Encode4(m_haveItem[i].dwTid);
				packet.Encode4(m_haveItem[i].dwItemid);

				itemCount++;
			}
		}

		packet.EncodeData(mark, &itemCount, 1);
		packet.Send(TO_GAME);
	}
	else
	{
		OnFreshOkay();
	}

	unsigned long selectedPart = m_selectedPart;
	m_partSelected = false;
	if (m_haveItem[0].iNum < 3)
	{
		if (m_pSelectPartBn)
		{
			ClickRecycleItem();
			m_pSelectPartBn->SetVisible(false);

			for (std::list<sItemInfo>::iterator it =
					 CSharedDoc::Instance()->m_partsList.begin();
				it != CSharedDoc::Instance()->m_partsList.end(); ++it)
			{
				if (it->guid != selectedPart && it->tid == m_haveItem[0].dwTid)
				{
					m_selectedPart = it->guid;
					return;
				}
			}
		}
	}
	else
	{
		m_selectedPart = 0;
		ClickRecycleItem();
	}
}

// The item ids RandRecycleItem accepts.
namespace
{
	unsigned long g_randItemList[3] = { 0x1A0000CD, 0x1A0000CE, 0x1A0000CF };
}

bool RandRecycleItem(unsigned long tid)
{
	for (int i = 0; i < 3; i++)
	{
		if (tid == g_randItemList[i])
		{
			return true;
		}
	}

	return false;
}

// Opens the item information form (the set item form for a set item).
void FrRecycleDlg::OpenInformation(const sRecycleItem* pRecycle)
{
	unsigned long tid = pRecycle->dwTid;

	FrWnd* pWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("information");
	if (pWnd == NULL)
	{
		__rtti_obj = NULL;
		pWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"information_setitem");
		if (pWnd == NULL)
		{
			__rtti_obj = NULL;
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(tid);
			if (pItem)
			{
				// JP 983: periodo da combinacao ("inicio: aaaa ano mm mes dd dia").
				// 983 008e0df6: [ebp-0xc]/[ebp-8] = periodo[0..1], passado ao SetSetItem
				const char* period[2] = { NULL, NULL };
				const sRecycleStatus983& status = GetRecycleStatus983(pRecycle);
				if (status.IsKnown() && status.Test(6))
				{
					const char* periodStartFormat = MakeStr("\\c0xffff0000\\c%s: %s\\c0xff000000\\c",
						K2L_Compatibility("\xc1\xb6\xc7\xd5\xbd\xc3\xc0\xdb"),
						K2L_Compatibility("%d\xb3\xe2 %d\xbf\xf9 %d\xc0\xcf"));
					period[0] = MakeStr(periodStartFormat,
						pRecycle->dateStart983.wYear, pRecycle->dateStart983.wMonth,
						pRecycle->dateStart983.wDay);
					const char* periodEndFormat = MakeStr("\\c0xffff0000\\c%s: %s\\c0xff000000\\c",
						K2L_Compatibility("\xc1\xb6\xc7\xd5\xc1\xbe\xb7\xe1"),
						K2L_Compatibility("%d\xb3\xe2 %d\xbf\xf9 %d\xc0\xcf"));
					period[1] = MakeStr(periodEndFormat,
						pRecycle->dateEnd983.wYear, pRecycle->dateEnd983.wMonth,
						pRecycle->dateEnd983.wDay);
				}

				if ((pItem->TypeId & 0xFC000000) == 0x24000000)
				{
					FrSetItemDlg* pSetDlg = CreateForm<FrSetItemDlg>(
						g_pFresh->GetManager(), this, "information_setitem", 0);
					if (pSetDlg)
					{
						pSetDlg->SetSetItem(pItem->TypeId, period);
						pSetDlg->Open((FRESH_PFN_RESULT)0, 3);
						return;
					}
				}
				else
				{
					FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(),
						this, "information", 0);
					if (pForm)
					{
						FrButton* pReset = DYNAMIC_CAST(FrButton,
							pForm->FindChildByName("reset"));
						if (pReset)
						{
							pReset->SetVisible(false);
						}

						FrButton* pWorkshop = DYNAMIC_CAST(FrButton,
							pForm->FindChildByName("icon_clubsetworkshop"));
						if (pWorkshop)
						{
							pWorkshop->SetVisible(false);
						}

						FrWnd* pAbility = pForm->FindChildByName("static_avility");
						if (pAbility)
						{
							pAbility->SetVisible(false);
						}

						bool random = false;
						if (RandRecycleItem(pItem->TypeId))
						{
							random = true;
						}

						FrArea* pPortrait = DYNAMIC_CAST(FrArea,
							pForm->FindChildByName("portrait"));
						if (pPortrait)
						{
							if (random == false)
							{
								pPortrait->SetBgImg(pItem->Icon);
							}
							else
							{
								pPortrait->SetBgImg("hide");
							}
						}

						FrEdit* pName = DYNAMIC_CAST(FrEdit,
							pForm->FindChildByName("name"));
						FrArea* pLevel = DYNAMIC_CAST(FrArea,
							pForm->FindChildByName("level"));

						if (pName && pLevel)
						{
							if (random == false)
							{
								pName->AddText(MakeStr(
												   K2L_Compatibility("\xc0\xcc\xb8\xa7: %s"),
												   pItem->Name),
									false, true);
							}
							else if (random == true)
							{
								pName->AddText(K2L_Compatibility(
												   "\xc0\xcc\xb8\xa7: \xb7\xa3\xb4\xfd\xc1\xb6\xc7\xd5\xbe\xc6\xc0\xcc\xc5\xdb"),
									false, true);
							}
							pName->AddText("\n", false, true);

							if (pItem->IsUnderLvl)
							{
								pLevel->SetBgImg(
									MakeStr("level_%03d", pItem->Level + 1));
								pName->AddText(
									MakeStr(K2L_Compatibility(
												"\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf"),
										pItem->Name),
									false, true);
							}
							else
							{
								pLevel->SetBgImg(
									MakeStr("level_%03d", pItem->Level + 1));
								pName->AddText(
									MakeStr(K2L_Compatibility(
												"\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3"),
										pItem->Name),
									false, true);
							}

							if (period[0] && period[1])
							{
								pName->AddText("\n", false, true);
								pName->AddText(period[0], false, true);
								pName->AddText("\n", false, true);
								pName->AddText(period[1], false, true);
							}
						}

						if (random == false)
						{
							IFF_STRUCT::sDesc* pDesc =
								ItemManager()->FindDesc(pItem->TypeId);
							if (pDesc)
							{
								pForm->SetMessage(pDesc->Desc, false);
							}
						}
						else
						{
							pForm->SetMessage(K2L_Compatibility(
												  "\xc8\xae\xb7\xfc\xbf\xa1 \xb5\xfb\xb6\xf3 \xc1\xb6\xc7\xd5\xc7\xd1 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xb0\xe1\xc1\xa4\xb5\xcb\xb4\xcf\xb4\xd9."),
								false);
						}

						pForm->Open((FRESH_PFN_RESULT)0, 3);
					}
				}
			}
		}
	}
}

// 983 008db560: the part is one of the 9x3 set parts of the recycle table.
static const unsigned long g_recycleSetParts983[42] = {
	0x08010032, 0x08000848, 0x0801A812, 0x0804E058, 0x08040863, 0x08050811,
	0x0808E025, 0x0808082B, 0x0809081D, 0x080CE041, 0x080C002C, 0x080D481C,
	0x0810A030, 0x08100033, 0x0811201B, 0x0814E05E, 0x0814003E, 0x08162810,
	0x0818A060, 0x0818005E, 0x08196013, 0x081CE02F, 0x081C002B, 0x081DA817,
	0x0820E02F, 0x08200018, 0x0821A80C, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0
};

bool IsRecycleSetPart983(unsigned long tid)
{
	for (int row = 0; row < 9; row++)
	{
		for (int col = 0; col < 3; col++)
		{
			if (g_recycleSetParts983[row * 3 + col] == tid)
				return true;
		}
	}

	return false;
}

// 983 008df9b0: owns at least 14 of the parts of the two sets (columns c and
// c + 3, seven rows) the part belongs to.
bool IsSetPartsComplete983(unsigned long tid)
{
	int col;
	for (int i = 0; i < 42; i += 6)
	{
		if (g_recycleSetParts983[i + 0] == tid)
		{
			col = 0;
			goto found;
		}
		if (g_recycleSetParts983[i + 1] == tid)
		{
			col = 1;
			goto found;
		}
		if (g_recycleSetParts983[i + 2] == tid)
		{
			col = 2;
			goto found;
		}
		if (g_recycleSetParts983[i + 3] == tid)
		{
			col = 0;
			goto found;
		}
		if (g_recycleSetParts983[i + 4] == tid)
		{
			col = 1;
			goto found;
		}
		if (g_recycleSetParts983[i + 5] == tid)
		{
			col = 2;
			goto found;
		}
	}

	return false;

found:
	int count = 0;
	std::list<sItemInfo>& parts = CSharedDoc::Instance()->m_partsList;
	for (std::list<sItemInfo>::iterator it = parts.begin(); it != parts.end();
		++it)
	{
		for (int i = 0; i < 42; i += 6)
		{
			if (g_recycleSetParts983[i + col] == it->tid)
				count++;
			if (g_recycleSetParts983[i + col + 3] == it->tid)
				count++;
		}
	}

	if (count >= 14)
		return true;
	return false;
}

// 983 00a18c20 (CItemManager): item restrito pelo tipo.
bool CItemManager::IsRestrictedItem983(unsigned long tid, bool gift, bool b3)
{
	bool result = false;

	IFF_ITEM_COMMON* pCommon = FindCommonItem(tid);
	if (!pCommon)
		return false;

	switch (tid >> 26)
	{
	case 1:
		result = (b3 == true);
		break;
	case 7:
		if (gift == true)
		{
			if (tid == 0x1c000001 || tid == 0x1c000002 || tid == 0x1c000003)
				result = true;
			else
				result = (tid == 0x1c000007);
		}
		break;
	case 16:
		result = (pCommon->TimeFlag != 0);
		break;
	case 2:
	case 4:
		result = (pCommon->IsSalable == 1 || pCommon->IsSalable == 3);
		break;
	case 9:
		result = (((tid >> 21) & 0x1f) == 6 || ((tid >> 21) & 0x1f) == 5);
		break;
	case 28:
		if ((tid & 0x1f0000) == 0)
			result = true;
		break;
	case 8:
	case 14:
	case 15:
		break;
	default:
		result = true;
		break;
	}

	return result;
}

// 983 00a18d20 (CItemManager): set 0x24 cujos elementos sao todos restritos.
bool CItemManager::IsRestrictedSet983(unsigned long tid)
{
	if ((tid & 0xfc000000) != 0x24000000)
		return false;

	IFF_STRUCT::sSetItem* pSetItem =
		CSharedDoc::Instance()->m_itemManager.FindSetItem(tid);
	if (!pSetItem)
		return false;

	bool result = true;
	for (int i = 0; i < pSetItem->nElems; i++)
	{
		if (pSetItem->ElemIds[i] != 0 &&
			IsRestrictedItem983(pSetItem->ElemIds[i], false, false) != true)
		{
			result = false;
			break;
		}
	}

	return result;
}

// 983 00a23710 (CItemManager): true when the owned part does not count for
// the set (usado so por IsHaveItem983 abaixo).
bool CItemManager::IsPartExcluded983(unsigned long tid, bool gift, bool b3)
{
	if (IsTwinsItem983(tid))
		return false;
	if ((tid & 0xfc000000) == 0x24000000)
		return IsRestrictedSet983(tid);

	return IsRestrictedItem983(tid, gift, b3);
}

// 983 008dfa60 (usercall, tid em eax): does the player already own the item.
static bool IsHaveItem983(unsigned long tid)
{
	bool result = false;

	switch (tid >> 26)
	{
	case 2:
		for (std::list<sItemInfo>::iterator it =
				 CSharedDoc::Instance()->m_partsList.begin();
			it != CSharedDoc::Instance()->m_partsList.end(); ++it)
		{
			if (it->tid == tid)
			{
				if (IsRecycleSetPart983(tid))
				{
					result = IsSetPartsComplete983(tid);
					return result;
				}

				result = true;
				break;
			}
		}
		break;
	case 9:
	{
		IFF_STRUCT::sSetItem* pSetItem = ItemManager()->FindSetItem(tid);
		if (pSetItem)
		{
			for (int i = 0; i < pSetItem->nElems; i++)
			{
				unsigned long elemId = pSetItem->ElemIds[i];
				for (std::list<sItemInfo>::iterator it =
						 CSharedDoc::Instance()->m_partsList.begin();
					it != CSharedDoc::Instance()->m_partsList.end(); ++it)
				{
					if (!ItemManager()->IsPartExcluded983(it->tid,
							it->gift_flag & 1, false) &&
						elemId == it->tid)
					{
						result = true;
						break;
					}
				}
			}
		}
		break;
	}
	case 4:
		for (std::map<unsigned int, sItemInfo>::iterator it =
				 CSharedDoc::Instance()->m_clubSetMap.begin();
			it != CSharedDoc::Instance()->m_clubSetMap.end(); ++it)
		{
			if (it->second.tid == tid)
			{
				result = true;
				break;
			}
		}
		break;
	case 7:
		for (std::map<unsigned int, sCaddieInfo>::iterator it =
				 CSharedDoc::Instance()->m_caddieMap.begin();
			it != CSharedDoc::Instance()->m_caddieMap.end(); ++it)
		{
			if (it->second.tid == tid)
			{
				result = true;
				break;
			}
		}
		break;
	case 14:
		for (std::list<sItemInfo>::iterator it =
				 CSharedDoc::Instance()->m_cutinList.begin();
			it != CSharedDoc::Instance()->m_cutinList.end(); ++it)
		{
			if (it->tid == tid)
			{
				result = true;
				break;
			}
		}
		break;
	}

	return result;
}

// 983 009e99b0 (CItemManager, chamado por 008e02a0): recipe within its period; 009d9950 = IsBetween
bool CItemManager::IsInRecyclePeriod983(short index, _SYSTEMTIME& now)
{
	sRecycleItem* pRecycle = NULL;
	if (index >= 0 && index < g_NumIndexRecycleItem)
		pRecycle = &g_ArrayRecycleItem[index];
	if (!pRecycle)
		return false;

	_SYSTEMTIME* pStart = &pRecycle->dateStart983;
	_SYSTEMTIME* pEnd = &pRecycle->dateEnd983;
	if (!pStart && !pEnd)
		return true;

	return IsBetween(pStart, pEnd, now);
}

// 983 008e02a0
void sRecycleStatus983::Set(const sRecycleItem* pItem)
{
	flags |= 0x80000000;

	if (IsHaveItem983(pItem->dwTid))
		flags |= 1;

	if (pItem->byLevel > CSharedDoc::Instance()->m_myInfo.stat.Level)
		flags |= 2;

	short index = pItem->iIndex;
	if (index >= 0 && index < g_NumIndexRecycleItem)
	{
		sRecycleItem* pRecycle = &g_ArrayRecycleItem[index];
		if (pRecycle && (pRecycle->dateStart983.wYear > 0 ||
							pRecycle->dateEnd983.wYear > 0))
		{
			if (ItemManager()->IsInRecyclePeriod983(pItem->iIndex,
					CSharedDoc::Instance()->GetServerTime()))
			{
				flags |= 0x40000000;
				return;
			}

			if (ItemManager()->CompareSystemTime(
					CSharedDoc::Instance()->GetServerTime(),
					pRecycle->dateStart983) < 0)
			{
				flags |= 8;
			}
			else
			{
				flags |= 4;
			}
		}
	}
}

sRecycleStatus983::sRecycleStatus983()
	: flags(0)
{
}

// 983 008ddcd0
bool sRecycleStatus983::IsKnown() const
{
	return (flags & 0x80000000) ? true : false;
}

// 983 008ddce0: no restriction (owned / level / period flags all clear)
bool sRecycleStatus983::IsEnabled() const
{
	return (flags & 0x3FFFFFFF) == 0;
}

// 983 008ddcf0
bool sRecycleStatus983::Test(unsigned int kind) const
{
	bool result = false;

	switch (kind)
	{
	case 0:
		result = bits.owned;
		break;
	case 1:
		result = bits.levelLow;
		break;
	case 2:
		result = bits.periodOver;
		break;
	case 3:
		result = bits.periodNotYet;
		break;
	case 4:
		result = bits.inPeriod;
		break;
	case 6:
		result = (flags & 0x4000000C) != 0;
		break;
	}

	return result;
}

void FrRecycleDlg::SetCadieMessage(const char* msg)
{
	if (m_pCadieMsg)
	{
		m_pCadieMsg->ClearLine();
		m_pCadieMsg->AddText(msg, false, true);
	}
}

bool FrRecycleDlg::SetEventMessage(int level)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		OnSelectPartInit(0);

	bool result = false;

	if (level == 4)
	{
		if (WSingleton<CSharedDoc>::Instance()->m_myInfo.info.gender)
		{
			SetCadieMessage(g_cadieValentineMessage.c_str());
			m_cadieMsgTime = 5.0f;
			result = true;
		}
	}

	return result;
}

IMPLEMENT_OBJECT(FrRecycleAboutDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrRecycleAboutDlg, FrForm)
ON_FRESH_VI("recycle_about", FRCMD_INIT, FrRecycleAboutDlg::OnAboutInit)
ON_FRESH_VI("aboutviewer", FRCMD_INIT, FrRecycleAboutDlg::OnAboutViewerInit)
ON_FRESH_VI("index01", FRCMD_INIT, FrRecycleAboutDlg::OnIndex01Init)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnIndex01BnUp)
ON_FRESH_VI("index02", FRCMD_INIT, FrRecycleAboutDlg::OnIndex02Init)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnIndex02BnUp)
ON_FRESH_VI("index03", FRCMD_INIT, FrRecycleAboutDlg::OnIndex03Init)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnIndex03BnUp)
ON_FRESH_VI("index04", FRCMD_INIT, FrRecycleAboutDlg::OnIndex04Init)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnIndex04BnUp)
ON_FRESH_VI("index05", FRCMD_INIT, FrRecycleAboutDlg::OnIndex05Init)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnIndex05BnUp)
ON_FRESH_VI("backward", FRCMD_INIT, FrRecycleAboutDlg::OnBackwardBnInit)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnBackwardBnUp)
ON_FRESH_VI("forward", FRCMD_INIT, FrRecycleAboutDlg::OnForwardBnInit)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnForwardBnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrRecycleAboutDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrRecycleAboutDlg::OnOkBnUp)
END_FRESH_MSGMAP()

FrRecycleAboutDlg::FrRecycleAboutDlg()
{
	for (int i = 0; i < 5; i++)
		m_pIndexBn[i] = 0;

	m_aboutIndex = 0;
	m_pBackwardBn = 0;
	m_pForwardBn = 0;
}

FrRecycleAboutDlg::~FrRecycleAboutDlg()
{
}

void FrRecycleAboutDlg::OnAboutInit(int)
{
}

void FrRecycleAboutDlg::OnAboutViewerInit(int param)
{
	m_pAboutViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex01Init(int param)
{
	m_pIndexBn[0] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex01BnUp()
{
	SetImgAboutIndex(0);
}

void FrRecycleAboutDlg::OnIndex02Init(int param)
{
	m_pIndexBn[1] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex02BnUp()
{
	SetImgAboutIndex(1);
}

void FrRecycleAboutDlg::OnIndex03Init(int param)
{
	m_pIndexBn[2] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex03BnUp()
{
	SetImgAboutIndex(2);
}

void FrRecycleAboutDlg::OnIndex04Init(int param)
{
	m_pIndexBn[3] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex04BnUp()
{
	SetImgAboutIndex(3);
}

void FrRecycleAboutDlg::OnIndex05Init(int param)
{
	m_pIndexBn[4] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnIndex05BnUp()
{
	SetImgAboutIndex(4);
}

void FrRecycleAboutDlg::OnBackwardBnInit(int param)
{
	m_pBackwardBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBackwardBn->UseDblClick(false); // 983 008dd690: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrRecycleAboutDlg::OnBackwardBnUp()
{
	SetImgAboutIndex(--m_aboutIndex);
	m_pBackwardBn->SetPushDelay(0.0f);
}

void FrRecycleAboutDlg::OnForwardBnInit(int param)
{
	m_pForwardBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pForwardBn->UseDblClick(false); // 983 008dd750: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrRecycleAboutDlg::OnForwardBnUp()
{
	SetImgAboutIndex(++m_aboutIndex);
	m_pForwardBn->SetPushDelay(0.0f);
}

void FrRecycleAboutDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrRecycleAboutDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrRecycleAboutDlg::OnInit()
{
	SetImgAboutIndex(0);

	return true;
}

void FrRecycleAboutDlg::SetImgAboutIndex(int index)
{
	for (int i = 0; i < 5; i++)
	{
		m_pIndexBn[i]->Unselect(0);
	}

	if (m_pBackwardBn)
	{
		m_pBackwardBn->Enable(true);
	}

	if (m_pForwardBn)
	{
		m_pForwardBn->Enable(true);
	}

	if (index <= 0)
	{
		index = 0;
		m_pIndexBn[0]->Select(0);

		if (m_pBackwardBn)
		{
			m_pBackwardBn->Enable(false);
		}
	}

	else if (index >= 4)
	{
		index = 4;
		m_pIndexBn[4]->Select(0);

		if (m_pForwardBn)
		{
			m_pForwardBn->Enable(false);
		}
	}

	m_pIndexBn[index]->Select(0);
	m_aboutIndex = index;

	if (m_pAboutViewer)
	{
		m_pAboutViewer->Open(MakeStr("help_cadie_%02d.jpg", index + 1));
	}
}

void FrRecycleDlg::OnSelectPartInit(int param)
{
	m_pSelectPartBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (m_pSelectPartBn)
	{
		m_pSelectPartBn->SetPushDelay(0.1f);
		m_pSelectPartBn->SetVisible(false);
	}
}

void FrRecycleDlg::OnSelectPartLBDown()
{
	if (m_pSelectPartDlg == 0)
	{
		m_pSelectPartDlg = CreateForm<FrSelectPartDlg>(g_pFresh->GetManager(),
			this, "select_part_dlg", 0);
		if (m_pSelectPartDlg)
		{
			m_selectedPart = 0;
			m_partSelected = false;
			m_pSelectPartDlg->SetPartTID(m_haveItem[0].dwTid);
			m_pSelectPartDlg->Open(
				(FRESH_PFN_RESULT)&FrRecycleDlg::OnSelectPartDlgResult,
				WPoint(68.0f, 48.0f), 0);
		}
	}
}

bool FrRecycleDlg::OnSelectPartDlgResult(int ret, FrForm* pForm)
{
	if (ret == 1)
	{
		m_selectedPart = m_pSelectPartDlg->m_result;
		m_partSelected = true;
		// 983 (008dbcf0): o ingrediente da peca escolhida passa a apontar para ela
		for (int i = 0; i < 4; i++)
			if (m_haveItem[i].dwTid == m_pSelectPartDlg->GetPartTID983())
				m_haveItem[i].dwItemid = m_pSelectPartDlg->m_result;
	}
	else
	{
		m_selectedPart = 0;
		m_partSelected = false;
	}

	m_pSelectPartDlg = 0;

	return true;
}

void FrRecycleDlg::OnCardSlotResultInit(int param)
{
	m_pCardSlotResult = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 (s102): controles novos (mapa 00e2a930)
void FrRecycleDlg::OnMagicBoxBnInit983(int param)
{
	m_pMagicBoxBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pMagicBoxBn983)
		m_pMagicBoxBn983->SetVisible(true);
	if (m_pMagicBoxBn983)
		m_pMagicBoxBn983->SetPushDelay(0.01f);
	if (m_pMagicBoxBn983)
		m_pMagicBoxBn983->EnableHover(true, 0.0f);
}

void FrRecycleDlg::OnMagicBoxBnUp983()
{
}

void FrRecycleDlg::OnCardComposeBnInit983(int param)
{
	m_pCardComposeBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pCardComposeBn983)
		m_pCardComposeBn983->SetVisible(true);
	if (m_pCardComposeBn983)
		m_pCardComposeBn983->SetPushDelay(0.01f);
	if (m_pCardComposeBn983)
		m_pCardComposeBn983->EnableHover(true, 0.0f);
}

// fecha e abre a composicao de cartas (msg 0x324 do 983: CTaskMain "card_compose")
void FrRecycleDlg::OnCardComposeBnUp983()
{
	OnCloseBnUp();
	CTaskManager* pTaskManager = CTaskManager::Instance();
	(!pTaskManager ? 0 : pTaskManager->GetCurrentTask())
		->SendMsgToMainActor(MsgObject(0, 0x324 /*so do 983*/, 0, 0, 0, 0, 0));
}

void FrRecycleDlg::OnExchangeBnInit983(int param)
{
	m_pExchangeBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pExchangeBn983)
		m_pExchangeBn983->SetVisible(true);
	if (m_pExchangeBn983)
		m_pExchangeBn983->SetPushDelay(0.01f);
	if (m_pExchangeBn983)
		m_pExchangeBn983->EnableHover(true, 0.0f);
}

// fecha e abre a troca (msg 0x2e3 do 983: CTaskMain "AlchemyDlg")
void FrRecycleDlg::OnExchangeBnUp983()
{
	OnCloseBnUp();
	CTaskManager* pTaskManager = CTaskManager::Instance();
	(!pTaskManager ? 0 : pTaskManager->GetCurrentTask())
		->SendMsgToMainActor(MsgObject(0, 0x2e3 /*so do 983*/, 0, 0, 0, 0, 0));
}

void FrRecycleDlg::OnBackCenterInit983(int param)
{
	m_pBackCenter983 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrRecycleDlg::OnBackBottomInit983(int param)
{
	m_pBackBottom983 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

// 983 (008dd9a0): posicao e cor dos 5 rotulos do item da lista (largura/altura do item)
void FrRecycleDlg::SetLabels983(int itemWidth, int itemHeight)
{
	m_label983[0].pBitmap = g_pFresh->GetBitmap("recycle_text_owned");
	m_label983[1].pBitmap = g_pFresh->GetBitmap("level_001");
	m_label983[2].pBitmap = g_pFresh->GetBitmap("recycle_expired");
	m_label983[3].pBitmap = g_pFresh->GetBitmap("recycle_scheduled");
	// 983: o ultimo bitmap fica num local (ebx) e o bloco [4] le dele
	const Bitmap* pInPeriod = g_pFresh->GetBitmap("recycle_in_period");
	m_label983[4].pBitmap = pInPeriod;

	if (m_label983[1].pBitmap)
	{
		m_label983[1].color = 0xff909090;
		m_label983[1].rect.x = (float)(itemWidth * 0.5 - (float)(int)(float)((float)m_label983[1].pBitmap->Width() * 0.5));
		m_label983[1].rect.y = (float)(itemHeight * 0.5 - (float)(int)(float)((float)m_label983[1].pBitmap->Height() * 0.5));
		m_label983[1].rect.w = (float)m_label983[1].pBitmap->Width();
		m_label983[1].rect.h = (float)m_label983[1].pBitmap->Height();
	}

	if (m_label983[0].pBitmap)
	{
		m_label983[0].color = 0xff909090;
		m_label983[0].rect.x = (float)(itemWidth * 0.5 - (float)(int)(float)((float)m_label983[0].pBitmap->Width() * 0.5));
		m_label983[0].rect.y = (float)(m_label983[1].rect.h + m_label983[1].rect.y + 10.0); // 983: fadd qword [00cff068] = 10.0
		m_label983[0].rect.w = (float)m_label983[0].pBitmap->Width();
		m_label983[0].rect.h = (float)m_label983[0].pBitmap->Height();
	}

	for (int i = 2; i < 4; i++)
	{
		if (m_label983[i].pBitmap)
		{
			m_label983[i].color = 0xffffffff;
			m_label983[i].rect.x = (float)(itemWidth - m_label983[i].pBitmap->Width());
			m_label983[i].rect.y = 1.0f;
			m_label983[i].rect.w = (float)m_label983[i].pBitmap->Width();
			m_label983[i].rect.h = (float)m_label983[i].pBitmap->Height();
		}
	}

	if (pInPeriod)
	{
		m_label983[4].color = 0xffffffff;
		m_label983[4].rect.x = (float)(itemWidth - pInPeriod->Width());
		m_label983[4].rect.y = 1.0f;
		m_label983[4].rect.w = (float)pInPeriod->Width();
		m_label983[4].rect.h = (float)pInPeriod->Height();
	}
}

IMPLEMENT_OBJECT(FrSelectPartDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrSelectPartDlg, FrForm)
ON_FRESH_VI("list_box", FRCMD_INIT, FrSelectPartDlg::OnInitListBox)
ON_FRESH_VV("list_box", FRCMD_LBUTTONUP, FrSelectPartDlg::OnLBUpListBox)
ON_FRESH_VI("list_box", FRCMD_OWNERDRAW, FrSelectPartDlg::OnOwnerDrawListBox)
END_FRESH_MSGMAP()

FrSelectPartDlg::FrSelectPartDlg()
{
	m_pListBox = 0;
	m_partTid = 0;
	m_result = 0;
}

FrSelectPartDlg::~FrSelectPartDlg()
{
	m_partTid = 0;
}

void FrSelectPartDlg::SetPartTID(unsigned long tid)
{
	m_partTid = tid;

	if (m_pListBox)
	{
		m_pListBox->ClearItem();

		for (std::list<sItemInfo>::iterator it =
				 CSharedDoc::Instance()->m_partsList.begin();
			it != CSharedDoc::Instance()->m_partsList.end(); ++it)
		{
			sItemInfo* pItem = &(*it);

			if (pItem->tid == m_partTid)
			{
				// A part a character wears is not offered (the search leaves
				// both loops at once: the original's jump to the next part).
				for (std::map<unsigned int, sCharacterInfo>::iterator ci =
						 CSharedDoc::Instance()->m_charMap.begin();
					ci != CSharedDoc::Instance()->m_charMap.end(); ++ci)
				{
					for (int i = 0; i < 24; i++)
					{
						if (pItem->guid == ci->second.ItemIdList[i])
							goto nextPart;
					}
				}

				m_pListBox->AddItem(pItem);
			}
nextPart:;
		}
	}
}

void FrSelectPartDlg::OnInitListBox(int param)
{
	m_pListBox = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrSelectPartDlg::OnLBUpListBox()
{
}

void FrSelectPartDlg::OnOwnerDrawListBox(int param)
{
	if (m_pListBox)
	{
		// 983 008deec0: o GDI nao e testado
		FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
		FrListItem* pItem = (FrListItem*)param;
		if (pItem)
		{
			sItemInfo* pInfo = (sItemInfo*)pItem->pData;
			if (pInfo)
			{
				const Bitmap* pNormal =
					g_pFresh->RegisterBitmap("item_base_n");
				const Bitmap* pOver =
					g_pFresh->RegisterBitmap("item_base_o");
				const Bitmap* pDown =
					g_pFresh->RegisterBitmap("item_base_d");
				const Bitmap* pEmpty =
					g_pFresh->RegisterBitmap("item_base_empty");

				const Bitmap* pBase;
				if (!pItem->pData)
				{
					pBase = pEmpty;
				}
				else if (pItem->selected)
				{
					pBase = pDown;
					m_result = pInfo->guid;
					Close(FrOK, true);
				}
				else if (pItem->underCursor)
				{
					pBase = pDown;
				}
				else
				{
					pBase = pNormal;
				}

				if (pBase)
				{
					pGI->DrawTexture(pBase,
						WRect(pItem->pos.x + 5.0f, pItem->pos.y,
							(float)pBase->Width(), (float)pBase->Height()),
						0xFFFFFFFF, 0);
				}

				unsigned long tid = pInfo->tid;
				const char* name = K2L_Compatibility(
					"\xbe\xcb\xbc\xf6\xbe\xf8\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb"); // "unknown item"
				const char* icon = "caddie_no";

				IFF_ITEM_COMMON* pCommon =
					ItemManager()->FindCommonItem(tid);
				if (pCommon)
				{
					name = pCommon->Name;
					icon = pCommon->Icon;
				}

				const Bitmap* pIcon = g_pFresh->GetBitmap(icon);
				if (pIcon)
				{
					pGI->DrawTexture(pIcon,
						WRect(pItem->pos.x + 7.0f, pItem->pos.y + 2.0f,
							(float)pIcon->Width(), (float)pIcon->Height()),
						0xFFFFFFFF, 0);
				}

				pGI->SetTextColor(0xFF000000, 0xFFFFFFFF);
				pGI->Print11(
					WPoint(pItem->pos.x + 182.0f, pItem->pos.y + 7.0f), 2,
					"%s", name);

				IFF_STRUCT::sPart* pPart = ItemManager()->FindPart(tid);
				if (pPart)
				{
					DrawStatus2(pGI, pItem, pPart->Attr, pPart->Slot, 0.0f);
				}
			}
		}
	}
}

// The text of one status line of a part: the status name with the part's
// bonus and the card slot's bonus ("%s+%d/%s+%d").
static const char* MakeStatusString(int value, int index, int slotValue)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		WSingleton<CSharedDoc>::Instance();

	const char* names[5] = {
		K2L_Compatibility("\xc6\xc4\xbf\xf6"),         // power
		K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1"), // control
		K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5"), // accuracy
		K2L_Compatibility("\xbd\xba\xc7\xc9"),         // spin
		K2L_Compatibility("\xc4\xbf\xba\xea")          // curve
	};

	const char* slotName = K2L_Compatibility("\xbd\xbd\xb7\xd4");

	if (value != 0)
	{
		if (slotValue != 0)
		{
			return MakeStr("%s+%d/%s+%d", names[index], value,
				slotName, slotValue);
		}
		else
		{
			return MakeStr("%s+%d", names[index], value);
		}
	}
	else if (slotValue != 0)
	{
		return MakeStr("%s%s+%d", names[index], slotName, slotValue);
	}

	return "";
}

void FrSelectPartDlg::DrawStatus2(FrGraphicInterface* pGI, FrListItem* pItem,
	short* const status, short* const slotStatus, float x)
{
	float offsetY = 0.0f;
	for (int i = 0; i < 5; i++)
	{
		if (status[i] != 0 || slotStatus[i] != 0)
		{
			pGI->SetTextColor(0xFF0000FF, 0xFFFFFFFF);
			pGI->Print11(WPoint(x + 182.0f, offsetY + x + 36.0f) + pItem->pos,
				2, MakeStatusString(status[i], i, slotStatus[i]));

			offsetY += 14.0f;
		}
	}
}

