#pragma once
#include "controller.h"
#include "windminusobject.h"

// JP 983: RTTI .?AVCWindMinusController@@, vtable 00d0a984, 0x10 bytes (sem campo próprio).
class CWindMinusController : public IController
{
	DECLARE_OBJECT(CWindMinusController)
public:
	CWindMinusController() { } // 0056ba80 (embutido: o MakeInstance grava a vtable direto)
	CWindMinusObject* GetControlled(); // 0056bb90
	void Send(CWindMinusObject* pObject); // 0056bba0
	virtual bool On05(); // 0056bcb0
	virtual bool On06(float arg); // 0056bc30
	virtual bool On07(int arg); // 0056bcf0
};
