#pragma once
#include "controller.h"
#include "groundinclinationobject.h"

// JP 983: RTTI .?AVCGroundInclinationController@@, vtable 00d0a328, 0x10 bytes (sem campo próprio).
class CGroundInclinationController : public IController
{
	DECLARE_OBJECT(CGroundInclinationController)
public:
	CGroundInclinationController() { } // 0055ee70 (embutido: o MakeInstance grava a vtable direto)
	CGroundInclinationObject* GetControlled(); // 0055efc0
	virtual bool On05(); // 0055f250
	virtual bool On06(float arg); // 0055f020
	virtual bool On07(int arg); // 0055f270
};

unsigned long __fastcall RandInt983(unsigned long n); // 00860180 (util.cpp do 983: MTRand1p1 estático, randInt(n); nome meu)
