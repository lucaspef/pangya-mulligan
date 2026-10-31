// JP 983: unidade 005a0920–005a0c70 (CLosslessHitController). Classe nova do 983.
// Não escrita: 005a0a50 (`push eax; call [00cfc7d0]; ret`, 11 bytes, sem nenhuma chamada no exe).
#include "losslesshitcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"
#include "quadtree.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CLosslessHitController, IController)

CLosslessHitObject* CLosslessHitController::GetControlled()
{
	return (CLosslessHitObject*)m_pObject;
}

// 005a0a60: pacote 0x181 com { 0x12, typeid, typeid, 1 }
void CLosslessHitController::Send(CLosslessHitObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_LosslessHit;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CLosslessHitController::On06(float)
{
	CLosslessHitObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 005a0b30: o objeto existe, o jogador `index` é o meu e é o dono do item, a bola não está no terreno 0 nem no 2 e o sorteio
// (1 a 100 <= valor do item; gerador próprio da unidade em 00e75f20) passou (o 983 passa o objeto em ebx e o índice em esi)
static bool IsMyLosslessHit(CLosslessHitObject* pObject, int index)
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

bool CLosslessHitController::On05()
{
	CLosslessHitObject* pObject = GetControlled();
	if (IsMyLosslessHit(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CLosslessHitController::On07(int arg)
{
	CLosslessHitObject* pObject = GetControlled();
	if (IsMyLosslessHit(pObject, arg))
		Send(pObject);
	return true;
}
