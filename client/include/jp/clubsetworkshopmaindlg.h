#pragma once
#include "vc10_layout.h"
// JP 983: FrClubSetWorkshopMainDlg (unidade 004cccd0–004ce0d4; vtables 00d039f4 / 00d039e4 / 00d039dc; RTTI 00e74cdc; mapa de
// mensagens 00e05640 com 179 entradas, montado pelo inicializador 00cc9120; s_pInstance 00e74cd8). Diálogo principal da oficina
// de tacos ("clubset_workshop"): três abas (subir nível, subir rank, cuidados) que são páginas singleton
// (ClubSetWorkshopChildWndBase + ClubSetWorkshopHelper), mais a janela de andamento e o objeto de efeitos. Quase todo o mapa de
// mensagens repassa às páginas. sizeof 0x170 pelo `operator new` de 004cce70. Existe no 839 (par 004d0e00…), não no 645.
// Nomes dos métodos e dos campos são meus (sessões B77/B78).
#include "clubsetworkshopform.h"

class FrClubSetLevelUpWnd;
class FrClubSetRankUpWnd;
class FrClubSetOptionalFunctionsWnd;
class FrClubSetWorkshopProgressWnd;
class ClubSetWorkshopFx983;
namespace lua_system { class lua_table_helper; }

class FrClubSetWorkshopMainDlg : public ClubSetWorkShopForm
{
	DECLARE_OBJECT(FrClubSetWorkshopMainDlg) // GetRTTI 004ccdb0; MakeInstance 004cce70; registro 004ccee0

public:
	static FrClubSetWorkshopMainDlg* s_pInstance; // 00e74cd8

	FrClubSetWorkshopMainDlg(); // 004cccf0
	virtual ~FrClubSetWorkshopMainDlg(); // 004ccf00 (com `delete`: 004cd720; ajustes 004cce10 / 004cce20)

	// 004cd710 (depois do CreateForm<FrClubSetWorkshopMainDlg> 004cd640); chamado em 0061f850
	static FrClubSetWorkshopMainDlg* __fastcall Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest);

	virtual bool OnInit(); // 004ce000

	// 004cd7f0: EnableCtrl983(this, "btn_tab_levelup"/"btn_tab_rankup"/"btn_tab_care"/"btn_help"/"btn_exit", false)
	void DisableTabs983();
	// 004cd8c0: o mesmo com true
	void EnableTabs983();
	// 004cdb70: troca a aba (0 = subir nível, 1 = subir rank, 2 = cuidados): botões, páginas e o aviso
	void SetTab983(int tab);

protected:
	virtual void OnProc(const float deltaTime); // 004ccff0: só as páginas (não chama a base)
	virtual void OnCancel(); // 004cd7e0: = OnExitBtnUp983
	virtual FrWnd* OnCtrlCreated983(IObject* pObj); // 004cccd0: AddCtrlByType983

	void OnTabLevelUpDown983() { SetTab983(0); } // 004cdcb0
	void OnTabRankUpDown983() { SetTab983(1); } // 004cdcc0
	void OnTabCareDown983() { SetTab983(2); } // 004cdcd0
	void OnHelpBtnDown983(); // 004cd070: abre a FrClubSetWorkshopAboutDlg
	void OnExitBtnUp983(); // 004cd750: pergunta ("notify_okcancel")
	bool OnExitConfirmResult983(int result, FrForm* pForm); // 004ccdf0
	bool OnExitKey983(); // 004cd7d0: OnExitBtnUp983 e true (sem chamador no 983)

	// criadores das páginas: métodos da MainDlg (`ecx` = this, sem uso) escritos nas unidades das páginas (o do
	// ClubSetWorkshopFx983 é estático da classe: o 983 não recarrega o `ecx` antes dele)
	FrClubSetLevelUpWnd* CreateLevelUp983(); // 004eb1c0: new FrClubSetLevelUpWnd (0xd8)
	FrClubSetRankUpWnd* CreateRankUp983(); // 004c7490: new FrClubSetRankUpWnd (0xd4)
	FrClubSetOptionalFunctionsWnd* CreateCare983(); // 004c2250: new FrClubSetOptionalFunctionsWnd (0x160)
	FrClubSetWorkshopProgressWnd* CreateProgress983(); // 0049fde0: new FrClubSetWorkshopProgressWnd (0x100)

	void ShowNotice983(int tab); // 004cdad0: texto sorteado da aba no "static_notice_clubworkhop"
	void LoadBg983(lua_system::lua_table_helper* pTbl); // 004cd9a0: "area_bg%d"
	void LoadTabs983(lua_system::lua_table_helper* pTbl); // 004cdce0: posição das abas e a aba 0
	void LoadLayout983(); // 004cded0: clubset_workshop.lua, tabela "clubset_workshop_ui_main"

	// repassadores do mapa de mensagens (004cd0c0–004cd450): `if (Pagina::s_pInstance) s_pInstance->F();`
	void OnLevelUpDrawNpcImg983(); // 004cd0c0
	void OnLevelUpDrawLogo983(); // 004cd0e0
	void OnLevelUpDrawClubinfo983(); // 004cd100
	void OnLevelUpDrawItem983(); // 004cd110
	void OnLevelUpConfirmBtnUp983(); // 004cd120
	void OnLevelUpClubselectBtnUp983(); // 004cd130
	void OnLevelUpDrawItemNum983(); // 004cd140
	void OnLevelUpCardAddBtnUp983(); // 004cd150
	void OnLevelUpCardSubBtnUp983(); // 004cd160
	void OnLevelUpDrawCardNum983(); // 004cd170
	void OnLevelUpDrawNpcSpeech983(); // 004cd180
	void OnRankUpDrawNpcImg983(); // 004cd190
	void OnRankUpDrawLogo983(); // 004cd1b0
	void OnRankUpDrawClubinfo983(); // 004cd1d0
	void OnRankUpDrawItem983(); // 004cd1e0
	void OnRankUpConfirmBtnUp983(); // 004cd1f0
	void OnRankUpClubselectBtnUp983(); // 004cd200
	void OnRankUpCardAddBtnUp983(); // 004cd210
	void OnRankUpCardSubBtnUp983(); // 004cd220
	void OnRankUpDrawCardNum983(); // 004cd230
	void OnRankUpDrawNpcSpeech983(); // 004cd240
	void OnCareDrawNpcImg983(); // 004cd250
	void OnCareDrawLogo983(); // 004cd270
	void OnCareDrawExpmoveIcon983(); // 004cd290
	void OnCareDrawClubselectExpmove0_983(); // 004cd2a0
	void OnCareDrawClubselectExpmove1_983(); // 004cd2b0
	void OnCareConfirmExpmoveBtnUp983(); // 004cd2c0
	void OnCareClubselectExpmove0BtnUp983(); // 004cd2d0
	void OnCareClubselectExpmove1BtnUp983(); // 004cd2e0
	void OnCareExpAddBtnUp983(); // 004cd2f0
	void OnCareExpSubBtnUp983(); // 004cd300
	void OnCareDrawExpmoveItemIcon983(); // 004cd310
	void OnCareDrawClubselectRecovery983(); // 004cd320
	void OnCareConfirmRecoveryBtnUp983(); // 004cd330
	void OnCareClubselectRecoveryBtnUp983(); // 004cd340
	void OnCareDrawRecoveryItemIcon983(); // 004cd350
	void OnCareDrawItemNumRecovery983(); // 004cd360
	void OnCareDrawNpcSpeech983(); // 004cd370
	void OnCareDrawFxExpmove983(); // 004cd380
	void OnCareDrawFxRecovery983(); // 004cd390
	void OnProgressDrawProgressEclipse983(); // 004cd3a0
	void OnProgressDrawProgressSubwndBg983(); // 004cd3b0
	void OnProgressDrawProgressAnimation983(); // 004cd3c0
	void OnProgressDrawProgressbar983(); // 004cd3d0
	void OnProgressWorkshopApplyBtnUp983(); // 004cd3e0
	void OnProgressWorkshopCancelBtnUp983(); // 004cd3f0
	void OnProgressWorkshopOkBtnUp983(); // 004cd400
	void OnProgressDrawSpeakballoonRankup983(); // 004cd410
	void OnProgressDrawProgressResultTitle983(); // 004cd420
	void OnProgressDrawProgressResultClubset983(); // 004cd430
	void OnProgressDrawProgressResultLevel983(); // 004cd440
	void OnProgressDrawProgressResultRank983(); // 004cd450

public:
	// retornos de diálogo que as páginas abrem com a MainDlg como destino (004cd460–004cd620): repassam e devolvem true
	bool IsRefresh983() const { return m_bRefresh983; } // CRealMyRoomForm983::OnClubWorkResult983 (0061e4a0 le +0x168)
	void SetRefresh983() { m_bRefresh983 = true; } // as páginas, ao chegar o resultado (B79: LevelUp 004ea8cb)
	bool OnLevelUpClubSelected983(int result, FrForm* pForm); // 004cd460
	bool OnRankUpClubSelected983(int result, FrForm* pForm); // 004cd480
	bool OnCareExpMoveSel0Result983(int result, FrForm* pForm); // 004cd4a0
	bool OnCareExpMoveSel1Result983(int result, FrForm* pForm); // 004cd4c0
	bool OnCareRecoverySelResult983(int result, FrForm* pForm); // 004cd4e0
	bool OnLevelUpConfirmResult983(int result, FrForm* pForm); // 004cd500
	bool OnRankUpConfirmResult983(int result, FrForm* pForm); // 004cd520
	bool OnCareExpMoveConfirmResult983(int result, FrForm* pForm); // 004cd540
	bool OnCareRecoveryConfirmResult983(int result, FrForm* pForm); // 004cd560
	bool OnCareNotifyResultA983(int result, FrForm* pForm); // 004cd580
	bool OnCareNotifyResultB983(int result, FrForm* pForm); // 004cd5a0
	bool OnProgressApplyResult983(int result, FrForm* pForm); // 004cd5c0
	bool OnProgressCancelResult983(int result, FrForm* pForm); // 004cd5e0
	bool OnProgressApplyResultB983(int result, FrForm* pForm); // 004cd600
	bool OnProgressCancelConfirmResult983(int result, FrForm* pForm); // 004cd620

protected:
	FrClubSetLevelUpWnd* m_pLevelUp; // +0x140 (aba 0)
	FrClubSetRankUpWnd* m_pRankUp; // +0x144 (aba 1)
	FrClubSetOptionalFunctionsWnd* m_pCare; // +0x148 (aba 2)
	FrClubSetWorkshopProgressWnd* m_pProgress; // +0x14c
	ClubSetWorkshopFx983* m_pFx; // +0x150
	int m_tab; // +0x154: 3 no início (nenhuma)
	int m_pad983_158; // +0x158: zerado no construtor
	unsigned char m_pad983_15c[0xc]; // +0x15c: sem uso visto
	bool m_bRefresh983; // +0x168: resultado da oficina chegou (SetRefresh983; lido em 0061e4a0)

	DECLARE_FRESH_MSGMAP() // GetMessageMap 004ccce0; mapa 00e05640
};
C_ASSERT_VC10(sizeof(FrClubSetWorkshopMainDlg) == 0x170);
