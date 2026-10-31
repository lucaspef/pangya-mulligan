#pragma once

#include "scenemanager.h"

#include "fxbox.h"

struct Attacher
{
	float x;
	float y;
	WMatrix* mat[4];
	WVector* pos[4];

	Attacher();
	bool Get(WVector& out);
};

struct defines_t;
struct sFxKeeper;

// Definicao parcial (F5, decisao): so os membros que as nossas unidades usam; o resto do 983 fica de fora.
class CPvsFxBox : public CFxBox
{
public:
	virtual void ProcessAttacher(float elapsed);

protected:
	virtual bool CheckGroundCollision(const WVector& from, const WVector& to,
		float& height, float radius);
	virtual void OnSequenceCreated(CFxSequence* sequence);
	virtual void OnSequenceDeleted(CFxSequence* sequence);
	virtual void OnSprayCreated(CFxSpray* spray);
	virtual void OnSprayDeleted(CFxSpray* spray);
#ifdef REBANG_FX_CPP
	void UpdateLightSrc(const LightSet& light);
#endif

	std::map<CFxSequence*, Attacher> m_seqAttacher;
	std::map<CFxSpray*, Attacher> m_sprayAttacher;
};

// HACK: see fxsequence.h (the effect controls are only visible to fx.cpp)
#ifdef REBANG_FX_CPP
class CGolfBall;

class CFxControl
{
public:
	virtual ~CFxControl() { }
	virtual bool Process(float elapsed) = 0;

	static unsigned long ms_serial;

	unsigned long m_serial;
};

class CFxRunDust : public CFxControl
{
public:
	CFxRunDust(CPvsFxBox& fxBox, const WVector& pos, const WVector& vel);
	virtual ~CFxRunDust();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	const WVector& m_pos;
	const WVector& m_vel;
	CFxSequence* m_pGround;
	CFxSequence* m_pWater;
	unsigned long m_unused1c;
};

class CFxCobra : public CFxControl
{
public:
	CFxCobra(CPvsFxBox& fxBox, const CGolfBall& ball);
	virtual ~CFxCobra();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	const CGolfBall& m_ball;
	CFxSequence* m_pSequence[4];
	bool m_bStarted;
};

class CFxTomahawk : public CFxControl
{
public:
	CFxTomahawk(CPvsFxBox& fxBox, const CGolfBall& ball, bool bNotify,
		unsigned long itemId);
	virtual ~CFxTomahawk();
	virtual bool Process(float elapsed);
	bool IsPlumeItem(unsigned long id); // 983 008794c0 (thiscall, ret 4)

	CPvsFxBox& m_fxBox;
	const CGolfBall& m_ball;
	CFxSequence* m_pSequence;
	unsigned long m_itemId; // 983: decide a pluma (plume_tomahawk_*.seq)
	bool m_bNotify;
	int m_state;
};

class CFxSpike : public CFxControl
{
public:
	CFxSpike(CPvsFxBox& fxBox, const CGolfBall& ball, bool bNotify);
	virtual ~CFxSpike();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	const CGolfBall& m_ball;
	CFxSequence* m_pSequence[2];
	bool m_bNotify;
	int m_state;
};

class CFxPowerSpin : public CFxControl
{
public:
	CFxPowerSpin(CPvsFxBox& fxBox, CGolfBall& ball);
	virtual ~CFxPowerSpin();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	CGolfBall& m_ball;
	CFxSequence* m_pSequence;
	bool m_bRotated;
};

class CFxShotImpact : public CFxControl
{
public:
	CFxShotImpact(CPvsFxBox& fxBox, CGolfBall& ball);
	virtual ~CFxShotImpact();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	CGolfBall& m_ball;
	CFxSequence* m_pSequence;
	bool m_bRotated;
};

class CFxClubImpact : public CFxControl
{
public:
	CFxClubImpact(CPvsFxBox& fxBox, CGolfBall& ball, unsigned long clubSet);
	virtual ~CFxClubImpact();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	CGolfBall& m_ball;
};

class CFxPowerShot2 : public CFxControl
{
public:
	CFxPowerShot2(CPvsFxBox& fxBox, CGolfBall& ball);
	virtual ~CFxPowerShot2();
	virtual bool Process(float elapsed);

	bool m_bDone;
	CPvsFxBox& m_fxBox;
	CGolfBall& m_ball;
};

class CFxBoosterWarp : public CFxControl
{
public:
	CFxBoosterWarp(CPvsFxBox& fxBox, CGolfBall& ball, unsigned char type);
	virtual ~CFxBoosterWarp();
	virtual bool Process(float elapsed);

	CPvsFxBox& m_fxBox;
	CGolfBall& m_ball;
	CFxSequence* m_pSequence[2];
	bool m_bLoaded;
	float m_time;
	unsigned char m_bHarp;
};
#endif

class CFx : public CRenderFuncPtr, public WSingleton<CFx>
{
public:
	using WSingleton<CFx>::m_pInstance;
	CFx();
	virtual ~CFx();

	virtual void Process(float elapsed);
	virtual void Display();

	CFxSequence* OpenSequence(const char* filename, bool bDelete);
	void CloseSequence(CFxSequence* sequence, bool bDeleteChild);
	CFxSpray* OpenSpray(const char* filename);
	void CloseSpray(CFxSpray* spray);
	void* Attach(const Attacher& attacher, const char* filename, bool bMark,
		defines_t* defines);
	void SetActive(bool bActive);
	void ClearAll(bool bMarkedOnly);
	void UpdateExtWind(WVector wind);
	bool FindSequence(CFxSequence* sequence);
	bool GetOutput(WVector& out, unsigned char& value);
#ifdef REBANG_FX_CPP
	void AddFxKeeper(sFxKeeper& keeper);
	void EraseFxKeeper(CFxKeeper* keeper);
	void ClearFxKeeper();
	CFxControl* FindControl(int serial);
	int MakeCobra(const CGolfBall& ball);
	int MakeTomahawk(const CGolfBall& ball, bool bNotify, unsigned long itemId);
	int MakePowerSpin(CGolfBall& ball);
	int MakeShotImpact(CGolfBall& ball);
	int MakeClubImpact(CGolfBall& ball, unsigned long clubSet);
	int MakeSpike(const CGolfBall& ball, bool bNotify);
	int MakePowerShot2(CGolfBall& ball);
	int MakeBoosterWarp(CGolfBall& ball, unsigned char type);
	CFxSpray* GetSpray(int index);
	bool FindSpray(CFxSpray* spray);
#endif
	CFxBox* PvsFxBox983() { return &m_pvsFxBox; } // B1466: cru +0x2c (VC10) anda -4 no cl 14.44
	void SetPvsFxBoxGroundHeight(float maxHeight, float minHeight)
	{
		m_pvsFxBox.SetGroundHeightArea(maxHeight, minHeight);
	}
	int MakeRunDust(const WVector& pos, const WVector& dir);
	bool RemoveRunDust(int handle);

protected:
#ifdef REBANG_FX_CPP
	void ClearControl();
	int AddFxControl(CFxControl* control);
#endif

	CPvsFxBox m_pvsFxBox;
#ifdef REBANG_FX_CPP
	std::map<int, CFxControl*> m_control;
#else
	std::map<int, void*> m_control;
#endif
	bool m_bActive;
};
