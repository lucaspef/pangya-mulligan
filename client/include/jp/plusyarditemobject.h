#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPlusYardItemObject@@, 0x60 bytes (operator new em 005e3340), habilidade 0x19.
class CPlusYardItemObject : public CControlledObject
{
public:
	CPlusYardItemObject(eSpecialAbility ability); // 005e3210
	virtual ~CPlusYardItemObject(); // 005e3280
	virtual void On01(const sControlledParam* pParam); // 005e3290

	bool Roll(); // 005e32e0
	bool IsHit() const; // 005e3310
	bool IsReady() const; // 005e3320

private:
	sControlledSource* m_pSource; // +0x50
	int m_54; // +0x54
	int m_percent; // +0x58
	bool m_bHit; // +0x5c
	unsigned char m_bReady; // +0x5d (não é bool: IsReady compara com 0 e usa setne)
};
