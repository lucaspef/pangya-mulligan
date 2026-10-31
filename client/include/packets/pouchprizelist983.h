// pouchprizelist983.h - frente C (pacotes; C-PKT lote 59). JP 983: lista dos premios da bolsa (caso 0x129 tipo 0 do my room,
// 005a947d). Objeto estatico local de 0x10 em 005a8160 (guarda 00e76910, dados 00e76900, destrutor por atexit 00cf9e80).
// Quem le: CRealMyRoomMain::HandleMsg (00624d41) e o FrFortuneOpenDlg (00aaa190, 00aaa320) - frente A.
// Corpos em src/projectg/jp/pktviews983.cpp (no 983 moram na unidade do FrFortuneOpenDlg).
#pragma once
#include <list>
#include "../../src/shared/globalgamedefine.h"

struct RbPouchPrizeList983
{
	std::list<sPouchPrize> m_list;		// +0x00 (VC10: cabeca, tamanho, 2 alocadores vazios)
	unsigned long m_typeId;				// +0x0c tipo da bolsa aberta

	RbPouchPrizeList983() : m_typeId(0) { }

	static RbPouchPrizeList983* Instance983();	// 005a8160
	void Clear983();							// 00aa9940
	void SetTypeId983(unsigned long typeId);	// 00aa8f90
	unsigned int Add983(const sPouchPrize* prize);	// 00aaa2d0 (devolve o tamanho; o 983 nao usa)
};
