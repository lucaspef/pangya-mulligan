// JP 983: unidade 0056efc0–00570660 (CHoleEffect, a animação do hole-in-one na tela de jogo; ver include/jp/holeeffect.h).
// Classe nova do 983 (existe no 839, não no 645). O vão 0056da5e–0056efc0 antes dela (câmera lenta perto do copo,
// "improve_ingame_play.lua") não tem chamador nem classe: fica para a F3.7.14. Sessão B119.
#include "minatl.h"
#include "projectg.h"
#include "vc10_layout.h"
#include "shareddoc.h"
#include "golfball.h"
#include "wview.h"
#include "woverlay.h"
#include "wresrcmng.h"
#include "lua983.h"
#include "holeeffect.h"
#include <stdio.h>
#include <string.h>

extern const WRect g_rectUnitW983; // 00e4296c (retângulo de origem padrão)

// JP 983: estado do jogo = CGolfDoc::m_gameMode (+0x158) de CSharedDoc::m_pGolfDoc (+0x11024); A746: membros (no VS 2022 o Doc anda)
#define GAMESTATE983() (::Doc()->m_pGolfDoc->m_gameMode)
C_ASSERT_VC10(offsetof(CSharedDoc, m_pGolfDoc) == 0x11024 && offsetof(CGolfDoc, m_gameMode) == 0x158);

// leitura inteira sem sinal por índice: o 983 chama fora de linha (004c0bb0, o GetInl983<unsigned long> do lua983.h; corpo
// em outra unidade, perto da realmyroommain)
struct LuaCell983 : LuaChild983
{
	LuaCell983(lua_tinker::table value) : LuaChild983(value) { }
	unsigned long GetULong983(int index); // 004c0bb0
};
// linha de uma tabela Lua por índice (004d5250, corpo na dummyplayeractor.cpp; aqui devolvida como LuaCell983)
struct LuaRow983 : lua_system::lua_table_helper
{
	LuaCell983 GetRow983(int index); // 004d5250
};
#define LUAROW983(tbl, index) (((LuaRow983&)(tbl)).GetRow983(index))
// 004d5250: empilha tbl[index] (ramo do nil -> nil, igual ao Push983 do 983) e devolve a tabela filha
LuaCell983 LuaRow983::GetRow983(int index)
{
	if (table.m_obj)
	{
		lua_pushnumber(state, (double)index);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) != LUA_TNIL)
		{
			lua_remove(state, -1);
			lua_rawgeti(state, table.m_obj->m_index, index);
			return LuaCell983(lua_tinker::pop<lua_tinker::table>(state));
		}
		lua_remove(state, -1);
	}
	lua_pushnil(state);
	return LuaCell983(lua_tinker::pop<lua_tinker::table>(state));
}
// 004c0bb0
unsigned long LuaCell983::GetULong983(int index)
{
	return GetInl983<unsigned long>(index);
}

// -1 na tabela = sem valor (0)
static inline float UNSET983(float v)
{
	return v != -1.0f ? v : 0.0f;
}

// 0056efc0
void CHoleEffect::ApplyEvent983(const sItem983& ev)
{
	sItem983& e = m_effects[ev.index];
	e.fromX = (ev.fromX == -1.0f) ? e.fromX : ev.fromX;
	e.fromY = (ev.fromY == -1.0f) ? e.fromY : ev.fromY;
	e.toX = (ev.toX == -1.0f) ? e.toX : ev.toX;
	e.toY = (ev.toY == -1.0f) ? e.toY : ev.toY;
	e.alpha = ev.alpha;
	e.scale = ev.scale;
	e.start = ev.start;
	e.duration = ev.duration;
	e.maxRot = ev.maxRot;
	e.flags |= ev.flags;
}

// 0056f080
void CHoleEffect::AddCount983(int count)
{
	m_count += count;
	sprintf(m_buffer, "+%d", m_count);
}

// 0056f260
void CHoleEffect::DrawItem983(WOverlay* overlay, const sItem983& item)
{
	WRect rc(item.x, item.y, overlay->GetWidth() * item.scale, overlay->GetHeight() * item.scale);
	overlay->Render(g_view, g_rectUnitW983, rc, 0, ((unsigned char)(int)item.alpha << 24) | 0xffffff, item.rot, 0);
}

// 0056f400
void CHoleEffect::Display()
{
	if (m_bPlaying && (GAMESTATE983() & ~0x80) == 0 && GolfBall().m_state == 0)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (m_overlays[i])
				DrawItem983(m_overlays[i], m_effects[i]);
		}
		m_color = ((int)m_effects[3].alpha << 24) | 0xffffff;
		Print(m_effects[3].x, m_effects[3].y, m_buffer, 4.0f, 0, m_effects[3].scale, NULL);
	}
}

// 0056f540
void CHoleEffect::Start983()
{
	if (!GolfBall().IsHoleIn())
	{
		m_bPlaying = 0;
		return;
	}
	m_bPlaying = 1;
	AddCount983(1);
}

// 0056f6d0: posição/tempos de uma figura em "holeineffectpos"; sem `usePos`, a figura com imagem fica no meio da tela
void LoadItem983(CHoleEffect::sItem983& e, lua_system::lua_table_helper& tbl, const char* name, WOverlay* overlay,
	int usePos)
{
	if (overlay && !usePos)
	{
		e.x = g_view->GetWidth() * 0.5 - overlay->GetWidth() * 0.5;
		e.y = g_view->GetHeight() * 0.5 - overlay->GetHeight() * 0.5;
	}
	else
	{
		float x = LUA983_POS(tbl, name, 1);
		float y = LUA983_POS(tbl, name, 2);
		e.x = UNSET983(x);
		e.y = UNSET983(y);
	}
	e.scale = LUA983_POS(tbl, name, 3);
	e.start = LUA983_POS(tbl, name, 4);
	e.duration = LUA983_POS(tbl, name, 5);
	e.alpha = LUA983_POS(tbl, name, 6);
	e.maxRot = LUA983_POS(tbl, name, 7);
	e.flags = ((LuaCell983&)((LuaHelper983&)tbl).GetChild983(name)).GetULong983(8);
}

// 0056fa80
void CHoleEffect::LoadOverlays983()
{
	memset(m_overlays, 0, sizeof(m_overlays));
	lua_system::lua_table_helper tbl = m_pScript->GetTable("holeineffectimg");
	m_overlays[0] = g_resrcmng->GetOverlay(((LuaHelper983&)tbl).GetChecked983<const char*>("ray"), 0);
	m_overlays[1] = g_resrcmng->GetOverlay(((LuaHelper983&)tbl).GetChecked983<const char*>("sun"), 0);
	m_overlays[2] = g_resrcmng->GetOverlay(((LuaHelper983&)tbl).GetChecked983<const char*>("ring"), 0);
	m_overlays[4] = g_resrcmng->GetOverlay(((LuaHelper983&)tbl).GetChecked983<const char*>("white"), 0);
	m_overlays[5] = g_resrcmng->GetOverlay(((LuaHelper983&)tbl).GetChecked983<const char*>("mini"), 0);
}

// 0056fc60: aplica e tira da lista os eventos cuja hora chegou
void CHoleEffect::ProcessEvents983(float dt)
{
	std::list<sItem983>::iterator it = m_events.begin();
	while (it != m_events.end())
	{
		if (it->start <= m_time)
		{
			ApplyEvent983(*it);
			it = m_events.erase(it);
		}
		else
			++it;
	}
}

// 0056fcd0
void CHoleEffect::Process(float dt)
{
	if (m_bPlaying && (GAMESTATE983() & ~0x80) == 0 && GolfBall().m_state == 0)
	{
		ProcessEvents983(dt);
		m_time += dt;
		if (m_time >= m_duration)
		{
			m_bPlaying = 0;
			m_time = m_duration;
		}
		for (int i = 0; i < 6; ++i)
		{
			sItem983& e = m_effects[i];
			if (m_time >= e.start && e.flags && e.duration + e.start >= m_time)
			{
				if (e.flags & 3)
				{
					if (e.flags & 2)
					{
						e.alpha -= m_fadeVel * dt;
						if (e.alpha < 0.0f)
						{
							e.alpha = 0.0f;
							e.flags &= ~2;
						}
					}
					else if (e.flags & 1)
					{
						e.alpha += m_fadeVel * dt;
						if (e.alpha > 255.0)
						{
							e.alpha = 255.0f;
							e.flags &= ~1;
						}
					}
				}
				if (e.flags & 8)
				{
					e.rot += m_rotVel * dt;
					if (e.rot >= e.maxRot)
						e.flags &= ~8;
				}
				if (e.flags & 4)
				{
					e.t += (e.duration - e.start) * dt;
					if (e.t > 1.0f)
						e.flags &= ~4;
					e.x = e.fromX * (1.0f - e.t) + e.toX * e.t;
					e.y = (1.0f - e.t) * e.fromY + e.toY * e.t;
				}
				if (e.flags & 0x20)
				{
					e.scale = 1.0 - e.t * 0.3;
					if (e.scale < 0.3f)
					{
						e.scale = 0.3f;
						e.flags &= ~0x20;
					}
				}
			}
		}
	}
}

// 0056ffe0
CHoleEffect::~CHoleEffect()
{
	lua_system::lua_script_manger::GetInstance()->ReleaseScript(m_pScript);
	for (int i = 0; i < 6; ++i)
	{
		if (g_resrcmng && m_overlays[i])
		{
			g_resrcmng->Release(m_overlays[i]);
			m_overlays[i] = NULL;
		}
	}
}

// 005700d0: um evento de "sceneevent" (linha `index`)
void LoadEvent983(std::list<CHoleEffect::sItem983>& events, lua_system::lua_table_helper& tbl, int index)
{
	CHoleEffect::sItem983 ev;
	memset(&ev, 0, sizeof(ev));
	ev.index = LUAROW983(tbl, index).GetIntByIndex983(1);
	ev.flags = LUAROW983(tbl, index).GetULong983(2);
	ev.start = LUAROW983(tbl, index).GetByIndex(3);
	ev.duration = LUAROW983(tbl, index).GetByIndex(4);
	ev.fromX = LUAROW983(tbl, index).GetByIndex(5);
	ev.fromY = LUAROW983(tbl, index).GetByIndex(6);
	ev.toX = LUAROW983(tbl, index).GetByIndex(7);
	ev.toY = LUAROW983(tbl, index).GetByIndex(8);
	ev.alpha = LUAROW983(tbl, index).GetByIndex(9);
	ev.scale = LUAROW983(tbl, index).GetByIndex(10);
	ev.maxRot = LUAROW983(tbl, index).GetByIndex(11);
	events.push_back(ev);
}

// 00570330
void CHoleEffect::Reset()
{
	lua_system::lua_table_helper tbl = m_pScript->GetTable("holeineffectpos");
	memset(m_effects, 0, sizeof(m_effects));
	LoadItem983(m_effects[0], tbl, "ray", m_overlays[0], 0);
	LoadItem983(m_effects[1], tbl, "sun", m_overlays[1], 0);
	LoadItem983(m_effects[2], tbl, "ring", m_overlays[2], 0);
	LoadItem983(m_effects[4], tbl, "white", m_overlays[4], 0);
	LoadItem983(m_effects[3], tbl, "point", m_overlays[3], 0);
	LoadItem983(m_effects[5], tbl, "mini", m_overlays[5], 1);
	m_duration = LUA983_GET(tbl, float, "sceneduration");
	m_time = 0.0f;
	m_fadeVel = LUA983_GET(tbl, int, "fadevel");
	m_rotVel = LUA983_GET(tbl, float, "rotvel");
	lua_system::lua_table_helper events = m_pScript->GetTable("sceneevent");
	int count = lua_tinker::call<int, lua_tinker::table>(events.state, "get_array_size", events.table);
	for (int i = 0; i < count; ++i)
		LoadEvent983(m_events, events, i + 1);
	m_count = 0;
	m_bPlaying = 0;
	m_effects[3].fromX = m_effects[3].x = m_effects[1].x + 82.0;
	m_effects[3].fromY = m_effects[3].y = m_effects[1].y + 101.0;
}

// 00570520
CHoleEffect::CHoleEffect()
{
	Init("font_holein_font.png", NULL);
	m_buffer = new char[32];
	m_pScript = lua_system::lua_script_manger::GetInstance()->CreateScript(false);
	if (m_pScript)
		m_pScript->RunFile("holeineffect.lua");
	LoadOverlays983();
	Reset();
	m_bPlaying = 0;
}
