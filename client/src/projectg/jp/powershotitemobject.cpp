// JP 983: unidade 005b7bd0–005b7f10 (CPowerShotItemObject). Classe nova do 983; como o CWindMinusObject, com o evento 0x10 do jogador.
// O corte da F3.7.1 (fim em 005b8320) estava longo: de 005b7f10 em diante é outra unidade (efeito/partícula com RFWnd, conversões de
// enumeração, getters de estáticos em 00e78114…00e781e4, "ui_button_ok_click").
#include "powershotitemobject.h"
#include "shareddoc.h"
#include "golfdocjp.h"
#include "taskmanager.h"
#include "actor.h"

CPowerShotItemObject::CPowerShotItemObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0xff)
	, m_typeId(0)
	, m_value(0)
	, m_bOn(false)
{
	AddController("CPowerShotItemController");
}

CPowerShotItemObject::~CPowerShotItemObject()
{
}

void CPowerShotItemObject::On01(const sControlledParam* pParam)
{
	On03(pParam);
}

static CControlledObject* CreatePowerShotItemObject(eSpecialAbility ability) // 005b7c60
{
	return new CPowerShotItemObject(ability);
}

void CPowerShotItemObject::On07(int typeId)
{
	if (typeId != 0 && typeId == m_typeId)
	{
		GOLFDOCJP()->EnablePlayerEvent(0x10, m_playerIndex);
		GOLFDOC()->IsCurrentPlayerEvent983(0x10);
		m_bOn = true;
		m_28 = false;
	}
}

void CPowerShotItemObject::Reset()
{
	if (m_value == 0 || m_value == 100)
		return;
	m_bOn = false;
	GOLFDOCJP()->DisablePlayerEvent(0x10, m_playerIndex);
	AfxGetTask()->GetActor("Screen") << MsgObject(NULL, 0x215, 0, 0, 0, 0, 0);
}

void CPowerShotItemObject::On10(int index)
{
	Reset();
	void* pRecord = m_record.Get(index);
	memcpy_s(&m_bOn, 1, pRecord, 1);
	if (m_bOn)
		GOLFDOCJP()->EnablePlayerEvent(0x10, m_playerIndex);
	else
		GOLFDOCJP()->DisablePlayerEvent(0x10, m_playerIndex);
}

static bool RegisterPowerShotItemObject() // 005b7e40
{
	return RegisterControlledObject(eSpecialAbility_PowerShotItem, CreatePowerShotItemObject);
}

static bool s_registered = RegisterPowerShotItemObject();

void CPowerShotItemObject::On03(const sControlledParam* pParam)
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
