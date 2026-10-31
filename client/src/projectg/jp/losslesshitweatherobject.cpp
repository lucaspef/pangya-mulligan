// JP 983: unidade 00569ce0–0056a0a0 (CLosslessHitWeatherObject). Classe nova do 983; como o CPutter10YardObject, com o evento 0x20 do jogador da vez, o aviso 0x35e à "Screen"
// e a instância CControlledRecord::Add<bool> (00569ff0) entre o registro e o On03.
#include "losslesshitweatherobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"
#include "golfball.h"

CLosslessHitWeatherObject::CLosslessHitWeatherObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CLosslessHitWeatherController");
}

CLosslessHitWeatherObject::~CLosslessHitWeatherObject()
{
}

void CLosslessHitWeatherObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreateLosslessHitWeatherObject(eSpecialAbility ability) // 00569d70
{
	return new CLosslessHitWeatherObject(ability);
}

void CLosslessHitWeatherObject::On07(int typeId)
{
	if (typeId == 0 || typeId != m_typeId)
		return;
	if (m_playerIndex != GOLFDOC()->m_currentPlayer)
		return;
	GOLFDOCJP()->EnablePlayerEvent(0x20, GOLFDOC()->m_currentPlayer);
	if (!PVSJP()->IsLosslessWeather())
	{
		GOLFCLUBJP()->Refresh983();
		m_bOn = false;
		m_28 = false;
		return;
	}
	m_bOn = true;
	GOLFCLUBJP()->Refresh983();
	PVSJP()->ApplyLosslessWeather();
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x35e, 0, 0, 0, 0, 0);
	m_28 = false;
}

void CLosslessHitWeatherObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(0x20, GOLFDOC()->m_currentPlayer);
}

void CLosslessHitWeatherObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(0x20, GOLFDOC()->m_currentPlayer);
	else
		GOLFDOCJP()->DisablePlayerEvent(0x20, GOLFDOC()->m_currentPlayer);
}

static bool RegisterLosslessHitWeatherObject() // 00569fa0
{
	return RegisterControlledObject(eSpecialAbility_LosslessHitWeather, CreateLosslessHitWeatherObject);
}

static bool s_registered = RegisterLosslessHitWeatherObject();

void CLosslessHitWeatherObject::On03(const sControlledParam* pParam)
{
	if (ItemManager()->FindCommonItem(pParam->m_pSource->m_typeId) == NULL)
		return;
	m_playerIndex = pParam->m_playerIndex;
	m_typeId = pParam->m_pSource->m_typeId;
	m_value = (unsigned char)(int)pParam->m_pSource->m_values[pParam->m_index];
	m_bOn = false;
	m_record.Add(&m_bOn);
	m_record.Commit();
}

// 009515e0: evento 0x20 ligado e tempo molhado (2/3) com o chao da bola diferente de 2
bool CQuadTreeJp::IsLosslessWeather()
{
	if (!GOLFDOC()->IsCurrentPlayerEvent983(0x20))
		return false;
	unsigned char weather = CSharedDoc::Instance()->m_golfGame.weather;
	if ((unsigned char)(weather - 2) >= 2)
		return false;
	return GolfBall().m_groundType != 2;
}

// 00951780: grava a condicao do chao (m_condition, +0x2098 no 983); com o evento 0x20 em tempo molhado vale 0
void CQuadTreeJp::ApplyLosslessWeather()
{
	CQuadTree* pTree = (CQuadTree*)this;
	unsigned char weather = CSharedDoc::Instance()->m_golfGame.weather;
	if (GOLFDOC()->IsCurrentPlayerEvent983(0x20) && (unsigned char)(weather - 2) < 2)
	{
		pTree->SetCondition(0.0f);
		return;
	}
	switch (weather)
	{
	case 0:
	case 1:
		pTree->SetCondition(0.0f);
		return;
	case 2:
	case 3:
		switch (GolfBall().m_groundType)
		{
		case 0:
		case 1:
		case 3:
			pTree->SetCondition(-2.0f);
			return;
		case 2:
			pTree->SetCondition(0.0f);
			return;
		default:
			pTree->SetCondition(-3.0f);
			return;
		}
	default:
		return;
	}
}
