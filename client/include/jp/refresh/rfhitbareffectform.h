#pragma once
#include "vc10_layout.h"
#include "rfbase.h"

// JP 983: efeito da barra de impacto ("hitbar_impact_effect.xml"; RTTI .?AVRFHitBarEffectForm@@, vtable 00d10d9c; base
// RFFormBase<RFWndEx>; 0x184 bytes, sem campos próprios). Unidade rfhitbareffectform.cpp (005d0f00–005d1580), sessão B137.
// Com a nota da tacada (CGolfDoc::GetBar().grade 2…4) toca "_BestShot_" e some sozinha quando a animação acaba. No 983 nada
// chama Create983/Play983 (nem por ponteiro): sobra de código. Instância 00e7820c. Nomes dos métodos meus (…983).
class RFHitBarEffectForm : public RFFormBase<RFWndEx>
{
public:
	RFHitBarEffectForm(); // 005d0f20
	virtual ~RFHitBarEffectForm(); // 005d0f40 (??_G 005d1110)

	virtual void OnInitialUpdate(); // 005d1330 (slot 10)

	void SetInstance983(); // 005d0f00 (ninguém chama)
	static void ClearInstance983(); // 005d0f10 (ninguém chama)
	// 005d11f0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFHitBarEffectForm* Create983(RFWnd* parent);
	void OnUpdate983(EventArgs& e); // 005d11d0: esconde quando a animação acaba
	void Play983(); // 005d1460: mostra com nota > 1 e toca "_BestShot_"

	static RFHitBarEffectForm* ms_pInstance; // 00e7820c
	static RFWnd* s_pParent983; // 00e78208
};
C_ASSERT_VC10(sizeof(RFHitBarEffectForm) == 0x184);
