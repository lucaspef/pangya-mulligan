#pragma once

#include "actor.h"

class CGallery : public IActor
{
public:
	virtual ~CGallery() { }

protected:
	DECLARE_OBJECT(CGallery)

protected:
	CGallery();

public:
	virtual void OnProcess(float delta);
	virtual void HandleMsg(const MsgObject& msg);

protected:
	void SetSound(const char* name, float time);

	float m_time;
	char m_sound[32];
};
