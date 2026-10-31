#include "minatl.h"
#include "vc10_layout.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "camerawork.h"
#include "golfball.h"
#include "club.h"

CGolfRuleSkins::CGolfRuleSkins(CGolfRule* pGolfRule)
{
	m_pGolfRule = pGolfRule;
}

CGolfRuleSkins::~CGolfRuleSkins()
{
}

void CGolfRuleSkins::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		std::list<sSlotInfo>& slotList = Doc()->m_slotList;

		m_pGolfRule->SendHoleData();

		GOLFDOC()->MakePlayer(slotList.size());

		unsigned char index = 0;
		for (std::list<sSlotInfo>::iterator it = slotList.begin();
			it != slotList.end(); ++it, ++index)
		{
			if ((*it).dwGuid == -1)
			{
				GOLFDOC()->GetPlayer(index)->state = 3;
				continue;
			}

			Doc()->SetIndex((*it).dwGuid);

			GOLFDOC()->GetPlayer(index)->index = index;
			sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
			pPlayer->team = (*it).bTeam;
			m_pGolfRule->AddPlayer(index, (*it).sNick, (*it).dwUserUID,
				(*it).dwGuid, Doc()->m_userInfo[index].caddieInfo.tid);
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

void CGolfRuleSkins::SetGrade()
{
	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; ++i)
	{
		if (GOLFDOC()->GetPlayer(i)->state != 3)
		{
			unsigned char grade;
			if (Doc()->m_userInfo[i].stat.Level < 6)
				grade = 1;
			else
				grade = (Doc()->m_userInfo[i].stat.Level - 1) / 5 + 1;

			if (GOLFDOC()->m_maxGrade < grade)
				GOLFDOC()->m_maxGrade = grade;
		}
	}
}

void CGolfRuleSkins::SetReady()
{
	CGolfRuleBase::SetReady();
}

unsigned char MyIndex(); // 0053a260
unsigned char __fastcall GetHoleIndex(unsigned char hole); // 0046f4f0

C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHolePot) == 0x1b0 && offsetof(CGolfDoc, m_skinsPotBase) == 0x1b8);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsPotMult) == 0x1c0 && offsetof(CGolfDoc, m_skinsTotalPang) == 0x1c8);
// A435: CalcSkinsHolePang (983 0068a950) le/zera os mesmos campos: ninguem leva -> +0x1c4 ++ e Pang do proximo = min(2x+0x1c0, 8)
// x (base + 0x1b8), sem mexer no pote; alguem leva -> zera +0x1c4, +0x1b0 e +0x1d0, e o ReadyNextShot acima recomeca pela base.
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsCarryCount) == 0x1c4 && offsetof(CGolfDoc, m_skinsWinnerBonus983) == 0x1d0 && offsetof(CGolfDoc, m_skinsNextTurnWord983) == 0x1a8 && offsetof(CGolfDoc, m_bNoStat) == 0x194);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHoleWinner) == 0x1a0 && offsetof(CGolfDoc, m_skinsPenaltyUid) == 0x1a4);
C_ASSERT(offsetof(sPlayerData, oid) == 0x4b4 && offsetof(sPlayerData, skinsSumPang) == 0xd98 && offsetof(sPlayerData, skinsHolePang) == 0xda0);
C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, stat.i64Pang) == 0x3a2e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.holes) == 0x10699);
C_ASSERT(offsetof(sPlayerData, skinsPang) == 0x38c && sizeof(sHoleData) == 0x38 && offsetof(sHoleData, pang) == 0x20);

// 983 00672240 = CGolfRule::GetHolePang (golfrule.cpp; la so existe com REBANG_GOLFRULE_PCH): base do pote pela posicao do buraco
// B1341: 983 le o tipo do canal em Doc+0x10a99 (2.5 = 00d08b8c, 5.0 = 00cfea7c, 20.0 = 00d05d90, 0.5/1.0 = 00d66ca0/00d66cb8)
C_ASSERT_VC10(offsetof(CSharedDoc, m_curChannel) + offsetof(sChannelInfo, Type) == 0x10a99);
static unsigned long HolePang983(unsigned char hole)
{
	unsigned long type = Doc()->m_curChannel.Type;
	float rate;
	if (type & 0x800)
		rate = 1.0f;
	else if (type & 0x1000)
		rate = 2.5f;
	else if (type & 0x2000)
		rate = 5.0f;
	else
		rate = 1.0f;
	float pang = rate * 20.0f * ((hole - 1) / 3 * 0.5f + 1.0f);
	if (hole == Doc()->m_golfGame.holes)
		pang += pang;
	return (unsigned long)pang;
}

// JP 983 [7] (006a9da0): a da base e o pang/base de cada buraco pela ordem (no 645 era o laco do GetHolePang do
// CGolfRule::OnPreLoadInit, que o 983 tirou de la; sem isto o pote do Skins comecava em 0)
void CGolfRuleSkins::V7_983()
{
	CGolfRuleBase::V7_983();
	for (int i = 0; i < Doc()->m_golfGame.holes; ++i)
	{
		unsigned long pang = HolePang983((unsigned char)(i + 1));
		GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).basePang = pang;
		GOLFDOC()->GetHoleData(Doc()->m_holeOrder[i]).pang = pang;
	}
}

// JP 983 [11] (006aa070): o aviso do turno do Skins (no 645, o "mode == 7" do CGolfRule::SetTurnChangeEvent). Bola no green:
// estado do buraco (+0x1a8) 1 = "matchshot3", 2 = "winningshot3" e o pote (+0x1b0) ao "Letter" (0x202); fora disso nada (o 645
// mandava a "chance"). Fora do green, com +0x194: Approach 30/60 pelo taco (taco > 3 manda o nome nulo, como o 983).
// Devolve *pbCaddie (byte em +0x6a03 do jogador da vez, so no estado 2): true = o SetTurnChangeEvent segue com o Letter do turno.
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentPlayer) == 0x189);
C_ASSERT_VC10(offsetof(CGolfDoc, m_skinsHolePot) == 0x1b0 && offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, caddieInfo.guid) == 0x6a03 && sizeof(sUserInfo) == 0x31c6); // A749: ex-crus GOLFDOC()+0x1a8/+0x1b0, Doc()+player*0x31c6+0x6a03
bool CGolfRuleSkins::V11_983(bool* pbCaddie)
{
	float duration = 2.0f;
	if (GolfBall().m_groundType == 2)
	{
		if ((short)GOLFDOC()->m_skinsNextTurnWord983 == 2 && Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.guid != 0)
			*pbCaddie = true;
		if (!IsMyTurn(true))
			return *pbCaddie;
		MsgObject msg(NULL, 0x1fe /*983: remetente 0*/, 0, 0, 0, 0, 0);
		msg.param2 = (int)&duration;
		short state = (short)GOLFDOC()->m_skinsNextTurnWord983;
		if (state == 1)
		{
			msg.param1 = (int)"matchshot3";
			m_pGolfRule->SendMsg983("Letter", msg);
		}
		else if (state == 2)
		{
			msg.param1 = (int)"winningshot3";
			m_pGolfRule->SendMsg983("Letter", msg);
			int params[4]; // [0] o 983 nao preenche
			params[1] = (int)GOLFDOC()->m_skinsHolePot;
			*(float*)&params[2] = 0.8f;
			params[3] = 0xffffde44;
			MsgObject pot(NULL, 0x202 /*983: remetente 0*/, 0, 0, 0, 0, 0);
			pot.param1 = (int)"winningshot3";
			pot.param2 = (int)params;
			m_pGolfRule->SendMsg983("Letter", pot);
		}
	}
	else if (IsMyTurn(true) && GOLFDOC()->m_bNoStat)
	{
		MsgObject msg(NULL, 0x1fe /*983: remetente 0*/, 0, 0, 0, 0, 0);
		if (*((unsigned char*)&GolfClub() + 0x3c) < 3)
			msg.param1 = (int)"\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1" "30_\xbf\xac\xc0\xe5";
		else if (*((unsigned char*)&GolfClub() + 0x3c) < 4)
			msg.param1 = (int)"\xbe\xee\xc7\xc1\xb7\xce\xc4\xa1" "60_\xbf\xac\xc0\xe5";
		msg.param2 = (int)&duration;
		m_pGolfRule->SendMsg983("Letter", msg);
	}
	return *pbCaddie;
}

// JP 983 [12] (006aa290): com CGolfDoc+0x194 = entrada/camera da base e o pote do buraco; senao 0x155 e a camera 983 0xcc
// (tee-back). Pote: +0x1b0 = pote do buraco, +0x1b8 = soma das bases acumuladas, +0x1c0 = multiplicador (1, 2, 4, 8),
// +0x1c8 = pote total do jogo (buracos jogados que ninguem levou ficam fora; os que faltam entram pela base).
void CGolfRuleSkins::ReadyNextShot()
{
	if (GOLFDOC()->m_bNoStat)
	{
		MsgObject msg(m_pGolfRule, 0x155, 1, 0, 0, 0, 0);
		m_pGolfRule->SendMsg983("Player", msg);
		CCameraWork::Instance()->SelectCamera(CCameraWork::kTeeBackCamera983); // 983 0xcc = its 204
		return;
	}
	CGolfRuleBase::ReadyNextShot();

	CGolfDoc* pDoc = GOLFDOC();
	if (pDoc->m_skinsHolePot == 0)
	{
		pDoc->m_skinsPotBase = HolePang983(GetHoleIndex(pDoc->m_currentHole));
		pDoc->m_skinsHolePot = HolePang983(GetHoleIndex(pDoc->m_currentHole));
		pDoc->m_skinsPotMult = 1;
	}
	else
	{
		pDoc->m_skinsPotBase += HolePang983(GetHoleIndex(pDoc->m_currentHole));
		unsigned long mult = pDoc->m_skinsPotMult * 2;
		if (mult >= 8)
			mult = 8;
		pDoc->m_skinsPotMult = mult;
		pDoc->m_skinsHolePot = (unsigned __int64)pDoc->m_skinsPotMult * pDoc->m_skinsPotBase;
		pDoc->m_pHoleData[GetHoleIndex(pDoc->m_currentHole) - 1].pang = pDoc->m_skinsHolePot;
	}

	pDoc->m_skinsTotalPang = 0;
	for (int i = 0; i < Doc()->m_golfGame.holes; i++)
	{
		unsigned char hole = Doc()->m_holeOrder[i];
		sHoleData& data = pDoc->m_pHoleData[hole - 1];
		if (i < GetHoleIndex(pDoc->m_currentHole) && data.pang != 0)
		{
			if (pDoc->GetPlayer(MyIndex())->skinsPang[hole - 1] != -1)
				pDoc->m_skinsTotalPang += data.pang;
		}
		else
			pDoc->m_skinsTotalPang += data.basePang;
	}
}
