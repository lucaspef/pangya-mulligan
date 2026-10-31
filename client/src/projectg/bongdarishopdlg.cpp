#include "minatl.h"
#include "exhibition.h"
#include "bongdarishopdlg.h"
#include "frarea.h"
#include "frdesktop.h"
#include "frbutton.h"
#include "fredit.h"
#include "frtextbutton.h"
#include "frviewer.h"
#include "wfont.h"
#include "wresrcmng.h"
#include "binstr.h"
#include "commonutil.h"
#include "frlistbox.h"
#include "frscrollbar.h"
#include "frgraphicinterface.h"
#include "frwndmanager.h"
#include "fresh.h"
#include "projectg.h"
#include "setitemdlg.h"
#include "jp/gamestringmanager.h" // GetGameString 00540a10
// --- part A ---
#include "simpleoverlay.h"
#include "woverlay.h"
#include "camerawork.h"
#include "soundmanager.h"
#include "avatar_avatar.h"
#include "taskmanager.h"
#include "jp/lua983.h"
#include "jp/shareddocjp.h" // IsControlServerService983
#include "s5/utilities.h"
#include "wlocalize.h"
#include "actor.h"
#include "msg983.h"
#include <math.h>
#include <stdio.h>
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwUID) == 0x8cb && offsetof(CSharedDoc, m_specialShotMark) == 0x11030); // A738
extern const WVector g_vZero983_f041c4; // 00f041c4


extern Fresh* g_pFresh;

// Null-preserving cast of a window found by name (the original stores the null in __rtti_obj).
#define STATIC_CAST(type, obj) \
	((obj) != NULL ? (type*)(obj) : (type*)(__rtti_obj = NULL))

// m_pScrBar is protected in FrWnd; the original reads it directly.
struct FrListBoxScroll : public FrListBox
{
	using FrWnd::m_pScrBar;
	using FrWnd::m_dwStyle; // D876: coinlist +0x44 (cl 14.44 0x40: +0x44 = m_wndText)
	using FrListBox::m_pItemUnderCursor; // D876: coinlist +0x104
};
C_ASSERT_VC10(offsetof(FrListBoxScroll, m_dwStyle) == 0x44);
C_ASSERT_VC10(offsetof(FrListBoxScroll, m_pItemUnderCursor) == 0x104);

#define STR_MOTION_RESTART "\xbb\xf5\xb7\xce\xbd\xc3\xc0\xdb"
#define STR_SFX_CLICK "\xb9\xf6\xc6\xb0\xc5\xac\xb8\xaf"
#define STR_ITEM0_13 "item0_13"

unsigned long g_arrdwRandNumCookieItem0[100] = { 0 };
unsigned long g_arrdwRandNumCookieItem1[100] = { 0 };
unsigned long g_arrdwRandNumCookieItem2[100] = { 0 };
unsigned long g_arrdwRandSelectCookieItem[100] = { 0 };
unsigned long g_arrdwRandSelectPangItem[100] = { 0 };

static char* s_strMotionDefault = "\xb1\xe2\xba\xbb";
static char* s_strMotionPick[5] = { "\xbb\xcc\xb1\xe2"
									"1",
	"\xbb\xcc\xb1\xe2"
	"2",
	"\xbb\xcc\xb1\xe2"
	"3",
	"\xbb\xcc\xb1\xe2"
	"4",
	"\xbb\xcc\xb1\xe2"
	"5" };
static char* s_strMotionRestart = STR_MOTION_RESTART;
static char* s_strMotionComboBase983 = "\xbf\xac\xbc\xd3\xbb\xcc\xb1\xe2\xb1\xe2\xba\xbb";
static char* s_strMotionComboFire983 = "\xbf\xac\xbc\xd3\xbb\xcc\xb1\xe2\xb9\xdf\xb5\xbf";
static char* s_strMotionComboReady983 = "\xbf\xac\xbc\xd3\xbb\xcc\xb1\xe2\xc1\xd8\xba\xf1";

inline WVector2D WView::GetScreenCenter() const
{
	return WVector2D(m_center_x, m_center_y);
}

// WMatrix::operator*=(const WMatrix&) is external for this unit, but wmath.h (PCH) only declares it for
// golfrule.cpp; a declaration there changes projectg (SaveScreenShot), so a local helper class names it.
class WMatrixEx : public WMatrix
{
public:
	void operator*=(const WMatrix& right);
};
#pragma comment(linker, \
	"/alternatename:??XWMatrixEx@@QAEXABVWMatrix@@@Z=??XWMatrix@@QAEXABV0@@Z")

// WView::SetScreenCenter(const WVector2D&) of the original. wview.h is in the PCH and one more declaration
// there changes projectg (SaveScreenShot), so a local helper class carries it.
class WViewEx : public WView
{
public:
	void SetScreenCenter(const WVector2D& center);
};

inline void WViewEx::SetScreenCenter(const WVector2D& center)
{
	if (0)
		((WView*)0)->WView::GetScreenCenter();
	WView::SetScreenCenter(center.x, center.y);
}

inline short CSharedDoc::GetBsUsableTimes()
{
	if (0)
		((WView*)0)->WView::GetScreenCenter();
	return m_myInfo.info.iBongdariShopUsableTimes;
}

inline short CSharedDoc::GetBsUsableBonusTimes()
{
	if (0)
		((WView*)0)->WView::GetScreenCenter();
	return m_myInfo.info.iBongdariShopBonusTimes;
}

inline short CSharedDoc::GetBsRemainedBonusTimes()
{
	if (0)
		((WView*)0)->WView::GetScreenCenter();
	return m_myInfo.info.iBongdariShopRemainedBonus;
}

inline FrBongdariShopDlg::sMovingData::sMovingData()
{
	if (0)
		((WView*)0)->WView::GetScreenCenter();
}

extern const WRect g_rectUnitW983; // 00e4296c (retangulo de origem padrao)
extern int g_lobbyModal983;        // 00e78cac

IMPLEMENT_OBJECT(FrBongdariShopDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBongdariShopDlg, FrForm)

ON_FRESH_VI("bongdari_shop", FRCMD_INIT, FrBongdariShopDlg::OnBongdariShopInit)
ON_FRESH_VI("back_select_gacha_stage", FRCMD_INIT, FrBongdariShopDlg::OnBackSelectBnInit)
ON_FRESH_VV("back_select_gacha_stage", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnBackSelectBnUp)
ON_FRESH_VI("aboutshop", FRCMD_INIT, FrBongdariShopDlg::OnAboutShopBnInit)
ON_FRESH_VV("aboutshop", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnAboutShopBnUp)
ON_FRESH_VI("totalitem", FRCMD_INIT, FrBongdariShopDlg::OnTotalItemBnInit)
ON_FRESH_VV("totalitem", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnTotalItemBnUp)
ON_FRESH_VI("bongdarishopnotice", FRCMD_INIT,
	FrBongdariShopDlg::OnBongdarishopNoticeBnInit)
ON_FRESH_VV("bongdarishopnotice", FRCMD_LBUTTONUP,
	FrBongdariShopDlg::OnBongdarishopNoticeBnUp)
ON_FRESH_VI("close", FRCMD_INIT, FrBongdariShopDlg::OnCloseBnInit)
ON_FRESH_VV("close", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnCloseBnUp)
ON_FRESH_VI("bongdari", FRCMD_INIT, FrBongdariShopDlg::OnBongdariInit)
ON_FRESH_VV("bongdari", FRCMD_OWNERDRAW, FrBongdariShopDlg::OnBongdariOwnerDraw)
ON_FRESH_VI("bonusball01", FRCMD_INIT, FrBongdariShopDlg::OnBonusBall01BnInit)
ON_FRESH_VV("bonusball01", FRCMD_OWNERDRAW,
	FrBongdariShopDlg::OnBonusBall01OwnerDraw)
ON_FRESH_VI("bonusball02", FRCMD_INIT, FrBongdariShopDlg::OnBonusBall02BnInit)
ON_FRESH_VV("bonusball02", FRCMD_OWNERDRAW,
	FrBongdariShopDlg::OnBonusBall02OwnerDraw)
ON_FRESH_VI("bonusball03", FRCMD_INIT, FrBongdariShopDlg::OnBonusBall03BnInit)
ON_FRESH_VV("bonusball03", FRCMD_OWNERDRAW,
	FrBongdariShopDlg::OnBonusBall03OwnerDraw)
ON_FRESH_VI("bonusball04", FRCMD_INIT, FrBongdariShopDlg::OnBonusBall04BnInit)
ON_FRESH_VV("bonusball04", FRCMD_OWNERDRAW,
	FrBongdariShopDlg::OnBonusBall04OwnerDraw)
ON_FRESH_VI("bonusball05", FRCMD_INIT, FrBongdariShopDlg::OnBonusBall05BnInit)
ON_FRESH_VV("bonusball05", FRCMD_OWNERDRAW,
	FrBongdariShopDlg::OnBonusBall05OwnerDraw)
ON_FRESH_VI("autostart_btn", FRCMD_INIT, FrBongdariShopDlg::OnAutoStartBnInit)
ON_FRESH_VV("autostart_btn", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnAutoStartBnUp)
ON_FRESH_VI("normal_play_btn", FRCMD_INIT, FrBongdariShopDlg::OnNormalPlayBnInit)
ON_FRESH_VV("normal_play_btn", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnNormalPlayBnUp)
ON_FRESH_VI("combo_play_btn", FRCMD_INIT, FrBongdariShopDlg::OnComboPlayBnInit)
ON_FRESH_VV("combo_play_btn", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnComboPlayBnUp)
ON_FRESH_VI("coinlist", FRCMD_INIT, FrBongdariShopDlg::OnCoinListInit)
ON_FRESH_VI("coinlist", FRCMD_OWNERDRAW, FrBongdariShopDlg::OnCoinListOwnerDraw)
ON_FRESH_VV("coinlist", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnCoinListUp)
ON_FRESH_VI("start", FRCMD_INIT, FrBongdariShopDlg::OnStartBnInit)
ON_FRESH_VV("start", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnStartBnUp)
ON_FRESH_VI("select_bongdari_shop", FRCMD_INIT, FrBongdariShopDlg::OnSelectBongBnInit)
ON_FRESH_VV("select_bongdari_shop", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnSelectBongBnUp)
ON_FRESH_VI("select_memorial_shop", FRCMD_INIT, FrBongdariShopDlg::OnSelectMemBnInit)
ON_FRESH_VV("select_memorial_shop", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnSelectMemBnUp)
ON_FRESH_VI("once_again", FRCMD_INIT, FrBongdariShopDlg::OnAgainBnInit)
ON_FRESH_VV("once_again", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnAgainBnUp)
ON_FRESH_VI("go_coin_select_state", FRCMD_INIT, FrBongdariShopDlg::OnGoCoinBnInit)
ON_FRESH_VV("go_coin_select_state", FRCMD_LBUTTONUP, FrBongdariShopDlg::OnGoCoinBnUp)

END_FRESH_MSGMAP()

FrBongdariShopDlg::FrBongdariShopDlg()
{
	m_pGacha983 = NULL;
	SendStateBongdariShop(true);
	m_state = 0;
	m_pBonusBallModel[0] = NULL;
	m_pBonusBallModel[1] = NULL;
	m_pBonusBallModel[2] = NULL;
	m_pBonusBallModel[3] = NULL;
	m_pBonusBallModel[4] = NULL;
	m_pDefaultSilhouette =
		g_pFresh->GetManager()->GetBitmap("ITEMS", STR_ITEM0_13);
	InitBongdariShop();
	InitRandomDataItemSilhouette();
	ShuffleRandomDataItemSilhouette();
	m_pCloseBn = NULL;
	m_pBongdariModel = NULL;
	m_pBongdari = NULL;
	m_pBonusBall[0] = NULL;
	m_pBonusBall[1] = NULL;
	m_pBonusBall[2] = NULL;
	m_pBonusBall[3] = NULL;
	m_pBonusBall[4] = NULL;
	m_pShadow = g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "shadow");
	m_pTextBongdariClick = g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP",
		"text_bongdari_click");
	m_pTextBonusBallClick = g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP",
		"text_bonusball_click");
	m_pTextFee[0] =
		g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "text_fee_pang");
	m_pTextFee[1] =
		g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "text_fee_coupon");
	m_textFee10Combo983 = (int)g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "text_fee_10combo_pang");
	m_pTextAutoClick983 = g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "text_bongdari_auto_click");
	m_pTextComboPlayClick983 = g_pFresh->GetManager()->GetBitmap("BONGDARI_SHOP", "text_bongdari_combo_play_click");
	m_clickTextOffset = 0.0f;
	m_clickTextDir = 0;
	m_bBongdariPressed = false;
	m_bUsingBonusBall = false;
	m_cursorArea = WRect(4.0f, 50.0f, 607.0f, 298.0f);
	m_bAnimating = false;
	m_hoverBonusBall = -1;
	m_cursor = 0;

	static char* texturesWind[1] = { "[font_wind.jpg" };
	tagWTITLEFONT fontWind;
	memset(&fontWind, 0, sizeof(fontWind));
	fontWind.filename = texturesWind;
	fontWind.fontw = 16;
	fontWind.fonth = 16;
	fontWind.numPages = 1;
	fontWind.texw = 128;
	fontWind.texh = 64;
	fontWind.pCharSet = "1234567890ym%-.?/:,";
	m_pFontWind = g_resrcmng->GetTitleFont();
	m_pFontWind->Create(&fontWind);

	static char* texturesBong[1] = { "[font_bong.jpg" };
	tagWTITLEFONT fontBong;
	memset(&fontBong, 0, sizeof(fontBong));
	fontBong.filename = texturesBong;
	fontBong.fontw = 9;
	fontBong.fonth = 16;
	fontBong.numPages = 1;
	fontBong.texw = 128;
	fontBong.texh = 32;
	fontBong.pCharSet = "1234567890,-";
	m_pFontBong = g_resrcmng->GetTitleFont();
	m_pFontBong->Create(&fontBong);

	static char* texturesBongRed[1] = { "[font_bong_red.jpg" };
	tagWTITLEFONT fontBongRed;
	memset(&fontBongRed, 0, sizeof(fontBongRed));
	fontBongRed.filename = texturesBongRed;
	fontBongRed.fontw = 9;
	fontBongRed.fonth = 16;
	fontBongRed.numPages = 1;
	fontBongRed.texw = 128;
	fontBongRed.texh = 32;
	fontBongRed.pCharSet = "1234567890,-";
	m_pFontBongRed = g_resrcmng->GetTitleFont();
	m_pFontBongRed->Create(&fontBongRed);

	m_pEmoti = new CSimpleUI("[emoti_object.png");
	m_pEmoti->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
	m_pEmoti->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);

	m_usableTimes[0] = -1;
	m_usableTimes[1] = -1;
	m_usableTimes[2] = 0;
	m_bChildDlgOpen = false;
	m_couponCount = 0;
	m_feeTextAlpha = 0x200;
	m_discountTextAlpha = 0x200;
	m_feeType = -1;
	m_feeTextY = 0.0f;
	m_bShowDiscountTimes = false;
	m_bShowDiscountCount = false;
	m_discountTextY = 0.0f;
	m_stateTimer = 0.0f;
	m_pTotalItemBn = NULL;
	m_exitEnableTime = 8.0f;
	// 983 (008c0a8d..008c0bf8): resultado do pacote 0x264 (SetBongdariResult983)
	m_bFxBoxActive983 = 0; // 983 008c0a8d grava word 0 em +0x590 (+0x591 = m_bResultReady983); lido no DrawMemorialDrop983
	m_bResultReady983 = false;
	m_result983 = -1;
	m_resultMode983 = -1;
	m_resultCount983 = 0;
	memset(m_resultBall983, 0, sizeof(m_resultBall983));
	m_usableTimes[0] = Doc()->GetBsUsableTimes();
	m_usableTimes[1] = Doc()->GetBsUsableBonusTimes();
	m_usableTimes[2] = Doc()->GetBsRemainedBonusTimes();

	if (IsLocalContent(S4_MATCHING_SYSTEM))
	{
		do
		{
			WSendPacket packet((enumClientPacket)0xc1);
			packet.Encode1(2);
			packet.Send((eSendTo)0);
		} while (0);
	}
	// 983 008c0340: campos do modo Memorial/combo
	m_bAutoPlay983 = false;
	m_i434_983 = 0;
	m_i438_983 = 9999;
	{
		static char* texturesCond[1] = { "[font_condition_x2.png" };
		tagWTITLEFONT fontCond;
		memset(&fontCond, 0, sizeof(fontCond));
		fontCond.filename = texturesCond;
		fontCond.fontw = 32;
		fontCond.fonth = 32;
		fontCond.numPages = 1;
		fontCond.texw = 256;
		fontCond.texh = 64;
		fontCond.pCharSet = "1234567890%";
		m_pFont983 = g_resrcmng->GetTitleFont();
		m_pFont983->Create(&fontCond);
		// D886: era *(float*)((char*)m_pFont983 + 0x60) cru (VC10 WFont::m_scale); no cl 14.44 m_scale = 0x5c (BaseObject
		// com std::string) e o cru gravava 0.7 em m_colorset[0] -> fonte "%" do Memorial em escala 1.0
		m_pFont983->SetScale(0.7f);
	}
	m_state = 0;
	m_f524_983 = 3.3f;
	m_pRareItemDlg983 = NULL;
	m_f51c_983 = 0.0f;
	m_effectAlpha983 = 0.0f;
	m_memorialStep983 = -1;
	m_achievementPoint983 = 0;
	m_resultCount983 = 0;
	m_bFlagAed983 = false;
	m_bFlagAee983 = false;
	for (int k = 0; k < 24; ++k)
		m_scaleTab983[k] = (k % 12) == 0 || (k % 12) == 4 || (k % 12) == 8 ? 1.0f : 0.0f;
	if (!CCameraWork::Instance())
		new CCameraWork();
	m_fxBox983.Init(g_resrcmng);
	m_fxBox983.SetCamera(&g_camera);
	m_fxBox983.m_frameTime = 33.333333f;
	{
		float s = (g_view->SCREEN_YS / 600.0f) * -30.0f;
		m_scaleTab983[9] = s * m_scaleTab983[6];
		m_scaleTab983[10] = m_scaleTab983[7] * s;
		m_scaleTab983[11] = s * m_scaleTab983[8];
	}
	m_pNormalPlayBn983 = NULL;
	m_pComboPlayBn983 = NULL;
	m_comboMode983 = 0;
	// overlays 983 +0x454..+0x4cc (ate m_pGacha983) e fonte de moedas: o dtor libera os != NULL
	memset(&m_pOvBg983, 0, (char*)&m_pGacha983 - (char*)&m_pOvBg983);
	m_pFontCoin983 = NULL;
	m_pOvComboBg983 = NULL;
	m_i0afc_983 = 0; // 983 008c042e
	m_coinCount983 = 0; // 983 008c0434
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 | 8);
	}
}

FrBongdariShopDlg::~FrBongdariShopDlg()
{
	Clear();
	SendReBuildMyInventory();
	SendDataClosedBonusBall();
	OpenClosedBonusBallResultDlg();
	if (IsLocalContent(S4_MATCHING_SYSTEM))
	{
		if (AfxGetTask())
		{
			WSendPacket packet((enumClientPacket)0xc1);
			packet.Encode1(1);
			packet.Send((eSendTo)0);
		}
	}
	// 983 008bef30: libera fontes/overlays do modo Memorial/combo e o gacha
	if (m_pFont983)
		m_pFont983->Erase();
	if (g_resrcmng && m_pFont983)
	{
		g_resrcmng->Release(m_pFont983);
		m_pFont983 = NULL;
	}
	{
		WOverlay** list[] = { &m_pOvMemDrop983[0], &m_pOvMemDrop983[1], &m_pOvMemDrop983[2],
			&m_pOvMemPointSlot983, &m_pOvBg983, &m_pOvComboBg983, &m_pOvPointSlot983,
			&m_pOvMemBack983[0], &m_pOvMemBack983[1], &m_pOvMemBack983[2], &m_pOvMemBack983[3],
			&m_pOvMemBack983[4], &m_pOvMemShopBg983,
			&m_pOvCoinSlot983[0], &m_pOvCoinSlot983[3], &m_pOvCoinSlot983[1], &m_pOvCoinSlot983[4],
			&m_pOvCoinSlot983[2], &m_pOvCoinSlot983[5],
			&m_pOvTxtExplanBong983, &m_pOvTxtExplanMem983, &m_pOvRareMask983, &m_pOvCoinList983,
			&m_pOvBongSelect983, &m_pOvBongSelectDis983, &m_pOvSelect983, &m_pOvSelectDis983,
			&m_pOvAgainDis983, &m_pOvAgain983, &m_pOvMainDis983, &m_pOvMain983 };
		for (int k = 0; k < (int)(sizeof(list) / sizeof(list[0])); ++k)
		{
			if (g_resrcmng && *list[k])
			{
				g_resrcmng->Release((WResource*)*list[k]);
				*list[k] = NULL;
			}
		}
	}
	if (m_pGacha983)
	{
		delete m_pGacha983;
		m_pGacha983 = NULL;
	}
	ClearCoinListData983();
	if (CCameraWork::Instance())
		delete CCameraWork::Instance();
	m_fxBox983.ClearAll(false);
	if (m_pFontCoin983)
		m_pFontCoin983->Erase();
	if (g_resrcmng && m_pFontCoin983)
	{
		g_resrcmng->Release(m_pFontCoin983);
		m_pFontCoin983 = NULL;
	}
	if (CTaskManager::Instance())
	{
		AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x1a3, 1, 0, 0, 0, 0); // 983 008bf398: direto, nao PostMsg
		CTaskManager::Instance()->PostMsg(NULL, "AvatarChat", 0x58, 0, 0, 0, 0);
	}
	m_listSecond983.clear();
	if (g_lobbyModal983 == 1 && ntAvatarManager::m_pInstance)
	{
		ntAvatar* pMyAvatar = ntAvatarManager::m_pInstance->m_pMyAvatar;
		if (pMyAvatar)
			pMyAvatar->SetState(pMyAvatar->m_state983 & ~8);
	}
}

// end coordinator provides

// {count, value}: counts add up to 100
unsigned long g_arrdwPercentNumCookieItem0[][2] = {
	{ 70, 0 },
    { 15, 1 },
    { 8,  2 },
    { 5,  3 },
    { 2,  4 }
};
unsigned long g_arrdwPercentNumCookieItem1[][2] = {
	{ 60, 1 },
    { 25, 2 },
    { 10, 3 },
    { 5,  4 }
};
unsigned long g_arrdwPercentNumCookieItem2[][2] = {
	{ 70, 2 },
    { 20, 3 },
    { 10, 4 }
};
// 00e27010 (983)
unsigned long g_arrdwPercentSelectCookieItem[][2] = {
	{ 12,0x1800000e },
	{ 11,0x1800000b },
	{ 11,0x1800000a },
	{ 12,0x18000009 },
	{ 11,0x18000006 },
	{ 13,0x1a000000 },
	{ 6, 0x1a000002 },
	{ 6, 0x14000005 },
	{ 6, 0x14000003 },
	{ 6, 0x14000002 },
	{ 6, 0x14000001 }
};
// 00e27068 (983)
unsigned long g_arrdwPercentSelectPangItem[][2] = {
	{ 14,0x18000008 },
	{ 14,0x18000007 },
	{ 14,0x18000001 },
	{ 14,0x18000000 },
	{ 15,0x18000004 },
	{ 14,0x18000005 },
	{ 15,0x1a000028 }
};

WPoint g_arrPtItemSilhouette[] = { WPoint(28.0f, 357.0f), WPoint(93.0f, 357.0f),
	WPoint(158.0f, 357.0f), WPoint(223.0f, 357.0f), WPoint(288.0f, 357.0f),
	WPoint(353.0f, 357.0f), WPoint(418.0f, 357.0f), WPoint(483.0f, 357.0f),
	WPoint(548.0f, 357.0f) };

void FrBongdariShopDlg::OnBongdariShopInit(int param)
{
}

void FrBongdariShopDlg::OnAboutShopBnInit(int param)
{
	m_pAboutBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param); // 008b5480
}

void FrBongdariShopDlg::OnAboutShopBnUp()
{
	if (m_shopMode983 == 0) // 008bc570: no modo de escolha so liga/desliga a explicacao
		m_bFlagAed983 = !m_bFlagAed983;
	else
		OpenAboutShopDlg();
}

void FrBongdariShopDlg::OnTotalItemBnInit(int param)
{
	m_pTotalItemBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pTotalItemBn != NULL)
		m_pTotalItemBn->EnableBlink(false);
}

void FrBongdariShopDlg::OnTotalItemBnUp()
{
	OpenTotalItemDlg();
}

void FrBongdariShopDlg::OnBongdarishopNoticeBnInit(int param)
{
	m_pNoticeBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param); // 008b5570
	m_pNoticeBn983->Enable(false);
}

void FrBongdariShopDlg::OnBongdarishopNoticeBnUp()
{
	OpenBongdarishopNoticeDlg(false);
}

void FrBongdariShopDlg::OnCloseBnInit(int param)
{
	m_pCloseBn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

// 983 008b9c90: devolve o fundo do lobby ja no clique, sem esperar o fade (o dtor repete)
void FrBongdariShopDlg::OnCloseBnUp()
{
	OnFreshOkay();
	AfxGetTask()->GetActor("LobbyAvatar") << MsgObject(NULL, 0x1a3, 1, 0, 0, 0, 0);
}

void FrBongdariShopDlg::OnBongdariInit(int param)
{
	m_pBongdari = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBongdari != NULL)
		m_pBongdari->SetMouseEvent(false);
}

// 983: modo Memorial/combo da loja (nomes meus; pseudo-C do 983 em 008b3df0..008b9630)
#define DrawOverlay983(ov, px, py, pcolor) \
	do \
	{ \
		WRect rOv983((px), (py), (float)(ov)->GetWidth(), (float)(ov)->GetHeight()); \
		(ov)->Render(g_view, g_rectUnitW983, rOv983, 0, (pcolor), 0.0f, 0); \
	} while (0)

void FrBongdariShopDlg::SetButtonStatus983() // 008b3df0
{
	if (m_comboMode983 == 0)
	{
		m_pNormalPlayBn983->SetStatus((FrButton::eButMode)2);
		m_pComboPlayBn983->SetStatus((FrButton::eButMode)0);
	}
	else if (m_comboMode983 == 1)
	{
		m_pNormalPlayBn983->SetStatus((FrButton::eButMode)0);
		m_pComboPlayBn983->SetStatus((FrButton::eButMode)2);
	}
}

void FrBongdariShopDlg::LoadMemorialOverlays983() // 008b4910
{
	m_pOvBg983 = g_resrcmng->GetOverlay("bongdari_shop_bg.tga", 0);
	m_pOvPointSlot983 = g_resrcmng->GetOverlay("bongdari_shop_pointslot.tga", 0);
	m_pOvComboBg983 = g_resrcmng->GetOverlay("bongdari_shop_combo_bg.tga", 0);
	m_pOvMemPointSlot983 = g_resrcmng->GetOverlay("memorial_shop_pointslot.tga", 0);
	m_pOvMemBack983[0] = g_resrcmng->GetOverlay("memorial_back0.png", 0);
	m_pOvMemBack983[1] = g_resrcmng->GetOverlay("memorial_back1.png", 0);
	m_pOvMemBack983[2] = g_resrcmng->GetOverlay("memorial_back2.png", 0);
	m_pOvMemBack983[3] = g_resrcmng->GetOverlay("memorial_back3.png", 0);
	m_pOvMemBack983[4] = g_resrcmng->GetOverlay("memorial_back4.png", 0);
	m_pOvMemShopBg983 = g_resrcmng->GetOverlay("memorial_shop_bg.tga", 0);
	m_pOvCoinSlot983[1] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_premium_n.tga", 0);
	m_pOvCoinSlot983[4] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_premium_d.tga", 0);
	m_pOvCoinSlot983[0] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_normal_n.tga", 0);
	m_pOvCoinSlot983[3] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_normal_d.tga", 0);
	m_pOvCoinSlot983[2] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_special_n.tga", 0);
	m_pOvCoinSlot983[5] = g_resrcmng->GetOverlay("memorial_shop_coinslot_bn_special_d.tga", 0);
	m_pOvMemDrop983[0] = g_resrcmng->GetOverlay("memorial_drop03.png", 0);
	m_pOvMemDrop983[1] = g_resrcmng->GetOverlay("memorial_drop02.png", 0);
	m_pOvMemDrop983[2] = g_resrcmng->GetOverlay("memorial_drop01.png", 0);
	m_pOvTxtExplanBong983 = g_resrcmng->GetOverlay("bongdari_shop_txt_explan.tga", 0);
	m_pOvTxtExplanMem983 = g_resrcmng->GetOverlay("memorial_shop_txt_explan.tga", 0);
	m_pOvRareMask983 = g_resrcmng->GetOverlay("rare_shop_mask.tga", 0);
	m_pOvCoinList983 = g_resrcmng->GetOverlay("memorial_shop_coinlist.tga", 0);
	m_pOvSelect983 = g_resrcmng->GetOverlay("memorial_shop_select.tga", 0);
	m_pOvSelectDis983 = g_resrcmng->GetOverlay("memorial_shop_select_disable.tga", 0);
	m_pOvBongSelect983 = g_resrcmng->GetOverlay("bongdari_shop_select.tga", 0);
	m_pOvBongSelectDis983 = g_resrcmng->GetOverlay("bongdari_shop_select_disable.tga", 0);
	m_pOvAgainDis983 = g_resrcmng->GetOverlay("memorial_shop_again_disable.tga", 0);
	m_pOvAgain983 = g_resrcmng->GetOverlay("memorial_shop_again.tga", 0);
	m_pOvMainDis983 = g_resrcmng->GetOverlay("memorial_shop_main_disable.tga", 0);
	m_pOvMain983 = g_resrcmng->GetOverlay("memorial_shop_main.tga", 0);
}

void FrBongdariShopDlg::CreateCoinFont983() // 008b4c00
{
	static char* textures[1] = { "[font_bong.jpg" }; // 983: tabela em 00e28110
	tagWTITLEFONT font;
	memset(&font, 0, sizeof(font));
	font.filename = textures;
	font.fontw = 9;
	font.fonth = 16;
	font.numPages = 1;
	font.texw = 128;
	font.texh = 32;
	font.pCharSet = "1234567890,-";
	m_pFontCoin983 = g_resrcmng->GetTitleFont();
	m_pFontCoin983->Create(&font);
}

void FrBongdariShopDlg::SetHitRects983() // 008b7e90
{
	const WRect& r = m_pBongdari->GetRect();
	m_rcHit1983.x = m_cfgBsOverX983 + r.x;
	m_rcHit1983.y = m_cfgBsOverY983 + r.y;
	m_rcHit1983.w = m_pSelectBongBn983->GetRect().w + 100.0f;
	m_rcHit1983.h = (float)m_pOvBongSelect983->GetHeight() + m_pSelectBongBn983->GetRect().h;
	m_rcHit2983.x = m_cfgMsOverX983 + r.x;
	m_rcHit2983.y = m_cfgMsOverY983 + r.y;
	m_rcHit2983.w = m_pSelectMemBn983->GetRect().w + 100.0f;
	m_rcHit2983.h = (float)m_pOvSelect983->GetHeight() + m_pSelectMemBn983->GetRect().h;
}

void FrBongdariShopDlg::CreateGacha983() // 008b7f30
{
	if (m_pGacha983)
	{
		delete m_pGacha983;
		m_pGacha983 = NULL;
	}
	m_pGacha983 = new CExhibitionByHardwareTnL(
		m_cfgGachaX983 + m_rect.x + m_rect.w * 0.5f,
		m_rect.h * 0.5f + m_rect.y + m_cfgGachaY983, m_cfgDolpiniFootDown983,
		true, 0xffffffff);
	if (m_pGacha983)
	{
		float fov = (m_cfgDistanceCamera983 * 800.0f) / 640.0f;
		float v = fov - 5.0f;
		m_pGacha983->SetDistanceRange(v, v);
		m_pGacha983->SetDistance(fov, true, true);
		m_pGacha983->SetArea(m_rect.w, m_rect.h);
		m_pGacha983->SetModel("memorial_gacha", (eExhibitType)1);
		m_pGacha983->SetDirection(m_cfgDolpiniAngleY983);
		m_pGacha983->m_pitch = m_cfgDolpiniAngleX983; // +0x28
		m_pGacha983->SetControlFlag(0x1f); // D873: m_flags +0x564 (cl 14.44 0x524)
	}
}

void FrBongdariShopDlg::DrawBgMode1983() // 008b86a0
{
	const WRect& r = m_pBongdari->GetRect();
	WOverlay* ov = (m_comboMode983 == 1) ? m_pOvComboBg983 : m_pOvBg983;
	DrawOverlay983(ov, r.x + 3.0f, r.y + 3.0f, 0xffffffff);
}

void FrBongdariShopDlg::DrawPointSlot983() // 008b8770
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvPointSlot983, r.x + m_cfgPointSlotX983, r.y + m_cfgPointSlotY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialBack983() // 008b8800
{
	const WRect& r = m_pBongdari->GetRect();
	int idx = (m_achievementPoint983 - 1) / 1000;
	if (idx > 4)
		idx = 4;
	DrawOverlay983(m_pOvMemBack983[idx], r.x + m_cfgBackX983, r.y + m_cfgBackY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialShopBg983() // 008b88b0
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvMemShopBg983, r.x + m_cfgBackX983, r.y + m_cfgBackY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialCoinList983() // 008b8940
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvCoinList983, 7.0f + r.x + m_cfgBackX983, m_cfgBackY983 + r.y + 7.0f, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialExplan983() // 008b89e0
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvTxtExplanBong983, r.x + 17.0f + m_cfgBackX983, r.y + 50.0f + m_cfgBackY983, 0xffffffff);
	DrawOverlay983(m_pOvTxtExplanMem983, r.x + 317.0f + m_cfgBackX983, r.y + 50.0f + m_cfgBackY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialPointSlot983() // 008b8af0
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvMemPointSlot983, r.x + m_cfgPointSlotX983, r.y + m_cfgPointSlotY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialDrop983(int index) // 008b8b80
{
	if (m_pOvMemDrop983[index] && m_bFxBoxActive983)
	{
		const WRect& r = m_pBongdari->GetRect();
		int alpha = (int)(m_effectAlpha983 + 0.5f);
		DrawOverlay983(m_pOvMemDrop983[index], r.x + m_cfgEffectImgX983, r.y + m_cfgEffectImgY983,
			alpha * 0x1000000 + 0xffffff);
	}
}

void FrBongdariShopDlg::DrawRareMask983() // 008b8c50
{
	const WRect& r = m_pBongdari->GetRect();
	WRect d(0.5f + r.x, r.y, r.w - 0.5f, r.h);
	m_pOvRareMask983->Render(g_view, g_rectUnitW983, d, 0, 0xffffffff, 0.0f, 0);
}

void FrBongdariShopDlg::DrawBongdariSelect983() // 008b8ce0
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvBongSelect983, r.x + m_cfgBsOverX983, r.y + m_cfgBsOverY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialSelect983() // 008b8d70
{
	const WRect& r = m_pBongdari->GetRect();
	DrawOverlay983(m_pOvSelect983, r.x + m_cfgMsOverX983, r.y + m_cfgMsOverY983, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialAgain983() // 008b8e00
{
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	const WRect& r = m_pBongdari->GetRect();
	WRect& hit = m_rcHit1983;
	if (!(hit.x < mouse.x) || hit.w + hit.x < mouse.x || !(hit.y < mouse.y) ||
		hit.h + hit.y < mouse.y || m_coinCount983 < 1)
	{
		DrawOverlay983(m_pOvAgainDis983, r.x + m_cfgBsOverX983, r.y + m_cfgBsOverY983 + 100.0f, 0xffffffff);
	}
	else
	{
		DrawOverlay983(m_pOvAgain983, r.x + m_cfgBsOverX983, r.y + m_cfgBsOverY983 + 100.0f, 0xffffffff);
		m_pAgainBn983->SetStatus((FrButton::eButMode)1);
	}
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	const Bitmap* icon = m_coinItemName983 ? g_pFresh->GetBitmap(m_coinItemName983) : NULL;
	if (icon)
	{
		WRect ir(r.x + m_cfgAddIconX983, r.y + m_cfgAddIconY983 + 3.0f, 50.0f, 50.0f);
		gdi->DrawTexture(icon, ir, 0xffffffff, 0);
	}
	float tx = m_cfgAddIconX983 + r.x + 63.0f;
	float ty = m_cfgAddIconY983 + r.y + 10.0f;
	char* s = MakeStr("%d", m_coinCount983);
	m_pFontCoin983->Print(g_view, tx, ty, s, 0, 0xffffffff, NULL);
	unsigned long saved = gdi->GetTextColor();
	gdi->SetTextColor(0xffffffff, 0xff000000);
	WPoint tp(m_cfgAddIconX983 + r.x + 47.0f, m_cfgAddIconY983 + r.y + 55.0f); // 983: x+47, y+55
	g_pFresh->GetManager()->PrintText(tp, 1, m_pCoinText983, -1.0f, 0xff000000);
	gdi->SetTextColor(saved, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialMain983() // 008b90f0
{
	const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
	const WRect& r = m_pBongdari->GetRect();
	WRect& hit = m_rcHit2983;
	if (hit.x < mouse.x && mouse.x <= hit.w + hit.x && hit.y < mouse.y && mouse.y <= hit.h + hit.y)
	{
		DrawOverlay983(m_pOvMain983, r.x + m_cfgMsOverX983, r.y + m_cfgMsOverY983 + 100.0f, 0xffffffff);
		m_pGoCoinBn983->SetStatus((FrButton::eButMode)1);
		return;
	}
	DrawOverlay983(m_pOvMainDis983, r.x + m_cfgMsOverX983, r.y + m_cfgMsOverY983 + 100.0f, 0xffffffff);
}

void FrBongdariShopDlg::DrawMemorialPoints983() // 008b9380
{
	// 983: o laco de strlen deixa o ponteiro apos o '\0'; a conta (p - 1 - (buf + 1)) da len - 1, nao len (E244: "0" e "40" 8 px a esquerda)
	char text[32] = { 0 };
	MakePriceStrA(text, (__int64)m_achievementPoint983);
	float w = m_pFontCoin983->GetFontInfo()->fontw - 1.0f;
	m_pFontCoin983->Print(g_view, (m_cfgPointFontX983 + m_rect.x) - (float)((int)strlen(text) - 1) * w,
		m_cfgPointFontY983 + m_rect.y, text, 0, 0xffffffff, NULL);
	memset(text, 0, sizeof(text));
	int next = (m_achievementPoint983 < 0x1c21) ? (m_achievementPoint983 - 1) / 300 : 0x18;
	MakePriceStrA(text, (__int64)next);
	m_pFontCoin983->Print(g_view, ((m_cfgPointFontX983 + m_rect.x) - 245.0f) - (float)((int)strlen(text) - 1) * w,
		m_cfgPointFontY983 + m_rect.y, text, 0, 0xffffffff, NULL);
}

void FrBongdariShopDlg::DisplayGacha983(float footDown) // 008b9520
{
	if (m_pGacha983 && CCameraWork::Instance())
	{
		WMatrix cam;
		const float* src = (m_memorialStep983 < 1) ? (const float*)((char*)CCameraWork::Instance() + VC10_CAMWORK_MATSELECT)
											   : &m_scaleTab983[12];
		memcpy(&cam, src, 12 * sizeof(float));
		m_pGacha983->DisplayCamera983(true, 0.5f, footDown, m_cfgDolpiniFov983,
			m_pBongdari->GetRect(), cam);
		g_view->SetCamera(g_camera); // o DisplayCamera983 nao repoe a camera: sem isto o mundo do lobby fica na camera do gacha
		g_view->UpdateCamera();
	}
}

void FrBongdariShopDlg::DisplayFxBox983() // 008b9630
{
	// CFxBox::Finished (0096d7f0): sem sequencias (+0x174) nem sprays (+0x164)
	if (*(int*)((char*)&m_fxBox983 + VC10_FXBOX_SEQN) == 0 && *(int*)((char*)&m_fxBox983 + VC10_FXBOX_SPRAYN) == 0)
		return;
	bool fog = g_view->SetFogEnable(false);
	WMatrix savedCam = g_view->GetCamera();
	float oldNear = g_view->clip_near;
	float oldFar = g_view->clip_far;
	WView::PROJECTION_MODE oldMode = g_view->GetProjectionMode();
	float oldFov = g_view->GetFOV();
	g_view->SetProjectionMode((WView::PROJECTION_MODE)0);
	g_view->SetCamera(*(const WMatrix*)m_scaleTab983);
	g_view->SetFOV(1.5707964f);
	g_view->SetClip(0.5f, 100.0f, false);
	g_view->SetClippingArea(m_pBongdari->GetRect());
	g_view->UpdateCamera();
	m_fxBox983.Display(g_view);
	g_view->Flush(1, 1);
	g_view->SetProjectionMode(oldMode);
	g_view->SetCamera(savedCam);
	g_view->SetClip(oldNear, oldFar, false);
	g_view->SetFOV(oldFov);
	WRect rc(0.0f, 0.0f, g_view->SCREEN_XS, g_view->SCREEN_YS);
	g_view->SetClippingArea(rc);
	g_view->SetScreenCenter(g_view->SCREEN_XS * 0.5f, g_view->SCREEN_YS * 0.5f);
	g_view->UpdateCamera();
	g_view->SetFogEnable(fog);
}

void FrBongdariShopDlg::LoadMemorialConfig983() // 008bd9c0: memorial.lua / memorial_ui_config
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("memorial.lua");
	lua_system::lua_table_helper t = script->GetTable("memorial_ui_config");
#define MEMCFG(field, key) field = (float)LUA983_GET(t, float, key)
	MEMCFG(m_cfgBackX983, "back_x");
	MEMCFG(m_cfgBackY983, "back_y");
	MEMCFG(m_cfgPointSlotX983, "point_slot_x");
	MEMCFG(m_cfgPointSlotY983, "point_slot_y");
	MEMCFG(m_cfgPointFontX983, "point_font_x");
	MEMCFG(m_cfgPointFontY983, "point_font_y");
	MEMCFG(m_cfgGachaX983, "gacha_machine_x");
	MEMCFG(m_cfgGachaY983, "gacha_machine_y");
	MEMCFG(m_cfgListFrameX983, "list_frame_x");
	MEMCFG(m_cfgListFrameY983, "list_frame_y");
	MEMCFG(m_cfgDistanceCamera983, "distance_camera");
	MEMCFG(m_cfgEffectAlpha983, "add_effect_img_alpha");
	MEMCFG(m_cfgEffectImgX983, "effect_img_x");
	MEMCFG(m_cfgEffectImgY983, "effect_img_y");
	MEMCFG(m_cfgCoinIconW983, "coin_icon_rect_w");
	MEMCFG(m_cfgCoinIconH983, "coin_icon_rect_h");
	MEMCFG(m_cfgAddCoinNumX983, "add_coin_num_x");
	MEMCFG(m_cfgAddListTextY983, "add_list_text_y");
	MEMCFG(m_cfgEffectTimeFail983, "add_effect_time_fail");
	MEMCFG(m_cfgShowResultDlg983, "show_result_dlg");
	MEMCFG(m_cfgDolpiniAngleX983, "dolpini_angle_x");
	MEMCFG(m_cfgDolpiniAngleY983, "dolpini_angle_y");
	MEMCFG(m_cfgDolpiniFov983, "dolpini_fov");
	MEMCFG(m_cfgDolpiniFootDown983, "dolpini_footdown");
	m_f540_983 = 200.0f;
	MEMCFG(m_cfgAddBongdariX983, "add_bongdari_x");
	MEMCFG(m_cfgAddBongdariY983, "add_bongdari_y");
	MEMCFG(m_cfgStartImgX983, "add_bongdari_start_img_x");
	MEMCFG(m_cfgStartImgY983, "add_bongdari_start_img_y");
	MEMCFG(m_cfgBsOverX983, "add_BS_btn_over_img_x");
	MEMCFG(m_cfgBsOverY983, "add_BS_btn_over_img_y");
	MEMCFG(m_cfgMsOverX983, "add_MS_btn_over_img_x");
	MEMCFG(m_cfgMsOverY983, "add_MS_btn_over_img_y");
	MEMCFG(m_cfgShopDisableX983, "add_shop_disable_img_x");
	MEMCFG(m_cfgShopDisableY983, "add_shop_disable_img_y");
	MEMCFG(m_cfgAddCloseGachaY983, "add_close_gacha_shop_y");
	MEMCFG(m_cfgAddIconX983, "add_icon_img_x");
	MEMCFG(m_cfgAddIconY983, "add_icon_img_y");
#undef MEMCFG
	lua_system::lua_script_manger::GetInstance()->DeleteScript(script);
}

void FrBongdariShopDlg::OnBongdariOwnerDraw()
{
	if (m_shopMode983 == 0)
	{
		if (m_bFlagAed983)
		{
			DrawRareMask983();
			DrawMemorialExplan983();
			return;
		}
		const WPoint& mouse = g_pFresh->GetManager()->GetMousePos();
		if (m_rcHit1983.IsInRect(mouse))
		{
			m_pSelectBongBn983->SetStatus((FrButton::eButMode)1);
			DrawBongdariSelect983();
		}
		else if (m_rcHit2983.IsInRect(mouse))
		{
			m_pSelectMemBn983->SetStatus((FrButton::eButMode)1);
			DrawMemorialSelect983();
		}
	}
	else if (m_shopMode983 == 2)
	{
		DrawMemorialPointSlot983();
		if (m_memorialStep983 < 0)
			DrawMemorialShopBg983();
		else
			DrawMemorialBack983();
		DrawMemorialPoints983();
		DisplayGacha983(m_f540_983);
		if (m_memorialStep983 < 0)
			DrawMemorialCoinList983();
		if (m_memorialStep983 > 2)
		{
			DisplayFxBox983();
			DrawMemorialDrop983(m_resultMode983);
		}
		if (m_bFlagAee983)
		{
			DrawRareMask983();
			DrawMemorialAgain983();
			DrawMemorialMain983();
		}
	}
	else if (m_shopMode983 == 1)
	{
		DrawBgMode1983();
		DrawPointSlot983();
		DrawShadow();
		if (m_comboMode983 == 0)
			DisplayItemSilhouette();
		DrawUserInfo();
		if (m_state == 0)
			DrawTextBongdariClick();
		else if (m_state == 4)
			DrawTextBonusBallClick();
		g_view->Flush(0, 1);
		if (m_pBongdariModel)
		{
			m_pBongdariModel->DisplayClip(
				WRect(m_rect.x + 4.0f, m_rect.y + 49.0f, 607.0f, 455.25f), 55.0f,
				0.1f, 100.0f);
		}
	}
}

void FrBongdariShopDlg::OnBonusBall01BnInit(int param)
{
	m_pBonusBall[0] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBonusBall[0] != NULL)
	{
		m_pBonusBall[0]->SetVisible(false);
		m_pBonusBall[0]->SetMouseEvent(false);
	}
}

void FrBongdariShopDlg::OnBonusBall01OwnerDraw()
{
	DrawBonusBall(0);
}

void FrBongdariShopDlg::OnBonusBall02BnInit(int param)
{
	m_pBonusBall[1] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBonusBall[1] != NULL)
	{
		m_pBonusBall[1]->SetVisible(false);
		m_pBonusBall[1]->SetMouseEvent(false);
	}
}

void FrBongdariShopDlg::OnBonusBall02OwnerDraw()
{
	DrawBonusBall(1);
}

void FrBongdariShopDlg::OnBonusBall03BnInit(int param)
{
	m_pBonusBall[2] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBonusBall[2] != NULL)
	{
		m_pBonusBall[2]->SetVisible(false);
		m_pBonusBall[2]->SetMouseEvent(false);
	}
}

void FrBongdariShopDlg::OnBonusBall03OwnerDraw()
{
	DrawBonusBall(2);
}

void FrBongdariShopDlg::OnBonusBall04BnInit(int param)
{
	m_pBonusBall[3] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBonusBall[3] != NULL)
	{
		m_pBonusBall[3]->SetVisible(false);
		m_pBonusBall[3]->SetMouseEvent(false);
	}
}

void FrBongdariShopDlg::OnBonusBall04OwnerDraw()
{
	DrawBonusBall(3);
}

void FrBongdariShopDlg::OnBonusBall05BnInit(int param)
{
	m_pBonusBall[4] = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pBonusBall[4] != NULL)
	{
		m_pBonusBall[4]->SetVisible(false);
		m_pBonusBall[4]->SetMouseEvent(false);
	}
}

void FrBongdariShopDlg::OnBonusBall05OwnerDraw()
{
	DrawBonusBall(4);
}

// ---- 983: botoes/lista do modo Memorial/combo (msgmap 00e27140) ----
struct sCoinItem983 // item da lista de moedas (16 bytes, new em 008bdd20)
{
	const char* name;
	unsigned long tid;
	int qty;
	const char* icon;
};

void FrBongdariShopDlg::OnBackSelectBnInit(int param)
{
	m_pBackSelectBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param); // 008b52c0
	m_pBackSelectBn983->Enable(false); // 983: vtbl+0x18 (Enable), nao SetVisible
}

void FrBongdariShopDlg::OnBackSelectBnUp() // 008b5330
{
	// slots do 983: +0x18 = Enable, +0x1c = SetVisible
	m_shopMode983 = 0;
	m_bFlagAed983 = false;
	m_pSelectBongBn983->SetVisible(true);
	m_pSelectMemBn983->SetVisible(true);
	m_pNoticeBn983->Enable(false);
	m_pAboutBn983->SetStatus((FrButton::eButMode)0);
	m_pAboutBn983->Enable(true);
	m_bFlagAed983 = false;
	m_pCloseBn->Enable(true);
	m_pBackSelectBn983->Enable(false);
	m_pAboutBn983->SetButtonImg("rare_shop_about_bn_normal", (FrButton::eButMode)0);
	m_pAboutBn983->SetButtonImg("rare_shop_about_bn_over", (FrButton::eButMode)1);
	m_pAboutBn983->SetButtonImg("rare_shop_about_bn_normal", (FrButton::eButMode)3);
	m_pCloseBn->SetButtonImg("rare_shop_end_bn_normal", (FrButton::eButMode)0);
	m_pCloseBn->SetButtonImg("rare_shop_end_bn_over", (FrButton::eButMode)1);
	m_pCloseBn->SetButtonImg("rare_shop_end_bn_normal", (FrButton::eButMode)3);
	m_bFlagAee983 = false;
	m_pTotalItemBn->Enable(false);
	m_pCoinList983->SetVisible(false);
	m_pStartBn983->SetVisible(false);
	m_pAgainBn983->SetVisible(false);
	m_pGoCoinBn983->SetVisible(false);
	m_pAutoStartBn983->SetVisible(false);
	m_pNormalPlayBn983->SetVisible(false); // FUN_008b3e40(0)
	m_pComboPlayBn983->SetVisible(false);
}

// D879: os 3 botoes gravavam +0x104/+0x108 cru (m_pushDelay/m_pushedTime); FrWnd tem std::string/list e no cl 14.44 os
// membros vao a 0xf8/0xfc. Mesmas 2 gravacoes pelo nome (int 0, float 0.1f).
struct FrButtonPush983 : FrButton { using FrButton::m_pushDelay; using FrButton::m_pushedTime; };
C_ASSERT_VC10(sizeof(FrWnd) == 0xd0); // m_pushDelay 0x104 no VC10 (makeroomdlg.cpp FrButtonLayout983)

void FrBongdariShopDlg::OnAutoStartBnInit(int param)
{
	m_pAutoStartBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	*(int*)((char*)m_pAutoStartBn983 + offsetof(FrButtonPush983, m_pushDelay)) = 0;
	*(float*)((char*)m_pAutoStartBn983 + offsetof(FrButtonPush983, m_pushedTime)) = 0.1f;
	m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_normal", (FrButton::eButMode)0);
	m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_click", (FrButton::eButMode)1);
	m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_click", (FrButton::eButMode)2);
	m_pAutoStartBn983->SetVisible(false);
}

void FrBongdariShopDlg::SetAutoPlayImg983(int kind) // 008b4670
{
	if (kind == 0)
	{
		m_pAutoStartBn983->SetButtonImg("autoplay_on_bn_normal", (FrButton::eButMode)0);
		m_pAutoStartBn983->SetButtonImg("autoplay_on_bn_click", (FrButton::eButMode)1);
		m_pAutoStartBn983->SetButtonImg("autoplay_on_bn_click", (FrButton::eButMode)2);
	}
	else if (kind == 1)
	{
		m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_normal", (FrButton::eButMode)0);
		m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_click", (FrButton::eButMode)1);
		m_pAutoStartBn983->SetButtonImg("autoplay_off_bn_click", (FrButton::eButMode)2);
	}
	else if (kind == 2)
	{
		m_pAutoStartBn983->SetButtonImg("autoplay_stop_bn_normal", (FrButton::eButMode)0);
		m_pAutoStartBn983->SetButtonImg("autoplay_stop_bn_click", (FrButton::eButMode)1);
		m_pAutoStartBn983->SetButtonImg("autoplay_stop_bn_click", (FrButton::eButMode)2);
	}
}

void FrBongdariShopDlg::OnAutoStartBnUp()
{
	m_bAutoPlay983 = !m_bAutoPlay983;
	if (m_bAutoPlay983)
	{
		SetAutoPlayImg983(0);
		m_pBongdariModel->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f);
		m_pBackSelectBn983->Enable(false);
		return;
	}
	if (m_state != 0)
	{
		m_pAutoStartBn983->SetVisible(false);
		return;
	}
	SetAutoPlayImg983(1);
	m_pBackSelectBn983->Enable(true);
}

void FrBongdariShopDlg::OnNormalPlayBnInit(int param)
{
	m_pNormalPlayBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	*(int*)((char*)m_pNormalPlayBn983 + offsetof(FrButtonPush983, m_pushDelay)) = 0;
	*(float*)((char*)m_pNormalPlayBn983 + offsetof(FrButtonPush983, m_pushedTime)) = 0.1f;
	m_pNormalPlayBn983->SetVisible(false);
}

// 983 008b3670: texto de ajuda do modo (normal: quantas vezes por rodada; combo: itens obtidos)
void FrBongdariShopDlg::SetModeMessage983()
{
	if (m_comboMode983 == 1)
		SetMessage(K2L_Compatibility("\xc8\xb9\xb5\xe6 \xbe\xc6\xc0\xcc\xc5\xdb \xc1\xa4\xba\xb8\xb0\xa1 \xc7\xa5\xbd\xc3\xb5\xcb\xb4\xcf\xb4\xd9"), false);
	else
		SetMessage(MakeStr(K2L_Compatibility("%d\xc8\xb8 \xba\xd0\xb7\xae\xc0\xc7 \xb5\xb9\xc7\xc7\xb4\xcf \xba\xbc\xc0\xbb %d\xc6\xce\xc0\xb8\xb7\xce \xc7\xd1\xb9\xf8\xbf\xa1 \xbb\xcc\xbe\xc6\xc1\xdd\xb4\xcf\xb4\xd9."), 10, 10000), false);
}

void FrBongdariShopDlg::OnNormalPlayBnUp()
{
	m_comboMode983 = 0;
	m_pAutoStartBn983->SetVisible(true);
	m_pBongdariModel->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f);
	SetModeMessage983();
	SetButtonStatus983();
}

void FrBongdariShopDlg::OnComboPlayBnInit(int param)
{
	m_pComboPlayBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	*(int*)((char*)m_pComboPlayBn983 + offsetof(FrButtonPush983, m_pushDelay)) = 0;
	*(float*)((char*)m_pComboPlayBn983 + offsetof(FrButtonPush983, m_pushedTime)) = 0.1f;
	m_pComboPlayBn983->SetVisible(false);
}

void FrBongdariShopDlg::OnComboPlayBnUp()
{
	m_comboMode983 = 1;
	m_pAutoStartBn983->SetVisible(false);
	m_pBongdariModel->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f);
	m_pBongdariModel->SetMotion("\xbf\xac\xbc\xd3\xbb\xcc\xb1\xe2\xc1\xd8\xba\xf1", false, 0.0f);
	SetModeMessage983();
	SetButtonStatus983();
}

void FrBongdariShopDlg::OnCoinListInit(int param)
{
	m_pCoinList983 = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	*(unsigned long*)&((FrListBoxScroll*)m_pCoinList983)->m_dwStyle |= 0x200;
	m_pCoinList983->SetVisible(false);
}

void FrBongdariShopDlg::OnCoinListUp()
{
	FrListItem* sel = ((FrListBoxScroll*)m_pCoinList983)->m_pItemUnderCursor;
	if (!sel)
		return;
	sCoinItem983* data = (sCoinItem983*)sel->pData;
	if (data && data->qty != 0)
	{
		m_i0afc_983 = data->tid;
		m_coinCount983 = data->qty;
		m_coinItemName983 = data->icon;
		m_pCoinText983 = data->name;
		m_pStartBn983->Enable(true);
	}
}

void FrBongdariShopDlg::OnCoinListOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (!(m_memorialStep983 < 0 && m_shopMode983 == 2 && item))
		return;
	sCoinItem983* data = (sCoinItem983*)item->pData;
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!data || !gdi)
		return;
	float x = item->pos.x - 1.0f;
	float y = item->pos.y + 1.0f;
	WOverlay* ov = (item->idx < 2) ? m_pOvCoinSlot983[item->idx] : m_pOvCoinSlot983[2];
	DrawOverlay983(ov, x, y, 0xffffffff);
	if ((int)data->tid == m_i0afc_983)
	{
		WOverlay* sel = (item->idx < 2) ? m_pOvCoinSlot983[3 + item->idx] : m_pOvCoinSlot983[5];
		DrawOverlay983(sel, x, y, 0xffffffff);
	}
	const Bitmap* icon = data->icon ? g_pFresh->GetBitmap(data->icon) : NULL;
	if (icon)
	{
		WRect r(item->pos.x + 5.0f, item->pos.y + 3.0f, 50.0f, 50.0f); // 008b8150: 50x50 fixo (_DAT_00cff0a8), nao coin_icon_rect_w/h
		gdi->DrawTexture(icon, r, 0xffffffff, 0);
	}
	gdi->SetTextStyle(1);
	WPoint p1(item->pos.x + 45.0f, m_cfgAddListTextY983 + item->pos.y);
	g_pFresh->GetManager()->PrintText(p1, 0, data->name, -1.0f, 0xffffffff);
	WPoint p2(m_cfgAddCoinNumX983 + item->pos.x, m_cfgAddListTextY983 + item->pos.y);
	g_pFresh->GetManager()->PrintText(p2, 0, MakeStr("x%d", data->qty), -1.0f, 0xffffffff);
	gdi->SetTextStyle(0);
}

void FrBongdariShopDlg::OnStartBnInit(int param)
{
	m_pStartBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pStartBn983->SetVisible(false);
}

void FrBongdariShopDlg::OnStartBnUp()
{
	if (m_i0afc_983 != 0 && m_coinCount983 != 0)
	{
		{
			WSendPacket packet((enumClientPacket)0x17f);
			packet.Encode4(m_i0afc_983);
			packet.Send((eSendTo)0);
		}
		ClearCoinListData983();
		m_pCoinList983->ClearItem();
		m_pCoinList983->SetVisible(false);
		m_pStartBn983->SetVisible(false);
		m_pCloseBn->Enable(false);
		m_pBackSelectBn983->Enable(false);
		m_pTotalItemBn->Enable(false);
		m_pAboutBn983->Enable(false);
		m_pNoticeBn983->Enable(false);
		--m_coinCount983;
		m_effectAlpha983 = 0.0f;
		if (CCameraWork::Instance())
			CCameraWork::Instance()->LoadCameraMotion983(5, "\xbd\xc7\xc7\xe0");
		g_audio->PlaySfx("\xb9\xf6\xc6\xb0\xc5\xac\xb8\xaf", NULL, 0, NULL, NULL, 0.5f, 200.0f);
		m_memorialStep983 = 0;
	}
}

void FrBongdariShopDlg::OnSelectBongBnInit(int param)
{
	m_pSelectBongBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdariShopDlg::OnSelectBongBnUp()
{
	// 983 008bbf50: servico 0x10 suspenso -> aviso (K2L) em vez de entrar
	if (DOCJP()->IsControlServerService983(0x10) != 0)
	{
		const char* msg = S5::UTIL::IsEventServerControl((eGameServerProperty)0x100)
			? K2L_Compatibility("\xb4\xeb\xc8\xb8 \xbc\xad\xb9\xf6\xbf\xa1\xbc\xad\xb4\xc2 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb1\xe2\xb4\xc9\xc0\xd4\xb4\xcf\xb4\xd9.")
			: K2L_Compatibility("\xba\xc0\xb4\xd9\xb8\xae\xbc\xa5 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9.");
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23), (int)msg, 0, 0, 0, 0);
		return;
	}
	m_shopMode983 = 1;
	m_pSelectBongBn983->SetVisible(false);
	m_pSelectMemBn983->SetVisible(false);
	m_pAutoStartBn983->SetVisible(true);
	m_pNormalPlayBn983->SetVisible(true); // 008b3e40(1)
	m_pComboPlayBn983->SetVisible(false);
	m_pAboutBn983->SetStatus((FrButton::eButMode)0);
	m_pAboutBn983->Enable(true);
	m_pCloseBn->Enable(true);
	m_pStartBn983->SetVisible(false);
	m_pNoticeBn983->Enable(true);
	m_pTotalItemBn->Enable(true);
	m_pBackSelectBn983->Enable(true);
	m_pAboutBn983->SetButtonImg("bongdari_shop_aboutshop_bn_normal", (FrButton::eButMode)0);
	m_pAboutBn983->SetButtonImg("bongdari_shop_aboutshop_bn_over", (FrButton::eButMode)1);
	m_pAboutBn983->SetButtonImg("bongdari_shop_aboutshop_bn_over", (FrButton::eButMode)3);
	m_pCloseBn->SetButtonImg("bongdari_shop_end_bn_normal", (FrButton::eButMode)0);
	m_pCloseBn->SetButtonImg("bongdari_shop_end_bn_over", (FrButton::eButMode)1);
	m_pCloseBn->SetButtonImg("bongdari_shop_end_bn_over", (FrButton::eButMode)3);
}

void FrBongdariShopDlg::OnSelectMemBnInit(int param)
{
	m_pSelectMemBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdariShopDlg::OnSelectMemBnUp()
{
	// 983 008bc0f0: servico 0x10000000 suspenso -> aviso GetGameString(0xd6d9)
	if (DOCJP()->IsControlServerService983(0x10000000) != 0)
	{
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
			(int)GetGameString(0xd6d9), 0, 0, 0, 0);
		return;
	}
	m_shopMode983 = 2;
	m_pSelectBongBn983->SetVisible(false);
	m_pSelectMemBn983->SetVisible(false);
	m_pCoinList983->SetVisible(true);
	m_pCoinList983->Enable(true);
	m_pStartBn983->SetVisible(true);
	m_pStartBn983->Enable(false);
	m_pNoticeBn983->Enable(false);
	m_pAboutBn983->SetStatus((FrButton::eButMode)0);
	m_pAboutBn983->Enable(true);
	m_pCloseBn->Enable(true);
	m_pBackSelectBn983->Enable(true);
	m_i0afc_983 = 0;
	m_coinCount983 = 0;
	m_coinItemName983 = NULL;
	m_pCoinText983 = NULL;
	m_pAboutBn983->SetButtonImg("memorial_shop_aboutshop_bn_normal", (FrButton::eButMode)0);
	m_pAboutBn983->SetButtonImg("memorial_shop_aboutshop_bn_over", (FrButton::eButMode)1);
	m_pAboutBn983->SetButtonImg("memorial_shop_aboutshop_bn_over", (FrButton::eButMode)3);
	m_pCloseBn->SetButtonImg("memorial_shop_end_bn_normal", (FrButton::eButMode)0);
	m_pCloseBn->SetButtonImg("memorial_shop_end_bn_over", (FrButton::eButMode)1);
	m_pCloseBn->SetButtonImg("memorial_shop_end_bn_over", (FrButton::eButMode)3);
}

void FrBongdariShopDlg::OnAgainBnInit(int param)
{
	m_pAgainBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pAgainBn983->SetVisible(false);
}

void FrBongdariShopDlg::OnAgainBnUp()
{
	m_pAgainBn983->SetVisible(false);
	m_pGoCoinBn983->SetVisible(false);
	m_f524_983 = 3.3f;
	m_f51c_983 = 0.0f;
	m_bFlagAee983 = false;
	m_effectAlpha983 = 0.0f;
	m_bFxBoxActive983 = 0;
	m_bResultReady983 = false;
	m_fxBox983.ClearAll(false);
	OnStartBnUp();
}

void FrBongdariShopDlg::OnGoCoinBnInit(int param)
{
	m_pGoCoinBn983 = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pGoCoinBn983->SetVisible(false);
}

void FrBongdariShopDlg::OnGoCoinBnUp()
{
	m_bFlagAee983 = false;
	ResetCoinSelect983();
}

void FrBongdariShopDlg::ClearCoinListData983() // 008bdfb0
{
	if (!m_pCoinList983)
		return;
	for (FrListBox::ITEM_LIST::iterator it = m_pCoinList983->m_itemList.begin();
		it != m_pCoinList983->m_itemList.end(); ++it)
	{
		if ((*it)->pData)
			operator delete((*it)->pData);
	}
}

void FrBongdariShopDlg::FillCoinList983() // 008bdd20
{
	// D885: Doc()+0x6a0/0x6a4 = m_itemManager (0x20c) + m_MemorialShopCoinItem983 (0x494) no VC10; no cl 14.44 o CSharedDoc anda
	// -> membro (no VC10 fica o cru: os acessores do vector sairiam call)
#if _MSC_VER > 1600
	std::vector<IFF983::sMemorialShopCoinItem>& coins = ItemManager()->m_MemorialShopCoinItem983;
	char* begin = coins.empty() ? NULL : (char*)&coins[0];
	unsigned int count = (unsigned int)coins.size();
#else
	C_ASSERT_VC10(offsetof(CSharedDoc, m_itemManager) + offsetof(CItemManager, m_MemorialShopCoinItem983) == 0x6a0);
	char* doc = (char*)Doc();
	char* begin = *(char**)(doc + 0x6a0);
	char* end = *(char**)(doc + 0x6a4);
	unsigned int count = (unsigned int)(end - begin) >> 6;
#endif
	for (unsigned int idx = 0; idx < count; ++idx)
	{
		unsigned long tid = *(unsigned long*)(begin + idx * 0x40 + 4);
		std::list<sItemInfo>::iterator it;
		for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end(); ++it)
			if ((*it).tid == tid)
				break;
		if (it == Doc()->m_myItemList.end())
		{
			if (idx < 2)
			{
				char* iff = (char*)ItemManager()->FindItem(tid);
				if (iff)
				{
					sCoinItem983* d = new sCoinItem983;
					d->name = iff + 8;
					d->tid = *(unsigned long*)(iff + 4);
					d->qty = 0;
					d->icon = iff + 0x49;
					m_pCoinList983->AddItem(d);
				}
			}
		}
		else if ((*it).Common[0] > 0)
		{
			char* iff = (char*)ItemManager()->FindItem(tid);
			if (iff)
			{
				sCoinItem983* d = new sCoinItem983;
				d->name = iff + 8;
				d->tid = *(unsigned long*)(iff + 4);
				d->qty = (*it).Common[0];
				d->icon = iff + 0x49;
				m_pCoinList983->AddItem(d);
			}
		}
	}
}

void FrBongdariShopDlg::ResetCoinSelect983() // 008bde90
{
	FillCoinList983();
	m_pCoinList983->SetVisible(true);
	m_pStartBn983->SetVisible(true);
	m_pStartBn983->Enable(false);
	m_pCloseBn->Enable(true);
	m_pTotalItemBn->Enable(true);
	m_pAboutBn983->Enable(true);
	m_pNoticeBn983->Enable(false);
	m_pAgainBn983->SetVisible(false);
	m_pGoCoinBn983->SetVisible(false);
	m_pBackSelectBn983->Enable(true);
	m_fxBox983.ClearAll(false);
	m_f524_983 = 3.3f;
	m_memorialStep983 = -1;
	m_f51c_983 = 0.0f;
	m_bFxBoxActive983 = 0;
	m_bResultReady983 = false;
	m_effectAlpha983 = 0.0f;
	m_resultCount983 = 0; // D821: 983 008bde90 zera +0x588, nao os pontos +0x584 (Again/Back mostravam 0 e LV 0)
	m_i0afc_983 = 0;
	m_coinCount983 = 0;
	memset(m_resultBall983, 0, sizeof(m_resultBall983));
	if (CCameraWork::Instance())
		CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)5, "\xb1\xe2\xba\xbb", WVector(0.0f, 0.0f, 0.0f));
}


static float s_lobbyFov983; // E230: o Process da CCameraWork poe o FOV da camera do modo 5 (mundo do lobby ampliado)
static WMatrix s_lobbyCam983; // E227: camera do lobby ao abrir; o Memorial nao a move (orig-c157-2/017)

void FrBongdariShopDlg::ProcessMemorial983(float deltaTime) // parte do modo 2 de 008c0dd0
{
	if (m_pGacha983)
		m_pGacha983->ProcessBongdariShop(deltaTime);
	if (CCameraWork::Instance())
	{
		WMatrix keepCam = s_lobbyCam983; // E227: nao g_camera (LoadCamera do ResetCoinSelect ja a moveu)
		CCameraWork::Instance()->Process(deltaTime, 0.0f);
		// E240: a camera do modo 5 (memorial_cam.apet) fica em g_camera logo apos o Process; o 983 le o gacha em CCameraWork+0x120
		// (008b9520/008b7da0), nunca escrito no lobby (e238-1: gacha fora do quadro).
		// E395: o Process (modo 5, m_i2ac != 0) grava a camera do memorial em +0x120 (m_matSelect983, 006d6da4); copiar g_camera
		// por cima punha a camera do lobby (piv 130,72,-891) e o E388 caia na camera propria (-12.8 em vez de ~-76). So copia com m_i2ac == 0
		// (o Process poe a camera so em g_camera).
		if (*(int*)((char*)CCameraWork::Instance() + VC10_CAMWORK_I2AC) == 0 && // m_i2ac (D873: cl 14.44 0x2d0)
			fabsf(g_camera.xx) + fabsf(g_camera.yy) + fabsf(g_camera.zz) > 0.5f)
			memcpy((char*)CCameraWork::Instance() + VC10_CAMWORK_MATSELECT, &g_camera, 12 * sizeof(float));
		g_camera = keepCam;
		g_view->SetFOV(s_lobbyFov983);
	}
	if (m_memorialStep983 == 0)
	{
		m_f51c_983 += deltaTime;
		if (m_f51c_983 > 1.5f && CCameraWork::Instance())
		{
			CCameraWork::Instance()->LoadCameraMotion983(5, "\xbd\xc7\xc7\xe0\xb3\xa1");
			memcpy(&m_scaleTab983[12], (char*)CCameraWork::Instance() + VC10_CAMWORK_MATSELECT, 12 * sizeof(float)); // D879: era 0x120 cru (cl 14.44 0x118)
			m_f51c_983 = 0.0f;
			m_memorialStep983 = 1;
		}
	}
	else if (m_memorialStep983 == 1)
	{
		if (m_bResultReady983)
			m_memorialStep983 = 2;
	}
	else if (m_memorialStep983 == 2) // 008bbf30: 008b95a0 (movimento + som por resultado) + 008b9810 (logo fx) -> 3
	{
		if (m_pGacha983) // 008b95a0: 3..4 rara; 0..2 normal1..3 (rand 10/70); -1 falha1/2 (rand 20)
		{
			const char* motion = NULL;
			const char* sfx = NULL;
			if (m_result983 >= 3 && m_result983 <= 4)
			{
				m_f524_983 += 4.0f;
				motion = "\xb7\xb9\xbe\xee"; // rare
				sfx = "memorial_rare";
			}
			else if (m_result983 >= 0 && m_result983 < 3)
			{
				float r = (float)(rand() % 100 + 1);
				if (r <= 10.0f)
				{
					m_f524_983 += 0.8f;
					motion = "\xb3\xeb\xb8\xbb" "1";
					sfx = "memorial_normal1";
				}
				else if (r <= 70.0f)
				{
					m_f524_983 += 0.8f;
					motion = "\xb3\xeb\xb8\xbb" "2";
					sfx = "memorial_normal2";
				}
				else
				{
					m_f524_983 += 1.22f;
					motion = "\xb3\xeb\xb8\xbb" "3";
					sfx = "memorial_normal3";
				}
			}
			else if (m_result983 == -1)
			{
				float r = (float)(rand() % 100 + 1);
				if (r <= 20.0f)
				{
					m_f524_983 += 0.72f;
					motion = "\xbd\xc7\xc6\xd0" "1";
					sfx = "memorial_fail1";
				}
				else
				{
					m_f524_983 += 1.0f;
					motion = "\xbd\xc7\xc6\xd0" "2";
					sfx = "memorial_fail2";
				}
			}
			if (motion)
			{
				m_pGacha983->SetMotion(motion, false, 0.0f);
				g_audio->PlaySfx(sfx, NULL, 0, NULL, NULL, 0.5f, 200.0f);
			}
		}
		// 008b9810: logo fx (grandprix_logoeff.seq) no centro do proprio rect (+0x34), 3 floats em +0x110c = vetor nulo (f041c4)
		if (CFxSequence* seq = m_fxBox983.LoadSequence("grandprix_logoeff.seq", true, NULL))
		{
			const WRect& rc = GetRect();
			seq->m_attachX = (float)(int)(rc.x + 0.5f * rc.w); // +0x18
			seq->m_attachY = (float)(int)(rc.y + 0.5f * rc.h); // +0x1c
			seq->m_pos = g_vZero983_f041c4; // +0x110c (E406: era visao crua VC10_FXSEQ_POS)
			m_fxBox983.AddSequence(seq);
		}
		m_memorialStep983 = 3;
	}
	else if (m_memorialStep983 == 3) // 008b4880: espera m_f524 e faz o alpha subir ate 255 -> 4
	{
		m_f51c_983 += deltaTime;
		if (m_f524_983 < m_f51c_983)
		{
			m_bFxBoxActive983 = 1;
			if (m_effectAlpha983 < 255.0f)
				m_effectAlpha983 = m_cfgEffectAlpha983 + m_effectAlpha983;
			if (m_effectAlpha983 > 255.0f)
			{
				m_memorialStep983 = 4;
				m_effectAlpha983 = 255.0f;
				m_f51c_983 = 0.0f;
			}
		}
	}	else if (m_memorialStep983 == 4) // 008bd940: espera m_cfgShowResultDlg983; abre o dialogo do item raro (008bcf90) e volta o gacha ao repouso -> 5
	{
		m_f51c_983 += deltaTime;
		if (m_f51c_983 > m_cfgShowResultDlg983)
		{
			m_f51c_983 = 0.0f;
			// 008bcf90: +0x580 == -1 (falha ou mais de 1 item) usa 008bbc10; preenche antes do Open
			FrBongdarishopRareItemDlg* dlg = CreateForm<FrBongdarishopRareItemDlg>(
				g_pFresh->GetManager(), this, "bongdarishop_rareitem", NULL);
			m_pRareItemDlg983 = dlg;
			if (dlg)
			{
				if (m_result983 == -1)
					dlg->SetRareItems983(m_resultBall983, m_resultCount983);
				else
					dlg->SetRareItem(&m_resultBall983[0]);
				dlg->Open((FRESH_PFN_RESULT)&FrBongdariShopDlg::OnBongdarishopRareItemDlgResult, 3);
			}
			SetOpeningBonusBallInfo983(m_resultBall983, m_resultCount983); // 008bd987
			if (m_pGacha983)
				m_pGacha983->SetMotion("\xb1\xe2\xba\xbb", false, 0.0f); // 983 passa 0 (movimento padrao); "basico" como repouso
			m_memorialStep983 = 5;
		}
	}
	// 008c0dd0 (depois do switch): +0x590 liga o processamento do fx box (CFxBox::Finished 0096d7f0 desliga), tempo em ms
	if (!m_bFxBoxActive983)
		return;
	if (*(int*)((char*)&m_fxBox983 + VC10_FXBOX_SEQN) == 0 && *(int*)((char*)&m_fxBox983 + VC10_FXBOX_SPRAYN) == 0)
	{
		m_bFxBoxActive983 = 0;
		return;
	}
	m_fxBox983.ProcessAttacher(deltaTime * 1000.0f);
	m_fxBox983.Process(deltaTime * 1000.0f); // 00971b60
}

bool FrBongdariShopDlg::OnInit()
{
	s_lobbyCam983 = g_camera;
	s_lobbyFov983 = g_view->GetFOV();
	// 983 008be2e0: modo 0 (escolha Bongdari/Memorial), memorial.lua, overlays, gacha 3D, pacote 0x157
	m_shopMode983 = 0;
	LoadMemorialConfig983();
	LoadMemorialOverlays983();
	SetHitRects983();
	CreateCoinFont983();
	CreateGacha983();
	FillCoinList983(); // 008be334: lista de moedas do Memorial ja na abertura
	{
		WSendPacket packet((enumClientPacket)0x157);
		packet.Encode4(Doc()->m_myInfo.info.dwUID);
		packet.Send((eSendTo)0);
	}
	if (CCameraWork::Instance())
		CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)5, "\xb1\xe2\xba\xbb", WVector(0.0f, 0.0f, 0.0f));
	EnableDrag(false);
	ShowBongdari();
	InitCouponBongdariShop();
	m_cursorArea.x = m_rect.x + m_cursorArea.x + m_cfgStartImgX983;
	m_cursorArea.y = m_rect.y + m_cursorArea.y + m_cfgStartImgY983;
	SetButtonStatus983();
	m_pTotalItemBn->Enable(false);
	return FrForm::OnInit();
}

void FrBongdariShopDlg::OnProc(const float deltaTime)
{
	static int i;

	FrForm::OnProc(deltaTime); // 983 008c0dd0: 1a chamada, antes do modo

	// 983 008c0dd0: modo 2 retorna apos o Memorial e modo 0 nao processa nada; sem o retorno o timer de 4s religava o
	// fechar no meio do sorteio do Memorial
	if (m_shopMode983 == 2)
	{
		ProcessMemorial983(deltaTime);
		return;
	}
	if (m_shopMode983 != 1)
		return;

	if (m_pBongdariModel)
	{
		int result = m_pBongdariModel->ProcessBongdariShop(deltaTime);
		if (result == 1)
			m_bAnimating = false;
		else if (result == 2)
			m_state = 6;
	}
	Processing(deltaTime);
	for (i = 0; i < m_bonusBallCount; i++)
	{
		if (m_pBonusBallModel[i])
			m_pBonusBallModel[i]->ProcessBongdariShop(deltaTime);
	}
	ControlNoticeDlg();

	if (!m_bChildDlgOpen)
	{
		if (m_exitEnableTime < 4.0f)
		{
			m_exitEnableTime += deltaTime;
			if (m_exitEnableTime >= 4.0f && !m_pCloseBn->IsEnabled())
				m_pCloseBn->Enable(true);
		}
		WVector mouse = g_input->GetMousePoint();
		if (m_cursorArea.IsInRect(mouse.x, mouse.y))
		{
			m_cursor = 0;
			if (m_state == 0)
			{
				if (!m_bUsingBonusBall)
				{
					const WRect& wnd = m_pBongdari->GetRect(); // 983 008c116b: mesma area 130x130 do OnLButtonUp
					WRect rect(wnd.x + (m_cfgStartImgX983 - 20.0f), wnd.y + (m_cfgStartImgY983 - 20.0f), 130.0f, 130.0f);
					if (rect.IsInRect(g_input->GetMousePoint().x,
							g_input->GetMousePoint().y))
					{
						m_cursor = 1;
						if (m_bBongdariPressed)
							m_cursor = 2;
					}
				}
				SetPosMovingClickText(deltaTime);
			}
			else if (m_state == 4)
			{
				m_hoverBonusBall = -1;
				for (i = 0; i < m_bonusBallCount; i++)
				{
					if (m_pBonusBall[i] && m_pBonusBall[i]->IsVisible() &&
						m_moving[i].state == 2)
					{
						WRect rect = m_pBonusBall[i]->GetRect();
						if (rect.IsInRect(g_input->GetMousePoint().x,
								g_input->GetMousePoint().y))
						{
							m_cursor = 1;
							m_hoverBonusBall = i;
							break;
						}
					}
				}
				SetPosMovingClickText(deltaTime);
			}
			SetCursor(m_cursor); // 983 008c1232: em todos os estados, inclusive 0 e 4 (mao sobre a maquina e as bolas)
		}
	}

	m_feeTextAlpha = (int)((float)m_feeTextAlpha - deltaTime * 480.0f);
	m_feeTextY -= deltaTime * 20.0f;
	m_discountTextAlpha =
		(int)((float)m_discountTextAlpha - deltaTime * 480.0f);
	m_discountTextY -= deltaTime * 20.0f;
}

bool FrBongdariShopDlg::OnLButtonDown(const WPoint& pt)
{
	FrForm::OnLButtonDown(pt);
	// 983 008bc810: no modo de escolha (0) os retangulos de hit abrem Papel/Memorial
	if (m_shopMode983 == 0 && !m_bFlagAed983)
	{
		if (m_rcHit1983.IsInRect(pt))
			OnSelectBongBnUp();
		else if (m_rcHit2983.IsInRect(pt))
			OnSelectMemBnUp();
	}
	if (m_state == 0)
	{
		m_bBongdariPressed = false;
		if (m_shopMode983 == 1 && const_cast<WRect&>(m_pBongdari->GetRect()).IsInRect(pt))
			m_bBongdariPressed = true;
	}
	else if (m_state == 4)
	{
		for (int i = 0; i < m_bonusBallCount; i++)
		{
			if (m_pBonusBall[i] && m_pBonusBall[i]->IsVisible() &&
				m_moving[i].state == 2)
			{
				WRect rect = m_pBonusBall[i]->GetRect();
				if (rect.IsInRect(pt))
				{
					if (m_pTotalItemBn)
						m_pTotalItemBn->EnableBlink(true);
					OpenBonusBall(i);
					g_audio->PlaySfx(STR_SFX_CLICK);
					return false;
				}
			}
		}
	}
	return false;
}

bool FrBongdariShopDlg::OnLButtonUp(const WPoint& pt)
{
	FrForm::OnLButtonUp(pt);
	// 983 008bc9e0: pacote por modo (0x14B normal, 0x186 combo), com teste de pang/cupom
	if (m_pBongdari && m_state == 0 && !m_bUsingBonusBall && m_bBongdariPressed)
	{
		// 983 008bca4b: so a area 130x130 da imagem START (+0x54c/+0x550, -20) sorteia, nao a janela inteira
		const WRect& wnd = m_pBongdari->GetRect();
		WRect rect(wnd.x + (m_cfgStartImgX983 - 20.0f), wnd.y + (m_cfgStartImgY983 - 20.0f), 130.0f, 130.0f);
		if (rect.IsInRect(pt))
		{
			const __int64 pang = Doc()->m_myInfo.stat.i64Pang;
			bool ok = false;
			int noPangMsg = 0; // 983 [ebp+8]: texto do aviso por modo
			if (m_comboMode983 == 0)
			{
				noPangMsg = 0xc351;
				ok = pang >= 900 || m_couponCount > 0;
			}
			else if (m_comboMode983 == 1)
			{
				noPangMsg = 0xc355;
				ok = pang >= 10000;
			}
			if (ok)
			{
				if (!Doc()->m_myInfo.stat.Level && m_shopMode983 == 1)
				{
					OpenBongdarishopWarningDlg(0, 0);
				}
				else
				{
					m_bUsingBonusBall = true;
					WSendPacket packet((enumClientPacket)(m_comboMode983 == 1 ? 0x186 : 0x14b));
					packet.Send((eSendTo)0);
					g_audio->PlaySfx(STR_SFX_CLICK);
					EnableExitButton(false);
					m_pBackSelectBn983->Enable(false); // 983 008bc9e0: +0x130 Enable(0)
					m_exitEnableTime = 0.0f;
					if (m_pAutoStartBn983 && m_bAutoPlay983 && m_comboMode983 == 0)
					{
						m_i434_983++;
						SetAutoPlayImg983(2);
					}
					else if (m_pAutoStartBn983)
						m_pAutoStartBn983->SetVisible(false);
					m_pNormalPlayBn983->SetVisible(false); // FUN_008b3e40(0)
					m_pComboPlayBn983->SetVisible(false);
				}
			}
			else
			{
				// 983 008bcafb: pang insuficiente -> janela "notify" (texto 0xc351 normal / 0xc355 combo)
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify", NULL);
				pForm->SetMessage(GetGameString(noPangMsg), false);
				// 983 008bcb2e: por cima e com o foco antes do Open (D679)
				pForm->SetTopmost(true);
				pForm->FindNextTopFocus(false);
				pForm->Open(NULL, 3);
			}
		}
	}
	m_bBongdariPressed = false;
	return false;
}

void FrBongdariShopDlg::OnCancel()
{
}

void FrBongdariShopDlg::OpenAboutShopDlg()
{
	FrAboutShopDlg* dlg = CreateForm<FrAboutShopDlg>(g_pFresh->GetManager(),
		this, "about_shop", NULL);
	if (dlg)
	{
		dlg->SetMode983(m_shopMode983);
		dlg->SetIndexCaptions983();
		dlg->Open((FRESH_PFN_RESULT)&FrBongdariShopDlg::OnAboutShopDlgResult,
			3);
		m_bChildDlgOpen = true;
	}
}

void FrBongdariShopDlg::OpenTotalItemDlg()
{
	// 983: botao "totalitem" = jmp 008bf5e0, lista por modo
	RefreshTotalItem983(m_shopMode983 == 2 ? &m_listResult983 : &m_listTotalItem);
}

void FrBongdariShopDlg::OpenBongdarishopNoticeDlg(bool bFirst)
{
	FrBongdarishopNoticeDlg* dlg = CreateForm<FrBongdarishopNoticeDlg>(
		g_pFresh->GetManager(), this, "bongdarishop_notice", NULL);
	if (dlg)
	{
		dlg->Open(
			(FRESH_PFN_RESULT)&FrBongdariShopDlg::OnBongdarishopNoticeDlgResult,
			0xc0);
		dlg->SetButtonType(bFirst);
		dlg->SetFlag(bFirst ? 0xc1 : 3);
		m_bChildDlgOpen = true;
	}
}

void FrBongdariShopDlg::OpenBongdarishopRareItemDlg(int index)
{
	FrBongdarishopRareItemDlg* dlg = CreateForm<FrBongdarishopRareItemDlg>(
		g_pFresh->GetManager(), this, "bongdarishop_rareitem", NULL);
	if (dlg)
	{
		// 983 008bcc80: resultado proprio 008b3540 (o de 008bc2a0 e so do Memorial) e item raro desliga o auto-play
		dlg->Open((FRESH_PFN_RESULT)&FrBongdariShopDlg::OnBongdariRareDlgResult983,
			3);
		if (index >= 0 && index < m_bonusBallCount)
		{
			dlg->SetRareItem(&m_bonusBall[index]);
			m_bChildDlgOpen = true;
		}
		m_bAutoPlay983 = false;
		SetAutoPlayImg983(1);
	}
}

void FrBongdariShopDlg::OpenBongdarishopWarningDlg(int type, int count)
{
	FrBongdarishopWarningDlg* dlg = CreateForm<FrBongdarishopWarningDlg>(
		g_pFresh->GetManager(), this, "bongdarishop_warning", NULL);
	if (dlg)
	{
		dlg->Open((FRESH_PFN_RESULT)&FrBongdariShopDlg::
					  OnBongdarishopWarningDlgResult,
			0x40);
		dlg->SetImgWarningBack(type, count);
		m_bChildDlgOpen = true;
	}
}

void FrBongdariShopDlg::OpenClosedBonusBallResultDlg()
{
	if (AfxGetTask())
	{
		FrClosedBonusBallResultDlg* dlg =
			CreateForm<FrClosedBonusBallResultDlg>(g_pFresh->GetManager(), NULL,
				"closed_bonusball", NULL);
		if (dlg)
		{
			dlg->Open(NULL, 3);
			dlg->SetClosedBonusBall(m_bonusBall);
		}
	}
}

bool FrBongdariShopDlg::OnAboutShopDlgResult(int result, FrForm* pForm)
{
	m_bChildDlgOpen = false;
	return true;
}

bool FrBongdariShopDlg::OnBongdarishopNoticeDlgResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		FrWnd* dlgWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"bongdarishop_notice");
		FrBongdarishopNoticeDlg* dlg =
			STATIC_CAST(FrBongdarishopNoticeDlg, dlgWnd);
		if (dlg)
		{
			if (dlg->GetMoveLayoutTid())
				OpenAboutShopDlg();
			if (0)
				SetUsingBonusBall(false);
		}
	}
	m_bChildDlgOpen = false;
	return true;
}

bool FrBongdariShopDlg::OnBongdarishopRareItemDlgResult(int result,
	FrForm* pForm)
{
	// 983 008bc2a0 (resultado do dialogo do item raro, aberto por 008bcf90): 1/2 = fechou -> libera "again"/"go coin"
	// e repede as conquistas (0x157); depois, com Doc +0x11030 == 2, desabilita os dois e avisa (limite de 20.000 itens)
	m_bChildDlgOpen = false;
	if (result == 1 || result == 2)
	{
		m_bFlagAee983 = true;
		m_pAgainBn983->SetVisible(true);
		m_pAgainBn983->Enable(m_coinCount983 != 0);
		m_pGoCoinBn983->SetVisible(true);
		m_pCloseBn->Enable(true); // 983 +0x26c (o mesmo botao do timer de saida em OnProc)
		WSendPacket packet((enumClientPacket)0x157);
		packet.Encode4(Doc()->m_myInfo.info.dwUID);
		packet.Send((eSendTo)0);
	}
	m_pRareItemDlg983 = NULL;
	if ((int)Doc()->m_specialShotMark == 2)
	{
		m_pAgainBn983->Enable(false);
		m_pGoCoinBn983->Enable(false);
		const char* msg = K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xd1 \xc1\xbe\xb7\xf9\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb \xba\xb8\xc0\xaf \xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xc7\xd8\xb4\xe7 \xc4\xc1\xc5\xd9\xc3\xf7\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23), (int)msg, 1, 0, 0, 0);
	}
	return true;
}

bool FrBongdariShopDlg::OnBongdariRareDlgResult983(int result, FrForm* pForm) // 008b3540
{
	m_bChildDlgOpen = false;
	return true;
}

bool FrBongdariShopDlg::OnBongdarishopWarningDlgResult(int result,
	FrForm* pForm)
{
	m_bChildDlgOpen = false;
	return true;
}

void FrBongdariShopDlg::ControlNoticeDlg()
{
	FrWnd* dlgWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
		"bongdarishop_notice");
	FrBongdarishopNoticeDlg* dlg = STATIC_CAST(FrBongdarishopNoticeDlg, dlgWnd);
	if (!dlg)
		return;
	if (g_input->GetDown("\xc1\xc2", false))
		dlg->PrevIndex();
	else if (g_input->GetDown("\xbf\xec", false))
		dlg->NextIndex();
}

void FrBongdariShopDlg::InitBongdariShop()
{
	for (int i = 0; i < 9; i++)
	{
		m_silhouette[i].bonusBallIndex = -1;
		m_silhouette[i].pBitmap = m_pDefaultSilhouette;
		m_silhouette[i].typeId = 0;
		m_silhouette[i].grade = 0;
		m_silhouette[i].state = 0;
		m_silhouette[i].emotiTime = 10.0f;
	}
	memset(m_bonusBall, 0, sizeof(m_bonusBall));
	m_bonusBallCount = 0;
	DeletePetBonusBall();
	memset(m_moving, 0, sizeof(m_moving));
	m_listSecond983.clear();
}

void FrBongdariShopDlg::SetBongdariShop()
{
	// 983 008bcda0
	if (m_bonusBallCount <= 5)
	{
		DeletePetBonusBall();
		SetRandomDataItemSilhouette();
		m_float42c983 = 0.0f;
		m_state = 1;
	}
}

void FrBongdariShopDlg::SetBonusBall(int count, sBonusBall* pBonusBall)
{
	if (pBonusBall != NULL && count <= 5)
	{
		memcpy(m_bonusBall, pBonusBall, count * sizeof(sBonusBall));
		m_bonusBallCount = count;
	}
}

void FrBongdariShopDlg::OpenBonusBall(int index)
{
	if (index < 0 || index >= m_bonusBallCount)
		return;

	int i;
	for (i = 0; i < 9; i++)
	{
		if (m_silhouette[i].bonusBallIndex == index)
			break;
	}
	if (i >= 9)
		return;

	sMovingData& moving = m_moving[index];
	moving.state = 3;
	moving.target.x = (float)(m_silhouette[i].pBitmap->Width() >> 1) +
		g_arrPtItemSilhouette[i].x + m_rect.x - 7.0f;
	moving.target.y = (float)(m_silhouette[i].pBitmap->Height() >> 1) +
		g_arrPtItemSilhouette[i].y + m_rect.y - 15.0f;
	moving.target.z = 0.0f;
	moving.velocity.x = moving.target.x - moving.pos.x;
	moving.velocity.y = moving.target.y - moving.pos.y;
	moving.velocity.z = 0.0f;
}

void FrBongdariShopDlg::Clear()
{
	SendStateBongdariShop(false);
	if (m_pEmoti)
	{
		delete m_pEmoti;
		m_pEmoti = NULL;
	}
	if (m_pFontBong)
		m_pFontBong->Erase();
	if (m_pFontWind)
		m_pFontWind->Erase();
	if (m_pFontBongRed)
		m_pFontBongRed->Erase();
	if (g_resrcmng && m_pFontBongRed)
	{
		g_resrcmng->Release(m_pFontBongRed);
		m_pFontBongRed = NULL;
	}
	if (g_resrcmng && m_pFontBong)
	{
		g_resrcmng->Release(m_pFontBong);
		m_pFontBong = NULL;
	}
	if (g_resrcmng && m_pFontWind)
	{
		g_resrcmng->Release(m_pFontWind);
		m_pFontWind = NULL;
	}
	if (m_pBongdariModel)
	{
		delete m_pBongdariModel;
		m_pBongdariModel = NULL;
	}
	DeletePetBonusBall();

	FrWnd* aboutWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("about_shop");
	FrForm* about = STATIC_CAST(FrForm, aboutWnd);
	if (about)
		about->Close(FrCANCEL, true);
	FrWnd* totalWnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("total_item");
	FrForm* total = STATIC_CAST(FrForm, totalWnd);
	if (total)
		total->Close(FrOK, true);
	FrWnd* noticeWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
		"bongdarishop_notice");
	FrForm* notice = STATIC_CAST(FrForm, noticeWnd);
	if (notice)
		notice->Close(FrOK, true);
	FrWnd* rareWnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
		"bongdarishop_rareitem");
	FrForm* rare = STATIC_CAST(FrForm, rareWnd);
	if (rare)
		rare->Close(FrCANCEL, true);

	if (AfxGetTask())
	{
		for (int i = 0; i < m_bonusBallCount; i++)
			UpdateListMyItem(&m_bonusBall[i]);
	}
}

void FrBongdariShopDlg::CreateBonusBall(WVector pos)
{
	static int i;

	WMatrix oldCamera = g_view->GetCamera();
	WVector2D oldCenter = g_view->GetScreenCenter();
	float oldFov = g_view->GetFOV();

	WMatrix camera(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	camera.pivot.z = -25.0f;
	g_view->SetCamera(camera);
	g_view->ResetScreenCenter();
	g_view->SetFOV(0.9773844f);
	g_view->UpdateCamera();

	WTVertex tv;
	g_view->Projection2(&tv, pos);

	g_view->SetCamera(oldCamera);
	static_cast<WViewEx*>(g_view)->SetScreenCenter(oldCenter);
	g_view->SetFOV(oldFov);
	g_view->UpdateCamera();

	tv.sy -= 80.0f;
	for (i = 0; i < m_bonusBallCount; i++)
	{
		if (m_moving[i].pos.x <= 0.0f)
		{
			m_moving[i].pos = WVector(tv.sx, tv.sy, 0.0f);
			break;
		}
	}
	if (i < m_bonusBallCount)
	{
		// 983 008b9e90: ordem dos rand() = %10 (vy), %30 (vx), %8 (vz), %70 (alvo), %30 (giro), %2 (sentido)
		float vy = (float)(rand() % 10 - 5) * 4.3f + 50.0f;
		float vx = ((float)(rand() % 30) - 20.0f) * 20.6f;
		float vz = (float)(rand() % 8) - 8.0f;
		m_moving[i].velocity = WVector(vx, -vy, vz);
		m_moving[i].target =
			WVector(0.0f, (float)(rand() % 70) + m_rect.y + 240.0f, 0.0f);
		m_moving[i].state = 1;
		m_moving[i].bounceCount = 0;
		m_moving[i].depth =
			(m_moving[i].target.y - (m_rect.y + 240.0f) - 35.0f) * 0.5f / 70.0f;
		float speed = (float)(((double)(rand() % 30) + 20.0) / 10.0);
		m_moving[i].rotSpeed = (float)(rand() % 2 ? -1 : 1) * speed;
		ShowBonusBall(i);
	}
}

void FrBongdariShopDlg::InitCouponBongdariShop()
{
	m_couponCount = 0;
	std::list<sItemInfo>::iterator it;
	for (it = Doc()->m_myItemList.begin(); it != Doc()->m_myItemList.end();
		++it)
	{
		if (ItemManager()->GetCouponKind((*it).tid) == 2)
			m_couponCount += (*it).Common[0];
	}
}

void FrBongdariShopDlg::SetDataFee(int feeType)
{
	// 983 008b35a0: com o tipo 1 (cupom) ja ativo nada muda; ao entrar no tipo 1 gasta 1 cupom
	if (m_feeType != 1 && feeType > m_feeType)
	{
		m_feeType = feeType;
		m_feeTextY = 0.0f;
		m_feeTextAlpha = 0x200;
		if (feeType == 1)
			--m_couponCount;
	}
}

void FrBongdariShopDlg::UpdateUsableTimes()
{
	m_usableTimes[0] = Doc()->GetBsUsableTimes();
	m_usableTimes[1] = Doc()->GetBsUsableBonusTimes();
	m_usableTimes[2] = Doc()->GetBsRemainedBonusTimes();
	if (m_usableTimes[0] < 10)
		m_b448_983 = false;
}

void FrBongdariShopDlg::ShowDiscountUsableTimes()
{
	if (m_usableTimes[0] > 0 || m_usableTimes[0] == -1)
	{
		m_bShowDiscountTimes = true;
		m_discountTextY = 0.0f;
		m_discountTextAlpha = 0x200;
	}
	else if (m_usableTimes[1] > 0)
	{
		m_bShowDiscountCount = true;
		m_discountTextY = 0.0f;
		m_discountTextAlpha = 0x200;
	}
	else if (m_usableTimes[1] == 0 && m_usableTimes[2] == 0)
	{
		OpenBongdarishopWarningDlg(1, 0);
	}
	else
	{
		OpenBongdarishopWarningDlg(2, m_usableTimes[2]);
	}
	if (m_usableTimes[0] == -1)
		m_bShowDiscountTimes = false;
	if (m_usableTimes[1] == -1)
		m_bShowDiscountCount = false;
}

void FrBongdariShopDlg::Processing(float deltaTime)
{
	static int i;

	switch (m_state)
	{
	case 1:
		for (i = 0; i < 9; i++)
			m_silhouette[i].emotiTime = 0.0f;
		g_audio->PlaySfx("play_fail_item");
		m_state = 2;
		break;
	case 2:
	{
		bool bDone = true;
		for (i = 0; i < 9; i++)
		{
			m_silhouette[i].emotiTime += deltaTime * 15.0f;
			if ((int)m_silhouette[i].emotiTime < 8)
			{
				if ((int)m_silhouette[i].emotiTime > 2)
					m_silhouette[i].state = 1;
				bDone = false;
			}
		}
		if (bDone)
		{
			SetStartAnimationBongdari();
			m_state = 3;
		}
		break;
	}
	case 3:
		for (i = 0; i < m_bonusBallCount; i++)
		{
			if (m_pBonusBallModel[i] && m_moving[i].state == 1)
			{
				m_pBonusBallModel[i]->SetCenter(m_moving[i].pos.x,
					m_moving[i].pos.y);
				if (m_moving[i].bounceCount < 3)
				{
					if (!m_pBonusBall[i]->IsVisible())
						m_pBonusBall[i]->SetVisible(true);
					m_moving[i].pos.x += deltaTime * m_moving[i].velocity.x;
					m_moving[i].pos.y += deltaTime * m_moving[i].velocity.y;
					if (m_moving[i].velocity.x > deltaTime * 60.0f)
						m_moving[i].velocity.x -= deltaTime * 60.0f;
					else if (m_moving[i].velocity.x < -(deltaTime * 60.0f))
						m_moving[i].velocity.x += deltaTime * 60.0f;
					m_moving[i].velocity.y += deltaTime * 480.0f;
					if (m_moving[i].velocity.y > 0.0f &&
						m_moving[i].pos.y >= m_moving[i].target.y)
					{
						m_moving[i].velocity.y =
							-(m_moving[i].velocity.y / 3.0); // 983 008bf9cf: fdiv
						m_moving[i].velocity.x /= 1.5;
						m_moving[i].bounceCount++;
						g_audio->PlaySfx("\xb5\xb9\xc7\xc7\xb4\xcf_\xb0\xf8");
					}
					if (m_moving[i].velocity.x < 0.0f)
					{
						if (m_moving[i].pos.x <= m_rect.x + 50.0f)
							m_moving[i].velocity.x = -m_moving[i].velocity.x;
					}
					else if (m_moving[i].velocity.x > 0.0f &&
						m_moving[i].pos.x >= m_rect.w + m_rect.x + 50.0f)
					{
						m_moving[i].velocity.x = -m_moving[i].velocity.x;
					}
				}
				else
				{
					m_moving[i].pos.y = m_moving[i].target.y;
					m_moving[i].state = 2;
					WRect rect = m_pBonusBall[i]->GetRect();
					rect.x = m_moving[i].pos.x - rect.w * 0.5f;
					rect.y = m_moving[i].pos.y - rect.h * 0.5f;
					m_pBonusBall[i]->SetRect(rect);
				}
			}
		}
		for (i = 0; i < m_bonusBallCount; i++)
		{
			if (m_moving[i].state != 2)
				return;
		}
		m_state = 4;
		break;
	case 4:
		m_float42c983 += deltaTime; // 983 008bf9ce..: +0x42c
		for (i = 0; i < m_bonusBallCount; i++)
		{
			switch (m_moving[i].state)
			{
			case 2:
				if (m_pBonusBallModel[i])
				{
					WMatrix& mat = m_pBonusBallModel[i]->GetMatrix();
					static_cast<WMatrixEx&>(mat) *= RotMat(deltaTime *
							m_moving[i].rotSpeed * 8.0f * g_DEGTORAD,
						2);
					static_cast<WMatrixEx&>(mat) *= RotMat(deltaTime *
							m_moving[i].rotSpeed * 8.0f * g_DEGTORAD,
						1);
					// 983 008bfc43: auto/combo seguinte abre as bolas sozinho, uma a cada 0.3 s
					if ((m_bAutoPlay983 || m_i434_983 != 0) &&
						(float)i * 0.3 < m_float42c983)
						OpenBonusBall(i);
				}
				break;
			case 3:
				if (m_pBonusBallModel[i])
				{
					m_pBonusBallModel[i]->SetCenter(m_moving[i].pos.x,
						m_moving[i].pos.y);
					if (m_moving[i].pos.y < m_moving[i].target.y)
					{
						m_moving[i].pos.x +=
							deltaTime * m_moving[i].velocity.x * 2.0f;
						m_moving[i].pos.y +=
							deltaTime * m_moving[i].velocity.y * 2.0f;
					}
					else
					{
						m_moving[i].pos.y = m_moving[i].target.y;
						m_moving[i].state = 4;
					}
				}
				break;
			case 4:
				if (m_pBonusBallModel[i])
				{
					delete m_pBonusBallModel[i];
					m_pBonusBallModel[i] = NULL;
					m_pBonusBall[i]->SetVisible(false);
					m_moving[i].state = 5;
					switch (m_bonusBall[i].rarity983)
					{
					case 2:
						g_audio->PlaySfx(
							"\xbe\xc6\xc0\xcc\xc5\xdb\xc8\xb9\xb5\xe6(\xb7\xb9\xbe\xee)");
						OpenBongdarishopRareItemDlg(i);
						break;
					case 1:
						g_audio->PlaySfx(
							"\xbe\xc6\xc0\xcc\xc5\xdb\xc8\xb9\xb5\xe6(\xc4\xed\xc5\xb0)");
						break;
					case 0:
						g_audio->PlaySfx(
							"\xbe\xc6\xc0\xcc\xc5\xdb\xc8\xb9\xb5\xe6(\xc6\xce)");
						break;
					}
					for (int j = 0; j < 9; j++)
					{
						if (m_silhouette[j].bonusBallIndex == i)
						{
							m_silhouette[j].emotiTime = 0.0f;
							break;
						}
					}
				}
				break;
			case 5:
				for (int j = 0; j < 9; j++)
				{
					if (m_silhouette[j].bonusBallIndex == i)
					{
						m_silhouette[j].emotiTime += deltaTime * 15.0f;
						if ((int)m_silhouette[j].emotiTime < 8)
						{
							if ((int)m_silhouette[j].emotiTime > 2)
							{
								m_silhouette[j].state = 2;
								SetOpeningBonusBallInfo983(&m_bonusBall[i], 1); // 983 008bacd0
							}
						}
						else
						{
							m_moving[i].state = 6;
							AddListTotalItem(&m_bonusBall[i]);
							UpdateListMyItem(&m_bonusBall[i]);
							SendReBuildMyInventory();
						}
						break;
					}
				}
				break;
			case 6:
				m_bonusBall[i].index = -1;
				break;
			}
		}
		{
			bool bAny = false;
			for (i = 0; i < m_bonusBallCount; i++)
			{
				if (m_bonusBall[i].index >= 0)
				{
					bAny = true;
					break;
				}
			}
			// 983 008bff24: combo vai direto ao estado 6
			if (m_comboMode983 == 1)
				m_state = 6;
			else if (!bAny)
			{
				m_pBongdariModel->SetMotion(s_strMotionRestart, false, 0.0f);
				m_state = 5;
			}
		}
		break;
	case 5:
		break;
	case 6:
		if (m_comboMode983 == 0)
			m_pBongdariModel->SetMotion(s_strMotionDefault, false, 0.0f);
		m_state = 7;
		m_stateTimer = 0.0f;
		break;
	case 7:
		m_stateTimer += deltaTime;
		if (m_stateTimer > 2.0f) // 983 00cfea84 = 2.0f
		{
			m_state = 8;
			m_stateTimer = 0.0f;
		}
		break;
	case 8:
		for (int j = 0; j < 9; j++)
			m_silhouette[j].emotiTime = 0.0f;
		g_audio->PlaySfx("play_fail_item");
		m_state = 9;
		SetOpeningBonusBallInfo983(NULL, 1); // 983 008bacd0
		if (m_comboMode983 == 1)
			SetModeMessage983(); // 008b3670
		break;
	case 9:
	{
		bool bDone = false;
		for (int j = 0; j < 9; j++)
		{
			m_silhouette[j].emotiTime += deltaTime * 15.0f;
			if ((int)m_silhouette[j].emotiTime < 8)
			{
				if ((int)m_silhouette[j].emotiTime > 2)
					m_silhouette[j].state = 0;
			}
			else
			{
				bDone = true;
			}
		}
		if (bDone)
			EndPlay983();
		break;
	}
	}
}

// 983 008bf5e0/008bf670: abre "total_item" (008be980 = CreateForm) com a lista dada; tambem no fim do auto-play/combo
void FrBongdariShopDlg::RefreshTotalItem983(std::list<sBonusBall>* pList)
{
	FrTotalItemDlg* dlg = CreateForm<FrTotalItemDlg>(g_pFresh->GetManager(),
		this, "total_item", NULL);
	if (dlg)
	{
		dlg->SetListItem(pList);
		dlg->Open((FRESH_PFN_RESULT)&FrBongdariShopDlg::OnTotalItemDlgResult983, 3); // vcall +0x64
		m_bChildDlgOpen = true; // +0x3d8
	}
}

// 983 008b3500: fecha "total_item"; no combo (+0x444 == 1) volta ao estado 0 e ao movimento de preparo
bool FrBongdariShopDlg::OnTotalItemDlgResult983(int result, FrForm* pForm)
{
	m_bChildDlgOpen = false;
	if (m_comboMode983 == 1)
	{
		m_state = 0;
		m_pBongdariModel->SetMotion("\xbf\xac\xbc\xd3\xbb\xcc\xb1\xe2\xc1\xd8\xba\xf1", false, 0.0f);
	}
	return true;
}

// 983 008c00a0 (fim do estado 9 de Processing): fim da jogada; auto-play repete, senao devolve os botoes
void FrBongdariShopDlg::EndPlay983()
{
	if ((int)Doc()->m_specialShotMark == 2)
	{
		m_state = 0;
		if (m_listTotalItem.size() != 0 || m_listResult983.size() != 0)
			RefreshTotalItem983(m_shopMode983 == 2 ? &m_listResult983 : &m_listTotalItem); // 983 008bf5e0: this+0xaf4 (D829: era Doc()+0xaf4 = mapStat)
		// 983 008c00f4: mesmo aviso de limite de 20.000 itens de 008bc2a0
		AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23), (int)K2L_Compatibility("\xc7\xf6\xc0\xe7 \xc7\xd1 \xc1\xbe\xb7\xf9\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb \xba\xb8\xc0\xaf \xb0\xb3\xbc\xf6\xb0\xa1 20,000\xb0\xb3\xc0\xcc\xb9\xc7\xb7\xce \xc7\xd8\xb4\xe7 \xc4\xc1\xc5\xd9\xc3\xf7\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."), 1, 0, 0, 0);
		return;
	}
	m_state = 0;
	m_bUsingBonusBall = false;
	if (!m_bAutoPlay983 || m_comboMode983 != 0)
	{
		m_pNormalPlayBn983->SetVisible(true); // 008b3e40(1)
		m_pComboPlayBn983->SetVisible(false);
		if (m_comboMode983 == 1)
		{
			RefreshTotalItem983(&m_listSecond983);
			for (std::list<sBonusBall>::iterator it = m_listSecond983.begin();
				it != m_listSecond983.end(); ++it)
				UpdateListMyItem(&*it);
		}
		else
		{
			if (m_i434_983 != 0)
				RefreshTotalItem983(m_shopMode983 == 2 ? &m_listResult983 : &m_listTotalItem); // 983 008c029e -> 008bf5e0
			m_i434_983 = 0;
			if (m_pAutoStartBn983)
			{
				SetAutoPlayImg983(1);
				m_pAutoStartBn983->SetVisible(true);
			}
		}
		m_pBackSelectBn983->Enable(true);
	}
	else if (m_i434_983 < m_i438_983 && m_usableTimes[0] != 0 &&
		(Doc()->m_myInfo.stat.i64Pang >= 900 || m_couponCount > 0))
	{
		WSendPacket packet((enumClientPacket)0x14b);
		packet.Send((eSendTo)0);
		g_audio->PlaySfx(STR_SFX_CLICK);
		if (m_exitEnableTime >= 4.0f && m_pCloseBn)
			m_pCloseBn->Enable(false); // 008b3d10
		m_pBackSelectBn983->Enable(false);
		m_i434_983++;
		m_exitEnableTime = 0.0f;
	}
	else
	{
		m_bAutoPlay983 = false;
		m_pBackSelectBn983->Enable(true);
		SetAutoPlayImg983(1);
		RefreshTotalItem983(m_shopMode983 == 2 ? &m_listResult983 : &m_listTotalItem); // 983 008c0248 -> 008bf5e0
		m_i434_983 = 0;
	}
}

void FrBongdariShopDlg::SetPosMovingClickText(float deltaTime)
{
	if (m_clickTextDir == 0)
	{
		m_clickTextOffset += deltaTime * 10.0f;
		if (m_clickTextOffset >= 5.0f)
			m_clickTextDir = 1;
	}
	else if (m_clickTextDir == 1)
	{
		m_clickTextOffset -= deltaTime * 10.0f;
		if (m_clickTextOffset <= 0.0f)
			m_clickTextDir = 0;
	}
}

void FrBongdariShopDlg::DisplayItemSilhouette()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device && m_bonusBallCount <= 5)
	{
		float baseX = GetRect().x;
		float baseY = GetRect().y;
		char text[8] = { 0 };
		unsigned long color = 0xffffffff;
		// 983 008ba2c0: o bitmap nao e zerado por slot e o estado 2 volta a cor para branco
		// (o nosso desenhava o item revelado com o tom do slot anterior)
		const Bitmap* bitmap = NULL;
		for (int i = 0; i < 9; i++)
		{
			switch (m_silhouette[i].state)
			{
			case 0:
				bitmap = m_pDefaultSilhouette;
				color = 0xffffffff;
				break;
			case 1:
				switch (m_silhouette[i].grade)
				{
				case 1:
					bitmap = m_silhouette[i].pBitmap;
					color = 0xa02020a0;
					break;
				case 2:
					bitmap = m_silhouette[i].pBitmap;
					color = 0xa0a02020;
					break;
				default:
					bitmap = m_silhouette[i].pBitmap;
					color = 0xa0202020;
					break;
				}
				break;
			case 2:
				bitmap = m_silhouette[i].pBitmap;
				color = 0xffffffff;
				break;
			}
			if (bitmap)
			{
				float ox = 0.0f;
				float oy = 0.0f;
				if (m_silhouette[i].state > 0 && m_silhouette[i].state <= 2)
				{
					if (ItemManager()->GetCouponKind(m_silhouette[i].typeId))
					{
						ox = -7.0f;
						oy = 2.0f;
					}
				}
				WRect rect(baseX + ox + g_arrPtItemSilhouette[i].x,
					baseY + g_arrPtItemSilhouette[i].y + oy,
					(float)bitmap->Width(), (float)bitmap->Height());
				device->DrawTexture(bitmap, rect, color, 0);
				if (m_silhouette[i].state == 2 &&
					m_silhouette[i].bonusBallIndex >= 0)
				{
					int quantity =
						m_bonusBall[m_silhouette[i].bonusBallIndex].quantity;
					if (quantity > 1 && m_pFontWind)
					{
						sprintf(text, "%d", quantity);
						m_pFontWind->Print(g_view, rect.x + 32.0f,
							rect.y + 30.0f, text, 0, 0xffffffff, 0);
					}
				}
				if (m_silhouette[i].emotiTime >= 0.0f &&
					(int)m_silhouette[i].emotiTime < 8)
				{
					if (m_pEmoti)
					{
						m_pEmoti->Render(
							MakeStr("%d", (int)m_silhouette[i].emotiTime),
							baseX + g_arrPtItemSilhouette[i].x - 25.0f,
							baseY + g_arrPtItemSilhouette[i].y - 43.0f, 1.5f,
							0);
					}
				}
			}
		}
	}
}

void FrBongdariShopDlg::DrawShadow()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device == NULL || m_pShadow == NULL)
		return;

	if (m_state == 0 && m_pBongdariModel != NULL)
	{
		const WRect& area = m_pBongdari->GetRect();
		// 983 008b5ac0: canto do Bongdari + offset START (+0x54c/+0x550), y +100, tamanho da sombra x1.5
		float w = (float)m_pShadow->Width() * 1.5f;
		float x = area.x + m_cfgStartImgX983;
		float h = (float)m_pShadow->Height() * 1.5f;
		float y = area.y + m_cfgStartImgY983 + 100.0f;
		device->DrawTexture(m_pShadow, WRect(x, y, w, h), 0x90ffffff, 0);
	}

	for (int i = 0; i < m_bonusBallCount; i++)
	{
		if (m_pBonusBallModel[i] == NULL)
			continue;

		float dist = fabsf(m_moving[i].pos.y - m_moving[i].target.y);
		if (dist >= 40.0f)
			continue;

		unsigned char alpha = 0x90;
		if (dist > 0.0f)
			alpha = (unsigned char)(144.0f / (dist * 1.5f));

		float x = m_moving[i].pos.x;
		float y = m_moving[i].target.y;
		WRect rect;
		if (m_hoverBonusBall == i)
		{
			float w = (float)m_pShadow->Width() * 0.7f;
			float h = (float)m_pShadow->Height() * 0.7f;
			rect = WRect(x - 0.5f * w, y - 0.5f * h + 36.0f, w, h);
		}
		else
		{
			float w = (float)m_pShadow->Width() * 0.55f;
			float h = (float)m_pShadow->Height() * 0.55f;
			rect = WRect(x - 0.5f * w, y - 0.5f * h + 28.0f, w, h);
		}
		device->DrawTexture(m_pShadow, rect, (alpha << 24) | 0xffffff, 0);
	}
}

void FrBongdariShopDlg::DrawTextBongdariClick()
{
	// 983 008b5da0: modo combo so mostra com +0x448; textura por modo (auto 0x430 / combo 0x44c / click)
	if (m_comboMode983 == 1 && !m_b448_983)
		return;
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device == NULL || m_pBongdari == NULL || m_pTextBongdariClick == NULL ||
		m_pTextAutoClick983 == NULL)
		return;

	const Bitmap* tex = m_pTextBongdariClick;
	if (m_comboMode983 == 0 && m_bAutoPlay983)
		tex = m_pTextAutoClick983;
	else if (m_comboMode983 == 1)
		tex = m_pTextComboPlayClick983;

	if (tex == NULL)
		return;
	const WRect& area = m_pBongdari->GetRect();
	device->DrawTexture(tex,
		WRect(m_cfgStartImgX983 + area.x - 13.0f,
			m_cfgStartImgY983 + area.y - 100.0f + m_clickTextOffset,
			m_pTextBongdariClick->Width(), m_pTextBongdariClick->Height()),
		0xffffffff, 0);
}

void FrBongdariShopDlg::DrawTextBonusBallClick()
{
	FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
	if (device == NULL || m_pTextBonusBallClick == NULL)
		return;

	for (int i = 0; i < 5; i++)
	{
		if (m_pBonusBall[i] == NULL)
			return;
	}

	int index = m_hoverBonusBall;
	if (index >= 0 && index < m_bonusBallCount)
	{
		const WRect& area = m_pBonusBall[index]->GetRect();
		device->DrawTexture(m_pTextBonusBallClick,
			WRect(area.x - 10.0f, area.y - 85.0f + m_clickTextOffset,
				m_pTextBonusBallClick->Width(),
				m_pTextBonusBallClick->Height()),
			0xffffffff, 0);
	}
}

void FrBongdariShopDlg::DrawUserInfo()
{
	if (IsLocalContent((localContentType_t)6))
	{
		FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
		if (!device || !m_pFontBong || !m_pFontBongRed)
			return;
		float baseX = m_rect.x;
		float baseY = m_rect.y;
		char text[32] = { 0 };
		static int fontw = m_pFontBong->GetFontInfo()->fontw;
		static int fontwRed = m_pFontBongRed->GetFontInfo()->fontw;
		static float width;

		// 983 008ba540: so o usavel [0] (sem -1), com a fonte normal; o [1] nao e desenhado
		if (m_usableTimes[0] < 0)
		{
			sprintf(text, "-");
			m_pFontBong->Print(g_view, baseX + 180.0f, baseY + 16.0f, text, 0,
				0xffffffff, NULL);
		}
		else
		{
			MakePriceStrA(text, m_usableTimes[0]);
			width = (float)strlen(text) * ((float)fontw - 1.0f);
			m_pFontBong->Print(g_view, baseX + 180.0f - width, baseY + 16.0f,
				text, 0, 0xffffffff, NULL);
		}

		MakePriceStrA(text, m_couponCount);
		width = (float)strlen(text) * ((float)fontw - 1.0f);
		m_pFontBong->Print(g_view, baseX + 327.0f - width, baseY + 16.0f, text,
			0, 0xffffffff, NULL);

		MakePriceStrA(text, Doc()->m_myInfo.stat.i64Pang);
		width = (float)strlen(text) * ((float)fontw - 1.0f);
		m_pFontBong->Print(g_view, baseX + 485.0f - width, baseY + 16.0f, text,
			0, 0xffffffff, NULL);

		if (m_feeType >= 0 && m_pTextFee[m_feeType])
		{
			unsigned char alpha;
			if (m_feeTextAlpha < 0)
			{
				alpha = 0;
				m_feeType = -1;
			}
			else if (m_feeTextAlpha < 0x100)
				alpha = (unsigned char)m_feeTextAlpha;
			else
				alpha = 0xff;
			WRect rect(baseX, baseY + m_feeTextY,
				(float)m_pTextFee[m_feeType]->Width(),
				(float)m_pTextFee[m_feeType]->Height());
			switch (m_feeType)
			{
			case 2: // 983: o tipo 2 posiciona como o 0
			case 0:
				rect.x = baseX + 446.0f;
				rect.y += 10.0f;
				break;
			case 1:
				rect.x = baseX + 317.0f;
				rect.y += 10.0f;
				break;
			}
			device->DrawTexture(m_pTextFee[m_feeType], rect,
				(alpha << 24) | 0xffffff, 0);
		}

		if (m_bShowDiscountTimes && m_pTextFee[1])
		{
			unsigned char alpha;
			if (m_discountTextAlpha < 0)
			{
				alpha = 0;
				m_bShowDiscountTimes = false;
			}
			else if (m_discountTextAlpha < 0x100)
				alpha = (unsigned char)m_discountTextAlpha;
			else
				alpha = 0xff;
			WRect rect(baseX + 160.0f, baseY + m_discountTextY + 10.0f,
				(float)m_pTextFee[1]->Width(), (float)m_pTextFee[1]->Height());
			device->DrawTexture(m_pTextFee[1], rect, (alpha << 24) | 0xffffff,
				0);
		}

		if (m_bShowDiscountCount && m_pTextFee[1])
		{
			unsigned char alpha;
			if (m_discountTextAlpha < 0)
			{
				alpha = 0;
				m_bShowDiscountTimes = false;
			}
			else if (m_discountTextAlpha < 0x100)
				alpha = (unsigned char)m_discountTextAlpha;
			else
				alpha = 0xff;
			WRect rect(baseX + 200.0f, baseY + m_discountTextY + 10.0f,
				(float)m_pTextFee[1]->Width(), (float)m_pTextFee[1]->Height());
			device->DrawTexture(m_pTextFee[1], rect, (alpha << 24) | 0xffffff,
				0);
		}
	}
	else
	{
		FrGraphicInterface* device = g_pFresh->GetManager()->GetGDI();
		if (!device || !m_pFontBong)
			return;
		float baseX = m_rect.x;
		float baseY = m_rect.y;
		char text[32] = { 0 };
		static int fontw = m_pFontBong->GetFontInfo()->fontw;
		static float width;

		MakePriceStrA(text, m_couponCount);
		width = (float)strlen(text) * ((float)fontw - 1.0f);
		m_pFontBong->Print(g_view, baseX + 306.0f - width, baseY + 16.0f, text,
			0, 0xffffffff, NULL);

		MakePriceStrA(text, Doc()->m_myInfo.stat.i64Pang);
		width = (float)strlen(text) * ((float)fontw - 1.0f);
		m_pFontBong->Print(g_view, baseX + 479.0f - width, baseY + 16.0f, text,
			0, 0xffffffff, NULL);

		if (m_feeType >= 0 && m_pTextFee[m_feeType])
		{
			unsigned char alpha;
			if (m_feeTextAlpha < 0)
			{
				alpha = 0;
				m_feeType = -1;
			}
			else if (m_feeTextAlpha < 0x100)
				alpha = (unsigned char)m_feeTextAlpha;
			else
				alpha = 0xff;
			WRect rect(baseX, baseY + m_feeTextY,
				(float)m_pTextFee[m_feeType]->Width(),
				(float)m_pTextFee[m_feeType]->Height());
			switch (m_feeType)
			{
			case 0:
				rect.x = baseX + 440.0f;
				rect.y += 10.0f;
				break;
			case 1:
				rect.x = baseX + 296.0f;
				rect.y += 10.0f;
				break;
			}
			device->DrawTexture(m_pTextFee[m_feeType], rect,
				(alpha << 24) | 0xffffff, 0);
		}
	}
}

void FrBongdariShopDlg::SetOpeningBonusBallInfo(sBonusBall* pBonusBall)
{
	char text[256];
	if (pBonusBall)
	{
		IFF_ITEM_COMMON* common =
			ItemManager()->FindCommonItem(pBonusBall->typeId);
		if (common)
		{
			SetMessage(MakeStr(K2L_Compatibility("%s %d\xb0\xb3 \xc8\xb9\xb5\xe6"), common->Name,
						   pBonusBall->quantity),
				false);
		}
	}
	else
	{
		SetMessage(
			K2L_Compatibility("\xc8\xb9\xb5\xe6 \xbe\xc6\xc0\xcc\xc5\xdb \xc1\xa4\xba\xb8\xb0\xa1 \xc7\xa5\xbd\xc3\xb5\xcb\xb4\xcf\xb4\xd9"),
			false);
	}
}

void FrBongdariShopDlg::SetOpeningBonusBallInfo983(sBonusBall* pBonusBall, int kind)
{
	// 983 008bacd0: 1 bola = "nome N copias"; varias = texto de recebimento; sem bola = texto padrao
	if (pBonusBall)
	{
		if (kind == 1)
		{
			IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(pBonusBall->typeId);
			if (common)
				SetMessage(MakeStr(K2L_Compatibility("%s %d\xb0\xb3 \xc8\xb9\xb5\xe6"),
							   common->Name, pBonusBall->quantity),
					false);
		}
		else
			SetMessage(
				K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."),
				false);
	}
	else
		SetMessage(
			K2L_Compatibility("\xc8\xb9\xb5\xe6 \xbe\xc6\xc0\xcc\xc5\xdb \xc1\xa4\xba\xb8\xb0\xa1 \xc7\xa5\xbd\xc3\xb5\xcb\xb4\xcf\xb4\xd9"),
			false);
}

void FrBongdariShopDlg::ShowBongdari()
{
	if (m_pBongdariModel != NULL)
	{
		delete m_pBongdariModel;
		m_pBongdariModel = NULL;
	}

	if (m_pBongdari != NULL)
	{
		WRect area = m_pBongdari->GetRect();
		// 983 008b5fb0: valores fixos (os do 645 em 800x600) e add_bongdari_x/y do memorial.lua
		m_pBongdariModel =
			new CExhibitionByHardwareTnL(m_cfgAddBongdariX983 + area.x + area.w * 0.5f,
				area.h * 0.5f + m_cfgAddBongdariY983 + area.y, 0.14375f, true);
		m_pBongdariModel->SetDistanceRange(28.75f, 38.75f);
		m_pBongdariModel->SetDistance(33.75f, true, true);
		m_pBongdariModel->SetArea(area.w, area.h);
		m_pBongdariModel->SetModel("bon_shop", (eExhibitType)1);
	}
}

void FrBongdariShopDlg::ShowBonusBall(int index)
{
	for (int i = 0; i < 5; i++)
	{
		if (m_pBonusBall[i] == NULL)
			return;
	}

	char model[64] = { 0 };
	float w = m_pBonusBall[index]->GetRect().w;
	float h = m_pBonusBall[index]->GetRect().h;
	m_pBonusBallModel[index] = new CExhibitionByHardwareTnL(m_moving[index].pos.x,
		m_moving[index].pos.y, 0.0f, true);
	m_pBonusBallModel[index]->SetDistance((3.5f - m_moving[index].depth) * 1.25f, true, true); // 983 008b620a
	m_pBonusBallModel[index]->SetArea(w, h);

	switch (m_bonusBall[index].index)
	{
	case 0:
		sprintf(model, "B_dol01");
		break;
	case 1:
		sprintf(model, "B_dol02");
		break;
	case 2:
		sprintf(model, "B_dol03");
		break;
	}

	if (strlen(model) != 0)
		m_pBonusBallModel[index]->SetModel(model, (eExhibitType)2);
}

void FrBongdariShopDlg::DeletePetBonusBall()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_pBonusBallModel[i] != NULL)
		{
			delete m_pBonusBallModel[i];
			m_pBonusBallModel[i] = NULL;
		}
	}
}

void FrBongdariShopDlg::SetStartAnimationBongdari()
{
	if (m_pBongdariModel != NULL)
	{
		m_bAnimating = true;
		if (m_comboMode983 == 1)
		{
			m_pBongdariModel->SetMotion(s_strMotionComboFire983, false, 0.0f);
			return;
		}
		m_pBongdariModel->SetMotion(s_strMotionPick[rand() % 5], false, 0.0f);
	}
}

void FrBongdariShopDlg::InitRandomDataItemSilhouette()
{
	int index;
	unsigned long limit;
	int i;

	index = 0;
	limit = g_arrdwPercentNumCookieItem0[0][0];
	for (i = 0; i < 100; i++)
	{
		g_arrdwRandNumCookieItem0[i] = g_arrdwPercentNumCookieItem0[index][1];
		if (i == limit - 1)
		{
			index++;
			limit += g_arrdwPercentNumCookieItem0[index][0];
		}
	}

	index = 0;
	limit = g_arrdwPercentNumCookieItem1[0][0];
	for (i = 0; i < 100; i++)
	{
		g_arrdwRandNumCookieItem1[i] = g_arrdwPercentNumCookieItem1[index][1];
		if (i == limit - 1)
		{
			index++;
			limit += g_arrdwPercentNumCookieItem1[index][0];
		}
	}

	index = 0;
	limit = g_arrdwPercentNumCookieItem2[0][0];
	for (i = 0; i < 100; i++)
	{
		g_arrdwRandNumCookieItem2[i] = g_arrdwPercentNumCookieItem2[index][1];
		if (i == limit - 1)
		{
			index++;
			limit += g_arrdwPercentNumCookieItem2[index][0];
		}
	}

	index = 0;
	limit = g_arrdwPercentSelectCookieItem[0][0];
	for (i = 0; i < 100; i++)
	{
		g_arrdwRandSelectCookieItem[i] =
			g_arrdwPercentSelectCookieItem[index][1];
		if (i == limit - 1)
		{
			index++;
			limit += g_arrdwPercentSelectCookieItem[index][0];
		}
	}

	index = 0;
	limit = g_arrdwPercentSelectPangItem[0][0];
	for (i = 0; i < 100; i++)
	{
		g_arrdwRandSelectPangItem[i] = g_arrdwPercentSelectPangItem[index][1];
		if (i == limit - 1)
		{
			index++;
			limit += g_arrdwPercentSelectPangItem[index][0];
		}
	}
}

void FrBongdariShopDlg::ShuffleRandomDataItemSilhouette()
{
	int i;
	int r;
	unsigned long temp;

	for (i = 0; i < 100; i++)
	{
		r = rand() % 100;
		temp = g_arrdwRandNumCookieItem0[i];
		g_arrdwRandNumCookieItem0[i] = g_arrdwRandNumCookieItem0[r];
		g_arrdwRandNumCookieItem0[r] = temp;

		r = rand() % 100;
		temp = g_arrdwRandNumCookieItem1[i];
		g_arrdwRandNumCookieItem1[i] = g_arrdwRandNumCookieItem1[r];
		g_arrdwRandNumCookieItem1[r] = temp;

		r = rand() % 100;
		temp = g_arrdwRandNumCookieItem2[i];
		g_arrdwRandNumCookieItem2[i] = g_arrdwRandNumCookieItem2[r];
		g_arrdwRandNumCookieItem2[r] = temp;
	}

	for (i = 0; i < 100; i++)
	{
		r = rand() % 100;
		temp = g_arrdwRandSelectCookieItem[i];
		g_arrdwRandSelectCookieItem[i] = g_arrdwRandSelectCookieItem[r];
		g_arrdwRandSelectCookieItem[r] = temp;
	}

	for (i = 0; i < 100; i++)
	{
		r = rand() % 100;
		temp = g_arrdwRandSelectPangItem[i];
		g_arrdwRandSelectPangItem[i] = g_arrdwRandSelectPangItem[r];
		g_arrdwRandSelectPangItem[r] = temp;
	}
}

void FrBongdariShopDlg::SetRandomDataItemSilhouette()
{
	int numCookie = 0;
	int numRare = 0;
	int count = 0;
	int i;
	for (i = 0; i < 5; i++)
	{
		if (m_bonusBall[i].rarity983 == 1)
			numCookie++;
		else if (m_bonusBall[i].rarity983 == 2)
			numRare++;
	}

	if (numRare > 0)
	{
		for (i = 0; i < m_bonusBallCount; i++)
		{
			if (m_bonusBall[i].rarity983 == 2)
			{
				m_silhouette[count].bonusBallIndex = i;
				m_silhouette[count].pBitmap =
					g_pFresh->GetManager()->GetBitmap("ITEMS", "item_rare");
				m_silhouette[count].typeId = m_bonusBall[i].typeId;
				m_silhouette[count].grade = 2;
				m_silhouette[count].state = 0;
				m_silhouette[count].emotiTime = 10.0f;
				count++;
			}
		}
	}

	int numCookieShow = 0;
	if (numCookie == 0)
		numCookieShow = g_arrdwRandNumCookieItem0[rand() % 100];
	else if (numCookie == 1)
		numCookieShow = g_arrdwRandNumCookieItem1[rand() % 100];
	else if (numCookie == 2)
		numCookieShow = g_arrdwRandNumCookieItem2[rand() % 100];

	if (numCookie > 0)
	{
		for (i = 0; i < m_bonusBallCount; i++)
		{
			if (m_bonusBall[i].rarity983 == 1)
			{
				IFF_ITEM_COMMON* common =
					ItemManager()->FindCommonItem(m_bonusBall[i].typeId);
				m_silhouette[count].bonusBallIndex = i;
				if (common)
					m_silhouette[count].pBitmap =
						g_pFresh->GetManager()->GetBitmap("ITEMS",
							common->Icon);
				else
					m_silhouette[count].pBitmap = NULL;
				m_silhouette[count].typeId = m_bonusBall[i].typeId;
				m_silhouette[count].grade = 1;
				m_silhouette[count].state = 0;
				m_silhouette[count].emotiTime = 10.0f;
				count++;
			}
		}
	}

	if (numCookieShow > 0)
	{
		for (i = numCookieShow - numCookie; i > 0; i--)
		{
			unsigned long typeId = SelectDifferentTypeId(1); // 983 008baf0c
			IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
			m_silhouette[count].bonusBallIndex = -1;
			if (common)
				m_silhouette[count].pBitmap =
					g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon);
			else
				m_silhouette[count].pBitmap = NULL;
			m_silhouette[count].typeId = typeId;
			m_silhouette[count].grade = 1;
			m_silhouette[count].state = 0;
			m_silhouette[count].emotiTime = 10.0f;
			count++;
		}
	}

	for (i = 0; i < m_bonusBallCount; i++)
	{
		if (m_bonusBall[i].rarity983 == 0)
		{
			IFF_ITEM_COMMON* common =
				ItemManager()->FindCommonItem(m_bonusBall[i].typeId);
			m_silhouette[count].bonusBallIndex = i;
			if (common)
				m_silhouette[count].pBitmap =
					g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon);
			else
				m_silhouette[count].pBitmap = NULL;
			m_silhouette[count].typeId = m_bonusBall[i].typeId;
			m_silhouette[count].grade = 0;
			m_silhouette[count].state = 0;
			m_silhouette[count].emotiTime = 10.0f;
			count++;
		}
	}

	for (; count < 9; count++)
	{
		unsigned long typeId = g_arrdwRandSelectPangItem[rand() % 100];
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
		m_silhouette[count].bonusBallIndex = -1;
		if (common)
			m_silhouette[count].pBitmap =
				g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon);
		else
			m_silhouette[count].pBitmap = NULL;
		m_silhouette[count].typeId = typeId;
		m_silhouette[count].grade = 0;
		m_silhouette[count].state = 0;
		m_silhouette[count].emotiTime = 10.0f;
	}

	for (i = 0; i < 9; i++)
	{
		int j = rand() % 9;
		sItemSilhouette temp = m_silhouette[i];
		m_silhouette[i] = m_silhouette[j];
		m_silhouette[j] = temp;
	}
}

unsigned long FrBongdariShopDlg::SelectDifferentTypeId(int type)
{
	unsigned long* table;
	if (type == 0)
		table = g_arrdwRandSelectPangItem;
	else if (type == 1)
		table = g_arrdwRandSelectCookieItem;
	else
		return 0;

	unsigned long typeId = table[rand() % 100];
	int i = 0;
	while (i < 9)
	{
		if (m_silhouette[i].typeId == 0)
			break;
		if (m_silhouette[i].typeId == typeId)
		{
			typeId = table[rand() % 100];
			i = 0;
			continue;
		}
		i++;
	}
	return typeId;
}

void FrBongdariShopDlg::AddListTotalItem(sBonusBall* pBonusBall)
{
	if (!pBonusBall)
		return;
	for (std::list<sBonusBall>::iterator it = m_listTotalItem.begin();
		it != m_listTotalItem.end(); it++)
	{
		if ((*it).typeId == pBonusBall->typeId)
		{
			if (!(*it).bTime)
				(*it).quantity += pBonusBall->quantity;
			else
				(*it).period += pBonusBall->period;
			return;
		}
	}
	m_listTotalItem.push_back(*pBonusBall);
}

// 008bec90 (so do 983)
void FrBongdariShopDlg::AddResultBalls983(const sBonusBall* balls, int count)
{
	for (int i = 0; i < count; i++)
		m_listResult983.push_back(balls[i]);
}

// 008bed00 (so do 983): msg 0x328 (pacote 0x264)
void FrBongdariShopDlg::SetBongdariResult983(const std::vector<unsigned int>& results,
	const std::vector<unsigned long>& typeIds, const std::vector<int>& quantities, int count)
{
	if (count == 1)
	{
		m_result983 = results[0];
		int result = results[0];
		if (result == -1)
			m_resultMode983 = 0;
		else if (result >= 0 && result <= 2)
			m_resultMode983 = count;
		else if (result >= 3 && result <= 4)
			m_resultMode983 = 2;
	}
	else
	{
		m_result983 = -1;
		m_resultMode983 = 0;
	}
	for (int i = 0; i < count; i++)
	{
		m_resultBall983[i].typeId = typeIds[i];
		m_resultBall983[i].quantity = quantities[i];
		m_resultBall983[i].period = 0;
	}
	AddResultBalls983(m_resultBall983, count);
	m_resultCount983 = count;
	m_bResultReady983 = true;
}

// 983 008bd4a0: o 983 nao mantem mais as listas do inventario aqui (logica do 645 removida); so recalcula os cupons.
void FrBongdariShopDlg::UpdateListMyItem(sBonusBall* pBonusBall)
{
	if (pBonusBall != NULL && pBonusBall->index >= 0)
	{
		if (ItemManager()->GetCouponKind(pBonusBall->typeId) == 2)
			InitCouponBongdariShop();
	}
}

void FrBongdariShopDlg::SendStateBongdariShop(bool bState)
{
	// 983 008bb130: grava o estado no CSharedDoc (+0x11410) em vez de mandar a msg 0xaf a Lobby/Shop/RealMyRoom.
	if (IsLocalContent((localContentType_t)1))
		Doc()->m_bBongdariShopOpen = bState;
}

void FrBongdariShopDlg::SendReBuildMyInventory()
{
	// 983 008bb160: uma unica msg 0xd4 ao ator principal (nao mais Shop + RealMyRoom por nome)
	if (IsLocalContent((localContentType_t)1))
	{
		if (AfxGetTask() && AfxGetTask()->GetMainActor())
			AfxGetTask()->GetMainActor() << MsgObject(NULL, 0xd4, 0, 0, 0, 0, 0);
	}
}

void FrBongdariShopDlg::SendDataClosedBonusBall()
{
	if (AfxGetTask())
	{
		bool bClosed = false;
		for (int i = 0; i < 5; i++)
		{
			if (m_bonusBall[i].typeId != 0 && m_bonusBall[i].index >= 0)
			{
				bClosed = true;
				break;
			}
		}
		if (!bClosed)
			return;
		if (IsLocalContent((localContentType_t)1))
		{
			if (AfxGetTask()->GetActor("GolfRule"))
				AfxGetTask()->GetActor("GolfRule")
					<< MsgObject(NULL, M983(0xb1), (int)m_bonusBall, 0, 0, 0, 0);
		}
	}
}

void FrBongdariShopDlg::EnableExitButton(bool enable)
{
	if (!(m_exitEnableTime < 4.0f) && m_pCloseBn != NULL)
		m_pCloseBn->Enable(enable);
}

void FrBongdariShopDlg::DrawBonusBall(unsigned int index)
{
	if (index < 5 && m_state > 2 && m_state < 5 &&
		m_pBonusBallModel[index] != NULL)
	{
		float base = 3.5f;
		if (m_hoverBonusBall == index)
			base = 2.9f;
		m_pBonusBallModel[index]->SetDistance((base - m_moving[index].depth) *
				1.25,
			true, true);
		m_pBonusBallModel[index]->Display(0.1f, 100.0f);
	}
}

IMPLEMENT_OBJECT(FrAboutShopDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrAboutShopDlg, FrForm)

ON_FRESH_VI("about_shop", FRCMD_INIT, FrAboutShopDlg::OnAboutShopInit)
ON_FRESH_VI("aboutviewer", FRCMD_INIT, FrAboutShopDlg::OnAboutViewerInit)
ON_FRESH_VI("index01", FRCMD_INIT, FrAboutShopDlg::OnIndex01Init)
ON_FRESH_VV("index01", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex01BnUp)
ON_FRESH_VI("index02", FRCMD_INIT, FrAboutShopDlg::OnIndex02Init)
ON_FRESH_VV("index02", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex02BnUp)
ON_FRESH_VI("index03", FRCMD_INIT, FrAboutShopDlg::OnIndex03Init)
ON_FRESH_VV("index03", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex03BnUp)
ON_FRESH_VI("index04", FRCMD_INIT, FrAboutShopDlg::OnIndex04Init)
ON_FRESH_VV("index04", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex04BnUp)
ON_FRESH_VI("index05", FRCMD_INIT, FrAboutShopDlg::OnIndex05Init)
ON_FRESH_VV("index05", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex05BnUp)
ON_FRESH_VI("index06", FRCMD_INIT, FrAboutShopDlg::OnIndex06Init)
ON_FRESH_VV("index06", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex06BnUp)
ON_FRESH_VI("index07", FRCMD_INIT, FrAboutShopDlg::OnIndex07Init)
ON_FRESH_VV("index07", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex07BnUp)
ON_FRESH_VI("index08", FRCMD_INIT, FrAboutShopDlg::OnIndex08Init)
ON_FRESH_VV("index08", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex08BnUp)
ON_FRESH_VI("index09", FRCMD_INIT, FrAboutShopDlg::OnIndex09Init)
ON_FRESH_VV("index09", FRCMD_LBUTTONUP, FrAboutShopDlg::OnIndex09BnUp)
ON_FRESH_VI("backward", FRCMD_INIT, FrAboutShopDlg::OnBackwardBnInit)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP, FrAboutShopDlg::OnBackwardBnUp)
ON_FRESH_VI("forward", FRCMD_INIT, FrAboutShopDlg::OnForwardBnInit)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrAboutShopDlg::OnForwardBnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrAboutShopDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrAboutShopDlg::OnOkBnUp)
ON_FRESH_VI("about_shop_img", FRCMD_INIT, FrAboutShopDlg::OnAboutShopImgInit)
ON_FRESH_VV("about_shop_img", FRCMD_OWNERDRAW,
	FrAboutShopDlg::OnAboutShopImgOwnerDraw)
ON_FRESH_VI("about_shop_img2", FRCMD_INIT, FrAboutShopDlg::OnAboutShopImg2Init)
ON_FRESH_VV("about_shop_img2", FRCMD_OWNERDRAW,
	FrAboutShopDlg::OnAboutShopImg2OwnerDraw)

END_FRESH_MSGMAP()

FrAboutShopDlg::FrAboutShopDlg()
{
	for (int i = 0; i < 9; i++)
		m_pIndex[i] = NULL;
	m_curIndex = 0;
	m_pBackward = NULL;
	m_pForward = NULL;
}

FrAboutShopDlg::~FrAboutShopDlg()
{
}

void FrAboutShopDlg::OnAboutShopInit(int param)
{
}

void FrAboutShopDlg::OnAboutViewerInit(int param)
{
	m_pViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex01Init(int param)
{
	m_pIndex[0] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex01BnUp()
{
	SetImgAboutIndex(0);
}

void FrAboutShopDlg::OnIndex02Init(int param)
{
	m_pIndex[1] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex02BnUp()
{
	SetImgAboutIndex(1);
}

void FrAboutShopDlg::OnIndex03Init(int param)
{
	m_pIndex[2] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex03BnUp()
{
	SetImgAboutIndex(2);
}

void FrAboutShopDlg::OnIndex04Init(int param)
{
	m_pIndex[3] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex04BnUp()
{
	SetImgAboutIndex(3);
}

void FrAboutShopDlg::OnIndex05Init(int param)
{
	m_pIndex[4] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex05BnUp()
{
	SetImgAboutIndex(4);
}

void FrAboutShopDlg::OnIndex06Init(int param)
{
	m_pIndex[5] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex06BnUp()
{
	SetImgAboutIndex(5);
}

void FrAboutShopDlg::OnIndex07Init(int param)
{
	m_pIndex[6] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex07BnUp()
{
	SetImgAboutIndex(6);
}

void FrAboutShopDlg::OnIndex08Init(int param)
{
	m_pIndex[7] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex08BnUp()
{
	SetImgAboutIndex(7);
}

void FrAboutShopDlg::OnIndex09Init(int param)
{
	m_pIndex[8] = DYNAMIC_CAST(FrTextButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnIndex09BnUp()
{
	SetImgAboutIndex(8);
}

void FrAboutShopDlg::OnBackwardBnInit(int param)
{
	m_pBackward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pBackward->UseDblClick(false); // 983 008b6880: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrAboutShopDlg::OnBackwardBnUp()
{
	m_curIndex--;
	SetImgAboutIndex(m_curIndex);
	m_pBackward->SetPushDelay(0);
}

void FrAboutShopDlg::OnForwardBnInit(int param)
{
	m_pForward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pForward->UseDblClick(false); // 983 008b6940: +0x44 |= 0x200 (FWS_NODBLCLICK)
}

void FrAboutShopDlg::OnForwardBnUp()
{
	m_curIndex++;
	SetImgAboutIndex(m_curIndex);
	m_pForward->SetPushDelay(0);
}

void FrAboutShopDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrAboutShopDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrAboutShopDlg::OnInit()
{
	SetImgAboutIndex(0);
	return true;
}

void FrAboutShopDlg::SetMode983(int mode)
{
	m_mode = mode;
}

void FrAboutShopDlg::OnAboutShopImgInit(int param)
{
	m_pImg = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAboutShopDlg::OnAboutShopImgOwnerDraw()
{
	if (m_mode == 2)
		m_pImg->SetBgImg("about_memorial_text_static_01");
	else
		m_pImg->SetBgImg("about_shop_text_static_01");
}

void FrAboutShopDlg::OnAboutShopImg2Init(int param)
{
	m_pImg2 = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrAboutShopDlg::OnAboutShopImg2OwnerDraw()
{
	if (m_mode == 2)
		m_pImg2->SetBgImg("about_memorial_text_static_02");
	else
		m_pImg2->SetBgImg("about_shop_text_static_02");
}

void FrAboutShopDlg::SetIndexCaptions983()
{
	if (m_mode == 2)
	{
		m_pIndex[0]->SetCaption(K2L_Compatibility("1. \xb8\xde\xb8\xf0\xb8\xae\xbe\xf3 \xbc\xa5\xc0\xcc\xb6\xf5?"));
		m_pIndex[1]->SetCaption(K2L_Compatibility("2. \xb8\xde\xb8\xf0\xb8\xae\xbe\xf3 \xc4\xda\xc0\xce"));
		m_pIndex[2]->SetCaption(K2L_Compatibility("3. \xc4\xda\xc0\xce\xc0\xc7 \xc6\xaf\xc2\xa1"));
		m_pIndex[3]->SetCaption(K2L_Compatibility("4. \xb8\xde\xb8\xf0\xb8\xae\xbe\xf3 \xb7\xb9\xba\xa7"));
		m_pIndex[4]->SetCaption(K2L_Compatibility("5. \xc8\xad\xb8\xe9\xbc\xb3\xc1\xa4 I"));
		m_pIndex[5]->SetCaption(K2L_Compatibility("6. \xc8\xad\xb8\xe9\xbc\xb3\xc1\xa4 II"));
		m_pIndex[6]->SetCaption(K2L_Compatibility("7. \xc0\xcc\xbf\xeb\xb9\xe6\xb9\xfd I"));
		m_pIndex[7]->SetCaption(K2L_Compatibility("8. \xc0\xcc\xbf\xeb\xb9\xe6\xb9\xfd II"));
		m_pIndex[8]->SetCaption(K2L_Compatibility("9. \xc0\xcc\xbf\xeb\xb9\xe6\xb9\xfd III"));
	}
}

void FrAboutShopDlg::SetImgAboutIndex(int index)
{
	// 983 008b4080: 9 abas (Bongdari/Memorial); modo 0 = nenhuma
	int count = 0;
	if (m_mode == 2 || m_mode == 1)
		count = 9;
	for (int i = 0; i < count; i++)
		m_pIndex[i]->Unselect(0);
	if (m_pBackward)
		m_pBackward->Enable(true);
	if (m_pForward)
		m_pForward->Enable(true);
	if (index <= 0)
	{
		index = 0;
		m_pIndex[0]->Select(0);
		if (m_pBackward)
			m_pBackward->Enable(false);
	}
	if (index >= count - 1)
	{
		index = count - 1;
		m_pIndex[index]->Select(0);
		if (m_pForward)
			m_pForward->Enable(false);
	}
	m_pIndex[index]->Select(0);
	m_curIndex = index;
	if (m_pViewer)
		m_pViewer->Open(MakeStr(m_mode == 2 ? "help_ms_%02d.jpg" :
											   "help_bs_%02d.jpg",
			index + 1));
}

IMPLEMENT_OBJECT(FrTotalItemDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrTotalItemDlg, FrForm)

ON_FRESH_VI("total_item", FRCMD_INIT, FrTotalItemDlg::OnTotalItemInit)
ON_FRESH_VI("totalitemlist", FRCMD_INIT, FrTotalItemDlg::OnTotalItemListInit)
ON_FRESH_VI("totalitemlist", FRCMD_OWNERDRAW,
	FrTotalItemDlg::OnTotalItemListOwnerDraw)
ON_FRESH_VV("totalitemlist", FRCMD_RBUTTONDOWN,
	FrTotalItemDlg::OnTotalItemListRBnDown)
ON_FRESH_VI("ok", FRCMD_INIT, FrTotalItemDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrTotalItemDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrTotalItemDlg::FrTotalItemDlg()
{
	C_ASSERT_VC10(offsetof(FrTotalItemDlg, m_deleteIcon) == 0x164 && offsetof(FrTotalItemDlg, m_clockIcon) == 0x17c); // E431
	m_pList = NULL;
	static char* textures[1] = { "[font_wind.jpg" };
	m_pBase = g_pFresh->GetManager()->GetBitmap("ITEMS", "shop_icon_base02");
	tagWTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	fontInfo.filename = textures;
	fontInfo.fontw = 16;
	fontInfo.fonth = 16;
	fontInfo.numPages = 1;
	fontInfo.texw = 128;
	fontInfo.texh = 64;
	fontInfo.pCharSet = "1234567890ym%-.?/:";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&fontInfo);
}

FrTotalItemDlg::~FrTotalItemDlg()
{
	m_bonusList.clear();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

// ---- run 3: FrTotalItemDlg / FrBongdarishopNoticeDlg / FrClosedBonusBallResultDlg (to be moved into source order)

void FrTotalItemDlg::OnTotalItemInit(int param)
{
}

void FrTotalItemDlg::OnTotalItemListInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_pList->UseRightButton(true);
	m_clockIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockIcon.rect = WRect(50, 35, m_clockIcon.pBitmap->Width(),
		m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "delete");
	m_deleteIcon.rect = WRect(50, 35, m_deleteIcon.pBitmap->Width(),
		m_deleteIcon.pBitmap->Height());
}

void FrTotalItemDlg::OnTotalItemListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi || !param)
		return;
	FrListItem* item = (FrListItem*)param;
	sBonusBall* ball = (sBonusBall*)item->pData;
	if (!ball->typeId)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(ball->typeId);
	if (!common)
		return;
	float ox = 0.0f;
	float oy = 0.0f;
	const Bitmap* icon = g_pFresh->GetBitmap(common->Icon);
	unsigned int type = ball->typeId >> 26;
	if (type == 2)
	{
		ox = -12.0f;
		oy = -20.0f;
	}
	if (type == 31)
	{
		ox = -2.0f;
		oy = -9.0f;
	}
	if (type == 9) // 983 008bb2a0
	{
		ox = -12.0f;
		oy = -22.0f;
	}
	if (!icon || !m_pBase)
		return;
	WRect dest(item->pos.x, item->pos.y, m_pBase->Width(), m_pBase->Height());
	gdi->DrawTexture(m_pBase, dest, 0xffffffff, 0);
	WRect iconDest(ox + item->pos.x + 18.0f, oy + item->pos.y + 11.0f,
		icon->Width(), icon->Height());
	gdi->DrawTexture(icon, iconDest, 0xffffffff, 0);
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(4);
	if (!ball->bTime)
	{
		if (ball->quantity > 1)
		{
			const char* count = MakeStr("%d", ball->quantity);
			float x = item->pos.x + 70.0f;
			for (unsigned int i = 0; i < strlen(count); ++i)
				x -= m_pFont->GetCharWidth(g_view, count[i] - '0');
			m_pFont->Print(g_view, x, item->pos.y + 45.0f, count, 0, 0xffffffff,
				0);
		}
	}
	else if (ball->period)
		gdi->DrawTexture(m_clockIcon.pBitmap, m_clockIcon.rect + item->pos,
			0xffffffff, 0);
}

void FrTotalItemDlg::OnTotalItemListRBnDown()
{
	if (!m_pList)
		return;
	FrListItem* item = m_pList->GetItemUnderCursor();
	if (item && item->pData)
		OpenInformation(((sBonusBall*)item->pData)->typeId);
}

void FrTotalItemDlg::OpenInformation(unsigned long typeId)
{
	FrWnd* wnd =
		g_pFresh->GetManager()->GetDesktop()->FindChildByName("information");
	if (wnd)
		return;
	DYNAMIC_CAST(FrForm, wnd);
	wnd = g_pFresh->GetManager()->GetDesktop()->FindChildByName(
		"information_setitem");
	if (wnd)
		return;
	DYNAMIC_CAST(FrForm, wnd);
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(typeId);
	if (!common)
		return;
	if ((common->TypeId & 0xfc000000) == 0x24000000)
	{
		FrSetItemDlg* dlg = CreateForm<FrSetItemDlg>(g_pFresh->GetManager(),
			this, "information_setitem", NULL);
		if (dlg)
		{
			dlg->SetSetItem(common->TypeId);
			dlg->Open(NULL, 3);
		}
		return;
	}
	FrForm* form =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "information", NULL);
	if (!form)
		return;
	FrArea* portrait = DYNAMIC_CAST(FrArea, form->FindChildByName("portrait"));
	if (portrait)
		portrait->SetBgImg(common->Icon);
	FrEdit* name = DYNAMIC_CAST(FrEdit, form->FindChildByName("name"));
	FrArea* level = DYNAMIC_CAST(FrArea, form->FindChildByName("level"));
	if (name && level)
	{
		name->AddText(MakeStr(K2L_Compatibility("\xc0\xcc\xb8\xa7: %s"), common->Name), false,
			true);
		name->AddText("\n", false, true);
		const char* fmt;
		if (common->IsUnderLvl)
		{
			level->SetBgImg(MakeStr("level_%03d", common->Level + 1));
			fmt = K2L_Compatibility("\xb7\xb9\xba\xa7:                \xc0\xcc\xc7\xcf");
		}
		else
		{
			level->SetBgImg(MakeStr("level_%03d", common->Level + 1));
			fmt = K2L_Compatibility("\xb7\xb9\xba\xa7:                \xc0\xcc\xbb\xf3");
		}
		name->AddText(MakeStr(fmt, common->Name), false, true);
	}
	IFF_STRUCT::sDesc* desc = ItemManager()->FindDesc(common->TypeId);
	if (desc)
		form->SetMessage(desc->Desc, false);
	form->Open(NULL, 3);
	// 983 008bb580: depois do Open esconde reset e icon_clubsetworkshop
	FrButton* reset = DYNAMIC_CAST(FrButton, form->FindChildByName("reset"));
	if (reset)
		reset->SetVisible(false);
	FrButton* club = DYNAMIC_CAST(FrButton,
		form->FindChildByName("icon_clubsetworkshop"));
	if (club)
		club->SetVisible(false);
}

void FrTotalItemDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrTotalItemDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrTotalItemDlg::OnInit()
{
	return true;
}

void FrTotalItemDlg::SetListItem(std::list<sBonusBall>* list)
{
	if (!list)
		return;
	m_bonusList.assign(list->begin(), list->end());
	if (m_bonusList.size() <= 0)
	{
		SetDesc(GetGameString(0xc352));
		return;
	}
	SetDesc(GetGameString(0xc353));
	std::list<sBonusBall>::iterator it;
	for (it = m_bonusList.begin(); it != m_bonusList.end(); it++)
		m_pList->AddItem(&(*it));
}

char* g_strIndex[] = { "1", "2", "3", "4", "5" };

IMPLEMENT_OBJECT(FrBongdarishopNoticeDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBongdarishopNoticeDlg, FrForm)

ON_FRESH_VI("bongdarishop_notice", FRCMD_INIT,
	FrBongdarishopNoticeDlg::OnBongdarishopNoticeInit)
ON_FRESH_VI("noticeviewer", FRCMD_INIT,
	FrBongdarishopNoticeDlg::OnNoticeViewerInit)
ON_FRESH_VV("noticeviewer", FRCMD_LBUTTONUP,
	FrBongdarishopNoticeDlg::OnNoticeViewerBnUp)
ON_FRESH_VI("listindex", FRCMD_INIT, FrBongdarishopNoticeDlg::OnListIndexInit)
ON_FRESH_VV("listindex", FRCMD_LBUTTONUP,
	FrBongdarishopNoticeDlg::OnListIndexLBnUp)
ON_FRESH_VI("listindex", FRCMD_OWNERDRAW,
	FrBongdarishopNoticeDlg::OnListIndexOwnerDraw)
ON_FRESH_VI("backward", FRCMD_INIT, FrBongdarishopNoticeDlg::OnBackwardBnInit)
ON_FRESH_VV("backward", FRCMD_LBUTTONUP,
	FrBongdarishopNoticeDlg::OnBackwardBnUp)
ON_FRESH_VI("forward", FRCMD_INIT, FrBongdarishopNoticeDlg::OnForwardBnInit)
ON_FRESH_VV("forward", FRCMD_LBUTTONUP, FrBongdarishopNoticeDlg::OnForwardBnUp)
ON_FRESH_VI("in", FRCMD_INIT, FrBongdarishopNoticeDlg::OnInBnInit)
ON_FRESH_VV("in", FRCMD_LBUTTONUP, FrBongdarishopNoticeDlg::OnInBnUp)
ON_FRESH_VI("out", FRCMD_INIT, FrBongdarishopNoticeDlg::OnOutBnInit)
ON_FRESH_VV("out", FRCMD_LBUTTONUP, FrBongdarishopNoticeDlg::OnOutBnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrBongdarishopNoticeDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrBongdarishopNoticeDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrBongdarishopNoticeDlg::FrBongdarishopNoticeDlg()
{
	m_pIndexList = NULL;
	m_curIndex = 0;
	m_moveLayoutTid = 0;
	m_pBackward = NULL;
	m_pForward = NULL;
	m_pViewer = NULL;
	m_pIn = NULL;
	m_pOut = NULL;
	m_pOk = NULL;
}

FrBongdarishopNoticeDlg::~FrBongdarishopNoticeDlg()
{
}

void FrBongdarishopNoticeDlg::OnBongdarishopNoticeInit(int param)
{
}

void FrBongdarishopNoticeDlg::OnNoticeViewerInit(int param)
{
	m_pViewer = DYNAMIC_CAST(FrViewer, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnNoticeViewerBnUp()
{
	if (m_curIndex < 0)
		return;
	WPoint point;
	point = g_pFresh->GetManager()->GetMousePos();
	std::list<sNoticeImage::sBn>::iterator it;
	for (it = m_images[m_curIndex].bnList.begin();
		it != m_images[m_curIndex].bnList.end(); ++it)
	{
		WPoint pos(GetRect().x, GetRect().y);
		WRect rect(pos.x + (*it).rect.x, pos.y + (*it).rect.y, (*it).rect.w,
			(*it).rect.h);
		if (rect.IsInRect(point))
		{
			m_moveLayoutTid = (*it).typeId;
			Close(FrOK, true);
			return;
		}
	}
	Close(FrNONE, true);
}

void FrBongdarishopNoticeDlg::OnListIndexInit(int param)
{
	m_pIndexList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnListIndexLBnUp()
{
	FrListItem* item = m_pIndexList->GetItemUnderCursor();
	if (item && item->pData)
	{
		m_curIndex = atoi((const char*)item->pData) - 1;
		SetImgNotice(m_curIndex);
	}
}

void FrBongdarishopNoticeDlg::OnListIndexOwnerDraw(int param)
{
	FrListItem* item = (FrListItem*)param;
	if (item && item->pData)
	{
		const char* text = (const char*)item->pData;
		FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
		if (gdi)
		{
			unsigned int page = atoi(text);
			if (m_curIndex == page - 1)
			{
				gdi->SetTextColor(0xffff0000, 0xffffffff);
				gdi->SetTextStyle(1);
			}
			else
			{
				gdi->SetTextColor(0xff000000, 0xffffffff);
				gdi->SetTextStyle(0);
			}
			gdi->Print(item->pos, 0, text);
		}
	}
}

void FrBongdarishopNoticeDlg::OnBackwardBnInit(int param)
{
	m_pBackward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnBackwardBnUp()
{
	m_curIndex--;
	SetImgNotice(m_curIndex);
	m_pBackward->SetPushDelay(0);
}

void FrBongdarishopNoticeDlg::OnForwardBnInit(int param)
{
	m_pForward = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnForwardBnUp()
{
	m_curIndex++;
	SetImgNotice(m_curIndex);
	m_pForward->SetPushDelay(0);
}

void FrBongdarishopNoticeDlg::OnInBnInit(int param)
{
	m_pIn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnInBnUp()
{
	OnFreshOkay();
}

void FrBongdarishopNoticeDlg::OnOutBnInit(int param)
{
	m_pOut = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnOutBnUp()
{
	FrForm* dlg =
		(FrForm*)g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"bongdari_shop");
	if (!dlg)
	{
		__rtti_obj = 0;
		OnFreshCancel();
		return;
	}
	dlg->Close(FrCANCEL, true);
	OnFreshCancel();
}

void FrBongdarishopNoticeDlg::OnOkBnInit(int param)
{
	m_pOk = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopNoticeDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrBongdarishopNoticeDlg::OnInit()
{
	LoadInitFile("bs_ppl.ini");
	SetImgNotice(0);
	return true;
}

void FrBongdarishopNoticeDlg::LoadInitFile(const char* filename)
{
	LoadNoticeImageFile(filename, m_images, 5, NULL);
	if (m_pBackward)
	{
		float y = m_pBackward->GetRect().y;
		m_pBackward->MoveWindow(WPoint((m_rect.w * 0.5f + m_rect.x) -
				(m_images.size() * 7.5f + 20.0f),
			y));
	}
	if (m_pForward)
	{
		float y = m_pForward->GetRect().y;
		m_pForward->MoveWindow(
			WPoint(m_rect.w * 0.5f + m_images.size() * 7.5f + m_rect.x, y));
	}
	int count = m_images.size();
	if (count > 0 && count <= 5)
	{
		for (unsigned char i = 0; i < count; i++)
			m_pIndexList->AddItem(g_strIndex[i]);
		WRect rect = m_pIndexList->GetRect();
		m_pIndexList->MoveWindow(
			WPoint((5 - m_images.size()) * 7.5f + rect.x, rect.y));
	}
}

void FrBongdarishopNoticeDlg::SetButtonType(bool type)
{
	if (m_pIn)
		m_pIn->SetVisible(type);
	if (m_pOut)
		m_pOut->SetVisible(type);
	if (m_pOk)
		m_pOk->SetVisible(!type);
}

void FrBongdarishopNoticeDlg::SetImgNotice(int index)
{
	int count = m_images.size();
	if (count <= 0)
		return;
	if (m_pBackward)
		m_pBackward->Enable(true);
	if (m_pForward)
		m_pForward->Enable(true);
	if (index <= 0)
	{
		index = 0;
		if (m_pBackward)
			m_pBackward->Enable(false);
	}
	if (index >= count - 1)
	{
		index = count - 1;
		if (m_pForward)
			m_pForward->Enable(false);
	}
	m_curIndex = index;
	if (m_pViewer)
		m_pViewer->Open(MakeStr(m_images[index].name.c_str()));
}

void FrBongdarishopNoticeDlg::PrevIndex()
{
	if (0)
		((FrBongdarishopNoticeDlg*)0)->FrBongdarishopNoticeDlg::OnInit();
	m_curIndex--;
	SetImgNotice(m_curIndex);
	m_pBackward->SetPushDelay(0);
}

void FrBongdarishopNoticeDlg::NextIndex()
{
	if (0)
		((FrBongdarishopNoticeDlg*)0)->FrBongdarishopNoticeDlg::PrevIndex();
	m_curIndex++;
	SetImgNotice(m_curIndex);
	m_pForward->SetPushDelay(0);
}

IMPLEMENT_OBJECT(FrBongdarishopRareItemDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBongdarishopRareItemDlg, FrForm)

ON_FRESH_VI("bongdarishop_rareitem", FRCMD_INIT,
	FrBongdarishopRareItemDlg::OnBongdarishopRareItemInit)
ON_FRESH_VI("rareitemicon", FRCMD_INIT,
	FrBongdarishopRareItemDlg::OnRareItemIconInit)
ON_FRESH_VI("rareitemname", FRCMD_INIT,
	FrBongdarishopRareItemDlg::OnRareItemNameInit)
ON_FRESH_VI("rareitemability", FRCMD_INIT,
	FrBongdarishopRareItemDlg::OnRareItemAbilityInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrBongdarishopRareItemDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrBongdarishopRareItemDlg::OnOkBnUp)
ON_FRESH_VI("msg02", FRCMD_INIT, FrBongdarishopRareItemDlg::OnMsg02Init983)

END_FRESH_MSGMAP()

FrBongdarishopRareItemDlg::FrBongdarishopRareItemDlg()
{
	m_pIcon = NULL;
	m_pName = NULL;
	m_pAbility = NULL;
	m_pMsg02983 = NULL;
}

FrBongdarishopRareItemDlg::~FrBongdarishopRareItemDlg()
{
}

void FrBongdarishopRareItemDlg::OnBongdarishopRareItemInit(int param)
{
}

void FrBongdarishopRareItemDlg::OnRareItemIconInit(int param)
{
	m_pIcon = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrBongdarishopRareItemDlg::OnRareItemNameInit(int param)
{
	m_pName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrBongdarishopRareItemDlg::OnRareItemAbilityInit(int param)
{
	m_pAbility = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrBongdarishopRareItemDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopRareItemDlg::OnMsg02Init983(int param) // 008b7530
{
	m_pMsg02983 = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrBongdarishopRareItemDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrBongdarishopRareItemDlg::OnInit()
{
	return true;
}

struct FrEditAlign983 : FrEdit { void SetCenterAlign() { m_align = centerAlign; } }; // 983 +0x124

void FrBongdarishopRareItemDlg::SetRareItems983(sBonusBall* balls, int count)
{
	// 983 008bbc10: msg02 = "item recebido"; 1 item = icone ITEMS + "nome   xN"; 2+ = uma linha centrada por item
	if (!balls || !m_pName)
		return;
	m_pMsg02983->SetCaption(K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb9\xde\xc0\xb8\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."));
	if (count > 1)
		((FrEditAlign983*)m_pName)->SetCenterAlign();
	m_pName->ClearLine();
	for (int i = 0; i < count; i++)
	{
		IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(balls[i].typeId);
		if (!common)
			continue;
		if (count < 2 && m_pIcon)
			m_pIcon->SetBgImg(g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon));
		m_pName->AddText(MakeStr("%s%s   x%d", "   ", common->Name, balls[i].quantity), false, true);
	}
}

void FrBongdarishopRareItemDlg::SetRareItem(sBonusBall* ball)
{
	if (!ball)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(ball->typeId);
	if (!common)
		return;
	if (m_pIcon)
	{
		switch (ball->typeId >> 26)
		{
		case 2:
			m_pIcon->SetBgImg(g_pFresh->GetManager()->GetBitmap("ITEMS_FASHION",
				common->Icon));
			break;
		default:
			m_pIcon->SetBgImg(
				g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon));
			break;
		}
	}
	if (!m_pName)
		return;
	m_pName->AddText(MakeStr(K2L_Compatibility("\\cT\\c%s\\cN\\c"), common->Name), false, true);
	short attr[5];
	short slot[5] = { 0 };
	unsigned long typeId = ball->typeId;
	switch (typeId >> 26)
	{
	case 2:
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
		if (!part)
			return;
		memcpy(attr, part->Attr, sizeof(attr));
		memcpy(slot, part->Slot, sizeof(slot));
		break;
	}
	case 8:
	{
		IFF_STRUCT::sCadItem* cadItem = ItemManager()->FindCadItem(typeId);
		if (!cadItem)
			return;
		memcpy(attr, cadItem->COM, sizeof(attr));
		break;
	}
	case 7:
	{
		IFF_STRUCT::sCaddie* caddie = ItemManager()->FindCaddie(typeId);
		if (!caddie)
			return;
		memcpy(attr, caddie->Attr, sizeof(attr));
		break;
	}
	default:
		return;
	}
	// 983 008bb940: nomes de atributo e formatos passam por K2L_Compatibility (o nosso mostrava o coreano cru)
	const char* statName = "";
	int i = 0;
	for (int n = 0; n < 5; n++, i++)
	{
		if (attr[i] || slot[i])
		{
			switch (i)
			{
			case 0:
				statName = K2L_Compatibility("\xc6\xc4\xbf\xf6");
				break;
			case 1:
				statName = K2L_Compatibility("\xc4\xc1\xc6\xae\xb7\xd1");
				break;
			case 2:
				statName = K2L_Compatibility("\xc1\xa4\xc8\xae\xb5\xb5");
				break;
			case 3:
				statName = K2L_Compatibility("\xbd\xba\xc7\xc9");
				break;
			case 4:
				statName = K2L_Compatibility("\xc4\xbf\xba\xea");
				break;
			}
			if (attr[i] > 0 || slot[i] > 0)
			{
				if (attr[i] == 0)
					m_pAbility->AddText(
						MakeStr(
							K2L_Compatibility("\\c0xff0000ff\\c%s\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
							statName, slot[i]),
						false, true);
				else if (slot[i] == 0)
					m_pAbility->AddText(
						MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c",
							statName, attr[i]),
						false, true);
				else
					m_pAbility->AddText(
						MakeStr(
							K2L_Compatibility("\\c0xff0000ff\\c%s%+d/\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
							statName, attr[i], slot[i]),
						false, true);
			}
		}
	}
}

IMPLEMENT_OBJECT(FrBongdarishopWarningDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrBongdarishopWarningDlg, FrForm)

ON_FRESH_VI("bongdarishop_warning", FRCMD_INIT,
	FrBongdarishopWarningDlg::OnBongdarishopWarningInit)
ON_FRESH_VI("warning_back", FRCMD_INIT,
	FrBongdarishopWarningDlg::OnWarningBackInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrBongdarishopWarningDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrBongdarishopWarningDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrBongdarishopWarningDlg::FrBongdarishopWarningDlg()
	: m_pBack(NULL), m_price(0), m_type(0)
{
	static char* textures[1] = { "[font_bong.jpg" };
	tagWTITLEFONT fontInfo;
	memset(&fontInfo, 0, sizeof(fontInfo));
	fontInfo.filename = textures;
	fontInfo.fontw = 9;
	fontInfo.fonth = 16;
	fontInfo.numPages = 1;
	fontInfo.texw = 128;
	fontInfo.texh = 32;
	fontInfo.pCharSet = "1234567890,-";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&fontInfo);
}

FrBongdarishopWarningDlg::~FrBongdarishopWarningDlg()
{
	if (m_pFont)
		m_pFont->Erase();
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

void FrBongdarishopWarningDlg::OnBongdarishopWarningInit(int param)
{
}

void FrBongdarishopWarningDlg::OnWarningBackInit(int param)
{
	m_pBack = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrBongdarishopWarningDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrBongdarishopWarningDlg::OnOkBnUp()
{
	FrForm* dlg =
		(FrForm*)g_pFresh->GetManager()->GetDesktop()->FindChildByName(
			"bongdari_shop");
	if (!dlg)
	{
		__rtti_obj = 0;
		OnFreshOkay();
		return;
	}
	dlg->Close(FrCANCEL, true);
	OnFreshOkay();
}

bool FrBongdarishopWarningDlg::OnInit()
{
	return true;
}

void FrBongdarishopWarningDlg::SetImgWarningBack(int type, int price)
{
	if (!m_pBack)
		return;
	const Bitmap* bitmap;
	if (type == 0)
		bitmap = g_pFresh->GetBitmap("warning_back");
	else if (type == 1)
		bitmap = g_pFresh->GetBitmap("warning_back_2");
	else if (type == 2)
	{
		m_price = price;
		m_type = 2;
		bitmap = g_pFresh->GetBitmap("warning_back_bonus");
	}
	else
		return;
	m_pBack->SetBgImg(bitmap);
}

IMPLEMENT_OBJECT(FrClosedBonusBallResultDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrClosedBonusBallResultDlg, FrForm)

ON_FRESH_VI("closed_bonusball", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallResultInit)
ON_FRESH_VI("closedbonusballlist", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallListInit)
ON_FRESH_VI("closedbonusballlist", FRCMD_OWNERDRAW,
	FrClosedBonusBallResultDlg::OnClosedBonusBallListOwnerDraw)
ON_FRESH_VI("closedbonusballname01", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallName01Init)
ON_FRESH_VI("closedbonusballability01", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallAbility01Init)
ON_FRESH_VI("closedbonusballname02", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallName02Init)
ON_FRESH_VI("closedbonusballability02", FRCMD_INIT,
	FrClosedBonusBallResultDlg::OnClosedBonusBallAbility02Init)
ON_FRESH_VI("ok", FRCMD_INIT, FrClosedBonusBallResultDlg::OnOkBnInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrClosedBonusBallResultDlg::OnOkBnUp)

END_FRESH_MSGMAP()

FrClosedBonusBallResultDlg::FrClosedBonusBallResultDlg()
	: m_pList(NULL)
{
	m_pName[0] = NULL;
	m_pName[1] = NULL;
	m_pAbility[0] = NULL;
	m_pAbility[1] = NULL;
	C_ASSERT_VC10(offsetof(FrClosedBonusBallResultDlg, m_deleteIcon) == 0x170 && offsetof(FrClosedBonusBallResultDlg, m_clockIcon) == 0x188 &&
		offsetof(FrClosedBonusBallResultDlg, m_bonusBall) == 0x1e8); // E431
	m_curTopRow = -1;
	memset(m_bonusBall, 0, sizeof(m_bonusBall));
}

FrClosedBonusBallResultDlg::~FrClosedBonusBallResultDlg()
{
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallResultInit(int param)
{
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallListInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	m_clockIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "clock");
	m_clockIcon.rect = WRect(50, 35, m_clockIcon.pBitmap->Width(),
		m_clockIcon.pBitmap->Height());
	m_deleteIcon.pBitmap = g_pFresh->GetManager()->GetBitmap("ICONS", "delete");
	m_deleteIcon.rect = WRect(50, 35, m_deleteIcon.pBitmap->Width(),
		m_deleteIcon.pBitmap->Height());
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallListOwnerDraw(int param)
{
	FrGraphicInterface* gdi = g_pFresh->GetManager()->GetGDI();
	if (!gdi || !param)
		return;
	FrListItem* item = (FrListItem*)param;
	sBonusBall* ball = (sBonusBall*)item->pData;
	if (!ball->typeId)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(ball->typeId);
	if (!common)
		return;
	float ox = 0.0f;
	float oy = 0.0f;
	const Bitmap* icon =
		g_pFresh->GetManager()->GetBitmap("ITEMS", common->Icon);
	if (!icon)
	{
		icon = g_pFresh->GetManager()->GetBitmap("ITEMS_FASHION", common->Icon);
		ox = -12.0f;
		oy = -20.0f;
		if (!icon)
			return;
	}
	WRect dest(ox + item->pos.x + 18.0f, oy + item->pos.y + 11.0f,
		icon->Width(), icon->Height());
	gdi->DrawTexture(icon, dest, 0xffffffff, 0);
	gdi->SetTextColor(0xff000000, 0xffffffff);
	gdi->SetTextStyle(4);
	if (ball->bTime && ball->period)
		gdi->DrawTexture(m_clockIcon.pBitmap, m_clockIcon.rect + item->pos,
			0xffffffff, 0);
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallName01Init(int param)
{
	m_pName[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallAbility01Init(int param)
{
	m_pAbility[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallName02Init(int param)
{
	m_pName[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrClosedBonusBallResultDlg::OnClosedBonusBallAbility02Init(int param)
{
	m_pAbility[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrClosedBonusBallResultDlg::OnOkBnInit(int param)
{
	DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrClosedBonusBallResultDlg::OnOkBnUp()
{
	OnFreshOkay();
}

bool FrClosedBonusBallResultDlg::OnInit()
{
	return true;
}

void FrClosedBonusBallResultDlg::OnProc(const float deltaTime)
{
	if (!m_pList)
		return;
	FrScrollBar* scrollBar = ((FrListBoxScroll*)m_pList)->m_pScrBar;
	if (!scrollBar)
		return;
	int top = scrollBar->GetCurTopRow_Int();
	if (m_curTopRow == top)
		return;
	m_curTopRow = top;
	SetEditDataClosedBonusBall(0, top);
	if (m_pList->m_itemList.size() > 1)
		SetEditDataClosedBonusBall(1, m_curTopRow + 1);
}

void FrClosedBonusBallResultDlg::SetClosedBonusBall(sBonusBall* ball)
{
	if (ball)
	{
		int i = 0;
		while (!ball[i].typeId || ball[i].index < 0)
		{
			if (++i >= 5)
			{
				SetDesc(GetGameString(0xc352));
				OnFreshOkay();
				return;
			}
		}
		SetDesc(GetGameString(0xc354));
		memcpy(m_bonusBall, ball, sizeof(m_bonusBall));
		for (i = 0; i < 5; i++)
		{
			if (m_bonusBall[i].typeId && m_bonusBall[i].index >= 0)
				m_pList->AddItem(&m_bonusBall[i]);
		}
	}
}

void FrClosedBonusBallResultDlg::SetEditDataClosedBonusBall(int index, int row)
{
	if (!m_pList)
		return;
	int i = 0;
	FrListBox::ITEM_LIST::iterator it;
	for (it = m_pList->m_itemList.begin();; it++)
	{
		if (it == m_pList->m_itemList.end())
			return;
		if (row == i)
			break;
		i++;
	}
	FrListItem* item = *it;
	if (!item)
		return;
	sBonusBall* ball = (sBonusBall*)item->pData;
	if (!ball)
		return;
	IFF_ITEM_COMMON* common = ItemManager()->FindCommonItem(ball->typeId);
	if (!common || !m_pName[index] || !m_pAbility[index])
		return;
	m_pName[index]->ClearLine();
	m_pAbility[index]->ClearLine();
	m_pName[index]->AddText(MakeStr(K2L_Compatibility("%s"), common->Name), false, true); // 983 008bd655
	short attr[5];
	short slot[5] = { 0 };
	unsigned long typeId = ball->typeId;
	bool bAbility = false;
	switch (typeId >> 26)
	{
	case 2:
	{
		IFF_STRUCT::sPart* part = ItemManager()->FindPart(typeId);
		if (part)
		{
			memcpy(attr, part->Attr, sizeof(attr));
			memcpy(slot, part->Slot, sizeof(slot));
			bAbility = true;
		}
		break;
	}
	case 7:
	{
		IFF_STRUCT::sCaddie* caddie = ItemManager()->FindCaddie(typeId);
		if (caddie)
		{
			memcpy(attr, caddie->Attr, sizeof(attr));
			bAbility = true;
		}
		break;
	}
	case 8:
	{
		IFF_STRUCT::sCadItem* cadItem = ItemManager()->FindCadItem(typeId);
		if (cadItem)
		{
			memcpy(attr, cadItem->COM, sizeof(attr));
			bAbility = true;
		}
		break;
	}
	}
	if (bAbility)
	{
		const char* statName = "";
		int j = 0;
		for (int n = 0; n < 5; n++, j++)
		{
			if (attr[j] || slot[j])
			{
				switch (j)
				{
				case 0:
					statName = "\xc6\xc4\xbf\xf6";
					break;
				case 1:
					statName = "\xc4\xc1\xc6\xae\xb7\xd1";
					break;
				case 2:
					statName = "\xc1\xa4\xc8\xae\xb5\xb5";
					break;
				case 3:
					statName = "\xbd\xba\xc7\xc9";
					break;
				case 4:
					statName = "\xc4\xbf\xba\xea";
					break;
				}
				// 983 008bd822: nome e os formatos com "\xbd\xbd\xb7\xd4" passam pelo K2L ("%s%+d" nao)
				statName = K2L_Compatibility(statName);
				if (attr[j] > 0 || slot[j] > 0)
				{
					if (attr[j] == 0)
						m_pAbility[index]->AddText(
							MakeStr(K2L_Compatibility(
								"\\c0xff0000ff\\c%s\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
								statName, slot[j]),
							false, true);
					else if (slot[j] == 0)
						m_pAbility[index]->AddText(
							MakeStr("\\c0xff0000ff\\c%s%+d\\c0xff000000\\c",
								statName, attr[j]),
							false, true);
					else
						m_pAbility[index]->AddText(
							MakeStr(K2L_Compatibility(
								"\\c0xff0000ff\\c%s%+d/\xbd\xbd\xb7\xd4%+d\\c0xff000000\\c"),
								statName, attr[j], slot[j]),
							false, true);
				}
			}
		}
	}
	else
		m_pAbility[index]->AddText(MakeStr(K2L_Compatibility("%d\xb0\xb3"), ball->quantity), false,
			true);
}
