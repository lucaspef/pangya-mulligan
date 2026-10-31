// JP 983: unidade 0056ba80–0056bd20 (CWindMinusController). Classe nova do 983.
#include "windminuscontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"

IMPLEMENT_OBJECT(CWindMinusController, IController)

CWindMinusObject* CWindMinusController::GetControlled()
{
	return (CWindMinusObject*)m_pObject;
}

// 0056bba0: pacote 0x181 com { 0x22, typeid, typeid, 1 }
void CWindMinusController::Send(CWindMinusObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_WindMinus;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CWindMinusController::On06(float)
{
	CWindMinusObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 0056bc70: o objeto existe, o jogador `index` é o meu e é o dono do item (o 983 passa o objeto em edi e o índice em ebx)
static bool IsMyWindMinus(CWindMinusObject* pObject, int index)
{
	if (pObject == NULL)
		return false;
	pObject->Reset();
	unsigned long uid = Doc()->m_myInfo.info.dwGuid;
	if (uid != GOLFDOC()->GetPlayer(index)->oid) // 983 +0x4b4 (oid = dwGuid, nao uid)
		return false;
	int owner = pObject->GetPlayerIndex();
	return index == owner;
}

bool CWindMinusController::On05()
{
	CWindMinusObject* pObject = GetControlled();
	if (IsMyWindMinus(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CWindMinusController::On07(int arg)
{
	CWindMinusObject* pObject = GetControlled();
	if (IsMyWindMinus(pObject, arg))
		Send(pObject);
	return true;
}
