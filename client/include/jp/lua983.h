#pragma once
// JP 983: leituras do Lua como o 983 as chama (cópia das visões de grandprixawarddlg.cpp / holeinonemissiondlg.cpp, para as
// unidades da oficina de tacos — sessão B76). Por nome fora de linha (00470f20 float, 00470e60 int, 00483880 texto); a tabela
// filha por nome (0049a4e0) volta como lua_table_helper de 8 bytes; por índice fora de linha (0049a000) ou embutido (GetInl983).
#include "../../shared/LuaSystem/luascriptmanager.h"
#include "../../shared/LuaSystem/luascript.h"
#include "../../shared/LuaSystem/luatablehelper.h"

struct LuaChild983 : lua_system::lua_table_helper
{
	LuaChild983(lua_tinker::table value) : lua_system::lua_table_helper(value) { } // devolvida pela GetChild983 (B89)
	float GetByIndex(int index); // 0049a000
	int GetIntByIndex983(int index); // 0049e3b0 (B80; o jp.tsv tinha o nome da cópia estática da shottrajectoryformassgame.cpp;
	                                 // corpo na clubsetworkshopprogresswnd.cpp, B86: ramo do nil primeiro)
	void Push983(int index)
	{
		// = 983 (copias de scoreboard.cpp/grandprixawarddlg.cpp, D760). O nil do gettable numerico (E146, e145-1) era o
		// lua_number2int da lib do rebang com a FPU em _PC_24, corrigido na causa pelo A222 (lua983_ltable/lua983_lapi)
		if (table.m_obj)
		{
			lua_pushnumber(state, (double)index);
			lua_gettable(state, table.m_obj->m_index);
			if (lua_type(state, -1) != LUA_TNIL)
			{
				lua_remove(state, -1);
				lua_rawgeti(state, table.m_obj->m_index, index);
				return;
			}
			lua_remove(state, -1);
		}
		lua_pushnil(state);
	}
	template <typename T>
	T GetInl983(int index)
	{
		Push983(index);
		return lua_tinker::pop<T>(state);
	}
};
struct LuaHelper983 : lua_system::lua_table_helper
{
	template <typename T>
	T Get(const char* name);
	LuaChild983 GetChild983(const char* name); // 0049a4e0
	template <typename T>
	T GetChecked983(const char* name); // 00483880 (<const char*>, B112): sem o campo, avisa e lê assim mesmo
};
// erro do gerenciador de scripts (00c1f480; sem declaração no luascriptmanager.h da frente A — empréstimo)
class LuaScriptManagerJp983
{
public:
	void Error983(const char* text);
};
template <typename T>
T LuaHelper983::GetChecked983(const char* name)
{
	if (table.m_obj)
	{
		lua_pushstring(state, name);
		lua_gettable(state, table.m_obj->m_index);
		if (lua_type(state, -1) == LUA_TNIL)
			lua_remove(state, -1);
		else
		{
			lua_remove(state, -1);
			goto found;
		}
	}
	char text[512];
	sprintf(text, "not found '%s' value.", name);
	((LuaScriptManagerJp983*)lua_system::lua_script_manger::GetInstance())->Error983(text);
found:
	return table.get<T>(name);
}
template <typename T>
T LuaHelper983::Get(const char* name)
{
	return table.get<T>(name);
}
#define LUA983_GET(tbl, type, name) ((LuaHelper983&)(tbl)).Get<type>(name)
#define LUA983_POS(tbl, name, index) ((LuaHelper983&)(tbl)).GetChild983(name).GetByIndex(index)
#define LUA983_POSI(tbl, name, index) ((LuaHelper983&)(tbl)).GetChild983(name).GetInl983<float>(index)
