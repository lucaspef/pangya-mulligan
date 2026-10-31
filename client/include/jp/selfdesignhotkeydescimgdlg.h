#pragma once
#include "vc10_layout.h"
// JP 983: `SelfDesignHotkeyDescImgDlg` (RTTI do 983; vtables 00d15d04 / 00d15cf8 / 00d15cf0, sizeof 0x110 = o do FrForm pelo
// `push` do `operator new` em 00627530). Janela só de imagem (atalhos do self design), sem campo nem método próprio: o mapa
// de mensagens (00e14b78) tem uma entrada, "close" no botão solto → FrForm::OnFreshOkay. Sessão B57.
#include "frform.h"

class SelfDesignHotkeyDescImgDlg : public FrForm
{
	DECLARE_OBJECT(SelfDesignHotkeyDescImgDlg)

public:
	SelfDesignHotkeyDescImgDlg() {} // embutido no MakeInstance; cópia fora de linha 006274a0 sem chamador
	virtual ~SelfDesignHotkeyDescImgDlg(); // 006274d0 (com `delete`: 006274f0)

	DECLARE_FRESH_MSGMAP()
};

C_ASSERT_VC10(sizeof(SelfDesignHotkeyDescImgDlg) == 0x110);
