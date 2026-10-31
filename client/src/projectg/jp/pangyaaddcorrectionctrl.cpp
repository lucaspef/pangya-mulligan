// JP 983: unidade 005abdc0–005abf30 (CPangyaAddCorrectionCtrl; gêmea do CPangyaCorrection, gerada por tools/f37/gemeo.py e conferida no 983).
#include "pangyaaddcorrectionctrl.h"

bool UpdatePangyaAddCorrection(CPangyaAddCorrectionObj* pObject) // 005abe30
{
	if (!pObject->IsReady())
		return false;
	return pObject->Roll();
}

IMPLEMENT_OBJECT(CPangyaAddCorrectionCtrl, IController)

CPangyaAddCorrectionObj* CPangyaAddCorrectionCtrl::GetControlled()
{
	return (CPangyaAddCorrectionObj*)m_pObject;
}

bool CPangyaAddCorrectionCtrl::On05()
{
	if (!IController::On05())
		return false;
	return UpdatePangyaAddCorrection(GetControlled());
}

bool CPangyaAddCorrectionCtrl::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return UpdatePangyaAddCorrection(GetControlled());
}
