#include "minatl.h"
#include <bitset>

#include "localize.h"

static __int64 g_localCountryCode;
static std::bitset<MAX_LOCAL_CONTENTS> s_localContentSet;

#include "localize_jpn.h" // 983 (país 2); a tabela do 645 KR fica em localize_kor.h

bool IsLocalCountry(__int64 code)
{
	return (g_localCountryCode & code) ? true : false;
}

bool IsLocalContent(localContentType_t code)
{
	if (code >= MAX_LOCAL_CONTENTS)
		return false;

	return s_localContentSet[code];
}

#ifdef JP983_SERVER_CONFIG
// modo de teste: JP983_EVENTOS=1 liga as travas dos eventos que o 983 deixa desligados
void EnableTestEvents983()
{
	static const unsigned char s_ids[] = { 0x16, 0x19, 0x1f, 0x21, 0x22, 0x4d, 0x77, 0x78, 0x80, 0x82, 0x89, 0x8b };
	char on[8];
	if (GetEnvironmentVariableA("JP983_EVENTOS", on, sizeof(on)) == 0)
		return;
	for (size_t i = 0; i < sizeof(s_ids); i++)
		s_localContentSet.set(s_ids[i]);
}
#endif

int GetLocalCountry()
{
	return (int)g_localCountryCode;
}
