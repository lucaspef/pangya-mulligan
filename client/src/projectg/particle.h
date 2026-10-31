#pragma once

#include <list>
#include <string>
#include <vector>
#include <memory>
#include "actor.h"
#include "scenemanager.h"
#include "wlist.h"
#include "fxinterpreter.h"

class CParticleManager;
class CParticleRecv;
class CParticle;
class CLightColumn;
class CBgEffect;
class CGolfDoc;
class W3dAniSpr;
class Bitmap;

typedef char RbBitmapVecSize[sizeof(std::vector<const Bitmap*>)];

struct sRecv
{
	WVector center;
	float ground;
	float duration;
};

class CParticleEffect : public IActor, public CRenderFuncPtr
{
public:
	virtual ~CParticleEffect() { }

	DECLARE_OBJECT(CParticleEffect)

public:
	virtual void OnInit();
	virtual void OnPreLoadInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);
	virtual void Display();
	virtual void DisplayShadow();

	bool CheckExtShotEffect(int message); // 983: 007783e0, 1 argumento (a mensagem)
	void CallSprayParticle(const char* spray, const char* sound,
		const WVector* pPos);

protected:
	CParticleEffect();
	void Clear();

private:
	WList<CParticleRecv*> m_recvList;
	CParticleManager* m_pManager;
	sRecv m_recv;
	WList<CBgEffect*> m_bgList;
	CLightColumn* m_pLight;
	std::tr1::shared_ptr<FxInterpreter> m_spFxInterpreter983; // 983: 0x78 (construtor 00779c87..00779d05); sizeof 0x80
#ifndef REBANG_SEMANTIC_ONLY
public:
	// HACK: EH funclets of the naked HandleMsg (nakedify)
	static void __cdecl RbHandlemsgFunclets();
#endif
};

class CParticleManager
{
public:
	struct sTexture
	{
		char name[0x20];
		int handle;

		sTexture() { handle = 0; }
	};

	CParticleManager();
	~CParticleManager();

	void Process(float elapsed);
	void Display();
	void DisplayShadow();
	void MakeParticle(CParticle* pParticle, const char* texName, bool bKeep);
	void RemoveAll();
	void Remove(const char* texName);

protected:
	void RegisterTexture(const char* key, const char* file);

private:
	CParticle* m_pParticle[20];
	int m_last;
	int m_count;
	int m_keep;
	WList<sTexture*> m_texList;
};

// A single-texture particle emitter described by a .eft file.
class CParticleRecv
{
public:
	struct sFrag
	{
		WVector pos;
		WVector vel;
		float alpha;
		bool bActive;
		float angle;
		float angleSpeed;
		float size;
		int frame;
		float frameTime;
		float life;
		float remain;
	};

	CParticleRecv(const char* file);
	~CParticleRecv();

	void SetActive(bool bActive) { m_bActive = bActive; }
	bool IsActive() { return m_bActive; }
	void SetCenter(const WVector& center) { m_center = center; }
	void SetGround(float ground) { m_ground = ground; }
	void SetDuration(float duration)
	{
		if (duration > 0.0f)
			m_duration = duration;
	}
	void SetTarget(WVector* pTarget);
	void SetBox(const Waabb& box);
	void Process(float elapsed);
	void Display();
	static void Disconnect();

protected:
	void Destroy();
	void Generate();
	void BlowAway(float elapsed, int index);
	void ApplyScale();

private:
	int m_type;
	float m_alphaStart;
	float m_alphaEnd;
	float m_sizeStart;
	float m_sizeEnd;
	float m_velYMin;
	float m_velYMax;
	float m_speedMin;
	float m_speedMax;
	float m_spinMin;
	float m_spinMax;
	float m_gravity;
	float m_lifeMin;
	float m_lifeMax;
	float m_frameTime;
	float m_interval;
	float m_bounce;
	float m_duration;
	float m_ground;
	unsigned long m_renderFlag;
	bool m_bActive;
	sFrag* m_pFrag;
	int m_liveCount;
	W3dAniSpr* m_pSpr;
	WMatrix m_matrix;
	WVector m_rot;
	WVector m_accel;
	Waabb m_box;
	float m_radius;
	bool m_bRandFrame;
	float m_sprSize;
	int m_frame;
	char m_sprName[0x20];
	int m_poolSize;
	int m_head;
	WVector* m_pTarget;
	WVector m_center;
	float m_accum;
	int m_emitCount;
	bool m_bVisible;
	int m_objIndex;
	float m_objAlpha;
	float m_scale;

	static CGolfDoc* m_pDoc;
};

// Screen-space particles (falling leaves, snow, ...).
class CParticle2D
{
public:
	enum eSTYLE
	{
		STYLE_NORMAL,
		STYLE_BLINK
	};

	struct sParticle
	{
		WVector2D pos;
		WVector2D vel;
		const Bitmap* pBitmap;
		float alpha;
		int alphaDir;
		float speed;
		int bAlive;
		WRect src;
		WRect dst;
	};

	CParticle2D() { }
	~CParticle2D();

	int Initialize(const std::list<std::string>& files, const WRect& rect,
		unsigned int count, eSTYLE style, float minSpeed, float maxSpeed,
		const WVector2D& velMin, const WVector2D& velMax);
	void Process(float elapsed);
	void Display();
	void Release();

private:
	void MakeDisplayRect(sParticle* pParticle);

	WRect m_rect;
	eSTYLE m_style;
	unsigned int m_count;
	float m_minSpeed;
	float m_maxSpeed;
	WVector2D m_velMin;
	WVector2D m_velMax;
	std::list<sParticle*> m_list;
	std::vector<const Bitmap*> m_bitmaps;
};
