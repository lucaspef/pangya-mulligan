#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPutter10YardObject@@, vtable 00d0a8c4, 0x5c bytes (operator new em 0056b540), habilidade 0x1e.
// Liga/desliga o evento 0x40 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CPutter10YardObject : public CControlledObject
{
public:
	CPutter10YardObject(eSpecialAbility ability); // 0056b4b0
	virtual ~CPutter10YardObject(); // 0056b520
	virtual void On01(const sControlledParam* pParam); // 0056b530 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 0056b710
	virtual void On07(int typeId); // 0056b5e0
	virtual void On10(int index); // 0056b650

	void Reset(); // 0056b620
	unsigned char GetPlayerIndex() const { return m_playerIndex; }
	unsigned long GetTypeId() const { return m_typeId; }

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item; 0 e 100 = não mexe no evento)
	bool m_bOn; // +0x59 (campo gravado)
};
