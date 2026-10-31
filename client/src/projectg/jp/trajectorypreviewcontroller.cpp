// JP 983: unidade 006b1a50–006b1d30 (CTrajectoryPreviewController). Classe nova do 983. O corte da F3.7.1 (início em 006b0fec)
// estava longo: 006b0ff0–006b1a50 é de outras classes (CTopIcon/CWebIcon e um objeto de 0x2bff8 bytes), sem dono.
#include "trajectorypreviewcontroller.h"
#include "shareddocjp.h"
#include "taskmanager.h"
#include "actor.h"

CTrajectoryPreviewController::CTrajectoryPreviewController()
{
}

IMPLEMENT_OBJECT(CTrajectoryPreviewController, IController)

CTrajectoryPreviewObject* CTrajectoryPreviewController::GetControlled()
{
	return (CTrajectoryPreviewObject*)m_pObject;
}

bool CTrajectoryPreviewController::On07(int)
{
	AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"PM_SET_TRAJECTORY_FAKE_FACTOR", 0, 0, 0, 0);
	return true;
}

// 006b1be0: com o item da prévia ligado (0063bba0) e valor > 0, liga a trajetória e o ponto de quique na "ShotPreview"
bool CTrajectoryPreviewController::Apply()
{
	bool bOn = DOCJP()->IsTrajectoryPreviewOn();
	if (!bOn)
		return false;
	CTrajectoryPreviewObject* pObject = GetControlled();
	if (pObject == NULL || !pObject->HasValue())
		return false;
	AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"TOGGLE_TRAJECTORY_PREVIEW", bOn, 0, 0, 0);
	AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"TOGGLE_BOUNDING_SPOT", bOn, 0, 0, 0);
	AfxGetTask()->GetActor("ShotPreview") << MsgObject(NULL, 0x2fd, (unsigned long)"PM_SET_TRAJECTORY_FAKE_FACTOR", 0, 0, 0, 0);
	return true;
}

bool CTrajectoryPreviewController::On04()
{
	return Apply();
}
