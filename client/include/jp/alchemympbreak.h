#pragma once
#include "vc10_layout.h"
#include <map>

// JP 983: unidade 00475740–00477740 (logo depois do aiplayer): CAlchemyMPBreak, a lógica da "quebra" de itens em pontos de
// alquimia (MP) do Tiki (FrAlchemyMPBreakDlg, 004f3110–004fb5f0, que a tem embutida em +0x188). RTTI .?AVCAlchemyMPBreak@@,
// vtable 00cfe5d0 (só o destrutor). Até 5 itens escolhidos num std::map (chave = guid); o pacote 0x18d manda a lista.
// Nomes meus (sessão B94).
class CAlchemyMPBreak
{
public:
	// valor do mapa (nó +0x10): o que a lista da janela guarda de cada item escolhido
	struct sItem
	{
		unsigned long guid;    // +0x0
		unsigned long typeId;  // +0x4
		int count;             // +0x8

		int GetCount() const; // 00475880 (sem chamador)
	};
	typedef std::map<unsigned long, sItem> ItemMap;

	enum { MAX_ITEMS = 5, FLAG_COUNT = 4 };

	CAlchemyMPBreak(); // 00477600
	virtual ~CAlchemyMPBreak(); // 00477480

	unsigned long GetKey(const sItem& item); // 00475740: guid só no modo 3
	void SetValues(int a, int b); // 00475770 (+0x2c/+0x30)
	void ResetFlags(); // 00475790
	bool GetFlag(int index); // 004757a0
	void SetFlag(int index, bool value); // 004757b0
	const char* GetFlagText(int index); // 004757d0: aviso de cada flag (textos 0x18705–0x18708)
	bool HasFlag(); // 00475840
	int GetFirstFlag(); // 00475860 (sem chamador)

	int GetBreakType(unsigned long typeId); // 00475ed0 (IFF +0x94)
	int GetBreakKind(unsigned long typeId); // 00475f00: 1, 2, 3 ou 0
	int GetMPItemCount(); // 00475f40 (item 0x1a0002a6)
	int GetMPPointCount(); // 00475f60 (item 0x1a0002a7)
	int GetItemMP(unsigned long typeId); // 00475f80 (IFF +0x8c)

	void SendBreak(); // 00476a20: pacote 0x18d
	__int64 GetTotalValue(); // 00476b20 (IFF +0x98 de cada item; a janela imprime com %I64d)
	int SetMode(int mode); // 00476ba0
	void Clear(); // 00476be0
	bool HasUccItem(); // 00476bf0
	bool HasType3Item(); // 00476c70
	bool HasUpgradedClubSet(); // 00476d20
	bool HasRareCard(); // 00476e60
	void UpdateFlags(); // 00476f10
	int CalcMP(); // 00476f90
	void UpdateMP(); // 00477270
	void UpdateOver(); // 004772a0
	int Remove(const sItem& item, int* pRemoved); // 004772d0
	sItem* Find(const sItem& item); // 00477340
	sItem* Add(const sItem& item); // 004776c0
	ItemMap GetItems() { return m_items; } // 004f9980 (COMDAT na alchemympbreakdlg)

	int m_mode; // +0x04 (3 = itens; o construtor não grava)
	ItemMap m_items; // +0x08 (0x10: cabeça +0xc, tamanho +0x10)
	bool m_flags[FLAG_COUNT]; // +0x18
	int m_mpItems; // +0x1c
	int m_20; // +0x20
	int m_mp; // +0x24
	int m_mpTotal; // +0x28
	int m_value2c; // +0x2c
	int m_value30; // +0x30
	bool m_bOver; // +0x34: MP + pontos ≥ 1000
};
C_ASSERT_VC10(sizeof(CAlchemyMPBreak) == 0x38); // provisório: a FrAlchemyMPBreakDlg tem o próximo membro gravado em +0x1c4 (0x188 + 0x3c)
