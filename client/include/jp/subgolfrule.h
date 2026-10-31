#pragma once
#include "golfrule.h"
#include "shareddocjp.h"
#include "actor.h"
#include "cardsystem.h"

// JP 983: sub-regra do CGolfRule (classes novas; RTTI CSubGolfRuleBase / CSubGolfRuleGame / CSubGolfRuleReplayTape). O CGolfRule
// guarda uma em +0x63c (criada em 00677290: `new` de 8 bytes, CSubGolfRuleReplayTape num ramo e CSubGolfRuleGame no outro) e
// repassa a ela o que no 645 estava dentro do próprio CGolfRule (o laço de carga dos buracos e a carga da fita de replay).
// Nomes de método meus; vtables 00d16980 / 00d169c4 / 00d169dc (5 virtuais).
class CSubGolfRuleBase
{
public:
	CSubGolfRuleBase(); // 0064f900
	virtual ~CSubGolfRuleBase(); // 0064f910 (slot 0 = 0064f9e0)
	virtual void Init(); // slot 1 (0064f920, vazio)
	virtual void Process(float dt); // slot 2 (0064f930, vazio)
	virtual void On3(); // slot 3 (0064f940, vazio; nenhuma derivada troca)
	virtual void OnShotEnd(); // slot 4 (0064f950, vazio)

protected:
	// 0064fd20: o CGolfRule::ShotDataLoad do 645, agora aqui (lê `m_pRule` em +4)
	bool ShotDataLoad(const char* filename);

	CGolfRule* m_pRule; // +0x04 (quem grava é o construtor da derivada)
};

class CSubGolfRuleGame : public CSubGolfRuleBase
{
public:
	CSubGolfRuleGame(CGolfRule* pRule); // 00650540
	virtual ~CSubGolfRuleGame(); // 00650560 (slot 0 = 00650570)
	virtual void Init(); // 006505a0
};

class CSubGolfRuleReplayTape : public CSubGolfRuleBase
{
public:
	CSubGolfRuleReplayTape(CGolfRule* pRule); // 006506e0
	virtual ~CSubGolfRuleReplayTape(); // 00650700 (slot 0 = 00650770)
	virtual void Init(); // 00650980
	virtual void Process(float dt); // 00650710
	virtual void OnShotEnd(); // 006507a0
};

// Campos do CSharedDoc do 983 por deslocamento medido no exe (PROVISÓRIO: sem nome conferido no shareddoc.h da frente A).
#define DOC983_U8(off) (*(unsigned char*)((char*)Doc() + (off)))
#define DOC983_U32(off) (*(unsigned long*)((char*)Doc() + (off)))
#define DOC983_F32(off) (*(float*)((char*)Doc() + (off)))
// B1537: DOC983_SHOTLIST (std::list cru em +0x110bc, offset do VC10; sem uso) removida: censo VS 2022.

// JP 983: o que o CGolfRule do 983 tem e o golfrule.h da frente A não (PROVISÓRIO, de empréstimo: pedido em out/PEDIDOS.md).
class CGolfRuleJp
{
public:
	CGolfRule::sShotData* GetShotData983() { return (CGolfRule::sShotData*)((char*)this + GOLFRULE_OFS_SHOTDATA); } // cópia fora de linha 00650730
	void SetFlag2c0_983(bool b) { *((unsigned char*)this + GOLFRULE_OFS_SHOTINFLIGHT983) = b; } // B1537: 983 +0x2c0 = m_bShotInFlight; no cl 14.44 0x2b4 (era 0x2c0 fixo)
	bool IsReplayShotDone983(); // 00680200 (lê Doc+0x11224 e o tamanho da lista de tacadas; corpo não lido)
	void SendMsg983(const char* name, const MsgObject& msg); // 00671330: GetActor(name) e, se existe, HandleMsg(msg)
	void ChangeGameMode983(unsigned long mode); // 0068c2f0 = CGolfRule::ChangeGameMode (protegido no golfrule.h)
};
#define RULEJP(p) ((CGolfRuleJp*)(p))

class CItemManagerJp2
{
public:
	void* Find983(unsigned long typeId); // 00a17160 (busca no mapa de +0x30; 0 se o item não existe)
};

class CCardManagerJp
{
public:
	void Set983(int index, void* pData); // 008099c0 (corpo não lido)
	void CalcCardPeriodAndStatus983(); // 0080a810
	void SetPlayerIndex983(int index); // 0080a590
};

class CSharedDocJp2
{
public:
	void AddCardPeriod983(const void* pInfo); // 00641410 (registro de 0x41 bytes na lista de +0x381c)
	void SetHoles983(unsigned char* pHoles, bool b); // 00636c60 (corpo não lido)
};

// 00627340: ReplayReadExtension983 (bloco "extension" da fita) em replayextension.h (sessão B58)
#include "replayextension.h"

// CSceneManager::GetPuppet 009b0da0 (o scenemanager.h só mostra a classe inteira dentro do pch dela)
class CSceneManagerJp
{
public:
	WPuppet* GetPuppet983(const char* name);
};

void RequestUccInfo(sUserInfo& info); // 007e9f10 (sem declaração em cabeçalho da frente A)
