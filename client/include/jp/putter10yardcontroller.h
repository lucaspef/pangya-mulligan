#pragma once
#include "controller.h"
#include "putter10yardobject.h"

// JP 983: RTTI .?AVCPutter10YardController@@, vtable 00d0a888, 0x10 bytes (sem campo próprio).
class CPutter10YardController : public IController
{
	DECLARE_OBJECT(CPutter10YardController)
public:
	CPutter10YardController(); // 0056b220 (fora de linha: o MakeInstance chama)
	CPutter10YardObject* GetControlled(); // 0056b260
	void Send(CPutter10YardObject* pObject); // 0056b330
	virtual bool On05(); // 0056b440
	virtual bool On06(float arg); // 0056b3c0
	virtual bool On07(int arg); // 0056b480
};
