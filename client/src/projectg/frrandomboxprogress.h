#pragma once
// JP 983: sizeof 0x130 (operator new de 008c53d0), com o "image" (FrArea, +0x118) que as derivadas do 983 animam.
// FRRANDOMBOXPROGRESS_983 liga o campo e o C_ASSERT da FrClubSetResetProgress (include/jp/clubsetresetwnd.h, frente B).
#define FRRANDOMBOXPROGRESS_983 1
class FrGaugeBar;
class FrArea;
#include "frform.h"
class FrStatic;
class FrRandomBoxProgress : public FrForm
{
public:
	DECLARE_OBJECT(FrRandomBoxProgress)

	FrRandomBoxProgress();
	virtual ~FrRandomBoxProgress();

	void SetExplane(const char* text);

protected:
	virtual bool OnInit();
	virtual void OnProc(const float dt);

	void OnCancelUp(int param);
	void OnProgressInit(int param);
	void OnExplaneStatic_Init(int param);
	void OnImageInit983(int param); // 008c55a0: "image" FRCMD_INIT

	FrGaugeBar* m_pProgress; // +0x110
	FrStatic* m_pExplaneStatic; // +0x114
	FrArea* m_pImage983; // +0x118
	unsigned long m_boxTid; // +0x11c
	float m_totalTime; // +0x120
	float m_elapsedTime; // +0x124
	unsigned char m_pad983_128[8]; // +0x128: 983 sem acesso (new 0x130 em 008c53d0; 008c52d0..008c5750, derivada e 00610e00 sem +0x128..+0x12f)

	DECLARE_FRESH_MSGMAP()

public:
	void SetBoxTid(unsigned long tid) { m_boxTid = tid; }
	void SetBoxTid983(unsigned long tid); // 0060c960: tid + icone do IFF no "image" (centrado na vertical)
};
