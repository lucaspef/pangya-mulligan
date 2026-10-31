#pragma once
// JP 983: _gateway::_grandprix::CGrandPrixDocument (documento 0xaf do CContentsDoc; unidade grandprixdocument, 0054f480–00559cc0;
// vtable 00d09ecc, sizeof 0x64 pelo `new` de 00559c00). Layout pelo construtor 00559b00, pelo destrutor 00559980 e pelos métodos
// (sessão B68). Classe nova do 983, sem par no 645; nomes de método e de campo meus (os da B65 ficaram onde o handler os usa).
// O resto da faixa são instâncias da STL (map, vector, sort, stable_sort) que vêm sozinhas com o uso.
#include "gatewaydocjp.h"
#include "grandprix983.h"

#include <map>
#include <memory>
#include <vector>

class WReceivedPacket;
class CGPRoomTimerSteward;
struct sRoomInfo;

// premiado do Grand Prix (0xe4; pacote lido em 005582b0, desenhado pela FrGrandPrixAwardDlg e pelo CDummyPlayerActor)
struct sGPAward983
{
	unsigned long uid; // +0x00 (-1 = sorteado)
	unsigned long place; // +0x04: 1–3 no pódio, 4 = eu fora do pódio (ordem do sort: sem sinal)
	int bUser; // +0x08: 1 = jogador da sala (peças completas em tids); 0 = adversário de IA (00a17cb0)
	unsigned long tids[24]; // +0x0c
	unsigned long buildUids[5]; // +0x6c
	unsigned long uids[24]; // +0x80
	unsigned char a; // +0xe0
	unsigned char b; // +0xe1
};
C_ASSERT(sizeof(sGPAward983) == 0xe4);

namespace _gateway
{
namespace _grandprix
{
class CGrandPrixDocument : public CGatewayControlDocJp
{
public:
	typedef std::map<unsigned long, sGPRoomItem983> RoomMap;
	typedef std::pair<unsigned long, int> KindPair;

	CGrandPrixDocument(int type); // 00559b00
	virtual ~CGrandPrixDocument(); // 00559980 (apagador 00559bd0)
	virtual bool Initialize(); // 00555070
	virtual bool Release(); // 0054f4a0
	virtual bool Reset(); // 0054f4b0
	virtual CGatewayHandler* GetGatewayHandler() { return m_pHandler; } // 00559a80

	static CGatewayControlDocJp* __fastcall Create(); // 00559c00: `new CGrandPrixDocument(0xaf)`

	// pacotes e avisos (CGrandPrixHandler)
	void ReadList983(WReceivedPacket* pPacket); // 005592b0: lista de m_list, pares (005580d0) e m_score
	void UpdatePair983(WReceivedPacket* pPacket); // 00558250: (chave, valor) em m_pairs (acrescenta se não há)
	void RefreshTicket983(); // 005525b0: m_ticket = quantidade do item 0x1a000144 (0 sem o item)
	void Read5564a0_983(WReceivedPacket* pPacket); // 005564a0: refaz m_list e avisa o "Lobby" (0x352 / 99)
	void Read5582b0_983(WReceivedPacket* pPacket); // 005582b0: premiados (m_awards), completa o pódio com a IA e põe o jogador
	void Fn555140_983(); // 00555140: recalcula o estado das linhas de evento pela hora do relógio
	void Fn552750_983(); // 00552750: CSharedDoc+0x114b4 = taxa do Grand Prix da sala (CItemManager 00a194a0)
	void SetTypeId983(unsigned long typeId); // 005525e0: arma o aviso de abertura (CGPRoomTimerSteward) do Grand Prix typeId
	void ClearTimer983(int unused); // 00552700: desarma o aviso (ClearTimer pelo nome "%d")
	void Fn5566b0_983(sRoomInfo* pRoom); // 005566b0: liga a sala à linha do Grand Prix dela (sRoomInfo+0xcd)
	void Fn5566e0_983(sRoomInfo* pRoom); // 005566e0: desliga

	// chamados de fora
	unsigned long GetCurrentTypeId983(); // 00552740: typeid do aviso armado (0 = nenhum)
	sGPRoomItem983* FindRoomItem983(unsigned long typeId); // 005551f0
	sGPRoomItem983* FindRoomItemByPrev983(unsigned long typeId); // 00553dd0: a linha cujo `prevTypeId` é typeId
	// 005587b0: fração dos grupos (bits 8–18 de ticket) do tipo `kind` já com kind > 0 (+1 pelo grupo de `extra`, se ainda não); a
	// FrGrandPrixAwardDlg (0054d6f7) a usa
	float GetClearRate983(int kind, unsigned long extra);
	void GetRoomItems983(std::vector<sGPRoomItem983*>* pOut, __int64* pNow); // 00558ef0: linhas ligadas, conferidas e ordenadas
	void Open983(int kind); // 00559700: refaz as linhas do tipo `kind` e liga as salas abertas
	float GetScore983(); // 0054f480 (cópia fora de linha sem chamada vista)
	void SetKind983(int kind); // 0054f490 (idem)

	std::vector<sGPAward983>& GetAwards983() { return m_awards; }

	void LinkRooms983(std::vector<sRoomInfo*>& rooms); // 00556620
	void BuildRooms983(); // 00559540: uma linha por registro do GrandPrixData.iff do tipo m_kind
	void UpdateEnabled983(); // 00555260: bEnabled pela m_list
	void UpdateKinds983(); // 005552f0: kind de cada linha pelos m_pairs

	CGatewayHandler* m_pHandler; // +0x0c: ator "CGrandPrixHandler"
	std::vector<int> m_list; // +0x10
	std::vector<KindPair> m_pairs; // +0x20: (ticket, kind)
	RoomMap m_rooms; // +0x30: typeid → linha (nó de 0x68)
	std::vector<sGPAward983> m_awards; // +0x40
	std::unique_ptr<CGPRoomTimerSteward> m_pSteward; // +0x50
	int m_timerId; // +0x54: índice no CTimerManager (-1 = nenhum)
	int m_ticket; // +0x58: quantidade do item 0x1a000144
	float m_score; // +0x5c: do pacote da lista (limites +0x98/+0x9c da linha)
	int m_kind; // +0x60: tipo (bits 19–25 de ticket) mostrado: 0, 1–3 ou 99
};
}
}
