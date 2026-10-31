#pragma once

#include "encrypttypes.h"

class CGolfBall
{
public:
	enum eBallState
	{
		BALL_STOP,
		BALL_BOUNCE,
		BALL_SPIN,
		BALL_ROLL,
		BALL_HOLE
	};
	enum eWater
	{
		WATER_NONE,
		WATER_SEA,
		WATER_RIVER,
		WATER_POOL
	};

	enum
	{
		MAX_EXPECTED = 1500,
		EXTRA_EXPECTED = 100
	};

	struct sExpected
	{
		WVector pos;
		WVector vel;
		float curveRot;
		float spinRot;
		char sound[64];
		unsigned char event;
		unsigned char effect;
		WVector normal;
		eBallState state;
		unsigned long hitObj983; // 983 +0x74: sQuadPuppetHolder* do objeto batido (event 5, quadtree.cpp); fx.cpp le no mapa 0x14; o registro tem 0x78 (PreData anda de 0x78 em 0x78; m_holeIn em +0x2bff4)
	};
	CGolfBall();
	~CGolfBall();
	void operator=(const CGolfBall& ball);

	void ClearBuffer();
	void AddCrypticValue();
	void Reset(WVector pos);
	void DbgTraceOutput(const char* msg, float value);
	void DbgTraceToFile(const char* msg, const WVector& pos);

	const sExpected* PreData(int index) const;
	sExpected* PreDataToWrite(int index);
	sExpected* NewPreData();

	float GetMass() const { return m_mass; }
	float GetRadius() const { return m_radius; }
	int IsHoleIn() { return m_holeIn == 1; }
	void SetHoleIn(int bHoleIn) { m_holeIn = bHoleIn; }

	float GetPowerFactor() const
	{
		return (m_initSpecial & 0x50) == 0 ? 1.0f : 1.3f;
	}

	WVector m_pos;
	WVector m_vel;
	eBallState m_state;
	unsigned char m_unused1c[12];
	int m_meshIndex;
	float m_curveRot;
	float m_spinRot;
	float m_initCurve;
	float m_initSpin;
	float m_curve;
	float m_spin;
	unsigned long m_initSpecial;
	unsigned long m_special;
	WVector m_afterBurner;
	bool m_bOB;
	unsigned char m_groundType;
	eWater m_water;
	int m_stayCount;
	int m_renderNum;
	int m_playFrame;
	int m_bounceFrame;
	int m_topFrame;
	int m_nearObjFrame;
	int m_holeFrame;
	int m_holeOutFrame;
	int m_greenFrame;
	int m_afterBurnerFrame;
	int m_rollFrame;
	int m_cupFrame;
	int m_cobraFrame;
	int m_tomahawkFrame;
	int m_vectorSlideFrame;
	int m_spikeFrame;
	int m_spikeDiveFrame;
	int m_spikeEndFrame;
	float m_totalTime;
	int m_attrIndex;
	unsigned char m_standGround;
	short m_prevColObjIndex;
	int m_prevColTriIndex;
	short m_colObjIndex;
	int m_colTriIndex;
	int m_vectorSlideCount;
	float m_mass;
	float m_radius;
	sExpected m_expected[MAX_EXPECTED];
	sExpected* m_pExtra;
	unsigned char m_extraNum;
	WCrypticValue<int> m_holeIn;

	void AddVectorSlideCount() { ++m_vectorSlideCount; }
	int GetVectorSlideCount() { return m_vectorSlideCount; }
	void ResetVectorSlideCount() { m_vectorSlideCount = 0; }
};

CGolfBall& GolfBall();

#include "golfball.inl"
