#pragma once
#include "controller.h"
#include "windvolumeobject.h"

// JP 983: RTTI .?AVCWindVolumeController@@, vtable 00d548b4, 0x10 bytes (sem campo próprio).
class CWindVolumeController : public IController
{
	DECLARE_OBJECT(CWindVolumeController)
public:
	CWindVolumeController(); // 00b455b0 (fora de linha: o MakeInstance chama)
	CWindVolumeObject* GetControlled(); // 00b45620
	unsigned long Roll(CWindVolumeObject* pObject); // 00b458c0: typeid do item sorteado ou 0
	bool Apply(unsigned char index); // 00b45980
	virtual bool On05(); // 00b459f0
	virtual bool On06(float arg); // 00b45800
	virtual bool On07(int arg); // 00b45a10 (salto para o Apply)
};
