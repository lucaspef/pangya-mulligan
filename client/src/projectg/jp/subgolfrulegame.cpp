// JP 983: unidade 00650540–006506e0 (CSubGolfRuleGame). Classe nova do 983; o `Init` é o laço de carga dos buracos que no 645
// estava no CGolfRule (golfrule.cpp da frente A, "LoadMapCheckData"). Deslocamentos medidos no exe 983.
#include "subgolfrule.h"
#include "golfdoc.h"
#include "polysoup.h"
#include "networksystem.h"
#include "lobbymain.h"
#include "vc10_layout.h"

C_ASSERT_VC10(offsetof(CSharedDoc, m_pinIndex) == 0x1050c && offsetof(CSharedDoc, m_holeOrder) == 0x1051e && offsetof(CSharedDoc, m_shotDataList) == 0x110bc);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame) + offsetof(sGolfGame, holes) == 0x10699 && offsetof(CSharedDoc, m_roomInfo) + offsetof(sRoomInfo, realGameType) == 0x109b8);
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8 && sizeof(sHoleData) == 0x38 && offsetof(sHoleData, playerNum) == 0x19 && offsetof(sHoleData, result) == 0x1e);

CSubGolfRuleGame::CSubGolfRuleGame(CGolfRule* pRule)
{
	m_pRule = pRule;
}

CSubGolfRuleGame::~CSubGolfRuleGame()
{
}

void CSubGolfRuleGame::Init()
{
	Doc()->m_shotDataList.clear();
	for (unsigned char i = 0; i < Doc()->m_golfGame.holes; i++)
	{
		GOLFDOC()->m_pHoleData[Doc()->m_holeOrder[i] - 1].playerNum = 0;
		GOLFDOC()->m_pHoleData[Doc()->m_holeOrder[i] - 1].result = 3;
		if (Doc()->m_roomInfo.realGameType == 0x12 || Doc()->m_roomInfo.realGameType == 0x14)
			SetCurMap(Doc()->m_pinIndex[i]);
		if (!GOLFDOC()->m_pPolySoup->LoadMapCheckData(Doc()->m_holeOrder[i]) &&
			WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME) != 0)
		{
			g_bQuit = true;
			return;
		}
		GetCurMap();
	}
	switch (Doc()->m_roomInfo.realGameType)
	{
	case 0x14:
	case 0x15:
		SetCurMap(Doc()->m_pinIndex[0]);
		break;
	}
}
