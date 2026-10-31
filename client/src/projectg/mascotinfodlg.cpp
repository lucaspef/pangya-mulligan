#include "minatl.h"
#include "../shared/sharedtables.h"
#include "mascotinfodlg.h"
#include "mascot.h"
#include "logininfo.h"
#include "jp/shareddocjp.h"
#include "actor.h"
#include "../shared/token.h"
#include "../shared/localize.h"

extern Fresh* g_pFresh;

IMPLEMENT_OBJECT(FrMascotInfoDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrMascotInfoDlg, FrForm)

ON_FRESH_VI("portrait", FRCMD_INIT, FrMascotInfoDlg::OnPortraitInit)
ON_FRESH_VI("vacation", FRCMD_INIT, FrMascotInfoDlg::OnVacationInit)
ON_FRESH_VI("limit", FRCMD_INIT, FrMascotInfoDlg::OnLimitInit)
ON_FRESH_VI("name", FRCMD_INIT, FrMascotInfoDlg::OnNameInit)
ON_FRESH_VI("text_message", FRCMD_INIT, FrMascotInfoDlg::OnTextMessageInit)
ON_FRESH_VI("mascot_msg", FRCMD_INIT, FrMascotInfoDlg::OnMascotMsgInit)
ON_FRESH_VI("ok", FRCMD_INIT, FrMascotInfoDlg::OnOkInit)
ON_FRESH_VV("ok", FRCMD_LBUTTONUP, FrMascotInfoDlg::OnOkBtnUp)
ON_FRESH_VI("cancel", FRCMD_INIT, FrMascotInfoDlg::OnCancelInit)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrMascotInfoDlg::OnCancelBtnUp)

END_FRESH_MSGMAP()

FrMascotInfoDlg::FrMascotInfoDlg()
{
	memset(&m_mascotInfo, 0, sizeof(m_mascotInfo));
	m_pPortrait = NULL;
	m_pVacation = NULL;
	m_pLimit = NULL;
	m_pName = NULL;
	m_pMascotMsg = NULL;
	m_pOk = NULL;
	m_pCancel = NULL;
	m_pConfirmDlg = NULL;
	m_pPuppet = NULL;
}

void FrMascotInfoDlg::OnPortraitInit(int param)
{
	m_pPortrait = DYNAMIC_CAST(FrArea, (FrWnd*)param);
}

void FrMascotInfoDlg::OnVacationInit(int param)
{
	m_pVacation = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pVacation)
		m_pVacation->SetVisible(false);
}

void FrMascotInfoDlg::OnLimitInit(int param)
{
	m_pLimit = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	if (m_pLimit)
		m_pLimit->SetVisible(false);
}

void FrMascotInfoDlg::OnNameInit(int param)
{
	m_pName = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
}

void FrMascotInfoDlg::OnTextMessageInit(int param)
{
	m_pTextMessage = DYNAMIC_CAST(FrStatic, (FrWnd*)param);
	if (m_pTextMessage)
		m_pTextMessage->SetVisible(false);
}

void FrMascotInfoDlg::OnMascotMsgInit(int param)
{
	m_pMascotMsg = DYNAMIC_CAST(FrEdit, (FrWnd*)param);
	if (m_pMascotMsg)
		m_pMascotMsg->SetVisible(false);
}

void FrMascotInfoDlg::OnOkInit(int param)
{
	m_pOk = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pOk)
		m_pOk->Enable(false);
}

void FrMascotInfoDlg::OnCancelInit(int param)
{
	m_pCancel = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrMascotInfoDlg::SetMascotInfo(const sMascotInfo& info, WPuppet* pPuppet)
{
	m_mascotInfo = info;
	m_message = m_mascotInfo.szMsg;
	m_pPuppet = pPuppet;
	// 983 (00a81710): mensagem nao editavel -> OK ja habilitado (sem teste de NULL)
	if (m_mascotInfo.tid != 0x40000000 && m_mascotInfo.tid != 0x40000004 && m_mascotInfo.tid != 0x40000005 &&
		m_mascotInfo.tid != 0x40000008 && m_mascotInfo.tid != 0x4000000f)
		m_pOk->Enable(true);
}

void FrMascotInfoDlg::OnCancelBtnUp()
{
	if (strcmp(m_mascotInfo.szMsg, m_message.c_str()))
	{
		IFF_STRUCT::sMascot* pMascot =
			ItemManager()->FindMascot(m_mascotInfo.tid);
		if (m_pPuppet && pMascot)
			CMascot::ChangeMascotMsgInTex(m_mascotInfo.szMsg, pMascot,
				m_pPuppet,
				MakeStr("myroom%02d", (pMascot->c.TypeId & 0x3ffffff)));
	}
	OnFreshCancel();
}

bool FrMascotInfoDlg::OnChangeMsgConfirmDlgResult(int result, FrForm* pForm)
{
	m_pConfirmDlg = NULL;
	if (result == 1)
	{
		// JP 983 (00a806fc, C-PKT lote 10): this+0x14d = PCBangMascot (+0x3d do sMascotInfo)
		if (CLoginInfo::Instance()->IsPcBang() && m_mascotInfo.PCBangMascot)
		{
			WSendPacket send((enumClientPacket)0x9a);
			send.Encode1(1);
			send.Encode4(m_mascotInfo.guid);
			send.EncodeStr(m_message);
			send.Send(TO_GAME);
		}
		else
		{
			WSendPacket send((enumClientPacket)115);
			send.Encode4(m_mascotInfo.guid);
			send.EncodeStr(m_message);
			send.Send(TO_GAME);
		}
		// 983 (00a80795): pela tarefa ao ator principal (vtable+0x30), nao por GetActor("RealMyRoom")
		AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x1), 0, 0, 0, 0, 0));
		Close(FrOK, true);
	}
	return true;
}

bool FrMascotInfoDlg::OnInit()
{
	if (!m_mascotInfo.tid)
		return false;
	if ((m_mascotInfo.tid & 0x3ffffff) == 4)
		m_pMascotMsg->SetCharLimit(8, true);
	IFF_STRUCT::sMascot* pMascot = ItemManager()->FindMascot(m_mascotInfo.tid);
	// 983 (00a80800): o texto so e montado com retrato E nome; sem mascote cai no "desconhecido"
	bool bBuilt = false;
	if (m_pPortrait)
	{
		if (pMascot)
		{
			m_pPortrait->SetBgImg(pMascot->c.Icon);
			if (m_pName)
			{
				char text[512] = { 0 };
				sprintf(text, K2L_Compatibility("\300\314  \270\247: %s"), pMascot->c.Name);
				if (m_mascotInfo.tid == 0x40000005)
				{
					static const unsigned int s_expTable[9] = { 50, 70, 100, 140, 190, 250, 320, 400, 490 };
					unsigned char lv = m_mascotInfo.Level;
					if (lv < 9)
						strcat(text, MakeStr(
							K2L_Compatibility("\n\267\271  \272\247: %d / %d ( %d / %d )"),
							lv + 1, 10, m_mascotInfo.Exp, s_expTable[lv]));
					else
						strcat(text, MakeStr(
							K2L_Compatibility("\n\267\271  \272\247: %d / %d"),
							lv + 1, 10));
				}
				if (pMascot->ItemDropUp)
					strcat(text, MakeStr(
						K2L_Compatibility("\n\276\306\300\314\305\333 \265\345\267\323\267\374 : %d%%"),
						pMascot->ItemDropUp));
				if (pMascot->PangUp)
					strcat(text, MakeStr(
						K2L_Compatibility("\n\306\316 \310\271\265\346\267\374 : %d%%"),
						pMascot->PangUp));
				if (pMascot->ExpUp)
					strcat(text, MakeStr(
						K2L_Compatibility("\n\260\346\307\350\304\241 \310\271\265\346\267\374 : %d%%"),
						pMascot->ExpUp));
				if (pMascot->ItemSlot)
					strcat(text, MakeStr(
						K2L_Compatibility("\n\276\306\300\314\305\333 \275\275\267\324 : %d\304\255"),
						pMascot->ItemSlot));
				// 983: this+0x13b = Remain_Date (byte: tem prazo), this+0x13d = endDate
				if (m_mascotInfo.Remain_Date == 0)
				{
					strcat(text, K2L_Compatibility("\n\263\262\300\272\275\303\260\243 : "));
					if (m_mascotInfo.tid != 0x40000004)
						strcat(text, K2L_Compatibility("\271\253\301\246\307\321"));
				}
				else
				{
					__int64 hours = 0;
					__int64 days = 0;
					IFF_STRUCT::sMascot* pTimed = ItemManager()->FindMascot(m_mascotInfo.tid);
					if (pTimed && pTimed->c.TimeFlag) // 983 00a80c20: cmp byte [+0x82] (TimeFlag, nao Time)
					{
						__int64 total = DOCJP()->GetRemainHours983(&m_mascotInfo.endDate);
						days = total / 24;
						hours = total - days * 24;
					}
					strcat(text, K2L_Compatibility("\n\263\262\300\272\275\303\260\243 : "));
					if (days > 0)
						strcat(text, MakeStr(K2L_Compatibility("%I64d\300\317 "), days));
					if (hours > 0)
						strcat(text, MakeStr(K2L_Compatibility("%I64d\275\303\260\243"), hours));
					if (days <= 0 && hours <= 0)
						strcat(text, K2L_Compatibility("\301\276\267\341"));
				}
				m_pName->AddText(text, false, true);
				IFF_STRUCT::sDesc* pDesc = ItemManager()->FindDesc(pMascot->c.TypeId);
				if (pDesc)
					SetMessage(pDesc->Desc, false);
				bBuilt = true;
			}
		}
		else
			m_pPortrait->SetBgImg("hide");
	}
	if (!bBuilt)
	{
		if (m_pName)
			m_pName->AddText(
				K2L_Compatibility("\300\314\270\247: \272\322\270\355\n\n\267\271\272\247: \272\322\270\355"),
				false, true);
		SetMessage(
			K2L_Compatibility("\272\243\300\317\277\241 \275\316\300\316 \301\244\303\274\272\322\270\355\300\307 \301\270\300\347"),
			false);
	}
	if (pMascot)
	{
		if (Doc()->m_myInfo.stat.Level < pMascot->c.Level)
		{
			if (m_pLimit)
				m_pLimit->SetVisible(true);
			SetDesc(
				K2L_Compatibility("\267\271\272\247 \301\246\307\321\300\270\267\316 \273\347\277\353\307\322 \274\366 \276\370\275\300\264\317\264\331."));
		}
		if (pMascot->ShowMsg)
		{
			if (m_pMascotMsg)
			{
				m_pMascotMsg->SetVisible(true);
				m_pMascotMsg->SetLine(1, m_mascotInfo.szMsg, 0, 0, false);
			}
			if (m_pTextMessage)
				m_pTextMessage->SetVisible(true);
			if (m_pOk)
				m_pOk->Enable(true);
		}
	}
	return true;
}

void FrMascotInfoDlg::OnProc(const float dt)
{
	if (m_pMascotMsg)
	{
		if (stricmp(m_pMascotMsg->GetLine(1, false), m_message.c_str()))
		{
			m_message = m_pMascotMsg->GetLine(1, false);
			IFF_STRUCT::sMascot* pMascot =
				ItemManager()->FindMascot(m_mascotInfo.tid);
			if (m_pPuppet && pMascot)
				CMascot::ChangeMascotMsgInTex(m_message.c_str(), pMascot,
					m_pPuppet,
					MakeStr("myroom%02d", (pMascot->c.TypeId & 0x3ffffff)));
		}
	}
}

#define SendRmMsg983(txt) \
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0x23), (int)(txt), 0, 0, 0, 0)) // 983 00a81166: ator principal
void FrMascotInfoDlg::OnOkBtnUp()
{
	// 983 (00a80fa0): tids de mensagem editavel (0x40000000/4/5/8/f) confirmam a troca; os demais so restauram a textura e fecham.
	if (m_mascotInfo.tid == 0x40000000 || m_mascotInfo.tid == 0x40000004 || m_mascotInfo.tid == 0x40000005 ||
		m_mascotInfo.tid == 0x40000008 || m_mascotInfo.tid == 0x4000000f)
	{
		if (!strcmp(m_message.c_str(), m_mascotInfo.szMsg))
			return;
		if (m_mascotInfo.tid == 0x40000004 || m_mascotInfo.tid == 0x40000008 || m_mascotInfo.tid == 0x4000000f)
		{
			if (strlen(m_message.c_str()) > 8)
			{
				SendRmMsg983(K2L_Compatibility("\270\336\274\274\301\366 \303\326\264\353 \261\346\300\314\264\302 \277\265\271\256 8\261\333\300\332, \307\321\261\333 4\261\333\300\332 \300\324\264\317\264\331."));
				return;
			}
		}
		else if (strlen(m_message.c_str()) > 16)
		{
			SendRmMsg983(K2L_Compatibility("\270\336\274\274\301\366 \303\326\264\353 \261\346\300\314\264\302 \277\265\271\256 16\261\333\300\332, \307\321\261\333 8\261\333\300\332 \300\324\264\317\264\331."));
			return;
		}
		cTokenV token;
		token.Init(m_message.c_str(), strlen(m_message.c_str()));
		int count = token.GetTokenNum(0, "'", 1) - 1;
		if (count > 0)
		{
			SendRmMsg983(K2L_Compatibility("[']\264\302 \270\336\274\274\301\366\277\241 \273\347\277\353\307\322 \274\366 \276\370\275\300\264\317\264\331."));
		}
		else
		{
			char text[256];
			strcpy(text, m_message.c_str());
			if (Doc()->m_chatManager.Filtering(text))
				SendRmMsg983(K2L_Compatibility("\300\373\264\347\307\317\301\366\276\312\300\272 \264\334\276\356\260\241 \306\367\307\324\265\307\276\372\275\300\264\317\264\331."));
			else
			{
				if (!m_pConfirmDlg)
					m_pConfirmDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this,
						"mascot_msg", NULL);
				if (m_pConfirmDlg)
				{
					if (m_mascotInfo.tid == 0x40000005)
					{
						m_pConfirmDlg->SetMessage(K2L_Compatibility("\270\266\275\272\304\332 \270\336\274\274\301\366\270\246 \272\257\260\346\307\317\270\351 \276\306\267\241\277\315 \260\260\300\314 \261\335\276\327\300\314 \\c0xffff0000\\c\260\250\274\322\\c0xff000000\\c\307\325\264\317\264\331.\n\n\270\336\274\274\301\366\270\246 \272\257\260\346 \307\317\275\303\260\332\275\300\264\317\261\356?"), false);
						IFF_STRUCT::sMascot* pMascot =
							ItemManager()->FindMascot(m_mascotInfo.tid);
						FrStatic* pFee = DYNAMIC_CAST(FrStatic,
							(FrWnd*)m_pConfirmDlg->FindChildByName("fee"));
						if (pFee && pMascot)
							pFee->SetCaption(MakeStr("%d", pMascot->MsgFee));
						pFee = DYNAMIC_CAST(FrStatic,
							(FrWnd*)m_pConfirmDlg->FindChildByName("mypang"));
						if (pFee)
							pFee->SetCaption(MakeStr("%d", Doc()->m_myInfo.stat.i64Pang));
						pFee = DYNAMIC_CAST(FrStatic,
							(FrWnd*)m_pConfirmDlg->FindChildByName("remainpang"));
						if (pFee)
							pFee->SetCaption(MakeStr("%d",
								Doc()->m_myInfo.stat.i64Pang - pMascot->MsgFee));
					}
					else
						m_pConfirmDlg->SetMessage(K2L_Compatibility("\270\266\275\272\304\332\306\256 \270\336\274\274\301\366\270\246 \272\257\260\346 \307\317\275\303\260\332\275\300\264\317\261\356?"), false);
					m_pConfirmDlg->Open((FRESH_PFN_RESULT)&FrMascotInfoDlg::
											OnChangeMsgConfirmDlgResult,
						1);
				}
			}
		}
	}
	else
	{
		if (strcmp(m_mascotInfo.szMsg, m_message.c_str()))
		{
			IFF_STRUCT::sMascot* pMascot =
				ItemManager()->FindMascot(m_mascotInfo.tid);
			if (m_pPuppet && pMascot)
				CMascot::ChangeMascotMsgInTex(m_mascotInfo.szMsg, pMascot,
					m_pPuppet,
					MakeStr("myroom%02d", (pMascot->c.TypeId & 0x3ffffff)));
		}
		OnFreshCancel();
	}
}
