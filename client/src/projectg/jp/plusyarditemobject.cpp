// JP 983: unidade 005e3210–005e3430 (CPlusYardItemObject; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "plusyarditemobject.h"

CPlusYardItemObject::CPlusYardItemObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CPlusYardItemController");
	memset(&m_pSource, 0, 0x10); // o 983 zera +0x50..+0x5f com 4 gravações de 32 bits
}

CPlusYardItemObject::~CPlusYardItemObject()
{
}

void CPlusYardItemObject::On01(const sControlledParam* pParam)
{
	m_54 = pParam->m_00;
	if ((float)m_percent < pParam->m_pSource->m_values[pParam->m_index])
	{
		m_pSource = pParam->m_pSource;
		m_percent = (int)pParam->m_pSource->m_values[pParam->m_index];
	}
	m_bReady = true;
}

bool CPlusYardItemObject::Roll()
{
	if (rand() % 100 < m_percent)
	{
		m_bHit = true;
		return true;
	}
	m_bHit = false;
	return false;
}

bool CPlusYardItemObject::IsHit() const
{
	return m_bHit;
}

bool CPlusYardItemObject::IsReady() const
{
	if (m_pSource == 0)
		return false;
	return m_bReady ? true : false;
}

static CControlledObject* CreatePlusYardItemObject(eSpecialAbility ability) // 005e3340
{
	return new CPlusYardItemObject(ability);
}

static bool RegisterPlusYardItemObject() // 005e33e0
{
	CControlledObject* (*creator)(eSpecialAbility) = CreatePlusYardItemObject;
	eSpecialAbility ability = eSpecialAbility_PlusYardItem;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterPlusYardItemObject();
