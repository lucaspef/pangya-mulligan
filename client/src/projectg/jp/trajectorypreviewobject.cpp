// JP 983: unidade 006b1d30–006b1ee0 (CTrajectoryPreviewObject). Classe nova do 983. O corte da F3.7.1 (fim em 006b1f60) estava
// longo: 006b1ee0–006b1f60 solta dois WResource e mexe em campos de outra classe (sem dono).
#include "trajectorypreviewobject.h"

CTrajectoryPreviewObject::CTrajectoryPreviewObject(eSpecialAbility ability)
	: CControlledObject(ability)
	, m_typeId(0)
	, m_value(0)
{
	AddController("CTrajectoryPreviewController");
}

CTrajectoryPreviewObject::~CTrajectoryPreviewObject()
{
}

void CTrajectoryPreviewObject::On01(const sControlledParam* pParam)
{
	m_typeId = pParam->m_pSource->m_typeId;
	m_value = (unsigned long)pParam->m_pSource->m_values[pParam->m_index];
}

static CControlledObject* CreateTrajectoryPreviewObject(eSpecialAbility ability) // 006b1df0
{
	return new CTrajectoryPreviewObject(ability);
}

static bool RegisterTrajectoryPreviewObject() // 006b1e90
{
	return RegisterControlledObject(eSpecialAbility_TrajectoryPreview, CreateTrajectoryPreviewObject);
}

static bool s_registered = RegisterTrajectoryPreviewObject();
