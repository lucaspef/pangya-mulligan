// JP 983: unidade do CClubsetExpMultiplyController (faixa 0056a440–0056ac90 com o par; gêmea do CLosslessHitController, gerada por tools/f37/gemeo2.py e conferida no 983).
#include "clubsetexpmultiplycontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"
#include "quadtree.h"
#include "../../shared/mtrand1p1.h"

IMPLEMENT_OBJECT(CClubsetExpMultiplyController, IController)

CClubsetExpMultiplyObject* CClubsetExpMultiplyController::GetControlled()
{
	return (CClubsetExpMultiplyObject*)m_pObject;
}

// 0056a580: pacote 0x181 com { 0x1f, typeid, typeid, 1 }
void CClubsetExpMultiplyController::Send(CClubsetExpMultiplyObject* pObject)
{
	struct
	{
		int ability;
		unsigned long typeId;
		unsigned long typeId2;
		int on;
	} data;
	data.on = 1;
	data.ability = eSpecialAbility_ClubsetExpMultiply;
	data.typeId = pObject->GetTypeId();
	data.typeId2 = pObject->GetTypeId();
	WSendPacket packet((enumClientPacket)0x181);
	packet.EncodeBuffer(&data, sizeof(data));
	packet.Send(TO_GAME);
}

bool CClubsetExpMultiplyController::On06(float)
{
	CClubsetExpMultiplyObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07(pObject->GetTypeId());
	}
	return true;
}

// 0056a650: o objeto existe, o jogador `index` é o meu e é o dono do item, a bola não está no terreno 0 nem no 2 e o sorteio
// (1 a 100 <= valor do item; gerador próprio da unidade em 00e75258) passou (o 983 passa o objeto em ebx e o índice em esi)
static bool IsMyClubsetExpMultiply(CClubsetExpMultiplyObject* pObject, int index)
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

bool CClubsetExpMultiplyController::On05()
{
	CClubsetExpMultiplyObject* pObject = GetControlled();
	if (IsMyClubsetExpMultiply(pObject, GOLFDOC()->m_currentPlayer))
		Send(pObject);
	return true;
}

bool CClubsetExpMultiplyController::On07(int arg)
{
	CClubsetExpMultiplyObject* pObject = GetControlled();
	if (IsMyClubsetExpMultiply(pObject, arg))
		Send(pObject);
	return true;
}
