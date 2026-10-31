// JP 983: unidade 005e0420–005e07a0 (CSafetyItemController). Classe nova do 983.
// Não escrita: 005e0550 (`push eax; call [00cfc7d0]; ret`, 11 bytes, sem nenhuma chamada no exe).
#include "safetyitemcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"
#include "quadtree.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CSafetyItemController, IController)

CSafetyItemObject* CSafetyItemController::GetControlled()
{
	return (CSafetyItemObject*)m_pObject;
}

bool CSafetyItemController::On06(float)
{
	CSafetyItemObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 005e05a0: o objeto existe, o jogador `index` é o meu e é o dono do item, a bola não está no terreno 0 nem no 2 e o sorteio
// (1 a 100 <= valor do item; gerador próprio da unidade em 00e78278) passou (o 983 passa o objeto em ebx e o índice em esi)
static bool IsMySafetyItem(CSafetyItemObject* pObject, int index)
{
	if (pObject == NULL)
		return false;
	pObject->Reset();
	unsigned long uid = Doc()->m_myInfo.info.dwGuid;
	if (uid != GOLFDOC()->GetPlayer(index)->oid) // 983 +0x4b4 (oid = dwGuid, nao uid)
		return false;
	int owner = pObject->GetPlayerIndex();
	if (index != owner)
		return false;
	unsigned char ground = GetPVS().GetGroundType(GOLFDOC()->GetPlayer(index)->pos, true, NULL);
	if (ground == 2 || ground == 0)
		return false;
	if (pObject->GetValue() == 0 || pObject->GetValue() == 100)
		return true;
	static MTRand1p1 s_random;
	int roll = s_random.randInt(99) + 1;
	return roll <= pObject->GetValue();
}

// 005e0670: pacote 0x181 com { 0x18, typeid, typeid, uid do jogador da vez }
void CSafetyItemController::Send(CSafetyItemObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		unsigned long uid;
	} data;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	data.uid = 1;
	data.ability = eSpecialAbility_SafetyItem;
	unsigned char current = GOLFDOC()->m_currentPlayer;
	data.uid = GOLFDOC()->GetPlayer(current)->oid; // 983 005e06cd: +0x4b4
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CSafetyItemController::On05()
{
	CSafetyItemObject* pObject = GetControlled();
	if (IsMySafetyItem(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CSafetyItemController::On07(int arg)
{
	CSafetyItemObject* pObject = GetControlled();
	if (IsMySafetyItem(pObject, arg))
		Send(pObject);
	return true;
}
