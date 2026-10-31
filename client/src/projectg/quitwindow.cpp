#include "minatl.h"
#include "quitwindow.h"
#include "projectg.h"
#include "golfdoc.h"
#include "avatar_task.h"
#include "messengerdlg.h"
#include "mousecursor.h"
#include "clientsetting.h"
#include "treasure.h"
#include "networksystem.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frdesktop.h"
#include "optiondlg.h"
#include "frbutton.h"
#include "../shared/localize.h"
#include "contentsdoc.h"
#include "chatmsg.h"
#include "s5/utilities.h"
#include "jp/grandprixdocument.h" // B1502: m_kind (+0x60 no VC10)
#include "vc10_layout.h"

C_ASSERT_VC10(offsetof(_gateway::_grandprix::CGrandPrixDocument, m_kind) == 0x60);

extern Fresh* g_pFresh;
extern bool g_bQuit;

// HACK: CGolfDoc::GetCurrentPlayerNum is inline in golfdoc.h (wind inlines it),
// but the original calls the out-of-line copy here, so it is reached through a
// fastcall alias instead.
extern "C" unsigned char __fastcall rb_GetCurrentPlayerNum(CGolfDoc* doc);
#pragma comment(linker, \
	"/alternatename:@rb_GetCurrentPlayerNum@4=?GetCurrentPlayerNum@CGolfDoc@@QAEEXZ")

IMPLEMENT_OBJECT(FrQuitForm, FrForm)

BEGIN_FRESH_MSGMAP(FrQuitForm, FrForm)

ON_FRESH_VI("option", FRCMD_INIT, FrQuitForm::OnOptionBtnInit)
ON_FRESH_VV("option", FRCMD_LBUTTONUP, FrQuitForm::OnOptionBtnUp)
ON_FRESH_VI("next", FRCMD_INIT, FrQuitForm::OnNextBtnInit)
ON_FRESH_VV("next", FRCMD_LBUTTONUP, FrQuitForm::OnNextBtnUp)
ON_FRESH_VI("tolobby", FRCMD_INIT, FrQuitForm::OnToLobbyBtnInit)
ON_FRESH_VV("tolobby", FRCMD_LBUTTONUP, FrQuitForm::OnToLobbyBtnUp)
ON_FRESH_VI("exit", FRCMD_INIT, FrQuitForm::OnExitBtnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, FrQuitForm::OnExitBtnUp)
ON_FRESH_VV("cancel", FRCMD_LBUTTONUP, FrQuitForm::OnCloseBtnUp)

END_FRESH_MSGMAP()

FrQuitForm::FrQuitForm()
	: m_pOptionBtn(NULL),
	  m_pNextBtn(NULL),
	  m_pToLobbyBtn(NULL),
	  m_pExitBtn(NULL),
	  m_pDescBtn(NULL),
	  m_time(0)
{
	if (GOLFDOC() && GOLFDOC()->m_gameMode == 0x200 &&
		COption::Instance()->gGetTerrainTooltip())
		CMouseCursor::Instance()->SetTerrainToolTip(false);
}

void FrQuitForm::OnDraw()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	pGDI->Box(g_pFresh->GetManager()->GetDesktop()->GetRect(),
		((unsigned char)(m_time * 64.0f) << 24) | 0x404040);
}

void FrQuitForm::OnProc(const float dt)
{
	FrForm::OnProc(dt);

	m_time += dt;
	if (m_time > 2.0f)
		m_time = 2.0f;

	if (m_pOptionBtn && m_pOptionBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pOptionBtn)
		{
			m_pDescBtn = m_pOptionBtn;
			SetDesc(
				K2L_Compatibility("\xc6\xce\xbe\xdf \xbc\xb3\xc1\xa4\xc0\xbb \xba\xaf\xb0\xe6\xc7"
				"\xd5\xb4\xcf\xb4\xd9."));
		}
	}

	if (m_pNextBtn && m_pNextBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pNextBtn)
		{
			m_pDescBtn = m_pNextBtn;
			SetDesc(
				"\xb4\xd9\xc0\xbd \xb4\xdc\xb0\xe8\xb7\xce \xc0\xcc\xb5\xbf\xc7"
				"\xd5\xb4\xcf\xb4\xd9.");
		}
	}
	else if (m_pToLobbyBtn && m_pToLobbyBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pToLobbyBtn)
		{
			m_pDescBtn = m_pToLobbyBtn;
			if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
			{
				SetDesc(
					K2L_Compatibility("\xb4\xeb\xc8\xad\xb9\xe6\xc0\xbb \xb3\xaa\xb0\xa9\xb4\xcf\xb4"
					"\xd9."));
				if ((int)CMessengerInfo::Instance()->GetMyStatus() != 3)
					CMessengerInfo::Instance()->SetMyStatus(
						(MSN::eStatusType)4);
			}
			else
				SetDesc(
					K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb7"
					"\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa9\xb4\xcf\xb4\xd9."));
		}
	}
	else if (m_pExitBtn && m_pExitBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pExitBtn)
		{
			m_pDescBtn = m_pExitBtn;
			SetDesc(
				K2L_Compatibility("\xc6\xce\xbe\xdf \xc7\xc1\xb7\xce\xb1\xd7\xb7\xa5\xc0\xbb \xb3"
				"\xa1\xb3\xbb\xb0\xed \xc0\xa9\xb5\xb5\xbf\xec\xb7\xce \xb3\xaa"
				"\xb0\xa9\xb4\xcf\xb4\xd9."));
		}
	}
}

void FrQuitForm::OnOptionBtnInit(int param)
{
	m_pOptionBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (GOLFDOC()->m_tutorialMode < 15)
		m_pOptionBtn->SetVisible(false);
}

void FrQuitForm::OnNextBtnInit(int param)
{
	m_pNextBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pNextBtn)
	{
		if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
			GOLFDOC()->m_tutorialMode >= 8 ||
			CQuitWindow::Instance()->HasOptionWindow())
			m_pNextBtn->SetVisible(false);
	}
}

void FrQuitForm::OnToLobbyBtnInit(int param)
{
	m_pToLobbyBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pToLobbyBtn)
	{
		if (Doc()->m_offlinePlay)
			m_pToLobbyBtn->Enable(false);

		// 983 (006ef710): o byte testado e Doc+0x109b8 = m_roomInfo.realGameType (o 645 lia m_golfGame.gameType)
		if (GOLFDOC() && GOLFDOC()->m_gameMode == 0x20000)
		{
			switch (Doc()->m_roomInfo.realGameType)
			{
			case 12:
			case 14:
			case 19:
				m_pToLobbyBtn->Enable(false);
				break;
			}
		}
	}
}

void FrQuitForm::OnExitBtnInit(int param)
{
	m_pExitBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
}

void FrQuitForm::OnOptionBtnUp()
{
	if (m_pOptionBtn)
		m_pOptionBtn->Enable(false);

	if (CQuitWindow::Instance())
		CQuitWindow::Instance()->CreateOptionWindow();
}

void FrQuitForm::OnNextBtnUp()
{
	Close(false);

	IActor* pActor = AfxGetTask()->GetActor("Tutorial");
	if (pActor)
		pActor << MsgObject(NULL, M983(0x14b), 0, 0, 0, 0, 0);
}

void FrQuitForm::OnToLobbyBtnUp()
{
	// 983 (006f0360): textos com K2L_Compatibility, aviso das salas 12/14/19 (bilhete de chip-in nas 13/14), "todos os
	// registros anulados" nas 11-14, pena em pang no "tolobby" (GetMyDisconPangPenalty) e as salas 20/21 pelo documento 0xaf
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		g_bQuit = true;
		return;
	}

	unsigned char gameType = Doc()->m_golfGame.gameType;
	if (gameType == 2)
	{
		OnToLobbyConfirmResult(1, NULL);
		return;
	}

	FrForm* pForm;
	const char* msg;
	if (Doc()->m_gameMode == 1)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb4"
			"\xeb\xb1\xe2\xbd\xc7\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda"
			"\xbd\xc0\xb4\xcf\xb1\xee?";
	}
	else if (Doc()->m_roomInfo.realGameType == 12 ||
		Doc()->m_roomInfo.realGameType == 14 ||
		Doc()->m_roomInfo.realGameType == 19)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		if ((unsigned)(Doc()->m_roomInfo.realGameType - 13) <= 1)
			msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xc1\xf6\xb1\xdd \xc6\xf7\xb1\xe2\xc7"
			"\xcf\xbf\xa9\xb5\xb5 \\c0xffff0000\\c<\xc4\xa8\xc0\xce \xbf\xac"
			"\xbd\xc0 \xc6\xbc\xc4\xcf> 1\xc0\xe5\\c0xff000000\\c\xc0\xba"
			" \\c0xffff0000\\c\xbc\xd2\xb8\xf0\\c0xff000000\\c\xb5\xcb\xb4"
			"\xcf\xb4\xd9.\n\n\xc1\xa4\xb8\xbb \xb0\xd4\xc0\xd3\xc0\xbb \xc6"
			"\xf7\xb1\xe2\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee"
			"?";
		else
			msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb4"
			"\xeb\xb1\xe2\xbd\xc7\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda"
			"\xbd\xc0\xb4\xcf\xb1\xee?";
	}
	else if (Doc()->m_gameMode == 5)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb4"
			"\xeb\xb1\xe2\xbd\xc7\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda"
			"\xbd\xc0\xb4\xcf\xb1\xee?       (\xb0\xad\xc1\xa6\xc1\xbe\xb7"
			"\xe1\xc0\xb2\xc0\xba \xb1\xe2\xb7\xcf\xb5\xc7\xc1\xf6 \xbe\xca"
			"\xc1\xf6\xb8\xb8, \xbb\xe7\xbf\xeb\xb5\xc8 \xbe\xc6\xc0\xcc\xc5"
			"\xdb\xc0\xba \xb5\xb9\xb7\xc1 \xb9\xde\xc1\xf6 \xb8\xf8\xc7\xd5"
			"\xb4\xcf\xb4\xd9.)";
	}
	else if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xb6\xb7\xaf\xb8\xae\xb8\xa6 \xb3\xa1\xb3\xbb\xb0\xed \xb7"
			"\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa9\xb4\xcf\xb4\xd9.";
	}
	else if (gameType == GAME_TYPE_TUTORIAL_BASIC ||
		gameType == GAME_TYPE_TUTORIAL_ADV)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb7"
			"\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda\xbd\xc0"
			"\xb4\xcf\xb1\xee? ";
	}
	else if ((gameType == 0 && rb_GetCurrentPlayerNum(GOLFDOC()) == 1) ||
		Doc()->m_golfGame.gameType == 10)
	{
	stop_yesno:
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xd4\xc0\xd3\xc0\xbb \xb1\xd7\xb8\xb8\xc7\xcf\xb0\xed \xb7"
			"\xce\xba\xf1\xb7\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda\xbd\xc0"
			"\xb4\xcf\xb1\xee?";
	}
	else if ((unsigned)(Doc()->m_roomInfo.realGameType - 11) <= 3)
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_yesno",
			NULL);
		msg = "\xb0\xd4\xc0\xd3\xc0\xcc \xbf\xcf\xc0\xfc\xc8\xf7 \xc1\xbe\xb7"
			"\xe1\xb5\xc7\xb1\xe2 \xc0\xfc\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xbb"
			" \xc6\xf7\xb1\xe2\xc7\xcf\xb8\xe9, \\c0xffff0000\\c\xb8\xf0\xb5"
			"\xe7 \xb1\xe2\xb7\xcf\xb0\xfa \xba\xb8\xbb\xf3\xc0\xcc \xb9\xab"
			"\xc8\xbf\xc3\xb3\xb8\xae\\c0xff000000\\c\xb5\xcb\xb4\xcf\xb4"
			"\xd9.\n\n\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xbd"
			"\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?";
	}
	else
	{
		// 983: documento 0xaf do CContentsDoc = CGrandPrixDocument, int em +0x60 = m_kind.
		// B1502: era *(int*)(pDoc+0x60) cru (VC10); no cl 14.44 os 3 vector + map antes encolhem e caia fora de m_kind.
		_gateway::_grandprix::CGrandPrixDocument* pRoomDoc = (_gateway::_grandprix::CGrandPrixDocument*)
			CContentsDoc::Instance()->GetContainer((localContentType_t)0xaf);
		if ((unsigned)(Doc()->m_roomInfo.realGameType - 20) <= 1 &&
			pRoomDoc->m_kind == 0)
			goto stop_yesno;
		pForm =
			CreateForm<FrForm>(g_pFresh->GetManager(), this, "tolobby", NULL);
		pForm->SetMessage(MakeStr(K2L_Compatibility("\xb0\xd4\xc0\xd3\xc0\xcc \\c0xffff0000\\c\xbf\xcf\xc0\xfc\xc8"
			"\xf7 \xc1\xbe\xb7\xe1\\c0xff000000\\c\xb5\xc7\xb1\xe2 \xc0\xfc"
			"\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb8"
			"\xe9, \\c0xffff0000\\c\xc6\xce -%d\\c0xff000000\\c\xc0\xc7 \xb9"
			"\xfa\xc4\xa2\xc0\xbb \xb9\xde\xb0\xd4 \xb5\xc7\xb8\xe7, \\c0"
			"xffff0000\\c\xb0\xe6\xc7\xe8\xc4\xa1 \xc8\xb9\xb5\xe6\xb0\xfa"
			" \xb0\xa2\xc1\xbe \xb1\xe2\xb7\xcf\xc0\xcc \xb9\xab\xc8\xbf\xc3"
			"\xb3\xb8\xae\\c0xff000000\\c\xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb6"
			"\xc7\xc7\xd1 \xb8\xb9\xc0\xba \xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1"
			"\xb7\xce \xc0\xce\xc7\xd8 \xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xc0"
			"\xb2\xc0\xcc \xb4\xd9\xc0\xbd\xb0\xfa \xb0\xb0\xc0\xba \xbc\xf6"
			"\xc4\xa1\xb0\xa1 \xb5\xc7\xb8\xe9 \\c0xffff0000\\c\xbc\xba\xba"
			"\xb0 \xc7\xa5\xbd\xc3 \xbe\xc6\xc0\xcc\xc4\xdc\\c0xff000000\\"
			"c\xc0\xcc \xc5\xb9\xc7\xd8\xc1\xfc\xb0\xfa \xb5\xbf\xbd\xc3\xbf"
			"\xa1 \\c0xffff0000\\c\xbb\xe7\xbf\xeb \xb1\xe2\xb4\xc9\xbf\xa1"
			" \xb4\xeb\xc7\xd1 \xc1\xa6\xbe\xe0\\c0xff000000\\c\xc0\xcc \xbb"
			"\xfd\xb1\xe9\xb4\xcf\xb4\xd9.\n\n\n\n\n\n\n\n\xb0\xd4\xc0\xd3"
			"\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xb7\xce\xba\xf1\xb7"
			"\xce \xb3\xaa\xb0\xa1\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee"
			"?"),
							  Doc()->GetMyDisconPangPenalty()),
			false);
		pForm->Open((FRESH_PFN_RESULT)&FrQuitForm::OnToLobbyConfirmResult, 1);
		return;
	}

	pForm->SetMessage(K2L_Compatibility(msg), false);
	pForm->Open((FRESH_PFN_RESULT)&FrQuitForm::OnToLobbyConfirmResult, 1);
}

void FrQuitForm::OnExitBtnUp()
{
	if (!WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		g_bQuit = true;
		return;
	}

	FrForm* pForm;
	if (OnlinePlay())
	{
		if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			pForm->SetMessage(K2L_Compatibility(
					"\xb0\xb6\xb7\xaf\xb8\xae\xb8\xa6 \xb3\xa1\xb3\xbb\xb0\xed \xc6\xce"
					"\xbe\xdf\xb8\xa6 \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0"
					"\xb4\xcf\xb1\xee?"), false);
		}
		else if (Doc()->m_golfGame.gameType == 2)
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			pForm->SetMessage(K2L_Compatibility(
					"\xb4\xeb\xc8\xad\xb9\xe6\xbf\xa1\xbc\xad \xb3\xaa\xb0\xa1\xb0\xed"
					" \xc6\xce\xbe\xdf\xb8\xa6 \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda"
					"\xbd\xc0\xb4\xcf\xb1\xee?"), false);
		}
		else if (Doc()->m_roomInfo.realGameType == 12 ||
			Doc()->m_roomInfo.realGameType == 14 ||
			Doc()->m_roomInfo.realGameType == 19)
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			const char* msg;
			if ((unsigned)(Doc()->m_roomInfo.realGameType - 13) <= 1)
				msg =
					"\xb0\xd4\xc0\xd3\xc0\xbb \xc1\xf6\xb1\xdd \xc6\xf7\xb1\xe2\xc7\xcf"
					"\xbf\xa9\xb5\xb5 \\c0xffff0000\\c<\xc4\xa8\xc0\xce \xbf\xac\xbd\xc0"
					" \xc6\xbc\xc4\xcf> 1\xc0\xe5\\c0xff000000\\c\xc0\xba \\c0xffff00"
					"00\\c\xbc\xd2\xb8\xf0\\c0xff000000\\c\xb5\xcb\xb4\xcf\xb4\xd9.\n"
					"\n\xc1\xa4\xb8\xbb \xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7"
					"\xcf\xb0\xed \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4"
					"\xcf\xb1\xee?";
			else
				msg =
					"\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xc6\xce"
					"\xbe\xdf\xb8\xa6 \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0"
					"\xb4\xcf\xb1\xee?";
			pForm->SetMessage(K2L_Compatibility(msg), false);
		}
		else if ((unsigned)(Doc()->m_roomInfo.realGameType - 11) <= 3)
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"notify_yesno", NULL);
			pForm->SetMessage(K2L_Compatibility(
					"\xb0\xd4\xc0\xd3\xc0\xcc \xbf\xcf\xc0\xfc\xc8\xf7 \xc1\xbe\xb7\xe1"
					"\xb5\xc7\xb1\xe2 \xc0\xfc\xbf\xa1 \xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7"
					"\xb1\xe2\xc7\xcf\xb8\xe9, \\c0xffff0000\\c\xb8\xf0\xb5\xe7 \xb1\xe2"
					"\xb7\xcf\xb0\xfa \xba\xb8\xbb\xf3\xc0\xcc \xb9\xab\xc8\xbf\xc3\xb3"
					"\xb8\xae\\c0xff000000\\c\xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb0\xd4\xc0"
					"\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4"
					"\xcf\xb1\xee?"), false);
		}
		else
		{
			pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
				"tolobby", NULL);
			pForm->SetMessage(MakeStr(K2L_Compatibility(
					"\xb0\xd4\xc0\xd3\xc0\xcc \\c0xffff0000\\c\xbf\xcf\xc0\xfc\xc8\xf7"
					" \xc1\xbe\xb7\xe1\\c0xff000000\\c\xb5\xc7\xb1\xe2 \xc0\xfc\xbf\xa1"
					" \xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb8\xe9, \\c0"
					"xffff0000\\c\xc6\xce -%d\\c0xff000000\\c\xc0\xc7 \xb9\xfa\xc4\xa2"
					"\xc0\xbb \xb9\xde\xb0\xd4 \xb5\xc7\xb8\xe7, \\c0xffff0000\\c\xb0"
					"\xe6\xc7\xe8\xc4\xa1 \xc8\xb9\xb5\xe6\xb0\xfa \xb0\xa2\xc1\xbe \xb1"
					"\xe2\xb7\xcf\xc0\xcc \xb9\xab\xc8\xbf\xc3\xb3\xb8\xae\\c0xff0000"
					"00\\c\xb5\xcb\xb4\xcf\xb4\xd9.\n\n\xb6\xc7\xc7\xd1 \xb8\xb9\xc0\xba"
					" \xb0\xad\xc1\xa6\xc1\xbe\xb7\xe1\xb7\xce \xc0\xce\xc7\xd8 \xb0\xad"
					"\xc1\xa6\xc1\xbe\xb7\xe1\xc0\xb2\xc0\xcc \xb4\xd9\xc0\xbd\xb0\xfa"
					" \xb0\xb0\xc0\xba \xbc\xf6\xc4\xa1\xb0\xa1 \xb5\xc7\xb8\xe9 \\c0"
					"xffff0000\\c\xbc\xba\xba\xb0 \xc7\xa5\xbd\xc3 \xbe\xc6\xc0\xcc\xc4"
					"\xdc\\c0xff000000\\c\xc0\xcc \xc5\xb9\xc7\xd8\xc1\xfc\xb0\xfa \xb5"
					"\xbf\xbd\xc3\xbf\xa1 \\c0xffff0000\\c\xbb\xe7\xbf\xeb \xb1\xe2\xb4"
					"\xc9\xbf\xa1 \xb4\xeb\xc7\xd1 \xc1\xa6\xbe\xe0\\c0xff000000\\c\xc0"
					"\xcc \xbb\xfd\xb1\xe9\xb4\xcf\xb4\xd9.\n\n\n\n\n\n\n\n\xb0\xd4\xc0"
					"\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xc6\xce\xbe\xdf\xb8"
					"\xa6 \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1"
					"\xee?"),
				Doc()->GetMyDisconPangPenalty()), false);
		}
	}
	else
	{
		pForm = CreateForm<FrForm>(g_pFresh->GetManager(), this,
			"notify_yesno", NULL);
		pForm->SetMessage(K2L_Compatibility(
					"\xb0\xd4\xc0\xd3\xc0\xbb \xc6\xf7\xb1\xe2\xc7\xcf\xb0\xed \xc6\xce"
					"\xbe\xdf\xb8\xa6 \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0"
					"\xb4\xcf\xb1\xee?"), false);
	}

	pForm->Open((FRESH_PFN_RESULT)&FrQuitForm::OnExitConfirmResult, 1);
}

void FrQuitForm::OnCloseBtnUp()
{
	Close(FrNONE, true);
}

// 983 (006efeb0): salas de realGameType 0xc/0xe/0x13 saem por 0x131/0x130, 0x14/0x15 pelo 0x17a (auxiliar S5
// 00b2e3d0) e a mensagem 1 ao ator principal; no resto o 0xf de sempre e o lobby abre no TOPPAGE (+ mensagem 0x8b 4).
// Fora da rede, o tutorial volta ao CTutorialWidget do CLobbyTask (o 983 não tem mais CTutorialTask).
bool FrQuitForm::OnToLobbyConfirmResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		if (OnlinePlay())
		{
			if (Doc()->m_golfGame.gameType == 0 &&
				rb_GetCurrentPlayerNum(GOLFDOC()) == 1 &&
				!(bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
			{
				WSendPacket packet((enumClientPacket)0x37);
				packet.Send(TO_GAME);
				if (CTHunter::Instance())
					CTHunter::Instance()->SetReturnLobby(true);
			}
			else
			{
				unsigned char realGameType = Doc()->m_roomInfo.realGameType;
				switch (realGameType)
				{
				case 0xc:
				case 0xe:
				case 0x13:
					if ((unsigned)(realGameType - 0xb) <= 3)
					{
						WSendPacket packet((enumClientPacket)0x131);
						packet.Send(TO_GAME);
					}
					else
					{
						WSendPacket packet((enumClientPacket)0x130);
						packet.Send(TO_GAME);
					}
					if (CTHunter::Instance())
						CTHunter::Instance()->SetReturnLobby(true);
					break;
				default:
					if ((unsigned)(realGameType - 0x14) <= 1)
					{
						S5::UTIL::SendExitGPRoom983(1);
						AfxGetTask()->SendMsgToMainActor(
							MsgObject(NULL, 1, 0, 0, 0, 0, 0));
					}
					else
					{
						Doc()->m_bGameOver = true;

						WSendPacket packet((enumClientPacket)0xf);
						packet.Encode1(1);
						packet.Encode2(0xffff);

						bool bFound = false;
						if (IsLocalContent((localContentType_t)0x14) &&
							(Doc()->m_roomInfo.gameType == 4 ||
								Doc()->m_roomInfo.gameType == 5))
						{
							std::vector<sRivalData>::iterator it;
							for (it = Doc()->m_rivalList.begin();
								it != Doc()->m_rivalList.end(); ++it)
							{
								if ((*it).oid == MyGuid(false))
								{
									packet.Encode8((*it).totalPang);
									packet.Encode8((*it).totalBonusPang);
									bFound = true;
									break;
								}
							}
						}

						if (!bFound)
						{
							packet.Encode8(0);
							packet.Encode8(0);
						}
						packet.Send(TO_GAME);

						CProjectG::Instance()->FromGolfToLobbySettings();
						CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
							false);
						CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0,
							(int)"TOPPAGE", 0, 0, 0);
						CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x8b, 4, 0, 0,
							0); // 983: número do 983
						CTaskManager::Instance()->PostMsg(NULL, "Lobby", M983(0x1a), 0, 0, 0,
							0);
					}
					break;
				}
			}
		}
		else
		{
			CProjectG::Instance()->FromGolfToLobbySettings();

			if (Doc()->m_gameMode == 1)
			{
				CTaskManager::Instance()->ChangeTask("CFamilyTask", "", false);
				CTaskManager::Instance()->PostMsg(NULL, "Family", 0,
					(int)"FAMILY", 0, 0, 0);
			}
			else if (Doc()->m_gameMode == 5)
			{
				CTaskManager::Instance()->ChangeTask("CGhostTask", "",
					CTaskManager::Instance()->GetPreservedTask() != NULL);
				CTaskManager::Instance()->PostMsg(NULL, "Ghost", 0,
					(int)"GHOST", 0, 0, 0);
			}
			else if (Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_BASIC ||
				Doc()->m_golfGame.gameType == GAME_TYPE_TUTORIAL_ADV)
			{
				Doc()->m_gameMode = 0;
				CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc",
					false);
				CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0x88,
					(int)"CTutorialWidget", (int)"TOPPAGE", 0, 0); // 983: número do 983
			}
		}

		Close(FrNONE, true);
	}

	return true;
}

// 983 (006ef850): fecha antes; na praça (ntAvatarChatTask) manda 0x16e e NÃO sai do jogo (o servidor responde);
// fora dela o 0xf vai pelo auxiliar S5 00b2f070 (que também zera a sala do jogador)
bool FrQuitForm::OnExitConfirmResult(int result, FrForm* pForm)
{
	if (result == 1)
	{
		Close(FrNONE, true);

		if (IS_EXACTKINDOF(ntAvatarChatTask, AfxGetTask()))
		{
			WSendPacket packet((enumClientPacket)0x16e);
			packet.Send(TO_GAME);
			return true;
		}

		if (OnlinePlay())
			S5::UTIL::SendExitRoom983(1);

		g_bQuit = true;
	}

	return true;
}

void FrQuitForm::EnableOptionBtn(bool enable)
{
	if (m_pOptionBtn)
		m_pOptionBtn->Enable(enable);
}

CQuitWindow::CQuitWindow()
	: m_pQuitForm(NULL), m_pOptionForm(NULL), m_blockedModes983(0), m_bMouseActive(true)
{
}

CQuitWindow::~CQuitWindow()
{
}

void CQuitWindow::Process()
{
	if (m_blockedModes983 & GOLFDOC()->m_gameMode)
		return;

	if (g_pFresh->GetManager()->HasEscKeyWindow())
		return;

	if (CChatMsg::Instance()->IME_IsCompInProgress())
		return;

	if (CChatMsg::Instance()->IME_IsCompDone())
		return;

	if (m_pQuitForm)
		return;

	m_pQuitForm =
		CreateForm<FrQuitForm>(g_pFresh->GetManager(), this, "quitdlg", NULL);
	m_pQuitForm->Open((FRESH_PFN_RESULT)&CQuitWindow::OnQuitFormResult, 1);
	m_pQuitForm->MoveCursor("tolobby");

	COption::Instance()->SetQuitWindowed(true);
	FrForm::ms_bModalLock983 = true; // 983: call 00be7950

	if (GOLFDOC()->m_tutorialMode < 15)
	{
		m_bMouseActive = CMouseCursor::Instance()->IsActive();
		CMouseCursor::Instance()->SetActive(true);
	}
}

bool CQuitWindow::IsActive()
{
	return m_pQuitForm ? true : false;
}

bool CQuitWindow::OnQuitFormResult(int result, FrForm* pForm)
{
	COption::Instance()->SetQuitWindowed(false);

	if (GOLFDOC() && GOLFDOC()->m_gameMode == 0x200 &&
		COption::Instance()->gGetTerrainTooltip())
		CMouseCursor::Instance()->SetTerrainToolTip(true);

	if (GOLFDOC()->m_tutorialMode < 15)
		CMouseCursor::Instance()->SetActive(m_bMouseActive);

	m_pQuitForm = NULL;
	return true;
}

void CQuitWindow::CreateOptionWindow()
{
	if (m_pQuitForm)
		m_pQuitForm->EnableOptionBtn(false);

	if (m_pOptionForm == NULL)
	{
		CMouseCursor::Instance()->m_bArea[CMouseCursor::AREA_AUXMENU_OPTION] =
			false;

		m_pOptionForm = CreateForm<FrOptionDlg>(g_pFresh->GetManager(), this,
			"option", NULL);
		m_pOptionForm->Open((FRESH_PFN_RESULT)&CQuitWindow::OnOptionResult,
			0x11);
		m_pOptionForm->SetApplyScrSize(true);
		m_pOptionForm->m_defaultRetCode = FrCANCEL;

		if (m_pQuitForm)
		{
			WRect rc = m_pQuitForm->GetOptionBtnRect();
			m_pOptionForm->SetIconRect(rc);
		}
		else
		{
			WRect rc(g_view->GetWidth() - 64.0f, 181.0f, 64.0f, 24.0f);
			m_pOptionForm->SetIconRect(rc);
		}
	}
}

extern WView* g_view; // 00f02458

void CQuitWindow::CenterQuitForm983()
{
	// JP 983 (006ef2d0): chamada pelo COption quando a resolucao do jogo muda
	if (m_pQuitForm)
	{
		WRect rect = m_pQuitForm->GetRect();
		rect.x = g_view->GetWidth() * 0.5 - rect.w * 0.5;
		rect.y = g_view->GetHeight() * 0.5 - rect.h * 0.5;
		m_pQuitForm->SetRect(rect);
	}
}

bool CQuitWindow::OnOptionResult(int result, FrForm* pForm)
{
	m_pOptionForm = NULL;

	if (m_pQuitForm)
		m_pQuitForm->EnableOptionBtn(true);

	CMouseCursor::Instance()->m_bArea[CMouseCursor::AREA_AUXMENU_OPTION] = true;
	return true;
}
