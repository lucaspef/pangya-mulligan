#pragma once

#include "actor.h"

class CAmbientSound : public IActor
{
public:
	virtual ~CAmbientSound() { }

	DECLARE_OBJECT(CAmbientSound)

	virtual void OnLoad();
	virtual void OnInit();
	virtual void OnProcess(float delta);

protected:
	virtual void HandleMsg(const MsgObject& msg);

	CAmbientSound();

	float GetVolume(const Wobb& obb, float range);
	void UpdateRandomSound(float delta); // 983 0070e6b0
	void RandomizeNextRandomSound(int base, int range); // 983 0070db40

	std::vector<std::string> m_randomSounds; // 983 +0x18: container 0x9f[GetCurMap()]
	bool m_bRandomSound; // 983 +0x28
	float m_fNextRandomSound; // 983 +0x2c
};
