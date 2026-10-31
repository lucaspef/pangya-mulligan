#pragma once
#include "msg983.h"

#include "actor.h"

class CTalkBox;
class MsgObject;

class CBubble : public IActor
{
public:
	virtual ~CBubble() { }

	DECLARE_OBJECT(CBubble)

	virtual void OnLoad();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();

	struct sBubble
	{
		CTalkBox* pTalkBox;
		WVector pos;
		bool bTalking;
	};

protected:
	CBubble();

	virtual void HandleMsg(const MsgObject& msg);

	sBubble* m_bubble;
	bool m_bShow;
};
