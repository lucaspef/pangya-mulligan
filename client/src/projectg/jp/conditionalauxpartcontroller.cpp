// JP 983: unidade 004acd40–004b0df0 (CConditionalAuxpartController). Classe nova do 983: a cada tacada testa a condição de cada
// peça auxiliar do CConditionalAuxpartObject (combo cheio, combo abaixo de 33, sorteio de 10%, bola fora do green) e manda o
// pacote 0x15d quando vale.
#include "conditionalauxpartcontroller.h"
#include "miraclesignskillcontroller.h"
#include "shareddocjp.h"
#include "golfdoc.h"
#include "club.h"
#include "quadtree.h"
#include "taskmanager.h"
#include "actor.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CConditionalAuxpartController, IController)

// 004ad0c0 (peças 0x70210000, 0x70210001 e 0x70210006): nada a fazer no slot 9
static void OnActionNone(CConditionalAuxpartObject*, int)
{
}

CConditionalAuxpartObject* CConditionalAuxpartController::GetControlled()
{
	return (CConditionalAuxpartObject*)m_pObject;
}

bool CConditionalAuxpartController::On04()
{
	if (!IController::On04())
		return false;
	GetControlled()->SyncDoc();
	return true;
}

// 004ae190 (peça 0x70210003): fora da tacada normal (CGolfDoc+0xf0 != 4) ou com power shot, desfaz; senão repete o aviso
static void OnActionDouble(CConditionalAuxpartObject* pObject, int index)
{
	CGolfDoc* pGolfDoc = GOLFDOC();
	if (pObject->GetPlayerIndex() != pGolfDoc->m_currentPlayer)
		return;
	sAuxpartValue* pValue = pObject->GetAt(index);
	if (pValue == NULL)
		return;
	if (GolfDocField_f0_983() != 4)
	{
		pObject->ResetAll();
		return;
	}
	if (GolfClub().PowerShotType() != 0)
	{
		pObject->ResetAll();
		return;
	}
	if (pValue->m_kind == -1)
		return;
	pObject->Notify(pValue->m_kind, pValue, GOLFDOC()->m_currentPlayer);
}

bool CConditionalAuxpartController::On02()
{
	DOCJP()->ClearAuxpartValues983();
	return true;
}

bool CConditionalAuxpartController::On03()
{
	DOCJP()->ClearAuxpartValues983();
	return true;
}

bool CConditionalAuxpartController::On06(float)
{
	CConditionalAuxpartObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07((int)pObject->GetApply());
	}
	return true;
}

// 004aec20 (peças 0x70210000 e 0x70210006): combo do jogador (ou do time) igual ao limite
static int CheckGaugeFull(int playerIndex)
{
	if (!IsControlledPlayerMine((unsigned char)playerIndex))
		return -1;
	float limit;
	float gauge;
	if (Doc()->m_golfGame.gameType != 1)
	{
		limit = Doc()->GetComboGaugeLimit((unsigned char)playerIndex);
		gauge = GOLFDOC()->GetPlayer((unsigned char)playerIndex)->gauge;
	}
	else
	{
		if (GOLFDOC()->GetPlayer((unsigned char)playerIndex)->teammate != 0xff)
		{
			float mine = Doc()->GetComboGaugeLimit((unsigned char)playerIndex);
			float mate = Doc()->GetComboGaugeLimit(GOLFDOC()->GetPlayer((unsigned char)playerIndex)->teammate);
			limit = mate > mine ? mate : mine;
		}
		else
			limit = Doc()->GetComboGaugeLimit((unsigned char)playerIndex);
		gauge = GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer((unsigned char)playerIndex)->team)->gauge;
	}
	return g_EPSILON > Wabs(gauge - limit) ? 0 : -1;
}

// 004aed70 (peça 0x70210001): combo do jogador (ou do time) abaixo de 33
static int CheckGaugeLow(int playerIndex)
{
	if (!IsControlledPlayerMine((unsigned char)playerIndex))
		return -1;
	float gauge;
	if (Doc()->m_golfGame.gameType != 1)
		gauge = GOLFDOC()->GetPlayer((unsigned char)playerIndex)->gauge;
	else
		gauge = GOLFDOC()->GetTeam(GOLFDOC()->GetPlayer((unsigned char)playerIndex)->team)->gauge;
	if (33.0f > gauge)
		return 0;
	return -1;
}

// 004aee00 (peça 0x70210003): fora do green, 10% de chance; devolve o kind 2
static int CheckRandom(int playerIndex)
{
	if (!IsControlledPlayerMine((unsigned char)playerIndex))
		return -1;
	if (GetPVS().GetGroundType(GOLFDOC()->GetPlayer((unsigned char)playerIndex)->pos, true, NULL) == 2)
		return -1;
	static MTRand1p1 s_random;
	return s_random.randInt(99) < 10 ? 2 : -1;
}

// 004aee90 (peça 0x70210005): fora do green
static int CheckNotGreen(int playerIndex)
{
	if (!IsControlledPlayerMine((unsigned char)playerIndex))
		return -1;
	return GetPVS().GetGroundType(GOLFDOC()->GetPlayer((unsigned char)playerIndex)->pos, true, NULL) != 2 ? 0 : -1;
}

bool CConditionalAuxpartController::On09()
{
	CConditionalAuxpartObject* pObject = GetControlled();
	if (pObject != NULL)
	{
		int count = pObject->GetCount();
		for (int i = 0; i < count; ++i)
		{
			sAuxpartValue* pValue = pObject->GetAt(i);
			if (pValue == NULL)
				continue;
			std::map<unsigned long, Action>::iterator it = m_actions.find(pValue->m_pItem->c.TypeId);
			if (it != m_actions.end())
				it->second(pObject, i);
		}
	}
	return true;
}

// 004af110: desfaz tudo, avisa o "Player" (0x337) e, para o jogador do objeto, testa a condição de cada peça
void CConditionalAuxpartController::Check(CConditionalAuxpartObject* pObject, int playerIndex)
{
	pObject->ResetAll();
	AfxGetTask()->GetActor("Player") << MsgObject(NULL, 0x337, 0, 0, 0, 0, 0);
	if (pObject->GetPlayerIndex() != playerIndex)
		return;
	int count = pObject->GetCount();
	for (int i = 0; i < count; ++i)
	{
		sAuxpartValue* pValue = pObject->GetAt(i);
		if (pValue == NULL)
			continue;
		std::map<unsigned long, Condition>::iterator it = m_conditions.find(pValue->m_pItem->c.TypeId);
		if (it == m_conditions.end())
			continue;
		int kind = it->second(playerIndex);
		if (kind != -1)
			pObject->Send(i, kind);
	}
}

bool CConditionalAuxpartController::On05()
{
	if (!IController::On05())
		return false;
	Check(GetControlled(), GOLFDOC()->m_currentPlayer);
	return true;
}

bool CConditionalAuxpartController::On07(int arg)
{
	if (!IController::On07(arg))
		return false;
	Check(GetControlled(), arg);
	return true;
}

bool CConditionalAuxpartController::On10(int arg)
{
	if (!IController::On10(arg))
		return false;
	if ((arg & 0xfffff) == 0x25)
	{
		Check(GetControlled(), GOLFDOC()->m_currentPlayer);
		AfxGetTask()->GetActor("Spot") << MsgObject(NULL, 0x11f, 0, 0, 0, 0, 0);
	}
	return true;
}

CConditionalAuxpartController::CConditionalAuxpartController()
{
	m_conditions.insert(std::make_pair(0x70210000UL, &CheckGaugeFull));
	m_conditions.insert(std::make_pair(0x70210001UL, &CheckGaugeLow));
	m_conditions.insert(std::make_pair(0x70210003UL, &CheckRandom));
	m_conditions.insert(std::make_pair(0x70210006UL, &CheckGaugeFull));
	m_actions.insert(std::make_pair(0x70210000UL, &OnActionNone));
	m_actions.insert(std::make_pair(0x70210001UL, &OnActionNone));
	m_actions.insert(std::make_pair(0x70210003UL, &OnActionDouble));
	m_conditions.insert(std::make_pair(0x70210005UL, &CheckNotGreen));
	m_actions.insert(std::make_pair(0x70210006UL, &OnActionNone));
}

CConditionalAuxpartController::~CConditionalAuxpartController()
{
}
