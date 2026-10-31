#pragma once
#include "vc10_layout.h"
// JP 983: quebra de itens em MP do Tiki ("Alchemy"/"Tiki recycle"). UMA unidade 004f3110–004fb5f0 com quatro janelas; ordem do
// fonte pelo inicializador estático (00ccad80…00ccb300): FrItemCountController (mapa 00e07ad0), FrMPBreakAboutDlg (00e07bd0),
// FrAlchemyMPBreakNoticeDlg (00e07f10) e FrAlchemyMPBreakDlg (00e07fd0, 0x6d0, com a CAlchemyMPBreak embutida em +0x188).
// Existe no 839, não no 645. Nomes dos métodos e dos campos são meus (sessão B94: as três janelas pequenas).
#include "frform.h"
#include "fredit.h"

class FrEdit;
class FrButton;
class FrTextButton;
class FrViewer;
struct FrListItem;

#ifndef KINDOF_CAST983
// DYNAMIC_CAST com o IsKindOf embutido (o 983 não chama IObject::DynamicCast aqui); nulo vai direto ao NULL
#define KINDOF_CAST983(type, obj) \
	((__rtti_obj = (void*)(obj)) ? (((IObject*)__rtti_obj)->IsKindOf(&type::m_RTTI) ? (type*)(obj) : NULL) : NULL)
#endif

// "MPBreakCountSetDlg": quantos de um item empilhável entram na quebra (setas com repetição ao segurar). 0x140.
class FrItemCountController : public FrForm
{
	DECLARE_OBJECT(FrItemCountController) // GetRTTI 004f3140; MakeInstance 004f6430; registro 004f64a0

public:
	FrItemCountController() { Reset983(); } // 004f3c40
	virtual ~FrItemCountController() { } // 004f3120 (com `delete`: 004f3c70)

	// 004f64c0: item da lista (pData começa pelo typeId) e o máximo; escreve o nome
	void SetItem983(FrListItem* pItem, int max);
	FrListItem* GetItem983() { return m_pItem; }
	int GetCount983() { return m_count; }

protected:
	int GetEditCount983(); // 004f3150: número escrito em "item_count"
	void Reset983() // 004f3170
	{
		m_pressTime = 0.0f;
		m_max = 0;
		m_count = 1;
		m_pItem = NULL;
		m_pDesc = NULL;
		m_pItemName = NULL;
		m_pIncrease = NULL;
		m_pDecrease = NULL;
		m_pressState = 0;
		m_bRepeated = false;
		m_repeatCount = 0;
	}

	virtual void OnProc(const float deltaTime); // 004f6530: repetição das setas e limite do número digitado

	void Increase983(); // 004f3ca0
	void Decrease983(); // 004f3d50
	void OnItemNameInit983(int param); // 004f3de0
	void OnDescInit983(int param); // 004f3e50 (texto 0x1870f)
	void OnCountInit983(int param); // 004f3ef0
	void OnIncreaseInit983(int param); // 004f3f90
	void OnIncreaseUp983(); // 004f4000
	void OnDecreaseInit983(int param); // 004f4030
	void OnDecreaseUp983(); // 004f40a0

	FrEdit* m_pItemName; // +0x110: "item_name"
	FrEdit* m_pDesc; // +0x114: "dlg_desc"
	FrEdit* m_pCount; // +0x118: "item_count" (o Reset983 não zera)
	FrButton* m_pIncrease; // +0x11c: "increase_cnt"
	FrButton* m_pDecrease; // +0x120: "Decrease_cnt"
	int m_max; // +0x124
	int m_count; // +0x128
	FrListItem* m_pItem; // +0x12c
	bool m_bRepeated; // +0x130: a seta já repetiu (o LBUTTONUP não soma de novo)
	int m_pressState; // +0x134: 0 nada, 1 "increase" apertado, 2 "Decrease"
	float m_pressTime; // +0x138
	int m_repeatCount; // +0x13c

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004f3110; mapa 00e07ad0
};
C_ASSERT_VC10(sizeof(FrItemCountController) == 0x140);

// "mpbreak_about": ajuda com 9 figuras (help_tiki_recycle_01…09.jpg). 0x150.
class FrMPBreakAboutDlg : public FrForm
{
	DECLARE_OBJECT(FrMPBreakAboutDlg) // GetRTTI 004f3240; MakeInstance 004f40d0; registro 004f4140

public:
	FrMPBreakAboutDlg(); // 004f31d0
	virtual ~FrMPBreakAboutDlg() { } // 004f3250 (com `delete`: 004f4160)

	virtual bool OnInit(); // 004f4850: página 0 (não chama FrForm::OnInit)

protected:
	void OnOkUp983() { OnFreshOkay(); } // 004f3270
	// 004f3280: desmarca os 9 índices, liga as setas, prende a página em 0…8, marca o índice e abre a figura
	void SetPage983(int page);

	void OnViewerInit983(int param); // 004f4190
	void OnIndex01Init983(int param); // 004f4200
	void OnIndex01Up983() { SetPage983(0); } // 004f4270
	void OnIndex02Init983(int param); // 004f4280
	void OnIndex02Up983() { SetPage983(1); } // 004f42f0
	void OnIndex03Init983(int param); // 004f4300
	void OnIndex03Up983() { SetPage983(2); } // 004f4370
	void OnIndex04Init983(int param); // 004f4380
	void OnIndex04Up983() { SetPage983(3); } // 004f43f0
	void OnIndex05Init983(int param); // 004f4400
	void OnIndex05Up983() { SetPage983(4); } // 004f4470
	void OnIndex06Init983(int param); // 004f4480
	void OnIndex06Up983() { SetPage983(5); } // 004f44f0
	void OnIndex07Init983(int param); // 004f4500
	void OnIndex07Up983() { SetPage983(6); } // 004f4570
	void OnIndex08Init983(int param); // 004f4580
	void OnIndex08Up983() { SetPage983(7); } // 004f45f0
	void OnIndex09Init983(int param); // 004f4600
	void OnIndex09Up983() { SetPage983(8); } // 004f4670
	void OnBackwardInit983(int param); // 004f4680
	void OnBackwardUp983(); // 004f4700
	void OnForwardInit983(int param); // 004f4740
	void OnForwardUp983(); // 004f47c0
	void OnOkInit983(int param); // 004f4800 (só o cast)

	enum { PAGE_COUNT = 9 };
	FrTextButton* m_pIndex[PAGE_COUNT]; // +0x110: "index01…09"
	int m_page; // +0x134
	FrButton* m_pBackward; // +0x138
	FrButton* m_pForward; // +0x13c
	FrViewer* m_pViewer; // +0x140: "aboutviewer" (o construtor não zera)
	int m_pad983_144[3]; // +0x144: sem leitura/escrita no 983 (objdump 004f2f00..004fb000, E414)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004f31c0; mapa 00e07bd0
};
C_ASSERT_VC10(sizeof(FrMPBreakAboutDlg) == 0x150);

// "AlchemyMPBreakNoticeDlg": aviso antes de quebrar (itens raros, UCC, tacos melhorados…). 0x120.
class FrAlchemyMPBreakNoticeDlg : public FrForm
{
	DECLARE_OBJECT(FrAlchemyMPBreakNoticeDlg) // GetRTTI 004f33a0; MakeInstance 004f4860; registro 004f48d0

public:
	FrAlchemyMPBreakNoticeDlg(); // 004f3360
	virtual ~FrAlchemyMPBreakNoticeDlg() { } // 004f33b0 (com `delete`: 004f48f0)

	void AddFocusMsg983(const char* text) // 004f33d0
	{
		if (text)
			m_pFocusMsg->AddText(text, false, true, 0, 0);
	}
	void AddMsg983(const char* text) // 004f3400
	{
		if (text)
			m_pMsg->AddText(text, false, true, 0, 0);
	}

protected:
	void OnCautionMsgInit983(int param); // 004f4a00
	void OnApplyUp983() { OnFreshOkay(); } // 004f3430
	void OnExitUp983() { OnFreshCancel(); } // 004f3440
	void OnFocusMsgInit983(int param); // 004f4920
	void OnMsgInit983(int param); // 004f4990

	FrEdit* m_pFocusMsg; // +0x110: "FocuseMsg"
	FrEdit* m_pMsg; // +0x114: "Msg"
	FrEdit* m_pCautionMsg; // +0x118: "CautionMsg"
	int m_pad983_11c; // +0x11c: sem leitura/escrita no 983 (E414)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004f3350; mapa 00e07f10
};
C_ASSERT_VC10(sizeof(FrAlchemyMPBreakNoticeDlg) == 0x120);

#include <list>
#include <map>
#include <string>
#include "fx.h"
#include "wtypes.h"
#include "alchemympbreak.h"

class FrArea;
class FrListBox;
class Bitmap;
class CGaugeBar983;
class CTPItemCountFont;
class CMPItemCountFont;

// Item da lista "Own_item"/"Exchange_item" (nó de 0x20 do std::list): tipo, quantos sobram e os guids escolhidos (guid → quantos).
struct sMPBreakListItem983
{
	unsigned long typeId; // +0x0
	int count; // +0x4
	std::map<unsigned long, unsigned long> guids; // +0x8
};

// "AlchemyMPBreakDlg": quebra de itens em MP do Tiki. vtables 00d05b14 / 00d05b08 / 00d05b00; sizeof 0x6d0 (dados até 0x6c8,
// FrForm alinhada a 16). Abas Pang/Cookie/Rare (+0x1cc 1/2/3), duas listas (inventário e escolhidos, até 5), três barras de
// MP (CGaugeBar983), fala do Tiki (4 textos sorteados) e o efeito "Item_break_N/B.seq" numa caixa de efeitos própria.
class FrAlchemyMPBreakDlg : public FrForm
{
	DECLARE_OBJECT(FrAlchemyMPBreakDlg) // GetRTTI 004fa660

public:
	FrAlchemyMPBreakDlg(); // 004fa3d0
	virtual ~FrAlchemyMPBreakDlg(); // 004fa690 (com `delete`: 004faa60; ajustes 004fa670 / 004fa680)

	virtual bool OnInit(); // 004fb490
	void SetState983(int state) { m_state = state; } // 004f3490 (chamada pela A em 0065f28d)
	void Reset983(); // 004fb460 (chamada em 00651086)
	void SetMPValues983(int a, int b); // 004f89e0: CTaskMain::HandleMsgCommon caso 0x2e4 (param1, param2); fala 3 se passou de 1000, senão 2

protected:
	virtual void OnProc(const float deltaTime); // 004f8000: máquina de estados da quebra (+0x120)

	void InitFx983(); // 004f4a70
	void InitGauges983(); // 004f4b60
	void DrawBitmap983(const Bitmap* pBmp, WRect rc, unsigned long color, bool bFit); // 004f4cc0 (ecx sem uso)
	void ProcGaugeState983(const float deltaTime); // 004f34a0 (deltaTime sem uso)
	void ProcGauges983(const float deltaTime); // 004f35b0
	void DrawGauges983(); // 004f3620
	bool OnNoticeResult983(int result, FrForm* pForm); // 004f6680
	void OpenNotice983(); // 004f6740
	void ShowTalk983(); // 004f6850
	int PickText983(int first, int last); // 004f8210
	void SetTalks983(); // 004f8650
	void SetTalk983(int index); // 004f8730
	sMPBreakListItem983* FindListItem983(unsigned long typeId); // 004f74d0
	int GetListCount983(FrListItem* pItem); // 004f7500
	bool CanBreak983(unsigned long typeId, void* pData); // 004f7530: sItemInfo* (cartas: sCards*)
	int GetBreakCount983(unsigned long typeId); // 004f77b0
	void SyncOwnList983(); // 004f7820
	void SendBreak983(); // 004f7c20
	void PlayFx983(); // 004f7d50
	void DrawListItem983(FrListItem* pItem, float h, int maxLen, int count); // 004f8b10
	void UpdateOk983(); // 004f99b0
	void OpenCountDlg983(FrListItem* pItem); // 004f9ed0
	bool OnCountResult983(int result, FrForm* pForm); // 004f9d50
	int AddListItem983(unsigned long typeId, unsigned long guid, int count, bool bFront); // 004fab30
	void RefreshList983(); // 004faca0
	void SetMode983(int mode); // 004fb2c0
	void ClearList983()
	{
		for (std::list<sMPBreakListItem983>::iterator it = m_list.begin(); it != m_list.end(); ++it)
			it->guids.clear();
		m_list.clear();
	}

	void OnTPPointInit983(int param); // 004f4d70
	void OnTPPointDraw983(); // 004f4df0
	void OnExitInit983(int param); // 004f4f10
	void OnExitUp983() { OnFreshOkay(); } // 004f3600
	void OnHelpInit983(int param); // 004f4e70
	void OnHelpUp983(); // 004f68a0
	void OnMagicBoxInit983(int param); // 004f4fb0
	void OnMagicBoxUp983(); // 004f6900
	void OnCardComposeInit983(int param); // 004f5060
	void OnCardComposeUp983(); // 004f6960
	void OnExchangeInit983(int param); // 004f5110
	void OnExchangeUp983() { } // 004f3610
	void OnCookieInit983(int param); // 004f51c0
	void OnCookieUp983(); // 004fb1a0
	void OnPangInit983(int param); // 004f5260
	void OnPangUp983(); // 004fb200
	void OnRareInit983(int param); // 004f5300
	void OnRareUp983(); // 004fb260
	void OnTalkInit983(int param); // 004f53a0
	void OnListBG1Init983(int param); // 004f5410
	void OnListBG2Init983(int param); // 004f5490
	void OnListBG3Init983(int param); // 004f5510
	void OnListBG4Init983(int param); // 004f5590
	void OnEffectInit983(int param); // 004f5610
	void OnEffectDraw983(); // 004f56a0
	void OnMPGaugeInit983(int param); // 004f5850
	void OnExpectMPInit983(int param); // 004f58d0
	void OnExpectMPDraw983(); // 004f5950
	void OnPrePointInit983(int param); // 004f59c0
	void OnPrePointDraw983(); // 004f5a40
	void OnOkInit983(int param); // 004f5b50
	void OnOkUp983(); // 004f9a50
	void OnOwnItemInit983(int param); // 004f5bd0
	void OnOwnItemDraw983(int param); // 004f9500
	void OnOwnItemUp983(); // 004f9f60
	void OnExchangeItemInit983(int param); // 004f5c60
	void OnExchangeItemDraw983(int param); // 004f95c0
	void OnExchangeItemUp983(); // 004f9ba0

	std::list<sMPBreakListItem983> m_list; // +0x110: inventário que pode ser quebrado (aba atual)
	int m_bChanged; // +0x11c: 1 depois de escolher um item
	int m_state; // +0x120: 1 tocar, 2 efeito, 3 fim do efeito, 4/5 barras, 6 enviar
	int m_gaugeState; // +0x124
	const Bitmap* m_pListOverL; // +0x128: "list_button_o_left"
	const Bitmap* m_pListOverR; // +0x12c: "list_button_o_right"
	FrArea* m_pListBG[4]; // +0x130: "Exchange_ListBG1…4"
	FrArea* m_pTPPoint; // +0x140: "My_TP_Point"
	FrArea* m_pMPGauge; // +0x144: "MPGauge"
	FrArea* m_pExpectMP; // +0x148: "ExpectMP"
	FrArea* m_pPrePoint; // +0x14c: "PrePoint"
	int m_pad983_150; // +0x150: sem leitura/escrita no 983 (E414)
	FrButton* m_pHelp; // +0x154: "Alchemy_Help"
	FrButton* m_pExit; // +0x158: "Alchemy_Exit"
	FrButton* m_pMagicBox; // +0x15c: "MagicBox" (o construtor não zera)
	FrButton* m_pCardCompose; // +0x160: "CardCompose" (o construtor não zera)
	FrButton* m_pExchange; // +0x164: "Exchange"
	FrButton* m_pOk; // +0x168: "Exchange_ok"
	FrListBox* m_pOwnList; // +0x16c: "Own_item"
	FrListBox* m_pExchangeList; // +0x170: "Exchange_item"
	FrButton* m_pCookie; // +0x174: "Cookie_Item"
	FrButton* m_pPang; // +0x178: "Pang_Item"
	FrButton* m_pRare; // +0x17c: "Rare_Item"
	FrEdit* m_pTalk; // +0x180: "TikiTalk_Edit"
	FrArea* m_pEffect; // +0x184: "ExchangeOnEffect"
	CAlchemyMPBreak m_break; // +0x188
	int m_mode; // +0x1c0
	FrItemCountController* m_pCountDlg; // +0x1c4
	FrAlchemyMPBreakNoticeDlg* m_pNoticeDlg; // +0x1c8
	int m_tab; // +0x1cc: 1 Pang, 2 Cookie, 3 Rare
	CGaugeBar983* m_pMileGauge; // +0x1d0
	CGaugeBar983* m_pRespectGauge; // +0x1d4
	CGaugeBar983* m_pFullGauge; // +0x1d8
	CTPItemCountFont* m_pTPFont; // +0x1dc
	CMPItemCountFont* m_pMPFont; // +0x1e0
	std::string m_talk[4]; // +0x1e4
	int m_talkIndex; // +0x254
	CPvsFxBox m_fxBox; // +0x258 (0x440)
	WMatrix m_camera; // +0x698

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004fa660?; mapa 00e07fd0 (39 entradas)
};
C_ASSERT_VC10(sizeof(FrAlchemyMPBreakDlg) == 0x6d0);
