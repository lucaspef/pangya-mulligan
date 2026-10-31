// JP 983: unidade 0056b220–0056b4b0 (CPutter10YardController). Classe nova do 983.
#include "putter10yardcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"

CPutter10YardController::CPutter10YardController()
{
}

IMPLEMENT_OBJECT(CPutter10YardController, IController)

CPutter10YardObject* CPutter10YardController::GetControlled()
{
	return (CPutter10YardObject*)m_pObject;
}

// 0056b330: pacote 0x181 com { 0x1e, typeid, typeid, 1 }
void CPutter10YardController::Send(CPutter10YardObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_Putter10Yard;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CPutter10YardController::On06(float)
{
	CPutter10YardObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 0056b400: o objeto existe, o jogador `index` é o meu e é o dono do item (o 983 passa o objeto em edi e o índice em ebx)
static bool IsMyPutter10Yard(CPutter10YardObject* pObject, int index)
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

bool CPutter10YardController::On05()
{
	CPutter10YardObject* pObject = GetControlled();
	if (IsMyPutter10Yard(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CPutter10YardController::On07(int arg)
{
	CPutter10YardObject* pObject = GetControlled();
	if (IsMyPutter10Yard(pObject, arg))
		Send(pObject);
	return true;
}
