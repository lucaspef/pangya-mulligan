#pragma once
#include "actor.h"
#include "encrypttypes.h"
#include "wind.h"

// JP 983: o que ainda falta no CWind da frente A (A135.1: os outros métodos já são do CWind, chamados por `Wind().`).
// PROVISÓRIO: quando a A der corpo ao 006c8200 e um acesso a m_direction, apagar a classe.
class CWindJp : public CWind
{
public:
	// 006c8200: direção nova para o item de troca de vento, a partir da posição do jogador (kind 0…3, steps 0…10)
	float CalcChangedDirection983(const WVector& pos, unsigned char kind, unsigned char steps);
	float GetDirection983() { return m_direction; } // cópia fora de linha em 0082dfd0
};

#define WINDJP() ((CWindJp*)&Wind())

// JP 983: RTTI .?AVCNoClassicWind@@, vtable 00cff91c, 0x18 bytes (sem campo próprio). Ator do modo sem vento clássico:
// só mantém o formato do pacote da mensagem 0x358 (dois inteiros zerados).
class CNoClassicWind : public IActor
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00490f00

	CNoClassicWind(); // 00490f40
	virtual ~CNoClassicWind(); // 00490ef0

protected:
	virtual void HandleMsg(const MsgObject& msg); // 00490fa0
};

// JP 983: RTTI .?AVCClassicWind@@, vtable 00cff95c, 0x3c bytes. Vento clássico: de tempos em tempos (5 a 7 s) sorteia um
// desvio da direção do vento (−20…+19 graus) a partir de CSharedDoc::m_holeRandom e avisa o ator "Effect" (0x1f0).
class CClassicWind : public IActor
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00491060

	CClassicWind(); // 00491630
	virtual ~CClassicWind(); // 00490ff0

	virtual void OnInit(); // 004916d0
	virtual void OnProcess(float delta); // 004918b0

protected:
	virtual void HandleMsg(const MsgObject& msg); // 00491700

	void ApplyWind(); // 00491430
	void Set(unsigned long count, int angle); // 004915b0
	void Next(); // 00491780
	void Update(); // 00491870

	int m_timer; // 0x18: milésimos até a próxima troca (negativo = trocar)
	WCrypticValue<int> m_angle; // 0x1c: desvio em graus
	WCrypticValue<unsigned long> m_count; // 0x2c: contador que alimenta o sorteio (soma os segundos de cada intervalo)
};
