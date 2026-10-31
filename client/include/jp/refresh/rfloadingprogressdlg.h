#pragma once
#include "vc10_layout.h"
#include "rfformmodalbase.h"

// JP 983: janela modal de espera do lobby RF ("common_loading_waiting_dialog.xml"; RTTI .?AVRFLoadingProgressDlg@@, vtables
// 00d10e94 / 00d10e8c; base RFFormModalBase<RFWndEx>; 0x188 bytes, sem campos próprios). Unidade rfloadingprogressdlg.cpp
// (005d1580–005d1a70), sessão B137. Aberta pela seleção de personagem e pelo RFLobbyDesktop enquanto esperam o servidor;
// fecha com Close(1). Instância 00e78214. Nomes dos métodos meus (…983), menos a classe e os da RF.
class RFLoadingProgressDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFLoadingProgressDlg(); // 005d15b0
	virtual ~RFLoadingProgressDlg(); // 005d1640 (??_G 005d17d0; 2ª base 005d1630)

	virtual void OnInitialUpdate(); // 005d15a0 (slot 10)
	virtual void OnDestroy(); // 005d1800 (slot 11)
	virtual void Start983(); // 005d19b0 (slot 34, novo): DoModal + "_Opening_"

	// 005d1870: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFLoadingProgressDlg* Create983(RFWnd* parent);
	void SetInstance983(); // 005d1580 (ninguém chama)
	static void ClearInstance983(); // 005d1590 (ninguém chama)

	static RFLoadingProgressDlg* ms_pInstance; // 00e78214
	static RFWnd* s_pParent983; // 00e78210
	static int s_218_983; // 00e78218 (só zerado no OnDestroy; ninguém mais o lê no 983)
};
C_ASSERT_VC10(sizeof(RFLoadingProgressDlg) == 0x188);
