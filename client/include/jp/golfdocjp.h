#pragma once
#include "golfdoc.h"
#include "quadtree.h"
#include "club.h"

// JP 983: métodos do CGolfDoc que o fonte do 645 não tem (PROVISÓRIO: o cabeçalho golfdoc.h é da frente A — pedido em
// out/PEDIDOS.md; quando entrarem no CGolfDoc, trocar `GOLFDOCJP()->` por `GOLFDOC()->` e apagar este arquivo).
// A classe não tem campo: só empresta o `this` do CGolfDoc às chamadas.
class CGolfDocJp
{
public:
	unsigned long* EnablePlayerEvent(unsigned long flag, unsigned char index); // 00661cb0: m_pPlayer[index].event |= flag
	void DisablePlayerEvent(unsigned long flag, unsigned char index); // 00661cf0: m_pPlayer[index].event &= ~flag
	// (C67.1: SetHoleRange983 00661eb0 e IsCurrentPlayerEvent983 00663870 agora pelo CGolfDoc da A)
};

#define GOLFDOCJP() ((CGolfDocJp*)GOLFDOC())

// JP 983: idem para CQuadTree e CClub (nomes provisórios; nenhum dos três tem nome no mapa da frente A).
class CQuadTreeJp
{
public:
	bool IsLosslessWeather(); // 009515e0 (consulta o evento 0x20 do jogador: CGolfDoc 00663870)
	void ApplyLosslessWeather(); // 00951780
};

class CClubJp
{
public:
	float Fn6add30_983(int arg); // 006add30 (club.cpp; devolve float; corpo não lido)
	void Refresh983(); // 006b0fb0 (refaz o taco do jogador da vez: 006b0630 com o sPlayer… de CGolfDoc+0xd8)
};

#define PVSJP() ((CQuadTreeJp*)&GetPVS())
#define GOLFCLUBJP() ((CClubJp*)&GolfClub())
