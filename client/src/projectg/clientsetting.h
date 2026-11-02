#pragma once

#include <string.h>
#include <list>
#include "wreg.h"
#include "miles.h"

class CSoundManager;
class CInputManager;
struct sGameOptionInfo;

struct sOption
{
	sOption();
	~sOption();

	static bool IsSupportShadow();
	static bool IsSupportReflection();
	static bool IsSupportLod();

	void vSetOverallByVidOptions();
	void vResetByOverall(const char* text);

	const char* GetThreeLvlText(int lvl);
	const char* GetTwoLvlText(int lvl);
	int vGetLvl3FromText(const char* text);

	const char* vGetOverallText(int lvl);
	int vGetOverallLvl(const char* text);
	const char* vGetTnlText(int lvl);
	int vGetTnlLvl(const char* text);
	const char* vGetTexResText(int lvl);
	int vGetTexResLvl(const char* text);
	const char* vGetParticleText(int lvl);
	int vGetParticleLvl(const char* text);

	const char* aGetOverallText(int lvl);
	int aGetOverallLvl(const char* text);
	const char* aGetSpeakerText(int lvl);
	int aGetSpeakerLvl(const char* text);

	static bool sbHwTnlSupport;
	static bool sbRtSupport;
	static bool sbVsSupport;
	static bool sbPsSupport;
	static bool sbMrtSupport;
	static bool sbClipPlaneSupport;
	// JP 983: video shader options need MRT + PS + VS (inlined 7x in vResetByOverall)
	static bool IsSupportVidShader983() { return sbMrtSupport && sbPsSupport && sbVsSupport; }

	int vWindowMode;
	int vFillMode;
	int vWideMode;
	int vScreenWidth;
	int vScreenHeight;
	int vUseMinScreen;
	int vMinScreenWidth;
	int vMinScreenHeight;
	int vScreenColor;
	int vOverall;
	int vTnLMode;
	int vDDSRes;
	int vUseMipmap;
	int vMaxMipLvl;
	int vMipCreateFilter;
	int vEffectLevel;
	int vProjectionShadow;
	int vShadowmap;
	int vReflection;
	int vLod;
	int vNpc;
	int vCoordX;
	int vCoordY;
	float vGamma;
	// JP 983: 8 campos novos do video (registro "Option\\viHdr"... indices 22-29; Init 004951c0)
	int viHdr;
	int viHdrStarEffect;
	int viDof;
	int viAmbientControl;
	int viRimLight;
	int viFXAA;
	float vfBright;
	int viShaderOnOff;

	int aMssOn;
	int aOverall;
	int aMssFreq;
	int aMssBits;
	int aMssChannels;
	int aMssEnableHwSound;
	int aMssBalance;
	int aMssSpeaker;
	float aSfxVolume;
	float aBGMVolume;

	float gMouseSensitivity;
	int gWhisper;
	int gInvitation;
	int gFriendConfirm;
	int gTransChatWin;
	int gUiEffect;
	char gLastLoginID[23];
	int gIdentity;
	int gExtendChatLine;
	int gUserSort;
	int gPPL_Enable;
	int gPPL_Size;
	int gPowerGauge;
	int gFullyProportionalPowerGaugeSize; // JP 983 (indice 53)
	int gCenteredPowerGauge; // JP 983 (indice 54)
	int gCaptureLogo;
	int gRestore;
	int gChatWnd;
	int gUnderTabBtn;
	int gAvatarNewbie;
	int gCaptureHideGUI;
	int gCaptureHidePI;
	int gTerrainTooltip;
	int gCutinDisplay;
	int gBindMouseCursor; // JP 983 (indice 64)

	char mMacro[8][64];
	int mSaveMacros;
	char mPCBangMascotMsg[30];
};

class COption : public WSingleton<COption>
{
	friend struct COptionLayout983;

public:
#ifdef REBANG_PCH_POOL // HACK: only in the pool PCH, other units keep their IL and name count
	using WSingleton<COption>::m_pInstance;
#endif

	struct sDisplayMode
	{
		int width;
		int height;
		int color;
		char text[16];
	};

	COption(HWND__* hWnd, WDeviceManager* pDeviceManager,
		WResourceManager* pResourceManager);
	virtual ~COption();

	void SetVideoDevice(WVideoDev* pVideoDev);
	void SetGlobalView(WView* pView);
	void SetAudioManager(CSoundManager* pAudioManager);
	void SetInputManager(CInputManager* pInputManager);

	const sOption* GetOption();
	void ApplyChange(sOption& option, bool bGameScreen);
	void CheckPendingChanges();
	void ChangeGameOption(sOption& option, sGameOptionInfo& info);
	void ChangeGameOption(sGameOptionInfo& info, sOption& option);
	void SendUpdateGameOptionPacket();
	void CheckDebugRegValid();
	void CheckDisplayModeList(WVideoDev* pVideoDev, bool bWindowed, bool bWide); // JP 983: 004965a0
	void CheckDisplayModeList(WVideoDev* pVideoDev, bool bWindowed, bool bWide,
		std::list<sDisplayMode>& modes); // JP 983: 00496180

	// 645 only: the 983 has no such function (console case 0x39 is not a toggle there); kept inline so it is not emitted standalone.
	void vToggleFullscreen()
	{
		m_newOpt = m_opt;
		m_newOpt.vWindowMode = m_opt.vWindowMode == 0;
		m_bPendingChange = true;
		m_bGameScreen = false;
	}
	void vApplyGameScreenSize();
	void vApplyLobbyScreenSize();
	void SetScreenSize983(int width, int height); // JP 983: 004949a0 (antes de abrir pagina web)
	void vChangeGameScreenSize(int width, int height, int color, bool bCheck);
	void vCheckScreenSize(int width, int height, int color);
	void vSetMinimumScreenSize(int bUse, int width, int height);
	int vGetScrResNum();
	const char* vGetScrResText(int index);
	const char* vGetScrResText(int width, int height, int color,
		bool bWindowed);
	void vGetScrResLvl(const char* text, int* width, int* height, int* color);
	void vSetMipmap(int ddsRes, bool bUseMipmap, int maxMipLvl,
		int mipCreateFilter);
	bool vIsWindowed();
	bool vIsWideMode();

	int vGetTnLMode() const { return m_nTnLMode; }
	int vGetParticleLevel() const { return m_opt.vEffectLevel; }
	sOption& Opt983() { return m_opt; } // B1466: m_opt cru (+0xcc VC10) anda -4 no cl 14.44 (std::string do BaseObject)
	void vSetParticleLevel(int level);
	void vEnableShadow(bool bEnable);
	void vEnableShadowmap(bool bEnable);
	bool vIsShadowEnabled();
	bool vIsShadowmapEnabled();
	bool vIsReflectionEnabled();
	bool vIsLodEnabled();
	bool vIsNpcEnabled();

	bool aIsMssEnabled();
	bool aIsMssHwSoundEnabled();
	void aApplySfxVolume(float volume);
	void aApplyBgmVolume(float volume);

	int aGetMssFrequency() const { return m_opt.aMssFreq; }
	int aGetMssBits() const { return m_opt.aMssBits; }
	int aGetMssChannels() const { return m_opt.aMssChannels; }
	WMilesSoundSystem::w_speaker_type aGetMssSpeaker();
	int aGetMssBalance() const { return m_opt.aMssBalance; }
	void aSetSpeakerType(int type);
	float aGetSfxVolume() const { return m_opt.aSfxVolume; }
	float aGetBgmVolume() const { return m_opt.aBGMVolume; }
	void aSetSfxVolume(float volume);
	void aSetBgmVolume(float volume);

	float gGetMouseSensitivity() const { return m_opt.gMouseSensitivity; }
	void gSetMouseSensitivity(float sensitivity);
	bool gIsWhisperAllowed();
	bool gIsInvitationAllowed();
	bool gIsFriendConfirmNeeded();
	void gSetLastLoginID(const char* id)
	{
		strncpy(m_opt.gLastLoginID, id, 22);
	}
	const char* gGetLastLoginID() const { return m_opt.gLastLoginID; }
	bool gIsChatWinTransparent();
	bool gIsChatLineEntended();
	bool gIsPPLEnable();
	void gSetPPLEnable(bool bEnable);
	void gSetPPLSize(int size);

	int gGetPPLSize() { return m_opt.gPPL_Size; }
	int gGetRestore();
	void gSetRestore();
	void gResetRestore();
	int gGetChatWndUp() { return m_opt.gChatWnd; }
	void gSetChatWndUp(int up) { m_opt.gChatWnd = up; }
	int gGetUnderExtBtn() { return m_opt.gUnderTabBtn; }
	void gSetUnderExtBtn(int btn) { m_opt.gUnderTabBtn = btn; }
	int gGetAvatarNewBie() { return m_opt.gAvatarNewbie; }
	void gSetAvatarNewBie(int newbie) { m_opt.gAvatarNewbie = newbie; }
	int gGetCaptureHideGUI() { return m_opt.gCaptureHideGUI; }

	int gGetCaptureHidePI() { return m_opt.gCaptureHidePI; }

	float gGetBar_Y();
	int gGetTerrainTooltip() { return m_opt.gTerrainTooltip; }

	const char* mGetMacroText(int index) { return m_opt.mMacro[index]; }
	void mSetMacroText(int index, const char* text)
	{
		strcpy(m_opt.mMacro[index], text);
	}
	int mGetSaveMacros() { return m_opt.mSaveMacros; }
	void mSetPCBangMascotMsg(const char* msg)
	{
		strncpy(m_opt.mPCBangMascotMsg, msg, 29);
	}

	int gGetCutinFlag() { return m_opt.gCutinDisplay; }

	void dApplyDebugSetting();
	const char* dGetCaptureFormat();
	void dSetCaptureFormat(int format);
	int dGetAviWidth();
	int dGetAviHeight();
	int dGetAviFps();
	int dGetStartMode();
	int dGetLobbyCourse();
	int dGetDefaultCharacter();
	int dGetDefaultHairColor();
	int dGetDefaultCaddie();
	int dGetDefaultClub();
	int dGetDefaultBall();
	int dGetMap();
	unsigned char dGetStartHole();
	int dGetReplayMode();
	void dSetReplayMode(int mode);
	int dGetNumPlayers();
	void dSetNumPlayers(int num);
	int dGetWeather();
	void SetQuitWindowed(bool bQuitWindowed)
	{
		m_bQuitWindowed = bQuitWindowed;
	}
	bool GetQuitWindowed() { return m_bQuitWindowed; }
	unsigned char dGetChatGender();
	bool dIsVtxClrTestEnabled();
	bool dIsBaseMeshTestEnabled();
	int dGetClickSystem();
	int dGetGameType();
	bool dIsVersionInfoEnabled();
	float GetBarX983() const { return m_fBarX983; } // JP 983: 00492760

protected:
	void InitVars();
	void Init();
	void SaveReg();
	bool ReadReg(int index, char* buf, int size);
	void WriteReg(int index, const char* value);
	bool ScanReg_I(int index, int* value);
	bool ScanReg_F(int index, float* value);
	bool ScanReg_S(int index, char* value);
	bool WriteReg_I(int index, int value);
	bool WriteReg_F(int index, float value);
	bool WriteReg_S(int index, const char* value);

	bool vChangeScreenSize(int width, int height, int color, bool bLobby);
	void UpdatePowerGaugeLayout983(); // JP 983: 00496b20 (medidas da barra de forca no CUiFrameSetManager)
	void CenterQuitWindow983(); // JP 983: 00493a60
	static bool OnScreenSizeChildWnd983(FrWnd* pWnd, void* param); // JP 983: 00494bb0 (EnumerateChildWindow)
	void vApplyMipmap();
	void aApplyAudioSetting();

	static const char* ms_szReg[][2];
	static const char* ms_szCaptureFmt[];

	WRegistry m_reg;
	HWND__* m_hWnd;
	WDeviceManager* m_pDeviceManager;
	WResourceManager* m_pResourceManager;
	WVideoDev* m_pVideoDev;
	WView* m_pGlobalView;
	CSoundManager* m_pAudioManager;
	CInputManager* m_pInputManager;
	sOption m_opt;
	int m_nTnLMode;
	bool m_bPendingChange;
	sOption m_newOpt;
	bool m_bGameScreen;
	bool m_bSwapMouseButton;
	bool m_bQuitWindowed;
	std::list<sDisplayMode> m_displayModeList;
	// JP 983: modo da area de trabalho (SetVideoDevice 00496770: zera 0x1c = sDisplayMode, comando 0x168 do video, padrao 800x600x32;
	// text nunca preenchido)
	sDisplayMode m_desktopMode983;

	int m_nCaptureFmt;
	int m_nAviWidth;
	int m_nAviHeight;
	int m_nAviFps;
	int m_nStartMode;
	int m_bDbgStartFlag983; // JP 983: bool de depuracao; so InitVars 00491f90 zera e CheckDebugRegValid 004934a0 limita; dGet 00492640 sem chamador
	int m_nLobbyCourse;
	int m_nDefaultCharacter;
	int m_nDefaultHairColor; // JP 983 0x7a8 (004925b0): cor do cabelo do jogador 0 no AddPlayerOffline 006a6330
	int m_nDefaultCaddie; // JP 983 0x7ac (004925c0): 00640890 usa | 0x1c000000; -1 = sem caddie
	int m_nDefaultClub; // JP 983 0x7b0 (004925d0)
	int m_nDefaultBall; // JP 983 0x7b4 (004925e0)
	int m_nMap;
	int m_nStartHole;
	int m_nReplayMode;
	int m_nNumPlayers;
	int m_nGameTime;
	int m_nWeather;
	int m_nChatGender;
	int m_bVtxClrTest;
	int m_bBaseMeshTest;
	int m_nClickSystem;
	int m_nGameType;
	// JP 983: depuracao sem uso: so CheckDebugRegValid 004934a0 limita; dGet 004926e0/004926f0/00492720 sem chamador,
	// nenhum acesso via g_option 00e73f00
	int m_nDbgGameType2_983; // 0..23 como m_nGameType
	int m_nDbgValue983; // >= 0
	int m_bDbgFlag983; // bool
	int m_bVersionInfo; // JP 983: 0x7f0 (dIsVersionInfoEnabled 00492740)
	float m_fBarX983; // JP 983: (float)vScreenWidth (InitVars 00491f10)
	float m_fBarY;
	// JP 983: ultimo pedido do vChangeScreenSize (00494120); InitVars poe 800x600, 0
	int m_lastScreenWidth983;
	int m_lastScreenHeight983;
	bool m_bLastScreenLobby983;
// HACK: only clientsetting.cpp sees these: new names here change projectg (SaveScreenShot)
#if !defined(REBANG_SEMANTIC_ONLY) && defined(REBANG_CLIENTSETTING_PCH)
public:
	static void __cdecl
	RbCoptionFunclets(); // HACK: EH funclets of the naked ~COption (nakedify)
	static void __cdecl
	RbCoption2Funclets(); // HACK: EH funclets of the naked COption (nakedify)
	static void __cdecl
	RbSendupdategameopFunclets(); // HACK: EH funclets of the naked SendUpdateGameOptionPacket (nakedify)
	static void __cdecl
	RbCheckpendingchanFunclets(); // HACK: EH funclets of the naked CheckPendingChanges (nakedify)
#endif
};

struct sGameOptionInfo
{
	char macro[8][64];
	char reserved[64];
};
