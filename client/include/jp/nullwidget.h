#pragma once
#include "vc10_layout.h"
// JP 983: CNullWidget (unidade 005aaf50–005ab060; vtables 00d0d760 / 00d0d758 / 00d0d74c; RTTI 00e76c8c, montado em 00cd1710;
// registro na fábrica 00cd1730; criador 005aafb0 com `new` de 0x1f8). O "boneco" que todo `CTask` guarda em +0x1fc quando não
// há widget aberta: o construtor do `CTask` (009bf436–009bf52a) cria um só pela fábrica (pelo nome do RTTI), guarda no
// estático `CTask::ms_pNullWidget983` (00f006f8) e o destrutor do `CTask` (009bc1bb) apaga-o. Não sobrescreve nada da
// `CTaskWidget` além do RTTI e do destrutor: o `OnPacket` [6] é o da base (devolve 0), então o repasse do `OnPacketCommon`
// (009c20d7) não precisa de guarda. Frente C, sessão C53 (pedido C46.1).

#include "taskwidget.h"

class CNullWidget : public CTaskWidget
{
	DECLARE_OBJECT(CNullWidget)

public:
	CNullWidget(); // 005aaf50
	virtual ~CNullWidget(); // 005aaf80 (com `delete`: 005ab040; repasse de +8: 005aafa0)
};

C_ASSERT_VC10(sizeof(CNullWidget) == 0x1f8);
