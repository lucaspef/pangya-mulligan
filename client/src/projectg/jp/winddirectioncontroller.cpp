// JP 983: unidade 0082d890–0082dd80 (CWindDirectionController). Classe nova do 983. Guarda as cópias fora de linha de quatro
// getters do objeto (0082d900…0082d930) e a instância std::min<unsigned char> (0082d9c0).
#include "winddirectioncontroller.h"
#include "miraclesignskillcontroller.h"
#include "classicwind.h"
#include "uiframesetmanager.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "quadtree.h"
#include "taskmanager.h"
#include "actor.h"
#include "packet.h"
#include "../../shared/mtrand1p1.h"
#include <algorithm>

extern WView* g_pMainView983; // 00f02458

IMPLEMENT_OBJECT(CWindDirectionController, IController)

CWindDirectionObject* CWindDirectionController::GetControlled()
{
	return (CWindDirectionObject*)m_pObject;
}

bool CWindDirectionController::Roll(const WVector& pos)
{
	if (GetPVS().GetGroundType(pos, true, NULL) == 2)
		return false;
	static MTRand1p1 s_random;
	int roll = s_random.randInt(99);
	return roll < 5;
}

// 006c8200: angulo entre o vetor (pino - jogador) (ou o inverso) e o eixo z, mais steps graus (x sinal); kind 0/2 invertem o sinal
float CWindJp::CalcChangedDirection983(const WVector& pos, unsigned char kind, unsigned char steps)
{
	static const WVector axis(0.0f, 0.0f, 1.0f); // 00e42544
	const WVector& pin = GOLFDOC()->GetHoleData().pin;
	WVector v(0.0f, 0.0f, 0.0f);
	float sign = 1.0f;
	switch (kind)
	{
	case 0:
		v.x = pin.x - pos.x;
		v.z = pin.z - pos.z;
		sign = -1.0f;
		break;
	case 1:
		v.x = pin.x - pos.x;
		v.z = pin.z - pos.z;
		break;
	case 2:
		v.x = pos.x - pin.x;
		v.z = pos.z - pin.z;
		sign = -1.0f;
		break;
	case 3:
		v.x = pos.x - pin.x;
		v.z = pos.z - pin.z;
		break;
	}
	float angle = CalcDeltaAngle(v, axis);
	if (steps > 10)
		steps = 10;
	return (float)steps * 0.017453292f * sign + angle; // tabela 00e16088 = grau * pi/180
}

// 0082da60: sorteia o tipo (0…3) e o tamanho (0…10) da troca, calcula a direção nova e manda o pacote 0x171
// { typeid do item, tipo, direção } + mensagem 1 à tarefa
void __fastcall SendWindDirection(const WVector& pos, unsigned long typeId)
{
	unsigned char steps = rand() % 11;
	unsigned char roll = rand() % 100;
	unsigned char kind = roll % 2;
	if (roll >= 70)
		kind += 2;
	kind = std::min<unsigned char>(kind, 3);
	steps = std::min<unsigned char>(steps, 10);
	float direction = WINDJP()->CalcChangedDirection983(pos, kind, steps);
	WSendPacket packet((enumClientPacket)0x171);
	packet.Encode4(typeId);
	packet.Encode1(kind);
	packet.EncodeBuffer(&direction, 4);
	packet.Send(TO_GAME);
	AfxGetTask()->SendMsgToMainActor(MsgObject(NULL, 1, 0, 0, 0, 0, 0));
}

bool CWindDirectionController::On06(float)
{
	CWindDirectionObject* pObject = GetControlled();
	if (pObject == NULL || pObject->Is28())
	{
		if (GOLFDOC()->m_gameMode & 8)
			pObject->On07((int)pObject->GetChange());
	}
	return true;
}

bool CWindDirectionController::Apply(unsigned char index)
{
	if (!IsControlledPlayerMine(index))
		return false;
	CWindDirectionObject* pObject = GetControlled();
	if (pObject == NULL || pObject->GetPlayerIndex() != index)
		return false;
	unsigned char owner = pObject->GetPlayerIndex();
	sPlayerData* pPlayer = GOLFDOC()->GetPlayer(owner);
	if (pPlayer == NULL)
		return false;
	if (!Roll(pPlayer->pos))
		return false;
	SendWindDirection(pPlayer->pos, pObject->GetTypeId());
	return true;
}

bool CWindDirectionController::On05()
{
	return Apply(GOLFDOC()->m_currentPlayer);
}

bool CWindDirectionController::On07(int arg)
{
	return Apply(arg);
}

bool CWindDirectionController::On12()
{
	CWindDirectionObject* pObject = GetControlled();
	if (pObject == NULL)
		return false;
	if (pObject->IsActive())
	{
		switch (pObject->GetKind())
		{
		case 1:
			CUiFrameSetManager::Instance()->DrawFrameSet("right_wind_wind_white", g_pMainView983, 0, -1);
			break;
		case 2:
			CUiFrameSetManager::Instance()->DrawFrameSet("right_wind_wind_dark", g_pMainView983, 0, -1);
			break;
		}
	}
	return true;
}
