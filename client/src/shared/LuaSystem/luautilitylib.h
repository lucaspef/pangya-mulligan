#pragma once

#include <string>
#include <vector>
#include <map>
#include "lua_tinker/lua_tinker.h"

namespace lua_system
{
	void LuaOpenUtilityLib(lua_State* state);

	class lua_file_line_text_manager
	{
	public:
		lua_file_line_text_manager() { }
		~lua_file_line_text_manager() { }

		static lua_file_line_text_manager* get_instance();
		const char* get_line_text(const char* file, int line);

	protected:
		bool is_loaded_lua_file(const char* file);
		void road_lua_file(const char* file);
		void road_lua_buffer(const char* file, const char* buffer);

	private:
		std::map<std::string, std::vector<std::string> > lines_;
	};
}
