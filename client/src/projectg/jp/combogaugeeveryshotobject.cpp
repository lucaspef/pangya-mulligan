// JP 983: unidade 004a5990–004a5bc0 (CComboGaugeEveryshotObject). Classe nova do 983; mesma forma do CPangyaCorrectionObject, sem sorteio.
// As duas funções seguintes (004a5bc0, 004a5bd0: functor que soma um float) não têm quem as cite daqui e ficaram sem dono.
#include "shareddoc.h"
#include "golfdoc.h"
#include "combogaugeeveryshotobject.h"

CComboGaugeEveryshotObject::CComboGaugeEveryshotObject(eSpecialAbility ability)
	: CControlledObject(ability)
{
	AddController("CComboGaugeEveryshotController");
	memset(&m_pSource, 0, 0x10); // o 983 zera +0x50..+0x5f com 4 gravações de 32 bits
}

CComboGaugeEveryshotObject::~CComboGaugeEveryshotObject()
{
}

void CComboGaugeEveryshotObject::On01(const sControlledParam* pParam)
{
	m_54 = pParam->m_00;
	if ((float)m_percent <= pParam->m_pSource->m_values[pParam->m_index])
	{
		m_pSource = pParam->m_pSource;
		m_percent = (int)pParam->m_pSource->m_values[pParam->m_index];
	}
	m_bReady = true;
}

bool CComboGaugeEveryshotObject::Roll()
{
	m_bHit = true;
	return true;
}

bool CComboGaugeEveryshotObject::IsReady() const
{
	if (m_pSource == 0)
		return false;
	return m_bReady ? true : false;
}

int CComboGaugeEveryshotObject::GetPercent() const
{
	return m_percent;
}

static CControlledObject* CreateComboGaugeEveryshotObject(eSpecialAbility ability) // 004a5aa0
{
	return new CComboGaugeEveryshotObject(ability);
}

bool CComboGaugeEveryshotObject::IsHit() const
{
	if (GOLFDOC()->GetPlayer((int)GOLFDOC()->m_currentPlayer) == NULL) // o 983 empilha o índice já estendido (movzx): só casa com o valor como int
		return false;
	return m_bHit;
}

static bool RegisterComboGaugeEveryshotObject() // 004a5b70
{
	CControlledObject* (*creator)(eSpecialAbility) = CreateComboGaugeEveryshotObject;
	eSpecialAbility ability = eSpecialAbility_ComboGaugeEveryshot;
	return CControlledObject::Register(ability, creator);
}

static bool s_registered = RegisterComboGaugeEveryshotObject();
