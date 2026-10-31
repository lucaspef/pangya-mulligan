// JP 983: unidade 005ac2c0–005ac4e0 (CPangyaCorrectionObject; o registro 005ac490 fica depois do corte 005ac47e da F3.7.1).
#include "pangyacorrectionobject.h"

CPangyaCorrectionObject::CPangyaCorrectionObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CPangyaCorrectionController");
	memset(&m_pSource, 0, 0x10); // o 983 zera +0x50..+0x5f com 4 gravações de 32 bits
}

CPangyaCorrectionObject::~CPangyaCorrectionObject()
{
}

void CPangyaCorrectionObject::On01(const sControlledParam* pParam)
{
	m_54 = pParam->m_00;
	if ((float)m_percent < pParam->m_pSource->m_values[pParam->m_index])
	{
		m_pSource = pParam->m_pSource;
		m_percent = (int)pParam->m_pSource->m_values[pParam->m_index];
	}
	m_bReady = true;
}

bool CPangyaCorrectionObject::Roll()
{
	if (rand() % 100 < m_percent)
	{
		m_bHit = true;
		return true;
	}
	m_bHit = false;
	return false;
}

bool CPangyaCorrectionObject::IsHit() const
{
	return m_bHit;
}

bool CPangyaCorrectionObject::IsReady() const
{
	if (m_pSource == 0)
		return false;
	return m_bReady ? true : false;
}

static CControlledObject* CreatePangyaCorrectionObject(eSpecialAbility ability) // 005ac3f0
{
	return new CPangyaCorrectionObject(ability);
}

static bool RegisterPangyaCorrectionObject() // 005ac490
{
	CControlledObject* (*creator)(eSpecialAbility) = CreatePangyaCorrectionObject;
	eSpecialAbility ability = eSpecialAbility_PangyaCorrection;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterPangyaCorrectionObject();
