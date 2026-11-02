#include "minatl.h"
#include "vc10_layout.h"
#include "clientsetting.h"
#include "soundmanager.h"
#include "inputmanager.h"
#include "wvideo.h"
#include <shlwapi.h>
#include <string>
#include "wutil.h"
#include "frform.h"
#include "packet.h"
#include "shadow.h"
#include "../shared/localize.h"
#include "packetversion.h"
#include "golftask.h"
#include "taskmanager.h"
#include "networksystem.h"
#include "projectg.h"
#include "shareddoc.h"
#include "wview.h"
#include "wdevmng.h"
#include "browser.h"
#include "shadow.h"
#include "scenemanager.h"
#include "chatmsg.h"
#include "polysoup.h"
#include "rtti.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "commonutil.h"
#include "jp/uiframesetmanager.h"
#include "jp/refresh/rflobbydesktop.h"
#include "jp/refresh/rfscenemanager.h"
#include "quitwindow.h"
#include "golfdoc.h"
#include "jp/posteffect983.h"

extern Fresh* g_pFresh; // 00f02340

// JP 983: os dois singletons que o COption avisa sao classes da frente B: o RFLobbyDesktop (00e78ca8, visto como
// CLobbyState983) e o RFSceneManager (00e73e4c, visto como CUiPointer983). As visoes locais de antes (CLobbyStateOpt983 /
// CUiPointerOpt983) nao tinham corpo nem instancia: no exe viravam stubs com ponteiro nulo e o aviso nunca saia.

// JP 983: layout medido (Init 004951c0, SaveReg 00493010, InitVars 00491f10, CheckDebugRegValid 004934a0,
// SetVideoDevice 00496770, vChangeScreenSize 00494120; operator new 0x808 em 00a66f51)
struct COptionLayout983
{
	C_ASSERT(sizeof(sOption) == 0x344);
	C_ASSERT_VC10(sizeof(COption) == 0x808);
	C_ASSERT_VC10(offsetof(COption, m_pVideoDev) == 0xbc);
	C_ASSERT_VC10(offsetof(COption, m_pGlobalView) == 0xc0);
	C_ASSERT_VC10(offsetof(COption, m_opt) == 0xcc);
	C_ASSERT_VC10(offsetof(COption, m_opt.vScreenWidth) == 0xd8);
	C_ASSERT_VC10(offsetof(COption, m_opt.vMinScreenWidth) == 0xe4);
	C_ASSERT_VC10(offsetof(COption, m_opt.vScreenColor) == 0xec);
	C_ASSERT_VC10(offsetof(COption, m_opt.vGamma) == 0x128);
	C_ASSERT_VC10(offsetof(COption, m_opt.viHdr) == 0x12c);
	C_ASSERT_VC10(offsetof(COption, m_opt.viShaderOnOff) == 0x148);
	C_ASSERT_VC10(offsetof(COption, m_opt.aMssOn) == 0x14c);
	C_ASSERT_VC10(offsetof(COption, m_opt.gMouseSensitivity) == 0x174);
	C_ASSERT_VC10(offsetof(COption, m_opt.gLastLoginID) == 0x18c);
	C_ASSERT_VC10(offsetof(COption, m_opt.gIdentity) == 0x1a4);
	C_ASSERT_VC10(offsetof(COption, m_opt.gPowerGauge) == 0x1b8);
	C_ASSERT_VC10(offsetof(COption, m_opt.gRestore) == 0x1c8);
	C_ASSERT_VC10(offsetof(COption, m_opt.gAvatarNewbie) == 0x1d4);
	C_ASSERT_VC10(offsetof(COption, m_opt.gBindMouseCursor) == 0x1e8);
	C_ASSERT_VC10(offsetof(COption, m_opt.mMacro) == 0x1ec);
	C_ASSERT_VC10(offsetof(COption, m_opt.mSaveMacros) == 0x3ec);
	C_ASSERT_VC10(offsetof(COption, m_nTnLMode) == 0x410);
	C_ASSERT_VC10(offsetof(COption, m_bPendingChange) == 0x414);
	C_ASSERT_VC10(offsetof(COption, m_newOpt) == 0x418);
	C_ASSERT_VC10(offsetof(COption, m_bSwapMouseButton) == 0x75d);
	C_ASSERT_VC10(offsetof(COption, m_displayModeList) == 0x760);
	C_ASSERT_VC10(offsetof(COption, m_desktopMode983) == 0x76c);
	C_ASSERT(sizeof(COption::sDisplayMode) == 0x1c);
	C_ASSERT_VC10(offsetof(COption, m_nCaptureFmt) == 0x788);
	C_ASSERT_VC10(offsetof(COption, m_nLobbyCourse) == 0x7a0);
	C_ASSERT_VC10(offsetof(COption, m_nDefaultCharacter) == 0x7a4);
	C_ASSERT_VC10(offsetof(COption, m_nDefaultCaddie) == 0x7ac);
	C_ASSERT_VC10(offsetof(COption, m_nDefaultBall) == 0x7b4);
	C_ASSERT_VC10(offsetof(COption, m_nMap) == 0x7b8);
	C_ASSERT_VC10(offsetof(COption, m_nReplayMode) == 0x7c0);
	C_ASSERT_VC10(offsetof(COption, m_nNumPlayers) == 0x7c4);
	C_ASSERT_VC10(offsetof(COption, m_bVtxClrTest) == 0x7d4);
	C_ASSERT_VC10(offsetof(COption, m_nClickSystem) == 0x7dc);
	C_ASSERT_VC10(offsetof(COption, m_nGameType) == 0x7e0);
	C_ASSERT_VC10(offsetof(COption, m_fBarX983) == 0x7f4);
	C_ASSERT_VC10(offsetof(COption, m_fBarY) == 0x7f8);
	C_ASSERT_VC10(offsetof(COption, m_lastScreenWidth983) == 0x7fc);
	C_ASSERT_VC10(offsetof(COption, m_bLastScreenLobby983) == 0x804);
};


bool sOption::sbClipPlaneSupport;
bool sOption::sbMrtSupport;
bool sOption::sbPsSupport;
bool sOption::sbVsSupport;
bool sOption::sbRtSupport;
bool sOption::sbHwTnlSupport;

// unused, but the original emitted g_HUGE/g_EPSILON into .rdata
static bool IsValidRange(float f)
{
	return f < g_HUGE && f > g_EPSILON;
}

extern Fresh* g_pFresh;

// 983: WView::GetFOV é a função 00b4fa30 (wview.cpp), sem cópia inline aqui

#ifndef REBANG_PCH_MATH_HELPERS
#endif

void LimitText(char* text, int maxLen);

bool sOption::IsSupportShadow()
{
	return sbRtSupport;
}

bool sOption::IsSupportReflection()
{
	return sbRtSupport && sbClipPlaneSupport && sbVsSupport && sbPsSupport;
}

bool sOption::IsSupportLod()
{
	return sbRtSupport;
}

sOption::sOption()
{
	vWindowMode = 1;
	vFillMode = 0;
	vWideMode = 0;
	vScreenWidth = 800;
	vMinScreenWidth = 800;
	vScreenHeight = 600;
	vMinScreenHeight = 600;
	vScreenColor = 16;
	vOverall = -1;
	vUseMinScreen = 0;
	vTnLMode = 1;
	vDDSRes = 0;
	vUseMipmap = 0;
	vMaxMipLvl = 1;
	vMipCreateFilter = 2;
	vEffectLevel = 1;
	vProjectionShadow = 1;
	vShadowmap = 1;
	vReflection = 1;
	vLod = 0;
	vNpc = 1;
	viHdr = 0;
	viHdrStarEffect = 0;
	viDof = 0;
	viAmbientControl = 0;
	viRimLight = 0;
	viFXAA = 0;
	viShaderOnOff = 0;
	vGamma = 1.3f;
	vfBright = 3.0f;
	aOverall = 0;
	aMssOn = 1;
	aMssFreq = 44100;
	aMssBits = 16;
	aMssChannels = 2;
	aMssEnableHwSound = 1;
	aMssBalance = 128;
	aMssSpeaker = 0;
	aSfxVolume = 1.0f;
	aBGMVolume = 1.0f;
	gMouseSensitivity = 1.0f;
	gWhisper = 1;
	gInvitation = 1;
	gFriendConfirm = 1;
	gTransChatWin = 1;
	gUiEffect = 1;
	gLastLoginID[0] = 0;
	gIdentity = 0;
	gExtendChatLine = 0;
	gUserSort = 0;
	gPPL_Enable = 1;
	gPPL_Size = 0;
	gPowerGauge = 1;
	gFullyProportionalPowerGaugeSize = 0;
	gCenteredPowerGauge = 0;
	gCaptureLogo = 1;
	gRestore = 0;
	gChatWnd = 0;
	gUnderTabBtn = 0;
	gAvatarNewbie = 1;
	gCaptureHideGUI = 0;
	gCaptureHidePI = 0;
	gTerrainTooltip = 1;
	gCutinDisplay = 1;
	gBindMouseCursor = 1;
	for (int i = 0; i < 8; ++i)
	{
		mMacro[i][0] = 0;
	}
	mSaveMacros = 1;
}

sOption::~sOption()
{
}

void sOption::vSetOverallByVidOptions()
{
	if (vScreenWidth == 1024 && vScreenHeight == 768 && vScreenColor == 32 &&
		vDDSRes == 0 && vEffectLevel == 0 && vProjectionShadow == 1 &&
		vShadowmap == 1 && vReflection == 1 && vLod == 0 && vNpc == 1
		&& viHdr == 1 && viHdrStarEffect == 1 && viDof == 1 &&
		viShaderOnOff == 1 && viAmbientControl == 1 && viRimLight == 1 &&
		viFXAA == 1)
	{
		vOverall = 0;
		return;
	}
	if (vScreenWidth == 800 && vScreenHeight == 600 && vScreenColor == 32 &&
		vDDSRes == 1 && vEffectLevel == 1 && vProjectionShadow == 1 &&
		vShadowmap == 1 && vReflection == 1 && vLod == 1 && vNpc == 1
		&& viHdr == 1 && viHdrStarEffect == 1 && viDof == 1 &&
		viShaderOnOff == 1 && viAmbientControl == 1 && viRimLight == 1 &&
		viFXAA == 1)
	{
		vOverall = 1;
		return;
	}
	if (vScreenWidth == 640 && vScreenHeight == 480 && vScreenColor == 16 &&
		vDDSRes == 2 && vEffectLevel == 2 && vProjectionShadow == 0 &&
		vShadowmap == 0 && vReflection == 0 && vLod == 1 && vNpc == 0
		&& viHdr == 0 && viHdrStarEffect == 0 && viDof == 0 &&
		viShaderOnOff == 0 && viAmbientControl == 0 && viRimLight == 0 &&
		viFXAA == 0)
	{
		vOverall = 2;
		return;
	}
	vOverall = -1;
}

void sOption::vResetByOverall(const char* text)
{
	if (!strcmp(text, vGetOverallText(0)))
	{
		vOverall = 0;
		vUseMinScreen = 0;
		vScreenWidth = 1024;
		vScreenHeight = 768;
		vScreenColor = 32;
		vDDSRes = 0;
		vEffectLevel = 0;
		vProjectionShadow = IsSupportShadow() != false;
		vShadowmap = 1;
		vReflection = IsSupportReflection() != false;
		vLod = 0;
		vNpc = 1;
		goto vidShaders;
		return;
	}
	if (!strcmp(text, vGetOverallText(1)))
	{
		vOverall = 1;
		vUseMinScreen = 0;
		vScreenWidth = 800;
		vScreenHeight = 600;
		vScreenColor = 32;
		vDDSRes = 1;
		vEffectLevel = 1;
		vProjectionShadow = IsSupportShadow() != false;
		vShadowmap = 1;
		vReflection = IsSupportReflection() != false;
		vLod = IsSupportLod() != false;
		vNpc = 1;
vidShaders:
		viHdr = IsSupportVidShader983();
		viHdrStarEffect = IsSupportVidShader983();
		viDof = IsSupportVidShader983();
		viAmbientControl = IsSupportVidShader983();
		viRimLight = IsSupportVidShader983();
		viFXAA = IsSupportVidShader983();
		viShaderOnOff = IsSupportVidShader983();
		return;
	}
	if (!strcmp(text, vGetOverallText(2)))
	{
		vUseMinScreen = 0;
		vOverall = 2;
		vDDSRes = 2;
		vEffectLevel = 2;
		vScreenWidth = 640;
		vScreenHeight = 480;
		vScreenColor = 16;
		vProjectionShadow = 0;
		vShadowmap = 0;
		vReflection = 0;
		vLod = IsSupportLod() != false;
		vNpc = 0;
		viHdr = 0;
		viHdrStarEffect = 0;
		viDof = 0;
		viAmbientControl = 0;
		viRimLight = 0;
		viFXAA = 0;
		viShaderOnOff = 0;
	}
}

const char* sOption::vGetOverallText(int lvl)
{
	return GetThreeLvlText(lvl);
}

int sOption::vGetOverallLvl(const char* text)
{
	return vGetLvl3FromText(text);
}

const char* sOption::vGetTnlText(int lvl)
{
	if (lvl == 0)
	{
		return K2L_Compatibility(
			"\xbc\xd2\xc7\xc1\xc6\xae\xbf\xfe\xbe\xee TnL");
	}
	return lvl == 1 ? K2L_Compatibility("\xc7\xcf\xb5\xe5\xbf\xfe\xbe\xee TnL")
					: "";
}

int sOption::vGetTnlLvl(const char* text)
{
	// the 983 inlines vGetTnlText here (literal straight into K2L_Compatibility)
	if (!strcmp(text, K2L_Compatibility(
						  "\xbc\xd2\xc7\xc1\xc6\xae\xbf\xfe\xbe\xee TnL")))
	{
		return 0;
	}
	return strcmp(text, K2L_Compatibility("\xc7\xcf\xb5\xe5\xbf\xfe\xbe\xee TnL")) == 0
		? 1
		: -1;
}

const char* sOption::vGetTexResText(int lvl)
{
	return GetThreeLvlText(lvl);
}

int sOption::vGetTexResLvl(const char* text)
{
	return vGetLvl3FromText(text);
}

const char* sOption::vGetParticleText(int lvl)
{
	return GetThreeLvlText(lvl);
}

int sOption::vGetParticleLvl(const char* text)
{
	return vGetLvl3FromText(text);
}

const char* sOption::aGetOverallText(int lvl)
{
	return GetThreeLvlText(lvl);
}

int sOption::aGetOverallLvl(const char* text)
{
	return vGetLvl3FromText(text);
}

const char* sOption::aGetSpeakerText(int lvl)
{
	switch (lvl)
	{
	case 0:
		return K2L_Compatibility("2\xc3\xa4\xb3\xce \xbd\xba\xc7\xc7\xc4\xbf");
	case 1:
		return K2L_Compatibility("\xc7\xec\xb5\xe5\xc6\xf9");
	case 2:
		return K2L_Compatibility("\xbc\xad\xb6\xf3\xbf\xee\xb5\xe5");
	case 3:
		return K2L_Compatibility("4\xc3\xa4\xb3\xce \xbd\xba\xc7\xc7\xc4\xbf");
	case 4:
		return K2L_Compatibility(
			"5.1\xc3\xa4\xb3\xce \xbd\xba\xc7\xc7\xc4\xbf");
	case 5:
		return K2L_Compatibility(
			"7.1\xc3\xa4\xb3\xce \xbd\xba\xc7\xc7\xc4\xbf");
	default:
		return "";
	}
}

int sOption::aGetSpeakerLvl(const char* text)
{
	for (int i = 0; i < 6; ++i)
	{
		if (strcmp(text, aGetSpeakerText(i)) == 0)
		{
			return i;
		}
	}
	return 0;
}

const char* sOption::GetThreeLvlText(int lvl)
{
	switch (lvl)
	{
	case 0:
		return K2L_Compatibility("\xb3\xf4\xc0\xbd");
	case 1:
		return K2L_Compatibility("\xc1\xdf\xb0\xa3");
	case 2:
		return K2L_Compatibility("\xb3\xb7\xc0\xbd");
	default:
		return K2L_Compatibility("\xbb\xe7\xbf\xeb\xc0\xda");
	}
}

const char* sOption::GetTwoLvlText(int lvl)
{
	switch (lvl)
	{
	case 0:
		return K2L_Compatibility("\xb3\xf4\xc0\xbd");
	case 1:
		return K2L_Compatibility("\xb3\xb7\xc0\xbd");
	default:
		return K2L_Compatibility("\xbb\xe7\xbf\xeb\xc0\xda");
	}
}

void COption::SaveReg()
{
	// JP 983 (00493010): numeracao da tabela do 983; vGamma continua sem gravar
	WriteReg_I(1, m_opt.vWindowMode);
	WriteReg_I(2, m_opt.vFillMode);
	WriteReg_I(3, m_opt.vWideMode);
	WriteReg_I(4, m_opt.vOverall);
	WriteReg_I(5, m_opt.vScreenWidth);
	WriteReg_I(6, m_opt.vScreenHeight);
	WriteReg_I(7, m_opt.vScreenColor);
	WriteReg_I(8, m_opt.vTnLMode);
	WriteReg_I(9, m_opt.vDDSRes);
	WriteReg_I(10, m_opt.vUseMipmap);
	WriteReg_I(11, m_opt.vMaxMipLvl);
	WriteReg_I(12, m_opt.vMipCreateFilter);
	WriteReg_I(13, m_opt.vEffectLevel);
	WriteReg_I(14, m_opt.vProjectionShadow);
	WriteReg_I(15, m_opt.vShadowmap);
	WriteReg_I(16, m_opt.vReflection);
	WriteReg_I(17, m_opt.vLod);
	WriteReg_I(18, m_opt.vNpc);
	WriteReg_I(19, m_opt.vCoordX);
	WriteReg_I(20, m_opt.vCoordY);
	WriteReg_I(22, m_opt.viHdr);
	WriteReg_I(23, m_opt.viHdrStarEffect);
	WriteReg_I(24, m_opt.viDof);
	WriteReg_I(26, m_opt.viAmbientControl);
	WriteReg_I(25, m_opt.viRimLight);
	WriteReg_I(27, m_opt.viFXAA);
	WriteReg_F(28, m_opt.vfBright);
	WriteReg_I(29, m_opt.viShaderOnOff);
	WriteReg_I(30, m_opt.aMssOn);
	WriteReg_I(31, m_opt.aOverall);
	WriteReg_I(32, m_opt.aMssFreq);
	WriteReg_I(33, m_opt.aMssBits);
	WriteReg_I(34, m_opt.aMssChannels);
	WriteReg_I(35, m_opt.aMssEnableHwSound);
	WriteReg_I(36, m_opt.aMssBalance);
	WriteReg_I(37, m_opt.aMssSpeaker);
	WriteReg_F(38, m_opt.aSfxVolume);
	WriteReg_F(39, m_opt.aBGMVolume);
	WriteReg_F(40, m_opt.gMouseSensitivity);
	WriteReg_I(41, m_opt.gWhisper);
	WriteReg_I(42, m_opt.gInvitation);
	WriteReg_I(43, m_opt.gFriendConfirm);
	WriteReg_I(44, m_opt.gTransChatWin);
	WriteReg_I(45, m_opt.gUiEffect);
	WriteReg_S(46, m_opt.gLastLoginID);
	WriteReg_I(47, m_opt.gIdentity);
	WriteReg_I(48, m_opt.gExtendChatLine);
	WriteReg_I(49, m_opt.gUserSort);
	WriteReg_I(50, m_opt.gPPL_Enable);
	WriteReg_I(51, m_opt.gPPL_Size);
	WriteReg_I(52, m_opt.gPowerGauge);
	WriteReg_I(53, m_opt.gFullyProportionalPowerGaugeSize);
	WriteReg_I(54, m_opt.gCenteredPowerGauge);
	WriteReg_I(55, m_opt.gCaptureLogo);
	WriteReg_I(56, m_opt.gRestore);
	WriteReg_I(57, m_opt.gChatWnd);
	WriteReg_I(58, m_opt.gUnderTabBtn);
	WriteReg_I(59, m_opt.gAvatarNewbie);
	WriteReg_I(60, m_opt.gCaptureHideGUI);
	WriteReg_I(61, m_opt.gCaptureHidePI);
	WriteReg_I(62, m_opt.gTerrainTooltip);
	WriteReg_I(63, m_opt.gCutinDisplay);
	WriteReg_I(64, m_opt.gBindMouseCursor);
	for (int i = 0; i < 8; ++i)
	{
		WriteReg_S(65 + i, m_opt.mMacro[i]);
	}
	WriteReg_I(73, m_opt.mSaveMacros);
}

int sOption::vGetLvl3FromText(const char* text)
{
	for (int i = 0; i < 3; ++i)
	{
		if (strcmp(text, vGetOverallText(i)) == 0)
		{
			return i;
		}
	}
	return -1;
}

void COption::InitVars()
{
	// JP 983 (00491f10): campos novos m_fBarX983 e o ultimo pedido de tela (800x600)
	m_pVideoDev = 0;
	m_fBarX983 = (float)m_opt.vScreenWidth;
	m_pAudioManager = 0;
	m_pInputManager = 0;
	m_bPendingChange = false;
	m_bSwapMouseButton = false;
	m_bQuitWindowed = false;
	m_nCaptureFmt = 0;
	m_nDefaultCharacter = 0;
	m_nDefaultHairColor = 0;
	m_fBarY = (float)m_opt.vScreenHeight - 62.0f;
	m_nDefaultCaddie = 0;
	m_nAviWidth = 320;
	m_nAviHeight = 240;
	m_nAviFps = 10;
	m_nStartMode = 0;
	m_bDbgStartFlag983 = 0;
	m_nLobbyCourse = -1;
	m_nMap = 0;
	m_nStartHole = 1;
	m_nReplayMode = 0;
	m_nNumPlayers = 1;
	m_nGameTime = 1;
	m_nWeather = 0;
	m_nClickSystem = 0;
	m_bVtxClrTest = 0;
	m_bBaseMeshTest = 0;
	m_bVersionInfo = 0;
	m_lastScreenWidth983 = 800;
	m_lastScreenHeight983 = 600;
	m_bLastScreenLobby983 = false;
}

bool COption::ReadReg(int index, char* buf, int size)
{
	if (index >= 0 && index < 74)
	{
		return m_reg.Read((char*)ms_szReg[index][0], buf, size);
	}
	return false;
}

bool COption::ScanReg_I(int index, int* value)
{
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		return m_reg.Scan((char*)ms_szReg[index][0], "%d", value) ? true
																  : false;
	}
	return false;
}

bool COption::ScanReg_F(int index, float* value)
{
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		return m_reg.Scan((char*)ms_szReg[index][0], "%f", value) ? true
																  : false;
	}
	return false;
}

bool COption::ScanReg_S(int index, char* value)
{
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		if (m_reg.Scan((char*)ms_szReg[index][0], "%s", value) != 0)
		{
			return true;
		}
		*value = 0;
		return false;
	}
	*value = 0;
	return false;
}

void COption::WriteReg(int index, const char* value)
{
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		m_reg.Write((char*)ms_szReg[index][0], value);
	}
}

bool COption::WriteReg_I(int index, int value)
{
	static char buf[256];
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		sprintf(buf, "%d", value);
		m_reg.Write((char*)ms_szReg[index][0], buf);
		return true;
	}
	return false;
}

bool COption::WriteReg_F(int index, float value)
{
	static char buf[256];
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		sprintf(buf, "%.2f", value);
		m_reg.Write((char*)ms_szReg[index][0], buf);
		return true;
	}
	return false;
}

bool COption::WriteReg_S(int index, const char* value)
{
	static char buf[256];
	if (index >= 0 && index < 74 && ms_szReg[index][0] &&
		strlen(ms_szReg[index][0]) != 0)
	{
		sprintf(buf, "%s", value);
		m_reg.Write((char*)ms_szReg[index][0], buf);
		return true;
	}
	return false;
}

// HACK: 512 increments of unused inline IL place the ATL helpers at their original
// PCH merge point without delaying the allocator _Construct COMDAT.
struct RbClientsettingAtlPad
{
	int v;
	void Grow() { ILFILL256 ILFILL256 }
};

void COption::SetGlobalView(WView* pView)
{
	m_pGlobalView = pView;
}

void COption::SetAudioManager(CSoundManager* pAudioManager)
{
	m_pAudioManager = pAudioManager;
}

void COption::vApplyMipmap()
{
	m_pVideoDev->Command((wVDevMessage)0x18, m_opt.vDDSRes, 0);
	m_pVideoDev->Command((wVDevMessage)0x19, m_opt.vUseMipmap, 0);
	m_pVideoDev->Command((wVDevMessage)0x1a, m_opt.vMaxMipLvl, 0);
	m_pVideoDev->Command((wVDevMessage)0x1b, m_opt.vMipCreateFilter, 0);
}

void COption::vSetMinimumScreenSize(int bUse, int width, int height)
{
	m_opt.vUseMinScreen = bUse;
	m_opt.vMinScreenWidth = width;
	m_opt.vMinScreenHeight = height;
}

void COption::vEnableShadow(bool bEnable)
{
	m_opt.vProjectionShadow = bEnable ? 1 : 0;
}

void COption::vEnableShadowmap(bool bEnable)
{
	m_opt.vShadowmap = bEnable ? 1 : 0;
}

bool COption::vIsWindowed()
{
	return m_opt.vWindowMode ? true : false;
}

bool COption::vIsWideMode()
{
	return m_opt.vWideMode ? true : false;
}

bool COption::vIsShadowEnabled()
{
	return m_opt.vProjectionShadow ? true : false;
}

bool COption::vIsShadowmapEnabled()
{
	return m_opt.vShadowmap ? true : false;
}

bool COption::vIsReflectionEnabled()
{
	return m_opt.vReflection ? true : false;
}

bool COption::vIsLodEnabled()
{
	return m_opt.vLod ? true : false;
}

bool COption::vIsNpcEnabled()
{
	return m_opt.vNpc ? true : false;
}

bool COption::aIsMssEnabled()
{
	return m_opt.aMssOn ? true : false;
}

bool COption::aIsMssHwSoundEnabled()
{
	return m_opt.aMssEnableHwSound ? true : false;
}

WMilesSoundSystem::w_speaker_type COption::aGetMssSpeaker()
{
	return (WMilesSoundSystem::w_speaker_type)m_opt.aMssSpeaker;
}

void COption::aApplyAudioSetting()
{
	g_audio->SetSpeakerType(m_opt.aMssSpeaker);
}

bool COption::gIsWhisperAllowed()
{
	return m_opt.gWhisper ? true : false;
}

bool COption::gIsInvitationAllowed()
{
	return m_opt.gInvitation ? true : false;
}

bool COption::gIsFriendConfirmNeeded()
{
	return m_opt.gFriendConfirm ? true : false;
}

bool COption::gIsChatWinTransparent()
{
	return m_opt.gTransChatWin ? true : false;
}

bool COption::gIsChatLineEntended()
{
	return m_opt.gExtendChatLine ? true : false;
}

bool COption::gIsPPLEnable()
{
	return m_opt.gPPL_Enable ? true : false;
}

void COption::gSetPPLEnable(bool bEnable)
{
	m_opt.gPPL_Enable = bEnable;
}

void COption::gSetPPLSize(int size)
{
	m_opt.gPPL_Size = size;
}

void COption::gResetRestore()
{
	m_opt.gRestore = 0;
	WriteReg_I(56, 0);
}

const char* COption::dGetCaptureFormat()
{
	return ms_szCaptureFmt[m_nCaptureFmt];
}

void COption::dSetCaptureFormat(int format)
{
	m_nCaptureFmt = format;
}

int COption::dGetDefaultCharacter()
{
	return m_nDefaultCharacter;
}

int COption::dGetDefaultHairColor()
{
	return m_nDefaultHairColor;
}

int COption::dGetDefaultCaddie()
{
	return m_nDefaultCaddie;
}

int COption::dGetDefaultClub()
{
	return m_nDefaultClub;
}

int COption::dGetDefaultBall()
{
	return m_nDefaultBall;
}

int COption::dGetMap()
{
	return m_nMap;
}

int COption::dGetAviWidth()
{
	return m_nAviWidth;
}

int COption::dGetAviHeight()
{
	return m_nAviHeight;
}

int COption::dGetAviFps()
{
	return m_nAviFps;
}

int COption::dGetStartMode()
{
	return m_nStartMode;
}

int COption::dGetLobbyCourse()
{
	return m_nLobbyCourse;
}

int COption::dGetNumPlayers()
{
	return m_nNumPlayers;
}

unsigned char COption::dGetChatGender()
{
	return (unsigned char)m_nChatGender;
}

bool COption::dIsVtxClrTestEnabled()
{
	return m_bVtxClrTest ? true : false;
}

bool COption::dIsBaseMeshTestEnabled()
{
	return m_bBaseMeshTest ? true : false;
}

int COption::dGetClickSystem()
{
	return m_nClickSystem;
}

int COption::dGetReplayMode()
{
	return m_nReplayMode;
}

void COption::dSetReplayMode(int mode)
{
	m_nReplayMode = mode;
}

int COption::dGetGameType()
{
	return m_nGameType;
}

int COption::dGetWeather()
{
	return m_nWeather;
}

unsigned char COption::dGetStartHole()
{
	return (unsigned char)m_nStartHole;
}

bool COption::dIsVersionInfoEnabled()
{
	return m_bVersionInfo ? true : false;
}

float COption::gGetBar_Y()
{
	return m_fBarY;
}

void COption::ApplyChange(sOption& option, bool bGameScreen)
{
	m_bPendingChange = true;
	m_newOpt = option;
	m_bGameScreen = bGameScreen;
}

const sOption* COption::GetOption()
{
	return &m_opt;
}

void COption::ChangeGameOption(sOption& option, sGameOptionInfo& info)
{
	for (int i = 0; i < 8; ++i)
	{
		strcpy(info.macro[i], option.mMacro[i]);
	}
}

void COption::ChangeGameOption(sGameOptionInfo& info, sOption& option)
{
	for (int i = 0; i < 8; ++i)
	{
		strcpy(option.mMacro[i], info.macro[i]);
	}
}

__declspec(selectany) ATL::_ATL_FUNC_INFO BeforeNavigate2Info = {
	CC_STDCALL, VT_EMPTY, 7,
	{ VT_DISPATCH, VT_VARIANT | VT_BYREF, VT_VARIANT | VT_BYREF,
				   VT_VARIANT | VT_BYREF, VT_VARIANT | VT_BYREF, VT_VARIANT | VT_BYREF,
				   VT_BOOL | VT_BYREF }
};
__declspec(selectany) ATL::_ATL_FUNC_INFO NavigateComplete2Info = {
	CC_STDCALL, VT_EMPTY, 2, { VT_DISPATCH, VT_VARIANT | VT_BYREF }
};
__declspec(selectany) ATL::_ATL_FUNC_INFO WindowClosingInfo = {
	CC_STDCALL, VT_EMPTY, 2, { VT_BOOL, VT_BOOL | VT_BYREF }
};
__declspec(selectany) ATL::_ATL_FUNC_INFO DownloadInfo = { CC_STDCALL, VT_EMPTY,
	0, { 0 } };

// JP 983: 10 modos normais (sem 1280x1024; 00e044f0) e 34 largos (+ 2048x1152, 2560x1440, 2560x1600; 00e04568)
static int s_normalModes[10][3] = {
	{ 640,  480,  16 },
	{ 640,  480,  32 },
	{ 800,  600,  16 },
	{ 800,  600,  32 },
	{ 1024, 768,  16 },
	{ 1024, 768,  32 },
	{ 1280, 960,  16 },
	{ 1280, 960,  32 },
	{ 1600, 1200, 16 },
	{ 1600, 1200, 32 },
};

static int s_wideModes[34][3] = {
	{ 1024, 576,  16 },
	{ 1024, 576,  32 },
	{ 1280, 720,  16 },
	{ 1280, 720,  32 },
	{ 1280, 768,  16 },
	{ 1280, 768,  32 },
	{ 1280, 800,  16 },
	{ 1280, 800,  32 },
	{ 1366, 768,  16 },
	{ 1366, 768,  32 },
	{ 1400, 900,  16 },
	{ 1400, 900,  32 },
	{ 1440, 990,  16 },
	{ 1440, 990,  32 },
	{ 1400, 1050, 16 },
	{ 1400, 1050, 32 },
	{ 1440, 900,  16 },
	{ 1440, 900,  32 },
	{ 1440, 1050, 16 },
	{ 1440, 1050, 32 },
	{ 1600, 900,  16 },
	{ 1600, 900,  32 },
	{ 1680, 1050, 16 },
	{ 1680, 1050, 32 },
	{ 1920, 1080, 16 },
	{ 1920, 1080, 32 },
	{ 1920, 1200, 16 },
	{ 1920, 1200, 32 },
	{ 2048, 1152, 16 },
	{ 2048, 1152, 32 },
	{ 2560, 1440, 16 },
	{ 2560, 1440, 32 },
	{ 2560, 1600, 16 },
	{ 2560, 1600, 32 },
};

const char* COption::ms_szCaptureFmt[] = { "jpg", "bmp", "avi", 0 };

// JP 983: tabela de 74 entradas (00e04710; inicializador 00cc8970). Diferencas do 645: 8 campos novos do video
// (22-29), 2 da barra de forca (53, 54), gBindMouseCursor (64); vWindowMode padrao "1"; macros novas; mSaveMacros "0".
const char* COption::ms_szReg[][2] = {
	{ "Ver", "1.xx" },
	{ "Option\\vWindowMode", "1" },
	{ "Option\\vFillMode", "0" },
	{ "Option\\vWideMode", "0" },
	{ "Option\\vOverall", "-1" },
	{ "Option\\vScreenWidth", "800" },
	{ "Option\\vScreenHeight", "600" },
	{ "Option\\vScreenColor", "32" },
	{ "Option\\vTnLMode", "1" },
	{ "Option\\Mip\\vDDSRes", "0" },
	{ "Option\\Mip\\vUseMipmap", "0" },
	{ "Option\\Mip\\vMaxMipLvl", "1" },
	{ "Option\\Mip\\vMipCreateFilter", "2" },
	{ "Option\\vEffectLevel", "1" },
	{ "Option\\vProjectionShadow", "1" },
	{ "Option\\vShadowmap", "1" },
	{ "Option\\vReflection", "0" },
	{ "Option\\vLod", "0" },
	{ "Option\\vNpc", "1" },
	{ "Option\\vCoordX", "0" },
	{ "Option\\vCoordY", "0" },
	{ "Option\\vGamma", "1.3" },
	{ "Option\\viHdr", "0" },
	{ "Option\\viHdrStarEffect", "0" },
	{ "Option\\viDof", "0" },
	{ "Option\\viRimLight", "0" },
	{ "Option\\viAmbientControl", "0" },
	{ "Option\\viFXAA", "0" },
	{ "Option\\vfBright", "3" },
	{ "Option\\viShaderOnOff", "0" },
	{ "Option\\aMssOn", "1" },
	{ "Option\\aOverall", "0" },
	{ "Option\\aMssFreq", "44100" },
	{ "Option\\aMssBits", "16" },
	{ "Option\\aMssChannels", "2" },
	{ "Option\\aMssEnableHwSound", "1" },
	{ "Option\\aMssBalance", "128" },
	{ "Option\\aMssSpeaker", "0" },
	{ "Option\\aSfxVolume", "1.00" },
	{ "Option\\aBGMVolume", "1.00" },
	{ "Option\\gMouseSensitivity", "1.00" },
	{ "Option\\gWhisper", "1" },
	{ "Option\\gInvitation", "1" },
	{ "Option\\gFriendConfirm", "1" },
	{ "Option\\gTransChatWin", "1" },
	{ "Option\\gUiEffect", "1" },
	{ "Option\\gLastLoginID", "" },
	{ "Option\\gIdentity", "0" },
	{ "Option\\gExtendChatLine", "0" },
	{ "Option\\gUserSort", "0" },
	{ "Option\\gPPL_Enable", "1" },
	{ "Option\\gPPL_Size", "0" },
	{ "Option\\gPowerGauge", "1" },
	{ "Option\\gFullyProportionalPowerGaugeSize", "0" },
	{ "Option\\gCenteredPowerGauge", "0" },
	{ "Option\\gCaptureLogo", "1" },
	{ "Option\\gRestore", "0" },
	{ "Option\\gChatWnd", "0" },
	{ "Option\\gUnderTabBtn", "0" },
	{ "Option\\gAvatarNewbie", "1" },
	{ "Option\\gCaptureHideGUI", "0" },
	{ "Option\\gCaptureHidePI", "0" },
	{ "Option\\gTerrainTooltip", "1" },
	{ "Option\\gCutinDisplay", "1" },
	{ "Option\\gBindMouseCursor", "1" },
	{ "Option\\mMacro1", K2L_Compatibility("\xbe\xc8\xb3\xe7\xc7\xcf\xbc\xbc\xbf\xe4.") },
	{ "Option\\mMacro2", K2L_Compatibility("\xb9\xdd\xb0\xa9\xbd\xc0\xb4\xcf\xb4\xd9.") },
	{ "Option\\mMacro3", K2L_Compatibility("\xbe\xc6~~~ \xbe\xc6\xb1\xf5\xb4\xd9.") },
	{ "Option\\mMacro4", K2L_Compatibility("\xc6\xce\xbe\xdf \xb3\xca\xb9\xab \xc0\xe7\xb9\xcc\xc0\xd6\xbe\xee\xbf\xe4.") },
	{ "Option\\mMacro5", K2L_Compatibility("\xc0\xcc\xb9\xf8 \xc7\xd1\xb9\xe6\xbf\xa1 \xb3\xa1\xb3\xbb\xb0\xda\xbe\xee\xbf\xe4.") },
	{ "Option\\mMacro6", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba~ \xbc\xa6!") },
	{ "Option\\mMacro7", K2L_Compatibility("\xb1\xc2! \xbc\xa6!") },
	{ "Option\\mMacro8", K2L_Compatibility("\xbf\xc0\xbf\xc0..\xb8\xda\xc1\xae\xbf\xe4.") },
	{ "Option\\mSaveMacros", "0" },
};


void COption::CheckDebugRegValid()
{
	// JP 983 (004934a0): campos novos 0x79c/0x7a8/0x7e4/0x7e8/0x7ec; m_nDefaultCaddie (0x7ac) >= -1; m_nGameType vai ate 23
	m_nCaptureFmt = Between(0, m_nCaptureFmt, 2);
	m_nDefaultCharacter = Max(0, m_nDefaultCharacter);
	m_nDefaultHairColor = Max(0, m_nDefaultHairColor);
	m_nDefaultCaddie = Max(-1, m_nDefaultCaddie);
	int width = m_nAviWidth;
	if (width != 640 && width != 800)
	{
		width = 320;
	}
	m_nAviWidth = width;
	m_nAviHeight = width == 640 ? 480 : width == 800 ? 600 : 240;
	m_nAviFps = Max(1, m_nAviFps);
	m_nStartMode = Between(0, m_nStartMode, 3);
	m_bDbgStartFlag983 = m_bDbgStartFlag983 != 0;
	m_nLobbyCourse = Max(-1, m_nLobbyCourse);
	m_nMap = Max(0, m_nMap);
	m_nStartHole = Between(1, m_nStartHole, 18);
	m_nNumPlayers = Between(1, m_nNumPlayers, 30);
	m_nGameTime = Max(0, m_nGameTime);
	m_nWeather = Between(0, m_nWeather, 4);
	m_nChatGender = m_nChatGender != 0;
	m_bVtxClrTest = m_bVtxClrTest != 0;
	m_bBaseMeshTest = m_bBaseMeshTest != 0;
	m_nClickSystem = m_nClickSystem != 0;
	m_nReplayMode = Between(0, m_nReplayMode, 3);
	m_nGameType = Between(0, m_nGameType, 23);
	m_nDbgGameType2_983 = Between(0, m_nDbgGameType2_983, 23);
	m_nDbgValue983 = Max(0, m_nDbgValue983);
	m_bDbgFlag983 = m_bDbgFlag983 != 0;
	m_bVersionInfo = m_bVersionInfo != 0;
}

void COption::vSetMipmap(int ddsRes, bool bUseMipmap, int maxMipLvl,
	int mipCreateFilter)
{
	m_opt.vDDSRes = Max(0, ddsRes);
	m_opt.vUseMipmap = bUseMipmap ? 1 : 0;
	m_opt.vMaxMipLvl = Max(0, maxMipLvl);
	m_opt.vMipCreateFilter = Max(0, mipCreateFilter);
}

void COption::vSetParticleLevel(int level)
{
	m_opt.vEffectLevel = Between(0, level, 2);
}

void COption::aApplySfxVolume(float volume)
{
	if (m_pAudioManager)
	{
		m_pAudioManager->SetVolume(Between(0.0f, volume, 1.0f));
	}
}

void COption::aApplyBgmVolume(float volume)
{
	if (m_pAudioManager)
	{
		m_pAudioManager->SetBGMVolume(Between(0.0f, volume, 1.0f));
	}
}

void COption::aSetSfxVolume(float volume)
{
	m_opt.aSfxVolume = Between(0.0f, volume, 1.0f);
}

void COption::aSetBgmVolume(float volume)
{
	m_opt.aBGMVolume = Between(0.0f, volume, 1.0f);
}

void COption::aSetSpeakerType(int type)
{
	m_opt.aMssSpeaker = Between(0, type, 5);
}

void COption::gSetMouseSensitivity(float sensitivity)
{
	m_opt.gMouseSensitivity = Between(0.5f, sensitivity, 2.0f);
	m_pInputManager->SetSensitivity(sensitivity);
}

void COption::dSetNumPlayers(int num)
{
	m_nNumPlayers = Between(1, num, 30);
}

int COption::vGetScrResNum()
{
	return m_displayModeList.size();
}

void COption::SetInputManager(CInputManager* pInputManager)
{
	if (0)
		((sOption*)0)->sOption::vGetLvl3FromText(*(const char**)0);
	m_pInputManager = pInputManager;
	if (pInputManager)
	{
		pInputManager->SwapMouseButton(m_bSwapMouseButton);
	}
	gSetMouseSensitivity(m_opt.gMouseSensitivity);
}

void COption::vApplyGameScreenSize()
{
	// JP 983 (00496f80): uma chamada so (4o argumento false), depois a barra de forca e as imagens da UI nova
	int width = m_opt.vScreenWidth;
	int height = m_opt.vScreenHeight;
	if (m_opt.vUseMinScreen &&
		(width < m_opt.vMinScreenWidth || height < m_opt.vMinScreenHeight))
	{
		height = m_opt.vMinScreenHeight;
		width = m_opt.vMinScreenWidth;
	}
	vChangeScreenSize(width, height, m_opt.vScreenColor, false);
	UpdatePowerGaugeLayout983();
	CUiFrameSetManager::Instance()->ReloadImages983(m_pGlobalView);
}

#pragma comment(linker, \
	"/alternatename:@rb_ShadowReset@4=?Reset@CShadowManager@@QAEXXZ")

void COption::vApplyLobbyScreenSize()
{
	// JP 983 (00494950): o lobby usa o tamanho escolhido (nao mais 800x600); 4o argumento true; avisa o lobby
	if (CShadowManager::Instance())
	{
		CShadowManager::Instance()->Reset(); // 645: apelido rb_ShadowReset
	}
	vChangeScreenSize(m_opt.vScreenWidth, m_opt.vScreenHeight,
		m_opt.vScreenColor, true);
	if (RFLOBBYDESKTOP983)
	{
		RFLOBBYDESKTOP983->UpdateAdLabel983(); // 005fa900
	}
}

void COption::SetScreenSize983(int width, int height)
{
	// JP 983 (004949a0): pedido B57; chamado com 800x600 antes de abrir pagina web
	if (CShadowManager::Instance())
	{
		CShadowManager::Instance()->Reset();
	}
	vChangeScreenSize(width, height, m_opt.vScreenColor, false);
	if (RFLOBBYDESKTOP983)
	{
		RFLOBBYDESKTOP983->UpdateAdLabel983(); // 005fa900
	}
}

COption::COption(HWND__* hWnd, WDeviceManager* pDeviceManager,
	WResourceManager* pResourceManager)
{
	m_pDeviceManager = pDeviceManager;
	m_hWnd = hWnd;
	m_pResourceManager = pResourceManager;
	InitVars();
	Init();
}

COption::~COption()
{
	SaveReg();
}


void COption::vChangeGameScreenSize(int width, int height, int color,
	bool bCheck)
{
	if (bCheck)
	{
		vCheckScreenSize(width, height, color);
	}
	else
	{
		m_opt.vScreenWidth = width;
		m_opt.vScreenHeight = height;
		m_opt.vScreenColor = color;
	}
	m_fBarX983 = (float)m_opt.vScreenWidth; // JP 983 (00495060)
	m_fBarY = (float)m_opt.vScreenHeight - 62.0f;
}

void COption::CenterQuitWindow983()
{
	// JP 983 (00493a60)
	if (CQuitWindow::Instance())
	{
		CQuitWindow::Instance()->CenterQuitForm983();
	}
}

bool COption::OnScreenSizeChildWnd983(FrWnd* pWnd, void* param)
{
	// JP 983 (00494bb0): repoe as telas visiveis na posicao do layout; my room, loja e sala de 1 jogador andam
	// 30% do que a largura passa de 800
	if (pWnd->IsVisible() && IS_KINDOF(FrForm, pWnd))
	{
		FrForm* pForm = DYNAMIC_CAST(FrForm, pWnd);
		if (pForm)
		{
			float x = 0.0f;
			const std::string& name = pForm->GetWindowName983();
			if (name == "realmyroom" || name == "shopmain" ||
				name == "gameroom_single")
			{
				x = (g_view->GetWidth() - 800.0) * 0.3f;
			}
			pForm->ResetPosition983(WPoint(x, 0.0f));
		}
	}
	return true;
}

void COption::CheckPendingChanges()
{
	// JP 983 (00497180): tela pelo vChangeGameScreenSize (jogo e lobby), janela de sair e telas reposicionadas,
	// opcoes novas do video (CTask+0x68), aApplyAudioSetting, Reset com o tamanho escolhido, barra de forca e macros
	// com o padrao do registro
	if (!m_bPendingChange)
	{
		return;
	}
	m_bPendingChange = false;

	if (m_opt.vWindowMode != m_newOpt.vWindowMode ||
		m_opt.vScreenWidth != m_newOpt.vScreenWidth ||
		m_opt.vScreenHeight != m_newOpt.vScreenHeight ||
		m_opt.vScreenColor != m_newOpt.vScreenColor ||
		m_opt.vFillMode != m_newOpt.vFillMode)
	{
		if (CBrowser::Instance())
		{
			CBrowser::Instance()->ResetCount();
		}
		if (m_bGameScreen)
		{
			int oldColor = m_opt.vScreenColor;
			int oldWidth = m_opt.vScreenWidth;
			int oldHeight = m_opt.vScreenHeight;
			m_opt.vFillMode = m_newOpt.vFillMode;
			m_opt.vWideMode = m_newOpt.vWideMode;
			m_opt.vWindowMode = m_newOpt.vWindowMode;
			vChangeGameScreenSize(m_newOpt.vScreenWidth, m_newOpt.vScreenHeight,
				m_newOpt.vScreenColor, true);
			vApplyGameScreenSize();
			CenterQuitWindow983();
			if (CSceneManager::Instance())
			{
				CSceneManager::Instance()->UpdateFog(false);
			}
			if (oldWidth != m_opt.vScreenWidth ||
				oldHeight != m_opt.vScreenHeight ||
				oldColor != m_opt.vScreenColor)
			{
				CTask* pTask = NULL;
				MsgObject msg;
				msg.message = 0x271b;
				msg.sender = NULL;
				msg.param1 = 0;
				msg.param2 = 0;
				msg.param3 = 0;
				msg.time = 0;
				if (CTaskManager::Instance())
				{
					pTask = CTaskManager::Instance()->GetCurrentTask();
				}
				pTask->SendGlobalMsg(msg);
				if (g_pFresh)
				{
					g_pFresh->GetManager()->GetDesktop()->ResetRect();
				}
			}
		}
		else
		{
			m_opt.vFillMode = m_newOpt.vFillMode;
			m_opt.vWideMode = m_newOpt.vWideMode;
			m_opt.vWindowMode = m_newOpt.vWindowMode;
			vChangeGameScreenSize(m_newOpt.vScreenWidth, m_newOpt.vScreenHeight,
				m_newOpt.vScreenColor, true);
			vApplyLobbyScreenSize();
		}
		if (CChatMsg::Instance())
		{
			CChatMsg::Instance()->ResetUiPosition();
		}
		g_pFresh->GetManager()->GetDesktop()->EnumerateChildWindow(
			OnScreenSizeChildWnd983, NULL);
	}

	m_opt.vOverall = m_newOpt.vOverall;
	m_opt.vTnLMode = m_newOpt.vTnLMode;
	m_opt.vDDSRes = m_newOpt.vDDSRes;
	m_opt.vUseMipmap = m_newOpt.vUseMipmap;
	m_opt.vMaxMipLvl = m_newOpt.vMaxMipLvl;
	m_opt.vMipCreateFilter = m_newOpt.vMipCreateFilter;
	m_opt.vEffectLevel = m_newOpt.vEffectLevel;
	if (!WisEqual(m_opt.vGamma, m_newOpt.vGamma, g_EPSILON))
	{
		m_opt.vGamma = m_newOpt.vGamma;
	}

	if (m_opt.vProjectionShadow != m_newOpt.vProjectionShadow)
	{
		m_opt.vProjectionShadow = m_newOpt.vProjectionShadow;
		if (CShadowManager::Instance())
		{
			CShadowManager::Instance()->Enable(m_newOpt.vProjectionShadow != 0);
		}
	}
	if (m_opt.vShadowmap != m_newOpt.vShadowmap)
	{
		m_opt.vShadowmap = m_newOpt.vShadowmap;
		if (IS_KINDOF(CGolfTask, AfxGetTask()))
		{
			CGolfDoc* pGolfDoc = CSharedDoc::Instance()->m_pGolfDoc;
			if (pGolfDoc && pGolfDoc->m_pPolySoup)
			{
				pGolfDoc->m_pPolySoup->EnableShadowmap(m_opt.vShadowmap != 0);
			}
		}
	}
	if (m_opt.vReflection != m_newOpt.vReflection)
	{
		m_opt.vReflection = m_newOpt.vReflection;
		if (CSceneManager::Instance())
		{
			CSceneManager::Instance()->SetReflection(m_newOpt.vReflection != 0);
		}
	}
	if (m_opt.vLod != m_newOpt.vLod)
	{
		m_opt.vLod = m_newOpt.vLod;
		if (CSceneManager::Instance())
		{
			CSceneManager::Instance()->SetLOD(m_newOpt.vLod != 0, true);
		}
	}
	m_opt.vNpc = m_newOpt.vNpc;

	CPostEffect983* pFx =
		CTaskManager::Instance()->GetCurrentTask()->GetPostEffect983();
	if (m_opt.viHdr != m_newOpt.viHdr)
	{
		m_opt.viHdr = m_newOpt.viHdr;
		bool bOn = m_newOpt.viHdr == 1;
		if (pFx)
		{
			pFx->m_bHdr = bOn && pFx->m_bEnable;
		}
	}
	if (m_opt.viHdrStarEffect != m_newOpt.viHdrStarEffect)
	{
		m_opt.viHdrStarEffect = m_newOpt.viHdrStarEffect;
		bool bOn = m_newOpt.viHdrStarEffect == 1;
		if (pFx)
		{
			pFx->m_bHdrStar = bOn && pFx->m_bHdr && pFx->m_bEnable;
		}
	}
	if (m_opt.viDof != m_newOpt.viDof)
	{
		m_opt.viDof = m_newOpt.viDof;
		bool bOn = m_newOpt.viDof == 1;
		if (pFx)
		{
			pFx->m_bDof = bOn && pFx->m_bEnable;
		}
	}
	if (m_opt.viAmbientControl != m_newOpt.viAmbientControl)
	{
		m_opt.viAmbientControl = m_newOpt.viAmbientControl;
		bool bOn = m_newOpt.viAmbientControl == 1;
		if (m_pVideoDev)
		{
			m_pVideoDev->SetRimLight(bOn); // slot 0x174 (nome provisorio da vtable)
		}
	}
	if (m_opt.viRimLight != m_newOpt.viRimLight)
	{
		m_opt.viRimLight = m_newOpt.viRimLight;
		bool bOn = m_newOpt.viRimLight == 1;
		if (pFx)
		{
			pFx->m_bRimLight = bOn && pFx->m_bEnable;
		}
	}
	if (m_opt.viFXAA != m_newOpt.viFXAA)
	{
		m_opt.viFXAA = m_newOpt.viFXAA;
		bool bOn = m_newOpt.viFXAA == 1;
		if (pFx)
		{
			pFx->m_bFxaa = bOn && pFx->m_bEnable;
		}
	}
	if (m_opt.vfBright != m_newOpt.vfBright)
	{
		m_opt.vfBright = m_newOpt.vfBright;
		if (pFx)
		{
			pFx->m_bright = (int)m_opt.vfBright;
		}
	}
	if (m_opt.viShaderOnOff != m_newOpt.viShaderOnOff)
	{
		m_opt.viShaderOnOff = m_newOpt.viShaderOnOff;
	}

	m_opt.aOverall = m_newOpt.aOverall;
	if (m_opt.aMssOn != m_newOpt.aMssOn ||
		m_opt.aMssFreq != m_newOpt.aMssFreq ||
		m_opt.aMssBits != m_newOpt.aMssBits ||
		m_opt.aMssChannels != m_newOpt.aMssChannels ||
		m_opt.aMssEnableHwSound != m_newOpt.aMssEnableHwSound ||
		m_opt.aMssBalance != m_newOpt.aMssBalance ||
		m_opt.aMssSpeaker != m_newOpt.aMssSpeaker)
	{
		m_opt.aMssOn = m_newOpt.aMssOn;
		m_opt.aMssFreq = m_newOpt.aMssFreq;
		m_opt.aMssBits = m_newOpt.aMssBits;
		m_opt.aMssChannels = m_newOpt.aMssChannels;
		m_opt.aMssSpeaker = m_newOpt.aMssSpeaker;
		m_opt.aMssEnableHwSound = m_newOpt.aMssEnableHwSound;
		m_opt.aMssBalance = m_newOpt.aMssBalance;
		aApplyAudioSetting();
	}
	if (m_opt.aMssOn)
	{
		if (!WisEqual(m_opt.aSfxVolume, m_newOpt.aSfxVolume, g_EPSILON))
		{
			aSetSfxVolume(m_newOpt.aSfxVolume);
		}
		if (!WisEqual(m_opt.aBGMVolume, m_newOpt.aBGMVolume, g_EPSILON))
		{
			aSetBgmVolume(m_newOpt.aBGMVolume);
		}
	}

	if (!WisEqual(m_opt.gMouseSensitivity, m_newOpt.gMouseSensitivity,
			g_EPSILON))
	{
		gSetMouseSensitivity(m_newOpt.gMouseSensitivity);
	}
	if (m_opt.gWhisper != m_newOpt.gWhisper)
	{
		m_opt.gWhisper = m_newOpt.gWhisper;
		WSendPacket packet((enumClientPacket)0x55);
		packet.Encode1((unsigned char)m_opt.gWhisper);
		packet.Send(TO_GAME);
	}
	m_opt.gInvitation = m_newOpt.gInvitation;
	if (m_opt.gFriendConfirm != m_newOpt.gFriendConfirm)
	{
		m_opt.gFriendConfirm = m_newOpt.gFriendConfirm;
	}
	m_opt.gTransChatWin = m_newOpt.gTransChatWin;
	if (m_opt.gUiEffect != m_newOpt.gUiEffect)
	{
		FrForm::ms_bHasTail = m_newOpt.gUiEffect != 0;
		m_opt.gUiEffect = m_newOpt.gUiEffect;
	}
	if (m_opt.gExtendChatLine != m_newOpt.gExtendChatLine)
	{
		m_opt.gExtendChatLine = m_newOpt.gExtendChatLine;
	}
	if (m_opt.gPowerGauge != m_newOpt.gPowerGauge)
	{
		m_opt.gPowerGauge = m_newOpt.gPowerGauge;
		m_pVideoDev->Command((wVDevMessage)14, m_newOpt.gPowerGauge,
			m_newOpt.gPowerGauge);
		if (m_opt.vWindowMode)
		{
			m_pDeviceManager->ResetVideoDevice(true, m_opt.vScreenWidth,
				m_opt.vScreenHeight, m_opt.vScreenWidth, m_opt.vScreenHeight,
				m_opt.vScreenColor,
				(m_opt.vFillMode != 0 ? 0x8f440000 : 0) + 0xc40000,
				m_opt.vFillMode);
		}
		else
		{
			m_pDeviceManager->ResetVideoDevice(false, m_opt.vScreenWidth,
				m_opt.vScreenHeight, m_opt.vScreenWidth, m_opt.vScreenHeight,
				m_opt.vScreenColor, 0x90080000, 0);
		}
	}
	if (m_opt.gFullyProportionalPowerGaugeSize !=
			m_newOpt.gFullyProportionalPowerGaugeSize ||
		m_opt.gCenteredPowerGauge != m_newOpt.gCenteredPowerGauge)
	{
		m_opt.gFullyProportionalPowerGaugeSize =
			m_newOpt.gFullyProportionalPowerGaugeSize;
		m_opt.gCenteredPowerGauge = m_newOpt.gCenteredPowerGauge;
		UpdatePowerGaugeLayout983();
		CTask* pTask = NULL;
		MsgObject msg;
		msg.message = 0x271b;
		msg.sender = NULL;
		msg.param1 = 0;
		msg.param2 = 0;
		msg.param3 = 0;
		msg.time = 0;
		if (CTaskManager::Instance())
		{
			pTask = CTaskManager::Instance()->GetCurrentTask();
		}
		pTask->SendGlobalMsg(msg);
	}
	m_opt.gCaptureHideGUI = m_newOpt.gCaptureHideGUI;
	m_opt.gCaptureHidePI = m_newOpt.gCaptureHidePI;
	m_opt.gTerrainTooltip = m_newOpt.gTerrainTooltip;
	m_opt.gCutinDisplay = m_newOpt.gCutinDisplay;
	m_opt.gBindMouseCursor = m_newOpt.gBindMouseCursor;

	for (int i = 0; i < 8; ++i)
	{
		std::string macro(m_newOpt.mMacro[i]);
		if (macro.length() == 0 ||
			macro.find_first_not_of(' ') == std::string::npos)
		{
			strcpy(m_opt.mMacro[i], ms_szReg[65 + i][1]); // 983: padrao do registro (645: texto coreano)
		}
		else
		{
			strcpy(m_opt.mMacro[i], m_newOpt.mMacro[i]);
		}
	}
	m_opt.mSaveMacros = m_newOpt.mSaveMacros;
	SaveReg();
}

const char* COption::vGetScrResText(int width, int height, int color,
	bool bWindowed)
{
	const char* pFallback = 0;
	std::list<sDisplayMode>::iterator it;
	it = m_displayModeList.begin();
	if (it != m_displayModeList.end())
	{
		do
		{
			if (width == (*it).width && height == (*it).height)
			{
				if (bWindowed)
				{
					pFallback = (*it).text;
					if (color == (*it).color)
					{
						return pFallback;
					}
				}
				else if (color == (*it).color)
				{
					return (*it).text;
				}
			}
			++it;
		} while (it != m_displayModeList.end());
		if (pFallback)
		{
			return pFallback;
		}
	}
	return "";
}

const char* COption::vGetScrResText(int index)
{
	std::list<sDisplayMode>::iterator it = m_displayModeList.begin();
	int i = 0;
	while (true)
	{
		// HACK: the equivalent negated inequality leaves operator== to the next
		// template-instantiation round, matching the original COMDAT order.
		if (!(it != m_displayModeList.end()))
		{
			return "";
		}
		if (i == index)
		{
			break;
		}
		++it;
		++i;
	}
	return (*it).text;
}

void COption::vGetScrResLvl(const char* text, int* width, int* height,
	int* color)
{
	for (std::list<sDisplayMode>::iterator it = m_displayModeList.begin();
		it != m_displayModeList.end(); ++it)
	{
		// HACK: the original imports strcmpi; _strcmpi extracts a different CRT entry.
		if (strcmpi(text, (*it).text) == 0)
		{
			*width = (*it).width;
			*height = (*it).height;
			*color = (*it).color;
			return;
		}
	}
}

void COption::vCheckScreenSize(int width, int height, int color)
{
	for (std::list<sDisplayMode>::iterator it = m_displayModeList.begin();
		it != m_displayModeList.end(); ++it)
	{
		if (width == (*it).width && height == (*it).height)
		{
			if (m_opt.vWindowMode != 0)
			{
				m_opt.vScreenWidth = width;
				m_opt.vScreenHeight = height;
				m_opt.vScreenColor = (*it).color;
				return;
			}
			if (color == (*it).color)
			{
				m_opt.vScreenColor = color;
				m_opt.vScreenWidth = width;
				m_opt.vScreenHeight = height;
				return;
			}
		}
	}
}

void COption::SendUpdateGameOptionPacket()
{
	sGameOptionInfo info;
	COption::Instance()->ChangeGameOption(m_opt, info);
	WSendPacket packet((enumClientPacket)0x69);
	packet.EncodeBuffer(&info, sizeof(info));
	packet.Send(TO_GAME);
}

void COption::Init()
{
	// JP 983 (004951c0): chave do registro pelo canal (Pangya_Jp, Hangame "@hg", Atgames "@gr", Lievo "@lv" no id da
	// linha de comando, so com o filtro de idioma ligado); tabela de 74; campos novos; "-safe" com a lista do 983
	char path[256];
	char buf[60] = "";
	char macros[8][260] = { 0 };

	path[0] = 0;
	if (!CSharedDoc::Instance()->m_bChannelLogin)
	{
		sprintf(path, "%s\\%s", "HKEY_LOCAL_MACHINE", "Software\\Ntreev\\Pangya_Jp");
	}
	else
	{
		// o 983 baixa o texto do proprio documento (strlwr no lugar)
		std::string id(_strlwr((char*)CSharedDoc::Instance()->m_commandLineID.c_str()));
		const char* key;
		if (id.find("@hg", 0, 3) != std::string::npos)
		{
			key = "Software\\Ntreev\\PangYa_Hangame";
		}
		else if (id.find("@gr", 0, 3) != std::string::npos)
		{
			key = "Software\\Ntreev\\PangYa_Atgames";
		}
		else if (id.find("@lv", 0, 3) != std::string::npos)
		{
			key = "Software\\Ntreev\\PangYa_Lievo";
		}
		else
		{
			key = "Software\\Ntreev\\Pangya_Jp";
		}
		sprintf(path, "%s\\%s", "HKEY_LOCAL_MACHINE", key);
	}
	m_reg.SetLocation(path);

	for (int i = 0; i < 74; ++i)
	{
		if (strlen(ms_szReg[i][0]) != 0 && !ReadReg(i, buf, 60))
		{
			WriteReg(i, K2L_Compatibility(ms_szReg[i][1]));
		}
	}

	ScanReg_S(0, PY_PUBLIC_VERSION);
	ScanReg_I(1, &m_opt.vWindowMode);
	ScanReg_I(2, &m_opt.vFillMode);
	ScanReg_I(3, &m_opt.vWideMode);
	ScanReg_I(4, &m_opt.vOverall);
	ScanReg_I(5, &m_opt.vScreenWidth);
	ScanReg_I(6, &m_opt.vScreenHeight);
	ScanReg_I(7, &m_opt.vScreenColor);
	ScanReg_I(8, &m_opt.vTnLMode);
	m_opt.vTnLMode = 1;
	ScanReg_I(9, &m_opt.vDDSRes);
	ScanReg_I(10, &m_opt.vUseMipmap);
	ScanReg_I(11, &m_opt.vMaxMipLvl);
	ScanReg_I(12, &m_opt.vMipCreateFilter);
	ScanReg_I(13, &m_opt.vEffectLevel);
	ScanReg_I(14, &m_opt.vProjectionShadow);
	ScanReg_I(15, &m_opt.vShadowmap);
	ScanReg_I(16, &m_opt.vReflection);
	ScanReg_I(17, &m_opt.vLod);
	ScanReg_I(18, &m_opt.vNpc);
	ScanReg_I(19, &m_opt.vCoordX);
	ScanReg_I(20, &m_opt.vCoordY);
	ScanReg_F(21, &m_opt.vGamma);
	ScanReg_I(22, &m_opt.viHdr);
	ScanReg_I(23, &m_opt.viHdrStarEffect);
	ScanReg_I(24, &m_opt.viDof);
	ScanReg_I(26, &m_opt.viAmbientControl);
	ScanReg_I(25, &m_opt.viRimLight);
	ScanReg_I(27, &m_opt.viFXAA);
	ScanReg_F(28, &m_opt.vfBright);
	ScanReg_I(29, &m_opt.viShaderOnOff);
	ScanReg_I(30, &m_opt.aMssOn);
	ScanReg_I(31, &m_opt.aOverall);
	ScanReg_I(32, &m_opt.aMssFreq);
	ScanReg_I(33, &m_opt.aMssBits);
	ScanReg_I(34, &m_opt.aMssChannels);
	ScanReg_I(35, &m_opt.aMssEnableHwSound);
	ScanReg_I(36, &m_opt.aMssBalance);
	ScanReg_I(37, &m_opt.aMssSpeaker);
	ScanReg_F(38, &m_opt.aSfxVolume);
	ScanReg_F(39, &m_opt.aBGMVolume);
	ScanReg_F(40, &m_opt.gMouseSensitivity);
	ScanReg_I(41, &m_opt.gWhisper);
	ScanReg_I(42, &m_opt.gInvitation);
	ScanReg_I(43, &m_opt.gFriendConfirm);
	ScanReg_I(44, &m_opt.gTransChatWin);
	ScanReg_I(45, &m_opt.gUiEffect);
	ScanReg_S(46, m_opt.gLastLoginID);
	ScanReg_I(47, &m_opt.gIdentity);
	ScanReg_I(48, &m_opt.gExtendChatLine);
	ScanReg_I(49, &m_opt.gUserSort);
	ScanReg_I(50, &m_opt.gPPL_Enable);
	ScanReg_I(51, &m_opt.gPPL_Size);
	ScanReg_I(52, &m_opt.gPowerGauge);
	ScanReg_I(53, &m_opt.gFullyProportionalPowerGaugeSize);
	ScanReg_I(54, &m_opt.gCenteredPowerGauge);
	ScanReg_I(55, &m_opt.gCaptureLogo);
	ScanReg_I(56, &m_opt.gRestore);
	ScanReg_I(57, &m_opt.gChatWnd);
	ScanReg_I(58, &m_opt.gUnderTabBtn);
	ScanReg_I(59, &m_opt.gAvatarNewbie);
	ScanReg_I(60, &m_opt.gCaptureHideGUI);
	ScanReg_I(61, &m_opt.gCaptureHidePI);
	ScanReg_I(62, &m_opt.gTerrainTooltip);
	ScanReg_I(63, &m_opt.gCutinDisplay);
	ScanReg_I(64, &m_opt.gBindMouseCursor);
	for (int i = 0; i < 8; ++i)
	{
		ScanReg_S(65 + i, macros[i]);
	}
	ScanReg_I(73, &m_opt.mSaveMacros);
	for (int i = 0; i < 8; ++i)
	{
		wnsprintfA(m_opt.mMacro[i], 64, "%s", macros[i]);
	}

	m_opt.vWindowMode = m_opt.vWindowMode != 0;
	m_opt.vFillMode = m_opt.vFillMode != 0;
	m_opt.vWideMode = m_opt.vWideMode != 0;
	m_opt.vOverall = Between(0, m_opt.vOverall, 2);
	vChangeGameScreenSize(m_opt.vScreenWidth, m_opt.vScreenHeight,
		m_opt.vScreenColor, false);
	m_opt.vTnLMode = Between(0, m_opt.vTnLMode, 2);
	m_opt.vDDSRes = m_opt.vDDSRes < 1 ? 0 : m_opt.vDDSRes;
	m_opt.vUseMipmap = m_opt.vUseMipmap != 0;
	m_opt.vMaxMipLvl = m_opt.vMaxMipLvl < 1 ? 0 : m_opt.vMaxMipLvl;
	m_opt.vMipCreateFilter = Between(0, m_opt.vMipCreateFilter, 5);
	m_opt.vEffectLevel = Between(0, m_opt.vEffectLevel, 2);
	m_opt.vProjectionShadow = m_opt.vProjectionShadow != 0;
	m_opt.vShadowmap = m_opt.vShadowmap != 0;
	m_opt.vReflection = m_opt.vReflection != 0;
	m_opt.vLod = m_opt.vLod != 0;
	m_opt.vNpc = m_opt.vNpc != 0;
	m_opt.vCoordX = Between(0, m_opt.vCoordX,
		GetSystemMetrics(SM_CXSCREEN) - m_opt.vScreenWidth);
	// o 983 limita o Y com o valor do X (00495aae le +0x120): mantido igual
	m_opt.vCoordY = Between(0, m_opt.vCoordX,
		GetSystemMetrics(SM_CYSCREEN) - m_opt.vScreenHeight);
	m_opt.vGamma = Between(0.5f, m_opt.vGamma, 3.0f);
	m_opt.viHdr = m_opt.viHdr != 0;
	m_opt.viHdrStarEffect = m_opt.viHdrStarEffect != 0;
	m_opt.viDof = m_opt.viDof != 0;
	m_opt.viAmbientControl = m_opt.viAmbientControl != 0;
	m_opt.viRimLight = m_opt.viRimLight != 0;
	m_opt.viFXAA = m_opt.viFXAA != 0;
	m_opt.vfBright = Between(0.0f, m_opt.vfBright, 4.0f);
	m_nTnLMode = m_opt.vTnLMode;
	m_opt.viShaderOnOff = m_opt.viShaderOnOff != 0;
	m_opt.aMssOn = m_opt.aMssOn != 0;
	m_opt.aOverall = Between(0, m_opt.aOverall, 2);
	m_opt.aMssFreq = m_opt.aMssFreq == 44100 ? 44100 : 1;
	m_opt.aMssBits = m_opt.aMssBits == 16 ? 16 : 8;
	m_opt.aMssChannels = m_opt.aMssChannels == 2 ? 2 : 1;
	m_opt.aMssEnableHwSound = m_opt.aMssEnableHwSound != 0;
	m_opt.aMssBalance = Between(0, m_opt.aMssBalance, 256);
	m_opt.aMssSpeaker = Between(0, m_opt.aMssSpeaker, 5);
	m_opt.aSfxVolume = Between(0.0f, m_opt.aSfxVolume, 1.0f);
	m_opt.aBGMVolume = Between(0.0f, m_opt.aBGMVolume, 1.0f);
	m_opt.gMouseSensitivity = Between(0.5f, m_opt.gMouseSensitivity, 2.0f);
	m_opt.gLastLoginID[22] = 0;
	m_opt.gWhisper = m_opt.gWhisper != 0;
	m_opt.gInvitation = m_opt.gInvitation != 0;
	m_opt.gTransChatWin = m_opt.gTransChatWin != 0;
	m_opt.gUiEffect = m_opt.gUiEffect != 0;
	m_opt.gExtendChatLine = m_opt.gExtendChatLine != 0;
	m_opt.gPowerGauge = m_opt.gPowerGauge != 0;
	m_opt.gFullyProportionalPowerGaugeSize =
		m_opt.gFullyProportionalPowerGaugeSize != 0;
	m_opt.gCenteredPowerGauge = m_opt.gCenteredPowerGauge != 0;
	m_opt.gCaptureLogo = m_opt.gCaptureLogo != 0;
	m_opt.gCutinDisplay = m_opt.gCutinDisplay != 0;
	m_opt.gBindMouseCursor = m_opt.gBindMouseCursor != 0;

	for (int i = 0; i < 8; ++i)
	{
		std::string macro(m_opt.mMacro[i]);
		if (macro.length() == 0 ||
			macro.find_first_not_of(' ') == std::string::npos)
		{
			sprintf(m_opt.mMacro[i],
				K2L_Compatibility("\xc3\xa4\xc6\xc3 \xb8\xc5\xc5\xa9\xb7\xce\xb8\xa6 \xbc\xb3\xc1\xa4\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."));
		}
		LimitText(m_opt.mMacro[i], 64);
	}
	m_opt.mSaveMacros = m_opt.mSaveMacros != 0;

	WRegistry mouseReg;
	mouseReg.SetLocation("HKEY_CURRENT_USER\\Control Panel\\Mouse");
	char swapBuf[128];
	if (mouseReg.Read("SwapMouseButtons", swapBuf, 128))
	{
		int swap;
		sscanf(swapBuf, "%d", &swap);
		m_bSwapMouseButton = swap != 0;
	}
	FrForm::ms_bHasTail = m_opt.gUiEffect != 0;

	char cmdLine[260];
	strcpy(cmdLine, GetCommandLineA());
	strlwr(cmdLine);
	if (strstr(cmdLine, "-safe"))
	{
		m_opt.vWindowMode = 1;
		m_opt.vWideMode = 0;
		m_opt.vOverall = 2;
		vChangeGameScreenSize(800, 600, 16, false);
		m_opt.vDDSRes = 1;
		m_opt.vMaxMipLvl = 1;
		m_opt.vTnLMode = 0;
		m_opt.vUseMipmap = 0;
		m_opt.vMipCreateFilter = 2;
		m_opt.vEffectLevel = 2;
		m_opt.vProjectionShadow = 0;
		m_opt.vShadowmap = 0;
		m_opt.vReflection = 0;
		m_opt.vLod = 0;
		m_opt.vNpc = 0;
		m_opt.vCoordX = (GetSystemMetrics(SM_CXSCREEN) - 800) / 2;
		m_opt.vCoordY = (GetSystemMetrics(SM_CYSCREEN) - 600) / 2;
		m_opt.viHdr = 0;
		m_opt.viHdrStarEffect = 0;
		m_opt.viDof = 0;
		m_opt.viAmbientControl = 0;
		m_opt.viRimLight = 0;
		m_opt.viFXAA = 0;
		m_opt.viShaderOnOff = 0;
		m_nTnLMode = m_opt.vTnLMode;
	}
	strcpy(m_opt.mPCBangMascotMsg, "PangYa!");
}

bool COption::vChangeScreenSize(int width, int height, int color, bool bLobby)
{
	// JP 983 (00494120): 4o argumento novo (lobby), guarda o ultimo pedido em +0x7fc..+0x804 e tambem o compara;
	// minimo sem olhar vUseMinScreen; ResetVideoDevice de 8 argumentos; depois avisa o desktop do Fresh, o
	// ponteiro da UI nova e a tarefa do lobby (0x5a). Tipos 15/16 = modos sem barra de forca (645: 11/12).
	int cur;
	bool bDiffCur = false;
	m_pVideoDev->Command((wVDevMessage)13, (int)&cur, 0);
	unsigned char gameType = CSharedDoc::Instance()->m_golfGame.gameType;
	if (gameType == 15 || gameType == 16)
	{
		if (cur != 0)
		{
			bDiffCur = true;
		}
	}
	else if (cur != m_opt.gPowerGauge)
	{
		bDiffCur = true;
	}
	if (m_pVideoDev->IsWindowed() == (bool)(m_opt.vWindowMode != 0) &&
		m_pVideoDev->GetWidth() == width &&
		m_pVideoDev->GetHeight() == height &&
		m_lastScreenWidth983 == width && m_lastScreenHeight983 == height &&
		m_bLastScreenLobby983 == bLobby &&
		m_pVideoDev->IsFillScreenMode() == (bool)(m_opt.vFillMode != 0) &&
		(m_opt.vWindowMode != 0 || m_pVideoDev->GetBackBufferBpp() == color) &&
		!bDiffCur)
	{
		return true;
	}
	float clipNear = m_pGlobalView->GetClipNearValue();
	float clipFar = m_pGlobalView->GetClipFarValue();
	float fov = m_pGlobalView->GetFOV();
	m_lastScreenWidth983 = width;
	m_lastScreenHeight983 = height;
	m_bLastScreenLobby983 = bLobby;
	if (width < m_opt.vMinScreenWidth || height < m_opt.vMinScreenHeight)
	{
		height = m_opt.vMinScreenHeight;
		width = m_opt.vMinScreenWidth;
		m_lastScreenWidth983 = width;
		m_lastScreenHeight983 = height;
	}
	CProjectG::Instance()->SetWindowed(m_opt.vWindowMode != 0);
	int param;
	gameType = CSharedDoc::Instance()->m_golfGame.gameType;
	if (gameType == 15 || gameType == 16)
	{
		param = 0;
	}
	else
	{
		param = m_opt.gPowerGauge;
	}
	m_pVideoDev->Command((wVDevMessage)14, param, m_opt.vWindowMode);
	int fillMode;
	long style;
	if (m_opt.vWindowMode != 0)
	{
		fillMode = m_opt.vFillMode;
		style = (fillMode != 0 ? 0x8f440000 : 0) + 0xc40000;
	}
	else
	{
		fillMode = 0;
		style = 0x90080000;
	}
	bool bRet = m_pDeviceManager->ResetVideoDevice(m_opt.vWindowMode != 0,
		width, height, m_lastScreenWidth983, m_lastScreenHeight983, color, style,
		fillMode);
	m_pGlobalView->SetViewport((float)m_pVideoDev->GetWidth(),
		(float)m_pVideoDev->GetHeight());
	m_pGlobalView->SetClip(clipNear, clipFar, false);
	m_pGlobalView->SetFOV(fov);
	m_pGlobalView->UpdateCamera();
	g_pFresh->GetManager()->GetDesktop()->Init(m_pGlobalView->GetWidth(),
		m_pGlobalView->GetHeight());
	if (RFSceneManager::Instance())
	{
		RFSceneManager::Instance()->SetResolution983(
			m_pGlobalView->GetWidth(), m_pGlobalView->GetHeight()); // 006035b0
	}
	if (CTaskManager::Instance())
	{
		CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x5a, 0, 0, 0, 0);
	}
	return bRet;
}

int COption::gGetRestore()
{
	if (m_opt.gRestore)
	{
		int v = m_opt.gRestore;
		unsigned char check =
			((((char)(v >> 30) * 4 | (unsigned char)(v >> 22) & 3) * 4 |
				 (unsigned char)(v >> 14) & 3) *
				4) |
			(unsigned char)(v >> 6) & 3;
		const char* id = CSharedDoc::Instance()->m_myInfo.info.sID; // 983: +0x7a2
		unsigned int len = strlen(id);
		unsigned char sum = 0;
		for (unsigned int i = 0; i < len; ++i)
		{
			sum += id[i];
		}
		if (check != sum)
		{
			gResetRestore();
			return 0;
		}
		return (m_opt.gRestore >> 24 & 0x3fU) << 25 | m_opt.gRestore & 0x3f3f3f;
	}
	return 0;
}

void COption::gSetRestore()
{
	// JP 983 (004944e0): tambem fora no tipo 7; indice 56 do registro
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		unsigned char gameType = CSharedDoc::Instance()->m_golfGame.gameType;
		if (gameType != 10 && gameType != 7)
		{
			const char* id = CSharedDoc::Instance()->m_myInfo.info.sID;
			char sum = 0;
			for (unsigned int i = 0; i < strlen(id); ++i)
			{
				sum += id[i];
			}
			SYSTEMTIME st;
			GetLocalTime(&st);
			unsigned int s = sum;
			unsigned int v = (s & 0xffffffc0) * 2 | st.wMinute & 0xfffffffe;
			v = (v << 5 | s & 0x30) * 4 | st.wHour;
			v = (v << 4 | s & 0xc) << 4 | st.wDay;
			v = (v * 4 | s & 3) << 6 | st.wMonth;
			m_opt.gRestore = v;
			WriteReg_I(56, m_opt.gRestore);
		}
	}
}

// A733: crus 0x1xxxx do CSharedDoc -> membros (VC10 = 983)
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeType) == 0x10490 && offsetof(CSharedDoc, m_golfGame.shotTimeLimit) == 0x1069c &&
	offsetof(CSharedDoc, m_golfGame.gameTimeLimit) == 0x106a0);

void COption::dApplyDebugSetting()
{
	if (0)
		((COption*)0)->COption::gSetRestore();
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		SetCurMap(m_nMap);
		CSharedDoc::Instance()->m_holeType = 0;
		CSharedDoc::Instance()->m_golfGame.gameTimeLimit = 0;
		switch (m_nGameTime)
		{
		case 0:
			CSharedDoc::Instance()->m_golfGame.shotTimeLimit = 0;
			break;
		case 1:
			CSharedDoc::Instance()->m_golfGame.shotTimeLimit = 40000;
			break;
		default:
			CSharedDoc::Instance()->m_golfGame.shotTimeLimit =
				m_nGameTime * 1000;
			break;
		}
	}
}

void COption::SetVideoDevice(WVideoDev* pVideoDev)
{
	// JP 983 (00496770): comando 0x21 (645: 0x20), modo da area de trabalho, tamanho escolhido com proporcao <= 0,75
	// (senao o maior modo mais baixo da lista, ou 640x480), Reset com o tamanho escolhido e o video novo so com Vs/Ps/Mrt
	m_pVideoDev = pVideoDev;
	int value;
	pVideoDev->Command((wVDevMessage)0x21, 0, (int)&value);
	sOption::sbHwTnlSupport = value != 0;
	sOption::sbRtSupport = m_pVideoDev->SupportRenderTargetFormat();
	sOption::sbVsSupport = m_pVideoDev->IsSupportVs();
	sOption::sbPsSupport = m_pVideoDev->IsSupportPs();
	sOption::sbMrtSupport = m_pVideoDev->IsSupportMrt();
	sOption::sbClipPlaneSupport = m_pVideoDev->IsSupportClipPlane();
	memset(&m_desktopMode983, 0, sizeof(m_desktopMode983));
	if (!m_pVideoDev->GetWindowDisplayMode(m_desktopMode983.width,
			m_desktopMode983.height, m_desktopMode983.color))
	{
		m_desktopMode983.width = 800;
		m_desktopMode983.height = 600;
		m_desktopMode983.color = 32;
	}
	if (m_opt.vScreenWidth < 640 || m_opt.vScreenHeight < 480)
	{
		m_opt.vWideMode = 0;
		m_opt.vScreenWidth = 640;
		m_opt.vScreenHeight = 480;
	}
	else if (m_opt.vScreenHeight / (float)(m_opt.vScreenWidth + 1.0) > 0.75)
	{
		std::list<sDisplayMode> modes;
		int bestHeight = 0;
		int bestWidth = 0;
		CheckDisplayModeList(m_pVideoDev, m_opt.vWindowMode != 0,
			m_opt.vWideMode != 0, modes);
		for (std::list<sDisplayMode>::iterator it = modes.begin();
			it != modes.end(); ++it)
		{
			if ((m_opt.vWindowMode != 0 || it->color == m_opt.vScreenColor) &&
				it->height < m_opt.vScreenHeight)
			{
				bestWidth = it->width;
				bestHeight = it->height;
			}
		}
		if (bestWidth == 0)
		{
			m_opt.vWideMode = 0;
			m_opt.vScreenWidth = 640;
			m_opt.vScreenHeight = 480;
		}
		else
		{
			m_opt.vScreenWidth = bestWidth;
			m_opt.vScreenHeight = bestHeight;
		}
	}
	m_pVideoDev->Command((wVDevMessage)0x21, 1, (int)&value);
	m_opt.vTnLMode = Between(0, value, 2);
	m_nTnLMode = m_opt.vTnLMode;
	m_pVideoDev->Command((wVDevMessage)0xe, m_opt.gPowerGauge,
		m_opt.vWindowMode);
	m_pDeviceManager->ResetVideoDevice(true, m_opt.vScreenWidth,
		m_opt.vScreenHeight, m_opt.vScreenWidth, m_opt.vScreenHeight,
		m_opt.vScreenColor, 0xc40000, 0);
	if (!sOption::IsSupportShadow())
	{
		m_opt.vProjectionShadow = 0;
	}
	if (!sOption::IsSupportReflection())
	{
		m_opt.vReflection = 0;
	}
	if (!sOption::IsSupportLod())
	{
		m_opt.vLod = 0;
	}
	if (!sOption::sbMrtSupport || !sOption::sbPsSupport ||
		!sOption::sbVsSupport)
	{
		m_opt.viHdr = 0;
		m_opt.viHdrStarEffect = 0;
		m_opt.viDof = 0;
		m_opt.viAmbientControl = 0;
		m_opt.viRimLight = 0;
		m_opt.viFXAA = 0;
		m_opt.viShaderOnOff = 0;
	}
	m_opt.vSetOverallByVidOptions();
	CheckDisplayModeList(pVideoDev, m_opt.vWindowMode != 0,
		m_opt.vWideMode != 0);
	vApplyMipmap();
}

void COption::CheckDisplayModeList(WVideoDev* pVideoDev, bool bWindowed,
	bool bWide)
{
	// JP 983 (004965a0): a lista do COption pela versao de 4 argumentos
	if (pVideoDev)
	{
		CheckDisplayModeList(pVideoDev, bWindowed, bWide, m_displayModeList);
	}
}

void COption::CheckDisplayModeList(WVideoDev* pVideoDev, bool bWindowed,
	bool bWide, std::list<sDisplayMode>& modes)
{
	// JP 983 (00496180): so os modos que cabem na area de trabalho e nao ficam abaixo do minimo; na lista normal o 983
	// compara o minimo com a linha da tabela larga de mesmo indice (erro do original, mantido)
	modes.clear();
	if (bWide)
	{
		for (unsigned int i = 0; i < 34; ++i)
		{
			if (s_wideModes[i][0] <= m_desktopMode983.width &&
				s_wideModes[i][1] <= m_desktopMode983.height &&
				s_wideModes[i][0] >= m_opt.vMinScreenWidth &&
				s_wideModes[i][1] >= m_opt.vMinScreenHeight)
			{
				sDisplayMode mode;
				mode.width = s_wideModes[i][0];
				mode.height = s_wideModes[i][1];
				mode.color = s_wideModes[i][2];
				sprintf(mode.text, "%dx%dx%d", mode.width, mode.height,
					mode.color);
				if (pVideoDev->IsSupportedDisplayMode(bWindowed, mode.width,
						mode.height, mode.color))
				{
					modes.push_back(mode);
				}
			}
		}
		return;
	}
	for (unsigned int i = 0; i < 10; ++i)
	{
		if (s_normalModes[i][0] <= m_desktopMode983.width &&
			s_normalModes[i][1] <= m_desktopMode983.height &&
			s_wideModes[i][0] >= m_opt.vMinScreenWidth &&
			s_wideModes[i][1] >= m_opt.vMinScreenHeight)
		{
			sDisplayMode mode;
			mode.width = s_normalModes[i][0];
			mode.height = s_normalModes[i][1];
			mode.color = s_normalModes[i][2];
			sprintf(mode.text, "%dx%dx%d", mode.width, mode.height, mode.color);
			if (pVideoDev->IsSupportedDisplayMode(bWindowed, mode.width,
					mode.height, mode.color))
			{
				modes.push_back(mode);
			}
		}
	}
}

extern WView* g_view; // 00f02458

void COption::UpdatePowerGaugeLayout983()
{
	// JP 983 (00496b20): escala sem arredondar pela opcao "proporcional", posicao x da barra (centrada: meio da tela - 180
	// na escala da altura; senao 140 scltype 4), largura do polegar arredondada e a area capturada recortada pela tela
	CUiFrameSetManager::Instance()->SetA4983(
		m_opt.gFullyProportionalPowerGaugeSize != 0, m_pGlobalView);
	float height = m_pGlobalView->GetHeight();
	float width = m_pGlobalView->GetWidth();
	CUiFrameSetManager::Size scl(width / 640.0, height / 480.0);
	CUiFrameSetManager::NamedDim* pDim =
		CUiFrameSetManager::Instance()->FindNamedDim("d_default_pangya_x");
	if (pDim)
	{
		if (m_opt.gCenteredPowerGauge)
		{
			float viewWidth = m_pGlobalView->GetWidth();
			pDim->Set(ScaleByViewHeight983(m_pGlobalView, -180.0f) +
				viewWidth * 0.5);
		}
		else
		{
			pDim->Set(140.0f, 4);
		}
	}
	float thumb = (float)(int)(ScaleByViewHeight983(g_view, 8.0f) + 0.5);
	CUiFrameSetManager::NamedDim* pThumb =
		CUiFrameSetManager::Instance()->FindNamedDim("d_thumb_rounded_w");
	CUiFrameSetManager::NamedDim* pHalf =
		CUiFrameSetManager::Instance()->FindNamedDim("d_thumb_half_rounded_w");
	if (pThumb && pHalf)
	{
		pThumb->Set(thumb);
		pHalf->Set(thumb * 0.5);
	}
	CUiFrameSetManager::NamedRect* pRect =
		CUiFrameSetManager::Instance()->FindNamedRect(
			"r_to_be_updated_when_captured");
	if (pRect)
	{
		float viewHeight = m_pGlobalView->GetHeight();
		float viewWidth = m_pGlobalView->GetWidth();
		WRect rect;
		WRect screen(0.0f, 0.0f, viewWidth, viewHeight);
		pRect->Calc(scl, rect);
		WRect cross;
		WOverlay::CrossRect(rect, screen, &cross);
		m_pVideoDev->Command((wVDevMessage)0xf, (int)&cross, 0);
	}
}
