#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCTrajectoryPreviewObject@@, vtable 00d19d90, 0x58 bytes (operator new em 006b1df0), habilidade 0x15.
// Guarda o typeid e o valor do item que liga a prévia da trajetória; quem liga é o controlador.
class CTrajectoryPreviewObject : public CControlledObject
{
public:
	CTrajectoryPreviewObject(eSpecialAbility ability); // 006b1d30
	virtual ~CTrajectoryPreviewObject(); // 006b1da0
	virtual void On01(const sControlledParam* pParam); // 006b1db0

	bool HasValue() const { return m_value > 0; } // cópia fora de linha em 006b1a50 (unidade do controlador)

private:
	unsigned long m_typeId; // +0x50
	unsigned long m_value; // +0x54
};
