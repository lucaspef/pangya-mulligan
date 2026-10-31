#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCSafetyItemObject@@, vtable 00d12338, 0x5c bytes (operator new em 005e0840), habilidade 0x18.
// Liga/desliga o evento 2 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CSafetyItemObject : public CControlledObject
{
public:
	CSafetyItemObject(eSpecialAbility ability); // 005e07a0
	virtual ~CSafetyItemObject(); // 005e0810
	virtual void On01(const sControlledParam* pParam); // 005e0820 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 005e0c20
	virtual void On07(int typeId); // 005e09d0
	virtual void On08(int arg); // 005e0830 (vazio, mas próprio)
	virtual void On10(int index); // 005e0960

	void Reset(); // 005e08e0
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 005e0490
	unsigned long GetTypeId() const { return m_typeId; } // 005e04a0
	unsigned char GetValue() const { return m_value; } // 005e04b0

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item = chance em %; 0 e 100 = não mexe no evento e não sorteia)
	bool m_bOn; // +0x59 (campo gravado)
};
