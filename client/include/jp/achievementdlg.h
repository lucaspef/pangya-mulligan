#pragma once
#include "vc10_layout.h"
// JP 983: janela das conquistas "ach_main" (_gateway::_questoneday::FrAchievement, RTTI m_RTTI 00e74db4, vtable 00d05040,
// 0x7a0 pelo `new` de 004ea710/005ee5e0). Unidade 004e11b0–004ea7a0 (a F3.7.1 cortava em ahievementpainter 004e11b0–004e13a0 e
// achievement_p2 004e13a0–004ea7a0; é uma só: as vtables dos pintores e o mapa de mensagens 00cca5a0 são do mesmo módulo).
// 9 abas ("btn_summary" … "btn_event" → categorias −1, 1, 0, 3, 2, 4, 6, 5, 7 do Achievement.iff), resumo com as 6 conquistas
// mais recentes ("list_ach_recent"), lista das conquistas da categoria ("list_ach_element") e das peças de uma conquista
// ("list_ach_item"), 8+4+1 medidores (CGaugeBar983). Desenho por três "pintores" aninhados (CAhievementPainter, com o erro de
// grafia do 983). Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B102).
#include "frform.h"
#include "gaugebar983.h"
#include <list>
#include <map>
#include <string>
#include <vector>

class FrArea;
class FrButton;
class FrListBox;
struct FrListItem;
class Bitmap;
class CAchieveTotalPointFont;
class CAchieveUnitEarnPoint;
class CAchievementDate;

namespace IFF983
{
struct sAchievement;
struct sQuestStuff;
}

namespace _achievement
{
class CAchievement;
}

namespace _gateway
{
namespace _questoneday
{
class CQuestDocument;

// prêmio de uma peça da conquista (0x10): as 3 quantidades do QuestStuff.iff (+0xf4/+0xf8/+0xfc) e se há mais de uma
struct sAchPrize983
{
	int count[3]; // +0x0
	int bMulti; // +0xc: mais de uma quantidade não nula
};

// conquista mostrada na lista "list_ach_element" (valor do mapa +0x63c, 0x4c), montada pelo SortElements983 (004e8c20)
struct sAchElement983
{
	sAchElement983()
	{
		pAch = NULL;
		pIff = NULL;
		pStuff = NULL;
		m_bOpen = 0;
		points = 0;
		maxPoints = 0;
		counter = 0;
		progress = 0.0f;
		height = 0.0f;
		memset(&date, 0, sizeof(date));
	}

	_achievement::CAchievement* pAch; // +0x00: conquista do jogador (NULL = nenhuma peça ainda)
	const IFF983::sAchievement* pIff; // +0x04
	const IFF983::sQuestStuff* pStuff; // +0x08: peça atual (QuestStuff.iff; meta em +0xd4)
	int m_bOpen; // +0x0c: a lista de peças ("list_ach_item") está aberta para esta conquista
	int points; // +0x10: pontos ganhos (CQuestDocument::GetPoint983)
	int maxPoints; // +0x14: pontos possíveis (sAchievementInfo983::points)
	int counter; // +0x18: índice da peça atual (−1 = nenhuma)
	int lastCounter; // +0x1c: último contador cumprido (−1 = nenhum)
	float progress; // +0x20: 0..100 da peça atual
	float height; // +0x24: altura da lista de peças aberta
	SYSTEMTIME date; // +0x28: quando cumpriu o contador mais recente
	const Bitmap* pBitmap; // +0x38: figura (nome em Achievement.iff +0x49)
	std::vector<sAchPrize983> prizes; // +0x3c: um por peça
};

// conquista recente do resumo (6 × 0x1c em +0x670), montada pelo BuildRecent983 (004e7bb0): nome = pAch->names[nameIdx],
// data do contador
struct sAchRecent983
{
	const IFF983::sAchievement* pAch; // +0x00
	int nameIdx; // +0x04
	const Bitmap* pBitmap; // +0x08: figura (Achievement.iff +0x49)
	SYSTEMTIME date; // +0x0c
};

class FrAchievement : public FrForm
{
	DECLARE_OBJECT(FrAchievement) // GetRTTI 004e9930 (m_RTTI 00e74db4); MakeInstance 004ea710 (`new` 0x7a0); registro 00cca580

public:
	// pintor de uma área da janela (0xc, vtable 00d04f18): dono e "mostrar"
	class CAhievementPainter
	{
	public:
		CAhievementPainter(FrAchievement* pOwner) : m_pOwner(pOwner), m_bShow(false) { } // 004e11b0
		virtual ~CAhievementPainter() { } // 004e1290 (com `delete`)
		virtual void Draw(int param) = 0;
		void SetShow983(bool show) { m_bShow = show; } // 004e11e0

		FrAchievement* m_pOwner; // +0x4
		bool m_bShow; // +0x8
	};
	// resumo ("area_summary", vtable 00d04f24)
	class CAhievementMainContents : public CAhievementPainter
	{
	public:
		CAhievementMainContents(FrAchievement* pOwner) : CAhievementPainter(pOwner) { } // 004e1200
		virtual void Draw(int param); // 004e5230: fundo, pontos totais, 6 recentes, pontos por categoria
		void DrawFrame983(); // 004e31e0: 7 figuras do fundo (m_bitmaps[0..6]) e os 8 medidores das categorias
	};
	// linha da conquista ("list_ach_element", vtable 00d04f30)
	class CAhievementDetailInfo : public CAhievementPainter
	{
	public:
		CAhievementDetailInfo(FrAchievement* pOwner) : CAhievementPainter(pOwner) { }
		virtual void Draw(int param); // 004e3550: param NULL = cabeçalho da categoria; senão a FrListItem da linha
	};
	// peças da conquista aberta ("area_element_layer", vtable 00d04f3c)
	class CAhievementPopupInfo : public CAhievementPainter
	{
	public:
		CAhievementPopupInfo(FrAchievement* pOwner) : CAhievementPainter(pOwner) { }
		virtual void Draw(int param); // 004e3e10: param = FrListItem da linha aberta
	};

	enum { TAB_COUNT = 9, CATEGORY_COUNT = 8, RECENT_COUNT = 6 };
	typedef std::map<unsigned long, sAchElement983> ElementMap;

	FrAchievement(); // 004ea420
	virtual ~FrAchievement(); // 004e96d0 (com `delete`: 004ea6e0)

	virtual bool OnInit(); // 004e9960

	void SelectTab983(int tab); // 004e92d0: botões, categoria, lista das conquistas e o medidor total
	void UpdateCategoryGauges983(); // 004e1450: pontos por categoria (documento) nos 8 medidores
	void BuildRecent983(); // 004e7bb0: as 6 conquistas mais recentes do resumo
	void LoadElements983(int category); // 004e8100: conquistas da categoria no mapa +0x63c
	void SortElements983(); // 004e8c20: m_elements montado de m_iffs (conquista do jogador, peça atual, progresso, data, prêmios)

protected:
	void SetCategory983(int category); // 004e2930: −1 = resumo (pintor 0), 0..7 = lista (pintores 1 e 2)
	void CloseItem983(FrListItem* pItem); // 004e29b0: fecha a lista de peças da linha
	void PlaceItemList983(FrListItem* pItem); // 004e29f0: põe "list_ach_item" ao lado da linha
	int FindItemColumn983(float width, const WPoint& pt); // 004e2ac0: figura de "list_ach_item" sob o ponto (−1 = nenhuma; até 10)
	int FindRecentRow983(const WPoint& pt); // 004e14f0: linha do resumo sob o ponto (−1 = nenhuma)

	void OnSummaryBtnInit983(int param); // 004e2b80
	void OnCharBtnInit983(int param); // 004e2bf0
	void OnGameModeBtnInit983(int param); // 004e2c60
	void OnAdventureBtnInit983(int param); // 004e2cd0
	void OnStatisticsBtnInit983(int param); // 004e2d40
	void OnItemBtnInit983(int param); // 004e2db0
	void OnSystemBtnInit983(int param); // 004e2e20
	void OnChallengeBtnInit983(int param); // 004e2e90
	void OnEventBtnInit983(int param); // 004e2f00: escondido
	void OnSummaryBtnUp983() { SelectTab983(0); } // 004e9580
	void OnCharBtnUp983() { SelectTab983(1); } // 004e9590
	void OnGameModeBtnUp983() { SelectTab983(2); } // 004e95a0
	void OnAdventureBtnUp983() { SelectTab983(3); } // 004e95b0
	void OnStatisticsBtnUp983() { SelectTab983(4); } // 004e95c0
	void OnItemBtnUp983() { SelectTab983(5); } // 004e95d0
	void OnSystemBtnUp983() { SelectTab983(6); } // 004e95e0
	void OnChallengeBtnUp983() { SelectTab983(7); } // 004e95f0
	void OnEventBtnUp983() { SelectTab983(8); } // 004e9600
	void OnSummaryAreaInit983(int param); // 004e2f70
	void OnSummaryAreaDraw983(int param); // 004e2fe0
	void OnElementLayerInit983(int param); // 004e3000
	void OnElementLayerDraw983(int param); // 004e6810
	void OnRecentListInit983(int param); // 004e3070
	void OnRecentListMove983(); // 004e68b0: dica com o nome e a data da conquista sob o cursor
	void OnElementListInit983(int param); // 004e30e0
	void OnElementListDraw983(int param); // 004e3150
	void OnElementListUp983(); // 004e6ed0
	void OnItemListInit983(int param); // 004e3170: escondida
	void OnItemListMove983(); // 004e69d0

public:
	int m_pad983_110; // +0x110: sem uso (não iniciado pelo construtor)
	FrButton* m_pTabBtn[TAB_COUNT]; // +0x114: "btn_summary" … "btn_event"
	FrListBox* m_pElementList; // +0x138: "list_ach_element"
	FrListBox* m_pItemList; // +0x13c: "list_ach_item"
	FrListBox* m_pRecentList; // +0x140: "list_ach_recent"
	FrArea* m_pSummaryArea; // +0x144: "area_summary"
	FrArea* m_pElementLayer; // +0x148: "area_element_layer"
	const Bitmap* m_bitmaps[25]; // +0x14c: tabela "ach_bitmap_list" de achievement.lua
	const Bitmap* m_challengeText[CATEGORY_COUNT]; // +0x1b0: "Challenge_text_%d"
	float m_pos[16][2]; // +0x1d0: tabela "ach_position" (pares x, y)
	CGaugeBar983 m_categoryGauge[CATEGORY_COUNT]; // +0x250: "challenges_guage1"
	CGaugeBar983 m_gauge4b0[4]; // +0x4b0: "challenges_guage3"
	CGaugeBar983 m_totalGauge; // +0x5e0: "challenges_guage2" (pontos da categoria aberta)
	std::vector<IFF983::sAchievement*> m_iffs; // +0x62c: Achievement.iff da categoria aberta (LoadElements983)
	ElementMap m_elements; // +0x63c: conquistas da categoria aberta, pelo typeid
	std::vector<CAhievementPainter*> m_painters; // +0x64c: resumo, linha, peças
	std::list<FrListItem*> m_openItems; // +0x65c: linhas com a lista de peças aberta
	int m_category; // +0x668: −1 = resumo
	int m_mode; // +0x66c: 0 = resumo, 1 = lista
	sAchRecent983 m_recent[RECENT_COUNT]; // +0x670
	float m_recentRows[RECENT_COUNT][2]; // +0x718: faixas das linhas do resumo
	int m_unused748983; // +0x748: o 983 só zera no ctor (004ea655), sem leitor
	int m_itemTipColumn; // +0x74c: peça da dica de "list_ach_item" (−1 = refazer)
	int m_recentCount; // +0x750
	int m_categoryPoints[CATEGORY_COUNT]; // +0x754: pontos do jogador por categoria
	CAchieveTotalPointFont* m_pTotalPointFont; // +0x774
	CAchieveUnitEarnPoint* m_pUnitPointFont; // +0x778
	CAchievementDate* m_pDateFont; // +0x77c
	std::string m_totalPoint; // +0x780: pontos de conquista do jogador ("%d")
	CQuestDocument* m_pDoc; // +0x79c: gravado pelo CQuestHandler antes do OnInit

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004e13a0; mapa 00e0708c (29 entradas), inicializador 00cca5a0
};
}
}
C_ASSERT(sizeof(_gateway::_questoneday::sAchPrize983) == 0x10);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::sAchElement983) == 0x4c);
C_ASSERT(sizeof(_gateway::_questoneday::sAchRecent983) == 0x1c);
C_ASSERT(sizeof(_gateway::_questoneday::FrAchievement::CAhievementPainter) == 0xc);
C_ASSERT_VC10(sizeof(_gateway::_questoneday::FrAchievement) == 0x7a0);
