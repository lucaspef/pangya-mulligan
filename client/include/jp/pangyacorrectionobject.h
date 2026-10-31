#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPangyaCorrectionObject@@, 0x60 bytes (operator new em 005ac3f0), habilidade 0x0a.
class CPangyaCorrectionObject : public CControlledObject
{
public:
	CPangyaCorrectionObject(eSpecialAbility ability); // 005ac2c0
	virtual ~CPangyaCorrectionObject(); // 005ac330
	virtual void On01(const sControlledParam* pParam); // 005ac340

	bool Roll(); // 005ac390
	bool IsHit() const; // 005ac3c0
	bool IsReady() const; // 005ac3d0

private:
	sControlledSource* m_pSource; // +0x50
	int m_54; // +0x54
	int m_percent; // +0x58
	bool m_bHit; // +0x5c
	unsigned char m_bReady; // +0x5d (não é bool: IsReady compara com 0 e usa setne)
};
