// Unidade "wvdshnames" do 983 (10058350..1005849f; nome de arquivo provisório, F1.3): nomes dos enums do shader
// (macros de variação, semânticas de sistema, semânticas básicas do material), cada tabela com Nome(i) e Procura(nome).
#include <string.h>
#include "wvideo.h"

static const char* s_macroNames[] = {
	"WVDSHMACRO_VERTEXCOLOR",
	"WVDSHMACRO_SKIN",
	"WVDSHMACRO_SKINWEIGHT1",
	"WVDSHMACRO_SKINWEIGHT2",
	"WVDSHMACRO_SKINWEIGHT3",
	"WVDSHMACRO_NOLIGHT",
	"WVDSHMACRO_FOG",
	"WVDSHMACRO_SHADOWCASTER",
	"WVDSHMACRO_SHADOWRECVER",
	"WVDSHMACRO_ACTIVATED",
	"WVDSHMACRO_NUM",
};

const char* WVDShMacroName(int i)
{
	return s_macroNames[i];
}

int WVDShMacroFromName(const char* name)
{
	for (unsigned int i = 0; i < 10; i++)
	{
		if (strcmp(s_macroNames[i], name) == 0)
			return i;
	}
	return 10;
}

static const char* s_sysParamSemanticNames[] = {
	"WORLD",
	"SKIN",
	"VIEW",
	"PROJECTION",
	"WORLDCAMERAPOSITION",
	"MTLCLR",
	"DIRECTION",
	"LIGHTCOLOR",
	"AMBIENTCOLOR",
	"FOGRANGE",
	"TIME",
	"MTLTIME",
	"TEXTURETRANSFORM",
	"NUM",
};

const char* WVDShSysParamSemanticName(int i)
{
	return s_sysParamSemanticNames[i];
}

int WVDShSysParamSemanticFromName(const char* name)
{
	for (unsigned int i = 0; i < 13; i++)
	{
		if (strcmp(s_sysParamSemanticNames[i], name) == 0)
			return i;
	}
	return 13;
}

static const char* s_userParamSemanticNames[] = {
	"WVDSHBASICUSERPARAMSEMANTIC_CULL",
	"WVDSHBASICUSERPARAMSEMANTIC_ALPHABLENDENABLE",
	"WVDSHBASICUSERPARAMSEMANTIC_SRCBLEND",
	"WVDSHBASICUSERPARAMSEMANTIC_DESTBLEND",
	"WVDSHBASICUSERPARAMSEMANTIC_ALPHATESTENABLE",
	"WVDSHBASICUSERPARAMSEMANTIC_ALPHAFUNC",
	"WVDSHBASICUSERPARAMSEMANTIC_ALPHAREF",
	"WVDSHBASICUSERPARAMSEMANTIC_ZWRITEENABLE",
	"WVDSHBASICUSERPARAMSEMANTIC_ZFUNC",
	"WVDSHBASICUSERPARAMSEMANTIC_NUM",
};

const char* WVDShUserParamSemanticName(int i)
{
	return s_userParamSemanticNames[i];
}

int WVDShUserParamSemanticFromName(const char* name)
{
	for (unsigned int i = 0; i < 9; i++)
	{
		if (strcmp(s_userParamSemanticNames[i], name) == 0)
			return i;
	}
	return 9;
}
