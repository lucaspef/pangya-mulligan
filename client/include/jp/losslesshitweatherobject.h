#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCLosslessHitWeatherObject@@, vtable 00d0a6d4, 0x5c bytes (operator new em 00569d70), habilidade 0x1d.
// Liga/desliga o evento 0x20 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CLosslessHitWeatherObject : public CControlledObject
{
public:
	CLosslessHitWeatherObject(eSpecialAbility ability); // 00569ce0
	virtual ~CLosslessHitWeatherObject(); // 00569d50
	virtual void On01(const sControlledParam* pParam); // 00569d60 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 0056a020
	virtual void On07(int typeId); // 00569e10
	virtual void On10(int index); // 00569f30

	void Reset(); // 00569f00
	unsigned char GetPlayerIndex() const { return m_playerIndex; }
	unsigned long GetTypeId() const { return m_typeId; }

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item; 0 e 100 = não mexe no evento)
	bool m_bOn; // +0x59 (campo gravado)
};
