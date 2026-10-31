// JP 983: unidade 00b45590–00b45a20 (CWindVolumeController). Classe nova do 983. Começa pelas cópias fora de linha de dois
// getters do objeto (00b45590, 00b455a0) e guarda as do iterador do vector (00b455f0…00b45610, 00b456f0…00b45730, 00b45840…00b458b0).
#include "windvolumecontroller.h"
#include "miraclesignskillcontroller.h"
#include "groundinclinationcontroller.h"
#include "shareddocjp.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"

CWindVolumeController::CWindVolumeController()
{
}

IMPLEMENT_OBJECT(CWindVolumeController, IController)

CWindVolumeObject* CWindVolumeController::GetControlled()
{
	return (CWindVolumeObject*)m_pObject;
}

// 00b45740: pacote 0x180 com o typeid do item sorteado + mensagem 1 à tarefa
void __fastcall SendWindVolume(unsigned long typeId)
{
	WSendPacket packet((enumClientPacket)0x180);
	packet.Encode4(typeId);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

bool CWindVolumeController::On06(float)
{
	CWindVolumeObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(0);
	}
	return true;
}

// 00b458c0: fora do green (tipo de chão 2), sorteia cada item (1 a 100 < chance); no mapa 0x14 não vale nos buracos cujo
// valor de 00633560 é 5 ou 0x0c
unsigned long CWindVolumeController::Roll(CWindVolumeObject* pObject)
{
	if (pObject == NULL)
		return 0;
	unsigned char index = pObject->GetPlayerIndex();
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
	if (pPlayer == NULL)
		return 0;
	if (GetPVS().GetGroundType(pPlayer->pos, true, NULL) == 2)
		return 0;
	CWindVolumeObject::ItemList& items = pObject->GetItems();
	for (CWindVolumeObject::ItemList::iterator it = items.begin(); it != items.end(); ++it)
	{
		if (RandInt983(99) + 1 >= it->second)
			continue;
		if (GetCurMap() == 0x14)
		{
			unsigned char hole = DOCJP()->GetHoleValue983(GetHoleIndex(GOLFDOC()->m_currentHole));
			if (hole == 5 || hole == 0xc)
				continue;
		}
		return it->first;
	}
	return 0;
}

bool CWindVolumeController::Apply(unsigned char index)
{
	if (!IsControlledPlayerMine(index))
		return false;
	CWindVolumeObject* pObject = GetControlled();
	if (pObject == NULL || pObject->GetPlayerIndex() != index)
		return false;
	unsigned char owner = pObject->GetPlayerIndex();
	if (GOLFDOC()->GetPlayer(owner) == NULL)
		return false;
	unsigned long typeId = Roll(pObject);
	if (typeId == 0)
		return false;
	SendWindVolume(typeId);
	return true;
}

bool CWindVolumeController::On05()
{
	return Apply(GOLFDOC()->m_currentPlayer);
}

bool CWindVolumeController::On07(int arg)
{
	return Apply(arg);
}
