#pragma once
#include <deque>
#include <functional>
#include <memory>
#include "actor.h"
#include "anotherhand.h"
#include "dummypet.h"
#include "lobbyjp.h"

// JP 983: pedido de carga da LobbyItems (fila em +0x48; blocos de 0x10 = 2 pedidos)
struct sLobbyItemReq983
{
	int index; // 0 = jogo de tacos (o único que o 983 carrega)
	unsigned long id;
};

// JP 983: itens do jogador postos no lobby — só o jogo de tacos (vtables 00d0a4ac / 00d0a4a4, sizeof 0x230, criador 00563680;
// par no KR 839 em 00563eb0…005654b0). Mesma forma da LobbyCaddie: carrega o boneco numa linha à parte (CAnotherHand), com
// uma fila de pedidos. Nomes dos campos e dos métodos sem símbolo são meus.
class LobbyItems : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00562ee0

	LobbyItems(); // 005633f0
	virtual ~LobbyItems(); // 00562df0 (com `delete`: 005635a0)

	virtual void OnPreLoadInit(); // 00563750
	virtual void OnLoad(); // 00561b80 (vazia)
	virtual void OnProcess(float delta); // 00562ef0
	virtual void OnDisplay(); // 005621f0
	virtual void OnDestroy(); // 005635d0
	virtual void HandleMsg(const MsgObject& msg); // 005637c0

private:
	virtual void OnAnotherHand(int key, void* param); // 00561f10

	void SetPoint983(int index, const char* name); // 00562220
	bool Place983(int index); // 00562270
	void Request983(int index, unsigned long id); // 00562ff0
	bool LoadClubSet983(unsigned long id); // 00563020
	void Unload983(int index); // 005631b0 (2 = todos, sem esperar a outra linha)
	void LoadNow983(int index); // 00563660
	bool LoadProc983(); // 00563720

	std::tr1::shared_ptr<IDummyPet> m_pets[2]; // +0x1c
	WVector m_pos[2]; // +0x2c
	bool m_bPlace[2]; // +0x44
	std::deque<sLobbyItemReq983> m_queue; // +0x48
	int m_60; // só o construtor grava
	std::tr1::function<bool()> m_loader; // +0x68
	bool m_bLoaded; // +0x80
	bool m_bReady; // +0x81: pode tirar o próximo pedido da fila
	int m_reqIndex; // +0x84
	unsigned long m_reqId; // +0x88
	int m_8c; // buraco de 4 bytes antes do CAnotherHand (igual ao +0x74 da LobbyCaddie; ninguém grava)
	CAnotherHand m_hand; // +0x90
	CRITICAL_SECTION m_csPet; // +0x218

	friend struct sLobbyItemsLayout983;
};
