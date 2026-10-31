#pragma once

class WOverlay;
#include "actor.h"

class CLogo : public IActor
{
public:
	virtual ~CLogo() { }

	DECLARE_OBJECT(CLogo)

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void OnDisplay();
	virtual void OnDestroy();
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CLogo();

	WOverlay* m_pLogo;
	float m_alpha;
};
