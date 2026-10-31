// JP 983: unidade 0047de90–0047e4b0 (CAutoMiracleSignObject). Classe nova do 983. Por ser o primeiro objeto controlado do
// exe, guarda as instâncias de std::tr1::function<CControlledObject*(eSpecialAbility)> que todos os registros usam
// (0047df60…0047e010, 0047e0c0…0047e430). O que vem depois de 0047e4b0 (CPetFrame::HierarchyChangeObserver, vector<WMatrix>)
// é de outra unidade.
#include "automiraclesignobject.h"

CAutoMiracleSignObject::CAutoMiracleSignObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_playerIndex(0)
	, m_typeId(0)
	, m_value(0)
{
	AddController("CAutoMiracleSignController");
}

CAutoMiracleSignObject::~CAutoMiracleSignObject()
{
}

void CAutoMiracleSignObject::On01(const sControlledParam* pParam)
{
	m_playerIndex = pParam->m_playerIndex;
	m_typeId = pParam->m_pSource->m_typeId;
	m_value = (unsigned long)pParam->m_pSource->m_values[pParam->m_index];
}

static CControlledObject* CreateAutoMiracleSignObject(eSpecialAbility ability) // 0047e020
{
	return new CAutoMiracleSignObject(ability);
}

static bool RegisterAutoMiracleSignObject() // 0047e460
{
	return RegisterControlledObject(eSpecialAbility_AutoMiracleSign, CreateAutoMiracleSignObject);
}

static bool s_registered = RegisterAutoMiracleSignObject();
