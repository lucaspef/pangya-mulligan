#pragma once

#include <string>
#include <vector>

#include "actor.h"

enum eBarButton;
enum PANGYA_ITEM_GROUP;
class CRealMyRoom_EnterSetting;
class FrViewer;
class FrArea;
class FrButton;
class FrEdit;
class FrGiftDlg;
class FrOptionDlg;
class FrKbdManualDlg;
class FrServerDlg;
class FrLoginRsDlg;
class FrRecycleDlg;
class FrTikiMagicBoxDlg;
class FrScratchDlg;
class FrOnelineReqDlg;
class FrNoticeDlg;
class FrLogoutDlg;
class FrAttendanceDlg;
class FrCardComposeDlg;
class FrAlchemyMPBreakDlg;
class FrPointShopDlg;
class FrSPCardBuffDlg;
class COfflineTradeDlg;
class FrEquipDlg;
class CPost_ReciveDlg;
class CPost_SendDlg;
class FrWnd;
class FrForm;
class FrLevelupItemForm;
class FrSubEquipBar;
class WTitleFont;
class Bitmap;
class FrGroupBox;
class FrTabButton;
class CTaskMain : public IActor, public FrCmdTarget
{
	DECLARE_OBJECT(CTaskMain)

	virtual void CloseAllDialogs();
	bool IsActiveRecycleDlg();
	bool IsActiveScratchDlg();
	virtual void OnDestroy();
	virtual void OnPreserveBack(char const*);
	virtual void OnPreLoadInit();
	virtual void OnInit();
	void CheckNaviButtons();
	virtual void OnLoad();
	virtual void OnRestorePreserved();
	void CreateMaketDlg();
	void ChangeRealMyRoom();
	void MakeRealMyRoom();
	virtual void OnProcess(float);

	CTaskMain() { }
	virtual ~CTaskMain() { }

protected:
	virtual bool OnResultSubEquipBar(int, FrForm*);
	virtual void OnUnderBar_ShopUp();
	virtual void OnUnderBar_MyRoomUp();
	virtual void OnUnderBar_CookieUp();
	virtual void OnUnderBar_GuildUp();
	// 983 (s106): BongdariShop/Ranking/Notice viraram virtuais e ha um novo entre Ranking e Notice (vtable 00d171c4, slots 21..24:
	// 00658ee0, 006590e0, 00659240, 00659290); os mapas de mensagem do 983 chamam por thunk vcall (005932e0/00593300/005932c0)
	virtual void OnUnderBar_BongdariShopUp();
	virtual void OnUnderBar_RankingUp();
	virtual void OnUnderBar_ClubUp983(); // 00659240: pagina do clube (on_client/club/login.aspx) pelo CWebPageConnector, tipo 5; nome meu
	virtual void OnUnderBar_NoticeUp();
	// 983 (s106): botoes "utab.combine" e "utab.gacha" dos mapas TOPPAGE/GHOST/FAMILY; nomes meus
	void OnUnderBar_CombineInit983(int); // 00652210 (+0x74)
	void OnUnderBar_CombineUp983(); // 00658e20: on_client/entrance.aspx, tipo 10
	void OnUnderBar_GachaInit983(int); // 00652290 (+0x70, o lugar do m_pUnderBarScratch)
	void OnUnderBar_GachaUp983(); // 00658e70: on_client/gacha/login.aspx, tipo 2
	// 983 (C40.3, s110): "ubar.achievement" (conquistas) no lugar do "ubar.itemstorage" do 645 (o 983 nao tem esse botao); o Init
	// grava no mesmo campo (+0x30, m_pUnderBarItemStorage). Nomes meus
	void OnUnderBar_AchievementInit983(int); // 00651e30
	void OnUnderBar_AchievementUp983(); // 006582f0: pacote 0x157 (online) ou msg 0x753a/7 ao "Gateway"
	// 983 (s111): "utab.point_shop" (abre a "AlchemyDlg", quebra de MP) e "utab.card_compose" (alquimia de cartas) dos mapas
	// TOPPAGE/GHOST/FAMILY; as janelas tambem abrem pelas msgs 0x2e3 / 0x324 (botoes da FrRecycleDlg). Nomes meus
	void OnUnderBar_PointShopInit983(int); // 00652520 (+0x78)
	void OnUnderBar_PointShopUp983(); // 00659340: CloseAllDialogs + OpenAlchemyDlg983
	void OnUnderBar_CardComposeInit983(int); // 00652600 (+0x6c, o lugar do m_pUnderBarMaket: o 983 nao tem "utab.maket")
	void OnUnderBar_CardComposeUp983(); // 00659600 -> OpenCardComposeDlg983
	void OpenAlchemyDlg983(); // 00655d70 (msg 0x2e3; servico 0x2_00000000)
	void OpenCardComposeDlg983(); // 00659490 (msg 0x324; servico 0x1_00000000)
	void ResetAlchemyDlg983(); // 00651080 (msg 0x2e6)
	void OpenRecycleDlg983(); // 00659540 (botao "utab.magicbox" 0065a160 e msgs 0x338/0x339)
	bool OnAlchemyDlgResult983(int, FrForm*); // 006525a0
	bool OnCardComposeDlgResult983(int, FrForm*); // 00653ae0
	// 983 (s114, pedido B106): loja de pontos do Tiki (FrPointShopDlg / FrPointShopMessageDlg da frente B); nomes meus
	void OpenPointShopDlg983(); // 00659360 (msg 0x2e0): pacote 0x127 e "PointShopDlg"
	void RefreshPointShopDlg983(); // 00651030 (msg 0x2e2)
	bool OnPointShopDlgResult983(int, FrForm*); // 00651260
	// 983 (s111): "obar.eqitem" (itens equipados) do mapa TOPPAGE (+0xcc, m_pOverBarEqItem); nomes meus
	void OnOverBar_EqItemInit983(int); // 00656700
	void OnOverBar_EqItemDown983(); // 006567e0: msg 0x81 ao ator principal (abre o "equip")
	void OnOverBar_EqItemOwnerDraw983(int); // 00659bd0
	void InitMessenger();
	void InitBongdariShop();
	void InitControls();
	void InitUnderBar();
	void OnUnderBar_TikiMagicBoxInit(int);
	void InitOverBar();
	void EnableUnderBar(bool);
	void ShowAllSubSelection();
	void CloseAllSubSelection();
	bool OnGiftDlgResult983(int, FrForm*); // 00651320
	bool OnQuitDlgResult(int, FrForm*);
	bool OnOptionDlgResult(int, FrForm*);
	bool OnAttendanceDlgResult983(int ret, FrForm*); // 983 00651270
	bool OnNotifyOneDlgResult(int, FrForm*);
	bool OnQuitNotifyResult983(int, FrForm*); // 00a62d90
	bool OnLoginRsDlgResult(int, FrForm*);
	bool OnMaketListDlgResult(int, FrForm*);
	// 983 (pedido C31, s100): resultados de diálogo novos; corpos com a C (pacotes)
	bool OnPostSendDlgResult983(int result, FrForm* pForm); // 0065c2d0: "write_post" -> 0x145
	bool OnPostReciveDlgResult983(int result, FrForm* pForm); // 00651070: m_pPostReciveDlg983 = NULL
	bool OnItemEquipResult983(int result, FrForm* pForm); // 00653be0: "equip" -> 0x20 tipo 2
	// 983 (pedido C32, s102): abridores sem método no fonte; corpos e casos (0x81, 0x2a3) com a C
	void OpenEquipDlg983(); // 00657600: msg 0x81 do HandleMsgCommon e CLobbyMain::HandleMsg (00abcc38); era o CLobbyMain::OnGameRoom_EquipItemDown do 645
	bool OnPostSendDlg2Result983(int result, FrForm* pForm); // 0065c510: resultado do "rmr_notify" (0x2a3, param 0x146) -> "write_post"
	bool OnKbdManualDlgResult(int, FrForm*);
	char const* MakeStatusString2(int, int, int);
	char const* MakeStatusString3(int, int, int);
	void SetBGMNone();
	bool CompletedMission_Result(int, FrForm*);
	unsigned long GetCategoryFlag(int);
	void CalcUIOffset(bool);
	void SetCenterOffset(FrWnd*, float, float);
	__declspec(noinline) void SetLeftOffset(FrWnd*, float);
	void OnUnderBarInit(int);
	void OnUnderTab_Init(int);
	void OnOverBar_Init(int);
	void OnUnderBar_BackInit(int);
	void OnUnderBar_FrontInit(int);
	void OnUnderBar_ItemStorageInit(int);
	void OnUnderBar_GiftInit(int);
	void OnUnderBar_CookieInit(int);
	void OnUnderBar_ExitInit(int);
	void OnUnderBar_TabToolTipInit(int);
	void OnUnderBar_TabBtnInit(int);
	void TurnOnUnderTabBar();
	void TurnOffUnderTabBar();
	void OnUnderBar_MagicBoxInit(int);
	void OnUnderBar_ScratchInit(int);
	void OnUnderBar_BongdariShopInit(int);
	void OnUnderBar_MaketInit(int);
	void OnUnderBar_RankingInit(int);
	void OnUnderBar_GuildInit(int);
	void OnUnderBar_NoticeInit(int);
	void ShowOverAndUnderBar(bool);
	void EnableOverAndUnderBar(bool);
	void OnOverBar_Bar1Init(int);
	void OnOverBar_Bar2Init(int);
	void OnOverBar_Bar3Init(int);
	void OnOverBar_TitleInit(int);
	void OnOverBar_Frame1Init(int);
	void OnOverBar_Frame2Init(int);
	void OnOverBar_Frame3Init(int);
	void OnOverBar_TabInit(int);
	void OnOverBar_TabDown();
	void OnOverBar_CharInit(int);
	void OnOverBar_CaddieInit(int);
	void OnOverBar_ClubInit(int);
	void OnOverBar_AztecInit(int);
	void OnOverBar_MascotInit(int);
	void OnOverBar_CharSelInit(int);
	void OnOverBar_CaddieSelInit(int);
	void OnOverBar_ClubSelInit(int);
	void OnOverBar_AztecSelInit(int);
	void OnOverBar_MascotSelInit(int);
	void OnOverBar_OneLineReqBtnInit(int);
	void OnOverBar_OneLineViewInit(int);
	void OnOverBar_OneLineViewOwnerDraw(int);
	void OnOverBar_OptionInit(int);
	void OnOverBar_MyInfoInit(int);
	void OnOverBar_PangCookieInit(int);
	void OnOverBar_SpecialCardInit(int);
	void OnTreasureAlarmInit(int);
	void OnMessengerBubbleInit(int);
	void OnMessengerBubbleOwnerDraw(int);
	void OnMessengerBubbleEditInit(int);
	void OnMessengerAlarmBtnUp();
	void OnMessengerGiftAlarmBtnInit(int);
	bool OnOnelineReqChkOutDlgResult(int, FrForm*);
	bool OnSpecialCardFormResult(int, FrForm*);
	float Print(float, float, char const*, float, int, float, unsigned long);
	char const* MakeStatusString(int, int, int);
	char const* MakeCardStatusString(int, int);
	void SetRandomBGM();
	void SetTimeVariableBGM();
	void SetConstantBGM(char const*, bool, bool);
	void SendRoomListExitPacket();
	bool GetBtnState(unsigned char, eBarButton);
	void OnUnderBar_ShopInit(int);
	void OnUnderBar_MyRoomInit(int);
	void OnUnderBar_GameInit(int);
	void OnUnderBar_ServerInit(int);
	void SendSubSelectionPacket(PANGYA_ITEM_GROUP);
	void OpenSubEquipBar(WPoint const&, PANGYA_ITEM_GROUP, int);
	void OnOverBar_MascotDown();
	void OnOverBar_PangCookieOwnerDraw(int);
	void OnBarHoverOn(int);
	void OnBarHoverOff(int);
	void OnBarTooltipOwnerDraw(int);
	void OnOverBar_SpecialCardUp();
	void OnOverBar_MessengerInit(int);
	void OnOverBar_MessengerUp();
	void OnTreasureAlarmUp();
	void OnMessengerAlarmBtnInit(int);
	bool OnWaitDlgResult(int, FrForm*);
	bool OnServerDlgResult(int, FrForm*);
	bool OnRecycleDlgResult(int, FrForm*);
	bool OnTikiMagicBoxDlgResult(int, FrForm*);
	bool OnBongdariDlgResult(int, FrForm*);
	bool OnScratchDlgResult(int, FrForm*);
	bool OnOnelineReqDlgResult(int, FrForm*);
	bool OnNoticeDlgResult(int, FrForm*);
	void TurnOn3DBackground();
	void TurnOff3DBackground();
	void SendRoomExitPacket();
	void OpenWaitedLayout();
	void UpdateSiteInfo();
	void OpenInformation(unsigned long);
	bool OnRMR_ResultSetting(int, FrForm*);
	void OnUnderBar_BackUp();
	void OnUnderBar_FrontUp();
	void OnUnderBar_GameUp();
	void OnUnderBar_ServerUp();
	void OnUnderBar_ExitUp();
	void OnUnderBar_TabBtnUp();
	void OnUnderBar_MagicBoxUp();
	void OnUnderBar_TikiMagicBoxUp();
	void OnUnderBar_ScratchUp();
	void OnUnderBar_MaketUp();
	void ProcessOverBar();
	void OnOverBar_CharDown();
	void OnOverBar_CaddieDown();
	void OnOverBar_ClubDown();
	void OnOverBar_AztecDown();
	void OnOverBar_AztecOwnerDraw(int);
	void OnOverBar_AztecSelOwnerDraw(int);
	void OnOverBar_SiteInit(int);
	void OnOverBar_OneLineReqBtnUp();
	void OnOverBar_OptionUp();
	void OnOverBar_MyInfoUp();
	bool OnNotifyDlgResult(int, FrForm*);
	void OnUnderBar_ItemStorageUp();
	void OnUnderBar_GiftUp();
	void OnOverBar_CharOwnerDraw(int);
	void OnOverBar_CaddieOwnerDraw(int);
	void OnOverBar_ClubOwnerDraw(int);
	void OnOverBar_MascotOwnerDraw(int);
	void OnOverBar_CharSelOwnerDraw(int);
	void OnOverBar_CaddieSelOwnerDraw(int);
	void OnOverBar_ClubSelOwnerDraw(int);
	void OnOverBar_MascotSelOwnerDraw(int);
	void OnMessengerGiftAlarmBtnUp();
	bool OnLevelupFormResult(int, FrForm*);
	void HandleMsgCommon(MsgObject const&);

private:
	void NotifyServerControlMsg(int);
	void NotifyServerError983(unsigned long code); // 0065df0e + 006577c0 (msg 0x27)
	void _DrawSelection(FrViewer*, WRect&, eBarButton, FrGraphicInterface*);

	DECLARE_FRESH_MSGMAP()

protected:
	CRealMyRoom_EnterSetting* m_pRMREnterSetting;
	FrButton* m_pUnderBarBack;
	FrButton* m_pUnderBarFront;
	FrButton* m_pUnderBarShop;
	FrButton* m_pUnderBarMyRoom;
	FrButton* m_pUnderBarItemStorage;
	FrButton* m_pUnderBarGift;
	FrButton* m_pUnderBarCookie;
	FrButton* m_pUnderBarGame;
	FrButton* m_pUnderBarServer;
	FrButton* m_pUnderBarExit;
	FrButton* m_pOverBarSpecialCard;
	FrButton* m_pOverBarMessenger;
	FrButton* m_pOverBarMyInfo;
	FrButton* m_pOverBarOption;
	FrButton* m_pUnderBarMagicBox;
	FrButton* m_pUnderBarBongdariShop;
	FrButton* m_pUnderBarRanking;
	FrButton* m_pUnderBarGuild;
	FrButton* m_pUnderBarNotice;
	FrButton* m_pUnderBarMaket;
	FrButton* m_pUnderBarScratch;
	FrButton* m_pUnderBarCombine; // 983: "utab.combine" 0x74 (handler 00652210); nome meu
	FrButton* m_pUnderBarPointShop; // 983: "utab.point_shop" 0x78 (00652520); nome meu
	FrGroupBox* m_pOverBar;
	FrGroupBox* m_pUnderTab;
	FrGroupBox* m_pUnderBar;
	FrButton* m_pUnderBarTabBtn;
	FrArea* m_pUnderBarTabToolTip;
	FrButton* m_pOverBarOneLineReqBtn;
	FrArea* m_pOverBarOneLineView;
	FrArea* m_pOverBarBar1;
	FrArea* m_pOverBarBar2;
	FrArea* m_pOverBarBar3;
	FrArea* m_pOverBarFrame1;
	FrArea* m_pOverBarFrame2;
	FrArea* m_pOverBarFrame3;
	FrArea* m_pOverBarTitle;
	FrTabButton* m_pOverBarTab;
	FrViewer* m_pOverBarChar;
	FrViewer* m_pOverBarCaddie;
	FrViewer* m_pOverBarAztec;
	FrViewer* m_pOverBarClub;
	FrViewer* m_pOverBarMascot;
	FrViewer* m_pOverBarEqItem; // 983: "obar.eqitem" 0xcc (00656700); nome meu
	unsigned long m_myItemSlot983[10]; // 983 (+0xd0; nome do C31): 10 dwords zerados no InitControls (00650f10); bloco de 0x28 copiado do FrEquipDlg (00ad3440) e mandado no 0x20 tipo 2 pelo resultado 00653be0 (s100)
	FrViewer* m_pOverBarCharSel;
	FrViewer* m_pOverBarCaddieSel;
	FrViewer* m_pOverBarAztecSel;
	FrViewer* m_pOverBarClubSel;
	FrViewer* m_pOverBarMascotSel;
	FrStatic* m_pOverBarSite;
	FrArea* m_pOverBarPangCookie;
	const Bitmap* m_pPangCookieBmp[4];
	bool m_bOverBarFlag0;
	bool m_bOverBarFlag1;
	FrEdit* m_pMessengerBubbleEdit;
	FrArea* m_pMessengerBubble;
	const Bitmap* m_pMessengerBubbleBmp[3];
	FrButton* m_pMessengerBtn;
	static FrButton* m_pMessengerAlarmBtn; // 983: saiu ("obar.messenger_alarm" não está no mapa); static provisório para o código compilar
	static FrButton* m_pMessengerGiftAlarmBtn; // 983: saiu ("obar.gift_alarm")
	FrButton* m_pTreasureAlarm;
	unsigned long m_reserved118;
	unsigned long m_reserved11c;
	bool m_bBongdariShop0;
	bool m_bBongdariShop1;
	// 983: m_bBongdariShop2 saiu; o estado mora em CSharedDoc::m_bBongdariShopOpen (006559b0, 008bb130)
	FrGiftDlg* m_pGiftDlg;
	FrOptionDlg* m_pOptionDlg;
	FrKbdManualDlg* m_pKbdManualDlg;
	FrServerDlg* m_pServerDlg;
	FrLoginRsDlg* m_pLoginRsDlg;
	FrRecycleDlg* m_pRecycleDlg;
	FrTikiMagicBoxDlg* m_pTikiMagicBoxDlg;
	FrScratchDlg* m_pScratchDlg;
	FrOnelineReqDlg* m_pOnelineReqDlg;
	FrForm* m_pWaitDlg;
	FrNoticeDlg* m_pNoticeDlg;
	FrForm* m_pDlg150;
	FrForm* m_pNotifyOneDlg;
	FrLogoutDlg* m_pQuitDlg;
	FrSPCardBuffDlg* m_pSpecialCardDlg;
	// 983 (sessão 100): 0x18c..0x1ac são 9 ponteiros de diálogo, medidos pelo InitControls (00650f10), CloseAllDialogs (0065ced0) e
	// pelos criadores (CreateForm<T> do taskmain.obj; classe pela RTTI da vtable do construtor). Saíram m_pMissionDlg[5] e o
	// COfflineTradeDlg ("offlinetraderanking" não existe no 983). Classes ainda não escritas ficam FrForm* (RTTI no comentário).
	FrCardComposeDlg* m_pCardComposeDlg983; // 0x18c: FrCardComposeDlg "card_compose" (00659490, msg 0x324; 0x325/0x326)
	FrAttendanceDlg* m_pAttendanceDlg983; // 0x190: FrAttendanceDlg "attendance_dlg" (msg 0x327; CreateForm 006550d0, new 0x1f0)
	FrWnd* m_pBongdariShop983; // 0x194: FindChildByName("bongdari_shop") (msg 0x328)
	FrEquipDlg* m_pEquipDlg983; // 0x198: FrEquipDlg "equip" (00657600; resultado 00653be0 = 0x20 tipo 2 com o bloco m_myItemSlot983)
	FrPointShopDlg* m_pPointShopDlg983; // 0x19c: FrPointShopDlg "PointShopDlg" (00659360, msg 0x2e0; new 0x1c0)
	CPost_ReciveDlg* m_pPostReciveDlg983; // 0x1a0: CPost_ReceiveDlg "postbox" (0065c2d0; resultado 00651070)
	CPost_SendDlg* m_pPostSendDlg983; // 0x1a4: CPost_SendDlg "write_post" (msg 0x29f; resultado 0065c2d0 = envio 0x145)
	FrAlchemyMPBreakDlg* m_pAlchemyDlg983; // 0x1a8: FrAlchemyMPBreakDlg "AlchemyDlg" (00655d70, msg 0x2e3; 0x2e4/0x2e5)
	FrForm* m_pMaketDlg; // 0x1ac: no 983 é o FrItemSearchDlg "ItemSearchDlg" (CreateMaketDlg 00655d10; resultado 00651060)
	std::vector<FrLevelupItemForm*> m_levelupForms;
	float m_fKbdManualTime;
	int m_hoverBtn;
	bool m_bShowSubSelection;
	unsigned long m_subSelTypeId;
	unsigned long m_subSelItemId;
	bool m_bUnderTabOn;
	WTitleFont* m_pTitleFont;
	float m_offsetX;
	float m_offsetY;
	float m_screenWidth;
	float m_screenHeight;
	unsigned long m_reserved1b4;
	std::string m_waitedLayout;
	bool m_bGiftUp;
	unsigned long m_reserved1d8;
	FrSubEquipBar* m_pSubEquipBar;
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbTurnonundertabbaFunclets(); // HACK: EH funclets of the naked TurnOnUnderTabBar (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnonelinereqchkoFunclets(); // HACK: EH funclets of the naked OnOnelineReqChkOutDlgResult (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbSendroomlistexitFunclets(); // HACK: EH funclets of the naked SendRoomListExitPacket (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnnoticedlgresulFunclets(); // HACK: EH funclets of the naked OnNoticeDlgResult (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnunderbarbongdaFunclets(); // HACK: EH funclets of the naked OnUnderBar_BongdariShopUp (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnunderbarbackupFunclets(); // HACK: EH funclets of the naked OnUnderBar_BackUp (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbChangerealmyroomFunclets(); // HACK: EH funclets of the naked ChangeRealMyRoom (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnunderbargiftupFunclets(); // HACK: EH funclets of the naked OnUnderBar_GiftUp (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbOnunderbarmyroomFunclets(); // HACK: EH funclets of the naked OnUnderBar_MyRoomUp (nakedify)
#endif
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_TASKMAIN_PCH)
public:
	static void __cdecl
	RbMakerealmyroomFunclets(); // HACK: EH funclets of the naked MakeRealMyRoom (nakedify)
#endif
};

// 983 (00657eb0, taskmain.cpp): imagens do botao do mensageiro pelas msgs 0xcd/0xce/0xd1 (CTaskMain e CGolfRule)
void __fastcall UpdateMessengerBtn983(FrButton* btn, int msgId, bool on);
