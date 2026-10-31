#pragma once
#include "vc10_layout.h"
// JP 983: CShopWidget (unidade 005e0ca0–005e30a0; vtables 00d12380 / 00d12378 / 00d1236c; RTTI 00e78c50; `new` de 0x1f8 em
// 005e0d70). A loja do 983 não é mais uma tarefa: a `CShopTask` do 645 não existe (sem RTTI; a fábrica só registra a
// `CLobbyTask`). É uma `CTaskWidget` aberta por cima do lobby (resultado 006572f0 do `CTaskMain`) com o form "shopmain" da
// fábrica do Fresh, gêmea da `CMyRoomWidget`. O `OnPacket` é o `CShopTask::OnPacket` do 645 (compra 0x68, presente 0x6a,
// 0xb6, 0x107, móveis 0x12d, caddie 0x194) com os avisos mandados ao ator principal da tarefa corrente. Sem campos próprios.
// Sessão B109; nomes dos métodos pela base.

#include "taskwidget.h"

class CShopWidget : public CTaskWidget
{
	DECLARE_OBJECT(CShopWidget)

public:
	CShopWidget(); // 005e0ca0
	virtual ~CShopWidget(); // 005e0e00 (com `delete`: 005e0e50; repasse de +8: 005e0d60)

	virtual void Open(); // 005e0fd0: form "shopmain"
	virtual void Activate(); // 005e0e80: encosta o form conforme a largura da tela e abre (igual à CMyRoomWidget)
	virtual void Close(); // 005e0cd0, vazia
	virtual void Process(float delta); // 005e0ce0: só repassa à base
	virtual int OnPacket(WReceivedPacket& packet); // 005e0ff0: 1 = tratou; 0x194 e os tipos de fora devolvem 0
};

C_ASSERT_VC10(sizeof(CShopWidget) == 0x1f8);
