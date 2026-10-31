#pragma once
#include "controller.h"
#include "losslesshitobject.h"

// JP 983: RTTI .?AVCLosslessHitController@@, vtable 00d0cde0, 0x10 bytes (sem campo próprio).
class CLosslessHitController : public IController
{
	DECLARE_OBJECT(CLosslessHitController)
public:
	CLosslessHitController() { } // 005a0920 (embutido: o MakeInstance grava a vtable direto)
	CLosslessHitObject* GetControlled(); // 005a0a40
	void Send(CLosslessHitObject* pObject); // 005a0a60
	virtual bool On05(); // 005a0c00
	virtual bool On06(float arg); // 005a0af0
	virtual bool On07(int arg); // 005a0c40
};
