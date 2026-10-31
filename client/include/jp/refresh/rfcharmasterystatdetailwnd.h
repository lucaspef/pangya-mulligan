#pragma once
#include "vc10_layout.h"
#include <string>
#include <vector>
#include "rfformmodalbase.h"
#include "masterygaugebar.h"

class RFMasteryToolTipWnd;
struct sItemInfo;
namespace IFF_STRUCT { struct sPart; struct sAuxPart; struct sCaddie; struct sMascot; struct sClubSet; }

// JP 983: janelas RF dos atributos do personagem na maestria (unidade rfcharmasterystatdetailwnd.cpp, 005c5a20–005ccb80;
// sessão B141 — a visão parcial da B140 virou a classe inteira). A masterygaugebar.cpp (005c6360–005c6400, só o construtor)
// fica no meio dela. Classes pela RTTI; métodos e campos com nome meu (…983), menos os da RF. Atributo (type) 0 força,
// 1 controle, 2 precisão, 3 efeito, 4 curva; 5 = nenhum (construtor).
// - RFMasteryStatNoticeDlg ("mastery_stat_notice_dialog.xml"; vtables 00d0fc8c / 00d0fc80; 0x19c; modal): resultado da
//   subida/descida de um atributo (imagens do atributo, seta, valor novo, texto). Instância 00e781a8.
// - RFMasteryStatUpDnDlg ("kam_status_updown_dialog.xml"; vtables 00d0fd24 / 00d0fd1c; 0x1b8; modal): "subir/descer o
//   atributo?" (slots antes/depois; ok manda o pacote 0x188 (subir) ou 0x189 (descer) com o atributo e o sCharacterInfo do
//   personagem em uso; subir custa pang (Enchant.iff 0x340 | atributo << 20)). Instância 00e781b0.
// - RFCharMasteryStatColumn ("kam_status_column.xml"; vtable 00d0fdb4; 0x198): uma linha da lista (botão do atributo,
//   medidor, "max", e a RFCharMasteryStatDetailWnd no lugar do marcador status_detail_wnd).
// - RFCharMasteryStatDetailWnd ("kam_status_detail_wnd.xml"; vtable 00d0ff34; 0x1e0): o detalhe que abre/fecha
//   (animações _push/_close/_over/_out_status_detail_): números (personagem, equipamento, encaixes, penalidade, total,
//   encaixes livres), botões de subir/descer e as dicas dos "ovos" (RFMasteryToolTipWnd).
// - RFCharMasteryStatusDlg ("kam_mastery_status.xml"; vtable 00d0fe44; 0x188): a lista dos 5 atributos. Instância 00e781c8
//   (pedido C76.2: IsCreated983 005bd3d0 na rfcharmasteryaboutdlg, Refresh983 005cc980, OnResult983 005c9b00).

class RFMasteryStatNoticeDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFMasteryStatNoticeDlg(); // 005c6400
	virtual ~RFMasteryStatNoticeDlg(); // 005c64a0 (??_G 005c71b0; 2ª base 005c6490)

	virtual void OnInitialUpdate(); // 005c8870 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005c6510 (slot 34): Enter/ESC = ok

	void SetInstance983(); // 005c5a20
	static void ClearInstance983(); // 005c5a30
	// 005c8400: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFMasteryStatNoticeDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005c5b90
	void SetStat983(unsigned long type, bool up); // 005c8540

	static RFMasteryStatNoticeDlg* ms_pInstance; // 00e781a8
	static RFWnd* s_pParent983; // 00e781a4

	enum { IMG_STAT983 = 0, IMG_CAP983 = 1 };
	// +0x188: "button_mastery_stat_%d_n.tga" / "button_mastery_stat_%d_cap.tga"; array porque o construtor do 983 zera por memset
	RFPicture* m_pStatImg983[2];
	RFPicture* m_pArrow983; // +0x190
	RFLabel* m_pStat983; // +0x194
	RFTextArea* m_pMessage983; // +0x198
};
C_ASSERT_VC10(sizeof(RFMasteryStatNoticeDlg) == 0x19c);

class RFMasteryStatUpDnDlg : public RFFormModalBase<RFWndEx>
{
public:
	enum { MODE_DOWN983 = -1, MODE_NONE983 = 0, MODE_UP983 = 1 };

	RFMasteryStatUpDnDlg(); // 005c6540
	virtual ~RFMasteryStatUpDnDlg(); // 005c6610 (??_G 005c71e0; 2ª base 005c6600)

	virtual void OnInitialUpdate(); // 005cb2f0 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005cb910 (slot 34): Enter = ok, ESC = fecha

	static RFMasteryStatUpDnDlg* GetInstance983(); // 005c5a40
	static RFMasteryStatUpDnDlg* GetInstance2_983(); // 005c5a50 (cópia igual, sem chamador)
	void SetInstance983(); // 005c5a60
	static void ClearInstance983(); // 005c5a70
	bool IsSent983(); // 005c5a80
	void SetSent983(bool sent); // 005c5a90
	void SetMode983(int mode); // 005c5b50
	// 005c8c50: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFMasteryStatUpDnDlg* Create983(RFWnd* parent);
	void UpdateSlots983(); // 005c5c10
	void UpdateImages983(); // 005c8d90
	void OnOk983(EventArgs& e); // 005c9bc0
	void OnCancel983(EventArgs& e); // 005c5bd0
	void UpdateText983(); // 005c9df0
	void Set983(unsigned int type, int mode); // 005cb260

	static RFMasteryStatUpDnDlg* ms_pInstance; // 00e781b0
	static RFWnd* s_pParent983; // 00e781ac

	enum { IMG_ARROW983 = 0, IMG_TITLE983 = 1, IMG_STATBG1_983 = 2, IMG_STATBG2_983 = 3 };
	enum { NUM_TOTAL983 = 0, NUM_BEFORE983 = 1, NUM_AFTER983 = 2 };
	// arrays porque o construtor do 983 zera +0x188…+0x1a3 por memset
	RFPicture* m_pImg983[4]; // +0x188 (IMG_*)
	RFLabel* m_pNum983[3]; // +0x198 (NUM_*)
	RFTextArea* m_pMessage983; // +0x1a4
	RFTextArea* m_pWarning983; // +0x1a8
	int m_mode983; // +0x1ac (MODE_*)
	unsigned int m_type983; // +0x1b0 (5 = nenhum)
	bool m_sent983; // +0x1b4 (pacote já mandado)
};
C_ASSERT_VC10(sizeof(RFMasteryStatUpDnDlg) == 0x1b8);

class RFCharMasteryStatDetailWnd : public RFFormBase<RFWndEx>
{
public:
	enum { LABEL_CHAR983 = 0, LABEL_PARTS983 = 1, LABEL_SLOT983 = 2, LABEL_PENALTY983 = 3, LABEL_TOTAL983 = 4, LABEL_EXTRA983 = 5 };

	RFCharMasteryStatDetailWnd(); // 005c7a70
	virtual ~RFCharMasteryStatDetailWnd(); // 005c7b10 (??_G 005c80a0)

	virtual void OnInitialUpdate(); // 005cbcd0 (slot 10)

	// 005c9020: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharMasteryStatDetailWnd* Create983(RFWnd* parent);
	void SetCap983(unsigned long type); // 005c9160: "button_mastery_stat_%d_cap.tga" em cap_img
	void OnCap983(EventArgs& e); // 005c92a0: abre/fecha
	void ShowTip983(bool show, const char* text, int label, unsigned long color); // 005c9400: dica ao lado do número
	void OnBaseEnter983(EventArgs& e); // 005ca030 (egg1)
	void OnEquipEnter983(EventArgs& e); // 005ca0f0 (egg2): peças, auxiliares, caddie, mascote, taco e cartas
	void OnSlotEnter983(EventArgs& e); // 005ca410 (egg3)
	void OnPenaltyEnter983(EventArgs& e); // 005ca640 (egg4)
	void OnTipLeave983(EventArgs& e); // 005ca770
	void OnCapEnter983(EventArgs& e); // 005ca790
	void OnCapLeave983(EventArgs& e); // 005ca8e0
	void LoadEquip983(); // 005ca9b0: o que o personagem em uso tem que dá ponto nesse atributo
	void Update983(unsigned long type); // 005cb950
	void OnStatUp983(EventArgs& e); // 005cbc30
	void OnStatDown983(EventArgs& e); // 005cbc70

	static RFWnd* s_pParent983; // 00e781b4

	unsigned long m_type983; // +0x184 (5 = nenhum)
	RFButton* m_pUp983; // +0x188
	RFButton* m_pDown983; // +0x18c
	RFLabel* m_pLabel983[6]; // +0x190 (LABEL_*)
	RFMasteryToolTipWnd* m_pTip983; // +0x1a8
	int m_open983; // +0x1ac
	std::vector<IFF_STRUCT::sPart*> m_parts983; // +0x1b0
	std::vector<IFF_STRUCT::sAuxPart*> m_auxParts983; // +0x1c0
	IFF_STRUCT::sCaddie* m_pCaddie983; // +0x1d0
	IFF_STRUCT::sMascot* m_pMascot983; // +0x1d4
	IFF_STRUCT::sClubSet* m_pClub983; // +0x1d8
	sItemInfo* m_pClubInfo983; // +0x1dc
};
C_ASSERT_VC10(sizeof(RFCharMasteryStatDetailWnd) == 0x1e0);

class RFCharMasteryStatColumn : public RFFormBase<RFWndEx>
{
public:
	RFCharMasteryStatColumn(); // 005c6680
	virtual ~RFCharMasteryStatColumn(); // 005c66c0 (??_G 005c7210)

	virtual void OnInitialUpdate(); // 005cacd0 (slot 10)

	// 005c9580: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharMasteryStatColumn* Create983(RFWnd* parent);
	void UpdateGauge983(); // 005c5d60
	void UpdateButton983(); // 005c7be0
	void UpdateBar983(); // 005c7d20
	void UpdateCap983(); // 005c96c0
	void OnEnter983(EventArgs& e); // 005c96e0
	void OnLeave983(EventArgs& e); // 005c9790
	void OnClick983(EventArgs& e); // 005c9840: abre/fecha o detalhe
	void SetType983(unsigned long type); // 005cca30
	void UpdateDetail983(); // 005cbcb0
	void Update983(); // 005cc970

	static RFWnd* s_pParent983; // 00e781bc

	unsigned long m_type983; // +0x184 (5 = nenhum)
	RFButton* m_pButton983; // +0x188
	RFPicture* m_pMax983; // +0x18c
	MasteryGaugeBar* m_pGauge983; // +0x190
	RFCharMasteryStatDetailWnd* m_pDetail983; // +0x194
};
C_ASSERT_VC10(sizeof(RFCharMasteryStatColumn) == 0x198);

class RFCharMasteryStatusDlg : public RFFormBase<RFWndEx>
{
public:
	RFCharMasteryStatusDlg(); // 005c6740
	virtual ~RFCharMasteryStatusDlg(); // 005c6770 (??_G 005c7240)

	virtual void OnInitialUpdate(); // 005ccb60 (slot 10)

	static bool IsCreated983(); // 005bd3d0 (unidade rfcharmasteryaboutdlg; o CTask::OnPacketCommon chama)
	// 005c99c0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharMasteryStatusDlg* Create983(RFWnd* parent);
	// 005c9b00: resposta do servidor à subida/descida (fecha a RFMasteryStatUpDnDlg e mostra a RFMasteryStatNoticeDlg)
	void OnResult983(unsigned long type, bool up);
	void Refresh983(); // 005cc980 (CCardManager::CalcCardPeriodAndStatus, aviso 0x109 ao ator principal, redesenha as linhas)
	void InitList983(); // 005cca60

	static RFCharMasteryStatusDlg* ms_pInstance; // 00e781c8
	static RFWnd* s_pParent983; // 00e781c4

	RFList* m_pList983; // +0x184 ("status_list", 5 RFCharMasteryStatColumn)
};
C_ASSERT_VC10(sizeof(RFCharMasteryStatusDlg) == 0x188);
