#pragma once
#include "controller.h"
#include "losslesshitweatherobject.h"

// JP 983: RTTI .?AVCLosslessHitWeatherController@@, vtable 00d0a698, 0x10 bytes (sem campo próprio).
class CLosslessHitWeatherController : public IController
{
	DECLARE_OBJECT(CLosslessHitWeatherController)
public:
	CLosslessHitWeatherController() { } // 005699e0 (embutido: o MakeInstance grava a vtable direto)
	CLosslessHitWeatherObject* GetControlled(); // 00569af0
	void Send(CLosslessHitWeatherObject* pObject); // 00569b00
	virtual bool On05(); // 00569c70
	virtual bool On06(float arg); // 00569b90
	virtual bool On07(int arg); // 00569cb0
};
