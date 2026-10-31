#include "luascriptmanager.h"
#include "luascript.h"
#include "luatablehelper.h"
#include "wresrcmng.h"
#include "cfile.h"
#include "../../projectg/console.h"
#include "minatl.h"
#include <algorithm>
extern "C"
{
#include "Lua/src/lualib.h"
}

namespace lua_system
{
	void LuaOpenUtilityLib(lua_State* state);
	void InitLuaSystem983();

	lua_script_manger* lua_script_manger::GetInstance()
	{
		static lua_script_manger manager;
		return &manager;
	}

	lua_script_manger::~lua_script_manger()
	{
		for (std::list<lua_script*>::iterator i = scripts.begin();
			i != scripts.end(); ++i)
		{
			delete *i;
			*i = 0;
		}
		scripts.clear();
		lua_close(master);
		if (fileCallbacks)
		{
			delete (char*)fileCallbacks;
			fileCallbacks = 0;
		}
	}

	// A220: erro Lua fora de pcall = luaD_throw -> exit(1) ("fecha com código 1" sem exception.log, ex.: a218-1 na aba GP).
	// Antes de sair, a mensagem e os endereços de retorno vão para o jp983_stubs.log (o robô traz); o 983 só escreve no stderr.
	static int LuaPanic983(lua_State* L)
	{
		void* frames[24];
		typedef WORD (WINAPI* CaptureFn)(DWORD, DWORD, void**, DWORD*); // o SDK do VS2010 daqui não declara
		CaptureFn capture = (CaptureFn)GetProcAddress(GetModuleHandleA("kernel32.dll"), "RtlCaptureStackBackTrace");
		int n = capture ? capture(1, 24, frames, NULL) : 0;
		char line[1024];
		int len = _snprintf(line, 512, "lua PANIC: %s | pilha:", lua_tostring(L, -1) ? lua_tostring(L, -1) : "?");
		for (int i = 0; i < n && len > 0 && len < (int)sizeof(line) - 12; i++)
			len += _snprintf(line + len, sizeof(line) - len, " %p", frames[i]);
		// A221: pilha Lua (tipo de cada posição; número com o valor) para achar o índice velho de um lua_tinker::table
		int top = lua_gettop(L);
		len += _snprintf(line + len, sizeof(line) - len, " | lua %d:", top);
		for (int i = 1; i <= top && len > 0 && len < (int)sizeof(line) - 24; i++)
			len += lua_type(L, i) == LUA_TNUMBER ? _snprintf(line + len, sizeof(line) - len, " %d=%g", i, lua_tonumber(L, i))
				: _snprintf(line + len, sizeof(line) - len, " %d=%s", i, lua_typename(L, lua_type(L, i)));
		lstrcatA(line, "\r\n");
		OutputDebugStringA(line);
		char path[MAX_PATH];
		DWORD k = GetModuleFileNameA(NULL, path, MAX_PATH);
		while (k && path[k - 1] != '\\')
			k--;
		lstrcpyA(path + k, "jp983_stubs.log");
		HANDLE h = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		if (h != INVALID_HANDLE_VALUE)
		{
			DWORD w;
			WriteFile(h, line, lstrlenA(line), &w, NULL);
			CloseHandle(h);
		}
		return 0;
	}

	bool lua_script_manger::Init(ErrorFn fn)
	{
		master = luaL_newstate();
		if (!master)
			return false;
		lua_atpanic(master, LuaPanic983);
		luaL_openlibs(master);
		LuaOpenUtilityLib(master);
		errorFn = fn;
		return true;
	}

	struct FileCallbacks983
	{
		void* loader;
		void* freer;
		FileCallbacks983() : loader(0), freer(0) {}
	};

	void lua_script_manger::SetFileCallbacks(void* loader, void* freer)
	{
		if (fileCallbacks)
		{
			delete (char*)fileCallbacks;
			fileCallbacks = 0;
		}
		FileCallbacks983* pair = new FileCallbacks983;
		fileCallbacks = pair;
		if (pair)
		{
			pair->loader = loader;
			pair->freer = freer;
		}
	}

	// 983 00b43240: le o arquivo inteiro para um buffer malloc terminado em 0
	static char* __fastcall LoadFileBuffer983(const char* name)
	{
		cFile* file = g_resrcmng->GetCFile(name, 65535);
		if (!file)
			return 0;
		int length = file->Length();
		char* buffer = (char*)malloc(length + 1);
		file->Read(buffer, length);
		CloseCFile(file);
		buffer[length] = 0;
		return buffer;
	}

	// 983 00b431b0
	static void __fastcall FreeFileBuffer983(void* buffer)
	{
		if (buffer)
			free(buffer);
	}

	// 983 00b43430 (chamada de CProjectG::Ready): estado Lua, tratador de erro = console e root.lua
	void InitLuaSystem983()
	{
		lua_script_manger::GetInstance()->Init(CConsole::Print);
		lua_script_manger::GetInstance()->SetFileCallbacks((void*)LoadFileBuffer983, (void*)FreeFileBuffer983);
		lua_script* script = lua_script_manger::GetInstance()->CreateScript(true);
		if (script)
			script->RunFile("root.lua");
	}

	lua_script* lua_script_manger::CreateScript(bool option)
	{
		if (!master)
			return 0;
		lua_script* value = new lua_script(this, option);
		if (!value)
			return 0;
		scripts.push_back(value);
		return value;
	}

	void lua_script_manger::ReleaseScript(lua_script* value)
	{
		if (value)
			value->ReserveDelete();
	}

	void lua_script_manger::DeleteScript(lua_script* value)
	{
		std::list<lua_script*>::iterator i;
		i = std::find(scripts.begin(), scripts.end(), value);
		if (i != scripts.end())
		{
			delete *i;
			*i = 0;
			scripts.erase(i);
		}
	}

	void lua_script_manger::ReloadAll()
	{
		for (std::list<lua_script*>::iterator i = scripts.begin();
			i != scripts.end(); ++i)
			(*i)->Reload();
	}

	void lua_script_manger::Update(float delta)
	{
		std::list<lua_script*>::iterator i = scripts.begin();
		while (i != scripts.end())
		{
			if ((*i)->Update(delta) == 4)
			{
				std::list<lua_script*>::iterator next = i;
				++next;
				lua_script* value = *i;
				std::list<lua_script*>::iterator found =
					std::find(scripts.begin(), scripts.end(), value);
				if (found != scripts.end())
				{
					delete *found;
					*found = 0;
					scripts.erase(found);
				}
				i = next;
			}
			else
				++i;
		}
	}

	bool lua_script_manger::RunString(const char* text)
	{
		if (!master)
			return false;
		return lua_tinker::dostring(master, text, 0);
	}
}
