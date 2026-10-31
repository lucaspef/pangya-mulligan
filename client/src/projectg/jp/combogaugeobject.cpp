// JP 983: unidade 004a5bf0–004a8a50 (CComboGaugeLimitIncrease + CComboGaugeObject + CComboGaugeGameObject). Classes novas do
// 983. O mapa da F3.7.1 cortava a faixa em três (combogaugelimitincrease, "sem classe" 004a635e, combogaugeobject): é uma só.
#include "shareddoc.h"
#include "golfdoc.h"
#include "combogaugeobject.h"

CComboGaugeLimitIncrease::CComboGaugeLimitIncrease(eSpecialAbility ability)
	: CControlledObject(ability)
{
}

CComboGaugeLimitIncrease::~CComboGaugeLimitIncrease()
{
}

// 004a5c20: zera tudo e guarda o item/valor do jogador do argumento (o teste só serve dentro da mesma chamada)
void CComboGaugeLimitIncrease::On01(const sControlledParam* pParam)
{
	memset(m_typeId, 0, sizeof(m_typeId));
	memset(m_value, 0, sizeof(m_value));
	if (m_typeId[pParam->m_00] == 0)
	{
		m_typeId[pParam->m_00] = pParam->m_pSource->m_typeId;
		m_value[pParam->m_00] = pParam->m_pSource->m_values[pParam->m_index];
	}
}

float CComboGaugeLimitIncrease::GetValue(unsigned char index)
{
	return m_value[index];
}

static CControlledObject* CreateComboGaugeLimitIncrease(eSpecialAbility ability) // 004a62d0
{
	return new CComboGaugeLimitIncrease(ability);
}

float CComboGaugeObject::Sum(const std::vector<sBonus>& list)
{
	float sum = 0.0f;
	std::vector<sBonus>::const_iterator end = list.end();
	for (std::vector<sBonus>::const_iterator it = list.begin(); it != end; ++it)
		sum += it->value;
	return sum;
}

// 004a6d90: soma o bônus de cada jogador ao medidor, entre 0 e o limite (CSharedDoc::GetComboGaugeLimit). No modo de jogo 1
// o medidor é o do time e vale o maior bônus visto até ali em cada time.
void CComboGaugeObject::Apply()
{
	if (Doc()->m_golfGame.gameType == 1)
	{
		float best0 = 0.0f;
		float best1 = 0.0f;
		for (BonusMap::iterator it = m_bonus.begin(); it != m_bonus.end(); ++it)
		{
			unsigned char player = it->first;
			float gauge = Doc()->GetComboGaugeLimit(player);
			float bonus = Sum(it->second);
			if (GOLFDOC()->GetPlayer(player)->team == 0)
			{
				if (bonus > best0)
					best0 = bonus;
				bonus = GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer(player)->team)->gauge + best0;
				gauge = Min(Max(bonus, 0.0f), gauge);
				GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer(player)->team)->gauge = gauge;
			}
			else
			{
				if (bonus > best1)
					best1 = bonus;
				bonus = GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer(player)->team)->gauge + best1;
				gauge = Min(Max(bonus, 0.0f), gauge);
				GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer(player)->team)->gauge = gauge;
			}
		}
	}
	else
	{
		for (BonusMap::iterator it = m_bonus.begin(); it != m_bonus.end(); ++it)
		{
			unsigned char player = it->first;
			float limit = Doc()->GetComboGaugeLimit(player);
			float bonus = Sum(it->second);
			bonus = GOLFDOC()->GetPlayer(player)->gauge + bonus;
			float gauge = Min(Max(bonus, 0.0f), limit);
			GOLFDOC()->GetPlayer(player)->gauge = gauge;
		}
	}
}

// 004a7230: bônus juntado do jogador (no modo de jogo 1, mais o do parceiro de time)
float CComboGaugeObject::GetBonus(unsigned char player)
{
	float bonus = 0.0f;
	if (Doc()->m_golfGame.gameType == 1)
	{
		BonusMap::iterator it = m_bonus.find(player);
		if (it != m_bonus.end())
			bonus = Sum(it->second);
		if (GOLFDOC()->GetPlayer(player)->teammate != 0xff)
		{
			BonusMap::iterator mate = m_bonus.find(GOLFDOC()->GetPlayer(player)->teammate);
			if (mate != m_bonus.end())
				bonus += Sum(mate->second);
		}
	}
	else
	{
		BonusMap::iterator it = m_bonus.find(player);
		if (it != m_bonus.end())
			bonus = Sum(it->second);
	}
	return bonus;
}

static bool RegisterComboGaugeLimitIncrease() // 004a7310
{
	CControlledObject* (*creator)(eSpecialAbility) = CreateComboGaugeLimitIncrease;
	eSpecialAbility ability = eSpecialAbility_ComboGaugeLimitIncrease;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterComboGaugeLimitIncrease();

// 004a8410: guarda o bônus dos itens de tipo 2, 16 e 28 (6 bits altos do typeid) na lista do jogador
void CComboGaugeObject::On01(const sControlledParam* pParam)
{
	sBonus bonus;
	bonus.value = pParam->m_pSource->m_values[pParam->m_index];
	bonus.typeId = pParam->m_pSource->m_typeId;
	bonus.player = pParam->m_playerIndex;
	switch (bonus.typeId >> 26)
	{
	case 2:
	case 16:
	case 28:
	{
		BonusMap::iterator it = m_bonus.find(bonus.player);
		if (it == m_bonus.end())
		{
			std::vector<sBonus> list;
			list.push_back(bonus);
			m_bonus.insert(std::make_pair(bonus.player, list));
		}
		else
			it->second.push_back(bonus);
		break;
	}
	}
}

CComboGaugeObject::~CComboGaugeObject()
{
}

CComboGaugeGameObject::~CComboGaugeGameObject()
{
}

CComboGaugeObject::CComboGaugeObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CComboGaugeHoleController");
}

CComboGaugeGameObject::CComboGaugeGameObject(eSpecialAbility ability)
	: CComboGaugeObject(ability)
{
	AddController("CComboGaugeGameController");
	RemoveController("CComboGaugeHoleController");
}

static CControlledObject* CreateComboGaugeObject(eSpecialAbility ability) // 004a88d0
{
	return new CComboGaugeObject(ability);
}

static bool RegisterComboGaugeObject() // 004a8940
{
	return RegisterControlledObject(eSpecialAbility_ComboGauge, CreateComboGaugeObject);
}

static bool s_registeredComboGauge = RegisterComboGaugeObject();

static CControlledObject* CreateComboGaugeGameObject(eSpecialAbility ability) // 004a8990
{
	return new CComboGaugeGameObject(ability);
}

static bool RegisterComboGaugeGameObject() // 004a8a00
{
	return RegisterControlledObject(eSpecialAbility_ComboGaugeGame, CreateComboGaugeGameObject);
}

static bool s_registeredComboGaugeGame = RegisterComboGaugeGameObject();
