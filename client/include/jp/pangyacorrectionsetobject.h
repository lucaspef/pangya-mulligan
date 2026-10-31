#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPangyaCorrectionSetObject@@, 0x60 bytes (operator new em 0056a350), habilidade 0x23.
class CPangyaCorrectionSetObject : public CControlledObject
{
public:
	CPangyaCorrectionSetObject(eSpecialAbility ability); // 0056a210
	virtual ~CPangyaCorrectionSetObject(); // 0056a280
	virtual void On01(const sControlledParam* pParam); // 0056a290

	bool Roll(); // 0056a2e0
	bool IsHit(); // 0056a310
	bool IsReady() const; // 0056a330

private:
	sControlledSource* m_pSource; // +0x50
	int m_54; // +0x54
	int m_percent; // +0x58
	bool m_bHit; // +0x5c
	unsigned char m_bReady; // +0x5d (não é bool: IsReady compara com 0 e usa setne)
};
