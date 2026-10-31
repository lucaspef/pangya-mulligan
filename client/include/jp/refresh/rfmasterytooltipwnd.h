#pragma once
#include "vc10_layout.h"
#include "rfbase.h"

// JP 983: dica de texto das janelas de maestria RF (unidade rfmasterytooltipwnd.cpp, 005d71e0–005d7910).
// "kam_mastery_tooltip.xml" (RTTI .?AVRFMasteryToolTipWnd@@, vtable 00d115a4; base RFFormBase<RFWndEx>; 0x188 bytes): uma
// RFTextArea "message" cujo tamanho acompanha o texto (12 por letra da linha mais longa, até 300; 19 por linha). Criada pelas
// janelas da maestria (005bbe83, 005c5708, 005cc941). Sem instância global; o pai fica em 00e78234. Nomes meus (…983).
class RFMasteryToolTipWnd : public RFFormBase<RFWndEx>
{
public:
	RFMasteryToolTipWnd(); // 005d71e0
	virtual ~RFMasteryToolTipWnd(); // 005d7200 (??_G 005d7440)

	virtual void OnInitialUpdate(); // 005d7840 (slot 10): acha a área de texto "message"
	virtual void ShowTip983(bool show); // 005d7800 (slot 34, novo; o Show983 da RFFormBase não é virtual): ajusta o tamanho ao texto e mostra/esconde

	// 005d76c0: cria pelo "kam_mastery_tooltip.xml" (modo g_rfCreateMode983) e guarda o pai
	static RFMasteryToolTipWnd* Create983(RFWnd* parent);
	void SetText983(const char* text); // 005b9db0 (cópia na unidade rfcharmasterycardselectdlg; corpo ainda não escrito)
	// 005c8330 (cópia na unidade rfcharmasterystatdetailwnd; sessão B141)
	void SetTextW983(const std::wstring& text)
	{
		if (m_pText)
			m_pText->SetText(std::wstring(text.c_str()));
	}
	void SetPos983(float x, float y); // 005d7210: posição na tela, presa dentro da tela
	void SetTextStyle983(int value); // 005d7300: RFTextArea +0x248
	void FitSize983(); // 005d7490: tamanho da área de texto pelo nº de linhas e a linha mais longa

	static RFWnd* s_pParent983; // 00e78234

	RFTextArea* m_pText; // +0x184
};
C_ASSERT_VC10(sizeof(RFMasteryToolTipWnd) == 0x188);
