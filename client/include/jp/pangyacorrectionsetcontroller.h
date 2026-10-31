#pragma once
#include "controller.h"
#include "pangyacorrectionsetobject.h"

// JP 983: RTTI .?AVCPangyaCorrectionSetController@@, 0x10 bytes (sem campo próprio).
class CPangyaCorrectionSetController : public IController
{
	DECLARE_OBJECT(CPangyaCorrectionSetController)
public:
	CPangyaCorrectionSetController() { } // 0056a0a0
	CPangyaCorrectionSetObject* GetControlled(); // 0056a1b0
	virtual bool On05(); // 0056a1c0
	virtual bool On07(int arg); // 0056a1e0
};
