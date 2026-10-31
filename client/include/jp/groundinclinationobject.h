#pragma once
#include "controlledobject.h"
#include <utility>

// JP 983: RTTI .?AVCGroundInclinationObject@@, vtable 00d0a364, 0x68 bytes (operator new em 0055fca0), habilidade 0x0e.
// Item que mostra a inclinação do chão: liga o evento 4 do jogador e desliga os eventos 4|2 de todos ao zerar.
class CGroundInclinationObject : public CControlledObject
{
public:
	typedef std::vector<std::pair<unsigned long, unsigned long> > ItemList; // { typeid do item, valor }

	CGroundInclinationObject(eSpecialAbility ability); // 0055fb20
	virtual ~CGroundInclinationObject(); // 0055fba0
	virtual void On01(const sControlledParam* pParam); // 0055fdc0
	virtual void On05(); // 0055f680 (salto para o Reset)
	virtual void On06(int); // 0055f690
	virtual void On07(int typeId); // 0055f790
	virtual void On08(int typeId); // 0055f2a0
	virtual void On10(int index); // 0055f6a0

	void Reset(); // 0055f4f0
	ItemList& GetItems() { return m_items; } // cópia fora de linha em 0055eee0 (unidade do controlador)
	unsigned char GetPlayerIndex() const { return m_playerIndex; } // 0055eef0
	unsigned long GetActiveTypeId() const { return m_activeTypeId; } // 0055ef00

private:
	unsigned long m_typeId; // +0x50
	unsigned char m_playerIndex; // +0x54
	unsigned long m_activeTypeId; // +0x58 (campo gravado; 0 = desligado)
	ItemList m_items; // +0x5c
};
