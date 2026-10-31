#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::CArinBirthDay2014Handler (RTTI .?AVCArinBirthDay2014Handler@_gateway@@, vtables 00d3c90c / 00d3c900 / 00d3c8f4;
// sem GetRTTI próprio), 0x28 bytes pelo `new` de 009211c0 (NewCreator, chamado pelo Initialize do CArinBirthDay2014Doc 00921360).
// Recebe os pacotes do evento de aniversário da Arin 2014 (param1 = função, tabela montada em 00478ab0) e abre/atualiza a janela RF
// RF2014ArinBirthdayDlg (instância única 00e78248). Unidade 00477bf0–00478cc2 (só as funções próprias; o resto da faixa são moldes
// da RF e do map<int, sFunctionHandler> que caíram aqui). Construtor/destrutor/criador fora de linha (00920fd0, 00921060, 009211c0)
// ficam na unidade do documento. Mesmo esqueleto do CWorkShopEvent2013Handler. Sessão B127; nomes dos métodos meus.
#include "controllerhandler.h"
#include "frcmdtarget.h"
#include "arinbirthday2014doc.h"

namespace _gateway
{
class CArinBirthDay2014Handler : public CGatewayHandlerJp, public FrCmdTarget
{
public:
	CArinBirthDay2014Handler() : m_24(0) { } // 00920fd0
	virtual ~CArinBirthDay2014Handler() { } // 00921060 (00921110 com delete; ajustes 00921040 / 00921050)
	virtual bool Initialize(); // 00478ab0
	virtual bool Execute(const MsgObject& msg); // 00478a30

	CArinBirthDay2014Doc* Doc983() const { return (CArinBirthDay2014Doc*)m_pDocument; }

	void OnOpen983(); // id 0x00, 00478720: passo pelos itens 0x1be00013/0x1a0002b8/0x1be00014; abre a janela se fechada
	void OnMissionList983(const MsgObject& msg); // id 0x01, 004787c0: lista do documento; com 5 ou mais missões abre
	void OnStep3_983(); // id 0x02, 00477bf0: passo 3 na janela aberta
	void OnStep5_983(); // id 0x03, 00477c10: passo 5 na janela aberta
	void OnMissionResult983(const MsgObject& msg); // id 0x04, 00477c30: 1 inteiro; janela da missão, estágio 10, resultado
	void OnMapResult983(); // id 0x05, 00477c80: janela do mapa e resultado
	void OnIgnore983(const MsgObject& msg) { } // id 0x06, 00477780 (vazia; `ret 4` dobrado com outras)

protected:
	int m_24; // +0x24 (zerado no construtor; ninguém mais lê)
};
}
C_ASSERT_VC10(sizeof(_gateway::CArinBirthDay2014Handler) == 0x28);
