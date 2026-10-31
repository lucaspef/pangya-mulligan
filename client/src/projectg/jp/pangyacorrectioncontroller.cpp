// JP 983: unidade 005ac150–005ac2c0 (CPangyaCorrectionController).
#include "pangyacorrectioncontroller.h"

bool UpdatePangyaCorrection(CPangyaCorrectionObject* pObject) // 005ac1c0
{
	if (!pObject->IsReady())
		return false;
	return pObject->Roll();
}

IMPLEMENT_OBJECT(CPangyaCorrectionController, IController)

CPangyaCorrectionObject* CPangyaCorrectionController::GetControlled()
{
	return (CPangyaCorrectionObject*)m_pObject;
}

bool CPangyaCorrectionController::On05()
{
	if (!IController::On05())
		return false;
	return UpdatePangyaCorrection(GetControlled());
}

bool CPangyaCorrectionController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return UpdatePangyaCorrection(GetControlled());
}
