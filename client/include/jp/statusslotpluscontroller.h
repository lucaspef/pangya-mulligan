#pragma once
#include "controller.h"

// JP 983: RTTI .?AVCStatusSlotPlusController@@, vtable 00d0a8f8, 0x10 bytes (sem campo próprio).
class CStatusSlotPlusController : public IController
{
	DECLARE_OBJECT(CStatusSlotPlusController)
public:
	CStatusSlotPlusController() { } // 0056b790 (cópia fora de linha; embutido no MakeInstance)
	virtual bool On05() { return true; } // 0056b880
	virtual bool On07(int) { return true; } // 0056b890
	virtual bool On06(float) { return true; } // 0056b8a0
};
