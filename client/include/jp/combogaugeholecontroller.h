#pragma once
#include "controller.h"
#include "combogaugeobject.h"

// JP 983: RTTI .?AVCComboGaugeHoleController@@, vtable 00d01f60, 0x10 bytes (sem campo próprio). Slot 5.
class CComboGaugeHoleController : public IController
{
	DECLARE_OBJECT(CComboGaugeHoleController)
public:
	CComboGaugeHoleController(); // 004a55a0
	virtual ~CComboGaugeHoleController(); // 004a55d0
	CComboGaugeObject* GetControlled(); // 004a5620 (o CComboGaugeGameController usa a mesma função)
	virtual bool On05(); // 004a56f0
};

// JP 983: RTTI .?AVCComboGaugeGameController@@, vtable 00d01f9c, 0x10 bytes (sem campo próprio). Slot 4.
class CComboGaugeGameController : public IController
{
	DECLARE_OBJECT(CComboGaugeGameController)
public:
	CComboGaugeGameController(); // 004a55e0
	virtual ~CComboGaugeGameController(); // 004a5610
	virtual bool On04(); // 004a57d0
};
