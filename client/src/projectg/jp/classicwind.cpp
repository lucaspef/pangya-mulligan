// JP 983: unidade 00490ef0–004918e0 (CNoClassicWind + CClassicWind). Classes novas do 983; nomes de método meus.
#include "classicwind.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "packet.h"

CNoClassicWind::~CNoClassicWind()
{
}

CNoClassicWind::CNoClassicWind()
{
	AddSkipList(0xffffffff);
}

// 00490fa0: mensagem 0x358 = (des)montar o estado no pacote de param1; param2 == 0 grava, senão lê
void CNoClassicWind::HandleMsg(const MsgObject& msg)
{
	if (msg.message != 0x358)
		return;
	if (msg.param2 == 0)
	{
		WSendPacket* packet = (WSendPacket*)msg.param1;
		packet->Encode4(0);
		packet->Encode4(0);
	}
	else
	{
		WReceivedPacket* packet = (WReceivedPacket*)msg.param1;
		packet->Decode4();
		packet->Decode4();
	}
}

CClassicWind::~CClassicWind()
{
	Wind().SetClassicDirection(0.0f);
}

// 004910a0: o vento está travado (campo 0x54 do CWind) ou o vento especial está ativo
static bool IsWindBusy()
{
	if (Wind().IsLocked983())
		return true;
	if (Wind().IsSpecial983())
		return true;
	return false;
}

// 004910c0: a camada 0x20 está em pausa (o mesmo teste de IActor::Skipped, com a camada fixa)
static bool IsLayerSkipped()
{
	if (IActor::m_pLayer && (*IActor::m_pLayer & 0x20))
		return true;
	return false;
}

IMPLEMENT_ACTOR(CNoClassicWind, IActor)

// 00491430: desvio em graus -> radianos no CWind, e o CWind refaz a direção
void CClassicWind::ApplyWind()
{
	float radian = (int)m_angle * (3.141592654f / 180.0f);
	Wind().SetClassicDirection(radian);
	Wind().ApplyClassicDirection();
}

// 00491470: próximo intervalo (5 a 7 s, em milésimos) e próximo desvio (−20…+19); se o desvio novo ficar a menos de 5 graus
// do atual, usa o atual + 25 (dentro da mesma faixa). O 983 recebe pTimer em esi e pCount em edi.
static void NextWind(unsigned long* pCount, int* pTimer, int angle, int* pAngle)
{
	unsigned long count = *pCount;
	unsigned long random = Doc()->m_holeRandom[(GOLFDOC()->m_currentHole + count - 1) % 18] + count;
	*pTimer = random % 3 + 5;
	*pCount += *pTimer;
	*pTimer *= 1000;
	int next = random % 40 - 20;
	float diff = (float)(next - angle);
	if (Wabs(diff) < 5.0f)
		*pAngle = (angle + 25) % 40 - 20;
	else
		*pAngle = next;
}

void CClassicWind::Set(unsigned long count, int angle)
{
	m_count = count;
	m_angle = angle;
}

CClassicWind::CClassicWind()
	: m_timer(0)
{
	m_dispPriority = DISP_PRIORITY_5;
	AddSkipList(0xfffb74d7);
	Set(0, 0);
	Wind().SetClassicDirection(0.0f);
}

void CClassicWind::OnInit()
{
	m_timer = 0;
	Set(0, 0);
	Wind().SetClassicDirection(0.0f);
}

// 00491700: mensagem 0x358, como no CNoClassicWind: grava contador e desvio, ou lê os dois e (vento destravado) aplica
void CClassicWind::HandleMsg(const MsgObject& msg)
{
	if (msg.message != 0x358)
		return;
	if (msg.param2 == 0)
	{
		WSendPacket* packet = (WSendPacket*)msg.param1;
		int angle = m_angle;
		packet->Encode4(m_count);
		packet->Encode4(angle);
	}
	else
	{
		WReceivedPacket* packet = (WReceivedPacket*)msg.param1;
		unsigned long count = packet->Decode4();
		int angle = packet->Decode4();
		Set(count, angle);
		if (!Wind().IsLocked983())
			ApplyWind();
	}
}

void CClassicWind::Next()
{
	unsigned long count = m_count;
	int angle = m_angle;
	NextWind(&count, &m_timer, angle, &angle);
	Set(count, angle);
}

IMPLEMENT_ACTOR(CClassicWind, IActor)

// 00491870: fora de pausa e com o tempo vencido: sorteia, aplica e avisa o ator "Effect" (mensagem 0x1f0)
void CClassicWind::Update()
{
	if (IsLayerSkipped())
		return;
	if (m_timer >= 0)
		return;
	Next();
	ApplyWind();
	SendMsg(this, "Effect", 0x1f0, 0, 0, 0, 0);
}

void CClassicWind::OnProcess(float delta)
{
	if (IsWindBusy())
		return;
	m_timer += (int)(delta * -1000.0);
	Update();
}
