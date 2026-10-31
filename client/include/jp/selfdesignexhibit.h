#pragma once
#include "vc10_layout.h"
// JP 983: janelas do Self Design (venda de roupas UCC desenhadas pelo jogador). Unidade 00b3b3a0–00b3d360 (jp/selfdesignexhibit.cpp),
// que também traz o corpo da `sd_shop::CCommodityList` (lista de mercadorias do Self Design; classe declarada pela A em shareddoc.h).
//  - FrSelfDesignAccept ("FrSelfDesignAccept", 0x150 pelo `new` de 00b3cb50): pergunta "의상판매신청을하겠습니까?" com a imagem
//    "selfdesign.jpg"; o "confirmar" abre a FrSelfDesignExhibit com os mesmos dados. Mapa 00e419b0 (5 entradas, inicializador 00cf6c30),
//    vtable 00d54380, RTTI 00f03b34, registro 00cf6c10. Quem preenche: 00620e00 (SetData983 00b3cbe0).
//  - FrSelfDesignExhibit ("selfdesign_exhibit", 0x150): o mesmo texto desenhado em "draw_area" e o "confirmar" envia o pacote 0xfd
//    (dois inteiros, índice UCC e texto) e manda as mensagens 0x291 e 1 ao ator principal. Mapa 00e419b8 (5 entradas, 00cf6d30),
//    vtable 00d54430, RTTI 00f03b40, registro 00cf6d10.
// Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B128).
#include "frform.h"
#include <string>

class FrButton;
class FrViewer;

class FrSelfDesignAccept : public FrForm
{
	DECLARE_OBJECT(FrSelfDesignAccept) // GetRTTI 00b3bde0; MakeInstance 00b3cb50; registro 00cf6c10

public:
	FrSelfDesignAccept(); // 00b3c670
	virtual ~FrSelfDesignAccept(); // 00b3bd70 (com `delete`: 00b3c190; ajustes 00b3bdc0 / 00b3bdd0)

	// 00b3cbe0: dados que seguem para a FrSelfDesignExhibit
	void SetData983(int arg1, int arg2, const char* uccIndex, const std::string& text);

protected:
	void OnViewInit(int param); // 00b3b630: "selfdesign.jpg"
	void OnConfirmInit(int param); // 00b3b6b0
	void OnCancelInit(int param); // 00b3b740
	void OnConfirmClick(); // 00b3d000: abre "selfdesign_exhibit"
	void OnCancelClick() { Close(true); } // 00b3b3b0

	FrButton* m_pConfirmBtn; // +0x110: "btn_confirm"
	FrButton* m_pCancelBtn; // +0x114: "btn_cancel"
	FrViewer* m_pView; // +0x118: "view"
	int m_arg1; // +0x11c
	int m_arg2; // +0x120
	char m_uccIndex[9]; // +0x124
	std::string m_text; // +0x130

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00b3b3a0; mapa 00e419b0
};
C_ASSERT_VC10(sizeof(FrSelfDesignAccept) == 0x150);

class FrSelfDesignExhibit : public FrForm
{
	DECLARE_OBJECT(FrSelfDesignExhibit) // GetRTTI 00b3be60; MakeInstance 00b3cc30; registro 00cf6d10

public:
	FrSelfDesignExhibit(); // 00b3c720
	virtual ~FrSelfDesignExhibit(); // 00b3bdf0 (com `delete`: 00b3c1c0; ajustes 00b3be40 / 00b3be50)

	void SetData983(int arg1, int arg2, const char* uccIndex, const std::string& text); // 00b3ccc0

protected:
	void OnDrawAreaDraw(int param); // 00b3b7d0: pergunta em (x + 150, y + 70)
	void OnConfirmInit(int param); // 00b3b830
	void OnCancelInit(int param); // 00b3b8c0
	void OnConfirmClick(); // 00b3c7d0: pacote 0xfd
	void OnCancelClick() { Close(true); } // 00b3b3d0

	FrButton* m_pConfirmBtn; // +0x110: "btn_confirm"
	FrButton* m_pCancelBtn; // +0x114: "btn_cancel"
	int m_arg1; // +0x118
	int m_arg2; // +0x11c
	char m_uccIndex[9]; // +0x120
	std::string m_text; // +0x12c

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00b3b3c0; mapa 00e419b8
};
C_ASSERT_VC10(sizeof(FrSelfDesignExhibit) == 0x150);
