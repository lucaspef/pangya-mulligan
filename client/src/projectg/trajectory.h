#pragma once

#include "actor.h"
#include "scenemanager.h"

// Base of the ball flight effects (comet, rainbow track, skid marks, ...). It
// owns a textured quad that the derived classes refill for every segment.
class CBallTrace
{
public:
	CBallTrace();
	virtual ~CBallTrace() { }

	virtual void Init() = 0;
	virtual void Process(float elapsed) = 0;
	virtual void Display() = 0;

protected:
	virtual void Generate() { }
	virtual void BlowAway(float power, int count) { }

	int m_unused4;
	int m_texHandle;
	int m_count;
	int m_head;
	float m_time;
	float m_interval;
	bool m_bActive;
	WTVertex* m_pVtx[5];
	WTVertex m_vtx[4];
};

class CBallTrack : public CBallTrace
{
public:
	struct sPoint
	{
		WVector left;
		WVector right;
		float alpha;

		sPoint() { alpha = 1.0f; }
	};

	struct sPointEx
	{
		unsigned char bActive;
		float age;
		WVector pos;
		WVector dir;
	};

	CBallTrack();
	virtual ~CBallTrack();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

private:
	void UpdateRainbowEffect(float elapsed);

	sPoint* m_pPoint;
	bool m_bRainbow;
	sPointEx* m_pPointEx;
	int m_rainbowTex;
	int m_pointNum;
	float m_alpha;
	float m_fadeStart;
	float m_fadeTime;
	float m_maxAlpha;
	int m_lastFrame;
};

class CSkidMark : public CBallTrace
{
public:
	struct sPoint
	{
		WVector left;
		WVector right;
		unsigned char alpha;
	};

	CSkidMark();
	virtual ~CSkidMark();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

	void SetFrame(int startFrame, int endFrame);

protected:
	unsigned char GetPointAlpha(int index, int frame);

private:
	sPoint* m_pPoint;
	int m_pointMax;
	int m_pointNum;
	int m_startFrame;
	int m_endFrame;
	int m_lastFrame;
};

class CBallWindTrack : public CBallTrace
{
public:
	struct sLineEffect
	{
		WVector from;
		WVector to;
		int alpha;
		float length;
		bool bActive;
	};

	CBallWindTrack();
	virtual ~CBallWindTrack();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

private:
	sLineEffect* m_pLine;
	CBallTrack m_track;
	float m_alpha;
	WVector m_prevCamera;
	WVector m_dir;
};

class CComet : public CBallTrace
{
public:
	CComet();
	virtual ~CComet();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

protected:
	virtual void Generate();

private:
	WVector m_line[20][2];
	float m_length;
	float m_unused2b8;
	float m_alpha;
	int m_cometTex;
	int m_bTail;
	WVector m_tailLeft;
	WVector m_tailRight;
};

class CDoughnut : public CBallTrace
{
public:
	struct sParticle
	{
		float scale;
		float alpha;
		bool bActive;
		WMatrix matrix;
	};

	CDoughnut();
	virtual ~CDoughnut();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

private:
	sParticle* m_pParticle;
};

class CPuttLine : public CBallTrace
{
public:
	struct sPoint
	{
		int alpha;
		WVector left;
		WVector right;
	};

	CPuttLine();
	virtual ~CPuttLine();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

	bool IsKeep();
	int HowOldAreYou();
	void RemoveUnkeptAlpha();

private:
	sPoint* m_pPoint;
	int m_pointNum;
	unsigned long m_startTime;
};

class ntSkidMark_Cobra : public CBallTrace
{
public:
	struct sPoint
	{
		WVector left;
		WVector right;
	};

	ntSkidMark_Cobra();
	virtual ~ntSkidMark_Cobra();

	virtual void Init();
	virtual void Process(float elapsed);
	virtual void Display();

	void SetFrame(int startFrame, int endFrame);

private:
	sPoint* m_pPoint;
	int m_pointMax;
	int m_pointNum;
	int m_startFrame;
	int m_endFrame;
	int m_lastFrame;
};

class ntExplodeMark
{
public:
	ntExplodeMark();
	~ntExplodeMark();

	void Init(const WVector& pos);
	void Uninit();
	void Process();
	void Display();

private:
	bool m_bActive;
	int m_texHandle;
	WVector m_pos;
};

class CTrajectory : public IActor, public CRenderFuncPtr
{
public:
	virtual ~CTrajectory() { }

	DECLARE_OBJECT(CTrajectory)

public:
	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);
	virtual void Display();

protected:
	CTrajectory();

private:
	CBallTrace* m_pActive[3]; // 983: 3 elementos (0x1c, 0x20, 0x24)
	CComet* m_pComet;
	CDoughnut* m_pDoughnut;
	CPuttLine* m_pPuttLine;
	CBallTrack* m_pBallTrack;
	CSkidMark* m_pSkidMark;
	CBallWindTrack* m_pWindTrack;
	CPuttLine* m_pPutt[32];
	CPuttLine* m_pCurPutt;
	ntSkidMark_Cobra* m_pCobra;
	ntExplodeMark* m_pExplode;
};
