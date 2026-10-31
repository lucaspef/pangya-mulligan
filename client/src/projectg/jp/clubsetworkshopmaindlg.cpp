// JP 983: unidade 004cccd0–004ce0d4 (FrClubSetWorkshopMainDlg, diálogo principal da oficina de tacos). Classe do 839/983, sem
// par no 645; ver include/jp/clubsetworkshopmaindlg.h. Sessão B78 (F3.7.9, 3º lote). O compilador gera (e casam): 004ccdc0
// (`delete` escalar do ClubSetWorkshopFx983, sem chamador), os ajustes 004cce10/004cce20, o 004cce30 (chamada virtual do
// OnCtrlCreated983 pelo mapa), 004cd640 (CreateForm<FrClubSetWorkshopMainDlg>) e 004cd720 (`delete` da MainDlg).
#include "minatl.h"
#include "projectg.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include "lua983.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetlevelupwnd.h"
#include "clubsetrankupwnd.h"
#include "clubsetoptionalfunctionswnd.h"
#include "clubsetworkshopprogresswnd.h"
#include "clubsetworkshopfx.h"
#include "clubsetworkshopaboutdlg.h"
#include "clubsetworkshopmaindlg.h"

extern Fresh* g_pFresh; // 00f02340

// textos do aviso de cada aba, sorteados por `rand() % 100` (unidade da FrClubSetWorkshopSelector; nomes meus)
const char* __fastcall GetLevelUpNotice983(int r); // 004a5370
const char* __fastcall GetRankUpNotice983(int r); // 004a53c0
const char* __fastcall GetCareNotice983(int r); // 004a5410

FrClubSetWorkshopMainDlg* FrClubSetWorkshopMainDlg::s_pInstance = NULL;

// 004cccd0
FrWnd* FrClubSetWorkshopMainDlg::OnCtrlCreated983(IObject* pObj)
{
	return AddCtrlByType983(pObj);
}

// handler das derivadas da ClubSetWorkShopForm (004cce30 = chamada virtual do +0x98), gravado como VI
#define ON_FRESH_CTRL983(name) \
	{ (FRESH_PFN)(void (FrCmdTarget::*)(IObject*))(void (ClubSetWorkShopForm::*)(IObject*)) \
		&ClubSetWorkShopForm::OnCtrlCreated983, name, FRCMD_INIT, FrFuncVI },

// 004ccce0 (GetMessageMap); mapa 00e05640 (179 entradas)
BEGIN_FRESH_MSGMAP(FrClubSetWorkshopMainDlg, ClubSetWorkShopForm)
ON_FRESH_CTRL983("area_bg1")
ON_FRESH_CTRL983("area_bg2")
ON_FRESH_CTRL983("area_bg3")
ON_FRESH_CTRL983("area_bg4")
ON_FRESH_CTRL983("area_bg5")
ON_FRESH_CTRL983("area_bg6")
ON_FRESH_CTRL983("btn_tab_levelup")
ON_FRESH_CTRL983("btn_tab_rankup")
ON_FRESH_CTRL983("btn_tab_care")
ON_FRESH_CTRL983("btn_help")
ON_FRESH_CTRL983("btn_exit")
ON_FRESH_CTRL983("static_notice_clubworkhop")
ON_FRESH_VV("btn_tab_levelup", FRCMD_LBUTTONDOWN, FrClubSetWorkshopMainDlg::OnTabLevelUpDown983)
ON_FRESH_VV("btn_tab_rankup", FRCMD_LBUTTONDOWN, FrClubSetWorkshopMainDlg::OnTabRankUpDown983)
ON_FRESH_VV("btn_tab_care", FRCMD_LBUTTONDOWN, FrClubSetWorkshopMainDlg::OnTabCareDown983)
ON_FRESH_VV("btn_help", FRCMD_LBUTTONDOWN, FrClubSetWorkshopMainDlg::OnHelpBtnDown983)
ON_FRESH_VV("btn_exit", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnExitBtnUp983)
ON_FRESH_CTRL983("area_npc_img1")
ON_FRESH_CTRL983("area_logo_levelup")
ON_FRESH_CTRL983("area_clubinfo_levelup")
ON_FRESH_CTRL983("area_item_levelup")
ON_FRESH_CTRL983("btn_confirm_levelup")
ON_FRESH_CTRL983("static_clubname_levelup")
ON_FRESH_CTRL983("static_power_levelup")
ON_FRESH_CTRL983("static_control_levelup")
ON_FRESH_CTRL983("static_accuracy_levelup")
ON_FRESH_CTRL983("static_spin_levelup")
ON_FRESH_CTRL983("static_curve_levelup")
ON_FRESH_CTRL983("static_power_max_levelup")
ON_FRESH_CTRL983("static_control_max_levelup")
ON_FRESH_CTRL983("static_accuracy_max_levelup")
ON_FRESH_CTRL983("static_spin_max_levelup")
ON_FRESH_CTRL983("static_curve_max_levelup")
ON_FRESH_CTRL983("static_clubtype_levelup")
ON_FRESH_CTRL983("static_exppoint_levelup")
ON_FRESH_CTRL983("static_slotcount_levelup")
ON_FRESH_CTRL983("btn_clubselect_levelup")
ON_FRESH_CTRL983("area_item_icon_levelup")
ON_FRESH_CTRL983("area_item_num_levelup")
ON_FRESH_CTRL983("static_item_name_levelup")
ON_FRESH_CTRL983("area_card_icon_levelup")
ON_FRESH_CTRL983("btn_card_add_levelup")
ON_FRESH_CTRL983("btn_card_sub_levelup")
ON_FRESH_CTRL983("static_upgrade_stat")
ON_FRESH_CTRL983("area_card_num_levelup")
ON_FRESH_CTRL983("area_cardinfo_tooltip_levelup")
ON_FRESH_CTRL983("area_npc_speech_levelup")
ON_FRESH_VV("area_npc_img1", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawNpcImg983)
ON_FRESH_VV("area_logo_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawLogo983)
ON_FRESH_VV("area_clubinfo_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawClubinfo983)
ON_FRESH_VV("area_item_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawItem983)
ON_FRESH_VV("btn_confirm_levelup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnLevelUpConfirmBtnUp983)
ON_FRESH_VV("btn_clubselect_levelup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnLevelUpClubselectBtnUp983)
ON_FRESH_VV("area_item_num_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawItemNum983)
ON_FRESH_VV("btn_card_add_levelup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnLevelUpCardAddBtnUp983)
ON_FRESH_VV("btn_card_sub_levelup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnLevelUpCardSubBtnUp983)
ON_FRESH_VV("area_card_num_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawCardNum983)
ON_FRESH_VV("area_npc_speech_levelup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnLevelUpDrawNpcSpeech983)
ON_FRESH_CTRL983("area_npc_img2")
ON_FRESH_CTRL983("area_logo_rankup")
ON_FRESH_CTRL983("area_clubinfo_rankup")
ON_FRESH_CTRL983("area_item_rankup")
ON_FRESH_CTRL983("btn_confirm_rankup")
ON_FRESH_CTRL983("static_clubname_rankup")
ON_FRESH_CTRL983("static_power_rankup")
ON_FRESH_CTRL983("static_control_rankup")
ON_FRESH_CTRL983("static_accuracy_rankup")
ON_FRESH_CTRL983("static_spin_rankup")
ON_FRESH_CTRL983("static_curve_rankup")
ON_FRESH_CTRL983("static_power_max_rankup")
ON_FRESH_CTRL983("static_control_max_rankup")
ON_FRESH_CTRL983("static_accuracy_max_rankup")
ON_FRESH_CTRL983("static_spin_max_rankup")
ON_FRESH_CTRL983("static_curve_max_rankup")
ON_FRESH_CTRL983("static_clubtype_rankup")
ON_FRESH_CTRL983("static_exppoint_rankup")
ON_FRESH_CTRL983("static_slotcount_rankup")
ON_FRESH_CTRL983("btn_clubselect_rankup")
ON_FRESH_CTRL983("edit_need_exp")
ON_FRESH_CTRL983("area_card_icon_rankup")
ON_FRESH_CTRL983("btn_card_add_rankup")
ON_FRESH_CTRL983("btn_card_sub_rankup")
ON_FRESH_CTRL983("static_addslot_stat")
ON_FRESH_CTRL983("area_card_num_rankup")
ON_FRESH_CTRL983("area_cardinfo_tooltip_rankup")
ON_FRESH_CTRL983("area_npc_speech_rankup")
ON_FRESH_VV("area_npc_img2", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawNpcImg983)
ON_FRESH_VV("area_logo_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawLogo983)
ON_FRESH_VV("area_clubinfo_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawClubinfo983)
ON_FRESH_VV("area_item_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawItem983)
ON_FRESH_VV("btn_confirm_rankup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnRankUpConfirmBtnUp983)
ON_FRESH_VV("btn_clubselect_rankup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnRankUpClubselectBtnUp983)
ON_FRESH_VV("btn_card_add_rankup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnRankUpCardAddBtnUp983)
ON_FRESH_VV("btn_card_sub_rankup", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnRankUpCardSubBtnUp983)
ON_FRESH_VV("area_card_num_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawCardNum983)
ON_FRESH_VV("area_npc_speech_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnRankUpDrawNpcSpeech983)
ON_FRESH_CTRL983("area_npc_img3")
ON_FRESH_CTRL983("area_logo_of")
ON_FRESH_CTRL983("area_expmove_bgfrm_l")
ON_FRESH_CTRL983("area_expmove_bgfrm_m")
ON_FRESH_CTRL983("area_expmove_bgfrm_r")
ON_FRESH_CTRL983("area_recovery_bgfrm_l")
ON_FRESH_CTRL983("area_recovery_bgfrm_m")
ON_FRESH_CTRL983("area_recovery_bgfrm_r")
ON_FRESH_CTRL983("area_title_expmove")
ON_FRESH_CTRL983("area_expmove_icon")
ON_FRESH_CTRL983("area_clubset_exp_bg")
ON_FRESH_CTRL983("area_expmove_bg_bot")
ON_FRESH_CTRL983("area_clubselect_expmove0")
ON_FRESH_CTRL983("area_clubselect_expmove1")
ON_FRESH_CTRL983("btn_confirm_expmove")
ON_FRESH_CTRL983("btn_clubselect_expmove0")
ON_FRESH_CTRL983("btn_clubselect_expmove1")
ON_FRESH_CTRL983("btn_exp_add")
ON_FRESH_CTRL983("btn_exp_sub")
ON_FRESH_CTRL983("static_exppoint0")
ON_FRESH_CTRL983("static_exppoint1")
ON_FRESH_CTRL983("static_exppoint_move")
ON_FRESH_CTRL983("static_itemname_expmove")
ON_FRESH_CTRL983("static_itemcount_expmove")
ON_FRESH_CTRL983("area_expmove_item_icon")
ON_FRESH_CTRL983("area_title_recovery")
ON_FRESH_CTRL983("area_clubselect_recovery")
ON_FRESH_CTRL983("area_clubset_slotadd_bg")
ON_FRESH_CTRL983("area_recovery_bg_bot")
ON_FRESH_CTRL983("btn_confirm_recovery")
ON_FRESH_CTRL983("btn_clubselect_recovery")
ON_FRESH_CTRL983("static_slotadd_recovery")
ON_FRESH_CTRL983("area_recovery_item_icon")
ON_FRESH_CTRL983("area_item_num_recovery")
ON_FRESH_CTRL983("area_addslot_cnt_imgfont")
ON_FRESH_CTRL983("area_npc_speech_op")
ON_FRESH_CTRL983("area_fx_expmove")
ON_FRESH_CTRL983("area_fx_recovery")
ON_FRESH_VV("area_npc_img3", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawNpcImg983)
ON_FRESH_VV("area_logo_of", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawLogo983)
ON_FRESH_VV("area_expmove_icon", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawExpmoveIcon983)
ON_FRESH_VV("area_clubselect_expmove0", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawClubselectExpmove0_983)
ON_FRESH_VV("area_clubselect_expmove1", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawClubselectExpmove1_983)
ON_FRESH_VV("btn_confirm_expmove", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareConfirmExpmoveBtnUp983)
ON_FRESH_VV("btn_clubselect_expmove0", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareClubselectExpmove0BtnUp983)
ON_FRESH_VV("btn_clubselect_expmove1", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareClubselectExpmove1BtnUp983)
ON_FRESH_VV("btn_exp_add", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareExpAddBtnUp983)
ON_FRESH_VV("btn_exp_sub", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareExpSubBtnUp983)
ON_FRESH_VV("area_expmove_item_icon", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawExpmoveItemIcon983)
ON_FRESH_VV("area_clubselect_recovery", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawClubselectRecovery983)
ON_FRESH_VV("btn_confirm_recovery", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareConfirmRecoveryBtnUp983)
ON_FRESH_VV("btn_clubselect_recovery", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnCareClubselectRecoveryBtnUp983)
ON_FRESH_VV("area_recovery_item_icon", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawRecoveryItemIcon983)
ON_FRESH_VV("area_item_num_recovery", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawItemNumRecovery983)
ON_FRESH_VV("area_npc_speech_op", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawNpcSpeech983)
ON_FRESH_VV("area_fx_expmove", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawFxExpmove983)
ON_FRESH_VV("area_fx_recovery", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnCareDrawFxRecovery983)
ON_FRESH_CTRL983("area_progress_eclipse")
ON_FRESH_CTRL983("area_progress_subwnd_bg")
ON_FRESH_CTRL983("area_progress_animation")
ON_FRESH_CTRL983("gauegebar_progress")
ON_FRESH_CTRL983("btn_workshop_apply")
ON_FRESH_CTRL983("btn_workshop_cancel")
ON_FRESH_CTRL983("btn_workshop_ok")
ON_FRESH_CTRL983("area_speakballoon_rankup")
ON_FRESH_CTRL983("static_speakballoon_rankup")
ON_FRESH_CTRL983("area_progress_result_title")
ON_FRESH_CTRL983("area_progress_result_clubset")
ON_FRESH_CTRL983("area_progress_result_level")
ON_FRESH_CTRL983("area_progress_result_rank")
ON_FRESH_CTRL983("edit_progress_result_text")
ON_FRESH_VV("area_progress_eclipse", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressEclipse983)
ON_FRESH_VV("area_progress_subwnd_bg", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressSubwndBg983)
ON_FRESH_VV("area_progress_animation", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressAnimation983)
ON_FRESH_VV("area_progressbar", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressbar983)
ON_FRESH_VV("btn_workshop_apply", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnProgressWorkshopApplyBtnUp983)
ON_FRESH_VV("btn_workshop_cancel", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnProgressWorkshopCancelBtnUp983)
ON_FRESH_VV("btn_workshop_ok", FRCMD_LBUTTONUP, FrClubSetWorkshopMainDlg::OnProgressWorkshopOkBtnUp983)
ON_FRESH_VV("area_speakballoon_rankup", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawSpeakballoonRankup983)
ON_FRESH_VV("area_progress_result_title", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultTitle983)
ON_FRESH_VV("area_progress_result_clubset", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultClubset983)
ON_FRESH_VV("area_progress_result_level", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultLevel983)
ON_FRESH_VV("area_progress_result_rank", FRCMD_OWNERDRAW, FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultRank983)
END_FRESH_MSGMAP()

// 004cccf0
FrClubSetWorkshopMainDlg::FrClubSetWorkshopMainDlg()
{
	s_pInstance = this;
	m_pLevelUp = CreateLevelUp983();
	m_pRankUp = CreateRankUp983();
	m_pCare = CreateCare983();
	m_pProgress = CreateProgress983();
	m_pFx = ClubSetWorkshopFx983::Create983();
	m_tab = 3;
	m_pad983_158 = 0;
	m_bRefresh983 = false;
}

// 004ccdb0 (GetRTTI), 004cce70 (MakeInstance), 004ccee0 (registro)
IMPLEMENT_OBJECT(FrClubSetWorkshopMainDlg, ClubSetWorkShopForm)

// 004ccdf0: "sim" no aviso de saída
bool FrClubSetWorkshopMainDlg::OnExitConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
		FrForm::Close(FrCANCEL, true);
	return true;
}

// 004cce40 / 004cce50 / 004cce60: sem chamador no código (004cce50: 009d5971…; 004cce60 citado em dados); nomes meus
bool IsClubSetWorkshopFxOn983()
{
	return ClubSetWorkshopFx983::s_pInstance != NULL;
}

bool IsClubSetCareWndOn983()
{
	return FrClubSetOptionalFunctionsWnd::s_pInstance != NULL;
}

bool IsClubSetWorkshopAboutOn983()
{
	return FrClubSetWorkshopAboutDlg::s_pInstance != NULL;
}

#define SAFE_DELETE(p) \
	if (p) \
	{ \
		delete p; \
		p = NULL; \
	}

// 004ccf00
FrClubSetWorkshopMainDlg::~FrClubSetWorkshopMainDlg()
{
	s_pInstance = NULL;
	SAFE_DELETE(m_pLevelUp);
	SAFE_DELETE(m_pRankUp);
	SAFE_DELETE(m_pCare);
	SAFE_DELETE(m_pProgress);
	SAFE_DELETE(m_pFx);
}

// 004ccff0
void FrClubSetWorkshopMainDlg::OnProc(const float deltaTime)
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->Proc983(deltaTime);
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->Proc983(deltaTime);
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->Proc983(deltaTime);
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->Proc983(deltaTime);
	if (ClubSetWorkshopFx983::s_pInstance)
		ClubSetWorkshopFx983::s_pInstance->Proc983(deltaTime);
}

// 004cd070
void FrClubSetWorkshopMainDlg::OnHelpBtnDown983()
{
	if (FrClubSetWorkshopAboutDlg::s_pInstance)
		return;
	FrClubSetWorkshopAboutDlg* pDlg = FrClubSetWorkshopAboutDlg::Create983(g_pFresh->GetManager(), this);
	if (pDlg)
		pDlg->Open(NULL, 3);
}

// 004cd0c0
void FrClubSetWorkshopMainDlg::OnLevelUpDrawNpcImg983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawNpcImg983();
}

// 004cd0e0
void FrClubSetWorkshopMainDlg::OnLevelUpDrawLogo983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawLogo983();
}

// 004cd100
void FrClubSetWorkshopMainDlg::OnLevelUpDrawClubinfo983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawClubinfo983();
}

// 004cd110
void FrClubSetWorkshopMainDlg::OnLevelUpDrawItem983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawItem983();
}

// 004cd120
void FrClubSetWorkshopMainDlg::OnLevelUpConfirmBtnUp983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnConfirmBtnUp983();
}

// 004cd130
void FrClubSetWorkshopMainDlg::OnLevelUpClubselectBtnUp983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnClubselectBtnUp983();
}

// 004cd140
void FrClubSetWorkshopMainDlg::OnLevelUpDrawItemNum983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawItemNum983();
}

// 004cd150
void FrClubSetWorkshopMainDlg::OnLevelUpCardAddBtnUp983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnCardAddBtnUp983();
}

// 004cd160
void FrClubSetWorkshopMainDlg::OnLevelUpCardSubBtnUp983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnCardSubBtnUp983();
}

// 004cd170
void FrClubSetWorkshopMainDlg::OnLevelUpDrawCardNum983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawCardNum983();
}

// 004cd180
void FrClubSetWorkshopMainDlg::OnLevelUpDrawNpcSpeech983()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnDrawNpcSpeech983();
}

// 004cd190
void FrClubSetWorkshopMainDlg::OnRankUpDrawNpcImg983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawNpcImg983();
}

// 004cd1b0
void FrClubSetWorkshopMainDlg::OnRankUpDrawLogo983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawLogo983();
}

// 004cd1d0
void FrClubSetWorkshopMainDlg::OnRankUpDrawClubinfo983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawClubinfo983();
}

// 004cd1e0
void FrClubSetWorkshopMainDlg::OnRankUpDrawItem983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawItem983();
}

// 004cd1f0
void FrClubSetWorkshopMainDlg::OnRankUpConfirmBtnUp983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnConfirmBtnUp983();
}

// 004cd200
void FrClubSetWorkshopMainDlg::OnRankUpClubselectBtnUp983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnClubselectBtnUp983();
}

// 004cd210
void FrClubSetWorkshopMainDlg::OnRankUpCardAddBtnUp983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnCardAddBtnUp983();
}

// 004cd220
void FrClubSetWorkshopMainDlg::OnRankUpCardSubBtnUp983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnCardSubBtnUp983();
}

// 004cd230
void FrClubSetWorkshopMainDlg::OnRankUpDrawCardNum983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawCardNum983();
}

// 004cd240
void FrClubSetWorkshopMainDlg::OnRankUpDrawNpcSpeech983()
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnDrawNpcSpeech983();
}

// 004cd250
void FrClubSetWorkshopMainDlg::OnCareDrawNpcImg983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawNpcImg983();
}

// 004cd270
void FrClubSetWorkshopMainDlg::OnCareDrawLogo983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawLogo983();
}

// 004cd290
void FrClubSetWorkshopMainDlg::OnCareDrawExpmoveIcon983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawExpmoveIcon983();
}

// 004cd2a0
void FrClubSetWorkshopMainDlg::OnCareDrawClubselectExpmove0_983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawClubselectExpmove0_983();
}

// 004cd2b0
void FrClubSetWorkshopMainDlg::OnCareDrawClubselectExpmove1_983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawClubselectExpmove1_983();
}

// 004cd2c0
void FrClubSetWorkshopMainDlg::OnCareConfirmExpmoveBtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnConfirmExpmoveBtnUp983();
}

// 004cd2d0
void FrClubSetWorkshopMainDlg::OnCareClubselectExpmove0BtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnClubselectExpmove0BtnUp983();
}

// 004cd2e0
void FrClubSetWorkshopMainDlg::OnCareClubselectExpmove1BtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnClubselectExpmove1BtnUp983();
}

// 004cd2f0
void FrClubSetWorkshopMainDlg::OnCareExpAddBtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpAddBtnUp983();
}

// 004cd300
void FrClubSetWorkshopMainDlg::OnCareExpSubBtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpSubBtnUp983();
}

// 004cd310
void FrClubSetWorkshopMainDlg::OnCareDrawExpmoveItemIcon983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawExpmoveItemIcon983();
}

// 004cd320
void FrClubSetWorkshopMainDlg::OnCareDrawClubselectRecovery983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawClubselectRecovery983();
}

// 004cd330
void FrClubSetWorkshopMainDlg::OnCareConfirmRecoveryBtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnConfirmRecoveryBtnUp983();
}

// 004cd340
void FrClubSetWorkshopMainDlg::OnCareClubselectRecoveryBtnUp983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnClubselectRecoveryBtnUp983();
}

// 004cd350
void FrClubSetWorkshopMainDlg::OnCareDrawRecoveryItemIcon983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawRecoveryItemIcon983();
}

// 004cd360
void FrClubSetWorkshopMainDlg::OnCareDrawItemNumRecovery983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawItemNumRecovery983();
}

// 004cd370
void FrClubSetWorkshopMainDlg::OnCareDrawNpcSpeech983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawNpcSpeech983();
}

// 004cd380
void FrClubSetWorkshopMainDlg::OnCareDrawFxExpmove983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawFxExpmove983();
}

// 004cd390
void FrClubSetWorkshopMainDlg::OnCareDrawFxRecovery983()
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnDrawFxRecovery983();
}

// 004cd3a0
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressEclipse983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressEclipse983();
}

// 004cd3b0
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressSubwndBg983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressSubwndBg983();
}

// 004cd3c0
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressAnimation983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressAnimation983();
}

// 004cd3d0
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressbar983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressbar983();
}

// 004cd3e0
void FrClubSetWorkshopMainDlg::OnProgressWorkshopApplyBtnUp983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnWorkshopApplyBtnUp983();
}

// 004cd3f0
void FrClubSetWorkshopMainDlg::OnProgressWorkshopCancelBtnUp983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnWorkshopCancelBtnUp983();
}

// 004cd400
void FrClubSetWorkshopMainDlg::OnProgressWorkshopOkBtnUp983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnWorkshopOkBtnUp983();
}

// 004cd410
void FrClubSetWorkshopMainDlg::OnProgressDrawSpeakballoonRankup983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawSpeakballoonRankup983();
}

// 004cd420
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultTitle983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressResultTitle983();
}

// 004cd430
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultClubset983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressResultClubset983();
}

// 004cd440
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultLevel983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressResultLevel983();
}

// 004cd450
void FrClubSetWorkshopMainDlg::OnProgressDrawProgressResultRank983()
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnDrawProgressResultRank983();
}

// 004cd460
bool FrClubSetWorkshopMainDlg::OnLevelUpClubSelected983(int result, FrForm* pForm)
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnClubSelected983(result, pForm);
	return true;
}

// 004cd480
bool FrClubSetWorkshopMainDlg::OnRankUpClubSelected983(int result, FrForm* pForm)
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnClubSelected983(result, pForm);
	return true;
}

// 004cd4a0
bool FrClubSetWorkshopMainDlg::OnCareExpMoveSel0Result983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpMoveSel0Result983(result, pForm);
	return true;
}

// 004cd4c0
bool FrClubSetWorkshopMainDlg::OnCareExpMoveSel1Result983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpMoveSel1Result983(result, pForm);
	return true;
}

// 004cd4e0
bool FrClubSetWorkshopMainDlg::OnCareRecoverySelResult983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnRecoverySelResult983(result, pForm);
	return true;
}

// 004cd500
bool FrClubSetWorkshopMainDlg::OnLevelUpConfirmResult983(int result, FrForm* pForm)
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnConfirmResult983(result, pForm);
	return true;
}

// 004cd520
bool FrClubSetWorkshopMainDlg::OnRankUpConfirmResult983(int result, FrForm* pForm)
{
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnConfirmResult983(result, pForm);
	return true;
}

// 004cd540
bool FrClubSetWorkshopMainDlg::OnCareExpMoveConfirmResult983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnExpMoveConfirmResult983(result, pForm);
	return true;
}

// 004cd560
bool FrClubSetWorkshopMainDlg::OnCareRecoveryConfirmResult983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnRecoveryConfirmResult983(result, pForm);
	return true;
}

// 004cd580
bool FrClubSetWorkshopMainDlg::OnCareNotifyResultA983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnNotifyResultA983(result, pForm);
	return true;
}

// 004cd5a0
bool FrClubSetWorkshopMainDlg::OnCareNotifyResultB983(int result, FrForm* pForm)
{
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->OnNotifyResultB983(result, pForm);
	return true;
}

// 004cd5c0
bool FrClubSetWorkshopMainDlg::OnProgressApplyResult983(int result, FrForm* pForm)
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnApplyResult983(result, pForm);
	return true;
}

// 004cd5e0
bool FrClubSetWorkshopMainDlg::OnProgressCancelResult983(int result, FrForm* pForm)
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnCancelResult983(result, pForm);
	return true;
}

// 004cd600
bool FrClubSetWorkshopMainDlg::OnProgressApplyResultB983(int result, FrForm* pForm)
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnApplyResultB983(result, pForm);
	return true;
}

// 004cd620
bool FrClubSetWorkshopMainDlg::OnProgressCancelConfirmResult983(int result, FrForm* pForm)
{
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->OnCancelConfirmResult983(result, pForm);
	return true;
}

// 004cd710 (o CreateForm<FrClubSetWorkshopMainDlg> 004cd640 vem antes)
FrClubSetWorkshopMainDlg* __fastcall FrClubSetWorkshopMainDlg::Create983(FrWndManager* pManager, FrCmdTarget* pCmdDest)
{
	return CreateForm<FrClubSetWorkshopMainDlg>(pManager, pCmdDest, "clubset_workshop", NULL);
}

// 004cd750: "Deseja sair da oficina?" (texto coreano passado pelo K2L)
void FrClubSetWorkshopMainDlg::OnExitBtnUp983()
{
	FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), this, "notify_okcancel", NULL);
	if (pDlg)
	{
		pDlg->SetMessage(K2L_Compatibility("\xc5\xac\xb7\xb4 \xb0\xf8\xb9\xe6\xc0\xbb \xc1\xbe\xb7\xe1\xc7\xcf\xbd\xc3\xb0\xda"
			"\xbd\xc0\xb4\xcf\xb1\xee?"), false);
		pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnExitConfirmResult983, 0x803);
	}
}

// 004cd7d0: sem chamador nem citação no 983 (nome meu)
bool FrClubSetWorkshopMainDlg::OnExitKey983()
{
	OnExitBtnUp983();
	return true;
}

// 004cd7e0
void FrClubSetWorkshopMainDlg::OnCancel()
{
	OnExitBtnUp983();
}

// 004cd7f0
void FrClubSetWorkshopMainDlg::DisableTabs983()
{
	EnableCtrl983(this, "btn_tab_levelup", false);
	EnableCtrl983(this, "btn_tab_rankup", false);
	EnableCtrl983(this, "btn_tab_care", false);
	EnableCtrl983(this, "btn_help", false);
	EnableCtrl983(this, "btn_exit", false);
}

// 004cd8c0
void FrClubSetWorkshopMainDlg::EnableTabs983()
{
	EnableCtrl983(this, "btn_tab_levelup", true);
	EnableCtrl983(this, "btn_tab_rankup", true);
	EnableCtrl983(this, "btn_tab_care", true);
	EnableCtrl983(this, "btn_help", true);
	EnableCtrl983(this, "btn_exit", true);
}

// 004cd9a0: fundo em "area_bg1".."area_bgN" (bg_file_name é um formato com o número), deslocado de bg_x/bg_y
void FrClubSetWorkshopMainDlg::LoadBg983(lua_system::lua_table_helper* pTbl)
{
	int x = LUA983_GET(*pTbl, int, "bg_x");
	int y = LUA983_GET(*pTbl, int, "bg_y");
	const char* fileName = LUA983_GET(*pTbl, const char*, "bg_file_name");
	int count = LUA983_GET(*pTbl, int, "bg_file_count");
	for (int i = 0; i < count; i++)
	{
		FrArea* pArea = GetCtrl983<FrArea>(MakeStr("area_bg%d", i + 1));
		if (pArea)
		{
			const Bitmap* pBmp = g_pFresh->RegisterBitmap(MakeStr(fileName, i + 1));
			if (pBmp)
			{
				pArea->SetBgImg(pBmp);
				WRect rc = pArea->GetRect();
				rc.x = x + rc.x;
				rc.y = y + rc.y;
				pArea->SetRect(rc);
			}
		}
	}
}

// 004cdad0
void FrClubSetWorkshopMainDlg::ShowNotice983(int tab)
{
	FrStatic* pStatic = GetCtrl983<FrStatic>("static_notice_clubworkhop");
	if (!pStatic)
		return;
	int r = rand() % 100;
	switch (tab)
	{
	case 0:
		pStatic->SetCaption(GetLevelUpNotice983(r));
		break;
	case 1:
		pStatic->SetCaption(GetRankUpNotice983(r));
		break;
	case 2:
		pStatic->SetCaption(GetCareNotice983(r));
		break;
	}
}

// 004cdb70
void FrClubSetWorkshopMainDlg::SetTab983(int tab)
{
	if (m_tab == tab)
		return;
	m_tab = tab;
	FrButton* pBtn = GetCtrl983<FrButton>("btn_tab_levelup");
	if (pBtn)
		pBtn->SetStatus(tab == 0 ? FrButton::PRESSED : FrButton::NORMAL);
	pBtn = GetCtrl983<FrButton>("btn_tab_rankup");
	if (pBtn)
		pBtn->SetStatus(tab == 1 ? FrButton::PRESSED : FrButton::NORMAL);
	pBtn = GetCtrl983<FrButton>("btn_tab_care");
	if (pBtn)
		pBtn->SetStatus(tab == 2 ? FrButton::PRESSED : FrButton::NORMAL);
	m_pLevelUp->SetShow983(tab == 0);
	m_pRankUp->SetShow983(tab == 1);
	m_pCare->SetShow983(tab == 2);
	ShowNotice983(tab);
}

// 004cdce0
void FrClubSetWorkshopMainDlg::LoadTabs983(lua_system::lua_table_helper* pTbl)
{
	FrButton* pBtn = GetCtrl983<FrButton>("btn_tab_levelup");
	if (pBtn)
	{
		int x = LUA983_GET(*pTbl, int, "btn_tab_levelup_x");
		int y = LUA983_GET(*pTbl, int, "btn_tab_levelup_y");
		WRect rc = pBtn->GetRect();
		pBtn->SetRect(WRect(x + rc.x, y + rc.y, rc.w, rc.h));
	}
	pBtn = GetCtrl983<FrButton>("btn_tab_rankup");
	if (pBtn)
	{
		int x = LUA983_GET(*pTbl, int, "btn_tab_rankup_x");
		int y = LUA983_GET(*pTbl, int, "btn_tab_rankup_y");
		WRect rc = pBtn->GetRect();
		pBtn->SetRect(WRect(x + rc.x, y + rc.y, rc.w, rc.h));
	}
	pBtn = GetCtrl983<FrButton>("btn_tab_care");
	if (pBtn)
	{
		int x = LUA983_GET(*pTbl, int, "btn_tab_of_x");
		int y = LUA983_GET(*pTbl, int, "btn_tab_of_y");
		WRect rc = pBtn->GetRect();
		pBtn->SetRect(WRect(x + rc.x, y + rc.y, rc.w, rc.h));
	}
	SetTab983(0);
}

// 004cded0 (o 983 lê a tabela mesmo sem o script)
void FrClubSetWorkshopMainDlg::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_main");
	LoadBg983(&tbl);
	LoadTabs983(&tbl);
	OffsetCtrlLua983("static_notice_clubworkhop", &tbl, "text_notice_pos");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->LoadLayout983();
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->LoadLayout983();
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->LoadLayout983();
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->LoadLayout983();
}

// 004ce000
bool FrClubSetWorkshopMainDlg::OnInit()
{
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->Init983();
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->Init983();
	if (FrClubSetOptionalFunctionsWnd::s_pInstance)
		FrClubSetOptionalFunctionsWnd::s_pInstance->Init983();
	if (FrClubSetWorkshopProgressWnd::s_pInstance)
		FrClubSetWorkshopProgressWnd::s_pInstance->Init983();
	if (ClubSetWorkshopFx983::s_pInstance)
		ClubSetWorkshopFx983::s_pInstance->Init983();
	LoadLayout983();
	if (m_pLevelUp)
		m_pLevelUp->SetShow983(true);
	if (m_pRankUp)
		m_pRankUp->SetShow983(false);
	if (m_pCare)
		m_pCare->SetShow983(false);
	if (m_pProgress)
		m_pProgress->SetShow983(false);
	EnableDrag(false);
	return ClubSetWorkShopForm::OnInit();
}
