#pragma once

#include <map>
#include <string>
#include "fxspray.h"

// D858: fora do VC10 (cl 14.44) o STL e menor e os tocos de bytes das visoes parciais (tamanhos do VC10) deslocam
// os campos entre unidades (vs2022-1: CFxSequence::LoadFile lia m_sequenceCache do CFxBox no lugar errado). La todas
// as unidades usam a visao completa; no VC10 (983) nada muda.
#if defined(REBANG_FXSEQUENCE_CPP) || _MSC_VER > 1600
#define RB_FXSEQUENCE_FULL
#endif

// HACK: the full layout is only visible to fxsequence.cpp; every other unit (projectg includes
// fx.h outside the PCH) keeps the partial view so its name count stays as in the original.
#ifdef RB_FXSEQUENCE_FULL
#include <list>

class CFxBox;
struct Attacher;
#endif

// Classe completa so com RB_FXSEQUENCE_FULL (ver acima); as outras unidades veem os tocos de bytes do VC10 (de proposito).
class CFxSequence
{
#ifdef REBANG_FXBOX_CPP
	friend class CFxBox;
#endif

public:
#ifdef RB_FXSEQUENCE_FULL
	struct sLine
	{
		int line;
		unsigned long minTime;
		unsigned long maxTime;
		unsigned long time;
		char text[260];
	};

	CFxSequence(CFxBox& box, bool bDelete);
	virtual ~CFxSequence();

	void Clear();
	bool LoadFile(const char* filename);
	void AddLine(const char* line, int lineNo);
	void SetDistLimit(float minDist, float maxDist);
	const sLine& GetLastError();
#endif

	void SetActive(bool bActive, bool bChild);
	WVector& Pos();
	std::map<CFxSpray*, CFxSpray*>& GetSprayList();
	void SetDelay(float delay);
	void GetBoneMatrix(WMatrix* mat);
	void SetWind(const WVector& wind);
	void SetLight(const LightSet& light);

#ifdef RB_FXSEQUENCE_FULL
protected:
	void SetLineTime();
	bool ParseLine(const char* line, bool& bError);
	bool Process(float elapsed, bool& bFinished);

private:
	void DeleteChildSpray(CFxSpray* spray);

public:
	bool m_bMark;
	WVector m_wind;
	const Attacher* m_pAttacher;
	float m_attachX;
	float m_attachY;
	std::map<CFxSpray*, CFxSpray*> m_spray;
	WMatrix* m_attachMat[4];
	WVector* m_attachPos[4];
	defines_t m_defines;
	WMatrix m_rotation;
	CFxBox* m_pFxBox;
	bool m_bActive;
	WVector m_pos;
	WVector m_vel;
	bool m_bDelete;
	float m_time;
	float m_delay;
	sLine m_curLine;
	CFxSpray* m_pCurSpray;
	std::list<sLine> m_lineList;
	std::list<sLine>::iterator m_curIt;
	std::string m_filename;
#else
	virtual ~CFxSequence(); // vptr em +0 (D198: delete fora do fxsequence.cpp chama pelo vtable)
	unsigned char m_unused4[0x14];
	float m_attachX; // 0x18
	float m_attachY; // 0x1c
	std::map<CFxSpray*, CFxSpray*> m_spray;
	WMatrix* m_attachMat[4];
	unsigned char m_unused3c[0x10c8];
	bool m_bActive;
	WVector m_pos;
	WVector m_vel;
	bool m_bDelete;
	float m_time;
	float m_delay;
	unsigned char m_unused112c[0x128];
	std::string m_filename;
#endif
};

#include "fxsequence.inl"
