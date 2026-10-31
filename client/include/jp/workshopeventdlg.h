#pragma once
#include "vc10_layout.h"
// JP 983: _gateway::FrWorkShopEventDlg ("WorkShopEvent2013"), a janela do evento da oficina de tacos de 2013: buracos jogados
// pelo jogador (barra pessoal, 30 por caixa) e por todos (barra geral até a meta), botão da caixa aleatória (pacote 0x173 com o
// número de caixas), olho piscando do boneco, 12 fundos "WorkShopEvent_bgN" que trocam para "event_club_bNN" quando a meta geral é
// alcançada, e ajuda ("BirthdayEventHelp" com "workshop_event_helpimage_jp.jpg"). Unidade 0082e940–0082fcd0 (vtables 00d30154 /
// 00d30148 / 00d30140; RTTI 00e7c6d8; mapa de mensagens 00e205c8 com 26 entradas montado pelo inicializador 00cdcda0; registro
// 00cdcd80). sizeof 0x190 pelo `operator new` de 0082ec20 e do CreateForm 0082e350 do handler (jp/workshopevent2013handler.cpp).
// Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B126).
#include "frform.h"

class FrArea;
class FrButton;
class WOverlay;
class WTitleFontEx;
class CGaugeBar983;

namespace _gateway
{
class FrWorkShopEventDlg : public FrForm
{
	DECLARE_OBJECT(FrWorkShopEventDlg) // GetRTTI 0082ea70; MakeInstance 0082ec20; registro 00cdcd80

public:
	enum { HOLES_PER_BOX = 30, BG_COUNT = 12 };

	FrWorkShopEventDlg(); // 0082e990
	virtual ~FrWorkShopEventDlg(); // 0082ecb0 (com `delete`: 0082fa30; ajustes 0082ec00 / 0082ec10)

	virtual bool OnInit(); // 0082fa60

	void SetMyHoles983(int count); // 0082e940 (sem chamador)
	void SetOpenedBoxes983(int count); // 0082e950 (sem chamador)
	void SetTotalHoles983(int count); // 0082e960 (sem chamador)
	void SetTotalGoal983(int count); // 0082e970 (sem chamador)
	// 0082edd0: buracos do jogador, caixas já abertas, buracos de todos, meta geral (só se > 0)
	void SetData983(int myHoles, int openedBoxes, int totalHoles, int totalGoal);
	// 0082ee30: (com bSet) SetData983; barras, botão da caixa e fundos de meta alcançada
	void Update983(bool bSet, int myHoles, int openedBoxes, int totalHoles, int totalGoal);

protected:
	virtual void OnProc(const float deltaTime); // 0082ed90: barras e olho

	void UpdateEyeBlink983(float deltaTime); // 0082ea80: 5 s aberto, 0,2 s em cada quadro "event_club_eye1/2"
	void ShowGoalBg983(); // 0082eb30: fundos "event_club_b01".."b12"; some o texto da contagem
	void CalcBoxCount983(); // 0082eb90: caixas a abrir = buracos / 30 − abertas
	void ShowGoalBgIfDone983(); // 0082ee20

	void OnRandomBoxBtnInit(int param); // 0082eee0
	void OnMyBarInit(int param); // 0082ef60
	void OnTotalBarDraw(int param); // 0082f020: barra geral, ou "event_club_complete-progress.tga" com a meta alcançada
	void OnBoxCountInit(int param); // 0082f0e0
	void OnBoxCountDraw(int param); // 0082f150
	void OnMyHoleCountInit(int param); // 0082f1c0
	void OnMyHoleCountDraw(int param); // 0082f230
	void OnEyeBlinkInit(int param); // 0082f2a0
	void OnHoleCountTextInit(int param); // 0082f340
	void OnBg1Init(int param); // 0082f3b0
	void OnBg2Init(int param); // 0082f420
	void OnBg3Init(int param); // 0082f490
	void OnBg4Init(int param); // 0082f500
	void OnBg5Init(int param); // 0082f570
	void OnBg6Init(int param); // 0082f5e0
	void OnBg7Init(int param); // 0082f650
	void OnBg8Init(int param); // 0082f6c0
	void OnBg9Init(int param); // 0082f730
	void OnBg10Init(int param); // 0082f7a0
	void OnBg11Init(int param); // 0082f810
	void OnBg12Init(int param); // 0082f880
	void OnHelpInit(int param); // 0082f8f0
	void OnRandomBoxBtnClick(); // 0082fba0: pacote 0x173 (caixas a abrir)
	void OnHelpClick(); // 0082fc70

	void OnMyBarDraw(int param); // 0082ebd0
	void OnTotalBarInit(int param); // 0082ebf0 (vazia)

	int m_myHoles; // +0x110
	int m_totalHoles; // +0x114
	int m_totalGoal; // +0x118
	int m_openedBoxes; // +0x11c
	int m_boxCount; // +0x120: caixas a abrir
	float m_eyeSpeed; // +0x124 (2,0)
	float m_eyeTime; // +0x128
	int m_eyeFrame; // +0x12c: 0 aberto, 1..2 "event_club_eye%d"
	bool m_bGoalDone; // +0x130: meta geral alcançada
	FrButton* m_pRandomBoxBtn; // +0x134: "workshop_random_box_btn"
	CGaugeBar983* m_pMyGauge; // +0x138: "myhole_count_bar_bg_area"
	CGaugeBar983* m_pTotalGauge; // +0x13c: "total_hole_count_bar_bg_area"
	WOverlay* m_pCompleteOverlay; // +0x140
	FrArea* m_pBg[BG_COUNT]; // +0x144: "WorkShopEvent_bg1".."bg12"
	FrArea* m_pEyeBlink; // +0x174: "workshop_eye_blink"
	FrArea* m_pHoleCountText; // +0x178: "workshop_holecount_text_area"
	FrArea* m_pBoxCountArea; // +0x17c: "randombox_reward_num_area"
	FrArea* m_pMyHoleCountArea; // +0x180: "myhole_count_area"
	WTitleFontEx* m_pFont; // +0x184: CClubSetWorkShopEventFont
	FrButton* m_pHelpBtn; // +0x188: "help"
	int m_unused18c; // +0x18c: sem acesso no 983 (nada em 0082e940..0082fe00, nem o construtor)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0082e980; mapa 00e205c8
};
}
C_ASSERT_VC10(sizeof(_gateway::FrWorkShopEventDlg) == 0x190);
