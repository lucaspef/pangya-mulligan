#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetOptionalFunctionsWnd (unidade 004c0690–004c6280; vtables 00d02b34 / 00d02b2c; s_pInstance 00e74cd0; sizeof 0x160
// pelo `operator new` de 004c2250). Aba "cuidados" (btn_tab_care) da FrClubSetWorkshopMainDlg, com duas partes:
// - mover experiência ("expmove"): taco de origem (slot 2 do Fx) → taco de destino (slot 3), 300 de experiência por item usado,
//   destino no máximo 99.900; envia 0x16c;
// - recuperar ("recovery"): devolve ao máximo as vezes que o resultado de um trabalho pode ser cancelado (slot 4); envia 0x16b.
// Layout medido no construtor 004c1db0 e no destrutor 004c1540 (sessão B82). Nomes dos métodos e dos campos são meus.
// A LoadLayout983 (004c3ea0, 9.074 B) escrita na sessão B83.
#include <vector>
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopfx.h"

class FrForm;
class Bitmap;
class WReceivedPacket;

class FrClubSetOptionalFunctionsWnd : public ClubSetWorkshopChildWndBase, public ClubSetWorkshopHelper
{
public:
	static FrClubSetOptionalFunctionsWnd* s_pInstance; // 00e74cd0
	// criador: FrClubSetWorkshopMainDlg::CreateCare983 (004c2250)

	FrClubSetOptionalFunctionsWnd(ClubSetWorkShopForm* pForm); // 004c1db0
	virtual ~FrClubSetOptionalFunctionsWnd(); // 004c1540 (com `delete`: 004c1ef0; ajuste do Helper 004c1620)

	virtual void SetShow983(bool bShow); // 004c3e60 (+4)
	virtual void OnBeginWork983(); // 004c1c40 (+8)
	virtual void OnEndWork983(); // 004c0690 (+0xc): só repassa à base
	virtual void OnDrawNpcImg983(); // 004c31b0 (+0x14)
	virtual void OnDrawLogo983(); // 004c32c0 (+0x1c)
	virtual bool Init983(); // 004c1630 (+0x20): chamado pelo OnInit da MainDlg
	virtual void Proc983(float deltaTime); // 004c0e30 (+0x24): chamado pelo OnProc da MainDlg
	void LoadLayout983(); // 004c3ea0: posições do clubset_workshop.lua (B83)

	// controles do mapa de mensagens da MainDlg (repassados por ela)
	void OnDrawExpmoveIcon983(); // 004c33d0
	void OnDrawClubselectExpmove0_983(); // 004c07c0
	void OnDrawClubselectExpmove1_983(); // 004c0920
	void OnConfirmExpmoveBtnUp983(); // 004c0fc0
	void OnClubselectExpmove0BtnUp983(); // 004c1f20
	void OnClubselectExpmove1BtnUp983(); // 004c1fc0
	void OnExpAddBtnUp983(); // 004c3590
	void OnExpSubBtnUp983(); // 004c35c0
	void OnDrawExpmoveItemIcon983(); // 004c06a0 (vazia)
	void OnDrawClubselectRecovery983(); // 004c0a50
	void OnConfirmRecoveryBtnUp983(); // 004c1140
	void OnClubselectRecoveryBtnUp983(); // 004c2060
	void OnDrawRecoveryItemIcon983(); // 004c06b0 (vazia)
	void OnDrawItemNumRecovery983(); // 004c35f0
	void OnDrawNpcSpeech983(); // 004c37a0
	void OnDrawFxExpmove983(); // 004c3880
	void OnDrawFxRecovery983(); // 004c3940
	bool OnExpMoveSel0Result983(int result, FrForm* pForm); // 004c3a00
	bool OnExpMoveSel1Result983(int result, FrForm* pForm); // 004c3a50
	bool OnRecoverySelResult983(int result, FrForm* pForm); // 004c3aa0
	bool OnExpMoveConfirmResult983(int result, FrForm* pForm); // 004c3af0
	bool OnRecoveryConfirmResult983(int result, FrForm* pForm); // 004c3b60
	bool OnNotifyResultA983(int result, FrForm* pForm); // 004c3cc0: fim do aviso "experiência movida"
	bool OnNotifyResultB983(int result, FrForm* pForm); // 004c3d00: fim do aviso "recuperado"

	// respostas do servidor (chamadas pelo 009d57b0 com o s_pInstance; NULL = falhou)
	void OnRecoveryResult983(WReceivedPacket* pPacket); // 004c3dd0
	void OnExpMoveResult983(WReceivedPacket* pPacket); // 004c3e20

protected:
	void Notify983(int kind); // 004c0c70: aviso "notify" (1 = experiência movida, 2 = recuperado)
	void UpdateRecoveryItem983(); // 004c1b90: ícone do item de recuperar
	void UpdateExpMoveClubs983(); // 004c2340: os dois tacos (figura, experiência, dica)
	void UpdateExpMove983(); // 004c27a0: experiência movida, item, botões +/−
	void UpdateExpMoveConfirm983(); // 004c2ba0: botão confirmar e o motivo
	void UpdateRecoveryClub983(); // 004c2dd0: taco, "cancelamentos / máximo", dica
	void UpdateRecoveryConfirm983(); // 004c2fb0: botão confirmar e o motivo
	void UpdateExpItem983(bool bKeepSlots); // 004c3bd0: itens de mover experiência que o jogador tem
	void UpdateRecoveryItemCount983(bool bKeepSlot); // 004c3c60: itens de recuperar que o jogador tem
	void ResetExpMove983(); // 004c3d30
	void ResetRecovery983(); // 004c3d80
	void ResetAll983(); // 004c3e90: chamada pela LoadLayout983

	std::vector<unsigned long> m_v78; // +0x78 (como na LevelUp/RankUp; tipo a conferir)
	unsigned long m_srcGuid; // +0x88: taco de origem (slot 2)
	unsigned long m_dstGuid; // +0x8c: taco de destino (slot 3)
	unsigned long m_expItemTypeId; // +0x90: item de mover experiência (GetTypeId983(3))
	int m_expItemUse; // +0x94: itens a usar
	int m_srcExp; // +0x98
	int m_dstExp; // +0x9c (não zerado no construtor)
	int m_moveExp; // +0xa0: 300 por item, no máximo m_srcExp
	const char* m_expItemName; // +0xa4
	unsigned long m_recGuid; // +0xa8: taco a recuperar (slot 4)
	unsigned long m_recItemTypeId; // +0xac: item de recuperar (GetTypeId983(4))
	Bitmap* m_clubBmp[3]; // +0xb0: figura dos tacos (origem, destino, recuperar)
	int m_clubBgOverlay[3]; // +0xbc (índices no Helper)
	float m_clubPos[3][2]; // +0xc8: canto de cada taco no form
	float m_recNumPos[2]; // +0xe0: deslocamento do número de itens de recuperar
	int m_selTextOverlay[3]; // +0xe8: figura que pisca enquanto não há taco escolhido
	float m_selTextPos[2]; // +0xf4
	int m_rankOverlay[2]; // +0xfc: marca do rank máximo (origem, recuperar)
	float m_rankPos[2]; // +0x104
	const char* m_expIconName[2]; // +0x10c: figuras do "area_expmove_icon"
	sOverlayAnim983 m_expAnim; // +0x114 (0x1c): seta animada entre os dois tacos
	const char* m_expAnimName; // +0x130
	float m_expAnimPos[2]; // +0x134
	float m_clubBgPos[2]; // +0x13c
	int m_expItemCount; // +0x144: itens de mover experiência que o jogador tem
	int m_recItemCount; // +0x148: itens de recuperar
	float m_notifyTime; // +0x14c
	float m_notifyDelay; // +0x150
	int m_notifyKind; // +0x154: aviso pendente (1/2), ver Notify983
	float m_blinkTime; // +0x158
	bool m_bBlink; // +0x15c
	bool m_bExpSending; // +0x15d: 0x16c enviado
	bool m_bRecSending; // +0x15e: 0x16b enviado
};
C_ASSERT_VC10(sizeof(FrClubSetOptionalFunctionsWnd) == 0x160);
