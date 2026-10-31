// JP 983: unidade 0055ee70–0055f290 (CGroundInclinationController). Classe nova do 983. Entre o MakeInstance e os virtuais o
// 983 guarda as cópias fora de linha dos getters do objeto e do iterador do vector (0055eee0…0055f0d0).
#include "groundinclinationcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CGroundInclinationController, IController)

// 00860180: gerador estatico do 983 (semeado na 1a chamada, guarda no e7d160 bit 0), randInt(n) em [0,n]
unsigned long __fastcall RandInt983(unsigned long n)
{
	static MTRand1p1 s_random;
	return s_random.randInt(n);
}

CGroundInclinationObject* CGroundInclinationController::GetControlled()
{
	return (CGroundInclinationObject*)m_pObject;
}

bool CGroundInclinationController::On06(float)
{
	CGroundInclinationObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetActiveTypeId());
	}
	return true;
}

// 0055f0e0: o jogador `index` é o meu e o dono do item, a bola não está no tee nem no green (tipo de chão 0 e 2); sorteia cada
// item da lista (1 a 100 < 5 = 4%; NAO ha limiar de inclinacao) e, no primeiro que sai, manda o pacote 0x138 com o typeid e avisa a tarefa (mensagem 1)
bool __fastcall UseGroundInclination(CGroundInclinationObject* pObject, int index)
{
	if (pObject == NULL)
		return false;
	unsigned long uid = Doc()->m_myInfo.info.dwGuid;
	if (uid != GOLFDOC()->GetPlayer(index)->oid) // 983 +0x4b4 (oid = dwGuid, nao uid)
		return false;
	if (index != pObject->GetPlayerIndex())
		return false;
	switch (GetPVS().GetGroundType(GOLFDOC()->GetPlayer(index)->pos, true, NULL))
	{
	case 0:
	case 2:
		return false;
	}
	// B1492 (diagnostico, NAO e do 983): com a variavel de ambiente JP983_ASA definida o sorteio sempre sai (asa em toda vez fora
	// do tee/green), para testar a White Guardian Wing sem esperar os 4%. Sem a variavel = 983.
	static const bool s_forceRoll = GetEnvironmentVariableA("JP983_ASA", NULL, 0) != 0;
	CGroundInclinationObject::ItemList& items = pObject->GetItems();
	for (CGroundInclinationObject::ItemList::iterator it = items.begin(); it != items.end(); ++it)
	{
		int roll = s_forceRoll ? 1 : RandInt983(99) + 1;
		if (roll < 5)
		{
			WSendPacket packet((enumClientPacket)0x138);
			packet.Encode4(it->first);
			packet.Send(TO_GAME);
			AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
			return true;
		}
	}
	return false;
}

bool CGroundInclinationController::On05()
{
	CGroundInclinationObject* pObject = GetControlled();
	return UseGroundInclination(pObject, GOLFDOC()->m_currentPlayer);
}

bool CGroundInclinationController::On07(int arg)
{
	return UseGroundInclination(GetControlled(), arg);
}
