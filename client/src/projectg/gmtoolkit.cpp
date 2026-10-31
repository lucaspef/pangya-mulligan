#include "minatl.h"
#include <algorithm>
#include "gmtoolkit.h"
#include "chatmsg.h"
#include "packet.h"
#include "commonutil.h"
#include "shareddoc.h"
#include "frlistbox.h"


#define COPY_STRING(dest, src) \
	{ \
		char* d = dest; \
		const char* s = src; \
		while ((*d++ = *s++) != 0) \
			; \
	}


struct sGMCommandEntry
{
	bool enabled;
	int tag;
	const char* name;
	const char* desc;
};

// 983: 32 linhas; 0 e 1 estaticas, o resto (e a descricao da 1) pelo inicializador dinamico 00ce9af0, com as descricoes
// passadas por K2L_Compatibility. Nova no 983: hioholecupscale (tag 0x1a, depois de setmission); gettid..matchhole 0x1b..0x20.
static sGMCommandEntry
	s_gmCommandTable[GM_COMMAND_COUNT983] = {
		{ false, GM_CMD_DUMMY, "dummy", "/dummy :: for Ntreev Soft." },
		{ true, GM_CMD_HELP, "help",
			K2L_Compatibility("/help [cmd] :: \xc1\xf6\xc1\xa4\xc7\xd1 \xb8\xed\xb7\xc9\xc0\xc7 \xbf\xeb\xb5\xb5 \xbc\xb3\xb8\xed.") }, // 0x1
		{ true, GM_CMD_COMMAND, "command",
			K2L_Compatibility("/command :: \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xc0\xd6\xb4\xc2 \xb8\xed\xb7\xc9\xbe\xee \xb8\xae\xbd\xba\xc6\xae \xc3\xe2\xb7\xc2.") }, // 0x2
		{ true, GM_CMD_STATUS, "status",
			K2L_Compatibility("/status :: \xc7\xf6\xc0\xe7 \xbc\xb3\xc1\xa4\xb5\xc8 \xbb\xf3\xc5\xc2\xb8\xa6 \xc3\xe2\xb7\xc2\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x6
		{ true, GM_CMD_VISIBLE, "visible",
			K2L_Compatibility("/visible [on/off] :: \xb7\xce\xba\xf1\xbf\xa1\xbc\xad \xc0\xaf\xc0\xfa\xb0\xa1 \xc0\xda\xbd\xc5\xc0\xbb \xba\xbc \xbc\xf6 \xc0\xd6\xb5\xb5\xb7\xcf \xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x3
		{ true, GM_CMD_WHISPER, "whisper",
			K2L_Compatibility("/whisper [on/off] :: \xb8\xf0\xb5\xe7 \xc0\xaf\xc0\xfa\xc0\xc7 \xb1\xd3\xbc\xd3\xb8\xbb\xc0\xbb \xc7\xe3\xbf\xeb\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x4
		{ true, GM_CMD_CHANNEL, "channel",
			K2L_Compatibility("/channel [on/off] :: \xc7\xe3\xbf\xeb\xb5\xc8 \xc0\xaf\xc0\xfa\xbf\xcd \xb1\xd3\xbc\xd3\xb8\xbb\xc0\xbb \xb0\xa1\xb4\xc9\xc7\xcf\xb0\xd4 \xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x5
		{ true, GM_CMD_LIST, "list",
			K2L_Compatibility("/list :: \xb1\xd3\xbc\xd3\xb8\xbb\xc0\xcc \xc7\xe3\xbf\xeb\xb5\xc8 \xc0\xaf\xc0\xfa \xb8\xae\xbd\xba\xc6\xae\xb8\xa6 \xc3\xe2\xb7\xc2\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x7
		{ true, GM_CMD_OPEN, "open",
			K2L_Compatibility("/open [nick] :: \xc1\xf6\xc1\xa4\xb5\xc8 \xc0\xaf\xc0\xfa\xc0\xc7 \xb1\xd3\xbc\xd3\xb8\xbb\xc0\xbb \xc7\xe3\xbf\xeb\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x8
		{ true, GM_CMD_CLOSE, "close",
			K2L_Compatibility("/close [nick] :: \xc1\xf6\xc1\xa4\xb5\xc8 \xc0\xaf\xc0\xfa\xb8\xa6 \xb1\xd3\xbc\xd3\xb8\xbb \xc7\xe3\xbf\xeb \xb8\xae\xbd\xba\xc6\xae\xbf\xa1\xbc\xad \xc1\xa6\xb0\xc5\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x9
		{ true, GM_CMD_KICK, "kick",
			K2L_Compatibility("/kick [nick][op] :: \xb4\xeb\xbb\xf3\xc0\xbb \xc5\xb1\xc7\xd5\xb4\xcf\xb4\xd9 '-p' \xbf\xc9\xbc\xc7\xc0\xbb \xc1\xd6\xb8\xe9 \xb0\xad\xc1\xa6 \xc1\xbe\xb7\xe1\xc0\xb2\xc0\xcc \xbf\xc3\xb6\xf3\xb0\xa9\xb4\xcf\xb4\xd9.") }, // 0xa
		{ true, GM_CMD_DISCONNECT, "disconnect",
			K2L_Compatibility("/disconnect [nick] :: \xb4\xeb\xbb\xf3\xc0\xbb \xb5\xf0\xbd\xba\xc4\xbf\xb3\xd8\xc6\xae \xbd\xc3\xc5\xb5\xb4\xcf\xb4\xd9.") }, // 0xb
		{ true, GM_CMD_DISCON_UID, "discon_uid",
			K2L_Compatibility("/discon_uid [uid] :: \xb4\xeb\xbb\xf3\xc0\xbb \xb5\xf0\xbd\xba\xc4\xbf\xb3\xd8\xc6\xae \xbd\xc3\xc5\xb5\xb4\xcf\xb4\xd9.") }, // 0xc
		{ true, GM_CMD_DESTROY, "destroy",
			K2L_Compatibility("/destroy [num] :: \xc1\xf6\xc1\xa4\xb5\xc8 \xb9\xf8\xc8\xa3\xc0\xc7 \xb9\xe6\xc0\xbb \xc6\xf8\xc6\xc4\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0xd
		{ true, GM_CMD_WIND, "wind",
			K2L_Compatibility("/wind [spd][dir] :: \xc1\xf6\xc1\xa4\xb5\xc8 \xbc\xd3\xb5\xb5\xbf\xcd \xb9\xe6\xc7\xe2\xc0\xb8\xb7\xce \xb9\xd9\xb6\xf7\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0xe
		{ true, GM_CMD_WEATHER, "weather",
			K2L_Compatibility("/weather [type] :: \xb3\xaf\xbe\xbe\xb8\xa6 \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9. 'fine', 'rain', 'snow', 'cloud'") }, // 0xf
		{ true, GM_CMD_IDENTITY, "identity",
			K2L_Compatibility("/identity [type] :: \xb0\xe8\xc1\xa4 \xb1\xc7\xc7\xd1\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9. 'admin', 'user'") }, // 0x10
		{ true, GM_CMD_NOTICE, "notice",
			K2L_Compatibility("/notice [msg] :: \xb8\xde\xbc\xbc\xc1\xf6\xb8\xa6 \xc3\xa4\xc6\xc3\xc3\xa2\xbf\xa1 \xb0\xf8\xc1\xf6\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x11
		{ true, GM_CMD_GIVEITEM, "giveitem",
			K2L_Compatibility("/giveitem [nick][typeid][num] :: \xc1\xf6\xc1\xa4\xb5\xc8 \xbb\xe7\xb6\xf7\xbf\xa1\xb0\xd4 \xbe\xc6\xc0\xcc\xc5\xdb \xbc\xb1\xb9\xb0.") }, // 0x12
		{ true, GM_CMD_GOLDENBELL, "goldenbell",
			K2L_Compatibility("/goldenbell [typeid][num] :: \xc7\xf6\xc0\xe7 \xb7\xeb\xbe\xc8\xc0\xc7 \xb8\xf0\xb5\xe7 \xbb\xe7\xb6\xf7\xbf\xa1\xb0\xd4 \xbe\xc6\xc0\xcc\xc5\xdb \xbc\xb1\xb9\xb0.") }, // 0x13
		{ true, GM_CMD_SETPRIZE, "setprize",
			K2L_Compatibility("/setprize [switch];[dest][typeid][num]") }, // 0x14
		{ true, GM_CMD_UNSETPRIZE, "unsetprize",
			K2L_Compatibility("/unsetprize :: setprize\xb7\xce \xbc\xb3\xc1\xa4\xb5\xc8 \xb3\xbb\xbf\xeb\xc0\xbb \xbb\xe8\xc1\xa6 \xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x15
		{ true, GM_CMD_SHOWPRIZE, "showprize",
			K2L_Compatibility("/showprize :: setprize\xb7\xce \xbc\xb3\xc1\xa4\xb5\xc8 \xb3\xbb\xbf\xeb\xc0\xbb \xc3\xe2\xb7\xc2 \xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x16
		{ false, GM_CMD_LOADSCRIPT, "loadscript",
			K2L_Compatibility("/loadscript [file] :: \xc6\xc4\xc0\xcf\xbf\xa1 \xbc\xb3\xc1\xa4\xb5\xc8 \xb3\xbb\xbf\xeb\xc0\xbb \xb0\xd4\xc0\xd3\xbf\xa1 \xc0\xfb\xbf\xeb\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x17
		{ true, GM_CMD_NOTICEPRIZE, "noticeprize",
			K2L_Compatibility("/noticeprize :: \xbb\xf3\xc7\xb0\xc1\xf6\xb1\xde \xc1\xb6\xb0\xc7 \xb0\xf8\xc1\xf6.") }, // 0x19
		{ false, GM_CMD_GETTID, "gettid",
			K2L_Compatibility("/gettid ITEM_NAME :: \xc7\xd8\xb4\xe7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xc7 TID\xb8\xa6 \xc3\xe2\xb7\xc2\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x1b
		{ true, GM_CMD_SETMISSION, "setmission",
			K2L_Compatibility("/setmission MISSION_NUM :: \xbe\xee\xc7\xc1\xb7\xce\xc4\xa1 \xb9\xcc\xbc\xc7\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9.") }, // 0x1c
		{ true, GM_CMD_HIOHOLECUPSCALE983, "hioholecupscale",
			K2L_Compatibility("/hio_holecup_scale [\xb1\xe2\xba\xbb\xb0\xaa(0)/\xb0\xed\xc1\xa4(1~9)] : \xc8\xa6\xc4\xc5\xc5\xa9\xb1\xe2 \xc1\xf6\xc1\xa4") }, // 0x1a
		{ true, GM_CMD_FINDITEM, "finditem",
			K2L_Compatibility("/finditem [category][keyword] :: \xc7\xd8\xb4\xe7\xc7\xcf\xb4\xc2 \xb9\xae\xc0\xda\xbf\xad\xc0\xcc \xb5\xe9\xbe\xee\xc0\xd6\xb4\xc2 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xb0\xcb\xbb\xf6.") }, // 0x1d
		{ true, GM_CMD_CATEGORY, "category",
			K2L_Compatibility("/category :: finditem \xb8\xed\xb7\xc9\xc0\xbb \xc0\xa7\xc7\xd1 category \xb8\xf1\xb7\xcf \xc3\xe2\xb7\xc2") }, // 0x1e
		{ true, GM_CMD_MATCHMAP, "matchmap",
			K2L_Compatibility("/matchmap [mapcount] :: \xb4\xeb\xc8\xb8\xbf\xeb \xb8\xca\xc0\xbb \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9. \xb1\xe2\xba\xbb\xbc\xb3\xc1\xa4\xc0\xba \xba\xed\xb7\xe7 \xb6\xf3\xb1\xba\xc0\xd4\xb4\xcf\xb4\xd9.") }, // 0x1f
		{ true, GM_CMD_MATCHHOLE, "matchhole",
			K2L_Compatibility("/matchhole [playhole] :: \xb4\xeb\xc8\xb8\xbf\xeb \xb4\xeb\xc0\xfc\xc8\xa6\xbc\xf6(count)\xb8\xa6 \xbc\xb3\xc1\xa4\xc7\xd5\xb4\xcf\xb4\xd9. 3, 6, 9, 18\xc8\xa6 \xc1\xdf\xbf\xa1 \xbc\xb1\xc5\xc3\xc7\xcf\xbc\xbc\xbf\xe4.") }, // 0x20
};

enum
{
	SD_MY_NAME = 0x3f8,
	SD_FLAGS = 0x433,
	SD_SCENE = 0x49e0,
	SD_ROOM = 0x49e1,
};
#define SD_BYTES() ((unsigned char*)CSharedDoc::Instance())
#define SD_U32(off) (*(unsigned int*)(SD_BYTES() + (off)))
#define SD_U8(off) (*(unsigned char*)(SD_BYTES() + (off)))
#define SD_S16(off) (*(short*)(SD_BYTES() + (off)))

bool CGMToolkit::GetPrizeSwitch() const
{
	return m_bPrizeSwitch;
}

eGMCommandTag CGMToolkit::GetConfirmWaitTag() const
{
	return m_confirmWaitTag;
}

eGMToolkitStatus CGMToolkit::GetToolKitStatus() const
{
	return m_toolkitStatus;
}

void CGMToolkit::SetToolKitStatus(eGMToolkitStatus status)
{
	m_toolkitStatus = status;
}

bool CGMToolkit::GetStatus(eGMStatusFlag flag)
{
	return (flag & m_status) != 0;
}

void CGMToolkit::SetStatus(eGMStatusFlag flag, eGMStatusSwitch sw)
{
	if (sw == GM_SWITCH_ON)
		m_status |= flag;
	else if (sw == GM_SWITCH_OFF)
		m_status &= ~flag;
}

void CGMToolkit::ReleaseStringBuffer()
{
	for (int i = 0; i < m_stringCount; i++)
	{
		if (m_stringBuffer[i])
		{
			delete[] m_stringBuffer[i];
			m_stringBuffer[i] = 0;
		}
	}

	if (m_stringBuffer)
	{
		delete[] m_stringBuffer;
		m_stringBuffer = 0;
	}

	m_stringCount = 0;
}

bool CGMToolkit::PhraseSaperator(const char* command)
{
	bool inWord = false;
	ReleaseStringBuffer();
	int length = strlen(command);
	if (command)
	{
		if (command[0] == '/')
		{
			for (int i = 0; command[i] != '\0'; i++)
			{
				if (command[i] == command[0])
				{
					if (i != 0)
					{
						m_stringCount = 0;
						break;
					}
				}
				else
				{
					m_stringCount = 1;
				}
			}
		}

		if (m_stringCount > 0)
		{
			for (int i = 0; command[i] != '\0'; i++)
			{
				if (command[i] == ' ' && i < length - 1 &&
					command[i + 1] != ' ' && command[i + 1] != '\0')
					m_stringCount++;
			}

			m_stringBuffer = new char*[m_stringCount];
			for (int i = 0; i < m_stringCount; i++)
				m_stringBuffer[i] = 0;

			int remain = m_stringCount;
			char word[128] = "\0";
			int pos = 0;
			const char* p = command + 1;
			while (*p != '\0')
			{
				bool flush = false;
				if (*p == ' ')
				{
					flush = true;
				}
				else
				{
					if (*p != '\0')
					{
						word[pos++] = *p;
						inWord = true;
					}

					if (p[1] == '\0')
						flush = true;
				}

				if (flush && inWord && remain > 0)
				{
					int size = strlen(word) + 1;
					m_stringBuffer[m_stringCount - remain] = new char[size];
					memset(m_stringBuffer[m_stringCount - remain], 0, size);
					strcpy(m_stringBuffer[m_stringCount - remain], word);
					memset(word, 0, 64);
					remain--;
					pos = 0;
					inWord = false;
				}

				p++;
			}

			return true;
		}
	}

	return false;
}

int CGMToolkit::GetGMCommandTableNumber()
{
	for (int i = 0; i < GM_COMMAND_COUNT983; i++)
	{
		if (strcmp(m_stringBuffer[0], s_gmCommandTable[i].name) == 0)
			return i;
	}

	return 0;
}

bool CGMToolkit::IsGM() const
{
	return m_bGM;
}

bool CGMToolkit::Confirm()
{
	bool result = false;
	if (m_stringCount == 1)
	{
		if (strcmp("y", m_stringBuffer[0]) == 0 ||
			strcmp("Y", m_stringBuffer[0]) == 0)
		{
			result = true;
			if (m_pConfirmPacket)
				m_pConfirmPacket->Send(TO_GAME);
		}
	}

	if (m_pConfirmPacket)
	{
		delete m_pConfirmPacket;
		m_pConfirmPacket = 0;
	}

	m_toolkitStatus = GM_TOOLKIT_NORMAL;
	return result;
}

// FindKeywordInString: inline in gmtoolkit.h (983 has no standalone body; it is inlined into the FindItem instances)

eGMToolResult CGMToolkit::CommandLoadScript(eGMToolkitStatus status)
{
	return GM_RESULT_OK;
}

void CGMToolkit::SetConfirmWaitTag(eGMCommandTag tag)
{
	if (0)
		CChatMsg::Instance();
	m_confirmWaitTag = tag;
	m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
}

void CGMToolkit::Display(const char* text, unsigned long color)
{
	CChatMsg::Instance()->AddChatMsg(text, color, true, false);
}

void CGMToolkit::SendStatus(eGMCommandTag tag)
{
	WSendPacket packet(0x8f);
	packet.Encode2(tag);
	packet.Encode2(m_status);
	packet.Send(TO_GAME);
}

void CGMToolkit::SendStringBuffer(eGMCommandTag tag, const char* str)
{
	std::string s(str);
	WSendPacket packet(0x8f);
	packet.Encode2(tag);
	packet.EncodeStr(s);
	packet.Send(TO_GAME);
}

void CGMToolkit::SendCharacterBuffer(eGMCommandTag tag, const char* str)
{
	int len = strlen(str) + 1;
	WSendPacket packet(0x8f);
	packet.Encode2(tag);
	packet.EncodeBuffer(str, len);
	packet.Send(TO_GAME);
}

eGMToolResult CGMToolkit::CommandHelp(int index)
{
	if (m_stringCount < 2)
	{
		if (m_stringCount != 1)
			return GM_RESULT_ARGUMENT_ERROR;

		Display(
			K2L_Compatibility("command \xb8\xed\xb7\xc9\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbf\xa9 \xb8\xed\xb7\xc9\xbe\xee \xc8\xae\xc0\xce\xc0\xcc \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
			0xc6ff85);
		return GM_RESULT_OK;
	}

	for (int i = 1; i < index; i++)
	{
		if (strcmp(m_stringBuffer[1], s_gmCommandTable[i].name) == 0)
		{
			if (s_gmCommandTable[i].enabled == 1)
				Display(s_gmCommandTable[i].desc, 0xc6ff85);
			else
				Display(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb8\xed\xb7\xc9\xbe\xee \xc0\xd4\xb4\xcf\xb4\xd9."),
					0xff7802);

			return GM_RESULT_OK;
		}
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandCommand(int index)
{
	for (int i = 1; i < index; i++)
	{
		if (s_gmCommandTable[i].enabled == 1)
			Display(s_gmCommandTable[i].desc, 0xc6ff85);
		else
			Display(s_gmCommandTable[i].desc, 0xff7802);
	}

	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandVisible(eGMToolkitStatus status)
{
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	const char* state;
	if (strcmp("on", m_stringBuffer[1]) == 0)
	{
		m_status |= GM_STATUS_VISIBLE;
		SendStatus(GM_CMD_VISIBLE);
		state = "visible on.";
	}
	else
	{
		if (strcmp("off", m_stringBuffer[1]) != 0)
			return GM_RESULT_ARGUMENT_ERROR;

		m_status &= ~GM_STATUS_VISIBLE;
		SendStatus(GM_CMD_VISIBLE);
		state = "visible off.";
	}

	Display(
		MakeStr(K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, %s"),
			state),
		0xc6ff85);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandWhisper(eGMToolkitStatus status)
{
	if (m_stringCount > 1)
	{
		if (strcmp("on", m_stringBuffer[1]) == 0)
		{
			m_status |= GM_STATUS_WHISPER;
			SendStatus(GM_CMD_WHISPER);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, whisper on."),
				0xc6ff85);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, channel off."),
				0xc6ff85);
			return GM_RESULT_OK;
		}

		if (strcmp("off", m_stringBuffer[1]) == 0)
		{
			m_status &= ~GM_STATUS_WHISPER;
			SendStatus(GM_CMD_WHISPER);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, whisper off."),
				0xc6ff85);
			return GM_RESULT_OK;
		}
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandChannel(eGMToolkitStatus status)
{
	if (m_stringCount > 1)
	{
		if (strcmp("on", m_stringBuffer[1]) == 0)
		{
			m_status = (m_status & ~GM_STATUS_WHISPER) | GM_STATUS_CHANNEL;
			SendStatus(GM_CMD_CHANNEL);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, channel on."),
				0xc6ff85);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, whisper off."),
				0xc6ff85);
			return GM_RESULT_OK;
		}

		if (strcmp("off", m_stringBuffer[1]) == 0)
		{
			m_status &= ~GM_STATUS_CHANNEL;
			SendStatus(GM_CMD_CHANNEL);
			Display(
				K2L_Compatibility("\xbb\xf3\xc5\xc2\xc7\xc3\xb7\xa1\xb1\xd7 \xba\xaf\xb0\xe6, channel off."),
				0xc6ff85);
			return GM_RESULT_OK;
		}
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandShowStatus(eGMToolkitStatus status)
{
	if (m_status & GM_STATUS_VISIBLE)
		Display("Visible : on", 0xc6ff85);
	else
		Display("Visible : off", 0xc6ff85);

	if (m_status & GM_STATUS_WHISPER)
		Display("Whisper : on", 0xc6ff85);
	else
		Display("Whisper : off", 0xc6ff85);

	if (m_status & GM_STATUS_CHANNEL)
	{
		Display("Channel : on", 0xc6ff85);
		return GM_RESULT_OK;
	}

	Display("Channel : off", 0xc6ff85);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandWeather(eGMToolkitStatus status)
{
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	unsigned char weather;
	if (strcmp("fine", m_stringBuffer[1]) == 0)
		weather = 0;
	else if (strcmp("cloud", m_stringBuffer[1]) == 0)
		weather = 1;
	else if (strcmp("rain", m_stringBuffer[1]) == 0)
		weather = 2;
	else if (strcmp("snow", m_stringBuffer[1]) == 0)
		weather = 3;
	else
		return GM_RESULT_ARGUMENT_ERROR;

	WSendPacket packet(0x8f);
	packet.Encode2(0xf);
	packet.Encode1(weather);
	packet.Send(TO_GAME);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandNotice(eGMToolkitStatus status)
{
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	std::string message = "";
	for (int i = 1; i < m_stringCount; i++)
	{
		message = message + m_stringBuffer[i];
		message = message + " ";
	}

	WSendPacket packet(0x57);
	packet.EncodeStr(message);
	packet.Send(TO_GAME);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandGetTID(eGMToolkitStatus status)
{
	if (m_stringCount > 1)
	{
		Display(K2L_Compatibility("Succeeded command."), 0xc6ff85);
		return GM_RESULT_OK;
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandSetMission(eGMToolkitStatus status)
{
	if (m_stringCount > 1)
	{
		unsigned long mission = atoi(m_stringBuffer[1]);
		WSendPacket packet(0x8f);
		packet.Encode2(GM_CMD_SETMISSION) /*983 00a78300: 0x1c; o 645 mandava 0x1b*/;
		packet.Encode4(mission);
		packet.Send(TO_GAME);
		Display(K2L_Compatibility("Succeeded command."), 0xc6ff85);
		return GM_RESULT_OK;
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandCategory(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount > 0)
	{
		Display(K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb"), 0xc6ff85);
		Display(K2L_Compatibility("\xc4\xab\xb5\xe5"), 0xc6ff85);
		Display(K2L_Compatibility("\xc6\xc4\xc3\xf7"), 0xc6ff85);
		Display(K2L_Compatibility("\xba\xb8\xc1\xb6\xc6\xc4\xc3\xf7"), 0xc6ff85);
		Display(K2L_Compatibility("\xc5\xac\xb7\xb4"), 0xc6ff85);
		Display(K2L_Compatibility("\xbe\xc6\xc1\xee\xc5\xd8"), 0xc6ff85);
		Display(K2L_Compatibility("\xbc\xbc\xc6\xae"), 0xc6ff85);
		Display(K2L_Compatibility("\xb0\xa1\xb1\xb8"), 0xc6ff85);
		Display(K2L_Compatibility("\xb8\xf0\xb5\xce"), 0xc6ff85);
		return result;
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

eGMToolResult CGMToolkit::CommandMatchMapSetting(eGMToolkitStatus status)
{
	if (m_stringCount != 2)
		return GM_RESULT_ARGUMENT_ERROR;

	unsigned long count = atoi(m_stringBuffer[1]);
	if (count <= 0x16 /*983 00a761de: ate 0x16; o 645 < 21*/)
	{
		WSendPacket packet(0x8f);
		packet.Encode2(GM_CMD_MATCHMAP) /*983 00a761f0: 0x1f*/;
		packet.Encode4(count);
		packet.Send(TO_GAME);
		return GM_RESULT_OK;
	}

	return GM_RESULT_ARGUMENT_ERROR;
}

// 983 00a783c0: "/hio_holecup_scale [0 / 1~9]" (tamanho do buraco); GM 0x8f/0x1a com a escala 1..9
eGMToolResult CGMToolkit::CommandHioHolecupScale983(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount > 1)
	{
		int scale = atoi(m_stringBuffer[1]);
		if (scale >= 1 && scale <= 9)
		{
			WSendPacket packet(0x8f);
			packet.Encode2(GM_CMD_HIOHOLECUPSCALE983);
			packet.Encode4(scale);
			packet.Send(TO_GAME);
			Display(K2L_Compatibility("Succeeded command."), 0xc6ff85);
		}
		else
		{
			Display(K2L_Compatibility("Invalid Scale.( 0 < Scale < 10 )"), 0xc6ff85);
			return GM_RESULT_ARGUMENT_ERROR;
		}
	}
	else
		result = GM_RESULT_ARGUMENT_ERROR;

	return result;
}

eGMToolResult CGMToolkit::CommandMatchHoleSetting(eGMToolkitStatus status)
{
	if (m_stringCount != 2)
		return GM_RESULT_ARGUMENT_ERROR;

	unsigned long hole = atoi(m_stringBuffer[1]);
	switch (hole)
	{
	case 3:
	case 6:
	case 9:
	case 18:
	{
		WSendPacket packet((enumClientPacket)0x8f);
		packet.Encode2(GM_CMD_MATCHHOLE) /*983 00a762c1: 0x20*/;
		packet.Encode4(hole);
		packet.Send(TO_GAME);
		return GM_RESULT_OK;
	}
	default:
		return GM_RESULT_ARGUMENT_ERROR;
	}
}

// 983 (00a764b0): bit 2 de m_myInfo.info.dwIdentity (+0x812); o 645 lia SD_FLAGS (+0x433).
bool CGMToolkit::IsAdministrator() const
{
	return (CSharedDoc::Instance()->m_myInfo.info.dwIdentity >> 2) & 1;
}

bool CGMToolkit::Command(const char* command)
{
	if (PhraseSaperator(command))
	{
		Display(MakeStr("GM_Command:%s", command), 0x6eecff);
		eGMToolResult result = Parser();
		if (result == GM_RESULT_SYNTAX_ERROR)
		{
			Display(K2L_Compatibility("Syntax error."), 0xff7802);
		}
		else
		{
			if (result == GM_RESULT_ARGUMENT_ERROR)
			{
				Display(K2L_Compatibility("Argument error."), 0xff7802);
				return false;
			}

			if (result == GM_RESULT_OK)
				return true;
		}
	}

	return false;
}

bool CGMToolkit::FindItem(unsigned long typeId, char* name) const
{
	IFF_ITEM_COMMON* item =
		CSharedDoc::Instance()->m_itemManager.FindCommonItem(typeId);
	if (!item)
		return false;

	strcpy(name, item->Name);
	return true;
}

eGMToolResult CGMToolkit::CommandDestroy(eGMToolkitStatus status)
{
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	if (CSharedDoc::Instance()->m_roomInfo.roomGuid != 0xffff) // 983: m_roomInfo.roomGuid (+0x1094d); era SD_ROOM do 645
	{
		Display(K2L_Compatibility(
			"\xb7\xce\xba\xf1 \xc0\xcc\xbf\xdc\xc0\xc7 \xc0\xe5\xbc\xd2\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xff7802);
		return GM_RESULT_OK;
	}

	int hole = atoi(m_stringBuffer[1]);
	WSendPacket destroy(0x60);
	destroy.Encode2(hole);
	destroy.Send(TO_GAME);
	WSendPacket packet(0x8f);
	packet.Encode2(0xd);
	packet.Send(TO_GAME);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandWind(eGMToolkitStatus status)
{
	if (0)
		((CGMToolkit*)0)->CGMToolkit::CommandDestroy(*(eGMToolkitStatus*)0);
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount > 2)
	{
		switch (CSharedDoc::Instance()->m_roomInfo.gameType) // 983: +0x1094c; era SD_SCENE do 645
		{
		case 4:
		case 5:
		case 6:
		case 9:
		case 10:
		case 11: // 983 00a77510: 0xb e 0x12, nao 14
		case 0x12:
			break;
		default:
			if (CSharedDoc::Instance()->m_roomInfo.gameType != 2)
			{
				unsigned char speed = atoi(m_stringBuffer[1]);
				unsigned char dir = atoi(m_stringBuffer[2]);
				if (speed > 18)
				{
					Display(K2L_Compatibility("Wind speed value [0-18]"), 0xff7802);
					result = GM_RESULT_ARGUMENT_ERROR;
				}

				if (dir > 360)
				{
					Display(K2L_Compatibility("Wind direction value [0-360]"), 0xff7802);
					result = GM_RESULT_ARGUMENT_ERROR;
				}
				else if (result != GM_RESULT_ARGUMENT_ERROR)
				{
					WSendPacket packet((enumClientPacket)0x8f);
					packet.Encode2(0xe);
					packet.Encode1(speed - 1);
					packet.Encode1(dir);
					packet.Send(TO_GAME);
				}
			}
			else
			{
				Display(K2L_Compatibility(
					"\xb4\xeb\xc8\xad\xb9\xe6\xbf\xa1\xbc\xad\xb4\xc2 \xbb\xe7\xbf\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xff7802);
			}
		}
	}
	else
		result = GM_RESULT_ARGUMENT_ERROR;

	return result;
}

eGMToolResult CGMToolkit::CommandIdentity(eGMToolkitStatus status)
{
	// JP 983 (00a77690; frente C, lote 14): a troca de identidade vai pelo comando GM (pacote 0x8f, GM_CMD_IDENTITY = 0x10)
	// e nao pelo pacote 0x41 do KR 645; textos por K2L_Compatibility; sala e nome pelos membros do CSharedDoc do 983
	// (m_roomInfo.roomGuid +0x1094d, m_myInfo.info.sNick +0x7b8; eram SD_ROOM/SD_MY_NAME do 645).
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount < 2)
	{
		if (m_stringCount != 1)
			return GM_RESULT_ARGUMENT_ERROR;

		Display(K2L_Compatibility(IsAdministrator()
					? "You are the Administrator now! (~+_+)~"
					: "You are a normal user T_T"),
			0xc6ff85);
		return result;
	}

	CSharedDoc* pDoc = CSharedDoc::Instance();
	if (pDoc->m_roomInfo.roomGuid != 0xffff)
	{
		Display(K2L_Compatibility(
			"\xb1\xc7\xc7\xd1 \xba\xaf\xb0\xe6\xc0\xba \xb7\xce\xba\xf1\xbf\xa1\xbc\xad\xb8\xb8 \xb0\xa1\xb4\xc9\xc7\xd5\xb4\xcf\xb4\xd9."),
			0xff7802);
		return result;
	}

	unsigned long identity;
	if (strcmp(m_stringBuffer[1], "admin") == 0)
	{
		identity = 0x484;
		Display(K2L_Compatibility("\xbc\xb3\xc1\xa4 : Administrator"), 0xc6ff85);
	}
	else
	{
		if (strcmp(m_stringBuffer[1], "user") != 0)
			return GM_RESULT_ARGUMENT_ERROR;

		identity = 0x80;
		Display(K2L_Compatibility("\xbc\xb3\xc1\xa4 : Normal user"), 0xc6ff85);
		m_bGM = true;
	}

	if (IsAdministrator())
	{
		if (identity != 0x484)
		{
			WSendPacket packet(0x8f);
			packet.Encode2(GM_CMD_IDENTITY);
			packet.Encode4(identity);
			packet.EncodeStr(std::string(CSharedDoc::Instance()->m_myInfo.info.sNick));
			packet.Send(TO_GAME);
		}
	}
	else if (identity != 0x80)
	{
		WSendPacket packet(0x8f);
		packet.Encode2(GM_CMD_IDENTITY);
		packet.Encode4(identity);
		packet.EncodeStr(std::string(CSharedDoc::Instance()->m_myInfo.info.sNick));
		packet.Send(TO_GAME);
	}

	return result;
}

void CGMToolkit::SendPrize(FrListBox* pList)
{
	int size = pList->GetCurrentItemSize(true);
	for (std::list<sPrize*>::iterator it = m_prizeList.begin();
		it != m_prizeList.end(); ++it)
	{
		sPrize* prize = *it;
		unsigned int end = prize->end;
		unsigned int rank = prize->start;
		if (prize->type == 0)
		{
			for (; rank <= end; rank++)
			{
				if (size < (int)rank)
					break;

				FrListItem* item = pList->GetItem(rank - 1);
				if (!item)
					break;

				unsigned char* data = (unsigned char*)item->pData;
				if (data[0x36] != 3)
				{
					WSendPacket packet(0x8f);
					packet.Encode2(0x14);
					packet.Encode4(*(unsigned long*)(data + 4));
					packet.Encode4(prize->itemId);
					packet.Encode4(prize->count);
					packet.Encode1(prize->type);
					packet.Encode1(rank);
					packet.Send(TO_GAME);
				}
			}
		}
		else
		{
			for (int i = 0; i < size; i++)
			{
				FrListItem* item = pList->GetItem(i);
				if (!item)
					break;

				unsigned char* data = (unsigned char*)item->pData;
				unsigned char mark;
				// 983 (00a76610): m_awardItem[0..5].uid (+0x1049a, passo 8); eram SD_U32(0x456e..0x4596) do 645
				switch (prize->type)
				{
				case 1:
					if (CSharedDoc::Instance()->m_awardItem[1].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x65;
					break;
				case 2:
					if (CSharedDoc::Instance()->m_awardItem[2].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x66;
					break;
				case 3:
					if (CSharedDoc::Instance()->m_awardItem[3].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x6a;
					break;
				case 4:
					if (CSharedDoc::Instance()->m_awardItem[4].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x69;
					break;
				case 5:
					if (CSharedDoc::Instance()->m_awardItem[5].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x67;
					break;
				case 6:
					if (CSharedDoc::Instance()->m_awardItem[0].uid != *(unsigned int*)(data + 4))
						continue;
					mark = 0x68;
					break;
				default:
					continue;
				}

				if (data[0x36] != 3)
				{
					WSendPacket packet(0x8f);
					packet.Encode2(0x14);
					packet.Encode4(*(unsigned long*)(data + 4));
					packet.Encode4(prize->itemId);
					packet.Encode4(prize->count);
					packet.Encode1(prize->type);
					packet.Encode1(mark);
					packet.Send(TO_GAME);
					break;
				}
			}
		}
	}
}

eGMToolResult CGMToolkit::CommandList(eGMToolkitStatus status)
{
	if (!m_openList.empty())
	{
		int count = m_openList.size();
		if (count > 0)
		{
			for (int i = 0; i < count; i++)
				CChatMsg::Instance()->AddChatMsg(m_openList[i], 0xc6ff85, true,
					false);

			return GM_RESULT_OK;
		}
	}

	Display(
		K2L_Compatibility("\xc3\xdf\xb0\xa1\xb5\xc8 \xb4\xeb\xc8\xad\xbb\xf3\xb4\xeb\xb0\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
		0xff7802);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandDiscon_UID(eGMToolkitStatus status)
{
	CSharedDoc* doc = CSharedDoc::Instance();
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount < 2)
	{
		result = GM_RESULT_ARGUMENT_ERROR;
	}
	else
	{
		std::map<unsigned long, sBriefUserInfo>::iterator it;
		unsigned long uid = atoi(m_stringBuffer[1]);
		if (doc->FindUserInfoUID(uid, it) == 0)
		{
			result = GM_RESULT_ARGUMENT_ERROR;
		}
		else
		{
			sBriefUserInfo info = it->second;
			if (it == CSharedDoc::Instance()->m_briefUserInfoMap.end())
			{
				Display(
					K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xb0\xc5\xb3\xaa \xbe\xf8\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xd4\xb4\xcf\xb4\xd9."),
					0xff7802);
			}
			else if (!info.IsIdentity(4))
			{
				WSendPacket packet(0x8f);
				packet.Encode2(0xb);
				packet.Encode4(info.dwGuid);
				packet.Send(TO_GAME);
				WSendPacket kick(0x61);
				kick.Encode4(info.dwGuid);
				kick.Send(TO_GAME);
			}
			else
			{
				Display(
					K2L_Compatibility("\xbf\xee\xbf\xb5\xc0\xda\xb4\xc2 \xb5\xf0\xbd\xba\xc4\xbf\xb3\xd8\xc6\xae \xbd\xc3\xc5\xb3 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xff7802);
			}
		}
	}

	return result;
}

eGMToolResult CGMToolkit::CommandUnsetPrize(eGMToolkitStatus status)
{
	if (status == GM_TOOLKIT_NORMAL)
	{
		if (m_stringCount > 0)
		{
			Display(
				K2L_Compatibility("\xbd\xc3\xbb\xf3\xc7\xb0\xb8\xf1 \xb8\xae\xbd\xba\xc6\xae\xb8\xa6 \xc0\xfc\xba\xce \xbb\xe8\xc1\xa6\xc7\xcf\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?. [/Y, /N]"),
				0xc6ff85);
			m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
			m_confirmWaitTag = GM_CMD_UNSETPRIZE;
			return GM_RESULT_OK;
		}

		return GM_RESULT_SYNTAX_ERROR;
	}

	if (!Confirm())
	{
		Display(
			K2L_Compatibility("'unsetprize' \xb8\xed\xb7\xc9\xc0\xcc \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xc6ff85);
		m_toolkitStatus = GM_TOOLKIT_NORMAL;
		return GM_RESULT_OK;
	}

	for (std::list<sPrize*>::iterator it = m_prizeList.begin();
		it != m_prizeList.end(); ++it)
	{
		if (*it)
			delete *it;
	}

	m_prizeList.clear();
	Display(
		K2L_Compatibility("\xbd\xc3\xbb\xf3\xc7\xb0\xb8\xf1 \xb8\xae\xbd\xba\xc6\xae \xbb\xe8\xc1\xa6\xbf\xcf\xb7\xe1."),
		0xc6ff85);
	m_bPrizeSwitch = false;
	m_toolkitStatus = GM_TOOLKIT_NORMAL;
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandShowPrize(eGMToolkitStatus status)
{
	char name[64] = "\0";
	char buffer[128] = "\0";
	if (!m_bPrizeSwitch)
		Display(K2L_Compatibility("SetPrize \xb1\xe2\xb4\xc9 \xba\xf1\xc8\xb0\xbc\xba\xc8\xad"),
			0xc6ff85);
	else
		Display(K2L_Compatibility("SetPrize \xb1\xe2\xb4\xc9 \xc8\xb0\xbc\xba\xc8\xad"), 0xc6ff85);

	int number = 0;
	for (std::list<sPrize*>::iterator it = m_prizeList.begin();
		it != m_prizeList.end(); ++it)
	{
		sPrize* prize = *it;
		if (!prize)
			continue;

		IFF_ITEM_COMMON* item =
			CSharedDoc::Instance()->m_itemManager.FindCommonItem(prize->itemId);
		if (!item)
		{
			Display(K2L_Compatibility("\xc0\xdf\xb8\xf8\xb5\xc8 \xc1\xb6\xb0\xc7\xb9\xae."),
				0xff7802);
			continue;
		}

		strcpy(name, item->Name);
		number++;
		memset(buffer, 0, sizeof(buffer));
		if (prize->type == 0)
		{
			if (prize->start == prize->end)
				sprintf(buffer,
					K2L_Compatibility("[%02d] \xb9\xfc\xc0\xa7 : %d\xb5\xee, \xbe\xc6\xc0\xcc\xc5\xdb : %s, \xbc\xf6\xb7\xae : %d"),
					number, prize->start, name, prize->count);
			else
				sprintf(buffer,
					K2L_Compatibility("[%02d] \xb9\xfc\xc0\xa7 : %d\xb5\xee \xba\xce\xc5\xcd %d\xb5\xee \xb1\xee\xc1\xf6, \xbe\xc6\xc0\xcc\xc5\xdb : %s, \xbc\xf6\xb7\xae : %d"),
					number, prize->start, prize->end, name, prize->count);
		}
		else
		{
			char range[32] = "\0";
			switch (prize->type)
			{
			case 1:
				COPY_STRING(range,
					K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xbd\xba\xc7\xc7\xb4\xf5"));
				break;
			case 2:
				COPY_STRING(range, K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb7\xaf\xb3\xca"));
				break;
			case 3:
				COPY_STRING(range, K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xc4\xa8\xc0\xce"));
				break;
			case 4:
				COPY_STRING(range,
					K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb7\xd5\xc6\xdb\xc5\xcd"));
				break;
			case 5:
				COPY_STRING(range,
					K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb8\xae\xc4\xbf\xb9\xf6\xb8\xae"));
				break;
			case 6:
				COPY_STRING(range, K2L_Compatibility("\xc7\xe0\xbf\xee\xbb\xf3"));
				break;
			}

			sprintf(buffer,
				K2L_Compatibility("[%02d] \xb9\xfc\xc0\xa7 : %s, \xbe\xc6\xc0\xcc\xc5\xdb : %s, \xbc\xf6\xb7\xae : %d"),
				number, range, name, prize->count);
		}

		Display(buffer, 0xc6ff85);
	}

	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandNoticePrize(eGMToolkitStatus status)
{
	unsigned long count = m_prizeList.size();
	if (count == 0)
	{
		Display(
			K2L_Compatibility("\xb5\xee\xb7\xcf\xb5\xc8 \xc1\xb6\xb0\xc7\xc0\xcc \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xc6ff85);
	}
	else
	{
		WSendPacket packet(0x8f);
		packet.Encode2(0x19);
		packet.Encode4(count);
		for (std::list<sPrize*>::iterator it = m_prizeList.begin();
			it != m_prizeList.end(); ++it)
		{
			if (*it)
				packet.EncodeBuffer(*it, sizeof(sPrize));
		}

		packet.Send(TO_GAME);
	}

	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::Parser()
{
	eGMToolkitStatus status = m_toolkitStatus;
	unsigned int tag = 0;
	int index = 0;
	eGMToolResult result = GM_RESULT_OK;
	if (status == GM_TOOLKIT_NORMAL)
	{
		index = GM_COMMAND_COUNT983; // 983 00a7a34b: 0x20
		int number = GetGMCommandTableNumber();
		tag = s_gmCommandTable[number].tag;
		if (s_gmCommandTable[number].enabled == false && tag != 0)
		{
			Display(
				K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbb\xe7\xbf\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb8\xed\xb7\xc9\xbe\xee \xc0\xd4\xb4\xcf\xb4\xd9."),
				0xff7802);
			return result;
		}
	}
	else if (status == GM_TOOLKIT_CONFIRM_WAIT)
	{
		tag = m_confirmWaitTag;
	}

	switch (tag)
	{
	case GM_CMD_DUMMY:
		result = GM_RESULT_SYNTAX_ERROR;
		break;
	case GM_CMD_HELP:
		result = CommandHelp(index);
		break;
	case GM_CMD_COMMAND:
		result = CommandCommand(index);
		break;
	case GM_CMD_VISIBLE:
		result = CommandVisible(status);
		break;
	case GM_CMD_WHISPER:
		result = CommandWhisper(status);
		break;
	case GM_CMD_CHANNEL:
		result = CommandChannel(status);
		break;
	case GM_CMD_LIST:
		result = CommandList(status);
		break;
	case GM_CMD_OPEN:
		result = CommandOpen(status);
		break;
	case GM_CMD_CLOSE:
		result = CommandClose(status);
		break;
	case GM_CMD_KICK:
		result = CommandKickUser(status);
		break;
	case GM_CMD_DISCONNECT:
		result = CommandDisconnect(status);
		break;
	case GM_CMD_DISCON_UID:
		result = CommandDiscon_UID(status);
		break;
	case GM_CMD_STATUS:
		result = CommandShowStatus(status);
		break;
	case GM_CMD_DESTROY:
		result = CommandDestroy(status);
		break;
	case GM_CMD_WIND:
		result = CommandWind(status);
		break;
	case GM_CMD_WEATHER:
		result = CommandWeather(status);
		break;
	case GM_CMD_IDENTITY:
		result = CommandIdentity(status);
		break;
	case GM_CMD_NOTICE:
		result = CommandNotice(status);
		break;
	case GM_CMD_GIVEITEM:
		result = CommandGiveItem(status);
		break;
	case GM_CMD_GOLDENBELL:
		result = CommandGoldenBell(status);
		break;
	case GM_CMD_SETPRIZE:
		result = CommandSetPrize(status);
		break;
	case GM_CMD_UNSETPRIZE:
		result = CommandUnsetPrize(status);
		break;
	case GM_CMD_SHOWPRIZE:
		result = CommandShowPrize(status);
		break;
	case GM_CMD_NOTICEPRIZE:
		result = CommandNoticePrize(status);
		break;
	case GM_CMD_GETTID:
		result = CommandGetTID(status);
		break;
	case GM_CMD_SETMISSION:
		result = CommandSetMission(status);
		break;
	case GM_CMD_HIOHOLECUPSCALE983: // 983 00a7a55e
		result = CommandHioHolecupScale983(status);
		break;
	case GM_CMD_FINDITEM:
		result = CommandFindItem(status);
		break;
	case GM_CMD_CATEGORY:
		result = CommandCategory(status);
		break;
	case GM_CMD_MATCHMAP:
		result = CommandMatchMapSetting(status);
		break;
	case GM_CMD_MATCHHOLE:
		result = CommandMatchHoleSetting(status);
		break;
	}

	return result;
}

eGMToolResult CGMToolkit::CommandKickUser(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_OK;
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	std::map<unsigned long, sBriefUserInfo>& users =
		CSharedDoc::Instance()->m_briefUserInfoMap;
	std::map<unsigned long, sBriefUserInfo>::iterator it = users.begin();
	for (; it != users.end(); ++it)
	{
		sBriefUserInfo info = it->second;
		if (strcmp(info.sNick, m_stringBuffer[1]) == 0)
			break;
	}

	sBriefUserInfo info = it->second;
	if (it == users.end())
	{
		Display(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xb0\xc5\xb3\xaa \xbe\xf8\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xd4\xb4\xcf\xb4\xd9."),
			0xff7802);
		return result;
	}

	if (info.IsIdentity(4))
	{
		Display(
			K2L_Compatibility("\xbf\xee\xbf\xb5\xc0\xda\xb4\xc2 \xb0\xad\xc1\xa6\xc5\xf0\xc0\xe5 \xbd\xc3\xc5\xb3 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xff7802);
		return result;
	}

	WSendPacket packet(0x8f);
	packet.Encode2(10);
	packet.Encode4(info.dwGuid);
	unsigned char option = 0;
	if (m_stringCount > 2)
	{
		if (strcmp("-p", m_stringBuffer[2]) != 0)
			return GM_RESULT_ARGUMENT_ERROR;

		option = 1;
	}

	packet.Encode1(option);
	packet.Send(TO_GAME);
	return result;
}

eGMToolResult CGMToolkit::CommandDisconnect(eGMToolkitStatus status)
{
	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	std::map<unsigned long, sBriefUserInfo>& users =
		CSharedDoc::Instance()->m_briefUserInfoMap;
	std::map<unsigned long, sBriefUserInfo>::iterator it = users.begin();
	for (; it != users.end(); ++it)
	{
		sBriefUserInfo info = it->second;
		if (strcmp(info.sNick, m_stringBuffer[1]) == 0)
			break;
	}

	sBriefUserInfo info = it->second;
	if (it == users.end())
	{
		Display(
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xb0\xc5\xb3\xaa \xbe\xf8\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xd4\xb4\xcf\xb4\xd9."),
			0xff7802);
	}
	else if (!info.IsIdentity(4))
	{
		WSendPacket packet(0x8f);
		packet.Encode2(0xb);
		packet.Encode4(info.dwGuid);
		packet.Send(TO_GAME);
		WSendPacket kick(0x61);
		kick.Encode4(info.dwGuid);
		kick.Send(TO_GAME);
	}
	else
	{
		Display(
			K2L_Compatibility("\xbf\xee\xbf\xb5\xc0\xda\xb4\xc2 \xb5\xf0\xbd\xba\xc4\xbf\xb3\xd8\xc6\xae \xbd\xc3\xc5\xb3 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xff7802);
	}

	return GM_RESULT_OK;
}

CGMToolkit::CGMToolkit()
{
	m_bGM = false;
	m_status = 0;
	m_stringBuffer = 0;
	m_stringCount = 0;
	m_toolkitStatus = GM_TOOLKIT_NORMAL;
	m_confirmWaitTag = GM_CMD_DUMMY;
	m_pConfirmPacket = 0;
	memset(m_pad983_50, 0, sizeof(m_pad983_50));
	memset(m_pad983_62, 0, sizeof(m_pad983_62));
	m_prizeList.clear();
	m_pPrize = 0;
	m_bPrizeSwitch = false;
}

CGMToolkit::~CGMToolkit()
{
	ReleaseStringBuffer();
	if (m_pConfirmPacket)
	{
		delete m_pConfirmPacket;
		m_pConfirmPacket = 0;
	}

	if (m_pPrize)
	{
		delete m_pPrize;
		m_pPrize = 0;
	}

	for (std::list<sPrize*>::iterator it = m_prizeList.begin();
		it != m_prizeList.end(); ++it)
	{
		if (*it)
		{
			delete *it;
			*it = 0;
		}
	}

	m_prizeList.clear();
}

eGMToolResult CGMToolkit::CommandClose(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_ARGUMENT_ERROR;
	if (m_stringCount > 1)
	{
		std::string name(m_stringBuffer[1]);
		std::vector<std::string>::iterator it =
			std::find(m_openList.begin(), m_openList.end(), name);
		if (it == m_openList.end())
		{
			Display(
				K2L_Compatibility("\xb4\xeb\xc8\xad\xbb\xf3\xb4\xeb \xb8\xae\xbd\xba\xc6\xae\xbf\xa1 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xff7802);
		}
		else
		{
			m_openList.erase(it);
			Display(
				K2L_Compatibility("\xb4\xeb\xc8\xad\xbb\xf3\xb4\xeb \xb8\xae\xbd\xba\xc6\xae\xbf\xa1\xbc\xad \xbb\xe8\xc1\xa6\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xc6ff85);
			SendStringBuffer(GM_CMD_CLOSE, m_stringBuffer[1]);
		}

		result = GM_RESULT_OK;
	}

	return result;
}

eGMToolResult CGMToolkit::CommandOpen(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_ARGUMENT_ERROR;
	if (m_stringCount > 1)
	{
		std::string name(m_stringBuffer[1]);
		const char* message;
		unsigned long color;
		if (std::find(m_openList.begin(), m_openList.end(), name) ==
			m_openList.end())
		{
			m_openList.push_back(name);
			SendStringBuffer(GM_CMD_OPEN, m_stringBuffer[1]);
			color = 0xc6ff85;
			message =
				K2L_Compatibility("\xb4\xeb\xc8\xad\xbb\xf3\xb4\xeb \xb8\xae\xbd\xba\xc6\xae\xbf\xa1 \xc3\xdf\xb0\xa1\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9.");
		}
		else
		{
			color = 0xff7802;
			message =
				"\xb4\xeb\xc8\xad\xbb\xf3\xb4\xeb \xb8\xae\xbd\xba\xc6\xae\xbf\xa1 \xc0\xcc\xb9\xcc \xc3\xdf\xb0\xa1\xb5\xc8 \xbb\xf3\xb4\xeb\xc0\xd4\xb4\xcf\xb4\xd9.";
		}

		Display(message, color);
		result = GM_RESULT_OK;
	}

	return result;
}

eGMToolResult CGMToolkit::CommandGoldenBell(eGMToolkitStatus status)
{
	if (CSharedDoc::Instance()->m_roomInfo.roomGuid == 0xffff) // 983: m_roomInfo.roomGuid; era SD_ROOM do 645
	{
		Display(
			K2L_Compatibility("\xb7\xce\xba\xf1\xbf\xa1\xbc\xad\xb4\xc2 \xbb\xe7\xbf\xeb \xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xff7802);
		return GM_RESULT_OK;
	}

	if (status != GM_TOOLKIT_NORMAL)
	{
		if (!Confirm())
		{
			Display(
				K2L_Compatibility("'goldenbell' \xb8\xed\xb7\xc9\xc0\xcc \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xc6ff85);
			m_toolkitStatus = GM_TOOLKIT_NORMAL;
			return GM_RESULT_OK;
		}

		Display(
			K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc1\xf6\xb1\xde\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xc6ff85);
		m_toolkitStatus = GM_TOOLKIT_NORMAL;
		return GM_RESULT_OK;
	}

	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	char name[64] = "\0";
	unsigned long count;
	if (m_stringCount == 2)
	{
		count = 1;
	}
	else
	{
		if (m_stringCount < 3)
			return GM_RESULT_OK;

		count = atoi(m_stringBuffer[2]);
		if (count == 0)
			return GM_RESULT_OK;
	}

	unsigned long typeId = atoi(m_stringBuffer[1]);
	if (FindItem(typeId, name))
	{
		if (m_pConfirmPacket)
			return GM_RESULT_SYNTAX_ERROR;

		m_pConfirmPacket = new WSendPacket(0x8f);
		m_pConfirmPacket->Encode2(0x13);
		m_pConfirmPacket->Encode4(typeId);
		m_pConfirmPacket->Encode4(count);
		char buffer[128] = "\0";
		sprintf(buffer,
			K2L_Compatibility("\xc5\xb8\xc0\xd4ID : %d, \xbe\xc6\xc0\xcc\xc5\xdb : %s, \xbc\xf6\xb7\xae : %d"),
			typeId, name, count);
		Display(buffer, 0xc6ff85);
		memset(buffer, 0, sizeof(buffer));
		sprintf(buffer,
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xb7\xeb\xbe\xc8\xc0\xc7 \xc0\xaf\xc0\xfa\xbf\xa1\xb0\xd4 \xc0\xa7\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc1\xf6\xb1\xde \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?. [/Y, /N]"));
		Display(buffer, 0xc6ff85);
		m_confirmWaitTag = GM_CMD_GOLDENBELL;
		m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
	}
	else
	{
		Display(
			K2L_Compatibility("\xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb TypeID \xc0\xd4\xb4\xcf\xb4\xd9."),
			0xff7802);
	}

	return GM_RESULT_OK;
}

// the original copies the category names with a hand-written byte loop

#pragma section(".text", execute, read)
eGMToolResult CGMToolkit::CommandSetPrize(eGMToolkitStatus status)
{
	eGMToolResult result = GM_RESULT_OK;
	if (status == GM_TOOLKIT_NORMAL)
	{
		if (m_stringCount == 2)
		{
			if (strcmp("on", m_stringBuffer[1]) == 0)
			{
				m_bPrizeSwitch = true;
				Display(K2L_Compatibility("SetPrize \xb1\xe2\xb4\xc9 \xc8\xb0\xbc\xba\xc8\xad."),
					0xc6ff85);
			}
			else if (strcmp("off", m_stringBuffer[1]) == 0)
			{
				m_bPrizeSwitch = false;
				Display(
					K2L_Compatibility("SetPrize \xb1\xe2\xb4\xc9 \xba\xf1\xc8\xb0\xbc\xba\xc8\xad."),
					0xc6ff85);
			}
			else
				result = GM_RESULT_ARGUMENT_ERROR;
		}
		else if (m_stringCount >= 3)
		{
			sPrize* prize = new sPrize;
			char name[64] = "";
			int index = 0;
			bool valid = true;
			if (strcmp(m_stringBuffer[1], "speeder") == 0)
			{
				prize->type = 1;
				index = 2;
			}
			else if (strcmp(m_stringBuffer[1], "runner") == 0)
			{
				prize->type = 2;
				index = 2;
			}
			else if (strcmp(m_stringBuffer[1], "chipin") == 0)
			{
				prize->type = 3;
				index = 2;
			}
			else if (strcmp(m_stringBuffer[1], "longputt") == 0)
			{
				prize->type = 4;
				index = 2;
			}
			else if (strcmp(m_stringBuffer[1], "recovery") == 0)
			{
				prize->type = 5;
				index = 2;
			}
			else if (strcmp(m_stringBuffer[1], "lucky") == 0)
			{
				prize->type = 6;
				index = 2;
			}
			else
			{
				char number[16] = "";
				int start = atoi(m_stringBuffer[1]);
				memset(number, 0, sizeof(number));
				_itoa(start, number, 10);
				if (strlen(number) == strlen(m_stringBuffer[1]) && start > 0 &&
					start < 301)
				{
					prize->start = start;
					if (strcmp(m_stringBuffer[2], "to") == 0)
					{
						int end = atoi(m_stringBuffer[3]);
						memset(number, 0, sizeof(number));
						_itoa(end, number, 10);
						if (strlen(number) == strlen(m_stringBuffer[3]) &&
							m_stringCount >= 5 && end > 0 && end < 301)
						{
							prize->end = end;
							index = 4;
							prize->type = 0;
						}
						else
						{
							result = GM_RESULT_ARGUMENT_ERROR;
							valid = false;
						}
					}
					else
					{
						prize->end = start;
						index = 2;
						prize->type = 0;
					}
				}
				else
				{
					result = GM_RESULT_ARGUMENT_ERROR;
					valid = false;
				}
			}

			if (index <= m_stringCount && valid)
			{
				unsigned long typeId = atoi(m_stringBuffer[index]);
				if (FindItem(typeId, name))
				{
					prize->itemId = typeId;
					if (index + 1 < m_stringCount)
					{
						int count = atoi(m_stringBuffer[index + 1]);
						if (count > 0)
						{
							prize->count = count;
							m_confirmWaitTag = GM_CMD_SETPRIZE;
							m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
						}
						else
							result = GM_RESULT_ARGUMENT_ERROR;
					}
					else
					{
						prize->count = 1;
						m_confirmWaitTag = GM_CMD_SETPRIZE;
						m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
					}

					if (m_toolkitStatus == GM_TOOLKIT_CONFIRM_WAIT)
					{
						char buffer[128];
						if (prize->type == 0)
						{
							unsigned int end = prize->end;
							unsigned int start = prize->start;
							memset(buffer, 0, sizeof(buffer));
							if (end == start)
								sprintf(buffer,
									K2L_Compatibility("\xb9\xfc   \xc0\xa7 : %d\xb5\xee"), start);
							else
								sprintf(buffer,
									K2L_Compatibility("\xb9\xfc   \xc0\xa7 : %d\xb5\xee \xba\xce\xc5\xcd %d\xb5\xee \xb1\xee\xc1\xf6 "),
									start, end);

							Display(buffer, 0xc6ff85);
							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb : %s"),
								name);
							Display(buffer, 0xc6ff85);
							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, K2L_Compatibility("\xbc\xf6   \xb7\xae : %d\xb0\xb3"),
								prize->count);
						}
						else
						{
							char range[32] = "";
							switch (prize->type)
							{
							case 1:
								COPY_STRING(range,
									K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xbd\xba\xc7\xc7\xb4\xf5"));
								break;
							case 2:
								COPY_STRING(range,
									K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb7\xaf\xb3\xca"));
								break;
							case 3:
								COPY_STRING(range,
									K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xc4\xa8\xc0\xce"));
								break;
							case 4:
								COPY_STRING(range,
									K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb7\xd5\xc6\xdb\xc5\xcd"));
								break;
							case 5:
								COPY_STRING(range,
									K2L_Compatibility("\xba\xa3\xbd\xba\xc6\xae \xb8\xae\xc4\xbf\xb9\xf6\xb8\xae"));
								break;
							case 6:
								COPY_STRING(range, K2L_Compatibility("\xc7\xe0\xbf\xee\xbb\xf3"));
								break;
							}

							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, K2L_Compatibility("\xb9\xfc   \xc0\xa7 : %s"), range);
							Display(buffer, 0xc6ff85);
							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb : %s"),
								name);
							Display(buffer, 0xc6ff85);
							memset(buffer, 0, sizeof(buffer));
							sprintf(buffer, K2L_Compatibility("\xbc\xf6   \xb7\xae : %d\xb0\xb3"),
								prize->count);
						}

						Display(buffer, 0xc6ff85);
						sprintf(buffer,
							K2L_Compatibility("\xbc\xb3\xc1\xa4\xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee?. [/Y, /N]"));
						Display(buffer, 0xc6ff85);
						m_pPrize = prize;
					}
				}
				else
					result = GM_RESULT_ARGUMENT_ERROR;
			}
			else
				result = GM_RESULT_ARGUMENT_ERROR;
		}
		else
			result = GM_RESULT_ARGUMENT_ERROR;
	}
	else
	{
		if (Confirm())
		{
			if (m_pPrize)
			{
				m_prizeList.push_back(m_pPrize);
				m_pPrize = 0;
			}

			Display(
				K2L_Compatibility("\xbd\xc3\xbb\xf3\xc7\xb0\xb8\xf1 \xb8\xae\xbd\xba\xc6\xae \xbc\xb3\xc1\xa4\xbf\xcf\xb7\xe1."),
				0xc6ff85);
		}
		else
		{
			if (m_pPrize)
			{
				delete m_pPrize;
				m_pPrize = 0;
			}

			Display(
				K2L_Compatibility("'setprize' \xb8\xed\xb7\xc9\xc0\xcc \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xc6ff85);
		}

		m_toolkitStatus = GM_TOOLKIT_NORMAL;
	}

	return result;
}


eGMToolResult CGMToolkit::CommandGiveItem(eGMToolkitStatus status)
{
	if (status != GM_TOOLKIT_NORMAL)
	{
		if (!Confirm())
		{
			Display(
				K2L_Compatibility("'giveitem' \xb8\xed\xb7\xc9\xc0\xcc \xc3\xeb\xbc\xd2 \xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				0xc6ff85);
			m_toolkitStatus = GM_TOOLKIT_NORMAL;
			return GM_RESULT_OK;
		}

		Display(
			K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xc0\xcc \xc1\xf6\xb1\xde\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xc6ff85);
		m_toolkitStatus = GM_TOOLKIT_NORMAL;
		return GM_RESULT_OK;
	}

	if (m_stringCount < 2)
		return GM_RESULT_ARGUMENT_ERROR;

	char name[64] = "\0";
	std::map<unsigned long, sBriefUserInfo>& users =
		CSharedDoc::Instance()->m_briefUserInfoMap;
	std::map<unsigned long, sBriefUserInfo>::iterator it = users.begin();
	for (; it != users.end(); ++it)
	{
		sBriefUserInfo info = it->second;
		if (strcmp(info.sNick, m_stringBuffer[1]) == 0)
			break;
	}

	sBriefUserInfo info = it->second;
	const char* message;
	if (it == users.end())
	{
		message =
			K2L_Compatibility("\xc7\xf6\xc0\xe7 \xbc\xad\xb9\xf6\xbf\xa1 \xc1\xa2\xbc\xd3\xc7\xcf\xb0\xed \xc0\xd6\xc1\xf6 \xbe\xca\xb0\xc5\xb3\xaa \xbe\xf8\xb4\xc2 \xc0\xaf\xc0\xfa\xc0\xd4\xb4\xcf\xb4\xd9.");
	}
	else
	{
		unsigned long count;
		if (m_stringCount == 3)
		{
			count = 1;
		}
		else if (m_stringCount < 4 || (count = atoi(m_stringBuffer[3])) == 0)
		{
			return GM_RESULT_ARGUMENT_ERROR;
		}

		unsigned long typeId = atoi(m_stringBuffer[2]);
		if (FindItem(typeId, name))
		{
			if (m_pConfirmPacket)
				return GM_RESULT_SYNTAX_ERROR;

			m_pConfirmPacket = new WSendPacket(0x8f);
			m_pConfirmPacket->Encode2(0x12);
			m_pConfirmPacket->Encode4(info.dwGuid);
			m_pConfirmPacket->Encode4(typeId);
			m_pConfirmPacket->Encode4(count);
			char buffer[128] = "\0";
			sprintf(buffer,
				K2L_Compatibility("\xc5\xb8\xc0\xd4ID : %d, \xbe\xc6\xc0\xcc\xc5\xdb : %s, \xbc\xf6\xb7\xae : %d"),
				typeId, name, count);
			Display(buffer, 0xc6ff85);
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,
				K2L_Compatibility("'%s' \xb4\xd4\xbf\xa1\xb0\xd4 \xc0\xa7\xc0\xc7 \xbe\xc6\xc0\xcc\xc5\xdb\xc0\xbb \xc1\xf6\xb1\xde \xc7\xcf\xbd\xc3\xb0\xda\xbd\xc0\xb4\xcf\xb1\xee? [/Y, /N]"),
				m_stringBuffer[1]);
			Display(buffer, 0xc6ff85);
			m_confirmWaitTag = GM_CMD_GIVEITEM;
			m_toolkitStatus = GM_TOOLKIT_CONFIRM_WAIT;
			return GM_RESULT_OK;
		}

		message =
			"\xc0\xdf\xb8\xf8\xb5\xc8 \xbe\xc6\xc0\xcc\xc5\xdb TypeID \xc0\xd4\xb4\xcf\xb4\xd9.";
	}

	Display(message, 0xff7802);
	return GM_RESULT_OK;
}

eGMToolResult CGMToolkit::CommandFindItem(eGMToolkitStatus status)
{
	if (m_stringCount < 3)
		return GM_RESULT_ARGUMENT_ERROR;

	CItemManager& items = CSharedDoc::Instance()->m_itemManager;
	bool found;
	if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb")) == 0)
	{
		found = FindItem<IFF_STRUCT::sItem>(m_stringBuffer[2], items.m_ItemMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xc4\xab\xb5\xe5")) == 0)
	{
		found = FindItem<IFF_STRUCT::sCard>(m_stringBuffer[2], items.m_CardMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xc6\xc4\xc3\xf7")) == 0)
	{
		found = FindItem<IFF_STRUCT::sPart>(m_stringBuffer[2], items.m_PartMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xba\xb8\xc1\xb6\xc6\xc4\xc3\xf7")) == 0)
	{
		found = FindItem<IFF_STRUCT::sAuxPart>(m_stringBuffer[2],
			items.m_AuxPartMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xc5\xac\xb7\xb4")) == 0)
	{
		found = FindItem<IFF_STRUCT::sClubSet>(m_stringBuffer[2],
			items.m_ClubSetMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xbe\xc6\xc1\xee\xc5\xd8")) == 0)
	{
		found = FindItem<IFF_STRUCT::sBall>(m_stringBuffer[2], items.m_BallMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xbc\xbc\xc6\xae")) == 0)
	{
		found = FindItem<IFF_STRUCT::sSetItem>(m_stringBuffer[2],
			items.m_SetItemMap);
	}
	else if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xb0\xa1\xb1\xb8")) == 0)
	{
		found = FindItem<IFF_STRUCT::sFurniture>(m_stringBuffer[2],
			items.m_FurnitureMap);
	}
	else
	{
		if (strcmp(m_stringBuffer[1], K2L_Compatibility("\xb8\xf0\xb5\xce")) != 0)
			return GM_RESULT_ARGUMENT_ERROR;

		bool item =
			FindItem<IFF_STRUCT::sItem>(m_stringBuffer[2], items.m_ItemMap);
		bool card =
			FindItem<IFF_STRUCT::sCard>(m_stringBuffer[2], items.m_CardMap);
		bool part =
			FindItem<IFF_STRUCT::sPart>(m_stringBuffer[2], items.m_PartMap);
		bool auxPart = FindItem<IFF_STRUCT::sAuxPart>(m_stringBuffer[2],
			items.m_AuxPartMap);
		bool clubSet = FindItem<IFF_STRUCT::sClubSet>(m_stringBuffer[2],
			items.m_ClubSetMap);
		bool ball =
			FindItem<IFF_STRUCT::sBall>(m_stringBuffer[2], items.m_BallMap);
		bool setItem = FindItem<IFF_STRUCT::sSetItem>(m_stringBuffer[2],
			items.m_SetItemMap);
		found = FindItem<IFF_STRUCT::sFurniture>(m_stringBuffer[2],
			items.m_FurnitureMap);
		found = item || card || part || auxPart || clubSet || ball || setItem ||
			found;
	}

	if (!found)
		Display(K2L_Compatibility("\xc3\xa3\xc0\xbb \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
			0xff7802);

	return GM_RESULT_OK;
}
