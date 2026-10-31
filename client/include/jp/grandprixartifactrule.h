#pragma once
// JP 983: _grandprix::_private::CGrandPrixArtifactRule (RTTI .?AVCGrandPrixArtifactRule@_private@_grandprix@@, vtable 00d097c4 só com o
// destrutor; WSingleton em 00e75194, criado em 00a65901 com `new` de 0x28 = BaseObject). Regras dos artefatos do Grand Prix (typeid do
// artefato em CSharedDoc+0x109b9): 0x1a000266 muda um valor de 1 a 15, 0x1a000267 olha um bit da tacada, 0x1a000268 / 0x1a00029e
// valem por 10 / 15 segundos. Unidade 00546900–00546b60 (sessão B65; o corte da F3.7.1 começava em 00546a20). Nomes meus.
#include "singleton.h"

namespace _grandprix
{
namespace _private
{
class CGrandPrixArtifactRule : public WSingleton<CGrandPrixArtifactRule>
{
public:
	CGrandPrixArtifactRule() { } // 00546a20
	virtual ~CGrandPrixArtifactRule() { } // 00546a40 (00546b10 com delete)

	bool IsTimed(unsigned long tid); // 00546900 (golfrule 0074be2b, 009c2920)
	int GetSeconds(unsigned long tid); // 00546920
	bool HasFlag(const unsigned char* pShot); // 00546950: bits 1–3 do byte +0x1f
	bool IsExpired(const unsigned char* pShot, unsigned long start, unsigned long tid); // 00546970: (agora − start) / 1000 >= segundos
	bool Check(unsigned long tid, const unsigned char* pShot, unsigned long start); // 00546a60 (CGolfRule 00684a96)
	float Modify(unsigned long tid, float value, unsigned char level, unsigned char unused); // 00546aa0 (006ca2eb): level + 1 entre 1 e 15; o 4º argumento não é lido
};
}
}
