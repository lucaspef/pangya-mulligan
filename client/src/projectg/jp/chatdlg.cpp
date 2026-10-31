// JP 983: unidade 0048f0a0–00490ef0 (FrChatDlg, a conversa do lobby; base de FrGameRoomDlg e FrRoomListDlg). É o código de
// conversa do `CLobbyMain` do 645 mudado para uma classe própria: os corpos partem dos do src/projectg/lobbymain.cpp e foram
// conferidos com o 983. 00490ef0 (11 B) já é o destrutor da CNoClassicWind (unidade classicwind). Sessão B108: a classe inteira.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "fredit.h"
#include "frcombobox.h"
#include "frscrollbar.h"
#include "frgraphicinterface.h"
#include "woverlay.h"
#include "wlocalize.h"
#include "inputmanager.h"
#include "clientsetting.h"
#include "chatmsg.h"
#include "rankingdlg.h"
#include "messengerdlg.h"
#include "emoticondlg.h"
#include "reportdlg.h"
#include "shareddoc.h"
#include "docofs983.h" // D862: crus do CSharedDoc ancorados no membro (VS 2022)
#include "shareddocjp.h"
#include "chatdlg.h"

extern Fresh* g_pFresh; // 00f02340

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif
#define DOC983_U8(off) (*(unsigned char*)DOCPTR983(off))

// 0048f0a0 (GetMessageMap); mapa 00e04210 (14 entradas; ajuste -4 em todas)
BEGIN_FRESH_MSGMAP(FrChatDlg, FrForm)
ON_FRESH_VI("chat_group.chatbg", FRCMD_INIT, FrChatDlg::OnChatBgInit)
ON_FRESH_VI("chat_group.chatbg", FRCMD_OWNERDRAW, FrChatDlg::OnChatBgOwnerDraw)
ON_FRESH_VI("chat_group.chatwndbtn", FRCMD_INIT, FrChatDlg::OnChatWndBtnInit)
ON_FRESH_VV("chat_group.chatwndbtn", FRCMD_LBUTTONDOWN, FrChatDlg::OnChatWndBtnUp)
ON_FRESH_VI("chat_group.emoticon", FRCMD_INIT, FrChatDlg::OnEmoticonInit)
ON_FRESH_VV("chat_group.emoticon", FRCMD_LBUTTONDOWN, FrChatDlg::OnEmoticonBtnUp)
ON_FRESH_VI("chat_group.language", FRCMD_INIT, FrChatDlg::OnLanguageInit)
ON_FRESH_VI("chat_group.chatinput", FRCMD_INIT, FrChatDlg::OnChatInputInit)
ON_FRESH_BI("chat_group.chatinput", FRCMD_ENTERKEY, FrChatDlg::OnChatInputEnterKey)
ON_FRESH_VI("chat_group.chattarget", FRCMD_INIT, FrChatDlg::OnChatTargetInit)
ON_FRESH_VI("chat_group.chattarget", FRCMD_LBUTTONDOWN, FrChatDlg::OnChatTargetBtnDown)
ON_FRESH_BI("chat_group.chattarget", FRCMD_ENTERKEY, FrChatDlg::OnChatTargetEnterKey)
ON_FRESH_VI("chat_group.chatview", FRCMD_INIT, FrChatDlg::OnChatViewInit)
ON_FRESH_VV("chat_group.report", FRCMD_LBUTTONDOWN, FrChatDlg::OnReportBtnUp)
END_FRESH_MSGMAP()

// 0048f0b0
FrChatDlg::FrChatDlg()
	: m_pEmoticonDlg(NULL), m_pReportDlg(NULL), m_pChatBg(NULL), m_pChatWndBtn(NULL), m_pLanguage(NULL),
	  m_pChatInput(NULL), m_pChatView(NULL), m_pChatTarget(NULL), m_pEmoticon(NULL)
{
}

IMPLEMENT_OBJECT(FrChatDlg, FrForm)

// 0048f120
FrChatDlg::~FrChatDlg()
{
}

// 0048f140
bool FrChatDlg::OnInit()
{
	return FrForm::OnInit();
}

// 0048f150
void FrChatDlg::SetLanguage983(bool bKor)
{
	if (m_pLanguage)
	{
		if (!bKor)
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");
	}
}

// 0048f180
const char* FrChatDlg::GetChatText983()
{
	if (m_pChatInput)
		return m_pChatInput->GetLine(1, false);
	return NULL;
}

// 0048f1a0
bool FrChatDlg::OnReportDlgResult(int result, FrForm* form)
{
	m_pReportDlg = NULL;
	return true;
}

// 0048f1b0
void FrChatDlg::OnChatTargetBtnDown(int param)
{
	m_pChatInput->SetKeyFocus(false);
}

// 0048f1c0
bool FrChatDlg::OnChatTargetEnterKey(int param)
{
	m_pChatInput->SetKeyFocus(true);
	return false;
}

// 0048f5e0
void FrChatDlg::OnChatBgInit(int param)
{
	m_pChatBg = KINDOF_CAST983(FrArea, param);
}

// 0048f650: caixa 0xa0000000 (o 645 usava 0x58000000)
void FrChatDlg::OnChatBgOwnerDraw(int param)
{
	if (g_pFresh->GetManager()->GetGDI())
	{
		const WRect& rect = m_pChatBg->GetRect();
		WOverlay::DrawBox(g_view, WRect(rect.x, rect.y, rect.w, rect.h), 0, 0xa0000000, 0);
	}
}

// 0048f6c0
void FrChatDlg::ToggleChatBtn(bool bMove)
{
	if (COption::Instance()->gGetChatWndUp())
	{
		m_pChatWndBtn->SetButtonImg("chatwnd_down", FrButton::NORMAL);
		if (!bMove)
			return;

		const WRect& bgRect = m_pChatBg->GetRect();
		m_pChatBg->SetRect(WRect(bgRect.x, bgRect.y - 200.0f, bgRect.w, bgRect.h + 200.0f));

		const WRect& viewRect = m_pChatView->GetRect();
		m_pChatView->SetRect(WRect(viewRect.x, viewRect.y - 200.0f, viewRect.w, viewRect.h + 200.0f));
	}
	else
	{
		m_pChatWndBtn->SetButtonImg("chatwnd_up", FrButton::NORMAL);
		if (!bMove)
			return;

		const WRect& bgRect = m_pChatBg->GetRect();
		m_pChatBg->SetRect(WRect(bgRect.x, bgRect.y + 200.0f, bgRect.w, bgRect.h - 200.0f));

		const WRect& viewRect = m_pChatView->GetRect();
		m_pChatView->SetRect(WRect(viewRect.x, viewRect.y + 200.0f, viewRect.w, viewRect.h - 200.0f));
	}

	FrScrollBar* bar = m_pChatView->GetScrollBar();
	if (bar)
		bar->ScrollToBottom();
}

// 0048f7f0: o 983 pergunta ao CChatMsg se o IME está aberto (o 645 usava g_ime->IsAlphaNumericMode)
void FrChatDlg::OnLanguageInit(int param)
{
	m_pLanguage = KINDOF_CAST983(FrArea, param);
	if (m_pLanguage)
	{
		if (!CChatMsg::Instance()->IME_IsImeOpened())
			m_pLanguage->SetBgImg("chat_eng");
		else
			m_pLanguage->SetBgImg("chat_kor");
	}
}

// 0048f890: limite de 46 caracteres e a entrada 45 mais estreita
void FrChatDlg::OnChatInputInit(int param)
{
	m_pChatInput = KINDOF_CAST983(FrEdit, param);
	if (m_pChatInput)
	{
		m_pChatInput->SetCharLimit(0x2e, true);
		WRect rect = m_pChatInput->GetRect();
		rect.w -= 45.0f;
		m_pChatInput->SetRect(rect);
		m_pChatInput->SetKeyFocus(true);
	}
}

// 0048f960
void FrChatDlg::OnEmoticonInit(int param)
{
	m_pEmoticon = KINDOF_CAST983(FrButton, param);
}

// 0048fcb0
void FrChatDlg::SetChatTarget983(const char* name)
{
	if (Doc()->AddWhisperPartner(m_pChatTarget, name))
		m_pChatTarget->SetLine(1, name, 0, false, 0);
	if (m_pChatInput)
		m_pChatInput->SetKeyFocus(true);
}

// 0048fd00
void FrChatDlg::AddWhisperPartner983(const char* name)
{
	Doc()->AddWhisperPartner(m_pChatTarget, name);
}

// 0048fd20
void FrChatDlg::ResetChatTarget983()
{
	Doc()->InitRecentWhisperList(m_pChatTarget);
	Doc()->InitReservedChatList(m_pChatTarget, (eReservedChatPartner)0);
}

// 0048fd50
void FrChatDlg::OnChatWndBtnInit(int param)
{
	m_pChatWndBtn = KINDOF_CAST983(FrButton, param);
	m_pChatWndBtn->SetPushDelay(0.0f);
	m_pChatWndBtn->UseDblClick(false);

	bool bUp = COption::Instance()->gGetChatWndUp() != 0;

	ToggleChatBtn(bUp);
}

// 0048fdf0
void FrChatDlg::OnChatWndBtnUp()
{
	COption::Instance()->gSetChatWndUp(!COption::Instance()->gGetChatWndUp());

	ToggleChatBtn(true);
}

// 0048fe10: a lista do alvo abre para cima; partner 3 só no tipo de sala 6
void FrChatDlg::OnChatTargetInit(int param)
{
	m_pChatTarget = KINDOF_CAST983(FrComboBox, param);
	m_pChatTarget->HidePrivacy(true);
	m_pChatTarget->SetListDir983(FrComboBox::LISTDIR983_ABOVE);
	Doc()->InitReservedChatList(m_pChatTarget, (eReservedChatPartner)((DOC983_U8(0x1094c) == 6) + 2));
}

// 0048ffe0
bool FrChatDlg::OnEmoticonResult(int result, FrForm* form)
{
	if (m_pEmoticonDlg && result == 1)
	{
		FrEmoticonDlg* pDlg = DYNAMIC_CAST(FrEmoticonDlg, form);
		if (pDlg)
		{
			const char* icon = pDlg->GetSelectedIcon();
			if (icon)
			{
				FrEdit* pEdit = m_pChatInput;
				if (m_pReportDlg)
					pEdit = DYNAMIC_CAST(FrEdit, m_pReportDlg->FindChildByName("id"));

				if (pEdit)
				{
					const char* front = pEdit->GetEditText_Front();
					const char* comp = pEdit->GetEditText_Comp();
					const char* end = pEdit->GetEditText_End();

					if (CChatMsg::Instance()->GetMaskedFont()->GetTextWidthOld983(
							g_view, MakeStr("%s%s%s%s", front, comp, icon, end)) < pEdit->GetWidthLimit())
						pEdit->SetLine(1, MakeStr("%s%s%s%s", front, comp, icon, end), 0, false, 0);
					pEdit->SetKeyFocus(true);
				}
			}
		}
	}

	m_pEmoticonDlg = NULL;
	return true;
}

// 00490170
void FrChatDlg::OnEmoticonBtnUp()
{
	if (m_pEmoticon && !m_pEmoticonDlg)
	{
		m_pEmoticonDlg = CreateForm<FrEmoticonDlg>(g_pFresh->GetManager(), this, "emoticon");
		if (m_pEmoticonDlg)
		{
			WPoint pos(0.0f, 0.0f);

			if (m_pEmoticon)
			{
				pos.x = m_pEmoticon->GetRect().x - 200.0f;
				pos.y = m_pEmoticon->GetRect().y - 260.0f;
			}

			m_pEmoticonDlg->Open((FRESH_PFN_RESULT)&FrChatDlg::OnEmoticonResult, pos, 3);
		}
	}
}

// 00490430: o pedaço de conversa do CLobbyMain::ProcessDialogKeyboard do 645 + o atalho Ctrl+EMOTICON do OnProc
void FrChatDlg::OnProc(const float deltaTime)
{
	if ((g_input->Get("LCONTROL", false) || g_input->Get("RCONTROL", false)) && g_input->GetDown("EMOTICON", false) &&
		!CRankingInfo::Instance()->GetDlg() && !CMessengerInfo::Instance()->IsVisible())
		OnEmoticonBtnUp();

	if (m_pChatTarget && m_pChatInput)
	{
		if (g_input->GetDown("PAGE_UP", true))
		{
			if ((bool)((m_pChatInput->m_nFlags >> 4) & 1) && Doc()->m_chatHistory.size() > 0)
			{
				std::list<std::string>::iterator it = std::find(
					Doc()->m_chatHistory.begin(), Doc()->m_chatHistory.end(), m_pChatInput->GetLine(1, false));
				if (it != Doc()->m_chatHistory.end())
				{
					if (it != Doc()->m_chatHistory.begin())
						CChatMsg::Instance()->SetChatText((*--it).c_str(), false);
				}
				else
				{
					CChatMsg::Instance()->SetChatText(Doc()->m_chatHistory.back().c_str(), false);
				}
			}
			else if ((bool)((m_pChatTarget->m_nFlags >> 4) & 1) && Doc()->m_reservedChatList.size() > 0)
			{
				CChatMsg::Instance()->SetChatText(Doc()->GetPrevChatTarget(m_pChatTarget->GetLine(1, false)), false);
			}
		}
		else if (g_input->GetDown("PAGE_DOWN", true))
		{
			if ((bool)((m_pChatInput->m_nFlags >> 4) & 1) && Doc()->m_chatHistory.size() > 0)
			{
				std::list<std::string>::iterator it = std::find(
					Doc()->m_chatHistory.begin(), Doc()->m_chatHistory.end(), m_pChatInput->GetLine(1, false));
				if (it != Doc()->m_chatHistory.end())
				{
					if (++it != Doc()->m_chatHistory.end())
						CChatMsg::Instance()->SetChatText((*it).c_str(), false);
					else
						CChatMsg::Instance()->SetChatText("", false);
				}
			}
			else if ((bool)((m_pChatTarget->m_nFlags >> 4) & 1) && Doc()->m_reservedChatList.size() > 0)
			{
				CChatMsg::Instance()->SetChatText(Doc()->GetNextChatTarget(m_pChatTarget->GetLine(1, false)), false);
			}
		}
		else if (g_input->GetDown("ENTER", true) && !CMessengerInfo::Instance()->IsVisible() &&
			!(bool)((m_pChatInput->m_nFlags >> 4) & 1))
		{
			m_pChatInput->SetKeyFocus(true);
		}

		if (strlen(m_pChatTarget->GetLine(1, false)) &&
			strcmp(m_pChatTarget->GetLine(1, false), K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4")))
		{
			if (strlen(m_pChatTarget->GetLine(1, false)) &&
				strcmp(m_pChatTarget->GetLine(1, false), K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1\xb0\xd4")))
			{
				m_pChatInput->SetFontColor(0xffcf67ff);
				return;
			}
			m_pChatInput->SetFontColor(0xff5bffb0);
			return;
		}
		m_pChatInput->SetFontColor(0xffffffff);
	}
}

// 004907f0: as três linhas passam pelo K2L_Compatibility no 983
void FrChatDlg::OnReportBtnUp()
{
	m_pReportDlg = CreateForm<FrReportDlg>(g_pFresh->GetManager(), this, "report");

	std::string msg;
	msg = K2L_Compatibility("1. \xb4\xeb\xc8\xad\xc3\xa2\xc0\xc7 \xc1\xf6\xb3\xad \xb3\xbb\xbf\xeb\xb5\xb5 \xb8\xf0\xb5\xce "
							"\xbc\xad\xb9\xf6\xbf\xa1 \xb1\xe2\xb7\xcf\xb5\xcb\xb4\xcf\xb4\xd9.\n");
	msg += K2L_Compatibility("2. \xc1\xb6\xc0\xdb, \xc7\xe3\xc0\xa7, \xb9\xdd\xba\xb9\xbd\xc5\xb0\xed\xb4\xc2 "
							 "\xc3\xb3\xb9\xfa\xc0\xbb \xb9\xde\xbd\xc0\xb4\xcf\xb4\xd9.\n");
	msg += K2L_Compatibility("3. \xbd\xc5\xb0\xed\xc7\xcf\xbd\xc5 \xb3\xbb\xbf\xeb\xc0\xba 48\xbd\xc3\xb0\xa3 "
							 "\xc0\xcc\xb3\xbb\xbf\xa1 \xc3\xb3\xb8\xae\xb5\xcb\xb4\xcf\xb4\xd9.");

	m_pReportDlg->SetMessage(msg.c_str(), false);
	m_pReportDlg->Open((FRESH_PFN_RESULT)&FrChatDlg::OnReportDlgResult, 3);
}

// 00490b30
void FrChatDlg::OnChatViewInit(int param)
{
	m_pChatView = KINDOF_CAST983(FrEdit, param);
	m_pChatView->HidePrivacy(true);

	Doc()->m_chatLineList.clear();
}

// 00490d10: o caso 3 do CLobbyMain::HandleMsg do 645 (a linha só fica guardada nos estados de sala 3, 4, 6 e 8)
void FrChatDlg::AddChatLine983(const char* text, unsigned long color)
{
	int state = DOCJP()->GetState983();
	if (state == 3 || state == 4 || state == 8 || state == 6)
	{
		sChatLine line;
		line.text = text;
		line.color = color;

		Doc()->m_chatLineList.push_back(line);
	}

	if (m_pChatView)
		m_pChatView->AddLine(text, color, false);
}

// 00490e50
bool FrChatDlg::OnChatInputEnterKey(int param)
{
	if (strlen((const char*)param))
	{
		if (!Doc()->UTIL_SendChatMessage(m_pChatTarget, (const char*)param, true))
		{
			m_pChatTarget->SetLine(1, K2L_Compatibility("\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4"), 0, false, 0);
			AddChatLine983(K2L_Compatibility("\xb1\xd3\xb8\xbb \xbb\xf3\xb4\xeb\xb0\xa1 \xc0\xdf\xb8\xf8\xb5\xc7\xbe\xfa\xbd\xc0"
											 "\xb4\xcf\xb4\xd9. \"\xb8\xf0\xb5\xce\xbf\xa1\xb0\xd4\"\xb7\xce \xba\xaf\xb0\xe6"
											 "\xc7\xd5\xb4\xcf\xb4\xd9."),
				0xffaf00a1);
		}
	}

	return true;
}
