#pragma once
#include "controller.h"
#include "clubsetexpmultiplyobject.h"

// JP 983: RTTI .?AVCClubsetExpMultiplyController@@, vtable 00d0cde0, 0x10 bytes (sem campo próprio).
class CClubsetExpMultiplyController : public IController
{
	DECLARE_OBJECT(CClubsetExpMultiplyController)
public:
	CClubsetExpMultiplyController() { } // 0056a440 (embutido: o MakeInstance grava a vtable direto)
	CClubsetExpMultiplyObject* GetControlled(); // 0056a560
	void Send(CClubsetExpMultiplyObject* pObject); // 0056a580
	virtual bool On05(); // 0056a720
	virtual bool On06(float arg); // 0056a610
	virtual bool On07(int arg); // 0056a760
};
