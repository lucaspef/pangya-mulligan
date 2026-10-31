#pragma once
#include "controller.h"
#include "trajectorypreviewobject.h"

// JP 983: RTTI .?AVCTrajectoryPreviewController@@, vtable 00d19d00, 0x10 bytes (sem campo próprio).
class CTrajectoryPreviewController : public IController
{
	DECLARE_OBJECT(CTrajectoryPreviewController)
public:
	CTrajectoryPreviewController(); // 006b1a60 (fora de linha: o MakeInstance chama)
	CTrajectoryPreviewObject* GetControlled(); // 006b1aa0
	bool Apply(); // 006b1be0
	virtual bool On04(); // 006b1d20 (salto para o Apply)
	virtual bool On07(int arg); // 006b1b70
};
