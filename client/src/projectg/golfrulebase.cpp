#include "minatl.h"
#include "vc10_layout.h"
#include "golfdoc.h"
#include "golfrule.h"
#include "golfrulebase.h"
#include "clientsetting.h"
#include "polysoup.h"
#include "cardsystem.h"
#include "gatewayactor.h"
#include "camerawork.h"
#include "golfball.h"
#include "logininfo.h"
#include "club.h"
#include "quadtree.h"
#include "../shared/localize.h"

CGolfRuleBase::CGolfRuleBase()
{
	m_pGolfRule = NULL;

	m_playerColor[0] = 0xfd0002;
	m_playerColor[1] = 0xdc00fd;
	m_playerColor[2] = 0x27d628;
	m_playerColor[3] = 0x2778d6;
}

CGolfRuleBase::~CGolfRuleBase()
{
}

C_ASSERT_VC10(offsetof(CGolfDoc, m_bTeeShotReady) == 0x15e);
// JP 983: layout medido em 006a69d0 / 006a7470 / 00663800 / 0065fc80 (sessao 74)
C_ASSERT(sizeof(sPlayerData) == 0xdb0);
C_ASSERT(offsetof(sPlayerData, stroke) == 0x2c);
C_ASSERT(offsetof(sPlayerData, gauge) == 0x4c8);
C_ASSERT(offsetof(sPlayerData, event) == 0x4e4);
C_ASSERT(offsetof(sPlayerData, team) == 0x4e9);
C_ASSERT(offsetof(sPlayerData, shotTime) == 0x4ec);
C_ASSERT(offsetof(sPlayerData, totalStroke) == 0x4f1);
C_ASSERT(offsetof(sPlayerData, extPrizeNum) == 0x589);
C_ASSERT(offsetof(sPlayerData, skinsTotalPang) == 0xd90);
C_ASSERT(offsetof(sPlayerData, level) == 0xda8);
C_ASSERT(sizeof(sTeamData) == 0x3b4);
C_ASSERT(sizeof(CGolfBall::sExpected) == 0x78);
C_ASSERT(sizeof(CGolfBall) == 0x2c004);
C_ASSERT(offsetof(CGolfBall, m_holeIn) == 0x2bff4);
C_ASSERT(offsetof(CGolfBall, m_standGround) == 0xb0);
C_ASSERT(sizeof(sPangYaUserStatistics) == 0x109);
C_ASSERT(offsetof(sPangYaUserStatistics, fLongest) == 0x10);
C_ASSERT(offsetof(sPangYaUserStatistics, dwOB) == 0x1c);
C_ASSERT(offsetof(sPangYaUserStatistics, dwDistance) == 0x20);
C_ASSERT(offsetof(sPangYaUserStatistics, wHoleInOne) == 0x2c);
C_ASSERT(offsetof(sPangYaUserStatistics, wBunker) == 0x30);
C_ASSERT(offsetof(sPangYaUserStatistics, dwFairway) == 0x32);
C_ASSERT(offsetof(sPangYaUserStatistics, wAlbatross) == 0x36);
C_ASSERT(offsetof(sPangYaUserStatistics, dwHoleIn) == 0x3a);
C_ASSERT(offsetof(sPangYaUserStatistics, dwPuttIn) == 0x3e);
C_ASSERT(offsetof(sPangYaUserStatistics, fLongestPuttIn) == 0x42);
C_ASSERT(offsetof(sPangYaUserStatistics, fLongestChipIn) == 0x46);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myItemList) == 0x375c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bGameOver) == 0x10488);
C_ASSERT_VC10(offsetof(CSharedDoc, m_pangRate) == 0x10580);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bFastForward) == 0x10588);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo) == 0x108f4);
C_ASSERT_VC10(offsetof(CSharedDoc, m_mapEvent983) == 0x1146c);
C_ASSERT(offsetof(sUserInfo, caddieInfo.tid) == 0x3157);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bNetGameEnd) == 0x15c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bNoStat) == 0x194);
C_ASSERT_VC10(offsetof(CGolfDoc, m_bApproach794) == 0x7b4);
C_ASSERT_VC10(offsetof(CGolfDoc, m_pHoleData) == 0xd8);
C_ASSERT(offsetof(sHoleData, par) == 0x18 && sizeof(sHoleData) == 0x38);

static inline bool HasMyItem983(unsigned long tid)
{
	return std::find(Doc()->m_myItemList.begin(), Doc()->m_myItemList.end(), tid) != Doc()->m_myItemList.end();
}

// JP 983 [7] (006a7a40): taxa de pang da partida. No 645 este trecho estava dentro do CGolfRule (carga da partida);
// novos no 983: 2a taxa do cartao (0080a3f0), itens 0x1a0003b7 (x4), 0x1a000007/8 (x1,2), 0x1a0000ae (x2) e a
// taxa do mapa lida de um registro (CSharedDoc+0x1146c, 0063f830) sem o teste de conteudo local.
void CGolfRuleBase::V7_983()
{
	Doc()->m_pangRate = 1.0f;
	if (!((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (!HasMyItem983(0x1a000001) && !HasMyItem983(0x1a000002))
		{
			if (IsPcBang())
				Doc()->m_pangRate = Doc()->m_pangRate * 1.2f;
		}
		else
		{
			float rate = Doc()->m_pangRate;
			if (!IsPcBang())
				rate = rate + rate;
			else
				rate = rate * 2.5f;
			Doc()->m_pangRate = rate;
		}
		if (IsLocalContent((localContentType_t)0x52))
		{
			CCardManager::Instance()->SetPlayerIndex(0xff);
			Doc()->m_pangRate = (CCardManager::Instance()->GetCardPeriodPangRate() / 100.0 + 1.0) * Doc()->m_pangRate;
			Doc()->m_pangRate = (CCardManager::Instance()->GetCardPeriodPangRate2_983() / 100.0 + 1.0) * Doc()->m_pangRate;
		}
		if (HasMyItem983(0x1a000005))
			Doc()->m_pangRate = Doc()->m_pangRate * 4.0f;
		if (HasMyItem983(0x1a0003b7))
			Doc()->m_pangRate = 4.0f * Doc()->m_pangRate;
		if (HasMyItem983(0x1a000009))
			Doc()->m_pangRate = Doc()->m_pangRate * 1.2f;
		if (HasMyItem983(0x1a00000c))
			Doc()->m_pangRate = Doc()->m_pangRate * 1.2f;
		if (HasMyItem983(0x1a000007))
			Doc()->m_pangRate = Doc()->m_pangRate * 1.2f;
		if (HasMyItem983(0x1a000008))
			Doc()->m_pangRate = 1.2f * Doc()->m_pangRate;
		if (HasMyItem983(0x1a0000ae))
			Doc()->m_pangRate = Doc()->m_pangRate + Doc()->m_pangRate;
		if (IsLocalContent((localContentType_t)3))
		{
			sMascotInfo* mascotInfo = Doc()->GetMyEquippedMascotInfo();
			if (mascotInfo != NULL)
			{
				IFF_STRUCT::sMascot* mascot = Doc()->m_itemManager.FindMascot(mascotInfo->tid);
				if (mascot != NULL && mascot->PangUp > 100)
					Doc()->m_pangRate = (float)mascot->PangUp / 100.0 * Doc()->m_pangRate;
			}
		}
		// 0063f830: procura o mapa no registro de eventos; +0xc = taxa em %
		std::map<int, int>::iterator it = Doc()->m_mapEvent983.find(GetCurMap());
		const unsigned char* pEvent = it != Doc()->m_mapEvent983.end() ? (const unsigned char*)it->second : NULL;
		if (pEvent != NULL)
			Doc()->m_pangRate = (float)*(const unsigned short*)(pEvent + 0xc) / 100.0 * Doc()->m_pangRate;
	}
	Doc()->m_bFastForward = false;
	if (!((Doc()->m_myInfo.info.dwIdentity >> 1) & 1))
	{
		if (HasMyItem983(0x1a000011))
			Doc()->m_bFastForward = true;
	}
}

// JP 983 [9] (006a69d0): estatisticas do jogador da vez quando a jogada termina (bola no buraco, OB, bunker, fairway, distancia)
void CGolfRuleBase::OnHoleIn()
{
	CGolfBall& ball = GolfBall();
	const CGolfBall::sExpected* pFirst = ball.PreData(0);
	unsigned char cur = GOLFDOC()->m_currentPlayer;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(cur);
	float dx = pPlayer->pos.x - pFirst->pos.x;
	float dz = pPlayer->pos.z - pFirst->pos.z;
	float dist;
	if (GOLFDOC()->IsCurrentPlayerEvent983(0xe) == true)
		dist = 0.0f;
	else
		dist = (float)sqrt(dx * dx + dz * dz) * 0.3125;

	if (ball.IsHoleIn() || ball.m_standGround > 0)
	{
		sPangYaUserStatistics& stat = Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer];
		if (stat.fLongest <= dist)
			stat.fLongest = dist;
	}
	if (ball.m_bOB)
	{
		++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].dwOB;
		GOLFDOC()->m_bApproach794 = true;
		return;
	}
	if (ball.IsHoleIn())
	{
		if (GolfClub().GetType() == 3)
		{
			++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].dwPuttIn;
			sPangYaUserStatistics& stat = Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer];
			if (stat.fLongestPuttIn <= dist)
				stat.fLongestPuttIn = dist;
		}
		else
		{
			sPangYaUserStatistics& stat = Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer];
			if (stat.fLongestChipIn <= dist)
				stat.fLongestChipIn = dist;
		}
		++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].dwHoleIn;

		bool bHoleInOne;
		if (Doc()->m_golfGame.gameType == 1)
		{
			sPlayerData* p = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
			bHoleInOne = GOLFDOC()->GetTeam(p->team)->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1;
		}
		else
			bHoleInOne = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1;
		if (bHoleInOne)
			++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].wHoleInOne;
		else if (Doc()->m_golfGame.gameType != 1)
		{
			sPlayerData* p = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
			if ((unsigned char)p->stroke[GOLFDOC()->m_currentHole - 1] - GOLFDOC()->GetHoleData().par == -3)
				++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].wAlbatross;
		}
	}
	unsigned char ground = GetPVS().GetGroundType(GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->pos, true, NULL);
	if (ground == 3)
		++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].wBunker;
	else if (ground == 1 || ground == 2)
	{
		if (GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer)->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1)
			++Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].dwFairway;
	}
	Doc()->m_holeStatistics[GOLFDOC()->m_currentPlayer].dwDistance +=
		(int)floor((float)sqrt(dx * dx + 0.0f * 0.0f + dz * dz) * 0.3125 + 0.5); // 983 6a6ee5: fistp (arredonda), nao trunca
	GOLFDOC()->m_bApproach794 = true;
}

// JP 983 [11] (006a7dc0): abaixo do par e com a bola no green, diz se ha caddie e mostra a chance no ator "Letter".
// Devolve false quando mandou o aviso.
bool CGolfRuleBase::V11_983(bool* pbCaddie)
{
	float duration = 2.0f;
	GolfBall();
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
	if ((unsigned char)pPlayer->stroke[GOLFDOC()->m_currentHole - 1] < GOLFDOC()->GetHoleData().par &&
		GolfBall().m_groundType == 2)
	{
		*pbCaddie = Doc()->m_userInfo[GOLFDOC()->m_currentPlayer].caddieInfo.tid != 0;
		if (IsMyTurn(true))
		{
			MsgObject msg(m_pGolfRule, 0x1fe /*983*/, 0, 0, 0, 0, 0);
			sPlayerData* p = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
			int diff = (unsigned char)p->stroke[GOLFDOC()->m_currentHole - 1] - GOLFDOC()->GetHoleData().par;
			if (diff == -3)
				msg.param1 = (int)"\xbe\xcb\xb9\xd9\xc6\xae\xb7\xce\xbd\xba\xc2\xf9\xbd\xba";
			else if (diff == -2)
				msg.param1 = (int)"\xc0\xcc\xb1\xdb\xc2\xf9\xbd\xba";
			else if (diff == -1)
				msg.param1 = (int)"\xb9\xf6\xb5\xf0\xc2\xf9\xbd\xba";
			msg.param2 = (int)&duration;
			m_pGolfRule->SendMsg983("Letter", msg);
			return false;
		}
	}
	return true;
}

// 00b305b0 (unidade classicserver): bit 1 do dword em m_roomInfo+0xc9
static inline bool IsRoomFlagC9Bit1_983()
{
	return ((*(unsigned long*)((char*)&Doc()->m_roomInfo + 0xc9) >> 1) & 1) == 1;
}

// 006a5ee0: tacadas a mais pelo par do buraco
static inline unsigned char ExtraStroke983(unsigned char par)
{
	if (par == 4)
		return 1;
	if (par == 5)
		return 2;
	return 0;
}

// JP 983 (006a71d0)
void CGolfRuleBase::SendShotReview983()
{
	if (IsRoomFlagC9Bit1_983())
	{
		unsigned char add = ExtraStroke983(GOLFDOC()->GetHoleData().par);
		sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
		pPlayer->stroke[GOLFDOC()->m_currentHole - 1] += add;
		if (Doc()->m_golfGame.gameType == 1)
		{
			sPlayerData* p = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
			GOLFDOC()->GetTeam(p->team)->stroke[GOLFDOC()->m_currentHole - 1] += add;
		}
	}
}

// JP 983 (006a5f90): fundo do buraco seguinte (0x17c ao "GolfBg") e placar (0x160 ao "Score") nos modos 0, 1, 4, 5, 6
void CGolfRuleBase::ShowHoleBg983(unsigned char gameType)
{
	m_pGolfRule->SendMsg983("GolfBg", MsgObject(m_pGolfRule, 0x17c /*983*/, 0, 0, 0, 0, 0));
	switch (gameType)
	{
	case 0:
	case 1:
	case 4:
	case 5:
	case 6:
		m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, 0x160 /*983*/, 2, 0, 0, 0, 0));
		break;
	}
}

// JP 983 (006a6130): passa ao buraco seguinte sem rede (ramo "offline" do CGolfRule::DoScoreBoard do 645)
void CGolfRuleBase::NextHoleOffline983()
{
	CSharedDoc* shared = Doc();
	if (shared->m_bGameOver)
	{
		m_pGolfRule->ChangeGameMode(0x20000);
		return;
	}
	CGolfDoc* doc = shared->m_pGolfDoc;
	if (GetHoleIndex(doc->m_currentHole) == shared->m_golfGame.holes)
	{
		doc->m_currentHole = 1;
		GOLFDOC()->m_bNoStat = true;
		GOLFDOC()->m_pPolySoup->LoadMapCheckData(GOLFDOC()->m_currentHole);
	}
	else
		doc->m_currentHole = shared->m_holeOrder[GetHoleIndex(doc->m_currentHole)];
	ShowHoleBg983(Doc()->m_golfGame.gameType);
}

unsigned char MyIndex(); // 0053a260 (unidade do golfplayer)

C_ASSERT_VC10(offsetof(CSharedDoc, m_bSentHoleStat) == 0x90);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.info.dwIdentity) == 0x812);
C_ASSERT_VC10(offsetof(CSharedDoc, m_pinIndex) == 0x1050c);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeOrder) == 0x1051e);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.gameType) == 0x10594);
C_ASSERT_VC10(offsetof(CSharedDoc, m_golfGame.holes) == 0x10699);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.holeType) == 0x1094f);
C_ASSERT_VC10(offsetof(CSharedDoc, m_roomInfo.realGameType) == 0x109b8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_holeStatistics) == 0xffc8);
C_ASSERT_VC10(offsetof(CSharedDoc, m_bFixHoleMoveTee983) == 0x11608);
C_ASSERT(offsetof(sPangYaUserStatistics, dwShotTime) == 0xc);
C_ASSERT_VC10(offsetof(CGolfDoc, m_teamNum) == 0xd0);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentHole) == 0x17c);
C_ASSERT_VC10(offsetof(CGolfDoc, m_currentPlayer) == 0x189);
C_ASSERT(offsetof(sTeamData, score) == 0x368);
C_ASSERT(offsetof(sTeamData, totalPang) == 0x36c);
C_ASSERT(offsetof(WPolySoup::sCamera, mat) == 0x20 && offsetof(WPolySoup::sCamera, fov) == 0x50);

// JP 983 (006a6f20): sala com holeType 4 e o buraco fechado em uma tacada (ou o byte CSharedDoc+0x11608 ligado)
bool CGolfRuleBase::IsHoleInOne983()
{
	if (Doc()->m_roomInfo.holeType != 4)
		return false;
	if (Doc()->m_bFixHoleMoveTee983 != 0)
		return true;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(GOLFDOC()->m_currentPlayer);
	return pPlayer->stroke[GOLFDOC()->m_currentHole - 1] == (unsigned char)1;
}

// JP 983 (006a6f90): passa ao buraco seguinte com rede (ramo "online" do CGolfRule::DoScoreBoard do 645).
// Novos no 983: pacote 0x141 no hole-in-one da sala de holeType 4, troca de mapa nos modos 0x12 e 0x14
// (no 645 era so o 0xe) e tee/pin pelo alcance do 1W no modo 0xb.
void CGolfRuleBase::NextHole983()
{
	if (IsHoleInOne983())
	{
		WSendPacket packet((enumClientPacket)0x141);
		packet.Send(TO_GAME);
	}
	GolfBall().SetHoleIn(0);
	CSharedDoc* shared = Doc();
	CGolfDoc* doc = shared->m_pGolfDoc;
	if (GetHoleIndex(doc->m_currentHole) == shared->m_golfGame.holes)
	{
		doc->m_currentHole = 1;
		GOLFDOC()->m_bNoStat = true;
	}
	else
	{
		if (shared->m_roomInfo.realGameType == 0x12 || shared->m_roomInfo.realGameType == 0x14)
		{
			SetCurMap(shared->m_pinIndex[GetHoleIndex(doc->m_currentHole)]);
			shared = Doc();
		}
		doc = shared->m_pGolfDoc;
		doc->m_currentHole = shared->m_holeOrder[GetHoleIndex(doc->m_currentHole)];
	}
	GOLFDOC()->m_pPolySoup->LoadMapCheckData(GOLFDOC()->m_currentHole);
	if (Doc()->m_golfGame.gameType == 0xb)
	{
		CGolfDoc* pDoc = GOLFDOC();
		pDoc->SetHoleRange983(pDoc->m_currentHole, GolfClub().GetDriverRange983(0));
	}
	m_pGolfRule->SetNetMsgWait(true, 0x47f, "DoScoreBoard", 40.0f);
	m_pGolfRule->SendHoleData();
	m_pGolfRule->CheckNetMsgWait_Send("DoScoreBoard");
}

// JP 983 [14] (006a8440): comeco da premiacao. E o case 0x20000 do CGolfRule::ChangeGameMode do 645 sem o
// CCapturedBg::Reset e o ResetRank (ficaram no chamador, 00690e6d) e sem a musica do placar.
void CGolfRuleBase::V14_983()
{
	m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, M983(0x12e), 1, 0, 0, 0, 0));
	m_pGolfRule->SendMsg983("GolfBg", MsgObject(m_pGolfRule, M983(0x1e7), 0, 0, 0, 0, 0));
	m_pGolfRule->SendMsg983("GolfBg", MsgObject(m_pGolfRule, M983(0x142), 0, 0, 0, 0, 0));
	WPolySoup::sCamera* pCam = GOLFDOC()->m_pPolySoup->FindCamera("\xbd\xc3\xbb\xf3\xbd\xc4\xc4\xab\xb8\xde\xb6\xf3");
	float fov;
	if (pCam == NULL)
	{
		g_camera.Reset();
		fov = 0.9599311f;
	}
	else
	{
		g_camera = pCam->mat;
		fov = pCam->fov * 0.017453292f;
	}
	g_view->SetFOV(fov);
	m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, M983(0x130), 0, 0, 0, 0, 0));
	m_pGolfRule->SendMsg983("Player", MsgObject(m_pGolfRule, M983(0x119), 0, 0, 0, 0, 0));
	char camName[64] = "";
	if (Doc()->m_golfGame.gameType == 1)
	{
		if (!OnlinePlay())
		{
			// m_teamNum (+0xd0) guarda aqui o time vencedor (nome herdado do 645)
			if (GOLFDOC()->GetTeam(0)->score > GOLFDOC()->GetTeam(1)->score)
				GOLFDOC()->m_teamNum = 0;
			else if (GOLFDOC()->GetTeam(0)->score < GOLFDOC()->GetTeam(1)->score)
				GOLFDOC()->m_teamNum = 1;
			else if (GOLFDOC()->GetTeam(0)->totalPang > (int)GOLFDOC()->GetTeam(1)->totalPang)
				GOLFDOC()->m_teamNum = 0;
			else if (GOLFDOC()->GetTeam(0)->totalPang < (int)GOLFDOC()->GetTeam(1)->totalPang)
				GOLFDOC()->m_teamNum = 1;
		}
		strcpy(camName, "\xbd\xc3\xbb\xf3\xbd\xc4_\xc4\xbf\xc7\xc3");
	}
	else
		strcpy(camName, "\xbd\xc3\xbb\xf3\xbd\xc4_\xb0\xb3\xc0\xce");
	m_pGolfRule->SetModeDuration983(CCameraWork::Instance()->LoadCamera((CCameraWork::eCamMode)1, camName, WVector::ZERO));
	if (!OnlinePlay())
		return;
	switch (Doc()->m_golfGame.gameType)
	{
	case 4:
	case 5:
	case 6:
	case 9:
	case 10:
	case 11:
		break;
	default:
		if (!Doc()->m_bSentHoleStat && ((Doc()->m_myInfo.info.dwIdentity >> 1) & 1) == 0)
		{
			Doc()->m_bSentHoleStat = true;
			WSendPacket packet((enumClientPacket)6);
			Doc()->m_holeStatistics[MyIndex()].dwShotTime = GOLFDOC()->GetPlayer(MyIndex())->shotTime / 1000;
			packet.EncodeBuffer(&Doc()->m_holeStatistics[MyIndex()], sizeof(sPangYaUserStatistics));
			packet.Send(TO_GAME);
		}
		break;
	}
}

// JP 983 (006a80a0): o CGolfRule::DoScoreBoard do 645 repartido (espera de 3 s em 0066e8c0)
void CGolfRuleBase::DoScoreBoard983()
{
	if (!m_pGolfRule->IsModeTimeOver983())
		return;
	if (!OnlinePlay())
	{
		NextHoleOffline983();
		return;
	}
	if (Doc()->m_bGameOver || GOLFDOC()->m_bNetGameEnd != 0)
	{
		m_pGolfRule->ChangeGameMode(0x20000);
		return;
	}
	if (!m_pGolfRule->IsNetWaiting983())
	{
		NextHole983();
		ShowHoleBg983(Doc()->m_golfGame.gameType);
		m_pGolfRule->CheckNetMsgWait_Send("DoScoreBoard");
	}
}


// JP 983 [10] (006a5eb0): vazia na base
void CGolfRuleBase::V10_983(float time)
{
}

// JP 983 [12] (006a5f40): no 645 este trecho estava embutido no CGolfRule (texto "deungjang-mosyeon" ao ator "Player" + camera 0xcb)
void CGolfRuleBase::ReadyNextShot()
{
	m_pGolfRule->SendMsg983("Player",
		MsgObject(m_pGolfRule, 0x2716, (int)"\xb5\xee\xc0\xe5\xb8\xf0\xbc\xc7", 0, 0, 0, 0));
	CCameraWork::Instance()->SelectCamera(0xcb);
}

// JP 983 [8] (006a6050): pergunta ao item de jogo (0x7532, sub 0xc) se a revisao da tacada pode ser ligada; sem rede, liga
void CGolfRuleBase::EnableShotReview()
{
	bool bEnable = false;
	if (WNetworkSystem::Instance()->IsConnected((WNetworkSystem::eNetUnit)0))
		AfxGetTask()->GetActor(s_gatewayActor[1])
			<< MsgObject(NULL, M983(0x26f), 0xc, (int)&bEnable, 0, 0, 0);
	else
		bEnable = true;

	if (bEnable)
		AfxGetTask()->GetActor("ShotReview")
			<< MsgObject(NULL, 0x2fd /*so do 983*/, (int)"ENABLE", bEnable != false, 0, 0, 0);
}

// JP 983 [13] (006a7fb0): o pedido do pacote 0x34 saiu do CGolfRule::DoPlayerPreview para ca
void CGolfRuleBase::RequestShotReview(float time)
{
	if (OnlinePlay() && GOLFDOC()->m_bTeeShotReady == 0)
	{
		GOLFDOC()->m_bTeeShotReady = 1;
		if (Doc()->m_golfGame.gameType == 10 || !IsMassGame())
			m_pGolfRule->SetNetMsgWait(true, 0x37a, "DoPlayerPreview", 20.0f);
		WSendPacket packet((enumClientPacket)0x34);
		packet.Send(TO_GAME);
		SendShotReview983();
		m_pGolfRule->CheckNetMsgWait_Send("DoPlayerPreview");
	}
}

// JP 983 [15] (006a8430)
void CGolfRuleBase::ShowScore(int arg)
{
	DoScoreBoard983();
}

// JP 983: 006a8130 (fastcall, this em ecx): ShowScore [15] do HoleInOne (00544d00), da GuildMatch (006a89a0) e da NewApproach
// (006a96d0). Fora dos tipos 0x0c/0x0e/0x13 pergunta ao "Score" (0x16a) se o placar terminou; fim de jogo = OnHoleIn [9] (se nao
// for approach) e ReturnLobby(0); tipos 0x14/0x15 nao passam ao proximo buraco por aqui.
void CGolfRuleBase::ShowScore983()
{
	unsigned char type = Doc()->m_roomInfo.realGameType;
	if (type != 0x0c && type != 0x0e && type != 0x13)
	{
		bool done = false;
		m_pGolfRule->SendMsg983("Score", MsgObject(m_pGolfRule, 0x16a /*983*/, (unsigned long)&done, 0, 0, 0, 0));
		if (!done)
			return;
	}
	if (!OnlinePlay())
	{
		NextHoleOffline983();
		return;
	}
	if (Doc()->m_bGameOver || GOLFDOC()->m_bNetGameEnd != 0)
	{
		if (!GOLFDOC()->m_bApproach794)
			OnHoleIn();
		m_pGolfRule->ReturnLobby(false);
		return;
	}
	if ((unsigned char)(Doc()->m_roomInfo.realGameType - 0x14) > 1 && !m_pGolfRule->IsNetWaiting983())
	{
		NextHole983();
		ShowHoleBg983(Doc()->m_golfGame.gameType);
		m_pGolfRule->CheckNetMsgWait_Send("DoScoreBoard");
	}
}

// JP 983: 006a8240 (fastcall, tid em ecx; 0 = soma de todos). 00a20330 enche a lista com o typeid do compasso
// (0x1a000040) se o CItemManager o conhece (00a17260 FindItem); a quantidade vem do word +0xc (Common[0]) do item
// da lista do jogador (0063b570).
static short CountCalipers983(unsigned long tid)
{
	std::vector<unsigned long> ids;
	if (Doc()->m_itemManager.FindItem(0x1a000040))
		ids.push_back(0x1a000040);

	unsigned long total = 0;
	std::vector<unsigned long>::iterator it;
	if (tid == 0)
	{
		short sum = 0;
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			sItemInfo* pItem = Doc()->FindMyItemByTypeId983(*it);
			if (pItem)
				sum += pItem->Common[0];
		}
		return sum;
	}

	for (it = ids.begin(); it != ids.end(); ++it)
	{
		if (*it == tid)
		{
			sItemInfo* pItem = Doc()->FindMyItemByTypeId983(tid);
			if (pItem)
				total = (unsigned short)pItem->Common[0];
			break;
		}
	}
	return (short)total;
}

// JP 983: 006a8340 (CGolfRuleBase::CheckCalipersCount; nao usa this). Grava no CSharedDoc +0x10700 (int) a
// quantidade de compassos e +0x10704 (bool) "tem"; fora dos modos 1 e 2 (+0x1048c m_gameMode).
C_ASSERT_VC10(offsetof(CSharedDoc, m_unused47cc) == 0x10700 && offsetof(CSharedDoc, m_unused47d0) == 0x10704); // A745
void CGolfRuleBase::CheckCalipersCount()
{
	Doc()->m_unused47cc = 0;
	Doc()->m_unused47d0 = false;

	if (Doc()->m_gameMode != 1 && Doc()->m_gameMode != 2)
	{
		short count = CountCalipers983(0);
		if (count > 0)
		{
			Doc()->m_unused47cc = count;
			Doc()->m_unused47d0 = true;
		}
	}
}

void CGolfRuleBase::SetPlayer()
{
	if (m_pGolfRule == NULL)
		return;

	if (WNetworkSystem::Instance()->IsConnected(WNetworkSystem::NET_GAME))
	{
		m_pGolfRule->SendHoleData();

		std::list<sSlotInfo>& slotList = Doc()->m_slotList;

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

		m_pGolfRule->LoadShot("save/pangya_000.sav");

		GOLFDOC()->m_pPolySoup->LoadMapCheckData(
			m_pGolfRule->GetShotData()->hole);
	}
}

void CGolfRuleBase::SetGrade()
{
	for (unsigned char i = 0; i < GOLFDOC()->m_playerNum; ++i)
	{
		if (PLAYER(i)->state != 3)
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

void CGolfRuleBase::SetReady()
{
	if (IsLocalContent(S3_AUTOCALIPERS_ITEM))
		CheckCalipersCount();

	AfxGetTask()->GetActor(s_gatewayActor[1])
		<< MsgObject(NULL, M983(0x26f), 8 /*983; 645: 10*/, 0, 0, 0, 0);

	m_pGolfRule->ChangeGameMode(1);
}

void CGolfRuleBase::AddPlayer(unsigned char index, const char* name,
	unsigned long uid, unsigned long oid, unsigned long caddie)
{
	if (index >= GOLFDOC()->m_playerNum)
		return;

	float gauge = 0.0f;

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

	pPlayer->gauge = gauge;

	pPlayer->gauge += Doc()->GetWtPepPangyaComboGauge(index);

	if (IsLocalContent(S4_CARD_SYSTEM))
	{
		CCardManager::Instance()->SetPlayerIndex(index);
		pPlayer->gauge += CCardManager::Instance()->GetCardPeriodComboGauge();
	}

	if (Doc()->m_curChannel.Type & 8)
	{
		pPlayer->gauge = 0;

		for (int k = 0; k < 10; ++k)
			Doc()->m_userInfo[index].userEquip.tidItemSlot[k] = 0;
	}

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

	GOLFDOC()->SetPlayerLevel(index);

	SetMyTicket983(pPlayer);
}

// JP 983 (004cef40): pecas padrao de cada personagem no jogo offline (o 645 so ia ate o 9 e o 2 punha 0x8083001 em [9])
static void SetOfflineDefaultParts983(unsigned long* parts, int character)
{
	switch (character)
	{
	case 0:
		parts[3] = 0x8006001;
		parts[5] = 0x800a000;
		parts[6] = 0;
		parts[7] = 0;
		parts[8] = 0x8010002;
		parts[19] = 0x8026800;
		break;
	case 1:
		parts[0] = 0x8040802;
		parts[2] = 0x8044000;
		parts[4] = 0;
		parts[5] = 0;
		parts[7] = 0x804e001;
		parts[18] = 0x8064800;
		break;
	case 2:
		parts[0] = 0x8080801;
		parts[1] = 0x8083001;
		parts[2] = 0x8084004;
		parts[3] = 0;
		parts[4] = 0x8088801;
		parts[5] = 0x808a004;
		parts[6] = 0;
		parts[7] = 0x808e003;
		parts[8] = 0x8090800;
		parts[17] = 0x80a2800;
		break;
	case 3:
		parts[2] = 0x80c4003;
		parts[3] = 0;
		parts[5] = 0x80ca003;
		parts[7] = 0x80ce003;
		parts[8] = 0x80d0801;
		parts[10] = 0x80d4800;
		parts[18] = 0x80e4800;
		break;
	case 4:
		parts[2] = 0x8104002;
		parts[3] = 0x8106001;
		parts[4] = 0x8108002;
		parts[5] = 0x810a002;
		parts[9] = 0x8112000;
		parts[10] = 0x8114000;
		parts[17] = 0x8122800;
		break;
	case 5:
		parts[23] = 0x816e801;
		break;
	case 6:
		parts[18] = 0x81a4800;
		break;
	case 7:
		parts[21] = 0x81ea800;
		break;
	case 8:
		parts[20] = 0x8228800;
		break;
	case 9:
		parts[2] = 0x824400b;
		parts[5] = 0;
		parts[20] = 0x8268800;
		break;
	case 10:
		parts[19] = 0x82a6800;
		break;
	case 11:
		parts[2] = 0x82c4001;
		parts[4] = 0x82c8000;
		parts[5] = 0x82ca001;
		parts[7] = 0x82ce000;
		break;
	case 12:
		parts[2] = 0x8304001;
		parts[3] = 0x8306400;
		parts[4] = 0x8308400;
		parts[5] = 0x830a400;
		break;
	case 14:
		parts[2] = 0x8384400;
		parts[3] = 0x8386400;
		parts[4] = 0x8388400;
		parts[5] = 0x838a400;
		break;
	}
}

void CGolfRuleBase::AddPlayerOffline(unsigned char index, const char* name,
	unsigned long oid, unsigned long caddie)
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

	strcpy(Doc()->m_userInfo[index].info.sNick, name);

	pPlayer->oid = oid;
	pPlayer->windAbility983 = 0; // 983 006a6440 zera +0x4c0 (o 645 nao tem)
	pPlayer->state = 2;
	pPlayer->chatBlock = 0;
	pPlayer->itemNum = 0;
	pPlayer->item = 0;
	pPlayer->totalStroke = 0;
	pPlayer->shotTime = 0;
	pPlayer->rank = 1;
	pPlayer->gauge = 0;

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

	Doc()->SetIndex(index);
	GOLFDOC()->GetPlayer(index)->index = index;

	Doc()->m_userInfo[index].userEquip.tidItemSlot[0] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[1] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[2] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[3] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[4] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[5] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[6] = 0x18000010;
	Doc()->m_userInfo[index].userEquip.tidItemSlot[7] = 0x18000010;
	GOLFDOC()->GetPlayer(index)->itemNum = 10;

	int character;
	unsigned char hairColor;
	if (index == 0)
	{
		character = COption::Instance()->dGetDefaultCharacter();
		hairColor = (unsigned char)COption::Instance()->dGetDefaultHairColor(); // 983 006a6330: 004925b0 (+0x7a8)
	}
	else
	{
		character = rand() % ItemManager()->GetNumItems(PIG_CHAR);
		hairColor = 0;
	}

	int club = COption::Instance()->dGetDefaultClub();
	int ball = COption::Instance()->dGetDefaultBall();

	unsigned long parts[24];
	unsigned long charTid = character | 0x4000000;
	ItemManager()->GetDefCombo(charTid, parts);
	SetOfflineDefaultParts983(parts, character);

	GOLFDOC()->SetPartTidList(index, charTid, hairColor, 0, parts, NULL);

	if (Doc()->m_offlinePlay)
		GOLFDOC()->GetPartTidList(index).SetDefaultTids();

	Doc()->m_userInfo[index].userEquip.guidChar = 0;
	Doc()->m_userInfo[index].userEquip.guidCaddie = 0;
	Doc()->m_userInfo[index].userEquip.guidClubSet = 0;
	Doc()->m_userInfo[index].userEquip.tidBall = ball | 0x14000000;
	Doc()->m_userInfo[index].userEquip.guidMascot = 0;
	Doc()->m_userInfo[index].charInfo.tid = charTid;
	memcpy(Doc()->m_userInfo[index].charInfo.tidParts, parts, sizeof(parts));
	Doc()->m_userInfo[index].charInfo.tidAuxParts[0] = 0x70000080;
	Doc()->m_userInfo[index].caddieInfo.guid = caddie ? index : 0;
	Doc()->m_userInfo[index].caddieInfo.tid = caddie;
	Doc()->m_userInfo[index].clubInfo.tid = club | 0x10000000;

	rand();

	Doc()->m_userInfo[index].stat.Level = 0;
	Doc()->m_userInfo[index].stat.dwExp = rand() % Doc()->m_levelTable[0].exp;

	GOLFDOC()->SetPlayerLevel(index);
}

int CGolfRuleBase::GetNumTimeout()
{
	return PLAYER(GOLFDOC()->m_currentPlayer)->timeout;
}

bool CGolfRuleBase::CheckGiveUp()
{
	if (PLAYER(GOLFDOC()->m_currentPlayer)->state == 3)
		return false;

	unsigned char limit = GOLFDOC()->GetHoleData().par + 4;
	int state = PLAYER(GOLFDOC()->m_currentPlayer)->state;
	unsigned char stroke = PLAYER(GOLFDOC()->m_currentPlayer)
							   ->stroke[GOLFDOC()->m_currentHole - 1];

	if (state == 1)
		return stroke > limit;

	return stroke >= limit;
}
