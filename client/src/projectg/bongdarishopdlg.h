#pragma once
#include "vc10_layout.h"

#include <stddef.h>
#include <list>
#include <vector>
#include "frform.h"
#include "noticeimage.h"
#include "fx.h"

class FrArea;
class FrButton;
class FrEdit;
class FrStatic;
class FrListBox;
class FrTextButton;
class WOverlay;
class FrViewer;
class WTitleFont;
class CExhibition;
class CSimpleUI;

struct sBonusBall
{
	int index;
	unsigned long typeId;
	int itemGuid983;   // +0x08 guid do item no inventario (mesmo registro do servidor que o scratch le em +8); a Papel Shop nao le
	int quantity;
	char rarity983;    // +0x10 0 = pang, 1 = cookie, 2 = rara (escolhe o som de item obtido e abre a janela rara no 983)
	unsigned char bTime;
	short period;
};

class FrBongdarishopRareItemDlg;

class FrBongdariShopDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrBongdariShopDlg)
	void SetUsingBonusBall(bool b) { m_bUsingBonusBall = b; }

protected:
	void OnBongdariShopInit(int);
	void OnCloseBnUp();
	virtual void OnCancel();
	bool OnAboutShopDlgResult(int, FrForm*);
	bool OnTotalItemDlgResult983(int, FrForm*); // 983 008b3500
	bool OnBongdarishopRareItemDlgResult(int, FrForm*);
	bool OnBongdariRareDlgResult983(int, FrForm*);
	bool OnBongdarishopWarningDlgResult(int, FrForm*);

public:
	void SetBonusBall(int, sBonusBall*);
	void SetDataFee(int);

protected:
	void SetPosMovingClickText(float);
	void DeletePetBonusBall();
	void SetStartAnimationBongdari();
	void InitRandomDataItemSilhouette();
	void ShuffleRandomDataItemSilhouette();
	unsigned long SelectDifferentTypeId(int);

public:
	void EnableExitButton(bool);

protected:
	void OnAboutShopBnInit(int);
	void OnTotalItemBnInit(int);
	void OnBongdarishopNoticeBnInit(int);
	void OnCloseBnInit(int);
	void OnBongdariInit(int);
	void OnBonusBall01BnInit(int);
	void OnBonusBall02BnInit(int);
	void OnBonusBall03BnInit(int);
	void OnBonusBall04BnInit(int);
	void OnBonusBall05BnInit(int);

public:
	void InitBongdariShop();
	void OpenBonusBall(int);

protected:
	void DrawShadow();
	void DrawTextBongdariClick();
	void DrawTextBonusBallClick();
	void ShowBongdari();
	void ShowBonusBall(int);
	void DrawBonusBall(unsigned int);
	void OnBonusBall01OwnerDraw();
	void OnBonusBall02OwnerDraw();
	void OnBonusBall03OwnerDraw();
	void OnBonusBall04OwnerDraw();
	void OnBonusBall05OwnerDraw();
	virtual bool OnLButtonDown(WPoint const&);
	void OpenAboutShopDlg();
	void OpenBongdarishopWarningDlg(int, int);
	void OpenClosedBonusBallResultDlg();
	bool OnBongdarishopNoticeDlgResult(int, FrForm*);

public:
	void CreateBonusBall(WVector);
	void UpdateUsableTimes();
	void ShowDiscountUsableTimes();

protected:
	void DisplayItemSilhouette();
	void DrawUserInfo();
	void SetOpeningBonusBallInfo(sBonusBall*);
	void SetOpeningBonusBallInfo983(sBonusBall*, int kind); // 983 008bacd0
	void SetRandomDataItemSilhouette();
	void SendStateBongdariShop(bool);
	void SendReBuildMyInventory();
	void SendDataClosedBonusBall();
	void OnAboutShopBnUp();
	void OnBongdariOwnerDraw();
	// 983 (msgmap em 00e27140): botoes/lista do modo Memorial/combo; nomes meus
	void OnBackSelectBnInit(int);   // 008b52c0
	void OnBackSelectBnUp();        // 008b5330
	void OnAutoStartBnInit(int);    // 008b7b40
	void OnAutoStartBnUp();         // 008b7c00
	void OnNormalPlayBnInit(int);   // 008b7c80
	void SetModeMessage983();
	void EndPlay983();                                       // 983 008c00a0
	void RefreshTotalItem983(std::list<sBonusBall>*);        // 983 008bf5e0
	void OnNormalPlayBnUp();        // 008b4780
	void OnComboPlayBnInit(int);    // 008b7d10
	void OnComboPlayBnUp();         // 008b47d0
	void OnCoinListInit(int);       // 008b80d0
	void OnCoinListOwnerDraw(int);  // 008b8150
	void OnCoinListUp();            // 008b8420
	void OnStartBnInit(int);        // 008b8470
	void OnStartBnUp();             // 008be000
	void OnSelectBongBnInit(int);   // 008b84e0
	void OnSelectBongBnUp();        // 008bbf50
	void OnSelectMemBnInit(int);    // 008b8550
	void OnSelectMemBnUp();         // 008bc0f0
	void OnAgainBnInit(int);        // 008b85c0
	void OnAgainBnUp();             // 008be110
	void OnGoCoinBnInit(int);       // 008b8630
	void OnGoCoinBnUp();            // 008be170
	void FillCoinList983();         // 008bdd20
	void ClearCoinListData983();    // 008bdfb0
	void ResetCoinSelect983();      // 008bde90
	void SetAutoPlayImg983(int);    // 008b4670
	virtual bool OnLButtonUp(WPoint const&);
	void OpenBongdarishopRareItemDlg(int);

public:
	// 983: modo Memorial/combo (helpers 008b3df0..008b9630, 008bd9c0..008bdd20); nomes meus
	void SetButtonStatus983();        // 008b3df0
	void LoadMemorialOverlays983();   // 008b4910
	void CreateCoinFont983();         // 008b4c00
	void SetHitRects983();            // 008b7e90
	void CreateGacha983();            // 008b7f30
	void DrawBgMode1983();            // 008b86a0
	void DrawPointSlot983();          // 008b8770
	void DrawMemorialBack983();       // 008b8800
	void DrawMemorialShopBg983();     // 008b88b0
	void DrawMemorialCoinList983();   // 008b8940
	void DrawMemorialExplan983();     // 008b89e0
	void DrawMemorialPointSlot983();  // 008b8af0
	void DrawMemorialDrop983(int);    // 008b8b80
	void DrawRareMask983();           // 008b8c50
	void DrawBongdariSelect983();     // 008b8ce0
	void DrawMemorialSelect983();     // 008b8d70
	void DrawMemorialAgain983();      // 008b8e00
	void DrawMemorialMain983();       // 008b90f0
	void DrawMemorialPoints983();     // 008b9380
	void ProcessMemorial983(float deltaTime);
	void DisplayGacha983(float);      // 008b9520
	void DisplayFxBox983();           // 008b9630
	void LoadMemorialConfig983();     // 008bd9c0
	void InitCouponBongdariShop();
	void SetBongdariShop();
	FrBongdariShopDlg();

protected:
	void AddListTotalItem(sBonusBall*);
	void ControlNoticeDlg();
	void UpdateListMyItem(sBonusBall*);
	void OpenTotalItemDlg();

public:
	void Clear();

protected:
	void Processing(float);

public:
	virtual ~FrBongdariShopDlg();

protected:
	void OnTotalItemBnUp();
	virtual void OnProc(const float);
	void OpenBongdarishopNoticeDlg(bool);
	void OnBongdarishopNoticeBnUp();
	virtual bool OnInit();

public:
	// size 0x34 -- FrBongdariShopDlg::sMovingData (has an (empty) ctor @ 00652e60)
	struct sMovingData
	{
		sMovingData();
		WVector
			velocity;   // +0x00 (x,y,z); x,y integrated into pos in Processing
		WVector target; // +0x0c landing point (y = floor in Processing bounce)
		WVector
			pos; // +0x18 screen pos; pos.x <= 0 == free slot (CreateBonusBall)
		float
			depth; // +0x24 used as (3.5 - depth) scale in ShowBonusBall/DrawBonusBall
		int bounceCount; // +0x28 Processing: < 3 bounces
		char
			state; // +0x2c 0 none, 1 falling, 2 landed/clickable, 3 opening, 4 opened
		float rotSpeed; // +0x30 rand-based spin speed (CreateBonusBall)
	};

	// size 0x14 -- name invented (no PDB evidence); 9 item silhouettes on the board
	struct sItemSilhouette
	{
		int bonusBallIndex;    // +0x00 index into m_bonusBall[], -1 = filler
		const Bitmap* pBitmap; // +0x04 default ITEMS/item0_13
		unsigned long typeId;  // +0x08
		char
			grade; // +0x0c 0 normal, 1 cookie, 2 rare (copied from sBonusBall+0x10)
		char state; // +0x0d 0 hidden, 1 revealed, 2 opened
					// +0x0e 2 bytes padding
		float
			emotiTime; // +0x10 init 10.0f; emoticon frame timer (DisplayItemSilhouette/Processing)
	};

protected:
	FrListBox* m_pCoinList983;        // 983 +0x110 "coinlist" (flag de estilo |= 0x200)
	FrButton* m_pStartBn983;          // 983 +0x114 "start"
	FrButton* m_pSelectBongBn983;     // 983 +0x118 "select_bongdari_shop"
	FrButton* m_pSelectMemBn983;      // 983 +0x11c "select_memorial_shop"
	FrButton* m_pAboutBn983;          // 983 +0x120 "aboutshop"
	FrButton* m_pNoticeBn983;         // 983 +0x124 "bongdarishopnotice"
	FrButton* m_pAgainBn983;          // 983 +0x128 "once_again"
	FrButton* m_pGoCoinBn983;         // 983 +0x12c "go_coin_select_state"
	FrButton* m_pBackSelectBn983;     // 983 +0x130 "back_select_gacha_stage"
	int m_state; // 983 +0x134 (645: +0x110) shop state machine 0..8 (Processing switch)
	sItemSilhouette m_silhouette[9];    // +0x114 .. 0x1c8
	sBonusBall m_bonusBall[5];          // +0x1c8 .. 0x22c
	int m_bonusBallCount;               // +0x22c (SetBonusBall param_1, < 6)
	CExhibition* m_pBonusBallModel[5];  // +0x230 .. 0x244
	CExhibition* m_pBongdariModel;      // +0x244
	FrButton* m_pCloseBn;               // +0x248
	FrArea* m_pBongdari;                // +0x24c
	FrArea* m_pBonusBall[5];            // +0x250 .. 0x264
	const Bitmap* m_pDefaultSilhouette; // +0x264 ITEMS/item0_13
	const Bitmap* m_pShadow;            // +0x268 BONGDARI_SHOP/shadow
	const Bitmap*
		m_pTextBongdariClick; // +0x26c BONGDARI_SHOP/text_bongdari_click
	const Bitmap*
		m_pTextBonusBallClick; // +0x270 BONGDARI_SHOP/text_bonusball_click
	const Bitmap* m_pTextFee
		[2]; // 645 +0x274 [0]=text_fee_pang, [1]=+0x278 text_fee_coupon (indexed by m_feeType)
	int m_textFee10Combo983; // 983 +0x2a0: Bitmap* "text_fee_10combo_pang" (novo no 983)
	float
		m_clickTextOffset; // +0x27c bobbing offset 0..5 (SetPosMovingClickText)
	char m_clickTextDir;   // +0x280 0 = up, 1 = down
	bool m_bBongdariPressed; // +0x281
	bool m_bUsingBonusBall;  // +0x282 SetUsingBonusBall
	bool m_bAnimating;       // +0x283 SetStartAnimationBongdari
	WRect m_cursorArea;   // +0x284 {4,50,607,298}, offset by form pos in OnInit
	int m_hoverBonusBall; // +0x294 -1 = none
	int m_cursor;         // +0x298 FrWnd::SetCursor arg (0/1/2)
	WTitleFont* m_pFontBong;    // +0x29c font_bong.jpg 9x16
	WTitleFont* m_pFontWind;    // +0x2a0 font_wind.jpg 16x16
	WTitleFont* m_pFontBongRed; // +0x2a4 font_bong_red.jpg 9x16
	CSimpleUI* m_pEmoti;        // +0x2a8 "[emoti_object.png", frames "0".."7"
	sMovingData m_moving[5];    // +0x2ac .. 0x3b0
	bool m_bChildDlgOpen; // 983 +0x3d8, set by Open*Dlg, cleared in On*DlgResult
	std::list<sBonusBall>
		m_listTotalItem; // 645 +0x3b4 (12 bytes: alloc, _Myhead @0x3b8, _Mysize @0x3bc)
	std::list<sBonusBall> m_listSecond983; // 983 +0x3e8 (lista nova do 983)
	int m_couponCount; // +0x3c0 InitCouponBongdariShop sum
	short m_usableTimes
		[3]; // +0x3c4 copies of CSharedDoc+0x455/0x457/0x459 (+0x3ca 2 bytes pad)
	int m_feeType;             // +0x3cc -1 none, index into m_pTextFee
	float m_feeTextY;          // +0x3d0 scrolls -20/s
	int m_feeTextAlpha;        // +0x3d4 init 0x200, -480/s
	bool m_bShowDiscountTimes; // 983 +0x40c
	bool m_bShowDiscountCount; // 983 +0x40d
	float m_discountTextY;     // +0x3dc
	int m_discountTextAlpha;   // +0x3e0 init 0x200
	float m_stateTimer;        // +0x3e4 Processing case 7 (1 s delay)
	FrButton* m_pTotalItemBn;  // +0x3e8
	float
		m_exitEnableTime; // 645 +0x3ec init 8.0f (0x41000000); close button enabled when >= 8.0
	FrButton* m_pAutoStartBn983;      // 983 +0x424 "autostart_btn"
	bool m_bAutoPlay983;              // 983 +0x428 (liga/desliga o auto)
	char m_pad429_983[3];
	float m_float42c983;              // 983 +0x42c (zerado em SetBongdariShop)
	const Bitmap* m_pTextAutoClick983; // 983 +0x430 BONGDARI_SHOP/text_bongdari_auto_click
	int m_i434_983;                   // 983 +0x434
	int m_i438_983;                   // 983 +0x438 (9999 no ctor)
	FrButton* m_pNormalPlayBn983;     // 983 +0x43c "normal_play_btn"
	FrButton* m_pComboPlayBn983;      // 983 +0x440 "combo_play_btn"
	int m_comboMode983;               // 983 +0x444 (1 = combo/"continuous" bongdari)
	bool m_b448_983;                  // 983 +0x448
	char m_pad449_983[3];
	const Bitmap* m_pTextComboPlayClick983; // 983 +0x44c BONGDARI_SHOP/text_bongdari_combo_play_click
	int m_unused450_983;              // 983 +0x450: sem acesso no 983 (008b34d0..008c1650)
	// 983 +0x454..0x4cc: overlays (WResourceManager::GetOverlay, 008b4910)
	WOverlay* m_pOvBg983;             // +0x454 bongdari_shop_bg.tga
	WOverlay* m_pOvComboBg983;        // +0x458 bongdari_shop_combo_bg.tga
	WOverlay* m_pOvPointSlot983;      // +0x45c bongdari_shop_pointslot.tga
	WOverlay* m_pOvMemPointSlot983;   // +0x460 memorial_shop_pointslot.tga
	WOverlay* m_pOvMemBack983[5];     // +0x464 memorial_back0..4.png
	WOverlay* m_pOvMemShopBg983;      // +0x478 memorial_shop_bg.tga
	WOverlay* m_pOvMemDrop983[3];     // +0x47c memorial_drop03/02/01.png
	WOverlay* m_pOvTxtExplanBong983;  // +0x488 bongdari_shop_txt_explan.tga
	WOverlay* m_pOvTxtExplanMem983;   // +0x48c memorial_shop_txt_explan.tga
	WOverlay* m_pOvRareMask983;       // +0x490 rare_shop_mask.tga
	WOverlay* m_pOvCoinList983;       // +0x494 memorial_shop_coinlist.tga
	WOverlay* m_pOvSelect983;         // +0x498 memorial_shop_select.tga
	WOverlay* m_pOvSelectDis983;      // +0x49c memorial_shop_select_disable.tga
	WOverlay* m_pOvBongSelect983;     // +0x4a0 bongdari_shop_select.tga
	WOverlay* m_pOvBongSelectDis983;  // +0x4a4 bongdari_shop_select_disable.tga
	WOverlay* m_pOvAgainDis983;       // +0x4a8 memorial_shop_again_disable.tga
	WOverlay* m_pOvAgain983;          // +0x4ac memorial_shop_again.tga
	WOverlay* m_pOvMainDis983;        // +0x4b0 memorial_shop_main_disable.tga
	WOverlay* m_pOvMain983;           // +0x4b4 memorial_shop_main.tga
	WOverlay* m_pOvCoinSlot983[6];    // +0x4b8 normal_n, premium_n, special_n, normal_d, premium_d, special_d
	CExhibition* m_pGacha983;         // +0x4d0 "memorial_gacha" (CreateGacha983)
	float m_cfgBackX983, m_cfgBackY983;           // +0x4d4 memorial.lua memorial_ui_config
	float m_cfgPointSlotX983, m_cfgPointSlotY983; // +0x4dc
	float m_cfgPointFontX983, m_cfgPointFontY983; // +0x4e4
	float m_cfgGachaX983, m_cfgGachaY983;         // +0x4ec
	float m_cfgListFrameX983, m_cfgListFrameY983; // +0x4f4
	float m_cfgDistanceCamera983;                 // +0x4fc
	float m_cfgEffectAlpha983;                    // +0x500
	float m_cfgEffectImgX983, m_cfgEffectImgY983; // +0x504
	float m_cfgCoinIconW983, m_cfgCoinIconH983;   // +0x50c
	float m_cfgAddCoinNumX983, m_cfgAddListTextY983; // +0x514
	float m_f51c_983;                             // +0x51c
	float m_effectAlpha983;                       // +0x520 (round -> alpha do efeito do cofre)
	float m_f524_983;                             // +0x524 (3.3 em 008be110)
	float m_cfgEffectTimeFail983;                 // +0x528
	float m_cfgShowResultDlg983;                  // +0x52c
	float m_cfgDolpiniAngleX983, m_cfgDolpiniAngleY983; // +0x530
	float m_cfgDolpiniFov983, m_cfgDolpiniFootDown983;  // +0x538
	float m_f540_983;                             // +0x540 (200.0 fixo)
	float m_cfgAddBongdariX983, m_cfgAddBongdariY983;   // +0x544
	float m_cfgStartImgX983, m_cfgStartImgY983;         // +0x54c
	float m_cfgBsOverX983, m_cfgBsOverY983;             // +0x554
	float m_cfgMsOverX983, m_cfgMsOverY983;             // +0x55c
	float m_cfgShopDisableX983, m_cfgShopDisableY983;   // +0x564
	float m_f56c_983, m_f570_983;                       // +0x56c
	float m_cfgAddCloseGachaY983;                       // +0x574
	float m_cfgAddIconX983, m_cfgAddIconY983;           // +0x578

public:
	// 983 (pedido C60.2, sessao A126): resultado do pacote 0x264 (bongdari) pela msg 0x328 do CTaskMain::HandleMsgCommon.
	// O FrBongdariShopDlg do 983 tem 0xb30 bytes; os campos abaixo ficam no fim do layout do 645 ate a conversao da janela
	// (no 983: lista em 0x594, bolas em 0x5b0, resultado em 0x580, contagem em 0x588, flag em 0x591, modo em 0x618).
	void SetBongdariResult983(const std::vector<unsigned int>& results, const std::vector<unsigned long>& typeIds,
		const std::vector<int>& quantities, int count); // 008bed00
	void AddResultBalls983(const sBonusBall* balls, int count); // 008bec90

	int m_result983;                      // 983 em 0x580: results[0] com 1 bola, -1 com mais
	int m_achievementPoint983;            // 983 em 0x584 (pontos de conquista; setter 008b4f20, so o ctor zera)
	int m_resultCount983;                 // 983 em 0x588
	int m_memorialStep983;                // 983 em 0x58c: passo da animacao do Memorial (-1 parado, 0..5)
	char m_bFxBoxActive983;               // 983 em 0x590: liga o processamento do m_fxBox983
	bool m_bResultReady983;               // 983 em 0x591
	char m_pad592_983[2];
	std::list<sBonusBall> m_listResult983; // 983 em 0x594
	char m_unused5a0_983[0x10];           // 983 em 0x5a0: sem acesso no 983 (ctor so faz a lista 0x594 e memset 0x5b0)
	sBonusBall m_resultBall983[5];        // 983 em 0x5b0
	FrBongdarishopRareItemDlg* m_pRareItemDlg983; // 983 em 0x614: janela do item raro (008bcf90 grava; 008bc2a0 zera)
	int m_resultMode983;                  // 983 em 0x618: 0 nenhum, 1 resultado 0..2, 2 resultado 3..4
	CPvsFxBox m_fxBox983;                 // 983 +0x61c (0x440): efeitos 3D (Init(rm), SetCamera)
	float m_scaleTab983[24];              // 983 +0xa5c: 2 x 12 floats (DAT_00e42574..e425a0); [0xa74..0xa7c] e [0xa80..0xa88] escalados no ctor
	char m_unusedabc_983[0xaed - 0xabc];  // 983 +0xabc..0xaed: sem acesso no 983 (ctor para em 0xab8)
	bool m_bFlagAed983;                   // 983 +0xaed
	bool m_bFlagAee983;                   // 983 +0xaee
	char m_padaef_983;
	WTitleFont* m_pFontCoin983;           // 983 +0xaf0 font "1234567890,-" 9x16 (CreateCoinFont983)
	int m_shopMode983;                    // 983 +0xaf4: 0/1 = Bongdari, 2 = Memorial
	const char* m_pCoinText983;           // 983 +0xaf8 (texto do item selecionado)
	int m_i0afc_983;                      // 983 +0xafc (id do cofre escolhido)
	int m_coinCount983;                   // 983 +0xb00 (moedas)
	const char* m_coinItemName983;        // 983 +0xb04 (nome do icone do item)
	WRect m_rcHit1983;                    // 983 +0xb08 (008b7e90): area do botao Bongdari / 'again'
	WRect m_rcHit2983;                    // 983 +0xb18: area do botao Memorial / 'main'
	WTitleFont* m_pFont983;               // 983 +0xb28 font "%" 32x32

	static void CheckLayout983()
	{
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_state) == 0x134);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_pBonusBallModel) == 0x254);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_clickTextOffset) == 0x2a4);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_listTotalItem) == 0x3dc);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_couponCount) == 0x3f4);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_usableTimes) == 0x3f8);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_exitEnableTime) == 0x420);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_result983) == 0x580);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_listResult983) == 0x594);
		C_ASSERT_VC10(offsetof(FrBongdariShopDlg, m_resultMode983) == 0x618);
		C_ASSERT_VC10(sizeof(FrBongdariShopDlg) == 0xb30);
	}

private:
	DECLARE_FRESH_MSGMAP()
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbShowbonusballFunclets(); // HACK: EH funclets of the naked ShowBonusBall (nakedify)
#endif
};

class FrBongdarishopNoticeDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrBongdarishopNoticeDlg)
	unsigned long GetMoveLayoutTid() { return m_moveLayoutTid; }

protected:
	void OnBongdarishopNoticeInit(int);
	void OnInBnUp();
	void OnOkBnUp();

public:
	void SetButtonType(bool);

protected:
	void OnNoticeViewerInit(int);
	void OnListIndexInit(int);
	void OnListIndexOwnerDraw(int);
	void OnBackwardBnInit(int);
	void OnForwardBnInit(int);
	void OnInBnInit(int);
	void OnOutBnInit(int);
	void OnOutBnUp();
	void OnOkBnInit(int);

public:
	void LoadInitFile(char const*);

protected:
	void OnNoticeViewerBnUp();

public:
	void SetImgNotice(int);

protected:
	void OnListIndexLBnUp();
	void OnBackwardBnUp();
	void OnForwardBnUp();
	virtual bool OnInit();

public:
	void PrevIndex();
	void NextIndex();
	FrBongdarishopNoticeDlg();
	virtual ~FrBongdarishopNoticeDlg();

protected:
	std::vector<sNoticeImage> m_images;
	FrListBox* m_pIndexList;
	int m_curIndex;
	unsigned long m_moveLayoutTid;
	FrButton* m_pBackward;
	FrButton* m_pForward;
	FrViewer* m_pViewer;
	FrButton* m_pIn;
	FrButton* m_pOut;
	FrButton* m_pOk;
	int m_pad983_144; // E406: 983 nao le nem escreve +0x144 (ctor 008c1340 zera 0x110..0x140; handlers/LoadInitFile 008b7100/SetImgNotice 008bb870 sem 0x144; new 0x150 em 008c147a)

private:
	DECLARE_FRESH_MSGMAP()
};

class FrAboutShopDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrAboutShopDlg)
	FrAboutShopDlg();
	virtual ~FrAboutShopDlg();

protected:
	void OnAboutShopInit(int);
	void OnOkBnUp();

public:
	void SetImgAboutIndex(int);

protected:
	void OnAboutViewerInit(int);
	void OnIndex01Init(int);
	void OnIndex01BnUp();
	void OnIndex02Init(int);
	void OnIndex02BnUp();
	void OnIndex03Init(int);
	void OnIndex03BnUp();
	void OnIndex04Init(int);
	void OnIndex04BnUp();
	void OnIndex05Init(int);
	void OnIndex05BnUp();
	void OnIndex06Init(int);
	void OnIndex06BnUp();
	void OnIndex07Init(int);
	void OnIndex07BnUp();
	void OnIndex08Init(int);
	void OnIndex08BnUp();
	void OnIndex09Init(int);
	void OnIndex09BnUp();
	void OnBackwardBnInit(int);
	void OnBackwardBnUp();
	void OnForwardBnInit(int);
	void OnForwardBnUp();
	void OnOkBnInit(int);
	void OnAboutShopImgInit(int);
	void OnAboutShopImgOwnerDraw();
	void OnAboutShopImg2Init(int);
	void OnAboutShopImg2OwnerDraw();
	virtual bool OnInit();

public:
	void SetMode983(int mode); // 983: ShopMode do pai (+0xaf4): 1 = Bongdari, 2 = Memorial
	void SetIndexCaptions983(); // 983 008b3fa0

protected:
	FrTextButton* m_pIndex[9];
	int m_curIndex;
	FrButton* m_pBackward;
	FrButton* m_pForward;
	FrViewer* m_pViewer;
	FrArea* m_pImg;   // +0x144 about_shop_img
	FrArea* m_pImg2;  // +0x148 about_shop_img2
	int m_mode;       // +0x14c

private:
	DECLARE_FRESH_MSGMAP()
};

class FrTotalItemDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrTotalItemDlg)
protected:
	void OnTotalItemInit(int);
	void OnOkBnUp();
	virtual bool OnInit();
	void OnTotalItemListInit(int);
	void OnOkBnInit(int);
	void OnTotalItemListOwnerDraw(int);
	void OpenInformation(unsigned long);
	void OnTotalItemListRBnDown();

public:
	virtual ~FrTotalItemDlg();
	FrTotalItemDlg();
	void SetListItem(std::list<sBonusBall>* list);

	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

protected:
	FrListBox* m_pList;
	const Bitmap* m_pBase;
	WTitleFont* m_pFont;
	// E431: 983 (ctor 008be620, msgmap 00e270a8, owner draw 008bb2a0, dtor 008be790) so usa delete +0x164 e clock +0x17c
	MiniBtn m_pad983_11c[3];
	MiniBtn m_deleteIcon;
	MiniBtn m_clockIcon;
	MiniBtn m_pad983_194[3];
	std::list<sBonusBall> m_bonusList;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrBongdarishopRareItemDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrBongdarishopRareItemDlg)
	FrBongdarishopRareItemDlg();
	virtual ~FrBongdarishopRareItemDlg();

protected:
	void OnBongdarishopRareItemInit(int);
	void OnOkBnUp();
	virtual bool OnInit();
	void OnRareItemIconInit(int);
	void OnRareItemNameInit(int);
	void OnRareItemAbilityInit(int);
	void OnOkBnInit(int);
	void OnMsg02Init983(int);

public:
	void SetRareItem(sBonusBall*);
	void SetRareItems983(sBonusBall* balls, int count); // 983 008bbc10 (+0x580 == -1)

protected:
	FrArea* m_pIcon;
	FrEdit* m_pName;
	FrEdit* m_pAbility;
	FrStatic* m_pMsg02983; // 983 +0x11c "msg02"

private:
	DECLARE_FRESH_MSGMAP()
};

class FrBongdarishopWarningDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrBongdarishopWarningDlg)
	FrBongdarishopWarningDlg();
	virtual ~FrBongdarishopWarningDlg();

protected:
	void OnBongdarishopWarningInit(int);
	virtual bool OnInit();

public:
	void SetImgWarningBack(int, int);

protected:
	void OnWarningBackInit(int);
	void OnOkBnInit(int);
	void OnOkBnUp();

protected:
	FrArea* m_pBack;
	WTitleFont* m_pFont;
	int m_price;
	int m_type;

private:
	DECLARE_FRESH_MSGMAP()
};

class FrClosedBonusBallResultDlg : public FrForm
{
public:
	DECLARE_OBJECT(FrClosedBonusBallResultDlg)
	virtual ~FrClosedBonusBallResultDlg();

protected:
	void OnClosedBonusBallResultInit(int);
	void OnOkBnUp();
	virtual bool OnInit();

public:
	FrClosedBonusBallResultDlg();

protected:
	void OnClosedBonusBallListInit(int);
	void OnClosedBonusBallName01Init(int);
	void OnClosedBonusBallAbility01Init(int);
	void OnClosedBonusBallName02Init(int);
	void OnClosedBonusBallAbility02Init(int);
	void OnOkBnInit(int);

public:
	void SetClosedBonusBall(sBonusBall*);

protected:
	void OnClosedBonusBallListOwnerDraw(int);

public:
	void SetEditDataClosedBonusBall(int, int);

protected:
	virtual void OnProc(const float);

public:
	struct MiniBtn
	{
		WRect rect;
		const Bitmap* pBitmap;
		int index;
	};

protected:
	FrListBox* m_pList;
	FrEdit* m_pName[2];
	FrEdit* m_pAbility[2];
	int m_curTopRow;
	// E431: 983 (ctor 008bbd40, msgmap 00e270dc, vtable 00d366ec) so usa delete +0x170 e clock +0x188
	MiniBtn m_pad983_128[3];
	MiniBtn m_deleteIcon;
	MiniBtn m_clockIcon;
	MiniBtn m_pad983_1a0[3];
	sBonusBall m_bonusBall[5];

private:
	DECLARE_FRESH_MSGMAP()
};
