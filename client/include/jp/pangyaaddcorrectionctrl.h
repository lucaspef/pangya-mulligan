#pragma once
#include "controller.h"
#include "pangyaaddcorrectionobj.h"

// JP 983: RTTI .?AVCPangyaAddCorrectionCtrl@@, 0x10 bytes (sem campo próprio).
class CPangyaAddCorrectionCtrl : public IController
{
	DECLARE_OBJECT(CPangyaAddCorrectionCtrl)
public:
	CPangyaAddCorrectionCtrl() { } // 005abdc0
	CPangyaAddCorrectionObj* GetControlled(); // 005abed0
	virtual bool On05(); // 005abee0
	virtual bool On07(int arg); // 005abf00
};
