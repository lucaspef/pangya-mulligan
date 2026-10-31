#include "minatl.h"
#include "vc10_layout.h"
#include "networksystem.h"
#include "optiondlg.h"
#include "clientsetting.h"
#include "objectfactory.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frbutton.h"
#include "frstatic.h"
#include "fredit.h"
#include "frcombobox.h"
#include "frgaugebar.h"
#include "projectg.h"
#include "shareddoc.h"
#include "logininfo.h"
#include "soundmanager.h"
#include "schooldlg.h"
#include "taskmanager.h"
#include "golftask.h"
#include "chatmsg.h"
#include "messengerdlg.h"
#include "lobbytask.h"
#include "inputmanager.h"
#include "wresrcmng.h"
#include "mousecursor.h"
#include "gatewayactor.h"
#include "actor.h"
#include "packet.h"
#include "../shared/localize.h"
#include "../shared/token.h"
#include "../shared/s5/sharedutilities.h"
#include "jp/gamestringmanager.h"

extern Fresh* g_pFresh;

// JP 983: condicao da aba "shader" (MRT, pixel shader e vertex shader), repetida em cada manipulador da aba
static inline bool IsSupportShader983()
{
	return sOption::sbMrtSupport && sOption::sbPsSupport &&
		sOption::sbVsSupport;
}

FrOptionDlg::sTab::sTab()
	: m_bSelected(true), m_pTabBtn(0), m_num(0), m_ppStatic(0), m_pppCtrl(0)
{
}

void FrOptionDlg::sTab::SetSelected(bool bSelected)
{
	m_bSelected = bSelected;

	if (!bSelected)
	{
		m_pTabBtn->SetStatus(FrButton::NORMAL);
	}

	if (m_ppStatic)
	{
		for (int i = 0; i < m_num; i++)
		{
			if (m_ppStatic[i])
			{
				if (m_pbDisable && m_pbDisable[i])
				{
					m_ppStatic[i]->Enable(false);
				}
				else
				{
					m_ppStatic[i]->Enable(m_bSelected);
				}

				m_ppStatic[i]->SetVisible(m_bSelected);
			}
		}
	}

	if (m_pppCtrl)
	{
		for (int i = 0; i < m_num; i++)
		{
			if (m_pppCtrl[i] && *m_pppCtrl[i])
			{
				if (m_pbDisable && m_pbDisable[i])
				{
					(*m_pppCtrl[i])->Enable(false);
				}
				else
				{
					(*m_pppCtrl[i])->Enable(m_bSelected);
				}

				(*m_pppCtrl[i])->EnableHover(m_bSelected, 0);
				(*m_pppCtrl[i])->SetVisible(m_bSelected);
			}
		}
	}
}

FrOptionDlg::sGameTab::sGameTab()
{
	m_num = 16;
	m_ppStatic = m_pStatic;
	m_pppCtrl = m_ppCtrl;
	m_pbDisable = 0;

	memset(m_pStatic, 0, sizeof(m_pStatic));

	m_pMsSensiGauge = 0;
	m_pWhisperBtn = 0;
	m_pInvitationBtn = 0;
	m_pFriendConfirmBtn = 0;
	m_pTransChatWinBtn = 0;
	m_pUiEffectBtn = 0;
	m_pNickEdit = 0;
	m_pChangeNickBtn = 0;
	m_pSchoolEdit = 0;
	m_pChangeSchoolBtn = 0;
	m_pPowerGaugeBtn = 0;
	m_pFullyPropPowerGaugeBtn = 0;
	m_pCenteredPowerGaugeBtn = 0;
	m_pHideGUIBtn = 0;
	m_pHidePIBtn = 0;
	// m_pTerrainBtn is not cleared (the original skips it).
	m_pCutinDisplayBtn = 0;
	m_pBindMouseCursorBtn = 0;

	m_ppCtrl[0] = (FrWnd**)&m_pMsSensiGauge;
	m_ppCtrl[1] = (FrWnd**)&m_pWhisperBtn;
	m_ppCtrl[2] = (FrWnd**)&m_pInvitationBtn;
	m_ppCtrl[3] = (FrWnd**)&m_pFriendConfirmBtn;
	m_ppCtrl[4] = (FrWnd**)&m_pTransChatWinBtn;
	m_ppCtrl[5] = (FrWnd**)&m_pUiEffectBtn;
	m_ppCtrl[6] = (FrWnd**)&m_pNickEdit;
	m_ppCtrl[7] = (FrWnd**)&m_pSchoolEdit;
	m_ppCtrl[8] = (FrWnd**)&m_pPowerGaugeBtn;
	m_ppCtrl[9] = (FrWnd**)&m_pFullyPropPowerGaugeBtn;
	m_ppCtrl[10] = (FrWnd**)&m_pCenteredPowerGaugeBtn;
	m_ppCtrl[11] = (FrWnd**)&m_pHideGUIBtn;
	m_ppCtrl[12] = (FrWnd**)&m_pHidePIBtn;
	m_ppCtrl[13] = (FrWnd**)&m_pTerrainBtn;
	m_ppCtrl[14] = (FrWnd**)&m_pCutinDisplayBtn;
	m_ppCtrl[15] = (FrWnd**)&m_pBindMouseCursorBtn;

	m_bEnableChangeNick = false;
}

void FrOptionDlg::sGameTab::SetSelected(bool bSelected)
{
	sTab::SetSelected(bSelected);

	if (bSelected)
	{
		if (m_pChangeNickBtn)
		{
			m_pChangeNickBtn->SetVisible(true);
			m_pChangeNickBtn->Enable(m_bEnableChangeNick);
		}

		if (m_pChangeSchoolBtn)
		{
			m_pChangeSchoolBtn->SetVisible(true);
		}
	}
	else
	{
		if (m_pChangeNickBtn)
		{
			m_pChangeNickBtn->SetVisible(false);
		}

		if (m_pChangeSchoolBtn)
		{
			m_pChangeSchoolBtn->SetVisible(false);
		}
	}
}

void FrOptionDlg::sGameTab::Init()
{
	if (OnlinePlay() && !(IS_KINDOF(CGolfTask, AfxGetTask())))
	{
		m_bEnableChangeNick = IsEnableChangeNickname();
	}
}

// JP 983 (008c86d0): sempre pode (a aba de jogo do 983 esconde o apelido em SetTab)
bool FrOptionDlg::sGameTab::IsEnableChangeNickname()
{
	return true;
}

FrOptionDlg::sVideoTab::sVideoTab()
{
	m_num = 13;
	m_ppStatic = m_pStatic;
	m_pppCtrl = m_ppCtrl;
	m_pbDisable = m_bDisable;

	memset(m_pStatic, 0, sizeof(m_pStatic));
	memset(m_bDisable, 0, sizeof(m_bDisable));

	m_pFullScreenBtn = 0;
	m_pFillScreenBtn = 0;
	m_pWideModeBtn = 0;
	m_pOverallCmb = 0;
	m_pScrResCmb = 0;
	m_pTnlCmb = 0;
	m_pTexResCmb = 0;
	m_pShadowBtn = 0;
	m_pShadowmapBtn = 0;
	m_pReflectionBtn = 0;
	m_pLodBtn = 0;
	m_pNpcBtn = 0;
	m_pParticleCmb = 0;

	m_ppCtrl[0] = (FrWnd**)&m_pFullScreenBtn;
	m_ppCtrl[1] = (FrWnd**)&m_pFillScreenBtn;
	m_ppCtrl[2] = (FrWnd**)&m_pWideModeBtn;
	m_ppCtrl[3] = (FrWnd**)&m_pOverallCmb;
	m_ppCtrl[4] = (FrWnd**)&m_pScrResCmb;
	m_ppCtrl[5] = (FrWnd**)&m_pTnlCmb;
	m_ppCtrl[6] = (FrWnd**)&m_pTexResCmb;
	m_ppCtrl[7] = (FrWnd**)&m_pShadowBtn;
	m_ppCtrl[8] = (FrWnd**)&m_pShadowmapBtn;
	m_ppCtrl[9] = (FrWnd**)&m_pReflectionBtn;
	m_ppCtrl[10] = (FrWnd**)&m_pLodBtn;
	m_ppCtrl[11] = (FrWnd**)&m_pNpcBtn;
	m_ppCtrl[12] = (FrWnd**)&m_pParticleCmb;

	// Options the video card cannot do are greyed out.
	if (!sOption::IsSupportShadow())
	{
		m_bDisable[7] = true;
	}

	if (!sOption::IsSupportReflection())
	{
		m_bDisable[9] = true;
	}

	if (!sOption::IsSupportLod())
	{
		m_bDisable[10] = true;
	}
}

FrOptionDlg::sAudioTab::sAudioTab()
{
	m_num = 5;
	m_ppStatic = m_pStatic;
	m_pppCtrl = m_ppCtrl;
	m_pbDisable = m_bDisable;

	memset(m_pStatic, 0, sizeof(m_pStatic));

	if (g_audio->GetMSS())
	{
		memset(m_bDisable, 0, sizeof(m_bDisable));
	}
	else
	{
		memset(m_bDisable, 1, sizeof(m_bDisable));
	}

	m_pEnableBtn = NULL;
	m_pOverallCmb = NULL;
	m_pSfxGauge = NULL;
	m_pBgmGauge = NULL;
	m_pSpeakerCmb = NULL;

	m_ppCtrl[0] = (FrWnd**)&m_pEnableBtn;
	m_ppCtrl[1] = (FrWnd**)&m_pOverallCmb;
	m_ppCtrl[2] = (FrWnd**)&m_pSfxGauge;
	m_ppCtrl[3] = (FrWnd**)&m_pBgmGauge;
	m_ppCtrl[4] = (FrWnd**)&m_pSpeakerCmb;
}

FrOptionDlg::sMacroTab::sMacroTab()
{
	m_num = 8;
	m_ppStatic = m_pStatic;
	m_pppCtrl = m_ppCtrl;
	m_pbDisable = m_bDisable;

	memset(m_pStatic, 0, sizeof(m_pStatic));
	memset(m_bDisable, 0, sizeof(m_bDisable));

	for (int i = 0; i < 8; i++)
	{
		m_pMacro[i] = 0;
		m_ppCtrl[i] = (FrWnd**)&m_pMacro[i];
	}
}

FrOptionDlg::sShaderTab::sShaderTab()
{
	m_num = 8;
	m_ppStatic = m_pStatic;
	m_pppCtrl = m_ppCtrl;
	m_pbDisable = m_bDisable;

	memset(m_pStatic, 0, sizeof(m_pStatic));
	memset(m_bDisable, IsSupportShader983() ? false : true, sizeof(m_bDisable));

	m_ppCtrl[0] = (FrWnd**)&m_pHdrBtn;
	m_ppCtrl[1] = (FrWnd**)&m_pHdrStarEffectBtn;
	m_ppCtrl[2] = (FrWnd**)&m_pDofBtn;
	m_ppCtrl[3] = (FrWnd**)&m_pAmbientControlBtn;
	m_ppCtrl[4] = (FrWnd**)&m_pRimLightBtn;
	m_ppCtrl[5] = (FrWnd**)&m_pFxaaBtn;
	m_ppCtrl[6] = (FrWnd**)&m_pBrightGauge;
	m_ppCtrl[7] = (FrWnd**)&m_pShaderOnOffBtn;

	m_pHdrBtn = 0;
	m_pHdrStarEffectBtn = 0;
	m_pDofBtn = 0;
	m_pAmbientControlBtn = 0;
	m_pRimLightBtn = 0;
	m_pFxaaBtn = 0;
	m_pBrightGauge = 0;
	m_pShaderOnOffBtn = 0;
}

IMPLEMENT_OBJECT(FrChangeNickDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrChangeNickDlg, FrForm)

ON_FRESH_VI("sprice", FRCMD_INIT, FrChangeNickDlg::OnPriceStaticInit)
ON_FRESH_VI("price", FRCMD_INIT, FrChangeNickDlg::OnPriceInit)
ON_FRESH_VI("smycookie", FRCMD_INIT, FrChangeNickDlg::OnMyCookieStaticInit)
ON_FRESH_VI("mycookie", FRCMD_INIT, FrChangeNickDlg::OnMyCookieInit)
ON_FRESH_VI("sremaincookie", FRCMD_INIT,
	FrChangeNickDlg::OnRemainCookieStaticInit)
ON_FRESH_VI("remaincookie", FRCMD_INIT, FrChangeNickDlg::OnRemainCookieInit)
ON_FRESH_VI("checkout", FRCMD_INIT, FrChangeNickDlg::OnCheckoutInit)
ON_FRESH_VV("checkout", FRCMD_LBUTTONUP, FrChangeNickDlg::OnCheckoutBtnUp)

END_FRESH_MSGMAP()

FrChangeNickDlg::FrChangeNickDlg()
	: m_nick(0)
{
	CSharedDoc* pDoc = Doc();
	std::list<sItemInfo>::iterator it = std::find(pDoc->m_myItemList.begin(),
		pDoc->m_myItemList.end(), 0x1a000003);

	m_bHaveItem = it != pDoc->m_myItemList.end();
	m_pOptionDlg = NULL;
}

void FrChangeNickDlg::OnPriceStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
}

void FrChangeNickDlg::OnPriceInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
	else
	{
		// 49 cookies.
		if (IsLocalCountry(30))
		{
			pStatic->SetCaption(MakeStr(K2L_Compatibility("%d\260\263(CK)"), 49));
		}
		else
		{
			pStatic->SetCaption(MakeStr(K2L_Compatibility("%d\260\263"), 49));
		}
	}
}

void FrChangeNickDlg::OnMyCookieStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
}

void FrChangeNickDlg::OnMyCookieInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
	else
	{
		if (IsLocalCountry(30))
		{
			pStatic->SetCaption(MakeStr(K2L_Compatibility("%I64d\260\263(CK)"), Doc()->m_cookie));
		}
		else
		{
			pStatic->SetCaption(MakeStr(K2L_Compatibility("%I64d\260\263"), Doc()->m_cookie));
		}
	}
}

void FrChangeNickDlg::OnRemainCookieStaticInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
}

void FrChangeNickDlg::OnRemainCookieInit(int param)
{
	FrStatic* pStatic = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!pStatic)
	{
		return;
	}

	if (m_bHaveItem || (Doc()->m_myInfo.info.dwIdentity & 0x204))
	{
		pStatic->SetVisible(false);
	}
	else
	{
		if (Doc()->m_cookie < 49)
		{
			// "%I64d cookies short".
			pStatic->SetCaption(MakeStr("%I64d\260\263 \272\316\301\267",
				49 - Doc()->m_cookie));
		}
		else
		{
			if (IsLocalCountry(30))
			{
				pStatic->SetCaption(
					MakeStr("%I64d\260\263(CK)", Doc()->m_cookie - 49));
			}
			else
			{
				pStatic->SetCaption(
					MakeStr(K2L_Compatibility("%I64d\260\263"), Doc()->m_cookie - 49));
			}
		}
	}
}

void FrChangeNickDlg::OnCheckoutInit(int param)
{
	FrButton* pBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!pBtn)
	{
		return;
	}

	CSharedDoc* pDoc = Doc();
	if (pDoc->m_cookie < 49 && !m_bHaveItem &&
		!(pDoc->m_myInfo.info.dwIdentity & 0x204))
	{
		pBtn->Enable(false);
	}
}

void FrChangeNickDlg::OnCheckoutBtnUp()
{
	FrForm* pForm =
		CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_okcancel", 0);
	// "Do you want to change your nickname?"
	pForm->SetMessage(
		K2L_Compatibility("\\c0xffff0000\\c\264\353\310\255\270\355\300\273 \272\257\260\346 \307\317\275\303\260\332\275\300\264\317\261\356?"),
		false);

	pForm->Open(
		(FRESH_PFN_RESULT)(&FrChangeNickDlg::OnChangeNickConfirmDlgResult), 1);
}

bool FrChangeNickDlg::OnInit()
{
	char msg[512] = "";

	sprintf(msg, K2L_Compatibility("\264\353\310\255\270\355\300\273 '%s'\277\241\274\255\n'%s'(\300\270)\267\316 \272\257\260\346\307\325\264\317\264\331."), Doc()->m_myInfo.info.sNick, m_nick);

	if (Doc()->m_myInfo.info.dwIdentity & 0x204)
		strcat(msg, K2L_Compatibility("\300\314 \276\306\300\314\265\360\264\302 \271\253\267\341\267\316 \264\353\310\255\270\355 \272\257\260\346\300\314 \260\241\264\311\307\325\264\317\264\331."));

	if (!IsLocalContent(S4_NT_NICKNAME_CHANGE_2ND))
	{
		if (CLoginInfo::Instance()->IsPcBang())
			strcat(msg, K2L_Compatibility("\\c0xffff0000\\c\n\n\307\307\276\276\271\346\277\241\274\255\264\302 \264\353\310\255\270\355\300\273 \271\253\267\341\267\316 \272\257\260\346\307\322 \274\366 \300\326\275\300\264\317\264\331.\\c0xff000000\\c"));
		else if (m_bHaveItem)
			strcat(msg, K2L_Compatibility("\n\n\264\353\310\255\270\355\300\273 \272\257\260\346\307\322 \266\247 \\c0xffff0000\\c\264\353\310\255\270\355 \272\257\260\346 \276\306\300\314\305\333\\c0xff000000\\c\300\314 1\260\263 \274\322\270\360\265\313\264\317\264\331."));
	}
	else
	{
		const char* prov = K2L_Compatibility(Doc()->m_provType == 2 ? "\306\256\270\256\304\263\275\254" : "\307\321\304\332\300\316");

		strcat(msg, K2L_Compatibility("\n\n\264\353\310\255\270\355\300\273 \272\257\260\346\307\322 \266\247 \\c0xffff0000\\c49\304\355\305\260\\c0xff000000\\c\260\241 \302\367\260\250\265\313\264\317\264\331."));

		if (Doc()->m_cookie < 49)
			strcat(msg, MakeStr(K2L_Compatibility("\\c0xffff0000,t\\c%s \300\334\276\327\300\314 \272\316\301\267\307\325\264\317\264\331."), prov));
		else
			strcat(msg, MakeStr(K2L_Compatibility("\\c0xffff0000,t\\c\272\257\260\346 \310\304 \303\326\301\276 %s \300\334\276\327\300\272 %I64d\277\370\\c0xff000000,n\\c\300\324\264\317\264\331."), prov,
				Doc()->m_cash - S5::ExchangeOwnCookieToCash(49, 100)));
	}

	SetMessage(msg, false);
	return true;
}

bool FrChangeNickDlg::OnChangeNickConfirmDlgResult(int result, FrForm* form)
{
	if (result == FrOK)
	{
		if (Doc()->m_cookie >= 49 || m_bHaveItem ||
			(Doc()->m_myInfo.info.dwIdentity & 0x204))
		{
			WSendPacket packet((enumClientPacket)0x38);
			packet.EncodeStr(std::string(m_nick));
			packet.Send(TO_GAME);

			AfxGetTask()->GetMainActor()
				<< MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);

			if (m_pOptionDlg)
				m_pOptionDlg->EnableChangeNickControls(false);
			Close(FrOK, true);
		}
		else
		{
			AfxGetTask()->GetMainActor() << MsgObject(NULL, M983(0x23),
				(int)K2L_Compatibility("\\c0xffff0000\\c\304\355\305\260\\c0xff000000\\c\260\241 \272\316\301\267\307\325\264\317\264\331.\n\273\363\301\241\277\241\274\255 \304\355\305\260\270\246 \303\346\300\374\307\330 \301\326\274\274\277\344."),
				0, 0, 0, 0);

			if (m_pOptionDlg)
				m_pOptionDlg->EnableChangeNickControls(false);
		}
	}
	else
	{
		if (m_pOptionDlg)
			m_pOptionDlg->EnableChangeNickControls(true);
	}

	m_pOptionDlg = NULL;
	return true;
}

IMPLEMENT_OBJECT(FrOptionDlg, FrForm)

FrOptionDlg::eTabType FrOptionDlg::ms_eCurTab;

BEGIN_FRESH_MSGMAP(FrOptionDlg, FrForm)

ON_FRESH_VI("ok", FRCMD_INIT, FrOptionDlg::OnOkBtnInit)
ON_FRESH_VI("game", FRCMD_INIT, FrOptionDlg::OnGameTabInit)
ON_FRESH_VV("game", FRCMD_LBUTTONDOWN, FrOptionDlg::OnGameTabUp)
ON_FRESH_VI("video", FRCMD_INIT, FrOptionDlg::OnVideoTabInit)
ON_FRESH_VV("video", FRCMD_LBUTTONDOWN, FrOptionDlg::OnVideoTabUp)
ON_FRESH_VI("audio", FRCMD_INIT, FrOptionDlg::OnAudioTabInit)
ON_FRESH_VV("audio", FRCMD_LBUTTONDOWN, FrOptionDlg::OnAudioTabUp)
ON_FRESH_VI("macro", FRCMD_INIT, FrOptionDlg::OnMacroTabInit)
ON_FRESH_VV("macro", FRCMD_LBUTTONDOWN, FrOptionDlg::OnMacroTabUp)
ON_FRESH_VI("shader", FRCMD_INIT, FrOptionDlg::sOnShaderTabInit)
ON_FRESH_VV("shader", FRCMD_LBUTTONDOWN, FrOptionDlg::sOnShaderTabUp)
ON_FRESH_VI("g_ssensi", FRCMD_INIT, FrOptionDlg::gOnMsSensiStaticInit)
ON_FRESH_VI("g_sensi", FRCMD_INIT, FrOptionDlg::gOnMsSensiGaugeInit)
ON_FRESH_VI("g_swhisper", FRCMD_INIT, FrOptionDlg::gOnWhisperStaticInit)
ON_FRESH_VI("g_whisper", FRCMD_INIT, FrOptionDlg::gOnWhisperBtnInit)
ON_FRESH_VV("g_whisper", FRCMD_LBUTTONUP, FrOptionDlg::gOnWhisperBtnUp)
ON_FRESH_VI("g_sinvite", FRCMD_INIT, FrOptionDlg::gOnInvitationStaticInit)
ON_FRESH_VI("g_invite", FRCMD_INIT, FrOptionDlg::gOnInvitationBtnInit)
ON_FRESH_VV("g_invite", FRCMD_LBUTTONUP, FrOptionDlg::gOnInvitationBtnUp)
ON_FRESH_VI("g_sconfirm", FRCMD_INIT, FrOptionDlg::gOnFriendConfirmStaticInit)
ON_FRESH_VI("g_confirm", FRCMD_INIT, FrOptionDlg::gOnFriendConfirmBtnInit)
ON_FRESH_VV("g_confirm", FRCMD_LBUTTONUP, FrOptionDlg::gOnFriendConfirmBtnUp)
ON_FRESH_VI("g_schatwin", FRCMD_INIT, FrOptionDlg::gOnTransChatWinStaticInit)
ON_FRESH_VI("g_chatwin", FRCMD_INIT, FrOptionDlg::gOnTransChatWinBtnInit)
ON_FRESH_VV("g_chatwin", FRCMD_LBUTTONUP, FrOptionDlg::gOnTransChatWinBtnUp)
ON_FRESH_VI("g_suieffect", FRCMD_INIT, FrOptionDlg::gOnUiEffectStaticInit)
ON_FRESH_VI("g_uieffect", FRCMD_INIT, FrOptionDlg::gOnUiEffectBtnInit)
ON_FRESH_VV("g_uieffect", FRCMD_LBUTTONUP, FrOptionDlg::gOnUiEffectBtnUp)
ON_FRESH_VI("g_snick", FRCMD_INIT, FrOptionDlg::gOnNickStaticInit)
ON_FRESH_VI("g_nick", FRCMD_INIT, FrOptionDlg::gOnNickEditInit)
ON_FRESH_BI("g_nick", FRCMD_ENTERKEY, FrOptionDlg::gOnNickEditEnterKey)
ON_FRESH_VI("g_changenick", FRCMD_INIT, FrOptionDlg::gOnChangeNickBtnInit)
ON_FRESH_VV("g_changenick", FRCMD_LBUTTONUP, FrOptionDlg::gOnChangeNickBtnUp)
ON_FRESH_VI("g_sschool", FRCMD_INIT, FrOptionDlg::gOnSchoolStaticInit)
ON_FRESH_VI("g_school", FRCMD_INIT, FrOptionDlg::gOnSchoolEditInit)
ON_FRESH_BI("g_school", FRCMD_ENTERKEY, FrOptionDlg::gOnSchoolEditEnterKey)
ON_FRESH_VI("g_changeschool", FRCMD_INIT, FrOptionDlg::gOnChangeSchoolBtnInit)
ON_FRESH_VV("g_changeschool", FRCMD_LBUTTONUP,
	FrOptionDlg::gOnChangeSchoolBtnUp)
ON_FRESH_VI("g_spowergauge", FRCMD_INIT, FrOptionDlg::gOnPowerGaugeStaticInit)
ON_FRESH_VI("g_powergauge", FRCMD_INIT, FrOptionDlg::gOnPowerGaugeBtnInit)
ON_FRESH_VV("g_powergauge", FRCMD_LBUTTONUP, FrOptionDlg::gOnPowerGaugeBtnUp)
ON_FRESH_VI("g_sfullyproportionalpowergaugesize", FRCMD_INIT,
	FrOptionDlg::gOnFullyPropPowerGaugeStaticInit)
ON_FRESH_VI("g_fullyproportionalpowergaugesize", FRCMD_INIT,
	FrOptionDlg::gOnFullyPropPowerGaugeBtnInit)
ON_FRESH_VV("g_fullyproportionalpowergaugesize", FRCMD_LBUTTONUP,
	FrOptionDlg::gOnFullyPropPowerGaugeBtnUp)
ON_FRESH_VI("g_scenteredpowergauge", FRCMD_INIT,
	FrOptionDlg::gOnCenteredPowerGaugeStaticInit)
ON_FRESH_VI("g_centeredpowergauge", FRCMD_INIT,
	FrOptionDlg::gOnCenteredPowerGaugeBtnInit)
ON_FRESH_VV("g_centeredpowergauge", FRCMD_LBUTTONUP,
	FrOptionDlg::gOnCenteredPowerGaugeBtnUp)
ON_FRESH_VI("g_shidegui", FRCMD_INIT, FrOptionDlg::gOnHideGUIStaticInit)
ON_FRESH_VI("g_hidegui", FRCMD_INIT, FrOptionDlg::gOnHideGUIBtnInit)
ON_FRESH_VV("g_hidegui", FRCMD_LBUTTONUP, FrOptionDlg::gOnHideGUIBtnUp)
ON_FRESH_VI("g_shidepi", FRCMD_INIT, FrOptionDlg::gOnHidePIStaticInit)
ON_FRESH_VI("g_hidepi", FRCMD_INIT, FrOptionDlg::gOnHidePIBtnInit)
ON_FRESH_VV("g_hidepi", FRCMD_LBUTTONUP, FrOptionDlg::gOnHidePIBtnUp)
ON_FRESH_VI("g_stooltip", FRCMD_INIT, FrOptionDlg::gOnTerrainTooltipStaticInit)
ON_FRESH_VI("g_btntooltip", FRCMD_INIT, FrOptionDlg::gOnTerrainBtnInit)
ON_FRESH_VV("g_btntooltip", FRCMD_LBUTTONUP, FrOptionDlg::gOnTerrainBtnUp)
ON_FRESH_VI("g_cutin", FRCMD_INIT, FrOptionDlg::gOnCutinDisplayStaticInit)
ON_FRESH_VI("g_btncutin", FRCMD_INIT, FrOptionDlg::gOnCutinDisplayBtnInit)
ON_FRESH_VV("g_btncutin", FRCMD_LBUTTONUP, FrOptionDlg::gOnCutinDisplayBtnUp)
ON_FRESH_VI("v_sfullscr", FRCMD_INIT, FrOptionDlg::vOnFullScreenStaticInit)
ON_FRESH_VI("v_fullscr", FRCMD_INIT, FrOptionDlg::vOnFullScreenBtnInit)
ON_FRESH_VV("v_fullscr", FRCMD_LBUTTONUP, FrOptionDlg::vOnFullScreenBtnUp)
ON_FRESH_VI("v_sfillscr", FRCMD_INIT, FrOptionDlg::vOnFillScreenStaticInit)
ON_FRESH_VI("v_fillscr", FRCMD_INIT, FrOptionDlg::vOnFillScreenBtnInit)
ON_FRESH_VV("v_fillscr", FRCMD_LBUTTONUP, FrOptionDlg::vOnFillScreenBtnUp)
ON_FRESH_VI("v_swidemode", FRCMD_INIT, FrOptionDlg::vOnWideModeStaticInit)
ON_FRESH_VI("v_widemode", FRCMD_INIT, FrOptionDlg::vOnWideModeBtnInit)
ON_FRESH_VV("v_widemode", FRCMD_LBUTTONUP, FrOptionDlg::vOnWideModeBtnUp)
ON_FRESH_VI("g_smbindmode", FRCMD_INIT, FrOptionDlg::gOnBindMouseCursorStaticInit)
ON_FRESH_VI("g_mbindmode", FRCMD_INIT, FrOptionDlg::gOnBindMouseCursorBtnInit)
ON_FRESH_VV("g_mbindmode", FRCMD_LBUTTONUP, FrOptionDlg::gOnBindMouseCursorBtnUp)
ON_FRESH_VI("v_soverall", FRCMD_INIT, FrOptionDlg::vOnOverallStaticInit)
ON_FRESH_VI("v_overall", FRCMD_INIT, FrOptionDlg::vOnOverallCmbInit)
ON_FRESH_VI("v_sres", FRCMD_INIT, FrOptionDlg::vOnScrResStaticInit)
ON_FRESH_VI("v_res", FRCMD_INIT, FrOptionDlg::vOnScrResCmbInit)
ON_FRESH_VI("v_stex", FRCMD_INIT, FrOptionDlg::vOnTexResStaticInit)
ON_FRESH_VI("v_tex", FRCMD_INIT, FrOptionDlg::vOnTexResCmbInit)
ON_FRESH_VI("v_sshadow", FRCMD_INIT, FrOptionDlg::vOnShadowStaticInit)
ON_FRESH_VI("v_shadow", FRCMD_INIT, FrOptionDlg::vOnShadowBtnInit)
ON_FRESH_VV("v_shadow", FRCMD_LBUTTONUP, FrOptionDlg::vOnShadowBtnUp)
ON_FRESH_VI("v_ssdwmap", FRCMD_INIT, FrOptionDlg::vOnShadowmapStaticInit)
ON_FRESH_VI("v_sdwmap", FRCMD_INIT, FrOptionDlg::vOnShadowmapBtnInit)
ON_FRESH_VV("v_sdwmap", FRCMD_LBUTTONUP, FrOptionDlg::vOnShadowmapBtnUp)
ON_FRESH_VI("v_srefl", FRCMD_INIT, FrOptionDlg::vOnReflectionStaticInit)
ON_FRESH_VI("v_refl", FRCMD_INIT, FrOptionDlg::vOnReflectionBtnInit)
ON_FRESH_VV("v_refl", FRCMD_LBUTTONUP, FrOptionDlg::vOnReflectionBtnUp)
ON_FRESH_VI("v_snpc", FRCMD_INIT, FrOptionDlg::vOnNpcStaticInit)
ON_FRESH_VI("v_npc", FRCMD_INIT, FrOptionDlg::vOnNpcBtnInit)
ON_FRESH_VV("v_npc", FRCMD_LBUTTONUP, FrOptionDlg::vOnNpcBtnUp)
ON_FRESH_VI("v_sparticle", FRCMD_INIT, FrOptionDlg::vOnParticleStaticInit)
ON_FRESH_VI("v_particle", FRCMD_INIT, FrOptionDlg::vOnParticleCmbInit)
ON_FRESH_VI("a_senable", FRCMD_INIT, FrOptionDlg::aOnEnableStaticInit)
ON_FRESH_VI("a_enable", FRCMD_INIT, FrOptionDlg::aOnEnableBtnInit)
ON_FRESH_VV("a_enable", FRCMD_LBUTTONUP, FrOptionDlg::aOnEnableBtnUp)
ON_FRESH_VI("a_soverall", FRCMD_INIT, FrOptionDlg::aOnOverallStaticInit)
ON_FRESH_VI("a_overall", FRCMD_INIT, FrOptionDlg::aOnOverallCmbInit)
ON_FRESH_VI("a_ssfx", FRCMD_INIT, FrOptionDlg::aOnSfxStaticInit)
ON_FRESH_VI("a_sfx", FRCMD_INIT, FrOptionDlg::aOnSfxGaugeInit)
ON_FRESH_VI("a_sbgm", FRCMD_INIT, FrOptionDlg::aOnBgmStaticInit)
ON_FRESH_VI("a_bgm", FRCMD_INIT, FrOptionDlg::aOnBgmGaugeInit)
ON_FRESH_VI("a_sspeaker", FRCMD_INIT, FrOptionDlg::aOnSpeakerStaticInit)
ON_FRESH_VI("a_speaker", FRCMD_INIT, FrOptionDlg::aOnSpeakerCmbInit)
ON_FRESH_VI("m_smacro1", FRCMD_INIT, FrOptionDlg::mOnMacroStatic1Init)
ON_FRESH_VI("m_macro1", FRCMD_INIT, FrOptionDlg::mOnMacro1Init)
ON_FRESH_VV("m_macro1", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro1", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro2", FRCMD_INIT, FrOptionDlg::mOnMacroStatic2Init)
ON_FRESH_VI("m_macro2", FRCMD_INIT, FrOptionDlg::mOnMacro2Init)
ON_FRESH_VV("m_macro2", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro2", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro3", FRCMD_INIT, FrOptionDlg::mOnMacroStatic3Init)
ON_FRESH_VI("m_macro3", FRCMD_INIT, FrOptionDlg::mOnMacro3Init)
ON_FRESH_VV("m_macro3", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro3", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro4", FRCMD_INIT, FrOptionDlg::mOnMacroStatic4Init)
ON_FRESH_VI("m_macro4", FRCMD_INIT, FrOptionDlg::mOnMacro4Init)
ON_FRESH_VV("m_macro4", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro4", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro5", FRCMD_INIT, FrOptionDlg::mOnMacroStatic5Init)
ON_FRESH_VI("m_macro5", FRCMD_INIT, FrOptionDlg::mOnMacro5Init)
ON_FRESH_VV("m_macro5", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro5", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro6", FRCMD_INIT, FrOptionDlg::mOnMacroStatic6Init)
ON_FRESH_VI("m_macro6", FRCMD_INIT, FrOptionDlg::mOnMacro6Init)
ON_FRESH_VV("m_macro6", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro6", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro7", FRCMD_INIT, FrOptionDlg::mOnMacroStatic7Init)
ON_FRESH_VI("m_macro7", FRCMD_INIT, FrOptionDlg::mOnMacro7Init)
ON_FRESH_VV("m_macro7", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro7", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("m_smacro8", FRCMD_INIT, FrOptionDlg::mOnMacroStatic8Init)
ON_FRESH_VI("m_macro8", FRCMD_INIT, FrOptionDlg::mOnMacro8Init)
ON_FRESH_VV("m_macro8", FRCMD_LBUTTONDOWN,
	FrOptionDlg::mOnMacroEnterKeyLBtnDown)
ON_FRESH_BI("m_macro8", FRCMD_ENTERKEY, FrOptionDlg::mOnMacroEnterKey)
ON_FRESH_VI("s_shdr", FRCMD_INIT, FrOptionDlg::sOnHdrStaticInit)
ON_FRESH_VI("s_hdr", FRCMD_INIT, FrOptionDlg::sOnHdrBtnInit)
ON_FRESH_VV("s_hdr", FRCMD_LBUTTONUP, FrOptionDlg::sOnHdrBtnUp)
ON_FRESH_VI("s_sdof", FRCMD_INIT, FrOptionDlg::sOnDofStaticInit)
ON_FRESH_VI("s_dof", FRCMD_INIT, FrOptionDlg::sOnDofBtnInit)
ON_FRESH_VV("s_dof", FRCMD_LBUTTONUP, FrOptionDlg::sOnDofBtnUp)
ON_FRESH_VI("s_srimlight", FRCMD_INIT, FrOptionDlg::sOnRimLightStaticInit)
ON_FRESH_VI("s_rimlight", FRCMD_INIT, FrOptionDlg::sOnRimLightBtnInit)
ON_FRESH_VV("s_rimlight", FRCMD_LBUTTONUP, FrOptionDlg::sOnRimLightBtnUp)
ON_FRESH_VI("s_sAmbientControl", FRCMD_INIT,
	FrOptionDlg::sOnAmbientControlStaticInit)
ON_FRESH_VI("s_AmbientControl", FRCMD_INIT,
	FrOptionDlg::sOnAmbientControlBtnInit)
ON_FRESH_VV("s_AmbientControl", FRCMD_LBUTTONUP,
	FrOptionDlg::sOnAmbientControlBtnUp)
ON_FRESH_VI("s_sbright", FRCMD_INIT, FrOptionDlg::sOnBrightStaticInit)
ON_FRESH_VI("s_bright_gauge", FRCMD_INIT, FrOptionDlg::sOnBrightGaugeInit)
ON_FRESH_VI("s_shdrStarEffect", FRCMD_INIT,
	FrOptionDlg::sOnHdrStarEffectStaticInit)
ON_FRESH_VI("s_hdrStarEffect", FRCMD_INIT,
	FrOptionDlg::sOnHdrStarEffectBtnInit)
ON_FRESH_VV("s_hdrStarEffect", FRCMD_LBUTTONUP,
	FrOptionDlg::sOnHdrStarEffectBtnUp)
ON_FRESH_VI("s_sFxaa", FRCMD_INIT, FrOptionDlg::sOnFxaaStaticInit)
ON_FRESH_VI("s_Fxaa", FRCMD_INIT, FrOptionDlg::sOnFxaaBtnInit)
ON_FRESH_VV("s_Fxaa", FRCMD_LBUTTONUP, FrOptionDlg::sOnFxaaBtnUp)
ON_FRESH_VI("s_ShaderOnOff", FRCMD_INIT, FrOptionDlg::sOnShaderOnOffBtnInit)
ON_FRESH_VV("s_ShaderOnOff", FRCMD_LBUTTONUP,
	FrOptionDlg::sOnShaderOnOffBtnUp)
ON_FRESH_VI("t_hint", FRCMD_INIT, FrOptionDlg::tOnHintInit)
ON_FRESH_VI("", FRCMD_HOVERON, FrOptionDlg::OnHoverOn)
ON_FRESH_VI("", FRCMD_HOVEROFF, FrOptionDlg::OnHoverOff)

END_FRESH_MSGMAP()

FrOptionDlg::FrOptionDlg()
	: m_bApplyScrSize(false)
{
	m_option = *COption::Instance()->GetOption();
	m_bApply = false;
	m_bOK = false;
	m_pOkBtn = 0;
	m_pTab[TAB_GAME] = &m_gameTab;
	m_pTab[TAB_VIDEO] = &m_videoTab;
	m_pTab[TAB_AUDIO] = &m_audioTab;
	m_pTab[TAB_MACRO] = &m_macroTab;
	m_pTab[TAB_SHADER] = &m_shaderTab;

	m_pChangeNickDlg = 0;
	m_pSchoolDlg = 0;

	m_pHint = 0;
	m_hoverCount = 0;

	g_pFresh->GetManager()->SetExclusiveKey(true);
}

FrOptionDlg::~FrOptionDlg()
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (!CChatMsg::Instance()->IsOpened())
		{
			g_pFresh->GetManager()->SetExclusiveKey(false);
		}
	}
}

void FrOptionDlg::SetApplyScrSize(bool bApply)
{
	m_bApplyScrSize = bApply;
}

void FrOptionDlg::OpenTab(eTabType type)
{
	if (type < TAB_GAME || type >= TAB_NUM)
	{
		return;
	}

	SetTab(type);
}

void FrOptionDlg::OnOkBtnInit(int param)
{
	m_pOkBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::OnGameTabInit(int param)
{
	m_pTab[TAB_GAME]->m_pTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTab[TAB_GAME]->Init();
}

void FrOptionDlg::OnGameTabUp()
{
	SetTab(TAB_GAME);
}

void FrOptionDlg::OnVideoTabInit(int param)
{
	m_pTab[TAB_VIDEO]->m_pTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTab[TAB_VIDEO]->Init();
}

void FrOptionDlg::OnVideoTabUp()
{
	SetTab(TAB_VIDEO);
}

void FrOptionDlg::OnAudioTabInit(int param)
{
	m_pTab[TAB_AUDIO]->m_pTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTab[TAB_AUDIO]->Init();
}

void FrOptionDlg::OnAudioTabUp()
{
	SetTab(TAB_AUDIO);
}

void FrOptionDlg::OnMacroTabInit(int param)
{
	m_pTab[TAB_MACRO]->m_pTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTab[TAB_MACRO]->Init();
}

void FrOptionDlg::OnMacroTabUp()
{
	SetTab(TAB_MACRO);
}

void FrOptionDlg::sOnShaderTabInit(int param)
{
	m_pTab[TAB_SHADER]->m_pTabBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	m_pTab[TAB_SHADER]->Init();
}

void FrOptionDlg::sOnShaderTabUp()
{
	SetTab(TAB_SHADER);
}

void FrOptionDlg::gOnMsSensiStaticInit(int param)
{
	m_gameTab.m_pStatic[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnMsSensiGaugeInit(int param)
{
	m_gameTab.m_pMsSensiGauge = DYNAMIC_CAST(FrGaugeBarEx, (FrWnd*)param);
}

void FrOptionDlg::gOnWhisperStaticInit(int param)
{
	m_gameTab.m_pStatic[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnWhisperBtnInit(int param)
{
	m_gameTab.m_pWhisperBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnWhisperBtnUp()
{
	m_option.gWhisper =
		m_gameTab.m_pWhisperBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnInvitationStaticInit(int param)
{
	m_gameTab.m_pStatic[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnInvitationBtnInit(int param)
{
	m_gameTab.m_pInvitationBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnInvitationBtnUp()
{
	m_option.gInvitation =
		m_gameTab.m_pInvitationBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnFriendConfirmStaticInit(int param)
{
	m_gameTab.m_pStatic[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnFriendConfirmBtnInit(int param)
{
	m_gameTab.m_pFriendConfirmBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnFriendConfirmBtnUp()
{
	m_option.gFriendConfirm =
		m_gameTab.m_pFriendConfirmBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnTransChatWinStaticInit(int param)
{
	m_gameTab.m_pStatic[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnTransChatWinBtnInit(int param)
{
	m_gameTab.m_pTransChatWinBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnTransChatWinBtnUp()
{
	m_option.gTransChatWin =
		m_gameTab.m_pTransChatWinBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnUiEffectStaticInit(int param)
{
	m_gameTab.m_pStatic[5] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnTerrainTooltipStaticInit(int param)
{
	m_gameTab.m_pStatic[13] =DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnUiEffectBtnInit(int param)
{
	m_gameTab.m_pUiEffectBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnUiEffectBtnUp()
{
	m_option.gUiEffect =
		m_gameTab.m_pUiEffectBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnNickStaticInit(int param)
{
	m_gameTab.m_pStatic[6] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnNickEditInit(int param)
{
	m_gameTab.m_pNickEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_gameTab.m_pNickEdit->HidePrivacy(true);

	if (!m_gameTab.m_bEnableChangeNick)
	{
		m_gameTab.m_pNickEdit->SetReadOnly(true);
		m_gameTab.m_pNickEdit->SetFontColor(0xff888888);
	}
}

void FrOptionDlg::gOnChangeNickBtnInit(int param)
{
	m_gameTab.m_pChangeNickBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnChangeNickBtnUp()
{
	char nick[24];
	if (!m_gameTab.m_bEnableChangeNick)
		return;
	if (!m_gameTab.m_pNickEdit)
		return;
	if (m_pChangeNickDlg)
		return;

	if (m_bApply)
		m_bOK = false;

	strcpy(nick, m_gameTab.m_pNickEdit->GetLine(1, false));

	const char* err = Doc()->m_chatManager.FilteringNick(nick);
	if (err)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify", NULL);
		if (pForm)
		{
			pForm->SetMessage(err, false);
			pForm->Open(NULL, 3);
		}
		m_bApply = false;
		return;
	}

	const char* line = m_gameTab.m_pNickEdit->GetLine(1, false);
	cTokenV token;
	token.Init(line, strlen(line));
	int nToken = token.GetTokenNum(0, " '", 2) - 1;
	if (nToken > 0)
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify", NULL);
		if (pForm)
		{
			pForm->SetMessage(
				"[']\264\302 \264\353\310\255\270\355\277\241 \273\347\277\353\307\322 \274\366 \276\370\275\300\264\317\264\331.",
				false);
			pForm->Open(NULL, 3);
		}
	}
	else if (strcmpi(Doc()->m_myInfo.info.sNick, line) != 0)
	{
		m_pChangeNickDlg = CreateForm<FrChangeNickDlg>(
			g_pFresh->GetManager(), this, "changenick", NULL);
		if (m_pChangeNickDlg)
		{
			m_pChangeNickDlg->SetOptionDlg(this);
			m_pChangeNickDlg->SetNick(
				m_gameTab.m_pNickEdit->GetLine(1, false));
			m_pChangeNickDlg->SetCloseResult(FrNONE);
			m_pChangeNickDlg->Open((FRESH_PFN_RESULT)&FrOptionDlg::
									   OnChangeNickConfirmDlgResult,
				0);
		}
		EnableChangeNickControls(false);
	}
	else if (m_bApply)
	{
		CloseDlg();
	}
	else
	{
		FrForm* pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify", NULL);
		if (pForm)
		{
			pForm->SetMessage(
				K2L_Compatibility("\273\365\267\316\277\356 \264\353\310\255\270\355\300\273 \300\324\267\302\307\330 \301\326\274\274\277\344"),
				false);
			pForm->Open(NULL, 3);
		}
	}
}

bool FrOptionDlg::gOnNickEditEnterKey(int key)
{
	return false;
}

void FrOptionDlg::gOnSchoolStaticInit(int param)
{
	m_gameTab.m_pStatic[7] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnSchoolEditInit(int param)
{
	m_gameTab.m_pSchoolEdit = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_gameTab.m_pSchoolEdit->SetReadOnly(true);
}

void FrOptionDlg::gOnChangeSchoolBtnInit(int param)
{
	m_gameTab.m_pChangeSchoolBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	// The school can only be changed from the lobby, online.
	if (IS_KINDOF(CLobbyTask, AfxGetTask()) &&
		WSingleton<WNetworkSystem>::Instance()->IsConnected(
			(WNetworkSystem::eNetUnit)0))
	{
		m_gameTab.m_pChangeSchoolBtn->Enable(true);
	}
	else
	{
		m_gameTab.m_pChangeSchoolBtn->Enable(false);
	}
}

void FrOptionDlg::gOnChangeSchoolBtnUp()
{
	if (m_gameTab.m_pSchoolEdit && !m_pSchoolDlg)
	{
		m_pSchoolDlg = CreateForm<FrSchoolDlg>(g_pFresh->GetManager(), this,
			"search_school", 0);
		m_pSchoolDlg->Open(
			(FRESH_PFN_RESULT)(&FrOptionDlg::OnChangeSchoolDlgResult), 1);
	}
}

bool FrOptionDlg::gOnSchoolEditEnterKey(int key)
{
	return false;
}

void FrOptionDlg::gOnPowerGaugeStaticInit(int param)
{
	m_gameTab.m_pStatic[8] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnPowerGaugeBtnInit(int param)
{
	m_gameTab.m_pPowerGaugeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnPowerGaugeBtnUp()
{
	m_option.gPowerGauge =
		m_gameTab.m_pPowerGaugeBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnFullyPropPowerGaugeStaticInit(int param)
{
	m_gameTab.m_pStatic[9] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnFullyPropPowerGaugeBtnInit(int param)
{
	m_gameTab.m_pFullyPropPowerGaugeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnFullyPropPowerGaugeBtnUp()
{
	m_option.gFullyProportionalPowerGaugeSize =
		m_gameTab.m_pFullyPropPowerGaugeBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnCenteredPowerGaugeStaticInit(int param)
{
	m_gameTab.m_pStatic[10] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnCenteredPowerGaugeBtnInit(int param)
{
	m_gameTab.m_pCenteredPowerGaugeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnCenteredPowerGaugeBtnUp()
{
	m_option.gCenteredPowerGauge =
		m_gameTab.m_pCenteredPowerGaugeBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnHideGUIStaticInit(int param)
{
	m_gameTab.m_pStatic[11] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnHideGUIBtnInit(int param)
{
	m_gameTab.m_pHideGUIBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnHideGUIBtnUp()
{
	m_option.gCaptureHideGUI =
		m_gameTab.m_pHideGUIBtn->GetStatus() == FrButton::PRESSED;
	CProjectG::Instance()->ResetCaptuerNum();
}

void FrOptionDlg::gOnHidePIStaticInit(int param)
{
	m_gameTab.m_pStatic[12] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnHidePIBtnInit(int param)
{
	m_gameTab.m_pHidePIBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnTerrainBtnInit(int param)
{
	m_gameTab.m_pTerrainBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (COption::Instance()->gGetTerrainTooltip())
	{
		m_gameTab.m_pTerrainBtn->SetStatus(FrButton::PRESSED);
	}
}

void FrOptionDlg::gOnTerrainBtnUp()
{
}

void FrOptionDlg::gOnCutinDisplayStaticInit(int param)
{
	m_gameTab.m_pStatic[14] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnCutinDisplayBtnInit(int param)
{
	m_gameTab.m_pCutinDisplayBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnCutinDisplayBtnUp()
{
	m_option.gCutinDisplay =
		m_gameTab.m_pCutinDisplayBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnBindMouseCursorStaticInit(int param)
{
	m_gameTab.m_pStatic[15] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::gOnBindMouseCursorBtnInit(int param)
{
	m_gameTab.m_pBindMouseCursorBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::gOnBindMouseCursorBtnUp()
{
	m_option.gBindMouseCursor =
		m_gameTab.m_pBindMouseCursorBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::gOnHidePIBtnUp()
{
	m_option.gCaptureHidePI =
		m_gameTab.m_pHidePIBtn->GetStatus() == FrButton::PRESSED;
	CProjectG::Instance()->ResetCaptuerNum();
}

void FrOptionDlg::vOnFullScreenStaticInit(int param)
{
	m_videoTab.m_pStatic[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnFullScreenBtnInit(int param)
{
	m_videoTab.m_pFullScreenBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::vOnFullScreenBtnUp()
{
	m_option.vWindowMode =
		m_videoTab.m_pFullScreenBtn->GetStatus() != FrButton::PRESSED;

	char cur[16];
	strcpy(cur,
		COption::Instance()->vGetScrResText(m_option.vScreenWidth,
			m_option.vScreenHeight, m_option.vScreenColor,
			m_option.vWindowMode != 0));

	COption::Instance()->CheckDisplayModeList(g_resrcmng->VideoReference(),
		m_option.vWindowMode != 0, m_option.vWideMode != 0);

	m_videoTab.m_pScrResCmb->ClearList();

	bool bFirst = true;
	for (int i = 0; i < COption::Instance()->vGetScrResNum(); i++)
	{
		const char* text = COption::Instance()->vGetScrResText(i);
		if (strlen(text))
		{
			m_videoTab.m_pScrResCmb->AddString(text);
			if (bFirst)
			{
				bFirst = false;
				m_videoTab.m_pScrResCmb->SetLine(1, text, 0, false, 0);
			}
			if (strcmp(text, cur) == 0)
				m_videoTab.m_pScrResCmb->SetLine(1, text, 0, false, 0);
		}
	}

	m_gameTab.m_pBindMouseCursorBtn->Enable(m_option.vWindowMode ? true : false); // JP 983
}

void FrOptionDlg::vOnFillScreenStaticInit(int param)
{
	m_videoTab.m_pStatic[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnFillScreenBtnInit(int param)
{
	m_videoTab.m_pFillScreenBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::vOnFillScreenBtnUp()
{
	if (m_videoTab.m_pFillScreenBtn &&
		!m_videoTab.m_pFillScreenBtn->IsEnabled())
		return;

	m_option.vFillMode =
		m_videoTab.m_pFillScreenBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::vOnWideModeStaticInit(int param)
{
	m_videoTab.m_pStatic[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnWideModeBtnInit(int param)
{
	m_videoTab.m_pWideModeBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::vOnWideModeBtnUp()
{
	m_option.vWideMode =
		m_videoTab.m_pWideModeBtn->GetStatus() == FrButton::PRESSED;

	char cur[16];
	strcpy(cur,
		COption::Instance()->vGetScrResText(m_option.vScreenWidth,
			m_option.vScreenHeight, m_option.vScreenColor,
			m_option.vWindowMode != 0));

	COption::Instance()->CheckDisplayModeList(g_resrcmng->VideoReference(),
		m_option.vWindowMode != 0, m_option.vWideMode != 0);

	m_videoTab.m_pScrResCmb->ClearList();

	bool bFirst = true;
	for (int i = 0; i < COption::Instance()->vGetScrResNum(); i++)
	{
		const char* text = COption::Instance()->vGetScrResText(i);
		if (strlen(text))
		{
			m_videoTab.m_pScrResCmb->AddString(text);
			if (bFirst)
			{
				bFirst = false;
				m_videoTab.m_pScrResCmb->SetLine(1, text, 0, false, 0);
			}
			if (strcmp(text, cur) == 0)
				m_videoTab.m_pScrResCmb->SetLine(1, text, 0, false, 0);
		}
	}
}

void FrOptionDlg::vOnOverallStaticInit(int param)
{
	m_videoTab.m_pStatic[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnOverallCmbInit(int param)
{
	m_videoTab.m_pOverallCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrOptionDlg::vOnScrResStaticInit(int param)
{
	m_videoTab.m_pStatic[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnScrResCmbInit(int param)
{
	m_videoTab.m_pScrResCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}



void FrOptionDlg::vOnTexResStaticInit(int param)
{
	m_videoTab.m_pStatic[6] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnTexResCmbInit(int param)
{
	m_videoTab.m_pTexResCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrOptionDlg::vOnShadowStaticInit(int param)
{
	m_videoTab.m_pStatic[7] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);

	if (!sOption::IsSupportShadow())
	{
		m_videoTab.m_pStatic[7]->SetTextColor(0xff808080, 0xffffffff);
	}
}

void FrOptionDlg::vOnShadowBtnInit(int param)
{
	m_videoTab.m_pShadowBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (!sOption::IsSupportShadow())
	{
		m_videoTab.m_pShadowBtn->Enable(false);
	}
}

void FrOptionDlg::vOnShadowBtnUp()
{
	m_option.vProjectionShadow =
		m_videoTab.m_pShadowBtn->GetStatus() == FrButton::PRESSED;
	m_option.vOverall = -1;
	m_videoTab.m_pOverallCmb->SetLine(1,
		m_option.vGetOverallText(m_option.vOverall), 0, false, 0);
}

void FrOptionDlg::vOnShadowmapStaticInit(int param)
{
	m_videoTab.m_pStatic[8] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnShadowmapBtnInit(int param)
{
	m_videoTab.m_pShadowmapBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::vOnShadowmapBtnUp()
{
	m_option.vShadowmap =
		m_videoTab.m_pShadowmapBtn->GetStatus() == FrButton::PRESSED;
	m_option.vOverall = -1;
	m_videoTab.m_pOverallCmb->SetLine(1,
		m_option.vGetOverallText(m_option.vOverall), 0, false, 0);
}

void FrOptionDlg::vOnReflectionStaticInit(int param)
{
	m_videoTab.m_pStatic[9] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);

	if (!sOption::IsSupportReflection())
	{
		m_videoTab.m_pStatic[9]->SetTextColor(0xff808080, 0xffffffff);
	}
}

void FrOptionDlg::vOnReflectionBtnInit(int param)
{
	m_videoTab.m_pReflectionBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (!sOption::IsSupportReflection())
	{
		m_videoTab.m_pReflectionBtn->Enable(false);
	}
}

void FrOptionDlg::vOnReflectionBtnUp()
{
	m_option.vReflection =
		m_videoTab.m_pReflectionBtn->GetStatus() == FrButton::PRESSED;
	m_option.vOverall = -1;
	m_videoTab.m_pOverallCmb->SetLine(1,
		m_option.vGetOverallText(m_option.vOverall), 0, false, 0);
}




void FrOptionDlg::vOnNpcStaticInit(int param)
{
	m_videoTab.m_pStatic[11] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnNpcBtnInit(int param)
{
	m_videoTab.m_pNpcBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::vOnNpcBtnUp()
{
	m_option.vNpc = m_videoTab.m_pNpcBtn->GetStatus() == FrButton::PRESSED;
	m_option.vOverall = -1;
	m_videoTab.m_pOverallCmb->SetLine(1,
		m_option.vGetOverallText(m_option.vOverall), 0, false, 0);
}

void FrOptionDlg::vOnParticleStaticInit(int param)
{
	m_videoTab.m_pStatic[12] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::vOnParticleCmbInit(int param)
{
	m_videoTab.m_pParticleCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrOptionDlg::aOnEnableStaticInit(int param)
{
	m_audioTab.m_pStatic[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::aOnEnableBtnInit(int param)
{
	m_audioTab.m_pEnableBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrOptionDlg::aOnEnableBtnUp()
{
	if (m_audioTab.m_pEnableBtn->GetStatus() == FrButton::PRESSED)
	{
		m_option.aMssOn = 1;
		g_audio->SetVolume(m_option.aSfxVolume);
		g_audio->SetBGMVolume(m_option.aBGMVolume);
		COption::Instance()->Opt983().aMssOn = 1;	// COption::m_opt.aMssOn (983 008cb560)
	}
	else
	{
		m_option.aMssOn = 0;
		g_audio->SetVolume(0);
		g_audio->SetBGMVolume(0);
		COption::Instance()->Opt983().aMssOn = 0;	// COption::m_opt.aMssOn (983 008cb560)
	}
}

void FrOptionDlg::aOnOverallStaticInit(int param)
{
	m_audioTab.m_pStatic[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::aOnOverallCmbInit(int param)
{
	m_audioTab.m_pOverallCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrOptionDlg::aOnSfxStaticInit(int param)
{
	m_audioTab.m_pStatic[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::aOnSfxGaugeInit(int param)
{
	m_audioTab.m_pSfxGauge = DYNAMIC_CAST(FrGaugeBarEx, (FrWnd*)param);
}

void FrOptionDlg::aOnBgmStaticInit(int param)
{
	m_audioTab.m_pStatic[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::aOnBgmGaugeInit(int param)
{
	m_audioTab.m_pBgmGauge = DYNAMIC_CAST(FrGaugeBarEx, (FrWnd*)param);
}

void FrOptionDlg::aOnSpeakerStaticInit(int param)
{
	m_audioTab.m_pStatic[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::aOnSpeakerCmbInit(int param)
{
	m_audioTab.m_pSpeakerCmb = DYNAMIC_CAST(FrComboBox, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic1Init(int param)
{
	m_macroTab.m_pStatic[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic2Init(int param)
{
	m_macroTab.m_pStatic[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

bool FrOptionDlg::mOnMacroEnterKey(int key)
{
	OnOK();
	Close(FrOK, true);
	return false;
}

void FrOptionDlg::mOnMacroEnterKeyLBtnDown()
{
	OnLButtonDown(g_pFresh->GetManager()->GetMousePos());
}

void FrOptionDlg::mOnMacroStatic3Init(int param)
{
	m_macroTab.m_pStatic[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic4Init(int param)
{
	m_macroTab.m_pStatic[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic5Init(int param)
{
	m_macroTab.m_pStatic[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic6Init(int param)
{
	m_macroTab.m_pStatic[5] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic7Init(int param)
{
	m_macroTab.m_pStatic[6] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacroStatic8Init(int param)
{
	m_macroTab.m_pStatic[7] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro1Init(int param)
{
	m_macroTab.m_pMacro[0] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro2Init(int param)
{
	m_macroTab.m_pMacro[1] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro3Init(int param)
{
	m_macroTab.m_pMacro[2] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro4Init(int param)
{
	m_macroTab.m_pMacro[3] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro5Init(int param)
{
	m_macroTab.m_pMacro[4] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro6Init(int param)
{
	m_macroTab.m_pMacro[5] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro7Init(int param)
{
	m_macroTab.m_pMacro[6] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrOptionDlg::mOnMacro8Init(int param)
{
	m_macroTab.m_pMacro[7] = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

bool FrOptionDlg::OnInit()
{
	SetGameColtrols();
	InitVideoControls();
	SetVideoColtrols(true);
	SetAudioColtrols();
	SetMacroColtrols();
	SetShaderColtrols();

	eTabType curTab = ms_eCurTab;

	ms_eCurTab = TAB_NUM;
	SetTab(curTab);
	m_pTab[ms_eCurTab]->m_pTabBtn->SetStatus(FrButton::PRESSED);

	// JP 983: macros de ate 63 caracteres
	for (int i = 0; i < 8; i++)
		m_macroTab.m_pMacro[i]->SetCharLimit(63, true);

	return FrForm::OnInit();
}

bool FrOptionDlg::OnLButtonDown(const WPoint& pt)
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		if (g_input->GetButton(LEFT_BUTTON) == 1)
		{
			g_pFresh->GetManager()->SetExclusiveKey(true);
			CMessengerInfo::Instance()->SetGameFocus(false);
		}
	}

	return FrForm::OnLButtonDown(pt);
}

void FrOptionDlg::OnProc(const float deltaTime)
{
	if (g_input->GetDown("TAB", true) && !g_input->Get("LALT", true) &&
		!g_input->Get("RALT", true))
	{
		switch (ms_eCurTab)
		{
		case TAB_GAME:
			SetTab(TAB_VIDEO);
			break;
		case TAB_VIDEO:
			SetTab(TAB_AUDIO);
			break;
		case TAB_AUDIO:
			SetTab(TAB_MACRO);
			break;
		case TAB_MACRO:
			SetTab(TAB_SHADER);
			break;
		case TAB_SHADER:
			SetTab(TAB_GAME);
			break;
		}
	}

	if (m_gameTab.m_pTabBtn && m_gameTab.m_pTabBtn->IsEnabled() &&
		m_gameTab.m_pMsSensiGauge)
	{
		float pos = m_gameTab.m_pMsSensiGauge->GetPos();
		float sens;

		if (pos > 50.0f)
		{
			sens = pos / 50.0; // 983 008cdd45: fdiv
		}
		else
		{
			sens = pos / 100.0 + 0.5;
		}

		if (!WisEqual(m_option.gMouseSensitivity, sens, g_EPSILON))
		{
			m_option.gMouseSensitivity = sens;
			g_input->SetSensitivity(sens);
		}
	}

	if (m_videoTab.m_pTabBtn && m_videoTab.m_pTabBtn->IsEnabled())
	{
		bool bOverall = false;
		bool bRes = false;
		bool bAny = false;

		if (m_videoTab.m_pOverallCmb)
		{
			const char* text = m_option.vGetOverallText(m_option.vOverall);
			const char* line = m_videoTab.m_pOverallCmb->GetLine(1, false);

			if (strcmp(text, line) != 0)
			{
				m_option.vResetByOverall(line);
				bOverall = true;
			}
		}

		if (!bOverall && m_videoTab.m_pScrResCmb)
		{
			const char* text = COption::Instance()->vGetScrResText(
				m_option.vScreenWidth, m_option.vScreenHeight,
				m_option.vScreenColor, m_option.vWindowMode != 0);
			const char* line = m_videoTab.m_pScrResCmb->GetLine(1, false);

			if (strcmp(text, line) != 0)
			{
				COption::Instance()->vGetScrResLvl(line, &m_option.vScreenWidth,
					&m_option.vScreenHeight, &m_option.vScreenColor);
				bRes = true;
				bAny = true;
			}
		}

		if (m_videoTab.m_pFillScreenBtn)
		{
			if (m_option.vWindowMode == 0)
			{
				m_videoTab.m_pFillScreenBtn->Enable(false);
			}
			else
			{
				m_videoTab.m_pFillScreenBtn->Enable(true);

				if (m_option.vFillMode)
				{
					m_gameTab.m_pPowerGaugeBtn->Enable(false);
				}
				else
				{
					m_gameTab.m_pPowerGaugeBtn->Enable(true);
				}
			}
		}

		m_gameTab.m_pBindMouseCursorBtn->Enable(m_option.vWindowMode != 0); // JP 983

		if (!bOverall)
		{
			if (m_videoTab.m_pTexResCmb)
			{
				const char* text = m_option.vGetTexResText(m_option.vDDSRes);
				const char* line = m_videoTab.m_pTexResCmb->GetLine(1, false);

				if (strcmp(text, line) != 0)
				{
					m_option.vDDSRes = m_option.vGetTexResLvl(line);
					bRes = true;
					bAny = true;
				}
			}

			if (m_videoTab.m_pParticleCmb)
			{
				const char* text =
					m_option.vGetParticleText(m_option.vEffectLevel);
				const char* line = m_videoTab.m_pParticleCmb->GetLine(1, false);

				if (strcmp(text, line) != 0)
				{
					m_option.vEffectLevel = m_option.vGetParticleLvl(line);
					bAny = true;
					bRes = true;
				}
			}
		}

		if (bOverall)
		{
			SetVideoColtrols(false);
		}
		else if (bRes)
		{
			SetVideoColtrols(false);

			if (bAny)
			{
				m_option.vOverall = -1;
				m_videoTab.m_pOverallCmb->SetLine(1,
					m_option.vGetOverallText(-1), 0, false, 0);
			}
		}
	}

	if (m_audioTab.m_pTabBtn && m_audioTab.m_pTabBtn->IsEnabled())
	{
		if (m_audioTab.m_pSfxGauge)
		{
			float vol = m_audioTab.m_pSfxGauge->GetPos() / 100.0; // 983 008ce0c6: fdiv; 0.01f perdia 1 ao reabrir

			if (!WisEqual(m_option.aSfxVolume, vol, g_EPSILON))
			{
				m_option.aSfxVolume = vol;

				if (m_option.aMssOn == 1)
				{
					COption::Instance()->aApplySfxVolume(vol);
				}
			}
		}

		if (m_audioTab.m_pBgmGauge)
		{
			float vol = m_audioTab.m_pBgmGauge->GetPos() / 100.0;

			if (!WisEqual(m_option.aBGMVolume, vol, g_EPSILON))
			{
				m_option.aBGMVolume = vol;

				if (m_option.aMssOn == 1)
				{
					COption::Instance()->aApplyBgmVolume(vol);
				}
			}
		}

		if (m_audioTab.m_pSpeakerCmb)
		{
			const char* text = m_option.aGetSpeakerText(m_option.aMssSpeaker);
			const char* line = m_audioTab.m_pSpeakerCmb->GetLine(1, false);

			if (strcmp(text, line) != 0)
			{
				m_option.aMssSpeaker = m_option.aGetSpeakerLvl(line);
			}
		}
	}

	// JP 983: brilho da aba "shader"
	if (m_shaderTab.m_pTabBtn && m_shaderTab.m_pTabBtn->IsEnabled() &&
		m_shaderTab.m_pBrightGauge)
	{
		float bright = m_shaderTab.m_pBrightGauge->GetPos() / 25.0;

		if (!WisEqual(m_option.vfBright, bright, g_EPSILON))
		{
			m_option.vfBright = bright;
		}
	}
}

void FrOptionDlg::SetGameColtrols()
{
	float pos = m_option.gMouseSensitivity;
	if (pos > 1.0f)
		pos = pos * 50.0f;
	else
		pos = (pos - 0.5f) * 100.0f;
	m_gameTab.m_pMsSensiGauge->SetPos((int)Between(0.0f, pos, 100.0f));

	if (m_option.gWhisper && m_gameTab.m_pWhisperBtn)
		m_gameTab.m_pWhisperBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gInvitation && m_gameTab.m_pInvitationBtn)
		m_gameTab.m_pInvitationBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gFriendConfirm && m_gameTab.m_pFriendConfirmBtn)
		m_gameTab.m_pFriendConfirmBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gTransChatWin && m_gameTab.m_pTransChatWinBtn)
		m_gameTab.m_pTransChatWinBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gUiEffect && m_gameTab.m_pUiEffectBtn)
		m_gameTab.m_pUiEffectBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gPowerGauge && m_gameTab.m_pPowerGaugeBtn)
		m_gameTab.m_pPowerGaugeBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gFullyProportionalPowerGaugeSize &&
		m_gameTab.m_pFullyPropPowerGaugeBtn)
		m_gameTab.m_pFullyPropPowerGaugeBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gCenteredPowerGauge && m_gameTab.m_pCenteredPowerGaugeBtn)
		m_gameTab.m_pCenteredPowerGaugeBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gCaptureHideGUI && m_gameTab.m_pHideGUIBtn)
		m_gameTab.m_pHideGUIBtn->SetStatus(FrButton::PRESSED);
	if (m_option.gCaptureHidePI && m_gameTab.m_pHidePIBtn)
		m_gameTab.m_pHidePIBtn->SetStatus(FrButton::PRESSED);

	if (m_gameTab.m_pNickEdit && Doc()->m_myInfo.info.sNick[0])
		m_gameTab.m_pNickEdit->SetLine(1, Doc()->m_myInfo.info.sNick, 0, false,
			0);

	if (m_option.gCutinDisplay && m_gameTab.m_pCutinDisplayBtn)
		m_gameTab.m_pCutinDisplayBtn->SetStatus(FrButton::PRESSED);

	// JP 983: prender o cursor so vale em janela
	if (m_option.gBindMouseCursor && m_gameTab.m_pBindMouseCursorBtn)
		m_gameTab.m_pBindMouseCursorBtn->SetStatus(FrButton::PRESSED);
	m_gameTab.m_pBindMouseCursorBtn->Enable(m_option.vWindowMode ? true : false);

	if (m_gameTab.m_pSchoolEdit && Doc()->m_myInfo.info.school)
	{
		char name[128];
		Doc()->LoadSchoolName(name, Doc()->m_myInfo.info.school);
		m_gameTab.m_pSchoolEdit->SetLine(1, name, 0, false, 0);
	}
}

void FrOptionDlg::InitVideoControls()
{
	int i;

	for (i = 0; i < 3; i++)
	{
		const char* text = m_option.vGetOverallText(i);
		m_videoTab.m_pOverallCmb->AddString(text);
	}

	COption::Instance()->CheckDisplayModeList(g_resrcmng->VideoReference(),
		m_option.vWindowMode != 0, m_option.vWideMode != 0);
	m_videoTab.m_pScrResCmb->ClearList();

	for (i = 0; i < COption::Instance()->vGetScrResNum(); i++)
	{
		const char* text = COption::Instance()->vGetScrResText(i);

		if (strlen(text))
		{
			m_videoTab.m_pScrResCmb->AddString(text);
		}
	}

	for (i = 0; i < 3; i++)
	{
		const char* text = m_option.vGetTexResText(i);
		m_videoTab.m_pTexResCmb->AddString(text);
	}

	for (i = 0; i < 3; i++)
	{
		const char* text = m_option.vGetParticleText(i);
		m_videoTab.m_pParticleCmb->AddString(text);
	}
}

void FrOptionDlg::SetVideoColtrols(bool bReset)
{
	if (!m_option.vWindowMode)
		m_videoTab.m_pFullScreenBtn->SetStatus(FrButton::PRESSED);
	if (m_option.vFillMode)
		m_videoTab.m_pFillScreenBtn->SetStatus(FrButton::PRESSED);
	if (m_option.vWideMode)
		m_videoTab.m_pWideModeBtn->SetStatus(FrButton::PRESSED);

	if (bReset)
		m_option.vResetByOverall(m_option.vGetOverallText(m_option.vOverall));

	const char* text1 = m_option.vGetOverallText(m_option.vOverall);
	m_videoTab.m_pOverallCmb->SetLine(1, text1, 0, false, 0);
	const char* text2 = COption::Instance()->vGetScrResText(
		m_option.vScreenWidth, m_option.vScreenHeight, m_option.vScreenColor,
		m_option.vWindowMode != 0);
	m_videoTab.m_pScrResCmb->SetLine(1, text2, 0, false, 0);
	const char* text4 = m_option.vGetTexResText(m_option.vDDSRes);
	m_videoTab.m_pTexResCmb->SetLine(1, text4, 0, false, 0);

	if (sOption::IsSupportShadow())
		m_videoTab.m_pShadowBtn->SetStatus(m_option.vProjectionShadow == 1
				? FrButton::PRESSED
				: FrButton::NORMAL);
	m_videoTab.m_pShadowmapBtn->SetStatus(
		m_option.vShadowmap == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	if (sOption::IsSupportReflection())
		m_videoTab.m_pReflectionBtn->SetStatus(
			m_option.vReflection == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_videoTab.m_pNpcBtn->SetStatus(
		m_option.vNpc == 1 ? FrButton::PRESSED : FrButton::NORMAL);

	const char* text5 = m_option.vGetParticleText(m_option.vEffectLevel);
	m_videoTab.m_pParticleCmb->SetLine(1, text5, 0, false, 0);
}

void FrOptionDlg::SetAudioColtrols()
{
	if (m_option.aMssOn)
	{
		m_audioTab.m_pEnableBtn->SetStatus(FrButton::PRESSED);
	}

	for (int i = 0; i < 3; i++)
	{
		m_audioTab.m_pOverallCmb->AddString(m_option.aGetOverallText(i));
	}

	for (int i = 0; i < 6; i++)
	{
		m_audioTab.m_pSpeakerCmb->AddString(m_option.aGetSpeakerText(i));
	}

	const char* text6 = m_option.aGetOverallText(m_option.aOverall);
	m_audioTab.m_pOverallCmb->SetLine(1, text6, 0, false, 0);

	const char* text7 = m_option.aGetSpeakerText(m_option.aMssSpeaker);
	m_audioTab.m_pSpeakerCmb->SetLine(1, text7, 0, false, 0);

	m_audioTab.m_pSfxGauge->SetPos(
		Between(0, (int)(m_option.aSfxVolume * 100.0f), 100));
	m_audioTab.m_pBgmGauge->SetPos(
		Between(0, (int)(m_option.aBGMVolume * 100.0f), 100));
}

void FrOptionDlg::SetMacroColtrols()
{
	for (int i = 0; i < 8; i++)
	{
		Doc()->m_chatManager.FilteringHack(m_option.mMacro[i], false);
		m_macroTab.m_pMacro[i]->SetLine(1, m_option.mMacro[i], 0, false, 0);
	}
}

// JP 983 (008cce00)
void FrOptionDlg::SetShaderColtrols()
{
	if (!IsSupportShader983())
		return;

	m_shaderTab.m_pHdrBtn->SetStatus(
		m_option.viHdr == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_shaderTab.m_pDofBtn->SetStatus(
		m_option.viDof == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_shaderTab.m_pAmbientControlBtn->SetStatus(
		m_option.viAmbientControl == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_shaderTab.m_pRimLightBtn->SetStatus(
		m_option.viRimLight == 1 ? FrButton::PRESSED : FrButton::NORMAL);

	float bright = m_option.vfBright * 25.0;
	int pos = (int)bright;
	if (pos < 0)
		pos = 0;
	else if (pos > 100)
		pos = 100;
	m_shaderTab.m_pBrightGauge->SetPos(pos);

	m_shaderTab.m_pHdrStarEffectBtn->SetStatus(
		m_option.viHdrStarEffect == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	m_shaderTab.m_pFxaaBtn->SetStatus(
		m_option.viFXAA == 1 ? FrButton::PRESSED : FrButton::NORMAL);
}

void FrOptionDlg::SetTab(eTabType type)
{
	// HACK: emission order: the original defers this function past the PCH inlines
	if (0)
	{
		g_pFresh->GetManager();
	}

	if (m_hoverCount)
	{
		m_hoverCount = 0;
	}

	if (type < TAB_GAME || type >= TAB_NUM || type == ms_eCurTab)
	{
		return;
	}

	ms_eCurTab = type;
	g_audio->PlaySfx("ui_button_ok_click", 0, 0, 0, 0, 0.5f, 200.0f);

	for (int i = 0; i < TAB_NUM; i++)
	{
		m_pTab[i]->SetSelected(i == type);
		m_pTab[i]->m_pTabBtn->SetStatus(
			i == type ? FrButton::PRESSED : FrButton::NORMAL);

		if (!g_audio->GetMSS())
		{
			if (type == TAB_AUDIO)
			{
				m_pHint->ClearLine();
				// "No audio device is available."
				m_pHint->AddText(
					K2L_Compatibility("\273\347\277\353\307\322 \274\366 \300\326\264\302 \277\300\265\360\277\300 \300\345\304\241\260\241 \276\370\275\300\264\317\264\331."),
					false, true);
				m_pHint->SetVisible(true);
			}
			else
			{
				m_pHint->SetVisible(false);
			}
		}
	}

	// JP 983: sem troca de apelido
	if (type == TAB_GAME)
	{
		if (m_gameTab.m_pStatic[6])
			m_gameTab.m_pStatic[6]->SetVisible(false);
		if (m_gameTab.m_pNickEdit)
			m_gameTab.m_pNickEdit->SetVisible(false);
		if (m_gameTab.m_pChangeNickBtn)
			m_gameTab.m_pChangeNickBtn->SetVisible(false);
	}
}

void FrOptionDlg::OnCancel()
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()) && !CChatMsg::Instance()->IsOpened())
	{
		g_pFresh->GetManager()->SetExclusiveKey(true);

		CMessengerInfo::Instance()->SetGameFocus(true);
	}

	g_input->SetSensitivity(
		Between(0.5f, COption::Instance()->gGetMouseSensitivity(), 2.0f));
	if (m_audioTab.m_pEnableBtn->GetStatus() == FrButton::PRESSED)
	{
		g_audio->SetBGMVolume(COption::Instance()->aGetBgmVolume());
		g_audio->SetVolume(COption::Instance()->aGetSfxVolume());
	}
	else
	{
		g_audio->SetVolume(0);
		g_audio->SetBGMVolume(0);
	}

	FrForm::OnCancel();
}

void FrOptionDlg::OnOK()
{
	if (IS_KINDOF(CGolfTask, AfxGetTask()) && !CChatMsg::Instance()->IsOpened())
	{
		g_pFresh->GetManager()->SetExclusiveKey(true);

		CMessengerInfo::Instance()->SetGameFocus(true);
	}

	m_bApply = true;
	m_bOK = true;

	CloseDlg();
}

extern HWND g_hwnd; // 00f0245c

void FrOptionDlg::CloseDlg()
{
	if (m_gameTab.m_pTerrainBtn->GetStatus() == FrButton::PRESSED)
		m_option.gTerrainTooltip = 1;
	else
		m_option.gTerrainTooltip = 0;
	CMouseCursor::Instance()->SetTerrainToolTip(m_option.gTerrainTooltip != 0);

	if (!m_bApply)
	{
		EnableChangeNickControls(true);
		return;
	}

	for (int i = 0; i < 8; i++)
	{
		strncpy(m_option.mMacro[i], m_macroTab.m_pMacro[i]->GetLine(1, false),
			63);
		m_option.mMacro[i][63] = '\0';
		Doc()->m_chatManager.FilteringHack(m_option.mMacro[i], true);
	}

	if (g_mouse)
		g_mouse->InitDevice(g_hwnd, m_option.gBindMouseCursor == 1 || m_option.vWindowMode == 0);

	// JP 983 (008ce6a0): cursor do sistema conforme prender/janela
	if (g_pMouseCursorHelper983)
	{
		CMouseCursorHelper* h = g_pMouseCursorHelper983;
		if (m_option.gBindMouseCursor == 1 || m_option.vWindowMode == 0)
		{
			::SetCursor(h->m_hCursor);
			h->m_showCount = ::ShowCursor(TRUE);
			if (h->m_showCount > 0)
			{
				do
					h->m_showCount = ::ShowCursor(FALSE);
				while (h->m_showCount != 0);
			}
			else if (h->m_showCount < 0)
				h->ShowCountFix(1);
		}
		else
		{
			h->m_showCount = ::ShowCursor(FALSE);
			if (h->m_showCount < 0)
			{
				if (h->m_showCount < -1)
					h->HideCountFix(1);
			}
			else
			{
				do
					h->m_showCount = ::ShowCursor(FALSE);
				while (h->m_showCount != -1);
			}
		}
	}

	COption::Instance()->ApplyChange(m_option, m_bApplyScrSize);

	struct sMacroInfo
	{
		char macro[9][64];
	};

	sMacroInfo info;
	memset(&info, 0, sizeof(info));
	for (int i = 0; i < 8; i++)
		strcpy(info.macro[i], m_option.mMacro[i]);

	WSendPacket packet(0x69);
	packet.EncodeBuffer(&info, sizeof(info));
	packet.Send(TO_GAME);

	FrForm::OnOK();
}

void FrOptionDlg::EnableChangeNickControls(bool bEnable)
{
	if (m_gameTab.m_pChangeNickBtn && m_gameTab.m_pChangeNickBtn->IsVisible())
	{
		m_gameTab.m_pChangeNickBtn->Enable(bEnable);
	}

	if (m_pOkBtn && m_pOkBtn->IsVisible())
	{
		m_pOkBtn->Enable(bEnable);
	}
}

void FrOptionDlg::sOnShaderOnOffBtnInit(int param)
{
	m_shaderTab.m_pShaderOnOffBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pShaderOnOffBtn->Enable(false);
}

void FrOptionDlg::sOnHdrStaticInit(int param)
{
	m_shaderTab.m_pStatic[0] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[0]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnHdrBtnInit(int param)
{
	m_shaderTab.m_pHdrBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pHdrBtn->Enable(false);
}

void FrOptionDlg::sOnHdrBtnUp()
{
	m_option.viHdr = m_shaderTab.m_pHdrBtn->GetStatus() == FrButton::PRESSED;
	m_shaderTab.m_pHdrStarEffectBtn->Enable(m_option.viHdr == 1);
}

void FrOptionDlg::sOnHdrStarEffectStaticInit(int param)
{
	m_shaderTab.m_pStatic[1] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[1]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnHdrStarEffectBtnInit(int param)
{
	m_shaderTab.m_pHdrStarEffectBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pHdrStarEffectBtn->Enable(false);
}

void FrOptionDlg::sOnHdrStarEffectBtnUp()
{
	m_option.viHdrStarEffect = m_shaderTab.m_pHdrStarEffectBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::sOnDofStaticInit(int param)
{
	m_shaderTab.m_pStatic[2] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[2]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnDofBtnInit(int param)
{
	m_shaderTab.m_pDofBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pDofBtn->Enable(false);
}

void FrOptionDlg::sOnDofBtnUp()
{
	m_option.viDof = m_shaderTab.m_pDofBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::sOnAmbientControlStaticInit(int param)
{
	m_shaderTab.m_pStatic[3] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[3]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnAmbientControlBtnInit(int param)
{
	m_shaderTab.m_pAmbientControlBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pAmbientControlBtn->Enable(false);
}

void FrOptionDlg::sOnAmbientControlBtnUp()
{
	m_option.viAmbientControl = m_shaderTab.m_pAmbientControlBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::sOnRimLightStaticInit(int param)
{
	m_shaderTab.m_pStatic[4] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[4]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnRimLightBtnInit(int param)
{
	m_shaderTab.m_pRimLightBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pRimLightBtn->Enable(false);
}

void FrOptionDlg::sOnRimLightBtnUp()
{
	m_option.viRimLight = m_shaderTab.m_pRimLightBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::sOnFxaaStaticInit(int param)
{
	m_shaderTab.m_pStatic[5] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[5]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnFxaaBtnInit(int param)
{
	m_shaderTab.m_pFxaaBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pFxaaBtn->Enable(false);
}

void FrOptionDlg::sOnFxaaBtnUp()
{
	m_option.viFXAA = m_shaderTab.m_pFxaaBtn->GetStatus() == FrButton::PRESSED;
}

void FrOptionDlg::sOnBrightStaticInit(int param)
{
	m_shaderTab.m_pStatic[6] = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pStatic[6]->SetTextColor(0xff808080, 0xffffffff);
}

void FrOptionDlg::sOnBrightGaugeInit(int param)
{
	m_shaderTab.m_pBrightGauge = DYNAMIC_CAST(FrGaugeBarEx, (FrWnd*)param);
	if (!IsSupportShader983())
		m_shaderTab.m_pBrightGauge->Enable(false);
}

// JP 983 (008cdba0): liga/desliga todos os efeitos de uma vez
void FrOptionDlg::sOnShaderOnOffBtnUp()
{
	m_option.viShaderOnOff = m_option.viShaderOnOff != 1;

	bool bOn = m_option.viShaderOnOff == 1;
	m_option.viRimLight = bOn;
	m_option.viAmbientControl = bOn;
	m_option.viDof = bOn;
	m_option.viHdr = bOn;
	m_option.viHdrStarEffect = bOn;
	m_option.viFXAA = bOn;

	SetShaderColtrols();
}

void FrOptionDlg::tOnHintInit(int param)
{
	m_pHint = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	m_pHint->SetVisible(false);
}

const char* FrOptionDlg::MakeHint(eTabType tab, int index) const
{
	// HACK: emission order: the original defers this function past the PCH inlines
	if (0)
	{
		g_pFresh->GetManager();
	}

	switch (tab)
	{
	case TAB_GAME:
		switch (index)
		{
		case 0:
			return K2L_Compatibility("\270\266\277\354\275\272 \260\250\265\265\270\246 \301\266\300\375\307\325\264\317\264\331.");
		case 1:
			return K2L_Compatibility("\273\363\264\353\271\346\300\314 \261\323\270\273\300\273 \272\270\263\276 \274\366 \300\326\265\265\267\317 \307\343\277\353\307\325\264\317\264\331.");
		case 2:
			return K2L_Compatibility("\273\363\264\353\271\346\300\307 \303\312\264\353\270\246 \307\343\277\353\307\325\264\317\264\331.");
		case 3:
			return K2L_Compatibility("\273\363\264\353\271\346\300\307 \304\243\261\270 \265\356\267\317\277\344\303\273\277\241 \300\332\265\277\300\270\267\316 \265\277\300\307\307\325\264\317\264\331.");
		case 4:
			return K2L_Compatibility("\264\353\310\255\307\245\275\303\277\265\277\252 \271\331\305\301\300\273 \305\365\270\355\307\317\260\324 \307\325\264\317\264\331.");
		case 5:
			return K2L_Compatibility("\303\242\300\273 \277\255\260\305\263\252 \264\335\300\273 \266\247 \276\326\264\317\270\336\300\314\274\307 \310\277\260\372\270\246 \301\335\264\317\264\331.");
		case 6:
			return K2L_Compatibility("\264\353\310\255\270\355\300\273 \272\257\260\346\307\325\264\317\264\331.");
		case 7:
			return K2L_Compatibility("\307\320\261\263 \267\251\305\267\277\241 \273\347\277\353\265\311 \274\322\274\323 \307\320\261\263\270\246 \272\257\260\346\307\325\264\317\264\331.");
		case 8:
			return K2L_Compatibility("\307\342\273\363\265\310 \303\326\275\305\300\307 \305\270\261\270\271\331\270\246 \273\347\277\353\307\325\264\317\264\331(\261\307\300\345).");
		case 9:
			return K2L_Compatibility("\310\255\270\351\300\307 \274\274\267\316 \307\330\273\363\265\265\277\241 \272\361\267\312\307\317\277\251 \305\270\261\270\271\331\270\246 \310\256\264\353\307\325\264\317\264\331.");
		case 10:
			return K2L_Compatibility("\305\270\261\270\271\331\270\246 \310\255\270\351 \301\337\276\323\277\241 \307\245\275\303\307\325\264\317\264\331.");
		case 11:
			return K2L_Compatibility("\304\270\303\304\275\303 GUI \272\316\272\320\300\273 \260\250\303\337\260\355 \300\372\300\345\307\325\264\317\264\331.");
		case 12:
			return K2L_Compatibility("\304\270\303\304\275\303 \260\263\300\316\301\244\272\270\270\246 \260\250\303\337\260\355 \300\372\300\345\307\325\264\317\264\331.");
		case 13:
			return K2L_Compatibility("\301\366\270\351\274\323\274\272\305\370\306\301\300\273 \304\321\260\305\263\252 \262\374\264\317\264\331.");
		case 14:
			return K2L_Compatibility("\304\306\300\316 \300\314\271\314\301\366\260\241 \260\324\300\323\301\337\277\241 \276\310\272\270\300\314\265\265\267\317 \307\325\264\317\264\331.");
		case 15:
			return K2L_Compatibility("\303\242 \270\360\265\345\277\241\274\255 \270\266\277\354\275\272 \304\277\274\255\270\246 \260\241\265\323\264\317\264\331.");
		}
		break;
	case TAB_VIDEO:
		switch (index)
		{
		case 0:
			return K2L_Compatibility("\306\316\276\337\270\246 \300\374\303\274\310\255\270\351\300\270\267\316 \301\361\261\346 \274\366 \300\326\275\300\264\317\264\331.");
		case 1:
			return K2L_Compatibility("\306\316\276\337\270\246 \300\374\303\274\303\242 \310\255\270\351\300\270\267\316 \301\361\261\346 \274\366 \300\326\275\300\264\317\264\331.");
		case 2:
			return K2L_Compatibility("\306\316\276\337\270\246 \277\315\300\314\265\345 \310\255\270\351\277\241\274\255 \301\361\261\346 \274\366 \300\326\275\300\264\317\264\331.");
		case 5:
			return K2L_Compatibility("\261\327\267\241\307\310 \303\263\270\256\271\346\275\304\300\273 \274\261\305\303\307\317\275\312\275\303\277\300(\300\347\275\303\300\333\307\312\277\344).");
		case 3:
			return K2L_Compatibility("\277\251\267\257\260\241\301\366 \272\361\265\360\277\300 \274\263\301\244\300\273 \307\321\262\250\271\370\277\241 \271\331\262\334 \274\366 \300\326\275\300\264\317\264\331.");
		case 4:
			return K2L_Compatibility("\310\255\270\351\300\307 \307\330\273\363\265\265\270\246 \260\341\301\244\307\325\264\317\264\331.");
		case 6:
			return K2L_Compatibility("\300\373\301\244\307\321 \261\327\267\241\307\310 \274\274\271\320\265\265\270\246 \274\261\305\303\307\317\275\312\275\303\277\300(\300\347\275\303\300\333\307\312\277\344).");
		case 7:
			return K2L_Compatibility("\306\316\276\337 \301\326\300\316\260\370\265\351\300\307 \261\327\270\262\300\332\270\246 \304\321\260\305\263\252 \262\374\264\317\264\331.");
		case 8:
			return K2L_Compatibility("\263\252\271\253\263\252 \260\307\271\260 \265\356\300\307 \261\327\270\262\300\332\270\246 \304\321\260\305\263\252 \262\374\264\317\264\331.");
		case 9:
			return K2L_Compatibility("\271\260\307\245\270\351\300\307 \271\335\273\347 \310\277\260\372\270\246 \273\347\277\353\307\325\264\317\264\331.");
		case 10:
			return K2L_Compatibility("LOD \261\342\264\311\300\273 \310\260\274\272\310\255 \275\303\305\265\264\317\264\331.");
		case 11:
			return K2L_Compatibility("\306\316\276\337\274\266 \260\367\260\367\300\307 \300\333\300\272 \304\243\261\270\265\351\300\273 \270\270\263\252\272\270\275\303\260\332\275\300\264\317\261\356?");
		case 12:
			return K2L_Compatibility("\277\251\267\257\260\241\301\366 \306\257\274\366\310\277\260\372\265\351\300\307 \307\260\301\372\300\273 \260\341\301\244\307\325\264\317\264\331.");
		}
		break;
	case TAB_AUDIO:
		switch (index)
		{
		case 0:
			return K2L_Compatibility("\270\360\265\347 \274\322\270\256\270\246 \304\321\260\305\263\252 \262\374\264\317\264\331.");
		case 1:
			return K2L_Compatibility("\300\373\301\244\307\321 \300\275\301\372\300\273 \274\261\305\303\307\317\275\312\275\303\277\300.");
		case 2:
			return K2L_Compatibility("\310\277\260\372\300\275\300\307 \272\274\267\375\300\273 \301\266\300\375\307\325\264\317\264\331.");
		case 3:
			return K2L_Compatibility("\271\350\260\346\300\275\276\307\300\307 \272\274\267\375\300\273 \301\266\300\375\307\325\264\317\264\331.");
		case 4:
			return K2L_Compatibility("\275\272\307\307\304\277 \305\270\300\324\300\273 \260\341\301\244\307\325\264\317\264\331.");
		}
		break;
	case TAB_MACRO:
		return NULL;
	case TAB_SHADER: // JP 983: textos do GameStringManager
		switch (index)
		{
		case 0:
			return GetGameString(90001);
		case 1:
			return GetGameString(90002);
		case 2:
			return GetGameString(90003);
		case 3:
			return GetGameString(90004);
		case 4:
			return GetGameString(90005);
		case 5:
			return GetGameString(90006);
		case 6:
			return GetGameString(90007);
		}
		break;
	}
	return NULL;
}

void FrOptionDlg::ProcessHoverMessage(FrWnd* pWnd)
{
	if (!m_pHint || !m_pTab[ms_eCurTab]->m_bSelected)
	{
		return;
	}

	if (m_hoverCount > 0)
	{
		if (pWnd)
		{
			for (int i = 0; i < m_pTab[ms_eCurTab]->m_num; i++)
			{
				if (*m_pTab[ms_eCurTab]->m_pppCtrl[i] == pWnd)
				{
					const char* hint = MakeHint(ms_eCurTab, i);
					if (hint)
					{
						m_pHint->ClearLine();
						m_pHint->AddText(hint, false, true);
						m_pHint->SetVisible(true);
					}
				}
			}
		}
	}
	else if (m_hoverCount == 0)
	{
		m_pHint->SetVisible(false);
	}
}

void FrOptionDlg::OnHoverOn(int param)
{
	m_hoverCount++;
	ProcessHoverMessage(DYNAMIC_CAST(FrWnd, (FrWnd*)param));
}

void FrOptionDlg::OnHoverOff(int param)
{
	m_hoverCount--;
	ProcessHoverMessage(0);
}

bool FrOptionDlg::OnChangeNickConfirmDlgResult(int result, FrForm* pForm)
{
	m_pChangeNickDlg = 0;
	EnableChangeNickControls(true);

	return true;
}

bool FrOptionDlg::OnChangeSchoolDlgResult(int result, FrForm* form)
{
	if (result == FrOK && m_pSchoolDlg &&
		Doc()->m_myInfo.info.school != m_pSchoolDlg->m_schoolCode)
	{
		char name[1024];
		strcpy(name, m_pSchoolDlg->m_schoolName.c_str());
		m_gameTab.m_pSchoolEdit->SetLine(1, name, 0, false, 0);

		WSendPacket packet((enumClientPacket)0x3b);
		packet.Encode4(m_pSchoolDlg->m_schoolCode);
		packet.Send(TO_GAME);

		if (IActor* pActor = AfxGetTask()->GetMainActor())
			pActor << MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0);
	}
	m_pSchoolDlg = NULL;
	return true;
}

// JP 983: posicoes medidas no construtor 008c9070 (sOption copiado para +0x114; abas em +0x460/+0x548/+0x610/+0x670)
struct FrOptionDlgLayout983 : public FrOptionDlg
{
	static void Check()
	{
	C_ASSERT_VC10(sizeof(FrOptionDlg) == 0x7a0);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_bApplyScrSize) == 0x110);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_option) == 0x114);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_gameTab) == 0x460);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_videoTab) == 0x548);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_audioTab) == 0x610);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_macroTab) == 0x670);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_shaderTab) == 0x6f4);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_pTab) == 0x778);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_pChangeNickDlg) == 0x78c);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_pHint) == 0x794);
	C_ASSERT_VC10(offsetof(FrOptionDlgLayout983, m_hoverCount) == 0x798);
	C_ASSERT(sizeof(sGameTab) == 0xe8);
	C_ASSERT(offsetof(sGameTab, m_ppCtrl) == 0x5c);
	C_ASSERT(offsetof(sGameTab, m_pMsSensiGauge) == 0x9c);
	C_ASSERT(offsetof(sGameTab, m_pFullyPropPowerGaugeBtn) == 0xc8);
	C_ASSERT(offsetof(sGameTab, m_pBindMouseCursorBtn) == 0xe0);
	C_ASSERT(offsetof(sGameTab, m_bEnableChangeNick) == 0xe4);
	C_ASSERT(sizeof(sShaderTab) == 0x84);
	C_ASSERT(offsetof(sShaderTab, m_bDisable) == 0x5c);
	C_ASSERT(offsetof(sShaderTab, m_pHdrBtn) == 0x64);
	C_ASSERT(offsetof(sShaderTab, m_pShaderOnOffBtn) == 0x80);
	}
};
