#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::CWorkShopEvent2013Handler (vtables 00d30110 / 00d30108 / 00d300fc; sem RTTI própria: GetRTTI = o do IObject),
// 0x28 bytes pelo `new` de 009200f0 / 009202e0 — este é o Initialize do CWorkShop2013Doc (documento 0xae, gatewaydoc.cpp da frente
// A, ainda sem fonte). Recebe os pacotes do evento da oficina de 2013 (param1 = função, tabela montada em 0082e6e0) e abre/atualiza
// a FrWorkShopEventDlg. Unidade 0082e330–0082e940 (o corte da F3.7.1 começava em 0082e6c0). Mesmo esqueleto do CQuestHandler
// (jp/questhandler.h): FrCmdTarget em +0x20 como destino do resultado da janela. Sessão B126; nomes dos métodos meus.
#include "controllerhandler.h"
#include "frcmdtarget.h"
#include "frform.h"

namespace _gateway
{
class FrWorkShopEventDlg;

class CWorkShopEvent2013Handler : public CGatewayHandlerJp, public FrCmdTarget
{
public:
	CWorkShopEvent2013Handler(); // 0082e7d0
	virtual ~CWorkShopEvent2013Handler() { } // 0082e860 (0082e910 com delete; ajustes 0082e840 / 0082e850)
	virtual bool Initialize(); // 0082e6e0
	virtual bool Execute(const MsgObject& msg); // 0082e6c0

	void OnOpen983(const MsgObject& msg); // id 0x00, 0082e420: 4 inteiros; abre a janela se fechada
	void OnUpdate983(const MsgObject& msg); // id 0x01, 0082e4d0: 4 inteiros; atualiza a janela (ou abre)
	void OnClose983(const MsgObject& msg) { } // id 0x02, 0082e340 (vazia)

	bool OnDlgResult983(int ret, FrForm* pForm); // 0082e330

protected:
	FrWorkShopEventDlg* m_pDlg; // +0x24
};
}
C_ASSERT_VC10(sizeof(_gateway::CWorkShopEvent2013Handler) == 0x28);
