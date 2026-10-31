#pragma once
// B1455/B1457: crus CGolfDoc+0xNNN do 983 (= VC10) ancorados no membro que os contem; no cl novo o CGolfDoc anda
// (std::list m_inputList983 em +0x8c e std::string m_xmlFile em +0xac menores: -0x2c) -> o offset segue o membro.
// `off` constante; no VC10 GDOCOFS983(off) == off.
// B1459: ancoras = todos os membros do CGolfDoc a partir de +0xc (layout VC10 medido, build/b1459/probe.py -> out/B1459/lay_vc10.txt).
#include <stddef.h>
#include "golfdoc.h"
#include "vc10_layout.h"

#define GDOCOFS983_IN(off, m, v) \
	((unsigned)((off) - (v)) < sizeof(((CGolfDoc*)0)->m)) ? (offsetof(CGolfDoc, m) + ((off) - (v))) :
#define GDOCOFS983(off) ( \
	GDOCOFS983_IN(off, m_pSky, 0xc) \
	GDOCOFS983_IN(off, m_pAutoTarget, 0x10) \
	GDOCOFS983_IN(off, m_propertyInfo, 0x14) \
	GDOCOFS983_IN(off, m_mapInfo, 0x38) \
	GDOCOFS983_IN(off, m_waveInfo, 0x48) \
	GDOCOFS983_IN(off, m_riverInfo, 0x58) \
	GDOCOFS983_IN(off, m_modelNameList, 0x68) \
	GDOCOFS983_IN(off, m_tutorialMode, 0x88) \
	GDOCOFS983_IN(off, m_inputList983, 0x8c) \
	GDOCOFS983_IN(off, m_pCameraTargetPuppet, 0x98) \
	GDOCOFS983_IN(off, m_tutorialPin983, 0x9c) \
	GDOCOFS983_IN(off, m_bTutorialPin983, 0xa8) \
	GDOCOFS983_IN(off, m_unused99, 0xa9) \
	GDOCOFS983_IN(off, m_xmlFile, 0xac) \
	GDOCOFS983_IN(off, m_pPlayer, 0xc8) \
	GDOCOFS983_IN(off, m_pTeam, 0xcc) \
	GDOCOFS983_IN(off, m_teamNum, 0xd0) \
	GDOCOFS983_IN(off, m_currentPlayerNum, 0xd4) \
	GDOCOFS983_IN(off, m_playerNum, 0xd5) \
	GDOCOFS983_IN(off, m_unusedc6, 0xd6) \
	GDOCOFS983_IN(off, m_pHoleData, 0xd8) \
	GDOCOFS983_IN(off, m_uiAlpha, 0xdc) \
	GDOCOFS983_IN(off, m_unusedcd, 0xdd) \
	GDOCOFS983_IN(off, m_bar, 0xe0) \
	GDOCOFS983_IN(off, m_cameraData, 0x104) \
	GDOCOFS983_IN(off, m_gameMode, 0x158) \
	GDOCOFS983_IN(off, m_bNetGameEnd, 0x15c) \
	GDOCOFS983_IN(off, m_bPause, 0x15d) \
	GDOCOFS983_IN(off, m_bTeeShotReady, 0x15e) \
	GDOCOFS983_IN(off, m_bReport, 0x15f) \
	GDOCOFS983_IN(off, m_bDay, 0x160) \
	GDOCOFS983_IN(off, m_bShowChat, 0x161) \
	GDOCOFS983_IN(off, m_unused152, 0x162) \
	GDOCOFS983_IN(off, m_saveChatNum, 0x164) \
	GDOCOFS983_IN(off, m_bReplaying, 0x168) \
	GDOCOFS983_IN(off, m_bDropEnable, 0x169) \
	GDOCOFS983_IN(off, m_bTutorialGaugeMute, 0x16a) \
	GDOCOFS983_IN(off, m_unused15b, 0x16b) \
	GDOCOFS983_IN(off, m_tutorialMinGauge983, 0x16c) \
	GDOCOFS983_IN(off, m_bGodMode, 0x170) \
	GDOCOFS983_IN(off, m_unused161, 0x171) \
	GDOCOFS983_IN(off, m_reserved174_983, 0x174) \
	GDOCOFS983_IN(off, m_debugBarValue983, 0x178) \
	GDOCOFS983_IN(off, m_currentHole, 0x17c) \
	GDOCOFS983_IN(off, m_unused169, 0x17d) \
	GDOCOFS983_IN(off, m_remainTime, 0x180) \
	GDOCOFS983_IN(off, m_shotTime, 0x184) \
	GDOCOFS983_IN(off, m_shotPhase, 0x188) \
	GDOCOFS983_IN(off, m_currentPlayer, 0x189) \
	GDOCOFS983_IN(off, m_unused176, 0x18a) \
	GDOCOFS983_IN(off, m_bright, 0x18c) \
	GDOCOFS983_IN(off, m_bDropRequest, 0x190) \
	GDOCOFS983_IN(off, m_bStopRequest, 0x191) \
	GDOCOFS983_IN(off, m_maxGrade, 0x192) \
	GDOCOFS983_IN(off, m_bStopDlg, 0x193) \
	GDOCOFS983_IN(off, m_bNoStat, 0x194) \
	GDOCOFS983_IN(off, m_b195_983, 0x195) \
	GDOCOFS983_IN(off, m_unused181, 0x196) \
	GDOCOFS983_IN(off, m_impactScale, 0x198) \
	GDOCOFS983_IN(off, m_controlScale, 0x19c) \
	GDOCOFS983_IN(off, m_skinsHoleWinner, 0x1a0) \
	GDOCOFS983_IN(off, m_skinsPenaltyUid, 0x1a4) \
	GDOCOFS983_IN(off, m_skinsNextTurnWord983, 0x1a8) \
	GDOCOFS983_IN(off, m_unused196, 0x1aa) \
	GDOCOFS983_IN(off, m_skinsHolePot, 0x1b0) \
	GDOCOFS983_IN(off, m_skinsPotBase, 0x1b8) \
	GDOCOFS983_IN(off, m_skinsPotMult, 0x1c0) \
	GDOCOFS983_IN(off, m_skinsCarryCount, 0x1c4) \
	GDOCOFS983_IN(off, m_unused1ad, 0x1c5) \
	GDOCOFS983_IN(off, m_skinsTotalPang, 0x1c8) \
	GDOCOFS983_IN(off, m_skinsWinnerBonus983, 0x1d0) \
	GDOCOFS983_IN(off, m_unused1c0, 0x1d8) \
	GDOCOFS983_IN(off, m_holeResult983, 0x340) \
	GDOCOFS983_IN(off, m_shotResultMsg, 0x3c4) \
	GDOCOFS983_IN(off, m_shotSync983, 0x45c) \
	GDOCOFS983_IN(off, m_remoteGauge983, 0x48e) \
	GDOCOFS983_IN(off, m_unused473, 0x493) \
	GDOCOFS983_IN(off, m_partTidList, 0x494) \
	GDOCOFS983_IN(off, m_bApproach794, 0x7b4) \
	GDOCOFS983_IN(off, m_bApproach795, 0x7b5) \
	GDOCOFS983_IN(off, m_approachMissionDesc, 0x7b6) \
	GDOCOFS983_IN(off, m_approachMissionType, 0x838) \
	GDOCOFS983_IN(off, m_approachPrizeType, 0x83c) \
	GDOCOFS983_IN(off, m_approachWinnerType, 0x840) \
	GDOCOFS983_IN(off, m_approachCurHoleResult, 0x844) \
	GDOCOFS983_IN(off, m_approachGameResult, 0x850) \
	GDOCOFS983_IN(off, m_approachTotalPrize, 0x85c) \
	GDOCOFS983_IN(off, m_pPangyaCorrection983, 0x860) \
	GDOCOFS983_IN(off, m_pPangyaAddCorrection983, 0x870) \
	GDOCOFS983_IN(off, m_pPangyaCorrectionSet983, 0x880) \
	GDOCOFS983_IN(off, m_pPowerShotItem983, 0x890) \
	GDOCOFS983_IN(off, m_pPlusYardItem983, 0x8a0) \
	GDOCOFS983_IN(off, m_pComboGaugeEveryshot983, 0x8b0) \
	(off))
#define GDOCPTR983(p, off) ((char*)(p) + GDOCOFS983(off))
C_ASSERT_VC10(offsetof(CGolfDoc, m_pSky) == 0xc && offsetof(CGolfDoc, m_pAutoTarget) == 0x10 && offsetof(CGolfDoc, m_propertyInfo) == 0x14 && offsetof(CGolfDoc, m_mapInfo) == 0x38);
C_ASSERT_VC10(offsetof(CGolfDoc, m_waveInfo) == 0x48 && offsetof(CGolfDoc, m_riverInfo) == 0x58 && offsetof(CGolfDoc, m_modelNameList) == 0x68 && offsetof(CGolfDoc, m_tutorialMode) == 0x88);
C_ASSERT_VC10(offsetof(CGolfDoc, m_inputList983) == 0x8c && offsetof(CGolfDoc, m_pCameraTargetPuppet) == 0x98 && offsetof(CGolfDoc, m_tutorialPin983) == 0x9c && offsetof(CGolfDoc, m_bTutorialPin983) == 0xa8);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused99) == 0xa9 && offsetof(CGolfDoc, m_xmlFile) == 0xac && offsetof(CGolfDoc, m_pPlayer) == 0xc8 && offsetof(CGolfDoc, m_pTeam) == 0xcc);
C_ASSERT_VC10(offsetof(CGolfDoc, m_teamNum) == 0xd0 && offsetof(CGolfDoc, m_currentPlayerNum) == 0xd4 && offsetof(CGolfDoc, m_playerNum) == 0xd5 && offsetof(CGolfDoc, m_unusedc6) == 0xd6);
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8 && offsetof(CGolfDoc, m_uiAlpha) == 0xdc && offsetof(CGolfDoc, m_unusedcd) == 0xdd && offsetof(CGolfDoc, m_bar) == 0xe0);
C_ASSERT_VC10(offsetof(CGolfDoc, m_cameraData) == 0x104 && offsetof(CGolfDoc, m_gameMode) == 0x158 && offsetof(CGolfDoc, m_bNetGameEnd) == 0x15c && offsetof(CGolfDoc, m_bPause) == 0x15d);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bTeeShotReady) == 0x15e && offsetof(CGolfDoc, m_bReport) == 0x15f && offsetof(CGolfDoc, m_bDay) == 0x160 && offsetof(CGolfDoc, m_bShowChat) == 0x161);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused152) == 0x162 && offsetof(CGolfDoc, m_saveChatNum) == 0x164 && offsetof(CGolfDoc, m_bReplaying) == 0x168 && offsetof(CGolfDoc, m_bDropEnable) == 0x169);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bTutorialGaugeMute) == 0x16a && offsetof(CGolfDoc, m_unused15b) == 0x16b && offsetof(CGolfDoc, m_tutorialMinGauge983) == 0x16c && offsetof(CGolfDoc, m_bGodMode) == 0x170);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused161) == 0x171 && offsetof(CGolfDoc, m_reserved174_983) == 0x174 && offsetof(CGolfDoc, m_debugBarValue983) == 0x178 && offsetof(CGolfDoc, m_currentHole) == 0x17c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused169) == 0x17d && offsetof(CGolfDoc, m_remainTime) == 0x180 && offsetof(CGolfDoc, m_shotTime) == 0x184 && offsetof(CGolfDoc, m_shotPhase) == 0x188);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentPlayer) == 0x189 && offsetof(CGolfDoc, m_unused176) == 0x18a && offsetof(CGolfDoc, m_bright) == 0x18c && offsetof(CGolfDoc, m_bDropRequest) == 0x190);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bStopRequest) == 0x191 && offsetof(CGolfDoc, m_maxGrade) == 0x192 && offsetof(CGolfDoc, m_bStopDlg) == 0x193 && offsetof(CGolfDoc, m_bNoStat) == 0x194);
C_ASSERT_VC10(offsetof(CGolfDoc, m_b195_983) == 0x195 && offsetof(CGolfDoc, m_unused181) == 0x196 && offsetof(CGolfDoc, m_impactScale) == 0x198 && offsetof(CGolfDoc, m_controlScale) == 0x19c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHoleWinner) == 0x1a0 && offsetof(CGolfDoc, m_skinsPenaltyUid) == 0x1a4 && offsetof(CGolfDoc, m_skinsNextTurnWord983) == 0x1a8 && offsetof(CGolfDoc, m_unused196) == 0x1aa);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHolePot) == 0x1b0 && offsetof(CGolfDoc, m_skinsPotBase) == 0x1b8 && offsetof(CGolfDoc, m_skinsPotMult) == 0x1c0 && offsetof(CGolfDoc, m_skinsCarryCount) == 0x1c4);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused1ad) == 0x1c5 && offsetof(CGolfDoc, m_skinsTotalPang) == 0x1c8 && offsetof(CGolfDoc, m_skinsWinnerBonus983) == 0x1d0 && offsetof(CGolfDoc, m_unused1c0) == 0x1d8);
C_ASSERT_VC10(offsetof(CGolfDoc, m_holeResult983) == 0x340 && offsetof(CGolfDoc, m_shotResultMsg) == 0x3c4 && offsetof(CGolfDoc, m_shotSync983) == 0x45c && offsetof(CGolfDoc, m_remoteGauge983) == 0x48e);
C_ASSERT_VC10(offsetof(CGolfDoc, m_unused473) == 0x493 && offsetof(CGolfDoc, m_partTidList) == 0x494 && offsetof(CGolfDoc, m_bApproach794) == 0x7b4 && offsetof(CGolfDoc, m_bApproach795) == 0x7b5);
C_ASSERT_VC10(offsetof(CGolfDoc, m_approachMissionDesc) == 0x7b6 && offsetof(CGolfDoc, m_approachMissionType) == 0x838 && offsetof(CGolfDoc, m_approachPrizeType) == 0x83c && offsetof(CGolfDoc, m_approachWinnerType) == 0x840);
C_ASSERT_VC10(offsetof(CGolfDoc, m_approachCurHoleResult) == 0x844 && offsetof(CGolfDoc, m_approachGameResult) == 0x850 && offsetof(CGolfDoc, m_approachTotalPrize) == 0x85c && offsetof(CGolfDoc, m_pPangyaCorrection983) == 0x860);
C_ASSERT_VC10(offsetof(CGolfDoc, m_pPangyaAddCorrection983) == 0x870 && offsetof(CGolfDoc, m_pPangyaCorrectionSet983) == 0x880 && offsetof(CGolfDoc, m_pPowerShotItem983) == 0x890 && offsetof(CGolfDoc, m_pComboGaugeEveryshot983) == 0x8b0);
