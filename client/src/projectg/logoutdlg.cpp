#include "minatl.h"
#include "logoutdlg.h"
#include "frbutton.h"
#include "fresh.h"
#include "lobbytask.h"
#include "messengerdlg.h"
#include "logininfo.h"
#include "../shared/localize.h"
#include "vc10_layout.h"

extern Fresh* g_pFresh;
extern bool g_bQuit;

IMPLEMENT_OBJECT(FrLogoutDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrLogoutDlg, FrForm)

ON_FRESH_VI("server", FRCMD_INIT, FrLogoutDlg::OnServerBtnInit)
ON_FRESH_VV("server", FRCMD_LBUTTONUP, FrLogoutDlg::OnServerBtnUp)
ON_FRESH_VI("logout", FRCMD_INIT, FrLogoutDlg::OnLogoutBtnInit)
ON_FRESH_VV("logout", FRCMD_LBUTTONUP, FrLogoutDlg::OnLogoutBtnUp)
ON_FRESH_VI("exit", FRCMD_INIT, FrLogoutDlg::OnExitBtnInit)
ON_FRESH_VV("exit", FRCMD_LBUTTONUP, FrLogoutDlg::OnExitBtnUp)

END_FRESH_MSGMAP()

FrLogoutDlg::FrLogoutDlg()
	: m_pServerBtn(NULL), m_pLogoutBtn(NULL), m_pExitBtn(NULL), m_pDescBtn(NULL),
	  m_bExitRequested983(false), m_bLogoutRequested983(false)
{
	C_ASSERT_VC10(sizeof(FrLogoutDlg) == 0x130 && offsetof(FrLogoutDlg, m_bExitRequested983) == 0x120); // 983: push 0x130 em 00adbe50
}

void FrLogoutDlg::DisableServerSelect()
{
	m_pServerBtn->Enable(false);
}

void FrLogoutDlg::OnProc(const float dt)
{
	FrForm::OnProc(dt);

	if (m_pServerBtn && m_pServerBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pServerBtn)
		{
			m_pDescBtn = m_pServerBtn;
			SetDesc(
				K2L_Compatibility("\xc1\xa2\xbc\xd3\xc7\xd2 \xbc\xad\xb9\xf6\xb8\xa6 \xbc\xb1\xc5\xc3\xc7\xd5\xb4\xcf\xb4\xd9."));
		}
	}
	if (m_pLogoutBtn && m_pLogoutBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pLogoutBtn)
		{
			m_pDescBtn = m_pLogoutBtn;
			SetDesc(
				K2L_Compatibility("\xb7\xce\xb1\xd7\xbe\xc6\xbf\xf4 \xc7\xd5\xb4\xcf\xb4\xd9."));
		}
	}
	else if (m_pExitBtn && m_pExitBtn->GetStatus() == FrButton::OVER)
	{
		if (m_pDescBtn != m_pExitBtn)
		{
			m_pDescBtn = m_pExitBtn;
			SetDesc(
				K2L_Compatibility("\xc6\xce\xbe\xdf \xc7\xc1\xb7\xce\xb1\xd7\xb7\xa5\xc0\xbb \xb3\xa1\xb3\xbb\xb0\xed \xc0\xa9\xb5\xb5\xbf\xec\xb7\xce \xb3\xaa\xb0\xa9\xb4\xcf\xb4\xd9."));
		}
	}
}

void FrLogoutDlg::OnServerBtnInit(int param)
{
	m_pServerBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pServerBtn == NULL)
		return;

	if (g_pFresh->IsCurrentLayout("SERVERLIST") || Doc()->m_offlinePlay ||
		g_pFresh->IsCurrentLayout("CREATE") ||
		g_pFresh->IsCurrentLayout("AGREEMENT") ||
		g_pFresh->IsCurrentLayout("BLANK"))
	{
		m_pServerBtn->Enable(false);
	}

	if (CLoginInfo::Instance()->IsWebLogin())
	{
		WPoint pos = (WPoint&)m_pServerBtn->GetRect();

		pos.x += 38.0f;

		m_pServerBtn->MoveWindow(pos);
	}
}

void FrLogoutDlg::OnServerBtnUp()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xea), 0, 0, 0, 0, 0));

	Close(FrOK, true);
}

void FrLogoutDlg::OnLogoutBtnInit(int param)
{
	m_pLogoutBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);
	if (m_pLogoutBtn == NULL)
		return;

	if (Doc()->m_offlinePlay)
		m_pLogoutBtn->Enable(false);

	if (CLoginInfo::Instance()->IsWebLogin())
		m_pLogoutBtn->SetVisible(false);
}

// 983 (00adc440): o logout de fato; o OnLogoutBtnUp só chega aqui nas telas de login/offline
void FrLogoutDlg::DoLogout983()
{
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, M983(0xdf), 0, 0, 0, 0, 0));

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
		WNetworkSystem::Instance()->ForceShutDown(WNetworkSystem::NET_GAME);

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_LOGIN))
		WNetworkSystem::Instance()->ForceShutDown(WNetworkSystem::NET_LOGIN);

	Doc()->m_provType = -1;
	Doc()->m_unused184 = 0;
	Doc()->m_unusedString188 = "";
	Doc()->m_paymentKID983 = "";
	if (!Doc()->m_bChannelLogin)
		Doc()->m_password = "";

	WSendPacket send((enumClientPacket)22);

	send.Send(TO_MSN);

	CMessengerInfo::Instance()->Close(true);

	WNetworkSystem::Instance()->ForceShutDown(WNetworkSystem::NET_MSN);

	if (Doc()->m_bTutorialResume == true)

		Doc()->m_bTutorialResume = false;

	if (!IS_EXACTKINDOF(CLobbyTask, AfxGetTask()))

		CTaskManager::Instance()->ChangeTask("CLobbyTask", "CGolfDoc", false);

	CTaskManager::Instance()->PostMsg(NULL, "Lobby", 0, (int)"LOGIN", 0, 0, 0);
}

// 983 (00adc630): fora das telas de login só marca +0x121 e fecha; quem decide é o CTaskMain::OnQuitDlgResult
void FrLogoutDlg::OnLogoutBtnUp()
{
	if (!g_pFresh->IsCurrentLayout("SERVERLIST") && !g_pFresh->IsCurrentLayout("CREATE") &&
		!g_pFresh->IsCurrentLayout("AGREEMENT") && !g_pFresh->IsCurrentLayout("BLANK") &&
		!Doc()->m_offlinePlay)
	{
		m_bLogoutRequested983 = true;
		Close(true);
		return;
	}

	DoLogout983();
}

void FrLogoutDlg::OnExitBtnInit(int param)
{
	m_pExitBtn = DYNAMIC_CAST(FrButton, (FrWnd*)param);

	if (IsLocalContent(S3_PARAN_CHANNELING))
		if (CLoginInfo::Instance()->IsWebLogin())
		{
			WPoint pos = (WPoint&)m_pExitBtn->GetRect();

			pos.x -= 38.0f;

			m_pExitBtn->MoveWindow(pos);
		}
}

// 983 (00adc3c0): idem ao OnLogoutBtnUp, com +0x120
void FrLogoutDlg::OnExitBtnUp()
{
	if (!g_pFresh->IsCurrentLayout("SERVERLIST") && !g_pFresh->IsCurrentLayout("CREATE") &&
		!g_pFresh->IsCurrentLayout("AGREEMENT") && !g_pFresh->IsCurrentLayout("BLANK") &&
		!Doc()->m_offlinePlay)
	{
		m_bExitRequested983 = true;
		Close(true);
		return;
	}

	g_bQuit = true;
}
