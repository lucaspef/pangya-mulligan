#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::CArinBirthDay2014Doc (RTTI .?AVCArinBirthDay2014Doc@_gateway@@, vtable 00d3c898, construtor 00920a90, 0x20 bytes
// pelo `new` do criador 00920be0, documento 0xb0 do CContentsDoc; registro 00920c70). No 983 o código está na unidade do
// gatewaydoc.cpp da frente A (00920a90–00920e60 e o Initialize 00921360); escrito pela B em jp/arinbirthday2014doc.cpp (sessão
// B127). Guarda as missões do evento de aniversário da Arin 2014 (lista do pacote, 0x10 cada). Nomes meus.
#include "gatewaydocjp.h"
#include <vector>

class WReceivedPacket;

namespace _gateway
{
class CArinBirthDay2014Handler;

// uma missão do evento (0x10; Decode983 lê 3 inteiros e 2 bytes "== 1")
struct sArinMission983
{
	sArinMission983(unsigned long id_, unsigned long value_, unsigned long count_, bool bDone_, bool bRewarded_)
		: id(id_), value(value_), count(count_), bDone(bDone_), bRewarded(bRewarded_) { }

	unsigned long id; // +0x0
	unsigned long value; // +0x4
	unsigned long count; // +0x8
	bool bDone; // +0xc
	bool bRewarded; // +0xd
};

class CArinBirthDay2014Doc : public CGatewayControlDocJp
{
public:
	CArinBirthDay2014Doc(int type); // 00920a90
	virtual ~CArinBirthDay2014Doc() { } // 00920ac0 (00920b40 com delete)
	virtual bool Initialize(); // 00921360
	virtual bool Release(); // 00920050
	virtual CGatewayHandler* GetGatewayHandler(); // 00920ab0

	static IContentsDataContainer* __fastcall Create(); // 00920be0: `new CArinBirthDay2014Doc(0xb0)`

	void Decode983(WReceivedPacket* pPacket); // 00920cc0: lista de missões (quantidade + registros)
	const std::vector<sArinMission983>& GetMissions983() const { return m_missions; }

protected:
	CArinBirthDay2014Handler* m_pHandler; // +0x0c
	std::vector<sArinMission983> m_missions; // +0x10
};
}
C_ASSERT(sizeof(_gateway::sArinMission983) == 0x10);
C_ASSERT_VC10(sizeof(_gateway::CArinBirthDay2014Doc) == 0x20);
