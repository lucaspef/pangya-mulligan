#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCMiracleSignSkillObject@@, vtable 00d0a854, 0x60 bytes (operator new em 0056b0f0), habilidade 0x20.
class CMiracleSignSkillObject : public CControlledObject
{
public:
	CMiracleSignSkillObject(eSpecialAbility ability); // 0056aff0
	virtual ~CMiracleSignSkillObject(); // 0056b060
	virtual void On01(const sControlledParam* pParam); // 0056b070

	bool Roll(); // 0056b0c0
	const char* GetItemName(); // 0056b190 (nome do item comum de m_typeId, ou 0)
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 0056b200
	unsigned long GetTypeId() const { return m_typeId; } // 0056b210
	unsigned int GetPercent() const { return m_percent; } // sem cópia fora de linha (o controlador lê +0x58)

private:
	unsigned char m_playerIndex; // +0x50
	unsigned long m_typeId; // +0x54
	unsigned int m_percent; // +0x58
	bool m_bHit; // +0x5c
};
