#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCWindMinusObject@@, vtable 00d0a9c0, 0x5c bytes (operator new em 0056bdb0), habilidade 0x22.
// Liga/desliga o evento 0x80 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CWindMinusObject : public CControlledObject
{
public:
	CWindMinusObject(eSpecialAbility ability); // 0056bd20
	virtual ~CWindMinusObject(); // 0056bd90
	virtual void On01(const sControlledParam* pParam); // 0056bda0 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 0056bff0
	virtual void On07(int typeId); // 0056be50
	virtual void On10(int index); // 0056bf20

	void Reset(); // 0056be90
	unsigned char GetPlayerIndex() const { return m_playerIndex; }
	unsigned long GetTypeId() const { return m_typeId; }

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item; 0 e 100 = não mexe no evento)
	bool m_bOn; // +0x59 (campo gravado)
};
