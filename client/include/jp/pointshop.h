#pragma once
#include "vc10_layout.h"
#include <map>

// JP 983: unidade 008515c0–008527c0 (logo depois da FrPointShopDlg; o que vem em 008527c0 já é da CChampionShipImpl): CPointShop,
// a lógica da loja de pontos do Tiki ("TP"): até 5 itens escolhidos num std::map. Modo 1 = troca de itens meus por TP (chave =
// uid, pacote 0x128), modo 2 = compra com TP (chave = typeId, pacote 0x129, preço do PointShop.iff), modo 3 = nada escolhido
// (abertura). RTTI .?AVCPointShop@@, vtable 00d3191c (só o destrutor). Embutida na FrPointShopDlg em +0x1a0. Gêmea da
// CAlchemyMPBreak (B94). Nomes meus (sessão B106).
class CPointShop
{
public:
	// valor do mapa (nó +0x10): o item escolhido na lista da janela
	struct sItem
	{
		unsigned long uid;     // +0x0
		unsigned long typeId;  // +0x4
		int count;             // +0x8
	};
	typedef std::map<unsigned long, sItem> ItemMap;

	enum { MAX_ITEMS = 5 };

	CPointShop(); // 00852650
	virtual ~CPointShop(); // 008524d0 (com `delete`: 008526d0)

	unsigned long GetKey983(const sItem& item); // 008515c0: uid no modo 1, typeId no modo 2
	int GetPoint983(unsigned long typeId); // 00851810: TP que o item rende na troca (IFF +0x88)
	int GetUnitCount983(unsigned long typeId); // 008518e0: peça/taco = 1 (se existe); bola/item/carta = IFF +0x84
	void Send983(); // 00851e90: pacote 0x128 (troca) ou 0x129 (compra)
	int SetMode983(int mode); // 00852080
	void Clear983(); // 008520c0
	int GetTotal983(); // 008520d0: TP da troca (modo 1) ou preço da compra (modo 2)
	int Remove983(const sItem& item); // 008523e0: tira 1; some do mapa quando chega a 0
	sItem* Find983(const sItem& item); // 00852450
	sItem* Add983(const sItem& item); // 00852700: soma 1 (peça e taco só uma vez)
	ItemMap GetItems983() { return m_items; } // 0084fba0 (COMDAT na pointshopdlg)

	int m_mode; // +0x04 (1 troca, 2 compra, 3 nada; o construtor não grava)
	ItemMap m_items; // +0x08 (0x10: cabeça +0xc, tamanho +0x10)
};
C_ASSERT_VC10(sizeof(CPointShop) == 0x18);
