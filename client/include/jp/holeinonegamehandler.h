// JP 983: ator "CHoleInOneGameHandler" (RTTI .?AVCHoleInOneGameHandler@_golfgame@_gateway@@, vtables 00d0ac7c / 00d0ac74, 0x3c bytes
// pelo `new` de 0057e7a0, construtor 0057dfc0). Recebe as mensagens 0x7535 do modo hole-in-one (param1 = função, tabela montada em
// 0057e0a0: 24 funções, ids 0x01–0x18). Nomes dos métodos meus. A unidade também tem os CTeeShotGroundBoneTransformer* (sessão B24).
#pragma once
#include "controllerhandler.h"
#include "holeinonegamedoc.h"
#include <vector>
#include "../shared/mtrand1p1.h"
#include "polysoup.h"

// JP 983: peça do chão do tee (o tipo de verdade é do polysoup.cpp, território da frente A; parece o WPolySoup::sModel do 983).
// EMPRÉSTIMO só com os campos que os transformadores usam (pedido B → A da B24).
class CTeeGroundAnim983
{
public:
	void Fn934b50_983(struct sTeeGroundModel983* pCenter, float height, float angle, const WVector& rot); // 00934b50 (polysoup.cpp)
};
struct sTeeGroundModel983
{
	char m_pad00[0x6c];
	float m_x; // +0x6c
	float m_y; // +0x70
	float m_z; // +0x74
	char m_pad78[0xb0 - 0x78];
	float m_b0; // +0xb0 (distância: 210 + i × 90/n no Normal; `CClub` 006add30(0) no Expert)
	CTeeGroundAnim983* m_pAnim; // +0xb4
};
typedef std::vector<sTeeGroundModel983*> TeeGroundModels983;

// JP 983: `cTeeShotGroundBoneTransformer` (vtable 00d0abfc: destrutor + 1 virtual puro; construtor 0057a120 guarda a semente
// em +4) é classe ANINHADA na WPolySoup (polysoup.h). B1166: era cópia com global próprio (g_pTeeShotTransformer983) e o
// WPolySoup::ms_pTeeShotTransformer (00e7ea88, lido por 009385c0 no modo 0xb) ficava NULL; agora é a mesma classe e o mesmo global.
typedef WPolySoup::cTeeShotGroundBoneTransformer cTeeShotGroundBoneTransformerJp;
// O registro do mapa (WPolySoup::sAttach983, 0xb8) visto pelos transformadores: +0x6c..+0x74 = posição de mat, +0xb0 = time.

// JP 983: derivadas (vtables 00d0ac08 / 00d0ac5c; `push 8` e `push 0x9e4` no OnEnter do handler).
class CTeeShotGroundBoneTransformerNormal : public cTeeShotGroundBoneTransformerJp
{
public:
	CTeeShotGroundBoneTransformerNormal(unsigned long seed); // 0057a190
	virtual ~CTeeShotGroundBoneTransformerNormal() { } // 0057ab00
	virtual bool Transform(WPolySoup::sAttach983* pin, const std::vector<WPolySoup::sAttach983*>& tees) // 0057d420
	{ return Transform983((sTeeGroundModel983*)pin, reinterpret_cast<const TeeGroundModels983&>(tees)); }
	bool Transform983(sTeeGroundModel983* pCenter, const TeeGroundModels983& models);
	static float RandAngle(MTRand1p1& rand); // 00579780: (rand × 45 − 20) / 0,28575
	static float Height(float dist, float angle); // 005797b0
};

class CTeeShotGroundBoneTransformerExpert : public cTeeShotGroundBoneTransformerJp
{
public:
	struct sItem // 0x10 (construtor 005797f0)
	{
		sItem() : m_pModel(NULL), m_angle(0.0f), m_kind(3), m_value(3) { }
		sTeeGroundModel983* m_pModel; // +0x00
		float m_angle; // +0x04 (ângulo em volta do centro; ordena a lista)
		int m_kind; // +0x08 (0, 1 ou 2 pela posição na volta)
		int m_value; // +0x0c (sorteado: 0…2)
	};
	CTeeShotGroundBoneTransformerExpert(int seed); // 0057d700 (o handler passa `rand()`)
	virtual ~CTeeShotGroundBoneTransformerExpert() { } // 0057dac0 (com delete)
	virtual bool Transform(WPolySoup::sAttach983* pin, const std::vector<WPolySoup::sAttach983*>& tees) // 0057db60
	{ return Transform983((sTeeGroundModel983*)pin, reinterpret_cast<const TeeGroundModels983&>(tees)); }
	bool Transform983(sTeeGroundModel983* pCenter, const TeeGroundModels983& models);
	float RandAngle(); // 00579830
	static float Height(float dist, float angle); // 00579860
	void AssignKinds(); // 0057a1c0
	void Apply(); // 0057ab30
	void SortByAngle(); // 0057d7d0

	MTRand1p1 m_rand; // +0x08 (0x9c8)
	sTeeGroundModel983* m_pCenter; // +0x9d0
	std::vector<sItem> m_items; // +0x9d4
};

namespace _gateway
{
namespace _golfgame
{
class CHoleInOneGameHandler : public CGatewayHandlerJp
{
public:
	static const WRTTI m_RTTI; // 00e75c70
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 0057dff0

	CHoleInOneGameHandler(); // 0057dfc0
	virtual ~CHoleInOneGameHandler() { } // 0057e010 (0057e830 com delete)
	virtual bool Initialize(); // 0057e0a0
	virtual bool Execute(const MsgObject& msg); // 0057ce90
	virtual void On05(); // 0057a2e0: doc 00577220 + 00576690; zera m_endTime
	virtual void OnProcess(float delta); // 0057c440
	virtual bool OnEnter(); // 0057d900
	virtual void OnLeave(); // 0057a300

	CHoleInOneGameDoc* Doc983() const { return (CHoleInOneGameDoc*)m_pDocument; }
	void SetHolePos(WVector& pos, bool bHoleIn); // 0057bad0: assenta no chão e grava no buraco atual do CGolfDoc
	void UpdateTime(float delta); // 0057bcf0: conta m_time para baixo; no zero, SendTimeOver
	void SendTimeOver(); // 0057aee0: em rede manda o pacote 0x137; fora dela avisa a "GolfRule" (0x93, 2)

	void On01(const MsgObject& msg); // id 0x01, 0057bd30: posição do buraco (da lista do doc nos modos 0x0d/0x0e; do pacote nos outros)
	void On02(const MsgObject& msg); // id 0x02, 0057c040: buraco na posição param2 da lista do doc
	void OnSetHole(); // id 0x03, 0057afb0: CGolfDoc 00661f30(buraco atual, &doc+0x48)
	void OnMsg04(const MsgObject& msg); // id 0x04, 005798b0: vazia
	void OnReadScreenValue(); // id 0x05, 0057afe0: "Screen" ← 0x246 e 0xbe (&valor); doc+0x44 = valor
	void OnGetScreenValue(const MsgObject& msg); // id 0x06, 0057b080: *(float*)param2 = doc+0x44
	void OnGetPair(const MsgObject& msg); // id 0x07, 0057b0a0: *(float*)param2 = doc+0x3c; *(float*)param3 = doc+0x40
	void On08(); // id 0x08, 0057c090: "Spin" ← 0x1cc (dois valores) → doc+0x3c/+0x40; zera com os tacos 0x18000000/1/0x11
	void OnGet7c(const MsgObject& msg); // id 0x09, 0057b0d0: *(int*)param2 = doc+0x7c
	void OnSetTime(const MsgObject& msg); // id 0x0a, 0057b0f0: m_time = *(float*)param2; se <= 0, SendTimeOver
	void OnSend12c(); // id 0x0b, 005798c0: pacote 0x12c
	void OnWait(); // id 0x0c, 0057b120: mensagem global 0x93 / 0x200000 + "Letter" ← 0x1fe "Wait"
	void On0d(const MsgObject& msg); // id 0x0d, 0057dc70: registro de um jogador → doc 00575ee0 + ficha da sala
	void On0e(const MsgObject& msg); // id 0x0e, 0057b1d0: doc 005706d0 (param2 != 0) ou 005706e0, depois 00570770
	void OnItem(const MsgObject& msg); // id 0x0f, 0057b210: param2 = item; se o registro tem +0x14, doc ×3 e "Score" ← 0x16f
	void On10(const MsgObject& msg); // id 0x10, 0057ceb0: lista de itens do pacote → "Screen" 0x16f e 0x247 (soma); doc+0x58
	void OnSet20(const MsgObject& msg); // id 0x11, 0057b2d0: m_20 = param2
	void OnGet20(const MsgObject& msg); // id 0x12, 0057b2f0: *(int*)param2 = m_20 (a CGolfRuleHoleInOne usa como índice do jogador)
	void OnToggleMission(); // id 0x13, 00579930: inverte m_bMission
	void OnGetMission(const MsgObject& msg); // id 0x14, 0057b310: *(bool*)param2 = m_bMission
	void On15(); // id 0x15, 0057a330: m_35 = false; doc 00570750
	void OnAddTime(const MsgObject& msg); // id 0x16, 0057b330: m_endTime += param2 × 1000
	void OnGetRemainTime(const MsgObject& msg); // id 0x17, 0057b350: *(int*)param2 = m_endTime − Doc+0x1057c + [00f02344]
	void On18(const MsgObject& msg); // id 0x18, 0057dce0: placar da sala (um bloco por jogador + pares)

private:
	int m_20; // +0x20 (não iniciado pelo construtor)
	bool m_bEntered; // +0x24 (1 no OnEnter; quem lê não foi visto)
	float m_time; // +0x28
	cTeeShotGroundBoneTransformerJp* m_pTransformer; // +0x2c
	int m_30; // +0x30
	bool m_bMission; // +0x34 (modo 0x0d no OnEnter; é o `bMission` que a regra lê pela mensagem 0x14)
	bool m_35; // +0x35
	int m_endTime; // +0x38 (ms)
};
}
}

extern cTeeShotGroundBoneTransformerJp* g_pTeeShotTransformer983; // 00e7ea88 (dono: wangreal/polysoup? — não visto)
