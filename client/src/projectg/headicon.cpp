#include "minatl.h"
#include "headicon.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "projectg.h"
#include "packet.h"
#include "commonutil.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "fremoticon.h"
#include "netresourcemanager.h"
#include "wresrcmng.h"
#include "woverlay.h"

extern Fresh* g_pFresh;
sPlayerData* PLAYER(unsigned char index);
int float2int(float value);
#if _MSC_VER <= 1600 // VC10: floorf do CRT em __fastcall (codegen do 983); toolset novo usa o da UCRT (math.h)
extern "C" float __fastcall floorf(float value);
#endif

inline unsigned char MyIndex()
{
	bool bGM = (Doc()->m_myInfo.info.dwIdentity >> 1) & 1;
	if (bGM)
	{
		switch (Doc()->m_golfGame.gameType)
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 0xb: // 983 0053a260 (MyIndex) inclui 0xb
			return Doc()->GetIndex(Doc()->m_myInfo.info.dwGalleryGuid);
		default:
			return GOLFDOC()->m_currentPlayer;
		}
	}
	return Doc()->GetIndex(Doc()->m_myInfo.info.dwGuid);
}

CHeadIcon::CHeadIcon()
{
	m_bShow = true;

	for (int i = 0; i < 4; i++)
	{
		m_head[i].icon = 0xffff;
		m_head[i].time = 0;
		m_head[i].pLevel = NULL;
	}

	m_pEmoticon = g_resrcmng->GetOverlay("[emoti_object.png", 0);

	m_dst.w = 64.0f;
	m_dst.h = 64.0f;
	m_src.w = 0.25f;
	m_src.h = 0.25f;

	m_iconMap.clear();

	for (unsigned char i = 0; i < 48; i++) // 983 (006e5594): 0x30 ícones (eram 44)
	{
		if (i == 13)
			continue;

		IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(0x18000000 | i);

		if (pItem)
		{
			WOverlay* pOverlay =
				g_resrcmng->GetOverlay(MakeStr("%s.tga", pItem->c.Icon), 0);
			if (pOverlay)
				m_iconMap[pItem->c.TypeId] = pOverlay;
		}
	}

	// 983 (006e559b): campos novos (s100)
	m_podiumOffX983 = 0.0f;
	m_playerNum983 = -1;
	m_podiumOffY983 = 0.0f;
}

CHeadIcon::~CHeadIcon()
{
	if (g_resrcmng && m_pEmoticon)
	{
		g_resrcmng->Release(m_pEmoticon);
		m_pEmoticon = NULL;
	}

	for (int i = 0; i < 4; i++)
	{
		if (g_resrcmng && m_head[i].pLevel)
		{
			g_resrcmng->Release(m_head[i].pLevel);
			m_head[i].pLevel = NULL;
		}
	}

	for (std::map<unsigned long, WOverlay*>::iterator it = m_iconMap.begin();
		it != m_iconMap.end(); ++it)
	{
		if (g_resrcmng && (*it).second)
		{
			g_resrcmng->Release((*it).second);
			(*it).second = NULL;
		}
	}

	m_iconMap.clear();
}

// 983 (006e4a80, s100): -1 = o do CGolfDoc; o pódio do Grand Prix grava 3 ou 4 em m_playerNum983
inline int CHeadIcon::GetPlayerNum983()
{
	if (m_playerNum983 == -1)
		return GOLFDOC()->m_playerNum;
	return m_playerNum983;
}

// 983 (006e4aa0): limite pelo GetPlayerNum983, sem o ramo do ladder; quem tem o bit 0x8000 da identidade (anfitrião do Grand
// Prix) usa "gp_host.tga?level.bres"
void CHeadIcon::SetLevel()
{
	int num = GetPlayerNum983();
	if (num > 4)
		num = 4;

	for (int i = 0; i < num; i++)
	{
		if (g_resrcmng && m_head[i].pLevel)
		{
			g_resrcmng->Release(m_head[i].pLevel);
			m_head[i].pLevel = NULL;
		}

		IFF_STRUCT::sSkin* pSkin = NULL;
		if (Doc()->m_userInfo[i].userEquip.tidSkin[5])
			pSkin = ItemManager()->FindSkin(Doc()->m_userInfo[i].userEquip.tidSkin[5]);
		if (pSkin)
			m_head[i].pLevel =
				g_resrcmng->GetOverlay(MakeStr("%s.tga", pSkin->c.Icon), 0);
		else
			m_head[i].pLevel = g_resrcmng->GetOverlay(
				MakeStr("level_%03d.tga", Doc()->m_userInfo[i].stat.Level + 1), 0);

		if (Doc()->m_userInfo[i].info.dwIdentity & 0x8000)
		{
			if (m_head[i].pLevel && g_resrcmng)
			{
				g_resrcmng->Release(m_head[i].pLevel);
				m_head[i].pLevel = NULL;
			}
			m_head[i].pLevel = g_resrcmng->GetOverlay("gp_host.tga?level.bres", 0);
		}
	}
}

void CHeadIcon::SetPlayerPos(int index, WVector& pos)
{
	if (index < 0 || index >= sizeof(m_head) / sizeof(m_head[0]))
		return;

	WVector screen = g_view->Projection(pos);

	sPlayerHead* pHead = &m_head[index];
	WVector dir = pos - g_view->camera.pivot;
	pHead->screen.x = screen.x;
	pHead->screen.y = screen.y;
	pHead->screen.z = screen.z;
	pHead->dist = dir.Magnitude();
}

bool CHeadIcon::SetIcon(unsigned long uid, unsigned short icon)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		CHeadIcon();

	unsigned char index = Doc()->GetIndex(uid);
	if (index < 4)
	{
		sPlayerHead* pHead = &m_head[index];
		pHead->icon = icon;
		pHead->time = 0;
		return true;
	}
	return false;
}

bool CHeadIcon::SendIcon(unsigned short icon)
{
	if (OnlinePlay())
	{
		WSendPacket packet((enumClientPacket)0x18);
		packet.Encode2(icon);
		packet.Send(TO_GAME);

		return SetIcon(Doc()->m_myInfo.info.dwGuid, icon);
	}

	return SetIcon(GOLFDOC()->m_currentPlayer, icon);
}

void CHeadIcon::Process(float dt)
{
	int n = GetPlayerNum983(); // 983 (006e4bc0)
	int num = min(4, n);

	for (int i = 0; i < num; i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state == 3)
			continue;

		m_head[i].time += dt;
		if (m_head[i].time > 3.0f && m_head[i].icon != 0x1d)
		{
			m_head[i].icon = 0xffff;
			m_head[i].time = 0;
		}
	}
}

// 983 (006e6060): limite pelo GetPlayerNum983, pula os tipos 0/7/1 (nao 3) e o tipo 3 do ramo 0x1000 cai no jogador da vez
void CHeadIcon::DisplayOverPlayer()
{
	if (!m_bShow)
		return;

	if (CProjectG::Instance()->HidePrivacy())
		return;

	if (GOLFDOC()->m_gameMode & 0x20000)
	{
		for (unsigned char i = 0; i < GetPlayerNum983(); i++)
		{
			if ((Doc()->m_golfGame.gameType == 0 ||
					Doc()->m_golfGame.gameType == 7 ||
					Doc()->m_golfGame.gameType == 1) &&
				GOLFDOC()->GetPlayer(i)->state == 3)
				continue;

			DisplayOverPlayer(i, true);
		}
	}
	else if (GOLFDOC()->m_gameMode & 0x1000)
	{
		if (Doc()->m_golfGame.gameType == 1)
		{
			DisplayOverPlayer(MyIndex(), false);
			if (PLAYER(MyIndex())->teammate != 0xff)
				DisplayOverPlayer(PLAYER(MyIndex())->teammate, false);
		}
		else if (Doc()->m_golfGame.gameType == 7)
		{
			DisplayOverPlayer(MyIndex(), false);
		}
		else
			DisplayOverPlayer(GOLFDOC()->m_currentPlayer, false);
	}
	else
		DisplayOverPlayer(GOLFDOC()->m_currentPlayer, false);
}

void CHeadIcon::DisplayOverPlayer(unsigned char index, bool bName)
{
	// HACK: preserve emission order by adding DCE'd dependencies
	if (0)
		SetIcon(0, 0);

	sPlayerHead* pHead = &m_head[index];

	if (pHead->screen.z < 0.0f)
		return;

	float dist = pHead->dist;
	if (pHead->screen.z <= 0.0f || 1.0f <= pHead->screen.z || dist > 640.0f)
		return;

	// 983 006e5760: deslocamento do pódio (+0xf4/+0xf8) ao quadrado / dist, com sinal; soma no src e no dst
	float offX = m_podiumOffX983 / dist * m_podiumOffX983;
	if (m_podiumOffX983 < 0.0f)
		offX = -offX;
	float offY = m_podiumOffY983 / dist * m_podiumOffY983;
	if (m_podiumOffY983 < 0.0f)
		offY = -offY;

	m_dst.x = float2int(pHead->screen.x - 60.0f - 200.0f / dist) + offX;
	m_dst.y = float2int(pHead->screen.y - 52.0f - 800.0f / dist) + offY;

	if (pHead->icon < 0xffff)
	{
		m_src.x = offX + pHead->icon % 4 * 0.25f;
		m_src.y = offY + pHead->icon % 16 / 4 * 0.25f;
		m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xffffffff, 0, 0);
	}
	else if (PLAYER(index)->item)
	{
		m_src.y = offY;
		// 983: falha do item = 0x18000031 (o 645 usava 0x1800002d, hoje item de vento de 1 m)
		if (PLAYER(index)->item == 0x18000031)
		{
			m_src.x = offX + 0.5f;
			m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xffffffff, 0, 0);
		}
		else if (PLAYER(index)->item != 0x1800000e)
		{
			m_src.x = offX;
			m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xffffffff, 0, 0);
			m_iconMap[PLAYER(index)->item]->Render(g_view,
				WRect(0.0f, 0.0f, 1.0f, 1.0f),
				WRect(m_dst.x + 28.0f, m_dst.y + 30.0f, 42.0f, 42.0f), 0,
				0xffffffff, 0, 0);
		}
	}

	if (GOLFDOC()->m_gameMode == 0x800 && Doc()->m_golfGame.gameType == 10)
		return;

	if (GOLFDOC()->m_gameMode == 0x200)
		return;

	// 983 006e5aee (FUN_00500200): salas de tipo real 0xc/0xe/0x13 nao desenham nome/nivel sobre a cabeca
	unsigned char realType = Doc()->m_roomInfo.realGameType;
	if (realType == 0xc || realType == 0xe || realType == 0x13)
		return;

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	FrEmoticon* pEmoticon = g_pFresh->GetManager()->GetEmoticon();

	if (pGDI == NULL)
		return;

	pGDI->SetAlpha(1.0f);

	const char* nick = Doc()->m_userInfo[index].info.sNick[0]
		? Doc()->m_userInfo[index].info.sNick
		: "???";

	float width =
		pEmoticon ? pEmoticon->GetTextWidth(nick) : pGDI->GetTextExtendOld983(nick);

	bool bGuild = Doc()->m_userInfo[index].info.dwGuildId &&
		!Doc()->m_userInfo[index].info.IsIdentity(0x14);

	WRect rc;
	rc.w = pHead->pLevel ? (float)pHead->pLevel->GetWidth() : 0;
	rc.h = pHead->pLevel ? (float)pHead->pLevel->GetHeight() : 0;

	float w;
	if (bName)
	{
		w = rc.w;
		if (bGuild)
			w = rc.w + 27.0f;
	}
	else
		w = rc.w + width + 5.0f;
	// 983 006e5760: o deslocamento do podio (offX/offY) tambem vale para o nome/nivel
	rc.x = floorf(pHead->screen.x - w * 0.5f) + offX;
	rc.y = floorf(pHead->screen.y - 30.0f - 1200.0f / dist) + offY;

	if (IsMassGame())
	{
		const Bitmap* pRank = NULL;

		if (Doc()->m_golfGame.gameType != 10)
		{
			if (Doc()->m_golfGame.gameType == 4)
				pRank =
					g_pFresh->GetManager()->GetBitmap("FRAMES4", "30in_rank");
			else if (Doc()->m_golfGame.gameType == 6)
				pRank = g_pFresh->GetManager()->GetBitmap("FRAMES4",
					"30in_rank_02");
			else if (Doc()->m_golfGame.gameType == 5)
			{
				if (Doc()->m_rivalList[MyIndex()].team == 0)
					pRank = g_pFresh->GetManager()->GetBitmap("FRAMES4",
						"30in_team_redteam_rank");
				else
					pRank = g_pFresh->GetManager()->GetBitmap("FRAMES4",
						"30in_team_blueteam_rank");
			}
		}

		if (pRank)
		{
			if (Doc()->m_golfGame.gameType == 6)
				pGDI->DrawTexture(pRank,
					WRect(rc.x - 30.0f, rc.y - 1.0f, (float)pRank->Width(),
						(float)pRank->Height()),
					0xffffffff, 0);
			else
				pGDI->DrawTexture(pRank,
					WRect(rc.x - 19.0f, rc.y - 1.0f + 2.0f,
						(float)pRank->Width(), (float)pRank->Height()),
					0xffffffff, 0);
		}

		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(0xffffffff, 0xffffffff);

		if (Doc()->m_golfGame.gameType == 6)
		{
			pGDI->PrintText(WPoint(rc.x - 15.0f, rc.y + 2.0f), 1,
				MakeStr("%d \xc1\xb6", Doc()->m_guildTeam), NULL);
		}
		else if (Doc()->m_golfGame.gameType != 10)
		{
			pGDI->PrintText(WPoint(rc.x - 10.0f, rc.y + 2.0f), 1,
				MakeStr("%d", Doc()->m_rivalList[MyIndex()].rank), NULL);
		}
	}

	if (pHead->pLevel && !Doc()->m_offlinePlay)
	{
		pHead->pLevel->Render(g_view, WRect(0.0f, 0.0f, 1.0f, 1.0f),
			WRect(rc.x, rc.y + 8.0f - rc.h * 0.5f, rc.w, rc.h), 0, 0xffffffff,
			0, 0);
	}

	float x = rc.x + rc.w + 5.0f, y = rc.y;

	if (bGuild)
	{
		const Bitmap* pEmblem = NetResourceManager::Instance()->GetEmblem983(
			Doc()->m_userInfo[index].info.dwGuildId, Doc()->m_userInfo[index].info.dwEmblemVer);

		if (pEmblem)
		{
			pGDI->DrawTexture(pEmblem,
				WRect(x, rc.y - 6.0f, (float)pEmblem->Width(),
					(float)pEmblem->Height()),
				0xffffffff, 0);
			x += 27.0f;
		}
	}

	if (bName)
	{
		x = pHead->screen.x - width * 0.5f;
		y = rc.y + 20.0f;
	}

	pGDI->SetTextStyle(2);
	pGDI->SetTextColor(0xffffffff, 0xff000000);

	WPoint pt;
	pt.x = (float)float2int(x);
	pt.y = (float)float2int(y);

	if (pEmoticon)
		pEmoticon->PrintText(pt, 0, nick, 0xffffffff);
	else
		pGDI->PrintText(pt, 0, nick, NULL);

	pGDI->SetAlpha(1.0f);
}

void CHeadIcon::DisplayBesideBoard(float left, float right)
{
	if (!(GOLFDOC()->m_gameMode & 0xfffff77e))
		return;

	right -= 64.0f;

	// 983 (006e4c60): limite pelo GetPlayerNum983 (006e4a80), relido a cada volta
	for (unsigned char i = 0; i < GetPlayerNum983(); i++)
	{
		if (GOLFDOC()->GetPlayer(i)->state == 3)
			continue;

		sPlayerHead* pHead = &m_head[i];

		if (GOLFDOC()->GetPlayer(i)->chatBlock)
		{
			m_src.x = 0.75f;
			m_src.y = 0;

			if (Doc()->m_golfGame.gameType == 1)
			{
				m_dst.x = GOLFDOC()->GetPlayer(i)->team == 0 ? left : right;
				if (GOLFDOC()->GetPlayer(i)->teammate == 0xff)
					m_dst.y = 10.0f;
				else
					m_dst.y =
						i < GOLFDOC()->GetPlayer(i)->teammate ? 10.0f : 60.0f;
				sPlayerData* pPlayer = GOLFDOC()->GetPlayer(i);
				m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xccffffff, 0,
					pPlayer->team == 0);
			}
			else
			{
				m_dst.x = (i & 2) ? right : left;
				m_dst.y = (i & 1) ? 81.0f : 10.0f;
				m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xccffffff, 0,
					!(i & 2));
			}
		}
		else if (i != GOLFDOC()->m_currentPlayer && pHead->icon < 0xffff)
		{
			int row = pHead->icon % 16 / 4, col = pHead->icon % 4;

			m_src.x = col * 0.25f;
			m_src.y = row * 0.25f;

			if (Doc()->m_golfGame.gameType == 1)
			{
				m_dst.x = GOLFDOC()->GetPlayer(i)->team == 0 ? left : right;
				if (GOLFDOC()->GetPlayer(i)->teammate == 0xff)
					m_dst.y = 10.0f;
				else
					m_dst.y = i < PLAYER(i)->teammate ? 10.0f : 60.0f;
				sPlayerData* pPlayer = GOLFDOC()->GetPlayer(i);
				m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xccffffff, 0,
					pPlayer->team == 0);
			}
			else
			{
				m_dst.x = (i & 2) ? right : left;
				m_dst.y = (i & 1) ? 81.0f : 10.0f;
				m_pEmoticon->Render(g_view, m_src, m_dst, 0, 0xccffffff, 0,
					!(i & 2));
			}
		}
	}
}
