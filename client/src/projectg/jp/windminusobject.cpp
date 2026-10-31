// JP 983: unidade 0056bd20–0056c070 (CWindMinusObject). Classe nova do 983; como o CPutter10YardObject, com o evento 0x80 e o aviso 0x215 à "Screen".
// As duas funções seguintes (0056c070/0056c080: construtor e destrutor de CWeatherType) são da unidade vizinha.
#include "windminusobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"

CWindMinusObject::CWindMinusObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CWindMinusController");
}

CWindMinusObject::~CWindMinusObject()
{
}

void CWindMinusObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreateWindMinusObject(eSpecialAbility ability) // 0056bdb0
{
	return new CWindMinusObject(ability);
}

void CWindMinusObject::On07(int typeId)
{
	if (typeId != 0 && typeId == m_typeId)
	{
		GOLFDOCJP()->EnablePlayerEvent(0x80, m_playerIndex);
		m_bOn = true;
		m_28 = false;
	}
}

void CWindMinusObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(0x80, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
}

void CWindMinusObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(0x80, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(0x80, m_playerIndex);
}

static bool RegisterWindMinusObject() // 0056bfa0
{
	return RegisterControlledObject(eSpecialAbility_WindMinus, CreateWindMinusObject);
}

static bool s_registered = RegisterWindMinusObject();

void CWindMinusObject::On03(const sControlledParam* pParam)
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
