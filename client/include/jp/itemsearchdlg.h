#pragma once
// JP 983: FrItemSearchDlg ("ItemSearchDlg"), unidade 005832d0–005846c0. Busca de itens por nome: o CTaskMain do 983 cria em
// CreateMaketDlg (00655d10, guarda em +0x1ac, resultado 00651060). Combo "combo_SearchItemGroup" com os 6 grupos da tabela
// estática (의상/아이템/캐디/마스코트/꾸미기/방꾸미기), "edit_SearchWord" (ENTER = buscar), "btn_Search" e a lista "list_Result"
// (OWNERDRAW: rosto do personagem + ícone + nome, e o item ligado ao lado). A busca não é feita aqui: o 0x2d3 ao ator principal
// leva o grupo, a palavra e a lista m_results, que o ator enche; vazia = aviso 0x24 "검색된 아이템이 없습니다.". Clique numa linha
// manda 0x2d6 e 0x2d7. Não existe no 645 (existe no 839). Mapa 00e0cd58 (9 entradas, inicializador 00ccf2e0); RTTI 00e75cec
// (00ccf2a0); registro 00ccf2c0; vtable 00d0b2b8; sizeof 0x150 (operator new de 00584510). Sessão B120; nomes dos membros meus.

#include <string>
#include <list>
#include "frform.h"
#include "../engine/wtypes.h"

class FrComboBox;
class FrEdit;
class FrButton;
class FrListBox;
class Bitmap;

// elemento da lista de resultados (nó de 0x4c: 8 + 0x44); quem enche é o ator principal (0x2d3)
struct sItemSearchResult983
{
	unsigned long typeId; // +0: 0 = linha vazia (não desenha)
	unsigned long uid; // +4: vai no 0x2d6/0x2d7
	std::string text; // +8: não usado aqui
	std::string name; // +0x24: texto da linha
	unsigned long linkedTypeId; // +0x40: item ligado (ícone + nome à direita; 0 = nenhum)
};

class FrItemSearchDlg : public FrForm
{
	DECLARE_OBJECT(FrItemSearchDlg) // GetRTTI 005843b0; MakeInstance 00584510

public:
	FrItemSearchDlg(); // 00584250
	virtual ~FrItemSearchDlg(); // 005843c0 (005845a0 com delete)

protected:
	virtual bool OnInit(); // 00583630

	void OnGroupInit(int param); // 005833a0
	void OnGroupLBtnDown(); // 00584410
	void OnWordInit(int param); // 00583410
	void OnWordEnterKey(int param); // 005846b0
	void OnSearchInit(int param); // 00583480
	void OnSearchBtnUp(); // 005845d0
	void OnResultInit(int param); // 005834f0
	void OnResultLBtnUp(); // 00583810
	void OnResultDraw(int param); // 00583e40

	void SendPick983(const sItemSearchResult983* p); // 005836c0: 0x2d6
	void SendPick2_983(unsigned long typeId, unsigned long uid); // 00583730: 0x2d7
	void AddResults983(std::list<sItemSearchResult983>& results); // 00583a00
	void DrawItemIcon983(unsigned long typeId, const WPoint& pos); // 00583a40
	void ClearResult983(); // 00584190
	void Search983(const std::string& word, int group); // 00584420

	FrComboBox* m_pGroup; // +0x110
	FrEdit* m_pWord; // +0x114
	FrButton* m_pSearch; // +0x118
	FrListBox* m_pResult; // +0x11c
	std::list<sItemSearchResult983> m_results; // +0x120 (0xc: cabeça, tamanho e o alocador vazio em +0x128)
	WRect m_faceRc; // +0x12c: rosto do personagem da última linha desenhada
	WRect m_iconRc; // +0x13c: ícone do item da última linha desenhada
	const Bitmap* m_pSelBmp; // +0x14c: "30_noplayer_base" (fundo da linha sob o cursor)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 005832d0
};
