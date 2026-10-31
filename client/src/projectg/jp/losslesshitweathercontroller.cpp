// JP 983: unidade 005699e0–00569ce0 (CLosslessHitWeatherController). Classe nova do 983.
#include "losslesshitweathercontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"
#include "quadtree.h"

IMPLEMENT_OBJECT(CLosslessHitWeatherController, IController)

CLosslessHitWeatherObject* CLosslessHitWeatherController::GetControlled()
{
	return (CLosslessHitWeatherObject*)m_pObject;
}

// 00569b00: pacote 0x181 com { 0x1d, typeid, typeid, 1 }
void CLosslessHitWeatherController::Send(CLosslessHitWeatherObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_LosslessHitWeather;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CLosslessHitWeatherController::On06(float)
{
	CLosslessHitWeatherObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 00569bd0: o objeto existe, o jogador `index` é o meu e é o dono do item, a bola não está no terreno 2 e o tempo é 2 ou 3
// (o 983 passa o objeto em ebx e o índice em esi)
static bool IsMyLosslessHitWeather(CLosslessHitWeatherObject* pObject, int index)
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
	int weather = Doc()->m_golfGame.weather;
	if (weather == 2 || weather == 3)
		return true;
	return false;
}

bool CLosslessHitWeatherController::On05()
{
	CLosslessHitWeatherObject* pObject = GetControlled();
	if (IsMyLosslessHitWeather(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CLosslessHitWeatherController::On07(int arg)
{
	CLosslessHitWeatherObject* pObject = GetControlled();
	if (IsMyLosslessHitWeather(pObject, arg))
		Send(pObject);
	return true;
}
