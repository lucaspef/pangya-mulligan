#pragma once

#include <stdio.h>
#include <list>
#include "gatewayinterface.h"

enum eTS_Keyword
{
	TSK_NONE = -1,
	TSK_WAIT = 0,
	TSK_KEY_DOWN,
	TSK_KEY_DOWN_KEEP,
	TSK_KEY_RELEASE,
	TSK_MOUSE_CLICK,
	TSK_MOUSE_MOVE,
	TSK_MISSION,
	TSK_TIP,
	TSK_TIP_VARROW,
	TSK_TIP_HARROW,
	TSK_TIP_CUSTOM_ARROW,
	TSK_TIP_IMAGE,
	TSK_TARGET,
	TSK_SEND_MSG,
	TSK_CURSOR,
	TSK_BLOCK_INPUT,
	TSK_FREE_INPUT,
	TSK_PUSHMACRO,
	TSK_ONLY_KEY,
	TSK_WITH_MOUSE,
	TSK_ENABLE,
	TSK_DISABLE,
	TSK_NULL,
	TSK_VIEW_WIDTH,
	TSK_VIEW_HEIGHT,
	TSK_OPTION_BAR_Y,
	TSK_GETPOINT,
	TSK_CAST_INT,
	TSK_VAL_CUSTOM_ARROW,
	TSK_BAR_RIGHTMOST,
	TSK_BAR_CENTER,
	TSK_SKIP_COUNT,
	TSK_POINTNAME,
	TSK_TARGETNAME,
	TSK_GAME_TYPE,
	TSK_GAME_TYPE_TUTORIAL_BASIC,
	TSK_IF,
	TSK_ELSE,
	TSK_ENDIF,
	TSK_GREATER,
	TSK_LESS,
	TSK_EQUAL,
	TSK_NOT
};

enum eMacroType
{
	MACRO_NONE = -1,
	MACRO_WAIT = 0,
	MACRO_KEY_DOWN,
	MACRO_KEY_DOWN_KEEP,
	MACRO_KEY_RELEASE,
	MACRO_MOUSE_CLICK,
	MACRO_MOUSE_MOVE,
	MACRO_MISSION = 8,
	MACRO_TIP,
	MACRO_TIP_VARROW,
	MACRO_TIP_HARROW,
	MACRO_TIP_CUSTOM_ARROW,
	MACRO_TIP_IMAGE,
	MACRO_TARGET = 0x15,
	MACRO_SEND_MSG,
	MACRO_CURSOR,
	MACRO_BLOCK_INPUT,
	MACRO_FREE_INPUT
};

// HACK: declaration order controls the reverse emission order of the MAX_* COMDATs.
struct sTS_TaskMsg
{
	static const int MAX_SYMBOL_LEN = 32;

	taskMsg_t msg;
	char name[MAX_SYMBOL_LEN];
};

struct sTS_Keyword
{
	static const int MAX_SYMBOL_LEN = 32;

	eTS_Keyword keyword;
	char name[MAX_SYMBOL_LEN];
};

struct sTS_Error
{
	static const int MAX_ERROR_MSG_LEN = 128;

	int line;
	char msg[MAX_ERROR_MSG_LEN];
};

struct sTS_ScriptInfo
{
	char name[32];
};

struct sTS_Macro
{
	int type;
	char* pszArg;
	int param;
	int count;

	sTS_Macro();
	~sTS_Macro();
	sTS_Macro& operator=(sTS_Macro& rhs);
};

class ts_Parser
{
public:
	enum eFileType
	{
		FILE_NONE = 0,
		FILE_BINARY,
		FILE_TEXT
	};

	// HACK: these constants are emitted in reverse declaration order by MSVC 7.1.
	static const int MAX_FILE_LEN = 256;
	static const int MAX_LINE_LEN = 2048;
	static const int MAX_TOKEN_LEN = 64;
	static const int MAX_BUFFER_LEN = 64;

	ts_Parser();
	ts_Parser(const char* pszFile);
	virtual ~ts_Parser();

	bool PTS_OpenFile(const char* pszFile, eFileType type);
	bool PTS_CloseFile();
	bool PTS_LoadScriptInfo();
	bool PTS_LoadScript(const char* pszName);
	bool PTS_PopMacro(sTS_Macro* pMacro);
	taskMsg_t GetTaskMessage(const char* pszName) const;
	eTS_Keyword GetKeyword(const char* pszName) const;
	void ConvertMacroType(eTS_Keyword keyword, eMacroType* pType) const;
	int FindQuotationMarkBlock(const char* pszText, int& start, int& end,
		int nth, bool bInclude);

protected:
	bool IsOpenedFile() const;
	void ClearStringBuffer();
	bool Rewind();
	bool NextLine();
	bool FindToken(const char* pszToken);
	bool RemoveToken();
	const char* GetToken();
	const char* GetLine();
	void ClearSymbolTable();
	void ClearVariables();
	int InsertSymbolTable(const char* pszSymbol);
	bool PushMacro(const char* pszLine, sTS_Macro& macro);
	sTS_Macro* CommandAnalyst(const char* pszLine);

	bool m_bOpened;
	char m_szFileName[MAX_FILE_LEN];
	eFileType m_eFileType;
	FILE* m_pFile;
	char m_szLine[MAX_LINE_LEN];
	char m_szToken[MAX_TOKEN_LEN];
	char m_szBuffer[MAX_BUFFER_LEN];
	short m_wLineNo;
	short m_wErrorNo;
	std::list<sTS_Error*> m_errors;
	std::list<sTS_ScriptInfo*> m_scriptInfos;
	std::list<sTS_Macro*> m_macros;
	std::list<char*> m_symbols;
};

extern sTS_TaskMsg gTS_TaskMsgTable[];
extern sTS_Keyword gTS_KeywordTable[];

inline sTS_Macro::sTS_Macro()
	: type(-1), pszArg(NULL), param(0), count(0)
{
}

inline sTS_Macro::~sTS_Macro()
{
	if (pszArg != NULL)
	{
		delete pszArg;
		pszArg = NULL;
	}
}

inline sTS_Macro& sTS_Macro::operator=(sTS_Macro& rhs)
{
	type = rhs.type;
	if (pszArg != NULL)
	{
		delete pszArg;
		pszArg = NULL;
	}
	if (rhs.pszArg != NULL)
	{
		pszArg = new char[strlen(rhs.pszArg) + 1];
		strcpy(pszArg, rhs.pszArg);
	}
	param = rhs.param;
	count = rhs.count;
	return *this;
}

#include "ts_parser.inl"
