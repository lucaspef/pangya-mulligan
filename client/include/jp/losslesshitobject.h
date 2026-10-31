#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCLosslessHitObject@@, vtable 00d0ce1c, 0x5c bytes (operator new em 005a0d00), habilidade 0x12.
// Liga/desliga o evento 8 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CLosslessHitObject : public CControlledObject
{
public:
	CLosslessHitObject(eSpecialAbility ability); // 005a0c70
	virtual ~CLosslessHitObject(); // 005a0ce0
	virtual void On01(const sControlledParam* pParam); // 005a0cf0 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 005a10f0
	virtual void On07(int typeId); // 005a0e90
	virtual void On10(int index); // 005a0e20

	void Reset(); // 005a0da0
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 005a0990
	unsigned long GetTypeId() const { return m_typeId; } // 005a09a0
	unsigned char GetValue() const { return m_value; } // 005a09b0

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item = chance em %; 0 e 100 = não mexe no evento e não sorteia)
	bool m_bOn; // +0x59 (campo gravado)
};
