// tutorialcontainer.h - frente C (pacotes). JP 983: documento 0xa4 do CContentsDoc (RTTI .?AVCTutorialContainer@@,
// vtable 00d3c5ec, 0x18 bytes; criado no CContentsDoc::Initialize 00915f9f com o construtor em linha que só grava a vtable).
// No 983 o estado do tutorial que o 645 guardava no CTutorialMain (m_Tutorials[i].type, m_reserved3cf...) mora aqui:
// o CTutorialMain guarda o ponteiro em +0x458 (007d2490) e o CTutorial em +0xdac (OnInit 007ccf20). Nomes nossos.
// Corpos em src/projectg/contentsdoc.cpp (unidade do 983: 009079a0-00907a79).
#pragma once

#include "icontentsdatacontainer.h"

class CTutorialContainer983 : public IContentsDataContainer
{
public:
	virtual bool Initialize(); // 009079a0: Reset()
	virtual bool Release(); // 009079b0
	virtual bool Reset(); // 009079c0: +4..+0x10 = -1, +0x14 = 0xff, +0x15 = 0

	// 009079e0 (C-PKT lote 51): fim da lição - pacote 0xae ao servidor de jogo. Era o caso M983(0x150) do
	// CTutorialMain::HandleMsg do 645; no 983 o CTutorial::MoveNextStep (007cd220) chama direto. `mode` não é lido.
	void SendTutorialResult983(int mode, unsigned long flags, int result, bool last);
	bool IsValid983() const; // 00907a80: m_category, m_mode e m_index >= 0

	// 007d2030/007d2040/007d2050/007d2060: definidos (fora de linha, sem chamador no 983) em tutorialmain.cpp
	void SetCategory983(int category);
	void SetMode983(int mode);
	void SetIndex983(int index);
	void SetSendResult983(bool bSend);

	int m_category; // +0x04: categoria (007d2300 grava; 645 m_Tutorials[i].type); vai no pacote 0xae
	int m_mode; // +0x08: índice + 1 (007d2300; 007d4110 copia para CSharedDoc+0x10d5c)
	int m_index; // +0x0c (007d2300)
	int m_10; // +0x10
	unsigned char m_pendingQuest; // +0x14: 0xff = nada (007d3a70); caso 0x120 do CTutorialWidget::OnPacket grava
	bool m_bSendResult; // +0x15: 645 CTutorialMain::m_reserved3cf (o 0xae só sai uma vez)
};
