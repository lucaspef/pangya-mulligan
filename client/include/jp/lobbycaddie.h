#pragma once
#include <functional>
#include <memory>
#include "actor.h"
#include "anotherhand.h"
#include "dummypet.h"
#include "lobbyjp.h"

// JP 983: caddie que fica em pé no lobby (vtables 00d0a40c / 00d0a404, sizeof 0x218, criador 00561600; par no KR 839 em
// 00562950…005634e0). Carrega o boneco numa linha à parte (CAnotherHand) e atende o mouse (girar e clicar). Nomes dos campos e
// dos métodos sem símbolo são meus.
class LobbyCaddie : public IActor, public CHandOwner
{
public:
	static const WRTTI m_RTTI; // 00e751fc
	virtual const WRTTI* GetRTTI() const { return &m_RTTI; } // 00560d60

	LobbyCaddie(); // 00561180
	virtual ~LobbyCaddie(); // 00560c90

	virtual void OnPreLoadInit(); // 005617b0
	virtual void OnProcess(float delta); // 00560d70
	virtual void OnDisplay(); // 00560290
	virtual void OnDestroy(); // 005616a0
	virtual void HandleMsg(const MsgObject& msg); // 00561820

private:
	virtual void OnAnotherHand(int key, void* param); // 00560080

	void RequestLoad983(unsigned long typeId, unsigned long caddieId); // 005600b0
	void SetPoint983(const char* name); // 005602a0
	void SetVisible983(bool bVisible, bool bFade); // 005602e0
	void Rotate983(float angle); // 00560330
	bool Fade983(float dt); // 005603c0
	bool Input983(float dt); // 005604b0
	bool Place983(); // 00560a60
	bool Load983(unsigned long caddieId, unsigned long typeId); // 00561240
	void Unload983(bool bWait); // 00561490
	void LoadNow983(); // 005616d0
	bool LoadProc983(); // 00561790

	std::tr1::shared_ptr<IDummyPet> m_pet; // +0x1c
	WVector m_pos; // +0x24
	bool m_bPlace; // +0x30
	unsigned long m_typeId; // +0x34
	unsigned long m_caddieId; // +0x38
	bool m_bVisible; // +0x3c
	bool m_bFading; // +0x3d
	float m_fadeTime; // +0x40
	bool m_bLocked; // +0x44 (mensagens 0x2ae / 0x2af)
	bool m_bDrag; // +0x45
	int m_48;
	std::tr1::function<bool()> m_loader; // +0x50
	bool m_bLoaded; // +0x68
	bool m_bRequest; // +0x69
	unsigned long m_reqCaddieId; // +0x6c
	unsigned long m_reqTypeId; // +0x70
	int m_74;
	CAnotherHand m_hand; // +0x78
	CRITICAL_SECTION m_csPet; // +0x200
};
