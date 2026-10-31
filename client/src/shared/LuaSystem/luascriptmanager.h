#pragma once

#include <list>
#include "lua_tinker/lua_tinker.h"
#include "luatablehelper.h"
class LuaScriptManagerJp983; // B1458: Error983 (jp/pktviews983.cpp) le errorFn

namespace lua_system
{
	class lua_script;
	class lua_table_helper;
	class LuaScriptManagerJp983; // B1458: Error983 (jp/pktviews983.cpp) le errorFn

	class lua_script_manger
	{
	public:
		typedef void (__fastcall *ErrorFn)(const char* text);
		lua_script_manger() : master(0), fileCallbacks(0), errorFn(0) {}	// 983: inline no GetInstance
		~lua_script_manger();

		bool Init(ErrorFn errorFn);	// 983 00c1f760: cria o estado Lua; errorFn fica em +0x14
		void SetFileCallbacks(void* loader, void* freer);	// 983 00c1f410: par em +0x10

		static lua_script_manger* GetInstance();
		lua_script* CreateScript(bool option);
		void ReleaseScript(lua_script* value);
		void Update(float delta);
		void ReloadAll();
		bool RunString(const char* text);
		lua_table_helper GetTable(const char* name)	// 983: sem funcao propria (inline no chamador)
		{
			lua_tinker::table value = lua_tinker::get<lua_tinker::table>(master, name);
			return lua_table_helper(value);
		}
		lua_State* GetMasterLuaState() { return master; }
		void DeleteScript(lua_script* value);	// 983: publica (chamada de 005980d0 e 0059dbe0)
		friend class ::LuaScriptManagerJp983;
		friend class LuaScriptManagerJp983; // lua_system::, mesmo 00c1f480

	private:

		std::list<lua_script*> scripts;
		lua_State* master;	// +0xc
		void* fileCallbacks;	// +0x10: new[8] {loader, freer}
		ErrorFn errorFn;	// +0x14 (Error983 chama)
	};

	void InitLuaSystem983();	// 983 00b43430, chamada de CProjectG::Ready (00a69a04)
}
