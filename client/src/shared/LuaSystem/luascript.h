#pragma once

#include <string>
#include "lua_tinker/lua_tinker.h"

namespace lua_system
{
	class lua_script_manger;
	class lua_table_helper;

	class lua_script
	{
		friend class lua_script_manger;

	public:
		void WaitTimeScript(float seconds);
		void WaitFrameScript(int frames);
		void AbortWait();
		lua_table_helper GetTable(const char* name);
		int Update(float delta);
		bool RunFile(const char* name);
		bool RunString(const char* text);
		void Reload();

	protected:
		lua_script(lua_script_manger* owner, bool option);
		~lua_script();
		void ReserveDelete() { flag = true; }

	private:
		void FormatError();
		void OutputError(char* prefix);
		void ResumeScript(float value);

		lua_script_manger* manager;
		lua_State* thread;
		int reference;
		int state;
		float elapsed, wait_time;
		int wait_frames;
		char error[256];
		std::string filename, script;
		bool flag;
	};
}
