#pragma once
#include "controller.h"
#include "plusyarditemobject.h"

// JP 983: RTTI .?AVCPlusYardItemController@@, 0x10 bytes (sem campo próprio).
class CPlusYardItemController : public IController
{
	DECLARE_OBJECT(CPlusYardItemController)
public:
	CPlusYardItemController() { } // 005e30a0
	CPlusYardItemObject* GetControlled(); // 005e31b0
	virtual bool On05(); // 005e31c0
	virtual bool On07(int arg); // 005e31e0
};
