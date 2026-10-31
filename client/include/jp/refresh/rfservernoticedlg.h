#pragma once
#include "vc10_layout.h"
#include <string>
#include "rfbase.h"

// JP 983: aviso do servidor no lobby RF (unidade rfservernoticedlg.cpp, 005d7910–005d8160). "server_notice_dialog.xml"
// (RTTI .?AVRFServerNoticeDlg@@, vtable 00d1166c; base RFFormBase<RFWndEx>; 0x188 bytes): mostra o nome do servidor
// ("label_server_name") ao trocar de servidor/canal (RFLobbyDesktop::UpdateChannelName983 005fa980) e fecha sozinho depois
// de 1 s com a animação "_Closing_". Instância 00e78240. Nomes dos métodos meus (…983).
class RFServerNoticeDlg : public RFFormBase<RFWndEx>
{
public:
	RFServerNoticeDlg(); // 005d7910
	virtual ~RFServerNoticeDlg(); // 005d7930 (??_G 005d7b00)

	virtual void OnInitialUpdate(); // 005d7f70 (slot 10)
	virtual void Show983(bool show) { ShowWindow983(show); } // 005d7bc0 (slot 34, novo)

	// 005d7bd0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFServerNoticeDlg* Create983(RFWnd* parent);
	// 005d7e30 (__fastcall): texto do RFLabel filho `name`
	static void SetLabel983(RFWnd* wnd, const wchar_t* name, const std::wstring& text);
	void SetNotice983(const std::string& text); // 005d8070: nome do servidor (multibyte) e centraliza
	void OnUpdate983(EventArgs& e); // 005d7d10: conta o tempo; no fim "_Closing_" e fecha

	static RFServerNoticeDlg* ms_pInstance; // 00e78240
	static RFWnd* s_pParent983; // 00e7823c

	float m_time983; // +0x184: tempo até fechar (1 s)
};
C_ASSERT_VC10(sizeof(RFServerNoticeDlg) == 0x188);
