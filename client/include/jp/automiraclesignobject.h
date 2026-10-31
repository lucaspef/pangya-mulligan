#pragma once
#include "controlledobject.h"

// JP 983: RTTI .?AVCAutoMiracleSignObject@@, vtable 00cfeaf0, 0x5c bytes (operator new em 0047e020), habilidade 0x13.
// Só guarda o dono, o typeid do item e o valor; quem age é o CAutoMiracleSignController.
class CAutoMiracleSignObject : public CControlledObject
{
public:
	CAutoMiracleSignObject(eSpecialAbility ability); // 0047de90
	virtual ~CAutoMiracleSignObject(); // 0047df00
	virtual void On01(const sControlledParam* pParam); // 0047df10

	unsigned char GetPlayerIndex() const { return m_playerIndex; }
	unsigned long GetTypeId() const { return m_typeId; }
	unsigned long GetValue() const { return m_value; }

private:
	unsigned char m_playerIndex; // +0x50
	unsigned long m_typeId; // +0x54 (typeid do item)
	unsigned long m_value; // +0x58 (valor do item; o controlador só confere se é maior que 0)
};
