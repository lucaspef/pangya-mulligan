#pragma once

#include "wmath.h"
#include "wvideo.h"
#include "w3dspr.h"
#include "w3danispr.h"

class CPetBody;
class IActor;

// Base of the in-game particle effects managed by CParticleManager.
class CParticle
{
public:
	CParticle();
	virtual ~CParticle();
	virtual void Process(float dt) = 0;
	virtual void Display() = 0;
	virtual void DisplayShadow() { }
	virtual void MakeParticle() = 0;

	int GetTexHandle() { return m_texHandle; }
	void LoadTexture(int texHandle) { m_texHandle = texHandle; }
	virtual void SetActive(bool bActive) { m_bActive = bActive; }
	void SetCenter(const WVector& center) { m_center = center; }
	bool IsActive() { return m_bActive; }

	virtual void SetNormal(const WVector& normal) { }

protected:
	virtual void Generate() { }
	virtual void BlowAway(float dt, int index) { }

	int m_count;

public:
	WVector m_center;

protected:
	int m_texHandle;
	bool m_bActive;
	W3dAniSpr* m_pAniSpr;
	W3dSpr* m_pSpr;
	float m_genTime;
	float m_genInterval;
	int m_unused2c;
	WTVertex* m_vl[5];
	WTVertex m_vtx[4];

#ifdef REBANG_PARTICLEPARTY_CPP
	// HACK: see RbDustCopy below; a declared copy constructor keeps CParticle's own implicit one
	// (and its vtable reference) from being created there.
	CParticle(const CParticle&);
#endif
};

class CShotParticle : public CParticle
{
public:
	struct sPiece
	{
		bool active;
		WVector pos;
		WVector dir;
		float life;
		float length;
		float maxLife;
	};

	CShotParticle();
	virtual ~CShotParticle();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	sPiece* m_pPiece;
	int m_numPiece;
};

class CPowerShot : public CParticle
{
public:
	CPowerShot();
	virtual ~CPowerShot();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	float m_radius;
	float m_ratio;
	int m_numSeg;
	int m_alpha;
	float m_duration;
	float m_time;
	float m_speed;
	float m_scale;
};

class CLightColumn : public CParticle
{
public:
	CLightColumn();
	virtual ~CLightColumn();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

	void FadeIn() { m_fadeState = 1; }
	void FadeOut() { m_fadeState = 2; }
	void SetMode983(int mode) { m_mode983 = mode; } // 983: usado por CParticleEffect::HandleMsg (mensagem 0x1ef); nome meu

private:
	int m_fadeState;
	float m_height;
	float m_radius;
	unsigned char m_alpha0;
	unsigned char m_alpha1;
	int m_tex[5];
	int m_texScreen;
	float m_scroll;
	int m_texLevel983; // 983: 0x1d0, zerado no construtor (006eb8d4); nivel da cor da coluna no gameType 0xb (modo 0x1ef: 3 -> 0, 4 -> 1)
	int m_mode983; // 983: 0x1d4, zerado no construtor; 3 ou 4 pela mensagem 0x1ef; sizeof 0x1d8 (push em 00778cf9)
};

class CDust : public CParticle
{
public:
	struct sParticle
	{
		WVector pos;
		float rot;
		float size0;
		float size1;
	};

	CDust(int num);
	virtual ~CDust();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	sParticle* m_pParticle;
	int m_numParticle;
	float m_interval;
	float m_time;
};

#ifdef REBANG_PARTICLEPARTY_CPP
// HACK: emission order. The original creates CDust's scalar deleting destructor at this point of
// the header (it is emitted between CParticle::SetCenter and CFragment::SetNormal) and not next
// to CDust's constructor in the .cpp like every other class. The first vtable reference decides
// that position: this never-called inline creates CDust's implicit copy constructor here.
inline void RbDustCopy(const CDust& dust)
{
	CDust copy(dust);
}
#endif

class CGrassParticle : public CParticle
{
public:
	struct sGrass
	{
		int frame;
		float alpha;
		float rot;
		float rotSpeed;
		WVector pos;
		WVector vel;
		bool active;
		float life;
	};

	CGrassParticle(int num, const WVector& dir);
	virtual ~CGrassParticle();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	sGrass* m_pGrass;
	int m_numGrass;
	WVector m_dir;
};

class CBunkerShotParticle : public CParticle
{
public:
	struct sSmoke
	{
		WVector pos;
		WVector vel;
	};

	struct sParticle
	{
		bool active;
		WVector pos;
		WVector vel;
		float length;
		float life;
		float maxLife;
	};

	CBunkerShotParticle(int numSmoke, int numParticle);
	virtual ~CBunkerShotParticle();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

protected:
	virtual void Generate();

private:
	sSmoke* m_pSmoke;
	sParticle* m_pParticle;
	int m_numSmoke;
	int m_numParticle;
	float m_interval;
	float m_time;
	unsigned long m_color;
};

class CBunkerParticle : public CParticle
{
public:
	CBunkerParticle();
	virtual ~CBunkerParticle();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	int m_alpha;
	float m_time;
	int m_unusedec;
	float m_size;
	float m_interval;
};

class CFragment : public CParticle
{
public:
	struct sFrag
	{
		WVector pos;
		WVector vel;
	};

	CFragment();
	virtual ~CFragment();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();
	virtual void SetNormal(const WVector& normal) { m_normal = normal; }

private:
	sFrag* m_pFrag;
	int m_numFrag;
	int m_unusedec[2];
	float m_alpha;
	WVector m_normal;
#ifndef REBANG_SEMANTIC_ONLY
public:
	static void __cdecl
	RbMakeparticleFunclets(); // HACK: EH funclets of the naked MakeParticle (nakedify)
#endif
};

class CDroplet : public CParticle
{
public:
	struct sDroplet
	{
		WVector pos;
		WVector vel;
		float speed;
	};

	CDroplet();
	virtual ~CDroplet();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

private:
	sDroplet* m_pDroplet;
	int m_numDroplet;
	float m_interval;
	float m_alpha;
};

class CRipple : public CParticle
{
public:
	struct sRipple
	{
		float size;
		float alpha;
		float startAlpha;
		float alphaSpeed;
		float sizeSpeed;
		bool active;
		float delay;
		float wait;
	};

	CRipple();
	virtual ~CRipple();
	void Init();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

protected:
	void AddRipple(float sizeSpeed, float delay, float startAlpha,
		float alphaSpeed);

private:
	sRipple* m_pRipple;
	int m_numRipple;
	CParticle* m_pDroplet;
};

class CCoin : public CParticle
{
public:
	struct sCoin
	{
		sCoin()
			: pBody(NULL)
		{
		}

		CPetBody* pBody;
		float rot;
		WVector vel;
		float bounce;
	};

	struct sSpark
	{
		bool active;
		WVector pos;
		WVector vel;
		float alpha;
		float life;
		float maxLife;
	};

	CCoin(int num, const WVector& pos);
	virtual ~CCoin();
	virtual void Process(float dt);
	virtual void Display();
	virtual void DisplayShadow();
	virtual void MakeParticle();
	virtual void SetActive(bool bActive);

protected:
	bool ShouldDisplay();
	virtual void Generate();

private:
	sCoin* m_pCoin;
	sSpark* m_pSpark;
	int m_numCoin;
	int m_numSpark;
	WVector m_pos;
	float m_interval;
	float m_time;
	int m_numDropSound;
};

class CEnergyCenter : public CParticle
{
public:
	struct sSpark
	{
		bool active;
		WVector pos;
		WVector dir;
		float alpha;
	};

	CEnergyCenter(int num, const WVector& pos, float rotSpeed);
	virtual ~CEnergyCenter();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

protected:
	void Generate(float dt);

private:
	sSpark* m_pSpark;
	float m_rotSpeed;
	int m_numGenerated;
	int m_numSpark;
	WVector m_pos;
	float m_coreAlpha;
	float m_coreRot;
	float m_bgAlpha;
	float m_lastBgAlpha;
	IActor* m_pGolfBg;
};

class CStarDust : public CParticle
{
public:
	struct sParticle
	{
		bool active;
		float radius;
		WVector pos;
		WVector axisX;
		WVector axisY;
		WVector unused2c;
		WVector center;
		float angle;
		float angleSpeed;
		float life;
	};

	CStarDust();
	virtual ~CStarDust();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

protected:
	virtual void Generate();
	virtual void BlowAway(float dt, int index);

private:
	sParticle* m_pParticle;
	int m_numParticle;
	int m_next;
	float m_rot;
	float m_alpha;
};

class CStarDust2 : public CParticle
{
public:
	struct sParticle
	{
		bool active;
		float radius;
		WVector pos;
		WVector axisX;
		WVector axisY;
		WVector unused2c;
		WVector center;
		float angle;
		float angleSpeed;
		float alpha;
	};

	CStarDust2();
	virtual ~CStarDust2();
	virtual void Process(float dt);
	virtual void Display();
	virtual void MakeParticle();

protected:
	virtual void Generate();

private:
	sParticle* m_pParticle;
	int m_numParticle;
	int m_next;
	float m_rot;
	float m_alpha;
};
