#pragma once
#include "controller.h"
#include "winddirectionobject.h"
#include "wmath.h"

// JP 983: RTTI .?AVCWindDirectionController@@, vtable 00d30024, 0x10 bytes (sem campo próprio).
class CWindDirectionController : public IController
{
	DECLARE_OBJECT(CWindDirectionController)
public:
	CWindDirectionController() { } // embutido no MakeInstance (0082d940); a cópia fora de linha 0082d890 não é chamada
	CWindDirectionObject* GetControlled(); // 0082d9d0
	bool Roll(const WVector& pos); // 0082d9f0: fora do green, 5% (gerador próprio da unidade)
	bool Apply(unsigned char index); // 0082dbf0
	virtual bool On05(); // 0082dc70
	virtual bool On06(float arg); // 0082dbb0
	virtual bool On07(int arg); // 0082dc90 (salto para o Apply)
	virtual bool On12(); // 0082dca0: desenha a seta do vento trocado
};
