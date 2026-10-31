// JP 983: unidade 00546900–00546b60 (_grandprix::_private::CGrandPrixArtifactRule). Fonte reconstruída do 983 (classe nova, sem par no
// 645). Sessão B65: os seis métodos; construtor e destrutor (e os do WSingleton, 005469a0–005469e0) saem do cabeçalho. 00546b40
// (`[ecx+0x2bc] > 3`) e 00546b50 (`[ecx+1]`) no fim do corte não têm chamador nem classe: não escritos.
#include "minatl.h"
#include "projectg.h"
#include "grandprixartifactrule.h"

using namespace _grandprix::_private;

// byte +0x1f do registro que a regra recebe (só o campo lido aqui)
struct sShotFlags983
{
	unsigned char m_00[0x1f];
	unsigned char b0 : 1;
	unsigned char kind : 3; // bits 1–3
};


bool CGrandPrixArtifactRule::IsTimed(unsigned long tid)
{
	switch (tid)
	{
	case 0x1a000268:
	case 0x1a00029e:
		return true;
	}
	return false;
}

int CGrandPrixArtifactRule::GetSeconds(unsigned long tid)
{
	int seconds = 0;
	switch (tid)
	{
	case 0x1a000268:
		return 10;
	case 0x1a00029e:
		seconds = 15;
		break;
	}
	return seconds;
}

bool CGrandPrixArtifactRule::HasFlag(const unsigned char* pShot)
{
	if (((const sShotFlags983*)pShot)->kind) // o 983 desvia (`jne` + `mov al, 1`); o nosso cl faz `setne`
		return true;
	return false;
}

bool CGrandPrixArtifactRule::IsExpired(const unsigned char* pShot, unsigned long start, unsigned long tid)
{
	unsigned int seconds = GetSeconds(tid);
	if (seconds <= (g_CurrentTime - start) / 1000)
		return true;
	return false;
}

bool CGrandPrixArtifactRule::Check(unsigned long tid, const unsigned char* pShot, unsigned long start)
{
	if (!tid)
		return false;
	switch (tid)
	{
	case 0x1a000267:
		return HasFlag(pShot);
	case 0x1a000268:
	case 0x1a00029e:
		return IsExpired(pShot, start, tid);
	}
	return false;
}

float CGrandPrixArtifactRule::Modify(unsigned long tid, float value, unsigned char level, unsigned char unused)
{
	if (tid != 0x1a000266)
		return value;
	float v = (float)(level + 1.0);
	if (v < 1.0f)
		v = 1.0f;
	else if (v > 15.0)
		v = 15.0f;
	return v;
}
