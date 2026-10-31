#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPangyaAddCorrectionObj@@, 0x60 bytes (operator new em 005ac060), habilidade 0x16.
class CPangyaAddCorrectionObj : public CControlledObject
{
public:
	CPangyaAddCorrectionObj(eSpecialAbility ability); // 005abf30
	virtual ~CPangyaAddCorrectionObj(); // 005abfa0
	virtual void On01(const sControlledParam* pParam); // 005abfb0

	bool Roll(); // 005ac000
	bool IsHit() const; // 005ac030
	bool IsReady() const; // 005ac040

private:
	sControlledSource* m_pSource; // +0x50
	int m_54; // +0x54
	int m_percent; // +0x58
	bool m_bHit; // +0x5c
	unsigned char m_bReady; // +0x5d (não é bool: IsReady compara com 0 e usa setne)
};
