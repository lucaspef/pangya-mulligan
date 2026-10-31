#pragma once
#include "vc10_layout.h"
#include "rfformmodalbase.h"
#include "fxbox.h"
#include "fx.h"
#include "statushelper.h"

class RFMasteryToolTipWnd;
namespace _gateway { namespace _questoneday { class CQuestDocument; } }
struct sCharacterInfo;

// JP 983: janelas RF dos encaixes de maestria do personagem (unidade rfcharmasterytiledlg.cpp, 005c0260–005c5a20; sessão B139).
// Classes pela RTTI; métodos e campos com nome meu (…983), menos os da RF. Registro de maestria = sCharacterMastery983
// (CharacterMastery.iff; tipo 1 força, 2 controle, 3 precisão, 4 efeito, 5 curva).
// - RFMasteryslotNoticeDlg ("mastery_slot_notice_dlg.xml"; vtables 00d0f424 / 00d0f41c; 0x194): aviso "encaixe aberto" com a
//   imagem do encaixe; instância 00e78178.
// - RFMasteryslotConfirmDlg ("mastery_slot_confirm_dialog.xml"; vtables 00d0f4bc / 00d0f4b4; 0x194): "abrir o encaixe?"
//   (ok manda o pacote 0x187 pela RFCharMasteryTileDlg).
// - RFMasteryTileColumn ("slot_mastery_column.xml"; vtable 00d0f54c; 0x198): um encaixe da tabela.
// - RFCharMasteryTileDlg ("slot_mastery_wnd.xml"; vtable 00d0f67c; 0x1ec): a tabela de encaixes (2 listas de 5) do
//   personagem em uso; instância 00e78190 (pedido C76.2).
// - RFSoltAniWnd ("kam_mastery_slotani.xml"; vtables 00d0fb6c / 00d0fb64; 0x5f8): a animação do encaixe aberto (CPvsFxBox
//   com câmera própria); ao fechar, avisa a RFCharMasteryTileDlg.

class RFMasteryslotNoticeDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFMasteryslotNoticeDlg(); // 005c0d20
	virtual ~RFMasteryslotNoticeDlg(); // 005c0dc0 (??_G 005c1630; 2ª base 005c0db0)

	virtual void OnInitialUpdate(); // 005c3340 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005c0e30 (slot 34): Enter/ESC = ok; assinado direto (005c0820 = thunk de vcall ??_9 {136}, dobrado nas duas)

	static RFMasteryslotNoticeDlg* GetInstance983(); // 005c0260
	void SetInstance983(); // 005c0270
	static void ClearInstance983(); // 005c0280
	static bool IsCreated983(); // 005c0830
	// 005c30c0: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFMasteryslotNoticeDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005c0360
	void SetSlot983(const sCharacterMastery983* p); // 005c1dc0: "slot_image_%d_1.tga"
	void SetMessage983(const char* text); // 005c3200

	static RFMasteryslotNoticeDlg* ms_pInstance; // 00e78178
	static RFWnd* s_pParent983; // 00e78174

	RFPicture* m_pSlotImg983; // +0x188
	RFTextArea* m_pMessage983; // +0x18c
	const sCharacterMastery983* m_pMastery983; // +0x190
};
C_ASSERT_VC10(sizeof(RFMasteryslotNoticeDlg) == 0x194);

class RFMasteryslotConfirmDlg : public RFFormModalBase<RFWndEx>
{
public:
	RFMasteryslotConfirmDlg(); // 005c0e60
	virtual ~RFMasteryslotConfirmDlg(); // 005c0ef0 (??_G 005c1660; 2ª base 005c0ee0)

	virtual void OnInitialUpdate(); // 005c3880 (slot 10)
	virtual void OnKey983(EventArgs& e); // 005c1b70 (slot 34): Enter = abre o encaixe, ESC = fecha

	// 005c3600: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFMasteryslotConfirmDlg* Create983(RFWnd* parent);
	void OnOk983(EventArgs& e); // 005c03a0
	void OnCancel983(EventArgs& e); // 005c03e0
	void SetSlot983(const sCharacterMastery983* p); // 005c1ec0: "slot_image_%d_0.tga"
	void SetMessage983(const char* text); // 005c3740

	static RFWnd* s_pParent983; // 00e7817c

	RFTextArea* m_pMessage983; // +0x188
	RFPicture* m_pSlotImg983; // +0x18c
	const sCharacterMastery983* m_pMastery983; // +0x190
};
C_ASSERT_VC10(sizeof(RFMasteryslotConfirmDlg) == 0x194);

class RFMasteryTileColumn : public RFFormBase<RFWndEx>
{
public:
	enum { STATE_LOCKED983 = 0, STATE_NEXT983 = 1, STATE_OPEN983 = 2 }; // 3 = sem registro (construtor)

	RFMasteryTileColumn(); // 005c0f60
	virtual ~RFMasteryTileColumn(); // 005c0fa0 (??_G 005c1690)

	virtual void OnInitialUpdate(); // 005c45c0 (slot 10)

	// 005c3c00: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFMasteryTileColumn* Create983(RFWnd* parent);
	const sCharacterMastery983* GetMastery983(); // 005c0290
	void SetMastery983(const sCharacterMastery983* p); // 005c0420
	void ShowSelected983(bool show); // 005c0430
	void UpdateState983(int level); // 005c1fc0
	void UpdateImage983(); // 005c2050
	void Update983(int level); // 005c2970
	void OnClick983(EventArgs& e); // 005c3d40

	static RFWnd* s_pParent983; // 00e78184

	RFPicture* m_pSlotImg983; // +0x184
	RFPicture* m_pLockedImg983; // +0x188
	RFPicture* m_pSelectedImg983; // +0x18c
	const sCharacterMastery983* m_pMastery983; // +0x190
	int m_state983; // +0x194
};
C_ASSERT_VC10(sizeof(RFMasteryTileColumn) == 0x198);

class RFCharMasteryTileDlg : public RFFormBase<RFWndEx>
{
public:
	enum { TILE_PER_LIST983 = 5 };
	enum { REQ_NEEDTP983 = 0, REQ_CURTP983 = 1, REQ_LEVEL983 = 2, REQ_ACHIEVE983 = 3 }; // m_pReqText983
	enum { REQCHK_LEVEL983 = 0, REQCHK_ACHIEVE983 = 1 }; // m_pReqCheck983

	RFCharMasteryTileDlg(); // 005c2190
	virtual ~RFCharMasteryTileDlg(); // 005c2280 (??_G 005c2990)

	virtual void OnInitialUpdate(); // 005c5270 (slot 10)

	static RFCharMasteryTileDlg* GetInstance983(); // 005c02a0
	void SetInstance983(); // 005c02b0
	static void ClearInstance983(); // 005c02c0
	static bool IsCreated983(); // 005bd3e0 (na unidade rfcharmasteryaboutdlg; o CTask::OnPacketCommon chama)
	// 005c3db0: cria pelo xml (modo g_rfCreateMode983) e guarda o pai
	static RFCharMasteryTileDlg* Create983(RFWnd* parent);
	void UpdateRank983(); // 005c0450
	void OnTipLeave983(EventArgs& e); // 005c0610
	void OnExpandOk983(EventArgs& e); // 005c16c0: pacote 0x187 (tid, guid do personagem)
	void HideSelected983(); // 005c17a0
	const sCharacterMastery983* GetMastery983(int level); // 005c1840
	bool IsStuffDone983(unsigned long id); // 005c1c70
	void UpdateTotals983(); // 005c2320
	void OnRankEnter983(EventArgs& e); // 005c2460
	void OnPowerSlotEnter983(EventArgs& e); // 005c2580
	void OnPenaltyEnter983(EventArgs& e); // 005c2730
	void Select983(const sCharacterMastery983* p); // 005c2a40
	void InitRank983(); // 005c3ef0
	void InitTotals983(); // 005c4190
	void OnExpand983(EventArgs& e); // 005c4370
	void UpdateColumns983(); // 005c4810
	void LoadMastery983(); // 005c4860
	void Refresh983(); // 005c4e70
	void InitTables983(); // 005c4eb0
	void OnSlotAniEnd983(); // 005c5000
	void InitExpand983(); // 005c48f0
	void PlaySlotAni983(); // 005c59e0

	static RFCharMasteryTileDlg* ms_pInstance; // 00e78190
	static RFWnd* s_pParent983; // 00e7818c

	RFMasteryTileColumn* m_pSelected983; // +0x184 (o encaixe escolhido, em "selected_slot_wnd")
	RFPicture* m_pRankImg983; // +0x188
	RFLabel* m_pRankSlot983; // +0x18c
	RFLabel* m_pRankPenalty983; // +0x190
	RFList* m_pTable983[2]; // +0x194
	RFLabel* m_pTotal983[4]; // +0x19c (força, controle, efeito, curva)
	// +0x1ac: requisitos do encaixe escolhido (REQ_*); arrays porque o construtor do 983 os zera por memset (sessão B140)
	RFTextArea* m_pReqText983[4]; // +0x1ac (TP necessário, TP atual, limite de nível, conquista)
	RFCheck* m_pReqCheck983[2]; // +0x1bc (nível, conquista)
	RFPicture* m_pExpandedImg983; // +0x1c4
	RFButton* m_pExpandBtn983; // +0x1c8
	MasteryList983 m_list983; // +0x1cc
	sCharacterInfo* m_pChar983; // +0x1dc (personagem em uso)
	RFMasteryToolTipWnd* m_pTip983; // +0x1e0
	_gateway::_questoneday::CQuestDocument* m_pQuestDoc983; // +0x1e4 (documento 0xac)
	int m_1e8_983; // +0x1e8 (nem iniciado nem lido nesta unidade)
};
C_ASSERT_VC10(sizeof(RFCharMasteryTileDlg) == 0x1ec);

class RFSoltAniWnd : public RFFormModalBase<RFWndEx>
{
public:
	RFSoltAniWnd(); // 005c5130
	virtual ~RFSoltAniWnd(); // 005c51c0 (??_G 005c59b0; 2ª base 005c51b0)

	virtual void OnInitialUpdate(); // 005c0c30 (slot 10; cópia 005c1620)
	virtual void OnUpdate(EventArgs& e); // 005c02e0
	virtual void OnDraw(); // 005c0a80

	// 005c5870: cria pelo xml (modo g_rfModalCreateMode983) e guarda o pai
	static RFSoltAniWnd* Create983(RFWnd* parent);
	void Play983(int type); // 005c08b0: efeito do atributo no centro da janela

	static RFWnd* s_pParent983; // 00e7816c

	CPvsFxBox m_fxBox983; // +0x188
	WMatrix m_camera983; // +0x5c8
};
C_ASSERT_VC10(sizeof(RFSoltAniWnd) == 0x5f8);
