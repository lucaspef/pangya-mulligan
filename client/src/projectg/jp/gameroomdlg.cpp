// JP 983: unidade 004ff1c0–0050f1e0 (FrGameRoomDlg). É o código de sala do `CLobbyMain` do 645 (`OnGameRoom_*`) mudado
// para uma classe própria: os corpos partem dos do src/projectg/lobbymain.cpp e foram conferidos, um a um, com o 983.
// Sessão B42 (1ª parte): registro, mapa de mensagens (69 entradas), construtor, destrutor, os 44 handlers INIT do mapa, os
// cliques pequenos (fechar, mapa anterior/seguinte, ordem dos buracos, os cinco "…_sel", os quatro equipamentos, o item) e
// os dois auxiliares `ReLoadHoleItem` / `InitControlsSelectedUserInfo`. O resto da unidade (OnProc, OnInit, desenhos,
// lista de jogadores, começar/pronto, moldes) fica para as próximas sessões — só declarado no cabeçalho.
// Sessão B43 (2ª parte, no fim do arquivo): modify, assist, change_team, ready, map, os 5 OWNERDRAW,
// DrawFrameSelectedUserInfo e os dois retornos de diálogo.
// Sessão B44 (3ª parte, no fim do arquivo): start (com o bilhete 0x1a000137 e a conferência do artefato), os 7 handlers que
// só as derivadas usam (lista de jogadores, "overgauge", "invite"), Banish, EnableSettingButton e SetCourse983.
// Sessão B45 (4ª parte, no fim do arquivo): ShowPet/HidePet, SetLevelBar, SetRoomControls, SetRoomCharControl,
// SetRoomCaddieControl e ReleaseRoom983.
// Sessão B46 (5ª parte, no fim do arquivo): ResetRoom983, recorde novo, ESC (CheckTikiReport, relatório, "sair?"),
// RemakeTeam, SetRoomTitle983 (título + artefato), InitRoom983/OnInit, OnProc e o retorno do presente do Approach.
// Sessão B47 (6ª parte, no fim do arquivo): os casos do `HandleMsg` do 645 que viraram métodos (lista de jogadores,
// entrada/saída, troca de personagem, asas, careta, dono da sala, pronto, times, resultado), SortRank, SetStartBtn,
// RefreshAllUccClothes, as IAs do Grand Prix e cinco pequenas. Com isso todos os métodos com corpo próprio achados na
// faixa estão escritos; o que sobra sem par são moldes da STL e embutidas da frente A (lista em out/F3.7/s47/sem_par.tsv).
#include "minatl.h"
#include "vc10_layout.h"
#include "projectg.h"
#include "singletypedlg.h"
#include "shareddocjp.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "holeinonescoreinfo.h"
#include "gamestringmanager.h"
#include "icontentsdoc.h"
#include "lobbycontainer983.h"
#include "taskmanager.h"
#include "netresourcemanager.h"
#include "packet.h"
#include "wfont.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frviewer.h"
#include "frbutton.h"
#include "frgroupbox.h"
#include "frcombobox.h"
#include "frlistbox.h"
#include "frgaugebar.h"
#include "frstatic.h"
#include "fredit.h"
#include "treasuredlg.h"
#include "frgraphicinterface.h"
#include "inputmanager.h"
#include "soundmanager.h"
#include "bitmap.h"
#include "changeroominfodlg.h"
#include "messengerdlg.h"
#include "userlistdlg.h"
#include "lobbyavataractor.h"
#include "user_info.h"
#include "treasure.h"
#include "cardsystem.h"
#include "statushelper.h"
#include "lobbyjp.h"
#include "newrecorddlg.h"
#include "rftooltiphelper.h"
#include "resultdlg.h"
#include "aiplayer.h"
#include "holeinonegamehandler.h"
#include "s5/utilities.h"
#include "s5/classicserver/classicserver.h"

#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))
// modos 0x14 e 0x15 (Grand Prix) do byte +0x109b8; o 983 testa como `(modo + -0x14) <= 1` sem sinal (forma de `switch`)
inline bool IsGPMode983(int mode)
{
	switch (mode)
	{
	case 0x14:
	case 0x15:
		return true;
	}
	return false;
}
#define IS_GP_MODE983() IsGPMode983(DOC983_U8(0x109b8))
// documento 0xa5 (+0x14c): +0x14 = uid do dono da sala (`m_masterOID` do CLobbyMain do 645)
#define ROOMDOC_MASTER(p) (static_cast<CLobbyContainer983*>(p)->m_master) // A844: era +0x14 cru

extern Fresh* g_pFresh;
// PrevMap 00502dc0, NextMap 00502e40, FirstMap 00505140 e LastMap 005051c0 são as embutidas do shareddoc.inl (frente A)
// emitidas nesta unidade; no 983 a tabela de ordem tem 21 mapas (00d05e80), não 18 — pedido B → A (B45)
#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

IMPLEMENT_OBJECT(FrGameRoomDlg, FrChatDlg)

BEGIN_FRESH_MSGMAP(FrGameRoomDlg, FrChatDlg)

ON_FRESH_VI("title", FRCMD_INIT, FrGameRoomDlg::OnTitleInit)
ON_FRESH_VI("frame_bg", FRCMD_INIT, FrGameRoomDlg::OnFrameBgInit)
ON_FRESH_VI("background", FRCMD_INIT, FrGameRoomDlg::OnBackGroundInit)
ON_FRESH_VV("close_wnd", FRCMD_LBUTTONUP, FrGameRoomDlg::OnCloseWndUp)
ON_FRESH_VI("map_background", FRCMD_INIT, FrGameRoomDlg::OnMapBgInit)
ON_FRESH_VI("map", FRCMD_INIT, FrGameRoomDlg::OnMapInit)
ON_FRESH_VV("map", FRCMD_LBUTTONUP, FrGameRoomDlg::OnMapBtnUp)
ON_FRESH_VI("mapprev", FRCMD_INIT, FrGameRoomDlg::OnMapPrevInit)
ON_FRESH_VI("mapnext", FRCMD_INIT, FrGameRoomDlg::OnMapNextInit)
ON_FRESH_VV("mapprev", FRCMD_LBUTTONUP, FrGameRoomDlg::OnMapPrevBtnUp)
ON_FRESH_VV("mapnext", FRCMD_LBUTTONUP, FrGameRoomDlg::OnMapNextBtnUp)
ON_FRESH_VI("mapcomboex", FRCMD_INIT, FrGameRoomDlg::OnMapHoleComboInit)
ON_FRESH_VV("mapcomboex", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnMapHoleComboDown)
ON_FRESH_VI("start_tip", FRCMD_INIT, FrGameRoomDlg::OnStartTipInit)
ON_FRESH_VI("ready_tip", FRCMD_INIT, FrGameRoomDlg::OnReadyTipInit)
ON_FRESH_VI("start", FRCMD_INIT, FrGameRoomDlg::OnStartInit)
ON_FRESH_VV("start", FRCMD_LBUTTONUP, FrGameRoomDlg::OnStartBtnUp)
ON_FRESH_VI("ready", FRCMD_INIT, FrGameRoomDlg::OnReadyInit)
ON_FRESH_VV("ready", FRCMD_LBUTTONUP, FrGameRoomDlg::OnReadyBtnUp)
ON_FRESH_VI("change_team", FRCMD_INIT, FrGameRoomDlg::OnChangeTeamBtnInit)
ON_FRESH_VV("change_team", FRCMD_LBUTTONUP, FrGameRoomDlg::OnChangeTeamBtnUp)
ON_FRESH_VI("modify", FRCMD_INIT, FrGameRoomDlg::OnModifyInit)
ON_FRESH_VV("modify", FRCMD_LBUTTONUP, FrGameRoomDlg::OnModify)
ON_FRESH_VI("powerbar", FRCMD_INIT, FrGameRoomDlg::OnPowerBarInit)
ON_FRESH_VI("controlbar", FRCMD_INIT, FrGameRoomDlg::OnControlBarInit)
ON_FRESH_VI("accuracybar", FRCMD_INIT, FrGameRoomDlg::OnAccuracyBarInit)
ON_FRESH_VI("spinbar", FRCMD_INIT, FrGameRoomDlg::OnSpinBarInit)
ON_FRESH_VI("curvebar", FRCMD_INIT, FrGameRoomDlg::OnCurveBarInit)
ON_FRESH_VI("epower", FRCMD_INIT, FrGameRoomDlg::OnPowerEditInit)
ON_FRESH_VI("econtrol", FRCMD_INIT, FrGameRoomDlg::OnControlEditInit)
ON_FRESH_VI("eaccuracy", FRCMD_INIT, FrGameRoomDlg::OnAccuracyEditInit)
ON_FRESH_VI("espin", FRCMD_INIT, FrGameRoomDlg::OnSpinEditInit)
ON_FRESH_VI("ecurve", FRCMD_INIT, FrGameRoomDlg::OnCurveEditInit)
ON_FRESH_VI("cur_caddie", FRCMD_INIT, FrGameRoomDlg::OnCaddieInit)
ON_FRESH_VV("cur_caddie", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnCaddieDown)
ON_FRESH_VI("cur_caddie", FRCMD_OWNERDRAW, FrGameRoomDlg::OnCaddieOwnerDraw)
ON_FRESH_VI("cur_club", FRCMD_INIT, FrGameRoomDlg::OnClubInit)
ON_FRESH_VV("cur_club", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnClubDown)
ON_FRESH_VI("cur_club", FRCMD_OWNERDRAW, FrGameRoomDlg::OnClubOwnerDraw)
ON_FRESH_VI("cur_aztec", FRCMD_INIT, FrGameRoomDlg::OnAztecInit)
ON_FRESH_VV("cur_aztec", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnAztecDown)
ON_FRESH_VI("cur_aztec", FRCMD_OWNERDRAW, FrGameRoomDlg::OnAztecOwnerDraw)
ON_FRESH_VI("cur_mascot", FRCMD_INIT, FrGameRoomDlg::OnMascotInit)
ON_FRESH_VV("cur_mascot", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnMascotDown)
ON_FRESH_VI("cur_mascot", FRCMD_OWNERDRAW, FrGameRoomDlg::OnMascotOwnerDraw)
ON_FRESH_VI("cur_equip_item", FRCMD_INIT, FrGameRoomDlg::OnEquipItemInit)
ON_FRESH_VV("cur_equip_item", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnEquipItemDown)
ON_FRESH_VI("cur_equip_item", FRCMD_OWNERDRAW, FrGameRoomDlg::OnEquipItemOwnerDraw)
ON_FRESH_VI("cur_char_sel", FRCMD_INIT, FrGameRoomDlg::OnCharSelInit)
ON_FRESH_VV("cur_char_sel", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnCharSelDown)
ON_FRESH_VI("cur_caddie_sel", FRCMD_INIT, FrGameRoomDlg::OnCaddieSelInit)
ON_FRESH_VV("cur_caddie_sel", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnCaddieSelDown)
ON_FRESH_VI("cur_club_sel", FRCMD_INIT, FrGameRoomDlg::OnClubSelInit)
ON_FRESH_VV("cur_club_sel", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnClubSelDown)
ON_FRESH_VI("cur_aztec_sel", FRCMD_INIT, FrGameRoomDlg::OnAztecSelInit)
ON_FRESH_VV("cur_aztec_sel", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnAztecSelDown)
ON_FRESH_VI("cur_mascot_sel", FRCMD_INIT, FrGameRoomDlg::OnMascotSelInit)
ON_FRESH_VV("cur_mascot_sel", FRCMD_LBUTTONDOWN, FrGameRoomDlg::OnMascotSelDown)
ON_FRESH_VI("atft", FRCMD_INIT, FrGameRoomDlg::OnAtftInit)
ON_FRESH_VI("atft.artifact_icon", FRCMD_INIT, FrGameRoomDlg::OnAtftArtifactIconInit)
ON_FRESH_VI("assist", FRCMD_INIT, FrGameRoomDlg::OnAssistInit)
ON_FRESH_VV("assist", FRCMD_LBUTTONUP, FrGameRoomDlg::OnAssist)
ON_FRESH_VI("assist_tip", FRCMD_INIT, FrGameRoomDlg::OnAssistTipInit)
ON_FRESH_VI("assist_title", FRCMD_INIT, FrGameRoomDlg::OnAssistTitleInit)
ON_FRESH_VI("shortgame", FRCMD_INIT, FrGameRoomDlg::OnShortgameInit)
ON_FRESH_VI("group_stroke", FRCMD_INIT, FrGameRoomDlg::OnGroupStrokeInit)
ON_FRESH_VI("group_30s", FRCMD_INIT, FrGameRoomDlg::OnGroup30sInit)
ON_FRESH_VI("group_result", FRCMD_INIT, FrGameRoomDlg::OnGroupResultInit)
ON_FRESH_VI("group_gp", FRCMD_INIT, FrGameRoomDlg::OnGroupGpInit)

END_FRESH_MSGMAP()

// 00e0903c
static char* s_windFont[] = { "[font_wind.jpg" };

FrGameRoomDlg::FrGameRoomDlg()
{
	m_newBlinkTime = 0.0f;
	m_pScoreDlg = 0;
	m_pCaddie = NULL;
	m_pAztec = NULL;
	m_pClub = NULL;
	m_pMascot = NULL;
	m_pEquipItem = NULL;
	m_pCharSel = NULL;
	m_pCaddieSel = NULL;
	m_pAztecSel = NULL;
	m_pClubSel = NULL;
	m_pMascotSel = NULL;
	m_pRoomUserRankTab = 0;
	m_pTitle = NULL;
	m_pMapBg = NULL;
	m_pRoomOption = NULL;
	m_pUser = NULL;
	m_pRoomUserRank = 0;
	m_pRoomGuildRank = 0;
	m_pMap = NULL;
	m_pMapPrev = NULL;
	m_pMapNext = NULL;
	m_pMapHoleCombo = NULL;
	m_pOverGauge = NULL;
	m_pBackGround = NULL;
	m_pStart = NULL;
	m_pReady = NULL;
	m_p230 = NULL;
	m_pScore = 0;
	m_pGuildInvite = 0;
	m_pChangeTeam = NULL;
	m_pTeamNum = 0;
	m_pTeamTab[0] = 0;
	m_pTeamTab[1] = 0;
	m_pGuildTab[0] = 0;
	m_pGuildTab[1] = 0;
	m_p1b8 = NULL;
	m_240 = 0;
	m_pTreasureCourse = NULL;
	m_1b4 = 0;
	m_244 = 0;
	m_pExtFrame = 0;
	m_pChar983 = NULL;
	m_pMasterSignBmp = NULL;
	m_pReadySignBmp = NULL;
	m_pTeamSelectBmp = NULL;
	m_pDiveBmp = NULL;
	m_pGuildScoreGauge = 0;
	m_pKickBmp = NULL;
	m_38c = 0;
	m_390 = 0;
	m_bNewBlink = false;
	m_newBlinkCount = 0;
	memset(m_pPlayerBaseBmp, 0, sizeof(m_pPlayerBaseBmp));
	memset(m_380, 0, sizeof(m_380));
	memset(m_pPet, 0, sizeof(m_pPet));
	memset(m_pStatBar, 0, sizeof(m_pStatBar));
	memset(m_pStatEdit, 0, sizeof(m_pStatEdit));
	m_resultDelay = 1.0f;
	m_pCurTip = NULL;
	m_pStartTip = NULL;
	m_pReadyTip = NULL;
	m_pCountdown = 0;
	m_pModeTitle = NULL;
	m_pCourseCtrl = NULL;
	m_pChipinCtrl = NULL;
	m_pBgCourse = NULL;
	m_pFixHole = NULL;
	m_pFixHolecupPos = NULL;
	m_pSlope = NULL;
	m_pSize = NULL;
	m_pTicket = NULL;
	m_pAtft = NULL;
	m_pArtifactIcon = NULL;
	m_b18d = true;
	m_pExtResQuitDlg = 0;
	m_pRoomEvent02Bmp = NULL;
	m_pGPInfo983 = 0;
	m_pGPDetail983 = 0;
	m_pTreasureGift = 0;
	m_bUseReport = false;
	m_b3dd = false;
	m_pNewRecordDlg = 0;
	m_pTimeBoard983 = 0;
	m_154 = 0;
	m_158 = 0;
	m_pHoleFont = NULL;
	m_pUniteResultDlg = 0;
	m_pUserListDlg = NULL;
	m_pNotifyDlg = NULL;
	m_250 = 0;
	m_254 = 0;
	m_258 = 0;
	m_pUserInfo = 0;
	m_pInviteBmp = NULL;
	m_3c0 = 0;
	m_lastSlotAddTime983 = 0;
	m_waitStartTime983 = 0;
	m_slotAddSec983 = 0.0f;
	m_startWaitSec983 = 0.0f;
	m_pGroupStroke = NULL;
	m_pGroupResult = NULL;
	m_bAssistTipOn = false;
	m_bArtifactTipOn = false;
	m_3e0 = DOCJP()->GetRoomListKind983();
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa5, m_pDoc);
	m_pCheer = new CRoomCheer983(DOC983_U8(0x10950));
	m_pAssist = NULL;
	m_pAssistTip = NULL;
	m_pAssistTitle = NULL;

	WTITLEFONT info;
	memset(&info, 0, sizeof(info));
	info.filename = s_windFont;
	info.fontw = 16;
	info.fonth = 16;
	info.numPages = 1;
	info.texw = 128;
	info.texh = 64;
	info.pCharSet = "1234567890ym%-.?/:,";
	m_pFont = g_resrcmng->GetTitleFont();
	m_pFont->Create(&info);
}

FrGameRoomDlg::~FrGameRoomDlg()
{
	ReleaseRoom983();
	if (m_pHoleFont)
	{
		delete m_pHoleFont;
		m_pHoleFont = NULL;
	}
	if (g_resrcmng && m_pFont)
	{
		g_resrcmng->Release(m_pFont);
		m_pFont = NULL;
	}
}

// 005034e0 (645: CLobbyMain::ReLoadHoleItem, sem argumento)
void FrGameRoomDlg::ReLoadHoleItem(int sel)
{
	if (m_pMapHoleCombo)
	{
		m_pMapHoleCombo->ClearAllListItem();
		m_pMapHoleCombo->AddTexItem("hole_front");
		if (DOC983_U8(0x1094b) < 18)
		{
			m_pMapHoleCombo->AddTexItem("hole_back");
			m_pMapHoleCombo->AddTexItem("hole_random");
		}
		m_pMapHoleCombo->AddTexItem("hole_shuffle");
		if (sel != -1)
		{
			if (DOC983_U8(0x1094b) == 18 && sel > 0)
				sel = 1;
			m_pMapHoleCombo->SelectItem(sel);
		}
	}
}

// 00503580 (645: zerava m_subSelTypeId / m_subSelItemId; o 983 avisa o ator principal com a mensagem 0x50)
void FrGameRoomDlg::InitControlsSelectedUserInfo()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x50, 0, 0, 0, 0, 0));
	if (m_pCharSel)
		m_pCharSel->SetVisible(false);
	if (m_pCaddieSel)
		m_pCaddieSel->SetVisible(false);
	if (m_pAztecSel)
		m_pAztecSel->SetVisible(false);
	if (m_pClubSel)
		m_pClubSel->SetVisible(false);
	if (m_pMascotSel)
		m_pMascotSel->SetVisible(false);
}

void FrGameRoomDlg::OnTitleInit(int param)
{
	m_pTitle = DYNAMIC_CAST(FrEdit, param);
	if (m_pTitle == NULL)
		return;
	SetRoomTitle983(NULL);
	Doc()->SortAllMyItemList();
	// 645: `nUserLimit >= 100`; o 983 só pinta fora do modo 0x0b e com o byte +0x1095e ligado
	if (DOC983_U8(0x1094c) != 0x0b && DOC983_U8(0x1095e) != 0 && DOC983_U8(0x10937) > 99)
		m_pTitle->SetFontColor(0xff47a5dd);
	else
		m_pTitle->SetFontColor(0xffffffff);
}

void FrGameRoomDlg::OnFrameBgInit(int param)
{
	m_pBg = DYNAMIC_CAST(FrViewer, param);
	if (m_pBg)
		m_pBg->SetMouseEvent(false);
}

void FrGameRoomDlg::OnBackGroundInit(int param)
{
	m_pBackGround = DYNAMIC_CAST(FrViewer, param);
	if (m_pBackGround)
		m_pBackGround->SetMouseEvent(false);
}

void FrGameRoomDlg::OnMapBgInit(int param)
{
	m_pMapBg = DYNAMIC_CAST(FrViewer, param);
	if (m_pMapBg)
		m_pMapBg->SetMouseEvent(false);
}

void FrGameRoomDlg::OnMapInit(int param)
{
	m_pMap = DYNAMIC_CAST(FrButton, param);
	if (m_pMap == NULL)
		return;
	m_pMap->SetStyle(FrButton::BT_NORMAL);
	switch (DOC983_U8(0x1094c))
	{
	case 6:
	case 10:
		m_pMap->SetStyle(FrButton::BT_ALWAYSON);
		m_pMap->SetPushDelay(0.0f);
		m_pMap->UseDblClick(false);
		break;
	case 11:
		m_pMap->Enable(false);
		break;
	}
}

void FrGameRoomDlg::OnMapPrevInit(int param)
{
	m_pMapPrev = DYNAMIC_CAST(FrButton, param);
	if (m_pMapPrev)
	{
		m_pMapPrev->Enable(false);
		m_pMapPrev->SetVisible(false);
		m_pMapPrev->SetPushDelay(0.0f);
		m_pMapPrev->UseDblClick(false);
	}
}

void FrGameRoomDlg::OnMapNextInit(int param)
{
	m_pMapNext = DYNAMIC_CAST(FrButton, param);
	if (m_pMapNext)
	{
		m_pMapNext->Enable(false);
		m_pMapNext->SetVisible(false);
		m_pMapNext->SetPushDelay(0.0f);
		m_pMapNext->UseDblClick(false);
	}
}

void FrGameRoomDlg::OnMapHoleComboInit(int param)
{
	m_pMapHoleCombo = DYNAMIC_CAST(FrComboCtlEx, param);
	ReLoadHoleItem(-1);
	unsigned char holeType = DOC983_U8(0x1094f);
	if (m_pMapHoleCombo)
	{
		if (DOC983_U8(0x1094b) == 18)
		{
			if (holeType > 0)
				holeType = 1;
		}
		m_pMapHoleCombo->SelectItem(holeType);
	}
}

void FrGameRoomDlg::OnStartTipInit(int param)
{
	m_pStartTip = DYNAMIC_CAST(FrArea, param);
	if (m_pStartTip)
		m_pStartTip->SetVisible(false);
}

void FrGameRoomDlg::OnReadyTipInit(int param)
{
	m_pReadyTip = DYNAMIC_CAST(FrArea, param);
	if (m_pReadyTip)
		m_pReadyTip->SetVisible(false);
}

void FrGameRoomDlg::OnStartInit(int param)
{
	m_pStart = DYNAMIC_CAST(FrButton, param);
}

void FrGameRoomDlg::OnReadyInit(int param)
{
	m_pReady = DYNAMIC_CAST(FrButton, param);
	if (m_pReady)
	{
		m_pReady->SetVisible(false);
		m_pReady->Enable(false);
	}
}

void FrGameRoomDlg::OnChangeTeamBtnInit(int param)
{
	m_pChangeTeam = DYNAMIC_CAST(FrButton, param);
	if (m_pChangeTeam)
		m_pChangeTeam->SetPushDelay(0.5f);
}

void FrGameRoomDlg::OnModifyInit(int param)
{
	m_pRoomOption = DYNAMIC_CAST(FrButton, param);
	if (m_pRoomOption == NULL)
		return;
	unsigned char mode = DOC983_U8(0x109b8);
	if (mode == 0x0c || mode == 0x0e)
	{
		m_pRoomOption->Enable(false);
		m_pRoomOption->SetVisible(false);
	}
}

void FrGameRoomDlg::OnPowerBarInit(int param)
{
	m_pStatBar[0] = DYNAMIC_CAST(FrGaugeBar, param);
	m_pStatBar[0]->SetRange(0, 50, 0);
}

void FrGameRoomDlg::OnControlBarInit(int param)
{
	m_pStatBar[1] = DYNAMIC_CAST(FrGaugeBar, param);
	m_pStatBar[1]->SetRange(0, 30, 0);
}

void FrGameRoomDlg::OnAccuracyBarInit(int param)
{
	m_pStatBar[2] = DYNAMIC_CAST(FrGaugeBar, param);
	m_pStatBar[2]->SetRange(0, 30, 0);
}

void FrGameRoomDlg::OnSpinBarInit(int param)
{
	m_pStatBar[3] = DYNAMIC_CAST(FrGaugeBar, param);
	m_pStatBar[3]->SetRange(0, 30, 0);
}

void FrGameRoomDlg::OnCurveBarInit(int param)
{
	m_pStatBar[4] = DYNAMIC_CAST(FrGaugeBar, param);
	m_pStatBar[4]->SetRange(0, 30, 0);
}

void FrGameRoomDlg::OnPowerEditInit(int param)
{
	m_pStatEdit[0] = DYNAMIC_CAST(FrStatic, param);
}

void FrGameRoomDlg::OnControlEditInit(int param)
{
	m_pStatEdit[1] = DYNAMIC_CAST(FrStatic, param);
}

void FrGameRoomDlg::OnAccuracyEditInit(int param)
{
	m_pStatEdit[2] = DYNAMIC_CAST(FrStatic, param);
}

void FrGameRoomDlg::OnSpinEditInit(int param)
{
	m_pStatEdit[3] = DYNAMIC_CAST(FrStatic, param);
}

void FrGameRoomDlg::OnCurveEditInit(int param)
{
	m_pStatEdit[4] = DYNAMIC_CAST(FrStatic, param);
}

void FrGameRoomDlg::OnCaddieInit(int param)
{
	m_pCaddie = DYNAMIC_CAST(FrViewer, param);
}

void FrGameRoomDlg::OnClubInit(int param)
{
	m_pClub = DYNAMIC_CAST(FrViewer, param);
}

void FrGameRoomDlg::OnAztecInit(int param)
{
	m_pAztec = DYNAMIC_CAST(FrViewer, param);
}

void FrGameRoomDlg::OnMascotInit(int param)
{
	m_pMascot = DYNAMIC_CAST(FrViewer, param);
}

void FrGameRoomDlg::OnEquipItemInit(int param)
{
	m_pEquipItem = DYNAMIC_CAST(FrViewer, param);
}

void FrGameRoomDlg::OnCharSelInit(int param)
{
	m_pCharSel = DYNAMIC_CAST(FrViewer, param);
	if (m_pCharSel)
		m_pCharSel->SetVisible(false);
}

void FrGameRoomDlg::OnCaddieSelInit(int param)
{
	m_pCaddieSel = DYNAMIC_CAST(FrViewer, param);
	if (m_pCaddieSel)
		m_pCaddieSel->SetVisible(false);
}

void FrGameRoomDlg::OnClubSelInit(int param)
{
	m_pClubSel = DYNAMIC_CAST(FrViewer, param);
	if (m_pClubSel)
		m_pClubSel->SetVisible(false);
}

void FrGameRoomDlg::OnAztecSelInit(int param)
{
	m_pAztecSel = DYNAMIC_CAST(FrViewer, param);
	if (m_pAztecSel)
		m_pAztecSel->SetVisible(false);
}

void FrGameRoomDlg::OnMascotSelInit(int param)
{
	m_pMascotSel = DYNAMIC_CAST(FrViewer, param);
	if (m_pMascotSel)
		m_pMascotSel->SetVisible(false);
}

void FrGameRoomDlg::OnAtftInit(int param)
{
	m_pAtft = DYNAMIC_CAST(FrGroupBox, param);
	if (m_pAtft)
		m_pAtft->SetVisible(false);
}

void FrGameRoomDlg::OnAtftArtifactIconInit(int param)
{
	m_pArtifactIcon = DYNAMIC_CAST(FrArea, param);
}

void FrGameRoomDlg::OnAssistInit(int param)
{
	m_pAssist = DYNAMIC_CAST(FrButton, param);
	if (m_pAssist == NULL)
		return;
	FrButton::eButMode status;
	switch (DOC983_U8(0x109b8))
	{
	case 0x0c:
	case 0x0e:
		m_pAssist->SetVisible(false);
		break;
	default:
		status = DOCJP()->IsTrajectoryPreviewOn() ? (FrButton::eButMode)2 : (FrButton::eButMode)0;
		m_pAssist->SetStatus(status);
		break;
	}
}

void FrGameRoomDlg::OnAssistTipInit(int param)
{
	m_pAssistTip = DYNAMIC_CAST(FrArea, param);
	if (m_pAssistTip == NULL)
		return;
	switch (DOC983_U8(0x109b8))
	{
	case 0x0c:
	case 0x0e:
		m_pAssistTip->SetVisible(false);
		break;
	}
	m_assistTip = std::string(GetGameString(0x187cd));
}

void FrGameRoomDlg::OnAssistTitleInit(int param)
{
	m_pAssistTitle = DYNAMIC_CAST(FrArea, param);
	if (m_pAssistTitle == NULL)
		return;
	switch (DOC983_U8(0x109b8))
	{
	case 0x0c:
	case 0x0e:
		m_pAssistTitle->SetVisible(false);
		break;
	}
}

void FrGameRoomDlg::OnShortgameInit(int param)
{
	m_pShortGame = DYNAMIC_CAST(FrArea, param);
	if (!S5::CLASSICSRV::IsShortGame983()) // 00b305b0 (zero esconde o "shortgame")
		m_pShortGame->SetVisible(false);
}

void FrGameRoomDlg::OnGroupStrokeInit(int param)
{
	m_pGroupStroke = DYNAMIC_CAST(FrGroupBox, param);
	if (m_pGroupStroke)
		m_pGroupStroke->SetVisible(false);
}

void FrGameRoomDlg::OnGroup30sInit(int param)
{
	m_pGroup30s = DYNAMIC_CAST(FrGroupBox, param);
	if (m_pGroup30s)
		m_pGroup30s->SetVisible(false);
}

void FrGameRoomDlg::OnGroupResultInit(int param)
{
	m_pGroupResult = DYNAMIC_CAST(FrGroupBox, param);
	if (m_pGroupResult)
		m_pGroupResult->SetVisible(false);
}

void FrGameRoomDlg::OnGroupGpInit(int param)
{
	m_pGroupGp = DYNAMIC_CAST(FrGroupBox, param);
	if (m_pGroupGp)
		m_pGroupGp->SetVisible(false);
}

// 00509880
void FrGameRoomDlg::OnCloseWndUp()
{
	if (m_state != 8)
	{
		Close((eFormRet)1, true);
		return;
	}
	OnEscape983();
}

// 00503a50
void FrGameRoomDlg::OnMapPrevBtnUp()
{
	if (m_bLockControls)
		return;
	unsigned char map = PrevMap(DOC983_U8(0x10950));
	WSendPacket send((enumClientPacket)10);
	send.Encode2(0xffff);
	send.Encode1(1);
	send.Encode1(3);
	send.Encode1(map);
	send.Send((eSendTo)0);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 00503b50
void FrGameRoomDlg::OnMapNextBtnUp()
{
	if (m_bLockControls)
		return;
	unsigned char map = NextMap(DOC983_U8(0x10950), false);
	WSendPacket send((enumClientPacket)10);
	send.Encode2(0xffff);
	send.Encode1(1);
	send.Encode1(3);
	send.Encode1(map);
	send.Send((eSendTo)0);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 00503cf0
void FrGameRoomDlg::OnMapHoleComboDown()
{
	if (!m_pMapHoleCombo)
		return;
	if (m_bLockControls)
		return;
	unsigned char holeType = m_pMapHoleCombo->GetCurrentIdx();
	if (DOC983_U8(0x1094b) != 18)
	{
		if (DOC983_U8(0x1094f) == holeType)
			return;
	}
	else
	{
		if (holeType == 0)
		{
			if (DOC983_U8(0x1094f) == 0)
				return;
		}
		else
		{
			if (DOC983_U8(0x1094f) == 3)
				return;
			holeType = 3;
		}
	}
	WSendPacket send((enumClientPacket)10);
	send.Encode2(0xffff);
	send.Encode1(1);
	send.Encode1(5);
	send.Encode1(holeType);
	send.Send((eSendTo)0);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// os quatro equipamentos: o 645 abria a barra (OpenSubEquipBar) aqui; o 983 manda a mensagem 0x52 ao ator principal com o
// retângulo do "…_sel" e o grupo do item
// 005041c0
void FrGameRoomDlg::OnCaddieDown()
{
	WRect rect = m_pCaddieSel->GetRect();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x52, (int)&rect, 7, 1, 0, 0));
}

// 00504260
void FrGameRoomDlg::OnClubDown()
{
	WRect rect = m_pClubSel->GetRect();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x52, (int)&rect, 4, 1, 0, 0));
}

// 00504300
void FrGameRoomDlg::OnAztecDown()
{
	WRect rect = m_pAztecSel->GetRect();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x52, (int)&rect, 5, 1, 0, 0));
}

// 005043a0
void FrGameRoomDlg::OnMascotDown()
{
	WRect rect = m_pMascotSel->GetRect();
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x52, (int)&rect, 16, 1, 0, 0));
}

// 00504440
void FrGameRoomDlg::OnEquipItemDown()
{
	InitControlsSelectedUserInfo();
	AfxGetTask()->GetActor("Lobby") << MsgObject(NULL, 0x8b, 5, 0, 0, 0, 0);
}

// 005044b0 … 005044f0
void FrGameRoomDlg::OnCharSelDown()
{
	InitControlsSelectedUserInfo();
}

void FrGameRoomDlg::OnCaddieSelDown()
{
	InitControlsSelectedUserInfo();
}

void FrGameRoomDlg::OnClubSelDown()
{
	InitControlsSelectedUserInfo();
}

void FrGameRoomDlg::OnAztecSelDown()
{
	InitControlsSelectedUserInfo();
}

void FrGameRoomDlg::OnMascotSelDown()
{
	InitControlsSelectedUserInfo();
}

// 00503430: imagem do botão "course_ctrl.fix_holecup_pos" e o byte CSharedDoc+0x11609 (7 = posição do copo fixa ligada)
void FrGameRoomDlg::Fn503430_983(int a)
{
	if (m_pFixHolecupPos == NULL)
		return;
	if (a == 7)
	{
		DOC983_U8(0x11609) = 1;
		m_pFixHolecupPos->SetButtonImg("single_font_on", (FrButton::eButMode)0);
		m_pFixHolecupPos->SetButtonImg("single_font_on", (FrButton::eButMode)1);
		m_pFixHolecupPos->SetButtonImg("single_font_on", (FrButton::eButMode)2);
	}
	else
	{
		DOC983_U8(0x11609) = 0;
		m_pFixHolecupPos->SetButtonImg("single_font_off", (FrButton::eButMode)0);
		m_pFixHolecupPos->SetButtonImg("single_font_off", (FrButton::eButMode)1);
		m_pFixHolecupPos->SetButtonImg("single_font_off", (FrButton::eButMode)2);
	}
}

// ---- sessão B43: cliques do mapa de mensagens que faltavam e retornos de diálogo ----

// FrChangeRoomInfoDlg do 983: método novo 00acac50 (buraco fixo ligado, posição do copo, fonte dos números). Empréstimo
// até a frente A pôr na classe (pedido B43).
class FrChangeRoomInfoDlgJp : public FrChangeRoomInfoDlg
{
public:
	void SetRepeat983(bool bFixHole, unsigned char holecupPos, CRepeatHoleFont* pFont); // 00acac50
};

// 004ff270
bool FrGameRoomDlg::OnMapSelectDlgResult(int result, FrForm* form)
{
	m_pTreasureCourse = NULL;
	return true;
}

// 004ff280
bool FrGameRoomDlg::OnUserListDlgResult(int result, FrForm* form)
{
	m_pUserListDlg = NULL;
	return true;
}

// 005040a0 (645: CLobbyMain::OnGameRoom_RoomOptionBtnUp)
void FrGameRoomDlg::OnModify()
{
	if (m_pCurTip && m_pCurTip->IsVisible() && !(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xb8\xf0\xb5\xce \xc1\xd8\xba\xf1\xb0\xa1 \xb5\xc8 \xbb\xf3\xc8\xb2\xbf\xa1\xbc\xad\xb4\xc2 \xb9\xe6\xc1\xa4\xba\xb8\xb8\xa6 \xba\xaf\xb0\xe6\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9"),
			0, 0, 0, 0));
		return;
	}
	FrChangeRoomInfoDlgJp* pDlg = (FrChangeRoomInfoDlgJp*)CreateForm<FrChangeRoomInfoDlg>(g_pFresh->GetManager(), this,
		"changeroominfo", NULL);
	pDlg->SetDesc(K2L_Compatibility("\xb9\xe6\xc0\xc7 \xbf\xc9\xbc\xc7\xc0\xbb \xba\xaf\xb0\xe6\xc7\xcf\xbc\xbc\xbf\xe4."));
	if (DOC983_U8(0x109b8) == 0x13)
		pDlg->SetRepeat983(DOC983_U8(0x1094f) == 4, DOC983_U8(0x11609), m_pHoleFont);
	pDlg->Open(NULL, 1);
	pDlg->MoveCursor("ok");
}

// 005045b0 (sem par no 645): desliga o botão, avisa o ator principal (mensagem 1) e manda o pacote 0x184 vazio
void FrGameRoomDlg::OnAssist()
{
	m_pAssist->Enable(false);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
	WSendPacket send((enumClientPacket)0x184);
	send.Send((eSendTo)0);
}

// 005076c0 (645: CLobbyMain::OnGameRoom_ChangeTeamBtnUp)
void FrGameRoomDlg::OnChangeTeamBtnUp()
{
	if (m_bReady || m_bLockControls)
		return;
	CSharedDoc* pDoc = Doc();
	unsigned long guid = MyGuid(false);
	std::list<sSlotInfo>::iterator it = std::find(pDoc->m_slotList.begin(), pDoc->m_slotList.end(), guid);
	if (it == pDoc->m_slotList.end())
		return;
	(*it).bTeam = 1 - (*it).bTeam;
	WSendPacket send((enumClientPacket)16);
	send.Encode1((*it).bTeam);
	send.Send((eSendTo)0);
}

// 00505a70 (645: CLobbyMain::OnGameRoom_MapBtnUp)
void FrGameRoomDlg::OnMapBtnUp()
{
	unsigned long identity = Doc()->m_myInfo.info.dwIdentity;
	if (ROOMDOC_MASTER(m_pDoc) != MyGuid(false)
		|| (m_bCanStart && Doc()->m_slotList.size() > 1 && !(bool)((identity >> 2) & 1)))
		return;
	if (IS_GP_MODE983())
		return;
	if (DOC983_U8(0x1094c) == 6 || (DOC983_U8(0x1094c) > 9 && DOC983_U8(0x1094c) <= 11))
		return;
	if (m_pTreasureCourse)
		return;
	m_pTreasureCourse = CreateForm<FrTreasureCourse>(g_pFresh->GetManager(), this, "treasuremap", NULL);
	if (m_pTreasureCourse)
	{
		m_pTreasureCourse->SetSelectMap(DOC983_U8(0x10950));
		m_pTreasureCourse->SetMapIndexChanged(false);
		if (DOCJP()->GetState983() == 7)
			m_pTreasureCourse->EnableRandomBtn(false);
		else
			m_pTreasureCourse->EnableRandomBtn(true);
		m_pTreasureCourse->LockRandom(Doc()->m_golfGame.gameType == 2);
		m_pTreasureCourse->EnableDrag(false);
		m_pTreasureCourse->Open((FRESH_PFN_RESULT)&FrGameRoomDlg::OnMapSelectDlgResult, 3);
	}
}

// 00500760 (645: CLobbyMain::DrawFrameSelectedUserInfo)
void FrGameRoomDlg::DrawFrameSelectedUserInfo(WRect rect)
{
	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice == NULL)
		return;

	if (rect.IsInRect(
			WPoint(g_input->GetMousePoint().x, g_input->GetMousePoint().y)))
	{
		const Bitmap* pFrame = g_pFresh->GetBitmap("gameroom_select_frame");
		if (pFrame)
			pDevice->DrawTexture(pFrame,
				WRect(rect.x - 2.0f, rect.y - 2.0f, pFrame->Width(),
					pFrame->Height()),
				0xffffffff, false);
	}
}

// 00501710 (645: CLobbyMain::OnGameRoom_EquipItemOwnerDraw)
void FrGameRoomDlg::OnEquipItemOwnerDraw(int param)
{
	if (m_pEquipItem)
	{
		FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();

		if (pGDI)
		{
			unsigned long color =
				m_pEquipItem->IsEnabled() ? 0xffffffff : 0x50b0b0b0;

			const Bitmap* pBitmap = g_pFresh->GetManager()->GetBitmap("ITEMS",
				"gameroom_equip_item_bn");

			if (pBitmap)
			{
				pGDI->DrawTexture(pBitmap,
					WRect(m_pEquipItem->GetRect().x, m_pEquipItem->GetRect().y,
						(float)pBitmap->Width(), (float)pBitmap->Height()),
					color, 0);
			}

			if (m_pEquipItem->IsEnabled())
				DrawFrameSelectedUserInfo(m_pEquipItem->GetRect());
		}
	}
}

// 0050a430 (645: CLobbyMain::OnGameRoom_MascotOwnerDraw)
void FrGameRoomDlg::OnMascotOwnerDraw(int param)
{
	if (m_pMascot == NULL)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice == NULL)
		return;

	unsigned long color = m_pMascot->IsEnabled() ? 0xffffffff : 0x50b0b0b0;

	if (Doc()->m_myInfo.userEquip.guidMascot == 0)
	{
		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "w_caddie_no");
		if (pBitmap)
			pDevice->DrawTexture(pBitmap,
				WRect(m_pMascot->GetRect().x, m_pMascot->GetRect().y,
					(float)pBitmap->Width(), (float)pBitmap->Height()),
				color, false);
	}
	else
	{
		std::map<unsigned int, sMascotInfo>::iterator it =
			Doc()->m_mascotMap.find(Doc()->m_myInfo.userEquip.guidMascot);
		IFF_ITEM_COMMON* pItem =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pItem)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
			if (pBitmap)
				pDevice->DrawTexture(pBitmap,
					WRect(m_pMascot->GetRect().x, m_pMascot->GetRect().y,
						(float)pBitmap->Width(), (float)pBitmap->Height()),
					color, false);
		}
	}

	if (m_pMascot->IsEnabled())
		DrawFrameSelectedUserInfo(m_pMascot->GetRect());
}

// 00507780 (645: CLobbyMain::OnGameRoom_AztecOwnerDraw)
void FrGameRoomDlg::OnAztecOwnerDraw(int param)
{
	if (m_pAztec == NULL)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice == NULL)
		return;

	unsigned long color = m_pAztec->IsEnabled() ? 0xffffffff : 0x50b0b0b0;

	if (Doc()->m_myInfo.userEquip.tidBall == 0)
	{
		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "w_caddie_no");
		if (pBitmap)
			pDevice->DrawTexture(pBitmap, m_pAztec->GetRect(), color, false);
	}
	else
	{
		IFF_ITEM_COMMON* pItem =
			ItemManager()->FindCommonItem(Doc()->m_myInfo.userEquip.tidBall);

		if (pItem)
		{
			int count = 0;
			if (IsLocalContent((localContentType_t)0x84 /*983: era 0x87; medido*/))
			{
				sItemInfo* pInfo = NULL;
				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_ballList.begin();
					it != Doc()->m_ballList.end(); ++it)
				{
					if ((*it).tid == Doc()->m_myInfo.userEquip.tidBall)
					{
						pInfo = &(*it);
						if (ItemManager()->IsUnlimitAztec(Doc()->m_myInfo.userEquip.tidBall))
							count = 0;
						else
							count = (*it).Common[0];
						break;
					}
				}

				if (pInfo == NULL)
				{
					Doc()->m_myInfo.userEquip.tidBall = 0x14000000;
					pItem = ItemManager()->FindCommonItem(
						Doc()->m_myInfo.userEquip.tidBall);
				}

				const Bitmap* pBitmap =
					g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
				if (pBitmap)
					pDevice->DrawTexture(pBitmap,
						WRect(m_pAztec->GetRect().x, m_pAztec->GetRect().y,
							(float)pBitmap->Width(), (float)pBitmap->Height()),
						color, false);
			}
			else
			{
				const Bitmap* pBitmap =
					g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
				if (pBitmap)
					pDevice->DrawTexture(pBitmap,
						WRect(m_pAztec->GetRect().x, m_pAztec->GetRect().y,
							(float)pBitmap->Width(), (float)pBitmap->Height()),
						color, false);

				std::list<sItemInfo>::iterator it;
				for (it = Doc()->m_ballList.begin();
					it != Doc()->m_ballList.end(); ++it)
				{
					if ((*it).tid == Doc()->m_myInfo.userEquip.tidBall)
					{
						count = (*it).Common[0];
						break;
					}
				}
			}

			if (count > 0 && Doc()->m_myInfo.userEquip.tidBall != 0x140000d8)
			{
				unsigned long textColor =
					m_pAztec->IsEnabled() ? 0xffffffff : 0x7fffffff;
				float x = m_pAztec->GetRect().x + 35.0;
				float y = m_pAztec->GetRect().y + 22.0;
				pDevice->PrintTitleFont983(m_pFont, x, y, MakeStr("%d", count), 1.0f, 2, 1.0f,
					textColor);
			}
		}
	}

	if (m_pAztec->IsEnabled())
		DrawFrameSelectedUserInfo(m_pAztec->GetRect());
}

// 00509f90 (645: CLobbyMain::OnGameRoom_CaddieOwnerDraw)
void FrGameRoomDlg::OnCaddieOwnerDraw(int param)
{
	if (m_pCaddie == NULL)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice == NULL)
		return;

	unsigned long color = m_pCaddie->IsEnabled() ? 0xffffffff : 0x50b0b0b0;

	WRect rect = m_pCaddie->GetRect();
	if (Doc()->m_myInfo.userEquip.guidCaddie == 0)
	{
		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "w_caddie_no");
		if (pBitmap)
			pDevice->DrawTexture(pBitmap,
				WRect(m_pCaddie->GetRect().x, m_pCaddie->GetRect().y,
					(float)pBitmap->Width(), (float)pBitmap->Height()),
				color, false);
	}
	else
	{
		std::map<unsigned int, sCaddieInfo>::iterator it =
			Doc()->m_caddieMap.find(Doc()->m_myInfo.userEquip.guidCaddie);

		IFF_ITEM_COMMON* pItem =
			ItemManager()->FindCommonItem((*it).second.tid);
		if (pItem)
		{
			const Bitmap* pBitmap =
				g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
			if (pBitmap)
				pDevice->DrawTexture(pBitmap,
					WRect(m_pCaddie->GetRect().x, m_pCaddie->GetRect().y,
						(float)pBitmap->Width(), (float)pBitmap->Height()),
					color, false);
		}

		IFF_STRUCT::sCaddie* pCaddie =
			ItemManager()->FindCaddie((*it).second.tid);
		if (pCaddie && (*it).second.Rent_flag &&
			(*it).second.Remain_Date <= 0 && (*it).second.tidPart == 0 &&
			pCaddie->MonthlyFee)
		{
			g_view->DrawLine2D(WPoint(rect.x, rect.h * 0.5f + rect.y - 1.0f),
				WPoint(rect.x + rect.w - 1.0f, rect.h * 0.5f + rect.y - 1.0f),
				0xffff0000, 0xffff0000, 0);
			g_view->DrawLine2D(WPoint(rect.x, rect.h * 0.5f + rect.y + 0.0f),
				WPoint(rect.x + rect.w - 1.0f, rect.h * 0.5f + rect.y + 0.0f),
				0xffff0000, 0xffff0000, 0);
			g_view->DrawLine2D(WPoint(rect.x, rect.h * 0.5f + rect.y + 1.0f),
				WPoint(rect.x + rect.w - 1.0f, rect.h * 0.5f + rect.y + 1.0f),
				0xffff0000, 0xffff0000, 0);
		}
	}

	if (m_pCaddie->IsEnabled())
		DrawFrameSelectedUserInfo(m_pCaddie->GetRect());
}

// 0050a2a0 (645: CLobbyMain::OnGameRoom_ClubOwnerDraw)
void FrGameRoomDlg::OnClubOwnerDraw(int param)
{
	if (m_pClub == NULL)
		return;

	FrGraphicInterface* pDevice = g_pFresh->GetManager()->GetGDI();
	if (pDevice == NULL)
		return;

	unsigned long color = m_pClub->IsEnabled() ? 0xffffffff : 0x50b0b0b0;

	if (Doc()->m_myInfo.userEquip.guidClubSet == 0)
	{
		const Bitmap* pBitmap =
			g_pFresh->GetManager()->GetBitmap("ITEMS", "w_caddie_no");
		if (pBitmap)
			pDevice->DrawTexture(pBitmap, m_pClub->GetRect(), color, false);
	}
	else
	{
		std::map<unsigned int, sItemInfo>::iterator it =
			Doc()->m_clubSetMap.find(Doc()->m_myInfo.userEquip.guidClubSet);
		if (it != Doc()->m_clubSetMap.end())
		{
			IFF_ITEM_COMMON* pItem =
				ItemManager()->FindCommonItem((*it).second.tid);
			if (pItem)
			{
				const Bitmap* pBitmap =
					g_pFresh->GetBitmap(MakeStr("w_%s", pItem->Icon));
				if (pBitmap)
					pDevice->DrawTexture(pBitmap,
						WRect(m_pClub->GetRect().x, m_pClub->GetRect().y,
							(float)pBitmap->Width(), (float)pBitmap->Height()),
						color, false);
			}
		}
	}

	if (m_pClub->IsEnabled())
		DrawFrameSelectedUserInfo(m_pClub->GetRect());
}

// 00505bc0 (645: CLobbyMain::OnGameRoom_ReadyBtnUp). Novo no 983: no Grand Prix (modos 0x14/0x15) com 0062bf30 em zero o
// botão manda o pacote 0x0e (uid), desliga-se e toca "gp_in_lobby_start_5sec"; no fim avisa o ator principal (0x4f, pronto).
void FrGameRoomDlg::OnReadyBtnUp()
{
	if (DOCJP()->IsControlServerService983(2))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
		return;
	}
	if (MyGuid(false) == ROOMDOC_MASTER(m_pDoc))
		return;
	if (m_bLockControls || m_bRoomStateReq)
		return;
	m_bRoomStateReq = true;
	if (IS_GP_MODE983() && DOCJP()->GetGPChannel983() == 0)
	{
		WSendPacket send((enumClientPacket)14);
		send.Encode4(MyGuid(false));
		send.Send((eSendTo)0);
		if (m_pReady)
			m_pReady->Enable(false);
		if (g_audio)
			g_audio->PlaySfx("gp_in_lobby_start_5sec");
	}
	else if (m_bReady)
	{
		WSendPacket send((enumClientPacket)13);
		send.Encode1(1);
		send.Send((eSendTo)0);
	}
	else
	{
		m_bNewBlink = false;
		if (m_pCurTip)
			m_pCurTip->SetVisible(false);
		WSendPacket send((enumClientPacket)13);
		send.Encode1(0);
		send.Send((eSendTo)0);
	}
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x4f, m_bReady != false, 0, 0, 0, 0));
	EnableSettingButton(m_bReady);
}

// ---- Sessão B44 (3ª parte): começar ----

bool LatestVersion983(); // 0059fa80 (645: CLobbyMain::LatestVersion; no 983 não usa o `this`)

// Nó das listas de itens do 983 lido à mão (PROVISÓRIO: a lista de CSharedDoc+0x375c é da frente A)
struct sItemNode983
{
	sItemNode983* pNext; // +0x00
	sItemNode983* pPrev; // +0x04
	unsigned long id; // +0x08
	unsigned long tid; // +0x0c
	unsigned long d10; // +0x10
	short count; // +0x14
};

// Unidade vizinha 00478cc2–0047a060 (ainda sem dono): `CItemCursor983` (cópia da lista de itens com um cursor) vem do
// itemcursor983.h da frente A (incluído pelo changeroominfodlg.h; sessão A118).
#include "itemcursor983.h"

// 00479bc0: pode a sala começar com o artefato escolhido? Declaração da frente A (corpo no changeroominfodlg.cpp; A159.2).
#include "artifactcheck983.h"

#define DOC983_AT(type, off) (*(type*)DOCPTR983(off))

// 00507ad0
bool FrGameRoomDlg::CanUseStartTicket983()
{
	if (DOC983_AT(bool, 0x10934) && DOC983_U8(0x1094c) == 4)
	{
		sItemNode983* pHead = DOC983_AT(sItemNode983*, 0x375c);
		sItemNode983* p;
		for (p = pHead->pNext; p != pHead; p = p->pNext)
		{
			if (p->tid == 0x1a000137)
				break;
		}
		if (p != pHead && p->count > 0)
			return true;
	}
	return false;
}

// 00505e20
bool FrGameRoomDlg::OnStartTicketResult983(int result, FrForm* form)
{
	if (result == 1)
	{
		if (m_pStart)
			m_pStart->Enable(false);

		WSendPacket packet((enumClientPacket)14);
		packet.Encode4(MyGuid(false));
		packet.Send((eSendTo)0);

		if (IsLocalContent(S4_INVITE_FRIEND))
			CMessengerInfo::Instance()->EnableInvite(false);
	}
	return true;
}

// 00509960 (645: CLobbyMain::OnGameRoom_StartBtnUp). Novo no 983: (1) torneio com menos jogadores que o mínimo e o bilhete
// 0x1a000137 no inventário abre o "notify_yesno" (gastar um bilhete e começar já); (2) o artefato da sala (+0x109b9) é
// conferido antes do pacote 0x0e; (3) GM no Grand Prix com 0062bf30 diferente de zero não manda o pacote.
void FrGameRoomDlg::OnStartBtnUp()
{
	if (m_pStart == NULL || !m_pStart->IsEnabled())
		return;

	if (DOCJP()->IsControlServerService983(2))
	{
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xb0\xd4\xc0\xd3 \xb0\xfc\xb7\xc3 \xba\xce\xba\xd0 \xc1\xa1\xb0\xcb\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9."), 0, 0, 0, 0));
		return;
	}

	if (MyGuid(false) == ROOMDOC_MASTER(m_pDoc))
	{
		if (!LatestVersion983())
			return;

		if ((Doc()->m_curChannel.Type & 0x800) &&
			!(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
		{
			IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(
				((Doc()->m_roomInfo.mapType > 0x7f &&
					 Doc()->m_roomInfo.mapType != 0xfd)
						? Doc()->m_roomInfo.mapType - 0x80
						: Doc()->m_roomInfo.mapType) |
				0x28000000);
			if (pCourse == NULL)
				return;

			if (pCourse->Difficulty >= 3 ||
				((Doc()->m_curChannel.Type & 0x800) &&
					((((Doc()->m_roomInfo.mapType > 0x7f &&
						   Doc()->m_roomInfo.mapType != 0xfd)
							  ? Doc()->m_roomInfo.mapType - 0x80
							  : Doc()->m_roomInfo.mapType) |
						 0x28000000) == 1 ||
						(((Doc()->m_roomInfo.mapType > 0x7f &&
							  Doc()->m_roomInfo.mapType != 0xfd)
								 ? Doc()->m_roomInfo.mapType - 0x80
								 : Doc()->m_roomInfo.mapType) |
							0x28000000) == 11 ||
						(((Doc()->m_roomInfo.mapType > 0x7f &&
							  Doc()->m_roomInfo.mapType != 0xfd)
								 ? Doc()->m_roomInfo.mapType - 0x80
								 : Doc()->m_roomInfo.mapType) |
							0x28000000) == 14)))
			{
				AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
					(int)K2L_Compatibility("\xb7\xe7\xc5\xb0\xc3\xa4\xb3\xce\xbf\xa1\xbc\xad\xb4\xc2 \xc0\xcc \xb8\xca\xc0\xbb \xc7\xc3\xb7\xb9\xc0\xcc \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					0, 0, 0, 0));
				return;
			}
		}

		bool bGM = (Doc()->m_myInfo.info.dwIdentity & 4) == 4;
		if (!bGM && !(Doc()->m_myInfo.info.dwIdentity & 0x8000) &&
			CanUseStartTicket983() == true &&
			DOC983_AT(unsigned long, 0x10a00) < (unsigned long)((DOC983_U8(0x10af3) & 2) ? 5 : 10))
		{
			IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(0x1a000137);
			if (pItem)
			{
				FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno", NULL);
				if (pForm)
				{
					pForm->SetMessage(MakeStr(K2L_Compatibility(
						"\xb4\xeb\xc8\xb8\xb8\xa6 \xbd\xc3\xc0\xdb\xc7\xcf\xb4\xc2 \xb5\xa5\xbf\xa1\n\xc7\xca\xbf\xe4\xc7\xd1 \xc0\xaf\xc0\xfa\xc0\xc7 \xbc\xf6\xb0\xa1 \xba\xce\xc1\xb7\xc7\xd5\xb4\xcf\xb4\xd9.\n\n%s\xc0\xbb 1\xc0\xe5 \xbb\xe7\xbf\xeb\xc7\xcf\xbf\xa9\n\xb0\xd4\xc0\xd3\xc0\xbb \xb9\xd9\xb7\xce \xbd\xc3\xc0\xdb\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
						pItem->Name), false);
					pForm->Open((FRESH_PFN_RESULT)&FrGameRoomDlg::OnStartTicketResult983, 1);
				}
			}
			return;
		}

		unsigned long* pArtifact = &DOC983_AT(unsigned long, 0x109b9);
		CItemCursor983 items(DOCPTR983(0x375c));
		if (!items.Find(pArtifact))
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
				(int)K2L_Compatibility("\xbc\xb3\xc1\xa4\xb5\xc8 \xbe\xc6\xc6\xbc\xc6\xd1\xc6\xae\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xb9\xe6\xbc\xb3\xc1\xa4 \xbc\xf6\xc1\xa4\xc0\xcc \xc7\xca\xbf\xe4\xc7\xd5\xb4\xcf\xb4\xd9."),
				0, 0, 0, 0));
			return;
		}

		unsigned char flag = items.GetFlag();
		sArtifactCheckArg983 arg;
		arg.pFlag = &flag;
		arg.pRoomType = &DOC983_U8(0x1094c);
		arg.pMode = &DOC983_U8(0x109b8);
		arg.p1094b = &DOC983_U8(0x1094b);
		sArtifactCheck983 check(pArtifact, &arg);
		if (check.bFail)
		{
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23, (int)check.text, 0, 0, 0, 0));
			return;
		}

		if (m_pStart)
			m_pStart->Enable(false);

		if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1) && IS_GP_MODE983() &&
			DOCJP()->GetGPChannel983() != 0)
			return;

		WSendPacket packet((enumClientPacket)14);
		packet.Encode4(MyGuid(false));
		packet.Send((eSendTo)0);

		if (IsLocalContent(S4_INVITE_FRIEND))
			CMessengerInfo::Instance()->EnableInvite(false);
	}
	else
	{
		if (m_bLockControls || m_bRoomStateReq)
			return;

		m_bRoomStateReq = true;

		if (m_bReady)
		{
			WSendPacket packet((enumClientPacket)13);
			packet.Encode1(1);
			packet.Send((eSendTo)0);
		}
		else
		{
			m_bNewBlink = false;
			if (m_pCurTip)
				m_pCurTip->SetVisible(false);

			WSendPacket packet((enumClientPacket)13);
			packet.Encode1(0);
			packet.Send((eSendTo)0);
		}

		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x4f, m_bReady != false, 0, 0, 0, 0));
		EnableSettingButton(m_bReady);
	}
}

// ---- Sessão B44: handlers que só os mapas das derivadas usam (lista de jogadores, "overgauge", "invite") ----

// documento 0xa5: +0x0c = jogador escolhido na lista (`m_selOID` do 645), +0x10 = uid dele (`m_selUID`), +0x18 = byte que
// tranca o botão direito no estado 6
#define ROOMDOC_SEL_OID(p) (static_cast<CLobbyContainer983*>(p)->m_selOID) // A844: era +0x0c cru
#define ROOMDOC_SEL_UID(p) (static_cast<CLobbyContainer983*>(p)->m_selUID) // A844: era +0x10 cru
#define ROOMDOC_B18(p) (static_cast<CLobbyContainer983*>(p)->m_b18) // A844: era +0x18 cru

// 008a9ae0: o `CUserInfo::SetInfo` do 983 tem dois argumentos a mais (zero aqui). PROVISÓRIO: pedido B → A (B44).
class CUserInfoJp
{
public:
	void SetInfo983(unsigned long uid, unsigned long guid, bool bCheckTime, bool bOption, bool bMsnControl,
		bool bControlOff, std::string nickname, int a8, int a9);
};

// 004ff320 (645: CLobbyMain::Banish)
void FrGameRoomDlg::Banish()
{
	if (ROOMDOC_SEL_OID(m_pDoc) != 0xffffffff)
	{
		WSendPacket packet((enumClientPacket)0x26);
		packet.Encode4(ROOMDOC_SEL_UID(m_pDoc));
		packet.Send((eSendTo)0);
	}
}

// 00500980 (645: CLobbyMain::OnGameRoom_RoomUserInit)
void FrGameRoomDlg::OnUserInit(int param)
{
	m_pUser = DYNAMIC_CAST(FrListBox, (FrWnd*)param);
	if (m_pUser)
	{
		m_pUser->UseRightButton(true);
		m_pUser->UseDummy(true);

		m_pTeamBmp[0] = (Bitmap*)m_pUser->GetBitmap("team_red");
		m_pTeamBmp[1] = (Bitmap*)m_pUser->GetBitmap("team_blue");
		m_pTeamBaseBmp = (Bitmap*)m_pUser->GetBitmap("team_base");
		m_pTeamBase02Bmp = (Bitmap*)m_pUser->GetBitmap("team_base02");
		m_pInviteBmp = (Bitmap*)m_pUser->GetBitmap("invite");

		m_pMasterSignBmp = (Bitmap*)m_pUser->GetBitmap("master_sign");
		m_pReadySignBmp = (Bitmap*)m_pUser->GetBitmap("ready_sign");
		m_pTeamSelectBmp = (Bitmap*)m_pUser->GetBitmap("team_select");

		m_pGenderBmp[0] = (Bitmap*)m_pUser->GetBitmap("i_male");
		m_pGenderBmp[1] = (Bitmap*)m_pUser->GetBitmap("i_female");
		m_pGenderBmp[2] = (Bitmap*)m_pUser->GetBitmap("i_male_02");
		m_pGenderBmp[3] = (Bitmap*)m_pUser->GetBitmap("i_female_02");
		m_pGenderBmp[4] = (Bitmap*)m_pUser->GetBitmap("i_male_03");
		m_pGenderBmp[5] = (Bitmap*)m_pUser->GetBitmap("i_female_03");
		m_pGenderBmp[6] = (Bitmap*)m_pUser->GetBitmap("i_male_manner");
		m_pGenderBmp[7] = (Bitmap*)m_pUser->GetBitmap("i_female_manner");
		m_pGenderBmp[8] = (Bitmap*)m_pUser->GetBitmap("i_male_angel");
		m_pGenderBmp[9] = (Bitmap*)m_pUser->GetBitmap("i_female_angel");

		m_pDiveBmp = (Bitmap*)m_pUser->GetBitmap("dive");

		m_pKickBmp = (Bitmap*)g_pFresh->RegisterBitmap("kick");
	}
}

// 00503e30 (645: CLobbyMain::OnGameRoom_MapGaugeInit). Novo no 983: some na sala do tipo 0x0b.
void FrGameRoomDlg::OnOvergaugeInit(int param)
{
	m_pOverGauge = DYNAMIC_CAST(FrGaugeBarImage, (FrWnd*)param);
	if (m_pOverGauge)
	{
		if (DOC983_U8(0x1094c) == 0x0b)
		{
			m_pOverGauge->SetVisible(false);
			return;
		}
		m_pOverGauge->SetRange(0, 300, 0);
		if (DOC983_U8(0x10950) < 127)
		{
			unsigned long gauge = CTHunter::Instance()->GetTHunterGauge(
				(eMapType)DOC983_U8(0x10950));
			m_pOverGauge->SetDestPos(gauge < 701 ? 0 : gauge - 700, false);
		}
		else
		{
			m_pOverGauge->SetPos(0);
		}
	}
}

// 00503f30 (645: CLobbyMain::OnGameRoom_UserListBtnUp)
void FrGameRoomDlg::OnInvite()
{
	if (m_pUserListDlg)
		return;

	m_pUserListDlg = CreateForm<FrUserListDlg>(g_pFresh->GetManager(), this,
		"userlist", NULL);
	if (!m_pUserListDlg)
		return;

	m_pUserListDlg->MakeUserList(Doc()->m_briefUserInfoMap);
	m_pUserListDlg->Open((FRESH_PFN_RESULT)&FrGameRoomDlg::OnUserListDlgResult, 3);

	WPoint pos;
	pos.x = g_view->GetWidth() - m_pUserListDlg->GetRect().w - 5.0f;
	pos.y = m_pUserListDlg->GetRect().y;
	m_pUserListDlg->MoveWindow(pos);

	if (m_pUserListDlg)
		m_pUserListDlg->EnableRightButton(!m_bReady);
}

// 00505800 (645: CLobbyMain::OnGameRoom_RoomUserLBtnUp). No 983 o jogador escolhido mora no documento 0xa5; no Grand Prix
// um lugar com o byte +0x159 (IA) não pode ser escolhido nem expulso; "GAMEROOM" virou o estado 3 do CSharedDoc.
void FrGameRoomDlg::OnUserLButtonDown()
{
	if (m_pUser == NULL)
		return;

	FrListItem* pItem = m_pUser->GetItemUnderCursor();
	if (pItem == NULL)
		return;

	sSlotInfo* pSlot = (sSlotInfo*)pItem->pData;
	if (pSlot == NULL)
		return;

	if (IS_GP_MODE983() && pSlot->bGrandPrixAI983)
	{
		ROOMDOC_SEL_OID(m_pDoc) = 0xffffffff;
		ROOMDOC_SEL_UID(m_pDoc) = 0xffffffff;
	}
	else if (pSlot->dwGuid != ROOMDOC_SEL_OID(m_pDoc))
	{
		ROOMDOC_SEL_OID(m_pDoc) = pSlot->dwGuid;
		ROOMDOC_SEL_UID(m_pDoc) = pSlot->dwUserUID;
	}

	WRect kickRect;
	if (DOCJP()->GetState983() == 3)
		kickRect = WRect((float)m_pTeamBmp[pSlot->bTeam]->Width() - 8.0f -
				(float)m_pKickBmp->Width(),
			8.0f, (float)m_pKickBmp->Width(), (float)m_pKickBmp->Height());
	else
		kickRect = WRect(227.0f, 5.0f, (float)m_pKickBmp->Width(),
			(float)m_pKickBmp->Height());

	if (IS_GP_MODE983() && pSlot->bGrandPrixAI983)
		return;

	FrWndManager* pManager = g_pFresh->GetManager();
	if (strcmp(pManager->GetLayoutID(), "GAMEROOM_EXTRES") != 0 &&
		ROOMDOC_MASTER(m_pDoc) == MyGuid(false) && ROOMDOC_SEL_OID(m_pDoc) != MyGuid(false) &&
		WRect(pItem->pos.x + kickRect.x, pItem->pos.y + kickRect.y, kickRect.w, kickRect.h)
			.IsInRect(pManager->GetMousePos()) &&
		DOC983_U8(0x1094c) != 6)
	{
		if (IsLocalContent(S4_INVITE_FRIEND))
		{
			if (pSlot->IsInvite)
				return;
		}
		Banish();
	}
}

// 005098a0 (645: CLobbyMain::OnGameRoom_RoomUserRBtnUp)
void FrGameRoomDlg::OnUserRButtonUp()
{
	if (m_state == 6 && ROOMDOC_B18(m_pDoc))
		return;

	OnUserLButtonDown();

	FrListItem* pItem = m_pUser->GetItemUnderCursor();
	if (pItem == NULL || pItem->pData == NULL)
		return;

	if (CUserInfo::IsInstantiated())
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it =
			Doc()->m_briefUserInfoMap.find(ROOMDOC_SEL_OID(m_pDoc));
		if (it != Doc()->m_briefUserInfoMap.end())
			(*it).second.roomIndex = Doc()->m_roomInfo.roomGuid;

		((CUserInfoJp*)CUserInfo::Instance())->SetInfo983((*it).second.dwUid, ROOMDOC_SEL_OID(m_pDoc), true, true,
			false, false, (*it).second.sNick, 0, 0);
	}
}

// 005038d0 (645: CLobbyMain::OnGameRoom_RoomUserDClick)
void FrGameRoomDlg::OnUserDblClick()
{
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (Doc()->m_myInfo.info.dwGalleryGuid != ROOMDOC_SEL_OID(m_pDoc))
		{
			WSendPacket send((enumClientPacket)0x3f);
			send.Encode4(ROOMDOC_SEL_UID(m_pDoc));
			send.Send((eSendTo)0);
		}
	}
}

static inline int float2int(float f) // no 983 sai embutida (`call` 00c3c320 = conversão do CRT)
{
	return (int)f;
}

// 0050d7d0 (645: CLobbyMain::OnGameRoom_RoomUserOwnerDraw). No 983: dono e escolhido vêm do documento 0xa5, o tipo de sala
// do byte +0x1094c, `m_bBongdariShop2` de CSharedDoc+0x11410; o teste do servidor (`property & 4`) saiu.
void FrGameRoomDlg::OnUserDraw(int param)
{
	FrListItem* pItem = (FrListItem*)param;
	if (!pItem)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	if (!pGDI)
		return;

	if (!pItem->pData)
	{
		pGDI->DrawTexture(m_pTeamBase02Bmp,
			WRect(pItem->pos.x, pItem->pos.y, (float)m_pTeamBase02Bmp->Width(),
				(float)m_pTeamBase02Bmp->Height()),
			0xffffffff, 0);
		return;
	}

	if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
	{
		if (((sSlotInfo*)pItem->pData)->IsInvite)
		{
			pGDI->DrawTexture(m_pInviteBmp,
				WRect(pItem->pos.x, pItem->pos.y, (float)m_pInviteBmp->Width(),
					(float)m_pInviteBmp->Height()),
				0xffffffff, 0);
		}
	}

	sSlotInfo* pSlot = (sSlotInfo*)pItem->pData;
	if (!pSlot)
		return;

	if (!Doc()->m_roomSlotMap[pSlot->dwGuid].pExhibition)
	{
		Doc()->m_roomSlotMap[pSlot->dwGuid].pArea =
			m_pPet[pSlot->connectionRank - 1];
		Doc()->m_roomSlotMap[pSlot->dwGuid].pExhibition = NULL;
		Doc()->m_roomSlotMap[pSlot->dwGuid].bAngelWing = pSlot->angelicWings;
		Doc()->m_roomSlotMap[pSlot->dwGuid].bGachaWing =
			pSlot->angelicWingsEffect;

		if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
		{
			if (!pSlot->IsInvite)
				ShowPet(pSlot->dwGuid);
		}
		else
		{
			ShowPet(pSlot->dwGuid);
		}
	}

	if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
	{
		WRect rect(pItem->pos.x, pItem->pos.y, (float)m_pTeamBaseBmp->Width(),
			(float)m_pTeamBaseBmp->Height());

		if (!pSlot->IsInvite)
		{
			if (!DOC983_AT(bool, 0x11410))
			{
				std::map<unsigned long, sRoomSlot>::iterator it =
					Doc()->m_roomSlotMap.find(pSlot->dwGuid);
				if (it != Doc()->m_roomSlotMap.end())
				{
					if ((*it).second.pExhibition)
						((CExhibition983*)(*it).second.pExhibition)->Display(0.0f, 0.0f);
				}
			}

			if (DOC983_U8(0x1094c) == 1)
				pGDI->DrawTexture(m_pTeamBmp[pSlot->bTeam], rect, 0xffffffff,
					0);
			else
				pGDI->DrawTexture(m_pTeamBaseBmp, rect, 0xffffffff, 0);
		}
		else
		{
			pGDI->DrawTexture(m_pInviteBmp, rect, 0xffffffff, 0);
		}

		if (pSlot->tidSkin[3])
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(pSlot->tidSkin[3]);
			if (pSkin)
			{
				const Bitmap* pBmp = g_pFresh->GetBitmap(pSkin->Tex);
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}
		}

		if (Doc()->m_myInfo.info.dwGuid == ROOMDOC_MASTER(m_pDoc) && !pSlot->bMaster)
		{
			pGDI->DrawTexture(m_pKickBmp,
				WRect(rect.x + rect.w - 8.0f - m_pKickBmp->Width(),
					rect.y + 8.0f, (float)m_pKickBmp->Width(),
					(float)m_pKickBmp->Height()),
				0xffffffff, 0);
		}

		if (pSlot->dwGuid == ROOMDOC_SEL_OID(m_pDoc))
			pGDI->DrawTexture(m_pTeamSelectBmp, rect, 0xffffffff, 0);

		int px = (int)pItem->pos.x;
		int x =
			(m_pTeamBmp[pSlot->bTeam]->Width() - m_pReadySignBmp->Width()) / 2 + px;

		if (pSlot->bMaster)
		{
			rect = WRect((float)x, pItem->pos.y + 6.0f,
				(float)m_pMasterSignBmp->Width(),
				(float)m_pMasterSignBmp->Height());
			pGDI->DrawTexture(m_pMasterSignBmp, rect, 0xffffffff, 0);
		}
		else if (pSlot->bReady)
		{
			rect = WRect((float)x, pItem->pos.y + 6.0f,
				(float)m_pReadySignBmp->Width(),
				(float)m_pReadySignBmp->Height());
			pGDI->DrawTexture(m_pReadySignBmp, rect, 0xffffffff, 0);
		}

		const Bitmap* pCharBmp = NULL;
		if (!pSlot->tidSkin[4])
		{
			IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(pSlot->tidChar);
			if (pChar)
				pCharBmp = g_pFresh->GetBitmap(pChar->c.Icon);
		}
		else
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(pSlot->tidSkin[4]);
			if (pSkin)
				pCharBmp = g_pFresh->GetBitmap(pSkin->Tex);
		}

		if (pCharBmp)
		{
			rect.x = pItem->pos.x + 8.0f;
			rect.y = pItem->pos.y + 36.0f;
			rect.w = (float)pCharBmp->Width();
			rect.h = (float)pCharBmp->Height();

			pGDI->DrawTexture(pCharBmp, rect, 0xffffffff, 0);
		}

		if (pSlot->bSleep)
		{
			pGDI->DrawTexture(m_pDiveBmp,
				WRect(pItem->pos.x + 6.0f, pItem->pos.y + 100.0f,
					(float)m_pDiveBmp->Width(), (float)m_pDiveBmp->Height()),
				0xffffffff, 0);
		}

		const Bitmap* pGenderBmp;
		if (pSlot->angelicWings)
			pGenderBmp = m_pGenderBmp[8 + pSlot->gender % 2];
		else if (pSlot->manner)
			pGenderBmp = m_pGenderBmp[6 + pSlot->gender % 2];
		else
			pGenderBmp = m_pGenderBmp[pSlot->gender];

		if (pGenderBmp)
		{
			pGDI->DrawTexture(pGenderBmp,
				WRect(pItem->pos.x + 2.0f, pItem->pos.y + 98.0f,
					(float)pGenderBmp->Width(), (float)pGenderBmp->Height()),
				0xffffffff, 0);
		}

		const Bitmap* pBmp = NULL;
		if (pSlot->dwTitle)
		{
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pSlot->dwTitle);
			if (pSkin)
				pBmp = g_pFresh->GetBitmap(pSkin->c.Icon);
		}
		else if (!((sSlotInfo*)pItem->pData)->IsInvite)
		{
			pBmp = g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
		}
		// 983: anfitrião do Grand Prix (bit 0x8000 da identidade do lugar)
		if (pSlot->dwIdentity & 0x8000)
			pBmp = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");

		if (pBmp)
		{
			unsigned h = pBmp->Height(), w = pBmp->Width();
			float x = pItem->pos.x + 90.0f;
			x -= float2int(pBmp->Width() * 0.5f);
			pGDI->DrawTexture(pBmp,
				WRect(x, pItem->pos.y + 68.0f, (float)w, (float)h), 0xffffffff,
				0);
		}

		if (pSlot->GuildId && pSlot->IsInvite)
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblem983(
					pSlot->GuildId, pSlot->dwEmblemVer983);
			if (pEmblem)
			{
				pGDI->DrawTexture(pEmblem,
					WRect(pItem->pos.x + 60.0f, pItem->pos.y + 44.0f,
						(float)pEmblem->Width(), (float)pEmblem->Height()),
					0xffffffff, 0);
			}
		}

		if (CProjectG::Instance()->HidePrivacy())
			return;

		if (pItem->underCursor)
		{
			pGDI->SetTextColor(0xffffffff, 0xff808080);
			pGDI->SetTextStyle(2);
		}
		else
		{
			if (pSlot->dwGuid == MyGuid(true))
				pGDI->SetTextColor(0xffff0000, 0xffffffff);
			else
				pGDI->SetTextColor(0xff000000, 0xffffffff);
			pGDI->SetTextStyle(0);
		}

		g_pFresh->GetManager()->PrintText(WPoint(pItem->pos.x + 20.0f, pItem->pos.y + 107.0f), 0, pSlot->sNick, -1.0f,
			0xffffffff);
	}
	else
	{
		if (!DOC983_AT(bool, 0x11410))
		{
			std::map<unsigned long, sRoomSlot>::iterator it =
				Doc()->m_roomSlotMap.find(pSlot->dwGuid);
			if (it != Doc()->m_roomSlotMap.end())
			{
				if ((*it).second.pExhibition)
					((CExhibition983*)(*it).second.pExhibition)->Display(0.0f, 0.0f);
			}
		}

		WRect rect(pItem->pos.x, pItem->pos.y, (float)m_pTeamBaseBmp->Width(),
			(float)m_pTeamBaseBmp->Height());

		if (DOC983_U8(0x1094c) == 1)
			pGDI->DrawTexture(m_pTeamBmp[pSlot->bTeam], rect, 0xffffffff, 0);
		else
			pGDI->DrawTexture(m_pTeamBaseBmp, rect, 0xffffffff, 0);

		if (pSlot->tidSkin[3])
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(pSlot->tidSkin[3]);
			if (pSkin)
			{
				const Bitmap* pBmp = g_pFresh->GetBitmap(pSkin->Tex);
				pGDI->DrawTexture(pBmp, rect, 0xffffffff, 0);
			}
		}

		if (Doc()->m_myInfo.info.dwGuid == ROOMDOC_MASTER(m_pDoc) && !pSlot->bMaster)
		{
			pGDI->DrawTexture(m_pKickBmp,
				WRect(rect.x + rect.w - 8.0f - m_pKickBmp->Width(),
					rect.y + 8.0f, (float)m_pKickBmp->Width(),
					(float)m_pKickBmp->Height()),
				0xffffffff, 0);
		}

		if (pSlot->dwGuid == ROOMDOC_SEL_OID(m_pDoc))
			pGDI->DrawTexture(m_pTeamSelectBmp, rect, 0xffffffff, 0);

		int px = (int)pItem->pos.x;
		int x =
			(m_pTeamBmp[pSlot->bTeam]->Width() - m_pReadySignBmp->Width()) / 2 + px;

		if (pSlot->bMaster)
		{
			rect = WRect((float)x, pItem->pos.y + 6.0f,
				(float)m_pMasterSignBmp->Width(),
				(float)m_pMasterSignBmp->Height());
			pGDI->DrawTexture(m_pMasterSignBmp, rect, 0xffffffff, 0);
		}
		else if (pSlot->bReady)
		{
			rect = WRect((float)x, pItem->pos.y + 6.0f,
				(float)m_pReadySignBmp->Width(),
				(float)m_pReadySignBmp->Height());
			pGDI->DrawTexture(m_pReadySignBmp, rect, 0xffffffff, 0);
		}

		const Bitmap* pCharBmp = NULL;
		if (!pSlot->tidSkin[4])
		{
			IFF_STRUCT::sChar* pChar = ItemManager()->FindChar(pSlot->tidChar);
			if (pChar)
				pCharBmp = g_pFresh->GetBitmap(pChar->c.Icon);
		}
		else
		{
			IFF_STRUCT::sSkin* pSkin =
				ItemManager()->FindSkin(pSlot->tidSkin[4]);
			if (pSkin)
				pCharBmp = g_pFresh->GetBitmap(pSkin->Tex);
		}

		if (pCharBmp)
		{
			rect.x = pItem->pos.x + 8.0f;
			rect.y = pItem->pos.y + 36.0f;
			rect.w = (float)pCharBmp->Width();
			rect.h = (float)pCharBmp->Height();

			pGDI->DrawTexture(pCharBmp, rect, 0xffffffff, 0);
		}

		if (pSlot->bSleep)
		{
			pGDI->DrawTexture(m_pDiveBmp,
				WRect(pItem->pos.x + 6.0f, pItem->pos.y + 100.0f,
					(float)m_pDiveBmp->Width(), (float)m_pDiveBmp->Height()),
				0xffffffff, 0);
		}

		const Bitmap* pGenderBmp;
		if (pSlot->angelicWings)
			pGenderBmp = m_pGenderBmp[8 + pSlot->gender % 2];
		else if (pSlot->manner)
			pGenderBmp = m_pGenderBmp[6 + pSlot->gender % 2];
		else
			pGenderBmp = m_pGenderBmp[pSlot->gender];

		if (pGenderBmp)
		{
			pGDI->DrawTexture(pGenderBmp,
				WRect(pItem->pos.x + 2.0f, pItem->pos.y + 98.0f,
					(float)pGenderBmp->Width(), (float)pGenderBmp->Height()),
				0xffffffff, 0);
		}

		const Bitmap* pBmp;
		if (pSlot->dwTitle)
		{
			IFF_STRUCT::sSkin* pSkin = ItemManager()->FindSkin(pSlot->dwTitle);
			if (pSkin)
				pBmp = g_pFresh->GetBitmap(pSkin->c.Icon);
			else
				pBmp = g_pFresh->GetBitmap(
					MakeStr("level_%03d", pSlot->level + 1));
		}
		else
		{
			pBmp = g_pFresh->GetBitmap(MakeStr("level_%03d", pSlot->level + 1));
		}
		if (pSlot->dwIdentity & 0x8000)
			pBmp = g_pFresh->GetManager()->GetBitmap("LEVELS", "gp_host");

		if (pBmp)
		{
			pGDI->DrawTexture(pBmp,
				WRect(pItem->pos.x + 60.0f, pItem->pos.y + 68.0f,
					(float)pBmp->Width(), (float)pBmp->Height()),
				0xffffffff, 0);
		}

		if (pSlot->GuildId)
		{
			const Bitmap* pEmblem =
				NetResourceManager::Instance()->GetEmblem983(
					pSlot->GuildId, pSlot->dwEmblemVer983);
			if (pEmblem)
			{
				pGDI->DrawTexture(pEmblem,
					WRect(pItem->pos.x + 60.0f, pItem->pos.y + 44.0f,
						(float)pEmblem->Width(), (float)pEmblem->Height()),
					0xffffffff, 0);
			}
		}

		if (CProjectG::Instance()->HidePrivacy())
			return;

		if (pItem->underCursor)
		{
			pGDI->SetTextColor(0xffffffff, 0xff808080);
			pGDI->SetTextStyle(2);
		}
		else
		{
			if (pSlot->dwGuid == MyGuid(true))
				pGDI->SetTextColor(0xffff0000, 0xffffffff);
			else
				pGDI->SetTextColor(0xff000000, 0xffffffff);
			pGDI->SetTextStyle(0);
		}

		g_pFresh->GetManager()->PrintText(WPoint(pItem->pos.x + 20.0f, pItem->pos.y + 107.0f), 0, pSlot->sNick, -1.0f,
			0xffffffff);
	}
}

// 00503660 (645: CLobbyMain::EnableSettingButton). No 983 só os cinco quadros de equipamento e o estado da seleção.
void FrGameRoomDlg::EnableSettingButton(bool enable)
{
	if (m_pCaddie)
		m_pCaddie->Enable(enable);

	if (m_pClub)
		m_pClub->Enable(enable);

	if (m_pAztec)
		m_pAztec->Enable(enable);

	if (m_pMascot)
		m_pMascot->Enable(enable);

	if (m_pEquipItem)
		m_pEquipItem->Enable(enable);

	InitControlsSelectedUserInfo();
}

// 0050d120 (645: caso 0x11 do CLobbyMain::HandleMsg): imagens do campo, barra do tesouro e controles de dono
void FrGameRoomDlg::SetCourse983(unsigned char course)
{
	IFF_STRUCT::sCourse* pCourse = ItemManager()->FindCourse(
		0x28000000 | (course >= 0x7f ? 0x7f : course));
	if (pCourse)
	{
		if (m_pMap)
			m_pMap->SetButtonImg(MakeStr("big_%s", pCourse->c.Icon),
				FrButton::NORMAL);

		if (m_pBackGround)
		{
			m_pBackGround->Open(
				MakeStr("background%s.tga", strstr(pCourse->c.Icon, "_")));
		}

		if (m_pMapBg)
		{
			m_pMapBg->Open("map_background.tga");
		}
	}

	if (m_pOverGauge)
	{
		((FrGaugeBarImage*)m_pOverGauge)->ResetGauge();
		if (course < 0x7f)
		{
			unsigned long gauge = CTHunter::Instance()->GetTHunterGauge(
				(eMapType)DOC983_U8(0x10950));
			m_pOverGauge->SetPos(gauge < 701 ? 0 : gauge - 700);
		}
		else
		{
			m_pOverGauge->SetPos(0);
		}
	}

	std::list<sSlotInfo>::iterator it = std::find(Doc()->m_slotList.begin(),
		Doc()->m_slotList.end(), MyGuid(false));

	if (it == Doc()->m_slotList.end())
		return;

	SetRoomControls(it->bMaster || Doc()->m_myInfo.info.IsIdentity(4));
}

// ---- Sessão B45 (4ª parte): boneco do lugar (ShowPet/HidePet), barras de atributo, controles da sala ----

// 983: as duas parcelas de cada atributo vêm do CStatusHelper (statushelper.h é da frente A: só empresta o `this`)
class CStatusHelperJp
{
public:
	int GetBonus983(unsigned long type); // 005e48a0: max(0, 005e4690(type) - 005e4740(type, 1, 0)); 0 se type > 4
};

// 00508f10 (645: CLobbyMain::HidePet)
void FrGameRoomDlg::HidePet(unsigned long uid)
{
	std::map<unsigned long, sRoomSlot>::iterator it =
		Doc()->m_roomSlotMap.find(uid);

	if (it != Doc()->m_roomSlotMap.end())
	{
		if (it->second.pExhibition)
		{
			delete (CExhibition983*)it->second.pExhibition;
			it->second.pExhibition = NULL;
		}
	}
}

// 0050b570 (645: CLobbyMain::SetLevelBar; no 983 a conta toda foi para o CStatusHelper)
void FrGameRoomDlg::SetLevelBar()
{
	CCardManager::Instance()->CalcCardPeriodAndStatus();

	CStatusHelper helper;
	for (int i = 0; i < 5; i++)
	{
		int nLevel = helper.GetStatus983(i);
		int nCapacity = nLevel + ((CStatusHelperJp*)&helper)->GetBonus983(i);

		if (m_pStatBar[i])
		{
			m_pStatBar[i]->SetDestPos(nLevel, false);
			m_pStatBar[i]->SetExpand(nCapacity - nLevel);
		}
		if (m_pStatEdit[i])
			m_pStatEdit[i]->SetCaption11(MakeStr("%d/%d", nLevel, nCapacity));
	}
}

void __fastcall Fn0b304b0_983(unsigned long uid, sCharacterInfo* pInfo); // 00b304b0 (corpo NÃO lido)

// `pExhibition` do lugar com o tipo do 983 (shareddoc.h é da frente A e ainda aponta o CExhibition do 645)
#define SLOT_EXH983(p) ((CExhibition983*)(p)->pExhibition)

// 0050b850 (645: CLobbyMain::ShowPet). No 983 o lugar sai do estado da janela (+0x178), não do nome do layout
void FrGameRoomDlg::ShowPet(unsigned long uid)
{
	std::map<unsigned long, sRoomSlot>::iterator it =
		Doc()->m_roomSlotMap.find(uid);
	if (it == Doc()->m_roomSlotMap.end())
		return;

	HidePet(uid);

	if (uid ==
		((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1)
				? Doc()->m_myInfo.info.dwGalleryGuid
				: Doc()->m_myInfo.info.dwGuid))
		SetLevelBar();

	sRoomSlot* pSlot = &it->second;
	if (m_state == 3)
	{
		std::list<sSlotInfo>::iterator itSlot =
			std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), uid);

		if (itSlot == Doc()->m_slotList.end())
			return;

		pSlot->pArea = m_pPet[(*itSlot).connectionRank - 1];
	}
	else if (m_state == 2 || m_state == 4)
	{
		pSlot->pArea = m_pPet[0];
	}

	if (m_state == 7)
		pSlot->pArea = m_pPet[0];

	if (pSlot == NULL || pSlot->pArea == NULL)
		return;

	if (!g_pFresh->GetManager()->IsValidWindow(pSlot->pArea))
		return;

	pSlot->pArea->SetMouseEvent(uid == Doc()->m_myInfo.info.dwGuid);

	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (Doc()->m_myInfo.info.dwGalleryGuid == -1)
			return;

		if (!pSlot->partTidList.SetTids(&pSlot->charInfo, 0xff) ||
			!pSlot->partTidList.IsComboValid())
		{
			pSlot->partTidList.m_charTid = pSlot->charInfo.tid;
			pSlot->partTidList.SetDefaultTids();
		}
	}
	else
	{
		if (!pSlot->partTidList.SetTids(&pSlot->charInfo, 0xff) ||
			!pSlot->partTidList.IsComboValid())
		{
			pSlot->partTidList.m_charTid = pSlot->charInfo.tid;
			pSlot->partTidList.SetDefaultTids();
		}

		if (uid == Doc()->m_myInfo.info.dwGuid)
			Doc()->BuildMyPartTidList(pSlot->charInfo);
	}

	WRect rect = pSlot->pArea->GetRect();

	// 983: +0x2d4 do lugar (uid da conta; `dwUserUID` no shareddoc.h) vai para 00b304b0 e para o boneco
	Fn0b304b0_983(pSlot->dwUserUID, &pSlot->charInfo);

	pSlot->pExhibition = (CExhibition*)new CExhibitionByHardwareTnL983(
		(float)(rect.x + rect.w * 0.5), (float)(rect.y + rect.h * 0.5), 0.35f,
		pSlot->dwUserUID, uid != Doc()->m_myInfo.info.dwGuid);
	SLOT_EXH983(pSlot)->SetDistanceRange(10.0f, 14.0f);
	SLOT_EXH983(pSlot)->SetDistance(12.0f, true, false);
	SLOT_EXH983(pSlot)->SetArea(rect.w, rect.h);
	SLOT_EXH983(pSlot)->SetModel(pSlot->partTidList, pSlot->charInfo.tidAuxParts, NULL,
		3.0f, uid);

	if (pSlot->bGachaWing)
		SLOT_EXH983(pSlot)->AttachAngelWingFx();

	if (uid == Doc()->m_myInfo.info.dwGuid)
	{
		if (!strcmpi(g_pFresh->GetManager()->GetLayoutID(), "CREATE"))
			SLOT_EXH983(pSlot)->m_flags = 0x1f;
		else
			SLOT_EXH983(pSlot)->m_flags = 0x1a;
	}
	else
	{
		SLOT_EXH983(pSlot)->m_flags = 0x10;
	}
}

// 0050c0b0 (645: CLobbyMain::SetRoomControls). 983: `realGameType == 14` virou modo 0x12 ou tipo 0x0b, o canal 0x80
// saiu das contas, o cadeado do mapa sumiu e o "start" ganhou o bilhete e o Grand Prix
void FrGameRoomDlg::SetRoomControls(bool bMaster)
{
	unsigned char nUser = (unsigned char)Doc()->m_slotList.size();
	bool bLock = false;

	if (Doc()->m_curChannel.Type & 0x800)
	{
		IFF_STRUCT::sCourse* pCourse =
			ItemManager()->FindCourse(((DOC983_U8(0x10950) > 0x7f &&
										   DOC983_U8(0x10950) != 0xfd)
											  ? DOC983_U8(0x10950) - 0x80
											  : DOC983_U8(0x10950)) |
				0x28000000);

		if (pCourse && pCourse->Difficulty >= 3)
			bLock = true;
	}

	bool bGM = (bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1);
	if (bGM)
		bLock = false;

	if (m_pRoomOption)
	{
		if (bMaster)
		{
			bool bEnable = false;
			if (nUser == 1 || !m_bCanStart || bGM == true)
			{
				if (DOC983_U8(0x109b8) != 0x12 && DOC983_U8(0x1094c) != 0x0b)
					bEnable = true;
			}
			m_pRoomOption->Enable(bEnable);
		}
		else
		{
			m_pRoomOption->Enable(false);
		}
	}

	if (m_p1b8)
	{
		if (DOC983_U8(0x1094c) == 0x0a || DOC983_U8(0x109b8) == 0x12)
		{
			m_p1b8->Enable(false);
		}
		else
		{
			if (bMaster)
				m_p1b8->Enable(nUser == 1 || !m_bCanStart ||
					(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1));
			else
				m_p1b8->Enable(false);
		}
	}

	if (m_pMapPrev)
	{
		if (bMaster)
		{
			bool bVisible = false;
			if (nUser == 1 || !m_bCanStart ||
				(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
			{
				if (DOC983_U8(0x109b8) != 0x12 && DOC983_U8(0x1094c) != 0x0b)
					bVisible = true;
			}
			m_pMapPrev->SetVisible(bVisible);

			if (DOC983_U8(0x1094c) == 0x0a)
				m_pMapPrev->Enable(false);
			else
				m_pMapPrev->Enable(DOC983_U8(0x10950) != FirstMap());
		}
		else
		{
			m_pMapPrev->SetVisible(false);
		}
	}

	if (m_pMapNext)
	{
		if (bMaster)
		{
			bool bVisible = false;
			if (nUser == 1 || !m_bCanStart ||
				(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
			{
				if (DOC983_U8(0x109b8) != 0x12 && DOC983_U8(0x1094c) != 0x0b)
					bVisible = true;
			}
			m_pMapNext->SetVisible(bVisible);

			if (DOC983_U8(0x1094c) == 0x0a)
				m_pMapNext->Enable(false);
			else
				m_pMapNext->Enable(
					DOC983_U8(0x10950) != LastMap(DOC983_U8(0x1094f) == 4));
		}
		else
		{
			m_pMapNext->SetVisible(false);
		}
	}

	if (m_pMapHoleCombo)
	{
		if (bLock || (Doc()->m_curChannel.Type & 0x10) ||
			DOC983_U8(0x1094c) == 0x0a || DOC983_U8(0x109b8) == 0x12 ||
			DOC983_U8(0x1094c) == 0x0b)
		{
			m_pMapHoleCombo->Enable(false);
		}
		else
		{
			if (bMaster)
				m_pMapHoleCombo->Enable(nUser == 1 || !m_bCanStart ||
					(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1));
			else
				m_pMapHoleCombo->Enable(false);
		}
	}

	if (m_pStart)
	{
		if (bMaster)
		{
			m_pStart->SetVisible(true);
			m_pStart->Enable(true);

			if (m_pReady)
			{
				m_pReady->SetVisible(false);

				if (m_pReady)
					m_pReady->Enable(false);
			}

			if ((DOC983_U8(0x1094c) != 0x0b && DOC983_U8(0x1095e) != 0 &&
					DOC983_U8(0x10937) > 99) ||
				(Doc()->m_myInfo.info.dwIdentity & 4) ||
				(Doc()->m_myInfo.info.dwIdentity & 0x8000))
			{
				m_pStart->Enable(Doc()->m_slotList.size() >= 1);
			}
			else if (CanUseStartTicket983())
			{
				m_pStart->Enable(m_bCanStart);
			}
			else if (DOCJP()->GetState983() == 7)
			{
				// Grand Prix: o dono começa sozinho
				if (m_bCanStart && Doc()->m_slotList.size() >= 1)
					m_pStart->Enable(true);
				else
					m_pStart->Enable(false);
			}
			else
			{
				if (m_bCanStart && Doc()->m_slotList.size() > 1)
					m_pStart->Enable(true);
				else
					m_pStart->Enable(false);
			}
		}
		else if (!m_bRoomStateReq)
		{
			m_pReady->SetButtonImg(m_bReady ? "btn_ready_cancel_n"
											: "btn_ready_n",
				FrButton::NORMAL);
			m_pReady->SetButtonImg(m_bReady ? "btn_ready_cancel_o"
											: "btn_ready_o",
				FrButton::OVER);

			m_pReady->SetVisible(true);
			m_pReady->Enable(true);
			m_pStart->SetVisible(false);
			m_pStart->Enable(true);

			if (IS_GP_MODE983() && m_b18d && DOCJP()->GetGPChannel983())
				m_pReady->Enable(false);
		}
	}

	if (m_p230)
	{
		if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		{
			m_p230->Enable(
				(Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe &&
				Doc()->m_myInfo.info.dwGalleryGuid != -1);
		}
		else
		{
			if (bMaster)
				m_p230->Enable(true);
			else
				m_p230->Enable(!m_bReady);
		}

		if (Doc()->m_curChannel.Type & 8)
			m_p230->Enable(false);
	}

	SetRoomCharControl(2);
	SetRoomCaddieControl(2);

	if (DOC983_U8(0x1094c) == 6)
	{
		if (m_p1b8)
			m_p1b8->Enable(false);
		if (m_pMapPrev)
			m_pMapPrev->Enable(false);
		if (m_pMapNext)
			m_pMapNext->Enable(false);
		if (m_pMapHoleCombo)
			m_pMapHoleCombo->Enable(false);

		if (m_state == 8)
			RemakeTeam(true);
	}

	if (IsLocalContent((localContentType_t)0x55 /*983: era 0x54; medido*/))
		CMessengerInfo::Instance()->EnableInvite(bMaster);
}

// 00508f60 (645: CLobbyMain::SetRoomCharControl)
void FrGameRoomDlg::SetRoomCharControl(int dir)
{
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (Doc()->m_myInfo.info.dwGalleryGuid == -1)
		{
			if (m_pChar983)
				m_pChar983->SetVisible(false);
		}
		else
		{
			if (m_pChar983)
			{
				m_pChar983->SetVisible(true);

				IFF_STRUCT::sChar* pChar =
					ItemManager()->FindChar(Doc()->m_userInfo[0].charInfo.tid);
				if (pChar)
					m_pChar983->SetButtonImg(pChar->c.Icon, FrButton::NORMAL);
			}
		}
	}

	if (!m_bReady && !(bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		std::map<unsigned int, sCharacterInfo>& charMap = Doc()->m_charMap;
		std::map<unsigned int, sCharacterInfo>::const_iterator it =
			charMap.find(Doc()->m_myInfo.userEquip.guidChar);

		if (it == charMap.end())
			return;

		switch (dir)
		{
		case 0:
			if (it == charMap.begin())
				break;

			if (--it != charMap.end())
			{
				WSendPacket packet((enumClientPacket)0xc);
				packet.Encode1(4);
				packet.Encode4((*it).second.guid);
				packet.Send(TO_GAME);
			}
			break;

		case 1:
			if (++it != charMap.end())
			{
				WSendPacket packet((enumClientPacket)0xc);
				packet.Encode1(4);
				packet.Encode4((*it).second.guid);
				packet.Send(TO_GAME);
			}
			break;
		}
	}
}

// 0050b660 (645: CLobbyMain::SetRoomCaddieControl). 983: sem as mensagens 0x13 ao ator (nem o trecho do GM)
void FrGameRoomDlg::SetRoomCaddieControl(int dir)
{
	std::map<unsigned int, sCaddieInfo>& caddies = Doc()->m_caddieMap;
	if (caddies.size() == 0)
	{
		Doc()->m_myInfo.userEquip.guidCaddie = 0;
		return;
	}

	unsigned long guid = Doc()->m_myInfo.userEquip.guidCaddie;

	std::map<unsigned int, sCaddieInfo>& caddieMap = Doc()->m_caddieMap;
	std::map<unsigned int, sCaddieInfo>::const_iterator it =
		caddieMap.find(guid);

	switch (dir)
	{
	case 0:
	{
		if (it != caddieMap.begin())
		{
			if (--it != caddieMap.end())
			{
				Doc()->m_myInfo.userEquip.guidCaddie = (*it).second.guid;
				SetLevelBar();
				WSendPacket packet((enumClientPacket)0xc);
				packet.Encode1(1);
				packet.Encode4((*it).second.guid);
				packet.Send(TO_GAME);
				break;
			}
		}
		Doc()->m_myInfo.userEquip.guidCaddie = 0;
		SetLevelBar();
		WSendPacket packet((enumClientPacket)0xc);
		packet.Encode1(1);
		packet.Encode4(0);
		packet.Send(TO_GAME);
		break;
	}
	case 1:
		if (guid == 0)
			it = caddieMap.begin();
		else
			++it;

		if (it == caddieMap.end())
			break;

		Doc()->m_myInfo.userEquip.guidCaddie = (*it).second.guid;

		SetLevelBar();

		{
			WSendPacket packet((enumClientPacket)0xc);
			packet.Encode1(1);
			packet.Encode4((*it).second.guid);
			packet.Send(TO_GAME);
		}
		break;

	case 2:
		if (it == caddieMap.end())
			Doc()->m_myInfo.userEquip.guidCaddie = 0;
		else
			Doc()->m_myInfo.userEquip.guidCaddie = (*it).second.guid;
		break;
	}
}

// 00503340 (nome meu): par do ResetRoom983, chamada pelo destrutor. Apaga a torcida e, se o lobby está no estado 10 ou
// 11, avisa o ator "Lobby" (mensagem 0x59 com 9 ou 1)
void FrGameRoomDlg::ReleaseRoom983()
{
	if (m_pCheer)
	{
		delete m_pCheer;
		m_pCheer = NULL;
	}

	CLobbyState983* pState = CLobbyState983::Instance983();
	if (pState)
	{
		int param = 0;
		if (pState->IsState10_983())
			param = 9;
		if (pState->IsState11_983())
			param = 1;
		if (param)
			AfxPostMsg(NULL, "Lobby", 0x59, param, 0, 0, 0);
	}
}

// ---- Sessão B46 (5ª parte): estado inicial, recorde novo, ESC (relatório do Tiki / sair no meio do jogo), times ----

// 00503290 (nome meu): é o fim do `CLobbyMain::OnGameRoom_Init` do 645; os construtores das derivadas chamam
void FrGameRoomDlg::ResetRoom983()
{
	m_bIdle = false;
	m_bReady = false;
	m_bLockControls = false;
	m_bCanStart = false;
	m_bAutoStart = false;

	if (Doc()->m_myInfo.info.dwIdentity == 2)
	{
		m_bReady = true;
		m_bLockControls = true;
	}

	m_bRoomStateReq = false;
	ROOMDOC_SEL_OID(m_pDoc) = 0xffffffff;
	ROOMDOC_SEL_UID(m_pDoc) = 0xffffffff;
	m_pCurTip = NULL;
	m_pStartTip = NULL;
	m_pReadyTip = NULL;

	g_mouse->ResetInputTime();
	g_ime->ResetInputTime();

	if (DOC983_U8(0x109b8) == 0x12)
	{
		AfxPostMsg(NULL, "Lobby", 0x23,
			(int)K2L_Compatibility("\xbd\xba\xc6\xf3\xbc\xc8 \xbc\xc5\xc7\xc3 \xb8\xf0\xb5\xe5\xbf\xa1\xbc\xad\xb4\xc2, \n\\c0xffff0000\\c\xb0\xd4\xc0\xd3 \xc1\xdf \xc0\xd4\xc0\xe5\\c0xff000000\\c \xb9\xd7 \\c0xffff0000\\c\xc6\xbc\xc5\xb0\xb8\xae\xc6\xf7\xc6\xae\\c0xff000000\\c \xbb\xe7\xbf\xeb\xc0\xcc \xba\xd2\xb0\xa1\xb4\xc9 \xc7\xd5\xb4\xcf\xb4\xd9."),
			0, 0, 0);
	}

	RFTooltipHelper* pTip = RFTooltipHelper::Instance983();
	if (pTip)
		pTip->Reset983();
}

// 004ff260 (645: CLobbyMain::OnNewRecordDlgResult)
bool FrGameRoomDlg::OnNewRecordDlgResult(int result, FrForm* form)
{
	m_pNewRecordDlg = NULL;
	return true;
}

// 005033b0 (645: CLobbyMain::OpenNewRecordForm, só a primeira metade)
void FrGameRoomDlg::OpenNewRecordForm()
{
	if (Doc()->m_newRecordState != 0xff && m_pNewRecordDlg == NULL)
	{
		m_pNewRecordDlg = CreateForm<FrNewRecordDlg>(g_pFresh->GetManager(),
			this, "new_record", NULL);
		m_pNewRecordDlg->Open(
			(FRESH_PFN_RESULT)&FrGameRoomDlg::OnNewRecordDlgResult, 0);
		Doc()->m_newRecordState = 0xff;
	}
}

// 00506f30 (645: CLobbyMain::CheckTikiReport, que gravava `m_bUseReport`): pode usar o papel de relatório do Tiki
// (0x1a000041)? Só no estado 8 (resultado do jogo). Novo no 983: não, com 10 ou mais relatórios 0x1a000042 guardados
bool FrGameRoomDlg::CheckTikiReport(int state)
{
	if (state != 8)
		return false;

	if ((Doc()->m_myInfo.info.dwIdentity & 2) ||
		(Doc()->m_myInfo.info.dwIdentity & 4) ||
		(Doc()->m_myInfo.info.dwIdentity & 8))
	{
		return false;
	}

	sItemNode983* pHead = DOC983_AT(sItemNode983*, 0x375c);
	sItemNode983* p;
	for (p = pHead->pNext; p != pHead; p = p->pNext)
	{
		if (p->tid == 0x1a000041)
			break;
	}
	if (p == pHead)
		return false;

	bool bUseReport = true;
	if (DOC983_U8(0x1094c) == 6)
		bUseReport = false;

	if (DOC983_U8(0x10937) > 30)
		bUseReport = false;

	if (m_pUniteResultDlg)
		bUseReport = false;

	if (Doc()->m_curChannel.Type & 0x800)
		bUseReport = false;

	if (Doc()->m_myInfo.stat.Level <= 5)
		bUseReport = false;

	unsigned int count = 0;
	for (p = pHead->pNext; p != pHead; p = p->pNext)
	{
		if (p->tid == 0x1a000042)
			count++;
	}
	if (count >= 10)
		bUseReport = false;

	return bUseReport;
}

// 005037b0 (nome meu): pede ao ator "Lobby" a tela do relatório (mensagem 0x8b com 0x0c)
void FrGameRoomDlg::OpenTikiReport983()
{
	MsgObject msg(NULL, 0x8b, 0xc, 0, 0, 0, 0);
	AfxGetTask()->GetActor("Lobby") << msg;
}

// 00505410 (645: CLobbyMain::OnRoomExtResQuitDlgResult)
bool FrGameRoomDlg::OnRoomExtResQuitDlgResult(int result, FrForm* form)
{
	if (result == 1)
	{
		FrWnd* pOk = form->FindChildByName("ok");
		if (pOk)
			pOk->Enable(false);

		WSendPacket packet((enumClientPacket)0xf);
		packet.Encode1(0);
		packet.Encode2(0xffff);

		bool bEncoded = false;
		if (IsLocalContent((localContentType_t)0x14))
		{
			unsigned char gameType = DOC983_U8(0x1094c);
			if (gameType == 4 || gameType == 5)
			{
				for (std::vector<sRivalData>::iterator it =
						 Doc()->m_rivalList.begin();
					it != Doc()->m_rivalList.end(); ++it)
				{
					if (it->oid == MyGuid(false))
					{
						packet.Encode8(it->totalPang);
						packet.Encode8(it->totalBonusPang);
						bEncoded = true;
						break;
					}
				}
			}
		}

		if (!bEncoded)
		{
			packet.Encode8(0);
			packet.Encode8(0);
		}

		packet.Send(TO_GAME);

		Doc()->m_bGameOver = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1, 0, 0, 0, 0, 0));
		Close(FrOK, true);
	}

	m_pExtResQuitDlg = NULL;
	DOC983_AT(unsigned short, 0x7a0) = 0xffff;
	DOC983_AT(unsigned short, 0x1094d) = 0xffff;
	return true;
}

// 005055c0 (645: CLobbyMain::OpenGameExtResQuitForm): "sair no meio do jogo?" (o 983 usa o molde "notify_tiki_sub")
void FrGameRoomDlg::OpenGameExtResQuitForm()
{
	if (m_pExtResQuitDlg)
		return;

	m_pExtResQuitDlg =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_tiki_sub", NULL);
	m_pExtResQuitDlg->SetMessage(
		K2L_Compatibility("\xbe\xc6\xc1\xf7 \xb0\xd4\xc0\xd3\xc0\xcc \xc1\xf8\xc7\xe0 \xc1\xdf \xc0\xd4\xb4\xcf\xb4\xd9.\n\xb4\xd9\xb8\xa5 \xc7\xc3\xb7\xb9\xc0\xcc\xbe\xee\xb5\xe9\xc0\xcc \xb8\xf0\xb5\xce \xbf\xcf\xc1\xd6\xc7\xd2 \xb6\xa7\xb1\xee\xc1\xf6\n\xb1\xe2\xb4\xd9\xb7\xc1 \xc1\xd6\xbd\xca\xbd\xc3\xbf\xc0.\n\n\xc1\xf6\xb1\xdd \xb7\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xb0\xd4 \xb5\xc7\xb8\xe9,\n\\c0xffff0000\\c\xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xc0\xb2\\c0xff000000\\c\xc0\xba \xbf\xc3\xb6\xf3\xb0\xa1\xc1\xf6 \xbe\xca\xc1\xf6\xb8\xb8\n\xb0\xd4\xc0\xd3 \xc1\xdf \xc8\xb9\xb5\xe6\xc7\xd1 \\c0xffff0000\\c\xb0\xe6\xc7\xe8\xc4\xa1, \xbe\xc6\xc0\xcc\xc5\xdb, \xb1\xe2\xb7\xcf \xb5\xee\xc0\xcc\n\xb8\xf0\xb5\xce \xb9\xab\xc8\xbf \xc3\xb3\xb8\xae\\c0xff000000\\c \xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb7\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?"),
		false);
	m_pExtResQuitDlg->Open(
		(FRESH_PFN_RESULT)&FrGameRoomDlg::OnRoomExtResQuitDlgResult, 0x801);
}

// 00508ea0 (nome meu): ESC na sala. Com o relatório do Tiki disponível (fora do modo 0x12) abre o relatório; senão
// pergunta se sai
void FrGameRoomDlg::OnEscape983()
{
	m_bUseReport = CheckTikiReport(m_state);
	if (m_pUniteResultDlg == NULL)
	{
		if (m_bUseReport && DOC983_U8(0x109b8) != 0x12)
		{
			OpenTikiReport983();
			g_input->ExclusiveGetDownUseDone("ESCAPE");
			return;
		}

		OpenGameExtResQuitForm();
		g_input->ExclusiveGetDownUseDone("ESCAPE");
	}
}

// 00509120 (645: CLobbyMain::RemakeTeam)
void FrGameRoomDlg::RemakeTeam(bool bRemake)
{
	if (bRemake)
	{
		m_redTeam.clear();
		m_blueTeam.clear();

		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); it++)
		{
			if (DOC983_U8(0x1094c) == 6)
			{
				if (it->GuildId == Doc()->m_roomInfo.GuildInfo.nGuildID[0])
					m_redTeam.push_back(&(*it));
				else if (it->GuildId == Doc()->m_roomInfo.GuildInfo.nGuildID[1])
					m_blueTeam.push_back(&(*it));
			}
			else
			{
				if (it->bTeam == 0)
					m_redTeam.push_back(&(*it));
				else
					m_blueTeam.push_back(&(*it));
			}
		}

		if (m_redTeam.size() == 0)
		{
			Doc()->m_roomInfo.GuildInfo.Reset(0);
		}

		if (m_blueTeam.size() == 0)
		{
			Doc()->m_roomInfo.GuildInfo.Reset(1);
		}
	}

	m_pUser->ClearItem();

	int count = Max(m_redTeam.size(), m_blueTeam.size());
	if (count < 12)
		count = 12;

	std::list<sSlotInfo*>::iterator itRed = m_redTeam.begin();
	int redCount = 0;
	int blueCount = 0;
	std::list<sSlotInfo*>::iterator itBlue = m_blueTeam.begin();
	for (int i = 0; i < count; i++)
	{
		if (itRed == m_redTeam.end())
			m_pUser->AddItem(NULL);
		else
		{
			m_pUser->AddItem(*itRed);
			itRed++;
			redCount++;
		}

		if (itBlue == m_blueTeam.end())
			m_pUser->AddItem(NULL);
		else
		{
			m_pUser->AddItem(*itBlue);
			itBlue++;
			blueCount++;
		}
	}

	unsigned long guid = Doc()->m_myInfo.info.dwGuid;
	std::list<sSlotInfo>::iterator it =
		std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), guid);

	if (it != Doc()->m_slotList.end())
	{
		if (m_pGuildInvite)
		{
			if (bRemake == true && m_state == 8)
			{
				if ((it->bMaster) &&
					((blueCount < 1 && redCount > 2) ||
						(blueCount > 2 && redCount < 1)))
				{
					m_pGuildInvite->Enable(true);
				}
				else
				{
					m_pGuildInvite->Enable(false);
				}
			}
		}
	}
}

// tipos de sala "em massa" do 983: 4, 5, 6, 9, 10, 11 e 18 (tabela de 00508e6c); o `IsMassGame(gameType)` do
// shareddoc.inl da frente A ainda tem a lista do 645 (4, 5, 6, 9, 10, 14) — pedido B → A (B46)
inline int IsMassGame983(unsigned char gameType)
{
	switch (gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
	case 18:
		return TRUE;
	}
	return FALSE;
}

// 00502ed0 (cópia fora de linha nesta unidade): o `GetGameTypeName` do 983 tem 0x16 tipos e a tabela em CSharedDoc+0xc8
inline const char* GetGameTypeName983(unsigned char gameType)
{
	if (gameType >= 0x16)
		return NULL;

	return *(const char**)((char*)&Doc()->m_gameTypeInfo + gameType * 8);
}

// CItemManager 00a19320 (sem nome no mapa): texto do artefato no `m_ArtifactManaInfoMap983` (+0x348), 0 se não tem
class CItemManagerGr983
{
public:
	const char* FindArtifactDesc983(unsigned long tid);
};

// 00508a50 (nome meu; no 645 é o caso 0x10 do `CLobbyMain::HandleMsg`): linha de título da sala e, novo no 983, o ícone
// e a dica do artefato da sala (+0x109b9)
void FrGameRoomDlg::SetRoomTitle983(const char* time)
{
	if (m_pTitle == NULL)
		return;

	char szTime[32] = "";
	if (time)
	{
		strcpy(szTime, time);
	}
	else if (IsMassGame983(DOC983_U8(0x1094c)))
	{
		if (IS_GP_MODE983())
		{
			m_pTitle->SetLine(1,
				MakeStr(K2L_Compatibility("[%d]   %s"), DOC983_AT(unsigned short, 0x1094d), DOCPTR983(0x108f4)),
				0, false, 0, 0);
			return;
		}

		if (DOC983_U8(0x1094c) == 10)
		{
			sprintf(szTime, K2L_Compatibility("%d\xc3\xca"), DOC983_AT(unsigned long, 0x10955) / 1000);
		}
		else if (DOC983_U8(0x1094c) == 6)
		{
			sprintf(szTime, "");
		}
		else
		{
			sprintf(szTime, K2L_Compatibility("%d\xba\xd0"), DOC983_AT(unsigned long, 0x10955) / 60000);
		}
	}
	else
	{
		if (DOC983_AT(unsigned long, 0x10951) == 0)
			sprintf(szTime, K2L_Compatibility("\xbd\xc3\xb0\xa3\xc1\xa6\xc7\xd1\xbe\xf8\xc0\xbd"));
		else
			sprintf(szTime, K2L_Compatibility("%d\xc3\xca"), DOC983_AT(unsigned long, 0x10951) / 1000);
	}

	const char* name;
	const char* title = DOCPTR983(0x108f4);
	if (!IsMassGame983(DOC983_U8(0x1094c)))
	{
		name = GetGameTypeName983(DOC983_U8(0x1094c));
	}
	else
	{
		switch (DOC983_U8(0x1094c))
		{
		case 10:
			name = GetGameTypeName983(DOC983_U8(0x1094c));
			break;

		case 11:
			name = GetGameTypeName983(DOC983_U8(0x1094c));
			title = K2L_Compatibility(title);
			break;

		default:
		{
			IFF_STRUCT::sMatch* pMatch = ItemManager()->FindMatch(DOC983_AT(unsigned long, 0x10959));
			if (pMatch == NULL)
				name = GetGameTypeName983(DOC983_U8(0x1094c));
			else if (DOC983_U8(0x109b8) != 0x12)
				name = pMatch->Name;
			else
				name = K2L_Compatibility("\xbc\xc5\xc7\xc3\xc4\xda\xbd\xba");
		}
		break;
		}
	}

	m_pTitle->SetLine(1,
		MakeStr(K2L_Compatibility("[%d]   %s %s    %s %s  %2d\xc8\xa6  (%d/%d)  %s"),
			DOC983_AT(unsigned short, 0x1094d),
			DOC983_AT(bool, 0x10934) ? "" : K2L_Compatibility("[\xba\xf1]"),
			title, name, "",
			DOC983_U8(0x1094b), DOC983_U8(0x10938), DOC983_U8(0x10937), szTime),
		0, false, 0, 0);

	unsigned long* pArtifact = &DOC983_AT(unsigned long, 0x109b9);
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(*pArtifact);
	if (pItem && (pItem->Artifact & 1))
	{
		if (m_pAtft)
			m_pAtft->SetVisible(true);

		if (m_pArtifactIcon)
		{
			const Bitmap* pBmp = g_pFresh->GetBitmap(MakeStr("s_%s", pItem->c.Icon));
			if (pBmp)
				m_pArtifactIcon->SetBgImg(pBmp);

			const char* desc = ((CItemManagerGr983*)ItemManager())->FindArtifactDesc983(*pArtifact);
			if (desc == NULL)
				desc = "";
			m_artifactTip = MakeStr("<font color=\"#ffaa00\">%s</font>\n%s", pItem->c.Name, desc);
		}
	}
	else
	{
		if (m_pAtft)
			m_pAtft->SetVisible(false);

		if (m_pArtifactIcon)
			m_pArtifactIcon->m_nFlags.Disable(FWF_TOOLTIPS);
	}
}

// 0050c000 (nome meu): põe os jogadores que já estão na sala na lista e monta os bonecos; manda o campo ao ator
// principal (mensagem 0x11) e abre o "new_record" se houver recorde novo
void FrGameRoomDlg::InitRoom983()
{
	for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
		it != Doc()->m_slotList.end(); it++)
	{
		if (m_pUser)
			m_pUser->AddItem(&(*it));
		ShowPet(it->dwGuid);
	}

	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x11, DOC983_U8(0x10950), 0, 0, 0, 0));
	OpenNewRecordForm();
}

class FrWnd983
{
public:
	static void SetF1ab38_983(); // 00be7950 (o mesmo empréstimo do singletypedlg.cpp)
};

// 0050d0e0
bool FrGameRoomDlg::OnInit()
{
	InitRoom983();
	FrWnd983::SetF1ab38_983();
	if (CLobbyState983::Instance983())
		AfxPostMsg(NULL, "Lobby", 0x59, 10, 0, 0, 0);
	return FrForm::OnInit();
}

// 00505380 (645: CLobbyMain::OnApproachGiftDlgResult)
bool FrGameRoomDlg::OnApproachGiftDlgResult(int result, FrForm* form)
{
	m_pTreasureGift = NULL;

	WSendPacket packet((enumClientPacket)0xcb);
	packet.Encode4(Doc()->m_myInfo.info.dwGuid);
	packet.Send(TO_GAME);
	return true;
}


// 0050acd0: parte de sala do `CLobbyMain::ProcessLobbyUI` do 645. O que era teste do nome do layout virou teste de
// `m_state`; as duas dicas (assist e artefato) passam pelo RFTooltipHelper
void FrGameRoomDlg::OnProc(const float deltaTime)
{
	FrChatDlg::OnProc(deltaTime);

	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) &&
		g_input->GetDown("REPLAY", false))
	{
		OnStartBtnUp();
	}

	if (!Doc()->m_bGameOver && m_bNewBlink)
	{
		m_newBlinkTime += deltaTime;
		if (m_newBlinkTime > 0.2f)
		{
			m_newBlinkTime -= 0.2f;
			m_newBlinkCount++;

			if (m_pCurTip)
				m_pCurTip->SetVisible(!m_pCurTip->IsVisible());

			if (m_newBlinkCount == 3)
			{
				m_bNewBlink = false;
				m_newBlinkCount = 0;
			}
		}
	}

	if (g_input->GetDown("ESCAPE", true) == 1 && m_state == 8 && !IS_GP_MODE983() && IsMassGame() &&
		Doc()->m_bGameOver)
	{
		OnEscape983();
	}

	if (m_resultDelay <= 0.0f)
	{
		m_resultDelay += deltaTime;

		bool bTimeOut = false;
		if (m_resultDelay > 0.0f)
			bTimeOut = true;

		if (g_input->GetDown("ESCAPE", true) == 1)
		{
			if (m_pUniteResultDlg)
			{
				if (m_pUniteResultDlg->GetDone())
					if (!m_pUniteResultDlg->CheckChildDlgVisible())
						bTimeOut = true;
			}
			else
			{
				bTimeOut = true;
			}
		}

		if (bTimeOut)
		{
			m_resultDelay = 1.0f;
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x51, 0, 0, 0, 0, 0));

			if (m_pUniteResultDlg)
				m_pUniteResultDlg->Close(false);

			if (m_pNewRecordDlg)
				m_pNewRecordDlg->Close(true);

			if (CUserInfo::Instance() && CUserInfo::Instance()->GetDlg())
				CUserInfo::Instance()->Close();

			if (m_state == 8)
			{
				if (DOC983_U8(0x109b8) != 0x12)
				{
					AfxPostMsg(NULL, "Lobby", 0x8b, 7, 0, 0, 0);
					AfxPostMsg(NULL, "Lobby", 0xf, 0, 0, 0, 0);
					AfxPostMsg(NULL, "Lobby", 0x30, 0, 0, 0, 0);
					Close(FrNONE, false);
				}
				else
				{
					S5::UTIL::SendExitRoom983(0); // 00b2f070
					AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x1, 0, 0, 0, 0, 0));
				}
			}
		}
	}
	else if (m_pTreasureGift)
	{
		m_pTreasureGift->Close(true);
		m_pTreasureGift = NULL;
	}

	if (Doc()->GetAppGiftDownLoadState() == 2)
	{
		if (Doc()->m_golfGame.gameType == GAME_TYPE_NEW_APPROACH)
		{
			if (!m_pTreasureGift)
			{
				unsigned int count = CTHunter::Instance()->GetGiftSize();
				if (count <= 8)
					m_resultDelay = -9.0f;
				else if (count <= 16)
					m_resultDelay = -12.0f;
				else
					m_resultDelay = -20.0f;

				m_pTreasureGift = CreateForm<FrTreasureGift>(
					g_pFresh->GetManager(), this, "trgift", NULL);
				m_pTreasureGift->SetApproachMode();
				m_pTreasureGift->Open(
					(FRESH_PFN_RESULT)&FrGameRoomDlg::OnApproachGiftDlgResult, 12);
				m_pTreasureGift->SetTimeLimit(12.0f);
				m_pTreasureGift->EnableDrag(false);
				Doc()->SetAppGiftDownloadState(
					(CSharedDoc::eAppGiftDownloadState)0);
			}
		}
	}

	if (m_state == 3 || m_state == 4 || m_state == 6)
	{
		bool bChanged = false;
		if (m_bIdle)
		{
			if (g_mouse->IsUpdated() || g_ime->IsUpdated())
			{
				bChanged = true;
				m_bIdle = false;
			}

			if (timeGetTime() - g_mouse->GetLastInputTime() > 300000 &&
				timeGetTime() - g_ime->GetLastInputTime() > 300000)
			{
				unsigned long guid = Doc()->m_myInfo.info.dwGuid;
				std::list<sSlotInfo>::iterator it =
					std::find(Doc()->m_slotList.begin(),
						Doc()->m_slotList.end(), guid);
				if (it != Doc()->m_slotList.end() && ((*it).bMaster))
				{
					// 645: mandava o pacote 0x0f daqui; o 983 fecha a janela e avisa o ator "Lobby"
					Close(FrOK, true);
					MsgObject msg(NULL, 0x1, 0, 0, 0, 0, 0);
					AfxGetTask()->GetActor("Lobby") << msg;
				}
			}
		}
		else
		{
			if (timeGetTime() - g_mouse->GetLastInputTime() > 60000 &&
				timeGetTime() - g_ime->GetLastInputTime() > 60000)
			{
				m_bIdle = true;
				bChanged = true;
			}
		}

		if (bChanged)
		{
			unsigned long guid = Doc()->m_myInfo.info.dwGuid;
			std::list<sSlotInfo>::iterator it =
				std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(),
					guid);
			if (it == Doc()->m_slotList.end())
				return;

			if ((*it).bMaster)
			{
				WSendPacket packet((enumClientPacket)0xa);
				packet.Encode2(0xffff);
				packet.Encode1(1);
				packet.Encode1(9);
				packet.Encode1(m_bIdle ? 1 : 0);
				packet.Send(TO_GAME);
			}

			WSendPacket packet((enumClientPacket)0x32);
			packet.Encode1(m_bIdle ? 1 : 0);
			packet.Send(TO_GAME);
		}
	}

	if (m_pAssistTip && m_pAssistTip->IsVisible())
	{
		if (RFTooltipHelper::Instance983())
		{
			WRect rect = m_pAssistTip->GetRect();
			if (rect.IsInRect(g_input->GetMousePoint().x, g_input->GetMousePoint().y))
			{
				RFTooltipHelper::Instance983()->Show983(0, std::string(m_assistTip.c_str()), 1, 0xb);
				m_bAssistTipOn = true;
			}
			else if (m_bAssistTipOn)
			{
				RFTooltipHelper::Instance983()->Hide983();
				m_bAssistTipOn = false;
			}
		}
	}

	if (m_pArtifactIcon && m_pArtifactIcon->IsVisible())
	{
		if (RFTooltipHelper::Instance983())
		{
			WRect rect = m_pArtifactIcon->GetRect();
			if (rect.IsInRect(g_input->GetMousePoint().x, g_input->GetMousePoint().y))
			{
				RFTooltipHelper::Instance983()->Show983(0, std::string(m_artifactTip.c_str()), 1, 0xb);
				m_bArtifactTipOn = true;
			}
			else if (m_bArtifactTipOn)
			{
				RFTooltipHelper::Instance983()->Hide983();
				m_bArtifactTipOn = false;
			}
		}
	}

	RefreshAllUccClothes();

	if (m_pCheer)
		m_pCheer->Process(deltaTime);
}

// ---- Sessão B47 (6ª parte): os casos do `CLobbyMain::HandleMsg` do 645 que no 983 viraram métodos da sala ----
bool __fastcall ConnectionUserCompare(const void* a, const void* b); // 0059f3c0 (unidade vizinha, não escrita)
bool __fastcall FinishUserCompare(const void* a, const void* b); // 0059fbd0 (idem)
// 0059fb40 (645: CLobbyMain::_RefreshUccClothes; no 983 é função livre da unidade vizinha; corpo não escrito)
void __fastcall RefreshUccClothes983(sSlotInfo* pSlot, unsigned long tid, const char* uccIndex);
// CSharedDoc+0x8cb: uid da conta (vai para `sRoomSlot::dwUserUID` do meu lugar e para 00b304b0)
// CSharedDoc+0x832 (`m_myInfo.info`, byte das bandeiras): no 983 as asas estão nos bits 5 e 6 (0050f0b7); o cabeçalho
// da frente A ainda tem os bits 3 e 4 do 645 — pedido B → A (B47)
struct sInfoFlags983
{
	unsigned char low : 5;
	unsigned char angelicWings : 1;
	unsigned char angelicWingsEffect : 1;
	unsigned char high : 1;
};
#define MYFLAGS983() (*(sInfoFlags983*)DOCPTR983(0x832))
#define DOC983_MYUID() (*(unsigned long*)DOCPTR983(0x8cb))

// 00506d80 (645: CLobbyMain::RefreshAllUccClothes)
void FrGameRoomDlg::RefreshAllUccClothes()
{
	if (CUserInfo::Instance()->GetDlg())
		return;

	sUccRefreshItem item;

	for (int i = 0; i < 10; i++)
	{
		if (!UccManager()->GetRefreshItem(item))
			break;

		for (std::list<sSlotInfo*>::iterator it = m_blueTeam.begin();
			it != m_blueTeam.end(); it++)
		{
			RefreshUccClothes983(*it, item.typeId, item.uccIndex);
		}

		for (std::list<sSlotInfo*>::iterator it2 = m_redTeam.begin();
			it2 != m_redTeam.end(); it2++)
		{
			RefreshUccClothes983(*it2, item.typeId, item.uccIndex);
		}

		if (m_pUser)
		{
			for (int j = 0; j < m_pUser->GetCurrentItemSize(false); j++)
			{
				FrListItem* pItem = m_pUser->GetItem(j);
				if (pItem)
				{
					RefreshUccClothes983((sSlotInfo*)pItem->pData, item.typeId,
						item.uccIndex);
				}
			}
		}
	}
}

// 00506e80 (645: CLobbyMain::OnResultDlgResult). Novo no 983: fecha a janela, menos no Grand Prix com 0062bf30 ≠ 0
bool FrGameRoomDlg::OnResultDlgResult(int result, FrForm* form)
{
	m_resultDelay = -0.1f;
	m_pUniteResultDlg = NULL;

	if (m_pUser)
	{
		unsigned char rank = 1;
		for (FrListBox::ITEM_LIST::iterator it = m_pUser->m_itemList.begin();
			it != m_pUser->m_itemList.end(); ++it)
		{
			FrListItem* pItem = *it;
			if (pItem)
			{
				sSlotInfo* pSlot = (sSlotInfo*)pItem->pData;
				if (pSlot)
					pSlot->connectionRank = rank;
			}
			rank++;
		}
	}

	if (!IS_GP_MODE983() || DOCJP()->GetGPChannel983() == 0)
		Close(FrOK, true);

	Doc()->m_bGameOver = false;
	return true;
}

// 0050bbe0 (nome meu): põe o jogador no time (0 = vermelho) se ainda não estiver na lista
void FrGameRoomDlg::AddTeamSlot983(int team, sSlotInfo* pSlot)
{
	if (team == 0)
	{
		if (std::find(m_redTeam.begin(), m_redTeam.end(), pSlot) == m_redTeam.end())
			m_redTeam.push_back(pSlot);
	}
	else
	{
		if (std::find(m_blueTeam.begin(), m_blueTeam.end(), pSlot) == m_blueTeam.end())
			m_blueTeam.push_back(pSlot);
	}
}

// 0050d280 (nome meu; 645: caso 0x60 do CLobbyMain::HandleMsg): careta do chat no boneco de quem falou
void FrGameRoomDlg::OnChatFace983(const std::string& nick, const std::string& chat)
{
	if (DOCJP()->GetState983() != 3)
		return;

	int face = Doc()->m_chatManager.GetFaceIndex(chat.c_str());
	if (face == 5)
		return;

	for (std::list<FrListItem*>::iterator it = m_pUser->m_itemList.begin();
		it != m_pUser->m_itemList.end(); ++it)
	{
		sSlotInfo* pSlot = (sSlotInfo*)(*it)->pData;

		if (pSlot)
		{
			if (nick == pSlot->sNick)
			{
				if (Doc()->m_roomSlotMap[pSlot->dwGuid].pExhibition)
				{
					SLOT_EXH983(&Doc()->m_roomSlotMap[pSlot->dwGuid])
						->SetChatFacial(face, pSlot->tidChar);
				}
				return;
			}
		}
	}
}

// 0050d390 (nome meu; 645: caso 0x49 do CLobbyMain::HandleMsg): um jogador trocou de personagem. O 983 não grava mais
// `guidChar`/`BuildMyPartTidList` aqui, chama 00b304b0 com o uid da conta e não sai do laço ao achar o lugar
void FrGameRoomDlg::OnChangeChar983(unsigned long guid, sCharacterInfo* pCharInfo)
{
	std::list<sSlotInfo>::iterator it;
	for (it = Doc()->m_slotList.begin(); it != Doc()->m_slotList.end(); ++it)
	{
		if (it->dwGuid != guid)
			continue;

		sRoomSlot& slot = Doc()->m_roomSlotMap[guid];

		it->tidChar = pCharInfo->tid;

		slot.charInfo = *pCharInfo;

		if (guid == MyGuid(true))
		{
			if (!slot.partTidList.SetTids(&slot.charInfo, 0xff) ||
				!slot.partTidList.IsComboValid())
			{
				slot.partTidList.m_charTid = slot.charInfo.tid;
				slot.partTidList.SetDefaultTids();
			}

			Fn0b304b0_983(DOC983_MYUID(), &slot.charInfo);

			if (slot.pExhibition)
				SLOT_EXH983(&slot)->SetModel(slot.partTidList,
					slot.charInfo.tidAuxParts, NULL, 3.0f, 0);

			if (slot.pExhibition && slot.bGachaWing)
				SLOT_EXH983(&slot)->AttachAngelWingFx();

			SetLevelBar();
			SetRoomCharControl(2);
		}
		else
		{
			ShowPet(guid);
		}
	}
}

// 0050d4c0 (nome meu; 645: caso 0x07 do CLobbyMain::HandleMsg): entrou um jogador na sala
void FrGameRoomDlg::OnEnterUser983(sSlotInfo* pSlot, sCharacterInfo* pCharInfo)
{
	if (DOC983_U8(0x1094c) == 6)
	{
		if (Doc()->m_bGameOver)
		{
			AddTeamSlot983(pSlot->bTeam, pSlot);
			RemakeTeam(false);
		}
		else
		{
			RemakeTeam(true);
		}
	}
	else
	{
		if (m_pUser)
		{
			m_pUser->AddItem(pSlot);
			m_pUser->SortItem(ConnectionUserCompare);
		}
	}

	if (DOC983_U8(0x1094c) == 4 || DOC983_U8(0x1094c) == 10)
	{
		if (m_pUser == NULL)
			return;

		if (m_pUser->GetCurrentItemSize(true) <= 24)
			return;

		for (std::list<FrListItem*>::iterator it = m_pUser->m_itemList.begin();
			it != m_pUser->m_itemList.end(); ++it)
		{
			if ((*it)->pData == NULL)
			{
				m_pUser->_DelItem(it);
				break;
			}
		}

		m_pUser->SortItem(ConnectionUserCompare);
	}
	else if (IsMassGame983(DOC983_U8(0x1094c)))
	{
		return;
	}
	else
	{
		sRoomSlot slot;
		slot.pArea = m_pPet[pSlot->connectionRank - 1];
		slot.pExhibition = NULL;
		slot.charInfo = *pCharInfo;

		slot.bAngelWing = pSlot->angelicWings;
		slot.bGachaWing = pSlot->angelicWingsEffect;
		slot.dwUserUID = pSlot->dwUserUID;

		Doc()->m_roomSlotMap[pSlot->dwGuid] = slot;

		if (IsLocalContent(S4_INVITE_FRIEND))
		{
			if (!pSlot->IsInvite)
				ShowPet(pSlot->dwGuid);
		}
		else
		{
			ShowPet(pSlot->dwGuid);
		}

		if (m_pUser == NULL)
			return;

		if (m_pUser->GetCurrentItemSize(true) <= 4)
			return;

		for (std::list<FrListItem*>::iterator it = m_pUser->m_itemList.begin();
			it != m_pUser->m_itemList.end(); ++it)
		{
			if ((*it)->pData == NULL)
			{
				m_pUser->_DelItem(it);
				break;
			}
		}

		m_pUser->SortItem(ConnectionUserCompare);
	}
}

// 0050d700 (nome meu): só o lugar do boneco de um jogador (o miolo do caso 0x07 do 645), sem a lista
void FrGameRoomDlg::AddRoomSlot983(sSlotInfo* pSlot, sCharacterInfo* pCharInfo)
{
	sRoomSlot slot;
	slot.pArea = m_pPet[pSlot->connectionRank - 1];
	slot.pExhibition = NULL;
	slot.charInfo = *pCharInfo;
	slot.dwUserUID = pSlot->dwUserUID;

	slot.bAngelWing = pSlot->angelicWings;
	slot.bGachaWing = pSlot->angelicWingsEffect;

	Doc()->m_roomSlotMap[pSlot->dwGuid] = slot;

	ShowPet(pSlot->dwGuid);
}

// 0050eb20 (645: CLobbyMain::MakeRoomUserList + o começo do caso 0x0e do HandleMsg). Novo no 983: a dica e o piscar do
// "new" vêm para cá; sem a lista só os modos 0x0c, 0x0e e 0x13 montam o meu boneco; 26 lugares no Grand Prix; jogador com
// `sSlotInfo+0x159` ligado fica fora da lista do resultado; os tipos com o meu boneco são 4, 5, 6, 9, 10, 11 e 18
void FrGameRoomDlg::MakeRoomUserList()
{
	if (DOC983_U8(0x10938) > 1)
	{
		m_bNewBlink = true;
		m_newBlinkTime = -10.0f;
		if (!IS_GP_MODE983())
			m_pCurTip = m_pReadyTip;
	}
	else
	{
		m_pCurTip = m_pStartTip;
	}

	if (m_pUser == NULL)
	{
		switch (DOC983_U8(0x109b8))
		{
		case 0x0c:
		case 0x0e:
		case 0x13:
			if (!m_b18d)
			{
				sRoomSlot slot;
				slot.pArea = m_pPet[0];
				slot.pExhibition = NULL;
				slot.partTidList = Doc()->GetMyPartTidList();
				slot.charInfo =
					Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
				slot.bAngelWing = MYFLAGS983().angelicWings;
				slot.bGachaWing = MYFLAGS983().angelicWingsEffect;
				slot.dwUserUID = DOC983_MYUID();
				Doc()->m_roomSlotMap[MyGuid(false)] = slot;

				ShowPet(MyGuid(false));
			}
			break;
		}
		return;
	}

	switch (DOC983_U8(0x1094c))
	{
	case 4:
	case 9:
	case 10:
	case 11:
	{
		m_pUser->ClearItem();

		int nMax = 24;
		if (IS_GP_MODE983())
			nMax = 26;

		if (Doc()->m_bGameOver)
		{
			int nNotFinish = 0;
			if (Doc()->m_rivalList.size() > 0)
			{
				for (std::list<sSlotInfo>::iterator it =
						 Doc()->m_slotList.begin();
					it != Doc()->m_slotList.end(); ++it)
				{
					if (it->bGrandPrixAI983)
						continue;

					sSlotInfo* pSlot = &(*it);

					unsigned char index = Doc()->GetIndex(pSlot->dwGuid);

					if (Doc()->m_rivalList[index].state == 2)
						m_pUser->AddItem(pSlot);
					else if (Doc()->m_rivalList[index].state == 0)
						nNotFinish++;
				}
			}

			for (int i = m_pUser->GetCurrentItemSize(true); i < nMax; i++)
				m_pUser->AddItem(NULL);

			m_pUser->SortItem(FinishUserCompare);

			if (nNotFinish == 0)
				AfxPostMsg(NULL, "Lobby", 0x2e, 0, 0, 0, 0);
		}
		else
		{
			for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
				it != Doc()->m_slotList.end(); ++it)
				m_pUser->AddItem(&(*it));

			for (int i = m_pUser->GetCurrentItemSize(true); i < nMax; i++)
				m_pUser->AddItem(NULL);

			if (Doc()->m_slotList.size() > 1)
				m_pUser->SortItem(ConnectionUserCompare);
		}
	}
	break;

	case 6:
		if (Doc()->m_bGameOver)
		{
			m_redTeam.clear();
			m_blueTeam.clear();

			for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
				it != Doc()->m_slotList.end(); ++it)
			{
				unsigned char index = Doc()->GetIndex((*it).dwGuid);

				if (Doc()->m_rivalList[index].state == 2)
				{
					sSlotInfo* pSlot = &(*it);
					if (pSlot->GuildId ==
						Doc()->m_roomInfo.GuildInfo.nGuildID[0])
						m_redTeam.push_back(&(*it));
					else if (pSlot->GuildId ==
						Doc()->m_roomInfo.GuildInfo.nGuildID[1])
						m_blueTeam.push_back(&(*it));
				}
			}

			m_redTeam.sort(FinishUserCompare);
			m_blueTeam.sort(FinishUserCompare);

			RemakeTeam(false);
		}
		else
		{
			RemakeTeam(true);
		}
		break;

	case 5:
		if (Doc()->m_bGameOver)
		{
			m_redTeam.clear();
			m_blueTeam.clear();

			int nNotFinish = 0;
			for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
				it != Doc()->m_slotList.end(); ++it)
			{
				unsigned char index = Doc()->GetIndex((*it).dwGuid);

				if (Doc()->m_rivalList[index].state == 2)
				{
					sSlotInfo* pSlot = &(*it);
					if (pSlot->bTeam == 0)
						m_redTeam.push_back(pSlot);
					else
						m_blueTeam.push_back(pSlot);
				}
				else if (Doc()->m_rivalList[index].state == 0)
				{
					nNotFinish++;
				}
			}

			m_redTeam.sort(FinishUserCompare);
			m_blueTeam.sort(FinishUserCompare);

			RemakeTeam(false);

			if (nNotFinish == 0)
				AfxPostMsg(NULL, "Lobby", 0x2e, 0, 0, 0, 0);
		}
		else
		{
			RemakeTeam(true);
		}
		break;

	default:
		m_pUser->ClearItem();

		for (std::list<sSlotInfo>::iterator it = Doc()->m_slotList.begin();
			it != Doc()->m_slotList.end(); ++it)
			m_pUser->AddItem(&(*it));

		for (unsigned char n = m_pUser->GetCurrentItemSize(true); n < 4; n++)
			m_pUser->AddItem(NULL);

		if (Doc()->m_slotList.size() > 1)
			m_pUser->SortItem(ConnectionUserCompare);
		break;
	}

	switch (DOC983_U8(0x1094c))
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
	case 18:
		if (!m_b18d)
		{
			sRoomSlot slot;
			slot.pArea = m_pPet[0];
			slot.pExhibition = NULL;
			slot.partTidList = Doc()->GetMyPartTidList();
			slot.charInfo =
				Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
			slot.bAngelWing = MYFLAGS983().angelicWings;
			slot.bGachaWing = MYFLAGS983().angelicWingsEffect;
			slot.dwUserUID = DOC983_MYUID();
			Doc()->m_roomSlotMap[MyGuid(false)] = slot;

			ShowPet(MyGuid(false));
		}
		break;
	}
}

// ---- Sessão B47, 2º lote ----
bool __fastcall RankUserCompare(const void* a, const void* b); // 0059f440 (unidade vizinha, não escrita)
bool __fastcall ApproachRankUserCompare(const void* a, const void* b); // 0059f3e0 (idem)

// modos 0x0c, 0x0e e 0x13 do byte +0x109b8 (os que jogam sem lista de jogadores; o 983 testa em cadeia de `sub`)
inline bool IsSoloMode983(int mode)
{
	switch (mode)
	{
	case 0x0c:
	case 0x0e:
	case 0x13:
		return true;
	}
	return false;
}

// 00503720 (nome meu; 645: caso 0x2b do CLobbyMain::HandleMsg). No 983 o "ready" também trava, só no Grand Prix
void FrGameRoomDlg::SetLockControls983(int lock)
{
	m_bLockControls = lock == 1;

	if (IS_GP_MODE983())
	{
		if (m_pReady)
			m_pReady->Enable(!m_bLockControls);
	}

	if (m_pStart)
		m_pStart->Enable(!m_bLockControls);

	m_bNewBlink = !m_bLockControls;

	if (m_pCurTip)
		m_pCurTip->SetVisible(!m_bLockControls);
}

// 00503810 (nome meu): centra a caixa da ordem dos buracos no espaço entre o botão do mapa e o fundo do quadro
void FrGameRoomDlg::AlignMapHoleCombo983()
{
	if (m_pMapHoleCombo == NULL)
		return;

	if (DOCJP()->GetGPChannel983() != 0)
		return;

	WRect rect = m_pMapHoleCombo->GetRect();

	float d = 0.0f;
	if (m_pMap)
	{
		d = (float)(m_pMapBg->GetRect().h + m_pMapBg->GetRect().y) -
			(float)(m_pMap->GetRect().h + m_pMap->GetRect().y);
		d = d * 0.5 - m_pMapHoleCombo->GetRect().h * 0.5;
	}

	rect.y += d;
	m_pMapHoleCombo->SetRect(rect);
}

// 005036f0 (nome meu): retorno do resultado nos modos 0x0c, 0x0e e 0x13 — como o OnResultDlgResult, sem a lista e sem fechar
bool FrGameRoomDlg::OnResultDlgResult2_983(int result, FrForm* form)
{
	m_pUniteResultDlg = NULL;
	m_resultDelay = -0.1f;
	Doc()->m_bGameOver = false;
	return true;
}

// 00506fe0 (nome meu): por que o relatório do Tiki não pode ser usado, em bits (os mesmos testes do CheckTikiReport), e
// quantos 0x1a000041 há
void FrGameRoomDlg::GetTikiReportState983(unsigned int* pFlags, int* pCount)
{
	if (m_state != 8)
		*pFlags |= 1;

	bool bFound = false;
	if ((Doc()->m_myInfo.info.dwIdentity & 2) ||
		(Doc()->m_myInfo.info.dwIdentity & 4) ||
		(Doc()->m_myInfo.info.dwIdentity & 8))
	{
		*pFlags |= 4;
	}
	else
	{
		sItemNode983* pHead = DOC983_AT(sItemNode983*, 0x375c);
		for (sItemNode983* p = pHead->pNext; p != pHead; p = p->pNext)
		{
			if (p->tid == 0x1a000041)
			{
				*pCount = p->count;
				bFound = true;
				break;
			}
		}
	}
	if (!bFound)
		*pFlags |= 2;

	if (DOC983_U8(0x1094c) == 6)
		*pFlags |= 8;

	if (DOC983_U8(0x10937) > 30)
		*pFlags |= 0x10;

	if (m_pUniteResultDlg)
		*pFlags |= 0x20;

	if (Doc()->m_curChannel.Type & 0x800)
		*pFlags |= 0x80;

	if (DOC983_U8(0x91d) <= 5)
		*pFlags |= 0x40;

	sItemNode983* pHead2 = DOC983_AT(sItemNode983*, 0x375c);
	unsigned int n = 0;
	for (sItemNode983* p2 = pHead2->pNext; p2 != pHead2; p2 = p2->pNext)
	{
		if (p2->tid == 0x1a000042)
			n++;
	}
	if (n >= 10)
		*pFlags |= 0x100;
}

// 005070c0 (nome meu; 645: caso 0x2d do CLobbyMain::HandleMsg): abre o resultado no fim do jogo. Novo no 983: tipo 11
// (sem o SortRank), molde "holeinoneresult" quando CSharedDoc+0x10594 é 0x0b, e o retorno sem fechar nos modos 0x0c/0x0e/0x13
void FrGameRoomDlg::OpenResult983()
{
	if (!Doc()->m_bGameOver)
		return;

	switch (DOC983_U8(0x1094c))
	{
	case 4:
		SortRank();
	case 11:
	{
		if (m_pExtFrame)
			m_pExtFrame->SetVisible(false);
		if (m_pRoomUserRankTab)
			m_pRoomUserRankTab->SetVisible(false);
		if (m_pRoomUserRank)
			m_pRoomUserRank->SetVisible(false);
		if (m_pRoomGuildRank)
			m_pRoomGuildRank->SetVisible(false);
		if (m_pScore)
			m_pScore->SetVisible(false);

		if (m_pUniteResultDlg == NULL)
		{
			m_pUniteResultDlg = CreateForm<FrUniteResultDlg>(
				g_pFresh->GetManager(), this,
				DOC983_U8(0x10594) == 0x0b ? "holeinoneresult" : "uniteresult", NULL);
			m_pUniteResultDlg->EnableDrag(false);

			if (IsSoloMode983(DOC983_U8(0x109b8)))
			{
				m_pUniteResultDlg->Open(
					(FRESH_PFN_RESULT)&FrGameRoomDlg::OnResultDlgResult2_983, 0);
			}
			else
			{
				m_pUniteResultDlg->Open(
					(FRESH_PFN_RESULT)&FrGameRoomDlg::OnResultDlgResult, 0);
			}
		}

		m_resultDelay = -20.0f;
	}
	break;

	case 10:
	{
		if (m_pUser)
		{
			unsigned char rank = 1;
			for (std::list<FrListItem*>::iterator it = m_pUser->m_itemList.begin();
				it != m_pUser->m_itemList.end(); ++it, rank++)
			{
				sSlotInfo* pSlot = (sSlotInfo*)(*it)->pData;
				if (pSlot)
					pSlot->connectionRank = rank;
			}
		}

		Doc()->m_bGameOver = false;
	}
		return;

	default:
	{
		SortRank();

		if (m_pUniteResultDlg == NULL && DOC983_U8(0x10594) != 10)
		{
			m_pUniteResultDlg = CreateForm<FrUniteResultDlg>(
				g_pFresh->GetManager(), this, "uniteresult", NULL);
			m_pUniteResultDlg->EnableDrag(false);
			m_pUniteResultDlg->Open(
				(FRESH_PFN_RESULT)&FrGameRoomDlg::OnResultDlgResult,
				WPoint(68.0f, 48.0f), 0);
		}

		m_resultDelay = -20.0f;
	}
	break;
	}
}

// 00507330 (nome meu; 645: começo do caso 0x46 do CLobbyMain::HandleMsg): imagem do botão de trocar de time
void FrGameRoomDlg::SetTeamChangeBtn983(unsigned long guid)
{
	std::list<sSlotInfo>::iterator it =
		std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), guid);
	if (it == Doc()->m_slotList.end())
		return;

	if (m_pChangeTeam)
		m_pChangeTeam->SetButtonImg(it->bTeam == 0 ? "btn_teamchange_blue_n"
												   : "btn_teamchange_red_n",
			FrButton::NORMAL);
}

// 00507380 (nome meu; 645: caso 0x2f do CLobbyMain::HandleMsg): abas de time/guilda. No 983 "time" é só o tipo 1
void FrGameRoomDlg::SetTeamControls983()
{
	bool bTeam = DOC983_U8(0x1094c) == 1;
	bool bGuild = DOC983_U8(0x1094c) == 6;

	std::list<sSlotInfo>::iterator it;

	if (!GalleryMode())
	{
		unsigned long guid = Doc()->m_myInfo.info.dwGuid;
		it = std::find(Doc()->m_slotList.begin(), Doc()->m_slotList.end(), guid);
		if (it == Doc()->m_slotList.end())
			return;
	}
	else
	{
		bTeam = false;
	}

	if (m_pChangeTeam)
		m_pChangeTeam->Enable(bTeam);

	if (m_pGuildTab[0])
		m_pGuildTab[0]->SetVisible(bGuild);
	if (m_pGuildTab[1])
		m_pGuildTab[1]->SetVisible(bGuild);

	if (m_pTeamTab[0])
	{
		if (bTeam)
			m_pTeamTab[0]->SetBgImg("bar_redteam");
		else
			m_pTeamTab[0]->SetBgImg("bar_base");
	}

	if (m_pTeamTab[1])
		m_pTeamTab[1]->SetVisible(bTeam);

	if (!bTeam)
		return;

	if (m_pChangeTeam)
		m_pChangeTeam->SetButtonImg(it->bTeam == 0 ? "btn_teamchange_blue_n"
												   : "btn_teamchange_red_n",
			FrButton::NORMAL);
}

// 00509440 (nome meu; 645: caso 0x08 do CLobbyMain::HandleMsg): saiu um jogador da sala
void FrGameRoomDlg::OnLeaveUser983(void* pItem, unsigned long guid, int team)
{
	if (DOC983_U8(0x1094c) == 6)
	{
		if (Doc()->m_bGameOver)
		{
			std::list<sSlotInfo*>::iterator it;
			if (team == 0)
			{
				for (it = m_redTeam.begin(); it != m_redTeam.end(); ++it)
				{
					if ((*it)->dwGuid == guid)
					{
						m_redTeam.erase(it);
						break;
					}
				}
			}
			else
			{
				for (it = m_blueTeam.begin(); it != m_blueTeam.end(); ++it)
				{
					if ((*it)->dwGuid == guid)
					{
						m_blueTeam.erase(it);
						break;
					}
				}
			}

			RemakeTeam(false);
		}
		else
		{
			RemakeTeam(true);
		}
	}
	else
	{
		if (m_pUser && pItem)
			m_pUser->DelItem(pItem);
	}

	if (DOC983_U8(0x1094c) == 4 || DOC983_U8(0x1094c) == 10)
	{
		if (m_pUser)
		{
			if (m_pUser->GetCurrentItemSize(true) < 24)
				m_pUser->AddItem(NULL);
		}
	}
	else if (!IsMassGame983(DOC983_U8(0x1094c)))
	{
		HidePet(guid);

		std::map<unsigned long, sRoomSlot>::iterator it =
			Doc()->m_roomSlotMap.find(guid);
		if (it != Doc()->m_roomSlotMap.end())
		{
			if (it->second.pArea)
				it->second.pArea->SetMouseEvent(false);
			Doc()->m_roomSlotMap.erase(it);
		}

		std::list<sSlotInfo>::iterator itSlot;
		for (it = Doc()->m_roomSlotMap.begin();
			it != Doc()->m_roomSlotMap.end(); ++it)
		{
			itSlot = std::find(Doc()->m_slotList.begin(),
				Doc()->m_slotList.end(), it->first);

			if (itSlot != Doc()->m_slotList.end())
			{
				it->second.pArea = m_pPet[itSlot->connectionRank - 1];

				if (it->second.pExhibition)
				{
					SLOT_EXH983(&it->second)->SetCenter(
						it->second.pArea->GetRect().x +
							it->second.pArea->GetRect().w * 0.5,
						it->second.pArea->GetRect().y +
							it->second.pArea->GetRect().h * 0.5);

					if (it->second.bGachaWing)
						SLOT_EXH983(&it->second)->AttachAngelWingFx();
				}
			}
		}

		if (m_pUser)
		{
			if (m_pUser->GetCurrentItemSize(true) < 4)
				m_pUser->AddItem(NULL);
		}
	}

	if (guid == ROOMDOC_SEL_OID(m_pDoc))
	{
		ROOMDOC_SEL_OID(m_pDoc) = 0xffffffff;
		ROOMDOC_SEL_UID(m_pDoc) = 0xffffffff;
	}
}

// 00509700 (nome meu; 645: caso 0x09 do CLobbyMain::HandleMsg): as asas de um jogador mudaram
void FrGameRoomDlg::OnChangeWings983(sSlotInfo* pSlot)
{
	std::map<unsigned long, sRoomSlot>::iterator itRoom =
		Doc()->m_roomSlotMap.find(pSlot->dwGuid);

	if (itRoom != Doc()->m_roomSlotMap.end())
	{
		itRoom->second.bGachaWing = pSlot->angelicWingsEffect;
		itRoom->second.bAngelWing = pSlot->angelicWings;
	}

	if (m_pUser == NULL)
		return;

	for (std::list<FrListItem*>::iterator it = m_pUser->m_itemList.begin();
		it != m_pUser->m_itemList.end(); ++it)
	{
		FrListItem* pItem = *it;
		sSlotInfo* pInfo = (sSlotInfo*)pItem->pData;
		if (pInfo)
		{
			if (pSlot->dwGuid == pInfo->dwGuid)
			{
				pInfo->angelicWings = pSlot->angelicWings;
				pInfo->angelicWingsEffect = pSlot->angelicWingsEffect;
				return;
			}
		}
	}
}

// 00505640 (645: CLobbyMain::SortRank). No 983 acaba na ordenação: o texto do placar dos times saiu
void FrGameRoomDlg::SortRank()
{
	if (DOC983_U8(0x1094c) == 10)
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); it++)
		{
			if (it->state == 3)
			{
				it->rank = Doc()->m_rivalList.size() - it->quitOrder;
			}
			else
			{
				it->rank = 1;
				for (std::vector<sRivalData>::iterator it2 =
						 Doc()->m_rivalList.begin();
					it2 != Doc()->m_rivalList.end(); it2++)
				{
					if (it == it2 || it2->state == 3)
						continue;

					if (it->approachResultDistance >
							it2->approachResultDistance ||
						(it->approachResultDistance ==
								it2->approachResultDistance &&
							it->approachTime < it2->approachTime))
						it->rank++;
				}
			}
		}

		if (m_pRoomUserRank)
			m_pRoomUserRank->SortItem(ApproachRankUserCompare);
	}
	else
	{
		for (std::vector<sRivalData>::iterator it = Doc()->m_rivalList.begin();
			it != Doc()->m_rivalList.end(); it++)
		{
			if (it->state == 3)
				continue;

			it->rank = 1;
			for (std::vector<sRivalData>::iterator it2 =
					 Doc()->m_rivalList.begin();
				it2 != Doc()->m_rivalList.end(); it2++)
			{
				if (it == it2 || it2->state == 3)
					continue;

				if (it->totalScore > it2->totalScore ||
					(it->totalScore == it2->totalScore &&
						it->totalPang < it2->totalPang))
					it->rank++;
			}
		}

		if (m_pRoomUserRank)
			m_pRoomUserRank->SortItem(RankUserCompare);
	}
}

// 005074b0 (645: CLobbyMain::SetStartBtn). No 983 os tipos em massa são 4, 5, 6, 9, 10, 11 e 18
void FrGameRoomDlg::SetStartBtn(bool bMaster)
{
	if (bMaster)
	{
		std::list<sSlotInfo>& slotList = Doc()->m_slotList;

		switch (DOC983_U8(0x1094c))
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 11:
		case 18:
			if (NET()->GetConnectionType() == 0 &&
				slotList.size() <
					((Doc()->m_curGameServer.property & 2)
							? 5
							: DOC983_U8(0xc8 + 4 * 8 + 5) /* m_gameTypeInfo[4].minPlayer: a tabela do 983 começa em +0xc8 */) &&
				!(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
			{
				m_bCanStart = true;
				for (std::list<sSlotInfo>::iterator it = slotList.begin();
					it != slotList.end(); ++it)
				{
					if (!(*it).bReady && !(*it).bMaster)
					{
						m_bCanStart = false;
						break;
					}
				}
			}
			else
			{
				m_bCanStart = true;
				for (std::list<sSlotInfo>::iterator it = slotList.begin();
					it != slotList.end(); ++it)
				{
					if (!(*it).bReady && !(*it).bMaster)
					{
						m_bCanStart = false;
						break;
					}
				}
			}
			break;

		default:
			if (slotList.size() == 1 &&
				!(bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
			{
				m_bCanStart = false;
			}
			else
			{
				m_bCanStart = true;

				if (DOC983_U8(0x1094c) == 1)
				{
					if (Doc()->m_roomInfo.nUserLimit > slotList.size())
						m_bCanStart = false;

					if (m_bCanStart)
					{
						int nRed = 0;
						int nBlue = 0;
						for (std::list<sSlotInfo>::iterator it =
								 slotList.begin();
							it != slotList.end(); ++it)
						{
							if ((*it).bTeam == 0)
								nRed++;
							else if ((*it).bTeam == 1)
								nBlue++;
						}

						if (nRed != nBlue)
							m_bCanStart = false;
					}
				}

				if (m_bCanStart)
				{
					for (std::list<sSlotInfo>::iterator it = slotList.begin();
						it != slotList.end(); ++it)
					{
						if (!(*it).bReady && !(*it).bMaster)
						{
							m_bCanStart = false;
							break;
						}
					}
				}
			}
			break;
		}

		if (m_bCanStart && slotList.size() > 1)
		{
			if (m_pCurTip)
			{
				m_bNewBlink = true;
				m_newBlinkTime = 0.0f;
			}

			if (m_pTreasureCourse)
				m_pTreasureCourse->Close(true);
		}
		else
		{
			m_bNewBlink = false;
			if (m_pCurTip)
				m_pCurTip->SetVisible(false);
		}
	}
}

// ---- Sessão B47, 3º lote ----
// 0050c760 (nome meu; 645: caso 0x0f do CLobbyMain::HandleMsg): acha o dono da sala e acerta os controles. Novo no 983:
// o dono confere o artefato da sala (uma vez, `m_b3dd`); sem o item ou reprovado, pede ao servidor para tirar (pacote
// 0x0a: 0xffff, 1, 0x0d, 0)
void FrGameRoomDlg::UpdateMaster983()
{
	std::list<sSlotInfo>& slotList = Doc()->m_slotList;
	bool bMaster = false;

	ROOMDOC_MASTER(m_pDoc) = 0xffffffff;

	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
	{
		bMaster = true;
		ROOMDOC_MASTER(m_pDoc) = MyGuid(false);
	}
	else
	{
		std::list<sSlotInfo>::iterator it;
		std::list<sSlotInfo>::iterator itEnd = Doc()->m_slotList2.end();
		for (it = Doc()->m_slotList2.begin(); it != itEnd; ++it)
		{
			sSlotInfo slot = *it;
			if (slot.bMaster)
			{
				ROOMDOC_MASTER(m_pDoc) = slot.dwGuid;

				if (slot.dwGuid == MyGuid(false))
					bMaster = true;
				break;
			}
		}

		for (it = slotList.begin(); it != slotList.end(); ++it)
		{
			if (it->bMaster)
			{
				ROOMDOC_MASTER(m_pDoc) = it->dwGuid;

				if (it->dwGuid == MyGuid(false))
					bMaster = true;
				break;
			}
		}
	}

	if (bMaster)
	{
		m_bReady = false;
		m_bRoomStateReq = false;
		m_pCurTip = m_pStartTip;

		if (DOC983_AT(unsigned long, 0x109b9) != 0 && !m_b3dd)
		{
			CItemCursor983 items(DOCPTR983(0x375c));
			bool bValid = false;
			if (items.Find(&DOC983_AT(unsigned long, 0x109b9)))
			{
				unsigned char flag = items.GetFlag();
				sArtifactCheckArg983 arg;
				arg.pFlag = &flag;
				arg.pRoomType = &DOC983_U8(0x1094c);
				arg.pMode = &DOC983_U8(0x109b8);
				arg.p1094b = &DOC983_U8(0x1094b);
				sArtifactCheck983 check(&DOC983_AT(unsigned long, 0x109b9), &arg);
				bValid = check.bFail == 0;
			}

			if (!bValid && (m_state == 3 || m_state == 4))
			{
				WSendPacket packet((enumClientPacket)10);
				packet.Encode2(0xffff);
				packet.Encode1(1);
				packet.Encode1(0x0d);
				packet.Encode4(0);
				packet.Send((eSendTo)0);
				m_b3dd = true;
			}
		}
	}

	if (m_pMap)
	{
		if (DOC983_U8(0x109b8) == 0x12)
		{
			m_pMap->Enable(false);
		}
		else
		{
			m_pMap->Enable(bMaster);
		}
	}

	SetStartBtn(bMaster);
	SetRoomControls(bMaster);

	if (m_p1b8)
		m_p1b8->Enable(bMaster && DOC983_U8(0x1094c) != 6);
}

// 0050ca30 (nome meu; 645: casos 0x71/0x29 do CLobbyMain::HandleMsg): resposta do "pronto". No 983 o time é só o tipo 1
void FrGameRoomDlg::OnReadyState983(unsigned long guid, int state)
{
	if (state == 2)
	{
		m_bRoomStateReq = false;
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 0x23,
			(int)K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb7\xb9\xb5\xf0\xc7\xd2 \xbc\xf6 "
							  "\xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9. \xc0\xe1\xbd\xc3\xc8\xc4 "
							  "\xb4\xd9\xbd\xc3 \xb7\xb9\xb5\xf0\xc7\xd8\xc1\xd6\xbc\xbc"
							  "\xbf\xe4."),
			0, 0, 0, 0));
		return;
	}

	if (guid == MyGuid(false) || guid == 0xffffffff)
	{
		m_bRoomStateReq = false;
		m_bReady = state == 0;
		m_bNewBlink = state == 1;
		m_newBlinkTime = -10.0f;

		if (DOC983_U8(0x1094c) == 1)
		{
			if (m_pChangeTeam)
				m_pChangeTeam->Enable(!m_bReady);
		}

		if (m_pUser)
			m_pUser->UseRightButton(!m_bReady);

		if (m_pUserListDlg)
			m_pUserListDlg->EnableRightButton(!m_bReady);
	}

	std::list<sSlotInfo>& slotList = Doc()->m_slotList;
	std::list<sSlotInfo>::iterator it;
	if (guid == 0xffffffff)
	{
		for (it = slotList.begin(); it != slotList.end(); ++it)
			it->bReady = state == 0;
	}
	else
	{
		it = std::find(slotList.begin(), slotList.end(), guid);
		if (it != slotList.end())
			it->bReady = state == 0;
	}

	bool bMaster = false;
	for (it = slotList.begin(); it != slotList.end(); ++it)
	{
		if (it->bMaster)
		{
			if (it->dwGuid == MyGuid(false))
				bMaster = true;
			break;
		}
	}

	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 2) & 1))
		bMaster = true;

	SetStartBtn(bMaster);
	SetRoomControls(bMaster);
}

// ---- Sessão B47, 4º lote ----
// 0050bc80 (nome meu): põe os adversários de IA do Grand Prix (documento 0xa3) na lista de jogadores da sala — lugar
// de 0x15c zerado, ordem depois dos jogadores, nome da tabela do Grand Prix, `sSlotInfo+0x159` ligado e o número da IA
// em +0x15a — e completa a lista com lugares vazios até o limite da sala
void FrGameRoomDlg::AddGrandPrixAI983()
{
	CAIDocJp* pAIDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xa3, pAIDoc);

	int nUser = Doc()->m_slotList.size();
	for (unsigned int i = 0; i < (unsigned int)pAIDoc->GetCount983(); i++)
	{
		sAIDocInfo983 info;
		pAIDoc->GetInfo983(i, 0, &info);

		int id = pAIDoc->GetNpcId983(i);

		sSlotInfo slot;
		memset(&slot, 0, sizeof(slot));
		slot.connectionRank = (unsigned char)nUser + 1 + i;
		slot.bGrandPrixAI983 = 1;
		strcpy(slot.sNick, (char*)Doc()->m_itemManager.FindGrandPrixAI983(id) + 8);
		slot.grandPrixAIId983 = id;

		Doc()->m_slotList.push_back(slot);
	}

	int nMax = DOC983_U8(0x10937);
	for (int n = m_pUser->GetCurrentItemSize(true); n < nMax; n++)
		m_pUser->AddItem(NULL);

	if (pAIDoc->GetCount983() != 0)
	{
		m_bAI983 = true;
		ROOMDOC_B18(m_pDoc) = true;
	}

	m_lastSlotAddTime983 = g_CurrentTime;
	m_waitStartTime983 = g_CurrentTime;
}

// ---- Sessão B47, 5º lote: três pequenas ----
// 004ff210 (nome meu): liga o botão "assist" e o deixa apertado (2) ou solto (0)
void FrGameRoomDlg::SetAssistBtn983(bool bOn)
{
	if (m_pAssist)
	{
		m_pAssist->Enable(true);
		m_pAssist->SetStatus(bOn ? (FrButton::eButMode)2 : (FrButton::eButMode)0);
	}
}

// 004ff250 (nome meu): retorno de um diálogo guardado em +0x160 (quem abre não está nesta unidade)
bool FrGameRoomDlg::OnDlg160Result983(int result, FrForm* form)
{
	m_pScoreDlg = 0;
	return true;
}

// 004ff200 (nome meu): estado da sala (+0x178), chamado pelo CLobbyMain::OpenGameRoomDlg983
void FrGameRoomDlg::SetState983(int state)
{
	m_state = state;
}

// 004ff300 (nome meu): grava o par de +0x154/+0x158
void FrGameRoomDlg::Set154_983(int a, int b)
{
	m_154 = a;
	m_158 = b;
}

// A217 (LAYOUTS): layout do FrGameRoomDlg travado no build. sizeof 0x430 = `new` de 0050ac20 (construtor 005083c0); os
// deslocamentos são os anotados no cabeçalho (134 campos; tools/f35/campos983.py: 94 usados nas funções pareadas = 983, 0 divergentes;
// OnChangeTeamBtnUp 005076c0 lê +0x17d/+0x17e como o nosso). Amigo declarado no cabeçalho para enxergar os campos protegidos.
struct FrGameRoomDlgAcc983
{
	static void Layout983()
	{
		C_ASSERT_VC10(sizeof(FrGameRoomDlg) == 0x430);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCheer) == 0x140);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pNewRecordDlg) == 0x144);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTreasureCourse) == 0x148);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pDoc) == 0x14c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTimeBoard983) == 0x150);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_154) == 0x154);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_158) == 0x158);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pHoleFont) == 0x15c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pScoreDlg) == 0x160);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pUniteResultDlg) == 0x164);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTreasureGift) == 0x168);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pUserListDlg) == 0x16c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pExtResQuitDlg) == 0x170);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pNotifyDlg) == 0x174);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_state) == 0x178);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bIdle) == 0x17c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bReady) == 0x17d);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bLockControls) == 0x17e);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bCanStart) == 0x17f);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bAutoStart) == 0x180);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bRoomStateReq) == 0x181);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_newBlinkTime) == 0x184);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_newBlinkCount) == 0x188);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bNewBlink) == 0x18c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_b18d) == 0x18d);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pUser) == 0x190);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pPet) == 0x194);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pBg) == 0x1a4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pBackGround) == 0x1a8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMapBg) == 0x1ac);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMap) == 0x1b0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_1b4) == 0x1b4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_p1b8) == 0x1b8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMapPrev) == 0x1bc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMapNext) == 0x1c0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMapHoleCombo) == 0x1c4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pOverGauge) == 0x1c8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCaddie) == 0x1cc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAztec) == 0x1d0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pClub) == 0x1d4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMascot) == 0x1d8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pEquipItem) == 0x1dc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCharSel) == 0x1e0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCaddieSel) == 0x1e4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAztecSel) == 0x1e8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pClubSel) == 0x1ec);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMascotSel) == 0x1f0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pStart) == 0x1f4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pReady) == 0x1f8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pStatBar) == 0x200);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pStatEdit) == 0x214);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGuildInvite) == 0x228);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pChangeTeam) == 0x22c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_p230) == 0x230);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pScore) == 0x234);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamNum) == 0x238);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGuildNum) == 0x23c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_240) == 0x240);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_244) == 0x244);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pExtFrame) == 0x248);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pChar983) == 0x24c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_250) == 0x250);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_254) == 0x254);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_258) == 0x258);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamTab) == 0x25c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGuildTab) == 0x264);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGuildScoreGauge) == 0x26c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pStartTip) == 0x274);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pReadyTip) == 0x278);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCurTip) == 0x27c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCountdown) == 0x280);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRoomUserRankTab) == 0x284);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTitle) == 0x288);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRoomOption) == 0x28c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pUserInfo) == 0x290);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRoomUserRank) == 0x294);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRoomGuildRank) == 0x298);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pModeTitle) == 0x29c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pCourseCtrl) == 0x2a0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pChipinCtrl) == 0x2a4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pBgCourse) == 0x2a8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pFixHole) == 0x2ac);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pFixHolecupPos) == 0x2b0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pSlope) == 0x2b4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pSize) == 0x2b8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTicket) == 0x2bc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAtft) == 0x2c0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pArtifactIcon) == 0x2c4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAssist) == 0x2c8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAssistTip) == 0x2cc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pAssistTitle) == 0x2d0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pShortGame) == 0x2d4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGPInfo983) == 0x2d8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGPDetail983) == 0x2dc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGroupStroke) == 0x2e0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGroup30s) == 0x2e4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGroupResult) == 0x2e8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGroupGp) == 0x2ec);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_redTeam) == 0x2f0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_blueTeam) == 0x2fc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamBmp) == 0x308);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamBaseBmp) == 0x310);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamBase02Bmp) == 0x314);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pInviteBmp) == 0x318);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pMasterSignBmp) == 0x31c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pReadySignBmp) == 0x320);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pSituBmp) == 0x324);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamSelectBmp) == 0x340);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pDiveBmp) == 0x344);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pTeamRankBmp) == 0x350);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRankBgBmp) == 0x358);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRankBmp) == 0x35c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pPlayerBaseBmp) == 0x36c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pKickBmp) == 0x37c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_380) == 0x380);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_38c) == 0x38c);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_390) == 0x390);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pGenderBmp) == 0x394);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pRoomEvent02Bmp) == 0x3bc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_3c0) == 0x3c0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bAI983) == 0x3c4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_lastSlotAddTime983) == 0x3c8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_waitStartTime983) == 0x3cc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_slotAddSec983) == 0x3d0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_startWaitSec983) == 0x3d4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_resultDelay) == 0x3d8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bUseReport) == 0x3dc);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_b3dd) == 0x3dd);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_3e0) == 0x3e0);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_pFont) == 0x3e4);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bAssistTipOn) == 0x3e8);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_bArtifactTipOn) == 0x3e9);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_assistTip) == 0x3ec);
		C_ASSERT_VC10(offsetof(FrGameRoomDlg, m_artifactTip) == 0x408);
	}
};

// 0059fb40 (B653; 645: CLobbyMain::_RefreshUccClothes): acha a ficha do jogador no mapa de CSharedDoc+0x3890 (chave = o
// ponteiro recebido, como o 983) e, se o item de roupa [tid>>13 & 0x1f] ainda e o mesmo, reaplica a roupa UCC no boneco.
class CPetFrameJp983Ucc
{
public:
	void SetPart983(const void* part); // 008b0960
};
void __fastcall RefreshUccClothes983(sSlotInfo* pSlot, unsigned long tid, const char* uccIndex)
{
	if (!pSlot)
		return;

	// D825: o 983 le a ficha como sRoomSlot (it->second): +4 pExhibition, +0x10 m_tid[], +0x70 m_uid[] (0059fb6d..0059fbb9).
	// Antes: map<unsigned long, char[0x80]> (no de outro tamanho = _Isnil lido fora do lugar) e campos 4 bytes antes.
	std::map<unsigned long, sRoomSlot>& m = Doc()->m_roomSlotMap;
	std::map<unsigned long, sRoomSlot>::iterator it = m.find((unsigned long)pSlot);
	if (it == m.end())
		return;

	sRoomSlot* pRoomSlot = &it->second;
	if (!pRoomSlot || !pRoomSlot->pExhibition)
		return;

	unsigned char idx = (unsigned char)(tid >> 13) & 0x1f;
	if (pRoomSlot->partTidList.m_tid[idx] != tid)
		return;

	sUccClothes* pClothes = UccManager()->FindClothes(pRoomSlot->partTidList.m_uid[idx]);
	if (pClothes && _stricmp(pClothes->uccIndex, uccIndex) == 0)
		(*(CPetFrameJp983Ucc**)((char*)pRoomSlot->pExhibition + 0xa8))->SetPart983(pClothes); // CExhibition::m_pPetFrame (C_ASSERT exhibition.cpp)
}
