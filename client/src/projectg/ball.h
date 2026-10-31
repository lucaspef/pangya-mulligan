#pragma once

#include <list>
#include "actor.h"
#include "scenemanager.h"
#include "fxbox.h"

class CFxSpray;
class CFxSequence;

class CBall : public IActor, public CRenderFuncPtr, public CFxKeeper
{
public:
	virtual ~CBall() { }

	DECLARE_OBJECT(CBall)

	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnReload();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();
	virtual void Display();
	virtual void EmptyFx(void* fx);

protected:
	virtual void HandleMsg(const MsgObject& msg);

	CBall();

	struct sBall
	{
		IFF_STRUCT::sBall* iff;
		WPuppet* puppet;
		CFxSpray* spray[4];
		CFxSequence* seq[4];

		sBall()
			: iff(NULL), puppet(NULL)
		{
			memset(spray, 0, sizeof(spray));
			memset(seq, 0, sizeof(seq));
		}
	};

	struct sFindBall
	{
		unsigned long m_tid;

		sFindBall(unsigned long tid)
			: m_tid(tid)
		{
		}
		bool operator()(sBall* ball) { return ball->iff->c.TypeId == m_tid; }
	};

	sBall* AddBall(unsigned long tid);
	sBall* FindBall(unsigned long tid);
	float GetDownDelta();
	void TransformBall();
	void AttachFx(int type);
	void DetachFx(int type);

	std::list<sBall*> m_ballList;
	WPuppet* m_pPuppet;
	int m_shadowTex;
	WMatrix m_matrix;
	WVector m_axis;
	bool m_bUpdateLight;
	float m_sink;
	int m_sinkFrame;
	LightSet m_light;
	bool m_bVisible;
	int m_lastFrame;
	WTVertex* m_pShadowVtx[5];
	WTVertex m_shadowVtx[4];
};
