#pragma once
// JP 983: FrGameRoomDlg (UMA unidade, 004ff1c0–0050f1e0; vtables 00d0642c / 00d0641c / 00d06414; sizeof 0x430 pelo `new` de
// 0050ac20). Janela da sala de jogo; base de FrStrokeTypeRoomDlg (0x430), FrSingleTypeDlg (0x440) e FrMassTypeRoomDlg →
// FrGPTypeRoomDlg. A base é a FrChatDlg (mapa-base 00e04210, construtor 0048f0b0), não a FrForm (corrigido na sessão B42).
// É o código de sala do `CLobbyMain` do 645 (`OnGameRoom_*`) mudado para uma classe: 56 das 69 entradas do mapa de mensagens
// têm par lá, e os nomes dos handlers e dos controles vêm de lá (sem o prefixo `GameRoom_`). O resto dos nomes é meu.
// Layout (sessão B42): pelo construtor 005083c0 (grava de +0x140 a +0x41c), pelo destrutor 005088c0 e pelos 46 handlers INIT.
// Campos `m_<deslocamento>`: gravados com zero pelo construtor, tipo e uso não levantados.

#include <list>
#include <string>
#include "chatdlg.h"

class FrArea;
class FrScoreDlg;
class FrViewer;
class FrButton;
class FrGroupBox;
class FrComboCtlEx;
class FrListBox;
class FrGaugeBar;
class FrStatic;
class FrEdit;
class WTitleFont;
class CRepeatHoleFont;
class CRoomCheer983;
class FrTreasureCourse;
class FrForm;
class FrUserListDlg;
class FrNewRecordDlg;
class FrTreasureGift;
class FrUniteResultDlg;
struct sSlotInfo;
class Bitmap;
class IContentsDataContainer;

class FrGameRoomDlg : public FrChatDlg
{
	DECLARE_OBJECT(FrGameRoomDlg)

public:
	FrGameRoomDlg(); // 005083c0
	virtual ~FrGameRoomDlg(); // 005088c0
	void Set154_983(int a, int b); // 004ff300 (nome meu; público: o CLobbyMain do 983 chama; C57.1)
	void SetState983(int state); // 004ff200 (nome meu; [this+0x178] = state; C57.1)

	// B215 (A174.1/C102.2): o CLobbyMain do 983 (HandleMsg 00abc140) chama direto os metodos da sala abaixo
	friend class CLobbyMain;
	friend struct FrGameRoomDlgAcc983;

protected:
	virtual void OnProc(const float deltaTime); // 0050acd0
	virtual bool OnInit(); // 0050d0e0

	void InitRoom983(); // 0050c000 (nome meu): jogadores já na sala → lista e bonecos; chamada pelo OnInit
	void ResetRoom983(); // 00503290 (chamada pelos construtores das derivadas)
	void ReleaseRoom983(); // 00503340 (par da de cima; chamada pelo destrutor)
	void Fn503430_983(int a); // 00503430 (a sala de repetição chama com 7; corpo não lido)
	void SetCourse983(unsigned char course); // 0050d120 (recebe CSharedDoc+0x10950)
	void ShowPet(unsigned long guid); // 0050b850 (nome do 645): monta o boneco do jogador no lugar dele
	void HidePet(unsigned long guid); // 00508f10 (nome do 645): apaga o boneco do lugar
	void SetLevelBar(); // 0050b570 (nome do 645): as cinco barras de atributo, pelo CStatusHelper
	void ReLoadHoleItem(int sel); // 005034e0 (645: sem argumento; -1 = não escolher)
	void InitControlsSelectedUserInfo(); // 00503580
	bool CheckTikiReport(int state); // 00506f30 (nome do 645; lá gravava m_bUseReport)
	void OpenTikiReport983(); // 005037b0 (nome meu): mensagem 0x8b ao ator "Lobby"
	void OpenGameExtResQuitForm(); // 005055c0 (nome do 645)
	bool OnRoomExtResQuitDlgResult(int result, FrForm* form); // 00505410 (nome do 645)
	bool OnApproachGiftDlgResult(int result, FrForm* form); // 00505380 (nome do 645)
	void RefreshAllUccClothes(); // 00506d80 (nome do 645)
	bool OnResultDlgResult(int result, FrForm* form); // 00506e80 (nome do 645)
	void AddTeamSlot983(int team, sSlotInfo* pSlot); // 0050bbe0 (nome meu)
	void OnChatFace983(const std::string& nick, const std::string& chat); // 0050d280 (nome meu; caso 0x60 do 645)
	void OnChangeChar983(unsigned long guid, sCharacterInfo* pCharInfo); // 0050d390 (nome meu; caso 0x49 do 645)
	void OnEnterUser983(sSlotInfo* pSlot, sCharacterInfo* pCharInfo); // 0050d4c0 (nome meu; caso 0x07 do 645)
	void AddRoomSlot983(sSlotInfo* pSlot, sCharacterInfo* pCharInfo); // 0050d700 (nome meu)
	void MakeRoomUserList(); // 0050eb20 (nome do 645)
	void SetLockControls983(int lock); // 00503720 (nome meu; caso 0x2b do 645)
	void AlignMapHoleCombo983(); // 00503810 (nome meu)
	bool OnResultDlgResult2_983(int result, FrForm* form); // 005036f0 (nome meu)
	void GetTikiReportState983(unsigned int* pFlags, int* pCount); // 00506fe0 (nome meu)
	void OpenResult983(); // 005070c0 (nome meu; caso 0x2d do 645)
	void SortRank(); // 00505640 (nome do 645)
	void SetTeamChangeBtn983(unsigned long guid); // 00507330 (nome meu; caso 0x46 do 645)
	void SetTeamControls983(); // 00507380 (nome meu; caso 0x2f do 645)
	void SetStartBtn(bool bMaster); // 005074b0 (nome do 645)
	void OnLeaveUser983(void* pItem, unsigned long guid, int team); // 00509440 (nome meu; caso 0x08 do 645)
	void OnChangeWings983(sSlotInfo* pSlot); // 00509700 (nome meu; caso 0x09 do 645)
	void UpdateMaster983(); // 0050c760 (nome meu; caso 0x0f do 645)
	void AddGrandPrixAI983(); // 0050bc80 (nome meu)
	void SetAssistBtn983(bool bOn); // 004ff210 (nome meu)
	bool OnDlg160Result983(int result, FrForm* form); // 004ff250 (nome meu)
	void OnReadyState983(unsigned long guid, int state); // 0050ca30 (nome meu; casos 0x71/0x29 do 645)
	void OpenNewRecordForm(); // 005033b0 (nome do 645)
	bool OnNewRecordDlgResult(int result, FrForm* form); // 004ff260 (nome do 645)
	void SetRoomTitle983(const char* time); // 00508a50 (nome meu; caso 0x10 do `HandleMsg` do 645): título + artefato
	void OnEscape983(); // 00508ea0 (nome meu): ESC — relatório do Tiki ou "sair?"

	// handlers do mapa de mensagens 00e0870c, na ordem do mapa
	void OnTitleInit(int param); // 005097b0
	void OnFrameBgInit(int param); // 00500880
	void OnBackGroundInit(int param); // 00500900
	void OnCloseWndUp(); // 00509880
	void OnMapBgInit(int param); // 00500bc0
	void OnMapInit(int param); // 00503970
	void OnMapBtnUp(); // 00505a70
	void OnMapPrevInit(int param); // 00500c40
	void OnMapNextInit(int param); // 00500cf0
	void OnMapPrevBtnUp(); // 00503a50
	void OnMapNextBtnUp(); // 00503b50
	void OnMapHoleComboInit(int param); // 00503c50
	void OnMapHoleComboDown(); // 00503cf0
	void OnStartTipInit(int param); // 00500da0
	void OnReadyTipInit(int param); // 00500e20
	void OnStartInit(int param); // 00500ea0
	void OnStartBtnUp(); // 00509960
	void OnReadyInit(int param); // 00500f10
	void OnReadyBtnUp(); // 00505bc0
	void OnChangeTeamBtnInit(int param); // 00500fa0
	void OnChangeTeamBtnUp(); // 005076c0
	void OnModifyInit(int param); // 00504000
	void OnModify(); // 005040a0
	void OnPowerBarInit(int param); // 00501030
	void OnControlBarInit(int param); // 005010b0
	void OnAccuracyBarInit(int param); // 00501130
	void OnSpinBarInit(int param); // 005011b0
	void OnCurveBarInit(int param); // 00501230
	void OnPowerEditInit(int param); // 005012b0
	void OnControlEditInit(int param); // 00501320
	void OnAccuracyEditInit(int param); // 00501390
	void OnSpinEditInit(int param); // 00501400
	void OnCurveEditInit(int param); // 00501470
	void OnCaddieInit(int param); // 005014e0
	void OnCaddieDown(); // 005041c0
	void OnCaddieOwnerDraw(int param); // 00509f90
	void OnClubInit(int param); // 00501550
	void OnClubDown(); // 00504260
	void OnClubOwnerDraw(int param); // 0050a2a0
	void OnAztecInit(int param); // 005015c0
	void OnAztecDown(); // 00504300
	void OnAztecOwnerDraw(int param); // 00507780
	void OnMascotInit(int param); // 00501630
	void OnMascotDown(); // 005043a0
	void OnMascotOwnerDraw(int param); // 0050a430
	void OnEquipItemInit(int param); // 005016a0
	void OnEquipItemDown(); // 00504440
	void OnEquipItemOwnerDraw(int param); // 00501710
	void OnCharSelInit(int param); // 00501820
	void OnCharSelDown(); // 005044b0
	void OnCaddieSelInit(int param); // 005018a0
	void OnCaddieSelDown(); // 005044c0
	void OnClubSelInit(int param); // 00501920
	void OnClubSelDown(); // 005044d0
	void OnAztecSelInit(int param); // 005019a0
	void OnAztecSelDown(); // 005044e0
	void OnMascotSelInit(int param); // 00501a20
	void OnMascotSelDown(); // 005044f0
	void OnAtftInit(int param); // 00501aa0
	void OnAtftArtifactIconInit(int param); // 00501b20
	void OnAssistInit(int param); // 00504500
	void OnAssist(); // 005045b0
	void OnAssistTipInit(int param); // 0050cc20
	void OnAssistTitleInit(int param); // 00504670
	void OnShortgameInit(int param); // 00501b90
	void OnGroupStrokeInit(int param); // 00501c90
	void OnGroup30sInit(int param); // 00501c10
	void OnGroupResultInit(int param); // 00501d10
	void OnGroupGpInit(int param); // 00501d90

	void DrawFrameSelectedUserInfo(WRect rect); // 00500760 (nome do 645)
	void EnableSettingButton(bool enable); // 00503660 (nome do 645)
	void SetRoomControls(bool bMaster); // 0050c0b0 (nome do 645)
	void SetRoomCharControl(int dir); // 00508f60 (nome do 645)
	void SetRoomCaddieControl(int dir); // 0050b660 (nome do 645)
	void RemakeTeam(bool bRemake); // 00509120 (nome do 645)
	bool OnMapSelectDlgResult(int result, FrForm* form); // 004ff270
	bool OnUserListDlgResult(int result, FrForm* form); // 004ff280 (nome do 645): a lista de convite fechou
	void Banish(); // 004ff320 (nome do 645): pacote 0x26 com o jogador escolhido
	bool CanUseStartTicket983(); // 00507ad0 (nome meu): torneio (+0x1094c == 4) e bilhete 0x1a000137 no inventário
	bool OnStartTicketResult983(int result, FrForm* form); // 00505e20 (nome meu): "sim" no aviso do bilhete = começar

	// handlers que só os mapas das derivadas usam
	void OnInvite(); // 00503f30
	void OnOvergaugeInit(int param); // 00503e30
	void OnUserInit(int param); // 00500980
	void OnUserDraw(int param); // 0050d7d0
	void OnUserLButtonDown(); // 00505800
	void OnUserRButtonUp(); // 005098a0
	void OnUserDblClick(); // 005038d0

	CRoomCheer983* m_pCheer; // +0x140: `new` de 0x54 com o campo (CSharedDoc+0x10950)
	FrNewRecordDlg* m_pNewRecordDlg; // +0x144 (nome do 645): "new_record" aberto
	FrTreasureCourse* m_pTreasureCourse; // +0x148 (nome do 645): diálogo do mapa aberto
	IContentsDataContainer* m_pDoc; // +0x14c: documento 0xa5 do CContentsDoc
	struct sGPTimeBoard983* m_pTimeBoard983; // +0x150: quadro de horário da FrGPTypeRoomDlg (`new` de 0x7c em 00512f80; B69)
	int m_154; // +0x154
	int m_158; // +0x158
	CRepeatHoleFont* m_pHoleFont; // +0x15c: números do buraco fixo ("1234567890!"); criada pela FrSingleTypeDlg se faltar
	FrScoreDlg* m_pScoreDlg; // +0x160 (nome do 645): placar aberto (FrMassTypeRoomDlg, B69)
	FrUniteResultDlg* m_pUniteResultDlg; // +0x164 (nome do 645, pelo uso no CheckTikiReport)
	FrTreasureGift* m_pTreasureGift; // +0x168 (nome do 645): "trgift" do Approach
	FrUserListDlg* m_pUserListDlg; // +0x16c (nome do 645): lista de convite aberta pelo "invite"
	FrForm* m_pExtResQuitDlg; // +0x170 (nome do 645): "notify_tiki_sub" aberto pelo ESC
	FrForm* m_pNotifyDlg; // +0x174 (nome do 645): "notify" do convite de guilda (FrMassTypeRoomDlg::OnGuildInviteLBtnUp, B70)
	int m_state; // +0x178: estado da sala (6, 8…); o construtor não grava
	bool m_bIdle; // +0x17c (nome do 645)
	bool m_bReady; // +0x17d (nome do 645)
	bool m_bLockControls; // +0x17e (nome do 645)
	bool m_bCanStart; // +0x17f (nome do 645)
	bool m_bAutoStart; // +0x180 (nome do 645)
	bool m_bRoomStateReq; // +0x181 (nome do 645): pedido de pronto/começar à espera da resposta
	char m_pad182[0x184 - 0x182];
	float m_newBlinkTime; // +0x184 (nome do 645)
	int m_newBlinkCount; // +0x188 (nome do 645)
	bool m_bNewBlink; // +0x18c (nome do 645)
	bool m_b18d; // +0x18d: começa true
	char m_pad18e[0x190 - 0x18e];
	FrListBox* m_pUser; // +0x190: "user" (lista dos jogadores)
	FrArea* m_pPet[4]; // +0x194: "group_stroke.pet0" … "pet3" (gravados pela FrStrokeTypeRoomDlg)
	FrViewer* m_pBg; // +0x1a4: "frame_bg"
	FrViewer* m_pBackGround; // +0x1a8: "background"
	FrViewer* m_pMapBg; // +0x1ac: "map_background"
	FrButton* m_pMap; // +0x1b0: "map"
	int m_1b4; // +0x1b4
	FrWnd* m_p1b8; // +0x1b8: controle que ninguém cria nesta unidade (645: `m_pUnused65c`); só Enable
	FrButton* m_pMapPrev; // +0x1bc: "mapprev"
	FrButton* m_pMapNext; // +0x1c0: "mapnext"
	FrComboCtlEx* m_pMapHoleCombo; // +0x1c4: "mapcomboex"
	FrGaugeBar* m_pOverGauge; // +0x1c8: "overgauge" (FrGaugeBarImage, 00f1ac0c; `m_pMapGauge` do 645)
	FrViewer* m_pCaddie; // +0x1cc: "cur_caddie"
	FrViewer* m_pAztec; // +0x1d0: "cur_aztec"
	FrViewer* m_pClub; // +0x1d4: "cur_club"
	FrViewer* m_pMascot; // +0x1d8: "cur_mascot"
	FrViewer* m_pEquipItem; // +0x1dc: "cur_equip_item"
	FrViewer* m_pCharSel; // +0x1e0: "cur_char_sel"
	FrViewer* m_pCaddieSel; // +0x1e4: "cur_caddie_sel"
	FrViewer* m_pAztecSel; // +0x1e8: "cur_aztec_sel"
	FrViewer* m_pClubSel; // +0x1ec: "cur_club_sel"
	FrViewer* m_pMascotSel; // +0x1f0: "cur_mascot_sel"
	FrButton* m_pStart; // +0x1f4: "start"
	FrButton* m_pReady; // +0x1f8: "ready"
	char m_pad1fc[0x200 - 0x1fc];
	FrGaugeBar* m_pStatBar[5]; // +0x200: "powerbar" … "curvebar"
	FrStatic* m_pStatEdit[5]; // +0x214: "epower" … "ecurve"
	FrWnd* m_pGuildInvite; // +0x228 (nome do 645): ninguém cria nesta unidade
	FrButton* m_pChangeTeam; // +0x22c: "change_team"
	FrWnd* m_p230; // +0x230: idem (645: `m_pUnused6dc`)
	FrWnd* m_pScore; // +0x234 (nome do 645, pela ordem no caso 0x2d; ninguém cria nesta unidade)
	FrArea* m_pTeamNum; // +0x238 (nome do 645): "group_30s.30_team_num" (FrMassTypeRoomDlg, B69)
	FrArea* m_pGuildNum; // +0x23c (nome do 645): "group_30s.30_guild_num"
	int m_240; // +0x240
	int m_244; // +0x244
	FrWnd* m_pExtFrame; // +0x248 (nome do 645, idem)
	FrButton* m_pChar983; // +0x24c: botão do personagem do GM (645: `m_pUnused6f8`); ninguém cria nesta unidade
	int m_250; // +0x250
	int m_254; // +0x254
	int m_258; // +0x258
	FrArea* m_pTeamTab[2]; // +0x25c (nome do 645): "bar_redteam"/"bar_base"
	FrWnd* m_pGuildTab[2]; // +0x264 (nome do 645)
	FrGaugeBar* m_pGuildScoreGauge; // +0x26c (nome do 645): "group_result.score_gb" (B69)
	char m_pad270[0x274 - 0x270];
	FrArea* m_pStartTip; // +0x274: "start_tip"
	FrArea* m_pReadyTip; // +0x278: "ready_tip"
	FrArea* m_pCurTip; // +0x27c (nome do 645): dica à mostra
	FrArea* m_pCountdown; // +0x280 (nome do 645): "group_30s.countdown" (B69)
	FrWnd* m_pRoomUserRankTab; // +0x284 (nome do 645, pela ordem no caso 0x2d)
	FrEdit* m_pTitle; // +0x288: "title"
	FrButton* m_pRoomOption; // +0x28c: "modify"
	FrButton* m_pUserInfo; // +0x290 (nome do 645): "group_result.user_info" (B69)
	FrListBox* m_pRoomUserRank; // +0x294 (nome do 645): o SortRank ordena
	FrListBox* m_pRoomGuildRank; // +0x298 (nome do 645, pela ordem no caso 0x2d): "group_result.guildranking" (B70)
	FrArea* m_pModeTitle; // +0x29c: "mode_title"
	FrGroupBox* m_pCourseCtrl; // +0x2a0: "course_ctrl"
	FrGroupBox* m_pChipinCtrl; // +0x2a4: "chipin_ctrl"
	FrArea* m_pBgCourse; // +0x2a8: "course_ctrl.bg_course"
	FrButton* m_pFixHole; // +0x2ac: "course_ctrl.fix_hole"
	FrButton* m_pFixHolecupPos; // +0x2b0: "course_ctrl.fix_holecup_pos"
	FrComboCtlEx* m_pSlope; // +0x2b4: "chipin_ctrl.slope"
	FrComboCtlEx* m_pSize; // +0x2b8: "chipin_ctrl.size"
	FrArea* m_pTicket; // +0x2bc: "chipin_ctrl.ticket"
	FrGroupBox* m_pAtft; // +0x2c0: "atft"
	FrArea* m_pArtifactIcon; // +0x2c4: "atft.artifact_icon"
	FrButton* m_pAssist; // +0x2c8: "assist"
	FrArea* m_pAssistTip; // +0x2cc: "assist_tip"
	FrArea* m_pAssistTitle; // +0x2d0: "assist_title"
	FrArea* m_pShortGame; // +0x2d4: "shortgame"
	FrArea* m_pGPInfo983; // +0x2d8: "group_gp.grandprixinfo" (FrGPTypeRoomDlg, B69)
	FrButton* m_pGPDetail983; // +0x2dc: "group_gp.gp_detail" (FrButton pelo DYNAMIC_CAST de 00515ac0; dica ligada/desligada pelo cursor)
	FrGroupBox* m_pGroupStroke; // +0x2e0: "group_stroke"
	FrGroupBox* m_pGroup30s; // +0x2e4: "group_30s"
	FrGroupBox* m_pGroupResult; // +0x2e8: "group_result"
	FrGroupBox* m_pGroupGp; // +0x2ec: "group_gp"
	std::list<sSlotInfo*> m_redTeam; // +0x2f0 (nome do 645)
	std::list<sSlotInfo*> m_blueTeam; // +0x2fc (nome do 645)
	Bitmap* m_pTeamBmp[2]; // +0x308 (nomes do 645 daqui até m_pKickBmp): "team_red", "team_blue"
	Bitmap* m_pTeamBaseBmp; // +0x310: "team_base"
	Bitmap* m_pTeamBase02Bmp; // +0x314: "team_base02"
	Bitmap* m_pInviteBmp; // +0x318: "invite"
	Bitmap* m_pMasterSignBmp; // +0x31c: "master_sign"
	Bitmap* m_pReadySignBmp; // +0x320: "ready_sign"
	Bitmap* m_pSituBmp[7]; // +0x324 (nome do 645): "30in_situ03" … "30in_situ_blue" (FrMassTypeRoomDlg::OnRoomUserInit, B70)
	Bitmap* m_pTeamSelectBmp; // +0x340: "team_select"
	Bitmap* m_pDiveBmp; // +0x344: "dive"
	char m_pad348[0x350 - 0x348];
	const Bitmap* m_pTeamRankBmp[2]; // +0x350 (nomes do 645 daqui até m_pPlayerBaseBmp): emblemas das guildas (B70)
	const Bitmap* m_pRankBgBmp; // +0x358: "30in_rank_02"
	const Bitmap* m_pRankBmp[4]; // +0x35c
	Bitmap* m_pPlayerBaseBmp[4]; // +0x36c (nome do 645): "30_noplayer_base", "30_player_base"
	Bitmap* m_pKickBmp; // +0x37c: "kick"
	int m_380[3]; // +0x380
	int m_38c; // +0x38c
	int m_390; // +0x390
	Bitmap* m_pGenderBmp[10]; // +0x394: "i_male" … "i_female_angel"
	const Bitmap* m_pRoomEvent02Bmp; // +0x3bc (nome do 645): "room_event02" (FrMassTypeRoomDlg::OnEventOwnerDraw, B70)
	int m_3c0; // +0x3c0
	bool m_bAI983; // +0x3c4 (nome meu): há adversários de IA na lista (0050bc80)
	char m_pad3c5[0x3c8 - 0x3c5];
	int m_lastSlotAddTime983; // +0x3c8 (nomes meus daqui até +0x3d4; B70): última IA posta na lista de espera do Grand Prix (ms)
	int m_waitStartTime983; // +0x3cc: começo da espera (ms)
	float m_slotAddSec983; // +0x3d0: "add_slot_time_sec" (grandprix_ai_player.lua)
	float m_startWaitSec983; // +0x3d4: "game_start_waiting_time_sec"
	float m_resultDelay; // +0x3d8 (nome do 645): começa 1.0
	bool m_bUseReport; // +0x3dc (nome do 645): o construtor zera junto com o byte seguinte
	bool m_b3dd; // +0x3dd
	char m_pad3de[0x3e0 - 0x3de];
	int m_3e0; // +0x3e0: CSharedDoc 0062bf50
	WTitleFont* m_pFont; // +0x3e4: "[font_wind.jpg", 16×16
	bool m_bAssistTipOn; // +0x3e8 (nome meu): a dica do "assist_tip" está no RFTooltipHelper
	bool m_bArtifactTipOn; // +0x3e9 (nome meu): idem, a do artefato
	char m_pad3ea[0x3ec - 0x3ea];
	std::string m_assistTip; // +0x3ec: texto de "assist_tip" (GetGameString 0x187cd)
	std::string m_artifactTip; // +0x408 (nome meu): dica do artefato da sala
	char m_pad424[0x430 - 0x424];

	DECLARE_FRESH_MSGMAP()
};
