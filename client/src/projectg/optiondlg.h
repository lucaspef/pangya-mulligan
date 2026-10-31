#pragma once

#include "clientsetting.h"

class FrSchoolDlg;
class FrOptionDlg;

class FrChangeNickDlg : public FrForm
{
	DECLARE_OBJECT(FrChangeNickDlg)

	FrChangeNickDlg();

	virtual ~FrChangeNickDlg() { }

	virtual bool OnInit();

	void SetOptionDlg(FrOptionDlg* pDlg) { m_pOptionDlg = pDlg; }
	void SetNick(const char* nick) { m_nick = nick; }

protected:
	void OnPriceStaticInit(int param);
	void OnPriceInit(int param);
	void OnMyCookieStaticInit(int param);
	void OnMyCookieInit(int param);
	void OnRemainCookieStaticInit(int param);
	void OnRemainCookieInit(int param);
	void OnCheckoutInit(int param);
	void OnCheckoutBtnUp();

	bool OnChangeNickConfirmDlgResult(int result, FrForm* form);

	FrOptionDlg* m_pOptionDlg;
	const char* m_nick;
	bool m_bHaveItem;

	DECLARE_FRESH_MSGMAP()
};

#include "frform.h"

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class FrOptionDlg : public FrForm
{
	DECLARE_OBJECT(FrOptionDlg)

	enum eTabType
	{
		TAB_GAME,
		TAB_VIDEO,
		TAB_AUDIO,
		TAB_MACRO,
		TAB_SHADER, // JP 983
		TAB_NUM,
	};

	FrOptionDlg();
	virtual ~FrOptionDlg();

	void SetApplyScrSize(bool bApply);
	void OpenTab(eTabType tab);
	void CloseDlg();
	void EnableChangeNickControls(bool bEnable);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float delta);
	virtual bool OnLButtonDown(const WPoint& pt);
	virtual void OnOK();
	virtual void OnCancel();

	void OnOkBtnInit(int param);
	void OnGameTabInit(int param);
	void OnVideoTabInit(int param);
	void OnAudioTabInit(int param);
	void OnMacroTabInit(int param);
	void OnGameTabUp();
	void OnVideoTabUp();
	void OnAudioTabUp();
	void OnMacroTabUp();
	void sOnShaderTabInit(int param); // JP 983: aba "shader" (008c94b0/008c9530)
	void sOnShaderTabUp();

	void gOnMsSensiStaticInit(int param);
	void gOnMsSensiGaugeInit(int param);
	void gOnWhisperStaticInit(int param);
	void gOnWhisperBtnInit(int param);
	void gOnWhisperBtnUp();
	void gOnInvitationStaticInit(int param);
	void gOnInvitationBtnInit(int param);
	void gOnInvitationBtnUp();
	void gOnFriendConfirmStaticInit(int param);
	void gOnFriendConfirmBtnInit(int param);
	void gOnFriendConfirmBtnUp();
	void gOnTransChatWinStaticInit(int param);
	void gOnTransChatWinBtnInit(int param);
	void gOnTransChatWinBtnUp();
	void gOnUiEffectStaticInit(int param);
	void gOnTerrainTooltipStaticInit(int param);
	void gOnUiEffectBtnInit(int param);
	void gOnUiEffectBtnUp();
	void gOnNickStaticInit(int param);
	void gOnNickEditInit(int param);
	bool gOnNickEditEnterKey(int param);
	void gOnChangeNickBtnInit(int param);
	void gOnChangeNickBtnUp();
	void gOnSchoolStaticInit(int param);
	void gOnSchoolEditInit(int param);
	bool gOnSchoolEditEnterKey(int param);
	void gOnChangeSchoolBtnInit(int param);
	void gOnChangeSchoolBtnUp();
	void gOnPowerGaugeStaticInit(int param);
	void gOnPowerGaugeBtnInit(int param);
	void gOnPowerGaugeBtnUp();
	// JP 983: "g_fullyproportionalpowergaugesize" / "g_centeredpowergauge" (008c9f00/008c9f70/008c9fe0, 008ca000/008ca070/008ca0e0)
	void gOnFullyPropPowerGaugeStaticInit(int param);
	void gOnFullyPropPowerGaugeBtnInit(int param);
	void gOnFullyPropPowerGaugeBtnUp();
	void gOnCenteredPowerGaugeStaticInit(int param);
	void gOnCenteredPowerGaugeBtnInit(int param);
	void gOnCenteredPowerGaugeBtnUp();
	void gOnHideGUIStaticInit(int param);
	void gOnHideGUIBtnInit(int param);
	void gOnHideGUIBtnUp();
	void gOnHidePIStaticInit(int param);
	void gOnHidePIBtnInit(int param);
	void gOnHidePIBtnUp();
	void gOnTerrainBtnInit(int param);
	void gOnTerrainBtnUp();
	void gOnCutinDisplayStaticInit(int param);
	void gOnCutinDisplayBtnInit(int param);
	void gOnCutinDisplayBtnUp();
	// JP 983: "g_mbindmode" (prender o cursor na janela; 008caaa0/008cab10/008cab80)
	void gOnBindMouseCursorStaticInit(int param);
	void gOnBindMouseCursorBtnInit(int param);
	void gOnBindMouseCursorBtnUp();

	void vOnFullScreenStaticInit(int param);
	void vOnFullScreenBtnInit(int param);
	void vOnFullScreenBtnUp();
	void vOnFillScreenStaticInit(int param);
	void vOnFillScreenBtnInit(int param);
	void vOnFillScreenBtnUp();
	void vOnWideModeStaticInit(int param);
	void vOnWideModeBtnInit(int param);
	void vOnWideModeBtnUp();
	void vOnOverallStaticInit(int param);
	void vOnOverallCmbInit(int param);
	void vOnScrResStaticInit(int param);
	void vOnScrResCmbInit(int param);
	void vOnTexResStaticInit(int param);
	void vOnTexResCmbInit(int param);
	void vOnShadowStaticInit(int param);
	void vOnShadowBtnInit(int param);
	void vOnShadowBtnUp();
	void vOnShadowmapStaticInit(int param);
	void vOnShadowmapBtnInit(int param);
	void vOnShadowmapBtnUp();
	void vOnReflectionStaticInit(int param);
	void vOnReflectionBtnInit(int param);
	void vOnReflectionBtnUp();
	void vOnNpcStaticInit(int param);
	void vOnNpcBtnInit(int param);
	void vOnNpcBtnUp();
	void vOnParticleStaticInit(int param);
	void vOnParticleCmbInit(int param);

	void aOnEnableStaticInit(int param);
	void aOnEnableBtnInit(int param);
	void aOnEnableBtnUp();
	void aOnOverallStaticInit(int param);
	void aOnOverallCmbInit(int param);
	void aOnSfxStaticInit(int param);
	void aOnSfxGaugeInit(int param);
	void aOnBgmStaticInit(int param);
	void aOnBgmGaugeInit(int param);
	void aOnSpeakerStaticInit(int param);
	void aOnSpeakerCmbInit(int param);

	void mOnMacroStatic1Init(int param);
	void mOnMacroStatic2Init(int param);
	void mOnMacroStatic3Init(int param);
	void mOnMacroStatic4Init(int param);
	void mOnMacroStatic5Init(int param);
	void mOnMacroStatic6Init(int param);
	void mOnMacroStatic7Init(int param);
	void mOnMacroStatic8Init(int param);
	void mOnMacro1Init(int param);
	void mOnMacro2Init(int param);
	void mOnMacro3Init(int param);
	void mOnMacro4Init(int param);
	void mOnMacro5Init(int param);
	void mOnMacro6Init(int param);
	void mOnMacro7Init(int param);
	void mOnMacro8Init(int param);
	bool mOnMacroEnterKey(int param);
	void mOnMacroEnterKeyLBtnDown();

	// JP 983: aba "shader" (008cc0a0..008cc9e7, 008cdba0)
	void sOnShaderOnOffBtnInit(int param);
	void sOnShaderOnOffBtnUp();
	void sOnHdrStaticInit(int param);
	void sOnHdrBtnInit(int param);
	void sOnHdrBtnUp();
	void sOnHdrStarEffectStaticInit(int param);
	void sOnHdrStarEffectBtnInit(int param);
	void sOnHdrStarEffectBtnUp();
	void sOnDofStaticInit(int param);
	void sOnDofBtnInit(int param);
	void sOnDofBtnUp();
	void sOnAmbientControlStaticInit(int param);
	void sOnAmbientControlBtnInit(int param);
	void sOnAmbientControlBtnUp();
	void sOnRimLightStaticInit(int param);
	void sOnRimLightBtnInit(int param);
	void sOnRimLightBtnUp();
	void sOnFxaaStaticInit(int param);
	void sOnFxaaBtnInit(int param);
	void sOnFxaaBtnUp();
	void sOnBrightStaticInit(int param);
	void sOnBrightGaugeInit(int param);

	void tOnHintInit(int param);
	void OnHoverOn(int param);
	void OnHoverOff(int param);
	const char* MakeHint(eTabType tab, int index) const;
	void ProcessHoverMessage(FrWnd* pWnd);

	void InitVideoControls();
	void SetGameColtrols();
	void SetVideoColtrols(bool bReset);
	void SetAudioColtrols();
	void SetMacroColtrols();
	void SetShaderColtrols(); // JP 983 (008cce00)
	void SetTab(eTabType tab);

	bool OnChangeNickConfirmDlgResult(int result, FrForm* form);
	bool OnChangeSchoolDlgResult(int result, FrForm* form);

	static eTabType ms_eCurTab;

public:
	struct sTab
	{
		sTab();

		virtual void SetSelected(bool bSelect);

		virtual void Init() { }

		bool m_bSelected;
		FrButton* m_pTabBtn;
		int m_num;
		FrStatic** m_ppStatic;
		FrWnd*** m_pppCtrl;
		bool* m_pbDisable;
	};

	struct sGameTab : public sTab
	{
		sGameTab();

		virtual void SetSelected(bool bSelect);
		virtual void Init();

		bool IsEnableChangeNickname();

		FrStatic* m_pStatic[16];
		FrWnd** m_ppCtrl[16];
		FrGaugeBarEx* m_pMsSensiGauge;
		FrButton* m_pWhisperBtn;
		FrButton* m_pInvitationBtn;
		FrButton* m_pFriendConfirmBtn;
		FrButton* m_pTransChatWinBtn;
		FrButton* m_pUiEffectBtn;
		FrEdit* m_pNickEdit;
		FrButton* m_pChangeNickBtn;
		FrEdit* m_pSchoolEdit;
		FrButton* m_pChangeSchoolBtn;
		FrButton* m_pPowerGaugeBtn;
		FrButton* m_pFullyPropPowerGaugeBtn; // JP 983 (+0xc8)
		FrButton* m_pCenteredPowerGaugeBtn; // JP 983 (+0xcc)
		FrButton* m_pHideGUIBtn;
		FrButton* m_pHidePIBtn;
		FrButton* m_pTerrainBtn;
		FrButton* m_pCutinDisplayBtn;
		FrButton* m_pBindMouseCursorBtn; // JP 983 (+0xe0)
		bool m_bEnableChangeNick;
	}; // JP 983: 0xe8 (construtor 008c84a0; 16 rotulos, 18 controles)

	struct sVideoTab : public sTab
	{
		sVideoTab();

		FrStatic* m_pStatic[13];
		FrWnd** m_ppCtrl[13];
		bool m_bDisable[13];
		FrButton* m_pWideModeBtn;
		FrButton* m_pFullScreenBtn;
		FrButton* m_pFillScreenBtn;
		FrComboBox* m_pOverallCmb;
		FrComboBox* m_pScrResCmb;
		FrComboBox* m_pTnlCmb; // JP 983: sem controle nem manipulador ("v_tnl" saiu do mapa)
		FrComboBox* m_pTexResCmb;
		FrButton* m_pShadowBtn;
		FrButton* m_pShadowmapBtn;
		FrButton* m_pReflectionBtn;
		FrButton* m_pLodBtn; // JP 983: sem controle nem manipulador ("v_lod" saiu do mapa)
		FrButton* m_pNpcBtn;
		FrComboBox* m_pParticleCmb;
	};

	struct sAudioTab : public sTab
	{
		sAudioTab();

		FrStatic* m_pStatic[5];
		FrWnd** m_ppCtrl[5];
		bool m_bDisable[5];
		FrButton* m_pEnableBtn;
		FrComboBox* m_pOverallCmb;
		FrGaugeBarEx* m_pSfxGauge;
		FrGaugeBarEx* m_pBgmGauge;
		FrComboBox* m_pSpeakerCmb;
	};

	struct sMacroTab : public sTab
	{
		sMacroTab();

		FrStatic* m_pStatic[8];
		FrWnd** m_ppCtrl[8];
		bool m_bDisable[8];
		FrEdit* m_pMacro[8];
	};

	// JP 983: 0x84 (construtor 008c8990; vtable 00d38170 = a do sTab)
	struct sShaderTab : public sTab
	{
		sShaderTab();

		FrStatic* m_pStatic[8];
		FrWnd** m_ppCtrl[8];
		bool m_bDisable[8];
		FrButton* m_pHdrBtn;
		FrButton* m_pHdrStarEffectBtn;
		FrButton* m_pDofBtn;
		FrButton* m_pAmbientControlBtn;
		FrButton* m_pRimLightBtn;
		FrButton* m_pFxaaBtn;
		FrGaugeBarEx* m_pBrightGauge;
		FrButton* m_pShaderOnOffBtn;
	};

protected:
	bool m_bApplyScrSize;
	sOption m_option;
	bool m_bApply;
	bool m_bOK;
	FrButton* m_pOkBtn;
	sGameTab m_gameTab;
	sVideoTab m_videoTab;
	sAudioTab m_audioTab;
	sMacroTab m_macroTab;
	sShaderTab m_shaderTab;
	sTab* m_pTab[TAB_NUM];
	FrChangeNickDlg* m_pChangeNickDlg;
	FrSchoolDlg* m_pSchoolDlg;
	FrEdit* m_pHint;
	int m_hoverCount;

	DECLARE_FRESH_MSGMAP()
public:
	using FrForm::m_defaultRetCode;
public:
	char m_pad983_79c[4]; // 983: sem leitura/escrita (+0x79c..0x79f ausente em 008c0000..008d2000: ctor 008c9070 e todas as funcoes da unidade); sizeof 0x7a0 (push em 008cd950)
};
