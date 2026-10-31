#pragma once
#include "controller.h"
#include "combogaugeeveryshotobject.h"

// JP 983: RTTI .?AVCComboGaugeEveryshotController@@, 0x10 bytes (sem campo próprio).
class CComboGaugeEveryshotController : public IController
{
	DECLARE_OBJECT(CComboGaugeEveryshotController)
public:
	CComboGaugeEveryshotController() { } // 004a57e0
	CComboGaugeEveryshotObject* GetControlled(); // 004a58f0
	virtual bool On05(); // 004a5900
	virtual bool On07(int arg); // 004a5940
};
