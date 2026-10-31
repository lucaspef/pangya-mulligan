// JP 983: unidade 004c6280–004ca630 (FrClubSetRankUpWnd, aba "subir rank" da oficina de tacos). Classe do 839/983, sem par no
// 645; ver include/jp/clubsetrankupwnd.h. Sessão B81 (F3.7.9, 6º lote). Também aqui, por serem a 1ª unidade a usar: métodos do
// ClubSetWorkshopHelper 004c9b20, 004ca410, 004ca4d0, 004ca510, 004ca590. O compilador gera: 004c6900 (ajuste do Helper),
// 004c7500 (`delete`), 004c7430 (vector<FrStatic*>::push_back) e os moldes do vector/map. A LoadLayout983 (004c86e0) não foi escrita.
#include "minatl.h"
#include "projectg.h"
#include "jp/clubsetstat.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "fredit.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "titlefontex.h"
#include "packet.h"
#include "shareddoc.h"
#include "wresrcmng.h"
#include "woverlay.h"
#include "lua983.h"
#include "../../../include/packets/clubsetworkpackets.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopmaindlg.h"
#include "clubsetworkshopprogresswnd.h"
#include "clubsetworkshopfx.h"
#include "clubsetworkshopselector.h"
#include "clubsetinfownd.h"
#include "clubsetrankupwnd.h"

extern Fresh* g_pFresh; // 00f02340
class WView;
extern WView* g_pMainView983; // 00f02458
extern const _WRECT g_rectUnit983_e4296c; // 00e4296c

// nome do atributo escolhido pela carta (unidade da FrClubSetWorkshopSelector; nome meu)
const char* __fastcall GetAddSlotStatText983(int index); // 004a52b0

FrClubSetRankUpWnd* FrClubSetRankUpWnd::s_pInstance = NULL;

// 004c6280
void FrClubSetRankUpWnd::Proc983(float deltaTime)
{
	if (m_bShow && m_pInfoWnd)
		m_pInfoWnd->Proc983(deltaTime);
}

// 004c62b0
void FrClubSetRankUpWnd::ResetInfo983(unsigned long guid, int mode)
{
	m_exp = ClubSetWorkshopFx983::GetSlotExp983(1);
	if (m_pInfoWnd)
	{
		m_pInfoWnd->SetMode983(0);
		m_pInfoWnd->Reset983(0);
	}
}

// 004c62f0
bool FrClubSetRankUpWnd::IsStatMax983(int index)
{
	int stat;
	switch (index)
	{
	case 0: stat = 2; break;
	case 1: stat = 4; break;
	case 2: stat = 0; break;
	case 3: stat = 3; break;
	case 4: stat = 1; break;
	default: return false;
	}
	int cur = ClubSetWorkshopFx983::GetSlotStat983(1, stat);
	return cur >= ClubSetWorkshopFx983::GetSlotStatMax983(1, stat, true);
}

// 004c6470: o trabalho começou
void FrClubSetRankUpWnd::OnRankUpStart983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
	{
		FrClubSetWorkshopProgressWnd::s_pInstance->SetWorking983(true, false);
		FrClubSetWorkshopProgressWnd::s_pInstance->m_bfa983 = true;
	}
	if (m_pInfoWnd)
		m_pInfoWnd->SetMode983(1);
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004c64c0
void FrClubSetRankUpWnd::OnRankUpResult983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		unsigned long rank = pPacket->Decode4();
		unsigned long guid = pPacket->Decode4();
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->StartRankUpResult983(m_clubRank, rank);
		m_clubGuid = guid;
		m_clubRank = rank;
		if (ClubSetWorkshopFx983::SetSlotClub983(1, guid) && m_pInfoWnd)
		{
			m_pInfoWnd->SetMode983(1);
			m_pInfoWnd->Reset983(0);
		}
		ClubSetWorkshopFx983::ResetSlot983(0);
		return;
	}
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->SetWorking983(false, false);
}

// 004c6810
FrClubSetRankUpWnd::FrClubSetRankUpWnd(ClubSetWorkShopForm* pForm)
{
	s_pInstance = this;
	m_cardIconX = 0;
	m_pForm = pForm;
	m_cardIconY = 0;
	m_pInfoWnd = NULL;
	m_itemBgName = NULL;
	m_cardIconOverlay = 0;
	m_clubGuid = 0;
	m_clubRank = 0;
	m_useTypeId = 0;
	m_exp = 0;
	m_cardCount = 0;
	m_cardMax = 0;
	m_needExp = 0;
	m_bSending = false;
	m_rankImgName[0] = NULL;
	m_rankImgName[1] = NULL;
	m_rankImgName[2] = NULL;
	m_rankImgName[3] = NULL;
	m_rankImgName[4] = NULL;
	m_rankImgName[5] = NULL;
}

// 004c6910
FrClubSetRankUpWnd::~FrClubSetRankUpWnd()
{
	s_pInstance = NULL;
	if (m_pInfoWnd)
	{
		delete m_pInfoWnd;
		m_pInfoWnd = NULL;
	}
}

// 004c69d0
bool FrClubSetRankUpWnd::Init983()
{
	CreateCardNumFont983();
	AddCtrlByName983("area_npc_img2");
	AddCtrlByName983("area_logo_rankup");
	AddCtrlByName983("area_clubinfo_rankup");
	AddCtrlByName983("area_item_rankup");
	AddCtrlByName983("btn_confirm_rankup");
	AddCtrlByName983("static_clubname_rankup");
	AddCtrlByName983("static_power_rankup");
	AddCtrlByName983("static_control_rankup");
	AddCtrlByName983("static_accuracy_rankup");
	AddCtrlByName983("static_spin_rankup");
	AddCtrlByName983("static_curve_rankup");
	AddCtrlByName983("static_power_max_rankup");
	AddCtrlByName983("static_control_max_rankup");
	AddCtrlByName983("static_accuracy_max_rankup");
	AddCtrlByName983("static_spin_max_rankup");
	AddCtrlByName983("static_curve_max_rankup");
	AddCtrlByName983("static_clubtype_rankup");
	AddCtrlByName983("static_exppoint_rankup");
	AddCtrlByName983("static_slotcount_rankup");
	AddCtrlByName983("btn_clubselect_rankup");
	AddCtrlByName983("edit_need_exp");
	AddCtrlByName983("area_card_icon_rankup");
	AddCtrlByName983("btn_card_add_rankup");
	AddCtrlByName983("btn_card_sub_rankup");
	AddCtrlByName983("static_addslot_stat");
	AddCtrlByName983("area_card_num_rankup");
	AddCtrlByName983("area_cardinfo_tooltip_rankup");
	AddCtrlByName983("area_npc_speech_rankup");
	return true;
}

// 004c6dd0
void FrClubSetRankUpWnd::OnBeginWork983()
{
	ShowCtrl983("btn_confirm_rankup", false);
	EnableCtrl983("btn_confirm_rankup", false);
	ShowCtrl983("edit_need_exp", false);
	ShowCtrl983("area_card_icon_rankup", false);
	ShowCtrl983("area_cardinfo_tooltip_rankup", false);
	ClubSetWorkshopChildWndBase::OnBeginWork983();
}

// 004c7000: "Deseja subir o rank do taco?" (texto coreano pelo K2L) com as figuras do rank atual → seta → rank seguinte
void FrClubSetRankUpWnd::OnConfirmBtnUp983()
{
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(1);
	if (!pSlot)
		return;
	EnableCtrl983("btn_confirm_rankup", false);
	ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
	if (pDlg)
	{
		pDlg->SetSize983(320.0f, 240.0f, true);
		pDlg->SetMessage(K2L_Compatibility("\305\254\267\264 \267\251\305\251\276\367\300\273 \301\370\307\340\307\317\275\303\260\332"
			"\275\300\264\317\261\356?"), false);
		pDlg->AddBitmap983(pSlot->pBmp);
		if (pSlot->rank != -1 && pSlot->rank != 5)
		{
			pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap(m_rankImgName[pSlot->rank]));
			pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap("clubmade_arrow"));
			pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap(m_rankImgName[pSlot->rank + 1]));
		}
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnRankUpConfirmResult983, 3);
		return;
	}
	EnableCtrl983("btn_confirm_rankup", true);
}

// 004c7250
bool FrClubSetRankUpWnd::OnConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
	{
		if (!m_bSending)
		{
			m_bSending = true;
			SendClubSetRankUp983(m_clubGuid, m_useTypeId, m_cardCount);
			if (FrClubSetWorkshopProgressWnd::s_pInstance)
				FrClubSetWorkshopProgressWnd::s_pInstance->OnSent983();
			OnBeginWork983();
			return true;
		}
	}
	else
		EnableCtrl983("btn_confirm_rankup", true);
	return true;
}

// 004c7360
void FrClubSetRankUpWnd::OnRankUpPacketA983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		unsigned long value = pPacket->Decode4();
		unsigned long guid = pPacket->Decode4();
		m_clubGuid = guid;
		if (m_pInfoWnd)
		{
			m_pInfoWnd->SetMode983(1);
			m_pInfoWnd->SetSelStat983(value);
		}
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
		{
			FrClubSetWorkshopProgressWnd::s_pInstance->m_bfa983 = false;
			FrClubSetWorkshopProgressWnd::s_pInstance->ShowRankUpBalloon983(false);
			FrClubSetWorkshopProgressWnd::s_pInstance->SetWorking983(true, true);
		}
		std::map<unsigned int, sItemInfo>::iterator it = Doc()->m_clubSetMap.find(guid);
		if (it != Doc()->m_clubSetMap.end())
			Doc()->m_itemManager.FindClubSet((*it).second.typeId);
		return;
	}
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->SetWorking983(false, false);
}

// 004c7490
FrClubSetRankUpWnd* FrClubSetWorkshopMainDlg::CreateRankUp983()
{
	return new FrClubSetRankUpWnd(this);
}

// 004c7530
void FrClubSetRankUpWnd::OnClubselectBtnUp983()
{
	FrClubSetWorkshopSelector* pDlg = CreateForm<FrClubSetWorkshopSelector>(g_pFresh->GetManager(), m_pForm, "selectclubset", NULL);
	if (pDlg)
	{
		unsigned long guid = m_clubGuid;
		pDlg->m_marked.push_back(guid);
		pDlg->m_kind = 2;
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnRankUpClubSelected983, 1);
	}
}

// 004c75c0: experiência pedida (e a diferença para a do taco, em vermelho se falta)
void FrClubSetRankUpWnd::UpdateNeedExp983()
{
	int rank = ClubSetWorkshopFx983::GetSlotRank983(1);
	CItemManagerRankUpJp* pMgr = (CItemManagerRankUpJp*)&Doc()->m_itemManager;
	pMgr->GetRankUpExp983(ClubSetWorkshopFx983::GetSlotTypeId983(1), rank + 1, &m_needExp);
	FrEdit* pEdit = GetCtrl983<FrEdit>("edit_need_exp");
	if (pEdit)
	{
		int diff = ClubSetWorkshopFx983::GetSlotExp983(1) - m_needExp;
		pEdit->ClearLine();
		if (rank != 5 && rank != -1)
		{
			pEdit->AddLine(MakeStr("%d", m_needExp), 0, false);
			if (diff < 0)
				pEdit->AddLine(MakeStr("(%d)", diff), 0xffff0000, false);
			else
				pEdit->AddLine(MakeStr("(+%d)", diff), 0xff0000ff, false);
		}
		else
		{
			pEdit->AddLine("-", 0, false);
			pEdit->AddLine("-", 0xff000000, false);
		}
	}
}

// 004c7760: até 4 cartas e no máximo as que o jogador tem
void FrClubSetRankUpWnd::UpdateCards983()
{
	bool bSub = m_cardCount != 0;
	bool bAdd = m_cardCount > -1;
	if (bAdd)
	{
		if (m_cardCount >= m_cardMax)
		{
			m_cardCount = m_cardMax;
			bAdd = false;
		}
		else if (m_cardCount >= 4)
		{
			m_cardCount = 4;
			bAdd = false;
		}
	}
	ShowCtrl983("area_card_icon_rankup", m_cardCount != 0);
	SetOverlayUse983(m_cardIconOverlay, m_cardCount != 0);
	FrButton* pAdd = GetCtrl983<FrButton>("btn_card_add_rankup");
	if (pAdd)
		pAdd->Enable(bAdd);
	FrButton* pSub = GetCtrl983<FrButton>("btn_card_sub_rankup");
	if (pSub)
		pSub->Enable(bSub);
	FrStatic* pStat = GetCtrl983<FrStatic>("static_addslot_stat");
	if (pStat)
		pStat->SetCaption(GetAddSlotStatText983(m_cardCount));
	SetCtrlToolTip983("area_cardinfo_tooltip_rankup", MakeStr(K2L_Compatibility("[%s] \273\363\275\302"),
		GetAddSlotStatText983(m_cardCount)));
}

// 004c7a60
void FrClubSetRankUpWnd::UpdateConfirm983()
{
	UpdateNeedExp983();
	int state;
	if (m_clubGuid == 0)
		state = 0;
	else if (ClubSetWorkshopFx983::GetSlotRank983(1) == 5)
		state = 4;
	else if (ClubSetWorkshopFx983::GetSlotLevel983(1) != 4)
		state = 3;
	else if (m_exp < m_needExp)
		state = 2;
	else
		state = IsStatMax983(m_cardCount) ? 1 : -1;
	bool bEnable = state == -1;
	FrButton* pBtn = GetCtrl983<FrButton>("btn_confirm_rankup");
	if (!pBtn)
		return;
	std::string tip;
	switch (state)
	{
	case 0: // "Escolha o conjunto de tacos."
		tip = K2L_Compatibility("\305\254\267\264\274\274\306\256\270\246 \274\261\305\303\307\330 \301\326\274\274\277\344.");
		break;
	case 1: // "A casa desse atributo está no máximo; não dá para subir o rank."
		tip = K2L_Compatibility("\307\330\264\347 \275\275\267\324\300\314 \303\326\264\353\304\241\277\241 \265\265\264\336\307\317"
			"\277\251, \267\251\305\251\276\367\300\273 \307\322 \274\366 \276\370\275\300\264\317\264\331.");
		break;
	case 2: // "Falta experiência para subir o rank."
		tip = K2L_Compatibility("\267\251\305\251\276\367\277\241 \307\312\277\344\307\321 \274\367\267\303\265\265\260\241 \272\316"
			"\301\267\307\325\264\317\264\331.");
		break;
	case 3: // "O nível do conjunto de tacos não basta."
		tip = K2L_Compatibility("\305\254\267\264\274\274\306\256\300\307 \267\271\272\247\300\314 \272\316\301\267\307\325\264\317"
			"\264\331.");
		break;
	case 4: // "Não dá para subir mais o rank."
		tip = K2L_Compatibility("\264\365 \300\314\273\363 \267\251\305\251\276\367\300\273 \307\322 \274\366 \276\370\275\300\264\317"
			"\264\331.");
		break;
	}
	pBtn->SetToolTipText(bEnable ? "" : tip.c_str());
	pBtn->Enable(bEnable);
}

// 004c7ce0
void FrClubSetRankUpWnd::OnEndWork983()
{
	ShowCtrl983("btn_confirm_rankup", true);
	UpdateConfirm983();
	ShowCtrl983("edit_need_exp", true);
	ShowCtrl983("area_cardinfo_tooltip_rankup", true);
	ClubSetWorkshopChildWndBase::OnEndWork983();
}

// 004c7e60
void FrClubSetRankUpWnd::OnDrawNpcImg983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_npc_img2");
	if (pArea)
		DrawOverlay983(m_npcImgName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c7f70
void FrClubSetRankUpWnd::OnDrawLogo983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_logo_rankup");
	if (pArea)
		DrawOverlay983(m_logoName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c8080
void FrClubSetRankUpWnd::OnDrawClubinfo983()
{
	FrArea* pArea = GetCtrl983<FrArea>("area_clubinfo_rankup");
	if (pArea && m_pInfoWnd)
		m_pInfoWnd->Draw983();
}

// 004c8140
void FrClubSetRankUpWnd::OnDrawItem983()
{
	FrArea* pArea = GetCtrl983<FrArea>("area_item_rankup");
	if (pArea)
		DrawOverlay983(m_itemBgName, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
	DrawOverlayByIndex983(m_cardIconOverlay, (int)(m_pForm->GetRect().x + m_cardIconX), (int)(m_pForm->GetRect().y + m_cardIconY), 0);
}

// 004c8270
void FrClubSetRankUpWnd::OnCardAddBtnUp983()
{
	m_cardCount++;
	UpdateCards983();
	UpdateConfirm983();
}

// 004c8290
void FrClubSetRankUpWnd::OnCardSubBtnUp983()
{
	m_cardCount--;
	UpdateCards983();
	UpdateConfirm983();
}

// 004c82b0
void FrClubSetRankUpWnd::OnDrawCardNum983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_card_num_rankup");
	if (pArea)
	{
		CClubSetWorkShopCardNumFont* pFont = m_pCardNumFont;
		if (pFont)
			pFont->Print(pArea->GetRect().x, pArea->GetRect().y, MakeStr("%d", m_cardCount));
	}
}

// 004c83a0
void FrClubSetRankUpWnd::OnDrawNpcSpeech983()
{
	if (!m_bShow)
		return;
	FrArea* pArea = GetCtrl983<FrArea>("area_npc_speech_rankup");
	if (pArea)
		DrawOverlayByIndex983(m_npcSpeechOverlay, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004c8480
void FrClubSetRankUpWnd::SelectClub983(unsigned long guid)
{
	m_cardCount = 0;
	m_cardMax = 0;
	m_useTypeId = GetTypeId983(2);
	sCards* pCard = FindCard983(2);
	if (pCard)
		m_cardMax = pCard->cardCount;
	m_bSending = false;
	ClubSetWorkshopFx983::SetSlotClub983(1, guid);
	ResetInfo983(guid, 1);
	UpdateCards983();
	UpdateConfirm983();
}

// 004c8500
bool FrClubSetRankUpWnd::OnClubSelected983(int result, FrForm* pForm)
{
	if (result == FrOK && pForm)
	{
		FrClubSetWorkshopSelector* pSel = (FrClubSetWorkshopSelector*)pForm;
		m_clubGuid = pSel->m_selGuid;
		m_clubRank = pSel->m_selTid;
		SelectClub983(m_clubGuid);
	}
	return true;
}

// 004c8540
void FrClubSetRankUpWnd::OnRankUpPacketB983(WReceivedPacket* pPacket)
{
	if (pPacket)
	{
		unsigned long value = pPacket->Decode4();
		unsigned long guid = pPacket->Decode4();
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
		{
			FrClubSetWorkshopProgressWnd::s_pInstance->SetWorking983(true, false);
			FrClubSetWorkshopProgressWnd::s_pInstance->m_bfa983 = false;
			FrClubSetWorkshopProgressWnd::s_pInstance->ShowRankUpBalloon983(false);
		}
		if (m_pInfoWnd)
		{
			m_pInfoWnd->SetMode983(1);
			m_pInfoWnd->SetSelStat983(value);
		}
		std::map<unsigned int, sItemInfo>::iterator it = Doc()->m_clubSetMap.find(guid);
		if (it != Doc()->m_clubSetMap.end())
			Doc()->m_itemManager.FindClubSet((*it).second.typeId);
	}
	else
	{
		OnEndWork983();
		m_bSending = false;
		SelectClub983(m_clubGuid);
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->Finish983();
		if (FrClubSetWorkshopProgressWnd::s_pInstance)
			FrClubSetWorkshopProgressWnd::s_pInstance->Finish983();
	}
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004c8640
void FrClubSetRankUpWnd::OnRankUpCancel983()
{
	m_bSending = false;
	OnEndWork983();
	SelectClub983(m_clubGuid);
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
	if (FrClubSetWorkshopMainDlg::s_pInstance)
		FrClubSetWorkshopMainDlg::s_pInstance->SetRefresh983();
}

// 004c8690
void FrClubSetRankUpWnd::SetShow983(bool bShow)
{
	ClubSetWorkshopChildWndBase::SetShow983(bShow);
	if (bShow)
	{
		if (m_pInfoWnd)
		{
			m_pInfoWnd->SetShow983(bShow);
			m_pInfoWnd->Reset983(0);
		}
		SelectClub983(m_clubGuid);
	}
}

// 004c86e0: tipos dos itens/cartas da oficina (clubset_workshop_tid_table), figuras e posições da página
// (clubset_workshop_ui_rankup; o npc, o logo, o balão e as figuras dos ranks pela clubset_workshop_ui_main), quadro de dados do taco.
// Mesma forma da FrClubSetLevelUpWnd::LoadLayout983 (004ec7b0), sem o ícone do item.
void FrClubSetRankUpWnd::LoadLayout983()
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
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_rankup");

	m_npcImgName = LUA983_GET(tbl, const char*, "npc_img_filename");
	m_logoName = LUA983_GET(tbl, const char*, "logo_img_filename");
	m_itemBgName = LUA983_GET(tbl, const char*, "item_bg_filename");
	LoadOverlay983(m_npcImgName, true);
	LoadOverlay983(m_logoName, true);
	LoadOverlay983(m_itemBgName, true);
	m_cardIconOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "card_select_bg_filename"), true);
	m_cardIconX = (float)((LuaHelper983&)tbl).GetChild983("card_select_bg_pos").GetInl983<int>(1);
	m_cardIconY = (float)((LuaHelper983&)tbl).GetChild983("card_select_bg_pos").GetInl983<int>(2);

	OffsetCtrlLua983("area_npc_img2", &mainTbl, "npc_img_x", "npc_img_y");
	OffsetCtrlLua983("area_logo_rankup", &mainTbl, "logo_img_x", "logo_img_y");
	OffsetCtrlLua983("area_item_rankup", &tbl, "item_wnd_x", "item_wnd_y");
	OffsetCtrlLua983("btn_confirm_rankup", &tbl, "confirm_btn_pos");
	OffsetCtrlLua983("edit_need_exp", &tbl, "text_need_exp_pos");
	OffsetCtrlLua983("area_card_icon_rankup", &tbl, "card_icon_pos");
	OffsetCtrlLua983("btn_card_add_rankup", &tbl, "btn_card_add_pos");
	OffsetCtrlLua983("btn_card_sub_rankup", &tbl, "btn_card_sub_pos");
	OffsetCtrlLua983("static_addslot_stat", &tbl, "text_slotadd_stat_pos");
	OffsetCtrlLua983("area_cardinfo_tooltip_rankup", &tbl, "card_info_tooltip_pos");
	OffsetCtrlLua983("area_card_num_rankup", &tbl, "img_font_pos");
	OffsetCtrlLua983("area_npc_speech_rankup", &mainTbl, "img_npc_speech_pos");

	std::vector<FrStatic*> statics;
	statics.push_back(GetCtrl983<FrStatic>("static_clubname_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_power_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_control_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_accuracy_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_spin_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_curve_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_clubtype_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_exppoint_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_slotcount_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_power_max_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_control_max_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_accuracy_max_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_spin_max_rankup"));
	statics.push_back(GetCtrl983<FrStatic>("static_curve_max_rankup"));
	FrArea* pInfoArea = GetCtrl983<FrArea>("area_clubinfo_rankup");
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
			FrButton* pBtn = GetCtrl983<FrButton>("btn_clubselect_rankup");
			if (pBtn)
				m_pInfoWnd->SetSelectBtn983(pBtn);
			m_pInfoWnd->SetPos983((int)pInfoArea->GetRect().x, (int)pInfoArea->GetRect().y);
			m_pInfoWnd->LoadLayout983(0);
			m_pInfoWnd->Reset983(0);
		}
	}

	m_npcSpeechOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "img_npc_speech_filename"), true);
	// "랭크업시 숙련도가 소모됩니다." (subir o rank gasta a experiência)
	SetCtrlToolTip983("edit_need_exp",
		K2L_Compatibility("\267\251\305\251\276\367\275\303 \274\367\267\303\265\265\260\241 \274\322\270\360\265\313\264\317\264\331."));
	IFF_STRUCT::sCard* pCard = ItemManager()->FindCard(GetTypeId983(2));
	SetCtrlToolTip983("area_card_icon_rankup", pCard ? MakeStr("%s", pCard->c.Name) : "");
	m_rankImgName[0] = LUA983_GET(mainTbl, const char*, "rank_icon_e");
	m_rankImgName[1] = LUA983_GET(mainTbl, const char*, "rank_icon_d");
	m_rankImgName[2] = LUA983_GET(mainTbl, const char*, "rank_icon_c");
	m_rankImgName[3] = LUA983_GET(mainTbl, const char*, "rank_icon_b");
	m_rankImgName[4] = LUA983_GET(mainTbl, const char*, "rank_icon_a");
	m_rankImgName[5] = LUA983_GET(mainTbl, const char*, "rank_icon_s");
	SelectClub983(0);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// ---------------------------------------------------------------- ClubSetWorkshopHelper (métodos nesta unidade)

// 004c9b20
bool ClubSetWorkshopHelper::CreateCardNumFont983()
{
	if (m_pCardNumFont)
		return false;
	m_pCardNumFont = new CClubSetWorkShopCardNumFont;
	return true;
}

// 004ca410
void ClubSetWorkshopHelper::DrawOverlayByIndex983(unsigned int index, int x, int y, int w)
{
	int left = x;
	if (m_overlays.size() > index && m_overlays[index].bUse == true)
	{
		sOverlay983& o = m_overlays[index];
		WOverlay* p = o.pOverlay;
		if (p)
		{
			int width = w;
			if (!width)
				width = p->GetWidth();
			_WRECT dst = { (float)left, (float)y, (float)width, (float)p->GetHeight() };
			p->Render(g_pMainView983, g_rectUnit983_e4296c, dst, 0, (o.alpha << 24) | 0xffffff, 0.0f, 0);
		}
	}
}

// 004ca4d0
void ClubSetWorkshopHelper::SetOverlayUse983(unsigned int index, bool bUse)
{
	if (m_overlays.size() > index)
		m_overlays[index].bUse = bUse;
}

// 004ca510: transparência 0…1 → 0…255
void ClubSetWorkshopHelper::SetOverlayAlpha983(unsigned int index, float alpha)
{
	if (m_overlays.size() > index)
	{
		if (alpha < 0.0f)
			alpha = 0.0f;
		else if (alpha > 1.0f)
			alpha = 1.0f;
		m_overlays[index].alpha = (unsigned char)(int)(alpha * 255.0);
	}
}

// 004ca590: moldura de três figuras (esquerda, meio, direita)
void ClubSetWorkshopHelper::DrawFrame983(unsigned int index, int x, int y)
{
	if (index < m_frames.size())
	{
		float pos[2] = { (float)(m_frames[index].offsetX + x), (float)(m_frames[index].offsetY + y) };
		for (int i = 0; i < 3; i++)
		{
			DrawOverlayByIndex983((&m_frames[index].left)[i], (int)pos[0], (int)pos[1], (&m_frames[index].leftW)[i]);
			pos[0] += (float)(&m_frames[index].leftW)[i];
		}
	}
}
