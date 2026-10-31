#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetWorkshopAboutDlg (unidade 004edd80–004ee570; vtables 00d054ec / 00d054e0 / 00d054d8; RTTI 00e74dc8; mapa de
// mensagens 00e075d8 com 25 entradas, montado pelo inicializador estático; s_pInstance 00e74dc4). Ajuda da oficina de tacos
// ("clubset_workshop_help"): um FrViewer "about_viewer" com 10 páginas (help_clubmade_01…10.jpg), os botões de texto
// "index01"…"index10" (a página da vez fica selecionada) e "backward"/"forward". Aberta pelo btn_help da
// FrClubSetWorkshopMainDlg. sizeof 0x160 pelo `operator new` de 004ede70/004edf30. Existe no 839, não no 645.
// Nomes dos métodos e dos campos são meus (sessões B78/B91).
#include "clubsetworkshopform.h"

class FrButton;
class FrViewer;

class FrClubSetWorkshopAboutDlg : public ClubSetWorkShopForm
{
	DECLARE_OBJECT(FrClubSetWorkshopAboutDlg) // GetRTTI 004eddf0; MakeInstance 004ede70; registro 004edee0

public:
	static FrClubSetWorkshopAboutDlg* s_pInstance; // 00e74dc4

	FrClubSetWorkshopAboutDlg(); // 004edda0
	virtual ~FrClubSetWorkshopAboutDlg(); // 004ede00 (com `delete`: 004edf00; ajustes 004ede40 / 004ede50)

	// 004ee000: CreateForm<FrClubSetWorkshopAboutDlg>(pManager, pCmdDest, "clubset_workshop_help", NULL) (004edf30)
	static FrClubSetWorkshopAboutDlg* __fastcall Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest);

	virtual bool OnInit(); // 004ee3d0: pega os controles, página 0 e FrForm::OnInit (pula a da ClubSetWorkShopForm)

protected:
	virtual void OnProc(const float deltaTime) { } // 004ede30: vazio (não chama a base)
	virtual FrWnd* OnCtrlCreated983(IObject* pObj); // 004edd80: AddCtrlByType983

	// 004ee2d0: guarda a página (0…9), liga/desliga backward/forward nas pontas, marca o "indexNN" e abre a figura
	void SetPage983(int page);

	void OnIndex01Up983() { SetPage983(0); } // 004ee490
	void OnIndex02Up983() { SetPage983(1); } // 004ee4a0
	void OnIndex03Up983() { SetPage983(2); } // 004ee4b0
	void OnIndex04Up983() { SetPage983(3); } // 004ee4c0
	void OnIndex05Up983() { SetPage983(4); } // 004ee4d0
	void OnIndex06Up983() { SetPage983(5); } // 004ee4e0
	void OnIndex07Up983() { SetPage983(6); } // 004ee4f0
	void OnIndex08Up983() { SetPage983(7); } // 004ee500
	void OnIndex09Up983() { SetPage983(8); } // 004ee510
	void OnIndex10Up983() { SetPage983(9); } // 004ee520
	void OnBackwardUp983() { --m_page; SetPage983(m_page); } // 004ee530
	void OnForwardUp983() { ++m_page; SetPage983(m_page); } // 004ee550

	FrButton* m_pBackward; // +0x140: "backward"
	FrButton* m_pForward; // +0x144: "forward"
	void* m_unused148983; // +0x148: só o construtor zera (004eddd6), sem leitura no 983
	FrViewer* m_pViewer; // +0x14c: "about_viewer"
	int m_page; // +0x150

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004edd90; mapa 00e075d8
};
C_ASSERT_VC10(sizeof(FrClubSetWorkshopAboutDlg) == 0x160);
