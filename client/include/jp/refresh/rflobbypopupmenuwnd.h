#pragma once
#include "vc10_layout.h"
#include "rfbase.h"

// JP 983: menu de botões do lobby RF (unidade rflobbypopupmenuwnd.cpp, 005d5d20–005d71e0). "lobby_popupmenu_wnd_jp.xml"
// (RTTI .?AVRFLobbyPopupMenuWnd@@, vtable 00d113d4; base RFFormBase<RFWndEx>; 0x184 bytes, sem campos próprios). Fica dentro
// do "panel_popup" do RFLobbyDesktop (criada no OnInitialUpdate 005fd000); cada botão chama um OnMenu*983 da área de trabalho e
// a perda de foco fecha o menu. Instância 00e78230. Nomes dos métodos meus (…983).
class RFLobbyPopupMenuWnd : public RFFormBase<RFWndEx>
{
public:
	RFLobbyPopupMenuWnd(); // 005d5d20
	virtual ~RFLobbyPopupMenuWnd(); // 005d5d40 (??_G 005d60b0)

	virtual void OnInitialUpdate(); // 005d62b0 (slot 10)

	// 005d6170: cria pelo "lobby_popupmenu_wnd_jp.xml" (modo g_rfCreateMode983) e guarda o pai
	static RFLobbyPopupMenuWnd* Create983(RFWnd* parent);

	void OnKillFocus983(EventArgs& e); // 005d5d60: mouse fora do menu = foco no "panel_popup"; e o tratador da área de trabalho
	void OnScratchClick983(EventArgs& e); // 005d5dc0 "btn_scratch"
	void OnBongClick983(EventArgs& e); // 005d5de0 "btn_bong"
	void OnItemTransformClick983(EventArgs& e); // 005d5e00 "btn_itemtransform"
	void OnQuestClick983(EventArgs& e); // 005d5e20 "btn_quest"
	void OnNoticeClick983(EventArgs& e); // 005d5e40 "btn_notice"
	void OnRankingClick983(EventArgs& e); // 005d5e60 "btn_ranking"
	void OnAchieveClick983(EventArgs& e); // 005d5e80 "btn_achieve"
	void OnMyGuildClick983(EventArgs& e); // 005d5ea0 "btn_myguild"
	void OnCenterClick983(EventArgs& e); // 005d5ec0 "btn_center"
	void OnGachaClick983(EventArgs& e); // 005d5ee0 "btn_gacha"

	static RFLobbyPopupMenuWnd* ms_pInstance; // 00e78230
	static RFWnd* s_pParent983; // 00e7822c
};
C_ASSERT_VC10(sizeof(RFLobbyPopupMenuWnd) == 0x184);
