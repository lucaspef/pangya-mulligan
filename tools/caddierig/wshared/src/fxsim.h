// SPDX-License-Identifier: MIT
// fxsim.h - sequences and sprays, the client's effect system without its renderer.
//
// A port of CFxBox, CFxSequence and CFxSpray. Times are milliseconds, as in the
// client. The viewport draws what this simulates; nothing here touches OpenGL, so
// crigc can run the same code headless.
#ifndef FXSIM_H
#define FXSIM_H

#include <deque>
#include <list>
#include <map>
#include <string>
#include <vector>
#include "wmath.h"

// Whatever a host draws a *pet particle with; fxsim only carries the pointer.
struct FxPetModel;
class FxBox;
class FxSequence;

// Everything the simulation needs from the application. Names are CP949.
class FxHost
{
public:
	virtual ~FxHost() { }
	virtual std::string ResolveAsset(const std::string& name) = 0;	// UTF-8 path or empty
	virtual void PlaySound(const std::string& name, float volume) = 0;
	virtual FxPetModel* LoadPetModel(const std::string& name) = 0;
};

// CFxBox::LoadSprite: one texture cut into equal cells, loaded by the viewport.
struct FxSprite
{
	std::string path;
	int cellW, cellH;
	unsigned int texture;
	int texW, texH;
	bool tried;
};

// Up to four bones and/or points whose positions are averaged (Attacher::Get).
struct FxAttacher
{
	int count;
	int bone[4];		// -1 for none
	bool hasPos[4];
	WVector pos[4];

	FxAttacher() : count(0)
	{
		for (int i = 0; i < 4; ++i)
		{
			bone[i] = -1;
			hasPos[i] = false;
		}
	}
	bool Get(const std::vector<WMatrix>& world, WVector* out) const;
	WMatrix BoneRotation(const std::vector<WMatrix>& world) const;
};

struct FxDefines
{
	std::vector<std::string> name;
	std::vector<float> value;
};

struct FxBand { WVector pos[2]; float length; };
struct FxTail { WVector pos; int frame; };

struct FxParticle
{
	bool bStop, bGround;
	WVector pos, bgPos;
	WMatrix bgRot;
	WVector bubble;
	std::vector<WVector> bubblePhase;
	WVector vel;
	float wind;
	WVector rotVel, rot;
	int frame;
	float frameTime, lifeTime, life, initSize, size, ratio;
	unsigned int color;		// ARGB
	std::deque<FxTail> tail;
	std::deque<FxBand> bandana;
	float tailTime, volOpacity;
	int spawnNum;
	float spawnTime;
};

struct FxSprayParam
{
	struct Vel { int start; float startRate; int end; float endRate; };
	struct Rot { float start, startRate, end, endRate; };
	struct NozzleRot { WVector axis; int start; float startSpeed; int end; float endSpeed; };
	struct Revol { WVector center, axis; int start; float startSpeed; int end; float endSpeed; };
	struct Gen { int start, end, num; float spread; int count; float time; };
	struct Gravity { WVector pos; float power, radius, outerPower; };
	struct Fade { float start; unsigned char startColor[4]; float end; unsigned char endColor[4]; };
	struct Size { float start, startSize, end, endSize; };
	struct Frame { float start, end; int startFrame, endFrame, interval; };
	struct Quake { WVector pos; int start; WVector startPower; int end; WVector endPower; int interval; float time; };
	struct Flash { int start, peak, end; };
	struct Bubble { WVector amplitude, frequency; };
	struct Spawn { int type; std::string name; bool attached; float time; int count; };
	struct Sound { std::string name; float range; };

	FxSprayParam();

	bool bStrongBond, bSpawnAttached, bBillboard, bAttachedToParent;
	int attachedAxis;
	FxSprite* sprite;
	std::string petName;
	FxPetModel* petModel;
	bool bPetLight, bSphere;
	WVector genMin, genMax;
	bool bRegenAtSamePos;
	float camOffset;
	int lifeTime[2];
	float volMaxOpacity, volCore[2], friction, groundFriction;
	WVector gravity;
	float groundHeight, groundBounce, groundRadius;
	unsigned int blend[2];
	int lighting;
	int initFrame[2];
	int tailNum;
	float tailInterval;
	unsigned char tailColor[4];
	FxSprite* tailTexture;
	float tailTexScale;
	bool bMonolith, bCameraSide;
	WVector initVelMin, initVelMax;
	float addWind[2];
	WVector extWind;
	int extOrientation;
	std::vector<Vel> velocity;
	WVector initAngleMin, initAngleMax, initRotMin, initRotMax;
	std::vector<Rot> rotation;
	std::vector<NozzleRot> nozzleRotation, sprayRotation;
	std::vector<Revol> revolution, preRevolution;
	std::vector<Gen> generation;
	std::vector<Gravity> gravityPoint;
	std::vector<Fade> fade;
	float initSize[2], initRatio;
	std::vector<Size> size;
	std::vector<Frame> frame;
	std::vector<Quake> quake;
	std::vector<Flash> flash;
	float distLimit[2];
	std::vector<Bubble> bubble;
	std::vector<Spawn> spawn;
	std::vector<Sound> sound;
	bool bEvenSphere;
};

class FxSpray
{
public:
	explicit FxSpray(FxBox& box);
	~FxSpray();

	bool Process(float elapsed);
	WVector GetBgPos() const;
	WVector GetBgPos(const FxParticle* p) const;
	WMatrix GetBgRot(const FxParticle* p) const;
	unsigned int ParticleColor(const WVector& pos, const FxParticle* p, unsigned int color) const;

	FxBox& m_box;
	FxSprayParam m_param;
	FxSequence* m_parent;
	FxSpray* m_spawnParent;
	FxAttacher* m_attacher;
	std::string m_filename;
	bool m_bMark, m_bActive;
	float m_time;
	WVector m_pos, m_vel, m_quake, m_quakePos;
	unsigned char m_flash;
	WMatrix m_rot, m_nozzleRot;
	float m_sphereAngle;
	std::list<FxParticle*> m_particle;

private:
	bool Generate(float time);
	void InitParticle(bool bRegen, FxParticle* p, float velRate, float spread, const WVector& coreMin, const WVector& coreMax, bool bFirst);
	void SpawnFrom(const FxSprayParam::Spawn& spawn, FxParticle* p);
	WMatrix RotateSpray(float time, float dt);
	WMatrix RotateNozzle(float time, float dt);
	void Revolute(float time, float dt);
	void RevoluteGrand(float time, float dt);
	void MakeTail(float time, FxParticle* p, const WVector& pos);
};

class FxSequence
{
public:
	struct Line
	{
		int number;
		int start, end, time;	// time: -1 label, -2 runs immediately
		std::string text;
	};

	FxSequence(FxBox& box);
	~FxSequence();
	bool LoadFile(const std::string& name);
	bool Process(float elapsed, bool* error);

	FxBox& m_box;
	FxAttacher* m_attacher;
	bool m_bMark;
	WVector m_pos;
	WMatrix m_rot;
	float m_time;
	FxDefines m_defines;
	std::vector<FxSpray*> m_spray;
	FxSpray* m_lastSpray;
	std::list<Line> m_lines;
	std::list<Line>::iterator m_current;
	std::string m_filename;
	Line m_lastError;

private:
	void AddLine(const std::string& text, int number);
	void SetLineTime();
	bool ParseLine(const std::string& text, bool* error);
};

class FxBox
{
public:
	explicit FxBox(FxHost* host);
	~FxBox();

	void Clear();					// everything, and the file caches
	void ClearAll(bool markedOnly);
	void StopSequences(bool markedOnly);	// sprays already emitted finish on their own
	void StopFile(const std::string& file);	// *fxstop
	void ResetTrails();						// the timeline jumped: don't draw a tail across it
	bool Attach(const FxAttacher& attacher, const std::string& file, bool mark);
	void Process(float elapsedMs, const std::vector<WMatrix>& world);

	bool Finished() const { return m_sequence.empty() && m_spray.empty(); }
	int ParticleCount() const;
	void GetOutput(const WVector& cameraPos, WVector* quake, int* flash) const;

	// Filled in by the viewport each frame, in game coordinates.
	WVector m_camPos, m_camForward;

	std::list<FxSpray*> m_spray;
	std::list<FxSequence*> m_sequence;
	std::map<std::string, FxSprite*> m_sprite;

	FxHost* Host() { return m_host; }
	float ParseNumber(const std::string& text) const;
	float Random(float lo, float hi) const;
	WVector Random(const WVector& lo, const WVector& hi) const;
	WVector RandomSphere(const WVector& center, float minRadius, float maxRadius, float angle) const;
	FxSprite* LoadSprite(const std::string& name, int w, int h);
	bool PreprocessDefines(FxDefines& defines, const std::string& text) const;
	void ProcessDefines(const FxDefines& defines, std::string* text) const;
	void ClearDefines(FxDefines& defines) const;
	void SplitArgs(const std::string& text, std::string* command, std::string args[16]) const;
	const std::vector<std::string>* FileLines(const std::string& name);
	bool LoadSprayParam(const std::string& name, FxSprayParam* out);
	FxSpray* LoadSpray(const std::string& name);
	void AddSpray(FxSpray* spray) { m_spray.push_back(spray); }
	void DeleteSpray(FxSpray* spray);
	void DeleteSequence(FxSequence* seq, bool deleteChild);

	// What a run could not do, for the log and for crigc. Names are CP949.
	std::vector<std::string> m_problems;
	void Problem(const std::string& text);
	static const float FRAME_TIME;

private:
	bool ParseSprayLine(const std::string& line, FxSprayParam* param, FxDefines& defines);

	FxHost* m_host;
	std::map<std::string, FxSprayParam*> m_sprayCache;
	std::map<std::string, std::vector<std::string> > m_fileCache;
	std::list<FxAttacher*> m_attachers;
};

// CPetBody::SetRotate's X * Y * Z order, for the viewport.
WMatrix FxRotEuler(const WVector& r);

#endif // FXSIM_H
