// JP 983: unidade 0056a210–0056a440 (CPangyaCorrectionSetObject; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "pangyacorrectionsetobject.h"

CPangyaCorrectionSetObject::CPangyaCorrectionSetObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CPangyaCorrectionSetController");
	memset(&m_pSource, 0, 0x10); // o 983 zera +0x50..+0x5f com 4 gravações de 32 bits
}

CPangyaCorrectionSetObject::~CPangyaCorrectionSetObject()
{
}

void CPangyaCorrectionSetObject::On01(const sControlledParam* pParam)
{
	m_54 = pParam->m_00;
	if ((float)m_percent < pParam->m_pSource->m_values[pParam->m_index])
	{
		m_pSource = pParam->m_pSource;
		m_percent = (int)pParam->m_pSource->m_values[pParam->m_index];
	}
	m_bReady = true;
}

bool CPangyaCorrectionSetObject::Roll()
{
	if (rand() % 100 < m_percent)
	{
		m_bHit = true;
		return true;
	}
	m_bHit = false;
	return false;
}

bool CPangyaCorrectionSetObject::IsHit() // no 983 sorteia de novo em vez de devolver m_bHit (única diferença para o CPangyaCorrectionObject)
{
	return rand() % 100 < m_percent;
}

bool CPangyaCorrectionSetObject::IsReady() const
{
	if (m_pSource == 0)
		return false;
	return m_bReady ? true : false;
}

static CControlledObject* CreatePangyaCorrectionSetObject(eSpecialAbility ability) // 0056a350
{
	return new CPangyaCorrectionSetObject(ability);
}

static bool RegisterPangyaCorrectionSetObject() // 0056a3f0
{
	CControlledObject* (*creator)(eSpecialAbility) = CreatePangyaCorrectionSetObject;
	eSpecialAbility ability = eSpecialAbility_PangyaCorrectionSet;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterPangyaCorrectionSetObject();
