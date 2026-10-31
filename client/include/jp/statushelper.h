#pragma once
// JP 983: CStatusHelper, unidade 005e3430–005e49b0 (vtable 00d12c0c, só o destrutor virtual). Classe nova do 983 (existe no
// 839, 005e7eb0…): o cálculo dos 5 atributos (0 força … 4 curva) que o 645 fazia no CItemManager (GetLevel/GetCapacity/
// GetLevelWithPenalty) agora num objeto de 4 B criado na pilha por quem pergunta (CClub::GetRangeByEquip983 006ad580, janelas
// RF de atributo 005c…, 00a8b410). Duas famílias: as "My" leem o personagem/caddie/mascote/taco equipados do CSharedDoc
// (m_myInfo.userEquip); as outras recebem os tids. "Slot" = capacidade (teto do PCL); "PCL" = pontos gastos do personagem.
// A maestria (CharacterMastery.iff, vetor CItemManager+0x538) dá slots: um registro por (personagem, nível, atributo).
// Sessão B125 (pedido A77); nomes dos métodos são meus, exceto GetStatus983 (da A).

#include <vector>

struct sCharacterInfo;

// registro do CharacterMastery.iff (0x3c; o CItemManager do 983 guarda como bloco cru). Campos lidos aqui: +4, +8, +0xc.
struct sCharacterMastery983
{
	// D950: +0x14..+0x3b = custo em 5 posições (typeid[5] + quantidade[5]); valores do CharacterMastery.iff do pak 966
	unsigned long active983; // +0x0: 1 nos 140 registros
	unsigned long tidChar; // +0x4
	int level; // +0x8: nível de maestria a partir do qual vale
	int statType; // +0xc: atributo + 1
	unsigned long reqLevel983; // +0x10: nível do jogador exigido (0 = nenhum; RFCharMasteryTileDlg)
	unsigned long tpItemTid983; // +0x14: item pago = 0x1a0002a6 nos 140 (a tela usa a constante, não lê o campo)
	unsigned long stuffId983; // +0x18: peça de conquista exigida (QuestStuff.iff; 0 = nenhuma)
	unsigned long costTid983_1c[3]; // +0x1c: posições 2..4 do custo (0 nos 140)
	int tpCost983; // +0x28: TP para abrir (quantidade do +0x14)
	int stuffCount983; // +0x2c: quantidade da peça (1 quando stuffId983 != 0, 0 senão)
	int costCount983_30[3]; // +0x30: quantidades das posições 2..4 (0 nos 140)
};
typedef char rb_size_check_mastery983[sizeof(sCharacterMastery983) == 0x3c ? 1 : -1];

typedef std::vector<const sCharacterMastery983*> MasteryList983;

class CStatusHelper
{
public:
	CStatusHelper(); // 005e3430
	virtual ~CStatusHelper(); // 005e3440

	// por tids
	int GetCharStat983(unsigned long type, unsigned long tidChar); // 005e3850: sChar::Attr
	int GetEquipStat983(unsigned long type, const unsigned long* tidParts, const unsigned long* tidAuxParts,
		unsigned long tidClub, unsigned long tidCaddie, unsigned long tidMascot); // 005e3890
	int GetPowerLimit983(); // 005e3970: CalcPowerPenalty(nível) + 20
	int GetClubSlot983(unsigned long type, unsigned long tidClub); // 005e3990: sClubSet::Slot
	int GetLevelSlot983(unsigned long type, int level = -1); // 005e39d0: +1 de força a cada 5 níveis (-1 = o meu)
	int GetCharPCL983(unsigned long type, unsigned long tidChar); // 005e3c00: PCL do meu personagem com esse tid
	int GetStatusOf983(unsigned long type, unsigned long tidChar, const unsigned long* tidParts,
		const unsigned long* tidAuxParts, unsigned long tidClub, unsigned long tidCaddie,
		unsigned long tidMascot); // 005e3cc0
	int GetPenaltyOf983(unsigned long type, unsigned long tidChar, const unsigned long* tidParts,
		const unsigned long* tidAuxParts, unsigned long tidClub, unsigned long tidCaddie,
		unsigned long tidMascot); // 005e3d40: controle/precisão perdem o que a força passa do limite

	// equipados (CSharedDoc::m_myInfo.userEquip)
	int GetMyCharStat983(unsigned long type); // 005e3f60
	int GetMyEquipStat983(unsigned long type); // 005e3fc0
	int GetPartsSlot983(unsigned long type, const unsigned long* tidParts); // 005e4180 (NULL = as peças do meu)
	int GetMyPartsSlot983(unsigned long type); // 005e4260
	int GetMyCardSlot983(unsigned long type); // 005e4340
	void GetMasteryList983(MasteryList983& out, unsigned long tidChar, int level); // 005e43c0 (tid 0 = o meu)
	int GetMasterySlot983(unsigned long type, sCharacterInfo* pChar); // 005e44c0 (NULL = o meu)
	int GetMasterySlotOf983(unsigned long type, unsigned long tidChar, int level); // 005e45c0
	int GetMySlot983(unsigned long type); // 005e4690
	int GetSlotOf983(unsigned long type, const unsigned long* tidParts, unsigned long tidChar, int level); // 005e46e0
	int GetMyPCL983(unsigned long type, bool bRaw, sCharacterInfo* pChar); // 005e4740 (!bRaw: limitado ao slot)
	int GetMyOverSlot983(unsigned long type); // 005e47e0
	int GetFreeSlotOf983(unsigned long type, const unsigned long* tidParts, unsigned long tidChar, int level); // 005e4850
	int GetMyFreeSlot983(unsigned long type); // 005e48a0
	int GetStatus983(unsigned long type); // 005e48e0: base + equipamento + PCL - penalidade; 0 se type > 4
	int GetMyPenalty983(unsigned long type); // 005e4930
	// 005c0840 (corpo na unidade rfcharmasterytiledlg; até a B140 estava como RFCharMasteryTileDlg::GetStatName983 — todos os
	// chamadores passam o CStatusHelper da pilha em ecx): nome do atributo ("" fora de 0..4)
	const char* GetStatName983(unsigned int index);
};
