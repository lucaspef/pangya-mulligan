// JP 983: unidade 0056b8c0–0056ba80 (CStatusSlotPlusObject). Classe nova do 983.
#include "statusslotplusobject.h"

static const std::string s_controllerName("CStatusSlotPlusController"); // 00e0c5f0 (inicializador 00cceb90)

CStatusSlotPlusObject::CStatusSlotPlusObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_50(-1)
	, m_54(0)
	, m_58(false)
	, m_59(false)
{
	AddController("CStatusSlotPlusController");
}

CStatusSlotPlusObject::~CStatusSlotPlusObject()
{
}

static CControlledObject* CreateStatusSlotPlusObject(eSpecialAbility ability) // 0056b990
{
	return new CStatusSlotPlusObject(ability);
}

static bool RegisterStatusSlotPlusObject() // 0056ba30
{
	return RegisterControlledObject(eSpecialAbility_StatusSlotPlus, CreateStatusSlotPlusObject);
}

static bool s_registered = RegisterStatusSlotPlusObject(); // 00e75c51 (inicializador 00ccebb0)
