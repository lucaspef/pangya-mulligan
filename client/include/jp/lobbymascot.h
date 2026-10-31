#pragma once
#include <map>
#include <functional>
#include <memory>
#include "actor.h"
#include "anotherhand.h"
#include "dummypet.h"
#include "lobbyjp.h"

// JP 983: mascote do jogador posto no lobby (vtables 00d0a55c / 00d0a554, sizeof 0x210, criador 005650e0; par no KR 839).
// Mesma forma da LobbyItems, sem fila: um pedido por vez (m_reqId) e o boneco num ponteiro simples. Nomes dos campos e dos
// métodos sem símbolo são meus.
class LobbyMascot : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI;
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00564f30

	LobbyMascot(); // 00564fb0
	virtual ~LobbyMascot(); // 00564e60 (com `delete`: 005650b0)

	virtual void OnPreLoadInit(); // 005648f0
	virtual void OnLoad(); // 00563860 (vazia)
	virtual void OnProcess(float delta); // 00564340
	virtual void OnDisplay(); // 00563870
	virtual void OnDestroy(); // 005643e0
	virtual void HandleMsg(const MsgObject& msg); // 00565180

private:
	virtual void OnAnotherHand(int key, void* param); // 00563c10

	void Request983(unsigned long id); // 00563c40 (ninguém chama no 983)
	void Rotate983(float angle); // 00563cb0 (ninguém chama no 983)
	void SetPoint983(const char* name); // 00563ef0
	bool Place983(); // 00563f30
	void Unload983(); // 00563fc0
	bool LoadMascot983(unsigned long id); // 00564610
	void LoadNow983(); // 00564820
	bool LoadProc983(); // 00564890

	IDummyPet* m_pPet; // +0x1c
	std::map<unsigned long, std::tr1::shared_ptr<IDummyPet> > m_cache; // +0x20 (só o construtor e o destrutor mexem)
	unsigned long m_id; // +0x30: mascote carregado
	WVector m_pos; // +0x34
	bool m_bPlace; // +0x40
	int m_44; // ninguém grava
	int m_48; // +0x48: só o construtor grava
	std::tr1::function<bool()> m_loader; // +0x50
	bool m_bLoaded; // +0x68
	bool m_bReady; // +0x69: há pedido à espera da outra linha
	unsigned long m_reqId; // +0x6c
	CAnotherHand m_hand; // +0x70
	CRITICAL_SECTION m_csPet; // +0x1f8

	friend struct sLobbyMascotLayout983;
};
