#pragma once
#include "controller.h"
#include "powershotitemobject.h"

// JP 983: RTTI .?AVCPowerShotItemController@@, vtable 00d0e324, 0x10 bytes (sem campo próprio).
class CPowerShotItemController : public IController
{
	DECLARE_OBJECT(CPowerShotItemController)
public:
	CPowerShotItemController() { } // 005b7890 (embutido: o MakeInstance grava a vtable direto)
	CPowerShotItemObject* GetControlled(); // 005b79b0
	void Send(CPowerShotItemObject* pObject); // 005b79d0
	virtual bool On05(); // 005b7b60
	virtual bool On06(float arg); // 005b7a60
	virtual bool On07(int arg); // 005b7ba0
};
