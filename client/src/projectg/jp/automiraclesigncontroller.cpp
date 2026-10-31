// JP 983: unidade 0047db10–0047de90 (CAutoMiracleSignController). Classe nova do 983. É a primeira unidade de controlador do
// exe e é a dona de `IsControlledPlayerMine` (0047dd60), que os outros controladores chamam. Guarda cópias fora de linha de
// três auxiliares embutidos (0047dc30 "sou galeria", 0047dd10 jogador pelo índice, 0047dd30 meu guid) e do TestMask (0047db60).
#include "automiraclesigncontroller.h"
#include "miraclesignskillcontroller.h"
#include "shareddocjp.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "networksystem.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"

CAutoMiracleSignController::CAutoMiracleSignController()
{
}

IMPLEMENT_OBJECT(CAutoMiracleSignController, IController)

CAutoMiracleSignObject* CAutoMiracleSignController::GetControlled()
{
	return (CAutoMiracleSignObject*)m_pObject;
}

// 0047dc50: pacote 0x185 com o typeid do item + mensagem 1 à tarefa
void __fastcall SendAutoMiracleSign(unsigned long typeId)
{
	WSendPacket packet((enumClientPacket)0x185);
	packet.Encode4(typeId);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

// 0047dd30: quem é galeria (bit 1 de dwIdentity) e pede o guid de galeria recebe dwGalleryGuid; os outros, dwGuid
inline unsigned long GetMyGuid983(bool bGallery)
{
	if (((Doc()->m_myInfo.info.dwIdentity >> 1) & 1) && bGallery)
		return Doc()->m_myInfo.info.dwGalleryGuid;
	return Doc()->m_myInfo.info.dwGuid;
}

// 0047dd60: o jogador `index` da partida sou eu? Só confere nos modos 0 (com o servidor de jogo conectado) e 5; nos outros
// modos (e sem conexão) responde sim
bool IsControlledPlayerMine(unsigned char index)
{
	sPlayerData* pPlayer;
	switch (Doc()->m_gameMode)
	{
	case 0:
		if (!WNetworkSystem::Instance()->IsConnected((WNetworkSystem::eNetUnit)0))
			break;
		pPlayer = GOLFDOC()->GetPlayer(index);
		return pPlayer->oid == GetMyGuid983(false); // 983 0047dd90: +0x4b4
	case 5:
		pPlayer = GOLFDOC()->GetPlayer(index);
		return pPlayer->oid == GetMyGuid983(false); // 983 0047dd90: +0x4b4
	}
	return true;
}

bool CAutoMiracleSignController::Apply(unsigned char index)
{
	if (!DOCJP()->IsTrajectoryPreviewOn())
		return false;
	if (!IsControlledPlayerMine(index))
		return false;
	CAutoMiracleSignObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	if (pObject->GetValue() <= 0)
		return false;
	if (pObject->GetPlayerIndex() != index)
		return false;
	unsigned char owner = pObject->GetPlayerIndex();
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(owner);
	if (pPlayer == NULL)
		return false;
	if (GetPVS().GetGroundType(pPlayer->pos, true, NULL) != 2)
		return false;
	SendAutoMiracleSign(pObject->GetTypeId());
	return true;
}

bool CAutoMiracleSignController::On07(int arg)
{
	return Apply(arg);
}
