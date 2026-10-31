// JP 983: unidade 004bd8e0–004c0690 (FrClubSetInfoWnd, quadro de dados do taco nas páginas "subir nível" e "subir rank" da
// oficina de tacos). Classe do 839/983, sem par no 645; ver include/jp/clubsetinfownd.h. Sessão B84 (F3.7.9, 9º lote).
// O compilador gera: 004be8e0 (ajuste da ChildWndBase), 004bf940 (`delete`), 004be770/004be320 (construtor/destrutor do
// sItem983), 004bf710 (vector<FrStatic*>::operator=) e moldes do vector. 004c0680 é uma instância dobrada de outra classe.
#include "minatl.h"
#include "projectg.h"
#include "bitmap.h"
#include "fresh.h"
#include "frwnd.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frbutton.h"
#include "frstatic.h"
#include "commonutil.h"
#include "lobbyjp.h"
#include "lua983.h"
#include "clubsetworkshopform.h"
#include "clubsetworkshopchildwndbase.h"
#include "clubsetworkshopfx.h"
#include "clubsetinfownd.h"

#define FORMGDI983() (FRMGR_GDI983(FRESHMGR983()))

// 004bd8e0
bool FrClubSetInfoWnd::Init983()
{
	return true;
}

// 004bd8f0: sem taco escolhido, o "escolha" pisca a cada 0,5 s
void FrClubSetInfoWnd::Proc983(float deltaTime)
{
	if (m_mode == 0)
	{
		if (!m_pSlot || !m_pSlot->guid)
		{
			m_blinkTime += deltaTime;
			if (m_blinkTime > 0.5f)
			{
				SetOverlayUse983(m_items[ITEM_SELECT_TEXT].overlay, m_bBlink);
				m_blinkTime = 0;
				m_bBlink = !m_bBlink;
			}
		}
		else if (!m_bBlink)
		{
			m_bBlink = true;
			SetOverlayUse983(m_items[ITEM_SELECT_TEXT].overlay, true);
		}
	}
	else if (m_bBlink)
	{
		m_bBlink = false;
		SetOverlayUse983(m_items[ITEM_SELECT_TEXT].overlay, false);
	}
}

// 004bd9b0
void FrClubSetInfoWnd::SetShow983(bool bShow)
{
	ClubSetWorkshopChildWndBase::SetShow983(bShow);
}

// 004bd9c0
void FrClubSetInfoWnd::SetSelectBtn983(FrButton* pBtn)
{
	m_pSelectBtn = pBtn;
}

// 004bd9d0
void FrClubSetInfoWnd::UpdateMode983()
{
	if (m_pSelectBtn)
		m_pSelectBtn->Enable(m_mode == 0);
	SetOverlayUse983(m_items[ITEM_SELECT_TEXT].overlay, m_mode == 0);
	if (m_pSlot)
		SetOverlayUse983(m_items[ITEM_SRANK_BORDER].overlay, m_pSlot->rank == 5);
}

// 004bda30: rank (aba "subir rank") ou nível (aba "subir nível") atual; meio apagado enquanto há um atributo escolhido
void FrClubSetInfoWnd::UpdateRankLevel983()
{
	if (m_pSlot)
	{
		for (int i = 0; i < 6; i++)
		{
			SetOverlayUse983(m_items[ITEM_RANK + i].overlay, m_pSlot->rank == i);
			if (!m_bLevelUp && m_pSlot->rank == i)
				SetOverlayAlpha983(m_items[ITEM_RANK + i].overlay, m_selStat != -1 ? 0.5f : 1.0f);
		}
		for (int i = 0; i < 5; i++)
		{
			SetOverlayUse983(m_items[ITEM_LEVEL + i].overlay, m_pSlot->level >= i);
			SetOverlayUse983(m_items[ITEM_LEVEL_D + i].overlay, m_pSlot->level < i);
			if (m_bLevelUp && m_pSlot->level == i)
				SetOverlayAlpha983(m_items[ITEM_LEVEL + i].overlay, m_selStat != -1 ? 0.5f : 1.0f);
		}
	}
}

// 004bdb30
void FrClubSetInfoWnd::SetSelStat983(int stat)
{
	if (m_pSlot && stat != -1)
	{
		m_selStat = stat;
		if (m_bLevelUp)
			m_pSlot = ClubSetWorkshopFx983::GetSlotPreview983(0, stat, true);
		else
			m_pSlot = ClubSetWorkshopFx983::GetSlotPreview983(1, stat, false);
	}
}

// 004bdd50
void FrClubSetInfoWnd::Draw983()
{
	if (!m_bShow)
		return;
	for (unsigned int i = 0; i < m_frames.size(); i++)
		DrawFrame983(m_frames[i], (int)m_x, (int)m_y);
	for (int i = 0; i < ITEM_COUNT; i++)
	{
		if (i != ITEM_SRANK_BORDER)
			DrawOverlayByIndex983(m_items[i].overlay, (int)(m_x + m_items[i].x), (int)(m_items[i].y + m_y), 0);
	}
	FrGraphicInterface* pGDI = FORMGDI983();
	Bitmap* pBmp = m_pSlot ? m_pSlot->pBmp : NULL;
	if (pGDI && pBmp)
		pGDI->DrawTexture(pBmp, WRect((float)m_iconX, (float)m_iconY, (float)pBmp->Width(), (float)pBmp->Height()), 0xffffffff, 0);
	DrawOverlayByIndex983(m_items[ITEM_SRANK_BORDER].overlay, (int)(m_items[ITEM_SRANK_BORDER].x + m_x),
		(int)(m_items[ITEM_SRANK_BORDER].y + m_y), 0);
}

// 004bded0: atributo escolhido (letra em destaque + seta), seta do rank máximo, textos dos atributos e dos máximos
void FrClubSetInfoWnd::UpdateStats983()
{
	for (int i = 0; i < 5; i++)
	{
		if (m_mode == 1 && m_selStat != -1)
		{
			SetOverlayUse983(m_items[ITEM_LETTER_STRONG + i].overlay, i == m_selStat);
			SetItemPos983(ITEM_ARROW, (int)m_arrowPos[m_selStat][0], (int)m_arrowPos[m_selStat][1]);
			SetOverlayUse983(m_items[ITEM_ARROW].overlay, true);
		}
		else
		{
			SetOverlayUse983(m_items[ITEM_LETTER_STRONG + i].overlay, false);
			SetOverlayUse983(m_items[ITEM_ARROW].overlay, false);
		}
	}
	if (m_pSlot && m_pSlot->rank == 5)
	{
		int index = ClubSetWorkshopFx983::GetSlotField58_983(!m_bLevelUp);
		SetItemPos983(ITEM_ARROW + 1, (int)m_arrowPos[index][0], (int)m_arrowPos[index][1]);
		SetOverlayUse983(m_items[ITEM_ARROW + 1].overlay, true);
	}
	else
		SetOverlayUse983(m_items[ITEM_ARROW + 1].overlay, false);
	for (int i = 0; i < 5; i++)
	{
		if (m_statics[i + 1])
			m_statics[i + 1]->SetCaption(m_pSlot ? MakeStr("%02d", m_pSlot->GetStat983(i)) : "-");
		if (m_statics[i + 9])
			m_statics[i + 9]->SetCaption(m_pSlot ? MakeStr("%02d", m_pSlot->GetStatMax983(i, !m_bLevelUp)) : "-");
	}
}

// 004be350: posição dos textos (nome; text_pos0…7 = atributos, tipo, experiência, cancelamentos; máximos) somada ao canto do quadro
void FrClubSetInfoWnd::LoadLayoutTexts983(lua_system::lua_script* script)
{
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_info");
	if (FrStatic* pName = m_statics[0])
	{
		float x = LUA983_POS(tbl, "clubname_text_pos", 1) + m_x;
		float y = LUA983_POS(tbl, "clubname_text_pos", 2) + m_y;
		pName->MoveWindow(WPoint(x, y));
	}
	for (int i = 1; i < 9; i++)
	{
		if (FrStatic* p = m_statics[i])
		{
			float x = LUA983_POS(tbl, MakeStr("text_pos%d", i - 1), 1) + m_x;
			float y = LUA983_POS(tbl, MakeStr("text_pos%d", i - 1), 2) + m_y;
			p->MoveWindow(WPoint(x, y));
		}
	}
	for (int i = 9; i < 14; i++)
	{
		if (FrStatic* p = m_statics[i])
		{
			float x = m_x + LUA983_POSI(tbl, MakeStr("text_max_pos%d", i - 9), 1);
			float y = m_y + LUA983_POSI(tbl, MakeStr("text_max_pos%d", i - 9), 2);
			p->MoveWindow(WPoint(x, y));
		}
	}
}

// 004be7e0
FrClubSetInfoWnd::~FrClubSetInfoWnd()
{
}

// 004be8f0: fundos do nome e dos atributos, letras dos atributos (normal e em destaque) e as setas
void FrClubSetInfoWnd::LoadLayoutBg983(lua_system::lua_script* script)
{
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_info");
	const char* nameBg = LUA983_GET(tbl, const char*, "clubname_bg_filename");
	int x = ((LuaHelper983&)tbl).GetChild983("clubname_bg_pos").GetIntByIndex983(1);
	int y = ((LuaHelper983&)tbl).GetChild983("clubname_bg_pos").GetIntByIndex983(2);
	SetItem983(ITEM_CLUBNAME_BG, nameBg, x, y, true);
	const char* statBg = LUA983_GET(tbl, const char*, "stat_bg_filename");
	const char* typeBg = LUA983_GET(tbl, const char*, "clubtype_bg_filename");
	for (int i = 1; i < 9; i++)
	{
		const char* bg = i < 6 ? statBg : typeBg;
		int bx = ((LuaHelper983&)tbl).GetChild983(MakeStr("stat_bg_pos%d", i - 1)).GetInl983<int>(1);
		int by = ((LuaHelper983&)tbl).GetChild983(MakeStr("stat_bg_pos%d", i - 1)).GetInl983<int>(2);
		SetItem983(ITEM_STAT_BG + i - 1, bg, bx, by, true);
	}
	for (int i = 0; i < 5; i++)
	{
		const char* letter = LUA983_GET(tbl, const char*, MakeStr("letter_filename%d", i));
		int lx = ((LuaHelper983&)tbl).GetChild983(MakeStr("letter_pos%d", i)).GetInl983<int>(1);
		int ly = ((LuaHelper983&)tbl).GetChild983(MakeStr("letter_pos%d", i)).GetInl983<int>(2);
		SetItem983(ITEM_LETTER + i, letter, lx, ly, true);
		SetItem983(ITEM_LETTER_STRONG + i, LUA983_GET(tbl, const char*, MakeStr("letter_strong_filename%d", i)), lx, ly, false);
	}
	for (int i = 5; i < 8; i++)
	{
		const char* letter = LUA983_GET(tbl, const char*, MakeStr("letter_filename%d", i));
		int lx = ((LuaHelper983&)tbl).GetChild983(MakeStr("letter_pos%d", i)).GetInl983<int>(1);
		int ly = ((LuaHelper983&)tbl).GetChild983(MakeStr("letter_pos%d", i)).GetInl983<int>(2);
		SetItem983(ITEM_LETTER + i, letter, lx, ly, true);
	}
	const char* arrow = LUA983_GET(tbl, const char*, "arrow_icon_filename");
	m_items[ITEM_ARROW].name = arrow;
	m_items[ITEM_ARROW].overlay = LoadOverlay983(arrow, false);
	m_items[ITEM_ARROW + 1].name = arrow;
	m_items[ITEM_ARROW + 1].overlay = LoadOverlay983(arrow, false);
	for (int i = 0; i < 5; i++)
	{
		m_arrowPos[i][0] = (float)((LuaHelper983&)tbl).GetChild983(MakeStr("arrow_icon_pos%d", i)).GetInl983<int>(1);
		m_arrowPos[i][1] = (float)((LuaHelper983&)tbl).GetChild983(MakeStr("arrow_icon_pos%d", i)).GetInl983<int>(2);
	}
}

// 004bf240: figuras do botão de escolher o taco, posição do botão e da figura do taco, borda do rank S, "escolha"
void FrClubSetInfoWnd::LoadLayoutSelect983(lua_system::lua_script* script)
{
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_info");
	if (m_pSelectBtn)
	{
		const char* selN = LUA983_GET(tbl, const char*, "clubset_select_n_filename");
		int x = ((LuaHelper983&)tbl).GetChild983("clubset_select_pos").GetIntByIndex983(1);
		int y = ((LuaHelper983&)tbl).GetChild983("clubset_select_pos").GetIntByIndex983(2);
		SetItem983(ITEM_SELECT_N, selN, x, y, true);
		SetItem983(ITEM_SELECT_O, LUA983_GET(tbl, const char*, "clubset_select_o_filename"), x, y - 20, false);
		int bx = (int)(((LuaHelper983&)tbl).GetChild983("clubset_btn_pos").GetIntByIndex983(1) + m_x);
		int by = (int)(((LuaHelper983&)tbl).GetChild983("clubset_btn_pos").GetIntByIndex983(2) + m_y);
		m_pSelectBtn->SetRect(WRect((float)bx, (float)by, m_pSelectBtn->GetRect().w, m_pSelectBtn->GetRect().h));
		const char* border = LUA983_GET(mainTbl, const char*, "img_srank_clubselect_border_filename");
		x = ((LuaHelper983&)tbl).GetChild983("img_srank_clubselect_border_pos").GetIntByIndex983(1);
		y = ((LuaHelper983&)tbl).GetChild983("img_srank_clubselect_border_pos").GetIntByIndex983(2);
		SetItem983(ITEM_SRANK_BORDER, border, x, y, false);
		m_iconX = (int)(((LuaHelper983&)tbl).GetChild983("clubset_icon_pos").GetIntByIndex983(1) + m_x);
		m_iconY = (int)(((LuaHelper983&)tbl).GetChild983("clubset_icon_pos").GetIntByIndex983(2) + m_y);
		const char* text = LUA983_GET(tbl, const char*, "clubset_select_text_filename");
		x = ((LuaHelper983&)tbl).GetChild983("clubset_select_text_pos").GetIntByIndex983(1);
		y = ((LuaHelper983&)tbl).GetChild983("clubset_select_text_pos").GetIntByIndex983(2);
		SetItem983(ITEM_SELECT_TEXT, text, x, y, true);
	}
}

// 004bf850
FrClubSetInfoWnd::FrClubSetInfoWnd(const std::vector<FrStatic*>& statics, ClubSetWorkShopForm* pForm)
{
	m_x = 0;
	m_y = 0;
	m_pSlot = NULL;
	m_pForm = pForm;
	m_statics = statics;
	m_blinkTime = 0;
	m_pSelectBtn = NULL;
	m_iconX = 0;
	m_iconY = 0;
	m_mode = 0;
	m_bBlink = false;
}

// 004bf970: moldura do quadro, ícones de rank (e…s) e de nível (ligado/apagado)
void FrClubSetInfoWnd::LoadLayoutLevel983(lua_system::lua_script* script)
{
	lua_system::lua_table_helper mainTbl = script->GetTable("clubset_workshop_ui_main");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_info");
	int x = ((LuaHelper983&)tbl).GetChild983("levelInfo_bg_pos").GetIntByIndex983(1);
	int y = ((LuaHelper983&)tbl).GetChild983("levelInfo_bg_pos").GetIntByIndex983(2);
	int frame = LoadFrame983(LUA983_GET(tbl, const char*, "levelInfo_bg_left"), LUA983_GET(tbl, const char*, "levelInfo_bg_mid"),
		LUA983_GET(tbl, const char*, "levelInfo_bg_right"), LUA983_GET(tbl, int, "levelInfo_bg_width"), x, y);
	m_frames.push_back(frame);
	int rx = ((LuaHelper983&)tbl).GetChild983("rank_icon_pos").GetIntByIndex983(1);
	int ry = ((LuaHelper983&)tbl).GetChild983("rank_icon_pos").GetIntByIndex983(2);
	m_items[ITEM_RANK + 0].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_e"));
	m_items[ITEM_RANK + 1].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_d"));
	m_items[ITEM_RANK + 2].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_c"));
	m_items[ITEM_RANK + 3].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_b"));
	m_items[ITEM_RANK + 4].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_a"));
	m_items[ITEM_RANK + 5].name = MakeStr("%s.tga", LUA983_GET(mainTbl, const char*, "rank_icon_s"));
	for (int i = 0; i < 6; i++)
	{
		sItem983& item = m_items[ITEM_RANK + i];
		const char* name = item.name.c_str();
		item.x = rx;
		item.y = ry;
		item.overlay = LoadOverlay983(name, false);
	}
	const char* fmt = LUA983_GET(tbl, const char*, "level_icon_n");
	int lx = ((LuaHelper983&)tbl).GetChild983("level_icon_pos").GetInl983<int>(1);
	int ly = ((LuaHelper983&)tbl).GetChild983("level_icon_pos").GetInl983<int>(2);
	int gap = LUA983_GET(tbl, int, "level_icon_gap");
	for (int i = ITEM_LEVEL; i < ITEM_LEVEL + 5; i++)
	{
		SetItem983(i, MakeStr(fmt, i - ITEM_LEVEL + 1), lx, ly, false);
		lx += gap;
	}
	fmt = LUA983_GET(tbl, const char*, "level_icon_d");
	lx = ((LuaHelper983&)tbl).GetChild983("level_icon_pos").GetInl983<int>(1);
	for (int i = ITEM_LEVEL_D; i < ITEM_LEVEL_D + 5; i++)
	{
		SetItem983(i, MakeStr(fmt, i - ITEM_LEVEL_D + 1), lx, ly, true);
		lx += gap;
	}
}

// 004c0070
void FrClubSetInfoWnd::LoadLayout983(int)
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("clubset_workshop.lua");
	lua_system::lua_table_helper tbl = script->GetTable("clubset_workshop_ui_info");
	int frame = LoadFrame983(LUA983_GET(tbl, const char*, "bg_img_left"), LUA983_GET(tbl, const char*, "bg_img_mid"),
		LUA983_GET(tbl, const char*, "bg_img_right"), LUA983_GET(tbl, int, "bg_img_width"), 0, 0);
	m_frames.push_back(frame);
	LoadLayoutBg983(script);
	LoadLayoutLevel983(script);
	LoadLayoutSelect983(script);
	LoadLayoutTexts983(script);
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script); // D608: 983 c1f510 (adiado), nao c1fe30
}

// 004c0210: dica do botão de escolher = nome do taco (vazia enquanto trabalha)
void FrClubSetInfoWnd::UpdateSelectBtnTip983()
{
	if (!m_pSlot)
		return;
	FrWnd* pBtn = FindCtrl983(m_bLevelUp ? "btn_clubselect_levelup" : "btn_clubselect_rankup");
	if (!pBtn)
		return;
	std::string tip = (m_pSlot && m_mode == 0) ? m_pSlot->name.c_str() : "";
	pBtn->SetToolTipText(tip.c_str());
}

// 004c03b0
void FrClubSetInfoWnd::SetMode983(int mode)
{
	m_mode = mode;
	UpdateSelectBtnTip983();
	UpdateMode983();
}

// 004c03d0: textos do taco (nome, tipo, experiência, cancelamentos) e as figuras
void FrClubSetInfoWnd::Update983(bool bLevelUp, int)
{
	if (!m_pSlot)
		m_pSlot = ClubSetWorkshopFx983::GetSlot983(!bLevelUp);
	std::string name("-");
	std::string type("-");
	m_bLevelUp = bLevelUp;
	name = m_pSlot ? m_pSlot->name.c_str() : "-";
	type = (m_pSlot && m_pSlot->clubType != -1) ? ClubSetTypeName983(m_pSlot->clubType) : "-";
	if (FrStatic* pExp = m_statics[7])
		pExp->SetCaption(m_pSlot ? MakeStr("%03d", m_pSlot->exp) : "-");
	if (FrStatic* pCancel = m_statics[8])
		pCancel->SetCaption(m_pSlot ? MakeStr("%d / %d", m_pSlot->recoveryLeft, m_pSlot->recoveryMax) : "-");
	if (FrStatic* pName = m_statics[0])
		pName->SetCaption(MakeStr("%s", name.c_str()));
	if (FrStatic* pType = m_statics[6])
		pType->SetCaption(MakeStr("%s", type.c_str()));
	UpdateSelectBtnTip983();
	if (m_pSlot && m_pSlot->pBmp)
		SetOverlayUse983(m_items[ITEM_SELECT_O].overlay, true);
	UpdateStats983();
	UpdateRankLevel983();
}

// 004c0620
void FrClubSetInfoWnd::Reset983(bool bLevelUp)
{
	m_pSlot = NULL;
	m_selStat = -1;
	Update983(bLevelUp);
	UpdateStats983();
	UpdateRankLevel983();
	UpdateMode983();
}
