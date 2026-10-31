#pragma once

#include "wlocalize.h"

static const char* NICK_ERR_EMPTY = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xc0\xbb \xc0\xd4\xb7\xc2\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4. ");
static const char* NICK_ERR_SIZE = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xc0\xba \\c0xffff0000\\c\xbf\xb5\xb9\xae 4 - 16 \xb1\xdb\xc0\xda\\c0xff000000\\c \xb6\xc7\xb4\xc2 \\c0xffff0000\\c\xc7\xd1\xb1\xdb 2 - 8\xb1\xdb\xc0\xda\\c0xff000000\\c\xc0\xc7 \xb1\xe6\xc0\xcc\xb8\xa6 \xb0\xa1\xc1\xae\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9.");
static const char* NICK_ERR_SPACE = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xc0\xba \\c0xffff0000\\c\xb0\xf8\xb9\xe9\xb9\xae\xc0\xda\\c0xff000000\\c\xb8\xa6 \xc6\xf7\xc7\xd4\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.");
static const char* NICK_ERR_INVALID_CHAR = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xbf\xa1\xbc\xad \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb9\xae\xc0\xda\xb8\xa6 \xc6\xf7\xc7\xd4\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");
static const char* NICK_ERR_RESTRICTED_WORD = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xc0\xb8\xb7\xce \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb4\xdc\xbe\xee\xc0\xd4\xb4\xcf\xb4\xd9.");
static const char* NICK_ERR_HAS_BAD_WORD = K2L_Compatibility(
	"\xb4\xeb\xc8\xad\xb8\xed\xc0\xb8\xb7\xce \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xb4\xc2 \xb4\xdc\xbe\xee\xb8\xa6 \xc6\xf7\xc7\xd4\xc7\xcf\xb0\xed \xc0\xd6\xbd\xc0\xb4\xcf\xb4\xd9.");

struct sChatCmd
{
	char name[20];
};

extern sChatCmd ChatCmd[];

class ChatManager
{
public:
	ChatManager();
	virtual ~ChatManager();

	bool Load();
	void ParseString(const char* str);
	void CheckPenaltyTime();
	void SendChatMessage(const char* msg, const char* target, bool bTeam);
	void SendChatMessage2(const char* msg, const char* target, bool bTeam);
	bool Filtering(char* str);
	int GetFaceIndex(const char* str);
	const char* GetMotionName(const char* str, unsigned long uid);
	void FilteringHack(const char* str, bool bChat);
	const char* FilteringNick(char* nick);
	int FilteringGuildString(const char* str, int minLen, int maxLen,
		bool bOption);
	const char* FilteringEmoticon(unsigned short* str);
	int CheckChatCommand(const char* str, int type);
	int SaveChatHistory(int from, int to);
	void Display(const char* str, int type);

private:
	bool LoadChatFilter();
	bool LoadNickFilter();
	void RecordTournament();
	const char* GetAlignedString(const char* str, int len, int width);
	void RecordMyChatHistory(const char* str);

public:
	// 983: tabela de emoticons 15 x 19 x 32 B em 00e2dd40; contagens por grupo em this+4
	int GetEmoEntry983(const int& group, const int& idx); // 009a3c00
	int GetEmoEntry983B(const int& group, const int& idx); // 009a3c40
	int GetEmoCount983(const int& group); // 009a3c80
	void InitEmoCount983(); // 009a3ca0

private:

	struct sNickFilter
	{
		char type;
		char word[30];
	};

	int m_emoCount[15];
	char (*m_chatFilter)[30];
	sNickFilter* m_nickFilter;
	int m_chatFilterNum;
	int m_nickFilterNum;
	unsigned long m_lastChatTime;
	int m_swearCount;
	int m_floodCount;
	bool m_bSwearPenalty;
	bool m_bFloodPenalty;
	bool m_bUnused;
	unsigned char m_reserved[0x15];
#ifndef REBANG_SEMANTIC_ONLY
	friend struct
		RbChatManagerFunclets; // HACK: holders of the nakedify EH funclets (hatmanager.cpp)
#endif
};
