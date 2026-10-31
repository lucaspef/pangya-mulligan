// JP 983: unidade 0056b4b0–0056b790 (CPutter10YardObject). Classe nova do 983.
#include "putter10yardobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"

CPutter10YardObject::CPutter10YardObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CPutter10YardController");
}

CPutter10YardObject::~CPutter10YardObject()
{
}

void CPutter10YardObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreatePutter10YardObject(eSpecialAbility ability) // 0056b540
{
	return new CPutter10YardObject(ability);
}

void CPutter10YardObject::On07(int typeId)
{
	if (typeId != 0 && typeId == m_typeId)
	{
		GOLFDOCJP()->EnablePlayerEvent(0x40, m_playerIndex);
		m_bOn = true;
		m_28 = false;
	}
}

void CPutter10YardObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(0x40, m_playerIndex);
}

void CPutter10YardObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(0x40, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(0x40, m_playerIndex);
}

static bool RegisterPutter10YardObject() // 0056b6c0
{
	return RegisterControlledObject(eSpecialAbility_Putter10Yard, CreatePutter10YardObject);
}

static bool s_registered = RegisterPutter10YardObject();

void CPutter10YardObject::On03(const sControlledParam* pParam)
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
