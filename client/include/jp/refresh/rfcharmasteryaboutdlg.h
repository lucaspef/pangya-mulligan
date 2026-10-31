#pragma once
#include "vc10_layout.h"
#include <string>
#include "rfformmodalbase.h"

struct sCharacterInfo;
class RFCharMasteryStatusDlg;
class RFCharMasteryCardDlg;
class RFCharMasteryTileDlg;

// JP 983: janela principal da maestria RF e a ajuda (unidade rfcharmasteryaboutdlg.cpp, 005bd040–005c0260; sessão B140;
// começo corrigido na B142).
// Classes pela RTTI; métodos e campos com nome meu (…983), menos os da RF.
// - RFCharMasteryAboutDlg ("kam_mastery_about_dialog.xml"; vtables 00d0ed2c / 00d0ed24; 0x1b8; modal): ajuda em 9 páginas
//   (label_btn_about_01..09 com os títulos em coreano por K2L_Compatibility; imagem "char_master_help_%d.jpg" em
//   pic_help_mastery_01; btn_forward/btn_backward/btn_confirm). Instância 00e78144 (só o construtor/destrutor mexem).
// - RFCharImgWnd ("kam_char_img_wnd.xml"; vtable 00d0edbc; 0x188; não modal): a figura do personagem em uso ("mastery_char_img_%d.png").
// - RFCharMasteryMainDlg ("kam_mastery_main_dialog.xml"; vtables 00d0ee54 / 00d0ee48; 0x1a8;
//   modal): moldura com duas abas (status_mgr_tap_check = janela de
//   status + janela de cartas; mastery_mgr_tap_check = janela de encaixes); as janelas filhas são criadas no lugar dos
//   marcadores status_wnd/card_wnd/slot_mastery_wnd/char_img_wnd do xml (que são apagados). Instância 00e78154. Aberta pelo
//   HandleMsgCommon (0065f60e) e por 006142e0.

class RFCharMasteryAboutDlg : public RFFormModalBase<RFWndEx>
{
public:
	enum { PAGE_COUNT983 = 9 };

	RFCharMasteryAboutDlg(); // 005bd3f0
	virtual ~RFCharMasteryAboutDlg(); // 005bd4c0 (??_G 005bdb10; 2ª base 005bd4b0)

	virtual void OnInitialUpdate(); // 005bf150 (slot 10)

	// 005bded0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCharMasteryAboutDlg* Create983(RFWnd* parent);
	void OnConfirm983(EventArgs& e); // 005bd250: fecha
	void ShowPage983(int page); // 005be010: página 1..9
	void Prev983(); // 005be1a0
	void Next983(); // 005be200
	void OnPage1_983(EventArgs& e); // 005bebb0
	void OnPage2_983(EventArgs& e); // 005bebc0
	void OnPage3_983(EventArgs& e); // 005bebd0
	void OnPage4_983(EventArgs& e); // 005bebe0
	void OnPage5_983(EventArgs& e); // 005bebf0
	void OnPage6_983(EventArgs& e); // 005bec00
	void OnPage7_983(EventArgs& e); // 005bec10
	void OnPage8_983(EventArgs& e); // 005bec20
	void OnPage9_983(EventArgs& e); // 005bec30
	void OnForward983(EventArgs& e); // 005bec40
	void OnBackward983(EventArgs& e); // 005bec50
	// 005bea70 (__fastcall): texto do RFLabelButton filho `name`
	static void SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text);
	void SetInstance983(); // 005bd040 (ninguém chama)
	static void ClearInstance983(); // 005bd050 (ninguém chama)

	static RFCharMasteryAboutDlg* ms_pInstance; // 00e78144
	static RFWnd* s_pParent983; // 00e78140

	RFLabelButton* m_pPageBtn983[PAGE_COUNT983]; // +0x188 (label_btn_about_01..09)
	int m_1ac_983; // +0x1ac (ninguém toca)
	int m_1b0_983; // +0x1b0 (ninguém toca)
	int m_page983; // +0x1b4 (1..9)
};
C_ASSERT_VC10(sizeof(RFCharMasteryAboutDlg) == 0x1b8);

class RFCharImgWnd : public RFFormBase<RFWndEx>
{
public:
	RFCharImgWnd(); // 005bd530
	virtual ~RFCharImgWnd(); // 005bd550 (??_G 005bdb40)

	virtual void OnInitialUpdate(); // 005be3a0 (slot 10)

	// 005be260: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharImgWnd* Create983(RFWnd* parent);
	void SetChar983(unsigned long typeId); // 005bdd50: "mastery_char_img_%d.png" (índice do typeid)
	void OnUpdate983(EventArgs& e); // 005bde40 (vazia; ninguém chama nem assina)

	static RFWnd* s_pParent983; // 00e78148

	RFPicture* m_pImg983; // +0x184 (char_img)
};
C_ASSERT_VC10(sizeof(RFCharImgWnd) == 0x188);

class RFCharMasteryMainDlg : public RFFormModalBase<RFWndEx>
{
public:
	enum { TAB_STATUS983 = 0, TAB_MASTERY983 = 1 };

	RFCharMasteryMainDlg(); // 005bd560
	virtual ~RFCharMasteryMainDlg(); // 005bd620 (??_G 005bdb70; 2ª base 005bd610)

	virtual void OnInitialUpdate(); // 005c0160 (slot 10)
	// 005bd290 (slot 34): ESC fecha. Assinado direto no EventKeyDown: o 005bd3c0 é o thunk de vcall do compilador (??_9 {136})
	virtual void OnKey983(EventArgs& e);

	// 005be450: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCharMasteryMainDlg* Create983(RFWnd* parent);
	void SetInstance983(); // 005bd060 (ninguém chama)
	static void ClearInstance983(); // 005bd070 (ninguém chama)
	static RFCharMasteryStatusDlg* GetStatusDlg983(); // 005bd080 (ninguém chama)
	static RFCharMasteryTileDlg* GetTileDlg983(); // 005bd240 (ninguém chama)
	void OnExit983(EventArgs& e); // 005bd2f0: fecha
	void SetTab983(int tab); // 005bdcc0 (não guarda a aba: m_tab983 fica −1)
	void OnStatusTab983(EventArgs& e); // 005bde50
	void OnMasteryTab983(EventArgs& e); // 005bde90
	void InitCharImg983(); // 005be590
	void InitStatus983(); // 005be6c0
	void InitCard983(); // 005be7e0
	void InitTile983(); // 005be920
	void OnHelp983(EventArgs& e); // 005bea50: abre a RFCharMasteryAboutDlg
	void InitControls983(); // 005bec60

	static RFCharMasteryMainDlg* ms_pInstance; // 00e78154
	static RFWnd* s_pParent983; // 00e78150

	RFCheck* m_pTab983[2]; // +0x188 (status_mgr_tap_check, mastery_mgr_tap_check)
	RFCharImgWnd* m_pCharImg983; // +0x190
	RFCharMasteryStatusDlg* m_pStatus983; // +0x194
	RFCharMasteryCardDlg* m_pCard983; // +0x198
	RFCharMasteryTileDlg* m_pTile983; // +0x19c
	sCharacterInfo* m_pChar983; // +0x1a0 (personagem em uso)
	int m_tab983; // +0x1a4 (−1)
};
C_ASSERT_VC10(sizeof(RFCharMasteryMainDlg) == 0x1a8);
