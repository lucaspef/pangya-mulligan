#pragma once
#include "vc10_layout.h"
// JP 983: CTutorialWidget (unidade 006b2bf0–006b30e0; vtables 00d19e6c / 00d19e64 / 00d19e58; RTTI 00e79058; `new` de 0x1f8
// em 006b2c60). Segunda derivada de `CTaskWidget` no 983 (a outra é a `CMyRoomWidget`): o tutorial aberto por cima do lobby.
// A `Activate` é o `CLobbyMain::OnToppage_Finish` do 645 mudado de lugar (a abertura do prêmio virou a mensagem 0x43 ao ator
// "Lobby"), e a `OnPacket` trata os pacotes 0x11f (progresso do tutorial, o 0x11b do 645) e 0x120. Sem campos próprios.
// Sessão B55; nomes meus.

#include "taskwidget.h"

class CTutorialWidget : public CTaskWidget
{
	DECLARE_OBJECT(CTutorialWidget)

public:
	CTutorialWidget(); // 006b2bf0
	virtual ~CTutorialWidget(); // 006b2cf0 (com `delete`: 006b2d10; repasse de +8: 006b2c50)

	virtual void Open(); // 006b3020: form "tutorialmain" (std::string estático 00e15ed4) e mensagem "TOPPAGE" ao ator principal
	virtual void Activate(); // 006b2d40
	virtual void Close(); // 006b2c20, vazia
	virtual void Process(float delta); // 006b2c30: só repassa à base
	virtual int OnPacket(WReceivedPacket& packet); // 006b2ee0
};

C_ASSERT_VC10(sizeof(CTutorialWidget) == 0x1f8);
