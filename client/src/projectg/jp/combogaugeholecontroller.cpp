// JP 983: unidade 004a55a0–004a57e0 (CComboGaugeHoleController + CComboGaugeGameController). Classes novas do 983.
// Os dois só mandam o objeto (CComboGaugeObject) somar o que juntou ao medidor de combo: o Hole no slot 5, o Game no slot 4.
#include "shareddoc.h"
#include "golfdoc.h"
#include "combogaugeholecontroller.h"

CComboGaugeHoleController::CComboGaugeHoleController()
{
}

CComboGaugeHoleController::~CComboGaugeHoleController()
{
}

CComboGaugeGameController::CComboGaugeGameController()
{
}

CComboGaugeGameController::~CComboGaugeGameController()
{
}

CComboGaugeObject* CComboGaugeHoleController::GetControlled()
{
	return (CComboGaugeObject*)m_pObject;
}

IMPLEMENT_OBJECT(CComboGaugeHoleController, IController)

bool CComboGaugeHoleController::On05()
{
	if (!IController::On05())
		return false;
	GetControlled()->Apply();
	return true;
}

IMPLEMENT_OBJECT(CComboGaugeGameController, IController)

bool CComboGaugeGameController::On04()
{
	// o 983 chama a mesma 004a5620 do outro controlador (as duas classes não são parentes: o `this` é emprestado)
	((CComboGaugeHoleController*)this)->GetControlled()->Apply();
	return true;
}
