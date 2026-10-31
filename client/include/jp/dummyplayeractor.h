#pragma once
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "actor.h"
#include "anotherhand.h"
#include "dummyplayer.h"
#include "lobbyjp.h"

namespace lua_system
{
	class lua_script;
}

// JP 983: um trabalho de carga da CDummyPlayerActor (0x28; construtor 004d68f0, destrutor 004d6900)
struct sDummyActorLoad983
{
	sDummyActorLoad983() : m_mask(0) { }
	unsigned long m_mask; // +0x00: estados do jogo (CGolfDoc+0x158) em que o resultado NÃO é entregue (0x1000 no 1º)
	std::tr1::function<bool()> m_fn; // +0x08
	bool m_bStop; // +0x20: o OnDestroy e o fim do trabalho põem 1; o trabalho da torcida para ao ver
	bool m_bReady; // +0x21: resultado pronto (o OnProcess entrega com a mensagem 0x33e)
};

// JP 983: um lugar do pódio (0x68; construtor 004d68d0, destrutor 004d6930). Só o começo foi levantado (B33); nenhuma
// função da unidade lê os campos (B34: os carregadores usam o registro do documento do Grand Prix, não este).
struct sDummyActorSlot983
{
	sDummyActorSlot983() : m_00(0), m_24(0), m_28(0) { }
	int m_00;
	int m_04;
	std::string m_name; // +0x08
	int m_24;
	int m_28;
	char m_2c[0x3c]; // NÃO levantado
};

// JP 983: argumentos de carga de um boneco do pódio (0x10; montado na pilha em 004da9c0 e em 004d9ff0)
struct sDummyActorArgs983
{
	unsigned long* tids; // +0x00: 24 peças
	unsigned long* buildUids; // +0x04
	unsigned long* uids; // +0x08
	unsigned char a; // +0x0c
	unsigned char b; // +0x0d
};

// JP 983: par (número, valor) da torcida do pódio ("AWARD_CROWD_%d")
struct sDummyActorCrowd983
{
	int id;
	int value;
};

// JP 983: bonecos do pódio do Grand Prix (RTTI .?AVCDummyPlayerActor@@, vtables 00d04c4c / 00d04c44, sizeof 0x3e0: `new` em
// 004d9a70; par no KR 839 004dde80…). Mesma forma da LobbyPlayer: IActor + CHandOwner, dois trabalhos de carga na outra
// linha (jogadores e torcida), roteiro "grandprix_ceremony.lua". Nomes meus. Sessões B33 (layout, construtor, destrutor,
// criador, registro e 7 funções) e B34 (HandleMsg, os carregadores e a leitura da torcida).
class CDummyPlayerActor : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 004d9880

	CDummyPlayerActor(); // 004d9760
	virtual ~CDummyPlayerActor(); // 004d9890 (com `delete`: 004d9b10)

	virtual void OnPreLoadInit(); // 004dab10
	virtual void OnLoad(); // 004d9fa0
	virtual void OnProcess(float delta); // 004d6960
	virtual void OnDisplay(); // 004d6a70
	virtual void OnDestroy(); // 004d5410
	virtual void HandleMsg(const MsgObject& msg); // 004d9ff0
	void RequestReady983(unsigned int key); // 004d4910 (mensagem 0x345: pede a entrega do trabalho `key`)

private:
	virtual void OnAnotherHand(int key, void* param); // 004d54c0

	void ReadCrowd983(); // 004d93a0: sorteia os 4 personagens da torcida para m_crowd
	void LoadPlayer983(unsigned long uid, unsigned char place, sDummyActorArgs983* args, int charIndex, int bUser); // 004d9ba0
	void LoadCrowd983(const char* name, int value); // 004d9e20
	bool LoadPlayersProc983(); // 004da9c0 (trabalho 0: os bonecos do documento do Grand Prix)
	bool LoadCrowdProc983(); // 004daaa0 (trabalho 1)

	lua_system::lua_script* m_pScript; // +0x1c
	std::vector<std::tr1::shared_ptr<IDummyPlayer> > m_players; // +0x20
	std::vector<std::tr1::shared_ptr<IDummyPlayer> > m_crowdPlayers; // +0x30 (tipo do elemento: 8 bytes com o destrutor 004d8eb0)
	std::vector<sDummyActorCrowd983> m_crowd; // +0x40
	CAnotherHand m_hand; // +0x50
	int m_1d8; // -1 no construtor
	int m_readyKey; // +0x1dc: trabalho cujo resultado se espera (-1 = nenhum)
	sDummyActorLoad983 m_loads[2]; // +0x1e0
	sDummyActorSlot983 m_slots[4]; // +0x230
	CPetFrame* m_pFrames[4]; // +0x3d0: bonecos dos 4 lugares (a cabeça dá a posição do nome)

	friend struct sDummyPlayerActorLayout983;
};
