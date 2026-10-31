#include "minatl.h"
#include "caddieinfodlg.h"
#include "actor.h"
#include "projectg.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrCaddieInfoDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrCaddieInfoDlg, FrForm)

ON_FRESH_VI("portrait", FRCMD_INIT, FrCaddieInfoDlg::OnPortraitInit)
ON_FRESH_VI("vacation", FRCMD_INIT, FrCaddieInfoDlg::OnVacationInit)
ON_FRESH_VI("limit", FRCMD_INIT, FrCaddieInfoDlg::OnLimitInit)
ON_FRESH_VI("name", FRCMD_INIT, FrCaddieInfoDlg::OnNameInit)
ON_FRESH_VI("contract", FRCMD_INIT, FrCaddieInfoDlg::OnContractInit)
ON_FRESH_VI("caddiefee", FRCMD_INIT, FrCaddieInfoDlg::OnCaddieFeeInit)
ON_FRESH_VI("signup", FRCMD_INIT, FrCaddieInfoDlg::OnSignUpInit)
ON_FRESH_VV("signup", FRCMD_LBUTTONUP, FrCaddieInfoDlg::OnSignUpBtnUp)
ON_FRESH_VI("cancel", FRCMD_INIT, FrCaddieInfoDlg::OnCancelInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrCaddieInfoDlg::OnCancelBtnUp)
ON_FRESH_VI("ok", FRCMD_INIT, FrCaddieInfoDlg::OnOkInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrCaddieInfoDlg::OnOkBtnUp)
ON_FRESH_VI("warning_button", FRCMD_INIT, FrCaddieInfoDlg::OnWarningButtonInit)
ON_FRESH_VI("warning_text", FRCMD_INIT, FrCaddieInfoDlg::OnWarningTextInit)

END_FRESH_MSGMAP()

FrCaddieInfoDlg::FrCaddieInfoDlg()
{
	memset(&m_caddieInfo, 0, sizeof(m_caddieInfo));
	m_pPortrait = NULL;
	m_pVacation = NULL;
	m_pLimit = NULL;
	m_pName = NULL;
	m_pMessage = NULL;
	m_pContract = NULL;
	m_pCaddieFee = NULL;
	m_pSignUp = NULL;
	m_pOk = NULL;
	m_pCancel = NULL;
	m_pWarningButton = NULL;
	m_pWarningText = NULL;
	m_pContractDlg = NULL;
}

void FrCaddieInfoDlg::OnPortraitInit(int param)
{
	m_pPortrait = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnVacationInit(int param)
{
	m_pVacation = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnLimitInit(int param)
{
	m_pLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnNameInit(int param)
{
	m_pName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnContractInit(int param)
{
	m_pContract = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnCaddieFeeInit(int param)
{
	m_pCaddieFee = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnSignUpInit(int param)
{
	m_pSignUp = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnOkInit(int param)
{
	m_pOk = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOk)
		m_pOk->SetVisible(false);
}

void FrCaddieInfoDlg::OnCancelInit(int param)
{
	m_pCancel = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnWarningButtonInit(int param)
{
	m_pWarningButton = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrCaddieInfoDlg::OnWarningTextInit(int param)
{
	m_pWarningText = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
}

void FrCaddieInfoDlg::HideContractControls()
{
	if (m_pVacation)
		m_pVacation->SetVisible(false);
	if (m_pMessage)
		m_pMessage->SetVisible(false);
	if (m_pContract)
		m_pContract->SetVisible(false);
	if (m_pCaddieFee)
		m_pCaddieFee->SetVisible(false);
	if (m_pSignUp)
		m_pSignUp->SetVisible(false);
	if (m_pLimit)
		m_pLimit->SetVisible(false);
	if (m_pCancel)
		m_pCancel->SetVisible(false);
	if (m_pOk)
		m_pOk->SetVisible(true);
	SetDesc("");
}

void FrCaddieInfoDlg::EndContract(int result)
{
	if (result == 2)
	{
		m_caddieInfo.Remain_Date = 30;
		HideContractControls();
		if (m_pOk && m_pOk->IsVisible())
			m_pOk->Enable(true);
	}
	else
	{
		if (m_pSignUp && m_pSignUp->IsVisible())
			m_pSignUp->Enable(true);
		if (m_pCancel && m_pCancel->IsVisible())
			m_pCancel->Enable(true);
	}
}

bool FrCaddieInfoDlg::OnInit()
{
	if (!m_caddieInfo.tid)
		return false;
	IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(m_caddieInfo.tid);
	bool bInfo = false;
	if (m_pPortrait)
	{
		if (pCaddie)
		{
			bInfo = true;
			m_pPortrait->SetBgImg(pCaddie->c.Icon);
		}
		else
			m_pPortrait->SetBgImg("hide");
	}
	if (bInfo && m_pName)
	{
		char text[256];
		sprintf(text, K2L_Compatibility("\300\314  \270\247: %s"), pCaddie->c.Name);
		unsigned char level;
		if ((level = m_caddieInfo.Level) <= 3 && pCaddie->c.InStock != 5)
		{
			unsigned long exp = 0, currentExp = 0;
			if (level < 3)
			{
				exp = Doc()->m_bonusPangTable[level].exp;
				currentExp = m_caddieInfo.Exp;
			}
			strcat(text,
				MakeStr(K2L_Compatibility("\n\267\271  \272\247: %d / %d ( %d / %d )"), level + 1,
					4, currentExp, exp));
		}
		if (m_caddieInfo.Rent_flag && pCaddie->MonthlyFee > 0)
		{
			if (m_caddieInfo.Remain_Date > 0)
				strcat(text,
					MakeStr(
						K2L_Compatibility("\n\263\262\300\272 \260\355\277\353\261\342\260\243: %d\300\317"),
						m_caddieInfo.Remain_Date));
			else
				strcat(text, K2L_Compatibility("\n\310\336\260\241\301\337"));
		}
		if (m_caddieInfo.tidPart & 0x1fff)
		{
			IFF_STRUCT::sCadItem* pPart =
				ItemManager()->FindCadItem(m_caddieInfo.tidPart);
			strcat(text,
				MakeStr(
					K2L_Compatibility("\n\276\367\261\327\267\271\300\314\265\345: \\c0xffff0000\\c%s\\c0xff000000\\c"),
					pPart->c.Name));
			if (m_caddieInfo.Remain_Partdate > 24)
				strcat(text,
					MakeStr(
						K2L_Compatibility("\n\304\263\265\360\262\331\271\314\261\342 \263\262\300\272 \261\342\260\243: \\c0xffff0000\\c%d\300\317\\c0xff000000\\c"),
						m_caddieInfo.Remain_Partdate / 24));
			else
				strcat(text,
					MakeStr(
						K2L_Compatibility("\n\304\263\265\360\262\331\271\314\261\342 \263\262\300\272 \261\342\260\243: \\c0xffff0000\\c%d\275\303\260\243\\c0xff000000\\c"),
						m_caddieInfo.Remain_Partdate));
			if (m_caddieInfo.Rent_flag && pCaddie->MonthlyFee > 0 &&
				m_caddieInfo.Remain_Date > 0)
				HideContractControls();
		}
		m_pName->AddText(text, false, true);
		if (m_caddieInfo.Remain_Date > 0 &&
				(m_caddieInfo.gift_flag || m_caddieInfo.Rent_flag) &&
				pCaddie->MonthlyFee > 0 ||
			pCaddie->MonthlyFee == 0 || m_caddieInfo.guid == -1)
		{
			IFF_STRUCT::sDesc* pDesc =
				ItemManager()->FindDesc(pCaddie->c.TypeId);
			if (pDesc)
				SetMessage(pDesc->Desc, false);
		}
		else
			SetMessage(
				MakeStr(
					K2L_Compatibility("\307\366\300\347 \304\263\265\360\264\302 \310\336\260\241\301\337\300\314\271\307\267\316 \304\263\265\360 \262\331\271\314\261\342\270\246 \307\322 \274\366 \276\370\275\300\264\317\264\331. %d \306\316\300\273 \301\366\261\336\307\317\277\251 \304\263\265\360\270\246 \310\336\260\241\272\271\261\315\275\303\305\260\275\303\260\332\275\300\264\317\261\356?"),
					pCaddie->MonthlyFee),
				false);
	}
	else
	{
		if (m_pName)
			m_pName->AddText(
				K2L_Compatibility("\300\314\270\247: \272\322\270\355\n\n\267\271\272\247: \272\322\270\355"),
				false, true);
		SetMessage(
			K2L_Compatibility("\272\243\300\317\277\241 \275\316\300\316 \301\244\303\274\272\322\270\355\300\307 \301\270\300\347"),
			false);
	}
	if (pCaddie)
	{
		if (m_caddieInfo.Rent_flag && pCaddie->MonthlyFee > 0)
		{
			m_pWarningButton->SetVisible(true);
			m_pWarningText->SetVisible(true);
		}
		else
		{
			m_pWarningButton->SetVisible(false);
			m_pWarningText->SetVisible(false);
		}
		if (m_caddieInfo.Rent_flag && !m_caddieInfo.Remain_Date &&
			pCaddie->MonthlyFee > 0 && m_pCaddieFee)
		{
			char fee[64];
			sprintf(fee, K2L_Compatibility("%d \306\316"), pCaddie->MonthlyFee);
			m_pCaddieFee->SetLine(1, fee, 0, 0, false);
			SetDesc(
				K2L_Compatibility("'\301\366\261\336' \271\366\306\260\300\273 \264\255\267\257 \304\263\265\360\270\246 \300\347\260\355\277\353 \307\322 \274\366 \300\326\275\300\264\317\264\331."));
		}
		else
			HideContractControls();
		if (Doc()->m_myInfo.stat.Level < (pCaddie->c.Level & 0x7f)) // 00a9a1d0: Level & 0x7f
		{
			if (m_pLimit)
				m_pLimit->SetVisible(true);
			SetDesc(
				K2L_Compatibility("\267\271\272\247 \301\246\307\321\300\270\267\316 \273\347\277\353\307\322 \274\366 \276\370\264\302 \304\263\265\360\300\324\264\317\264\331."));
		}
	}
	return true;
}

void FrCaddieInfoDlg::SetCaddieInfo(const sCaddieInfo& info)
{
	m_caddieInfo = Doc()->m_caddieMap[info.guid];
	if (m_caddieInfo.byCheckCaddieWarning == 1)
		m_pWarningButton->SetStatus(FrButton::PRESSED);
	else
		m_pWarningButton->SetStatus(FrButton::NORMAL);
	CaddieIterator it = ItemManager()->m_CaddieMap.begin();
	CaddieIterator end = ItemManager()->m_CaddieMap.end();
	for (; it != end; ++it)
	{
		if ((*it).second.c.TypeId == m_caddieInfo.tid)
		{
			if (((*it).second.c.Level & 0x7f) > Doc()->m_myInfo.stat.Level) // 00a9a6c0: (byte)Level & 0x7f
				m_pSignUp->Enable(false);
			break;
		}
	}
}

void FrCaddieInfoDlg::SendCaddieWarningCheckOption()
{
	if (m_pWarningButton->GetStatus() == FrButton::PRESSED)
	{
		Doc()->m_caddieMap[m_caddieInfo.guid].byCheckCaddieWarning = 1;
		m_caddieInfo.byCheckCaddieWarning = 1;
	}
	else
	{
		Doc()->m_caddieMap[m_caddieInfo.guid].byCheckCaddieWarning = 0;
		m_caddieInfo.byCheckCaddieWarning = 0;
	}
	WSendPacket send((enumClientPacket)0x6b);
	send.Encode4(m_caddieInfo.guid);
	send.Encode1(m_caddieInfo.byCheckCaddieWarning);
	send.Send(TO_GAME);
}

void FrCaddieInfoDlg::OnOkBtnUp()
{
	SendCaddieWarningCheckOption();
	Close(FrOK, true);
}
void FrCaddieInfoDlg::OnCancelBtnUp()
{
	SendCaddieWarningCheckOption();
	Close(FrCANCEL, true);
}

bool FrCaddieInfoDlg::OnSignUpConfirmDlgResult(int result, FrForm* form)
{
	m_pContractDlg = NULL;
	if (result == 1 || result == 2)
		SendCaddieWarningCheckOption();
	if (result == 1)
	{
		WSendPacket send((enumClientPacket)57);
		send.Encode4(m_caddieInfo.guid);
		send.Send(TO_GAME);
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		g_audio->PlaySfx("caddie_return");
		Close(FrOK, true);
	}
	else
	{
		if (m_pSignUp && m_pSignUp->IsVisible())
			m_pSignUp->Enable(true);
		if (m_pCancel && m_pCancel->IsVisible())
			m_pCancel->Enable(true);
	}
	return true;
}

void FrCaddieInfoDlg::OnSignUpBtnUp()
{
	FrStatic* pFee;
	if (m_pSignUp)
		m_pSignUp->Enable(false);
	if (m_pCancel)
		m_pCancel->Enable(false);
	IFF_STRUCT::sCaddie* pCaddie = ItemManager()->FindCaddie(m_caddieInfo.tid);
	if (!pCaddie)
		return;
	if (!m_pContractDlg)
		m_pContractDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"cad_contract", NULL);
	m_pContractDlg->SetMessage(
		K2L_Compatibility("\304\263\265\360\270\246 \300\347\260\355\277\353\307\317\270\351 \276\306\267\241\277\315 \260\260\300\314 \261\335\276\327\300\314 \\c0xffff0000\\c\260\250\274\322\\c0xff000000\\c\307\325\264\317\264\331.\n\n\304\263\265\360\270\246 \\c0xffff0000\\c\307\321\264\336\260\243 \\c0xff000000\\c\300\347\260\355\277\353 \307\317\275\303\260\332\275\300\264\317\261\356?"),
		false);
	pFee =
		DYNAMIC_CAST(FrStatic, (FrWnd*)m_pContractDlg->FindChildByName("fee"));
	if (pFee)
		pFee->SetCaption(MakeStr("%d", pCaddie->MonthlyFee));
	pFee = DYNAMIC_CAST(FrStatic,
		(FrWnd*)m_pContractDlg->FindChildByName("mypang"));
	if (pFee)
		pFee->SetCaption(MakeStr("%I64d", Doc()->m_myInfo.stat.i64Pang));
	pFee = DYNAMIC_CAST(FrStatic,
		(FrWnd*)m_pContractDlg->FindChildByName("remainpang"));
	if (pFee)
		pFee->SetCaption(MakeStr("%I64d",
			Doc()->m_myInfo.stat.i64Pang - pCaddie->MonthlyFee));
	m_pContractDlg->Open(
		(FRESH_PFN_RESULT)&FrCaddieInfoDlg::OnSignUpConfirmDlgResult, 0x801); // 00a9aa10: flags 0x801; 0xfffffffc,0,0 do decomp = pfn de 16 bytes (MI), nao pos
}
