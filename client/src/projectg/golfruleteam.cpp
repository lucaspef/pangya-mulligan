#include "minatl.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "cardsystem.h"
#include "club.h"
#include "golfball.h"
#include "../shared/localize.h"

CGolfRuleTeam::CGolfRuleTeam(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;
}

CGolfRuleTeam::~CGolfRuleTeam()
{
}

void CGolfRuleTeam::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pGolfRule->SendHoleData();

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;

		GOLFDOC()->MakeTeam();
		GOLFDOC()->ResetTeamVars();
		GOLFDOC()->MakePlayer(slotList.size());

		unsigned char index = 0;
		for (std::list<sSlotInfo>::iterator it = slotList.begin();
			it != slotList.end(); ++it, ++index)
		{
			if (it->dwGuid == -1)
			{
				GOLFDOC()->GetPlayer(index)->state = 3;
				continue;
			}

			Doc()->SetIndex(it->dwGuid);

			GOLFDOC()->GetPlayer(index)->index = index;
			sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
			pPlayer->team = it->bTeam;
			m_pGolfRule->AddPlayer(index, it->sNick, it->dwUserUID, it->dwGuid,
				Doc()->m_userInfo[index].caddieInfo.tid);
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

void CGolfRuleTeam::SetGrade()
{
	unsigned char i;

	for (i = 0; i < 2; ++i)
	{
		unsigned char sum = 0;

		for (i = 0; i < GOLFDOC()->m_playerNum; ++i)
		{
			if (PLAYER(i)->team == i)
			{
				if (Doc()->m_userInfo[i].stat.Level < 6)
					++sum;
				else
					sum += (Doc()->m_userInfo[i].stat.Level - 1) / 5 + 1;
			}
		}

		unsigned char grade = sum / 2;
		if (GOLFDOC()->m_maxGrade < grade)
			GOLFDOC()->m_maxGrade = grade;
	}
}

void CGolfRuleTeam::SetReady()
{
	CGolfRuleBase::SetReady();
}

void CGolfRuleTeam::AddPlayer(unsigned char index, const char* name,
	unsigned long uid, unsigned long oid, unsigned long caddie)
{
	if (index >= GOLFDOC()->m_playerNum)
		return;

	float gauge = 0.0f;

	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
	pPlayer->teammate = 0xff;

	for (int i = 0; i < index; ++i)
	{
		if (GOLFDOC()->GetPlayer(i)->team == pPlayer->team)
		{
			GOLFDOC()->GetPlayer(i)->teammate = index;
			pPlayer->teammate = i;
			break;
		}
	}

	pPlayer->color = TEAM(pPlayer->team)->color;

	pPlayer->uid = uid;
	pPlayer->oid = oid;
	pPlayer->state = 2;
	pPlayer->chatBlock = 0;
	pPlayer->itemNum = 0;
	pPlayer->item = 0;
	pPlayer->windAbility983 = 0; // 983 (006a7470/006ab150) zera +0x4c0 (o 645 nao tem)
	pPlayer->totalStroke = 0;
	pPlayer->shotTime = 0;
	pPlayer->rank = 1;

	if (Doc()->m_userInfo[index].caddieInfo.tidPart)
	{
		IFF_STRUCT::sCadItem* pCadItem = ItemManager()->FindCadItem(
			Doc()->m_userInfo[index].caddieInfo.tidPart);

		if (pCadItem)
		{
			gauge = pCadItem->COM[4];
		}
	}

	gauge += Doc()->GetWtPepPangyaComboGauge(index);

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(index);
		gauge += CCardManager::Instance()->GetCardPeriodComboGauge();
	}

	if (TEAM(pPlayer->team)->gauge < gauge)
	{
		// 983 (006ab150): limite pelo indice do jogador (FUN_0062fd10), o maior entre ele e o parceiro (no 645 era pelo time)
		float limit = Doc()->GetComboGaugeLimit(index);
		if (pPlayer->teammate != 0xff)
			limit = Max<float>(limit, Doc()->GetComboGaugeLimit(pPlayer->teammate));
		TEAM(pPlayer->team)->gauge =
			gauge < 0.0f ? 0.0f : (gauge > limit ? limit : gauge);
	}

	pPlayer->gauge = 0;

	if (Doc()->m_curChannel.Type & 8)
		pPlayer->gauge = 0;

	pPlayer->score = 0;
	pPlayer->totalPang = 0;
	pPlayer->bonusPang = 0;

	pPlayer->skinsHolePang = 0;
	pPlayer->holeNum = 0;

	pPlayer->skinsTotalPang = 0;
	pPlayer->skinsSumPang = 0;

	pPlayer->extPrizeNum = 0;
	memset(&pPlayer->prize, 0, sizeof(pPlayer->prize));
	pPlayer->level = Doc()->m_userInfo[index].stat.Level;

	for (int j = 0; j < 18; ++j)
	{
		pPlayer->stroke[j] = 0;
		pPlayer->putt[j] = 0;
		pPlayer->pang[j] = 0;
		pPlayer->skinsPang[j] = 0;
	}

	if (Doc()->m_curChannel.Type & 8)
	{
		for (int k = 0; k < 10; ++k)
		{
			Doc()->m_userInfo[index].userEquip.tidItemSlot[k] = 0;
		}
	}

	sCharacterInfo* pCharInfo = &Doc()->m_userInfo[index].charInfo;
	CPartTidList& tidList = GOLFDOC()->GetPartTidList(index);
	tidList.SetTids(pCharInfo, 0xff);

	pPlayer->itemNum = Doc()->CollapseItemSlot(index);

	GOLFDOC()->SetPlayerLevel(index);

	SetMyTicket983(pPlayer);
}

int CGolfRuleTeam::GetNumTimeout()
{
	int num = PLAYER(GOLFDOC()->m_currentPlayer)->timeout;

	if (PLAYER(GOLFDOC()->m_currentPlayer)->teammate != 0xff)
		num += PLAYER(PLAYER(GOLFDOC()->m_currentPlayer)->teammate)->timeout;

	return num;
}

bool CGolfRuleTeam::CheckGiveUp()
{
	if (PLAYER(GOLFDOC()->m_currentPlayer)->state == 3)
		return false;

	unsigned char limit = GOLFDOC()->GetHoleData().par + 4;
	int state = TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)->state;
	unsigned char stroke = TEAM(PLAYER(GOLFDOC()->m_currentPlayer)->team)
							   ->stroke[GOLFDOC()->m_currentHole - 1];

	if (state == 1)
		return stroke > limit;

	return stroke >= limit;
}

// JP 983 [12] (006aab70): entrada/camera da base e, antes do ChangeTurn, zera os 2 times e marca no time de quem joga
// stroke 1 do buraco, quem bate (+0x3ad) e o tipo do taco (+0x3ae); no 645 isso ficava no ChangeGameMode, depois do ChangeTurn
void CGolfRuleTeam::ReadyNextShot()
{
	CGolfRuleBase::ReadyNextShot();
	for (int t = 0; t < 2; t++)
		*(unsigned long*)TEAM((unsigned char)t) = 0;
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	TEAM(PLAYER(cur)->team)->stroke[GOLFDOC()->m_currentHole - 1] = 1;
	*((unsigned char*)TEAM(PLAYER(cur)->team) + 0x3ad) = cur;
	unsigned char club = *(unsigned char*)(*(int*)((char*)&GolfClub() + 4) + 0x22);
	*((unsigned char*)TEAM(PLAYER(cur)->team) + 0x3ae) = club;
}

// JP 983 [11] (006aacb0): o aviso do turno do Team (no 645, o "mode == 1" do CGolfRule::SetTurnChangeEvent). So com a bola no
// green; fora dele devolve true (segue o Letter do turno). Time da vez a frente no buraco (ou empatado se o outro ja acabou) =
// "winningshot"; diferenca do total maior que os buracos que faltam = "matchshot". Mesmas contas do 645; o 983 manda 0x1fe
// (remetente 0) e devolve false quando chega a avaliar (o SetTurnChangeEvent devolve *pbCaddie sem o Letter do turno).
#define TEAM_HOLE_STROKE983(t, h) \
	((unsigned char)~(((unsigned char*)(t))[(h) * 0x10 - 0xc + 0xe] ^ ((unsigned char*)(t))[(h) * 0x10 - 0xc]))
// 983 006aab70/006aacb0: stroke do time em +4 + (h-1)*0x10 (dado +0, chave +0xe), score +0x368, quem bate +0x3ad, taco +0x3ae
C_ASSERT(offsetof(sTeamData, stroke) == 4 && sizeof(WCrypticValue<unsigned char>) == 0x10);
C_ASSERT(offsetof(sTeamData, score) == 0x368 && offsetof(sTeamData, shooter) == 0x3ad && offsetof(sTeamData, club) == 0x3ae);
bool CGolfRuleTeam::V11_983(bool* pbCaddie)
{
	float duration = 2.0f;
	if (GolfBall().m_groundType != 2)
		return true;
	unsigned char myTeam = *((unsigned char*)PLAYER(GOLFDOC()->m_currentPlayer) + 0x4e9);
	unsigned char other = 1 - myTeam;
	bool winShot = false;
	bool matchShot = false;
	int diff = *(int*)((char*)TEAM(myTeam) + 0x368) - *(int*)((char*)TEAM(other) + 0x368);
	unsigned char otherStroke = TEAM_HOLE_STROKE983(TEAM(other), GOLFDOC()->m_currentHole);
	unsigned char myStroke = TEAM_HOLE_STROKE983(TEAM(myTeam), GOLFDOC()->m_currentHole);
	bool ahead = TEAM(other)->state == 1 ? myStroke < otherStroke : myStroke <= otherStroke;
	int left = Doc()->m_golfGame.holes - GetHoleIndex(GOLFDOC()->m_currentHole);
	bool check = true;
	if (ahead)
	{
		winShot = true;
		if (left < diff + 1)
			matchShot = true;
	}
	else if (left < diff)
		matchShot = true;
	else
		check = false;
	if (check && Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.guid != 0)
		*pbCaddie = true;
	if (!IsMyTurn(true))
		return true;
	MsgObject msg(NULL, 0x1fe /*983: remetente 0*/, 0, 0, 0, 0, 0);
	if (matchShot)
		msg.param1 = (int)"matchshot";
	else if (winShot)
		msg.param1 = (int)"winningshot";
	else
		return true;
	msg.param2 = (int)&duration;
	m_pGolfRule->SendMsg983("Letter", msg);
	return false;
}
