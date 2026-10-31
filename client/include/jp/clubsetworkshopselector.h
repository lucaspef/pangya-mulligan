#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetWorkshopSelector (unidade 004a39f0–004a55a0; vtables 00d01774 / 00d01768 / 00d01760; RTTI 00e7428c; mapa de
// mensagens 00e04d30; sizeof 0x180 pelo `operator new` de 004a4e30, construtor 004a4bd0). Diálogo "selectclubset" de escolher o
// taco da oficina: lista "lt_item" com os tacos do jogador (CSharedDoc::m_clubSetMap) filtrados pelo uso (`m_kind`), "bt_ok" /
// "bt_cancel" e o aviso "static_notice". Quem abre põe `m_kind` e os tacos já escolhidos em `m_marked`; o resultado volta pelo
// retorno do Open (`m_selTid` / `m_selGuid`). Base FrForm direta (não a ClubSetWorkShopForm). Existe no 839 (par 004a7480…), não
// no 645. Layout pelo construtor, destrutor e métodos (sessão B90); nomes dos métodos e dos campos são meus.
#include <vector>
#include "frform.h"

class FrListBox;
class FrButton;
class FrStatic;
class CWindFont;
class CSmallNumFont;
class CClubSetInfoFont;
struct sItemInfo;
namespace IFF_STRUCT { struct sClubSet; }

class FrClubSetWorkshopSelector : public FrForm
{
	DECLARE_OBJECT(FrClubSetWorkshopSelector) // GetRTTI 004a4c60; MakeInstance 004a4e30; registro 00cc8c60

public:
	// uso do diálogo (quem abre grava em `m_kind`)
	enum eKind
	{
		KIND_LEVELUP = 1, // FrClubSetLevelUpWnd
		KIND_RANKUP = 2, // FrClubSetRankUpWnd
		KIND_EXP_FROM = 3, // FrClubSetOptionalFunctionsWnd: mover experiência, taco de origem
		KIND_EXP_TO = 4, // … taco de destino (só os que sobem de rank e ainda não estão no S)
		KIND_RECOVERY = 5, // … recuperar cancelamentos
		KIND_RESET = 6, // FrClubSetResetWnd: só os tacos mudados
	};
	// item da lista (8 bytes; `pData` do FrListItem)
	struct sItem983
	{
		sItemInfo* pItem; // o taco do jogador (valor do m_clubSetMap)
		IFF_STRUCT::sClubSet* pClub; // ClubSet.iff
	};

	FrClubSetWorkshopSelector(); // 004a4bd0
	virtual ~FrClubSetWorkshopSelector(); // 004a4c90 (com `delete`: 004a4ec0; ajustes 004a4c70 / 004a4c80)

	virtual bool OnInit(); // 004a4ef0

	// 004997e0 / 004997f0 (unidade da FrClubSetInfoDlg; sem chamador no 983; nomes meus)
	void SetKind983(int kind);
	unsigned long GetSelGuid983();

protected:
	void LoadLayout983(); // 004a4120: clubset_workshop.lua, tabela "clubset_workshop_ui_selector"
	bool IsMarked983(unsigned long guid); // 004a3c80

	void OnItemInit(int param); // 004a3cc0
	void OnItemDraw(int param); // 004a4540
	void OnItemClick(); // 004a3d30
	void OnItemDblClick(); // 004a3d90
	void OnItemHoverOn(int param); // 004a3a20
	void OnItemHoverOff(int param); // 004a3a30
	void OnOkInit(int param); // 004a3de0
	void OnOkClick(); // 004a3a40
	void OnCancelClick(); // 004a3a50
	void OnNoticeInit(int param); // 004a3e50

	std::vector<sItem983> m_items; // +0x110
	FrListBox* m_pList; // +0x120: "lt_item"
	FrButton* m_pOk; // +0x124: "bt_ok" (o construtor não zera)
	FrStatic* m_pNotice; // +0x128: "static_notice" (o construtor não zera)
	CWindFont* m_pWindFont; // +0x12c: cancelamentos restantes ("%d/%d")
	CSmallNumFont* m_pNumFont; // +0x130: experiência
	CClubSetInfoFont* m_pInfoFont; // +0x134: nível e rank
	float m_valuePos[2]; // +0x138: "level_img_pos" / "exp_imgfont_pos" / "recovery_imgfont_pos", conforme o uso
	float m_rankPos[2]; // +0x140: "rank_img_pos"
	float m_hoverPos[2]; // +0x148: "hover_img_pos"
	float m_selectedPos[2]; // +0x150: "selected_img_pos"
public:
	int m_kind; // +0x158 (eKind)
	unsigned long m_selTid; // +0x15c: typeId do taco escolhido (a LevelUp/RankUp guardam em +0xb8)
	unsigned long m_selGuid; // +0x160: o taco escolhido
	std::vector<unsigned long> m_marked; // +0x164: tacos já escolhidos (não podem ser escolhidos de novo)
protected:
	int m_pad983_174; // +0x174: zerado no construtor, sem uso visto
	const char* m_selectedImg; // +0x178: "selected_img_filename"
	const char* m_hoverImg; // +0x17c: "hover_img_filename"

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004a39f0; mapa 00e04d30 (10 entradas)
};
C_ASSERT_VC10(sizeof(FrClubSetWorkshopSelector) == 0x180);

// textos da unidade (funções livres, __fastcall pelo /Gr; K2L); nomes meus
const char* __fastcall GetUpgradeStatText983(int count); // 004a5240: atributo da carta de nível (0 = ao acaso)
const char* __fastcall GetAddSlotStatText983(int index); // 004a52b0: atributo do slot posto pela carta de rank
const char* __fastcall ClubSetTypeName983(int type); // 004a5310: tipo do taco
const char* __fastcall GetLevelUpNotice983(int r); // 004a5370: aviso da aba (r % 3)
const char* __fastcall GetRankUpNotice983(int r); // 004a53c0 (r % 3)
const char* __fastcall GetCareNotice983(int r); // 004a5410 (r % 4)
const char* __fastcall GetStatName983(int stat); // 004a5470: nome do atributo
const char* __fastcall GetRankUpTipText983(); // 004a54d0: uma de seis dicas ao acaso
const char* __fastcall GetFailTalkText983(); // 004a5550: uma de três falas ao acaso (sem chamador no exe)
const char* __fastcall GetLuckyChanceText983(); // 004a5590
