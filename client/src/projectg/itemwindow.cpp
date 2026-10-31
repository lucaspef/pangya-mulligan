#include "minatl.h"
#include "s5/utilities.h"
#include "itemwindow.h"
#include "wbone.h"
#include "projectg.h"
#include "golfdoc.h"
#include "jp/golfdocjp.h"
#include "golfball.h"
#include "mousecursor.h"
#include "club.h"
#include "quadtree.h"
#include "clientsetting.h"
#include "quitwindow.h"
#include "chatmsg.h"
#include "cardsystem.h"
#include "simpleoverlay.h"
#include "../shared/localize.h"
#include "jp/uiframesetmanager.h"
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.artifactTid983) == 0x109b9 && offsetof(CSharedDoc, m_curChannel.Type) == 0x10a99); // A747: ex-crus

// HACK: CChatMsg::Close is not declared in chatmsg.h. Adding it there perturbs
// projectg.cpp (SaveScreenShot), so it is declared in a local helper class.
class CChatMsgEx : public CChatMsg
{
public:
	void Close(bool b1, bool b2);
};

#pragma comment(linker, \
	"/alternatename:?Close@CChatMsgEx@@QAEX_N0@Z=?Close@CChatMsg@@QAEX_N0@Z")

extern WResourceManager* g_resrcmng;

class CHelpMsg : public WSingleton<CHelpMsg>
{
public:
	void SetItemHelpMsg(unsigned long tid);
	void SetHelpMsg(int id, char* msg);
};

namespace S5
{
	bool IsUsableCalipers();
}

bool IsMyTurn(bool check);
int float2int(float value);
void PerformPowerBarSetting();

// HACK: the original has an unreferenced 8-byte constant pair in .rdata; an
// extern const array is the only form the compiler emits without a use.
extern const float rb_unused_consts[2];
const float rb_unused_consts[2] = { 0.00001f, 140.0f };

CItemWindow::eState CItemWindow::ms_state = CItemWindow::STATE_CLOSED;

IMPLEMENT_ACTOR(CItemWindow, IActor)

CItemWindow::CItemWindow()
{
	m_pOverlay = NULL;
	m_pUI = NULL;
	m_bActive = true;
	m_dispPriority = DISP_PRIORITY_4;
}

void CItemWindow::OnDestroy()
{
	std::map<unsigned long, WOverlay*>::iterator it;
	for (it = m_iconMap.begin(); it != m_iconMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}
	m_iconMap.clear();

	if (g_resrcmng && m_pOverlay)
	{
		g_resrcmng->Release(m_pOverlay);
		m_pOverlay = NULL;
	}

	if (m_pUI)
	{
		delete m_pUI;
		m_pUI = NULL;
	}
}

void CItemWindow::OnLoad()
{
	if (GOLFDOC()->m_tutorialMode >= 15 || GOLFDOC()->m_tutorialMode == 5)
	{
		m_pOverlay = g_resrcmng->GetOverlay("{Item_win.png", 0);
		m_pUI = new CSimpleUI("[emoti_object.png");

		m_pUI->Add("0", WRect(0.0f, 128.0f, 64.0f, 64.0f), false);
		m_pUI->Add("1", WRect(64.0f, 128.0f, 64.0f, 64.0f), false);
		m_pUI->Add("2", WRect(128.0f, 128.0f, 64.0f, 64.0f), false);
		m_pUI->Add("3", WRect(192.0f, 128.0f, 64.0f, 64.0f), false);
		m_pUI->Add("4", WRect(0.0f, 192.0f, 64.0f, 64.0f), false);
		m_pUI->Add("5", WRect(64.0f, 192.0f, 64.0f, 64.0f), false);
		m_pUI->Add("6", WRect(128.0f, 192.0f, 64.0f, 64.0f), false);
		m_pUI->Add("7", WRect(192.0f, 192.0f, 64.0f, 64.0f), false);

		ResetUiPosition();
	}
}

void CItemWindow::OnInit()
{
	ResetVars();
}

void CItemWindow::ResetVars()
{
	m_curX = 0.0f;
	m_velocity = 0.0f;
	ms_state = STATE_CLOSED;
	m_hover = -1;
	m_curSlot = -1;
	m_selected = -1;
	m_itemTid = 0;
	m_unused74 = 200.0f;
	m_unused78 = 200.0f;
	m_bAnim = false;
	m_bOpenAllowed = true;

	// 983 (0075c780): evento tipo 4 do jogador atual avisa o ator; limpa o evento 2 do jogador atual
	if (GOLFDOC()->IsCurrentPlayerEvent983(4) == true)
		SendMsg(this, "Player", M983(0x203), 0, 0, 0, 0);
	GOLFDOCJP()->DisablePlayerEvent(2, GOLFDOC()->m_currentPlayer);
}

static void GetItemWindowAreaPos983(float& x, float& y);

void CItemWindow::OnProcess(float delta)
{
	if (GOLFDOC()->m_tutorialMode >= 15 || GOLFDOC()->m_tutorialMode == 5)
	{
		if (m_bAnim)
		{
			if (!(GOLFDOC()->m_gameMode & 8))
				m_bAnim = false;

			m_effTime += delta;

			if (m_bFail)
			{
				m_frame = (unsigned char)(m_effTime * 10.0f);
				if (m_frame == 8)
				{
					m_frame = 0;
					m_bAnim = false;
				}
			}
			else if (m_effTime < 0.8f)
			{
				m_effVY += delta * 6.0f;
				m_effX += m_effVX;
				m_effY += m_effVY;
			}
			else
			{
				WBone* pHead;
				SendMsg(this, "Player", M983(0x111), (int)&pHead, (int)"Bip01 Head",
					0, 0);

				WVector pos(pHead->m_matrix.pivot.x, pHead->m_matrix.pivot.y - 0.5f, pHead->m_matrix.pivot.z);

				WVector screen = g_view->Projection(pos);

				float dx = screen.x - m_effX;
				m_effVX += dx * delta * 0.5f;
				m_effVY += (screen.y - m_effY) * delta * 0.5f;
				float x = m_effVX + m_effX;
				float y = m_effVY + m_effY;

				if ((screen.x - x) * dx < 0.0f)
					m_bAnim = false;

				m_effX = x;
				m_effY = y;
			}
		}

		if (m_bActive && IsMyTurn(true))
		{
			WVector p1 = g_input->GetMousePoint();
			m_mouseX = p1.x;
			WVector p2 = g_input->GetMousePoint();
			m_mouseY = p2.y;

			CheckInput();

			switch (ms_state)
			{
			case 0:
				OnOpened(delta);
				break;

			case 1:
				OnRight(delta);
				break;

			case 2:
				OnLeft(delta);
				break;

			case 3:
				OnClosed(delta);
				break;
			}

			if (ms_state != STATE_CLOSED)
			{
				// 983 0075cf30: x da area (p_item_window_area) - 509 + m_curX (645: m_curX - 444)
				float areaX, areaY;
				GetItemWindowAreaPos983(areaX, areaY);
				m_dst.x = areaX - 509.0f + m_curX;
			}
		}
	}
}

void CItemWindow::CheckInput()
{
	if (CQuitWindow::Instance()->IsActive())
		return;

	if (PLAYER(GOLFDOC()->m_currentPlayer)->itemNum == 0)
		return;

	if (!(GOLFDOC()->m_gameMode & 0x308))
		return;

	if (g_input->GetDown("ITEMSLOT", true) ||
		(CMouseCursor::Instance()->GetArea() == 6 &&
			CMouseCursor::Instance()->IsInButtonDownArea(LEFT_BUTTON) &&
			g_input->GetButton(LEFT_BUTTON) == 2))
	{
		static_cast<CChatMsgEx*>(CChatMsg::Instance())->Close(false, false);

		if ((PLAYER(GOLFDOC()->m_currentPlayer)->item == 0 &&
				m_selected == -1) ||
			(Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe)
			ExecuteNow(true);
	}
}

void CItemWindow::ExecuteNow(bool bSound)
{
	switch (ms_state)
	{
	case 0:
		if (bSound)
			g_audio->PlaySfx("play_menu_close");

	case 1:
		ms_state = STATE_CLOSING;
		m_hover = m_curSlot = -1;
		CMouseCursor::Instance()->SetItemWindow(false);
		CMouseCursor::Instance()->SetAutoHide(true);
		SendMsg(this, "Screen", M983(0x1b0), 0, 0, 0, 0);
		if (CHelpMsg::IsInstantiated())
			CHelpMsg::Instance()->SetItemHelpMsg(0);
		break;

	case 3:
		if (bSound)
			g_audio->PlaySfx("play_menu_open");

		GolfClub().SetItemWindow(true);
		if (GolfClub().BarIs(0))
			GolfClub().SetBar(1);

	case 2:
		ms_state = STATE_OPENING;
		m_targetX = PLAYER(GOLFDOC()->m_currentPlayer)->itemNum * 45.0f + 64.0f;
		m_velocity = 1500.0f;
		SendMsg(this, "Screen", M983(0x1af), 0, 0, 0, 0);
		CMouseCursor::Instance()->SetAutoHide(false);
		break;
	}
}

void CItemWindow::OnClosed(float delta)
{
	if (PLAYER(GOLFDOC()->m_currentPlayer)->item != 0)
		CHelpMsg::Instance()->SetItemHelpMsg(
			PLAYER(GOLFDOC()->m_currentPlayer)->item);
}

void CItemWindow::OnRight(float delta)
{
	if (CChatMsg::Instance()->IsActive())
	{
		ms_state = STATE_CLOSING;
		CMouseCursor::Instance()->SetItemWindow(false);
	}

	m_curX += delta * m_velocity;
	if (m_curX > m_targetX)
	{
		m_velocity -= delta * 52500.0f;
		if (m_curX > m_targetX + 20.0f)
			m_curX = m_targetX + 20.0f;
	}
	else if (m_velocity < 0.0f)
	{
		m_curX = m_targetX;
		ms_state = STATE_OPENED;
		m_curSlot = 0;
		CMouseCursor::Instance()->SetItemWindow(true);
	}
}

void CItemWindow::OnLeft(float delta)
{
	m_curX -= delta * 1500.0f;
	if (m_curX < 0.0f)
	{
		m_curX = 0.0f;
		ms_state = STATE_CLOSED;
		SendMsg(this, "Screen", M983(0x1af), 0, 0, 0, 0);
		GolfClub().SetItemWindow(false);
	}
}

void CItemWindow::OnOpened(float delta)
{
	if (CQuitWindow::Instance()->IsActive())
		return;

	if (CChatMsg::Instance()->IsActive())
	{
		ms_state = STATE_CLOSING;
		CMouseCursor::Instance()->SetItemWindow(false);
	}

	if (m_selected < 0)
	{
		if (m_bOpenAllowed)
		{
			bool bUsed = false;
			unsigned long* pSlot = GetTidItemSlot(GOLFDOC()->m_currentPlayer);
			m_hover = -1;
			m_itemTid = 0;
			// 983 0075dcfd: a area dos itens vem do helper 0075cf30 (645: x fixo 65)
			float areaX, areaY;
			GetItemWindowAreaPos983(areaX, areaY);

			if (CMouseCursor::Instance()->GetArea() == 18)
			{
				m_hover = m_curSlot =
					float2int((m_mouseX - areaX - 49.0f) / 45.0f);

				if (m_hover >= 0 &&
					m_hover < PLAYER(GOLFDOC()->m_currentPlayer)->itemNum)
				{
					if ((g_input->GetButton(LEFT_BUTTON) == 2 &&
							CMouseCursor::Instance()->IsInButtonDownArea(
								LEFT_BUTTON)) ||
						g_input->GetDown(
							"\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
						bUsed = UseItem(m_hover);
					else if (CHelpMsg::IsInstantiated())
						CHelpMsg::Instance()->SetItemHelpMsg(pSlot[m_hover]);
				}
				else if (CHelpMsg::IsInstantiated())
					CHelpMsg::Instance()->SetItemHelpMsg(0);

				if (m_targetX + areaX - 15.0f < m_mouseX &&
					g_input->GetButton(LEFT_BUTTON) == 2 &&
					CMouseCursor::Instance()->IsInButtonDownArea(LEFT_BUTTON))
				{
					CMouseCursor::Instance()->SetItemWindow(false);
					ms_state = STATE_CLOSING;
					g_audio->PlaySfx("play_menu_close");
				}
			}

			if (!bUsed)
			{
				if (m_curSlot >= 0 &&
					m_curSlot < PLAYER(GOLFDOC()->m_currentPlayer)->itemNum)
				{
					int cur = m_curSlot;
					if (CHelpMsg::IsInstantiated() && cur != m_hover)
						CHelpMsg::Instance()->SetItemHelpMsg(pSlot[cur]);

					if (g_input->GetDown(
							"\xbd\xba\xc6\xe4\xc0\xcc\xbd\xba\xb9\xd9", true))
					{
						if (UseItem(m_curSlot))
							return;
					}
				}

				if (g_input->GetDown("\xc1\xc2", true))
				{
					WVector delta2 = g_input->GetMouseDelta();
					if (m_curSlot >= 1)
					{
						m_curSlot -= 1;
						if (CMouseCursor::Instance()->GetArea() == 18 &&
							WisZero(delta2.x) && WisZero(delta2.y))
							CMouseCursor::Instance()->ForceMove(
								g_input->GetMousePoint().x - 45.0f,
								g_input->GetMousePoint().y, -1.0f);

						if (CHelpMsg::IsInstantiated() && m_curSlot != m_hover)
							CHelpMsg::Instance()->SetItemHelpMsg(
								pSlot[m_curSlot]);
					}
				}

				if (g_input->GetDown("\xbf\xec", true))
				{
					WVector delta2 = g_input->GetMouseDelta();
					if (m_curSlot <
						PLAYER(GOLFDOC()->m_currentPlayer)->itemNum - 1)
					{
						m_curSlot += 1;
						if (CMouseCursor::Instance()->GetArea() == 18 &&
							WisZero(delta2.x) && WisZero(delta2.y))
							CMouseCursor::Instance()->ForceMove(
								g_input->GetMousePoint().x + 45.0f,
								g_input->GetMousePoint().y, -1.0f);

						if (CHelpMsg::IsInstantiated() && m_curSlot != m_hover)
							CHelpMsg::Instance()->SetItemHelpMsg(
								pSlot[m_curSlot]);
					}
				}
			}
		}
	}
}

bool CItemWindow::ProcessItem(unsigned char player, unsigned long tid,
	bool bConsume, unsigned long random)
{
	if (player >= GOLFDOC()->m_playerNum)
		return false;

	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(player);
	unsigned long* pSlot = GetTidItemSlot(player);
	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(tid);

	// 983 (0075d060): so a variante do Card System decide a falha; sem o ramo 0x70 do 645
	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		if (pItem == NULL)
		{
			m_bFail = true;
			return false;
		}
		int rate = pItem->COM[1];
		CCardManager::Instance()->SetPlayerIndex(
			GOLFDOC()->m_currentPlayer);
		m_bFail = (short)(random % 100) <
			rate - CCardManager::Instance()->GetCardSuccesssProbUp();
	}
	else
	{
		short prob = pItem ? pItem->COM[1] : 100;
		m_bFail = (short)(random % 100) < prob;
	}

	if (player != GOLFDOC()->m_currentPlayer)
		return false;

	if (bConsume)
	{
		if (m_selected < 0 || pSlot[m_selected] != tid)
		{
			if (CCardManager::Instance()->GetPeriodFlag20_983() == 0)
				return false;
			SendMsg(this, "GolfRule", M983(0x9c), 0, 0, 0, 0);
			pPlayer->item = 0;
			return false;
		}

		pPlayer->itemNum--;
		if (m_selected < pPlayer->itemNum)
			memmove(&pSlot[m_selected], &pSlot[m_selected + 1],
				(pPlayer->itemNum - m_selected) * 4);
		pSlot[pPlayer->itemNum] = 0;
		random = m_selected;
		m_selected = -1;
	}
	else
	{
		int index = 0;
		while (index < pPlayer->itemNum && pSlot[index] != tid)
			index++;

		if (index >= pPlayer->itemNum)
			return false;

		pPlayer->itemNum--;
		if (index < pPlayer->itemNum)
			memmove(&pSlot[index], &pSlot[index + 1],
				(pPlayer->itemNum - index) * 4);
		pSlot[pPlayer->itemNum] = 0;
		random = index;
	}

	if (GOLFDOC()->m_gameMode == 8)
	{
		// 983 0075d2a9: posicao do efeito pela area de itens (0075cf30); 645: x = slot*45 + 114, y = barra - 23
		float areaX, areaY;
		GetItemWindowAreaPos983(areaX, areaY);
		m_frame = 0;
		m_effTime = 0.0f;
		m_effVX = 0.0f;
		m_effVY = -3.0f;
		m_bAnim = true;
		m_effX = (int)random * 45.0 + (areaX + 49.0);
		m_effY = areaY + 2.0;
	}

	m_itemTid = tid;
	if (m_bFail)
		tid = 0x18000031;

	// 983: ids de mensagem crus (0x14b/0x14c/0x14d nao tem par 645 votado)
	switch (tid & 0xfffff)
	{
	case 4:
	case 16:
	case 34:
	case 47:
		SendMsg(this, "Player", 0x14b, 0, 0, 0, 0);
		break;

	case 5:
		SendMsg(this, "GuideLine", M983(0x1a2), 1, 0, 0, 0);
		break;

	case 9:
		PerformPowerBarSetting();
		break;

	case 37:
		if (Doc()->m_golfGame.gameType == 1)
		{
			float gauge = TEAM(PLAYER(player)->team)->gauge;
			float limit = Doc()->GetComboGaugeLimit(player);
			if (PLAYER(player)->teammate != 0xff)
				limit = Max<float>(limit,
					Doc()->GetComboGaugeLimit(PLAYER(player)->teammate));
			TEAM(PLAYER(player)->team)->gauge =
				Between(0.0f, gauge + 33.0f, limit);
		}
		else
		{
			float gauge = pPlayer->gauge;
			pPlayer->gauge =
				Between(0.0f, gauge + 33.0f, Doc()->GetComboGaugeLimit(player));
		}
		break;

	case 39:
		if (IsLocalContent(S3_STRENGTH))
			SendMsg(this, "Player", 0x14c, 0, 0, 0, 0);
		break;

	case 40:
	case 45:
		GOLFDOCJP()->EnablePlayerEvent(2, player);
		break;

	case 46:
		SendMsg(this, "Player", 0x14d, 0, 0, 0, 0);
		break;
	}

	pPlayer->item = tid;

	if (bConsume && OnlinePlay() &&
		Doc()->m_roomInfo.artifactTid983 != 0x1a0001fa &&
		!(Doc()->m_curChannel.Type & 0x88))
	{
		memcpy(Doc()->m_myInfo.userEquip.tidItemSlot,
			Doc()->m_userInfo[player].userEquip.tidItemSlot,
			sizeof(Doc()->m_myInfo.userEquip.tidItemSlot));
	}

	if (m_bFail)
		g_audio->PlaySfx("play_fail_item");
	else if (m_itemTid == 0x1800000f)
		g_audio->PlaySfx(MakeStr("t_Ex9-%d", rand() % 4 + 1));
	else
		g_audio->PlaySfx("play_use_item");

	if (player == GOLFDOC()->m_currentPlayer && m_itemTid == 0x1800000e)
	{
		SendMsg(this, "GolfRule", M983(0x9c), 0, 0, 0, 0);
		pPlayer->item = 0;
	}

	if (player == GOLFDOC()->m_currentPlayer &&
		(m_itemTid == 0x18000028 || m_itemTid == 0x1800002d))
		SendMsg(this, "Screen", M983(0x202), 0, 0, 0, 0);

	// 983: avisa o "Gateway" (0x753b, sub 6, tid do item)
	IActor* pGateway = AfxGetTask()->GetActor("Gateway");
	if (pGateway)
		pGateway << MsgObject(NULL, 0x753b, 6, (int)m_itemTid, 0, 0, 0);

	return true;
}

bool CItemWindow::IsUsable(unsigned long tid)
{
	if (tid == 0)
		return false;

	if ((tid & 0xfc000000) != 0x18000000)
		return false;

	switch (tid & 0xfffff)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 7:
	case 8:
	case 10:
	case 11:
	case 15:
	case 17:
	case 18:
	case 19:
	case 31:
	case 33:
	case 41: // 983 0075caf0: 0x29-0x2b tambem (o 645 nao tinha)
	case 42:
	case 43:
		return GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2;

	case 12:
		if ((int)PLAYER(GOLFDOC()->m_currentPlayer)->gauge < 33)
			return false;

	case 4:
	case 6:
	case 22:
	case 32:
	case 34:
	case 37:
	case 39:
	case 44: // 983 0075caf0: 0x2c-0x2f fora do green e sem putter (o 645 recusava)
	case 45:
	case 46:
	case 47:
		return GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2 &&
			GolfClub().GetType() != 3;

		case 5:
		return GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) == 2;

	case 9:
		return true;

	case 14:
		return GolfBall().m_bOB;

	case 16:
		return GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2 &&
			GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2 &&
			GolfClub().GetType() != 3;

		case 40:
		// 983 0075cc61: Safety recusado com o evento 2/4 do jogador da vez ja ligado (0x28/0x2d no ProcessItem);
		// sem o IsLocalContent(S3_SAFETEE_ITEM) e sem o teste de chao 0 do 645
		if (GOLFDOC()->IsCurrentPlayerEvent983(6))
			return false;
		return GetPVS().GetGroundType(GolfBall().m_pos, true, NULL) != 2;

		 case 13:
		return false;
	}

	return false;
}

bool CItemWindow::UseItem(int slot)
{
	if ((bool)((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
		return false;

	unsigned long tid = GetTidItemSlot(GOLFDOC()->m_currentPlayer)[slot];
	if (!IsUsable(tid))
	{
		if (CHelpMsg::IsInstantiated())
			CHelpMsg::Instance()->SetHelpMsg(17, NULL);
		return false;
	}

	if (tid == 0x18000009 && Doc()->m_gameMode != 2 && Doc()->m_gameMode != 1 &&
		S5::IsUsableCalipers() == 1)
	{
		if (CHelpMsg::IsInstantiated())
			CHelpMsg::Instance()->SetHelpMsg(27, NULL);
		return false;
	}

	CMouseCursor::Instance()->SetItemWindow(false);
	m_selected = slot;
	ms_state = STATE_CLOSING;

	if (OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0x17);
		packet.Encode4(tid);
		packet.Send(TO_GAME);
	}
	else if (Doc()->m_gameMode != 5)
	{
		SendMsg(this, "GolfRule", M983(0x92), 0, 0, 0, 0);
		ProcessItem(GOLFDOC()->m_currentPlayer, tid, true, rand());
	}

	return true;
}

void CItemWindow::NetUseItem(unsigned long a, unsigned long b, unsigned long c)
{
	unsigned char i = 0;

	if ((Doc()->m_myInfo.info.dwIdentity & 0xe) == 0xe &&
		ms_state != STATE_CLOSED)
	{
		CMouseCursor::Instance()->SetItemWindow(false);
		CHelpMsg::Instance()->SetItemHelpMsg(b);
		ms_state = STATE_CLOSING;
		m_curSlot = -1;
		m_hover = -1;
	}

	if (OnlinePlay())
	{
		for (i = 0; i < GOLFDOC()->m_playerNum; i++)
		{
			if (PLAYER(i)->state != 3 && PLAYER(i)->oid == a)
			{
				// HACK: the original loads the guid into a register before the
				// compare; a volatile read keeps the compiler from folding it
				unsigned long guid =
					*(volatile unsigned long*)&Doc()->m_myInfo.info.dwGuid;
				ProcessItem(i, b, a == guid, c);
				return;
			}
		}
	}
	else
		ProcessItem(GOLFDOC()->m_currentPlayer, b, true, c);
}

// 983 0075cf30: posicao da area de itens pelo ponto "p_item_window_area" (tela / 640x480, floor);
// sem o ponto: x = 65, y = barra - 25
static void GetItemWindowAreaPos983(float& x, float& y)
{
	CUiFrameSetManager::Size scl((float)(g_view->GetWidth() / 640.0),
		(float)(g_view->GetHeight() / 480.0));
	CUiFrameSetManager::NamedPoint* pPt =
		CUiFrameSetManager::Instance()->FindNamedPoint("p_item_window_area");
	if (pPt)
	{
		WPoint pt;
		pPt->Calc(scl, pt);
		x = (float)floor((double)pt.x);
		y = (float)floor((double)pt.y);
	}
	else
	{
		x = 65.0f;
		y = COption::Instance()->gGetBar_Y() - 25.0f;
	}
}

// 983 0075c810: o fundo some com o equipamento 0xcc do jogador atual (633450)
void CItemWindow::DrawBackground983()
{
	unsigned char curPlayer = GOLFDOC()->m_currentPlayer;
	if ((Doc()->m_userInfo[curPlayer].userEquip.tidBall &
			0x3ffffff) != 0xcc &&
		m_pOverlay)
		m_pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f), m_dst,
			0x6000000, 0xffffffff, 0.0f, 0);
}

void CItemWindow::Draw()
{
	if (GOLFDOC()->m_tutorialMode >= 15 || GOLFDOC()->m_tutorialMode == 5)
	{
		if (ms_state != STATE_CLOSED)
		{
			DrawBackground983();

			float areaX, areaY;
			GetItemWindowAreaPos983(areaX, areaY);
			unsigned long* pSlot = GetTidItemSlot(GOLFDOC()->m_currentPlayer);

			float x;
			if (IsLocalContent(S4_CARD_SYSTEM))
				x = (10 - PLAYER(GOLFDOC()->m_currentPlayer)->itemNum) * 45.0f +
					m_dst.x + 49.0f - 4.0f;
			else
				x = (10 - PLAYER(GOLFDOC()->m_currentPlayer)->itemNum) * 45.0f +
					m_dst.x + 57.0f;

			WRect rc;
			rc.x = x;
			rc.y = areaY + 8.0f;
			rc.w = rc.h = 54.0f;

			// 983: so desenha o que passa do centro da bola (d_ball_center_x)
			float ballX = 0.0f;
			CUiFrameSetManager::NamedDim* pBallX =
				CUiFrameSetManager::Instance()->FindNamedDim("d_ball_center_x");
			if (pBallX)
				ballX = pBallX->Calc(CUiFrameSetManager::Size(
					(float)(g_view->GetWidth() / 640.0),
					(float)(g_view->GetHeight() / 480.0)));

			for (int i = 0; i < 10; i++, rc.x += 45.0f)
			{
				if (!(ballX < rc.x))
					continue;

				if (pSlot[i] == 0)
					continue;

				WOverlay* pOverlay = NULL;
				std::map<unsigned long, WOverlay*>::iterator it =
					m_iconMap.find(pSlot[i]);
				if (it != m_iconMap.end())
					pOverlay = (*it).second;
				else
				{
					IFF_STRUCT::sItem* pItem =
						ItemManager()->FindItem(pSlot[i]);
					if (pItem)
					{
						pOverlay = g_resrcmng->GetOverlay(
							MakeStr("%s.tga", pItem->c.Icon), 0);
						if (pOverlay)
							m_iconMap.insert(
								std::pair<const unsigned long, WOverlay*>(
									pSlot[i], pOverlay));
					}
				}

				if (pOverlay)
				{
					if (i == m_curSlot)
						pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							WRect(rc.x - 8.0f, rc.y - 8.0f, 70.0f, 70.0f), 0,
							0xffffffff, 0.0f, 0);
					else
						pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
							rc, 0, 0xffffffff, 0.0f, 0);
				}
			}
		}

		if (m_bAnim)
		{
			if (m_bFail)
			{
				if (m_pUI)
					m_pUI->Render(MakeStr("%d", m_frame), m_effX, m_effY, 1.0f,
						0);
			}
			else
			{
				WOverlay* pOverlay;
				std::map<unsigned long, WOverlay*>::iterator it =
					m_iconMap.find(m_itemTid);
				if (it != m_iconMap.end())
					pOverlay = (*it).second;
				else
				{
					IFF_STRUCT::sItem* pItem =
						ItemManager()->FindItem(m_itemTid);
					if (!pItem)
						return;
					pOverlay = g_resrcmng->GetOverlay(
						MakeStr("%s.tga", pItem->c.Icon), 0);
					if (!pOverlay)
						return;
					m_iconMap.insert(std::pair<const unsigned long, WOverlay*>(
						m_itemTid, pOverlay));
				}

				if (pOverlay)
					pOverlay->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
						WRect((float)(int)m_effX, (float)(int)m_effY,
							(float)pOverlay->GetWidth(),
							(float)pOverlay->GetHeight()),
						0, 0xffffffff, 0.0f, 0);
			}
		}
	}
}

void CItemWindow::HandleMsg(const MsgObject& msg)
{
	switch (msg.message)
	{
	case M983(0x194):
		CheckInput();
		break;

	case M983(0x195):
		ms_state = STATE_OPENING;
		ExecuteNow(false);
		m_bOpenAllowed = true;
		break;

	case M983(0x196):
		ms_state = STATE_CLOSING;
		ExecuteNow(false);
		m_bOpenAllowed = false;
		break;

	case M983(0x199):
		*(int*)msg.param1 = ms_state;
		break;

	case M983(0x197):
		if (ms_state != STATE_CLOSED)
			*(int*)msg.param1 = 1;
		else
			*(int*)msg.param1 = 0;
		break;

	case M983(0x198):
		Draw();
		break;

	case M983(0x19a):
		m_bActive = true;
		break;

	case M983(0x19b):
		m_bActive = false;
		break;

	case M983(0x19e):
		UseItem(msg.param1);
		break;

	case M983(0x19f):
		NetUseItem(msg.param1, msg.param2, msg.param3);
		break;

	case M983(0x1a0):
		ResetVars();
		break;

	case 10011:
		ResetUiPosition();
		break;
	}
}

void CItemWindow::ResetUiPosition()
{
	// 983 0075d9d0: posicao pelo helper "p_item_window_area" (0075cf30); recorte = x,y,largura da
	// tela - x, altura do overlay; m_dst.x = x - 509 (o 645 usava 65 - 509 = -444)
	float areaX, areaY;
	GetItemWindowAreaPos983(areaX, areaY);

	if (m_pOverlay)
	{
		WRect clip;
		clip.x = areaX;
		clip.y = areaY;
		clip.w = (float)g_view->GetWidth() - areaX;
		clip.h = (float)m_pOverlay->GetHeight();
		m_pOverlay->SetClippingArea(&clip);
		m_dst.w = (float)m_pOverlay->GetWidth();
		m_dst.h = (float)m_pOverlay->GetHeight();
	}

	m_dst.x = areaX - 509.0f;
	m_dst.y = areaY;
}

bool CItemWindow::IsClosed()
{
	return (ms_state == STATE_CLOSED) ? true : false;
}

unsigned long* CItemWindow::GetTidItemSlot(unsigned char player)
{
	IActor* pActor = GetActor("Tutorial");
	if (pActor && !player)
	{
		unsigned long* pSlot = NULL;
		pActor << MsgObject(NULL, M983(0x152), (int)&pSlot, 0, 0, 0, 0);
		if (pSlot)
			return pSlot;
	}

	return Doc()->m_userInfo[player].userEquip.tidItemSlot;
}
