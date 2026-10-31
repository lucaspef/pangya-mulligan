#pragma once
#include "controller.h"
#include "safetyitemobject.h"

// JP 983: RTTI .?AVCSafetyItemController@@, vtable 00d122fc, 0x10 bytes (sem campo próprio).
class CSafetyItemController : public IController
{
	DECLARE_OBJECT(CSafetyItemController)
public:
	CSafetyItemController() { } // 005e0420 (embutido: o MakeInstance grava a vtable direto)
	CSafetyItemObject* GetControlled(); // 005e0540
	void Send(CSafetyItemObject* pObject); // 005e0670
	virtual bool On05(); // 005e0730
	virtual bool On06(float arg); // 005e0560
	virtual bool On07(int arg); // 005e0770
};
