#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../shared/globalgamedefine.h"
#include "../shared/classdefine.h" // 983: IFF_STRUCT::sHairStyle (m_createHair983)
#include "taskmain.h"
#include "treasure.h"
#include "playerdlg.h"
#include "frcombobox.h"

class FrLoginDlg;
class FrJukeBoxDlg;
class FrAvatarMotionDlg;
struct sRoomListDoc983;
class CLobbyRoomSelector983;
class CGrandPrixGameRoomTimerFont;
class CGrandPrixGameRoomTimerBigFont;
class CGrandPrixGameRoomTicketFont;
class CGrandPrixDiffLevelFont;
class FrInviteDlg;
class FrUserListDlg;
class FrDtRoomDlg;
class FrScoreDlg;
class FrPasswordDlg;
class FrTreasureCourse;
class FrEmoticonDlg;
class FrReportDlg;
class FrMailBoxDlg;
class FrEventPrizeDlg;
class FrNewRecordDlg;
class FrParanNickDlg;
class FrCaddieWarningDlg;
class FrEquipDlg;
class ntHolesEventDlg;
class FrPointEventDlg;
class FrMakeRoomDlg;
class FrWorldTourEventDlg;
class FrFindEbortEventDlg;
class FrHalloweenEventDlg;
class FrHalloweenEventGiftDlg;
class FrChristmasEventDlg;
class FrTicketExchangeDlg;
class FrQuickResultDlg;
class FrCreateNickDlg;
class FrUseTikiReportDlg; // 983: jp/usetikireportdlg.h (frente C)
class CGaugeBar983;
class FrMissionEventDlg;
class FrBingoEventDlg;
class FrUniteResultDlg;
class FrGuildNoticeDlg;
class CPartTidList;
class FrComboBox;
class FrComboCtlEx;
struct sSlotInfo;
struct sRoomInfo;
enum eTopIconID;

extern Fresh* g_pFresh;
extern WMatrix g_camera;
extern bool g_bQuit;

class CLobbyMain : public CTaskMain
{
	DECLARE_OBJECT(CLobbyMain)

	enum roomSort_t
	{
		ROOM_SORT_NO,
		ROOM_SORT_CATE,
		ROOM_SORT_TITLE,
		ROOM_SORT_GAMETYPE,
		ROOM_SORT_HOLE,
		ROOM_SORT_COURSE,
		ROOM_SORT_STATE
	};
	enum userSort_t
	{
		USER_SORT_GENDER,
		USER_SORT_LEVEL,
		USER_SORT_GUILD,
		USER_SORT_NICK
	};
	enum eBtnDir
	{
	};

	CLobbyMain();
	virtual ~CLobbyMain();

public:
	virtual void OnLoad();
	virtual void OnDestroy();
	virtual void OnDisplay();
	virtual void OnPreLoadInit();
	virtual void OnInit();
	virtual void OnProcess(float);
	virtual void HandleMsg(const MsgObject&);
	void OpenSingleModeStarter983(); // 00595f60: "single_mode_start" (botao do modo solo do RFLobbyDesktop 005f7540, frente B)

protected:
	void AnimateWebEventButton(float);
	void OnToppage_Destroy();
	void OnToppage_GameUp();
	void OnToppage_TreasureIslandLBDown();
	void OnToppage_ServerUp();
	void OnServer_Init(int);
	bool OnNewRecordDlgResult(int, FrForm*);
	void OnBlank_Init(int);
	void OnSecond_Pwd_Init(int);
	void OnSecond_Pwd_Finish();
	bool OnEventDlgResult(int, FrForm*);
	bool OnMailBoxDlgResult(int, FrForm*);
	bool OnNotifyQuitDlgResult(int, FrForm*);
	bool OnCreateInfoResult(int, FrForm*);
	bool OnHolesEventDlgResult(int, FrForm*);
	bool OnPointEventDlgResult(int, FrForm*);
	bool OnWorldTourEventDlgResult(int, FrForm*);
	bool OnFindEbortEventDlgResult(int, FrForm*);
	bool OnHalloweenEventDlgResult(int, FrForm*);
	bool OnChristmasEventDlgResult(int, FrForm*);
	bool OnTicketExchangeDlgResult(int, FrForm*);
	bool OnQuickResultDlgResult(int, FrForm*);
	void ResetControl();
	void InitControlsSelectedUserInfo();
	void OnCreate_Init(int);
	void OnCreate_Finish();
	void OnCreate_Destroy();
	bool OnCreateNickResult(int, FrForm*);
	bool OnMissionEventResult(int, FrForm*);
	bool OnBingoEventResult(int, FrForm*);
	float GetTopPageBtnRect();
	void OpenEventPrizeForm(unsigned char);
	void OnToppage_GameInit(int);
	void OnToppage_FamilyInit(int);
	void OnToppage_PangysStudyInit(int);
	void OnToppage_PremiumServiceInit983(int); // 00596290
	void OnToppage_PremiumEndNoticeInit983(int); // 00596180
	void OnToppage_PremiumEndNoticeOwnerDraw983(int); // 00597160 (D644)
	void OnToppage_NoticeInit(int);
	void OnToppage_ChangeBgInit983(int); // 005951b0
	void OnToppage_ChangeBgUp983(); // 00595230
	void OnToppage_OneDayQuestInit983(int); // 00593900
	void OnToppage_OneDayQuestDown983(); // 00593990
	void OnToppage_RankUpInit983(int); // 005949d0
	void OnToppage_RankUpDown983(); // 00594a90
	void OnToppage_GuildRankInit983(int); // 00594b00
	void OnToppage_GuildRankDown983(); // 00594bb0
	void OnToppage_MotionBtnInit983(int); // 00ab7430
	void OnToppage_JukeboxBtnInit983(int); // 00ab76b0
	void OnToppage_JukeboxBtnUp983(); // 00ab8620 -> 00ab7820
	void OnToppage_MotionBtnUp983(); // 00ab8600
	void OnToppage_MotionBtnHoverOn983(); // 00ab7510
	void OnToppage_MotionBtnHoverOff983(); // 00ab7590
	void OnToppage_FamilyHoverOn983(); // 00596d60
	void OnToppage_PangyaStudyHoverOn983(); // 00596e60
	void OnToppage_NoticeHoverOn983(); // 00596f60
	void OnToppage_HoverOff983(); // 00596de0/00596ee0/00596fe0
	void OnToppage_PremiumServiceHoverOn983(); // 005978b0
	void OnToppage_PremiumServiceHoverOff983(); // 00597a40
	void SetMotionBtnOnImg983(); // 00ab3620
	void OpenMotionDlg983(); // 00ab8430
	bool OnMotionDlgResult983(int result, FrForm* pForm); // 00ab7610
	bool OnJukeBoxDlgResult983(int result, FrForm* pForm); // 00ab36a0
	void AnchorMotionBtn983(); // 00ab45c0
	void AnchorJukeboxBtn983(); // 00ab4620
	void ReanchorRightIcons983(); // 00ab7780 (OnProcess)
	void ProcessPremiumEndNotice983(float dt); // cauda TOPPAGE do 00ab7a70 (D644)
	void OnToppage_NoticeUp();
	void OnToppage_PcbangInit(int);
	void OnToppage_Event1Init(int);
	void OnToppage_Event1Up();
	void OnToppage_NewInit(int);
	void OnToppage_NewOwnerDraw(int);
	void OnToppage_PointEventInit(int);
	void OnToppage_ValEventInit(int);
	void OnToppage_ValEventDown();
	void OnToppage_WorldTourEventInit(int);
	void OnToppage_WorldTourEventDown();
	void OnToppage_FindEbortEventInit(int);
	void OnToppage_FindEbortEventDown();
	void OnToppage_Halloween2007Init(int);
	void OnToppage_Halloween2007EvenDown();
	void OnToppage_Christmas2007EventInit(int);
	void OnToppage_Christmas2007EventDown();
	void OnToppage_TicketExchangeInit(int);
	void OnToppage_TicketExchangeDown();
	void OnToppage_MissionEventInit(int);
	void OnToppage_MissionEventDown();
	void OnToppage_BingoEventInit(int);
	void OnToppage_BingoEventDown();
	void OnToppage_KoohBirthdayEventInit(int);
	void OnToppage_KoohBirthdayEventDown();
	void OnToppage_TreasureIslandInit(int);
	void OnToppage_ServerInit(int);
	void OnServer_Finish();
	bool OnLoginDlgResult(int, FrForm*);
	bool OnVoteDlgResult(int, FrForm*);
	bool OnQuickStartDlgResult(int, FrForm*);
	void OpenLayout(const char*);
	void OnCreate_CharInit(int);
	void OnCreate_CharPrevInit(int);
	void OnCreate_CharNextInit(int);
	void OnCreate_HairInit(int);
	void OnCreate_HairPrevInit(int);
	void OnCreate_HairNextInit(int);
	void OnCreate_ShirtsInit(int);
	void OnCreate_ShirtsPrevInit(int);
	void OnCreate_ShirtsNextInit(int);
	void OnCreate_DescInit(int);
	void OnCreate_WarningInit(int);
	void On_PowerBarInit(int);
	void On_ControlBarInit(int);
	void On_AccuracyBarInit(int);
	void On_SpinBarInit(int);
	void On_CurveBarInit(int);
	void OnToppage_TopIcon1Init(int);
	void OnToppage_TopIcon1Up();
	void OnToppage_TopIcon2Init(int);
	void OnToppage_TopIcon2Up();
	void OnToppage_TopIcon3Init(int);
	void OnToppage_TopIcon3Up();
	void OnToppage_TopIcon4Init(int);
	void OnToppage_TopIcon4Up();
	void OnToppage_TopIcon5Init(int);
	void OnToppage_TopIcon5Up();
	void OnToppage_TopIcon6Init(int);
	void OnToppage_TopIcon6Up();
	void ProcessLobbyBgCamera(float);
	void ProcessAdministrator(float);
	void OpenPCBangNoticeForm();
	bool NeedsChangeNick();
	void OpenNoteForm();
	void OnToppage_Init(int);
	void OnToppage_Finish();
	void OnToppage_PangysStudyUp();
	void OnToppage_RentalExpiredAlramInit(int);
	void OnFirstConnectOwnerDraw(int);
	void OnToppage_PointEventDown();
	virtual void CloseAllDialogs();
	void OnLogin_Init(int);
	void OnLogin_Finish();
	void OnLogin_Copyright1Init983(int); // 983: 005934b0 ("LOGIN.copyright1")
	void OnLogin_Copyright2Init983(int); // 983: 005935c0 ("LOGIN.copyright2")
	void OnBlank_Finish();
	bool OnPCBangDlgResult(int, FrForm*);
	void OpenPremiumService983(); // 00596370: "TOPPAGE.PremiumService" -> notice_popup + premiumservice.ini
	bool OnPremiumServiceDlgResult983(int, FrForm*); // 00ab6860
	bool OnCaddieWarningDlgResult(int, FrForm*);
	void OpenCreateInfo(unsigned long, const char*);
	bool LatestVersion();
	void OpenNewRecordForm();
	bool SetFirstChar(CPartTidList*);
	void OnCreate_CreateBtnUp();
	bool OnCreateConfirmResult983(int, FrForm*); // 00ab6fc0: "notify_yesno" do OnCreate_CreateBtnUp; com 1 manda o 0x8
	void OnCreate_CharacterListInit983(int); // 00ab8850: "CREATE.character_list" (lista dos personagens do Character.iff)
	void OnCreate_CharacterListDown983(int); // 00abc0e0: escolhe o personagem da linha
	void OnCreate_CharacterListOwnerDraw983(int); // 00ab70b0: rosto + moldura + cursor
	void OnCreate_DrawAreaInit983(int); // 00ab4330
	void OnCreate_DrawAreaOwnerDraw983(int); // 00ab4380: figura do cabelo e os 5 medidores com o valor
	void OnCreate_CharNameImageInit983(int); // 00ab82a0: "CREATE.char_name_image" na "character_name_pos" do Lua
	void SelectCreateChar983(unsigned char index); // 00abbb90: troca o personagem (0/1/3 -> 11/12/14), boneco e medidores
	bool OnUseTikiReportResult983(int, FrForm*); // 00ab2fa0: resultado da "notify_tiki"
	bool OnSingleModeStartResult983(int, FrForm*); // 00abaa10: resultado da "single_mode_start"; FrYES manda o 0x8 da sala solo
	bool OnRoomListDlgResult983(int, FrForm*); // 00ab6620: resultado da "roomlist"/"grandprix_list"; manda 0x82/0x177
	void OpenGameRoomDlg983(int state); // 00ab6cc0: HandleMsg 0x8b casos 6..0xb; "gameroom_main"/"gameroom_single" pelo estado 3..8
	bool OnGameRoomDlgResult983(int, FrForm*); // 00ab6700: resultado da janela da sala; sai da sala (SendExitRoom983/SendExitGPRoom983)
	bool OnMatchingDlgResult(int, FrForm*);
	void OnToppage_UCCShopBtnUp();
	void OnToppage_CardShopBtnUp();
	void ProcessLobbyPetSync(float);
	void OpenCaddieWarningForm();
	void OnToppage_FamilyUp();
	void OnToppage_PcbangUp();
	bool OnMakeRoomDlgResult(int, FrForm*);
	bool OnInviteDlgResult(int, FrForm*);
	bool OnPasswordDlgResult(int, FrForm*);
	bool OnParanNickChangeResult(int, FrForm*);
	void SortRank();
	void ProcessDialogKeyboard(float);
	void ProcessLobbyUI(float);
	void ProcessAutoStart(float);
	void OpenParanNickChange();
	void SetLevelBar();
	void RefreshAllUccClothes();
	void OnToppage_RentalExpiredAlramUp();
	void OnCreatePetInit(int);
	void OnCreatePetOwnerDraw();
	void OnCreate_HairPrevBtnUp();
	void OnCreate_HairNextBtnUp();
	void OnCreate_ShirtsPrevBtnUp();
	void OnCreate_ShirtsNextBtnUp();

private:

	DECLARE_FRESH_MSGMAP()

protected:
	// s112: campos na ordem do 983 (sizeof 0x7a4; out/F3.5/s112/RELATORIO.md). O 983 tirou do CLobbyMain as telas ROOMLIST/GAMEROOM
	// (FrRoomListDlg/FrGameRoomDlg da frente B): os campos do 645 sem lugar no 983 ficam no apêndice depois de 0x7a4.
	int m_topBtnIndex; // 218
	unsigned long m_reserved1e4; // 21c
	FrButton* m_pToppageGame; // 220
	FrButton* m_pToppageFamily;
	FrButton* m_pToppagePangyaStudy;
	FrWnd* m_pToppageReserved;
	FrButton* m_pToppageNotice; // 230
	FrButton* m_pToppageOneDayQuest; // 234: "TOPPAGE.oneday_quest" (00593900); nome meu
	FrButton* m_pToppageServer;
	FrButton* m_pToppagePcbang;
	FrButton* m_pToppageEvent1; // 240
	FrButton* m_pToppagePointEvent;
	FrButton* m_pToppageWorldTourEvent;
	FrButton* m_pToppageFindEbortEvent;
	FrButton* m_pToppageHalloween2007Event; // 250
	FrButton* m_pToppageChristmas2007Event;
	FrButton* m_pToppageTicketExchange;
	FrButton* m_pToppageMissionEvent;
	FrButton* m_pToppageBingoEvent; // 260
	FrButton* m_pToppageKoohBirthdayEvent; // 264: sem uso no 983 (o lugar ficou)
	FrButton* m_pToppageValEvent; // 268: idem
	FrButton* m_pToppageTreasureIsland; // 26c
	FrArea* m_pCreatePetArea983; // 270: "CREATE.pet" (OnCreatePetInit 00abc070; lida em 00abbb90)
	FrButton* m_pToppagePremiumService983; // 274: "TOPPAGE.PremiumService" (Init 00596290)
	FrWnd* m_pToppagePremiumEndNotice983; // 278: "TOPPAGE.premium_service_end_notice" (Init 00596180)
	unsigned long m_premiumEndNoticeAlpha983; // 27c: alfa (float) do aviso de fim do premium no TOPPAGE (ProcessPremiumEndNotice983); 280: velocidade do fade. Zerados no ResetControl 00ab3120; 00ab7a70, 00597160
	unsigned long m_premiumEndNoticeFadeSpeed983;
	unsigned long m_premiumEndNoticeBmp983[4]; // 284..290: Bitmap* "Premium service_close_00/01/02" e "Premium service_base" (Init 00596180; desenhados no 00597160)
	FrArea* m_pToppageNew; // 294 ("new_2", 645 0x274)
	Bitmap* m_pLoginBmp[3]; // 298
	Bitmap* m_pNewBmp; // 2a4
	void* m_pGuildNoticeObj; // 2a8
	FrForm* m_pGuildNoticeDlg; // 2ac
	bool m_bCaddieWarning; // 2b0
	sCaddieInfo m_caddieWarning[12]; // 2b1 (memset 0x12c no construtor)
	float m_bgCameraDir; // 3e0
	bool m_bBgCameraPause; // 3e4
	int m_bgCameraStep; // 3e8 (983 00ab5be0)
	float m_bgCameraTime; // 3ec
	float m_bgCameraDuration; // 3f0
	bool m_bLoginBlock; // 3f4
	float m_autoStartTime; // 3f8
	bool m_bAutoStart; // 3fc
	unsigned long m_reserved3b0; // 400
	WRect m_event1Rect; // 404
	float m_event1Dir; // 414
	unsigned long m_reserved3cc; // 418
	FrLoginDlg* m_pLoginDlg; // 41c
	FrInviteDlg* m_pInviteDlg; // 420
	FrUserListDlg* m_pUserListDlg; // 424 (pela ordem do ResetControl; sem outro uso no 983)
	FrForm* m_pRemovedDtRoomDlg983; // 428: vaga do m_pDtRoomDlg/m_pScoreDlg do 645 (entre UserList e Password); o 983 so zera no ResetControl 00ab316f (sala = FrGameRoomDlg)
	FrPasswordDlg* m_pPasswordDlg; // 42c
	FrNoticeDlg* m_pPCBangNoticeDlg; // 430
	unsigned long m_reserved418; // 434
	FrForm* m_pPremiumServiceDlg983; // 438: aberta pelo "TOPPAGE.PremiumService" LBUTTONDOWN 00596370
	FrEquipDlg* m_pEquipDlg; // 43c
	FrMailBoxDlg* m_pMailBoxDlg; // 440
	FrEventPrizeDlg* m_pEventPrizeDlg[6]; // 444
	FrNewRecordDlg* m_pNewRecordDlg; // 45c
	FrParanNickDlg* m_pParanNickDlg; // 460
	FrCaddieWarningDlg* m_pCaddieWarningDlg; // 464
	FrUseTikiReportDlg* m_pUseTikiReportDlg983; // 468: "notify_tiki" (HandleMsg 0x8b caso 0xc 00abcc8a; resultado 00ab2fa0)
	ntHolesEventDlg* m_pHolesEventDlg; // 46c (OnToppage_Event1Up 00596480)
	FrPointEventDlg* m_pPointEventDlg; // 470
	unsigned long m_reserved474; // 474: o 983 so zera no construtor (00aba52d), sem leitura
	FrMakeRoomDlg* m_pMakeRoomDlg; // 478
	unsigned long m_reserved46c; // 47c
	FrWorldTourEventDlg* m_pWorldTourEventDlg; // 480
	FrFindEbortEventDlg* m_pFindEbortEventDlg;
	FrHalloweenEventDlg* m_pHalloweenEventDlg;
	FrHalloweenEventGiftDlg* m_pHalloweenEventGiftDlg;
	FrChristmasEventDlg* m_pChristmasEventDlg; // 490
	FrTicketExchangeDlg* m_pTicketExchangeDlg;
	FrForm* m_pQuickStartDlg;
	FrQuickResultDlg* m_pQuickResultDlg;
	FrForm* m_pReservedDlg490; // 4a0
	FrCreateNickDlg* m_pCreateNickDlg; // 4a4
	FrMissionEventDlg* m_pMissionEventDlg; // 4a8
	FrBingoEventDlg* m_pBingoEventDlg; // 4ac
	// 4b0..5d7: sem tipo ainda. Usos no 983: 4b0 ("motion_btn_start" Init 00ab7430, 00ab3620/00ab3660/00ab8430), 4b4, 4b8 (Banish),
	// 4bc (ResetControl), 4c0 (00595280, dynamic_cast do FrWnd), 4c4 (construtor), 4c8 (m_roomPassword), 5cc (m_bCreateRoom), 5d0 (m_bCanStart, 00ab5e00).
	// (o +0x568 do ProcessLobbyPetSync 00ab8e32 e de outro objeto, o valor do mapa em doc+0x3894, nao do CLobbyMain)
	FrButton* m_pMotionBtn983; // 4b0: "TOPPAGE.motion_btn_start" (Init 00ab7430; ancorado a direita por 00ab45c0)
	FrAvatarMotionDlg* m_pMotionDlg983; // 4b4: janela das emocoes (00ab8430 abre, 00ab4500 fecha)
	sRoomListDoc983* m_pRoomListDoc983; // 4b8: CContentsDoc::GetContainer(0xa5) (OnPreLoadInit 00ab9800)
	unsigned long m_reserved4bc; // 4bc: o 983 so zera no ResetControl (00ab31e1), sem leitura
	FrWnd* m_pHiddenWnd983; // 4c0: so o 00595280 grava (dynamic_cast do remetente + SetVisible(false)); sem chamador no 983
	unsigned long m_reserved4c4; // 4c4: o 983 so zera no construtor (00aba509), sem leitura
	char m_roomPassword[260]; // 4c8: construtor zera [+0x4c8] (00aba552); HandleMsg 0x27/0x28 add ebx,0x4c8 (00abcf79/00abcf92)
	bool m_bCreateRoom; // 5cc: HandleMsg 0x16 grava / 0x17 le [+0x5cc] (00abc7bf/00abc7ca)
	unsigned char m_pad983_5cd[3];
	bool m_bCanStart; // 5d0: auto-start da sala (00ab5e00: [+0x5d0] || m_bAutoStart [+0x3fc])
	unsigned char m_quickStartCate; // 5d1: so o quick start le (00abd803 movzx [+0x5d1]); o 983 nao grava
	unsigned char m_pad983_5d2[6];
	FrEdit* m_pCreateChar; // 5d8 (645 0x970)
	FrEdit* m_pCreateDesc;
	unsigned char m_createCharType; // 5e0
	FrEdit* m_pCreateHair;
	unsigned char m_createHairColor; // 5e8
	FrEdit* m_pCreateShirt;
	unsigned char m_createShirtColor; // 5f0
	FrGaugeBar* m_pStatBar[5]; // 5f4: CREATE.powerbar..curvebar (On_PowerBarInit 00ab40b0)
	FrStatic* m_pStatEdit[5]; // 608 (zerados no construtor)
	FrButton* m_pToppageRankUp983; // 61c: "TOPPAGE.rankup" (Init 005949d0)
	FrButton* m_pToppageWebEvent983[2]; // 620, 624: "TOPPAGE.WebEvent1/2" (Init 00597ae0 / 00597c50)
	FrButton* m_pToppageGuildRank983; // 628: "TOPPAGE.guildrank" (Init 00594b00)
	unsigned long m_toppageWebBtnSlots983; // 62c: std::bitset<5> das vagas dos botoes do TOPPAGE (00597820 no construtor: bit0 = IsLocalContent(0x16), bit3 = (0x1f), bit4 = (0x98)); lido por 00596ca0 (WebEvent1/2 Init 00597ae0/00597c50) e 00596ce0 (conta vagas antes, desloca x em 00597650)
	// 630: cabelos de cada personagem (HairStyle.iff sem os de cash e com preço < 10000; índice = cCharID), refeitos no
	// OnCreate_Init 00abaf40; m_createHairColor é o índice no vetor do m_createCharType (construtor 00ab78d0, destrutor 00aba1f0)
	std::vector<IFF_STRUCT::sHairStyle> m_createHair983[15];
	FrListBox* m_pCreateCharacterList983; // 720: "CREATE.character_list" (Init 00ab8850)
	FrArea* m_pCreateCharNameImage983; // 724: "CREATE.char_name_image" (Init 00ab82a0)
	WPoint m_hairColorPos983; // 728: "hair_color_pos" do char_select_free.lua (OnCreate_Init 00abaf40; "CREATE.draw_area")
	WPoint m_statusPos983; // 730: "status_pos" (origem dos 5 medidores)
	CGaugeBar983* m_pStatGauge983[5]; // 738..748: medidores "chselect_gauge_%d.tga" (OnCreate_Init 00abaf40, OnCreate_Destroy 00ab9670)
	bool m_motionBtnClicked983; // 74c: "motion_btn_start" LBUTTONUP 00ab8600 e fim da janela 00ab7610
	WRect m_motionBtnRect983; // 750..75c: retangulo do "motion_btn_start" no recurso (base 800 de largura; Init 00ab7430)
	WRect m_jukeboxBtnRect983; // 760..76c: retangulo do "portrait_jukeboxbtn" no recurso (Init 00ab76b0)
	FrButton* m_pToppageJukeboxBtn983; // 770: "TOPPAGE.portrait_jukeboxbtn" Init 00ab76b0
	FrJukeBoxDlg* m_pJukeBoxDlg983; // 774: "jukeboxdlg" (00ab7820 abre; 00ab36a0 zera)
	CLobbyRoomSelector983* m_pRoomSelector983; // 778: zerado no construtor; OnPreLoadInit 00ab9800 (new 0x24, 005a0780)
	unsigned long m_pRoomListForm983; // 77c: último do ResetControl
	FrForm* m_pGameRoomDlg983; // 780: "gameroom_main"/"gameroom_single" (00ab6cc0; janela da frente B); 00ab2fa0 fecha
	// 784..7a0: fontes do Grand Prix (OnPreLoadInit 00ab9800; titlefontex.h)
	std::tr1::shared_ptr<CGrandPrixGameRoomTimerFont> m_pGPTimerFont983; // 784 (006f5180)
	std::tr1::shared_ptr<CGrandPrixGameRoomTimerBigFont> m_pGPTimerBigFont983; // 78c (006f5240): CGrandPrixClock::m_pFont
	std::tr1::shared_ptr<CGrandPrixGameRoomTicketFont> m_pGPTicketFont983; // 794 (006f5300)
	std::tr1::shared_ptr<CGrandPrixDiffLevelFont> m_pGPDiffLevelFont983; // 79c (006f53c0)
	// 7a4 = sizeof do 983 (push 0x7a4 em 00592fd3)
};
