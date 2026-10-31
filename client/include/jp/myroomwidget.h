#pragma once
#include "vc10_layout.h"
// JP 983: CMyRoomWidget (unidade 005a7010–005aaf50; vtables 00d0ceb8 / 00d0ceb0 / 00d0cea4; RTTI 00e76c80). O "my room" do
// 983 não é mais uma tarefa: é uma `CTaskWidget` aberta por cima do lobby, com o form "realmyroom" da fábrica do Fresh. Não
// tem campos próprios (sizeof 0x1f8, o da base, pelo `new` de 005a74a0). A `CRealMyRoomTask` do 645 não existe no 983: o
// `OnPacket` dela é o [6] daqui (005a8360, 10409 B), que continua em src/projectg/realmyroomtask.cpp (frente A) até a
// resposta ao pedido B → A da B52 — aqui fica só declarado. Sessão B53.

#include "taskwidget.h"

class CMyRoomWidget : public CTaskWidget
{
	DECLARE_OBJECT(CMyRoomWidget)

public:
	CMyRoomWidget(); // 005a7010
	virtual ~CMyRoomWidget(); // 005a7b00 (com `delete`: 005a7d10; repasse de +8: 005a73c0)

	virtual void Open(); // 005a7f60: form "realmyroom", cria o CItemStorage
	virtual void Activate(); // 005a7790: encosta o form conforme a largura da tela e abre
	virtual void Close(); // 005a7870: mensagem 0x2af a "LobbyAvatar" e "LobbyCaddie", apaga o CItemStorage
	virtual void Process(float delta); // 005a7040: só repassa à base
	virtual int OnPacket(WReceivedPacket& packet); // 005a8360 (não escrita aqui — ver acima)
};

C_ASSERT_VC10(sizeof(CMyRoomWidget) == 0x1f8);
