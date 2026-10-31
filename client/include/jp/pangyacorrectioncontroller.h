#pragma once
#include "controller.h"
#include "pangyacorrectionobject.h"

// JP 983: RTTI .?AVCPangyaCorrectionController@@, 0x10 bytes (sem campo próprio).
class CPangyaCorrectionController : public IController
{
	DECLARE_OBJECT(CPangyaCorrectionController)
public:
	CPangyaCorrectionController() { } // 005ac150
	CPangyaCorrectionObject* GetControlled(); // 005ac260
	virtual bool On05(); // 005ac270
	virtual bool On07(int arg); // 005ac290
};
