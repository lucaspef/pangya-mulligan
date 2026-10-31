#include "minatl.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "golfball.h"
#include "quadtree.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "../shared/localize.h"
#include "contentsdoc.h"
#include "packets/tutorialcontainer.h"

int __fastcall CalcPowerPenalty(int upgrade);

CGolfRuleTutorial::CGolfRuleTutorial(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;
}

CGolfRuleTutorial::~CGolfRuleTutorial()
{
}

void CGolfRuleTutorial::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pGolfRule->SendHoleData();

		GOLFDOC()->MakePlayer(1);
		Doc()->SetIndex(Doc()->m_myInfo.info.dwGuid);
		SetEquipCharCaddieInfoForTutorial();

		switch (Doc()->m_golfGame.gameType)
		{
		case GAME_TYPE_TUTORIAL_BASIC:
			m_pGolfRule->AddPlayer(0, Doc()->m_myInfo.info.sNick,
				Doc()->m_myInfo.info.dwUID, Doc()->m_myInfo.info.dwGuid,
				0x1c000001);
			GOLFDOC()->m_tutorialMode = 2;
			break;

		case GAME_TYPE_TUTORIAL_ADV:
			m_pGolfRule->AddPlayer(0, Doc()->m_myInfo.info.sNick,
				Doc()->m_myInfo.info.dwUID, Doc()->m_myInfo.info.dwGuid,
				0x1c000001);
			{
				// 983 (006ac320): modo do documento 0xa4 (CTutorialContainer), nao do ator "TutorialMain" (M983(0x14f))
				int mode = 1;
				CTutorialContainer983* pDoc = (CTutorialContainer983*)
					CContentsDoc::Instance()->GetContainer((localContentType_t)0xa4);
				if (pDoc)
					mode = pDoc->m_mode;
				GOLFDOC()->m_tutorialMode = mode;
			}
			break;
		}

		GOLFDOC()->RefreshPlayerNum();
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
}

// 983 006aba80: copia do CGolfRuleBase::AddPlayer sem caddie/carta/canal e com os atributos fixos do tutorial
// (poder 15, controle 17, precisao 10, spin 5, curva 5) + o limite do SetPlayerLevel inline.
void CGolfRuleTutorial::AddPlayer(unsigned char index, const char* name,
	unsigned long uid, unsigned long oid, unsigned long caddie)
{
	if (index >= GOLFDOC()->m_playerNum)
		return;

	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
	pPlayer->teeOrder = 0;
	pPlayer->order = 1;

	for (int i = 0; i < index; ++i)
	{
		if (GOLFDOC()->GetPlayer(i)->state != 3)
		{
			++pPlayer->teeOrder;
			++pPlayer->order;
		}
	}

	pPlayer->color = m_playerColor[index];

	pPlayer->uid = uid;
	pPlayer->oid = oid;
	pPlayer->state = 2;
	pPlayer->chatBlock = 0;
	pPlayer->itemNum = 0;
	pPlayer->item = 0;
	pPlayer->totalStroke = 0;
	pPlayer->shotTime = 0;
	pPlayer->rank = 1;

	pPlayer->score = 0;
	pPlayer->totalPang = 0;
	pPlayer->bonusPang = 0;

	pPlayer->skinsHolePang = 0;
	pPlayer->skinsTotalPang = 0;
	pPlayer->skinsSumPang = 0;

	pPlayer->holeNum = 0;

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

	sCharacterInfo* pCharInfo = &Doc()->m_userInfo[index].charInfo;
	CPartTidList& tidList = GOLFDOC()->GetPartTidList(index);
	tidList.SetTids(pCharInfo, 0xff);

	pPlayer->itemNum = Doc()->CollapseItemSlot(index);

	pPlayer->power = 15;
	pPlayer->control = 17;
	pPlayer->accuracy = 10;
	pPlayer->spin = 5;
	pPlayer->curve = 5;

	int power = pPlayer->power;
	if (power < 0)
		power = 0;
	else if (power > 100)
		power = 100;
	pPlayer->power = power - 15;

	pPlayer->control -= Max(0, pPlayer->power - (CalcPowerPenalty(pPlayer->level) + 5));
	pPlayer->accuracy -= Max(0, pPlayer->power - (CalcPowerPenalty(pPlayer->level) + 5));

	int accuracy = pPlayer->accuracy;
	if (accuracy < 0)
		accuracy = 0;
	else if (accuracy > 30)
		accuracy = 30;
	pPlayer->accuracy = accuracy;

	int control = pPlayer->control;
	if (control < 0)
		control = 0;
	else if (control > 30)
		control = 30;
	pPlayer->control = control;

	int spin = pPlayer->spin;
	if (spin < 1)
		spin = 1;
	else if (spin > 30)
		spin = 30;
	pPlayer->spin = spin;

	int curve = pPlayer->curve;
	if (curve < 1)
		curve = 1;
	else if (curve > 30)
		curve = 30;
	pPlayer->curve = curve;
}

void CGolfRuleTutorial::SetGrade()
{
}

void CGolfRuleTutorial::SetReady()
{
	GetPVS().ResetHoleData();
	if (GOLFDOC()->m_tutorialMode != 9 && GOLFDOC()->m_tutorialMode < 15)
		GolfBall().Reset(GolfBall().m_pos);

	GolfBall().m_meshIndex = GetPVS().GetMeshIndex(GolfBall().m_pos, false);
	m_pGolfRule->ResetData(GolfBall().m_pos);

	GOLFDOC()->m_currentPlayer = m_pGolfRule->GetFirstTurn();
	PLAYER(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] =
		1;
	PLAYER(GOLFDOC()->m_currentPlayer)->state = 0;
	m_pGolfRule->SendMsg(m_pGolfRule, "Logo", M983(0x18b), 0, 0, 0, 0);
}

void CGolfRuleTutorial::SetEquipCharCaddieInfoForTutorial()
{
	if (!IsLocalContent(S3_TUTORIAL_RENEWAL))
		return;

	memset(&Doc()->m_userInfo[0].charInfo, 0, sizeof(sCharacterInfo));
	memset(&Doc()->m_userInfo[0].caddieInfo, 0, sizeof(sCaddieInfo));
	memset(&Doc()->m_userInfo[0].clubInfo, 0, sizeof(sClubInfo));
	memset(&Doc()->m_userInfo[0].mascotInfo, 0, sizeof(sMascotInfo));
	memset(&Doc()->m_userInfo[0].userEquip, 0, sizeof(sUserEquip));

	Doc()->m_userInfo[0].info = Doc()->m_myInfo.info;
	Doc()->m_userInfo[0].stat = Doc()->m_myInfo.stat;
	Doc()->m_userInfo[0].userEquip.tidSkin[5] =
		Doc()->m_myInfo.userEquip.tidSkin[5];

	sCharacterInfo* pCharInfo = &Doc()->m_userInfo[0].charInfo;
	// 983 006ac1d5: `gender` e o bit 3 do byte logo apos dwEmblemVer (nossa sPangYaUserInfo o tem no bit 1)
	int gender = (((unsigned char*)&Doc()->m_userInfo[0].info.dwEmblemVer)[4] >> 3) & 1;
	if (gender % 2)
	{
		pCharInfo->tid = 0x4000001;
	}
	else
	{
		pCharInfo->tid = 0x4000000;
	}

	Doc()->m_itemManager.GetDefCombo(pCharInfo->tid, pCharInfo->tidParts);

	Doc()->m_userInfo[0].clubInfo.tid = 0x10000000;

	{
		// 983 006ac232: procura no m_clubSetMap o conjunto cujo id e o tid do taco e guarda a chave
		std::map<unsigned int, sItemInfo>& clubSet = Doc()->m_clubSetMap;
		unsigned long tid = Doc()->m_userInfo[0].clubInfo.tid;
		for (CLUBSET_MAP_IT it = clubSet.begin(); it != clubSet.end(); ++it)
		{
			if ((*it).second.guid == tid)
			{
				Doc()->m_userInfo[0].clubInfo.guid = (*it).first;
				break;
			}
		}
	}

	Doc()->m_userInfo[0].userEquip.tidBall = 0x14000000;

	Doc()->m_userInfo[0].caddieInfo.Level = 2;
}
