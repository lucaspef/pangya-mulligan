#pragma once

#include <string>
#include <vector>
#include <list>
#include <map>

class FrListBox;

struct sPrize
{
	int type;
	unsigned int start;
	unsigned int end;
	unsigned long itemId;
	int count;
};
class WSendPacket;

enum eGMToolResult
{
	GM_RESULT_SYNTAX_ERROR,
	GM_RESULT_ARGUMENT_ERROR,
	GM_RESULT_OK,
};

enum eGMToolkitStatus
{
	GM_TOOLKIT_NORMAL,
	GM_TOOLKIT_CONFIRM_WAIT,
};

enum eGMStatusFlag
{
	GM_STATUS_VISIBLE = 1,
	GM_STATUS_WHISPER = 2,
	GM_STATUS_CHANNEL = 4,
};

enum eGMStatusSwitch
{
	GM_SWITCH_OFF,
	GM_SWITCH_ON,
};

enum eGMCommandTag
{
	GM_CMD_DUMMY,
	GM_CMD_HELP,
	GM_CMD_COMMAND,
	GM_CMD_VISIBLE,
	GM_CMD_WHISPER,
	GM_CMD_CHANNEL,
	GM_CMD_STATUS,
	GM_CMD_LIST,
	GM_CMD_OPEN,
	GM_CMD_CLOSE,
	GM_CMD_KICK,
	GM_CMD_DISCONNECT,
	GM_CMD_DISCON_UID,
	GM_CMD_DESTROY,
	GM_CMD_WIND,
	GM_CMD_WEATHER,
	GM_CMD_IDENTITY,
	GM_CMD_NOTICE,
	GM_CMD_GIVEITEM,
	GM_CMD_GOLDENBELL,
	GM_CMD_SETPRIZE,
	GM_CMD_UNSETPRIZE,
	GM_CMD_SHOWPRIZE,
	GM_CMD_LOADSCRIPT,
	GM_CMD_NOTICEPRIZE = 25,
	GM_CMD_HIOHOLECUPSCALE983, // 0x1a: nova no 983 (tabela 00ce9f00); as de baixo andaram 1 (gettid 0x1b .. matchhole 0x20)
	GM_CMD_GETTID,
	GM_CMD_SETMISSION,
	GM_CMD_FINDITEM,
	GM_CMD_CATEGORY,
	GM_CMD_MATCHMAP,
	GM_CMD_MATCHHOLE,
};

enum { GM_COMMAND_COUNT983 = 32 }; // 983: linhas da s_gmCommandTable (0x1e no 645 + dummy)

class CGMToolkit : public WSingleton<CGMToolkit>
{
public:
	CGMToolkit();
	virtual ~CGMToolkit();

	bool Command(const char* command);
	void Display(const char* text, unsigned long color);
	bool IsAdministrator() const;
	bool IsGM() const;
	bool GetStatus(eGMStatusFlag flag);
	void SetStatus(eGMStatusFlag flag, eGMStatusSwitch sw);
	void SendStatus(eGMCommandTag tag);
	void SendStringBuffer(eGMCommandTag tag, const char* str);
	void SendCharacterBuffer(eGMCommandTag tag, const char* str);
	void SendPrize(FrListBox* pList);

	bool GetPrizeSwitch() const;

protected:
	static const int MAX_HOLE;

	eGMToolResult Parser();
	bool PhraseSaperator(const char* command);
	void ReleaseStringBuffer();
	int GetGMCommandTableNumber();
	bool Confirm();
	__forceinline bool FindKeywordInString(const char* const str, const char* const keyword)
	{
		int keyLen = strlen(keyword);
		int strLen = strlen(str);
		int k = 0;
		for (int i = 0; i < strLen; i++)
		{
			if (str[i] == keyword[k])
			{
				k++;
				if (k == keyLen)
					return true;
			}
			else
				k = 0;
		}
		return false;
	}
	eGMCommandTag GetConfirmWaitTag() const;
	eGMToolkitStatus GetToolKitStatus() const;
	void SetConfirmWaitTag(eGMCommandTag tag);
	void SetToolKitStatus(eGMToolkitStatus status);
	bool FindItem(unsigned long typeId, char* name) const;
	template <class T, class M>
	bool FindItem(const char* const keyword, M& table)
	{
		bool bFind = false;
		char buffer[256] = "\0";
		for (typename M::iterator it = table.begin(); it != table.end(); ++it)
		{
			if (FindKeywordInString((*it).second.c.Name, keyword))
			{
				memset(buffer, 0, sizeof(buffer));
				bFind = true;
				sprintf(buffer, "%12d : %s", (*it).second.c.TypeId,
					(*it).second.c.Name);
				Display(buffer, 0xc6ff85);
			}
		}

		return bFind;
	}

	eGMToolResult CommandHelp(int index);
	eGMToolResult CommandCommand(int index);
	eGMToolResult CommandVisible(eGMToolkitStatus status);
	eGMToolResult CommandWhisper(eGMToolkitStatus status);
	eGMToolResult CommandChannel(eGMToolkitStatus status);
	eGMToolResult CommandShowStatus(eGMToolkitStatus status);
	eGMToolResult CommandList(eGMToolkitStatus status);
	eGMToolResult CommandOpen(eGMToolkitStatus status);
	eGMToolResult CommandClose(eGMToolkitStatus status);
	eGMToolResult CommandKickUser(eGMToolkitStatus status);
	eGMToolResult CommandDisconnect(eGMToolkitStatus status);
	eGMToolResult CommandDiscon_UID(eGMToolkitStatus status);
	eGMToolResult CommandDestroy(eGMToolkitStatus status);
	eGMToolResult CommandWind(eGMToolkitStatus status);
	eGMToolResult CommandWeather(eGMToolkitStatus status);
	eGMToolResult CommandIdentity(eGMToolkitStatus status);
	eGMToolResult CommandNotice(eGMToolkitStatus status);
	eGMToolResult CommandGiveItem(eGMToolkitStatus status);
	eGMToolResult CommandGoldenBell(eGMToolkitStatus status);
	eGMToolResult CommandSetPrize(eGMToolkitStatus status);
	eGMToolResult CommandUnsetPrize(eGMToolkitStatus status);
	eGMToolResult CommandShowPrize(eGMToolkitStatus status);
	eGMToolResult CommandLoadScript(eGMToolkitStatus status);
	eGMToolResult CommandNoticePrize(eGMToolkitStatus status);
	eGMToolResult CommandGetTID(eGMToolkitStatus status);
	eGMToolResult CommandSetMission(eGMToolkitStatus status);
	eGMToolResult CommandFindItem(eGMToolkitStatus status);
	eGMToolResult CommandCategory(eGMToolkitStatus status);
	eGMToolResult CommandMatchMapSetting(eGMToolkitStatus status);
	eGMToolResult CommandMatchHoleSetting(eGMToolkitStatus status);
	eGMToolResult CommandHioHolecupScale983(eGMToolkitStatus status); // 00a783c0

	eGMToolkitStatus m_toolkitStatus;
	eGMCommandTag m_confirmWaitTag;
	WSendPacket* m_pConfirmPacket;
	bool m_bGM;
	unsigned short m_status;
	char** m_stringBuffer;
	int m_stringCount;
	std::vector<std::string> m_openList;
	char m_pad983_50[18]; // 983 so zera no ctor (00a79eb1..ebd), sem leitura
	char m_pad983_62[36]; // 983 so zera no ctor (00a79ec1..ed9), sem leitura
	std::list<sPrize*> m_prizeList;
	sPrize* m_pPrize;
	bool m_bPrizeSwitch;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandweatherFunclets(); // HACK: EH funclets of the naked CommandWeather (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandnoticeFunclets(); // HACK: EH funclets of the naked CommandNotice (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandsetmissioFunclets(); // HACK: EH funclets of the naked CommandSetMission (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandmatchmapsFunclets(); // HACK: EH funclets of the naked CommandMatchMapSetting (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommanddestroyFunclets(); // HACK: EH funclets of the naked CommandDestroy (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandidentityFunclets(); // HACK: EH funclets of the naked CommandIdentity (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandgoldenbelFunclets(); // HACK: EH funclets of the naked CommandGoldenBell (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandnoticepriFunclets(); // HACK: EH funclets of the naked CommandNoticePrize (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSendprizeFunclets(); // HACK: EH funclets of the naked SendPrize (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandkickuserFunclets(); // HACK: EH funclets of the naked CommandKickUser (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommanddisconnecFunclets(); // HACK: EH funclets of the naked CommandDisconnect (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommanddisconuidFunclets(); // HACK: EH funclets of the naked CommandDiscon_UID (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandgiveitemFunclets(); // HACK: EH funclets of the naked CommandGiveItem (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandcloseFunclets(); // HACK: EH funclets of the naked CommandClose (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbCommandopenFunclets(); // HACK: EH funclets of the naked CommandOpen (nakedify)
#endif
};
