// JP 983: unidade 004a57e0–004a5990 (CComboGaugeEveryshotController). Classe nova do 983; como o CPangyaCorrectionController,
// mais a checagem do jogador da vez.
#include "shareddoc.h"
#include "golfdoc.h"
#include "combogaugeeveryshotcontroller.h"

bool UpdateComboGaugeEveryshot(CComboGaugeEveryshotObject* pObject) // 004a5850
{
	if (!pObject->IsReady())
		return false;
	return pObject->Roll();
}

IMPLEMENT_OBJECT(CComboGaugeEveryshotController, IController)

CComboGaugeEveryshotObject* CComboGaugeEveryshotController::GetControlled()
{
	return (CComboGaugeEveryshotObject*)m_pObject;
}

bool CComboGaugeEveryshotController::On05()
{
	if (!IController::On05())
		return false;
	if (GOLFDOC()->GetPlayer((int)GOLFDOC()->m_currentPlayer) == NULL)
		return false;
	return UpdateComboGaugeEveryshot(GetControlled());
}

bool CComboGaugeEveryshotController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	if (GOLFDOC()->GetPlayer((int)GOLFDOC()->m_currentPlayer) == NULL)
		return false;
	return UpdateComboGaugeEveryshot(GetControlled());
}
