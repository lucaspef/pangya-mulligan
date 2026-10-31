// JP 983: unidade 0056a0a0–0056a210 (CPangyaCorrectionSetController; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "pangyacorrectionsetcontroller.h"

bool UpdatePangyaCorrectionSet(CPangyaCorrectionSetObject* pObject) // 0056a110
{
	if (!pObject->IsReady())
		return false;
	return pObject->Roll();
}

IMPLEMENT_OBJECT(CPangyaCorrectionSetController, IController)

CPangyaCorrectionSetObject* CPangyaCorrectionSetController::GetControlled()
{
	return (CPangyaCorrectionSetObject*)m_pObject;
}

bool CPangyaCorrectionSetController::On05()
{
	if (!IController::On05())
		return false;
	return UpdatePangyaCorrectionSet(GetControlled());
}

bool CPangyaCorrectionSetController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return UpdatePangyaCorrectionSet(GetControlled());
}
