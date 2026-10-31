// JP 983: FrClubSetWorkshopProgressWnd + FrClubSetWorkshopProgressResultWnd (andamento e resultado da oficina de tacos). Classes
// do 839/983, sem par no 645; ver include/jp/clubsetworkshopprogresswnd.h. Sessão B85 (F3.7.9, 10º lote). A unidade do 983 começa
// antes da faixa 0049eaa0–004a39f0 do mapa: métodos pequenos em 0049da20–0049e160 (entre os do Fx) e a ResultWnd em 0049e1a0….
// O compilador gera: 0049f830/0049e5b0 (`delete`), 0049ebd0 (ajuste da ChildWndBase), os vazios da ChildWndBase (0049eb90…) e os
// moldes do vector/string. As duas LoadLayout983 (004a0620 e 0049ed90) são da sessão B86.
#include "minatl.h"
#include <stdlib.h>
#include "projectg.h"
#include "bitmap.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frarea.h"
#include "frbutton.h"
#include "frstatic.h"
#include "fredit.h"
#include "frgaugebar.h"
#include "wlocalize.h"
#include "commonutil.h"
#include "inputmanager.h"
#include "lobbyjp.h"
#include "lua983.h"
#include "../../../include/packets/clubsetworkpackets.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopmaindlg.h"
#include "clubsetworkshopfx.h"
#include "clubsetinfownd.h"
#include "clubsetlevelupwnd.h"
#include "clubsetrankupwnd.h"
#include "clubsetworkshopprogresswnd.h"

extern Fresh* g_pFresh; // 00f02340
#define FORMGDI983() (FRMGR_GDI983(FRESHMGR983()))

// textos da unidade da FrClubSetWorkshopSelector (nomes meus)
const char* __fastcall GetStatName983(int stat); // 004a5470: nome do atributo (K2L)
const char* __fastcall GetLuckyChanceText983(); // 004a5590 (K2L)
const char* __fastcall GetRankUpTipText983(); // 004a54d0: uma de seis dicas ao acaso (K2L), no balão da subida de rank

FrClubSetWorkshopProgressWnd* FrClubSetWorkshopProgressWnd::s_pInstance = NULL;

// 0049da30
bool FrClubSetWorkshopProgressResultWnd::IsBegin983()
{
	return m_state == STATE_BEGIN;
}

// 0049da40
bool FrClubSetWorkshopProgressResultWnd::IsText983()
{
	return m_state == STATE_TEXT;
}

// 0049da50
bool FrClubSetWorkshopProgressResultWnd::IsMove983()
{
	return m_state == STATE_MOVE;
}

// 0049da60
bool FrClubSetWorkshopProgressResultWnd::IsEnd983()
{
	return m_state == STATE_END;
}

// 0049db40
bool FrClubSetWorkshopProgressWnd::StepGauge983(const float* pDeltaTime)
{
	if (!m_pGauge)
		return false;
	if (m_gaugeTime > m_gaugeTimeMax)
	{
		m_pGauge->SetDestPos(100, false);
		return true;
	}
	m_gaugeTime += *pDeltaTime;
	m_pGauge->SetDestPos((int)(m_gaugeTime / m_gaugeTimeMax * 100.0), false);
	return false;
}

// 0049dbc0
bool FrClubSetWorkshopProgressWnd::StepTimer983(const float* pDeltaTime)
{
	m_timer += *pDeltaTime;
	if (m_timer > m_timerMax)
	{
		m_timer = 0;
		return true;
	}
	return false;
}

// 0049dc00
void FrClubSetWorkshopProgressWnd::SetFx983(int index)
{
	switch (m_mode)
	{
	case MODE_LEVELUP:
		if (index == 0)
		{
			if (!m_bFx0)
			{
				ClubSetWorkshopFx983::StartFx983(0);
				m_bFx3 = false;
				m_bFx0 = true;
				return;
			}
		}
		else
		{
			ClubSetWorkshopFx983::StartFx983(index);
			m_bFx0 = false;
		}
		m_bFx3 = false;
		break;
	case MODE_RANKUP:
	case MODE_RANKUP_RESULT:
		if (index == 3)
		{
			if (!m_bFx3)
			{
				ClubSetWorkshopFx983::StartFx983(3);
				m_bFx0 = false;
				m_bFx3 = true;
				return;
			}
		}
		else
		{
			ClubSetWorkshopFx983::StartFx983(index);
			m_bFx3 = false;
		}
		m_bFx0 = false;
		break;
	}
}

// 0049dca0
void FrClubSetWorkshopProgressWnd::OnDrawProgressEclipse983()
{
	switch (m_mode)
	{
	case MODE_LEVELUP:
		ClubSetWorkshopFx983::DrawFx983(0);
		break;
	case MODE_RANKUP:
		ClubSetWorkshopFx983::DrawFx983(m_bfa983 ? 2 : 1);
		break;
	case MODE_RANKUP_RESULT:
		ClubSetWorkshopFx983::DrawFx983(1);
		break;
	}
}

// 0049dce0 (dobrada com outras vazias)
void FrClubSetWorkshopProgressWnd::OnDrawProgressbar983()
{
}

// 0049dcf0
float FrClubSetWorkshopProgressResultWnd::GetWaitTime983()
{
	return 1.0f;
}

// 0049dd00
float FrClubSetWorkshopProgressResultWnd::GetMoveTime983()
{
	return 1.0f;
}

// 0049dd40
void FrClubSetWorkshopProgressResultWnd::SetRankTo983(float x, float y)
{
	m_rankTo[0] = x;
	m_rankTo[1] = y;
}

// 0049dd60: as cinco posições finais do nível, lado a lado
void FrClubSetWorkshopProgressResultWnd::SetLevelTo983(float x, float y, float gap)
{
	for (int i = 0; i < 5; i++)
	{
		m_levelTo[i][0] = x + i * gap;
		m_levelTo[i][1] = y;
	}
}

// 0049dde0
void FrClubSetWorkshopProgressResultWnd::Begin983()
{
	m_state = STATE_BEGIN;
}

// 0049e030: troca a figura a cada intervalo
void sOverlayAnim983::Proc983(float deltaTime, ClubSetWorkshopHelper* pHelper)
{
	if (!frames.size() || !pHelper)
		return;
	if (interval > time)
	{
		time += deltaTime;
		return;
	}
	time = 0;
	for (unsigned int i = 0; i < frames.size(); i++)
		pHelper->SetOverlayUse983(frames[i], i == cur);
	cur++;
	if ((unsigned int)cur >= frames.size())
		cur = 0;
}

// 0049e0d0
void sOverlayAnim983::SetUse983(bool bUse, ClubSetWorkshopHelper* pHelper)
{
	if (!frames.size() || !pHelper)
		return;
	for (unsigned int i = 0; i < frames.size(); i++)
		pHelper->SetOverlayUse983(frames[i], bUse);
}

// 0049e160
void FrClubSetWorkshopProgressWnd::OnDrawProgressSubwndBg983()
{
	DrawOverlayByIndex983(m_bgOverlay, (int)(m_pForm->GetRect().x + m_bgPos[0]), (int)(m_pForm->GetRect().y + m_bgPos[1]), 0);
}

// 0049e1a0
FrClubSetWorkshopProgressResultWnd::~FrClubSetWorkshopProgressResultWnd()
{
	if (m_pClubTween)
	{
		delete m_pClubTween;
		m_pClubTween = NULL;
	}
	if (m_pLevelTween)
	{
		delete m_pLevelTween;
		m_pLevelTween = NULL;
	}
}

// 0049e240
void FrClubSetWorkshopProgressResultWnd::OnDrawTitle983()
{
	int title = m_title;
	if (title == -1)
		return;
	if (title == 2)
		DrawOverlayByIndex983(m_titleOverlay[2], (int)(m_luckyPos[0] + m_pParent->GetFormRect983().x),
			(int)(m_luckyPos[1] + m_pParent->GetFormRect983().y), 0);
	DrawOverlayByIndex983(m_titleOverlay[title], (int)(m_titlePos[0] + m_pParent->GetFormRect983().x),
		(int)(m_titlePos[1] + m_pParent->GetFormRect983().y), 0);
}

// 0049e3b0: inteiro pelo índice de uma tabela filha (declarada em lua983.h; a cópia fora de linha fica nesta unidade)
int LuaChild983::GetIntByIndex983(int index)
{
	if (table.m_obj)
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
			lua_remove(state, -1);
		else
		{
			lua_remove(state, -1);
			lua_rawgeti(state, table.m_obj->m_index, index);
			return lua_tinker::pop<int>(state);
		}
	}
	lua_pushnil(state);
	return lua_tinker::pop<int>(state);
}

// 0049e5a0
void FrClubSetWorkshopProgressWnd::OnDrawProgressResultTitle983()
{
	if (m_pResult)
		m_pResult->OnDrawTitle983();
}

// 0049ed90: posições e figuras do quadro do resultado (clubset_workshop.lua)
void FrClubSetWorkshopProgressResultWnd::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper infoTbl = script->GetTable("clubset_workshop_ui_info");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_progress");
	m_titlePos[0] = LUA983_POS(tbl, "img_result_title_pos", 1);
	m_titlePos[1] = LUA983_POS(tbl, "img_result_title_pos", 2);
	m_luckyPos[0] = LUA983_POS(tbl, "img_result_title_luckychance_pos", 1);
	m_luckyPos[1] = LUA983_POS(tbl, "img_result_title_luckychance_pos", 2);
	m_clubPos[0] = LUA983_POS(tbl, "icon_result_clubset_pos", 1);
	m_clubPos[1] = LUA983_POS(tbl, "icon_result_clubset_pos", 2);
	m_clubDrawPos[0] = LUA983_POS(tbl, "icon_result_clubset_pos0", 1);
	m_clubDrawPos[1] = LUA983_POS(tbl, "icon_result_clubset_pos0", 2);
	m_clubChangePos[0] = LUA983_POS(tbl, "icon_result_clubset_pos1", 1);
	m_clubChangePos[1] = LUA983_POS(tbl, "icon_result_clubset_pos1", 2);
	m_textPos[0] = LUA983_POS(tbl, "text_result_pos", 1);
	m_textPos[1] = LUA983_POS(tbl, "text_result_pos", 2);
	m_luckyTextPos[0] = LUA983_POS(tbl, "text_result_luckychance_pos", 1);
	m_luckyTextPos[1] = LUA983_POS(tbl, "text_result_luckychance_pos", 2);
	m_levelPos[0] = LUA983_POS(tbl, "icon_result_level_pos", 1);
	m_levelPos[1] = LUA983_POS(tbl, "icon_result_level_pos", 2);
	m_rankPos[0] = LUA983_POS(tbl, "icon_result_rank_pos", 1);
	m_rankPos[1] = LUA983_POS(tbl, "icon_result_rank_pos", 2);
	m_clubTo[0] = LUA983_POS(tbl, "clubset_icon_pos", 1);
	m_clubTo[1] = LUA983_POS(tbl, "clubset_icon_pos", 2);
	float rx = LUA983_POS(tbl, "rank_icon_pos", 1);
	float ry = LUA983_POS(tbl, "rank_icon_pos", 2);
	SetRankTo983(rx, ry);
	float lx = LUA983_POS(tbl, "level_icon_pos", 1);
	float ly = LUA983_POS(tbl, "level_icon_pos", 2);
	float gap = LUA983_GET(tbl, float, "level_icon_gap");
	SetLevelTo983(lx, ly, gap);
	m_titleOverlay[0] = LoadOverlay983(LUA983_GET(tbl, const char*, "img_result_title_levelup_filename"), true);
	m_titleOverlay[1] = LoadOverlay983(LUA983_GET(tbl, const char*, "img_result_title_rankup_filename"), true);
	m_titleOverlay[2] = LoadOverlay983(LUA983_GET(tbl, const char*, "img_result_title_luckychance_filename"), true);
	m_titleOverlay[3] = LoadOverlay983(LUA983_GET(tbl, const char*, "img_result_title_clubchange_filename"), true);
	m_rankOverlay[0] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_e")), true);
	m_rankOverlay[1] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_d")), true);
	m_rankOverlay[2] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_c")), true);
	m_rankOverlay[3] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_b")), true);
	m_rankOverlay[4] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_a")), true);
	m_rankOverlay[5] = LoadOverlay983(MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_s")), true);
	const char* fmt = LUA983_GET(infoTbl, const char*, "level_icon_n");
	for (int i = 0; i < 5; i++)
	{
		const char* name = MakeStr(fmt, i + 1);
		m_levelOverlay[i] = LoadOverlay983(name, true);
	}
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 0049eaa0
FrClubSetWorkshopProgressWnd::~FrClubSetWorkshopProgressWnd()
{
	s_pInstance = NULL;
	if (m_pResult)
	{
		delete m_pResult;
		m_pResult = NULL;
	}
}

// 0049ebe0
void FrClubSetWorkshopProgressWnd::ResetButtons983()
{
	m_gaugeTime = 0;
	m_bRunning = false;
	m_bWorking = false;
	if (m_pGauge)
	{
		m_pGauge->ResetGauge();
		m_pGauge->SetDestPos(0, false);
	}
	if (m_bShow)
	{
		ShowCtrl983("btn_workshop_apply", true);
		ShowCtrl983("btn_workshop_cancel", true);
		ShowCtrl983("btn_workshop_ok", false);
		SetOverlayUse983(m_bgOverlay, false);
	}
}

// 0049f720
FrClubSetWorkshopProgressWnd::FrClubSetWorkshopProgressWnd(ClubSetWorkShopForm* pForm)
{
	s_pInstance = this;
	m_gaugeTime = 0;
	m_gaugeTimeMax = 0;
	m_pForm = pForm;
	m_pad983_f0 = 0;
	m_bRunning = false;
	m_bWorking = false;
	m_pad983_f4 = 0;
	m_mode = -1;
	m_timer = 0;
	m_pGauge = NULL;
	m_timerMax = 0;
	m_pResult = NULL;
	m_pad983_d4 = 0;
	m_balloonOverlay = 0;
	m_bfa983 = true;
	m_bNoCancel = false;
	m_oldRank = 0;
	m_newRank = 0;
	m_bFx0 = false;
	m_bFx3 = false;
}

// 0049fde0
FrClubSetWorkshopProgressWnd* FrClubSetWorkshopMainDlg::CreateProgress983()
{
	return new FrClubSetWorkshopProgressWnd(this);
}

// 004a0310
void FrClubSetWorkshopProgressWnd::ShowRankUpBalloon983(bool bShow)
{
	SetOverlayUse983(m_balloonOverlay, bShow);
	FrStatic* pStatic = GetCtrl983<FrStatic>("static_speakballoon_rankup");
	if (pStatic)
		pStatic->SetVisible(bShow);
}

// 004a03e0: volta os botões (bDone = o trabalho foi aceito/acabou)
void FrClubSetWorkshopProgressWnd::Back983(bool bDone)
{
	EnableCtrl983("btn_workshop_apply", !bDone);
	EnableCtrl983("btn_workshop_ok", !bDone);
	SetOverlayUse983(m_bgOverlay, !m_bfa983);
	FrButton* pBtn = GetCtrl983<FrButton>("btn_workshop_cancel");
	if (!pBtn)
		return;
	if (m_mode == MODE_LEVELUP)
	{
		if (bDone)
			pBtn->Enable(false);
		else
		{
			pBtn->Enable(!m_bNoCancel);
			pBtn->SetToolTipText(m_bNoCancel ? K2L_Compatibility("\263\262\276\306\300\326\264\302 \260\263\301\266 \303\353\274\322 "
				"\260\241\264\311 \310\275\274\366\260\241 \276\370\275\300\264\317\264\331.") : "");
		}
	}
	else
		pBtn->Enable(bDone ? false : m_bfa983);
}

// 004a0620: posições, figuras e controles da janela de andamento (clubset_workshop.lua); chamada pela LoadLayout983 da
// MainDlg (004cded0). Fora de linha no 983 e embutidas aqui: 5 das 7 ShowCtrl983 (0049e860 nas duas últimas).
void FrClubSetWorkshopProgressWnd::LoadLayout983()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper infoTbl = script->GetTable("clubset_workshop_ui_info");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_progress");
	m_gaugeTimeMax = LUA983_GET(tbl, float, "animation_play_time");
	const char* fmt = LUA983_GET(tbl, const char*, "img_loading_anim_filename");
	for (int i = 0; i < 4; i++)
	{
		m_animOverlay[i] = LoadOverlay983(MakeStr(fmt, i + 1), i == 0);
		m_anim.frames.push_back(m_animOverlay[i]);
	}
	m_anim.interval = LUA983_GET(tbl, float, "loading_anim_delay_time");
	OffsetCtrlLua983("area_progress_eclipse", &tbl, "img_eclipse_pos");
	OffsetCtrlLua983("area_progress_subwnd_bg", &tbl, "bg_subwnd_pos");
	OffsetCtrlLua983("area_progress_animation", &tbl, "area_animation_pos");
	OffsetCtrlLua983("gauegebar_progress", &tbl, "gaugebar_pos");
	OffsetCtrlLua983("btn_workshop_apply", &tbl, "btn_apply_pos");
	OffsetCtrlLua983("btn_workshop_cancel", &tbl, "btn_cancel_pos");
	OffsetCtrlLua983("btn_workshop_ok", &tbl, "btn_ok_pos");
	OffsetCtrlLua983("area_speakballoon_rankup", &tbl, "area_speech_balloon_pos");
	OffsetCtrlLua983("static_speakballoon_rankup", &tbl, "text_speech_balloon_pos");
	OffsetCtrlLua983("area_progress_result_title", &tbl, "img_result_title_pos");
	float x = LUA983_POSI(tbl, "icon_result_clubset_pos", 1);
	float y = LUA983_POSI(tbl, "icon_result_clubset_pos", 2);
	FrWnd* pWnd = GetCtrl983<FrWnd>("area_progress_result_clubset");
	if (pWnd)
	{
		const WRect& rc = pWnd->GetRect();
		pWnd->SetRect(WRect(m_pForm->GetRect().x + x, m_pForm->GetRect().y + y, rc.w, rc.h));
	}
	x = LUA983_POSI(tbl, "icon_result_level_pos", 1);
	y = LUA983_POSI(tbl, "icon_result_level_pos", 2);
	pWnd = GetCtrl983<FrWnd>("area_progress_result_level");
	if (pWnd)
	{
		const WRect& rc = pWnd->GetRect();
		pWnd->SetRect(WRect(m_pForm->GetRect().x + x, m_pForm->GetRect().y + y, rc.w, rc.h));
	}
	x = LUA983_POSI(tbl, "icon_result_rank_pos", 1);
	y = LUA983_POSI(tbl, "icon_result_rank_pos", 2);
	pWnd = GetCtrl983<FrWnd>("area_progress_result_rank");
	if (pWnd)
	{
		const WRect& rc = pWnd->GetRect();
		pWnd->SetRect(WRect(m_pForm->GetRect().x + x, m_pForm->GetRect().y + y, rc.w, rc.h));
	}
	x = LUA983_POSI(tbl, "text_result_pos", 1);
	y = LUA983_POSI(tbl, "text_result_pos", 2);
	pWnd = GetCtrl983<FrWnd>("edit_progress_result_text");
	if (pWnd)
	{
		const WRect& rc = pWnd->GetRect();
		pWnd->SetRect(WRect(m_pForm->GetRect().x + x, m_pForm->GetRect().y + y, rc.w, rc.h));
	}
	m_pGauge = GetCtrl983<FrGaugeBarImage>("gauegebar_progress");
	if (m_pGauge)
	{
		m_pGauge->SetRange(0, 100, 0);
		m_pGauge->SetBarSpeed(LUA983_GET(tbl, float, "gaugebar_speed"));
	}
	ShowCtrl983("area_progress_eclipse", false);
	ShowCtrl983("area_progress_subwnd_bg", false);
	ShowCtrl983("area_progress_animation", false);
	ShowCtrl983("gauegebar_progress", false);
	ShowCtrl983("btn_workshop_apply", false);
	ShowCtrl983("btn_workshop_cancel", false);
	ShowCtrl983("btn_workshop_ok", false);
	const char* balloon = LUA983_GET(tbl, const char*, "img_speech_balloon_filename");
	m_balloonOverlay = LoadOverlay983(balloon, false);
	FrEdit* pEdit = GetCtrl983<FrEdit>("static_speakballoon_rankup");
	if (pEdit)
	{
		pEdit->AddText(GetRankUpTipText983(), false, true, 0, 0);
		pEdit->SetVisible(false);
	}
	ShowRankUpBalloon983(false);
	m_bgOverlay = LoadOverlay983(LUA983_GET(tbl, const char*, "btn_ok_bg_filename"), false);
	m_bgPos[0] = (float)((LuaHelper983&)tbl).GetChild983("btn_ok_bg_pos").GetIntByIndex983(1);
	m_bgPos[1] = (float)((LuaHelper983&)tbl).GetChild983("btn_ok_bg_pos").GetIntByIndex983(2);
	FrWnd* pEclipse = GetCtrl983<FrWnd>("area_progress_eclipse");
	if (pEclipse)
	{
		const WRect* pRect = &pEclipse->GetRect();
		ClubSetWorkshopFx983::SetFxRect983(0, pRect);
		ClubSetWorkshopFx983::SetFxRect983(1, pRect);
		ClubSetWorkshopFx983::SetFxRect983(2, pRect);
	}
	m_timerMax = LUA983_GET(mainTbl, float, "fx_loading_playtime");
	if (m_pResult)
		m_pResult->LoadLayout983();
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 004a1860: as quatro figuras da animação no "area_progress_animation"
void FrClubSetWorkshopProgressWnd::OnDrawProgressAnimation983()
{
	FrWnd* pWnd = GetCtrl983<FrWnd>("area_progress_animation");
	if (pWnd)
	{
		const WRect& rc = pWnd->GetRect();
		for (int i = 0; i < 4; i++)
			DrawOverlayByIndex983(m_animOverlay[i], (int)rc.x, (int)rc.y, 0);
	}
}

// 004a1950: aplicar o resultado (subir o atributo / o rank)
void FrClubSetWorkshopProgressWnd::OnWorkshopApplyBtnUp983()
{
	if (m_mode == MODE_LEVELUP)
	{
		ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(0);
		FrClubSetInfoWnd* pInfo = FrClubSetLevelUpWnd::s_pInstance->m_pInfoWnd;
		if (pSlot && pInfo)
		{
			ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
			if (pDlg)
			{
				int stat = pInfo->m_selStat;
				int cur = ClubSetWorkshopFx983::GetSlotStat983(0, stat);
				int level = pSlot->level;
				pDlg->SetSize983(320.0f, 290.0f, true);
				pDlg->SetMessage(MakeStr(K2L_Compatibility("[%s] :: %d -> %d\012\012\260\341\301\244 \310\304\277\241\264\302 "
					"\134c0xffff0000\134c\303\353\274\322\260\241 \272\322\260\241\264\311\134c0xff000000\134c\307\317\270\347,\012"
					"\305\254\267\264\274\274\306\256\300\307 \134c0xffff0000\134c\260\305\267\241\260\241 \272\322\260\241\264\311"
					"\134c0xff000000\134c\307\330\301\375\264\317\264\331.\012\307\366\300\347\300\307 \260\263\301\266 \273\363\305\302"
					"\267\316 \260\341\301\244\307\317\275\303\260\332\275\300\264\317\261\356?"), GetStatName983(stat), cur, cur + 1), false);
				pDlg->AddBitmap983(pSlot->pBmp);
				pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap(MakeStr("clubmade_rank%d_normal", level + 2)));
				pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnProgressApplyResult983, 1);
				if (FrClubSetLevelUpWnd::s_pInstance)
					FrClubSetLevelUpWnd::s_pInstance->OnEndWork983();
			}
		}
	}
	else if (m_mode == MODE_RANKUP)
	{
		ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
		if (pDlg)
		{
			pDlg->SetSize983(320.0f, 310.0f, false);
			pDlg->SetMessage(K2L_Compatibility("\307\366\300\347 \305\254\267\264\300\314 \273\347\266\363\301\366\260\355,\012"
				"\273\365\267\316\277\356 \305\254\267\264\300\270\267\316 \271\331\262\356\270\347\012\272\257\310\255\265\307\264\302 "
				"\301\244\272\270\264\302 \276\306\267\241\277\315 \260\260\275\300\264\317\264\331.\012\012\134c0xffff0000\134c- "
				"\304\253\265\345 \300\316\260\350\012- \274\367\267\303\265\265 \303\312\261\342\310\255\012\012\134c0xff000000\134c"
				"\305\254\267\264 \272\257\310\255\270\246 \301\370\307\340\307\317\275\303\260\332\275\300\264\317\261\356?\012"
				"\134c0xffff0000\134c(\260\341\301\244 \310\304\277\241\264\302 \303\353\274\322\260\241 \272\322\260\241\264\311\307\325"
				"\264\317\264\331.)"), false);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnProgressApplyResultB983, 1);
			if (FrClubSetRankUpWnd::s_pInstance)
				FrClubSetRankUpWnd::s_pInstance->OnEndWork983();
		}
	}
	Back983(true);
}

// 004a1b60: cancelar o resultado
void FrClubSetWorkshopProgressWnd::OnWorkshopCancelBtnUp983()
{
	if (m_mode == MODE_LEVELUP)
	{
		ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(0);
		FrClubSetInfoWnd* pInfo = FrClubSetLevelUpWnd::s_pInstance->m_pInfoWnd;
		if (pSlot && pInfo)
		{
			ClubSetWorkShopForm* pDlg = CreateForm<ClubSetWorkShopForm>(g_pFresh->GetManager(), m_pForm, "clubsetwork_okcancel", NULL);
			if (pDlg)
			{
				int stat = pInfo->m_selStat;
				int cur = ClubSetWorkshopFx983::GetSlotStat983(0, stat);
				int level = pSlot->level;
				pDlg->SetSize983(320.0f, 280.0f, true);
				pDlg->SetMessage(MakeStr(K2L_Compatibility("[%s] :: %d -> %d\012\012\303\353\274\322\307\317\270\351 \260\263\301\266 "
					"\303\353\274\322 \260\241\264\311 \310\275\274\366\260\241 \274\322\270\360\265\313\264\317\264\331.\012\307\366\300\347"
					"\300\307 \260\263\301\266 \273\363\305\302\270\246 \303\353\274\322\307\317\275\303\260\332\275\300\264\317\261\356?"),
					GetStatName983(stat), cur, cur + 1), false);
				pDlg->AddBitmap983(pSlot->pBmp);
				pDlg->AddBitmap983((Bitmap*)g_pFresh->GetBitmap(MakeStr("clubmade_rank%d_normal", level + 2)));
				pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnProgressCancelResult983, 1);
				if (FrClubSetLevelUpWnd::s_pInstance)
					FrClubSetLevelUpWnd::s_pInstance->OnEndWork983();
			}
		}
	}
	else if (m_mode == MODE_RANKUP)
	{
		FrForm* pDlg = CreateForm<FrForm>(g_pFresh->GetManager(), m_pForm, "notify_okcancel", NULL);
		if (pDlg)
		{
			pDlg->SetMessage(K2L_Compatibility("\305\254\267\264 \272\257\310\255 \276\370\300\314 \300\317\271\335 \267\251\305\251"
				"\276\367\300\273 \301\370\307\340\307\325\264\317\264\331.\012\012\300\317\271\335 \267\251\305\251\276\367\300\273 "
				"\301\370\307\340\307\317\275\303\260\332\275\300\264\317\261\356?"), false);
			pDlg->Open((FRESH_PFN_RESULT)&FrClubSetWorkshopMainDlg::OnProgressCancelConfirmResult983, 0x801);
			if (FrClubSetRankUpWnd::s_pInstance)
				FrClubSetRankUpWnd::s_pInstance->OnEndWork983();
		}
	}
	Back983(true);
}

// 004a1d50
void FrClubSetWorkshopProgressWnd::OnDrawSpeakballoonRankup983()
{
	FrArea* pArea = GetCtrl983<FrArea>("area_speakballoon_rankup");
	if (pArea)
		DrawOverlayByIndex983(m_balloonOverlay, (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004a1e20
bool FrClubSetWorkshopProgressWnd::OnApplyResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
		SendClubSetWork165_983(true);
	else
		Back983(false);
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnBeginWork983();
	return true;
}

// 004a1e50
bool FrClubSetWorkshopProgressWnd::OnCancelResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
		SendClubSetWork165_983(false);
	else
		Back983(false);
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->OnBeginWork983();
	return true;
}

// 004a1e80
bool FrClubSetWorkshopProgressWnd::OnApplyResultB983(int result, FrForm* pForm)
{
	if (result == FrOK)
		SendClubSetWork168_983(true);
	else
		Back983(false);
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnBeginWork983();
	return true;
}

// 004a1eb0
bool FrClubSetWorkshopProgressWnd::OnCancelConfirmResult983(int result, FrForm* pForm)
{
	if (result == FrOK)
		SendClubSetWork168_983(false);
	else
		Back983(false);
	if (FrClubSetRankUpWnd::s_pInstance)
		FrClubSetRankUpWnd::s_pInstance->OnBeginWork983();
	return true;
}

// 004a1ee0
void FrClubSetWorkshopProgressWnd::Start983(bool bStart)
{
	if (!bStart)
	{
		Back983(false);
		return;
	}
	ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlot983(0);
	if (pSlot)
		m_bNoCancel = pSlot->recoveryLeft == 0;
	m_bWorking = bStart;
	m_mode = MODE_LEVELUP;
}

// 004a1f30
void FrClubSetWorkshopProgressResultWnd::Proc983(const float* pDeltaTime)
{
	if (m_state == STATE_END)
		return;
	if (m_state == STATE_BEGIN)
		m_state = STATE_TEXT;
	if (m_state == STATE_TEXT)
	{
		FrWnd* pWnd = m_pParent->GetResultCtrl983<FrWnd>("edit_progress_result_text");
		if (pWnd)
			pWnd->SetVisible(true);
	}
	if (m_state == STATE_MOVE && m_pClubTween->IsEnd983() && m_pLevelTween->IsEnd983())
	{
		m_wait -= *pDeltaTime;
		if (m_wait < 0)
		{
			m_state = STATE_END;
			m_wait = GetWaitTime983();
		}
	}
	if (m_pClubTween)
		m_pClubTween->Proc983(pDeltaTime);
	if (m_pLevelTween)
		m_pLevelTween->Proc983(pDeltaTime);
}

// 004a2030
void FrClubSetWorkshopProgressResultWnd::Reset983()
{
	m_pClubBmp = NULL;
	m_pClubBmp2 = NULL;
	m_state = STATE_NONE;
	m_title = -1;
	m_level = -1;
	m_rank = -1;
	m_stat = -1;
	FrWnd* pWnd = m_pParent->GetResultCtrl983<FrWnd>("edit_progress_result_text");
	if (pWnd)
		pWnd->SetVisible(false);
	pWnd = m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_clubset");
	if (pWnd)
		pWnd->SetRect(WRect(m_clubPos[0] + m_pParent->GetFormRect983().x, m_clubPos[1] + m_pParent->GetFormRect983().y,
			pWnd->GetRect().w, pWnd->GetRect().h));
	pWnd = m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_level");
	if (pWnd)
		pWnd->SetRect(WRect(m_levelPos[0] + m_pParent->GetFormRect983().x, m_levelPos[1] + m_pParent->GetFormRect983().y,
			pWnd->GetRect().w, pWnd->GetRect().h));
	pWnd = m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_rank");
	if (pWnd)
		pWnd->SetRect(WRect(m_rankPos[0] + m_pParent->GetFormRect983().x, m_rankPos[1] + m_pParent->GetFormRect983().y,
			pWnd->GetRect().w, pWnd->GetRect().h));
	FrArea* pArea = m_pParent->GetResultCtrl983<FrArea>("area_progress_result_clubset");
	if (pArea)
		pArea->SetRect(WRect(m_clubPos[0] + m_pParent->GetFormRect983().x, m_clubPos[1] + m_pParent->GetFormRect983().y,
			pArea->GetRect().w, pArea->GetRect().h));
}

// 004a22c0: o taco (e o nível ou o rank) voam da posição inicial para a final, com curva sorteada
void FrClubSetWorkshopProgressResultWnd::StartMove983()
{
	int easeX = rand() % 22;
	int easeY = rand() % 22;
	if (easeX % 3 == 1)
		easeX++;
	if (easeY % 3 == 1)
		easeY++;
	if (m_pClubTween)
	{
		m_pClubTween->Attach983(m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_clubset"));
		float time = GetMoveTime983();
		m_pClubTween->SetX983(easeX, m_clubPos[0] + m_pParent->GetFormRect983().x, m_clubTo[0] + m_pParent->GetFormRect983().x, time);
		m_pClubTween->SetY983(easeY, m_pParent->GetFormRect983().y + m_clubPos[1], m_pParent->GetFormRect983().y + m_clubTo[1], time);
	}
	if (m_title == 0)
	{
		if (m_pLevelTween)
		{
			m_pLevelTween->Attach983(m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_level"));
			float time = GetMoveTime983();
			m_pLevelTween->SetX983(easeX, m_levelPos[0] + m_pParent->GetFormRect983().x,
				m_levelTo[m_level][0] + m_pParent->GetFormRect983().x, time);
			m_pLevelTween->SetY983(easeY, m_pParent->GetFormRect983().y + m_levelPos[1],
				m_levelTo[m_level][1] + m_pParent->GetFormRect983().y, time);
		}
		ClubSetWorkshopFx983::StartFx983(2);
	}
	else if (m_title == 1 || m_title == 3)
	{
		if (m_pLevelTween)
		{
			m_pLevelTween->Attach983(m_pParent->GetResultCtrl983<FrWnd>("area_progress_result_rank"));
			float time = GetMoveTime983();
			m_pLevelTween->SetX983(easeX, m_rankPos[0] + m_pParent->GetFormRect983().x, m_rankTo[0] + m_pParent->GetFormRect983().x, time);
			m_pLevelTween->SetY983(easeY, m_pParent->GetFormRect983().y + m_rankPos[1], m_pParent->GetFormRect983().y + m_rankTo[1], time);
		}
		ClubSetWorkshopFx983::StartFx983(5);
	}
	m_state = STATE_MOVE;
}

// 004a2670
void FrClubSetWorkshopProgressResultWnd::SetText983(std::string text)
{
	if (!m_pParent)
		return;
	FrEdit* pEdit = m_pParent->GetResultCtrl983<FrEdit>("edit_progress_result_text");
	if (!pEdit)
		return;
	pEdit->ClearLine();
	pEdit->AddText(text.c_str(), false, true, 0, 0);
	if (m_title == 2)
		pEdit->SetRect(WRect(m_luckyTextPos[0] + m_pParent->GetFormRect983().x, m_luckyTextPos[1] + m_pParent->GetFormRect983().y,
			pEdit->GetRect().w, pEdit->GetRect().h));
	else
		pEdit->SetRect(WRect(m_textPos[0] + m_pParent->GetFormRect983().x, m_textPos[1] + m_pParent->GetFormRect983().y,
			pEdit->GetRect().w, pEdit->GetRect().h));
}

// 004a27d0
void FrClubSetWorkshopProgressResultWnd::OnDrawClubset983()
{
	FrArea* pArea = m_pParent->GetResultCtrl983<FrArea>("area_progress_result_clubset");
	if (!pArea)
		return;
	FrGraphicInterface* pGDI = FORMGDI983();
	if (!pGDI)
		return;
	if (m_pClubBmp)
	{
		if (m_title == 3)
			pGDI->DrawTexture(m_pClubBmp, WRect(m_clubDrawPos[0] + m_pParent->GetFormRect983().x,
				m_clubDrawPos[1] + m_pParent->GetFormRect983().y, (float)m_pClubBmp->Width(), (float)m_pClubBmp->Height()),
				0xffffffff, 0);
		else
			pGDI->DrawTexture(m_pClubBmp, WRect(pArea->GetRect().x, pArea->GetRect().y, (float)m_pClubBmp->Width(),
				(float)m_pClubBmp->Height()), 0xffffffff, 0);
	}
	if (m_pClubBmp2)
		pGDI->DrawTexture(m_pClubBmp2, WRect(pArea->GetRect().x, pArea->GetRect().y, (float)m_pClubBmp2->Width(),
			(float)m_pClubBmp2->Height()), 0xffffffff, 0);
}

// 004a2980
void FrClubSetWorkshopProgressResultWnd::OnDrawLevel983()
{
	FrArea* pArea = m_pParent->GetResultCtrl983<FrArea>("area_progress_result_level");
	if (pArea && m_level != -1)
		DrawOverlayByIndex983(m_levelOverlay[m_level], (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004a29f0
void FrClubSetWorkshopProgressResultWnd::OnDrawRank983()
{
	FrArea* pArea = m_pParent->GetResultCtrl983<FrArea>("area_progress_result_rank");
	if (pArea && m_rank != -1)
		DrawOverlayByIndex983(m_rankOverlay[m_rank], (int)pArea->GetRect().x, (int)pArea->GetRect().y, 0);
}

// 004a2a70: o pedido foi mandado; a barra começa a andar
void FrClubSetWorkshopProgressWnd::OnSent983()
{
	m_bRunning = true;
	SetShow983(true);
	FrButton* pBtn = GetCtrl983<FrButton>("btn_workshop_apply");
	if (pBtn)
	{
		pBtn->SetVisible(true);
		pBtn->Enable(false);
	}
	pBtn = GetCtrl983<FrButton>("btn_workshop_cancel");
	if (pBtn)
	{
		pBtn->SetVisible(true);
		pBtn->Enable(false);
	}
	pBtn = GetCtrl983<FrButton>("btn_workshop_ok");
	if (pBtn)
	{
		pBtn->SetVisible(false);
		pBtn->Enable(false);
		SetOverlayUse983(m_bgOverlay, false);
	}
	ShowRankUpBalloon983(false);
	if (m_pResult)
		m_pResult->Reset983();
}

// 004a2c80
void FrClubSetWorkshopProgressWnd::Finish983()
{
	if (m_pResult)
		m_pResult->Reset983();
	SetShow983(false);
	ResetButtons983();
	ClubSetWorkshopChildWndBase* pPage = m_mode == MODE_LEVELUP ? (ClubSetWorkshopChildWndBase*)FrClubSetLevelUpWnd::s_pInstance
		: (ClubSetWorkshopChildWndBase*)FrClubSetRankUpWnd::s_pInstance;
	if (pPage)
		pPage->OnEndWork983();
	m_mode = -1;
	ClubSetWorkshopFx983::StopFx983(0);
	ClubSetWorkshopFx983::StopFx983(1);
	ClubSetWorkshopFx983::StopFx983(2);
}

// 004a2cf0
void FrClubSetWorkshopProgressWnd::OnWorkshopOkBtnUp983()
{
	if (m_mode == MODE_RANKUP || m_mode == MODE_RANKUP_RESULT)
	{
		if (m_pResult)
			m_pResult->StartMove983();
		Back983(true);
		return;
	}
	Finish983();
	Back983(true);
}

// 004a2d30
void FrClubSetWorkshopProgressWnd::OnDrawProgressResultClubset983()
{
	if (m_pResult)
		m_pResult->OnDrawClubset983();
}

// 004a2d40
void FrClubSetWorkshopProgressWnd::OnDrawProgressResultLevel983()
{
	if (m_pResult)
		m_pResult->OnDrawLevel983();
}

// 004a2d50
void FrClubSetWorkshopProgressWnd::OnDrawProgressResultRank983()
{
	if (m_pResult)
		m_pResult->OnDrawRank983();
}

// 004a2d60
void FrClubSetWorkshopProgressWnd::StartRankUpResult983(unsigned long oldRank, unsigned long newRank)
{
	ResetButtons983();
	OnSent983();
	m_mode = MODE_RANKUP_RESULT;
	m_bWorking = true;
	m_bfa983 = false;
	ShowRankUpBalloon983(false);
	m_oldRank = oldRank;
	m_newRank = newRank;
	ClubSetWorkshopFx983::ResetSlot983(0);
	if (FrClubSetLevelUpWnd::s_pInstance)
		FrClubSetLevelUpWnd::s_pInstance->SelectClub983(0);
}

float CameraEase983(float b, float e, float t, float d, int type); // lobbybg.cpp: curvas de Penner 004db270-004dba80

// 004db130 (ctor chama Attach983 com o argumento)
CTween983::CTween983(int pWnd)
{
	m_fromX = m_fromY = 0;
	m_easeX = m_easeY = -1;
	m_toX = m_toY = 0;
	m_timeX = m_timeY = 0;
	m_pWnd = NULL;
	m_curX = m_curY = 0;
	m_elapX = m_elapY = 0;
	m_bRunX = false;
	m_bRunY = false;
	Attach983((FrWnd*)pWnd);
}

// 004dabe0
CTween983::~CTween983()
{
	m_fromX = m_fromY = 0;
	m_pWnd = NULL;
	m_toX = m_toY = 0;
	m_easeX = m_easeY = -1;
	m_timeX = m_timeY = 0;
	m_bRunX = m_bRunY = false;
	m_curX = m_curY = 0;
	m_elapX = m_elapY = 0;
}

// 004db080
void CTween983::Attach983(FrWnd* pWnd)
{
	if (!pWnd)
		return;
	m_curX = m_fromX = pWnd->GetRect().x;
	m_pWnd = pWnd;
	m_curY = m_fromY = pWnd->GetRect().y;
	m_elapX = 0;
	m_elapY = 0;
}

// 004dac20
void CTween983::SetX983(int ease, float from, float to, float time)
{
	if (!m_pWnd)
		return;
	m_curX = from;
	m_easeX = ease;
	m_fromX = from;
	m_bRunX = true;
	m_toX = to;
	m_timeX = time;
}

// 004dac60
void CTween983::SetY983(int ease, float from, float to, float time)
{
	if (!m_pWnd)
		return;
	m_curY = from;
	m_easeY = ease;
	m_fromY = from;
	m_bRunY = true;
	m_toY = to;
	m_timeY = time;
}

// 004db180 (+ 004db0c0: SetRect na posicao atual, ou no destino quando parou)
void CTween983::Proc983(const float* pDeltaTime)
{
	if (!m_bRunX && !m_bRunY)
		return;
	if (m_bRunX)
	{
		m_curX = CameraEase983(m_fromX, m_toX, m_elapX, m_timeX, m_easeX);
		m_elapX = *pDeltaTime + m_elapX;
		m_bRunX = m_elapX < m_timeX;
	}
	if (m_bRunY)
	{
		m_curY = CameraEase983(m_fromY, m_toY, m_elapY, m_timeY, m_easeY);
		m_elapY = m_elapY + *pDeltaTime;
		m_bRunY = m_elapY < m_timeY;
	}
	if (m_pWnd)
	{
		const WRect& r = m_pWnd->GetRect();
		bool run = m_bRunX || m_bRunY;
		WRect rc(run ? m_curX : m_toX, run ? m_curY : m_toY, r.w, r.h);
		m_pWnd->SetRect(rc);
	}
}

// 004a2dd0
FrClubSetWorkshopProgressResultWnd::FrClubSetWorkshopProgressResultWnd(FrClubSetWorkshopProgressWnd* pParent)
{
	m_pParent = pParent;
	m_pClubTween = new CTween983(0);
	m_pLevelTween = new CTween983(0);
	m_clubPos[0] = 0;
	m_clubPos[1] = 0;
	m_clubDrawPos[0] = 0;
	m_clubDrawPos[1] = 0;
	m_clubChangePos[0] = 0;
	m_clubChangePos[1] = 0;
	m_levelPos[0] = 0;
	m_levelPos[1] = 0;
	m_rankPos[0] = 0;
	m_rankPos[1] = 0;
	m_wait = GetWaitTime983();
	Reset983();
}

// 004a2ed0
void FrClubSetWorkshopProgressResultWnd::SetResult983(int kind, unsigned long typeIdA, unsigned long typeIdB)
{
	m_title = kind;
	switch (kind)
	{
	case 0:
		if (FrClubSetLevelUpWnd::s_pInstance)
		{
			ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlotPreview983(0, -1, false);
			if (pSlot)
			{
				m_pClubBmp = pSlot->pBmp;
				m_level = pSlot->level;
			}
			FrClubSetInfoWnd* pInfo = FrClubSetLevelUpWnd::s_pInstance->m_pInfoWnd;
			if (pSlot && pInfo)
			{
				m_stat = pInfo->m_selStat;
				int cur = ClubSetWorkshopFx983::GetSlotStat983(0, pInfo->m_selStat);
				SetText983(MakeStr("[%s] :: %d -> %d", GetStatName983(m_stat), cur, cur + 1));
			}
		}
		break;
	case 1:
		if (FrClubSetRankUpWnd::s_pInstance)
		{
			ClubSetWorkshopFx983::sSlot983* pSlot = ClubSetWorkshopFx983::GetSlotPreview983(1, -1, false);
			if (pSlot)
			{
				m_pClubBmp = pSlot->pBmp;
				m_rank = pSlot->rank;
			}
			FrClubSetInfoWnd* pInfo = FrClubSetRankUpWnd::s_pInstance->m_pInfoWnd;
			if (pSlot && pInfo)
			{
				m_stat = pInfo->m_selStat;
				int before = ClubSetWorkshopFx983::GetSlotStat983(1, pInfo->m_selStat);
				ClubSetWorkshopFx983::ClearSlot983(1);
				int after = ClubSetWorkshopFx983::GetSlotStat983(1, m_stat);
				SetText983(MakeStr("[%s] :: %d -> %d", GetStatName983(m_stat), before, after));
			}
		}
		break;
	case 2:
		SetText983(GetLuckyChanceText983());
		break;
	case 3:
		{
			IFF_STRUCT::sClubSet* pFrom = Doc()->m_itemManager.FindClubSet(typeIdA);
			IFF_STRUCT::sClubSet* pTo = Doc()->m_itemManager.FindClubSet(typeIdB);
			if (!pFrom || !pTo)
			{
				m_pClubBmp = NULL;
				m_pClubBmp2 = NULL;
				SetText983("");
				break;
			}
			m_pClubBmp = (Bitmap*)g_pFresh->GetBitmap(pFrom->c.Icon);
			m_pClubBmp2 = (Bitmap*)g_pFresh->GetBitmap(pTo->c.Icon);
			SetText983(pTo->c.Name);
			FrArea* pArea = m_pParent->GetResultCtrl983<FrArea>("area_progress_result_clubset");
			if (pArea)
				pArea->SetRect(WRect(m_clubChangePos[0] + m_pParent->GetFormRect983().x,
					m_clubChangePos[1] + m_pParent->GetFormRect983().y, pArea->GetRect().w, pArea->GetRect().h));
		}
		break;
	}
}

// 004a3170
bool FrClubSetWorkshopProgressWnd::Init983()
{
	AddCtrlByName983("area_progress_eclipse");
	AddCtrlByName983("area_progress_subwnd_bg");
	AddCtrlByName983("area_progress_animation");
	AddCtrlByName983("gauegebar_progress");
	AddCtrlByName983("btn_workshop_apply");
	AddCtrlByName983("btn_workshop_cancel");
	AddCtrlByName983("btn_workshop_ok");
	AddCtrlByName983("area_speakballoon_rankup");
	AddCtrlByName983("static_speakballoon_rankup");
	AddCtrlByName983("area_progress_result_title");
	AddCtrlByName983("area_progress_result_clubset");
	AddCtrlByName983("area_progress_result_level");
	AddCtrlByName983("area_progress_result_rank");
	AddCtrlByName983("edit_progress_result_text");
	m_pResult = new FrClubSetWorkshopProgressResultWnd(this);
	return true;
}

// 004a33f0: o servidor respondeu; botões e quadro do resultado pelo modo
void FrClubSetWorkshopProgressWnd::Update983(bool bSuccess)
{
	if (!m_pResult)
	{
		Finish983();
		return;
	}
	ShowCtrl983("btn_workshop_apply", bSuccess);
	EnableCtrl983("btn_workshop_apply", bSuccess);
	FrButton* pBtn = GetCtrl983<FrButton>("btn_workshop_cancel");
	if (pBtn)
	{
		pBtn->SetVisible(bSuccess);
		if (m_mode == MODE_LEVELUP)
		{
			pBtn->Enable(!m_bNoCancel);
			pBtn->SetToolTipText(m_bNoCancel ? K2L_Compatibility("\263\262\276\306\300\326\264\302 \260\263\301\266 \303\353\274\322 "
				"\260\241\264\311 \310\275\274\366\260\241 \276\370\275\300\264\317\264\331.") : "");
		}
		else
			pBtn->Enable(bSuccess);
	}
	ShowCtrl983("btn_workshop_ok", !bSuccess);
	EnableCtrl983("btn_workshop_ok", !bSuccess);
	SetOverlayUse983(m_bgOverlay, !bSuccess);
	switch (m_mode)
	{
	case MODE_LEVELUP:
		if (FrClubSetLevelUpWnd::s_pInstance && FrClubSetLevelUpWnd::s_pInstance->m_pInfoWnd)
		{
			FrClubSetLevelUpWnd::s_pInstance->m_pInfoWnd->Update983(true, -1);
			m_pResult->SetResult983(0, 0, 0);
			m_pResult->Begin983();
		}
		SetFx983(1);
		break;
	case MODE_RANKUP:
		if (FrClubSetRankUpWnd::s_pInstance && FrClubSetRankUpWnd::s_pInstance->m_pInfoWnd)
		{
			FrClubSetRankUpWnd::s_pInstance->m_pInfoWnd->Update983(false, -1);
			if (bSuccess)
			{
				m_pResult->SetResult983(2, 0, 0);
				ShowRankUpBalloon983(true);
				SetFx983(6);
			}
			else
			{
				m_pResult->SetResult983(1, 0, 0);
				SetFx983(1);
			}
			m_pResult->Begin983();
		}
		break;
	case MODE_RANKUP_RESULT:
		m_pResult->SetResult983(3, m_oldRank, m_newRank);
		m_pResult->Begin983();
		SetFx983(1);
		break;
	}
}

// 004a37e0
void FrClubSetWorkshopProgressWnd::SetResult983(unsigned long guid)
{
	m_resultGuid = guid;
	if (m_pResult && guid)
		m_pResult->StartMove983();
	else
		Finish983();
	if (m_pGauge)
	{
		m_pGauge->SetDestPos(0, false);
		m_pGauge->ResetGauge();
	}
}

// 004a3830
void FrClubSetWorkshopProgressWnd::SetWorking983(bool bWorking, bool bAnim)
{
	if (!bWorking)
	{
		Back983(false);
		return;
	}
	m_mode = MODE_RANKUP;
	m_bWorking = bWorking;
	if (bAnim)
		Update983(false);
}

// 004a3870
void FrClubSetWorkshopProgressWnd::Proc983(float deltaTime)
{
	if (m_bWorking)
	{
		if (m_pForm && m_pForm->IsViewFocused())
			g_input->SetFakeInput("ESCAPE", true);
		if (m_bRunning)
		{
			m_anim.Proc983(deltaTime, this);
			if (StepGauge983(&deltaTime))
			{
				Update983(m_bfa983);
				m_anim.SetUse983(false, this);
				m_bRunning = false;
			}
			else if (m_mode == MODE_LEVELUP)
			{
				SetFx983(0);
				ClubSetWorkshopFx983::HoldFx983(0);
			}
			else
			{
				SetFx983(3);
				ClubSetWorkshopFx983::HoldFx983(m_bfa983 ? 2 : 1);
			}
		}
		if (ClubSetWorkshopFx983::IsFxEnd983(m_mode != MODE_LEVELUP ? (m_bfa983 ? 2 : 1) : 0))
		{
			m_gaugeTime = 0;
			if (m_pGauge)
			{
				m_pGauge->SetDestPos(0, false);
				m_pGauge->ResetGauge();
			}
			else if (FrClubSetRankUpWnd::s_pInstance)
				FrClubSetRankUpWnd::s_pInstance->OnRankUpCancel983();
			Finish983();
		}
		m_pResult->Proc983(&deltaTime); // 983 004a398b: sem return, cai no bloco de baixo (Finish983 do STATE_END com m_bWorking)
	}
	if (m_pResult)
	{
		if (m_pResult->m_state == FrClubSetWorkshopProgressResultWnd::STATE_END)
		{
			if (m_mode == MODE_LEVELUP)
			{
				if (FrClubSetLevelUpWnd::s_pInstance)
					FrClubSetLevelUpWnd::s_pInstance->SelectClub983(m_resultGuid);
			}
			else if (FrClubSetRankUpWnd::s_pInstance)
				FrClubSetRankUpWnd::s_pInstance->OnRankUpCancel983();
			Finish983();
		}
		m_pResult->Proc983(&deltaTime);
	}
}
