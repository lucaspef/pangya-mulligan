// JP 983: unidade 005b7890–005b7bd0 (CPowerShotItemController). Classe nova do 983.
// Não escrita: 005b79c0 (`push eax; call [00cfc7d0]; ret`, 11 bytes, sem nenhuma chamada no exe).
#include "powershotitemcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"
#include "quadtree.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CPowerShotItemController, IController)

CPowerShotItemObject* CPowerShotItemController::GetControlled()
{
	return (CPowerShotItemObject*)m_pObject;
}

// 005b79d0: pacote 0x197 com { 0x17, typeid, typeid, 1 }
void CPowerShotItemController::Send(CPowerShotItemObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_PowerShotItem;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x197);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CPowerShotItemController::On06(float)
{
	CPowerShotItemObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 005b7aa0: o objeto existe, o jogador `index` é o meu e é o dono do item, a bola não está no terreno 2 e o sorteio
// (1 a 100 <= valor do item; gerador próprio da unidade em 00e77738) passou — aí marca o m_28 do objeto
// (o 983 passa o objeto em edi e o índice em esi)
static bool IsMyPowerShotItem(CPowerShotItemObject* pObject, int index)
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
	if (GetPVS().GetGroundType(GOLFDOC()->GetPlayer(index)->pos, true, NULL) == 2)
		return false;
	if (pObject->GetValue() == 0 || pObject->GetValue() == 100)
		return true;
	static MTRand1p1 s_random;
	int roll = s_random.randInt(99) + 1;
	if (roll > pObject->GetValue())
		return false;
	pObject->Set28(true);
	return true;
}

bool CPowerShotItemController::On05()
{
	CPowerShotItemObject* pObject = GetControlled();
	if (IsMyPowerShotItem(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CPowerShotItemController::On07(int arg)
{
	CPowerShotItemObject* pObject = GetControlled();
	if (IsMyPowerShotItem(pObject, arg))
		Send(pObject);
	return true;
}
