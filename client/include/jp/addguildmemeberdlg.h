#pragma once
// JP 983: FrAddGuildMemeberDlg (grafia da RTTI do 983), unidade 008f98f0–008fa3d0 (008f94e0–008f98f0 ainda é o tradedlg da
// frente A: FrTradeEditDlg::OnInit 008f9870). No 645 o addguildmemberdlg.cpp é vazio. Convite de jogador para a guilda: é a
// FrAddFriendDlg do 645 (addfrienddlg.cpp) sem emoticon — "id" (digitar o ID; ENTER = conferir), "check" (pacote 7 ao
// servidor de jogo, com 1), "nick" (mostra o ID conferido, "<확인 안됨>" no começo) e "ok" (pacote 0x26 ao messenger com o meu
// ID, o alvo e a minha guilda). Mapa 00e2c920 (6 entradas, inicializador 00ce5f60); registro 00ce5f40.
// vtables 00d3b754 / ... ; sizeof 0x160 (operator new de 008f9ca0). Sessão B112; nomes dos membros meus.

#include <string>
#include "frform.h"

class FrEdit;

class FrAddGuildMemeberDlg : public FrForm
{
	DECLARE_OBJECT(FrAddGuildMemeberDlg) // GetRTTI 008f9c00; MakeInstance 008f9ca0

public:
	FrAddGuildMemeberDlg(); // 008f9b90
	// ~ 008f9c30 (008f9d30 com delete)

	void SetNick983(const char* text); // 008f9900: "nick"
	void SetId983(const char* text); // 008f9930: "id"
	void SetCheckResult983(const char* id, const char* target); // 008f9d60: resposta da conferência

protected:
	virtual void OnProc(const float delta); // 008f9a10

	void OnEditIdInit(int param); // 008f9a70
	bool OnEditIdEnterKey(int param); // 008fa3c0
	void OnEditIdDown(); // 008f9b30
	void OnDispNickInit(int param); // 008f9970
	void OnCheckBtnUp(); // 008f9dd0
	void OnOKBtnUp(); // 008f9fa0

	FrEdit* m_pEditId; // +0x110
	FrEdit* m_pDispNick; // +0x114
	std::string m_target; // +0x118: vai no pacote 0x26
	std::string m_checkedId; // +0x134: o ID conferido (o "ok" exige o mesmo, sem caixa)
	float m_elapsed; // +0x150
	bool m_bFocused; // +0x154
	char m_158[0x160 - 0x158]; // não iniciado; uso não visto

	DECLARE_FRESH_MSGMAP() // GetMessageMap 008f98f0
};
