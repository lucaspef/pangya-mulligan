#include "minatl.h"
// HACK: declared before hatmanager.h (FacialTextTable): the .data order of the RB_DEFER_INIT tables
// is the reverse of their first declaration.
namespace
{
	extern const char* MotionNameTable[10][15][8];
}
#include "hatmanager.h"
#include "jp/levelnametable.h"
#include "shareddoc.h"
#include "commonutil.h"
#include "xunzip.h"
#include "projectg.h"
#include "taskmanager.h"
#include "actor.h"
#include "chatmsg.h"
#include "gmtoolkit.h"
#include "packet.h"
#include "golfdoc.h"
#include "avatar_task.h"
#include "messengerdlg.h"
#include "networksystem.h"
#include "rtti.h"
#include "frwndmanager.h"
#include "fremoticon.h"
#include "fresh.h"
#include "jp/docofs983.h" // D885: crus do CSharedDoc ancorados no membro (VS 2022)
#include "vc10_layout.h"
#include "../shared/sharedtables.h"
#include "guildactor.h"
extern Fresh* g_pFresh;
#include <algorithm>
#include <wchar.h>
#include <ctype.h>

namespace BABO_STR
{
	bool __fastcall WStrStr(char* out, unsigned short* str, int strLen,
		unsigned short* key, int keyLen, bool bCheckOnly);
	bool __fastcall WStrStr_JP(char* out, unsigned short* str, int strLen,
		unsigned short* key, int keyLen, bool bCheckOnly);
	bool __fastcall WStrCmp(unsigned short* a, int aLen, unsigned short* b,
		int bLen);
	bool __fastcall WStrCmp_JP(unsigned short* a, int aLen, unsigned short* b,
		int bLen);
}

#include <direct.h> // _mkdir (C3861)
namespace BABO_STR
{
	inline bool WStrStr(char* out, wchar_t* str, int strLen, wchar_t* key, int keyLen, bool bCheckOnly)
	{
		return WStrStr(out, (unsigned short*)str, strLen, (unsigned short*)key, keyLen, bCheckOnly);
	}
	inline bool WStrCmp(wchar_t* a, int aLen, wchar_t* b, int bLen)
	{
		return WStrCmp((unsigned short*)a, aLen, (unsigned short*)b, bLen);
	}
}

extern unsigned short FakeTable[28];
extern const char* FacialTextTable[4][12];



extern "C" __declspec(dllimport) int __cdecl mkdir(const char*);
extern "C" __declspec(dllimport) int __cdecl _access(const char*, int);

unsigned short FakeTable[28] = { ' ', '`', '~', '!', '@', '#', '$', '%', '^',
	'&', '*', '(', ')', '-', '_', '=', '+', '|', ';', ':', '\'', '"', ',', '<',
	'.', '>', '/', '?' };
sChatCmd ChatCmd[3] = {
	{ "identity" },
	{ "notice" },
	{ "itemdrop" },
};

// HACK: RB_DEFER_INIT makes the front end treat the initializer as dynamic (c2 folds it back to static
// data): the original emits these tables' literals after every function (end of the TU) and places the
// tables at the end of .data. Motion before Facial gives the original literal and .data order.
#define RB_DEFER_INIT(s) (1 ? (s) : 0)
// 983 00ce69c0: as 36 entradas passam por K2L_Compatibility no inicializador (tabela 00f004c0, lida por GetFaceIndex 009a3930)
const char* FacialTextTable[4][12] = {
	{ K2L_Compatibility("^^"), K2L_Compatibility("\xa4\xbb\xa4\xbb"), K2L_Compatibility("\xa4\xbe\xa4\xbe"),
     K2L_Compatibility("\xc7\xcf\xc7\xcf"), K2L_Compatibility("\xc8\xa3\xc8\xa3"), K2L_Compatibility("\xbb\xe7\xb6\xfb"), K2L_Compatibility("\xa4\xa3"),
     K2L_Compatibility("\xb0\xed\xb8\xb6\xbf\xf6"), K2L_Compatibility("\xb1\xe8\xc4\xa1"), K2L_Compatibility("\xb0\xa8\xbb\xe7"),
     K2L_Compatibility("\xc7\xcf\xc0\xcc"), K2L_Compatibility("\xa4\xbe\xa4\xb7") },
	{ K2L_Compatibility("\xa4\xd0.\xa4\xd0"), K2L_Compatibility("\xa4\xd0\xa4\xd0"), K2L_Compatibility("\xc8\xc7\xc2\xbd"), K2L_Compatibility("OTL"), K2L_Compatibility("T_T"),
     K2L_Compatibility("\xc0\xe1\xbc\xf6"), K2L_Compatibility("\xbf\xec\xbf\xef"), K2L_Compatibility("\xc2\xa1\xc2\xa1"), K2L_Compatibility("orz"),
     K2L_Compatibility("\xc8\xe6\xc8\xe6"), K2L_Compatibility("\xa4\xcc\xa4\xcc"), K2L_Compatibility("\xa4\xcc.\xa4\xcc") },
	{ K2L_Compatibility("\x94\xee"), K2L_Compatibility("\x94\xe6"), K2L_Compatibility("-_-+"), K2L_Compatibility("\xb9\xf6\xb7\xb0"), K2L_Compatibility("\xc7\xe6"),
     K2L_Compatibility("\xb0\xad\xc5\xf0"), K2L_Compatibility("\xb9\xcc\xbf\xf6"), K2L_Compatibility("\xc2\xaf\xb3\xaa"),
     K2L_Compatibility("\xc2\xa5\xc1\xf5"), K2L_Compatibility("\xa4\xd1\xa4\xd1+"), K2L_Compatibility("\xc8\xad\xb3\xb2"),
     K2L_Compatibility("\xa4\xd1\xa4\xd1^") },
	{ 0 },
};

namespace
{
	const char* MotionNameTable[10][15][8] = {
		{
         { RB_DEFER_INIT(
				  "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee"),
				"\xc1\xc2\xc0\xfd", "\xc0\xfd\xb8\xc1" },
         { "1\xb5\xee\xb8\xf0\xbc\xc7", "\xa4\xbb\xa4\xbb",
				"\xa4\xbe\xa4\xbe", "\xc7\xcf\xc7\xcf",
				"\xc0\xdf\xb3\xad\xc3\xbc" },
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "OTL", "orz" },
         { "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xbd\xc7\xb8\xc1" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xb3\xaa\xc0\xcc\xbd\xba",
				"nice" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "\xbe\xee\xc0\xcc\xc4\xed",
				"\xb2\xca\xb4\xe7", "\xc0\xb8\xbe\xc7" },
         { "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xb8\xb8\xbc\xbc" },
         { "chat_\xb9\xda\xbc\xf6", "\xb9\xda\xbc\xf6" },
         { "chat_\xc0\xce\xbb\xe7", "\xc0\xce\xbb\xe7" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc2\xa5\xc1\xf5" },
         { "1\xb5\xee\xb8\xf0\xbc\xc7", "\xb8\xb8\xbc\xbc" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xc8\xaf\xc8\xa3" },
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "\xbe\xfb\xbe\xfb" },
         { "chat_\xbb\xdf\xc1\xfc", "\xc8\xef", "\xbb\xdf\xc1\xfc" },
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xbe\xee\xb8\xd3" },
         { "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc8\xe6\xc8\xe6", "\xbf\xef\xc0\xbd" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xb3\xaa\xc0\xcc\xbd\xba",
				"nice" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "OTL", "orz",
				"\xc1\xc2\xc0\xfd", "\xc0\xfd\xb8\xc1", "\xc7\xae\xbd\xe2",
				"\xc5\xd0\xbd\xe2" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xba\xf1\xc6\xb2" },
         { "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xc1\xc1\xbe\xc6", "\xc3\xe3" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xbf\xc0\xba\xfc" },
         { "1\xb5\xee\xb8\xf0\xbc\xc7", "\xb8\xb8\xbc\xbc" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xbf\xb9\xbd\xba", "Yes" },
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xc8\xaf\xc8\xa3", "\xbd\xc2\xb8\xae" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xb3\xaa\xc0\xcc\xbd\xba",
				"nice" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "\xc4\xe2\xb4\xe7",
				"\xbe\xee\xc0\xcc\xc4\xed", "\xc7\xae\xbd\xe2",
				"\xc5\xd0\xbd\xe2" },
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc1\xa8\xc0\xe5", "\xc8\xad\xb3\xb2", "\xba\xd0\xb3\xeb" },
         { "chat_\xb8\xd3\xbe\xb3", "\xb8\xd3\xbe\xb3", "\xc0\xb8\xc0\xbd",
				"..." },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xb8\xf4\xb6\xf3" },
         { "\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xbe\xc8\xb3\xe7", "\xa4\xbe\xa4\xb7", "\xc7\xcf\xc0\xcc" },
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xbf\xb9\xbd\xba", "Yes" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xb3\xaa\xc0\xcc\xbd\xba",
				"nice" },
         { "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xb8\xb8\xbc\xbc" },
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "\xc5\xd0\xbd\xe2",
				"\xc7\xae\xbd\xe2" },
         { "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xbd\xc7\xb8\xc1" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "\xbb\xdf\xc1\xfc",
				"\xc8\xef" },
         { "chat_\xc8\xa3\xc8\xa3", "\xc8\xa3\xc8\xa3" },
         { "chat_\xba\xea\xc0\xcc", "\xba\xea\xc0\xcc", "V" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "1\xb5\xee\xb8\xf0\xbc\xc7", "Yes", "\xbf\xb9\xbd\xba" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xc0\xce\xbb\xe7",
				"\xb0\xa8\xbb\xe7" },
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc1\xc2\xc0\xfd" },
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", "\xc8\xca" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc7\xe3\xb9\xab", "\xc7\xe6" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xb3\xaa\xc0\xcc\xbd\xba",
				"nice" },
         { "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xbd\xc7\xb8\xc1" },
         { "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xc5\xd7\xb4\xcf\xbd\xba" },
         { "chat_\xc2\xf7\xc0\xce", "\xc2\xf7\xc0\xce" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xb8\xd3\xb8\xae" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7"
			  "2",
				"\xb8\xb8\xbc\xbc" },
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "\xbf\xd5\xb5\xfb" },
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", "\xc3\xe3" },
         { "\xbf\xec\xb5\xe5\xbc\xa6\xc8\xc4\xb8\xf0\xbc\xc7",
				"\xb8\xc1\xbf\xf8\xb0\xe6", "\xbd\xba\xc5\xe4\xc4\xbf" },
         { "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xc1\xc2\xc0\xfd", "OTL", "orz" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xb4\xeb\xb1\xe2",
				"\xb3\xaa\xc0\xcc\xbd\xba", "nice" },
         { "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", "\xbf\xb9\xbd\xba", "Yes" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "\xc5\xd0\xbd\xe2",
				"\xc7\xae\xbd\xe2" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xbd\xc7\xb8\xc1" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xb8\xb8\xbc\xbc" },
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xb4\xe4\xb4\xe4", "\xc7\xd1\xbc\xfb" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xb8\xf4\xb6\xf3", "\xb9\xce\xb8\xc1" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xbd\xc2\xb8\xae",
				"\xbf\xb9\xbd\xba", "Yes" },
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "\xc5\xd0\xbd\xe2",
				"\xc7\xae\xbd\xe2" },
         { "\xc6\xdb\xc6\xc3\xc8\xc4\xb4\xeb\xb1\xe2", "\xc7\xcf\xc0\xcc",
				"\xa4\xbe\xa4\xb7" },
         { "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee",
				"\xba\xf9\xb1\xdb" },
         { "chat_\xc5\xb0\xbd\xba", "\xc5\xb0\xbd\xba", "\xc2\xca" },
         { "chat_\xbd\xc7\xb8\xc1", "\xbd\xc7\xb8\xc1", "\xbb\xdf\xc1\xfc" },
         { "chat_\xbd\xc8\xbe\xee", "\xbd\xc8\xbe\xee", "\xb8\xf4\xb6\xf3" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "3\xb5\xee\xb8\xf0\xbc\xc7", "\xc5\xd0\xbd\xe2",
				"\xc7\xae\xbd\xe2" },
         { "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee",
				"\xba\xd0\xb3\xeb", "\xbd\xc7\xb8\xc1" },
         { "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "\xbe\xc8\xb5\xc5",
				"\xc1\xc2\xc0\xfd" },
         { "2\xb5\xee\xb8\xf0\xbc\xc7", "\xbf\xb9\xbd\xba", "Yes" },
         { "chat_\xb8\xb8\xbc\xbc", "\xb8\xb8\xbc\xbc" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { "chat_\xbd\xc7\xb8\xc1", "\xbd\xc7\xb8\xc1" },
         { "chat_\xc8\xad\xb3\xb2", "\xc8\xad\xb3\xb2" },
         { "chat_\xc7\xcf\xc6\xae", "\xc7\xcf\xc6\xae" },
         { "chat_\xc3\xe3", "\xc3\xe3" },
         { "chat_\xc7\xcf\xc0\xcc", "\xc7\xcf\xc0\xcc" },
         { "chat_\xb8\xb8\xbc\xbc", "\xb8\xb8\xbc\xbc" },
         { "chat_\xb3\xaa\xc0\xcc\xbd\xba", "\xb3\xaa\xc0\xcc\xbd\xba" },
         { "chat_\xbe\xc8\xb3\xe7", "\xbe\xc8\xb3\xe7" },
         { "chat_\xbe\xfb\xbe\xfb", "\xbe\xfb\xbe\xfb" },
         { "chat_\xc5\xb0\xbd\xba", "\xc5\xb0\xbd\xba" },
         { "chat_\xba\xce\xb2\xf4", "\xba\xce\xb2\xf4" },
         { "chat_\xb4\xed\xbd\xba", "\xb4\xed\xbd\xba" },
         { 0 },
         { 0 },
         { 0 },
		 },
		{
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
         { 0 },
		 },
	};
}

// 983: inline, chama msvcr100!tolower direto (0098c1b0 do mapa era DeleteFile)
static __forceinline unsigned short TryToLower(unsigned short ch)
{
	return (unsigned short)tolower(ch);
}

static __forceinline bool IsFakeChar(unsigned short ch)
{
	int j;
	for (j = 0; j < 28; j++)
	{
		if (ch == FakeTable[j])
		{
			break;
		}
	}
	return j != 28;
}

static __forceinline bool EqualJP(unsigned short a, unsigned short b)
{
	if (a < 0x80 && b < 0x80)
	{
		return tolower(a) == tolower(b);
	}
	return a == b;
}

bool __fastcall BABO_STR::WStrStr(char* out, unsigned short* str, int strLen,
	unsigned short* key, int keyLen, bool bCheckOnly)
{
	bool found = false;
	int n = strLen - 1;
	for (int i = 0; i < n; i++)
	{
		if (TryToLower(str[i]) == TryToLower(key[0]))
		{
			unsigned short* p = &str[i + 1];
			unsigned short* k = &key[1];
			while (*p != 0 && *k != 0)
			{
				if (TryToLower(*p) == TryToLower(*k))
				{
					k++;
				}
				else
				{
					int j;
					for (j = 0; j < 28; j++)
					{
						if (*p == FakeTable[j])
						{
							break;
						}
					}
					if (j == 28)
					{
						break;
					}
				}
				p++;
			}
			if (k - key == keyLen - 1)
			{
				if (bCheckOnly)
				{
					return true;
				}
				found = true;
				int cnt = p - &str[i];
				unsigned short* q = &str[i];
				for (; cnt > 0; cnt--, q++)
				{
					int j;
					for (j = 0; j < 28; j++)
					{
						if (*q == FakeTable[j])
						{
							break;
						}
					}
					if (j == 28)
					{
						*q = 0x2665;
					}
				}
			}
		}
	}
	if (found)
	{
		WideCharToMultiByte(GAME_CODEPAGE, 0, (LPCWSTR)str, -1, out, 0x400, NULL, NULL);
	}
	return found;
}

bool __fastcall BABO_STR::WStrCmp(unsigned short* a, int aLen,
	unsigned short* b, int bLen)
{
	if (aLen != bLen)
	{
		return true;
	}
	for (int i = 0; i < aLen - 1; i++)
	{
		if (tolower(a[i]) != tolower(b[i]))
		{
			return true;
		}
	}
	return false;
}

bool __fastcall BABO_STR::WStrCmp_JP(unsigned short* a, int aLen,
	unsigned short* b, int bLen)
{
	if (aLen != bLen)
	{
		return true;
	}
	for (int i = 0; i < aLen - 1; i++, a++)
	{
		if (*a < 0x80 && *b < 0x80)
		{
			if (tolower(*a) != tolower(b[i]))
			{
				return true;
			}
		}
		else if (*a != b[i])
		{
			return true;
		}
	}
	return false;
}

bool __fastcall BABO_STR::WStrStr_JP(char* out, unsigned short* str, int strLen,
	unsigned short* key, int keyLen, bool bCheckOnly)
{
	bool found = false;
	WCHAR wrep[32];
	for (int i = 0; i < strLen - 1; i++)
	{
		if (!EqualJP(str[i], key[0]))
		{
			continue;
		}
		unsigned short* p = &str[i + 1];
		unsigned short* k = &key[1];
		while (*p != 0 && *k != 0)
		{
			if (EqualJP(*p, *k))
			{
				k++;
			}
			else if (!IsFakeChar(*p))
			{
				break;
			}
			p++;
		}
		if (k - key != keyLen - 1)
		{
			continue;
		}
		if (bCheckOnly)
		{
			return true;
		}
		found = true;
		static const char* s_rep = K2L_Compatibility("\xa2\xbe");
		int wl =
			MultiByteToWideChar(GAME_CODEPAGE, 0, s_rep, (int)strlen(s_rep) + 1, wrep, 32);
		unsigned short* q = &str[i];
		for (int cnt = p - q; cnt > 0; cnt--, q++)
		{
			if (!IsFakeChar(*q))
			{
				*q = (!s_rep || wl == 0) ? 0x2665 : wrep[0];
			}
		}
	}
	if (found)
	{
		WideCharToMultiByte(GAME_CODEPAGE, 0, (LPCWSTR)str, -1, out, 0x400, NULL, NULL);
	}
	return found;
}

ChatManager::ChatManager()
{
	m_swearCount = 0;
	m_floodCount = 0;
	m_lastChatTime = g_CurrentTime;
	m_bSwearPenalty = false;
	m_bFloodPenalty = false;
	m_chatFilter = NULL;
	m_nickFilter = NULL;
	m_chatFilterNum = 0;
	m_nickFilterNum = 0;
	m_bUnused = false;
	InitEmoCount983();
}

// 983 00e2dd40: 15 personagens x 19 entradas de 8 textos (animacao, apelidos do chat); o 983 a preenche no
// inicializador estatico 00ce6bf0 (K2L nos apelidos coreanos). Sem ela a lista de emocoes do lobby so tem Basico/(aleatorio).
static const char* s_emoTable983[15][19][8] = {
	{ // 0
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc1\xc2\xc0\xfd"), K2L_Compatibility("\xc0\xfd\xb8\xc1") },
		{ "1\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xa4\xbb\xa4\xbb"), K2L_Compatibility("\xa4\xbe\xa4\xbe"), K2L_Compatibility("\xc7\xcf\xc7\xcf"), K2L_Compatibility("\xc0\xdf\xb3\xad\xc3\xbc") },
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", "OTL", "orz" },
		{ "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", K2L_Compatibility("\xbe\xee\xc0\xcc\xc4\xed"), K2L_Compatibility("\xb2\xca\xb4\xe7"), K2L_Compatibility("\xc0\xb8\xbe\xc7") },
		{ "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xc0\xce\xbb\xe7", K2L_Compatibility("\xc0\xce\xbb\xe7") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 1
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc2\xa5\xc1\xf5") },
		{ "1\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xc8\xaf\xc8\xa3") },
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xbe\xfb\xbe\xfb") },
		{ "chat_\xbb\xdf\xc1\xfc", K2L_Compatibility("\xc8\xef"), K2L_Compatibility("\xbb\xdf\xc1\xfc") },
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xbe\xee\xb8\xd3") },
		{ "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc8\xe6\xc8\xe6"), K2L_Compatibility("\xbf\xef\xc0\xbd") },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", "OTL", "orz", K2L_Compatibility("\xc1\xc2\xc0\xfd"), K2L_Compatibility("\xc0\xfd\xb8\xc1"), K2L_Compatibility("\xc7\xae\xbd\xe2"), K2L_Compatibility("\xc5\xd0\xbd\xe2") },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xba\xf1\xc6\xb2") },
		{ "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xc1\xc1\xbe\xc6"), K2L_Compatibility("\xc3\xe3") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 2
		{ "\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xbf\xc0\xba\xfc") },
		{ "1\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xbf\xb9\xbd\xba"), "Yes" },
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xc8\xaf\xc8\xa3"), K2L_Compatibility("\xbd\xc2\xb8\xae") },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", K2L_Compatibility("\xc4\xe2\xb4\xe7"), K2L_Compatibility("\xbe\xee\xc0\xcc\xc4\xed"), K2L_Compatibility("\xc7\xae\xbd\xe2"), K2L_Compatibility("\xc5\xd0\xbd\xe2") },
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc1\xa8\xc0\xe5"), K2L_Compatibility("\xc8\xad\xb3\xb2"), K2L_Compatibility("\xba\xd0\xb3\xeb") },
		{ "chat_\xb8\xd3\xbe\xb3", K2L_Compatibility("\xb8\xd3\xbe\xb3"), K2L_Compatibility("\xc0\xb8\xc0\xbd"), "..." },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 3
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xb8\xf4\xb6\xf3") },
		{ "\xbc\xbc\xc0\xcc\xba\xea\xc6\xc4\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xbe\xc8\xb3\xe7"), K2L_Compatibility("\xa4\xbe\xa4\xb7"), K2L_Compatibility("\xc7\xcf\xc0\xcc") },
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xbf\xb9\xbd\xba"), "Yes" },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xc5\xd0\xbd\xe2"), K2L_Compatibility("\xc7\xae\xbd\xe2") },
		{ "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", K2L_Compatibility("\xbb\xdf\xc1\xfc"), K2L_Compatibility("\xc8\xef") },
		{ "chat_\xc8\xa3\xc8\xa3", K2L_Compatibility("\xc8\xa3\xc8\xa3") },
		{ "chat_\xba\xea\xc0\xcc", K2L_Compatibility("\xba\xea\xc0\xcc"), "V" },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 4
		{ "1\xb5\xee\xb8\xf0\xbc\xc7", "Yes", K2L_Compatibility("\xbf\xb9\xbd\xba") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xc0\xce\xbb\xe7"), K2L_Compatibility("\xb0\xa8\xbb\xe7") },
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc1\xc2\xc0\xfd") },
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xc8\xca") },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc7\xe3\xb9\xab"), K2L_Compatibility("\xc7\xe6") },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xc5\xd7\xb4\xcf\xbd\xba") },
		{ "chat_\xc2\xf7\xc0\xce", "NO" },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 5
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xb8\xd3\xb8\xae") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7" "2", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xbf\xd5\xb5\xfb") },
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xc3\xe3") },
		{ "\xbf\xec\xb5\xe5\xbc\xa6\xc8\xc4\xb8\xf0\xbc\xc7", K2L_Compatibility("\xb8\xc1\xbf\xf8\xb0\xe6"), K2L_Compatibility("\xbd\xba\xc5\xe4\xc4\xbf") },
		{ "\xc5\xb8\xc0\xd3\xbf\xc0\xb9\xf6\xba\xa1\xc4\xbfOB\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xc1\xc2\xc0\xfd"), "OTL", "orz" },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xb4\xeb\xb1\xe2", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba"), "nice" },
		{ "\xc6\xdb\xc6\xc3\xbc\xba\xb0\xf8", K2L_Compatibility("\xbf\xb9\xbd\xba"), "Yes" },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", K2L_Compatibility("\xc5\xd0\xbd\xe2"), K2L_Compatibility("\xc7\xae\xbd\xe2") },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 6
		{ "\xb9\xf6\xb5\xf0\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xb4\xe4\xb4\xe4"), K2L_Compatibility("\xc7\xd1\xbc\xfb") },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xb8\xf4\xb6\xf3"), K2L_Compatibility("\xb9\xce\xb8\xc1") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xbd\xc2\xb8\xae"), K2L_Compatibility("\xbf\xb9\xbd\xba"), "Yes" },
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xc5\xd0\xbd\xe2"), K2L_Compatibility("\xc7\xae\xbd\xe2") },
		{ "\xc6\xdb\xc6\xc3\xc8\xc4\xb4\xeb\xb1\xe2", K2L_Compatibility("\xc7\xcf\xc0\xcc"), K2L_Compatibility("\xa4\xbe\xa4\xb7") },
		{ "\xc0\xcc\xb1\xdb\xbd\xc2\xb8\xae\xc6\xf7\xc1\xee", K2L_Compatibility("\xba\xf9\xb1\xdb") },
		{ "chat_\xc5\xb0\xbd\xba", K2L_Compatibility("\xc5\xb0\xbd\xba"), K2L_Compatibility("\xc2\xca") },
		{ "chat_\xbd\xc7\xb8\xc1", K2L_Compatibility("\xbd\xc7\xb8\xc1"), K2L_Compatibility("\xbb\xdf\xc1\xfc") },
		{ "chat_\xbd\xc8\xbe\xee", K2L_Compatibility("\xbd\xc8\xbe\xee") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 7
		{ "3\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xc5\xd0\xbd\xe2"), K2L_Compatibility("\xc7\xae\xbd\xe2") },
		{ "\xba\xb8\xb1\xe2\xbd\xc7\xb0\xdd\xbd\xc7\xb8\xc1\xc6\xf7\xc1\xee", K2L_Compatibility("\xba\xd0\xb3\xeb"), K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "\xc6\xdb\xc6\xc3\xbd\xc7\xc6\xd0", K2L_Compatibility("\xbe\xc8\xb5\xc5"), K2L_Compatibility("\xc1\xc2\xc0\xfd") },
		{ "2\xb5\xee\xb8\xf0\xbc\xc7", K2L_Compatibility("\xbf\xb9\xbd\xba"), "Yes" },
		{ "chat_\xb8\xb8\xbc\xbc", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 8
		{ "chat_\xbd\xc7\xb8\xc1", K2L_Compatibility("\xbd\xc7\xb8\xc1") },
		{ "chat_\xc8\xad\xb3\xb2", K2L_Compatibility("\xc8\xad\xb3\xb2") },
		{ "chat_\xc7\xcf\xc6\xae", K2L_Compatibility("\xc7\xcf\xc6\xae") },
		{ "chat_\xc3\xe3", K2L_Compatibility("\xc3\xe3") },
		{ "chat_\xc7\xcf\xc0\xcc", K2L_Compatibility("\xc7\xcf\xc0\xcc") },
		{ "chat_\xb8\xb8\xbc\xbc", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "chat_\xb3\xaa\xc0\xcc\xbd\xba", K2L_Compatibility("\xb3\xaa\xc0\xcc\xbd\xba") },
		{ "chat_\xbe\xc8\xb3\xe7", K2L_Compatibility("\xbe\xc8\xb3\xe7") },
		{ "chat_\xbe\xfb\xbe\xfb", K2L_Compatibility("\xbe\xfb\xbe\xfb") },
		{ "chat_\xc5\xb0\xbd\xba", K2L_Compatibility("\xc5\xb0\xbd\xba") },
		{ "chat_\xba\xce\xb2\xf4", K2L_Compatibility("\xba\xce\xb2\xf4") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 9
		{ "chat_\xb3\xee\xb6\xf7", K2L_Compatibility("\xb3\xee\xb6\xf7") },
		{ "chat_\xba\xf1\xc6\xb2", K2L_Compatibility("\xba\xf1\xc6\xb2") },
		{ "chat_\xc6\xc4\xc0\xcc\xc6\xc3", K2L_Compatibility("\xc6\xc4\xc0\xcc\xc6\xc3") },
		{ "chat_\xb8\xb8\xbc\xbc", K2L_Compatibility("\xb8\xb8\xbc\xbc") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xc5\xe4\xb3\xa2", K2L_Compatibility("\xc5\xe4\xb3\xa2") },
		{ "chat_\xbe\xc8\xb3\xe7", K2L_Compatibility("\xbe\xc8\xb3\xe7") },
		{ "chat_\xb4\xed\xbd\xba", K2L_Compatibility("\xb4\xed\xbd\xba") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 10
		{ "chat_\xb1\xe2\xbb\xdd", K2L_Compatibility("\xb1\xe2\xbb\xdd") },
		{ "chat_\xb1\xe2\xc1\xf6\xb0\xb3", K2L_Compatibility("\xb1\xe2\xc1\xf6\xb0\xb3") },
		{ "chat_\xb6\xa7\xb8\xae\xb1\xe2", K2L_Compatibility("\xb6\xa7\xb8\xae\xb1\xe2") },
		{ "chat_\xb8\xde\xb7\xd5", K2L_Compatibility("\xb8\xde\xb7\xd5") },
		{ "chat_\xbd\xc5\xb9\xdf", K2L_Compatibility("\xbd\xc5\xb9\xdf") },
		{ "chat_\xb8\xc2\xb1\xe2", K2L_Compatibility("\xb8\xc2\xb1\xe2") },
		{ "chat_\xc0\xce\xbb\xe7", K2L_Compatibility("\xc0\xce\xbb\xe7") },
		{ "chat_\xc1\xd6\xc0\xc7", K2L_Compatibility("\xc1\xd6\xc0\xc7") },
		{ "chat_\xc7\xd1\xbd\xc9", K2L_Compatibility("\xc7\xd1\xbd\xc9") },
		{ "chat_\xba\xf1\xbf\xf4\xb1\xe2", K2L_Compatibility("\xba\xf1\xbf\xf4\xb1\xe2") },
		{ "chat_\xc1\xc2\xc0\xfd", K2L_Compatibility("\xc1\xc2\xc0\xfd") },
		{ "chat_\xba\xd0\xb3\xeb", K2L_Compatibility("\xba\xd0\xb3\xeb") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xb3\xaf\xb1\xe2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
	{ // 11
		{ "chat_\xb1\xe2\xbb\xdd", K2L_Compatibility("\xb1\xe2\xbb\xdd") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xbb\xdf\xc1\xfc", K2L_Compatibility("\xbb\xdf\xc1\xfc") },
		{ "chat_\xba\xd0\xb3\xeb", K2L_Compatibility("\xba\xd0\xb3\xeb") },
		{ "chat_\xbd\xbd\xc7\xc4", K2L_Compatibility("\xbd\xbd\xc7\xc4") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
	},
	{ // 12
		{ "chat_\xb1\xe2\xbb\xdd", K2L_Compatibility("\xb1\xe2\xbb\xdd") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xba\xd0\xb3\xeb", K2L_Compatibility("\xba\xd0\xb3\xeb") },
		{ "chat_\xbb\xdf\xc1\xfc", K2L_Compatibility("\xbb\xdf\xc1\xfc") },
		{ "chat_\xbd\xbd\xc7\xc4", K2L_Compatibility("\xbd\xbd\xc7\xc4") },
		{ "chat_\xc0\xce\xbb\xe7", K2L_Compatibility("\xc0\xce\xbb\xe7") },
		{ "chat_\xbe\xc6\xc0\xcc\xc5\xdb\xb4\xed\xbd\xba", K2L_Compatibility("\xb8\xae\xb5\xeb") },
	},
	{ // 13
		{ "chat_\xb1\xe2\xbb\xdd", K2L_Compatibility("\xb1\xe2\xbb\xdd") },
		{ "chat_\xb9\xda\xbc\xf6", K2L_Compatibility("\xb9\xda\xbc\xf6") },
		{ "chat_\xbb\xdf\xc1\xfc", K2L_Compatibility("\xbb\xdf\xc1\xfc") },
		{ "chat_\xba\xd0\xb3\xeb", K2L_Compatibility("\xba\xd0\xb3\xeb") },
		{ "chat_\xbd\xbd\xc7\xc4", K2L_Compatibility("\xbd\xbd\xc7\xc4") },
		{ "chat_\xc0\xce\xbb\xe7", K2L_Compatibility("\xc0\xce\xbb\xe7") },
	},
	{ // 14
		{ "chat_\xb1\xe2\xbb\xdd", K2L_Compatibility("\xb1\xe2\xbb\xdd") },
		{ "chat_\xb5\xb5\xb9\xdf", K2L_Compatibility("\xb5\xb5\xb9\xdf") },
		{ "chat_\xbd\xbd\xc7\xc4", K2L_Compatibility("\xbd\xbd\xc7\xc4") },
		{ "chat_\xc2\xea\xc2\xea", K2L_Compatibility("\xc2\xea\xc2\xea") },
		{ "chat_\xc8\xad\xb3\xb2", K2L_Compatibility("\xc8\xad\xb3\xb2") },
		{ "chat_\xb3\xaf\xb1\xe2" "2", K2L_Compatibility("\xb3\xaf\xb1\xe2") },
	},
};

int ChatManager::GetEmoEntry983(const int& group, const int& idx)
{
	if (group >= 0 && group < 15 && idx >= 0 && idx < m_emoCount[group])
	{
		return (int)s_emoTable983[group][idx][0];
	}
	return 0;
}

int ChatManager::GetEmoEntry983B(const int& group, const int& idx)
{
	if (group >= 0 && group < 15 && idx >= 0 && idx < m_emoCount[group])
	{
		return (int)s_emoTable983[group][idx][1];
	}
	return 0;
}

int ChatManager::GetEmoCount983(const int& group)
{
	if (group >= 0 && group < 15)
	{
		return m_emoCount[group];
	}
	return 0;
}

void ChatManager::InitEmoCount983()
{
	for (int g = 0; g < 15; g++)
	{
		for (int i = 0; i < 19; i++)
		{
			if (!s_emoTable983[g][i][0])
			{
				m_emoCount[g] = i;
				break;
			}
		}
	}
}

// visão do 983 sobre a mesma tabela (CSharedDoc+0x10c24), usada por avatarmotiondlg/lobbyplayer/lobbyavataractor;
// sem estas definições o link as trocava por stubs (Count 0 -> lista de emoções só com Basic/(random))
class CLobbyMotionTable983
{
public:
	int Count983(const unsigned long* pTid); // 009a3c80
	const char* Get983(const unsigned long* pTid, const int* pIndex); // 009a3c00
	const char* GetName983(const unsigned long* pTid, const int* pIndex); // 009a3c40
};

static int EmoCount983(int group)
{
	int i = 0;
	while (i < 19 && s_emoTable983[group][i][0])
		i++;
	return i;
}

// 009a3c80
int CLobbyMotionTable983::Count983(const unsigned long* pTid)
{
	int group = (int)*pTid;
	if (group >= 0 && group < 15)
		return EmoCount983(group);
	return 0;
}

// 009a3c00
const char* CLobbyMotionTable983::Get983(const unsigned long* pTid, const int* pIndex)
{
	int group = (int)*pTid;
	if (group >= 0 && group < 15 && *pIndex >= 0 && *pIndex < EmoCount983(group))
		return s_emoTable983[group][*pIndex][0];
	return NULL;
}

// 009a3c40
const char* CLobbyMotionTable983::GetName983(const unsigned long* pTid, const int* pIndex)
{
	int group = (int)*pTid;
	if (group >= 0 && group < 15 && *pIndex >= 0 && *pIndex < EmoCount983(group))
		return s_emoTable983[group][*pIndex][1];
	return NULL;
}

ChatManager::~ChatManager()
{
	if (m_chatFilter)
	{
		delete[] m_chatFilter;
		m_chatFilter = NULL;
	}
	if (m_nickFilter)
	{
		delete[] m_nickFilter;
		m_nickFilter = NULL;
	}
}

bool ChatManager::Load()
{
	bool bChat = LoadChatFilter();
	bool bNick = LoadNickFilter();
	bNick |= bChat;
	return bNick;
}

bool ChatManager::LoadChatFilter()
{
	WFile file("chat.bin");
	unsigned int len = file.Length();
	unsigned char* buff = new unsigned char[len];

	file.Read(buff, len);
	file.Close();

	HZIP hz = OpenZipU(buff, len, ZIP_MEMORY);
	if (!hz)
	{
		delete[] buff;
		return false;
	}

	ZIPENTRY ze;
	GetZipItemA(hz, -1, &ze);
	int numItems = ze.index;

	for (int i = 0; i < numItems; i++)
	{
		GetZipItemA(hz, i, &ze);
		unsigned char* pBuffer = new unsigned char[ze.unc_size];
		UnzipItem(hz, i, pBuffer, ze.unc_size, ZIP_MEMORY);

		for (int j = 0; j < (int)ze.unc_size; j++)
		{
			pBuffer[j] = ~pBuffer[j];
		}

		m_chatFilterNum = 0;
		m_chatFilterNum = *(int*)pBuffer;
		if (m_chatFilterNum > 0)
		{
			m_chatFilter = new char[m_chatFilterNum][30];
			memcpy(m_chatFilter, pBuffer + 4, m_chatFilterNum * 30);
		}
		delete[] pBuffer;
	}

	CloseZipU(hz);
	delete[] buff;
	return true;
}

bool ChatManager::LoadNickFilter()
{
	WFile file("nick.bin");
	unsigned int len = file.Length();
	unsigned char* buff = new unsigned char[len];

	file.Read(buff, len);
	file.Close();

	HZIP hz = OpenZipU(buff, len, ZIP_MEMORY);
	if (!hz)
	{
		delete[] buff;
		return false;
	}

	ZIPENTRY ze;
	GetZipItemA(hz, -1, &ze);
	int numItems = ze.index;

	for (int i = 0; i < numItems; i++)
	{
		GetZipItemA(hz, i, &ze);
		unsigned char* pBuffer = new unsigned char[ze.unc_size];
		UnzipItem(hz, i, pBuffer, ze.unc_size, ZIP_MEMORY);

		for (int j = 0; j < (int)ze.unc_size; j++)
		{
			pBuffer[j] = ~pBuffer[j];
		}

		m_nickFilterNum = 0;
		m_nickFilterNum = *(int*)pBuffer;
		if (m_nickFilterNum > 0)
		{
			m_nickFilter = new sNickFilter[m_nickFilterNum];
			memcpy(m_nickFilter, pBuffer + 4, m_nickFilterNum * 31);
		}
		delete[] pBuffer;
	}

	CloseZipU(hz);
	delete[] buff;
	return true;
}

void ChatManager::ParseString(const char* str)
{
}

// ---- CSharedDoc fields used by ChatManager (offsets in the original layout) ----
#define SD_BYTES() ((unsigned char*)CSharedDoc::Instance())
#define SD_U32(off) (*(unsigned int*)(SD_BYTES() + (off)))
#define SD_INT(off) (*(int*)(SD_BYTES() + (off)))
enum
{
	SD_MY_NAME = 0x3f8,
	SD_FLAGS = 0x433,
	SD_CHAT_BLOCK = 0x45f,
	SD_TOURN_BEGIN = 0x1042c, // 983 (645: 0x4508)
	SD_TOURN_END = 0x10430,
	SD_SCENE_STATE = 0x1122c // 983: CSharedDoc::m_playingMode (645: 0x5b9c; s88); D885: usar o membro (cl 14.44 anda)
};
C_ASSERT_VC10(offsetof(CSharedDoc, m_playingMode) == 0x1122c && offsetof(CSharedDoc, m_myInfo) + 2 + offsetof(sPangYaUserInfo, dwIdentity) == 0x812);
#define SD_FLAG_NOFILTER 4
#define LOBBY_COLOR 0xffff7878

extern unsigned char MyIndex();
extern sPlayerData* PLAYER(unsigned char index);
extern const char* GetCharsTillSpace(const char* str, char* out);

// Posts a text message to the Lobby actor of the current task.
#define SEND_LOBBY_MSG(text, color) \
	do \
	{ \
		MsgObject msg(NULL, 3, (int)(text), (color), 0, 0, 0); \
		IActor* lobby = AfxGetTask()->GetActor("Lobby"); \
		if (lobby) \
		{ \
			lobby << msg; \
		} \
	} while (0)

void ChatManager::CheckPenaltyTime()
{
	if ((int)CSharedDoc::Instance()->m_playingMode == 2)
	{
		return;
	}
	sPlayerData* player = NULL;
	if (!IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		player = CSharedDoc::Instance()->m_pGolfDoc->GetPlayer(MyIndex());
	}
	if (OnlinePlay() && player && player->chatBlock &&
		g_CurrentTime - m_lastChatTime > 60000)
	{
		player->chatBlock = 0; // 983 009a4cc0: +0x56c (o 645 lia +0x55c cru)
		WSendPacket packet(0x4f);
		packet.Encode1(0);
		packet.Send((eSendTo)0);
	}
}

// JP 983 (C-PKT lote 18): 004d0b10 -- o objeto global de 00e73e84 guarda em +0x1d8 cinco registros de 8 bytes cujo 1o dword e
// um comando de bate-papo do avatar sem a barra; devolve o indice do 1o que aparece em msg como "/" + comando. Classe sem nome
// no mapa (a A chamou o global de g_pSetPartTable983 em golfitemhandler.cpp); escrito como estatica (pedido C -> A).
// B124.1 (sessao A126): e o CreateCharacterOption::FindSetCommand983 (jp/createcharacteroption.cpp, frente B).
#include "jp/createcharacteroption.h"

// JP 983 (009a4f50; C-PKT lote 18): textos pelo K2L_Compatibility; mensagem ao "AvatarChat" 0x6d (0x115 quando a frase tem um
// comando do avatar, 004d0b10); sPlayerData::chatBlock (+0x56c; o 645 lia +0x55c cru); CSharedDoc::m_gameMode (+0x1048c; o 645
// lia +0x4560 cru) antes do 0x3; CMessengerInfo::GetComplete() (+0xa4; o 645 lia +0x80 cru) antes do 0x25; a ordem dos ramos
// do envio e a do 983 (cochicho 0x54, apelido -> guilda 0x25 ou 0x2a, sem apelido 0x3).
void ChatManager::SendChatMessage(const char* msg, const char* target,
	bool bTeam)
{
	if (IsLocalContent((localContentType_t)0x50 /*983: era 0x4f; medido*/) && (int)CSharedDoc::Instance()->m_playingMode == 2)
	{
		return;
	}
	int len = (int)strlen(msg);
	if (len == 0)
	{
		return;
	}
	int i = 0;
	while (i < len && msg[i] == ' ')
	{
		i++;
	}
	if (i == len)
	{
		return;
	}
	if (CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 8)
	{
		CChatMsg::Instance()->AddChatMsg(
			std::string(
				K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xc3\xa4\xc6\xc3 \xc0\xcc\xbf\xeb \xc1\xa6\xc0\xe7 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xc3\xa4\xc6\xc3\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
			LOBBY_COLOR, true, false);
		return;
	}
	std::list<std::string>& history = CSharedDoc::Instance()->m_chatHistory;
	char prevMsg[1024];
	if (history.size() == 0)
	{
		prevMsg[0] = '\0';
	}
	else
	{
		strcpy(prevMsg, history.back().c_str());
	}
	RecordMyChatHistory(msg);
	int cmdResult = CheckChatCommand(msg, 1);
	if (IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
	{
		unsigned int cmdIndex = 0;
		bool bCommand = CreateCharacterOption::Instance()->FindSetCommand983(msg, &cmdIndex);
		MsgObject avatarMsg(NULL, bCommand ? 0x115 : M983(0x60), 0, (int)msg, 0, 0, 0);
		IActor* avatarChat = AfxGetTask()->GetActor("AvatarChat");
		if (avatarChat)
		{
			avatarChat << avatarMsg;
		}
	}
	if (cmdResult == 1)
	{
		return;
	}
	unsigned long elapsed = g_CurrentTime - m_lastChatTime;
	if (m_bSwearPenalty)
	{
		if (elapsed <= 60000)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3 \xb1\xdd\xc1\xf6\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9")),
				LOBBY_COLOR, true, false);
			return;
		}
		m_bSwearPenalty = false;
		m_swearCount = 0;
	}
	if (m_bFloodPenalty)
	{
		if (elapsed <= 60000)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3 \xb1\xdd\xc1\xf6\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9")),
				LOBBY_COLOR, true, false);
			return;
		}
		m_bFloodPenalty = false;
		m_floodCount = 0;
	}
	// a penalty was just imposed: tell the player and flag the avatar so the
	// server learns about it (packet 0x4f)
	if (!(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & SD_FLAG_NOFILTER) &&
		(elapsed < 1000 || (elapsed < 10000 && _stricmp(msg, prevMsg) == 0)))
	{
		m_floodCount++;
		if (m_floodCount == 3)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xc3\xa4\xc6\xc3\xb5\xb5\xb9\xe8 \xb9\xfa\xc4\xa2\xbf\xa1 \xb0\xc9\xb8\xae\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.")),
				LOBBY_COLOR, true, false);
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3\xb1\xdd\xc1\xf6\xc0\xd4\xb4\xcf\xb4\xd9.")),
				LOBBY_COLOR, true, false);
			m_lastChatTime = g_CurrentTime;
			m_bFloodPenalty = true;
			sPlayerData* player = NULL;
			if (!IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
			{
				player = PLAYER(MyIndex());
			}
			if (OnlinePlay() && player && !player->chatBlock)
			{
				player->chatBlock = 1;
				WSendPacket packet(0x4f);
				packet.Encode1(1);
				packet.Send((eSendTo)0);
			}
			return;
		}
	}
	else
	{
		m_floodCount = 0;
	}
	char buf[1024];
	{
		memset(buf, 0, sizeof(buf));
		strcpy(buf, msg);
		if (!(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & SD_FLAG_NOFILTER) && Filtering(buf) &&
			++m_swearCount == 5)
		{
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)5\xc8\xb8\xc0\xcc\xbb\xf3 \xbf\xe5\xbc\xb3\xb4\xdc\xbe\xee\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9.")),
				LOBBY_COLOR, true, false);
			CChatMsg::Instance()->AddChatMsg(
				std::string(
					K2L_Compatibility("(\xbe\xcb\xb8\xb2)1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3\xb1\xdd\xc1\xf6\xc0\xd4\xb4\xcf\xb4\xd9.")),
				LOBBY_COLOR, true, false);
			m_lastChatTime = g_CurrentTime;
			m_bSwearPenalty = true;
			sPlayerData* player = NULL;
			if (!IS_KINDOF(ntAvatarChatTask, AfxGetTask()))
			{
				player = PLAYER(MyIndex());
			}
			if (OnlinePlay() && player && !player->chatBlock)
			{
				player->chatBlock = 1;
				WSendPacket packet(0x4f);
				packet.Encode1(1);
				packet.Send((eSendTo)0);
			}
			return;
		}
	}
	if (bTeam)
	{
		WSendPacket packet(0x54);
		packet.EncodeStr(std::string(buf));
		packet.Send((eSendTo)0);
	}
	else if (target != NULL)
	{
		if (strcmp(target, K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1\xb0\xd4")) == 0)
		{
			if (!WNetworkSystem::Instance()->IsConnected(
					(WNetworkSystem::eNetUnit)1) ||
				CMessengerInfo::Instance()->GetComplete() != CONNECT_STATUS_COMPLETE)
			{
				MsgObject m(NULL, 3,
					(int)K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb1\xe6\xb5\xe5 \xb4\xeb\xc8\xad\xb8\xa6 \xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
					0xffffcc00, 0, 0, 0);
				AfxGetTask()->GetActor("Lobby") << m;
			}
			else
			{
				std::string text(buf);
				WSendPacket packet(0x25);
				packet.EncodeStr(std::string(text.c_str()));
				packet.Send((eSendTo)1);
			}
		}
		else
		{
			std::string to(target);
			std::string text(buf);
			WSendPacket packet(0x2a);
			packet.EncodeStr(to);
			packet.EncodeStr(text);
			packet.Send((eSendTo)0);
		}
	}
	else if (CSharedDoc::Instance()->m_gameMode != 5)
	{
		std::string me(CSharedDoc::Instance()->m_myInfo.info.sNick);
		std::string text(buf);
		WSendPacket packet(3);
		packet.EncodeStr(me);
		packet.EncodeStr(text);
		packet.Send((eSendTo)0);
	}
	m_lastChatTime = g_CurrentTime;
}

bool ChatManager::Filtering(char* str)
{
	WCHAR wstr[1024];
	WCHAR wword[1024];
	if (!str)
	{
		return false;
	}
	bool bFound = false;
	int wlen = MultiByteToWideChar(GAME_CODEPAGE, 0, str, (int)strlen(str) + 1, wstr, 1024);
	for (int i = 0; i < m_chatFilterNum; i++)
	{
		int wordLen = MultiByteToWideChar(GAME_CODEPAGE, 0, m_chatFilter[i],
			(int)strlen(m_chatFilter[i]) + 1, wword, 1024);
		if (BABO_STR::WStrStr(str, wstr, wlen, wword, wordLen, false))
		{
			bFound = true;
		}
	}
	FilteringHack(str, true);
	return bFound;
}

int ChatManager::GetFaceIndex(const char* str)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 12; j++)
		{
			if (strstr(str, FacialTextTable[i][j]))
			{
				return i;
			}
		}
	}
	return 5;
}

const char* ChatManager::GetMotionName(const char* str, unsigned long uid)
{
	char buf[64];
	int retry = 0;
	// 983 009a3990: tabela 00e2dd40 (s_emoTable983, K2L) com passo 0x260 = 19 linhas x 8; nao a MotionNameTable do 645
	const char** base = &s_emoTable983[0][0][0] + (uid & 0x3ffffff) * (19 * 8);
	do
	{
		int i = 0;
		const char** row = base;
		do
		{
			if (!row)
			{
				break;
			}
			int j = 0;
			do
			{
				if (!row[j + 1])
				{
					break;
				}
				sprintf(buf, "/%s", row[j + 1]);
				if (strstr(str, buf))
				{
					return base[i * 8];
				}
				j++;
			} while (j < 7);
			i++;
			row += 8;
		} while (i < 19);
		retry++;
	} while (retry < 9);
	return 0;
}

void ChatManager::FilteringHack(const char* str, bool bChat)
{
	char dummy[16];
	char* p = (char*)str;
	if (p)
	{
		int len = (int)strlen(p);
		for (int i = 0; i < len; i++)
		{
			if (bChat)
			{
				if (p[i] == 0x3b)
				{
					p[i] = 0x11;
				}
				if (p[i] == 0x22)
				{
					p[i] = 0x12;
				}
				if (p[i] == 0x27)
				{
					p[i] = 0x13;
				}
				if (p[i] == 0x2d && i < len - 1 && p[i + 1] == 0x2d)
				{
					p[i] = 0x14;
					p[i + 1] = 0x14;
				}
			}
			else
			{
				if (p[i] == 0x11)
				{
					p[i] = 0x3b;
				}
				if (p[i] == 0x12)
				{
					p[i] = 0x22;
				}
				if (p[i] == 0x13)
				{
					p[i] = 0x27;
				}
				if (p[i] == 0x14)
				{
					p[i] = 0x2d;
				}
			}
		}
	}
}

// HACK: the original buffer/index are COMDAT statics shared with the naked RecordTournament; template statics
// give external COMDATs emitted after their first user (like function-local statics) that the asm can alias
template <int N>
struct RbAlignedStr
{
	static char buf[8][128];
	static int idx;
};
template <int N>
char RbAlignedStr<N>::buf[8][128];
template <int N>
int RbAlignedStr<N>::idx = -1;
#define s_alignedBuf RbAlignedStr<0>::buf
#define s_alignedIdx RbAlignedStr<0>::idx

const char* ChatManager::GetAlignedString(const char* str, int len, int width)
{
	if (len > 128)
	{
		return str;
	}
	s_alignedIdx++;
	memset(s_alignedBuf[s_alignedIdx], 0x20, len);
	int off;
	switch (width)
	{
	case 0:
		off = 0;
		break;
	case 1:
		off = (unsigned)(len - (int)strlen(str)) >> 1;
		break;
	case 2:
		off = len - (int)strlen(str);
		break;
	}
	strncpy(s_alignedBuf[s_alignedIdx] + off, str, strlen(str));
	s_alignedBuf[s_alignedIdx][len] = 0;
	return s_alignedBuf[s_alignedIdx];
}

const char* ChatManager::FilteringNick(char* nick)
{
	WCHAR wnick[1024];
	WCHAR wword[1024];
	if (!nick)
	{
		return NICK_ERR_EMPTY;
	}
	if (strlen(nick) < 4 || strlen(nick) > 16)
	{
		return NICK_ERR_SIZE;
	}
	int wlen =
		MultiByteToWideChar(GAME_CODEPAGE, 0, nick, (int)strlen(nick) + 1, wnick, 1024);
	for (int i = 0; i < wlen - 1; i++)
	{
		if (wnick[i] == 0x20)
		{
			return NICK_ERR_SPACE;
		}
		if (wnick[i] == 0x28)
		{
			for (int j = i + 1; j < wlen - 1; j++)
			{
				if (wnick[j] == 0x29)
				{
					unsigned short emo[256] = {0};
					memmove(emo, &wnick[i], (j - i + 1) * 2);
					const char* emoticon = FilteringEmoticon(emo);
					if (emoticon)
					{
						return emoticon;
					}
				}
			}
		}
	}
	if ((CSharedDoc::Instance()->m_myInfo.info.dwIdentity & 0x14) == 0) // 983: +0x812 = m_myInfo.info.dwIdentity (645: +0x433); D885: era cru
	{
		for (int i = 0; i < m_nickFilterNum; i++)
		{
			int wordLen = MultiByteToWideChar(GAME_CODEPAGE, 0, m_nickFilter[i].word,
				(int)strlen(m_nickFilter[i].word) + 1, wword, 1024);
			if (m_nickFilter[i].type == 1)
			{
				// A910: 983 009a43e0 e 009a4436 usam as versoes _JP (tolower so abaixo de 0x80)
				if (!BABO_STR::WStrCmp_JP((unsigned short*)wnick, wlen, (unsigned short*)wword, wordLen))
				{
					return NICK_ERR_RESTRICTED_WORD;
				}
			}
			else if (BABO_STR::WStrStr_JP(nick, (unsigned short*)wnick, wlen, (unsigned short*)wword, wordLen,
						 false))
			{
				return NICK_ERR_HAS_BAD_WORD;
			}
		}
	}
	return 0;
}

int ChatManager::FilteringGuildString(const char* str, int minLen, int maxLen,
	bool bOption)
{
	WCHAR wstr[1024];
	WCHAR wword[1024];
	if (!str)
	{
		return 1000;
	}
	if (minLen <= (int)strlen(str) && (int)strlen(str) <= maxLen)
	{
		int wlen =
			MultiByteToWideChar(GAME_CODEPAGE, 0, str, (int)strlen(str) + 1, wstr, 1024);
		for (int i = 0; i < wlen - 1; i++)
		{
			if (bOption && wstr[i] == L' ')
			{
				return 0x413;
			}
			if (wstr[i] == L'\'')
			{
				return 0x414;
			}
		}
		memset(wword, 0, sizeof(wword));
		for (int i = 0; i < m_nickFilterNum; i++)
		{
			int wordLen = MultiByteToWideChar(GAME_CODEPAGE, 0, m_nickFilter[i].word,
				(int)strlen(m_nickFilter[i].word) + 1, wword, 1024);
			if (m_nickFilter[i].type == 1)
			{
				if (!BABO_STR::WStrCmp(wstr, wlen, wword, wordLen))
				{
					return 0x415;
				}
			}
			else if (BABO_STR::WStrStr((char*)str, wstr, wlen, wword, wordLen,
						 false))
			{
				return 0x415;
			}
		}
		return 1;
	}
	CTask* task = AfxGetTask();
	MsgObject msg(NULL, 0x23,
		(int)MakeStr(
			"\\c0xffff0000\\c\xbf\xb5\xb9\xae %d - %d \xb1\xdb\xc0\xda\\c0xff000000\\c \xb6\xc7\xb4\xc2 \\c0xffff0000\\c\xc7\xd1\xb1\xdb %d - %d\xb1\xdb\xc0\xda\\c0xff000000\\c\xc0\xc7 \xb1\xe6\xc0\xcc\xb8\xa6 \xb0\xa1\xc1\xae\xbe\xdf \xc7\xd5\xb4\xcf\xb4\xd9.",
			minLen, maxLen, minLen / 2, maxLen / 2),
		0, 0, 0, 0);
	task->SendMsgToMainActor(msg);
	return 0;
}

const char* ChatManager::FilteringEmoticon(unsigned short* str)
{
	if (!IsLocalContent((localContentType_t)0x7e /*983: era 0x81; medido*/))
	{
		return 0;
	}
	FrEmoticon* emoticon = g_pFresh->GetManager()->GetEmoticon();
	int iconNum = emoticon->GetIconNum();
	std::string alias[2];
	char name[256];
	WideCharToMultiByte(GAME_CODEPAGE, 0, (LPCWSTR)str, -1, name, 256, 0, 0);
	if (strstr(name, "-"))
	{
		return K2L_Compatibility("\xbf\xaa\xc0\xa7 \xc0\xcc\xb8\xf0\xc6\xbc\xc4\xdc\xc0\xba \xb4\xd0\xb3\xd7\xc0\xd3\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
	}
	for (int i = 0; i < iconNum; i++)
	{
		if (strcmp(name, emoticon->GetIconName(i)) == 0)
		{
			break;
		}
		emoticon->GetAlias(&alias, i);
		if (alias[0] == name)
		{
			return K2L_Compatibility("\xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc5 \xc0\xcc\xb8\xf0\xc6\xbc\xc4\xdc \xc1\xf6\xbd\xc3\xbe\xee\xb4\xc2 \xb4\xd0\xb3\xd7\xc0\xd3\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
		}
		if (alias[1] == name)
		{
			return K2L_Compatibility("\xc0\xd4\xb7\xc2\xc7\xcf\xbd\xc5 \xc0\xcc\xb8\xf0\xc6\xbc\xc4\xdc \xc1\xf6\xbd\xc3\xbe\xee\xb4\xc2 \xb4\xd0\xb3\xd7\xc0\xd3\xbf\xa1 \xbb\xe7\xbf\xeb\xc7\xd2 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9");
		}
	}
	return 0;
}

// 983 009a5950: (msg, alvo, equipe) como o UTIL_SendChatMessage 006456b0 passa; o 645 tinha nick/msg trocados (AV a200-1)
void ChatManager::SendChatMessage2(const char* msg, const char* target,
	bool bTeam)
{
	int len = (int)strlen(msg);
	if (len == 0)
	{
		return;
	}
	int i = 0;
	while (i < len && msg[i] == ' ')
	{
		i++;
	}
	if (i == len)
	{
		return;
	}
	// JP 983 (009a59cd, 009a5ade, 009a625f; C-PKT lote 10): doc+0x83f, +0x812 e +0x7b8 pelos nomes (os SD_* sao do KR 645).
	if (CSharedDoc::Instance()->m_myInfo.info.dwFlagBlock & 8)
	{
		CChatMsg::Instance()->AddChatMsg(
			std::string(
				K2L_Compatibility("(\xbe\xcb\xb8\xb2)\xc3\xa4\xc6\xc3 \xc0\xcc\xbf\xeb \xc1\xa6\xc0\xe7 \xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9. \xc3\xa4\xc6\xc3\xc0\xbb \xc0\xcc\xbf\xeb\xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9.")),
			LOBBY_COLOR, true, false);
		return;
	}
	std::list<std::string>& history = CSharedDoc::Instance()->m_chatHistory;
	char prevMsg[1024];
	if (history.size() == 0)
	{
		prevMsg[0] = '\0';
	}
	else
	{
		strcpy(prevMsg, history.back().c_str());
	}
	RecordMyChatHistory(msg);
	if (CheckChatCommand(msg, 0) == 1)
	{
		return;
	}
	unsigned long elapsed = g_CurrentTime - m_lastChatTime;
	if (m_bSwearPenalty)
	{
		if (elapsed > 60000)
		{
			m_bSwearPenalty = false;
			m_swearCount = 0;
		}
		else
		{
			std::string text(
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : 1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3 \xb1\xdd\xc1\xf6\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9"));
			SEND_LOBBY_MSG(text.c_str(), LOBBY_COLOR);
			return;
		}
	}
	if (m_bFloodPenalty)
	{
		if (elapsed > 60000)
		{
			m_bFloodPenalty = false;
			m_floodCount = 0;
		}
		else
		{
			std::string text(
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : 1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3 \xb1\xdd\xc1\xf6\xc1\xdf\xc0\xd4\xb4\xcf\xb4\xd9"));
			SEND_LOBBY_MSG(text.c_str(), LOBBY_COLOR);
			return;
		}
	}
	if (!(CSharedDoc::Instance()->m_myInfo.info.dwIdentity & SD_FLAG_NOFILTER) &&
		(elapsed < 1000 || (elapsed < 10000 && _stricmp(msg, prevMsg) == 0)))
	{
		m_floodCount++;
		if (m_floodCount == 3)
		{
			std::string text(
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : \xc3\xa4\xc6\xc3\xb5\xb5\xb9\xe8 \xb9\xfa\xc4\xa2\xbf\xa1 \xb0\xc9\xb8\xae\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."));
			SEND_LOBBY_MSG(text.c_str(), LOBBY_COLOR);
			text =
				K2L_Compatibility("\xbe\xcb\xb8\xb2 : 1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3\xb1\xdd\xc1\xf6\xc0\xd4\xb4\xcf\xb4\xd9.");
			SEND_LOBBY_MSG(text.c_str(), LOBBY_COLOR);
			m_lastChatTime = g_CurrentTime;
			m_bFloodPenalty = true;
			return;
		}
	}
	else
	{
		m_floodCount = 0;
	}
	char buf[1024];
	memset(buf, 0, sizeof(buf));
	strcpy(buf, msg);
	if ((CSharedDoc::Instance()->m_myInfo.info.dwIdentity & SD_FLAG_NOFILTER) || !Filtering(buf) ||
		++m_swearCount != 5)
	{
		// JP 983 (009a5950; C-PKT lote 18): ordem dos ramos do 983 (cochicho 0x54, apelido -> guilda 0x25 ou 0x2a, sem apelido
		// 0x3); guilda por strcmp; CMessengerInfo::GetComplete() (+0xa4; o 645 lia +0x80 cru).
		if (bTeam)
		{
			WSendPacket packet(0x54);
			packet.EncodeStr(std::string(buf));
			packet.Send((eSendTo)0);
		}
		else if (target != NULL)
		{
			if (strcmp(target, K2L_Compatibility("\xb1\xe6\xb5\xe5\xbf\xa1\xb0\xd4")) == 0)
			{
				if (!WNetworkSystem::Instance()->IsConnected(
						(WNetworkSystem::eNetUnit)1) ||
					CMessengerInfo::Instance()->GetComplete() != CONNECT_STATUS_COMPLETE)
				{
					MsgObject m(NULL, 3,
						(int)K2L_Compatibility("\xc1\xf6\xb1\xdd\xc0\xba \xb1\xe6\xb5\xe5 \xb4\xeb\xc8\xad\xb8\xa6 \xc7\xcf\xbd\xc7 \xbc\xf6 \xbe\xf8\xbd\xc0\xb4\xcf\xb4\xd9."),
						0xffffcc00, 0, 0, 0);
					AfxGetTask()->GetActor("Lobby") << m;
				}
				else
				{
					std::string text(buf);
					WSendPacket packet(0x25);
					packet.EncodeStr(std::string(text.c_str()));
					packet.Send((eSendTo)1);
				}
			}
			else
			{
				std::string to(target);
				std::string text(buf);
				WSendPacket packet(0x2a);
				packet.EncodeStr(std::string(target));
				packet.EncodeStr(std::string(buf));
				packet.Send((eSendTo)0);
			}
		}
		else
		{
			std::string me(CSharedDoc::Instance()->m_myInfo.info.sNick);
			std::string text(buf);
			WSendPacket packet(3);
			packet.EncodeStr(me);
			packet.EncodeStr(text);
			packet.Send((eSendTo)0);
		}
		m_lastChatTime = g_CurrentTime;
		return;
	}
	std::string text(
		K2L_Compatibility("\xbe\xcb\xb8\xb2 : 5\xc8\xb8\xc0\xcc\xbb\xf3 \xbf\xe5\xbc\xb3\xb4\xdc\xbe\xee\xb8\xa6 \xbb\xe7\xbf\xeb\xc7\xcf\xbc\xcc\xbd\xc0\xb4\xcf\xb4\xd9."));
	MsgObject m1(NULL, 3, (int)text.c_str(), LOBBY_COLOR, 0, 0, 0);
	AfxGetTask()->GetActor("Lobby") << m1;
	text =
		K2L_Compatibility("\xbe\xcb\xb8\xb2 : 1\xba\xd0\xb0\xa3 \xc3\xa4\xc6\xc3\xb1\xdd\xc1\xf6\xc0\xd4\xb4\xcf\xb4\xd9.");
	MsgObject m2(NULL, 3, (int)text.c_str(), LOBBY_COLOR, 0, 0, 0);
	AfxGetTask()->GetActor("Lobby") << m2;
	m_lastChatTime = g_CurrentTime;
	m_bSwearPenalty = true;
}

void ChatManager::Display(const char* str, int type)
{
	if (type == 0)
	{
		SEND_LOBBY_MSG(str, LOBBY_COLOR);
	}
	else if (type == 1)
	{
		CChatMsg::Instance()->AddChatMsg(std::string(str), LOBBY_COLOR, true,
			false);
	}
}

int ChatManager::CheckChatCommand(const char* str, int type)
{
	// JP 983 009a47a0
	CGMToolkit* gm = CGMToolkit::Instance();
	if (*str != '/')
		return 0;
	if (gm->IsAdministrator() && gm->Command(str))
		return 1;
	if (*str != '/') // 983 009a4801: confere de novo depois do Command
		return 0;
	const char* args = str + 1;
	char cmd[512];
	GetCharsTillSpace(args, cmd);
	if (cmd[0] == '\0')
		return 1;
	unsigned int idx;
	for (idx = 0; idx < 3; idx++)
	{
		if (_stricmp(cmd, ChatCmd[idx].name) == 0)
			break;
	}
	if (idx >= 3)
		return 1;
	switch (idx)
	{
	case 0:
	{
		unsigned long target = 0;
		char reason[24]; // 983: 0x18 bytes (quadro de 0x294)
		switch (sscanf(args, "%s %d %s", cmd, &target, reason))
		{
		case 1:
		{
			WSendPacket packet(0x41);
			packet.Encode4(0xffffffff);
			packet.EncodeStr(std::string(CSharedDoc::Instance()->m_myInfo.info.sNick));
			packet.Send((eSendTo)0);
		}
		break;
		case 2:
		{
			WSendPacket packet(0x41);
			packet.Encode4(target);
			packet.EncodeStr(std::string(CSharedDoc::Instance()->m_myInfo.info.sNick));
			packet.Send((eSendTo)0);
		}
		break;
		case 3:
		{
			WSendPacket packet(0x41);
			packet.Encode4(target);
			packet.EncodeStr(std::string(reason));
			packet.Send((eSendTo)0);
		}
		break;
		}
	}
	break;
	case 2:
	{
		if (!((CSharedDoc::Instance()->m_myInfo.info.dwIdentity >> 6) & 1))
			return 1;
		unsigned long rate = 0;
		switch (sscanf(args, "%s %d", cmd, &rate))
		{
		case 1:
		{
			WSendPacket packet(0x4e);
			packet.Encode4(3);
			packet.Encode1(0);
			packet.Send((eSendTo)0);
		}
		break;
		case 2:
		{
			if ((int)rate < 100)
			{
				Display(
					K2L_Compatibility("\xbe\xc6\xc0\xcc\xc5\xdb\xb5\xe5\xb7\xd3 \xc8\xae\xb7\xfc\xc0\xcc \xb1\xe2\xba\xbb(100%)\xba\xb8\xb4\xd9 \xc0\xdb\xbd\xc0\xb4\xcf\xb4\xd9. \xb4\xd9\xbd\xc3 \xbc\xb3\xc1\xa4\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4."),
					type);
				return 1;
			}
			WSendPacket packet(0x4e);
			packet.Encode4(3);
			packet.Encode1(1);
			packet.Encode4(rate);
			packet.Send((eSendTo)0);
		}
		break;
		default:
			Display(
				MakeStr(
					K2L_Compatibility("\xb4\xd9\xbd\xc3 \xc0\xd4\xb7\xc2\xc7\xd8 \xc1\xd6\xbc\xbc\xbf\xe4. \xbf\xb9)/itemdrop \xb9\xe8\xc0\xb2")),
				type);
			return 1;
		}
	}
	break;
	}
	return 1;
}

int ChatManager::SaveChatHistory(int from, int to)
{
	char path[260];
	if (CSharedDoc::Instance()->m_chatLineList.size() == 0)
	{
		return 0;
	}
	mkdir("history");
	mkdir("history/chatting");
	if (from < 0)
	{
		from = 1;
		int ret;
		do
		{
			sprintf(path, "history/chatting/chat%03d.txt", from);
			ret = _access(path, 0);
			from++;
		} while (ret != -1);
		from--;
	}
	else
	{
		from = from + 1;
		sprintf(path, "history/chatting/chat%03d.txt", from);
	}
	FILE* fp = fopen(path, "wt");
	if (!fp)
	{
		return 0;
	}
	{
		std::list<sChatLine>::iterator it =
			CSharedDoc::Instance()->m_chatLineList.begin();
		while (it != CSharedDoc::Instance()->m_chatLineList.end())
		{
			fwrite(it->text.c_str(), strlen(it->text.c_str()), 1, fp);
			fwrite("\n", 1, 1, fp);
			++it;
		}
		fclose(fp);
		Display(
			MakeStr(
				K2L_Compatibility("\xc3\xa4\xc6\xc3 \xb3\xbb\xbf\xeb\xc0\xcc %s\xbf\xa1 \xc0\xfa\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
				path),
			to);
		return from;
	}
}

// Grade (rank) names with the score needed for each one, followed by two float limits.
struct sGradeTable
{
	struct sGrade
	{
		char name[16];
		int score;
	};
	sGrade grades[71];
	float maxValue;
	float epsilon;
};
static CLevelNameTable983 s_levelNames; // 00f004ac
static const sGradeTable s_gradeTable = {
	{
     { "\xb7\xe7\xc5\xb0"
		  "F",
			30 },
     { "\xb7\xe7\xc5\xb0"
		  "E",
			40 },
     { "\xb7\xe7\xc5\xb0"
		  "D",
			50 },
     { "\xb7\xe7\xc5\xb0"
		  "C",
			60 },
     { "\xb7\xe7\xc5\xb0"
		  "B",
			70 },
     { "\xb7\xe7\xc5\xb0"
		  "A",
			100 },
     { "\xba\xf1\xb1\xe2\xb3\xca"
		  "E",
			133 },
     { "\xba\xf1\xb1\xe2\xb3\xca"
		  "D",
			148 },
     { "\xba\xf1\xb1\xe2\xb3\xca"
		  "C",
			163 },
     { "\xba\xf1\xb1\xe2\xb3\xca"
		  "B",
			178 },
     { "\xba\xf1\xb1\xe2\xb3\xca"
		  "A",
			213 },
     { "\xc1\xd6\xb4\xcf\xbe\xee"
		  "E",
			267 },
     { "\xc1\xd6\xb4\xcf\xbe\xee"
		  "D",
			288 },
     { "\xc1\xd6\xb4\xcf\xbe\xee"
		  "C",
			309 },
     { "\xc1\xd6\xb4\xcf\xbe\xee"
		  "B",
			330 },
     { "\xc1\xd6\xb4\xcf\xbe\xee"
		  "A",
			491 },
     { "\xbd\xc3\xb4\xcf\xbe\xee"
		  "E",
			660 },
     { "\xbd\xc3\xb4\xcf\xbe\xee"
		  "D",
			716 },
     { "\xbd\xc3\xb4\xcf\xbe\xee"
		  "C",
			772 },
     { "\xbd\xc3\xb4\xcf\xbe\xee"
		  "B",
			828 },
     { "\xbd\xc3\xb4\xcf\xbe\xee"
		  "A",
			1224 },
     { "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
		  "E",
			1656 },
     { "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
		  "D",
			1742 },
     { "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
		  "C",
			1828 },
     { "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
		  "B",
			1914 },
     { "\xbe\xc6\xb8\xb6\xc3\xdf\xbe\xee"
		  "A",
			2346 },
     { "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
		  "E",
			2871 },
     { "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
		  "D",
			2994 },
     { "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
		  "C",
			3117 },
     { "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
		  "B",
			3240 },
     { "\xbc\xbc\xb9\xcc\xc7\xc1\xb7\xce"
		  "A",
			6480 },
     { "\xc7\xc1\xb7\xce"
		  "E",
			4536 },
     { "\xc7\xc1\xb7\xce"
		  "D",
			4684 },
     { "\xc7\xc1\xb7\xce"
		  "C",
			4832 },
     { "\xc7\xc1\xb7\xce"
		  "B",
			4980 },
     { "\xc7\xc1\xb7\xce"
		  "A",
			14940 },
     { "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
		  "E",
			6972 },
     { "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
		  "D",
			7168 },
     { "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
		  "C",
			7364 },
     { "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
		  "B",
			7560 },
     { "\xb3\xbb\xbc\xc5\xb3\xaf\xc7\xc1\xb7\xce"
		  "A",
			15120 },
     { "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
		  "E",
			9072 },
     { "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
		  "D",
			9322 },
     { "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
		  "C",
			9572 },
     { "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
		  "B",
			9822 },
     { "\xbf\xf9\xb5\xe5\xc7\xc1\xb7\xce"
		  "A",
			19644 },
     { "\xb8\xb6\xbd\xba\xc5\xcd"
		  "E",
			9822 },
     { "\xb8\xb6\xbd\xba\xc5\xcd"
		  "D",
			10122 },
     { "\xb8\xb6\xbd\xba\xc5\xcd"
		  "C",
			10422 },
     { "\xb8\xb6\xbd\xba\xc5\xcd"
		  "B",
			10722 },
     { "\xb8\xb6\xbd\xba\xc5\xcd"
		  "A",
			21444 },
     { "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
		  "E",
			10722 },
     { "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
		  "D",
			11122 },
     { "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
		  "C",
			11522 },
     { "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
		  "B",
			11922 },
     { "\xc5\xbe\xb8\xb6\xbd\xba\xc5\xcd"
		  "A",
			23844 },
     { "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
		  "E",
			23844 },
     { "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
		  "D",
			24394 },
     { "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
		  "C",
			24944 },
     { "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
		  "B",
			25494 },
     { "\xbf\xf9\xb5\xe5\xb8\xb6\xbd\xba\xc5\xcd"
		  "A",
			50988 },
     { "\xb7\xb9\xc0\xfc\xb5\xe5"
		  "E",
			101978 },
     { "\xb7\xb9\xc0\xfc\xb5\xe5"
		  "D",
			203952 },
     { "\xb7\xb9\xc0\xfc\xb5\xe5"
		  "C",
			407904 },
     { "\xb7\xb9\xc0\xfc\xb5\xe5"
		  "B",
			815808 },
     { "\xb7\xb9\xc0\xfc\xb5\xe5"
		  "A",
			1631616 },
     { "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
		  "E",
			1631616 },
     { "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
		  "D",
			1631616 },
     { "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
		  "C",
			1631616 },
     { "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
		  "B",
			1631616 },
     { "\xc0\xce\xc7\xc7\xb4\xcf\xc6\xbc\xb7\xb9\xc0\xfc\xb5\xe5"
		  "A",
			1631616 },
	 },
	3.402823466e+38f, 1.0e-5f
};

void ChatManager::RecordTournament()
{
	char path[260];
	// JP 983 (009a45a0): entradas de 0x1b8 bytes (645: 400); nomes dos niveis pela tabela do jogador (CLevelNameTable983, 008329d0)
	sRivalData* it = *(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_BEGIN));
	sRivalData* end = *(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_END));
	for (; it != end; ++it)
	{
		if (it->state == 0)
		{
			return;
		}
	}
	sRivalData* first = *(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_BEGIN));
	if (((unsigned char*)end - (unsigned char*)first) / (int)sizeof(sRivalData) == 0)
	{
		return;
	}
	_mkdir("history");
	_mkdir("history/tournament");
	unsigned char no = 1;
	do
	{
		sprintf(path, "history/tournament/result%03d.txt", no);
		no++;
	} while (_access(path, 0) != -1);
	FILE* fp = fopen(path, "wt");
	if (!fp)
	{
		return;
	}
	fprintf(fp, K2L_Compatibility("\xb9\xe6\xc0\xe5 : %s\n\n"),
		(*(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_BEGIN)))->nickname);
	fprintf(fp,
		"\xbc\xf8\xc0\xa7     \xb7\xb9  \xba\xa7           \xb4\xeb \xc8\xad \xb8\xed       \xbd\xba\xc4\xda\xbe\xee   \xc6\xce\n");
	fprintf(fp, "--------------------------------------------------------\n");
	for (sRivalData* e2 = *(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_BEGIN));
		e2 != *(sRivalData**)(SD_BYTES() + DOCOFS983(SD_TOURN_END)); ++e2)
	{
		unsigned int rank = (e2->state == 3) ? 0 : e2->rank;
		const char* alignedName = GetAlignedString(e2->nickname, 16, 0);
		const char* alignedClass = GetAlignedString(
			K2L_Compatibility(
				s_levelNames.Get(e2->level).name),
			11, 0);
		fprintf(fp, " %2d    %s    %s    %3d    %I64d\n", rank, alignedClass,
			alignedName, e2->totalScore, e2->totalPang);
	}
	fclose(fp);
	Display(
		MakeStr(
			K2L_Compatibility("30\xc0\xce\xb4\xeb\xc8\xb8 \xb0\xe1\xb0\xfa\xb0\xa1 %s\xbf\xa1 \xc0\xfa\xc0\xe5\xb5\xc7\xbe\xfa\xbd\xc0\xb4\xcf\xb4\xd9."),
			path),
		0);
}

void ChatManager::RecordMyChatHistory(const char* str)
{
	std::list<std::string>& history = CSharedDoc::Instance()->m_chatHistory;
	std::list<std::string>::iterator it =
		std::find(history.begin(), history.end(), str);
	if (it != history.end())
	{
		history.erase(it);
		return;
	}
	if (history.size() == 20)
	{
		history.erase(history.begin());
	}
	history.push_back(str);
}
