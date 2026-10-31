#pragma once
#include "controller.h"
#include "seteffectobject.h"

// JP 983: RTTI .?AVCSetEffectController@@, vtable 00d15dc0, 0x10 bytes (sem campo próprio).
class CSetEffectController : public IController
{
	DECLARE_OBJECT(CSetEffectController)
public:
	CSetEffectController() { } // embutido no MakeInstance (00627680); a cópia fora de linha 00627610 não é chamada
	CSetEffectObject* GetControlled(); // 00627720
	virtual bool On02(); // 00627790
	virtual bool On03(); // 00627730
	virtual bool On04(); // 00627750
	virtual bool On11(); // 00627770
};
