// JP 983: unidade 0056acc0–0056aff0 (CMiracleSignSkillController). Classe nova do 983.
#include "miraclesignskillcontroller.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "chatmsg.h"
#include "taskmanager.h"
#include "packet.h"
#include "commonutil.h"

CMiracleSignSkillController::CMiracleSignSkillController()
{
}

IMPLEMENT_OBJECT(CMiracleSignSkillController, IController)

CMiracleSignSkillObject* CMiracleSignSkillController::GetControlled()
{
	return (CMiracleSignSkillObject*)m_pObject;
}

// 0056ade0: avisa o servidor (pacote 0x198 com o typeid do item) e manda a mensagem 1 ao ator principal
void SendMiracleSignSkill(unsigned long typeId)
{
	WSendPacket packet((enumClientPacket)0x198);
	packet.Encode4(typeId);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 0056aea0: fora dos jogos 0x0b..0x0e, com a bola do meu jogador no terreno 2, sorteia; se der, avisa e escreve no chat
bool CMiracleSignSkillController::Apply(unsigned char index)
{
	if (IsMiracleSignOffGame(Doc()->m_roomInfo.realGameType))
		return false;
	if (!IsControlledPlayerMine(index))
		return false;
	CMiracleSignSkillObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	if (!(pObject->GetPercent() > 0))
		return false;
	if (pObject->GetPlayerIndex() != index)
		return false;
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(pObject->GetPlayerIndex());
	if (pPlayer == NULL)
		return false;
	if (GetPVS().GetGroundType(pPlayer->pos, true, NULL) != 2)
		return false;
	if (!pObject->Roll())
		return false;
	SendMiracleSignSkill(pObject->GetTypeId());
	CChatMsg::Instance()->AddChatMsg(MakeStr(GetGameString(0x9c48), pObject->GetItemName()), 0xffffff00, false, false);
	return true;
}

bool CMiracleSignSkillController::On07(int arg)
{
	return Apply(arg);
}
