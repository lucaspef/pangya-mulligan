// JP 983: unidade 00627610–006277b0 (CSetEffectController). Classe nova do 983. Guarda as cópias fora de linha de dois acessos
// ao global 00e768f8: 00627700 = CMotionAdvisor::Instance, 00627710 = CMotionAdvisor::IsInstantiated.
#include "seteffectcontroller.h"
#include "shareddocjp.h"
#include "motionadvisor.h"

IMPLEMENT_OBJECT(CSetEffectController, IController)

CSetEffectObject* CSetEffectController::GetControlled()
{
	return (CSetEffectObject*)m_pObject;
}

bool CSetEffectController::On03()
{
	if (m_0c != 0x10 && CMotionAdvisor::IsInstantiated())
		CMotionAdvisor::Instance()->Reset();
	return true;
}

bool CSetEffectController::On04()
{
	CSetEffectObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	pObject->Apply(m_0c);
	return true;
}

bool CSetEffectController::On11()
{
	CSetEffectObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	pObject->Apply(m_0c);
	return true;
}

bool CSetEffectController::On02()
{
	CMotionAdvisor::Instance()->Reset();
	DOCJP()->ClearSetEffect983();
	return true;
}
