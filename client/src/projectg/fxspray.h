#pragma once

struct defines_t
{
	char name[16][260];
	float value[16];
	int num;
};

// HACK: full layout only for fxspray.cpp and the REBANG_FXSEQUENCE_CPP units
// (fxsequence.cpp, sky.cpp); the other includers (polysoup, netresourcemanager,
// projectg) change their code with the extra declarations.
// D858: fora do VC10 (cl 14.44) o STL e menor e os tocos de bytes das visoes parciais (tamanhos do VC10) deslocam
// os campos entre unidades (vs2022-1: CFxSequence::LoadFile lia m_sequenceCache do CFxBox no lugar errado). La todas
// as unidades usam a visao completa; no VC10 (983) nada muda.
#if defined(REBANG_FXSPRAY_CPP) || defined(REBANG_FXSEQUENCE_CPP) || _MSC_VER > 1600
#include <list>
#include "ringq.h"

class CFxBox;
struct Attacher;
class CPetBody;
class W3dAniSpr;
class CFxSequence;
class WView;
struct CFxParticle;
struct LightSet;

struct CFxParticle
{
public:
	struct sTail
	{
		WVector m_pos; // 0x00
		int m_frame;   // 0x0c
	};
	struct sBandana
	{
		WVector m_left;  // 0x00
		WVector m_right; // 0x0c
		float m_len;     // 0x18
	};

	CFxParticle();
	virtual ~CFxParticle();

	bool m_bNoWind; // 0x04
	bool m_bGrounded; // 0x05
	unsigned char m_unused6[2];
	WVector m_origin;           // 0x08
	WVector m_unused983_14;      // 0x14 (983: só zerado no InitParticle 00980ddd; sem leitura em fxspray 00976b50-0098a9c0)
	WVector m_pos;              // 0x20
	WMatrix m_rot;              // 0x2c
	WVector m_offset;           // 0x5c
	std::list<WVector> m_trail; // 0x68
	WVector m_spawnDir;         // 0x74
	float m_windRatio;              // 0x80
	WVector m_rotSpeed;            // 0x84
	WVector m_angle;            // 0x90
	int m_frame;                // 0x9c
	float m_lastFrameTime;              // 0xa0 (983: float, Process 00987420)
	float m_lifetime;              // 0xa4
	float m_lifeLeft;              // 0xa8
	int m_killTime;                // 0xac (983: novo; = CFxSpray+0x1594)
	float m_size0;              // 0xb0
	float m_size;               // 0xb4
	float m_ratio;              // 0xb8
	union
	{
		unsigned long m_color;   // 0xbc
		unsigned char m_rgba[4]; // b, g, r, a
	};
	W3dAniSpr* m_pSprite;       // 0xc0
	CPetBody* m_pPet;           // 0xc4
	TRingQ<sTail> m_tail;       // 0xc8
	TRingQ<sBandana> m_bandana; // 0xd8
	float m_lastTail;           // 0xe8
	float m_alpha;              // 0xec
	int m_nSpawn;               // 0xf0
	float m_lastSpawnTime;              // 0xf4
};

typedef std::list<CFxParticle*> CFxParticleList;
typedef char rb_inst_CFxParticleList[sizeof(CFxParticleList)];

class CFxSpray
{
#ifdef REBANG_FXBOX_CPP
	friend class CFxBox;
#endif

public:
	// Element sizes come from the list node allocators (node - 8)
	struct sInit_WVector
	{
		WVector min; // 0x00
		WVector max; // 0x0c
	};
	struct sVel
	{
		int m_start;      // 0x00
		float m_startVal; // 0x04
		int m_end;        // 0x08
		float m_endVal;   // 0x0c
	};
	struct sRot
	{
		float m_start;    // 0x00
		float m_startVal; // 0x04
		float m_end;      // 0x08
		float m_endVal;   // 0x0c
	};
	struct sNozzleRot
	{
		WVector m_axis;   // 0x00
		int m_start;      // 0x0c
		float m_startVal; // 0x10
		int m_end;        // 0x14
		float m_endVal;   // 0x18
	};
	struct sRevol
	{
		WVector m_center; // 0x00
		WVector m_axis;   // 0x0c
		int m_start;      // 0x18
		float m_startVal; // 0x1c
		int m_end;        // 0x20
		float m_endVal;   // 0x24
	};
	struct sGen
	{
		int m_start;      // 0x00
		int m_end;        // 0x04
		int m_count;      // 0x08
		float m_rate;     // 0x0c
		int m_emitted;    // 0x10
		float m_lastTime; // 0x14
	};
	struct sGravity
	{
		WVector m_pos;  // 0x00
		float m_near;   // 0x0c
		float m_radius; // 0x10
		float m_far;    // 0x14
	};
	struct sFade
	{
		float m_time;                // 0x00
		unsigned char m_color[4];    // 0x04: a, r, g, b
		float m_end;                 // 0x08
		unsigned char m_endColor[4]; // 0x0c: a, r, g, b
	};
	struct sSize
	{
		float m_start;    // 0x00
		float m_startVal; // 0x04
		float m_end;      // 0x08
		float m_endVal;   // 0x0c
	};
	struct sFrame
	{
		float m_start;    // 0x00
		float m_end;      // 0x04
		int m_first;      // 0x08
		int m_last;       // 0x0c
		float m_interval; // 0x10
	};
	struct sQuake
	{
		WVector m_amp;      // 0x00
		int m_start;        // 0x0c
		WVector m_startVec; // 0x10
		int m_end;          // 0x1c
		WVector m_endVec;   // 0x20
		float m_interval;   // 0x2c
		float m_lastTime;   // 0x30
	};
	struct sFlash
	{
		int m_start; // 0x00
		int m_mid;   // 0x04
		int m_end;   // 0x08
	};
	struct sBubble
	{
		WVector m_amp;  // 0x00
		WVector m_freq; // 0x0c
	};
	struct sSpawn
	{
		int m_type;         // 0x00
		char m_name[0x104]; // 0x04
		bool m_bAttach;     // 0x108
		unsigned char m_unused109[3];
		float m_interval; // 0x10c
		int m_max;        // 0x110
	};
	struct sSound
	{
		char m_name[0x104]; // 0x00
		float m_volume;     // 0x104
	};

	// Parameter block, at CFxSpray+0x11c0 (sizeof == 0x3d4)
	struct Param
	{
	public:
		bool m_bBackground;                // 0x00
		bool m_bChild;                     // 0x01
		bool m_bBillboard;                      // 0x02
		bool m_bAttach;                    // 0x03
		int m_attachAxis;                        // 0x04
		W3dAniSpr* m_pSprite;              // 0x08
		char m_spriteName[0x104];          // 0x0c
		bool m_bPetLight;                    // 0x110
		bool m_bPetBillboard;              // 0x111 (983: "petbillboard"; fica ao lado de m_bPetLight)
		bool m_bSphere;                    // 0x112 (983; no 645 era 0x111)
		Waabb m_aabb;                      // 0x114
		bool m_bRegenAtSamePos;                    // 0x12c
		float m_zBias;                     // 0x130
		int m_lifetime;                    // 0x134
		int m_lifetime2;                   // 0x138
		float m_alpha;                     // 0x13c
		float m_coreMin;                   // 0x140
		float m_coreMax;                   // 0x144
		float m_friction;                  // 0x148
		float m_groundFriction;            // 0x14c
		WVector m_gravityVec;              // 0x150
		float m_groundX;                   // 0x15c
		float m_groundY;                   // 0x160
		float m_groundZ;                   // 0x164
		int m_blendSrc;                    // 0x168
		int m_blendDst;                    // 0x16c
		unsigned long m_lightType;         // 0x170
		unsigned long m_lightColor;        // 0x174
		int m_initFrame0;                  // 0x178
		int m_initFrame1;                  // 0x17c
		int m_bTail;                       // 0x180
		float m_tailInterval;              // 0x184
		unsigned char m_tailColor[4];      // 0x188: a, r, g, b
		char m_tailTexture[0x104];         // 0x18c
		int m_bandana;                     // 0x290
		float m_bandanaWidth;              // 0x294
		bool m_bBandanaMonolith;                    // 0x298
		bool m_bBandanaCameraSide;                    // 0x299
		sInit_WVector m_initVel;           // 0x29c
		float m_windEnable[2];             // 0x2b4
		WVector m_wind;                    // 0x2bc
		unsigned int m_extOrient;          // 0x2c8
		std::list<sVel> m_vel;             // 0x2cc
		sInit_WVector m_initAngle;         // 0x2d8
		sInit_WVector m_initRot;           // 0x2f0
		std::list<sRot> m_rot;             // 0x308
		std::list<sNozzleRot> m_nozzleRot; // 0x314
		std::list<sNozzleRot> m_rotate;    // 0x320
		std::list<sRevol> m_revol;         // 0x32c
		std::list<sRevol> m_revolGrand;    // 0x338
		std::list<sGen> m_gen;             // 0x344
		std::list<sGravity> m_gravity;     // 0x350
		std::list<sFade> m_fade;           // 0x35c
		float m_initSize0;                 // 0x368
		float m_initSize1;                 // 0x36c
		float m_initSize2;                 // 0x370
		std::list<sSize> m_size;           // 0x374
		std::list<sFrame> m_frame;         // 0x380
		std::list<sQuake> m_quake;         // 0x38c
		std::list<sFlash> m_flash;         // 0x398
		float m_distNear;                  // 0x3a4
		float m_distFar;                   // 0x3a8
		std::list<sBubble> m_bubble;       // 0x3ac
		std::list<sSpawn> m_spawn;         // 0x3b8
		std::list<sSound> m_sound;         // 0x3c4
		bool m_bRing;                    // 0x3d0 (issphere 2)
	};

	CFxSpray(CFxBox& box);
	virtual ~CFxSpray();

	WVector& Pos() { return m_pos; }
#ifdef REBANG_FX_CPP // HACK: only fx.cpp emits these two accessors
	WVector& InitialMinVel() { return m_param.m_initVel.min; }
	WVector& InitialMaxVel() { return m_param.m_initVel.max; }
#endif
#ifdef REBANG_GOLFRULE_PCH
	WVector& Vel() { return m_offset; }
#endif
	void Reset();
	void SetLight(const LightSet& light);
	void SetDistLimit(float nearDist, float farDist);
	void SetWind(const WVector& wind);
	bool ParseFile(const char* filename, bool useCache);
	bool ParseLine(const char* line);

protected:
	bool Process(float elapsed);
	void MakeTail(float time, CFxParticle* particle, const WVector& pos);
	void DrawSprite(WView* view, CFxParticle* particle, const WVector& pos);
	void DrawTail(WView* view, CFxParticle* particle, const WVector& pos);
	void DrawBandana(WView* view, CFxParticle* particle, const WVector& pos);
	void Display(WView* view);
	void Spawn(const sSpawn& spawn, CFxParticle* particle);
	bool Generate(float time);
	bool InitParticle(bool bChild, CFxParticle* particle, float speed,
		float rate, Waabb& area, bool bInit);
	void GetBoneMatrix(WMatrix* mat);
	WMatrix Rotate(float time, float scale);
	WMatrix RotateNozzle(float time, float scale);
	void Revolute(float time, float scale);
	void RevoluteGrand(float time, float scale);
	WMatrix GetBgRot(const CFxParticle* particle);
	WVector GetBgPos();
	WVector GetBgPos(const CFxParticle* particle);
	unsigned long GetParticleColor(const WView* view, const WVector& pos,
		const CFxParticle* particle, unsigned long color);

public:
	bool m_bMark;                        // 0x04 (bMark do CFx::Load*; CFxBox::ClearSpray poupa)
	const Attacher* m_pAttacher;         // 0x08
	float m_attachX;                     // 0x0c
	float m_attachY;                     // 0x10
	WMatrix* m_attachMat[4];             // 0x14
	WVector* m_attachPos[4];             // 0x24
	unsigned char m_defines[0x1084];       // 0x34
	char m_filename[0x40];               // 0x10b8
	CFxBox* m_pFxBox;                    // 0x10f8
	std::list<CFxParticle*> m_particles; // 0x10fc
	CFxSequence* m_pSequence;            // 0x1108
	bool m_bActive;                      // 0x110c
	float m_time;                        // 0x1110
	WVector m_quakeShake;                   // 0x1114
	WVector m_quakeCenter;                   // 0x1120
	unsigned char m_flashAlpha;          // 0x112c
	unsigned char m_unused112d[3];
	int m_nDrawn;        // 0x1130
	int m_nTailDrawn;       // 0x1134
	WVector m_pos;       // 0x1138
	WVector m_drawPos;   // 0x1144
	WVector m_offset;    // 0x1150
	CFxSpray* m_pParent; // 0x115c
	WMatrix m_bgRot;     // 0x1160
	WMatrix m_rot1190;   // 0x1190
	Param m_param;       // 0x11c0
	int m_killTime;       // 0x1594
	int m_pad983_1598;   // 0x1598 (983: novo; sizeof 0x15a0; sem leitura/escrita: ctor 00986760, fxspray/fxbox/fxsequence/fx/particleparty)
	float m_ringAngle;     // 0x159c (no 645 era 0x1598)
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbGenerateFunclets(); // HACK: EH funclets of the naked Generate (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbInitparticleFunclets(); // HACK: EH funclets of the naked InitParticle (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbParselineFunclets(); // HACK: EH funclets of the naked ParseLine (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbParsefileFunclets(); // HACK: EH funclets of the naked ParseFile (nakedify)
#endif
};

#include "fxspray.inl"
#else
class CFxSpray
{
public:
	WVector& Pos() { return m_pos; }
#ifdef REBANG_GOLFRULE_PCH
	WVector& Vel() { return m_vel; }
#endif
	void SetWind(const WVector& wind);
	void SetLight(const LightSet& light);

	unsigned char m_unused0[0xc];
	float m_attachX; // 0x0c
	float m_attachY; // 0x10
	unsigned char m_unused14[0x110c - 0x14];
	bool m_bActive;
	unsigned char m_unused110d[0x2b];
	WVector m_pos;
#ifdef REBANG_GOLFRULE_PCH
	unsigned char m_unused1144[0xc];
	WVector m_vel;
#endif
};
#endif
