#pragma once
#include "shareddoc.h"

// JP 983: métodos do CSharedDoc que o fonte do 645 não tem (PROVISÓRIO: shareddoc.h é da frente A — pedido em out/PEDIDOS.md;
// quando entrarem no CSharedDoc, trocar `DOCJP()->` por `Doc()->` e apagar este arquivo). A classe não tem campo: só empresta
// o `this` do CSharedDoc às chamadas. Nomes meus.
class CSharedDocJp
{
public:
	// 0063bba0: fora dos modos 0x0b–0x0e (byte +0x109b8), acha o item 0x1be00016 (0063b570) e devolve `word +0xc == 1`
	bool IsTrajectoryPreviewOn();
	// 00633560: valor cifrado de 1 byte por buraco (`~(b[+0x114b8 + i*16 + 0xe] ^ b[+0x114b8 + i*16])`); significado não levantado
	unsigned char GetHoleValue983(int holeIndex);
	// 0062be40: zera os quatro inteiros de +0x11448…+0x11454 (estado do efeito de conjunto; significado não levantado)
	void ClearSetEffect983();
	// 0063f6c0: (1, jogador) — não lida além do começo; o CSetAbilityHiddenCutIn só grava o corte quando devolve falso
	bool Is63f6c0_983(int kind, unsigned short player);
	// 0062be60: `d[+0x11448 + player*4] = typeId` (um dos quatro inteiros que o ClearSetEffect983 zera)
	void SetSetEffect983(unsigned char player, unsigned long typeId);
	// 0062a340 (cópia fora de linha na unidade do CSetEffectObject): chave do std::map de +0x38a0 cujo valor (1 byte) é o
	// índice do jogador; -1 se não achar ou se o índice for 0xff
	int FindKeyByPlayer983(unsigned char player);
	// 0062bea0: `d[+0x114b8 + player*4] = p` (vetor de efeitos aplicados do CConditionalAuxpartObject de cada jogador)
	void SetAuxpartValues983(void* pValues, int player);
	// 0062bec0: zera os quatro ponteiros de +0x114b8
	void ClearAuxpartValues983();
	// 0062bee0: índice (0–3) do jogador da sala com este uid (`m_userInfo[i].info.dwUID`); 0xff se não achar
	unsigned char FindPlayerByUid983(unsigned long uid);
	// 00633580: buraco fixo da sala de repetição, cifrado (`~(b[+0x11606] ^ b[+0x115f8])`); 1–18, acima disso = nenhum
	unsigned char GetFixHole983();
	// 00636c40: grava o buraco fixo cifrado (b[+0x115f8] = ~v ^ b[+0x11606]; b[+0x115f9] = b[+0x115f8] ^ (byte do endereço))
	void SetFixHole983(unsigned char hole);
	// 0062bf50: tipo da lista de salas (CSharedDoc::m_roomListKind983 +0x11630; 0 = salas, 1 = Grand Prix); a FrGameRoomDlg guarda em +0x3e0
	int GetRoomListKind983();
	// 0062bf90: estado de +0x11638 (3, 4, 7 = dentro de sala; nome pela frente C, sessão C9)
	int GetState983();
	// 0062ba40: `IsControlServerService` do 645 com máscara e retorno de 64 bits (corpo não lido)
	__int64 IsControlServerService983(__int64 service);
	// 0062bf30: canal do Grand Prix (CSharedDoc::m_gpChannel983 +0x1163c, gravado pelo 0062bf20); zero deixa o "ready" do Grand Prix mandar o pacote 0x0e
	int GetGPChannel983();
	// 0063b570: item da lista `m_myItemList` (+0x375c) com este typeid (o `FindMyItemByGuid` compara o guid); NULL se não há (B65)
	sItemInfo* FindMyItemByTypeId983(unsigned long typeId);
	// 00636040: taxa de bonus do jogador idx (item auxiliar +0x69a6 e vetor +0x114b8); kind 0 soma, 1-3 multiplica, 4 fixa (B298)
	float GetBonusRate983(unsigned char idx, unsigned char kind);
	// 006333c0: horas que faltam até `end` (pela hora do servidor)
	__int64 GetRemainHours983(SYSTEMTIME* end);
};

#define DOCJP() ((CSharedDocJp*)Doc())
