#include "minatl.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "camerawork.h"

CGolfRuleGuildMatch::CGolfRuleGuildMatch(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;
}

CGolfRuleGuildMatch::~CGolfRuleGuildMatch()
{
}

// B1344: 983 006a8b70 (vtable 00d1982c [0]) = nosso campo a campo: SendHoleData 006735c0 antes do MakePlayer; rival sem tee
// aleatorio (tee do buraco Doc+0x11024), rank 1, order/quit/finish, capability/mascote/pangma; eu -> team +0x4e9 e AddPlayer
// (caddie Doc+0x6a07), sem mexer no tee; offline LoadShot(Doc+0x11230) + LoadMapCheckData(+0x354). [1]/[2]/[16] = 006a8980/90/b0.
C_ASSERT(offsetof(sSlotInfo, sNick) == 0x4 && offsetof(sSlotInfo, sGuild) == 0x1a && offsetof(sSlotInfo, dwIdentity) == 0x2f &&
	offsetof(sSlotInfo, dwTitle) == 0x33 && offsetof(sSlotInfo, level) == 0x55 && offsetof(sSlotInfo, szEmblemName) == 0x5c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_fileName) == 0x11230);

void CGolfRuleGuildMatch::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pGolfRule->SendHoleData();

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
			rival.observerState = 1;
			rival.observerRank = 1;
			rival.uid = (*it).dwUserUID;
			rival.oid = (*it).dwGuid;
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

			Doc()->m_rivalList.push_back(rival);

			if ((*it).dwGuid == MyGuid(true))
			{
				sPlayerData* pPlayer = GOLFDOC()->GetPlayer(0);
				pPlayer->team = (*it).bTeam;
				m_pGolfRule->AddPlayer(0, (*it).sNick, (*it).dwUserUID,
					(*it).dwGuid, Doc()->m_userInfo[0].caddieInfo.tid);
			}

			Doc()->ClearUserInfoTimeMap((*it).dwUserUID);
		}

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

void CGolfRuleGuildMatch::SetGrade()
{
}

void CGolfRuleGuildMatch::SetReady()
{
	CGolfRuleBase::SetReady();
}

// JP 983 [12] (006a89e0): fora do primeiro buraco a entrada vai com -3 (3 s finais) + 0x155 e a camera 983 0xcc (tee-back)
void CGolfRuleGuildMatch::ReadyNextShot()
{
	if (GOLFDOC()->m_currentHole != Doc()->m_holeOrder[0])
	{
		MsgObject msg1(m_pGolfRule, 0x2716, (int)"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", -3, 1, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg1);
		MsgObject msg2(m_pGolfRule, 0x155, 1, 0, 0, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg2);
		CCameraWork::Instance()->SelectCamera(CCameraWork::kTeeBackCamera983); // 983 0xcc = its 204
		return;
	}
	// JP 983 006a89e0: no primeiro buraco NAO chama o da base: so a animacao de entrada e a camera 0xcb
	MsgObject msg(m_pGolfRule, 0x2716, (int)"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", 0, 0, 0, 0);
	m_pGolfRule->SendMsg983("Player", msg);
	CCameraWork::Instance()->SelectCamera(0xcb);
}

// JP 983 [13] (006a8ad0): o da base sem OnlinePlay() e sem SetNetMsgWait
void CGolfRuleGuildMatch::RequestShotReview(float time)
{
	if (GOLFDOC()->m_bTeeShotReady == 0)
	{
		GOLFDOC()->m_bTeeShotReady = 1;
		WSendPacket packet((enumClientPacket)0x34);
		packet.Send(TO_GAME);
		SendShotReview983();
		m_pGolfRule->CheckNetMsgWait_Send("DoPlayerPreview");
	}
}

// JP 983 [15] (006a89a0): so chama o 006a8130
void CGolfRuleGuildMatch::ShowScore(int arg)
{
	ShowScore983();
}
