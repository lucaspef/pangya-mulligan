#include "minatl.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "../shared/localize.h"
#include "s5/classicserver/classicserver.h"
#include <math.h>
#include "fresh.h"
extern Fresh* g_pFresh; // 00f02340
#include "jp/grandprixawarddlg.h"
#include "vc10_layout.h"
C_ASSERT_VC10(offsetof(CSharedDoc, m_awardItem) == 0x1049a && offsetof(CSharedDoc, m_guildRoundScore) == 0x106e4 && offsetof(CSharedDoc, m_unused47c8) == 0x106f8); // A747: ex-crus
#include "soundmanager.h"
#include "camerawork.h"


// Grand Prix (0x14/0x15)
#define IsGrandPrix983() ((unsigned)(Doc()->m_roomInfo.realGameType - 0x14) <= 1)

// JP 983 (006a5030): no "shortgame" liga o +0x195 do CGolfDoc
CGolfRule30Battle::CGolfRule30Battle(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;
	if (S5::CLASSICSRV::IsShortGame983()) // 00b305b0
		GOLFDOC()->m_b195_983 = true;
}

CGolfRule30Battle::~CGolfRule30Battle()
{
}

void CGolfRule30Battle::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pGolfRule->SendHoleData();

		GOLFDOC()->MakePlayer(1);

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;
		Doc()->m_rivalList.clear();
		Doc()->m_rivalList.reserve(slotList.size());

		for (std::list<sSlotInfo>::iterator it = slotList.begin();
			it != slotList.end(); ++it)
		{
			if ((*it).dwGuid == -1)
			{
				GOLFDOC()->GetPlayer(0)->state = 3;
				continue;
			}

			if (IsGrandPrix983() && (*it).bGrandPrixAI983 != 0)
				continue;

			Doc()->SetIndex((*it).dwGuid);

			sRivalData rival;
			rival.Clear983(); // 983
			rival.hole = Doc()->m_holeOrder[0];
			rival.rank = 1;
			rival.state = 0;
			rival.totalStroke = 0;
			rival.totalScore = 0;
			rival.totalPang = 0;
			rival.totalBonusPang = 0;
			rival.observerState = 1;
			rival.observerRank = 1;
			rival.uid = (*it).dwUserUID;
			rival.oid = (*it).dwGuid;
			if (S5::CLASSICSRV::IsShortGame983() && GOLFDOC()->GetHoleData().par != 3)
				rival.ballPos = GetShortGameBallPos983((*it).dwGuid);
			else
				rival.ballPos = GOLFDOC()->GetHoleData().tee;

			rival.titleTypeId = (*it).dwTitle;

			rival.team = (*it).bTeam;
			rival.flag = (*it).gender;

			memset(rival.holeStroke, 0, sizeof(rival.holeStroke));
			memset(rival.holeScore, 0, sizeof(rival.holeScore));
			memset(rival.holePang, 0, sizeof(rival.holePang));

			rival.order = (*it).connectionRank;
			rival.quitOrder = slotList.size();
			rival.finishOrder = slotList.size();
			rival.level = (*it).level;
			strcpy(rival.nickname, (*it).sNick);
			strcpy(rival.guildName, (*it).sGuild);
			rival.guildUID = (*it).GuildId;

			rival.guildEmblemVer983 = (*it).dwEmblemVer983; // 983
			strcpy(rival.guildMark, (*it).szEmblemName);

			rival.capability = (*it).dwIdentity;
			rival.mascotTypeId = (*it).tidMascot;
			rival.pangMastery = (*it).pangma[0];
			rival.guildPoint = 0;
			rival.pangNitro = (*it).pangma[1];

			rival.prevRank983 = 0;
			rival.tidChar983 = (*it).tidChar;
			rival.rankTieBreak983 = (*it).rankTieBreak983;
			rival.bGrandPrixAI983 = (*it).bGrandPrixAI983;
			rival.grandPrixAIId983 = (*it).grandPrixAIId983;

			if (S5::CLASSICSRV::IsShortGame983())
			{
				// distancia da bola ao pino em jardas (x10 * 0.3125)
				const WVector& pin = GOLFDOC()->GetHoleData().pin;
				WVector diff(pin.x - rival.ballPos.x, 0.0f, pin.z - rival.ballPos.z);
				float dist = diff.Magnitude();
				dist = (float)floor(dist * 10.0 * 0.3125);
				rival.approachDistance = (int)dist;
				rival.approachResultDistance = -1;
				rival.approachTime = -1;
			}

			Doc()->m_rivalList.push_back(rival);

			if ((*it).dwGuid == MyGuid(true))
			{
				sPlayerData* pPlayer = GOLFDOC()->GetPlayer(0);
				pPlayer->team = (*it).bTeam;
				m_pGolfRule->AddPlayer(0, (*it).sNick, (*it).dwUserUID,
					(*it).dwGuid, Doc()->m_userInfo[0].caddieInfo.tid);
				if (S5::CLASSICSRV::IsShortGame983())
					GOLFDOC()->GetHoleData().tee = rival.ballPos;
			}

			Doc()->ClearUserInfoTimeMap((*it).dwUserUID);
		}

		if (IsGrandPrix983())
			AfxGetTask()->GetMainActor()->SendMsg(NULL, "AIPlayer", 0x350, 0, 0, 0, 0);
		GOLFDOC()->RefreshPlayerNum();
	}
	else
	{
		Doc()->m_golfGame.shotTimeLimit = 0;
		COption::Instance()->dSetNumPlayers(1);
		GOLFDOC()->MakePlayer(1);

		m_pGolfRule->LoadShot(Doc()->GetFileName());

		GOLFDOC()->m_pPolySoup->LoadMapCheckData(
			m_pGolfRule->GetShotData()->hole);
	}
}

void CGolfRule30Battle::SetGrade()
{
}

void CGolfRule30Battle::SetReady()
{
	CGolfRuleBase::SetReady();
}

// JP 983 [13] (006a5230): o da base sem OnlinePlay() e sem SetNetMsgWait; no Grand Prix (0x14/0x15) reinicia o relogio
void CGolfRule30Battle::RequestShotReview(float time)
{
	if (!(bool)GOLFDOC()->m_bTeeShotReady)
	{
		GOLFDOC()->m_bTeeShotReady = 1;
		WSendPacket packet((enumClientPacket)0x34);
		packet.Send(TO_GAME);
		SendShotReview983();
		m_pGolfRule->CheckNetMsgWait_Send("DoPlayerPreview");
		if (IsGrandPrix983())
		{
			Doc()->m_approachStartTime = g_CurrentTime;
			Doc()->SetRivalState983(0);
			AfxGetTask()->GetMainActor()->SendMsg(NULL, "Screen", 0x248, 1, 0, 0, 0);
			// CGolfRule::m_turnStartTime (+0x648): os campos do CGolfRule so existem com REBANG_GOLFRULE_PCH (golfrule.cpp)
			*(unsigned long*)((char*)m_pGolfRule + GOLFRULE_OFS_TURNSTART983) = g_CurrentTime; // B1463: 0x648 so no VC10
		}
	}
}
// JP 983 [7] (006a50b0): a da base e zera a tabela de 12 pares (guid -1 / 0) e 6 campos do CSharedDoc (+0x106e4)
void CGolfRule30Battle::V7_983()
{
	CGolfRuleBase::V7_983();
	for (int off = 0, n = 12; n != 0; --n, off += 8)
	{
		*(unsigned long*)((char*)&Doc()->m_awardItem[0].uid + off) = (unsigned long)-1;
		*(unsigned long*)((char*)&Doc()->m_awardItem[0].pang + off) = 0;
	}
	unsigned long* p = Doc()->m_guildRoundScore; // 6 dwords: m_guildRoundScore[2], m_unused47bc, m_teamPang[2], m_unused47c8
	p[0] = 0;
	p[1] = 0;
	p[2] = 0;
	p[3] = 0;
	p[4] = 0;
	p[5] = 0;
}

// JP 983 [12] (006a5120): com o jogador da vez == o primeiro do buraco (e, no shortgame, par 3) so a "Player" muda
void CGolfRule30Battle::ReadyNextShot()
{
	if (GOLFDOC()->m_currentHole == Doc()->m_holeOrder[0]
		&& (!GOLFDOC()->m_b195_983 || GOLFDOC()->GetHoleData().par == 3))
	{
		MsgObject msg(m_pGolfRule, 0x2716, (int)"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", 0, 0, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg);
		CCameraWork::Instance()->SelectCamera(0xcb);
	}
	else
	{
		MsgObject msg1(m_pGolfRule, 0x2716, (int)"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", -3, 1, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg1);
		MsgObject msg2(m_pGolfRule, 0x155, 1, 0, 0, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg2);
		CCameraWork::Instance()->SelectCamera(CCameraWork::kTeeBackCamera983); // 983 006a521d: 0xcc = its 204
	}
}

// JP 983 (006a5500): posicao da bola do shortgame dentro do retangulo da pista (sorteio por guid / tabela do buraco)
WVector CGolfRule30Battle::GetShortGameBallPos983(unsigned long guid)
{
	// B1573: era ps+0x110..0x124 cru (VC10); no cl 14.44 m_extraHoleStartArea fica em 0xfc
	C_ASSERT_VC10(offsetof(WPolySoup, m_extraHoleStartArea) == 0x110);
	const Waabb& area = GOLFDOC()->m_pPolySoup->m_extraHoleStartArea;
	unsigned long r = guid % 18;
	float a = (float)((Doc()->m_holeRandom[r] + guid) & 0x7f);
	float b = (float)((Doc()->m_holeRandom[17 - r] + guid) & 0x7f);
	WVector pos;
	pos.x = a * (area.max.x - area.min.x) * 0.0078125 + area.min.x;
	pos.y = 0.0f;
	pos.z = b * (area.max.z - area.min.z) * 0.0078125 + area.min.z;
	return pos;
}

// JP 983 [14] (006a58c0): no Grand Prix (0x14/0x15) a premiacao e a do 006a55b0
void CGolfRule30Battle::V14_983()
{
	if (!IsGrandPrix983())
		CGolfRuleBase::V14_983();
	else
		StartGrandPrixAwards983();
}

// JP 983 (006a55b0): comeco da premiacao do Grand Prix
void CGolfRule30Battle::StartGrandPrixAwards983()
{
	m_pGolfRule->SendMsg983("GolfBg", MsgObject(m_pGolfRule, 0x33d, 0, 0, 0, 0, 0));
	m_pGolfRule->SendMsg983("GolfBg", MsgObject(m_pGolfRule, 0x17e, 0, 0, 0, 0, 0));
	WPolySoup::sCamera* pCam = GOLFDOC()->m_pPolySoup->FindCamera("\xbd\xc3\xbb\xf3\xbd\xc4\xc4\xab\xb8\xde\xb6\xf3");
	float fov;
	if (pCam)
	{
		g_camera = pCam->mat;
		fov = pCam->fov * 0.017453292f;
	}
	else
	{
		g_camera.Reset();
		fov = 0.9599311f;
	}
	g_view->SetFOV(fov);
	m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, 0x161, 0, 0, 0, 0, 0));
	AfxGetTask()->GetActor("DummyPlayer") << MsgObject(NULL, 0x341, 0, 0, 0, 0, 0);

	static int s_podiumCam = 1;
	static int s_otherCam = 6;
	bool bPodium = FrGrandPrixAwardDlg::IsMeOnPodium983();
	int idx = bPodium ? s_podiumCam : s_otherCam;
	CGolfRule* pRule = m_pGolfRule;
	*(float*)((char*)pRule + GOLFRULE_OFS_MODEDUR983) = CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)1,
		MakeStr("\xb1\xd7\xb6\xfb\xc7\xc1\xb8\xae" "0%d", idx), WVector::ZERO);
	if (FrGrandPrixAwardDlg::s_pInstance == NULL)
		FrGrandPrixAwardDlg::Create983(g_pFresh->GetManager(), NULL);
	FrGrandPrixAwardDlg::s_pInstance->StartTimer983(*(float*)((char*)m_pGolfRule + GOLFRULE_OFS_MODEDUR983), std::string(""));
	FrGrandPrixAwardDlg::s_pInstance->Open(FRESH_PFN_RESULT(), 0);
	if (bPodium)
	{
		if (++s_podiumCam > 5)
			s_podiumCam = 1;
	}
	else
	{
		if (++s_otherCam > 7)
			s_otherCam = 6;
	}
	g_audio->PlaySfx("gp_awards_amb");
}

// JP 983 [15] (006a5340): o argumento e o tempo (float) do quadro
C_ASSERT_VC10(offsetof(CGolfDoc, m_bNetGameEnd) == 0x15c && offsetof(CGolfDoc, m_bApproach794) == 0x7b4); // A749: ex-crus GOLFDOC()+0x15c/+0x7b4

void CGolfRule30Battle::ShowScore(int arg)
{
	float time = *(float*)&arg;
	switch (Doc()->m_roomInfo.realGameType)
	{
	case 12:
	case 14:
	case 19:
		if (!m_pGolfRule->IsModeTimeOver983())
			return;
		break;
	default:
		{
			bool flag = false;
			m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, 0x16a, (int)&flag, 0, 0, 0, 0));
			if (!flag)
				return;
		}
		break;
	}
	if (OnlinePlay())
	{
		if (Doc()->m_bGameOver == 0 && GOLFDOC()->m_bNetGameEnd == 0)
		{
			if (IsGrandPrix983())
			{
				ProcessCamera983(time);
				return;
			}
			if (m_pGolfRule->IsNetWaiting983())
				return;
			NextHole983();
			ShowHoleBg983(Doc()->m_golfGame.gameType);
			m_pGolfRule->CheckNetMsgWait_Send("DoScoreBoard");
			return;
		}
		if (GOLFDOC()->m_bApproach794 == 0)
			OnHoleIn();
		if (IsGrandPrix983())
		{
			bool flag = false;
			AfxGetTask()->GetActor("DummyPlayer") << MsgObject(NULL, 0x342, (int)&flag, 0, 0, 0, 0);
			if (flag)
				m_pGolfRule->ChangeGameMode(0x20000);
		}
		else
			m_pGolfRule->ReturnLobby(false);
	}
	else
		NextHoleOffline983();
}

// JP 983 (006a5000)
void CGolfRule30Battle::ProcessCamera983(float time)
{
	CCameraWork::Instance()->Process(time, 0.0f);
}
