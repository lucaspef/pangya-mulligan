#pragma once
#include "controller.h"
#include "automiraclesignobject.h"

// JP 983: RTTI .?AVCAutoMiracleSignController@@, vtable 00cfea98, 0x10 bytes (sem campo próprio).
class CAutoMiracleSignController : public IController
{
	DECLARE_OBJECT(CAutoMiracleSignController)
public:
	CAutoMiracleSignController(); // 0047db10 (fora de linha: o MakeInstance chama)
	CAutoMiracleSignObject* GetControlled(); // 0047db50
	bool Apply(unsigned char index); // 0047dde0
	virtual bool On07(int arg); // 0047de80 (salto para o Apply)
};
