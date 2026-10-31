// JP 983: unidade 005abf30–005ac150 (CPangyaAddCorrectionObj; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "pangyaaddcorrectionobj.h"

CPangyaAddCorrectionObj::CPangyaAddCorrectionObj(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CPangyaAddCorrectionCtrl");
	memset(&m_pSource, 0, 0x10); // o 983 zera +0x50..+0x5f com 4 gravações de 32 bits
}

CPangyaAddCorrectionObj::~CPangyaAddCorrectionObj()
{
}

void CPangyaAddCorrectionObj::On01(const sControlledParam* pParam)
{
	m_54 = pParam->m_00;
	if ((float)m_percent < pParam->m_pSource->m_values[pParam->m_index])
	{
		m_pSource = pParam->m_pSource;
		m_percent = (int)pParam->m_pSource->m_values[pParam->m_index];
	}
	m_bReady = true;
}

bool CPangyaAddCorrectionObj::Roll()
{
	if (rand() % 100 < m_percent)
	{
		m_bHit = true;
		return true;
	}
	m_bHit = false;
	return false;
}

bool CPangyaAddCorrectionObj::IsHit() const
{
	return m_bHit;
}

bool CPangyaAddCorrectionObj::IsReady() const
{
	if (m_pSource == 0)
		return false;
	return m_bReady ? true : false;
}

static CControlledObject* CreatePangyaAddCorrectionObject(eSpecialAbility ability) // 005ac060
{
	return new CPangyaAddCorrectionObj(ability);
}

static bool RegisterPangyaAddCorrectionObject() // 005ac100
{
	CControlledObject* (*creator)(eSpecialAbility) = CreatePangyaAddCorrectionObject;
	eSpecialAbility ability = eSpecialAbility_PangyaAddCorrection;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterPangyaAddCorrectionObject();
