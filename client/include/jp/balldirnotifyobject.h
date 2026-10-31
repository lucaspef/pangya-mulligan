#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCBallDirNotifyObject@@, vtable 00cfee80, 0x58 bytes (operator new em 004843e0), habilidade 0x14.
// Guarda o typeid do item que mostra a direção da bola; o controlador gasta uma unidade do item por tacada.
class CBallDirNotifyObject : public CControlledObject
{
public:
	CBallDirNotifyObject(eSpecialAbility ability); // 00484330
	virtual ~CBallDirNotifyObject(); // 004843a0
	virtual void On01(const sControlledParam* pParam); // 004843b0
	virtual void On05(); // 004843d0

	unsigned long GetTypeId() const { return m_typeId; }

private:
	unsigned long m_typeId; // +0x50
	bool m_54; // +0x54 (zerado no construtor e no slot 5)
};
