// JP 983: unidade 0047c930–0047dae0 (FrAttendanceDlg, presença diária: carimbo de entrada/saída com o prêmio). Classe do
// 839/983, sem par no 645; ver include/jp/attendancedlg.h. Sessão B96 (F3.7.10, 5º lote: `quests_presenca`). O corte da F3.7.1
// começava em 0047c5a1: 0047c580 é o `basic_stringstream::str` (molde) e 0047c6c0 é da CAchievement (usa 0047a070/0047a200,
// texto "TID:"), unidade anterior. O compilador gera (e casam): os ajustes 0047cb10/0047cb20, os inline do FrWnd/FrForm
// das vtables (0047ccd0…0047cd70), WRect::IsInRect (0047cb60) e os moldes do IObject (0047cc10…0047cc60).
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "commonutil.h"
#include "../../shared/itemmanager.h"
#include "packet.h"
#include "actor.h"
#include "taskmanager.h"
#include "lobbytask.h"
#include "shareddoc.h"
#include "messengerdlg.h"
#include "titlefontex.h"
#include "s5/utilities.h"
#include "lua983.h"
#include "attendancedlg.h"

extern Fresh* g_pFresh; // 00f02340
extern bool g_bQuit; // 00f02489

// 0047c930
void FrAttendanceDlg::SetCountdown983(int countdown)
{
	m_countdown = countdown;
}

// 0047c940 (GetMessageMap); mapa 00e034b0 (2 entradas)
BEGIN_FRESH_MSGMAP(FrAttendanceDlg, FrForm)
ON_FRESH_VI("image_area", FRCMD_INIT, FrAttendanceDlg::OnImageAreaInit983)
ON_FRESH_VI("image_area", FRCMD_OWNERDRAW, FrAttendanceDlg::OnImageAreaDraw983)
END_FRESH_MSGMAP()

// 0047c950
FrAttendanceDlg::~FrAttendanceDlg()
{
	if (m_pRoundFont)
	{
		delete m_pRoundFont;
		m_pRoundFont = NULL;
	}
	if (m_pCountFont)
	{
		delete m_pCountFont;
		m_pCountFont = NULL;
	}
}

// 0047ca00
void FrAttendanceDlg::LoadBitmaps983()
{
	if (m_checkMode983 == 0)
	{
		m_pBmp[4] = g_pFresh->GetBitmap("check_in_Normal");
		m_pBmp[0] = g_pFresh->GetBitmap("check_item_today");
		m_pHoverBmp = g_pFresh->GetBitmap("check_in_Hover");
	}
	else if (m_checkMode983 == 1)
	{
		m_pBmp[4] = g_pFresh->GetBitmap("check_out_Normal");
		m_pBmp[0] = g_pFresh->GetBitmap("check_item_next");
	}
	m_pBmp[1] = g_pFresh->GetBitmap("check_out_countdown1");
	m_pBmp[2] = g_pFresh->GetBitmap("check_out_countdown2");
	m_pBmp[3] = g_pFresh->GetBitmap("check_out_countdown3");
}

// 0047cad0
void FrAttendanceDlg::SetCount983()
{
	if (m_checkMode983 == 0)
		m_count = m_reward.todayCount;
	else if (m_checkMode983 == 1)
		m_count = m_reward.nextCount;
}

// 0047ce10
FrAttendanceDlg::FrAttendanceDlg()
	: m_pImageArea(NULL)
{
	m_fontX = 0.0f;
	m_fontY = 0.0f;
	m_pHoverBmp = NULL;
	m_itemX = 0.0f;
	m_pIconBmp = NULL;
	m_itemY = 0.0f;
	m_bHover = false;
	m_bHasReward = false;
	m_bExitRoom983 = false;
	m_time = 0.0f;
	m_pRoundFont = NULL;
	m_pCountFont = NULL;
	m_exitAction983 = -1;
	m_checkMode983 = -1;
	m_countdown = -1;
	m_count = -1;
	for (int i = 0; i < 5; i++)
		m_pBmp[i] = NULL;
}

// 0047cf10
void FrAttendanceDlg::OnImageAreaInit983(int param)
{
	m_pImageArea = DYNAMIC_CAST(FrArea, (FrWnd*)param);
	m_baseX = m_pImageArea->GetRect().x;
	m_baseY = m_pImageArea->GetRect().y;
	m_pad983_1e0 = 0.0f;
	m_pad983_1e4 = 0.0f;
}

// 0047cfa0: carimbo (aceso ou não), item, contagem; ícone com "x%d"; rodada "%04d"; dica só dentro do quadro da dica
void FrAttendanceDlg::OnImageAreaDraw983(int param)
{
	FrGraphicInterface* pGI = g_pFresh->GetManager()->GetGDI();
	if (!pGI)
		return;
	if (!m_bHover)
		pGI->DrawTexture(m_pBmp[4], m_checkRect, 0xffffffff, 0);
	else
		pGI->DrawTexture(m_pHoverBmp, m_checkRect, 0xffffffff, 0);
	pGI->DrawTexture(m_pBmp[0], m_itemRect, 0xffffffff, 0);
	if (m_countdown != -1 && m_countdown >= 0 && (unsigned)m_countdown < 5)
		pGI->DrawTexture(m_pBmp[m_countdown], m_checkRect, 0xffffffff, 0);
	if (m_pImageArea)
	{
		if (m_pIconBmp)
			pGI->DrawTexture(m_pIconBmp,
				WRect(m_itemX, m_itemY, (float)m_pIconBmp->Width(), (float)m_pIconBmp->Height()), 0xffffffff, 0);
		if (m_pCountFont)
			m_pCountFont->Print((float)(m_itemX + 45.0), (float)(m_itemY + 30.0), MakeStr("x%d", m_count),
				(eTITLEFONTALINE)2, 0.0f, 1.0f);
		if (m_bHasReward && m_pRoundFont)
			m_pRoundFont->Print(m_fontX, m_fontY, MakeStr("%04d", m_reward.round), (eTITLEFONTALINE)2, 3.0f, 1.0f);
	}
	if (m_toolTipRect.IsInRect(g_pFresh->GetManager()->GetMousePos()))
		m_pImageArea->m_nFlags.Enable(FWF_TOOLTIPS);
	else
		m_pImageArea->m_nFlags.Disable(FWF_TOOLTIPS);
}

IMPLEMENT_OBJECT(FrAttendanceDlg, FrForm)

// 0047d2e0
void FrAttendanceDlg::LoadIcon983()
{
	IFF_STRUCT::sItem* pItem;
	if (m_checkMode983 == 0)
		pItem = ItemManager()->FindItem(m_reward.todayTid);
	else if (m_checkMode983 == 1)
		pItem = ItemManager()->FindItem(m_reward.nextTid);
	else
		return;
	if (pItem)
		m_pIconBmp = g_pFresh->GetBitmap(pItem->c.Icon);
}

// 0047d330
void FrAttendanceDlg::LoadConfig983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("attendance.lua");
	lua_system::lua_table_helper tbl = script->GetTable("attendance_ui_config");
	int v;
	v = LUA983_GET(tbl, int, "checkin_or_checkout_x");
	m_checkRect.x = (float)v + m_baseX;
	v = LUA983_GET(tbl, int, "checkin_or_checkout_y");
	m_checkRect.y = (float)v + m_baseY;
	v = LUA983_GET(tbl, int, "checkin_or_checkout_w");
	m_checkRect.w = (float)v;
	v = LUA983_GET(tbl, int, "checkin_or_checkout_h");
	m_checkRect.h = (float)v;
	v = LUA983_GET(tbl, int, "todayitem_or_tomorrowitem_x");
	m_itemRect.x = (float)v + m_baseX;
	v = LUA983_GET(tbl, int, "todayitem_or_tomorrowitem_y");
	m_itemRect.y = (float)v + m_baseY;
	v = LUA983_GET(tbl, int, "todayitem_or_tomorrowitem_w");
	m_itemRect.w = (float)v;
	v = LUA983_GET(tbl, int, "todayitem_or_tomorrowitem_h");
	m_itemRect.h = (float)v;
	v = LUA983_GET(tbl, int, "font_x");
	m_fontX = (float)v + m_baseX;
	v = LUA983_GET(tbl, int, "font_y");
	m_fontY = (float)v + m_baseY;
	v = LUA983_GET(tbl, int, "item_x");
	m_itemX = (float)v + m_baseX;
	v = LUA983_GET(tbl, int, "item_y");
	m_itemY = (float)v + m_baseY;
	v = LUA983_GET(tbl, int, "itemtooltip_x");
	m_toolTipRect.x = (float)v + m_baseX;
	v = LUA983_GET(tbl, int, "itemtooltip_y");
	m_toolTipRect.y = (float)v + m_baseY;
	v = LUA983_GET(tbl, int, "itemtooltip_w");
	m_toolTipRect.w = (float)v;
	v = LUA983_GET(tbl, int, "itemtooltip_h");
	m_toolTipRect.h = (float)v;
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);
}

// 0047d590
bool FrAttendanceDlg::OnInit()
{
	LoadConfig983();
	LoadBitmaps983();
	m_pRoundFont = new CAttendanceDate;
	m_pCountFont = new CWindFont;
	return FrForm::OnInit();
}

// 0047d640
void FrAttendanceDlg::SetToolTip983()
{
	if (m_checkMode983 == 0)
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(m_reward.todayTid);
		if (pItem && m_pImageArea)
			m_pImageArea->SetToolTipText(std::string(pItem->c.Name));
	}
	else if (m_checkMode983 == 1)
	{
		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(m_reward.nextTid);
		if (pItem && m_pImageArea)
			m_pImageArea->SetToolTipText(pItem->c.Name);
	}
}

// 0047d770
void FrAttendanceDlg::DoLogout983()
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

// 0047d960
void FrAttendanceDlg::OnProc(float t)
{
	FrForm::OnProc(t);
	t += m_time;
	m_time = t;
	if (!(t < 1.0f))
	{
		if (t < 2.0f)
		{
			if (m_checkMode983 == 0)
				m_bHover = true;
		}
		else if (t < 3.0f)
			m_countdown = 3;
		else if (t < 4.0f)
			m_countdown = 2;
		else if (t < 5.0f)
			m_countdown = 1;
		else if (m_checkMode983 == 1)
		{
			if (m_exitAction983 == 0)
			{
				if (m_bExitRoom983 && OnlinePlay())
					S5::UTIL::SendExitRoom983(0);
				g_bQuit = true;
			}
			else if (m_exitAction983 == 1)
				DoLogout983();
		}
		else if (m_checkMode983 == 0)
			Close(true);
	}
}

// 0047da80
void FrAttendanceDlg::SetReward983(const sAttendanceReward983* pReward)
{
	m_reward = *pReward;
	m_bHasReward = true;
	LoadIcon983();
	SetCount983();
	SetToolTip983();
}
