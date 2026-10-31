#pragma once
#include "vc10_layout.h"
// JP 983: FrGrandPrixAwardDlg (unidade grandprixawarddlg, 00546b60–0054f2fe; RTTI .?AVFrGrandPrixAwardDlg@@; vtables 00d09c8c /
// 00d09c7c / 00d09c74 e 00d09c68 (CTimerEvent em +0x150) pelo construtor 0054dc90; sizeof 0x2e0 pelo `new` de 0054f33a no
// CGrandPrixDocument). Tela de premiação do Grand Prix: pódio (char1..3 com nome), minha colocação ("myrank123"/"myrank4"),
// troféu, caixa de prêmios animada (GrandPrixAwardAnimation, `new 0x288`) e o aviso "notify_ok2". Posições em
// grandprix_award_ui.lua. Existe no 839 (par 0054f5c0…), não no 645. Sessão B72: layout e 1º lote; nomes de campos e métodos meus.

#include "formbase.h"
#include "wtimer.h"
#include "titlefontex.h"
#include <vector>
#include <string>
#include <memory>

class FrGraphicInterface;
class CSimpleUI;
class Bitmap;
class WTitleFontEx;
namespace IFF_STRUCT { struct sMatch; }
struct sGPRoomItem983;

// CGrandPrixAwardRankFont (fonte dos números da colocação; construtor 006f5480) já está em titlefontex.h (frente A)

// prêmio da lista (0xc): do registro do Grand Prix (prize/prizeExtra/prizeCount) e do GrandPrixRankReward.iff (+0xc/+0x34/+0x20)
struct sGPAwardReward983
{
	unsigned long typeId; // +0
	int kind; // +4: 0 = quantidade (CWindFont), 1 = selo "recycle_in_period"
	int count; // +8
};

// caixa de prêmios animada (0x288; construtor 0054be50, destrutor 0054c030). Fases (m_phase): 0 = prêmios chegando à
// posição "Ready" com a fumaça, 1 = ícone escurecido (0xa0202020) na caixa, 2 = caixa aberta (th_item_box_o) com a quantidade
// (CWindFont) ou o selo "recycle_in_period"; a fumaça é o "[emoti_object.png" (quadros "0".."7" de 64×64).
class GrandPrixAwardAnimation
{
public:
	struct sPos983
	{
		float x, y;
	};

	GrandPrixAwardAnimation(); // 0054be50
	~GrandPrixAwardAnimation(); // 0054c030
	void SetRewards983(const std::vector<sGPAwardReward983>& rewards); // 0054cab0
	void LoadItemLayout983(); // 00549410: grandprix_award_ui_rewardbox_item
	void SetPadding983(int x, int y); // 00547390: posição de cada prêmio (5 por linha) a partir de "th_item_box_n"
	void SetShow983(bool bShow) { m_bShow = bShow; SetPadding983((int)m_padX, (int)m_padY); } // 00547a90 (embutida na ShowMain983)
	void Draw983(FrGraphicInterface* pGDI, float dt); // 00547ca0

	void Skip983(bool bSkip); // 00546d20: pula a fase 0 ou 1 (tempo de +0x1e8 / +0x1ec)

	bool m_bShow; // +0x0
	bool m_bFinished; // +0x1: animação terminou (a FrGrandPrixAwardDlg passa ao aviso)
	std::tr1::shared_ptr<CSimpleUI> m_pSmoke; // +0x4: "[emoti_object.png"
	const Bitmap* m_pBoxN; // +0xc: "th_item_box_n"
	const Bitmap* m_pBoxO; // +0x10: "th_item_box_o"
	const Bitmap* m_slotBmp[10][3]; // +0x14: por fase (0 "th_item_mark1", 1 e 2 o ícone do item)
	const Bitmap* m_pPeriodBmp[10]; // +0x8c: "recycle_in_period" (kind 1)
	std::tr1::shared_ptr<WTitleFontEx> m_pCountFont[10]; // +0xb4: quantidade (kind 0; CWindFont)
	sPos983 m_pad983_104[3]; // +0x104: sem uso fora do construtor (que zera [0..9], passando por cima de +0x11c..+0x150)
	int m_phase; // +0x11c
	bool m_bSkip; // +0x120
	float m_time; // +0x124
	unsigned long m_color[10]; // +0x128
	int m_iconW[10]; // +0x150: tamanho do ícone (reduzido a ItemWidth × ItemHeight)
	int m_iconH[10]; // +0x178
	float m_gapX; // +0x1a0: RewardItemPaddingX
	float m_gapY; // +0x1a4
	float m_itemX; // +0x1a8: ItemPosX
	float m_itemY; // +0x1ac
	float m_offX; // +0x1b0: ItemOffsetX (itens 0x18000000)
	float m_offY; // +0x1b4
	float m_readyX; // +0x1b8: ReadyPosX
	float m_readyY; // +0x1bc
	float m_emptyX; // +0x1c0: EmptyPosX
	float m_emptyY; // +0x1c4
	float m_smokeX; // +0x1c8: SmokePosX
	float m_smokeY; // +0x1cc
	float m_digitX; // +0x1d0: ItemDigitPos
	float m_digitY; // +0x1d4
	float m_periodX; // +0x1d8: ItemPeriodImgPos
	float m_periodY; // +0x1dc
	int m_itemW; // +0x1e0: ItemWidth
	int m_itemH; // +0x1e4: ItemHeight
	float m_skipTime0; // +0x1e8: ReadyPlayTime
	float m_skipTime1; // +0x1ec: MaskPlayTime
	float m_padX; // +0x1f0
	float m_padY; // +0x1f4
	sPos983 m_itemPos[10]; // +0x1f8
	float m_smokeFrame[10]; // +0x248: quadro da fumaça (avança 15 por segundo; 8 = acabou)
	float m_elapsed; // +0x270: tempo na fase
	std::vector<sGPAwardReward983> m_rewards; // +0x274
	int m_itemCount; // +0x284: 5 ou 10
};
C_ASSERT_VC10(sizeof(GrandPrixAwardAnimation) == 0x288);

class FrGrandPrixAwardDlg : public FrFormBase, public CTimerEvent
{
	DECLARE_OBJECT(FrGrandPrixAwardDlg) // GetRTTI 0054dbf0

public:
	// controle mostrado/escondido em grupo (0x20): cond 0 = sempre, 1 = no pódio (colocação < 4), 2 = fora do pódio
	struct sCtrlShow983
	{
		sCtrlShow983(const std::string& n, int c) : name(n), cond(c) { }
		std::string name; // +0x00
		int cond; // +0x1c
	};
	// posição e escala de um número da colocação (CGrandPrixAwardRankFont::Print)
	struct sRankPos983
	{
		float x, y, scale;
	};
	// posição de um controle (lida do grandprix_award_ui.lua)
	struct sPos983
	{
		float x, y;
	};
	typedef sGPAwardReward983 sReward983;

	FrGrandPrixAwardDlg(); // 0054dc90
	virtual ~FrGrandPrixAwardDlg(); // 0054da40 (com delete: 0054f2e0)

	virtual void SetSubForm983(FrFormBase* pForm); // 00546b60: guarda a instância em s_pInstance
	virtual void ClearSubForm983(); // 00546b80
	virtual void OnTimer(); // 00549210 (pela CTimerEvent): fim da espera da caixa de prêmios

	void StartTimer983(float time, std::string name); // 00548d10: nome vazio = s_timerName
	void ShowNotify983(); // 00549d80: aviso "notify_ok2" (corpo no próximo lote: o retorno é membro do CGolfRule, 00671310)
	void OnNotifyOk983(); // 00546cd0: 3 tentativas do aviso, depois a espera final (+0x1fc)
	static bool IsMeOnPodium983(); // 00547ac0: eu entre os premiados com colocação < 4
	// 0054f470: abre a tela ("GrandPrix_Award"); chamada pelo golfrule30battle.cpp (006a57e4) quando s_pInstance é nulo
	static FrGrandPrixAwardDlg* __fastcall Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest);
	virtual bool OnInit(); // 0054d110: NÃO chama a base
	virtual bool PreOpen983(); // 0054a9f0 (slot 40): grandprix_award_ui_main e _cutscene (fundos e posições)

	static FrGrandPrixAwardDlg* s_pInstance; // 00e75198
	bool IsEnd983() const { return m_bEnd; } // B1483: o golfrule lia +0x170 cru (cl 14.44 = 0x14c)
	static std::string s_timerName; // 00e0b568

protected:
	virtual void OnProc(const float deltaTime); // 0054a2e0: NÃO chama a base

	bool TickEnd983(float dt); // 00546ba0
	bool TickTrophy983(float dt); // 00546bf0
	bool TickMain983(float dt); // 00546c40
	bool TickRetry983(float dt); // 00546c90
	void ShowMain983(bool bShow, bool bFade); // 00549a90
	void ShowTrophy983(bool bShow, bool bFade); // 00549c20
	void LoadRewardBox983(); // 00549290: grandprix_award_ui_rewardbox
	void OnCommonAreaDraw(int param); // 00547b40: números da colocação
	void OnRewardListDraw(int param); // 00548da0
	void SetRects983(const char* fmt, float x, float y, int w, int h); // 0054b900: moldura de 9 peças fmt%d (0..8) no retângulo
	void LayoutTrophy983(); // 0054baf0: "trophy", "trophy_attr%d", "trophy_other%d" centrados na tela
	void LayoutMain983(); // 0054c360: pódio, minha colocação, molduras e "rewardlist"
	void UpdateTrophy983(); // 0054c7d0: só chama a LayoutTrophy983

	bool m_bEnd; // +0x170
	std::tr1::shared_ptr<GrandPrixAwardAnimation> m_pAnim; // +0x174
	float m_originX; // +0x17c
	float m_originY; // +0x180
	sPos983 m_charPos[3]; // +0x184: "char%d" do pódio
	sPos983 m_namePos[3]; // +0x19c: "char%d_name"
	sPos983 m_myRankInPos[3]; // +0x1b4: "myrank123" por colocação ("myrank%d_pic_pos" 1..3; o 4º é m_myRankOutPos)
	sPos983 m_myRankOutPos; // +0x1cc: "myrank4"
	sRankPos983 m_myRankPos; // +0x1d4: "myrank4" (fora do pódio)
	float m_boxX; // +0x1e0
	float m_boxY; // +0x1e4
	int m_boxW; // +0x1e8
	int m_boxH; // +0x1ec
	int m_padW; // +0x1f0
	int m_padH; // +0x1f4
	float m_rewardDelay; // +0x1f8
	float m_endDelay; // +0x1fc
	sPos983 m_trophyPicPos; // +0x200: "trophy_pic_pos" (somado ao "trophy_pos")
	sPos983 m_attrPos[2]; // +0x208: "trophy_attr%d" (deslocamento a partir do centro)
	sPos983 m_otherPos[2]; // +0x218: "trophy_other%d"
	sRankPos983 m_trophyPos123; // +0x228
	sRankPos983 m_trophyPos4; // +0x234
	IFF_STRUCT::sMatch* m_pMatch; // +0x240
	float m_ratio; // +0x244
	int m_class; // +0x248
	std::string m_gpName; // +0x24c
	sGPRoomItem983* m_pNextItem; // +0x268
	std::tr1::shared_ptr<CGrandPrixAwardRankFont> m_pRankFont; // +0x26c
	std::vector<sReward983> m_rewards; // +0x274
	std::vector<sCtrlShow983> m_trophyCtrls; // +0x284
	std::vector<sCtrlShow983> m_mainCtrls; // +0x294
	int m_step; // +0x2a4: 1 = troféu, 2 = caixa de prêmios
	int m_notifyCount; // +0x2a8
	bool m_bNotify; // +0x2ac
	float m_2b0; // +0x2b0
	float m_2b4; // +0x2b4
	bool m_bShowMain; // +0x2b8
	bool m_bShowTrophy; // +0x2b9
	int m_place; // +0x2bc: minha colocação (0 = sem)
	float m_dt; // +0x2c0
	float m_trophyTime; // +0x2c4
	float m_retryTime; // +0x2c8
	float m_mainTime; // +0x2cc
	float m_endTime; // +0x2d0
	char m_2d4[0x2e0 - 0x2d4];

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00546b90; mapa 00e0b564 (41 entradas)
};
C_ASSERT_VC10(sizeof(FrGrandPrixAwardDlg) == 0x2e0);
