#pragma once
#include "controlledobject.h"
#include <utility>

// JP 983: RTTI .?AVCWindVolumeObject@@, vtable 00d54928, 0x68 bytes (operator new em 00b46420), habilidade 0x10.
// Item que põe a força do vento em 1: marca sPlayerData::item do jogador da vez e avisa o ator "Player" (mensagem 0x31b).
class CWindVolumeObject : public CControlledObject
{
public:
	typedef std::vector<std::pair<unsigned long, unsigned long> > ItemList; // { typeid do item, valor (chance em %) }

	CWindVolumeObject(eSpecialAbility ability); // 00b462b0
	virtual ~CWindVolumeObject(); // 00b46320
	virtual void On01(const sControlledParam* pParam); // 00b46510
	virtual void On05(); // 00b45d80
	virtual void On06(int); // 00b45e10
	virtual void On07(int); // 00b45f20
	virtual void On08(int typeId); // 00b45a30

	unsigned char GetPlayerIndex() const { return (unsigned char)m_playerIndex; } // cópia fora de linha em 00b45590
	ItemList& GetItems() { return m_items; } // 00b455a0

private:
	int m_playerIndex; // +0x50 (sControlledParam::m_00 inteiro)
	unsigned long m_activeTypeId; // +0x54 (item sorteado; 0 = nenhum)
	ItemList m_items; // +0x58
};
