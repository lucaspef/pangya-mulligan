#pragma once

#include <list>
#include <map>
#include <string>
#include <vector>
#include "actor.h"
#include "scenemanager.h"
#include "wmath.h"
#include "wpuppet.h"

class CCreature;
namespace lua_system { class lua_script; }

class CNPC : public IActor
{
public:
	virtual ~CNPC() { }

	DECLARE_OBJECT(CNPC)

	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();

protected:
	virtual void HandleMsg(const MsgObject& msg);

	CNPC() : m_pCrowdScript983(NULL) { }

	void Destroy();
	void SetGPCrowd983(const Waabb& area, const char* option); // 983 00772630 (HandleMsg 0x1fb)
	void SetNPC(const std::vector<WVector>& points, const char* option);
	void SetNPC(const Waabb& area, char* option);

	std::list<CCreature*> m_creatures;
	lua_system::lua_script* m_pCrowdScript983; // 983 +0x24: "grandprix_crowd.lua" (OnInit 00775a90, Destroy 0076a680)
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbOninitFunclets(); // HACK: EH funclets of the naked OnInit (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSetnpcFunclets(); // HACK: EH funclets of the naked SetNPC (nakedify)
#endif
};

// Catmull-Rom spline through a set of control points, parameterised by
// the given knot values.
class ntCatmullRom
{
public:
	ntCatmullRom();
	~ntCatmullRom();

	bool Init(const WVector* points, const float* knots, int count);
	void Clean();
	WVector Evaluate(float t);
	WVector Velocity(float t);
	WVector Acceleration(float t);
	float ArcLength(float from, float to);
	float FindParameterByDistance(float from, float distance);

protected:
	float SegmentArcLength(int segment, float from, float to);

	WVector* m_pPoints;
	float* m_pKnots;
	float* m_pLengths;
	float m_totalLength;
	int m_count;
};

class CCreature : public CRenderFuncPtr
{
public:
	struct sAlpha
	{
		float alpha;
		float time;
	};

	friend class CNPC;
	virtual ~CCreature();

	bool GetActive() { return m_bActive; }
	Waabb GetArea() { return m_area; }
	void SetType(int type) { m_type = type; }
	int GetType() { return m_type; }
	void SetRadius(float radius) { m_radius = radius; }
	void SetActive(bool bActive) { m_bActive = bActive; }
	WPuppet* GetPet() { return m_pPet; }
	void SetAddTex(int tex) { m_addTex = tex; }

	void DrawDebugLine();
	void SetMotion(const char* name, float time);
	void ParsePuppetScript(const char* script);
	void ScriptProcess(const w_motion_data* motion, float from, float to,
		bool single = false);
	void Replay();
	void Record();

	virtual void Display();
	virtual void Process(float delta);
	virtual void SetArea(const Waabb& area, unsigned long flags);
	virtual void SetArea(const std::vector<WVector>& points, float speed,
		unsigned long flags);
	virtual void PlayEvent();
	virtual void CheckTarget(float delta);

protected:
	CCreature()
		: m_flags(0),
		  m_pPet(NULL),
		  m_motionTime(0),
		  m_pMotion(NULL),
		  m_pCells(NULL),
		  m_bPetBillboard983(false),
		  m_radius(0)
	{
	}
	void Init(const char* name, unsigned long layer);
	void InitFx();
	void BillboardToCamera983(); // 00764230

	unsigned int m_flags;
	Waabb m_area;
	WPuppet* m_pPet;
	int m_addTex;
	WMatrix m_mat;
	float m_motionTime;
	int m_type;
	w_motion_data* m_pMotion;
	WVector m_target;
	float m_f70;
	float m_speed;
	float m_angle;
	float m_f7c;
	float m_f80;
	float m_cellSize;
	int m_gridX;
	int m_gridY;
	int m_gridZ;
	float m_cellX;
	float m_cellY;
	float m_cellZ;
	int m_cellCount;
	int* m_pCells;
	unsigned char m_alpha;
	bool m_bActive;
	bool m_bPetBillboard983; // +0x11e: "*petbillboard" do script (00769d83); o Process vira o modelo para a camera
	float m_radius;
	std::list<sAlpha> m_alphaList;
	std::vector<WVector> m_path;
	unsigned char m_nodeIndex;
	std::vector<WVector> m_recPath;
	unsigned char m_recIndex;
	WMatrix m_recMatrix;
	float m_recTime;
	float m_recAngle;
	bool m_recActive;
	bool m_bRecorded;
};

class CGull : public CCreature
{
public:
	CGull(const char* name);
	virtual ~CGull();
};

class CButterfly : public CCreature
{
public:
	CButterfly(const char* name);
	virtual ~CButterfly();
};

class CDuck : public CCreature
{
public:
	CDuck(const char* name, int count);
	virtual ~CDuck()
	{
		if (m_pItems)
		{
			delete[] m_pItems;
			m_pItems = NULL;
		}
	}

	virtual void SetArea(const Waabb& area, unsigned long flags);
	virtual void Process(float delta);
	virtual void Display();

protected:
	int m_count;
	WMatrix* m_pItems;
};

class CMole : public CCreature
{
public:
	CMole(const char* name);
	virtual ~CMole() { }

	virtual void PlayEvent();
};

class CDolphin : public CCreature
{
public:
	CDolphin(const char* name);
	virtual ~CDolphin() { }

	virtual void PlayEvent();
};

class CCrab : public CCreature
{
public:
	CCrab(const char* name);
	virtual ~CCrab() { }

	virtual void PlayEvent();
};

class CSparrow : public CCreature
{
public:
	CSparrow(const char* name, int count);
	virtual ~CSparrow()
	{
		if (m_pItems)
		{
			delete[] m_pItems;
			m_pItems = NULL;
		}
	}

	virtual void Display();
	virtual void PlayEvent();

protected:
	int m_maxCount;
	int m_count;
	WMatrix* m_pItems;
};

class CSanta : public CCreature
{
public:
	CSanta(const char* name);
	virtual ~CSanta();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	float m_climbRate;
	int m_state;
};

class CFighter : public CCreature
{
public:
	CFighter(const char* name);
	virtual ~CFighter();

	virtual void CheckTarget(float delta);

protected:
	float m_climbRate;
	int m_state;
};

class CUfo : public CCreature
{
public:
	CUfo(const char* name);
	virtual ~CUfo();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	float m_climbRate;
	int m_state;
};

class ntPenguin : public CCreature
{
public:
	ntPenguin(const char* name);
	virtual ~ntPenguin();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	float m_climbRate;
	int m_state;
};

class ntPenguin_Sailor : public CCreature
{
public:
	ntPenguin_Sailor(const char* name);
	virtual ~ntPenguin_Sailor();

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	float m_f120;
	bool m_b124;
};

class ntPenguin_Duck : public CCreature
{
public:
	ntPenguin_Duck(const char* name, int count);
	virtual ~ntPenguin_Duck();

	virtual void SetArea(const Waabb& area, unsigned long flags);
	virtual void Display();
	virtual void Process(float delta);

protected:
	bool m_b120;
	int m_count;
	WMatrix* m_pItems;
	float* m_pValues;
};

class ntNpc_Dragon : public CCreature
{
public:
	ntNpc_Dragon(const char* name, float speed);
	virtual ~ntNpc_Dragon();
	virtual void SetArea(const std::vector<WVector>& points, float speed,
		unsigned long flags);

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	int m_i120;
	char m_bWait;
	float m_waitEnd;
	float m_dist;
	ntCatmullRom m_spline;
};

class ntNpc_BlackSparrow : public CCreature
{
public:
	ntNpc_BlackSparrow(const char* name);
	virtual ~ntNpc_BlackSparrow();

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	bool m_b120;
};

class ntNpc_PolaShip : public CCreature
{
public:
	ntNpc_PolaShip(const char* name, float speed);
	virtual ~ntNpc_PolaShip();
	virtual void SetArea(const std::vector<WVector>& points, float speed,
		unsigned long flags);

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	int m_i120;
	char m_bWait;
	float m_waitEnd;
	float m_dist;
	ntCatmullRom m_spline;
};

class ntNpc_PolaPenguin1 : public CCreature
{
public:
	ntNpc_PolaPenguin1(const char* name, float speed);
	virtual ~ntNpc_PolaPenguin1();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	void FindTarget();
};

class ntNpc_PolaPenguin2 : public CCreature
{
public:
	ntNpc_PolaPenguin2(const char* name);
	virtual ~ntNpc_PolaPenguin2();

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	void FindTarget();
	bool IsSlidable();

	bool m_b120;
};

class CNPC_BIRD : public CCreature
{
public:
	CNPC_BIRD(const char* name, float speed);
	virtual ~CNPC_BIRD();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	void FindTarget();
};

// 983 (vtable 00d228a8, sizeof 0x120): *type 0x1c/0x1e do SetNPC(Waabb); copia do CNPC_BIRD
class ntNpc_DLCreature : public CCreature
{
public:
	ntNpc_DLCreature(const char* name, float speed); // 00770dd0
	virtual ~ntNpc_DLCreature();

	virtual void Process(float delta); // 0076aac0
	virtual void CheckTarget(float delta); // 007683a0

protected:
	void FindTarget(); // 00764e40
};

class CNPC_PARROT : public CCreature
{
public:
	CNPC_PARROT(const char* name);
	virtual ~CNPC_PARROT();
};

class CNPC_TORTOISE : public CCreature
{
public:
	CNPC_TORTOISE(const char* name, float speed);
	virtual ~CNPC_TORTOISE();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	float m_climbRate;
	int m_state;
};

class CNPC_TRSHIP : public CCreature
{
public:
	CNPC_TRSHIP(const char* name, float speed);
	virtual ~CNPC_TRSHIP();
	virtual void SetArea(const std::vector<WVector>& points, float speed,
		unsigned long flags);

	virtual void Display();
	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

	// 983 0076bf50: motion played when the wait ends (+0x140)
	void SetMotionName(const char* name) { m_motionName.assign(name, strlen(name)); }

protected:
	float m_dist;
	float m_f124;
	bool m_bWait;
	ntCatmullRom m_spline;
	std::string m_motionName; // 983 +0x140, sizeof 0x15c
};

class ntNpc_NodeWalker : public CCreature
{
public:
	ntNpc_NodeWalker(const char* name, float speed);
	virtual ~ntNpc_NodeWalker() = 0;
	virtual void SetArea(const std::vector<WVector>& points, float speed,
		unsigned long flags);

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	void MakeSpline(const std::vector<WVector>& points, float& length,
		ntCatmullRom& spline);

	float m_length;
	float m_f124;
	ntCatmullRom m_spline;
};

class ntNpc_WizCityNode : public ntNpc_NodeWalker
{
public:
	ntNpc_WizCityNode(const char* name, float speed);

	virtual void Display();
	void SetScale(float scale) { m_scale = scale; }

protected:
	float m_scale;
};

class CDisposable : public CCreature
{
public:
	CDisposable(const char* name);
	virtual ~CDisposable() { }

	virtual void PlayEvent();
};

class COnGameNet : public CCreature
{
public:
	COnGameNet(const char* name, unsigned char type);
	virtual ~COnGameNet();

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	unsigned char m_playerIndex;
	WMatrix m_playerMatrix;
	int m_hTexture;
};

class ntNpc_AreaWalker : public CCreature
{
public:
	struct MotionMap
	{
		int index;
		std::string name;
	};
	typedef std::map<std::string, std::list<MotionMap> > MotionMapTable;

	ntNpc_AreaWalker(const char* name);
	virtual ~ntNpc_AreaWalker() = 0;

	virtual void Process(float delta);
	virtual void CheckTarget(float delta);

protected:
	void FindTarget();
	void SelectNextMotion();
	void AddWalkingMotion(const char* name);
	virtual void ConstructNextMotion() = 0;

	std::vector<std::string> m_walkingMotions;
	bool m_walking;
	MotionMapTable m_motions;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbSelectnextmotionFunclets(); // HACK: EH funclets of the naked SelectNextMotion (nakedify)
#endif
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbAddwalkingmotionFunclets(); // HACK: EH funclets of the naked AddWalkingMotion (nakedify)
#endif
};

class ntNpc_WizCityArea : public ntNpc_AreaWalker
{
public:
	ntNpc_WizCityArea(const char* name);

	virtual void Display();
	void SetScale(float scale) { m_scale = scale; }

protected:
	virtual void ConstructNextMotion();

	float m_scale;
};
