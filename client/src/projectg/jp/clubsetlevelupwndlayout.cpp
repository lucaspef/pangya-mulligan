// JP 983: unidade 004ec7b0–004edd80, uma função só: FrClubSetLevelUpWnd::LoadLayout983 (posições do clubset_workshop.lua;
// no mapa da F3.7.1 era "semclasse_004ec7b0"). Chamada pela FrClubSetWorkshopMainDlg::LoadLayout983 (004cded0). Escrita na
// sessão B80 a partir da desmontagem (build/B/s80/ll.asm). Nomes meus. Fora de linha no 983 e embutidas aqui: as três
// repassadoras da ClubSetWorkshopChildWndBase (0049e980, 004c12a0, 004c1470) e as GetCtrl983<T> a partir da 15ª chamada.
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "fredit.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "shareddoc.h"
#include "lua983.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetinfownd.h"
#include "clubsetlevelupwnd.h"

extern Fresh* g_pFresh; // 00f02340

// 004ec7b0: tipos dos itens/cartas da oficina (clubset_workshop_tid_table), figuras e posições da página
// (clubset_workshop_ui_levelup; o npc, o logo e o balão pela clubset_workshop_ui_main), quadro de dados do taco
void FrClubSetLevelUpWnd::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	{
		lua_system::lua_table_helper tidTbl = script->GetTable("clubset_workshop_tid_table");
		for (int i = 0; i < 5; i++)
			m_typeIds.push_back(((LuaHelper983&)tidTbl).GetChild983("clubset_workshop_tid_list").GetInl983<unsigned long>(i + 1));
	}
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_levelup");

	m_npcImgName = LUA983_GET(tbl, const char*, "npc_img_filename");
	m_logoName = LUA983_GET(tbl, const char*, "logo_img_filename");
	m_itemBgName = LUA983_GET(tbl, const char*, "item_bg_filename");
	LoadOverlay983(m_npcImgName, true);
	LoadOverlay983(m_logoName, true);
	LoadOverlay983(m_itemBgName, true);
	m_itemIconOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "item_select_bg_filename"), true);
	m_cardIconOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "card_select_bg_filename"), true);
	m_itemIconX = (float)((LuaHelper983&)tbl).GetChild983("item_select_bg_pos").GetInl983<int>(1);
	m_itemIconY = (float)((LuaHelper983&)tbl).GetChild983("item_select_bg_pos").GetInl983<int>(2);
	m_cardIconX = (float)((LuaHelper983&)tbl).GetChild983("card_select_bg_pos").GetInl983<int>(1);
	m_cardIconY = (float)((LuaHelper983&)tbl).GetChild983("card_select_bg_pos").GetInl983<int>(2);

	OffsetCtrlLua983("area_npc_img1", &mainTbl, "npc_img_x", "npc_img_y");
	OffsetCtrlLua983("area_logo_levelup", &mainTbl, "logo_img_x", "logo_img_y");
	OffsetCtrlLua983("area_item_levelup", &tbl, "item_wnd_x", "item_wnd_y");
	OffsetCtrlLua983("btn_confirm_levelup", &tbl, "confirm_btn_pos");
	OffsetCtrlLua983("area_card_icon_levelup", &tbl, "card_icon_pos");
	OffsetCtrlLua983("btn_card_add_levelup", &tbl, "btn_card_add_pos");
	OffsetCtrlLua983("btn_card_sub_levelup", &tbl, "btn_card_sub_pos");
	OffsetCtrlLua983("static_upgrade_stat", &tbl, "text_upgrade_stat_pos");
	OffsetCtrlLua983("static_item_name_levelup", &tbl, "item_name_text_pos");
	OffsetCtrlLua983("area_item_num_levelup", &tbl, "img_font_pos_s");
	OffsetCtrlLua983("area_card_num_levelup", &tbl, "img_font_pos");
	OffsetCtrlLua983("area_cardinfo_tooltip_levelup", &tbl, "card_info_tooltip_pos");
	OffsetCtrlLua983("area_npc_speech_levelup", &mainTbl, "img_npc_speech_pos");

	IFF_STRUCT::sItem* pItem = ItemManager()->FindItem(GetTypeId983(0));
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(GetTypeId983(1));
	FrArea* pArea = GetCtrl983<FrArea>("area_item_icon_levelup");
	if (pArea && pItem)
	{
		int x = ((LuaHelper983&)tbl).GetChild983("item_icon_pos").GetIntByIndex983(1);
		int y = ((LuaHelper983&)tbl).GetChild983("item_icon_pos").GetIntByIndex983(2);
		const Bitmap* pBmp = g_pFresh->RegisterBitmap(pItem->c.Icon);
		if (pBmp)
			pArea->SetBgImg(pBmp);
		const WRect& rc = pArea->GetRect();
		pArea->SetRect(WRect(x + rc.x, y + rc.y, rc.w, rc.h));
	}

	std::vector<FrStatic*> statics;
	statics.push_back(GetCtrl983<FrStatic>("static_clubname_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_power_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_control_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_accuracy_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_spin_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_curve_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_clubtype_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_exppoint_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_slotcount_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_power_max_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_control_max_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_accuracy_max_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_spin_max_levelup"));
	statics.push_back(GetCtrl983<FrStatic>("static_curve_max_levelup"));
	FrArea* pInfoArea = GetCtrl983<FrArea>("area_clubinfo_levelup");
	if (pInfoArea)
	{
		int x = LUA983_GET(mainTbl, int, "wnd_clubinfo_x");
		int y = LUA983_GET(mainTbl, int, "wnd_clubinfo_y");
		const WRect& rc = pInfoArea->GetRect();
		pInfoArea->SetRect(WRect(x + rc.x, rc.y + y, rc.w, rc.h));
		if (!m_pInfoWnd)
			m_pInfoWnd = new FrClubSetInfoWnd(statics, m_pForm);
		if (m_pInfoWnd)
		{
			FrButton* pBtn = GetCtrl983<FrButton>("btn_clubselect_levelup");
			if (pBtn)
				m_pInfoWnd->SetSelectBtn983(pBtn);
			m_pInfoWnd->SetPos983((int)pInfoArea->GetRect().x, (int)pInfoArea->GetRect().y);
			m_pInfoWnd->LoadLayout983(0);
			m_pInfoWnd->Reset983(1);
		}
	}

	m_npcSpeechOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "img_npc_speech_filename"), true);
	// "스테이터스 무작위 상승" (status sobe ao acaso)
	SetCtrlToolTip983("area_item_icon_levelup",
		K2L_Compatibility("\275\272\305\327\300\314\305\315\275\272 \271\253\300\333\300\247 \273\363\275\302"));
	SetCtrlToolTip983("area_card_icon_levelup", pCard ? MakeStr("%s", pCard->c.Name) : "");
	FrEdit* pEdit = GetCtrl983<FrEdit>("static_item_name_levelup");
	if (pEdit)
		pEdit->AddText(MakeStr("%s", pItem ? pItem->c.Name : ""), false, true, 0, 0);
	SelectClub983(0);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}
