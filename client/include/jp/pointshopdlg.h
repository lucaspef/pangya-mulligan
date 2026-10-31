#pragma once
#include "vc10_layout.h"
// JP 983: loja de pontos do Tiki ("TP"). UMA unidade 0084b8d0–008515c0 com quatro janelas; ordem do fonte pelo inicializador
// estático (00cddf10…00cde660): FrPointShopHelp (mapa 00e21c10), FrPointShopMessageDlg (00e21c18), FrPointShopNoticeDlg
// (00e21c20) e FrPointShopDlg (00e21c28, 0x1c0, com a CPointShop embutida em +0x1a0). A lógica fica em pointshop.cpp (unidade
// seguinte). Gêmea da quebra de MP (alchemympbreakdlg.h). Quem abre é o CTaskMain (frente A): 00659360 manda o pacote 0x127 e
// cria a "PointShopDlg" com o resultado 00651260 (mensagem 0x2e0); 0x2e1 abre a "PointShopMessageDlg" (00655680 +
// SetMessage983 0084e8d0); 0x2e2 → 00651030 → Refresh983 008515a0. Existe no 839, não no 645 (lá pointshop.cpp e
// frpointshopdlg.cpp estão vazios). Nomes dos métodos e dos campos são meus (sessão B106).
#include <list>
#include <map>
#include <string>
#include "frform.h"
#include "pointshop.h"

class FrArea;
class FrButton;
class FrViewer;
class FrEdit;
class FrListBox;
struct FrListItem;
class CWindFont;
class Bitmap;

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// "PointShopHelp": figura de ajuda "pointshop_explanation.jpg" num FrViewer. vtables 00d312cc / 00d312c0 / 00d312b8; 0x120.
class FrPointShopHelp : public FrForm
{
	DECLARE_OBJECT(FrPointShopHelp) // GetRTTI 0084b910; MakeInstance 0084bfe0; registro 00cddf10

public:
	FrPointShopHelp(); // 0084b8e0
	virtual ~FrPointShopHelp(); // 0084b920 (com `delete`: 0084c070; ajustes 0084bf80 / 0084bf90)

protected:
	void OnDescViewInit983(int param); // 0084c0a0

	FrViewer* m_pDescView; // +0x110: "desc_view"
	char m_pad983_114[0xc]; // +0x114: o 983 não lê nem escreve (construtor 0084b8e0 só zera +0x110)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0084b8d0; mapa 00e21c10
};
C_ASSERT_VC10(sizeof(FrPointShopHelp) == 0x120);

// "PointShopMessageDlg": aviso do resultado (texto vermelho sobre "tikipoint_popup_bg" ou "tikipoint_popup_fail"). vtables
// 00d31594 / …; 0x140 (operator new de 0084ea10 e de 00655680, o CreateForm do CTaskMain).
class FrPointShopMessageDlg : public FrForm
{
	DECLARE_OBJECT(FrPointShopMessageDlg) // GetRTTI 0084ded0; MakeInstance 0084ea10; registro 00cddfa0

public:
	FrPointShopMessageDlg(); // 0084e820
	virtual ~FrPointShopMessageDlg(); // 0084de70 (com `delete`: 0084e2f0; ajustes 0084dee0 / 0084def0)

	void SetSuccess983(int bSuccess) { m_bSuccess = bSuccess; } // 0084b950 (COMDAT; quem chama é o CTaskMain)
	void SetMessage983(const char* msg) // 0084e8d0 (chamada pelo CTaskMain, mensagem 0x2e1)
	{
		if (msg)
			m_msg = msg;
	}

protected:
	virtual void OnDraw(); // 0084d9d0

	void OnBackgroundInit983(int param); // 0084c130

	int m_bSuccess; // +0x110: 0 = figura de falha (1 no construtor)
	std::string m_msg; // +0x114
	const Bitmap* m_pBgBmp; // +0x130: "tikipoint_popup_bg"
	const Bitmap* m_pFailBmp; // +0x134: "tikipoint_popup_fail"
	FrArea* m_pBackground; // +0x138: "background"
	int m_pad983_13c; // +0x13c: o 983 não lê nem escreve (construtor 0084e820, OnDraw 0084d9d0)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0084b940; mapa 00e21c18
};
C_ASSERT_VC10(sizeof(FrPointShopMessageDlg) == 0x140);

// "PointShopNoticeDlg": confirmação da troca (modo 1, "%s(%d) x %d" de cada item e o total de TP) ou da compra (modo 2,
// ícone, "%s(%d) : %dTP" e a descrição do item no "desc"); "Apply" confere o saldo e manda o pacote. vtables 00d313c4 /
// 00d313b4 / …; 0x130.
class FrPointShopNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrPointShopNoticeDlg) // GetRTTI 0084ba20; MakeInstance 0084c1a0; registro 00cde030

public:
	FrPointShopNoticeDlg(); // 0084b970
	virtual ~FrPointShopNoticeDlg(); // 0084ba30 (com `delete`: 0084c230; ajustes 0084bfa0 / 0084bfb0)

	void SetShop983(CPointShop* pShop); // 008500a0: guarda a loja; na compra escreve a descrição do item

protected:
	virtual void OnDraw(); // 0084fbd0

	void OnBackgroundInit983(int param); // 0084c260
	void OnTagInit983(int param); // 0084c2d0
	void OnTagDraw983(); // 0084c340: "tikipoint_popup_tec1" (troca) ou "…tec2" (compra)
	void OnDescInit983(int param); // 0084c410
	void OnApplyUp983(); // 0084dac0
	void OnExitUp983() { Close(true); } // 0084ba70

	const Bitmap* m_pTec1Bmp; // +0x110: "tikipoint_popup_tec1"
	const Bitmap* m_pTec2Bmp; // +0x114: "tikipoint_popup_tec2"
	const Bitmap* m_pBgBmp; // +0x118: "tikipoint_popup_bg"
	FrArea* m_pBackground; // +0x11c: "background"
	FrArea* m_pTag; // +0x120: "tag"
	FrEdit* m_pDesc; // +0x124: "desc"
	CPointShop* m_pShop; // +0x128: a da FrPointShopDlg
	int m_pad983_12c; // +0x12c: o 983 não lê nem escreve (construtor 0084b970 nem funções da classe)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0084b960; mapa 00e21c20
};
C_ASSERT_VC10(sizeof(FrPointShopNoticeDlg) == 0x130);

// Item das listas "Own_item"/"Exchange_item"/"Shop_item" (nó de 0x20 do std::list): tipo, quantos e os uid escolhidos (uid →
// quantos). Mesmo desenho do sMPBreakListItem983 (os moldes do std::map<unsigned long, unsigned long> são os mesmos no 983,
// 004f7060/004f7990/004f8820), mas o std::list tem moldes próprios (00850490…00850790): tipo próprio.
struct sPointShopListItem983
{
	unsigned long typeId; // +0x0
	int count; // +0x4
	std::map<unsigned long, unsigned long> uids; // +0x8
};

// "PointShopDlg": a loja de pontos do Tiki. vtables 00d3171c / 00d31710 / 00d31708; sizeof 0x1c0 (operator new de 00850c40).
// Três páginas (+0x1b8): 1 troca (meus itens → TP; "Own_item" e "Exchange_item"), 2 compra ("Shop_item", PointShop.iff),
// 3 abertura ("Intro_*"). "Exchange_ok" e o clique na lista de compra abrem a "PointShopNoticeDlg".
class FrPointShopDlg : public FrForm
{
	DECLARE_OBJECT(FrPointShopDlg) // GetRTTI 00850a00; MakeInstance 00850c40; registro 00cde150

public:
	FrPointShopDlg(); // 00850850
	virtual ~FrPointShopDlg(); // 00850a30 (com `delete`: 00850cd0; ajustes 00850a10 / 00850a20)

	virtual bool OnInit(); // 0084ba90
	void Refresh983(); // 008515a0 (CTaskMain 00651030, mensagem 0x2e2): refaz a página e esvazia a escolha

protected:
	virtual void OnProc(const float deltaTime); // 00850200: "Exchange_ok" só com algo escolhido (troca)

	void DrawBitmap983(const Bitmap* pBmp, WRect rc, unsigned long color, bool bFit); // 0084c480 (ecx sem uso)
	sPointShopListItem983* FindListItem983(unsigned long typeId); // 0084e320
	bool CanExchange983(unsigned long typeId, void* pData); // 0084e350: sItemInfo (sCards para cartas) pode ir para a troca?
	void FillListBox983(); // 0084e5d0: lista da página atual → "Own_item" (troca) ou "Shop_item" (compra)
	int AddListItem983(unsigned long typeId, unsigned long uid, int count, bool bFront); // 00850da0
	void RefreshList983(); // 00850f10: inventário (troca) ou PointShop.iff (compra)
	void SetPage983(int page); // 00851310
	void DrawListItem983(FrListItem* pItem, float h, unsigned int maxLen, unsigned int count); // 0084eda0
	void ClearList983()
	{
		for (std::list<sPointShopListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
			it->uids.clear();
		m_list.clear();
	}

	void OnMyPointInit983(int param); // 0084c530
	void OnMyPointDraw983(); // 0084dbc0: TP que eu tenho
	void OnExitInit983(int param); // 0084c6f0
	void OnExitUp983() { OnFreshOkay(); } // 0084bb20
	void OnMainInit983(int param); // 0084c5a0
	void OnMainUp983() { SetPage983(3); } // 00851570
	void OnHelpInit983(int param); // 0084c650
	void OnHelpUp983(); // 0084dc30
	void OnGotoMainBgInit983(int param); // 0084c790
	void OnIntroBg1Init983(int param); // 0084c810
	void OnIntroBg2Init983(int param); // 0084c890
	void OnIntroNoticeInit983(int param); // 0084c910
	void OnIntroNoticeDraw983(); // 0084c9b0
	void OnExchangeInit983(int param); // 0084c9d0
	void OnExchangeUp983() { SetPage983(1); } // 00851580
	void OnBuyInit983(int param); // 0084ca80
	void OnBuyUp983() { SetPage983(2); } // 00851590
	void OnExchangeListBG1Init983(int param); // 0084cb20
	void OnExchangeListBG2Init983(int param); // 0084cba0
	void OnExchangeListBG3Init983(int param); // 0084cc20
	void OnExchangeListBG4Init983(int param); // 0084cca0
	void OnExchangeListBG5Init983(int param); // 0084cd20
	void OnExchangeListBG6Init983(int param); // 0084cda0
	void OnPrePointInit983(int param); // 0084ce20
	void OnPrePointDraw983(); // 0084cea0: TP da troca escolhida
	void OnOkInit983(int param); // 0084cf20
	void OnOkUp983(); // 008502b0
	void OnOwnItemInit983(int param); // 0084cfa0
	void OnOwnItemDraw983(int param); // 0084f700
	void OnOwnItemUp983(); // 0084e620
	void OnExchangeItemInit983(int param); // 0084d030
	void OnExchangeItemDraw983(int param); // 0084f7c0
	void OnExchangeItemUp983(); // 0084e6e0
	void OnShopListBG1Init983(int param); // 0084d0b0
	void OnShopListBG2Init983(int param); // 0084d130
	void OnShopListBG3Init983(int param); // 0084d1b0
	void OnShopListBG4Init983(int param); // 0084d230
	void OnShopListBG5Init983(int param); // 0084d2b0
	void OnShopListBG6Init983(int param); // 0084d330
	void OnShopItemInit983(int param); // 0084d3b0
	void OnShopItemDraw983(int param); // 0084f8d0
	void OnShopItemUp983(); // 008503a0

	std::list<sPointShopListItem983> m_list; // +0x110: o que a página atual lista
	const Bitmap* m_pMyOverBmp; // +0x11c: "tikipoint_btn_pointget_list_my_o"
	const Bitmap* m_pUseOverBmp; // +0x120: "tikipoint_btn_pointget_list_use_o"
	const Bitmap* m_pListNormalBmp; // +0x124: "tikipoint_pointuse_list_n"
	const Bitmap* m_pListOverBmp; // +0x128: "tikipoint_pointuse_list_o"
	const Bitmap* m_pBuyBmp; // +0x12c: "tikipoint_buy"
	FrArea* m_pGotoMainBg; // +0x130: "GotoMainBg"
	FrArea* m_pShopListBG[6]; // +0x134: "Shop_ListBG1…6"
	FrArea* m_pExchangeListBG[6]; // +0x14c: "Exchange_ListBG1…6"
	FrArea* m_pIntroBg1; // +0x164: "Intro_bg1"
	FrArea* m_pIntroBg2; // +0x168: "Intro_bg2"
	FrArea* m_pMyPoint; // +0x16c: "MyPoint"
	FrArea* m_pPrePoint; // +0x170: "PrePoint"
	FrViewer* m_pIntroNotice; // +0x174: "Intro_notice" ("pointshop_notice.jpg")
	FrButton* m_pMain; // +0x178: "PointShop_Main"
	FrButton* m_pHelp; // +0x17c: "PointShop_Help"
	int m_pad983_180; // +0x180: o 983 não lê nem escreve (nenhum [reg+0x180] em 0084b800..00851200)
	FrButton* m_pExit; // +0x184: "PointShop_Exit"
	FrButton* m_pExchange; // +0x188: "Exchange"
	FrButton* m_pBuy; // +0x18c: "Buy"
	FrButton* m_pOk; // +0x190: "Exchange_ok"
	FrListBox* m_pOwnItem; // +0x194: "Own_item" (meus itens)
	FrListBox* m_pExchangeItem; // +0x198: "Exchange_item" (escolhidos)
	FrListBox* m_pShopItem; // +0x19c: "Shop_item"
	CPointShop m_shop; // +0x1a0
	int m_page; // +0x1b8: 1 troca, 2 compra, 3 abertura
	CWindFont* m_pFont; // +0x1bc

	DECLARE_FRESH_MSGMAP() // GetMessageMap 0084ba80; mapa 00e21c28
};
C_ASSERT_VC10(sizeof(FrPointShopDlg) == 0x1c0);
