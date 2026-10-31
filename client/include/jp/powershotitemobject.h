#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCPowerShotItemObject@@, vtable 00d0e360, 0x5c bytes (operator new em 005b7c60), habilidade 0x17.
// Liga/desliga o evento 0x10 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CPowerShotItemObject : public CControlledObject
{
public:
	CPowerShotItemObject(eSpecialAbility ability); // 005b7bd0
	virtual ~CPowerShotItemObject(); // 005b7c40
	virtual void On01(const sControlledParam* pParam); // 005b7c50 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 005b7e90
	virtual void On07(int typeId); // 005b7d00
	virtual void On10(int index); // 005b7dd0

	void Reset(); // 005b7d50
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 005b7900
	unsigned long GetTypeId() const { return m_typeId; } // 005b7910
	unsigned char GetValue() const { return m_value; } // 005b7920

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item = chance em %; 0 e 100 = não mexe no evento e não sorteia)
	bool m_bOn; // +0x59 (campo gravado)
};
