// JP 983: unidade 0057e8f0–00580330 (_gateway::FrHoleinoneMissionDlg). Fonte reconstruída do 983 (classe nova, sem par no 645).
// Sessão B26: construtor, destrutor, registro, mapa de mensagens ("clicklist"), OnInit, OnProc, OnDraw e os auxiliares. O resto
// da faixa são instâncias da STL (vector de linhas de 0x30).
#include "minatl.h"
#include "projectg.h"
#include "holeinonemissiondlg.h"
#include "holeinonegamedoc.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "vc10_layout.h"
#include "golfball.h"
#include "fontinfo.h"
#include "icontentsdoc.h"
#include "fresh.h"
#include "frwndmanager.h"
#include "frgraphicinterface.h"
#include "frlistbox.h"
#include "bitmap.h"
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"
#include <stdio.h>

extern Fresh* g_pFresh;

namespace _gateway
{
// JP 983: estado do jogo = CGolfDoc::m_gameMode (+0x158) de CSharedDoc::m_pGolfDoc (+0x11024); A739: membros (no VS 2022 o Doc anda)
#define GAMESTATE983() (::Doc()->m_pGolfDoc->m_gameMode)
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && offsetof(CGolfDoc, m_gameMode) == 0x158);
// nome da missão: texto em +0x18 do registro da tabela de itens (include/jp/holeinonegamedoc.h só nomeia +0x04, +0x14 e +0x9c)

// leitura por índice de uma tabela filha com resultado float (0049a000 no 983; o lua_tinker da frente A só lê por nome)
struct LuaTable983 : lua_tinker::table
{
	float GetByIndex(int index);
};
inline LuaTable983& AsTable983(const lua_tinker::table& tbl) { return *(LuaTable983*)&tbl; }
#define LUA983_POS(tbl, name, index) AsTable983((tbl).GetValueByName<lua_tinker::table>(name)).GetByIndex(index)

// No 983 a leitura por nome (lua_table_helper::GetValueByName<T>) é chamada por `call` (00483880 texto, 00470f20 float); a do
// fonte da frente A é definida dentro da classe e fica embutida. Mesmo corpo, definido fora da classe (pedido B → A da B26).
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
};
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)

IMPLEMENT_OBJECT(FrHoleinoneMissionDlg, FrForm)

BEGIN_FRESH_MSGMAP(FrHoleinoneMissionDlg, FrForm)

ON_FRESH_VI("clicklist", FRCMD_INIT, FrHoleinoneMissionDlg::OnClickListInit)
ON_FRESH_VV("clicklist", FRCMD_LBUTTONUP, FrHoleinoneMissionDlg::OnClickList)

END_FRESH_MSGMAP()

void FrHoleinoneMissionDlg::UpdateSlide983(sSlide983* pSlide, float deltaTime)
{
	float time = (float)(deltaTime * 1.5 + pSlide->m_time);
	pSlide->m_time = time;
	if (m_movingTime < time)
	{
		pSlide->m_state = 0;
		pSlide->m_time = m_movingTime;
		pSlide->m_fromX = pSlide->m_curX;
		pSlide->m_fromY = pSlide->m_curY;
	}
	pSlide->m_curX = (m_movingTime - pSlide->m_time) * pSlide->m_fromX + pSlide->m_toX * pSlide->m_time;
	pSlide->m_curY = (m_movingTime - pSlide->m_time) * pSlide->m_fromY + pSlide->m_toY * pSlide->m_time;
}

void FrHoleinoneMissionDlg::DrawBg983()
{
	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	float alpha = m_wndAlpha;
	float width = (float)(m_textWidth + 8.0);
	float x = m_bgPos[0].x + m_rect.x;
	WRect rc(x, m_bgPos[0].y + m_rect.y, width, (float)m_pBg[0]->Height());
	pGDI->DrawTexture(m_pBg[0], rc, ((unsigned long)(alpha * 255.0) << 24) | 0xffffff, 0);

	WRect rc2(width + x, m_bgPos[1].y + m_rect.y, (float)m_pBg[1]->Width(), (float)m_pBg[1]->Height());
	pGDI->DrawTexture(m_pBg[1], rc2, ((unsigned long)(alpha * 255.0) << 24) | 0xffffff, 0);
}

void FrHoleinoneMissionDlg::StartSlide983(bool bShow)
{
	m_slide.m_fromX = m_rect.x;
	m_slide.m_fromY = 135.0f;
	m_slide.m_toY = 135.0f;
	m_slide.m_curX = 0.0f;
	m_slide.m_curY = 0.0f;
	m_slide.m_time = 0.0f;
	m_movingTime = 1.0f;
	if (bShow)
	{
		m_slide.m_state = 2;
		m_slide.m_toX = 0.0f;
	}
	else
	{
		m_slide.m_toX = m_hideOffsetX - m_rect.w;
		m_slide.m_state = 3;
	}
}

void FrHoleinoneMissionDlg::Show983()
{
	WPoint pt(0.0f, 135.0f);
	MoveWindow(pt);
	m_slide.m_state = 1;
}

void FrHoleinoneMissionDlg::InitSlide983(sSlide983* pSlide, int a, int index, int b)
{
	pSlide->m_fromX = 0.0f;
	pSlide->m_fromY = m_rect.y;
	pSlide->m_toX = 0.0f;
	pSlide->m_toY = m_rect.y;
	pSlide->m_state = 0;
	pSlide->m_alpha = 0.0f;
	pSlide->m_time = 0.0f;
	pSlide->m_curX = 0.0f;
	pSlide->m_curY = 0.0f;
	memset(pSlide->m_text, 0, sizeof(pSlide->m_text));
}

void FrHoleinoneMissionDlg::OnClickListInit(int param)
{
	m_pList = DYNAMIC_CAST(FrListBox, param);
	m_pList->SetPushSound("ui_desktop_icon_click");
	m_pList->AddItem(NULL);
}

void FrHoleinoneMissionDlg::OnProc(const float deltaTime)
{
	switch (m_slide.m_state)
	{
	case 0:
		break;
	case 1:
		m_slide.m_alpha += deltaTime;
		if (m_slide.m_alpha > 1.0f)
		{
			m_slide.m_state = 0;
			m_slide.m_alpha = 1.0f;
			StartSlide983(false);
			m_wndAlpha = m_slide.m_alpha;
			m_bShown = false;
		}
		else
		{
			m_wndAlpha = m_slide.m_alpha;
		}
		return;
	default:
	{
		UpdateSlide983(&m_slide, deltaTime);
		WPoint pt(m_slide.m_curX, m_slide.m_curY);
		MoveWindow(pt);
		break;
	}
	}

	if (m_prevState != GAMESTATE983())
	{
		if ((m_prevState & 0xffffffc7) && (GAMESTATE983() & 8))
		{
			if (m_bShown)
				StartSlide983(true);
		}
		else if ((GAMESTATE983() & 0xffffffc5) && (m_prevState & 0x2a))
		{
			StartSlide983(false);
		}
		m_prevState = GAMESTATE983();
	}

	if (!(GAMESTATE983() & 0xffffff7f))
	{
		if (GolfBall().m_state == 0)
		{
			int count = m_rows.size();
			for (unsigned int i = 0; i < (unsigned int)count; ++i)
			{
				sSlide983& slide = m_rows[i].m_slide;
				if (slide.m_state & 4)
				{
					if (slide.m_state & 2)
					{
						slide.m_alpha = deltaTime + slide.m_alpha;
						if (slide.m_alpha > 1.0f)
						{
							slide.m_alpha = 1.0f;
							slide.m_state = (slide.m_state & ~2) | 1;
						}
					}
					else if (slide.m_state & 1)
					{
						slide.m_alpha -= deltaTime;
						if (slide.m_alpha < 0.0f)
						{
							slide.m_alpha = 0.0f;
							slide.m_state = 0;
						}
					}
				}
			}
		}
	}
}

void FrHoleinoneMissionDlg::OnDraw()
{
	DrawBg983();

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	float alpha = Alpha983();
	unsigned int count = m_pMissions->size();
	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned long textColor = (unsigned long)(alpha * 255.0) << 24;
		unsigned long color = textColor | 0xffffff;
		float x = m_bgPos[2].x + m_rect.x;
		float y = (float)i * m_bgPos[2].y + m_rect.y + (float)(m_pBg[2]->Height() * i) + m_innerOffsetY;
		WRect rc(x, y, (float)(m_textWidth + 8.0), (float)m_pBg[2]->Height());
		pGDI->DrawTexture(m_pBg[2], rc, color, 0);

		IFF983::sCupInCondition* pItem = m_pMissions->at(i);
		pGDI->SetTextStyle(1);
		pGDI->SetTextColor(textColor, 0xffffffff);
		WPoint pt((float)(m_bgPos[2].x + m_rect.x + 3.0), y + m_innerOffsetY);
		pGDI->PrintText(pt, 0, pItem->Name, NULL);
		pGDI->SetTextStyle(0);
		pGDI->SetTextColor(color, 0xffffffff);

		x = (float)(x + m_textWidth + 9.0);
		unsigned long markColor = ((unsigned long)(m_rows[i].m_slide.m_alpha * 255.0) << 24) | 0xffffff;
		WRect rc2(x, y, (float)m_pBg[3]->Width(), (float)m_pBg[3]->Height() + m_bgPos[2].y);
		pGDI->DrawTexture(m_pBg[3], rc2, markColor, 0);

		y += m_innerOffsetY;
		m_pFont->SetColor(color);
		m_pFont->Print((float)(x + 1.0), y, m_rows[i].m_slide.m_text, 2.0f, 0, 0.5f, NULL);
	}
}

void FrHoleinoneMissionDlg::SetCleared983(const std::vector<unsigned long>& ids)
{
	unsigned int rowCount = m_rows.size();
	unsigned int idCount = ids.size();
	for (unsigned int i = 0; i < idCount; ++i)
	{
		for (unsigned int j = 0; j < rowCount; ++j)
		{
			if (ids.at(i) == m_rows[j].m_pItem->id)
			{
				m_rows[j].m_slide.m_state = 6;
				break;
			}
		}
	}
}

void FrHoleinoneMissionDlg::OnClickList()
{
	if (GAMESTATE983() & 0x200)
		return;
	m_bShown = !m_bShown;
	if (m_bShown)
		StartSlide983(true);
	else
		StartSlide983(false);
}

FrHoleinoneMissionDlg::FrHoleinoneMissionDlg()
{
	memset(m_pBg, 0, sizeof(m_pBg));
	memset(m_bgPos, 0, sizeof(m_bgPos));
	m_movingTime = 0.0f;
	m_innerOffsetY = 0.0f;
	m_hideOffsetX = 0.0f;
	m_textWidth = 0.0f;
	m_pFont = NULL;
	m_prevState = 0;
	memset(&m_slide, 0, sizeof(m_slide));
	m_188 = 0;
	m_pList = NULL;
	m_bShown = true;
}

FrHoleinoneMissionDlg::~FrHoleinoneMissionDlg()
{
	if (m_pFont)
	{
		delete m_pFont;
		m_pFont = NULL;
	}
}

bool FrHoleinoneMissionDlg::OnInit()
{
	lua_system::lua_script* script = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (script)
		script->RunFile("fr_holeinonemission_ui.lua");
	lua_system::lua_table_helper tbl = script->GetTable("holeinmode_mission_layout");
	m_pBg[0] = g_pFresh->RegisterBitmap(LUA983_GET(tbl, const char*, "bg_img1"));
	m_pBg[1] = g_pFresh->RegisterBitmap(LUA983_GET(tbl, const char*, "bg_img2"));
	m_pBg[2] = g_pFresh->RegisterBitmap(LUA983_GET(tbl, const char*, "bg_img3"));
	m_pBg[3] = g_pFresh->RegisterBitmap(LUA983_GET(tbl, const char*, "bg_img5"));
	m_bgPos[0].x = LUA983_POS(tbl, "bg_img1_pos", 1);
	m_bgPos[0].y = LUA983_POS(tbl, "bg_img1_pos", 2);
	m_bgPos[1].x = LUA983_POS(tbl, "bg_img2_pos", 1);
	m_bgPos[1].y = LUA983_POS(tbl, "bg_img2_pos", 2);
	m_bgPos[2].x = LUA983_POS(tbl, "bg_img3_pos", 1);
	m_bgPos[2].y = LUA983_POS(tbl, "bg_img3_pos", 2);
	m_movingTime = LUA983_GET(tbl, float, "moving_time");
	m_innerOffsetY = LUA983_GET(tbl, float, "inner_offsety");
	m_hideOffsetX = LUA983_GET(tbl, float, "hide_offsetx");
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(script);

	_golfgame::CHoleInOneGameDoc* pDoc = NULL;
	CContentsDoc::Instance()->GetContainer((localContentType_t)0xaa, pDoc);
	m_pMissions = &pDoc->m_items;
	for (unsigned int i = 0; i < m_pMissions->size(); ++i)
	{
		sRow983 row;
		InitSlide983(&row.m_slide, 0, i, 1);
		row.m_pItem = m_pMissions->at(i);
		sprintf_s(row.m_slide.m_text, sizeof(row.m_slide.m_text), "%%%d", row.m_pItem->param14);
		m_rows.push_back(row);
	}

	FrGraphicInterface* pGDI = g_pFresh->GetManager()->GetGDI();
	float maxWidth = 0.0f;
	unsigned int count = m_rows.size();
	for (unsigned int i = 0; i < count; ++i)
	{
		float width = pGDI->GetTextExtendOld983(m_rows[i].m_pItem->Name);
		maxWidth = (width > maxWidth) ? width : maxWidth;
	}
	m_textWidth = maxWidth;

	float x = m_rect.x;
	float y = m_rect.y;
	float w = (float)m_pBg[1]->Width() + maxWidth;
	float h = m_rect.h;
	WRect rc(x, y, w, h);
	SetRect(rc);
	m_pList->SetItemSize((int)((float)m_pBg[1]->Width() * 2.0f + m_textWidth), (int)m_rect.h);

	m_pFont = new CFontInfo;
	m_pFont->Init("[bigfont.jpg", NULL);
	return FrForm::OnInit();
}
}
