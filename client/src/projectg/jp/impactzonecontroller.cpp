// JP 983: unidade 00580330–005819b0 (nove controladores da zona de impacto). Classes novas do 983. Começa pelo construtor,
// GetRTTI e destrutores das nove (00580330–00580710) e pelas cópias fora de linha de SetZone/AddZone do objeto (00580720, 00580730).
#include "impactzonecontroller.h"
#include "classicwind.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "club.h"
#include "quadtree.h"
#include "chatmsg.h"
#include "commonutil.h"

#include "engine/wlocalize.h" // K2L_Compatibility 00b4ec60

// JP 983: sPlayerData+0x4bc = item.
// Diferente de zero = a zona fica guardada/zerada na tacada. Significado não levantado.
static inline unsigned long PlayerField4bc983(const sPlayerData* pPlayer)
{
	return pPlayer->item;
}

bool IsWindZoneItem983(unsigned long typeId)
{
	switch (typeId & 0xfffff)
	{
	case 6:
	case 32:
	case 44:
	case 45:
	case 47:
		return true;
	}
	return false;
}

IMPLEMENT_OBJECT(CWindImpactZoneElfEarController, IController)

bool CWindImpactZoneElfEarController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneElfEarController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(CWindImpactZoneElfEarImitationController, IController)

bool CWindImpactZoneElfEarImitationController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneElfEarImitationController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(CWindImpactZoneHighElfEarController, IController)

bool CWindImpactZoneHighElfEarController::On06(float arg)
{
	return IController::On06(arg) ? true : false;
}

bool CWindImpactZoneHighElfEarController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneHighElfEarController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(COwnerItemImpactZoneController, IController)

IMPLEMENT_OBJECT(CWindImpactZoneAuxPartController, IController)

bool CWindImpactZoneAuxPartController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneAuxPartController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(CProbability5ImpactZoneController, IController)

IMPLEMENT_OBJECT(CWindImpactZoneUndineController, IController)

bool CWindImpactZoneUndineController::On06(float arg)
{
	return IController::On06(arg) ? true : false;
}

bool CWindImpactZoneUndineController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneUndineController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(CWindImpactZoneMascotController, IController)

bool CWindImpactZoneMascotController::On06(float arg)
{
	return IController::On06(arg) ? true : false;
}

bool CWindImpactZoneMascotController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWindImpactZoneMascotController::On13()
{
	return On05();
}

IMPLEMENT_OBJECT(CWeatherImpactZoneController, IController)

bool CWeatherImpactZoneController::On06(float arg)
{
	return IController::On06(arg) ? true : false;
}

bool CWeatherImpactZoneController::On10(int typeId)
{
	if (!IsWindZoneItem983(typeId))
		return true;
	return On05();
}

bool CWeatherImpactZoneController::Refresh983()
{
	return On05();
}

int GetWindLevel983(float wind)
{
	int level = 3;
	if ((wind >= 1.0f || Wabs(wind - 1.0) < g_EPSILON) && wind < 4.0f && !(Wabs(wind - 4.0) < g_EPSILON))
		level = 0;
	else if ((wind >= 4.0f || Wabs(wind - 4.0) < g_EPSILON) && !(wind > 7.0) && !(Wabs(wind - 7.0) < g_EPSILON))
		level = 1;
	else if ((wind > 7.0 || Wabs(wind - 7.0) < g_EPSILON) && (wind <= 9.0f || Wabs(wind - 9.0) < g_EPSILON))
		level = 2;
	return level;
}

// 00581040: orelha de elfo — vento 4 a 7 = 1, 7 a 9 = 2
bool ApplyElfEar(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->ResetSaved();
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (pObject->GetItemTypeId(0) == 0)
			return false;
		switch (GetWindLevel983(Wind().GetPower983()))
		{
		case 1:
			pObject->SetZone(1.0f);
			break;
		case 2:
			pObject->SetZone(2.0f);
			break;
		}
	}
	return true;
}

// 005810a0: imitação — só o vento 7 a 9 = 1
bool ApplyElfEarImitation(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->ResetSaved();
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (GetWindLevel983(Wind().GetPower983()) == 2)
			pObject->SetZone(1.0f);
	}
	return true;
}

// 005810e0: orelha de alto elfo — vento 4 a 7 = 0,5, 7 a 9 = 1
bool ApplyHighElfEar(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (pObject->GetItemTypeId(0) == 0)
			return false;
		switch (GetWindLevel983(Wind().GetPower983()))
		{
		case 1:
			pObject->SetZone(0.5f);
			break;
		case 2:
			pObject->SetZone(1.0f);
			break;
		}
	}
	return true;
}

// 00581140: peça auxiliar — vento fraco (1 a 4) = 0,5
bool ApplyAuxPart(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (GetWindLevel983(Wind().GetPower983()) == 0)
			pObject->SetZone(0.5f);
	}
	return true;
}

// 00581180
bool ResetZone(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	return pObject->HasItems();
}

// 005811a0: ondina — vento 4 a 7 = 1, 7 a 9 = 0,5
bool ApplyUndine(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (pObject->GetItemTypeId(0) == 0)
			return false;
		switch (GetWindLevel983(Wind().GetPower983()))
		{
		case 1:
			pObject->SetZone(1.0f);
			break;
		case 2:
			pObject->SetZone(0.5f);
			break;
		}
	}
	return true;
}

// 00581200: mascote — tabela [nível do item][faixa do vento]; nível fora de 0…1 usa a linha 0
bool ApplyMascot(CImpactZoneObject* pObject)
{
	const float table[2][3] = { { 0.0f, 0.5f, 1.0f }, { 0.5f, 1.0f, 0.5f } };
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (pObject->GetItemTypeId(0) == 0)
			return false;
		int level = GetWindLevel983(Wind().GetPower983());
		if (level < 3)
		{
			int row = pObject->GetLevel();
			if (row < 2)
			{
				pObject->SetZone(table[row][level]);
				return true;
			}
			pObject->SetZone(table[0][level]);
		}
	}
	return true;
}

bool CWindImpactZoneElfEarController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyElfEar(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneElfEarController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyElfEar(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneElfEarImitationController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyElfEarImitation(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneElfEarImitationController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyElfEarImitation(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneHighElfEarController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyHighElfEar(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneHighElfEarController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyHighElfEar(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneAuxPartController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyAuxPart(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneAuxPartController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyAuxPart(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneUndineController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyUndine(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneUndineController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyUndine(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneMascotController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyMascot(GetControlledAs<CImpactZoneObject>());
}

bool CWindImpactZoneMascotController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyMascot(GetControlledAs<CImpactZoneObject>());
}

// 005814a0: clima — com o tempo 2 ou 3 (m_golfGame.weather) a zona é metade do nível do item
bool ApplyWeather(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	pObject->SetZone(0.0f);
	if (pObject->HasItems())
	{
		if (pObject->GetItemTypeId(0) == 0)
			return false;
		unsigned char weather = Doc()->m_golfGame.weather;
		if (weather == 2 || weather == 3)
			pObject->SetZone(pObject->GetLevel() * 0.5);
	}
	return true;
}

bool CWeatherImpactZoneController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyWeather(GetControlledAs<CImpactZoneObject>());
}

bool CWeatherImpactZoneController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyWeather(GetControlledAs<CImpactZoneObject>());
}

// 00581540: tacada normal (sem power shot e sem o campo +0x4bc do jogador da vez) repõe a zona guardada; senão guarda e zera
void ToggleSavedZone(CImpactZoneObject* pObject)
{
	if (GolfClub().PowerShotType() == 0)
	{
		unsigned char current = GOLFDOC()->m_currentPlayer;
		if (PlayerField4bc983(GOLFDOC()->GetPlayer(current)) == 0)
		{
			if (pObject->IsSaved() == true)
				pObject->Restore();
			return;
		}
	}
	if (!pObject->IsSaved())
		pObject->Save();
}

bool CWindImpactZoneElfEarController::On06(float arg)
{
	if (!IController::On06(arg))
		return false;
	CImpactZoneObject* pObject = GetControlledAs<CImpactZoneObject>();
	if (pObject == NULL)
		return false;
	ToggleSavedZone(pObject);
	return true;
}

bool CWindImpactZoneElfEarImitationController::On06(float arg)
{
	if (!IController::On06(arg))
		return false;
	CImpactZoneObject* pObject = GetControlledAs<CImpactZoneObject>();
	if (pObject == NULL)
		return false;
	ToggleSavedZone(pObject);
	return true;
}

// item que o jogador ainda tem: quantidade > 0 em m_myItemList
static inline bool HasMyItem(unsigned long typeId)
{
	const std::list<sItemInfo>& items = Doc()->GetConstMyItemList();
	std::list<sItemInfo>::const_iterator it = items.begin();
	for (; it != items.end(); ++it)
	{
		if (it->typeId == typeId)
			break;
	}
	return it != items.end() && it->Common[0] > 0;
}

// 00581630: item do dono — soma o valor de cada item da lista que o jogador ainda tem
bool ApplyOwnerItem(CImpactZoneObject* pObject)
{
	if (pObject == NULL)
		return false;
	float zone = 0.0f;
	unsigned int count = pObject->GetItemCount();
	for (unsigned int i = 0; i < count; ++i)
	{
		if (HasMyItem(pObject->GetItemTypeId(i)))
			zone += pObject->GetItemValue(i);
	}
	pObject->SetZone(zone);
	return true;
}

bool COwnerItemImpactZoneController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyOwnerItem(GetControlledAs<CImpactZoneObject>());
}

bool COwnerItemImpactZoneController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	return ApplyOwnerItem(GetControlledAs<CImpactZoneObject>());
}

// 00581700: só no cliente do jogador da vez — cada item da lista tem `percent` % de entrar na zona, com aviso no chat
void RollProbability(CImpactZoneObject* pObject, int percent)
{
	unsigned char current = GOLFDOC()->m_currentPlayer;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(current);
	unsigned long myUid = Doc()->m_myInfo.info.dwUID;
	if (pPlayer->uid != myUid)
		return;
	unsigned int count = pObject->GetItemCount();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long typeId = pObject->GetItemTypeId(i);
		if (typeId == 0)
			continue;
		if (rand() % 100 >= percent)
			continue;
		pObject->AddZone(pObject->GetItemValue(i));
		// 00d0b078: "%s의 능력으로 팡야 임팩트존이 커집니다." em CP949 (texto coreano do 983)
		const char* str = MakeStr(K2L_Compatibility("%s\xc0\xc7 \xb4\xc9\xb7\xc2\xc0\xb8\xb7\xce \xc6\xce\xbe\xdf "
			"\xc0\xd3\xc6\xd1\xc6\xae\xc1\xb8\xc0\xcc \xc4\xbf\xc1\xfd\xb4\xcf\xb4\xd9."), ItemManager()->GetItemName(typeId));
		CChatMsg::Instance()->AddChatMsg(std::string(str), 0xffffff00, false, false);
	}
}

// 00581880
bool ApplyProbability(CImpactZoneObject* pObject, int percent)
{
	if (!ResetZone(pObject))
		return false;
	RollProbability(pObject, percent);
	return true;
}

// 005818a0: o mesmo para o jogador do argumento, fora do green (tipo de chão 2)
bool ApplyProbabilityAt(CImpactZoneObject* pObject, int percent, int index)
{
	if (!ResetZone(pObject))
		return false;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(index);
	if (pPlayer == NULL)
		return false;
	if (GetPVS().GetGroundType(pPlayer->pos, true, NULL) == 2)
		return false;
	RollProbability(pObject, percent);
	return true;
}

bool CProbability5ImpactZoneController::On05()
{
	if (!IController::On05())
		return false;
	return ApplyProbability(GetControlledAs<CImpactZoneProbability5>(), 5);
}

bool CProbability5ImpactZoneController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	CImpactZoneProbability5* pObject = GetControlledAs<CImpactZoneProbability5>();
	return ApplyProbabilityAt(pObject, 5, arg);
}
