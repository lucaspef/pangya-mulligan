#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCClubsetExpMultiplyObject@@, vtable 00d0ce1c, 0x5c bytes (operator new em 0056a820), habilidade 0x1f.
// Liga/desliga o evento 8 do jogador (sPlayerData::event) e guarda o estado no gravador da base.
class CClubsetExpMultiplyObject : public CControlledObject
{
public:
	CClubsetExpMultiplyObject(eSpecialAbility ability); // 0056a790
	virtual ~CClubsetExpMultiplyObject(); // 0056a800
	virtual void On01(const sControlledParam* pParam); // 0056a810 (repassa ao slot 3)
	virtual void On03(const sControlledParam* pParam); // 0056ac10
	virtual void On07(int typeId); // 0056a9b0
	virtual void On10(int index); // 0056a940

	void Reset(); // 0056a8c0
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 0056a4b0
	unsigned long GetTypeId() const { return m_typeId; } // 0056a4c0
	unsigned char GetValue() const { return m_value; } // 0056a4d0

private:
	unsigned char m_playerIndex; // +0x50 (0xff no construtor)
	unsigned long m_typeId; // +0x54
	unsigned char m_value; // +0x58 (valor do item = chance em %; 0 e 100 = não mexe no evento e não sorteia)
	bool m_bOn; // +0x59 (campo gravado)
};
