#include "minatl.h"
#include "projectg.h"
#include "ts_parser.h"
#include "clientsetting.h"
#include "shareddoc.h"
#include "golfdoc.h"
#include "wview.h"
#include "msg983.h"



// HACK: retain dynamic aggregate initialization of the fixed name arrays.
// The identity is inlined; dllimport suppresses an unused out-of-line helper.
__declspec(dllimport) __forceinline int RbTsInitValue(int value)
{
	return value;
}

sTS_TaskMsg gTS_TaskMsgTable[] = {
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x1d7)), "PM_RUN"               },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x1bc)), "PM_SET_TUTORIAL_UI"   },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)0x140), "PM_LOOKUP"            },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x87)),  "PM_SET_MACRO_SHOT"    },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x88)),  "PM_WAIT"              },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x8a)),  "PM_GET_CUSTOM_ARROW"  },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x1be)), "PM_FLICKER_HOLEINFO"  },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x1bd)), "PM_FLICKER_MINISCORE" },
	{ (taskMsg_t)RbTsInitValue((taskMsg_t)M983(0x8f)),  "PM_RETURN_LOBBY"      }
};

sTS_Keyword gTS_KeywordTable[] = {
	{ (eTS_Keyword)RbTsInitValue(TSK_WAIT),                     "TS_WAIT"             },
	{ (eTS_Keyword)RbTsInitValue(TSK_KEY_DOWN),                 "TS_KEY_DOWN"         },
	{ (eTS_Keyword)RbTsInitValue(TSK_KEY_DOWN_KEEP),            "TS_KEY_DOWN_KEEP"    },
	{ (eTS_Keyword)RbTsInitValue(TSK_KEY_RELEASE),              "TS_KEY_RELEASE"      },
	{ (eTS_Keyword)RbTsInitValue(TSK_MOUSE_CLICK),              "TS_MOUSE_CLICK"      },
	{ (eTS_Keyword)RbTsInitValue(TSK_MOUSE_MOVE),               "TS_MOUSE_MOVE"       },
	{ (eTS_Keyword)RbTsInitValue(TSK_MISSION),                  "TS_MISSION"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_TIP),                      "TS_TIP"              },
	{ (eTS_Keyword)RbTsInitValue(TSK_TIP_VARROW),               "TS_TIP_VARROW"       },
	{ (eTS_Keyword)RbTsInitValue(TSK_TIP_HARROW),               "TS_TIP_HARROW"       },
	{ (eTS_Keyword)RbTsInitValue(TSK_TIP_CUSTOM_ARROW),         "TS_TIP_CUSTOM_ARROW" },
	{ (eTS_Keyword)RbTsInitValue(TSK_TIP_IMAGE),                "TS_TIP_IMAGE"        },
	{ (eTS_Keyword)RbTsInitValue(TSK_TARGET),                   "TS_TARGET"           },
	{ (eTS_Keyword)RbTsInitValue(TSK_SEND_MSG),                 "TS_SEND_MSG"         },
	{ (eTS_Keyword)RbTsInitValue(TSK_CURSOR),                   "TS_CURSOR"           },
	{ (eTS_Keyword)RbTsInitValue(TSK_BLOCK_INPUT),              "TS_BLOCK_INPUT"      },
	{ (eTS_Keyword)RbTsInitValue(TSK_FREE_INPUT),               "TS_FREE_INPUT"       },
	{ (eTS_Keyword)RbTsInitValue(TSK_PUSHMACRO),                "PushMacro"           },
	{ (eTS_Keyword)RbTsInitValue(TSK_ONLY_KEY),                 "ONLY_KEY"            },
	{ (eTS_Keyword)RbTsInitValue(TSK_WITH_MOUSE),               "WITH_MOUSE"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_ENABLE),                   "ENABLE"              },
	{ (eTS_Keyword)RbTsInitValue(TSK_DISABLE),                  "DISABLE"             },
	{ (eTS_Keyword)RbTsInitValue(TSK_NULL),                     "NULL"                },
	{ (eTS_Keyword)RbTsInitValue(TSK_VIEW_WIDTH),               "VIEW_WIDTH"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_VIEW_HEIGHT),              "VIEW_HEIGHT"         },
	{ (eTS_Keyword)RbTsInitValue(TSK_OPTION_BAR_Y),             "OPTION_BAR_Y"        },
	{ (eTS_Keyword)RbTsInitValue(TSK_GETPOINT),                 "GETPOINT"            },
	{ (eTS_Keyword)RbTsInitValue(TSK_CAST_INT),                 "CAST_INT"            },
	{ (eTS_Keyword)RbTsInitValue(TSK_VAL_CUSTOM_ARROW),         "VAL_CUSTOM_ARROW"    },
	{ (eTS_Keyword)RbTsInitValue(TSK_BAR_RIGHTMOST),            "BAR_RIGHTMOST"       },
	{ (eTS_Keyword)RbTsInitValue(TSK_BAR_CENTER),               "BAR_CENTER"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_SKIP_COUNT),               "SKIP_COUNT"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_POINTNAME),                "POINTNAME"           },
	{ (eTS_Keyword)RbTsInitValue(TSK_TARGETNAME),               "TARGETNAME"          },
	{ (eTS_Keyword)RbTsInitValue(TSK_GAME_TYPE),                "GAME_TYPE"           },
	{ (eTS_Keyword)RbTsInitValue(TSK_GAME_TYPE_TUTORIAL_BASIC),
     "GAME_TYPE_TUTORIAL_BASIC"													   },
	{ (eTS_Keyword)RbTsInitValue(TSK_IF),                       "IF"                  },
	{ (eTS_Keyword)RbTsInitValue(TSK_ELSE),                     "ELSE"                },
	{ (eTS_Keyword)RbTsInitValue(TSK_ENDIF),                    "ENDIF"               },
	{ (eTS_Keyword)RbTsInitValue(TSK_GREATER),                  ">"                   },
	{ (eTS_Keyword)RbTsInitValue(TSK_LESS),                     "<"                   },
	{ (eTS_Keyword)RbTsInitValue(TSK_EQUAL),                    "=="                  },
	{ (eTS_Keyword)RbTsInitValue(TSK_NOT),                      "!"                   }
};

static const float BAR_OFFSET[2] = { 500.0f, 140.0f };

ts_Parser::ts_Parser()
{
	ClearVariables();
}

ts_Parser::ts_Parser(const char* pszFile)
{
	ClearVariables();
}

ts_Parser::~ts_Parser()
{
	PTS_CloseFile();
	ClearVariables();
}

void ts_Parser::ClearVariables()
{
	m_bOpened = false;
	memset(m_szFileName, 0, sizeof(m_szFileName));
	m_eFileType = FILE_NONE;
	m_pFile = NULL;
	memset(m_szLine, 0, sizeof(m_szLine));
	memset(m_szToken, 0, sizeof(m_szToken));
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
	m_wLineNo = 0;
	m_wErrorNo = 0;

	std::list<sTS_Error*>::iterator itError;
	for (itError = m_errors.begin(); itError != m_errors.end(); ++itError)
	{
		if (*itError != NULL)
		{
			delete *itError;
			*itError = NULL;
		}
	}
	m_errors.clear();

	std::list<sTS_ScriptInfo*>::iterator itInfo;
	for (itInfo = m_scriptInfos.begin(); itInfo != m_scriptInfos.end();
		++itInfo)
	{
		if (*itInfo != NULL)
		{
			delete *itInfo;
			*itInfo = NULL;
		}
	}
	m_scriptInfos.clear();

	std::list<sTS_Macro*>::iterator itMacro;
	for (itMacro = m_macros.begin(); itMacro != m_macros.end(); ++itMacro)
	{
		if (*itMacro != NULL)
		{
			delete *itMacro;
			*itMacro = NULL;
		}
	}
	m_macros.clear();

	ClearSymbolTable();
}

void ts_Parser::ClearSymbolTable()
{
	std::list<char*>::iterator it;
	for (it = m_symbols.begin(); it != m_symbols.end(); ++it)
	{
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_symbols.clear();
}

bool ts_Parser::PTS_OpenFile(const char* pszFile, eFileType type)
{
	if (m_bOpened)
		return false;

	if (pszFile == NULL)
		return false;

	memset(m_szFileName, 0, sizeof(m_szFileName));
	strcpy(m_szFileName, pszFile);

	m_eFileType = type;
	switch (type)
	{
	case FILE_NONE:
		break;
	case FILE_BINARY:
		m_pFile = fopen(m_szFileName, "rb");
		break;
	case FILE_TEXT:
		m_pFile = fopen(m_szFileName, "rt");
		break;
	}

	if (m_pFile == NULL)
		return false;

	m_bOpened = true;
	return true;
}

bool ts_Parser::PTS_CloseFile()
{
	// HACK: anchor the inline reader that owns the shared rt literal.
	if (0)
		PTS_OpenFile(NULL, FILE_TEXT);
	if (!m_bOpened && m_pFile != NULL)
	{
		fclose(m_pFile);
		m_pFile = NULL;
		m_bOpened = false;
		return true;
	}

	return false;
}

taskMsg_t ts_Parser::GetTaskMessage(const char* pszName) const
{
	if (pszName == NULL)
		return (taskMsg_t)-1;

	for (int i = 0; i < 9; i++)
	{
		if (strcmp(pszName, gTS_TaskMsgTable[i].name) == 0)
			return gTS_TaskMsgTable[i].msg;
	}

	return (taskMsg_t)-1;
}

eTS_Keyword ts_Parser::GetKeyword(const char* pszName) const
{
	if (pszName == NULL)
		return TSK_NONE;

	for (int i = 0; i < 43; i++)
	{
		if (strcmp(pszName, gTS_KeywordTable[i].name) == 0)
			return gTS_KeywordTable[i].keyword;
	}

	return TSK_NONE;
}

void ts_Parser::ConvertMacroType(eTS_Keyword keyword, eMacroType* pType) const
{
	if (pType == NULL)
		return;

	switch (keyword)
	{
	case TSK_WAIT:
		*pType = MACRO_WAIT;
		break;
	case TSK_KEY_DOWN:
		*pType = MACRO_KEY_DOWN;
		break;
	case TSK_KEY_DOWN_KEEP:
		*pType = MACRO_KEY_DOWN_KEEP;
		break;
	case TSK_KEY_RELEASE:
		*pType = MACRO_KEY_RELEASE;
		break;
	case TSK_MOUSE_CLICK:
		*pType = MACRO_MOUSE_CLICK;
		break;
	case TSK_MOUSE_MOVE:
		*pType = (eMacroType)5;
		break;
	case TSK_MISSION:
		*pType = MACRO_MISSION;
		break;
	case TSK_TIP:
		*pType = MACRO_TIP;
		break;
	case TSK_TIP_VARROW:
		*pType = MACRO_TIP_VARROW;
		break;
	case TSK_TIP_HARROW:
		*pType = MACRO_TIP_HARROW;
		break;
	case TSK_TIP_CUSTOM_ARROW:
		*pType = MACRO_TIP_CUSTOM_ARROW;
		break;
	case TSK_TIP_IMAGE:
		*pType = MACRO_TIP_IMAGE;
		break;
	case TSK_TARGET:
		*pType = MACRO_TARGET;
		break;
	case TSK_SEND_MSG:
		*pType = MACRO_SEND_MSG;
		break;
	case TSK_CURSOR:
		*pType = MACRO_CURSOR;
		break;
	case TSK_BLOCK_INPUT:
		*pType = MACRO_BLOCK_INPUT;
		break;
	case TSK_FREE_INPUT:
		*pType = MACRO_FREE_INPUT;
		break;
	default:
		*pType = MACRO_NONE;
		break;
	case TSK_POINTNAME:
	case TSK_TARGETNAME:
		break;
	}
}

int ts_Parser::InsertSymbolTable(const char* pszSymbol)
{
	int len = strlen(pszSymbol) + 1;
	char* pszCopy = new char[len];
	memset(pszCopy, 0, len);
	strcpy(pszCopy, pszSymbol);
	m_symbols.push_back(pszCopy);
	return (int)pszCopy;
}

int ts_Parser::FindQuotationMarkBlock(const char* pszText, int& start, int& end,
	int nth, bool bInclude)
{
	if (nth <= 5)
	{
		int quote[10] = { 0 };
		int count = 0;
		int len = strlen(pszText);
		for (int i = 0; i < len; i++)
		{
			if (pszText[i] == '\"')
			{
				quote[count] = i;
				count++;
			}
		}

		int s = quote[nth * 2 - 2];
		int e = quote[nth * 2 - 1];
		if (e - s > 0)
		{
			if (!bInclude)
			{
				s++;
				e--;
			}
			start = s;
			end = e;
			return e - s + 1;
		}
	}

	return 0;
}

void ts_Parser::ClearStringBuffer()
{
	memset(m_szLine, 0, sizeof(m_szLine));
	memset(m_szToken, 0, sizeof(m_szToken));
	memset(m_szBuffer, 0, sizeof(m_szBuffer));
}

bool ts_Parser::Rewind()
{
	if (!IsOpenedFile())
		return false;

	fpos_t pos = 0;
	bool bRet = true;
	if (fsetpos(m_pFile, &pos) != 0)
		bRet = false;

	return bRet;
}

bool ts_Parser::NextLine()
{
	if (!IsOpenedFile())
		return false;

	bool bRet = true;
	if (feof(m_pFile))
	{
		bRet = false;
	}
	else
	{
		fgets(m_szBuffer, MAX_BUFFER_LEN, m_pFile);
		if (feof(m_pFile))
			bRet = false;
	}

	return bRet;
}

bool ts_Parser::FindToken(const char* pszToken)
{
	if (IsOpenedFile())
	{
		fpos_t save = 0;
		fpos_t cur = 0;
		if (fgetpos(m_pFile, &save) == 0)
		{
			memset(m_szLine, 0, sizeof(m_szLine));
			memset(m_szToken, 0, sizeof(m_szToken));

			while (true)
			{
				if (feof(m_pFile))
				{
					fsetpos(m_pFile, &save);
					return false;
				}

				if (fgetpos(m_pFile, &cur) != 0)
					return false;

				fgets(m_szLine, MAX_LINE_LEN, m_pFile);
				sscanf(m_szLine, "%s", m_szToken);
				if (strcmp(m_szToken, pszToken) == 0)
				{
					int r = fsetpos(m_pFile, &cur);
					return r == 0;
				}
			}
		}
	}

	return false;
}

bool ts_Parser::RemoveToken()
{
	if (!IsOpenedFile())
		return false;

	bool bRet = true;
	if (fscanf(m_pFile, "%s", m_szToken) != 0)
		bRet = false;

	return bRet;
}

const char* ts_Parser::GetToken()
{
	if (!IsOpenedFile())
		return NULL;

	fscanf(m_pFile, "%s", m_szToken);

	fpos_t pos;
	if (fgetpos(m_pFile, &pos) != 0)
		return NULL;

	pos -= strlen(m_szToken);
	if (fsetpos(m_pFile, &pos) != 0)
		return NULL;

	return m_szToken;
}

const char* ts_Parser::GetLine()
{
	if (!IsOpenedFile())
		return NULL;

	if (feof(m_pFile))
		return NULL;

	fpos_t pos;
	if (fgetpos(m_pFile, &pos) != 0)
		return NULL;

	fgets(m_szLine, MAX_LINE_LEN, m_pFile);
	if (fsetpos(m_pFile, &pos) != 0)
		return NULL;

	return m_szLine;
}

bool ts_Parser::PTS_LoadScriptInfo()
{
	if (!IsOpenedFile())
		return false;

	fpos_t pos = 0;
	fsetpos(m_pFile, &pos);

	ClearStringBuffer();

	std::list<sTS_ScriptInfo*>::iterator it;
	for (it = m_scriptInfos.begin(); it != m_scriptInfos.end(); ++it)
	{
		if (*it != NULL)
		{
			delete *it;
			*it = NULL;
		}
	}
	m_scriptInfos.clear();

	ClearSymbolTable();

	if (FindToken("PTS_SCRIPT_INFO"))
	{
		RemoveToken();
		int r = memcmp(GetToken(), "{", 2);
		if (r == 0)
		{
			RemoveToken();
			r = memcmp(GetToken(), "}", 2);
			while (r != 0)
			{
				sTS_ScriptInfo* pInfo = new sTS_ScriptInfo;
				if (pInfo)
				{
					memset(pInfo, 0, sizeof(sTS_ScriptInfo));
					strcpy(pInfo->name, GetToken());
					RemoveToken();
					m_scriptInfos.push_back(pInfo);
				}
				r = memcmp(GetToken(), "}", 2);
			}
		}
	}

	return (int)m_scriptInfos.size() > 0;
}

bool ts_Parser::PTS_LoadScript(const char* pszName)
{
	if (!IsOpenedFile())
		return false;

	std::list<sTS_ScriptInfo*>::iterator it;
	for (it = m_scriptInfos.begin(); it != m_scriptInfos.end(); ++it)
	{
		if (*it != NULL && strcmp((*it)->name, pszName) == 0)
		{
			std::list<sTS_Macro*>::iterator itMacro;
			for (itMacro = m_macros.begin(); itMacro != m_macros.end();
				++itMacro)
			{
				if (*itMacro != NULL)
				{
					delete *itMacro;
					*itMacro = NULL;
				}
			}
			m_macros.clear();

			ClearStringBuffer();
			Rewind();
			ClearSymbolTable();

			if (FindToken("#BEGIN_SCRIPT_DECLARE") && FindToken(pszName))
			{
				RemoveToken();
				int r = memcmp(GetToken(), "{", 2);
				if (r == 0)
				{
					RemoveToken();
					r = memcmp(GetToken(), "}", 2);
					while (r != 0)
					{
						sTS_Macro* pMacro = CommandAnalyst(GetLine());
						if (pMacro != NULL)
							m_macros.push_back(pMacro);

						NextLine();
						r = memcmp(GetToken(), "}", 2);
					}
				}
			}

			return (int)m_macros.size() > 0;
		}
	}

	return false;
}

bool ts_Parser::PTS_PopMacro(sTS_Macro* pMacro)
{
	if (pMacro == NULL)
		return false;

	bool bRet = m_macros.size() > 0;
	if (bRet == true)
	{
		std::list<sTS_Macro*>::iterator it = m_macros.begin();
		if (it == m_macros.end())
			return false;

		if (*it == NULL)
			return false;

		*pMacro = **it;
		delete *it;
		*it = NULL;
		m_macros.erase(it);
	}

	return bRet;
}

sTS_Macro* ts_Parser::CommandAnalyst(const char* pszLine)
{
	sTS_Macro* pMacro = NULL;
	if (pszLine != NULL)
	{
		char szCmd[64] = { 0 };
		pMacro = new sTS_Macro;
		if (pMacro != NULL)
		{
			int len = strlen(pszLine) + 1;
			char* pszCopy = new char[len];
			memset(pszCopy, 0, len);
			strcpy(pszCopy, pszLine);
			sscanf(pszCopy, "%s", szCmd);
			if (GetKeyword(szCmd) != TSK_PUSHMACRO ||
				!PushMacro(pszCopy, *pMacro))
			{
				delete pMacro;
				pMacro = NULL;
			}

			if (pszCopy)
				delete pszCopy;
		}
	}

	return pMacro;
}

bool ts_Parser::PushMacro(const char* pszLine, sTS_Macro& macro)
{
	bool bResult = false;
	char szCmd[64] = { 0 };
	char szArg[64] = { 0 };
	char szBuf[2048] = { 0 };

	sscanf(pszLine, "%s %s", szCmd, szArg);
	switch (GetKeyword(szArg))
	{
	case TSK_TIP:
	case TSK_TIP_CUSTOM_ARROW:
	{
		int start = 0;
		int end;
		int len = FindQuotationMarkBlock(pszLine, start, end, 1, false);
		if (len <= 1)
			break;

		macro.type = 7;
		macro.pszArg = new char[len + 1];
		memset(macro.pszArg, 0, len + 1);
		memcpy(macro.pszArg, pszLine + start, len);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_TIP_VARROW:
	{
		int start = 0;
		int end = 0;
		int len = FindQuotationMarkBlock(pszLine, start, end, 1, true);
		memset(szBuf, 0, sizeof(szBuf));
		memcpy(szBuf, pszLine + start, len);

		int v[4] = { 0 };
		char szKw[4][64] = { 0 };
		char szOut[64] = { 0 };
		sscanf(pszLine + end + 1, "%d %d %d %d %s %s %s %s", &v[0], &v[1],
			&v[2], &v[3], szKw[0], szKw[1], szKw[2], szKw[3]);
		for (int i = 0; i < 4; i++)
		{
			switch (GetKeyword(szKw[i]))
			{
			case TSK_VIEW_WIDTH:
				v[i] += (int)g_view->GetWidth();
				break;
			case TSK_VIEW_HEIGHT:
				v[i] += (int)g_view->GetHeight();
				break;
			case TSK_OPTION_BAR_Y:
				v[i] += (int)COption::Instance()->gGetBar_Y();
				break;
			}
		}

		sprintf(szOut, "%d %d %d %d", v[0], v[1], v[2], v[3]);
		macro.type = 8;
		macro.pszArg = new char[strlen(szBuf) + strlen(szOut) + 2];
		memset(macro.pszArg, 0, strlen(szBuf) + strlen(szOut) + 2);
		sprintf(macro.pszArg, "%s %s", szBuf, szOut);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_TIP_HARROW:
	{
		int start = 0;
		int end;
		int len = strlen(pszLine);
		FindQuotationMarkBlock(pszLine, start, end, 1, true);
		len -= start;
		if (len <= 1)
			break;

		macro.type = 9;
		macro.pszArg = new char[len + 1];
		memset(macro.pszArg, 0, len + 1);
		memcpy(macro.pszArg, pszLine + start, len);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_TIP_IMAGE:
	{
		int start = 0;
		int end;
		int len = strlen(pszLine);
		FindQuotationMarkBlock(pszLine, start, end, 1, true);
		len -= start;
		if (len <= 1)
			break;

		macro.type = 0xb;
		macro.pszArg = new char[len + 1];
		memset(macro.pszArg, 0, len + 1);
		memcpy(macro.pszArg, pszLine + start, len);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_WAIT:
	{
		int n = -1;
		sscanf(pszLine, "%s %s %d", szCmd, szCmd, &n);
		macro.type = 0;
		macro.pszArg = NULL;
		macro.param = n;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_KEY_DOWN:
	{
		int n = -1;
		int count = 1;
		sscanf(pszLine, "%s %s %s %d %d", szCmd, szCmd, szArg, &n, &count);
		int len = strlen(szArg);
		if (len == 0)
			break;

		macro.type = 1;
		macro.pszArg = new char[strlen(szArg) + 1];
		memset(macro.pszArg, 0, strlen(szArg) + 1);
		strcpy(macro.pszArg, szArg);
		macro.param = n;
		macro.count = count;
		if (count < 1)
		{
			macro.count = 1;
			bResult = true;
			break;
		}
		bResult = true;
		break;
	}
	case TSK_KEY_DOWN_KEEP:
	{
		int n = -1;
		int count = 1;
		sscanf(pszLine, "%s %s %s %d %d", szCmd, szCmd, szArg, &n, &count);
		int len = strlen(szArg);
		if (len == 0)
			break;

		macro.type = 1;
		macro.pszArg = new char[strlen(szArg) + 1];
		memset(macro.pszArg, 0, strlen(szArg) + 1);
		strcpy(macro.pszArg, szArg);
		macro.param = n;
		macro.count = count;
		if (count < 1)
		{
			macro.count = 1;
			bResult = true;
			break;
		}
		bResult = true;
		break;
	}
	case TSK_KEY_RELEASE:
		macro.type = 3;
		macro.pszArg = NULL;
		macro.param = 0;
		macro.count = 1;
		bResult = true;
		break;
	case TSK_MOUSE_CLICK:
	{
		int x = 0;
		int y = 0;
		sscanf(pszLine, "%s %s %d %d", szCmd, szArg, &x, &y);
		macro.type = 4;
		sprintf(szCmd, "%d", x);
		macro.pszArg = new char[strlen(szCmd) + 1];
		memset(macro.pszArg, 0, strlen(szCmd) + 1);
		strcpy(macro.pszArg, szCmd);
		macro.param = y;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_MOUSE_MOVE:
	{
		int x = 0;
		int y = 0;
		int z = 0;
		sscanf(pszLine, "%s %s %d %d %d", szCmd, szArg, &y, &x, &z);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "%d %d", y, x);
		int len = strlen(szBuf);
		if (len == 0)
			break;

		macro.type = 5;
		macro.pszArg = new char[strlen(szBuf) + 1];
		memset(macro.pszArg, 0, strlen(szBuf) + 1);
		strcpy(macro.pszArg, szBuf);
		macro.param = z;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_MISSION:
		macro.type = 6;
		macro.pszArg = NULL;
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	case TSK_TARGET:
	{
		int n = 0;
		memset(szArg, 0, sizeof(szArg));
		sscanf(pszLine, "%s %s %d", szCmd, szArg, &n);
		int len = strlen(szArg);
		if (len == 0)
			break;

		macro.type = 0xc;
		macro.pszArg = new char[strlen(szArg) + 1];
		memset(macro.pszArg, 0, strlen(szArg) + 1);
		strcpy(macro.pszArg, szArg);
		macro.param = n;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_SEND_MSG:
	{
		char szTypes[3][64] = { 0 };
		char szVals[3][64] = { 0 };
		memset(szCmd, 0, sizeof(szCmd));
		memset(szArg, 0, sizeof(szArg));
		int start = 0;
		int end = 0;
		int len = FindQuotationMarkBlock(pszLine, start, end, 1, false);
		if (end - start >= 0x40)
			break;

		memcpy(szCmd, pszLine + start, len);
		sscanf(pszLine + end + 2, "%s %s %s %s %s %s %s", szArg, szTypes[0],
			szTypes[1], szTypes[2], szVals[0], szVals[1], szVals[2]);
		taskMsg_t msg = GetTaskMessage(szArg);
		for (int i = 0; i < 3; i++)
		{
			switch (GetKeyword(szTypes[i]))
			{
			case TSK_GETPOINT:
				sprintf(szVals[i], "%d",
					(int)CSharedDoc::Instance()
							->m_pGolfDoc->m_pPolySoup->FindPoint(szVals[i]) +
						0x24);
				break;
			case TSK_CAST_INT:
				sprintf(szVals[i], "%d", InsertSymbolTable(szVals[i]));
				break;
			case TSK_VAL_CUSTOM_ARROW:
				break;
			case TSK_BAR_RIGHTMOST:
				sprintf(szVals[i], "%f", atof(szVals[i]) + BAR_OFFSET[0]);
				break;
			case TSK_BAR_CENTER:
				sprintf(szVals[i], "%f", atof(szVals[i]) + BAR_OFFSET[1]);
				break;
			}
		}

		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, szCmd, msg, szVals[0], szVals[1], szVals[2]);
		macro.type = 0xd;
		macro.pszArg = new char[strlen(szBuf) + 1];
		memset(macro.pszArg, 0, strlen(szBuf) + 1);
		strcpy(macro.pszArg, szBuf);
		macro.param = 0;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_CURSOR:
	{
		sscanf(pszLine, "%s %s %s", szCmd, szArg, szBuf);
		int len = strlen(szBuf);
		if (len == 0)
			break;

		int v;
		switch (GetKeyword(szBuf))
		{
		case TSK_ENABLE:
			v = 1;
			break;
		case TSK_DISABLE:
			v = 0;
			break;
		default:
			return false;
		}

		memset(szCmd, 0, sizeof(szCmd));
		sprintf(szCmd, "%d", v);
		macro.type = 0xe;
		macro.pszArg = new char[strlen(szCmd) + 1];
		memset(macro.pszArg, 0, strlen(szCmd) + 1);
		strcpy(macro.pszArg, szCmd);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_BLOCK_INPUT:
	{
		bResult = false;
		sscanf(pszLine, "%s %s %s", szCmd, szArg, szBuf);
		int v;
		switch (GetKeyword(szBuf))
		{
		case TSK_ONLY_KEY:
			v = 0;
			break;
		case TSK_WITH_MOUSE:
			v = 1;
			break;
		default:
			return false;
		}

		macro.type = 0xf;
		macro.pszArg = new char[2];
		macro.pszArg[0] = 0;
		macro.pszArg[1] = 0;
		sprintf(macro.pszArg, "%d", v);
		macro.param = 0;
		macro.count = 1;
		bResult = true;
		break;
	}
	case TSK_FREE_INPUT:
		macro.type = 0x10;
		macro.pszArg = NULL;
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	case TSK_POINTNAME:
	case TSK_TARGETNAME:
	{
		sscanf(pszLine, "%s %s %s", szCmd, szArg, szBuf);
		switch (GetKeyword(szArg))
		{
		case TSK_POINTNAME:
			macro.type = 0x20;
			break;
		case TSK_TARGETNAME:
			macro.type = 0x21;
			break;
		}

		macro.pszArg = new char[strlen(szBuf) + 1];
		memset(macro.pszArg, 0, strlen(szBuf) + 1);
		strcpy(macro.pszArg, szBuf);
		macro.param = -1;
		macro.count = 1;
		bResult = true;
		break;
	}
	default:
		bResult = false;
		break;
	}

	return bResult;
}
