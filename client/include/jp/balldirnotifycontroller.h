#pragma once
#include "controller.h"
#include "balldirnotifyobject.h"

// JP 983: RTTI .?AVCBallDirNotifyController@@, vtable 00cfee04, 0x10 bytes (sem campo próprio).
class CBallDirNotifyController : public IController
{
	DECLARE_OBJECT(CBallDirNotifyController)
public:
	CBallDirNotifyController(); // 00484010 (fora de linha: o MakeInstance chama)
	CBallDirNotifyObject* GetControlled(); // 00484050
	bool Apply(); // 00484240
	virtual bool On05(); // 00484320 (salto para o Apply)
};
