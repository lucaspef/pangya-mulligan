#pragma once
// JP 983: FrGuildManageDlg (RTTI do 983), unidade 008fa3d0–008fb8e0 (vizinhas: addguildmemeberdlg da B e messengerchatdlg da
// A). Janela de gerência da guilda que o mensageiro guarda em FrMessengerDlg+0x18c: lista "memberlist" com os pedidos de
// entrada / convites pendentes (vetor CSharedDoc+0x10fe8 de sGuildJoinEntry983, preenchido pelo caso 0x116 do
// CTask::OnPacketCommon, que chama RefreshList983), botão "addmember" (abre a FrAddGuildMemeberDlg) e o menu de botão
// direito "rmenu_reqJoin" (aceitar/recusar entrada, recusar convite, mandar bilhete, ver informação). Molde: as listas do
// FrMessengerDlg do 645 (grupos com seta, ícones de sexo, bilhete com o custo de 10 pangs).
// Mapa 00e2ca30 (8 entradas, inicializador 00ce6080); registro 00ce6040, fábrica 00ce6060; sizeof 0x210 (operator new de
// 008faaf0). Sessão B113; nomes dos membros e dos métodos meus.

#include "frform.h"
#include "messengerdlg.h" // CMessengerInfo (usado pelo guildpackets.h)
#include "packets/guildpackets.h"

class FrListBox;
class FrContextMenuCtrl;
class FrButton;
class FrNoteDlg;
class FrAddGuildMemeberDlg;
class Bitmap;

class FrGuildManageDlg : public FrForm
{
	DECLARE_OBJECT(FrGuildManageDlg) // GetRTTI 008fa680; MakeInstance 008faaf0

public:
	FrGuildManageDlg(); // 008fa4b0
	// ~ 008fa690 (FrForm::~FrForm; 008fa6c0 com delete)

	void RequestList983(); // 008fa3f0 (não usa o this): pacote 0xa5 ao servidor de jogo (também chamado de 009c82ae)
	void RefreshList983(); // 008fafc0: refaz a lista a partir de CSharedDoc+0x10fe8 (caso 0x116 → 009b71f0)

protected:
	void OnRightMenuInit(int param); // 008fa6f0
	void OnRightMenuLBtnDown(); // 008fb790
	void OnAddMemberInit(int param); // 008fa770
	void OnAddMemberLBtnDown(); // 008faea0
	void OnMemberListInit(int param); // 008fa7e0
	void OnMemberListOwnerDraw(int param); // 008fab80
	void OnMemberListLBtnDown(); // 008fb1c0
	void OnMemberListRBtnUp(); // 008fb220

	void DrawGroup983(int param); // 008fa860
	sGuildJoinEntry983* FindJoinEntry983(unsigned long uid); // 008fb0d0
	void SendJoinAnswer983(unsigned long uid, unsigned char accept); // 008fb120: pacote 0xa7
	void SendInviteAnswer983(unsigned long uid, unsigned char accept); // 008fb340: pacote 0x26 ao mensageiro
	void OnNoteToMember983(); // 008fb5d0
	bool OnNoteDlgResult983(int ret, FrForm* pForm); // 008fb4b0
	bool OnAddMemberDlgResult983(int ret, FrForm* pForm); // 008fa3e0

	FrAddGuildMemeberDlg* m_pAddMemberDlg; // +0x110
	FrListBox* m_pMemberList; // +0x114
	FrContextMenuCtrl* m_pRightMenu; // +0x118
	FrButton* m_pAddMemberBtn; // +0x11c
	sGuildJoinEntry983 m_groupRow[3]; // +0x120 / +0x157 / +0x18e: linhas de grupo (uid −1; state 0, 1, 2 = separador)
	char m_pad1c5[3];
	int m_collapsed[3]; // +0x1c8: grupo fechado (0, 0, 1)
	const Bitmap* m_pGroupIcon[2]; // +0x1d4: up_btn_n (grupo aberto) / down_btn_n (fechado)
	const Bitmap* m_pSexIcon[6]; // +0x1dc: i_male, i_female, i_male_02, i_female_02, i_male_03, i_female_03
	FrNoteDlg* m_pNoteDlg; // +0x1f4
	unsigned long m_selectUID; // +0x1f8
	int m_inviteCount; // +0x1fc: linhas com state 2
	int m_joinCount; // +0x200: linhas com state 3
	char m_204[0x210 - 0x204]; // não iniciado; uso não visto

	DECLARE_FRESH_MSGMAP() // GetMessageMap 008fa3d0
};
