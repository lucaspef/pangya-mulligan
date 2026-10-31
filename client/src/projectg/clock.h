#pragma once

#include "actor.h"

class CClock : public IActor
{
public:
	virtual ~CClock() { }

protected:
	DECLARE_OBJECT(CClock)

public:
	virtual void OnInit();
	virtual void OnProcess(float delta);
	virtual void HandleMsg(const MsgObject& msg);

protected:
	CClock();

	void Reset();

	unsigned long m_reserved;
	float m_angle;
	unsigned long m_color;
	unsigned char m_reserved2;
	bool m_bActive;
};
