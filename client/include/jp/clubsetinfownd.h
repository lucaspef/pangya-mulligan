#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetInfoWnd (unidade 004bd8e0–004c0690; RTTI .?AVFrClubSetInfoWnd@@; vtables 00d027c4 (Helper) / 00d027a0
// (ChildWndBase)). Quadro de dados do taco escolhido (nome, tipo, 5 atributos com o máximo, experiência, cancelamentos, rank
// e nível em ícones; tabela "clubset_workshop_ui_info") dentro das páginas "subir nível" (FrClubSetLevelUpWnd +0x88) e "subir
// rank" (FrClubSetRankUpWnd), que criam o quadro nas suas LoadLayout983 (004ec7b0, 004c86e0). Como a
// FrClubSetWorkshopProgressWnd: o Helper vem primeiro, a ChildWndBase em +0x50.
// Layout medido no construtor 004bf850, no destrutor 004be7e0 e nas leituras do Lua (sessão B84). Nomes dos métodos e dos
// campos são meus.
#include <string>
#include <vector>
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopfx.h"

class FrStatic;
class FrButton;
namespace lua_system { class lua_script; }

const char* __fastcall ClubSetTypeName983(int type); // 004a5310 (unidade da ProgressWnd): nome do tipo do taco

class FrClubSetInfoWnd : public ClubSetWorkshopHelper, public ClubSetWorkshopChildWndBase
{
public:
	// figura do quadro: posição relativa ao canto (m_x, m_y), nome e índice no Helper. 0x28 (construtor 004be770,
	// destrutor 004be320 pelo `eh_vector_*_iterator` de 45)
	struct sItem983
	{
		sItem983() : x(0), y(0), name(""), overlay(-1) { }
		int x; // +0
		int y; // +4
		std::string name; // +8
		int overlay; // +0x24
	};
	enum
	{
		ITEM_CLUBNAME_BG = 0, // clubname_bg
		ITEM_STAT_BG = 1, // 1…5 stat_bg, 6…8 clubtype_bg
		ITEM_LETTER = 9, // 9…16 letter_filename0…7
		ITEM_LETTER_STRONG = 17, // 17…21 letter_strong_filename0…4 (atributo em destaque)
		ITEM_RANK = 22, // 22…27 rank_icon_e…s
		ITEM_LEVEL = 28, // 28…32 level_icon_n (ligados até o nível)
		ITEM_LEVEL_D = 33, // 33…37 level_icon_d (acima do nível)
		ITEM_SELECT_N = 38, // clubset_select_n
		ITEM_SELECT_O = 39, // clubset_select_o (taco escolhido)
		ITEM_ARROW = 41, // 41 seta do atributo escolhido, 42 seta do rank máximo
		ITEM_SELECT_TEXT = 43, // "escolha" piscando
		ITEM_SRANK_BORDER = 44, // borda do rank S (desenhada fora do laço)
		ITEM_COUNT = 45
	};

	// 004bf850 (B80: argumentos pela LoadLayout983 da LevelUp: os 14 FrStatic "static_*_levelup" e o form, que vai a +0x54)
	FrClubSetInfoWnd(const std::vector<FrStatic*>& statics, ClubSetWorkShopForm* pForm);
	virtual ~FrClubSetInfoWnd(); // 004be7e0 (com `delete`: 004bf940; ajuste da ChildWndBase 004be8e0)

	virtual bool Init983(); // 004bd8e0 (Helper +4)
	virtual void Proc983(float deltaTime); // 004bd8f0 (Helper +8): pisca o "escolha" sem taco
	virtual void SetShow983(bool bShow); // 004bd9b0 (ChildWndBase +4): só repassa à base

	void Update983(bool bLevelUp, int unused983 = -1); // 004c03d0 (2º argumento sempre −1, não usado; também chamada pela ProgressWnd 004a33f0)
	void SetMode983(int mode); // 004c03b0: +0x7f0 (0 = escolher, 1 = trabalhando)
	void Reset983(bool bLevelUp); // 004c0620: sem taco
	void SetSelStat983(int stat); // 004bdb30: +0x7ec = atributo escolhido (−1 não muda); o taco mostrado vira a prévia do Fx
	void Draw983(); // 004bdd50: figuras, molduras e a figura do taco
	void SetSelectBtn983(FrButton* pBtn); // 004bd9c0: +0x7c (B80)
	void LoadLayout983(int unused); // 004c0070: tabela "clubset_workshop_ui_info" (a LevelUp passa 0; não usado)
	void SetPos983(int x, int y) { m_x = (float)x; m_y = (float)y; } // embutida (B80: a LevelUp põe a posição do "area_clubinfo_*")

	// B85: a FrClubSetWorkshopProgressWnd e o quadro do resultado leem m_pInfoWnd / m_selStat e chamam SelectClub983
	friend class FrClubSetWorkshopProgressWnd;
	friend class FrClubSetWorkshopProgressResultWnd;

protected:
	void UpdateMode983(); // 004bd9d0: botão de escolher, "escolha", borda do rank S
	void UpdateRankLevel983(); // 004bda30: ícones de rank e nível
	void UpdateStats983(); // 004bded0: setas e textos dos atributos
	void LoadLayoutBg983(lua_system::lua_script* script); // 004be8f0: fundos, letras, setas
	void LoadLayoutTexts983(lua_system::lua_script* script); // 004be350: posição dos 14 textos
	void LoadLayoutSelect983(lua_system::lua_script* script); // 004bf240: botão de escolher, ícone do taco, borda do rank S
	void LoadLayoutLevel983(lua_system::lua_script* script); // 004bf970: moldura, rank e nível
	void UpdateSelectBtnTip983(); // 004c0210: dica do "btn_clubselect_*" = nome do taco
	// embutida: nome, posição e figura de um item (as leituras do Lua todas chamam assim; bUse = começa visível)
	void SetItemPos983(int index, int x, int y) { m_items[index].x = x; m_items[index].y = y; } // embutida (seta: y antes de x)
	__forceinline void SetItem983(int index, const char* name, int x, int y, bool bUse)
	{
		sItem983& item = m_items[index];
		item.name = name;
		item.x = x;
		item.y = y;
		item.overlay = LoadOverlay983(name, bUse);
	}

	ClubSetWorkshopFx983::sSlot983* m_pSlot; // +0x78: taco mostrado (slot do Fx ou a prévia do 004ce620)
	FrButton* m_pSelectBtn; // +0x7c: "btn_clubselect_*" da página
	float m_x; // +0x80
	float m_y; // +0x84
	sItem983 m_items[ITEM_COUNT]; // +0x88
	std::vector<int> m_frames; // +0x790: molduras (Helper) desenhadas no canto
	std::vector<FrStatic*> m_statics; // +0x7a0: 0 nome, 1…5 atributos, 6 tipo, 7 experiência, 8 cancelamentos, 9…13 máximos
	int m_iconX; // +0x7b0: figura do taco (clubset_icon_pos)
	int m_iconY; // +0x7b4
	float m_arrowPos[5][2]; // +0x7b8: arrow_icon_pos0…4
	int m_pad983_7e0[3]; // +0x7e0 (sem uso na unidade)
	int m_selStat; // +0x7ec: atributo escolhido (−1 = nenhum; não iniciado no construtor)
	int m_mode; // +0x7f0
	float m_blinkTime; // +0x7f4
	bool m_bBlink; // +0x7f8
	bool m_bLevelUp; // +0x7f9: true na LevelUp, false na RankUp
};
C_ASSERT_VC10(sizeof(FrClubSetInfoWnd) == 0x7fc); // `operator new` de 0x7fc na LoadLayout983 004ec7b0 da LevelUp
