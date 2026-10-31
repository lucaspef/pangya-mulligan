#pragma once
#include "vc10_layout.h"
#include <string>
#include "rfformmodalbase.h"

// JP 983: "sobre o evento" da RF (unidade rfcommoneventaboutdlg.cpp, 005cfe00–005d0f00; sessão B138). "common_event_about_dialog.xml"
// (RTTI .?AVRFCommonEventAboutDlg@@, vtables 00d10c14 / 00d10c08; base RFFormModalBase<RFWndEx>; 0x19c bytes): imagem do evento
// (pic_event_bg) dentro da moldura wnd_event_bg; se a imagem for mais alta que a moldura, liga a barra vscrollBar_event (0–100,
// passo 10) e a roda do mouse. ESC fecha (Close(2)); label_event_help diz isso em coreano (K2L_Compatibility). Instância
// 00e78204. Create983 e SetImage983 sem chamador no 983. Nomes dos métodos e campos meus (…983), menos a classe e os da RF.

// RFVector2 com o construtor padrão FORA DE LINHA 004289b0 (zera x/y); o RFVector2 do rftypes.h tem o construtor vazio embutido
struct RFVector2Zero983 : public RFVector2
{
	RFVector2Zero983(); // 004289b0
};

class RFCommonEventAboutDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFCommonEventAboutDlg(); // 005cfe80
	virtual ~RFCommonEventAboutDlg(); // 005cff50 (??_G 005d0170; 2ª base 005cff40)

	virtual void OnInitialUpdate(); // 005d0800 (slot 10)

	void SetInstance983(); // 005cfe00 (ninguém chama)
	static void ClearInstance983(); // 005cfe10 (ninguém chama)
	void OnUpdate983(EventArgs& e); // 005cfe20 (vazia)
	void OnKeyDown983(EventArgs& e); // 005cfe30: ESC fecha
	// 005d0230: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCommonEventAboutDlg* Create983(RFWnd* parent);
	void OnMouseWheel983(EventArgs& e); // 005d0370: roda → RFScrollBar::Next/Prev
	void OnScroll983(EventArgs& e); // 005d0470: barra (0–100) → posição vertical da imagem
	// 005d05f0 (__fastcall): texto do RFLabel filho `name`
	static void SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text);
	// 005d0730 (__fastcall): mostra/esconde o filho `name`
	static void ShowChild983(RFWnd* wnd, const wchar_t* name, bool show);
	void SetImage983(const char* name); // 005d0c60: imagem do evento; mede e liga a rolagem

	static RFCommonEventAboutDlg* ms_pInstance; // 00e78204
	static RFWnd* s_pParent983; // 00e78200

	RFSize m_imageSize983; // +0x188: tamanho da imagem; depois de SetImage983, height = o quanto ela passa da moldura
	bool m_bScroll983; // +0x190: imagem mais alta que a moldura
	RFVector2Zero983 m_194; // +0x194 (só o construtor mexe)
};
C_ASSERT_VC10(sizeof(RFCommonEventAboutDlg) == 0x19c);
