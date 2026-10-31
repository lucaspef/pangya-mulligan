#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetWorkshopProgressWnd (vtables 00d00f00 / 00d00edc — o Helper vem primeiro, a ChildWndBase em +0x50;
// s_pInstance 00e74288; sizeof 0x100 pelo `operator new` de 0049fde0) + FrClubSetWorkshopProgressResultWnd (vtable 00d00ed4,
// só o Helper; sizeof 0x138 pelo `operator new` de 004a3170). Janela de andamento/resultado da oficina: barra de progresso,
// efeitos do Fx e o quadro do resultado (atributo/nível/rank que subiu, com o taco voando para o lugar). Layout medido nos
// construtores 0049f720/004a2dd0 e no uso (sessão B85). A unidade do 983 vai de antes de 0049da20 (métodos pequenos entre os do
// Fx) a 004a39f0; o mapa da F3.7.1 cortava em 0049e1a0 (ResultWnd) e 0049eaa0. Nomes dos métodos e dos campos são meus.
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopfx.h"

class FrForm;
class FrWnd;
class FrGaugeBarImage;
class Bitmap;
class FrClubSetWorkshopProgressWnd;
extern const WRect g_rcZero983; // 00f14668 (0, 0, 0, 0)

// Interpolação de dois valores (x e y) de um controle; sem RTTI (unidade semclasse 004dabd2 da F3.7.1; nomes meus). sizeof 0x38.
class CTween983
{
public:
	CTween983(int pWnd); // 004db130 (0 = sem controle)
	~CTween983(); // 004dabe0: zera tudo (sem tabela virtual)
	void Attach983(FrWnd* pWnd); // 004db080: controle que anda (o construtor chama com o argumento)
	void SetX983(int ease, float from, float to, float time); // 004dac20
	void SetY983(int ease, float from, float to, float time); // 004dac60
	void Proc983(const float* pDeltaTime); // 004db180
	bool IsEnd983() const { return !m_bRunX && !m_bRunY; } // embutida (0049db20 fora de linha)

	FrWnd* m_pWnd; // +0x00
	float m_fromX; // +0x04
	float m_fromY; // +0x08
	float m_toX; // +0x0c
	float m_toY; // +0x10
	float m_timeX; // +0x14
	float m_timeY; // +0x18
	int m_easeX; // +0x1c
	int m_easeY; // +0x20
	float m_curX; // +0x24
	float m_curY; // +0x28
	float m_elapX; // +0x2c
	float m_elapY; // +0x30
	bool m_bRunX; // +0x34
	bool m_bRunY; // +0x35
	unsigned char m_pad983_36[2]; // alinhamento; 983 sem acesso (004dabe0..004db180)
};

// Quadro do resultado. sizeof 0x138.
class FrClubSetWorkshopProgressResultWnd : public ClubSetWorkshopHelper
{
public:
	enum eState { STATE_NONE, STATE_BEGIN, STATE_TEXT, STATE_MOVE, STATE_END };

	FrClubSetWorkshopProgressResultWnd(FrClubSetWorkshopProgressWnd* pParent); // 004a2dd0
	virtual ~FrClubSetWorkshopProgressResultWnd(); // 0049e1a0 (com `delete`: 0049e5b0)

	void LoadLayout983(); // 0049ed90 (chamada pela LoadLayout983 da ProgressWnd)
	void Reset983(); // 004a2030
	void Proc983(const float* pDeltaTime); // 004a1f30
	void StartMove983(); // 004a22c0: o taco e o nível/rank voam para o lugar (tipo de curva sorteado)
	void SetText983(std::string text); // 004a2670
	void SetResult983(int kind, unsigned long typeIdA, unsigned long typeIdB); // 004a2ed0 (kind 0 nível, 1 rank, 2 sorte, 3 troca)
	void Begin983(); // 0049dde0
	void SetRankTo983(float x, float y); // 0049dd40 (LoadLayout983)
	void SetLevelTo983(float x, float y, float gap); // 0049dd60 (LoadLayout983)
	bool IsBegin983(); // 0049da30 (sem chamador no 983)
	bool IsText983(); // 0049da40 (idem)
	bool IsMove983(); // 0049da50 (idem)
	bool IsEnd983(); // 0049da60 (idem)
	static float GetWaitTime983(); // 0049dcf0
	static float GetMoveTime983(); // 0049dd00

	// desenho (pelos repassadores da ProgressWnd)
	void OnDrawTitle983(); // 0049e240
	void OnDrawClubset983(); // 004a27d0
	void OnDrawLevel983(); // 004a2980
	void OnDrawRank983(); // 004a29f0

	FrClubSetWorkshopProgressWnd* m_pParent; // +0x50
	Bitmap* m_pClubBmp; // +0x54: taco (prévia do slot ou o de origem da troca)
	Bitmap* m_pClubBmp2; // +0x58: taco de destino da troca
	int m_titleOverlay[4]; // +0x5c: "img_result_title*" pelo m_title
	int m_levelOverlay[5]; // +0x6c
	int m_rankOverlay[6]; // +0x80
	int m_title; // +0x98 (−1 = nenhum; = kind do SetResult983)
	int m_state; // +0x9c (eState)
	int m_level; // +0xa0
	int m_rank; // +0xa4
	int m_stat; // +0xa8: atributo que subiu
	float m_titlePos[2]; // +0xac
	float m_luckyPos[2]; // +0xb4: "img_result_title_luckychance_pos"
	float m_textPos[2]; // +0xbc
	float m_luckyTextPos[2]; // +0xc4
	float m_clubPos[2]; // +0xcc
	float m_clubDrawPos[2]; // +0xd4
	float m_clubChangePos[2]; // +0xdc
	float m_levelPos[2]; // +0xe4
	float m_rankPos[2]; // +0xec
	float m_clubTo[2]; // +0xf4
	float m_rankTo[2]; // +0xfc
	float m_levelTo[5][2]; // +0x104
	CTween983* m_pClubTween; // +0x12c
	CTween983* m_pLevelTween; // +0x130
	float m_wait; // +0x134
};
C_ASSERT_VC10(sizeof(FrClubSetWorkshopProgressResultWnd) == 0x138);

class FrClubSetWorkshopProgressWnd : public ClubSetWorkshopHelper, public ClubSetWorkshopChildWndBase
{
public:
	enum eMode { MODE_LEVELUP, MODE_RANKUP, MODE_RANKUP_RESULT };

	static FrClubSetWorkshopProgressWnd* s_pInstance; // 00e74288
	// criador: FrClubSetWorkshopMainDlg (0049fde0)

	FrClubSetWorkshopProgressWnd(ClubSetWorkShopForm* pForm); // 0049f720
	virtual ~FrClubSetWorkshopProgressWnd(); // 0049eaa0 (com `delete`: 0049f830; ajuste da ChildWndBase 0049ebd0)
	virtual bool Init983(); // 004a3170 (Helper +4): chamado pelo OnInit da MainDlg
	virtual void Proc983(float deltaTime); // 004a3870 (Helper +8): chamado pelo OnProc da MainDlg
	void LoadLayout983(); // 004a0620: posições do clubset_workshop.lua (B86)

	// controles do mapa de mensagens da MainDlg (repassados por ela)
	void OnDrawProgressEclipse983(); // 0049dca0
	void OnDrawProgressSubwndBg983(); // 0049e160
	void OnDrawProgressAnimation983(); // 004a1860
	void OnDrawProgressbar983(); // 0049dce0 (vazia, dobrada)
	void OnWorkshopApplyBtnUp983(); // 004a1950
	void OnWorkshopCancelBtnUp983(); // 004a1b60
	void OnWorkshopOkBtnUp983(); // 004a2cf0
	void OnDrawSpeakballoonRankup983(); // 004a1d50
	void OnDrawProgressResultTitle983(); // 0049e5a0
	void OnDrawProgressResultClubset983(); // 004a2d30
	void OnDrawProgressResultLevel983(); // 004a2d40
	void OnDrawProgressResultRank983(); // 004a2d50
	bool OnApplyResult983(int result, FrForm* pForm); // 004a1e20
	bool OnCancelResult983(int result, FrForm* pForm); // 004a1e50
	bool OnApplyResultB983(int result, FrForm* pForm); // 004a1e80
	bool OnCancelConfirmResult983(int result, FrForm* pForm); // 004a1eb0

	// chamados pelas páginas (B79, nomes meus)
	void Start983(bool bStart); // 004a1ee0: false = volta (004a03e0)
	void OnSent983(); // 004a2a70: esconde apply/cancel/ok
	void Finish983(); // 004a2c80: fecha e devolve o trabalho à página (OnEndWork983)
	void SetResult983(unsigned long guid); // 004a37e0
	// B81 (chamados pela RankUp, nomes meus)
	void ShowRankUpBalloon983(bool bShow); // 004a0310: "static_speakballoon_rankup"
	void StartRankUpResult983(unsigned long oldRank, unsigned long newRank); // 004a2d60
	void SetWorking983(bool bWorking, bool bAnim); // 004a3830: false = volta (004a03e0)

	// B85
	void ResetButtons983(); // 0049ebe0
	void Back983(bool bDone); // 004a03e0
	void Update983(bool bSuccess); // 004a33f0: botões e o quadro do resultado pelo modo
	void SetFx983(int index); // 0049dc00
	bool StepGauge983(const float* pDeltaTime); // 0049db40: true = barra cheia
	bool StepTimer983(const float* pDeltaTime); // 0049dbc0 (sem chamador no 983)
	void SetLevelFlag983(bool b) { m_bfa983 = b; } // 0049da20 fora de linha (sem chamador)
	// para o quadro do resultado: controle pelo nome no form (004a00d0 FrWnd, 004a0190 FrArea, 004a0250 FrEdit)
	template <class T>
	__declspec(noinline) T* GetResultCtrl983(std::string name) { return m_pForm->GetCtrl983<T>(name); }
	const WRect& GetFormRect983() { return m_pForm ? m_pForm->GetRect() : g_rcZero983; } // embutida (0049e120 fora de linha)

	std::vector<int> m_v78; // +0x78 (LoadLayout983)
	sOverlayAnim983 m_anim; // +0x88: "img_loading_anim_filename"
	int m_animOverlay[4]; // +0xa4: figuras do "area_progress_animation"
	int m_mode; // +0xb4 (eMode; −1 = parada)
	float m_timer; // +0xb8
	float m_timerMax; // +0xbc
	FrGaugeBarImage* m_pGauge; // +0xc0: "gauegebar_progress"
	FrClubSetWorkshopProgressResultWnd* m_pResult; // +0xc4
	unsigned long m_resultGuid; // +0xc8
	unsigned long m_oldRank; // +0xcc
	unsigned long m_newRank; // +0xd0
	int m_pad983_d4; // +0xd4
	int m_balloonOverlay; // +0xd8: "area_speakballoon_rankup"
	int m_bgOverlay; // +0xdc: "area_progress_subwnd_bg"
	float m_bgPos[2]; // +0xe0
	float m_gaugeTime; // +0xe8
	float m_gaugeTimeMax; // +0xec: "animation_play_time"
	float m_pad983_f0; // +0xf0
	float m_pad983_f4; // +0xf4
	bool m_bFx0; // +0xf8
	bool m_bFx3; // +0xf9
	bool m_bfa983; // +0xfa: a LevelUp liga ao chegar o resultado
	bool m_bRunning; // +0xfb: a barra anda
	bool m_bWorking; // +0xfc
	bool m_bNoCancel; // +0xfd: o slot não pode mais cancelar (+0x60 == 0)
	unsigned char m_pad983_fe[2]; // alinhamento; 983 sem acesso (0049da20..004a3870)
};
C_ASSERT_VC10(sizeof(FrClubSetWorkshopProgressWnd) == 0x100);
