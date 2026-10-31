#pragma once
#include "vc10_layout.h"
// JP 983: alquimia de cartas ("card_compose": junta 3 cartas numa nova). UMA unidade com as quatro janelas, 00486700–0048f0a0
// (a F3.7.1 cortou em três pedaços + o fim do "semclasse_0048446e"; o pedaço 004844d0–00486700 é de outra unidade, câmera/
// tr1::shared_ptr<CameraMovingHelper::sCameraMoving>, chamada por 0068596b/00ab254f). Inicializador estático da unidade na ordem
// do fonte: FrCardComposeDlg (00cc7e90/00cc7eb0/00cc7ed0), FrCardComposeCheckDlg (00cc8280…), FrCardComposeAboutDlg (00cc8380…),
// FrCardComposeResultDlg (00cc85d0…). Mapas de mensagens 00e03948 / 00e03950 / 00e03958 / 00e03960. Existe no 839, não no 645.
// Nomes dos métodos e dos campos são meus (sessões B92: Check/About/Result; B93: FrCardComposeDlg, 0x570, CTaskMain +0x18c).
#include <string>
#include <map>
#include <list>
#include <time.h>
#include "frform.h"
#include "fx.h"
#include "../../src/shared/globalgamedefine.h"
#include "../../src/shared/classdefine.h"

class FrArea;
class FrButton;
class FrTextButton;
class FrViewer;
class FrListBox;
struct FrListItem;
class FrGraphicInterface;
class WOverlay;
class WResource;
class WTitleFont;
class Bitmap;
class FrCardComposeDlg;
class FrCardComposeCheckDlg;
class FrCardComposeResultDlg;

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// Visão do ntAvatar do 983 (avatar_avatar.cpp; a frente A tem a mesma visão em src/projectg/recycledlg.cpp): só o estado.
class ntAvatarJp983
{
public:
	void SetState(unsigned long state); // 007a4820 (manda o 0x63 quando é o meu boneco)

	unsigned char m_pad0[0x158];
	unsigned long m_state; // +0x158
};

// 00e73e80: ntAvatarManager::m_pInstance (WSingleton; visão da frente A em src/projectg/recycledlg.cpp). +0xa4 = o meu boneco,
// +0xb0 = bool zerado quando a alquimia ou a ajuda abrem. Empréstimo; nome meu. (00488d20 / 00488d40 são cópias soltas de dois
// inline desta visão, sem chamador no 983: m_pMyAvatar->SetState(m_state | x) e m_bB0 = x.)
struct sAvatarMotionJp983
{
	unsigned char m_pad0[0xa4];
	ntAvatarJp983* m_pMyAvatar; // +0xa4
	unsigned char m_padA8[0xb0 - 0xa8];
	bool m_bB0; // +0xb0
};
extern sAvatarMotionJp983* g_pAvatarMotion983; // 00e73e80
extern int g_lobbyModal983; // 00e78cac (ver descimageonlyicon.cpp)

// Regras da alquimia (objeto vazio em FrCardComposeDlg +0x168; construtor 00486700 e destrutor 00486710 vazios, fora de linha).
// As cartas vêm como os 3 typeIds escolhidos.
class CCardComposeRule983
{
public:
	CCardComposeRule983(); // 00486700
	~CCardComposeRule983(); // 00486710

	bool IsFull(const int& count); // 00486720: as 3 casas cheias
	bool HasCards(const unsigned long* tids); // 00486730 (testa o ponteiro, não as casas)
	bool NoSpecialType3(const unsigned long* tids); // 00486750: nenhuma carta do tipo 3 (bits 22–25 do typeId)
	bool NoSpecialType4(const unsigned long* tids); // 00486790: nenhuma do tipo 4
	void Release983(struct sCardComposeSel983* pSel); // 004867d0: sem chamador no 983
	void Send983(__int64 price, const unsigned long* tids); // 00486f70: pacote 0x155 { preço, 3 typeIds } e espera (msg 1)
	bool HasPang(const __int64& price); // 00487050: pang do jogador >= preço
	bool NoRare3(const unsigned long* tids); // 00487080: nenhuma carta de raridade 3 (Card.iff RareType)
	__int64 CalcPrice(const unsigned long* tids); // 004870d0: 1000 / 2000 / 5000 por carta de raridade 0 / 1 / 2
	bool HasStock(const unsigned long* tids); // 004877f0: o inventário de cartas tem as repetidas
	bool CanCompose(const int& count, const __int64& price, const unsigned long* tids); // 00487960
};

// Seleção da alquimia (FrCardComposeDlg +0x13c..+0x168). Nome meu; o 983 só a trata campo a campo.
struct sCardComposeSel983
{
	sCardComposeSel983() : m_newTid(0), m_pRes(NULL), m_unused20(0), m_count(0), m_filter(3) { } // no construtor da Dlg

	unsigned long m_newTid; // +0x00 (+0x13c): carta nova (0x325 do CTaskMain)
	unsigned long m_tid[3]; // +0x04 (+0x140): casas
	WOverlay* m_pOverlay[3]; // +0x10 (+0x14c): figura de cada casa (do cache)
	WResource* m_pRes; // +0x1c (+0x158): só liberado
	int m_unused20; // +0x20 (+0x15c): 983 só zera no ctor da Dlg (0048e467)
	int m_count; // +0x24 (+0x160)
	int m_filter; // +0x28 (+0x164): raridade da lista (3 = todas)
};

// Cache de 20 WOverlay por nome, o mais velho sai (FrCardComposeDlg +0x170). Nome meu.
class CCardOverlayCache983
{
public:
	CCardOverlayCache983(); // 00488920
	~CCardOverlayCache983() { } // 00488740 (implícito: mapa e os 20 nomes)

	void Clear983(); // 004885d0: solta as figuras e esvazia o mapa
	void Add983(const char* name, WOverlay* pOverlay); // 004889f0: na entrada mais velha (solta o WOverlay dela)
	WOverlay* Get983(const char* name); // 00488b60: do mapa; senão do WResourceManager (e guarda)

	struct sEntry
	{
		std::string name; // +0x00
		__time64_t time; // +0x20
	};
	sEntry m_entry[20]; // +0x000
	std::map<std::string, WOverlay*> m_map; // +0x320
};
C_ASSERT_VC10(sizeof(CCardOverlayCache983) == 0x330);

// Carta na lista "my_card_list" (valor do mapa por typeId, 0x1bc): o registro do inventário e a linha do Card.iff
struct sCardComposeItem983
{
	sCardComposeItem983(const sCards& card, const IFF_STRUCT::sCard& iff) : card(card), iff(iff) { } // 00488cf0 (cópia solta)
	sCards card; // +0x00
	IFF_STRUCT::sCard iff; // +0x3c
};

// Alquimia de cartas ("card_compose"): lista das minhas cartas (filtro por raridade), 3 casas, preço e pang, botão de juntar;
// "MagicBox"/"Exchange" mandam mensagens ao CTaskMain. vtables 00cff25c / 00cff24c / 00cff244; RTTI 00e73edc (?). sizeof 0x570
// pelo `operator new` de 0048e920 / 006549b5 (CTaskMain +0x18c).
class FrCardComposeDlg : public FrForm
{
	DECLARE_OBJECT(FrCardComposeDlg) // MakeInstance 0048e920; registro 0048e990

public:
	FrCardComposeDlg(); // 0048e400
	virtual ~FrCardComposeDlg(); // 0048dd20 (com `delete`: 0048e590; ajustes 0048de70 / 0048de80)

	virtual bool OnInit(); // 0048e5c0

	void SetSeq983(const int& rarity); // 0048d7c0: efeito do resultado pela raridade (0x325 do CTaskMain)
	bool SetNewCard983(const unsigned long& tid); // 00488fa0 (0x325)
	void OpenResult983(); // 0048ef20 (0x326)

protected:
	void LoadBitmaps983(); // 00488e20
	void SetPriceRect983(); // 00488e70
	void SetListBmpSize983(); // 00489d30
	void SetSlotBmpSize983(); // 00489d80
	void InitFont983(); // 0048bb90
	bool FillList983(std::list<sCards>& cards, int filter); // 0048de90
	static bool CompareItem983(const void* a, const void* b); // 00488db0
	static int TypeOrder983(const IFF_STRUCT::sCard* p); // 00488d70: ordem pelo tipo (bits 22–25 do typeId)
	int AddSlot983(const sCardComposeItem983* p); // 00488ea0: casa livre (a última) ou -1
	bool ClearSlot983(int slot); // 00488f30
	int GetLeft983(const sCardComposeItem983* p); // 00488f60: quantas ainda dá para pôr
	void SetSlotImage983(const sCardComposeItem983* p, int slot); // 0048d6f0
	void RemoveSlot983(int slot); // 00488fc0
	bool OnCheckResult983(int result, FrForm* pForm); // 00489040
	bool OnResultClose983(int result, FrForm* pForm); // 0048e230
	void OpenCheck983(); // 0048e000
	void Reset983(); // 0048e160
	void SetFilter983(int filter); // 0048e260

	void OnListInit983(int param); // 00489dd0
	void OnListDraw983(int param); // 0048bd40
	void OnListDown983(); // 0048da90
	void OnComposeInit983(int param); // 0048a1f0
	void OnComposeDown983(); // 0048e650
	void OnSlot1Init983(int param); // 0048a370
	void OnSlot2Init983(int param); // 0048a3f0
	void OnSlot3Init983(int param); // 0048a470
	void OnSlot1Up983() { RemoveSlot983(0); } // 0048a3e0
	void OnSlot2Up983() { RemoveSlot983(1); } // 0048a460
	void OnSlot3Up983() { RemoveSlot983(2); } // 0048a4e0
	void DrawSlotOverlay983(int slot); // 0048a4f0
	void DrawSlotFrame983(int slot, FrButton* pBtn, FrGraphicInterface* pGI); // 0048a580
	void DrawSlot983(int slot); // 0048a5f0
	void OnSlot1Draw983() { DrawSlot983(0); } // 0048bde0
	void OnSlot2Draw983() { DrawSlot983(1); } // 0048bdf0
	void OnSlot3Draw983() { DrawSlot983(2); } // 0048be00
	void OnAboutInit983(int param); // 0048a650
	void OnAboutDown983(); // 0048be10
	void OnWholeInit983(int param); // 0048a6c0
	void OnNormalInit983(int param); // 0048a740
	void OnRareInit983(int param); // 0048a7b0
	void OnSuperRareInit983(int param); // 0048a820
	void OnWholeDown983() { SetFilter983(3); } // 0048e740
	void OnNormalDown983() { SetFilter983(0); } // 0048e750
	void OnRareDown983() { SetFilter983(1); } // 0048e760
	void OnSuperRareDown983() { SetFilter983(2); } // 0048e770
	void OnPangAreaInit983(int param); // 00489e50
	void PrintPang983(FrGraphicInterface* pGI); // 0048a0d0
	void PrintPrice983(FrGraphicInterface* pGI); // 0048a160
	void OnPangAreaDraw983(int param); // 0048bc40
	void DrawItemIcon983(FrListItem* pItem, FrGraphicInterface* pGI, const sCardComposeItem983* p); // 0048a260
	void DrawItemName983(FrListItem* pItem, const sCardComposeItem983* p); // 0048a2c0
	void DrawItemLeft983(FrListItem* pItem, int left); // 0048a310
	void OnMagicBoxInit983(int param); // 00489ec0
	void OnMagicBoxUp983(); // 0048bc80
	void OnCardComposeInit983(int param); // 00489f70
	void OnCardComposeUp983() { } // 00488e90
	void OnExchangeInit983(int param); // 0048a020
	void OnExchangeUp983(); // 0048bce0

	FrListBox* m_pList; // +0x110: "my_card_list"
	FrButton* m_pCompose; // +0x114: "compose_card"
	FrButton* m_pSlot[3]; // +0x118: "compose_card_btn1…3"
	FrButton* m_pAbout; // +0x124
	FrArea* m_pPangArea; // +0x128: "print_pang_area"
	FrButton* m_pMagicBox; // +0x12c
	FrButton* m_pCardCompose; // +0x130
	FrButton* m_pExchange; // +0x134
	int m_unused138983; // +0x138: 983 só grava -1 no ctor (0048e451), sem leitura
	sCardComposeSel983 m_sel; // +0x13c
	CCardComposeRule983 m_rule; // +0x168
	CCardOverlayCache983 m_cache; // +0x170
	FrCardComposeCheckDlg* m_pCheckDlg; // +0x4a0
	FrCardComposeResultDlg* m_pResultDlg; // +0x4a4
	FrButton* m_pFilter[4]; // +0x4a8: "normal", "rare", "super_rare", "whole" (índice = filtro)
	const Bitmap* m_pSlotOverBmp; // +0x4b8: "cardmix_btn_slot_my_o"
	const Bitmap* m_pSlotDownBmp; // +0x4bc: "cardmix_btn_slot_my_d"
	const Bitmap* m_pListOverBmp; // +0x4c0: "cardmix_btn_list_my_o"
	WTitleFont* m_pFont; // +0x4c4
	char m_pangStr[0x20]; // +0x4c8
	char m_priceStr[0x20]; // +0x4e8
	WRect m_iconRc; // +0x508
	WRect m_listOverRc; // +0x518
	WRect m_slotRc; // +0x528
	std::map<unsigned long, sCardComposeItem983> m_items; // +0x538
	int m_topRow; // +0x548
	std::string m_seq; // +0x54c
	int m_unused568983; // +0x568: sem acesso no 983 (00488d60..0048ef20)
	int m_unused56c983; // +0x56c: sem acesso no 983

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00488d60; mapa 00e03948
};
C_ASSERT_VC10(sizeof(FrCardComposeDlg) == 0x570);

// Confirmação das 3 cartas escolhidas ("card_compose_check"): as 3 figuras e aceitar/cancelar. vtables 00cff0fc / 00cff0f0 /
// 00cff0e8; RTTI 00e73ee4. sizeof 0x170 pelo `operator new` de 0048d3a0 / 0048d0a0.
class FrCardComposeCheckDlg : public FrForm
{
	DECLARE_OBJECT(FrCardComposeCheckDlg) // GetRTTI 0048c580; MakeInstance 0048d3a0; registro 0048d410

public:
	FrCardComposeCheckDlg(); // 0048c500
	virtual ~FrCardComposeCheckDlg(); // 0048c5b0 (com `delete`: 0048d430; ajustes 0048c590 / 0048c5a0)

	void SetImages983(const std::string* names); // 0048dbb0: as 3 figuras (nome + ".png")
	void OpenImages983(); // 0048be60: abre as figuras nos visores (para no 1º visor nulo)

protected:
	void OnViewer1Init983(int param); // 0048a890
	void OnViewer2Init983(int param); // 0048a900
	void OnViewer3Init983(int param); // 0048a970
	void OnAcceptUp983() { Close(FrYES, true); } // 004890e0
	void OnCancelUp983() { Close(FrCANCEL, true); } // 004890f0

	std::string m_image[3]; // +0x110
	FrViewer* m_pViewer[3]; // +0x164: "card_compose_viewer1…3" (o construtor não zera)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004890d0; mapa 00e03950
};
C_ASSERT_VC10(sizeof(FrCardComposeCheckDlg) == 0x170);

// Ajuda da alquimia ("card_compose_about"): visor de 6 páginas (help_cardmix_01…06.jpg), "index01…06", "backward"/"forward".
// vtables 00cfef8c / 00cfef7c / 00cfef74; RTTI 00e73eec. sizeof 0x140 pelo `operator new` de 0048a9e0 / 0048b970.
class FrCardComposeAboutDlg : public FrForm
{
	DECLARE_OBJECT(FrCardComposeAboutDlg) // GetRTTI 00489170; MakeInstance 0048a9e0; registro 0048aa50

public:
	FrCardComposeAboutDlg(); // 00489110
	virtual ~FrCardComposeAboutDlg() { } // 00489180 (com `delete`: 0048aa70; ajustes 00489b10 / 00489b20)

	virtual bool OnInit(); // 0048af90: página 0, zera o g_pAvatarMotion983->m_bB0 e FrForm::OnInit

protected:
	// 004891a0: liga/desliga backward/forward nas pontas, marca o "indexNN" da página e abre a figura da m_page (não do
	// argumento: quem chama grava a m_page antes)
	void SetPage983(int page);

	void OnViewerInit983(int param); // 0048aaa0
	void OnIndex01Init983(int param); // 0048ab10
	void OnIndex02Init983(int param); // 0048aba0
	void OnIndex03Init983(int param); // 0048ac30
	void OnIndex04Init983(int param); // 0048acc0
	void OnIndex05Init983(int param); // 0048ad50
	void OnIndex06Init983(int param); // 0048ade0
	void OnBackwardInit983(int param); // 0048ae70
	void OnForwardInit983(int param); // 0048af00
	void OnIndex01Up983() { m_page = 0; SetPage983(0); } // 0048ab80
	void OnIndex02Up983() { m_page = 1; SetPage983(1); } // 0048ac10
	void OnIndex03Up983() { m_page = 2; SetPage983(2); } // 0048aca0
	void OnIndex04Up983() { m_page = 3; SetPage983(3); } // 0048ad30
	void OnIndex05Up983() { m_page = 4; SetPage983(4); } // 0048adc0
	void OnIndex06Up983() { m_page = 5; SetPage983(5); } // 0048ae50
	void OnBackwardUp983() { --m_page; SetPage983(m_page); } // 0048aee0
	void OnForwardUp983() { ++m_page; SetPage983(m_page); } // 0048af70

	FrTextButton* m_pIndex[6]; // +0x110: "index01…06"
	int m_page; // +0x128
	FrButton* m_pBackward; // +0x12c
	FrButton* m_pForward; // +0x130
	void* m_unused134983; // +0x134: sem uso visto
	FrViewer* m_pViewer; // +0x138: "about_viewer" (o construtor não zera)
	void* m_unused13c983; // +0x13c: sem uso visto

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00489100; mapa 00e03958
};
C_ASSERT_VC10(sizeof(FrCardComposeAboutDlg) == 0x140);

// Resultado ("result_card_compose"): a carta nova num visor que aparece depois de um efeito (.seq) tocado numa caixa de efeitos
// própria; o "ok" só liga no fim. vtables 00cff374 / 00cff368 / 00cff360; RTTI 00e73ef4. sizeof 0x5c0 pelo `operator new` de
// 0048efd0 / 0048ee50. Criada pela FrCardComposeDlg (0048ef20, pedido do CTaskMain).
class FrCardComposeResultDlg : public FrForm
{
	DECLARE_OBJECT(FrCardComposeResultDlg) // GetRTTI 0048ec80; MakeInstance 0048efd0; registro 0048f040

public:
	FrCardComposeResultDlg(); // 0048ecb0
	virtual ~FrCardComposeResultDlg(); // 0048ebc0 (com `delete`: 0048ee20; ajustes 0048ec90 / 0048eca0)

	void SetCard983(const unsigned long& typeId); // 0048d850: figura da carta (Card.iff Image + ".png")
	void PlayFx983(const char* seqName); // 0048b380: efeito no centro do visor
	void SetOwner983(FrCardComposeDlg* pOwner); // 00489260
	void ShowCard983(); // 0048bea0: abre a figura e liga o relógio

protected:
	virtual void OnProc(const float deltaTime); // 0048b0e0: efeito; visor aos 2 s e "ok" aos 4 s do relógio

	void OnCardInit983(int param); // 0048afc0
	void OnFxPlayerInit983(int param); // 0048b030: área do efeito = retângulo do "fx_player" (sem guardar a janela)
	void OnFxPlayerDraw983(); // 0048b1c0: câmera, campo de visão e corte próprios; devolve os da tela
	void OnOkInit983(int param); // 0048b420
	void OnOkUp983() { OnFreshOkay(); } // 00489270

	std::string m_image; // +0x110
	FrViewer* m_pCard; // +0x12c: "synthesized_card" (o construtor não zera)
	FrButton* m_pOk; // +0x130: "ok" (o construtor não zera)
	CPvsFxBox m_fxBox; // +0x134 (0x440)
	WMatrix m_camera; // +0x574
	WRect m_rc; // +0x5a4: área do "fx_player"
	FrCardComposeDlg* m_pOwner; // +0x5b4
	float m_timer; // +0x5b8: -1 parado; 1 ao abrir a figura
	int m_unused5bc983; // +0x5bc: sem uso visto

	DECLARE_FRESH_MSGMAP() // GetMessageMap 00489250; mapa 00e03960
};
C_ASSERT_VC10(sizeof(FrCardComposeResultDlg) == 0x5c0);
