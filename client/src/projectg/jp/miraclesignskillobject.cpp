// JP 983: unidade 0056aff0–0056b220 (CMiracleSignSkillObject). Classe nova do 983.
#include "miraclesignskillobject.h"
#include "shareddoc.h"

CMiracleSignSkillObject::CMiracleSignSkillObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0)
	, m_typeId(0)
	, m_percent(0)
	, m_bHit(false)
{
	AddController("CMiracleSignSkillController");
}

CMiracleSignSkillObject::~CMiracleSignSkillObject()
{
}

void CMiracleSignSkillObject::On01(const sControlledParam* pParam)
{
	m_playerIndex = pParam->m_playerIndex;
	m_typeId = pParam->m_pSource->m_typeId;
	m_percent = (unsigned int)pParam->m_pSource->m_values[pParam->m_index];
	m_bHit = false;
}

bool CMiracleSignSkillObject::Roll()
{
	if ((unsigned int)(rand() % 100) < m_percent)
	{
		m_bHit = true;
		return true;
	}
	m_bHit = false;
	return false;
}

static CControlledObject* CreateMiracleSignSkillObject(eSpecialAbility ability) // 0056b0f0
{
	return new CMiracleSignSkillObject(ability);
}

const char* CMiracleSignSkillObject::GetItemName()
{
	IFF_ITEM_COMMON* pItem = ItemManager()->FindCommonItem(m_typeId);
	if (pItem != NULL)
		return pItem->Name;
	return NULL;
}

static bool RegisterMiracleSignSkillObject() // 0056b1b0
{
	return RegisterControlledObject(eSpecialAbility_MiracleSignSkill, CreateMiracleSignSkillObject);
}

static bool s_registered = RegisterMiracleSignSkillObject();
