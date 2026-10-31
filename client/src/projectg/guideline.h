#pragma once

#include "actor.h"
#include "scenemanager.h"

class CFxSequence;

class CGuideLine : public IActor, public CRenderFuncPtr
{
public:
	enum
	{
		MAX_POINT = 80
	};

	virtual ~CGuideLine() { }

	DECLARE_OBJECT(CGuideLine)

public:
	virtual void OnPreLoadInit();
	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);
	virtual void Display();

protected:
	CGuideLine();

	void SetActive(bool bActive);
	void SetPoint(const WVector& dir);
	void CalcSegment();

private:
	struct sRoad
	{
		WVector left;
		WVector right;
		WVector pos;
		CFxSequence* pSeq;
	};

	bool m_bHidden;
	bool m_bHideReq;
	int m_pointNum;
	float m_alpha;
	bool m_bActive;
	float m_lifeTime;
	bool m_bDelay;
	sRoad m_point[MAX_POINT];
	int m_texHandle;
	WTVertex m_vtx[4];
	WTVertex* m_pVtx[5];
	unsigned long m_color983; // +0xe2c: cor (diffuse) dos vertices no Display (983 0075b3f0); -1 = branco (padrao), outra cor encurta m_lifeTime
};
