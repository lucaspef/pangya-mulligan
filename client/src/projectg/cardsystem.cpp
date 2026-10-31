#include "minatl.h"
#include "vc10_layout.h"
#include "cardsystem.h"
#include "carddlg.h"
#include "itembuffactor.h"
#include "wind.h"
#include "club.h"
#include "projectg.h"
#include "jp/classicwind.h"

// JP 983: a unidade vai de 00809300 (Init) a 0080b08a. A ordem das funcoes abaixo e a do 983.
struct CCardManagerLayout983
{
	C_ASSERT_VC10(sizeof(CCardManager) == 0x1384);
	C_ASSERT(sizeof(sCharacterInfo) == 0x201);
	C_ASSERT(sizeof(sClubInfo) == 0x1c);
	C_ASSERT_VC10(offsetof(CCardManager, m_playerSlot) == 0x28);
	C_ASSERT_VC10(offsetof(CCardManager, m_charInfo) == 0x2d);
	C_ASSERT_VC10(offsetof(CCardManager, m_clubInfo) == 0xa32);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodStatus) == 0xac0);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodSlot) == 0xb24);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodComboGauge) == 0xb38);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodPangRate) == 0xb4c);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodPangyaZone) == 0xb60);
	C_ASSERT_VC10(offsetof(CCardManager, m_specialClearBonusProb) == 0xb74);
	C_ASSERT_VC10(offsetof(CCardManager, m_specialClearBonusCourse) == 0xb88);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodRangeUp) == 0xb8c);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodComboGaugeUp) == 0xba0);
	C_ASSERT_VC10(offsetof(CCardManager, m_periodFlag20) == 0xbb4);
	C_ASSERT_VC10(offsetof(CCardManager, m_cardStatus) == 0xbbc);
	C_ASSERT_VC10(offsetof(CCardManager, m_clubCardStatus) == 0xc20);
	C_ASSERT_VC10(offsetof(CCardManager, m_powerRangeDown) == 0xc84);
	C_ASSERT_VC10(offsetof(CCardManager, m_windColor) == 0xc98);
	C_ASSERT_VC10(offsetof(CCardManager, m_caddieCard) == 0xc9c);
	C_ASSERT_VC10(offsetof(CCardManager, m_playerIndex) == 0x109c);
	C_ASSERT_VC10(offsetof(CCardManager, m_activeCardList) == 0x10a0);
	C_ASSERT_VC10(offsetof(CCardManager, m_npcCard) == 0x10ac);
	C_ASSERT_VC10(offsetof(CCardManager, m_pCardBuffDlg) == 0x137c);
	C_ASSERT_VC10(offsetof(CCardManager, m_bSPCardAlarm) == 0x1380);
	C_ASSERT(offsetof(sCharacterInfo, tidParts) == 0xc);
	C_ASSERT(offsetof(sCharacterInfo, ItemIdList) == 0x6c);
	C_ASSERT(offsetof(sCharacterInfo, card) == 0x1d1);
	C_ASSERT(offsetof(sSCardAvilityPeriodInfo, useEndTime) == 0x2c);
	C_ASSERT(offsetof(sSCardAvilityPeriodInfo, cardType) == 0x3c);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, RareType) == 0xc0);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, COM) == 0xea);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, Avility) == 0xf4);
	C_ASSERT(offsetof(IFF_STRUCT::sCard, AvilityValue) == 0xf6);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cardAbilityList) == 0x37ec);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_cardPeriodList) == 0x381c);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_charMap) == 0x3704);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_gameMode) == 0x1048c);
	C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip.guidChar) == 0xa2a);
};

// alcances (jardas) que ligam os efeitos 5-8 e 9-12 do cartao de NPC (00e1e01c, 00e1e02c)
static float s_npcStatusRange983[4] = { 240.0f, 260.0f, 280.0f, 300.0f };
static float s_npcPangyaZoneRange983[4] = { 260.0f, 280.0f, 300.0f, 320.0f };

void CCardManager::Init()
{
	m_playerIndex = 0;

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
		{
			m_cardStatus[i][j] = 0;
			m_periodStatus[i][j] = 0;
			m_clubCardStatus[i][j] = 0;
		}

		m_periodComboGauge[i] = 0;
		m_periodPangRate[i] = 0;
		m_powerRangeDown[i] = 0;
		m_periodPangyaZone[i] = 0;
		m_periodSlot[i] = 0;
		m_specialClearBonusProb[i] = 0;
		m_periodComboGaugeUp[i] = 0;
		m_periodRangeUp[i] = 0;
		m_periodFlag20[i] = 0;

		for (int k = 0; k < 17; ++k)
		{
			m_caddieCard[i][k].effect = 0;
			m_caddieCard[i][k].value = 0;
			m_caddieCard[i][k].type = 0;
		}
		for (int k = 0; k < 12; ++k)
		{
			m_npcCard[i][k].effect = 0;
			m_npcCard[i][k].value = 0;
			m_npcCard[i][k].type = 0;
		}
	}
	m_specialClearBonusCourse = 22;
}

void CCardManager::OnProc(float delta)
{
}

// 008093d0: cartao de personagem (ou preso a uma peca dele)
void CCardManager::AddCharacterCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	if (pCard->COM[0] > 0)
		m_cardStatus[slot][0] += pCard->COM[0];
	if (pCard->COM[1] > 0)
		m_cardStatus[slot][1] += pCard->COM[1];
	if (pCard->COM[2] > 0)
		m_cardStatus[slot][2] += pCard->COM[2];
	if (pCard->COM[3] > 0)
		m_cardStatus[slot][3] += pCard->COM[3];
	if (pCard->COM[4] > 0)
		m_cardStatus[slot][4] += pCard->COM[4];

	switch (pCard->Avility)
	{
	case 1:
		m_powerRangeDown[slot] += pCard->AvilityValue;
		break;
	}
}

// 008094a0: cartao preso ao taco
void CCardManager::AddClubCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	if (pCard->COM[0] > 0)
		m_clubCardStatus[slot][0] += pCard->COM[0];
	if (pCard->COM[1] > 0)
		m_clubCardStatus[slot][1] += pCard->COM[1];
	if (pCard->COM[2] > 0)
		m_clubCardStatus[slot][2] += pCard->COM[2];
	if (pCard->COM[3] > 0)
		m_clubCardStatus[slot][3] += pCard->COM[3];
	if (pCard->COM[4] > 0)
		m_clubCardStatus[slot][4] += pCard->COM[4];

	switch (pCard->Avility)
	{
	case 1:
		m_powerRangeDown[slot] += pCard->AvilityValue;
		break;
	}
}

// 00809570: cartao de caddie
void CCardManager::AddCaddieCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	int effect = pCard->Avility;
	int index = effect - 1;
	m_caddieCard[slot][index].effect = effect;
	m_caddieCard[slot][index].type = pCard->RareType;

	if (m_caddieCard[slot][index].type <= pCard->RareType)
	{
		if (m_caddieCard[slot][index].value < pCard->AvilityValue)
			m_caddieCard[slot][index].value = pCard->AvilityValue;
	}
}

// 008095e0: cartao de NPC
void CCardManager::AddNpcCard983(IFF_STRUCT::sCard* pCard, int slot)
{
	int effect = pCard->Avility;
	int index = effect - 1;
	m_npcCard[slot][index].effect = effect;
	m_npcCard[slot][index].type = pCard->RareType;

	if (m_npcCard[slot][index].type <= pCard->RareType)
	{
		if (m_npcCard[slot][index].value < pCard->AvilityValue)
			m_npcCard[slot][index].value = pCard->AvilityValue;
	}
}

int CCardManager::GetPowerRangeDown()
{
	return m_powerRangeDown[m_playerIndex];
}

float CCardManager::GetCardRangeUp()
{
	return m_caddieCard[m_playerIndex][1].value;
}

float CCardManager::GetCardPowerRangeUp()
{
	return m_caddieCard[m_playerIndex][4].value;
}

int CCardManager::GetCardComboGaugeUp()
{
	return m_caddieCard[m_playerIndex][5].value;
}

float CCardManager::GetCardPangyaZoneUp()
{
	return m_caddieCard[m_playerIndex][6].value * 0.5f;
}

float CCardManager::GetSpecialCardPangyaZoneUp()
{
	return m_periodPangyaZone[m_playerIndex] * 0.5f;
}

float CCardManager::GetPeriodRangeUp983()
{
	return m_periodRangeUp[m_playerIndex];
}

int CCardManager::GetPeriodComboGaugeUp983()
{
	return m_periodComboGaugeUp[m_playerIndex];
}

float CCardManager::GetCaddieCardPangYaZoneUpWind1()
{
	return m_caddieCard[m_playerIndex][10].value * 0.5f;
}

float CCardManager::GetCaddieCardPangYaZoneUpWind7_983()
{
	return m_caddieCard[m_playerIndex][14].value * 0.5f;
}

int CCardManager::GetNpcCardValue983(int index)
{
	return m_npcCard[m_playerIndex][index].value;
}

float CCardManager::GetNpcCardPangyaZone983(int index)
{
	return m_npcCard[m_playerIndex][index].value * 0.5f;
}

float CCardManager::GetNpcCard4Rate983()
{
	return m_npcCard[m_playerIndex][3].value * 0.01;
}

void CCardManager::UpdateCardBuffDlg()
{
	if (m_pCardBuffDlg)

		CItemBuff::UpdateBuffDlg(m_pCardBuffDlg);
}

void CCardManager::ResetSpecialBuffTick()
{
	FrSPCardBuffDlg::SetSpecialBuffStartTick(g_CurrentTime);
}

void CCardManager::SetOffLinePlayerIndex(unsigned char index)
{
	m_playerIndex = index;
}

// 00809950: o 7o cartao do personagem, quando e de caddie, so vale com a peca certa (CSharedDoc 006332f0)
static bool IsSeventhCaddieCard983(unsigned long tid, int number)
{
	if (number == 7 && (tid & 0x3c00000) == 0x400000)
		return true;
	return false;
}

void CCardManager::InitCardBuffDlgPosition()
{
	_WPOINT pos = { -1.0f, 0.0f };
	FrSPCardBuffDlg::SetPosition(*(WPoint*)&pos);
	m_bSPCardAlarm = false;
}

void CCardManager::SetClubInfo983(int slot, const sClubInfo& info)
{
	m_clubInfo[slot] = info;
}

sClubInfo& CCardManager::GetClubInfo983(int slot)
{
	return m_clubInfo[slot];
}

int CCardManager::SetSpecialCardAvility(const sSCardAvilityPeriodInfo& info,
	int slot)
{
	_SYSTEMTIME endTime = info.useEndTime;

	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(info.tid);
	if (pCard == NULL)
		return -1;

	int ret = ItemManager()->CompareSystemTime(endTime, Doc()->GetServerTime());

	if (Doc()->m_gameMode == 5 || Doc()->GetPlayingMode() == 2)
		ret = 1;

	if (ret <= 0)
		return ret;

	switch (pCard->Avility)
	{
	case 5:
		m_periodStatus[slot][0] += pCard->AvilityValue;
		break;

	case 6:
		m_periodStatus[slot][1] += pCard->AvilityValue;
		break;

	case 7:
		m_periodStatus[slot][2] += pCard->AvilityValue;
		break;

	case 8:
		m_periodStatus[slot][3] += pCard->AvilityValue;
		break;

	case 9:
		m_periodStatus[slot][4] += pCard->AvilityValue;
		break;

	case 10:
		m_periodComboGauge[slot] = pCard->AvilityValue;
		break;

	case 11:
		m_periodSlot[slot] = pCard->AvilityValue;
		break;

	case 2:
		m_periodPangRate[slot] = pCard->AvilityValue;
		break;

	case 12:
		m_periodPangyaZone[slot] = pCard->AvilityValue;
		break;
	case 13:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 2;
		break;
	case 14:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 3;
		break;
	case 15:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 11;
		break;
	case 16:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 6;
		break;
	case 20:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 0;
		break;
	case 21:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 1;
		break;
	case 22:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 10;
		break;
	case 23:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 13;
		break;
	case 24:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 7;
		break;
	case 25:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 16;
		break;
	case 26:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 15;
		break;
	case 27:
		m_periodRangeUp[slot] = pCard->AvilityValue;
		break;
	case 28:
		m_periodComboGaugeUp[slot] = pCard->AvilityValue;
		break;
	case 29:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 18;
		break;
	case 30:
		m_specialClearBonusProb[slot] = pCard->AvilityValue * 0.01;
		m_specialClearBonusCourse = 19;
		break;
	case 32:
		m_periodFlag20[slot] = 1;
		break;
	}

	return ret;
}

float CCardManager::GetTotalRange(bool bPowerRange)
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	float range = GetCardRangeUp() + 0.0;
	if (bPowerRange == true)
		range += GetCardPowerRangeUp();
	range += GetPeriodRangeUp983();
	return range - GetPowerRangeDown();
}

int CCardManager::GetTotalComboGauge983()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;
	return GetCardComboGaugeUp() + GetPeriodComboGaugeUp983();
}

int CCardManager::GetCardPeriodStatus(int index)
{
	if ((GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf) || Doc()->m_golfGame.gameType == 0xf ||
		Doc()->m_golfGame.gameType == 0x10)
		return 0;
	int status = 0;
	if (m_periodStatus[m_playerIndex][index] > 0)
		status = m_periodStatus[m_playerIndex][index];
	return status;
}

int CCardManager::GetCardStatusSlot(int index)
{
	if ((GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf) || Doc()->m_golfGame.gameType == 0xf ||
		Doc()->m_golfGame.gameType == 0x10)
		return 0;
	int status = 0;
	if (m_cardStatus[m_playerIndex][index] > 0)
		status = m_cardStatus[m_playerIndex][index];
	return status;
}

int CCardManager::GetClubCardStatusSlot983(int index)
{
	if ((GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf) || Doc()->m_golfGame.gameType == 0xf ||
		Doc()->m_golfGame.gameType == 0x10)
		return 0;
	int status = 0;
	if (m_clubCardStatus[m_playerIndex][index] > 0)
		status = m_clubCardStatus[m_playerIndex][index];
	return status;
}

float CCardManager::GetCardPeriodComboGauge()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	return m_periodComboGauge[m_playerIndex];
}

int CCardManager::GetCardPeriodSlot()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;
	return m_periodSlot[m_playerIndex];
}

float CCardManager::GetCardPeriodPangRate()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	return m_periodPangRate[m_playerIndex];
}

int CCardManager::GetCardSuccesssProbUp()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;
	return m_caddieCard[m_playerIndex][0].value;
}

__int64 CCardManager::GetCardBounceBonusUp()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;
	return m_caddieCard[m_playerIndex][3].value;
}

// 0080a160. Efeitos do caddie: 13 (vento de 2 a 5 tira 1, de 6 a 9 tira 2), 17 (vento de 6 a 9), 3 (qualquer vento)
// e 12 (so vento 9). O primeiro que existir decide; os outros so entram se nenhum anterior existir.
unsigned char CCardManager::GetWindPowerDown(float wind)
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;

	unsigned char down = 0;
	sCaddieCardAvility* pCard = m_caddieCard[m_playerIndex];
	bool bApplied = false;

	if (pCard[12].value > 0)
	{
		bApplied = true;
		if ((2.0f <= wind || WisEqual(wind, 2.0f, 0.00001f)) &&
			(wind <= 5.0f || WisEqual(wind, 5.0f, 0.00001f)))
			down = 1;
		else if ((6.0f <= wind || WisEqual(wind, 6.0f, 0.00001f)) &&
			(wind <= 9.0f || WisEqual(wind, 9.0f, 0.00001f)))
			down = 2;
	}

	if (pCard[16].value > 0 && !bApplied)
	{
		bApplied = true;
		if ((6.0f <= wind || WisEqual(wind, 6.0f, 0.00001f)) &&
			(wind <= 9.0f || WisEqual(wind, 9.0f, 0.00001f)))
		{
			if (down < pCard[16].value)
				down = pCard[16].value;
		}
	}

	if (down < pCard[2].value && !bApplied)
	{
		bApplied = true;
		down = pCard[2].value;
	}

	if (Wabs(wind - 9.0) < 0.00001f && !bApplied)
	{
		if (down < pCard[11].value)
			down = pCard[11].value;
	}
	return down;
}

float CCardManager::GetSpecialClearBonusProb(unsigned char course)
{
	if (course > 21 || course != m_specialClearBonusCourse)

		return 0.0f;

	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;

	return m_specialClearBonusProb[m_playerIndex];
}

float CCardManager::GetCaddieCard16_983()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	return m_caddieCard[m_playerIndex][15].value;
}

// JP 983 (0080a3f0): efeito 1 do cartao de NPC (taxa de pang)
float CCardManager::GetCardPeriodPangRate2_983()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	return m_npcCard[m_playerIndex][0].value;
}

float CCardManager::GetNpcCard3Rate983()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;
	return m_npcCard[m_playerIndex][2].value * 0.01;
}

unsigned char CCardManager::GetPeriodFlag20_983()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;
	return m_periodFlag20[m_playerIndex];
}

void CCardManager::SetPlayerIndex(unsigned char index)
{
	if (index == 0xff)
		m_playerIndex = 0;
	else if (Doc()->m_gameMode == 5)
		CCardManager::Instance()->SetOffLinePlayerIndex(index);
	else if (Doc()->GetPlayingMode() == 2)
		CCardManager::Instance()->SetOffLinePlayerIndex(index);
	else if (Doc()->m_gameMode == 1)
		CCardManager::Instance()->SetOffLinePlayerIndex(index + 1);
	else
		m_playerIndex = m_playerSlot[index];
}

// 0080a620: so o indice 1 tem efeito de NPC (efeitos 5-8, pelo alcance do taco na mao)
int CCardManager::GetNpcCardStatus983(int index)
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0;

	int status = 0;
	if (index == 1)
	{
		float range = GetCurrentRange983();
		if (range != 0.0f)
		{
			for (int i = 0; i < 4; ++i)
			{
				if (range >= s_npcStatusRange983[i] && GetNpcCardValue983(i + 4) > status)
					status = GetNpcCardValue983(i + 4);
			}
		}
	}
	return status;
}

// 0080a6b0: efeitos 9-12 do cartao de NPC
float CCardManager::GetNpcCardPangyaZoneByRange983()
{
	float zone = 0.0f;
	float range = GetCurrentRange983();
	if (range == 0.0f)
		return 0.0f;

	for (int i = 0; i < 4; ++i)
	{
		if (range >= s_npcPangyaZoneRange983[i])
		{
			if (GetNpcCardPangyaZone983(i + 8) > zone)
				zone = GetNpcCardPangyaZone983(i + 8);
		}
	}
	return zone;
}

float CCardManager::GetTotalPangyaZone()
{
	if (GOLFDOC() != NULL && GOLFDOC()->m_tutorialMode != 0xf)
		return 0.0f;

	float wind = ((CWindJp*)&Wind())->GetPower983();
	float card = CCardManager::Instance()->GetCardPangyaZoneUp();
	float zone = card + 0.0;
	zone += CCardManager::Instance()->GetSpecialCardPangyaZoneUp();

	if (Wabs(wind - 1.0) < 0.00001f && card == 0.0f)
		zone += CCardManager::Instance()->GetCaddieCardPangYaZoneUpWind1();
	if (wind >= 7.0)
		zone += CCardManager::Instance()->GetCaddieCardPangYaZoneUpWind7_983();
	zone += GetNpcCardPangyaZoneByRange983();
	return zone;
}

// 0080a810. O `continue` sem avancar quando o cartao nao existe no Card.iff e o salto de um no depois de apagar
// estao assim no 983 (00a847 -> 00a8c6 e 00a8ba -> 00a8c4).
void CCardManager::CalcCardPeriodAndStatus983()
{
	Init();

	for (std::list<sSCardAvilityPeriodInfo>::iterator it = Doc()->m_cardPeriodList.begin();
		it != Doc()->m_cardPeriodList.end();)
	{
		IFF_STRUCT::sCard* pCard = ItemManager()->FindCard((*it).tid);
		if (pCard == NULL)
			continue;

		switch ((*it).cardType)
		{
		case 0:
			AddCharacterCard983(pCard, 0);
			break;
		case 1:
			AddCaddieCard983(pCard, 0);
			break;
		case 5:
			AddNpcCard983(pCard, 0);
		case 2:
			if (SetSpecialCardAvility(*it, 0) == 0)
			{
				it = Doc()->m_cardPeriodList.erase(it);
				UpdateCardBuffDlg();
			}
			break;
		}
		++it;
	}
}

// 983: sCharacterInfo::sCharacterInfo (00500230) fica fora de linha no ctor
#pragma inline_depth(0)
CCardManager::CCardManager()
{
	Init();

	for (int i = 0; i < 5; ++i)
		m_playerSlot[i] = 0;
	m_pCardBuffDlg = NULL;
	m_bSPCardAlarm = false;
	m_windColor = 0;

	sClubInfo info;
	memset(&info, 0, sizeof(info));
	for (int i = 0; i < 5; ++i)
		m_clubInfo[i] = info;
}
#pragma inline_depth()
CCardManager::~CCardManager()
{
}

void CCardManager::CalcCardPeriodAndStatus()
{
	Init();

	int slot = 0;

	for (int i = 0; i < 4; ++i)
	{
		if (Doc()->m_gameMode == 1)
		{
			i = 0;

			if (++slot > 5)
				return;
		}
		else
			slot = i;

		sCharacterInfo* pChar;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
			pChar = &Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		else
			pChar = &m_charInfo[slot];

		bool bSeventh = Doc()->HasSeventhCardSlot983(pChar);

		unsigned long* pTid = &pChar->card[0][0];
		for (int k = 0; k < 12; ++k, ++pTid)
		{
			IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(*pTid);
			if (pCard == NULL)
				continue;

			switch ((*pTid >> 22) & 0xf)
			{
			case 0:
				AddCharacterCard983(pCard, slot);
				break;
			case 1:
				if (!IsSeventhCaddieCard983(pCard->c.TypeId, k + 1) || bSeventh)
					AddCaddieCard983(pCard, slot);
				break;
			case 5:
				AddNpcCard983(pCard, slot);
				break;
			}
		}

		for (std::list<sSCardAvilityPeriodInfo>::iterator it =
				 Doc()->m_cardAbilityList[i].begin();
			it != Doc()->m_cardAbilityList[i].end();)
		{
			if ((*it).cardType == 2 && SetSpecialCardAvility(*it, slot) != 0)
			{
				++it;
				continue;
			}

			it = Doc()->m_cardAbilityList[i].erase(it);

			UpdateCardBuffDlg();
		}
	}
}

// 0080ac70 (sem chamador no 983): cartao preso a uma peca do personagem (tipo 2) ou ao taco (tipo 4)
void CCardManager::SetCharacterCardAvility(const sSCardAvilityPeriodInfo& info,
	int slot)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(info.tid);
	if (pCard == NULL)
		return;

	switch (info.partsTid >> 26)
	{
	case 4:
	{
		unsigned long guid;
		unsigned long tid;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
		{
			guid = Doc()->m_myInfo.userEquip.guidClubSet;
			tid = (*Doc()->m_clubSetMap.find(guid)).second.tid;
		}
		else
		{
			sClubInfo& club = GetClubInfo983(slot);
			guid = club.guid;
			tid = club.tid;
		}
		if (guid == info.partsUid && tid == info.partsTid)
			AddClubCard983(pCard, slot);
		break;
	}
	case 2:
	{
		sCharacterInfo charInfo;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
			charInfo = Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		else
			charInfo = m_charInfo[slot];

		for (int i = 0; i < 24; ++i)
		{
			if (info.partsTid == charInfo.tidParts[i] &&
				info.partsUid == charInfo.ItemIdList[i])
				AddCharacterCard983(pCard, slot);
		}
		break;
	}
	}
}

// 0080ae00 (sem chamador no 983)
void CCardManager::SetCaddieCardAvility(const sSCardAvilityPeriodInfo& info,
	int slot)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(info.tid);
	if (pCard == NULL)
		return;

	switch (info.partsTid >> 26)
	{
	case 4:
	{
		unsigned long guid;
		unsigned long tid;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
		{
			guid = Doc()->m_myInfo.userEquip.guidClubSet;
			tid = (*Doc()->m_clubSetMap.find(guid)).second.tid;
		}
		else
		{
			sClubInfo& club = GetClubInfo983(slot);
			guid = club.guid;
			tid = club.tid;
		}
		if (guid == info.partsUid && tid == info.partsTid)
			AddCaddieCard983(pCard, slot);
		break;
	}
	case 2:
	{
		sCharacterInfo charInfo;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
			charInfo = Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		else
			charInfo = m_charInfo[slot];

		for (int i = 0; i < 24; ++i)
		{
			if (info.partsTid == charInfo.tidParts[i] &&
				info.partsUid == charInfo.ItemIdList[i])
				AddCaddieCard983(pCard, slot);
		}
		break;
	}
	}
}

// 0080af90 (sem chamador no 983)
void CCardManager::SetNpcCardAvility983(const sSCardAvilityPeriodInfo& info,
	int slot)
{
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(info.tid);
	if (pCard == NULL)
		return;

	if (info.partsTid >> 26 == 2)
	{
		sCharacterInfo charInfo;
		if (slot == 0 && Doc()->GetPlayingMode() != 2)
			charInfo = Doc()->m_charMap[Doc()->m_myInfo.userEquip.guidChar];
		else
			charInfo = m_charInfo[slot];

		for (int i = 0; i < 24; ++i)
		{
			if (info.partsTid == charInfo.tidParts[i] &&
				info.partsUid == charInfo.ItemIdList[i])
				AddNpcCard983(pCard, slot);
		}
	}
}

C_ASSERT_VC10(offsetof(CSharedDoc, m_userInfo) + offsetof(sUserInfo, userEquip) == 0x3b36);
C_ASSERT_VC10(offsetof(CSharedDoc, m_myInfo.userEquip) == 0xa26);

float CCardManager::GetCurrentRange983() // 0080a470
{
	if (!OnlinePlay())
		return 0.0f;

	if (IS_KINDOF(CGolfTask, AfxGetTask()))
	{
		return GolfClub().GetRangeByEquip983(Doc()->m_pGolfDoc->m_currentPlayer,
			&Doc()->m_userInfo[Doc()->m_pGolfDoc->m_currentPlayer].userEquip);
	}

	float range;
	if (m_playerIndex == 0 && Doc()->GetPlayingMode() != 2)
		range = GolfClub().GetRangeByEquip983(0xff, &Doc()->m_myInfo.userEquip);
	else
		range = GolfClub().GetRangeByEquip983(m_playerIndex, &Doc()->m_userInfo[m_playerIndex].userEquip);
	return range;
}
