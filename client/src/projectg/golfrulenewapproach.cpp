#include "minatl.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "camerawork.h"
#include "golfball.h"
#include "capturedbg.h"
#include "projectg.h"

CGolfRuleNewApproach::CGolfRuleNewApproach(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;

	GOLFDOC()->m_bNoStat = true;
}

CGolfRuleNewApproach::~CGolfRuleNewApproach()
{
}

void CGolfRuleNewApproach::SetReady()
{
	CGolfRuleBase::SetReady();

	GOLFDOC()->ApproachDataAllClear();
}

// JP 983 [12] (006a9710): sem a entrada do jogador (0x2716) da base; so a camera 983 0xcc (tee-back)
void CGolfRuleNewApproach::ReadyNextShot()
{
	CCameraWork::Instance()->SelectCamera(CCameraWork::kTeeBackCamera983); // 983 0xcc = its 204
}

// JP 983 [10] (006a9790): o bloco do tempo da Approach que o 645 tinha no CGolfRule::OnProcess
// B1324: 006a9790 le o inicio em Doc+0x1057c e o limite em Doc+0x106a0 (clientsetting.cpp grava 0x106a0 cru)
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameTimeLimit) == 0x106a0);
void CGolfRuleNewApproach::V10_983(float time)
{
	if (!(GOLFDOC()->m_gameMode & 0x800cc0) && Doc()->m_approachStartTime != 0 &&
		Doc()->m_golfGame.gameTimeLimit + Doc()->m_approachStartTime < g_CurrentTime)
	{
		*(unsigned long*)((char*)&GolfBall() + 0x68) = 0;
		m_pGolfRule->SendShotResult();
		if (CCapturedBg::Instance()->IsCaptured())
			CCapturedBg::Instance()->Reset();
		m_pGolfRule->ApproachTimeOut983();
	}
}

// JP 983 [11] (006a96b0): return 0
bool CGolfRuleNewApproach::V11_983(bool* pbCaddie)
{
	return false;
}

void CGolfRuleNewApproach::SetGrade()
{
}

// B1343: 983 006a9830 = nosso (Doc+0x10534+(oid%18)*4 e Doc+(0x415e-oid%18)*4 = m_holeRandom[17-oid%18]; area
// min +0x110/+0x118, max +0x11c/+0x124; 1/128 double 00d19768). SetPlayer 006a98e0 = nosso campo a campo (no do
// sSlotInfo = +8; distancia sqrt*10.0*0.3125 doubles 00cff068/00d08c80, floor; offline LoadMapCheckData(+0x354)).
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeRandom) == 0x10534 && offsetof(CSharedDoc, m_slotList) == 0x109fc);
C_ASSERT(offsetof(sSlotInfo, connectionRank) == 0x2e && offsetof(sSlotInfo, GuildId) == 0x58 &&
	offsetof(sSlotInfo, dwEmblemVer983) == 0x68 && offsetof(sSlotInfo, dwUserUID) == 0x6c && offsetof(sSlotInfo, tidMascot) == 0xca &&
	offsetof(sSlotInfo, pangma) == 0xce);
C_ASSERT(offsetof(sRivalData, guildUID) == 0x174 && offsetof(sRivalData, guildEmblemVer983) == 0x178);
WVector CGolfRuleNewApproach::GetRandomStartPos(unsigned long oid)
{
	Waabb area = GOLFDOC()->m_pPolySoup->GetExtraHoleStartArea();

	WVector pos;

	pos.x = ((oid + Doc()->m_holeRandom[oid % 18]) & 0x7f) *
			(area.max.x - area.min.x) * 0.0078125f +
		area.min.x;
	pos.y = 0.0f;
	pos.z = ((oid + Doc()->m_holeRandom[17 - oid % 18]) & 0x7f) *
			(area.max.z - area.min.z) * 0.0078125f +
		area.min.z;

	return pos;
}

void CGolfRuleNewApproach::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		GOLFDOC()->MakePlayer(1);

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;
		Doc()->m_rivalList.reserve(slotList.size());

		for (std::list<sSlotInfo>::iterator it = slotList.begin();
			it != slotList.end(); ++it)
		{
			if ((*it).dwGuid == -1)
			{
				GOLFDOC()->GetPlayer(0)->state = 3;
				continue;
			}

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

			rival.uid = (*it).dwUserUID;
			rival.oid = (*it).dwGuid;
			rival.ballPos = GetRandomStartPos(rival.oid);

			rival.titleTypeId = (*it).dwTitle;
			rival.team = (*it).bTeam;
			rival.flag = (*it).gender;

			memset(rival.holeStroke, 0, sizeof(rival.holeStroke));
			rival.observerState = 1;
			rival.observerRank = 1;
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
			rival.guildPoint = 0;
			WVector diff;
			diff = WVector(GOLFDOC()->GetHoleData().pin) - rival.ballPos;

			rival.approachDistance = (int)floor(
				(double)((sqrtf(diff.x * diff.x + diff.z * diff.z) * 10.0f) *
					0.3125f));

			rival.approachResultDistance = -1;
			rival.approachTime = -1;
			rival.capability = (*it).dwIdentity;
			rival.mascotTypeId = (*it).tidMascot;
			rival.pangMastery = (*it).pangma[0];
			rival.pangNitro = (*it).pangma[1];
			Doc()->m_rivalList.push_back(rival);

			if ((*it).dwGuid == MyGuid(true))
			{
				sPlayerData* pPlayer = GOLFDOC()->GetPlayer(0);
				pPlayer->team = (*it).bTeam;
				m_pGolfRule->AddPlayer(0, (*it).sNick, (*it).dwUserUID,
					(*it).dwGuid, Doc()->m_userInfo[0].caddieInfo.tid);

				GOLFDOC()->GetHoleData().tee = rival.ballPos;
			}

			Doc()->ClearUserInfoTimeMap((*it).dwUserUID);
		}

		GOLFDOC()->RefreshPlayerNum();

		m_pGolfRule->SendHoleData();
	}
	else
	{
		Doc()->m_golfGame.shotTimeLimit = 0;
		COption::Instance()->dSetNumPlayers(1);
		GOLFDOC()->MakePlayer(1);

		m_pGolfRule->LoadShot("save/pangya_000.sav");

		GOLFDOC()->m_pPolySoup->LoadMapCheckData(
			m_pGolfRule->GetShotData()->hole);
	}
	Doc()->m_approachStartTime = 0;
}

// JP 983 [15] (006a96d0): so chama o 006a8130
void CGolfRuleNewApproach::ShowScore(int arg)
{
	ShowScore983();
}
