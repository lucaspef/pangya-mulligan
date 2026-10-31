#pragma once
#include "vc10_layout.h"
// JP 983: registros do Grand Prix usados por mais de uma unidade da frente B (sessão B65: tirados do normallobbyroom.cpp, onde
// estavam desde a B51). A tabela é a GrandPrixData.iff (`IFF983::sGrandPrixData`, 0x330, na frente A só com bytes crus): aqui só os
// campos que o código da B lê. Nomes meus.
#include <string>

struct sRoomInfo;

// motivos de recusa de uma linha do Grand Prix (sGPRoomItem983::flags). O 983 testa cada um com `shr n; test 1` (campo de bits
// `bool`: 8 por byte, o 9º cai no bit 8); os dois de baixo juntos (`test 3`). Textos pela FrGrandPrixLimitDlg::OnInit (0059ec80).
struct sGPDeny983
{
	bool level0 : 1; // bits 0–1 (testados juntos): texto 0x7532 (nível)
	bool level1 : 1;
	bool b2 : 1; // texto 0x7534
	bool b3 : 1; // texto 0x7533
	bool b4 : 1; // texto 0x7536
	bool b5 : 1; // textos 0x7538 + 0x7550 (o único que não marca a janela como "limitada")
	bool b6 : 1;
	bool condition : 1; // bit 7 (0x80): falta o item de condição (0055e310); texto 0x7537 no detalhe
	bool prev : 1; // bit 8 (0x100): falta o Grand Prix `prevTypeId` (texto 0x753f com o nome dele)
	char m_2[2];
};
C_ASSERT(sizeof(sGPDeny983) == 4);

// registro de um Grand Prix (tabela do cliente); só os campos que esta unidade lê
struct sGPInfo983
{
	int m_00;
	int typeId; // +0x04
	unsigned long ticket; // +0x08: bits 19–25 = tipo (0 = comum, 0x63 = evento com horário), bits 8–18 = grupo; inteiro = item pedido
	                      // (0055ea60) e chave dos pares (tipo da linha) do CGrandPrixDocument
	int listId; // +0x0c: != 0 = a linha só fica ligada se este número veio na lista do servidor (CGrandPrixDocument 00555260)
	char m_10[0x12 - 0x10];
	char name[0x54 - 0x12]; // +0x12: nome (escrito na linha)
	int m_54; // +0x54: item de condição (0063b570)
	int m_58; // +0x58: quantidade mínima desse item
	char bmpName[0x85 - 0x5c]; // +0x5c: bitmap do fundo da linha no canal de evento
	bool m_85; // +0x85: liga o selo de m_pNameBmp[24] ao lado do nome
	bool m_86; // +0x86: liga o selo de m_pNameBmp[32]
	unsigned char m_87; // +0x87: 1 ou 2 acrescentam um texto de regra (0055ecc0)
	unsigned long artifact; // +0x88: artefato da regra (texto pelo FindArtifactDesc983; 0055ecc0)
	unsigned long course; // +0x8c: mapa (typeid 0x28000000 | n)
	int startHole; // +0x90: buraco inicial (2 = aleatório, 3 = embaralhado, 0 = 1º, outro = 10º; dica do "gp_detail", 00515ac0)
	unsigned char nHole; // +0x94: número de buracos ("%d홀")
	unsigned char minLevel; // +0x95: nível mínimo (FrGrandPrixLimitDlg: nome do nível pela tabela 00e75f0c)
	unsigned char maxLevel; // +0x96: nível máximo (0 = sem limite)
	char m_97;
	int m_98; // +0x98: 1º número da condição de tempo/placar (textos 0x754b/0x7556; significado não levantado)
	int m_9c; // +0x9c: 2º número (0xb4 escolhe o texto 0x7556; 0 = sem condição)
	char m_a0[0xac - 0xa0];
	unsigned char m_ac; // +0xac: número escrito com a fonte de FrRoomListDlg+0x15c
	char m_ad[0xb4 - 0xad];
	unsigned long prize[5]; // +0xb4: itens de prêmio (FrGrandPrixOpenDlg, "prize"; B66)
	int prizeCount[5]; // +0xc8: quantidade de cada um (lida como short)
	int prizeExtra[5]; // +0xdc: 3� n�mero de cada pr�mio (vai para o 2� campo do pr�mio da FrGrandPrixAwardDlg; sentido n�o levantado)
	unsigned short open[8]; // +0xf0: SYSTEMTIME da abertura
	unsigned short start[8]; // +0x100: do começo
	unsigned short end[8]; // +0x110: do fim
	char m_120[0x124 - 0x120];
	unsigned long prevTypeId; // +0x124: Grand Prix pedido antes deste (bit 8 da recusa; FrGrandPrixLimitDlg)
	char m_128[0x12c - 0x128];
	char desc[1]; // +0x12c: texto de descrição (FrTEXT)
};
C_ASSERT(offsetof(sGPInfo983, name) == 0x12 && offsetof(sGPInfo983, bmpName) == 0x5c && offsetof(sGPInfo983, m_86) == 0x86);
C_ASSERT(offsetof(sGPInfo983, course) == 0x8c && offsetof(sGPInfo983, nHole) == 0x94 && offsetof(sGPInfo983, m_ac) == 0xac);
C_ASSERT(offsetof(sGPInfo983, start) == 0x100 && offsetof(sGPInfo983, desc) == 0x12c);
C_ASSERT(offsetof(sGPInfo983, m_98) == 0x98 && offsetof(sGPInfo983, prevTypeId) == 0x124);
C_ASSERT(offsetof(sGPInfo983, artifact) == 0x88 && offsetof(sGPInfo983, prize) == 0xb4 && offsetof(sGPInfo983, prizeCount) == 0xc8);
// linha da lista do Grand Prix
struct sGPRoomItem983
{
	sGPInfo983* pInfo; // +0x00
	sRoomInfo* pRoom; // +0x04: sala já criada (NULL = ainda não)
	union
	{
		unsigned long flags; // +0x08: motivos de recusa (0x80 condição, 0x100 já jogado)
		sGPDeny983 deny;
	};
	int kind; // +0x0c: escolhe o selo da linha (0, 1, 2–3, outro)
	int state; // +0x10: 0 fora de hora, 1 o escolhido, 2 aberto, 3 em jogo, 4 encerrado (0055e1f0)
	int m_14;
	__int64 openTime; // +0x18
	unsigned short opts; // +0x20: bits 1 = mapa sorteado, 2 = com prêmio ("gp_gift"), 4 = selo de m_pNameBmp[27]
	std::string title; // +0x24: texto da linha (quando há)
	bool bEnabled; // +0x40

	// CGrandPrixDocument 00559540 monta a linha assim (state e m_14 ficam como vieram)
	explicit sGPRoomItem983(sGPInfo983* p) : pInfo(p), pRoom(NULL), flags(0), kind(0), openTime(0), opts(0), bEnabled(true) { }
};
C_ASSERT_VC10(offsetof(sGPRoomItem983, title) == 0x24 && offsetof(sGPRoomItem983, bEnabled) == 0x40);
// argumento do aviso 0x753e / 7 (entrar no Grand Prix)
struct sGPEnter983
{
	int state;
	sGPInfo983* pInfo;
	int m_08; // FrRoomListDlg+0x15c (fonte dos números: FrGrandPrixOpenDlg+0x130)
	int m_0c; // FrRoomListDlg+0x158 (fonte das horas: FrGrandPrixOpenDlg+0x134)
};


// 0055ecc0 (unidade gproomtimersteward; também chamada de 00515ac0 e 00acfcf0): texto da regra especial de um Grand Prix — descrição
// do artefato (+0x88) e os textos de m_86/m_87. O 983 passa o retorno em ecx e o registro em edx. Nome meu.
std::string __fastcall GPRuleText983(const sGPInfo983* pInfo);
