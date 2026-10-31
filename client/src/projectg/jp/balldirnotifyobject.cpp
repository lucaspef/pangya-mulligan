// JP 983: unidade 00484330–004844d0 (CBallDirNotifyObject). Classe nova do 983. O corte da F3.7.1 (fim em 0048446e) estava curto:
// o registro da habilidade 0x14 (00484480) é desta unidade.
#include "balldirnotifyobject.h"

CBallDirNotifyObject::CBallDirNotifyObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_typeId(0)
	, m_54(false)
{
	AddController("CBallDirNotifyController");
}

CBallDirNotifyObject::~CBallDirNotifyObject()
{
}

void CBallDirNotifyObject::On01(const sControlledParam* pParam)
{
	m_typeId = pParam->m_pSource->m_typeId;
}

void CBallDirNotifyObject::On05()
{
	m_54 = false;
}

static CControlledObject* CreateBallDirNotifyObject(eSpecialAbility ability) // 004843e0
{
	return new CBallDirNotifyObject(ability);
}

static bool RegisterBallDirNotifyObject() // 00484480
{
	return RegisterControlledObject(eSpecialAbility_BallDirNotify, CreateBallDirNotifyObject);
}

static bool s_registered = RegisterBallDirNotifyObject();
