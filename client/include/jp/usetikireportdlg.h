#pragma once
// JP 983: `FrUseTikiReportDlg` (RTTI do 983; vtables 00d4ea04 / 00d4e9f4 / 00d4e9ec; sizeof 0x120 pelo `new` de 00afa460).
// Unidade 00afa390-00afa9ec (jp/usetikireportdlg.cpp; escrita pela frente C na sessao C47, C-PKT lote 40). Janela "notify_tiki"
// aberta pelo CLobbyMain::HandleMsg 0x8b com param1 0xc (00abcc8a; mandado pelo ESC da sala, FrGameRoomDlg::OpenTikiReport983):
// "usar o relatorio do Tiki?" com a quantidade do item 0x1a000041. No 983 ela substitui o par do 645 CLobbyMain::OpenUseReportDlg /
// OnRoomUseReportDlgResult (pacote 0xaa) + OpenGameExtResQuitForm / OnRoomExtResQuitDlgResult (pacote 0xf): "yes" usa o relatorio,
// "no" pergunta se sai ("notify_tiki_sub"). Nomes dos metodos e dos campos sao meus.

#include "frform.h"

class FrEdit;

class FrUseTikiReportDlg : public FrForm
{
	DECLARE_OBJECT(FrUseTikiReportDlg)

public:
	FrUseTikiReportDlg(); // 00afa3a0
	virtual ~FrUseTikiReportDlg() { } // 00afa3f0 (00afa4f0 com delete)

	void SetNotice983(const char* text); // 00afa400
	void SetAmount983(const char* text); // 00afa420

protected:
	void OnNoticeInit(int param); // 00afa520
	void OnAmountInit(int param); // 00afa590
	void OnYes983(); // 00afa810: usa o relatorio (pacote 0xaa)
	void OnNo983(); // 00afa790: abre "notify_tiki_sub" (sair da partida?)
	bool OnQuitConfirmResult983(int result, FrForm* form); // 00afa600: pacote 0xf

	int m_110; // +0x110: zerado no construtor; uso nao visto
	FrEdit* m_pNotice; // +0x114
	FrEdit* m_pAmount; // +0x118
	char m_11c[0x120 - 0x11c]; // nao iniciado no construtor; uso nao visto

	DECLARE_FRESH_MSGMAP()
};
