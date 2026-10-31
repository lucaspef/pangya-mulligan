#pragma once
#include "vc10_layout.h"
#include "rfformmodalbase.h"

struct sCards;
struct sItemInfo;
class RFMasteryToolTipWnd;
namespace IFF_STRUCT { struct sCard; }

// JP 983: janelas RF das cartas da maestria (unidade rfcharmasterycardselectdlg.cpp, 005b7f10–005bd040; sessão B142 — a
// visão parcial da B140 virou as classes inteiras). Classes pela RTTI; métodos e campos com nome meu (…983), menos os da RF.
// Encaixe ("slot") 1–10 da janela de cartas; índice 0–9 no sCharacterInfo::card (SlotToIndex983). Tipo da carta (Card.iff,
// typeId >> 22 & 0xf): 0 personagem, 1 caddie, 5 NPC. Os encaixes 9 e 10 pedem o item 0x1a00018f; o 6 só abre com o
// 7º encaixe do personagem (CSharedDoc::HasSeventhCardSlot983).
// - RFCharMasteryCardNoticeDlg ("mastery_card_notice_dialog.xml"; vtables 00d0e514 / 00d0e508; 0x190; modal): resultado do
//   pôr/tirar carta (imagem e texto). Instância 00e78114 (o 983 só lê; ninguém grava).
// - RFCharMasteryCardSelectColumn ("kam_mastery_card_select_column.xml"; vtable 00d0e5a4; 0x194): uma carta da lista.
// - RFCharMasteryCardSelectDlg ("kam_mastery_card_select_dialog.xml"; vtables 00d0e63c / 00d0e630; 0x1b4; modal): escolhe a
//   carta do encaixe; ok manda 0x18a (0x18b nos encaixes 9 e 10). Instância 00e78124.
// - RFCharacterMasteryCardDetachDlg ("kam_card_deselect_dialog.xml"; vtables 00d0e6d4 / 00d0e6c8; 0x190; modal): tira a
//   carta com o item 0x1a0000c2 (RFUseItemDlg; ok manda 0x18c). Instância 00e7812c.
// - RFCardIcon ("kam_mastery_card_thumb.xml"; vtable 00d0e764; 0x19c): um encaixe da janela de cartas.
// - RFCharMasteryCardDlg ("kam_mastery_card.xml"; vtable 00d0e7f4; 0x1b4): os 10 encaixes no lugar dos marcadores
//   slot_wnd1…10 e a dica (RFMasteryToolTipWnd). Instância 00e7813c (pedido C76.2: SetChar983 005bbdb0, OnResult983 005bb820).

// Funções da unidade (fora de classe; as de encaixe têm cópias iguais em 005bd090… no começo da unidade seguinte)
class CExhibition983;
void FitExhibition983(CExhibition983* pExh, float minDist, float maxDist); // 005b7f10 (sem chamador)
bool IsNotSlot10_11_983(int slot); // 005b7ff0 (sem chamador)
bool IsExtraSlot983(int slot); // 005b8000: encaixes 9 e 10 (pedem o item 0x1a00018f)
bool IsSeventhSlot983(int slot); // 005b8010: encaixe 6
int SlotToIndex983(int slot); // 005b8020: encaixe 1–10 → índice do sCharacterInfo::card (−1 fora)
int IndexToSlot983(int index); // 005b80a0 (sem chamador)
int SlotCardType983(int slot); // 005b8120: tipo de carta do encaixe (6 fora)
int IndexCardType983(int index); // 005b8160 (sem chamador)

class RFCharMasteryCardNoticeDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFCharMasteryCardNoticeDlg(); // 005b87f0
	virtual ~RFCharMasteryCardNoticeDlg(); // 005b8880 (??_G 005b97f0; 2ª base 005b8870)

	virtual void OnInitialUpdate(); // 005bad10 (slot 10)

	static RFCharMasteryCardNoticeDlg* GetInstance983(); // 005b81a0
	static bool IsCreated983(); // 005b87a0
	// 005babd0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCharMasteryCardNoticeDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005b82c0
	void SetCard983(unsigned long cardId, bool attach); // 005b9e90

	static RFCharMasteryCardNoticeDlg* ms_pInstance; // 00e78114
	static RFWnd* s_pParent983; // 00e78110

	RFPicture* m_pCardImg983; // +0x188
	RFTextArea* m_pMessage983; // +0x18c
};
C_ASSERT_VC10(sizeof(RFCharMasteryCardNoticeDlg) == 0x190);

class RFCharMasteryCardSelectColumn : public RFFormBase<RFWndEx>
{
public:
	RFCharMasteryCardSelectColumn(); // 005b88f0
	virtual ~RFCharMasteryCardSelectColumn(); // 005b8920 (??_G 005b9820)

	virtual void OnInitialUpdate(); // 005bbec0 (slot 10)

	// 005baf20: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharMasteryCardSelectColumn* Create983(RFWnd* parent);
	void SetCard983(sCards* pCard); // 005ba0f0: imagem e "x%d nome"
	void OnClick983(EventArgs& e); // 005bba60: passa a carta para a RFCharMasteryCardSelectDlg

	static RFWnd* s_pParent983; // 00e78118

	sCards* m_pCard983; // +0x184
	void* m_188_983; // +0x188 (só o construtor zera)
	RFPicture* m_pCardImg983; // +0x18c
	RFLabel* m_pName983; // +0x190
};
C_ASSERT_VC10(sizeof(RFCharMasteryCardSelectColumn) == 0x194);

class RFCharMasteryCardSelectDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFCharMasteryCardSelectDlg(); // 005b8930
	virtual ~RFCharMasteryCardSelectDlg(); // 005b8a00 (??_G 005b9850; 2ª base 005b89f0)

	virtual void OnInitialUpdate(); // 005bc0f0 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005b8320 (slot 34): ESC fecha

	static RFCharMasteryCardSelectDlg* GetInstance983(); // 005b81b0
	static RFCharMasteryCardSelectDlg* GetInstance2_983(); // 005b81c0 (cópia igual)
	void SetInstance983(); // 005b81d0
	static void ClearInstance983(); // 005b81e0
	bool IsSent983(); // 005b81f0
	void SetSent983(bool sent); // 005b8200
	static bool IsCreated983(); // 005b87b0
	void OnCancel983(EventArgs& e); // 005b8340
	void UpdateOkButton983(); // 005b9880: encaixe 9/10 sem o item 0x1a00018f = ok desligado
	void UpdateUseItem983(); // 005ba2c0: o item 0x1a00018f (imagem, "nome x%d") e o aviso
	void UpdateNotice983(int type); // 005ba8e0
	// 005bb060: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCharMasteryCardSelectDlg* Create983(RFWnd* parent);
	void SetCard983(sCards* pCard); // 005bb200: carta escolhida (imagem + animação _open_card_img_)
	void SetTitle983(int type); // 005bb360
	void OnOk983(EventArgs& e); // 005bbab0: manda 0x18a/0x18b
	void Set983(int slot); // 005bc080: lista as cartas do tipo do encaixe

	static RFCharMasteryCardSelectDlg* ms_pInstance; // 00e78124
	static RFWnd* s_pParent983; // 00e78120

	RFListSC* m_pList983; // +0x188 ("card_list")
	RFPicture* m_pTitle983; // +0x18c ("title_card_type_img")
	RFPicture* m_pCardImg983; // +0x190
	RFPicture* m_pUseItemImg983; // +0x194
	RFTextArea* m_pUseItemText983; // +0x198
	RFTextArea* m_pNotice983; // +0x19c
	RFTextArea* m_pWarning983; // +0x1a0 ("warnning_text")
	RFButton* m_pOk983; // +0x1a4
	sCards* m_pCard983; // +0x1a8 (a escolhida)
	int m_slot983; // +0x1ac (−1 = nenhum)
	bool m_sent983; // +0x1b0 (pacote já mandado)
};
C_ASSERT_VC10(sizeof(RFCharMasteryCardSelectDlg) == 0x1b4);

class RFCharacterMasteryCardDetachDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFCharacterMasteryCardDetachDlg(); // 005b8a70
	virtual ~RFCharacterMasteryCardDetachDlg(); // 005b8b10 (??_G 005b98d0; 2ª base 005b8b00)

	virtual void OnInitialUpdate(); // 005bc9a0 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005b8380 (slot 34): ESC fecha

	static RFCharacterMasteryCardDetachDlg* GetInstance983(); // 005b8210
	static RFCharacterMasteryCardDetachDlg* GetInstance2_983(); // 005b8220 (cópia igual)
	void SetInstance983(); // 005b8230
	static void ClearInstance983(); // 005b8240
	static bool IsCreated983(); // 005b87c0
	void OnClose983(EventArgs& e); // 005b83e0
	void SetCard983(IFF_STRUCT::sCard* pCard, int slot); // 005ba570
	// 005bb460: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFCharacterMasteryCardDetachDlg* Create983(RFWnd* parent);
	void OnUseItem983(EventArgs& e); // 005bbc50: ok da RFUseItemDlg → manda 0x18c
	void OnDetach983(EventArgs& e); // 005bc6b0: pede o item 0x1a0000c2 (RFUseItemDlg) ou avisa que falta

	static RFCharacterMasteryCardDetachDlg* ms_pInstance; // 00e7812c
	static RFWnd* s_pParent983; // 00e78128

	RFPicture* m_pCardImg983; // +0x188 ("card_origin_img")
	int m_slot983; // +0x18c (−1 = nenhum)
};
C_ASSERT_VC10(sizeof(RFCharacterMasteryCardDetachDlg) == 0x190);

class RFCardIcon : public RFFormBase<RFWndEx>
{
public:
	RFCardIcon(); // 005b8b80
	virtual ~RFCardIcon() { } // ??_G 005b8bd0 (cópia fora de linha 005b8bc0)

	virtual void OnInitialUpdate(); // 005bccc0 (slot 10)

	void SetCard983(IFF_STRUCT::sCard* pCard, int slot); // 005b8420
	void SetState983(bool special, bool open); // 005b8440: encaixe 6 só abre com o 7º encaixe
	static RFWnd* GetParent983(); // 005b8250
	void Update983(); // 005ba670: imagem da carta; "card_img_not" (fechado com carta) / "card_blank" (aberto e vazio)
	void OnEnter983(EventArgs& e); // 005baad0: dica do encaixe fechado
	void OnLeave983(EventArgs& e); // 005baba0
	// 005bb5a0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCardIcon* Create983(RFWnd* parent);
	void OnClick983(EventArgs& e); // 005bc880: vazio → escolher carta; com carta → tirar

	static RFWnd* s_pParent983; // 00e78130

	int m_slot983; // +0x184 (−1 = nenhum)
	IFF_STRUCT::sCard* m_pCard983; // +0x188
	enum { IMG_CARD983 = 0, IMG_CLOSED983 = 1, IMG_BLANK983 = 2 };
	// +0x18c: "card_img", "card_img_not" (fechado com carta), "card_blank" (aberto e vazio); array porque o construtor zera por memset
	RFPicture* m_pImg983[3];
	bool m_bSpecial983; // +0x198 (encaixe 6)
	bool m_bOpen983; // +0x199
};
C_ASSERT_VC10(sizeof(RFCardIcon) == 0x19c);

class RFCharMasteryCardDlg : public RFFormBase<RFWndEx>
{
public:
	RFCharMasteryCardDlg(); // 005b8c00
	virtual ~RFCharMasteryCardDlg(); // 005b8c60 (??_G 005b9900)

	virtual void OnInitialUpdate(); // 005bbe70 (slot 10)

	static RFCharMasteryCardDlg* GetInstance983(); // 005b8260
	static RFCharMasteryCardDlg* GetInstance2_983(); // 005b8270 (cópia igual)
	void SetInstance983(); // 005b8280
	static void ClearInstance983(); // 005b8290
	static bool IsCreated983(); // 005b87d0
	static void CloseDialogs983(); // 005b8c80: fecha a RFUseItemDlg e a RFCharacterMasteryCardDetachDlg
	void ShowTip983(bool show, const char* text); // 005ba830: dica junto do mouse
	// 005bb6e0: cria pelo xml e guarda o pai
	static RFCharMasteryCardDlg* Create983(RFWnd* parent);
	// 005bb820: resposta do servidor (fecha as escolhas e mostra a RFCharMasteryCardNoticeDlg, filha desta); o CTask chama
	void OnResult983(unsigned long cardId, bool attach);
	void InitSlots983(); // 005bb880: um RFCardIcon no lugar de cada slot_wnd1…10
	void SetChar983(unsigned long charId); // 005bbdb0: −1 = nada; redesenha os 10 encaixes de carta

	static RFCharMasteryCardDlg* ms_pInstance; // 00e7813c
	static RFWnd* s_pParent983; // 00e78138

	int m_184_983; // +0x184 (ninguém mexe)
	RFCardIcon* m_pSlot983[10]; // +0x188 (encaixe 1–10; array porque o construtor zera por memset)
	RFMasteryToolTipWnd* m_pTip983; // +0x1b0
};
C_ASSERT_VC10(sizeof(RFCharMasteryCardDlg) == 0x1b4);
