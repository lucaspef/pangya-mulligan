#pragma once
// JP 983: _gateway::FrHoleinoneMissionDlg (unidade 0057e8f0–00580330; vtables 00d0acdc / 00d0acd0 / 00d0acc8; sizeof 0x1b0 pelo
// `new` de 0057fd70). Painel lateral do modo hole-in-one: lista das missões (itens do CHoleInOneGameDoc) com o prêmio em "%N" e a
// marca de cumprida. Layout pelo construtor 0057fb30 e pelos métodos (sessão B26). Nomes dos campos e dos métodos são meus.

#include "frform.h"
#include <vector>

class FrListBox;
class Bitmap;
class CFontInfo;
namespace IFF983 { struct sCupInCondition; }

namespace _gateway
{
class FrHoleinoneMissionDlg : public FrForm
{
	DECLARE_OBJECT(FrHoleinoneMissionDlg)

public:
	// Deslize/desvanecimento de uma peça (0x2c). O diálogo usa um (posição da janela) e cada linha da lista outro (só m_state,
	// m_alpha e m_text).
	struct sSlide983
	{
		unsigned int m_state; // +0x00: diálogo 0 parado, 1 aparecendo, 2 abrindo, 3 fechando; linha: bits 4 ativa, 2 subindo, 1 descendo
		float m_fromX; // +0x04
		float m_fromY; // +0x08
		float m_toX; // +0x0c
		float m_toY; // +0x10
		float m_curX; // +0x14
		float m_curY; // +0x18
		float m_time; // +0x1c
		float m_alpha; // +0x20
		char m_text[8]; // +0x24: "%N" do prêmio (só nas linhas)
	};
	struct sRow983 // 0x30
	{
		IFF983::sCupInCondition* m_pItem; // +0x00
		sSlide983 m_slide; // +0x04
	};
	struct sPos983
	{
		float x;
		float y;
	};
	typedef std::vector<sRow983> RowList;
	typedef std::vector<IFF983::sCupInCondition*> ItemList;

	FrHoleinoneMissionDlg(); // 0057fb30
	virtual ~FrHoleinoneMissionDlg(); // 0057fc30

	void Show983(); // 0057ed60 (chamada de 0074c0a2)
	void SetCleared983(const std::vector<unsigned long>& ids); // 0057f730: marca (estado 6) a linha de cada typeId

protected:
	virtual void OnDraw(); // 0057f450
	virtual void OnProc(const float deltaTime); // 0057eff0
	virtual bool OnInit(); // 0057fe30

	float Alpha983() const { return m_wndAlpha; } // embutida no 983 (cópia de +0x80 num temporário); FrWnd::GetAlpha da frente A é fora de linha
	void UpdateSlide983(sSlide983* pSlide, float deltaTime); // 0057e900
	void DrawBg983(); // 0057ebb0
	void StartSlide983(bool bShow); // 0057ece0
	void InitSlide983(sSlide983* pSlide, int a, int index, int b); // 0057eda0 (os três inteiros não são lidos)
	void OnClickListInit(int param); // 0057ede0
	void OnClickList(); // 0057f800

	const Bitmap* m_pBg[4]; // +0x110: bg_img1, bg_img2, bg_img3, bg_img5
	sPos983 m_bgPos[4]; // +0x120: bg_img1_pos, bg_img2_pos, bg_img3_pos (o 4º fica zerado)
	float m_movingTime; // +0x140
	float m_innerOffsetY; // +0x144
	float m_hideOffsetX; // +0x148
	float m_textWidth; // +0x14c: maior largura de nome de missão
	CFontInfo* m_pFont; // +0x150
	unsigned int m_prevState; // +0x154: último estado do jogo visto (CGolfDoc do 983 +0x158)
	sSlide983 m_slide; // +0x158
	bool m_bShown; // +0x184
	int m_188; // +0x188 (só zerado)
	FrListBox* m_pList; // +0x18c: "clicklist"
	ItemList* m_pMissions; // +0x190: m_items do CHoleInOneGameDoc
	RowList m_rows; // +0x194
	char m_pad1a4[0x1b0 - 0x1a4]; // sem leitor nem gravador nesta unidade

	DECLARE_FRESH_MSGMAP()
};
}
