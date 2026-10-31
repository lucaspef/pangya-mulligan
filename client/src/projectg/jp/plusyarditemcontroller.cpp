// JP 983: unidade 005e30a0–005e3210 (CPlusYardItemController; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "plusyarditemcontroller.h"

bool UpdatePlusYardItem(CPlusYardItemObject* pObject) // 005e3110
{
	if (!pObject->IsReady())
		return false;
	return pObject->Roll();
}

IMPLEMENT_OBJECT(CPlusYardItemController, IController)

CPlusYardItemObject* CPlusYardItemController::GetControlled()
{
	return (CPlusYardItemObject*)m_pObject;
}

bool CPlusYardItemController::On05()
{
	if (!IController::On05())
		return false;
	return UpdatePlusYardItem(GetControlled());
}

bool CPlusYardItemController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return UpdatePlusYardItem(GetControlled());
}
