#pragma once
// JP 983: FrGrandPrixRoomDlg (unidade 0050f1e0–005100c0; vtables 00d06a9c / 00d06a90 / 00d06a88; sizeof 0x310 pelo `new` de
// 00510000; construtor 0050ff90 chamado também pelo CLobbyMain em 00ab4b85). Lista de salas do Grand Prix: a mesma FrRoomListDlg
// da FrMultiplayRoomDlg com a lista "roomlist" na `m_pRoomList2` (+0x2bc), cinco botões de filtro por canal (99 = evento,
// 0 = novatos, 1–3 = classes) e o relógio do Grand Prix (CGrandPrixClock) desenhado no controle "gpstate". Mapa de mensagens
// 00e09074 (19 entradas, inicializador 00ccc220). Sessão B69. Nomes dos métodos e campos são meus.

#include "roomlistdlg.h"

class FrArea;

// 0050f810 (chamada por 7 unidades de fora): taxa de partidas sem educação (CSharedDoc+0x98c / +0x960 × 100) abaixo de 3%
bool IsGoodMannerRate983();
// 0050f800 (chamada de 009cbdf7): peça que sai do boneco de quem tem má conduta (tabela 00e73e84)
bool IsNoMannerPart983(unsigned long tid);

class FrGrandPrixRoomDlg : public FrRoomListDlg
{
	DECLARE_OBJECT(FrGrandPrixRoomDlg)

public:
	FrGrandPrixRoomDlg(); // 0050ff90
	virtual ~FrGrandPrixRoomDlg(); // 0050f8a0

protected:
	virtual void OnProc(const float deltaTime); // 0050f1f0
	virtual bool OnInit(); // 0050fc80

	void InitRoomList983(); // 0050fd90 (o `OnRoomList_Init` do CLobbyMain do 645, como na FrMultiplayRoomDlg)
	void Leave983(); // 0050f270: para o som do lobby e esconde o relógio
	void Enter983(); // 0050fb20: pacote 0x5c, canal atual, aviso 0x753e / 4 ao "Gateway", relógio e som
	void SelectChannel983(int channel); // 0050f940: aperta o botão do canal e grava o canal no CSharedDoc (0062bf20)

	void OnRoomListInit(int param); // 0050f2a0
	void OnRoomListOwnerDraw(int param); // 0050f330
	void OnRoomListLDown(); // 0050f220
	void OnRoomListRUp(); // 0050f230
	void OnRoomListDClick(); // 0050f240
	void OnSortEventInit(int param); // 0050f360
	void OnSortEventUp(); // 0050fa40
	void OnSortRookieInit(int param); // 0050f3d0
	void OnSortRookieUp(); // 0050fa60
	void OnSortC1Init(int param); // 0050f440
	void OnSortC1Up(); // 0050fa80
	void OnSortC2Init(int param); // 0050f4b0
	void OnSortC2Up(); // 0050faa0
	void OnSortC3Init(int param); // 0050f520
	void OnSortC3Up(); // 0050fac0
	void OnLimitedTooltipInit(int param); // 0050f590
	void OnStateTicketInit(int param); // 0050f600
	void OnStateInit(int param); // 0050f670: põe o relógio na posição do controle
	void OnStateOwnerDraw(int param); // 0050f700: "gp_txt_clock", o relógio e o número de salas

	FrArea* m_pLimitedTooltip; // +0x300: "gp_limited_tooltip"
	FrArea* m_pStateTicket; // +0x304: "gpstate_ticket"
	char m_pad308[0x310 - 0x308];

	DECLARE_FRESH_MSGMAP()
};
